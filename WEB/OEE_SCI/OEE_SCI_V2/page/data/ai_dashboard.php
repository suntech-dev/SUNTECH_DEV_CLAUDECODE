<?php
$page_title    = 'AI Intelligence Dashboard';
$page_css_files = [
  '../../assets/css/fiori-page.css',
  '../../assets/css/daterangepicker.css',
  'css/dashboard.css',
  'css/ai_dashboard.css',
];

require_once(__DIR__ . '/../../inc/head.php');
require_once(__DIR__ . '/../../inc/nav-fiori.php');
?>

<div class="fiori-container">
  <main>

    <!-- Main Header -->
    <div class="fiori-main-header">
      <div style="display: flex; justify-content: space-between; align-items: center; width: 100%;">
        <h2>
          AI Intelligence Dashboard
          <span class="ai-badge">AI POWERED</span>
        </h2>
        <div class="ai-last-update">
          <div class="ai-pulse-dot"></div>
          <span id="aiLastUpdateTime">Initializing...</span>
        </div>
      </div>

      <!-- Filters -->
      <div class="fiori-card__content filter-header-content">
        <div class="filter-section">
          <div>
            <select id="factoryFilterSelect" class="fiori-select">
              <option value="">All Factory</option>
            </select>
          </div>
          <div>
            <select id="factoryLineFilterSelect" class="fiori-select" disabled>
              <option value="">All Line</option>
            </select>
          </div>
          <div>
            <select id="factoryLineMachineFilterSelect" class="fiori-select" disabled>
              <option value="">All Machine</option>
            </select>
          </div>
          <div>
            <button id="aiRefreshBtn" class="fiori-btn fiori-btn--tertiary">Refresh</button>
            <a href="ai_dashboard_manual_kor.html" target="_blank" class="fiori-btn fiori-btn--ghost" style="margin-left:8px; text-decoration:none;">Help</a>
          </div>
        </div>
      </div>
    </div>

    <!-- ===== 1. AI 요약 카드 4개 ===== -->
    <section class="oee-dashboard-section">
      <div class="ai-summary-grid">

        <!-- OEE 예측 카드 -->
        <div class="ai-summary-card ai-summary-card--prediction">
          <span class="ai-summary-card__label">Next 4H OEE Forecast</span>
          <div class="ai-summary-card__value" id="aiPredCurrentOee">--</div>
          <div class="ai-summary-card__sub" id="aiPredSub">
            <span class="ai-spinner"></span>
          </div>
          <div style="margin-top: var(--sap-spacing-sm);">
            <span class="ai-trend-badge ai-trend-badge--stable" id="aiPredTrendBadge">--</span>
          </div>
        </div>

        <!-- 이상 감지 카드 -->
        <div class="ai-summary-card ai-summary-card--anomaly">
          <span class="ai-summary-card__label">Anomaly Detection</span>
          <div class="ai-summary-card__value" id="aiAnomalyTotal">--</div>
          <div class="ai-summary-card__sub" id="aiAnomalySub">
            <span class="ai-spinner"></span>
          </div>
          <div style="margin-top: var(--sap-spacing-sm);">
            <span id="aiAnomalyCriticalBadge" class="ai-status-badge" style="display:none;"></span>
          </div>
        </div>

        <!-- 위험 기계 카드 -->
        <div class="ai-summary-card ai-summary-card--maintenance">
          <span class="ai-summary-card__label">High-Risk Machines</span>
          <div class="ai-summary-card__value" id="aiMaintDanger">--</div>
          <div class="ai-summary-card__sub" id="aiMaintSub">
            <span class="ai-spinner"></span>
          </div>
          <div style="margin-top: var(--sap-spacing-sm);">
            <span id="aiMaintWarnBadge" class="ai-status-badge ai-status-badge--warning" style="display:none;"></span>
          </div>
        </div>

        <!-- 라인 건강지수 카드 -->
        <div class="ai-summary-card ai-summary-card--health">
          <span class="ai-summary-card__label">Line Health Index (Avg)</span>
          <div class="ai-summary-card__value" id="aiHealthAvg">--</div>
          <div class="ai-summary-card__sub" id="aiHealthSub">
            <span class="ai-spinner"></span>
          </div>
          <div style="margin-top: var(--sap-spacing-sm);">
            <span id="aiHealthStatusBadge" class="ai-status-badge" style="display:none;"></span>
          </div>
        </div>

      </div>
    </section>

    <!-- ===== 2. OEE 예측 차트 + 이상 감지 패널 ===== -->
    <section class="oee-dashboard-section">
      <div class="ai-main-grid">

        <!-- OEE 트렌드 + 예측선 차트 -->
        <div class="fiori-card" style="display: flex; flex-direction: column;">
          <div class="fiori-card__header">
            <div>
              <h3 class="fiori-card__title fiori-text-primary">OEE Trend & AI Forecast</h3>
              <p class="fiori-card__subtitle fiori-text-secondary">Solid line = Actual / Dashed line = AI Forecast (90% CI)</p>
            </div>
          </div>
          <div class="fiori-card__content" style="flex: 1; padding: var(--sap-spacing-md);">
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
        <div class="fiori-card" style="display: flex; flex-direction: column;">
          <div class="fiori-card__header">
            <div>
              <h3 class="fiori-card__title fiori-text-primary">Anomaly Detection</h3>
              <p class="fiori-card__subtitle fiori-text-secondary">Z-Score based real-time detection</p>
            </div>
            <div id="aiAnomalyHeaderCount" class="ai-last-update" style="display:none;">
              <div class="ai-pulse-dot" style="background: var(--sap-negative);"></div>
              <span id="aiAnomalyHeaderText"></span>
            </div>
          </div>
          <div class="fiori-card__content" style="flex: 1; padding: var(--sap-spacing-md);">
            <div class="ai-anomaly-list" id="aiAnomalyList">
              <div class="ai-empty-state">
                <div class="ai-spinner"></div>
                <span>Analyzing anomalies...</span>
              </div>
            </div>
          </div>
        </div>

      </div>
    </section>

    <!-- ===== 3. 라인 건강지수 + 예방정비 순위 ===== -->
    <section class="oee-dashboard-section">
      <div class="ai-bottom-grid">

        <!-- 라인별 AI 건강지수 -->
        <div class="fiori-card">
          <div class="fiori-card__header">
            <div>
              <h3 class="fiori-card__title fiori-text-primary">Line Health Index</h3>
              <p class="fiori-card__subtitle fiori-text-secondary">Based on 7-day OEE average per line</p>
            </div>
          </div>
          <div class="fiori-card__content" style="padding: var(--sap-spacing-md);">
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
          <div class="fiori-card__content" style="padding: var(--sap-spacing-md);">
            <div class="ai-maintenance-list" id="aiMaintenanceList">
              <div class="ai-empty-state">
                <div class="ai-spinner"></div>
                <span>Calculating risk scores...</span>
              </div>
            </div>
          </div>
        </div>

      </div>
    </section>

    <!-- ===== 4. F12 — Real-time AI Streaming Analysis ===== -->
    <section class="oee-dashboard-section">
      <div class="fiori-card">
        <div class="fiori-card__header">
          <div>
            <h3 class="fiori-card__title fiori-text-primary">
              📡 Real-time AI Streaming Analysis
              <span class="ai-badge">LIVE</span>
            </h3>
            <p class="fiori-card__subtitle fiori-text-secondary">
              Receive anomaly detection, downtime, and maintenance risk events in real-time
            </p>
          </div>
          <div class="ai-last-update">
            <div class="ai-pulse-dot" id="aiStreamDot" style="background: #e67e22;"></div>
            <span id="aiStreamStatus">Connecting...</span>
            <span id="aiStreamCount" style="font-size:0.8rem; color:var(--sap-text-secondary); margin-left:8px;">0 events</span>
          </div>
        </div>
        <div class="fiori-card__content" style="padding: var(--sap-spacing-md);">
          <div id="aiStreamFeed" class="ai-stream-feed">
            <div class="ai-stream-empty">
              <span class="ai-spinner"></span> Connecting to AI stream...
            </div>
          </div>
        </div>
      </div>
    </section>

    <!-- ===== 5. F13 — Production Optimization ===== -->
    <section class="oee-dashboard-section">
      <div class="fiori-card">
        <div class="fiori-card__header">
          <div>
            <h3 class="fiori-card__title fiori-text-primary">
              🎯 Production Optimization
              <span class="ai-badge">AI</span>
            </h3>
            <p class="fiori-card__subtitle fiori-text-secondary">
              OEE bottleneck component analysis · Prioritized improvement opportunities (last 14 days)
            </p>
          </div>
          <div id="aiOptSummary" class="ai-opt-summary-bar"></div>
        </div>
        <div class="fiori-card__content" style="padding: var(--sap-spacing-md);">
          <div id="aiOptList" class="ai-opt-list">
            <div class="ai-empty-state">
              <span class="ai-spinner"></span> Analyzing optimization opportunities...
            </div>
          </div>
        </div>
      </div>
    </section>

  </main>
</div>

<footer class="fiori-footer">
  <p>&copy; 2025 SUNTECH. All Rights Reserved.</p>
</footer>

<!-- JavaScript Libraries -->
<script src="../../assets/js/chart.js"></script>
<script src="../../assets/js/jquery-3.6.1.min.js"></script>
<script src="../../assets/js/moment.min.js"></script>
<script src="../../assets/js/common.js"></script>

<!-- AI Dashboard JavaScript -->
<script src="js/ai_dashboard.js"></script>
<script src="js/ai_stream_monitor.js"></script>
<script src="js/ai_optimization.js"></script>

</body>
</html>
