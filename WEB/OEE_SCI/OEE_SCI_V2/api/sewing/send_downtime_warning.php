<?php

/**
 * OEE 모니터링 시스템 — 비가동(Downtime) 경고 발생 API
 *
 * 재봉기에서 비가동 버튼을 눌렀을 때 호출됩니다.
 * data_downtime 테이블에 Warning 상태의 새 레코드를 삽입하고,
 * 업데이트된 비가동 목록을 응답으로 반환합니다.
 * 비가동 시간은 OEE 가용성(Availability) 계산에 직접 영향을 줍니다.
 *
 * INVENTORY 머신(line_idx=99)은 데이터를 저장하지 않고 건너뜁니다.
 *
 * 필수 파라미터: mac (MAC 주소), downtime_idx (비가동 항목 번호)
 */

// ─────────────────────────────────────────────────────────────────────
// OEE 공통 헬퍼 라이브러리 로드
// ─────────────────────────────────────────────────────────────────────
// Downtime warning API for OEE monitoring system
// Creates new downtime warning events
// Required parameters: mac, downtime_idx
// Handles work shift validation and data insertion

require_once(__DIR__ . '/../../lib/api_helper.lib.php');

// 현재 서버 시간 (비가동 발생 시각 기록에 사용)
$today = date('Y-m-d H:i:s');

// ApiHelper 인스턴스 생성
$apiHelper = new ApiHelper($pdo);

// ─────────────────────────────────────────────────────────────────────
// 1단계: 요청 파라미터 검증
// ─────────────────────────────────────────────────────────────────────
// Validate and process MAC address
$mac = $apiHelper->validateAndProcessMac($_REQUEST['mac'] ?? '');

// 비가동 항목 번호 수신 및 비가동 정보 조회
// getDowntimeInfo(): info_downtime 테이블에서 항목명·약어 조회
// Validate downtime_idx parameter and get downtime information
$downtime_idx = !empty($_REQUEST['downtime_idx']) ? trim($_REQUEST['downtime_idx']) : '0';
$downtime_info = $apiHelper->getDowntimeInfo($downtime_idx);

$downtime_name     = $downtime_info['downtime_name'];     // 비가동 항목 전체 이름 (예: Setup Time)
$downtime_shortcut = $downtime_info['downtime_shortcut']; // 약어 (예: SETUP)

// ─────────────────────────────────────────────────────────────────────
// 2단계: 기계 정보 조회
// ─────────────────────────────────────────────────────────────────────
// Get machine information
$machine_data = $apiHelper->getMachineInfo($mac, false);

$factory_idx = $machine_data['factory_idx']; // 공장 인덱스
$line_idx = $machine_data['line_idx'];       // 라인 인덱스
$machine_idx = $machine_data['idx'];         // 기계 PK (data_downtime 저장용)
$machine_no = $machine_data['machine_no'];   // 기계 번호 (로그용)
$status = 'Warning';                         // 비가동 발생 상태값

// ─────────────────────────────────────────────────────────────────────
// INVENTORY 머신 체크
// line_idx = 99 (미배정 기기): 데이터 저장하지 않고 성공 응답만 반환
// ─────────────────────────────────────────────────────────────────────
// INVENTORY 머신 체크 (line_idx = 99는 데이터 저장 안 함)
if ($line_idx == 99) {
    jsonReturn(['code' => '00', 'msg' => 'Skipped: Machine in INVENTORY (line_idx=99)']);
}

// ─────────────────────────────────────────────────────────────────────
// 3단계: 현재 시프트 정보 조회
// ─────────────────────────────────────────────────────────────────────
// Get current shift information
$current_shift_info = $apiHelper->getCurrentShiftInfo($factory_idx, $line_idx, $today);
$work_date = $current_shift_info ? $current_shift_info['date'] : null;       // 작업 날짜
$shift_idx = $current_shift_info ? $current_shift_info['shift_idx'] : null; // 시프트 번호

// ─────────────────────────────────────────────────────────────────────
// 4단계: 비가동 경고 데이터 삽입
// insertWarningData('downtime', ...): data_downtime 테이블에 Warning 레코드 INSERT
// ─────────────────────────────────────────────────────────────────────
// Insert downtime warning data
$warning_data = [
    'work_date' => $work_date,               // 작업 날짜
    'shift_idx' => $shift_idx,               // 시프트 번호
    'factory_idx' => $factory_idx,           // 공장 인덱스
    'line_idx' => $line_idx,                 // 라인 인덱스
    'machine_idx' => $machine_idx,           // 기계 PK
    'machine_no' => $machine_no,             // 기계 번호
    'mac' => $mac,                           // MAC 주소
    'downtime_idx' => $downtime_idx,         // 비가동 항목 번호
    'downtime_name' => $downtime_name,       // 비가동 항목 전체명
    'downtime_shortcut' => $downtime_shortcut, // 비가동 약어
    'status' => $status,                     // 상태: 'Warning'
    'reg_date' => $today                     // 비가동 발생 시각
];

$result_warning = $apiHelper->insertWarningData('downtime', $warning_data);

if ($result_warning) {
    // ─────────────────────────────────────────────────────────────────
    // 삽입 성공: 업데이트된 비가동 목록을 응답으로 반환
    // 기기가 화면에 현재 비가동 상태를 즉시 갱신할 수 있도록 목록 포함
    // ─────────────────────────────────────────────────────────────────
    // Retrieve updated downtime list for response
    $items = $apiHelper->getStatusList('downtime', $mac, $machine_data, $work_date, $shift_idx);
    $response = $apiHelper->createResponse_onlyItems(['items' => $items]);
} else {
    jsonReturn(array('code' => '99', 'msg' => 'Failed to insert Warning data'));
}

// ─────────────────────────────────────────────────────────────────────
// 5단계: API 호출 로그 저장
// ─────────────────────────────────────────────────────────────────────
// Log API call
$apiHelper->logApiCall('logs_api_send_downtime_warning', 'send_downtime_warning', $machine_no, $mac, $_REQUEST, $response, $today);

/*
## API Endpoint
send_downtime_warning.php

## Example Request
http://49.247.26.228/OEE_SCI/OEE_SCI_V2/api/sewing.php?code=send_downtime_warning&mac=84:72:07:50:37:73&downtime_idx=1

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
