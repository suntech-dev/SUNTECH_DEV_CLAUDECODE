<?php
$page_title = 'OEE Hourly Data Log';
$page_css_files = [
    '../../assets/css/fiori-page.css',
    '../../assets/css/daterangepicker.css',
    'css/log_oee_hourly_2.css',
];

require_once(__DIR__ . '/../../inc/head.php');
?>

<?php $nav_context = 'data';
$nav_active = 'log_oee_hourly';
require_once(__DIR__ . '/../../inc/nav-drawer-manage.php'); ?>

<!-- Signage Header -->
<div class="signage-header">
    <button id="navDrawerBtn" class="nav-drawer-btn" aria-label="Menu">&#9776;</button>
    <span class="signage-header__title">OEE Hourly Data Log</span>

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
        <input type="text" id="dateRangePicker" class="fiori-input date-range-input" readonly placeholder="Select date range">
        <select id="shiftSelect" class="fiori-select">
            <option value="">All Shift</option>
            <option value="1">Shift 1</option>
            <option value="2">Shift 2</option>
            <option value="3">Shift 3</option>
        </select>
        <div class="log-hourly-dropdown">
            <button id="columnToggleBtn" class="fiori-btn fiori-btn--secondary">Columns</button>
            <div id="columnToggleDropdown" class="log-hourly-dropdown__content"></div>
        </div>
        <button id="toggleStatsBtn" class="fiori-btn fiori-btn--secondary">Show Stats</button>
        <button id="toggleDataBtn" class="fiori-btn fiori-btn--secondary">Hide Table</button>
        <button id="excelDownloadBtn" class="fiori-btn fiori-btn--secondary">Export</button>
        <button id="refreshBtn" class="fiori-btn fiori-btn--tertiary">Refresh</button>
    </div>
</div>

<!-- Log OEE Hourly Signage Main -->
<div class="log-hourly-main" id="logHourlyMain">

    <!-- Row A: Stats (기본 hidden) -->
    <div id="logHourlyStats" class="log-hourly-row log-hourly-row--stats hidden">
        <div class="oee-stats-grid">
            <div class="stat-card stat-card--red">
                <div class="stat-value" id="overallOee">-</div>
                <div class="stat-label">Overall OEE</div>
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
            <div class="stat-card stat-card--rose">
                <div class="stat-value" id="previousDayOee">-</div>
                <div class="stat-label">Previous Day OEE</div>
            </div>
        </div>
    </div>

    <!-- Row B: Table -->
    <div id="logHourlyTable" class="log-hourly-row log-hourly-row--table">
        <div class="fiori-card">
            <div class="fiori-card__header">
                <h3 class="fiori-card__title">OEE Hourly Data Log</h3>
                <div class="real-time-status">
                    <div class="status-dot"></div>
                    <span id="lastUpdateTime">Last updated: -</span>
                    <span id="connectionStatus" class="connection-status-info">Connection ready...</span>
                </div>
            </div>
            <div class="fiori-card__content">
                <div class="oee-table-wrap">
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
                                        <strong>Information:</strong> Loading OEE hourly data log. Please wait...
                                    </div>
                                </td>
                            </tr>
                        </tbody>
                    </table>
                </div>
            </div>
        </div>
    </div>

    <!-- Row C: Pagination -->
    <div id="logHourlyPagination" class="log-hourly-row log-hourly-row--pagination">
        <div id="pagination-controls" class="fiori-pagination"></div>
    </div>

</div><!-- /log-hourly-main -->


<script src="../../assets/js/jquery-3.6.1.min.js"></script>
<script src="../../assets/js/moment.min.js"></script>
<script src="../../assets/js/daterangepicker.js"></script>
<script src="js/log_oee_hourly_2.js"></script>


</body>

</html>