<?php
/**
 * Downtime 데이터 실시간 스트리밍 API (Server-Sent Events)
 * 
 * 기능:
 * - data_downtime 테이블의 실시간 데이터를 SSE 방식으로 스트리밍
 * - Factory → Line → Machine 3단계 필터링 지원
 * - 날짜 범위 필터링 지원
 * - 실시간 상태 변화 감지 및 전송
 * 
 * 사용법:
 * GET /data/proc/data_downtime_stream.php
 * 
 * 파라미터:
 * - factory_filter: 공장 필터 (선택)
 * - line_filter: 라인 필터 (선택)  
 * - machine_filter: 기계 필터 (선택)
 * - start_date: 시작 날짜 (YYYY-MM-DD, 선택)
 * - end_date: 종료 날짜 (YYYY-MM-DD, 선택)
 * - limit: 조회 개수 제한 (기본: 100)
 */

// 타임존 설정
date_default_timezone_set('Asia/Jakarta');

// 공통 라이브러리 로드
require_once(__DIR__ . '/../../../lib/db.php');
require_once(__DIR__ . '/../../../lib/api_helper.lib.php');
require_once(__DIR__ . '/../../../lib/worktime.lib.php');
require_once(__DIR__ . '/../../../lib/get_shift.lib.php');

// SSE 헤더 설정
header('Content-Type: text/event-stream');
header('Cache-Control: no-cache');
header('Connection: keep-alive');
header('Access-Control-Allow-Origin: *');
header('Access-Control-Allow-Methods: GET');
header('Access-Control-Allow-Headers: Cache-Control');

// 출력 버퍼링 비활성화
if (ob_get_level()) ob_end_clean();

// API 헬퍼 초기화
$apiHelper = new ApiHelper($pdo);

/**
 * 필터 파라미터 파싱 함수
 */
function parseFilterParams() {
  $params = [];
  $where_clauses = [];
  
  // Factory 필터
  if (!empty($_GET['factory_filter'])) {
    $where_clauses[] = 'dd.factory_idx = ?';
    $params[] = $_GET['factory_filter'];
  }
  
  // Line 필터  
  if (!empty($_GET['line_filter'])) {
    $where_clauses[] = 'dd.line_idx = ?';
    $params[] = $_GET['line_filter'];
  }
  
  // Machine 필터
  if (!empty($_GET['machine_filter'])) {
    $where_clauses[] = 'dd.machine_idx = ?';
    $params[] = $_GET['machine_filter'];
  }
  
  // Shift 필터
  if (!empty($_GET['shift_filter'])) {
    $where_clauses[] = 'dd.shift_idx = ?';
    $params[] = $_GET['shift_filter'];
  }
  
  // 날짜 범위 필터
  if (!empty($_GET['start_date'])) {
    $where_clauses[] = 'dd.reg_date >= ?';
    $params[] = $_GET['start_date'] . ' 00:00:00';
  }
  
  if (!empty($_GET['end_date'])) {
    $where_clauses[] = 'dd.reg_date <= ?';
    $params[] = $_GET['end_date'] . ' 23:59:59';
  }
  
  // 날짜 필터가 없을 때만 기본 범위 적용 (최근 2일)
  if (empty($_GET['start_date']) && empty($_GET['end_date'])) {
    $where_clauses[] = 'dd.reg_date >= DATE_SUB(NOW(), INTERVAL 2 DAY)';
  }
  
  $where_sql = count($where_clauses) > 0 ? ' WHERE ' . implode(' AND ', $where_clauses) : '';
  
  return ['where_sql' => $where_sql, 'params' => $params];
}

/**
 * Downtime 데이터 조회 함수
 */
function getDowntimeData($pdo, $where_sql, $params, $limit = 100) {
  try {
    $sql = "
      SELECT 
        dd.idx,
        dd.work_date,
        dd.shift_idx,
        dd.machine_no,
        dd.downtime_name,
        dd.status,
        dd.reg_date,
        dd.update_date,
        dd.duration_his,
        dd.duration_sec,
        f.factory_name,
        l.line_name,
        -- 상태 표시용
        CASE 
          WHEN dd.status = 'Warning' THEN '경고'
          WHEN dd.status = 'Completed' THEN '완료'  
          ELSE dd.status
        END as status_korean,
        CASE
          WHEN dd.duration_sec IS NULL OR dd.duration_sec = 0 THEN 'in progress'
          ELSE dd.duration_his
        END as duration_display
      FROM data_downtime dd
      LEFT JOIN info_factory f ON dd.factory_idx = f.idx
      LEFT JOIN info_line l ON dd.line_idx = l.idx  
      {$where_sql}
      ORDER BY dd.reg_date DESC, dd.idx DESC
      LIMIT " . (int)$limit . "
    ";
    
    $stmt = $pdo->prepare($sql);
    $stmt->execute($params);
    
    $result = $stmt->fetchAll(PDO::FETCH_ASSOC);
    
    return $result;
    
  } catch (PDOException $e) {
    error_log("Downtime 데이터 조회 오류: " . $e->getMessage());
    return [];
  }
}

/**
 * 집계 통계 데이터 조회 함수
 */
function getDowntimeStats($pdo, $where_sql, $params) {
  try {
    $sql = "
      SELECT 
        COUNT(*) as total_count,
        SUM(CASE WHEN dd.status = 'Warning' THEN 1 ELSE 0 END) as warning_count,
        SUM(CASE WHEN dd.status = 'Completed' THEN 1 ELSE 0 END) as completed_count,
        AVG(CASE WHEN dd.duration_sec IS NOT NULL AND dd.duration_sec > 0 THEN dd.duration_sec ELSE NULL END) as avg_duration_sec,
        COUNT(DISTINCT dd.machine_idx) as affected_machines,
        COUNT(DISTINCT dd.downtime_idx) as downtime_types_used,
        -- 오늘 데이터
        SUM(CASE WHEN DATE(dd.reg_date) = CURDATE() THEN 1 ELSE 0 END) as today_count,
        SUM(CASE WHEN DATE(dd.reg_date) = CURDATE() AND dd.status = 'Warning' THEN 1 ELSE 0 END) as today_warning_count,
        -- 30분 이상 다운타임 수량 (장시간 다운타임)
        SUM(CASE WHEN dd.status = 'Warning' AND TIMESTAMPDIFF(MINUTE, dd.reg_date, NOW()) >= 30 THEN 1 ELSE 0 END) as long_downtimes_count,
        -- 평균 완료 시간 계산
        SEC_TO_TIME(AVG(CASE WHEN dd.status = 'Completed' AND dd.duration_sec IS NOT NULL AND dd.duration_sec > 0 THEN dd.duration_sec ELSE NULL END)) as avg_completed_time
      FROM data_downtime dd
      {$where_sql}
    ";
    
    $stmt = $pdo->prepare($sql);
    $stmt->execute($params);
    
    $stats = $stmt->fetch(PDO::FETCH_ASSOC);
    
    // 현재 shift 발생 수량 계산 (get_shift.lib.php 사용)
    // shift_filter 파라미터가 있으면 해당 shift의 수량, 없으면 현재 진행중인 shift 수량
    $selected_shift = $_GET['shift_filter'] ?? null;
    $target_date = $_GET['start_date'] ?? $_GET['end_date'] ?? null;
    $stats['current_shift_count'] = getCurrentShiftDowntimeCount($pdo, $selected_shift, $target_date);
    
    // 평균 지속 시간을 사람이 읽기 쉬운 형태로 변환
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
    
    // 평균 완료 시간 포맷팅
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
    error_log("Downtime 통계 조회 오류: " . $e->getMessage());
    return [
      'total_count' => 0,
      'warning_count' => 0, 
      'completed_count' => 0,
      'avg_duration_sec' => 0,
      'affected_machines' => 0,
      'downtime_types_used' => 0,
      'today_count' => 0,
      'today_warning_count' => 0,
      'avg_duration_display' => '-',
      'current_shift_count' => 0,
      'long_downtimes_count' => 0,
      'avg_completed_time' => '-'
    ];
  }
}

/**
 * 다운타임 유형별 통계 데이터 조회 (차트용)
 */
function getDowntimeTypeStats($pdo, $where_sql, $params) {
  try {
    // 먼저 모든 활성 다운타임 유형 조회
    $allDowntimesQuery = "SELECT downtime_name FROM info_downtime WHERE status = 'Y' ORDER BY downtime_name";
    $allDowntimesStmt = $pdo->prepare($allDowntimesQuery);
    $allDowntimesStmt->execute();
    $allDowntimes = $allDowntimesStmt->fetchAll(PDO::FETCH_ASSOC);
    
    // info_downtime 테이블에 데이터가 없으면 data_downtime에서 고유한 다운타임 유형 조회 후 0으로 채우기
    if (empty($allDowntimes)) {
      // 먼저 data_downtime에 있는 모든 고유한 downtime_name 조회
      $uniqueDowntimesQuery = "SELECT DISTINCT dd.downtime_name FROM data_downtime dd ORDER BY dd.downtime_name";
      $uniqueDowntimesStmt = $pdo->prepare($uniqueDowntimesQuery);
      $uniqueDowntimesStmt->execute();
      $uniqueDowntimes = $uniqueDowntimesStmt->fetchAll(PDO::FETCH_ASSOC);
      
      // 각 다운타임 유형을 allDowntimes 형태로 변환
      $allDowntimes = $uniqueDowntimes;
    }
    
    // 실제 데이터가 있는 다운타임 유형별 통계 조회
    $dataQuery = "
      SELECT
        dd.downtime_name,
        COUNT(*) as count,
        SUM(CASE WHEN dd.status = 'Warning' THEN 1 ELSE 0 END) as warning_count,
        SUM(CASE WHEN dd.status = 'Completed' THEN 1 ELSE 0 END) as completed_count,
        -- 총 다운타임 지속시간 (초 단위)
        SUM(COALESCE(dd.duration_sec, 0)) as total_duration_sec,
        -- 총 다운타임 지속시간 (분 단위, 소수점 1자리)
        ROUND(SUM(COALESCE(dd.duration_sec, 0)) / 60.0, 1) as total_duration_min
      FROM data_downtime dd
      LEFT JOIN info_factory f ON dd.factory_idx = f.idx
      LEFT JOIN info_line l ON dd.line_idx = l.idx
      {$where_sql}
      GROUP BY dd.downtime_name
      ORDER BY dd.downtime_name ASC
    ";
    
    $dataStmt = $pdo->prepare($dataQuery);
    $dataStmt->execute($params);
    $actualData = $dataStmt->fetchAll(PDO::FETCH_ASSOC);
    
    // 실제 데이터를 downtime_name으로 인덱싱
    $dataByName = [];
    foreach ($actualData as $item) {
      $dataByName[$item['downtime_name']] = $item;
    }
    
    // 모든 다운타임 유형에 대해 결과 구성
    $result = [];
    foreach ($allDowntimes as $downtime) {
      $downtimeName = $downtime['downtime_name'];
      if (isset($dataByName[$downtimeName])) {
        // 실제 데이터가 있는 경우
        $result[] = $dataByName[$downtimeName];
      } else {
        // 데이터가 없는 경우 0으로 설정
        $result[] = [
          'downtime_name' => $downtimeName,
          'count' => 0,
          'warning_count' => 0,
          'completed_count' => 0,
          'total_duration_sec' => 0,
          'total_duration_min' => 0.0
        ];
      }
    }
    
    // 다운타임 이름(downtime_name) 기준으로 알파벳순 정렬
    usort($result, function($a, $b) {
      return strcmp($a['downtime_name'], $b['downtime_name']);
    });
    
    return $result;
    
  } catch (PDOException $e) {
    error_log("다운타임 유형별 통계 조회 오류: " . $e->getMessage());
    return [];
  }
}

/**
 * Get work hours information for a specific date
 * @param PDO $pdo Database connection
 * @param string $targetDate Target date in Y-m-d format
 * @return array|null Work hours information
 */
function getWorkHoursForDate($pdo, $targetDate) {
  try {
    $worktime = new Worktime($pdo);
    $factory_idx = '';
    $line_idx = '';

    // Get shift information for target date
    $dayShifts = $worktime->getDayShift($targetDate, $factory_idx, $line_idx);

    if (!$dayShifts || !isset($dayShifts['shift']) || empty($dayShifts['shift'])) {
      return null;
    }

    $shifts = $dayShifts['shift'];

    // Process all shifts to find earliest start and latest end
    $earliestStartMinutes = 24 * 60; // Initialize to end of day
    $latestEndMinutes = 0;

    foreach ($shifts as $shift) {
      if (empty($shift['available_stime']) || empty($shift['available_etime'])) {
        continue;
      }

      // Convert start time to minutes since midnight
      list($startHour, $startMin) = explode(':', $shift['available_stime']);
      $startMinutes = (int)$startHour * 60 + (int)$startMin;

      // Convert end time to minutes since midnight
      list($endHour, $endMin) = explode(':', $shift['available_etime']);
      $endMinutes = (int)$endHour * 60 + (int)$endMin;

      // Add over_time to end minutes
      if (isset($shift['over_time']) && $shift['over_time'] > 0) {
        $endMinutes += (int)$shift['over_time'];
      }

      // Handle overnight shifts (end time < start time)
      if ($endMinutes <= $startMinutes) {
        $endMinutes += 24 * 60; // Add 24 hours
      }

      // Track earliest start and latest end
      if ($startMinutes < $earliestStartMinutes) {
        $earliestStartMinutes = $startMinutes;
      }
      if ($endMinutes > $latestEndMinutes) {
        $latestEndMinutes = $endMinutes;
      }
    }

    // Convert back to HH:mm format
    $startHour = floor($earliestStartMinutes / 60);
    $startMin = $earliestStartMinutes % 60;
    $workStartTime = sprintf('%02d:%02d', $startHour, $startMin);

    $endHour = floor($latestEndMinutes / 60) % 24; // Modulo 24 for display
    $endMin = $latestEndMinutes % 60;
    $workEndTime = sprintf('%02d:%02d', $endHour, $endMin);

    return [
      'start_time' => $workStartTime,
      'end_time' => $workEndTime,
      'start_minutes' => $earliestStartMinutes,
      'end_minutes' => $latestEndMinutes,
      'shifts' => array_values($shifts)
    ];

  } catch (Exception $e) {
    error_log("Work hours query error: " . $e->getMessage());
    return null;
  }
}

/**
 * 다운타임 발생 추이 데이터 조회 (스마트 뷰 전환: ≤1일 시간별, >1일 일별)
 */
function getDowntimeTrendStats($pdo, $where_sql, $params) {
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
      } else {
        $diff = $start->diff($end);
        $daysDiff = $diff->days;

        // ≤1일이면 시간별, >1일이면 일별
        if ($daysDiff <= 1) {
          $isHourlyView = true;
        }
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
          DATE_FORMAT(dd.reg_date, '%Y-%m-%d %H:00:00') as time_label,
          DATE_FORMAT(dd.reg_date, '%H:00') as display_label,
          COUNT(*) as total_count,
          SUM(CASE WHEN dd.status = 'Warning' THEN 1 ELSE 0 END) as warning_count,
          SUM(CASE WHEN dd.status = 'Completed' THEN 1 ELSE 0 END) as completed_count
        FROM data_downtime dd
        LEFT JOIN info_factory f ON dd.factory_idx = f.idx
        LEFT JOIN info_line l ON dd.line_idx = l.idx
        {$where_sql}
        GROUP BY DATE_FORMAT(dd.reg_date, '%Y-%m-%d %H:00:00'), DATE_FORMAT(dd.reg_date, '%H:00')
        ORDER BY time_label ASC
        LIMIT 48
      ";
      $viewType = 'hourly';
    } else {
      // 일별 추이 (>1일)
      // WHERE 조건에서 JOIN 조건 제거 (data_downtime만 사용)
      $trend_where_sql = $where_sql;
      // JOIN 조건 제거하고 data_downtime만 사용하도록 WHERE 절 단순화
      $trend_where_sql = str_replace('dd.factory_idx', 'dd.factory_idx', $trend_where_sql);
      $trend_where_sql = str_replace('dd.line_idx', 'dd.line_idx', $trend_where_sql);

      $sql = "
        SELECT
          DATE(dd.reg_date) as time_label,
          DATE_FORMAT(dd.reg_date, '%m/%d') as display_label,
          COUNT(*) as total_count,
          SUM(CASE WHEN dd.status = 'Warning' THEN 1 ELSE 0 END) as warning_count,
          SUM(CASE WHEN dd.status = 'Completed' THEN 1 ELSE 0 END) as completed_count
        FROM data_downtime dd
        {$trend_where_sql}
        GROUP BY DATE(dd.reg_date), DATE_FORMAT(dd.reg_date, '%m/%d')
        ORDER BY time_label ASC
        LIMIT 30
      ";
      $viewType = 'daily';
    }
    
    // SQL 쿼리 준비 및 실행
    $stmt = $pdo->prepare($sql);
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
            DATE_FORMAT(dd.reg_date, '%Y-%m-%d %H:00:00') as time_label,
            DATE_FORMAT(dd.reg_date, '%H:00') as display_label,
            COUNT(*) as total_count,
            SUM(CASE WHEN dd.status = 'Warning' THEN 1 ELSE 0 END) as warning_count,
            SUM(CASE WHEN dd.status = 'Completed' THEN 1 ELSE 0 END) as completed_count
          FROM data_downtime dd
          WHERE dd.reg_date >= DATE_SUB(NOW(), INTERVAL 2 DAY)
          GROUP BY DATE(dd.reg_date), HOUR(dd.reg_date)
          ORDER BY DATE(dd.reg_date), HOUR(dd.reg_date) ASC
          LIMIT 48
        ";
        $fallbackStmt = $pdo->prepare($fallbackSql);
        $fallbackStmt->execute();
        $results = $fallbackStmt->fetchAll(PDO::FETCH_ASSOC);
      } catch (Exception $e) {
        error_log("❌ 대체 쿼리도 실패: " . $e->getMessage());
        return [
          'view_type' => $viewType,
          'data' => []
        ];
      }
    } else {
      $results = $stmt->fetchAll(PDO::FETCH_ASSOC);
    }
    
    // 결과가 비어있는 경우에는 빈 배열 반환 (실제 데이터만 사용)
    if (empty($results)) {
      // 빈 데이터
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
    error_log("다운타임 발생 추이 조회 오류: " . $e->getMessage());

    return [
      'data' => [],
      'work_hours' => null,
      'view_type' => 'hourly'
    ];
  }
}

/**
 * 다운타임 지속시간 분포 데이터 조회 (파이 차트용)
 */
function getDowntimeDurationStats($pdo, $where_sql, $params) {
  try {
    $sql = "
      SELECT 
        SUM(CASE WHEN dd.duration_sec IS NOT NULL AND dd.duration_sec < 300 THEN 1 ELSE 0 END) as under_5min,
        SUM(CASE WHEN dd.duration_sec IS NOT NULL AND dd.duration_sec >= 300 AND dd.duration_sec < 900 THEN 1 ELSE 0 END) as min_5_15,
        SUM(CASE WHEN dd.duration_sec IS NOT NULL AND dd.duration_sec >= 900 AND dd.duration_sec < 1800 THEN 1 ELSE 0 END) as min_15_30,
        SUM(CASE WHEN dd.duration_sec IS NOT NULL AND dd.duration_sec >= 1800 AND dd.duration_sec < 3600 THEN 1 ELSE 0 END) as min_30_60,
        SUM(CASE WHEN dd.duration_sec IS NOT NULL AND dd.duration_sec >= 3600 THEN 1 ELSE 0 END) as over_1hour
      FROM data_downtime dd
      LEFT JOIN info_factory f ON dd.factory_idx = f.idx
      LEFT JOIN info_line l ON dd.line_idx = l.idx  
      {$where_sql}
    ";
    
    $stmt = $pdo->prepare($sql);
    $stmt->execute($params);
    
    $result = $stmt->fetch(PDO::FETCH_ASSOC);
    
    return $result;
    
  } catch (PDOException $e) {
    error_log("다운타임 지속시간 분포 조회 오류: " . $e->getMessage());
    return [
      'under_5min' => 0,
      'min_5_15' => 0,
      'min_15_30' => 0,
      'min_30_60' => 0,
      'over_1hour' => 0
    ];
  }
}

/**
 * 최근 활성 다운타임 (Warning 상태) 조회
 */
function getActiveDowntimes($pdo, $where_sql, $params) {
  try {
    // Warning 상태만 필터링하도록 WHERE 절 수정
    $active_where = $where_sql;
    if (empty($active_where)) {
      $active_where = " WHERE dd.status = 'Warning'";
    } else {
      $active_where .= " AND dd.status = 'Warning'";
    }
    
    $sql = "
      SELECT 
        dd.idx,
        dd.machine_no,
        dd.downtime_name,
        dd.reg_date,
        f.factory_name,
        l.line_name,
        TIMESTAMPDIFF(SECOND, dd.reg_date, NOW()) as seconds_elapsed,
        -- 경과 시간을 사람이 읽기 쉬운 형태로
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
        ) as elapsed_display
      FROM data_downtime dd
      LEFT JOIN info_factory f ON dd.factory_idx = f.idx
      LEFT JOIN info_line l ON dd.line_idx = l.idx
      {$active_where}
      ORDER BY dd.reg_date DESC
      LIMIT 5
    ";
    
    $stmt = $pdo->prepare($sql);
    $stmt->execute($params);
    
    return $stmt->fetchAll(PDO::FETCH_ASSOC);
    
  } catch (PDOException $e) {
    error_log("활성 다운타임 조회 오류: " . $e->getMessage());
    return [];
  }
}

/**
 * SSE 데이터 전송 함수
 */
function sendSSEData($eventType, $data) {
  echo "event: {$eventType}\n";
  echo "data: " . json_encode($data, JSON_UNESCAPED_UNICODE | JSON_UNESCAPED_SLASHES) . "\n\n";
  flush();
}

/**
 * 메인 스트리밍 로직
 */
function startStreaming($pdo) {
  $lastDataHash = '';
  $startTime = time();
  $maxRunTime = 3600; // 1시간 최대 실행

  // 필터 파라미터 파싱
  $filterConfig = parseFilterParams();
  $limit = !empty($_GET['limit']) ? (int)$_GET['limit'] : 100;

  // 성능 측정을 위한 초기 로깅
  $performanceLog = [];
  
  // 초기 연결 확인 메시지
  sendSSEData('connected', [
    'status' => 'connected',
    'message' => 'Downtime 데이터 스트리밍이 시작되었습니다.',
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

      // 1. 메인 Downtime 데이터 조회
      $t1 = microtime(true);
      $downtimeData = getDowntimeData($pdo, $filterConfig['where_sql'], $filterConfig['params'], $limit);
      $performanceLog['downtimeData'] = round((microtime(true) - $t1) * 1000, 2) . 'ms';

      // 2. 집계 통계 조회
      $t2 = microtime(true);
      $stats = getDowntimeStats($pdo, $filterConfig['where_sql'], $filterConfig['params']);
      $performanceLog['stats'] = round((microtime(true) - $t2) * 1000, 2) . 'ms';

      // 3. 활성 다운타임 조회
      $t3 = microtime(true);
      $activeDowntimes = getActiveDowntimes($pdo, $filterConfig['where_sql'], $filterConfig['params']);
      $performanceLog['activeDowntimes'] = round((microtime(true) - $t3) * 1000, 2) . 'ms';

      // 4. 다운타임 유형별 통계 조회 (차트용)
      $t4 = microtime(true);
      $downtimeTypeStats = getDowntimeTypeStats($pdo, $filterConfig['where_sql'], $filterConfig['params']);
      $performanceLog['downtimeTypeStats'] = round((microtime(true) - $t4) * 1000, 2) . 'ms';

      // 5. 다운타임 발생 추이 통계 조회 (시간별/일별 차트용)
      $t5 = microtime(true);
      $downtimeTrendStats = getDowntimeTrendStats($pdo, $filterConfig['where_sql'], $filterConfig['params']);
      $performanceLog['downtimeTrendStats'] = round((microtime(true) - $t5) * 1000, 2) . 'ms';

      // 6. 다운타임 지속시간 분포 통계 조회 (파이 차트용)
      $t6 = microtime(true);
      $downtimeDurationStats = getDowntimeDurationStats($pdo, $filterConfig['where_sql'], $filterConfig['params']);
      $performanceLog['downtimeDurationStats'] = round((microtime(true) - $t6) * 1000, 2) . 'ms';

      // 총 쿼리 실행 시간
      $totalQueryTime = round((microtime(true) - $queryStartTime) * 1000, 2);
      $performanceLog['totalQueryTime'] = $totalQueryTime . 'ms';

      // 성능 로그 출력 (느린 쿼리 감지: 1초 이상)
      if ($totalQueryTime > 1000) {
        error_log("⚠️ 느린 쿼리 감지 (총 {$totalQueryTime}ms): " . json_encode($performanceLog, JSON_UNESCAPED_UNICODE));
      }
      
      // 다운타임 추이 데이터 변화 감지를 위한 해시 생성 (즉시 업데이트 위해)
      $trendDataForHash = [];
      if (isset($downtimeTrendStats['data']) && is_array($downtimeTrendStats['data'])) {
        foreach ($downtimeTrendStats['data'] as $trendItem) {
          $trendDataForHash[] = [
            'time_label' => $trendItem['time_label'] ?? '',
            'total_count' => $trendItem['total_count'] ?? 0,
            'warning_count' => $trendItem['warning_count'] ?? 0,
            'completed_count' => $trendItem['completed_count'] ?? 0
          ];
        }
      }
      
      // 데이터 해시 생성 (다운타임 발생/해결 시 즉시 감지)
      $hashData = [
        'downtime_count' => count($downtimeData),
        'downtime_ids' => array_column($downtimeData, 'idx'),
        'downtime_status_changes' => array_map(function($item) {
          return $item['idx'] . '_' . $item['status']; 
        }, $downtimeData),
        'stats_core' => [
          'total_count' => $stats['total_count'] ?? 0,
          'warning_count' => $stats['warning_count'] ?? 0,
          'completed_count' => $stats['completed_count'] ?? 0,
          'affected_machines' => $stats['affected_machines'] ?? 0
        ],
        'active_downtime_ids' => array_column($activeDowntimes, 'idx'),
        'downtime_type_stats' => $downtimeTypeStats,
        'trend_data_hash' => $trendDataForHash // 추이 데이터 변화 감지
      ];
      
      $currentDataHash = md5(serialize($hashData));
      
      // 데이터가 변경된 경우에만 전송
      if ($currentDataHash !== $lastDataHash) {
        $responseData = [
          'timestamp' => date('Y-m-d H:i:s'),
          'stats' => $stats,
          'active_downtimes' => $activeDowntimes,
          'downtime_data' => $downtimeData,
          'downtime_type_stats' => $downtimeTypeStats,
          'downtime_trend_stats' => $downtimeTrendStats,
          'downtime_duration_stats' => $downtimeDurationStats,
          'data_count' => count($downtimeData),
          'has_changes' => true
        ];
        
        sendSSEData('downtime_data', $responseData);
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
 * shift의 다운타임 발생 수량을 계산하는 함수
 * @param PDO $pdo 데이터베이스 연결
 * @param string $selected_shift_idx 선택된 shift 인덱스 (없으면 현재 shift 사용)
 * @param string $target_date 대상 날짜 (Y-m-d 형식, 없으면 오늘)
 */
function getCurrentShiftDowntimeCount($pdo, $selected_shift_idx = null, $target_date = null) {
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
      
      // Apply overtime
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
    
    // shift 시간 범위 내의 다운타임 수량 조회
    $sql = "
      SELECT COUNT(*) as shift_downtime_count
      FROM data_downtime dd
      WHERE dd.reg_date >= ? AND dd.reg_date < ?
    ";
    
    $stmt = $pdo->prepare($sql);
    $stmt->execute([$shift_start, $shift_end]);
    
    $result = $stmt->fetch(PDO::FETCH_ASSOC);
    
    return (int)($result['shift_downtime_count'] ?? 0);
    
  } catch (Exception $e) {
    error_log("shift 다운타임 수량 조회 오류: " . $e->getMessage());
    return 0;
  }
}

/**
 * 에러 핸들링 및 로깅
 */
function handleStreamingError($error) {
  $logDir = __DIR__ . '/../../logs';
  if (!is_dir($logDir)) {
    mkdir($logDir, 0777, true);
  }
  
  $logFile = $logDir . '/downtime_stream_errors.log';
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
    'message' => 'Downtime 데이터 스트리밍이 종료되었습니다.',
    'timestamp' => date('Y-m-d H:i:s')
  ]);
}

/*
## API 사용 예시

### 1. 기본 사용
GET /data/proc/data_downtime_stream.php

### 2. 필터 적용
GET /data/proc/data_downtime_stream.php?factory_filter=1&line_filter=2

### 3. 날짜 범위 지정
GET /data/proc/data_downtime_stream.php?start_date=2025-01-01&end_date=2025-01-07

### 4. 조합 필터
GET /data/proc/data_downtime_stream.php?factory_filter=1&start_date=2025-01-01&limit=50

## SSE 이벤트 타입

1. **connected**: 연결 성공
2. **downtime_data**: 메인 다운타임 데이터 (변화 시에만)
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
    "downtime_types_used": 5,
    "today_count": 25,
    "today_warning_count": 3,
    "avg_duration_display": "3m 5s",
    "current_shift_count": 5,
    "long_downtimes_count": 2,
    "avg_completed_time": "15m 30s"
  },
  "active_downtimes": [
    {
      "idx": 1001,
      "machine_no": "M001", 
      "downtime_name": "EDIT PROGRAM",
      "reg_date": "2025-01-06 10:25:10",
      "factory_name": "Factory A",
      "line_name": "Line A-1",
      "seconds_elapsed": 315,
      "elapsed_display": "5m 15s"
    }
  ],
  "downtime_data": [
    {
      "idx": 1002,
      "work_date": "2025-01-06",
      "shift_idx": 1,
      "machine_no": "M002",
      "downtime_name": "6S", 
      "status": "Completed",
      "status_korean": "완료",
      "duration_display": "2m 30s",
      "factory_name": "Factory A",
      "line_name": "Line A-2"
    }
  ],
  "downtime_type_stats": [
    {
      "downtime_name": "6S",
      "count": 25,
      "warning_count": 3,
      "completed_count": 22
    },
    {
      "downtime_name": "EDIT PROGRAM",
      "count": 18,
      "warning_count": 2,
      "completed_count": 16
    }
  ],
  "downtime_trend_stats": {
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
  "downtime_duration_stats": {
    "under_5min": 20,
    "min_5_15": 15,
    "min_15_30": 8,
    "min_30_60": 5,
    "over_1hour": 2
  },
  "data_count": 50,
  "has_changes": true
}
```
*/
?>