<?php
// Downtime list retrieval API for OEE monitoring system
// Fetches downtime status list from data_downtime table
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

// Retrieve downtime list for current shift
$items = $apiHelper->getStatusList('downtime', $mac, $machine_data, $work_date, $shift_idx);

// Create response
$response = $apiHelper->createResponse_onlyItems(['items' => $items]);

// Log API call
$apiHelper->logApiCall('logs_api_get_downtimelist', 'get_downtimeList', $machine_no, $mac, $_REQUEST, $response, $today);

/*
## API Endpoint
get_downtimeList.php

## Example Request
http://49.247.26.228/2025/sci/new/api/sewing.php?code=get_downtimeList&mac=84:72:07:50:3A:CC

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
