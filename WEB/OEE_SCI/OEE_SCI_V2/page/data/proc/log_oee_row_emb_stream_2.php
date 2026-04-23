<?php
/**
 * 자수기(EMB) 패킷 스냅샷 로그 SSE 스트리밍
 * 대상 테이블: data_oee_rows_emb (alias: dore)
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

function getEmbRowDataLog($pdo, $where_sql, $params, $limit = 1000)
{
    try {
        $sql = "
            SELECT
                dore.idx, dore.work_date, dore.time_update, dore.shift_idx,
                dore.factory_idx, dore.factory_name, dore.line_idx, dore.line_name,
                dore.mac, dore.machine_idx, dore.machine_no, dore.process_name,
                dore.planned_work_time, dore.runtime, dore.actual_output, dore.packet_qty,
                dore.cycle_time, dore.thread_breakage, dore.motor_run_time,
                dore.pair_info, dore.pair_count,
                dore.work_hour, dore.reg_date
            FROM data_oee_rows_emb dore
            {$where_sql}
            ORDER BY dore.work_date DESC, dore.work_hour DESC, dore.reg_date DESC, dore.idx DESC
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
        error_log("EMB row data log query error: " . $e->getMessage());
        return [];
    }
}

function getEmbRowStats($pdo, $where_sql, $params)
{
    try {
        $sql = "
            SELECT
                COUNT(*)                                              AS total_count,
                COALESCE(SUM(dore.packet_qty), 0)                    AS total_actual_output,
                COALESCE(SUM(dore.thread_breakage), 0)               AS total_thread_breakage,
                COALESCE(ROUND(
                    SUM(dore.motor_run_time) / NULLIF(SUM(dore.runtime), 0) * 100, 1
                ), 0)                                                AS motor_run_rate,
                COALESCE(ROUND(AVG(CASE WHEN dore.cycle_time > 0 THEN dore.cycle_time END), 1), 0) AS avg_cycle_time,
                COUNT(DISTINCT dore.machine_idx)                     AS active_machines,
                COALESCE(SUM(CASE WHEN dore.work_date = CURDATE() THEN dore.packet_qty ELSE 0 END), 0) AS today_output
            FROM data_oee_rows_emb dore
            {$where_sql}";

        $stmt = $pdo->prepare($sql);
        $stmt->execute($params);
        return $stmt->fetch(PDO::FETCH_ASSOC);
    } catch (PDOException $e) {
        error_log("EMB row stats query error: " . $e->getMessage());
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

    $filterConfig = parseFilterParams('dore', 'work_date', true, '7 DAY');
    $limit        = !empty($_GET['limit']) ? (int)$_GET['limit'] : 1000;

    sendSSEData('connected', [
        'status'    => 'connected',
        'message'   => 'EMB row data log streaming started.',
        'timestamp' => date('Y-m-d H:i:s'),
    ]);

    while (true) {
        if (time() - $startTime > $maxRunTime) {
            sendSSEData('timeout', ['status' => 'timeout', 'message' => 'Maximum execution time reached.']);
            break;
        }
        if (connection_aborted()) break;

        try {
            $dataLog = getEmbRowDataLog($pdo, $filterConfig['where_sql'], $filterConfig['params'], $limit);
            $stats   = getEmbRowStats($pdo, $filterConfig['where_sql'], $filterConfig['params']);

            $hashData = [
                'count'  => count($dataLog),
                'ids'    => array_column($dataLog, 'idx'),
                'values' => array_map(fn($r) => $r['idx'] . '_' . ($r['packet_qty'] ?? 0), $dataLog),
                'stats'  => ['total_output' => $stats['total_actual_output'] ?? 0, 'machines' => $stats['active_machines'] ?? 0],
            ];
            $currentHash = md5(serialize($hashData));

            if ($currentHash !== $lastDataHash) {
                sendSSEData('row_emb_data', [
                    'timestamp'   => date('Y-m-d H:i:s'),
                    'stats'       => $stats,
                    'emb_data'    => $dataLog,
                    'data_count'  => count($dataLog),
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
            error_log("EMB row streaming error: " . $e->getMessage());
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
    sendSSEData('disconnected', ['status' => 'disconnected', 'message' => 'EMB row data log streaming ended.', 'timestamp' => date('Y-m-d H:i:s')]);
}
