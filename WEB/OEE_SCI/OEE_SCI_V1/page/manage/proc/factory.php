<?php
ini_set('display_errors', 1);
error_reporting(E_ALL);


require_once(__DIR__ . '/../../../lib/config.php');
require_once(__DIR__ . '/../../../lib/db.php');


header('Content-Type: application/json');

$method = $_SERVER['REQUEST_METHOD'];

if ($method === 'POST' && isset($_POST['_method'])) {
    $method = strtoupper($_POST['_method']);
}

try {
    switch ($method) {
        case 'GET':
            if (isset($_GET['for']) && $_GET['for'] === 'check-duplicate') {
                checkDuplicateFactory($pdo);
            } elseif (isset($_GET['for']) && $_GET['for'] === 'statistics') {
                getStatistics($pdo);
            } elseif (isset($_GET['id'])) {
                getFactory($pdo);
            } else {
                getFactories($pdo);
            }
            break;
        case 'POST':
            addFactory($pdo);
            break;
        case 'PUT':
            updateFactory($pdo);
            break;
        default:
            http_response_code(405); // Method Not Allowed
            echo json_encode(['success' => false, 'message' => 'Method Not Allowed']);
            break;
    }
} catch (PDOException $e) {
    http_response_code(500); // Internal Server Error
    echo json_encode(['success' => false, 'message' => 'Database error: ' . $e->getMessage()]);
} catch (Exception $e) {
    http_response_code(500); // Internal Server Error
    echo json_encode(['success' => false, 'message' => 'An unexpected error occurred: ' . $e->getMessage()]);
}


function parse_factory_list_params() {
    $sort_column = $_GET['sort'] ?? 'factory_name';
    $sort_order = strtoupper($_GET['order'] ?? 'ASC');

    // SQL Injection 방지를 위해 정렬 가능한 컬럼을 화이트리스트로 관리
    $valid_columns = ['idx', 'factory_name', 'status', 'remark', 'line_count', 'machine_count', 'total_mp'];
    if (!in_array($sort_column, $valid_columns)) {
        $sort_column = 'factory_name';
    }
    if (!in_array($sort_order, ['ASC', 'DESC'])) {
        $sort_order = 'ASC';
    }

    $status_filter = $_GET['status_filter'] ?? '';
    $search_query = trim($_GET['search'] ?? '');

    $where_conditions = [];
    $params = [];

    if (!empty($status_filter)) {
        $where_conditions[] = 'status = ?';
        $params[] = $status_filter;
    }

    // 검색 파라미터 추가 (factory_name, remark에서 검색)
    if (!empty($search_query)) {
        $where_conditions[] = '(factory_name LIKE ? OR remark LIKE ?)';
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

function getFactories(PDO $pdo) {
    $query_conditions = parse_factory_list_params();
    $where_sql = $query_conditions['where_sql'];
    $params = $query_conditions['params'];
    $sort_column = $query_conditions['sort_column'];
    $sort_order = $query_conditions['sort_order'];

    $page = (int)($_GET['page'] ?? 1);
    $limit = (int)($_GET['limit'] ?? 10);
    $offset = ($page - 1) * $limit;

    $total_stmt = $pdo->prepare("SELECT COUNT(*) FROM info_factory" . $where_sql);
    $total_stmt->execute($params);
    $total_records = (int)$total_stmt->fetchColumn();

    $sql = "SELECT 
                f.idx, 
                f.factory_name, 
                f.status, 
                f.remark,
                COALESCE(line_stats.line_count, 0) as line_count,
                COALESCE(machine_stats.machine_count, 0) as machine_count,
                COALESCE(line_stats.total_mp, 0) as total_mp
            FROM info_factory f
            LEFT JOIN (
                SELECT 
                    factory_idx, 
                    COUNT(*) as line_count,
                    SUM(mp) as total_mp
                FROM info_line 
                GROUP BY factory_idx
            ) line_stats ON f.idx = line_stats.factory_idx
            LEFT JOIN (
                SELECT 
                    factory_idx, 
                    COUNT(*) as machine_count
                FROM info_machine 
                GROUP BY factory_idx
            ) machine_stats ON f.idx = machine_stats.factory_idx
            {$where_sql} 
            ORDER BY `{$sort_column}` {$sort_order} 
            LIMIT {$limit} OFFSET {$offset}";
    $stmt = $pdo->prepare($sql);
    $stmt->execute($params);
    $factories = $stmt->fetchAll(PDO::FETCH_ASSOC);

    echo json_encode([
        'success' => true,
        'data' => $factories,
        'pagination' => [
            'total_records' => $total_records,
            'current_page' => $page,
            'total_pages' => ceil($total_records / $limit)
        ]
    ]);
}

function getFactory(PDO $pdo) {
    $id = $_GET['id'] ?? 0;
    if (empty($id)) {
        http_response_code(400); // Bad Request
        echo json_encode(['success' => false, 'message' => 'ID is required.']);
        return;
    }
    $sql = "SELECT 
                f.idx, 
                f.factory_name, 
                f.status, 
                f.remark,
                COALESCE(line_stats.line_count, 0) as line_count,
                COALESCE(machine_stats.machine_count, 0) as machine_count,
                COALESCE(line_stats.total_mp, 0) as total_mp
            FROM info_factory f
            LEFT JOIN (
                SELECT 
                    factory_idx, 
                    COUNT(*) as line_count,
                    SUM(mp) as total_mp
                FROM info_line 
                GROUP BY factory_idx
            ) line_stats ON f.idx = line_stats.factory_idx
            LEFT JOIN (
                SELECT 
                    factory_idx, 
                    COUNT(*) as machine_count
                FROM info_machine 
                GROUP BY factory_idx
            ) machine_stats ON f.idx = machine_stats.factory_idx
            WHERE f.idx = ?";
    $stmt = $pdo->prepare($sql);
    $stmt->execute([$id]);
    $factory = $stmt->fetch(PDO::FETCH_ASSOC);
    if ($factory) {
        echo json_encode(['success' => true, 'data' => $factory]);
    } else {
        http_response_code(404); // Not Found
        echo json_encode(['success' => false, 'message' => 'Factory not found.']);
    }
}

function addFactory(PDO $pdo) {
    $factory_name = trim($_POST['factory_name'] ?? '');
    $status = $_POST['status'] ?? 'Y';
    $remark = trim($_POST['remark'] ?? '');

    if (empty($factory_name)) {
        http_response_code(400); // Bad Request
        echo json_encode(['success' => false, 'message' => 'Factory name is required.']);
        return;
    }

    try {
        $stmt = $pdo->prepare("INSERT INTO info_factory (factory_name, status, remark, reg_date, update_date) VALUES (?, ?, ?, NOW(), NOW())");
        $stmt->execute([$factory_name, $status, $remark]);
        http_response_code(201); // Created
        echo json_encode(['success' => true, 'message' => 'Factory added successfully.']);
    } catch (PDOException $e) {
        if ($e->getCode() == '23000') {
            http_response_code(409); // Conflict
            echo json_encode(['success' => false, 'message' => 'Factory name already exists.']);
        } else {
            throw $e;
        }
    }
}

function updateFactory(PDO $pdo) {
    $id = $_POST['idx'] ?? 0;
    $factory_name = trim($_POST['factory_name'] ?? '');
    $status = $_POST['status'] ?? 'Y';
    $remark = trim($_POST['remark'] ?? '');

    if (empty($id) || empty($factory_name)) {
        http_response_code(400); // Bad Request
        echo json_encode(['success' => false, 'message' => 'ID and Factory name are required.']);
        return;
    }

    try {
        $stmt = $pdo->prepare("UPDATE info_factory SET factory_name = ?, status = ?, remark = ?, update_date = NOW() WHERE idx = ?");
        $stmt->execute([$factory_name, $status, $remark, $id]);
        echo json_encode(['success' => true, 'message' => 'Factory updated successfully.']);
    } catch (PDOException $e) {
        if ($e->getCode() == '23000') {
            http_response_code(409); // Conflict
            echo json_encode(['success' => false, 'message' => 'Factory name already exists.']);
        } else {
            throw $e;
        }
    }
}

function checkDuplicateFactory(PDO $pdo) {
    $factory_name = trim($_GET['factory_name'] ?? '');
    $current_idx = $_GET['current_idx'] ?? null;
    
    if (empty($factory_name)) {
        http_response_code(400);
        echo json_encode(['success' => false, 'message' => 'Factory name is required.']);
        return;
    }
    
    $sql = "SELECT COUNT(*) FROM info_factory WHERE factory_name = ?";
    $params = [$factory_name];
    
    if ($current_idx) {
        $sql .= " AND idx != ?";
        $params[] = $current_idx;
    }
    
    $stmt = $pdo->prepare($sql);
    $stmt->execute($params);
    $count = (int)$stmt->fetchColumn();
    
    if ($count > 0) {
        echo json_encode(['success' => false, 'message' => 'This factory name already exists. Please enter a different name.']);
    } else {
        echo json_encode(['success' => true, 'message' => '']);
    }
}

function getStatistics(PDO $pdo) {
    $status_filter = $_GET['status_filter'] ?? '';
    
    try {
        // Factory 필터링에 따른 factory_idx 목록 가져오기
        $factory_where = '';
        $factory_params = [];
        
        if (!empty($status_filter)) {
            $factory_where = ' WHERE status = ?';
            $factory_params[] = $status_filter;
        }
        
        // 필터된 factory의 idx 목록 가져오기
        $factory_stmt = $pdo->prepare("SELECT idx FROM info_factory" . $factory_where);
        $factory_stmt->execute($factory_params);
        $factory_indices = $factory_stmt->fetchAll(PDO::FETCH_COLUMN);
        
        if (empty($factory_indices)) {
            // 조건에 맞는 factory가 없으면 모든 값을 0으로 반환
            echo json_encode([
                'success' => true,
                'data' => [
                    'total_factories' => 0,
                    'total_lines' => 0,
                    'total_machines' => 0,
                    'total_mp' => 0
                ]
            ]);
            return;
        }
        
        // IN 절을 위한 플레이스홀더 생성
        $placeholders = str_repeat('?,', count($factory_indices) - 1) . '?';
        
        // Line 수량 계산
        $line_stmt = $pdo->prepare("SELECT COUNT(*) FROM info_line WHERE factory_idx IN ($placeholders)");
        $line_stmt->execute($factory_indices);
        $total_lines = (int)$line_stmt->fetchColumn();
        
        // Machine 수량 계산
        $machine_stmt = $pdo->prepare("SELECT COUNT(*) FROM info_machine WHERE factory_idx IN ($placeholders)");
        $machine_stmt->execute($factory_indices);
        $total_machines = (int)$machine_stmt->fetchColumn();
        
        // MP(Man Power) 합계 계산
        $mp_stmt = $pdo->prepare("SELECT SUM(mp) FROM info_line WHERE factory_idx IN ($placeholders)");
        $mp_stmt->execute($factory_indices);
        $total_mp = (int)$mp_stmt->fetchColumn();
        
        // Factory 수량도 같은 조건으로 계산
        $factory_stmt = $pdo->prepare("SELECT COUNT(*) FROM info_factory" . $factory_where);
        $factory_stmt->execute($factory_params);
        $total_factories = (int)$factory_stmt->fetchColumn();
        
        echo json_encode([
            'success' => true,
            'data' => [
                'total_factories' => $total_factories,
                'total_lines' => $total_lines,
                'total_machines' => $total_machines,
                'total_mp' => $total_mp
            ]
        ]);
        
    } catch (PDOException $e) {
        http_response_code(500);
        echo json_encode(['success' => false, 'message' => 'Database error: ' . $e->getMessage()]);
    }
}

