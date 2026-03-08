<?php
/**
 * ai_report_export.php — HTML 리포트 다운로드
 * ai_report_engine.php 데이터를 받아 독립형 HTML 파일 생성
 * 브라우저에서 Ctrl+P → PDF 저장 가능
 *
 * Method: GET (report.php와 동일한 파라미터)
 */

require_once(__DIR__ . '/../../../lib/db.php');
require_once(__DIR__ . '/../../../lib/statistics.lib.php');

// ai_report_engine.php 의 핵심 로직 재사용 (include)
ob_start();
$_SERVER['REQUEST_METHOD'] = 'GET'; // ensure
include __DIR__ . '/ai_report_engine.php';
$json = ob_get_clean();
$data = json_decode($json, true);

if (!$data || $data['code'] !== '00') {
  header('Content-Type: text/html; charset=utf-8');
  echo '<p style="color:red;">Report generation failed: ' . htmlspecialchars($data['msg'] ?? 'Unknown error') . '</p>';
  exit;
}

$s    = $data['summary'];
$pred = $data['prediction'] ?? [];
$anom = $data['anomalies']  ?? [];
$maint= $data['maintenance']?? [];
$opt  = $data['optimization']??[];
$ins  = $data['insights']   ?? [];
$from = $data['period']['from'];
$to   = $data['period']['to'];
$dt   = $data['downtime']   ?? [];

$fname = 'OEE_Report_' . date('Y-m-d_His') . '.html';
header('Content-Type: text/html; charset=utf-8');
header('Content-Disposition: attachment; filename="' . $fname . '"');
header('Cache-Control: no-cache');

// ── 인사이트 HTML 생성 헬퍼 ──────────────────────────
function insightColor(string $level): string {
  return ['success' => '#3fb950', 'warning' => '#d29922', 'error' => '#f85149', 'info' => '#58a6ff'][$level] ?? '#8b949e';
}
function riskColor(string $level): string {
  return ['danger' => '#f85149', 'warning' => '#d29922', 'normal' => '#3fb950'][$level] ?? '#8b949e';
}
function oeeColor(float $v, float $target = 85.0): string {
  return $v >= $target ? '#3fb950' : ($v >= $target * 0.85 ? '#d29922' : '#f85149');
}

?><!DOCTYPE html>
<html lang="en">
<head>
<meta charset="utf-8">
<title>OEE AI Report — <?= htmlspecialchars($from) ?> ~ <?= htmlspecialchars($to) ?></title>
<style>
* { box-sizing: border-box; margin: 0; padding: 0; }
body { background: #0d1117; color: #e6edf3; font-family: 'Segoe UI', Arial, sans-serif; font-size: 13px; line-height: 1.5; }
.page { max-width: 1100px; margin: 0 auto; padding: 32px 24px; }

/* Header */
.rpt-header { background: #161b22; border-radius: 8px; padding: 24px 28px; margin-bottom: 20px; border: 1px solid #30363d; }
.rpt-header h1 { font-size: 1.4rem; color: #58a6ff; margin-bottom: 4px; }
.rpt-header .meta { color: #8b949e; font-size: 0.85rem; }
.rpt-header .badge { background: #0070f2; color: #fff; font-size: 0.7rem; padding: 2px 8px; border-radius: 3px; margin-left: 8px; vertical-align: middle; }

/* Section */
.section { margin-bottom: 20px; }
.section-title { font-size: 0.95rem; font-weight: 600; color: #58a6ff; border-bottom: 1px solid #30363d; padding-bottom: 6px; margin-bottom: 12px; }

/* KPI Grid */
.kpi-grid { display: grid; grid-template-columns: repeat(4, 1fr); gap: 10px; }
.kpi-card { background: #161b22; border: 1px solid #30363d; border-radius: 8px; padding: 14px 16px; text-align: center; }
.kpi-card__label { font-size: 0.75rem; color: #8b949e; margin-bottom: 6px; }
.kpi-card__value { font-size: 1.8rem; font-weight: 700; }
.kpi-card__sub   { font-size: 0.75rem; color: #8b949e; margin-top: 4px; }

/* Insights */
.insight-list { display: flex; flex-direction: column; gap: 6px; }
.insight-item { padding: 8px 12px; border-radius: 6px; border-left: 3px solid; background: #161b22; font-size: 0.85rem; }

/* Table */
.rpt-table { width: 100%; border-collapse: collapse; font-size: 0.82rem; }
.rpt-table th { background: #21262d; color: #8b949e; padding: 7px 10px; text-align: left; border-bottom: 1px solid #30363d; }
.rpt-table td { padding: 7px 10px; border-bottom: 1px solid #21262d; }
.rpt-table tr:last-child td { border-bottom: none; }
.badge-level { display: inline-block; padding: 1px 7px; border-radius: 3px; font-size: 0.72rem; font-weight: 600; }

/* 2-col grid */
.grid-2 { display: grid; grid-template-columns: 1fr 1fr; gap: 10px; }
.card { background: #161b22; border: 1px solid #30363d; border-radius: 8px; padding: 14px 16px; }
.card-title { font-size: 0.85rem; font-weight: 600; color: #8b949e; margin-bottom: 10px; }

/* Prediction */
.pred-row { display: flex; justify-content: space-between; padding: 5px 0; border-bottom: 1px solid #21262d; }
.pred-row:last-child { border: none; }
.pred-label { color: #8b949e; }
.pred-val   { font-weight: 600; }

/* Footer */
.rpt-footer { text-align: center; color: #8b949e; font-size: 0.75rem; margin-top: 28px; padding-top: 16px; border-top: 1px solid #30363d; }

@media print {
  body { background: #0d1117 !important; -webkit-print-color-adjust: exact !important; print-color-adjust: exact !important; }
  .page { padding: 16px; }
  .kpi-card, .card { break-inside: avoid; }
  .section { break-inside: avoid; }
}
</style>
</head>
<body>
<div class="page">

  <!-- Header -->
  <div class="rpt-header">
    <h1>OEE AI Manufacturing Report <span class="badge">AI POWERED</span></h1>
    <div class="meta">
      Period: <?= htmlspecialchars($from) ?> ~ <?= htmlspecialchars($to) ?> &nbsp;|&nbsp;
      Generated: <?= date('Y-m-d H:i:s') ?> &nbsp;|&nbsp;
      Machines: <?= (int)$s['machine_count'] ?>
    </div>
  </div>

  <!-- KPI Summary -->
  <div class="section">
    <div class="section-title">OEE Summary</div>
    <div class="kpi-grid">
      <?php
      $kpis = [
        ['OEE', $s['avg_oee'], 85, '%'],
        ['Availability', $s['avg_avail'], 90, '%'],
        ['Performance',  $s['avg_perf'],  90, '%'],
        ['Quality',      $s['avg_quality'],95, '%'],
      ];
      foreach ($kpis as $k):
        $color = oeeColor((float)$k[1], (float)$k[2]);
      ?>
      <div class="kpi-card">
        <div class="kpi-card__label"><?= htmlspecialchars($k[0]) ?></div>
        <div class="kpi-card__value" style="color:<?= $color ?>"><?= htmlspecialchars($k[1]) ?><?= $k[3] ?></div>
        <div class="kpi-card__sub">Target: <?= $k[2] ?><?= $k[3] ?></div>
      </div>
      <?php endforeach; ?>
    </div>
    <?php if ($dt): ?>
    <div style="margin-top:10px; display:flex; gap:20px; font-size:0.82rem; color:#8b949e; padding:8px 0;">
      <span>Downtime Events: <strong style="color:#e6edf3"><?= (int)$dt['dt_count'] ?></strong></span>
      <span>Total Downtime: <strong style="color:#e6edf3"><?= htmlspecialchars($dt['dt_total_min']) ?> min</strong></span>
      <span>Avg / Event: <strong style="color:#e6edf3"><?= htmlspecialchars($dt['dt_avg_min']) ?> min</strong></span>
    </div>
    <?php endif; ?>
  </div>

  <!-- AI Insights -->
  <div class="section">
    <div class="section-title">AI Insights</div>
    <div class="insight-list">
      <?php foreach ($ins as $i):
        $c = insightColor($i['level']);
      ?>
      <div class="insight-item" style="border-color:<?= $c ?>; color:#e6edf3;">
        <?= htmlspecialchars($i['text']) ?>
      </div>
      <?php endforeach; ?>
    </div>
  </div>

  <!-- Prediction + Anomalies -->
  <div class="grid-2 section">
    <!-- Predictive Analytics -->
    <div class="card">
      <div class="card-title">Predictive Analytics</div>
      <?php if (!empty($pred)): ?>
      <div class="pred-row">
        <span class="pred-label">7-Day OEE Forecast</span>
        <span class="pred-val" style="color:<?= oeeColor((float)$pred['7d']) ?>"><?= htmlspecialchars($pred['7d']) ?>%</span>
      </div>
      <div class="pred-row">
        <span class="pred-label">30-Day OEE Forecast</span>
        <span class="pred-val" style="color:<?= oeeColor((float)$pred['30d']) ?>"><?= htmlspecialchars($pred['30d']) ?>%</span>
      </div>
      <div class="pred-row">
        <span class="pred-label">Trend</span>
        <span class="pred-val"><?= htmlspecialchars(ucfirst($pred['trend'] ?? '--')) ?> (<?= htmlspecialchars($pred['slope'] ?? 0) ?>%/day)</span>
      </div>
      <?php else: ?>
      <div style="color:#8b949e;font-size:0.82rem;">Insufficient data for prediction (need 3+ days).</div>
      <?php endif; ?>
    </div>

    <!-- Anomaly Detection -->
    <div class="card">
      <div class="card-title">Anomaly Detection (Z-Score)</div>
      <?php if (empty($anom)): ?>
      <div style="color:#3fb950;font-size:0.82rem;">No anomalies detected.</div>
      <?php else: ?>
      <table class="rpt-table">
        <thead><tr><th>Machine</th><th>Line</th><th>OEE</th><th>Z-Score</th><th>Level</th></tr></thead>
        <tbody>
        <?php foreach ($anom as $a):
          $ac = $a['severity'] === 'critical' ? '#f85149' : '#d29922';
        ?>
          <tr>
            <td><?= htmlspecialchars($a['machine']) ?></td>
            <td><?= htmlspecialchars($a['line']) ?></td>
            <td><?= htmlspecialchars($a['cur_oee']) ?>%</td>
            <td><?= htmlspecialchars($a['z_score']) ?></td>
            <td><span class="badge-level" style="background:<?= $ac ?>;color:#fff"><?= strtoupper($a['severity']) ?></span></td>
          </tr>
        <?php endforeach; ?>
        </tbody>
      </table>
      <?php endif; ?>
    </div>
  </div>

  <!-- Maintenance + Optimization -->
  <div class="grid-2 section">
    <!-- Predictive Maintenance -->
    <div class="card">
      <div class="card-title">Predictive Maintenance (Top 5)</div>
      <?php if (empty($maint)): ?>
      <div style="color:#8b949e;font-size:0.82rem;">No downtime history found.</div>
      <?php else: ?>
      <table class="rpt-table">
        <thead><tr><th>Machine</th><th>Line</th><th>Risk</th><th>Events</th></tr></thead>
        <tbody>
        <?php foreach ($maint as $m):
          $mc = riskColor($m['risk_level']);
        ?>
          <tr>
            <td><?= htmlspecialchars($m['machine']) ?></td>
            <td><?= htmlspecialchars($m['line']) ?></td>
            <td><strong style="color:<?= $mc ?>"><?= (int)$m['risk_score'] ?>%</strong></td>
            <td><?= (int)$m['dt_count'] ?></td>
          </tr>
        <?php endforeach; ?>
        </tbody>
      </table>
      <?php endif; ?>
    </div>

    <!-- Optimization -->
    <div class="card">
      <div class="card-title">Optimization Opportunities</div>
      <?php if (empty($opt)): ?>
      <div style="color:#8b949e;font-size:0.82rem;">No optimization data available.</div>
      <?php else: ?>
      <table class="rpt-table">
        <thead><tr><th>Line</th><th>OEE</th><th>Bottleneck</th><th>Gain</th><th>Pri</th></tr></thead>
        <tbody>
        <?php foreach ($opt as $o):
          $pri_c = $o['priority'] === 'P1' ? '#f85149' : ($o['priority'] === 'P2' ? '#d29922' : '#8b949e');
        ?>
          <tr>
            <td><?= htmlspecialchars($o['line']) ?></td>
            <td><?= htmlspecialchars($o['current_oee']) ?>%</td>
            <td><?= htmlspecialchars(ucfirst($o['bottleneck'])) ?> (<?= htmlspecialchars($o['bottleneck_val']) ?>%)</td>
            <td style="color:#3fb950">+<?= htmlspecialchars($o['potential_gain']) ?>%p</td>
            <td><span class="badge-level" style="background:<?= $pri_c ?>;color:#fff"><?= htmlspecialchars($o['priority']) ?></span></td>
          </tr>
        <?php endforeach; ?>
        </tbody>
      </table>
      <?php endif; ?>
    </div>
  </div>

  <div class="rpt-footer">
    AI Manufacturing Report &mdash; Generated by OEE_SCI_V2 AI Engine (Rule-based, No external API) &mdash; <?= date('Y-m-d H:i:s') ?>
  </div>

</div>
</body>
</html>
