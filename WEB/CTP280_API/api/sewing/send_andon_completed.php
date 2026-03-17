<?php
// Andon completion API for OEE monitoring system
// Updates andon status from Warning to Completed
// Required parameters: mac, andon_idx
// Calculates duration time between Warning and Completed status

require_once(__DIR__ . '/../../lib/api_helper.lib.php');

$today = date('Y-m-d H:i:s');
$apiHelper = new ApiHelper($pdo);

// Validate and process MAC address
$mac = $apiHelper->validateAndProcessMac($_REQUEST['mac'] ?? '');

// Validate andon_idx parameter
$andon_idx = !empty($_REQUEST['andon_idx']) ? trim($_REQUEST['andon_idx']) : '0';
if ($andon_idx == '0') {
  jsonReturn(array('code' => '99', 'msg' => 'Andon ID is required'));
}

// Get andon information
$stmt = $pdo->prepare("SELECT * FROM info_andon WHERE idx = ?");
$stmt->execute([$andon_idx]);
$andon_data = $stmt->fetch(PDO::FETCH_ASSOC);

if (!$andon_data) {
  jsonReturn(array('code' => '99', 'msg' => 'Invalid andon ID'));
}

// Get machine information
$machine_data = $apiHelper->getMachineInfo($mac, false);
$factory_idx = $machine_data['factory_idx'];
$line_idx = $machine_data['line_idx'];
$machine_no = $machine_data['machine_no'];

$prev_status = 'Warning';
$update_status = 'Completed';

// Get current shift information
$current_shift_info = $apiHelper->getCurrentShiftInfo($factory_idx, $line_idx, $today);
$work_date = $current_shift_info ? $current_shift_info['date'] : null;
$shift_idx = $current_shift_info ? $current_shift_info['shift_idx'] : null;

// Find the earliest Warning status record for this andon
$stmt = $pdo->prepare(
  "SELECT * FROM data_andon 
   WHERE mac = ? AND andon_idx = ? AND status = ? AND work_date = ? AND shift_idx = ? 
   ORDER BY idx ASC LIMIT 1"
);
$stmt->execute([$mac, $andon_idx, $prev_status, $work_date, $shift_idx]);
$result_warning = $stmt->fetch(PDO::FETCH_ASSOC);

if (!$result_warning) {
  jsonReturn(array('code' => '99', 'msg' => 'No Warning status found for this andon'));
}

$idx = $result_warning['idx'];

// Calculate duration time between Warning and Completed
$date_diff = '';
$date_diff_sec = 0;

if ($result_warning['reg_date'] && $result_warning['reg_date'] != '0000-00-00 00:00:00') {
  $diff = strtotime($today) - strtotime($result_warning['reg_date']);
  $date_diff_sec = $diff;
  
  $hour = floor($diff / 3600);
  if ($hour > 0) $date_diff .= $hour . 'h ';
  
  $min = floor(($diff % 3600) / 60);
  if ($min > 0) $date_diff .= $min . 'm ';
  
  $sec = $diff % 60;
  if ($sec > 0) $date_diff .= $sec . 's ';
}

// Update andon status to Completed
$stmt = $pdo->prepare(
  "UPDATE data_andon 
   SET status = ?, update_date = ?, duration_sec = ?, duration_his = ? 
   WHERE idx = ?"
);
$result_completed = $stmt->execute([$update_status, $today, $date_diff_sec, $date_diff, $idx]);

if ($result_completed) {
  $response = array('code' => '00', 'msg' => 'Andon completed successfully');
} else {
  jsonReturn(array('code' => '99', 'msg' => 'Failed to update andon status'));
}

// Log API call
$apiHelper->logApiCall('logs_api_send_andon_completed', 'send_andon_completed', $machine_no, $mac, $_REQUEST, $response, $today);

/*
## API Endpoint
send_andon_completed.php

## Example Request
http://49.247.26.228/2025/sci/new/api/sewing.php?code=send_andon_completed&mac=84:72:07:50:37:73&andon_idx=1

## Example Response
{
  "code": "00",
  "msg": "Andon completed successfully"
}
*/