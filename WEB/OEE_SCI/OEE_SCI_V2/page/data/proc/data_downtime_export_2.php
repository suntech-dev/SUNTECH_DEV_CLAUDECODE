<?php

/**
 * ============================================================
 * 파일명: data_downtime_export_2.php
 * 목  적: 다운타임(Downtime) 데이터를 Excel(.xlsx) 파일로 내보내는 API
 *
 * 주요 기능:
 *  - 공장 → 라인 → 기계 3단계 필터링 지원
 *  - 날짜 범위(start_date / end_date) 및 교대(shift) 필터링
 *  - PhpSpreadsheet 라이브러리를 이용한 Excel 파일 생성
 *  - SAP Fiori 스타일 헤더 색상 및 교대 행 배경 적용
 *  - 상태에 따른 duration 계산:
 *      Warning 상태 → 현재까지 경과 시간
 *      Completed 상태 → duration_his 컬럼 값 (또는 'in progress')
 *
 * GET 파라미터:
 *  factory_filter  : 공장 idx
 *  line_filter     : 라인 idx
 *  machine_filter  : 기계 idx
 *  shift_filter    : 교대 idx
 *  start_date      : 조회 시작 날짜 (YYYY-MM-DD)
 *  end_date        : 조회 종료 날짜 (YYYY-MM-DD)
 * ============================================================
 */

// 공통 초기화 및 필터 빌더 로드 (DB 연결, PhpSpreadsheet autoload 포함)
require_once __DIR__ . '/export_common.php';

// PhpSpreadsheet 네임스페이스 import
use PhpOffice\PhpSpreadsheet\Spreadsheet;
use PhpOffice\PhpSpreadsheet\Writer\Xlsx;
use PhpOffice\PhpSpreadsheet\Style\Fill;
use PhpOffice\PhpSpreadsheet\Style\Alignment;
use PhpOffice\PhpSpreadsheet\Style\Border;
use PhpOffice\PhpSpreadsheet\Cell\DataType;

/**
 * 다운타임(Downtime) 데이터를 Excel로 내보내는 메인 함수
 *
 * @param PDO $pdo PDO 데이터베이스 연결 객체
 */
function exportDowntimes(PDO $pdo)
{
    try {
        /*
         * 공통 필터 파라미터 빌더 호출:
         *  - 테이블 alias: 'dd' (data_downtime)
         *  - 날짜 컬럼: 'reg_date' (등록일시 기준)
         *  - datetime 보정: true (00:00:00 ~ 23:59:59 범위)
         *  - 기본 조건: 날짜 미지정 시 최근 2일 데이터
         */
        $query_conditions = buildExportFilterParams('dd', 'reg_date', true, 'dd.reg_date >= DATE_SUB(NOW(), INTERVAL 2 DAY)');

        /*
         * 다운타임 데이터 조회 SQL
         * - data_downtime(dd): 다운타임 발생 이력 테이블 (메인)
         * - info_factory(f): 공장명 조인
         * - info_line(l): 라인명 조인
         * - duration 컬럼:
         *    Warning → 현재까지 경과 시간 (h/m/s 포맷)
         *    duration_sec=0 → 'in progress' (아직 완료되지 않음)
         *    Completed → duration_his 컬럼 저장값
         */
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

        // PDO Prepared Statement 실행 (SQL 인젝션 방지)
        $stmt = $pdo->prepare($sql);
        $stmt->execute($query_conditions['params']);
        // 전체 결과를 연관 배열로 가져오기
        $data = $stmt->fetchAll(PDO::FETCH_ASSOC);

        // PhpSpreadsheet 객체 생성 — 새 Excel 워크북 초기화
        $spreadsheet = new Spreadsheet();
        // 첫 번째 시트(기본 활성 시트) 가져오기
        $sheet = $spreadsheet->getActiveSheet();
        // 시트 탭 이름 설정
        $sheet->setTitle('Downtime Data');

        /*
         * 헤더 컬럼 정의
         * 순서: NO, ID, Machine No, Factory/Line, Shift, Downtime Type,
         *        Status, Occurrence Time, Resolution Time, Duration, Work Date
         */
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

        // 헤더 배열을 A1 셀부터 시트에 기록
        $sheet->fromArray($headers, NULL, 'A1');

        /*
         * 헤더 스타일 정의:
         *  - 폰트: 굵게, 흰색
         *  - 배경: SAP Fiori Blue (#0070F2) 단색 채우기
         *  - 정렬: 가로/세로 중앙
         *  - 테두리: 연한 회색 얇은 테두리
         */
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
        // A1부터 마지막 헤더 열까지 스타일 적용
        $sheet->getStyle('A1:' . $sheet->getHighestColumn() . '1')->applyFromArray($headerStyle);

        // 헤더 행 높이 25px로 설정 (가독성 향상)
        $sheet->getRowDimension(1)->setRowHeight(25);

        // 데이터 행 작성 시작 — 2행부터 (1행은 헤더)
        $rowNum = 2;
        foreach ($data as $index => $row) {
            // 교대(Shift) 표시 포맷: shift_idx가 없으면 '-', 있으면 'Shift N' 형식
            $shiftDisplay = '-';
            if (!empty($row['shift_idx'])) {
                $shiftDisplay = 'Shift ' . $row['shift_idx'];
            }

            // 상태(Status) 표시 포맷: Warning/Completed 이모지 접두사 추가
            $statusDisplay = '-';
            if ($row['status'] === 'Warning') {
                $statusDisplay = '⚠️ Warning';
            } elseif ($row['status'] === 'Completed') {
                $statusDisplay = '✅ Completed';
            } else {
                $statusDisplay = $row['status'];
            }

            // 공장/라인 통합 표시 포맷: "공장명 / 라인명" (null이면 '-')
            $factoryLineDisplay = ($row['factory_name'] ?? '-') . ' / ' . ($row['line_name'] ?? '-');

            /*
             * 행 데이터 배열을 현재 rowNum 위치에 기록
             * 순서: NO(순번), ID(DB idx), Machine No, Factory/Line, Shift,
             *        Downtime Type, Status, 발생시각, 해결시각, Duration, Work Date
             */
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

            // 짝수 행에 연한 회색(#F9F9F9) 교대 배경색 적용 (가독성 향상)
            if ($rowNum % 2 == 0) {
                $sheet->getStyle('A' . $rowNum . ':' . $sheet->getHighestColumn() . $rowNum)
                    ->getFill()
                    ->setFillType(Fill::FILL_SOLID)
                    ->getStartColor()->setRGB('F9F9F9');
            }

            $rowNum++;
        }

        // 데이터 영역 전체에 테두리 및 세로 중앙 정렬 스타일 적용
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
            // A2부터 마지막 데이터 행까지 스타일 적용
            $sheet->getStyle('A2:' . $sheet->getHighestColumn() . ($rowNum - 1))->applyFromArray($dataStyle);
        }

        // 모든 컬럼 너비를 내용에 맞게 자동 조정
        foreach ($sheet->getColumnIterator() as $column) {
            $sheet->getColumnDimension($column->getColumnIndex())->setAutoSize(true);
        }

        /*
         * 필터 정보 문자열 생성 (시트 노트용 선택 사항)
         * - 내보내기 일시, 적용된 필터 조건(공장, 라인, 기계, 교대, 날짜 범위) 기록
         */
        $filterInfo = "Export Date: " . date('Y-m-d H:i:s') . "\n";
        $filterInfo .= "Filters Applied:\n";
        $filterInfo .= "- Factory: " . ($_GET['factory_filter'] ?? 'All') . "\n";
        $filterInfo .= "- Line: " . ($_GET['line_filter'] ?? 'All') . "\n";
        $filterInfo .= "- Machine: " . ($_GET['machine_filter'] ?? 'All') . "\n";
        $filterInfo .= "- Shift: " . ($_GET['shift_filter'] ?? 'All') . "\n";
        $filterInfo .= "- Date Range: " .
            ($_GET['start_date'] ?? 'N/A') . ' ~ ' . ($_GET['end_date'] ?? 'N/A');

        /*
         * 다운로드 파일명 생성:
         *  - 기본: 'downtime_data'
         *  - 날짜 범위가 지정된 경우 start_date, end_date 추가
         *  - 타임스탬프(YmdHis) 추가로 중복 방지
         *  예) downtime_data_2026-01-01_2026-01-31_20260101120000.xlsx
         */
        $filename_parts = ['downtime_data'];
        if (!empty($_GET['start_date'])) {
            $filename_parts[] = $_GET['start_date'];
        }
        if (!empty($_GET['end_date'])) {
            $filename_parts[] = $_GET['end_date'];
        }
        $filename = implode('_', $filename_parts) . '_' . date('YmdHis') . '.xlsx';

        /*
         * Excel 파일 다운로드를 위한 HTTP 응답 헤더 설정:
         *  - Content-Type: OOXML Excel 형식
         *  - Content-Disposition: attachment로 강제 다운로드
         *  - Cache-Control: 캐시 비활성화
         */
        header('Content-Type: application/vnd.openxmlformats-officedocument.spreadsheetml.sheet');
        header('Content-Disposition: attachment;filename="' . $filename . '"');
        header('Cache-Control: max-age=0');

        // php://output으로 직접 스트리밍 출력 (파일 저장 없이 브라우저로 전송)
        $writer = new Xlsx($spreadsheet);
        $writer->save('php://output');
    } catch (PDOException $e) {
        // DB 오류: 500 에러 코드 반환 및 에러 로그 기록
        http_response_code(500);
        error_log("Excel export error: " . $e->getMessage());
        echo json_encode(['success' => false, 'message' => 'Database error: ' . $e->getMessage()]);
    } catch (Exception $e) {
        // 일반 오류: 500 에러 코드 반환 및 에러 로그 기록
        http_response_code(500);
        error_log("Excel export error: " . $e->getMessage());
        echo json_encode(['success' => false, 'message' => 'Export error: ' . $e->getMessage()]);
    }
}

// ── 메인 실행부 ──────────────────────────────────────────────────────────────
// DB 연결 확인 후 exportDowntimes() 호출
try {
    if (!$pdo) {
        throw new Exception("Database connection failed");
    }

    exportDowntimes($pdo);
} catch (Exception $e) {
    // 치명적 오류: 500 에러 반환
    http_response_code(500);
    error_log("Fatal error in downtime export: " . $e->getMessage());
    echo json_encode(['success' => false, 'message' => 'Server Error: ' . $e->getMessage()]);
}
