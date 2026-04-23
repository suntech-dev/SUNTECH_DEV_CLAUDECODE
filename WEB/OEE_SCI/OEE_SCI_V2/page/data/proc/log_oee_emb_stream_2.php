<?php
/**
 * 자수기(EMB) 일별 집계 로그 SSE 스트리밍
 * 대상 테이블: data_oee_emb (alias: doe)
 */

require_once(__DIR__ . '/../../../lib/db.php');
require_once(__DIR__ . '/../../../lib/api_helper.lib.php');
require_once(__DIR__ . '/../../../lib/stream_helper.lib.php');

header('Content-Type: text/event-stream');
header('Cache-Control: no-cache');
header('Connection: keep-alive');
header('Access-Control-Allow-Origin: *');
header('Access-Control-Allow-Methods: GET');
header('Access-Control-Allow-Headers: Cache-Control');

if (ob_get_level()) ob_end_clean();

function getEmbDataLog($pdo, $where_sql, $params, $limit = 500)
{
    try {
        $sql = "
            SELECT
                doe.idx, doe.work_date, doe.time_update, doe.shift_idx,
                doe.factory_idx, doe.factory_name, doe.line_idx, doe.line_name,
                doe.mac, doe.machine_idx, doe.machine_no, doe.process_name,
                doe.planned_work_time, doe.runtime, doe.actual_output,
                doe.cycle_time, doe.thread_breakage, doe.motor_run_time,
                doe.pair_info, doe.pair_count,
                doe.work_hour, doe.reg_date, doe.update_date
            FROM data_oee_emb doe
            {$where_sql}
            ORDER BY doe.work_date DESC, doe.machine_no ASC
            LIMIT " . (int)$limit;

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
        error_log("EMB data log query error: " . $e->getMessage());
        return [];
    }
}

function getEmbStats($pdo, $where_sql, $params)
{
    try {
        $sql = "
            SELECT
                COUNT(*)                                              AS total_count,
                COALESCE(SUM(doe.actual_output), 0)                  AS total_actual_output,
                COALESCE(SUM(doe.thread_breakage), 0)                AS total_thread_breakage,
                COALESCE(ROUND(
                    SUM(doe.motor_run_time) / NULLIF(SUM(doe.runtime), 0) * 100, 1
                ), 0)                                                AS motor_run_rate,
                COALESCE(ROUND(AVG(CASE WHEN doe.cycle_time > 0 THEN doe.cycle_time END), 1), 0) AS avg_cycle_time,
                COUNT(DISTINCT doe.machine_idx)                      AS active_machines,
                COALESCE(SUM(CASE WHEN doe.work_date = CURDATE() THEN doe.actual_output ELSE 0 END), 0) AS today_output
            FROM data_oee_emb doe
            {$where_sql}";

        $stmt = $pdo->prepare($sql);
        $stmt->execute($params);
        return $stmt->fetch(PDO::FETCH_ASSOC);
    } catch (PDOException $e) {
        error_log("EMB stats query error: " . $e->getMessage());
        return [
            'total_count' => 0, 'total_actual_output' => 0,
            'total_thread_breakage' => 0, 'motor_run_rate' => 0,
            'avg_cycle_time' => 0, 'active_machines' => 0, 'today_output' => 0,
        ];
    }
}

function startStreaming($pdo)
{
    $lastDataHash = '';
    $startTime    = time();
    $maxRunTime   = 3600;

    $filterConfig = parseFilterParams('doe', 'work_date', true, '7 DAY');
    $limit        = !empty($_GET['limit']) ? (int)$_GET['limit'] : 500;

    sendSSEData('connected', [
        'status'    => 'connected',
        'message'   => 'EMB data log streaming started.',
        'timestamp' => date('Y-m-d H:i:s'),
    ]);

    while (true) {
        if (time() - $startTime > $maxRunTime) {
            sendSSEData('timeout', ['status' => 'timeout', 'message' => 'Maximum execution time reached.']);
            break;
        }
        if (connection_aborted()) break;

        try {
            $dataLog = getEmbDataLog($pdo, $filterConfig['where_sql'], $filterConfig['params'], $limit);
            $stats   = getEmbStats($pdo, $filterConfig['where_sql'], $filterConfig['params']);

            $hashData = [
                'count'  => count($dataLog),
                'ids'    => array_column($dataLog, 'idx'),
                'values' => array_map(fn($r) => $r['idx'] . '_' . ($r['actual_output'] ?? 0) . '_' . ($r['thread_breakage'] ?? 0), $dataLog),
                'stats'  => ['total_output' => $stats['total_actual_output'] ?? 0, 'machines' => $stats['active_machines'] ?? 0],
            ];
            $currentHash = md5(serialize($hashData));

            if ($currentHash !== $lastDataHash) {
                sendSSEData('emb_data', [
                    'timestamp'  => date('Y-m-d H:i:s'),
                    'stats'      => $stats,
                    'emb_data'   => $dataLog,
                    'data_count' => count($dataLog),
                    'has_changes' => true,
                ]);
                $lastDataHash = $currentHash;
            } else {
                sendSSEData('heartbeat', [
                    'timestamp'       => date('Y-m-d H:i:s'),
                    'status'          => 'no_changes',
                    'active_machines' => $stats['active_machines'] ?? 0,
                ]);
            }
        } catch (Exception $e) {
            error_log("EMB streaming error: " . $e->getMessage());
            sendSSEData('error', ['status' => 'error', 'message' => 'Data query error.', 'timestamp' => date('Y-m-d H:i:s')]);
        }

        sleep(5);
    }
}

try {
    if (!$pdo) throw new Exception("Database connection failed");
    startStreaming($pdo);
} catch (Exception $e) {
    sendSSEData('error', ['status' => 'fatal_error', 'message' => $e->getMessage(), 'timestamp' => date('Y-m-d H:i:s')]);
} finally {
    sendSSEData('disconnected', ['status' => 'disconnected', 'message' => 'EMB data log streaming ended.', 'timestamp' => date('Y-m-d H:i:s')]);
}
