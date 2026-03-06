<?php
/* error_reporting(E_ALL);
ini_set('display_errors', 1);
$page_title = 'SCI OEE Dashboard'; */

$page_title = 'SCI OEE Dashboard';
$page_css_files = ['../../assets/css/fiori-page.css', '../../assets/css/daterangepicker.css', 'css/dashboard.css'];

require_once(__DIR__ . '/../../inc/head.php');
require_once(__DIR__ . '/../../inc/nav-fiori.php');
?>

<div class="fiori-container">
  <main>

    <!-- Main Header with Filters -->
    <div class="fiori-main-header">
      <div style="display: flex; justify-content: space-between; align-items: center; width: 100%;">
        <h2>AI OEE CS DASHBOARD</h2>
        <div class="real-time-status">
          <div class="status-dot"></div>
          <span>Real-time data connected</span>
          <span id="lastUpdateTime">Last updated: --:--:--</span>
        </div>
      </div>

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
            <select id="timeRangeSelect" class="fiori-select">
              <option value="today" selected>Today</option>
              <option value="yesterday">Yesterday</option>
              <option value="1w">Last Week</option>
              <option value="1m">Last Month</option>
            </select>
          </div>
          <div>
            <input type="text" id="dateRangePicker" class="fiori-input" readonly placeholder="Select date range" />
          </div>
          <div>
            <select id="shiftSelect" class="fiori-select">
              <option value="">All Shift</option>
              <option value="1">Shift 1</option>
              <option value="2">Shift 2</option>
              <option value="3">Shift 3</option>
            </select>
          </div>
          <div>
            <button id="refreshBtn" class="fiori-btn fiori-btn--tertiary">🔄 Refresh</button>
          </div>
        </div>
      </div>
    </div>

    <!-- OEE Core Metrics Cards -->
    <section class="oee-dashboard-section">
      <div class="oee-metrics-grid">
        
        <!-- Availability Card -->
        <div class="oee-metric-card oee-metric-card--availability">
          <div class="oee-metric-label">Availability</div>
          <div class="oee-metric-gauge">
            <canvas id="availabilityGauge" width="150" height="150"></canvas>
          </div>
          <div class="availability-metrics" style="width:100%">
            <div class="metric-row">
              <span class="metric-label" style="font-size: 0.875rem;">Runtime</span>
              <span class="metric-value" id="runtime-value">-</span>
            </div>
            <div class="metric-progress">
              <div class="progress-bar" id="runtime-progress" style="width: 0%;"></div>
            </div>
            <div class="metric-row">
              <span class="metric-label" style="font-size: 0.875rem;">Planned Production Time</span>
              <span class="metric-value" id="planned-time-value">-</span>
            </div>
            <div class="metric-progress">
              <div class="progress-bar" id="planned-progress" style="width: 0%;"></div>
            </div>
          </div>
          <div class="oee-metric-change">
            <span id="availabilityTrend">-</span>
            <span id="availabilityChange">vs Last Day</span>
          </div>
        </div>

        <!-- Performance Card -->
        <div class="oee-metric-card oee-metric-card--performance">
          <div class="oee-metric-label">Performance</div>
          <div class="oee-metric-gauge">
            <canvas id="performanceGauge" width="150" height="150"></canvas>
          </div>
          <div class="performance-metrics" style="width:100%">
            <div class="metric-row">
              <span class="metric-label" style="font-size: 0.875rem;">Actual Output</span>
              <span class="metric-value" id="actual-output-value">-</span>
            </div>
            <div class="metric-progress">
              <div class="progress-bar" id="actual-output-progress" style="width: 0%;"></div>
            </div>
            <div class="metric-row">
              <span class="metric-label" style="font-size: 0.875rem;">Theoretical Output</span>
              <span class="metric-value" id="theoretical-output-value">-</span>
            </div>
            <div class="metric-progress">
              <div class="progress-bar" id="theoretical-output-progress" style="width: 0%;"></div>
            </div>
          </div>
          <div class="oee-metric-change">
            <span id="performanceTrend">-</span>
            <span id="performanceChange">vs Last Day</span>
          </div>
        </div>

        <!-- Quality Card -->
        <div class="oee-metric-card oee-metric-card--quality">
          <div class="oee-metric-label">Quality</div>
          <div class="oee-metric-gauge">
            <canvas id="qualityGauge" width="150" height="150"></canvas>
          </div>
          <div class="quality-metrics" style="width:100%">
            <div class="metric-row">
              <span class="metric-label" style="font-size: 0.875rem;">Good Products</span>
              <span class="metric-value" id="good-products-value">-</span>
            </div>
            <div class="metric-progress">
              <div class="progress-bar" id="good-products-progress" style="width: 0%;"></div>
            </div>
            <div class="metric-row">
              <span class="metric-label" style="font-size: 0.875rem;">Defective Products</span>
              <span class="metric-value" id="defective-products-value">-</span>
            </div>
            <div class="metric-progress">
              <div class="progress-bar" id="defective-products-progress" style="width: 0%;"></div>
            </div>
          </div>
          <div class="oee-metric-change">
            <span id="qualityTrend">-</span>
            <span id="qualityChange">vs Last Day</span>
          </div>
        </div>

        <!-- Overall OEE Card -->
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

      </div>
    </section>    

    <!-- Downtime & Defective Status Section -->
    <section class="oee-dashboard-section" style="margin-top: var(--sap-spacing-3xl);">
      
      <div class="dashboard-grid" style="display: grid; grid-template-columns: repeat(auto-fit, minmax(350px, 1fr)); gap: var(--sap-spacing-lg); margin-bottom: var(--sap-spacing-3xl);">
        
        <!-- Downtime Status Card -->
        <div class="fiori-card" style="height: 400px; display: flex; flex-direction: column;">
          <div class="fiori-card__header">
            <h3 class="fiori-card__title fiori-text-primary">Downtime</h3>
            <p class="fiori-card__subtitle fiori-text-secondary">Downtime occurrence status by type</p>
          </div>
          <div class="fiori-card__content" style="flex: 1; padding: var(--sap-spacing-md); display: flex; flex-direction: column;">
            <div style="flex: 1; position: relative; min-height: 280px;">
              <canvas id="downtimeOccurrenceChart"></canvas>
            </div>
          </div>
        </div>

        <!-- Defective Status Card -->
        <div class="fiori-card" style="height: 400px; display: flex; flex-direction: column;">
          <div class="fiori-card__header">
            <h3 class="fiori-card__title fiori-text-primary">Defective</h3>
            <p class="fiori-card__subtitle fiori-text-secondary">Defective occurrence status by type</p>
          </div>
          <div class="fiori-card__content" style="flex: 1; padding: var(--sap-spacing-md); display: flex; flex-direction: column;">
            <div style="flex: 1; position: relative; min-height: 280px;">
              <canvas id="defectiveOccurrenceChart"></canvas>
            </div>
          </div>
        </div>

      </div>
    </section>

    <!-- Andon Monitoring Section -->
    <section class="oee-dashboard-section" style="margin-top: var(--sap-spacing-3xl);">

      <div class="dashboard-grid" style="display: grid; grid-template-columns: 8fr 5fr 7fr; gap: var(--sap-spacing-lg); margin-bottom: var(--sap-spacing-3xl);">

        <!-- Currently active Andon -->
        <div class="fiori-card" style="height: 400px; display: flex; flex-direction: column;">
          <div class="fiori-card__header">
            <h3 class="fiori-card__title fiori-text-primary">🔥 Currently active Andon</h3>
            <div class="real-time-status real-time-status-header">
              <div class="status-dot"></div>
              <span id="activeAndonCount">0 active alerts</span>
            </div>
          </div>
          <div class="fiori-card__content" style="flex: 1; padding: var(--sap-spacing-md); display: flex; flex-direction: column;">
            <div id="andonAlarmFeed" style="flex: 1; display: flex; flex-direction: column; gap: var(--sap-spacing-sm); overflow-y: auto;">
              <!-- Real-time active andon data loaded here -->
              <div class="fiori-alert fiori-alert--info">
                <strong>ℹ️ Information:</strong> There are currently no active Andon. Real-time monitoring is active.
              </div>
            </div>
          </div>
        </div>

        <!-- Andon Occurrence Analysis Card -->
        <div class="fiori-card" style="height: 400px; display: flex; flex-direction: column;">
          <div class="fiori-card__header">
            <h3 class="fiori-card__title fiori-text-primary">Andon Warning Quantity</h3>
            <p class="fiori-card__subtitle fiori-text-secondary">Andon Warning Status by Type</p>
          </div>
          <div class="fiori-card__content" style="flex: 1; padding: var(--sap-spacing-md); display: flex; flex-direction: column;">
            <div style="flex: 1; position: relative; min-height: 280px;">
              <canvas id="andonOccurrenceChart"></canvas>
            </div>
          </div>
        </div>

        <!-- Weekly Andon Trend Card -->
        <div class="fiori-card" style="height: 400px; display: flex; flex-direction: column;">
          <div class="fiori-card__header">
            <h3 class="fiori-card__title fiori-text-primary">Weekly Andon Warning Status</h3>
            <p class="fiori-card__subtitle fiori-text-secondary">Andon Warning trend over the last 7 days</p>
          </div>
          <div class="fiori-card__content" style="flex: 1; padding: var(--sap-spacing-md); display: flex; flex-direction: column;">
            <div style="flex: 1; position: relative; min-height: 280px;">
              <canvas id="weeklyAndonTrendChart"></canvas>
            </div>
          </div>
        </div>

      </div>
    </section>

    <!-- Production Analysis - OEE Trend & Production Timeline -->
    <section class="oee-dashboard-section" style="margin-top: var(--sap-spacing-3xl);">

      <!-- OEE Trend (Full Width) -->
      <div class="fiori-card" style="height: 400px; display: flex; flex-direction: column; margin-bottom: var(--sap-spacing-xl);">
        <div class="fiori-card__header">
          <h3 class="fiori-card__title fiori-text-primary">📈 OEE Trend</h3>
          <p class="fiori-card__subtitle fiori-text-secondary">Hourly OEE trend</p>
        </div>
        <div class="fiori-card__content" style="flex: 1; padding: var(--sap-spacing-md); display: flex; flex-direction: column;">
          <div style="flex: 1; position: relative; min-height: 280px;">
            <canvas id="oeeTrendChart"></canvas>
          </div>
        </div>
      </div>

      <!-- Production Timeline (Full Width) -->
      <div class="fiori-card" style="height: 400px; display: flex; flex-direction: column; margin-bottom: var(--sap-spacing-xl);">
        <div class="fiori-card__header">
          <h3 class="fiori-card__title fiori-text-primary">📅 OEE Timeline</h3>
          <p class="fiori-card__subtitle fiori-text-secondary" id="timelineSubtitle">Hourly OEE timeline</p>
        </div>
        <div class="fiori-card__content" style="flex: 1; padding: var(--sap-spacing-md);">
          <div class="production-timeline">
            <div class="timeline-header" id="timelineHeader">
              <!-- Dynamically generated by JavaScript based on date range -->
            </div>
            <div class="timeline-bars">
              <div class="timeline-bar" id="productionTimeline">
                <!-- Real-time timeline bars generated dynamically -->
              </div>
            </div>
            <div class="timeline-legend">
              <!-- Dynamically generated by JavaScript based on info_rate_color table -->
              <div class="legend-item">
                <div style="padding: var(--sap-spacing-sm); color: var(--sap-text-secondary); font-size: var(--sap-font-size-sm);">
                  Loading legend...
                </div>
              </div>
            </div>
          </div>
        </div>
      </div>

      <!-- Production Heatmap (Full Width) -->
      <div class="fiori-card" style="margin-bottom: var(--sap-spacing-xl);">
        <div class="fiori-card__header">
          <h3 class="fiori-card__title fiori-text-primary">🔥 Production Heatmap</h3>
          <p class="fiori-card__subtitle fiori-text-secondary">Distribution of production by day and time zone</p>
        </div>
        <div class="fiori-card__content">
          <div class="heatmap">
            <div class="heatmap-grid" id="productionHeatmap">
              <!-- Heatmap data generated dynamically -->
            </div>
            <div class="heatmap-axis">
              <span>08:00</span>
              <span>12:00</span>
              <span>16:00</span>
              <span>20:00</span>
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
<!-- <script src="https://cdn.jsdelivr.net/npm/chart.js"></script> -->
<script src="../../assets/js/chart.js"></script>
<script src="../../assets/js/jquery-3.6.1.min.js"></script>
<script src="../../assets/js/moment.min.js"></script>
<script src="../../assets/js/daterangepicker.js"></script>

<!-- Dashboard JavaScript -->
<script src="js/dashboard.js"></script>

</body>
</html>