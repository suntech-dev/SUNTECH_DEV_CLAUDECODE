<?php
// Prevent HTML error output for JSON API
ini_set('display_errors', 0);
error_reporting(E_ALL);

// Error handler to catch errors and output as JSON
set_error_handler(function($severity, $message, $file, $line) {
    if (!(error_reporting() & $severity)) return;
    http_response_code(500);
    echo json_encode(['success' => false, 'message' => 'PHP Error: ' . $message]);
    exit;
});

/**
 * info_machine table related API (RESTful)
 * Handles C.R.U.D functions, Excel export, and related data queries.
 */

// Load common libraries and configuration files
require_once(__DIR__ . '/../../../lib/config.php');
require_once(__DIR__ . '/../../../lib/db.php');

// PhpSpreadsheet 클래스 사용
use PhpOffice\PhpSpreadsheet\Spreadsheet;
use PhpOffice\PhpSpreadsheet\Writer\Xlsx;
use PhpOffice\PhpSpreadsheet\Style\Fill;
use PhpOffice\PhpSpreadsheet\Style\Alignment;
use PhpOffice\PhpSpreadsheet\Style\Border;
use PhpOffice\PhpSpreadsheet\Cell\DataType;

// Request routing
$for = $_GET['for'] ?? '';
if (isset($_GET['export']) && $_GET['export'] === 'true') {
  require_once __DIR__ . '/../../lib/vendor/autoload.php';
  exportMachines($pdo);
  exit;
}

header('Content-Type: application/json');
$method = $_SERVER['REQUEST_METHOD'];
if ($method === 'POST' && isset($_POST['_method'])) {
  $method = strtoupper($_POST['_method']);
}

try {
  switch ($method) {
    case 'GET':
      if ($for === 'check-duplicate') checkDuplicateMachine($pdo);
      elseif ($for === 'factories') getFactoriesForSelect($pdo);
      elseif ($for === 'lines') getLinesForSelect($pdo);
      elseif ($for === 'models') getMachineModelsForSelect($pdo);
      elseif (isset($_GET['id'])) getMachine($pdo);
      else getMachines($pdo);
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

function parse_list_params() {
  $sort_column = $_GET['sort'] ?? 'm.machine_no';
  $sort_order = strtoupper($_GET['order'] ?? 'ASC');
  $valid_columns = ['m.idx', 'f.factory_name', 'l.line_name', 'mm.machine_model_name', 'm.machine_no', 'dp.design_process', 'm.mac', 'm.status', 'm.app_ver'];
  if (!in_array($sort_column, $valid_columns)) $sort_column = 'm.machine_no';
  if (!in_array($sort_order, ['ASC', 'DESC'])) $sort_order = 'ASC';

  $params = [];
  $where_clauses = [];
  if (!empty($_GET['factory_filter'])) {
    $where_clauses[] = 'm.factory_idx = ?';
    $params[] = $_GET['factory_filter'];
  }
  if (!empty($_GET['line_filter'])) {
    $where_clauses[] = 'm.line_idx = ?';
    $params[] = $_GET['line_filter'];
  }
  if (!empty($_GET['machine_filter'])) {
    $where_clauses[] = 'm.idx = ?';
    $params[] = $_GET['machine_filter'];
  }
  if (!empty($_GET['status_filter'])) {
    $where_clauses[] = 'm.status = ?';
    $params[] = $_GET['status_filter'];
  }
  if (!empty($_GET['type_filter'])) {
    $where_clauses[] = 'm.type = ?';
    $params[] = $_GET['type_filter'];
  }

  // 검색 파라미터 추가
  $search_query = trim($_GET['search'] ?? '');
  if (!empty($search_query)) {
    $where_clauses[] = '(m.machine_no LIKE ? OR f.factory_name LIKE ? OR l.line_name LIKE ? OR mm.machine_model_name LIKE ? OR dp.design_process LIKE ? OR m.mac LIKE ? OR m.ip LIKE ?)';
    $search_param = '%' . $search_query . '%';
    $params[] = $search_param;
    $params[] = $search_param;
    $params[] = $search_param;
    $params[] = $search_param;
    $params[] = $search_param;
    $params[] = $search_param;
    $params[] = $search_param;
  }

  $where_sql = count($where_clauses) > 0 ? ' WHERE ' . implode(' AND ', $where_clauses) : '';

  return ['where_sql' => $where_sql, 'params' => $params, 'sort_column' => $sort_column, 'sort_order' => $sort_order];
}

function getMachines(PDO $pdo) {
  $query_conditions = parse_list_params();
  $page = (int)($_GET['page'] ?? 1);
  $limit = (int)($_GET['limit'] ?? 9999);  // 드롭다운에서 모든 machine 표시를 위해 기본값 증가
  $offset = ($page - 1) * $limit;

  $count_sql = "SELECT COUNT(*) FROM info_machine AS m
                LEFT JOIN info_factory AS f ON m.factory_idx = f.idx
                LEFT JOIN info_line AS l ON m.line_idx = l.idx
                LEFT JOIN info_machine_model AS mm ON m.machine_model_idx = mm.idx
                LEFT JOIN info_design_process AS dp ON m.design_process_idx = dp.idx
                " . $query_conditions['where_sql'];
  $total_stmt = $pdo->prepare($count_sql);
  $total_stmt->execute($query_conditions['params']);
  $total_records = (int)$total_stmt->fetchColumn();

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

function getMachine(PDO $pdo) {
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

function addMachine(PDO $pdo) {
  try {
    $data = $_POST;
    
    // Validate required fields
    if (empty($data['factory_idx']) || empty($data['line_idx']) || empty($data['machine_model_idx']) || empty($data['machine_no'])) {
      http_response_code(400);
      echo json_encode(['success' => false, 'message' => 'Factory, Line, Model, and Machine No are required.']);
      return;
    }
    
    // Execute SQL
    $sql = "INSERT INTO info_machine (factory_idx, line_idx, machine_model_idx, machine_no, mac, ip, type, status, remark, target, pos_x, pos_y, app_ver, reg_date, update_date)
            VALUES (?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, NOW(), NOW())";
    $stmt = $pdo->prepare($sql);
    
    $result = $stmt->execute([
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
    
    if ($result) {
      http_response_code(201);
      echo json_encode(['success' => true, 'message' => 'Machine added successfully.']);
    } else {
      http_response_code(500);
      echo json_encode(['success' => false, 'message' => 'Failed to add machine.']);
    }
  } catch (PDOException $e) {
    http_response_code(500);
    echo json_encode(['success' => false, 'message' => 'Database error: ' . $e->getMessage()]);
  } catch (Exception $e) {
    http_response_code(500);
    echo json_encode(['success' => false, 'message' => 'Server error: ' . $e->getMessage()]);
  }
}

function updateMachine(PDO $pdo) {
  try {
    $data = $_POST;
    $id = $data['idx'] ?? 0;
    
    // Validate required fields
    if (empty($id) || empty($data['factory_idx']) || empty($data['line_idx']) || empty($data['machine_model_idx']) || empty($data['machine_no'])) {
      http_response_code(400);
      echo json_encode(['success' => false, 'message' => 'ID, Factory, Line, Model, and Machine No are required.']);
      return;
    }
    
    // Execute SQL
    $sql = "UPDATE info_machine SET factory_idx=?, line_idx=?, machine_model_idx=?, machine_no=?, type=?, status=?, remark=?, target=?, pos_x=?, pos_y=?, update_date=NOW() WHERE idx = ?";
    $stmt = $pdo->prepare($sql);
    
    $result = $stmt->execute([
      $data['factory_idx'], 
      $data['line_idx'], 
      $data['machine_model_idx'], 
      $data['machine_no'], 
      // $data['mac'] ?? '', 
      // $data['ip'] ?? '', 
      $data['type'] ?? 'P', 
      $data['status'] ?? 'Y', 
      $data['remark'] ?? '', 
      (int)($data['target'] ?? 0), 
      (int)($data['pos_x'] ?? 0), 
      (int)($data['pos_y'] ?? 0), 
      // $data['app_ver'] ?? '', 
      $id
    ]);
    
    if ($result && $stmt->rowCount() > 0) {
      echo json_encode(['success' => true, 'message' => 'Machine updated successfully.']);
    } else {
      http_response_code(404);
      echo json_encode(['success' => false, 'message' => 'Machine not found or no changes made.']);
    }
  } catch (PDOException $e) {
    http_response_code(500);
    echo json_encode(['success' => false, 'message' => 'Database error: ' . $e->getMessage()]);
  } catch (Exception $e) {
    http_response_code(500);
    echo json_encode(['success' => false, 'message' => 'Server error: ' . $e->getMessage()]);
  }
}

function deleteMachine(PDO $pdo) {
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

function getFactoriesForSelect(PDO $pdo) {
  $stmt = $pdo->prepare("SELECT idx, factory_name FROM info_factory WHERE status = 'Y' ORDER BY factory_name");
  $stmt->execute();
  echo json_encode(['success' => true, 'data' => $stmt->fetchAll(PDO::FETCH_ASSOC)]);
}

function getLinesForSelect(PDO $pdo) {
  $factory_id = $_GET['factory_id'] ?? 0;
  $sql = "SELECT idx, line_name FROM info_line WHERE status = 'Y'";
  $params = [];
  if (!empty($factory_id)) {
    $sql .= " AND factory_idx = ?";
    $params[] = $factory_id;
  }
  $sql .= " ORDER BY line_name";
  $stmt = $pdo->prepare($sql);
  $stmt->execute($params);
  echo json_encode(['success' => true, 'data' => $stmt->fetchAll(PDO::FETCH_ASSOC)]);
}

function getMachineModelsForSelect(PDO $pdo) {
  $stmt = $pdo->prepare("SELECT idx, machine_model_name FROM info_machine_model WHERE status = 'Y' ORDER BY machine_model_name");
  $stmt->execute();
  echo json_encode(['success' => true, 'data' => $stmt->fetchAll(PDO::FETCH_ASSOC)]);
}

/**
 * Check duplicate machine number
 * @param PDO $pdo PDO object
 */
function checkDuplicateMachine(PDO $pdo) {
    $machine_no = trim($_GET['machine_no'] ?? '');
    $current_idx = $_GET['current_idx'] ?? null; // Exclude current record in edit mode
    
    if (empty($machine_no)) {
        http_response_code(400);
        echo json_encode(['success' => false, 'message' => 'Machine number is required.']);
        return;
    }
    
    // Duplicate check query
    $sql = "SELECT COUNT(*) FROM info_machine WHERE machine_no = ?";
    $params = [$machine_no];
    
    // Exclude current record in edit mode
    if ($current_idx) {
        $sql .= " AND idx != ?";
        $params[] = $current_idx;
    }
    
    $stmt = $pdo->prepare($sql);
    $stmt->execute($params);
    $count = (int)$stmt->fetchColumn();
    
    if ($count > 0) {
        echo json_encode(['success' => false, 'message' => 'This machine number already exists. Please enter a different number.']);
    } else {
        echo json_encode(['success' => true, 'message' => '']);
    }
}


function exportMachines(PDO $pdo) {
  $query_conditions = parse_list_params();
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

  $spreadsheet = new Spreadsheet();
  $sheet = $spreadsheet->getActiveSheet();
  $sheet->setTitle('Machine List');
  $headers = ['NO', 'ID', 'Factory', 'Line', 'Model', 'Machine No', 'Design Process', 'MAC', 'IP', 'Type', 'Status', 'Target', 'Reg. Date', 'Update Date'];
  $sheet->fromArray($headers, NULL, 'A1');

  $headerStyle = ['font' => ['bold' => true], 'fill' => ['fillType' => Fill::FILL_SOLID, 'startColor' => ['rgb' => 'E9E9E9']], 'alignment' => ['horizontal' => Alignment::HORIZONTAL_CENTER], 'borders' => ['allBorders' => ['borderStyle' => Border::BORDER_THIN]]];
  $sheet->getStyle('A1:' . $sheet->getHighestColumn() . '1')->applyFromArray($headerStyle);

  $rowNum = 2;
  foreach ($data as $index => $row) {
    $sheet->fromArray([
      $index + 1, $row['idx'], $row['factory_name'], $row['line_name'], $row['machine_model_name'], $row['machine_no'], $row['design_process'],
      $row['mac'], $row['ip'], $row['type'], $row['status'] == 'Y' ? 'Used' : 'Unused', $row['target'], $row['reg_date'], $row['update_date']
    ], NULL, 'A' . $rowNum++);
  }

  if ($rowNum > 2) {
    $dataStyle = ['borders' => ['allBorders' => ['borderStyle' => Border::BORDER_THIN]]];
    $sheet->getStyle('A2:' . $sheet->getHighestColumn() . ($rowNum - 1))->applyFromArray($dataStyle);
  }

  foreach ($sheet->getColumnIterator() as $column) {
    $sheet->getColumnDimension($column->getColumnIndex())->setAutoSize(true);
  }

  $filename = "machine_list_" . date('Y-m-d') . ".xlsx";
  header('Content-Type: application/vnd.openxmlformats-officedocument.spreadsheetml.sheet');
  header('Content-Disposition: attachment;filename="' . $filename . '"');
  header('Cache-Control: max-age=0');
  $writer = new Xlsx($spreadsheet);
  $writer->save('php://output');
}
?>