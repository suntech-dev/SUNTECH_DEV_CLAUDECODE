<?php
/**
 * 자수기(EMB) 패킷 스냅샷 로그 Excel 내보내기
 * 대상 테이블: data_oee_rows_emb
 */

require_once __DIR__ . '/export_common.php';

use PhpOffice\PhpSpreadsheet\Spreadsheet;
use PhpOffice\PhpSpreadsheet\Writer\Xlsx;
use PhpOffice\PhpSpreadsheet\Style\Fill;
use PhpOffice\PhpSpreadsheet\Style\Alignment;
use PhpOffice\PhpSpreadsheet\Style\Border;

function exportEmbRowDataLog(PDO $pdo)
{
    try {
        $query_conditions = buildExportFilterParams('dore');

        $sql = "
            SELECT
                dore.idx, dore.work_date, dore.time_update, dore.shift_idx,
                dore.factory_idx, dore.factory_name, dore.line_idx, dore.line_name,
                dore.mac, dore.machine_idx, dore.machine_no, dore.process_name,
                dore.planned_work_time, dore.runtime, dore.actual_output, dore.packet_qty,
                dore.cycle_time, dore.thread_breakage, dore.motor_run_time,
                dore.pair_info, dore.pair_count,
                dore.work_hour, dore.reg_date
            FROM data_oee_rows_emb dore
            {$query_conditions['where_sql']}
            ORDER BY dore.work_date DESC, dore.reg_date DESC
            LIMIT 10000";

        $stmt = $pdo->prepare($sql);
        $stmt->execute($query_conditions['params']);
        $data = $stmt->fetchAll(PDO::FETCH_ASSOC);

        $spreadsheet = new Spreadsheet();
        $sheet = $spreadsheet->getActiveSheet();
        $sheet->setTitle('EMB Row Data Log');

        $headers = [
            'idx', 'work_date', 'time_update', 'shift_idx',
            'factory_idx', 'factory_name', 'line_idx', 'line_name',
            'mac', 'machine_idx', 'machine_no', 'process_name',
            'planned_work_time', 'runtime', 'actual_output', 'packet_qty',
            'cycle_time', 'thread_breakage', 'motor_run_time',
            'pair_info', 'pair_count', 'work_hour', 'reg_date',
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
                $row['planned_work_time'] ?? '', $row['runtime'] ?? '', $row['actual_output'] ?? '', $row['packet_qty'] ?? '',
                $row['cycle_time'] ?? '', $row['thread_breakage'] ?? '', $row['motor_run_time'] ?? '',
                $row['pair_info'] ?? '', $row['pair_count'] ?? '',
                $row['work_hour'] ?? '', $row['reg_date'] ?? '',
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

        $filename_parts = ['EMB_Row_Data_Log'];
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
        error_log("EMB row export error: " . $e->getMessage());
        echo json_encode(['success' => false, 'message' => $e->getMessage()]);
    }
}

try {
    if (!$pdo) throw new Exception("Database connection failed");
    exportEmbRowDataLog($pdo);
} catch (Exception $e) {
    http_response_code(500);
    echo json_encode(['success' => false, 'message' => $e->getMessage()]);
}
