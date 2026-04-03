/*
 * ============================================================
 * data_downtime_2.js
 * ============================================================
 * 목적: 다운타임(Downtime) 데이터 실시간 모니터링 페이지 스크립트
 *
 * 주요 기능:
 *   - SSE(EventSource)를 통해 서버에서 다운타임 데이터를 실시간 수신
 *   - Chart.js 5개 차트(유형별/상태별/추이/라인·기계별/지속시간 분포) 렌더링
 *   - 날짜 범위 필터(daterangepicker) 및 공장·라인·기계 필터 지원
 *   - 진행 중(Warning) 다운타임의 지속 시간을 setInterval로 1초마다 갱신
 *   - 페이지네이션(페이지 당 10건) 및 상세 모달 팝업
 *   - 페이지 숨김/언로드 시 SSE 연결 자동 종료 및 지수 백오프 재연결 로직
 *
 * 연관 백엔드: proc/data_downtime_stream_2.php (SSE),
 *              proc/data_downtime_export_2.php (Excel 내보내기)
 * ============================================================
 */

// data_downtime_2.js — Row Grid Layout version (data_oee_2 model)

/* ── 전역 상태 변수 ─────────────────────────────────────── */
let eventSource = null;          // SSE EventSource 인스턴스
let isTracking = false;          // 현재 SSE 수신 중 여부
let charts = {};                 // Chart.js 인스턴스 맵 (chart 이름 → 인스턴스)
let downtimeData = [];           // SSE로 수신된 다운타임 데이터 전체 배열
let reconnectAttempts = 0;       // SSE 재연결 시도 횟수
let maxReconnectAttempts = 3;    // 최대 재연결 시도 횟수
let stats = {};                  // 통계 요약 데이터 (API 응답)
let activeDowntimesList = [];    // 현재 활성(Warning) 다운타임 목록
let elapsedTimeTimer = null;     // 경과 시간 갱신용 setInterval ID (미사용, 하위 호환)
let durationUpdateTimer = null;  // 진행 중 다운타임 지속 시간 갱신용 setInterval ID
let isPageUnloading = false;     // 페이지 언로드 진행 중 여부 플래그

/* ── 페이지네이션 상태 ──────────────────────────────────── */
let currentPage = 1;             // 현재 페이지 번호
let itemsPerPage = 10;           // 페이지당 표시 건수
let totalItems = 0;              // 전체 데이터 건수

/* ── Chart.js 전역 기본 스타일 설정 ────────────────────── */
Chart.defaults.color = '#1a1a1a';
Chart.defaults.borderColor = '#e8eaed';

/* ── 차트 색상 팔레트 정의 ─────────────────────────────── */
const chartColors = {
    warning:   '#da1e28',   // 활성(경고) 색상 (빨강)
    completed: '#30914c',   // 완료 색상 (초록)
    primary:   '#0070f2',   // 기본 색상 (파랑)
    info:      '#7c3aed',   // 정보 색상 (보라)
    secondary: '#1e88e5',   // 보조 색상 (밝은 파랑)
    accent:    '#00d4aa',   // 강조 색상 (민트)
    orange:    '#e26b0a',   // 주황 색상
    maroon:    '#800000'    // 장기 다운타임 색상 (어두운 빨강)
};

/* ── DOMContentLoaded: 페이지 초기화 진입점 ────────────── */
// DOM이 완전히 로드된 후 순서대로 초기화 함수를 호출
document.addEventListener('DOMContentLoaded', async function () {
    initDateRangePicker();      // 날짜 범위 선택기 초기화
    await initFilterSystem();   // 공장/라인/기계 필터 옵션 로드
    initCharts();               // Chart.js 차트 인스턴스 생성
    setupEventListeners();      // 버튼·셀렉트박스 이벤트 등록
    updateLayout(); // 초기 grid-template-rows 설정
    await loadInitialData();    // 초기 빈 상태 표시
    await startAutoTracking();  // SSE 실시간 추적 시작
});

// ─── 페이지 언로드 처리 ──────────────────────────────────
// 페이지를 벗어날 때 SSE 연결과 모든 타이머를 정리
window.addEventListener('beforeunload', function () {
    isPageUnloading = true;
    if (eventSource) { eventSource.close(); eventSource = null; isTracking = false; }
    stopElapsedTimeTimer();
    stopDurationUpdateTimer();
});

// pagehide: 모바일/뒤로가기 등 beforeunload가 발생하지 않는 경우 대비
window.addEventListener('pagehide', function () {
    isPageUnloading = true;
    if (eventSource) { eventSource.close(); eventSource = null; isTracking = false; }
    stopElapsedTimeTimer();
    stopDurationUpdateTimer();
});

// visibilitychange: 탭 전환 시 SSE 일시 중지 및 복귀 시 재연결
document.addEventListener('visibilitychange', function () {
    if (document.hidden) {
        // 탭이 숨겨지면 SSE 연결 해제 및 타이머 정지
        if (eventSource && isTracking) { eventSource.close(); eventSource = null; isTracking = false; }
        stopElapsedTimeTimer();
        stopDurationUpdateTimer();
    } else {
        // 탭이 다시 보이면 SSE 재연결 시도
        if (!isPageUnloading && !isTracking && eventSource === null) {
            reconnectAttempts = 0;
            startAutoTracking();
        }
    }
});

// focus 이벤트: 언로드 플래그 초기화 (탭 포커스 복귀 시)
window.addEventListener('focus', function () { if (isPageUnloading) isPageUnloading = false; });

// ─── 레이아웃 동적 업데이트 (핵심 함수) ──────────────────
// 표시/숨김 Row 상태에 따라 CSS grid-template-rows를 재계산
function updateLayout() {
    var main = document.getElementById('dtSignageMain');
    if (!main) return;

    var rows = [];
    // 각 Row가 숨김 상태가 아닐 때만 grid 행 크기 추가
    if (!document.getElementById('dtRowStats').classList.contains('hidden'))        rows.push('auto');
    if (!document.getElementById('dtRowChartsTop').classList.contains('hidden'))    rows.push('1fr');
    if (!document.getElementById('dtRowChartsBottom').classList.contains('hidden')) rows.push('1fr');
    if (!document.getElementById('dtRowTable').classList.contains('hidden'))        rows.push('1fr');
    rows.push('auto'); // pagination

    main.style.gridTemplateRows = rows.join(' ');

    // Chart.js ResizeObserver 트리거 (DOM 레이아웃 반영 후)
    // 레이아웃 변경 후 50ms 지연하여 차트 크기를 재계산
    setTimeout(function () {
        Object.values(charts).forEach(function (c) { if (c) c.resize(); });
    }, 50);
}

// ─── 토글 함수 ────────────────────────────────────────────
// 통계(Stats) Row 표시/숨김 토글
function toggleStatsDisplay() {
    var row = document.getElementById('dtRowStats');
    var btn = document.getElementById('toggleStatsBtn');
    if (!row || !btn) return;

    row.classList.toggle('hidden');
    btn.textContent = row.classList.contains('hidden') ? 'Show Stats' : 'Hide Stats';
    updateLayout();
}

// 차트(Charts) Row 표시/숨김 토글 (상단/하단 동시)
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

// 데이터 테이블 Row 표시/숨김 토글 (페이지네이션 포함)
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
// jQuery daterangepicker 플러그인으로 날짜 범위 선택기 초기화
function initDateRangePicker() {
    try {
        // daterangepicker 옵션: 날짜 형식, 빠른 범위 버튼 포함
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
            // 빠른 범위 선택: 오늘, 어제, 지난 1주, 지난 1달
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
            // 날짜 범위가 변경될 때 실시간 모니터링 재시작
            handleDateRangeChange(start, end);
        });
        // 초기값: 오늘 날짜로 설정
        $('#dateRangePicker').val(moment().format('YYYY-MM-DD') + ' ~ ' + moment().format('YYYY-MM-DD'));
    } catch (e) {
        console.error('DateRangePicker init error:', e);
    }
}

// ─── 필터 시스템 ──────────────────────────────────────────
// 공장·라인·기계 필터 셀렉트박스를 순서대로 초기화
async function initFilterSystem() {
    await loadFactoryOptions();   // 공장 목록 로드
    await loadLineOptions();      // 라인 목록 로드
    await loadMachineOptions();   // 기계 목록 로드
}

// 공장 목록을 API로 가져와 셀렉트박스에 채우고 change 이벤트 등록
async function loadFactoryOptions() {
    try {
        // fetch: 활성화된 공장 목록 요청
        var res = await fetch('../manage/proc/factory.php?status_filter=Y').then(function (r) { return r.json(); });
        if (!res.success || !res.data) return;
        var sel = document.getElementById('factoryFilterSelect');
        sel.innerHTML = '<option value="">All Factory</option>';
        res.data.forEach(function (f) {
            sel.innerHTML += '<option value="' + f.idx + '">' + f.factory_name + '</option>';
        });
        // 공장 변경 시 기계 셀렉트 초기화 후 라인 목록 재로드
        sel.addEventListener('change', async function (e) {
            document.getElementById('factoryLineMachineFilterSelect').innerHTML = '<option value="">All Machine</option>';
            document.getElementById('factoryLineMachineFilterSelect').disabled = true;
            await updateLineOptions(e.target.value);
            await restartRealTimeMonitoring();
        });
    } catch (e) { console.error('Factory options error:', e); }
}

// 라인 목록을 API로 가져와 셀렉트박스에 채우고 change 이벤트 등록
async function loadLineOptions() {
    try {
        // fetch: 활성화된 라인 목록 요청
        var res = await fetch('../manage/proc/line.php?status_filter=Y').then(function (r) { return r.json(); });
        if (!res.success || !res.data) return;
        var sel = document.getElementById('factoryLineFilterSelect');
        sel.innerHTML = '<option value="">All Line</option>';
        res.data.forEach(function (l) {
            sel.innerHTML += '<option value="' + l.idx + '">' + l.line_name + '</option>';
        });
        // 라인 변경 시 기계 목록 재로드 및 실시간 모니터링 재시작
        sel.addEventListener('change', async function (e) {
            await updateMachineOptions(document.getElementById('factoryFilterSelect').value, e.target.value);
            await restartRealTimeMonitoring();
        });
    } catch (e) { console.error('Line options error:', e); }
}

// 기계 목록을 API로 가져와 셀렉트박스에 채움 (초기 전체 목록)
async function loadMachineOptions() {
    try {
        // fetch: 전체 기계 목록 요청
        var res = await fetch('../manage/proc/machine.php').then(function (r) { return r.json(); });
        if (!res.success || !res.data) return;
        var sel = document.getElementById('factoryLineMachineFilterSelect');
        sel.innerHTML = '<option value="">All Machine</option>';
        res.data.forEach(function (m) {
            sel.innerHTML += '<option value="' + m.idx + '">' + m.machine_no + ' (' + (m.machine_model_name || 'No Model') + ')</option>';
        });
    } catch (e) { console.error('Machine options error:', e); }
}

// 공장 선택 변경 시 해당 공장의 라인만 필터링하여 셀렉트박스 갱신
async function updateLineOptions(factoryId) {
    var sel = document.getElementById('factoryLineFilterSelect');
    sel.disabled = true;  // 로딩 중 비활성화
    try {
        // factoryId가 있으면 공장 필터 파라미터 추가
        var url = '../manage/proc/line.php?status_filter=Y' + (factoryId ? '&factory_filter=' + factoryId : '');
        var res = await fetch(url).then(function (r) { return r.json(); });
        sel.innerHTML = '<option value="">All Line</option>';
        if (res.success) res.data.forEach(function (l) { sel.innerHTML += '<option value="' + l.idx + '">' + l.line_name + '</option>'; });
        sel.disabled = false;
        // 공장이 선택된 경우 기계 셀렉트도 활성화 및 갱신
        if (factoryId) {
            document.getElementById('factoryLineMachineFilterSelect').disabled = false;
            updateMachineOptions(factoryId, '');
        }
    } catch (e) { sel.innerHTML = '<option value="">All Line</option>'; sel.disabled = false; }
}

// 공장+라인 선택 변경 시 해당 조건의 기계만 필터링하여 셀렉트박스 갱신
async function updateMachineOptions(factoryId, lineId) {
    var sel = document.getElementById('factoryLineMachineFilterSelect');
    sel.disabled = true;  // 로딩 중 비활성화
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
// 버튼 및 셀렉트박스에 클릭/변경 이벤트 리스너를 일괄 등록
function setupEventListeners() {
    var el;

    // Excel 내보내기 버튼 클릭 시 exportData 호출
    el = document.getElementById('excelDownloadBtn');
    if (el) el.addEventListener('click', exportData);

    // 새로고침 버튼 클릭 시 SSE 재연결
    el = document.getElementById('refreshBtn');
    if (el) el.addEventListener('click', refreshData);

    // 통계 영역 표시/숨김 토글 버튼
    el = document.getElementById('toggleStatsBtn');
    if (el) el.addEventListener('click', toggleStatsDisplay);

    // 차트 영역 표시/숨김 토글 버튼
    el = document.getElementById('toggleChartsBtn');
    if (el) el.addEventListener('click', toggleChartsDisplay);

    // 테이블 영역 표시/숨김 토글 버튼
    el = document.getElementById('toggleDataBtn');
    if (el) el.addEventListener('click', toggleDataDisplay);

    // 시간 범위 셀렉트박스 변경 시 날짜 범위 자동 갱신
    el = document.getElementById('timeRangeSelect');
    if (el) el.addEventListener('change', handleTimeRangeChange);

    // 기계 필터 변경 시 실시간 모니터링 재시작
    el = document.getElementById('factoryLineMachineFilterSelect');
    if (el) el.addEventListener('change', function () { restartRealTimeMonitoring(); });

    // 교대(shift) 필터 변경 시 실시간 모니터링 재시작
    el = document.getElementById('shiftSelect');
    if (el) el.addEventListener('change', function () { restartRealTimeMonitoring(); });
}

// ─── 초기 데이터 ──────────────────────────────────────────
// 페이지 초기 로드 시 빈 상태(로딩 중) UI를 표시
async function loadInitialData() {
    displayEmptyState();
}

// 통계 카드와 테이블에 로딩 중 기본값('-') 표시
function displayEmptyState() {
    // 통계 카드 요소들을 '-'로 초기화
    ['totalDowntime', 'activeDowntimes', 'currentShiftDowntime',
     'affectedMachinesDowntime', 'longDowntimes', 'avgDowntimeResolution'].forEach(function (id) {
        var el = document.getElementById(id);
        if (el) el.textContent = '-';
    });
    // 테이블 본문에 로딩 메시지 표시
    var tbody = document.getElementById('downtimeDataBody');
    if (tbody) tbody.innerHTML = '<tr><td colspan="11" class="data-table-centered"><div class="fiori-alert fiori-alert--info"><strong>Information:</strong> Loading real-time Downtime data.</div></td></tr>';
}

// ─── 차트 초기화 ──────────────────────────────────────────
// Chart.js 차트 5개를 생성하여 charts 객체에 저장
function initCharts() {
    charts.dtType     = createDtTypeChart();      // 다운타임 유형별 막대 차트
    charts.dtStatus   = createDtStatusChart();    // 활성/완료 도넛 차트
    charts.dtTrend    = createDtTrendChart();     // 다운타임 발생 추이 선 차트
    charts.dtLine     = createDtLineChart();      // 라인/기계별 다운타임 막대 차트
    charts.dtDuration = createDtDurationChart();  // 지속 시간 분포 막대 차트
}

// 다운타임 유형별 누적 지속 시간(분)을 보여주는 세로 막대 차트 생성
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
                // 툴팁에 분 단위 표시
                tooltip: { callbacks: { label: function (c) { return c.dataset.label + ': ' + (c.parsed.y || 0).toFixed(1) + ' min'; } } }
            },
            scales: {
                y: { beginAtZero: true, title: { display: true, text: 'Duration (min)' }, ticks: { callback: function (v) { return v + 'm'; } } },
                x: { title: { display: true, text: 'Downtime Type' } }
            }
        }
    });
}

// 활성(Warning) vs 완료(Completed) 다운타임 비율을 보여주는 도넛 차트 생성
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
                // 툴팁: 건수와 비율(%) 동시 표시
                tooltip: { callbacks: { label: function (c) { var t = c.dataset.data.reduce(function (a, b) { return a + b; }, 0); var p = t > 0 ? ((c.raw / t) * 100).toFixed(1) : 0; return c.label + ': ' + c.raw + ' (' + p + '%)'; } } }
            }
        }
    });
}

// 시간/일자별 다운타임 발생 추이(Warning/Completed)를 보여주는 선 차트 생성
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
                    backgroundColor: chartColors.warning + '20',  // 10% 투명도
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

// 라인/기계별 다운타임 건수(Warning/Completed)를 보여주는 그룹 막대 차트 생성
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

// 다운타임 지속 시간 구간별 건수 분포를 보여주는 막대 차트 생성
// 구간: <5분 / 5~15분 / 15~30분 / 30~60분 / >60분
function createDtDurationChart() {
    var ctx = document.getElementById('dtDurationChart').getContext('2d');
    return new Chart(ctx, {
        type: 'bar',
        data: {
            labels: ['< 5min', '5–15min', '15–30min', '30–60min', '> 60min'],
            datasets: [{
                label: 'Count',
                data: [0, 0, 0, 0, 0],
                // 짧은 시간(초록)에서 긴 시간(어두운 빨강)으로 색상 변화
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
// 현재 필터 상태에서 SSE URL 파라미터 객체를 반환
function getFilterParams() {
    var dateRange = $('#dateRangePicker').val();
    var startDate = '', endDate = '';
    // 날짜 범위 문자열을 시작일/종료일로 분리
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
        limit: 100  // 한 번에 최대 100건 수신
    };
}

// SSE EventSource를 생성하고 실시간 다운타임 데이터 수신을 시작
async function startAutoTracking() {
    if (isTracking) return;  // 이미 추적 중이면 중복 실행 방지
    var params = new URLSearchParams(getFilterParams());
    // EventSource: 서버에서 다운타임 데이터를 SSE로 스트리밍
    eventSource = new EventSource('proc/data_downtime_stream_2.php?' + params.toString());
    setupSSEEventListeners();
    isTracking = true;
    var el = document.getElementById('connectionStatus');
    if (el) el.textContent = 'Downtime System Real-time Connected';
}

// SSE 이벤트 리스너 등록 (연결/데이터/heartbeat/종료)
function setupSSEEventListeners() {
    // SSE 오류 발생 시 재연결 시도
    eventSource.onerror = function () {
        isTracking = false;
        attemptReconnection();
    };

    // 'connected' 이벤트: 서버 연결 성공 확인
    eventSource.addEventListener('connected', function (e) {
        reconnectAttempts = 0;  // 재연결 카운터 초기화
        var el = document.getElementById('connectionStatus');
        if (el) el.textContent = 'Downtime system connected';
    });

    // 'downtime_data' 이벤트: 실제 다운타임 데이터 수신 및 UI 갱신
    eventSource.addEventListener('downtime_data', function (e) {
        var data = JSON.parse(e.data);
        stats             = data.stats;                      // 통계 요약
        activeDowntimesList = data.active_downtimes || [];   // 활성 다운타임 목록
        downtimeData      = data.downtime_data || [];        // 전체 다운타임 데이터
        window.downtimeData = downtimeData;                  // 전역 참조 (차트 폴백용)

        // UI 각 영역 순서대로 갱신
        updateStatCardsFromAPI(stats);
        updateDetailsFromAPI(stats, activeDowntimesList);
        updateTableFromAPI(downtimeData);
        updateChartsFromAPI(data);

        // 마지막 업데이트 시각 표시
        var el = document.getElementById('lastUpdateTime');
        if (el) el.textContent = 'Last updated: ' + data.timestamp;
    });

    // 'heartbeat' 이벤트: 연결 유지 확인 (활성 경고 수 표시)
    eventSource.addEventListener('heartbeat', function (e) {
        var data = JSON.parse(e.data);
        var el = document.getElementById('connectionStatus');
        if (el) el.textContent = 'Connection maintained (Active warnings: ' + (data.active_warnings || 0) + ')';
    });

    // 'disconnected' 이벤트: 서버에서 연결 종료 통보
    eventSource.addEventListener('disconnected', function () {
        var el = document.getElementById('connectionStatus');
        if (el) el.textContent = 'Downtime System Connection Closed';
    });
}

// SSE 연결을 수동으로 종료하고 상태를 초기화
function stopTracking() {
    if (!isTracking) return;
    if (eventSource) { eventSource.close(); eventSource = null; }
    isTracking = false;
    var el = document.getElementById('connectionStatus');
    if (el) el.textContent = 'Downtime System Connection Closed';
}

// 필터 변경 등으로 SSE를 재시작: 현재 연결 종료 후 재연결
async function restartRealTimeMonitoring() {
    currentPage = 1;  // 페이지를 첫 페이지로 초기화
    if (isTracking) { stopTracking(); await new Promise(function (r) { setTimeout(r, 100); }); }
    reconnectAttempts = 0;
    await startAutoTracking();
}

// SSE 오류 발생 시 지수 백오프 방식으로 재연결 시도
async function attemptReconnection() {
    if (isPageUnloading || reconnectAttempts >= maxReconnectAttempts) return;
    reconnectAttempts++;
    // 재연결 대기 시간: 1s → 2s → 4s (최대 10s)
    var delay = Math.min(1000 * Math.pow(2, reconnectAttempts - 1), 10000);
    if (eventSource) { eventSource.close(); eventSource = null; }
    setTimeout(async function () {
        if (isPageUnloading) return;
        try { await startAutoTracking(); } catch (e) { attemptReconnection(); }
    }, delay);
}

// ─── 데이터 업데이트 ──────────────────────────────────────
// SSE로 수신된 통계 데이터로 상단 통계 카드를 갱신
function updateStatCardsFromAPI(s) {
    if (!s) return;
    // DOM 요소 ID와 표시 값의 매핑
    var map = {
        totalDowntime:           s.total_count     || '0',           // 전체 다운타임 건수
        activeDowntimes:         s.warning_count   || '0',           // 활성(진행 중) 건수
        currentShiftDowntime:    s.current_shift_count || '0',       // 현재 교대 발생 건수
        affectedMachinesDowntime: s.affected_machines || '0',        // 영향받은 기계 수
        longDowntimes:           s.long_downtimes_count || '0',      // 30분 초과 장기 건수
        avgDowntimeResolution:   s.avg_completed_time  || '-'        // 평균 해소 시간
    };
    // 각 DOM 요소의 텍스트를 매핑된 값으로 일괄 업데이트
    Object.keys(map).forEach(function (id) {
        var el = document.getElementById(id);
        if (el) el.textContent = map[id];
    });
}

// 다운타임 상세 패널(4개 Row)의 세부 수치를 갱신
function updateDetailsFromAPI(s, activeList) {
    if (!s) return;

    var totalCount     = parseInt(s.total_count)    || 0;
    var warningCount   = parseInt(s.warning_count)  || 0;
    var completedCount = totalCount - warningCount;  // 완료 건수 = 전체 - 활성

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
    set('dtDetailMaxDuration',      s.max_duration_display || '-');  // 최대 다운타임 시간

    // Row 4: Resolution Performance
    set('dtResolutionDetail',   s.avg_completed_time || '-');
    set('dtDetailAvgResolution', s.avg_completed_time || '-');
    set('dtDetailOver30',        s.long_downtimes_count || '0');     // 30분 초과 건수
}

// ─── 테이블 업데이트 ──────────────────────────────────────
// 페이지네이션을 적용하여 다운타임 데이터 테이블을 렌더링
function updateTableFromAPI(list) {
    var tbody = document.getElementById('downtimeDataBody');
    if (!tbody) return;
    totalItems = list.length;  // 전체 건수 갱신 (페이지네이션 계산용)

    // 데이터가 없으면 안내 메시지 표시
    if (list.length === 0) {
        tbody.innerHTML = '<tr><td colspan="11" class="data-table-centered"><div class="fiori-alert fiori-alert--info"><strong>Information:</strong> No Downtime data matching the selected conditions.</div></td></tr>';
        renderPagination();
        return;
    }

    // 현재 페이지에 해당하는 데이터 슬라이스
    var start = (currentPage - 1) * itemsPerPage;
    var paged = list.slice(start, start + itemsPerPage);
    tbody.innerHTML = '';

    // 각 행 렌더링
    paged.forEach(function (dt) {
        // 상태 배지: Warning(빨강) vs Completed(초록)
        var statusBadge = dt.status === 'Warning'
            ? '<span class="fiori-badge fiori-badge--error">Warning</span>'
            : '<span class="fiori-badge fiori-badge--success">Completed</span>';

        var shift = dt.shift_idx ? 'Shift ' + dt.shift_idx : '-';

        // 진행 중 다운타임: data-start-time 속성으로 1초 갱신, 완료는 실제 시간 표시
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
    startDurationUpdateTimer();  // 진행 중 셀 지속 시간 갱신 타이머 시작
}

// 테이블의 각 Details 버튼에 클릭 이벤트 리스너를 등록
function setupDetailsButtonListeners() {
    document.querySelectorAll('.dt-details-btn').forEach(function (btn) {
        btn.removeEventListener('click', handleDetailsButtonClick);  // 중복 방지
        btn.addEventListener('click', handleDetailsButtonClick);
    });
}

// Details 버튼 클릭 핸들러: 이벤트 전파를 막고 상세 모달 오픈
function handleDetailsButtonClick(e) {
    e.preventDefault();
    e.stopPropagation();
    var btn = e.target.closest('.dt-details-btn');
    if (btn) openDowntimeDetailModal(btn);
}

// ─── 페이지네이션 ──────────────────────────────────────────
// 페이지네이션 컨트롤 렌더링 (이전/다음/페이지 번호 버튼)
function renderPagination() {
    var container = document.getElementById('pagination-controls');
    if (!container) return;
    var totalPages = Math.ceil(totalItems / itemsPerPage);
    if (totalPages <= 1) { container.innerHTML = ''; return; }  // 1페이지면 미표시

    // 현재 페이지의 시작/끝 인덱스 계산
    var s = totalItems === 0 ? 0 : (currentPage - 1) * itemsPerPage + 1;
    var e = Math.min(currentPage * itemsPerPage, totalItems);
    var html = '<div class="fiori-pagination__info">' + s + '-' + e + ' / ' + totalItems + ' items</div>';
    html += '<div class="fiori-pagination__buttons">';
    html += '<button class="fiori-pagination__button' + (currentPage === 1 ? ' fiori-pagination__button--disabled' : '') + '" onclick="changePage(' + (currentPage - 1) + ')" ' + (currentPage === 1 ? 'disabled' : '') + '>&larr;</button>';

    // 앞뒤로 2페이지씩 표시, 생략 부호(...) 자동 추가
    var sp = Math.max(1, currentPage - 2);
    var ep = Math.min(totalPages, currentPage + 2);
    if (sp > 1) { html += '<button class="fiori-pagination__button" onclick="changePage(1)">1</button>'; if (sp > 2) html += '<span class="fiori-pagination__ellipsis">...</span>'; }
    for (var i = sp; i <= ep; i++) html += '<button class="fiori-pagination__button' + (i === currentPage ? ' fiori-pagination__button--active' : '') + '" onclick="changePage(' + i + ')">' + i + '</button>';
    if (ep < totalPages) { if (ep < totalPages - 1) html += '<span class="fiori-pagination__ellipsis">...</span>'; html += '<button class="fiori-pagination__button" onclick="changePage(' + totalPages + ')">' + totalPages + '</button>'; }

    html += '<button class="fiori-pagination__button' + (currentPage === totalPages ? ' fiori-pagination__button--disabled' : '') + '" onclick="changePage(' + (currentPage + 1) + ')" ' + (currentPage === totalPages ? 'disabled' : '') + '>&rarr;</button>';
    html += '</div>';
    container.innerHTML = html;
}

// 페이지 번호 변경 시 테이블 데이터를 해당 페이지로 재렌더링
function changePage(p) {
    var total = Math.ceil(totalItems / itemsPerPage);
    if (p < 1 || p > total || p === currentPage) return;
    currentPage = p;
    updateTableFromAPI(downtimeData);
}

// ─── 차트 업데이트 ────────────────────────────────────────
// SSE 수신 데이터로 5개 차트를 모두 갱신
function updateChartsFromAPI(data) {
    if (data.downtime_type_stats  && charts.dtType)  updateDtTypeChart(data.downtime_type_stats);
    if (data.stats                && charts.dtStatus) updateDtStatusChart(data.stats);
    if (data.downtime_trend_stats && charts.dtTrend)  updateDtTrendChart(data.downtime_trend_stats);

    // dtLine: API 통계가 있으면 사용, 없으면 수신 데이터에서 클라이언트 계산
    if (data.downtime_machine_stats && charts.dtLine) {
        updateDtLineChartFromAPI(data.downtime_machine_stats);
    } else if (downtimeData.length > 0 && charts.dtLine) {
        updateDtLineChartFromData(downtimeData);
    }

    // dtDuration: 수신 데이터에서 클라이언트 측 구간 계산
    if (downtimeData.length > 0 && charts.dtDuration) {
        updateDtDurationChartFromData(downtimeData);
    }
}

// 다운타임 유형별 차트 데이터를 이름순 정렬 후 갱신 (지속 시간 단위: 분)
function updateDtTypeChart(stat) {
    if (!charts.dtType || !stat || stat.length === 0) return;
    // 이름순 정렬하여 일관된 차트 표시
    var sorted = stat.slice().sort(function (a, b) {
        return (a.downtime_name || '').localeCompare(b.downtime_name || '');
    });
    var labels  = sorted.map(function (d) { return d.downtime_name || 'Unclassified'; });
    var durations = sorted.map(function (d) { return parseFloat(d.total_duration_min) || 0; });

    // 색상을 순환 배열로 할당
    var colorKeys = ['orange', 'warning', 'primary', 'info', 'accent', 'secondary'];
    var bgColors = labels.map(function (_, i) { return chartColors[colorKeys[i % colorKeys.length]]; });

    charts.dtType.data.labels                        = labels;
    charts.dtType.data.datasets[0].data              = durations;
    charts.dtType.data.datasets[0].backgroundColor   = bgColors;
    charts.dtType.data.datasets[0].borderColor       = bgColors;
    charts.dtType.update('none');  // 애니메이션 없이 즉시 갱신
}

// 활성/완료 상태 도넛 차트 데이터 갱신
function updateDtStatusChart(s) {
    if (!charts.dtStatus || !s) return;
    var warning   = parseInt(s.warning_count)  || 0;
    var total     = parseInt(s.total_count)    || 0;
    var completed = Math.max(0, total - warning);  // 음수 방지
    charts.dtStatus.data.datasets[0].data = [warning, completed];
    charts.dtStatus.update('none');
}

// 다운타임 발생 추이 차트 갱신 (시간별/일별 뷰타입 자동 분기)
function updateDtTrendChart(stat) {
    if (!charts.dtTrend) return;
    // API 데이터 없으면 수신 데이터로 단순 추이 생성
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
    var viewType  = stat.view_type || 'hourly';  // 기본값: 시간별
    var labels    = [];

    // 뷰타입에 따라 X축 레이블 생성
    if (viewType === 'hourly' && workHours && workHours.start_time) {
        labels = generateWorkHoursLabels(workHours.start_time, workHours.end_time, workHours.start_minutes, workHours.end_minutes);
    } else if (viewType === 'daily') {
        labels = trendData.map(function (d) { return d.display_label || d.time_label; });
    } else {
        // 'YYYY-MM-DD HH:' 형식에서 시간(HH+'H') 추출
        labels = trendData.map(function (d) {
            var lbl = d.time_label || d.display_label;
            var m = lbl && lbl.match(/^(\d{4}-\d{2}-\d{2})\s+(\d{2}):/);
            return m ? m[2] + 'H' : lbl;
        });
    }

    // Warning/Completed 건수를 레이블 키로 맵핑
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
    charts.dtTrend.update('show');  // show 애니메이션으로 갱신
}

// 근무 시간 범위에 맞는 시간 단위 레이블 배열 생성 (예: ['08H','09H',...])
function generateWorkHoursLabels(startTime, endTime, startMinutes, endMinutes) {
    var labels = [];
    // 분 단위가 있으면 분→시간 변환, 없으면 시간 문자열 직접 파싱
    var sh = startMinutes !== null && startMinutes !== undefined ? Math.floor(startMinutes / 60) : parseInt(startTime.split(':')[0]);
    var eh = endMinutes   !== null && endMinutes   !== undefined ? Math.floor(endMinutes / 60)   : parseInt(endTime.split(':')[0]);
    if (eh < sh) eh += 24;  // 자정을 넘는 야간 근무 처리
    for (var h = sh; h <= eh; h++) labels.push(String(h % 24).padStart(2, '0') + 'H');
    return labels;
}

// API 추이 데이터가 없을 때 수신된 downtimeData에서 날짜별 단순 집계 생성
function generateSimpleTrendFromDowntimeData() {
    if (!charts.dtTrend || !window.downtimeData || window.downtimeData.length === 0) return;
    // 작업일(work_date)별로 Warning/Completed 건수 집계
    var grp = {};
    window.downtimeData.forEach(function (d) {
        if (!d.work_date) return;
        if (!grp[d.work_date]) grp[d.work_date] = { warn: 0, comp: 0 };
        if (d.status === 'Warning') grp[d.work_date].warn++;
        else grp[d.work_date].comp++;
    });
    var dates = Object.keys(grp).sort();
    // X축 레이블: M/D 형식으로 변환
    charts.dtTrend.data.labels = dates.map(function (d) { var x = new Date(d); return (x.getMonth() + 1) + '/' + x.getDate(); });
    charts.dtTrend.data.datasets[0].data = dates.map(function (d) { return grp[d].warn; });
    charts.dtTrend.data.datasets[1].data = dates.map(function (d) { return grp[d].comp; });
    charts.dtTrend.options.scales.x.title.text = 'Date';
    charts.dtTrend.update('show');
}

// API 통계 데이터로 라인/기계별 다운타임 차트 갱신
function updateDtLineChartFromAPI(stat) {
    if (!charts.dtLine || !stat || !Array.isArray(stat) || stat.length === 0) return;
    // machine_no 속성 유무로 기계별/라인별 구분
    var isMachine = stat[0].hasOwnProperty('machine_no');
    var labels = stat.map(function (d) { return isMachine ? (d.machine_no || 'Unknown') : (d.line_name || 'Unknown'); });

    charts.dtLine.data.labels = labels;
    charts.dtLine.data.datasets[0].data = stat.map(function (d) { return parseInt(d.warning_count) || 0; });
    charts.dtLine.data.datasets[1].data = stat.map(function (d) { return parseInt(d.completed_count) || 0; });
    charts.dtLine.options.scales.x.title.text = isMachine ? 'Machine Number' : 'Line Name';

    // 차트 제목/부제목 동적 변경
    var titleEl    = document.getElementById('dtLineChartTitle');
    var subtitleEl = document.getElementById('dtLineChartSubtitle');
    if (titleEl)    titleEl.textContent    = isMachine ? 'Machine Downtime' : 'Line Downtime';
    if (subtitleEl) subtitleEl.textContent = isMachine ? 'Downtime comparison by machine' : 'Downtime comparison by line';

    charts.dtLine.update('none');
}

// 수신 데이터에서 라인별 다운타임을 집계하여 차트 갱신 (API 없을 때 폴백)
function updateDtLineChartFromData(list) {
    if (!charts.dtLine || !list || list.length === 0) return;
    // 라인 이름별 Warning/Completed 건수 집계
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

// 수신 데이터에서 지속 시간 구간별 건수를 계산하여 분포 차트 갱신
function updateDtDurationChartFromData(list) {
    if (!charts.dtDuration || !list || list.length === 0) return;
    var buckets = [0, 0, 0, 0, 0]; // <5, 5-15, 15-30, 30-60, >60 (분 단위 구간)
    list.forEach(function (d) {
        var min = parseDurationToMinutes(d.duration_his || d.duration_display);
        if (min === null) return;
        // 지속 시간을 구간 인덱스로 분류
        if (min < 5)       buckets[0]++;
        else if (min < 15) buckets[1]++;
        else if (min < 30) buckets[2]++;
        else if (min < 60) buckets[3]++;
        else               buckets[4]++;
    });
    charts.dtDuration.data.datasets[0].data = buckets;
    charts.dtDuration.update('none');
}

// "1h 23m" 또는 "23m" 또는 숫자 형식의 지속 시간 문자열을 분(minutes)으로 변환
function parseDurationToMinutes(str) {
    if (!str) return null;
    // "1h 23m" 또는 "23m" 또는 숫자 형식 처리
    var total = 0;
    var hMatch = String(str).match(/(\d+)\s*h/);  // 시간 부분 추출
    var mMatch = String(str).match(/(\d+)\s*m/);  // 분 부분 추출
    if (hMatch) total += parseInt(hMatch[1]) * 60;
    if (mMatch) total += parseInt(mMatch[1]);
    // 시간/분 모두 없으면 숫자 직접 파싱
    if (!hMatch && !mMatch) { var n = parseFloat(str); if (!isNaN(n)) total = n; else return null; }
    return total;
}

// ─── Elapsed Time Timer ────────────────────────────────────
// 발생 시각(startDateStr)부터 현재까지의 경과 시간 문자열 반환 (HH:MM:SS 형식)
function calculateElapsedTime(startDateStr) {
    if (!startDateStr) return '-';
    var start = new Date(startDateStr.replace(' ', 'T'));
    if (isNaN(start.getTime())) return '-';
    var diff = Math.floor((Date.now() - start.getTime()) / 1000);
    if (diff < 0) diff = 0;
    var h = Math.floor(diff / 3600);
    var m = Math.floor((diff % 3600) / 60);
    var s = diff % 60;
    // 1시간 이상: "Xh MM m SS s" 형식, 미만: "MM m SS s" 형식
    if (h > 0) return h + 'h ' + String(m).padStart(2, '0') + 'm ' + String(s).padStart(2, '0') + 's';
    return String(m).padStart(2, '0') + 'm ' + String(s).padStart(2, '0') + 's';
}

// 진행 중(Warning) 다운타임 셀의 지속 시간을 1초마다 갱신하는 타이머 시작
function startDurationUpdateTimer() {
    stopDurationUpdateTimer();  // 기존 타이머 중복 방지
    // setInterval: 1초마다 .duration-in-progress 셀의 시간 갱신
    durationUpdateTimer = setInterval(function () {
        document.querySelectorAll('.duration-in-progress[data-start-time]').forEach(function (cell) {
            cell.textContent = calculateElapsedTime(cell.getAttribute('data-start-time'));
        });
    }, 1000);
}

// 지속 시간 갱신 타이머 정지 및 ID 초기화
function stopDurationUpdateTimer() {
    if (durationUpdateTimer) { clearInterval(durationUpdateTimer); durationUpdateTimer = null; }
}

// 경과 시간 타이머 정지 (elapsedTimeTimer, 하위 호환용)
function stopElapsedTimeTimer() {
    if (elapsedTimeTimer) { clearInterval(elapsedTimeTimer); elapsedTimeTimer = null; }
}

// ─── 시간 범위 핸들러 ─────────────────────────────────────
// daterangepicker 날짜 변경 시: 커스텀 옵션 선택 후 SSE 재시작
async function handleDateRangeChange(start, end) {
    var sel = document.getElementById('timeRangeSelect');
    if (sel) {
        // 'custom' 옵션이 없으면 동적으로 생성 후 선택
        var opt = sel.querySelector('option[value="custom"]');
        if (!opt) { opt = document.createElement('option'); opt.value = 'custom'; opt.textContent = 'Custom'; sel.appendChild(opt); }
        sel.value = 'custom';
    }
    if (isTracking) { stopTracking(); setTimeout(async function () { await startAutoTracking(); }, 1000); }
    else await startAutoTracking();
}

// 시간 범위 셀렉트박스 변경 시: 해당 범위로 날짜 피커를 업데이트하고 SSE 재시작
async function handleTimeRangeChange(e) {
    var v = e.target.value;
    var s, en;
    // 선택된 범위에 따라 시작/종료 moment 계산
    switch (v) {
        case 'today':     s = moment().startOf('day'); en = moment().endOf('day'); break;
        case 'yesterday': s = moment().subtract(1,'days').startOf('day'); en = moment().subtract(1,'days').endOf('day'); break;
        case '1w':        s = moment().subtract(7,'days').startOf('day'); en = moment().endOf('day'); break;
        case '1m':        s = moment().subtract(30,'days').startOf('day'); en = moment().endOf('day'); break;
        default:          s = moment().startOf('day'); en = moment().endOf('day');
    }
    // custom이 아닌 경우 daterangepicker 값도 동기화
    if (v !== 'custom') {
        $('#dateRangePicker').data('daterangepicker').setStartDate(s);
        $('#dateRangePicker').data('daterangepicker').setEndDate(en);
        $('#dateRangePicker').val(s.format('YYYY-MM-DD') + ' ~ ' + en.format('YYYY-MM-DD'));
    }
    await restartRealTimeMonitoring();
}

// ─── 내보내기 / 새로고침 ──────────────────────────────────
// 현재 필터 조건으로 Excel 내보내기 (새 창으로 다운로드)
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

// 새로고침: SSE 재연결로 데이터 즉시 갱신
async function refreshData() {
    if (isTracking) { stopTracking(); setTimeout(async function () { await startAutoTracking(); }, 1000); }
    else await startAutoTracking();
}

// ─── Modal ────────────────────────────────────────────────
// Details 버튼의 data 속성에서 다운타임 정보를 파싱하여 모달 오픈
function openDowntimeDetailModal(btn) {
    try {
        var data = JSON.parse(btn.getAttribute('data-dt-data'));
        populateDowntimeModal(data);
        var modal = document.getElementById('downtimeDetailModal');
        if (modal) { modal.classList.add('show'); document.body.style.overflow = 'hidden'; }
    } catch (e) { console.error('Modal open error:', e); }
}

// 다운타임 상세 모달 닫기 (CSS 트랜지션 후 클래스 제거)
function closeDowntimeDetailModal() {
    var modal = document.getElementById('downtimeDetailModal');
    if (!modal) return;
    modal.classList.add('hide');
    setTimeout(function () { modal.classList.remove('show', 'hide'); document.body.style.overflow = ''; }, 300);
}

// 모달 DOM 요소에 다운타임 상세 데이터를 채워넣음
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

    // 다운타임 유형 색상 표시 (컬러 인디케이터)
    var colorIndicator = document.getElementById('modal-color-indicator');
    if (colorIndicator && d.downtime_color) {
        colorIndicator.style.background = d.downtime_color;
    }
}

// 모달에서 단건 내보내기 버튼 클릭 시 (모달만 닫음)
function exportSingleDowntime() {
    closeDowntimeDetailModal();
}