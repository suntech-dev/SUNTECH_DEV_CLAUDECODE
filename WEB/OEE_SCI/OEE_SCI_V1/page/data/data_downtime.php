<?php
$page_title = 'Downtime Data Monitoring';
$page_css_files = ['../../assets/css/fiori-page.css', '../../assets/css/daterangepicker.css', 'css/data_downtime.css'];

require_once(__DIR__ . '/../../inc/head.php');
require_once(__DIR__ . '/../../inc/nav-fiori.php');
?>

<div class="fiori-container">
  <main>

    <div class="fiori-main-header">
      <div>
        <h2>Downtime Monitoring</h2>
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
            <button id="toggleStatsBtn" class="fiori-btn fiori-btn--secondary">Hide Stats</button>
            <button id="toggleChartsBtn" class="fiori-btn fiori-btn--secondary">📈 Hide Charts</button>
          </div>
        </div>
      </div>
    </div>

    <div class="downtime-monitoring-grid" id="statsGrid">
      <div class="stat-card stat-card--red">
        <div class="stat-value" id="totalDowntime">-</div>
        <div class="stat-label">⏱️ Total Downtime</div>
      </div>
      <div class="stat-card stat-card--rose">
        <div class="stat-value" id="activeDowntimes">-</div>
        <div class="stat-label">Active Downtimes</div>
      </div>
      <div class="stat-card stat-card--info">
        <div class="stat-value" id="currentShiftDowntime">-</div>
        <div class="stat-label">Current Shift Downtime</div>
      </div>
      <div class="stat-card stat-card--warning">
        <div class="stat-value" id="affectedMachinesDowntime">-</div>
        <div class="stat-label">Affected Machines</div>
      </div>
      <div class="stat-card stat-card--maroon">
        <div class="stat-value" id="longDowntimes">-</div>
        <div class="stat-label">Long Downtimes (>30min)</div>
      </div>
      <div class="stat-card stat-card--success">
        <div class="stat-value" id="avgDowntimeResolution">-</div>
        <div class="stat-label">Avg Resolution Time</div>
      </div>
    </div>

    <div class="downtime-row-layout">
      <div class="fiori-section downtime-active-section">
        <div class="fiori-card">
          <div class="fiori-card__header">
            <h3 class="fiori-card__title">🔥 Currently Active Downtime</h3>
            <div class="real-time-status real-time-status-header">
              <div class="status-dot"></div>
              <span id="activeDowntimeCount">0 active downtimes</span>
            </div>
          </div>
          <div class="fiori-card__content">
            <div id="activeDowntimesContainer">
              <div class="fiori-alert fiori-alert--info">
                <strong>ℹ️ Information:</strong> There are currently no active Downtimes. Real-time monitoring is active.
              </div>
            </div>
          </div>
        </div>
      </div>

      <div class="fiori-section">
        <div class="fiori-card downtime-chart-small">
          <div class="fiori-card__header">
            <h3 class="fiori-card__title">📊 Analysis by Downtime type</h3>
            <p class="fiori-card__subtitle">Frequency of occurrence by Downtime type</p>
          </div>
          <div class="fiori-card__content">
            <div class="chart-container">
              <canvas id="downtimeTypeChart"></canvas>
            </div>
          </div>
        </div>
      </div>
    </div>

    <div class="fiori-section downtime-trend-chart-full" id="downtimeTrendSection">
      <div class="fiori-card downtime-chart-small">
        <div class="fiori-card__header">
          <h3 class="fiori-card__title">📈 Downtime occurrence trend</h3>
          <p class="fiori-card__subtitle">Hourly change in the number of Downtime occurrences</p>
        </div>
        <div class="fiori-card__content">
          <div class="chart-container">
            <canvas id="downtimeTrendChart"></canvas>
          </div>
        </div>
      </div>
    </div>

    <div class="fiori-section">
      <div class="fiori-card">
        <div class="fiori-card__header">
          <h3 class="fiori-card__title">📋 Real-time Downtime data</h3>
          <div class="real-time-status">
            <div class="status-dot"></div>
            <span id="lastUpdateTime">Last updated: -</span>
            <span id="connectionStatus" class="connection-status-info">Connection ready...</span>
          </div>
        </div>
        <div class="fiori-card__content fiori-p-0">
          <div class="data-table-wrapper">
            <table class="fiori-table" id="downtimeDataTable">
              <thead class="fiori-table__header">
                <tr>
                  <th>Machine No</th>
                  <th>Factory/Line</th>
                  <th>Shift</th>
                  <th>Downtime Type</th>
                  <th>Status</th>
                  <th>Occurrence Time</th>
                  <th>Resolution Time</th>
                  <th>Duration</th>
                  <th>Work Date</th>
                  <th>DETAIL</th>
                </tr>
              </thead>
              <tbody id="downtimeDataBody">
                <tr>
                  <td colspan="10" class="data-table-centered">
                    <div class="fiori-alert fiori-alert--info">
                      <strong>ℹ️ Information:</strong> Loading real-time Downtime data. Automatic monitoring is in progress.
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

<div id="downtimeDetailModal" class="fiori-modal">
  <div class="fiori-modal__backdrop" onclick="closeDowntimeDetailModal()"></div>
  <div class="fiori-modal__content">
    <div class="fiori-card">
      <div class="fiori-card__header">
        <h3 class="fiori-card__title">🔍 Downtime Details</h3>
        <button class="fiori-btn fiori-btn--icon" onclick="closeDowntimeDetailModal()">
          <span>✕</span>
        </button>
      </div>
      <div class="fiori-card__content">
        <div class="downtime-detail-grid">
          <!-- 기본 정보 섹션 -->
          <div class="downtime-detail-section">
            <h4 class="downtime-detail-section-title">📋 Basic Information</h4>
            <div class="downtime-detail-row">
              <span class="downtime-detail-label">Machine Number:</span>
              <span class="downtime-detail-value" id="modal-machine-no">-</span>
            </div>
            <div class="downtime-detail-row">
              <span class="downtime-detail-label">Factory/Line:</span>
              <span class="downtime-detail-value" id="modal-factory-line">-</span>
            </div>
            <div class="downtime-detail-row">
              <span class="downtime-detail-label">Downtime Type:</span>
              <span class="downtime-detail-value" id="modal-downtime-type">-</span>
            </div>
            <div class="downtime-detail-row">
              <span class="downtime-detail-label">Status:</span>
              <span class="downtime-detail-value" id="modal-status">-</span>
            </div>
          </div>
          
          <!-- 시간 정보 섹션 -->
          <div class="downtime-detail-section">
            <h4 class="downtime-detail-section-title">⏰ Time Information</h4>
            <div class="downtime-detail-row">
              <span class="downtime-detail-label">Occurrence Time:</span>
              <span class="downtime-detail-value" id="modal-reg-date">-</span>
            </div>
            <div class="downtime-detail-row">
              <span class="downtime-detail-label">Resolution Time:</span>
              <span class="downtime-detail-value" id="modal-update-date">-</span>
            </div>
            <div class="downtime-detail-row">
              <span class="downtime-detail-label">Duration:</span>
              <span class="downtime-detail-value" id="modal-duration">-</span>
            </div>
            <div class="downtime-detail-row">
              <span class="downtime-detail-label">Work Date:</span>
              <span class="downtime-detail-value" id="modal-work-date">-</span>
            </div>
          </div>
          
          <!-- 작업 정보 섹션 -->
          <div class="downtime-detail-section">
            <h4 class="downtime-detail-section-title">🏭 Work Information</h4>
            <div class="downtime-detail-row">
              <span class="downtime-detail-label">Shift:</span>
              <span class="downtime-detail-value" id="modal-shift">-</span>
            </div>
            <div class="downtime-detail-row">
              <span class="downtime-detail-label">Downtime Color:</span>
              <span class="downtime-detail-value" id="modal-downtime-color">
                <span class="downtime-color-indicator" id="modal-color-indicator"></span>
                <span id="modal-color-value">Default Color</span>
              </span>
            </div>
          </div>
          
          <!-- 추가 정보 섹션 -->
          <div class="downtime-detail-section downtime-detail-section--full">
            <h4 class="downtime-detail-section-title">📝 Additional Information</h4>
            <div class="downtime-detail-row">
              <span class="downtime-detail-label">Database ID:</span>
              <span class="downtime-detail-value" id="modal-idx">-</span>
            </div>
            <div class="downtime-detail-row">
              <span class="downtime-detail-label">Registration Date:</span>
              <span class="downtime-detail-value" id="modal-created-at">-</span>
            </div>
          </div>
        </div>
      </div>
      <div class="fiori-card__footer">
        <div class="downtime-detail-actions">
          <button class="fiori-btn fiori-btn--secondary" onclick="closeDowntimeDetailModal()">Close</button>
          <button class="fiori-btn fiori-btn--primary" onclick="exportSingleDowntime()">📊 Export</button>
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

<script src="js/data_downtime.js"></script>

</body>
</html>