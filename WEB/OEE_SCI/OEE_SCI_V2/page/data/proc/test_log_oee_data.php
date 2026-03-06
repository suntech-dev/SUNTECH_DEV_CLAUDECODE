<?php
/**
 * Test script to check log_oee_stream.php data
 */

date_default_timezone_set('Asia/Jakarta');

require_once(__DIR__ . '/../../../lib/db.php');

header('Content-Type: application/json; charset=utf-8');

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
    ORDER BY do.work_date DESC, do.update_date DESC, do.idx DESC
    LIMIT 5
  ";

  $stmt = $pdo->prepare($sql);
  $stmt->execute();

  $result = $stmt->fetchAll(PDO::FETCH_ASSOC);

  echo json_encode([
    'success' => true,
    'count' => count($result),
    'data' => $result
  ], JSON_UNESCAPED_UNICODE | JSON_PRETTY_PRINT);

} catch (Exception $e) {
  echo json_encode([
    'success' => false,
    'error' => $e->getMessage()
  ], JSON_UNESCAPED_UNICODE | JSON_PRETTY_PRINT);
}
?>
