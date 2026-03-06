<?php
$page_title = 'AI Manufacturing Report';
$page_css_files = ['../../assets/css/fiori-style.css', '../../assets/css/fiori-icons.css', '../../assets/css/fiori-page.css', '../../assets/css/daterangepicker.css', 'css/report.css'];

require_once(__DIR__ . '/../../inc/head.php');
require_once(__DIR__ . '/../../inc/nav-fiori.php');
?>

<div class="fiori-container">
  <main>
    
    <!-- AI Report Hero Section -->
    <section class="ai-report-hero">
      <div class="hero-content">
        <h1><i class="sap-icon sap-icon--ai sap-icon--md"></i> AI-Powered Manufacturing Report</h1>
        <p>Advanced Analytics & Predictive Insights for SUNTECH OEE Optimization</p>
        <div class="ai-status-indicators">
          <div class="ai-indicator">
            <div class="status-dot status-dot--success"></div>
            <span><i class="sap-icon sap-icon--brain sap-icon--xs"></i> AI Engine: Active</span>
          </div>
          <div class="ai-indicator">
            <div class="status-dot status-dot--success"></div>
            <span><i class="sap-icon sap-icon--analytics sap-icon--xs"></i> Real-time Analytics: Connected</span>
          </div>
          <div class="ai-indicator">
            <div class="status-dot status-dot--success"></div>
            <span id="lastAnalysisTime">Last Analysis: Loading...</span>
          </div>
        </div>
      </div>
    </section>

    <!-- Smart Filtering System -->
    <section class="smart-filter-section">
      <div class="fiori-card">
        <div class="fiori-card__header">
          <h3><i class="sap-icon sap-icon--filter sap-icon--sm"></i> Smart Filtering & Analysis Controls</h3>
        </div>
        <div class="fiori-card__content">
          
          <!-- Basic 3-Level Filters -->
          <div class="filter-group">
            <h4><i class="sap-icon sap-icon--machine sap-icon--xs"></i> Location Filters</h4>
            <div class="basic-filters">
              <div>
                <label for="factoryFilterSelect" class="fiori-label">Factory</label>
                <select id="factoryFilterSelect" class="fiori-select">
                  <option value="">All Factory</option>
                  <!-- JS로 동적 생성 -->
                </select>
              </div>
              <div>
                <label for="factoryLineFilterSelect" class="fiori-label">Line</label>
                <select id="factoryLineFilterSelect" class="fiori-select" disabled>
                  <option value="">All Line</option>
                  <!-- JS로 동적 생성 -->
                </select>
              </div>
              <div>
                <label for="factoryLineMachineFilterSelect" class="fiori-label">Machine</label>
                <select id="factoryLineMachineFilterSelect" class="fiori-select" disabled>
                  <option value="">All Machine</option>
                  <!-- JS로 동적 생성 -->
                </select>
              </div>
            </div>
          </div>

          <!-- Time Filters -->
          <div class="filter-group">
            <h4><i class="sap-icon sap-icon--time sap-icon--xs"></i> Time Filters</h4>
            <div class="time-filters">
              <div>
                <label for="timeRangeSelect" class="fiori-label">Quick Range</label>
                <select id="timeRangeSelect" class="fiori-select">
                  <option value="1h">Last 1 Hour</option>
                  <option value="4h">Last 4 Hours</option>
                  <option value="8h">Last 8 Hours</option>
                  <option value="today" selected>Today</option>
                  <option value="yesterday">Yesterday</option>
                  <option value="1w">Last Week</option>
                  <option value="1m">Last Month</option>
                </select>
              </div>
              <div>
                <label for="dateRangePicker" class="fiori-label">Custom Range</label>
                <input type="text" id="dateRangePicker" class="fiori-input" readonly placeholder="Select custom date range" />
              </div>
              <div>
                <label for="shiftSelect" class="fiori-label">Shift</label>
                <select id="shiftSelect" class="fiori-select">
                  <option value="">All Shift</option>
                  <option value="1">Shift 1</option>
                  <option value="2">Shift 2</option>
                  <option value="3">Shift 3</option>
                </select>
              </div>
            </div>
          </div>

          <!-- Advanced AI Filters -->
          <div class="filter-group">
            <h4><i class="sap-icon sap-icon--ai sap-icon--xs"></i> AI Advanced Filters</h4>
            <div class="advanced-filters">
              <div>
                <label for="performanceFilter" class="fiori-label">Performance Grade</label>
                <select id="performanceFilter" class="fiori-select">
                  <option value="">All Performance</option>
                  <option value="excellent">Excellent (≥85%)</option>
                  <option value="good">Good (70-85%)</option>
                  <option value="fair">Fair (50-70%)</option>
                  <option value="poor">Poor (<50%)</option>
                </select>
              </div>
              <div>
                <label for="defectRateFilter" class="fiori-label">Defect Rate</label>
                <select id="defectRateFilter" class="fiori-select">
                  <option value="">All Defect Rates</option>
                  <option value="low">Low (<2%)</option>
                  <option value="medium">Medium (2-5%)</option>
                  <option value="high">High (>5%)</option>
                </select>
              </div>
              <div>
                <label for="downtimeFilter" class="fiori-label">Downtime Level</label>
                <select id="downtimeFilter" class="fiori-select">
                  <option value="">All Downtime</option>
                  <option value="short">Short (<15min)</option>
                  <option value="medium">Medium (15-60min)</option>
                  <option value="long">Long (>1hour)</option>
                </select>
              </div>
              <div>
                <label for="timeGranularity" class="fiori-label">Analysis Granularity</label>
                <select id="timeGranularity" class="fiori-select">
                  <option value="hourly" selected>Hourly</option>
                  <option value="daily">Daily</option>
                  <option value="weekly">Weekly</option>
                  <option value="monthly">Monthly</option>
                </select>
              </div>
            </div>
          </div>

          <!-- Action Buttons -->
          <div class="filter-actions">
            <button id="startAIAnalysisBtn" class="fiori-btn fiori-btn--primary"><i class="sap-icon sap-icon--analytics sap-icon--xs"></i> Start AI Analysis</button>
            <button id="exportReportBtn" class="fiori-btn fiori-btn--secondary"><i class="sap-icon sap-icon--export sap-icon--xs"></i> Export Report</button>
            <button id="refreshDataBtn" class="fiori-btn fiori-btn--tertiary"><i class="sap-icon sap-icon--refresh sap-icon--xs"></i> Refresh</button>
            <button id="resetFiltersBtn" class="fiori-btn fiori-btn--tertiary"><i class="sap-icon sap-icon--reset sap-icon--xs"></i> Reset Filters</button>
          </div>
          
        </div>
      </div>
    </section>

    <!-- AI Insights Dashboard -->
    <section class="ai-insights-section">
      <div class="ai-insights-grid">
        
        <!-- Predictive Analytics Card -->
        <div class="insight-card insight-card--predictive">
          <div class="insight-header">
            <h3><i class="sap-icon sap-icon--prediction sap-icon--sm"></i> Predictive Analytics</h3>
            <div class="insight-status">
              <span class="status-badge status-badge--processing" id="predictionStatus">Analyzing...</span>
            </div>
          </div>
          <div class="insight-content" id="predictiveContent">
            <div class="prediction-item">
              <div class="prediction-label">Next Week OEE Forecast</div>
              <div class="prediction-value" id="oeeForecasts7d">--%</div>
              <div class="prediction-trend" id="oeeTrend7d">Loading...</div>
            </div>
            <div class="prediction-item">
              <div class="prediction-label">Next Month OEE Forecast</div>
              <div class="prediction-value" id="oeeForecasts30d">--%</div>
              <div class="prediction-trend" id="oeeTrend30d">Loading...</div>
            </div>
            <div class="prediction-item">
              <div class="prediction-label">Equipment Risk Assessment</div>
              <div class="prediction-value" id="equipmentRisk">--</div>
              <div class="prediction-trend" id="riskTrend">Loading...</div>
            </div>
          </div>
        </div>

        <!-- Anomaly Detection Card -->
        <div class="insight-card insight-card--anomaly">
          <div class="insight-header">
            <h3><i class="sap-icon sap-icon--anomaly sap-icon--sm"></i> Anomaly Detection</h3>
            <div class="insight-status">
              <span class="status-badge status-badge--warning" id="anomalyStatus">0 Anomalies</span>
            </div>
          </div>
          <div class="insight-content" id="anomalyContent">
            <div class="anomaly-list" id="anomalyList">
              <div class="no-anomalies">No anomalies detected in current data range</div>
            </div>
          </div>
        </div>

        <!-- Correlation Analysis Card -->
        <div class="insight-card insight-card--correlation">
          <div class="insight-header">
            <h3><i class="sap-icon sap-icon--correlation sap-icon--sm"></i> Correlation Analysis</h3>
            <div class="insight-status">
              <span class="status-badge status-badge--info" id="correlationStatus">Ready</span>
            </div>
          </div>
          <div class="insight-content" id="correlationContent">
            <div class="correlation-item">
              <div class="correlation-factors">OEE ↔ Defect Rate</div>
              <div class="correlation-value" id="oeeDefectCorr">-0.--</div>
            </div>
            <div class="correlation-item">
              <div class="correlation-factors">Availability ↔ Downtime</div>
              <div class="correlation-value" id="availDownCorr">-0.--</div>
            </div>
            <div class="correlation-item">
              <div class="correlation-factors">Performance ↔ Quality</div>
              <div class="correlation-value" id="perfQualCorr">-0.--</div>
            </div>
          </div>
        </div>

        <!-- AI Recommendations Card -->
        <div class="insight-card insight-card--optimization">
          <div class="insight-header">
            <h3><i class="sap-icon sap-icon--optimization sap-icon--sm"></i> AI Recommendations</h3>
            <div class="insight-status">
              <span class="status-badge status-badge--success" id="recommendationStatus">Ready</span>
            </div>
          </div>
          <div class="insight-content" id="recommendationContent">
            <div class="recommendation-list" id="recommendationList">
              <div class="no-recommendations">Analyzing data to generate recommendations...</div>
            </div>
          </div>
        </div>

      </div>
    </section>

    <!-- Phase 2: Multi-KPI Gauge Dashboard -->
    <section class="multi-kpi-dashboard">
      <div class="fiori-card">
        <div class="fiori-card__header">
          <h3><i class="sap-icon sap-icon--gauge sap-icon--sm"></i> Real-time KPI Gauges</h3>
          <div class="kpi-dashboard-controls">
            <button id="toggleKpiViewBtn" class="fiori-btn fiori-btn--tertiary"><i class="sap-icon sap-icon--switch-view sap-icon--xs"></i> Switch View</button>
            <button id="kpiFullscreenBtn" class="fiori-btn fiori-btn--tertiary"><i class="sap-icon sap-icon--fullscreen sap-icon--xs"></i> Fullscreen</button>
          </div>
        </div>
        <div class="fiori-card__content">
          <div class="kpi-gauges-grid">
            
            <!-- OEE Gauge -->
            <div class="kpi-gauge-container">
              <div class="kpi-gauge-header">
                <h4>Overall Equipment Effectiveness</h4>
                <div class="kpi-target">Target: 85%</div>
              </div>
              <div class="kpi-gauge-chart">
                <canvas id="oeeGauge" width="200" height="200"></canvas>
              </div>
              <div class="kpi-gauge-details">
                <div class="kpi-value" id="oeeCurrentValue">--%</div>
                <div class="kpi-status" id="oeeStatus">Loading...</div>
                <div class="kpi-trend" id="oeeTrend">--</div>
              </div>
            </div>

            <!-- Availability Gauge -->
            <div class="kpi-gauge-container">
              <div class="kpi-gauge-header">
                <h4>Machine Availability</h4>
                <div class="kpi-target">Target: 90%</div>
              </div>
              <div class="kpi-gauge-chart">
                <canvas id="availabilityGauge" width="200" height="200"></canvas>
              </div>
              <div class="kpi-gauge-details">
                <div class="kpi-value" id="availabilityCurrentValue">--%</div>
                <div class="kpi-status" id="availabilityStatus">Loading...</div>
                <div class="kpi-trend" id="availabilityTrend">--</div>
              </div>
            </div>

            <!-- Performance Gauge -->
            <div class="kpi-gauge-container">
              <div class="kpi-gauge-header">
                <h4>Performance Rate</h4>
                <div class="kpi-target">Target: 85%</div>
              </div>
              <div class="kpi-gauge-chart">
                <canvas id="performanceGauge" width="200" height="200"></canvas>
              </div>
              <div class="kpi-gauge-details">
                <div class="kpi-value" id="performanceCurrentValue">--%</div>
                <div class="kpi-status" id="performanceStatus">Loading...</div>
                <div class="kpi-trend" id="performanceTrend">--</div>
              </div>
            </div>

            <!-- Quality Gauge -->
            <div class="kpi-gauge-container">
              <div class="kpi-gauge-header">
                <h4>Quality Rate</h4>
                <div class="kpi-target">Target: 95%</div>
              </div>
              <div class="kpi-gauge-chart">
                <canvas id="qualityGauge" width="200" height="200"></canvas>
              </div>
              <div class="kpi-gauge-details">
                <div class="kpi-value" id="qualityCurrentValue">--%</div>
                <div class="kpi-status" id="qualityStatus">Loading...</div>
                <div class="kpi-trend" id="qualityTrend">--</div>
              </div>
            </div>

            <!-- Defect Rate Gauge -->
            <div class="kpi-gauge-container">
              <div class="kpi-gauge-header">
                <h4>Defect Rate</h4>
                <div class="kpi-target">Target: <2%</div>
              </div>
              <div class="kpi-gauge-chart">
                <canvas id="defectRateGauge" width="200" height="200"></canvas>
              </div>
              <div class="kpi-gauge-details">
                <div class="kpi-value" id="defectRateCurrentValue">--%</div>
                <div class="kpi-status" id="defectRateStatus">Loading...</div>
                <div class="kpi-trend" id="defectRateTrend">--</div>
              </div>
            </div>

            <!-- Downtime Gauge -->
            <div class="kpi-gauge-container">
              <div class="kpi-gauge-header">
                <h4>Downtime Rate</h4>
                <div class="kpi-target">Target: <5%</div>
              </div>
              <div class="kpi-gauge-chart">
                <canvas id="downtimeGauge" width="200" height="200"></canvas>
              </div>
              <div class="kpi-gauge-details">
                <div class="kpi-value" id="downtimeCurrentValue">--%</div>
                <div class="kpi-status" id="downtimeStatus">Loading...</div>
                <div class="kpi-trend" id="downtimeTrend">--</div>
              </div>
            </div>

          </div>
        </div>
      </div>
    </section>

    <!-- Advanced Visualization Section -->
    <section class="advanced-charts-section">
      <div class="charts-grid">
        
        <!-- Performance Heat Map -->
        <div class="chart-container">
          <div class="chart-header">
            <h3><i class="sap-icon sap-icon--heatmap sap-icon--sm"></i> Performance Heat Map</h3>
            <div class="chart-controls">
              <select id="heatmapMetric" class="fiori-select fiori-select--compact">
                <option value="oee">OEE</option>
                <option value="availability">Availability</option>
                <option value="performance">Performance</option>
                <option value="quality">Quality</option>
              </select>
            </div>
          </div>
          <div class="chart-content">
            <canvas id="performanceHeatMap" width="400" height="300"></canvas>
          </div>
        </div>

        <!-- Predictive Trend Chart -->
        <div class="chart-container">
          <div class="chart-header">
            <h3><i class="sap-icon sap-icon--trend-up sap-icon--sm"></i> Predictive Trend Analysis</h3>
            <div class="chart-controls">
              <button id="togglePredictionBtn" class="fiori-btn fiori-btn--compact">Show Prediction</button>
            </div>
          </div>
          <div class="chart-content">
            <canvas id="predictiveTrendChart" width="400" height="300"></canvas>
          </div>
        </div>

        <!-- Correlation Matrix -->
        <div class="chart-container">
          <div class="chart-header">
            <h3><i class="sap-icon sap-icon--correlation sap-icon--sm"></i> Factor Correlation Matrix</h3>
            <div class="chart-legend">
              <span class="legend-item">
                <span class="legend-color" style="background: #da1e28;"></span>
                Strong Negative
              </span>
              <span class="legend-item">
                <span class="legend-color" style="background: #30914c;"></span>
                Strong Positive
              </span>
            </div>
          </div>
          <div class="chart-content">
            <canvas id="correlationMatrix" width="400" height="300"></canvas>
          </div>
        </div>

        <!-- Pareto Analysis -->
        <div class="chart-container">
          <div class="chart-header">
            <h3><i class="sap-icon sap-icon--chart-bar sap-icon--sm"></i> Pareto Analysis</h3>
            <div class="chart-controls">
              <select id="paretoCategory" class="fiori-select fiori-select--compact">
                <option value="andon">Andon Types</option>
                <option value="defective">Defective Types</option>
                <option value="downtime">Downtime Types</option>
              </select>
            </div>
          </div>
          <div class="chart-content">
            <canvas id="paretoChart" width="400" height="300"></canvas>
          </div>
        </div>

      </div>
    </section>

    <!-- Data Summary Tables -->
    <section class="data-summary-section">
      <div class="fiori-card">
        <div class="fiori-card__header">
          <h3><i class="sap-icon sap-icon--table sap-icon--sm"></i> Integrated Data Summary</h3>
          <div class="summary-actions">
            <button id="toggleTableView" class="fiori-btn fiori-btn--tertiary"><i class="sap-icon sap-icon--toggle sap-icon--xs"></i> Toggle View</button>
            <button id="exportTableData" class="fiori-btn fiori-btn--tertiary"><i class="sap-icon sap-icon--download sap-icon--xs"></i> Export Data</button>
          </div>
        </div>
        <div class="fiori-card__content">
          <div class="summary-tabs">
            <button class="tab-btn active" data-tab="integrated">Integrated View</button>
            <button class="tab-btn" data-tab="oee">OEE Data</button>
            <button class="tab-btn" data-tab="andon">Andon Data</button>
            <button class="tab-btn" data-tab="defective">Defective Data</button>
            <button class="tab-btn" data-tab="downtime">Downtime Data</button>
          </div>
          <div class="summary-content">
            <div id="integratedTable" class="data-table-wrapper active">
              <!-- 데이터 테이블 영역 -->
              <table class="fiori-table" id="integratedDataTable">
                <thead>
                  <tr>
                    <th>Machine</th>
                    <th>OEE %</th>
                    <th>Availability %</th>
                    <th>Performance %</th>
                    <th>Quality %</th>
                    <th>Active Issues</th>
                    <th>Status</th>
                  </tr>
                </thead>
                <tbody>
                  <!-- JS로 동적 생성 -->
                </tbody>
              </table>
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

<!-- JavaScript 라이브러리 -->
<script src="../assets/js/jquery-3.6.1.min.js"></script>
<script src="../assets/js/moment.min.js"></script>
<script src="../assets/js/daterangepicker.js"></script>
<script src="../assets/js/chart.js"></script>
<script src="js/report.js"></script>

<?php require_once(__DIR__ . '/../inc/foot.php'); ?>