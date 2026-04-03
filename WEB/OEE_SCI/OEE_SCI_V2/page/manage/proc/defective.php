<?php
/**
 * proc/defective.php — 불량 유형(Defective) 관리 REST API
 * GET    : 목록/단건 조회, 이름/단축키 중복 체크, 경고 통계
 * POST   : 불량 유형 추가
 * PUT    : 불량 유형 수정 (_method 오버라이드 지원)
 * DELETE : 불량 유형 삭제
 *
 * defective_shortcut: 현장 작업자가 빠르게 입력할 수 있는 단축 코드
 * usage_rate: 전체 data_defective 건수 대비 해당 불량 유형 발생 비율(%)
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
                // 불량명 중복 여부 확인
                checkDuplicateDefective($pdo);
            } elseif (isset($_GET['for']) && $_GET['for'] === 'check-duplicate-shortcut') {
                // 단축 코드 중복 여부 확인
                checkDuplicateDefectiveShortcut($pdo);
            } elseif (isset($_GET['for']) && $_GET['for'] === 'warning-stats') {
                // 불량 유형별 현재 경고 건수 반환
                getWarningStats($pdo);
            } elseif (isset($_GET['id'])) {
                // 특정 idx 불량 유형 단건 조회
                getDefective($pdo);
            } else {
                // 불량 유형 목록 조회 (발생 통계 포함)
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
 * - total_count, usage_rate: 집계 컬럼으로 DB ORDER BY 불가 → is_calc_sort 플래그로 PHP 정렬 처리
 * - search: defective_name, defective_shortcut, remark 대상 LIKE 검색
 */
function parse_defective_list_params(): array {
    $sort_column = $_GET['sort'] ?? 'defective_name';
    $sort_order  = strtoupper($_GET['order'] ?? 'ASC');

    $valid_columns   = ['idx', 'defective_name', 'defective_shortcut', 'status', 'remark', 'total_count', 'usage_rate'];
    // 집계 컬럼은 DB에서 ORDER BY할 경우 서브쿼리 별칭 충돌 위험 → PHP 정렬로 처리
    $calc_columns    = ['total_count', 'usage_rate'];

    if (!in_array($sort_column, $valid_columns)) $sort_column = 'defective_name';
    if (!in_array($sort_order, ['ASC', 'DESC']))  $sort_order  = 'ASC';

    $status_filter = $_GET['status_filter'] ?? '';
    $search_query  = trim($_GET['search']   ?? '');

    $where_conditions = [];
    $params           = [];

    if (!empty($status_filter)) {
        $where_conditions[] = 'status = ?';
        $params[]           = $status_filter;
    }
    if (!empty($search_query)) {
        $where_conditions[] = '(defective_name LIKE ? OR defective_shortcut LIKE ? OR remark LIKE ?)';
        $p = '%' . $search_query . '%';
        $params = array_merge($params, [$p, $p, $p]);
    }

    $where_sql = $where_conditions ? ' WHERE ' . implode(' AND ', $where_conditions) : '';

    return [
        'where_sql'   => $where_sql,
        'params'      => $params,
        'sort_column' => $sort_column,
        'sort_order'  => $sort_order,
        // 집계 컬럼 정렬 여부 (true이면 PHP usort 사용)
        'is_calc_sort'=> in_array($sort_column, $calc_columns),
    ];
}

/**
 * 불량 유형 목록 조회 (페이징 + data_defective 발생 통계 포함)
 *
 * 통계 서브쿼리 설명:
 * - data_defective에서 불량명별 발생 건수(total_count) 집계
 * - CROSS JOIN으로 전체 발생 건수(total_all_count)와 함께 조회
 * - usage_rate = 해당 불량 건수 / 전체 불량 건수 × 100
 *
 * 집계 컬럼(total_count, usage_rate) 정렬:
 * - DB ORDER BY가 어렵기 때문에 전체 조회 후 PHP usort로 정렬 후 슬라이스
 */
function getDefectives(PDO $pdo): void {
    $cond        = parse_defective_list_params();
    $where_sql   = $cond['where_sql'];
    $params      = $cond['params'];
    $sort_column = $cond['sort_column'];
    $sort_order  = $cond['sort_order'];
    $is_calc     = $cond['is_calc_sort'];

    $page   = (int)($_GET['page']  ?? 1);
    $limit  = (int)($_GET['limit'] ?? 10);
    $offset = ($page - 1) * $limit;

    // 전체 레코드 수 카운트
    $total_stmt = $pdo->prepare("SELECT COUNT(*) FROM info_defective{$where_sql}");
    $total_stmt->execute($params);
    $total_records = (int)$total_stmt->fetchColumn();

    // 통계를 서브쿼리 JOIN으로 한 번에 가져오기
    // CROSS JOIN으로 전체 발생 건수를 모든 행에 포함시켜 usage_rate 계산
    $stats_subquery = "(
        SELECT defective_name,
               COUNT(*) AS total_count,
               total_all.total_all_count
        FROM data_defective
        CROSS JOIN (SELECT COUNT(*) AS total_all_count FROM data_defective) total_all
        GROUP BY defective_name, total_all.total_all_count
    ) stats";

    if ($is_calc) {
        // 집계 컬럼 정렬: 전체 조회 후 PHP usort → array_slice로 페이지 데이터 추출
        $sql = "SELECT d.idx, d.defective_name, d.defective_shortcut, d.status, d.remark,
                       COALESCE(stats.total_count, 0) AS total_count,
                       CASE WHEN COALESCE(stats.total_all_count, 0) > 0
                            THEN ROUND(COALESCE(stats.total_count, 0) / stats.total_all_count * 100, 1)
                            ELSE 0 END AS usage_rate
                FROM info_defective d
                LEFT JOIN {$stats_subquery} ON d.defective_name = stats.defective_name
                {$where_sql}
                ORDER BY defective_name ASC";
        $stmt = $pdo->prepare($sql);
        $stmt->execute($params);
        $all = $stmt->fetchAll(PDO::FETCH_ASSOC);

        // PHP에서 집계 컬럼 기준 정렬 (우주선 연산자 <=>)
        usort($all, function ($a, $b) use ($sort_column, $sort_order) {
            return $sort_order === 'DESC'
                ? $b[$sort_column] <=> $a[$sort_column]
                : $a[$sort_column] <=> $b[$sort_column];
        });

        // 정렬된 전체 결과에서 현재 페이지 데이터만 슬라이스
        $defectives = array_slice($all, $offset, $limit);
    } else {
        // 일반 컬럼 정렬: DB LIMIT/OFFSET 사용
        $sql = "SELECT d.idx, d.defective_name, d.defective_shortcut, d.status, d.remark,
                       COALESCE(stats.total_count, 0) AS total_count,
                       CASE WHEN COALESCE(stats.total_all_count, 0) > 0
                            THEN ROUND(COALESCE(stats.total_count, 0) / stats.total_all_count * 100, 1)
                            ELSE 0 END AS usage_rate
                FROM info_defective d
                LEFT JOIN {$stats_subquery} ON d.defective_name = stats.defective_name
                {$where_sql}
                ORDER BY `{$sort_column}` {$sort_order}
                LIMIT {$limit} OFFSET {$offset}";
        $stmt = $pdo->prepare($sql);
        $stmt->execute($params);
        $defectives = $stmt->fetchAll(PDO::FETCH_ASSOC);
    }

    echo json_encode([
        'success'    => true,
        'data'       => $defectives,
        'pagination' => [
            'total_records' => $total_records,
            'current_page'  => $page,
            'total_pages'   => ceil($total_records / $limit),
        ],
    ]);
}

/**
 * 불량 유형 단건 조회 (수정 폼 데이터 로드용)
 */
function getDefective(PDO $pdo): void {
    $id = $_GET['id'] ?? 0;
    if (empty($id)) {
        http_response_code(400);
        echo json_encode(['success' => false, 'message' => 'ID is required.']);
        return;
    }
    $stmt = $pdo->prepare("SELECT idx, defective_name, defective_shortcut, status, remark FROM info_defective WHERE idx = ?");
    $stmt->execute([$id]);
    $row = $stmt->fetch(PDO::FETCH_ASSOC);
    if ($row) {
        echo json_encode(['success' => true, 'data' => $row]);
    } else {
        http_response_code(404);
        echo json_encode(['success' => false, 'message' => 'Defective not found.']);
    }
}

/**
 * 신규 불량 유형 추가 (POST)
 * - defective_name 필수, defective_shortcut 선택
 */
function addDefective(PDO $pdo): void {
    $defective_name     = trim($_POST['defective_name']     ?? '');
    $defective_shortcut = trim($_POST['defective_shortcut'] ?? '');
    $status             = $_POST['status'] ?? 'Y';
    $remark             = trim($_POST['remark'] ?? '');

    if (empty($defective_name)) {
        http_response_code(400);
        echo json_encode(['success' => false, 'message' => 'Defective name is required.']);
        return;
    }
    try {
        $stmt = $pdo->prepare("INSERT INTO info_defective (defective_name, defective_shortcut, status, remark, reg_date, update_date) VALUES (?, ?, ?, ?, NOW(), NOW())");
        $stmt->execute([$defective_name, $defective_shortcut, $status, $remark]);
        http_response_code(201);
        echo json_encode(['success' => true, 'message' => 'Defective added successfully.']);
    } catch (PDOException $e) {
        if ($e->getCode() == '23000') {
            http_response_code(409);
            echo json_encode(['success' => false, 'message' => 'Defective name already exists.']);
        } else {
            throw $e;
        }
    }
}

/**
 * 불량 유형 수정 (PUT)
 */
function updateDefective(PDO $pdo): void {
    $id                 = $_POST['idx'] ?? 0;
    $defective_name     = trim($_POST['defective_name']     ?? '');
    $defective_shortcut = trim($_POST['defective_shortcut'] ?? '');
    $status             = $_POST['status'] ?? 'Y';
    $remark             = trim($_POST['remark'] ?? '');

    if (empty($id) || empty($defective_name)) {
        http_response_code(400);
        echo json_encode(['success' => false, 'message' => 'ID and Defective name are required.']);
        return;
    }
    try {
        $stmt = $pdo->prepare("UPDATE info_defective SET defective_name = ?, defective_shortcut = ?, status = ?, remark = ?, update_date = NOW() WHERE idx = ?");
        $stmt->execute([$defective_name, $defective_shortcut, $status, $remark, $id]);
        echo json_encode(['success' => true, 'message' => 'Defective updated successfully.']);
    } catch (PDOException $e) {
        if ($e->getCode() == '23000') {
            http_response_code(409);
            echo json_encode(['success' => false, 'message' => 'Defective name already exists.']);
        } else {
            throw $e;
        }
    }
}

/**
 * 불량 유형 삭제 (DELETE)
 */
function deleteDefective(PDO $pdo): void {
    $id = $_GET['id'] ?? 0;
    if (empty($id)) {
        http_response_code(400);
        echo json_encode(['success' => false, 'message' => 'ID is required.']);
        return;
    }
    $stmt = $pdo->prepare("DELETE FROM info_defective WHERE idx = ?");
    $stmt->execute([$id]);
    if ($stmt->rowCount() > 0) {
        echo json_encode(['success' => true, 'message' => 'Defective deleted successfully.']);
    } else {
        http_response_code(404);
        echo json_encode(['success' => false, 'message' => 'Defective not found or already deleted.']);
    }
}

/**
 * 불량명 중복 여부 확인 (실시간 폼 검증용)
 */
function checkDuplicateDefective(PDO $pdo): void {
    $defective_name = trim($_GET['defective_name'] ?? '');
    $current_idx    = $_GET['current_idx'] ?? null;

    if (empty($defective_name)) {
        http_response_code(400);
        echo json_encode(['success' => false, 'message' => 'Defective name is required.']);
        return;
    }
    $sql    = "SELECT COUNT(*) FROM info_defective WHERE defective_name = ?";
    $params = [$defective_name];
    if ($current_idx) { $sql .= " AND idx != ?"; $params[] = $current_idx; }

    $stmt = $pdo->prepare($sql);
    $stmt->execute($params);
    echo json_encode((int)$stmt->fetchColumn() > 0
        ? ['success' => false, 'message' => 'This defective name already exists. Please enter a different name.']
        : ['success' => true,  'message' => 'Defective name is available.']
    );
}

/**
 * 단축 코드(shortcut) 중복 여부 확인
 * - defective_name과 별도로 shortcut도 UNIQUE 제약이 있으므로 따로 검사
 * - current_idx가 있으면 자기 자신 제외 (수정 모드)
 */
function checkDuplicateDefectiveShortcut(PDO $pdo): void {
    $defective_shortcut = trim($_GET['defective_shortcut'] ?? '');
    $current_idx        = $_GET['current_idx'] ?? null;

    if (empty($defective_shortcut)) {
        http_response_code(400);
        echo json_encode(['success' => false, 'message' => 'Defective shortcut is required.']);
        return;
    }
    $sql    = "SELECT COUNT(*) FROM info_defective WHERE defective_shortcut = ?";
    $params = [$defective_shortcut];
    if ($current_idx) { $sql .= " AND idx != ?"; $params[] = $current_idx; }

    $stmt = $pdo->prepare($sql);
    $stmt->execute($params);
    echo json_encode((int)$stmt->fetchColumn() > 0
        ? ['success' => false, 'message' => 'This shortcut code already exists. Please enter a different shortcut.']
        : ['success' => true,  'message' => 'Shortcut code is available.']
    );
}

/**
 * 불량 유형별 현재 경고(Warning) 건수 반환
 * - data_defective와 info_defective INNER JOIN으로 유효한 불량명만 집계
 * - status_filter가 있으면 활성 불량(info_defective.status = 'Y')만 대상
 * - 결과: { defective_name: warning_count } 형태의 연관 배열
 */
function getWarningStats(PDO $pdo): void {
    $status_filter = $_GET['status_filter'] ?? '';

    // data_defective에서 status='Warning'인 레코드를 불량명별로 집계
    $sql    = "SELECT dd.defective_name, COUNT(*) AS warning_count
               FROM data_defective dd
               INNER JOIN info_defective id ON dd.defective_name = id.defective_name
               WHERE dd.status = 'Warning'";
    $params = [];

    if (!empty($status_filter)) {
        $sql    .= " AND id.status = ?";
        $params[] = $status_filter;
    }
    $sql .= " GROUP BY dd.defective_name ORDER BY dd.defective_name";

    try {
        $stmt = $pdo->prepare($sql);
        $stmt->execute($params);
        // 불량명을 키로 하는 연관 배열로 변환
        $warning_counts = [];
        foreach ($stmt->fetchAll(PDO::FETCH_ASSOC) as $row) {
            $warning_counts[$row['defective_name']] = (int)$row['warning_count'];
        }
        echo json_encode(['success' => true, 'data' => $warning_counts]);
    } catch (PDOException $e) {
        http_response_code(500);
        echo json_encode(['success' => false, 'message' => 'Database error while fetching warning statistics: ' . $e->getMessage()]);
    }
}
