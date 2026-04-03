<?php

/**
 * ============================================================
 * 파일명: ai_maintenance_dash_2.php
 * 목  적: AI 예측 정비 대시보드 API v5 (OEE 클램핑 적용 버전)
 *         머신별 다운타임 패턴 분석 → 위험도 스코어 산출
 *
 * v4 대비 수정 사항:
 *  - OEE 조회 시 LEAST(100, GREATEST(0, oee)) 클램핑 적용
 *    → 125.6% 등 비정상 DB 값이 표준편차를 왜곡하여 오판정하는 버그 수정
 *
 * 동작 개요:
 *   1) 등록된 머신 목록 조회 (필터 조건 적용)
 *   2) 최근 90일 다운타임 이력 집계 (빈도·총 시간·최근 발생일)
 *   3) 마지막 다운타임 이후 경과 시간 계산
 *   4) 최근 N일 OEE 트렌드 조회 (클램핑 적용)
 *   5) 세 가지 지표를 가중 합산하여 위험도 스코어(0~100) 산출
 *      - A(40%): 런타임 위험도 — 경과시간 / 평균 고장 간격
 *      - B(35%): 다운타임 빈도 증가율 — 최근 30일 vs 이전 30일
 *      - C(25%): OEE 불안정도 — 평균 OEE 낮을수록 + 변동성 클수록 높음
 *
 * Method: GET
 * Params:
 *   factory_filter, line_filter, machine_filter (optional)
 *   limit: 반환할 위험 기계 수 (default: 10, max: 50)
 *   date_range: today|yesterday|7d|30d (default: today)
 * Response JSON 구조:
 *   {
 *     code: '00' | '99',
 *     machines: [
 *       {
 *         machine_idx, machine_no, line_name,
 *         risk_score: 0~100,
 *         risk_level: 'danger'(>=80) | 'warning'(>=50) | 'normal',
 *         score_detail: { runtime, frequency, oee_quality },
 *         details: { runtime_ratio, avg_interval_hrs, hours_since_last_dt,
 *                    recent_30d_cnt, prev_30d_cnt,
 *                    avg_oee_7d, std_oee_7d, min_oee_7d }
 *       }
 *     ],
 *     summary: { total, danger, warning, normal }
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
// limit: 결과 최대 반환 수 (1~50 사이로 클램핑)
$limit          = isset($_GET['limit'])          ? max(1, min(50, (int)$_GET['limit'])) : 10;
$date_range     = isset($_GET['date_range'])     ? trim($_GET['date_range'])     : 'today';

// ── date_range → OEE 조회 날짜 조건 변환 ───────────────────────
// 이 날짜 조건은 OEE 트렌드 분석 기간에만 적용됨
switch ($date_range) {
    case 'yesterday': // 어제 하루
        $oee_date_where = "work_date = DATE_SUB(CURDATE(), INTERVAL 1 DAY)";
        break;
    case '7d': // 최근 7일
        $oee_date_where = "work_date >= DATE_SUB(CURDATE(), INTERVAL 6 DAY)";
        break;
    case '30d': // 최근 30일
        $oee_date_where = "work_date >= DATE_SUB(CURDATE(), INTERVAL 29 DAY)";
        break;
    default: // today → 실질적 통계 안정성을 위해 7일 기간으로 분석
        $oee_date_where = "work_date >= DATE_SUB(CURDATE(), INTERVAL 6 DAY)";
}

// ── info_machine 기반 필터 조건 구성 ────────────────────────────
// (data_oee와 달리 info_machine 테이블 컬럼 기준으로 필터링)
$where_parts = [];
$params      = [];

if ($factory_filter !== '') {
    $where_parts[] = 'im.factory_idx = ?'; // 공장 필터
    $params[] = $factory_filter;
}
if ($line_filter    !== '') {
    $where_parts[] = 'im.line_idx = ?';   // 라인 필터
    $params[] = $line_filter;
}
if ($machine_filter !== '') {
    $where_parts[] = 'im.idx = ?';        // 특정 머신 필터
    $params[] = $machine_filter;
}

// WHERE 절 조립 (있으면 'WHERE ...', 없으면 빈 문자열)
$where_sql = $where_parts ? 'WHERE ' . implode(' AND ', $where_parts) : '';

try {
    // ════════════════════════════════════════════════════════
    // STEP 1. 등록된 모든 기계 목록 조회
    //   - 필터 조건에 맞는 info_machine 레코드 반환
    //   - 이후 단계에서 machine_ids 배열로 IN 조회에 사용
    // ════════════════════════════════════════════════════════
    $sql_machines = "
    SELECT im.idx, im.machine_no, il.line_name, il.idx AS line_idx
    FROM info_machine im
    JOIN info_line il ON im.line_idx = il.idx
    $where_sql
    ORDER BY il.line_name, im.machine_no
  ";
    $stmt = $pdo->prepare($sql_machines);
    $stmt->execute($params);
    $machines = $stmt->fetchAll(PDO::FETCH_ASSOC);

    // 조회된 머신이 없으면 빈 결과 반환
    if (empty($machines)) {
        echo json_encode(['code' => '00', 'machines' => [], 'summary' => ['total' => 0]]);
        exit;
    }

    // 머신 ID 배열 및 빠른 조회용 해시맵 구성
    $machine_ids = array_column($machines, 'idx'); // [id1, id2, ...]
    $machine_map = [];
    foreach ($machines as $m) $machine_map[$m['idx']] = $m; // {id => 머신정보}

    // IN 절용 플레이스홀더 생성 (?, ?, ...)
    $placeholders = implode(',', array_fill(0, count($machine_ids), '?'));

    // ════════════════════════════════════════════════════════
    // STEP 2. 최근 90일 머신별 다운타임 이력 집계
    //   - total_dt_count : 총 다운타임 건수
    //   - total_dt_min   : 총 다운타임 시간(분)
    //   - avg_dt_min     : 평균 다운타임 시간(분)
    //   - last_dt_time   : 마지막 다운타임 발생 일시
    //   - recent_7d_cnt  : 최근 7일 다운타임 건수
    //   - recent_30d_cnt : 최근 30일 다운타임 건수 (비교 기준)
    //   - prev_30d_cnt   : 이전 30일(31~60일 전) 다운타임 건수 (증가율 비교용)
    // ════════════════════════════════════════════════════════
    // data_downtime 은 독립 테이블 — machine_idx, work_date, reg_date, duration_sec 직접 보유
    $sql_downtime = "
    SELECT
      dd.machine_idx,
      COUNT(*)                         AS total_dt_count,
      SUM(dd.duration_sec / 60)        AS total_dt_min,
      AVG(dd.duration_sec / 60)        AS avg_dt_min,
      MAX(dd.reg_date)                 AS last_dt_time,
      SUM(CASE WHEN dd.work_date >= DATE_SUB(CURDATE(), INTERVAL 7 DAY) THEN 1 ELSE 0 END)  AS recent_7d_cnt,
      SUM(CASE WHEN dd.work_date >= DATE_SUB(CURDATE(), INTERVAL 30 DAY) THEN 1 ELSE 0 END) AS recent_30d_cnt,
      SUM(CASE WHEN dd.work_date BETWEEN DATE_SUB(CURDATE(), INTERVAL 60 DAY)
                                     AND DATE_SUB(CURDATE(), INTERVAL 31 DAY) THEN 1 ELSE 0 END) AS prev_30d_cnt
    FROM data_downtime dd
    WHERE dd.machine_idx IN ($placeholders)
      AND dd.work_date >= DATE_SUB(CURDATE(), INTERVAL 90 DAY)
    GROUP BY dd.machine_idx
  ";
    $stmt2 = $pdo->prepare($sql_downtime);
    $stmt2->execute($machine_ids); // machine_ids 배열을 바인딩 파라미터로 전달
    $dt_stats = $stmt2->fetchAll(PDO::FETCH_ASSOC);

    // machine_idx 기준 해시맵으로 변환
    $dt_map = [];
    foreach ($dt_stats as $dt) $dt_map[$dt['machine_idx']] = $dt;

    // ════════════════════════════════════════════════════════
    // STEP 3. 마지막 다운타임 이후 경과 시간 계산
    //   - TIMESTAMPDIFF(HOUR, ...) 로 현재 시각 기준 경과 시간(시) 산출
    //   - 이 값이 평균 고장 간격에 근접할수록 위험도(score_a) 상승
    // ════════════════════════════════════════════════════════
    $sql_runtime = "
    SELECT
      dd.machine_idx,
      MAX(dd.reg_date) AS last_dt_time,
      TIMESTAMPDIFF(HOUR, MAX(dd.reg_date), NOW()) AS hours_since_last_dt
    FROM data_downtime dd
    WHERE dd.machine_idx IN ($placeholders)
    GROUP BY dd.machine_idx
  ";
    $stmt3 = $pdo->prepare($sql_runtime);
    $stmt3->execute($machine_ids);
    $runtime_data = $stmt3->fetchAll(PDO::FETCH_ASSOC);

    // machine_idx 기준 해시맵으로 변환
    $runtime_map = [];
    foreach ($runtime_data as $r) $runtime_map[$r['machine_idx']] = $r;

    // ════════════════════════════════════════════════════════
    // STEP 4. OEE 트렌드 조회 (v5: 비정상 OEE 클램핑 적용)
    //   - LEAST(100, GREATEST(0, oee)) : OEE 값을 0~100 범위로 강제 제한
    //     → DB에 125.6% 같은 비정상 값이 있어도 표준편차 왜곡 방지
    //   - avg_oee_7d : 기간 평균 OEE (클램핑 후)
    //   - min_oee_7d : 기간 최솟값 (극단 저하 감지)
    //   - std_oee_7d : 표준편차 (불안정도 측정)
    // ════════════════════════════════════════════════════════
    // OEE 트렌드 — v5: LEAST(100, GREATEST(0, oee)) 클램핑으로 비정상 값 제거
    $sql_oee = "
    SELECT
      machine_idx,
      AVG(LEAST(100, GREATEST(0, oee)))    AS avg_oee_7d,
      MIN(LEAST(100, GREATEST(0, oee)))    AS min_oee_7d,
      STDDEV(LEAST(100, GREATEST(0, oee))) AS std_oee_7d
    FROM data_oee
    WHERE machine_idx IN ($placeholders)
      AND $oee_date_where
    GROUP BY machine_idx
  ";
    $stmt4 = $pdo->prepare($sql_oee);
    $stmt4->execute($machine_ids);
    $oee_stats = $stmt4->fetchAll(PDO::FETCH_ASSOC);

    // machine_idx 기준 해시맵으로 변환
    $oee_map = [];
    foreach ($oee_stats as $o) $oee_map[$o['machine_idx']] = $o;

    // ════════════════════════════════════════════════════════
    // STEP 5. 머신별 위험도 스코어 계산
    //   최종 위험도 = score_a * 0.40 + score_b * 0.35 + score_c * 0.25
    // ════════════════════════════════════════════════════════
    $results = [];
    foreach ($machines as $m) {
        $mid = $m['idx'];
        // 각 통계 데이터 (없으면 null — 해당 가중치 0점 처리)
        $dt  = $dt_map[$mid]     ?? null;
        $rt  = $runtime_map[$mid] ?? null;
        $oe  = $oee_map[$mid]    ?? null;

        $score_a = 0; // 가동 시간 위험도 (가중치 40%)
        $score_b = 0; // 다운타임 빈도 증가율 (가중치 35%)
        $score_c = 0; // OEE 불안정도 (가중치 25%)
        $details = []; // 상세 정보 (디버깅·UI 표시용)

        // ── 가중치 A: 런타임 위험도 (40%) ────────────────────
        // 마지막 고장 이후 경과 시간이 평균 고장 간격에 대비 몇 % 인지 계산
        // ratio = 1.0이면 이론적으로 다음 고장 시점 = score 100
        if ($dt && $rt) {
            $avg_interval_hrs = 0;
            if ($dt['total_dt_count'] > 1) {
                // 90일 / 총 다운타임 건수 = 평균 고장 간격(일) → 시간 변환
                $avg_interval_hrs = (90 * 24) / (int)$dt['total_dt_count'];
            }
            $hours_since = (float)($rt['hours_since_last_dt'] ?? 0);
            if ($avg_interval_hrs > 0) {
                $ratio   = $hours_since / $avg_interval_hrs; // 경과 비율
                $score_a = min(100, $ratio * 100);           // 100점 상한
                // 상세 정보 저장
                $details['runtime_ratio']       = round($ratio, 2);
                $details['avg_interval_hrs']    = round($avg_interval_hrs, 1);
                $details['hours_since_last_dt'] = round($hours_since, 1);
            }
        }

        // ── 가중치 B: 다운타임 빈도 증가율 (35%) ─────────────
        // 최근 30일 vs 이전 30일 다운타임 건수 비율로 증가 추세 감지
        if ($dt) {
            $recent = (int)($dt['recent_30d_cnt'] ?? 0); // 최근 30일 건수
            $prev   = (int)($dt['prev_30d_cnt']   ?? 0); // 이전 30일 건수
            if ($prev > 0) {
                // 증가율 계산: (최근 - 이전) / 이전
                $increase_rate = ($recent - $prev) / $prev;
                // 0~100 사이로 클램핑 (감소 추세는 0, 급증은 100)
                $score_b = min(100, max(0, $increase_rate * 100));
            } elseif ($recent > 0) {
                // 이전 기간 없고 최근에만 있으면 중간 위험 (50점 부여)
                $score_b = 50;
            }
            $details['recent_30d_cnt'] = $recent;
            $details['prev_30d_cnt']   = $prev;
        }

        // ── 가중치 C: OEE 불안정도 (25%) — 클램핑된 값 사용 ─
        // 평균 OEE가 낮을수록 + 표준편차가 클수록 위험도 상승
        if ($oe) {
            $avg_oee = (float)($oe['avg_oee_7d'] ?? 85); // 기간 평균 OEE (없으면 85% 기본)
            $std_oee = (float)($oe['std_oee_7d'] ?? 0);  // OEE 표준편차
            $min_oee = (float)($oe['min_oee_7d'] ?? 85); // 기간 최소 OEE

            // OEE 낮을수록 위험: 85%를 기준으로 미달 비율 (0~100)
            $oee_risk   = max(0, (85 - $avg_oee) / 85 * 100);
            // 변동성 위험: std * 3 (std 33%p = 100점)
            $stdev_risk = min(100, $std_oee * 3);
            // 복합 OEE 불안정 스코어 (60% OEE 레벨 + 40% 변동성)
            $score_c    = min(100, ($oee_risk * 0.6 + $stdev_risk * 0.4));

            $details['avg_oee_7d'] = round($avg_oee, 1);
            $details['std_oee_7d'] = round($std_oee, 1);
            $details['min_oee_7d'] = round($min_oee, 1);
        }

        // ── 세 가중치 합산 → 최종 위험도 스코어 ────────────────
        // 공식: risk_score = A*0.40 + B*0.35 + C*0.25
        $risk_score = round($score_a * 0.40 + $score_b * 0.35 + $score_c * 0.25, 1);

        // 위험 레벨 분류 (80점 이상 danger, 50점 이상 warning, 그 외 normal)
        $risk_level = 'normal';
        if ($risk_score >= 80)     $risk_level = 'danger';
        elseif ($risk_score >= 50) $risk_level = 'warning';

        $results[] = [
            'machine_idx'  => $mid,
            'machine_no'   => $m['machine_no'],
            'line_name'    => $m['line_name'],
            'risk_score'   => $risk_score,   // 최종 위험도 (0~100)
            'risk_level'   => $risk_level,   // 위험 레벨
            'score_detail' => [
                'runtime'    => round($score_a, 1),  // A 항목 점수
                'frequency'  => round($score_b, 1),  // B 항목 점수
                'oee_quality' => round($score_c, 1), // C 항목 점수
            ],
            'details' => $details,
        ];
    }

    // ── 위험도 내림차순 정렬 (가장 위험한 머신 먼저) ─────────
    usort($results, fn($a, $b) => $b['risk_score'] <=> $a['risk_score']);

    // ── 위험 레벨별 카운트 집계 ──────────────────────────────
    $danger_count  = count(array_filter($results, fn($r) => $r['risk_level'] === 'danger'));
    $warning_count = count(array_filter($results, fn($r) => $r['risk_level'] === 'warning'));

    // ── 최종 JSON 응답 출력 ──────────────────────────────────
    echo json_encode([
        'code'     => '00',
        'machines' => array_slice($results, 0, $limit), // limit 개수만큼 상위 반환
        'summary'  => [
            'total'   => count($results),      // 전체 머신 수
            'danger'  => $danger_count,        // danger 레벨 머신 수
            'warning' => $warning_count,       // warning 레벨 머신 수
            'normal'  => count($results) - $danger_count - $warning_count, // normal 머신 수
        ],
    ]);
} catch (PDOException $e) {
    // DB 오류 발생 시 code 99 와 오류 메시지 반환
    echo json_encode(['code' => '99', 'msg' => 'DB error: ' . $e->getMessage()]);
}
