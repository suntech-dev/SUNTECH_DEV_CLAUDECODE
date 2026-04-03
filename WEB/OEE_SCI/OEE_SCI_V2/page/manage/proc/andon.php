<?php
/**
 * proc/andon.php — 안돈(Andon) 관리 REST API
 * GET    : 목록/단건 조회, 중복 체크, 경고 통계
 * POST   : 안돈 추가
 * PUT    : 안돈 수정 (_method 오버라이드 지원)
 * DELETE : 안돈 삭제
 *
 * 안돈(Andon): 생산라인의 이상 신호를 알리는 경보 유형 마스터 데이터
 * data_andon 테이블과 JOIN하여 실제 발생 건수/경고율 집계
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
                // 안돈명 중복 여부 확인
                checkDuplicateAndon($pdo);
            } elseif (isset($_GET['for']) && $_GET['for'] === 'warning-stats') {
                // 안돈 유형별 현재 경고(Warning) 건수 반환
                getWarningStats($pdo);
            } elseif (isset($_GET['id'])) {
                // 특정 idx 안돈 단건 조회
                getAndon($pdo);
            } else {
                // 안돈 목록 조회 (발생 통계 포함)
                getAndons($pdo);
            }
            break;
        case 'POST':
            // 신규 안돈 추가
            addAndon($pdo);
            break;
        case 'PUT':
            // 기존 안돈 수정
            updateAndon($pdo);
            break;
        case 'DELETE':
            // 안돈 삭제
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
 * GET 파라미터에서 목록 조회 조건을 파싱하여 반환
 * - sort/order: 화이트리스트 (total_count, warning_count 등 집계 컬럼 포함)
 * - status_filter: 사용 여부(Y/N) 필터
 * - search: andon_name, remark 대상 LIKE 검색
 */
function parse_andon_list_params(): array {
    $sort_column = $_GET['sort'] ?? 'andon_name';
    $sort_order  = strtoupper($_GET['order'] ?? 'ASC');

    // 집계 컬럼도 정렬 허용 (total_count, warning_count, warning_rate 등)
    $valid_columns = ['idx', 'andon_name', 'color', 'status', 'remark', 'total_count', 'completed_count', 'warning_count', 'warning_rate'];
    if (!in_array($sort_column, $valid_columns)) $sort_column = 'andon_name';
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
        $where_conditions[] = '(andon_name LIKE ? OR remark LIKE ?)';
        $p = '%' . $search_query . '%';
        $params = array_merge($params, [$p, $p]);
    }

    $where_sql = $where_conditions ? ' WHERE ' . implode(' AND ', $where_conditions) : '';

    return ['where_sql' => $where_sql, 'params' => $params, 'sort_column' => $sort_column, 'sort_order' => $sort_order];
}

/**
 * 안돈 목록 조회 (페이징 + data_andon 통계 포함)
 * - LEFT JOIN 서브쿼리로 data_andon에서 총 발생 수, 완료 수, 경고 수 집계
 * - warning_rate: 경고 건수 / 총 건수 × 100 (소수점 1자리)
 * - WHERE는 info_andon에 적용 (필터링 후 통계 JOIN)
 */
function getAndons(PDO $pdo): void {
    $cond        = parse_andon_list_params();
    $where_sql   = $cond['where_sql'];
    $params      = $cond['params'];
    $sort_column = $cond['sort_column'];
    $sort_order  = $cond['sort_order'];

    $page   = (int)($_GET['page']  ?? 1);
    $limit  = (int)($_GET['limit'] ?? 10);
    $offset = ($page - 1) * $limit;

    // 필터 조건에 맞는 전체 안돈 수 카운트
    $total_stmt = $pdo->prepare("SELECT COUNT(*) FROM info_andon{$where_sql}");
    $total_stmt->execute($params);
    $total_records = (int)$total_stmt->fetchColumn();

    // data_andon 서브쿼리: 안돈명 기준으로 총/완료/경고 건수 집계
    // warning_rate: 총 건수가 0이면 0, 아니면 (경고 수 / 총 수 × 100) 반올림
    $sql = "SELECT
                ia.idx, ia.andon_name, ia.color, ia.status, ia.remark,
                COALESCE(stats.total_count, 0)     AS total_count,
                COALESCE(stats.completed_count, 0) AS completed_count,
                COALESCE(stats.warning_count, 0)   AS warning_count,
                CASE WHEN COALESCE(stats.total_count, 0) = 0 THEN 0
                     ELSE ROUND(COALESCE(stats.warning_count, 0) * 100.0 / stats.total_count, 1)
                END AS warning_rate
            FROM info_andon ia
            LEFT JOIN (
                SELECT
                    andon_name,
                    COUNT(*) AS total_count,
                    SUM(CASE WHEN status != 'Warning' THEN 1 ELSE 0 END) AS completed_count,
                    SUM(CASE WHEN status  = 'Warning' THEN 1 ELSE 0 END) AS warning_count
                FROM data_andon
                WHERE andon_name IS NOT NULL AND andon_name != ''
                GROUP BY andon_name
            ) stats ON ia.andon_name = stats.andon_name
            {$where_sql}
            ORDER BY `{$sort_column}` {$sort_order}
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
 * 안돈 단건 조회 (수정 폼 데이터 로드용)
 * - idx, andon_name, color, status, remark 반환
 */
function getAndon(PDO $pdo): void {
    $id = $_GET['id'] ?? 0;
    if (empty($id)) {
        http_response_code(400);
        echo json_encode(['success' => false, 'message' => 'ID is required.']);
        return;
    }
    $stmt = $pdo->prepare("SELECT idx, andon_name, color, status, remark FROM info_andon WHERE idx = ?");
    $stmt->execute([$id]);
    $row = $stmt->fetch(PDO::FETCH_ASSOC);
    if ($row) {
        echo json_encode(['success' => true, 'data' => $row]);
    } else {
        http_response_code(404);
        echo json_encode(['success' => false, 'message' => 'Andon not found.']);
    }
}

/**
 * 신규 안돈 추가 (POST)
 * - andon_name 필수 검증
 * - color: 안돈 표시용 HEX 색상 코드
 */
function addAndon(PDO $pdo): void {
    $andon_name = trim($_POST['andon_name'] ?? '');
    $color      = trim($_POST['color']      ?? '');
    $status     = $_POST['status'] ?? 'Y';
    $remark     = trim($_POST['remark'] ?? '');

    if (empty($andon_name)) {
        http_response_code(400);
        echo json_encode(['success' => false, 'message' => 'Andon name is required.']);
        return;
    }
    try {
        $stmt = $pdo->prepare("INSERT INTO info_andon (andon_name, color, status, remark, reg_date, update_date) VALUES (?, ?, ?, ?, NOW(), NOW())");
        $stmt->execute([$andon_name, $color, $status, $remark]);
        http_response_code(201);
        echo json_encode(['success' => true, 'message' => 'Andon added successfully.']);
    } catch (PDOException $e) {
        if ($e->getCode() == '23000') {
            http_response_code(409);
            echo json_encode(['success' => false, 'message' => 'Andon name already exists.']);
        } else {
            throw $e;
        }
    }
}

/**
 * 안돈 수정 (PUT)
 * - color 필드 포함 수정 (Spectrum 컬러피커 값)
 */
function updateAndon(PDO $pdo): void {
    $id         = $_POST['idx'] ?? 0;
    $andon_name = trim($_POST['andon_name'] ?? '');
    $color      = trim($_POST['color']      ?? '');
    $status     = $_POST['status'] ?? 'Y';
    $remark     = trim($_POST['remark'] ?? '');

    if (empty($id) || empty($andon_name)) {
        http_response_code(400);
        echo json_encode(['success' => false, 'message' => 'ID and Andon name are required.']);
        return;
    }
    try {
        $stmt = $pdo->prepare("UPDATE info_andon SET andon_name = ?, color = ?, status = ?, remark = ?, update_date = NOW() WHERE idx = ?");
        $stmt->execute([$andon_name, $color, $status, $remark, $id]);
        echo json_encode(['success' => true, 'message' => 'Andon updated successfully.']);
    } catch (PDOException $e) {
        if ($e->getCode() == '23000') {
            http_response_code(409);
            echo json_encode(['success' => false, 'message' => 'Andon name already exists.']);
        } else {
            throw $e;
        }
    }
}

/**
 * 안돈 삭제 (DELETE)
 * - rowCount() > 0으로 실제 삭제 여부 확인
 * - 이미 삭제된 경우 404 반환
 */
function deleteAndon(PDO $pdo): void {
    $id = $_GET['id'] ?? 0;
    if (empty($id)) {
        http_response_code(400);
        echo json_encode(['success' => false, 'message' => 'ID is required.']);
        return;
    }
    $stmt = $pdo->prepare("DELETE FROM info_andon WHERE idx = ?");
    $stmt->execute([$id]);
    if ($stmt->rowCount() > 0) {
        echo json_encode(['success' => true, 'message' => 'Andon deleted successfully.']);
    } else {
        http_response_code(404);
        echo json_encode(['success' => false, 'message' => 'Andon not found or already deleted.']);
    }
}

/**
 * 안돈 유형별 현재 경고(Warning) 건수 반환
 * - data_andon에서 status = 'Warning' 인 레코드를 안돈명별로 집계
 * - status_filter가 있으면 info_andon INNER JOIN으로 필터링
 * - 결과: { andon_name: warning_count } 형태의 연관 배열
 */
function getWarningStats(PDO $pdo): void {
    $status_filter = $_GET['status_filter'] ?? '';

    $sql    = "SELECT da.andon_name, COUNT(*) AS warning_count FROM data_andon da";
    $params = [];

    if (!empty($status_filter)) {
        // 활성 안돈(info_andon.status = 'Y')만 대상으로 필터링
        $sql   .= " INNER JOIN info_andon ia ON da.andon_name = ia.andon_name
                    WHERE da.status = 'Warning'
                      AND da.andon_name IS NOT NULL AND da.andon_name != ''
                      AND ia.status = ?";
        $params[] = $status_filter;
    } else {
        $sql .= " WHERE da.status = 'Warning' AND da.andon_name IS NOT NULL AND da.andon_name != ''";
    }
    $sql .= " GROUP BY da.andon_name";

    try {
        $stmt = $pdo->prepare($sql);
        $stmt->execute($params);
        // 안돈명을 키로 하는 연관 배열로 변환
        $warningCounts = [];
        foreach ($stmt->fetchAll(PDO::FETCH_ASSOC) as $row) {
            $warningCounts[$row['andon_name']] = (int)$row['warning_count'];
        }
        echo json_encode(['success' => true, 'data' => $warningCounts]);
    } catch (Exception $e) {
        http_response_code(500);
        echo json_encode(['success' => false, 'message' => 'Warning stats retrieval failed: ' . $e->getMessage()]);
    }
}

/**
 * 안돈명 중복 여부 확인 (실시간 폼 검증용)
 * - current_idx가 있으면 자기 자신 제외 (수정 모드)
 */
function checkDuplicateAndon(PDO $pdo): void {
    $andon_name  = trim($_GET['andon_name'] ?? '');
    $current_idx = $_GET['current_idx'] ?? null;

    if (empty($andon_name)) {
        http_response_code(400);
        echo json_encode(['success' => false, 'message' => 'Andon name is required.']);
        return;
    }
    $sql    = "SELECT COUNT(*) FROM info_andon WHERE andon_name = ?";
    $params = [$andon_name];
    if ($current_idx) { $sql .= " AND idx != ?"; $params[] = $current_idx; }

    $stmt = $pdo->prepare($sql);
    $stmt->execute($params);
    echo json_encode((int)$stmt->fetchColumn() > 0
        ? ['success' => false, 'message' => 'This andon name already exists. Please enter a different name.']
        : ['success' => true,  'message' => 'Andon name is available.']
    );
}
