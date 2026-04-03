<?php
/**
 * proc/line.php — 라인(Line) 관리 REST API
 * GET    : 목록/단건 조회, 팩토리 셀렉트 옵션, 중복 체크, 통계
 * POST   : 라인 추가
 * PUT    : 라인 수정 (_method 오버라이드 지원)
 * DELETE : 미지원 (405 반환)
 */
require_once(__DIR__ . '/../../../lib/config.php');
require_once(__DIR__ . '/../../../lib/db.php');

header('Content-Type: application/json');

// HTML Form에서 PUT을 직접 보낼 수 없으므로 _method 필드로 오버라이드 처리
$method = $_SERVER['REQUEST_METHOD'];
if ($method === 'POST' && isset($_POST['_method'])) {
    $method = strtoupper($_POST['_method']);
}

// HTTP 메서드 및 'for' 파라미터에 따라 처리 함수 분기
try {
    switch ($method) {
        case 'GET':
            $for = $_GET['for'] ?? '';
            if ($for === 'select' || $for === 'factories') {
                // 공장 선택 드롭다운 옵션 반환 (라인 등록/필터 폼용)
                getFactoriesForSelect($pdo);
            } elseif ($for === 'check-duplicate') {
                // 라인명 중복 여부 확인 (같은 공장 내에서만 검사)
                checkDuplicateLine($pdo);
            } elseif ($for === 'statistics') {
                // 라인 통계 요약 반환
                getLineStatistics($pdo);
            } elseif (isset($_GET['id'])) {
                // 특정 idx 라인 단건 조회 (수정 폼용)
                getLine($pdo);
            } else {
                // 라인 목록 조회 (페이징, 필터, 정렬)
                getLines($pdo);
            }
            break;
        case 'POST':
            // 신규 라인 추가
            addLine($pdo);
            break;
        case 'PUT':
            // 기존 라인 수정
            updateLine($pdo);
            break;
        case 'DELETE':
            // 라인 삭제는 지원하지 않음
            http_response_code(405);
            echo json_encode(['success' => false, 'message' => 'Delete operation not supported']);
            break;
        default:
            http_response_code(405);
            echo json_encode(['success' => false, 'message' => 'Method Not Allowed']);
            break;
    }
} catch (PDOException $e) {
    http_response_code(500);
    echo json_encode(['success' => false, 'message' => 'Database error: ' . $e->getMessage()]);
} catch (Exception $e) {
    http_response_code(500);
    echo json_encode(['success' => false, 'message' => 'An unexpected error occurred: ' . $e->getMessage()]);
}


/**
 * GET 파라미터에서 목록 조회 조건을 파싱하여 반환
 * - sort/order: 허용 컬럼 화이트리스트로 SQL 인젝션 방지
 * - factory_filter: 특정 공장의 라인만 조회
 * - status_filter: 사용 여부(Y/N) 필터
 * - search: line_name, factory_name 대상 LIKE 검색
 */
function parse_line_list_params(): array {
    $sort_column = $_GET['sort'] ?? 'l.line_name';
    $sort_order  = strtoupper($_GET['order'] ?? 'ASC');

    // 허용된 정렬 컬럼 화이트리스트
    $valid_columns = ['l.idx', 'f.factory_name', 'l.line_name', 'l.status', 'l.mp', 'l.line_target', 'machine_count'];
    if (!in_array($sort_column, $valid_columns)) $sort_column = 'l.line_name';
    if (!in_array($sort_order, ['ASC', 'DESC']))  $sort_order  = 'ASC';

    $factory_filter = $_GET['factory_filter'] ?? '';
    $status_filter  = $_GET['status_filter']  ?? '';
    $search_query   = trim($_GET['search']    ?? '');

    $where_clauses = [];
    $params        = [];

    // 공장 필터 조건 추가
    if (!empty($factory_filter)) { $where_clauses[] = 'l.factory_idx = ?'; $params[] = $factory_filter; }
    // 사용 여부 필터 조건 추가
    if (!empty($status_filter))  { $where_clauses[] = 'l.status = ?';      $params[] = $status_filter; }
    // 검색어: 라인명 또는 공장명 LIKE 검색
    if (!empty($search_query)) {
        $where_clauses[] = '(l.line_name LIKE ? OR f.factory_name LIKE ?)';
        $p = '%' . $search_query . '%';
        $params = array_merge($params, [$p, $p]);
    }

    $where_sql = $where_clauses ? ' WHERE ' . implode(' AND ', $where_clauses) : '';

    return ['where_sql' => $where_sql, 'params' => $params, 'sort_column' => $sort_column, 'sort_order' => $sort_order];
}

/**
 * 라인 목록 조회 (페이징 + 필터 + 정렬)
 * - COUNT(DISTINCT l.idx): GROUP BY 시 중복 카운트 방지
 * - LEFT JOIN info_machine: 기계 수(machine_count) 집계
 * - GROUP BY로 라인별 기계 수 집계
 */
function getLines(PDO $pdo): void {
    $cond        = parse_line_list_params();
    $where_sql   = $cond['where_sql'];
    $params      = $cond['params'];
    $sort_column = $cond['sort_column'];
    $sort_order  = $cond['sort_order'];

    $page   = (int)($_GET['page']  ?? 1);
    $limit  = (int)($_GET['limit'] ?? 10);
    $offset = ($page - 1) * $limit;

    // GROUP BY 사용 시 COUNT(DISTINCT)로 정확한 전체 건수 산출
    $total_stmt = $pdo->prepare(
        "SELECT COUNT(DISTINCT l.idx) FROM info_line AS l
         LEFT JOIN info_factory AS f ON l.factory_idx = f.idx{$where_sql}"
    );
    $total_stmt->execute($params);
    $total_records = (int)$total_stmt->fetchColumn();

    // 라인 정보 + 공장명 + 기계 수를 한 번에 조회
    $sql = "SELECT l.idx, l.factory_idx, f.factory_name, l.line_name, l.status, l.mp, l.line_target, l.remark,
                   COUNT(m.idx) AS machine_count
            FROM info_line AS l
            LEFT JOIN info_factory AS f ON l.factory_idx = f.idx
            LEFT JOIN info_machine AS m ON l.idx = m.line_idx
            {$where_sql}
            GROUP BY l.idx, l.factory_idx, f.factory_name, l.line_name, l.status, l.mp, l.line_target, l.remark
            ORDER BY {$sort_column} {$sort_order}
            LIMIT {$limit} OFFSET {$offset}";
    $stmt = $pdo->prepare($sql);
    $stmt->execute($params);

    echo json_encode([
        'success'    => true,
        'data'       => $stmt->fetchAll(PDO::FETCH_ASSOC),
        'pagination' => [
            'total_records' => $total_records,
            'current_page'  => $page,
            'total_pages'   => ceil($total_records / $limit),
        ],
    ]);
}

/**
 * 라인 단건 조회 (수정 폼 데이터 로드용)
 * - info_line 단순 조회 (통계 미포함)
 */
function getLine(PDO $pdo): void {
    $id = $_GET['id'] ?? 0;
    if (empty($id)) {
        http_response_code(400);
        echo json_encode(['success' => false, 'message' => 'ID is required.']);
        return;
    }
    $stmt = $pdo->prepare("SELECT * FROM info_line WHERE idx = ?");
    $stmt->execute([$id]);
    $row = $stmt->fetch(PDO::FETCH_ASSOC);
    if ($row) {
        echo json_encode(['success' => true, 'data' => $row]);
    } else {
        http_response_code(404);
        echo json_encode(['success' => false, 'message' => 'Line not found.']);
    }
}

/**
 * 신규 라인 추가 (POST)
 * - factory_idx, line_name 필수 검증
 * - mp(인원), line_target(목표 수량) 정수로 저장
 * - UNIQUE(factory_idx, line_name) 위반 시 409 반환
 */
function addLine(PDO $pdo): void {
    $factory_idx = $_POST['factory_idx'] ?? null;
    $line_name   = trim($_POST['line_name'] ?? '');
    $status      = $_POST['status'] ?? 'Y';
    $mp          = (int)($_POST['mp']          ?? 0);
    $line_target = (int)($_POST['line_target'] ?? 0);
    $remark      = trim($_POST['remark'] ?? '');

    // 공장과 라인명은 필수 입력값
    if (empty($factory_idx) || empty($line_name)) {
        http_response_code(400);
        echo json_encode(['success' => false, 'message' => 'Factory and Line Name are required.']);
        return;
    }
    try {
        $stmt = $pdo->prepare("INSERT INTO info_line (factory_idx, line_name, status, mp, line_target, remark, reg_date, update_date) VALUES (?, ?, ?, ?, ?, ?, NOW(), NOW())");
        $stmt->execute([$factory_idx, $line_name, $status, $mp, $line_target, $remark]);
        http_response_code(201);
        echo json_encode(['success' => true, 'message' => 'Line added successfully.']);
    } catch (PDOException $e) {
        // 같은 공장 내 동일 라인명 중복 시 409 응답
        if ($e->getCode() == '23000') {
            http_response_code(409);
            echo json_encode(['success' => false, 'message' => 'The same line name already exists in this factory.']);
        } else {
            throw $e;
        }
    }
}

/**
 * 라인 정보 수정 (PUT)
 * - idx, factory_idx, line_name 필수 검증
 * - update_date 자동 갱신
 */
function updateLine(PDO $pdo): void {
    $id          = $_POST['idx'] ?? 0;
    $factory_idx = $_POST['factory_idx'] ?? null;
    $line_name   = trim($_POST['line_name'] ?? '');
    $status      = $_POST['status'] ?? 'Y';
    $mp          = (int)($_POST['mp']          ?? 0);
    $line_target = (int)($_POST['line_target'] ?? 0);
    $remark      = trim($_POST['remark'] ?? '');

    if (empty($id) || empty($factory_idx) || empty($line_name)) {
        http_response_code(400);
        echo json_encode(['success' => false, 'message' => 'ID, Factory, and Line Name are required.']);
        return;
    }
    try {
        $stmt = $pdo->prepare("UPDATE info_line SET factory_idx = ?, line_name = ?, status = ?, mp = ?, line_target = ?, remark = ?, update_date = NOW() WHERE idx = ?");
        $stmt->execute([$factory_idx, $line_name, $status, $mp, $line_target, $remark, $id]);
        echo json_encode(['success' => true, 'message' => 'Line updated successfully.']);
    } catch (PDOException $e) {
        if ($e->getCode() == '23000') {
            http_response_code(409);
            echo json_encode(['success' => false, 'message' => 'The same line name already exists in this factory.']);
        } else {
            throw $e;
        }
    }
}

/**
 * 활성 공장 목록 반환 (라인 등록/필터 폼의 셀렉트 옵션용)
 * - status = 'Y'인 공장만 반환
 * - factory_name 오름차순 정렬
 */
function getFactoriesForSelect(PDO $pdo): void {
    $stmt = $pdo->prepare("SELECT idx, factory_name FROM info_factory WHERE status = 'Y' ORDER BY factory_name ASC");
    $stmt->execute();
    echo json_encode(['success' => true, 'data' => $stmt->fetchAll(PDO::FETCH_ASSOC)]);
}

/**
 * 라인명 중복 여부 확인 (같은 공장 내에서만 검사)
 * - factory_idx + line_name 조합으로 중복 체크
 * - current_idx가 있으면 자기 자신 제외 (수정 모드)
 */
function checkDuplicateLine(PDO $pdo): void {
    $factory_idx = $_GET['factory_idx'] ?? null;
    $line_name   = trim($_GET['line_name'] ?? '');
    $current_idx = $_GET['current_idx']   ?? null;

    if (empty($factory_idx) || empty($line_name)) {
        http_response_code(400);
        echo json_encode(['success' => false, 'message' => 'Factory and Line Name are required.']);
        return;
    }
    // 같은 공장 내에서 동일 라인명 존재 여부 확인
    $sql    = "SELECT COUNT(*) FROM info_line WHERE factory_idx = ? AND line_name = ?";
    $params = [$factory_idx, $line_name];
    if ($current_idx) { $sql .= " AND idx != ?"; $params[] = $current_idx; }

    $stmt = $pdo->prepare($sql);
    $stmt->execute($params);
    echo json_encode((int)$stmt->fetchColumn() > 0
        ? ['success' => false, 'message' => 'The same line name already exists in this factory.']
        : ['success' => true,  'message' => '']
    );
}

/**
 * Line 통계 — 단일 쿼리로 집계 (Machine 수는 JOIN 서브쿼리로 처리)
 * - total_lines: 라인 수
 * - total_manpower: 총 인원 합계
 * - total_target: 총 목표 수량 합계
 * - total_machines: 총 기계 수
 * - target_per_man: 인당 목표 수량 (total_target / total_manpower, 0 나누기 방지)
 */
function getLineStatistics(PDO $pdo): void {
    $factory_filter = $_GET['factory_filter'] ?? '';
    $status_filter  = $_GET['status_filter']  ?? '';

    $where_clauses = [];
    $params        = [];
    if (!empty($factory_filter)) { $where_clauses[] = 'l.factory_idx = ?'; $params[] = $factory_filter; }
    if (!empty($status_filter))  { $where_clauses[] = 'l.status = ?';      $params[] = $status_filter; }
    $where_sql = $where_clauses ? ' WHERE ' . implode(' AND ', $where_clauses) : '';

    try {
        // LEFT JOIN으로 기계 수 함께 집계
        $sql = "SELECT
                    COUNT(DISTINCT l.idx)           AS total_lines,
                    COALESCE(SUM(l.mp), 0)          AS total_manpower,
                    COALESCE(SUM(l.line_target), 0) AS total_target,
                    COUNT(m.idx)                    AS total_machines
                FROM info_line AS l
                LEFT JOIN info_machine AS m ON l.idx = m.line_idx
                {$where_sql}";
        $stmt = $pdo->prepare($sql);
        $stmt->execute($params);
        $row = $stmt->fetch(PDO::FETCH_ASSOC);

        $total_manpower = (int)$row['total_manpower'];
        $total_target   = (int)$row['total_target'];

        echo json_encode([
            'success' => true,
            'data'    => [
                'total_lines'    => (int)$row['total_lines'],
                'total_machines' => (int)$row['total_machines'],
                'total_manpower' => $total_manpower,
                'total_target'   => $total_target,
                // 인원이 0이면 0으로 처리하여 ZeroDivisionError 방지
                'target_per_man' => $total_manpower > 0 ? round($total_target / $total_manpower) : 0,
            ],
        ]);
    } catch (PDOException $e) {
        http_response_code(500);
        echo json_encode(['success' => false, 'message' => 'Database error: ' . $e->getMessage()]);
    }
}
