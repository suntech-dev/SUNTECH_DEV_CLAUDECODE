<?php

/**
 * OEE 모니터링 시스템 — 비가동(Downtime) 목록 조회 API
 *
 * 현재 교대 근무(시프트)의 비가동 항목별 상태 목록을 반환합니다.
 * 비가동 유형 예시: 6S(정리정돈), SETUP(설정), CHANGEOVER(품종교체), MEETING(회의), MATERIAL(소재 부족)
 * 비가동 시간은 OEE 가용성(Availability) 계산에 직접 영향을 줍니다.
 *
 * 필수 파라미터: mac (재봉기 MAC 주소)
 * 응답 필드: downtime_idx, downtime_name(shortcut값), not_completed_qty
 */

// ─────────────────────────────────────────────────────────────────────
// OEE 공통 헬퍼 라이브러리 로드
// ─────────────────────────────────────────────────────────────────────
// Downtime list retrieval API for OEE monitoring system
// Fetches downtime status list from data_downtime table
// Required parameter: mac

require_once(__DIR__ . '/../../lib/api_helper.lib.php');

// 현재 서버 시간 (Y-m-d H:i:s 형식)
$today = date('Y-m-d H:i:s');

// ApiHelper 인스턴스 생성 (PDO 데이터베이스 연결 주입)
$apiHelper = new ApiHelper($pdo);

// ─────────────────────────────────────────────────────────────────────
// 1단계: MAC 주소 검증 및 기계 정보 조회
// getMachineInfo() 두 번째 인자 false = INVENTORY 체크 생략
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
// ─────────────────────────────────────────────────────────────────────
// Get current shift information
$current_shift_info = $apiHelper->getCurrentShiftInfo($factory_idx, $line_idx, $today);
$work_date = $current_shift_info ? $current_shift_info['date'] : null;       // 작업 날짜
$shift_idx = $current_shift_info ? $current_shift_info['shift_idx'] : null; // 시프트 번호

// ─────────────────────────────────────────────────────────────────────
// 3단계: 현재 시프트의 비가동 목록 조회
// getStatusList('downtime', ...): data_downtime 테이블 집계
// - not_completed_qty: 아직 완료 처리되지 않은 비가동 경고 건수
//   (진행 중인 비가동이 있으면 1 이상의 값)
// ─────────────────────────────────────────────────────────────────────
// Retrieve downtime list for current shift
$items = $apiHelper->getStatusList('downtime', $mac, $machine_data, $work_date, $shift_idx);

// ─────────────────────────────────────────────────────────────────────
// 4단계: 응답 구성 및 API 호출 로그 저장
// ─────────────────────────────────────────────────────────────────────
// Create response
$response = $apiHelper->createResponse_onlyItems(['items' => $items]);

// Log API call
$apiHelper->logApiCall('logs_api_get_downtimelist', 'get_downtimeList', $machine_no, $mac, $_REQUEST, $response, $today);

/*
## API Endpoint
get_downtimeList.php

## Example Request
http://49.247.26.228/OEE_SCI/OEE_SCI_V2/api/sewing.php?code=get_downtimeList&mac=84:72:07:50:3A:CC

## Example Response
{
  "items": [
    {
      "downtime_idx": "1",
      "downtime_name": "6S",
      "not_completed_qty": "0"
    },
    {
      "downtime_idx": "2",
      "downtime_name": "SETUP",
      "not_completed_qty": "0"
    },
    {
      "downtime_idx": "3",
      "downtime_name": "CHANGEOVER",
      "not_completed_qty": "0"
    },
    {
      "downtime_idx": "4",
      "downtime_name": "MEETING",
      "not_completed_qty": "0"
    },
    {
      "downtime_idx": "5",
      "downtime_name": "MATERIAL",
      "not_completed_qty": "0"
    }
  ]
}
*/
