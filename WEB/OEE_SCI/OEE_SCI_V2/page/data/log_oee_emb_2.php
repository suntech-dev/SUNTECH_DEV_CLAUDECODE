<?php
$page_title = 'EMB Data Log';
$page_css_files = [
    '../../assets/css/fiori-page.css',
    '../../assets/css/daterangepicker.css',
    'css/log_oee_emb_2.css',
];
require_once(__DIR__ . '/../../inc/head.php');
?>

<?php
$nav_context = 'data';
$nav_active = 'log_oee_emb';
require_once(__DIR__ . '/../../inc/nav-drawer-manage.php'); ?>

<div class="signage-header">
    <button id="navDrawerBtn" class="nav-drawer-btn" aria-label="Menu">&#9776;</button>
    <span class="signage-header__title">EMB Data Log</span>

    <div class="signage-header__filters">
        <?php include __DIR__ . '/inc/signage_filters.php'; ?>

        <div class="log-oee-emb-dropdown">
            <button id="columnToggleBtn" class="fiori-btn fiori-btn--secondary">Columns</button>
            <div id="columnToggleDropdown" class="log-oee-emb-dropdown__content"></div>
        </div>

        <button id="toggleStatsBtn" class="fiori-btn fiori-btn--secondary">Show Stats</button>
        <button id="excelDownloadBtn" class="fiori-btn fiori-btn--secondary">Export</button>
        <button id="refreshBtn" class="fiori-btn fiori-btn--tertiary">Refresh</button>
    </div>
</div>

<div class="log-oee-emb-main" id="logEmbMain">

    <!-- Row A: Stats (기본 hidden) -->
    <div id="logEmbStats" class="log-oee-emb log-oee-emb--stats hidden">
        <div class="oee-stats-grid">
            <?php include __DIR__ . '/inc/emb_stats_grid.php'; ?>
        </div>
    </div>

    <!-- Row B: Table -->
    <div id="logEmbTable" class="log-oee-emb log-oee-emb--table">
        <div class="fiori-card">
            <div class="fiori-card__header">
                <h3 class="fiori-card__title">EMB Data Log</h3>
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
                            <tr id="tableHeaderRow"></tr>
                        </thead>
                        <tbody id="oeeDataBody">
                            <tr>
                                <td colspan="25" class="data-table-centered">
                                    <div class="fiori-alert fiori-alert--info">
                                        <strong>Information:</strong> Loading EMB data log. Please wait...
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
    <div id="logEmbPagination" class="log-oee-emb log-oee-emb--pagination">
        <div id="pagination-controls" class="fiori-pagination"></div>
    </div>

</div>

<script src="../../assets/js/jquery-3.6.1.min.js"></script>
<script src="../../assets/js/moment.min.js"></script>
<script src="../../assets/js/daterangepicker.js"></script>
<script src="js/log_oee_emb_2.js"></script>

</body>
</html>
