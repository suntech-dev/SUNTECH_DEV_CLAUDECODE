/**
 * ============================================================
 * data_andon_2.js
 * ============================================================
 * 파일 목적:
 *   안돈(Andon) 데이터 실시간 모니터링 페이지의 핵심 JavaScript.
 *   SSE(Server-Sent Events)를 통해 서버에서 안돈 이벤트 데이터를
 *   스트리밍 수신하고, Chart.js 차트·통계 카드·데이터 테이블을
 *   실시간으로 갱신한다.
 *
 * 주요 기능:
 *   - SSE 실시간 연결 / 재연결 (지수 백오프)
 *   - 공장·라인·기계 필터 시스템 (동적 연계 셀렉트박스)
 *   - DateRangePicker 날짜 범위 필터
 *   - Chart.js 바 차트(안돈 유형별) + 라인 차트(시간별 트렌드)
 *   - 활성 안돈 카드 실시간 경과시간 표시 (setInterval 1초)
 *   - SAP Fiori 스타일 페이지네이션
 *   - Excel 내보내기 (proc/data_andon_export_2.php)
 *   - 안돈 상세 모달 팝업
 * ============================================================
 */

/* ── 전역 상태 변수 ── */
let eventSource = null;          // SSE EventSource 인스턴스
let isTracking = false;          // 현재 SSE 추적 중 여부
let charts = {};                 // Chart.js 인스턴스 저장 객체
let andonData = [];              // 전체 안돈 데이터 배열 (페이지네이션 소스)
let activeAndons = [];           // 현재 활성(Warning) 안돈 목록
let reconnectAttempts = 0;       // SSE 재연결 시도 횟수
let maxReconnectAttempts = 3;    // 최대 재연결 시도 횟수
let stats = {};                  // 통계 카드용 데이터 객체
let elapsedTimeTimer = null;     // 활성 안돈 경과시간 갱신 타이머 ID
let durationUpdateTimer = null;  // 테이블 내 진행중 항목 경과시간 타이머 ID
let isPageUnloading = false;     // 페이지 언로드 중 여부 (불필요한 재연결 방지)

/* ── 페이지네이션 상태 ── */
let currentPage = 1;    // 현재 페이지 번호
let itemsPerPage = 10;  // 페이지당 표시 행 수
let totalItems = 0;     // 전체 데이터 건수

/* ── Chart.js 전역 기본값 설정 ── */
Chart.defaults.color = '#1a1a1a';        // 차트 기본 텍스트 색상
Chart.defaults.borderColor = '#e8eaed'; // 차트 기본 테두리 색상

/* ── 차트 색상 팔레트 ── */
const chartColors = {
    warning:   '#e26b0a', // 경고 (주황)
    error:     '#da1e28', // 오류 (빨강)
    success:   '#30914c', // 정상 (초록)
    info:      '#7c3aed', // 정보 (보라)
    primary:   '#0070f2', // 기본 (파랑)
    secondary: '#1e88e5', // 보조 (연파랑)
    accent:    '#00d4aa'  // 강조 (민트)
};

/**
 * DOMContentLoaded 이벤트 핸들러
 * 페이지 로드 완료 후 모든 초기화 작업을 순차적으로 수행한다.
 */
document.addEventListener('DOMContentLoaded', async function () {
    initDateRangePicker();       // 날짜 범위 선택기 초기화
    await initFilterSystem();    // 공장/라인/기계 필터 셀렉트박스 로드
    initCharts();                // Chart.js 차트 인스턴스 생성
    setupEventListeners();       // 버튼·셀렉트박스 이벤트 리스너 등록
    updateLayout(); // 초기 grid-template-rows 설정
    await startAutoTracking();   // SSE 실시간 데이터 수신 시작
});

// ─── 페이지 언로드 처리 ──────────────────────────────────
// 페이지를 닫거나 다른 페이지로 이동할 때 SSE 연결을 안전하게 종료한다.
window.addEventListener('beforeunload', () => {
    isPageUnloading = true;
    stopAllTimers(); // 실행 중인 모든 인터벌 타이머 정지
    if (eventSource) { eventSource.close(); eventSource = null; isTracking = false; }
});

// 모바일 브라우저의 페이지 캐시(bfcache) 처리
window.addEventListener('pagehide', () => {
    isPageUnloading = true;
    stopAllTimers();
    if (eventSource) { eventSource.close(); eventSource = null; isTracking = false; }
});

/**
 * 탭 가시성 변경 이벤트 핸들러
 * 탭이 숨겨지면 SSE를 종료하고, 다시 표시되면 재연결한다.
 * (백그라운드 탭에서 불필요한 서버 연결을 방지)
 */
document.addEventListener('visibilitychange', () => {
    if (document.hidden) {
        // 탭이 비활성화되면 타이머와 SSE 중단
        stopAllTimers();
        if (eventSource && isTracking) { eventSource.close(); eventSource = null; isTracking = false; }
    } else {
        // 탭이 다시 활성화되면 SSE 재연결 시도
        if (!isPageUnloading && !isTracking && eventSource === null) {
            reconnectAttempts = 0;
            startAutoTracking();
        }
    }
});

// 페이지 포커스 복귀 시 언로드 플래그 초기화
window.addEventListener('focus', () => { if (isPageUnloading) isPageUnloading = false; });

// ─── 레이아웃 동적 업데이트 (핵심 함수) ──────────────────
/**
 * CSS Grid 레이아웃의 행 크기를 동적으로 재계산한다.
 * 통계·차트·테이블 행의 hidden 상태에 따라
 * grid-template-rows 값을 런타임에 변경한다.
 * 차트 리사이즈도 함께 트리거한다.
 */
function updateLayout() {
    var main = document.getElementById('andonSignageMain');
    if (!main) return;

    var rows = [];
    // 각 섹션의 표시 여부에 따라 행 크기 배열 구성
    if (!document.getElementById('andonRowStats').classList.contains('hidden'))        rows.push('auto');
    if (!document.getElementById('andonRowChartsTop').classList.contains('hidden'))    rows.push('1fr');
    if (!document.getElementById('andonRowChartsBottom').classList.contains('hidden')) rows.push('1fr');
    if (!document.getElementById('andonRowTable').classList.contains('hidden'))        rows.push('1fr');
    rows.push('auto'); // pagination 영역은 항상 표시

    // grid-template-rows 적용
    main.style.gridTemplateRows = rows.join(' ');

    // 레이아웃 변경 후 Chart.js가 새 크기에 맞게 리사이즈하도록 지연 호출
    setTimeout(function () {
        Object.values(charts).forEach(function (c) { if (c) c.resize(); });
    }, 50);
}

// ─── 토글 함수 ────────────────────────────────────────────
/**
 * 통계 카드 섹션의 표시/숨김을 토글한다.
 */
function toggleStatsDisplay() {
    var row = document.getElementById('andonRowStats');
    var btn = document.getElementById('toggleStatsBtn');
    if (!row || !btn) return;
    row.classList.toggle('hidden');
    btn.textContent = row.classList.contains('hidden') ? 'Show Stats' : 'Hide Stats';
    updateLayout(); // 레이아웃 재계산
}

/**
 * 차트 섹션(상단/하단 모두)의 표시/숨김을 토글한다.
 */
function toggleChartsDisplay() {
    var rowTop    = document.getElementById('andonRowChartsTop');
    var rowBottom = document.getElementById('andonRowChartsBottom');
    var btn       = document.getElementById('toggleChartsBtn');
    if (!rowTop || !rowBottom || !btn) return;
    var isHidden = rowTop.classList.contains('hidden');
    // 현재 숨김 상태이면 표시로, 표시 상태이면 숨김으로 전환
    rowTop.classList.toggle('hidden', !isHidden);
    rowBottom.classList.toggle('hidden', !isHidden);
    btn.textContent = isHidden ? 'Hide Charts' : 'Show Charts';
    updateLayout();
}

/**
 * 데이터 테이블 섹션과 페이지네이션의 표시/숨김을 토글한다.
 */
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
/**
 * jQuery DateRangePicker 플러그인을 초기화한다.
 * 오늘/어제/최근 1주/최근 1달 등 사전 정의 범위를 제공하며,
 * 날짜 변경 시 handleDateRangeChange()를 호출해 SSE를 재시작한다.
 */
function initDateRangePicker() {
    try {
        $('#dateRangePicker').daterangepicker({
            opens: 'left', // 캘린더가 입력창 왼쪽으로 열림
            locale: {
                format: 'YYYY-MM-DD',
                separator: ' ~ ',
                applyLabel: 'Apply',
                cancelLabel: 'Cancel',
                daysOfWeek: ['Sun', 'Mon', 'Tue', 'Wed', 'Thu', 'Fri', 'Sat'],
                monthNames: ['Jan', 'Feb', 'Mar', 'Apr', 'May', 'Jun', 'Jul', 'Aug', 'Sep', 'Oct', 'Nov', 'Dec'],
                firstDay: 0 // 주 시작: 일요일
            },
            ranges: {
                'Today':      [moment(), moment()],
                'Yesterday':  [moment().subtract(1, 'days'), moment().subtract(1, 'days')],
                'Last Week':  [moment().subtract(6, 'days'), moment()],
                'Last Month': [moment().subtract(29, 'days'), moment()]
            },
            startDate: moment().startOf('day'),
            endDate:   moment().endOf('day'),
            showDropdowns: true,         // 연/월 드롭다운 표시
            alwaysShowCalendars: true    // 사전 정의 범위와 캘린더 동시 표시
        }, function (start, end) {
            // 날짜 범위가 선택되었을 때 SSE 재시작
            handleDateRangeChange(start, end);
        });
        // 초기값: 오늘 날짜로 설정
        $('#dateRangePicker').val(moment().format('YYYY-MM-DD') + ' ~ ' + moment().format('YYYY-MM-DD'));
    } catch (e) {
        console.error('DateRangePicker init error:', e);
    }
}

// ─── 필터 시스템 ──────────────────────────────────────────
/**
 * 공장·라인·기계 필터 셀렉트박스를 순서대로 초기화한다.
 * 각 옵션은 서버 API에서 동적으로 로드된다.
 */
async function initFilterSystem() {
    await loadFactoryOptions();  // 공장 목록 로드
    await loadLineOptions();     // 라인 목록 로드
    await loadMachineOptions();  // 기계 목록 로드
}

/**
 * 공장 필터 셀렉트박스에 활성 공장 목록을 로드한다.
 * 공장 선택 시 라인 목록을 연계 갱신하고 SSE를 재시작한다.
 */
async function loadFactoryOptions() {
    try {
        // 활성 공장만 필터링하여 가져옴 (status_filter=Y)
        var res = await fetch('../manage/proc/factory.php?status_filter=Y').then(r => r.json());
        if (!res.success || !res.data) return;
        var sel = document.getElementById('factoryFilterSelect');
        sel.innerHTML = '<option value="">All Factory</option>';
        // 각 공장을 옵션으로 추가
        res.data.forEach(function (f) {
            sel.innerHTML += '<option value="' + f.idx + '">' + f.factory_name + '</option>';
        });
        // 공장 선택 변경 시: 기계 초기화 → 라인 재로드 → SSE 재시작
        sel.addEventListener('change', async function (e) {
            document.getElementById('factoryLineMachineFilterSelect').innerHTML = '<option value="">All Machine</option>';
            document.getElementById('factoryLineMachineFilterSelect').disabled = true;
            await updateLineOptions(e.target.value);
            await restartRealTimeMonitoring();
        });
    } catch (e) { console.error('Factory options error:', e); }
}

/**
 * 라인 필터 셀렉트박스에 활성 라인 목록을 로드한다.
 * 라인 선택 시 기계 목록을 연계 갱신하고 SSE를 재시작한다.
 */
async function loadLineOptions() {
    try {
        var res = await fetch('../manage/proc/line.php?status_filter=Y').then(r => r.json());
        if (!res.success || !res.data) return;
        var sel = document.getElementById('factoryLineFilterSelect');
        sel.innerHTML = '<option value="">All Line</option>';
        res.data.forEach(function (l) {
            sel.innerHTML += '<option value="' + l.idx + '">' + l.line_name + '</option>';
        });
        // 라인 선택 변경 시: 기계 목록 재로드 → SSE 재시작
        sel.addEventListener('change', async function (e) {
            await updateMachineOptions(document.getElementById('factoryFilterSelect').value, e.target.value);
            await restartRealTimeMonitoring();
        });
    } catch (e) { console.error('Line options error:', e); }
}

/**
 * 기계 필터 셀렉트박스에 전체 기계 목록을 로드한다.
 * (초기 로드 시 필터 없이 전체 기계를 가져옴)
 */
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

/**
 * 공장 ID가 변경되었을 때 라인 필터 셀렉트박스를 갱신한다.
 * @param {string} factoryId - 선택된 공장의 idx 값
 */
async function updateLineOptions(factoryId) {
    var lineSelect = document.getElementById('factoryLineFilterSelect');
    lineSelect.disabled = true; // 로드 중 비활성화
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
        // 공장이 선택된 경우 기계 셀렉트박스도 활성화
        if (factoryId) { machineSelect.disabled = false; updateMachineOptions(factoryId, ''); }
    } catch (e) {
        lineSelect.innerHTML = '<option value="">All Line</option>';
        lineSelect.disabled = false;
    }
}

/**
 * 공장/라인 ID가 변경되었을 때 기계 필터 셀렉트박스를 갱신한다.
 * @param {string} factoryId - 선택된 공장의 idx 값
 * @param {string} lineId    - 선택된 라인의 idx 값
 */
async function updateMachineOptions(factoryId, lineId) {
    var machineSelect = document.getElementById('factoryLineMachineFilterSelect');
    machineSelect.disabled = true; // 로드 중 비활성화
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
/**
 * 페이지에서 사용하는 모든 Chart.js 인스턴스를 생성하고
 * charts 객체에 저장한다.
 */
function initCharts() {
    charts.andonType  = createAndonTypeChart();  // 안돈 유형별 발생 횟수 바 차트
    charts.andonTrend = createAndonTrendChart(); // 시간대별 안돈 추이 라인 차트
}

/**
 * 안돈 유형별 발생 건수를 보여주는 수직 바 차트를 생성한다.
 * SSE 데이터 수신 후 updateAndonTypeChart()에서 데이터가 채워진다.
 */
function createAndonTypeChart() {
    var ctx = document.getElementById('andonTypeChart').getContext('2d');
    return new Chart(ctx, {
        type: 'bar', // 수직 바 차트
        data: {
            labels: [], // 안돈 유형명 (동적으로 채워짐)
            datasets: [{
                label: 'Andon Occurrence Count',
                data: [],
                // 안돈 유형별로 다른 색상 적용
                backgroundColor: [chartColors.warning, chartColors.error, chartColors.primary, chartColors.info, chartColors.accent, chartColors.secondary],
                borderColor:     [chartColors.warning, chartColors.error, chartColors.primary, chartColors.info, chartColors.accent, chartColors.secondary],
                borderWidth: 1
            }]
        },
        options: {
            responsive: true,
            maintainAspectRatio: false, // 컨테이너 높이에 맞게 조절
            plugins: { legend: { display: false } }, // 범례 숨김 (단일 데이터셋)
            scales: {
                y: { beginAtZero: true, ticks: { stepSize: 1, precision: 0 }, title: { display: true, text: 'Andon Count' } },
                x: { title: { display: true, text: 'Andon Type' } }
            }
        }
    });
}

/**
 * 시간대별 안돈 Warning/Completed 추이를 보여주는 라인 차트를 생성한다.
 * 2개 데이터셋(Warning, Completed)으로 구성된다.
 */
function createAndonTrendChart() {
    var ctx = document.getElementById('andonTrendChart').getContext('2d');
    return new Chart(ctx, {
        type: 'line', // 라인(꺾은선) 차트
        data: {
            labels: [],
            datasets: [
                {
                    label: 'Warning',
                    data: [],
                    borderColor: chartColors.error,
                    backgroundColor: chartColors.error + '20', // 20 = 12% 투명도 (16진수)
                    fill: true, tension: 0.4, // 부드러운 곡선
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
                tooltip: { mode: 'index', intersect: false } // 같은 x축 위치의 두 값 동시 표시
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
/**
 * 페이지 내 모든 인터랙티브 요소에 이벤트 리스너를 등록한다.
 * - 버튼 클릭: 내보내기, 새로고침, 섹션 토글
 * - 셀렉트박스 변경: 시간 범위, 기계 필터, 근무조 필터
 * - 키보드: Escape 키로 모달 닫기
 */
function setupEventListeners() {
    var toggleStatsBtn   = document.getElementById('toggleStatsBtn');
    var toggleChartsBtn  = document.getElementById('toggleChartsBtn');
    var toggleDataBtn    = document.getElementById('toggleDataBtn');
    var excelDownloadBtn = document.getElementById('excelDownloadBtn'); // Excel 내보내기 버튼
    var refreshBtn       = document.getElementById('refreshBtn');       // 수동 새로고침 버튼
    var timeRangeSelect  = document.getElementById('timeRangeSelect');  // 시간 범위 셀렉트
    var machineSelect    = document.getElementById('factoryLineMachineFilterSelect');
    var shiftSelect      = document.getElementById('shiftSelect');       // 근무조 선택

    if (toggleStatsBtn)   toggleStatsBtn.addEventListener('click', toggleStatsDisplay);
    if (toggleChartsBtn)  toggleChartsBtn.addEventListener('click', toggleChartsDisplay);
    if (toggleDataBtn)    toggleDataBtn.addEventListener('click', toggleDataDisplay);
    if (excelDownloadBtn) excelDownloadBtn.addEventListener('click', exportData);
    if (refreshBtn)       refreshBtn.addEventListener('click', refreshData);
    // 시간 범위 변경: DateRangePicker 값 동기화 + SSE 재시작
    if (timeRangeSelect)  timeRangeSelect.addEventListener('change', handleTimeRangeChange);
    // 기계 필터 변경: 즉시 SSE 재시작
    if (machineSelect)    machineSelect.addEventListener('change', async function () { await restartRealTimeMonitoring(); });
    // 근무조 변경: 즉시 SSE 재시작
    if (shiftSelect)      shiftSelect.addEventListener('change', async function () { await restartRealTimeMonitoring(); });

    // Escape 키 → 열린 모달 닫기
    document.addEventListener('keydown', function (e) {
        if (e.key === 'Escape') {
            var modal = document.getElementById('andonDetailModal');
            if (modal && modal.classList.contains('show')) closeAndonDetailModal();
        }
    });
}

// ─── 초기 데이터 로딩 ────────────────────────────────────
/**
 * 페이지 최초 진입 시 통계 카드와 활성 안돈 카운트를 '-'로 초기화한다.
 * 실제 데이터는 SSE 연결 후 채워진다.
 */
async function loadInitialData() {
    // 통계 카드 ID 목록을 순회하며 빈 상태('-')로 초기화
    var statIds = ['totalAndons', 'activeWarnings', 'currentShiftCount', 'affectedMachines', 'urgentWarnings', 'avgCompletedTime'];
    statIds.forEach(function (id) {
        var el = document.getElementById(id);
        if (el) el.textContent = '-';
    });
    // 활성 안돈 카운트 초기화
    var countEl = document.getElementById('activeAndonCount');
    if (countEl) countEl.textContent = '0 active alerts';
}

// ─── SSE 실시간 모니터링 ─────────────────────────────────
/**
 * SSE(Server-Sent Events) 연결을 시작하여 실시간 안돈 데이터를 수신한다.
 * 현재 필터 파라미터를 URL에 포함시켜 서버에서 필터링된 데이터만 전송받는다.
 * 이미 추적 중이면 중복 연결을 방지한다.
 */
async function startAutoTracking() {
    if (isTracking) return; // 이미 연결 중이면 중복 실행 방지

    var filters = getFilterParams(); // 현재 적용된 필터 값 수집
    var params = new URLSearchParams(filters);
    // SSE 엔드포인트 URL 생성 (필터 파라미터 포함)
    var sseUrl = 'proc/data_andon_stream_2.php?' + params.toString();

    eventSource = new EventSource(sseUrl); // SSE 연결 생성
    setupSSEEventListeners();              // SSE 이벤트 핸들러 등록
    isTracking = true;
}

/**
 * 현재 선택된 필터 값들을 수집하여 객체로 반환한다.
 * SSE 연결 URL 파라미터 및 Export URL 생성에 사용된다.
 * @returns {Object} 필터 파라미터 객체
 */
function getFilterParams() {
    var dateRange = $('#dateRangePicker').val(); // jQuery DateRangePicker 값
    var startDate = '', endDate = '';
    // ' ~ ' 구분자로 시작/종료 날짜 분리
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
        limit: 100 // 최대 수신 레코드 수
    };
}

/**
 * SSE EventSource에 이벤트 핸들러를 등록한다.
 * 처리하는 이벤트 타입:
 *   - onerror: 연결 오류 시 재연결 시도
 *   - connected: 초기 연결 성공 확인
 *   - andon_data: 실제 안돈 데이터 수신 (메인 데이터)
 *   - heartbeat: 주기적 연결 유지 확인
 *   - disconnected: 서버 측 연결 종료 알림
 */
function setupSSEEventListeners() {
    // SSE 연결 오류 시 자동 재연결 시도
    eventSource.onerror = function () {
        isTracking = false;
        attemptReconnection(); // 지수 백오프 재연결
    };

    // 서버에서 'connected' 이벤트 수신 시: 재연결 카운터 초기화
    eventSource.addEventListener('connected', function () {
        reconnectAttempts = 0;
        var el = document.getElementById('connectionStatus');
        if (el) el.textContent = 'Andon system connected';
    });

    // 서버에서 'andon_data' 이벤트 수신 시: 메인 데이터 처리
    eventSource.addEventListener('andon_data', function (event) {
        var data = JSON.parse(event.data); // JSON 파싱
        stats        = data.stats;          // 통계 데이터 저장
        activeAndons = data.active_andons;  // 현재 활성 안돈 목록
        andonData    = data.andon_data;     // 전체 안돈 데이터 (테이블용)
        window.andonData = data.andon_data; // 전역 접근 가능하도록 노출

        // UI 각 영역 업데이트
        updateStatCards(stats);                       // 통계 카드 값 갱신
        updateActiveAndonsDisplay(activeAndons);       // 활성 안돈 카드 영역 갱신
        updateTableFromAPI(andonData);                 // 데이터 테이블 갱신
        updateChartsFromAPI(data);                     // 차트 데이터 갱신

        var el = document.getElementById('lastUpdateTime');
        if (el) el.textContent = 'Last updated: ' + data.timestamp;
    });

    // 서버에서 'heartbeat' 이벤트 수신 시: 연결 상태 표시 업데이트
    eventSource.addEventListener('heartbeat', function (event) {
        var data = JSON.parse(event.data);
        var el = document.getElementById('connectionStatus');
        if (el) el.textContent = 'Connection maintained (Active warnings: ' + data.active_warnings + ')';
    });

    // 서버에서 'disconnected' 이벤트 수신 시: 연결 종료 표시
    eventSource.addEventListener('disconnected', function () {
        var el = document.getElementById('connectionStatus');
        if (el) el.textContent = 'Disconnected';
    });
}

// ─── 통계 카드 업데이트 ───────────────────────────────────
/**
 * SSE로 수신된 통계 데이터를 각 통계 카드 DOM 요소에 반영한다.
 * @param {Object} statsData - 서버에서 수신한 stats 객체
 */
function updateStatCards(statsData) {
    if (!statsData) return;
    // 카드 ID와 데이터 키를 매핑
    var map = {
        'totalAndons':       statsData.total_count           || '0',
        'activeWarnings':    statsData.warning_count         || '0',
        'currentShiftCount': statsData.current_shift_count   || '0',
        'affectedMachines':  statsData.affected_machines     || '0',
        'urgentWarnings':    statsData.urgent_warnings_count || '0',
        'avgCompletedTime':  statsData.avg_completed_time    || '-'
    };
    // 각 ID의 요소에 값 설정
    Object.entries(map).forEach(function ([id, val]) {
        var el = document.getElementById(id);
        if (el) el.textContent = val;
    });
}

// ─── 활성 안돈 표시 ───────────────────────────────────────
/**
 * 현재 활성(Warning) 상태인 안돈 항목들을 카드 형태로 렌더링한다.
 * 각 카드는 안돈 색상으로 테두리를 표시하며 경과시간이 실시간으로 갱신된다.
 * @param {Array} activeAndonsList - 활성 안돈 객체 배열
 */
function updateActiveAndonsDisplay(activeAndonsList) {
    var container = document.getElementById('activeAndonsContainer');
    var countEl   = document.getElementById('activeAndonCount');

    // 활성 안돈 개수 표시 업데이트
    if (countEl) countEl.textContent = activeAndonsList.length + ' active alerts';
    if (!container) return;

    // 활성 안돈이 없으면 정상 상태 메시지 표시
    if (activeAndonsList.length === 0) {
        container.innerHTML = '<div class="fiori-alert fiori-alert--success"><strong>Good:</strong> No active Andons. All systems normal.</div>';
        stopElapsedTimeTimer(); // 불필요한 타이머 정지
        return;
    }

    // 각 활성 안돈을 카드로 렌더링
    var html = '';
    activeAndonsList.forEach(function (andon) {
        var initialElapsed = calculateElapsedTime(andon.reg_date); // 초기 경과시간 계산
        var borderColor    = andon.andon_color || '#0070f2';       // 안돈 유형 색상
        var bgColor        = borderColor + '1A';                   // 배경색 (10% 투명도)
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
    startElapsedTimeTimer(); // 1초 간격 경과시간 갱신 타이머 시작
}

// ─── 테이블 업데이트 ──────────────────────────────────────
/**
 * 안돈 데이터 배열을 받아 페이지네이션에 맞게 슬라이싱 후
 * 테이블 tbody를 DOM으로 재렌더링한다.
 * Warning 상태 행은 실시간 경과시간 셀(duration-in-progress)을 포함한다.
 * @param {Array} andonDataList - 전체 안돈 데이터 배열
 */
function updateTableFromAPI(andonDataList) {
    var tbody = document.getElementById('andonDataBody');
    if (!tbody) return;

    totalItems = andonDataList.length; // 전체 아이템 수 업데이트 (페이지네이션에 사용)

    // 데이터가 없을 때 안내 메시지 표시
    if (andonDataList.length === 0) {
        tbody.innerHTML = '<tr><td colspan="10" class="data-table-centered"><div class="fiori-alert fiori-alert--info"><strong>Information:</strong> No Andon data matching the selected conditions.</div></td></tr>';
        renderPagination();
        return;
    }

    // 현재 페이지에 해당하는 데이터 슬라이싱
    var startIndex = (currentPage - 1) * itemsPerPage;
    var paginatedData = andonDataList.slice(startIndex, startIndex + itemsPerPage);

    tbody.innerHTML = '';
    paginatedData.forEach(function (andon) {
        var row = document.createElement('tr');

        // 상태에 따른 배지(badge) 클래스 결정
        var statusBadge = andon.status === 'Warning'
            ? '<span class="fiori-badge fiori-badge--error">Warning</span>'
            : '<span class="fiori-badge fiori-badge--success">Completed</span>';

        // 근무조 표시 텍스트 생성
        var shiftDisplay = andon.shift_idx ? 'Shift ' + andon.shift_idx : '-';

        // Warning 상태이면 실시간 갱신 셀, Completed이면 고정 시간 표시
        var durationCell = '';
        if (andon.status === 'Warning' && andon.reg_date) {
            var initialDuration = calculateElapsedTime(andon.reg_date);
            // data-start-time 속성으로 시작 시각 저장 (타이머가 참조)
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
            // 상세 버튼: JSON 데이터를 data 속성에 직렬화하여 저장
            + '<td><button class="fiori-btn fiori-btn--tertiary andon-details-btn" style="padding:0.25rem 0.5rem;font-size:0.75rem;" data-andon-data=\'' + JSON.stringify(andon).replace(/'/g, "&#39;") + '\'>Detail</button></td>';

        tbody.appendChild(row);
    });

    renderPagination();           // 페이지네이션 버튼 렌더링
    setupDetailsButtonListeners(); // 상세 버튼에 이벤트 리스너 재등록
    startDurationUpdateTimer();    // 진행중 행의 경과시간 타이머 시작
}

/**
 * 테이블의 모든 'Detail' 버튼에 클릭 이벤트 리스너를 등록한다.
 * 기존 리스너를 제거 후 재등록하여 중복 실행을 방지한다.
 */
function setupDetailsButtonListeners() {
    document.querySelectorAll('.andon-details-btn').forEach(function (btn) {
        btn.removeEventListener('click', handleDetailsButtonClick);
        btn.addEventListener('click', handleDetailsButtonClick);
    });
}

/**
 * Detail 버튼 클릭 이벤트 핸들러.
 * 버블링을 막고 해당 버튼의 안돈 모달을 열기 위해
 * closest()로 정확한 버튼 요소를 찾는다.
 */
function handleDetailsButtonClick(event) {
    event.preventDefault();
    event.stopPropagation(); // 이벤트 버블링 차단
    var btn = event.target.closest('.andon-details-btn');
    if (btn) openAndonDetailModal(btn);
}

// ─── 페이지네이션 ─────────────────────────────────────────
/**
 * 현재 페이지 상태(currentPage, totalItems, itemsPerPage)를 기반으로
 * Fiori 스타일 페이지네이션 HTML을 생성하여 렌더링한다.
 * 1페이지 이하이면 페이지네이션을 숨긴다.
 */
function renderPagination() {
    var container = document.getElementById('pagination-controls');
    if (!container) return;

    var totalPages = Math.ceil(totalItems / itemsPerPage);
    if (totalPages <= 1) { container.innerHTML = ''; return; } // 1페이지 이하면 숨김

    // 현재 표시 범위 계산 (예: "1-10 / 54 items")
    var startItem = totalItems === 0 ? 0 : (currentPage - 1) * itemsPerPage + 1;
    var endItem   = Math.min(currentPage * itemsPerPage, totalItems);

    var html = '<div class="fiori-pagination__info">' + startItem + '-' + endItem + ' / ' + totalItems + ' items (' + itemsPerPage + ' per page)</div>'
             + '<div class="fiori-pagination__buttons">';

    // 이전 페이지 버튼 (첫 페이지이면 비활성화)
    html += '<button class="fiori-pagination__button' + (currentPage === 1 ? ' fiori-pagination__button--disabled' : '') + '" onclick="changePage(' + (currentPage - 1) + ')"' + (currentPage === 1 ? ' disabled' : '') + '>&larr;</button>';

    // 현재 페이지 기준 앞뒤 2페이지만 표시 (슬라이딩 윈도우)
    var startPage = Math.max(1, currentPage - 2);
    var endPage   = Math.min(totalPages, currentPage + 2);
    // 시작 페이지 앞에 1 페이지와 생략 부호 추가
    if (startPage > 1) { html += '<button class="fiori-pagination__button" onclick="changePage(1)">1</button>'; if (startPage > 2) html += '<span class="fiori-pagination__ellipsis">...</span>'; }
    for (var i = startPage; i <= endPage; i++) {
        html += '<button class="fiori-pagination__button' + (i === currentPage ? ' fiori-pagination__button--active' : '') + '" onclick="changePage(' + i + ')">' + i + '</button>';
    }
    // 마지막 페이지와 생략 부호 추가
    if (endPage < totalPages) { if (endPage < totalPages - 1) html += '<span class="fiori-pagination__ellipsis">...</span>'; html += '<button class="fiori-pagination__button" onclick="changePage(' + totalPages + ')">' + totalPages + '</button>'; }
    // 다음 페이지 버튼 (마지막 페이지이면 비활성화)
    html += '<button class="fiori-pagination__button' + (currentPage === totalPages ? ' fiori-pagination__button--disabled' : '') + '" onclick="changePage(' + (currentPage + 1) + ')"' + (currentPage === totalPages ? ' disabled' : '') + '>&rarr;</button>';
    html += '</div>';

    container.innerHTML = html;
}

/**
 * 페이지 번호 변경 처리 함수.
 * 유효 범위 밖이거나 현재 페이지와 같으면 무시한다.
 * @param {number} newPage - 이동할 페이지 번호
 */
function changePage(newPage) {
    var totalPages = Math.ceil(totalItems / itemsPerPage);
    if (newPage < 1 || newPage > totalPages || newPage === currentPage) return;
    currentPage = newPage;
    updateTableFromAPI(andonData); // 해당 페이지 데이터 재렌더링
}

// ─── 차트 업데이트 ────────────────────────────────────────
/**
 * SSE로 수신된 전체 API 데이터에서 차트별 통계를 추출하여 각 차트를 갱신한다.
 * @param {Object} apiData - SSE andon_data 이벤트의 전체 data 객체
 */
function updateChartsFromAPI(apiData) {
    if (apiData.andon_type_stats && charts.andonType)  updateAndonTypeChart(apiData.andon_type_stats);
    if (apiData.andon_trend_stats && charts.andonTrend) updateAndonTrendChart(apiData.andon_trend_stats);
}

/**
 * 안돈 유형별 발생 건수 바 차트를 갱신한다.
 * 안돈 이름 알파벳 순으로 정렬 후 각 안돈의 고유 색상을 적용한다.
 * @param {Array} andonTypeStats - 안돈 유형별 집계 배열
 */
function updateAndonTypeChart(andonTypeStats) {
    if (!charts.andonType || !andonTypeStats || andonTypeStats.length === 0) return;

    // 안돈 이름 알파벳 오름차순 정렬
    var sorted = andonTypeStats.slice().sort(function (a, b) {
        return (a.andon_name || '').toLowerCase().localeCompare((b.andon_name || '').toLowerCase());
    });
    var labels  = sorted.map(function (i) { return i.andon_name || 'Unclassified'; });
    var counts  = sorted.map(function (i) { return parseInt(i.count) || 0; });
    // 안돈 유형의 지정 색상이 없을 때 사용할 대체 색상 배열
    var fallback = ['#0070f2', '#da1e28', '#e26b0a', '#30914c', '#8e44ad', '#1e88e5'];
    var colors  = sorted.map(function (i, idx) { return i.andon_color || fallback[idx % fallback.length]; });

    charts.andonType.data.labels                        = labels;
    charts.andonType.data.datasets[0].data              = counts;
    charts.andonType.data.datasets[0].backgroundColor   = colors;
    charts.andonType.data.datasets[0].borderColor       = colors;
    charts.andonType.update('none'); // 애니메이션 없이 즉시 갱신
}

/**
 * 시간대별 안돈 추이 라인 차트를 갱신한다.
 * 작업 시간(workHours) 또는 날짜 범위에 따라 x축 레이블을 동적으로 생성한다.
 * @param {Object|Array} andonTrendStats - 추이 데이터 (work_hours, data, view_type 포함 가능)
 */
function updateAndonTrendChart(andonTrendStats) {
    if (!charts.andonTrend || !andonTrendStats) return;

    // 중첩 구조와 단순 배열 두 형식 모두 처리
    var trendData = andonTrendStats.data || andonTrendStats;
    var workHours = andonTrendStats.work_hours || null;
    var viewType  = andonTrendStats.view_type  || 'hourly'; // 'hourly' 또는 'daily'

    // 데이터가 없을 때 폴백 처리
    if (!trendData || trendData.length === 0) {
        if (window.andonData && window.andonData.length > 0) { generateSimpleTrendFromAndonData(); return; }
        charts.andonTrend.data.labels = ['No Data'];
        charts.andonTrend.data.datasets[0].data = [0];
        charts.andonTrend.data.datasets[1].data = [0];
        charts.andonTrend.update('none');
        return;
    }

    // x축 레이블 생성 (시간별: 근무시간 기준, 일별: 날짜 레이블 사용)
    var labels = [];
    if (viewType === 'hourly' && workHours && workHours.start_time && workHours.end_time) {
        labels = generateWorkHoursLabels(workHours.start_time, workHours.end_time, workHours.start_minutes, workHours.end_minutes);
    } else if (viewType === 'daily') {
        labels = trendData.map(function (i) { return i.display_label || i.time_label; });
    } else {
        // 'YYYY-MM-DD HH:...' 형식에서 시간 부분만 추출하여 'HHH' 형태로 변환
        labels = trendData.map(function (i) {
            var lbl = i.time_label || i.display_label || '';
            var m = lbl.match(/^(\d{4}-\d{2}-\d{2})\s+(\d{2}):/);
            return m ? m[2] + 'H' : lbl;
        });
    }

    // 레이블을 키로 Warning/Completed 카운트 매핑
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
    // Dataset[0]: 전체(Warning + Completed), Dataset[1]: Completed만
    charts.andonTrend.data.datasets[0].data = labels.map(function (l) { return (warningMap[l] || 0) + (completedMap[l] || 0); });
    charts.andonTrend.data.datasets[1].data = labels.map(function (l) { return completedMap[l] || 0; });
    // x축 제목 동적 변경
    charts.andonTrend.options.scales.x.title.text = viewType === 'hourly' ? 'Time' : 'Date';
    charts.andonTrend.update('show'); // 애니메이션 포함 갱신
}

/**
 * 근무 시작/종료 시간을 기반으로 시간별 x축 레이블 배열을 생성한다.
 * 야간 근무(자정 넘김)도 지원한다.
 * @param {string} startTime     - "HH:mm" 형식 근무 시작 시간
 * @param {string} endTime       - "HH:mm" 형식 근무 종료 시간
 * @param {number} startMinutes  - 자정 기준 분 단위 시작 시간 (선택)
 * @param {number} endMinutes    - 자정 기준 분 단위 종료 시간 (선택)
 * @returns {Array} "HHH" 형식 레이블 배열
 */
function generateWorkHoursLabels(startTime, endTime, startMinutes, endMinutes) {
    var labels = [], startHour, endHour;
    if (startMinutes !== null && endMinutes !== null) {
        // 분 단위 값으로 시간 계산
        startHour = Math.floor(startMinutes / 60);
        endHour   = Math.floor(endMinutes / 60);
    } else {
        // "HH:mm" 문자열에서 시간 파싱
        startHour = parseInt(startTime.split(':')[0]);
        endHour   = parseInt(endTime.split(':')[0]);
        // 야간 근무: 종료 시간이 시작 시간보다 작으면 +24
        if (endHour < startHour) endHour += 24;
    }
    for (var h = startHour; h <= endHour; h++) {
        labels.push((h % 24).toString().padStart(2, '0') + 'H'); // "09H", "22H" 형태
    }
    return labels;
}

/**
 * SSE 추이 데이터가 없을 때 andonData 배열에서 날짜별 집계를 직접 생성하여
 * 추이 차트를 업데이트하는 폴백 함수.
 */
function generateSimpleTrendFromAndonData() {
    if (!charts.andonTrend || !window.andonData || window.andonData.length === 0) return;
    // 작업 날짜(work_date) 기준으로 그룹화
    var dateGroups = {};
    window.andonData.forEach(function (item) {
        if (!item.work_date) return;
        if (!dateGroups[item.work_date]) dateGroups[item.work_date] = { total: 0, completed: 0 };
        dateGroups[item.work_date].total++;
        if (item.status === 'Completed') dateGroups[item.work_date].completed++;
    });
    var dates  = Object.keys(dateGroups).sort();
    // "M/D" 형식 레이블 생성
    var labels = dates.map(function (d) { var x = new Date(d); return (x.getMonth() + 1) + '/' + x.getDate(); });
    charts.andonTrend.data.labels            = labels;
    charts.andonTrend.data.datasets[0].data  = dates.map(function (d) { return dateGroups[d].total; });
    charts.andonTrend.data.datasets[1].data  = dates.map(function (d) { return dateGroups[d].completed; });
    charts.andonTrend.options.scales.x.title.text = 'Date';
    charts.andonTrend.update('show');
}

// ─── 실시간 타이머 ────────────────────────────────────────
/**
 * 주어진 시작 시각으로부터 현재까지의 경과 시간을 "Xh Ym Zs" 형식 문자열로 반환한다.
 * 서버 시간대는 Asia/Jakarta(WIB)로 가정한다.
 * @param {string} startTime - "YYYY-MM-DD HH:mm:ss" 형식 시작 시각
 * @returns {string} 경과 시간 문자열
 */
function calculateElapsedTime(startTime) {
    // WIB(Asia/Jakarta) 기준 현재 시각
    var now   = new Date(new Date().toLocaleString('en-US', { timeZone: 'Asia/Jakarta' }));
    var start = new Date(startTime.replace(' ', 'T')); // ISO 8601 형식으로 변환
    var diff  = Math.floor((now - start) / 1000); // 초 단위 차이
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

/**
 * 활성 안돈 카드의 경과시간을 일괄 갱신한다.
 * elapsedTimeTimer에 의해 1초 간격으로 호출된다.
 */
function updateElapsedTimes() {
    document.querySelectorAll('.active-andon-item').forEach(function (item, i) {
        if (activeAndons[i] && activeAndons[i].reg_date) {
            var span = item.querySelector('.andon-elapsed-time span');
            if (span) span.textContent = calculateElapsedTime(activeAndons[i].reg_date);
        }
    });
}

/**
 * 테이블에서 진행 중(Warning) 상태 행의 경과시간 셀을 일괄 갱신한다.
 * durationUpdateTimer에 의해 1초 간격으로 호출된다.
 */
function updateInProgressDurations() {
    document.querySelectorAll('.duration-in-progress').forEach(function (cell) {
        var t = cell.getAttribute('data-start-time');
        if (t) cell.textContent = calculateElapsedTime(t);
    });
}

/**
 * 활성 안돈 카드 경과시간 갱신 타이머를 시작한다.
 * 기존 타이머가 있으면 먼저 제거 후 재시작한다.
 */
function startElapsedTimeTimer() {
    if (elapsedTimeTimer) clearInterval(elapsedTimeTimer);
    elapsedTimeTimer = setInterval(updateElapsedTimes, 1000); // 1초 간격
}

/**
 * 활성 안돈 카드 경과시간 타이머를 정지한다.
 */
function stopElapsedTimeTimer() {
    if (elapsedTimeTimer) { clearInterval(elapsedTimeTimer); elapsedTimeTimer = null; }
}

/**
 * 테이블 내 진행중 행 경과시간 타이머를 시작한다.
 */
function startDurationUpdateTimer() {
    if (durationUpdateTimer) clearInterval(durationUpdateTimer);
    durationUpdateTimer = setInterval(updateInProgressDurations, 1000); // 1초 간격
}

/**
 * 테이블 내 진행중 행 경과시간 타이머를 정지한다.
 */
function stopDurationUpdateTimer() {
    if (durationUpdateTimer) { clearInterval(durationUpdateTimer); durationUpdateTimer = null; }
}

/**
 * 모든 실시간 타이머(경과시간, 진행중 기간)를 한 번에 정지한다.
 * 페이지 언로드 또는 탭 숨김 시 호출된다.
 */
function stopAllTimers() {
    stopElapsedTimeTimer();
    stopDurationUpdateTimer();
}

// ─── 필터 변경 / 재시작 ──────────────────────────────────
/**
 * SSE 추적을 즉시 중단하고 타이머도 정지한다.
 */
function stopTracking() {
    if (eventSource) { eventSource.close(); eventSource = null; }
    stopAllTimers();
    isTracking = false;
}

/**
 * SSE 모니터링을 재시작한다.
 * 필터가 변경되었을 때 새 파라미터로 재연결한다.
 * 페이지를 1로 리셋 후 기존 연결 종료 → 새 연결 시작.
 */
async function restartRealTimeMonitoring() {
    currentPage = 1; // 필터 변경 시 첫 페이지로 이동
    if (isTracking) { stopTracking(); await new Promise(function (r) { setTimeout(r, 100); }); }
    reconnectAttempts = 0;
    await startAutoTracking();
}

/**
 * SSE 연결 오류 시 지수 백오프(Exponential Backoff) 방식으로 재연결을 시도한다.
 * 최대 maxReconnectAttempts 회까지 시도하며, 이후에는 포기한다.
 * 딜레이: 1s → 2s → 4s (최대 10s 상한)
 */
async function attemptReconnection() {
    if (isPageUnloading || reconnectAttempts >= maxReconnectAttempts) return;
    reconnectAttempts++;
    // 지수 백오프: 2^(시도-1) * 1000ms, 최대 10000ms
    var delay = Math.min(1000 * Math.pow(2, reconnectAttempts - 1), 10000);
    if (eventSource) { eventSource.close(); eventSource = null; }
    setTimeout(async function () {
        if (isPageUnloading) return;
        try { await startAutoTracking(); } catch (e) { attemptReconnection(); }
    }, delay);
}

// ─── 날짜 / 시간 범위 핸들러 ─────────────────────────────
/**
 * DateRangePicker에서 날짜 범위가 변경되었을 때 호출된다.
 * timeRangeSelect를 'custom'으로 변경하고 SSE를 재시작한다.
 */
async function handleDateRangeChange(start, end) {
    var sel = document.getElementById('timeRangeSelect');
    if (sel) {
        // 'custom' 옵션이 없으면 동적으로 생성
        var opt = sel.querySelector('option[value="custom"]');
        if (!opt) { opt = document.createElement('option'); opt.value = 'custom'; opt.textContent = 'Custom'; sel.appendChild(opt); }
        sel.value = 'custom';
    }
    if (isTracking) { stopTracking(); setTimeout(async function () { await startAutoTracking(); }, 500); }
    else { await startAutoTracking(); }
}

/**
 * 시간 범위 셀렉트박스의 값이 변경되었을 때 호출된다.
 * 선택된 범위에 따라 DateRangePicker 값을 동기화하고 SSE를 재시작한다.
 */
async function handleTimeRangeChange(event) {
    var range = event.target.value;
    if (range !== 'custom') {
        var startDate, endDate;
        // 시간 범위 옵션별 moment.js 날짜 계산
        switch (range) {
            case 'yesterday': startDate = moment().subtract(1, 'days').startOf('day'); endDate = moment().subtract(1, 'days').endOf('day'); break;
            case '1w':        startDate = moment().subtract(7, 'days').startOf('day'); endDate = moment().endOf('day'); break;
            case '1m':        startDate = moment().subtract(30, 'days').startOf('day'); endDate = moment().endOf('day'); break;
            default:          startDate = moment().startOf('day'); endDate = moment().endOf('day'); // 기본: 오늘
        }
        // DateRangePicker 위젯의 선택 날짜를 코드로 강제 변경
        $('#dateRangePicker').data('daterangepicker').setStartDate(startDate);
        $('#dateRangePicker').data('daterangepicker').setEndDate(endDate);
        $('#dateRangePicker').val(startDate.format('YYYY-MM-DD') + ' ~ ' + endDate.format('YYYY-MM-DD'));
    }
    await restartRealTimeMonitoring();
}

// ─── Export / Refresh ────────────────────────────────────
/**
 * 현재 필터 조건으로 안돈 데이터를 Excel 형태로 내보낸다.
 * 서버의 export PHP를 새 탭에서 열어 파일 다운로드를 유도한다.
 */
function exportData() {
    var filters = getFilterParams(); // 현재 필터 파라미터 수집
    var params  = new URLSearchParams();
    // 값이 있는 필터만 URL 파라미터에 포함
    if (filters.factory_filter) params.append('factory_filter', filters.factory_filter);
    if (filters.line_filter)    params.append('line_filter',    filters.line_filter);
    if (filters.machine_filter) params.append('machine_filter', filters.machine_filter);
    if (filters.shift_filter)   params.append('shift_filter',   filters.shift_filter);
    if (filters.start_date)     params.append('start_date',     filters.start_date);
    if (filters.end_date)       params.append('end_date',       filters.end_date);
    // Export PHP를 현재 창에서 열어 파일 다운로드
    window.location.href = 'proc/data_andon_export_2.php?' + params.toString();
}

/**
 * 데이터를 수동으로 새로고침한다.
 * SSE를 재시작하여 최신 데이터를 다시 수신한다.
 */
async function refreshData() {
    if (isTracking) { stopTracking(); setTimeout(async function () { await startAutoTracking(); }, 500); }
    else { await startAutoTracking(); }
}

// ─── Andon Detail Modal ──────────────────────────────────
/**
 * 안돈 상세 정보 모달을 열고 데이터를 채운다.
 * @param {HTMLElement} buttonElement - 'data-andon-data' 속성을 가진 버튼 요소
 */
function openAndonDetailModal(buttonElement) {
    try {
        var json = buttonElement.getAttribute('data-andon-data');
        if (!json) return;
        var data = JSON.parse(json); // JSON 역직렬화
        populateAndonModal(data);   // 모달 필드에 데이터 채우기
        var modal = document.getElementById('andonDetailModal');
        if (modal) { modal.classList.add('show'); document.body.style.overflow = 'hidden'; } // 모달 표시 + 스크롤 잠금
    } catch (e) { console.error('Modal open error:', e); }
}

/**
 * 안돈 상세 모달을 닫는다.
 * 애니메이션('hide' 클래스) 후 모달을 숨기고 스크롤을 복원한다.
 */
function closeAndonDetailModal() {
    var modal = document.getElementById('andonDetailModal');
    if (modal) {
        modal.classList.add('hide'); // CSS 닫힘 애니메이션 트리거
        setTimeout(function () {
            modal.classList.remove('show', 'hide'); // 300ms 후 클래스 정리
            document.body.style.overflow = '';      // 스크롤 복원
        }, 300);
    }
}

/**
 * 안돈 데이터 객체의 각 필드를 모달 내부 DOM 요소에 채운다.
 * @param {Object} data - 안돈 데이터 객체
 */
function populateAndonModal(data) {
    // 짧은 헬퍼: ID로 요소를 찾아 textContent 설정
    function setVal(id, val) { var el = document.getElementById(id); if (el) el.textContent = val; }

    setVal('modal-machine-no',   data.machine_no || '-');
    setVal('modal-factory-line', (data.factory_name || '-') + ' / ' + (data.line_name || '-'));
    setVal('modal-andon-type',   data.andon_name || '-');

    // 상태 배지는 innerHTML로 렌더링
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

    // 안돈 색상 인디케이터 표시
    var indicator = document.getElementById('modal-color-indicator');
    var colorVal  = document.getElementById('modal-color-value');
    if (indicator && colorVal) {
        indicator.style.backgroundColor = data.andon_color || '#cccccc';
        colorVal.textContent = data.andon_color || 'Default Color';
    }

    setVal('modal-idx',        data.idx        || '-');
    setVal('modal-created-at', data.created_at || data.reg_date || '-');
}

/**
 * 단일 안돈 항목 내보내기 (현재는 모달 닫기만 수행, 추후 확장 가능).
 */
function exportSingleAndon() { closeAndonDetailModal(); }

// ─── 전역 함수 등록 ──────────────────────────────────────
// HTML의 onclick 속성 및 renderPagination 내 인라인 onclick에서 직접 호출하기 위해
// window 객체에 명시적으로 등록한다.
window.openAndonDetailModal  = openAndonDetailModal;
window.closeAndonDetailModal = closeAndonDetailModal;
window.exportSingleAndon     = exportSingleAndon;
window.changePage            = changePage;
