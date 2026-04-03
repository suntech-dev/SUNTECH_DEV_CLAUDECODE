<?php

/**
 * OEE 모니터링 시스템 — 안돈(Andon) 경고 발생 API
 *
 * 재봉기에서 안돈 버튼을 눌렀을 때 호출됩니다.
 * data_andon 테이블에 Warning 상태의 새 레코드를 삽입합니다.
 * INVENTORY 머신(line_idx=99)은 데이터를 저장하지 않고 건너뜁니다.
 *
 * 필수 파라미터: mac (MAC 주소), andon_idx (안돈 항목 번호)
 */

// ─────────────────────────────────────────────────────────────────────
// OEE 공통 헬퍼 라이브러리 로드
// ─────────────────────────────────────────────────────────────────────
// Andon warning API for OEE monitoring system
// Creates new andon warning events
// Required parameters: mac, andon_idx
// Handles work shift validation and data insertion

require_once(__DIR__ . '/../../lib/api_helper.lib.php');

// 현재 서버 시간 (경고 발생 시각 기록에 사용)
$today = date('Y-m-d H:i:s');

// ApiHelper 인스턴스 생성
$apiHelper = new ApiHelper($pdo);

// ─────────────────────────────────────────────────────────────────────
// 1단계: 요청 파라미터 검증
// ─────────────────────────────────────────────────────────────────────
// Validate and process MAC address
$mac = $apiHelper->validateAndProcessMac($_REQUEST['mac'] ?? '');

// 안돈 항목 번호 수신 (빈 값이면 '0'으로 처리 → 내부에서 오류 반환)
// Validate andon_idx parameter
$andon_idx = !empty($_REQUEST['andon_idx']) ? trim($_REQUEST['andon_idx']) : '0';

// ─────────────────────────────────────────────────────────────────────
// 2단계: 안돈 항목 정보 조회
// getAndonInfo(): info_andon 테이블에서 andon_idx에 해당하는 이름 등 조회
// 유효하지 않은 idx면 내부에서 오류 응답 후 종료
// ─────────────────────────────────────────────────────────────────────
// Get andon information
$andon_data = $apiHelper->getAndonInfo($andon_idx);
$andon_name = $andon_data['andon_name']; // 안돈 항목명 (예: Machine, Process, Quality)

// ─────────────────────────────────────────────────────────────────────
// 3단계: 기계 정보 조회
// getMachineInfo() 두 번째 인자 true = INVENTORY 머신이면 내부에서 처리 가능
// ─────────────────────────────────────────────────────────────────────
// Get machine information
$machine_data = $apiHelper->getMachineInfo($mac, true);

$factory_idx = $machine_data['factory_idx']; // 공장 인덱스
$line_idx = $machine_data['line_idx'];       // 라인 인덱스
$machine_idx = $machine_data['idx'];         // 기계 PK (data_andon 저장용)
$machine_no = $machine_data['machine_no'];   // 기계 번호 (로그용)
$status = 'Warning';                         // 경고 발생 상태값

// ─────────────────────────────────────────────────────────────────────
// INVENTORY 머신 체크
// line_idx = 99: 아직 라인에 배정되지 않은 미배정 기기
// 미배정 기기의 데이터는 저장하지 않고 성공 응답만 반환
// ─────────────────────────────────────────────────────────────────────
// INVENTORY 머신 체크 (line_idx = 99는 데이터 저장 안 함)
if ($line_idx == 99) {
    jsonReturn(['code' => '00', 'msg' => 'Skipped: Machine in INVENTORY (line_idx=99)']);
}

// ─────────────────────────────────────────────────────────────────────
// 4단계: 현재 시프트 정보 조회
// 경고 데이터를 올바른 work_date·shift_idx 와 연결하기 위해 필요
// 시프트 외 시간에는 null 값으로 저장
// ─────────────────────────────────────────────────────────────────────
// Get current shift information
$current_shift_info = $apiHelper->getCurrentShiftInfo($factory_idx, $line_idx, $today);

$work_date = null;
$shift_idx = null;

if ($current_shift_info) {
    // 시프트 지표 계산 (경과 시간, 계획 근무 시간 등) — 현재는 참조용
    // Calculate shift metrics
    $metrics = $apiHelper->calculateShiftMetrics($current_shift_info, $today);

    $work_date = $current_shift_info['date'];       // 작업 날짜
    $shift_idx = $current_shift_info['shift_idx'];  // 시프트 번호
}

// ─────────────────────────────────────────────────────────────────────
// 5단계: 경고 데이터 삽입
// insertWarningData('andon', ...): data_andon 테이블에 Warning 레코드 INSERT
// - work_date·shift_idx 가 null 이면 시프트 외 시간대 경고로 기록됨
// ─────────────────────────────────────────────────────────────────────
// Insert warning data
$warning_data = [
    'work_date' => $work_date,       // 작업 날짜 (시프트 기준)
    'shift_idx' => $shift_idx,       // 시프트 번호
    'factory_idx' => $factory_idx,   // 공장 인덱스
    'line_idx' => $line_idx,         // 라인 인덱스
    'machine_idx' => $machine_idx,   // 기계 PK
    'machine_no' => $machine_no,     // 기계 번호
    'mac' => $mac,                   // MAC 주소
    'andon_idx' => $andon_idx,       // 안돈 항목 번호
    'andon_name' => $andon_name,     // 안돈 항목명
    'status' => $status,             // 상태: 'Warning'
    'reg_date' => $today             // 경고 발생 시각
];

$result_warning = $apiHelper->insertWarningData('andon', $warning_data);

if ($result_warning) {
    // 삽입 성공
    $response = $apiHelper->createResponse('00', 'Successfully inserted Warning data.');
} else {
    jsonReturn($apiHelper->createResponse('99', 'Failed to insert Warning data'));
}

// ─────────────────────────────────────────────────────────────────────
// 6단계: API 호출 로그 저장
// ─────────────────────────────────────────────────────────────────────
// Log API call
$apiHelper->logApiCall('logs_api_send_andon_warning', 'send_andon_warning', $machine_no, $mac, $_REQUEST, $response, $today);

/*
## API Endpoint
send_andon_warning.php

## Example Request
http://49.247.26.228/OEE_SCI/OEE_SCI_V2/api/sewing.php?code=send_andon_warning&mac=84:72:07:50:37:73&andon_idx=1

## Example Response
{
  "code": "00",
  "msg": "Successfully inserted Warning data."
}
*/
