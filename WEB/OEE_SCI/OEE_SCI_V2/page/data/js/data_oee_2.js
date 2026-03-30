// data_oee_2.js — Row Grid Layout version (ai_dashboard_5 model)

let eventSource = null;
let isTracking = false;
let charts = {};
let oeeData = [];
let reconnectAttempts = 0;
let maxReconnectAttempts = 3;
let stats = {};
let isPageUnloading = false;

let currentPage = 1;
let itemsPerPage = 10;
let totalItems = 0;

Chart.defaults.color = '#1a1a1a';
Chart.defaults.borderColor = '#e8eaed';

const chartColors = {
    oee_overall:  '#0070f2',
    availability: '#30914c',
    performance:  '#1e88e5',
    quality:      '#e26b0a',
    target:       '#da1e28',
    production:   '#7c3aed',
    excellent:    '#30914c',
    good:         '#0070f2',
    fair:         '#e26b0a',
    poor:         '#da1e28'
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
window.addEventListener('beforeunload', () => {
    isPageUnloading = true;
    if (eventSource) { eventSource.close(); eventSource = null; isTracking = false; }
});

window.addEventListener('pagehide', () => {
    isPageUnloading = true;
    if (eventSource) { eventSource.close(); eventSource = null; isTracking = false; }
});

document.addEventListener('visibilitychange', () => {
    if (document.hidden) {
        if (eventSource && isTracking) { eventSource.close(); eventSource = null; isTracking = false; }
    } else {
        if (!isPageUnloading && !isTracking && eventSource === null) {
            reconnectAttempts = 0;
            startAutoTracking();
        }
    }
});

window.addEventListener('focus', () => { if (isPageUnloading) isPageUnloading = false; });

// ─── 레이아웃 동적 업데이트 (핵심 함수) ──────────────────
function updateLayout() {
    const main = document.getElementById('oeeSignageMain');
    if (!main) return;

    const rows = [];
    if (!document.getElementById('oeeRowStats').classList.contains('hidden'))        rows.push('auto');
    if (!document.getElementById('oeeRowChartsTop').classList.contains('hidden'))    rows.push('1fr');
    if (!document.getElementById('oeeRowChartsBottom').classList.contains('hidden')) rows.push('1fr');
    if (!document.getElementById('oeeRowTable').classList.contains('hidden'))        rows.push('1fr');
    rows.push('auto'); // pagination

    main.style.gridTemplateRows = rows.join(' ');

    // Chart.js ResizeObserver 트리거 (DOM 레이아웃 반영 후)
    setTimeout(function () {
        Object.values(charts).forEach(function (c) { if (c) c.resize(); });
    }, 50);
}

// ─── 토글 함수 ────────────────────────────────────────────
function toggleStatsDisplay() {
    var row = document.getElementById('oeeRowStats');
    var btn = document.getElementById('toggleStatsBtn');
    if (!row || !btn) return;

    row.classList.toggle('hidden');
    btn.textContent = row.classList.contains('hidden') ? 'Show Stats' : 'Hide Stats';
    updateLayout();
}

function toggleChartsDisplay() {
    var rowTop    = document.getElementById('oeeRowChartsTop');
    var rowBottom = document.getElementById('oeeRowChartsBottom');
    var btn       = document.getElementById('toggleChartsBtn');
    if (!rowTop || !rowBottom || !btn) return;

    var isHidden = rowTop.classList.contains('hidden');
    rowTop.classList.toggle('hidden', !isHidden);
    rowBottom.classList.toggle('hidden', !isHidden);
    btn.textContent = isHidden ? 'Hide Charts' : 'Show Charts';
    updateLayout();
}

function toggleDataDisplay() {
    var rowTable      = document.getElementById('oeeRowTable');
    var rowPagination = document.getElementById('oeeRowPagination');
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
        var res = await fetch('../manage/proc/factory.php?status_filter=Y').then(r => r.json());
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
        var res = await fetch('../manage/proc/line.php?status_filter=Y').then(r => r.json());
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
        var res = await fetch('../manage/proc/machine.php').then(r => r.json());
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
        var res = await fetch(url).then(r => r.json());
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
        var res = await fetch(url).then(r => r.json());
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
    ['overallOee', 'availability', 'performance', 'quality', 'currentShiftOee', 'previousDayOee'].forEach(function (id) {
        var el = document.getElementById(id);
        if (el) el.textContent = '-';
    });
    ['runtime', 'plannedTime', 'availabilityDetail', 'actualOutput', 'theoreticalOutput',
        'performanceDetail', 'goodProducts', 'defectiveProducts', 'qualityDetail'].forEach(function (id) {
        var el = document.getElementById(id);
        if (el) el.textContent = '-';
    });
    var tbody = document.getElementById('oeeDataBody');
    if (tbody) tbody.innerHTML = '<tr><td colspan="10" class="data-table-centered"><div class="fiori-alert fiori-alert--info"><strong>Information:</strong> Loading real-time OEE data.</div></td></tr>';
}

// ─── 차트 초기화 ──────────────────────────────────────────
function initCharts() {
    charts.oeeTrend       = createOeeTrendChart();
    charts.oeeComponent   = createOeeComponentChart();
    charts.productionTrend = createProductionTrendChart();
    charts.machineOee     = createMachineOeeChart();
    charts.oeeGrade       = createOeeGradeChart();
}

function createOeeTrendChart() {
    var ctx = document.getElementById('oeeTrendChart').getContext('2d');
    return new Chart(ctx, {
        type: 'line',
        data: {
            labels: [],
            datasets: [{
                label: 'OEE %',
                data: [],
                borderColor: chartColors.oee_overall,
                backgroundColor: chartColors.oee_overall + '20',
                fill: true,
                tension: 0.4,
                pointRadius: 4,
                pointHoverRadius: 6
            }]
        },
        options: {
            responsive: true,
            maintainAspectRatio: false,
            plugins: { legend: { display: true, position: 'top' }, tooltip: { mode: 'index', intersect: false } },
            scales: {
                y: { beginAtZero: true, title: { display: true, text: 'OEE (%)' }, ticks: { callback: function (v) { return v + '%'; } } },
                x: { title: { display: true, text: 'Time/Period' } }
            }
        }
    });
}

function createOeeComponentChart() {
    var ctx = document.getElementById('oeeComponentChart').getContext('2d');
    return new Chart(ctx, {
        type: 'radar',
        data: {
            labels: ['Availability', 'Performance', 'Quality'],
            datasets: [
                { label: 'Current OEE', data: [0, 0, 0], borderColor: chartColors.oee_overall, backgroundColor: chartColors.oee_overall + '30', pointRadius: 5 },
                { label: 'Target', data: [100, 100, 100], borderColor: chartColors.target, backgroundColor: chartColors.target + '20', pointRadius: 4, borderDash: [5, 5] }
            ]
        },
        options: {
            responsive: true,
            maintainAspectRatio: false,
            scales: { r: { beginAtZero: true, max: 100, ticks: { callback: function (v) { return v + '%'; } } } },
            plugins: { legend: { display: true, position: 'top' } }
        }
    });
}

function createProductionTrendChart() {
    var ctx = document.getElementById('productionTrendChart').getContext('2d');
    return new Chart(ctx, {
        type: 'bar',
        data: {
            labels: [],
            datasets: [
                { label: 'Actual Output', data: [], backgroundColor: chartColors.production + '80', borderColor: chartColors.production, borderWidth: 1, yAxisID: 'y' },
                { label: 'Target Output', data: [], type: 'line', borderColor: chartColors.target, backgroundColor: 'transparent', borderWidth: 2, pointRadius: 3, yAxisID: 'y', borderDash: [5, 5] }
            ]
        },
        options: {
            responsive: true,
            maintainAspectRatio: false,
            plugins: { legend: { display: true, position: 'top' } },
            scales: {
                y: { beginAtZero: true, title: { display: true, text: 'Production Count' } },
                x: { title: { display: true, text: 'Time Period' } }
            }
        }
    });
}

function createMachineOeeChart() {
    var ctx = document.getElementById('machineOeeChart').getContext('2d');
    return new Chart(ctx, {
        type: 'bar',
        data: {
            labels: [],
            datasets: [
                { label: 'Availability', data: [], backgroundColor: chartColors.availability + '80', borderColor: chartColors.availability, borderWidth: 1 },
                { label: 'Performance',  data: [], backgroundColor: chartColors.performance + '80',  borderColor: chartColors.performance,  borderWidth: 1 },
                { label: 'Quality',      data: [], backgroundColor: chartColors.quality + '80',      borderColor: chartColors.quality,      borderWidth: 1 }
            ]
        },
        options: {
            responsive: true,
            maintainAspectRatio: false,
            plugins: { legend: { display: true, position: 'top' }, tooltip: { callbacks: { label: function (c) { return c.dataset.label + ': ' + c.parsed.y + '%'; } } } },
            scales: {
                y: { beginAtZero: true, suggestedMax: 100, title: { display: true, text: 'Performance (%)' }, ticks: { callback: function (v) { return v + '%'; } } },
                x: { title: { display: true, text: 'Line Name' } }
            }
        }
    });
}

function createOeeGradeChart() {
    var ctx = document.getElementById('oeeGradeChart').getContext('2d');
    return new Chart(ctx, {
        type: 'doughnut',
        data: {
            labels: [' >=85%', ' 70-84%', ' 50-69%', ' <50%'],
            datasets: [{
                label: 'OEE Grade',
                data: [0, 0, 0, 0],
                backgroundColor: [chartColors.excellent + 'CC', chartColors.good + 'CC', chartColors.fair + 'CC', chartColors.poor + 'CC'],
                borderColor:     [chartColors.excellent, chartColors.good, chartColors.fair, chartColors.poor],
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
    eventSource = new EventSource('proc/data_oee_stream_2.php?' + params.toString());
    setupSSEEventListeners();
    isTracking = true;
    var el = document.getElementById('connectionStatus');
    if (el) el.textContent = 'OEE System Real-time Connected';
}

function setupSSEEventListeners() {
    eventSource.onerror = function () {
        isTracking = false;
        attemptReconnection();
    };

    eventSource.addEventListener('connected', function (e) {
        reconnectAttempts = 0;
        var el = document.getElementById('connectionStatus');
        if (el) el.textContent = 'OEE system connected';
    });

    eventSource.addEventListener('oee_data', function (e) {
        var data = JSON.parse(e.data);
        stats   = data.stats;
        oeeData = data.oee_data;
        window.oeeData = data.oee_data;
        updateStatCardsFromAPI(stats);
        updateOeeDetailsFromAPI(data.oee_details);
        updateTableFromAPI(oeeData);
        updateChartsFromAPI(data);
        var el = document.getElementById('lastUpdateTime');
        if (el) el.textContent = 'Last updated: ' + data.timestamp;
    });

    eventSource.addEventListener('heartbeat', function (e) {
        var data = JSON.parse(e.data);
        var el = document.getElementById('connectionStatus');
        if (el) el.textContent = 'Connection maintained (Active machines: ' + data.active_machines + ')';
    });

    eventSource.addEventListener('disconnected', function () {
        var el = document.getElementById('connectionStatus');
        if (el) el.textContent = 'OEE System Connection Closed';
    });
}

function stopTracking() {
    if (!isTracking) return;
    if (eventSource) { eventSource.close(); eventSource = null; }
    isTracking = false;
    var el = document.getElementById('connectionStatus');
    if (el) el.textContent = 'OEE System Connection Closed';
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
function formatPercentage(value) {
    var n = parseFloat(value);
    if (isNaN(n)) return '0%';
    return (n % 1 === 0 ? Math.floor(n) : n) + '%';
}

function formatDecimal(value, decimals) {
    var n = parseFloat(value);
    return isNaN(n) ? '-' : n.toFixed(decimals || 2);
}

function updateStatCardsFromAPI(s) {
    if (!s) return;
    var map = {
        overallOee:      formatPercentage(s.overall_oee || 0),
        availability:    formatPercentage(s.availability || 0),
        performance:     formatPercentage(s.performance || 0),
        quality:         formatPercentage(s.quality || 0),
        currentShiftOee: formatPercentage(s.current_shift_oee || 0),
        previousDayOee:  formatPercentage(s.previous_day_oee || 0)
    };
    Object.keys(map).forEach(function (id) {
        var el = document.getElementById(id);
        if (el) el.textContent = map[id];
    });
}

function updateOeeDetailsFromAPI(d) {
    if (!d) return;
    var map = {
        oeeRateDetail:      formatPercentage(d.overall_oee || 0),
        overallEfficiency:  formatPercentage(d.overall_oee || 0),
        targetAchievement:  d.target_achievement || '-',
        runtime:            d.runtime || '-',
        plannedTime:        d.planned_time || '-',
        availabilityDetail: formatPercentage(d.availability || 0),
        actualOutput:       d.actual_output || '-',
        theoreticalOutput:  d.theoretical_output ? formatDecimal(d.theoretical_output) : '-',
        performanceDetail:  formatPercentage(d.performance || 0),
        goodProducts:       d.good_products || '-',
        defectiveProducts:  d.defective_products || '-',
        qualityDetail:      formatPercentage(d.quality || 0)
    };
    Object.keys(map).forEach(function (id) {
        var el = document.getElementById(id);
        if (el) el.textContent = map[id];
    });
}

function updateTableFromAPI(list) {
    var tbody = document.getElementById('oeeDataBody');
    if (!tbody) return;
    totalItems = list.length;

    if (list.length === 0) {
        tbody.innerHTML = '<tr><td colspan="10" class="data-table-centered"><div class="fiori-alert fiori-alert--info"><strong>Information:</strong> No OEE data matching the selected conditions.</div></td></tr>';
        renderPagination();
        return;
    }

    var start = (currentPage - 1) * itemsPerPage;
    var paged = list.slice(start, start + itemsPerPage);
    tbody.innerHTML = '';
    paged.forEach(function (oee) {
        var cls = 'fiori-badge';
        if (parseFloat(oee.overall_oee) >= 85) cls += ' fiori-badge--success';
        else if (parseFloat(oee.overall_oee) >= 70) cls += ' fiori-badge--warning';
        else cls += ' fiori-badge--error';

        var shift = oee.shift_idx ? 'Shift ' + oee.shift_idx : '-';
        var tr = document.createElement('tr');
        tr.innerHTML =
            '<td>' + (oee.machine_no || '-') + '</td>' +
            '<td>' + (oee.factory_name || '') + ' / ' + (oee.line_name || '') + '</td>' +
            '<td>' + shift + '</td>' +
            '<td><span class="' + cls + '">' + (oee.overall_oee || 0) + '%</span></td>' +
            '<td><span class="fiori-badge fiori-badge--success">' + (oee.availability || 0) + '%</span></td>' +
            '<td><span class="fiori-badge fiori-badge--info">' + (oee.performance || 0) + '%</span></td>' +
            '<td><span class="fiori-badge fiori-badge--warning">' + (oee.quality || 0) + '%</span></td>' +
            '<td>' + (oee.work_date || '-') + '</td>' +
            '<td>' + (oee.update_date || '-') + '</td>' +
            '<td><button class="fiori-btn fiori-btn--tertiary oee-details-btn" style="padding:.25rem .5rem;font-size:.75rem;" data-oee-data=\'' + JSON.stringify(oee).replace(/'/g, "&#39;") + '\'>Details</button></td>';
        tbody.appendChild(tr);
    });

    renderPagination();
    setupDetailsButtonListeners();
}

function setupDetailsButtonListeners() {
    document.querySelectorAll('.oee-details-btn').forEach(function (btn) {
        btn.removeEventListener('click', handleDetailsButtonClick);
        btn.addEventListener('click', handleDetailsButtonClick);
    });
}

function handleDetailsButtonClick(e) {
    e.preventDefault();
    e.stopPropagation();
    var btn = e.target.closest('.oee-details-btn');
    if (btn) openOeeDetailModal(btn);
}

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
    updateTableFromAPI(oeeData);
}

// ─── 차트 업데이트 ────────────────────────────────────────
function updateChartsFromAPI(data) {
    if (data.oee_trend_stats && charts.oeeTrend)         updateOeeTrendChart(data.oee_trend_stats);
    if (data.oee_component_stats && charts.oeeComponent) updateOeeComponentChart(data.oee_component_stats);
    if (data.production_trend_stats && charts.productionTrend) updateProductionTrendChart(data.production_trend_stats);
    if (data.machine_oee_stats && charts.machineOee)     updateMachineOeeChart(data.machine_oee_stats);
    if (data.stats && charts.oeeGrade)                   updateOeeGradeChart(data.stats);
}

function updateOeeTrendChart(stat) {
    if (!charts.oeeTrend) return;
    if (!stat || !stat.data || stat.data.length === 0) {
        if (window.oeeData && window.oeeData.length > 0) { generateSimpleTrendFromOeeData(); return; }
        charts.oeeTrend.data.labels = ['No Data'];
        charts.oeeTrend.data.datasets[0].data = [0];
        charts.oeeTrend.update('none');
        return;
    }
    var trendData = stat.data;
    var workHours = stat.work_hours;
    var viewType  = stat.view_type || 'hourly';
    var labels = [];

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

    var valMap = {};
    trendData.forEach(function (d) {
        var key = viewType === 'daily' ? (d.display_label || d.time_label) : (function () { var m = (d.time_label || '').match(/^(\d{4}-\d{2}-\d{2})\s+(\d{2}):/); return m ? m[2] + 'H' : ''; })();
        if (key) valMap[key] = parseFloat(d.overall_oee) || 0;
    });

    charts.oeeTrend.data.labels = labels;
    charts.oeeTrend.data.datasets[0].data = labels.map(function (l) { return valMap[l] || 0; });
    charts.oeeTrend.options.scales.x.title.text = viewType === 'hourly' ? 'Time (within 1 day)' : 'Date';
    charts.oeeTrend.update('show');
}

function generateWorkHoursLabels(startTime, endTime, startMinutes, endMinutes) {
    var labels = [];
    var sh = startMinutes !== null && startMinutes !== undefined ? Math.floor(startMinutes / 60) : parseInt(startTime.split(':')[0]);
    var eh = endMinutes   !== null && endMinutes   !== undefined ? Math.floor(endMinutes / 60)   : parseInt(endTime.split(':')[0]);
    if (eh < sh) eh += 24;
    for (var h = sh; h <= eh; h++) labels.push(String(h % 24).padStart(2, '0') + 'H');
    return labels;
}

function generateSimpleTrendFromOeeData() {
    if (!charts.oeeTrend || !window.oeeData || window.oeeData.length === 0) return;
    var grp = {};
    window.oeeData.forEach(function (d) {
        if (!d.work_date) return;
        if (!grp[d.work_date]) grp[d.work_date] = { sum: 0, cnt: 0 };
        grp[d.work_date].sum += parseFloat(d.overall_oee) || 0;
        grp[d.work_date].cnt++;
    });
    var dates = Object.keys(grp).sort();
    charts.oeeTrend.data.labels = dates.map(function (d) { var x = new Date(d); return (x.getMonth() + 1) + '/' + x.getDate(); });
    charts.oeeTrend.data.datasets[0].data = dates.map(function (d) { return grp[d].cnt > 0 ? grp[d].sum / grp[d].cnt : 0; });
    charts.oeeTrend.options.scales.x.title.text = 'Date';
    charts.oeeTrend.update('show');
}

function updateOeeComponentChart(stat) {
    if (!charts.oeeComponent || !stat) return;
    charts.oeeComponent.data.datasets[0].data = [parseFloat(stat.availability) || 0, parseFloat(stat.performance) || 0, parseFloat(stat.quality) || 0];
    charts.oeeComponent.update('none');
}

function updateProductionTrendChart(stat) {
    if (!charts.productionTrend) return;
    if (!stat || !stat.data || stat.data.length === 0) {
        charts.productionTrend.data.labels = ['No Data'];
        charts.productionTrend.data.datasets[0].data = [0];
        charts.productionTrend.data.datasets[1].data = [0];
        charts.productionTrend.update('none');
        return;
    }
    var trendData = stat.data;
    var workHours = stat.work_hours;
    var viewType  = stat.view_type || 'hourly';
    var labels = [];

    if (viewType === 'hourly' && workHours && workHours.start_time) {
        labels = generateWorkHoursLabels(workHours.start_time, workHours.end_time, workHours.start_minutes, workHours.end_minutes);
    } else if (viewType === 'daily') {
        labels = trendData.map(function (d) { return d.display_label || d.time_label; });
    } else {
        labels = trendData.map(function (d) { var m = (d.time_label || '').match(/^(\d{4}-\d{2}-\d{2})\s+(\d{2}):/); return m ? m[2] + 'H' : d.time_label; });
    }

    var actMap = {}, tgtMap = {};
    trendData.forEach(function (d) {
        var key = viewType === 'daily' ? (d.display_label || d.time_label) : (function () { var m = (d.time_label || '').match(/^(\d{4}-\d{2}-\d{2})\s+(\d{2}):/); return m ? m[2] + 'H' : ''; })();
        if (key) { actMap[key] = parseInt(d.actual_output) || 0; tgtMap[key] = parseInt(d.target_output) || 0; }
    });

    charts.productionTrend.data.labels = labels;
    charts.productionTrend.data.datasets[0].data = labels.map(function (l) { return actMap[l] || 0; });
    charts.productionTrend.data.datasets[1].data = labels.map(function (l) { return tgtMap[l] || 0; });
    charts.productionTrend.options.scales.x.title.text = viewType === 'hourly' ? 'Time (within 1 day)' : 'Date';
    charts.productionTrend.update('show');
}

function updateMachineOeeChart(stat) {
    if (!charts.machineOee || !stat || !Array.isArray(stat) || stat.length === 0) return;
    var isMachine = stat[0].hasOwnProperty('machine_no');
    var labels = stat.map(function (d) { return isMachine ? (d.machine_no || 'Unknown') : (d.line_name || 'Unknown'); });

    charts.machineOee.data.labels = labels;
    charts.machineOee.data.datasets[0].data = stat.map(function (d) { return parseFloat(d.availability) || 0; });
    charts.machineOee.data.datasets[1].data = stat.map(function (d) { return parseFloat(d.performance)  || 0; });
    charts.machineOee.data.datasets[2].data = stat.map(function (d) { return parseFloat(d.quality)      || 0; });
    charts.machineOee.options.scales.x.title.text = isMachine ? 'Machine Number' : 'Line Name';

    var titleEl    = document.getElementById('oeeLineOeeCardTitle');
    var subtitleEl = document.getElementById('oeeLineOeeCardSubtitle');
    if (titleEl)    titleEl.textContent    = isMachine ? 'Machine OEE Performance' : 'Line OEE Performance';
    if (subtitleEl) subtitleEl.textContent = isMachine ? 'OEE performance comparison by machine' : 'OEE performance comparison by production line';

    charts.machineOee.update('none');
}

function updateOeeGradeChart(s) {
    if (!charts.oeeGrade || !s) return;
    charts.oeeGrade.data.datasets[0].data = [
        parseInt(s.excellent_count) || 0,
        parseInt(s.good_count)      || 0,
        parseInt(s.fair_count)      || 0,
        parseInt(s.poor_count)      || 0
    ];
    charts.oeeGrade.update('none');
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
    if (!oeeData || oeeData.length === 0) return;
    var f = getFilterParams();
    var p = new URLSearchParams();
    if (f.factory_filter) p.append('factory_filter', f.factory_filter);
    if (f.line_filter)    p.append('line_filter',    f.line_filter);
    if (f.machine_filter) p.append('machine_filter', f.machine_filter);
    if (f.shift_filter)   p.append('shift_filter',   f.shift_filter);
    if (f.start_date)     p.append('start_date',     f.start_date);
    if (f.end_date)       p.append('end_date',       f.end_date);
    window.open('proc/data_oee_export_2.php?' + p.toString(), '_blank');
}

async function refreshData() {
    if (isTracking) { stopTracking(); setTimeout(async function () { await startAutoTracking(); }, 1000); }
    else await startAutoTracking();
}

// ─── Modal ────────────────────────────────────────────────
function openOeeDetailModal(btn) {
    try {
        var data = JSON.parse(btn.getAttribute('data-oee-data'));
        populateOeeModal(data);
        var modal = document.getElementById('oeeDetailModal');
        if (modal) { modal.classList.add('show'); document.body.style.overflow = 'hidden'; }
    } catch (e) { console.error('Modal open error:', e); }
}

function closeOeeDetailModal() {
    var modal = document.getElementById('oeeDetailModal');
    if (!modal) return;
    modal.classList.add('hide');
    setTimeout(function () { modal.classList.remove('show', 'hide'); document.body.style.overflow = ''; }, 300);
}

function populateOeeModal(d) {
    var set = function (id, v) { var el = document.getElementById(id); if (el) el.textContent = v; };
    set('modal-machine-no',        d.machine_no || '-');
    set('modal-factory-line',      (d.factory_name || '-') + ' / ' + (d.line_name || '-'));
    set('modal-work-date',         d.work_date || '-');
    set('modal-shift',             d.shift_idx ? 'Shift ' + d.shift_idx : '-');
    set('modal-overall-oee',       formatPercentage(d.overall_oee || 0));
    set('modal-availability',      formatPercentage(d.availability || 0));
    set('modal-performance',       formatPercentage(d.performance || 0));
    set('modal-quality',           formatPercentage(d.quality || 0));
    set('modal-planned-time',      d.planned_time || '-');
    set('modal-runtime',           d.runtime || '-');
    set('modal-downtime',          d.downtime || '-');
    set('modal-actual-output',     d.actual_output || '-');
    set('modal-theoretical-output', d.theoretical_output ? formatDecimal(d.theoretical_output) : '-');
    set('modal-defective',         d.defective_count || '-');
    set('modal-cycletime',         d.cycle_time || '-');
    set('modal-update-time',       d.update_date || '-');
}

function exportSingleOee() {
    closeOeeDetailModal();
}
