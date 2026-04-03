<?php

/**
 * proc/machine.php — 기계(Machine) 관리 REST API
 * GET    : 목록/단건 조회, 중복 체크, 셀렉트용 팩토리/라인/모델 목록
 * POST   : 기계 추가
 * PUT    : 기계 수정 (_method 오버라이드 지원)
 * DELETE : 기계 삭제
 * GET ?export=true : 현재 필터 조건을 유지한 채 Excel(.xlsx) 파일로 내보내기
 *
 * 기계(Machine): OEE 데이터를 수집하는 개별 재봉기/자수기 장비
 * info_machine 테이블 관리, factory/line/machine_model/design_process와 JOIN
 */
require_once(__DIR__ . '/../../../lib/config.php');
require_once(__DIR__ . '/../../../lib/db.php');

// PhpSpreadsheet 관련 use 선언 (Excel 내보내기에서만 실제 로드됨)
use PhpOffice\PhpSpreadsheet\Spreadsheet;
use PhpOffice\PhpSpreadsheet\Writer\Xlsx;
use PhpOffice\PhpSpreadsheet\Style\Fill;
use PhpOffice\PhpSpreadsheet\Style\Alignment;
use PhpOffice\PhpSpreadsheet\Style\Border;

// export=true 파라미터 시 Excel 내보내기 처리 후 즉시 종료
// (PhpSpreadsheet autoload는 export 요청에서만 include하여 불필요한 로드 방지)
$for = $_GET['for'] ?? '';
if (isset($_GET['export']) && $_GET['export'] === 'true') {
    require_once __DIR__ . '/../../../lib/vendor/autoload.php';
    exportMachines($pdo);
    exit;
}

header('Content-Type: application/json');

// HTML Form에서 PUT 전송 불가 → _method 필드로 메서드 오버라이드
$method = $_SERVER['REQUEST_METHOD'];
if ($method === 'POST' && isset($_POST['_method'])) {
    $method = strtoupper($_POST['_method']);
}

// HTTP 메서드 및 'for' 파라미터에 따라 처리 함수 분기
try {
    switch ($method) {
        case 'GET':
            if ($for === 'check-duplicate') checkDuplicateMachine($pdo);       // 기계번호 중복 확인
            elseif ($for === 'factories') getFactoriesForSelect($pdo);          // 공장 셀렉트 목록
            elseif ($for === 'lines') getLinesForSelect($pdo);                  // 라인 셀렉트 목록 (factory_id 필터 가능)
            elseif ($for === 'models') getMachineModelsForSelect($pdo);         // 기계 모델 셀렉트 목록
            elseif (isset($_GET['id'])) getMachine($pdo);                       // 특정 기계 단건 조회
            else getMachines($pdo);                                             // 기계 목록 조회 (페이징)
            break;
        case 'POST':
            addMachine($pdo);
            break;
        case 'PUT':
            updateMachine($pdo);
            break;
        case 'DELETE':
            deleteMachine($pdo);
            break;
        default:
            http_response_code(405);
            echo json_encode(['success' => false, 'message' => 'Method Not Allowed']);
            break;
    }
} catch (Exception $e) {
    http_response_code(500);
    echo json_encode(['success' => false, 'message' => 'Server Error: ' . $e->getMessage()]);
}

/**
 * GET 파라미터에서 목록 조회 조건을 파싱하여 반환
 * - sort/order: 화이트리스트 검증 (JOIN 테이블 별칭 포함, 예: 'm.machine_no', 'f.factory_name')
 * - factory_filter / line_filter / machine_filter / status_filter / type_filter: 복합 필터
 * - search: machine_no, factory_name, line_name, machine_model_name, design_process, mac, ip 대상 LIKE 검색
 *   → 7개 컬럼 LIKE 이므로 $params에 동일한 검색어를 7회 추가
 */
function parse_list_params(): array
{
    // 정렬 컬럼 화이트리스트 (JOIN 테이블 별칭 포함하여 그대로 쿼리에 삽입)
    $sort_column = $_GET['sort'] ?? 'm.machine_no';
    $sort_order = strtoupper($_GET['order'] ?? 'ASC');
    $valid_columns = ['m.idx', 'f.factory_name', 'l.line_name', 'mm.machine_model_name', 'm.machine_no', 'dp.design_process', 'm.mac', 'm.status', 'm.app_ver'];
    if (!in_array($sort_column, $valid_columns)) $sort_column = 'm.machine_no';
    if (!in_array($sort_order, ['ASC', 'DESC'])) $sort_order = 'ASC';

    $params = [];
    $where_clauses = [];

    // 공장 필터: 특정 공장 소속 기계만 조회
    if (!empty($_GET['factory_filter'])) {
        $where_clauses[] = 'm.factory_idx = ?';
        $params[] = $_GET['factory_filter'];
    }
    // 라인 필터: 특정 라인 소속 기계만 조회
    if (!empty($_GET['line_filter'])) {
        $where_clauses[] = 'm.line_idx = ?';
        $params[] = $_GET['line_filter'];
    }
    // 기계 필터: 특정 기계 단건 필터 (기계 목록에서 특정 기계 선택 시)
    if (!empty($_GET['machine_filter'])) {
        $where_clauses[] = 'm.idx = ?';
        $params[] = $_GET['machine_filter'];
    }
    // 사용 여부 필터: Y(사용중) / N(미사용)
    if (!empty($_GET['status_filter'])) {
        $where_clauses[] = 'm.status = ?';
        $params[] = $_GET['status_filter'];
    }
    // 타입 필터: P(패턴재봉기) / E(자수기)
    if (!empty($_GET['type_filter'])) {
        $where_clauses[] = 'm.type = ?';
        $params[] = $_GET['type_filter'];
    }

    // 검색어: 7개 컬럼 LIKE 검색 → $params에 동일 값 7개 추가
    $search_query = trim($_GET['search'] ?? '');
    if (!empty($search_query)) {
        $where_clauses[] = '(m.machine_no LIKE ? OR f.factory_name LIKE ? OR l.line_name LIKE ? OR mm.machine_model_name LIKE ? OR dp.design_process LIKE ? OR m.mac LIKE ? OR m.ip LIKE ?)';
        $search_param = '%' . $search_query . '%';
        $params = array_merge($params, array_fill(0, 7, $search_param));
    }

    $where_sql = count($where_clauses) > 0 ? ' WHERE ' . implode(' AND ', $where_clauses) : '';

    return ['where_sql' => $where_sql, 'params' => $params, 'sort_column' => $sort_column, 'sort_order' => $sort_order];
}

/**
 * 기계 목록 조회 (페이징 + 4개 테이블 LEFT JOIN)
 * - info_machine (m) + info_factory (f) + info_line (l) + info_machine_model (mm) + info_design_process (dp)
 * - limit 기본값 9999: 화면에서 전체 목록을 한 번에 가져오는 경우 대응
 * - COUNT 쿼리에도 동일한 JOIN/WHERE 조건 적용 (LEFT JOIN 컬럼으로 필터링 가능하도록)
 */
function getMachines(PDO $pdo): void
{
    $query_conditions = parse_list_params();
    $page = (int)($_GET['page'] ?? 1);
    $limit = (int)($_GET['limit'] ?? 9999);
    $offset = ($page - 1) * $limit;

    // 전체 레코드 수 카운트 (동일 JOIN/WHERE 조건 적용)
    $count_sql = "SELECT COUNT(*) FROM info_machine AS m
                LEFT JOIN info_factory AS f ON m.factory_idx = f.idx
                LEFT JOIN info_line AS l ON m.line_idx = l.idx
                LEFT JOIN info_machine_model AS mm ON m.machine_model_idx = mm.idx
                LEFT JOIN info_design_process AS dp ON m.design_process_idx = dp.idx
                " . $query_conditions['where_sql'];
    $total_stmt = $pdo->prepare($count_sql);
    $total_stmt->execute($query_conditions['params']);
    $total_records = (int)$total_stmt->fetchColumn();

    // 기계 목록 조회: idx/이름 쌍을 함께 반환하여 프론트에서 수정 모달 기본값 세팅에 활용
    $sql = "SELECT m.idx, m.factory_idx, f.factory_name, m.line_idx, l.line_name, m.machine_model_idx, mm.machine_model_name, m.machine_no, m.design_process_idx, dp.design_process, m.mac, m.ip, m.type, m.status, m.target, m.app_ver
          FROM info_machine AS m
          LEFT JOIN info_factory AS f ON m.factory_idx = f.idx
          LEFT JOIN info_line AS l ON m.line_idx = l.idx
          LEFT JOIN info_machine_model AS mm ON m.machine_model_idx = mm.idx
          LEFT JOIN info_design_process AS dp ON m.design_process_idx = dp.idx
          {$query_conditions['where_sql']}
          ORDER BY {$query_conditions['sort_column']} {$query_conditions['sort_order']}
          LIMIT {$limit} OFFSET {$offset}";
    $stmt = $pdo->prepare($sql);
    $stmt->execute($query_conditions['params']);
    $data = $stmt->fetchAll(PDO::FETCH_ASSOC);

    echo json_encode([
        'success' => true,
        'data' => $data,
        'pagination' => ['total_records' => $total_records, 'current_page' => $page, 'total_pages' => ceil($total_records / $limit)]
    ]);
}

/**
 * 기계 단건 조회 (수정 폼 데이터 로드용)
 * - SELECT * 사용: mac, ip, pos_x, pos_y 등 수정 폼에 필요한 모든 컬럼 반환
 */
function getMachine(PDO $pdo): void
{
    $id = $_GET['id'] ?? 0;
    $stmt = $pdo->prepare("SELECT * FROM info_machine WHERE idx = ?");
    $stmt->execute([$id]);
    $data = $stmt->fetch(PDO::FETCH_ASSOC);
    if ($data) {
        echo json_encode(['success' => true, 'data' => $data]);
    } else {
        http_response_code(404);
        echo json_encode(['success' => false, 'message' => 'Machine not found.']);
    }
}

/**
 * 기계 추가 (POST)
 * - factory_idx, line_idx, machine_model_idx, machine_no 필수
 * - mac, ip: 장비 네트워크 식별자 (선택 입력)
 * - type: P(패턴재봉기) / E(자수기), 기본값 'P'
 * - target: 1일 목표 생산량
 * - pos_x, pos_y: 라인 배치도에서의 좌표 (UI 레이아웃용)
 * - app_ver: 장비에 설치된 앱 버전 (OTA 업데이트 현황 파악용)
 */
function addMachine(PDO $pdo): void
{
    try {
        $data = $_POST;

        // 필수 입력값 검증
        if (empty($data['factory_idx']) || empty($data['line_idx']) || empty($data['machine_model_idx']) || empty($data['machine_no'])) {
            http_response_code(400);
            echo json_encode(['success' => false, 'message' => 'Factory, Line, Model, and Machine No are required.']);
            return;
        }

        $sql = "INSERT INTO info_machine (factory_idx, line_idx, machine_model_idx, machine_no, mac, ip, type, status, remark, target, pos_x, pos_y, app_ver, reg_date, update_date)
            VALUES (?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, NOW(), NOW())";
        $stmt = $pdo->prepare($sql);

        $stmt->execute([
            $data['factory_idx'],
            $data['line_idx'],
            $data['machine_model_idx'],
            $data['machine_no'],
            $data['mac'] ?? '',
            $data['ip'] ?? '',
            $data['type'] ?? 'P',
            $data['status'] ?? 'Y',
            $data['remark'] ?? '',
            (int)($data['target'] ?? 0),
            (int)($data['pos_x'] ?? 0),
            (int)($data['pos_y'] ?? 0),
            $data['app_ver'] ?? ''
        ]);
        http_response_code(201);
        echo json_encode(['success' => true, 'message' => 'Machine added successfully.']);
    } catch (PDOException $e) {
        http_response_code(500);
        echo json_encode(['success' => false, 'message' => 'Database error: ' . $e->getMessage()]);
    }
}

/**
 * 기계 수정 (PUT)
 * - mac, ip, app_ver는 장비 자체에서 업데이트하는 경우가 많아 수정 폼에서 제외
 * - pos_x, pos_y: 라인 배치도 드래그&드롭 이동 시에도 이 API를 사용
 * - rowCount() > 0 검사: 값 변경이 없는 경우(동일값 재저장)와 not found를 구분
 */
function updateMachine(PDO $pdo): void
{
    try {
        $data = $_POST;
        $id   = $data['idx'] ?? 0;

        // 필수 입력값 검증
        if (empty($id) || empty($data['factory_idx']) || empty($data['line_idx']) || empty($data['machine_model_idx']) || empty($data['machine_no'])) {
            http_response_code(400);
            echo json_encode(['success' => false, 'message' => 'ID, Factory, Line, Model, and Machine No are required.']);
            return;
        }

        $stmt = $pdo->prepare("UPDATE info_machine SET factory_idx=?, line_idx=?, machine_model_idx=?, machine_no=?, type=?, status=?, remark=?, target=?, pos_x=?, pos_y=?, update_date=NOW() WHERE idx = ?");
        $stmt->execute([
            $data['factory_idx'],
            $data['line_idx'],
            $data['machine_model_idx'],
            $data['machine_no'],
            $data['type'] ?? 'P',
            $data['status'] ?? 'Y',
            $data['remark'] ?? '',
            (int)($data['target'] ?? 0),
            (int)($data['pos_x'] ?? 0),
            (int)($data['pos_y'] ?? 0),
            $id
        ]);

        if ($stmt->rowCount() > 0) {
            echo json_encode(['success' => true, 'message' => 'Machine updated successfully.']);
        } else {
            http_response_code(404);
            echo json_encode(['success' => false, 'message' => 'Machine not found or no changes made.']);
        }
    } catch (PDOException $e) {
        http_response_code(500);
        echo json_encode(['success' => false, 'message' => 'Database error: ' . $e->getMessage()]);
    }
}

/**
 * 기계 삭제 (DELETE)
 * - 관련 data_pcount, data_andon 등 실적 데이터는 별도 처리 필요 (CASCADE 미적용)
 */
function deleteMachine(PDO $pdo): void
{
    $id = $_GET['id'] ?? 0;
    if (empty($id)) {
        http_response_code(400);
        echo json_encode(['success' => false, 'message' => 'ID is required.']);
        return;
    }
    $stmt = $pdo->prepare("DELETE FROM info_machine WHERE idx = ?");
    $stmt->execute([$id]);
    if ($stmt->rowCount() > 0) {
        echo json_encode(['success' => true, 'message' => 'Machine deleted successfully.']);
    } else {
        http_response_code(404);
        echo json_encode(['success' => false, 'message' => 'Machine not found.']);
    }
}

/**
 * 공장 목록 반환 (셀렉트박스용)
 * - 기계 추가/수정 모달에서 공장 드롭다운 초기화에 사용
 * - status='Y' 필터: 비활성 공장은 선택 불가
 */
function getFactoriesForSelect(PDO $pdo): void
{
    $stmt = $pdo->prepare("SELECT idx, factory_name FROM info_factory WHERE status = 'Y' ORDER BY factory_name");
    $stmt->execute();
    echo json_encode(['success' => true, 'data' => $stmt->fetchAll(PDO::FETCH_ASSOC)]);
}

/**
 * 라인 목록 반환 (셀렉트박스용)
 * - factory_id 파라미터가 있으면 해당 공장 소속 라인만 반환 (연쇄 드롭다운)
 * - factory_id가 없으면 전체 활성 라인 반환
 */
function getLinesForSelect(PDO $pdo): void
{
    $factory_id = $_GET['factory_id'] ?? 0;
    $sql = "SELECT idx, line_name FROM info_line WHERE status = 'Y'";
    $params = [];
    if (!empty($factory_id)) {
        // 공장 선택 시: 해당 공장 소속 라인만 필터링
        $sql .= " AND factory_idx = ?";
        $params[] = $factory_id;
    }
    $sql .= " ORDER BY line_name";
    $stmt = $pdo->prepare($sql);
    $stmt->execute($params);
    echo json_encode(['success' => true, 'data' => $stmt->fetchAll(PDO::FETCH_ASSOC)]);
}

/**
 * 기계 모델 목록 반환 (셀렉트박스용)
 * - status='Y' 필터: 비활성 모델은 선택 불가
 */
function getMachineModelsForSelect(PDO $pdo): void
{
    $stmt = $pdo->prepare("SELECT idx, machine_model_name FROM info_machine_model WHERE status = 'Y' ORDER BY machine_model_name");
    $stmt->execute();
    echo json_encode(['success' => true, 'data' => $stmt->fetchAll(PDO::FETCH_ASSOC)]);
}

/**
 * 기계번호 중복 여부 확인 (실시간 폼 검증용)
 * - current_idx가 있으면 자기 자신 제외 (수정 모드에서 동일 번호 재사용 허용)
 * - machine_no는 전체 시스템에서 고유해야 함 (공장/라인 구분 없이)
 */
function checkDuplicateMachine(PDO $pdo): void
{
    $machine_no  = trim($_GET['machine_no'] ?? '');
    $current_idx = $_GET['current_idx'] ?? null;

    if (empty($machine_no)) {
        http_response_code(400);
        echo json_encode(['success' => false, 'message' => 'Machine number is required.']);
        return;
    }

    $sql    = "SELECT COUNT(*) FROM info_machine WHERE machine_no = ?";
    $params = [$machine_no];
    // 수정 모드: 자기 자신(current_idx)은 중복 검사에서 제외
    if ($current_idx) {
        $sql .= " AND idx != ?";
        $params[] = $current_idx;
    }

    $stmt = $pdo->prepare($sql);
    $stmt->execute($params);
    echo json_encode(
        (int)$stmt->fetchColumn() > 0
            ? ['success' => false, 'message' => 'This machine number already exists. Please enter a different number.']
            : ['success' => true,  'message' => '']
    );
}


/**
 * 기계 목록 Excel 내보내기 (PhpSpreadsheet)
 * - parse_list_params()로 현재 필터/정렬 조건을 그대로 유지하여 내보내기
 *   (화면에서 보이는 데이터와 동일한 결과를 엑셀로 제공)
 * - 헤더 스타일: 굵게 + 회색 배경 + 가운데 정렬 + 테두리
 * - 데이터 행: 테두리만 적용
 * - 각 컬럼 너비 자동 조정 (AutoSize)
 * - status: DB 'Y'/'N' 값을 'Used'/'Unused' 텍스트로 변환하여 가독성 향상
 * - 파일명: machine_list_YYYY-MM-DD.xlsx 형식
 */
function exportMachines(PDO $pdo): void
{
    $query_conditions = parse_list_params();
    // 내보내기 전용 쿼리: pos_x, pos_y는 내보내기 불필요하여 제외
    $sql = "SELECT m.idx, f.factory_name, l.line_name, mm.machine_model_name, m.machine_no, dp.design_process, m.mac, m.ip, m.type, m.status, m.target, m.reg_date, m.update_date
          FROM info_machine AS m
          LEFT JOIN info_factory AS f ON m.factory_idx = f.idx
          LEFT JOIN info_line AS l ON m.line_idx = l.idx
          LEFT JOIN info_machine_model AS mm ON m.machine_model_idx = mm.idx
          LEFT JOIN info_design_process AS dp ON m.design_process_idx = dp.idx
          {$query_conditions['where_sql']}
          ORDER BY {$query_conditions['sort_column']} {$query_conditions['sort_order']}";
    $stmt = $pdo->prepare($sql);
    $stmt->execute($query_conditions['params']);
    $data = $stmt->fetchAll(PDO::FETCH_ASSOC);

    // Spreadsheet 생성 및 헤더 설정
    $spreadsheet = new Spreadsheet();
    $sheet = $spreadsheet->getActiveSheet();
    $sheet->setTitle('Machine List');
    $headers = ['NO', 'ID', 'Factory', 'Line', 'Model', 'Machine No', 'Design Process', 'MAC', 'IP', 'Type', 'Status', 'Target', 'Reg. Date', 'Update Date'];
    $sheet->fromArray($headers, NULL, 'A1');

    // 헤더 스타일 적용 (굵게 + 회색 배경 + 가운데 정렬 + 테두리)
    $headerStyle = ['font' => ['bold' => true], 'fill' => ['fillType' => Fill::FILL_SOLID, 'startColor' => ['rgb' => 'E9E9E9']], 'alignment' => ['horizontal' => Alignment::HORIZONTAL_CENTER], 'borders' => ['allBorders' => ['borderStyle' => Border::BORDER_THIN]]];
    $sheet->getStyle('A1:' . $sheet->getHighestColumn() . '1')->applyFromArray($headerStyle);

    // 데이터 행 삽입: NO는 1부터 시작하는 순번, status는 Y/N → Used/Unused 변환
    $rowNum = 2;
    foreach ($data as $index => $row) {
        $sheet->fromArray([
            $index + 1,
            $row['idx'],
            $row['factory_name'],
            $row['line_name'],
            $row['machine_model_name'],
            $row['machine_no'],
            $row['design_process'],
            $row['mac'],
            $row['ip'],
            $row['type'],
            $row['status'] == 'Y' ? 'Used' : 'Unused',
            $row['target'],
            $row['reg_date'],
            $row['update_date']
        ], NULL, 'A' . $rowNum++);
    }

    // 데이터 영역 테두리 적용 (데이터가 있는 경우에만)
    if ($rowNum > 2) {
        $dataStyle = ['borders' => ['allBorders' => ['borderStyle' => Border::BORDER_THIN]]];
        $sheet->getStyle('A2:' . $sheet->getHighestColumn() . ($rowNum - 1))->applyFromArray($dataStyle);
    }

    // 모든 컬럼 너비 자동 조정
    foreach ($sheet->getColumnIterator() as $column) {
        $sheet->getColumnDimension($column->getColumnIndex())->setAutoSize(true);
    }

    // 다운로드 헤더 설정 및 파일 출력
    $filename = "machine_list_" . date('Y-m-d') . ".xlsx";
    header('Content-Type: application/vnd.openxmlformats-officedocument.spreadsheetml.sheet');
    header('Content-Disposition: attachment;filename="' . $filename . '"');
    header('Cache-Control: max-age=0');
    $writer = new Xlsx($spreadsheet);
    $writer->save('php://output');
}
