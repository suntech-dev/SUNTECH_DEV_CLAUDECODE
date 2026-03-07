<?php
/**
 * Defective Data Real-time Streaming API (Server-Sent Events)
 * Optimized (2026-03-07):
 * - Merged getDefectiveTypeStats: 2 queries (info_defective + data_defective) + PHP merge
 *   → 1 query (info_defective LEFT JOIN data_defective, filter in ON clause)
 * - Removed unnecessary info_factory/info_line JOINs from type stats query
 *
 * Features:
 * - Real-time data streaming from data_defective table via SSE
 * - 3-level filtering support (Factory → Line → Machine)
 * - Date range filtering
 * - Real-time status change detection
 * - Warning status only (Completed status not used)
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

function getDefectiveData($pdo, $where_sql, $params, $limit = 100) {
  try {
    $sql = "
      SELECT 
        dd.idx,
        dd.work_date,
        dd.shift_idx,
        dd.machine_no,
        dd.defective_name,
        dd.status,
        dd.reg_date,
        dd.update_date,
        f.factory_name,
        l.line_name,
        CASE 
          WHEN dd.status = 'Warning' THEN 'Warning'
          ELSE dd.status
        END as status_display
      FROM data_defective dd
      LEFT JOIN info_factory f ON dd.factory_idx = f.idx
      LEFT JOIN info_line l ON dd.line_idx = l.idx  
      {$where_sql}
      ORDER BY dd.reg_date DESC, dd.idx DESC
      LIMIT " . (int)$limit . "
    ";
    
    $stmt = $pdo->prepare($sql);
    $stmt->execute($params);
    
    $result = $stmt->fetchAll(PDO::FETCH_ASSOC);
    
    return $result;
    
  } catch (PDOException $e) {
    error_log("Defective data query error: " . $e->getMessage());
    return [];
  }
}

/**
 * Calculate defective rate and quality score from data_oee table
 * Formula:
 * - Defective Rate = (SUM(defective) / SUM(actual_output)) * 100
 * - Quality Score = 100 - Defective Rate
 */
function calculateRatesFromOEE($pdo, $filter_params) {
  try {
    // Build WHERE clause for data_oee table
    $oee_where_clauses = [];
    $oee_params = [];

    if (!empty($_GET['factory_filter'])) {
      $oee_where_clauses[] = 'factory_idx = ?';
      $oee_params[] = $_GET['factory_filter'];
    }

    if (!empty($_GET['line_filter'])) {
      $oee_where_clauses[] = 'line_idx = ?';
      $oee_params[] = $_GET['line_filter'];
    }

    if (!empty($_GET['machine_filter'])) {
      $oee_where_clauses[] = 'machine_idx = ?';
      $oee_params[] = $_GET['machine_filter'];
    }

    if (!empty($_GET['shift_filter'])) {
      $oee_where_clauses[] = 'shift_idx = ?';
      $oee_params[] = $_GET['shift_filter'];
    }

    if (!empty($_GET['start_date'])) {
      $oee_where_clauses[] = 'work_date >= ?';
      $oee_params[] = $_GET['start_date'];
    }

    if (!empty($_GET['end_date'])) {
      $oee_where_clauses[] = 'work_date <= ?';
      $oee_params[] = $_GET['end_date'];
    }

    // Default date range if not specified
    if (empty($_GET['start_date']) && empty($_GET['end_date'])) {
      $oee_where_clauses[] = 'work_date >= DATE_SUB(CURDATE(), INTERVAL 2 DAY)';
    }

    $oee_where_sql = count($oee_where_clauses) > 0 ? ' WHERE ' . implode(' AND ', $oee_where_clauses) : '';

    $sql = "
      SELECT
        SUM(actual_output) as total_output,
        SUM(defective) as total_defective
      FROM data_oee
      {$oee_where_sql}
    ";

    $stmt = $pdo->prepare($sql);
    $stmt->execute($oee_params);

    $result = $stmt->fetch(PDO::FETCH_ASSOC);

    $total_output = floatval($result['total_output'] ?? 0);
    $total_defective = floatval($result['total_defective'] ?? 0);

    // Calculate defective rate and quality score
    if ($total_output > 0) {
      $defective_rate = ($total_defective / $total_output) * 100;
      $quality_score = 100 - $defective_rate;
      return [
        'defective_rate' => round($defective_rate, 2),
        'quality_score' => round($quality_score, 2),
        'total_output' => $total_output,
        'total_defective' => $total_defective
      ];
    } else {
      // No data available - return null for rates
      return [
        'defective_rate' => null,
        'quality_score' => null,
        'total_output' => $total_output,
        'total_defective' => $total_defective
      ];
    }

  } catch (PDOException $e) {
    error_log("OEE rates calculation error: " . $e->getMessage());
    return [
      'defective_rate' => null,
      'quality_score' => null,
      'total_output' => 0,
      'total_defective' => 0
    ];
  }
}

function getDefectiveStats($pdo, $where_sql, $params) {
  try {
    $sql = "
      SELECT
        COUNT(*) as total_count,
        SUM(CASE WHEN dd.status = 'Warning' THEN 1 ELSE 0 END) as warning_count,
        COUNT(DISTINCT dd.machine_idx) as affected_machines,
        COUNT(DISTINCT dd.defective_idx) as defective_types_used,
        SUM(CASE WHEN DATE(dd.reg_date) = CURDATE() THEN 1 ELSE 0 END) as today_count,
        SUM(CASE WHEN DATE(dd.reg_date) = CURDATE() AND dd.status = 'Warning' THEN 1 ELSE 0 END) as today_warning_count
      FROM data_defective dd
      {$where_sql}
    ";

    $stmt = $pdo->prepare($sql);
    $stmt->execute($params);

    $stats = $stmt->fetch(PDO::FETCH_ASSOC);

    // Calculate rates from data_oee table
    $rates = calculateRatesFromOEE($pdo, $params);

    // Merge OEE rates into stats
    $stats['defective_rate'] = $rates['defective_rate'];
    $stats['quality_score'] = $rates['quality_score'];

    $selected_shift = $_GET['shift_filter'] ?? null;
    $target_date = $_GET['start_date'] ?? $_GET['end_date'] ?? null;
    $stats['current_shift_count'] = getCurrentShiftDefectiveCount($pdo, $selected_shift, $target_date);

    return $stats;

  } catch (PDOException $e) {
    error_log("Defective stats query error: " . $e->getMessage());
    return [
      'total_count' => 0,
      'warning_count' => 0,
      'affected_machines' => 0,
      'defective_types_used' => 0,
      'today_count' => 0,
      'today_warning_count' => 0,
      'current_shift_count' => 0,
      'defective_rate' => null,
      'quality_score' => null
    ];
  }
}

function getDefectiveTypeStats($pdo, $where_sql, $params) {
  try {
    $on_conditions = !empty(trim($where_sql))
      ? ' AND ' . trim(preg_replace('/^\s*WHERE\s*/i', '', $where_sql))
      : '';

    $sql = "
      SELECT
        id.defective_name,
        COALESCE(COUNT(dd.idx), 0) as count,
        COALESCE(SUM(CASE WHEN dd.status = 'Warning' THEN 1 ELSE 0 END), 0) as warning_count
      FROM info_defective id
      LEFT JOIN data_defective dd
        ON id.idx = dd.defective_idx{$on_conditions}
      WHERE id.status = 'Y'
      GROUP BY id.idx, id.defective_name
      ORDER BY id.defective_name ASC
    ";

    $stmt = $pdo->prepare($sql);
    $stmt->execute($params);
    return $stmt->fetchAll(PDO::FETCH_ASSOC);

  } catch (PDOException $e) {
    error_log("Defective type stats query error: " . $e->getMessage());
    return [];
  }
}

function getDefectiveRateStats($pdo, $where_sql, $params) {
  try {
    $startDate = $_GET['start_date'] ?? '';
    $endDate = $_GET['end_date'] ?? '';
    $isHourlyView = false;
    $workHoursInfo = null;

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

    // Get work hours information for hourly view (1 day or less)
    if ($isHourlyView) {
      $targetDate = !empty($startDate) ? $startDate : date('Y-m-d');
      $workHoursInfo = getWorkHoursForDate($pdo, $targetDate);
    }
    
    if ($isHourlyView) {
      $sql = "
        SELECT
          DATE_FORMAT(dd.reg_date, '%Y-%m-%d %H:00:00') as time_label,
          DATE_FORMAT(dd.reg_date, '%H:00') as display_label,
          COUNT(*) as total_count,
          SUM(CASE WHEN dd.status = 'Warning' THEN 1 ELSE 0 END) as warning_count,
          COUNT(*) as defective_count
        FROM data_defective dd
        LEFT JOIN info_factory f ON dd.factory_idx = f.idx
        LEFT JOIN info_line l ON dd.line_idx = l.idx
        {$where_sql}
        GROUP BY DATE_FORMAT(dd.reg_date, '%Y-%m-%d %H:00:00'), DATE_FORMAT(dd.reg_date, '%H:00')
        ORDER BY time_label ASC
        LIMIT 48
      ";
      $viewType = 'hourly';
    } else {
      $sql = "
        SELECT
          DATE(dd.reg_date) as time_label,
          DATE_FORMAT(dd.reg_date, '%m/%d') as display_label,
          COUNT(*) as total_count,
          SUM(CASE WHEN dd.status = 'Warning' THEN 1 ELSE 0 END) as warning_count,
          COUNT(*) as defective_count
        FROM data_defective dd
        LEFT JOIN info_factory f ON dd.factory_idx = f.idx
        LEFT JOIN info_line l ON dd.line_idx = l.idx
        {$where_sql}
        GROUP BY DATE(dd.reg_date), DATE_FORMAT(dd.reg_date, '%m/%d')
        ORDER BY time_label ASC
        LIMIT 30
      ";
      $viewType = 'daily';
    }
    
    $stmt = $pdo->prepare($sql);
    $executeResult = $stmt->execute($params);
    
    if (!$executeResult) {
      $errorInfo = $stmt->errorInfo();
      error_log("SQL execution failed: " . json_encode($errorInfo, JSON_UNESCAPED_UNICODE));
      try {
        $fallbackSql = "
          SELECT
            DATE_FORMAT(dd.reg_date, '%Y-%m-%d %H:00:00') as time_label,
            DATE_FORMAT(dd.reg_date, '%H:00') as display_label,
            COUNT(*) as total_count,
            SUM(CASE WHEN dd.status = 'Warning' THEN 1 ELSE 0 END) as warning_count,
            COUNT(*) as defective_count
          FROM data_defective dd
          WHERE dd.reg_date >= DATE_SUB(NOW(), INTERVAL 2 DAY)
          GROUP BY DATE_FORMAT(dd.reg_date, '%Y-%m-%d %H:00:00'), DATE_FORMAT(dd.reg_date, '%H:00')
          ORDER BY time_label ASC
          LIMIT 48
        ";
        $fallbackStmt = $pdo->prepare($fallbackSql);
        $fallbackStmt->execute();
        $results = $fallbackStmt->fetchAll(PDO::FETCH_ASSOC);
      } catch (Exception $e) {
        error_log("Fallback query failed: " . $e->getMessage());
        return [];
      }
    } else {
      $results = $stmt->fetchAll(PDO::FETCH_ASSOC);
    }

    // Return results with work hours information for hourly view
    if ($isHourlyView && $workHoursInfo) {
      return [
        'data' => $results,
        'work_hours' => $workHoursInfo,
        'view_type' => 'hourly'
      ];
    }

    return [
      'data' => $results,
      'work_hours' => null,
      'view_type' => $isHourlyView ? 'hourly' : 'daily'
    ];

  } catch (PDOException $e) {
    error_log("Defective rate trend query error: " . $e->getMessage());
    return [
      'data' => [],
      'work_hours' => null,
      'view_type' => 'hourly'
    ];
  }
}

function getMachineDefectiveStats($pdo, $where_sql, $params) {
  try {
    $sql = "
      SELECT 
        dd.machine_no,
        COUNT(*) as total_count,
        SUM(CASE WHEN dd.status = 'Warning' THEN 1 ELSE 0 END) as warning_count,
        ROUND(
          (SUM(CASE WHEN dd.status = 'Warning' THEN 1 ELSE 0 END) / 
           NULLIF(COUNT(*), 0) * 100), 2
        ) as defective_rate
      FROM data_defective dd
      LEFT JOIN info_factory f ON dd.factory_idx = f.idx
      LEFT JOIN info_line l ON dd.line_idx = l.idx  
      {$where_sql}
      GROUP BY dd.machine_no
      HAVING COUNT(*) > 0
      ORDER BY defective_rate DESC, total_count DESC
      LIMIT 10
    ";
    
    $stmt = $pdo->prepare($sql);
    $stmt->execute($params);
    
    $result = $stmt->fetchAll(PDO::FETCH_ASSOC);
    
    return $result;
    
  } catch (PDOException $e) {
    error_log("Machine defective stats query error: " . $e->getMessage());
    return [];
  }
}

function getActiveDefectives($pdo, $where_sql, $params) {
  try {
    $active_where = $where_sql;
    if (empty($active_where)) {
      $active_where = " WHERE dd.status = 'Warning'";
    } else {
      $active_where .= " AND dd.status = 'Warning'";
    }
    
    $sql = "
      SELECT 
        dd.idx,
        dd.machine_no,
        dd.defective_name,
        dd.reg_date,
        f.factory_name,
        l.line_name,
        TIMESTAMPDIFF(SECOND, dd.reg_date, NOW()) as seconds_elapsed,
        CONCAT(
          CASE 
            WHEN TIMESTAMPDIFF(HOUR, dd.reg_date, NOW()) > 0 
            THEN CONCAT(TIMESTAMPDIFF(HOUR, dd.reg_date, NOW()), 'h ')
            ELSE ''
          END,
          CASE 
            WHEN TIMESTAMPDIFF(MINUTE, dd.reg_date, NOW()) % 60 > 0
            THEN CONCAT(TIMESTAMPDIFF(MINUTE, dd.reg_date, NOW()) % 60, 'm ')
            ELSE ''
          END,
          TIMESTAMPDIFF(SECOND, dd.reg_date, NOW()) % 60, 's'
        ) as elapsed_display
      FROM data_defective dd
      LEFT JOIN info_factory f ON dd.factory_idx = f.idx
      LEFT JOIN info_line l ON dd.line_idx = l.idx
      {$active_where}
      ORDER BY dd.reg_date DESC
      LIMIT 5
    ";
    
    $stmt = $pdo->prepare($sql);
    $stmt->execute($params);
    
    return $stmt->fetchAll(PDO::FETCH_ASSOC);
    
  } catch (PDOException $e) {
    error_log("Active defectives query error: " . $e->getMessage());
    return [];
  }
}

function startStreaming($pdo) {
  $lastDataHash = '';
  $startTime = time();
  $maxRunTime = 3600;
  
  $filterConfig = parseFilterParams('dd', 'reg_date', false, '2 DAY');
  $limit = !empty($_GET['limit']) ? (int)$_GET['limit'] : 100;
  
  sendSSEData('connected', [
    'status' => 'connected',
    'message' => 'Defective data streaming started.',
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
        'message' => 'Maximum runtime reached. Please reconnect.'
      ]);
      break;
    }
    
    if (connection_aborted()) {
      error_log("Client connection aborted.");
      break;
    }
    
    try {
      // 성능 측정 시작
      $queryStartTime = microtime(true);
      $performanceLog = [];

      // 1. 메인 Defective 데이터 조회
      $t1 = microtime(true);
      $defectiveData = getDefectiveData($pdo, $filterConfig['where_sql'], $filterConfig['params'], $limit);
      $performanceLog['defectiveData'] = round((microtime(true) - $t1) * 1000, 2) . 'ms';

      // 2. 집계 통계 조회
      $t2 = microtime(true);
      $stats = getDefectiveStats($pdo, $filterConfig['where_sql'], $filterConfig['params']);
      $performanceLog['stats'] = round((microtime(true) - $t2) * 1000, 2) . 'ms';

      // 3. 활성 불량품 조회
      $t3 = microtime(true);
      $activeDefectives = getActiveDefectives($pdo, $filterConfig['where_sql'], $filterConfig['params']);
      $performanceLog['activeDefectives'] = round((microtime(true) - $t3) * 1000, 2) . 'ms';

      // 4. 불량품 유형별 통계 조회
      $t4 = microtime(true);
      $defectiveTypeStats = getDefectiveTypeStats($pdo, $filterConfig['where_sql'], $filterConfig['params']);
      $performanceLog['defectiveTypeStats'] = round((microtime(true) - $t4) * 1000, 2) . 'ms';

      // 5. 불량률 추이 통계 조회
      $t5 = microtime(true);
      $defectiveRateStats = getDefectiveRateStats($pdo, $filterConfig['where_sql'], $filterConfig['params']);
      $performanceLog['defectiveRateStats'] = round((microtime(true) - $t5) * 1000, 2) . 'ms';

      // 6. 기계별 불량품 통계 조회
      $t6 = microtime(true);
      $machineDefectiveStats = getMachineDefectiveStats($pdo, $filterConfig['where_sql'], $filterConfig['params']);
      $performanceLog['machineDefectiveStats'] = round((microtime(true) - $t6) * 1000, 2) . 'ms';

      // 총 쿼리 실행 시간
      $totalQueryTime = round((microtime(true) - $queryStartTime) * 1000, 2);
      $performanceLog['totalQueryTime'] = $totalQueryTime . 'ms';

      // 성능 로그 출력 (느린 쿼리 감지: 1초 이상)
      if ($totalQueryTime > 1000) {
        error_log("⚠️ [Defective] 느린 쿼리 감지 (총 {$totalQueryTime}ms): " . json_encode($performanceLog, JSON_UNESCAPED_UNICODE));
      }
      
      $hashData = [
        'defective_count' => count($defectiveData),
        'defective_ids' => array_column($defectiveData, 'idx'),
        'defective_status_changes' => array_map(function($item) {
          return $item['idx'] . '_' . $item['status']; 
        }, $defectiveData),
        'stats_core' => [
          'total_count' => $stats['total_count'] ?? 0,
          'warning_count' => $stats['warning_count'] ?? 0,
          'affected_machines' => $stats['affected_machines'] ?? 0,
          'defective_rate' => $stats['defective_rate'] ?? '0%'
        ],
        'active_defective_ids' => array_column($activeDefectives, 'idx'),
        'defective_type_stats' => $defectiveTypeStats,
        'defective_rate_stats' => $defectiveRateStats,
        'machine_defective_stats' => $machineDefectiveStats
      ];
      
      $currentDataHash = md5(serialize($hashData));
      
      if ($currentDataHash !== $lastDataHash) {
        $responseData = [
          'timestamp' => date('Y-m-d H:i:s'),
          'stats' => $stats,
          'active_defectives' => $activeDefectives,
          'defective_data' => $defectiveData,
          'defective_type_stats' => $defectiveTypeStats,
          'defective_rate_stats' => $defectiveRateStats,
          'machine_defective_stats' => $machineDefectiveStats,
          'data_count' => count($defectiveData),
          'has_changes' => true
        ];
        
        sendSSEData('defective_data', $responseData);
        $lastDataHash = $currentDataHash;
      } else {
        sendSSEData('heartbeat', [
          'timestamp' => date('Y-m-d H:i:s'),
          'status' => 'no_changes',
          'active_warnings' => $stats['warning_count'] ?? 0
        ]);
      }
      
    } catch (Exception $e) {
      error_log("Streaming error: " . $e->getMessage());
      
      sendSSEData('error', [
        'status' => 'error',
        'message' => 'Error occurred during data query.',
        'timestamp' => date('Y-m-d H:i:s')
      ]);
    }
    
    sleep(5);
  }
}

function getCurrentShiftDefectiveCount($pdo, $selected_shift_idx = null, $target_date = null) {
  try {
    $current_datetime = date('Y-m-d H:i:s');
    $target_date = $target_date ?: date('Y-m-d');
    
    $worktime = new Worktime($pdo);
    
    $factory_idx = ''; // 전체 공장 대상
    $line_idx = '';    // 전체 라인 대상
    
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
      SELECT COUNT(*) as shift_defective_count
      FROM data_defective dd
      WHERE dd.reg_date >= ? AND dd.reg_date < ?
    ";
    
    $stmt = $pdo->prepare($sql);
    $stmt->execute([$shift_start, $shift_end]);
    
    $result = $stmt->fetch(PDO::FETCH_ASSOC);
    
    return (int)($result['shift_defective_count'] ?? 0);
    
  } catch (Exception $e) {
    error_log("Shift defective count query error: " . $e->getMessage());
    return 0;
  }
}

function handleStreamingError($error) {
  $logDir = __DIR__ . '/../../logs';
  if (!is_dir($logDir)) {
    mkdir($logDir, 0777, true);
  }
  
  $logFile = $logDir . '/defective_stream_errors.log';
  $errorMessage = "[" . date("Y-m-d H:i:s") . "] " . $error . "\n";
  error_log($errorMessage, 3, $logFile);
  
  sendSSEData('error', [
    'status' => 'fatal_error',
    'message' => 'Streaming service error occurred.',
    'timestamp' => date('Y-m-d H:i:s')
  ]);
}

// 메인 실행 부분
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
    'message' => 'Defective data streaming ended.',
    'timestamp' => date('Y-m-d H:i:s')
  ]);
}

?>