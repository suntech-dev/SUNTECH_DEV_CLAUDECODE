<?php

/**
 * Andon Data Real-time Streaming API (Server-Sent Events)
 * Real-time andon data streaming with 3-level filtering support.
 *
 * Optimized (2026-03-07):
 * - Merged getAndonTypeStats: 2 queries (info_andon + data_andon) + PHP merge
 *   → 1 query (info_andon LEFT JOIN data_andon, filter in ON clause)
 * - Removed unnecessary info_factory/info_line JOINs from type stats query
 */


require_once(__DIR__ . '/../../../lib/db.php');
require_once(__DIR__ . '/../../../lib/api_helper.lib.php');
require_once(__DIR__ . '/../../../lib/worktime.lib.php');
require_once(__DIR__ . '/../../../lib/get_shift.lib.php');
require_once(__DIR__ . '/../../../lib/stream_helper.lib.php');

header('Content-Type: text/event-stream');
header('Cache-Control: no-cache');
header('Connection: keep-alive');
header('Access-Control-Allow-Origin: *');
header('Access-Control-Allow-Methods: GET');
header('Access-Control-Allow-Headers: Cache-Control');

if (ob_get_level()) ob_end_clean();

$apiHelper = new ApiHelper($pdo);

// parseFilterParams(), getWorkHoursForDate(), sendSSEData() → stream_helper.lib.php

function getAndonData($pdo, $where_sql, $params, $limit = 100)
{
    try {
        $sql = "
      SELECT 
        da.idx,
        da.work_date,
        da.shift_idx,
        da.machine_no,
        da.andon_name,
        da.status,
        da.reg_date,
        da.update_date,
        da.duration_his,
        da.duration_sec,
        f.factory_name,
        l.line_name,
        ia.color as andon_color,
        CASE
          WHEN da.status = 'Warning' THEN 'Warning'
          WHEN da.status = 'Completed' THEN 'Completed'
          ELSE da.status
        END as status_korean,
        CASE
          WHEN da.duration_sec IS NULL OR da.duration_sec = 0 THEN 'in progress'
          ELSE da.duration_his
        END as duration_display
      FROM data_andon da
      LEFT JOIN info_factory f ON da.factory_idx = f.idx
      LEFT JOIN info_line l ON da.line_idx = l.idx
      LEFT JOIN info_andon ia ON da.andon_idx = ia.idx
      {$where_sql}
      ORDER BY da.reg_date DESC, da.idx DESC
      LIMIT " . (int)$limit . "
    ";

        $stmt = $pdo->prepare($sql);
        $stmt->execute($params);

        $result = $stmt->fetchAll(PDO::FETCH_ASSOC);

        return $result;
    } catch (PDOException $e) {
        error_log("Andon data query error: " . $e->getMessage());
        return [];
    }
}

function getAndonStats($pdo, $where_sql, $params)
{
    try {
        $sql = "
      SELECT 
        COUNT(*) as total_count,
        SUM(CASE WHEN da.status = 'Warning' THEN 1 ELSE 0 END) as warning_count,
        SUM(CASE WHEN da.status = 'Completed' THEN 1 ELSE 0 END) as completed_count,
        AVG(CASE WHEN da.duration_sec IS NOT NULL AND da.duration_sec > 0 THEN da.duration_sec ELSE NULL END) as avg_duration_sec,
        COUNT(DISTINCT da.machine_idx) as affected_machines,
        COUNT(DISTINCT da.andon_idx) as andon_types_used,
        SUM(CASE WHEN DATE(da.reg_date) = CURDATE() THEN 1 ELSE 0 END) as today_count,
        SUM(CASE WHEN DATE(da.reg_date) = CURDATE() AND da.status = 'Warning' THEN 1 ELSE 0 END) as today_warning_count,
        SUM(CASE WHEN da.status = 'Warning' AND TIMESTAMPDIFF(MINUTE, da.reg_date, NOW()) >= 5 THEN 1 ELSE 0 END) as urgent_warnings_count,
        SEC_TO_TIME(AVG(CASE WHEN da.status = 'Completed' AND da.duration_sec IS NOT NULL AND da.duration_sec > 0 THEN da.duration_sec ELSE NULL END)) as avg_completed_time
      FROM data_andon da
      {$where_sql}
    ";

        $stmt = $pdo->prepare($sql);
        $stmt->execute($params);

        $stats = $stmt->fetch(PDO::FETCH_ASSOC);

        $selected_shift = $_GET['shift_filter'] ?? null;
        $target_date = $_GET['start_date'] ?? $_GET['end_date'] ?? null;
        $stats['current_shift_count'] = getCurrentShiftAndonCount($pdo, $selected_shift, $target_date);

        if ($stats['avg_duration_sec'] > 0) {
            $avg_sec = (int)$stats['avg_duration_sec'];
            $hours = floor($avg_sec / 3600);
            $minutes = floor(($avg_sec % 3600) / 60);
            $seconds = $avg_sec % 60;

            $avg_display = '';
            if ($hours > 0) $avg_display .= $hours . 'h ';
            if ($minutes > 0) $avg_display .= $minutes . 'm ';
            if ($seconds > 0) $avg_display .= $seconds . 's';

            $stats['avg_duration_display'] = trim($avg_display);
        } else {
            $stats['avg_duration_display'] = '-';
        }

        if (!empty($stats['avg_completed_time']) && $stats['avg_completed_time'] !== '00:00:00') {
            $time_parts = explode(':', $stats['avg_completed_time']);
            $hours = (int)$time_parts[0];
            $minutes = (int)$time_parts[1];
            $seconds = (int)$time_parts[2];

            $avg_completed_display = '';
            if ($hours > 0) $avg_completed_display .= $hours . 'h ';
            if ($minutes > 0) $avg_completed_display .= $minutes . 'm ';
            if ($seconds > 0) $avg_completed_display .= $seconds . 's';

            $stats['avg_completed_time'] = trim($avg_completed_display);
        } else {
            $stats['avg_completed_time'] = '-';
        }

        return $stats;
    } catch (PDOException $e) {
        error_log("Andon stats query error: " . $e->getMessage());
        return [
            'total_count' => 0,
            'warning_count' => 0,
            'completed_count' => 0,
            'avg_duration_sec' => 0,
            'affected_machines' => 0,
            'andon_types_used' => 0,
            'today_count' => 0,
            'today_warning_count' => 0,
            'avg_duration_display' => '-',
            'current_shift_count' => 0,
            'urgent_warnings_count' => 0,
            'avg_completed_time' => '-'
        ];
    }
}

function getAndonNames($pdo)
{
    try {
        $sql = "SELECT idx, andon_name FROM info_andon WHERE status = 'Y' ORDER BY andon_name";
        $stmt = $pdo->prepare($sql);
        $stmt->execute();

        return $stmt->fetchAll(PDO::FETCH_ASSOC);
    } catch (PDOException $e) {
        error_log("Andon names query error: " . $e->getMessage());
        return [];
    }
}

/**
 * Andon type statistics for charts
 * Optimized: info_andon LEFT JOIN data_andon (was 2 queries + PHP merge)
 * Filter conditions moved to ON clause so all active types appear even with no data.
 */
function getAndonTypeStats($pdo, $where_sql, $params)
{
    try {
        // Convert WHERE clause to ON conditions for LEFT JOIN
        $on_conditions = !empty(trim($where_sql))
            ? ' AND ' . trim(preg_replace('/^\s*WHERE\s*/i', '', $where_sql))
            : '';

        $sql = "
      SELECT
        ia.andon_name,
        ia.color as andon_color,
        COALESCE(COUNT(da.idx), 0) as count,
        COALESCE(SUM(CASE WHEN da.status = 'Warning' THEN 1 ELSE 0 END), 0) as warning_count,
        COALESCE(SUM(CASE WHEN da.status = 'Completed' THEN 1 ELSE 0 END), 0) as completed_count
      FROM info_andon ia
      LEFT JOIN data_andon da
        ON ia.idx = da.andon_idx{$on_conditions}
      WHERE ia.status = 'Y'
      GROUP BY ia.idx, ia.andon_name, ia.color
      ORDER BY COUNT(da.idx) DESC, ia.andon_name ASC
    ";

        $stmt = $pdo->prepare($sql);
        $stmt->execute($params);
        return $stmt->fetchAll(PDO::FETCH_ASSOC);
    } catch (PDOException $e) {
        error_log("Andon type stats query error: " . $e->getMessage());
        return [];
    }
}

/**
 * 안돈 발생 추이 데이터 조회 (스마트 뷰 전환: ≤1일 시간별, >1일 일별)
 */
function getAndonTrendStats($pdo, $where_sql, $params)
{
    try {
        // 날짜 범위 계산으로 뷰 타입 결정
        $startDate = $_GET['start_date'] ?? '';
        $endDate = $_GET['end_date'] ?? '';
        $isHourlyView = false;
        $workHoursInfo = null;

        if (!empty($startDate) && !empty($endDate)) {
            $start = new DateTime($startDate);
            $end = new DateTime($endDate);

            // 시작일과 종료일이 같은 날인지 확인 (오늘 선택 시)
            $startDateOnly = $start->format('Y-m-d');
            $endDateOnly = $end->format('Y-m-d');

            if ($startDateOnly === $endDateOnly) {
                // 같은 날 = 시간별 뷰 (오늘, 어제 등)
                $isHourlyView = true;
                // 디버깅: 같은 날 선택 - 시간별 뷰 사용
            } else {
                $diff = $start->diff($end);
                $daysDiff = $diff->days;

                // ≤1일이면 시간별, >1일이면 일별
                if ($daysDiff <= 1) {
                    $isHourlyView = true;
                }
                // 디버깅: 다른 날 선택 - 일별 뷰 사용
            }
        } else {
            // 기본값: 최근 1일 (시간별 뷰)
            $isHourlyView = true;
        }

        // Get work hours information for hourly view (1 day or less)
        if ($isHourlyView) {
            $targetDate = !empty($startDate) ? $startDate : date('Y-m-d');
            $workHoursInfo = getWorkHoursForDate($pdo, $targetDate);
        }

        if ($isHourlyView) {
            // 시간별 추이 (≤1일) - sql_mode=only_full_group_by 호환
            $sql = "
        SELECT
          DATE_FORMAT(da.reg_date, '%Y-%m-%d %H:00:00') as time_label,
          DATE_FORMAT(da.reg_date, '%H:00') as display_label,
          COUNT(*) as total_count,
          SUM(CASE WHEN da.status = 'Warning' THEN 1 ELSE 0 END) as warning_count,
          SUM(CASE WHEN da.status = 'Completed' THEN 1 ELSE 0 END) as completed_count
        FROM data_andon da
        LEFT JOIN info_factory f ON da.factory_idx = f.idx
        LEFT JOIN info_line l ON da.line_idx = l.idx
        {$where_sql}
        GROUP BY DATE_FORMAT(da.reg_date, '%Y-%m-%d %H:00:00'), DATE_FORMAT(da.reg_date, '%H:00')
        ORDER BY time_label ASC
        LIMIT 48
      ";
            $viewType = 'hourly';

            // 시간별 뷰 준비 완료
        } else {
            // 일별 추이 (>1일)
            // WHERE 조건에서 JOIN 조건 제거 (data_andon만 사용)
            // info_factory와 info_line은 이 쿼리에서 필요하지 않음
            $trend_where_sql = $where_sql;
            // JOIN 조건 제거하고 data_andon만 사용하도록 WHERE 절 단순화
            $trend_where_sql = str_replace('da.factory_idx', 'da.factory_idx', $trend_where_sql);
            $trend_where_sql = str_replace('da.line_idx', 'da.line_idx', $trend_where_sql);

            $sql = "
        SELECT
          DATE(da.reg_date) as time_label,
          DATE_FORMAT(da.reg_date, '%m/%d') as display_label,
          COUNT(*) as total_count,
          SUM(CASE WHEN da.status = 'Warning' THEN 1 ELSE 0 END) as warning_count,
          SUM(CASE WHEN da.status = 'Completed' THEN 1 ELSE 0 END) as completed_count
        FROM data_andon da
        {$trend_where_sql}
        GROUP BY DATE(da.reg_date), DATE_FORMAT(da.reg_date, '%m/%d')
        ORDER BY time_label ASC
        LIMIT 30
      ";
            $viewType = 'daily';
        }

        // 디버깅: 안돈 추이 뷰 타입 정보

        // SQL 쿼리 준비 및 실행
        $stmt = $pdo->prepare($sql);

        // 쿼리 준비 완료

        $executeResult = $stmt->execute($params);

        if (!$executeResult) {
            $errorInfo = $stmt->errorInfo();
            error_log("❌ SQL 실행 실패: " . json_encode($errorInfo, JSON_UNESCAPED_UNICODE));
            error_log("❌ 실패한 SQL: " . $sql);
            error_log("❌ 실패한 파라미터: " . json_encode($params, JSON_UNESCAPED_UNICODE));

            // 💡 실패 시 대체 쿼리로 기본 데이터라도 가져오기
            try {
                $fallbackSql = "
          SELECT 
            DATE_FORMAT(da.reg_date, '%Y-%m-%d %H:00:00') as time_label,
            DATE_FORMAT(da.reg_date, '%H:00') as display_label,
            COUNT(*) as total_count,
            SUM(CASE WHEN da.status = 'Warning' THEN 1 ELSE 0 END) as warning_count,
            SUM(CASE WHEN da.status = 'Completed' THEN 1 ELSE 0 END) as completed_count
          FROM data_andon da
          WHERE da.reg_date >= DATE_SUB(NOW(), INTERVAL 2 DAY)
          GROUP BY DATE_FORMAT(da.reg_date, '%Y-%m-%d %H:00:00'), DATE_FORMAT(da.reg_date, '%H:00')
          ORDER BY time_label ASC
          LIMIT 48
        ";
                $fallbackStmt = $pdo->prepare($fallbackSql);
                $fallbackStmt->execute();
                $results = $fallbackStmt->fetchAll(PDO::FETCH_ASSOC);
                // 대체 쿼리 실행됨
            } catch (Exception $e) {
                error_log("❌ 대체 쿼리도 실패: " . $e->getMessage());
                return [
                    'view_type' => $viewType,
                    'data' => []
                ];
            }
        } else {
            // 디버깅: SQL 실행 성공
            $results = $stmt->fetchAll(PDO::FETCH_ASSOC);
        }

        // 조회 완료 - 결과 확인
        // 디버깅: 안돈 추이 데이터 조회 완료

        // 결과가 비어있는 경우에는 빈 배열 반환 (실제 데이터만 사용)
        if (empty($results)) {
            // 디버깅: 추이 결과가 비어있음
        }

        // Return results with work hours information for hourly view
        if ($isHourlyView && $workHoursInfo) {
            return [
                'data' => $results,
                'work_hours' => $workHoursInfo,
                'view_type' => 'hourly'
            ];
        }

        return [
            'data' => $results,
            'work_hours' => null,
            'view_type' => $viewType
        ];
    } catch (PDOException $e) {
        error_log("안돈 발생 추이 조회 오류: " . $e->getMessage());

        // 오류 발생 시 빈 배열 반환

        return [
            'view_type' => 'hourly',
            'data' => []
        ];
    }
}

/**
 * 최근 활성 안돈 (Warning 상태) 조회
 */
function getActiveAndons($pdo, $where_sql, $params)
{
    try {
        // Warning 상태만 필터링하도록 WHERE 절 수정
        $active_where = $where_sql;
        if (empty($active_where)) {
            $active_where = " WHERE da.status = 'Warning'";
        } else {
            $active_where .= " AND da.status = 'Warning'";
        }

        $sql = "
      SELECT 
        da.idx,
        da.machine_no,
        da.andon_name,
        da.reg_date,
        f.factory_name,
        l.line_name,
        ia.color as andon_color,
        TIMESTAMPDIFF(SECOND, da.reg_date, NOW()) as seconds_elapsed,
        -- 경과 시간을 사람이 읽기 쉬운 형태로
        CONCAT(
          CASE 
            WHEN TIMESTAMPDIFF(HOUR, da.reg_date, NOW()) > 0 
            THEN CONCAT(TIMESTAMPDIFF(HOUR, da.reg_date, NOW()), 'h ')
            ELSE ''
          END,
          CASE 
            WHEN TIMESTAMPDIFF(MINUTE, da.reg_date, NOW()) % 60 > 0
            THEN CONCAT(TIMESTAMPDIFF(MINUTE, da.reg_date, NOW()) % 60, 'm ')
            ELSE ''
          END,
          TIMESTAMPDIFF(SECOND, da.reg_date, NOW()) % 60, 's'
        ) as elapsed_display
      FROM data_andon da
      LEFT JOIN info_factory f ON da.factory_idx = f.idx
      LEFT JOIN info_line l ON da.line_idx = l.idx
      LEFT JOIN info_andon ia ON da.andon_idx = ia.idx
      {$active_where}
      ORDER BY da.reg_date DESC
      LIMIT 5
    ";

        $stmt = $pdo->prepare($sql);
        $stmt->execute($params);

        return $stmt->fetchAll(PDO::FETCH_ASSOC);
    } catch (PDOException $e) {
        error_log("활성 안돈 조회 오류: " . $e->getMessage());
        return [];
    }
}

/**
 * 메인 스트리밍 로직
 */
function startStreaming($pdo)
{
    $lastDataHash = '';
    $startTime = time();
    $maxRunTime = 3600; // 1시간 최대 실행

    // 필터 파라미터 파싱
    $filterConfig = parseFilterParams('da', 'reg_date', false, '2 DAY');
    $limit = !empty($_GET['limit']) ? (int)$_GET['limit'] : 100;

    // 초기 연결 확인 메시지
    sendSSEData('connected', [
        'status' => 'connected',
        'message' => 'Andon 데이터 스트리밍이 시작되었습니다.',
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

    while (true) {
        // 최대 실행 시간 체크
        if (time() - $startTime > $maxRunTime) {
            sendSSEData('timeout', [
                'status' => 'timeout',
                'message' => '최대 실행 시간에 도달했습니다. 연결을 다시 시도해주세요.'
            ]);
            break;
        }

        // 클라이언트 연결 상태 체크
        if (connection_aborted()) {
            error_log("클라이언트 연결이 중단되었습니다.");
            break;
        }

        try {
            // 성능 측정 시작
            $queryStartTime = microtime(true);
            $performanceLog = [];

            // 1. 메인 Andon 데이터 조회
            $t1 = microtime(true);
            $andonData = getAndonData($pdo, $filterConfig['where_sql'], $filterConfig['params'], $limit);
            $performanceLog['andonData'] = round((microtime(true) - $t1) * 1000, 2) . 'ms';

            // 2. 집계 통계 조회
            $t2 = microtime(true);
            $stats = getAndonStats($pdo, $filterConfig['where_sql'], $filterConfig['params']);
            $performanceLog['stats'] = round((microtime(true) - $t2) * 1000, 2) . 'ms';

            // 3. 활성 안돈 조회
            $t3 = microtime(true);
            $activeAndons = getActiveAndons($pdo, $filterConfig['where_sql'], $filterConfig['params']);
            $performanceLog['activeAndons'] = round((microtime(true) - $t3) * 1000, 2) . 'ms';

            // 4. 안돈 유형별 통계 조회 (차트용)
            $t4 = microtime(true);
            $andonTypeStats = getAndonTypeStats($pdo, $filterConfig['where_sql'], $filterConfig['params']);
            $performanceLog['andonTypeStats'] = round((microtime(true) - $t4) * 1000, 2) . 'ms';

            // 5. 안돈 발생 추이 통계 조회 (시간별/일별 차트용)
            $t5 = microtime(true);
            $andonTrendStats = getAndonTrendStats($pdo, $filterConfig['where_sql'], $filterConfig['params']);
            $performanceLog['andonTrendStats'] = round((microtime(true) - $t5) * 1000, 2) . 'ms';

            // 총 쿼리 실행 시간
            $totalQueryTime = round((microtime(true) - $queryStartTime) * 1000, 2);
            $performanceLog['totalQueryTime'] = $totalQueryTime . 'ms';

            // 성능 로그 출력 (느린 쿼리 감지: 1초 이상)
            if ($totalQueryTime > 1000) {
                error_log("⚠️ [Andon] 느린 쿼리 감지 (총 {$totalQueryTime}ms): " . json_encode($performanceLog, JSON_UNESCAPED_UNICODE));
            }

            // 안돈 추이 데이터 변화 감지를 위한 해시 생성 (즉시 업데이트 위해)
            $trendDataForHash = [];
            if (isset($andonTrendStats['data']) && is_array($andonTrendStats['data'])) {
                foreach ($andonTrendStats['data'] as $trendItem) {
                    $trendDataForHash[] = [
                        'time_label' => $trendItem['time_label'] ?? '',
                        'total_count' => $trendItem['total_count'] ?? 0,
                        'warning_count' => $trendItem['warning_count'] ?? 0,
                        'completed_count' => $trendItem['completed_count'] ?? 0
                    ];
                }
            }

            // 데이터 해시 생성 (안돈 발생/해결 시 즉시 감지)
            $hashData = [
                'andon_count' => count($andonData),
                'andon_ids' => array_column($andonData, 'idx'),
                'andon_status_changes' => array_map(function ($item) {
                    return $item['idx'] . '_' . $item['status'];
                }, $andonData),
                'stats_core' => [
                    'total_count' => $stats['total_count'] ?? 0,
                    'warning_count' => $stats['warning_count'] ?? 0,
                    'completed_count' => $stats['completed_count'] ?? 0,
                    'affected_machines' => $stats['affected_machines'] ?? 0
                ],
                'active_andon_ids' => array_column($activeAndons, 'idx'),
                'andon_type_stats' => $andonTypeStats,
                'trend_data_hash' => $trendDataForHash // 추이 데이터 변화 감지
            ];

            $currentDataHash = md5(serialize($hashData));

            // 디버깅: 해시 변화 로깅 (필요시)

            // 데이터가 변경된 경우에만 전송
            if ($currentDataHash !== $lastDataHash) {
                $responseData = [
                    'timestamp' => date('Y-m-d H:i:s'),
                    'stats' => $stats,
                    'active_andons' => $activeAndons,
                    'andon_data' => $andonData,
                    'andon_type_stats' => $andonTypeStats,
                    'andon_trend_stats' => $andonTrendStats,
                    'data_count' => count($andonData),
                    'has_changes' => true
                ];

                sendSSEData('andon_data', $responseData);
                $lastDataHash = $currentDataHash;
            } else {
                // 변화가 없어도 주기적으로 heartbeat 전송
                sendSSEData('heartbeat', [
                    'timestamp' => date('Y-m-d H:i:s'),
                    'status' => 'no_changes',
                    'active_warnings' => $stats['warning_count'] ?? 0
                ]);
            }
        } catch (Exception $e) {
            error_log("스트리밍 중 오류: " . $e->getMessage());

            sendSSEData('error', [
                'status' => 'error',
                'message' => '데이터 조회 중 오류가 발생했습니다.',
                'timestamp' => date('Y-m-d H:i:s')
            ]);
        }

        // 5초 대기
        sleep(5);
    }
}

/**
 * shift의 안돈 발생 수량을 계산하는 함수
 * @param PDO $pdo 데이터베이스 연결
 * @param string $selected_shift_idx 선택된 shift 인덱스 (없으면 현재 shift 사용)
 * @param string $target_date 대상 날짜 (Y-m-d 형식, 없으면 오늘)
 */
function getCurrentShiftAndonCount($pdo, $selected_shift_idx = null, $target_date = null)
{
    try {
        // 현재 시간
        $current_datetime = date('Y-m-d H:i:s');
        $target_date = $target_date ?: date('Y-m-d');

        // Worktime 객체 초기화
        $worktime = new Worktime($pdo);

        // 기본값 설정 (필요에 따라 파라미터로 받을 수 있음)
        $factory_idx = ''; // 전체 공장 대상
        $line_idx = '';    // 전체 라인 대상

        // 특정 shift가 선택된 경우
        if (!empty($selected_shift_idx)) {
            // 특정 날짜의 특정 shift 정보를 가져옴
            $day_shifts = $worktime->getDayShift($target_date, $factory_idx, $line_idx);

            if (!$day_shifts || !isset($day_shifts['shift'][$selected_shift_idx])) {
                return 0; // 해당 shift 정보가 없으면 0 반환
            }

            $shift = $day_shifts['shift'][$selected_shift_idx];

            // shift 시작/종료 시간 계산
            $work_stime_str = $target_date . ' ' . $shift['available_stime'] . ':00';
            $work_etime_str = $target_date . ' ' . $shift['available_etime'] . ':00';

            // 잔업(over_time) 적용
            if ($shift['over_time']) {
                $work_etime_str = date('Y-m-d H:i:s', strtotime($work_etime_str . ' +' . $shift['over_time'] . ' minutes'));
            }

            // 야간 근무 처리 (종료시간이 시작시간보다 이른 경우 다음날로 처리)
            if ($work_etime_str <= $work_stime_str) {
                $work_etime_str = date('Y-m-d H:i:s', strtotime($work_etime_str . ' +1 day'));
            }

            $shift_start = $work_stime_str;
            $shift_end = $work_etime_str;
        } else {
            // 현재 진행 중인 shift 정보 조회 (기존 로직)
            $current_shift_info = findCurrentShift($pdo, $worktime, $factory_idx, $line_idx, $current_datetime);

            if (!$current_shift_info) {
                return 0; // 현재 진행 중인 shift가 없으면 0 반환
            }

            $shift_start = $current_shift_info['work_stime'];
            $shift_end = $current_shift_info['work_etime'];
        }

        // shift 시간 범위 내의 안돈 수량 조회
        $sql = "
      SELECT COUNT(*) as shift_andon_count
      FROM data_andon da
      WHERE da.reg_date >= ? AND da.reg_date < ?
    ";

        $stmt = $pdo->prepare($sql);
        $stmt->execute([$shift_start, $shift_end]);

        $result = $stmt->fetch(PDO::FETCH_ASSOC);

        return (int)($result['shift_andon_count'] ?? 0);
    } catch (Exception $e) {
        error_log("shift 안돈 수량 조회 오류: " . $e->getMessage());
        return 0;
    }
}

/**
 * 에러 핸들링 및 로깅
 */
function handleStreamingError($error)
{
    $logDir = __DIR__ . '/../../logs';
    if (!is_dir($logDir)) {
        mkdir($logDir, 0777, true);
    }

    $logFile = $logDir . '/andon_stream_errors.log';
    $errorMessage = "[" . date("Y-m-d H:i:s") . "] " . $error . "\n";
    error_log($errorMessage, 3, $logFile);

    sendSSEData('error', [
        'status' => 'fatal_error',
        'message' => '스트리밍 서비스에 오류가 발생했습니다.',
        'timestamp' => date('Y-m-d H:i:s')
    ]);
}

// 메인 실행 부분
try {
    if (!$pdo) {
        throw new Exception("데이터베이스 연결 실패");
    }

    // 스트리밍 시작
    startStreaming($pdo);
} catch (Exception $e) {
    handleStreamingError($e->getMessage());
} finally {
    // 연결 종료 메시지
    sendSSEData('disconnected', [
        'status' => 'disconnected',
        'message' => 'Andon 데이터 스트리밍이 종료되었습니다.',
        'timestamp' => date('Y-m-d H:i:s')
    ]);
}

/*
## API 사용 예시

### 1. 기본 사용
GET /data/proc/data_andon_stream_2.php

### 2. 필터 적용
GET /data/proc/data_andon_stream_2.php?factory_filter=1&line_filter=2

### 3. 날짜 범위 지정
GET /data/proc/data_andon_stream_2.php?start_date=2025-01-01&end_date=2025-01-07

### 4. 조합 필터
GET /data/proc/data_andon_stream_2.php?factory_filter=1&start_date=2025-01-01&limit=50

## SSE 이벤트 타입

1. **connected**: 연결 성공
2. **andon_data**: 메인 안돈 데이터 (변화 시에만)
3. **heartbeat**: 연결 유지 신호 (변화 없을 때)  
4. **error**: 오류 발생
5. **timeout**: 최대 실행 시간 초과
6. **disconnected**: 연결 종료

## 응답 데이터 구조

```javascript
{
  "timestamp": "2025-01-06 10:30:25",
  "stats": {
    "total_count": 150,
    "warning_count": 8,
    "completed_count": 142,
    "avg_duration_sec": 185,
    "affected_machines": 12,
    "andon_types_used": 5,
    "today_count": 25,
    "today_warning_count": 3,
    "avg_duration_display": "3m 5s"
  },
  "active_andons": [
    {
      "idx": 1001,
      "machine_no": "M001", 
      "andon_name": "재료 부족",
      "reg_date": "2025-01-06 10:25:10",
      "factory_name": "Factory A",
      "line_name": "Line A-1",
      "seconds_elapsed": 315,
      "elapsed_display": "5m 15s"
    }
  ],
  "andon_data": [
    {
      "idx": 1002,
      "work_date": "2025-01-06",
      "shift_idx": 1,
      "machine_no": "M002",
      "andon_name": "품질 불량", 
      "status": "Completed",
      "status_korean": "완료",
      "status_color": "success",
      "duration_display": "2m 30s",
      "factory_name": "Factory A",
      "line_name": "Line A-2"
    }
  ],
  "andon_type_stats": [
    {
      "andon_name": "재료부족",
      "count": 25,
      "warning_count": 3,
      "completed_count": 22
    },
    {
      "andon_name": "품질불량",
      "count": 18,
      "warning_count": 2,
      "completed_count": 16
    }
  ],
  "andon_trend_stats": {
    "view_type": "hourly",
    "data": [
      {
        "time_label": "2025-01-06 08:00:00",
        "display_label": "08:00",
        "total_count": 5,
        "warning_count": 1,
        "completed_count": 4
      },
      {
        "time_label": "2025-01-06 09:00:00",
        "display_label": "09:00",
        "total_count": 8,
        "warning_count": 2,
        "completed_count": 6
      }
    ]
  },
  "data_count": 50,
  "has_changes": true
}
```
*/
