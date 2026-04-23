<?php
/**
 * 자수기(EMB) 시간대별 집계 로그 Excel 내보내기
 * 대상 테이블: data_oee_rows_hourly_emb
 */

require_once __DIR__ . '/export_common.php';

use PhpOffice\PhpSpreadsheet\Spreadsheet;
use PhpOffice\PhpSpreadsheet\Writer\Xlsx;
use PhpOffice\PhpSpreadsheet\Style\Fill;
use PhpOffice\PhpSpreadsheet\Style\Alignment;
use PhpOffice\PhpSpreadsheet\Style\Border;

function exportEmbHourlyDataLog(PDO $pdo)
{
    try {
        $query_conditions = buildExportFilterParams('dohe');

        $sql = "
            SELECT
                dohe.idx, dohe.work_date, dohe.time_update, dohe.shift_idx, dohe.work_hour,
                dohe.factory_idx, dohe.factory_name, dohe.line_idx, dohe.line_name,
                dohe.mac, dohe.machine_idx, dohe.machine_no, dohe.process_name,
                dohe.planned_work_time, dohe.runtime, dohe.actual_output,
                dohe.cycle_time, dohe.thread_breakage, dohe.motor_run_time,
                dohe.pair_info, dohe.pair_count,
                dohe.reg_date, dohe.update_date
            FROM data_oee_rows_hourly_emb dohe
            {$query_conditions['where_sql']}
            ORDER BY dohe.work_date DESC, dohe.work_hour DESC, dohe.update_date DESC
            LIMIT 10000";

        $stmt = $pdo->prepare($sql);
        $stmt->execute($query_conditions['params']);
        $data = $stmt->fetchAll(PDO::FETCH_ASSOC);

        $spreadsheet = new Spreadsheet();
        $sheet = $spreadsheet->getActiveSheet();
        $sheet->setTitle('EMB Hourly Data Log');

        $headers = [
            'idx', 'work_date', 'time_update', 'shift_idx', 'work_hour',
            'factory_idx', 'factory_name', 'line_idx', 'line_name',
            'mac', 'machine_idx', 'machine_no', 'process_name',
            'planned_work_time', 'runtime', 'actual_output',
            'cycle_time', 'thread_breakage', 'motor_run_time',
            'pair_info', 'pair_count', 'reg_date', 'update_date',
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
                $row['idx'] ?? '', $row['work_date'] ?? '', $row['time_update'] ?? '',
                $row['shift_idx'] ?? '', $row['work_hour'] ?? '',
                $row['factory_idx'] ?? '', $row['factory_name'] ?? '', $row['line_idx'] ?? '', $row['line_name'] ?? '',
                $row['mac'] ?? '', $row['machine_idx'] ?? '', $row['machine_no'] ?? '', $row['process_name'] ?? '',
                $row['planned_work_time'] ?? '', $row['runtime'] ?? '', $row['actual_output'] ?? '',
                $row['cycle_time'] ?? '', $row['thread_breakage'] ?? '', $row['motor_run_time'] ?? '',
                $row['pair_info'] ?? '', $row['pair_count'] ?? '',
                $row['reg_date'] ?? '', $row['update_date'] ?? '',
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

        $filename_parts = ['EMB_Hourly_Data_Log'];
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
        error_log("EMB hourly export error: " . $e->getMessage());
        echo json_encode(['success' => false, 'message' => $e->getMessage()]);
    }
}

try {
    if (!$pdo) throw new Exception("Database connection failed");
    exportEmbHourlyDataLog($pdo);
} catch (Exception $e) {
    http_response_code(500);
    echo json_encode(['success' => false, 'message' => $e->getMessage()]);
}
