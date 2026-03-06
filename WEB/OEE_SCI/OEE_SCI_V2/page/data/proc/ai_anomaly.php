<?php
/**
 * AI Anomaly Detection API
 * Z-Score 기반 실시간 이상 감지
 *
 * Method: GET
 * Params:
 *   factory_filter, line_filter, machine_filter (optional)
 * Response:
 *   { code, anomalies: [...], summary: {...} }
 */

date_default_timezone_set('Asia/Jakarta');

require_once(__DIR__ . '/../../../lib/db.php');
require_once(__DIR__ . '/../../../lib/statistics.lib.php');

header('Content-Type: application/json');
header('Cache-Control: no-cache');

$factory_filter = isset($_GET['factory_filter']) ? trim($_GET['factory_filter']) : '';
$line_filter    = isset($_GET['line_filter'])    ? trim($_GET['line_filter'])    : '';
$machine_filter = isset($_GET['machine_filter']) ? trim($_GET['machine_filter']) : '';

$where_parts = [];
$params      = [];

if ($factory_filter !== '') {
  $where_parts[] = 'do.factory_idx = ?';
  $params[]      = $factory_filter;
}
if ($line_filter !== '') {
  $where_parts[] = 'do.line_idx = ?';
  $params[]      = $line_filter;
}
if ($machine_filter !== '') {
  $where_parts[] = 'do.machine_idx = ?';
  $params[]      = $machine_filter;
}

$where_sql = $where_parts ? 'AND ' . implode(' AND ', $where_parts) : '';

try {
  // 1. 기준 통계: 최근 30일 머신별 OEE 평균/표준편차
  $sql_baseline = "
    SELECT
      do.machine_idx,
      im.machine_no,
      il.line_name,
      AVG(do.oee)              AS mean_oee,
      STDDEV(do.oee)           AS std_oee,
      AVG(do.availabilty_rate) AS mean_avail,
      STDDEV(do.availabilty_rate) AS std_avail,
      AVG(do.productivity_rate)  AS mean_perf,
      STDDEV(do.productivity_rate) AS std_perf,
      AVG(do.quality_rate)     AS mean_quality,
      STDDEV(do.quality_rate)  AS std_quality,
      COUNT(*)           AS data_cnt
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

  if (empty($baselines)) {
    echo json_encode(['code' => '00', 'anomalies' => [], 'summary' => ['total' => 0, 'msg' => 'Not enough baseline data (need 3+ days)']]);
    exit;
  }

  // 기준 맵 구성
  $baseline_map = [];
  foreach ($baselines as $b) {
    $baseline_map[$b['machine_idx']] = $b;
  }

  // 2. 오늘 데이터 (현재 교대 기준)
  $params_today = $params;
  $sql_today = "
    SELECT
      do.machine_idx,
      do.line_idx,
      im.machine_no,
      il.line_name,
      do.oee,
      do.availabilty_rate  AS availability,
      do.productivity_rate AS performance,
      do.quality_rate      AS quality,
      do.work_date,
      do.shift_idx
    FROM data_oee do
    JOIN info_machine im ON do.machine_idx = im.idx
    JOIN info_line    il ON do.line_idx = il.idx
    WHERE do.work_date = CURDATE()
      $where_sql
    ORDER BY do.machine_idx, do.shift_idx DESC
  ";
  $stmt2 = $pdo->prepare($sql_today);
  $stmt2->execute($params_today);
  $today_data = $stmt2->fetchAll(PDO::FETCH_ASSOC);

  $anomalies = [];
  $seen_machines = [];

  foreach ($today_data as $row) {
    $mid = $row['machine_idx'];
    if (isset($seen_machines[$mid])) continue; // 같은 머신 중복 제거 (최신 shift만)
    $seen_machines[$mid] = true;

    if (!isset($baseline_map[$mid])) continue;
    $base = $baseline_map[$mid];

    $machine_anomalies = [];

    // OEE 이상 감지
    $oee_z = zScoreSingle((float)$row['oee'], (float)$base['mean_oee'], max(1.0, (float)$base['std_oee']), 2.0);
    if ($oee_z['is_anomaly'] && $oee_z['direction'] === 'low') {
      $machine_anomalies[] = [
        'type'      => 'OEE',
        'value'     => round((float)$row['oee'], 1),
        'baseline'  => round((float)$base['mean_oee'], 1),
        'z_score'   => $oee_z['z_score'],
        'severity'  => abs($oee_z['z_score']) >= 3 ? 'critical' : 'warning',
        'message'   => sprintf('OEE %.1f%% (baseline avg %.1f%%)', $row['oee'], $base['mean_oee']),
      ];
    }

    // Availability 이상 감지
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

    // Quality 이상 감지
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

    if (!empty($machine_anomalies)) {
      // 최고 심각도 결정
      $max_severity = 'warning';
      foreach ($machine_anomalies as $a) {
        if ($a['severity'] === 'critical') { $max_severity = 'critical'; break; }
      }

      $anomalies[] = [
        'machine_idx'  => $mid,
        'machine_no'   => $row['machine_no'],
        'line_name'    => $row['line_name'],
        'severity'     => $max_severity,
        'anomaly_count' => count($machine_anomalies),
        'details'      => $machine_anomalies,
        'current_oee'  => round((float)$row['oee'], 1),
      ];
    }
  }

  // 심각도 순 정렬 (critical 우선, 그 다음 Z-score 크기)
  usort($anomalies, function($a, $b) {
    if ($a['severity'] !== $b['severity']) {
      return $a['severity'] === 'critical' ? -1 : 1;
    }
    return $b['anomaly_count'] - $a['anomaly_count'];
  });

  // 3. 연쇄 이상 감지 (같은 라인에서 복수 머신 동시 이상)
  $line_anomaly_counts = [];
  foreach ($anomalies as $a) {
    $line_anomaly_counts[$a['line_name']] = ($line_anomaly_counts[$a['line_name']] ?? 0) + 1;
  }

  $cascade_alerts = [];
  foreach ($line_anomaly_counts as $line => $cnt) {
    if ($cnt >= 2) {
      $cascade_alerts[] = [
        'line_name'     => $line,
        'machine_count' => $cnt,
        'message'       => "Line {$line}: {$cnt} machines with simultaneous anomalies detected",
      ];
    }
  }

  $critical_count = count(array_filter($anomalies, fn($a) => $a['severity'] === 'critical'));
  $warning_count  = count($anomalies) - $critical_count;

  echo json_encode([
    'code'            => '00',
    'anomalies'       => $anomalies,
    'cascade_alerts'  => $cascade_alerts,
    'summary' => [
      'total'         => count($anomalies),
      'critical'      => $critical_count,
      'warning'       => $warning_count,
      'cascade_lines' => count($cascade_alerts),
    ],
  ]);

} catch (PDOException $e) {
  echo json_encode(['code' => '99', 'msg' => 'DB error: ' . $e->getMessage()]);
}
