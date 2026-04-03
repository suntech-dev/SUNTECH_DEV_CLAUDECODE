<?php

/**
 * proc/design_process.php — 디자인 공정(Design Process) 관리 REST API
 * GET    : 목록/단건 조회, 공장·라인 셀렉트 옵션, 중복 체크
 * POST   : 공정 추가 (SOP 파일 업로드 포함)
 * PUT    : 공정 수정 (SOP 파일 교체 포함)
 * DELETE : 공정 삭제 (SOP 파일 함께 삭제)
 *
 * SOP(Standard Operating Procedure) 파일:
 * - JPG/JPEG/PNG 형식, 최대 10MB
 * - upload/sop/ 디렉토리에 저장
 * - 파일명: 원본명_YYYYMMDDHHIISS.확장자
 */
ini_set('upload_max_filesize', '10M');
ini_set('post_max_size', '20M');
ini_set('max_execution_time', '300');
ini_set('memory_limit', '128M');

/**
 * info_design_process Table API (RESTful)
 * Handles C.R.U.D operations, file upload and Excel export functionality.
 */

require_once(__DIR__ . '/../../../lib/config.php');
require_once(__DIR__ . '/../../../lib/db.php');

// SOP 파일 업로드 경로 상수 정의
define('UPLOAD_SOP_DIR', __DIR__ . '/../../../upload/sop/');

/**
 * 이미지 파일 업로드 처리 함수
 * JPG, JPEG, PNG 파일만 허용하며, 업로드된 파일을 upload/sop 폴더에 저장합니다.
 *
 * 검증 순서:
 * 1) 파일 없음 → 빈 파일로 간주, 성공 반환 (선택 필드이므로)
 * 2) 업로드 에러 코드 확인 (서버 설정, 부분 업로드 등)
 * 3) 파일 크기 제한 (10MB)
 * 4) MIME 타입 검사 (finfo 사용, 확장자 위조 방지)
 * 5) 파일 확장자 검사 (이중 체크)
 * 6) 업로드 디렉토리 존재 여부 및 쓰기 권한 확인
 * 7) 파일명: 원본명_타임스탬프.확장자 형식으로 저장
 *
 * @param array $file $_FILES['file_upload'] 배열
 * @return array ['success' => bool, 'filename' => string|null, 'message' => string]
 */
function handleFileUpload(array $file): array
{
    if (empty($file) || $file['error'] === UPLOAD_ERR_NO_FILE) {
        return ['success' => true, 'filename' => null, 'message' => 'No file uploaded'];
    }

    // PHP 표준 업로드 에러 코드별 한국어 메시지 매핑
    if ($file['error'] !== UPLOAD_ERR_OK) {
        $errorMessages = [
            UPLOAD_ERR_INI_SIZE   => '파일이 서버 설정 최대 크기(' . ini_get('upload_max_filesize') . ')를 초과했습니다.',
            UPLOAD_ERR_FORM_SIZE  => '파일이 폼에서 지정한 최대 크기를 초과했습니다.',
            UPLOAD_ERR_PARTIAL    => '파일이 부분적으로만 업로드되었습니다.',
            UPLOAD_ERR_NO_TMP_DIR => '임시 폴더가 없습니다.',
            UPLOAD_ERR_CANT_WRITE => '디스크에 파일을 쓸 수 없습니다.',
            UPLOAD_ERR_EXTENSION  => 'PHP 확장에 의해 파일 업로드가 중단되었습니다.',
        ];
        $message = $errorMessages[$file['error']] ?? '알 수 없는 업로드 오류가 발생했습니다. (Error code: ' . $file['error'] . ')';
        return ['success' => false, 'filename' => null, 'message' => $message];
    }

    // 10MB 크기 제한 확인
    if ($file['size'] > 10 * 1024 * 1024) {
        return ['success' => false, 'filename' => null, 'message' => '파일 크기가 10MB를 초과합니다. (현재: ' . round($file['size'] / 1024 / 1024, 2) . 'MB)'];
    }

    // MIME 타입 검사 (실제 파일 내용 기반, 확장자 위조 방지)
    $finfo    = finfo_open(FILEINFO_MIME_TYPE);
    $mimeType = finfo_file($finfo, $file['tmp_name']);
    finfo_close($finfo);

    $allowedTypes      = ['image/jpeg', 'image/jpg', 'image/png'];
    $allowedExtensions = ['jpg', 'jpeg', 'png'];
    $fileExtension     = strtolower(pathinfo($file['name'], PATHINFO_EXTENSION));

    if (!in_array($mimeType, $allowedTypes)) {
        return ['success' => false, 'filename' => null, 'message' => 'Only JPG, JPEG, and PNG files can be uploaded. (Current: ' . $mimeType . ')'];
    }
    if (!in_array($fileExtension, $allowedExtensions)) {
        return ['success' => false, 'filename' => null, 'message' => 'Invalid file extension. (Current: ' . $fileExtension . ')'];
    }

    // 업로드 디렉토리 생성 및 쓰기 권한 확인
    if (!file_exists(UPLOAD_SOP_DIR) && !mkdir(UPLOAD_SOP_DIR, 0755, true)) {
        return ['success' => false, 'filename' => null, 'message' => 'Unable to create upload folder.'];
    }
    if (!is_writable(UPLOAD_SOP_DIR)) {
        return ['success' => false, 'filename' => null, 'message' => 'You do not have write permission for the upload folder.'];
    }

    // 타임스탬프 접미어로 파일명 충돌 방지
    $originalName = pathinfo($file['name'], PATHINFO_FILENAME);
    $newFileName  = $originalName . '_' . date('YmdHis') . '.' . $fileExtension;
    $uploadPath   = UPLOAD_SOP_DIR . $newFileName;

    if (move_uploaded_file($file['tmp_name'], $uploadPath)) {
        return ['success' => true, 'filename' => $newFileName, 'message' => 'File upload successful'];
    }
    return ['success' => false, 'filename' => null, 'message' => 'The file upload failed. Please check the server logs.'];
}

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
                // 공장+라인 조합 내 공정명 중복 여부 확인
                checkDuplicateDesignProcess($pdo);
            } elseif (isset($_GET['id'])) {
                // 특정 idx 공정 단건 조회
                getDesignProcess($pdo);
            } else {
                // 공정 목록 조회 (공장/라인 셀렉트 옵션 포함)
                getDesignProcesses($pdo);
            }
            break;
        case 'POST':
            addDesignProcess($pdo);
            break;
        case 'PUT':
            updateDesignProcess($pdo);
            break;
        case 'DELETE':
            deleteDesignProcess($pdo);
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
 * - factory_filter, line_filter: 공장/라인별 필터
 * - search: design_process, model_name, factory_name, line_name 대상 LIKE 검색
 */
function parse_design_process_list_params(): array
{
    $sort_column = $_GET['sort'] ?? 'dp.design_process';
    $sort_order  = strtoupper($_GET['order'] ?? 'ASC');

    $valid_columns = ['dp.idx', 'dp.design_process', 'dp.model_name', 'dp.std_mc_needed', 'dp.fname', 'dp.status', 'dp.remark', 'f.factory_name', 'l.line_name'];
    if (!in_array($sort_column, $valid_columns)) $sort_column = 'dp.design_process';
    if (!in_array($sort_order, ['ASC', 'DESC']))  $sort_order  = 'ASC';

    $status_filter  = $_GET['status_filter']  ?? '';
    $factory_filter = $_GET['factory_filter'] ?? '';
    $line_filter    = $_GET['line_filter']    ?? '';
    $search_query   = trim($_GET['search']    ?? '');

    $where_conditions = [];
    $params           = [];

    if (!empty($status_filter)) {
        $where_conditions[] = 'dp.status = ?';
        $params[] = $status_filter;
    }
    if (!empty($factory_filter)) {
        $where_conditions[] = 'dp.factory_idx = ?';
        $params[] = $factory_filter;
    }
    if (!empty($line_filter)) {
        $where_conditions[] = 'dp.line_idx = ?';
        $params[] = $line_filter;
    }

    // 공정명, 모델명, 공장명, 라인명 중 하나라도 일치하면 검색 결과에 포함
    if (!empty($search_query)) {
        $where_conditions[] = '(dp.design_process LIKE ? OR dp.model_name LIKE ? OR f.factory_name LIKE ? OR l.line_name LIKE ?)';
        $p = '%' . $search_query . '%';
        $params = array_merge($params, [$p, $p, $p, $p]);
    }

    $where_sql = $where_conditions ? ' WHERE ' . implode(' AND ', $where_conditions) : '';

    return ['where_sql' => $where_sql, 'params' => $params, 'sort_column' => $sort_column, 'sort_order' => $sort_order];
}

/**
 * 공정 목록 조회 또는 셀렉트 옵션 반환
 *
 * 'for' 파라미터에 따른 분기:
 * - 'factories': 활성 공장 목록 반환 (드롭다운용)
 * - 'lines': 활성 라인 목록 반환 (factory_id 필터 지원)
 * - 없음: 공정 목록 조회 (페이징, 필터, 정렬, 공장/라인명 포함)
 */
function getDesignProcesses(PDO $pdo): void
{
    if (isset($_GET['for'])) {
        $for = $_GET['for'];
        // 공장 셀렉트 옵션 반환 (활성 공장만)
        if ($for === 'factories') {
            $stmt = $pdo->prepare("SELECT idx, factory_name FROM info_factory WHERE status = 'Y' ORDER BY factory_name ASC");
            $stmt->execute();
            echo json_encode(['success' => true, 'data' => $stmt->fetchAll(PDO::FETCH_ASSOC)]);
            return;
        }
        // 라인 셀렉트 옵션 반환 (factory_id가 있으면 해당 공장의 라인만)
        if ($for === 'lines') {
            $factory_id = $_GET['factory_id'] ?? '';
            if ($factory_id) {
                $stmt = $pdo->prepare("SELECT idx, line_name FROM info_line WHERE factory_idx = ? AND status = 'Y' ORDER BY line_name ASC");
                $stmt->execute([$factory_id]);
            } else {
                $stmt = $pdo->prepare("SELECT idx, line_name FROM info_line WHERE status = 'Y' ORDER BY line_name ASC");
                $stmt->execute();
            }
            echo json_encode(['success' => true, 'data' => $stmt->fetchAll(PDO::FETCH_ASSOC)]);
            return;
        }
    }

    // 공정 목록 조회: FROM 절을 공유하여 COUNT와 SELECT를 일관성 있게 처리
    $cond       = parse_design_process_list_params();
    $where_sql  = $cond['where_sql'];
    $params     = $cond['params'];
    $sort_col   = $cond['sort_column'];
    $sort_order = $cond['sort_order'];

    $page   = (int)($_GET['page']  ?? 1);
    $limit  = (int)($_GET['limit'] ?? 10);
    $offset = ($page - 1) * $limit;

    // FROM/JOIN/WHERE를 공유하여 COUNT와 SELECT 일관성 유지
    $base_from = "FROM info_design_process dp
        LEFT JOIN info_factory f ON dp.factory_idx = f.idx
        LEFT JOIN info_line    l ON dp.line_idx    = l.idx
        {$where_sql}";

    $total_stmt = $pdo->prepare("SELECT COUNT(*) {$base_from}");
    $total_stmt->execute($params);
    $total_records = (int)$total_stmt->fetchColumn();

    // 공정 정보 + 공장명 + 라인명 함께 조회
    $sql = "SELECT dp.idx, dp.design_process, dp.model_name, dp.std_mc_needed, dp.fname,
                   dp.status, dp.remark, dp.factory_idx, dp.line_idx,
                   f.factory_name, l.line_name
            {$base_from}
            ORDER BY {$sort_col} {$sort_order}
            LIMIT {$limit} OFFSET {$offset}";
    $stmt = $pdo->prepare($sql);
    $stmt->execute($params);

    echo json_encode([
        'success' => true,
        'data'    => $stmt->fetchAll(PDO::FETCH_ASSOC),
        'pagination' => [
            'total_records' => $total_records,
            'current_page'  => $page,
            'total_pages'   => ceil($total_records / $limit),
        ],
    ]);
}

/**
 * 공정 단건 조회 (수정 폼 데이터 로드용)
 * - 공장명, 라인명도 JOIN하여 함께 반환
 */
function getDesignProcess(PDO $pdo): void
{
    $id = $_GET['id'] ?? 0;
    if (empty($id)) {
        http_response_code(400);
        echo json_encode(['success' => false, 'message' => 'ID is required.']);
        return;
    }
    $stmt = $pdo->prepare(
        "SELECT dp.idx, dp.design_process, dp.model_name, dp.std_mc_needed, dp.fname,
                dp.status, dp.remark, dp.factory_idx, dp.line_idx,
                f.factory_name, l.line_name
         FROM info_design_process dp
         LEFT JOIN info_factory f ON dp.factory_idx = f.idx
         LEFT JOIN info_line    l ON dp.line_idx    = l.idx
         WHERE dp.idx = ?"
    );
    $stmt->execute([$id]);
    $row = $stmt->fetch(PDO::FETCH_ASSOC);
    if ($row) {
        echo json_encode(['success' => true, 'data' => $row]);
    } else {
        http_response_code(404);
        echo json_encode(['success' => false, 'message' => 'Design Process not found.']);
    }
}

/**
 * 공정명 중복 여부 확인 (같은 공장+라인 조합 내에서만 검사)
 * - design_process + factory_idx + line_idx 조합으로 유일성 확인
 * - current_idx가 있으면 자기 자신 제외 (수정 모드)
 */
function checkDuplicateDesignProcess(PDO $pdo): void
{
    $design_process = trim($_GET['design_process'] ?? '');
    $factory_idx    = $_GET['factory_idx']    ?? null;
    $line_idx       = $_GET['line_idx']       ?? null;
    $current_idx    = $_GET['current_idx']    ?? null;

    if (empty($design_process)) {
        http_response_code(400);
        echo json_encode(['success' => false, 'message' => 'Design Process name is required.']);
        return;
    }
    if (empty($factory_idx) || empty($line_idx)) {
        http_response_code(400);
        echo json_encode(['success' => false, 'message' => 'Factory and Line are required.']);
        return;
    }

    // 같은 공장+라인 내에서 동일 공정명 존재 여부 확인
    $sql    = "SELECT COUNT(*) FROM info_design_process WHERE design_process = ? AND factory_idx = ? AND line_idx = ?";
    $params = [$design_process, $factory_idx, $line_idx];
    if ($current_idx) {
        $sql    .= " AND idx != ?";
        $params[] = $current_idx;
    }

    $stmt = $pdo->prepare($sql);
    $stmt->execute($params);
    if ((int)$stmt->fetchColumn() > 0) {
        echo json_encode(['success' => false, 'message' => 'This design process name already exists in the selected Factory/Line. Please enter a different name.']);
    } else {
        echo json_encode(['success' => true, 'message' => 'Design process name is available.']);
    }
}

/**
 * POST/PUT 요청의 공정 입력값 검증
 * - $_POST와 $_FILES 모두 비어있으면 요청 데이터 없음 (서버 사이즈 제한 초과 가능성)
 * - design_process, factory_idx, line_idx 필수 검증
 * - std_mc_needed: 0 이상의 정수
 *
 * @return array|null 검증 통과 시 데이터 배열, 실패 시 null (이미 에러 응답 전송됨)
 */
function validateDesignProcessInput(): ?array
{
    // 요청 데이터가 완전히 비어있으면 서버 크기 제한 초과 가능성 안내
    if (empty($_POST) && empty($_FILES)) {
        http_response_code(400);
        echo json_encode([
            'success' => false,
            'message' => 'No data received. The request may be too large. Please check if the file size exceeds the server limit (post_max_size: ' . ini_get('post_max_size') . ', upload_max_filesize: ' . ini_get('upload_max_filesize') . ').',
        ]);
        return null;
    }

    $design_process = trim($_POST['design_process'] ?? '');
    $factory_idx    = (int)($_POST['factory_idx']   ?? 0);
    $line_idx       = (int)($_POST['line_idx']      ?? 0);
    $std_mc_needed  = (int)($_POST['std_mc_needed'] ?? 1);

    if (empty($design_process)) {
        http_response_code(400);
        echo json_encode(['success' => false, 'message' => 'Design Process name is required.']);
        return null;
    }
    if ($factory_idx <= 0 || $line_idx <= 0) {
        http_response_code(400);
        echo json_encode(['success' => false, 'message' => 'Factory and Line are required.']);
        return null;
    }
    if ($std_mc_needed < 0) {
        http_response_code(400);
        echo json_encode(['success' => false, 'message' => 'Standard MC Needed must be a non-negative integer.']);
        return null;
    }

    return [
        'design_process' => $design_process,
        'factory_idx'    => $factory_idx,
        'line_idx'       => $line_idx,
        'model_name'     => trim($_POST['model_name'] ?? ''),
        'std_mc_needed'  => $std_mc_needed,
        'fname'          => trim($_POST['fname'] ?? ''),
        'status'         => $_POST['status'] ?? 'Y',
        'remark'         => trim($_POST['remark'] ?? ''),
    ];
}

/**
 * 파일 업로드 처리 (handleFileUpload 래퍼)
 * - 파일이 없으면 그냥 성공 반환 (파일은 선택 필드)
 * - 업로드 성공 시 $data['fname']과 $data['new_filename']을 갱신
 * - 실패 시 에러 응답 전송 후 false 반환 → 호출자가 즉시 종료
 *
 * @param array $data 공정 데이터 배열 (참조 전달로 fname 갱신)
 * @return bool 업로드 성공(또는 파일 없음) 시 true, 실패 시 false
 */
function processFileUpload(array &$data): bool
{
    if (!isset($_FILES['file_upload']) || $_FILES['file_upload']['error'] === UPLOAD_ERR_NO_FILE) {
        return true;
    }
    $result = handleFileUpload($_FILES['file_upload']);
    if (!$result['success']) {
        http_response_code(400);
        echo json_encode(['success' => false, 'message' => '파일 업로드 실패: ' . $result['message']]);
        return false;
    }
    if ($result['filename']) {
        // 새 파일명을 데이터 배열에 반영
        $data['new_filename'] = $result['filename'];
        $data['fname']        = $result['filename'];
    }
    return true;
}

/**
 * 신규 공정 추가 (POST + 파일 업로드)
 * - DB INSERT 실패 시 업로드한 파일 롤백(@unlink로 파일 삭제)
 * - UNIQUE(design_process, factory_idx, line_idx) 위반 시 409 반환
 */
function addDesignProcess(PDO $pdo): void
{
    $data = validateDesignProcessInput();
    if ($data === null) return;

    // 파일 업로드 처리 (실패 시 함수 종료)
    if (!processFileUpload($data)) return;

    try {
        $stmt = $pdo->prepare(
            "INSERT INTO info_design_process
                (design_process, factory_idx, line_idx, model_name, std_mc_needed, fname, status, remark, reg_date, update_date)
             VALUES (?, ?, ?, ?, ?, ?, ?, ?, NOW(), NOW())"
        );
        $stmt->execute([
            $data['design_process'],
            $data['factory_idx'],
            $data['line_idx'],
            $data['model_name'],
            $data['std_mc_needed'],
            $data['fname'],
            $data['status'],
            $data['remark'],
        ]);
        http_response_code(201);
        $message = 'Design Process added successfully.';
        if (!empty($data['new_filename'])) $message .= ' File upload complete: ' . $data['new_filename'];
        echo json_encode(['success' => true, 'message' => $message]);
    } catch (PDOException $e) {
        // DB 실패 시 업로드된 파일 롤백
        if (!empty($data['new_filename'])) {
            @unlink(UPLOAD_SOP_DIR . $data['new_filename']);
        }
        if ($e->getCode() == '23000') {
            http_response_code(409);
            echo json_encode(['success' => false, 'message' => 'Design Process name already exists in the selected Factory/Line.']);
        } else {
            throw $e;
        }
    }
}

/**
 * 공정 수정 (PUT + 파일 교체)
 * - 새 파일이 업로드되면 기존 파일 삭제 (@unlink)
 * - DB 실패 시 새로 업로드된 파일 롤백
 */
function updateDesignProcess(PDO $pdo): void
{
    $data = validateDesignProcessInput();
    if ($data === null) return;

    $id = $_POST['idx'] ?? 0;
    if (empty($id)) {
        http_response_code(400);
        echo json_encode(['success' => false, 'message' => 'ID and Design Process name are required.']);
        return;
    }

    // 기존 파일명 조회 (새 파일로 교체 시 삭제 대상 파악용)
    $existing_stmt = $pdo->prepare("SELECT fname FROM info_design_process WHERE idx = ?");
    $existing_stmt->execute([$id]);
    $existing_filename = ($existing_stmt->fetch(PDO::FETCH_ASSOC))['fname'] ?? '';

    if (!processFileUpload($data)) return;

    // 새 파일이 업로드되었고 기존 파일이 있으면 기존 파일 삭제 예정
    $old_file_to_delete = null;
    if (!empty($data['new_filename']) && !empty($existing_filename) && $existing_filename !== $data['new_filename']) {
        $old_file_to_delete = $existing_filename;
    }

    try {
        $stmt = $pdo->prepare(
            "UPDATE info_design_process
             SET design_process = ?, factory_idx = ?, line_idx = ?, model_name = ?,
                 std_mc_needed = ?, fname = ?, status = ?, remark = ?, update_date = NOW()
             WHERE idx = ?"
        );
        $stmt->execute([
            $data['design_process'],
            $data['factory_idx'],
            $data['line_idx'],
            $data['model_name'],
            $data['std_mc_needed'],
            $data['fname'],
            $data['status'],
            $data['remark'],
            $id,
        ]);

        // DB 업데이트 성공 후 기존 파일 삭제 (에러 무시)
        if ($old_file_to_delete) {
            @unlink(UPLOAD_SOP_DIR . $old_file_to_delete);
        }

        $message = 'Design Process updated successfully.';
        if (!empty($data['new_filename'])) $message .= ' File upload complete: ' . $data['new_filename'];
        echo json_encode(['success' => true, 'message' => $message]);
    } catch (PDOException $e) {
        // DB 실패 시 새로 업로드된 파일 롤백
        if (!empty($data['new_filename'])) {
            @unlink(UPLOAD_SOP_DIR . $data['new_filename']);
        }
        if ($e->getCode() == '23000') {
            http_response_code(409);
            echo json_encode(['success' => false, 'message' => 'Design Process name already exists in the selected Factory/Line.']);
        } else {
            throw $e;
        }
    }
}

/**
 * 공정 삭제 (DELETE)
 * - 삭제 전 fname 조회하여 파일도 함께 삭제
 * - DB 삭제 성공 후 파일 삭제 (파일 삭제 실패는 무시)
 */
function deleteDesignProcess(PDO $pdo): void
{
    $id = $_GET['id'] ?? 0;
    if (empty($id)) {
        http_response_code(400);
        echo json_encode(['success' => false, 'message' => 'ID is required.']);
        return;
    }

    // 삭제 전 파일명 조회
    $file_stmt = $pdo->prepare("SELECT fname FROM info_design_process WHERE idx = ?");
    $file_stmt->execute([$id]);
    $row = $file_stmt->fetch(PDO::FETCH_ASSOC);

    $stmt = $pdo->prepare("DELETE FROM info_design_process WHERE idx = ?");
    $stmt->execute([$id]);

    if ($stmt->rowCount() > 0) {
        // DB 삭제 성공 후 SOP 파일도 삭제 (@로 에러 무시)
        if ($row && !empty($row['fname'])) {
            @unlink(UPLOAD_SOP_DIR . $row['fname']);
        }
        echo json_encode(['success' => true, 'message' => 'Design Process deleted successfully.']);
    } else {
        http_response_code(404);
        echo json_encode(['success' => false, 'message' => 'Design Process not found or already deleted.']);
    }
}
