<?php

/**
 * ============================================================
 * 파일명: ai_oee_prediction_dash_2.php
 * 목  적: AI OEE 예측 대시보드 API v5 (클램핑 + today_data 신규 반환)
 *
 * v4 대비 수정 사항:
 *  - current_oee: date_range 무관하게 항상 오늘(CURDATE()) 기준 조회
 *    → date_range가 '7d'여도 현재 OEE는 오늘 데이터에서 가져옴
 *  - current_oee: min(100, max(0, ...)) 클램핑 적용
 *    → DB에 100% 초과 비정상 값이 있어도 current_oee는 정상 범위
 *  - today_data: 오늘 시간대별 실제 OEE 배열 반환 (차트 Actual 라인용)
 *  - CI 계산 시 seasonal_std 상한 15%로 제한
 *    → 변동성이 매우 큰 경우 신뢰구간이 0~100% 전 구간이 되는 현상 방지
 *  - date_range: 예측 패턴 학습용 히스토리 기간에만 영향
 *    (current_oee·today_data는 항상 오늘 기준)
 *
 * 동작 개요:
 *   1) date_range에 따라 히스토리 학습 기간(7일/30일) 결정
 *   2) 과거 N일 시간대별 OEE 통계 → 계절성 패턴 매트릭스 구성 (클램핑 적용)
 *   3) 오늘 시간대별 실제 OEE 조회 (항상 CURDATE() 고정, 클램핑 적용)
 *   4) 지수평활법 + 계절성 가중 평균으로 향후 4시간 OEE 예측
 *   5) 90% 신뢰구간 산출 (seasonal_std 상한 15%로 제한)
 *   6) 선형회귀로 트렌드 방향(up/down/stable) 판단
 *
 * Method: GET
 * Params:
 *   factory_filter, line_filter, machine_filter (optional)
 *   date_range: today|yesterday|7d|30d (default: today)
 *     → 히스토리 학습 기간 결정에만 영향
 * Response JSON 구조:
 *   {
 *     code: '00' | '99',
 *     current_oee: 클램핑된 현재(오늘 최신 시간대) OEE,
 *     current_hour: 현재 집계 시간(0~23),
 *     today_data: [ { hour, label, oee } ],  ← v5 신규
 *     forecast: [ { hour, label, oee, lower, upper, data_cnt } ],
 *     trend: 'up' | 'down' | 'stable',
 *     method: 'exponential_smoothing + seasonal_v5',
 *     history_days: 학습 기간(일)
 *   }
 * ============================================================
 */

// DB 연결 및 통계 라이브러리 로드
require_once(__DIR__ . '/../../../lib/db.php');
require_once(__DIR__ . '/../../../lib/statistics.lib.php');

// JSON 응답 헤더 설정
header('Content-Type: application/json');
header('Cache-Control: no-cache');

// ── GET 파라미터 수집 ────────────────────────────────────────────
$factory_filter = isset($_GET['factory_filter']) ? trim($_GET['factory_filter']) : '';
$line_filter    = isset($_GET['line_filter'])    ? trim($_GET['line_filter'])    : '';
$machine_filter = isset($_GET['machine_filter']) ? trim($_GET['machine_filter']) : '';
$date_range     = isset($_GET['date_range'])     ? trim($_GET['date_range'])     : 'today';

// ── date_range → 히스토리 학습 기간 결정 ────────────────────────
// 예측 패턴 학습 기간에만 영향, current_oee·today_data는 항상 오늘 기준
switch ($date_range) {
    case 'yesterday':
        $history_days = 30; // 어제 기반이어도 패턴은 30일 학습
        break;
    case '7d':
        $history_days = 7;  // 최근 7일 패턴 학습
        break;
    case '30d':
        $history_days = 30; // 최근 30일 패턴 학습
        break;
    default: // today → 30일 기본 학습
        $history_days = 30;
}

// ── 필터 조건 구성 ───────────────────────────────────────────────
$where_parts = [];
$params      = [];

if ($factory_filter !== '') {
    $where_parts[] = 'doh.factory_idx = ?'; // 공장 필터
    $params[] = $factory_filter;
}
if ($line_filter    !== '') {
    $where_parts[] = 'doh.line_idx = ?';   // 라인 필터
    $params[] = $line_filter;
}
if ($machine_filter !== '') {
    $where_parts[] = 'doh.machine_idx = ?'; // 머신 필터
    $params[] = $machine_filter;
}

// 조건이 있으면 'AND ...' 형태로 결합
$where_sql = $where_parts ? 'AND ' . implode(' AND ', $where_parts) : '';

try {
    // ════════════════════════════════════════════════════════
    // STEP 1. 히스토리 기반 계절성 패턴 학습 (요일×시간대 매트릭스)
    //   - history_days(7 또는 30일) 기간의 시간대별 OEE 통계 산출
    //   - LEAST(100, GREATEST(0, doh.oee)) : 클램핑 적용
    //     → 125.6% 같은 비정상 OEE가 표준편차를 왜곡하지 않도록 방지
    //   - ? 바인딩 파라미터로 history_days 값 전달
    // ════════════════════════════════════════════════════════
    $sql_history = "
    SELECT
      DAYOFWEEK(doh.work_date) AS dow,
      doh.work_hour            AS hour,
      AVG(LEAST(100, GREATEST(0, doh.oee))) AS avg_oee,    -- 클램핑 후 평균
      STDDEV(LEAST(100, GREATEST(0, doh.oee))) AS std_oee, -- 클램핑 후 표준편차
      COUNT(*) AS cnt
    FROM data_oee_rows_hourly doh
    WHERE doh.work_date >= DATE_SUB(CURDATE(), INTERVAL ? DAY)
      AND doh.oee IS NOT NULL
      $where_sql
    GROUP BY DAYOFWEEK(doh.work_date), doh.work_hour
    ORDER BY dow, hour
  ";
    $stmt = $pdo->prepare($sql_history);
    // history_days 를 첫 번째 파라미터로, 이후 필터 파라미터 추가
    $stmt->execute(array_merge([$history_days], $params));
    $history = $stmt->fetchAll(PDO::FETCH_ASSOC);

    // ── 요일×시간대 계절성 매트릭스 구성 ─────────────────────
    // key 형식: '{dow}_{hour}' (예: '2_9' = 월요일 9시)
    $matrix = [];
    foreach ($history as $row) {
        $key = $row['dow'] . '_' . $row['hour'];
        $matrix[$key] = [
            'avg' => (float)$row['avg_oee'], // 해당 요일·시간대 평균 OEE (클램핑 후)
            'std' => (float)$row['std_oee'], // 해당 요일·시간대 표준편차 (클램핑 후)
            'cnt' => (int)$row['cnt'],       // 데이터 건수
        ];
    }

    // ════════════════════════════════════════════════════════
    // STEP 2a. 오늘 실제 OEE 조회 (항상 CURDATE() 고정)
    //   - LIVE Real-time OEE 카드 및 예측 알고리즘 기반 데이터
    //   - date_range 영향 없음 — 항상 오늘 기준
    // ════════════════════════════════════════════════════════
    $sql_today_live = "
    SELECT
      doh.work_hour AS hour,
      AVG(LEAST(100, GREATEST(0, doh.oee))) AS avg_oee  -- 클램핑된 시간대별 평균 OEE
    FROM data_oee_rows_hourly doh
    WHERE doh.work_date = CURDATE()        -- LIVE 카드·예측 기반: 항상 오늘 고정
      AND doh.oee IS NOT NULL
      $where_sql
    GROUP BY doh.work_hour
    ORDER BY doh.work_hour
  ";
    $stmt2 = $pdo->prepare($sql_today_live);
    $stmt2->execute($params);
    $today_rows_live = $stmt2->fetchAll(PDO::FETCH_ASSOC);

    // LIVE 카드 및 예측 기반용 배열
    $today_hours = []; // 시간(0~23) 정수 배열
    $today_oees  = []; // OEE 값 배열 (지수평활 입력용)
    foreach ($today_rows_live as $row) {
        $today_hours[] = (int)$row['hour'];
        $today_oees[]  = round((float)$row['avg_oee'], 1);
    }

    // current_oee: 오늘 가장 최근 시간대 값, 클램핑 적용 (0~100 범위 강제)
    $current_oee  = !empty($today_oees) ? min(100.0, max(0.0, round(end($today_oees), 1))) : null;
    // current_hour: 최신 집계 시간(없으면 시스템 현재 시각 사용)
    $current_hour = !empty($today_hours) ? end($today_hours) : (int)date('G');

    // ════════════════════════════════════════════════════════
    // STEP 2b. 차트 Actual 라인용 OEE 조회 (date_range에 따라 날짜 결정)
    //   - today / 7d / 30d → CURDATE() (오늘)
    //   - yesterday         → DATE_SUB(CURDATE(), INTERVAL 1 DAY) (어제)
    //   - today_data: 차트 Actual 라인 응답 배열 [{hour, label, oee}]
    // ════════════════════════════════════════════════════════
    $chart_date_sql = ($date_range === 'yesterday')
        ? "DATE_SUB(CURDATE(), INTERVAL 1 DAY)"
        : "CURDATE()";

    $sql_today_chart = "
    SELECT
      doh.work_hour AS hour,
      AVG(LEAST(100, GREATEST(0, doh.oee))) AS avg_oee  -- 클램핑된 시간대별 평균 OEE
    FROM data_oee_rows_hourly doh
    WHERE doh.work_date = {$chart_date_sql}  -- date_range 반영: today=오늘, yesterday=어제
      AND doh.oee IS NOT NULL
      $where_sql
    GROUP BY doh.work_hour
    ORDER BY doh.work_hour
  ";
    $stmt_chart = $pdo->prepare($sql_today_chart);
    $stmt_chart->execute($params);
    $today_rows_chart = $stmt_chart->fetchAll(PDO::FETCH_ASSOC);

    // today_data: API 응답용 차트 Actual 라인 배열
    $today_data = [];
    foreach ($today_rows_chart as $row) {
        $hour = (int)$row['hour'];
        $oee  = round((float)$row['avg_oee'], 1);
        $today_data[] = ['hour' => $hour, 'label' => sprintf('%02d:00', $hour), 'oee' => $oee];
    }

    // ════════════════════════════════════════════════════════
    // STEP 3. 예측 시간대 계산
    //   - 현재 시간 이후 4시간의 시각(0~23 범위로 순환)
    // ════════════════════════════════════════════════════════
    $forecast_hours = [];
    for ($i = 1; $i <= 4; $i++) {
        $forecast_hours[] = ($current_hour + $i) % 24; // 자정(24시) 이후 순환 처리
    }

    // PHP date('w'): 0=일요일, MySQL DAYOFWEEK(): 1=일요일 → +1 보정
    $dow = (int)date('w') + 1;

    // ════════════════════════════════════════════════════════
    // STEP 4. 예측값 계산 (계절성 + 지수평활 가중 평균)
    //   - exponentialSmoothing(α=0.3, 4시간): 오늘 OEE 배열로 단기 트렌드 추출
    //   - 계절성(70%) + 지수평활(30%) 가중 평균
    //   - 신뢰구간 CI: seasonal_std 상한 15% 제한 (v5 수정)
    //     → std가 매우 크면 CI가 0~100% 전 구간이 되는 문제 방지
    // ════════════════════════════════════════════════════════
    $forecast   = [];
    // 지수평활법으로 단기 트렌드 추출 (α=0.3, 4시간 예측)
    $es_result  = !empty($today_oees) ? exponentialSmoothing($today_oees, 0.3, 4) : null;
    $es_forecast = $es_result ? $es_result['forecast'] : null;

    foreach ($forecast_hours as $idx => $hour) {
        $key = $dow . '_' . $hour; // 오늘 요일 × 예측 시간 키

        // 계절성 패턴 값 (해당 요일·시간대 과거 평균)
        $seasonal_avg = isset($matrix[$key]) ? $matrix[$key]['avg'] : null;
        // v5 수정: std 상한 15% — CI가 0~100% 전 구간이 되는 현상 방지
        $seasonal_std = isset($matrix[$key]) ? min(15.0, $matrix[$key]['std']) : 5.0;

        // ── 예측 우선순위: 계절성+지수평활 → 계절성만 → 지수평활만 → 현재값 ──
        if ($seasonal_avg !== null && $es_forecast !== null) {
            // 계절성 70% + 지수평활 30% 가중 평균
            $pred_oee = $seasonal_avg * 0.7 + $es_forecast[$idx] * 0.3;
        } elseif ($seasonal_avg !== null) {
            $pred_oee = $seasonal_avg; // 계절성 패턴만 사용
        } elseif ($es_forecast !== null) {
            $pred_oee = $es_forecast[$idx]; // 지수평활만 사용
        } else {
            $pred_oee = $current_oee ?? 70.0; // 데이터 없으면 현재값 또는 70% 기본값
        }

        // OEE 범위 클램핑 (0~100%)
        $pred_oee = round(max(0, min(100, $pred_oee)), 1);

        // 90% 신뢰구간 계산 (z=1.645, std 최솟값 1.0 보정)
        $ci = calcConfidenceInterval($pred_oee, max(1.0, $seasonal_std), 1.645);

        $forecast[] = [
            'hour'     => $hour,
            'label'    => sprintf('%02d:00', $hour), // 시간 레이블 (예: '09:00')
            'oee'      => $pred_oee,       // 예측 OEE (%)
            'lower'    => $ci['lower'],    // 90% CI 하한
            'upper'    => $ci['upper'],    // 90% CI 상한
            'data_cnt' => isset($matrix[$key]) ? $matrix[$key]['cnt'] : 0, // 학습 데이터 건수
        ];
    }

    // ════════════════════════════════════════════════════════
    // STEP 5. 트렌드 방향 판단
    //   - 오늘 데이터 3개 이상이어야 신뢰성 있는 기울기 계산 가능
    //   - 최근 6개 시간대 데이터로 선형회귀 실행
    //   - slope > +0.5 → 'up' (상승 추세)
    //   - slope < -0.5 → 'down' (하락 추세)
    //   - 그 외       → 'stable' (안정적)
    // ════════════════════════════════════════════════════════
    $trend = 'stable';
    if (!empty($today_oees) && count($today_oees) >= 3) {
        // 최근 6개 시간대 데이터로 선형회귀, 1시간 단위 기울기 계산
        $lr = linearRegression(array_slice($today_oees, -6), 1);
        if ($lr['slope'] > 0.5)      $trend = 'up';   // 상승 추세
        elseif ($lr['slope'] < -0.5) $trend = 'down'; // 하락 추세
    }

    // ── 최종 JSON 응답 출력 ──────────────────────────────────
    echo json_encode([
        'code'         => '00',
        'current_oee'  => $current_oee,   // 클램핑된 현재(오늘 최신) OEE
        'current_hour' => $current_hour,  // 현재 집계 시간
        'today_data'   => $today_data,    // v5 신규: 오늘 시간대별 실제 OEE 배열
        'forecast'     => $forecast,      // 향후 4시간 예측 배열
        'trend'        => $trend,         // 트렌드 방향
        'method'       => 'exponential_smoothing + seasonal_v5', // 사용 알고리즘
        'history_days' => $history_days,  // 학습 기간 (일)
    ]);
} catch (PDOException $e) {
    // DB 오류 발생 시 code 99 와 오류 메시지 반환
    echo json_encode(['code' => '99', 'msg' => 'DB error: ' . $e->getMessage()]);
}
