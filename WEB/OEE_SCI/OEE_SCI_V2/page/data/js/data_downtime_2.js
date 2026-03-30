// data_downtime_2.js — Row Grid Layout version (data_oee_2 model)

let eventSource = null;
let isTracking = false;
let charts = {};
let downtimeData = [];
let reconnectAttempts = 0;
let maxReconnectAttempts = 3;
let stats = {};
let activeDowntimesList = [];
let elapsedTimeTimer = null;
let durationUpdateTimer = null;
let isPageUnloading = false;

let currentPage = 1;
let itemsPerPage = 10;
let totalItems = 0;

Chart.defaults.color = '#1a1a1a';
Chart.defaults.borderColor = '#e8eaed';

const chartColors = {
    warning:   '#da1e28',
    completed: '#30914c',
    primary:   '#0070f2',
    info:      '#7c3aed',
    secondary: '#1e88e5',
    accent:    '#00d4aa',
    orange:    '#e26b0a',
    maroon:    '#800000'
};

document.addEventListener('DOMContentLoaded', async function () {
    initDateRangePicker();
    await initFilterSystem();
    initCharts();
    setupEventListeners();
    updateLayout(); // 초기 grid-template-rows 설정
    await loadInitialData();
    await startAutoTracking();
});

// ─── 페이지 언로드 처리 ──────────────────────────────────
window.addEventListener('beforeunload', function () {
    isPageUnloading = true;
    if (eventSource) { eventSource.close(); eventSource = null; isTracking = false; }
    stopElapsedTimeTimer();
    stopDurationUpdateTimer();
});

window.addEventListener('pagehide', function () {
    isPageUnloading = true;
    if (eventSource) { eventSource.close(); eventSource = null; isTracking = false; }
    stopElapsedTimeTimer();
    stopDurationUpdateTimer();
});

document.addEventListener('visibilitychange', function () {
    if (document.hidden) {
        if (eventSource && isTracking) { eventSource.close(); eventSource = null; isTracking = false; }
        stopElapsedTimeTimer();
        stopDurationUpdateTimer();
    } else {
        if (!isPageUnloading && !isTracking && eventSource === null) {
            reconnectAttempts = 0;
            startAutoTracking();
        }
    }
});

window.addEventListener('focus', function () { if (isPageUnloading) isPageUnloading = false; });

// ─── 레이아웃 동적 업데이트 (핵심 함수) ──────────────────
function updateLayout() {
    var main = document.getElementById('dtSignageMain');
    if (!main) return;

    var rows = [];
    if (!document.getElementById('dtRowStats').classList.contains('hidden'))        rows.push('auto');
    if (!document.getElementById('dtRowChartsTop').classList.contains('hidden'))    rows.push('1fr');
    if (!document.getElementById('dtRowChartsBottom').classList.contains('hidden')) rows.push('1fr');
    if (!document.getElementById('dtRowTable').classList.contains('hidden'))        rows.push('1fr');
    rows.push('auto'); // pagination

    main.style.gridTemplateRows = rows.join(' ');

    // Chart.js ResizeObserver 트리거 (DOM 레이아웃 반영 후)
    setTimeout(function () {
        Object.values(charts).forEach(function (c) { if (c) c.resize(); });
    }, 50);
}

// ─── 토글 함수 ────────────────────────────────────────────
function toggleStatsDisplay() {
    var row = document.getElementById('dtRowStats');
    var btn = document.getElementById('toggleStatsBtn');
    if (!row || !btn) return;

    row.classList.toggle('hidden');
    btn.textContent = row.classList.contains('hidden') ? 'Show Stats' : 'Hide Stats';
    updateLayout();
}

function toggleChartsDisplay() {
    var rowTop    = document.getElementById('dtRowChartsTop');
    var rowBottom = document.getElementById('dtRowChartsBottom');
    var btn       = document.getElementById('toggleChartsBtn');
    if (!rowTop || !rowBottom || !btn) return;

    var isHidden = rowTop.classList.contains('hidden');
    rowTop.classList.toggle('hidden', !isHidden);
    rowBottom.classList.toggle('hidden', !isHidden);
    btn.textContent = isHidden ? 'Hide Charts' : 'Show Charts';
    updateLayout();
}

function toggleDataDisplay() {
    var rowTable      = document.getElementById('dtRowTable');
    var rowPagination = document.getElementById('dtRowPagination');
    var btn           = document.getElementById('toggleDataBtn');
    if (!rowTable || !btn) return;

    rowTable.classList.toggle('hidden');
    if (rowPagination) rowPagination.classList.toggle('hidden');
    btn.textContent = rowTable.classList.contains('hidden') ? 'Show Table' : 'Hide Table';
    updateLayout();
}

// ─── DateRangePicker ──────────────────────────────────────
function initDateRangePicker() {
    try {
        $('#dateRangePicker').daterangepicker({
            opens: 'left',
            locale: {
                format: 'YYYY-MM-DD',
                separator: ' ~ ',
                applyLabel: 'Apply',
                cancelLabel: 'Cancel',
                daysOfWeek: ['Sun', 'Mon', 'Tue', 'Wed', 'Thu', 'Fri', 'Sat'],
                monthNames: ['Jan', 'Feb', 'Mar', 'Apr', 'May', 'Jun', 'Jul', 'Aug', 'Sep', 'Oct', 'Nov', 'Dec'],
                firstDay: 0
            },
            ranges: {
                'today':      [moment(), moment()],
                'yesterday':  [moment().subtract(1, 'days'), moment().subtract(1, 'days')],
                'last week':  [moment().subtract(6, 'days'), moment()],
                'last month': [moment().subtract(29, 'days'), moment()]
            },
            startDate: moment().startOf('day'),
            endDate:   moment().endOf('day'),
            showDropdowns: true,
            alwaysShowCalendars: true
        }, function (start, end) {
            handleDateRangeChange(start, end);
        });
        $('#dateRangePicker').val(moment().format('YYYY-MM-DD') + ' ~ ' + moment().format('YYYY-MM-DD'));
    } catch (e) {
        console.error('DateRangePicker init error:', e);
    }
}

// ─── 필터 시스템 ──────────────────────────────────────────
async function initFilterSystem() {
    await loadFactoryOptions();
    await loadLineOptions();
    await loadMachineOptions();
}

async function loadFactoryOptions() {
    try {
        var res = await fetch('../manage/proc/factory.php?status_filter=Y').then(function (r) { return r.json(); });
        if (!res.success || !res.data) return;
        var sel = document.getElementById('factoryFilterSelect');
        sel.innerHTML = '<option value="">All Factory</option>';
        res.data.forEach(function (f) {
            sel.innerHTML += '<option value="' + f.idx + '">' + f.factory_name + '</option>';
        });
        sel.addEventListener('change', async function (e) {
            document.getElementById('factoryLineMachineFilterSelect').innerHTML = '<option value="">All Machine</option>';
            document.getElementById('factoryLineMachineFilterSelect').disabled = true;
            await updateLineOptions(e.target.value);
            await restartRealTimeMonitoring();
        });
    } catch (e) { console.error('Factory options error:', e); }
}

async function loadLineOptions() {
    try {
        var res = await fetch('../manage/proc/line.php?status_filter=Y').then(function (r) { return r.json(); });
        if (!res.success || !res.data) return;
        var sel = document.getElementById('factoryLineFilterSelect');
        sel.innerHTML = '<option value="">All Line</option>';
        res.data.forEach(function (l) {
            sel.innerHTML += '<option value="' + l.idx + '">' + l.line_name + '</option>';
        });
        sel.addEventListener('change', async function (e) {
            await updateMachineOptions(document.getElementById('factoryFilterSelect').value, e.target.value);
            await restartRealTimeMonitoring();
        });
    } catch (e) { console.error('Line options error:', e); }
}

async function loadMachineOptions() {
    try {
        var res = await fetch('../manage/proc/machine.php').then(function (r) { return r.json(); });
        if (!res.success || !res.data) return;
        var sel = document.getElementById('factoryLineMachineFilterSelect');
        sel.innerHTML = '<option value="">All Machine</option>';
        res.data.forEach(function (m) {
            sel.innerHTML += '<option value="' + m.idx + '">' + m.machine_no + ' (' + (m.machine_model_name || 'No Model') + ')</option>';
        });
    } catch (e) { console.error('Machine options error:', e); }
}

async function updateLineOptions(factoryId) {
    var sel = document.getElementById('factoryLineFilterSelect');
    sel.disabled = true;
    try {
        var url = '../manage/proc/line.php?status_filter=Y' + (factoryId ? '&factory_filter=' + factoryId : '');
        var res = await fetch(url).then(function (r) { return r.json(); });
        sel.innerHTML = '<option value="">All Line</option>';
        if (res.success) res.data.forEach(function (l) { sel.innerHTML += '<option value="' + l.idx + '">' + l.line_name + '</option>'; });
        sel.disabled = false;
        if (factoryId) {
            document.getElementById('factoryLineMachineFilterSelect').disabled = false;
            updateMachineOptions(factoryId, '');
        }
    } catch (e) { sel.innerHTML = '<option value="">All Line</option>'; sel.disabled = false; }
}

async function updateMachineOptions(factoryId, lineId) {
    var sel = document.getElementById('factoryLineMachineFilterSelect');
    sel.disabled = true;
    try {
        var params = new URLSearchParams();
        if (factoryId) params.append('factory_filter', factoryId);
        if (lineId)    params.append('line_filter', lineId);
        var url = '../manage/proc/machine.php' + (params.toString() ? '?' + params.toString() : '');
        var res = await fetch(url).then(function (r) { return r.json(); });
        sel.innerHTML = '<option value="">All Machine</option>';
        if (res.success) res.data.forEach(function (m) { sel.innerHTML += '<option value="' + m.idx + '">' + m.machine_no + ' (' + (m.machine_model_name || 'No Model') + ')</option>'; });
        sel.disabled = false;
    } catch (e) { sel.innerHTML = '<option value="">All Machine</option>'; sel.disabled = false; }
}

// ─── 이벤트 리스너 ────────────────────────────────────────
function setupEventListeners() {
    var el;

    el = document.getElementById('excelDownloadBtn');
    if (el) el.addEventListener('click', exportData);

    el = document.getElementById('refreshBtn');
    if (el) el.addEventListener('click', refreshData);

    el = document.getElementById('toggleStatsBtn');
    if (el) el.addEventListener('click', toggleStatsDisplay);

    el = document.getElementById('toggleChartsBtn');
    if (el) el.addEventListener('click', toggleChartsDisplay);

    el = document.getElementById('toggleDataBtn');
    if (el) el.addEventListener('click', toggleDataDisplay);

    el = document.getElementById('timeRangeSelect');
    if (el) el.addEventListener('change', handleTimeRangeChange);

    el = document.getElementById('factoryLineMachineFilterSelect');
    if (el) el.addEventListener('change', function () { restartRealTimeMonitoring(); });

    el = document.getElementById('shiftSelect');
    if (el) el.addEventListener('change', function () { restartRealTimeMonitoring(); });
}

// ─── 초기 데이터 ──────────────────────────────────────────
async function loadInitialData() {
    displayEmptyState();
}

function displayEmptyState() {
    ['totalDowntime', 'activeDowntimes', 'currentShiftDowntime',
     'affectedMachinesDowntime', 'longDowntimes', 'avgDowntimeResolution'].forEach(function (id) {
        var el = document.getElementById(id);
        if (el) el.textContent = '-';
    });
    var tbody = document.getElementById('downtimeDataBody');
    if (tbody) tbody.innerHTML = '<tr><td colspan="11" class="data-table-centered"><div class="fiori-alert fiori-alert--info"><strong>Information:</strong> Loading real-time Downtime data.</div></td></tr>';
}

// ─── 차트 초기화 ──────────────────────────────────────────
function initCharts() {
    charts.dtType     = createDtTypeChart();
    charts.dtStatus   = createDtStatusChart();
    charts.dtTrend    = createDtTrendChart();
    charts.dtLine     = createDtLineChart();
    charts.dtDuration = createDtDurationChart();
}

function createDtTypeChart() {
    var ctx = document.getElementById('dtTypeChart').getContext('2d');
    return new Chart(ctx, {
        type: 'bar',
        data: {
            labels: [],
            datasets: [{
                label: 'Duration (min)',
                data: [],
                backgroundColor: [
                    chartColors.orange, chartColors.warning, chartColors.primary,
                    chartColors.info, chartColors.accent, chartColors.secondary
                ],
                borderColor: [
                    chartColors.orange, chartColors.warning, chartColors.primary,
                    chartColors.info, chartColors.accent, chartColors.secondary
                ],
                borderWidth: 1
            }]
        },
        options: {
            responsive: true,
            maintainAspectRatio: false,
            plugins: {
                legend: { display: false },
                tooltip: { callbacks: { label: function (c) { return c.dataset.label + ': ' + (c.parsed.y || 0).toFixed(1) + ' min'; } } }
            },
            scales: {
                y: { beginAtZero: true, title: { display: true, text: 'Duration (min)' }, ticks: { callback: function (v) { return v + 'm'; } } },
                x: { title: { display: true, text: 'Downtime Type' } }
            }
        }
    });
}

function createDtStatusChart() {
    var ctx = document.getElementById('dtStatusChart').getContext('2d');
    return new Chart(ctx, {
        type: 'doughnut',
        data: {
            labels: ['Active (Warning)', 'Completed'],
            datasets: [{
                label: 'Status',
                data: [0, 0],
                backgroundColor: [chartColors.warning + 'CC', chartColors.completed + 'CC'],
                borderColor:     [chartColors.warning, chartColors.completed],
                borderWidth: 2,
                hoverOffset: 10
            }]
        },
        options: {
            responsive: true,
            maintainAspectRatio: false,
            plugins: {
                legend: { display: true, position: 'bottom', labels: { padding: 16, usePointStyle: true } },
                tooltip: { callbacks: { label: function (c) { var t = c.dataset.data.reduce(function (a, b) { return a + b; }, 0); var p = t > 0 ? ((c.raw / t) * 100).toFixed(1) : 0; return c.label + ': ' + c.raw + ' (' + p + '%)'; } } }
            }
        }
    });
}

function createDtTrendChart() {
    var ctx = document.getElementById('dtTrendChart').getContext('2d');
    return new Chart(ctx, {
        type: 'line',
        data: {
            labels: [],
            datasets: [
                {
                    label: 'Warning',
                    data: [],
                    borderColor: chartColors.warning,
                    backgroundColor: chartColors.warning + '20',
                    fill: true, tension: 0.4, pointRadius: 4, pointHoverRadius: 6
                },
                {
                    label: 'Completed',
                    data: [],
                    borderColor: chartColors.completed,
                    backgroundColor: chartColors.completed + '20',
                    fill: true, tension: 0.4, pointRadius: 4, pointHoverRadius: 6
                }
            ]
        },
        options: {
            responsive: true,
            maintainAspectRatio: false,
            plugins: { legend: { display: true, position: 'top' }, tooltip: { mode: 'index', intersect: false } },
            scales: {
                y: { beginAtZero: true, title: { display: true, text: 'Count' }, ticks: { stepSize: 1, precision: 0 } },
                x: { title: { display: true, text: 'Time/Period' } }
            }
        }
    });
}

function createDtLineChart() {
    var ctx = document.getElementById('dtLineChart').getContext('2d');
    return new Chart(ctx, {
        type: 'bar',
        data: {
            labels: [],
            datasets: [
                { label: 'Warning', data: [], backgroundColor: chartColors.warning + '80', borderColor: chartColors.warning, borderWidth: 1 },
                { label: 'Completed', data: [], backgroundColor: chartColors.completed + '80', borderColor: chartColors.completed, borderWidth: 1 }
            ]
        },
        options: {
            responsive: true,
            maintainAspectRatio: false,
            plugins: { legend: { display: true, position: 'top' } },
            scales: {
                y: { beginAtZero: true, title: { display: true, text: 'Count' }, ticks: { precision: 0 } },
                x: { title: { display: true, text: 'Line / Machine' } }
            }
        }
    });
}

function createDtDurationChart() {
    var ctx = document.getElementById('dtDurationChart').getContext('2d');
    return new Chart(ctx, {
        type: 'bar',
        data: {
            labels: ['< 5min', '5–15min', '15–30min', '30–60min', '> 60min'],
            datasets: [{
                label: 'Count',
                data: [0, 0, 0, 0, 0],
                backgroundColor: [
                    chartColors.completed + 'CC',
                    chartColors.primary + 'CC',
                    chartColors.orange + 'CC',
                    chartColors.warning + 'CC',
                    chartColors.maroon + 'CC'
                ],
                borderColor: [
                    chartColors.completed, chartColors.primary,
                    chartColors.orange, chartColors.warning, chartColors.maroon
                ],
                borderWidth: 1
            }]
        },
        options: {
            responsive: true,
            maintainAspectRatio: false,
            plugins: { legend: { display: false } },
            scales: {
                y: { beginAtZero: true, title: { display: true, text: 'Count' }, ticks: { precision: 0 } },
                x: { title: { display: true, text: 'Duration Bucket' } }
            }
        }
    });
}

// ─── SSE / 데이터 로딩 ────────────────────────────────────
function getFilterParams() {
    var dateRange = $('#dateRangePicker').val();
    var startDate = '', endDate = '';
    if (dateRange && dateRange.includes(' ~ ')) {
        var parts = dateRange.split(' ~ ');
        startDate = parts[0]; endDate = parts[1];
    }
    return {
        factory_filter: document.getElementById('factoryFilterSelect').value,
        line_filter:    document.getElementById('factoryLineFilterSelect').value,
        machine_filter: document.getElementById('factoryLineMachineFilterSelect').value,
        shift_filter:   document.getElementById('shiftSelect').value,
        start_date:     startDate,
        end_date:       endDate,
        limit: 100
    };
}

async function startAutoTracking() {
    if (isTracking) return;
    var params = new URLSearchParams(getFilterParams());
    eventSource = new EventSource('proc/data_downtime_stream_2.php?' + params.toString());
    setupSSEEventListeners();
    isTracking = true;
    var el = document.getElementById('connectionStatus');
    if (el) el.textContent = 'Downtime System Real-time Connected';
}

function setupSSEEventListeners() {
    eventSource.onerror = function () {
        isTracking = false;
        attemptReconnection();
    };

    eventSource.addEventListener('connected', function (e) {
        reconnectAttempts = 0;
        var el = document.getElementById('connectionStatus');
        if (el) el.textContent = 'Downtime system connected';
    });

    eventSource.addEventListener('downtime_data', function (e) {
        var data = JSON.parse(e.data);
        stats             = data.stats;
        activeDowntimesList = data.active_downtimes || [];
        downtimeData      = data.downtime_data || [];
        window.downtimeData = downtimeData;

        updateStatCardsFromAPI(stats);
        updateDetailsFromAPI(stats, activeDowntimesList);
        updateTableFromAPI(downtimeData);
        updateChartsFromAPI(data);

        var el = document.getElementById('lastUpdateTime');
        if (el) el.textContent = 'Last updated: ' + data.timestamp;
    });

    eventSource.addEventListener('heartbeat', function (e) {
        var data = JSON.parse(e.data);
        var el = document.getElementById('connectionStatus');
        if (el) el.textContent = 'Connection maintained (Active warnings: ' + (data.active_warnings || 0) + ')';
    });

    eventSource.addEventListener('disconnected', function () {
        var el = document.getElementById('connectionStatus');
        if (el) el.textContent = 'Downtime System Connection Closed';
    });
}

function stopTracking() {
    if (!isTracking) return;
    if (eventSource) { eventSource.close(); eventSource = null; }
    isTracking = false;
    var el = document.getElementById('connectionStatus');
    if (el) el.textContent = 'Downtime System Connection Closed';
}

async function restartRealTimeMonitoring() {
    currentPage = 1;
    if (isTracking) { stopTracking(); await new Promise(function (r) { setTimeout(r, 100); }); }
    reconnectAttempts = 0;
    await startAutoTracking();
}

async function attemptReconnection() {
    if (isPageUnloading || reconnectAttempts >= maxReconnectAttempts) return;
    reconnectAttempts++;
    var delay = Math.min(1000 * Math.pow(2, reconnectAttempts - 1), 10000);
    if (eventSource) { eventSource.close(); eventSource = null; }
    setTimeout(async function () {
        if (isPageUnloading) return;
        try { await startAutoTracking(); } catch (e) { attemptReconnection(); }
    }, delay);
}

// ─── 데이터 업데이트 ──────────────────────────────────────
function updateStatCardsFromAPI(s) {
    if (!s) return;
    var map = {
        totalDowntime:           s.total_count     || '0',
        activeDowntimes:         s.warning_count   || '0',
        currentShiftDowntime:    s.current_shift_count || '0',
        affectedMachinesDowntime: s.affected_machines || '0',
        longDowntimes:           s.long_downtimes_count || '0',
        avgDowntimeResolution:   s.avg_completed_time  || '-'
    };
    Object.keys(map).forEach(function (id) {
        var el = document.getElementById(id);
        if (el) el.textContent = map[id];
    });
}

function updateDetailsFromAPI(s, activeList) {
    if (!s) return;

    var totalCount     = parseInt(s.total_count)    || 0;
    var warningCount   = parseInt(s.warning_count)  || 0;
    var completedCount = totalCount - warningCount;

    var set = function (id, v) { var el = document.getElementById(id); if (el) el.textContent = v; };

    // Row 1: Total Downtime
    set('dtTotalDetail',          totalCount);
    set('dtDetailWarningCount',   warningCount);
    set('dtDetailCompletedCount', completedCount);

    // Row 2: Current Shift Impact
    set('dtShiftDetail',         s.current_shift_count || '0');
    set('dtDetailShiftCount',    s.current_shift_count || '0');
    set('dtDetailLongDowntimes', s.long_downtimes_count || '0');

    // Row 3: Machine Availability
    set('dtMachineDetail',          s.affected_machines || '0');
    set('dtDetailAffectedMachines', s.affected_machines || '0');
    set('dtDetailMaxDuration',      s.max_duration_display || '-');

    // Row 4: Resolution Performance
    set('dtResolutionDetail',   s.avg_completed_time || '-');
    set('dtDetailAvgResolution', s.avg_completed_time || '-');
    set('dtDetailOver30',        s.long_downtimes_count || '0');
}

// ─── 테이블 업데이트 ──────────────────────────────────────
function updateTableFromAPI(list) {
    var tbody = document.getElementById('downtimeDataBody');
    if (!tbody) return;
    totalItems = list.length;

    if (list.length === 0) {
        tbody.innerHTML = '<tr><td colspan="11" class="data-table-centered"><div class="fiori-alert fiori-alert--info"><strong>Information:</strong> No Downtime data matching the selected conditions.</div></td></tr>';
        renderPagination();
        return;
    }

    var start = (currentPage - 1) * itemsPerPage;
    var paged = list.slice(start, start + itemsPerPage);
    tbody.innerHTML = '';

    paged.forEach(function (dt) {
        var statusBadge = dt.status === 'Warning'
            ? '<span class="fiori-badge fiori-badge--error">Warning</span>'
            : '<span class="fiori-badge fiori-badge--success">Completed</span>';

        var shift = dt.shift_idx ? 'Shift ' + dt.shift_idx : '-';

        var durationCell;
        if (dt.status === 'Warning' && dt.reg_date) {
            var initial = calculateElapsedTime(dt.reg_date);
            durationCell = '<td class="duration-in-progress" data-start-time="' + dt.reg_date + '">' + initial + '</td>';
        } else {
            durationCell = '<td>' + (dt.duration_display || dt.duration_his || '-') + '</td>';
        }

        var tr = document.createElement('tr');
        tr.innerHTML =
            '<td>' + (dt.machine_no || '-') + '</td>' +
            '<td>' + (dt.factory_name || '') + ' / ' + (dt.line_name || '') + '</td>' +
            '<td>' + shift + '</td>' +
            '<td>' + (dt.downtime_name || '-') + '</td>' +
            '<td>' + statusBadge + '</td>' +
            '<td>' + (dt.reg_date || '-') + '</td>' +
            '<td>' + (dt.update_date || '-') + '</td>' +
            durationCell +
            '<td>' + (dt.work_date || '-') + '</td>' +
            '<td></td>' + // AI Risk — ai_downtime_risk_2.js 가 채움
            '<td><button class="fiori-btn fiori-btn--tertiary dt-details-btn" style="padding:.25rem .5rem;font-size:.75rem;" data-dt-data=\'' + JSON.stringify(dt).replace(/'/g, '&#39;') + '\'>Details</button></td>';
        tbody.appendChild(tr);
    });

    renderPagination();
    setupDetailsButtonListeners();
    startDurationUpdateTimer();
}

function setupDetailsButtonListeners() {
    document.querySelectorAll('.dt-details-btn').forEach(function (btn) {
        btn.removeEventListener('click', handleDetailsButtonClick);
        btn.addEventListener('click', handleDetailsButtonClick);
    });
}

function handleDetailsButtonClick(e) {
    e.preventDefault();
    e.stopPropagation();
    var btn = e.target.closest('.dt-details-btn');
    if (btn) openDowntimeDetailModal(btn);
}

// ─── 페이지네이션 ──────────────────────────────────────────
function renderPagination() {
    var container = document.getElementById('pagination-controls');
    if (!container) return;
    var totalPages = Math.ceil(totalItems / itemsPerPage);
    if (totalPages <= 1) { container.innerHTML = ''; return; }

    var s = totalItems === 0 ? 0 : (currentPage - 1) * itemsPerPage + 1;
    var e = Math.min(currentPage * itemsPerPage, totalItems);
    var html = '<div class="fiori-pagination__info">' + s + '-' + e + ' / ' + totalItems + ' items</div>';
    html += '<div class="fiori-pagination__buttons">';
    html += '<button class="fiori-pagination__button' + (currentPage === 1 ? ' fiori-pagination__button--disabled' : '') + '" onclick="changePage(' + (currentPage - 1) + ')" ' + (currentPage === 1 ? 'disabled' : '') + '>&larr;</button>';

    var sp = Math.max(1, currentPage - 2);
    var ep = Math.min(totalPages, currentPage + 2);
    if (sp > 1) { html += '<button class="fiori-pagination__button" onclick="changePage(1)">1</button>'; if (sp > 2) html += '<span class="fiori-pagination__ellipsis">...</span>'; }
    for (var i = sp; i <= ep; i++) html += '<button class="fiori-pagination__button' + (i === currentPage ? ' fiori-pagination__button--active' : '') + '" onclick="changePage(' + i + ')">' + i + '</button>';
    if (ep < totalPages) { if (ep < totalPages - 1) html += '<span class="fiori-pagination__ellipsis">...</span>'; html += '<button class="fiori-pagination__button" onclick="changePage(' + totalPages + ')">' + totalPages + '</button>'; }

    html += '<button class="fiori-pagination__button' + (currentPage === totalPages ? ' fiori-pagination__button--disabled' : '') + '" onclick="changePage(' + (currentPage + 1) + ')" ' + (currentPage === totalPages ? 'disabled' : '') + '>&rarr;</button>';
    html += '</div>';
    container.innerHTML = html;
}

function changePage(p) {
    var total = Math.ceil(totalItems / itemsPerPage);
    if (p < 1 || p > total || p === currentPage) return;
    currentPage = p;
    updateTableFromAPI(downtimeData);
}

// ─── 차트 업데이트 ────────────────────────────────────────
function updateChartsFromAPI(data) {
    if (data.downtime_type_stats  && charts.dtType)  updateDtTypeChart(data.downtime_type_stats);
    if (data.stats                && charts.dtStatus) updateDtStatusChart(data.stats);
    if (data.downtime_trend_stats && charts.dtTrend)  updateDtTrendChart(data.downtime_trend_stats);

    // dtLine: machine_stats 혹은 downtime_data 에서 유도
    if (data.downtime_machine_stats && charts.dtLine) {
        updateDtLineChartFromAPI(data.downtime_machine_stats);
    } else if (downtimeData.length > 0 && charts.dtLine) {
        updateDtLineChartFromData(downtimeData);
    }

    // dtDuration: downtime_data 에서 클라이언트 계산
    if (downtimeData.length > 0 && charts.dtDuration) {
        updateDtDurationChartFromData(downtimeData);
    }
}

function updateDtTypeChart(stat) {
    if (!charts.dtType || !stat || stat.length === 0) return;
    var sorted = stat.slice().sort(function (a, b) {
        return (a.downtime_name || '').localeCompare(b.downtime_name || '');
    });
    var labels  = sorted.map(function (d) { return d.downtime_name || 'Unclassified'; });
    var durations = sorted.map(function (d) { return parseFloat(d.total_duration_min) || 0; });

    var colorKeys = ['orange', 'warning', 'primary', 'info', 'accent', 'secondary'];
    var bgColors = labels.map(function (_, i) { return chartColors[colorKeys[i % colorKeys.length]]; });

    charts.dtType.data.labels                        = labels;
    charts.dtType.data.datasets[0].data              = durations;
    charts.dtType.data.datasets[0].backgroundColor   = bgColors;
    charts.dtType.data.datasets[0].borderColor       = bgColors;
    charts.dtType.update('none');
}

function updateDtStatusChart(s) {
    if (!charts.dtStatus || !s) return;
    var warning   = parseInt(s.warning_count)  || 0;
    var total     = parseInt(s.total_count)    || 0;
    var completed = Math.max(0, total - warning);
    charts.dtStatus.data.datasets[0].data = [warning, completed];
    charts.dtStatus.update('none');
}

function updateDtTrendChart(stat) {
    if (!charts.dtTrend) return;
    if (!stat || !stat.data || stat.data.length === 0) {
        if (window.downtimeData && window.downtimeData.length > 0) { generateSimpleTrendFromDowntimeData(); return; }
        charts.dtTrend.data.labels = ['No Data'];
        charts.dtTrend.data.datasets[0].data = [0];
        charts.dtTrend.data.datasets[1].data = [0];
        charts.dtTrend.update('none');
        return;
    }

    var trendData = stat.data;
    var workHours = stat.work_hours;
    var viewType  = stat.view_type || 'hourly';
    var labels    = [];

    if (viewType === 'hourly' && workHours && workHours.start_time) {
        labels = generateWorkHoursLabels(workHours.start_time, workHours.end_time, workHours.start_minutes, workHours.end_minutes);
    } else if (viewType === 'daily') {
        labels = trendData.map(function (d) { return d.display_label || d.time_label; });
    } else {
        labels = trendData.map(function (d) {
            var lbl = d.time_label || d.display_label;
            var m = lbl && lbl.match(/^(\d{4}-\d{2}-\d{2})\s+(\d{2}):/);
            return m ? m[2] + 'H' : lbl;
        });
    }

    var warnMap = {}, compMap = {};
    trendData.forEach(function (d) {
        var key = viewType === 'daily'
            ? (d.display_label || d.time_label)
            : (function () { var m = (d.time_label || '').match(/^(\d{4}-\d{2}-\d{2})\s+(\d{2}):/); return m ? m[2] + 'H' : ''; })();
        if (key) { warnMap[key] = parseInt(d.warning_count) || 0; compMap[key] = parseInt(d.completed_count) || 0; }
    });

    charts.dtTrend.data.labels = labels;
    charts.dtTrend.data.datasets[0].data = labels.map(function (l) { return warnMap[l] || 0; });
    charts.dtTrend.data.datasets[1].data = labels.map(function (l) { return compMap[l] || 0; });
    charts.dtTrend.options.scales.x.title.text = viewType === 'hourly' ? 'Time (within 1 day)' : 'Date';
    charts.dtTrend.update('show');
}

function generateWorkHoursLabels(startTime, endTime, startMinutes, endMinutes) {
    var labels = [];
    var sh = startMinutes !== null && startMinutes !== undefined ? Math.floor(startMinutes / 60) : parseInt(startTime.split(':')[0]);
    var eh = endMinutes   !== null && endMinutes   !== undefined ? Math.floor(endMinutes / 60)   : parseInt(endTime.split(':')[0]);
    if (eh < sh) eh += 24;
    for (var h = sh; h <= eh; h++) labels.push(String(h % 24).padStart(2, '0') + 'H');
    return labels;
}

function generateSimpleTrendFromDowntimeData() {
    if (!charts.dtTrend || !window.downtimeData || window.downtimeData.length === 0) return;
    var grp = {};
    window.downtimeData.forEach(function (d) {
        if (!d.work_date) return;
        if (!grp[d.work_date]) grp[d.work_date] = { warn: 0, comp: 0 };
        if (d.status === 'Warning') grp[d.work_date].warn++;
        else grp[d.work_date].comp++;
    });
    var dates = Object.keys(grp).sort();
    charts.dtTrend.data.labels = dates.map(function (d) { var x = new Date(d); return (x.getMonth() + 1) + '/' + x.getDate(); });
    charts.dtTrend.data.datasets[0].data = dates.map(function (d) { return grp[d].warn; });
    charts.dtTrend.data.datasets[1].data = dates.map(function (d) { return grp[d].comp; });
    charts.dtTrend.options.scales.x.title.text = 'Date';
    charts.dtTrend.update('show');
}

function updateDtLineChartFromAPI(stat) {
    if (!charts.dtLine || !stat || !Array.isArray(stat) || stat.length === 0) return;
    var isMachine = stat[0].hasOwnProperty('machine_no');
    var labels = stat.map(function (d) { return isMachine ? (d.machine_no || 'Unknown') : (d.line_name || 'Unknown'); });

    charts.dtLine.data.labels = labels;
    charts.dtLine.data.datasets[0].data = stat.map(function (d) { return parseInt(d.warning_count) || 0; });
    charts.dtLine.data.datasets[1].data = stat.map(function (d) { return parseInt(d.completed_count) || 0; });
    charts.dtLine.options.scales.x.title.text = isMachine ? 'Machine Number' : 'Line Name';

    var titleEl    = document.getElementById('dtLineChartTitle');
    var subtitleEl = document.getElementById('dtLineChartSubtitle');
    if (titleEl)    titleEl.textContent    = isMachine ? 'Machine Downtime' : 'Line Downtime';
    if (subtitleEl) subtitleEl.textContent = isMachine ? 'Downtime comparison by machine' : 'Downtime comparison by line';

    charts.dtLine.update('none');
}

function updateDtLineChartFromData(list) {
    if (!charts.dtLine || !list || list.length === 0) return;
    var grp = {};
    list.forEach(function (d) {
        var key = d.line_name || 'Unknown';
        if (!grp[key]) grp[key] = { warn: 0, comp: 0 };
        if (d.status === 'Warning') grp[key].warn++;
        else grp[key].comp++;
    });
    var labels = Object.keys(grp);
    charts.dtLine.data.labels = labels;
    charts.dtLine.data.datasets[0].data = labels.map(function (l) { return grp[l].warn; });
    charts.dtLine.data.datasets[1].data = labels.map(function (l) { return grp[l].comp; });
    charts.dtLine.options.scales.x.title.text = 'Line Name';
    charts.dtLine.update('none');
}

function updateDtDurationChartFromData(list) {
    if (!charts.dtDuration || !list || list.length === 0) return;
    var buckets = [0, 0, 0, 0, 0]; // <5, 5-15, 15-30, 30-60, >60
    list.forEach(function (d) {
        var min = parseDurationToMinutes(d.duration_his || d.duration_display);
        if (min === null) return;
        if (min < 5)       buckets[0]++;
        else if (min < 15) buckets[1]++;
        else if (min < 30) buckets[2]++;
        else if (min < 60) buckets[3]++;
        else               buckets[4]++;
    });
    charts.dtDuration.data.datasets[0].data = buckets;
    charts.dtDuration.update('none');
}

function parseDurationToMinutes(str) {
    if (!str) return null;
    // "1h 23m" 또는 "23m" 또는 숫자 형식 처리
    var total = 0;
    var hMatch = String(str).match(/(\d+)\s*h/);
    var mMatch = String(str).match(/(\d+)\s*m/);
    if (hMatch) total += parseInt(hMatch[1]) * 60;
    if (mMatch) total += parseInt(mMatch[1]);
    if (!hMatch && !mMatch) { var n = parseFloat(str); if (!isNaN(n)) total = n; else return null; }
    return total;
}

// ─── Elapsed Time Timer ────────────────────────────────────
function calculateElapsedTime(startDateStr) {
    if (!startDateStr) return '-';
    var start = new Date(startDateStr.replace(' ', 'T'));
    if (isNaN(start.getTime())) return '-';
    var diff = Math.floor((Date.now() - start.getTime()) / 1000);
    if (diff < 0) diff = 0;
    var h = Math.floor(diff / 3600);
    var m = Math.floor((diff % 3600) / 60);
    var s = diff % 60;
    if (h > 0) return h + 'h ' + String(m).padStart(2, '0') + 'm ' + String(s).padStart(2, '0') + 's';
    return String(m).padStart(2, '0') + 'm ' + String(s).padStart(2, '0') + 's';
}

function startDurationUpdateTimer() {
    stopDurationUpdateTimer();
    durationUpdateTimer = setInterval(function () {
        document.querySelectorAll('.duration-in-progress[data-start-time]').forEach(function (cell) {
            cell.textContent = calculateElapsedTime(cell.getAttribute('data-start-time'));
        });
    }, 1000);
}

function stopDurationUpdateTimer() {
    if (durationUpdateTimer) { clearInterval(durationUpdateTimer); durationUpdateTimer = null; }
}

function stopElapsedTimeTimer() {
    if (elapsedTimeTimer) { clearInterval(elapsedTimeTimer); elapsedTimeTimer = null; }
}

// ─── 시간 범위 핸들러 ─────────────────────────────────────
async function handleDateRangeChange(start, end) {
    var sel = document.getElementById('timeRangeSelect');
    if (sel) {
        var opt = sel.querySelector('option[value="custom"]');
        if (!opt) { opt = document.createElement('option'); opt.value = 'custom'; opt.textContent = 'Custom'; sel.appendChild(opt); }
        sel.value = 'custom';
    }
    if (isTracking) { stopTracking(); setTimeout(async function () { await startAutoTracking(); }, 1000); }
    else await startAutoTracking();
}

async function handleTimeRangeChange(e) {
    var v = e.target.value;
    var s, en;
    switch (v) {
        case 'today':     s = moment().startOf('day'); en = moment().endOf('day'); break;
        case 'yesterday': s = moment().subtract(1,'days').startOf('day'); en = moment().subtract(1,'days').endOf('day'); break;
        case '1w':        s = moment().subtract(7,'days').startOf('day'); en = moment().endOf('day'); break;
        case '1m':        s = moment().subtract(30,'days').startOf('day'); en = moment().endOf('day'); break;
        default:          s = moment().startOf('day'); en = moment().endOf('day');
    }
    if (v !== 'custom') {
        $('#dateRangePicker').data('daterangepicker').setStartDate(s);
        $('#dateRangePicker').data('daterangepicker').setEndDate(en);
        $('#dateRangePicker').val(s.format('YYYY-MM-DD') + ' ~ ' + en.format('YYYY-MM-DD'));
    }
    await restartRealTimeMonitoring();
}

// ─── 내보내기 / 새로고침 ──────────────────────────────────
function exportData() {
    if (!downtimeData || downtimeData.length === 0) return;
    var f = getFilterParams();
    var p = new URLSearchParams();
    if (f.factory_filter) p.append('factory_filter', f.factory_filter);
    if (f.line_filter)    p.append('line_filter',    f.line_filter);
    if (f.machine_filter) p.append('machine_filter', f.machine_filter);
    if (f.shift_filter)   p.append('shift_filter',   f.shift_filter);
    if (f.start_date)     p.append('start_date',     f.start_date);
    if (f.end_date)       p.append('end_date',       f.end_date);
    window.open('proc/data_downtime_export_2.php?' + p.toString(), '_blank');
}

async function refreshData() {
    if (isTracking) { stopTracking(); setTimeout(async function () { await startAutoTracking(); }, 1000); }
    else await startAutoTracking();
}

// ─── Modal ────────────────────────────────────────────────
function openDowntimeDetailModal(btn) {
    try {
        var data = JSON.parse(btn.getAttribute('data-dt-data'));
        populateDowntimeModal(data);
        var modal = document.getElementById('downtimeDetailModal');
        if (modal) { modal.classList.add('show'); document.body.style.overflow = 'hidden'; }
    } catch (e) { console.error('Modal open error:', e); }
}

function closeDowntimeDetailModal() {
    var modal = document.getElementById('downtimeDetailModal');
    if (!modal) return;
    modal.classList.add('hide');
    setTimeout(function () { modal.classList.remove('show', 'hide'); document.body.style.overflow = ''; }, 300);
}

function populateDowntimeModal(d) {
    var set = function (id, v) { var el = document.getElementById(id); if (el) el.textContent = v; };
    set('modal-machine-no',    d.machine_no   || '-');
    set('modal-factory-line',  (d.factory_name || '-') + ' / ' + (d.line_name || '-'));
    set('modal-downtime-type', d.downtime_name || '-');
    set('modal-status',        d.status        || '-');
    set('modal-reg-date',      d.reg_date      || '-');
    set('modal-update-date',   d.update_date   || '-');
    set('modal-duration',      d.duration_display || d.duration_his || '-');
    set('modal-work-date',     d.work_date     || '-');
    set('modal-shift',         d.shift_idx ? 'Shift ' + d.shift_idx : '-');
    set('modal-idx',           d.idx           || '-');
    set('modal-created-at',    d.reg_date      || '-');
    set('modal-color-value',   d.downtime_color || 'Default Color');

    var colorIndicator = document.getElementById('modal-color-indicator');
    if (colorIndicator && d.downtime_color) {
        colorIndicator.style.background = d.downtime_color;
    }
}

function exportSingleDowntime() {
    closeDowntimeDetailModal();
}
