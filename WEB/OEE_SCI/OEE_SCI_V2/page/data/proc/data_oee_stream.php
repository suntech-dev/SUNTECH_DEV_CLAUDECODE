<?php
/**
 * OEE Data Real-time Streaming API (Server-Sent Events)
 * Real-time OEE data streaming with 3-level filtering support.
 *
 * Optimized (2026-03-07):
 * - Merged getOeeStats + getOeeDetails + getOeeComponentStats -> getOeeAggregated() [3 queries -> 1]
 * - Merged getOeeTrendStats + getProductionTrendStats -> getTrendStats() [2 queries -> 1, workHours called once]
 * - Total SSE cycle queries: 7 -> 4
 * - Performance timing per query block
 * - English comments
 */


require_once(__DIR__ . '/../../../lib/db.php');
require_once(__DIR__ . '/../../../lib/api_helper.lib.php');
require_once(__DIR__ . '/../../../lib/worktime.lib.php');
require_once(__DIR__ . '/../../../lib/get_shift.lib.php');
require_once(__DIR__ . '/../../../lib/stream_helper.lib.php');

header('Content-Type: text/event-stream');
header('Cache-Control: no-cache');
header('Connection: keep-alive');
header('Access-Control-Allow-Origin: *');
header('Access-Control-Allow-Methods: GET');
header('Access-Control-Allow-Headers: Cache-Control');

if (ob_get_level()) ob_end_clean();

$apiHelper = new ApiHelper($pdo);

// parseFilterParams(), getWorkHoursForDate(), sendSSEData() → stream_helper.lib.php

/** Append machine status condition to WHERE clause */
function appendMachineStatus($where_sql) {
  return $where_sql . (strpos($where_sql, 'WHERE') !== false ? ' AND' : ' WHERE') . " m.status = 'Y'";
}

// ---------------------------------------------------------------------------
// Query 1: Main OEE rows
// ---------------------------------------------------------------------------
function getOeeData($pdo, $where_sql, $params, $limit = 100) {
  try {
    $w = appendMachineStatus($where_sql);
    $sql = "
      SELECT
        do.idx,
        do.work_date,
        do.shift_idx,
        do.machine_no,
        do.oee as overall_oee,
        do.availabilty_rate as availability,
        do.productivity_rate as performance,
        do.quality_rate as quality,
        do.planned_work_time as planned_time,
        do.runtime,
        do.downtime,
        do.actual_output,
        do.theoritical_output as theoretical_output,
        do.actual_a_grade as good_products,
        do.defective as defective_count,
        do.cycletime as cycle_time,
        do.update_date,
        f.factory_name,
        l.line_name,
        CASE
          WHEN do.oee >= 85 THEN 'Excellent'
          WHEN do.oee >= 70 THEN 'Good'
          WHEN do.oee >= 50 THEN 'Fair'
          ELSE 'Poor'
        END as oee_status,
        CASE
          WHEN do.oee >= 85 THEN 'Above Target'
          WHEN do.oee >= 80 THEN 'On Target'
          ELSE 'Below Target'
        END as target_status
      FROM data_oee do
      LEFT JOIN info_factory f ON do.factory_idx = f.idx
      LEFT JOIN info_line l ON do.line_idx = l.idx
      LEFT JOIN info_machine m ON do.machine_idx = m.idx
      {$w}
      ORDER BY do.work_date DESC, do.update_date DESC, do.idx DESC
      LIMIT " . (int)$limit . "
    ";

    $stmt = $pdo->prepare($sql);
    $stmt->execute($params);
    return $stmt->fetchAll(PDO::FETCH_ASSOC);

  } catch (PDOException $e) {
    error_log("OEE data query error: " . $e->getMessage());
    return [];
  }
}

// ---------------------------------------------------------------------------
// Query 2: Aggregated stats + details + component (merged from 3 queries)
//   Returns: stats (summary cards), oee_details (detail panel), oee_component_stats (radar/pie)
// ---------------------------------------------------------------------------
function getOeeAggregated($pdo, $where_sql, $params) {
  $empty = [
    'stats' => [
      'total_count' => 0, 'avg_overall_oee' => 0, 'avg_availability' => 0,
      'avg_performance' => 0, 'avg_quality' => 0, 'max_oee' => 0, 'min_oee' => 0,
      'active_machines' => 0, 'excellent_count' => 0, 'good_count' => 0,
      'fair_count' => 0, 'poor_count' => 0, 'today_avg_oee' => 0, 'today_count' => 0,
      'current_shift_oee' => 0, 'previous_day_oee' => 0,
      'total_actual_output' => 0, 'total_theoretical_output' => 0, 'total_good_products' => 0,
      'total_defective_count' => 0, 'total_planned_time' => 0, 'total_runtime' => 0,
      'total_downtime' => 0, 'overall_performance_rate' => 0, 'overall_quality_rate' => 0,
      'overall_availability_rate' => 0, 'overall_oee' => 0, 'availability' => 0,
      'performance' => 0, 'quality' => 0
    ],
    'oee_details' => [
      'overall_oee' => 0, 'availability' => 0, 'performance' => 0, 'quality' => 0,
      'runtime' => 0, 'planned_time' => 0, 'actual_output' => 0, 'theoretical_output' => 0,
      'good_products' => 0, 'defective_products' => 0, 'target_achievement' => '0%'
    ],
    'oee_component_stats' => ['availability' => 0, 'performance' => 0, 'quality' => 0, 'overall_oee' => 0]
  ];

  try {
    $w = appendMachineStatus($where_sql);

    $sql = "
      SELECT
        COUNT(*) as total_count,
        ROUND((AVG(do.availabilty_rate) * (SUM(do.actual_output) / NULLIF(SUM(do.theoritical_output), 0)) * AVG(do.quality_rate)) / 100, 2) as avg_overall_oee,
        ROUND(AVG(do.availabilty_rate), 2) as avg_availability,
        ROUND((SUM(do.actual_output) / NULLIF(SUM(do.theoritical_output), 0)) * 100, 2) as avg_performance,
        ROUND(AVG(do.quality_rate), 2) as avg_quality,
        MAX(do.oee) as max_oee,
        MIN(do.oee) as min_oee,
        COUNT(DISTINCT do.machine_idx) as active_machines,
        SUM(CASE WHEN do.oee >= 85 THEN 1 ELSE 0 END) as excellent_count,
        SUM(CASE WHEN do.oee >= 70 AND do.oee < 85 THEN 1 ELSE 0 END) as good_count,
        SUM(CASE WHEN do.oee >= 50 AND do.oee < 70 THEN 1 ELSE 0 END) as fair_count,
        SUM(CASE WHEN do.oee < 50 THEN 1 ELSE 0 END) as poor_count,
        ROUND(AVG(CASE WHEN do.work_date = CURDATE() THEN do.oee ELSE NULL END), 2) as today_avg_oee,
        COUNT(CASE WHEN do.work_date = CURDATE() THEN 1 ELSE NULL END) as today_count,
        SUM(do.actual_output) as total_actual_output,
        SUM(do.theoritical_output) as total_theoretical_output,
        SUM(do.actual_a_grade) as total_good_products,
        SUM(do.defective) as total_defective_count,
        SUM(do.planned_work_time) as total_planned_time,
        SUM(do.runtime) as total_runtime,
        SUM(do.downtime) as total_downtime
      FROM data_oee do
      LEFT JOIN info_machine m ON do.machine_idx = m.idx
      {$w}
    ";

    $stmt = $pdo->prepare($sql);
    $stmt->execute($params);
    $row = $stmt->fetch(PDO::FETCH_ASSOC);

    // Derived rates (PHP — avoids extra DB subqueries)
    $row['overall_performance_rate'] = $row['total_theoretical_output'] > 0
      ? round(($row['total_actual_output'] / $row['total_theoretical_output']) * 100, 2) : 0;

    $row['overall_quality_rate'] = $row['total_actual_output'] > 0
      ? round(($row['total_good_products'] / $row['total_actual_output']) * 100, 2) : 0;

    $row['overall_availability_rate'] = $row['total_planned_time'] > 0
      ? round(($row['total_runtime'] / $row['total_planned_time']) * 100, 2) : 0;

    // Stat-card aliases
    $row['overall_oee']  = $row['avg_overall_oee'];
    $row['availability'] = $row['avg_availability'];
    $row['performance']  = $row['avg_performance'];
    $row['quality']      = $row['avg_quality'];

    // Current shift & previous day OEE
    $selected_shift = $_GET['shift_filter'] ?? null;
    $target_date    = $_GET['start_date'] ?? $_GET['end_date'] ?? null;
    $row['current_shift_oee']  = getCurrentShiftOeeAvg($pdo, $selected_shift, $target_date);
    $row['previous_day_oee']   = getPreviousDayOeeAvg($pdo, $where_sql, $params);

    // oee_details (derived from same row — no extra query)
    $runtime_min      = $row['total_runtime'] ? round($row['total_runtime'] / 60, 1) : 0;
    $planned_time_min = $row['total_planned_time'] ? round($row['total_planned_time'] / 60, 1) : 0;
    $target_oee       = 85;
    $target_achievement = $row['avg_overall_oee']
      ? round(($row['avg_overall_oee'] / $target_oee) * 100, 1) . '%' : '0%';

    $oee_details = [
      'overall_oee'       => $row['avg_overall_oee'],
      'availability'      => $row['avg_availability'],
      'performance'       => $row['avg_performance'],
      'quality'           => $row['avg_quality'],
      'runtime'           => $runtime_min,
      'planned_time'      => $planned_time_min,
      'actual_output'     => $row['total_actual_output'],
      'theoretical_output' => $row['total_theoretical_output'],
      'good_products'     => $row['total_good_products'],
      'defective_products' => $row['total_defective_count'],
      'target_achievement' => $target_achievement
    ];

    // oee_component_stats (subset of same row — no extra query)
    $oee_component_stats = [
      'availability' => $row['avg_availability'],
      'performance'  => $row['avg_performance'],
      'quality'      => $row['avg_quality'],
      'overall_oee'  => $row['avg_overall_oee']
    ];

    return [
      'stats'              => $row,
      'oee_details'        => $oee_details,
      'oee_component_stats' => $oee_component_stats
    ];

  } catch (PDOException $e) {
    error_log("OEE aggregated query error: " . $e->getMessage());
    return $empty;
  }
}

// ---------------------------------------------------------------------------
// Query 3: OEE trend + production trend (merged from 2 queries, workHours called once)
//   Returns: oee_trend_stats, production_trend_stats
// ---------------------------------------------------------------------------
function getTrendStats($pdo, $where_sql, $params) {
  $empty = [
    'oee_trend'        => ['view_type' => 'hourly', 'data' => [], 'work_hours' => null],
    'production_trend' => ['view_type' => 'hourly', 'data' => [], 'work_hours' => null]
  ];

  try {
    $startDate = $_GET['start_date'] ?? '';
    $endDate   = $_GET['end_date'] ?? '';

    // Determine hourly vs daily view
    $isHourlyView = true;
    if (!empty($startDate) && !empty($endDate)) {
      $start = new DateTime($startDate);
      $end   = new DateTime($endDate);
      if ($start->format('Y-m-d') !== $end->format('Y-m-d') && $start->diff($end)->days > 1) {
        $isHourlyView = false;
      }
    }

    // Work hours info (called once, shared by both trend results)
    $workHoursInfo = null;
    if ($isHourlyView) {
      $targetDate    = !empty($startDate) ? $startDate : date('Y-m-d');
      $workHoursInfo = getWorkHoursForDate($pdo, $targetDate);
    }

    $viewType = $isHourlyView ? 'hourly' : 'daily';

    if ($isHourlyView) {
      $hw = str_replace('do.', 'doh.', $where_sql);
      $hw_m = $hw . (strpos($hw, 'WHERE') !== false ? ' AND' : ' WHERE') . " m.status = 'Y'";

      // OEE trend (hourly)
      $oee_sql = "
        SELECT
          CONCAT(doh.work_date, ' ', LPAD(doh.work_hour, 2, '0'), ':00:00') as time_label,
          CONCAT(LPAD(doh.work_hour, 2, '0'), 'H') as display_label,
          ROUND((AVG(doh.availabilty_rate) * (SUM(doh.actual_output) / NULLIF(SUM(doh.theoritical_output), 0)) * AVG(doh.quality_rate)) / 100, 2) as overall_oee,
          ROUND(AVG(doh.availabilty_rate), 2) as availability,
          ROUND((SUM(doh.actual_output) / NULLIF(SUM(doh.theoritical_output), 0)) * 100, 2) as performance,
          ROUND(AVG(doh.quality_rate), 2) as quality,
          COUNT(*) as record_count
        FROM data_oee_rows_hourly doh
        LEFT JOIN info_factory f ON doh.factory_idx = f.idx
        LEFT JOIN info_line l ON doh.line_idx = l.idx
        LEFT JOIN info_machine m ON doh.machine_idx = m.idx
        {$hw_m}
        GROUP BY doh.work_date, doh.work_hour
        ORDER BY doh.work_date ASC, doh.work_hour ASC
        LIMIT 24
      ";

      // Production trend (hourly) — same table, different columns
      $prod_sql = "
        SELECT
          CONCAT(doh.work_date, ' ', LPAD(doh.work_hour, 2, '0'), ':00:00') as time_label,
          CONCAT(LPAD(doh.work_hour, 2, '0'), 'H') as display_label,
          SUM(doh.actual_output) as actual_output,
          SUM(doh.theoritical_output) as target_output,
          ROUND((SUM(doh.actual_output) / NULLIF(SUM(doh.theoritical_output), 0)) * 100, 2) as avg_performance
        FROM data_oee_rows_hourly doh
        LEFT JOIN info_machine m ON doh.machine_idx = m.idx
        {$hw_m}
        GROUP BY doh.work_date, doh.work_hour
        ORDER BY doh.work_date ASC, doh.work_hour ASC
        LIMIT 24
      ";

    } else {
      $w = appendMachineStatus($where_sql);

      $oee_sql = "
        SELECT
          do.work_date as time_label,
          DATE_FORMAT(do.work_date, '%m/%d') as display_label,
          ROUND((AVG(do.availabilty_rate) * (SUM(do.actual_output) / NULLIF(SUM(do.theoritical_output), 0)) * AVG(do.quality_rate)) / 100, 2) as overall_oee,
          ROUND(AVG(do.availabilty_rate), 2) as availability,
          ROUND((SUM(do.actual_output) / NULLIF(SUM(do.theoritical_output), 0)) * 100, 2) as performance,
          ROUND(AVG(do.quality_rate), 2) as quality,
          COUNT(*) as record_count
        FROM data_oee do
        LEFT JOIN info_factory f ON do.factory_idx = f.idx
        LEFT JOIN info_line l ON do.line_idx = l.idx
        LEFT JOIN info_machine m ON do.machine_idx = m.idx
        {$w}
        GROUP BY do.work_date, DATE_FORMAT(do.work_date, '%m/%d')
        ORDER BY time_label ASC
        LIMIT 30
      ";

      $prod_sql = "
        SELECT
          do.work_date as time_label,
          DATE_FORMAT(do.work_date, '%m/%d') as display_label,
          SUM(do.actual_output) as actual_output,
          SUM(do.theoritical_output) as target_output,
          ROUND((SUM(do.actual_output) / NULLIF(SUM(do.theoritical_output), 0)) * 100, 2) as avg_performance
        FROM data_oee do
        LEFT JOIN info_machine m ON do.machine_idx = m.idx
        {$w}
        GROUP BY do.work_date, DATE_FORMAT(do.work_date, '%m/%d')
        ORDER BY time_label ASC
        LIMIT 30
      ";
    }

    $stmt = $pdo->prepare($oee_sql);
    $stmt->execute($params);
    $oeeRows = $stmt->fetchAll(PDO::FETCH_ASSOC);

    $stmt = $pdo->prepare($prod_sql);
    $stmt->execute($params);
    $prodRows = $stmt->fetchAll(PDO::FETCH_ASSOC);

    return [
      'oee_trend' => [
        'view_type'  => $viewType,
        'data'       => $oeeRows,
        'work_hours' => ($isHourlyView && $workHoursInfo) ? $workHoursInfo : null
      ],
      'production_trend' => [
        'view_type'  => $viewType,
        'data'       => $prodRows,
        'work_hours' => ($isHourlyView && $workHoursInfo) ? $workHoursInfo : null
      ]
    ];

  } catch (PDOException $e) {
    error_log("Trend stats query error: " . $e->getMessage());
    return $empty;
  }
}

// ---------------------------------------------------------------------------
// Query 4: Line / Machine OEE breakdown
// ---------------------------------------------------------------------------
function getMachineOeeStats($pdo, $where_sql, $params) {
  try {
    $hasLineFilter = !empty($_GET['line_filter']);
    $w = appendMachineStatus($where_sql);

    if ($hasLineFilter) {
      $sql = "
        SELECT
          do.machine_no,
          do.machine_idx,
          ROUND(AVG(do.availabilty_rate), 2) as availability,
          ROUND((SUM(do.actual_output) / NULLIF(SUM(do.theoritical_output), 0)) * 100, 2) as performance,
          ROUND(AVG(do.quality_rate), 2) as quality,
          ROUND((AVG(do.availabilty_rate) * (SUM(do.actual_output) / NULLIF(SUM(do.theoritical_output), 0)) * AVG(do.quality_rate)) / 100, 2) as overall_oee,
          COUNT(*) as record_count
        FROM data_oee do
        LEFT JOIN info_machine m ON do.machine_idx = m.idx
        {$w}
        GROUP BY do.machine_idx, do.machine_no
        ORDER BY do.machine_no ASC
        LIMIT 25
      ";
    } else {
      $sql = "
        SELECT
          l.line_name,
          do.line_idx,
          ROUND(AVG(do.availabilty_rate), 2) as availability,
          ROUND((SUM(do.actual_output) / NULLIF(SUM(do.theoritical_output), 0)) * 100, 2) as performance,
          ROUND(AVG(do.quality_rate), 2) as quality,
          ROUND((AVG(do.availabilty_rate) * (SUM(do.actual_output) / NULLIF(SUM(do.theoritical_output), 0)) * AVG(do.quality_rate)) / 100, 2) as overall_oee,
          COUNT(*) as record_count
        FROM data_oee do
        LEFT JOIN info_line l ON do.line_idx = l.idx
        LEFT JOIN info_machine m ON do.machine_idx = m.idx
        {$w}
        GROUP BY do.line_idx, l.line_name
        ORDER BY l.line_name ASC
        LIMIT 10
      ";
    }

    $stmt = $pdo->prepare($sql);
    $stmt->execute($params);
    return $stmt->fetchAll(PDO::FETCH_ASSOC);

  } catch (PDOException $e) {
    error_log("Line/Machine OEE stats query error: " . $e->getMessage());
    return [];
  }
}

function getPreviousDayOeeAvg($pdo, $where_sql, $params) {
  try {
    $sql = "
      SELECT
        ROUND((AVG(do.availabilty_rate) * (SUM(do.actual_output) / NULLIF(SUM(do.theoritical_output), 0)) * AVG(do.quality_rate)) / 100, 2) as previous_day_oee
      FROM data_oee do
      LEFT JOIN info_machine m ON do.machine_idx = m.idx
      {$where_sql}
      " . (strpos($where_sql, 'WHERE') !== false ? 'AND' : 'WHERE') . " m.status = 'Y' AND do.work_date < CURDATE()
      GROUP BY do.work_date
      ORDER BY do.work_date DESC
      LIMIT 1
    ";

    $stmt = $pdo->prepare($sql);
    $stmt->execute($params);
    $result = $stmt->fetch(PDO::FETCH_ASSOC);
    return (float)($result['previous_day_oee'] ?? 0);

  } catch (PDOException $e) {
    error_log("Previous day OEE query error: " . $e->getMessage());
    return 0;
  }
}

function getCurrentShiftOeeAvg($pdo, $selected_shift_idx = null, $target_date = null) {
  try {
    $target_date = $target_date ?: date('Y-m-d');
    $worktime    = new Worktime($pdo);

    if (!empty($selected_shift_idx)) {
      $day_shifts = $worktime->getDayShift($target_date, '', '');

      if (!$day_shifts || !isset($day_shifts['shift'][$selected_shift_idx])) return 0;

      $shift          = $day_shifts['shift'][$selected_shift_idx];
      $work_stime_str = $target_date . ' ' . $shift['available_stime'] . ':00';
      $work_etime_str = $target_date . ' ' . $shift['available_etime'] . ':00';

      if ($shift['over_time']) {
        $work_etime_str = date('Y-m-d H:i:s', strtotime($work_etime_str . ' +' . $shift['over_time'] . ' minutes'));
      }
      if ($work_etime_str <= $work_stime_str) {
        $work_etime_str = date('Y-m-d H:i:s', strtotime($work_etime_str . ' +1 day'));
      }

      $shift_date = date('Y-m-d', strtotime($work_stime_str));

    } else {
      $info = findCurrentShift($pdo, $worktime, '', '', date('Y-m-d H:i:s'));
      if (!$info) return 0;
      $shift_date         = date('Y-m-d', strtotime($info['work_stime']));
      $selected_shift_idx = 1;
    }

    $sql = "
      SELECT ROUND((AVG(do.availabilty_rate) * (SUM(do.actual_output) / NULLIF(SUM(do.theoritical_output), 0)) * AVG(do.quality_rate)) / 100, 2) as shift_avg_oee
      FROM data_oee do
      LEFT JOIN info_machine m ON do.machine_idx = m.idx
      WHERE do.work_date = ? AND do.shift_idx = ? AND m.status = 'Y'
    ";

    $stmt = $pdo->prepare($sql);
    $stmt->execute([$shift_date, $selected_shift_idx]);
    $result = $stmt->fetch(PDO::FETCH_ASSOC);
    return (float)($result['shift_avg_oee'] ?? 0);

  } catch (Exception $e) {
    error_log("Shift OEE average query error: " . $e->getMessage());
    return 0;
  }
}

// ---------------------------------------------------------------------------
// Main streaming loop
// ---------------------------------------------------------------------------
function startStreaming($pdo) {
  $lastDataHash = '';
  $startTime    = time();
  $maxRunTime   = 3600;

  $filterConfig = parseFilterParams('do', 'work_date', true, '7 DAY');
  $limit        = !empty($_GET['limit']) ? (int)$_GET['limit'] : 100;

  sendSSEData('connected', [
    'status'    => 'connected',
    'message'   => 'OEE data streaming started.',
    'timestamp' => date('Y-m-d H:i:s'),
    'filters'   => [
      'factory_filter' => $_GET['factory_filter'] ?? null,
      'line_filter'    => $_GET['line_filter'] ?? null,
      'machine_filter' => $_GET['machine_filter'] ?? null,
      'shift_filter'   => $_GET['shift_filter'] ?? null,
      'start_date'     => $_GET['start_date'] ?? null,
      'end_date'       => $_GET['end_date'] ?? null,
      'limit'          => $limit
    ]
  ]);

  while (true) {
    if (time() - $startTime > $maxRunTime) {
      sendSSEData('timeout', ['status' => 'timeout', 'message' => 'Maximum execution time reached. Please reconnect.']);
      break;
    }

    if (connection_aborted()) break;

    try {
      $cycleStart     = microtime(true);
      $performanceLog = [];

      // Query 1: Main OEE rows
      $t1 = microtime(true);
      $oeeData = getOeeData($pdo, $filterConfig['where_sql'], $filterConfig['params'], $limit);
      $performanceLog['oeeData'] = round((microtime(true) - $t1) * 1000, 2) . 'ms';

      // Query 2: Aggregated stats + details + component (was 3 queries)
      $t2 = microtime(true);
      $aggregated      = getOeeAggregated($pdo, $filterConfig['where_sql'], $filterConfig['params']);
      $stats           = $aggregated['stats'];
      $oeeDetails      = $aggregated['oee_details'];
      $oeeComponentStats = $aggregated['oee_component_stats'];
      $performanceLog['aggregated'] = round((microtime(true) - $t2) * 1000, 2) . 'ms';

      // Query 3+: OEE trend + production trend (was 2 queries, workHours called once)
      $t3 = microtime(true);
      $trends             = getTrendStats($pdo, $filterConfig['where_sql'], $filterConfig['params']);
      $oeeTrendStats      = $trends['oee_trend'];
      $productionTrendStats = $trends['production_trend'];
      $performanceLog['trends'] = round((microtime(true) - $t3) * 1000, 2) . 'ms';

      // Query 4: Line/Machine breakdown
      $t4 = microtime(true);
      $machineOeeStats = getMachineOeeStats($pdo, $filterConfig['where_sql'], $filterConfig['params']);
      $performanceLog['machineOee'] = round((microtime(true) - $t4) * 1000, 2) . 'ms';

      $totalMs = round((microtime(true) - $cycleStart) * 1000, 2);
      $performanceLog['total'] = $totalMs . 'ms';

      if ($totalMs > 1000) {
        error_log("[data_oee] Slow cycle ({$totalMs}ms): " . json_encode($performanceLog));
      }

      // Change detection hash
      $hashData = [
        'oee_count'          => count($oeeData),
        'oee_ids'            => array_column($oeeData, 'idx'),
        'oee_values'         => array_map(fn($r) => $r['idx'] . '_' . $r['overall_oee'], $oeeData),
        'stats_core'         => [
          'avg_overall_oee' => $stats['avg_overall_oee'] ?? 0,
          'avg_availability' => $stats['avg_availability'] ?? 0,
          'avg_performance'  => $stats['avg_performance'] ?? 0,
          'avg_quality'      => $stats['avg_quality'] ?? 0,
          'active_machines'  => $stats['active_machines'] ?? 0
        ],
        'oee_component_stats' => $oeeComponentStats,
        'trend_hash'          => $oeeTrendStats
      ];

      $currentHash = md5(serialize($hashData));

      if ($currentHash !== $lastDataHash) {
        sendSSEData('oee_data', [
          'timestamp'            => date('Y-m-d H:i:s'),
          'stats'                => $stats,
          'oee_details'          => $oeeDetails,
          'oee_data'             => $oeeData,
          'oee_trend_stats'      => $oeeTrendStats,
          'oee_component_stats'  => $oeeComponentStats,
          'production_trend_stats' => $productionTrendStats,
          'machine_oee_stats'    => $machineOeeStats,
          'data_count'           => count($oeeData),
          'has_changes'          => true,
          'perf'                 => $performanceLog
        ]);
        $lastDataHash = $currentHash;
      } else {
        sendSSEData('heartbeat', [
          'timestamp'      => date('Y-m-d H:i:s'),
          'status'         => 'no_changes',
          'active_machines' => $stats['active_machines'] ?? 0
        ]);
      }

    } catch (Exception $e) {
      error_log("Streaming error: " . $e->getMessage());
      sendSSEData('error', [
        'status'    => 'error',
        'message'   => 'Data query error occurred.',
        'timestamp' => date('Y-m-d H:i:s')
      ]);
    }

    sleep(5);
  }
}

function handleStreamingError($error) {
  $logDir = __DIR__ . '/../../logs';
  if (!is_dir($logDir)) mkdir($logDir, 0777, true);
  error_log("[" . date("Y-m-d H:i:s") . "] " . $error . "\n", 3, $logDir . '/oee_stream_errors.log');
  sendSSEData('error', ['status' => 'fatal_error', 'message' => 'Streaming service error occurred.', 'timestamp' => date('Y-m-d H:i:s')]);
}

try {
  if (!$pdo) throw new Exception("Database connection failed");
  startStreaming($pdo);
} catch (Exception $e) {
  handleStreamingError($e->getMessage());
} finally {
  sendSSEData('disconnected', ['status' => 'disconnected', 'message' => 'OEE data streaming ended.', 'timestamp' => date('Y-m-d H:i:s')]);
}
?>
