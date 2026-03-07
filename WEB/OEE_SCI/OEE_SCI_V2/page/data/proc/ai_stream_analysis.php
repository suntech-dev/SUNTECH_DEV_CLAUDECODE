<?php
/**
 * F12 — AI 실시간 스트리밍 분석 (SSE 엔드포인트)
 * 이상 감지, 신규 다운타임, 정비 위험 경고를 실시간으로 스트리밍
 *
 * Method: GET (EventSource)
 * Params: factory_filter, line_filter, machine_filter
 * Events: connected | anomaly | downtime_new | maintenance_risk | heartbeat | disconnected
 */


require_once(__DIR__ . '/../../../lib/db.php');
require_once(__DIR__ . '/../../../lib/statistics.lib.php');

header('Content-Type: text/event-stream');
header('Cache-Control: no-cache');
header('Connection: keep-alive');
header('Access-Control-Allow-Origin: *');
header('X-Accel-Buffering: no');

if (ob_get_level()) ob_end_clean();

$factory_filter = isset($_GET['factory_filter']) ? trim($_GET['factory_filter']) : '';
$line_filter    = isset($_GET['line_filter'])    ? trim($_GET['line_filter'])    : '';
$machine_filter = isset($_GET['machine_filter']) ? trim($_GET['machine_filter']) : '';

// ── 공통 WHERE 빌더 ─────────────────────────────────────────────
function buildWhere(array &$params, string $fac, string $ln, string $mc, string $alias): string {
    $w = [];
    if ($fac !== '') { $w[] = "{$alias}.factory_idx = ?"; $params[] = $fac; }
    if ($ln  !== '') { $w[] = "{$alias}.line_idx = ?";    $params[] = $ln; }
    if ($mc  !== '') { $w[] = "{$alias}.machine_idx = ?"; $params[] = $mc; }
    return $w ? 'AND ' . implode(' AND ', $w) : '';
}

// ── SSE 이벤트 전송 ─────────────────────────────────────────────
function sendEvent(string $type, array $data): void {
    $json = json_encode($data, JSON_UNESCAPED_UNICODE);
    echo "event: {$type}\n";
    echo "data: {$json}\n\n";
    if (ob_get_level()) ob_flush();
    flush();
}

// ── 이상 감지 (최근 N분 OEE Z-Score) ───────────────────────────
function detectRecentAnomalies(
    PDO $pdo, int $minutes,
    string $fac, string $ln, string $mc
): array {
    // 30일 기준 통계
    $p1 = [];
    $w1 = buildWhere($p1, $fac, $ln, $mc, 'do');
    $sql_base = "
        SELECT
            do.machine_idx, im.machine_no, il.line_name,
            AVG(do.oee)               AS mean_oee,
            STDDEV(do.oee)            AS std_oee,
            AVG(do.quality_rate)      AS mean_quality,
            STDDEV(do.quality_rate)   AS std_quality
        FROM data_oee do
        JOIN info_machine im ON do.machine_idx = im.idx
        JOIN info_line    il ON do.line_idx = il.idx
        WHERE do.work_date BETWEEN DATE_SUB(CURDATE(), INTERVAL 30 DAY)
                                AND DATE_SUB(CURDATE(), INTERVAL 1 DAY)
          $w1
        GROUP BY do.machine_idx, im.machine_no, il.line_name
        HAVING COUNT(*) >= 3
    ";
    $stmt = $pdo->prepare($sql_base);
    $stmt->execute($p1);
    $baselines = $stmt->fetchAll(PDO::FETCH_ASSOC);
    if (empty($baselines)) return [];

    $bmap = [];
    foreach ($baselines as $b) $bmap[$b['machine_idx']] = $b;

    // 최근 N분 데이터 (work_date = 오늘, 최근 레코드)
    $p2 = [];
    $w2 = buildWhere($p2, $fac, $ln, $mc, 'do');
    $p2[] = $minutes;
    $sql_recent = "
        SELECT
            do.machine_idx, im.machine_no, il.line_name,
            do.oee, do.quality_rate, do.shift_idx, do.work_date
        FROM data_oee do
        JOIN info_machine im ON do.machine_idx = im.idx
        JOIN info_line    il ON do.line_idx = il.idx
        WHERE do.work_date = CURDATE()
          AND do.oee IS NOT NULL
          $w2
        ORDER BY do.machine_idx, do.shift_idx DESC
        LIMIT 100
    ";
    // N분 필터는 work_date=CURDATE()로 대체 (시간별 SSE 환경)
    unset($p2[count($p2) - 1]);
    $stmt2 = $pdo->prepare(str_replace('AND do.oee IS NOT NULL', 'AND do.oee IS NOT NULL', $sql_recent));
    $stmt2->execute($p2);
    $recents = $stmt2->fetchAll(PDO::FETCH_ASSOC);

    $events = [];
    $seen   = [];
    foreach ($recents as $r) {
        $mid = $r['machine_idx'];
        if (isset($seen[$mid])) continue;  // 머신당 1건
        $seen[$mid] = true;
        $b = $bmap[$mid] ?? null;
        if (!$b) continue;

        if ((float)$b['std_oee'] > 0) {
            $z = abs(((float)$r['oee'] - (float)$b['mean_oee']) / (float)$b['std_oee']);
            if ($z >= 2.0 && (float)$r['oee'] < (float)$b['mean_oee']) {
                $sev = $z >= 3.0 ? 'danger' : 'warning';
                $events[] = [
                    'machine_no' => $r['machine_no'],
                    'line_name'  => $r['line_name'],
                    'metric'     => 'OEE',
                    'value'      => round((float)$r['oee'], 1),
                    'mean'       => round((float)$b['mean_oee'], 1),
                    'z_score'    => round($z, 2),
                    'severity'   => $sev,
                    'message'    => "{$r['machine_no']} OEE " . round((float)$r['oee'], 1) . "% — 기준(" . round((float)$b['mean_oee'], 1) . "%) 대비 이상 (Z=" . round($z, 2) . ")",
                    'timestamp'  => date('H:i:s'),
                ];
            }
        }

        if ((float)$b['std_quality'] > 0 && $r['quality_rate'] !== null) {
            $zq = abs(((float)$r['quality_rate'] - (float)$b['mean_quality']) / (float)$b['std_quality']);
            if ($zq >= 2.0 && (float)$r['quality_rate'] < (float)$b['mean_quality']) {
                $events[] = [
                    'machine_no' => $r['machine_no'],
                    'line_name'  => $r['line_name'],
                    'metric'     => 'Quality',
                    'value'      => round((float)$r['quality_rate'], 1),
                    'mean'       => round((float)$b['mean_quality'], 1),
                    'z_score'    => round($zq, 2),
                    'severity'   => $zq >= 3.0 ? 'danger' : 'warning',
                    'message'    => "{$r['machine_no']} 품질률 " . round((float)$r['quality_rate'], 1) . "% — 기준(" . round((float)$b['mean_quality'], 1) . "%) 대비 품질 경고 (Z=" . round($zq, 2) . ")",
                    'timestamp'  => date('H:i:s'),
                ];
            }
        }
    }

    return $events;
}

// ── 신규 다운타임 감지 ───────────────────────────────────────────
function detectActiveDowntimes(
    PDO $pdo,
    string $fac, string $ln, string $mc
): array {
    $p = [];
    $w = buildWhere($p, $fac, $ln, $mc, 'dd');
    $sql = "
        SELECT
            dd.machine_no, il.line_name, ift.factory_name,
            dd.reg_date,
            COALESCE(ddt.downtime_name, '미분류') AS downtime_name,
            TIMESTAMPDIFF(MINUTE, dd.reg_date, NOW()) AS elapsed_min
        FROM data_downtime dd
        JOIN info_line    il  ON dd.line_idx = il.idx
        JOIN info_factory ift ON dd.factory_idx = ift.idx
        LEFT JOIN data_downtime_type ddt ON dd.downtime_idx = ddt.idx
        WHERE dd.update_date IS NULL
          AND dd.work_date = CURDATE()
          $w
        ORDER BY dd.reg_date DESC
        LIMIT 5
    ";
    $stmt = $pdo->prepare($sql);
    $stmt->execute($p);
    $rows = $stmt->fetchAll(PDO::FETCH_ASSOC);

    $events = [];
    foreach ($rows as $r) {
        $elapsed = (int)$r['elapsed_min'];
        $events[] = [
            'machine_no'    => $r['machine_no'],
            'line_name'     => $r['line_name'],
            'factory_name'  => $r['factory_name'],
            'downtime_name' => $r['downtime_name'],
            'elapsed_min'   => $elapsed,
            'severity'      => $elapsed > 30 ? 'danger' : 'warning',
            'message'       => "활성 다운타임: {$r['machine_no']} — {$r['downtime_name']} ({$elapsed}분 경과)",
            'timestamp'     => date('H:i:s', strtotime($r['reg_date'])),
        ];
    }
    return $events;
}

// ── 고위험 머신 감지 (risk_score >= 70) ─────────────────────────
function detectHighRiskMachines(
    PDO $pdo,
    string $fac, string $ln, string $mc
): array {
    $p = [];
    $w_parts = [];
    if ($fac !== '') { $w_parts[] = 'im.factory_idx = ?'; $p[] = $fac; }
    if ($ln  !== '') { $w_parts[] = 'im.line_idx = ?';    $p[] = $ln; }
    if ($mc  !== '') { $w_parts[] = 'im.idx = ?';         $p[] = $mc; }
    $where_sql = $w_parts ? 'WHERE ' . implode(' AND ', $w_parts) : '';

    // 기계 목록
    $stmt = $pdo->prepare("
        SELECT im.idx, im.machine_no, il.line_name
        FROM info_machine im
        JOIN info_line il ON im.line_idx = il.idx
        $where_sql
        LIMIT 100
    ");
    $stmt->execute($p);
    $machines = $stmt->fetchAll(PDO::FETCH_ASSOC);
    if (empty($machines)) return [];

    $ids = array_column($machines, 'idx');
    $ph  = implode(',', array_fill(0, count($ids), '?'));

    // 다운타임 집계 (최근 14일)
    $stmt2 = $pdo->prepare("
        SELECT
            dd.machine_idx,
            SUM(CASE WHEN dd.work_date >= DATE_SUB(CURDATE(), INTERVAL 7 DAY)  THEN 1 ELSE 0 END) AS cnt_7d,
            SUM(CASE WHEN dd.work_date BETWEEN DATE_SUB(CURDATE(), INTERVAL 14 DAY)
                                           AND DATE_SUB(CURDATE(), INTERVAL 8 DAY) THEN 1 ELSE 0 END) AS cnt_prev7d,
            TIMESTAMPDIFF(HOUR, MAX(dd.reg_date), NOW()) AS hrs_since
        FROM data_downtime dd
        WHERE dd.machine_idx IN ($ph)
          AND dd.work_date >= DATE_SUB(CURDATE(), INTERVAL 14 DAY)
        GROUP BY dd.machine_idx
    ");
    $stmt2->execute($ids);
    $dtmap = [];
    foreach ($stmt2->fetchAll(PDO::FETCH_ASSOC) as $d) $dtmap[$d['machine_idx']] = $d;

    $events = [];
    foreach ($machines as $m) {
        $d = $dtmap[$m['idx']] ?? null;
        if (!$d || (int)$d['cnt_7d'] === 0) continue;

        $curr  = (int)$d['cnt_7d'];
        $prev  = max(1, (int)$d['cnt_prev7d']);
        $rate  = min(1.0, ($curr - $prev) / $prev);
        $hrs   = max(0, (int)($d['hrs_since'] ?? 9999));
        $score = (int)round(
            min(100, $curr * 8) * 0.40 +
            max(0, $rate * 100) * 0.35 +
            max(0, 100 - $hrs)  * 0.25
        );

        if ($score >= 70) {
            $events[] = [
                'machine_no'  => $m['machine_no'],
                'line_name'   => $m['line_name'],
                'risk_score'  => $score,
                'dt_count_7d' => $curr,
                'severity'    => $score >= 85 ? 'danger' : 'warning',
                'message'     => "정비 위험: {$m['machine_no']} — 위험도 {$score}% (7일 내 다운타임 {$curr}건)",
                'timestamp'   => date('H:i:s'),
            ];
        }
    }

    usort($events, fn($a, $b) => $b['risk_score'] - $a['risk_score']);
    return array_slice($events, 0, 3);
}

// ════════════════════════════════════════════════════════════════
// 메인 스트림
// ════════════════════════════════════════════════════════════════

// 연결 이벤트
sendEvent('connected', [
    'message'   => 'AI 스트림 분석 연결됨',
    'timestamp' => date('H:i:s'),
]);

// 초기 이벤트 전송 (오늘 현황)
$init_anomalies = detectRecentAnomalies($pdo, 60, $factory_filter, $line_filter, $machine_filter);
foreach ($init_anomalies as $a) sendEvent('anomaly', $a);

$init_downtimes = detectActiveDowntimes($pdo, $factory_filter, $line_filter, $machine_filter);
foreach ($init_downtimes as $d) sendEvent('downtime_new', $d);

$init_risks = detectHighRiskMachines($pdo, $factory_filter, $line_filter, $machine_filter);
foreach ($init_risks as $r) sendEvent('maintenance_risk', $r);

$total_init = count($init_anomalies) + count($init_downtimes) + count($init_risks);
if ($total_init === 0) {
    sendEvent('status', [
        'message'   => '현재 감지된 이상 없음. 실시간 모니터링 중...',
        'timestamp' => date('H:i:s'),
    ]);
}

// 스트림 루프 (최대 5분, 15초 간격)
$start          = time();
$maxRuntime     = 300;
$pollInterval   = 15;
$heartbeatEvery = 60;
$lastHeartbeat  = time();

while (!connection_aborted() && (time() - $start) < $maxRuntime) {
    sleep($pollInterval);
    if (connection_aborted()) break;

    // 신규 활성 다운타임 폴링
    $new_dt = detectActiveDowntimes($pdo, $factory_filter, $line_filter, $machine_filter);
    foreach ($new_dt as $d) sendEvent('downtime_new', $d);

    // 하트비트
    if ((time() - $lastHeartbeat) >= $heartbeatEvery) {
        sendEvent('heartbeat', ['timestamp' => date('H:i:s'), 'status' => 'ok']);
        $lastHeartbeat = time();
    }
}

sendEvent('disconnected', [
    'message'   => '스트림 분석 세션 종료 — 재연결 대기 중',
    'timestamp' => date('H:i:s'),
]);
