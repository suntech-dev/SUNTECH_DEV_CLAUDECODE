<?php
// Andon warning API for OEE monitoring system
// Creates new andon warning events
// Required parameters: mac, andon_idx
// Handles work shift validation and data insertion

require_once(__DIR__ . '/../../lib/api_helper.lib.php');

$today = date('Y-m-d H:i:s');
$apiHelper = new ApiHelper($pdo);

// Validate and process MAC address
$mac = $apiHelper->validateAndProcessMac($_REQUEST['mac'] ?? '');

// Validate andon_idx parameter
$andon_idx = !empty($_REQUEST['andon_idx']) ? trim($_REQUEST['andon_idx']) : '0';

// Get andon information
$andon_data = $apiHelper->getAndonInfo($andon_idx);
$andon_name = $andon_data['andon_name'];

// Get machine information
$machine_data = $apiHelper->getMachineInfo($mac, true);

$factory_idx = $machine_data['factory_idx'];
$line_idx = $machine_data['line_idx'];
$machine_idx = $machine_data['idx'];
$machine_no = $machine_data['machine_no'];
$status = 'Warning';

// INVENTORY 머신 체크 (line_idx = 99는 데이터 저장 안 함)
if ($line_idx == 99) {
  jsonReturn(['code' => '00', 'msg' => 'Skipped: Machine in INVENTORY (line_idx=99)']);
}

// Get current shift information
$current_shift_info = $apiHelper->getCurrentShiftInfo($factory_idx, $line_idx, $today);

$work_date = null;
$shift_idx = null;

if ($current_shift_info) {
  // Calculate shift metrics
  $metrics = $apiHelper->calculateShiftMetrics($current_shift_info, $today);
  
  $work_date = $current_shift_info['date'];
  $shift_idx = $current_shift_info['shift_idx'];
}

// Insert warning data
$warning_data = [
  'work_date' => $work_date,
  'shift_idx' => $shift_idx,
  'factory_idx' => $factory_idx,
  'line_idx' => $line_idx,
  'machine_idx' => $machine_idx,
  'machine_no' => $machine_no,
  'mac' => $mac,
  'andon_idx' => $andon_idx,
  'andon_name' => $andon_name,
  'status' => $status,
  'reg_date' => $today
];

$result_warning = $apiHelper->insertWarningData('andon', $warning_data);

if ($result_warning) {
  $response = $apiHelper->createResponse('00', 'Successfully inserted Warning data.');
} else {
  jsonReturn($apiHelper->createResponse('99', 'Failed to insert Warning data'));
}

// Log API call
$apiHelper->logApiCall('logs_api_send_andon_warning', 'send_andon_warning', $machine_no, $mac, $_REQUEST, $response, $today);

/*
## API Endpoint
send_andon_warning.php

## Example Request
http://49.247.26.228/2025/sci/new/api/sewing.php?code=send_andon_warning&mac=84:72:07:50:37:73&andon_idx=1

## Example Response
{
  "code": "00",
  "msg": "Successfully inserted Warning data."
}
*/