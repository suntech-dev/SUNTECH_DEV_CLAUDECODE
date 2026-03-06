<?php
// Andon list retrieval API for OEE monitoring system
// Fetches andon status list from data_andon table
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

// Retrieve andon list for current shift
$items = $apiHelper->getStatusList('andon', $mac, $machine_data, $work_date, $shift_idx);

// Create response
$response = $apiHelper->createResponse_onlyItems(['items' => $items]);

// Log API call
$apiHelper->logApiCall('logs_api_get_andonlist', 'get_andonList', $machine_no, $mac, $_REQUEST, $response, $today);

/*
## API Endpoint
get_andonList.php

## Example Request
http://49.247.26.228/2025/sci/new/api/sewing.php?code=get_andonList&mac=84:72:07:50:37:73

## Example Response
{
  "items": [
    {
      "andon_idx": "1",
      "andon_name": "Machine",
      "not_completed_qty": "0",
      "warning_blink": "0"
    },
    {
      "andon_idx": "2",
      "andon_name": "Process",
      "not_completed_qty": "0",
      "warning_blink": "0"
    },
    {
      "andon_idx": "3",
      "andon_name": "Quality",
      "not_completed_qty": "0",
      "warning_blink": "0"
    }
  ]
}
*/
