<?php
$page_title = 'OEE Data Log';
$page_css_files = ['../../assets/css/fiori-page.css', '../../assets/css/daterangepicker.css', 'css/log_oee.css'];

require_once(__DIR__ . '/../../inc/head.php');
require_once(__DIR__ . '/../../inc/nav-fiori.php');
?>

<div class="fiori-container">
  <main>

    <div class="fiori-main-header">
      <div>
        <h2>OEE Data Log</h2>
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
            <div class="fiori-dropdown">
              <button id="columnToggleBtn" class="fiori-btn fiori-btn--secondary">📋 Columns</button>
              <div id="columnToggleDropdown" class="fiori-dropdown__content">
                <!-- JS로 컬럼 체크박스 생성 -->
              </div>
            </div>
            <button id="excelDownloadBtn" class="fiori-btn fiori-btn--primary">📊 Export</button>
            <button id="refreshBtn" class="fiori-btn fiori-btn--tertiary">🔄 Refresh</button>
            <button id="toggleStatsBtn" class="fiori-btn fiori-btn--secondary">📊 Show Stats</button>
          </div>
        </div>
      </div>
    </div>

    <div class="oee-monitoring-grid hidden" id="statsGrid">
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

    <div class="fiori-section">
      <div class="fiori-card">
        <div class="fiori-card__header">
          <h3 class="fiori-card__title">📋 OEE Data Log</h3>
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
                <tr id="tableHeaderRow">
                  <!-- JS로 헤더 생성 -->
                </tr>
              </thead>
              <tbody id="oeeDataBody">
                <tr>
                  <td colspan="35" class="data-table-centered">
                    <div class="fiori-alert fiori-alert--info">
                      <strong>ℹ️ Information:</strong> Loading OEE data log. Please wait...
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

<footer class="fiori-footer">
  <p>&copy; 2025 SUNTECH. All Rights Reserved.</p>
</footer>

<script src="../../assets/js/jquery-3.6.1.min.js"></script>
<script src="../../assets/js/moment.min.js"></script>
<script src="../../assets/js/daterangepicker.js"></script>

<script src="js/log_oee.js"></script>

</body>
</html>
