<?php

/**
 * info_line 테이블 관련 API (RESTful)
 * C.R.U.D 기능 및 엑셀 내보내기 기능을 처리합니다.
 */

// 공통 라이브러리 및 설정 파일 로드
require_once(__DIR__ . '/../../../lib/config.php');
require_once(__DIR__ . '/../../../lib/db.php');


// API 응답을 JSON 형식으로 설정
header('Content-Type: application/json');

// HTTP 요청 메서드를 확인
$method = $_SERVER['REQUEST_METHOD'];

// POST 요청에서 _method 필드가 있는 경우 해당 값으로 메서드를 재정의 (HTML 폼에서 PUT, DELETE 지원)
if ($method === 'POST' && isset($_POST['_method'])) {
    $method = strtoupper($_POST['_method']);
}

try {
    // 요청 메서드에 따라 적절한 함수 호출
    switch ($method) {
        case 'GET':
            if (isset($_GET['for']) && ($_GET['for'] === 'select' || $_GET['for'] === 'factories')) {
                getFactoriesForSelect($pdo);
            } elseif (isset($_GET['for']) && $_GET['for'] === 'check-duplicate') {
                checkDuplicateLine($pdo);
            } elseif (isset($_GET['for']) && $_GET['for'] === 'statistics') {
                getLineStatistics($pdo);
            } elseif (isset($_GET['id'])) {
                getLine($pdo);
            } else {
                getLines($pdo);
            }
            break;
        case 'POST':
            addLine($pdo);
            break;
        case 'PUT':
            updateLine($pdo);
            break;
        case 'DELETE':
            http_response_code(405); // Method Not Allowed
            echo json_encode(['success' => false, 'message' => 'Delete operation not supported']);
            break;
        default:
            http_response_code(405); // Method Not Allowed
            echo json_encode(['success' => false, 'message' => 'Method Not Allowed']);
            break;
    }
} catch (PDOException $e) {
    // 데이터베이스 관련 예외 처리
    http_response_code(500); // Internal Server Error
    echo json_encode(['success' => false, 'message' => 'Database error: ' . $e->getMessage()]);
} catch (Exception $e) {
    // 기타 예외 처리
    http_response_code(500); // Internal Server Error
    echo json_encode(['success' => false, 'message' => 'An unexpected error occurred: ' . $e->getMessage()]);
}


/**
 * 목록 조회(페이지네이션)와 전체 조회(엑셀)에 사용될 쿼리 파라미터를 파싱하고 유효성을 검사합니다.
 * @return array 정렬 및 필터링 조건 배열
 */
function parse_line_list_params() {
    // 정렬 파라미터
    $sort_column = $_GET['sort'] ?? 'l.line_name';
    $sort_order = strtoupper($_GET['order'] ?? 'ASC');

    // SQL Injection 방지를 위해 정렬 가능한 컬럼을 화이트리스트로 관리
    $valid_columns = ['l.idx', 'f.factory_name', 'l.line_name', 'l.status', 'l.mp', 'l.line_target', 'machine_count'];
    if (!in_array($sort_column, $valid_columns)) {
        $sort_column = 'l.line_name';
    }
    if (!in_array($sort_order, ['ASC', 'DESC'])) {
        $sort_order = 'ASC';
    }

    // 필터링 파라미터
    $factory_filter = $_GET['factory_filter'] ?? '';
    $status_filter = $_GET['status_filter'] ?? '';
    $search_query = trim($_GET['search'] ?? '');

    // WHERE 절 구성
    $where_clauses = [];
    $params = [];
    if (!empty($factory_filter)) {
        $where_clauses[] = 'l.factory_idx = ?';
        $params[] = $factory_filter;
    }
    if (!empty($status_filter)) {
        $where_clauses[] = 'l.status = ?';
        $params[] = $status_filter;
    }

    // 검색 파라미터 추가 (line_name, factory_name에서 검색)
    if (!empty($search_query)) {
        $where_clauses[] = '(l.line_name LIKE ? OR f.factory_name LIKE ?)';
        $search_param = '%' . $search_query . '%';
        $params[] = $search_param;
        $params[] = $search_param;
    }

    $where_sql = count($where_clauses) > 0 ? ' WHERE ' . implode(' AND ', $where_clauses) : '';

    return [
        'where_sql' => $where_sql,
        'params' => $params,
        'sort_column' => $sort_column,
        'sort_order' => $sort_order
    ];
}

/**
 * 모든 line 목록을 조회하여 JSON으로 반환합니다.
 * 정렬, 페이지네이션, 필터링 지원.
 * @param PDO $pdo PDO 객체
 */
function getLines(PDO $pdo) {
    $query_conditions = parse_line_list_params();
    $where_sql = $query_conditions['where_sql'];
    $params = $query_conditions['params'];
    $sort_column = $query_conditions['sort_column'];
    $sort_order = $query_conditions['sort_order'];

    // 페이지네이션 파라미터
    $page = (int)($_GET['page'] ?? 1);
    $limit = (int)($_GET['limit'] ?? 10);
    $offset = ($page - 1) * $limit;

    // 전체 레코드 수 계산 (필터링 적용)
    $count_sql = "SELECT COUNT(DISTINCT l.idx) FROM info_line AS l
                  LEFT JOIN info_factory AS f ON l.factory_idx = f.idx" . $where_sql;
    $total_stmt = $pdo->prepare($count_sql);
    $total_stmt->execute($params);
    $total_records = (int)$total_stmt->fetchColumn();

    // 현재 페이지에 해당하는 데이터 조회 (필터링, 정렬, 페이지네이션 적용)
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
    $lines = $stmt->fetchAll(PDO::FETCH_ASSOC);

    // 최종 응답 데이터 구성
    echo json_encode([
        'success' => true,
        'data' => $lines,
        'pagination' => [
            'total_records' => $total_records,
            'current_page' => $page,
            'total_pages' => ceil($total_records / $limit)
        ]
    ]);
}

/**
 * 특정 line 정보를 조회하여 JSON으로 반환합니다.
 * @param PDO $pdo PDO 객체
 */
function getLine(PDO $pdo) {
    $id = $_GET['id'] ?? 0;
    if (empty($id)) {
        http_response_code(400); // Bad Request
        echo json_encode(['success' => false, 'message' => 'ID is required.']);
        return;
    }
    $stmt = $pdo->prepare("SELECT * FROM info_line WHERE idx = ?");
    $stmt->execute([$id]);
    $line = $stmt->fetch(PDO::FETCH_ASSOC);
    if ($line) {
        echo json_encode(['success' => true, 'data' => $line]);
    } else {
        http_response_code(404);
        echo json_encode(['success' => false, 'message' => 'Line not found.']);
    }
}

/**
 * 신규 line을 추가합니다.
 * @param PDO $pdo PDO 객체
 */
function addLine(PDO $pdo) {
    $factory_idx = $_POST['factory_idx'] ?? null;
    $line_name = trim($_POST['line_name'] ?? '');
    $status = $_POST['status'] ?? 'Y';
    $mp = (int)($_POST['mp'] ?? 0);
    $line_target = (int)($_POST['line_target'] ?? 0);
    $remark = trim($_POST['remark'] ?? '');

    if (empty($factory_idx) || empty($line_name)) {
        http_response_code(400); // Bad Request
        echo json_encode(['success' => false, 'message' => 'Factory and Line Name are required.']);
        return;
    }
    try {
        $stmt = $pdo->prepare("INSERT INTO info_line (factory_idx, line_name, status, mp, line_target, remark, reg_date, update_date) VALUES (?, ?, ?, ?, ?, ?, NOW(), NOW())");
        $stmt->execute([$factory_idx, $line_name, $status, $mp, $line_target, $remark]);
        http_response_code(201); // Created
        echo json_encode(['success' => true, 'message' => 'Line added successfully.']);
    } catch (PDOException $e) {
        if ($e->getCode() == '23000') {
            http_response_code(409); // Conflict
            echo json_encode(['success' => false, 'message' => 'The same line name already exists in this factory.']);
        } else {
            throw $e; // Re-throw exception for upper handler to process
        }
    }
}

/**
 * 기존 line 정보를 수정합니다.
 * @param PDO $pdo PDO 객체
 */
function updateLine(PDO $pdo) {
    // HTML form에서 _method로 PUT을 전송하므로, 데이터는 $_POST에 있습니다.
    $id = $_POST['idx'] ?? 0;
    $factory_idx = $_POST['factory_idx'] ?? null;
    $line_name = trim($_POST['line_name'] ?? '');
    $status = $_POST['status'] ?? 'Y';
    $mp = (int)($_POST['mp'] ?? 0);
    $line_target = (int)($_POST['line_target'] ?? 0);
    $remark = trim($_POST['remark'] ?? '');

    if (empty($id) || empty($factory_idx) || empty($line_name)) {
        http_response_code(400); // Bad Request
        echo json_encode(['success' => false, 'message' => 'ID, Factory, and Line Name are required.']);
        return;
    }

    try {
        $stmt = $pdo->prepare("UPDATE info_line SET factory_idx = ?, line_name = ?, status = ?, mp = ?, line_target = ?, remark = ?, update_date = NOW() WHERE idx = ?");
        $stmt->execute([$factory_idx, $line_name, $status, $mp, $line_target, $remark, $id]);
        echo json_encode(['success' => true, 'message' => 'Line updated successfully.']);
    } catch (PDOException $e) {
        if ($e->getCode() == '23000') {
            http_response_code(409); // Conflict
            echo json_encode(['success' => false, 'message' => 'The same line name already exists in this factory.']);
        } else {
            throw $e;
        }
    }
}

/**
 * factory 필터.
 * @param PDO $pdo PDO 객체
 */
function getFactoriesForSelect(PDO $pdo) {
    $stmt = $pdo->prepare("SELECT idx, factory_name FROM info_factory WHERE status = 'Y' ORDER BY factory_name ASC");
    $stmt->execute();
    $factories = $stmt->fetchAll(PDO::FETCH_ASSOC);
    echo json_encode(['success' => true, 'data' => $factories]);
}

/**
 * 중복 line 이름 확인
 * @param PDO $pdo PDO 객체
 */
function checkDuplicateLine(PDO $pdo) {
    $factory_idx = $_GET['factory_idx'] ?? null;
    $line_name = trim($_GET['line_name'] ?? '');
    $current_idx = $_GET['current_idx'] ?? null; // 수정 모드에서 현재 레코드 제외

    if (empty($factory_idx) || empty($line_name)) {
        http_response_code(400);
        echo json_encode(['success' => false, 'message' => 'Factory and Line Name are required.']);
        return;
    }

    // 중복 확인 쿼리 (수정 모드에서는 현재 레코드 제외)
    $sql = "SELECT COUNT(*) FROM info_line WHERE factory_idx = ? AND line_name = ?";
    $params = [$factory_idx, $line_name];
    
    if ($current_idx) {
        $sql .= " AND idx != ?";
        $params[] = $current_idx;
    }

    $stmt = $pdo->prepare($sql);
    $stmt->execute($params);
    $count = (int)$stmt->fetchColumn();

    if ($count > 0) {
        echo json_encode(['success' => false, 'message' => 'The same line name already exists in this factory.']);
    } else {
        echo json_encode(['success' => true, 'message' => '']);
    }
}


/**
 * Line 통계 정보를 조회하여 JSON으로 반환합니다.
 * @param PDO $pdo PDO 객체
 */
function getLineStatistics(PDO $pdo) {
    $factory_filter = $_GET['factory_filter'] ?? '';
    $status_filter = $_GET['status_filter'] ?? '';
    
    try {
        // 필터링 조건 구성
        $where_clauses = [];
        $params = [];
        
        if (!empty($factory_filter)) {
            $where_clauses[] = 'l.factory_idx = ?';
            $params[] = $factory_filter;
        }
        if (!empty($status_filter)) {
            $where_clauses[] = 'l.status = ?';
            $params[] = $status_filter;
        }
        
        $where_sql = count($where_clauses) > 0 ? ' WHERE ' . implode(' AND ', $where_clauses) : '';
        
        // 1. Total Lines
        $line_stmt = $pdo->prepare("SELECT COUNT(*) FROM info_line AS l" . $where_sql);
        $line_stmt->execute($params);
        $total_lines = (int)$line_stmt->fetchColumn();
        
        // 2. Total Machines - 필터된 라인들의 machine 수 계산
        if ($total_lines > 0) {
            // 필터된 라인들의 idx 목록 가져오기
            $line_idx_stmt = $pdo->prepare("SELECT l.idx FROM info_line AS l" . $where_sql);
            $line_idx_stmt->execute($params);
            $line_indices = $line_idx_stmt->fetchAll(PDO::FETCH_COLUMN);
            
            if (!empty($line_indices)) {
                // IN 절을 위한 플레이스홀더 생성
                $placeholders = str_repeat('?,', count($line_indices) - 1) . '?';
                
                // Machine 수량 계산
                $machine_stmt = $pdo->prepare("SELECT COUNT(*) FROM info_machine WHERE line_idx IN ($placeholders)");
                $machine_stmt->execute($line_indices);
                $total_machines = (int)$machine_stmt->fetchColumn();
            } else {
                $total_machines = 0;
            }
        } else {
            $total_machines = 0;
        }
        
        // 3. Total Manpower (MP)
        $mp_stmt = $pdo->prepare("SELECT SUM(l.mp) FROM info_line AS l" . $where_sql);
        $mp_stmt->execute($params);
        $total_manpower = (int)$mp_stmt->fetchColumn();
        
        // 4. Total Target
        $target_stmt = $pdo->prepare("SELECT SUM(l.line_target) FROM info_line AS l" . $where_sql);
        $target_stmt->execute($params);
        $total_target = (int)$target_stmt->fetchColumn();
        
        // 5. Target per Man (Total Target / Total Manpower)
        $target_per_man = $total_manpower > 0 ? round($total_target / $total_manpower) : 0;
        
        echo json_encode([
            'success' => true,
            'data' => [
                'total_lines' => $total_lines,
                'total_machines' => $total_machines,
                'total_manpower' => $total_manpower,
                'total_target' => $total_target,
                'target_per_man' => $target_per_man
            ]
        ]);
        
    } catch (PDOException $e) {
        http_response_code(500);
        echo json_encode(['success' => false, 'message' => 'Database error: ' . $e->getMessage()]);
    }
}