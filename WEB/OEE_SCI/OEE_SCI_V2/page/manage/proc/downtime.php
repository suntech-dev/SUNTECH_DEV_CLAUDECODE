<?php
/**
 * proc/downtime.php — 비가동(Downtime) 유형 관리 REST API
 * GET    : 목록/단건 조회, 이름/단축키 중복 체크, 경고 통계
 * POST   : 비가동 유형 추가
 * PUT    : 비가동 유형 수정 (_method 오버라이드 지원)
 * DELETE : 비가동 유형 삭제
 *
 * downtime_shortcut: 현장 작업자가 빠르게 입력할 수 있는 단축 코드
 * data_downtime 테이블과 JOIN하여 총 발생/완료/경고 건수 및 경고율 집계
 */
require_once(__DIR__ . '/../../../lib/config.php');
require_once(__DIR__ . '/../../../lib/db.php');

header('Content-Type: application/json');

// HTML Form의 PUT 불가 문제를 _method 필드로 우회
$method = $_SERVER['REQUEST_METHOD'];
if ($method === 'POST' && isset($_POST['_method'])) {
    $method = strtoupper($_POST['_method']);
}

// HTTP 메서드 및 'for' 파라미터에 따라 처리 함수 분기
try {
    switch ($method) {
        case 'GET':
            if (isset($_GET['for']) && $_GET['for'] === 'check-duplicate') {
                // 비가동명 중복 여부 확인
                checkDuplicateDowntime($pdo);
            } elseif (isset($_GET['for']) && $_GET['for'] === 'check-duplicate-shortcut') {
                // 단축 코드 중복 여부 확인
                checkDuplicateDowntimeShortcut($pdo);
            } elseif (isset($_GET['for']) && $_GET['for'] === 'warning-stats') {
                // 비가동 유형별 현재 경고 건수 반환
                getWarningStats($pdo);
            } elseif (isset($_GET['id'])) {
                // 특정 idx 비가동 유형 단건 조회
                getDowntime($pdo);
            } else {
                // 비가동 유형 목록 조회 (발생 통계 포함)
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


/**
 * GET 파라미터에서 목록 조회 조건을 파싱하여 반환
 * - order_column: 테이블 별칭(id.) 포함 여부를 stats_columns로 구분
 *   (통계 컬럼은 서브쿼리 별칭 그대로, 일반 컬럼은 'id.' 접두어 추가)
 * - search: downtime_name, downtime_shortcut, remark 대상 LIKE 검색
 */
function parse_downtime_list_params(): array {
    $sort_column = $_GET['sort'] ?? 'downtime_name';
    $sort_order  = strtoupper($_GET['order'] ?? 'ASC');

    $valid_columns  = ['idx', 'downtime_name', 'downtime_shortcut', 'status', 'remark', 'total_count', 'completed_count', 'warning_count', 'warning_rate'];
    // 통계 컬럼: 서브쿼리 별칭 그대로 ORDER BY (테이블 별칭 없음)
    $stats_columns  = ['total_count', 'completed_count', 'warning_count', 'warning_rate'];

    if (!in_array($sort_column, $valid_columns)) $sort_column = 'downtime_name';
    if (!in_array($sort_order, ['ASC', 'DESC']))  $sort_order  = 'ASC';

    // 통계 컬럼은 별칭 그대로, 일반 컬럼은 테이블 별칭 추가
    $order_column = in_array($sort_column, $stats_columns) ? $sort_column : 'id.' . $sort_column;

    $status_filter = $_GET['status_filter'] ?? '';
    $search_query  = trim($_GET['search']   ?? '');

    $where_conditions = [];
    $params           = [];

    if (!empty($status_filter)) {
        $where_conditions[] = 'id.status = ?';
        $params[]           = $status_filter;
    }
    if (!empty($search_query)) {
        $where_conditions[] = '(id.downtime_name LIKE ? OR id.downtime_shortcut LIKE ? OR id.remark LIKE ?)';
        $p = '%' . $search_query . '%';
        $params = array_merge($params, [$p, $p, $p]);
    }

    $where_sql = $where_conditions ? ' WHERE ' . implode(' AND ', $where_conditions) : '';

    return ['where_sql' => $where_sql, 'params' => $params, 'order_column' => $order_column, 'sort_order' => $sort_order];
}

/**
 * 비가동 유형 목록 조회 (페이징 + data_downtime 통계 포함)
 *
 * COUNT 쿼리 문제 해결:
 * - 메인 쿼리는 info_downtime에 별칭 'id' 사용 → WHERE 조건에도 'id.' 접두어 필요
 * - 그러나 COUNT 쿼리는 테이블 단독 조회 → 별칭 없이 컬럼명만 사용
 * - 해결: COUNT용 WHERE 조건을 별도로 구성 (테이블 별칭 제거)
 *
 * 통계 서브쿼리: data_downtime에서 비가동명별 총/완료/경고 건수 집계
 * warning_rate: (경고 수 / 총 수) × 100
 */
function getDowntimes(PDO $pdo): void {
    $cond         = parse_downtime_list_params();
    $where_sql    = $cond['where_sql'];
    $params       = $cond['params'];
    $order_column = $cond['order_column'];
    $sort_order   = $cond['sort_order'];

    $page   = (int)($_GET['page']  ?? 1);
    $limit  = (int)($_GET['limit'] ?? 10);
    $offset = ($page - 1) * $limit;

    // COUNT 쿼리는 info_downtime 단독으로 실행 (테이블 별칭 없이)
    // 'id.' 접두어를 제거한 WHERE 조건 별도 구성
    $count_where_sql = '';
    $count_params    = [];
    $status_filter   = $_GET['status_filter'] ?? '';
    $search_query    = trim($_GET['search']   ?? '');
    if (!empty($status_filter)) {
        $count_where_sql .= ($count_where_sql ? ' AND ' : ' WHERE ') . 'status = ?';
        $count_params[]   = $status_filter;
    }
    if (!empty($search_query)) {
        $count_where_sql .= ($count_where_sql ? ' AND ' : ' WHERE ') . '(downtime_name LIKE ? OR downtime_shortcut LIKE ? OR remark LIKE ?)';
        $p = '%' . $search_query . '%';
        $count_params = array_merge($count_params, [$p, $p, $p]);
    }

    $total_stmt = $pdo->prepare("SELECT COUNT(*) FROM info_downtime{$count_where_sql}");
    $total_stmt->execute($count_params);
    $total_records = (int)$total_stmt->fetchColumn();

    // 메인 쿼리: 비가동 정보 + 통계 서브쿼리 LEFT JOIN
    $sql = "
        SELECT
            id.idx, id.downtime_name, id.downtime_shortcut, id.status, id.remark,
            COALESCE(stats.total_count, 0)     AS total_count,
            COALESCE(stats.completed_count, 0) AS completed_count,
            COALESCE(stats.warning_count, 0)   AS warning_count,
            COALESCE(
                CASE WHEN stats.total_count > 0
                     THEN ROUND((stats.warning_count / stats.total_count) * 100, 1)
                     ELSE 0 END,
                0
            ) AS warning_rate
        FROM info_downtime id
        LEFT JOIN (
            SELECT
                downtime_name,
                COUNT(*) AS total_count,
                SUM(CASE WHEN status = 'Completed' THEN 1 ELSE 0 END) AS completed_count,
                SUM(CASE WHEN status = 'Warning'   THEN 1 ELSE 0 END) AS warning_count
            FROM data_downtime
            GROUP BY downtime_name
        ) stats ON id.downtime_name = stats.downtime_name
        {$where_sql}
        ORDER BY {$order_column} {$sort_order}
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
 * 비가동 유형 단건 조회 (수정 폼 데이터 로드용)
 */
function getDowntime(PDO $pdo): void {
    $id = $_GET['id'] ?? 0;
    if (empty($id)) {
        http_response_code(400);
        echo json_encode(['success' => false, 'message' => 'ID is required.']);
        return;
    }
    $stmt = $pdo->prepare("SELECT idx, downtime_name, downtime_shortcut, status, remark FROM info_downtime WHERE idx = ?");
    $stmt->execute([$id]);
    $row = $stmt->fetch(PDO::FETCH_ASSOC);
    if ($row) {
        echo json_encode(['success' => true, 'data' => $row]);
    } else {
        http_response_code(404);
        echo json_encode(['success' => false, 'message' => 'Downtime not found.']);
    }
}

/**
 * 신규 비가동 유형 추가 (POST)
 */
function addDowntime(PDO $pdo): void {
    $downtime_name     = trim($_POST['downtime_name']     ?? '');
    $downtime_shortcut = trim($_POST['downtime_shortcut'] ?? '');
    $status            = $_POST['status'] ?? 'Y';
    $remark            = trim($_POST['remark'] ?? '');

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

/**
 * 비가동 유형 수정 (PUT)
 */
function updateDowntime(PDO $pdo): void {
    $id                = $_POST['idx'] ?? 0;
    $downtime_name     = trim($_POST['downtime_name']     ?? '');
    $downtime_shortcut = trim($_POST['downtime_shortcut'] ?? '');
    $status            = $_POST['status'] ?? 'Y';
    $remark            = trim($_POST['remark'] ?? '');

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

/**
 * 비가동 유형 삭제 (DELETE)
 */
function deleteDowntime(PDO $pdo): void {
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

/**
 * 비가동명 중복 여부 확인 (실시간 폼 검증용)
 */
function checkDuplicateDowntime(PDO $pdo): void {
    $downtime_name = trim($_GET['downtime_name'] ?? '');
    $current_idx   = $_GET['current_idx'] ?? null;

    if (empty($downtime_name)) {
        http_response_code(400);
        echo json_encode(['success' => false, 'message' => 'Downtime name is required.']);
        return;
    }
    $sql    = "SELECT COUNT(*) FROM info_downtime WHERE downtime_name = ?";
    $params = [$downtime_name];
    if ($current_idx) { $sql .= " AND idx != ?"; $params[] = $current_idx; }

    $stmt = $pdo->prepare($sql);
    $stmt->execute($params);
    echo json_encode((int)$stmt->fetchColumn() > 0
        ? ['success' => false, 'message' => 'This downtime name already exists. Please enter a different name.']
        : ['success' => true,  'message' => 'Downtime name is available.']
    );
}

/**
 * 단축 코드(shortcut) 중복 여부 확인
 * - downtime_name과 별도로 shortcut도 UNIQUE 제약이 있으므로 따로 검사
 */
function checkDuplicateDowntimeShortcut(PDO $pdo): void {
    $downtime_shortcut = trim($_GET['downtime_shortcut'] ?? '');
    $current_idx       = $_GET['current_idx'] ?? null;

    if (empty($downtime_shortcut)) {
        http_response_code(400);
        echo json_encode(['success' => false, 'message' => 'Downtime shortcut is required.']);
        return;
    }
    $sql    = "SELECT COUNT(*) FROM info_downtime WHERE downtime_shortcut = ?";
    $params = [$downtime_shortcut];
    if ($current_idx) { $sql .= " AND idx != ?"; $params[] = $current_idx; }

    $stmt = $pdo->prepare($sql);
    $stmt->execute($params);
    echo json_encode((int)$stmt->fetchColumn() > 0
        ? ['success' => false, 'message' => 'This shortcut code already exists. Please enter a different shortcut.']
        : ['success' => true,  'message' => 'Shortcut code is available.']
    );
}

/**
 * 비가동 유형별 현재 경고(Warning) 건수 반환
 * - data_downtime와 info_downtime INNER JOIN으로 유효한 비가동명만 집계
 * - status_filter가 있으면 활성 비가동(info_downtime.status = 'Y')만 대상
 * - 결과: { downtime_name: warning_count } 형태의 연관 배열
 */
function getWarningStats(PDO $pdo): void {
    $status_filter = $_GET['status_filter'] ?? '';

    // data_downtime에서 status='Warning'인 레코드를 비가동명별로 집계
    $sql    = "SELECT dd.downtime_name, COUNT(*) AS warning_count
               FROM data_downtime dd
               INNER JOIN info_downtime id ON dd.downtime_name = id.downtime_name
               WHERE dd.status = 'Warning'";
    $params = [];

    if (!empty($status_filter)) {
        $sql    .= " AND id.status = ?";
        $params[] = $status_filter;
    }
    $sql .= " GROUP BY dd.downtime_name ORDER BY dd.downtime_name";

    try {
        $stmt = $pdo->prepare($sql);
        $stmt->execute($params);
        $results = $stmt->fetchAll(PDO::FETCH_ASSOC);

        // 비가동명을 키로 하는 연관 배열로 변환
        $warning_counts = [];
        foreach ($results as $row) {
            $warning_counts[$row['downtime_name']] = (int)$row['warning_count'];
        }
        echo json_encode(['success' => true, 'data' => $warning_counts]);
    } catch (PDOException $e) {
        http_response_code(500);
        echo json_encode(['success' => false, 'message' => 'Database error while fetching warning statistics: ' . $e->getMessage()]);
    }
}
