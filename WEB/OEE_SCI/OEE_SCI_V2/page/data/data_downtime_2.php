<?php
$page_title = 'Downtime Data Monitoring';
$page_css_files = [
    '../../assets/css/fiori-page.css',
    '../../assets/css/daterangepicker.css',
    'css/data_downtime_2.css',
];

require_once(__DIR__ . '/../../inc/head.php');
?>

<?php $nav_context = 'data';
$nav_active = 'downtime_m';
require_once(__DIR__ . '/../../inc/nav-drawer-manage.php'); ?>

<!-- Signage Header -->
<div class="signage-header">
    <button id="navDrawerBtn" class="nav-drawer-btn" aria-label="Menu">&#9776;</button>
    <span class="signage-header__title">Downtime Monitoring</span>

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
        <button id="toggleStatsBtn" class="fiori-btn fiori-btn--secondary">Show Stats</button>
        <button id="toggleChartsBtn" class="fiori-btn fiori-btn--secondary">Show Charts</button>
        <button id="toggleDataBtn" class="fiori-btn fiori-btn--secondary">Hide Table</button>
        <button id="excelDownloadBtn" class="fiori-btn fiori-btn--secondary">Export</button>
        <button id="refreshBtn" class="fiori-btn fiori-btn--tertiary">Refresh</button>
    </div>
</div>

<!-- Downtime Signage Main -->
<div class="dt-signage-main" id="dtSignageMain">

    <!-- Row A: Stats (기본 hidden) -->
    <div id="dtRowStats" class="dt-row dt-row--stats hidden">
        <div class="dt-stats-grid">
            <div class="stat-card stat-card--red">
                <div class="stat-value" id="totalDowntime">-</div>
                <div class="stat-label">Total Downtime</div>
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
                <div class="stat-label">Long Downtimes (&gt;30min)</div>
            </div>
            <div class="stat-card stat-card--success">
                <div class="stat-value" id="avgDowntimeResolution">-</div>
                <div class="stat-label">Avg Resolution Time</div>
            </div>
        </div>
    </div>

    <!-- Row B: Charts Top — Summary Details(2fr) + 차트 2개(3fr) (기본 hidden) -->
    <div id="dtRowChartsTop" class="dt-row dt-row--charts-top hidden">
        <div class="dt-charts-top-grid">

            <!-- 좌: Downtime Summary Details -->
            <div class="fiori-card">
                <div class="fiori-card__header">
                    <div class="card-title-row">
                        <h3 class="fiori-card__title">Downtime Summary Details</h3>
                    </div>
                    <div class="real-time-status">
                        <div class="status-dot"></div>
                        <span id="dtLiveStatus">Real-time monitoring active</span>
                    </div>
                </div>
                <div class="fiori-card__content">
                    <div class="dt-details-list" id="dtDetailsContainer">
                        <div class="dt-component-item">
                            <div class="dt-component-info">
                                <div class="dt-component-name">Total Downtime</div>
                                <div class="dt-component-details">
                                    <span class="dt-detail-item">Active: <span id="dtDetailWarningCount">-</span></span>
                                    <span class="dt-detail-item">Completed: <span id="dtDetailCompletedCount">-</span></span>
                                </div>
                            </div>
                            <div class="dt-component-value"><span id="dtTotalDetail">-</span></div>
                        </div>
                        <div class="dt-component-item">
                            <div class="dt-component-info">
                                <div class="dt-component-name">Current Shift Impact</div>
                                <div class="dt-component-details">
                                    <span class="dt-detail-item">Shift Count: <span id="dtDetailShiftCount">-</span></span>
                                    <span class="dt-detail-item">Long Downtimes: <span id="dtDetailLongDowntimes">-</span></span>
                                </div>
                            </div>
                            <div class="dt-component-value"><span id="dtShiftDetail">-</span></div>
                        </div>
                        <div class="dt-component-item">
                            <div class="dt-component-info">
                                <div class="dt-component-name">Machine Availability</div>
                                <div class="dt-component-details">
                                    <span class="dt-detail-item">Affected: <span id="dtDetailAffectedMachines">-</span></span>
                                    <span class="dt-detail-item">Max Duration: <span id="dtDetailMaxDuration">-</span></span>
                                </div>
                            </div>
                            <div class="dt-component-value"><span id="dtMachineDetail">-</span></div>
                        </div>
                        <div class="dt-component-item">
                            <div class="dt-component-info">
                                <div class="dt-component-name">Resolution Performance</div>
                                <div class="dt-component-details">
                                    <span class="dt-detail-item">Avg Time: <span id="dtDetailAvgResolution">-</span></span>
                                    <span class="dt-detail-item">Over 30min: <span id="dtDetailOver30">-</span></span>
                                </div>
                            </div>
                            <div class="dt-component-value"><span id="dtResolutionDetail">-</span></div>
                        </div>
                    </div>
                </div>
            </div>

            <!-- 우: 차트 2개 -->
            <div class="dt-charts-pair">
                <div class="fiori-card">
                    <div class="fiori-card__header">
                        <div class="card-title-row">
                            <h3 class="fiori-card__title">Downtime Type Analysis</h3>
                            <span class="card-subtitle-inline">Duration by type (minutes)</span>
                        </div>
                    </div>
                    <div class="fiori-card__content">
                        <div class="chart-container"><canvas id="dtTypeChart"></canvas></div>
                    </div>
                </div>
                <div class="fiori-card">
                    <div class="fiori-card__header">
                        <div class="card-title-row">
                            <h3 class="fiori-card__title">Downtime Status</h3>
                            <span class="card-subtitle-inline">Active vs Completed</span>
                        </div>
                    </div>
                    <div class="fiori-card__content">
                        <div class="chart-container"><canvas id="dtStatusChart"></canvas></div>
                    </div>
                </div>
            </div>

        </div>
    </div>

    <!-- Row C: Charts Bottom — 3 Trend 차트 (기본 hidden) -->
    <div id="dtRowChartsBottom" class="dt-row dt-row--charts-bottom hidden">
        <div class="dt-charts-trio">

            <div class="fiori-card">
                <div class="fiori-card__header">
                    <div class="card-title-row">
                        <h3 class="fiori-card__title">Downtime Trend</h3>
                        <span class="card-subtitle-inline">Hourly occurrence (Warning / Completed)</span>
                    </div>
                </div>
                <div class="fiori-card__content">
                    <div class="chart-container"><canvas id="dtTrendChart"></canvas></div>
                </div>
            </div>

            <div class="fiori-card">
                <div class="fiori-card__header">
                    <div class="card-title-row">
                        <h3 class="fiori-card__title" id="dtLineChartTitle">Line Downtime</h3>
                        <span class="card-subtitle-inline" id="dtLineChartSubtitle">Downtime comparison by line</span>
                    </div>
                </div>
                <div class="fiori-card__content">
                    <div class="chart-container"><canvas id="dtLineChart"></canvas></div>
                </div>
            </div>

            <div class="fiori-card">
                <div class="fiori-card__header">
                    <div class="card-title-row">
                        <h3 class="fiori-card__title">Duration Distribution</h3>
                        <span class="card-subtitle-inline">Downtime duration buckets</span>
                    </div>
                </div>
                <div class="fiori-card__content">
                    <div class="chart-container"><canvas id="dtDurationChart"></canvas></div>
                </div>
            </div>

        </div>
    </div>

    <!-- Row D: Real-time Downtime Table -->
    <div id="dtRowTable" class="dt-row dt-row--table">
        <div class="fiori-card">
            <div class="fiori-card__header">
                <h3 class="fiori-card__title">Real-time Downtime Data</h3>
                <div class="real-time-status">
                    <div class="status-dot"></div>
                    <span id="lastUpdateTime">Last updated: -</span>
                    <span id="connectionStatus" class="connection-status-info">Connection ready...</span>
                </div>
            </div>
            <div class="fiori-card__content">
                <div class="dt-table-wrap">
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
                                <th>AI Risk <span class="ai-badge" style="font-size:0.6rem;padding:1px 6px;">AI</span></th>
                                <th>DETAIL</th>
                            </tr>
                        </thead>
                        <tbody id="downtimeDataBody">
                            <tr>
                                <td colspan="11" class="data-table-centered">
                                    <div class="fiori-alert fiori-alert--info">
                                        <strong>Information:</strong> Loading real-time Downtime data. Automatic monitoring is in progress.
                                    </div>
                                </td>
                            </tr>
                        </tbody>
                    </table>
                </div>
            </div>
        </div>
    </div>

    <!-- Row E: Pagination -->
    <div id="dtRowPagination" class="dt-row dt-row--pagination">
        <div id="pagination-controls" class="fiori-pagination"></div>
    </div>

</div><!-- /dt-signage-main -->


<!-- Downtime Detail Modal -->
<div id="downtimeDetailModal" class="fiori-modal">
    <div class="fiori-modal__backdrop" onclick="closeDowntimeDetailModal()"></div>
    <div class="fiori-modal__content">
        <div class="fiori-card">
            <div class="fiori-card__header">
                <h3 class="fiori-card__title">Downtime Details</h3>
                <button class="fiori-btn fiori-btn--icon" onclick="closeDowntimeDetailModal()"><span>&#10005;</span></button>
            </div>
            <div class="fiori-card__content">
                <div class="dt-detail-grid">
                    <div class="dt-detail-section">
                        <h4 class="dt-detail-section-title">Basic Information</h4>
                        <div class="dt-detail-row"><span class="dt-detail-label">Machine Number:</span><span class="dt-detail-value" id="modal-machine-no">-</span></div>
                        <div class="dt-detail-row"><span class="dt-detail-label">Factory/Line:</span><span class="dt-detail-value" id="modal-factory-line">-</span></div>
                        <div class="dt-detail-row"><span class="dt-detail-label">Downtime Type:</span><span class="dt-detail-value" id="modal-downtime-type">-</span></div>
                        <div class="dt-detail-row"><span class="dt-detail-label">Status:</span><span class="dt-detail-value" id="modal-status">-</span></div>
                    </div>
                    <div class="dt-detail-section">
                        <h4 class="dt-detail-section-title">Time Information</h4>
                        <div class="dt-detail-row"><span class="dt-detail-label">Occurrence Time:</span><span class="dt-detail-value" id="modal-reg-date">-</span></div>
                        <div class="dt-detail-row"><span class="dt-detail-label">Resolution Time:</span><span class="dt-detail-value" id="modal-update-date">-</span></div>
                        <div class="dt-detail-row"><span class="dt-detail-label">Duration:</span><span class="dt-detail-value" id="modal-duration">-</span></div>
                        <div class="dt-detail-row"><span class="dt-detail-label">Work Date:</span><span class="dt-detail-value" id="modal-work-date">-</span></div>
                    </div>
                    <div class="dt-detail-section">
                        <h4 class="dt-detail-section-title">Work Information</h4>
                        <div class="dt-detail-row"><span class="dt-detail-label">Shift:</span><span class="dt-detail-value" id="modal-shift">-</span></div>
                        <div class="dt-detail-row"><span class="dt-detail-label">Downtime Color:</span><span class="dt-detail-value" id="modal-downtime-color"><span class="dt-color-indicator" id="modal-color-indicator"></span><span id="modal-color-value">Default Color</span></span></div>
                    </div>
                    <div class="dt-detail-section dt-detail-section--full">
                        <h4 class="dt-detail-section-title">Additional Information</h4>
                        <div class="dt-detail-row"><span class="dt-detail-label">Database ID:</span><span class="dt-detail-value" id="modal-idx">-</span></div>
                        <div class="dt-detail-row"><span class="dt-detail-label">Registration Date:</span><span class="dt-detail-value" id="modal-created-at">-</span></div>
                    </div>
                </div>
            </div>
            <div class="fiori-card__footer">
                <div class="dt-detail-actions">
                    <button class="fiori-btn fiori-btn--secondary" onclick="closeDowntimeDetailModal()">Close</button>
                    <button class="fiori-btn fiori-btn--primary" onclick="exportSingleDowntime()">Export</button>
                </div>
            </div>
        </div>
    </div>
</div>


<script src="../../assets/js/chart.js"></script>
<script src="../../assets/js/jquery-3.6.1.min.js"></script>
<script src="../../assets/js/moment.min.js"></script>
<script src="../../assets/js/daterangepicker.js"></script>
<script src="js/data_downtime_2.js"></script>
<script src="js/ai_downtime_risk.js"></script>


</body>

</html>