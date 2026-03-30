// log_oee_row_2.js — Row Grid Layout version (data_oee_2 model)

let eventSource = null;
let isTracking = false;
let oeeData = [];
let reconnectAttempts = 0;
let maxReconnectAttempts = 3;
let stats = {};
let isPageUnloading = false;

let currentPage = 1;
let itemsPerPage = 20;
let totalItems = 0;

// 컬럼 설정 (data_oee_rows 테이블 전체 컬럼)
const columnConfig = [
    { key: 'idx',                  label: 'idx',                  visible: true  },
    { key: 'work_date',            label: 'work_date',            visible: true  },
    { key: 'time_update',          label: 'time_update',          visible: true  },
    { key: 'shift_idx',            label: 'shift_idx',            visible: true  },
    { key: 'factory_idx',          label: 'factory_idx',          visible: false },
    { key: 'factory_name',         label: 'factory_name',         visible: true  },
    { key: 'line_idx',             label: 'line_idx',             visible: false },
    { key: 'line_name',            label: 'line_name',            visible: true  },
    { key: 'mac',                  label: 'mac',                  visible: true  },
    { key: 'machine_idx',          label: 'machine_idx',          visible: false },
    { key: 'machine_no',           label: 'machine_no',           visible: true  },
    { key: 'process_name',         label: 'process_name',         visible: true  },
    { key: 'planned_work_time',    label: 'planned_work_time',    visible: true  },
    { key: 'runtime',              label: 'runtime',              visible: true  },
    { key: 'productive_runtime',   label: 'productive_runtime',   visible: true  },
    { key: 'downtime',             label: 'downtime',             visible: true  },
    { key: 'availabilty_rate',     label: 'availabilty_rate',     visible: true  },
    { key: 'target_line_per_day',  label: 'target_line_per_day',  visible: true  },
    { key: 'target_line_per_hour', label: 'target_line_per_hour', visible: true  },
    { key: 'target_mc_per_day',    label: 'target_mc_per_day',    visible: true  },
    { key: 'target_mc_per_hour',   label: 'target_mc_per_hour',   visible: true  },
    { key: 'cycletime',            label: 'cycletime',            visible: true  },
    { key: 'pair_info',            label: 'pair_info',            visible: true  },
    { key: 'pair_count',           label: 'pair_count',           visible: true  },
    { key: 'theoritical_output',   label: 'theoritical_output',   visible: true  },
    { key: 'actual_output',        label: 'actual_output',        visible: true  },
    { key: 'productivity_rate',    label: 'productivity_rate',    visible: true  },
    { key: 'defective',            label: 'defective',            visible: true  },
    { key: 'actual_a_grade',       label: 'actual_a_grade',       visible: true  },
    { key: 'quality_rate',         label: 'quality_rate',         visible: true  },
    { key: 'oee',                  label: 'oee',                  visible: true  },
    { key: 'reg_date',             label: 'reg_date',             visible: true  },
    { key: 'work_hour',            label: 'work_hour',            visible: true  }
];

document.addEventListener('DOMContentLoaded', async function () {
    initDateRangePicker();
    await initFilterSystem();
    initColumnToggle();
    setupEventListeners();
    renderTableHeader();
    updateLayout(); // 초기 grid-template-rows 설정
    initStickyColumnsScroll(); // JS 기반 컬럼 고정 초기화
    await loadInitialData();
    await startAutoTracking();
});

// ─── JS sticky 컬럼 스크롤 동기화 ──────────────────────────
// overflow:clip/hidden 부모 구조에서 CSS sticky가 작동하지 않으므로
// scroll 이벤트로 transform을 적용해 컬럼 고정 효과를 구현
function initStickyColumnsScroll() {
    const wrap = document.querySelector('.oee-table-wrap');
    if (!wrap) return;

    let _stickyEls = [];
    let _naturalOffsets = [];

    function _captureStickyOffsets() {
        const prevScroll = wrap.scrollLeft;
        if (prevScroll !== 0) wrap.scrollLeft = 0;

        _stickyEls = Array.from(wrap.querySelectorAll('.sticky-column'));
        _naturalOffsets = _stickyEls.map(el => el.offsetLeft);

        if (prevScroll !== 0) wrap.scrollLeft = prevScroll;
        _updatePositions();
    }

    function _updatePositions() {
        const sl = wrap.scrollLeft;
        _stickyEls.forEach((el, i) => {
            const nat = _naturalOffsets[i] || 0;
            const shift = sl > nat ? sl - nat : 0;
            el.style.transform = shift > 0 ? `translateX(${shift}px)` : '';
        });
    }

    let _ticking = false;
    wrap.addEventListener('scroll', () => {
        if (!_ticking) {
            requestAnimationFrame(() => { _updatePositions(); _ticking = false; });
            _ticking = true;
        }
    }, { passive: true });

    // 테이블 렌더링 후 외부에서 호출할 수 있도록 노출
    wrap._refreshStickyColumns = _captureStickyOffsets;
    // 초기 offset 측정 (데이터 로드 후 자동 실행)
    setTimeout(_captureStickyOffsets, 300);
}

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
    const main = document.getElementById('logRowMain');
    if (!main) return;

    const rows = [];
    if (!document.getElementById('logRowStats').classList.contains('hidden'))  rows.push('auto');
    if (!document.getElementById('logRowTable').classList.contains('hidden'))  rows.push('1fr');
    rows.push('auto'); // pagination 항상

    main.style.gridTemplateRows = rows.join(' ');
}

// ─── 토글 함수 ────────────────────────────────────────────
function toggleStatsDisplay() {
    const row = document.getElementById('logRowStats');
    const btn = document.getElementById('toggleStatsBtn');
    if (!row || !btn) return;

    row.classList.toggle('hidden');
    btn.textContent = row.classList.contains('hidden') ? 'Show Stats' : 'Hide Stats';
    updateLayout();
}

function toggleDataDisplay() {
    const rowTable      = document.getElementById('logRowTable');
    const rowPagination = document.getElementById('logRowPagination');
    const btn           = document.getElementById('toggleDataBtn');
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
        const res = await fetch('../manage/proc/factory.php?status_filter=Y').then(r => r.json());
        if (!res.success || !res.data) return;
        const sel = document.getElementById('factoryFilterSelect');
        sel.innerHTML = '<option value="">All Factory</option>';
        res.data.forEach(f => {
            sel.innerHTML += `<option value="${f.idx}">${f.factory_name}</option>`;
        });
        sel.addEventListener('change', async (e) => {
            document.getElementById('factoryLineMachineFilterSelect').innerHTML = '<option value="">All Machine</option>';
            document.getElementById('factoryLineMachineFilterSelect').disabled = true;
            await updateLineOptions(e.target.value);
            await restartRealTimeMonitoring();
        });
    } catch (e) { console.error('Factory options error:', e); }
}

async function loadLineOptions() {
    try {
        const res = await fetch('../manage/proc/line.php?status_filter=Y').then(r => r.json());
        if (!res.success || !res.data) return;
        const sel = document.getElementById('factoryLineFilterSelect');
        sel.innerHTML = '<option value="">All Line</option>';
        res.data.forEach(l => {
            sel.innerHTML += `<option value="${l.idx}">${l.line_name}</option>`;
        });
        sel.addEventListener('change', async (e) => {
            await updateMachineOptions(document.getElementById('factoryFilterSelect').value, e.target.value);
            await restartRealTimeMonitoring();
        });
    } catch (e) { console.error('Line options error:', e); }
}

async function loadMachineOptions() {
    try {
        const res = await fetch('../manage/proc/machine.php').then(r => r.json());
        if (!res.success || !res.data) return;
        const sel = document.getElementById('factoryLineMachineFilterSelect');
        sel.innerHTML = '<option value="">All Machine</option>';
        res.data.forEach(m => {
            sel.innerHTML += `<option value="${m.idx}">${m.machine_no} (${m.machine_model_name || 'No Model'})</option>`;
        });
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
        const params = new URLSearchParams();
        if (factoryId) params.append('factory_filter', factoryId);
        if (lineId)    params.append('line_filter', lineId);
        const url = '../manage/proc/machine.php' + (params.toString() ? '?' + params.toString() : '');
        const res = await fetch(url).then(r => r.json());
        sel.innerHTML = '<option value="">All Machine</option>';
        if (res.success) res.data.forEach(m => { sel.innerHTML += `<option value="${m.idx}">${m.machine_no} (${m.machine_model_name || 'No Model'})</option>`; });
        sel.disabled = false;
    } catch (e) { sel.innerHTML = '<option value="">All Machine</option>'; sel.disabled = false; }
}

// ─── 컬럼 토글 ────────────────────────────────────────────
function initColumnToggle() {
    const btn      = document.getElementById('columnToggleBtn');
    const dropdown = document.getElementById('columnToggleDropdown');
    if (!btn || !dropdown) return;

    dropdown.innerHTML = '';

    // 드롭다운 헤더
    dropdown.insertAdjacentHTML('beforeend', `
        <div style="display:flex;justify-content:space-between;align-items:center;margin-bottom:6px;padding-bottom:4px;border-bottom:1px solid var(--sap-border-subtle);">
            <strong style="color:var(--sap-text-primary);font-size:var(--sap-font-size-sm);">Show/Hide Columns</strong>
            <button type="button" class="col-toggle-close" style="background:none;border:none;cursor:pointer;color:var(--sap-text-secondary);font-size:16px;padding:2px;" title="Close">&#10005;</button>
        </div>
    `);

    // 체크박스 생성
    columnConfig.forEach(col => {
        const label    = document.createElement('label');
        const checkbox = document.createElement('input');
        checkbox.type              = 'checkbox';
        checkbox.checked           = col.visible;
        checkbox.dataset.columnKey = col.key;
        label.appendChild(checkbox);
        label.appendChild(document.createTextNode(' ' + col.label));
        dropdown.appendChild(label);

        checkbox.addEventListener('change', (e) => {
            col.visible = e.target.checked;
            renderTableHeader();
            updateTableFromAPI(oeeData);
        });
    });

    // 토글
    btn.addEventListener('click', (e) => {
        e.stopPropagation();
        dropdown.classList.toggle('show');
    });

    // 내부 클릭 전파 차단 + 닫기 버튼
    dropdown.addEventListener('click', (e) => {
        if (e.target.classList.contains('col-toggle-close')) {
            dropdown.classList.remove('show');
            return;
        }
        e.stopPropagation();
    });

    // 외부 클릭 닫기
    document.addEventListener('click', (e) => {
        if (!btn.contains(e.target) && !dropdown.contains(e.target)) {
            dropdown.classList.remove('show');
        }
    });
}

// ─── 테이블 헤더 렌더링 ───────────────────────────────────
function renderTableHeader() {
    const headerRow = document.getElementById('tableHeaderRow');
    if (!headerRow) return;
    headerRow.innerHTML = '';

    columnConfig.forEach(col => {
        const th = document.createElement('th');
        th.textContent         = col.label;
        th.dataset.columnKey   = col.key;
        if (!col.visible) th.classList.add('hidden-column');
        if (col.key === 'machine_no') th.classList.add('sticky-column');
        headerRow.appendChild(th);
    });
}

// ─── 이벤트 리스너 ────────────────────────────────────────
function setupEventListeners() {
    const bind = (id, fn) => { const el = document.getElementById(id); if (el) el.addEventListener('click', fn); };
    const bindChange = (id, fn) => { const el = document.getElementById(id); if (el) el.addEventListener('change', fn); };

    bind('excelDownloadBtn', exportData);
    bind('refreshBtn',       refreshData);
    bind('toggleStatsBtn',   toggleStatsDisplay);
    bind('toggleDataBtn',    toggleDataDisplay);

    bindChange('timeRangeSelect',               handleTimeRangeChange);
    bindChange('factoryLineMachineFilterSelect', () => restartRealTimeMonitoring());
    bindChange('shiftSelect',                   () => restartRealTimeMonitoring());
}

// ─── 초기 데이터 ──────────────────────────────────────────
async function loadInitialData() {
    displayEmptyState();
}

function displayEmptyState() {
    ['overallOee', 'availability', 'performance', 'quality', 'currentShiftOee', 'previousDayOee'].forEach(id => {
        const el = document.getElementById(id);
        if (el) el.textContent = '-';
    });
    const tbody         = document.getElementById('oeeDataBody');
    const visibleCount  = columnConfig.filter(c => c.visible).length;
    if (tbody) tbody.innerHTML = `
        <tr>
            <td colspan="${visibleCount}" class="data-table-centered">
                <div class="fiori-alert fiori-alert--info">
                    <strong>Information:</strong> Loading OEE row data log. Please wait...
                </div>
            </td>
        </tr>`;
}

// ─── SSE / 데이터 로딩 ────────────────────────────────────
function getFilterParams() {
    const dateRange = $('#dateRangePicker').val();
    let startDate = '', endDate = '';
    if (dateRange && dateRange.includes(' ~ ')) {
        [startDate, endDate] = dateRange.split(' ~ ');
    }
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
    eventSource   = new EventSource('proc/log_oee_row_stream_2.php?' + params.toString());
    setupSSEEventListeners();
    isTracking = true;
    const el = document.getElementById('connectionStatus');
    if (el) el.textContent = 'OEE Row Data Real-time Connected';
}

function setupSSEEventListeners() {
    eventSource.onerror = function () {
        isTracking = false;
        attemptReconnection();
    };

    eventSource.addEventListener('connected', function (e) {
        reconnectAttempts = 0;
        const el = document.getElementById('connectionStatus');
        if (el) el.textContent = 'OEE row data system connected';
    });

    eventSource.addEventListener('oee_data', function (e) {
        const data = JSON.parse(e.data);
        stats   = data.stats;
        oeeData = data.oee_data;
        window.oeeData = data.oee_data;
        updateStatCardsFromAPI(stats);
        updateTableFromAPI(oeeData);
        const el = document.getElementById('lastUpdateTime');
        if (el) el.textContent = 'Last updated: ' + data.timestamp;
    });

    eventSource.addEventListener('heartbeat', function (e) {
        const data = JSON.parse(e.data);
        const el   = document.getElementById('connectionStatus');
        if (el) el.textContent = 'Connection maintained (Active records: ' + data.active_machines + ')';
    });

    eventSource.addEventListener('disconnected', function () {
        const el = document.getElementById('connectionStatus');
        if (el) el.textContent = 'OEE Row Data System Connection Closed';
    });
}

function stopTracking() {
    if (!isTracking) return;
    if (eventSource) { eventSource.close(); eventSource = null; }
    isTracking = false;
    const el = document.getElementById('connectionStatus');
    if (el) el.textContent = 'OEE Row Data System Connection Closed';
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
    setTimeout(async () => {
        if (isPageUnloading) return;
        try { await startAutoTracking(); } catch (e) { attemptReconnection(); }
    }, delay);
}

// ─── 데이터 업데이트 ──────────────────────────────────────
function formatPercentage(value) {
    const n = parseFloat(value);
    if (isNaN(n)) return '0%';
    return (n % 1 === 0 ? Math.floor(n) : n) + '%';
}

function updateStatCardsFromAPI(s) {
    if (!s) return;
    const map = {
        overallOee:      formatPercentage(s.overall_oee    || 0),
        availability:    formatPercentage(s.availability   || 0),
        performance:     formatPercentage(s.performance    || 0),
        quality:         formatPercentage(s.quality        || 0),
        currentShiftOee: formatPercentage(s.current_shift_oee  || 0),
        previousDayOee:  formatPercentage(s.previous_day_oee   || 0)
    };
    Object.keys(map).forEach(id => {
        const el = document.getElementById(id);
        if (el) el.textContent = map[id];
    });
}

function updateTableFromAPI(list) {
    const tbody = document.getElementById('oeeDataBody');
    if (!tbody) return;
    totalItems = list.length;

    if (list.length === 0) {
        const visibleCount = columnConfig.filter(c => c.visible).length;
        tbody.innerHTML = `
            <tr>
                <td colspan="${visibleCount}" class="data-table-centered">
                    <div class="fiori-alert fiori-alert--info">
                        <strong>Information:</strong> No OEE row data matching the selected conditions.
                    </div>
                </td>
            </tr>`;
        renderPagination();
        return;
    }

    const start  = (currentPage - 1) * itemsPerPage;
    const paged  = list.slice(start, start + itemsPerPage);
    tbody.innerHTML = '';

    paged.forEach(oee => {
        const row = document.createElement('tr');
        columnConfig.forEach(col => {
            const td  = document.createElement('td');
            td.dataset.columnKey = col.key;
            if (!col.visible) td.classList.add('hidden-column');
            if (col.key === 'machine_no') td.classList.add('sticky-column');

            let value = (oee[col.key] !== undefined && oee[col.key] !== null) ? oee[col.key] : '-';

            // 비율 컬럼 배지 렌더링
            if (['oee', 'availabilty_rate', 'productivity_rate', 'quality_rate'].includes(col.key) && value !== '-') {
                let badgeClass = 'fiori-badge';
                const num = parseFloat(value);
                if (col.key === 'oee') {
                    if (num >= 85)      badgeClass += ' fiori-badge--success';
                    else if (num >= 70) badgeClass += ' fiori-badge--warning';
                    else                badgeClass += ' fiori-badge--error';
                } else if (col.key === 'availabilty_rate') {
                    badgeClass += ' fiori-badge--success';
                } else if (col.key === 'productivity_rate') {
                    badgeClass += ' fiori-badge--info';
                } else if (col.key === 'quality_rate') {
                    badgeClass += ' fiori-badge--warning';
                }
                td.innerHTML = `<span class="${badgeClass}">${value}%</span>`;
            } else {
                td.textContent = value;
            }

            row.appendChild(td);
        });
        tbody.appendChild(row);
    });

    renderPagination();
    // 테이블 렌더 후 sticky offset 갱신
    document.querySelector('.oee-table-wrap')?._refreshStickyColumns?.();
}

// ─── 페이지네이션 ─────────────────────────────────────────
function renderPagination() {
    const container  = document.getElementById('pagination-controls');
    if (!container) return;
    const totalPages = Math.ceil(totalItems / itemsPerPage);
    if (totalPages <= 1) { container.innerHTML = ''; return; }

    const s = totalItems === 0 ? 0 : (currentPage - 1) * itemsPerPage + 1;
    const e = Math.min(currentPage * itemsPerPage, totalItems);
    let html = `<div class="fiori-pagination__info">${s}-${e} / ${totalItems} items</div>`;
    html += '<div class="fiori-pagination__buttons">';
    html += `<button class="fiori-pagination__button${currentPage === 1 ? ' fiori-pagination__button--disabled' : ''}" onclick="changePage(${currentPage - 1})" ${currentPage === 1 ? 'disabled' : ''}>&larr;</button>`;

    const sp = Math.max(1, currentPage - 2);
    const ep = Math.min(totalPages, currentPage + 2);
    if (sp > 1) {
        html += `<button class="fiori-pagination__button" onclick="changePage(1)">1</button>`;
        if (sp > 2) html += `<span class="fiori-pagination__ellipsis">...</span>`;
    }
    for (let i = sp; i <= ep; i++) {
        html += `<button class="fiori-pagination__button${i === currentPage ? ' fiori-pagination__button--active' : ''}" onclick="changePage(${i})">${i}</button>`;
    }
    if (ep < totalPages) {
        if (ep < totalPages - 1) html += `<span class="fiori-pagination__ellipsis">...</span>`;
        html += `<button class="fiori-pagination__button" onclick="changePage(${totalPages})">${totalPages}</button>`;
    }
    html += `<button class="fiori-pagination__button${currentPage === totalPages ? ' fiori-pagination__button--disabled' : ''}" onclick="changePage(${currentPage + 1})" ${currentPage === totalPages ? 'disabled' : ''}>&rarr;</button>`;
    html += '</div>';
    container.innerHTML = html;
}

function changePage(p) {
    const total = Math.ceil(totalItems / itemsPerPage);
    if (p < 1 || p > total || p === currentPage) return;
    currentPage = p;
    updateTableFromAPI(oeeData);
}

// ─── 날짜/시간 처리 ───────────────────────────────────────
function handleDateRangeChange(start, end) {
    const timeRangeSelect = document.getElementById('timeRangeSelect');
    if (timeRangeSelect) {
        let customOption = timeRangeSelect.querySelector('option[value="custom"]');
        if (!customOption) {
            customOption          = document.createElement('option');
            customOption.value    = 'custom';
            customOption.textContent = 'Custom Selection';
            timeRangeSelect.appendChild(customOption);
        }
        timeRangeSelect.value = 'custom';
    }
    if (isTracking) {
        stopTracking();
        setTimeout(() => startAutoTracking(), 1000);
    } else {
        startAutoTracking();
    }
}

function handleTimeRangeChange(event) {
    const timeRange = event.target.value;
    if (timeRange !== 'custom') {
        let startDate, endDate;
        switch (timeRange) {
            case 'today':
                startDate = moment().startOf('day');
                endDate   = moment().endOf('day');
                break;
            case 'yesterday':
                startDate = moment().subtract(1, 'days').startOf('day');
                endDate   = moment().subtract(1, 'days').endOf('day');
                break;
            case '1w':
                startDate = moment().subtract(7, 'days').startOf('day');
                endDate   = moment().endOf('day');
                break;
            case '1m':
                startDate = moment().subtract(30, 'days').startOf('day');
                endDate   = moment().endOf('day');
                break;
            default:
                startDate = moment().startOf('day');
                endDate   = moment().endOf('day');
        }
        $('#dateRangePicker').data('daterangepicker').setStartDate(startDate);
        $('#dateRangePicker').data('daterangepicker').setEndDate(endDate);
        $('#dateRangePicker').val(startDate.format('YYYY-MM-DD') + ' ~ ' + endDate.format('YYYY-MM-DD'));
    }
    restartRealTimeMonitoring();
}

// ─── Export / Refresh ─────────────────────────────────────
function exportData() {
    if (!oeeData || oeeData.length === 0) return;
    try {
        const filters = getFilterParams();
        const params  = new URLSearchParams();
        if (filters.factory_filter) params.append('factory_filter', filters.factory_filter);
        if (filters.line_filter)    params.append('line_filter',    filters.line_filter);
        if (filters.machine_filter) params.append('machine_filter', filters.machine_filter);
        if (filters.shift_filter)   params.append('shift_filter',   filters.shift_filter);
        if (filters.start_date)     params.append('start_date',     filters.start_date);
        if (filters.end_date)       params.append('end_date',       filters.end_date);
        window.open('proc/log_oee_row_export_2.php?' + params.toString(), '_blank');
    } catch (e) {
        console.error('Export error:', e);
    }
}

async function refreshData() {
    if (isTracking) {
        stopTracking();
        setTimeout(() => startAutoTracking(), 1000);
    } else {
        await startAutoTracking();
    }
}
