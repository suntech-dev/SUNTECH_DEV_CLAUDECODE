<?php

/**
 * OEE 모니터링 시스템 — 불량(Defective) 목록 조회 API
 *
 * 현재 교대 근무(시프트)의 불량 항목별 상태 목록을 반환합니다.
 * 불량 항목: NEEDLE(바늘), THREAD(실), BOBBIN(보빈), MATERIAL(소재), TENSION(장력) 등
 *
 * 필수 파라미터: mac (재봉기 MAC 주소)
 * 응답 필드: defective_idx, defective_name(shortcut값), not_completed_qty
 */

// ─────────────────────────────────────────────────────────────────────
// OEE 공통 헬퍼 라이브러리 로드
// ─────────────────────────────────────────────────────────────────────
// Defective list retrieval API for OEE monitoring system
// Fetches defective status list from data_defective table
// Required parameter: mac

require_once(__DIR__ . '/../../lib/api_helper.lib.php');

// 현재 서버 시간 (Y-m-d H:i:s 형식)
$today = date('Y-m-d H:i:s');

// ApiHelper 인스턴스 생성 (PDO 데이터베이스 연결 주입)
$apiHelper = new ApiHelper($pdo);

// ─────────────────────────────────────────────────────────────────────
// 1단계: MAC 주소 검증 및 기계 정보 조회
// getMachineInfo() 두 번째 인자 false = INVENTORY 체크 생략 (목록 조회 허용)
// ─────────────────────────────────────────────────────────────────────
// Validate and process MAC address
$mac = $apiHelper->validateAndProcessMac($_REQUEST['mac'] ?? '');

// Get machine information
$machine_data = $apiHelper->getMachineInfo($mac, false);
$factory_idx = $machine_data['factory_idx']; // 공장 인덱스
$line_idx = $machine_data['line_idx'];       // 라인 인덱스
$machine_no = $machine_data['machine_no'];   // 기계 번호 (로그 기록용)

// ─────────────────────────────────────────────────────────────────────
// 2단계: 현재 교대 근무(시프트) 정보 조회
// 시프트 외 시간대에는 null 반환 → 빈 목록으로 응답
// ─────────────────────────────────────────────────────────────────────
// Get current shift information
$current_shift_info = $apiHelper->getCurrentShiftInfo($factory_idx, $line_idx, $today);
$work_date = $current_shift_info ? $current_shift_info['date'] : null;       // 작업 날짜
$shift_idx = $current_shift_info ? $current_shift_info['shift_idx'] : null; // 시프트 번호

// ─────────────────────────────────────────────────────────────────────
// 3단계: 현재 시프트의 불량 목록 조회
// getStatusList('defective', ...): data_defective 테이블 집계
// - not_completed_qty: 해당 불량 항목의 미완료 경고 건수
//   (같은 불량 유형으로 여러 건 발생 가능)
// ─────────────────────────────────────────────────────────────────────
// Retrieve defective list for current shift
$items = $apiHelper->getStatusList('defective', $mac, $machine_data, $work_date, $shift_idx);

// ─────────────────────────────────────────────────────────────────────
// 4단계: 응답 구성 및 API 호출 로그 저장
// ─────────────────────────────────────────────────────────────────────
// Create response
$response = $apiHelper->createResponse_onlyItems(['items' => $items]);

// Log API call
$apiHelper->logApiCall('logs_api_get_defectivelist', 'get_defectiveList', $machine_no, $mac, $_REQUEST, $response, $today);

/*
## API Endpoint
get_defectiveList.php

## Example Request
http://49.247.26.228/OEE_SCI/OEE_SCI_V2/api/sewing.php?code=get_defectiveList&mac=84:72:07:50:3A:CC

## Example Response
{
  "items": [
    {
      "defective_idx": "1",
      "defective_name": "NEEDLE",
      "not_completed_qty": "0"
    },
    {
      "defective_idx": "2",
      "defective_name": "THREAD",
      "not_completed_qty": "0"
    },
    {
      "defective_idx": "3",
      "defective_name": "BOBBIN",
      "not_completed_qty": "0"
    },
    {
      "defective_idx": "4",
      "defective_name": "MATERIAL",
      "not_completed_qty": "0"
    },
    {
      "defective_idx": "5",
      "defective_name": "TENSION",
      "not_completed_qty": "0"
    }
  ]
}
*/
