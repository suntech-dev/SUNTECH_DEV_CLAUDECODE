<?php

/**
 * ============================================================
 * 파일명  : data_offline_stream_2.php
 * 목  적  : 지정 날짜에 OEE 데이터가 없는 미수신 머신 실시간 SSE
 *
 * 쿼리 구조:
 *   Query 1: 요약 통계 (전체 활성 머신 / 데이터 있는 머신 / 미수신 머신 / 미연결 머신)
 *   Query 2: 미수신 머신 목록 (NOT EXISTS 방식, 가장 오래된 순 정렬)
 *
 * GET params:
 *   factory_filter : 공장 idx 필터
 *   line_filter    : 라인 idx 필터
 *   ref_date       : 기준 날짜 YYYY-MM-DD (기본값: 오늘)
 * ============================================================
 */

require_once(__DIR__ . '/../../../lib/db.php');
require_once(__DIR__ . '/../../../lib/stream_helper.lib.php');

header('Content-Type: text/event-stream');
header('Cache-Control: no-cache');
header('Connection: keep-alive');
header('Access-Control-Allow-Origin: *');
header('Access-Control-Allow-Methods: GET');
header('Access-Control-Allow-Headers: Cache-Control');

if (ob_get_level()) ob_end_clean();

// ---------------------------------------------------------------------------
// 필터 WHERE 절 생성 (info_machine 기준)
// ---------------------------------------------------------------------------
function buildMachineWhere(): array
{
    $clauses = ["m.status = 'Y'", "COALESCE(f.idx, 0) != 99"];
    $params  = [];

    if (!empty($_GET['factory_filter'])) {
        $clauses[] = 'm.factory_idx = ?';
        $params[]  = (int)$_GET['factory_filter'];
    }
    if (!empty($_GET['line_filter'])) {
        $clauses[] = 'm.line_idx = ?';
        $params[]  = (int)$_GET['line_filter'];
    }

    return [
        'where_sql' => 'WHERE ' . implode(' AND ', $clauses),
        'params'    => $params,
    ];
}

// ---------------------------------------------------------------------------
// Query 1: 요약 통계
// ---------------------------------------------------------------------------
function getOfflineStats(PDO $pdo, string $where_sql, array $params, string $refDate): array
{
    $empty = [
        'total_active'       => 0,
        'machines_with_data' => 0,
        'machines_offline'   => 0,
        'never_connected'    => 0,
    ];

    try {
        // 파라미터 순서: [refDate] + where_sql params
        $sql = "
            SELECT
                COUNT(*)                                                                         AS total_active,
                SUM(CASE WHEN td.machine_idx  IS NOT NULL THEN 1 ELSE 0 END)                    AS machines_with_data,
                SUM(CASE WHEN td.machine_idx  IS NULL     THEN 1 ELSE 0 END)                    AS machines_offline,
                SUM(CASE WHEN td.machine_idx  IS NULL AND hist.last_time IS NULL THEN 1 ELSE 0 END) AS never_connected
            FROM info_machine m
            LEFT JOIN info_factory f  ON m.factory_idx = f.idx
            LEFT JOIN info_line    l  ON m.line_idx    = l.idx
            LEFT JOIN (
                SELECT DISTINCT machine_idx FROM data_oee WHERE work_date = ?
            ) td ON m.idx = td.machine_idx
            LEFT JOIN (
                SELECT machine_idx, MAX(update_date) AS last_time
                FROM   data_oee
                GROUP  BY machine_idx
            ) hist ON m.idx = hist.machine_idx
            {$where_sql}
        ";
        $stmt = $pdo->prepare($sql);
        $stmt->execute(array_merge([$refDate], $params));
        return $stmt->fetch(PDO::FETCH_ASSOC) ?: $empty;
    } catch (PDOException $e) {
        error_log("[data_offline] Stats query error: " . $e->getMessage());
        return $empty;
    }
}

// ---------------------------------------------------------------------------
// Query 2: 미수신 머신 목록
// ---------------------------------------------------------------------------
function getOfflineList(PDO $pdo, string $where_sql, array $params, string $refDate): array
{
    try {
        // 파라미터 순서: where_sql params + [refDate for NOT EXISTS]
        $sql = "
            SELECT
                m.idx,
                m.machine_no,
                m.type,
                COALESCE(f.factory_name, '-') AS factory_name,
                COALESCE(l.line_name,    '-') AS line_name,
                hist.last_time                AS last_data_time,
                hist.last_work_date,
                CASE
                    WHEN hist.last_time IS NULL THEN NULL
                    ELSE TIMESTAMPDIFF(MINUTE, hist.last_time, NOW())
                END AS minutes_offline
            FROM info_machine m
            LEFT JOIN info_factory f  ON m.factory_idx = f.idx
            LEFT JOIN info_line    l  ON m.line_idx    = l.idx
            LEFT JOIN (
                SELECT machine_idx,
                       MAX(update_date) AS last_time,
                       MAX(work_date)   AS last_work_date
                FROM   data_oee
                GROUP  BY machine_idx
            ) hist ON m.idx = hist.machine_idx
            {$where_sql}
              AND NOT EXISTS (
                  SELECT 1 FROM data_oee do2
                  WHERE  do2.machine_idx = m.idx
                    AND  do2.work_date   = ?
              )
            ORDER BY
                CASE
                    WHEN hist.last_time IS NULL THEN 999999
                    ELSE TIMESTAMPDIFF(MINUTE, hist.last_time, NOW())
                END DESC,
                f.factory_name ASC,
                l.line_name    ASC,
                m.machine_no   ASC
        ";
        $stmt = $pdo->prepare($sql);
        $stmt->execute(array_merge($params, [$refDate]));
        return $stmt->fetchAll(PDO::FETCH_ASSOC);
    } catch (PDOException $e) {
        error_log("[data_offline] List query error: " . $e->getMessage());
        return [];
    }
}

// ---------------------------------------------------------------------------
// 원인 분류 레이블 (PHP 보조 — JS에서도 동일 로직 사용)
// ---------------------------------------------------------------------------
function classifyCause(?int $minutesOffline): string
{
    if ($minutesOffline === null) return 'never_connected';
    if ($minutesOffline < 60)    return 'today_no_data';
    if ($minutesOffline < 480)   return 'uart_suspect';
    return 'power_off_suspect';
}

// ---------------------------------------------------------------------------
// 스트리밍 메인 루프
// ---------------------------------------------------------------------------
function startStreaming(PDO $pdo): void
{
    $lastHash  = '';
    $startTime = time();
    $maxRun    = 3600;

    $filter  = buildMachineWhere();
    $refDate = !empty($_GET['ref_date']) ? $_GET['ref_date'] : date('Y-m-d');

    sendSSEData('connected', [
        'status'    => 'connected',
        'message'   => 'Offline machine monitoring started.',
        'timestamp' => date('Y-m-d H:i:s'),
        'ref_date'  => $refDate,
        'filters'   => [
            'factory_filter' => $_GET['factory_filter'] ?? null,
            'line_filter'    => $_GET['line_filter']    ?? null,
        ],
    ]);

    while (true) {
        if (time() - $startTime > $maxRun) {
            sendSSEData('timeout', ['status' => 'timeout', 'message' => 'Maximum execution time reached. Please reconnect.']);
            break;
        }
        if (connection_aborted()) break;

        try {
            $t0 = microtime(true);

            $stats = getOfflineStats($pdo, $filter['where_sql'], $filter['params'], $refDate);
            $list  = getOfflineList($pdo,  $filter['where_sql'], $filter['params'], $refDate);

            // 원인 레이블 보강
            foreach ($list as &$row) {
                $min = ($row['minutes_offline'] !== null) ? (int)$row['minutes_offline'] : null;
                $row['cause'] = classifyCause($min);
            }
            unset($row);

            $totalMs = round((microtime(true) - $t0) * 1000, 2);

            $hash = md5(serialize([
                'stats' => $stats,
                'ids'   => array_column($list, 'idx'),
                'times' => array_column($list, 'last_data_time'),
            ]));

            if ($hash !== $lastHash) {
                sendSSEData('offline_data', [
                    'timestamp'   => date('Y-m-d H:i:s'),
                    'ref_date'    => $refDate,
                    'stats'       => $stats,
                    'list'        => $list,
                    'list_count'  => count($list),
                    'has_changes' => true,
                    'query_ms'    => $totalMs,
                ]);
                $lastHash = $hash;
            } else {
                sendSSEData('heartbeat', [
                    'timestamp' => date('Y-m-d H:i:s'),
                    'status'    => 'no_changes',
                ]);
            }
        } catch (Exception $e) {
            error_log("[data_offline] Stream error: " . $e->getMessage());
            sendSSEData('error', [
                'status'    => 'error',
                'message'   => 'Data query error occurred.',
                'timestamp' => date('Y-m-d H:i:s'),
            ]);
        }

        sleep(30); // 진단 목적 → 30초 간격
    }
}

// ---------------------------------------------------------------------------
// 진입점
// ---------------------------------------------------------------------------
try {
    if (!$pdo) throw new Exception("Database connection failed");
    startStreaming($pdo);
} catch (Exception $e) {
    sendSSEData('error', [
        'status'    => 'fatal_error',
        'message'   => $e->getMessage(),
        'timestamp' => date('Y-m-d H:i:s'),
    ]);
} finally {
    sendSSEData('disconnected', [
        'status'    => 'disconnected',
        'message'   => 'Offline machine monitoring ended.',
        'timestamp' => date('Y-m-d H:i:s'),
    ]);
}
