<?php
/**
 * AI Predictive Maintenance API
 * 머신별 다운타임 패턴 분석 → 위험도 스코어 산출
 *
 * Method: GET
 * Params:
 *   factory_filter, line_filter, machine_filter (optional)
 *   limit: 반환할 위험 기계 수 (default: 10)
 * Response:
 *   { code, machines: [{machine_no, line_name, risk_score, risk_level, details}] }
 */


require_once(__DIR__ . '/../../../lib/db.php');
require_once(__DIR__ . '/../../../lib/statistics.lib.php');

header('Content-Type: application/json');
header('Cache-Control: no-cache');

$factory_filter = isset($_GET['factory_filter']) ? trim($_GET['factory_filter']) : '';
$line_filter    = isset($_GET['line_filter'])    ? trim($_GET['line_filter'])    : '';
$machine_filter = isset($_GET['machine_filter']) ? trim($_GET['machine_filter']) : '';
$limit          = isset($_GET['limit'])          ? max(1, min(50, (int)$_GET['limit'])) : 10;
$date_range     = isset($_GET['date_range'])     ? trim($_GET['date_range'])     : 'today';

// date_range → OEE 조회 날짜 조건
switch ($date_range) {
  case 'yesterday':
    $oee_date_where = "work_date = DATE_SUB(CURDATE(), INTERVAL 1 DAY)";
    break;
  case '7d':
    $oee_date_where = "work_date >= DATE_SUB(CURDATE(), INTERVAL 6 DAY)";
    break;
  case '30d':
    $oee_date_where = "work_date >= DATE_SUB(CURDATE(), INTERVAL 29 DAY)";
    break;
  default:
    $oee_date_where = "work_date >= DATE_SUB(CURDATE(), INTERVAL 6 DAY)"; // today → 7일 기본
}

$where_parts = [];
$params      = [];

if ($factory_filter !== '') {
  $where_parts[] = 'im.factory_idx = ?';
  $params[]      = $factory_filter;
}
if ($line_filter !== '') {
  $where_parts[] = 'im.line_idx = ?';
  $params[]      = $line_filter;
}
if ($machine_filter !== '') {
  $where_parts[] = 'im.idx = ?';
  $params[]      = $machine_filter;
}

$where_sql = $where_parts ? 'WHERE ' . implode(' AND ', $where_parts) : '';

try {
  // 1. 등록된 모든 기계 목록
  $sql_machines = "
    SELECT im.idx, im.machine_no, il.line_name, il.idx AS line_idx
    FROM info_machine im
    JOIN info_line il ON im.line_idx = il.idx
    $where_sql
    ORDER BY il.line_name, im.machine_no
  ";
  $stmt = $pdo->prepare($sql_machines);
  $stmt->execute($params);
  $machines = $stmt->fetchAll(PDO::FETCH_ASSOC);

  if (empty($machines)) {
    echo json_encode(['code' => '00', 'machines' => [], 'summary' => ['total' => 0]]);
    exit;
  }

  $machine_ids  = array_column($machines, 'idx');
  $machine_map  = [];
  foreach ($machines as $m) $machine_map[$m['idx']] = $m;

  // 2. 최근 90일 머신별 다운타임 이력 집계
  // data_downtime 은 독립 테이블 — machine_idx, work_date, reg_date, duration_sec 직접 보유
  $placeholders = implode(',', array_fill(0, count($machine_ids), '?'));

  $sql_downtime = "
    SELECT
      dd.machine_idx,
      COUNT(*)                         AS total_dt_count,
      SUM(dd.duration_sec / 60)        AS total_dt_min,
      AVG(dd.duration_sec / 60)        AS avg_dt_min,
      MAX(dd.reg_date)                 AS last_dt_time,
      SUM(CASE WHEN dd.work_date >= DATE_SUB(CURDATE(), INTERVAL 7 DAY) THEN 1 ELSE 0 END)  AS recent_7d_cnt,
      SUM(CASE WHEN dd.work_date >= DATE_SUB(CURDATE(), INTERVAL 30 DAY) THEN 1 ELSE 0 END) AS recent_30d_cnt,
      SUM(CASE WHEN dd.work_date BETWEEN DATE_SUB(CURDATE(), INTERVAL 60 DAY)
                                     AND DATE_SUB(CURDATE(), INTERVAL 31 DAY) THEN 1 ELSE 0 END) AS prev_30d_cnt
    FROM data_downtime dd
    WHERE dd.machine_idx IN ($placeholders)
      AND dd.work_date >= DATE_SUB(CURDATE(), INTERVAL 90 DAY)
    GROUP BY dd.machine_idx
  ";
  $stmt2 = $pdo->prepare($sql_downtime);
  $stmt2->execute($machine_ids);
  $dt_stats = $stmt2->fetchAll(PDO::FETCH_ASSOC);

  $dt_map = [];
  foreach ($dt_stats as $dt) $dt_map[$dt['machine_idx']] = $dt;

  // 3. 현재 가동 시간 (마지막 다운타임 이후 경과 시간)
  $sql_runtime = "
    SELECT
      dd.machine_idx,
      MAX(dd.reg_date) AS last_dt_time,
      TIMESTAMPDIFF(HOUR, MAX(dd.reg_date), NOW()) AS hours_since_last_dt
    FROM data_downtime dd
    WHERE dd.machine_idx IN ($placeholders)
    GROUP BY dd.machine_idx
  ";
  $stmt3 = $pdo->prepare($sql_runtime);
  $stmt3->execute($machine_ids);
  $runtime_data = $stmt3->fetchAll(PDO::FETCH_ASSOC);

  $runtime_map = [];
  foreach ($runtime_data as $r) $runtime_map[$r['machine_idx']] = $r;

  // 4. OEE 트렌드 (최근 7일 평균 OEE)
  $sql_oee = "
    SELECT
      machine_idx,
      AVG(oee)        AS avg_oee_7d,
      MIN(oee)        AS min_oee_7d,
      STDDEV(oee)     AS std_oee_7d
    FROM data_oee
    WHERE machine_idx IN ($placeholders)
      AND $oee_date_where
    GROUP BY machine_idx
  ";
  $stmt4 = $pdo->prepare($sql_oee);
  $stmt4->execute($machine_ids);
  $oee_stats = $stmt4->fetchAll(PDO::FETCH_ASSOC);

  $oee_map = [];
  foreach ($oee_stats as $o) $oee_map[$o['machine_idx']] = $o;

  // 5. 위험도 스코어 계산
  $results = [];
  foreach ($machines as $m) {
    $mid = $m['idx'];
    $dt  = $dt_map[$mid]    ?? null;
    $rt  = $runtime_map[$mid] ?? null;
    $oe  = $oee_map[$mid]   ?? null;

    $score_a = 0; // 가동 시간 위험도 (가중치 40%)
    $score_b = 0; // 다운타임 빈도 증가율 (가중치 35%)
    $score_c = 0; // OEE 불안정도 (가중치 25%)

    $details = [];

    // --- 가중치 A: 런타임 위험도 ---
    if ($dt && $rt) {
      $avg_interval_hrs = 0;
      if ($dt['total_dt_count'] > 1) {
        // 90일 / 총 다운타임 건수 = 평균 고장 간격(일) → 시간 변환
        $avg_interval_hrs = (90 * 24) / (int)$dt['total_dt_count'];
      }

      $hours_since = (float)($rt['hours_since_last_dt'] ?? 0);

      if ($avg_interval_hrs > 0) {
        $ratio = $hours_since / $avg_interval_hrs;
        $score_a = min(100, $ratio * 100);
        $details['runtime_ratio']       = round($ratio, 2);
        $details['avg_interval_hrs']    = round($avg_interval_hrs, 1);
        $details['hours_since_last_dt'] = round($hours_since, 1);
      }
    }

    // --- 가중치 B: 다운타임 빈도 증가율 ---
    if ($dt) {
      $recent  = (int)($dt['recent_30d_cnt']  ?? 0);
      $prev    = (int)($dt['prev_30d_cnt']    ?? 0);

      if ($prev > 0) {
        $increase_rate = ($recent - $prev) / $prev;
        $score_b = min(100, max(0, $increase_rate * 100));
      } elseif ($recent > 0) {
        // 이전 기간 없고 최근에만 있으면 중간 위험
        $score_b = 50;
      }

      $details['recent_30d_cnt'] = $recent;
      $details['prev_30d_cnt']   = $prev;
    }

    // --- 가중치 C: OEE 불안정도 ---
    if ($oe) {
      $avg_oee = (float)($oe['avg_oee_7d'] ?? 85);
      $std_oee = (float)($oe['std_oee_7d'] ?? 0);
      $min_oee = (float)($oe['min_oee_7d'] ?? 85);

      // OEE 낮을수록, 변동성 클수록 위험
      $oee_risk   = max(0, (85 - $avg_oee) / 85 * 100);
      $stdev_risk = min(100, $std_oee * 3);
      $score_c    = min(100, ($oee_risk * 0.6 + $stdev_risk * 0.4));

      $details['avg_oee_7d'] = round($avg_oee, 1);
      $details['std_oee_7d'] = round($std_oee, 1);
      $details['min_oee_7d'] = round($min_oee, 1);
    }

    // 가중 합산
    $risk_score = round($score_a * 0.40 + $score_b * 0.35 + $score_c * 0.25, 1);

    $risk_level = 'normal';
    if ($risk_score >= 80)     $risk_level = 'danger';
    elseif ($risk_score >= 50) $risk_level = 'warning';

    $results[] = [
      'machine_idx'  => $mid,
      'machine_no'   => $m['machine_no'],
      'line_name'    => $m['line_name'],
      'risk_score'   => $risk_score,
      'risk_level'   => $risk_level,
      'score_detail' => [
        'runtime'    => round($score_a, 1),
        'frequency'  => round($score_b, 1),
        'oee_quality' => round($score_c, 1),
      ],
      'details'      => $details,
    ];
  }

  // 위험도 내림차순 정렬
  usort($results, fn($a, $b) => $b['risk_score'] <=> $a['risk_score']);

  $danger_count  = count(array_filter($results, fn($r) => $r['risk_level'] === 'danger'));
  $warning_count = count(array_filter($results, fn($r) => $r['risk_level'] === 'warning'));

  echo json_encode([
    'code'     => '00',
    'machines' => array_slice($results, 0, $limit),
    'summary'  => [
      'total'   => count($results),
      'danger'  => $danger_count,
      'warning' => $warning_count,
      'normal'  => count($results) - $danger_count - $warning_count,
    ],
  ]);

} catch (PDOException $e) {
  echo json_encode(['code' => '99', 'msg' => 'DB error: ' . $e->getMessage()]);
}
