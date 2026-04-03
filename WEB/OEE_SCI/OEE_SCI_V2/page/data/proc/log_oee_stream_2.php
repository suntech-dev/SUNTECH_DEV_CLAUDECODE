<?php

/**
 * OEE Report Log Streaming API (Server-Sent Events)
 * Streams all columns from data_oee table with summary stats.
 *
 * Optimized (2026-03-07):
 * - Fixed hash bug: removed `|| count($oeeDataLog) > 0` (was sending every 5s regardless of changes)
 * - Reduced default LIMIT 1000 -> 500
 * - Independent try-catch per query block (partial data sent on partial failure)
 * - Performance timing per query block
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

// parseFilterParams(), sendSSEData() → stream_helper.lib.php

function getOeeDataLog($pdo, $where_sql, $params, $limit = 500)
{
    try {
        $sql = "
      SELECT
        do.idx,
        do.work_date,
        do.time_update,
        do.shift_idx,
        do.factory_idx,
        do.factory_name,
        do.line_idx,
        do.line_name,
        do.mac,
        do.machine_idx,
        do.machine_no,
        do.process_name,
        do.planned_work_time,
        do.runtime,
        do.productive_runtime,
        do.downtime,
        do.availabilty_rate,
        do.target_line_per_day,
        do.target_line_per_hour,
        do.target_mc_per_day,
        do.target_mc_per_hour,
        do.cycletime,
        do.pair_info,
        do.pair_count,
        do.theoritical_output,
        do.actual_output,
        do.productivity_rate,
        do.defective,
        do.actual_a_grade,
        do.quality_rate,
        do.oee,
        do.reg_date,
        do.update_date,
        do.work_hour
      FROM data_oee do
      {$where_sql}
      ORDER BY do.work_date DESC, do.machine_no ASC
      LIMIT " . (int)$limit . "
    ";

        $stmt = $pdo->prepare($sql);
        $stmt->execute($params);
        $result = $stmt->fetchAll(PDO::FETCH_ASSOC);

        foreach ($result as &$row) {
            if (isset($row['shift_idx']) && $row['shift_idx'] !== null && $row['shift_idx'] !== '') {
                $row['shift_idx'] = 'Shift ' . $row['shift_idx'];
            }
        }

        return $result;
    } catch (PDOException $e) {
        error_log("OEE data log query error: " . $e->getMessage());
        return [];
    }
}

function getOeeStats($pdo, $where_sql, $params)
{
    try {
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
      {$where_sql}
    ";

        $stmt = $pdo->prepare($sql);
        $stmt->execute($params);
        $stats = $stmt->fetch(PDO::FETCH_ASSOC);

        // Derived rates (PHP calculation avoids extra subqueries)
        $stats['overall_performance_rate'] = $stats['total_theoretical_output'] > 0
            ? round(($stats['total_actual_output'] / $stats['total_theoretical_output']) * 100, 2) : 0;

        $stats['overall_quality_rate'] = $stats['total_actual_output'] > 0
            ? round(($stats['total_good_products'] / $stats['total_actual_output']) * 100, 2) : 0;

        $stats['overall_availability_rate'] = $stats['total_planned_time'] > 0
            ? round(($stats['total_runtime'] / $stats['total_planned_time']) * 100, 2) : 0;

        // Field aliases for stat-card
        $stats['overall_oee']  = $stats['avg_overall_oee'];
        $stats['availability'] = $stats['avg_availability'];
        $stats['performance']  = $stats['avg_performance'];
        $stats['quality']      = $stats['avg_quality'];

        // Current shift OEE and previous day OEE (independent — failures return 0)
        $selected_shift = $_GET['shift_filter'] ?? null;
        $target_date    = $_GET['start_date'] ?? $_GET['end_date'] ?? null;
        $stats['current_shift_oee']  = getCurrentShiftOeeAvg($pdo, $selected_shift, $target_date);
        $stats['previous_day_oee']   = getPreviousDayOeeAvg($pdo, $where_sql, $params);

        return $stats;
    } catch (PDOException $e) {
        error_log("OEE stats query error: " . $e->getMessage());
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
            'overall_oee' => 0,
            'availability' => 0,
            'performance' => 0,
            'quality' => 0
        ];
    }
}

function getPreviousDayOeeAvg($pdo, $where_sql, $params)
{
    try {
        $sql = "
      SELECT
        ROUND((AVG(do.availabilty_rate) * (SUM(do.actual_output) / NULLIF(SUM(do.theoritical_output), 0)) * AVG(do.quality_rate)) / 100, 2) as previous_day_oee,
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

function getCurrentShiftOeeAvg($pdo, $selected_shift_idx = null, $target_date = null)
{
    try {
        $target_date = $target_date ?: date('Y-m-d');
        $worktime    = new Worktime($pdo);
        $factory_idx = '';
        $line_idx    = '';

        if (!empty($selected_shift_idx)) {
            $day_shifts = $worktime->getDayShift($target_date, $factory_idx, $line_idx);

            if (!$day_shifts || !isset($day_shifts['shift'][$selected_shift_idx])) {
                return 0;
            }

            $shift          = $day_shifts['shift'][$selected_shift_idx];
            $work_stime_str = $target_date . ' ' . $shift['available_stime'] . ':00';
            $work_etime_str = $target_date . ' ' . $shift['available_etime'] . ':00';

            if ($shift['over_time']) {
                $work_etime_str = date('Y-m-d H:i:s', strtotime($work_etime_str . ' +' . $shift['over_time'] . ' minutes'));
            }

            if ($work_etime_str <= $work_stime_str) {
                $work_etime_str = date('Y-m-d H:i:s', strtotime($work_etime_str . ' +1 day'));
            }

            $shift_start = $work_stime_str;
        } else {
            $current_shift_info = findCurrentShift($pdo, $worktime, $factory_idx, $line_idx, date('Y-m-d H:i:s'));

            if (!$current_shift_info) return 0;

            $shift_start = $current_shift_info['work_stime'];
        }

        $sql = "
      SELECT ROUND((AVG(availabilty_rate) * (SUM(actual_output) / NULLIF(SUM(theoritical_output), 0)) * AVG(quality_rate)) / 100, 2) as shift_avg_oee
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

function startStreaming($pdo)
{
    $lastDataHash = '';
    $startTime    = time();
    $maxRunTime   = 3600;

    $filterConfig = parseFilterParams('do', 'work_date', true, '7 DAY');
    $limit        = !empty($_GET['limit']) ? (int)$_GET['limit'] : 500;

    sendSSEData('connected', [
        'status'    => 'connected',
        'message'   => 'OEE data log streaming started.',
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

            // 1. Main log data
            $t1 = microtime(true);
            $oeeDataLog = getOeeDataLog($pdo, $filterConfig['where_sql'], $filterConfig['params'], $limit);
            $performanceLog['dataLog'] = round((microtime(true) - $t1) * 1000, 2) . 'ms';

            // 2. Summary stats (independent — failure returns zero-filled array)
            $t2 = microtime(true);
            try {
                $stats = getOeeStats($pdo, $filterConfig['where_sql'], $filterConfig['params']);
            } catch (Exception $statsErr) {
                error_log("Stats query error: " . $statsErr->getMessage());
                $stats = [
                    'total_count' => 0,
                    'avg_overall_oee' => 0,
                    'overall_oee' => 0,
                    'availability' => 0,
                    'performance' => 0,
                    'quality' => 0,
                    'active_machines' => 0,
                    'current_shift_oee' => 0,
                    'previous_day_oee' => 0
                ];
            }
            $performanceLog['stats'] = round((microtime(true) - $t2) * 1000, 2) . 'ms';

            $totalMs = round((microtime(true) - $cycleStart) * 1000, 2);
            $performanceLog['total'] = $totalMs . 'ms';

            if ($totalMs > 1000) {
                error_log("[log_oee] Slow cycle detected ({$totalMs}ms): " . json_encode($performanceLog));
            }

            // Change detection hash
            $hashData = [
                'count'      => count($oeeDataLog),
                'ids'        => array_column($oeeDataLog, 'idx'),
                'oee_values' => array_map(fn($r) => $r['idx'] . '_' . ($r['oee'] ?? '0'), $oeeDataLog),
                'stats_core' => [
                    'avg_overall_oee' => $stats['avg_overall_oee'] ?? 0,
                    'active_machines' => $stats['active_machines'] ?? 0,
                    'total_count'     => $stats['total_count'] ?? 0
                ]
            ];

            $currentHash = md5(serialize($hashData));

            // [FIX] Only send when data actually changed (removed buggy `|| count(...) > 0`)
            if ($currentHash !== $lastDataHash) {
                sendSSEData('oee_data', [
                    'timestamp'   => date('Y-m-d H:i:s'),
                    'stats'       => $stats,
                    'oee_data'    => $oeeDataLog,
                    'data_count'  => count($oeeDataLog),
                    'has_changes' => true,
                    'perf'        => $performanceLog
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

function handleStreamingError($error)
{
    $logDir  = __DIR__ . '/../../logs';
    if (!is_dir($logDir)) mkdir($logDir, 0777, true);
    error_log("[" . date("Y-m-d H:i:s") . "] " . $error . "\n", 3, $logDir . '/log_oee_stream_errors.log');
    sendSSEData('error', ['status' => 'fatal_error', 'message' => 'Streaming service error occurred.', 'timestamp' => date('Y-m-d H:i:s')]);
}

try {
    if (!$pdo) throw new Exception("Database connection failed");
    startStreaming($pdo);
} catch (Exception $e) {
    handleStreamingError($e->getMessage());
} finally {
    sendSSEData('disconnected', ['status' => 'disconnected', 'message' => 'OEE data log streaming ended.', 'timestamp' => date('Y-m-d H:i:s')]);
}
