<?php

/**
 * ============================================================
 * 파일명: ai_anomaly_2.php
 * 목  적: AI 이상 감지 API — Z-Score 기반 실시간 OEE 이상 탐지
 *
 * 동작 개요:
 *   1) 최근 30일 머신별 OEE/가용성/품질 기준 통계(평균·표준편차) 산출
 *   2) 당일(또는 지정 날짜 범위) 실제 값과 비교하여 Z-Score 계산
 *   3) |Z| >= 2.0 이고 기준 대비 낮은 방향이면 이상(anomaly)으로 판정
 *   4) 동일 라인에서 2대 이상 동시 이상 감지 시 연쇄 경보(cascade_alerts) 추가
 *
 * Method: GET
 * Params:
 *   factory_filter, line_filter, machine_filter (optional — 미입력 시 전체)
 *   date_range: today|yesterday|7d|30d (default: today)
 * Response JSON 구조:
 *   {
 *     code: '00' (성공) | '99' (DB 오류),
 *     anomalies: [
 *       {
 *         machine_idx, machine_no, line_name,
 *         severity: 'critical'(|Z|>=3) | 'warning'(|Z|>=2),
 *         anomaly_count, current_oee,
 *         details: [ { type, value, baseline, z_score, severity, message } ]
 *       }
 *     ],
 *     cascade_alerts: [ { line_name, machine_count, message } ],
 *     summary: { total, critical, warning, cascade_lines }
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
// 공장·라인·머신 필터 (빈 문자열이면 전체 조회)
$factory_filter = isset($_GET['factory_filter']) ? trim($_GET['factory_filter']) : '';
$line_filter    = isset($_GET['line_filter'])    ? trim($_GET['line_filter'])    : '';
$machine_filter = isset($_GET['machine_filter']) ? trim($_GET['machine_filter']) : '';
// 날짜 범위 파라미터 (기본값: today)
$date_range     = isset($_GET['date_range'])     ? trim($_GET['date_range'])     : 'today';

// ── date_range → SQL 날짜 조건 변환 (data_oee alias: do) ─────────
// 당일 데이터를 조회할 때 적용할 WHERE 절 생성
switch ($date_range) {
    case 'yesterday': // 어제 하루
        $actual_date_where = "do.work_date = DATE_SUB(CURDATE(), INTERVAL 1 DAY)";
        break;
    case '7d': // 최근 7일
        $actual_date_where = "do.work_date >= DATE_SUB(CURDATE(), INTERVAL 6 DAY)";
        break;
    case '30d': // 최근 30일
        $actual_date_where = "do.work_date >= DATE_SUB(CURDATE(), INTERVAL 29 DAY)";
        break;
    default: // today (기본) — 오늘 날짜만
        $actual_date_where = "do.work_date = CURDATE()";
}

// ── 동적 필터 조건 배열 구성 ────────────────────────────────────
$where_parts = []; // SQL WHERE 조건 문자열 목록
$params      = []; // PDO 바인딩 파라미터 목록

// 공장 필터가 있으면 조건 추가
if ($factory_filter !== '') {
    $where_parts[] = 'do.factory_idx = ?';
    $params[]      = $factory_filter;
}
// 라인 필터가 있으면 조건 추가
if ($line_filter !== '') {
    $where_parts[] = 'do.line_idx = ?';
    $params[]      = $line_filter;
}
// 머신 필터가 있으면 조건 추가
if ($machine_filter !== '') {
    $where_parts[] = 'do.machine_idx = ?';
    $params[]      = $machine_filter;
}

// 조건이 있으면 'AND ...' 형태로 결합, 없으면 빈 문자열
$where_sql = $where_parts ? 'AND ' . implode(' AND ', $where_parts) : '';

try {
    // ════════════════════════════════════════════════════════
    // STEP 1. 기준 통계 산출
    //   - 최근 30일(어제까지) 머신별 OEE·가용성·성능·품질의
    //     평균(mean)과 표준편차(std) 계산
    //   - HAVING COUNT(*) >= 3 : 데이터가 3일 이상인 머신만 포함
    //     (표준편차 산출에 최소 표본 수 필요)
    // ════════════════════════════════════════════════════════
    $sql_baseline = "
    SELECT
      do.machine_idx,
      im.machine_no,
      il.line_name,
      AVG(do.oee)              AS mean_oee,      -- OEE 평균
      STDDEV(do.oee)           AS std_oee,       -- OEE 표준편차
      AVG(do.availabilty_rate) AS mean_avail,    -- 가용성 평균
      STDDEV(do.availabilty_rate) AS std_avail,  -- 가용성 표준편차
      AVG(do.productivity_rate)  AS mean_perf,   -- 성능(생산성) 평균
      STDDEV(do.productivity_rate) AS std_perf,  -- 성능 표준편차
      AVG(do.quality_rate)     AS mean_quality,  -- 품질 평균
      STDDEV(do.quality_rate)  AS std_quality,   -- 품질 표준편차
      COUNT(*)           AS data_cnt             -- 데이터 건수
    FROM data_oee do
    JOIN info_machine im ON do.machine_idx = im.idx
    JOIN info_line    il ON do.line_idx = il.idx
    WHERE do.work_date BETWEEN DATE_SUB(CURDATE(), INTERVAL 30 DAY) AND DATE_SUB(CURDATE(), INTERVAL 1 DAY)
      $where_sql
    GROUP BY do.machine_idx, im.machine_no, il.line_name
    HAVING COUNT(*) >= 3
  ";
    $stmt = $pdo->prepare($sql_baseline);
    $stmt->execute($params);
    $baselines = $stmt->fetchAll(PDO::FETCH_ASSOC);

    // 기준 데이터가 없으면 빈 결과 반환
    if (empty($baselines)) {
        echo json_encode(['code' => '00', 'anomalies' => [], 'summary' => ['total' => 0, 'msg' => 'Not enough baseline data (need 3+ days)']]);
        exit;
    }

    // 기준 통계를 machine_idx 키로 맵 구성 (빠른 조회를 위한 해시맵)
    $baseline_map = [];
    foreach ($baselines as $b) {
        $baseline_map[$b['machine_idx']] = $b;
    }

    // ════════════════════════════════════════════════════════
    // STEP 2. 당일(또는 지정 범위) 실제 OEE 데이터 조회
    //   - date_range에 따른 날짜 조건 적용
    //   - 머신별로 shift_idx 내림차순 정렬 → 최신 교대 데이터 우선
    // ════════════════════════════════════════════════════════
    $params_today = $params; // 기준 통계와 동일한 필터 파라미터 재사용
    $sql_today = "
    SELECT
      do.machine_idx,
      do.line_idx,
      im.machine_no,
      il.line_name,
      do.oee,
      do.availabilty_rate  AS availability, -- 가용성 (컬럼명 alias)
      do.productivity_rate AS performance,  -- 성능
      do.quality_rate      AS quality,      -- 품질
      do.work_date,
      do.shift_idx
    FROM data_oee do
    JOIN info_machine im ON do.machine_idx = im.idx
    JOIN info_line    il ON do.line_idx = il.idx
    WHERE $actual_date_where
      $where_sql
    ORDER BY do.machine_idx, do.shift_idx DESC
  ";
    $stmt2 = $pdo->prepare($sql_today);
    $stmt2->execute($params_today);
    $today_data = $stmt2->fetchAll(PDO::FETCH_ASSOC);

    $anomalies    = []; // 이상 감지 결과 목록
    $seen_machines = []; // 머신 중복 처리 방지용 플래그

    // ── 머신별 이상 감지 루프 ────────────────────────────────
    foreach ($today_data as $row) {
        $mid = $row['machine_idx'];
        // 동일 머신이 여러 교대(shift)에 걸쳐 있으면 최신 1건만 처리
        if (isset($seen_machines[$mid])) continue; // 같은 머신 중복 제거 (최신 shift만)
        $seen_machines[$mid] = true;

        // 기준 통계가 없는 머신은 건너뜀
        if (!isset($baseline_map[$mid])) continue;
        $base = $baseline_map[$mid];

        $machine_anomalies = []; // 해당 머신의 이상 항목 목록

        // ── OEE 이상 감지 ────────────────────────────────────
        // zScoreSingle(현재값, 평균, 표준편차, 임계Z) → is_anomaly, direction, z_score 반환
        // std 최솟값 1.0 보정: 표준편차가 거의 0이면 항상 이상으로 오판되는 것 방지
        $oee_z = zScoreSingle((float)$row['oee'], (float)$base['mean_oee'], max(1.0, (float)$base['std_oee']), 2.0);
        // direction === 'low': 기준 평균보다 낮은 방향의 이상만 감지 (높은 건 양호로 간주)
        if ($oee_z['is_anomaly'] && $oee_z['direction'] === 'low') {
            $machine_anomalies[] = [
                'type'      => 'OEE',
                'value'     => round((float)$row['oee'], 1),
                'baseline'  => round((float)$base['mean_oee'], 1),
                'z_score'   => $oee_z['z_score'],
                // |Z| >= 3 이면 critical, 2~3 이면 warning
                'severity'  => abs($oee_z['z_score']) >= 3 ? 'critical' : 'warning',
                'message'   => sprintf('OEE %.1f%% (baseline avg %.1f%%)', $row['oee'], $base['mean_oee']),
            ];
        }

        // ── Availability(가용성) 이상 감지 ───────────────────
        $avail_z = zScoreSingle((float)$row['availability'], (float)$base['mean_avail'], max(1.0, (float)$base['std_avail']), 2.0);
        if ($avail_z['is_anomaly'] && $avail_z['direction'] === 'low') {
            $machine_anomalies[] = [
                'type'      => 'Availability',
                'value'     => round((float)$row['availability'], 1),
                'baseline'  => round((float)$base['mean_avail'], 1),
                'z_score'   => $avail_z['z_score'],
                'severity'  => abs($avail_z['z_score']) >= 3 ? 'critical' : 'warning',
                'message'   => sprintf('Availability %.1f%% (baseline avg %.1f%%)', $row['availability'], $base['mean_avail']),
            ];
        }

        // ── Quality(품질) 이상 감지 ──────────────────────────
        $qual_z = zScoreSingle((float)$row['quality'], (float)$base['mean_quality'], max(1.0, (float)$base['std_quality']), 2.0);
        if ($qual_z['is_anomaly'] && $qual_z['direction'] === 'low') {
            $machine_anomalies[] = [
                'type'      => 'Quality',
                'value'     => round((float)$row['quality'], 1),
                'baseline'  => round((float)$base['mean_quality'], 1),
                'z_score'   => $qual_z['z_score'],
                'severity'  => abs($qual_z['z_score']) >= 3 ? 'critical' : 'warning',
                'message'   => sprintf('Quality %.1f%% (baseline avg %.1f%%)', $row['quality'], $base['mean_quality']),
            ];
        }

        // 해당 머신에 이상 항목이 1건 이상이면 결과에 추가
        if (!empty($machine_anomalies)) {
            // 해당 머신의 최고 심각도 결정 (critical > warning)
            $max_severity = 'warning';
            foreach ($machine_anomalies as $a) {
                if ($a['severity'] === 'critical') {
                    $max_severity = 'critical';
                    break;
                }
            }

            $anomalies[] = [
                'machine_idx'  => $mid,
                'machine_no'   => $row['machine_no'],
                'line_name'    => $row['line_name'],
                'severity'     => $max_severity,       // 머신 대표 심각도
                'anomaly_count' => count($machine_anomalies), // 이상 항목 수
                'details'      => $machine_anomalies,  // 상세 이상 목록
                'current_oee'  => round((float)$row['oee'], 1),
            ];
        }
    }

    // ── 이상 머신 정렬: critical 우선, 같은 심각도 내에서는 이상 항목 수 많은 순 ──
    usort($anomalies, function ($a, $b) {
        if ($a['severity'] !== $b['severity']) {
            return $a['severity'] === 'critical' ? -1 : 1;
        }
        return $b['anomaly_count'] - $a['anomaly_count'];
    });

    // ════════════════════════════════════════════════════════
    // STEP 3. 연쇄 이상 감지 (Cascade Alert)
    //   - 동일 라인에서 2대 이상 동시 이상이 발생하면
    //     설비 공통 원인(전력, 원자재, 환경)을 시사하는 경보 생성
    // ════════════════════════════════════════════════════════
    $line_anomaly_counts = []; // 라인별 이상 머신 카운트
    foreach ($anomalies as $a) {
        $line_anomaly_counts[$a['line_name']] = ($line_anomaly_counts[$a['line_name']] ?? 0) + 1;
    }

    $cascade_alerts = []; // 연쇄 경보 목록
    foreach ($line_anomaly_counts as $line => $cnt) {
        // 2대 이상 동시 이상 발생 라인만 경보 생성
        if ($cnt >= 2) {
            $cascade_alerts[] = [
                'line_name'     => $line,
                'machine_count' => $cnt,
                'message'       => "Line {$line}: {$cnt} machines with simultaneous anomalies detected",
            ];
        }
    }

    // ── 심각도별 카운트 집계 ────────────────────────────────
    $critical_count = count(array_filter($anomalies, fn($a) => $a['severity'] === 'critical'));
    $warning_count  = count($anomalies) - $critical_count;

    // ── 최종 JSON 응답 출력 ──────────────────────────────────
    echo json_encode([
        'code'            => '00',
        'anomalies'       => $anomalies,       // 머신별 이상 감지 결과 배열
        'cascade_alerts'  => $cascade_alerts,  // 연쇄(동일 라인 복수 머신) 경보 배열
        'summary' => [
            'total'         => count($anomalies),   // 이상 머신 총 수
            'critical'      => $critical_count,     // critical 수
            'warning'       => $warning_count,      // warning 수
            'cascade_lines' => count($cascade_alerts), // 연쇄 경보 라인 수
        ],
    ]);
} catch (PDOException $e) {
    // DB 오류 발생 시 code 99 와 오류 메시지 반환
    echo json_encode(['code' => '99', 'msg' => 'DB error: ' . $e->getMessage()]);
}
