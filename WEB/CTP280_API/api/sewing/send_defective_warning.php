<?php
// Defective warning API for OEE monitoring system
// Creates new defective warning events
// Required parameters: mac, defective_idx
// Handles work shift validation and data insertion

require_once(__DIR__ . '/../../lib/api_helper.lib.php'); 

$today = date('Y-m-d H:i:s');
$apiHelper = new ApiHelper($pdo);

// Validate and process MAC address
$mac = $apiHelper->validateAndProcessMac($_REQUEST['mac'] ?? '');

// Validate defective_idx parameter
$defective_idx = !empty($_REQUEST['defective_idx']) ? trim($_REQUEST['defective_idx']) : '0';
if ($defective_idx == '0') {
  jsonReturn(array('code' => '99', 'msg' => 'Defective ID is required'));
}

// Get defective information
$stmt = $pdo->prepare("SELECT * FROM info_defective WHERE idx = ?");
$stmt->execute([$defective_idx]);
$defective_data = $stmt->fetch(PDO::FETCH_ASSOC);

if (!$defective_data) {
  jsonReturn(array('code' => '99', 'msg' => 'Invalid defective ID'));
}

$defective_name = $defective_data['defective_name'];
$defective_shortcut = $defective_data['defective_shortcut'];

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

// Insert defective warning data
$defective_data = [
  'work_date' => $work_date,
  'shift_idx' => $shift_idx,
  'factory_idx' => $factory_idx,
  'line_idx' => $line_idx,
  'machine_idx' => $machine_idx,
  'machine_no' => $machine_no,
  'mac' => $mac,
  'defective_idx' => $defective_idx,
  'defective_name' => $defective_name,
  'defective_shortcut' => $defective_shortcut,
  'status' => $status,
  'reg_date' => $today,
  'update_date' => $today
];

$result_warning = $apiHelper->insertWarningData('defective', $defective_data);

if ($result_warning) {
  // Retrieve updated defective list for response
  $items = $apiHelper->getStatusList('defective', $mac, $machine_data, $work_date, $shift_idx);
  $response = $apiHelper->createResponse_onlyItems(['items' => $items]);
} else {
  jsonReturn(array('code' => '99', 'msg' => 'Failed to insert Warning data'));
} 

// Log API call
$apiHelper->logApiCall('logs_api_send_defective_warning', 'send_defective_warning', $machine_no, $mac, $_REQUEST, $response, $today);

/*
## API Endpoint
send_defective_warning.php

## Example Request
http://49.247.26.228/2025/sci/new/api/sewing.php?code=send_defective_warning&mac=84:72:07:50:37:73&defective_idx=1

## Example Response
{
  "items": [
    {
      "defective_idx": "1",
      "defective_shortcut": "NEEDLE",
      "not_completed_qty": "0"
    },
    {
      "defective_idx": "2",
      "defective_shortcut": "THREAD",
      "not_completed_qty": "0"
    },
    {
      "defective_idx": "3",
      "defective_shortcut": "BOBBIN",
      "not_completed_qty": "0"
    },
    {
      "defective_idx": "4",
      "defective_shortcut": "MATERIAL",
      "not_completed_qty": "0"
    },
    {
      "defective_idx": "5",
      "defective_shortcut": "TENSION",
      "not_completed_qty": "0"
    }
  ]
}
*/