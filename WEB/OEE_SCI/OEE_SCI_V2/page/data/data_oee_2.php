<?php
$page_title = 'OEE Data Monitoring';
$page_css_files = [
    '../../assets/css/fiori-page.css',
    '../../assets/css/daterangepicker.css',
    'css/data_oee_2.css',
    'css/ai_dashboard.css',
];

require_once(__DIR__ . '/../../inc/head.php');
?>

<?php $nav_context = 'data';
$nav_active = 'oee';
require_once(__DIR__ . '/../../inc/nav-drawer-manage.php'); ?>

<!-- Signage Header -->
<div class="signage-header">
    <button id="navDrawerBtn" class="nav-drawer-btn" aria-label="Menu">&#9776;</button>
    <span class="signage-header__title">OEE Monitoring</span>

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

<!-- OEE Signage Main -->
<div class="oee-signage-main" id="oeeSignageMain">

    <!-- Row A: Stats (기본 hidden) -->
    <div id="oeeRowStats" class="oee-row oee-row--stats hidden">
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

    <!-- Row B: Charts Top — Components Details(2fr) + 차트 2개(3fr) (기본 hidden) -->
    <div id="oeeRowChartsTop" class="oee-row oee-row--charts-top hidden">
        <div class="oee-charts-top-grid">

            <!-- 좌: OEE Components Details -->
            <div class="fiori-card">
                <div class="fiori-card__header">
                    <div class="card-title-row">
                        <h3 class="fiori-card__title">OEE Components Details</h3>
                    </div>
                    <div class="real-time-status">
                        <div class="status-dot"></div>
                        <span id="oeeLiveStatus">Real-time monitoring active</span>
                    </div>
                </div>
                <div class="fiori-card__content">
                    <div class="oee-details-list" id="oeeDetailsContainer">
                        <div class="oee-component-item">
                            <div class="oee-component-info">
                                <div class="oee-component-name">OEE Rate</div>
                                <div class="oee-component-details">
                                    <span class="oee-detail-item">Overall Efficiency: <span id="overallEfficiency">-</span></span>
                                    <span class="oee-detail-item">Target Achievement: <span id="targetAchievement">-</span></span>
                                </div>
                            </div>
                            <div class="oee-component-value"><span id="oeeRateDetail">-</span></div>
                        </div>
                        <div class="oee-component-item">
                            <div class="oee-component-info">
                                <div class="oee-component-name">Availability Rate</div>
                                <div class="oee-component-details">
                                    <span class="oee-detail-item">Runtime: <span id="runtime">-</span>h</span>
                                    <span class="oee-detail-item">Planned Time: <span id="plannedTime">-</span>h</span>
                                </div>
                            </div>
                            <div class="oee-component-value"><span id="availabilityDetail">-</span></div>
                        </div>
                        <div class="oee-component-item">
                            <div class="oee-component-info">
                                <div class="oee-component-name">Performance Rate</div>
                                <div class="oee-component-details">
                                    <span class="oee-detail-item">Actual Output: <span id="actualOutput">-</span></span>
                                    <span class="oee-detail-item">Theoretical Output: <span id="theoreticalOutput">-</span></span>
                                </div>
                            </div>
                            <div class="oee-component-value"><span id="performanceDetail">-</span></div>
                        </div>
                        <div class="oee-component-item">
                            <div class="oee-component-info">
                                <div class="oee-component-name">Quality Rate</div>
                                <div class="oee-component-details">
                                    <span class="oee-detail-item">Good Products: <span id="goodProducts">-</span></span>
                                    <span class="oee-detail-item">Defective: <span id="defectiveProducts">-</span></span>
                                </div>
                            </div>
                            <div class="oee-component-value"><span id="qualityDetail">-</span></div>
                        </div>
                    </div>
                </div>
            </div>

            <!-- 우: 차트 2개 -->
            <div class="oee-charts-pair">
                <div class="fiori-card">
                    <div class="fiori-card__header">
                        <div class="card-title-row">
                            <h3 class="fiori-card__title">OEE Components</h3>
                            <span class="card-subtitle-inline">Availability, Performance, Quality</span>
                        </div>
                    </div>
                    <div class="fiori-card__content">
                        <div class="chart-container"><canvas id="oeeComponentChart"></canvas></div>
                    </div>
                </div>
                <div class="fiori-card">
                    <div class="fiori-card__header">
                        <div class="card-title-row">
                            <h3 class="fiori-card__title">OEE Grade Distribution</h3>
                            <span class="card-subtitle-inline">By performance grade</span>
                        </div>
                    </div>
                    <div class="fiori-card__content">
                        <div class="chart-container"><canvas id="oeeGradeChart"></canvas></div>
                    </div>
                </div>
            </div>

        </div>
    </div>

    <!-- Row C: Charts Bottom — 3 Trend 차트 (기본 hidden) -->
    <div id="oeeRowChartsBottom" class="oee-row oee-row--charts-bottom hidden">
        <div class="oee-charts-trio">

            <div class="fiori-card">
                <div class="fiori-card__header">
                    <div class="card-title-row">
                        <h3 class="fiori-card__title">
                            OEE Trend
                            <span class="ai-badge">AI POWERED</span>
                        </h3>
                        <span class="card-subtitle-inline">
                            Hourly OEE trend &nbsp;&middot;&nbsp; 점선 = AI 예측
                            &nbsp;<span id="aiOeeTrendBadge" class="ai-trend-badge ai-trend-badge--stable" style="display:none;"></span>
                        </span>
                    </div>
                </div>
                <div class="fiori-card__content">
                    <div class="chart-container"><canvas id="oeeTrendChart"></canvas></div>
                </div>
            </div>

            <div class="fiori-card">
                <div class="fiori-card__header">
                    <div class="card-title-row">
                        <h3 class="fiori-card__title">OEE Timeline</h3>
                        <span class="card-subtitle-inline">Hourly OEE timeline</span>
                    </div>
                </div>
                <div class="fiori-card__content">
                    <div class="chart-container"><canvas id="productionTrendChart"></canvas></div>
                </div>
            </div>

            <div class="fiori-card" id="oeeLineOeeCard">
                <div class="fiori-card__header">
                    <div class="card-title-row">
                        <h3 class="fiori-card__title" id="oeeLineOeeCardTitle">Line OEE Performance</h3>
                        <span class="card-subtitle-inline" id="oeeLineOeeCardSubtitle">OEE performance comparison by production line</span>
                    </div>
                </div>
                <div class="fiori-card__content">
                    <div class="chart-container"><canvas id="machineOeeChart"></canvas></div>
                </div>
            </div>

        </div>
    </div>

    <!-- Row D: Real-time OEE Table -->
    <div id="oeeRowTable" class="oee-row oee-row--table">
        <div class="fiori-card">
            <div class="fiori-card__header">
                <h3 class="fiori-card__title">Real-time OEE Data</h3>
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
                                        <strong>Information:</strong> Loading real-time OEE data. Automatic monitoring is in progress.
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
    <div id="oeeRowPagination" class="oee-row oee-row--pagination">
        <div id="pagination-controls" class="fiori-pagination"></div>
    </div>

</div><!-- /oee-signage-main -->


<!-- OEE Detail Modal -->
<div id="oeeDetailModal" class="fiori-modal">
    <div class="fiori-modal__backdrop" onclick="closeOeeDetailModal()"></div>
    <div class="fiori-modal__content">
        <div class="fiori-card">
            <div class="fiori-card__header">
                <h3 class="fiori-card__title">OEE Details</h3>
                <button class="fiori-btn fiori-btn--icon" onclick="closeOeeDetailModal()"><span>&#10005;</span></button>
            </div>
            <div class="fiori-card__content">
                <div class="oee-detail-grid">
                    <div class="oee-detail-section">
                        <h4 class="oee-detail-section-title">Basic Information</h4>
                        <div class="oee-detail-row"><span class="oee-detail-label">Machine Number:</span><span class="oee-detail-value" id="modal-machine-no">-</span></div>
                        <div class="oee-detail-row"><span class="oee-detail-label">Factory/Line:</span><span class="oee-detail-value" id="modal-factory-line">-</span></div>
                        <div class="oee-detail-row"><span class="oee-detail-label">Work Date:</span><span class="oee-detail-value" id="modal-work-date">-</span></div>
                        <div class="oee-detail-row"><span class="oee-detail-label">Shift:</span><span class="oee-detail-value" id="modal-shift">-</span></div>
                    </div>
                    <div class="oee-detail-section">
                        <h4 class="oee-detail-section-title">OEE Performance</h4>
                        <div class="oee-detail-row"><span class="oee-detail-label">Overall OEE:</span><span class="oee-detail-value" id="modal-overall-oee">-</span></div>
                        <div class="oee-detail-row"><span class="oee-detail-label">Availability:</span><span class="oee-detail-value" id="modal-availability">-</span></div>
                        <div class="oee-detail-row"><span class="oee-detail-label">Performance:</span><span class="oee-detail-value" id="modal-performance">-</span></div>
                        <div class="oee-detail-row"><span class="oee-detail-label">Quality:</span><span class="oee-detail-value" id="modal-quality">-</span></div>
                    </div>
                    <div class="oee-detail-section">
                        <h4 class="oee-detail-section-title">Time &amp; Production</h4>
                        <div class="oee-detail-row"><span class="oee-detail-label">Planned Work Time:</span><span class="oee-detail-value" id="modal-planned-time">-</span></div>
                        <div class="oee-detail-row"><span class="oee-detail-label">Runtime:</span><span class="oee-detail-value" id="modal-runtime">-</span></div>
                        <div class="oee-detail-row"><span class="oee-detail-label">Downtime:</span><span class="oee-detail-value" id="modal-downtime">-</span></div>
                        <div class="oee-detail-row"><span class="oee-detail-label">Actual Output:</span><span class="oee-detail-value" id="modal-actual-output">-</span></div>
                    </div>
                    <div class="oee-detail-section oee-detail-section--full">
                        <h4 class="oee-detail-section-title">Additional Information</h4>
                        <div class="oee-detail-row"><span class="oee-detail-label">Theoretical Output:</span><span class="oee-detail-value" id="modal-theoretical-output">-</span></div>
                        <div class="oee-detail-row"><span class="oee-detail-label">Defective Count:</span><span class="oee-detail-value" id="modal-defective">-</span></div>
                        <div class="oee-detail-row"><span class="oee-detail-label">Cycle Time:</span><span class="oee-detail-value" id="modal-cycletime">-</span></div>
                        <div class="oee-detail-row"><span class="oee-detail-label">Update Time:</span><span class="oee-detail-value" id="modal-update-time">-</span></div>
                    </div>
                </div>
            </div>
            <div class="fiori-card__footer">
                <div class="oee-detail-actions">
                    <button class="fiori-btn fiori-btn--secondary" onclick="closeOeeDetailModal()">Close</button>
                    <button class="fiori-btn fiori-btn--primary" onclick="exportSingleOee()">Export</button>
                </div>
            </div>
        </div>
    </div>
</div>


<script src="../../assets/js/chart.js"></script>
<script src="../../assets/js/jquery-3.6.1.min.js"></script>
<script src="../../assets/js/moment.min.js"></script>
<script src="../../assets/js/daterangepicker.js"></script>
<script src="js/data_oee_2.js"></script>
<script src="js/ai_oee_overlay.js"></script>


</body>

</html>