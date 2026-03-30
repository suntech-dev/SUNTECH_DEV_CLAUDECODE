<?php

/**
 * ai_dashboard_2.php — 1920x1080 Signage AI Dashboard
 * ai_dashboard_4.php 개선본
 *
 * 버그 수정:
 *  [P1] Real-time OEE 카드: current_oee 항상 오늘 기준, min/max 클램핑 적용
 *  [P1] OEE Forecast 차트: Actual OEE solid 라인 추가 (today_data 활용)
 *  [P2] CI 범위 0~100% 방지: seasonal_std 상한 15% 적용
 *  [P2] Predictive Maintenance: OEE 100% 초과 값 SQL 클램핑으로 오판정 수정
 *  [P3] Production Optimization: date_range 파라미터 연동
 *  [P3] Line Health Index: 서브타이틀을 date_range에 맞게 동적 표시
 *
 * 연관 파일:
 *  - css/ai_dashboard_2.css
 *  - js/ai_dashboard_2.js
 *  - js/ai_optimization_2.js
 *  - proc/ai_oee_prediction_5.php
 *  - proc/ai_maintenance_5.php
 *  - proc/ai_optimization_2.php
 */

$page_title = 'AI Intelligence Dashboard - Signage';
$page_css_files = [
  '../../assets/css/fiori-page.css',
  'css/dashboard.css',
  'css/ai_dashboard.css',
  'css/ai_dashboard_2.css',
];

require_once(__DIR__ . '/../../inc/head.php');
/* nav-fiori.php 제거 */
?>

<?php $nav_context = 'data';
$nav_active = 'ai_dashboard';
require_once(__DIR__ . '/../../inc/nav-drawer-manage.php'); ?>

<!-- Hamburger Drawer -->
<!-- <div id="navDrawerOverlay" class="nav-drawer-overlay"></div>
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
    <a href="dashboard_2.php" class="nav-drawer__link">Dashboard</a>
    <a href="ai_dashboard_3.php" class="nav-drawer__link">AI Dashboard v3</a>
    <a href="ai_dashboard_4.php" class="nav-drawer__link">AI Dashboard v4</a>
    <a href="ai_dashboard_2.php" class="nav-drawer__link nav-drawer__link--active">AI Dashboard</a>
  </nav>
</div> -->

<!-- Signage Header -->
<div class="signage-header">
  <button id="navDrawerBtn" class="nav-drawer-btn" aria-label="Menu">&#9776;</button>
  <span class="signage-header__title">
    AI Intelligence Dashboard
    <span class="ai-badge">AI POWERED</span>
  </span>

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
    <select id="dateRangeSelect" class="fiori-select date-range-select">
      <option value="today" selected>Today</option>
      <option value="yesterday">Yesterday</option>
      <option value="7d">Last 7 Days</option>
      <option value="30d">Last 30 Days</option>
    </select>
    <button id="aiRefreshBtn" class="fiori-btn fiori-btn--tertiary">Refresh</button>
    <button id="exportReportBtn" class="fiori-btn fiori-btn--ghost">Export</button>
    <a href="ai_dashboard_manual_kor.html" target="_blank" class="fiori-btn fiori-btn--ghost" style="text-decoration:none;">Help</a>
  </div>

  <div class="signage-header__status">
    <div class="ai-pulse-dot"></div>
    <span id="aiLastUpdateTime">Initializing...</span>
  </div>

  <div class="signage-header__clock" id="signageClock"></div>
</div>

<!-- AI Signage Main: 4행 CSS Grid -->
<div class="ai-signage-main">

  <!-- Row A: AI Summary 카드 5개 -->
  <div class="ai-signage-row-a">
    <div class="ai-summary-grid">

      <!-- Real-time OEE -->
      <div class="ai-summary-card ai-summary-card--realtime">
        <div>
          <div class="ai-realtime-live">
            <div class="ai-realtime-live__dot"></div>
            LIVE
          </div>
          <span class="ai-summary-card__label" style="margin-top:2px;">Real-time OEE</span>
        </div>
        <div class="ai-summary-card__value" id="aiRealtimeOee">--</div>
        <div class="ai-summary-card__sub" id="aiRealtimeSub">
          <span class="ai-spinner"></span>
        </div>
        <div>
          <span id="aiRealtimeBadge" class="ai-status-badge" style="display:none;"></span>
        </div>
      </div>

      <!-- Next 4H AI Forecast -->
      <div class="ai-summary-card ai-summary-card--prediction">
        <span class="ai-summary-card__label">Next 4H AI Forecast</span>
        <div class="ai-summary-card__value" id="aiPredForecastOee">--</div>
        <div class="ai-summary-card__sub" id="aiPredSub">
          <span class="ai-spinner"></span>
        </div>
        <div>
          <span class="ai-trend-badge ai-trend-badge--stable" id="aiPredTrendBadge">--</span>
        </div>
      </div>

      <!-- Anomaly Detection -->
      <div class="ai-summary-card ai-summary-card--anomaly">
        <span class="ai-summary-card__label">Anomaly Detection</span>
        <div class="ai-summary-card__value" id="aiAnomalyTotal">--</div>
        <div class="ai-summary-card__sub" id="aiAnomalySub">
          <span class="ai-spinner"></span>
        </div>
        <div>
          <span id="aiAnomalyCriticalBadge" class="ai-status-badge" style="display:none;"></span>
        </div>
      </div>

      <!-- High-Risk Machines -->
      <div class="ai-summary-card ai-summary-card--maintenance">
        <span class="ai-summary-card__label">High-Risk Machines</span>
        <div class="ai-summary-card__value" id="aiMaintDanger">--</div>
        <div class="ai-summary-card__sub" id="aiMaintSub">
          <span class="ai-spinner"></span>
        </div>
        <div>
          <span id="aiMaintWarnBadge" class="ai-status-badge ai-status-badge--warning" style="display:none;"></span>
        </div>
      </div>

      <!-- Line Health Index -->
      <div class="ai-summary-card ai-summary-card--health">
        <span class="ai-summary-card__label">Line Health Index (Avg)</span>
        <div class="ai-summary-card__value" id="aiHealthAvg">--</div>
        <div class="ai-summary-card__sub" id="aiHealthSub">
          <span class="ai-spinner"></span>
        </div>
        <div>
          <span id="aiHealthStatusBadge" class="ai-status-badge" style="display:none;"></span>
        </div>
      </div>

    </div>
  </div><!-- /ai-signage-row-a -->

  <!-- Row B: OEE Forecast (2fr) + Anomaly Detection (1fr) -->
  <div class="ai-signage-row-b">

    <div class="fiori-card">
      <div class="fiori-card__header">
        <div class="card-title-row">
          <h3 class="fiori-card__title fiori-text-primary">OEE Trend & AI Forecast</h3>
          <span class="card-subtitle-inline fiori-text-secondary">Solid = Actual &nbsp;/&nbsp; Dashed = AI Forecast (90% CI)</span>
        </div>
      </div>
      <div class="fiori-card__content">
        <div class="ai-prediction-chart-wrap">
          <canvas id="aiOeeForecastChart"></canvas>
        </div>
        <div class="ai-chart-legend">
          <div class="ai-chart-legend__item">
            <div class="ai-chart-legend__dot ai-chart-legend__dot--actual"></div>
            <span>Actual OEE</span>
          </div>
          <div class="ai-chart-legend__item">
            <div class="ai-chart-legend__dot ai-chart-legend__dot--forecast"></div>
            <span>AI Forecast</span>
          </div>
          <div class="ai-chart-legend__item">
            <div class="ai-chart-legend__dot ai-chart-legend__dot--ci" style="width:20px;height:8px;border-radius:2px;"></div>
            <span>Confidence Interval (90%)</span>
          </div>
        </div>
      </div>
    </div>

    <div class="fiori-card">
      <div class="fiori-card__header">
        <div class="card-title-row">
          <h3 class="fiori-card__title fiori-text-primary">Anomaly Detection</h3>
          <span class="card-subtitle-inline fiori-text-secondary">Z-Score based real-time detection</span>
        </div>
        <div id="aiAnomalyHeaderCount" class="ai-last-update" style="display:none;">
          <div class="ai-pulse-dot" style="background:var(--sap-negative);"></div>
          <span id="aiAnomalyHeaderText"></span>
        </div>
      </div>
      <div class="fiori-card__content">
        <div class="ai-anomaly-list" id="aiAnomalyList">
          <div class="ai-empty-state">
            <div class="ai-spinner"></div>
            <span>Analyzing anomalies...</span>
          </div>
        </div>
      </div>
    </div>

  </div><!-- /ai-signage-row-b -->

  <!-- Row C: Line Health (1fr) + Predictive Maintenance (1fr) -->
  <div class="ai-signage-row-c">

    <div class="fiori-card">
      <div class="fiori-card__header">
        <div class="card-title-row">
          <h3 class="fiori-card__title fiori-text-primary">Line Health Index</h3>
          <!-- .ai-health-subtitle 는 JS 에서 date_range 에 맞게 동적으로 업데이트 -->
          <span class="card-subtitle-inline ai-health-subtitle">Based on 7-day OEE average per line</span>
        </div>
      </div>
      <div class="fiori-card__content">
        <div class="ai-health-list" id="aiHealthList">
          <div class="ai-empty-state">
            <div class="ai-spinner"></div>
            <span>Calculating health index...</span>
          </div>
        </div>
      </div>
    </div>

    <div class="fiori-card">
      <div class="fiori-card__header">
        <div class="card-title-row">
          <h3 class="fiori-card__title fiori-text-primary">Predictive Maintenance</h3>
          <span class="card-subtitle-inline fiori-text-secondary">Machines ranked by risk score (high to low)</span>
        </div>
      </div>
      <div class="fiori-card__content">
        <div class="ai-maintenance-list" id="aiMaintenanceList">
          <div class="ai-empty-state">
            <div class="ai-spinner"></div>
            <span>Calculating risk scores...</span>
          </div>
        </div>
      </div>
    </div>

  </div><!-- /ai-signage-row-c -->

  <!-- Row D: AI Streaming (1fr) + Production Optimization (2fr) -->
  <div class="ai-signage-row-d">

    <div class="fiori-card">
      <div class="fiori-card__header">
        <div class="card-title-row">
          <h3 class="fiori-card__title fiori-text-primary">
            Real-time AI Streaming
            <span class="ai-badge">LIVE</span>
          </h3>
          <span class="card-subtitle-inline fiori-text-secondary">Anomaly, downtime, maintenance risk events</span>
        </div>
        <div class="ai-last-update">
          <div class="ai-pulse-dot" id="aiStreamDot" style="background:#e67e22;"></div>
          <span id="aiStreamStatus">Connecting...</span>
          <span id="aiStreamCount" style="font-size:0.75rem;color:var(--sap-text-secondary);margin-left:6px;">0 events</span>
        </div>
      </div>
      <div class="fiori-card__content">
        <div id="aiStreamFeed" class="ai-stream-feed">
          <div class="ai-stream-empty">
            <span class="ai-spinner"></span> Connecting to AI stream...
          </div>
        </div>
      </div>
    </div>

    <div class="fiori-card">
      <div class="fiori-card__header">
        <div class="card-title-row">
          <h3 class="fiori-card__title fiori-text-primary">
            Production Optimization
            <span class="ai-badge">AI</span>
          </h3>
          <!-- 분석 기간이 date_range 에 따라 달라짐을 명시 -->
          <span class="card-subtitle-inline fiori-text-secondary">OEE bottleneck analysis · Improvement opportunities (period by filter)</span>
        </div>
        <div id="aiOptSummary" class="ai-opt-summary-bar"></div>
      </div>
      <div class="fiori-card__content">
        <div id="aiOptList" class="ai-opt-list">
          <div class="ai-empty-state">
            <span class="ai-spinner"></span> Analyzing optimization opportunities...
          </div>
        </div>
      </div>
    </div>

  </div><!-- /ai-signage-row-d -->

</div><!-- /ai-signage-main -->

<!-- JavaScript Libraries -->
<script src="../../assets/js/chart.js"></script>
<script src="../../assets/js/jquery-3.6.1.min.js"></script>
<script src="../../assets/js/moment.min.js"></script>
<script src="../../assets/js/common.js"></script>

<!-- AI Dashboard JS -->
<script src="js/ai_dashboard_2.js"></script>
<script src="js/ai_stream_monitor.js"></script>
<script src="js/ai_optimization_2.js"></script>

<!-- 날짜 필터 getFilterParams() 확장 -->
<script>
  (function() {
    var _origGetFilterParams = getFilterParams;
    getFilterParams = function() {
      var p = _origGetFilterParams();
      var sel = document.getElementById('dateRangeSelect');
      if (sel && sel.value) p.date_range = sel.value;
      return p;
    };
    document.getElementById('dateRangeSelect').addEventListener('change', function() {
      if (typeof refreshAll === 'function') refreshAll();
    });
  })();
</script>

<!-- loadPrediction() 오버라이드 — ai_oee_prediction_5.php 호출 -->
<script>
  (function() {
    loadPrediction = function() {
      $.getJSON('proc/ai_oee_prediction_5.php', getFilterParams(), function(data) {
        if (data.code !== '00') {
          $('#aiRealtimeSub').text('API error');
          $('#aiPredSub').text('API error');
          return;
        }

        /* ── 카드1: Real-time OEE (항상 오늘 기준, 클램핑 완료) ── */
        var curOee = (data.current_oee !== null && data.current_oee !== undefined) ?
          parseFloat(data.current_oee) : null;

        if (curOee !== null) {
          var colorClass = curOee >= 85 ? 'ai-oee-good' : (curOee >= 60 ? 'ai-oee-warning' : 'ai-oee-danger');
          $('#aiRealtimeOee')
            .text(curOee + '%')
            .removeClass('ai-oee-good ai-oee-warning ai-oee-danger')
            .addClass(colorClass);

          var badgeText = curOee >= 85 ? 'GOOD' : (curOee >= 60 ? 'WARNING' : 'CRITICAL');
          var badgeCls = curOee >= 85 ? 'ai-status-badge--normal' : (curOee >= 60 ? 'ai-status-badge--warning' : 'ai-status-badge--danger');
          $('#aiRealtimeBadge')
            .show()
            .text(badgeText)
            .removeClass('ai-status-badge--normal ai-status-badge--warning ai-status-badge--danger')
            .addClass(badgeCls);
        } else {
          $('#aiRealtimeOee').text('--').removeClass('ai-oee-good ai-oee-warning ai-oee-danger');
          $('#aiRealtimeBadge').hide();
        }

        var hourLabel = (data.current_hour !== undefined && data.current_hour !== null) ?
          'Current: ' + String(data.current_hour).padStart(2, '0') + ':00' :
          'Current hour';
        $('#aiRealtimeSub').text(hourLabel);

        /* ── 카드2: Next 4H AI Forecast ──────────────── */
        var forecastAvg = null;
        var ciMin = null,
          ciMax = null;

        if (data.forecast && data.forecast.length > 0) {
          var sum = 0;
          data.forecast.forEach(function(f) {
            sum += parseFloat(f.oee);
          });
          forecastAvg = (sum / data.forecast.length).toFixed(1);

          ciMin = Math.min.apply(null, data.forecast.map(function(f) {
            return parseFloat(f.lower);
          })).toFixed(1);
          ciMax = Math.max.apply(null, data.forecast.map(function(f) {
            return parseFloat(f.upper);
          })).toFixed(1);

          $('#aiPredForecastOee').text(forecastAvg + '%');
          $('#aiPredSub').text('CI: ' + ciMin + '% ~ ' + ciMax + '%');
        } else {
          $('#aiPredForecastOee').text('--');
          $('#aiPredSub').text('Insufficient data');
        }

        /* ── 트렌드 배지 ──────────────────────────────── */
        var trendMap = {
          up: {
            cls: 'ai-trend-badge--up',
            text: 'Trending Up'
          },
          down: {
            cls: 'ai-trend-badge--down',
            text: 'Trending Down'
          },
          stable: {
            cls: 'ai-trend-badge--stable',
            text: 'Stable'
          },
        };
        var trend = trendMap[data.trend] || trendMap.stable;
        $('#aiPredTrendBadge')
          .removeClass('ai-trend-badge--up ai-trend-badge--down ai-trend-badge--stable')
          .addClass(trend.cls)
          .text(trend.text);

        /* ── OEE Forecast 차트 (today_data 포함) & 라인 건강지수 ── */
        renderForecastChart(data);
        loadLineHealth();

      }).fail(function() {
        $('#aiRealtimeSub').text('API error');
        $('#aiPredSub').text('API error');
      });
    };
  })();
</script>

<!-- Export Report Modal -->
<div id="exportModal" style="display:none;position:fixed;inset:0;z-index:9999;background:rgba(0,0,0,.6);align-items:center;justify-content:center;">
  <div style="background:#161b22;border:1px solid #30363d;border-radius:10px;padding:24px 28px;min-width:340px;box-shadow:0 8px 32px #000a;">
    <div style="font-size:1rem;font-weight:600;color:#58a6ff;margin-bottom:16px;">Export Report — Select Period</div>

    <div style="display:flex;gap:8px;margin-bottom:16px;flex-wrap:wrap;">
      <button class="export-preset fiori-btn fiori-btn--tertiary" data-range="today">Today</button>
      <button class="export-preset fiori-btn fiori-btn--tertiary" data-range="yesterday">Yesterday</button>
      <button class="export-preset fiori-btn fiori-btn--tertiary" data-range="1w">Last 7 Days</button>
      <button class="export-preset fiori-btn fiori-btn--tertiary" data-range="1m">Last 30 Days</button>
    </div>

    <div style="font-size:.8rem;color:#8b949e;margin-bottom:6px;">Custom Range</div>
    <div style="display:flex;gap:8px;align-items:center;margin-bottom:20px;">
      <input type="date" id="exportDateFrom" class="fiori-input" style="flex:1;background:#21262d;color:#e6edf3;border:1px solid #30363d;border-radius:4px;padding:5px 8px;">
      <span style="color:#8b949e;">~</span>
      <input type="date" id="exportDateTo" class="fiori-input" style="flex:1;background:#21262d;color:#e6edf3;border:1px solid #30363d;border-radius:4px;padding:5px 8px;">
    </div>

    <div style="display:flex;gap:8px;justify-content:flex-end;">
      <button id="exportCancelBtn" class="fiori-btn fiori-btn--ghost">Cancel</button>
      <button id="exportConfirmBtn" class="fiori-btn fiori-btn--primary">Export</button>
    </div>
  </div>
</div>

<script>
  (function() {
    var modal = document.getElementById('exportModal');

    function fmtDate(d) {
      return d.toISOString().slice(0, 10);
    }

    function calcRange(range) {
      var now = new Date(),
        ms = 86400000;
      var map = {
        today: {
          from: fmtDate(now),
          to: fmtDate(now)
        },
        yesterday: {
          from: fmtDate(new Date(now - ms)),
          to: fmtDate(new Date(now - ms))
        },
        '1w': {
          from: fmtDate(new Date(now - 6 * ms)),
          to: fmtDate(now)
        },
        '1m': {
          from: fmtDate(new Date(now - 29 * ms)),
          to: fmtDate(now)
        },
      };
      return map[range] || map['today'];
    }

    function setPreset(range) {
      var r = calcRange(range);
      document.getElementById('exportDateFrom').value = r.from;
      document.getElementById('exportDateTo').value = r.to;
      document.querySelectorAll('.export-preset').forEach(function(b) {
        b.classList.toggle('fiori-btn--emphasized', b.dataset.range === range);
        b.classList.toggle('fiori-btn--tertiary', b.dataset.range !== range);
      });
    }

    document.getElementById('exportReportBtn').addEventListener('click', function() {
      setPreset('today');
      modal.style.display = 'flex';
    });

    document.querySelectorAll('.export-preset').forEach(function(btn) {
      btn.addEventListener('click', function() {
        setPreset(this.dataset.range);
      });
    });

    ['exportDateFrom', 'exportDateTo'].forEach(function(id) {
      document.getElementById(id).addEventListener('change', function() {
        document.querySelectorAll('.export-preset').forEach(function(b) {
          b.classList.remove('fiori-btn--emphasized');
          b.classList.add('fiori-btn--tertiary');
        });
      });
    });

    document.getElementById('exportConfirmBtn').addEventListener('click', function() {
      var from = document.getElementById('exportDateFrom').value;
      var to = document.getElementById('exportDateTo').value;
      if (!from || !to) {
        alert('Please select a date range.');
        return;
      }
      var p = getFilterParams();
      p.range = 'custom';
      p.date_from = from;
      p.date_to = to;
      window.open('proc/ai_report_export.php?' + new URLSearchParams(p), '_blank');
      modal.style.display = 'none';
    });

    document.getElementById('exportCancelBtn').addEventListener('click', function() {
      modal.style.display = 'none';
    });

    modal.addEventListener('click', function(e) {
      if (e.target === modal) modal.style.display = 'none';
    });
  })();
</script>

<!-- 햄버거 드로어 -->
<!-- <script>
  (function() {
    var btn = document.getElementById('navDrawerBtn');
    var drawer = document.getElementById('navDrawer');
    var overlay = document.getElementById('navDrawerOverlay');

    function open() {
      drawer.classList.add('is-open');
      overlay.classList.add('is-open');
    }

    function close() {
      drawer.classList.remove('is-open');
      overlay.classList.remove('is-open');
    }

    btn.addEventListener('click', function() {
      drawer.classList.contains('is-open') ? close() : open();
    });
    overlay.addEventListener('click', close);
  })();
</script> -->

<!-- 실시간 시계 -->
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