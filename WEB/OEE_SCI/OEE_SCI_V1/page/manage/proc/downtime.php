<?php
ini_set('display_errors', 1);
error_reporting(E_ALL);

// 공통 라이브러리 및 설정 파일 로드
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
                checkDuplicateDowntime($pdo);
            } elseif (isset($_GET['for']) && $_GET['for'] === 'check-duplicate-shortcut') {
                checkDuplicateDowntimeShortcut($pdo);
            } elseif (isset($_GET['for']) && $_GET['for'] === 'warning-stats') {
                getWarningStats($pdo);
            } elseif (isset($_GET['id'])) {
                getDowntime($pdo);
            } else {
                getDowntimes($pdo);
            }
            break;
        case 'POST':
            addDowntime($pdo);
            break;
        case 'PUT':
            updateDowntime($pdo);
            break;
        case 'DELETE':
            deleteDowntime($pdo);
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


function parse_downtime_list_params() {
    $sort_column = $_GET['sort'] ?? 'downtime_name';
    $sort_order = strtoupper($_GET['order'] ?? 'ASC');

    $valid_columns = ['idx', 'downtime_name', 'downtime_shortcut', 'status', 'remark', 'total_count', 'completed_count', 'warning_count', 'warning_rate'];
    if (!in_array($sort_column, $valid_columns)) {
        $sort_column = 'downtime_name';
    }
    if (!in_array($sort_order, ['ASC', 'DESC'])) {
        $sort_order = 'ASC';
    }

    // 통계 컬럼의 경우 별칭을 그대로 사용, 일반 컬럼의 경우 테이블 별칭 추가
    $stats_columns = ['total_count', 'completed_count', 'warning_count', 'warning_rate'];
    if (in_array($sort_column, $stats_columns)) {
        $order_column = $sort_column;
    } else {
        $order_column = 'id.' . $sort_column;
    }

    $status_filter = $_GET['status_filter'] ?? '';
    $search_query = trim($_GET['search'] ?? '');

    $where_conditions = [];
    $params = [];

    if (!empty($status_filter)) {
        $where_conditions[] = 'id.status = ?';
        $params[] = $status_filter;
    }

    // 검색 파라미터 추가 (downtime_name, downtime_shortcut, remark에서 검색)
    if (!empty($search_query)) {
        $where_conditions[] = '(id.downtime_name LIKE ? OR id.downtime_shortcut LIKE ? OR id.remark LIKE ?)';
        $search_param = '%' . $search_query . '%';
        $params[] = $search_param;
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
        'order_column' => $order_column,
        'sort_order' => $sort_order
    ];
}

function getDowntimes(PDO $pdo) {
    $query_conditions = parse_downtime_list_params();
    $where_sql = $query_conditions['where_sql'];
    $params = $query_conditions['params'];
    $order_column = $query_conditions['order_column'];
    $sort_order = $query_conditions['sort_order'];

    $page = (int)($_GET['page'] ?? 1);
    $limit = (int)($_GET['limit'] ?? 10);
    $offset = ($page - 1) * $limit;

    // 카운트용 쿼리
    $count_where = str_replace('id.', '', $where_sql);
    $total_stmt = $pdo->prepare("SELECT COUNT(*) FROM info_downtime" . $count_where);
    $total_stmt->execute($params);
    $total_records = (int)$total_stmt->fetchColumn();

    // 기본 downtime 정보와 통계를 함께 조회하는 쿼리
    $sql = "
        SELECT 
            id.idx, 
            id.downtime_name, 
            id.downtime_shortcut, 
            id.status, 
            id.remark,
            COALESCE(stats.total_count, 0) as total_count,
            COALESCE(stats.completed_count, 0) as completed_count,
            COALESCE(stats.warning_count, 0) as warning_count,
            COALESCE(
                CASE 
                    WHEN stats.total_count > 0 
                    THEN ROUND((stats.warning_count / stats.total_count) * 100, 1)
                    ELSE 0 
                END, 
                0
            ) as warning_rate
        FROM info_downtime id
        LEFT JOIN (
            SELECT 
                downtime_name,
                COUNT(*) as total_count,
                SUM(CASE WHEN status = 'Completed' THEN 1 ELSE 0 END) as completed_count,
                SUM(CASE WHEN status = 'Warning' THEN 1 ELSE 0 END) as warning_count
            FROM data_downtime 
            GROUP BY downtime_name
        ) stats ON id.downtime_name = stats.downtime_name
        {$where_sql} 
        ORDER BY {$order_column} {$sort_order} 
        LIMIT {$limit} OFFSET {$offset}
    ";
    
    $stmt = $pdo->prepare($sql);
    $stmt->execute($params);
    $downtimes = $stmt->fetchAll(PDO::FETCH_ASSOC);
    
    echo json_encode([
        'success' => true,
        'data' => $downtimes,
        'pagination' => [
            'total_records' => $total_records,
            'current_page' => $page,
            'total_pages' => ceil($total_records / $limit)
        ]
    ]);
}

function getDowntime(PDO $pdo) {
    $id = $_GET['id'] ?? 0;
    if (empty($id)) {
        http_response_code(400);
        echo json_encode(['success' => false, 'message' => 'ID is required.']);
        return;
    }
    $stmt = $pdo->prepare("SELECT idx, downtime_name, downtime_shortcut, status, remark FROM info_downtime WHERE idx = ?");
    $stmt->execute([$id]);
    $downtime = $stmt->fetch(PDO::FETCH_ASSOC);
    if ($downtime) {
        echo json_encode(['success' => true, 'data' => $downtime]);
    } else {
        http_response_code(404);
        echo json_encode(['success' => false, 'message' => 'Downtime not found.']);
    }
}

function addDowntime(PDO $pdo) {
    $downtime_name = trim($_POST['downtime_name'] ?? '');
    $downtime_shortcut = trim($_POST['downtime_shortcut'] ?? '');
    $status = $_POST['status'] ?? 'Y';
    $remark = trim($_POST['remark'] ?? '');

    if (empty($downtime_name)) {
        http_response_code(400);
        echo json_encode(['success' => false, 'message' => 'Downtime name is required.']);
        return;
    }

    try {
        $stmt = $pdo->prepare("INSERT INTO info_downtime (downtime_name, downtime_shortcut, status, remark, reg_date, update_date) VALUES (?, ?, ?, ?, NOW(), NOW())");
        $stmt->execute([$downtime_name, $downtime_shortcut, $status, $remark]);
        http_response_code(201);
        echo json_encode(['success' => true, 'message' => 'Downtime added successfully.']);
    } catch (PDOException $e) {
        if ($e->getCode() == '23000') {
            http_response_code(409);
            echo json_encode(['success' => false, 'message' => 'Downtime name already exists.']);
        } else {
            throw $e;
        }
    }
}

function updateDowntime(PDO $pdo) {
    $id = $_POST['idx'] ?? 0;
    $downtime_name = trim($_POST['downtime_name'] ?? '');
    $downtime_shortcut = trim($_POST['downtime_shortcut'] ?? '');
    $status = $_POST['status'] ?? 'Y';
    $remark = trim($_POST['remark'] ?? '');

    if (empty($id) || empty($downtime_name)) {
        http_response_code(400);
        echo json_encode(['success' => false, 'message' => 'ID and Downtime name are required.']);
        return;
    }

    try {
        $stmt = $pdo->prepare("UPDATE info_downtime SET downtime_name = ?, downtime_shortcut = ?, status = ?, remark = ?, update_date = NOW() WHERE idx = ?");
        $stmt->execute([$downtime_name, $downtime_shortcut, $status, $remark, $id]);
        echo json_encode(['success' => true, 'message' => 'Downtime updated successfully.']);
    } catch (PDOException $e) {
        if ($e->getCode() == '23000') {
            http_response_code(409);
            echo json_encode(['success' => false, 'message' => 'Downtime name already exists.']);
        } else {
            throw $e;
        }
    }
}

function deleteDowntime(PDO $pdo) {
    $id = $_GET['id'] ?? 0;

    if (empty($id)) {
        http_response_code(400);
        echo json_encode(['success' => false, 'message' => 'ID is required.']);
        return;
    }

    $stmt = $pdo->prepare("DELETE FROM info_downtime WHERE idx = ?");
    $stmt->execute([$id]);

    if ($stmt->rowCount() > 0) {
        echo json_encode(['success' => true, 'message' => 'Downtime deleted successfully.']);
    } else {
        http_response_code(404);
        echo json_encode(['success' => false, 'message' => 'Downtime not found or already deleted.']);
    }
}

function checkDuplicateDowntime(PDO $pdo) {
    $downtime_name = trim($_GET['downtime_name'] ?? '');
    $current_idx = $_GET['current_idx'] ?? null;
    
    if (empty($downtime_name)) {
        http_response_code(400);
        echo json_encode(['success' => false, 'message' => 'Downtime name is required.']);
        return;
    }
    
    $sql = "SELECT COUNT(*) FROM info_downtime WHERE downtime_name = ?";
    $params = [$downtime_name];
    
    if ($current_idx) {
        $sql .= " AND idx != ?";
        $params[] = $current_idx;
    }
    
    $stmt = $pdo->prepare($sql);
    $stmt->execute($params);
    $count = (int)$stmt->fetchColumn();
    
    if ($count > 0) {
        echo json_encode(['success' => false, 'message' => 'This downtime name already exists. Please enter a different name.']);
    } else {
        echo json_encode(['success' => true, 'message' => 'Downtime name is available.']);
    }
}

function checkDuplicateDowntimeShortcut(PDO $pdo) {
    $downtime_shortcut = trim($_GET['downtime_shortcut'] ?? '');
    $current_idx = $_GET['current_idx'] ?? null;
    
    if (empty($downtime_shortcut)) {
        http_response_code(400);
        echo json_encode(['success' => false, 'message' => 'Downtime shortcut is required.']);
        return;
    }
    
    $sql = "SELECT COUNT(*) FROM info_downtime WHERE downtime_shortcut = ?";
    $params = [$downtime_shortcut];
    
    if ($current_idx) {
        $sql .= " AND idx != ?";
        $params[] = $current_idx;
    }
    
    $stmt = $pdo->prepare($sql);
    $stmt->execute($params);
    $count = (int)$stmt->fetchColumn();
    
    if ($count > 0) {
        echo json_encode(['success' => false, 'message' => 'This shortcut code already exists. Please enter a different shortcut.']);
    } else {
        echo json_encode(['success' => true, 'message' => 'Shortcut code is available.']);
    }
}

function getWarningStats(PDO $pdo) {
    $status_filter = $_GET['status_filter'] ?? '';
    
    // 기본 쿼리: data_downtime 테이블에서 Warning 상태의 downtime_name별 카운트
    $sql = "
        SELECT dd.downtime_name, COUNT(*) as warning_count
        FROM data_downtime dd
        INNER JOIN info_downtime id ON dd.downtime_name = id.downtime_name
        WHERE dd.status = 'Warning'
    ";
    
    $params = [];
    
    // 필터가 있는 경우 info_downtime 테이블의 status로 필터링
    if (!empty($status_filter)) {
        $sql .= " AND id.status = ?";
        $params[] = $status_filter;
    }
    
    $sql .= " GROUP BY dd.downtime_name ORDER BY dd.downtime_name";
    
    try {
        $stmt = $pdo->prepare($sql);
        $stmt->execute($params);
        $results = $stmt->fetchAll(PDO::FETCH_ASSOC);
        
        // 결과를 연관 배열로 변환 (downtime_name => warning_count)
        $warning_counts = [];
        foreach ($results as $row) {
            $warning_counts[$row['downtime_name']] = (int)$row['warning_count'];
        }
        
        echo json_encode([
            'success' => true,
            'data' => $warning_counts
        ]);
        
    } catch (PDOException $e) {
        http_response_code(500);
        echo json_encode([
            'success' => false, 
            'message' => 'Database error while fetching warning statistics: ' . $e->getMessage()
        ]);
    }
}

