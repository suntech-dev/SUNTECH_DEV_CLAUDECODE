<?php
/**
 * 자수기 OEE 시스템 - 메인 API 라우터
 * 자수기(type='E') 전용 API 요청을 받아서 적절한 핸들러로 라우팅합니다.
 * 보안 강화: 화이트리스트 기반 코드 검증
 *
 * 자수기 전용 핸들러: api/embroidery/ 폴더
 * 공통 핸들러(안돈/비가동/불량 등): api/sewing/ 폴더 파일 공유
 */

// ─────────────────────────────────────────────────────────────────────
// 공통 라이브러리 로드
// ─────────────────────────────────────────────────────────────────────
require_once('../lib/db.php');
require_once('../lib/validator.lib.php');
require_once('../lib/worktime.lib.php');
require_once('../lib/get_shift.lib.php');

/**
 * JSON 응답 함수
 */
function jsonReturn($json, $httpCode = 200)
{
    http_response_code($httpCode);
    header('Content-Type: application/json; charset=utf-8');
    header('X-Content-Type-Options: nosniff');
    header('X-Frame-Options: DENY');
    header('X-XSS-Protection: 1; mode=block');
    echo json_encode($json, JSON_UNESCAPED_UNICODE | JSON_UNESCAPED_SLASHES);
    exit();
}

// ─────────────────────────────────────────────────────────────────────
// 입력값 검증: API 코드 추출
// ─────────────────────────────────────────────────────────────────────
$code = isset($_REQUEST['code']) ? trim($_REQUEST['code']) : '';

if (empty($code)) {
    jsonReturn(array('code' => '99', 'msg' => 'API 코드가 필요합니다.'), 400);
}

// ─────────────────────────────────────────────────────────────────────
// 허용된 API 코드 화이트리스트
// 자수기 전용: start, send_eCount
// 공통(sewing/ 폴더 공유): 나머지
// ─────────────────────────────────────────────────────────────────────
$allowedCodes = [
    'start',                    // 장비 전원 ON 시 초기화 및 등록 (자수기 전용 — type='E')
    'get_andonList',            // 안돈 목록 조회
    'get_downtimeList',         // 비가동 항목 목록 조회
    'get_defectiveList',        // 불량 항목 목록 조회
    'get_dateTime',             // 서버 현재 시간 조회 (AUTO RESET 교대 기반)
    'send_andon_warning',       // 안돈 경고 발생 전송
    'send_andon_completed',     // 안돈 경고 완료 처리 전송
    'send_downtime_warning',    // 비가동 경고 발생 전송
    'send_downtime_completed',  // 비가동 경고 완료 처리 전송
    'send_defective_warning',   // 불량 경고 발생 전송
    'send_eCount',              // 자수기 생산 카운트 및 OEE 지표 전송
];

if (!in_array($code, $allowedCodes, true)) {
    error_log("[SECURITY] 허용되지 않은 자수기 API 코드 접근 시도: {$code} from IP: " . ($_SERVER['REMOTE_ADDR'] ?? 'unknown'));
    jsonReturn(array('code' => '99', 'msg' => '허용되지 않은 API 코드입니다.'), 403);
}

// ─────────────────────────────────────────────────────────────────────
// 핸들러 파일 경로 결정
// 1순위: api/embroidery/{code}.php (자수기 전용)
// 2순위: api/sewing/{code}.php     (공통 핸들러 공유)
// ─────────────────────────────────────────────────────────────────────
$embroidery_file = __DIR__ . '/embroidery/' . $code . '.php';
$sewing_file     = __DIR__ . '/sewing/'     . $code . '.php';

if (is_file($embroidery_file)) {
    $inc_file = $embroidery_file;
} elseif (is_file($sewing_file)) {
    $inc_file = $sewing_file;
} else {
    error_log("[ERROR] 자수기 API 파일을 찾을 수 없음: {$code}");
    jsonReturn(array('code' => '99', 'msg' => 'API 파일을 찾을 수 없습니다.'), 404);
}

// ─────────────────────────────────────────────────────────────────────
// API 핸들러 파일 실행
// ─────────────────────────────────────────────────────────────────────
try {
    require_once($inc_file);

    if (!isset($response)) {
        jsonReturn(array('code' => '99', 'msg' => 'API 응답이 올바르게 설정되지 않았습니다.'), 500);
    }

    jsonReturn($response);

} catch (\Throwable $e) {
    error_log("[ERROR] 자수기 API 실행 중 오류 발생: " . $e->getMessage());
    jsonReturn(array('code' => '99', 'msg' => 'API 처리 중 오류가 발생했습니다.'), 500);
}
