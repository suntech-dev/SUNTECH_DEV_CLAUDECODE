// data_defective_2.js — Row Grid Layout version (ai_dashboard_5 model)

let eventSource = null;
let isTracking = false;
let charts = {};
let defectiveData = [];
let reconnectAttempts = 0;
let maxReconnectAttempts = 3;
let stats = {};
let activeDefectives = [];
let elapsedTimeTimer = null;
let isPageUnloading = false;

let currentPage = 1;
let itemsPerPage = 10;
let totalItems = 0;

Chart.defaults.color = '#1a1a1a';
Chart.defaults.borderColor = '#e8eaed';

const chartColors = {
    warning:   '#e26b0a',
    error:     '#da1e28',
    success:   '#30914c',
    info:      '#7c3aed',
    primary:   '#0070f2',
    secondary: '#1e88e5',
    accent:    '#00d4aa'
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
});

window.addEventListener('pagehide', function () {
    isPageUnloading = true;
    if (eventSource) { eventSource.close(); eventSource = null; isTracking = false; }
    stopElapsedTimeTimer();
});

document.addEventListener('visibilitychange', function () {
    if (document.hidden) {
        if (eventSource && isTracking) { eventSource.close(); eventSource = null; isTracking = false; }
        stopElapsedTimeTimer();
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
    var main = document.getElementById('defectiveSignageMain');
    if (!main) return;

    var rows = [];
    if (!document.getElementById('defectiveRowStats').classList.contains('hidden'))        rows.push('auto');
    if (!document.getElementById('defectiveRowChartsTop').classList.contains('hidden'))    rows.push('1fr');
    if (!document.getElementById('defectiveRowChartsBottom').classList.contains('hidden')) rows.push('1fr');
    if (!document.getElementById('defectiveRowTable').classList.contains('hidden'))        rows.push('1fr');
    rows.push('auto'); // pagination

    main.style.gridTemplateRows = rows.join(' ');

    // Chart.js ResizeObserver 트리거 (DOM 레이아웃 반영 후)
    setTimeout(function () {
        Object.values(charts).forEach(function (c) { if (c) c.resize(); });
    }, 50);
}

// ─── 토글 함수 ────────────────────────────────────────────
function toggleStatsDisplay() {
    var row = document.getElementById('defectiveRowStats');
    var btn = document.getElementById('toggleStatsBtn');
    if (!row || !btn) return;

    row.classList.toggle('hidden');
    btn.textContent = row.classList.contains('hidden') ? 'Show Stats' : 'Hide Stats';
    updateLayout();
}

function toggleChartsDisplay() {
    var rowTop    = document.getElementById('defectiveRowChartsTop');
    var rowBottom = document.getElementById('defectiveRowChartsBottom');
    var btn       = document.getElementById('toggleChartsBtn');
    if (!rowTop || !rowBottom || !btn) return;

    var isHidden = rowTop.classList.contains('hidden');
    rowTop.classList.toggle('hidden', !isHidden);
    rowBottom.classList.toggle('hidden', !isHidden);
    btn.textContent = isHidden ? 'Hide Charts' : 'Show Charts';
    updateLayout();
}

function toggleDataDisplay() {
    var rowTable      = document.getElementById('defectiveRowTable');
    var rowPagination = document.getElementById('defectiveRowPagination');
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
    ['activeDefectives', 'currentShiftDefective', 'affectedMachinesDefective',
     'defectiveRate', 'qualityScore', 'totalDefectiveCount'].forEach(function (id) {
        var el = document.getElementById(id);
        if (el) el.textContent = '-';
    });
    var activeCountEl = document.getElementById('activeDefectiveCount');
    if (activeCountEl) activeCountEl.textContent = '0 active defectives';

    var tbody = document.getElementById('defectiveDataBody');
    if (tbody) tbody.innerHTML = '<tr><td colspan="8" class="data-table-centered"><div class="fiori-alert fiori-alert--info"><strong>Information:</strong> Loading real-time Defective data.</div></td></tr>';
}

// ─── 차트 초기화 ──────────────────────────────────────────
function initCharts() {
    charts.defectiveType   = createDefectiveTypeChart();
    charts.defectiveStatus = createDefectiveStatusChart();
    charts.defectiveTrend  = createDefectiveTrendChart();
    charts.defectiveMachine = createDefectiveMachineChart();
    charts.defectiveLine   = createDefectiveLineChart();
}

function createDefectiveTypeChart() {
    var ctx = document.getElementById('defectiveTypeChart').getContext('2d');
    return new Chart(ctx, {
        type: 'bar',
        data: {
            labels: [],
            datasets: [{
                label: 'Defective Count',
                data: [],
                backgroundColor: [
                    chartColors.warning, chartColors.error, chartColors.primary,
                    chartColors.info, chartColors.accent, chartColors.secondary
                ],
                borderColor: [
                    chartColors.warning, chartColors.error, chartColors.primary,
                    chartColors.info, chartColors.accent, chartColors.secondary
                ],
                borderWidth: 1
            }]
        },
        options: {
            responsive: true,
            maintainAspectRatio: false,
            plugins: { legend: { display: false } },
            scales: {
                y: { beginAtZero: true, title: { display: true, text: 'Count' }, ticks: { stepSize: 1, precision: 0 } },
                x: { title: { display: true, text: 'Defective Type' } }
            }
        }
    });
}

function createDefectiveStatusChart() {
    var ctx = document.getElementById('defectiveStatusChart').getContext('2d');
    return new Chart(ctx, {
        type: 'doughnut',
        data: {
            labels: ['Warning', 'Completed'],
            datasets: [{
                label: 'Defective Status',
                data: [0, 0],
                backgroundColor: [chartColors.error + 'CC', chartColors.success + 'CC'],
                borderColor:     [chartColors.error, chartColors.success],
                borderWidth: 2,
                hoverOffset: 10
            }]
        },
        options: {
            responsive: true,
            maintainAspectRatio: false,
            plugins: {
                legend: { display: true, position: 'bottom', labels: { padding: 16, usePointStyle: true } },
                tooltip: {
                    callbacks: {
                        label: function (c) {
                            var t = c.dataset.data.reduce(function (a, b) { return a + b; }, 0);
                            var p = t > 0 ? ((c.raw / t) * 100).toFixed(1) : 0;
                            return c.label + ': ' + c.raw + ' (' + p + '%)';
                        }
                    }
                }
            }
        }
    });
}

function createDefectiveTrendChart() {
    var ctx = document.getElementById('defectiveTrendChart').getContext('2d');
    return new Chart(ctx, {
        type: 'line',
        data: {
            labels: [],
            datasets: [{
                label: 'Defective Count',
                data: [],
                borderColor: chartColors.error,
                backgroundColor: chartColors.error + '20',
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
                y: { beginAtZero: true, title: { display: true, text: 'Count' }, ticks: { stepSize: 1, precision: 0 } },
                x: { title: { display: true, text: 'Time/Period' } }
            }
        }
    });
}

function createDefectiveMachineChart() {
    var ctx = document.getElementById('defectiveMachineChart').getContext('2d');
    return new Chart(ctx, {
        type: 'bar',
        data: {
            labels: [],
            datasets: [{
                label: 'Defective Count',
                data: [],
                backgroundColor: chartColors.warning + '80',
                borderColor: chartColors.warning,
                borderWidth: 1
            }]
        },
        options: {
            responsive: true,
            maintainAspectRatio: false,
            plugins: { legend: { display: true, position: 'top' } },
            scales: {
                y: { beginAtZero: true, title: { display: true, text: 'Count' }, ticks: { stepSize: 1, precision: 0 } },
                x: { title: { display: true, text: 'Machine Number' } }
            }
        }
    });
}

function createDefectiveLineChart() {
    var ctx = document.getElementById('defectiveLineChart').getContext('2d');
    return new Chart(ctx, {
        type: 'bar',
        data: {
            labels: [],
            datasets: [{
                label: 'Defective Count',
                data: [],
                backgroundColor: chartColors.info + '80',
                borderColor: chartColors.info,
                borderWidth: 1
            }]
        },
        options: {
            responsive: true,
            maintainAspectRatio: false,
            plugins: { legend: { display: true, position: 'top' } },
            scales: {
                y: { beginAtZero: true, title: { display: true, text: 'Count' }, ticks: { stepSize: 1, precision: 0 } },
                x: { title: { display: true, text: 'Line Name' } }
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
    eventSource = new EventSource('proc/data_defective_stream.php?' + params.toString());
    setupSSEEventListeners();
    isTracking = true;
    var el = document.getElementById('connectionStatus');
    if (el) el.textContent = 'Defective System Real-time Connected';
}

function setupSSEEventListeners() {
    eventSource.onerror = function () {
        isTracking = false;
        attemptReconnection();
    };

    eventSource.addEventListener('connected', function (e) {
        reconnectAttempts = 0;
        var el = document.getElementById('connectionStatus');
        if (el) el.textContent = 'Defective system connected';
    });

    eventSource.addEventListener('defective_data', function (e) {
        var data = JSON.parse(e.data);
        stats           = data.stats;
        activeDefectives = data.active_defectives || [];
        defectiveData   = data.defective_data;
        window.defectiveData = data.defective_data;

        updateStatCardsFromAPI(stats);
        updateActiveDefectivesDisplay(activeDefectives);
        updateTableFromAPI(defectiveData);
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
        if (el) el.textContent = 'Defective System Connection Closed';
    });
}

function stopTracking() {
    if (!isTracking) return;
    if (eventSource) { eventSource.close(); eventSource = null; }
    stopElapsedTimeTimer();
    isTracking = false;
    var el = document.getElementById('connectionStatus');
    if (el) el.textContent = 'Defective System Connection Closed';
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
function formatRateNumber(value) {
    var n = parseFloat(value);
    if (isNaN(n)) return '-';
    return (Number.isInteger(n) ? n.toString() : n.toFixed(1));
}

function updateStatCardsFromAPI(s) {
    if (!s) return;

    var defectiveRateDisplay = '-';
    if (s.defective_rate !== null && s.defective_rate !== undefined) {
        defectiveRateDisplay = formatRateNumber(s.defective_rate) + '%';
    }

    var qualityScoreDisplay = '-';
    if (s.quality_score !== null && s.quality_score !== undefined) {
        qualityScoreDisplay = formatRateNumber(s.quality_score) + '%';
    }

    var map = {
        activeDefectives:         s.warning_count       || '0',
        currentShiftDefective:    s.current_shift_count || '0',
        affectedMachinesDefective: s.affected_machines  || '0',
        defectiveRate:            defectiveRateDisplay,
        qualityScore:             qualityScoreDisplay,
        totalDefectiveCount:      s.total_count         || '0'
    };

    Object.keys(map).forEach(function (id) {
        var el = document.getElementById(id);
        if (el) el.textContent = map[id];
    });
}

function updateActiveDefectivesDisplay(list) {
    var container  = document.getElementById('activeDefectivesContainer');
    var countEl    = document.getElementById('activeDefectiveCount');

    if (countEl) countEl.textContent = list.length + ' active defectives';
    if (!container) return;

    if (list.length === 0) {
        container.innerHTML = '<div class="fiori-alert fiori-alert--success"><strong>Good:</strong> No active defectives. All systems operating normally.</div>';
        stopElapsedTimeTimer();
        return;
    }

    var sorted = list.slice().sort(function (a, b) {
        return new Date(b.reg_date) - new Date(a.reg_date);
    }).slice(0, 5);

    var html = '';
    sorted.forEach(function (d) {
        var borderColor = d.defective_color || '#0070f2';
        var bgColor     = borderColor + '1A';
        var elapsed     = calculateElapsedTime(d.reg_date);
        html +=
            '<div class="active-defective-item" style="border: 2px dashed ' + borderColor + '; background-color: ' + bgColor + ';">' +
                '<div class="defective-machine-info">' +
                    '<div class="defective-machine-name">' + (d.defective_name || '-') + '</div>' +
                    '<div class="defective-location">' +
                        '<span class="defective-location-item">' + (d.factory_name || '-') + '</span>' +
                        '<span class="defective-location-item"> / </span>' +
                        '<span class="defective-location-item">' + (d.line_name || '-') + '</span>' +
                        '<span class="defective-location-item"> / </span>' +
                        '<span class="defective-location-item">' + (d.machine_no || '-') + '</span>' +
                    '</div>' +
                '</div>' +
                '<div class="defective-elapsed-time"><span>' + elapsed + '</span></div>' +
            '</div>';
    });

    if (list.length > 5) {
        html += '<div class="fiori-alert fiori-alert--info" style="margin-top:4px;"><strong>Info:</strong> Showing 5 most recent out of ' + list.length + ' total active defectives.</div>';
    }

    container.innerHTML = html;
    startElapsedTimeTimer();
}

function updateTableFromAPI(list) {
    var tbody = document.getElementById('defectiveDataBody');
    if (!tbody) return;
    totalItems = list.length;

    if (list.length === 0) {
        tbody.innerHTML = '<tr><td colspan="8" class="data-table-centered"><div class="fiori-alert fiori-alert--info"><strong>Information:</strong> No Defective data matching the selected conditions.</div></td></tr>';
        renderPagination();
        return;
    }

    var start = (currentPage - 1) * itemsPerPage;
    var paged = list.slice(start, start + itemsPerPage);
    tbody.innerHTML = '';
    paged.forEach(function (d) {
        var shift   = d.shift_idx ? 'Shift ' + d.shift_idx : '-';
        var elapsed = d.status === 'Warning'
            ? calculateElapsedTime(d.reg_date)
            : (d.duration_display || d.duration_his || '-');

        var tr = document.createElement('tr');
        tr.innerHTML =
            '<td>' + (d.machine_no || '-') + '</td>' +
            '<td>' + (d.factory_name || '') + ' / ' + (d.line_name || '') + '</td>' +
            '<td>' + shift + '</td>' +
            '<td>' + (d.defective_name || '-') + '</td>' +
            '<td>' + (d.reg_date || '-') + '</td>' +
            '<td class="defective-elapsed-time-cell"' +
                ' data-status="' + (d.status || '') + '"' +
                ' data-reg-date="' + (d.reg_date || '') + '"' +
                ' data-duration="' + (d.duration_display || d.duration_his || '-') + '">' + elapsed + '</td>' +
            '<td>' + (d.work_date || '-') + '</td>' +
            '<td><button class="fiori-btn fiori-btn--tertiary defective-details-btn" style="padding:.25rem .5rem;font-size:.75rem;" data-defective-data=\'' + JSON.stringify(d).replace(/'/g, "&#39;") + '\'>Details</button></td>';
        tbody.appendChild(tr);
    });

    renderPagination();
    setupDetailsButtonListeners();

    var hasWarning = paged.some(function (d) { return d.status === 'Warning'; });
    if (hasWarning && !elapsedTimeTimer) startElapsedTimeTimer();
}

function setupDetailsButtonListeners() {
    document.querySelectorAll('.defective-details-btn').forEach(function (btn) {
        btn.removeEventListener('click', handleDetailsButtonClick);
        btn.addEventListener('click', handleDetailsButtonClick);
    });
}

function handleDetailsButtonClick(e) {
    e.preventDefault();
    e.stopPropagation();
    var btn = e.target.closest('.defective-details-btn');
    if (btn) openDefectiveDetailModal(btn);
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
    updateTableFromAPI(defectiveData);
}

// ─── 차트 업데이트 ────────────────────────────────────────
function updateChartsFromAPI(data) {
    if (data.defective_type_stats && charts.defectiveType)    updateDefectiveTypeChart(data.defective_type_stats);
    if (data.stats && charts.defectiveStatus)                 updateDefectiveStatusChart(data.stats);
    if (data.defective_rate_stats && charts.defectiveTrend)   updateDefectiveTrendChart(data.defective_rate_stats);
    if (data.machine_defective_stats && charts.defectiveMachine) updateDefectiveMachineChart(data.machine_defective_stats);
    if (data.line_defective_stats && charts.defectiveLine)    updateDefectiveLineChart(data.line_defective_stats);
}

function updateDefectiveTypeChart(stat) {
    if (!charts.defectiveType || !stat || stat.length === 0) return;

    var sorted = stat.slice().sort(function (a, b) {
        return (a.defective_name || '').localeCompare(b.defective_name || '');
    });

    var labels = sorted.map(function (d) { return d.defective_name || 'Unclassified'; });
    var counts = sorted.map(function (d) { return parseInt(d.count) || 0; });
    var colorKeys = ['warning', 'error', 'primary', 'info', 'accent', 'secondary'];
    var bgColors  = labels.map(function (_, i) { return chartColors[colorKeys[i % colorKeys.length]]; });

    charts.defectiveType.data.labels = labels;
    charts.defectiveType.data.datasets[0].data = counts;
    charts.defectiveType.data.datasets[0].backgroundColor = bgColors;
    charts.defectiveType.data.datasets[0].borderColor      = bgColors;
    charts.defectiveType.update('none');
}

function updateDefectiveStatusChart(s) {
    if (!charts.defectiveStatus || !s) return;
    charts.defectiveStatus.data.datasets[0].data = [
        parseInt(s.warning_count)   || 0,
        parseInt(s.completed_count) || 0
    ];
    charts.defectiveStatus.update('none');
}

function updateDefectiveTrendChart(stat) {
    if (!charts.defectiveTrend) return;

    var statsData = stat;
    var workHours = null;
    var viewType  = 'hourly';

    if (stat && typeof stat === 'object' && stat.data) {
        statsData = stat.data;
        workHours = stat.work_hours;
        viewType  = stat.view_type || 'hourly';
    }

    if (!statsData || statsData.length === 0) {
        charts.defectiveTrend.data.labels = ['No Data'];
        charts.defectiveTrend.data.datasets[0].data = [0];
        charts.defectiveTrend.update('none');
        return;
    }

    var labels = [];
    if (viewType === 'hourly' && workHours && workHours.start_time) {
        labels = generateWorkHoursLabels(workHours.start_time, workHours.end_time, workHours.start_minutes, workHours.end_minutes);
    } else if (viewType === 'daily') {
        labels = statsData.map(function (d) { return d.display_label || d.time_label; });
    } else {
        labels = statsData.map(function (d) {
            var lbl = d.time_label || d.display_label;
            var m   = lbl && lbl.match(/^(\d{4}-\d{2}-\d{2})\s+(\d{2}):/);
            return m ? m[2] + 'H' : lbl;
        });
    }

    var countsMap = {};
    statsData.forEach(function (d) {
        var lbl = d.time_label || d.display_label;
        var key;
        if (viewType === 'daily') {
            key = lbl;
        } else {
            var m = lbl && lbl.match(/^(\d{4}-\d{2}-\d{2})\s+(\d{2}):/);
            key = m ? m[2] + 'H' : lbl;
        }
        if (key) countsMap[key] = parseInt(d.defective_count) || 0;
    });

    charts.defectiveTrend.data.labels = labels;
    charts.defectiveTrend.data.datasets[0].data = labels.map(function (l) { return countsMap[l] || 0; });
    charts.defectiveTrend.options.scales.x.title.text = viewType === 'hourly' ? 'Time (within 1 day)' : 'Date';
    charts.defectiveTrend.update('show');
}

function updateDefectiveMachineChart(stat) {
    if (!charts.defectiveMachine || !stat || !Array.isArray(stat) || stat.length === 0) return;
    charts.defectiveMachine.data.labels  = stat.map(function (d) { return d.machine_no || 'Unknown'; });
    charts.defectiveMachine.data.datasets[0].data = stat.map(function (d) { return parseInt(d.defective_count || d.count) || 0; });
    charts.defectiveMachine.update('none');
}

function updateDefectiveLineChart(stat) {
    if (!charts.defectiveLine || !stat || !Array.isArray(stat) || stat.length === 0) return;
    charts.defectiveLine.data.labels = stat.map(function (d) { return d.line_name || 'Unknown'; });
    charts.defectiveLine.data.datasets[0].data = stat.map(function (d) { return parseInt(d.defective_count || d.count) || 0; });
    charts.defectiveLine.update('none');
}

function generateWorkHoursLabels(startTime, endTime, startMinutes, endMinutes) {
    var labels = [];
    var sh = startMinutes !== null && startMinutes !== undefined ? Math.floor(startMinutes / 60) : parseInt(startTime.split(':')[0]);
    var eh = endMinutes   !== null && endMinutes   !== undefined ? Math.floor(endMinutes / 60)   : parseInt(endTime.split(':')[0]);
    if (eh < sh) eh += 24;
    for (var h = sh; h <= eh; h++) labels.push(String(h % 24).padStart(2, '0') + 'H');
    return labels;
}

// ─── 경과 시간 ────────────────────────────────────────────
function calculateElapsedTime(startTime) {
    var nowJakarta = new Date(new Date().toLocaleString('en-US', { timeZone: 'Asia/Jakarta' }));
    var start      = new Date(startTime.replace(' ', 'T'));
    var diffMs     = nowJakarta - start;
    if (diffMs < 0) return '0s';

    var totalSec = Math.floor(diffMs / 1000);
    var hours    = Math.floor(totalSec / 3600);
    var minutes  = Math.floor((totalSec % 3600) / 60);
    var seconds  = totalSec % 60;

    var result = '';
    if (hours   > 0) result += hours   + 'h ';
    if (minutes > 0) result += minutes + 'm ';
    if (seconds > 0 || result === '') result += seconds + 's';
    return result.trim();
}

function updateElapsedTimes() {
    var sorted = activeDefectives.slice().sort(function (a, b) {
        return new Date(b.reg_date) - new Date(a.reg_date);
    }).slice(0, 5);

    document.querySelectorAll('.active-defective-item').forEach(function (item, i) {
        if (sorted[i] && sorted[i].reg_date) {
            var span = item.querySelector('.defective-elapsed-time span');
            if (span) span.textContent = calculateElapsedTime(sorted[i].reg_date);
        }
    });

    document.querySelectorAll('.defective-elapsed-time-cell').forEach(function (cell) {
        var status   = cell.getAttribute('data-status');
        var regDate  = cell.getAttribute('data-reg-date');
        var duration = cell.getAttribute('data-duration');
        if (status === 'Warning' && regDate) {
            cell.textContent = calculateElapsedTime(regDate);
        } else if (duration && duration !== '-') {
            cell.textContent = duration;
        }
    });

    if (window.currentModalDefectiveData) {
        var d   = window.currentModalDefectiveData;
        var mel = document.getElementById('modal-elapsed-time');
        if (mel && d.status === 'Warning' && d.reg_date) {
            mel.textContent = calculateElapsedTime(d.reg_date);
        }
    }
}

function startElapsedTimeTimer() {
    if (elapsedTimeTimer) clearInterval(elapsedTimeTimer);
    elapsedTimeTimer = setInterval(updateElapsedTimes, 1000);
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
    var f = getFilterParams();
    var p = new URLSearchParams();
    if (f.factory_filter) p.append('factory_filter', f.factory_filter);
    if (f.line_filter)    p.append('line_filter',    f.line_filter);
    if (f.machine_filter) p.append('machine_filter', f.machine_filter);
    if (f.shift_filter)   p.append('shift_filter',   f.shift_filter);
    if (f.start_date)     p.append('start_date',     f.start_date);
    if (f.end_date)       p.append('end_date',       f.end_date);
    window.location.href = 'proc/data_defective_export.php?' + p.toString();
}

async function refreshData() {
    if (isTracking) { stopTracking(); setTimeout(async function () { await startAutoTracking(); }, 1000); }
    else await startAutoTracking();
}

// ─── Modal ────────────────────────────────────────────────
function openDefectiveDetailModal(btn) {
    try {
        var data = JSON.parse(btn.getAttribute('data-defective-data').replace(/&#39;/g, "'"));
        window.currentModalDefectiveData = data;
        populateDefectiveModal(data);
        var modal = document.getElementById('defectiveDetailModal');
        if (modal) { modal.classList.add('show'); document.body.style.overflow = 'hidden'; }
    } catch (e) { console.error('Modal open error:', e); }
}

function closeDefectiveDetailModal() {
    var modal = document.getElementById('defectiveDetailModal');
    if (!modal) return;
    modal.classList.add('hide');
    window.currentModalDefectiveData = null;
    setTimeout(function () { modal.classList.remove('show', 'hide'); document.body.style.overflow = ''; }, 300);
}

function populateDefectiveModal(d) {
    var set = function (id, v) { var el = document.getElementById(id); if (el) el.textContent = v; };

    set('modal-machine-no',     d.machine_no || '-');
    set('modal-factory-line',   (d.factory_name || '-') + ' / ' + (d.line_name || '-'));
    set('modal-defective-type', d.defective_name || '-');

    var statusEl = document.getElementById('modal-status');
    if (statusEl) {
        statusEl.innerHTML = d.status === 'Warning'
            ? '<span class="fiori-badge fiori-badge--error">Warning</span>'
            : '<span class="fiori-badge fiori-badge--success">Completed</span>';
    }

    set('modal-reg-date',   d.reg_date  || '-');
    var elapsed = d.status === 'Warning'
        ? calculateElapsedTime(d.reg_date)
        : (d.duration_display || d.duration_his || '-');
    set('modal-elapsed-time', elapsed);
    set('modal-work-date',  d.work_date || '-');
    set('modal-shift',      d.shift_idx ? 'Shift ' + d.shift_idx : '-');

    var colorIndicator = document.getElementById('modal-color-indicator');
    var colorValue     = document.getElementById('modal-color-value');
    if (colorIndicator && colorValue) {
        if (d.defective_color) {
            colorIndicator.style.backgroundColor = d.defective_color;
            colorIndicator.style.display = 'inline-block';
            colorValue.textContent = d.defective_color;
        } else {
            colorIndicator.style.display = 'none';
            colorValue.textContent = 'Default Color';
        }
    }

    set('modal-idx',        d.idx     || '-');
    set('modal-created-at', d.reg_date || '-');
}

function exportSingleDefective() {
    closeDefectiveDetailModal();
}
