/*
 * ============================================================
 * data_oee_2.js
 * ============================================================
 * 목적: OEE(종합설비효율) 데이터 실시간 모니터링 페이지 스크립트
 *
 * 주요 기능:
 *   - SSE(EventSource)를 통해 서버에서 OEE 데이터를 실시간 수신
 *   - Chart.js 5개 차트 렌더링:
 *       oeeTrend       : OEE % 시간/일별 추이 (선 차트)
 *       oeeComponent   : 가용성·성능·품질 레이더 차트
 *       productionTrend: 실제/목표 생산량 추이 (혼합 막대+선)
 *       machineOee     : 기계/라인별 OEE 3요소 막대 차트
 *       oeeGrade       : OEE 등급 분포 도넛 차트
 *   - 날짜 범위 필터(daterangepicker) 및 공장·라인·기계 필터 지원
 *   - 페이지네이션(페이지 당 10건) 및 상세 모달 팝업
 *   - 페이지 숨김/언로드 시 SSE 연결 자동 종료 및 지수 백오프 재연결 로직
 *
 * 연관 백엔드: proc/data_oee_stream_2.php (SSE),
 *              proc/data_oee_export_2.php (Excel 내보내기)
 * ============================================================
 */

// data_oee_2.js — Row Grid Layout version (ai_dashboard_5 model)

/* ── 전역 상태 변수 ─────────────────────────────────────── */
let eventSource = null;          // SSE EventSource 인스턴스
let isTracking = false;          // 현재 SSE 수신 중 여부
let charts = {};                 // Chart.js 인스턴스 맵 (chart 이름 → 인스턴스)
let oeeData = [];                // SSE로 수신된 OEE 데이터 전체 배열
let reconnectAttempts = 0;       // SSE 재연결 시도 횟수
let maxReconnectAttempts = 3;    // 최대 재연결 시도 횟수
let stats = {};                  // 통계 요약 데이터 (API 응답)
let isPageUnloading = false;     // 페이지 언로드 진행 중 여부 플래그

/* ── 페이지네이션 상태 ──────────────────────────────────── */
let currentPage = 1;             // 현재 페이지 번호
let itemsPerPage = 10;           // 페이지당 표시 건수
let totalItems = 0;              // 전체 데이터 건수

/* ── Chart.js 전역 기본 스타일 설정 ────────────────────── */
Chart.defaults.color = '#1a1a1a';
Chart.defaults.borderColor = '#e8eaed';

/* ── OEE 전용 차트 색상 팔레트 정의 ────────────────────── */
const chartColors = {
    oee_overall:  '#0070f2',   // OEE 종합 (파랑)
    availability: '#30914c',   // 가용성 (초록)
    performance:  '#1e88e5',   // 성능 (밝은 파랑)
    quality:      '#e26b0a',   // 품질 (주황)
    target:       '#da1e28',   // 목표선 (빨강)
    production:   '#7c3aed',   // 생산량 (보라)
    excellent:    '#30914c',   // 우수 등급 ≥85% (초록)
    good:         '#0070f2',   // 양호 등급 70~84% (파랑)
    fair:         '#e26b0a',   // 보통 등급 50~69% (주황)
    poor:         '#da1e28'    // 불량 등급 <50% (빨강)
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
// 페이지를 벗어날 때 SSE 연결을 정리
window.addEventListener('beforeunload', () => {
    isPageUnloading = true;
    if (eventSource) { eventSource.close(); eventSource = null; isTracking = false; }
});

// pagehide: 모바일/뒤로가기 등 beforeunload가 발생하지 않는 경우 대비
window.addEventListener('pagehide', () => {
    isPageUnloading = true;
    if (eventSource) { eventSource.close(); eventSource = null; isTracking = false; }
});

// visibilitychange: 탭 전환 시 SSE 일시 중지 및 복귀 시 재연결
document.addEventListener('visibilitychange', () => {
    if (document.hidden) {
        // 탭이 숨겨지면 SSE 연결 해제
        if (eventSource && isTracking) { eventSource.close(); eventSource = null; isTracking = false; }
    } else {
        // 탭이 다시 보이면 SSE 재연결 시도
        if (!isPageUnloading && !isTracking && eventSource === null) {
            reconnectAttempts = 0;
            startAutoTracking();
        }
    }
});

// focus 이벤트: 언로드 플래그 초기화 (탭 포커스 복귀 시)
window.addEventListener('focus', () => { if (isPageUnloading) isPageUnloading = false; });

// ─── 레이아웃 동적 업데이트 (핵심 함수) ──────────────────
// 표시/숨김 Row 상태에 따라 CSS grid-template-rows를 재계산
function updateLayout() {
    const main = document.getElementById('oeeSignageMain');
    if (!main) return;

    const rows = [];
    // 각 Row가 숨김 상태가 아닐 때만 grid 행 크기 추가
    if (!document.getElementById('oeeRowStats').classList.contains('hidden'))        rows.push('auto');
    if (!document.getElementById('oeeRowChartsTop').classList.contains('hidden'))    rows.push('1fr');
    if (!document.getElementById('oeeRowChartsBottom').classList.contains('hidden')) rows.push('1fr');
    if (!document.getElementById('oeeRowTable').classList.contains('hidden'))        rows.push('1fr');
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
    var row = document.getElementById('oeeRowStats');
    var btn = document.getElementById('toggleStatsBtn');
    if (!row || !btn) return;

    row.classList.toggle('hidden');
    btn.textContent = row.classList.contains('hidden') ? 'Show Stats' : 'Hide Stats';
    updateLayout();
}

// 차트(Charts) Row 표시/숨김 토글 (상단/하단 동시)
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

// 데이터 테이블 Row 표시/숨김 토글 (페이지네이션 포함)
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
        var res = await fetch('../manage/proc/factory.php?status_filter=Y').then(r => r.json());
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
        var res = await fetch('../manage/proc/line.php?status_filter=Y').then(r => r.json());
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
        var res = await fetch('../manage/proc/machine.php').then(r => r.json());
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
        var res = await fetch(url).then(r => r.json());
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
        var res = await fetch(url).then(r => r.json());
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
    // OEE 주요 지표 카드 요소들을 '-'로 초기화
    ['overallOee', 'availability', 'performance', 'quality', 'currentShiftOee', 'previousDayOee'].forEach(function (id) {
        var el = document.getElementById(id);
        if (el) el.textContent = '-';
    });
    // OEE 상세 수치 요소들도 '-'로 초기화
    ['runtime', 'plannedTime', 'availabilityDetail', 'actualOutput', 'theoreticalOutput',
        'performanceDetail', 'goodProducts', 'defectiveProducts', 'qualityDetail'].forEach(function (id) {
        var el = document.getElementById(id);
        if (el) el.textContent = '-';
    });
    // 테이블 본문에 로딩 메시지 표시
    var tbody = document.getElementById('oeeDataBody');
    if (tbody) tbody.innerHTML = '<tr><td colspan="10" class="data-table-centered"><div class="fiori-alert fiori-alert--info"><strong>Information:</strong> Loading real-time OEE data.</div></td></tr>';
}

// ─── 차트 초기화 ──────────────────────────────────────────
// Chart.js 차트 5개를 생성하여 charts 객체에 저장
function initCharts() {
    charts.oeeTrend       = createOeeTrendChart();        // OEE 추이 선 차트
    charts.oeeComponent   = createOeeComponentChart();    // 가용성/성능/품질 레이더 차트
    charts.productionTrend = createProductionTrendChart(); // 생산량 추이 혼합 차트
    charts.machineOee     = createMachineOeeChart();      // 기계/라인별 OEE 3요소 차트
    charts.oeeGrade       = createOeeGradeChart();        // OEE 등급 분포 도넛 차트
}

// OEE % 시간/일별 추이를 보여주는 선 차트 생성
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
                backgroundColor: chartColors.oee_overall + '20',  // 10% 투명도 배경
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
                // Y축: 0~100% 범위, '%' 단위 표시
                y: { beginAtZero: true, title: { display: true, text: 'OEE (%)' }, ticks: { callback: function (v) { return v + '%'; } } },
                x: { title: { display: true, text: 'Time/Period' } }
            }
        }
    });
}

// 가용성·성능·품질 3요소를 100% 기준으로 비교하는 레이더 차트 생성
function createOeeComponentChart() {
    var ctx = document.getElementById('oeeComponentChart').getContext('2d');
    return new Chart(ctx, {
        type: 'radar',
        data: {
            labels: ['Availability', 'Performance', 'Quality'],
            datasets: [
                // 현재 OEE 실적
                { label: 'Current OEE', data: [0, 0, 0], borderColor: chartColors.oee_overall, backgroundColor: chartColors.oee_overall + '30', pointRadius: 5 },
                // 목표선 (100% 기준, 점선)
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

// 실제/목표 생산량 추이를 막대(실제)+선(목표)으로 보여주는 혼합 차트 생성
function createProductionTrendChart() {
    var ctx = document.getElementById('productionTrendChart').getContext('2d');
    return new Chart(ctx, {
        type: 'bar',
        data: {
            labels: [],
            datasets: [
                // 실제 생산량: 막대 형태
                { label: 'Actual Output', data: [], backgroundColor: chartColors.production + '80', borderColor: chartColors.production, borderWidth: 1, yAxisID: 'y' },
                // 목표 생산량: 점선 오버레이
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

// 기계/라인별 가용성·성능·품질 3요소를 그룹 막대로 보여주는 차트 생성
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
            plugins: {
                legend: { display: true, position: 'top' },
                // 툴팁에 '%' 단위 추가
                tooltip: { callbacks: { label: function (c) { return c.dataset.label + ': ' + c.parsed.y + '%'; } } }
            },
            scales: {
                y: { beginAtZero: true, suggestedMax: 100, title: { display: true, text: 'Performance (%)' }, ticks: { callback: function (v) { return v + '%'; } } },
                x: { title: { display: true, text: 'Line Name' } }
            }
        }
    });
}

// OEE 등급 분포(우수/양호/보통/불량)를 보여주는 도넛 차트 생성
function createOeeGradeChart() {
    var ctx = document.getElementById('oeeGradeChart').getContext('2d');
    return new Chart(ctx, {
        type: 'doughnut',
        data: {
            // OEE 등급: ≥85% 우수, 70~84% 양호, 50~69% 보통, <50% 불량
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
                // 툴팁에 건수와 비율(%) 동시 표시
                tooltip: { callbacks: { label: function (c) { var t = c.dataset.data.reduce(function (a, b) { return a + b; }, 0); var p = t > 0 ? ((c.raw / t) * 100).toFixed(1) : 0; return c.label + ': ' + c.raw + ' (' + p + '%)'; } } }
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

// SSE EventSource를 생성하고 실시간 OEE 데이터 수신을 시작
async function startAutoTracking() {
    if (isTracking) return;  // 이미 추적 중이면 중복 실행 방지
    var params = new URLSearchParams(getFilterParams());
    // EventSource: 서버에서 OEE 데이터를 SSE로 스트리밍
    eventSource = new EventSource('proc/data_oee_stream_2.php?' + params.toString());
    setupSSEEventListeners();
    isTracking = true;
    var el = document.getElementById('connectionStatus');
    if (el) el.textContent = 'OEE System Real-time Connected';
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
        if (el) el.textContent = 'OEE system connected';
    });

    // 'oee_data' 이벤트: 실제 OEE 데이터 수신 및 UI 갱신
    eventSource.addEventListener('oee_data', function (e) {
        var data = JSON.parse(e.data);
        stats   = data.stats;         // 통계 요약
        oeeData = data.oee_data;      // 전체 OEE 데이터
        window.oeeData = data.oee_data;  // 전역 참조 (차트 폴백용)
        // UI 각 영역 순서대로 갱신
        updateStatCardsFromAPI(stats);
        updateOeeDetailsFromAPI(data.oee_details);
        updateTableFromAPI(oeeData);
        updateChartsFromAPI(data);
        // 마지막 업데이트 시각 표시
        var el = document.getElementById('lastUpdateTime');
        if (el) el.textContent = 'Last updated: ' + data.timestamp;
    });

    // 'heartbeat' 이벤트: 연결 유지 확인 (활성 기계 수 표시)
    eventSource.addEventListener('heartbeat', function (e) {
        var data = JSON.parse(e.data);
        var el = document.getElementById('connectionStatus');
        if (el) el.textContent = 'Connection maintained (Active machines: ' + data.active_machines + ')';
    });

    // 'disconnected' 이벤트: 서버에서 연결 종료 통보
    eventSource.addEventListener('disconnected', function () {
        var el = document.getElementById('connectionStatus');
        if (el) el.textContent = 'OEE System Connection Closed';
    });
}

// SSE 연결을 수동으로 종료하고 상태를 초기화
function stopTracking() {
    if (!isTracking) return;
    if (eventSource) { eventSource.close(); eventSource = null; }
    isTracking = false;
    var el = document.getElementById('connectionStatus');
    if (el) el.textContent = 'OEE System Connection Closed';
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
// 숫자값을 % 형식으로 포맷 (소수이면 소수점 그대로, 정수이면 정수로)
function formatPercentage(value) {
    var n = parseFloat(value);
    if (isNaN(n)) return '0%';
    return (n % 1 === 0 ? Math.floor(n) : n) + '%';
}

// 숫자값을 지정 소수 자릿수로 포맷 (기본 2자리)
function formatDecimal(value, decimals) {
    var n = parseFloat(value);
    return isNaN(n) ? '-' : n.toFixed(decimals || 2);
}

// SSE로 수신된 통계 데이터로 상단 OEE 통계 카드를 갱신
function updateStatCardsFromAPI(s) {
    if (!s) return;
    // DOM 요소 ID와 포맷된 표시 값의 매핑
    var map = {
        overallOee:      formatPercentage(s.overall_oee || 0),        // 종합 OEE
        availability:    formatPercentage(s.availability || 0),        // 가용성
        performance:     formatPercentage(s.performance || 0),         // 성능
        quality:         formatPercentage(s.quality || 0),             // 품질
        currentShiftOee: formatPercentage(s.current_shift_oee || 0),  // 현재 교대 OEE
        previousDayOee:  formatPercentage(s.previous_day_oee || 0)    // 전일 OEE
    };
    // 각 DOM 요소의 텍스트를 매핑된 값으로 일괄 업데이트
    Object.keys(map).forEach(function (id) {
        var el = document.getElementById(id);
        if (el) el.textContent = map[id];
    });
}

// OEE 상세 패널(가용성/성능/품질 분해 수치)을 갱신
function updateOeeDetailsFromAPI(d) {
    if (!d) return;
    var map = {
        oeeRateDetail:      formatPercentage(d.overall_oee || 0),
        overallEfficiency:  formatPercentage(d.overall_oee || 0),
        targetAchievement:  d.target_achievement || '-',
        // runtime 필드: downtime 값이 있으면 downtime 표시 (가용성 계산 기반)
        runtime:            d.downtime != null ? d.downtime : (d.runtime || '-'),
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

// 페이지네이션을 적용하여 OEE 데이터 테이블을 렌더링
function updateTableFromAPI(list) {
    var tbody = document.getElementById('oeeDataBody');
    if (!tbody) return;
    totalItems = list.length;  // 전체 건수 갱신 (페이지네이션 계산용)

    // 데이터가 없으면 안내 메시지 표시
    if (list.length === 0) {
        tbody.innerHTML = '<tr><td colspan="10" class="data-table-centered"><div class="fiori-alert fiori-alert--info"><strong>Information:</strong> No OEE data matching the selected conditions.</div></td></tr>';
        renderPagination();
        return;
    }

    // 현재 페이지에 해당하는 데이터 슬라이스
    var start = (currentPage - 1) * itemsPerPage;
    var paged = list.slice(start, start + itemsPerPage);
    tbody.innerHTML = '';

    // 각 행 렌더링: OEE 수치별 등급 배지 색상 적용
    paged.forEach(function (oee) {
        // OEE 등급 배지: ≥85% 성공(초록), ≥70% 경고(주황), 미만 오류(빨강)
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
            // 가용성: 항상 success(초록) 배지
            '<td><span class="fiori-badge fiori-badge--success">' + (oee.availability || 0) + '%</span></td>' +
            // 성능: info(파랑) 배지
            '<td><span class="fiori-badge fiori-badge--info">' + (oee.performance || 0) + '%</span></td>' +
            // 품질: warning(주황) 배지
            '<td><span class="fiori-badge fiori-badge--warning">' + (oee.quality || 0) + '%</span></td>' +
            '<td>' + (oee.work_date || '-') + '</td>' +
            '<td>' + (oee.update_date || '-') + '</td>' +
            '<td><button class="fiori-btn fiori-btn--tertiary oee-details-btn" style="padding:.25rem .5rem;font-size:.75rem;" data-oee-data=\'' + JSON.stringify(oee).replace(/'/g, "&#39;") + '\'>Details</button></td>';
        tbody.appendChild(tr);
    });

    renderPagination();
    setupDetailsButtonListeners();
}

// 테이블의 각 Details 버튼에 클릭 이벤트 리스너를 등록
function setupDetailsButtonListeners() {
    document.querySelectorAll('.oee-details-btn').forEach(function (btn) {
        btn.removeEventListener('click', handleDetailsButtonClick);  // 중복 방지
        btn.addEventListener('click', handleDetailsButtonClick);
    });
}

// Details 버튼 클릭 핸들러: 이벤트 전파를 막고 OEE 상세 모달 오픈
function handleDetailsButtonClick(e) {
    e.preventDefault();
    e.stopPropagation();
    var btn = e.target.closest('.oee-details-btn');
    if (btn) openOeeDetailModal(btn);
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
    updateTableFromAPI(oeeData);
}

// ─── 차트 업데이트 ────────────────────────────────────────
// SSE 수신 데이터로 5개 차트를 모두 갱신
function updateChartsFromAPI(data) {
    if (data.oee_trend_stats && charts.oeeTrend)         updateOeeTrendChart(data.oee_trend_stats);
    if (data.oee_component_stats && charts.oeeComponent) updateOeeComponentChart(data.oee_component_stats);
    if (data.production_trend_stats && charts.productionTrend) updateProductionTrendChart(data.production_trend_stats);
    if (data.machine_oee_stats && charts.machineOee)     updateMachineOeeChart(data.machine_oee_stats);
    if (data.stats && charts.oeeGrade)                   updateOeeGradeChart(data.stats);
}

// OEE 추이 차트 갱신 (시간별/일별 뷰타입 자동 분기)
function updateOeeTrendChart(stat) {
    if (!charts.oeeTrend) return;
    // API 데이터 없으면 수신 데이터로 단순 추이 생성
    if (!stat || !stat.data || stat.data.length === 0) {
        if (window.oeeData && window.oeeData.length > 0) { generateSimpleTrendFromOeeData(); return; }
        charts.oeeTrend.data.labels = ['No Data'];
        charts.oeeTrend.data.datasets[0].data = [0];
        charts.oeeTrend.update('none');
        return;
    }
    var trendData = stat.data;
    var workHours = stat.work_hours;
    var viewType  = stat.view_type || 'hourly';  // 기본값: 시간별
    var labels = [];

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

    // OEE % 값을 레이블 키로 맵핑
    var valMap = {};
    trendData.forEach(function (d) {
        var key = viewType === 'daily' ? (d.display_label || d.time_label) : (function () { var m = (d.time_label || '').match(/^(\d{4}-\d{2}-\d{2})\s+(\d{2}):/); return m ? m[2] + 'H' : ''; })();
        if (key) valMap[key] = parseFloat(d.overall_oee) || 0;
    });

    charts.oeeTrend.data.labels = labels;
    charts.oeeTrend.data.datasets[0].data = labels.map(function (l) { return valMap[l] || 0; });
    charts.oeeTrend.options.scales.x.title.text = viewType === 'hourly' ? 'Time (within 1 day)' : 'Date';
    charts.oeeTrend.update('show');  // show 애니메이션으로 갱신
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

// API 추이 데이터가 없을 때 수신된 oeeData에서 날짜별 OEE 평균 계산
function generateSimpleTrendFromOeeData() {
    if (!charts.oeeTrend || !window.oeeData || window.oeeData.length === 0) return;
    // 작업일(work_date)별 OEE 합계와 건수 집계
    var grp = {};
    window.oeeData.forEach(function (d) {
        if (!d.work_date) return;
        if (!grp[d.work_date]) grp[d.work_date] = { sum: 0, cnt: 0 };
        grp[d.work_date].sum += parseFloat(d.overall_oee) || 0;
        grp[d.work_date].cnt++;
    });
    var dates = Object.keys(grp).sort();
    // X축 레이블: M/D 형식으로 변환
    charts.oeeTrend.data.labels = dates.map(function (d) { var x = new Date(d); return (x.getMonth() + 1) + '/' + x.getDate(); });
    // 날짜별 OEE 평균값으로 데이터 설정
    charts.oeeTrend.data.datasets[0].data = dates.map(function (d) { return grp[d].cnt > 0 ? grp[d].sum / grp[d].cnt : 0; });
    charts.oeeTrend.options.scales.x.title.text = 'Date';
    charts.oeeTrend.update('show');
}

// 가용성·성능·품질 레이더 차트 데이터 갱신
function updateOeeComponentChart(stat) {
    if (!charts.oeeComponent || !stat) return;
    // 레이더 차트의 3개 꼭짓점 값 업데이트
    charts.oeeComponent.data.datasets[0].data = [parseFloat(stat.availability) || 0, parseFloat(stat.performance) || 0, parseFloat(stat.quality) || 0];
    charts.oeeComponent.update('none');
}

// 생산량 추이 혼합 차트 갱신 (실제/목표 생산량)
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

    // 뷰타입에 따라 X축 레이블 생성
    if (viewType === 'hourly' && workHours && workHours.start_time) {
        labels = generateWorkHoursLabels(workHours.start_time, workHours.end_time, workHours.start_minutes, workHours.end_minutes);
    } else if (viewType === 'daily') {
        labels = trendData.map(function (d) { return d.display_label || d.time_label; });
    } else {
        labels = trendData.map(function (d) { var m = (d.time_label || '').match(/^(\d{4}-\d{2}-\d{2})\s+(\d{2}):/); return m ? m[2] + 'H' : d.time_label; });
    }

    // 실제/목표 생산량을 레이블 키로 맵핑
    var actMap = {}, tgtMap = {};
    trendData.forEach(function (d) {
        var key = viewType === 'daily' ? (d.display_label || d.time_label) : (function () { var m = (d.time_label || '').match(/^(\d{4}-\d{2}-\d{2})\s+(\d{2}):/); return m ? m[2] + 'H' : ''; })();
        if (key) { actMap[key] = parseInt(d.actual_output) || 0; tgtMap[key] = parseInt(d.target_output) || 0; }
    });

    charts.productionTrend.data.labels = labels;
    charts.productionTrend.data.datasets[0].data = labels.map(function (l) { return actMap[l] || 0; });  // 실제 생산량
    charts.productionTrend.data.datasets[1].data = labels.map(function (l) { return tgtMap[l] || 0; });  // 목표 생산량
    charts.productionTrend.options.scales.x.title.text = viewType === 'hourly' ? 'Time (within 1 day)' : 'Date';
    charts.productionTrend.update('show');
}

// 기계/라인별 OEE 3요소 차트 갱신
function updateMachineOeeChart(stat) {
    if (!charts.machineOee || !stat || !Array.isArray(stat) || stat.length === 0) return;
    // machine_no 속성 유무로 기계별/라인별 구분
    var isMachine = stat[0].hasOwnProperty('machine_no');
    var labels = stat.map(function (d) { return isMachine ? (d.machine_no || 'Unknown') : (d.line_name || 'Unknown'); });

    charts.machineOee.data.labels = labels;
    charts.machineOee.data.datasets[0].data = stat.map(function (d) { return parseFloat(d.availability) || 0; });  // 가용성
    charts.machineOee.data.datasets[1].data = stat.map(function (d) { return parseFloat(d.performance)  || 0; });  // 성능
    charts.machineOee.data.datasets[2].data = stat.map(function (d) { return parseFloat(d.quality)      || 0; });  // 품질
    charts.machineOee.options.scales.x.title.text = isMachine ? 'Machine Number' : 'Line Name';

    // 차트 제목/부제목 동적 변경
    var titleEl    = document.getElementById('oeeLineOeeCardTitle');
    var subtitleEl = document.getElementById('oeeLineOeeCardSubtitle');
    if (titleEl)    titleEl.textContent    = isMachine ? 'Machine OEE Performance' : 'Line OEE Performance';
    if (subtitleEl) subtitleEl.textContent = isMachine ? 'OEE performance comparison by machine' : 'OEE performance comparison by production line';

    charts.machineOee.update('none');
}

// OEE 등급 분포 도넛 차트 갱신 (우수/양호/보통/불량 건수)
function updateOeeGradeChart(s) {
    if (!charts.oeeGrade || !s) return;
    charts.oeeGrade.data.datasets[0].data = [
        parseInt(s.excellent_count) || 0,  // ≥85%
        parseInt(s.good_count)      || 0,  // 70~84%
        parseInt(s.fair_count)      || 0,  // 50~69%
        parseInt(s.poor_count)      || 0   // <50%
    ];
    charts.oeeGrade.update('none');
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

// 새로고침: SSE 재연결로 데이터 즉시 갱신
async function refreshData() {
    if (isTracking) { stopTracking(); setTimeout(async function () { await startAutoTracking(); }, 1000); }
    else await startAutoTracking();
}

// ─── Modal ────────────────────────────────────────────────
// Details 버튼의 data 속성에서 OEE 정보를 파싱하여 모달 오픈
function openOeeDetailModal(btn) {
    try {
        var data = JSON.parse(btn.getAttribute('data-oee-data'));
        populateOeeModal(data);
        var modal = document.getElementById('oeeDetailModal');
        if (modal) { modal.classList.add('show'); document.body.style.overflow = 'hidden'; }
    } catch (e) { console.error('Modal open error:', e); }
}

// OEE 상세 모달 닫기 (CSS 트랜지션 후 클래스 제거)
function closeOeeDetailModal() {
    var modal = document.getElementById('oeeDetailModal');
    if (!modal) return;
    modal.classList.add('hide');
    setTimeout(function () { modal.classList.remove('show', 'hide'); document.body.style.overflow = ''; }, 300);
}

// 모달 DOM 요소에 OEE 상세 데이터를 채워넣음
function populateOeeModal(d) {
    var set = function (id, v) { var el = document.getElementById(id); if (el) el.textContent = v; };
    set('modal-machine-no',        d.machine_no || '-');
    set('modal-factory-line',      (d.factory_name || '-') + ' / ' + (d.line_name || '-'));
    set('modal-work-date',         d.work_date || '-');
    set('modal-shift',             d.shift_idx ? 'Shift ' + d.shift_idx : '-');
    // OEE 3요소 및 종합 OEE 포맷 후 표시
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

// 모달에서 단건 내보내기 버튼 클릭 시 (모달만 닫음)
function exportSingleOee() {
    closeOeeDetailModal();
}