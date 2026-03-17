<?php
// Defective list retrieval API for OEE monitoring system
// Fetches defective status list from data_defective table
// Required parameter: mac

require_once(__DIR__ . '/../../lib/api_helper.lib.php');

$today = date('Y-m-d H:i:s');
$apiHelper = new ApiHelper($pdo);

// Validate and process MAC address
$mac = $apiHelper->validateAndProcessMac($_REQUEST['mac'] ?? '');

// Get machine information
$machine_data = $apiHelper->getMachineInfo($mac, false);
$factory_idx = $machine_data['factory_idx'];
$line_idx = $machine_data['line_idx'];
$machine_no = $machine_data['machine_no'];

// Get current shift information
$current_shift_info = $apiHelper->getCurrentShiftInfo($factory_idx, $line_idx, $today);
$work_date = $current_shift_info ? $current_shift_info['date'] : null;
$shift_idx = $current_shift_info ? $current_shift_info['shift_idx'] : null;

// Retrieve defective list for current shift
$items = $apiHelper->getStatusList('defective', $mac, $machine_data, $work_date, $shift_idx);

// Create response
$response = $apiHelper->createResponse_onlyItems(['items' => $items]);

// Log API call
$apiHelper->logApiCall('logs_api_get_defectivelist', 'get_defectiveList', $machine_no, $mac, $_REQUEST, $response, $today);

/*
## API Endpoint
get_defectiveList.php

## Example Request
http://49.247.26.228/2025/sci/new/api/sewing.php?code=get_defectiveList&mac=84:72:07:50:3A:CC

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