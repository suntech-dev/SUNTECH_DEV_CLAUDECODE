<?php
/**
 * 재봉기 OEE 시스템 - 메인 API 라우터
 * 모든 API 요청을 받아서 적절한 핸들러로 라우팅합니다.
 * 보안 강화: 화이트리스트 기반 코드 검증
 */

// ─────────────────────────────────────────────────────────────────────
// 공통 라이브러리 로드
// - db.php       : PDO 데이터베이스 연결 및 타임존 설정
// - validator    : 입력값 유효성 검사 유틸리티
// - worktime     : 근무 시간 계산 라이브러리
// - get_shift    : 현재 시프트(교대 근무) 조회 라이브러리
// ─────────────────────────────────────────────────────────────────────
require_once('../lib/db.php');
require_once('../lib/validator.lib.php');
require_once('../lib/worktime.lib.php');
require_once('../lib/get_shift.lib.php');

/**
 * JSON 응답 함수 (개선된 버전)
 *
 * HTTP 상태 코드와 보안 헤더를 함께 전송한 뒤 JSON 데이터를 출력합니다.
 * exit()로 스크립트를 즉시 종료하여 이중 출력을 방지합니다.
 *
 * @param array $json     응답 데이터 배열
 * @param int   $httpCode HTTP 상태 코드 (기본값: 200 OK)
 */
function jsonReturn($json, $httpCode = 200)
{
    // HTTP 상태 코드 설정 (200, 400, 403, 404, 500 등)
    http_response_code($httpCode);

    // ── 보안 헤더 설정 ──────────────────────────────────────
    // Content-Type: JSON 응답임을 명시, UTF-8 인코딩 선언
    header('Content-Type: application/json; charset=utf-8');
    // MIME 타입 스니핑 차단 (브라우저가 임의로 Content-Type을 추측하지 못하게)
    header('X-Content-Type-Options: nosniff');
    // 클릭재킹(Clickjacking) 공격 방지 — iframe 내 삽입 금지
    header('X-Frame-Options: DENY');
    // 구형 브라우저용 XSS 필터 활성화
    header('X-XSS-Protection: 1; mode=block');

    // JSON 출력 (한글 유니코드를 이스케이프하지 않고, 슬래시도 이스케이프하지 않음)
    echo json_encode($json, JSON_UNESCAPED_UNICODE | JSON_UNESCAPED_SLASHES);
    exit();
}

// ─────────────────────────────────────────────────────────────────────
// 입력값 검증: API 코드 추출 및 공백 제거
// GET/POST/COOKIE 모두 수용하는 $_REQUEST 사용
// ─────────────────────────────────────────────────────────────────────
$code = isset($_REQUEST['code']) ? trim($_REQUEST['code']) : '';

// API 코드가 없으면 400 Bad Request 응답
if (empty($code)) {
    jsonReturn(array('code' => '99', 'msg' => 'API 코드가 필요합니다.'), 400);
}

// ─────────────────────────────────────────────────────────────────────
// 허용된 API 코드 화이트리스트
// 화이트리스트 방식: 등록된 코드만 허용하여 경로 탐색(Path Traversal) 공격 차단
// 새 API 핸들러 추가 시 반드시 이 목록에도 등록해야 합니다.
// ─────────────────────────────────────────────────────────────────────
$allowedCodes = [
    'start',                    // 장비 전원 ON 시 초기화 및 등록
    'get_andonList',           // 안돈(이상 신호) 목록 조회
    'get_downtimeList',        // 비가동 항목 목록 조회
    'get_defectiveList',       // 불량 항목 목록 조회
    'get_dateTime',            // 서버 현재 시간 조회
    'send_andon_warning',      // 안돈 경고 발생 전송
    'send_andon_completed',    // 안돈 경고 완료 처리 전송
    'send_downtime_warning',   // 비가동 경고 발생 전송
    'send_downtime_completed', // 비가동 경고 완료 처리 전송
    'send_defective_warning',  // 불량 경고 발생 전송
    'send_pCount'             // 생산 카운트 및 OEE 지표 전송
];

// ─────────────────────────────────────────────────────────────────────
// 화이트리스트 검증 (strict=true: 타입까지 일치 확인)
// 허용되지 않은 코드는 보안 로그를 남기고 403 Forbidden 응답
// ─────────────────────────────────────────────────────────────────────
if (!in_array($code, $allowedCodes, true)) {
    // 보안 이벤트 로그: 서버 error_log에 접근 시도 기록
    error_log("[SECURITY] 허용되지 않은 API 코드 접근 시도: {$code} from IP: " . ($_SERVER['REMOTE_ADDR'] ?? 'unknown'));

    jsonReturn(array(
        'code' => '99',
        'msg' => '허용되지 않은 API 코드입니다.'
    ), 403);
}

// ─────────────────────────────────────────────────────────────────────
// 실행할 PHP 파일 경로 생성
// __DIR__ 사용으로 절대경로 보장 → 상대경로 이동 공격 방지
// 예: code=send_pCount → /path/to/api/sewing/send_pCount.php
// ─────────────────────────────────────────────────────────────────────
$inc_file = __DIR__ . '/sewing/' . $code . '.php';

// 파일 존재 여부 확인 (심볼릭 링크 등 비정상 경로도 차단)
if (!is_file($inc_file)) {
    error_log("[ERROR] API 파일을 찾을 수 없음: {$inc_file}");
    jsonReturn(array(
        'code' => '99',
        'msg' => 'API 파일을 찾을 수 없습니다.'
    ), 404);
}

// ─────────────────────────────────────────────────────────────────────
// API 핸들러 파일 실행
// 각 핸들러 파일은 $response 변수에 결과를 담아야 합니다.
// 예외 발생 시 500 Internal Server Error 응답
// ─────────────────────────────────────────────────────────────────────
try {
    require_once($inc_file);

    // 핸들러가 $response를 설정하지 않은 경우 오류 처리
    if (!isset($response)) {
        jsonReturn(array(
            'code' => '99',
            'msg' => 'API 응답이 올바르게 설정되지 않았습니다.'
        ), 500);
    }

    // 핸들러에서 설정한 $response 를 JSON 으로 반환
    jsonReturn($response);

} catch (\Throwable $e) {
    // 예외 상세 내용은 서버 로그에만 기록 (클라이언트에는 노출 금지)
    error_log("[ERROR] API 실행 중 오류 발생: " . $e->getMessage());

    jsonReturn(array(
        'code' => '99',
        'msg' => 'API 처리 중 오류가 발생했습니다.'
    ), 500);
}
