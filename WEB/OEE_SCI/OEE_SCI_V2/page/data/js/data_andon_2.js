// data_andon_2.js — Row Grid Layout version (ai_dashboard_5 model)

let eventSource = null;
let isTracking = false;
let charts = {};
let andonData = [];
let activeAndons = [];
let reconnectAttempts = 0;
let maxReconnectAttempts = 3;
let stats = {};
let elapsedTimeTimer = null;
let durationUpdateTimer = null;
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
    await startAutoTracking();
});

// ─── 페이지 언로드 처리 ──────────────────────────────────
window.addEventListener('beforeunload', () => {
    isPageUnloading = true;
    stopAllTimers();
    if (eventSource) { eventSource.close(); eventSource = null; isTracking = false; }
});

window.addEventListener('pagehide', () => {
    isPageUnloading = true;
    stopAllTimers();
    if (eventSource) { eventSource.close(); eventSource = null; isTracking = false; }
});

document.addEventListener('visibilitychange', () => {
    if (document.hidden) {
        stopAllTimers();
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
    var main = document.getElementById('andonSignageMain');
    if (!main) return;

    var rows = [];
    if (!document.getElementById('andonRowStats').classList.contains('hidden'))        rows.push('auto');
    if (!document.getElementById('andonRowChartsTop').classList.contains('hidden'))    rows.push('1fr');
    if (!document.getElementById('andonRowChartsBottom').classList.contains('hidden')) rows.push('1fr');
    if (!document.getElementById('andonRowTable').classList.contains('hidden'))        rows.push('1fr');
    rows.push('auto'); // pagination

    main.style.gridTemplateRows = rows.join(' ');

    setTimeout(function () {
        Object.values(charts).forEach(function (c) { if (c) c.resize(); });
    }, 50);
}

// ─── 토글 함수 ────────────────────────────────────────────
function toggleStatsDisplay() {
    var row = document.getElementById('andonRowStats');
    var btn = document.getElementById('toggleStatsBtn');
    if (!row || !btn) return;
    row.classList.toggle('hidden');
    btn.textContent = row.classList.contains('hidden') ? 'Show Stats' : 'Hide Stats';
    updateLayout();
}

function toggleChartsDisplay() {
    var rowTop    = document.getElementById('andonRowChartsTop');
    var rowBottom = document.getElementById('andonRowChartsBottom');
    var btn       = document.getElementById('toggleChartsBtn');
    if (!rowTop || !rowBottom || !btn) return;
    var isHidden = rowTop.classList.contains('hidden');
    rowTop.classList.toggle('hidden', !isHidden);
    rowBottom.classList.toggle('hidden', !isHidden);
    btn.textContent = isHidden ? 'Hide Charts' : 'Show Charts';
    updateLayout();
}

function toggleDataDisplay() {
    var rowTable      = document.getElementById('andonRowTable');
    var rowPagination = document.getElementById('andonRowPagination');
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
                'Today':      [moment(), moment()],
                'Yesterday':  [moment().subtract(1, 'days'), moment().subtract(1, 'days')],
                'Last Week':  [moment().subtract(6, 'days'), moment()],
                'Last Month': [moment().subtract(29, 'days'), moment()]
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
    var lineSelect = document.getElementById('factoryLineFilterSelect');
    lineSelect.disabled = true;
    try {
        var url = '../manage/proc/line.php?status_filter=Y' + (factoryId ? '&factory_filter=' + factoryId : '');
        var res = await fetch(url).then(r => r.json());
        lineSelect.innerHTML = '<option value="">All Line</option>';
        if (res.success && res.data) {
            res.data.forEach(function (l) {
                lineSelect.innerHTML += '<option value="' + l.idx + '">' + l.line_name + '</option>';
            });
        }
        lineSelect.disabled = false;
        var machineSelect = document.getElementById('factoryLineMachineFilterSelect');
        if (factoryId) { machineSelect.disabled = false; updateMachineOptions(factoryId, ''); }
    } catch (e) {
        lineSelect.innerHTML = '<option value="">All Line</option>';
        lineSelect.disabled = false;
    }
}

async function updateMachineOptions(factoryId, lineId) {
    var machineSelect = document.getElementById('factoryLineMachineFilterSelect');
    machineSelect.disabled = true;
    try {
        var params = new URLSearchParams();
        if (factoryId) params.append('factory_filter', factoryId);
        if (lineId) params.append('line_filter', lineId);
        var url = '../manage/proc/machine.php' + (params.toString() ? '?' + params.toString() : '');
        var res = await fetch(url).then(r => r.json());
        machineSelect.innerHTML = '<option value="">All Machine</option>';
        if (res.success && res.data) {
            res.data.forEach(function (m) {
                machineSelect.innerHTML += '<option value="' + m.idx + '">' + m.machine_no + ' (' + (m.machine_model_name || 'No Model') + ')</option>';
            });
        }
        machineSelect.disabled = false;
    } catch (e) {
        machineSelect.innerHTML = '<option value="">All Machine</option>';
        machineSelect.disabled = false;
    }
}

// ─── 차트 초기화 ──────────────────────────────────────────
function initCharts() {
    charts.andonType  = createAndonTypeChart();
    charts.andonTrend = createAndonTrendChart();
}

function createAndonTypeChart() {
    var ctx = document.getElementById('andonTypeChart').getContext('2d');
    return new Chart(ctx, {
        type: 'bar',
        data: {
            labels: [],
            datasets: [{
                label: 'Andon Occurrence Count',
                data: [],
                backgroundColor: [chartColors.warning, chartColors.error, chartColors.primary, chartColors.info, chartColors.accent, chartColors.secondary],
                borderColor:     [chartColors.warning, chartColors.error, chartColors.primary, chartColors.info, chartColors.accent, chartColors.secondary],
                borderWidth: 1
            }]
        },
        options: {
            responsive: true,
            maintainAspectRatio: false,
            plugins: { legend: { display: false } },
            scales: {
                y: { beginAtZero: true, ticks: { stepSize: 1, precision: 0 }, title: { display: true, text: 'Andon Count' } },
                x: { title: { display: true, text: 'Andon Type' } }
            }
        }
    });
}

function createAndonTrendChart() {
    var ctx = document.getElementById('andonTrendChart').getContext('2d');
    return new Chart(ctx, {
        type: 'line',
        data: {
            labels: [],
            datasets: [
                {
                    label: 'Warning',
                    data: [],
                    borderColor: chartColors.error,
                    backgroundColor: chartColors.error + '20',
                    fill: true, tension: 0.4,
                    pointBackgroundColor: chartColors.error,
                    pointRadius: 4, pointHoverRadius: 6
                },
                {
                    label: 'Completed',
                    data: [],
                    borderColor: chartColors.success,
                    backgroundColor: chartColors.success + '20',
                    fill: true, tension: 0.4,
                    pointBackgroundColor: chartColors.success,
                    pointRadius: 4, pointHoverRadius: 6
                }
            ]
        },
        options: {
            responsive: true,
            maintainAspectRatio: false,
            plugins: {
                legend: { display: true, position: 'top' },
                tooltip: { mode: 'index', intersect: false }
            },
            scales: {
                y: { beginAtZero: true, ticks: { stepSize: 1, precision: 0 }, title: { display: true, text: 'Andon Count' } },
                x: { title: { display: true, text: 'Time/Period' } }
            },
            interaction: { mode: 'nearest', axis: 'x', intersect: false }
        }
    });
}

// ─── 이벤트 리스너 ────────────────────────────────────────
function setupEventListeners() {
    var toggleStatsBtn   = document.getElementById('toggleStatsBtn');
    var toggleChartsBtn  = document.getElementById('toggleChartsBtn');
    var toggleDataBtn    = document.getElementById('toggleDataBtn');
    var excelDownloadBtn = document.getElementById('excelDownloadBtn');
    var refreshBtn       = document.getElementById('refreshBtn');
    var timeRangeSelect  = document.getElementById('timeRangeSelect');
    var machineSelect    = document.getElementById('factoryLineMachineFilterSelect');
    var shiftSelect      = document.getElementById('shiftSelect');

    if (toggleStatsBtn)   toggleStatsBtn.addEventListener('click', toggleStatsDisplay);
    if (toggleChartsBtn)  toggleChartsBtn.addEventListener('click', toggleChartsDisplay);
    if (toggleDataBtn)    toggleDataBtn.addEventListener('click', toggleDataDisplay);
    if (excelDownloadBtn) excelDownloadBtn.addEventListener('click', exportData);
    if (refreshBtn)       refreshBtn.addEventListener('click', refreshData);
    if (timeRangeSelect)  timeRangeSelect.addEventListener('change', handleTimeRangeChange);
    if (machineSelect)    machineSelect.addEventListener('change', async function () { await restartRealTimeMonitoring(); });
    if (shiftSelect)      shiftSelect.addEventListener('change', async function () { await restartRealTimeMonitoring(); });

    document.addEventListener('keydown', function (e) {
        if (e.key === 'Escape') {
            var modal = document.getElementById('andonDetailModal');
            if (modal && modal.classList.contains('show')) closeAndonDetailModal();
        }
    });
}

// ─── 초기 데이터 로딩 ────────────────────────────────────
async function loadInitialData() {
    var statIds = ['totalAndons', 'activeWarnings', 'currentShiftCount', 'affectedMachines', 'urgentWarnings', 'avgCompletedTime'];
    statIds.forEach(function (id) {
        var el = document.getElementById(id);
        if (el) el.textContent = '-';
    });
    var countEl = document.getElementById('activeAndonCount');
    if (countEl) countEl.textContent = '0 active alerts';
}

// ─── SSE 실시간 모니터링 ─────────────────────────────────
async function startAutoTracking() {
    if (isTracking) return;

    var filters = getFilterParams();
    var params = new URLSearchParams(filters);
    var sseUrl = 'proc/data_andon_stream.php?' + params.toString();

    eventSource = new EventSource(sseUrl);
    setupSSEEventListeners();
    isTracking = true;
}

function getFilterParams() {
    var dateRange = $('#dateRangePicker').val();
    var startDate = '', endDate = '';
    if (dateRange && dateRange.includes(' ~ ')) {
        var dates = dateRange.split(' ~ ');
        startDate = dates[0];
        endDate   = dates[1];
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

function setupSSEEventListeners() {
    eventSource.onerror = function () {
        isTracking = false;
        attemptReconnection();
    };

    eventSource.addEventListener('connected', function () {
        reconnectAttempts = 0;
        var el = document.getElementById('connectionStatus');
        if (el) el.textContent = 'Andon system connected';
    });

    eventSource.addEventListener('andon_data', function (event) {
        var data = JSON.parse(event.data);
        stats        = data.stats;
        activeAndons = data.active_andons;
        andonData    = data.andon_data;
        window.andonData = data.andon_data;

        updateStatCards(stats);
        updateActiveAndonsDisplay(activeAndons);
        updateTableFromAPI(andonData);
        updateChartsFromAPI(data);

        var el = document.getElementById('lastUpdateTime');
        if (el) el.textContent = 'Last updated: ' + data.timestamp;
    });

    eventSource.addEventListener('heartbeat', function (event) {
        var data = JSON.parse(event.data);
        var el = document.getElementById('connectionStatus');
        if (el) el.textContent = 'Connection maintained (Active warnings: ' + data.active_warnings + ')';
    });

    eventSource.addEventListener('disconnected', function () {
        var el = document.getElementById('connectionStatus');
        if (el) el.textContent = 'Disconnected';
    });
}

// ─── 통계 카드 업데이트 ───────────────────────────────────
function updateStatCards(statsData) {
    if (!statsData) return;
    var map = {
        'totalAndons':      statsData.total_count      || '0',
        'activeWarnings':   statsData.warning_count    || '0',
        'currentShiftCount': statsData.current_shift_count || '0',
        'affectedMachines': statsData.affected_machines || '0',
        'urgentWarnings':   statsData.urgent_warnings_count || '0',
        'avgCompletedTime': statsData.avg_completed_time || '-'
    };
    Object.entries(map).forEach(function ([id, val]) {
        var el = document.getElementById(id);
        if (el) el.textContent = val;
    });
}

// ─── 활성 안돈 표시 ───────────────────────────────────────
function updateActiveAndonsDisplay(activeAndonsList) {
    var container = document.getElementById('activeAndonsContainer');
    var countEl   = document.getElementById('activeAndonCount');

    if (countEl) countEl.textContent = activeAndonsList.length + ' active alerts';
    if (!container) return;

    if (activeAndonsList.length === 0) {
        container.innerHTML = '<div class="fiori-alert fiori-alert--success"><strong>Good:</strong> No active Andons. All systems normal.</div>';
        stopElapsedTimeTimer();
        return;
    }

    var html = '';
    activeAndonsList.forEach(function (andon) {
        var initialElapsed = calculateElapsedTime(andon.reg_date);
        var borderColor    = andon.andon_color || '#0070f2';
        var bgColor        = borderColor + '1A';
        html += '<div class="active-andon-item" style="border: 2px dashed ' + borderColor + '; background-color: ' + bgColor + ';">'
              +   '<div class="andon-machine-info">'
              +     '<div class="andon-machine-name">' + (andon.andon_name || '-') + '</div>'
              +     '<div class="andon-location">'
              +       '<span class="andon-location-item">' + (andon.factory_name || '') + '</span>'
              +       '<span class="andon-location-item"> / </span>'
              +       '<span class="andon-location-item">' + (andon.line_name || '') + '</span>'
              +       '<span class="andon-location-item"> / </span>'
              +       '<span class="andon-location-item">' + (andon.machine_no || '') + '</span>'
              +     '</div>'
              +   '</div>'
              +   '<div class="andon-elapsed-time"><span>' + initialElapsed + '</span></div>'
              + '</div>';
    });
    container.innerHTML = html;
    startElapsedTimeTimer();
}

// ─── 테이블 업데이트 ──────────────────────────────────────
function updateTableFromAPI(andonDataList) {
    var tbody = document.getElementById('andonDataBody');
    if (!tbody) return;

    totalItems = andonDataList.length;

    if (andonDataList.length === 0) {
        tbody.innerHTML = '<tr><td colspan="10" class="data-table-centered"><div class="fiori-alert fiori-alert--info"><strong>Information:</strong> No Andon data matching the selected conditions.</div></td></tr>';
        renderPagination();
        return;
    }

    var startIndex = (currentPage - 1) * itemsPerPage;
    var paginatedData = andonDataList.slice(startIndex, startIndex + itemsPerPage);

    tbody.innerHTML = '';
    paginatedData.forEach(function (andon) {
        var row = document.createElement('tr');

        var statusBadge = andon.status === 'Warning'
            ? '<span class="fiori-badge fiori-badge--error">Warning</span>'
            : '<span class="fiori-badge fiori-badge--success">Completed</span>';

        var shiftDisplay = andon.shift_idx ? 'Shift ' + andon.shift_idx : '-';

        var durationCell = '';
        if (andon.status === 'Warning' && andon.reg_date) {
            var initialDuration = calculateElapsedTime(andon.reg_date);
            durationCell = '<td class="duration-in-progress" data-start-time="' + andon.reg_date + '">' + initialDuration + '</td>';
        } else {
            durationCell = '<td>' + (andon.duration_display || andon.duration_his || '-') + '</td>';
        }

        row.innerHTML = '<td>' + (andon.machine_no || '-') + '</td>'
            + '<td>' + ((andon.factory_name || '') + ' / ' + (andon.line_name || '')) + '</td>'
            + '<td>' + shiftDisplay + '</td>'
            + '<td>' + (andon.andon_name || '-') + '</td>'
            + '<td>' + statusBadge + '</td>'
            + '<td>' + (andon.reg_date || '-') + '</td>'
            + '<td>' + (andon.update_date || '-') + '</td>'
            + durationCell
            + '<td>' + (andon.work_date || '-') + '</td>'
            + '<td><button class="fiori-btn fiori-btn--tertiary andon-details-btn" style="padding:0.25rem 0.5rem;font-size:0.75rem;" data-andon-data=\'' + JSON.stringify(andon).replace(/'/g, "&#39;") + '\'>Detail</button></td>';

        tbody.appendChild(row);
    });

    renderPagination();
    setupDetailsButtonListeners();
    startDurationUpdateTimer();
}

function setupDetailsButtonListeners() {
    document.querySelectorAll('.andon-details-btn').forEach(function (btn) {
        btn.removeEventListener('click', handleDetailsButtonClick);
        btn.addEventListener('click', handleDetailsButtonClick);
    });
}

function handleDetailsButtonClick(event) {
    event.preventDefault();
    event.stopPropagation();
    var btn = event.target.closest('.andon-details-btn');
    if (btn) openAndonDetailModal(btn);
}

// ─── 페이지네이션 ─────────────────────────────────────────
function renderPagination() {
    var container = document.getElementById('pagination-controls');
    if (!container) return;

    var totalPages = Math.ceil(totalItems / itemsPerPage);
    if (totalPages <= 1) { container.innerHTML = ''; return; }

    var startItem = totalItems === 0 ? 0 : (currentPage - 1) * itemsPerPage + 1;
    var endItem   = Math.min(currentPage * itemsPerPage, totalItems);

    var html = '<div class="fiori-pagination__info">' + startItem + '-' + endItem + ' / ' + totalItems + ' items (' + itemsPerPage + ' per page)</div>'
             + '<div class="fiori-pagination__buttons">';

    html += '<button class="fiori-pagination__button' + (currentPage === 1 ? ' fiori-pagination__button--disabled' : '') + '" onclick="changePage(' + (currentPage - 1) + ')"' + (currentPage === 1 ? ' disabled' : '') + '>&larr;</button>';

    var startPage = Math.max(1, currentPage - 2);
    var endPage   = Math.min(totalPages, currentPage + 2);
    if (startPage > 1) { html += '<button class="fiori-pagination__button" onclick="changePage(1)">1</button>'; if (startPage > 2) html += '<span class="fiori-pagination__ellipsis">...</span>'; }
    for (var i = startPage; i <= endPage; i++) {
        html += '<button class="fiori-pagination__button' + (i === currentPage ? ' fiori-pagination__button--active' : '') + '" onclick="changePage(' + i + ')">' + i + '</button>';
    }
    if (endPage < totalPages) { if (endPage < totalPages - 1) html += '<span class="fiori-pagination__ellipsis">...</span>'; html += '<button class="fiori-pagination__button" onclick="changePage(' + totalPages + ')">' + totalPages + '</button>'; }
    html += '<button class="fiori-pagination__button' + (currentPage === totalPages ? ' fiori-pagination__button--disabled' : '') + '" onclick="changePage(' + (currentPage + 1) + ')"' + (currentPage === totalPages ? ' disabled' : '') + '>&rarr;</button>';
    html += '</div>';

    container.innerHTML = html;
}

function changePage(newPage) {
    var totalPages = Math.ceil(totalItems / itemsPerPage);
    if (newPage < 1 || newPage > totalPages || newPage === currentPage) return;
    currentPage = newPage;
    updateTableFromAPI(andonData);
}

// ─── 차트 업데이트 ────────────────────────────────────────
function updateChartsFromAPI(apiData) {
    if (apiData.andon_type_stats && charts.andonType)  updateAndonTypeChart(apiData.andon_type_stats);
    if (apiData.andon_trend_stats && charts.andonTrend) updateAndonTrendChart(apiData.andon_trend_stats);
}

function updateAndonTypeChart(andonTypeStats) {
    if (!charts.andonType || !andonTypeStats || andonTypeStats.length === 0) return;

    var sorted = andonTypeStats.slice().sort(function (a, b) {
        return (a.andon_name || '').toLowerCase().localeCompare((b.andon_name || '').toLowerCase());
    });
    var labels  = sorted.map(function (i) { return i.andon_name || 'Unclassified'; });
    var counts  = sorted.map(function (i) { return parseInt(i.count) || 0; });
    var fallback = ['#0070f2', '#da1e28', '#e26b0a', '#30914c', '#8e44ad', '#1e88e5'];
    var colors  = sorted.map(function (i, idx) { return i.andon_color || fallback[idx % fallback.length]; });

    charts.andonType.data.labels                        = labels;
    charts.andonType.data.datasets[0].data              = counts;
    charts.andonType.data.datasets[0].backgroundColor   = colors;
    charts.andonType.data.datasets[0].borderColor       = colors;
    charts.andonType.update('none');
}

function updateAndonTrendChart(andonTrendStats) {
    if (!charts.andonTrend || !andonTrendStats) return;

    var trendData = andonTrendStats.data || andonTrendStats;
    var workHours = andonTrendStats.work_hours || null;
    var viewType  = andonTrendStats.view_type  || 'hourly';

    if (!trendData || trendData.length === 0) {
        if (window.andonData && window.andonData.length > 0) { generateSimpleTrendFromAndonData(); return; }
        charts.andonTrend.data.labels = ['No Data'];
        charts.andonTrend.data.datasets[0].data = [0];
        charts.andonTrend.data.datasets[1].data = [0];
        charts.andonTrend.update('none');
        return;
    }

    var labels = [];
    if (viewType === 'hourly' && workHours && workHours.start_time && workHours.end_time) {
        labels = generateWorkHoursLabels(workHours.start_time, workHours.end_time, workHours.start_minutes, workHours.end_minutes);
    } else if (viewType === 'daily') {
        labels = trendData.map(function (i) { return i.display_label || i.time_label; });
    } else {
        labels = trendData.map(function (i) {
            var lbl = i.time_label || i.display_label || '';
            var m = lbl.match(/^(\d{4}-\d{2}-\d{2})\s+(\d{2}):/);
            return m ? m[2] + 'H' : lbl;
        });
    }

    var warningMap = {}, completedMap = {};
    trendData.forEach(function (i) {
        var key = '';
        if (viewType === 'daily') {
            key = i.display_label || i.time_label;
        } else {
            var lbl = i.time_label || i.display_label || '';
            var m = lbl.match(/^(\d{4}-\d{2}-\d{2})\s+(\d{2}):/);
            key = m ? m[2] + 'H' : lbl;
        }
        if (key) {
            warningMap[key]   = parseInt(i.warning_count)   || 0;
            completedMap[key] = parseInt(i.completed_count) || 0;
        }
    });

    charts.andonTrend.data.labels = labels;
    charts.andonTrend.data.datasets[0].data = labels.map(function (l) { return (warningMap[l] || 0) + (completedMap[l] || 0); });
    charts.andonTrend.data.datasets[1].data = labels.map(function (l) { return completedMap[l] || 0; });
    charts.andonTrend.options.scales.x.title.text = viewType === 'hourly' ? 'Time' : 'Date';
    charts.andonTrend.update('show');
}

function generateWorkHoursLabels(startTime, endTime, startMinutes, endMinutes) {
    var labels = [], startHour, endHour;
    if (startMinutes !== null && endMinutes !== null) {
        startHour = Math.floor(startMinutes / 60);
        endHour   = Math.floor(endMinutes / 60);
    } else {
        startHour = parseInt(startTime.split(':')[0]);
        endHour   = parseInt(endTime.split(':')[0]);
        if (endHour < startHour) endHour += 24;
    }
    for (var h = startHour; h <= endHour; h++) {
        labels.push((h % 24).toString().padStart(2, '0') + 'H');
    }
    return labels;
}

function generateSimpleTrendFromAndonData() {
    if (!charts.andonTrend || !window.andonData || window.andonData.length === 0) return;
    var dateGroups = {};
    window.andonData.forEach(function (item) {
        if (!item.work_date) return;
        if (!dateGroups[item.work_date]) dateGroups[item.work_date] = { total: 0, completed: 0 };
        dateGroups[item.work_date].total++;
        if (item.status === 'Completed') dateGroups[item.work_date].completed++;
    });
    var dates  = Object.keys(dateGroups).sort();
    var labels = dates.map(function (d) { var x = new Date(d); return (x.getMonth() + 1) + '/' + x.getDate(); });
    charts.andonTrend.data.labels            = labels;
    charts.andonTrend.data.datasets[0].data  = dates.map(function (d) { return dateGroups[d].total; });
    charts.andonTrend.data.datasets[1].data  = dates.map(function (d) { return dateGroups[d].completed; });
    charts.andonTrend.options.scales.x.title.text = 'Date';
    charts.andonTrend.update('show');
}

// ─── 실시간 타이머 ────────────────────────────────────────
function calculateElapsedTime(startTime) {
    var now   = new Date(new Date().toLocaleString('en-US', { timeZone: 'Asia/Jakarta' }));
    var start = new Date(startTime.replace(' ', 'T'));
    var diff  = Math.floor((now - start) / 1000);
    if (diff < 0) return '0s';
    var h = Math.floor(diff / 3600);
    var m = Math.floor((diff % 3600) / 60);
    var s = diff % 60;
    var r = '';
    if (h > 0) r += h + 'h ';
    if (m > 0) r += m + 'm ';
    if (s > 0 || r === '') r += s + 's';
    return r.trim();
}

function updateElapsedTimes() {
    document.querySelectorAll('.active-andon-item').forEach(function (item, i) {
        if (activeAndons[i] && activeAndons[i].reg_date) {
            var span = item.querySelector('.andon-elapsed-time span');
            if (span) span.textContent = calculateElapsedTime(activeAndons[i].reg_date);
        }
    });
}

function updateInProgressDurations() {
    document.querySelectorAll('.duration-in-progress').forEach(function (cell) {
        var t = cell.getAttribute('data-start-time');
        if (t) cell.textContent = calculateElapsedTime(t);
    });
}

function startElapsedTimeTimer() {
    if (elapsedTimeTimer) clearInterval(elapsedTimeTimer);
    elapsedTimeTimer = setInterval(updateElapsedTimes, 1000);
}

function stopElapsedTimeTimer() {
    if (elapsedTimeTimer) { clearInterval(elapsedTimeTimer); elapsedTimeTimer = null; }
}

function startDurationUpdateTimer() {
    if (durationUpdateTimer) clearInterval(durationUpdateTimer);
    durationUpdateTimer = setInterval(updateInProgressDurations, 1000);
}

function stopDurationUpdateTimer() {
    if (durationUpdateTimer) { clearInterval(durationUpdateTimer); durationUpdateTimer = null; }
}

function stopAllTimers() {
    stopElapsedTimeTimer();
    stopDurationUpdateTimer();
}

// ─── 필터 변경 / 재시작 ──────────────────────────────────
function stopTracking() {
    if (eventSource) { eventSource.close(); eventSource = null; }
    stopAllTimers();
    isTracking = false;
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

// ─── 날짜 / 시간 범위 핸들러 ─────────────────────────────
async function handleDateRangeChange(start, end) {
    var sel = document.getElementById('timeRangeSelect');
    if (sel) {
        var opt = sel.querySelector('option[value="custom"]');
        if (!opt) { opt = document.createElement('option'); opt.value = 'custom'; opt.textContent = 'Custom'; sel.appendChild(opt); }
        sel.value = 'custom';
    }
    if (isTracking) { stopTracking(); setTimeout(async function () { await startAutoTracking(); }, 500); }
    else { await startAutoTracking(); }
}

async function handleTimeRangeChange(event) {
    var range = event.target.value;
    if (range !== 'custom') {
        var startDate, endDate;
        switch (range) {
            case 'yesterday': startDate = moment().subtract(1, 'days').startOf('day'); endDate = moment().subtract(1, 'days').endOf('day'); break;
            case '1w':        startDate = moment().subtract(7, 'days').startOf('day'); endDate = moment().endOf('day'); break;
            case '1m':        startDate = moment().subtract(30, 'days').startOf('day'); endDate = moment().endOf('day'); break;
            default:          startDate = moment().startOf('day'); endDate = moment().endOf('day');
        }
        $('#dateRangePicker').data('daterangepicker').setStartDate(startDate);
        $('#dateRangePicker').data('daterangepicker').setEndDate(endDate);
        $('#dateRangePicker').val(startDate.format('YYYY-MM-DD') + ' ~ ' + endDate.format('YYYY-MM-DD'));
    }
    await restartRealTimeMonitoring();
}

// ─── Export / Refresh ────────────────────────────────────
function exportData() {
    var filters = getFilterParams();
    var params  = new URLSearchParams();
    if (filters.factory_filter) params.append('factory_filter', filters.factory_filter);
    if (filters.line_filter)    params.append('line_filter',    filters.line_filter);
    if (filters.machine_filter) params.append('machine_filter', filters.machine_filter);
    if (filters.shift_filter)   params.append('shift_filter',   filters.shift_filter);
    if (filters.start_date)     params.append('start_date',     filters.start_date);
    if (filters.end_date)       params.append('end_date',       filters.end_date);
    window.location.href = 'proc/data_andon_export.php?' + params.toString();
}

async function refreshData() {
    if (isTracking) { stopTracking(); setTimeout(async function () { await startAutoTracking(); }, 500); }
    else { await startAutoTracking(); }
}

// ─── Andon Detail Modal ──────────────────────────────────
function openAndonDetailModal(buttonElement) {
    try {
        var json = buttonElement.getAttribute('data-andon-data');
        if (!json) return;
        var data = JSON.parse(json);
        populateAndonModal(data);
        var modal = document.getElementById('andonDetailModal');
        if (modal) { modal.classList.add('show'); document.body.style.overflow = 'hidden'; }
    } catch (e) { console.error('Modal open error:', e); }
}

function closeAndonDetailModal() {
    var modal = document.getElementById('andonDetailModal');
    if (modal) {
        modal.classList.add('hide');
        setTimeout(function () {
            modal.classList.remove('show', 'hide');
            document.body.style.overflow = '';
        }, 300);
    }
}

function populateAndonModal(data) {
    function setVal(id, val) { var el = document.getElementById(id); if (el) el.textContent = val; }

    setVal('modal-machine-no',   data.machine_no || '-');
    setVal('modal-factory-line', (data.factory_name || '-') + ' / ' + (data.line_name || '-'));
    setVal('modal-andon-type',   data.andon_name || '-');

    var statusEl = document.getElementById('modal-status');
    if (statusEl) {
        statusEl.innerHTML = data.status === 'Warning'
            ? '<span class="fiori-badge fiori-badge--error">Warning</span>'
            : '<span class="fiori-badge fiori-badge--success">Completed</span>';
    }

    setVal('modal-reg-date',    data.reg_date    || '-');
    setVal('modal-update-date', data.update_date || '-');
    setVal('modal-duration',    data.duration_display || data.duration_his || '-');
    setVal('modal-work-date',   data.work_date   || '-');
    setVal('modal-shift',       data.shift_idx ? 'Shift ' + data.shift_idx : '-');

    var indicator = document.getElementById('modal-color-indicator');
    var colorVal  = document.getElementById('modal-color-value');
    if (indicator && colorVal) {
        indicator.style.backgroundColor = data.andon_color || '#cccccc';
        colorVal.textContent = data.andon_color || 'Default Color';
    }

    setVal('modal-idx',        data.idx        || '-');
    setVal('modal-created-at', data.created_at || data.reg_date || '-');
}

function exportSingleAndon() { closeAndonDetailModal(); }

// 전역 등록 (HTML onclick에서 사용)
window.openAndonDetailModal  = openAndonDetailModal;
window.closeAndonDetailModal = closeAndonDetailModal;
window.exportSingleAndon     = exportSingleAndon;
window.changePage            = changePage;
