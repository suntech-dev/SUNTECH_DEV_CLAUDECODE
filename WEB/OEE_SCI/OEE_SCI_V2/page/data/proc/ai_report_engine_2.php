<?php

/**
 * F11 — AI Report Engine API
 * Claude API 없이 규칙 기반 텍스트 인사이트 생성
 * OEE 요약 / 이상 감지 / 유지보수 위험 / 최적화 통합 반환
 *
 * Method: GET
 * Params:
 *   factory_filter, line_filter, machine_filter (optional)
 *   range: today|yesterday|1w|1m|custom (default: today)
 *   date_from, date_to: custom range (YYYY-MM-DD)
 * Response:
 *   { code, period, summary, prediction, anomalies, maintenance, optimization, insights }
 */

require_once(__DIR__ . '/../../../lib/db.php');
require_once(__DIR__ . '/../../../lib/statistics.lib.php');

header('Content-Type: application/json; charset=utf-8');
header('Cache-Control: no-cache');

$factory = isset($_GET['factory_filter']) ? trim($_GET['factory_filter']) : '';
$line    = isset($_GET['line_filter'])    ? trim($_GET['line_filter'])    : '';
$machine = isset($_GET['machine_filter']) ? trim($_GET['machine_filter']) : '';
$range   = isset($_GET['range'])          ? trim($_GET['range'])          : 'today';
$d_from  = isset($_GET['date_from'])      ? trim($_GET['date_from'])      : '';
$d_to    = isset($_GET['date_to'])        ? trim($_GET['date_to'])        : '';
$lang    = in_array($_GET['lang'] ?? 'en', ['ko', 'en']) ? $_GET['lang'] : 'en';

// ── 날짜 범위 계산 ─────────────────────────────────────
$today = date('Y-m-d');
switch ($range) {
    case 'yesterday':
        $from = $to = date('Y-m-d', strtotime('-1 day'));
        break;
    case '1w':
        $from = date('Y-m-d', strtotime('-7 days'));
        $to = $today;
        break;
    case '1m':
        $from = date('Y-m-d', strtotime('-30 days'));
        $to = $today;
        break;
    case 'custom':
        $from = ($d_from && preg_match('/^\d{4}-\d{2}-\d{2}$/', $d_from)) ? $d_from : $today;
        $to   = ($d_to   && preg_match('/^\d{4}-\d{2}-\d{2}$/', $d_to))   ? $d_to   : $today;
        break;
    default:
        $from = $to = $today;
}

// ── 필터 SQL ───────────────────────────────────────────
$fp = [];
$fv = [];
if ($factory !== '') {
    $fp[] = 'do.factory_idx = ?';
    $fv[] = $factory;
}
if ($line    !== '') {
    $fp[] = 'do.line_idx = ?';
    $fv[] = $line;
}
if ($machine !== '') {
    $fp[] = 'do.machine_idx = ?';
    $fv[] = $machine;
}
$f_sql = $fp ? 'AND ' . implode(' AND ', $fp) : '';

try {
    // ── 1. OEE Summary ─────────────────────────────────
    $stmt = $pdo->prepare("
    SELECT
      ROUND(AVG(do.oee), 1)              AS avg_oee,
      ROUND(AVG(do.availabilty_rate), 1) AS avg_avail,
      ROUND(AVG(do.productivity_rate), 1) AS avg_perf,
      ROUND(AVG(do.quality_rate), 1)     AS avg_quality,
      COUNT(DISTINCT do.machine_idx)     AS machine_count,
      COUNT(*)                           AS record_count
    FROM data_oee do
    WHERE do.work_date BETWEEN ? AND ?
      $f_sql
  ");
    $stmt->execute(array_merge([$from, $to], $fv));
    $summary = $stmt->fetch(PDO::FETCH_ASSOC);

    if (!$summary || (int)$summary['record_count'] === 0) {
        echo json_encode(['code' => '99', 'msg' => 'No OEE data for selected period'], JSON_UNESCAPED_UNICODE);
        exit;
    }

    // ── 2. 다운타임 집계 ────────────────────────────────
    $dt_fp = [];
    $dt_fv = [$from, $to];
    if ($factory !== '') {
        $dt_fp[] = 'dd.factory_idx = ?';
        $dt_fv[] = $factory;
    }
    if ($line    !== '') {
        $dt_fp[] = 'dd.line_idx = ?';
        $dt_fv[] = $line;
    }
    if ($machine !== '') {
        $dt_fp[] = 'dd.machine_idx = ?';
        $dt_fv[] = $machine;
    }
    $dt_sql = $dt_fp ? 'AND ' . implode(' AND ', $dt_fp) : '';

    $stmt = $pdo->prepare("
    SELECT
      COUNT(*) AS dt_count,
      ROUND(COALESCE(SUM(duration_sec / 60), 0), 1) AS dt_total_min,
      ROUND(COALESCE(AVG(duration_sec / 60), 0), 1) AS dt_avg_min
    FROM data_downtime dd
    WHERE dd.work_date BETWEEN ? AND ?
      $dt_sql
  ");
    $stmt->execute($dt_fv);
    $downtime = $stmt->fetch(PDO::FETCH_ASSOC);

    // ── 3. OEE 추세 + 예측 ──────────────────────────────
    $stmt = $pdo->prepare("
    SELECT work_date, ROUND(AVG(do.oee), 1) AS daily_oee
    FROM data_oee do
    WHERE do.work_date BETWEEN DATE_SUB(?, INTERVAL 30 DAY) AND ?
      $f_sql
    GROUP BY work_date
    ORDER BY work_date ASC
  ");
    $stmt->execute(array_merge([$from, $to], $fv));
    $trend_rows = $stmt->fetchAll(PDO::FETCH_ASSOC);
    $oee_series = array_map('floatval', array_column($trend_rows, 'daily_oee'));

    $prediction = [];
    if (count($oee_series) >= 3) {
        $es  = exponentialSmoothing($oee_series, 0.3, 7);
        $reg = linearRegression($oee_series, 30);
        $forecast_7d  = max(0, min(100, round(end($es['forecast']), 1)));
        $forecast_30d = max(0, min(100, round(end($oee_series) + $reg['slope'] * 30, 1)));
        $prediction = [
            '7d'    => $forecast_7d,
            '30d'   => $forecast_30d,
            'trend' => $reg['slope'] > 0.1 ? 'improving' : ($reg['slope'] < -0.1 ? 'declining' : 'stable'),
            'slope' => round($reg['slope'], 3),
            'dates' => array_column($trend_rows, 'work_date'),
            'actuals' => $oee_series,
        ];
    }

    // ── 4. 이상 감지 (Z-Score, 오늘 기준) ─────────────
    $stmt = $pdo->prepare("
    SELECT do.machine_idx, im.machine_no, il.line_name,
      AVG(do.oee)              AS mean_oee,
      STDDEV(do.oee)           AS std_oee,
      AVG(do.availabilty_rate) AS mean_avail,
      STDDEV(do.availabilty_rate) AS std_avail
    FROM data_oee do
    JOIN info_machine im ON do.machine_idx = im.idx
    JOIN info_line    il ON do.line_idx = il.idx
    WHERE do.work_date BETWEEN DATE_SUB(CURDATE(), INTERVAL 30 DAY) AND DATE_SUB(CURDATE(), INTERVAL 1 DAY)
      $f_sql
    GROUP BY do.machine_idx, im.machine_no, il.line_name
    HAVING COUNT(*) >= 3
  ");
    $stmt->execute($fv);
    $baselines = $stmt->fetchAll(PDO::FETCH_ASSOC);
    $bmap = [];
    foreach ($baselines as $b) $bmap[$b['machine_idx']] = $b;

    $stmt = $pdo->prepare("
    SELECT do.machine_idx, im.machine_no, il.line_name,
      ROUND(AVG(do.oee), 1)              AS cur_oee,
      ROUND(AVG(do.availabilty_rate), 1) AS cur_avail
    FROM data_oee do
    JOIN info_machine im ON do.machine_idx = im.idx
    JOIN info_line    il ON do.line_idx = il.idx
    WHERE do.work_date = ?
      $f_sql
    GROUP BY do.machine_idx, im.machine_no, il.line_name
  ");
    $stmt->execute(array_merge([$to], $fv));
    $cur_rows = $stmt->fetchAll(PDO::FETCH_ASSOC);

    $anomalies = [];
    foreach ($cur_rows as $row) {
        $mid = $row['machine_idx'];
        if (!isset($bmap[$mid]) || $bmap[$mid]['std_oee'] < 0.5) continue;
        $b = $bmap[$mid];
        $z = ($row['cur_oee'] - $b['mean_oee']) / $b['std_oee'];
        if (abs($z) >= 2.0) {
            $anomalies[] = [
                'machine'  => $row['machine_no'],
                'line'     => $row['line_name'],
                'z_score'  => round($z, 2),
                'cur_oee'  => $row['cur_oee'],
                'mean_oee' => round($b['mean_oee'], 1),
                'severity' => abs($z) >= 3.0 ? 'critical' : 'warning',
            ];
        }
    }
    usort($anomalies, fn($a, $b) => abs($b['z_score']) <=> abs($a['z_score']));
    $anomalies = array_slice($anomalies, 0, 5);

    // ── 5. 유지보수 위험도 (Top 5) ─────────────────────
    $mfp = [];
    $mfv = [];
    if ($factory !== '') {
        $mfp[] = 'im.factory_idx = ?';
        $mfv[] = $factory;
    }
    if ($line    !== '') {
        $mfp[] = 'im.line_idx = ?';
        $mfv[] = $line;
    }
    if ($machine !== '') {
        $mfp[] = 'im.idx = ?';
        $mfv[] = $machine;
    }
    $m_where = $mfp ? 'WHERE ' . implode(' AND ', $mfp) : '';

    $stmt = $pdo->prepare("
    SELECT im.idx, im.machine_no, il.line_name
    FROM info_machine im
    JOIN info_line il ON im.line_idx = il.idx
    $m_where
    ORDER BY il.line_name, im.machine_no
    LIMIT 50
  ");
    $stmt->execute($mfv);
    $all_machines = $stmt->fetchAll(PDO::FETCH_ASSOC);

    $maintenance = [];
    foreach ($all_machines as $m) {
        $mid = $m['idx'];

        $stmt2 = $pdo->prepare("
      SELECT COUNT(*) AS cnt30,
        SUM(CASE WHEN work_date >= DATE_SUB(CURDATE(), INTERVAL 7 DAY) THEN 1 ELSE 0 END) AS cnt7,
        SUM(CASE WHEN work_date BETWEEN DATE_SUB(CURDATE(), INTERVAL 60 DAY)
                                    AND DATE_SUB(CURDATE(), INTERVAL 31 DAY) THEN 1 ELSE 0 END) AS cnt_prev,
        MAX(reg_date) AS last_dt
      FROM data_downtime
      WHERE machine_idx = ? AND work_date >= DATE_SUB(CURDATE(), INTERVAL 60 DAY)
    ");
        $stmt2->execute([$mid]);
        $dt2 = $stmt2->fetch(PDO::FETCH_ASSOC);
        if ((int)$dt2['cnt30'] === 0) continue;

        $stmt3 = $pdo->prepare("
      SELECT COALESCE(STDDEV(oee), 0) AS std_oee, COALESCE(AVG(oee), 0) AS avg_oee
      FROM data_oee
      WHERE machine_idx = ? AND work_date >= DATE_SUB(CURDATE(), INTERVAL 30 DAY)
    ");
        $stmt3->execute([$mid]);
        $ov = $stmt3->fetch(PDO::FETCH_ASSOC);

        $runtime_s = min(100, (int)$dt2['cnt30'] * 5);
        $freq_s    = $dt2['cnt_prev'] > 0
            ? min(100, max(0, (($dt2['cnt7'] * 4.3 - $dt2['cnt_prev']) / $dt2['cnt_prev']) * 50 + 50))
            : 50;
        $instab_s  = $ov['avg_oee'] > 0
            ? min(100, ($ov['std_oee'] / $ov['avg_oee']) * 200)
            : 0;
        $risk = max(0, min(100, (int)round($runtime_s * 0.4 + $freq_s * 0.35 + $instab_s * 0.25)));

        $maintenance[] = [
            'machine'    => $m['machine_no'],
            'line'       => $m['line_name'],
            'risk_score' => $risk,
            'risk_level' => $risk >= 80 ? 'danger' : ($risk >= 50 ? 'warning' : 'normal'),
            'dt_count'   => (int)$dt2['cnt30'],
            'last_dt'    => $dt2['last_dt'],
        ];
    }
    usort($maintenance, fn($a, $b) => $b['risk_score'] <=> $a['risk_score']);
    $maintenance = array_slice($maintenance, 0, 5);

    // ── 6. 병목 라인 최적화 ─────────────────────────────
    $ofv = [$from, $to];
    $ofp = [];
    if ($factory !== '') {
        $ofp[] = 'do.factory_idx = ?';
        $ofv[] = $factory;
    }
    if ($line    !== '') {
        $ofp[] = 'do.line_idx = ?';
        $ofv[] = $line;
    }
    $o_sql = $ofp ? 'AND ' . implode(' AND ', $ofp) : '';

    $stmt = $pdo->prepare("
    SELECT il.line_name,
      ROUND(AVG(do.oee), 1)              AS avg_oee,
      ROUND(AVG(do.availabilty_rate), 1) AS avg_avail,
      ROUND(AVG(do.productivity_rate), 1) AS avg_perf,
      ROUND(AVG(do.quality_rate), 1)     AS avg_quality
    FROM data_oee do
    JOIN info_line il ON do.line_idx = il.idx
    WHERE do.work_date BETWEEN ? AND ?
      $o_sql
    GROUP BY il.idx, il.line_name
    HAVING COUNT(*) >= 3
    ORDER BY avg_oee ASC
    LIMIT 5
  ");
    $stmt->execute($ofv);
    $opt_lines = $stmt->fetchAll(PDO::FETCH_ASSOC);

    $optimization = [];
    foreach ($opt_lines as $ol) {
        $bn = 'availability';
        $bn_val = (float)$ol['avg_avail'];
        $bn_target = 90.0;
        if ((float)$ol['avg_perf']    < $bn_val) {
            $bn = 'performance';
            $bn_val = (float)$ol['avg_perf'];
            $bn_target = 90.0;
        }
        if ((float)$ol['avg_quality'] < $bn_val) {
            $bn = 'quality';
            $bn_val = (float)$ol['avg_quality'];
            $bn_target = 99.0;
        }

        $gain = round(($bn_target - $bn_val) * ((float)$ol['avg_oee'] / max(1, $bn_val)) * 0.5, 1);

        $optimization[] = [
            'line'           => $ol['line_name'],
            'current_oee'    => (float)$ol['avg_oee'],
            'avg_avail'      => (float)$ol['avg_avail'],
            'avg_perf'       => (float)$ol['avg_perf'],
            'avg_quality'    => (float)$ol['avg_quality'],
            'bottleneck'     => $bn,
            'bottleneck_val' => $bn_val,
            'potential_gain' => $gain,
            'priority'       => $ol['avg_oee'] < 60 ? 'P1' : ((float)$ol['avg_oee'] < 75 ? 'P2' : 'P3'),
        ];
    }

    // ── 7. 규칙 기반 인사이트 생성 ───────────────────────
    $insights = buildInsights($summary, $anomalies, $maintenance, $optimization, $prediction, $lang);

    echo json_encode([
        'code'         => '00',
        'period'       => ['from' => $from, 'to' => $to],
        'summary'      => $summary,
        'downtime'     => $downtime,
        'prediction'   => $prediction,
        'anomalies'    => $anomalies,
        'maintenance'  => $maintenance,
        'optimization' => $optimization,
        'insights'     => $insights,
    ], JSON_UNESCAPED_UNICODE);
} catch (Exception $e) {
    echo json_encode(['code' => '99', 'msg' => $e->getMessage()]);
}

// ── 규칙 기반 인사이트 생성 함수 ─────────────────────
function buildInsights(array $s, array $anomalies, array $maint, array $opt, array $pred, string $lang = 'en'): array
{
    $ko = ($lang === 'ko');
    $out = [];

    // OEE 상태
    $oee = (float)$s['avg_oee'];
    $gap = round(85 - $oee, 1);
    if ($gap <= 0) {
        $out[] = ['level' => 'success', 'text' => $ko
            ? "OEE {$oee}% — 목표(85%) 달성. 현재 성능을 유지하세요."
            : "OEE {$oee}% — Target (85%) achieved. Maintain current performance."];
    } elseif ($gap <= 5) {
        $out[] = ['level' => 'warning', 'text' => $ko
            ? "OEE {$oee}% — 목표(85%) 대비 {$gap}%p 미달. 소폭 개선이 필요합니다."
            : "OEE {$oee}% — {$gap}%p below target (85%). Minor improvements needed."];
    } else {
        $out[] = ['level' => 'error', 'text' => $ko
            ? "OEE {$oee}% — 목표(85%) 대비 {$gap}%p 미달. 즉각적인 조치가 필요합니다."
            : "OEE {$oee}% — {$gap}%p below target (85%). Immediate action required."];
    }

    // 병목 컴포넌트
    $avail = (float)$s['avg_avail'];
    $perf  = (float)$s['avg_perf'];
    $qual  = (float)$s['avg_quality'];
    $min_v = min($avail, $perf, $qual);
    if ($min_v === $avail) {
        $out[] = ['level' => 'warning', 'text' => $ko
            ? "주요 병목: 가동률({$avail}%). 비계획 다운타임 감소가 OEE 향상에 가장 효과적입니다."
            : "Primary bottleneck: Availability ({$avail}%). Reducing unplanned downtime will yield the highest OEE gain."];
    } elseif ($min_v === $perf) {
        $out[] = ['level' => 'warning', 'text' => $ko
            ? "주요 병목: 성능률({$perf}%). 이론적 최대 생산량 대비 처리량 향상을 권장합니다."
            : "Primary bottleneck: Performance ({$perf}%). Increasing throughput toward theoretical maximum is recommended."];
    } else {
        $out[] = ['level' => 'warning', 'text' => $ko
            ? "주요 병목: 품질률({$qual}%). 불량 감소를 최우선 과제로 삼아야 합니다."
            : "Primary bottleneck: Quality ({$qual}%). Defect reduction should be the priority focus."];
    }

    // OEE 추세
    if (!empty($pred)) {
        $slope = $pred['slope'];
        $f7    = $pred['7d'];
        if ($pred['trend'] === 'improving') {
            $out[] = ['level' => 'success', 'text' => $ko
                ? "OEE 추세: 개선 중(+{$slope}%/일). 7일 예측: {$f7}%."
                : "OEE trend: Improving (+{$slope}%/day). 7-day forecast: {$f7}%."];
        } elseif ($pred['trend'] === 'declining') {
            $out[] = ['level' => 'error', 'text' => $ko
                ? "OEE 추세: 하락 중({$slope}%/일). 원인 조사가 필요합니다. 7일 예측: {$f7}%."
                : "OEE trend: Declining ({$slope}%/day). Investigate root cause. 7-day forecast: {$f7}%."];
        } else {
            $out[] = ['level' => 'info', 'text' => $ko
                ? "OEE 추세: 안정적. 7일 예측: {$f7}%."
                : "OEE trend: Stable. 7-day forecast: {$f7}%."];
        }
    }

    // 이상 감지
    $acnt = count($anomalies);
    if ($acnt === 0) {
        $out[] = ['level' => 'success', 'text' => $ko
            ? "이상 감지: Z-Score 이상 없음."
            : "Anomaly detection: No Z-Score anomalies detected."];
    } else {
        $critical = count(array_filter($anomalies, fn($a) => $a['severity'] === 'critical'));
        if ($ko) {
            $msg = "이상 감지: {$acnt}건의 이상이 발견되었습니다.";
            if ($critical > 0) $msg .= " {$critical}건 위험 — 즉각적인 점검이 필요합니다.";
        } else {
            $msg = "Anomaly detection: {$acnt} anomaly(ies) found.";
            if ($critical > 0) $msg .= " {$critical} critical — immediate inspection required.";
        }
        $out[] = ['level' => 'error', 'text' => $msg];
    }

    // 유지보수 경보
    $danger_list = array_filter($maint, fn($m) => $m['risk_level'] === 'danger');
    if (!empty($danger_list)) {
        $names = implode(', ', array_column(array_values($danger_list), 'machine'));
        $cnt   = count($danger_list);
        $out[] = ['level' => 'error', 'text' => $ko
            ? "정비 경보: 고위험 기계 {$cnt}대 — {$names}. 즉시 점검 일정을 수립하세요."
            : "Maintenance alert: {$cnt} high-risk machine(s) — {$names}. Schedule inspection immediately."];
    }

    // 최적화 기회
    if (!empty($opt) && $opt[0]['potential_gain'] >= 2.0) {
        $top   = $opt[0];
        $label = $ko
            ? ['availability' => '가동률', 'performance' => '성능률', 'quality' => '품질률'][$top['bottleneck']]
            : ['availability' => 'Availability', 'performance' => 'Performance', 'quality' => 'Quality'][$top['bottleneck']];
        $out[] = ['level' => 'info', 'text' => $ko
            ? "최우선 기회: {$top['line']} — {$label}({$top['bottleneck_val']}%) 개선 시 OEE +{$top['potential_gain']}%p 향상 예상."
            : "Top opportunity: {$top['line']} — improving {$label} ({$top['bottleneck_val']}%) could yield +{$top['potential_gain']}%p OEE gain."];
    }

    return $out;
}
