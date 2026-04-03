/*
 * ============================================================
 * log_oee_row_2.js
 * ============================================================
 * 목적: OEE Row 데이터 로그 조회 페이지 스크립트
 *       data_oee_rows 테이블의 전체 컬럼을 사용자가 선택하여
 *       실시간으로 조회·정렬·페이지네이션할 수 있도록 지원
 *
 * 주요 기능:
 *   - SSE(EventSource)를 통해 서버에서 OEE Row 데이터를 실시간 수신
 *   - 컬럼 표시/숨김 토글 드롭다운 (columnConfig 기반)
 *   - CSS sticky가 동작하지 않는 환경에서 JS transform으로
 *     machine_no 컬럼을 가로 스크롤 시에도 고정
 *   - 날짜 범위 필터(daterangepicker) 및 공장·라인·기계 필터 지원
 *   - 페이지네이션(페이지 당 20건) 및 OEE/비율 컬럼 배지 색상 표시
 *   - 페이지 숨김/언로드 시 SSE 연결 자동 종료 및 지수 백오프 재연결 로직
 *
 * 연관 백엔드: proc/log_oee_row_stream_2.php (SSE),
 *              proc/log_oee_row_export_2.php (Excel 내보내기)
 * ============================================================
 */

// log_oee_row_2.js — Row Grid Layout version (data_oee_2 model)

/* ── 전역 상태 변수 ─────────────────────────────────────── */
let eventSource = null;          // SSE EventSource 인스턴스
let isTracking = false;          // 현재 SSE 수신 중 여부
let oeeData = [];                // SSE로 수신된 OEE Row 데이터 전체 배열
let reconnectAttempts = 0;       // SSE 재연결 시도 횟수
let maxReconnectAttempts = 3;    // 최대 재연결 시도 횟수
let stats = {};                  // 통계 요약 데이터 (API 응답)
let isPageUnloading = false;     // 페이지 언로드 진행 중 여부 플래그

/* ── 페이지네이션 상태 ──────────────────────────────────── */
let currentPage = 1;             // 현재 페이지 번호
let itemsPerPage = 20;           // 페이지당 표시 건수 (OEE 로그는 20건)
let totalItems = 0;              // 전체 데이터 건수

// 컬럼 설정 (data_oee_rows 테이블 전체 컬럼)
// visible: true → 기본 표시, false → 기본 숨김 (사용자가 드롭다운으로 토글 가능)
const columnConfig = [
    { key: 'idx',                  label: 'idx',                  visible: true  },
    { key: 'work_date',            label: 'work_date',            visible: true  },
    { key: 'time_update',          label: 'time_update',          visible: true  },
    { key: 'shift_idx',            label: 'shift_idx',            visible: true  },
    { key: 'factory_idx',          label: 'factory_idx',          visible: false }, // 기본 숨김
    { key: 'factory_name',         label: 'factory_name',         visible: true  },
    { key: 'line_idx',             label: 'line_idx',             visible: false }, // 기본 숨김
    { key: 'line_name',            label: 'line_name',            visible: true  },
    { key: 'mac',                  label: 'mac',                  visible: true  },
    { key: 'machine_idx',          label: 'machine_idx',          visible: false }, // 기본 숨김
    { key: 'machine_no',           label: 'machine_no',           visible: true  }, // sticky 컬럼
    { key: 'process_name',         label: 'process_name',         visible: true  },
    { key: 'planned_work_time',    label: 'planned_work_time',    visible: true  },
    { key: 'runtime',              label: 'runtime',              visible: true  },
    { key: 'productive_runtime',   label: 'productive_runtime',   visible: true  },
    { key: 'downtime',             label: 'downtime',             visible: true  },
    { key: 'availabilty_rate',     label: 'availabilty_rate',     visible: true  }, // 비율 배지
    { key: 'target_line_per_day',  label: 'target_line_per_day',  visible: true  },
    { key: 'target_line_per_hour', label: 'target_line_per_hour', visible: true  },
    { key: 'target_mc_per_day',    label: 'target_mc_per_day',    visible: true  },
    { key: 'target_mc_per_hour',   label: 'target_mc_per_hour',   visible: true  },
    { key: 'cycletime',            label: 'cycletime',            visible: true  },
    { key: 'pair_info',            label: 'pair_info',            visible: true  },
    { key: 'pair_count',           label: 'pair_count',           visible: true  },
    { key: 'theoritical_output',   label: 'theoritical_output',   visible: true  },
    { key: 'actual_output',        label: 'actual_output',        visible: true  },
    { key: 'productivity_rate',    label: 'productivity_rate',    visible: true  }, // 비율 배지
    { key: 'defective',            label: 'defective',            visible: true  },
    { key: 'actual_a_grade',       label: 'actual_a_grade',       visible: true  },
    { key: 'quality_rate',         label: 'quality_rate',         visible: true  }, // 비율 배지
    { key: 'oee',                  label: 'oee',                  visible: true  }, // OEE 배지 (등급 색상)
    { key: 'reg_date',             label: 'reg_date',             visible: true  },
    { key: 'work_hour',            label: 'work_hour',            visible: true  }
];

/* ── DOMContentLoaded: 페이지 초기화 진입점 ────────────── */
// DOM이 완전히 로드된 후 순서대로 초기화 함수를 호출
document.addEventListener('DOMContentLoaded', async function () {
    initDateRangePicker();          // 날짜 범위 선택기 초기화
    await initFilterSystem();       // 공장/라인/기계 필터 옵션 로드
    initColumnToggle();             // 컬럼 표시/숨김 드롭다운 초기화
    setupEventListeners();          // 버튼·셀렉트박스 이벤트 등록
    renderTableHeader();            // columnConfig 기반 테이블 헤더 렌더링
    updateLayout(); // 초기 grid-template-rows 설정
    initStickyColumnsScroll(); // JS 기반 컬럼 고정 초기화
    await loadInitialData();        // 초기 빈 상태 표시
    await startAutoTracking();      // SSE 실시간 추적 시작
});

// ─── JS sticky 컬럼 스크롤 동기화 ──────────────────────────
// overflow:clip/hidden 부모 구조에서 CSS sticky가 작동하지 않으므로
// scroll 이벤트로 transform을 적용해 컬럼 고정 효과를 구현
function initStickyColumnsScroll() {
    const wrap = document.querySelector('.oee-table-wrap');
    if (!wrap) return;

    // sticky 컬럼 요소 목록과 자연 offset 위치 캐시
    let _stickyEls = [];
    let _naturalOffsets = [];

    // scroll=0 상태에서 각 sticky 컬럼의 offsetLeft를 측정하여 캐시
    function _captureStickyOffsets() {
        const prevScroll = wrap.scrollLeft;
        if (prevScroll !== 0) wrap.scrollLeft = 0;  // 측정 전 스크롤 초기화

        _stickyEls = Array.from(wrap.querySelectorAll('.sticky-column'));
        _naturalOffsets = _stickyEls.map(el => el.offsetLeft);

        if (prevScroll !== 0) wrap.scrollLeft = prevScroll;  // 측정 후 복원
        _updatePositions();
    }

    // 현재 scrollLeft에 따라 각 sticky 컬럼에 translateX 적용
    function _updatePositions() {
        const sl = wrap.scrollLeft;
        _stickyEls.forEach((el, i) => {
            const nat = _naturalOffsets[i] || 0;
            // 스크롤이 컬럼 자연 위치를 넘은 경우에만 이동
            const shift = sl > nat ? sl - nat : 0;
            el.style.transform = shift > 0 ? `translateX(${shift}px)` : '';
        });
    }

    // rAF(requestAnimationFrame) 기반 스크롤 이벤트: 불필요한 렌더 방지
    let _ticking = false;
    // scroll 이벤트: passive 옵션으로 스크롤 성능 최적화
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
    const main = document.getElementById('logRowMain');
    if (!main) return;

    const rows = [];
    // 각 Row가 숨김 상태가 아닐 때만 grid 행 크기 추가
    if (!document.getElementById('logRowStats').classList.contains('hidden'))  rows.push('auto');
    if (!document.getElementById('logRowTable').classList.contains('hidden'))  rows.push('1fr');
    rows.push('auto'); // pagination 항상

    main.style.gridTemplateRows = rows.join(' ');
}

// ─── 토글 함수 ────────────────────────────────────────────
// 통계(Stats) Row 표시/숨김 토글
function toggleStatsDisplay() {
    const row = document.getElementById('logRowStats');
    const btn = document.getElementById('toggleStatsBtn');
    if (!row || !btn) return;

    row.classList.toggle('hidden');
    btn.textContent = row.classList.contains('hidden') ? 'Show Stats' : 'Hide Stats';
    updateLayout();
}

// 데이터 테이블 Row 표시/숨김 토글 (페이지네이션 포함)
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
        const res = await fetch('../manage/proc/factory.php?status_filter=Y').then(r => r.json());
        if (!res.success || !res.data) return;
        const sel = document.getElementById('factoryFilterSelect');
        sel.innerHTML = '<option value="">All Factory</option>';
        res.data.forEach(f => {
            sel.innerHTML += `<option value="${f.idx}">${f.factory_name}</option>`;
        });
        // 공장 변경 시 기계 셀렉트 초기화 후 라인 목록 재로드
        sel.addEventListener('change', async (e) => {
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
        const res = await fetch('../manage/proc/line.php?status_filter=Y').then(r => r.json());
        if (!res.success || !res.data) return;
        const sel = document.getElementById('factoryLineFilterSelect');
        sel.innerHTML = '<option value="">All Line</option>';
        res.data.forEach(l => {
            sel.innerHTML += `<option value="${l.idx}">${l.line_name}</option>`;
        });
        // 라인 변경 시 기계 목록 재로드 및 실시간 모니터링 재시작
        sel.addEventListener('change', async (e) => {
            await updateMachineOptions(document.getElementById('factoryFilterSelect').value, e.target.value);
            await restartRealTimeMonitoring();
        });
    } catch (e) { console.error('Line options error:', e); }
}

// 기계 목록을 API로 가져와 셀렉트박스에 채움 (초기 전체 목록)
async function loadMachineOptions() {
    try {
        // fetch: 전체 기계 목록 요청
        const res = await fetch('../manage/proc/machine.php').then(r => r.json());
        if (!res.success || !res.data) return;
        const sel = document.getElementById('factoryLineMachineFilterSelect');
        sel.innerHTML = '<option value="">All Machine</option>';
        res.data.forEach(m => {
            sel.innerHTML += `<option value="${m.idx}">${m.machine_no} (${m.machine_model_name || 'No Model'})</option>`;
        });
    } catch (e) { console.error('Machine options error:', e); }
}

// 공장 선택 변경 시 해당 공장의 라인만 필터링하여 셀렉트박스 갱신
async function updateLineOptions(factoryId) {
    const sel = document.getElementById('factoryLineFilterSelect');
    sel.disabled = true;  // 로딩 중 비활성화
    try {
        // factoryId가 있으면 공장 필터 파라미터 추가
        const url = '../manage/proc/line.php?status_filter=Y' + (factoryId ? '&factory_filter=' + factoryId : '');
        const res = await fetch(url).then(r => r.json());
        sel.innerHTML = '<option value="">All Line</option>';
        if (res.success) res.data.forEach(l => { sel.innerHTML += `<option value="${l.idx}">${l.line_name}</option>`; });
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
    const sel = document.getElementById('factoryLineMachineFilterSelect');
    sel.disabled = true;  // 로딩 중 비활성화
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
// 컬럼 표시/숨김 드롭다운 UI를 초기화하고 체크박스·토글 이벤트를 등록
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

    // columnConfig를 순회하며 각 컬럼의 체크박스 생성
    columnConfig.forEach(col => {
        const label    = document.createElement('label');
        const checkbox = document.createElement('input');
        checkbox.type              = 'checkbox';
        checkbox.checked           = col.visible;       // 초기 표시 여부
        checkbox.dataset.columnKey = col.key;
        label.appendChild(checkbox);
        label.appendChild(document.createTextNode(' ' + col.label));
        dropdown.appendChild(label);

        // 체크박스 변경 시 columnConfig 업데이트 후 헤더·데이터 재렌더링
        checkbox.addEventListener('change', (e) => {
            col.visible = e.target.checked;
            renderTableHeader();
            updateTableFromAPI(oeeData);  // 현재 데이터로 테이블 재렌더링
        });
    });

    // 컬럼 토글 버튼 클릭 시 드롭다운 show/hide 토글
    btn.addEventListener('click', (e) => {
        e.stopPropagation();  // 이벤트 버블링 방지 (외부 클릭 닫기와 충돌 방지)
        dropdown.classList.toggle('show');
    });

    // 드롭다운 내부 클릭 전파 차단 + 닫기 버튼 처리
    dropdown.addEventListener('click', (e) => {
        if (e.target.classList.contains('col-toggle-close')) {
            dropdown.classList.remove('show');  // 닫기 버튼: 드롭다운 닫기
            return;
        }
        e.stopPropagation();  // 내부 클릭이 document로 전파되어 닫히지 않도록 방지
    });

    // 드롭다운 외부 클릭 시 자동 닫기
    document.addEventListener('click', (e) => {
        if (!btn.contains(e.target) && !dropdown.contains(e.target)) {
            dropdown.classList.remove('show');
        }
    });
}

// ─── 테이블 헤더 렌더링 ───────────────────────────────────
// columnConfig의 visible 상태에 따라 <thead> 행을 동적으로 렌더링
function renderTableHeader() {
    const headerRow = document.getElementById('tableHeaderRow');
    if (!headerRow) return;
    headerRow.innerHTML = '';

    // 각 컬럼에 대해 <th> 생성: 숨김 컬럼은 hidden-column 클래스 추가
    columnConfig.forEach(col => {
        const th = document.createElement('th');
        th.textContent         = col.label;
        th.dataset.columnKey   = col.key;
        if (!col.visible) th.classList.add('hidden-column');
        // machine_no 컬럼: JS sticky 스크롤 대상
        if (col.key === 'machine_no') th.classList.add('sticky-column');
        headerRow.appendChild(th);
    });
}

// ─── 이벤트 리스너 ────────────────────────────────────────
// 버튼 및 셀렉트박스에 클릭/변경 이벤트 리스너를 일괄 등록
function setupEventListeners() {
    // 헬퍼: click 이벤트 등록
    const bind = (id, fn) => { const el = document.getElementById(id); if (el) el.addEventListener('click', fn); };
    // 헬퍼: change 이벤트 등록
    const bindChange = (id, fn) => { const el = document.getElementById(id); if (el) el.addEventListener('change', fn); };

    bind('excelDownloadBtn', exportData);       // Excel 내보내기
    bind('refreshBtn',       refreshData);      // 새로고침
    bind('toggleStatsBtn',   toggleStatsDisplay); // 통계 Row 토글
    bind('toggleDataBtn',    toggleDataDisplay);  // 테이블 Row 토글

    // 시간 범위 셀렉트박스 변경 시 날짜 범위 자동 갱신
    bindChange('timeRangeSelect',               handleTimeRangeChange);
    // 기계 필터 변경 시 실시간 모니터링 재시작
    bindChange('factoryLineMachineFilterSelect', () => restartRealTimeMonitoring());
    // 교대 필터 변경 시 실시간 모니터링 재시작
    bindChange('shiftSelect',                   () => restartRealTimeMonitoring());
}

// ─── 초기 데이터 ──────────────────────────────────────────
// 페이지 초기 로드 시 빈 상태(로딩 중) UI를 표시
async function loadInitialData() {
    displayEmptyState();
}

// 통계 카드와 테이블에 로딩 중 기본값('-') 표시
function displayEmptyState() {
    // OEE 주요 지표 카드 요소들을 '-'로 초기화
    ['overallOee', 'availability', 'performance', 'quality', 'currentShiftOee', 'previousDayOee'].forEach(id => {
        const el = document.getElementById(id);
        if (el) el.textContent = '-';
    });
    // 현재 표시 중인 컬럼 수로 colspan 계산
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
// 현재 필터 상태에서 SSE URL 파라미터 객체를 반환
function getFilterParams() {
    const dateRange = $('#dateRangePicker').val();
    let startDate = '', endDate = '';
    // 날짜 범위 문자열을 시작일/종료일로 분리
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
        limit: 1000  // OEE 로그는 한 번에 최대 1000건 수신 (데이터 다량)
    };
}

// SSE EventSource를 생성하고 실시간 OEE Row 데이터 수신을 시작
async function startAutoTracking() {
    if (isTracking) return;  // 이미 추적 중이면 중복 실행 방지
    const params = new URLSearchParams(getFilterParams());
    // EventSource: 서버에서 OEE Row 데이터를 SSE로 스트리밍
    eventSource   = new EventSource('proc/log_oee_row_stream_2.php?' + params.toString());
    setupSSEEventListeners();
    isTracking = true;
    const el = document.getElementById('connectionStatus');
    if (el) el.textContent = 'OEE Row Data Real-time Connected';
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
        const el = document.getElementById('connectionStatus');
        if (el) el.textContent = 'OEE row data system connected';
    });

    // 'oee_data' 이벤트: 실제 OEE Row 데이터 수신 및 UI 갱신
    eventSource.addEventListener('oee_data', function (e) {
        const data = JSON.parse(e.data);
        stats   = data.stats;         // 통계 요약
        oeeData = data.oee_data;      // 전체 OEE Row 데이터
        window.oeeData = data.oee_data;  // 전역 참조 (컬럼 토글 재렌더링용)
        // UI 갱신: 통계 카드와 테이블
        updateStatCardsFromAPI(stats);
        updateTableFromAPI(oeeData);
        // 마지막 업데이트 시각 표시
        const el = document.getElementById('lastUpdateTime');
        if (el) el.textContent = 'Last updated: ' + data.timestamp;
    });

    // 'heartbeat' 이벤트: 연결 유지 확인 (활성 레코드 수 표시)
    eventSource.addEventListener('heartbeat', function (e) {
        const data = JSON.parse(e.data);
        const el   = document.getElementById('connectionStatus');
        if (el) el.textContent = 'Connection maintained (Active records: ' + data.active_machines + ')';
    });

    // 'disconnected' 이벤트: 서버에서 연결 종료 통보
    eventSource.addEventListener('disconnected', function () {
        const el = document.getElementById('connectionStatus');
        if (el) el.textContent = 'OEE Row Data System Connection Closed';
    });
}

// SSE 연결을 수동으로 종료하고 상태를 초기화
function stopTracking() {
    if (!isTracking) return;
    if (eventSource) { eventSource.close(); eventSource = null; }
    isTracking = false;
    const el = document.getElementById('connectionStatus');
    if (el) el.textContent = 'OEE Row Data System Connection Closed';
}

// 필터 변경 등으로 SSE를 재시작: 현재 연결 종료 후 재연결
async function restartRealTimeMonitoring() {
    currentPage = 1;  // 페이지를 첫 페이지로 초기화
    if (isTracking) { stopTracking(); await new Promise(r => setTimeout(r, 100)); }
    reconnectAttempts = 0;
    await startAutoTracking();
}

// SSE 오류 발생 시 지수 백오프 방식으로 재연결 시도
async function attemptReconnection() {
    if (isPageUnloading || reconnectAttempts >= maxReconnectAttempts) return;
    reconnectAttempts++;
    // 재연결 대기 시간: 1s → 2s → 4s (최대 10s)
    const delay = Math.min(1000 * Math.pow(2, reconnectAttempts - 1), 10000);
    if (eventSource) { eventSource.close(); eventSource = null; }
    setTimeout(async () => {
        if (isPageUnloading) return;
        try { await startAutoTracking(); } catch (e) { attemptReconnection(); }
    }, delay);
}

// ─── 데이터 업데이트 ──────────────────────────────────────
// 숫자값을 % 형식으로 포맷 (소수이면 소수점 그대로, 정수이면 정수로)
function formatPercentage(value) {
    const n = parseFloat(value);
    if (isNaN(n)) return '0%';
    return (n % 1 === 0 ? Math.floor(n) : n) + '%';
}

// SSE로 수신된 통계 데이터로 상단 OEE 통계 카드를 갱신
function updateStatCardsFromAPI(s) {
    if (!s) return;
    // DOM 요소 ID와 포맷된 표시 값의 매핑
    const map = {
        overallOee:      formatPercentage(s.overall_oee    || 0),        // 종합 OEE
        availability:    formatPercentage(s.availability   || 0),        // 가용성
        performance:     formatPercentage(s.performance    || 0),         // 성능
        quality:         formatPercentage(s.quality        || 0),         // 품질
        currentShiftOee: formatPercentage(s.current_shift_oee  || 0),   // 현재 교대 OEE
        previousDayOee:  formatPercentage(s.previous_day_oee   || 0)    // 전일 OEE
    };
    // 각 DOM 요소의 텍스트를 매핑된 값으로 일괄 업데이트
    Object.keys(map).forEach(id => {
        const el = document.getElementById(id);
        if (el) el.textContent = map[id];
    });
}

// columnConfig 기반으로 페이지네이션을 적용하여 테이블을 렌더링
function updateTableFromAPI(list) {
    const tbody = document.getElementById('oeeDataBody');
    if (!tbody) return;
    totalItems = list.length;  // 전체 건수 갱신 (페이지네이션 계산용)

    // 데이터가 없으면 안내 메시지 표시
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

    // 현재 페이지에 해당하는 데이터 슬라이스
    const start  = (currentPage - 1) * itemsPerPage;
    const paged  = list.slice(start, start + itemsPerPage);
    tbody.innerHTML = '';

    // 각 행을 columnConfig 순서대로 렌더링
    paged.forEach(oee => {
        const row = document.createElement('tr');
        columnConfig.forEach(col => {
            const td  = document.createElement('td');
            td.dataset.columnKey = col.key;
            // 숨김 컬럼 처리
            if (!col.visible) td.classList.add('hidden-column');
            // machine_no 컬럼: JS sticky 스크롤 대상
            if (col.key === 'machine_no') td.classList.add('sticky-column');

            // null/undefined 값은 '-'로 표시
            let value = (oee[col.key] !== undefined && oee[col.key] !== null) ? oee[col.key] : '-';

            // 비율 컬럼 배지 렌더링
            // oee: 등급별 색상 (≥85% 초록, ≥70% 주황, 미만 빨강)
            // availabilty_rate: success(초록), productivity_rate: info(파랑), quality_rate: warning(주황)
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
// 페이지네이션 컨트롤 렌더링 (이전/다음/페이지 번호 버튼)
function renderPagination() {
    const container  = document.getElementById('pagination-controls');
    if (!container) return;
    const totalPages = Math.ceil(totalItems / itemsPerPage);
    if (totalPages <= 1) { container.innerHTML = ''; return; }  // 1페이지면 미표시

    // 현재 페이지의 시작/끝 인덱스 계산
    const s = totalItems === 0 ? 0 : (currentPage - 1) * itemsPerPage + 1;
    const e = Math.min(currentPage * itemsPerPage, totalItems);
    let html = `<div class="fiori-pagination__info">${s}-${e} / ${totalItems} items</div>`;
    html += '<div class="fiori-pagination__buttons">';
    html += `<button class="fiori-pagination__button${currentPage === 1 ? ' fiori-pagination__button--disabled' : ''}" onclick="changePage(${currentPage - 1})" ${currentPage === 1 ? 'disabled' : ''}>&larr;</button>`;

    // 앞뒤로 2페이지씩 표시, 생략 부호(...) 자동 추가
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

// 페이지 번호 변경 시 테이블 데이터를 해당 페이지로 재렌더링
function changePage(p) {
    const total = Math.ceil(totalItems / itemsPerPage);
    if (p < 1 || p > total || p === currentPage) return;
    currentPage = p;
    updateTableFromAPI(oeeData);
}

// ─── 날짜/시간 처리 ───────────────────────────────────────
// daterangepicker 날짜 변경 시: 커스텀 옵션 선택 후 SSE 재시작
function handleDateRangeChange(start, end) {
    const timeRangeSelect = document.getElementById('timeRangeSelect');
    if (timeRangeSelect) {
        // 'custom' 옵션이 없으면 동적으로 생성 후 선택
        let customOption = timeRangeSelect.querySelector('option[value="custom"]');
        if (!customOption) {
            customOption          = document.createElement('option');
            customOption.value    = 'custom';
            customOption.textContent = 'Custom Selection';
            timeRangeSelect.appendChild(customOption);
        }
        timeRangeSelect.value = 'custom';
    }
    // SSE 재시작: 1초 지연 후 재연결 (안정적 종료 보장)
    if (isTracking) {
        stopTracking();
        setTimeout(() => startAutoTracking(), 1000);
    } else {
        startAutoTracking();
    }
}

// 시간 범위 셀렉트박스 변경 시: 해당 범위로 날짜 피커를 업데이트하고 SSE 재시작
function handleTimeRangeChange(event) {
    const timeRange = event.target.value;
    if (timeRange !== 'custom') {
        let startDate, endDate;
        // 선택된 범위에 따라 시작/종료 moment 계산
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
        // daterangepicker 값도 동기화
        $('#dateRangePicker').data('daterangepicker').setStartDate(startDate);
        $('#dateRangePicker').data('daterangepicker').setEndDate(endDate);
        $('#dateRangePicker').val(startDate.format('YYYY-MM-DD') + ' ~ ' + endDate.format('YYYY-MM-DD'));
    }
    restartRealTimeMonitoring();
}

// ─── Export / Refresh ─────────────────────────────────────
// 현재 필터 조건으로 Excel 내보내기 (새 창으로 다운로드)
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

// 새로고침: SSE 재연결로 데이터 즉시 갱신
async function refreshData() {
    if (isTracking) {
        stopTracking();
        setTimeout(() => startAutoTracking(), 1000);
    } else {
        await startAutoTracking();
    }
}