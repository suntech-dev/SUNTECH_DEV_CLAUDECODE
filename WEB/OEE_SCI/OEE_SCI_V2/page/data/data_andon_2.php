<?php
$page_title = 'Andon Data Monitoring';
$page_css_files = [
    '../../assets/css/fiori-page.css',
    '../../assets/css/daterangepicker.css',
    'css/data_andon_2.css',
];

require_once(__DIR__ . '/../../inc/head.php');
?>

<?php $nav_context = 'data';
$nav_active = 'andon_m';
require_once(__DIR__ . '/../../inc/nav-drawer-manage.php'); ?>

<!-- Signage Header -->
<div class="signage-header">
    <button id="navDrawerBtn" class="nav-drawer-btn" aria-label="Menu">&#9776;</button>
    <span class="signage-header__title">Andon Monitoring</span>

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

<!-- Andon Signage Main -->
<div class="andon-signage-main" id="andonSignageMain">

    <!-- Row A: Stats (기본 hidden) -->
    <div id="andonRowStats" class="andon-row andon-row--stats hidden">
        <div class="andon-stats-grid">
            <div class="stat-card stat-card--red">
                <div class="stat-value" id="totalAndons">-</div>
                <div class="stat-label">Total Andon Count</div>
            </div>
            <div class="stat-card stat-card--warning">
                <div class="stat-value" id="activeWarnings">-</div>
                <div class="stat-label">Active Warnings</div>
            </div>
            <div class="stat-card stat-card--info">
                <div class="stat-value" id="currentShiftCount">-</div>
                <div class="stat-label">Current Shift Count</div>
            </div>
            <div class="stat-card stat-card--maroon">
                <div class="stat-value" id="affectedMachines">-</div>
                <div class="stat-label">Affected Machine Count</div>
            </div>
            <div class="stat-card stat-card--red">
                <div class="stat-value" id="urgentWarnings">-</div>
                <div class="stat-label">Unresolved Over 5min</div>
            </div>
            <div class="stat-card stat-card--success">
                <div class="stat-value" id="avgCompletedTime">-</div>
                <div class="stat-label">Avg Completion Time</div>
            </div>
        </div>
    </div>

    <!-- Row B: Charts Top — Active Andons(2fr) + Andon Type Chart(3fr) (기본 hidden) -->
    <div id="andonRowChartsTop" class="andon-row andon-row--charts-top hidden">
        <div class="andon-charts-top-grid">

            <!-- 좌: Currently Active Andons -->
            <div class="fiori-card">
                <div class="fiori-card__header">
                    <div class="card-title-row">
                        <h3 class="fiori-card__title">Currently Active Andon</h3>
                        <span class="card-subtitle-inline" id="activeAndonCount">0 active alerts</span>
                    </div>
                    <div class="real-time-status">
                        <div class="status-dot"></div>
                        <span id="oeeLiveStatus">Real-time monitoring active</span>
                    </div>
                </div>
                <div class="fiori-card__content">
                    <div class="andon-active-list" id="activeAndonsContainer">
                        <div class="fiori-alert fiori-alert--info">
                            <strong>Information:</strong> No active Andon. Real-time monitoring active.
                        </div>
                    </div>
                </div>
            </div>

            <!-- 우: Andon Type Analysis Chart -->
            <div class="fiori-card">
                <div class="fiori-card__header">
                    <div class="card-title-row">
                        <h3 class="fiori-card__title">Analysis by Andon Type</h3>
                    </div>
                </div>
                <div class="fiori-card__content">
                    <div class="chart-container">
                        <canvas id="andonTypeChart"></canvas>
                    </div>
                </div>
            </div>

        </div>
    </div>

    <!-- Row C: Charts Bottom — Andon Trend (기본 hidden) -->
    <div id="andonRowChartsBottom" class="andon-row andon-row--charts-bottom hidden">
        <div class="fiori-card">
            <div class="fiori-card__header">
                <div class="card-title-row">
                    <h3 class="fiori-card__title">Andon Occurrence Trend</h3>
                </div>
            </div>
            <div class="fiori-card__content">
                <div class="chart-container">
                    <canvas id="andonTrendChart"></canvas>
                </div>
            </div>
        </div>
    </div>

    <!-- Row D: Table (기본 표시) -->
    <div id="andonRowTable" class="andon-row andon-row--table">
        <div class="fiori-card">
            <div class="fiori-card__header">
                <div class="card-title-row">
                    <h3 class="fiori-card__title">Real-time Andon Data</h3>
                </div>
                <div class="real-time-status">
                    <div class="status-dot"></div>
                    <span id="lastUpdateTime">Last updated: -</span>
                    <span id="connectionStatus" class="connection-status-info">Connection ready...</span>
                </div>
            </div>
            <div class="fiori-card__content">
                <div class="andon-table-wrap">
                    <table class="fiori-table" id="andonDataTable">
                        <thead class="fiori-table__header">
                            <tr>
                                <th>Machine No</th>
                                <th>Factory/Line</th>
                                <th>Shift</th>
                                <th>Andon Type</th>
                                <th>Status</th>
                                <th>Occurrence Time</th>
                                <th>Resolution Time</th>
                                <th>Duration</th>
                                <th>Work Date</th>
                                <th>DETAIL</th>
                            </tr>
                        </thead>
                        <tbody id="andonDataBody">
                            <tr>
                                <td colspan="10" class="data-table-centered">
                                    <div class="fiori-alert fiori-alert--info">
                                        <strong>Information:</strong> Loading real-time Andon data.
                                    </div>
                                </td>
                            </tr>
                        </tbody>
                    </table>
                </div>
            </div>
        </div>
    </div>

    <!-- Row E: Pagination (항상 auto) -->
    <div id="andonRowPagination" class="andon-row">
        <div id="pagination-controls" class="fiori-pagination"></div>
    </div>

</div>

<!-- Andon Detail Modal -->
<div id="andonDetailModal" class="fiori-modal">
    <div class="fiori-modal__backdrop" onclick="closeAndonDetailModal()"></div>
    <div class="fiori-modal__content">
        <div class="fiori-card">
            <div class="fiori-card__header">
                <h3 class="fiori-card__title">Andon Details</h3>
                <button class="fiori-btn fiori-btn--icon" onclick="closeAndonDetailModal()"><span>&#x2715;</span></button>
            </div>
            <div class="fiori-card__content">
                <div class="andon-detail-grid">
                    <div class="andon-detail-section">
                        <h4 class="andon-detail-section-title">Basic Information</h4>
                        <div class="andon-detail-row"><span class="andon-detail-label">Machine Number:</span><span class="andon-detail-value" id="modal-machine-no">-</span></div>
                        <div class="andon-detail-row"><span class="andon-detail-label">Factory/Line:</span><span class="andon-detail-value" id="modal-factory-line">-</span></div>
                        <div class="andon-detail-row"><span class="andon-detail-label">Andon Type:</span><span class="andon-detail-value" id="modal-andon-type">-</span></div>
                        <div class="andon-detail-row"><span class="andon-detail-label">Status:</span><span class="andon-detail-value" id="modal-status">-</span></div>
                    </div>
                    <div class="andon-detail-section">
                        <h4 class="andon-detail-section-title">Time Information</h4>
                        <div class="andon-detail-row"><span class="andon-detail-label">Occurrence Time:</span><span class="andon-detail-value" id="modal-reg-date">-</span></div>
                        <div class="andon-detail-row"><span class="andon-detail-label">Resolution Time:</span><span class="andon-detail-value" id="modal-update-date">-</span></div>
                        <div class="andon-detail-row"><span class="andon-detail-label">Duration:</span><span class="andon-detail-value" id="modal-duration">-</span></div>
                        <div class="andon-detail-row"><span class="andon-detail-label">Work Date:</span><span class="andon-detail-value" id="modal-work-date">-</span></div>
                    </div>
                    <div class="andon-detail-section">
                        <h4 class="andon-detail-section-title">Work Information</h4>
                        <div class="andon-detail-row"><span class="andon-detail-label">Shift:</span><span class="andon-detail-value" id="modal-shift">-</span></div>
                        <div class="andon-detail-row">
                            <span class="andon-detail-label">Andon Color:</span>
                            <span class="andon-detail-value" id="modal-andon-color">
                                <span class="andon-color-indicator" id="modal-color-indicator"></span>
                                <span id="modal-color-value">Default Color</span>
                            </span>
                        </div>
                    </div>
                    <div class="andon-detail-section andon-detail-section--full">
                        <h4 class="andon-detail-section-title">Additional Information</h4>
                        <div class="andon-detail-row"><span class="andon-detail-label">Database ID:</span><span class="andon-detail-value" id="modal-idx">-</span></div>
                        <div class="andon-detail-row"><span class="andon-detail-label">Registration Date:</span><span class="andon-detail-value" id="modal-created-at">-</span></div>
                    </div>
                </div>
            </div>
            <div class="fiori-card__footer">
                <div class="andon-detail-actions">
                    <button class="fiori-btn fiori-btn--secondary" onclick="closeAndonDetailModal()">Close</button>
                    <button class="fiori-btn fiori-btn--primary" onclick="exportSingleAndon()">Export</button>
                </div>
            </div>
        </div>
    </div>
</div>

<script src="../../assets/js/chart.js"></script>
<script src="../../assets/js/jquery-3.6.1.min.js"></script>
<script src="../../assets/js/moment.min.js"></script>
<script src="../../assets/js/daterangepicker.js"></script>
<script src="js/data_andon_2.js"></script>

</body>

</html>