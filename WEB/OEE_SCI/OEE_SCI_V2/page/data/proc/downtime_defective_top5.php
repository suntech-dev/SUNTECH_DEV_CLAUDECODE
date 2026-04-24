<?php

/*
 * ============================================================
 * 파일명  : downtime_defective_top5.php
 * 목  적  : AI 대시보드 _2_1 전용 — Downtime Top5 (지속시간 기준) +
 *           Defective Top5 (건수 기준) JSON API
 * Method  : GET
 * Params  : factory_filter, line_filter, machine_filter
 *           date_range: today|yesterday|7d|30d (default: today)
 * Response:
 *   { code:'00',
 *     downtime: [{downtime_name, count, total_duration_min}, …] (5건),
 *     defective: [{defective_name, count}, …] (5건),
 *     date_range }
 * ============================================================
 */

require_once(__DIR__ . '/../../../lib/db.php');
header('Content-Type: application/json; charset=utf-8');

$factory_filter = trim($_GET['factory_filter'] ?? '');
$line_filter    = trim($_GET['line_filter']    ?? '');
$machine_filter = trim($_GET['machine_filter'] ?? '');
$date_range     = trim($_GET['date_range']     ?? 'today');

$today = date('Y-m-d');

switch ($date_range) {
    case 'yesterday':
        $date_where  = 'dd.work_date = ?';
        $date_params = [date('Y-m-d', strtotime('-1 day'))];
        break;
    case '7d':
        $date_where  = 'dd.work_date BETWEEN ? AND ?';
        $date_params = [date('Y-m-d', strtotime('-6 days')), $today];
        break;
    case '30d':
        $date_where  = 'dd.work_date BETWEEN ? AND ?';
        $date_params = [date('Y-m-d', strtotime('-29 days')), $today];
        break;
    default:
        $date_where  = 'dd.work_date = ?';
        $date_params = [$today];
}

function buildFilter(string $factory, string $line, string $machine, string $date_where, array $date_params): array
{
    $clauses = [$date_where];
    $params  = $date_params;
    if ($factory !== '') { $clauses[] = 'dd.factory_idx = ?'; $params[] = $factory; }
    if ($line    !== '') { $clauses[] = 'dd.line_idx = ?';    $params[] = $line; }
    if ($machine !== '') { $clauses[] = 'dd.machine_idx = ?'; $params[] = $machine; }
    return ['where' => 'WHERE ' . implode(' AND ', $clauses), 'params' => $params];
}

try {
    $filter = buildFilter($factory_filter, $line_filter, $machine_filter, $date_where, $date_params);

    // ── Top 5 Downtime (지속시간 내림차순) ───────────────────────
    $dt_sql = "
        SELECT
            dd.downtime_name,
            COUNT(*) AS count,
            ROUND(SUM(COALESCE(dd.duration_sec, 0)) / 60.0, 1) AS total_duration_min
        FROM data_downtime dd
        {$filter['where']}
        GROUP BY dd.downtime_name
        ORDER BY total_duration_min DESC
        LIMIT 5
    ";
    $stmt = $pdo->prepare($dt_sql);
    $stmt->execute($filter['params']);
    $downtime = $stmt->fetchAll(PDO::FETCH_ASSOC);

    while (count($downtime) < 5) {
        $downtime[] = ['downtime_name' => '', 'count' => 0, 'total_duration_min' => 0.0];
    }

    // ── Top 5 Defective (건수 내림차순) ─────────────────────────
    $def_sql = "
        SELECT
            dd.defective_name,
            COUNT(*) AS count
        FROM data_defective dd
        {$filter['where']}
        GROUP BY dd.defective_name
        ORDER BY count DESC
        LIMIT 5
    ";
    $stmt = $pdo->prepare($def_sql);
    $stmt->execute($filter['params']);
    $defective = $stmt->fetchAll(PDO::FETCH_ASSOC);

    while (count($defective) < 5) {
        $defective[] = ['defective_name' => '', 'count' => 0];
    }

    echo json_encode([
        'code'       => '00',
        'downtime'   => $downtime,
        'defective'  => $defective,
        'date_range' => $date_range,
    ]);
} catch (Exception $e) {
    http_response_code(500);
    echo json_encode(['code' => '99', 'message' => $e->getMessage()]);
}
