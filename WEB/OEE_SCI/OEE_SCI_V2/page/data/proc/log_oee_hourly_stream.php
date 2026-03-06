<?php
/**
 * OEE Hourly Data Log Streaming API (Server-Sent Events)
 * Real-time OEE hourly data log streaming from data_oee_rows_hourly table
 */

date_default_timezone_set('Asia/Jakarta');

// Load required libraries
require_once(__DIR__ . '/../../../lib/db.php');
require_once(__DIR__ . '/../../../lib/api_helper.lib.php');
require_once(__DIR__ . '/../../../lib/worktime.lib.php');
require_once(__DIR__ . '/../../../lib/get_shift.lib.php');

// SSE headers
header('Content-Type: text/event-stream');
header('Cache-Control: no-cache');
header('Connection: keep-alive');
header('Access-Control-Allow-Origin: *');
header('Access-Control-Allow-Methods: GET');
header('Access-Control-Allow-Headers: Cache-Control');

// Disable output buffering
if (ob_get_level()) ob_end_clean();

// Initialize API helper
$apiHelper = new ApiHelper($pdo);

function parseFilterParams() {
  $params = [];
  $where_clauses = [];

  if (!empty($_GET['factory_filter'])) {
    $where_clauses[] = 'doh.factory_idx = ?';
    $params[] = $_GET['factory_filter'];
  }

  if (!empty($_GET['line_filter'])) {
    $where_clauses[] = 'doh.line_idx = ?';
    $params[] = $_GET['line_filter'];
  }

  if (!empty($_GET['machine_filter'])) {
    $where_clauses[] = 'doh.machine_idx = ?';
    $params[] = $_GET['machine_filter'];
  }

  if (!empty($_GET['shift_filter'])) {
    $where_clauses[] = 'doh.shift_idx = ?';
    $params[] = $_GET['shift_filter'];
  }

  if (!empty($_GET['start_date'])) {
    $where_clauses[] = 'doh.work_date >= ?';
    $params[] = $_GET['start_date'];
  }

  if (!empty($_GET['end_date'])) {
    $where_clauses[] = 'doh.work_date <= ?';
    $params[] = $_GET['end_date'];
  }

  if (empty($_GET['start_date']) && empty($_GET['end_date'])) {
    $where_clauses[] = 'doh.work_date >= DATE_SUB(CURDATE(), INTERVAL 7 DAY)';
  }

  $where_sql = count($where_clauses) > 0 ? ' WHERE ' . implode(' AND ', $where_clauses) : '';

  return ['where_sql' => $where_sql, 'params' => $params];
}

function getOeeHourlyDataLog($pdo, $where_sql, $params, $limit = 1000) {
  try {
    // data_oee_rows_hourly 테이블의 모든 컬럼을 조회
    $sql = "
      SELECT
        doh.idx,
        doh.work_date,
        doh.time_update,
        doh.shift_idx,
        doh.factory_idx,
        doh.factory_name,
        doh.line_idx,
        doh.line_name,
        doh.mac,
        doh.machine_idx,
        doh.machine_no,
        doh.process_name,
        doh.planned_work_time,
        doh.runtime,
        doh.productive_runtime,
        doh.downtime,
        doh.availabilty_rate,
        doh.target_line_per_day,
        doh.target_line_per_hour,
        doh.target_mc_per_day,
        doh.target_mc_per_hour,
        doh.cycletime,
        doh.pair_info,
        doh.pair_count,
        doh.theoritical_output,
        doh.actual_output,
        doh.productivity_rate,
        doh.defective,
        doh.actual_a_grade,
        doh.quality_rate,
        doh.oee,
        doh.reg_date,
        doh.update_date,
        doh.work_hour
      FROM data_oee_rows_hourly doh
      {$where_sql}
      ORDER BY doh.work_date DESC, doh.work_hour DESC, doh.update_date DESC, doh.idx DESC
      LIMIT " . (int)$limit . "
    ";

    $stmt = $pdo->prepare($sql);
    $stmt->execute($params);

    $result = $stmt->fetchAll(PDO::FETCH_ASSOC);

    // shift_idx를 "Shift 1" 형식으로 변환
    foreach ($result as &$row) {
      if (isset($row['shift_idx']) && $row['shift_idx'] !== null && $row['shift_idx'] !== '') {
        $row['shift_idx'] = 'Shift ' . $row['shift_idx'];
      }
    }

    return $result;

  } catch (PDOException $e) {
    error_log("OEE hourly data log query error: " . $e->getMessage());
    return [];
  }
}

function getOeeHourlyStats($pdo, $where_sql, $params) {
  try {
    $sql = "
      SELECT
        COUNT(*) as total_count,
        ROUND((AVG(doh.availabilty_rate) * (SUM(doh.actual_output) / NULLIF(SUM(doh.theoritical_output), 0)) * AVG(doh.quality_rate)) / 100, 2) as avg_overall_oee,
        ROUND(AVG(doh.availabilty_rate), 2) as avg_availability,
        ROUND((SUM(doh.actual_output) / NULLIF(SUM(doh.theoritical_output), 0)) * 100, 2) as avg_performance,
        ROUND(AVG(doh.quality_rate), 2) as avg_quality,
        MAX(doh.oee) as max_oee,
        MIN(doh.oee) as min_oee,
        COUNT(DISTINCT doh.machine_idx) as active_machines,
        SUM(CASE WHEN doh.oee >= 85 THEN 1 ELSE 0 END) as excellent_count,
        SUM(CASE WHEN doh.oee >= 70 AND doh.oee < 85 THEN 1 ELSE 0 END) as good_count,
        SUM(CASE WHEN doh.oee >= 50 AND doh.oee < 70 THEN 1 ELSE 0 END) as fair_count,
        SUM(CASE WHEN doh.oee < 50 THEN 1 ELSE 0 END) as poor_count,
        ROUND(AVG(CASE WHEN doh.work_date = CURDATE() THEN doh.oee ELSE NULL END), 2) as today_avg_oee,
        COUNT(CASE WHEN doh.work_date = CURDATE() THEN 1 ELSE NULL END) as today_count,
        SUM(doh.actual_output) as total_actual_output,
        SUM(doh.theoritical_output) as total_theoretical_output,
        SUM(doh.actual_a_grade) as total_good_products,
        SUM(doh.defective) as total_defective_count,
        SUM(doh.planned_work_time) as total_planned_time,
        SUM(doh.runtime) as total_runtime,
        SUM(doh.downtime) as total_downtime
      FROM data_oee_rows_hourly doh
      {$where_sql}
    ";

    $stmt = $pdo->prepare($sql);
    $stmt->execute($params);

    $stats = $stmt->fetch(PDO::FETCH_ASSOC);

    $selected_shift = $_GET['shift_filter'] ?? null;
    $target_date = $_GET['start_date'] ?? $_GET['end_date'] ?? null;
    $stats['current_shift_oee'] = getCurrentShiftOeeAvg($pdo, $selected_shift, $target_date);

    // 가장 최근 day의 OEE 계산 (오늘 제외, 휴일 등으로 데이터 없으면 그 이전 날짜)
    $stats['previous_day_oee'] = getPreviousDayOeeAvg($pdo, $where_sql, $params);

    if ($stats['total_theoretical_output'] > 0) {
      $stats['overall_performance_rate'] = round(($stats['total_actual_output'] / $stats['total_theoretical_output']) * 100, 2);
    } else {
      $stats['overall_performance_rate'] = 0;
    }

    if ($stats['total_actual_output'] > 0) {
      $stats['overall_quality_rate'] = round(($stats['total_good_products'] / $stats['total_actual_output']) * 100, 2);
    } else {
      $stats['overall_quality_rate'] = 0;
    }

    if ($stats['total_planned_time'] > 0) {
      $stats['overall_availability_rate'] = round(($stats['total_runtime'] / $stats['total_planned_time']) * 100, 2);
    } else {
      $stats['overall_availability_rate'] = 0;
    }

    // stat-card용 필드 매핑
    $stats['overall_oee'] = $stats['avg_overall_oee'];
    $stats['availability'] = $stats['avg_availability'];
    $stats['performance'] = $stats['avg_performance'];
    $stats['quality'] = $stats['avg_quality'];

    return $stats;

  } catch (PDOException $e) {
    error_log("OEE hourly statistics query error: " . $e->getMessage());
    return [
      'total_count' => 0,
      'avg_overall_oee' => 0,
      'avg_availability' => 0,
      'avg_performance' => 0,
      'avg_quality' => 0,
      'max_oee' => 0,
      'min_oee' => 0,
      'active_machines' => 0,
      'excellent_count' => 0,
      'good_count' => 0,
      'fair_count' => 0,
      'poor_count' => 0,
      'today_avg_oee' => 0,
      'today_count' => 0,
      'current_shift_oee' => 0,
      'previous_day_oee' => 0,
      'total_actual_output' => 0,
      'total_theoretical_output' => 0,
      'total_good_products' => 0,
      'total_defective_count' => 0,
      'total_planned_time' => 0,
      'total_runtime' => 0,
      'total_downtime' => 0,
      'overall_performance_rate' => 0,
      'overall_quality_rate' => 0,
      'overall_availability_rate' => 0,
      // stat-card용 필드 매핑 (기본값)
      'overall_oee' => 0,
      'availability' => 0,
      'performance' => 0,
      'quality' => 0
    ];
  }
}

function sendSSEData($eventType, $data) {
  echo "event: {$eventType}\n";
  echo "data: " . json_encode($data, JSON_UNESCAPED_UNICODE | JSON_UNESCAPED_SLASHES) . "\n\n";
  flush();
}

function startStreaming($pdo) {
  $lastDataHash = '';
  $startTime = time();
  $maxRunTime = 3600;

  $filterConfig = parseFilterParams();
  $limit = !empty($_GET['limit']) ? (int)$_GET['limit'] : 1000;

  sendSSEData('connected', [
    'status' => 'connected',
    'message' => 'OEE hourly data log streaming started.',
    'timestamp' => date('Y-m-d H:i:s'),
    'filters' => [
      'factory_filter' => $_GET['factory_filter'] ?? null,
      'line_filter' => $_GET['line_filter'] ?? null,
      'machine_filter' => $_GET['machine_filter'] ?? null,
      'shift_filter' => $_GET['shift_filter'] ?? null,
      'start_date' => $_GET['start_date'] ?? null,
      'end_date' => $_GET['end_date'] ?? null,
      'limit' => $limit
    ]
  ]);

  while (true) {
    if (time() - $startTime > $maxRunTime) {
      sendSSEData('timeout', [
        'status' => 'timeout',
        'message' => 'Maximum execution time reached. Please reconnect.'
      ]);
      break;
    }

    if (connection_aborted()) {
      error_log("Client connection aborted.");
      break;
    }

    try {
      $oeeDataLog = getOeeHourlyDataLog($pdo, $filterConfig['where_sql'], $filterConfig['params'], $limit);

      // Stats 조회를 try-catch로 감싸서 실패해도 데이터는 전송되도록 함
      try {
        $stats = getOeeHourlyStats($pdo, $filterConfig['where_sql'], $filterConfig['params']);
      } catch (Exception $statsError) {
        error_log("Stats query error: " . $statsError->getMessage());
        $stats = [
          'total_count' => 0,
          'avg_overall_oee' => 0,
          'avg_availability' => 0,
          'avg_performance' => 0,
          'avg_quality' => 0,
          'max_oee' => 0,
          'min_oee' => 0,
          'active_machines' => 0,
          'current_shift_oee' => 0,
          'previous_day_oee' => 0,
          'overall_oee' => 0,
          'availability' => 0,
          'performance' => 0,
          'quality' => 0
        ];
      }

      $hashData = [
        'oee_count' => count($oeeDataLog),
        'oee_ids' => array_column($oeeDataLog, 'idx'),
        'oee_values' => array_map(function($item) {
          return $item['idx'] . '_' . ($item['oee'] ?? '0');
        }, $oeeDataLog),
        'stats_core' => [
          'avg_overall_oee' => $stats['avg_overall_oee'] ?? 0,
          'avg_availability' => $stats['avg_availability'] ?? 0,
          'avg_performance' => $stats['avg_performance'] ?? 0,
          'avg_quality' => $stats['avg_quality'] ?? 0,
          'active_machines' => $stats['active_machines'] ?? 0
        ]
      ];

      $currentDataHash = md5(serialize($hashData));

      if ($currentDataHash !== $lastDataHash || count($oeeDataLog) > 0) {
        $responseData = [
          'timestamp' => date('Y-m-d H:i:s'),
          'stats' => $stats,
          'oee_data' => $oeeDataLog,
          'data_count' => count($oeeDataLog),
          'has_changes' => true
        ];

        sendSSEData('oee_data', $responseData);
        $lastDataHash = $currentDataHash;
      } else {
        sendSSEData('heartbeat', [
          'timestamp' => date('Y-m-d H:i:s'),
          'status' => 'no_changes',
          'active_machines' => $stats['active_machines'] ?? 0
        ]);
      }

    } catch (Exception $e) {
      error_log("Streaming error: " . $e->getMessage());
      error_log("Stack trace: " . $e->getTraceAsString());

      sendSSEData('error', [
        'status' => 'error',
        'message' => 'Data query error occurred: ' . $e->getMessage(),
        'timestamp' => date('Y-m-d H:i:s')
      ]);
    }

    sleep(5);
  }
}

function getPreviousDayOeeAvg($pdo, $where_sql, $params) {
  try {
    // 오늘 제외하고 가장 최근 날짜의 평균 OEE 조회
    $sql = "
      SELECT
        ROUND((AVG(doh.availabilty_rate) * (SUM(doh.actual_output) / NULLIF(SUM(doh.theoritical_output), 0)) * AVG(doh.quality_rate)) / 100, 2) as previous_day_oee,
        doh.work_date
      FROM data_oee_rows_hourly doh
      {$where_sql}
      " . (strpos($where_sql, 'WHERE') !== false ? 'AND' : 'WHERE') . " doh.work_date < CURDATE()
      GROUP BY doh.work_date
      ORDER BY doh.work_date DESC
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
    $current_datetime = date('Y-m-d H:i:s');
    $target_date = $target_date ?: date('Y-m-d');

    $worktime = new Worktime($pdo);

    $factory_idx = '';
    $line_idx = '';

    if (!empty($selected_shift_idx)) {
      $day_shifts = $worktime->getDayShift($target_date, $factory_idx, $line_idx);

      if (!$day_shifts || !isset($day_shifts['shift'][$selected_shift_idx])) {
        return 0;
      }

      $shift = $day_shifts['shift'][$selected_shift_idx];

      $work_stime_str = $target_date . ' ' . $shift['available_stime'] . ':00';
      $work_etime_str = $target_date . ' ' . $shift['available_etime'] . ':00';

      if ($shift['over_time']) {
        $work_etime_str = date('Y-m-d H:i:s', strtotime($work_etime_str . ' +' . $shift['over_time'] . ' minutes'));
      }

      if ($work_etime_str <= $work_stime_str) {
        $work_etime_str = date('Y-m-d H:i:s', strtotime($work_etime_str . ' +1 day'));
      }

      $shift_start = $work_stime_str;
      $shift_end = $work_etime_str;

    } else {
      $current_shift_info = findCurrentShift($pdo, $worktime, $factory_idx, $line_idx, $current_datetime);

      if (!$current_shift_info) {
        return 0;
      }

      $shift_start = $current_shift_info['work_stime'];
      $shift_end = $current_shift_info['work_etime'];
    }

    $sql = "
      SELECT ROUND((AVG(availabilty_rate) * (SUM(actual_output) / NULLIF(SUM(theoritical_output), 0)) * AVG(quality_rate)) / 100, 2) as shift_avg_oee
      FROM data_oee_rows_hourly
      WHERE work_date = ? AND shift_idx = ?
    ";

    $stmt = $pdo->prepare($sql);
    $stmt->execute([date('Y-m-d', strtotime($shift_start)), $selected_shift_idx ?: 1]);

    $result = $stmt->fetch(PDO::FETCH_ASSOC);

    return (float)($result['shift_avg_oee'] ?? 0);

  } catch (Exception $e) {
    error_log("Shift OEE average query error: " . $e->getMessage());
    return 0;
  }
}

function handleStreamingError($error) {
  $logDir = __DIR__ . '/../../logs';
  if (!is_dir($logDir)) {
    mkdir($logDir, 0777, true);
  }

  $logFile = $logDir . '/log_oee_hourly_stream_errors.log';
  $errorMessage = "[" . date("Y-m-d H:i:s") . "] " . $error . "\n";
  error_log($errorMessage, 3, $logFile);

  sendSSEData('error', [
    'status' => 'fatal_error',
    'message' => 'Streaming service error occurred.',
    'timestamp' => date('Y-m-d H:i:s')
  ]);
}

try {
  if (!$pdo) {
    throw new Exception("Database connection failed");
  }

  startStreaming($pdo);

} catch (Exception $e) {
  handleStreamingError($e->getMessage());
} finally {
  sendSSEData('disconnected', [
    'status' => 'disconnected',
    'message' => 'OEE hourly data log streaming ended.',
    'timestamp' => date('Y-m-d H:i:s')
  ]);
}

?>
