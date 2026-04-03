<?php

/**
 * ============================================================
 * 파일명  : log_oee_hourly_export_2.php
 * 목  적  : OEE 시간별(Hourly) 데이터 로그를 Excel(.xlsx) 파일로 내보내기
 *
 * 주요 기능:
 *  - data_oee_rows_hourly 테이블의 전체 컬럼 데이터를 Excel로 출력
 *  - 공장(Factory) → 라인(Line) → 기계(Machine) 계층 필터 지원
 *  - 날짜 범위(start_date / end_date) 필터 지원
 *  - 교대(Shift) 필터 지원
 *  - PhpSpreadsheet 라이브러리를 이용한 Excel 생성
 *  - SAP Fiori 스타일(파란색 헤더, 교번 행 색상) 적용
 *  - 최대 10,000건 조회 제한
 *
 * log_oee_export_2.php 와의 차이점:
 *  - 대상 테이블 : data_oee_rows_hourly (시간 단위 집계 행 데이터)
 *  - LEFT JOIN 없이 테이블 내장 factory_name, line_name 컬럼 직접 사용
 *  - 정렬 기준에 work_hour(작업 시간대) 추가
 *
 * 의존 파일:
 *  - export_common.php : buildExportFilterParams(), PDO $pdo 초기화
 * ============================================================
 */

require_once __DIR__ . '/export_common.php';

/* PhpSpreadsheet 네임스페이스 임포트 */
use PhpOffice\PhpSpreadsheet\Spreadsheet;
use PhpOffice\PhpSpreadsheet\Writer\Xlsx;
use PhpOffice\PhpSpreadsheet\Style\Fill;
use PhpOffice\PhpSpreadsheet\Style\Alignment;
use PhpOffice\PhpSpreadsheet\Style\Border;
use PhpOffice\PhpSpreadsheet\Cell\DataType;

/**
 * exportOeeHourlyDataLog — data_oee_rows_hourly 테이블의 시간별 OEE 로그를 Excel로 내보내기
 *
 * @param PDO $pdo  데이터베이스 연결 객체
 * @return void     Excel 파일을 HTTP 응답으로 직접 출력(php://output)
 *
 * 처리 순서:
 *  1. GET 파라미터로 필터 조건(WHERE절, 바인딩 파라미터) 생성
 *  2. data_oee_rows_hourly 테이블에서 데이터 조회
 *  3. PhpSpreadsheet 객체 생성 및 헤더 행 작성
 *  4. SAP Fiori Blue(#0070F2) 헤더 스타일 적용
 *  5. 데이터 행 순서대로 기록 + 짝수 행 연회색(#F9F9F9) 배경 적용
 *  6. 전체 테두리 스타일 일괄 적용
 *  7. 컬럼 너비 자동 조정
 *  8. 파일명 생성 후 HTTP 다운로드 헤더 설정 → php://output 으로 저장
 */
function exportOeeHourlyDataLog(PDO $pdo)
{
    try {
        /* ----------------------------------------------------------------
         * 1. GET 파라미터로 WHERE 절 및 바인딩 파라미터 배열 생성
         *    buildExportFilterParams('doh') → 테이블 별칭 'doh' 기준으로
         *    factory_idx, line_idx, machine_idx, shift_idx, work_date 필터 적용
         * ---------------------------------------------------------------- */
        $query_conditions = buildExportFilterParams('doh');

        /* ----------------------------------------------------------------
         * 2. data_oee_rows_hourly 테이블 전체 컬럼 조회 SQL
         *    - factory_name, line_name 은 테이블 내 컬럼으로 직접 조회
         *    - 최신 작업일(work_date DESC) → 최신 작업 시간(work_hour DESC)
         *      → 최신 업데이트(update_date DESC) 순 정렬
         *    - 성능 보호를 위해 LIMIT 10000 적용
         * ---------------------------------------------------------------- */
        // Query all columns from data_oee_rows_hourly table
        $sql = "
      SELECT
        doh.idx,
        doh.work_date,
        doh.time_update,
        doh.shift_idx,
        doh.factory_idx,
        doh.factory_name,
        doh.line_idx,
        doh.line_name,
        doh.mac,
        doh.machine_idx,
        doh.machine_no,
        doh.process_name,
        doh.planned_work_time,
        doh.runtime,
        doh.productive_runtime,
        doh.downtime,
        doh.availabilty_rate,
        doh.target_line_per_day,
        doh.target_line_per_hour,
        doh.target_mc_per_day,
        doh.target_mc_per_hour,
        doh.cycletime,
        doh.pair_info,
        doh.pair_count,
        doh.theoritical_output,
        doh.actual_output,
        doh.productivity_rate,
        doh.defective,
        doh.actual_a_grade,
        doh.quality_rate,
        doh.oee,
        doh.reg_date,
        doh.update_date,
        doh.work_hour
      FROM data_oee_rows_hourly doh
      {$query_conditions['where_sql']}
      ORDER BY doh.work_date DESC, doh.work_hour DESC, doh.update_date DESC, doh.idx DESC
      LIMIT 10000
    ";

        /* SQL 실행 및 결과 배열 취득 */
        $stmt = $pdo->prepare($sql);
        $stmt->execute($query_conditions['params']);
        $data = $stmt->fetchAll(PDO::FETCH_ASSOC);

        /* ----------------------------------------------------------------
         * 3. PhpSpreadsheet 객체 생성 및 시트 초기화
         * ---------------------------------------------------------------- */
        // Create Excel spreadsheet
        $spreadsheet = new Spreadsheet();
        $sheet = $spreadsheet->getActiveSheet();
        $sheet->setTitle('OEE Hourly Data Log'); // 시트 탭 이름 설정

        /* ----------------------------------------------------------------
         * 헤더 행 정의 (data_oee_rows_hourly 테이블 컬럼 순서와 동일)
         *  - 총 34개 컬럼 (work_hour 포함)
         * ---------------------------------------------------------------- */
        // Define all headers based on data_oee_rows_hourly table columns
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

        /* 헤더 배열을 A1 셀부터 한 행으로 일괄 기록 */
        // Write headers
        $sheet->fromArray($headers, NULL, 'A1');

        /* ----------------------------------------------------------------
         * 4. 헤더 행 스타일 정의 (SAP Fiori Blue 테마)
         *    - 배경색 : #0070F2 (SAP Fiori 공식 파란색)
         *    - 글꼴   : 흰색 Bold
         *    - 정렬   : 수평·수직 중앙
         *    - 테두리 : 얇은 회색 선
         * ---------------------------------------------------------------- */
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
        /* 헤더 행(1행) 전체 컬럼에 스타일 일괄 적용 */
        $sheet->getStyle('A1:' . $sheet->getHighestColumn() . '1')->applyFromArray($headerStyle);

        /* 헤더 행 높이 25pt 고정 */
        // Set header row height
        $sheet->getRowDimension(1)->setRowHeight(25);

        /* ----------------------------------------------------------------
         * 5. 데이터 행 기록 루프
         *    - $rowNum : 2행부터 시작 (1행은 헤더)
         *    - null 안전 연산자(??)로 값이 없을 경우 빈 문자열 처리
         *    - 짝수 행은 연회색(#F9F9F9) 배경 적용
         * ---------------------------------------------------------------- */
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

            /* 짝수 행에 연회색 배경 적용 (가독성을 위한 줄무늬 패턴) */
            // Apply alternating row colors
            if ($rowNum % 2 == 0) {
                $sheet->getStyle('A' . $rowNum . ':' . $sheet->getHighestColumn() . $rowNum)
                    ->getFill()
                    ->setFillType(Fill::FILL_SOLID)
                    ->getStartColor()->setRGB('F9F9F9');
            }

            $rowNum++;
        }

        /* ----------------------------------------------------------------
         * 6. 데이터 영역 전체(2행 ~ 마지막 행)에 테두리 및 수직 중앙 정렬 적용
         *    - 데이터가 한 건 이상인 경우($rowNum > 2)에만 실행
         * ---------------------------------------------------------------- */
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

        /* ----------------------------------------------------------------
         * 7. 모든 컬럼 너비 자동 조정 (내용에 맞게 자동 계산)
         * ---------------------------------------------------------------- */
        // Auto-size columns
        foreach ($sheet->getColumnIterator() as $column) {
            $sheet->getColumnDimension($column->getColumnIndex())->setAutoSize(true);
        }

        /* ----------------------------------------------------------------
         * 8. 다운로드 파일명 생성
         *    - 기본 접두사 : OEE_Hourly_Data_Log
         *    - start_date / end_date GET 파라미터가 있으면 파일명에 추가
         *    - 타임스탬프(YmdHis) 후미 추가로 중복 방지
         *    예) OEE_Hourly_Data_Log_2025-01-01_2025-01-31_20250131120000.xlsx
         * ---------------------------------------------------------------- */
        // Generate filename with timestamp and filters
        $filename_parts = ['OEE_Hourly_Data_Log'];
        if (!empty($_GET['start_date'])) {
            $filename_parts[] = $_GET['start_date'];
        }
        if (!empty($_GET['end_date'])) {
            $filename_parts[] = $_GET['end_date'];
        }
        $filename = implode('_', $filename_parts) . '_' . date('YmdHis') . '.xlsx';

        /* ----------------------------------------------------------------
         * Excel 파일 다운로드 HTTP 헤더 설정
         *  - Content-Type : xlsx MIME 타입
         *  - Content-Disposition : 첨부 파일명 지정
         *  - Cache-Control : 브라우저 캐시 비활성화
         * ---------------------------------------------------------------- */
        // Set headers for download
        header('Content-Type: application/vnd.openxmlformats-officedocument.spreadsheetml.sheet');
        header('Content-Disposition: attachment;filename="' . $filename . '"');
        header('Cache-Control: max-age=0');

        /* php://output 스트림으로 직접 저장 (메모리 절약) */
        // Write to output
        $writer = new Xlsx($spreadsheet);
        $writer->save('php://output');
    } catch (PDOException $e) {
        /* DB 오류 발생 시 500 응답 및 에러 로그 기록 */
        http_response_code(500);
        error_log("Excel export error: " . $e->getMessage());
        echo json_encode(['success' => false, 'message' => 'Database error: ' . $e->getMessage()]);
    } catch (Exception $e) {
        /* PhpSpreadsheet 또는 기타 오류 발생 시 500 응답 */
        http_response_code(500);
        error_log("Excel export error: " . $e->getMessage());
        echo json_encode(['success' => false, 'message' => 'Export error: ' . $e->getMessage()]);
    }
}

/* ================================================================
 * 진입점 — export_common.php에서 초기화된 $pdo를 사용해 내보내기 실행
 *  - $pdo가 null이면 즉시 예외를 발생시켜 오류 응답 반환
 * ================================================================ */
// Execute export
try {
    if (!$pdo) {
        throw new Exception("Database connection failed");
    }

    exportOeeHourlyDataLog($pdo);
} catch (Exception $e) {
    http_response_code(500);
    error_log("Fatal error in OEE hourly data log export: " . $e->getMessage());
    echo json_encode(['success' => false, 'message' => 'Server Error: ' . $e->getMessage()]);
}
