<?php
/**
 * OEE Data Log Excel Export API
 *
 * Features:
 * - Excel export with all columns from data_oee table
 * - Filtering support (Factory → Line → Machine)
 * - Date range filtering
 * - Shift filtering
 * - PhpSpreadsheet-based Excel generation
 */

// Prevent HTML error output for Excel export
ini_set('display_errors', 0);
error_reporting(E_ALL);

// Error handler to catch errors
set_error_handler(function($severity, $message, $file, $line) {
    if (!(error_reporting() & $severity)) return;
    http_response_code(500);
    echo json_encode(['success' => false, 'message' => 'PHP Error: ' . $message]);
    exit;
});


// Load common libraries and configuration files
require_once(__DIR__ . '/../../../lib/config.php');
require_once(__DIR__ . '/../../../lib/db.php');

// PhpSpreadsheet classes
require_once __DIR__ . '/../../../lib/vendor/autoload.php';

use PhpOffice\PhpSpreadsheet\Spreadsheet;
use PhpOffice\PhpSpreadsheet\Writer\Xlsx;
use PhpOffice\PhpSpreadsheet\Style\Fill;
use PhpOffice\PhpSpreadsheet\Style\Alignment;
use PhpOffice\PhpSpreadsheet\Style\Border;
use PhpOffice\PhpSpreadsheet\Cell\DataType;

/**
 * Parse filter parameters from GET request
 */
function parseFilterParams() {
  $params = [];
  $where_clauses = [];

  if (!empty($_GET['factory_filter'])) {
    $where_clauses[] = 'do.factory_idx = ?';
    $params[] = $_GET['factory_filter'];
  }

  if (!empty($_GET['line_filter'])) {
    $where_clauses[] = 'do.line_idx = ?';
    $params[] = $_GET['line_filter'];
  }

  if (!empty($_GET['machine_filter'])) {
    $where_clauses[] = 'do.machine_idx = ?';
    $params[] = $_GET['machine_filter'];
  }

  if (!empty($_GET['shift_filter'])) {
    $where_clauses[] = 'do.shift_idx = ?';
    $params[] = $_GET['shift_filter'];
  }

  if (!empty($_GET['start_date'])) {
    $where_clauses[] = 'do.work_date >= ?';
    $params[] = $_GET['start_date'];
  }

  if (!empty($_GET['end_date'])) {
    $where_clauses[] = 'do.work_date <= ?';
    $params[] = $_GET['end_date'];
  }

  // Default date range if not specified (today)
  if (empty($_GET['start_date']) && empty($_GET['end_date'])) {
    $where_clauses[] = 'do.work_date = CURDATE()';
  }

  $where_sql = count($where_clauses) > 0 ? ' WHERE ' . implode(' AND ', $where_clauses) : '';

  return ['where_sql' => $where_sql, 'params' => $params];
}

/**
 * Export OEE data log to Excel
 */
function exportOeeDataLog(PDO $pdo) {
  try {
    $query_conditions = parseFilterParams();

    // Query all columns from data_oee table
    $sql = "
      SELECT
        do.idx,
        do.work_date,
        do.time_update,
        do.shift_idx,
        do.factory_idx,
        f.factory_name,
        do.line_idx,
        l.line_name,
        do.mac,
        do.machine_idx,
        do.machine_no,
        do.process_name,
        do.planned_work_time,
        do.runtime,
        do.productive_runtime,
        do.downtime,
        do.availabilty_rate,
        do.target_line_per_day,
        do.target_line_per_hour,
        do.target_mc_per_day,
        do.target_mc_per_hour,
        do.cycletime,
        do.pair_info,
        do.pair_count,
        do.theoritical_output,
        do.actual_output,
        do.productivity_rate,
        do.defective,
        do.actual_a_grade,
        do.quality_rate,
        do.oee,
        do.reg_date,
        do.update_date,
        do.work_hour
      FROM data_oee do
      LEFT JOIN info_factory f ON do.factory_idx = f.idx
      LEFT JOIN info_line l ON do.line_idx = l.idx
      {$query_conditions['where_sql']}
      ORDER BY do.work_date DESC, do.update_date DESC, do.idx DESC
      LIMIT 10000
    ";

    $stmt = $pdo->prepare($sql);
    $stmt->execute($query_conditions['params']);
    $data = $stmt->fetchAll(PDO::FETCH_ASSOC);

    // Create Excel spreadsheet
    $spreadsheet = new Spreadsheet();
    $sheet = $spreadsheet->getActiveSheet();
    $sheet->setTitle('OEE Data Log');

    // Define all headers based on data_oee table columns
    $headers = [
      'idx',
      'work_date',
      'time_update',
      'shift_idx',
      'factory_idx',
      'factory_name',
      'line_idx',
      'line_name',
      'mac',
      'machine_idx',
      'machine_no',
      'process_name',
      'planned_work_time',
      'runtime',
      'productive_runtime',
      'downtime',
      'availabilty_rate',
      'target_line_per_day',
      'target_line_per_hour',
      'target_mc_per_day',
      'target_mc_per_hour',
      'cycletime',
      'pair_info',
      'pair_count',
      'theoritical_output',
      'actual_output',
      'productivity_rate',
      'defective',
      'actual_a_grade',
      'quality_rate',
      'oee',
      'reg_date',
      'update_date',
      'work_hour'
    ];

    // Write headers
    $sheet->fromArray($headers, NULL, 'A1');

    // Apply header styling
    $headerStyle = [
      'font' => ['bold' => true, 'color' => ['rgb' => 'FFFFFF']],
      'fill' => [
        'fillType' => Fill::FILL_SOLID,
        'startColor' => ['rgb' => '0070F2']  // SAP Fiori Blue
      ],
      'alignment' => [
        'horizontal' => Alignment::HORIZONTAL_CENTER,
        'vertical' => Alignment::VERTICAL_CENTER
      ],
      'borders' => [
        'allBorders' => [
          'borderStyle' => Border::BORDER_THIN,
          'color' => ['rgb' => 'CCCCCC']
        ]
      ]
    ];
    $sheet->getStyle('A1:' . $sheet->getHighestColumn() . '1')->applyFromArray($headerStyle);

    // Set header row height
    $sheet->getRowDimension(1)->setRowHeight(25);

    // Write data rows
    $rowNum = 2;
    foreach ($data as $row) {
      // Write row data (all columns in order)
      $sheet->fromArray([
        $row['idx'] ?? '',
        $row['work_date'] ?? '',
        $row['time_update'] ?? '',
        $row['shift_idx'] ?? '',
        $row['factory_idx'] ?? '',
        $row['factory_name'] ?? '',
        $row['line_idx'] ?? '',
        $row['line_name'] ?? '',
        $row['mac'] ?? '',
        $row['machine_idx'] ?? '',
        $row['machine_no'] ?? '',
        // $row['design_no'] ?? '',
        $row['process_name'] ?? '',
        $row['planned_work_time'] ?? '',
        $row['runtime'] ?? '',
        $row['productive_runtime'] ?? '',
        $row['downtime'] ?? '',
        $row['availabilty_rate'] ?? '',
        $row['target_line_per_day'] ?? '',
        $row['target_line_per_hour'] ?? '',
        $row['target_mc_per_day'] ?? '',
        $row['target_mc_per_hour'] ?? '',
        $row['cycletime'] ?? '',
        $row['pair_info'] ?? '',
        $row['pair_count'] ?? '',
        $row['theoritical_output'] ?? '',
        $row['actual_output'] ?? '',
        $row['productivity_rate'] ?? '',
        $row['defective'] ?? '',
        $row['actual_a_grade'] ?? '',
        $row['quality_rate'] ?? '',
        $row['oee'] ?? '',
        $row['reg_date'] ?? '',
        $row['update_date'] ?? '',
        $row['work_hour'] ?? ''
      ], NULL, 'A' . $rowNum);

      // Apply alternating row colors
      if ($rowNum % 2 == 0) {
        $sheet->getStyle('A' . $rowNum . ':' . $sheet->getHighestColumn() . $rowNum)
          ->getFill()
          ->setFillType(Fill::FILL_SOLID)
          ->getStartColor()->setRGB('F9F9F9');
      }

      $rowNum++;
    }

    // Apply borders to data rows
    if ($rowNum > 2) {
      $dataStyle = [
        'borders' => [
          'allBorders' => [
            'borderStyle' => Border::BORDER_THIN,
            'color' => ['rgb' => 'CCCCCC']
          ]
        ],
        'alignment' => [
          'vertical' => Alignment::VERTICAL_CENTER
        ]
      ];
      $sheet->getStyle('A2:' . $sheet->getHighestColumn() . ($rowNum - 1))->applyFromArray($dataStyle);
    }

    // Auto-size columns
    foreach ($sheet->getColumnIterator() as $column) {
      $sheet->getColumnDimension($column->getColumnIndex())->setAutoSize(true);
    }

    // Generate filename with timestamp and filters
    $filename_parts = ['OEE_Data_Log'];
    if (!empty($_GET['start_date'])) {
      $filename_parts[] = $_GET['start_date'];
    }
    if (!empty($_GET['end_date'])) {
      $filename_parts[] = $_GET['end_date'];
    }
    $filename = implode('_', $filename_parts) . '_' . date('YmdHis') . '.xlsx';

    // Set headers for download
    header('Content-Type: application/vnd.openxmlformats-officedocument.spreadsheetml.sheet');
    header('Content-Disposition: attachment;filename="' . $filename . '"');
    header('Cache-Control: max-age=0');

    // Write to output
    $writer = new Xlsx($spreadsheet);
    $writer->save('php://output');

  } catch (PDOException $e) {
    http_response_code(500);
    error_log("Excel export error: " . $e->getMessage());
    echo json_encode(['success' => false, 'message' => 'Database error: ' . $e->getMessage()]);
  } catch (Exception $e) {
    http_response_code(500);
    error_log("Excel export error: " . $e->getMessage());
    echo json_encode(['success' => false, 'message' => 'Export error: ' . $e->getMessage()]);
  }
}

// Execute export
try {
  if (!$pdo) {
    throw new Exception("Database connection failed");
  }

  exportOeeDataLog($pdo);

} catch (Exception $e) {
  http_response_code(500);
  error_log("Fatal error in OEE data log export: " . $e->getMessage());
  echo json_encode(['success' => false, 'message' => 'Server Error: ' . $e->getMessage()]);
}
?>
