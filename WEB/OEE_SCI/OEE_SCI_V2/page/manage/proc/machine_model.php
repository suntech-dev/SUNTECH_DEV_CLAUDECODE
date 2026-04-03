<?php
/**
 * proc/machine_model.php — 기계 모델(Machine Model) 관리 REST API
 * GET    : 목록/단건 조회, 중복 체크, 통계
 * POST   : 모델 추가
 * PUT    : 모델 수정 (_method 오버라이드 지원)
 * DELETE : 미지원 (405 반환)
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
            $for = $_GET['for'] ?? '';
            if ($for === 'check-duplicate') {
                // 모델명 중복 여부 확인
                checkDuplicateMachineModel($pdo);
            } elseif ($for === 'statistics') {
                // 모델 통계 요약 반환
                getMachineModelStatistics($pdo);
            } elseif (isset($_GET['id'])) {
                // 특정 idx 모델 단건 조회
                getMachineModel($pdo);
            } else {
                // 모델 목록 조회 (페이징, 필터, 정렬)
                getMachineModels($pdo);
            }
            break;
        case 'POST':
            // 신규 모델 추가
            addMachineModel($pdo);
            break;
        case 'PUT':
            // 기존 모델 수정
            updateMachineModel($pdo);
            break;
        case 'DELETE':
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
 * - sort/order: 화이트리스트로 SQL 인젝션 방지
 * - status_filter: 사용 여부(Y/N) 필터
 * - search: machine_model_name, remark 대상 LIKE 검색
 */
function parse_machine_model_list_params(): array {
    $sort_column = $_GET['sort'] ?? 'machine_model_name';
    $sort_order  = strtoupper($_GET['order'] ?? 'ASC');

    // 허용된 정렬 컬럼 화이트리스트 (machine_count는 집계 컬럼)
    $valid_columns = ['idx', 'machine_model_name', 'status', 'remark', 'machine_count', 'type'];
    if (!in_array($sort_column, $valid_columns)) $sort_column = 'machine_model_name';
    if (!in_array($sort_order, ['ASC', 'DESC']))  $sort_order  = 'ASC';

    $status_filter = $_GET['status_filter'] ?? '';
    $search_query  = trim($_GET['search']   ?? '');

    $where_conditions = [];
    $params           = [];

    if (!empty($status_filter)) {
        $where_conditions[] = 'mm.status = ?';
        $params[]           = $status_filter;
    }
    // 모델명 또는 비고(remark) 검색
    if (!empty($search_query)) {
        $where_conditions[] = '(mm.machine_model_name LIKE ? OR mm.remark LIKE ?)';
        $p = '%' . $search_query . '%';
        $params = array_merge($params, [$p, $p]);
    }

    $where_sql = $where_conditions ? ' WHERE ' . implode(' AND ', $where_conditions) : '';

    return ['where_sql' => $where_sql, 'params' => $params, 'sort_column' => $sort_column, 'sort_order' => $sort_order];
}

/**
 * 기계 모델 목록 조회 (페이징 + 필터 + 정렬)
 * - LEFT JOIN info_machine: 해당 모델에 등록된 기계 수(machine_count) 집계
 * - GROUP BY로 모델별 기계 수 산출
 * - COUNT(DISTINCT mm.idx): GROUP BY 사용 시 정확한 전체 건수 산출
 */
function getMachineModels(PDO $pdo): void {
    $cond        = parse_machine_model_list_params();
    $where_sql   = $cond['where_sql'];
    $params      = $cond['params'];
    $sort_column = $cond['sort_column'];
    $sort_order  = $cond['sort_order'];

    $page   = (int)($_GET['page']  ?? 1);
    $limit  = (int)($_GET['limit'] ?? 10);
    $offset = ($page - 1) * $limit;

    // GROUP BY 사용 시 COUNT(DISTINCT)로 정확한 전체 건수 산출
    $total_stmt = $pdo->prepare("SELECT COUNT(DISTINCT mm.idx) FROM info_machine_model mm{$where_sql}");
    $total_stmt->execute($params);
    $total_records = (int)$total_stmt->fetchColumn();

    // 모델 정보 + 기계 수 함께 조회
    $sql = "SELECT mm.idx, mm.machine_model_name, mm.type, mm.status, mm.remark,
                   COUNT(m.idx) AS machine_count
            FROM info_machine_model mm
            LEFT JOIN info_machine m ON mm.idx = m.machine_model_idx
            {$where_sql}
            GROUP BY mm.idx, mm.machine_model_name, mm.type, mm.status, mm.remark
            ORDER BY mm.`{$sort_column}` {$sort_order}
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
 * 기계 모델 단건 조회
 * - 수정 폼 데이터 로드 시 사용
 * - 해당 모델에 등록된 기계 수(machine_count)도 함께 반환
 */
function getMachineModel(PDO $pdo): void {
    $id = $_GET['id'] ?? 0;
    if (empty($id)) {
        http_response_code(400);
        echo json_encode(['success' => false, 'message' => 'ID is required.']);
        return;
    }
    $stmt = $pdo->prepare(
        "SELECT mm.idx, mm.machine_model_name, mm.type, mm.status, mm.remark,
                COUNT(m.idx) AS machine_count
         FROM info_machine_model mm
         LEFT JOIN info_machine m ON mm.idx = m.machine_model_idx
         WHERE mm.idx = ?
         GROUP BY mm.idx, mm.machine_model_name, mm.type, mm.status, mm.remark"
    );
    $stmt->execute([$id]);
    $row = $stmt->fetch(PDO::FETCH_ASSOC);
    if ($row) {
        echo json_encode(['success' => true, 'data' => $row]);
    } else {
        http_response_code(404);
        echo json_encode(['success' => false, 'message' => 'Machine model not found.']);
    }
}

/**
 * 신규 기계 모델 추가 (POST)
 * - machine_model_name 필수 검증
 * - type: P=패턴재봉기(Computer Sewing Machine), E=자수기(Embroidery Machine)
 * - UNIQUE 위반 시 409 반환
 */
function addMachineModel(PDO $pdo): void {
    $machine_model_name = trim($_POST['machine_model_name'] ?? '');
    $status             = $_POST['status'] ?? 'Y';
    $remark             = trim($_POST['remark'] ?? '');

    if (empty($machine_model_name)) {
        http_response_code(400);
        echo json_encode(['success' => false, 'message' => 'Machine model name is required.']);
        return;
    }
    try {
        $stmt = $pdo->prepare("INSERT INTO info_machine_model (machine_model_name, type, status, remark, reg_date, update_date) VALUES (?, ?, ?, ?, NOW(), NOW())");
        $stmt->execute([$machine_model_name, $_POST['type'] ?? 'P', $status, $remark]);
        http_response_code(201);
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
 * 기계 모델 수정 (PUT)
 * - idx, machine_model_name 필수 검증
 * - type 변경 가능 (P ↔ E)
 */
function updateMachineModel(PDO $pdo): void {
    $id                 = $_POST['idx'] ?? 0;
    $machine_model_name = trim($_POST['machine_model_name'] ?? '');
    $status             = $_POST['status'] ?? 'Y';
    $remark             = trim($_POST['remark'] ?? '');

    if (empty($id) || empty($machine_model_name)) {
        http_response_code(400);
        echo json_encode(['success' => false, 'message' => 'ID and Machine model name are required.']);
        return;
    }
    try {
        $stmt = $pdo->prepare("UPDATE info_machine_model SET machine_model_name = ?, type = ?, status = ?, remark = ?, update_date = NOW() WHERE idx = ?");
        $stmt->execute([$machine_model_name, $_POST['type'] ?? 'P', $status, $remark, $id]);
        echo json_encode(['success' => true, 'message' => 'Machine model updated successfully.']);
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
 * 기계 모델 통계 조회
 * - 전체 모델 수(total_models)
 * - 모델별 기계 수(machine_count) 목록 반환
 * - status_filter로 사용 중인 모델만 필터 가능
 */
function getMachineModelStatistics(PDO $pdo): void {
    $status_filter    = $_GET['status_filter'] ?? '';
    $where_conditions = [];
    $params           = [];

    if (!empty($status_filter)) {
        $where_conditions[] = 'mm.status = ?';
        $params[]           = $status_filter;
    }
    $where_sql = $where_conditions ? 'WHERE ' . implode(' AND ', $where_conditions) : '';

    // 전체 모델 수 카운트
    $total_stmt = $pdo->prepare("SELECT COUNT(*) FROM info_machine_model mm {$where_sql}");
    $total_stmt->execute($params);
    $total_models = (int)$total_stmt->fetchColumn();

    // 모델별 기계 수 집계 (통계 테이블 구성용)
    $sql = "SELECT mm.machine_model_name, mm.type, mm.status, COUNT(m.idx) AS machine_count
            FROM info_machine_model mm
            LEFT JOIN info_machine m ON mm.idx = m.machine_model_idx
            {$where_sql}
            GROUP BY mm.idx, mm.machine_model_name, mm.type, mm.status
            ORDER BY mm.machine_model_name";
    $stmt = $pdo->prepare($sql);
    $stmt->execute($params);

    echo json_encode([
        'success' => true,
        'data'    => [
            'total_models' => $total_models,
            'model_stats'  => $stmt->fetchAll(PDO::FETCH_ASSOC),
        ],
    ]);
}

/**
 * 기계 모델명 중복 여부 확인 (실시간 폼 검증용)
 * - current_idx가 있으면 자기 자신 제외 (수정 모드)
 */
function checkDuplicateMachineModel(PDO $pdo): void {
    $machine_model_name = trim($_GET['machine_model_name'] ?? '');
    $current_idx        = $_GET['current_idx'] ?? null;

    if (empty($machine_model_name)) {
        http_response_code(400);
        echo json_encode(['success' => false, 'message' => 'Machine model name is required.']);
        return;
    }
    $sql    = "SELECT COUNT(*) FROM info_machine_model WHERE machine_model_name = ?";
    $params = [$machine_model_name];
    if ($current_idx) { $sql .= " AND idx != ?"; $params[] = $current_idx; }

    $stmt = $pdo->prepare($sql);
    $stmt->execute($params);
    echo json_encode((int)$stmt->fetchColumn() > 0
        ? ['success' => false, 'message' => 'This machine model name already exists. Please enter a different name.']
        : ['success' => true,  'message' => 'Machine model name is available.']
    );
}
