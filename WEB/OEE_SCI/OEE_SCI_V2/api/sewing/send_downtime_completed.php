<?php

/**
 * OEE 모니터링 시스템 — 비가동(Downtime) 완료 처리 API
 *
 * 비가동 상황이 해소되었을 때 호출됩니다.
 * data_downtime 테이블에서 현재 시프트의 가장 오래된 Warning 레코드를 찾아
 * 상태를 Completed 로 변경하고, 비가동 소요 시간을 기록합니다.
 *
 * ※ 알려진 기기 버그: downtime_idx 파라미터가 항상 "1"로 전송되므로 사용하지 않습니다.
 *    대신 현재 시프트의 첫 번째 Warning 비가동 레코드를 조회하여 완료 처리합니다.
 *
 * 필수 파라미터: mac (MAC 주소)
 */

// ─────────────────────────────────────────────────────────────────────
// OEE 공통 헬퍼 라이브러리 로드
// ─────────────────────────────────────────────────────────────────────
// Downtime completion API for OEE monitoring system
// Updates downtime status from Warning to Completed
// Required parameters: mac, downtime_idx
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

// Note: downtime_idx 파라미터는 디바이스 버그로 항상 "1"을 전송하므로 사용하지 않음.
// 현 근무시간의 첫 번째 Warning downtime을 조회해서 Completed 처리함.

// ─────────────────────────────────────────────────────────────────────
// 2단계: 기계 정보 조회
// ─────────────────────────────────────────────────────────────────────
// Get machine information
$machine_data = $apiHelper->getMachineInfo($mac, false);

$factory_idx = $machine_data['factory_idx']; // 공장 인덱스
$line_idx = $machine_data['line_idx'];       // 라인 인덱스
$machine_no = $machine_data['machine_no'];   // 기계 번호 (로그용)
$prev_status = 'Warning';                   // 조회할 이전 상태값
$update_status = 'Completed';               // 변경할 목표 상태값

// ─────────────────────────────────────────────────────────────────────
// 3단계: 현재 시프트 정보 조회
// ─────────────────────────────────────────────────────────────────────
// Get current shift information
$current_shift_info = $apiHelper->getCurrentShiftInfo($factory_idx, $line_idx, $today);
$work_date = $current_shift_info ? $current_shift_info['date'] : null;       // 작업 날짜
$shift_idx = $current_shift_info ? $current_shift_info['shift_idx'] : null; // 시프트 번호

// ─────────────────────────────────────────────────────────────────────
// 4단계: 가장 오래된 미완료 Warning 비가동 레코드 조회
// downtime_idx 를 조건에 포함하지 않는 것이 핵심 — 기기 버그 우회
// ORDER BY idx ASC LIMIT 1: 가장 먼저 발생한 비가동을 우선 완료 (FIFO)
// ─────────────────────────────────────────────────────────────────────
// Find the earliest Warning status record for this downtime
$stmt = $pdo->prepare(
    "SELECT * FROM data_downtime
   WHERE mac = ? AND status = ? AND work_date = ? AND shift_idx = ?
   ORDER BY idx ASC LIMIT 1"
);
$stmt->execute([$mac, $prev_status, $work_date, $shift_idx]);
$result_warning = $stmt->fetch(PDO::FETCH_ASSOC);

// Warning 레코드가 없으면 오류 응답
if (!$result_warning) {
    jsonReturn(array('code' => '99', 'msg' => 'No Warning status found for this downtime'));
}

$idx = $result_warning['idx']; // 완료 처리할 레코드의 PK

// ─────────────────────────────────────────────────────────────────────
// 5단계: 소요 시간 계산
// calculateDuration(): 비가동 시작(reg_date) ~ 완료(today) 시간 차이 계산
// - $date_diff_sec: 초 단위 (OEE 가용성 계산에 사용)
// - $date_diff    : HH:MM:SS 형식 (사람이 읽기 좋은 표기)
// ─────────────────────────────────────────────────────────────────────
// Calculate duration time between Warning and Completed
[$date_diff_sec, $date_diff] = $apiHelper->calculateDuration($result_warning['reg_date'], $today);

// ─────────────────────────────────────────────────────────────────────
// 6단계: data_downtime 레코드 상태 업데이트
// - status      : Warning → Completed
// - update_date : 완료 처리 시각
// - duration_sec: 비가동 소요 시간 (초) — OEE 가용성 계산에 활용
// - duration_his: 비가동 소요 시간 (HH:MM:SS)
// ─────────────────────────────────────────────────────────────────────
// Update downtime status to Completed
$stmt = $pdo->prepare(
    "UPDATE data_downtime
   SET status = ?, update_date = ?, duration_sec = ?, duration_his = ?
   WHERE idx = ?"
);
$result_completed = $stmt->execute([$update_status, $today, $date_diff_sec, $date_diff, $idx]);

if ($result_completed) {
    // ─────────────────────────────────────────────────────────────────
    // 업데이트 성공: 업데이트된 비가동 목록을 응답으로 반환
    // 기기가 화면에 현재 비가동 상태를 즉시 갱신할 수 있도록 목록 포함
    // ─────────────────────────────────────────────────────────────────
    // Retrieve updated downtime list for response
    $items = $apiHelper->getStatusList('downtime', $mac, $machine_data, $work_date, $shift_idx);
    $response = $apiHelper->createResponse_onlyItems(['items' => $items]);
} else {
    jsonReturn(array('code' => '99', 'msg' => 'Failed to update downtime status'));
}

// ─────────────────────────────────────────────────────────────────────
// 7단계: API 호출 로그 저장
// ─────────────────────────────────────────────────────────────────────
// Log API call
$apiHelper->logApiCall('logs_api_send_downtime_completed', 'send_downtime_completed', $machine_no, $mac, $_REQUEST, $response, $today);

/*
## API Endpoint
send_downtime_completed.php

## Example Request
http://49.247.26.228/OEE_SCI/OEE_SCI_V2/api/sewing.php?code=send_downtime_completed&mac=84:72:07:50:37:73&downtime_idx=1

## Example Response
{
  "code": "00",
  "msg": "Downtime completed successfully"
}
*/
