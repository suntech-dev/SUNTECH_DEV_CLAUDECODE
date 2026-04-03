<?php

/**
 * OEE 모니터링 시스템 — 안돈(Andon) 완료 처리 API
 *
 * 안돈 경고가 해소되었을 때 호출됩니다.
 * data_andon 테이블에서 해당 기계·시프트의 최초 Warning 레코드를 찾아
 * 상태를 Completed 로 변경하고, 경고 발생~완료까지의 소요 시간을 기록합니다.
 *
 * 필수 파라미터: mac (MAC 주소), andon_idx (안돈 항목 번호)
 */

// ─────────────────────────────────────────────────────────────────────
// OEE 공통 헬퍼 라이브러리 로드
// ─────────────────────────────────────────────────────────────────────
// Andon completion API for OEE monitoring system
// Updates andon status from Warning to Completed
// Required parameters: mac, andon_idx
// Calculates duration time between Warning and Completed status

require_once(__DIR__ . '/../../lib/api_helper.lib.php');

// 현재 서버 시간 (완료 처리 시각 기록에 사용)
$today = date('Y-m-d H:i:s');

// ApiHelper 인스턴스 생성
$apiHelper = new ApiHelper($pdo);

// ─────────────────────────────────────────────────────────────────────
// 1단계: 요청 파라미터 검증
// ─────────────────────────────────────────────────────────────────────
// Validate and process MAC address
$mac = $apiHelper->validateAndProcessMac($_REQUEST['mac'] ?? '');

// 안돈 항목 번호 수신
// Validate andon_idx parameter
$andon_idx = !empty($_REQUEST['andon_idx']) ? trim($_REQUEST['andon_idx']) : '0';

// ─────────────────────────────────────────────────────────────────────
// 2단계: 안돈 항목 정보 조회 (유효성 검증 포함)
// 유효하지 않은 andon_idx 이면 내부에서 오류 응답 후 종료
// ─────────────────────────────────────────────────────────────────────
// Get andon information (validates idx and handles errors internally)
$andon_data = $apiHelper->getAndonInfo($andon_idx);

// ─────────────────────────────────────────────────────────────────────
// 3단계: 기계 정보 조회
// ─────────────────────────────────────────────────────────────────────
// Get machine information
$machine_data = $apiHelper->getMachineInfo($mac, false);
$factory_idx = $machine_data['factory_idx']; // 공장 인덱스
$line_idx = $machine_data['line_idx'];       // 라인 인덱스
$machine_no = $machine_data['machine_no'];   // 기계 번호 (로그용)

// 상태 전환: Warning → Completed
$prev_status = 'Warning';
$update_status = 'Completed';

// ─────────────────────────────────────────────────────────────────────
// 4단계: 현재 시프트 정보 조회
// 완료 처리 시각이 경고 발생 시프트와 같은지 확인하기 위해 사용
// ─────────────────────────────────────────────────────────────────────
// Get current shift information
$current_shift_info = $apiHelper->getCurrentShiftInfo($factory_idx, $line_idx, $today);
$work_date = $current_shift_info ? $current_shift_info['date'] : null;       // 작업 날짜
$shift_idx = $current_shift_info ? $current_shift_info['shift_idx'] : null; // 시프트 번호

// ─────────────────────────────────────────────────────────────────────
// 5단계: 미완료 Warning 레코드 조회
// 같은 기계·안돈 항목·시프트에서 Warning 상태인 레코드 중 가장 오래된 것(첫 번째) 선택
// ORDER BY idx ASC LIMIT 1: 먼저 발생한 경고를 우선 완료 처리 (FIFO)
// ─────────────────────────────────────────────────────────────────────
// Find the earliest Warning status record for this andon
$stmt = $pdo->prepare(
    "SELECT * FROM data_andon
   WHERE mac = ? AND andon_idx = ? AND status = ? AND work_date = ? AND shift_idx = ?
   ORDER BY idx ASC LIMIT 1"
);
$stmt->execute([$mac, $andon_idx, $prev_status, $work_date, $shift_idx]);
$result_warning = $stmt->fetch(PDO::FETCH_ASSOC);

// Warning 레코드가 없으면 오류 응답 (이미 완료되었거나 경고 없이 완료 호출)
if (!$result_warning) {
    jsonReturn(array('code' => '99', 'msg' => 'No Warning status found for this andon'));
}

$idx = $result_warning['idx']; // 완료 처리할 레코드의 PK

// ─────────────────────────────────────────────────────────────────────
// 6단계: 소요 시간 계산
// calculateDuration(): 경고 발생 시각(reg_date) ~ 완료 시각(today) 차이 계산
// - $date_diff_sec: 초 단위 소요 시간 (숫자)
// - $date_diff    : 사람이 읽기 좋은 형식 (예: "00:05:23" → 5분 23초)
// ─────────────────────────────────────────────────────────────────────
// Calculate duration time between Warning and Completed
[$date_diff_sec, $date_diff] = $apiHelper->calculateDuration($result_warning['reg_date'], $today);

// ─────────────────────────────────────────────────────────────────────
// 7단계: data_andon 레코드 상태 업데이트
// - status      : Warning → Completed
// - update_date : 완료 처리 시각
// - duration_sec: 소요 시간 (초)
// - duration_his: 소요 시간 (HH:MM:SS 형식)
// ─────────────────────────────────────────────────────────────────────
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

// ─────────────────────────────────────────────────────────────────────
// 8단계: API 호출 로그 저장
// ─────────────────────────────────────────────────────────────────────
// Log API call
$apiHelper->logApiCall('logs_api_send_andon_completed', 'send_andon_completed', $machine_no, $mac, $_REQUEST, $response, $today);

/*
## API Endpoint
send_andon_completed.php

## Example Request
http://49.247.26.228/OEE_SCI/OEE_SCI_V2/api/sewing.php?code=send_andon_completed&mac=84:72:07:50:37:73&andon_idx=1

## Example Response
{
  "code": "00",
  "msg": "Andon completed successfully"
}
*/
