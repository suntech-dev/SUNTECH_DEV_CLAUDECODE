<?php
$page_title = 'Defective Data Monitoring';
$page_css_files = ['../../assets/css/fiori-page.css', '../../assets/css/daterangepicker.css', 'css/data_defective.css'];

require_once(__DIR__ . '/../../inc/head.php');
require_once(__DIR__ . '/../../inc/nav-fiori.php');
?>

<div class="fiori-container">
  <main>

    <!-- 페이지 헤더 -->
    <div class="fiori-main-header">
      <div>
        <h2>Defective Monitoring</h2>
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
              <option value="today" selected>today</option>
              <option value="yesterday">yesterday</option>
              <option value="1w">last week</option>
              <option value="1m">last month</option>
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
            <button id="excelDownloadBtn" class="fiori-btn fiori-btn--primary">Export</button>
            <button id="refreshBtn" class="fiori-btn fiori-btn--tertiary">🔄 Refresh</button>
            <button id="toggleStatsBtn" class="fiori-btn fiori-btn--secondary">📊 Hide Stats</button>
            <button id="toggleChartsBtn" class="fiori-btn fiori-btn--secondary">📈 Hide Charts</button>
          </div>
        </div>
      </div>
    </div>

    <div class="defective-monitoring-grid" id="statsGrid">
      <!-- <div class="stat-card stat-card--success">
        <div class="stat-value" id="totalDefective">-</div>
        <div class="stat-label">Total Defective Count</div>
      </div> -->
      <div class="stat-card stat-card--red">
        <div class="stat-value" id="activeDefectives">-</div>
        <div class="stat-label">⚠️ Active Defectives</div>
      </div>
      <div class="stat-card stat-card--info">
        <div class="stat-value" id="currentShiftDefective">-</div>
        <div class="stat-label">Current Shift Defective</div>
      </div>
      <div class="stat-card stat-card--warning">
        <div class="stat-value" id="affectedMachinesDefective">-</div>
        <div class="stat-label">Affected Machines</div>
      </div>
      <div class="stat-card stat-card--maroon">
        <div class="stat-value" id="defectiveRate">-</div>
        <div class="stat-label">Defective Rate (%)</div>
      </div>
      <div class="stat-card stat-card--success">
        <div class="stat-value" id="qualityScore">-</div>
        <div class="stat-label">Quality Rate (%)</div>
      </div>
    </div>

    <div class="defective-row-layout">
      <div class="fiori-section defective-active-section">
        <div class="fiori-card">
          <div class="fiori-card__header">
            <h3 class="fiori-card__title">🔥 Currently Active Defective</h3>
            <div class="real-time-status real-time-status-header">
              <div class="status-dot"></div>
              <span id="activeDefectiveCount">0 active defectives</span>
            </div>
          </div>
          <div class="fiori-card__content">
            <div id="activeDefectivesContainer">
              <div class="fiori-alert fiori-alert--info">
                <strong>ℹ️ Information:</strong> There are currently no active Defectives. Real-time monitoring is active.
              </div>
            </div>
          </div>
        </div>
      </div>

      <div class="fiori-section">
        <div class="fiori-card defective-chart-small">
          <div class="fiori-card__header">
            <h3 class="fiori-card__title">📊 Analysis by Defective type</h3>
            <p class="fiori-card__subtitle">Frequency of occurrence by Defective type</p>
          </div>
          <div class="fiori-card__content">
            <div class="chart-container">
              <canvas id="defectiveTypeChart"></canvas>
            </div>
          </div>
        </div>
      </div>
    </div>

    <div class="fiori-section" id="defectiveTrendSection">
      <div class="fiori-card">
        <div class="fiori-card__header">
          <h3 class="fiori-card__title">📈 Defective Count Trend</h3>
          <p class="fiori-card__subtitle">Hourly/Daily change in defective occurrence count</p>
        </div>
        <div class="fiori-card__content">
          <div class="chart-container">
            <canvas id="defectiveRateChart"></canvas>
          </div>
        </div>
      </div>
    </div>

    <div class="fiori-section">
      <div class="fiori-card">
        <div class="fiori-card__header">
          <h3 class="fiori-card__title">📋 Real-time Defective data</h3>
          <div class="real-time-status">
            <div class="status-dot"></div>
            <span id="lastUpdateTime">Last updated: -</span>
            <span id="connectionStatus" class="connection-status-info">Connection ready...</span>
          </div>
        </div>
        <div class="fiori-card__content fiori-p-0">
          <div class="data-table-wrapper">
            <table class="fiori-table" id="defectiveDataTable">
              <thead class="fiori-table__header">
                <tr>
                  <th>Machine No</th>
                  <th>Factory/Line</th>
                  <th>Shift</th>
                  <th>Defective Type</th>
                  <!-- <th>Status</th> -->
                  <th>Occurrence Time</th>
                  <th>Elapsed Time</th>
                  <th>Work Date</th>
                  <th>Detail</th>
                </tr>
              </thead>
              <tbody id="defectiveDataBody">
                <tr>
                  <td colspan="9" class="data-table-centered">
                    <div class="fiori-alert fiori-alert--info">
                      <strong>ℹ️ Information:</strong> Loading real-time Defective data. Automatic monitoring is in progress.
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

<div id="defectiveDetailModal" class="fiori-modal">
  <div class="fiori-modal__backdrop" onclick="closeDefectiveDetailModal()"></div>
  <div class="fiori-modal__content">
    <div class="fiori-card">
      <div class="fiori-card__header">
        <h3 class="fiori-card__title">🔍 Defective Details</h3>
        <button class="fiori-btn fiori-btn--icon" onclick="closeDefectiveDetailModal()">
          <span>✕</span>
        </button>
      </div>
      <div class="fiori-card__content">
        <div class="defective-detail-grid">
          <!-- 기본 정보 섹션 -->
          <div class="defective-detail-section">
            <h4 class="defective-detail-section-title">📋 Basic Information</h4>
            <div class="defective-detail-row">
              <span class="defective-detail-label">Machine Number:</span>
              <span class="defective-detail-value" id="modal-machine-no">-</span>
            </div>
            <div class="defective-detail-row">
              <span class="defective-detail-label">Factory/Line:</span>
              <span class="defective-detail-value" id="modal-factory-line">-</span>
            </div>
            <div class="defective-detail-row">
              <span class="defective-detail-label">Defective Type:</span>
              <span class="defective-detail-value" id="modal-defective-type">-</span>
            </div>
            <div class="defective-detail-row">
              <span class="defective-detail-label">Status:</span>
              <span class="defective-detail-value" id="modal-status">-</span>
            </div>
          </div>
          
          <!-- 시간 정보 섹션 -->
          <div class="defective-detail-section">
            <h4 class="defective-detail-section-title">⏰ Time Information</h4>
            <div class="defective-detail-row">
              <span class="defective-detail-label">Occurrence Time:</span>
              <span class="defective-detail-value" id="modal-reg-date">-</span>
            </div>
            <div class="defective-detail-row">
              <span class="defective-detail-label">Elapsed Time:</span>
              <span class="defective-detail-value" id="modal-elapsed-time">-</span>
            </div>
            <div class="defective-detail-row">
              <span class="defective-detail-label">Work Date:</span>
              <span class="defective-detail-value" id="modal-work-date">-</span>
            </div>
          </div>
          
          <!-- 작업 정보 섹션 -->
          <div class="defective-detail-section">
            <h4 class="defective-detail-section-title">🏭 Work Information</h4>
            <div class="defective-detail-row">
              <span class="defective-detail-label">Shift:</span>
              <span class="defective-detail-value" id="modal-shift">-</span>
            </div>
            <div class="defective-detail-row">
              <span class="defective-detail-label">Defective Color:</span>
              <span class="defective-detail-value" id="modal-defective-color">
                <span class="defective-color-indicator" id="modal-color-indicator"></span>
                <span id="modal-color-value">Default Color</span>
              </span>
            </div>
          </div>
          
          <!-- 추가 정보 섹션 -->
          <div class="defective-detail-section defective-detail-section--full">
            <h4 class="defective-detail-section-title">📝 Additional Information</h4>
            <div class="defective-detail-row">
              <span class="defective-detail-label">Database ID:</span>
              <span class="defective-detail-value" id="modal-idx">-</span>
            </div>
            <div class="defective-detail-row">
              <span class="defective-detail-label">Registration Date:</span>
              <span class="defective-detail-value" id="modal-created-at">-</span>
            </div>
          </div>
        </div>
      </div>
      <div class="fiori-card__footer">
        <div class="defective-detail-actions">
          <button class="fiori-btn fiori-btn--secondary" onclick="closeDefectiveDetailModal()">Close</button>
          <button class="fiori-btn fiori-btn--primary" onclick="exportSingleDefective()">📊 Export</button>
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

<script src="js/data_defective.js"></script>

</body>
</html>