<?php

/**
 * OEE Data Excel Export API
 *
 * Features:
 * - Excel export with filtering support (Factory → Line → Machine)
 * - Date range filtering
 * - Shift filtering
 * - PhpSpreadsheet-based Excel generation
 */

require_once __DIR__ . '/export_common.php';

use PhpOffice\PhpSpreadsheet\Spreadsheet;
use PhpOffice\PhpSpreadsheet\Writer\Xlsx;
use PhpOffice\PhpSpreadsheet\Style\Fill;
use PhpOffice\PhpSpreadsheet\Style\Alignment;
use PhpOffice\PhpSpreadsheet\Style\Border;
use PhpOffice\PhpSpreadsheet\Cell\DataType;

/**
 * Export OEE data to Excel
 */
function exportOeeData(PDO $pdo)
{
    try {
        $query_conditions = buildExportFilterParams('do', 'work_date', false, 'do.work_date >= DATE_SUB(CURDATE(), INTERVAL 7 DAY)');

        // Query OEE data with all necessary fields
        $sql = "
      SELECT
        do.idx,
        do.work_date,
        do.shift_idx,
        do.machine_no,
        do.oee as overall_oee,
        do.availabilty_rate as availability,
        do.productivity_rate as performance,
        do.quality_rate as quality,
        do.planned_work_time as planned_time,
        do.runtime,
        do.downtime,
        do.actual_output,
        do.theoritical_output as theoretical_output,
        do.actual_a_grade as good_products,
        do.defective as defective_count,
        do.cycletime as cycle_time,
        do.update_date,
        f.factory_name,
        l.line_name
      FROM data_oee do
      LEFT JOIN info_factory f ON do.factory_idx = f.idx
      LEFT JOIN info_line l ON do.line_idx = l.idx
      {$query_conditions['where_sql']}
      ORDER BY do.work_date DESC, do.update_date DESC, do.idx DESC
    ";

        $stmt = $pdo->prepare($sql);
        $stmt->execute($query_conditions['params']);
        $data = $stmt->fetchAll(PDO::FETCH_ASSOC);

        // Create Excel spreadsheet
        $spreadsheet = new Spreadsheet();
        $sheet = $spreadsheet->getActiveSheet();
        $sheet->setTitle('OEE Data');

        // Define headers
        $headers = [
            'NO',
            'Machine No',
            'Factory/Line',
            'Shift',
            'Overall OEE (%)',
            'Availability (%)',
            'Performance (%)',
            'Quality (%)',
            'Work Date',
            'Update Time',
            'Runtime',
            'Planned Time',
            'Downtime',
            'Actual Output',
            'Theoretical Output',
            'Defective Count',
            'Cycle Time'
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

            // Format factory/line display
            $factoryLineDisplay = ($row['factory_name'] ?? '-') . ' / ' . ($row['line_name'] ?? '-');

            // Write row data
            $sheet->fromArray([
                $index + 1,
                $row['machine_no'] ?? '-',
                $factoryLineDisplay,
                $shiftDisplay,
                $row['overall_oee'] ?? '0',
                $row['availability'] ?? '0',
                $row['performance'] ?? '0',
                $row['quality'] ?? '0',
                $row['work_date'] ?? '-',
                $row['update_date'] ?? '-',
                $row['runtime'] ?? '-',
                $row['planned_time'] ?? '-',
                $row['downtime'] ?? '-',
                $row['actual_output'] ?? '-',
                $row['theoretical_output'] ?? '-',
                $row['defective_count'] ?? '-',
                $row['cycle_time'] ?? '-'
            ], NULL, 'A' . $rowNum);

            // Apply alternating row colors
            if ($rowNum % 2 == 0) {
                $sheet->getStyle('A' . $rowNum . ':' . $sheet->getHighestColumn() . $rowNum)
                    ->getFill()
                    ->setFillType(Fill::FILL_SOLID)
                    ->getStartColor()->setRGB('F9F9F9');
            }

            // Apply color to OEE cells based on value
            $oeeValue = floatval($row['overall_oee'] ?? 0);
            $oeeCellStyle = [];
            if ($oeeValue >= 85) {
                $oeeCellStyle = [
                    'fill' => [
                        'fillType' => Fill::FILL_SOLID,
                        'startColor' => ['rgb' => 'D4EDDA']  // Light green
                    ]
                ];
            } elseif ($oeeValue >= 70) {
                $oeeCellStyle = [
                    'fill' => [
                        'fillType' => Fill::FILL_SOLID,
                        'startColor' => ['rgb' => 'FFF3CD']  // Light yellow
                    ]
                ];
            } elseif ($oeeValue > 0) {
                $oeeCellStyle = [
                    'fill' => [
                        'fillType' => Fill::FILL_SOLID,
                        'startColor' => ['rgb' => 'F8D7DA']  // Light red
                    ]
                ];
            }

            if (!empty($oeeCellStyle)) {
                $sheet->getStyle('E' . $rowNum)->applyFromArray($oeeCellStyle);
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
        $filename_parts = ['OEE_Data'];
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

    exportOeeData($pdo);
} catch (Exception $e) {
    http_response_code(500);
    error_log("Fatal error in OEE export: " . $e->getMessage());
    echo json_encode(['success' => false, 'message' => 'Server Error: ' . $e->getMessage()]);
}
