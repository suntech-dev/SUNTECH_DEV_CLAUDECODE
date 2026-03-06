<?php
/**
 * OEE Data Real-time Streaming API (Server-Sent Events)
 * Real-time OEE data streaming with 3-level filtering support
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
    $where_clauses[] = 'do.factory_idx = ?';
    $params[] = $_GET['factory_filter'];
  }
  
  if (!empty($_GET['line_filter'])) {
    $where_clauses[] = 'do.line_idx = ?';
    $params[] = $_GET['line_filter'];
  }
  
  if (!empty($_GET['machine_filter'])) {
    $where_clauses[] = 'do.machine_idx = ?';
    $params[] = $_GET['machine_filter'];
  }
  
  if (!empty($_GET['shift_filter'])) {
    $where_clauses[] = 'do.shift_idx = ?';
    $params[] = $_GET['shift_filter'];
  }
  
  if (!empty($_GET['start_date'])) {
    $where_clauses[] = 'do.work_date >= ?';
    $params[] = $_GET['start_date'];
  }
  
  if (!empty($_GET['end_date'])) {
    $where_clauses[] = 'do.work_date <= ?';
    $params[] = $_GET['end_date'];
  }
  
  if (empty($_GET['start_date']) && empty($_GET['end_date'])) {
    $where_clauses[] = 'do.work_date >= DATE_SUB(CURDATE(), INTERVAL 7 DAY)';
  }
  
  $where_sql = count($where_clauses) > 0 ? ' WHERE ' . implode(' AND ', $where_clauses) : '';
  
  return ['where_sql' => $where_sql, 'params' => $params];
}

function getOeeData($pdo, $where_sql, $params, $limit = 100) {
  try {
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
      {$where_sql}
      ORDER BY do.work_date DESC, do.update_date DESC, do.idx DESC
      LIMIT " . (int)$limit . "
    ";
    
    $stmt = $pdo->prepare($sql);
    $stmt->execute($params);
    
    $result = $stmt->fetchAll(PDO::FETCH_ASSOC);
    
    return $result;
    
  } catch (PDOException $e) {
    error_log("OEE data query error: " . $e->getMessage());
    return [];
  }
}

function getOeeStats($pdo, $where_sql, $params) {
  try {
    $sql = "
      SELECT 
        COUNT(*) as total_count,
        ROUND(AVG(do.oee), 2) as avg_overall_oee,
        ROUND(AVG(do.availabilty_rate), 2) as avg_availability,
        ROUND(AVG(do.productivity_rate), 2) as avg_performance,
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
    error_log("OEE statistics query error: " . $e->getMessage());
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

function getOeeDetails($pdo, $where_sql, $params) {
  try {
    $sql = "
      SELECT
        ROUND(AVG(do.oee), 2) as overall_oee,
        ROUND(AVG(do.availabilty_rate), 2) as availability,
        ROUND(AVG(do.productivity_rate), 2) as performance,
        ROUND(AVG(do.quality_rate), 2) as quality,
        SUM(do.runtime) as runtime,
        SUM(do.planned_work_time) as planned_time,
        SUM(do.actual_output) as actual_output,
        SUM(do.theoritical_output) as theoretical_output,
        SUM(do.actual_a_grade) as good_products,
        SUM(do.defective) as defective_products
      FROM data_oee do
      {$where_sql}
      AND do.work_date = CURDATE()
    ";

    $stmt = $pdo->prepare($sql);
    $stmt->execute($params);

    $details = $stmt->fetch(PDO::FETCH_ASSOC);

    if ($details['runtime']) {
      $details['runtime'] = round($details['runtime'] / 60, 1);
    }
    if ($details['planned_time']) {
      $details['planned_time'] = round($details['planned_time'] / 60, 1);
    }

    // target_achievement 계산
    $target_oee = 85;
    if ($details['overall_oee']) {
      $details['target_achievement'] = round(($details['overall_oee'] / $target_oee) * 100, 1) . '%';
    } else {
      $details['target_achievement'] = '0%';
    }

    return $details ?: [
      'overall_oee' => 0,
      'availability' => 0,
      'performance' => 0,
      'quality' => 0,
      'runtime' => 0,
      'planned_time' => 0,
      'actual_output' => 0,
      'theoretical_output' => 0,
      'good_products' => 0,
      'defective_products' => 0,
      'target_achievement' => '0%'
    ];

  } catch (PDOException $e) {
    error_log("OEE details query error: " . $e->getMessage());
    return [
      'overall_oee' => 0,
      'availability' => 0,
      'performance' => 0,
      'quality' => 0,
      'runtime' => 0,
      'planned_time' => 0,
      'actual_output' => 0,
      'theoretical_output' => 0,
      'good_products' => 0,
      'defective_products' => 0,
      'target_achievement' => '0%'
    ];
  }
}

function getOeeTrendStats($pdo, $where_sql, $params) {
  try {
    $startDate = $_GET['start_date'] ?? '';
    $endDate = $_GET['end_date'] ?? '';
    $isHourlyView = false;
    
    if (!empty($startDate) && !empty($endDate)) {
      $start = new DateTime($startDate);
      $end = new DateTime($endDate);
      
      $startDateOnly = $start->format('Y-m-d');
      $endDateOnly = $end->format('Y-m-d');
      
      if ($startDateOnly === $endDateOnly) {
        $isHourlyView = true;
      } else {
        $diff = $start->diff($end);
        $daysDiff = $diff->days;
        
        if ($daysDiff <= 1) {
          $isHourlyView = true;
        }
      }
    } else {
      $isHourlyView = true;
    }
    
    if ($isHourlyView) {
      $sql = "
        SELECT 
          DATE_FORMAT(do.update_date, '%Y-%m-%d %H:00:00') as time_label,
          DATE_FORMAT(do.update_date, '%H:00') as display_label,
          ROUND(AVG(do.oee), 2) as overall_oee,
          ROUND(AVG(do.availabilty_rate), 2) as availability,
          ROUND(AVG(do.productivity_rate), 2) as performance,
          ROUND(AVG(do.quality_rate), 2) as quality,
          COUNT(*) as record_count
        FROM data_oee do
        LEFT JOIN info_factory f ON do.factory_idx = f.idx
        LEFT JOIN info_line l ON do.line_idx = l.idx
        {$where_sql}
        GROUP BY DATE_FORMAT(do.update_date, '%Y-%m-%d %H:00:00'), DATE_FORMAT(do.update_date, '%H:00')
        ORDER BY time_label ASC
        LIMIT 24
      ";
      $viewType = 'hourly';
    } else {
      $sql = "
        SELECT 
          do.work_date as time_label,
          DATE_FORMAT(do.work_date, '%m/%d') as display_label,
          ROUND(AVG(do.oee), 2) as overall_oee,
          ROUND(AVG(do.availabilty_rate), 2) as availability,
          ROUND(AVG(do.productivity_rate), 2) as performance,
          ROUND(AVG(do.quality_rate), 2) as quality,
          COUNT(*) as record_count
        FROM data_oee do
        LEFT JOIN info_factory f ON do.factory_idx = f.idx
        LEFT JOIN info_line l ON do.line_idx = l.idx
        {$where_sql}
        GROUP BY do.work_date, DATE_FORMAT(do.work_date, '%m/%d')
        ORDER BY time_label ASC
        LIMIT 30
      ";
      $viewType = 'daily';
    }
    
    $stmt = $pdo->prepare($sql);
    $stmt->execute($params);
    $results = $stmt->fetchAll(PDO::FETCH_ASSOC);
    
    return [
      'view_type' => $viewType,
      'data' => $results
    ];
    
  } catch (PDOException $e) {
    error_log("OEE trend stats query error: " . $e->getMessage());
    
    return [
      'view_type' => 'hourly',
      'data' => []
    ];
  }
}

function getOeeComponentStats($pdo, $where_sql, $params) {
  try {
    $sql = "
      SELECT 
        ROUND(AVG(do.availabilty_rate), 2) as availability,
        ROUND(AVG(do.productivity_rate), 2) as performance,
        ROUND(AVG(do.quality_rate), 2) as quality,
        ROUND(AVG(do.oee), 2) as overall_oee
      FROM data_oee do
      {$where_sql}
      AND do.work_date >= CURDATE() - INTERVAL 1 DAY
    ";
    
    $stmt = $pdo->prepare($sql);
    $stmt->execute($params);
    
    $result = $stmt->fetch(PDO::FETCH_ASSOC);
    
    return $result ?: [
      'availability' => 0,
      'performance' => 0,
      'quality' => 0,
      'overall_oee' => 0
    ];
    
  } catch (PDOException $e) {
    error_log("OEE component stats query error: " . $e->getMessage());
    return [
      'availability' => 0,
      'performance' => 0,
      'quality' => 0,
      'overall_oee' => 0
    ];
  }
}

function getProductionTrendStats($pdo, $where_sql, $params) {
  try {
    $sql = "
      SELECT 
        DATE_FORMAT(do.update_date, '%H:00') as time_label,
        DATE_FORMAT(do.update_date, '%H:00') as display_label,
        SUM(do.actual_output) as actual_output,
        SUM(do.theoritical_output) as target_output,
        ROUND(AVG(do.productivity_rate), 2) as avg_performance
      FROM data_oee do
      {$where_sql}
      AND do.work_date = CURDATE()
      GROUP BY DATE_FORMAT(do.update_date, '%H:00')
      ORDER BY time_label ASC
      LIMIT 24
    ";
    
    $stmt = $pdo->prepare($sql);
    $stmt->execute($params);
    
    return $stmt->fetchAll(PDO::FETCH_ASSOC);
    
  } catch (PDOException $e) {
    error_log("Production trend stats query error: " . $e->getMessage());
    return [];
  }
}

function getMachineOeeStats($pdo, $where_sql, $params) {
  try {
    $sql = "
      SELECT 
        do.machine_no,
        ROUND(AVG(do.availabilty_rate), 2) as availability,
        ROUND(AVG(do.productivity_rate), 2) as performance,
        ROUND(AVG(do.quality_rate), 2) as quality,
        ROUND(AVG(do.oee), 2) as overall_oee,
        COUNT(*) as record_count
      FROM data_oee do
      {$where_sql}
      AND do.work_date >= CURDATE() - INTERVAL 3 DAY
      GROUP BY do.machine_no
      ORDER BY overall_oee DESC
      LIMIT 10
    ";
    
    $stmt = $pdo->prepare($sql);
    $stmt->execute($params);
    
    return $stmt->fetchAll(PDO::FETCH_ASSOC);
    
  } catch (PDOException $e) {
    error_log("Machine OEE stats query error: " . $e->getMessage());
    return [];
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
  $limit = !empty($_GET['limit']) ? (int)$_GET['limit'] : 100;
  
  sendSSEData('connected', [
    'status' => 'connected',
    'message' => 'OEE data streaming started.',
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
      $oeeData = getOeeData($pdo, $filterConfig['where_sql'], $filterConfig['params'], $limit);
      $stats = getOeeStats($pdo, $filterConfig['where_sql'], $filterConfig['params']);
      $oeeDetails = getOeeDetails($pdo, $filterConfig['where_sql'], $filterConfig['params']);
      $oeeTrendStats = getOeeTrendStats($pdo, $filterConfig['where_sql'], $filterConfig['params']);
      $oeeComponentStats = getOeeComponentStats($pdo, $filterConfig['where_sql'], $filterConfig['params']);
      $productionTrendStats = getProductionTrendStats($pdo, $filterConfig['where_sql'], $filterConfig['params']);
      $machineOeeStats = getMachineOeeStats($pdo, $filterConfig['where_sql'], $filterConfig['params']);
      $hashData = [
        'oee_count' => count($oeeData),
        'oee_ids' => array_column($oeeData, 'idx'),
        'oee_values' => array_map(function($item) {
          return $item['idx'] . '_' . $item['overall_oee']; 
        }, $oeeData),
        'stats_core' => [
          'avg_overall_oee' => $stats['avg_overall_oee'] ?? 0,
          'avg_availability' => $stats['avg_availability'] ?? 0,
          'avg_performance' => $stats['avg_performance'] ?? 0,
          'avg_quality' => $stats['avg_quality'] ?? 0,
          'active_machines' => $stats['active_machines'] ?? 0
        ],
        'oee_component_stats' => $oeeComponentStats,
        'trend_data_hash' => $oeeTrendStats
      ];
      
      $currentDataHash = md5(serialize($hashData));
      
      if ($currentDataHash !== $lastDataHash) {
        $responseData = [
          'timestamp' => date('Y-m-d H:i:s'),
          'stats' => $stats,
          'oee_details' => $oeeDetails,
          'oee_data' => $oeeData,
          'oee_trend_stats' => $oeeTrendStats,
          'oee_component_stats' => $oeeComponentStats,
          'production_trend_stats' => $productionTrendStats,
          'machine_oee_stats' => $machineOeeStats,
          'data_count' => count($oeeData),
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
      
      sendSSEData('error', [
        'status' => 'error',
        'message' => 'Data query error occurred.',
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
        ROUND(AVG(do.oee), 2) as previous_day_oee,
        do.work_date
      FROM data_oee do
      {$where_sql}
      " . (strpos($where_sql, 'WHERE') !== false ? 'AND' : 'WHERE') . " do.work_date < CURDATE()
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
      SELECT ROUND(AVG(oee), 2) as shift_avg_oee
      FROM data_oee
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
  
  $logFile = $logDir . '/oee_stream_errors.log';
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
    'message' => 'OEE data streaming ended.',
    'timestamp' => date('Y-m-d H:i:s')
  ]);
}

?>