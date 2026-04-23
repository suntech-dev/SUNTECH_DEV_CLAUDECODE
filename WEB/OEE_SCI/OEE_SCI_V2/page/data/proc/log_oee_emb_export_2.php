<?php
/**
 * 자수기(EMB) 일별 집계 로그 Excel 내보내기
 * 대상 테이블: data_oee_emb
 */

require_once __DIR__ . '/export_common.php';

use PhpOffice\PhpSpreadsheet\Spreadsheet;
use PhpOffice\PhpSpreadsheet\Writer\Xlsx;
use PhpOffice\PhpSpreadsheet\Style\Fill;
use PhpOffice\PhpSpreadsheet\Style\Alignment;
use PhpOffice\PhpSpreadsheet\Style\Border;

function exportEmbDataLog(PDO $pdo)
{
    try {
        $query_conditions = buildExportFilterParams('doe');

        $sql = "
            SELECT
                doe.idx, doe.work_date, doe.time_update, doe.shift_idx,
                doe.factory_idx, doe.factory_name, doe.line_idx, doe.line_name,
                doe.mac, doe.machine_idx, doe.machine_no, doe.process_name,
                doe.planned_work_time, doe.runtime, doe.actual_output,
                doe.cycle_time, doe.thread_breakage, doe.motor_run_time,
                doe.pair_info, doe.pair_count,
                doe.work_hour, doe.reg_date, doe.update_date
            FROM data_oee_emb doe
            {$query_conditions['where_sql']}
            ORDER BY doe.work_date DESC, doe.update_date DESC
            LIMIT 10000";

        $stmt = $pdo->prepare($sql);
        $stmt->execute($query_conditions['params']);
        $data = $stmt->fetchAll(PDO::FETCH_ASSOC);

        $spreadsheet = new Spreadsheet();
        $sheet = $spreadsheet->getActiveSheet();
        $sheet->setTitle('EMB Data Log');

        $headers = [
            'idx', 'work_date', 'time_update', 'shift_idx',
            'factory_idx', 'factory_name', 'line_idx', 'line_name',
            'mac', 'machine_idx', 'machine_no', 'process_name',
            'planned_work_time', 'runtime', 'actual_output',
            'cycle_time', 'thread_breakage', 'motor_run_time',
            'pair_info', 'pair_count',
            'work_hour', 'reg_date', 'update_date',
        ];
        $sheet->fromArray($headers, NULL, 'A1');

        $headerStyle = [
            'font'      => ['bold' => true, 'color' => ['rgb' => 'FFFFFF']],
            'fill'      => ['fillType' => Fill::FILL_SOLID, 'startColor' => ['rgb' => '0070F2']],
            'alignment' => ['horizontal' => Alignment::HORIZONTAL_CENTER, 'vertical' => Alignment::VERTICAL_CENTER],
            'borders'   => ['allBorders' => ['borderStyle' => Border::BORDER_THIN, 'color' => ['rgb' => 'CCCCCC']]],
        ];
        $sheet->getStyle('A1:' . $sheet->getHighestColumn() . '1')->applyFromArray($headerStyle);
        $sheet->getRowDimension(1)->setRowHeight(25);

        $rowNum = 2;
        foreach ($data as $row) {
            $sheet->fromArray([
                $row['idx'] ?? '', $row['work_date'] ?? '', $row['time_update'] ?? '', $row['shift_idx'] ?? '',
                $row['factory_idx'] ?? '', $row['factory_name'] ?? '', $row['line_idx'] ?? '', $row['line_name'] ?? '',
                $row['mac'] ?? '', $row['machine_idx'] ?? '', $row['machine_no'] ?? '', $row['process_name'] ?? '',
                $row['planned_work_time'] ?? '', $row['runtime'] ?? '', $row['actual_output'] ?? '',
                $row['cycle_time'] ?? '', $row['thread_breakage'] ?? '', $row['motor_run_time'] ?? '',
                $row['pair_info'] ?? '', $row['pair_count'] ?? '',
                $row['work_hour'] ?? '', $row['reg_date'] ?? '', $row['update_date'] ?? '',
            ], NULL, 'A' . $rowNum);

            if ($rowNum % 2 == 0) {
                $sheet->getStyle('A' . $rowNum . ':' . $sheet->getHighestColumn() . $rowNum)
                    ->getFill()->setFillType(Fill::FILL_SOLID)->getStartColor()->setRGB('F9F9F9');
            }
            $rowNum++;
        }

        if ($rowNum > 2) {
            $sheet->getStyle('A2:' . $sheet->getHighestColumn() . ($rowNum - 1))->applyFromArray([
                'borders'   => ['allBorders' => ['borderStyle' => Border::BORDER_THIN, 'color' => ['rgb' => 'CCCCCC']]],
                'alignment' => ['vertical' => Alignment::VERTICAL_CENTER],
            ]);
        }

        foreach ($sheet->getColumnIterator() as $column) {
            $sheet->getColumnDimension($column->getColumnIndex())->setAutoSize(true);
        }

        $filename_parts = ['EMB_Data_Log'];
        if (!empty($_GET['start_date'])) $filename_parts[] = $_GET['start_date'];
        if (!empty($_GET['end_date']))   $filename_parts[] = $_GET['end_date'];
        $filename = implode('_', $filename_parts) . '_' . date('YmdHis') . '.xlsx';

        header('Content-Type: application/vnd.openxmlformats-officedocument.spreadsheetml.sheet');
        header('Content-Disposition: attachment;filename="' . $filename . '"');
        header('Cache-Control: max-age=0');

        $writer = new Xlsx($spreadsheet);
        $writer->save('php://output');
    } catch (Exception $e) {
        http_response_code(500);
        error_log("EMB export error: " . $e->getMessage());
        echo json_encode(['success' => false, 'message' => $e->getMessage()]);
    }
}

try {
    if (!$pdo) throw new Exception("Database connection failed");
    exportEmbDataLog($pdo);
} catch (Exception $e) {
    http_response_code(500);
    echo json_encode(['success' => false, 'message' => $e->getMessage()]);
}
