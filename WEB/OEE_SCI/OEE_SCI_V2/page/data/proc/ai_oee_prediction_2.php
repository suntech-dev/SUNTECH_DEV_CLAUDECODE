<?php

/**
 * ============================================================
 * 파일명: ai_oee_prediction_2.php
 * 목  적: AI OEE 예측 API — 과거 30일 데이터 기반 향후 4시간 OEE 예측
 *
 * 동작 개요:
 *   1) 과거 30일 시간대별(요일×시간) OEE 통계 수집 → 계절성 패턴 매트릭스 구성
 *   2) 오늘 현재까지의 실제 시간대별 OEE 조회
 *   3) 지수평활법(α=0.3)으로 단기 트렌드 추출
 *   4) 계절성(70%) + 지수평활(30%) 가중 평균으로 향후 4시간 예측값 산출
 *   5) 90% 신뢰구간(CI) 계산 → lower/upper 반환
 *   6) 선형회귀로 트렌드 방향(up/down/stable) 판단
 *
 * Method: GET
 * Params:
 *   factory_filter, line_filter, machine_filter (optional)
 *   date_range: today|yesterday|7d|30d (default: today)
 *     → 오늘(today) 기준 현재 OEE 조회 날짜 조건에 적용
 * Response JSON 구조:
 *   {
 *     code: '00' | '99',
 *     current_oee: 현재(가장 최근 시간대) OEE,
 *     current_hour: 현재 시간(0~23),
 *     forecast: [
 *       { hour, label, oee, lower, upper, data_cnt }
 *     ],
 *     trend: 'up' | 'down' | 'stable',
 *     method: 'exponential_smoothing + seasonal',
 *     data_days: 30
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

// ── date_range → SQL 날짜 조건 변환 (data_oee_rows_hourly alias: doh) ─────
// 오늘(당일) 실제 OEE를 조회할 때 적용되는 날짜 조건
switch ($date_range) {
    case 'yesterday': // 어제 하루
        $actual_date_where = "doh.work_date = DATE_SUB(CURDATE(), INTERVAL 1 DAY)";
        break;
    case '7d': // 최근 7일
        $actual_date_where = "doh.work_date >= DATE_SUB(CURDATE(), INTERVAL 6 DAY)";
        break;
    case '30d': // 최근 30일
        $actual_date_where = "doh.work_date >= DATE_SUB(CURDATE(), INTERVAL 29 DAY)";
        break;
    default: // today (기본) — 오늘 날짜만 조회
        $actual_date_where = "doh.work_date = CURDATE()";
}

// ── 필터 조건 구성 ───────────────────────────────────────────────
// data_oee_rows_hourly 는 독립 테이블 — JOIN 없이 직접 필터링
$where_parts = [];
$params      = [];

if ($factory_filter !== '') {
    $where_parts[] = 'doh.factory_idx = ?'; // 공장 필터
    $params[]      = $factory_filter;
}
if ($line_filter !== '') {
    $where_parts[] = 'doh.line_idx = ?';   // 라인 필터
    $params[]      = $line_filter;
}
if ($machine_filter !== '') {
    $where_parts[] = 'doh.machine_idx = ?'; // 머신 필터
    $params[]      = $machine_filter;
}

// 조건이 있으면 'AND ...' 형태로 결합
$where_sql = $where_parts ? 'AND ' . implode(' AND ', $where_parts) : '';

try {
    // ════════════════════════════════════════════════════════
    // STEP 1. 과거 30일 시간대별 OEE 데이터 수집
    //   - DAYOFWEEK() : 1=일요일 ~ 7=토요일 (MySQL 기준)
    //   - 요일(dow) × 시간(hour) 조합의 평균·표준편차 산출
    //   - 이 결과로 계절성 패턴 매트릭스($matrix) 구성
    // ════════════════════════════════════════════════════════
    $sql_history = "
    SELECT
      DAYOFWEEK(doh.work_date) AS dow,
      doh.work_hour            AS hour,
      AVG(doh.oee)             AS avg_oee,
      STDDEV(doh.oee)          AS std_oee,
      COUNT(*)                 AS cnt
    FROM data_oee_rows_hourly doh
    WHERE doh.work_date >= DATE_SUB(CURDATE(), INTERVAL 30 DAY)
      AND doh.oee IS NOT NULL
      $where_sql
    GROUP BY DAYOFWEEK(doh.work_date), doh.work_hour
    ORDER BY dow, hour
  ";
    $stmt = $pdo->prepare($sql_history);
    $stmt->execute($params);
    $history = $stmt->fetchAll(PDO::FETCH_ASSOC);

    // ── 요일×시간대 계절성 매트릭스 구성 ─────────────────────
    // key 형식: '{dow}_{hour}' (예: '2_9' = 월요일 9시)
    $matrix = [];
    foreach ($history as $row) {
        $key = $row['dow'] . '_' . $row['hour'];
        $matrix[$key] = [
            'avg' => (float)$row['avg_oee'], // 해당 요일·시간대 평균 OEE
            'std' => (float)$row['std_oee'], // 해당 요일·시간대 표준편차
            'cnt' => (int)$row['cnt'],       // 데이터 건수
        ];
    }

    // ════════════════════════════════════════════════════════
    // STEP 2. 오늘(또는 지정 날짜) 현재까지의 시간대별 실제 OEE 조회
    //   - 가장 최근 시간대 OEE → current_oee 에 사용
    //   - 배열 형태로 지수평활법 입력값($today_data)으로 사용
    // ════════════════════════════════════════════════════════
    $sql_today = "
    SELECT
      doh.work_hour AS hour,
      AVG(doh.oee)  AS avg_oee
    FROM data_oee_rows_hourly doh
    WHERE $actual_date_where
      AND doh.oee IS NOT NULL
      $where_sql
    GROUP BY doh.work_hour
    ORDER BY doh.work_hour
  ";
    $stmt2 = $pdo->prepare($sql_today);
    $stmt2->execute($params);
    $today_rows = $stmt2->fetchAll(PDO::FETCH_ASSOC);

    // 오늘 시간대별 OEE 배열 구성
    $today_data  = []; // OEE 값 배열 (지수평활 입력용)
    $today_hours = []; // 시간(0~23) 배열
    foreach ($today_rows as $row) {
        $today_hours[] = (int)$row['hour'];
        $today_data[]  = (float)$row['avg_oee'];
    }

    // 현재 OEE: 가장 최근 시간대 값 (없으면 null)
    $current_oee  = !empty($today_data) ? round(end($today_data), 1) : null;
    // 현재 시간: 데이터가 있으면 마지막 집계 시간, 없으면 시스템 시각 사용
    $current_hour = !empty($today_hours) ? end($today_hours) : (int)date('G');

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
    // STEP 4. 예측값 계산
    //   방법: 계절성(70%) + 지수평활(30%) 가중 평균
    //
    //   지수평활법(exponentialSmoothing):
    //     - α=0.3: 과거 데이터에 지수적 가중치 부여
    //     - 최근 데이터일수록 높은 가중치 → 단기 트렌드 반영
    //     - 반환: ['forecast' => [예측값 배열, 4개]]
    //
    //   계절성 패턴:
    //     - 같은 요일·시간대의 과거 30일 평균 OEE 활용
    //     - 70% 비중으로 반영 → 장기 패턴 안정성 확보
    // ════════════════════════════════════════════════════════
    $forecast = [];

    // 지수평활법으로 단기 트렌드 추출 (α=0.3, 4시간 예측)
    $es_result   = !empty($today_data) ? exponentialSmoothing($today_data, 0.3, 4) : null;
    $es_forecast = $es_result ? $es_result['forecast'] : null;

    foreach ($forecast_hours as $idx => $hour) {
        $key = $dow . '_' . $hour; // 오늘 요일 × 예측 시간 키

        // 계절성 패턴 값 (해당 요일·시간대 과거 평균)
        $seasonal_avg = isset($matrix[$key]) ? $matrix[$key]['avg'] : null;
        // 표준편차 (CI 계산에 사용, 최솟값 5.0 보정)
        $seasonal_std = isset($matrix[$key]) ? $matrix[$key]['std'] : 5.0;

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

        // 시간 레이블 생성 (예: '09:00')
        $label = sprintf('%02d:00', $hour);
        $forecast[] = [
            'hour'      => $hour,
            'label'     => $label,
            'oee'       => $pred_oee,       // 예측 OEE (%)
            'lower'     => $ci['lower'],    // 90% CI 하한
            'upper'     => $ci['upper'],    // 90% CI 상한
            'data_cnt'  => isset($matrix[$key]) ? $matrix[$key]['cnt'] : 0, // 학습 데이터 건수
        ];
    }

    // ════════════════════════════════════════════════════════
    // STEP 5. 트렌드 방향 판단
    //   - 오늘 데이터가 3개 이상이어야 신뢰성 있는 기울기 계산 가능
    //   - linearRegression(): 최근 6개 시간대 데이터로 선형회귀 실행
    //   - slope > +0.5 → 'up' (시간당 0.5%p 이상 상승)
    //   - slope < -0.5 → 'down' (시간당 0.5%p 이상 하락)
    //   - 그 외       → 'stable' (안정적)
    // ════════════════════════════════════════════════════════
    $trend = 'stable';
    if (!empty($today_data) && count($today_data) >= 3) {
        // 최근 6개 시간대 데이터로 선형회귀, 1시간 단위 기울기 계산
        $lr = linearRegression(array_slice($today_data, -6), 1);
        if ($lr['slope'] > 0.5)      $trend = 'up';   // 상승 추세
        elseif ($lr['slope'] < -0.5) $trend = 'down'; // 하락 추세
    }

    // ── 최종 JSON 응답 출력 ──────────────────────────────────
    echo json_encode([
        'code'         => '00',
        'current_oee'  => $current_oee,   // 현재(최신) OEE
        'current_hour' => $current_hour,  // 현재 집계 시간
        'forecast'     => $forecast,      // 향후 4시간 예측 배열
        'trend'        => $trend,         // 트렌드 방향
        'method'       => 'exponential_smoothing + seasonal', // 사용 알고리즘
        'data_days'    => 30,             // 학습 데이터 기간 (일)
    ]);
} catch (PDOException $e) {
    // DB 오류 발생 시 code 99 와 오류 메시지 반환
    echo json_encode(['code' => '99', 'msg' => 'DB error: ' . $e->getMessage()]);
}
