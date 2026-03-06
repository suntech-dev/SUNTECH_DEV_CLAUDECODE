<?php
ini_set('display_errors', 1);
error_reporting(E_ALL);

// 파일 업로드를 위한 PHP 설정 조정
ini_set('upload_max_filesize', '10M');
ini_set('post_max_size', '20M');
ini_set('max_execution_time', '300');
ini_set('memory_limit', '128M');

/**
 * info_design_process Table API (RESTful)
 * Handles C.R.U.D operations, file upload and Excel export functionality.
 */

// 공통 라이브러리 및 설정 파일 로드
require_once(__DIR__ . '/../../../lib/config.php');
require_once(__DIR__ . '/../../../lib/db.php');



/**
 * 이미지 파일 업로드 처리 함수
 * JPG, JPEG, PNG 파일만 허용하며, 업로드된 파일을 new/upload/sop 폴더에 저장합니다.
 *
 * @param array $file $_FILES 배열의 파일 정보
 * @return array 업로드 결과 (success: boolean, filename: string|null, message: string)
 */
function handleFileUpload($file) {
  // 디버깅 로그 추가
  error_log("=== File Upload Debug Info ===");
  error_log("File info: " . print_r($file, true));
  error_log("POST size: " . strlen(serialize($_POST)) . " bytes");
  error_log("FILES size: " . (isset($_FILES) ? strlen(serialize($_FILES)) : 0) . " bytes");
  error_log("php.ini settings - upload_max_filesize: " . ini_get('upload_max_filesize'));
  error_log("php.ini settings - post_max_size: " . ini_get('post_max_size'));

  // 파일이 업로드되지 않은 경우
  if (empty($file) || $file['error'] === UPLOAD_ERR_NO_FILE) {
    error_log("No file uploaded");
    return ['success' => true, 'filename' => null, 'message' => 'No file uploaded'];
  }

  // 업로드 오류 확인
  if ($file['error'] !== UPLOAD_ERR_OK) {
    $errorMessages = [
      UPLOAD_ERR_INI_SIZE => '파일이 서버 설정 최대 크기(' . ini_get('upload_max_filesize') . ')를 초과했습니다.',
      UPLOAD_ERR_FORM_SIZE => '파일이 폼에서 지정한 최대 크기를 초과했습니다.',
      UPLOAD_ERR_PARTIAL => '파일이 부분적으로만 업로드되었습니다.',
      UPLOAD_ERR_NO_TMP_DIR => '임시 폴더가 없습니다.',
      UPLOAD_ERR_CANT_WRITE => '디스크에 파일을 쓸 수 없습니다.',
      UPLOAD_ERR_EXTENSION => 'PHP 확장에 의해 파일 업로드가 중단되었습니다.'
    ];
    $message = $errorMessages[$file['error']] ?? '알 수 없는 업로드 오류가 발생했습니다. (Error code: ' . $file['error'] . ')';
    error_log("Upload error: " . $message);
    return ['success' => false, 'filename' => null, 'message' => $message];
  }

  // 파일 크기 검증 (10MB = 10 * 1024 * 1024 bytes)
  $maxSize = 10 * 1024 * 1024;
  error_log("File size: " . $file['size'] . " bytes, Max allowed: " . $maxSize . " bytes");
  if ($file['size'] > $maxSize) {
    return ['success' => false, 'filename' => null, 'message' => '파일 크기가 10MB를 초과합니다. (현재: ' . round($file['size'] / 1024 / 1024, 2) . 'MB)'];
  }

  // 파일 MIME 타입 검증
  $allowedTypes = ['image/jpeg', 'image/jpg', 'image/png'];
  $finfo = finfo_open(FILEINFO_MIME_TYPE);
  $mimeType = finfo_file($finfo, $file['tmp_name']);
  finfo_close($finfo);

  error_log("MIME Type: " . $mimeType);
  if (!in_array($mimeType, $allowedTypes)) {
    return ['success' => false, 'filename' => null, 'message' => 'JPG, JPEG, PNG 파일만 업로드 가능합니다. (현재: ' . $mimeType . ')'];
  }

  // 파일 확장자 검증
  $allowedExtensions = ['jpg', 'jpeg', 'png'];
  $fileExtension = strtolower(pathinfo($file['name'], PATHINFO_EXTENSION));
  error_log("File extension: " . $fileExtension);
  if (!in_array($fileExtension, $allowedExtensions)) {
    return ['success' => false, 'filename' => null, 'message' => '잘못된 파일 확장자입니다. (현재: ' . $fileExtension . ')'];
  }

  // 업로드 폴더 경로 설정
  $uploadDir = __DIR__ . '/../../../upload/sop/';
  error_log("Upload directory: " . $uploadDir);

  // 업로드 폴더가 없으면 생성
  if (!file_exists($uploadDir)) {
    error_log("Creating upload directory...");
    if (!mkdir($uploadDir, 0755, true)) {
      error_log("Failed to create upload directory");
      return ['success' => false, 'filename' => null, 'message' => '업로드 폴더를 생성할 수 없습니다.'];
    }
    error_log("Upload directory created successfully");
  }

  // 폴더 쓰기 권한 확인
  if (!is_writable($uploadDir)) {
    error_log("Upload directory is not writable");
    return ['success' => false, 'filename' => null, 'message' => '업로드 폴더에 쓰기 권한이 없습니다.'];
  }

  // 중복 파일명 방지를 위해 타임스탬프 추가
  $originalName = pathinfo($file['name'], PATHINFO_FILENAME);
  $extension = pathinfo($file['name'], PATHINFO_EXTENSION);
  $timestamp = date('YmdHis');
  $newFileName = $originalName . '_' . $timestamp . '.' . $extension;
  $uploadPath = $uploadDir . $newFileName;

  error_log("Final upload path: " . $uploadPath);
  error_log("Temp file path: " . $file['tmp_name']);
  error_log("Temp file exists: " . (file_exists($file['tmp_name']) ? 'yes' : 'no'));

  // 파일을 최종 경로로 이동
  if (move_uploaded_file($file['tmp_name'], $uploadPath)) {
    error_log("File uploaded successfully: " . $newFileName);
    return ['success' => true, 'filename' => $newFileName, 'message' => '파일 업로드 성공'];
  } else {
    error_log("Failed to move uploaded file");
    return ['success' => false, 'filename' => null, 'message' => '파일 업로드에 실패했습니다. 서버 로그를 확인해주세요.'];
  }
}


// API 응답을 JSON 형식으로 설정
header('Content-Type: application/json');

// 디버깅: 요청 정보 로깅
error_log("=== Request Debug Info ===");
error_log("Request Method: " . $_SERVER['REQUEST_METHOD']);
error_log("Content Length: " . ($_SERVER['CONTENT_LENGTH'] ?? 'Not set'));
error_log("Content Type: " . ($_SERVER['CONTENT_TYPE'] ?? 'Not set'));
error_log("POST data count: " . count($_POST));
error_log("FILES data count: " . count($_FILES));
error_log("POST empty: " . (empty($_POST) ? 'yes' : 'no'));
error_log("FILES empty: " . (empty($_FILES) ? 'yes' : 'no'));

// HTTP 요청 메서드를 확인
$method = $_SERVER['REQUEST_METHOD'];

// POST 요청에서 _method 필드가 있는 경우 해당 값으로 메서드를 재정의 (HTML 폼에서 PUT, DELETE 지원)
if ($method === 'POST' && isset($_POST['_method'])) {
    $method = strtoupper($_POST['_method']);
    error_log("Method overridden to: " . $method);
}

try {
    // 요청 메서드에 따라 적절한 함수 호출
    switch ($method) {
        case 'GET':
            if (isset($_GET['for']) && $_GET['for'] === 'check-duplicate') {
                checkDuplicateDesignProcess($pdo);
            } elseif (isset($_GET['id'])) {
                getDesignProcess($pdo);
            } else {
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
function parse_design_process_list_params() {
    // 정렬 파라미터
    $sort_column = $_GET['sort'] ?? 'dp.design_process';
    $sort_order = strtoupper($_GET['order'] ?? 'ASC');

    // SQL Injection 방지를 위해 정렬 가능한 컬럼을 화이트리스트로 관리
    $valid_columns = ['dp.idx', 'dp.design_process', 'dp.model_name', 'dp.std_mc_needed', 'dp.fname', 'dp.status', 'dp.remark', 'f.factory_name', 'l.line_name'];
    if (!in_array($sort_column, $valid_columns)) {
        $sort_column = 'dp.design_process'; // 기본값
    }
    if (!in_array($sort_order, ['ASC', 'DESC'])) {
        $sort_order = 'ASC'; // 기본값
    }

    // 필터링 파라미터
    $status_filter = $_GET['status_filter'] ?? '';
    $factory_filter = $_GET['factory_filter'] ?? '';
    $line_filter = $_GET['line_filter'] ?? '';
    $search_query = trim($_GET['search'] ?? '');

    // WHERE 절 구성
    $where_conditions = [];
    $params = [];

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

    // 검색 파라미터 추가 (design_process, model_name, factory_name, line_name에서 검색)
    if (!empty($search_query)) {
        $where_conditions[] = '(dp.design_process LIKE ? OR dp.model_name LIKE ? OR f.factory_name LIKE ? OR l.line_name LIKE ?)';
        $search_param = '%' . $search_query . '%';
        $params[] = $search_param;
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
 * 모든 design_process 목록을 조회하여 JSON으로 반환합니다.
 * 정렬, 페이지네이션, 필터링 지원.
 * @param PDO $pdo PDO 객체
 */
function getDesignProcesses(PDO $pdo) {
    // Factory/Line 목록 조회인 경우
    if (isset($_GET['for'])) {
        $for = $_GET['for'];
        if ($for === 'factories') {
            $stmt = $pdo->prepare("SELECT idx, factory_name FROM info_factory WHERE status = 'Y' ORDER BY factory_name ASC");
            $stmt->execute();
            echo json_encode(['success' => true, 'data' => $stmt->fetchAll(PDO::FETCH_ASSOC)]);
            return;
        } elseif ($for === 'lines') {
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

    $query_conditions = parse_design_process_list_params();
    $where_sql = $query_conditions['where_sql'];
    $params = $query_conditions['params'];
    $sort_column = $query_conditions['sort_column'];
    $sort_order = $query_conditions['sort_order'];

    // 페이지네이션 파라미터
    $page = (int)($_GET['page'] ?? 1);
    $limit = (int)($_GET['limit'] ?? 10);
    $offset = ($page - 1) * $limit;

    // 전체 레코드 수 계산 (필터링 적용)
    $total_stmt = $pdo->prepare("
        SELECT COUNT(*)
        FROM info_design_process dp
        LEFT JOIN info_factory f ON dp.factory_idx = f.idx
        LEFT JOIN info_line l ON dp.line_idx = l.idx
        {$where_sql}
    ");
    $total_stmt->execute($params);
    $total_records = (int)$total_stmt->fetchColumn();

    // 현재 페이지에 해당하는 데이터 조회 (필터링, 정렬, 페이지네이션 적용)
    $sql = "
        SELECT
            dp.idx,
            dp.design_process,
            dp.model_name,
            dp.std_mc_needed,
            dp.fname,
            dp.status,
            dp.remark,
            dp.factory_idx,
            dp.line_idx,
            f.factory_name,
            l.line_name
        FROM info_design_process dp
        LEFT JOIN info_factory f ON dp.factory_idx = f.idx
        LEFT JOIN info_line l ON dp.line_idx = l.idx
        {$where_sql}
        ORDER BY {$sort_column} {$sort_order}
        LIMIT {$limit} OFFSET {$offset}
    ";
    $stmt = $pdo->prepare($sql);
    $stmt->execute($params);
    $design_processes = $stmt->fetchAll(PDO::FETCH_ASSOC);

    // 최종 응답 데이터 구성
    echo json_encode([
        'success' => true,
        'data' => $design_processes,
        'pagination' => [
            'total_records' => $total_records,
            'current_page' => $page,
            'total_pages' => ceil($total_records / $limit)
        ]
    ]);
}

/**
 * 특정 design_process 정보를 조회하여 JSON으로 반환합니다.
 * @param PDO $pdo PDO 객체
 */
function getDesignProcess(PDO $pdo) {
    $id = $_GET['id'] ?? 0;
    if (empty($id)) {
        http_response_code(400); // Bad Request
        echo json_encode(['success' => false, 'message' => 'ID is required.']);
        return;
    }
    $stmt = $pdo->prepare("
        SELECT
            dp.idx,
            dp.design_process,
            dp.model_name,
            dp.std_mc_needed,
            dp.fname,
            dp.status,
            dp.remark,
            dp.factory_idx,
            dp.line_idx,
            f.factory_name,
            l.line_name
        FROM info_design_process dp
        LEFT JOIN info_factory f ON dp.factory_idx = f.idx
        LEFT JOIN info_line l ON dp.line_idx = l.idx
        WHERE dp.idx = ?
    ");
    $stmt->execute([$id]);
    $design_process = $stmt->fetch(PDO::FETCH_ASSOC);
    if ($design_process) {
        echo json_encode(['success' => true, 'data' => $design_process]);
    } else {
        http_response_code(404); // Not Found
        echo json_encode(['success' => false, 'message' => 'Design Process not found.']);
    }
}

/**
 * 중복 design_process 이름 확인
 * 같은 factory_idx와 line_idx 내에서 design_process 이름 중복을 체크합니다.
 * @param PDO $pdo PDO 객체
 */
function checkDuplicateDesignProcess(PDO $pdo) {
    $design_process = trim($_GET['design_process'] ?? '');
    $factory_idx = $_GET['factory_idx'] ?? null;
    $line_idx = $_GET['line_idx'] ?? null;
    $current_idx = $_GET['current_idx'] ?? null; // 수정 모드에서 현재 레코드 제외

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

    // 중복 확인 쿼리 - 같은 factory_idx와 line_idx 내에서만 확인
    $sql = "SELECT COUNT(*) FROM info_design_process WHERE design_process = ? AND factory_idx = ? AND line_idx = ?";
    $params = [$design_process, $factory_idx, $line_idx];

    // 수정 모드에서 현재 레코드 제외
    if ($current_idx) {
        $sql .= " AND idx != ?";
        $params[] = $current_idx;
    }

    $stmt = $pdo->prepare($sql);
    $stmt->execute($params);
    $count = (int)$stmt->fetchColumn();

    if ($count > 0) {
        echo json_encode(['success' => false, 'message' => 'This design process name already exists in the selected Factory/Line. Please enter a different name.']);
    } else {
        echo json_encode(['success' => true, 'message' => 'Design process name is available.']);
    }
}

/**
 * 신규 design_process를 추가합니다.
 * 파일 업로드 처리도 포함합니다.
 * @param PDO $pdo PDO 객체
 */
function addDesignProcess(PDO $pdo) {
    // POST 데이터가 비어있는지 확인 (post_max_size 초과 시 발생)
    if (empty($_POST) && empty($_FILES)) {
        http_response_code(400);
        echo json_encode([
            'success' => false,
            'message' => 'No data received. The request may be too large. Please check if the file size exceeds the server limit (post_max_size: ' . ini_get('post_max_size') . ', upload_max_filesize: ' . ini_get('upload_max_filesize') . ').'
        ]);
        return;
    }

    $design_process = trim($_POST['design_process'] ?? '');
    $factory_idx = (int)($_POST['factory_idx'] ?? 0);
    $line_idx = (int)($_POST['line_idx'] ?? 0);
    $model_name = trim($_POST['model_name'] ?? '');
    $std_mc_needed = (int)($_POST['std_mc_needed'] ?? 1);
    $fname = trim($_POST['fname'] ?? '');
    $status = $_POST['status'] ?? 'Y';
    $remark = trim($_POST['remark'] ?? '');

    if (empty($design_process)) {
        http_response_code(400); // Bad Request
        echo json_encode(['success' => false, 'message' => 'Design Process name is required.']);
        return;
    }

    if ($factory_idx <= 0 || $line_idx <= 0) {
        http_response_code(400); // Bad Request
        echo json_encode(['success' => false, 'message' => 'Factory and Line are required.']);
        return;
    }

    if ($std_mc_needed < 0) {
        http_response_code(400); // Bad Request
        echo json_encode(['success' => false, 'message' => 'Standard MC Needed must be a non-negative integer.']);
        return;
    }

    // 파일 업로드 처리
    $uploadResult = ['success' => true, 'filename' => null, 'message' => 'No file'];
    if (isset($_FILES['file_upload']) && $_FILES['file_upload']['error'] !== UPLOAD_ERR_NO_FILE) {
        $uploadResult = handleFileUpload($_FILES['file_upload']);
        if (!$uploadResult['success']) {
            http_response_code(400); // Bad Request
            echo json_encode(['success' => false, 'message' => '파일 업로드 실패: ' . $uploadResult['message']]);
            return;
        }
        // 업로드된 파일이 있으면 fname을 업로드된 파일명으로 업데이트
        if ($uploadResult['filename']) {
            $fname = $uploadResult['filename'];
        }
    }

    try {
        $stmt = $pdo->prepare("INSERT INTO info_design_process (design_process, factory_idx, line_idx, model_name, std_mc_needed, fname, status, remark, reg_date, update_date) VALUES (?, ?, ?, ?, ?, ?, ?, ?, NOW(), NOW())");
        $stmt->execute([$design_process, $factory_idx, $line_idx, $model_name, $std_mc_needed, $fname, $status, $remark]);
        http_response_code(201); // Created
        $message = 'Design Process added successfully.';
        if ($uploadResult['filename']) {
            $message .= ' 파일 업로드 완료: ' . $uploadResult['filename'];
        }
        echo json_encode(['success' => true, 'message' => $message]);
    } catch (PDOException $e) {
        // 데이터베이스 에러 발생 시 업로드된 파일 삭제
        if ($uploadResult['filename']) {
            $uploadPath = __DIR__ . '/../../upload/sop/' . $uploadResult['filename'];
            if (file_exists($uploadPath)) {
                unlink($uploadPath);
            }
        }

        if ($e->getCode() == '23000') {
            http_response_code(409); // Conflict
            echo json_encode(['success' => false, 'message' => 'Design Process name already exists in the selected Factory/Line.']);
        } else {
            throw $e; // 상위 핸들러에서 처리하도록 예외를 다시 던짐
        }
    }
}

/**
 * 기존 design_process 정보를 수정합니다.
 * 파일 업로드 처리도 포함합니다.
 * @param PDO $pdo PDO 객체
 */
function updateDesignProcess(PDO $pdo) {
    // POST 데이터가 비어있는지 확인 (post_max_size 초과 시 발생)
    if (empty($_POST) && empty($_FILES)) {
        http_response_code(400);
        echo json_encode([
            'success' => false,
            'message' => 'No data received. The request may be too large. Please check if the file size exceeds the server limit (post_max_size: ' . ini_get('post_max_size') . ', upload_max_filesize: ' . ini_get('upload_max_filesize') . ').'
        ]);
        return;
    }

    // HTML form에서 _method로 PUT을 전송하므로, 데이터는 $_POST에 있습니다.
    $id = $_POST['idx'] ?? 0;
    $design_process = trim($_POST['design_process'] ?? '');
    $factory_idx = (int)($_POST['factory_idx'] ?? 0);
    $line_idx = (int)($_POST['line_idx'] ?? 0);
    $model_name = trim($_POST['model_name'] ?? '');
    $std_mc_needed = (int)($_POST['std_mc_needed'] ?? 1);
    $fname = trim($_POST['fname'] ?? '');
    $status = $_POST['status'] ?? 'Y';
    $remark = trim($_POST['remark'] ?? '');

    if (empty($id) || empty($design_process)) {
        http_response_code(400); // Bad Request
        echo json_encode(['success' => false, 'message' => 'ID and Design Process name are required.']);
        return;
    }

    if ($factory_idx <= 0 || $line_idx <= 0) {
        http_response_code(400); // Bad Request
        echo json_encode(['success' => false, 'message' => 'Factory and Line are required.']);
        return;
    }

    if ($std_mc_needed < 0) {
        http_response_code(400); // Bad Request
        echo json_encode(['success' => false, 'message' => 'Standard MC Needed must be a non-negative integer.']);
        return;
    }

    // 기존 파일 정보 조회 (새 파일 업로드 시 기존 파일 삭제를 위해)
    $existing_stmt = $pdo->prepare("SELECT fname FROM info_design_process WHERE idx = ?");
    $existing_stmt->execute([$id]);
    $existing_data = $existing_stmt->fetch(PDO::FETCH_ASSOC);
    $existing_filename = $existing_data['fname'] ?? '';

    // 파일 업로드 처리
    $uploadResult = ['success' => true, 'filename' => null, 'message' => 'No file'];
    $oldFileToDelete = null;

    if (isset($_FILES['file_upload']) && $_FILES['file_upload']['error'] !== UPLOAD_ERR_NO_FILE) {
        $uploadResult = handleFileUpload($_FILES['file_upload']);
        if (!$uploadResult['success']) {
            http_response_code(400); // Bad Request
            echo json_encode(['success' => false, 'message' => '파일 업로드 실패: ' . $uploadResult['message']]);
            return;
        }
        // 업로드된 파일이 있으면 fname을 업로드된 파일명으로 업데이트
        if ($uploadResult['filename']) {
            $fname = $uploadResult['filename'];
            // 기존 파일이 있고 새 파일과 다르면 나중에 삭제하기 위해 저장
            if (!empty($existing_filename) && $existing_filename !== $uploadResult['filename']) {
                $oldFileToDelete = $existing_filename;
            }
        }
    }

    try {
        $stmt = $pdo->prepare("UPDATE info_design_process SET design_process = ?, factory_idx = ?, line_idx = ?, model_name = ?, std_mc_needed = ?, fname = ?, status = ?, remark = ?, update_date = NOW() WHERE idx = ?");
        $stmt->execute([$design_process, $factory_idx, $line_idx, $model_name, $std_mc_needed, $fname, $status, $remark, $id]);

        // 데이터베이스 업데이트 성공 시 기존 파일 삭제
        if ($oldFileToDelete) {
            $oldFilePath = __DIR__ . '/../../upload/sop/' . $oldFileToDelete;
            if (file_exists($oldFilePath)) {
                unlink($oldFilePath);
            }
        }

        $message = 'Design Process updated successfully.';
        if ($uploadResult['filename']) {
            $message .= ' 파일 업로드 완료: ' . $uploadResult['filename'];
        }
        echo json_encode(['success' => true, 'message' => $message]);
    } catch (PDOException $e) {
        // 데이터베이스 에러 발생 시 새로 업로드된 파일 삭제
        if ($uploadResult['filename']) {
            $newFilePath = __DIR__ . '/../../upload/sop/' . $uploadResult['filename'];
            if (file_exists($newFilePath)) {
                unlink($newFilePath);
            }
        }

        if ($e->getCode() == '23000') {
            http_response_code(409); // Conflict
            echo json_encode(['success' => false, 'message' => 'Design Process name already exists in the selected Factory/Line.']);
        } else {
            throw $e;
        }
    }
}

/**
 * design process를 삭제합니다.
 * 연관된 파일도 함께 삭제합니다.
 * @param PDO $pdo PDO 객체
 */
function deleteDesignProcess(PDO $pdo) {
    $id = $_GET['id'] ?? 0;

    if (empty($id)) {
        http_response_code(400); // Bad Request
        echo json_encode(['success' => false, 'message' => 'ID is required.']);
        return;
    }

    // 삭제 전에 파일 정보 조회
    $file_stmt = $pdo->prepare("SELECT fname FROM info_design_process WHERE idx = ?");
    $file_stmt->execute([$id]);
    $design_process = $file_stmt->fetch(PDO::FETCH_ASSOC);

    $stmt = $pdo->prepare("DELETE FROM info_design_process WHERE idx = ?");
    $stmt->execute([$id]);

    if ($stmt->rowCount() > 0) {
        // 데이터베이스에서 삭제가 성공했으면 연관된 파일도 삭제
        if ($design_process && !empty($design_process['fname'])) {
            $filePath = __DIR__ . '/../../upload/sop/' . $design_process['fname'];
            if (file_exists($filePath)) {
                unlink($filePath);
            }
        }
        echo json_encode(['success' => true, 'message' => 'Design Process deleted successfully.']);
    } else {
        http_response_code(404); // Not Found
        echo json_encode(['success' => false, 'message' => 'Design Process not found or already deleted.']);
    }
}

