/*
 * ============================================================
 * data_offline_2.js
 * ============================================================
 * 목적: 미수신(오프라인) 머신 모니터링 페이지 스크립트
 *
 * 주요 기능:
 *   - SSE(EventSource)를 통해 미수신 머신 목록 실시간 수신
 *   - 요약 통계 카드 4개 렌더링 (전체/활성/미수신/미연결)
 *   - 미수신 머신 테이블 렌더링 (원인 배지, 경과시간 포함)
 *   - 공장·라인 필터 및 기준 날짜 선택 지원
 *   - 페이지 숨김/언로드 시 SSE 자동 종료 및 지수 백오프 재연결
 *
 * 연관 백엔드: proc/data_offline_stream_2.php (SSE)
 * ============================================================
 */

/* ── 전역 상태 ──────────────────────────────────────────── */
let eventSource       = null;
let isTracking        = false;
let reconnectAttempts = 0;
const maxReconnect    = 3;
let isPageUnloading   = false;

/* ── 페이지네이션 ───────────────────────────────────────── */
let currentPage  = 1;
const perPage    = 15;
let allListData  = [];

/* ── 원인 분류 상수 ─────────────────────────────────────── */
const CAUSE = {
    never_connected : { label: '신규/미연결',          badgeClass: 'cause-badge--never', icon: '⚪' },
    today_no_data   : { label: '금일 미수신',           badgeClass: 'cause-badge--today', icon: '🔵' },
    uart_suspect    : { label: 'UART 케이블 점검 필요', badgeClass: 'cause-badge--uart',  icon: '🟠' },
    power_off_suspect: { label: '전원 OFF 의심',        badgeClass: 'cause-badge--power', icon: '🔴' },
};

/* ── 머신 타입 라벨 ─────────────────────────────────────── */
const MACHINE_TYPE = {
    P: { label: 'Computer Sewing', cls: 'type-badge--P' },
    E: { label: 'Embroidery',      cls: 'type-badge--E' },
};

/* ── DOMContentLoaded ───────────────────────────────────── */
document.addEventListener('DOMContentLoaded', async function () {
    setDefaultDate();
    await initFilterSystem();
    setupEventListeners();
    await startMonitoring();
});

window.addEventListener('beforeunload', cleanupSSE);
window.addEventListener('pagehide',     cleanupSSE);
window.addEventListener('focus', () => { if (isPageUnloading) isPageUnloading = false; });

document.addEventListener('visibilitychange', () => {
    if (document.hidden) {
        cleanupSSE();
    } else if (!isPageUnloading && !isTracking) {
        reconnectAttempts = 0;
        startMonitoring();
    }
});

function cleanupSSE() {
    isPageUnloading = true;
    if (eventSource) { eventSource.close(); eventSource = null; isTracking = false; }
}

/* ── 기본 날짜 설정 (오늘) ──────────────────────────────── */
function setDefaultDate() {
    const today = new Date();
    const yyyy  = today.getFullYear();
    const mm    = String(today.getMonth() + 1).padStart(2, '0');
    const dd    = String(today.getDate()).padStart(2, '0');
    const el    = document.getElementById('offlineRefDate');
    if (el) el.value = `${yyyy}-${mm}-${dd}`;
}

/* ── 필터 시스템 초기화 ─────────────────────────────────── */
async function initFilterSystem() {
    await loadFactoryOptions();
    await loadLineOptions();
}

async function loadFactoryOptions() {
    try {
        const res = await fetch('../manage/proc/factory.php?status_filter=Y').then(r => r.json());
        if (!res.success || !res.data) return;

        const sel = document.getElementById('offlineFactoryFilter');
        sel.innerHTML = '<option value="">All Factory</option>';
        const factories = res.data.filter(f => Number(f.idx) !== 99);
        factories.forEach(f => {
            sel.innerHTML += `<option value="${f.idx}">${f.factory_name}</option>`;
        });

        sel.addEventListener('change', async function () {
            document.getElementById('offlineLineFilter').innerHTML = '<option value="">All Line</option>';
            document.getElementById('offlineLineFilter').disabled = true;
            await updateLineOptions(this.value);
            restartMonitoring();
        });

        if (factories.length === 1) {
            sel.value = factories[0].idx;
            sel.dispatchEvent(new Event('change'));
        }
    } catch (e) { console.error('Factory options error:', e); }
}

async function loadLineOptions() {
    try {
        const res = await fetch('../manage/proc/line.php?status_filter=Y').then(r => r.json());
        if (!res.success || !res.data) return;

        const sel = document.getElementById('offlineLineFilter');
        sel.innerHTML = '<option value="">All Line</option>';
        res.data.forEach(l => {
            sel.innerHTML += `<option value="${l.idx}">${l.line_name}</option>`;
        });

        sel.addEventListener('change', function () {
            restartMonitoring();
        });
    } catch (e) { console.error('Line options error:', e); }
}

async function updateLineOptions(factoryId) {
    const sel = document.getElementById('offlineLineFilter');
    sel.disabled = true;
    try {
        const url = '../manage/proc/line.php?status_filter=Y' + (factoryId ? '&factory_filter=' + factoryId : '');
        const res = await fetch(url).then(r => r.json());
        sel.innerHTML = '<option value="">All Line</option>';
        if (res.success) res.data.forEach(l => {
            sel.innerHTML += `<option value="${l.idx}">${l.line_name}</option>`;
        });
        sel.disabled = false;
    } catch (e) {
        sel.innerHTML = '<option value="">All Line</option>';
        sel.disabled = false;
    }
}

/* ── 이벤트 리스너 ──────────────────────────────────────── */
function setupEventListeners() {
    const refreshBtn = document.getElementById('offlineRefreshBtn');
    if (refreshBtn) refreshBtn.addEventListener('click', restartMonitoring);

    const refDate = document.getElementById('offlineRefDate');
    if (refDate) refDate.addEventListener('change', restartMonitoring);
}

/* ── SSE 스트림 URL 생성 ────────────────────────────────── */
function buildStreamUrl() {
    const params = new URLSearchParams();
    const factory = document.getElementById('offlineFactoryFilter')?.value;
    const line    = document.getElementById('offlineLineFilter')?.value;
    const refDate = document.getElementById('offlineRefDate')?.value;

    if (factory) params.append('factory_filter', factory);
    if (line)    params.append('line_filter', line);
    if (refDate) params.append('ref_date', refDate);

    return 'proc/data_offline_stream_2.php?' + params.toString();
}

/* ── 모니터링 시작 ──────────────────────────────────────── */
async function startMonitoring() {
    if (isTracking) return;

    setConnectionStatus('connecting');

    const url = buildStreamUrl();
    eventSource = new EventSource(url);
    isTracking  = true;

    eventSource.addEventListener('connected', function (e) {
        const data = JSON.parse(e.data);
        reconnectAttempts = 0;
        setConnectionStatus('connected');
        const refDateEl = document.getElementById('currentRefDate');
        if (refDateEl) refDateEl.textContent = data.ref_date || '-';
    });

    eventSource.addEventListener('offline_data', function (e) {
        const data = JSON.parse(e.data);
        updateStats(data.stats);
        allListData = data.list || [];
        currentPage = 1;
        renderTable();
        renderPagination();
        setLastUpdate(data.timestamp);
        const refDateEl = document.getElementById('currentRefDate');
        if (refDateEl) refDateEl.textContent = data.ref_date || '-';
    });

    eventSource.addEventListener('heartbeat', function () {
        setConnectionStatus('connected');
    });

    eventSource.addEventListener('error', function () {
        setConnectionStatus('error');
        isTracking = false;
        eventSource.close();
        eventSource = null;
        scheduleReconnect();
    });

    eventSource.addEventListener('timeout', function () {
        isTracking = false;
        if (eventSource) { eventSource.close(); eventSource = null; }
        scheduleReconnect();
    });

    eventSource.onerror = function () {
        if (!isPageUnloading) {
            isTracking = false;
            if (eventSource) { eventSource.close(); eventSource = null; }
            setConnectionStatus('error');
            scheduleReconnect();
        }
    };
}

function restartMonitoring() {
    if (eventSource) { eventSource.close(); eventSource = null; }
    isTracking        = false;
    reconnectAttempts = 0;
    currentPage       = 1;
    startMonitoring();
}

function scheduleReconnect() {
    if (isPageUnloading || reconnectAttempts >= maxReconnect) return;
    const delay = Math.min(5000 * Math.pow(2, reconnectAttempts), 60000);
    reconnectAttempts++;
    setTimeout(() => {
        if (!isTracking && !isPageUnloading) startMonitoring();
    }, delay);
}

/* ── 연결 상태 표시 ─────────────────────────────────────── */
function setConnectionStatus(status) {
    const dot    = document.querySelector('.offline-status-dot');
    const label  = document.getElementById('offlineConnectionStatus');
    if (!dot || !label) return;

    const MAP = {
        connecting: { color: '#e26b0a', text: 'Connecting...' },
        connected:  { color: 'var(--sap-status-success)', text: 'Live monitoring' },
        error:      { color: 'var(--sap-status-error)',   text: 'Connection error' },
    };
    const s = MAP[status] || MAP.connecting;
    dot.style.backgroundColor = s.color;
    label.textContent = s.text;
}

/* ── 마지막 업데이트 시간 ────────────────────────────────── */
function setLastUpdate(ts) {
    const el = document.getElementById('offlineLastUpdate');
    if (el && ts) el.textContent = 'Last updated: ' + ts;
}

/* ── 통계 카드 업데이트 ─────────────────────────────────── */
function updateStats(stats) {
    if (!stats) return;
    setText('offlineStatTotal',   stats.total_active       ?? 0);
    setText('offlineStatActive',  stats.machines_with_data ?? 0);
    setText('offlineStatOffline', stats.machines_offline   ?? 0);
    setText('offlineStatNever',   stats.never_connected    ?? 0);
}

function setText(id, val) {
    const el = document.getElementById(id);
    if (el) el.textContent = val;
}

/* ── 경과 시간 포맷 ─────────────────────────────────────── */
function formatMinutesAgo(minutes) {
    if (minutes === null || minutes === undefined) return '-';
    const m = Number(minutes);
    if (m < 60)   return m + '분 전';
    if (m < 1440) return Math.floor(m / 60) + '시간 ' + (m % 60) + '분 전';
    return Math.floor(m / 1440) + '일 전';
}

function getTimeClass(minutes) {
    if (minutes === null || minutes === undefined) return '';
    const m = Number(minutes);
    if (m >= 480) return 'time-ago--critical';
    if (m >= 60)  return 'time-ago--warning';
    return '';
}

/* ── 원인 배지 HTML ─────────────────────────────────────── */
function buildCauseBadge(cause) {
    const c = CAUSE[cause] || CAUSE.never_connected;
    return `<span class="cause-badge ${c.badgeClass}">${c.icon} ${c.label}</span>`;
}

/* ── 머신 타입 배지 HTML ────────────────────────────────── */
function buildTypeBadge(type) {
    const t = MACHINE_TYPE[type];
    if (!t) return `<span class="type-badge">${type || '-'}</span>`;
    return `<span class="type-badge ${t.cls}">${t.label}</span>`;
}

/* ── 테이블 렌더링 ──────────────────────────────────────── */
function renderTable() {
    const tbody = document.getElementById('offlineTableBody');
    if (!tbody) return;

    if (!allListData.length) {
        tbody.innerHTML = `
            <tr>
                <td colspan="7">
                    <div class="offline-empty">
                        <div class="offline-empty__icon">✅</div>
                        <div class="offline-empty__text">미수신 머신 없음</div>
                        <div class="offline-empty__sub">모든 활성 머신이 해당 날짜에 데이터를 전송했습니다.</div>
                    </div>
                </td>
            </tr>`;
        return;
    }

    const start = (currentPage - 1) * perPage;
    const page  = allListData.slice(start, start + perPage);

    tbody.innerHTML = page.map(row => {
        const min       = row.minutes_offline !== null ? Number(row.minutes_offline) : null;
        const timeText  = formatMinutesAgo(min);
        const timeCls   = getTimeClass(min);
        const causeBadge = buildCauseBadge(row.cause || 'never_connected');
        const typeBadge  = buildTypeBadge(row.type);
        const lastDate   = row.last_work_date || '-';

        return `
            <tr>
                <td><strong>${escHtml(row.machine_no)}</strong></td>
                <td>${escHtml(row.factory_name)}</td>
                <td>${escHtml(row.line_name)}</td>
                <td>${typeBadge}</td>
                <td>
                    ${row.last_data_time
                        ? `<div>${escHtml(row.last_data_time)}</div><div style="font-size:var(--sap-font-size-xs);color:var(--sap-text-secondary);">작업일: ${escHtml(lastDate)}</div>`
                        : '<span style="color:var(--sap-text-secondary);">-</span>'}
                </td>
                <td>
                    <span class="time-ago ${timeCls}">${timeText}</span>
                </td>
                <td>${causeBadge}</td>
            </tr>`;
    }).join('');
}

/* ── 페이지네이션 렌더링 ─────────────────────────────────── */
function renderPagination() {
    const container = document.getElementById('offlinePagination');
    if (!container) return;

    const totalPages = Math.ceil(allListData.length / perPage);
    if (totalPages <= 1) { container.innerHTML = ''; return; }

    let html = '';
    if (currentPage > 1) {
        html += `<button class="fiori-pagination__btn" onclick="goPage(${currentPage - 1})">&#8249;</button>`;
    }
    for (let i = 1; i <= totalPages; i++) {
        html += `<button class="fiori-pagination__btn${i === currentPage ? ' fiori-pagination__btn--active' : ''}" onclick="goPage(${i})">${i}</button>`;
    }
    if (currentPage < totalPages) {
        html += `<button class="fiori-pagination__btn" onclick="goPage(${currentPage + 1})">&#8250;</button>`;
    }
    container.innerHTML = html;
}

function goPage(page) {
    currentPage = page;
    renderTable();
    renderPagination();
}

/* ── HTML 이스케이프 ─────────────────────────────────────── */
function escHtml(str) {
    if (!str) return '';
    return String(str)
        .replace(/&/g, '&amp;')
        .replace(/</g, '&lt;')
        .replace(/>/g, '&gt;')
        .replace(/"/g, '&quot;');
}
