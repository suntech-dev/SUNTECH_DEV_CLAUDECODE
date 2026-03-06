<?php
ini_set('display_errors', 1);
error_reporting(E_ALL);

/**
 * info_andon 테이블 관련 API (RESTful)
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
            if (isset($_GET['for']) && $_GET['for'] === 'check-duplicate') {
                checkDuplicateAndon($pdo);
            } elseif (isset($_GET['for']) && $_GET['for'] === 'warning-stats') {
                getWarningStats($pdo);
            } elseif (isset($_GET['id'])) {
                getAndon($pdo);
            } else {
                getAndons($pdo);
            }
            break;
        case 'POST':
            addAndon($pdo);
            break;
        case 'PUT':
            updateAndon($pdo);
            break;
        case 'DELETE':
            deleteAndon($pdo);
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
 * 목록 조회(페이지네이션)와 전체 조회(엑셀)에 사용될 쿼리 파라미터를 파싱하고 유효성을 검사합니다.
 * @return array 정렬 및 필터링 조건 배열
 */
function parse_andon_list_params() {
    // 정렬 파라미터
    $sort_column = $_GET['sort'] ?? 'andon_name';
    $sort_order = strtoupper($_GET['order'] ?? 'ASC');

    // SQL Injection 방지를 위해 정렬 가능한 컬럼을 화이트리스트로 관리
    $valid_columns = ['idx', 'andon_name', 'color', 'status', 'remark', 'total_count', 'completed_count', 'warning_count', 'warning_rate'];
    if (!in_array($sort_column, $valid_columns)) {
        $sort_column = 'andon_name'; // 기본값
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

    // 검색 파라미터 추가 (andon_name, remark에서 검색)
    if (!empty($search_query)) {
        $where_conditions[] = '(andon_name LIKE ? OR remark LIKE ?)';
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
 * 모든 andon 목록을 조회하여 JSON으로 반환합니다.
 * 정렬, 페이지네이션, 필터링 지원.
 * andon_name별 통계 데이터도 함께 제공.
 * @param PDO $pdo PDO 객체
 */
function getAndons(PDO $pdo) {
    $query_conditions = parse_andon_list_params();
    $where_sql = $query_conditions['where_sql'];
    $params = $query_conditions['params'];
    $sort_column = $query_conditions['sort_column'];
    $sort_order = $query_conditions['sort_order'];

    // 페이지네이션 파라미터
    $page = (int)($_GET['page'] ?? 1);
    $limit = (int)($_GET['limit'] ?? 10);
    $offset = ($page - 1) * $limit;

    // 전체 레코드 수 계산 (필터링 적용)
    $total_stmt = $pdo->prepare("SELECT COUNT(*) FROM info_andon" . $where_sql);
    $total_stmt->execute($params);
    $total_records = (int)$total_stmt->fetchColumn();

    // 현재 페이지에 해당하는 데이터와 통계 함께 조회 (필터링, 정렬, 페이지네이션 적용)
    $sql = "SELECT 
                ia.idx, 
                ia.andon_name, 
                ia.color, 
                ia.status, 
                ia.remark,
                COALESCE(stats.total_count, 0) as total_count,
                COALESCE(stats.completed_count, 0) as completed_count,
                COALESCE(stats.warning_count, 0) as warning_count,
                CASE 
                    WHEN COALESCE(stats.total_count, 0) = 0 THEN 0
                    ELSE ROUND((COALESCE(stats.warning_count, 0) * 100.0 / stats.total_count), 1)
                END as warning_rate
            FROM info_andon ia
            LEFT JOIN (
                SELECT 
                    andon_name,
                    COUNT(*) as total_count,
                    SUM(CASE WHEN status != 'Warning' THEN 1 ELSE 0 END) as completed_count,
                    SUM(CASE WHEN status = 'Warning' THEN 1 ELSE 0 END) as warning_count
                FROM data_andon 
                WHERE andon_name IS NOT NULL AND andon_name != ''
                GROUP BY andon_name
            ) stats ON ia.andon_name = stats.andon_name
            {$where_sql} 
            ORDER BY `{$sort_column}` {$sort_order} 
            LIMIT {$limit} OFFSET {$offset}";
    
    $stmt = $pdo->prepare($sql);
    $stmt->execute($params);
    $andons = $stmt->fetchAll(PDO::FETCH_ASSOC);

    // 최종 응답 데이터 구성
    echo json_encode([
        'success' => true,
        'data' => $andons,
        'pagination' => [
            'total_records' => $total_records,
            'current_page' => $page,
            'total_pages' => ceil($total_records / $limit)
        ]
    ]);
}

/**
 * 특정 andon 정보를 조회하여 JSON으로 반환합니다.
 * @param PDO $pdo PDO 객체
 */
function getAndon(PDO $pdo) {
    $id = $_GET['id'] ?? 0;
    if (empty($id)) {
        http_response_code(400); // Bad Request
        echo json_encode(['success' => false, 'message' => 'ID is required.']);
        return;
    }
    $stmt = $pdo->prepare("SELECT idx, andon_name, color, status, remark FROM info_andon WHERE idx = ?");
    $stmt->execute([$id]);
    $andon = $stmt->fetch(PDO::FETCH_ASSOC);
    if ($andon) {
        echo json_encode(['success' => true, 'data' => $andon]);
    } else {
        http_response_code(404); // Not Found
        echo json_encode(['success' => false, 'message' => 'Andon not found.']);
    }
}

/**
 * 신규 andon를 추가합니다.
 * @param PDO $pdo PDO 객체
 */
function addAndon(PDO $pdo) {
    $andon_name = trim($_POST['andon_name'] ?? '');
    $color = trim($_POST['color'] ?? '');
    $status = $_POST['status'] ?? 'Y';
    $remark = trim($_POST['remark'] ?? '');

    if (empty($andon_name)) {
        http_response_code(400); // Bad Request
        echo json_encode(['success' => false, 'message' => 'Andon name is required.']);
        return;
    }

    try {
        $stmt = $pdo->prepare("INSERT INTO info_andon (andon_name, color, status, remark, reg_date, update_date) VALUES (?, ?, ?, ?, NOW(), NOW())");
        $stmt->execute([$andon_name, $color, $status, $remark]);
        http_response_code(201); // Created
        echo json_encode(['success' => true, 'message' => 'Andon added successfully.']);
    } catch (PDOException $e) {
        if ($e->getCode() == '23000') {
            http_response_code(409); // Conflict
            echo json_encode(['success' => false, 'message' => 'Andon name already exists.']);
        } else {
            throw $e; // 상위 핸들러에서 처리하도록 예외를 다시 던짐
        }
    }
}

/**
 * 기존 andon 정보를 수정합니다.
 * @param PDO $pdo PDO 객체
 */
function updateAndon(PDO $pdo) {
    // HTML form에서 _method로 PUT을 전송하므로, 데이터는 $_POST에 있습니다.
    $id = $_POST['idx'] ?? 0;
    $andon_name = trim($_POST['andon_name'] ?? '');
    $color = trim($_POST['color'] ?? '');
    $status = $_POST['status'] ?? 'Y';
    $remark = trim($_POST['remark'] ?? '');

    if (empty($id) || empty($andon_name)) {
        http_response_code(400); // Bad Request
        echo json_encode(['success' => false, 'message' => 'ID and Andon name are required.']);
        return;
    }

    try {
        $stmt = $pdo->prepare("UPDATE info_andon SET andon_name = ?, color = ?, status = ?, remark = ?, update_date = NOW() WHERE idx = ?");
        $stmt->execute([$andon_name, $color, $status, $remark, $id]);
        echo json_encode(['success' => true, 'message' => 'Andon updated successfully.']);
    } catch (PDOException $e) {
        if ($e->getCode() == '23000') {
            http_response_code(409); // Conflict
            echo json_encode(['success' => false, 'message' => 'Andon name already exists.']);
        } else {
            throw $e;
        }
    }
}

/**
 * andon를 삭제합니다.
 * @param PDO $pdo PDO 객체
 */
function deleteAndon(PDO $pdo) {
    $id = $_GET['id'] ?? 0;

    if (empty($id)) {
        http_response_code(400); // Bad Request
        echo json_encode(['success' => false, 'message' => 'ID is required.']);
        return;
    }

    $stmt = $pdo->prepare("DELETE FROM info_andon WHERE idx = ?");
    $stmt->execute([$id]);

    if ($stmt->rowCount() > 0) {
        echo json_encode(['success' => true, 'message' => 'Andon deleted successfully.']);
    } else {
        http_response_code(404); // Not Found
        echo json_encode(['success' => false, 'message' => 'Andon not found or already deleted.']);
    }
}

/**
 * data_andon 테이블에서 andon_name별 Warning 상태 수량 조회
 * status_filter 매개변수를 받아 info_andon 테이블의 필터 적용
 * @param PDO $pdo PDO 객체
 */
function getWarningStats(PDO $pdo) {
    try {
        // 필터 매개변수 가져오기
        $status_filter = $_GET['status_filter'] ?? '';
        
        // 기본 쿼리: data_andon에서 Warning 상태 수량 조회
        $sql = "SELECT da.andon_name, COUNT(*) as warning_count 
                FROM data_andon da";
        
        // status_filter가 있으면 info_andon과 JOIN하여 필터 적용
        if (!empty($status_filter)) {
            $sql .= " INNER JOIN info_andon ia ON da.andon_name = ia.andon_name 
                      WHERE da.status = 'Warning' 
                        AND da.andon_name IS NOT NULL 
                        AND da.andon_name != ''
                        AND ia.status = ?";
            $params = [$status_filter];
        } else {
            $sql .= " WHERE da.status = 'Warning' 
                        AND da.andon_name IS NOT NULL 
                        AND da.andon_name != ''";
            $params = [];
        }
        
        $sql .= " GROUP BY da.andon_name";
        
        $stmt = $pdo->prepare($sql);
        $stmt->execute($params);
        $results = $stmt->fetchAll(PDO::FETCH_ASSOC);
        
        // 배열을 andon_name을 key로 하는 연관배열로 변환
        $warningCounts = [];
        foreach ($results as $row) {
            $warningCounts[$row['andon_name']] = (int)$row['warning_count'];
        }
        
        echo json_encode([
            'success' => true, 
            'data' => $warningCounts
        ]);
    } catch (Exception $e) {
        http_response_code(500);
        echo json_encode([
            'success' => false, 
            'message' => 'Warning stats retrieval failed: ' . $e->getMessage()
        ]);
    }
}

/**
 * 중복 andon 이름 확인
 * @param PDO $pdo PDO 객체
 */
function checkDuplicateAndon(PDO $pdo) {
    $andon_name = trim($_GET['andon_name'] ?? '');
    $current_idx = $_GET['current_idx'] ?? null; // 수정 모드에서 현재 레코드 제외
    
    if (empty($andon_name)) {
        http_response_code(400);
        echo json_encode(['success' => false, 'message' => 'Andon name is required.']);
        return;
    }
    
    // 중복 확인 쿼리
    $sql = "SELECT COUNT(*) FROM info_andon WHERE andon_name = ?";
    $params = [$andon_name];
    
    // 수정 모드에서 현재 레코드 제외
    if ($current_idx) {
        $sql .= " AND idx != ?";
        $params[] = $current_idx;
    }
    
    $stmt = $pdo->prepare($sql);
    $stmt->execute($params);
    $count = (int)$stmt->fetchColumn();
    
    if ($count > 0) {
        echo json_encode(['success' => false, 'message' => 'This andon name already exists. Please enter a different name.']);
    } else {
        echo json_encode(['success' => true, 'message' => 'Andon name is available.']);
    }
}

