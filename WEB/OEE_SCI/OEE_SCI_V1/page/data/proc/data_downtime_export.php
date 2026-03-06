<?php
/**
 * Downtime Data Excel Export API
 *
 * Features:
 * - Excel export with filtering support (Factory → Line → Machine)
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

date_default_timezone_set('Asia/Jakarta');

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
    $where_clauses[] = 'dd.factory_idx = ?';
    $params[] = $_GET['factory_filter'];
  }

  if (!empty($_GET['line_filter'])) {
    $where_clauses[] = 'dd.line_idx = ?';
    $params[] = $_GET['line_filter'];
  }

  if (!empty($_GET['machine_filter'])) {
    $where_clauses[] = 'dd.machine_idx = ?';
    $params[] = $_GET['machine_filter'];
  }

  if (!empty($_GET['shift_filter'])) {
    $where_clauses[] = 'dd.shift_idx = ?';
    $params[] = $_GET['shift_filter'];
  }

  if (!empty($_GET['start_date'])) {
    $where_clauses[] = 'dd.reg_date >= ?';
    $params[] = $_GET['start_date'] . ' 00:00:00';
  }

  if (!empty($_GET['end_date'])) {
    $where_clauses[] = 'dd.reg_date <= ?';
    $params[] = $_GET['end_date'] . ' 23:59:59';
  }

  // Default date range if not specified (last 2 days)
  if (empty($_GET['start_date']) && empty($_GET['end_date'])) {
    $where_clauses[] = 'dd.reg_date >= DATE_SUB(NOW(), INTERVAL 2 DAY)';
  }

  $where_sql = count($where_clauses) > 0 ? ' WHERE ' . implode(' AND ', $where_clauses) : '';

  return ['where_sql' => $where_sql, 'params' => $params];
}

/**
 * Export downtime data to Excel
 */
function exportDowntimes(PDO $pdo) {
  try {
    $query_conditions = parseFilterParams();

    // Query downtime data with all necessary fields
    $sql = "
      SELECT
        dd.idx,
        f.factory_name,
        l.line_name,
        dd.machine_no,
        dd.shift_idx,
        dd.downtime_name,
        dd.status,
        dd.reg_date,
        dd.work_date,
        dd.update_date,
        dd.duration_his,
        dd.duration_sec,
        CASE
          WHEN dd.status = 'Warning' THEN
            CONCAT(
              CASE
                WHEN TIMESTAMPDIFF(HOUR, dd.reg_date, NOW()) > 0
                THEN CONCAT(TIMESTAMPDIFF(HOUR, dd.reg_date, NOW()), 'h ')
                ELSE ''
              END,
              CASE
                WHEN TIMESTAMPDIFF(MINUTE, dd.reg_date, NOW()) % 60 > 0
                THEN CONCAT(TIMESTAMPDIFF(MINUTE, dd.reg_date, NOW()) % 60, 'm ')
                ELSE ''
              END,
              TIMESTAMPDIFF(SECOND, dd.reg_date, NOW()) % 60, 's'
            )
          WHEN dd.duration_sec IS NULL OR dd.duration_sec = 0 THEN 'in progress'
          ELSE dd.duration_his
        END as duration
      FROM data_downtime dd
      LEFT JOIN info_factory f ON dd.factory_idx = f.idx
      LEFT JOIN info_line l ON dd.line_idx = l.idx
      {$query_conditions['where_sql']}
      ORDER BY dd.reg_date DESC, dd.idx DESC
    ";

    $stmt = $pdo->prepare($sql);
    $stmt->execute($query_conditions['params']);
    $data = $stmt->fetchAll(PDO::FETCH_ASSOC);

    // Create Excel spreadsheet
    $spreadsheet = new Spreadsheet();
    $sheet = $spreadsheet->getActiveSheet();
    $sheet->setTitle('Downtime Data');

    // Define headers
    $headers = [
      'NO',
      'ID',
      'Machine No',
      'Factory/Line',
      'Shift',
      'Downtime Type',
      'Status',
      'Occurrence Time',
      'Resolution Time',
      'Duration',
      'Work Date'
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
    foreach ($data as $index => $row) {
      // Format shift display
      $shiftDisplay = '-';
      if (!empty($row['shift_idx'])) {
        $shiftDisplay = 'Shift ' . $row['shift_idx'];
      }

      // Format status display
      $statusDisplay = '-';
      if ($row['status'] === 'Warning') {
        $statusDisplay = '⚠️ Warning';
      } elseif ($row['status'] === 'Completed') {
        $statusDisplay = '✅ Completed';
      } else {
        $statusDisplay = $row['status'];
      }

      // Format factory/line display
      $factoryLineDisplay = ($row['factory_name'] ?? '-') . ' / ' . ($row['line_name'] ?? '-');

      // Write row data
      $sheet->fromArray([
        $index + 1,
        $row['idx'],
        $row['machine_no'] ?? '-',
        $factoryLineDisplay,
        $shiftDisplay,
        $row['downtime_name'] ?? '-',
        $statusDisplay,
        $row['reg_date'] ?? '-',
        $row['update_date'] ?? '-',
        $row['duration'] ?? '-',
        $row['work_date'] ?? '-'
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

    // Add filter info as a separate sheet note (optional)
    $filterInfo = "Export Date: " . date('Y-m-d H:i:s') . "\n";
    $filterInfo .= "Filters Applied:\n";
    $filterInfo .= "- Factory: " . ($_GET['factory_filter'] ?? 'All') . "\n";
    $filterInfo .= "- Line: " . ($_GET['line_filter'] ?? 'All') . "\n";
    $filterInfo .= "- Machine: " . ($_GET['machine_filter'] ?? 'All') . "\n";
    $filterInfo .= "- Shift: " . ($_GET['shift_filter'] ?? 'All') . "\n";
    $filterInfo .= "- Date Range: " .
      ($_GET['start_date'] ?? 'N/A') . ' ~ ' . ($_GET['end_date'] ?? 'N/A');

    // Generate filename with timestamp and filters
    $filename_parts = ['downtime_data'];
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

  exportDowntimes($pdo);

} catch (Exception $e) {
  http_response_code(500);
  error_log("Fatal error in downtime export: " . $e->getMessage());
  echo json_encode(['success' => false, 'message' => 'Server Error: ' . $e->getMessage()]);
}
?>
