<?php
$page_title = 'OEE Data Monitoring';
$page_css_files = ['../../assets/css/fiori-page.css', '../../assets/css/daterangepicker.css', 'css/data_oee.css'];

require_once(__DIR__ . '/../../inc/head.php');
require_once(__DIR__ . '/../../inc/nav-fiori.php');
?>

<div class="fiori-container">
  <main>
    
    <div class="fiori-main-header">
      <div>
        <h2>OEE Monitoring</h2>
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
            <button id="excelDownloadBtn" class="fiori-btn fiori-btn--primary">📊 Export</button>
            <button id="refreshBtn" class="fiori-btn fiori-btn--tertiary">🔄 Refresh</button>
            <button id="toggleStatsBtn" class="fiori-btn fiori-btn--secondary">📊 Hide Stats</button>
            <button id="toggleChartsBtn" class="fiori-btn fiori-btn--secondary">📈 Hide Charts</button>
          </div>
        </div>
      </div>
    </div>

    <div class="oee-monitoring-grid" id="statsGrid">
      <div class="stat-card stat-card--red">
        <div class="stat-value" id="overallOee">-</div>
        <div class="stat-label">📈 Overall OEE</div>
      </div>
      <div class="stat-card stat-card--success">
        <div class="stat-value" id="availability">-</div>
        <div class="stat-label">Availability</div>
      </div>
      <div class="stat-card stat-card--info">
        <div class="stat-value" id="performance">-</div>
        <div class="stat-label">Performance</div>
      </div>
      <div class="stat-card stat-card--warning">
        <div class="stat-value" id="quality">-</div>
        <div class="stat-label">Quality</div>
      </div>
      <div class="stat-card stat-card--maroon">
        <div class="stat-value" id="currentShiftOee">-</div>
        <div class="stat-label">Current Shift OEE</div>
      </div>
      <div class="stat-card">
        <div class="stat-value" id="previousDayOee">-</div>
        <div class="stat-label">📅 Previous Day OEE</div>
      </div>
    </div>

    <div class="oee-main-layout">
      <div class="fiori-section oee-details-section">
        <div class="fiori-card">
          <div class="fiori-card__header">
            <h3 class="fiori-card__title">📊 OEE Components Details</h3>
            <div class="real-time-status real-time-status-header">
              <div class="status-dot"></div>
              <span id="oeeLiveStatus">Real-time monitoring active</span>
            </div>
          </div>
          <div class="fiori-card__content">
            <div id="oeeDetailsContainer">
              <div class="oee-component-item">
                <div class="oee-component-info">
                  <div class="oee-component-name">OEE Rate</div>
                  <div class="oee-component-details">
                    <span class="oee-detail-item">Overall Efficiency: <span id="overallEfficiency">-</span></span>
                    <span class="oee-detail-item">Target Achievement: <span id="targetAchievement">-</span></span>
                  </div>
                </div>
                <div class="oee-component-value">
                  <span id="oeeRateDetail">-</span>
                </div>
              </div>

              <div class="oee-component-item">
                <div class="oee-component-info">
                  <div class="oee-component-name">Availability Rate</div>
                  <div class="oee-component-details">
                    <span class="oee-detail-item">Runtime: <span id="runtime">-</span>h</span>
                    <span class="oee-detail-item">Planned Time: <span id="plannedTime">-</span>h</span>
                  </div>
                </div>
                <div class="oee-component-value">
                  <span id="availabilityDetail">-</span>
                </div>
              </div>

              <div class="oee-component-item">
                <div class="oee-component-info">
                  <div class="oee-component-name">Performance Rate</div>
                  <div class="oee-component-details">
                    <span class="oee-detail-item">Actual Output: <span id="actualOutput">-</span></span>
                    <span class="oee-detail-item">Theoretical Output: <span id="theoreticalOutput">-</span></span>
                  </div>
                </div>
                <div class="oee-component-value">
                  <span id="performanceDetail">-</span>
                </div>
              </div>

              <div class="oee-component-item">
                <div class="oee-component-info">
                  <div class="oee-component-name">Quality Rate</div>
                  <div class="oee-component-details">
                    <span class="oee-detail-item">Good Products: <span id="goodProducts">-</span></span>
                    <span class="oee-detail-item">Defective: <span id="defectiveProducts">-</span></span>
                  </div>
                </div>
                <div class="oee-component-value">
                  <span id="qualityDetail">-</span>
                </div>
              </div>
            </div>
          </div>
        </div>
      </div>

      <div class="oee-charts-section">
        <div class="fiori-card oee-chart-small">
          <div class="fiori-card__header">
            <h3 class="fiori-card__title">📊 OEE Components</h3>
            <p class="fiori-card__subtitle">Availability, Performance, Quality comparison</p>
          </div>
          <div class="fiori-card__content">
            <div class="chart-container">
              <canvas id="oeeComponentChart"></canvas>
            </div>
          </div>
        </div>
        
        <div class="fiori-card oee-chart-small oee-grade-chart">
          <div class="fiori-card__header">
            <h3 class="fiori-card__title">📊 OEE Performance Grade Distribution</h3>
            <p class="fiori-card__subtitle">Distribution by performance grades</p>
          </div>
          <div class="fiori-card__content">
            <div class="chart-container">
              <canvas id="oeeGradeChart"></canvas>
            </div>
          </div>
        </div>
      </div>
    </div>

    <div class="oee-additional-charts oee-charts-full">
      <div class="fiori-card oee-chart-card">
        <div class="fiori-card__header">
          <h3 class="fiori-card__title fiori-text-primary">📈 OEE Trend</h3>
          <p class="fiori-card__subtitle fiori-text-secondary">Hourly OEE trend</p>
        </div>
        <div class="fiori-card__content oee-chart-content">
          <div class="chart-container">
            <canvas id="oeeTrendChart"></canvas>
          </div>
        </div>
      </div>
    </div>

    <div class="oee-additional-charts oee-charts-full">
      <div class="fiori-card oee-chart-card">
        <div class="fiori-card__header">
          <h3 class="fiori-card__title fiori-text-primary">📈 OEE Timeline</h3>
          <p class="fiori-card__subtitle fiori-text-secondary">Hourly OEE timeline</p>
        </div>
        <div class="fiori-card__content oee-chart-content">
          <div class="chart-container">
            <canvas id="productionTrendChart"></canvas>
          </div>
        </div>
      </div>
    </div>

    <div class="oee-additional-charts oee-charts-full">
      <div class="fiori-card oee-chart-card">
        <div class="fiori-card__header">
          <h3 class="fiori-card__title fiori-text-primary">🏭 Line OEE Performance</h3>
          <p class="fiori-card__subtitle fiori-text-secondary">OEE performance comparison by production line</p>
        </div>
        <div class="fiori-card__content oee-chart-content">
          <div class="chart-container">
            <canvas id="machineOeeChart"></canvas>
          </div>
        </div>
      </div>
    </div>

    <div class="oee-additional-charts" style="display: none;">
      <div class="fiori-card oee-chart-card">
        <div class="fiori-card__header">
          <h3 class="fiori-card__title fiori-text-primary">🎯 Quality vs Performance Efficiency Matrix</h3>
          <p class="fiori-card__subtitle fiori-text-secondary">Machine efficiency positioning by Quality-Performance correlation</p>
        </div>
        <div class="fiori-card__content oee-chart-content">
          <div class="chart-container">
            <canvas id="efficiencyMatrixChart"></canvas>
          </div>
        </div>
      </div>
    </div>

    <div class="fiori-section">
      <div class="fiori-card">
        <div class="fiori-card__header">
          <h3 class="fiori-card__title">📋 Real-time OEE Data</h3>
          <div class="real-time-status">
            <div class="status-dot"></div>
            <span id="lastUpdateTime">Last updated: -</span>
            <span id="connectionStatus" class="connection-status-info">Connection ready...</span>
          </div>
        </div>
        <div class="fiori-card__content fiori-p-0">
          <div class="data-table-wrapper">
            <table class="fiori-table" id="oeeDataTable">
              <thead class="fiori-table__header">
                <tr>
                  <th>Machine No</th>
                  <th>Factory/Line</th>
                  <th>Shift</th>
                  <th>Overall OEE</th>
                  <th>Availability</th>
                  <th>Performance</th>
                  <th>Quality</th>
                  <th>Work Date</th>
                  <th>Update Time</th>
                  <th>Action</th>
                </tr>
              </thead>
              <tbody id="oeeDataBody">
                <tr>
                  <td colspan="10" class="data-table-centered">
                    <div class="fiori-alert fiori-alert--info">
                      <strong>ℹ️ Information:</strong> Loading real-time OEE data. Automatic monitoring is in progress.
                    </div>
                  </td>
                </tr>
              </tbody>
            </table>
          </div>
        </div>
      </div>
      
      <div id="pagination-controls" class="fiori-pagination"></div>
    </div>

  </main>
</div>

<div id="oeeDetailModal" class="fiori-modal">
  <div class="fiori-modal__backdrop" onclick="closeOeeDetailModal()"></div>
  <div class="fiori-modal__content">
    <div class="fiori-card">
      <div class="fiori-card__header">
        <h3 class="fiori-card__title">🔍 OEE Details</h3>
        <button class="fiori-btn fiori-btn--icon" onclick="closeOeeDetailModal()">
          <span>✕</span>
        </button>
      </div>
      <div class="fiori-card__content">
        <div class="oee-detail-grid">
          <div class="oee-detail-section">
            <h4 class="oee-detail-section-title">📋 Basic Information</h4>
            <div class="oee-detail-row">
              <span class="oee-detail-label">Machine Number:</span>
              <span class="oee-detail-value" id="modal-machine-no">-</span>
            </div>
            <div class="oee-detail-row">
              <span class="oee-detail-label">Factory/Line:</span>
              <span class="oee-detail-value" id="modal-factory-line">-</span>
            </div>
            <div class="oee-detail-row">
              <span class="oee-detail-label">Work Date:</span>
              <span class="oee-detail-value" id="modal-work-date">-</span>
            </div>
            <div class="oee-detail-row">
              <span class="oee-detail-label">Shift:</span>
              <span class="oee-detail-value" id="modal-shift">-</span>
            </div>
          </div>
          
          <div class="oee-detail-section">
            <h4 class="oee-detail-section-title">📊 OEE Performance</h4>
            <div class="oee-detail-row">
              <span class="oee-detail-label">Overall OEE:</span>
              <span class="oee-detail-value" id="modal-overall-oee">-</span>
            </div>
            <div class="oee-detail-row">
              <span class="oee-detail-label">Availability:</span>
              <span class="oee-detail-value" id="modal-availability">-</span>
            </div>
            <div class="oee-detail-row">
              <span class="oee-detail-label">Performance:</span>
              <span class="oee-detail-value" id="modal-performance">-</span>
            </div>
            <div class="oee-detail-row">
              <span class="oee-detail-label">Quality:</span>
              <span class="oee-detail-value" id="modal-quality">-</span>
            </div>
          </div>
          
          <div class="oee-detail-section">
            <h4 class="oee-detail-section-title">⏰ Time & Production</h4>
            <div class="oee-detail-row">
              <span class="oee-detail-label">Planned Work Time:</span>
              <span class="oee-detail-value" id="modal-planned-time">-</span>
            </div>
            <div class="oee-detail-row">
              <span class="oee-detail-label">Runtime:</span>
              <span class="oee-detail-value" id="modal-runtime">-</span>
            </div>
            <div class="oee-detail-row">
              <span class="oee-detail-label">Downtime:</span>
              <span class="oee-detail-value" id="modal-downtime">-</span>
            </div>
            <div class="oee-detail-row">
              <span class="oee-detail-label">Actual Output:</span>
              <span class="oee-detail-value" id="modal-actual-output">-</span>
            </div>
          </div>
          
          <div class="oee-detail-section oee-detail-section--full">
            <h4 class="oee-detail-section-title">📝 Additional Information</h4>
            <div class="oee-detail-row">
              <span class="oee-detail-label">Theoretical Output:</span>
              <span class="oee-detail-value" id="modal-theoretical-output">-</span>
            </div>
            <div class="oee-detail-row">
              <span class="oee-detail-label">Defective Count:</span>
              <span class="oee-detail-value" id="modal-defective">-</span>
            </div>
            <div class="oee-detail-row">
              <span class="oee-detail-label">Cycle Time:</span>
              <span class="oee-detail-value" id="modal-cycletime">-</span>
            </div>
            <div class="oee-detail-row">
              <span class="oee-detail-label">Update Time:</span>
              <span class="oee-detail-value" id="modal-update-time">-</span>
            </div>
          </div>
        </div>
      </div>
      <div class="fiori-card__footer">
        <div class="oee-detail-actions">
          <button class="fiori-btn fiori-btn--secondary" onclick="closeOeeDetailModal()">Close</button>
          <button class="fiori-btn fiori-btn--primary" onclick="exportSingleOee()">📊 Export</button>
        </div>
      </div>
    </div>
  </div>
</div>

<footer class="fiori-footer">
  <p>&copy; 2025 SUNTECH. All Rights Reserved.</p>
</footer>

<script src="../../assets/js/chart.js"></script>

<script src="../../assets/js/jquery-3.6.1.min.js"></script>
<script src="../../assets/js/moment.min.js"></script>
<script src="../../assets/js/daterangepicker.js"></script>

<script src="js/data_oee.js"></script>

</body>
</html>