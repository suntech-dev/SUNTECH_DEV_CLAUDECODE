<?php
ini_set('display_errors', 1);
error_reporting(E_ALL);

/**
 * Defective Management API (RESTful)
 * Handles C.R.U.D operations and Excel export functionality.
 */

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
            if (isset($_GET['for']) && $_GET['for'] === 'check-duplicate') {
                checkDuplicateDefective($pdo);
            } elseif (isset($_GET['for']) && $_GET['for'] === 'check-duplicate-shortcut') {
                checkDuplicateDefectiveShortcut($pdo);
            } elseif (isset($_GET['for']) && $_GET['for'] === 'warning-stats') {
                getWarningStats($pdo);
            } elseif (isset($_GET['id'])) {
                getDefective($pdo);
            } else {
                getDefectives($pdo);
            }
            break;
        case 'POST':
            addDefective($pdo);
            break;
        case 'PUT':
            updateDefective($pdo);
            break;
        case 'DELETE':
            deleteDefective($pdo);
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
function parse_defective_list_params() {
    // 정렬 파라미터
    $sort_column = $_GET['sort'] ?? 'defective_name';
    $sort_order = strtoupper($_GET['order'] ?? 'ASC');

    // SQL Injection 방지를 위해 정렬 가능한 컬럼을 화이트리스트로 관리
    $valid_columns = ['idx', 'defective_name', 'defective_shortcut', 'status', 'remark', 'total_count', 'usage_rate'];
    if (!in_array($sort_column, $valid_columns)) {
        $sort_column = 'defective_name'; // 기본값
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
        $where_conditions[] = 'status = ?';
        $params[] = $status_filter;
    }

    // 검색 파라미터 추가 (defective_name, defective_shortcut, remark에서 검색)
    if (!empty($search_query)) {
        $where_conditions[] = '(defective_name LIKE ? OR defective_shortcut LIKE ? OR remark LIKE ?)';
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
        'sort_column' => $sort_column,
        'sort_order' => $sort_order
    ];
}

/**
 * 모든 defective 목록을 조회하여 JSON으로 반환합니다.
 * 정렬, 페이지네이션, 필터링 지원.
 * @param PDO $pdo PDO 객체
 */
function getDefectives(PDO $pdo) {
    $query_conditions = parse_defective_list_params();
    $where_sql = $query_conditions['where_sql'];
    $params = $query_conditions['params'];
    $sort_column = $query_conditions['sort_column'];
    $sort_order = $query_conditions['sort_order'];

    // 페이지네이션 파라미터
    $page = (int)($_GET['page'] ?? 1);
    $limit = (int)($_GET['limit'] ?? 10);
    $offset = ($page - 1) * $limit;

    // 전체 레코드 수 계산 (필터링 적용)
    $total_stmt = $pdo->prepare("SELECT COUNT(*) FROM info_defective" . $where_sql);
    $total_stmt->execute($params);
    $total_records = (int)$total_stmt->fetchColumn();

    // 계산된 필드에 대한 정렬인지 확인
    $calculated_fields = ['total_count', 'usage_rate'];
    $is_calculated_sort = in_array($sort_column, $calculated_fields);

    if ($is_calculated_sort) {
        // 계산된 필드로 정렬하는 경우: 전체 데이터를 가져와서 정렬
        $sql = "SELECT idx, defective_name, defective_shortcut, status, remark FROM info_defective {$where_sql} ORDER BY defective_name ASC";
        $stmt = $pdo->prepare($sql);
        $stmt->execute($params);
        $all_defectives = $stmt->fetchAll(PDO::FETCH_ASSOC);
        
        // 통계 정보 추가
        $all_defectives = addDefectiveStatistics($pdo, $all_defectives);
        
        // 계산된 필드로 정렬
        usort($all_defectives, function($a, $b) use ($sort_column, $sort_order) {
            $val_a = $a[$sort_column];
            $val_b = $b[$sort_column];
            
            if ($sort_order === 'DESC') {
                return $val_b <=> $val_a;
            } else {
                return $val_a <=> $val_b;
            }
        });
        
        // 페이지네이션 적용
        $defectives = array_slice($all_defectives, $offset, $limit);
    } else {
        // 일반 필드로 정렬하는 경우: 기존 방식
        $sql = "SELECT idx, defective_name, defective_shortcut, status, remark FROM info_defective {$where_sql} ORDER BY `{$sort_column}` {$sort_order} LIMIT {$limit} OFFSET {$offset}";
        $stmt = $pdo->prepare($sql);
        $stmt->execute($params);
        $defectives = $stmt->fetchAll(PDO::FETCH_ASSOC);
        
        // 각 defective에 대한 통계 정보 추가
        $defectives = addDefectiveStatistics($pdo, $defectives);
    }

    // 최종 응답 데이터 구성
    echo json_encode([
        'success' => true,
        'data' => $defectives,
        'pagination' => [
            'total_records' => $total_records,
            'current_page' => $page,
            'total_pages' => ceil($total_records / $limit)
        ]
    ]);
}

/**
 * 특정 defective 정보를 조회하여 JSON으로 반환합니다.
 * @param PDO $pdo PDO 객체
 */
function getDefective(PDO $pdo) {
    $id = $_GET['id'] ?? 0;
    if (empty($id)) {
        http_response_code(400); // Bad Request
        echo json_encode(['success' => false, 'message' => 'ID is required.']);
        return;
    }
    $stmt = $pdo->prepare("SELECT idx, defective_name, defective_shortcut, status, remark FROM info_defective WHERE idx = ?");
    $stmt->execute([$id]);
    $defective = $stmt->fetch(PDO::FETCH_ASSOC);
    if ($defective) {
        echo json_encode(['success' => true, 'data' => $defective]);
    } else {
        http_response_code(404); // Not Found
        echo json_encode(['success' => false, 'message' => 'Defective not found.']);
    }
}

/**
 * 신규 defective를 추가합니다.
 * @param PDO $pdo PDO 객체
 */
function addDefective(PDO $pdo) {
    $defective_name = trim($_POST['defective_name'] ?? '');
    $defective_shortcut = trim($_POST['defective_shortcut'] ?? '');
    $status = $_POST['status'] ?? 'Y';
    $remark = trim($_POST['remark'] ?? '');

    if (empty($defective_name)) {
        http_response_code(400); // Bad Request
        echo json_encode(['success' => false, 'message' => 'Defective name is required.']);
        return;
    }

    try {
        $stmt = $pdo->prepare("INSERT INTO info_defective (defective_name, defective_shortcut, status, remark, reg_date, update_date) VALUES (?, ?, ?, ?, NOW(), NOW())");
        $stmt->execute([$defective_name, $defective_shortcut, $status, $remark]);
        http_response_code(201); // Created
        echo json_encode(['success' => true, 'message' => 'Defective added successfully.']);
    } catch (PDOException $e) {
        if ($e->getCode() == '23000') {
            http_response_code(409); // Conflict
            echo json_encode(['success' => false, 'message' => 'Defective name already exists.']);
        } else {
            throw $e; // 상위 핸들러에서 처리하도록 예외를 다시 던짐
        }
    }
}

/**
 * 기존 defective 정보를 수정합니다.
 * @param PDO $pdo PDO 객체
 */
function updateDefective(PDO $pdo) {
    // HTML form에서 _method로 PUT을 전송하므로, 데이터는 $_POST에 있습니다.
    $id = $_POST['idx'] ?? 0;
    $defective_name = trim($_POST['defective_name'] ?? '');
    $defective_shortcut = trim($_POST['defective_shortcut'] ?? '');
    $status = $_POST['status'] ?? 'Y';
    $remark = trim($_POST['remark'] ?? '');

    if (empty($id) || empty($defective_name)) {
        http_response_code(400); // Bad Request
        echo json_encode(['success' => false, 'message' => 'ID and Defective name are required.']);
        return;
    }

    try {
        $stmt = $pdo->prepare("UPDATE info_defective SET defective_name = ?, defective_shortcut = ?, status = ?, remark = ?, update_date = NOW() WHERE idx = ?");
        $stmt->execute([$defective_name, $defective_shortcut, $status, $remark, $id]);
        echo json_encode(['success' => true, 'message' => 'Defective updated successfully.']);
    } catch (PDOException $e) {
        if ($e->getCode() == '23000') {
            http_response_code(409); // Conflict
            echo json_encode(['success' => false, 'message' => 'Defective name already exists.']);
        } else {
            throw $e;
        }
    }
}

/**
 * defective를 삭제합니다.
 * @param PDO $pdo PDO 객체
 */
function deleteDefective(PDO $pdo) {
    $id = $_GET['id'] ?? 0;

    if (empty($id)) {
        http_response_code(400); // Bad Request
        echo json_encode(['success' => false, 'message' => 'ID is required.']);
        return;
    }

    $stmt = $pdo->prepare("DELETE FROM info_defective WHERE idx = ?");
    $stmt->execute([$id]);

    if ($stmt->rowCount() > 0) {
        echo json_encode(['success' => true, 'message' => 'Defective deleted successfully.']);
    } else {
        http_response_code(404); // Not Found
        echo json_encode(['success' => false, 'message' => 'Defective not found or already deleted.']);
    }
}

/**
 * 중복 defective 이름 확인
 * @param PDO $pdo PDO 객체
 */
function checkDuplicateDefective(PDO $pdo) {
    $defective_name = trim($_GET['defective_name'] ?? '');
    $current_idx = $_GET['current_idx'] ?? null; // 수정 모드에서 현재 레코드 제외
    
    if (empty($defective_name)) {
        http_response_code(400);
        echo json_encode(['success' => false, 'message' => 'Defective name is required.']);
        return;
    }
    
    // 중복 확인 쿼리
    $sql = "SELECT COUNT(*) FROM info_defective WHERE defective_name = ?";
    $params = [$defective_name];
    
    // 수정 모드에서 현재 레코드 제외
    if ($current_idx) {
        $sql .= " AND idx != ?";
        $params[] = $current_idx;
    }
    
    $stmt = $pdo->prepare($sql);
    $stmt->execute($params);
    $count = (int)$stmt->fetchColumn();
    
    if ($count > 0) {
        echo json_encode(['success' => false, 'message' => 'This defective name already exists. Please enter a different name.']);
    } else {
        echo json_encode(['success' => true, 'message' => 'Defective name is available.']);
    }
}

/**
 * 중복 defective shortcut 확인
 * @param PDO $pdo PDO 객체
 */
function checkDuplicateDefectiveShortcut(PDO $pdo) {
    $defective_shortcut = trim($_GET['defective_shortcut'] ?? '');
    $current_idx = $_GET['current_idx'] ?? null; // 수정 모드에서 현재 레코드 제외
    
    if (empty($defective_shortcut)) {
        http_response_code(400);
        echo json_encode(['success' => false, 'message' => 'Defective shortcut is required.']);
        return;
    }
    
    // 중복 확인 쿼리
    $sql = "SELECT COUNT(*) FROM info_defective WHERE defective_shortcut = ?";
    $params = [$defective_shortcut];
    
    // 수정 모드에서 현재 레코드 제외
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

/**
 * defective 목록에 통계 정보를 추가하는 함수
 * @param PDO $pdo PDO 객체
 * @param array $defectives defective 목록 배열
 * @return array 통계 정보가 추가된 defective 목록 배열
 */
function addDefectiveStatistics(PDO $pdo, $defectives) {
    // data_defective 테이블에서 전체 레코드 수 계산
    $total_count_stmt = $pdo->prepare("SELECT COUNT(*) FROM data_defective");
    $total_count_stmt->execute();
    $total_all_records = (int)$total_count_stmt->fetchColumn();
    
    // defective_name별 수량 계산
    $stats_stmt = $pdo->prepare("
        SELECT defective_name, COUNT(*) as total_count 
        FROM data_defective 
        GROUP BY defective_name
    ");
    $stats_stmt->execute();
    $defective_stats = $stats_stmt->fetchAll(PDO::FETCH_ASSOC);
    
    // 통계 정보를 연관 배열로 변환
    $stats_map = [];
    foreach ($defective_stats as $stat) {
        $stats_map[$stat['defective_name']] = (int)$stat['total_count'];
    }
    
    // 각 defective에 통계 정보 추가
    foreach ($defectives as &$defective) {
        $defective_name = $defective['defective_name'];
        $total_count = $stats_map[$defective_name] ?? 0;
        
        // 비율 계산 (전체 레코드 대비 해당 defective의 비율)
        $usage_rate = 0;
        if ($total_all_records > 0) {
            $usage_rate = round(($total_count / $total_all_records) * 100, 1);
        }
        
        $defective['total_count'] = $total_count;
        $defective['usage_rate'] = $usage_rate;
    }
    
    return $defectives;
}

/**
 * Warning 통계 조회 함수
 * data_defective 테이블에서 defective_name별 Warning 수량을 반환
 * @param PDO $pdo PDO 객체
 */
function getWarningStats(PDO $pdo) {
    $status_filter = $_GET['status_filter'] ?? '';
    
    // 기본 쿼리: data_defective 테이블에서 Warning 상태의 defective_name별 카운트
    $sql = "
        SELECT dd.defective_name, COUNT(*) as warning_count
        FROM data_defective dd
        INNER JOIN info_defective id ON dd.defective_name = id.defective_name
        WHERE dd.status = 'Warning'
    ";
    
    $params = [];
    
    // 필터가 있는 경우 info_defective 테이블의 status로 필터링
    if (!empty($status_filter)) {
        $sql .= " AND id.status = ?";
        $params[] = $status_filter;
    }
    
    $sql .= " GROUP BY dd.defective_name ORDER BY dd.defective_name";
    
    try {
        $stmt = $pdo->prepare($sql);
        $stmt->execute($params);
        $results = $stmt->fetchAll(PDO::FETCH_ASSOC);
        
        // 결과를 연관 배열로 변환 (defective_name => warning_count)
        $warning_counts = [];
        foreach ($results as $row) {
            $warning_counts[$row['defective_name']] = (int)$row['warning_count'];
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

