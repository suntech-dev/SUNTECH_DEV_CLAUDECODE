<?php
/**
 * SCI OEE Dashboard 실시간 데이터 스트리밍 API
 * Server-Sent Events (SSE) 기반 통합 데이터 제공
 */


require_once(__DIR__ . '/../../../lib/db.php');
require_once(__DIR__ . '/../../../lib/api_helper.lib.php');
require_once(__DIR__ . '/../../../lib/worktime.lib.php');
require_once(__DIR__ . '/../../../lib/get_shift.lib.php');
require_once(__DIR__ . '/../../../lib/stream_helper.lib.php');

// SSE 헤더 설정
header('Content-Type: text/event-stream');
header('Cache-Control: no-cache');
header('Connection: keep-alive');
header('Access-Control-Allow-Origin: *');
header('Access-Control-Allow-Methods: GET');
header('Access-Control-Allow-Headers: Cache-Control');

// 출력 버퍼링 비활성화
if (ob_get_level()) ob_end_clean();

$apiHelper = new ApiHelper($pdo);

/**
 * 필터 파라미터 파싱 (dashboard 전용: BETWEEN/= 날짜 로직, 반환 키 'where')
 */
function parseDashboardFilterParams($tableAlias = '') {
  $params = [];
  $where_clauses = [];

  $prefix = $tableAlias ? $tableAlias . '.' : '';

  if (!empty($_GET['factory_filter'])) {
    $where_clauses[] = $prefix . 'factory_idx = ?';
    $params[] = $_GET['factory_filter'];
  }

  if (!empty($_GET['line_filter'])) {
    $where_clauses[] = $prefix . 'line_idx = ?';
    $params[] = $_GET['line_filter'];
  }

  if (!empty($_GET['machine_filter'])) {
    $where_clauses[] = $prefix . 'machine_idx = ?';
    $params[] = $_GET['machine_filter'];
  }

  if (!empty($_GET['shift_filter'])) {
    $where_clauses[] = $prefix . 'shift_idx = ?';
    $params[] = $_GET['shift_filter'];
  }

  // 날짜 범위 처리
  $today = date('Y-m-d');
  if (!empty($_GET['start_date']) && !empty($_GET['end_date'])) {
    $where_clauses[] = $prefix . 'work_date BETWEEN ? AND ?';
    $params[] = $_GET['start_date'];
    $params[] = $_GET['end_date'];
  } else if (!empty($_GET['date_filter'])) {
    $where_clauses[] = $prefix . 'work_date = ?';
    $params[] = $_GET['date_filter'];
  } else {
    $where_clauses[] = $prefix . 'work_date = ?';
    $params[] = $today;
  }

  return [
    'params' => $params,
    'where' => empty($where_clauses) ? '' : 'WHERE ' . implode(' AND ', $where_clauses)
  ];
}

/**
 * 이전 기간 OEE 데이터 조회 (변화율 계산용)
 */
function getPreviousOEEData($pdo, $filter) {
  try {
    // 현재 필터의 날짜 범위를 파싱하여 이전 기간 계산
    $where_clauses = [];
    $params = [];

    // 공장/라인/기계 필터는 동일하게 유지
    if (!empty($_GET['factory_filter'])) {
      $where_clauses[] = 'factory_idx = ?';
      $params[] = $_GET['factory_filter'];
    }

    if (!empty($_GET['line_filter'])) {
      $where_clauses[] = 'line_idx = ?';
      $params[] = $_GET['line_filter'];
    }

    if (!empty($_GET['machine_filter'])) {
      $where_clauses[] = 'machine_idx = ?';
      $params[] = $_GET['machine_filter'];
    }

    if (!empty($_GET['shift_filter'])) {
      $where_clauses[] = 'shift_idx = ?';
      $params[] = $_GET['shift_filter'];
    }

    // 이전 기간 날짜 계산
    $today = date('Y-m-d');
    $yesterday = date('Y-m-d', strtotime('-1 day'));

    if (!empty($_GET['start_date']) && !empty($_GET['end_date'])) {
      $start = $_GET['start_date'];
      $end = $_GET['end_date'];

      // 기간 차이 계산
      $date1 = new DateTime($start);
      $date2 = new DateTime($end);
      $interval = $date1->diff($date2);
      $days = $interval->days + 1;

      // 이전 기간 계산 (동일한 기간만큼 이전)
      $prev_end = date('Y-m-d', strtotime($start . ' -1 day'));
      $prev_start = date('Y-m-d', strtotime($prev_end . " -{$days} days"));

      $where_clauses[] = 'work_date BETWEEN ? AND ?';
      $params[] = $prev_start;
      $params[] = $prev_end;
    } else {
      // 기본값: 어제 데이터
      $where_clauses[] = 'work_date = ?';
      $params[] = $yesterday;
    }

    $where_sql = empty($where_clauses) ? '' : 'WHERE ' . implode(' AND ', $where_clauses);

    $sql = "
      SELECT
        AVG(do.availabilty_rate) as avg_availability,
        (SUM(do.actual_output) / NULLIF(SUM(do.theoritical_output), 0)) * 100 as avg_performance,
        AVG(do.quality_rate) as avg_quality,
        ROUND((AVG(do.availabilty_rate) * (SUM(do.actual_output) / NULLIF(SUM(do.theoritical_output), 0)) * AVG(do.quality_rate)) / 100, 2) as avg_oee
      FROM data_oee do
      LEFT JOIN info_machine m ON do.machine_idx = m.idx
      {$where_sql}" . (strpos($where_sql, 'WHERE') !== false ? ' AND' : ' WHERE') . " m.status = 'Y'
    ";

    $stmt = $pdo->prepare($sql);
    $stmt->execute($params);
    $result = $stmt->fetch(PDO::FETCH_ASSOC);

    return $result ?: [
      'avg_availability' => 0,
      'avg_performance' => 0,
      'avg_quality' => 0,
      'avg_oee' => 0
    ];

  } catch (Exception $e) {
    error_log("이전 OEE 데이터 조회 오류: " . $e->getMessage());
    return [
      'avg_availability' => 0,
      'avg_performance' => 0,
      'avg_quality' => 0,
      'avg_oee' => 0
    ];
  }
}

/**
 * OEE 데이터 조회
 */
function getOEEData($pdo, $filter) {
  try {
    $sql = "
      SELECT
        AVG(do.availabilty_rate) as avg_availability,
        (SUM(do.actual_output) / NULLIF(SUM(do.theoritical_output), 0)) * 100 as avg_performance,
        AVG(do.quality_rate) as avg_quality,
        ROUND((AVG(do.availabilty_rate) * (SUM(do.actual_output) / NULLIF(SUM(do.theoritical_output), 0)) * AVG(do.quality_rate)) / 100, 2) as avg_oee,
        SUM(do.runtime) as total_runtime,
        SUM(do.planned_work_time) as total_planned_time,
        SUM(do.downtime) as total_downtime,
        SUM(do.actual_output) as total_actual_output,
        SUM(do.theoritical_output) as total_theoretical_output,
        SUM(do.actual_a_grade) as total_good_products,
        SUM(do.defective) as total_defective_products
      FROM data_oee do
      LEFT JOIN info_machine m ON do.machine_idx = m.idx
      {$filter['where']}" . (strpos($filter['where'], 'WHERE') !== false ? ' AND' : ' WHERE') . " m.status = 'Y'
    ";

    $stmt = $pdo->prepare($sql);
    $stmt->execute($filter['params']);
    $result = $stmt->fetch(PDO::FETCH_ASSOC);

    if (!$result || $result['avg_availability'] === null) {
      return [
        'availability' => ['value' => 0, 'runtime' => 0, 'planned_time' => 0, 'trend' => '→', 'change' => 0],
        'performance' => ['value' => 0, 'actual_output' => 0, 'theoretical_output' => 0, 'trend' => '→', 'change' => 0],
        'quality' => ['value' => 0, 'good_products' => 0, 'defective_products' => 0, 'trend' => '→', 'change' => 0],
        'overall' => ['value' => 0, 'trend' => '→', 'change' => 0]
      ];
    }

    // 계산된 메트릭 (100% 미만은 소수점 1자리 내림, 100 이상은 100으로 제한)
    $formatRate = function($value) {
      $v = $value ?? 0;
      return $v >= 100 ? 100 : floor($v * 10) / 10;
    };

    // [FIX v2] Availability = (planned - downtime) / planned
    // - SUM(runtime)/SUM(planned) 는 미분류 대기시간을 손실로 오인
    // - 올바른 공식: downtime=0 이면 100%, downtime>0 이면 비례 감소
    $avail_rate = ($result['total_planned_time'] ?? 0) > 0
      ? (($result['total_planned_time'] - ($result['total_downtime'] ?? 0)) / $result['total_planned_time']) * 100 : 0;
    $availability = $formatRate($avail_rate);
    $performance = $formatRate($result['avg_performance']);
    $quality = $formatRate($result['avg_quality']);
    $overall = $formatRate($result['avg_oee']);

    // 이전 기간 데이터 조회하여 실제 변화율 계산
    $previous = getPreviousOEEData($pdo, $filter);

    $prev_availability = $formatRate($previous['avg_availability']);
    $prev_performance = $formatRate($previous['avg_performance']);
    $prev_quality = $formatRate($previous['avg_quality']);
    $prev_overall = $formatRate($previous['avg_oee']);

    // 변화율 계산 (소수점 1자리)
    $availability_change = $prev_availability > 0 ? round($availability - $prev_availability, 1) : 0;
    $performance_change = $prev_performance > 0 ? round($performance - $prev_performance, 1) : 0;
    $quality_change = $prev_quality > 0 ? round($quality - $prev_quality, 1) : 0;
    $overall_change = $prev_overall > 0 ? round($overall - $prev_overall, 1) : 0;

    return [
      'availability' => [
        'value'        => $availability,
        'downtime'     => round(($result['total_downtime'] ?? 0) / 60, 1),
        'planned_time' => round(($result['total_planned_time'] ?? 0) / 60, 1),
        'downtime_pct' => round(($result['total_downtime'] ?? 0) / max($result['total_planned_time'], 1) * 100, 1),
        'trend'  => $availability_change > 0 ? '↗️' : ($availability_change < 0 ? '↘️' : '→'),
        'change' => $availability_change
      ],
      'performance' => [
        'value' => $performance,
        'actual_output' => round($result['total_actual_output'] ?? 0, 0),
        'theoretical_output' => round($result['total_theoretical_output'] ?? 0, 0),
        'trend' => $performance_change > 0 ? '↗️' : ($performance_change < 0 ? '↘️' : '→'),
        'change' => $performance_change
      ],
      'quality' => [
        'value' => $quality,
        'good_products' => round($result['total_good_products'] ?? 0, 0),
        'defective_products' => round($result['total_defective_products'] ?? 0, 0),
        'trend' => $quality_change > 0 ? '↗️' : ($quality_change < 0 ? '↘️' : '→'),
        'change' => $quality_change
      ],
      'overall' => [
        'value' => $overall,
        'trend' => $overall_change > 0 ? '↗️' : ($overall_change < 0 ? '↘️' : '→'),
        'change' => $overall_change
      ]
    ];

  } catch (Exception $e) {
    error_log("OEE 데이터 조회 오류: " . $e->getMessage());
    return ['error' => $e->getMessage()];
  }
}

/**
 * Andon 데이터 조회
 */
function getAndonData($pdo, $filter) {
  try {
    // 모든 활성 Andon 유형 조회 (data_andon.php와 일관성 유지)
    $allAndonsQuery = "SELECT andon_name, color FROM info_andon WHERE status = 'Y' ORDER BY andon_name";
    $allAndonsStmt = $pdo->prepare($allAndonsQuery);
    $allAndonsStmt->execute();
    $allAndons = $allAndonsStmt->fetchAll(PDO::FETCH_ASSOC);

    // info_andon 테이블에 데이터가 없으면 data_andon에서 고유한 안돈 유형 조회
    if (empty($allAndons)) {
      $uniqueAndonsQuery = "SELECT DISTINCT andon_name FROM data_andon ORDER BY andon_name";
      $uniqueAndonsStmt = $pdo->prepare($uniqueAndonsQuery);
      $uniqueAndonsStmt->execute();
      $uniqueAndons = $uniqueAndonsStmt->fetchAll(PDO::FETCH_ASSOC);
      $allAndons = $uniqueAndons;
    }

    // Andon 발생수량 조회 (실제 데이터) - 테이블 alias 'da' 사용
    // $filter['where']가 이미 'WHERE da.factory_idx = ?' 형식이므로 직접 사용
    $occurrence_sql = "
      SELECT
        da.andon_name,
        ia.color as andon_color,
        COUNT(*) as count
      FROM data_andon da
      LEFT JOIN info_andon ia ON da.andon_idx = ia.idx
      {$filter['where']}
      GROUP BY da.andon_name, ia.color
      ORDER BY count DESC
    ";

    $stmt = $pdo->prepare($occurrence_sql);
    $stmt->execute($filter['params']);
    $occurrence_data = $stmt->fetchAll(PDO::FETCH_ASSOC);

    // 실제 데이터를 andon_name으로 매핑
    $dataByName = [];
    foreach ($occurrence_data as $item) {
      $dataByName[$item['andon_name']] = $item;
    }

    // 모든 안돈 유형에 대해 데이터 생성 (0 카운트 포함)
    $occurrence_stats = [];
    foreach ($allAndons as $andon) {
      $andonName = $andon['andon_name'];
      if (isset($dataByName[$andonName])) {
        $occurrence_stats[] = $dataByName[$andonName];
      } else {
        $occurrence_stats[] = [
          'andon_name' => $andonName,
          'andon_color' => $andon['color'] ?? null,
          'count' => 0
        ];
      }
    }

    // count 기준 내림차순 정렬 (동일하면 이름 순)
    usort($occurrence_stats, function($a, $b) {
      if ($a['count'] == $b['count']) {
        return strcmp($a['andon_name'], $b['andon_name']);
      }
      return $b['count'] - $a['count'];
    });
    
    // Andon 유형 및 색상 조회 (info_andon 테이블에서 동적 조회)
    $andonTypesQuery = "SELECT idx, andon_name, color FROM info_andon WHERE status = 'Y' ORDER BY idx";
    $andonTypesStmt = $pdo->prepare($andonTypesQuery);
    $andonTypesStmt->execute();
    $andonTypes = $andonTypesStmt->fetchAll(PDO::FETCH_ASSOC);

    // andon_idx를 키로 하는 색상 및 이름 맵 생성
    $andonColorMap = [];
    $andonNameMap = [];

    foreach ($andonTypes as $andon) {
      $idx = (int)$andon['idx'];
      $andonColorMap[$idx] = $andon['color'];
      $andonNameMap[$idx] = $andon['andon_name'];
    }

    // 주간 추이 데이터 (최근 7일) - 필터 적용
    // 날짜 범위 조건 추가
    $where_clauses = [];
    $weekly_params = [];

    // 기존 필터 조건 추가 (factory, line, machine, shift)
    if (!empty($_GET['factory_filter'])) {
      $where_clauses[] = 'da.factory_idx = ?';
      $weekly_params[] = $_GET['factory_filter'];
    }

    if (!empty($_GET['line_filter'])) {
      $where_clauses[] = 'da.line_idx = ?';
      $weekly_params[] = $_GET['line_filter'];
    }

    if (!empty($_GET['machine_filter'])) {
      $where_clauses[] = 'da.machine_idx = ?';
      $weekly_params[] = $_GET['machine_filter'];
    }

    if (!empty($_GET['shift_filter'])) {
      $where_clauses[] = 'da.shift_idx = ?';
      $weekly_params[] = $_GET['shift_filter'];
    }

    // 날짜 범위 필터 적용 - 이번 주 월요일~일요일 기준
    // 사용자가 선택한 날짜 범위의 마지막 날이 속한 주의 월요일~일요일
    if (!empty($_GET['start_date']) && !empty($_GET['end_date'])) {
      $reference_date = $_GET['end_date']; // 사용자가 선택한 마지막 날
    } else {
      $reference_date = date('Y-m-d'); // 오늘
    }

    // 기준 날짜가 속한 주의 월요일 계산 (ISO-8601: 월요일 = 1, 일요일 = 7)
    $ref_day_of_week = date('N', strtotime($reference_date)); // 1(월)~7(일)
    $days_since_monday = $ref_day_of_week - 1; // 월요일부터 며칠 지났는지
    $week_start = date('Y-m-d', strtotime($reference_date . " -{$days_since_monday} days")); // 이번 주 월요일
    $week_end = date('Y-m-d', strtotime($week_start . ' +6 days')); // 이번 주 일요일

    $where_clauses[] = 'DATE(da.reg_date) BETWEEN ? AND ?';
    $weekly_params[] = $week_start;
    $weekly_params[] = $week_end;

    $weekly_where = empty($where_clauses) ? '' : 'WHERE ' . implode(' AND ', $where_clauses);

    $weekly_sql = "
      SELECT
        DATE(da.reg_date) as date,
        da.andon_idx,
        ia.andon_name,
        COUNT(*) as count
      FROM data_andon da
      JOIN info_andon ia ON da.andon_idx = ia.idx
      {$weekly_where}
      GROUP BY DATE(da.reg_date), da.andon_idx, ia.andon_name
      ORDER BY date ASC
    ";

    $stmt = $pdo->prepare($weekly_sql);
    $stmt->execute($weekly_params);
    $weekly_raw = $stmt->fetchAll(PDO::FETCH_ASSOC);

    // 주간 데이터 가공 (7일간) - 동적 Andon 유형 기반
    $weekly_data = [];

    // 각 andon_idx별 데이터 배열 초기화
    foreach ($andonTypes as $andon) {
      $idx = (int)$andon['idx'];
      $weekly_data[$idx] = [0, 0, 0, 0, 0, 0, 0];
    }

    // 전체 합계 초기화
    $weekly_data['total'] = [0, 0, 0, 0, 0, 0, 0];

    // 이번 주 월요일~일요일 날짜 배열 생성 (차트 레이블과 일치)
    // 인덱스 0 = 월요일, 인덱스 1 = 화요일, ..., 인덱스 6 = 일요일
    $date_map = [];
    for ($i = 0; $i < 7; $i++) {
      $date = date('Y-m-d', strtotime($week_start . " +{$i} days"));
      $date_map[$date] = $i; // 인덱스 매핑 (0=월요일, 6=일요일)
    }

    // 실제 데이터로 채우기
    foreach ($weekly_raw as $row) {
      $date = $row['date'];
      $andon_idx = (int)$row['andon_idx'];
      $count = (int)$row['count'];

      // 날짜가 매핑에 있는지 확인
      if (!isset($date_map[$date])) continue;

      $day_index = $date_map[$date];

      // Andon 유형별 카운트 (동적)
      if (isset($weekly_data[$andon_idx])) {
        $weekly_data[$andon_idx][$day_index] += $count;
      }

      // 전체 합계
      $weekly_data['total'][$day_index] += $count;
    }
    
    // 실시간 알람 조회 (필터 적용)
    // Warning 상태만 필터링하도록 WHERE 절 수정 (data_andon_stream.php와 일관성 유지)
    $alarm_where = $filter['where'];

    // 필터가 없으면 기본 WHERE 절 추가 (Warning 상태만)
    if (empty($alarm_where)) {
      $alarm_where = "WHERE da.status = 'Warning'";
      $alarm_params = [];
    } else {
      // 필터가 있으면 Warning 조건 추가
      $alarm_where .= " AND da.status = 'Warning'";
      $alarm_params = $filter['params'];
    }

    $alarm_sql = "
      SELECT
        da.idx,
        da.reg_date,
        da.status,
        da.machine_no,
        factory.factory_name,
        line.line_name,
        ia.andon_name,
        ia.color as andon_color
      FROM data_andon da
      LEFT JOIN info_factory factory ON da.factory_idx = factory.idx
      LEFT JOIN info_line line ON da.line_idx = line.idx
      LEFT JOIN info_andon ia ON da.andon_idx = ia.idx
      {$alarm_where}
      ORDER BY da.reg_date DESC
      LIMIT 10
    ";

    $stmt = $pdo->prepare($alarm_sql);
    $stmt->execute($alarm_params);
    $alarms_raw = $stmt->fetchAll(PDO::FETCH_ASSOC);

    $alarms = array_map(function($alarm) {
      return [
        'id' => $alarm['idx'],
        'type' => $alarm['status'] == 'Warning' ? 'error' : 'success',
        'andon_name' => $alarm['andon_name'],
        'factory_name' => $alarm['factory_name'],
        'line_name' => $alarm['line_name'],
        'machine_no' => $alarm['machine_no'],
        'andon_color' => $alarm['andon_color'],
        'reg_date' => $alarm['reg_date'],
        'timestamp' => strtotime($alarm['reg_date']) * 1000,
        'acknowledged' => $alarm['status'] == 'Completed'
      ];
    }, $alarms_raw);
    
    return [
      'occurrence' => $occurrence_stats, // andon_name, andon_color, count 포함
      'weekly' => $weekly_data,
      'weekly_colors' => $andonColorMap, // 동적 색상 맵
      'andon_types' => $andonTypes, // Andon 유형 정보 (idx, andon_name, color)
      'alarms' => $alarms
    ];
    
  } catch (Exception $e) {
    error_log("Andon 데이터 조회 오류: " . $e->getMessage());
    return ['error' => $e->getMessage()];
  }
}

/**
 * Downtime 데이터 조회 (data_downtime.php와 일관성 유지)
 */
function getDowntimeData($pdo, $filter) {
  try {
    // 먼저 모든 활성 다운타임 유형 조회 (data_downtime_stream.php와 동일 패턴)
    $allDowntimesQuery = "SELECT downtime_name FROM info_downtime WHERE status = 'Y' ORDER BY downtime_name";
    $allDowntimesStmt = $pdo->prepare($allDowntimesQuery);
    $allDowntimesStmt->execute();
    $allDowntimes = $allDowntimesStmt->fetchAll(PDO::FETCH_ASSOC);

    // info_downtime 테이블에 데이터가 없으면 data_downtime에서 고유한 다운타임 유형 조회
    if (empty($allDowntimes)) {
      $uniqueDowntimesQuery = "SELECT DISTINCT downtime_name FROM data_downtime ORDER BY downtime_name";
      $uniqueDowntimesStmt = $pdo->prepare($uniqueDowntimesQuery);
      $uniqueDowntimesStmt->execute();
      $allDowntimes = $uniqueDowntimesStmt->fetchAll(PDO::FETCH_ASSOC);
    }

    // 실제 데이터가 있는 다운타임 유형별 통계 조회
    $sql = "
      SELECT
        dd.downtime_name,
        COUNT(*) as count,
        -- 총 다운타임 지속시간 (초 단위)
        SUM(COALESCE(dd.duration_sec, 0)) as total_duration_sec,
        -- 총 다운타임 지속시간 (분 단위, 소수점 1자리)
        ROUND(SUM(COALESCE(dd.duration_sec, 0)) / 60.0, 1) as total_duration_min
      FROM data_downtime dd
      {$filter['where']}
      GROUP BY dd.downtime_name
      ORDER BY dd.downtime_name ASC
    ";

    $stmt = $pdo->prepare($sql);
    $stmt->execute($filter['params']);
    $actualData = $stmt->fetchAll(PDO::FETCH_ASSOC);

    // 실제 데이터를 downtime_name으로 인덱싱
    $dataByName = [];
    foreach ($actualData as $item) {
      $dataByName[$item['downtime_name']] = $item;
    }

    // 모든 다운타임 유형에 대해 결과 구성 (데이터 없으면 0으로 채움)
    $occurrence_stats = [];
    foreach ($allDowntimes as $downtime) {
      $downtimeName = $downtime['downtime_name'];
      if (isset($dataByName[$downtimeName])) {
        // 실제 데이터가 있는 경우
        $occurrence_stats[] = [
          'downtime_name' => $downtimeName,
          'count' => (int)$dataByName[$downtimeName]['count'],
          'total_duration_sec' => (int)$dataByName[$downtimeName]['total_duration_sec'],
          'total_duration_min' => (float)$dataByName[$downtimeName]['total_duration_min']
        ];
      } else {
        // 데이터가 없는 경우 0으로 설정
        $occurrence_stats[] = [
          'downtime_name' => $downtimeName,
          'count' => 0,
          'total_duration_sec' => 0,
          'total_duration_min' => 0.0
        ];
      }
    }

    return [
      'occurrence' => $occurrence_stats
    ];

  } catch (Exception $e) {
    error_log("Downtime 데이터 조회 오류: " . $e->getMessage());
    return ['error' => $e->getMessage()];
  }
}

/**
 * Defective 데이터 조회 (data_downtime.php와 일관성 유지)
 */
function getDefectiveData($pdo, $filter) {
  try {
    // 먼저 모든 활성 불량 유형 조회 (data_downtime_stream.php와 동일 패턴)
    $allDefectivesQuery = "SELECT defective_name FROM info_defective WHERE status = 'Y' ORDER BY defective_name";
    $allDefectivesStmt = $pdo->prepare($allDefectivesQuery);
    $allDefectivesStmt->execute();
    $allDefectives = $allDefectivesStmt->fetchAll(PDO::FETCH_ASSOC);

    // info_defective 테이블에 데이터가 없으면 data_defective에서 고유한 불량 유형 조회
    if (empty($allDefectives)) {
      $uniqueDefectivesQuery = "SELECT DISTINCT defective_name FROM data_defective ORDER BY defective_name";
      $uniqueDefectivesStmt = $pdo->prepare($uniqueDefectivesQuery);
      $uniqueDefectivesStmt->execute();
      $allDefectives = $uniqueDefectivesStmt->fetchAll(PDO::FETCH_ASSOC);
    }

    // 실제 데이터가 있는 불량 유형별 통계 조회
    $sql = "
      SELECT
        dd.defective_name,
        COUNT(*) as count
      FROM data_defective dd
      {$filter['where']}
      GROUP BY dd.defective_name
      ORDER BY count DESC
    ";

    $stmt = $pdo->prepare($sql);
    $stmt->execute($filter['params']);
    $actualData = $stmt->fetchAll(PDO::FETCH_ASSOC);

    // 실제 데이터를 defective_name으로 인덱싱
    $dataByName = [];
    foreach ($actualData as $item) {
      $dataByName[$item['defective_name']] = $item;
    }

    // 모든 불량 유형에 대해 결과 구성 (데이터 없으면 0으로 채움)
    $occurrence_stats = [];
    foreach ($allDefectives as $defective) {
      $defectiveName = $defective['defective_name'];
      if (isset($dataByName[$defectiveName])) {
        // 실제 데이터가 있는 경우
        $occurrence_stats[] = [
          'defective_name' => $defectiveName,
          'count' => (int)$dataByName[$defectiveName]['count']
        ];
      } else {
        // 데이터가 없는 경우 0으로 설정
        $occurrence_stats[] = [
          'defective_name' => $defectiveName,
          'count' => 0
        ];
      }
    }

    return [
      'occurrence' => $occurrence_stats
    ];

  } catch (Exception $e) {
    error_log("Defective 데이터 조회 오류: " . $e->getMessage());
    return ['error' => $e->getMessage()];
  }
}

/**
 * Get rate color configuration from info_rate_color table
 * @param PDO $pdo Database connection
 * @return array Rate color configuration
 */
function getRateColorConfig($pdo) {
  try {
    $sql = "
      SELECT
        idx,
        start_rate,
        end_rate,
        color
      FROM info_rate_color
      ORDER BY start_rate ASC
    ";

    $stmt = $pdo->prepare($sql);
    $stmt->execute();
    $results = $stmt->fetchAll(PDO::FETCH_ASSOC);

    return $results ?: [];

  } catch (Exception $e) {
    error_log("Rate color config query error: " . $e->getMessage());
    // 기본값 반환 (fallback)
    return [
      ['idx' => 1, 'start_rate' => 0, 'end_rate' => 40, 'color' => '#da1e28'],
      ['idx' => 2, 'start_rate' => 40, 'end_rate' => 60, 'color' => '#f5f310'],
      ['idx' => 3, 'start_rate' => 60, 'end_rate' => 78, 'color' => '#ff9800'],
      ['idx' => 4, 'start_rate' => 78, 'end_rate' => 100, 'color' => '#009432'],
      ['idx' => 5, 'start_rate' => 100, 'end_rate' => 999, 'color' => '#0070f2']
    ];
  }
}

/**
 * Get color and status for a specific OEE value based on rate_color config
 * @param float $oee OEE value
 * @param array $rateColorConfig Rate color configuration
 * @return array Color and status information
 */
function getOeeColorAndStatus($oee, $rateColorConfig) {
  if ($oee === null) {
    return [
      'status' => 'no-data',
      'color' => '#e0e0e0',
      'status_text' => 'No data'
    ];
  }

  // Rate color config를 순회하며 해당하는 범위 찾기
  foreach ($rateColorConfig as $config) {
    $startRate = (float)$config['start_rate'];
    $endRate = (float)$config['end_rate'];

    if ($oee >= $startRate && $oee < $endRate) {
      return [
        'status' => 'rate-' . $config['idx'],
        'color' => $config['color'],
        'status_text' => getRateStatusText($startRate, $endRate)
      ];
    }
  }

  // 범위에 해당하지 않는 경우 기본값
  return [
    'status' => 'unknown',
    'color' => '#e0e0e0',
    'status_text' => '알 수 없음'
  ];
}

/**
 * Get status text for rate range
 * @param float $startRate Start rate
 * @param float $endRate End rate
 * @return string Status text
 */
function getRateStatusText($startRate, $endRate) {
  if ($endRate > 100) {
    return '최고 성능';
  } else if ($startRate >= 78) {
    return '우수';
  } else if ($startRate >= 60) {
    return '양호';
  } else if ($startRate >= 40) {
    return '주의';
  } else {
    return '불량';
  }
}

// getWorkHoursForDate() → stream_helper.lib.php

/**
 * OEE Trend 데이터 조회
 */
function getOeeTrendData($pdo, $filter) {
  try {
    $startDate = $_GET['start_date'] ?? '';
    $endDate = $_GET['end_date'] ?? '';
    $isHourlyView = false;
    $workHoursInfo = null;

    // 날짜 범위가 지정되지 않은 경우 오늘 날짜 사용
    if (empty($startDate) && empty($endDate)) {
      $startDate = date('Y-m-d');
      $endDate = date('Y-m-d');
      $isHourlyView = true;
    } else if (!empty($startDate) && !empty($endDate)) {
      $start = new DateTime($startDate);
      $end = new DateTime($endDate);

      $startDateOnly = $start->format('Y-m-d');
      $endDateOnly = $end->format('Y-m-d');

      if ($startDateOnly === $endDateOnly) {
        $isHourlyView = true;
      } else {
        $diff = $start->diff($end);
        $daysDiff = $diff->days;

        if ($daysDiff <= 1) {
          $isHourlyView = true;
        }
      }
    } else {
      $isHourlyView = true;
    }

    // Get work hours information for hourly view (1 day or less)
    if ($isHourlyView) {
      $targetDate = !empty($startDate) ? $startDate : date('Y-m-d');
      $workHoursInfo = getWorkHoursForDate($pdo, $targetDate);
    }

    if ($isHourlyView) {
      // 1일 이하: 근무시간 기준 시간별 표시
      $sql = "
        SELECT
          CONCAT(doh.work_date, ' ', LPAD(doh.work_hour, 2, '0'), ':00:00') as time_label,
          CONCAT(LPAD(doh.work_hour, 2, '0'), ':00') as display_label,
          ROUND((AVG(doh.availabilty_rate) * (SUM(doh.actual_output) / NULLIF(SUM(doh.theoritical_output), 0)) * AVG(doh.quality_rate)) / 100, 2) as overall_oee,
          ROUND(AVG(doh.availabilty_rate), 2) as availability,
          ROUND((SUM(doh.actual_output) / NULLIF(SUM(doh.theoritical_output), 0)) * 100, 2) as performance,
          ROUND(AVG(doh.quality_rate), 2) as quality,
          COUNT(*) as record_count
        FROM data_oee_rows_hourly doh
        LEFT JOIN info_machine m ON doh.machine_idx = m.idx
        {$filter['where']}" . (strpos($filter['where'], 'WHERE') !== false ? ' AND' : ' WHERE') . " m.status = 'Y'
        GROUP BY doh.work_date, doh.work_hour
        ORDER BY doh.work_date ASC, doh.work_hour ASC
        LIMIT 24
      ";
      $viewType = 'hourly';
    } else {
      // 1일 초과: 0H~24H 모든 시간의 평균값 표시
      $sql = "
        SELECT
          CONCAT(LPAD(doh.work_hour, 2, '0'), 'H') as time_label,
          CONCAT(LPAD(doh.work_hour, 2, '0'), 'H') as display_label,
          ROUND((AVG(doh.availabilty_rate) * (SUM(doh.actual_output) / NULLIF(SUM(doh.theoritical_output), 0)) * AVG(doh.quality_rate)) / 100, 2) as overall_oee,
          ROUND(AVG(doh.availabilty_rate), 2) as availability,
          ROUND((SUM(doh.actual_output) / NULLIF(SUM(doh.theoritical_output), 0)) * 100, 2) as performance,
          ROUND(AVG(doh.quality_rate), 2) as quality,
          COUNT(*) as record_count
        FROM data_oee_rows_hourly doh
        LEFT JOIN info_machine m ON doh.machine_idx = m.idx
        {$filter['where']}" . (strpos($filter['where'], 'WHERE') !== false ? ' AND' : ' WHERE') . " m.status = 'Y'
        GROUP BY doh.work_hour
        ORDER BY doh.work_hour ASC
      ";
      $viewType = 'hourly';
      // 1일 초과 시에는 work_hours를 null로 설정하여 0H~24H 전체 표시
      $workHoursInfo = null;
    }

    $stmt = $pdo->prepare($sql);
    $stmt->execute($filter['params']);
    $results = $stmt->fetchAll(PDO::FETCH_ASSOC);

    // 1일 이하이고 근무시간 정보가 있는 경우에만 work_hours 반환
    // 1일 초과인 경우 work_hours는 null (0H~24H 전체 표시)
    return [
      'view_type' => $viewType,
      'data' => $results,
      'work_hours' => $workHoursInfo
    ];

  } catch (Exception $e) {
    error_log("OEE Trend 데이터 조회 오류: " . $e->getMessage());
    return [
      'view_type' => 'hourly',
      'data' => [],
      'work_hours' => null
    ];
  }
}

/**
 * Production 데이터 조회
 */
function getProductionData($pdo, $filter) {
  try {
    // Rate color configuration 조회 (한 번만 조회)
    $rateColorConfig = getRateColorConfig($pdo);

    // 시간별 생산량 데이터 (8시-20시) - data_oee_rows_hourly 사용
    $where_clause = $filter['where'];
    $hour_condition = 'work_hour BETWEEN 8 AND 19';

    if (empty($where_clause)) {
      $hourly_where = "WHERE {$hour_condition}";
    } else {
      $hourly_where = "{$where_clause} AND {$hour_condition}";
    }

    $hourly_sql = "
      SELECT
        doh.work_hour as hour,
        SUM(doh.actual_output) as production
      FROM data_oee_rows_hourly doh
      LEFT JOIN info_machine m ON doh.machine_idx = m.idx
      {$hourly_where}" . (strpos($hourly_where, 'WHERE') !== false ? ' AND' : ' WHERE') . " m.status = 'Y'
      GROUP BY doh.work_hour
      ORDER BY doh.work_hour ASC
    ";

    $stmt = $pdo->prepare($hourly_sql);
    $stmt->execute($filter['params']);
    $hourly_raw = $stmt->fetchAll(PDO::FETCH_ASSOC);

    // 12시간 데이터 초기화
    $hourly_data = array_fill(0, 12, 0);
    foreach ($hourly_raw as $row) {
      $index = $row['hour'] - 8; // 8시를 0번 인덱스로
      if ($index >= 0 && $index < 12) {
        $hourly_data[$index] = (int)$row['production'];
      }
    }

    // 타임라인 데이터 생성 (시간별 OEE 상태 기반) - data_oee_rows_hourly 사용
    // OEE Trend와 동일한 로직 적용
    $startDate = $_GET['start_date'] ?? '';
    $endDate = $_GET['end_date'] ?? '';
    $isTimelineHourlyView = false;
    $timelineWorkHoursInfo = null;

    // 날짜 범위가 1일 이하인지 확인
    if (empty($startDate) && empty($endDate)) {
      $isTimelineHourlyView = true;
      $timeline_date = date('Y-m-d');
    } else if (!empty($startDate) && !empty($endDate)) {
      $start = new DateTime($startDate);
      $end = new DateTime($endDate);
      $startDateOnly = $start->format('Y-m-d');
      $endDateOnly = $end->format('Y-m-d');

      if ($startDateOnly === $endDateOnly) {
        $isTimelineHourlyView = true;
        $timeline_date = $endDateOnly;
      } else {
        $diff = $start->diff($end);
        $daysDiff = $diff->days;
        if ($daysDiff <= 1) {
          $isTimelineHourlyView = true;
          $timeline_date = $endDateOnly;
        } else {
          $isTimelineHourlyView = false;
        }
      }
    } else {
      $isTimelineHourlyView = true;
      $timeline_date = !empty($endDate) ? $endDate : (!empty($startDate) ? $startDate : date('Y-m-d'));
    }

    // 근무시간 정보 가져오기 (1일 이하일 때만)
    if ($isTimelineHourlyView) {
      $timelineWorkHoursInfo = getWorkHoursForDate($pdo, $timeline_date);
    }

    // 근무시간 범위 계산
    $work_start_hour = 0;
    $work_end_hour = 23;

    if ($timelineWorkHoursInfo && isset($timelineWorkHoursInfo['start_minutes']) && isset($timelineWorkHoursInfo['end_minutes'])) {
      $work_start_hour = floor($timelineWorkHoursInfo['start_minutes'] / 60);
      $work_end_hour = floor($timelineWorkHoursInfo['end_minutes'] / 60);
    }

    $work_hours_range = $work_end_hour - $work_start_hour + 1;

    // Timeline용 필터 생성
    $timeline_where_clauses = [];
    $timeline_params = [];

    // 날짜 필터
    if ($isTimelineHourlyView) {
      // 1일 이하: 특정 날짜만
      $timeline_where_clauses[] = 'work_date = ?';
      $timeline_params[] = $timeline_date;
    } else {
      // 1일 초과: 날짜 범위
      if (!empty($startDate) && !empty($endDate)) {
        $timeline_where_clauses[] = 'work_date BETWEEN ? AND ?';
        $timeline_params[] = $startDate;
        $timeline_params[] = $endDate;
      }
    }

    // 기존 필터 조건 추가 (factory, line, machine, shift)
    if (!empty($_GET['factory_filter'])) {
      $timeline_where_clauses[] = 'factory_idx = ?';
      $timeline_params[] = $_GET['factory_filter'];
    }
    if (!empty($_GET['line_filter'])) {
      $timeline_where_clauses[] = 'line_idx = ?';
      $timeline_params[] = $_GET['line_filter'];
    }
    if (!empty($_GET['machine_filter'])) {
      $timeline_where_clauses[] = 'machine_idx = ?';
      $timeline_params[] = $_GET['machine_filter'];
    }
    if (!empty($_GET['shift_filter'])) {
      $timeline_where_clauses[] = 'shift_idx = ?';
      $timeline_params[] = $_GET['shift_filter'];
    }

    $timeline_where = empty($timeline_where_clauses) ? '' : 'WHERE ' . implode(' AND ', $timeline_where_clauses);

    $timeline_sql = "
      SELECT
        doh.work_hour as hour,
        ROUND((AVG(doh.availabilty_rate) * (SUM(doh.actual_output) / NULLIF(SUM(doh.theoritical_output), 0)) * AVG(doh.quality_rate)) / 100, 2) as avg_oee,
        COUNT(*) as count
      FROM data_oee_rows_hourly doh
      LEFT JOIN info_machine m ON doh.machine_idx = m.idx
      {$timeline_where}" . (strpos($timeline_where, 'WHERE') !== false ? ' AND' : ' WHERE') . " m.status = 'Y'
      GROUP BY doh.work_hour
      ORDER BY doh.work_hour ASC
    ";

    $stmt = $pdo->prepare($timeline_sql);
    $stmt->execute($timeline_params);
    $timeline_raw = $stmt->fetchAll(PDO::FETCH_ASSOC);

    // 24시간 OEE 상태 배열 초기화 (야간 근무 시 work_end_hour가 23 초과 가능하므로 동적 크기)
    $hourly_oee = array_fill(0, max(24, $work_end_hour + 1), null);
    foreach ($timeline_raw as $row) {
      $hour = (int)$row['hour'];
      $hourly_oee[$hour] = round($row['avg_oee'], 1);
    }

    // 연속된 상태 구간을 세그먼트로 병합 (근무시간 범위만)
    $timeline_data = [];
    $current_segment = null;

    for ($hour = $work_start_hour; $hour <= $work_end_hour; $hour++) {
      $oee = $hourly_oee[$hour];

      // info_rate_color 테이블 기반 동적 색상 결정
      $colorStatus = getOeeColorAndStatus($oee, $rateColorConfig);
      $status = $colorStatus['status'];
      $color = $colorStatus['color'];
      $status_text = $colorStatus['status_text'];

      // 새 세그먼트 시작 또는 기존 세그먼트 확장
      if ($current_segment === null || $current_segment['status'] !== $status) {
        // 이전 세그먼트 저장
        if ($current_segment !== null) {
          $timeline_data[] = $current_segment;
        }

        // 새 세그먼트 시작
        $current_segment = [
          'status' => $status,
          'color' => $color,
          'status_text' => $status_text,
          'start_hour' => $hour,
          'end_hour' => $hour,
          'oee_values' => [$oee], // OEE 값들을 배열로 저장
          'width' => 0,
          'title' => ''
        ];
      } else {
        // 기존 세그먼트 확장
        $current_segment['end_hour'] = $hour;
        $current_segment['oee_values'][] = $oee; // OEE 값 추가
      }
    }

    // 마지막 세그먼트 저장
    if ($current_segment !== null) {
      $timeline_data[] = $current_segment;
    }

    // 각 세그먼트의 너비와 제목 계산
    foreach ($timeline_data as &$segment) {
      $start = $segment['start_hour'];
      $end = $segment['end_hour'];
      $duration = $end - $start + 1;

      // 평균 OEE 계산 (null 값 제외)
      $valid_oee_values = array_filter($segment['oee_values'], function($val) {
        return $val !== null;
      });

      $avg_oee = count($valid_oee_values) > 0
        ? round(array_sum($valid_oee_values) / count($valid_oee_values), 1)
        : null;

      // 근무시간 범위 기준으로 width 계산
      $segment['width'] = round(($duration / $work_hours_range) * 100, 2);

      // Title 생성: status_text와 OEE 값 포함
      if ($avg_oee !== null) {
        $segment['title'] = sprintf(
          "%s: OEE %s%% (%02d:00-%02d:59)",
          $segment['status_text'],
          $avg_oee,
          $start,
          $end
        );
      } else {
        $segment['title'] = sprintf(
          "No data (%02d:00-%02d:59)",
          $start,
          $end
        );
      }

      // 불필요한 필드 제거
      unset($segment['status']);
      unset($segment['status_text']);
      unset($segment['start_hour']);
      unset($segment['end_hour']);
      unset($segment['oee_values']);
    }
    unset($segment);
    
    // 히트맵 데이터 생성 (필터 기준 최근 6 평일+토 x 24시간)
    // data_oee_rows_hourly 테이블 사용, OEE Timeline과 동일한 색상 로직 적용
    // 필터 end_date(또는 start_date) 기준으로 최근 7일 조회 — CURDATE() 하드코딩 제거
    $heatmap_base = !empty($_GET['end_date']) ? $_GET['end_date']
                  : (!empty($_GET['start_date']) ? $_GET['start_date'] : date('Y-m-d'));

    $heatmap_sql = "
      SELECT
        doh.work_date,
        doh.work_hour,
        ROUND((AVG(doh.availabilty_rate) * (SUM(doh.actual_output) / NULLIF(SUM(doh.theoritical_output), 0)) * AVG(doh.quality_rate)) / 100, 2) as avg_oee
      FROM data_oee_rows_hourly doh
      LEFT JOIN info_machine m ON doh.machine_idx = m.idx
      WHERE doh.work_date >= DATE_SUB(?, INTERVAL 7 DAY)
        AND doh.work_date <= ?
        AND WEEKDAY(doh.work_date) BETWEEN 0 AND 5
        AND m.status = 'Y'
      GROUP BY doh.work_date, doh.work_hour
      ORDER BY doh.work_date ASC, doh.work_hour ASC
    ";

    $stmt = $pdo->prepare($heatmap_sql);
    $stmt->execute([$heatmap_base, $heatmap_base]);
    $heatmap_raw = $stmt->fetchAll(PDO::FETCH_ASSOC);

    // 히트맵 데이터 초기화 (6일 x 24시간)
    // 각 셀은 { oee: null|float, color: string, status_text: string } 형태
    $heatmap_data = [];
    for ($day = 0; $day < 6; $day++) {
      $heatmap_data[$day] = [];
      for ($hour = 0; $hour < 24; $hour++) {
        $heatmap_data[$day][$hour] = [
          'oee' => null,
          'color' => '#e0e0e0',
          'status_text' => 'No data'
        ];
      }
    }

    // 최근 6일간의 평일+토요일 날짜 생성 (월~토)
    $weekday_map = [];
    $weekday_labels = []; // 실제 요일 레이블 저장
    $weekday_count = 0;
    $day_name_map = ['Mon', 'Tue', 'Wed', 'Thu', 'Fri', 'Sat', 'Sun'];

    for ($i = 6; $i >= 0 && $weekday_count < 6; $i--) {
      $date = date('Y-m-d', strtotime($heatmap_base . " -{$i} days"));
      $day_of_week = date('N', strtotime($date)); // 1=월요일, 7=일요일

      // 월~토 (일요일 제외)
      if ($day_of_week >= 1 && $day_of_week <= 6) {
        $weekday_map[$date] = $weekday_count;
        $weekday_labels[] = $day_name_map[$day_of_week - 1]; // 실제 요일 이름 저장
        $weekday_count++;
      }
    }

    // 실제 데이터로 히트맵 채우기
    foreach ($heatmap_raw as $row) {
      $date = $row['work_date'];
      $hour = (int)$row['work_hour'];
      $oee = $row['avg_oee'] !== null ? (float)$row['avg_oee'] : null;

      // 날짜가 매핑에 있는지 확인
      if (!isset($weekday_map[$date])) continue;

      $day_index = $weekday_map[$date];

      // 시간은 0~23 범위
      if ($hour < 0 || $hour > 23) continue;

      // OEE 값에 따른 색상 계산 (Timeline과 동일한 로직)
      $colorStatus = getOeeColorAndStatus($oee, $rateColorConfig);

      $heatmap_data[$day_index][$hour] = [
        'oee' => $oee,
        'color' => $colorStatus['color'],
        'status_text' => $colorStatus['status_text']
      ];
    }

    return [
      'oee_trend' => getOeeTrendData($pdo, $filter), // OEE Trend 추가
      'timeline' => $timeline_data,
      'heatmap' => $heatmap_data,
      'heatmap_labels' => $weekday_labels, // 실제 요일 레이블 추가
      'rate_color_config' => $rateColorConfig // Rate color 설정 추가
    ];

  } catch (Exception $e) {
    error_log("Production 데이터 조회 오류: " . $e->getMessage());
    return ['error' => $e->getMessage()];
  }
}

/**
 * 통합 Dashboard 데이터 조회 및 전송
 */
function sendDashboardData($pdo) {
  // 각 테이블에 맞는 alias로 필터 생성
  $oeeFilter = parseDashboardFilterParams(); // data_oee는 alias 없이 사용
  $andonFilter = parseDashboardFilterParams('da'); // data_andon은 'da' alias 사용
  $downtimeFilter = parseDashboardFilterParams('dd'); // data_downtime은 'dd' alias 사용
  $defectiveFilter = parseDashboardFilterParams('dd'); // data_defective는 'dd' alias 사용
  $productionFilter = parseDashboardFilterParams(); // data_oee는 alias 없이 사용

  $dashboard_data = [
    'timestamp' => time(),
    'oee' => getOEEData($pdo, $oeeFilter),
    'andon' => getAndonData($pdo, $andonFilter),
    'downtime' => getDowntimeData($pdo, $downtimeFilter),
    'defective' => getDefectiveData($pdo, $defectiveFilter),
    'production' => getProductionData($pdo, $productionFilter)
  ];

  // SSE 형식으로 데이터 전송
  echo "data: " . json_encode($dashboard_data) . "\n\n";

  if (ob_get_level()) {
    ob_flush();
  }
  flush();
}

// 연결 유지 및 데이터 전송 루프
$last_update = 0;
$update_interval = 5; // 5초마다 업데이트

while (true) {
  // 클라이언트 연결 확인
  if (connection_aborted()) {
    break;
  }
  
  $current_time = time();
  
  // 업데이트 간격 확인
  if ($current_time - $last_update >= $update_interval) {
    sendDashboardData($pdo);
    $last_update = $current_time;
  }
  
  // CPU 부하 방지
  sleep(1);
}

// 연결 종료 시 정리
if ($pdo) {
  $pdo = null;
}
?>