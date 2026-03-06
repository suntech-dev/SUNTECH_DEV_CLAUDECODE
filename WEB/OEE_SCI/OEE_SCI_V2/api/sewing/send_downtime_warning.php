<?php
// Downtime warning API for OEE monitoring system
// Creates new downtime warning events
// Required parameters: mac, downtime_idx
// Handles work shift validation and data insertion

require_once(__DIR__ . '/../../lib/api_helper.lib.php'); 

$today = date('Y-m-d H:i:s');
$apiHelper = new ApiHelper($pdo);

// Validate and process MAC address
$mac = $apiHelper->validateAndProcessMac($_REQUEST['mac'] ?? '');

// Validate downtime_idx parameter
$downtime_idx = !empty($_REQUEST['downtime_idx']) ? trim($_REQUEST['downtime_idx']) : '0';
if ($downtime_idx == '0') {
  jsonReturn(array('code' => '99', 'msg' => 'Downtime ID is required'));
}

// Get downtime information
$stmt = $pdo->prepare("SELECT * FROM info_downtime WHERE idx = ?");
$stmt->execute([$downtime_idx]);
$downtime_data = $stmt->fetch(PDO::FETCH_ASSOC);

if (!$downtime_data) {
  jsonReturn(array('code' => '99', 'msg' => 'Invalid downtime ID'));
}

$downtime_name = $downtime_data['downtime_name'];
$downtime_shortcut = $downtime_data['downtime_shortcut'];

// Get machine information
$machine_data = $apiHelper->getMachineInfo($mac, false);

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
$work_date = $current_shift_info ? $current_shift_info['date'] : null;
$shift_idx = $current_shift_info ? $current_shift_info['shift_idx'] : null;

// Insert downtime warning data
$downtime_data = [
  'work_date' => $work_date,
  'shift_idx' => $shift_idx,
  'factory_idx' => $factory_idx,
  'line_idx' => $line_idx,
  'machine_idx' => $machine_idx,
  'machine_no' => $machine_no,
  'mac' => $mac,
  'downtime_idx' => $downtime_idx,
  'downtime_name' => $downtime_name,
  'downtime_shortcut' => $downtime_shortcut,
  'status' => $status,
  'reg_date' => $today
];

$result_warning = $apiHelper->insertWarningData('downtime', $downtime_data);

if ($result_warning) {
  // Retrieve updated downtime list for response
  $items = $apiHelper->getStatusList('downtime', $mac, $machine_data, $work_date, $shift_idx);
  $response = $apiHelper->createResponse_onlyItems(['items' => $items]);
} else {
  jsonReturn(array('code' => '99', 'msg' => 'Failed to insert Warning data'));
} 

// Log API call
$apiHelper->logApiCall('logs_api_send_downtime_warning', 'send_downtime_warning', $machine_no, $mac, $_REQUEST, $response, $today);

/*
## API Endpoint
send_downtime_warning.php

## Example Request
http://49.247.26.228/2025/sci/new/api/sewing.php?code=send_downtime_warning&mac=84:72:07:50:37:73&downtime_idx=1

## Example Response
{
  "items": [
    {
      "downtime_idx": "1",
      "downtime_shortcut": "6S",
      "not_completed_qty": "0"
    },
    {
      "downtime_idx": "2",
      "downtime_shortcut": "SETUP",
      "not_completed_qty": "0"
    },
    {
      "downtime_idx": "3",
      "downtime_shortcut": "CHANGEOVER",
      "not_completed_qty": "0"
    },
    {
      "downtime_idx": "4",
      "downtime_shortcut": "MEETING",
      "not_completed_qty": "0"
    },
    {
      "downtime_idx": "5",
      "downtime_shortcut": "MATERIAL",
      "not_completed_qty": "0"
    }
  ]
}
*/