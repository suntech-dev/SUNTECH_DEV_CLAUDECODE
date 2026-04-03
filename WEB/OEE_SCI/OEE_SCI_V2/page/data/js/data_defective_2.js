/*
 * ============================================================
 * data_defective_2.js
 * ============================================================
 * 목적: 불량(Defective) 데이터 실시간 모니터링 페이지 스크립트
 *
 * 주요 기능:
 *   - SSE(EventSource)를 통해 서버에서 불량 데이터를 실시간 수신
 *   - Chart.js 5개 차트(불량 유형/상태/추이/기계별/라인별) 렌더링
 *   - 날짜 범위 필터(daterangepicker) 및 공장·라인·기계 필터 지원
 *   - 활성 불량 항목의 경과 시간(setInterval) 실시간 갱신
 *   - 페이지네이션(페이지 당 10건) 및 상세 모달 팝업
 *   - 페이지 숨김/언로드 시 SSE 연결 자동 종료 및 재연결 로직
 *
 * 연관 백엔드: proc/data_defective_stream_2.php (SSE),
 *              proc/data_defective_export_2.php (Excel 내보내기)
 * ============================================================
 */

// data_defective_2.js — Row Grid Layout version (ai_dashboard_5 model)

/* ── 전역 상태 변수 ─────────────────────────────────────── */
let eventSource = null;          // SSE EventSource 인스턴스
let isTracking = false;          // 현재 SSE 수신 중 여부
let charts = {};                 // Chart.js 인스턴스 맵 (chart 이름 → 인스턴스)
let defectiveData = [];          // SSE로 수신된 불량 데이터 전체 배열
let reconnectAttempts = 0;       // SSE 재연결 시도 횟수
let maxReconnectAttempts = 3;    // 최대 재연결 시도 횟수
let stats = {};                  // 통계 요약 데이터 (API 응답)
let activeDefectives = [];       // 현재 활성(Warning) 불량 목록
let elapsedTimeTimer = null;     // 경과 시간 갱신용 setInterval ID
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
    warning:   '#e26b0a',   // 경고 색상 (주황)
    error:     '#da1e28',   // 오류 색상 (빨강)
    success:   '#30914c',   // 성공 색상 (초록)
    info:      '#7c3aed',   // 정보 색상 (보라)
    primary:   '#0070f2',   // 기본 색상 (파랑)
    secondary: '#1e88e5',   // 보조 색상 (밝은 파랑)
    accent:    '#00d4aa'    // 강조 색상 (민트)
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
// 페이지를 벗어날 때 SSE 연결과 타이머를 정리
window.addEventListener('beforeunload', function () {
    isPageUnloading = true;
    if (eventSource) { eventSource.close(); eventSource = null; isTracking = false; }
    stopElapsedTimeTimer();
});

// pagehide: 모바일/뒤로가기 등 beforeunload가 발생하지 않는 경우 대비
window.addEventListener('pagehide', function () {
    isPageUnloading = true;
    if (eventSource) { eventSource.close(); eventSource = null; isTracking = false; }
    stopElapsedTimeTimer();
});

// visibilitychange: 탭 전환 시 SSE 일시 중지 및 복귀 시 재연결
document.addEventListener('visibilitychange', function () {
    if (document.hidden) {
        // 탭이 숨겨지면 SSE 연결 해제 및 타이머 정지
        if (eventSource && isTracking) { eventSource.close(); eventSource = null; isTracking = false; }
        stopElapsedTimeTimer();
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
    var main = document.getElementById('defectiveSignageMain');
    if (!main) return;

    var rows = [];
    // 각 Row가 숨김 상태가 아닐 때만 grid 행 크기 추가
    if (!document.getElementById('defectiveRowStats').classList.contains('hidden'))        rows.push('auto');
    if (!document.getElementById('defectiveRowChartsTop').classList.contains('hidden'))    rows.push('1fr');
    if (!document.getElementById('defectiveRowChartsBottom').classList.contains('hidden')) rows.push('1fr');
    if (!document.getElementById('defectiveRowTable').classList.contains('hidden'))        rows.push('1fr');
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
    var row = document.getElementById('defectiveRowStats');
    var btn = document.getElementById('toggleStatsBtn');
    if (!row || !btn) return;

    row.classList.toggle('hidden');
    btn.textContent = row.classList.contains('hidden') ? 'Show Stats' : 'Hide Stats';
    updateLayout();
}

// 차트(Charts) Row 표시/숨김 토글 (상단/하단 동시)
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

// 데이터 테이블 Row 표시/숨김 토글 (페이지네이션 포함)
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
// jQuery daterangepicker 플러그인으로 날짜 범위 선택기 초기화
function initDateRangePicker() {
    try {
        // daterangepicker 옵션: 한국 기준 날짜 형식, 빠른 범위 버튼 포함
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
    ['activeDefectives', 'currentShiftDefective', 'affectedMachinesDefective',
     'defectiveRate', 'qualityScore', 'totalDefectiveCount'].forEach(function (id) {
        var el = document.getElementById(id);
        if (el) el.textContent = '-';
    });
    var activeCountEl = document.getElementById('activeDefectiveCount');
    if (activeCountEl) activeCountEl.textContent = '0 active defectives';

    // 테이블 본문에 로딩 메시지 표시
    var tbody = document.getElementById('defectiveDataBody');
    if (tbody) tbody.innerHTML = '<tr><td colspan="8" class="data-table-centered"><div class="fiori-alert fiori-alert--info"><strong>Information:</strong> Loading real-time Defective data.</div></td></tr>';
}

// ─── 차트 초기화 ──────────────────────────────────────────
// Chart.js 차트 5개를 생성하여 charts 객체에 저장
function initCharts() {
    charts.defectiveType   = createDefectiveTypeChart();    // 불량 유형별 막대 차트
    charts.defectiveStatus = createDefectiveStatusChart();  // 활성/완료 도넛 차트
    charts.defectiveTrend  = createDefectiveTrendChart();   // 불량 발생 추이 선 차트
    charts.defectiveMachine = createDefectiveMachineChart(); // 기계별 불량 막대 차트
    charts.defectiveLine   = createDefectiveLineChart();    // 라인별 불량 막대 차트
}

// 불량 유형별 발생 건수를 보여주는 세로 막대 차트 생성
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

// 활성(Warning) vs 완료(Completed) 불량 비율을 보여주는 도넛 차트 생성
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
                        // 툴팁에 건수와 비율(%) 동시 표시
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

// 시간/일자별 불량 발생 건수 추이를 보여주는 선 차트 생성
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
                backgroundColor: chartColors.error + '20',  // 20% 투명도 배경
                fill: true,
                tension: 0.4,       // 곡선 부드럽게
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

// 기계 번호별 불량 건수를 보여주는 세로 막대 차트 생성
function createDefectiveMachineChart() {
    var ctx = document.getElementById('defectiveMachineChart').getContext('2d');
    return new Chart(ctx, {
        type: 'bar',
        data: {
            labels: [],
            datasets: [{
                label: 'Defective Count',
                data: [],
                backgroundColor: chartColors.warning + '80',  // 50% 투명도
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

// 생산 라인별 불량 건수를 보여주는 세로 막대 차트 생성
function createDefectiveLineChart() {
    var ctx = document.getElementById('defectiveLineChart').getContext('2d');
    return new Chart(ctx, {
        type: 'bar',
        data: {
            labels: [],
            datasets: [{
                label: 'Defective Count',
                data: [],
                backgroundColor: chartColors.info + '80',  // 50% 투명도
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

// SSE EventSource를 생성하고 실시간 불량 데이터 수신을 시작
async function startAutoTracking() {
    if (isTracking) return;  // 이미 추적 중이면 중복 실행 방지
    var params = new URLSearchParams(getFilterParams());
    // EventSource: 서버에서 불량 데이터를 SSE로 스트리밍
    eventSource = new EventSource('proc/data_defective_stream_2.php?' + params.toString());
    setupSSEEventListeners();
    isTracking = true;
    var el = document.getElementById('connectionStatus');
    if (el) el.textContent = 'Defective System Real-time Connected';
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
        if (el) el.textContent = 'Defective system connected';
    });

    // 'defective_data' 이벤트: 실제 불량 데이터 수신 및 UI 갱신
    eventSource.addEventListener('defective_data', function (e) {
        var data = JSON.parse(e.data);
        stats           = data.stats;                     // 통계 요약
        activeDefectives = data.active_defectives || [];  // 활성 불량 목록
        defectiveData   = data.defective_data;            // 전체 불량 데이터
        window.defectiveData = data.defective_data;       // 전역 참조 (모달 등 외부 접근용)

        // UI 각 영역 순서대로 갱신
        updateStatCardsFromAPI(stats);
        updateActiveDefectivesDisplay(activeDefectives);
        updateTableFromAPI(defectiveData);
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
        if (el) el.textContent = 'Defective System Connection Closed';
    });
}

// SSE 연결을 수동으로 종료하고 상태를 초기화
function stopTracking() {
    if (!isTracking) return;
    if (eventSource) { eventSource.close(); eventSource = null; }
    stopElapsedTimeTimer();
    isTracking = false;
    var el = document.getElementById('connectionStatus');
    if (el) el.textContent = 'Defective System Connection Closed';
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
// 숫자값을 소수점 1자리 또는 정수로 포맷 (표시용)
function formatRateNumber(value) {
    var n = parseFloat(value);
    if (isNaN(n)) return '-';
    return (Number.isInteger(n) ? n.toString() : n.toFixed(1));
}

// SSE로 수신된 통계 데이터로 상단 통계 카드를 갱신
function updateStatCardsFromAPI(s) {
    if (!s) return;

    // 불량률/품질 점수: null이면 '-' 표시, 있으면 포맷 후 '%' 추가
    var defectiveRateDisplay = '-';
    if (s.defective_rate !== null && s.defective_rate !== undefined) {
        defectiveRateDisplay = formatRateNumber(s.defective_rate) + '%';
    }

    var qualityScoreDisplay = '-';
    if (s.quality_score !== null && s.quality_score !== undefined) {
        qualityScoreDisplay = formatRateNumber(s.quality_score) + '%';
    }

    // DOM 요소 ID와 표시 값의 매핑
    var map = {
        activeDefectives:         s.warning_count       || '0',   // 활성 불량 수
        currentShiftDefective:    s.current_shift_count || '0',   // 현재 교대 불량
        affectedMachinesDefective: s.affected_machines  || '0',   // 영향받은 기계 수
        defectiveRate:            defectiveRateDisplay,            // 불량률
        qualityScore:             qualityScoreDisplay,             // 품질 점수
        totalDefectiveCount:      s.total_count         || '0'    // 전체 불량 건수
    };

    // 각 DOM 요소의 텍스트를 매핑된 값으로 일괄 업데이트
    Object.keys(map).forEach(function (id) {
        var el = document.getElementById(id);
        if (el) el.textContent = map[id];
    });
}

// 활성 불량 목록 패널을 최신 5건으로 갱신하고 경과 시간 타이머 시작
function updateActiveDefectivesDisplay(list) {
    var container  = document.getElementById('activeDefectivesContainer');
    var countEl    = document.getElementById('activeDefectiveCount');

    // 활성 불량 건수 표시
    if (countEl) countEl.textContent = list.length + ' active defectives';
    if (!container) return;

    // 활성 불량이 없으면 정상 메시지 표시 후 타이머 정지
    if (list.length === 0) {
        container.innerHTML = '<div class="fiori-alert fiori-alert--success"><strong>Good:</strong> No active defectives. All systems operating normally.</div>';
        stopElapsedTimeTimer();
        return;
    }

    // 최신순 정렬 후 상위 5건만 표시
    var sorted = list.slice().sort(function (a, b) {
        return new Date(b.reg_date) - new Date(a.reg_date);
    }).slice(0, 5);

    // 각 활성 불량 항목의 HTML 카드 생성
    var html = '';
    sorted.forEach(function (d) {
        var borderColor = d.defective_color || '#0070f2';  // 불량 유형 색상
        var bgColor     = borderColor + '1A';               // 10% 투명도 배경
        var elapsed     = calculateElapsedTime(d.reg_date); // 발생 경과 시간
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

    // 5건 초과 시 전체 건수 안내 메시지 추가
    if (list.length > 5) {
        html += '<div class="fiori-alert fiori-alert--info" style="margin-top:4px;"><strong>Info:</strong> Showing 5 most recent out of ' + list.length + ' total active defectives.</div>';
    }

    container.innerHTML = html;
    startElapsedTimeTimer();  // 경과 시간 1초 단위 갱신 타이머 시작
}

// 페이지네이션을 적용하여 데이터 테이블을 렌더링
function updateTableFromAPI(list) {
    var tbody = document.getElementById('defectiveDataBody');
    if (!tbody) return;
    totalItems = list.length;  // 전체 건수 갱신 (페이지네이션 계산용)

    // 데이터가 없으면 안내 메시지 표시
    if (list.length === 0) {
        tbody.innerHTML = '<tr><td colspan="8" class="data-table-centered"><div class="fiori-alert fiori-alert--info"><strong>Information:</strong> No Defective data matching the selected conditions.</div></td></tr>';
        renderPagination();
        return;
    }

    // 현재 페이지에 해당하는 데이터 슬라이스
    var start = (currentPage - 1) * itemsPerPage;
    var paged = list.slice(start, start + itemsPerPage);
    tbody.innerHTML = '';

    // 각 행 렌더링: Warning 상태면 경과 시간, Completed면 처리 시간 표시
    paged.forEach(function (d) {
        var shift   = d.shift_idx ? 'Shift ' + d.shift_idx : '-';
        var elapsed = d.status === 'Warning'
            ? calculateElapsedTime(d.reg_date)          // 진행 중: 경과 시간
            : (d.duration_display || d.duration_his || '-');  // 완료: 처리 시간

        var tr = document.createElement('tr');
        tr.innerHTML =
            '<td>' + (d.machine_no || '-') + '</td>' +
            '<td>' + (d.factory_name || '') + ' / ' + (d.line_name || '') + '</td>' +
            '<td>' + shift + '</td>' +
            '<td>' + (d.defective_name || '-') + '</td>' +
            '<td>' + (d.reg_date || '-') + '</td>' +
            // data 속성에 상태/발생시각/처리시간 저장 (setInterval 갱신용)
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

    // Warning 상태 행이 있으면 경과 시간 타이머 시작
    var hasWarning = paged.some(function (d) { return d.status === 'Warning'; });
    if (hasWarning && !elapsedTimeTimer) startElapsedTimeTimer();
}

// 테이블의 각 Details 버튼에 클릭 이벤트 리스너를 등록
function setupDetailsButtonListeners() {
    document.querySelectorAll('.defective-details-btn').forEach(function (btn) {
        btn.removeEventListener('click', handleDetailsButtonClick);  // 중복 방지
        btn.addEventListener('click', handleDetailsButtonClick);
    });
}

// Details 버튼 클릭 핸들러: 이벤트 전파를 막고 상세 모달 오픈
function handleDetailsButtonClick(e) {
    e.preventDefault();
    e.stopPropagation();
    var btn = e.target.closest('.defective-details-btn');
    if (btn) openDefectiveDetailModal(btn);
}

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
    updateTableFromAPI(defectiveData);
}

// ─── 차트 업데이트 ────────────────────────────────────────
// SSE 수신 데이터로 5개 차트를 모두 갱신
function updateChartsFromAPI(data) {
    if (data.defective_type_stats && charts.defectiveType)    updateDefectiveTypeChart(data.defective_type_stats);
    if (data.stats && charts.defectiveStatus)                 updateDefectiveStatusChart(data.stats);
    if (data.defective_rate_stats && charts.defectiveTrend)   updateDefectiveTrendChart(data.defective_rate_stats);
    if (data.machine_defective_stats && charts.defectiveMachine) updateDefectiveMachineChart(data.machine_defective_stats);
    if (data.line_defective_stats && charts.defectiveLine)    updateDefectiveLineChart(data.line_defective_stats);
}

// 불량 유형별 차트 데이터를 이름순 정렬 후 갱신
function updateDefectiveTypeChart(stat) {
    if (!charts.defectiveType || !stat || stat.length === 0) return;

    // 이름순 정렬하여 일관된 차트 표시
    var sorted = stat.slice().sort(function (a, b) {
        return (a.defective_name || '').localeCompare(b.defective_name || '');
    });

    var labels = sorted.map(function (d) { return d.defective_name || 'Unclassified'; });
    var counts = sorted.map(function (d) { return parseInt(d.count) || 0; });
    // 색상을 순환 배열로 할당
    var colorKeys = ['warning', 'error', 'primary', 'info', 'accent', 'secondary'];
    var bgColors  = labels.map(function (_, i) { return chartColors[colorKeys[i % colorKeys.length]]; });

    charts.defectiveType.data.labels = labels;
    charts.defectiveType.data.datasets[0].data = counts;
    charts.defectiveType.data.datasets[0].backgroundColor = bgColors;
    charts.defectiveType.data.datasets[0].borderColor      = bgColors;
    charts.defectiveType.update('none');  // 애니메이션 없이 즉시 갱신
}

// 활성/완료 상태 도넛 차트 데이터 갱신
function updateDefectiveStatusChart(s) {
    if (!charts.defectiveStatus || !s) return;
    charts.defectiveStatus.data.datasets[0].data = [
        parseInt(s.warning_count)   || 0,   // 활성(Warning) 건수
        parseInt(s.completed_count) || 0    // 완료(Completed) 건수
    ];
    charts.defectiveStatus.update('none');
}

// 불량 발생 추이 차트 갱신 (시간별/일별 뷰타입 자동 분기)
function updateDefectiveTrendChart(stat) {
    if (!charts.defectiveTrend) return;

    var statsData = stat;
    var workHours = null;
    var viewType  = 'hourly';  // 기본값: 시간별

    // stat이 데이터+메타 구조면 분해
    if (stat && typeof stat === 'object' && stat.data) {
        statsData = stat.data;
        workHours = stat.work_hours;
        viewType  = stat.view_type || 'hourly';
    }

    // 데이터 없으면 No Data 표시
    if (!statsData || statsData.length === 0) {
        charts.defectiveTrend.data.labels = ['No Data'];
        charts.defectiveTrend.data.datasets[0].data = [0];
        charts.defectiveTrend.update('none');
        return;
    }

    // 뷰타입에 따라 X축 레이블 생성
    var labels = [];
    if (viewType === 'hourly' && workHours && workHours.start_time) {
        // 근무 시간 범위에 맞는 시간 레이블 생성
        labels = generateWorkHoursLabels(workHours.start_time, workHours.end_time, workHours.start_minutes, workHours.end_minutes);
    } else if (viewType === 'daily') {
        labels = statsData.map(function (d) { return d.display_label || d.time_label; });
    } else {
        // 'YYYY-MM-DD HH:' 형식에서 시간(HH+'H') 추출
        labels = statsData.map(function (d) {
            var lbl = d.time_label || d.display_label;
            var m   = lbl && lbl.match(/^(\d{4}-\d{2}-\d{2})\s+(\d{2}):/);
            return m ? m[2] + 'H' : lbl;
        });
    }

    // 레이블 키로 건수 맵 생성
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
    charts.defectiveTrend.update('show');  // show 애니메이션으로 갱신
}

// 기계별 불량 건수 차트 갱신
function updateDefectiveMachineChart(stat) {
    if (!charts.defectiveMachine || !stat || !Array.isArray(stat) || stat.length === 0) return;
    charts.defectiveMachine.data.labels  = stat.map(function (d) { return d.machine_no || 'Unknown'; });
    charts.defectiveMachine.data.datasets[0].data = stat.map(function (d) { return parseInt(d.defective_count || d.count) || 0; });
    charts.defectiveMachine.update('none');
}

// 라인별 불량 건수 차트 갱신
function updateDefectiveLineChart(stat) {
    if (!charts.defectiveLine || !stat || !Array.isArray(stat) || stat.length === 0) return;
    charts.defectiveLine.data.labels = stat.map(function (d) { return d.line_name || 'Unknown'; });
    charts.defectiveLine.data.datasets[0].data = stat.map(function (d) { return parseInt(d.defective_count || d.count) || 0; });
    charts.defectiveLine.update('none');
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

// ─── 경과 시간 ────────────────────────────────────────────
// 발생 시각(startTime)부터 현재(Jakarta 시간 기준)까지의 경과 시간 문자열 반환
function calculateElapsedTime(startTime) {
    // Jakarta(WIB, UTC+7) 기준 현재 시각
    var nowJakarta = new Date(new Date().toLocaleString('en-US', { timeZone: 'Asia/Jakarta' }));
    var start      = new Date(startTime.replace(' ', 'T'));
    var diffMs     = nowJakarta - start;
    if (diffMs < 0) return '0s';

    var totalSec = Math.floor(diffMs / 1000);
    var hours    = Math.floor(totalSec / 3600);
    var minutes  = Math.floor((totalSec % 3600) / 60);
    var seconds  = totalSec % 60;

    // 시간/분/초 조합하여 문자열 생성
    var result = '';
    if (hours   > 0) result += hours   + 'h ';
    if (minutes > 0) result += minutes + 'm ';
    if (seconds > 0 || result === '') result += seconds + 's';
    return result.trim();
}

// setInterval 콜백: 활성 불량 패널과 테이블의 경과 시간을 1초마다 갱신
function updateElapsedTimes() {
    // 활성 불량 패널: 최신 5건 경과 시간 갱신
    var sorted = activeDefectives.slice().sort(function (a, b) {
        return new Date(b.reg_date) - new Date(a.reg_date);
    }).slice(0, 5);

    document.querySelectorAll('.active-defective-item').forEach(function (item, i) {
        if (sorted[i] && sorted[i].reg_date) {
            var span = item.querySelector('.defective-elapsed-time span');
            if (span) span.textContent = calculateElapsedTime(sorted[i].reg_date);
        }
    });

    // 테이블 행: Warning 상태 셀의 경과 시간 갱신
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

    // 열린 상세 모달의 경과 시간도 갱신
    if (window.currentModalDefectiveData) {
        var d   = window.currentModalDefectiveData;
        var mel = document.getElementById('modal-elapsed-time');
        if (mel && d.status === 'Warning' && d.reg_date) {
            mel.textContent = calculateElapsedTime(d.reg_date);
        }
    }
}

// 경과 시간 갱신 타이머 시작 (1초 간격 setInterval)
function startElapsedTimeTimer() {
    if (elapsedTimeTimer) clearInterval(elapsedTimeTimer);
    elapsedTimeTimer = setInterval(updateElapsedTimes, 1000);
}

// 경과 시간 갱신 타이머 정지 및 ID 초기화
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
    var f = getFilterParams();
    var p = new URLSearchParams();
    if (f.factory_filter) p.append('factory_filter', f.factory_filter);
    if (f.line_filter)    p.append('line_filter',    f.line_filter);
    if (f.machine_filter) p.append('machine_filter', f.machine_filter);
    if (f.shift_filter)   p.append('shift_filter',   f.shift_filter);
    if (f.start_date)     p.append('start_date',     f.start_date);
    if (f.end_date)       p.append('end_date',       f.end_date);
    window.location.href = 'proc/data_defective_export_2.php?' + p.toString();
}

// 새로고침: SSE 재연결로 데이터 즉시 갱신
async function refreshData() {
    if (isTracking) { stopTracking(); setTimeout(async function () { await startAutoTracking(); }, 1000); }
    else await startAutoTracking();
}

// ─── Modal ────────────────────────────────────────────────
// Details 버튼의 data 속성에서 불량 정보를 파싱하여 모달 오픈
function openDefectiveDetailModal(btn) {
    try {
        var data = JSON.parse(btn.getAttribute('data-defective-data').replace(/&#39;/g, "'"));
        window.currentModalDefectiveData = data;  // 모달 열린 동안 경과 시간 갱신용
        populateDefectiveModal(data);
        var modal = document.getElementById('defectiveDetailModal');
        if (modal) { modal.classList.add('show'); document.body.style.overflow = 'hidden'; }
    } catch (e) { console.error('Modal open error:', e); }
}

// 불량 상세 모달 닫기 (CSS 트랜지션 후 클래스 제거)
function closeDefectiveDetailModal() {
    var modal = document.getElementById('defectiveDetailModal');
    if (!modal) return;
    modal.classList.add('hide');
    window.currentModalDefectiveData = null;
    setTimeout(function () { modal.classList.remove('show', 'hide'); document.body.style.overflow = ''; }, 300);
}

// 모달 DOM 요소에 불량 상세 데이터를 채워넣음
function populateDefectiveModal(d) {
    var set = function (id, v) { var el = document.getElementById(id); if (el) el.textContent = v; };

    set('modal-machine-no',     d.machine_no || '-');
    set('modal-factory-line',   (d.factory_name || '-') + ' / ' + (d.line_name || '-'));
    set('modal-defective-type', d.defective_name || '-');

    // 상태 배지: Warning(빨강) vs Completed(초록)
    var statusEl = document.getElementById('modal-status');
    if (statusEl) {
        statusEl.innerHTML = d.status === 'Warning'
            ? '<span class="fiori-badge fiori-badge--error">Warning</span>'
            : '<span class="fiori-badge fiori-badge--success">Completed</span>';
    }

    set('modal-reg-date',   d.reg_date  || '-');
    // Warning 중이면 경과 시간, 완료면 처리 시간 표시
    var elapsed = d.status === 'Warning'
        ? calculateElapsedTime(d.reg_date)
        : (d.duration_display || d.duration_his || '-');
    set('modal-elapsed-time', elapsed);
    set('modal-work-date',  d.work_date || '-');
    set('modal-shift',      d.shift_idx ? 'Shift ' + d.shift_idx : '-');

    // 불량 유형 색상 표시 (컬러 인디케이터)
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

// 모달에서 단건 내보내기 버튼 클릭 시 (모달만 닫음)
function exportSingleDefective() {
    closeDefectiveDetailModal();
}