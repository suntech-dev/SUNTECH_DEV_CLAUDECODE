/**
 * ============================================================
 * log_oee_2.js
 * ============================================================
 * 파일 목적:
 *   OEE(Overall Equipment Effectiveness) 일별 로그 데이터를
 *   실시간으로 조회하고 테이블로 표시하는 페이지의 핵심 JavaScript.
 *   SSE(Server-Sent Events)를 통해 data_oee 테이블의 전체 컬럼 데이터를
 *   스트리밍 수신하며, 컬럼 표시/숨김 토글, 페이지네이션,
 *   공장·라인·기계 필터, 날짜 범위 필터, Excel 내보내기를 지원한다.
 *
 * 주요 기능:
 *   - SSE 실시간 연결 / 재연결 (지수 백오프)
 *   - 공장·라인·기계 필터 시스템 (동적 연계 셀렉트박스)
 *   - DateRangePicker 날짜 범위 필터
 *   - 컬럼 표시/숨김 토글 드롭다운
 *   - JS 기반 sticky 컬럼 스크롤 동기화 (machine_no 고정)
 *   - SAP Fiori 스타일 OEE 등급 배지 (85%↑ 성공, 70%↑ 경고, 미만 오류)
 *   - 페이지네이션 (페이지당 20행, 슬라이딩 윈도우)
 *   - Excel 내보내기 (proc/log_oee_export_2.php)
 *   - 통계 카드: Overall OEE / Availability / Performance / Quality
 * ============================================================
 */

/* ── SSE 및 추적 상태 전역 변수 ── */
let eventSource = null;           // SSE EventSource 인스턴스
let isTracking = false;           // 현재 SSE 추적 활성 여부
let oeeData = [];                 // 서버에서 수신한 전체 OEE 데이터 배열
let reconnectAttempts = 0;        // SSE 재연결 시도 횟수
let maxReconnectAttempts = 3;     // 최대 재연결 허용 횟수
let stats = {};                   // 통계 카드용 집계 데이터
let isPageUnloading = false;      // 페이지 언로드 중 여부 (재연결 방지)

/* ── 페이지네이션 상태 ── */
let currentPage = 1;    // 현재 페이지 번호
let itemsPerPage = 20;  // 페이지당 표시 행 수
let totalItems = 0;     // 전체 데이터 건수

// 컬럼 설정 (data_oee 테이블 전체 컬럼)
// key: DB 컬럼명, label: 헤더 표시명, visible: 초기 표시 여부
const columnConfig = [
    { key: 'idx',                  label: 'idx',                  visible: false }, // PK (기본 숨김)
    { key: 'work_date',            label: 'work_date',            visible: true  }, // 작업일
    { key: 'time_update',          label: 'time_update',          visible: true  }, // 갱신 시각
    { key: 'shift_idx',            label: 'shift_idx',            visible: true  }, // 근무조 ID
    { key: 'factory_idx',          label: 'factory_idx',          visible: false }, // 공장 ID (기본 숨김)
    { key: 'factory_name',         label: 'factory_name',         visible: true  }, // 공장명
    { key: 'line_idx',             label: 'line_idx',             visible: false }, // 라인 ID (기본 숨김)
    { key: 'line_name',            label: 'line_name',            visible: true  }, // 라인명
    { key: 'mac',                  label: 'mac',                  visible: true  }, // MAC 주소
    { key: 'machine_idx',          label: 'machine_idx',          visible: false }, // 기계 ID (기본 숨김)
    { key: 'machine_no',           label: 'machine_no',           visible: true  }, // 기계 번호 (sticky 컬럼)
    { key: 'process_name',         label: 'process_name',         visible: true  }, // 공정명
    { key: 'planned_work_time',    label: 'planned_work_time',    visible: true  }, // 계획 작업시간
    { key: 'runtime',              label: 'runtime',              visible: true  }, // 실제 가동시간
    { key: 'productive_runtime',   label: 'productive_runtime',   visible: true  }, // 생산적 가동시간
    { key: 'downtime',             label: 'downtime',             visible: true  }, // 비가동시간
    { key: 'availabilty_rate',     label: 'availabilty_rate',     visible: true  }, // 가용률 (%)
    { key: 'target_line_per_day',  label: 'target_line_per_day',  visible: true  }, // 라인 일 목표
    { key: 'target_line_per_hour', label: 'target_line_per_hour', visible: true  }, // 라인 시간당 목표
    { key: 'target_mc_per_day',    label: 'target_mc_per_day',    visible: true  }, // 기계 일 목표
    { key: 'target_mc_per_hour',   label: 'target_mc_per_hour',   visible: true  }, // 기계 시간당 목표
    { key: 'cycletime',            label: 'cycletime',            visible: true  }, // 사이클 타임
    { key: 'pair_info',            label: 'pair_info',            visible: true  }, // 페어 정보
    { key: 'pair_count',           label: 'pair_count',           visible: true  }, // 페어 수량
    { key: 'theoritical_output',   label: 'theoritical_output',   visible: true  }, // 이론 생산량
    { key: 'actual_output',        label: 'actual_output',        visible: true  }, // 실제 생산량
    { key: 'productivity_rate',    label: 'productivity_rate',    visible: true  }, // 성능률 (%)
    { key: 'defective',            label: 'defective',            visible: true  }, // 불량 수
    { key: 'actual_a_grade',       label: 'actual_a_grade',       visible: true  }, // 실제 양품 수
    { key: 'quality_rate',         label: 'quality_rate',         visible: true  }, // 품질률 (%)
    { key: 'oee',                  label: 'oee',                  visible: true  }, // OEE (%)
    { key: 'reg_date',             label: 'reg_date',             visible: true  }, // 등록 일시
    { key: 'update_date',          label: 'update_date',          visible: true  }  // 수정 일시
];

/**
 * DOMContentLoaded 이벤트 핸들러
 * 페이지 로드 완료 후 모든 초기화를 순차적으로 수행한다.
 */
document.addEventListener('DOMContentLoaded', async function () {
    initDateRangePicker();          // 날짜 범위 선택기 초기화
    await initFilterSystem();       // 공장/라인/기계 필터 셀렉트박스 로드
    initColumnToggle();             // 컬럼 표시/숨김 드롭다운 초기화
    setupEventListeners();          // 버튼·셀렉트박스 이벤트 리스너 등록
    renderTableHeader();            // 테이블 헤더 행 렌더링
    updateLayout(); // 초기 grid-template-rows 설정
    initStickyColumnsScroll(); // JS 기반 컬럼 고정 초기화
    await loadInitialData();        // 초기 빈 상태 표시
    await startAutoTracking();      // SSE 실시간 데이터 수신 시작
});

// ─── JS sticky 컬럼 스크롤 동기화 ──────────────────────────
/**
 * overflow:clip/hidden 부모 구조에서 CSS position:sticky가 작동하지 않으므로
 * 수평 스크롤 이벤트에 따라 sticky 컬럼에 translateX를 적용하여
 * 컬럼 고정 효과를 JS로 구현한다.
 */
function initStickyColumnsScroll() {
    const wrap = document.querySelector('.oee-table-wrap');
    if (!wrap) return;

    let _stickyEls = [];         // sticky 처리할 DOM 요소 배열
    let _naturalOffsets = [];    // 각 요소의 자연 scrollLeft=0 기준 offsetLeft

    /**
     * sticky 컬럼 요소들의 자연 위치(offsetLeft)를 다시 측정한다.
     * 스크롤을 일시적으로 0으로 리셋 후 측정하여 정확도를 보장한다.
     */
    function _captureStickyOffsets() {
        const prevScroll = wrap.scrollLeft;
        if (prevScroll !== 0) wrap.scrollLeft = 0; // 측정 전 스크롤 초기화

        _stickyEls = Array.from(wrap.querySelectorAll('.sticky-column'));
        _naturalOffsets = _stickyEls.map(el => el.offsetLeft);

        if (prevScroll !== 0) wrap.scrollLeft = prevScroll; // 스크롤 복원
        _updatePositions();
    }

    /**
     * 현재 scrollLeft 값을 기반으로 각 sticky 컬럼의 translateX를 계산하여 적용한다.
     * 스크롤 위치가 요소의 자연 위치를 초과하면 shift 값만큼 이동시킨다.
     */
    function _updatePositions() {
        const sl = wrap.scrollLeft;
        _stickyEls.forEach((el, i) => {
            const nat = _naturalOffsets[i] || 0;
            const shift = sl > nat ? sl - nat : 0; // 초과분 계산
            el.style.transform = shift > 0 ? `translateX(${shift}px)` : '';
        });
    }

    // scroll 이벤트: requestAnimationFrame으로 성능 최적화 (ticking 패턴)
    let _ticking = false;
    wrap.addEventListener('scroll', () => {
        if (!_ticking) {
            requestAnimationFrame(() => { _updatePositions(); _ticking = false; });
            _ticking = true;
        }
    }, { passive: true }); // passive: true로 스크롤 성능 향상

    // 외부(테이블 렌더 후)에서 offset 재측정을 트리거할 수 있도록 노출
    wrap._refreshStickyColumns = _captureStickyOffsets;
    // 300ms 후 초기 offset 측정 (DOM 렌더링 완료 대기)
    setTimeout(_captureStickyOffsets, 300);
}

// ─── 페이지 언로드 처리 ──────────────────────────────────
/**
 * 페이지 닫기/이동 시 SSE 연결을 안전하게 종료한다.
 * isPageUnloading 플래그로 불필요한 재연결을 방지한다.
 */
window.addEventListener('beforeunload', () => {
    isPageUnloading = true;
    if (eventSource) { eventSource.close(); eventSource = null; isTracking = false; }
});

// 모바일 브라우저의 bfcache(뒤로가기 캐시) 처리
window.addEventListener('pagehide', () => {
    isPageUnloading = true;
    if (eventSource) { eventSource.close(); eventSource = null; isTracking = false; }
});

/**
 * 탭 가시성 변경 이벤트
 * 탭이 숨겨지면 SSE를 종료하고, 다시 표시되면 재연결한다.
 */
document.addEventListener('visibilitychange', () => {
    if (document.hidden) {
        // 탭 비활성화: SSE 즉시 종료
        if (eventSource && isTracking) { eventSource.close(); eventSource = null; isTracking = false; }
    } else {
        // 탭 재활성화: SSE 재연결 시도
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
 * 통계·테이블·페이지네이션 섹션의 hidden 상태에 따라
 * CSS Grid의 grid-template-rows를 동적으로 재계산한다.
 */
function updateLayout() {
    const main = document.getElementById('logOeeMain');
    if (!main) return;

    const rows = [];
    // 통계 섹션이 표시 중이면 auto 추가
    if (!document.getElementById('logOeeStats').classList.contains('hidden'))  rows.push('auto');
    // 테이블 섹션이 표시 중이면 1fr (남은 공간 모두 차지)
    if (!document.getElementById('logOeeTable').classList.contains('hidden'))  rows.push('1fr');
    rows.push('auto'); // pagination 항상 표시

    main.style.gridTemplateRows = rows.join(' ');
}

// ─── 토글 함수 ────────────────────────────────────────────
/**
 * 통계 카드 섹션의 표시/숨김을 토글하고 레이아웃을 재계산한다.
 */
function toggleStatsDisplay() {
    const row = document.getElementById('logOeeStats');
    const btn = document.getElementById('toggleStatsBtn');
    if (!row || !btn) return;

    row.classList.toggle('hidden');
    btn.textContent = row.classList.contains('hidden') ? 'Show Stats' : 'Hide Stats';
    updateLayout();
}

/**
 * 데이터 테이블 및 페이지네이션 섹션의 표시/숨김을 토글한다.
 */
function toggleDataDisplay() {
    const rowTable      = document.getElementById('logOeeTable');
    const rowPagination = document.getElementById('logOeePagination');
    const btn           = document.getElementById('toggleDataBtn');
    if (!rowTable || !btn) return;

    rowTable.classList.toggle('hidden');
    if (rowPagination) rowPagination.classList.toggle('hidden');
    btn.textContent = rowTable.classList.contains('hidden') ? 'Show Table' : 'Hide Table';
    updateLayout();
}

// ─── DateRangePicker ──────────────────────────────────────
/**
 * jQuery DateRangePicker 플러그인을 초기화한다.
 * 오늘/어제/최근 1주/최근 1달 등 사전 범위를 제공하며,
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
                'today':      [moment(), moment()],                                                      // 오늘
                'yesterday':  [moment().subtract(1, 'days'), moment().subtract(1, 'days')],              // 어제
                'last week':  [moment().subtract(6, 'days'), moment()],                                  // 최근 7일
                'last month': [moment().subtract(29, 'days'), moment()]                                  // 최근 30일
            },
            startDate: moment().startOf('day'), // 기본 시작: 오늘 0시
            endDate:   moment().endOf('day'),   // 기본 종료: 오늘 23:59
            showDropdowns: true,             // 연/월 드롭다운 표시
            alwaysShowCalendars: true        // 사전 범위와 캘린더 동시 표시
        }, function (start, end) {
            // 날짜 범위 변경 콜백 → SSE 재시작
            handleDateRangeChange(start, end);
        });
        // 입력창에 오늘 날짜 초기값 설정
        $('#dateRangePicker').val(moment().format('YYYY-MM-DD') + ' ~ ' + moment().format('YYYY-MM-DD'));
    } catch (e) {
        console.error('DateRangePicker init error:', e);
    }
}

// ─── 필터 시스템 ──────────────────────────────────────────
/**
 * 공장·라인·기계 필터 셀렉트박스를 순서대로 초기화한다.
 */
async function initFilterSystem() {
    await loadFactoryOptions();  // 공장 목록 fetch 및 셀렉트박스 구성
    await loadLineOptions();     // 라인 목록 fetch 및 셀렉트박스 구성
    await loadMachineOptions();  // 기계 목록 fetch 및 셀렉트박스 구성
}

/**
 * 활성 공장 목록을 서버에서 fetch하여 공장 필터 셀렉트박스를 구성한다.
 * 공장 선택 변경 시 기계 목록을 초기화하고 라인 목록을 연계 갱신한다.
 */
async function loadFactoryOptions() {
    try {
        // status_filter=Y: 활성 공장만 조회
        const res = await fetch('../manage/proc/factory.php?status_filter=Y').then(r => r.json());
        if (!res.success || !res.data) return;
        const sel = document.getElementById('factoryFilterSelect');
        sel.innerHTML = '<option value="">All Factory</option>';
        const factories = res.data.filter(f => Number(f.idx) !== 99);
        // 각 공장을 option으로 추가
        factories.forEach(f => {
            sel.innerHTML += `<option value="${f.idx}">${f.factory_name}</option>`;
        });
        // 공장 선택 변경 이벤트 리스너
        sel.addEventListener('change', async (e) => {
            // 기계 셀렉트박스 초기화 및 비활성화
            document.getElementById('factoryLineMachineFilterSelect').innerHTML = '<option value="">All Machine</option>';
            document.getElementById('factoryLineMachineFilterSelect').disabled = true;
            await updateLineOptions(e.target.value); // 라인 목록 연계 갱신
            await restartRealTimeMonitoring();        // SSE 재시작
        });
        if (factories.length === 1) {
            sel.value = factories[0].idx;
            sel.dispatchEvent(new Event('change'));
        }
    } catch (e) { console.error('Factory options error:', e); }
}

/**
 * 활성 라인 목록을 서버에서 fetch하여 라인 필터 셀렉트박스를 구성한다.
 * 라인 선택 변경 시 기계 목록을 연계 갱신한다.
 */
async function loadLineOptions() {
    try {
        const res = await fetch('../manage/proc/line.php?status_filter=Y').then(r => r.json());
        if (!res.success || !res.data) return;
        const sel = document.getElementById('factoryLineFilterSelect');
        sel.innerHTML = '<option value="">All Line</option>';
        res.data.forEach(l => {
            sel.innerHTML += `<option value="${l.idx}">${l.line_name}</option>`;
        });
        // 라인 선택 변경 이벤트 리스너
        sel.addEventListener('change', async (e) => {
            // 현재 선택된 공장 ID와 라인 ID로 기계 목록 갱신
            await updateMachineOptions(document.getElementById('factoryFilterSelect').value, e.target.value);
            await restartRealTimeMonitoring();
        });
    } catch (e) { console.error('Line options error:', e); }
}

/**
 * 전체 기계 목록을 서버에서 fetch하여 기계 필터 셀렉트박스를 구성한다.
 * (초기 로드: 필터 없이 전체 기계 조회)
 */
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

/**
 * 공장 ID가 변경되었을 때 라인 필터 셀렉트박스를 동적으로 갱신한다.
 * @param {string} factoryId - 선택된 공장의 idx 값 (빈 문자열이면 전체)
 */
async function updateLineOptions(factoryId) {
    const sel = document.getElementById('factoryLineFilterSelect');
    sel.disabled = true; // 로드 중 비활성화
    try {
        // 공장 ID가 있으면 해당 공장의 라인만 조회
        const url = '../manage/proc/line.php?status_filter=Y' + (factoryId ? '&factory_filter=' + factoryId : '');
        const res = await fetch(url).then(r => r.json());
        sel.innerHTML = '<option value="">All Line</option>';
        if (res.success) res.data.forEach(l => { sel.innerHTML += `<option value="${l.idx}">${l.line_name}</option>`; });
        sel.disabled = false;
        if (factoryId) {
            // 공장이 선택된 경우 기계 셀렉트박스도 활성화
            document.getElementById('factoryLineMachineFilterSelect').disabled = false;
            updateMachineOptions(factoryId, '');
        }
    } catch (e) { sel.innerHTML = '<option value="">All Line</option>'; sel.disabled = false; }
}

/**
 * 공장/라인 ID가 변경되었을 때 기계 필터 셀렉트박스를 동적으로 갱신한다.
 * @param {string} factoryId - 선택된 공장의 idx 값
 * @param {string} lineId    - 선택된 라인의 idx 값
 */
async function updateMachineOptions(factoryId, lineId) {
    const sel = document.getElementById('factoryLineMachineFilterSelect');
    sel.disabled = true; // 로드 중 비활성화
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
/**
 * 컬럼 표시/숨김 드롭다운을 초기화한다.
 * columnConfig 배열의 각 컬럼에 체크박스를 생성하고,
 * 체크박스 상태 변경 시 테이블 헤더와 데이터 행을 즉시 재렌더링한다.
 */
function initColumnToggle() {
    const btn      = document.getElementById('columnToggleBtn');
    const dropdown = document.getElementById('columnToggleDropdown');
    if (!btn || !dropdown) return;

    dropdown.innerHTML = '';

    // 드롭다운 헤더 (제목 + 닫기 버튼)
    dropdown.insertAdjacentHTML('beforeend', `
        <div style="display:flex;justify-content:space-between;align-items:center;margin-bottom:6px;padding-bottom:4px;border-bottom:1px solid var(--sap-border-subtle);">
            <strong style="color:var(--sap-text-primary);font-size:var(--sap-font-size-sm);">Show/Hide Columns</strong>
            <button type="button" class="col-toggle-close" style="background:none;border:none;cursor:pointer;color:var(--sap-text-secondary);font-size:16px;padding:2px;" title="Close">&#10005;</button>
        </div>
    `);

    // columnConfig 배열을 순회하여 컬럼별 체크박스 생성
    columnConfig.forEach(col => {
        const label    = document.createElement('label');
        const checkbox = document.createElement('input');
        checkbox.type              = 'checkbox';
        checkbox.checked           = col.visible;        // 초기 표시 상태 반영
        checkbox.dataset.columnKey = col.key;
        label.appendChild(checkbox);
        label.appendChild(document.createTextNode(' ' + col.label));
        dropdown.appendChild(label);

        // 체크박스 변경 이벤트: columnConfig 업데이트 → 테이블 즉시 재렌더
        checkbox.addEventListener('change', (e) => {
            col.visible = e.target.checked;
            renderTableHeader();         // 헤더 행 재렌더링
            updateTableFromAPI(oeeData); // 데이터 행 재렌더링
        });
    });

    // 컬럼 토글 버튼 클릭: 드롭다운 열기/닫기
    btn.addEventListener('click', (e) => {
        e.stopPropagation(); // 이벤트 버블링 차단 (외부 클릭 감지와 충돌 방지)
        dropdown.classList.toggle('show');
    });

    // 드롭다운 내부 클릭: 이벤트 버블링 차단 + 닫기 버튼 처리
    dropdown.addEventListener('click', (e) => {
        if (e.target.classList.contains('col-toggle-close')) {
            dropdown.classList.remove('show'); // 닫기 버튼 클릭 시 드롭다운 닫기
            return;
        }
        e.stopPropagation();
    });

    // 드롭다운 외부 클릭 시 자동으로 닫기
    document.addEventListener('click', (e) => {
        if (!btn.contains(e.target) && !dropdown.contains(e.target)) {
            dropdown.classList.remove('show');
        }
    });
}

// ─── 테이블 헤더 렌더링 ───────────────────────────────────
/**
 * columnConfig를 기반으로 테이블 헤더(<th>) 행을 동적으로 생성한다.
 * visible=false인 컬럼은 hidden-column 클래스로 숨기고,
 * machine_no 컬럼에는 sticky-column 클래스를 추가한다.
 */
function renderTableHeader() {
    const headerRow = document.getElementById('tableHeaderRow');
    if (!headerRow) return;
    headerRow.innerHTML = ''; // 기존 헤더 초기화

    columnConfig.forEach(col => {
        const th = document.createElement('th');
        th.textContent         = col.label;
        th.dataset.columnKey   = col.key;
        if (!col.visible) th.classList.add('hidden-column');       // 숨김 처리
        if (col.key === 'machine_no') th.classList.add('sticky-column'); // 고정 컬럼
        headerRow.appendChild(th);
    });
}

// ─── 이벤트 리스너 ────────────────────────────────────────
/**
 * 페이지 내 모든 인터랙티브 요소에 이벤트 리스너를 등록한다.
 * bind: 클릭 이벤트, bindChange: change 이벤트 등록 헬퍼 함수 사용
 */
function setupEventListeners() {
    // 헬퍼: click 이벤트 등록
    const bind = (id, fn) => { const el = document.getElementById(id); if (el) el.addEventListener('click', fn); };
    // 헬퍼: change 이벤트 등록
    const bindChange = (id, fn) => { const el = document.getElementById(id); if (el) el.addEventListener('change', fn); };

    bind('excelDownloadBtn', exportData);       // Excel 내보내기 버튼
    bind('refreshBtn',       refreshData);       // 수동 새로고침 버튼
    bind('toggleStatsBtn',   toggleStatsDisplay); // 통계 섹션 토글 버튼
    bind('toggleDataBtn',    toggleDataDisplay);  // 테이블 섹션 토글 버튼

    bindChange('timeRangeSelect',               handleTimeRangeChange);           // 시간 범위 셀렉트
    bindChange('factoryLineMachineFilterSelect', () => restartRealTimeMonitoring()); // 기계 필터
    bindChange('shiftSelect',                   () => restartRealTimeMonitoring()); // 근무조 필터
}

// ─── 초기 데이터 ──────────────────────────────────────────
/**
 * 페이지 최초 진입 시 빈 상태를 표시한다.
 * 실제 데이터는 SSE 연결 후 채워진다.
 */
async function loadInitialData() {
    displayEmptyState();
}

/**
 * 통계 카드를 '-'로 초기화하고 테이블에 로딩 메시지를 표시한다.
 */
function displayEmptyState() {
    // 통계 카드 ID 목록을 순회하며 빈 상태('-')로 초기화
    ['overallOee', 'availability', 'performance', 'quality', 'currentShiftOee', 'previousDayOee'].forEach(id => {
        const el = document.getElementById(id);
        if (el) el.textContent = '-';
    });
    // 테이블 본문에 로딩 안내 메시지 표시
    const tbody        = document.getElementById('oeeDataBody');
    const visibleCount = columnConfig.filter(c => c.visible).length; // 현재 표시 중인 컬럼 수
    if (tbody) tbody.innerHTML = `
        <tr>
            <td colspan="${visibleCount}" class="data-table-centered">
                <div class="fiori-alert fiori-alert--info">
                    <strong>Information:</strong> Loading OEE data log. Please wait...
                </div>
            </td>
        </tr>`;
}

// ─── SSE / 데이터 로딩 ────────────────────────────────────
/**
 * 현재 적용된 필터 값들을 수집하여 객체로 반환한다.
 * SSE 연결 URL 파라미터 및 Export URL 생성에 사용된다.
 * @returns {Object} 필터 파라미터 객체
 */
function getFilterParams() {
    const dateRange = $('#dateRangePicker').val(); // jQuery DateRangePicker 값
    let startDate = '', endDate = '';
    // ' ~ ' 구분자로 시작/종료 날짜 분리
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
        limit: 1000 // 최대 수신 레코드 수
    };
}

/**
 * SSE(Server-Sent Events) 연결을 시작하여 실시간 OEE 로그 데이터를 수신한다.
 * 이미 추적 중이면 중복 연결을 방지한다.
 */
async function startAutoTracking() {
    if (isTracking) return; // 중복 연결 방지
    const params = new URLSearchParams(getFilterParams());
    // SSE 엔드포인트 연결 (필터 파라미터 포함)
    eventSource   = new EventSource('proc/log_oee_stream_2.php?' + params.toString());
    setupSSEEventListeners(); // SSE 이벤트 핸들러 등록
    isTracking = true;
    const el = document.getElementById('connectionStatus');
    if (el) el.textContent = 'OEE Data Log Real-time Connected';
}

/**
 * SSE EventSource에 이벤트 핸들러를 등록한다.
 * 처리 이벤트:
 *   - onerror: 연결 오류 시 재연결
 *   - connected: 초기 연결 성공
 *   - oee_data: OEE 로그 데이터 수신 (메인)
 *   - heartbeat: 주기적 연결 유지 확인
 *   - disconnected: 서버 측 연결 종료
 */
function setupSSEEventListeners() {
    // SSE 연결 오류 시 재연결 시도
    eventSource.onerror = function () {
        isTracking = false;
        attemptReconnection(); // 지수 백오프 재연결
    };

    // 'connected' 이벤트: 초기 연결 성공, 재연결 카운터 초기화
    eventSource.addEventListener('connected', function (e) {
        reconnectAttempts = 0;
        const el = document.getElementById('connectionStatus');
        if (el) el.textContent = 'OEE data log system connected';
    });

    // 'oee_data' 이벤트: 메인 데이터 수신 및 UI 업데이트
    eventSource.addEventListener('oee_data', function (e) {
        const data = JSON.parse(e.data); // JSON 파싱
        stats   = data.stats;            // 통계 데이터 저장
        oeeData = data.oee_data;         // OEE 로그 데이터 저장
        window.oeeData = data.oee_data;  // 전역 접근 가능하도록 노출

        updateStatCardsFromAPI(stats);   // 통계 카드 값 갱신
        updateTableFromAPI(oeeData);     // 데이터 테이블 갱신

        const el = document.getElementById('lastUpdateTime');
        if (el) el.textContent = 'Last updated: ' + data.timestamp;
    });

    // 'heartbeat' 이벤트: 주기적 연결 유지 확인
    eventSource.addEventListener('heartbeat', function (e) {
        const data = JSON.parse(e.data);
        const el   = document.getElementById('connectionStatus');
        if (el) el.textContent = 'Connection maintained (Active records: ' + data.active_machines + ')';
    });

    // 'disconnected' 이벤트: 서버 측 연결 종료 알림
    eventSource.addEventListener('disconnected', function () {
        const el = document.getElementById('connectionStatus');
        if (el) el.textContent = 'OEE Data Log System Connection Closed';
    });
}

/**
 * SSE 추적을 즉시 중단하고 연결 상태를 표시한다.
 */
function stopTracking() {
    if (!isTracking) return;
    if (eventSource) { eventSource.close(); eventSource = null; }
    isTracking = false;
    const el = document.getElementById('connectionStatus');
    if (el) el.textContent = 'OEE Data Log System Connection Closed';
}

/**
 * SSE 모니터링을 재시작한다.
 * 필터 변경 시 새 파라미터로 재연결한다.
 * 페이지를 1로 리셋 후 기존 연결 종료 → 새 연결 시작.
 */
async function restartRealTimeMonitoring() {
    currentPage = 1; // 필터 변경 시 첫 페이지로 이동
    if (isTracking) { stopTracking(); await new Promise(r => setTimeout(r, 100)); }
    reconnectAttempts = 0;
    await startAutoTracking();
}

/**
 * SSE 연결 오류 시 지수 백오프(Exponential Backoff)로 재연결을 시도한다.
 * 최대 maxReconnectAttempts 회까지 시도하며 이후 포기한다.
 * 딜레이: 1s → 2s → 4s (최대 10s 상한)
 */
async function attemptReconnection() {
    if (isPageUnloading || reconnectAttempts >= maxReconnectAttempts) return;
    reconnectAttempts++;
    // 지수 백오프 딜레이 계산 (최대 10초)
    const delay = Math.min(1000 * Math.pow(2, reconnectAttempts - 1), 10000);
    if (eventSource) { eventSource.close(); eventSource = null; }
    setTimeout(async () => {
        if (isPageUnloading) return;
        try { await startAutoTracking(); } catch (e) { attemptReconnection(); }
    }, delay);
}

// ─── 데이터 업데이트 ──────────────────────────────────────
/**
 * 숫자 값을 퍼센트 문자열로 포맷한다.
 * 정수면 소수점 없이, 소수면 소수점 유지.
 * @param {number|string} value - 퍼센트 값
 * @returns {string} "XX%" 형식 문자열
 */
function formatPercentage(value) {
    const n = parseFloat(value);
    if (isNaN(n)) return '0%';
    return (n % 1 === 0 ? Math.floor(n) : n) + '%';
}

/**
 * SSE로 수신된 통계 데이터를 각 통계 카드 DOM 요소에 반영한다.
 * @param {Object} s - 서버에서 수신한 stats 객체
 */
function updateStatCardsFromAPI(s) {
    if (!s) return;
    // 카드 ID와 표시할 값 매핑
    const map = {
        overallOee:      formatPercentage(s.overall_oee    || 0), // 전체 OEE
        availability:    formatPercentage(s.availability   || 0), // 가용률
        performance:     formatPercentage(s.performance    || 0), // 성능률
        quality:         formatPercentage(s.quality        || 0), // 품질률
        currentShiftOee: formatPercentage(s.current_shift_oee  || 0), // 현재 근무조 OEE
        previousDayOee:  formatPercentage(s.previous_day_oee   || 0)  // 전일 OEE
    };
    Object.keys(map).forEach(id => {
        const el = document.getElementById(id);
        if (el) el.textContent = map[id];
    });
}

/**
 * OEE 데이터 배열을 받아 현재 페이지에 해당하는 데이터만
 * 테이블 tbody에 렌더링한다.
 * OEE/가용률/성능률/품질률 컬럼은 SAP Fiori 배지로 표시한다.
 * @param {Array} list - 전체 OEE 데이터 배열
 */
function updateTableFromAPI(list) {
    const tbody = document.getElementById('oeeDataBody');
    if (!tbody) return;
    totalItems = list.length; // 페이지네이션을 위한 전체 건수 업데이트

    // 데이터가 없을 때 안내 메시지 표시
    if (list.length === 0) {
        const visibleCount = columnConfig.filter(c => c.visible).length;
        tbody.innerHTML = `
            <tr>
                <td colspan="${visibleCount}" class="data-table-centered">
                    <div class="fiori-alert fiori-alert--info">
                        <strong>Information:</strong> No OEE data matching the selected conditions.
                    </div>
                </td>
            </tr>`;
        renderPagination();
        return;
    }

    // 현재 페이지에 해당하는 데이터 슬라이싱
    const start  = (currentPage - 1) * itemsPerPage;
    const paged  = list.slice(start, start + itemsPerPage);
    tbody.innerHTML = '';

    // 각 OEE 레코드를 행으로 렌더링
    paged.forEach(oee => {
        const row = document.createElement('tr');
        columnConfig.forEach(col => {
            const td  = document.createElement('td');
            td.dataset.columnKey = col.key;
            if (!col.visible) td.classList.add('hidden-column');         // 숨김 컬럼 처리
            if (col.key === 'machine_no') td.classList.add('sticky-column'); // sticky 컬럼

            let value = (oee[col.key] !== undefined && oee[col.key] !== null) ? oee[col.key] : '-';

            // 비율 컬럼 배지 렌더링 (OEE/가용률/성능률/품질률)
            if (['oee', 'availabilty_rate', 'productivity_rate', 'quality_rate'].includes(col.key) && value !== '-') {
                let badgeClass = 'fiori-badge';
                const num = parseFloat(value);
                if (col.key === 'oee') {
                    // OEE 등급별 배지 색상: 85%↑ 성공(초록), 70%↑ 경고(주황), 미만 오류(빨강)
                    if (num >= 85)      badgeClass += ' fiori-badge--success';
                    else if (num >= 70) badgeClass += ' fiori-badge--warning';
                    else                badgeClass += ' fiori-badge--error';
                } else if (col.key === 'availabilty_rate') {
                    badgeClass += ' fiori-badge--success'; // 가용률: 항상 성공 색상
                } else if (col.key === 'productivity_rate') {
                    badgeClass += ' fiori-badge--info';    // 성능률: 정보 색상
                } else if (col.key === 'quality_rate') {
                    badgeClass += ' fiori-badge--warning'; // 품질률: 경고 색상
                }
                td.innerHTML = `<span class="${badgeClass}">${value}%</span>`;
            } else {
                td.textContent = value; // 일반 컬럼: 텍스트로 표시
            }

            row.appendChild(td);
        });
        tbody.appendChild(row);
    });

    renderPagination(); // 페이지네이션 버튼 렌더링
    // 테이블 렌더 후 sticky 컬럼 offset 갱신
    document.querySelector('.oee-table-wrap')?._refreshStickyColumns?.();
}

// ─── 페이지네이션 ─────────────────────────────────────────
/**
 * 현재 페이지 상태를 기반으로 Fiori 스타일 페이지네이션 HTML을 생성한다.
 * 슬라이딩 윈도우(현재 페이지 ±2) 방식으로 페이지 버튼을 표시한다.
 */
function renderPagination() {
    const container  = document.getElementById('pagination-controls');
    if (!container) return;
    const totalPages = Math.ceil(totalItems / itemsPerPage);
    if (totalPages <= 1) { container.innerHTML = ''; return; } // 1페이지 이하 숨김

    // 현재 표시 범위 계산 (예: "1-20 / 100 items")
    const s = totalItems === 0 ? 0 : (currentPage - 1) * itemsPerPage + 1;
    const e = Math.min(currentPage * itemsPerPage, totalItems);
    let html = `<div class="fiori-pagination__info">${s}-${e} / ${totalItems} items</div>`;
    html += '<div class="fiori-pagination__buttons">';
    // 이전 페이지 버튼 (첫 페이지 비활성화)
    html += `<button class="fiori-pagination__button${currentPage === 1 ? ' fiori-pagination__button--disabled' : ''}" onclick="changePage(${currentPage - 1})" ${currentPage === 1 ? 'disabled' : ''}>&larr;</button>`;

    // 슬라이딩 윈도우: 현재 페이지 기준 ±2
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
    // 다음 페이지 버튼 (마지막 페이지 비활성화)
    html += `<button class="fiori-pagination__button${currentPage === totalPages ? ' fiori-pagination__button--disabled' : ''}" onclick="changePage(${currentPage + 1})" ${currentPage === totalPages ? 'disabled' : ''}>&rarr;</button>`;
    html += '</div>';
    container.innerHTML = html;
}

/**
 * 페이지 번호 변경 처리 함수.
 * 유효 범위 밖이거나 현재 페이지와 같으면 무시한다.
 * @param {number} p - 이동할 페이지 번호
 */
function changePage(p) {
    const total = Math.ceil(totalItems / itemsPerPage);
    if (p < 1 || p > total || p === currentPage) return;
    currentPage = p;
    updateTableFromAPI(oeeData); // 해당 페이지 데이터 재렌더링
}

// ─── 날짜/시간 처리 ───────────────────────────────────────
/**
 * DateRangePicker에서 날짜 범위가 변경되었을 때 호출된다.
 * timeRangeSelect를 'custom'으로 변경하고 SSE를 재시작한다.
 * @param {moment} start - 선택된 시작 날짜 moment 객체
 * @param {moment} end   - 선택된 종료 날짜 moment 객체
 */
function handleDateRangeChange(start, end) {
    const timeRangeSelect = document.getElementById('timeRangeSelect');
    if (timeRangeSelect) {
        // 'custom' 옵션이 없으면 동적으로 생성
        let customOption = timeRangeSelect.querySelector('option[value="custom"]');
        if (!customOption) {
            customOption             = document.createElement('option');
            customOption.value       = 'custom';
            customOption.textContent = 'Custom Selection';
            timeRangeSelect.appendChild(customOption);
        }
        timeRangeSelect.value = 'custom';
    }
    // SSE 재시작
    if (isTracking) {
        stopTracking();
        setTimeout(() => startAutoTracking(), 1000); // 1초 후 재연결
    } else {
        startAutoTracking();
    }
}

/**
 * 시간 범위 셀렉트박스 값이 변경되었을 때 호출된다.
 * 선택된 범위에 따라 DateRangePicker 값을 동기화하고 SSE를 재시작한다.
 * @param {Event} event - change 이벤트 객체
 */
function handleTimeRangeChange(event) {
    const timeRange = event.target.value;
    if (timeRange !== 'custom') {
        let startDate, endDate;
        // 시간 범위 옵션별 moment.js 날짜 계산
        switch (timeRange) {
            case 'today':     // 오늘
                startDate = moment().startOf('day');
                endDate   = moment().endOf('day');
                break;
            case 'yesterday': // 어제
                startDate = moment().subtract(1, 'days').startOf('day');
                endDate   = moment().subtract(1, 'days').endOf('day');
                break;
            case '1w':        // 최근 7일
                startDate = moment().subtract(7, 'days').startOf('day');
                endDate   = moment().endOf('day');
                break;
            case '1m':        // 최근 30일
                startDate = moment().subtract(30, 'days').startOf('day');
                endDate   = moment().endOf('day');
                break;
            default:          // 기본: 오늘
                startDate = moment().startOf('day');
                endDate   = moment().endOf('day');
        }
        // DateRangePicker 위젯 값 강제 동기화
        $('#dateRangePicker').data('daterangepicker').setStartDate(startDate);
        $('#dateRangePicker').data('daterangepicker').setEndDate(endDate);
        $('#dateRangePicker').val(startDate.format('YYYY-MM-DD') + ' ~ ' + endDate.format('YYYY-MM-DD'));
    }
    restartRealTimeMonitoring(); // 필터 변경으로 SSE 재시작
}

// ─── Export / Refresh ─────────────────────────────────────
/**
 * 현재 필터 조건으로 OEE 로그 데이터를 Excel 형태로 내보낸다.
 * 서버의 export PHP를 새 탭에서 열어 파일 다운로드를 유도한다.
 */
function exportData() {
    if (!oeeData || oeeData.length === 0) return; // 데이터 없으면 중단
    try {
        const filters = getFilterParams(); // 현재 필터 파라미터 수집
        const params  = new URLSearchParams();
        // 값이 있는 필터만 URL 파라미터에 추가
        if (filters.factory_filter) params.append('factory_filter', filters.factory_filter);
        if (filters.line_filter)    params.append('line_filter',    filters.line_filter);
        if (filters.machine_filter) params.append('machine_filter', filters.machine_filter);
        if (filters.shift_filter)   params.append('shift_filter',   filters.shift_filter);
        if (filters.start_date)     params.append('start_date',     filters.start_date);
        if (filters.end_date)       params.append('end_date',       filters.end_date);
        // Export PHP를 새 탭에서 열기
        window.open('proc/log_oee_export_2.php?' + params.toString(), '_blank');
    } catch (e) {
        console.error('Export error:', e);
    }
}

/**
 * 데이터를 수동으로 새로고침한다.
 * SSE를 재시작하여 최신 데이터를 다시 수신한다.
 */
async function refreshData() {
    if (isTracking) {
        stopTracking();
        setTimeout(() => startAutoTracking(), 1000); // 1초 후 재연결
    } else {
        await startAutoTracking();
    }
}
