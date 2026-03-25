<?php
$page_title = 'Defective Data Monitoring';
$page_css_files = [
    '../../assets/css/fiori-page.css',
    '../../assets/css/daterangepicker.css',
    'css/data_defective_2.css',
];

require_once(__DIR__ . '/../../inc/head.php');
?>

<?php $nav_context = 'data';
$nav_active = 'defective_m';
require_once(__DIR__ . '/../../inc/nav-drawer-manage.php'); ?>

<!-- Signage Header -->
<div class="signage-header">
    <button id="navDrawerBtn" class="nav-drawer-btn" aria-label="Menu">&#9776;</button>
    <span class="signage-header__title">Defective Monitoring</span>

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

<!-- Defective Signage Main -->
<div class="defective-signage-main" id="defectiveSignageMain">

    <!-- Row A: Stats (기본 hidden) -->
    <div id="defectiveRowStats" class="defective-row defective-row--stats hidden">
        <div class="defective-stats-grid">
            <div class="stat-card stat-card--red">
                <div class="stat-value" id="activeDefectives">-</div>
                <div class="stat-label">Active Defectives</div>
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
            <div class="stat-card">
                <div class="stat-value" id="totalDefectiveCount">-</div>
                <div class="stat-label">Total Count</div>
            </div>
        </div>
    </div>

    <!-- Row B: Charts Top — Active Defectives(2fr) + 차트 2개(3fr) (기본 hidden) -->
    <div id="defectiveRowChartsTop" class="defective-row defective-row--charts-top hidden">
        <div class="defective-charts-top-grid">

            <!-- 좌: Currently Active Defectives -->
            <div class="fiori-card">
                <div class="fiori-card__header">
                    <div class="card-title-row">
                        <h3 class="fiori-card__title">Currently Active Defectives</h3>
                    </div>
                    <div class="real-time-status">
                        <div class="status-dot"></div>
                        <span id="activeDefectiveCount">0 active defectives</span>
                    </div>
                </div>
                <div class="fiori-card__content">
                    <div class="defective-active-list" id="activeDefectivesContainer">
                        <div class="fiori-alert fiori-alert--info">
                            <strong>Information:</strong> There are currently no active Defectives.
                        </div>
                    </div>
                </div>
            </div>

            <!-- 우: 차트 2개 -->
            <div class="defective-charts-pair">
                <div class="fiori-card">
                    <div class="fiori-card__header">
                        <div class="card-title-row">
                            <h3 class="fiori-card__title">Defective Type Analysis</h3>
                            <span class="card-subtitle-inline">Frequency by defective type</span>
                        </div>
                    </div>
                    <div class="fiori-card__content">
                        <div class="chart-container"><canvas id="defectiveTypeChart"></canvas></div>
                    </div>
                </div>
                <div class="fiori-card">
                    <div class="fiori-card__header">
                        <div class="card-title-row">
                            <h3 class="fiori-card__title">Defective Status Distribution</h3>
                            <span class="card-subtitle-inline">Warning vs Completed</span>
                        </div>
                    </div>
                    <div class="fiori-card__content">
                        <div class="chart-container"><canvas id="defectiveStatusChart"></canvas></div>
                    </div>
                </div>
            </div>

        </div>
    </div>

    <!-- Row C: Charts Bottom — 3 Trend 차트 (기본 hidden) -->
    <div id="defectiveRowChartsBottom" class="defective-row defective-row--charts-bottom hidden">
        <div class="defective-charts-trio">

            <div class="fiori-card">
                <div class="fiori-card__header">
                    <div class="card-title-row">
                        <h3 class="fiori-card__title">Defective Count Trend</h3>
                        <span class="card-subtitle-inline">Hourly/Daily defective occurrence</span>
                    </div>
                </div>
                <div class="fiori-card__content">
                    <div class="chart-container"><canvas id="defectiveTrendChart"></canvas></div>
                </div>
            </div>

            <div class="fiori-card">
                <div class="fiori-card__header">
                    <div class="card-title-row">
                        <h3 class="fiori-card__title">Machine Defective Comparison</h3>
                        <span class="card-subtitle-inline">Defective count by machine</span>
                    </div>
                </div>
                <div class="fiori-card__content">
                    <div class="chart-container"><canvas id="defectiveMachineChart"></canvas></div>
                </div>
            </div>

            <div class="fiori-card" id="defectiveLineCard">
                <div class="fiori-card__header">
                    <div class="card-title-row">
                        <h3 class="fiori-card__title" id="defectiveLineCardTitle">Line Defective Performance</h3>
                        <span class="card-subtitle-inline" id="defectiveLineCardSubtitle">Defective count comparison by production line</span>
                    </div>
                </div>
                <div class="fiori-card__content">
                    <div class="chart-container"><canvas id="defectiveLineChart"></canvas></div>
                </div>
            </div>

        </div>
    </div>

    <!-- Row D: Real-time Defective Table -->
    <div id="defectiveRowTable" class="defective-row defective-row--table">
        <div class="fiori-card">
            <div class="fiori-card__header">
                <h3 class="fiori-card__title">Real-time Defective Data</h3>
                <div class="real-time-status">
                    <div class="status-dot"></div>
                    <span id="lastUpdateTime">Last updated: -</span>
                    <span id="connectionStatus" class="connection-status-info">Connection ready...</span>
                </div>
            </div>
            <div class="fiori-card__content">
                <div class="defective-table-wrap">
                    <table class="fiori-table" id="defectiveDataTable">
                        <thead class="fiori-table__header">
                            <tr>
                                <th>Machine No</th>
                                <th>Factory/Line</th>
                                <th>Shift</th>
                                <th>Defective Type</th>
                                <th>Occurrence Time</th>
                                <th>Elapsed Time</th>
                                <th>Work Date</th>
                                <th>Detail</th>
                            </tr>
                        </thead>
                        <tbody id="defectiveDataBody">
                            <tr>
                                <td colspan="8" class="data-table-centered">
                                    <div class="fiori-alert fiori-alert--info">
                                        <strong>Information:</strong> Loading real-time Defective data. Automatic monitoring is in progress.
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
    <div id="defectiveRowPagination" class="defective-row defective-row--pagination">
        <div id="pagination-controls" class="fiori-pagination"></div>
    </div>

</div><!-- /defective-signage-main -->


<!-- Defective Detail Modal -->
<div id="defectiveDetailModal" class="fiori-modal">
    <div class="fiori-modal__backdrop" onclick="closeDefectiveDetailModal()"></div>
    <div class="fiori-modal__content">
        <div class="fiori-card">
            <div class="fiori-card__header">
                <h3 class="fiori-card__title">Defective Details</h3>
                <button class="fiori-btn fiori-btn--icon" onclick="closeDefectiveDetailModal()"><span>&#10005;</span></button>
            </div>
            <div class="fiori-card__content">
                <div class="defective-detail-grid">
                    <div class="defective-detail-section">
                        <h4 class="defective-detail-section-title">Basic Information</h4>
                        <div class="defective-detail-row"><span class="defective-detail-label">Machine Number:</span><span class="defective-detail-value" id="modal-machine-no">-</span></div>
                        <div class="defective-detail-row"><span class="defective-detail-label">Factory/Line:</span><span class="defective-detail-value" id="modal-factory-line">-</span></div>
                        <div class="defective-detail-row"><span class="defective-detail-label">Defective Type:</span><span class="defective-detail-value" id="modal-defective-type">-</span></div>
                        <div class="defective-detail-row"><span class="defective-detail-label">Status:</span><span class="defective-detail-value" id="modal-status">-</span></div>
                    </div>
                    <div class="defective-detail-section">
                        <h4 class="defective-detail-section-title">Time Information</h4>
                        <div class="defective-detail-row"><span class="defective-detail-label">Occurrence Time:</span><span class="defective-detail-value" id="modal-reg-date">-</span></div>
                        <div class="defective-detail-row"><span class="defective-detail-label">Elapsed Time:</span><span class="defective-detail-value" id="modal-elapsed-time">-</span></div>
                        <div class="defective-detail-row"><span class="defective-detail-label">Work Date:</span><span class="defective-detail-value" id="modal-work-date">-</span></div>
                    </div>
                    <div class="defective-detail-section">
                        <h4 class="defective-detail-section-title">Work Information</h4>
                        <div class="defective-detail-row"><span class="defective-detail-label">Shift:</span><span class="defective-detail-value" id="modal-shift">-</span></div>
                        <div class="defective-detail-row">
                            <span class="defective-detail-label">Defective Color:</span>
                            <span class="defective-detail-value" id="modal-defective-color">
                                <span class="defective-color-indicator" id="modal-color-indicator"></span>
                                <span id="modal-color-value">Default Color</span>
                            </span>
                        </div>
                    </div>
                    <div class="defective-detail-section defective-detail-section--full">
                        <h4 class="defective-detail-section-title">Additional Information</h4>
                        <div class="defective-detail-row"><span class="defective-detail-label">Database ID:</span><span class="defective-detail-value" id="modal-idx">-</span></div>
                        <div class="defective-detail-row"><span class="defective-detail-label">Registration Date:</span><span class="defective-detail-value" id="modal-created-at">-</span></div>
                    </div>
                </div>
            </div>
            <div class="fiori-card__footer">
                <div class="defective-detail-actions">
                    <button class="fiori-btn fiori-btn--secondary" onclick="closeDefectiveDetailModal()">Close</button>
                    <button class="fiori-btn fiori-btn--primary" onclick="exportSingleDefective()">Export</button>
                </div>
            </div>
        </div>
    </div>
</div>


<script src="../../assets/js/chart.js"></script>
<script src="../../assets/js/jquery-3.6.1.min.js"></script>
<script src="../../assets/js/moment.min.js"></script>
<script src="../../assets/js/daterangepicker.js"></script>
<script src="js/data_defective_2.js"></script>


</body>

</html>