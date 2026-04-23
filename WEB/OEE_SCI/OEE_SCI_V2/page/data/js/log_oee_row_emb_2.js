/**
 * log_oee_row_emb_2.js — 자수기(EMB) 패킷 스냅샷 로그 페이지
 * SSE 대상: proc/log_oee_row_emb_stream_2.php (data_oee_rows_emb)
 */

let eventSource = null;
let isTracking = false;
let rowEmbData = [];
let reconnectAttempts = 0;
let maxReconnectAttempts = 3;
let stats = {};
let isPageUnloading = false;

let currentPage = 1;
let itemsPerPage = 20;
let totalItems = 0;

const columnConfig = [
    { key: 'idx',               label: 'idx',               visible: false },
    { key: 'work_date',         label: 'work_date',         visible: true  },
    { key: 'time_update',       label: 'time_update',       visible: true  },
    { key: 'shift_idx',         label: 'shift_idx',         visible: true  },
    { key: 'factory_idx',       label: 'factory_idx',       visible: false },
    { key: 'factory_name',      label: 'factory_name',      visible: true  },
    { key: 'line_idx',          label: 'line_idx',          visible: false },
    { key: 'line_name',         label: 'line_name',         visible: true  },
    { key: 'mac',               label: 'mac',               visible: true  },
    { key: 'machine_idx',       label: 'machine_idx',       visible: false },
    { key: 'machine_no',        label: 'machine_no',        visible: true  },
    { key: 'process_name',      label: 'process_name',      visible: true  },
    { key: 'planned_work_time', label: 'planned_work_time', visible: true  },
    { key: 'runtime',           label: 'runtime',           visible: true  },
    { key: 'actual_output',     label: 'actual_output',     visible: true  },
    { key: 'packet_qty',        label: 'packet_qty',        visible: true  },
    { key: 'cycle_time',        label: 'cycle_time',        visible: true  },
    { key: 'thread_breakage',   label: 'thread_breakage',   visible: true  },
    { key: 'motor_run_time',    label: 'motor_run_time',    visible: true  },
    { key: 'pair_info',         label: 'pair_info',         visible: false },
    { key: 'pair_count',        label: 'pair_count',        visible: false },
    { key: 'work_hour',         label: 'work_hour',         visible: true  },
    { key: 'reg_date',          label: 'reg_date',          visible: true  }
];

document.addEventListener('DOMContentLoaded', async function () {
    initDateRangePicker();
    await initFilterSystem();
    initColumnToggle();
    setupEventListeners();
    renderTableHeader();
    updateLayout();
    initStickyColumnsScroll();
    await loadInitialData();
    await startAutoTracking();
});

function initStickyColumnsScroll() {
    const wrap = document.querySelector('.oee-table-wrap');
    if (!wrap) return;
    let _stickyEls = [], _naturalOffsets = [];
    function _captureStickyOffsets() {
        const prev = wrap.scrollLeft;
        if (prev !== 0) wrap.scrollLeft = 0;
        _stickyEls = Array.from(wrap.querySelectorAll('.sticky-column'));
        _naturalOffsets = _stickyEls.map(el => el.offsetLeft);
        if (prev !== 0) wrap.scrollLeft = prev;
        _updatePositions();
    }
    function _updatePositions() {
        const sl = wrap.scrollLeft;
        _stickyEls.forEach((el, i) => {
            const shift = sl > (_naturalOffsets[i] || 0) ? sl - _naturalOffsets[i] : 0;
            el.style.transform = shift > 0 ? `translateX(${shift}px)` : '';
        });
    }
    let _ticking = false;
    wrap.addEventListener('scroll', () => {
        if (!_ticking) { requestAnimationFrame(() => { _updatePositions(); _ticking = false; }); _ticking = true; }
    }, { passive: true });
    wrap._refreshStickyColumns = _captureStickyOffsets;
    setTimeout(_captureStickyOffsets, 300);
}

window.addEventListener('beforeunload', () => { isPageUnloading = true; if (eventSource) { eventSource.close(); eventSource = null; isTracking = false; } });
window.addEventListener('pagehide', () => { isPageUnloading = true; if (eventSource) { eventSource.close(); eventSource = null; isTracking = false; } });
document.addEventListener('visibilitychange', () => {
    if (document.hidden) {
        if (eventSource && isTracking) { eventSource.close(); eventSource = null; isTracking = false; }
    } else {
        if (!isPageUnloading && !isTracking && eventSource === null) { reconnectAttempts = 0; startAutoTracking(); }
    }
});
window.addEventListener('focus', () => { if (isPageUnloading) isPageUnloading = false; });

function updateLayout() {
    const main = document.getElementById('logRowEmbMain');
    if (!main) return;
    const rows = [];
    if (!document.getElementById('logRowEmbStats').classList.contains('hidden')) rows.push('auto');
    if (!document.getElementById('logRowEmbTable').classList.contains('hidden')) rows.push('1fr');
    rows.push('auto');
    main.style.gridTemplateRows = rows.join(' ');
}

function toggleStatsDisplay() {
    const row = document.getElementById('logRowEmbStats');
    const btn = document.getElementById('toggleStatsBtn');
    if (!row || !btn) return;
    row.classList.toggle('hidden');
    btn.textContent = row.classList.contains('hidden') ? 'Show Stats' : 'Hide Stats';
    updateLayout();
}

function toggleDataDisplay() {
    const rowTable      = document.getElementById('logRowEmbTable');
    const rowPagination = document.getElementById('logRowEmbPagination');
    const btn           = document.getElementById('toggleDataBtn');
    if (!rowTable || !btn) return;
    rowTable.classList.toggle('hidden');
    if (rowPagination) rowPagination.classList.toggle('hidden');
    btn.textContent = rowTable.classList.contains('hidden') ? 'Show Table' : 'Hide Table';
    updateLayout();
}

function initDateRangePicker() {
    try {
        $('#dateRangePicker').daterangepicker({
            opens: 'left',
            locale: { format: 'YYYY-MM-DD', separator: ' ~ ', applyLabel: 'Apply', cancelLabel: 'Cancel', daysOfWeek: ['Sun','Mon','Tue','Wed','Thu','Fri','Sat'], monthNames: ['Jan','Feb','Mar','Apr','May','Jun','Jul','Aug','Sep','Oct','Nov','Dec'], firstDay: 0 },
            ranges: { 'today': [moment(), moment()], 'yesterday': [moment().subtract(1,'days'), moment().subtract(1,'days')], 'last week': [moment().subtract(6,'days'), moment()], 'last month': [moment().subtract(29,'days'), moment()] },
            startDate: moment().startOf('day'), endDate: moment().endOf('day'), showDropdowns: true, alwaysShowCalendars: true
        }, function (start, end) { handleDateRangeChange(start, end); });
        $('#dateRangePicker').val(moment().format('YYYY-MM-DD') + ' ~ ' + moment().format('YYYY-MM-DD'));
    } catch (e) { console.error('DateRangePicker init error:', e); }
}

async function initFilterSystem() {
    await loadFactoryOptions();
    await loadLineOptions();
    await loadMachineOptions();
}

async function loadFactoryOptions() {
    try {
        const res = await fetch('../manage/proc/factory.php?status_filter=Y').then(r => r.json());
        if (!res.success || !res.data) return;
        const sel = document.getElementById('factoryFilterSelect');
        sel.innerHTML = '<option value="">All Factory</option>';
        const factories = res.data.filter(f => Number(f.idx) !== 99);
        factories.forEach(f => { sel.innerHTML += `<option value="${f.idx}">${f.factory_name}</option>`; });
        sel.addEventListener('change', async (e) => {
            document.getElementById('factoryLineMachineFilterSelect').innerHTML = '<option value="">All Machine</option>';
            document.getElementById('factoryLineMachineFilterSelect').disabled = true;
            await updateLineOptions(e.target.value);
            await restartRealTimeMonitoring();
        });
        if (factories.length === 1) { sel.value = factories[0].idx; sel.dispatchEvent(new Event('change')); }
    } catch (e) { console.error('Factory options error:', e); }
}

async function loadLineOptions() {
    try {
        const res = await fetch('../manage/proc/line.php?status_filter=Y').then(r => r.json());
        if (!res.success || !res.data) return;
        const sel = document.getElementById('factoryLineFilterSelect');
        sel.innerHTML = '<option value="">All Line</option>';
        res.data.forEach(l => { sel.innerHTML += `<option value="${l.idx}">${l.line_name}</option>`; });
        sel.addEventListener('change', async (e) => {
            await updateMachineOptions(document.getElementById('factoryFilterSelect').value, e.target.value);
            await restartRealTimeMonitoring();
        });
    } catch (e) { console.error('Line options error:', e); }
}

async function loadMachineOptions() {
    try {
        const res = await fetch('../manage/proc/machine.php?type_filter=E').then(r => r.json());
        if (!res.success || !res.data) return;
        const sel = document.getElementById('factoryLineMachineFilterSelect');
        sel.innerHTML = '<option value="">All Machine</option>';
        res.data.forEach(m => { sel.innerHTML += `<option value="${m.idx}">${m.machine_no} (${m.machine_model_name || 'No Model'})</option>`; });
    } catch (e) { console.error('Machine options error:', e); }
}

async function updateLineOptions(factoryId) {
    const sel = document.getElementById('factoryLineFilterSelect');
    sel.disabled = true;
    try {
        const url = '../manage/proc/line.php?status_filter=Y' + (factoryId ? '&factory_filter=' + factoryId : '');
        const res = await fetch(url).then(r => r.json());
        sel.innerHTML = '<option value="">All Line</option>';
        if (res.success) res.data.forEach(l => { sel.innerHTML += `<option value="${l.idx}">${l.line_name}</option>`; });
        sel.disabled = false;
        if (factoryId) {
            document.getElementById('factoryLineMachineFilterSelect').disabled = false;
            updateMachineOptions(factoryId, '');
        }
    } catch (e) { sel.innerHTML = '<option value="">All Line</option>'; sel.disabled = false; }
}

async function updateMachineOptions(factoryId, lineId) {
    const sel = document.getElementById('factoryLineMachineFilterSelect');
    sel.disabled = true;
    try {
        const params = new URLSearchParams({ type_filter: 'E' });
        if (factoryId) params.append('factory_filter', factoryId);
        if (lineId)    params.append('line_filter', lineId);
        const res = await fetch('../manage/proc/machine.php?' + params.toString()).then(r => r.json());
        sel.innerHTML = '<option value="">All Machine</option>';
        if (res.success) res.data.forEach(m => { sel.innerHTML += `<option value="${m.idx}">${m.machine_no} (${m.machine_model_name || 'No Model'})</option>`; });
        sel.disabled = false;
    } catch (e) { sel.innerHTML = '<option value="">All Machine</option>'; sel.disabled = false; }
}

function initColumnToggle() {
    const btn      = document.getElementById('columnToggleBtn');
    const dropdown = document.getElementById('columnToggleDropdown');
    if (!btn || !dropdown) return;
    dropdown.innerHTML = '';
    dropdown.insertAdjacentHTML('beforeend', `<div style="display:flex;justify-content:space-between;align-items:center;margin-bottom:6px;padding-bottom:4px;border-bottom:1px solid var(--sap-border-subtle);"><strong style="color:var(--sap-text-primary);font-size:var(--sap-font-size-sm);">Show/Hide Columns</strong><button type="button" class="col-toggle-close" style="background:none;border:none;cursor:pointer;color:var(--sap-text-secondary);font-size:16px;padding:2px;" title="Close">&#10005;</button></div>`);
    columnConfig.forEach(col => {
        const label = document.createElement('label');
        const checkbox = document.createElement('input');
        checkbox.type = 'checkbox'; checkbox.checked = col.visible; checkbox.dataset.columnKey = col.key;
        label.appendChild(checkbox); label.appendChild(document.createTextNode(' ' + col.label));
        dropdown.appendChild(label);
        checkbox.addEventListener('change', (e) => { col.visible = e.target.checked; renderTableHeader(); updateTableFromAPI(rowEmbData); });
    });
    btn.addEventListener('click', (e) => { e.stopPropagation(); dropdown.classList.toggle('show'); });
    dropdown.addEventListener('click', (e) => { if (e.target.classList.contains('col-toggle-close')) { dropdown.classList.remove('show'); return; } e.stopPropagation(); });
    document.addEventListener('click', (e) => { if (!btn.contains(e.target) && !dropdown.contains(e.target)) dropdown.classList.remove('show'); });
}

function renderTableHeader() {
    const headerRow = document.getElementById('tableHeaderRow');
    if (!headerRow) return;
    headerRow.innerHTML = '';
    columnConfig.forEach(col => {
        const th = document.createElement('th');
        th.textContent = col.label; th.dataset.columnKey = col.key;
        if (!col.visible) th.classList.add('hidden-column');
        if (col.key === 'machine_no') th.classList.add('sticky-column');
        headerRow.appendChild(th);
    });
}

function setupEventListeners() {
    const bind = (id, fn) => { const el = document.getElementById(id); if (el) el.addEventListener('click', fn); };
    const bindChange = (id, fn) => { const el = document.getElementById(id); if (el) el.addEventListener('change', fn); };
    bind('excelDownloadBtn', exportData);
    bind('refreshBtn', refreshData);
    bind('toggleStatsBtn', toggleStatsDisplay);
    bind('toggleDataBtn', toggleDataDisplay);
    bindChange('timeRangeSelect', handleTimeRangeChange);
    bindChange('factoryLineMachineFilterSelect', () => restartRealTimeMonitoring());
    bindChange('shiftSelect', () => restartRealTimeMonitoring());
}

async function loadInitialData() { displayEmptyState(); }

function displayEmptyState() {
    ['embTotalOutput','embTotalThreadBreak','embMotorRunRate','embAvgCycleTime','embActiveMachines','embTodayOutput'].forEach(id => {
        const el = document.getElementById(id); if (el) el.textContent = '-';
    });
    const tbody = document.getElementById('oeeDataBody');
    const visibleCount = columnConfig.filter(c => c.visible).length;
    if (tbody) tbody.innerHTML = `<tr><td colspan="${visibleCount}" class="data-table-centered"><div class="fiori-alert fiori-alert--info"><strong>Information:</strong> Loading EMB row data log. Please wait...</div></td></tr>`;
}

function getFilterParams() {
    const dateRange = $('#dateRangePicker').val();
    let startDate = '', endDate = '';
    if (dateRange && dateRange.includes(' ~ ')) [startDate, endDate] = dateRange.split(' ~ ');
    return {
        factory_filter: document.getElementById('factoryFilterSelect').value,
        line_filter:    document.getElementById('factoryLineFilterSelect').value,
        machine_filter: document.getElementById('factoryLineMachineFilterSelect').value,
        shift_filter:   document.getElementById('shiftSelect').value,
        start_date:     startDate,
        end_date:       endDate,
        limit: 1000
    };
}

async function startAutoTracking() {
    if (isTracking) return;
    const params = new URLSearchParams(getFilterParams());
    eventSource = new EventSource('proc/log_oee_row_emb_stream_2.php?' + params.toString());
    setupSSEEventListeners();
    isTracking = true;
    const el = document.getElementById('connectionStatus');
    if (el) el.textContent = 'EMB Row Data Log Real-time Connected';
}

function setupSSEEventListeners() {
    eventSource.onerror = function () { isTracking = false; attemptReconnection(); };
    eventSource.addEventListener('connected', function () {
        reconnectAttempts = 0;
        const el = document.getElementById('connectionStatus');
        if (el) el.textContent = 'EMB row data log system connected';
    });
    eventSource.addEventListener('row_emb_data', function (e) {
        const data = JSON.parse(e.data);
        stats      = data.stats;
        rowEmbData = data.emb_data;
        updateStatCardsFromAPI(stats);
        updateTableFromAPI(rowEmbData);
        const el = document.getElementById('lastUpdateTime');
        if (el) el.textContent = 'Last updated: ' + data.timestamp;
    });
    eventSource.addEventListener('heartbeat', function (e) {
        const data = JSON.parse(e.data);
        const el   = document.getElementById('connectionStatus');
        if (el) el.textContent = 'Connection maintained (Active: ' + data.active_machines + ')';
    });
    eventSource.addEventListener('disconnected', function () {
        const el = document.getElementById('connectionStatus');
        if (el) el.textContent = 'EMB Row Data Log System Connection Closed';
    });
}

function stopTracking() {
    if (!isTracking) return;
    if (eventSource) { eventSource.close(); eventSource = null; }
    isTracking = false;
    const el = document.getElementById('connectionStatus');
    if (el) el.textContent = 'EMB Row Data Log System Connection Closed';
}

async function restartRealTimeMonitoring() {
    currentPage = 1;
    if (isTracking) { stopTracking(); await new Promise(r => setTimeout(r, 100)); }
    reconnectAttempts = 0;
    await startAutoTracking();
}

async function attemptReconnection() {
    if (isPageUnloading || reconnectAttempts >= maxReconnectAttempts) return;
    reconnectAttempts++;
    const delay = Math.min(1000 * Math.pow(2, reconnectAttempts - 1), 10000);
    if (eventSource) { eventSource.close(); eventSource = null; }
    setTimeout(async () => { if (isPageUnloading) return; try { await startAutoTracking(); } catch (e) { attemptReconnection(); } }, delay);
}

function updateStatCardsFromAPI(s) {
    if (!s) return;
    const map = {
        embTotalOutput:      (s.total_actual_output || 0).toLocaleString(),
        embTotalThreadBreak: (s.total_thread_breakage || 0).toLocaleString(),
        embMotorRunRate:     parseFloat(s.motor_run_rate || 0).toFixed(1) + '%',
        embAvgCycleTime:     parseFloat(s.avg_cycle_time || 0).toFixed(1),
        embActiveMachines:   (s.active_machines || 0).toLocaleString(),
        embTodayOutput:      (s.today_output || 0).toLocaleString()
    };
    Object.keys(map).forEach(id => { const el = document.getElementById(id); if (el) el.textContent = map[id]; });
}

function updateTableFromAPI(list) {
    const tbody = document.getElementById('oeeDataBody');
    if (!tbody) return;
    totalItems = list.length;
    if (list.length === 0) {
        const visibleCount = columnConfig.filter(c => c.visible).length;
        tbody.innerHTML = `<tr><td colspan="${visibleCount}" class="data-table-centered"><div class="fiori-alert fiori-alert--info"><strong>Information:</strong> No EMB row data matching the selected conditions.</div></td></tr>`;
        renderPagination();
        return;
    }
    const start = (currentPage - 1) * itemsPerPage;
    const paged = list.slice(start, start + itemsPerPage);
    tbody.innerHTML = '';
    paged.forEach(row => {
        const tr = document.createElement('tr');
        columnConfig.forEach(col => {
            const td = document.createElement('td');
            td.dataset.columnKey = col.key;
            if (!col.visible) td.classList.add('hidden-column');
            if (col.key === 'machine_no') td.classList.add('sticky-column');
            let value = (row[col.key] !== undefined && row[col.key] !== null) ? row[col.key] : '-';
            if (col.key === 'thread_breakage' && value !== '-' && parseInt(value) > 0) {
                td.style.color = 'var(--sap-status-error)';
                td.style.fontWeight = 'var(--sap-font-weight-semibold)';
            }
            td.textContent = value;
            tr.appendChild(td);
        });
        tbody.appendChild(tr);
    });
    renderPagination();
    document.querySelector('.oee-table-wrap')?._refreshStickyColumns?.();
}

function renderPagination() {
    const container = document.getElementById('pagination-controls');
    if (!container) return;
    const totalPages = Math.ceil(totalItems / itemsPerPage);
    if (totalPages <= 1) { container.innerHTML = ''; return; }
    const s = totalItems === 0 ? 0 : (currentPage - 1) * itemsPerPage + 1;
    const e = Math.min(currentPage * itemsPerPage, totalItems);
    let html = `<div class="fiori-pagination__info">${s}-${e} / ${totalItems} items</div>`;
    html += '<div class="fiori-pagination__buttons">';
    html += `<button class="fiori-pagination__button${currentPage === 1 ? ' fiori-pagination__button--disabled' : ''}" onclick="changePage(${currentPage - 1})" ${currentPage === 1 ? 'disabled' : ''}>&larr;</button>`;
    const sp = Math.max(1, currentPage - 2), ep = Math.min(totalPages, currentPage + 2);
    if (sp > 1) { html += `<button class="fiori-pagination__button" onclick="changePage(1)">1</button>`; if (sp > 2) html += `<span class="fiori-pagination__ellipsis">...</span>`; }
    for (let i = sp; i <= ep; i++) html += `<button class="fiori-pagination__button${i === currentPage ? ' fiori-pagination__button--active' : ''}" onclick="changePage(${i})">${i}</button>`;
    if (ep < totalPages) { if (ep < totalPages - 1) html += `<span class="fiori-pagination__ellipsis">...</span>`; html += `<button class="fiori-pagination__button" onclick="changePage(${totalPages})">${totalPages}</button>`; }
    html += `<button class="fiori-pagination__button${currentPage === totalPages ? ' fiori-pagination__button--disabled' : ''}" onclick="changePage(${currentPage + 1})" ${currentPage === totalPages ? 'disabled' : ''}>&rarr;</button>`;
    html += '</div>';
    container.innerHTML = html;
}

function changePage(p) {
    const total = Math.ceil(totalItems / itemsPerPage);
    if (p < 1 || p > total || p === currentPage) return;
    currentPage = p;
    updateTableFromAPI(rowEmbData);
}

function handleDateRangeChange(start, end) {
    const sel = document.getElementById('timeRangeSelect');
    if (sel) {
        let opt = sel.querySelector('option[value="custom"]');
        if (!opt) { opt = document.createElement('option'); opt.value = 'custom'; opt.textContent = 'Custom Selection'; sel.appendChild(opt); }
        sel.value = 'custom';
    }
    if (isTracking) { stopTracking(); setTimeout(() => startAutoTracking(), 1000); } else { startAutoTracking(); }
}

function handleTimeRangeChange(event) {
    const timeRange = event.target.value;
    if (timeRange !== 'custom') {
        let startDate, endDate;
        switch (timeRange) {
            case 'today':     startDate = moment().startOf('day');                       endDate = moment().endOf('day');        break;
            case 'yesterday': startDate = moment().subtract(1,'days').startOf('day');    endDate = moment().subtract(1,'days').endOf('day'); break;
            case '1w':        startDate = moment().subtract(7,'days').startOf('day');    endDate = moment().endOf('day');        break;
            case '1m':        startDate = moment().subtract(30,'days').startOf('day');   endDate = moment().endOf('day');        break;
            default:          startDate = moment().startOf('day');                       endDate = moment().endOf('day');
        }
        $('#dateRangePicker').data('daterangepicker').setStartDate(startDate);
        $('#dateRangePicker').data('daterangepicker').setEndDate(endDate);
        $('#dateRangePicker').val(startDate.format('YYYY-MM-DD') + ' ~ ' + endDate.format('YYYY-MM-DD'));
    }
    restartRealTimeMonitoring();
}

function exportData() {
    if (!rowEmbData || rowEmbData.length === 0) return;
    try {
        const filters = getFilterParams();
        const params  = new URLSearchParams();
        if (filters.factory_filter) params.append('factory_filter', filters.factory_filter);
        if (filters.line_filter)    params.append('line_filter',    filters.line_filter);
        if (filters.machine_filter) params.append('machine_filter', filters.machine_filter);
        if (filters.shift_filter)   params.append('shift_filter',   filters.shift_filter);
        if (filters.start_date)     params.append('start_date',     filters.start_date);
        if (filters.end_date)       params.append('end_date',       filters.end_date);
        window.open('proc/log_oee_row_emb_export_2.php?' + params.toString(), '_blank');
    } catch (e) { console.error('Export error:', e); }
}

async function refreshData() {
    if (isTracking) { stopTracking(); setTimeout(() => startAutoTracking(), 1000); } else { await startAutoTracking(); }
}
