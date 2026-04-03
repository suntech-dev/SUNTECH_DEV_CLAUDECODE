<?php

/**
 * OEE 모니터링 시스템 — 안돈(Andon) 목록 조회 API
 *
 * 현재 교대 근무(시프트)의 안돈 상태 목록을 반환합니다.
 * 안돈(Andon)은 생산 현장에서 이상 상황(기계 고장, 품질 불량 등)을 알리는 신호 시스템입니다.
 *
 * 필수 파라미터: mac (재봉기 MAC 주소)
 * 응답 필드: andon_idx, andon_name, not_completed_qty, warning_blink
 */

// ─────────────────────────────────────────────────────────────────────
// OEE 공통 헬퍼 라이브러리 로드
// ApiHelper: MAC 검증, 기계 정보 조회, 시프트 조회, 상태 목록 조회 등 제공
// ─────────────────────────────────────────────────────────────────────
// Andon list retrieval API for OEE monitoring system
// Fetches andon status list from data_andon table
// Required parameter: mac

require_once(__DIR__ . '/../../lib/api_helper.lib.php');

// 현재 서버 시간 (Y-m-d H:i:s 형식)
$today = date('Y-m-d H:i:s');

// ApiHelper 인스턴스 생성 (PDO 데이터베이스 연결 주입)
$apiHelper = new ApiHelper($pdo);

// ─────────────────────────────────────────────────────────────────────
// 1단계: MAC 주소 검증 및 기계 정보 조회
// validateAndProcessMac(): 빈 값·형식 오류 처리, 소문자 정규화 등 수행
// getMachineInfo(): info_machine 테이블에서 기계 메타데이터 반환
//   - 두 번째 인자 false = INVENTORY 머신(line_idx=99) 체크 생략
// ─────────────────────────────────────────────────────────────────────
// Validate and process MAC address
$mac = $apiHelper->validateAndProcessMac($_REQUEST['mac'] ?? '');

// Get machine information
$machine_data = $apiHelper->getMachineInfo($mac, false);
$factory_idx = $machine_data['factory_idx']; // 공장 인덱스 (시프트·라인 특정에 사용)
$line_idx = $machine_data['line_idx'];       // 라인 인덱스
$machine_no = $machine_data['machine_no'];   // 기계 번호 (로그 기록용)

// ─────────────────────────────────────────────────────────────────────
// 2단계: 현재 교대 근무(시프트) 정보 조회
// getCurrentShiftInfo(): 현재 시각 기준으로 유효한 시프트를 조회합니다.
// - 근무 시간 외에는 null 반환 가능 → work_date, shift_idx 모두 null 처리
// ─────────────────────────────────────────────────────────────────────
// Get current shift information
$current_shift_info = $apiHelper->getCurrentShiftInfo($factory_idx, $line_idx, $today);
$work_date = $current_shift_info ? $current_shift_info['date'] : null;       // 작업 날짜 (YYYY-MM-DD)
$shift_idx = $current_shift_info ? $current_shift_info['shift_idx'] : null; // 시프트 번호

// ─────────────────────────────────────────────────────────────────────
// 3단계: 현재 시프트의 안돈 목록 조회
// getStatusList('andon', ...): data_andon 테이블에서 안돈 상태 집계
// - not_completed_qty: 아직 Completed 처리되지 않은 Warning 레코드 수
// - warning_blink: 경고 표시등 점멸 여부 (1=점멸 중, 0=정상)
// - work_date·shift_idx 가 null이면 현재 시프트 데이터가 없는 것으로 처리됨
// ─────────────────────────────────────────────────────────────────────
// Retrieve andon list for current shift
$items = $apiHelper->getStatusList('andon', $mac, $machine_data, $work_date, $shift_idx);

// ─────────────────────────────────────────────────────────────────────
// 4단계: 응답 구성 및 API 호출 로그 저장
// createResponse_onlyItems(): { "items": [...] } 형식 응답 배열 생성
// logApiCall(): logs_api_get_andonlist 테이블에 요청·응답 이력 저장
// ─────────────────────────────────────────────────────────────────────
// Create response
$response = $apiHelper->createResponse_onlyItems(['items' => $items]);

// Log API call
$apiHelper->logApiCall('logs_api_get_andonlist', 'get_andonList', $machine_no, $mac, $_REQUEST, $response, $today);

/*
## API Endpoint
get_andonList.php

## Example Request
http://49.247.26.228/OEE_SCI/OEE_SCI_V2/api/sewing.php?code=get_andonList&mac=84:72:07:50:37:73

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
