<?php
/**
 * proc/factory.php — 공장(Factory) 관리 REST API
 * GET  : 목록 조회, 단건 조회, 중복 체크, 통계 조회
 * POST : 공장 추가
 * PUT  : 공장 수정 (_method 오버라이드 지원)
 */
require_once(__DIR__ . '/../../../lib/config.php');
require_once(__DIR__ . '/../../../lib/db.php');

header('Content-Type: application/json');

// POST Body 내 _method 필드로 PUT 오버라이드 처리 (HTML Form은 PUT을 직접 보낼 수 없으므로)
$method = $_SERVER['REQUEST_METHOD'];
if ($method === 'POST' && isset($_POST['_method'])) {
    $method = strtoupper($_POST['_method']);
}

// HTTP 메서드에 따라 해당 함수 분기 처리
try {
    switch ($method) {
        case 'GET':
            if (isset($_GET['for']) && $_GET['for'] === 'check-duplicate') {
                // 공장명 중복 여부 확인 (폼 실시간 검증에 사용)
                checkDuplicateFactory($pdo);
            } elseif (isset($_GET['for']) && $_GET['for'] === 'statistics') {
                // 공장 통계 요약 (공장 수, 라인 수, 기계 수, 인원 수) 반환
                getStatistics($pdo);
            } elseif (isset($_GET['id'])) {
                // 특정 idx 공장 단건 조회 (수정 폼 로드 시 사용)
                getFactory($pdo);
            } else {
                // 공장 목록 조회 (페이징, 필터, 정렬 지원)
                getFactories($pdo);
            }
            break;
        case 'POST':
            // 신규 공장 추가
            addFactory($pdo);
            break;
        case 'PUT':
            // 기존 공장 정보 수정
            updateFactory($pdo);
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
 * Factory 목록/단건 조회에 공통으로 사용되는 SELECT + LEFT JOIN SQL 반환
 * - info_line 서브쿼리: 라인 수(line_count), 총 인원(total_mp) 집계
 * - info_machine 서브쿼리: 기계 수(machine_count) 집계
 * - COALESCE로 NULL → 0 처리하여 항상 숫자 반환
 * @param string $where_sql WHERE 절 문자열 (없으면 빈 문자열)
 */
function buildFactorySelectFrom(string $where_sql): string {
    return "SELECT
                f.idx, f.factory_name, f.status, f.remark,
                COALESCE(line_stats.line_count, 0)    AS line_count,
                COALESCE(machine_stats.machine_count, 0) AS machine_count,
                COALESCE(line_stats.total_mp, 0)      AS total_mp
            FROM info_factory f
            LEFT JOIN (
                SELECT factory_idx, COUNT(*) AS line_count, SUM(mp) AS total_mp
                FROM info_line GROUP BY factory_idx
            ) line_stats    ON f.idx = line_stats.factory_idx
            LEFT JOIN (
                SELECT factory_idx, COUNT(*) AS machine_count
                FROM info_machine GROUP BY factory_idx
            ) machine_stats ON f.idx = machine_stats.factory_idx
            {$where_sql}";
}

/**
 * GET 파라미터에서 목록 조회 조건(정렬·필터·검색)을 파싱하여 배열로 반환
 * - sort/order: 화이트리스트로 SQL 인젝션 방지
 * - status_filter: 사용 여부(Y/N) 필터
 * - search: factory_name, remark 대상 LIKE 검색
 */
function parse_factory_list_params(): array {
    $sort_column = $_GET['sort']  ?? 'factory_name';
    $sort_order  = strtoupper($_GET['order'] ?? 'ASC');

    // 허용된 정렬 컬럼 화이트리스트 — 임의 컬럼명 주입 차단
    $valid_columns = ['idx', 'factory_name', 'status', 'remark', 'line_count', 'machine_count', 'total_mp'];
    if (!in_array($sort_column, $valid_columns)) $sort_column = 'factory_name';
    if (!in_array($sort_order, ['ASC', 'DESC']))  $sort_order  = 'ASC';

    $status_filter = $_GET['status_filter'] ?? '';
    $search_query  = trim($_GET['search']   ?? '');

    $where_conditions = [];
    $params           = [];

    // 사용 여부 필터 조건 추가
    if (!empty($status_filter)) {
        $where_conditions[] = 'status = ?';
        $params[]           = $status_filter;
    }
    // 검색어가 있으면 factory_name, remark에 LIKE 검색 적용
    if (!empty($search_query)) {
        $where_conditions[] = '(factory_name LIKE ? OR remark LIKE ?)';
        $p        = '%' . $search_query . '%';
        $params[] = $p;
        $params[] = $p;
    }

    $where_sql = $where_conditions ? ' WHERE ' . implode(' AND ', $where_conditions) : '';

    return ['where_sql' => $where_sql, 'params' => $params, 'sort_column' => $sort_column, 'sort_order' => $sort_order];
}

/**
 * 공장 목록 조회 (페이징 + 필터 + 정렬)
 * 1) 전체 레코드 수 COUNT → 페이지네이션 계산
 * 2) buildFactorySelectFrom()으로 라인/기계/인원 통계 포함 SELECT
 * 3) JSON 응답: { success, data[], pagination{} }
 */
function getFactories(PDO $pdo): void {
    $cond        = parse_factory_list_params();
    $where_sql   = $cond['where_sql'];
    $params      = $cond['params'];
    $sort_column = $cond['sort_column'];
    $sort_order  = $cond['sort_order'];

    // 페이지 번호 및 페이지당 레코드 수 처리 (기본값: 1페이지, 10건)
    $page   = (int)($_GET['page']  ?? 1);
    $limit  = (int)($_GET['limit'] ?? 10);
    $offset = ($page - 1) * $limit;

    // 전체 레코드 수 조회 (페이지네이션 계산용)
    $total_stmt = $pdo->prepare("SELECT COUNT(*) FROM info_factory{$where_sql}");
    $total_stmt->execute($params);
    $total_records = (int)$total_stmt->fetchColumn();

    // 통계 포함 목록 조회 (LIMIT/OFFSET으로 현재 페이지 데이터만 가져옴)
    $sql = buildFactorySelectFrom($where_sql)
        . " ORDER BY `{$sort_column}` {$sort_order} LIMIT {$limit} OFFSET {$offset}";
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
 * 공장 단건 조회 (수정 폼 데이터 로드용)
 * - $_GET['id'] 값으로 특정 공장 1건 조회
 * - 라인 수, 기계 수, 인원 통계도 함께 반환
 */
function getFactory(PDO $pdo): void {
    $id = $_GET['id'] ?? 0;
    if (empty($id)) {
        http_response_code(400);
        echo json_encode(['success' => false, 'message' => 'ID is required.']);
        return;
    }
    $sql  = buildFactorySelectFrom('WHERE f.idx = ?');
    $stmt = $pdo->prepare($sql);
    $stmt->execute([$id]);
    $factory = $stmt->fetch(PDO::FETCH_ASSOC);
    if ($factory) {
        echo json_encode(['success' => true, 'data' => $factory]);
    } else {
        http_response_code(404);
        echo json_encode(['success' => false, 'message' => 'Factory not found.']);
    }
}

/**
 * 신규 공장 추가 (POST)
 * - factory_name 필수 검증
 * - UNIQUE 제약 위반(23000) 시 409 Conflict 반환
 * - 성공 시 201 Created
 */
function addFactory(PDO $pdo): void {
    $factory_name = trim($_POST['factory_name'] ?? '');
    $status       = $_POST['status'] ?? 'Y';
    $remark       = trim($_POST['remark'] ?? '');

    // 공장명 필수값 검증
    if (empty($factory_name)) {
        http_response_code(400);
        echo json_encode(['success' => false, 'message' => 'Factory name is required.']);
        return;
    }
    try {
        $stmt = $pdo->prepare("INSERT INTO info_factory (factory_name, status, remark, reg_date, update_date) VALUES (?, ?, ?, NOW(), NOW())");
        $stmt->execute([$factory_name, $status, $remark]);
        http_response_code(201);
        echo json_encode(['success' => true, 'message' => 'Factory added successfully.']);
    } catch (PDOException $e) {
        // UNIQUE 키 중복 시 409 응답 (같은 이름의 공장이 이미 존재)
        if ($e->getCode() == '23000') {
            http_response_code(409);
            echo json_encode(['success' => false, 'message' => 'Factory name already exists.']);
        } else {
            throw $e;
        }
    }
}

/**
 * 공장 정보 수정 (PUT — _method 오버라이드로 전달됨)
 * - idx, factory_name 필수 검증
 * - update_date 자동 갱신
 * - UNIQUE 제약 위반(23000) 시 409 Conflict 반환
 */
function updateFactory(PDO $pdo): void {
    $id           = $_POST['idx'] ?? 0;
    $factory_name = trim($_POST['factory_name'] ?? '');
    $status       = $_POST['status'] ?? 'Y';
    $remark       = trim($_POST['remark'] ?? '');

    // idx와 공장명 필수값 검증
    if (empty($id) || empty($factory_name)) {
        http_response_code(400);
        echo json_encode(['success' => false, 'message' => 'ID and Factory name are required.']);
        return;
    }
    try {
        $stmt = $pdo->prepare("UPDATE info_factory SET factory_name = ?, status = ?, remark = ?, update_date = NOW() WHERE idx = ?");
        $stmt->execute([$factory_name, $status, $remark, $id]);
        echo json_encode(['success' => true, 'message' => 'Factory updated successfully.']);
    } catch (PDOException $e) {
        // 다른 공장과 이름 중복 시 409 응답
        if ($e->getCode() == '23000') {
            http_response_code(409);
            echo json_encode(['success' => false, 'message' => 'Factory name already exists.']);
        } else {
            throw $e;
        }
    }
}

/**
 * 공장명 중복 여부 확인 (실시간 폼 검증용)
 * - current_idx가 있으면 자기 자신 제외 (수정 시 사용)
 * - 중복이면 success:false, 가용하면 success:true 반환
 */
function checkDuplicateFactory(PDO $pdo): void {
    $factory_name = trim($_GET['factory_name'] ?? '');
    $current_idx  = $_GET['current_idx'] ?? null;

    if (empty($factory_name)) {
        http_response_code(400);
        echo json_encode(['success' => false, 'message' => 'Factory name is required.']);
        return;
    }

    $sql    = "SELECT COUNT(*) FROM info_factory WHERE factory_name = ?";
    $params = [$factory_name];
    // 수정 모드: 자기 자신의 idx는 중복 체크에서 제외
    if ($current_idx) {
        $sql    .= " AND idx != ?";
        $params[] = $current_idx;
    }

    $stmt = $pdo->prepare($sql);
    $stmt->execute($params);
    if ((int)$stmt->fetchColumn() > 0) {
        echo json_encode(['success' => false, 'message' => 'This factory name already exists. Please enter a different name.']);
    } else {
        echo json_encode(['success' => true, 'message' => '']);
    }
}

/**
 * Factory 통계 — 단일 CROSS JOIN 쿼리로 4개 집계 동시 처리
 * - 공장 수(total_factories), 라인 수(total_lines), 총 인원(total_mp), 기계 수(total_machines)
 * - status_filter가 있으면 공장·라인·기계 모두 필터 적용
 * - CROSS JOIN: 각 집계 서브쿼리 결과를 1행으로 합침
 */
function getStatistics(PDO $pdo): void {
    $status_filter = $_GET['status_filter'] ?? '';

    $f_where   = '';  // FROM info_factory WHERE ...
    $join_where = ''; // JOIN info_factory f ... AND f.status = ?
    $params     = [];

    // 상태 필터가 있을 때 각 서브쿼리에 동일한 조건 적용
    if (!empty($status_filter)) {
        $f_where    = ' WHERE status = ?';
        $join_where = ' AND f.status = ?';
        $params[]   = $status_filter;
    }

    try {
        // CROSS JOIN으로 공장/라인/기계 집계를 한 번의 쿼리로 처리
        $sql = "SELECT
                    f_cnt.total_factories,
                    COALESCE(l_cnt.total_lines, 0)    AS total_lines,
                    COALESCE(l_cnt.total_mp, 0)       AS total_mp,
                    COALESCE(m_cnt.total_machines, 0) AS total_machines
                FROM (SELECT COUNT(*) AS total_factories FROM info_factory{$f_where}) f_cnt
                CROSS JOIN (
                    SELECT COUNT(l.idx) AS total_lines, COALESCE(SUM(l.mp), 0) AS total_mp
                    FROM info_line l
                    JOIN info_factory f ON l.factory_idx = f.idx{$join_where}
                ) l_cnt
                CROSS JOIN (
                    SELECT COUNT(m.idx) AS total_machines
                    FROM info_machine m
                    JOIN info_factory f ON m.factory_idx = f.idx{$join_where}
                ) m_cnt";

        // f_where + join_where + join_where 각각에 대한 파라미터 (3번 바인딩 필요)
        $all_params = array_merge($params, $params, $params);
        $stmt = $pdo->prepare($sql);
        $stmt->execute($all_params);
        $result = $stmt->fetch(PDO::FETCH_ASSOC);

        echo json_encode(['success' => true, 'data' => $result]);
    } catch (PDOException $e) {
        http_response_code(500);
        echo json_encode(['success' => false, 'message' => 'Database error: ' . $e->getMessage()]);
    }
}
