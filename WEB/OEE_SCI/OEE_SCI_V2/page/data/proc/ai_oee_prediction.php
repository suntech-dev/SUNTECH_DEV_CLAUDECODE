<?php
/**
 * AI OEE Prediction API
 * 과거 데이터 기반 향후 4시간 OEE 예측
 *
 * Method: GET
 * Params:
 *   factory_filter, line_filter, machine_filter (optional)
 * Response:
 *   { code, current_oee, forecast: [{hour, oee, lower, upper}], trend, method }
 */


require_once(__DIR__ . '/../../../lib/db.php');
require_once(__DIR__ . '/../../../lib/statistics.lib.php');

header('Content-Type: application/json');
header('Cache-Control: no-cache');

$factory_filter = isset($_GET['factory_filter']) ? trim($_GET['factory_filter']) : '';
$line_filter    = isset($_GET['line_filter'])    ? trim($_GET['line_filter'])    : '';
$machine_filter = isset($_GET['machine_filter']) ? trim($_GET['machine_filter']) : '';
$date_range     = isset($_GET['date_range'])     ? trim($_GET['date_range'])     : 'today';

// date_range → SQL 날짜 조건 (data_oee_rows_hourly alias: doh)
switch ($date_range) {
  case 'yesterday':
    $actual_date_where = "doh.work_date = DATE_SUB(CURDATE(), INTERVAL 1 DAY)";
    break;
  case '7d':
    $actual_date_where = "doh.work_date >= DATE_SUB(CURDATE(), INTERVAL 6 DAY)";
    break;
  case '30d':
    $actual_date_where = "doh.work_date >= DATE_SUB(CURDATE(), INTERVAL 29 DAY)";
    break;
  default:
    $actual_date_where = "doh.work_date = CURDATE()";
}

// 필터 조건 구성 (data_oee_rows_hourly 는 독립 테이블 - JOIN 불필요)
$where_parts = [];
$params      = [];

if ($factory_filter !== '') {
  $where_parts[] = 'doh.factory_idx = ?';
  $params[]      = $factory_filter;
}
if ($line_filter !== '') {
  $where_parts[] = 'doh.line_idx = ?';
  $params[]      = $line_filter;
}
if ($machine_filter !== '') {
  $where_parts[] = 'doh.machine_idx = ?';
  $params[]      = $machine_filter;
}

$where_sql = $where_parts ? 'AND ' . implode(' AND ', $where_parts) : '';

try {
  // 1. 과거 30일 시간대별 OEE 데이터 수집 (data_oee_rows_hourly 독립 테이블)
  $sql_history = "
    SELECT
      DAYOFWEEK(doh.work_date) AS dow,
      doh.work_hour            AS hour,
      AVG(doh.oee)             AS avg_oee,
      STDDEV(doh.oee)          AS std_oee,
      COUNT(*)                 AS cnt
    FROM data_oee_rows_hourly doh
    WHERE doh.work_date >= DATE_SUB(CURDATE(), INTERVAL 30 DAY)
      AND doh.oee IS NOT NULL
      $where_sql
    GROUP BY DAYOFWEEK(doh.work_date), doh.work_hour
    ORDER BY dow, hour
  ";
  $stmt = $pdo->prepare($sql_history);
  $stmt->execute($params);
  $history = $stmt->fetchAll(PDO::FETCH_ASSOC);

  // 요일×시간대 매트릭스 구성
  $matrix = [];
  foreach ($history as $row) {
    $key = $row['dow'] . '_' . $row['hour'];
    $matrix[$key] = [
      'avg' => (float)$row['avg_oee'],
      'std' => (float)$row['std_oee'],
      'cnt' => (int)$row['cnt'],
    ];
  }

  // 2. 오늘 현재까지의 시간대별 실제 OEE
  $sql_today = "
    SELECT
      doh.work_hour AS hour,
      AVG(doh.oee)  AS avg_oee
    FROM data_oee_rows_hourly doh
    WHERE $actual_date_where
      AND doh.oee IS NOT NULL
      $where_sql
    GROUP BY doh.work_hour
    ORDER BY doh.work_hour
  ";
  $stmt2 = $pdo->prepare($sql_today);
  $stmt2->execute($params);
  $today_rows = $stmt2->fetchAll(PDO::FETCH_ASSOC);

  $today_data  = [];
  $today_hours = [];
  foreach ($today_rows as $row) {
    $today_hours[] = (int)$row['hour'];
    $today_data[]  = (float)$row['avg_oee'];
  }

  // 현재 OEE (가장 최근 시간대)
  $current_oee = !empty($today_data) ? round(end($today_data), 1) : null;
  $current_hour = !empty($today_hours) ? end($today_hours) : (int)date('G');

  // 3. 예측 시간대 계산 (현재 시간 이후 4시간)
  $forecast_hours = [];
  for ($i = 1; $i <= 4; $i++) {
    $forecast_hours[] = ($current_hour + $i) % 24;
  }

  $dow = (int)date('w') + 1; // PHP date('w'): 0=Sun, MySQL DAYOFWEEK: 1=Sun

  // 4. 예측값 계산
  $forecast = [];

  // 지수평활법으로 단기 트렌드 추출
  $es_result = !empty($today_data) ? exponentialSmoothing($today_data, 0.3, 4) : null;
  $es_forecast = $es_result ? $es_result['forecast'] : null;

  foreach ($forecast_hours as $idx => $hour) {
    $key = $dow . '_' . $hour;

    $seasonal_avg = isset($matrix[$key]) ? $matrix[$key]['avg'] : null;
    $seasonal_std = isset($matrix[$key]) ? $matrix[$key]['std'] : 5.0;

    // 가중 평균: 계절성 70% + 지수평활 30%
    if ($seasonal_avg !== null && $es_forecast !== null) {
      $pred_oee = $seasonal_avg * 0.7 + $es_forecast[$idx] * 0.3;
    } elseif ($seasonal_avg !== null) {
      $pred_oee = $seasonal_avg;
    } elseif ($es_forecast !== null) {
      $pred_oee = $es_forecast[$idx];
    } else {
      $pred_oee = $current_oee ?? 70.0;
    }

    $pred_oee = round(max(0, min(100, $pred_oee)), 1);
    $ci = calcConfidenceInterval($pred_oee, max(1.0, $seasonal_std), 1.645); // 90% CI

    $label = sprintf('%02d:00', $hour);
    $forecast[] = [
      'hour'      => $hour,
      'label'     => $label,
      'oee'       => $pred_oee,
      'lower'     => $ci['lower'],
      'upper'     => $ci['upper'],
      'data_cnt'  => isset($matrix[$key]) ? $matrix[$key]['cnt'] : 0,
    ];
  }

  // 5. 트렌드 방향 판단
  $trend = 'stable';
  if (!empty($today_data) && count($today_data) >= 3) {
    $lr = linearRegression(array_slice($today_data, -6), 1);
    if ($lr['slope'] > 0.5)      $trend = 'up';
    elseif ($lr['slope'] < -0.5) $trend = 'down';
  }

  echo json_encode([
    'code'         => '00',
    'current_oee'  => $current_oee,
    'current_hour' => $current_hour,
    'forecast'     => $forecast,
    'trend'        => $trend,
    'method'       => 'exponential_smoothing + seasonal',
    'data_days'    => 30,
  ]);

} catch (PDOException $e) {
  echo json_encode(['code' => '99', 'msg' => 'DB error: ' . $e->getMessage()]);
}
