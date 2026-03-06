<?php
/**
 * Debug script for log_oee_stream.php
 */

date_default_timezone_set('Asia/Jakarta');

require_once(__DIR__ . '/../../../lib/db.php');
require_once(__DIR__ . '/../../../lib/api_helper.lib.php');
require_once(__DIR__ . '/../../../lib/worktime.lib.php');
require_once(__DIR__ . '/../../../lib/get_shift.lib.php');

header('Content-Type: application/json; charset=utf-8');

// 같은 parseFilterParams 로직
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

try {
  $filterConfig = parseFilterParams();
  $limit = !empty($_GET['limit']) ? (int)$_GET['limit'] : 1000;

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
    {$filterConfig['where_sql']}
    ORDER BY do.work_date DESC, do.update_date DESC, do.idx DESC
    LIMIT " . (int)$limit . "
  ";

  $stmt = $pdo->prepare($sql);
  $stmt->execute($filterConfig['params']);

  $result = $stmt->fetchAll(PDO::FETCH_ASSOC);

  echo json_encode([
    'success' => true,
    'debug_info' => [
      'get_params' => $_GET,
      'where_sql' => $filterConfig['where_sql'],
      'bind_params' => $filterConfig['params'],
      'final_sql' => $sql
    ],
    'count' => count($result),
    'data' => $result
  ], JSON_UNESCAPED_UNICODE | JSON_PRETTY_PRINT);

} catch (Exception $e) {
  echo json_encode([
    'success' => false,
    'error' => $e->getMessage(),
    'trace' => $e->getTraceAsString()
  ], JSON_UNESCAPED_UNICODE | JSON_PRETTY_PRINT);
}
?>
