<?php
/**
 * AI OEE Prediction API v5
 * v4 대비 수정:
 *  - current_oee: date_range 무관하게 항상 오늘(CURDATE()) 기준 조회
 *  - current_oee: min(100, max(0, ...)) 클램핑 적용
 *  - today_data: 오늘 시간대별 실제 OEE 배열 반환 (차트 Actual 라인용)
 *  - CI 계산 시 seasonal_std 상한 15%로 제한 (0~100% CI 방지)
 *  - date_range: 예측 패턴 학습용 히스토리 기간에만 영향
 */

require_once(__DIR__ . '/../../../lib/db.php');
require_once(__DIR__ . '/../../../lib/statistics.lib.php');

header('Content-Type: application/json');
header('Cache-Control: no-cache');

$factory_filter = isset($_GET['factory_filter']) ? trim($_GET['factory_filter']) : '';
$line_filter    = isset($_GET['line_filter'])    ? trim($_GET['line_filter'])    : '';
$machine_filter = isset($_GET['machine_filter']) ? trim($_GET['machine_filter']) : '';
$date_range     = isset($_GET['date_range'])     ? trim($_GET['date_range'])     : 'today';

// date_range → 히스토리 학습 기간 (예측 패턴에만 적용, current_oee는 영향 없음)
switch ($date_range) {
  case 'yesterday':
    $history_days = 30; // 어제 기반이어도 패턴은 30일 학습
    break;
  case '7d':
    $history_days = 7;
    break;
  case '30d':
    $history_days = 30;
    break;
  default:
    $history_days = 30;
}

$where_parts = [];
$params      = [];

if ($factory_filter !== '') { $where_parts[] = 'doh.factory_idx = ?'; $params[] = $factory_filter; }
if ($line_filter    !== '') { $where_parts[] = 'doh.line_idx = ?';    $params[] = $line_filter; }
if ($machine_filter !== '') { $where_parts[] = 'doh.machine_idx = ?'; $params[] = $machine_filter; }

$where_sql = $where_parts ? 'AND ' . implode(' AND ', $where_parts) : '';

try {
  // 1. 히스토리 기반 패턴 학습 (요일×시간대 매트릭스)
  $sql_history = "
    SELECT
      DAYOFWEEK(doh.work_date) AS dow,
      doh.work_hour            AS hour,
      AVG(LEAST(100, GREATEST(0, doh.oee))) AS avg_oee,
      STDDEV(LEAST(100, GREATEST(0, doh.oee))) AS std_oee,
      COUNT(*) AS cnt
    FROM data_oee_rows_hourly doh
    WHERE doh.work_date >= DATE_SUB(CURDATE(), INTERVAL ? DAY)
      AND doh.oee IS NOT NULL
      $where_sql
    GROUP BY DAYOFWEEK(doh.work_date), doh.work_hour
    ORDER BY dow, hour
  ";
  $stmt = $pdo->prepare($sql_history);
  $stmt->execute(array_merge([$history_days], $params));
  $history = $stmt->fetchAll(PDO::FETCH_ASSOC);

  $matrix = [];
  foreach ($history as $row) {
    $key = $row['dow'] . '_' . $row['hour'];
    $matrix[$key] = [
      'avg' => (float)$row['avg_oee'],
      'std' => (float)$row['std_oee'],
      'cnt' => (int)$row['cnt'],
    ];
  }

  // 2. 오늘 실제 OEE (항상 CURDATE() 고정 — date_range 영향 없음)
  $sql_today = "
    SELECT
      doh.work_hour AS hour,
      AVG(LEAST(100, GREATEST(0, doh.oee))) AS avg_oee
    FROM data_oee_rows_hourly doh
    WHERE doh.work_date = CURDATE()
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
  $today_oees  = [];
  foreach ($today_rows as $row) {
    $hour = (int)$row['hour'];
    $oee  = round((float)$row['avg_oee'], 1);
    $today_hours[] = $hour;
    $today_oees[]  = $oee;
    $today_data[]  = ['hour' => $hour, 'label' => sprintf('%02d:00', $hour), 'oee' => $oee];
  }

  // current_oee: 오늘 가장 최근 시간대 값, 클램핑 적용
  $current_oee  = !empty($today_oees) ? min(100.0, max(0.0, round(end($today_oees), 1))) : null;
  $current_hour = !empty($today_hours) ? end($today_hours) : (int)date('G');

  // 3. 예측 시간대 (현재 시간 이후 4시간)
  $forecast_hours = [];
  for ($i = 1; $i <= 4; $i++) {
    $forecast_hours[] = ($current_hour + $i) % 24;
  }

  $dow = (int)date('w') + 1;

  // 4. 예측값 계산
  $forecast   = [];
  $es_result  = !empty($today_oees) ? exponentialSmoothing($today_oees, 0.3, 4) : null;
  $es_forecast = $es_result ? $es_result['forecast'] : null;

  foreach ($forecast_hours as $idx => $hour) {
    $key = $dow . '_' . $hour;

    $seasonal_avg = isset($matrix[$key]) ? $matrix[$key]['avg'] : null;
    // std 상한 15% — CI가 0~100% 전 구간이 되는 현상 방지
    $seasonal_std = isset($matrix[$key]) ? min(15.0, $matrix[$key]['std']) : 5.0;

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

    $forecast[] = [
      'hour'     => $hour,
      'label'    => sprintf('%02d:00', $hour),
      'oee'      => $pred_oee,
      'lower'    => $ci['lower'],
      'upper'    => $ci['upper'],
      'data_cnt' => isset($matrix[$key]) ? $matrix[$key]['cnt'] : 0,
    ];
  }

  // 5. 트렌드 방향
  $trend = 'stable';
  if (!empty($today_oees) && count($today_oees) >= 3) {
    $lr = linearRegression(array_slice($today_oees, -6), 1);
    if ($lr['slope'] > 0.5)      $trend = 'up';
    elseif ($lr['slope'] < -0.5) $trend = 'down';
  }

  echo json_encode([
    'code'         => '00',
    'current_oee'  => $current_oee,
    'current_hour' => $current_hour,
    'today_data'   => $today_data,   // v5 신규: 오늘 실제 OEE 배열
    'forecast'     => $forecast,
    'trend'        => $trend,
    'method'       => 'exponential_smoothing + seasonal_v5',
    'history_days' => $history_days,
  ]);

} catch (PDOException $e) {
  echo json_encode(['code' => '99', 'msg' => 'DB error: ' . $e->getMessage()]);
}
