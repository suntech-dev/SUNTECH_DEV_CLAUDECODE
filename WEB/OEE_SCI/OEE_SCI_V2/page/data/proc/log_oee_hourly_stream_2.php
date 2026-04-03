<?php

/**
 * ============================================================
 * 파일명  : log_oee_hourly_stream_2.php
 * 목  적  : OEE 시간별(Hourly) 데이터 로그를 SSE(Server-Sent Events)로 실시간 스트리밍
 *
 * 주요 기능:
 *  - data_oee_rows_hourly 테이블의 시간 단위 OEE 데이터를 5초마다 실시간 전송
 *  - 공장(Factory) → 라인(Line) → 기계(Machine) 계층 필터 지원
 *  - 날짜 범위 / 교대(Shift) 필터 지원 (기본: 최근 7일)
 *  - 데이터 변경 감지 (MD5 해시 비교) → 변경 없으면 heartbeat만 전송
 *  - 통계 요약 (avg OEE / Availability / Performance / Quality 등) 동시 전송
 *  - 현재 교대 OEE 및 전일 OEE 비교 데이터 포함
 *  - 최대 실행 시간 3600초(1시간) → timeout 이벤트 후 루프 종료
 *
 * log_oee_row_stream_2.php 와의 차이점:
 *  - 대상 테이블 : data_oee_rows_hourly (시간 단위 집계 데이터)
 *  - 테이블 별칭  : 'doh' (data_oee_hourly)
 *  - parseFilterParams 호출 시 'doh' 별칭 사용
 *  - 정렬 기준에 work_hour DESC 포함
 *
 * SSE 이벤트 타입:
 *  - connected    : 연결 성공 초기 이벤트 (필터 정보 포함)
 *  - oee_data     : 데이터 변경 감지 시 전체 데이터 전송
 *  - heartbeat    : 데이터 변경 없음 상태 유지 신호
 *  - timeout      : 최대 실행 시간 초과
 *  - error        : 쿼리 오류
 *  - disconnected : 스트리밍 종료 (finally 블록)
 *
 * 의존 파일:
 *  - lib/db.php               : PDO $pdo 초기화
 *  - lib/api_helper.lib.php   : ApiHelper 클래스
 *  - lib/worktime.lib.php     : Worktime 클래스 (교대 시간 계산)
 *  - lib/get_shift.lib.php    : findCurrentShift() 함수
 *  - lib/stream_helper.lib.php: parseFilterParams(), sendSSEData() 함수
 * ============================================================
 */


// Load required libraries
require_once(__DIR__ . '/../../../lib/db.php');
require_once(__DIR__ . '/../../../lib/api_helper.lib.php');
require_once(__DIR__ . '/../../../lib/worktime.lib.php');
require_once(__DIR__ . '/../../../lib/get_shift.lib.php');
require_once(__DIR__ . '/../../../lib/stream_helper.lib.php');

/* ----------------------------------------------------------------
 * SSE 응답 헤더 설정
 *  - Content-Type: text/event-stream  → SSE 프로토콜 명시
 *  - Cache-Control: no-cache          → 브라우저/프록시 캐시 완전 비활성화
 *  - Connection: keep-alive           → 연결 유지 (지속적 스트리밍)
 *  - Access-Control-Allow-Origin: *   → CORS 허용 (크로스 도메인 접근 허용)
 * ---------------------------------------------------------------- */
// SSE headers
header('Content-Type: text/event-stream');
header('Cache-Control: no-cache');
header('Connection: keep-alive');
header('Access-Control-Allow-Origin: *');
header('Access-Control-Allow-Methods: GET');
header('Access-Control-Allow-Headers: Cache-Control');

/* 출력 버퍼링 비활성화 — SSE는 즉시 플러시되어야 하므로 버퍼 제거 */
// Disable output buffering
if (ob_get_level()) ob_end_clean();

/* ApiHelper 인스턴스 초기화 (공통 API 유틸리티) */
// Initialize API helper
$apiHelper = new ApiHelper($pdo);

// parseFilterParams(), sendSSEData() → stream_helper.lib.php

/**
 * getOeeHourlyDataLog — data_oee_rows_hourly 테이블에서 시간별 OEE 데이터 조회
 *
 * @param PDO    $pdo       데이터베이스 연결 객체
 * @param string $where_sql WHERE 절 SQL 문자열 (parseFilterParams가 생성)
 * @param array  $params    PDO 바인딩 파라미터 배열
 * @param int    $limit     조회 최대 건수 (기본 1000)
 * @return array            조회된 시간별 OEE 데이터 연관 배열 목록
 *                          (오류 시 빈 배열 반환)
 */
function getOeeHourlyDataLog($pdo, $where_sql, $params, $limit = 1000)
{
    try {
        /* data_oee_rows_hourly 테이블의 모든 컬럼을 조회 */
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
      {$where_sql}
      ORDER BY doh.work_date DESC, doh.work_hour DESC, doh.update_date DESC, doh.idx DESC
      LIMIT " . (int)$limit . "
    ";

        $stmt = $pdo->prepare($sql);
        $stmt->execute($params);

        $result = $stmt->fetchAll(PDO::FETCH_ASSOC);

        /* shift_idx 숫자값을 "Shift N" 형식의 표시 문자열로 변환
         * 예) 1 → "Shift 1", null 또는 빈 문자열은 그대로 유지 */
        // shift_idx를 "Shift 1" 형식으로 변환
        foreach ($result as &$row) {
            if (isset($row['shift_idx']) && $row['shift_idx'] !== null && $row['shift_idx'] !== '') {
                $row['shift_idx'] = 'Shift ' . $row['shift_idx'];
            }
        }

        return $result;
    } catch (PDOException $e) {
        /* DB 쿼리 오류 시 로그 기록 후 빈 배열 반환 (스트리밍 중단 방지) */
        error_log("OEE hourly data log query error: " . $e->getMessage());
        return [];
    }
}

/**
 * getOeeHourlyStats — data_oee_rows_hourly 테이블의 시간별 통계 요약 데이터 조회
 *
 * 반환 지표:
 *  - 전체 건수, 평균 OEE / Availability / Performance / Quality
 *  - 최대·최소 OEE, 활성 기계 수
 *  - OEE 등급별 건수 (excellent≥85 / good≥70 / fair≥50 / poor<50)
 *  - 오늘 평균 OEE, 오늘 건수
 *  - 생산량 합계 (실제/이론/양품/불량)
 *  - 시간 합계 (계획/실행/비가동)
 *  - PHP 계산 파생 지표 (overall_performance_rate, overall_quality_rate, overall_availability_rate)
 *  - 현재 교대 OEE / 전일 OEE
 *  - stat-card 표시용 필드 별칭 (overall_oee, availability, performance, quality)
 *
 * @param PDO    $pdo       데이터베이스 연결 객체
 * @param string $where_sql WHERE 절 SQL 문자열
 * @param array  $params    PDO 바인딩 파라미터 배열
 * @return array            통계 연관 배열 (오류 시 기본값 0으로 채운 배열 반환)
 */
function getOeeHourlyStats($pdo, $where_sql, $params)
{
    try {
        /* ----------------------------------------------------------------
         * 통계 집계 SQL
         *  - avg_overall_oee : (가용률 × 성능률 × 품질률) / 100 공식으로 계산
         *  - avg_performance : 실제생산 / 이론생산 × 100 (%)
         *  - OEE 등급 분류 : CASE WHEN 으로 4구간 카운트
         *  - today_avg_oee  : CURDATE() 당일 데이터만 조건부 평균 (NULL 제외)
         *  - NULLIF(SUM(...), 0) : 분모 0 방지 (0 나누기 오류 방지)
         * ---------------------------------------------------------------- */
        $sql = "
      SELECT
        COUNT(*) as total_count,
        ROUND((AVG(doh.availabilty_rate) * (SUM(doh.actual_output) / NULLIF(SUM(doh.theoritical_output), 0)) * AVG(doh.quality_rate)) / 100, 2) as avg_overall_oee,
        ROUND(AVG(doh.availabilty_rate), 2) as avg_availability,
        ROUND((SUM(doh.actual_output) / NULLIF(SUM(doh.theoritical_output), 0)) * 100, 2) as avg_performance,
        ROUND(AVG(doh.quality_rate), 2) as avg_quality,
        MAX(doh.oee) as max_oee,
        MIN(doh.oee) as min_oee,
        COUNT(DISTINCT doh.machine_idx) as active_machines,
        SUM(CASE WHEN doh.oee >= 85 THEN 1 ELSE 0 END) as excellent_count,
        SUM(CASE WHEN doh.oee >= 70 AND doh.oee < 85 THEN 1 ELSE 0 END) as good_count,
        SUM(CASE WHEN doh.oee >= 50 AND doh.oee < 70 THEN 1 ELSE 0 END) as fair_count,
        SUM(CASE WHEN doh.oee < 50 THEN 1 ELSE 0 END) as poor_count,
        ROUND(AVG(CASE WHEN doh.work_date = CURDATE() THEN doh.oee ELSE NULL END), 2) as today_avg_oee,
        COUNT(CASE WHEN doh.work_date = CURDATE() THEN 1 ELSE NULL END) as today_count,
        SUM(doh.actual_output) as total_actual_output,
        SUM(doh.theoritical_output) as total_theoretical_output,
        SUM(doh.actual_a_grade) as total_good_products,
        SUM(doh.defective) as total_defective_count,
        SUM(doh.planned_work_time) as total_planned_time,
        SUM(doh.runtime) as total_runtime,
        SUM(doh.downtime) as total_downtime
      FROM data_oee_rows_hourly doh
      {$where_sql}
    ";

        $stmt = $pdo->prepare($sql);
        $stmt->execute($params);

        $stats = $stmt->fetch(PDO::FETCH_ASSOC);

        /* 현재 교대 OEE 계산 (GET 파라미터 shift_filter 우선 적용) */
        $selected_shift = $_GET['shift_filter'] ?? null;
        /* 대상 날짜 : start_date → end_date → 오늘 순으로 결정 */
        $target_date = $_GET['start_date'] ?? $_GET['end_date'] ?? null;
        $stats['current_shift_oee'] = getCurrentShiftOeeAvg($pdo, $selected_shift, $target_date);

        /* 가장 최근 day의 OEE 계산 (오늘 제외, 휴일 등으로 데이터 없으면 그 이전 날짜) */
        $stats['previous_day_oee'] = getPreviousDayOeeAvg($pdo, $where_sql, $params);

        /* PHP로 파생 지표 계산 (DB 서브쿼리 추가 없이 처리하여 성능 향상) */
        if ($stats['total_theoretical_output'] > 0) {
            /* 종합 성능률 = 실제생산 / 이론생산 × 100 */
            $stats['overall_performance_rate'] = round(($stats['total_actual_output'] / $stats['total_theoretical_output']) * 100, 2);
        } else {
            $stats['overall_performance_rate'] = 0;
        }

        if ($stats['total_actual_output'] > 0) {
            /* 종합 품질률 = 양품수 / 실제생산 × 100 */
            $stats['overall_quality_rate'] = round(($stats['total_good_products'] / $stats['total_actual_output']) * 100, 2);
        } else {
            $stats['overall_quality_rate'] = 0;
        }

        if ($stats['total_planned_time'] > 0) {
            /* 종합 가용률 = 실제 가동 시간 / 계획 작업 시간 × 100 */
            $stats['overall_availability_rate'] = round(($stats['total_runtime'] / $stats['total_planned_time']) * 100, 2);
        } else {
            $stats['overall_availability_rate'] = 0;
        }

        /* stat-card용 필드 매핑 (프론트엔드 JS에서 통일된 키 이름으로 접근) */
        // stat-card용 필드 매핑
        $stats['overall_oee'] = $stats['avg_overall_oee'];
        $stats['availability'] = $stats['avg_availability'];
        $stats['performance'] = $stats['avg_performance'];
        $stats['quality'] = $stats['avg_quality'];

        return $stats;
    } catch (PDOException $e) {
        /* DB 오류 시 모든 지표를 0으로 초기화한 기본 배열 반환 */
        error_log("OEE hourly statistics query error: " . $e->getMessage());
        return [
            'total_count' => 0,
            'avg_overall_oee' => 0,
            'avg_availability' => 0,
            'avg_performance' => 0,
            'avg_quality' => 0,
            'max_oee' => 0,
            'min_oee' => 0,
            'active_machines' => 0,
            'excellent_count' => 0,
            'good_count' => 0,
            'fair_count' => 0,
            'poor_count' => 0,
            'today_avg_oee' => 0,
            'today_count' => 0,
            'current_shift_oee' => 0,
            'previous_day_oee' => 0,
            'total_actual_output' => 0,
            'total_theoretical_output' => 0,
            'total_good_products' => 0,
            'total_defective_count' => 0,
            'total_planned_time' => 0,
            'total_runtime' => 0,
            'total_downtime' => 0,
            'overall_performance_rate' => 0,
            'overall_quality_rate' => 0,
            'overall_availability_rate' => 0,
            // stat-card용 필드 매핑 (기본값)
            'overall_oee' => 0,
            'availability' => 0,
            'performance' => 0,
            'quality' => 0
        ];
    }
}

/**
 * startStreaming — SSE 메인 루프 실행
 *
 * 동작 방식:
 *  1. GET 파라미터로 필터 조건 파싱 (parseFilterParams, 'doh' 별칭)
 *  2. 연결 성공 이벤트(connected) 전송
 *  3. 5초마다 데이터 조회 → MD5 해시 비교
 *     - 변경 있음 : oee_data 이벤트 전송 (전체 데이터 + 통계)
 *     - 변경 없음 : heartbeat 이벤트 전송 (상태 유지)
 *  4. 클라이언트 연결 종료 또는 타임아웃 시 루프 종료
 *
 * @param PDO $pdo  데이터베이스 연결 객체
 * @return void
 */
function startStreaming($pdo)
{
    /* 이전 데이터 해시값 초기화 (첫 번째 루프에서 항상 데이터 전송 보장) */
    $lastDataHash = '';
    $startTime = time();     // 스트리밍 시작 시간 기록
    $maxRunTime = 3600;      // 최대 실행 시간 : 3600초 (1시간)

    /* parseFilterParams('doh', 'work_date', true, '7 DAY')
     *  - 테이블 별칭 'doh' 기준 WHERE 절 생성
     *  - work_date 컬럼 기준 날짜 필터
     *  - 기본 날짜 범위 : 최근 7일 */
    $filterConfig = parseFilterParams('doh', 'work_date', true, '7 DAY');
    /* GET 파라미터 limit으로 조회 건수 제한 (미입력 시 1000건) */
    $limit = !empty($_GET['limit']) ? (int)$_GET['limit'] : 1000;

    /* 클라이언트에 연결 성공 이벤트 전송 (현재 적용된 필터 정보 포함) */
    sendSSEData('connected', [
        'status' => 'connected',
        'message' => 'OEE hourly data log streaming started.',
        'timestamp' => date('Y-m-d H:i:s'),
        'filters' => [
            'factory_filter' => $_GET['factory_filter'] ?? null,
            'line_filter' => $_GET['line_filter'] ?? null,
            'machine_filter' => $_GET['machine_filter'] ?? null,
            'shift_filter' => $_GET['shift_filter'] ?? null,
            'start_date' => $_GET['start_date'] ?? null,
            'end_date' => $_GET['end_date'] ?? null,
            'limit' => $limit
        ]
    ]);

    /* ----------------------------------------------------------------
     * 무한 루프 — 5초마다 데이터 조회 및 SSE 이벤트 전송
     * ---------------------------------------------------------------- */
    while (true) {
        /* 최대 실행 시간 초과 시 timeout 이벤트 전송 후 루프 종료 */
        if (time() - $startTime > $maxRunTime) {
            sendSSEData('timeout', [
                'status' => 'timeout',
                'message' => 'Maximum execution time reached. Please reconnect.'
            ]);
            break;
        }

        /* 클라이언트 연결 종료(브라우저 탭 닫기 등) 시 루프 즉시 종료 */
        if (connection_aborted()) {
            error_log("Client connection aborted.");
            break;
        }

        try {
            /* 시간별 OEE 데이터 조회 */
            $oeeDataLog = getOeeHourlyDataLog($pdo, $filterConfig['where_sql'], $filterConfig['params'], $limit);

            /* Stats 조회를 try-catch로 감싸서 실패해도 데이터는 전송되도록 함 */
            try {
                $stats = getOeeHourlyStats($pdo, $filterConfig['where_sql'], $filterConfig['params']);
            } catch (Exception $statsError) {
                /* 통계 조회 실패 시 기본값 0으로 채운 배열 사용 */
                error_log("Stats query error: " . $statsError->getMessage());
                $stats = [
                    'total_count' => 0,
                    'avg_overall_oee' => 0,
                    'avg_availability' => 0,
                    'avg_performance' => 0,
                    'avg_quality' => 0,
                    'max_oee' => 0,
                    'min_oee' => 0,
                    'active_machines' => 0,
                    'current_shift_oee' => 0,
                    'previous_day_oee' => 0,
                    'overall_oee' => 0,
                    'availability' => 0,
                    'performance' => 0,
                    'quality' => 0
                ];
            }

            /* ----------------------------------------------------------------
             * 데이터 변경 감지 해시 생성
             *  - 행 수, 인덱스 목록, idx_oee 조합 문자열, 핵심 통계 값을 포함
             *  - 이 데이터가 이전 루프와 동일하면 heartbeat만 전송 (대역폭 절약)
             * ---------------------------------------------------------------- */
            $hashData = [
                'oee_count' => count($oeeDataLog),
                'oee_ids' => array_column($oeeDataLog, 'idx'),
                'oee_values' => array_map(function ($item) {
                    return $item['idx'] . '_' . ($item['oee'] ?? '0');
                }, $oeeDataLog),
                'stats_core' => [
                    'avg_overall_oee' => $stats['avg_overall_oee'] ?? 0,
                    'avg_availability' => $stats['avg_availability'] ?? 0,
                    'avg_performance' => $stats['avg_performance'] ?? 0,
                    'avg_quality' => $stats['avg_quality'] ?? 0,
                    'active_machines' => $stats['active_machines'] ?? 0
                ]
            ];

            /* MD5 해시로 이전 데이터와 비교 */
            $currentDataHash = md5(serialize($hashData));

            if ($currentDataHash !== $lastDataHash) {
                /* 데이터 변경 감지 → 전체 데이터 및 통계를 oee_data 이벤트로 전송 */
                $responseData = [
                    'timestamp' => date('Y-m-d H:i:s'),
                    'stats' => $stats,
                    'oee_data' => $oeeDataLog,
                    'data_count' => count($oeeDataLog),
                    'has_changes' => true
                ];

                sendSSEData('oee_data', $responseData);
                $lastDataHash = $currentDataHash; // 해시값 갱신
            } else {
                /* 변경 없음 → heartbeat 이벤트로 연결 상태만 유지 */
                sendSSEData('heartbeat', [
                    'timestamp' => date('Y-m-d H:i:s'),
                    'status' => 'no_changes',
                    'active_machines' => $stats['active_machines'] ?? 0
                ]);
            }
        } catch (Exception $e) {
            /* 루프 내 예외 발생 시 에러 이벤트 전송 (스트리밍 자체는 유지) */
            error_log("Streaming error: " . $e->getMessage());
            error_log("Stack trace: " . $e->getTraceAsString());

            sendSSEData('error', [
                'status' => 'error',
                'message' => 'Data query error occurred: ' . $e->getMessage(),
                'timestamp' => date('Y-m-d H:i:s')
            ]);
        }

        /* 5초 대기 후 다음 루프 실행 */
        sleep(5);
    }
}

/**
 * getPreviousDayOeeAvg — 전일(오늘 제외) 평균 OEE 조회 (시간별 테이블 기준)
 *
 * 현재 필터 조건을 유지하면서 CURDATE() 이전 날짜 중
 * 가장 최근 작업일 1개의 평균 OEE를 반환.
 * WHERE 절 존재 여부에 따라 AND/WHERE 키워드를 동적으로 결정.
 *
 * @param PDO    $pdo       데이터베이스 연결 객체
 * @param string $where_sql 현재 적용된 WHERE 절 SQL 문자열
 * @param array  $params    PDO 바인딩 파라미터 배열
 * @return float            전일 평균 OEE (데이터 없음 시 0 반환)
 */
function getPreviousDayOeeAvg($pdo, $where_sql, $params)
{
    try {
        /* 오늘 제외하고 가장 최근 날짜의 평균 OEE 조회 */
        $sql = "
      SELECT
        ROUND((AVG(doh.availabilty_rate) * (SUM(doh.actual_output) / NULLIF(SUM(doh.theoritical_output), 0)) * AVG(doh.quality_rate)) / 100, 2) as previous_day_oee,
        doh.work_date
      FROM data_oee_rows_hourly doh
      {$where_sql}
      " . (strpos($where_sql, 'WHERE') !== false ? 'AND' : 'WHERE') . " doh.work_date < CURDATE()
      GROUP BY doh.work_date
      ORDER BY doh.work_date DESC
      LIMIT 1
    ";

        $stmt = $pdo->prepare($sql);
        $stmt->execute($params);

        $result = $stmt->fetch(PDO::FETCH_ASSOC);

        /* 결과가 없거나 null이면 0 반환 */
        return (float)($result['previous_day_oee'] ?? 0);
    } catch (PDOException $e) {
        error_log("Previous day OEE query error: " . $e->getMessage());
        return 0;
    }
}

/**
 * getCurrentShiftOeeAvg — 현재(또는 선택된) 교대의 시간별 평균 OEE 조회
 *
 * 처리 흐름:
 *  1. selected_shift_idx가 있으면 해당 교대의 시간 범위 조회
 *  2. 없으면 findCurrentShift()로 현재 시각 기준 교대 자동 감지
 *  3. 교대의 work_date 및 shift_idx 기준으로 data_oee_rows_hourly 테이블 조회
 *  4. 야간 교대(종료 시간 < 시작 시간)는 +1일 처리
 *
 * @param PDO      $pdo                데이터베이스 연결 객체
 * @param int|null $selected_shift_idx 선택된 교대 번호 (null이면 현재 교대 자동 감지)
 * @param string|null $target_date     기준 날짜 (null이면 오늘)
 * @return float                       해당 교대 평균 OEE (0~100, 오류 시 0 반환)
 */
function getCurrentShiftOeeAvg($pdo, $selected_shift_idx = null, $target_date = null)
{
    try {
        $current_datetime = date('Y-m-d H:i:s'); // 현재 일시
        $target_date = $target_date ?: date('Y-m-d'); // 기준 날짜 (없으면 오늘)

        $worktime = new Worktime($pdo); // Worktime 라이브러리 인스턴스

        /* 공장/라인 인덱스는 필터 없이 전체 교대 정보 조회 (빈 문자열 = 전체) */
        $factory_idx = '';
        $line_idx = '';

        if (!empty($selected_shift_idx)) {
            /* 교대가 선택된 경우: 해당 교대의 시간 범위 계산 */
            $day_shifts = $worktime->getDayShift($target_date, $factory_idx, $line_idx);

            /* 교대 정보가 없으면 0 반환 */
            if (!$day_shifts || !isset($day_shifts['shift'][$selected_shift_idx])) {
                return 0;
            }

            $shift = $day_shifts['shift'][$selected_shift_idx];

            /* 교대 시작/종료 시각 문자열 생성 */
            $work_stime_str = $target_date . ' ' . $shift['available_stime'] . ':00';
            $work_etime_str = $target_date . ' ' . $shift['available_etime'] . ':00';

            /* 연장 근무(over_time)가 있으면 종료 시각에 분 단위 추가 */
            if ($shift['over_time']) {
                $work_etime_str = date('Y-m-d H:i:s', strtotime($work_etime_str . ' +' . $shift['over_time'] . ' minutes'));
            }

            /* 야간 교대 처리: 종료 시각이 시작 시각보다 이르면 다음 날로 조정 */
            if ($work_etime_str <= $work_stime_str) {
                $work_etime_str = date('Y-m-d H:i:s', strtotime($work_etime_str . ' +1 day'));
            }

            $shift_start = $work_stime_str;
            $shift_end = $work_etime_str;
        } else {
            /* 교대 미선택 시: 현재 시각 기준으로 진행 중인 교대 자동 감지 */
            $current_shift_info = findCurrentShift($pdo, $worktime, $factory_idx, $line_idx, $current_datetime);

            /* 현재 교대 정보가 없으면 0 반환 */
            if (!$current_shift_info) {
                return 0;
            }

            $shift_start = $current_shift_info['work_stime'];
            $shift_end = $current_shift_info['work_etime'];
        }

        /* 해당 교대의 작업일 및 교대 번호로 시간별 OEE 평균 조회 */
        $sql = "
      SELECT ROUND((AVG(availabilty_rate) * (SUM(actual_output) / NULLIF(SUM(theoritical_output), 0)) * AVG(quality_rate)) / 100, 2) as shift_avg_oee
      FROM data_oee_rows_hourly
      WHERE work_date = ? AND shift_idx = ?
    ";

        $stmt = $pdo->prepare($sql);
        /* 교대 시작 시각으로 작업일 추출, 교대 번호 기본값 1 */
        $stmt->execute([date('Y-m-d', strtotime($shift_start)), $selected_shift_idx ?: 1]);

        $result = $stmt->fetch(PDO::FETCH_ASSOC);

        return (float)($result['shift_avg_oee'] ?? 0);
    } catch (Exception $e) {
        error_log("Shift OEE average query error: " . $e->getMessage());
        return 0;
    }
}

/**
 * handleStreamingError — 치명적 스트리밍 오류 처리
 *
 * 로그 디렉터리가 없으면 생성 후 에러 메시지를 파일에 기록하고,
 * 클라이언트에 fatal_error SSE 이벤트를 전송.
 *
 * @param string $error  오류 메시지 문자열
 * @return void
 */
function handleStreamingError($error)
{
    /* 로그 디렉터리 경로 설정 (없으면 자동 생성) */
    $logDir = __DIR__ . '/../../logs';
    if (!is_dir($logDir)) {
        mkdir($logDir, 0777, true);
    }

    /* 타임스탬프 포함 에러 메시지를 전용 로그 파일에 기록 */
    $logFile = $logDir . '/log_oee_hourly_stream_errors.log';
    $errorMessage = "[" . date("Y-m-d H:i:s") . "] " . $error . "\n";
    error_log($errorMessage, 3, $logFile);

    /* 클라이언트에 치명적 오류 이벤트 전송 */
    sendSSEData('error', [
        'status' => 'fatal_error',
        'message' => 'Streaming service error occurred.',
        'timestamp' => date('Y-m-d H:i:s')
    ]);
}

/* ================================================================
 * 진입점 — $pdo 유효성 확인 후 스트리밍 시작
 *  - finally 블록에서 disconnected 이벤트 전송 (정상/오류 종료 모두)
 * ================================================================ */
try {
    if (!$pdo) {
        throw new Exception("Database connection failed");
    }

    startStreaming($pdo); // SSE 메인 루프 시작
} catch (Exception $e) {
    /* 치명적 오류 시 에러 로그 기록 및 fatal_error SSE 이벤트 전송 */
    handleStreamingError($e->getMessage());
} finally {
    /* 스트리밍 종료 시 (정상/오류 모두) 클라이언트에 종료 이벤트 전송 */
    sendSSEData('disconnected', [
        'status' => 'disconnected',
        'message' => 'OEE hourly data log streaming ended.',
        'timestamp' => date('Y-m-d H:i:s')
    ]);
}
