<?php
ini_set('display_errors', 1);
error_reporting(E_ALL);

/**
 * info_machine_model API (RESTful)
 * Handles C.R.U.D operations and Excel export functionality.
 */

// Load common libraries and configuration files
require_once(__DIR__ . '/../../../lib/config.php');
require_once(__DIR__ . '/../../../lib/db.php');


// Set API response to JSON format
header('Content-Type: application/json');

// Check HTTP request method
$method = $_SERVER['REQUEST_METHOD'];

// Override method if _method field exists in POST request
if ($method === 'POST' && isset($_POST['_method'])) {
    $method = strtoupper($_POST['_method']);
}

try {
    // Call appropriate function based on request method
    switch ($method) {
        case 'GET':
            if (isset($_GET['for']) && $_GET['for'] === 'check-duplicate') {
                checkDuplicateMachineModel($pdo);
            } elseif (isset($_GET['for']) && $_GET['for'] === 'statistics') {
                getMachineModelStatistics($pdo);
            } elseif (isset($_GET['id'])) {
                getMachineModel($pdo);
            } else {
                getMachineModels($pdo);
            }
            break;
        case 'POST':
            addMachineModel($pdo);
            break;
        case 'PUT':
            updateMachineModel($pdo);
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
    http_response_code(500);
    echo json_encode(['success' => false, 'message' => 'Database error: ' . $e->getMessage()]);
} catch (Exception $e) {
    http_response_code(500);
    echo json_encode(['success' => false, 'message' => 'An unexpected error occurred: ' . $e->getMessage()]);
}


/**
 * 목록 조회(페이지네이션)와 전체 조회(엑셀)에 사용될 쿼리 파라미터를 파싱하고 유효성을 검사합니다.
 * @return array 정렬 및 필터링 조건 배열
 */
function parse_machine_model_list_params() {
    // 정렬 파라미터
    $sort_column = $_GET['sort'] ?? 'machine_model_name';
    $sort_order = strtoupper($_GET['order'] ?? 'ASC');

    // Whitelist sortable columns to prevent SQL Injection
    $valid_columns = ['idx', 'machine_model_name', 'status', 'remark', 'machine_count', 'type'];
    if (!in_array($sort_column, $valid_columns)) {
        $sort_column = 'machine_model_name'; // 기본값
    }
    if (!in_array($sort_order, ['ASC', 'DESC'])) {
        $sort_order = 'ASC'; // 기본값
    }

    // 필터링 파라미터
    $status_filter = $_GET['status_filter'] ?? '';
    $search_query = trim($_GET['search'] ?? '');

    // WHERE 절 구성
    $where_conditions = [];
    $params = [];

    if (!empty($status_filter)) {
        $where_conditions[] = 'mm.status = ?';
        $params[] = $status_filter;
    }

    // 검색 파라미터 추가 (machine_model_name, remark에서 검색)
    if (!empty($search_query)) {
        $where_conditions[] = '(mm.machine_model_name LIKE ? OR mm.remark LIKE ?)';
        $search_param = '%' . $search_query . '%';
        $params[] = $search_param;
        $params[] = $search_param;
    }

    $where_sql = '';
    if (count($where_conditions) > 0) {
        $where_sql = ' WHERE ' . implode(' AND ', $where_conditions);
    }

    return [
        'where_sql' => $where_sql,
        'params' => $params,
        'sort_column' => $sort_column,
        'sort_order' => $sort_order
    ];
}

/**
 * 모든 machine_model 목록을 조회하여 JSON으로 반환합니다.
 * 정렬, 페이지네이션, 필터링 지원.
 * @param PDO $pdo PDO 객체
 */
function getMachineModels(PDO $pdo) {
    $query_conditions = parse_machine_model_list_params();
    $where_sql = $query_conditions['where_sql'];
    $params = $query_conditions['params'];
    $sort_column = $query_conditions['sort_column'];
    $sort_order = $query_conditions['sort_order'];

    // 페이지네이션 파라미터
    $page = (int)($_GET['page'] ?? 1);
    $limit = (int)($_GET['limit'] ?? 10);
    $offset = ($page - 1) * $limit;

    // 전체 레코드 수 계산 (필터링 적용)
    $total_stmt = $pdo->prepare("SELECT COUNT(DISTINCT mm.idx) FROM info_machine_model mm" . $where_sql);
    $total_stmt->execute($params);
    $total_records = (int)$total_stmt->fetchColumn();

    // 현재 페이지에 해당하는 데이터 조회 (필터링, 정렬, 페이지네이션 적용)
    // JOIN with info_machine to get machine count per model
    $sql = "
        SELECT 
            mm.idx, 
            mm.machine_model_name, 
            mm.type, 
            mm.status, 
            mm.remark,
            COUNT(m.idx) as machine_count
        FROM info_machine_model mm
        LEFT JOIN info_machine m ON mm.idx = m.machine_model_idx
        {$where_sql}
        GROUP BY mm.idx, mm.machine_model_name, mm.type, mm.status, mm.remark
        ORDER BY mm.`{$sort_column}` {$sort_order} 
        LIMIT {$limit} OFFSET {$offset}
    ";
    $stmt = $pdo->prepare($sql);
    $stmt->execute($params);
    $machine_models = $stmt->fetchAll(PDO::FETCH_ASSOC);

    
    // Build final response data
    echo json_encode([
        'success' => true,
        'data' => $machine_models,
        'pagination' => [
            'total_records' => $total_records,
            'current_page' => $page,
            'total_pages' => ceil($total_records / $limit)
        ]
    ]);
}

/**
 * Get specific machine_model information and return as JSON.
 * @param PDO $pdo PDO object
 */
function getMachineModel(PDO $pdo) {
    $id = $_GET['id'] ?? 0;
    if (empty($id)) {
        http_response_code(400); // Bad Request
        echo json_encode(['success' => false, 'message' => 'ID is required.']);
        return;
    }
    $stmt = $pdo->prepare("
        SELECT 
            mm.idx, 
            mm.machine_model_name, 
            mm.type, 
            mm.status, 
            mm.remark,
            COUNT(m.idx) as machine_count
        FROM info_machine_model mm
        LEFT JOIN info_machine m ON mm.idx = m.machine_model_idx
        WHERE mm.idx = ?
        GROUP BY mm.idx, mm.machine_model_name, mm.type, mm.status, mm.remark
    ");
    $stmt->execute([$id]);
    $machine_model = $stmt->fetch(PDO::FETCH_ASSOC);
    if ($machine_model) {
        echo json_encode(['success' => true, 'data' => $machine_model]);
    } else {
        http_response_code(404); // Not Found
        echo json_encode(['success' => false, 'message' => 'Machine model not found.']);
    }
}

/**
 * Add new machine_model.
 * @param PDO $pdo PDO object
 */
function addMachineModel(PDO $pdo) {
    $machine_model_name = trim($_POST['machine_model_name'] ?? '');
    $status = $_POST['status'] ?? 'Y';
    $remark = trim($_POST['remark'] ?? '');

    if (empty($machine_model_name)) {
        http_response_code(400); // Bad Request
        echo json_encode(['success' => false, 'message' => 'Machine model name is required.']);
        return;
    }

    try {
        $stmt = $pdo->prepare("INSERT INTO info_machine_model (machine_model_name, type, status, remark, reg_date, update_date) VALUES (?, ?, ?, ?, NOW(), NOW())");
        $stmt->execute([$machine_model_name, $_POST['type'] ?? 'P', $status, $remark]);
        http_response_code(201); // Created
        echo json_encode(['success' => true, 'message' => 'Machine model added successfully.']);
    } catch (PDOException $e) {
        if ($e->getCode() == '23000') {
            http_response_code(409);
            echo json_encode(['success' => false, 'message' => 'Machine model name already exists.']);
        } else {
            throw $e;
        }
    }
}

/**
 * Update existing machine_model information.
 * @param PDO $pdo PDO object
 */
function updateMachineModel(PDO $pdo) {
    // Data is in $_POST since HTML form sends PUT via _method
    $id = $_POST['idx'] ?? 0;
    $machine_model_name = trim($_POST['machine_model_name'] ?? '');
    $status = $_POST['status'] ?? 'Y';
    $remark = trim($_POST['remark'] ?? '');

    if (empty($id) || empty($machine_model_name)) {
        http_response_code(400); // Bad Request
        echo json_encode(['success' => false, 'message' => 'ID and Machine model name are required.']);
        return;
    }

    try {
        $stmt = $pdo->prepare("UPDATE info_machine_model SET machine_model_name = ?, type = ?, status = ?, remark = ?, update_date = NOW() WHERE idx = ?");
        $stmt->execute([$machine_model_name, $_POST['type'] ?? 'P', $status, $remark, $id]);
        echo json_encode(['success' => true, 'message' => 'Machine model updated successfully.']);
    } catch (PDOException $e) {
        if ($e->getCode() == '23000') {
            http_response_code(409); // Conflict
            echo json_encode(['success' => false, 'message' => 'Machine model name already exists.']);
        } else {
            throw $e;
        }
    }
}

/**
 * Get machine model statistics including machine counts per model
 * @param PDO $pdo PDO object
 */
function getMachineModelStatistics(PDO $pdo) {
    // Apply status filter if provided
    $status_filter = $_GET['status_filter'] ?? '';
    $where_conditions = [];
    $params = [];
    
    if (!empty($status_filter)) {
        $where_conditions[] = 'mm.status = ?';
        $params[] = $status_filter;
    }
    
    $where_sql = !empty($where_conditions) ? 'WHERE ' . implode(' AND ', $where_conditions) : '';
    
    // Get total machine models count
    $total_stmt = $pdo->prepare("SELECT COUNT(*) FROM info_machine_model mm {$where_sql}");
    $total_stmt->execute($params);
    $total_models = (int)$total_stmt->fetchColumn();
    
    // Get machine count per model with status filter
    $sql = "
        SELECT 
            mm.machine_model_name,
            mm.type,
            mm.status,
            COUNT(m.idx) as machine_count
        FROM info_machine_model mm
        LEFT JOIN info_machine m ON mm.idx = m.machine_model_idx
        {$where_sql}
        GROUP BY mm.idx, mm.machine_model_name, mm.type, mm.status
        ORDER BY mm.machine_model_name
    ";
    
    $stmt = $pdo->prepare($sql);
    $stmt->execute($params);
    $model_stats = $stmt->fetchAll(PDO::FETCH_ASSOC);
    
    echo json_encode([
        'success' => true,
        'data' => [
            'total_models' => $total_models,
            'model_stats' => $model_stats
        ]
    ]);
}

/**
 * Check duplicate machine_model name
 * @param PDO $pdo PDO object
 */
function checkDuplicateMachineModel(PDO $pdo) {
    $machine_model_name = trim($_GET['machine_model_name'] ?? '');
    $current_idx = $_GET['current_idx'] ?? null; // Exclude current record in edit mode
    
    if (empty($machine_model_name)) {
        http_response_code(400);
        echo json_encode(['success' => false, 'message' => 'Machine model name is required.']);
        return;
    }
    
    // Duplicate check query
    $sql = "SELECT COUNT(*) FROM info_machine_model WHERE machine_model_name = ?";
    $params = [$machine_model_name];
    
    // Exclude current record in edit mode
    if ($current_idx) {
        $sql .= " AND idx != ?";
        $params[] = $current_idx;
    }
    
    $stmt = $pdo->prepare($sql);
    $stmt->execute($params);
    $count = (int)$stmt->fetchColumn();
    
    if ($count > 0) {
        echo json_encode(['success' => false, 'message' => 'This machine model name already exists. Please enter a different name.']);
    } else {
        echo json_encode(['success' => true, 'message' => 'Machine model name is available.']);
    }
}

/**
 * Delete machine_model.
 * @param PDO $pdo PDO object
 */
/* function deleteMachineModel(PDO $pdo) {
    $id = $_GET['id'] ?? 0;

    if (empty($id)) {
        http_response_code(400); // Bad Request
        echo json_encode(['success' => false, 'message' => 'ID is required.']);
        return;
    }

    $stmt = $pdo->prepare("DELETE FROM info_machine_model WHERE idx = ?");
    $stmt->execute([$id]);

    if ($stmt->rowCount() > 0) {
        echo json_encode(['success' => true, 'message' => 'Machine model deleted successfully.']);
    } else {
        http_response_code(404); // Not Found
        echo json_encode(['success' => false, 'message' => 'Machine model not found or already deleted.']);
    }
} */

?>