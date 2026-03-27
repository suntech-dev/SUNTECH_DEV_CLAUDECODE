<?php
/**
 * 재봉기 OEE 시스템 - 메인 API 라우터
 * 모든 API 요청을 받아서 적절한 핸들러로 라우팅합니다.
 * 보안 강화: 화이트리스트 기반 코드 검증
 */

// 필요한 라이브러리 로드
require_once(__DIR__ . '/../lib/db.php');
require_once(__DIR__ . '/../lib/validator.lib.php');
require_once(__DIR__ . '/../lib/worktime.lib.php');
require_once(__DIR__ . '/../lib/get_shift.lib.php');

/**
 * JSON 응답 함수 (개선된 버전)
 * @param array $json 응답 데이터
 * @param int $httpCode HTTP 상태 코드
 */
function jsonReturn($json, $httpCode = 200)
{
    // HTTP 상태 코드 설정
    http_response_code($httpCode);
    
    // 보안 헤더 추가
    header('Content-Type: application/json; charset=utf-8');
    header('X-Content-Type-Options: nosniff');
    header('X-Frame-Options: DENY');
    header('X-XSS-Protection: 1; mode=block');
    
    // JSON 출력 (한글 안전하게)
    echo json_encode($json, JSON_UNESCAPED_UNICODE | JSON_UNESCAPED_SLASHES);
    exit();
}

// 입력값 검증
$code = isset($_REQUEST['code']) ? trim($_REQUEST['code']) : '';

// 빈 코드 체크
if (empty($code)) {
    jsonReturn(array('code' => '99', 'msg' => 'API 코드가 필요합니다.'), 400);
}

// 허용된 API 코드 목록 (화이트리스트 방식으로 보안 강화)
$allowedCodes = [
    'start',                    // 장비 등록
    'get_andonList',           // 안돈 목록 조회
    'get_downtimeList',        // 비가동 목록 조회
    'get_defectiveList',       // 불량 목록 조회
    'get_dateTime',            // 서버 시간 조회
    'send_andon_warning',      // 안돈 경고 전송
    'send_andon_completed',    // 안돈 완료 전송
    'send_downtime_warning',   // 비가동 경고 전송
    'send_downtime_completed', // 비가동 완료 전송
    'send_defective_warning',  // 불량 경고 전송
    'send_pCount',            // 생산수량 전송 (재봉기)
    'send_eCount'             // 자수기 생산데이터 전송
];

// 코드 유효성 검증 (보안: 경로 탐색 공격 방지)
if (!in_array($code, $allowedCodes, true)) {
    // 로그 기록 (보안 이벤트)
    error_log("[SECURITY] 허용되지 않은 API 코드 접근 시도: {$code} from IP: " . ($_SERVER['REMOTE_ADDR'] ?? 'unknown'));
    
    jsonReturn(array(
        'code' => '99', 
        'msg' => '허용되지 않은 API 코드입니다.'
    ), 403);
}

// 파일 경로 생성 (안전한 방식)
$inc_file = __DIR__ . '/sewing/' . $code . '.php';

// 파일 존재 여부 확인
if (!is_file($inc_file)) {
    error_log("[ERROR] API 파일을 찾을 수 없음: {$inc_file}");
    jsonReturn(array(
        'code' => '99', 
        'msg' => 'API 파일을 찾을 수 없습니다.'
    ), 404);
}

// 파일 실행 (try-catch로 에러 처리)
try {
    require_once($inc_file);
    
    // $response 변수가 설정되어 있는지 확인
    if (!isset($response)) {
        jsonReturn(array(
            'code' => '99', 
            'msg' => 'API 응답이 올바르게 설정되지 않았습니다.'
        ), 500);
    }
    
    jsonReturn($response);
    
} catch (Exception $e) {
    // 에러 로깅
    error_log("[ERROR] API 실행 중 오류 발생: " . $e->getMessage());
    
    jsonReturn(array(
        'code' => '99', 
        'msg' => 'API 처리 중 오류가 발생했습니다.'
    ), 500);
}