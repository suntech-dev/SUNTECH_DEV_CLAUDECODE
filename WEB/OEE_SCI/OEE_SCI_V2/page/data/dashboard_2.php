<?php

/**
 * dashboard_2.php — 1920x1080 Signage Dashboard
 * dashboard.php 의 사이니지 전용 버전
 * - nav 제거, 슬림 헤더 55px
 * - CSS Grid 4행 고정 레이아웃 (overflow: hidden)
 * - js/dashboard.js 100% 재사용
 */

$page_title = 'SCI OEE Dashboard - Signage';
$page_css_files = [
  '../../assets/css/fiori-page.css',
  '../../assets/css/daterangepicker.css',
  'css/dashboard.css',
  'css/dashboard_2.css',
];

require_once(__DIR__ . '/../../inc/head.php');
/* nav-fiori.php 제거 — 사이니지에는 네비게이션 불필요 */
?>

<!-- Hamburger Drawer -->
<div id="navDrawerOverlay" class="nav-drawer-overlay"></div>
<div id="navDrawer" class="nav-drawer">
  <div class="nav-drawer__header">OEE SYSTEM</div>
  <nav class="nav-drawer__menu">
    <div class="nav-drawer__group">
      <div class="nav-drawer__group-title">Setting</div>
      <a href="../manage/info_factory.php" class="nav-drawer__link">Factory</a>
      <a href="../manage/info_line.php" class="nav-drawer__link">Line</a>
      <a href="../manage/info_machine_model.php" class="nav-drawer__link">Machine Model</a>
      <a href="../manage/info_machine.php" class="nav-drawer__link">Machine</a>
      <a href="../manage/info_design_process.php" class="nav-drawer__link">Design Process</a>
      <a href="../manage/info_andon.php" class="nav-drawer__link">Andon</a>
      <a href="../manage/info_downtime.php" class="nav-drawer__link">Downtime</a>
      <a href="../manage/info_defective.php" class="nav-drawer__link">Defective</a>
      <a href="../manage/info_rate_color.php" class="nav-drawer__link">Rate Color</a>
      <a href="../manage/info_worktime.php" class="nav-drawer__link">Work Time</a>
    </div>
    <div class="nav-drawer__divider"></div>
    <div class="nav-drawer__group">
      <div class="nav-drawer__group-title">Monitoring</div>
      <a href="data_oee.php" class="nav-drawer__link">OEE Monitoring</a>
      <a href="data_andon.php" class="nav-drawer__link">Andon Monitoring</a>
      <a href="data_downtime.php" class="nav-drawer__link">Downtime Monitoring</a>
      <a href="data_defective.php" class="nav-drawer__link">Defective Monitoring</a>
    </div>
    <div class="nav-drawer__divider"></div>
    <div class="nav-drawer__group">
      <div class="nav-drawer__group-title">Report</div>
      <a href="log_oee.php" class="nav-drawer__link">OEE Report by Shift</a>
      <a href="log_oee_hourly.php" class="nav-drawer__link">OEE Report by Hourly</a>
      <a href="log_oee_row.php" class="nav-drawer__link">OEE Report by Row data</a>
    </div>
    <div class="nav-drawer__divider"></div>
    <a href="dashboard_2.php" class="nav-drawer__link nav-drawer__link--active">Dashboard</a>
    <a href="ai_dashboard_3.php" class="nav-drawer__link">AI Dashboard</a>
  </nav>
</div>

<!-- Signage Header (nav 대체, 52px) -->
<div class="signage-header">
  <button id="navDrawerBtn" class="nav-drawer-btn" aria-label="Menu">&#9776;</button>
  <span class="signage-header__title">OEE CS DASHBOARD</span>

  <div class="signage-header__filters">
    <select id="factoryFilterSelect" class="fiori-select">
      <option value="">All Factory</option>
    </select>
    <select id="factoryLineFilterSelect" class="fiori-select" disabled>
      <option value="">All Line</option>
    </select>
    <select id="factoryLineMachineFilterSelect" class="fiori-select" disabled>
      <option value="">All Machine</option>
    </select>
    <select id="timeRangeSelect" class="fiori-select">
      <option value="today" selected>Today</option>
      <option value="yesterday">Yesterday</option>
      <option value="1w">Last Week</option>
      <option value="1m">Last Month</option>
    </select>
    <input type="text" id="dateRangePicker" class="fiori-input" readonly placeholder="Select date range" />
    <select id="shiftSelect" class="fiori-select">
      <option value="">All Shift</option>
      <option value="1">Shift 1</option>
      <option value="2">Shift 2</option>
      <option value="3">Shift 3</option>
    </select>
    <button id="refreshBtn" class="fiori-btn fiori-btn--tertiary">Refresh</button>
  </div>

  <div class="signage-header__status">
    <div class="status-dot"></div>
    <span id="lastUpdateTime">--:--:--</span>
  </div>

  <div class="signage-header__clock" id="signageClock"></div>
</div>

<!-- Signage Main: 4행 CSS Grid -->
<div class="signage-main">

  <!-- Row A: OEE 4 metrics (3fr) + Active Andon Feed (1fr) -->
  <div class="signage-row-a">

    <!-- OEE 4 metrics -->
    <div class="signage-oee-metrics">

      <!-- Availability -->
      <div class="oee-metric-card oee-metric-card--availability">
        <div class="oee-metric-label">Availability</div>
        <div class="oee-metric-gauge">
          <canvas id="availabilityGauge" width="150" height="150"></canvas>
        </div>
        <div class="availability-metrics" style="width:100%">
          <div class="metric-row">
            <span class="metric-label" style="font-size:0.8rem">Runtime</span>
            <span class="metric-value" id="runtime-value">-</span>
          </div>
          <div class="metric-progress">
            <div class="progress-bar" id="runtime-progress" style="width:0%"></div>
          </div>
          <div class="metric-row">
            <span class="metric-label" style="font-size:0.8rem">Planned Time</span>
            <span class="metric-value" id="planned-time-value">-</span>
          </div>
          <div class="metric-progress">
            <div class="progress-bar" id="planned-progress" style="width:0%"></div>
          </div>
        </div>
        <div class="oee-metric-change">
          <span id="availabilityTrend">-</span>
          <span id="availabilityChange">vs Last Day</span>
        </div>
      </div>

      <!-- Performance -->
      <div class="oee-metric-card oee-metric-card--performance">
        <div class="oee-metric-label">Performance</div>
        <div class="oee-metric-gauge">
          <canvas id="performanceGauge" width="150" height="150"></canvas>
        </div>
        <div class="performance-metrics" style="width:100%">
          <div class="metric-row">
            <span class="metric-label" style="font-size:0.8rem">Actual Output</span>
            <span class="metric-value" id="actual-output-value">-</span>
          </div>
          <div class="metric-progress">
            <div class="progress-bar" id="actual-output-progress" style="width:0%"></div>
          </div>
          <div class="metric-row">
            <span class="metric-label" style="font-size:0.8rem">Theoretical Output</span>
            <span class="metric-value" id="theoretical-output-value">-</span>
          </div>
          <div class="metric-progress">
            <div class="progress-bar" id="theoretical-output-progress" style="width:0%"></div>
          </div>
        </div>
        <div class="oee-metric-change">
          <span id="performanceTrend">-</span>
          <span id="performanceChange">vs Last Day</span>
        </div>
      </div>

      <!-- Quality -->
      <div class="oee-metric-card oee-metric-card--quality">
        <div class="oee-metric-label">Quality</div>
        <div class="oee-metric-gauge">
          <canvas id="qualityGauge" width="150" height="150"></canvas>
        </div>
        <div class="quality-metrics" style="width:100%">
          <div class="metric-row">
            <span class="metric-label" style="font-size:0.8rem">Good Products</span>
            <span class="metric-value" id="good-products-value">-</span>
          </div>
          <div class="metric-progress">
            <div class="progress-bar" id="good-products-progress" style="width:0%"></div>
          </div>
          <div class="metric-row">
            <span class="metric-label" style="font-size:0.8rem">Defective Products</span>
            <span class="metric-value" id="defective-products-value">-</span>
          </div>
          <div class="metric-progress">
            <div class="progress-bar" id="defective-products-progress" style="width:0%"></div>
          </div>
        </div>
        <div class="oee-metric-change">
          <span id="qualityTrend">-</span>
          <span id="qualityChange">vs Last Day</span>
        </div>
      </div>

      <!-- Overall OEE -->
      <div class="oee-metric-card oee-metric-card--overall">
        <div class="oee-metric-label">OEE</div>
        <div class="oee-metric-gauge">
          <canvas id="overallGauge" width="150" height="150"></canvas>
        </div>
        <div class="oee-metric-change">
          <span id="overallTrend">-</span>
          <span id="overallChange">vs Last Day</span>
        </div>
      </div>

    </div><!-- /signage-oee-metrics -->

  </div><!-- /signage-row-a -->

  <!-- Row B: Downtime / Defective / Andon Warning Qty / Currently active Andon -->
  <div class="signage-row-b">

    <div class="fiori-card">
      <div class="fiori-card__header">
        <h3 class="fiori-card__title fiori-text-primary">Downtime</h3>
        <p class="fiori-card__subtitle fiori-text-secondary">By type</p>
      </div>
      <div class="fiori-card__content">
        <div style="flex:1; position:relative; min-height:0;">
          <canvas id="downtimeOccurrenceChart"></canvas>
        </div>
      </div>
    </div>

    <div class="fiori-card">
      <div class="fiori-card__header">
        <h3 class="fiori-card__title fiori-text-primary">Defective</h3>
        <p class="fiori-card__subtitle fiori-text-secondary">By type</p>
      </div>
      <div class="fiori-card__content">
        <div style="flex:1; position:relative; min-height:0;">
          <canvas id="defectiveOccurrenceChart"></canvas>
        </div>
      </div>
    </div>

    <div class="fiori-card">
      <div class="fiori-card__header">
        <h3 class="fiori-card__title fiori-text-primary">Andon Warning Qty</h3>
        <p class="fiori-card__subtitle fiori-text-secondary">By type</p>
      </div>
      <div class="fiori-card__content">
        <div style="flex:1; position:relative; min-height:0;">
          <canvas id="andonOccurrenceChart"></canvas>
        </div>
      </div>
    </div>

    <!-- Weekly Andon Warning (hidden — moved to Row B slot 4 with Andon Feed) -->
    <div style="display:none;">
      <canvas id="weeklyAndonTrendChart"></canvas>
    </div>

    <!-- Currently active Andon (moved from Row A) -->
    <div class="fiori-card">
      <div class="fiori-card__header">
        <h3 class="fiori-card__title fiori-text-primary">Currently active Andon</h3>
        <div class="real-time-status real-time-status-header">
          <div class="status-dot"></div>
          <span id="activeAndonCount">0 active alerts</span>
        </div>
      </div>
      <div class="fiori-card__content">
        <div id="andonAlarmFeed" style="flex:1; display:flex; flex-direction:column; overflow-y:auto;">
          <div class="fiori-alert fiori-alert--info">
            <strong>Info:</strong> No active Andon. Real-time monitoring active.
          </div>
        </div>
      </div>
    </div>

  </div><!-- /signage-row-b -->

  <!-- Row C: OEE Trend (2fr) + OEE Timeline (1fr) -->
  <div class="signage-row-c">

    <div class="fiori-card">
      <div class="fiori-card__header">
        <h3 class="fiori-card__title fiori-text-primary">OEE Trend</h3>
        <p class="fiori-card__subtitle fiori-text-secondary">Hourly OEE trend</p>
      </div>
      <div class="fiori-card__content">
        <div style="flex:1; position:relative; min-height:0;">
          <canvas id="oeeTrendChart"></canvas>
        </div>
      </div>
    </div>

    <div class="fiori-card">
      <div class="fiori-card__header">
        <h3 class="fiori-card__title fiori-text-primary">OEE Timeline</h3>
        <p class="fiori-card__subtitle fiori-text-secondary" id="timelineSubtitle">Hourly OEE timeline</p>
      </div>
      <div class="fiori-card__content">
        <div class="production-timeline">
          <div class="timeline-header" id="timelineHeader"></div>
          <div class="timeline-bars">
            <div class="timeline-bar" id="productionTimeline"></div>
          </div>
          <div class="timeline-legend">
            <div class="legend-item">
              <div style="padding:4px; color:var(--sap-text-secondary); font-size:10px;">Loading legend...</div>
            </div>
          </div>
        </div>
      </div>
    </div>

  </div><!-- /signage-row-c -->

  <!-- Row D: Production Heatmap (full width) -->
  <div class="signage-row-d">

    <div class="fiori-card">
      <div class="fiori-card__header">
        <h3 class="fiori-card__title fiori-text-primary">Production Heatmap</h3>
        <p class="fiori-card__subtitle fiori-text-secondary">Production distribution by day and time</p>
      </div>
      <div class="fiori-card__content">
        <div class="heatmap">
          <div class="heatmap-grid" id="productionHeatmap"></div>
          <div class="heatmap-axis">
            <span>08:00</span>
            <span>12:00</span>
            <span>16:00</span>
            <span>20:00</span>
          </div>
        </div>
      </div>
    </div>

  </div><!-- /signage-row-d -->

</div><!-- /signage-main -->

<!-- JavaScript Libraries -->
<script src="../../assets/js/chart.js"></script>
<script src="../../assets/js/jquery-3.6.1.min.js"></script>
<script src="../../assets/js/moment.min.js"></script>
<script src="../../assets/js/daterangepicker.js"></script>

<!-- Dashboard JS (100% 재사용) -->
<script src="js/dashboard.js"></script>

<!-- 햄버거 드로어 -->
<script>
  (function() {
    var btn = document.getElementById('navDrawerBtn');
    var drawer = document.getElementById('navDrawer');
    var overlay = document.getElementById('navDrawerOverlay');
    function open() { drawer.classList.add('is-open'); overlay.classList.add('is-open'); }
    function close() { drawer.classList.remove('is-open'); overlay.classList.remove('is-open'); }
    btn.addEventListener('click', function() { drawer.classList.contains('is-open') ? close() : open(); });
    overlay.addEventListener('click', close);
  })();
</script>

<!-- 사이니지 전용: 실시간 시계 -->
<script>
  (function() {
    function updateClock() {
      var now = new Date();
      var h = String(now.getHours()).padStart(2, '0');
      var m = String(now.getMinutes()).padStart(2, '0');
      var s = String(now.getSeconds()).padStart(2, '0');
      var el = document.getElementById('signageClock');
      if (el) el.textContent = h + ':' + m + ':' + s;
    }
    updateClock();
    setInterval(updateClock, 1000);
  })();
</script>

</body>

</html>