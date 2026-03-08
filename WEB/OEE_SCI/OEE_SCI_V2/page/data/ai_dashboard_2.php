<?php
/**
 * ai_dashboard_2.php — 1920x1080 Signage AI Dashboard
 * ai_dashboard.php 의 사이니지 전용 버전
 * - nav 제거, 슬림 헤더 52px
 * - CSS Grid 4행 고정 레이아웃 (overflow: hidden)
 * - js/ai_dashboard.js, ai_stream_monitor.js, ai_optimization.js 100% 재사용
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

<!-- Signage Header (nav 대체, 52px) -->
<div class="signage-header">
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
    <button id="aiRefreshBtn" class="fiori-btn fiori-btn--tertiary">Refresh</button>
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

  <!-- Row A: AI Summary 카드 4개 -->
  <div class="ai-signage-row-a">
    <div class="ai-summary-grid">

      <!-- OEE 예측 -->
      <div class="ai-summary-card ai-summary-card--prediction">
        <span class="ai-summary-card__label">Next 4H OEE Forecast</span>
        <div class="ai-summary-card__value" id="aiPredCurrentOee">--</div>
        <div class="ai-summary-card__sub" id="aiPredSub">
          <span class="ai-spinner"></span>
        </div>
        <div>
          <span class="ai-trend-badge ai-trend-badge--stable" id="aiPredTrendBadge">--</span>
        </div>
      </div>

      <!-- 이상 감지 -->
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

      <!-- 위험 기계 -->
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

      <!-- 라인 건강지수 -->
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

    <!-- OEE Trend & AI Forecast 차트 -->
    <div class="fiori-card">
      <div class="fiori-card__header">
        <div>
          <h3 class="fiori-card__title fiori-text-primary">OEE Trend & AI Forecast</h3>
          <p class="fiori-card__subtitle fiori-text-secondary">Solid = Actual / Dashed = AI Forecast (90% CI)</p>
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

    <!-- 실시간 이상 감지 패널 -->
    <div class="fiori-card">
      <div class="fiori-card__header">
        <div>
          <h3 class="fiori-card__title fiori-text-primary">Anomaly Detection</h3>
          <p class="fiori-card__subtitle fiori-text-secondary">Z-Score based real-time detection</p>
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

    <!-- 라인별 AI 건강지수 -->
    <div class="fiori-card">
      <div class="fiori-card__header">
        <div>
          <h3 class="fiori-card__title fiori-text-primary">Line Health Index</h3>
          <p class="fiori-card__subtitle fiori-text-secondary">Based on 7-day OEE average per line</p>
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

    <!-- 예방정비 권고 목록 -->
    <div class="fiori-card">
      <div class="fiori-card__header">
        <div>
          <h3 class="fiori-card__title fiori-text-primary">Predictive Maintenance</h3>
          <p class="fiori-card__subtitle fiori-text-secondary">Machines ranked by risk score (high to low)</p>
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

    <!-- Real-time AI Streaming -->
    <div class="fiori-card">
      <div class="fiori-card__header">
        <div>
          <h3 class="fiori-card__title fiori-text-primary">
            Real-time AI Streaming
            <span class="ai-badge">LIVE</span>
          </h3>
          <p class="fiori-card__subtitle fiori-text-secondary">Anomaly, downtime, maintenance risk events</p>
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

    <!-- Production Optimization -->
    <div class="fiori-card">
      <div class="fiori-card__header">
        <div>
          <h3 class="fiori-card__title fiori-text-primary">
            Production Optimization
            <span class="ai-badge">AI</span>
          </h3>
          <p class="fiori-card__subtitle fiori-text-secondary">OEE bottleneck analysis · Improvement opportunities (last 14 days)</p>
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

<!-- AI Dashboard JS (100% 재사용) -->
<script src="js/ai_dashboard.js"></script>
<script src="js/ai_stream_monitor.js"></script>
<script src="js/ai_optimization.js"></script>

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
