<?php

/**
 * ============================================================
 * 파일명: export_common.php
 * 목  적: 모든 Excel 내보내기 파일(*_export_2.php)에서 공통으로
 *         사용하는 초기화 및 필터 WHERE 절 빌더를 제공한다.
 *
 * 주요 역할:
 *  1) PHP 에러를 JSON 형태로 출력해 클라이언트가 파싱할 수 있도록 처리
 *  2) DB 연결(db.php) 및 PhpSpreadsheet 라이브러리(autoload.php) 로드
 *  3) buildExportFilterParams() 함수로 공장/라인/기계/교대/날짜 필터를
 *     PDO 바인딩 파라미터 배열과 SQL WHERE 절로 변환
 *
 * 사용법:
 *   require_once __DIR__ . '/export_common.php';
 *   // work_date 기준, 기본값 오늘
 *   $filter = buildExportFilterParams('do');
 *   // reg_date 기준, 날짜 미지정 시 최근 2일
 *   $filter = buildExportFilterParams('da', 'reg_date', true, "da.reg_date >= DATE_SUB(NOW(), INTERVAL 2 DAY)");
 * ============================================================
 */

// HTML 에러 출력 방지 — Excel 바이너리 스트림에 HTML 오류가 섞이지 않도록 설정
ini_set('display_errors', 0);
error_reporting(E_ALL);

/* PHP 런타임 에러가 발생했을 때 JSON 형태로 응답하는 커스텀 에러 핸들러 등록
   - 클라이언트 JavaScript에서 JSON.parse() 로 파싱 가능하게 한다. */
set_error_handler(function (int $severity, string $message) {
    if (!(error_reporting() & $severity)) return;
    http_response_code(500);
    echo json_encode(['success' => false, 'message' => 'PHP Error: ' . $message]);
    exit;
});

// 애플리케이션 설정(DB 접속 정보 등) 로드
require_once(__DIR__ . '/../../../lib/config.php');
// PDO 데이터베이스 연결 객체($pdo) 초기화
require_once(__DIR__ . '/../../../lib/db.php');
// PhpSpreadsheet Composer 자동 로드 (Excel 생성에 필요)
require_once __DIR__ . '/../../../lib/vendor/autoload.php';

/**
 * Export 공통 필터 파라미터 빌더
 *
 * GET 파라미터를 읽어 SQL WHERE 절과 PDO 바인딩 파라미터 배열을 생성한다.
 * 지원 필터: factory_filter, line_filter, machine_filter, shift_filter,
 *            start_date, end_date (날짜 미지정 시 defaultSql 또는 CURDATE() 사용)
 *
 * @param string      $alias        SQL 테이블 별칭 (예: 'do', 'da', 'dd', 'doh', 'dor')
 * @param string      $dateField    날짜 컬럼명 (예: 'work_date', 'reg_date')
 * @param bool        $datetime     true 이면 시간 보정 적용 (start → 00:00:00 / end → 23:59:59)
 * @param string|null $defaultSql   날짜 미지정 시 기본 WHERE 조건 (null → CURDATE())
 * @return array{where_sql: string, params: array}
 */
function buildExportFilterParams(
    string $alias,
    string $dateField = 'work_date',
    bool $datetime = false,
    ?string $defaultSql = null
): array {
    // PDO 바인딩 파라미터 배열 및 WHERE 절 배열 초기화
    $params = [];
    $where_clauses = [];
    // 테이블 alias에 점(.)을 붙여 컬럼 접두사로 사용
    $p = $alias . '.';

    /* ── 공장/라인/기계/교대 필터 ──────────────────────────────────────── */
    // 공장 필터: GET 파라미터 factory_filter가 있으면 factory_idx 조건 추가
    if (!empty($_GET['factory_filter'])) {
        $where_clauses[] = $p . 'factory_idx = ?';
        $params[] = $_GET['factory_filter'];
    }
    // 라인 필터: GET 파라미터 line_filter가 있으면 line_idx 조건 추가
    if (!empty($_GET['line_filter'])) {
        $where_clauses[] = $p . 'line_idx = ?';
        $params[] = $_GET['line_filter'];
    }
    // 기계 필터: GET 파라미터 machine_filter가 있으면 machine_idx 조건 추가
    if (!empty($_GET['machine_filter'])) {
        $where_clauses[] = $p . 'machine_idx = ?';
        $params[] = $_GET['machine_filter'];
    }
    // 교대 필터: GET 파라미터 shift_filter가 있으면 shift_idx 조건 추가
    if (!empty($_GET['shift_filter'])) {
        $where_clauses[] = $p . 'shift_idx = ?';
        $params[] = $_GET['shift_filter'];
    }

    // 날짜 컬럼 전체 참조명 생성 (예: "da.reg_date")
    $col = $p . $dateField;

    /* ── 날짜 범위 필터 ─────────────────────────────────────────────────── */
    // start_date가 지정된 경우: datetime=true 이면 "YYYY-MM-DD 00:00:00" 으로 보정
    if (!empty($_GET['start_date'])) {
        $where_clauses[] = $col . ' >= ?';
        $params[] = $datetime ? $_GET['start_date'] . ' 00:00:00' : $_GET['start_date'];
    }
    // end_date가 지정된 경우: datetime=true 이면 "YYYY-MM-DD 23:59:59" 으로 보정
    if (!empty($_GET['end_date'])) {
        $where_clauses[] = $col . ' <= ?';
        $params[] = $datetime ? $_GET['end_date'] . ' 23:59:59' : $_GET['end_date'];
    }

    // 날짜 미지정 시 기본 조건 적용 (defaultSql이 없으면 오늘 날짜)
    if (empty($_GET['start_date']) && empty($_GET['end_date'])) {
        $where_clauses[] = $defaultSql ?? ($col . ' = CURDATE()');
    }

    // WHERE 절 조합: 조건이 없으면 빈 문자열 반환
    $where_sql = $where_clauses ? 'WHERE ' . implode(' AND ', $where_clauses) : '';

    return ['where_sql' => $where_sql, 'params' => $params];
}
