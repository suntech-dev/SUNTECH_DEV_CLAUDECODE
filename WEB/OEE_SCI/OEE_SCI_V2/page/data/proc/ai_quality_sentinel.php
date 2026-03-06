<?php
/**
 * AI Quality Sentinel API
 * 불량 패턴 분석: Pareto, 시간대별 히트맵, 기계 위험 순위, OEE 상관관계
 */

date_default_timezone_set('Asia/Jakarta');
require_once(__DIR__ . '/../../../lib/db.php');

header('Content-Type: application/json; charset=utf-8');
header('Cache-Control: no-cache');

$factory_filter = isset($_GET['factory_filter']) ? trim($_GET['factory_filter']) : '';
$line_filter    = isset($_GET['line_filter'])    ? trim($_GET['line_filter'])    : '';
$machine_filter = isset($_GET['machine_filter']) ? trim($_GET['machine_filter']) : '';
$days           = isset($_GET['days']) ? max(7, min(90, (int)$_GET['days'])) : 30;

// data_defective 테이블 WHERE 조건
$where_parts = ['dd.reg_date >= DATE_SUB(CURDATE(), INTERVAL ? DAY)'];
$params      = [$days];

if ($factory_filter !== '') { $where_parts[] = 'dd.factory_idx = ?'; $params[] = $factory_filter; }
if ($line_filter    !== '') { $where_parts[] = 'dd.line_idx = ?';    $params[] = $line_filter;    }
if ($machine_filter !== '') { $where_parts[] = 'dd.machine_idx = ?'; $params[] = $machine_filter; }

$where_sql = 'WHERE ' . implode(' AND ', $where_parts);

try {
    // 1. 전체 불량 건수
    $stmt = $pdo->prepare("SELECT COUNT(*) FROM data_defective dd $where_sql");
    $stmt->execute($params);
    $total_defects = (int)$stmt->fetchColumn();

    if ($total_defects === 0) {
        echo json_encode([
            'code'           => '00',
            'total_defects'  => 0,
            'analysis_days'  => $days,
            'pareto'         => [],
            'pareto_summary' => ['top_n_types' => 0, 'top_n_pct' => 80],
            'hourly_heatmap' => [],
            'peak_hour'      => null,
            'machine_ranking'=> [],
            'oee_correlation'=> ['coefficient' => null, 'insight' => null, 'data_points' => 0],
            'summary'        => ['total_defects' => 0, 'top_type' => null, 'peak_hour' => null, 'analysis_days' => $days],
        ], JSON_UNESCAPED_UNICODE);
        exit;
    }

    // 2. Pareto 분석 (불량 유형별 빈도)
    $stmt = $pdo->prepare("
        SELECT dd.defective_name, COUNT(*) AS cnt
        FROM data_defective dd $where_sql
        GROUP BY dd.defective_name
        ORDER BY cnt DESC
    ");
    $stmt->execute($params);
    $raw_pareto = $stmt->fetchAll(PDO::FETCH_ASSOC);

    $pareto_types  = [];
    $cum           = 0;
    $top_n_types   = 0;
    $top_n_reached = false;

    foreach ($raw_pareto as $row) {
        $pct  = round($row['cnt'] / $total_defects * 100, 1);
        $cum += $pct;
        $pareto_types[] = [
            'name'    => $row['defective_name'],
            'count'   => (int)$row['cnt'],
            'pct'     => $pct,
            'cum_pct' => round(min($cum, 100), 1),
        ];
        if (!$top_n_reached) {
            $top_n_types++;
            if ($cum >= 80) $top_n_reached = true;
        }
    }

    // 3. 시간대별 히트맵 (24시간 기준)
    $stmt = $pdo->prepare("
        SELECT
            HOUR(dd.reg_date) AS hour,
            COUNT(*) AS cnt,
            COUNT(DISTINCT dd.machine_idx) AS machines_affected
        FROM data_defective dd $where_sql
        GROUP BY HOUR(dd.reg_date)
        ORDER BY hour
    ");
    $stmt->execute($params);
    $raw_hourly = $stmt->fetchAll(PDO::FETCH_ASSOC);

    $max_hourly     = max(array_column($raw_hourly, 'cnt') ?: [1]);
    $hourly_heatmap = [];

    foreach ($raw_hourly as $row) {
        $ratio = $row['cnt'] / $max_hourly;
        $hourly_heatmap[] = [
            'hour'              => (int)$row['hour'],
            'label'             => sprintf('%02d:00', (int)$row['hour']),
            'count'             => (int)$row['cnt'],
            'machines_affected' => (int)$row['machines_affected'],
            'risk'              => $ratio >= 0.7 ? 'high' : ($ratio >= 0.4 ? 'medium' : 'low'),
        ];
    }

    $peak_row  = count($raw_hourly)
        ? array_reduce($raw_hourly, fn($c, $r) => ($c === null || $r['cnt'] > $c['cnt']) ? $r : $c, null)
        : null;
    $peak_hour = $peak_row ? (int)$peak_row['hour'] : null;

    // 4. 기계별 위험 순위 (최근 7일 vs 이전 7일 증가율)
    $stmt = $pdo->prepare("
        SELECT
            dd.machine_no,
            dd.machine_idx,
            COUNT(*) AS total_count,
            SUM(CASE WHEN dd.reg_date >= DATE_SUB(CURDATE(), INTERVAL 7 DAY) THEN 1 ELSE 0 END) AS recent_7d,
            SUM(CASE WHEN dd.reg_date BETWEEN DATE_SUB(CURDATE(), INTERVAL 14 DAY)
                                          AND DATE_SUB(CURDATE(), INTERVAL 8 DAY)  THEN 1 ELSE 0 END) AS prev_7d,
            GROUP_CONCAT(DISTINCT dd.defective_name ORDER BY dd.defective_name SEPARATOR ', ') AS defective_types
        FROM data_defective dd $where_sql
        GROUP BY dd.machine_no, dd.machine_idx
        ORDER BY total_count DESC
        LIMIT 10
    ");
    $stmt->execute($params);
    $machine_rows = $stmt->fetchAll(PDO::FETCH_ASSOC);

    $machine_ranking = [];
    foreach ($machine_rows as $m) {
        $recent        = (int)$m['recent_7d'];
        $prev          = (int)$m['prev_7d'];
        $increase_rate = $prev > 0 ? ($recent - $prev) / $prev : ($recent > 0 ? 1.0 : 0.0);
        $risk_score    = (int)min(100, max(0, round(abs($increase_rate) * 100)));

        if ($increase_rate >= 0.8)      $risk_level = 'danger';
        elseif ($increase_rate >= 0.3)  $risk_level = 'warning';
        else                             $risk_level = 'normal';

        $machine_ranking[] = [
            'machine_no'      => $m['machine_no'],
            'total_count'     => (int)$m['total_count'],
            'recent_7d'       => $recent,
            'prev_7d'         => $prev,
            'increase_rate'   => round($increase_rate * 100, 1),
            'risk_score'      => $risk_score,
            'risk_level'      => $risk_level,
            'defective_types' => $m['defective_types'],
        ];
    }

    // 5. OEE 상관관계 (일별 불량건수 vs 일별 평균 OEE — Pearson)
    $oee_parts  = ['work_date >= DATE_SUB(CURDATE(), INTERVAL ? DAY)'];
    $oee_params = [$days];
    if ($factory_filter !== '') { $oee_parts[] = 'factory_idx = ?'; $oee_params[] = $factory_filter; }
    if ($line_filter    !== '') { $oee_parts[] = 'line_idx = ?';    $oee_params[] = $line_filter;    }
    if ($machine_filter !== '') { $oee_parts[] = 'machine_idx = ?'; $oee_params[] = $machine_filter; }
    $oee_where = 'WHERE ' . implode(' AND ', $oee_parts);

    $stmt = $pdo->prepare("
        SELECT DATE(dd.reg_date) AS dt, COUNT(*) AS defect_cnt
        FROM data_defective dd $where_sql
        GROUP BY DATE(dd.reg_date)
        ORDER BY dt
    ");
    $stmt->execute($params);
    $defect_daily = $stmt->fetchAll(PDO::FETCH_ASSOC);

    $stmt = $pdo->prepare("
        SELECT work_date AS dt, AVG(oee) AS avg_oee
        FROM data_oee $oee_where
        GROUP BY work_date
        ORDER BY work_date
    ");
    $stmt->execute($oee_params);
    $oee_daily = $stmt->fetchAll(PDO::FETCH_ASSOC);

    $defect_by_date = [];
    foreach ($defect_daily as $d) $defect_by_date[$d['dt']] = (float)$d['defect_cnt'];
    $oee_by_date = [];
    foreach ($oee_daily as $o)   $oee_by_date[$o['dt']]    = (float)$o['avg_oee'];

    $x_arr = [];
    $y_arr = [];
    foreach ($defect_by_date as $dt => $cnt) {
        if (isset($oee_by_date[$dt])) {
            $x_arr[] = $cnt;
            $y_arr[] = $oee_by_date[$dt];
        }
    }

    $correlation  = null;
    $corr_insight = null;
    $n = count($x_arr);

    if ($n >= 5) {
        $mean_x = array_sum($x_arr) / $n;
        $mean_y = array_sum($y_arr) / $n;
        $num = 0; $den_x = 0; $den_y = 0;
        for ($i = 0; $i < $n; $i++) {
            $dx     = $x_arr[$i] - $mean_x;
            $dy     = $y_arr[$i] - $mean_y;
            $num   += $dx * $dy;
            $den_x += $dx * $dx;
            $den_y += $dy * $dy;
        }
        $denom = sqrt($den_x * $den_y);
        if ($denom > 0) {
            $correlation = round($num / $denom, 3);
            if      ($correlation <= -0.6) $corr_insight = 'strong_negative';
            elseif  ($correlation <= -0.3) $corr_insight = 'weak_negative';
            elseif  ($correlation >= 0.6)  $corr_insight = 'strong_positive';
            elseif  ($correlation >= 0.3)  $corr_insight = 'weak_positive';
            else                            $corr_insight = 'no_correlation';
        }
    }

    echo json_encode([
        'code'           => '00',
        'total_defects'  => $total_defects,
        'analysis_days'  => $days,
        'pareto'         => $pareto_types,
        'pareto_summary' => ['top_n_types' => $top_n_types, 'top_n_pct' => 80],
        'hourly_heatmap' => $hourly_heatmap,
        'peak_hour'      => $peak_hour,
        'machine_ranking'=> $machine_ranking,
        'oee_correlation'=> ['coefficient' => $correlation, 'insight' => $corr_insight, 'data_points' => $n],
        'summary'        => [
            'total_defects' => $total_defects,
            'top_type'      => !empty($pareto_types) ? $pareto_types[0]['name'] : null,
            'peak_hour'     => $peak_hour,
            'analysis_days' => $days,
        ],
    ], JSON_UNESCAPED_UNICODE);

} catch (PDOException $e) {
    echo json_encode(['code' => '99', 'msg' => 'DB error: ' . $e->getMessage()]);
}
?>
