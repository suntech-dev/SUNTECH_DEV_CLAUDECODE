<?php

/**
 * ============================================================
 * 파일명  : log_oee_row_export_2.php
 * 목  적  : OEE Row(개별 행) 데이터 로그를 Excel(.xlsx) 파일로 내보내기
 *
 * 주요 기능:
 *  - data_oee_rows 테이블의 전체 컬럼 데이터를 Excel로 출력
 *  - 공장(Factory) → 라인(Line) → 기계(Machine) 계층 필터 지원
 *  - 날짜 범위(start_date / end_date) 필터 지원
 *  - 교대(Shift) 필터 지원
 *  - PhpSpreadsheet 라이브러리를 이용한 Excel 생성
 *  - SAP Fiori 스타일(파란색 헤더, 교번 행 색상) 적용
 *  - 최대 10,000건 조회 제한
 *
 * log_oee_export_2.php / log_oee_hourly_export_2.php 와의 차이점:
 *  - 대상 테이블 : data_oee_rows (개별 행 단위 OEE 데이터)
 *  - factory_name, line_name 컬럼은 테이블 내장 컬럼 직접 사용 (JOIN 불필요)
 *  - update_date 컬럼 없음 → reg_date 기준 정렬
 *  - 정렬 기준 : work_date DESC → work_hour DESC → reg_date DESC → idx DESC
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
 * exportOeeRowDataLog — data_oee_rows 테이블의 개별 행 OEE 로그를 Excel로 내보내기
 *
 * @param PDO $pdo  데이터베이스 연결 객체
 * @return void     Excel 파일을 HTTP 응답으로 직접 출력(php://output)
 *
 * 처리 순서:
 *  1. GET 파라미터로 필터 조건(WHERE절, 바인딩 파라미터) 생성
 *  2. data_oee_rows 테이블에서 데이터 조회 (LIMIT 10000)
 *  3. PhpSpreadsheet 객체 생성 및 헤더 행 작성
 *  4. SAP Fiori Blue(#0070F2) 헤더 스타일 적용
 *  5. 데이터 행 순서대로 기록 + 짝수 행 연회색(#F9F9F9) 배경 적용
 *  6. 데이터 전체 영역 테두리 스타일 일괄 적용
 *  7. 컬럼 너비 자동 조정
 *  8. 파일명 생성 후 HTTP 다운로드 헤더 설정 → php://output 으로 저장
 */
function exportOeeRowDataLog(PDO $pdo)
{
    try {
        /* ----------------------------------------------------------------
         * 1. GET 파라미터로 WHERE 절 및 바인딩 파라미터 배열 생성
         *    buildExportFilterParams('dor') → 테이블 별칭 'dor' 기준으로
         *    factory_idx, line_idx, machine_idx, shift_idx, work_date 필터 적용
         * ---------------------------------------------------------------- */
        $query_conditions = buildExportFilterParams('dor');

        /* ----------------------------------------------------------------
         * 2. data_oee_rows 테이블 전체 컬럼 조회 SQL
         *    - factory_name, line_name 은 테이블 내장 컬럼으로 직접 사용
         *    - update_date 컬럼이 없으므로 reg_date 기준으로 정렬
         *    - 정렬 : work_date DESC → work_hour DESC → reg_date DESC → idx DESC
         *    - 성능 보호를 위해 LIMIT 10000 적용
         * ---------------------------------------------------------------- */
        // Query all columns from data_oee_rows table
        $sql = "
      SELECT
        dor.idx,
        dor.work_date,
        dor.time_update,
        dor.shift_idx,
        dor.factory_idx,
        dor.factory_name,
        dor.line_idx,
        dor.line_name,
        dor.mac,
        dor.machine_idx,
        dor.machine_no,
        dor.process_name,
        dor.planned_work_time,
        dor.runtime,
        dor.productive_runtime,
        dor.downtime,
        dor.availabilty_rate,
        dor.target_line_per_day,
        dor.target_line_per_hour,
        dor.target_mc_per_day,
        dor.target_mc_per_hour,
        dor.cycletime,
        dor.pair_info,
        dor.pair_count,
        dor.theoritical_output,
        dor.actual_output,
        dor.productivity_rate,
        dor.defective,
        dor.actual_a_grade,
        dor.quality_rate,
        dor.oee,
        dor.reg_date,
        dor.work_hour
      FROM data_oee_rows dor
      {$query_conditions['where_sql']}
      ORDER BY dor.work_date DESC, dor.work_hour DESC, dor.reg_date DESC, dor.idx DESC
      LIMIT 10000
    ";

        /* SQL 실행 및 결과 배열 취득 (연관 배열 형태) */
        $stmt = $pdo->prepare($sql);
        $stmt->execute($query_conditions['params']);
        $data = $stmt->fetchAll(PDO::FETCH_ASSOC);

        /* ----------------------------------------------------------------
         * 3. PhpSpreadsheet 객체 생성 및 시트 초기화
         * ---------------------------------------------------------------- */
        // Create Excel spreadsheet
        $spreadsheet = new Spreadsheet();
        $sheet = $spreadsheet->getActiveSheet();
        $sheet->setTitle('OEE Row Data Log'); // 시트 탭 이름 설정

        /* ----------------------------------------------------------------
         * 헤더 행 정의 (data_oee_rows 테이블 컬럼 순서와 동일)
         *  - 총 33개 컬럼 (update_date 제외, work_hour 포함)
         * ---------------------------------------------------------------- */
        // Define all headers based on data_oee_rows table columns
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
         *    - 테두리 : 얇은 회색 선(#CCCCCC)
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
         *    - 각 행의 데이터를 정해진 컬럼 순서대로 배열로 전달
         *    - null 안전 연산자(??)로 값이 없을 경우 빈 문자열 처리
         *    - 짝수 행은 연회색(#F9F9F9) 배경으로 가독성 향상 (줄무늬 패턴)
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
                // $row['design_no'] ?? '',   // 비활성화된 design_no 컬럼 (추후 복원 가능)
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

            $rowNum++; // 다음 행으로 이동
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
            /* A2 ~ 마지막 데이터 행까지 스타일 일괄 적용 */
            $sheet->getStyle('A2:' . $sheet->getHighestColumn() . ($rowNum - 1))->applyFromArray($dataStyle);
        }

        /* ----------------------------------------------------------------
         * 7. 모든 컬럼 너비 자동 조정 (셀 내용 길이에 맞게 자동 계산)
         * ---------------------------------------------------------------- */
        // Auto-size columns
        foreach ($sheet->getColumnIterator() as $column) {
            $sheet->getColumnDimension($column->getColumnIndex())->setAutoSize(true);
        }

        /* ----------------------------------------------------------------
         * 8. 다운로드 파일명 생성
         *    - 기본 접두사 : OEE_Row_Data_Log
         *    - start_date / end_date GET 파라미터가 있으면 파일명에 추가
         *    - 타임스탬프(YmdHis) 후미 추가로 중복 방지
         *    예) OEE_Row_Data_Log_2025-01-01_2025-01-31_20250131120000.xlsx
         * ---------------------------------------------------------------- */
        // Generate filename with timestamp and filters
        $filename_parts = ['OEE_Row_Data_Log'];
        if (!empty($_GET['start_date'])) {
            $filename_parts[] = $_GET['start_date']; // 시작 날짜 파일명에 추가
        }
        if (!empty($_GET['end_date'])) {
            $filename_parts[] = $_GET['end_date']; // 종료 날짜 파일명에 추가
        }
        /* 파일명 조각들을 '_'로 연결하고 타임스탬프 추가 */
        $filename = implode('_', $filename_parts) . '_' . date('YmdHis') . '.xlsx';

        /* ----------------------------------------------------------------
         * Excel 파일 다운로드 HTTP 헤더 설정
         *  - Content-Type : xlsx MIME 타입
         *  - Content-Disposition : 첨부 파일로 다운로드 지정 및 파일명 설정
         *  - Cache-Control : 브라우저 캐시 비활성화 (max-age=0)
         * ---------------------------------------------------------------- */
        // Set headers for download
        header('Content-Type: application/vnd.openxmlformats-officedocument.spreadsheetml.sheet');
        header('Content-Disposition: attachment;filename="' . $filename . '"');
        header('Cache-Control: max-age=0');

        /* php://output 스트림으로 직접 저장 (임시 파일 생성 없이 메모리 절약) */
        // Write to output
        $writer = new Xlsx($spreadsheet);
        $writer->save('php://output');
    } catch (PDOException $e) {
        /* DB 오류 발생 시 HTTP 500 응답 및 에러 로그 기록 */
        http_response_code(500);
        error_log("Excel export error: " . $e->getMessage());
        echo json_encode(['success' => false, 'message' => 'Database error: ' . $e->getMessage()]);
    } catch (Exception $e) {
        /* PhpSpreadsheet 또는 기타 오류 발생 시 HTTP 500 응답 */
        http_response_code(500);
        error_log("Excel export error: " . $e->getMessage());
        echo json_encode(['success' => false, 'message' => 'Export error: ' . $e->getMessage()]);
    }
}

/* ================================================================
 * 진입점 — export_common.php에서 초기화된 $pdo를 사용해 내보내기 실행
 *  - $pdo가 null(DB 연결 실패)이면 즉시 예외를 발생시켜 오류 응답 반환
 * ================================================================ */
// Execute export
try {
    if (!$pdo) {
        throw new Exception("Database connection failed");
    }

    exportOeeRowDataLog($pdo); // OEE Row 데이터 Excel 내보내기 실행
} catch (Exception $e) {
    /* 치명적 오류 시 HTTP 500 응답 및 에러 로그 기록 */
    http_response_code(500);
    error_log("Fatal error in OEE row data log export: " . $e->getMessage());
    echo json_encode(['success' => false, 'message' => 'Server Error: ' . $e->getMessage()]);
}
