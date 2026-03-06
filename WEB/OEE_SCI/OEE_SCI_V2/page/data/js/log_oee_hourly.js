// OEE Hourly Data Log JavaScript v1.0

let eventSource = null;
let isTracking = false;
let oeeData = [];
let reconnectAttempts = 0;
let maxReconnectAttempts = 3;
let stats = {};
let isPageUnloading = false; // 페이지 언로드 상태 추적

let currentPage = 1;
let itemsPerPage = 20;
let totalItems = 0;

// 컬럼 설정 (data_oee_rows_hourly 테이블의 모든 컬럼)
const columnConfig = [
  { key: 'idx', label: 'idx', visible: true },
  { key: 'work_date', label: 'work_date', visible: true },
  { key: 'time_update', label: 'time_update', visible: true },
  { key: 'shift_idx', label: 'shift_idx', visible: true },
  { key: 'factory_idx', label: 'factory_idx', visible: false },
  { key: 'factory_name', label: 'factory_name', visible: true },
  { key: 'line_idx', label: 'line_idx', visible: false },
  { key: 'line_name', label: 'line_name', visible: true },
  { key: 'mac', label: 'mac', visible: true },
  { key: 'machine_idx', label: 'machine_idx', visible: false },
  { key: 'machine_no', label: 'machine_no', visible: true },
  // { key: 'design_no', label: 'design_no', visible: true },
  { key: 'process_name', label: 'process_name', visible: true },
  { key: 'planned_work_time', label: 'planned_work_time', visible: true },
  { key: 'runtime', label: 'runtime', visible: true },
  { key: 'productive_runtime', label: 'productive_runtime', visible: true },
  { key: 'downtime', label: 'downtime', visible: true },
  { key: 'availabilty_rate', label: 'availabilty_rate', visible: true },
  { key: 'target_line_per_day', label: 'target_line_per_day', visible: true },
  { key: 'target_line_per_hour', label: 'target_line_per_hour', visible: true },
  { key: 'target_mc_per_day', label: 'target_mc_per_day', visible: true },
  { key: 'target_mc_per_hour', label: 'target_mc_per_hour', visible: true },
  { key: 'cycletime', label: 'cycletime', visible: true },
  { key: 'pair_info', label: 'pair_info', visible: true },
  { key: 'pair_count', label: 'pair_count', visible: true },
  { key: 'theoritical_output', label: 'theoritical_output', visible: true },
  { key: 'actual_output', label: 'actual_output', visible: true },
  { key: 'productivity_rate', label: 'productivity_rate', visible: true },
  { key: 'defective', label: 'defective', visible: true },
  { key: 'actual_a_grade', label: 'actual_a_grade', visible: true },
  { key: 'quality_rate', label: 'quality_rate', visible: true },
  { key: 'oee', label: 'oee', visible: true },
  { key: 'reg_date', label: 'reg_date', visible: true },
  { key: 'update_date', label: 'update_date', visible: true },
  { key: 'work_hour', label: 'work_hour', visible: true }
];

document.addEventListener('DOMContentLoaded', async function () {
  console.log('OEE Hourly Data Log initialization started');

  try {
    initDateRangePicker();
    await initFilterSystem();
    initColumnToggle();
    setupEventListeners();
    renderTableHeader();
    await loadInitialData();
    await startAutoTracking();
    console.log('OEE Hourly Data Log initialized');
  } catch (error) {
    console.error('Initialization error:', error);
  }
});

// ============================================================================
// 페이지 라이프사이클 이벤트 처리 (우선순위 순서)
// SSE 연결 관리 및 브라우저 동시 연결 수 제한 문제 해결
// ============================================================================

// 1. beforeunload - 최우선순위: 페이지 언로드 시작 시 즉시 정리
window.addEventListener('beforeunload', () => {
  console.log('Log OEE Hourly: beforeunload - 페이지 언로드 시작');
  isPageUnloading = true; // 언로드 상태 설정 (재연결 방지)

  if (eventSource) {
    eventSource.close();
    eventSource = null;
    isTracking = false;
  }
});

// 2. pagehide - 2순위: 페이지 숨김 시 정리 (모바일 브라우저 지원)
window.addEventListener('pagehide', () => {
  console.log('Log OEE Hourly: pagehide - 페이지 숨김');
  isPageUnloading = true; // 언로드 상태 설정

  if (eventSource) {
    eventSource.close();
    eventSource = null;
    isTracking = false;
  }
});

// 3. visibilitychange - 3순위: 탭 전환 시 처리
document.addEventListener('visibilitychange', () => {
  if (document.hidden) {
    console.log('Log OEE Hourly: visibilitychange - 페이지 숨김, SSE 연결 종료');

    // 탭 전환 시 즉시 연결 종료
    if (eventSource) {
      eventSource.close();
      eventSource = null;
      isTracking = false;
    }
  } else {
    console.log('Log OEE Hourly: visibilitychange - 페이지 표시');

    // 페이지가 언로드 중이 아닐 때만 재연결
    if (!isPageUnloading && !isTracking && eventSource === null) {
      console.log('Log OEE Hourly: 페이지 재표시 - SSE 재연결 시도');
      reconnectAttempts = 0;
      startAutoTracking();
    } else if (isPageUnloading) {
      console.log('⚠️ Log OEE Hourly: 페이지 언로드 중 - 재연결 취소');
    }
  }
});

// 4. focus - 페이지 포커스 시 언로드 플래그 리셋
window.addEventListener('focus', () => {
  if (isPageUnloading) {
    console.log('Log OEE Hourly: 페이지 포커스 - 언로드 플래그 리셋');
    isPageUnloading = false;
  }
});

function initDateRangePicker() {
  try {
    $('#dateRangePicker').daterangepicker({
      opens: 'left',
      locale: {
        format: 'YYYY-MM-DD',
        separator: ' ~ ',
        applyLabel: 'Apply',
        cancelLabel: 'Cancel',
        fromLabel: 'From',
        toLabel: 'To',
        customRangeLabel: 'Custom Range',
        daysOfWeek: ['Sun', 'Mon', 'Tue', 'Wed', 'Thu', 'Fri', 'Sat'],
        monthNames: ['Jan', 'Feb', 'Mar', 'Apr', 'May', 'Jun',
          'Jul', 'Aug', 'Sep', 'Oct', 'Nov', 'Dec'],
        firstDay: 0
      },
      ranges: {
        'today': [moment(), moment()],
        'yesterday': [moment().subtract(1, 'days'), moment().subtract(1, 'days')],
        'last week': [moment().subtract(6, 'days'), moment()],
        'last month': [moment().subtract(29, 'days'), moment()],
        'this month': [moment().startOf('month'), moment().endOf('month')],
        'last month': [moment().subtract(1, 'month').startOf('month'), moment().subtract(1, 'month').endOf('month')]
      },
      startDate: moment().startOf('day'),
      endDate: moment().endOf('day'),
      showDropdowns: true,
      showWeekNumbers: true,
      alwaysShowCalendars: true
    }, function (start, end, label) {
      handleDateRangeChange(start, end, label);
    });

    // 기본 날짜 범위를 오늘로 설정
    const initialStart = moment().startOf('day');
    const initialEnd = moment().endOf('day');
    $('#dateRangePicker').val(initialStart.format('YYYY-MM-DD') + ' ~ ' + initialEnd.format('YYYY-MM-DD'));

  } catch (error) {
    console.error('DateRangePicker initialization error:', error);
  }
}

async function initFilterSystem() {
  try {
    await loadFactoryOptions();
    await loadLineOptions();
    await loadMachineOptions();
  } catch (error) {
    console.error('Filtering system initialization error:', error);
  }
}

async function loadFactoryOptions() {
  try {
    const response = await fetch('../manage/proc/factory.php?status_filter=Y');
    const res = await response.json();

    if (res.success && res.data) {
      const factorySelect = document.getElementById('factoryFilterSelect');

      if (factorySelect) {
        factorySelect.innerHTML = '<option value="">All Factory</option>';
        res.data.forEach(factory => {
          factorySelect.innerHTML += `<option value="${factory.idx}">${factory.factory_name}</option>`;
        });

        factorySelect.addEventListener('change', async (e) => {
          const machineSelect = document.getElementById('factoryLineMachineFilterSelect');
          machineSelect.innerHTML = '<option value="">All Machine</option>';
          machineSelect.disabled = true;

          await updateLineOptions(e.target.value);
          await restartRealTimeMonitoring();
        });
      }
    } else {
      console.error('Factory list loading failed:', res.message || 'Unknown error');
    }
  } catch (error) {
    console.error('Factory options loading error:', error);
  }
}

async function loadLineOptions() {
  try {
    const response = await fetch('../manage/proc/line.php?status_filter=Y');
    const res = await response.json();

    if (res.success && res.data) {
      const lineSelect = document.getElementById('factoryLineFilterSelect');

      if (lineSelect) {
        lineSelect.innerHTML = '<option value="">All Line</option>';
        res.data.forEach(line => {
          lineSelect.innerHTML += `<option value="${line.idx}">${line.line_name}</option>`;
        });

        lineSelect.addEventListener('change', async (e) => {
          const factoryId = document.getElementById('factoryFilterSelect').value;
          await updateMachineOptions(factoryId, e.target.value);
          await restartRealTimeMonitoring();
        });
      }
    } else {
      console.error('Line list loading failed:', res.message || 'Unknown error');
    }
  } catch (error) {
    console.error('Line options loading error:', error);
  }
}

async function loadMachineOptions() {
  try {
    const response = await fetch('../manage/proc/machine.php');
    const res = await response.json();

    if (res.success && res.data) {
      const machineSelect = document.getElementById('factoryLineMachineFilterSelect');

      if (machineSelect) {
        machineSelect.innerHTML = '<option value="">All Machine</option>';
        res.data.forEach(machine => {
          machineSelect.innerHTML += `<option value="${machine.idx}">${machine.machine_no} (${machine.machine_model_name || 'No Model'})</option>`;
        });
      }
    } else {
      console.error('Machine list loading failed:', res.message || 'Unknown error');
    }
  } catch (error) {
    console.error('Machine options loading error:', error);
  }
}

async function updateLineOptions(factoryId) {
  const lineSelect = document.getElementById('factoryLineFilterSelect');
  lineSelect.disabled = true;

  try {
    let url = '../manage/proc/line.php?status_filter=Y';
    if (factoryId) {
      url += `&factory_filter=${factoryId}`;
    }

    const response = await fetch(url);
    const res = await response.json();

    if (res.success) {
      lineSelect.innerHTML = '<option value="">All Line</option>';
      res.data.forEach(line => {
        lineSelect.innerHTML += `<option value="${line.idx}">${line.line_name}</option>`;
      });
    } else {
      console.error('Line options update failed:', res.message || 'Unknown error');
    }

    lineSelect.disabled = false;

    const machineSelect = document.getElementById('factoryLineMachineFilterSelect');
    if (factoryId) {
      machineSelect.disabled = false;
      updateMachineOptions(factoryId, '');
    }
  } catch (error) {
    console.error('Line options update failed:', error);
    lineSelect.innerHTML = '<option value="">All Line</option>';
    lineSelect.disabled = false;
  }
}

async function updateMachineOptions(factoryId, lineId) {
  const machineSelect = document.getElementById('factoryLineMachineFilterSelect');
  machineSelect.disabled = true;

  try {
    let url = '../manage/proc/machine.php';
    const params = new URLSearchParams();
    if (factoryId) {
      params.append('factory_filter', factoryId);
    }
    if (lineId) {
      params.append('line_filter', lineId);
    }
    if (params.toString()) {
      url += '?' + params.toString();
    }

    const response = await fetch(url);
    const res = await response.json();

    if (res.success) {
      machineSelect.innerHTML = '<option value="">All Machine</option>';
      res.data.forEach(machine => {
        machineSelect.innerHTML += `<option value="${machine.idx}">${machine.machine_no} (${machine.machine_model_name || 'No Model'})</option>`;
      });
    } else {
      console.error('Machine options update failed:', res.message || 'Unknown error');
    }

    machineSelect.disabled = false;
  } catch (error) {
    console.error('Machine options update failed:', error);
    machineSelect.innerHTML = '<option value="">All Machine</option>';
    machineSelect.disabled = false;
  }
}

function initColumnToggle() {
  const columnToggleBtn = document.getElementById('columnToggleBtn');
  const columnToggleDropdown = document.getElementById('columnToggleDropdown');

  if (!columnToggleBtn || !columnToggleDropdown) {
    console.error('Column toggle button or dropdown not found');
    return;
  }

  // 드롭다운 내용 초기화
  columnToggleDropdown.innerHTML = '';

  // 드롭다운 헤더와 닫기 버튼 추가
  columnToggleDropdown.insertAdjacentHTML('beforeend', `
    <div style="display: flex; justify-content: space-between; align-items: center; margin-bottom: var(--sap-spacing-sm); padding-bottom: var(--sap-spacing-xs); border-bottom: 1px solid var(--sap-border-subtle);">
      <strong style="color: var(--sap-text-primary);">Show/Hide Columns</strong>
      <button type="button" class="dropdown-close-btn" style="background: none; border: none; cursor: pointer; color: var(--sap-text-secondary); font-size: 18px; padding: 2px;" title="닫기">✕</button>
    </div>
  `);

  // 컬럼 체크박스 생성
  columnConfig.forEach((col, index) => {
    const label = document.createElement('label');
    label.style.display = 'flex';
    label.style.alignItems = 'center';
    label.style.padding = '4px 8px';
    label.style.cursor = 'pointer';

    const checkbox = document.createElement('input');
    checkbox.type = 'checkbox';
    checkbox.checked = col.visible;
    checkbox.dataset.columnKey = col.key;
    checkbox.id = `col-toggle-${col.key}`;

    const labelText = document.createTextNode(col.label);

    label.appendChild(checkbox);
    label.appendChild(labelText);
    columnToggleDropdown.appendChild(label);

    // 체크박스 변경 이벤트
    checkbox.addEventListener('change', (e) => {
      col.visible = e.target.checked;
      renderTableHeader();
      updateTableFromAPI(oeeData);
    });
  });

  // 드롭다운 토글
  columnToggleBtn.addEventListener('click', (e) => {
    e.stopPropagation();
    columnToggleDropdown.classList.toggle('show');
  });

  // 드롭다운 내부 클릭 처리
  columnToggleDropdown.addEventListener('click', (e) => {
    // 닫기 버튼 클릭 시
    if (e.target.classList.contains('dropdown-close-btn')) {
      e.stopPropagation();
      columnToggleDropdown.classList.remove('show');
      return;
    }

    // 체크박스나 레이블 클릭 시에는 드롭다운 유지
    e.stopPropagation();
  });

  // 외부 클릭 시 드롭다운 닫기
  document.addEventListener('click', (e) => {
    if (!columnToggleBtn.contains(e.target) && !columnToggleDropdown.contains(e.target)) {
      columnToggleDropdown.classList.remove('show');
    }
  });
}

function renderTableHeader() {
  const headerRow = document.getElementById('tableHeaderRow');
  if (!headerRow) return;

  headerRow.innerHTML = '';

  columnConfig.forEach(col => {
    const th = document.createElement('th');
    th.textContent = col.label;
    th.dataset.columnKey = col.key;

    if (!col.visible) {
      th.classList.add('hidden-column');
    }

    // Machine No 컬럼을 sticky로 고정
    if (col.key === 'machine_no') {
      th.classList.add('sticky-column');
    }

    headerRow.appendChild(th);
  });
}

function setupEventListeners() {
  const excelDownloadBtn = document.getElementById('excelDownloadBtn');
  if (excelDownloadBtn) {
    excelDownloadBtn.addEventListener('click', exportData);
  }

  const refreshBtn = document.getElementById('refreshBtn');
  if (refreshBtn) {
    refreshBtn.addEventListener('click', refreshData);
  }

  const toggleStatsBtn = document.getElementById('toggleStatsBtn');
  if (toggleStatsBtn) {
    toggleStatsBtn.addEventListener('click', toggleStatsDisplay);
  }

  const timeRangeSelect = document.getElementById('timeRangeSelect');
  if (timeRangeSelect) {
    timeRangeSelect.addEventListener('change', handleTimeRangeChange);
  }

  const factoryLineMachineFilterSelect = document.getElementById('factoryLineMachineFilterSelect');
  if (factoryLineMachineFilterSelect) {
    factoryLineMachineFilterSelect.addEventListener('change', async (e) => {
      await restartRealTimeMonitoring();
    });
  }

  const shiftSelect = document.getElementById('shiftSelect');
  if (shiftSelect) {
    shiftSelect.addEventListener('change', async (e) => {
      await restartRealTimeMonitoring();
    });
  }
}

async function loadInitialData() {
  try {
    displayEmptyState();
  } catch (error) {
    console.error('Initial data preparation error:', error);
  }
}

function displayEmptyState() {
  const statElements = [
    'overallOee', 'availability', 'performance', 'quality', 'currentShiftOee', 'previousDayOee'
  ];

  statElements.forEach(elementId => {
    const element = document.getElementById(elementId);
    if (element) {
      element.textContent = '-';
    }
  });

  const tbody = document.getElementById('oeeDataBody');
  const visibleColCount = columnConfig.filter(col => col.visible).length;

  tbody.innerHTML = `
    <tr>
      <td colspan="${visibleColCount}" class="data-table-centered">
        <div class="fiori-alert fiori-alert--info">
          <strong>ℹ️ Information:</strong> Loading OEE hourly data log. Please wait...
        </div>
      </td>
    </tr>
  `;
}

async function startAutoTracking() {
  if (isTracking) {
    return;
  }

  try {
    const filters = getFilterParams();
    const params = new URLSearchParams(filters);

    const sseUrl = `proc/log_oee_hourly_stream.php?${params.toString()}`;

    eventSource = new EventSource(sseUrl);
    setupSSEEventListeners();

    isTracking = true;

    const connectionStatusEl = document.getElementById('connectionStatus');
    if (connectionStatusEl) {
      connectionStatusEl.textContent = 'OEE Hourly Data Real-time Connected';
    }


  } catch (error) {
    console.error('Auto SSE connection error:', error);

    const connectionStatusEl = document.getElementById('connectionStatus');
    if (connectionStatusEl) {
      connectionStatusEl.textContent = 'Connection failed';
      connectionStatusEl.className = 'connection-status--error';
    }
  }
}

function getFilterParams() {
  const factoryFilter = document.getElementById('factoryFilterSelect').value;
  const lineFilter = document.getElementById('factoryLineFilterSelect').value;
  const machineFilter = document.getElementById('factoryLineMachineFilterSelect').value;
  const shiftFilter = document.getElementById('shiftSelect').value;

  const dateRange = $('#dateRangePicker').val();
  let startDate = '', endDate = '';

  if (dateRange && dateRange.includes(' ~ ')) {
    const dates = dateRange.split(' ~ ');
    startDate = dates[0];
    endDate = dates[1];
  }

  let shift_idx = '';
  if (shiftFilter) {
    shift_idx = shiftFilter;
  }

  return {
    factory_filter: factoryFilter,
    line_filter: lineFilter,
    machine_filter: machineFilter,
    shift_filter: shift_idx,
    start_date: startDate,
    end_date: endDate,
    limit: 1000
  };
}

function setupSSEEventListeners() {
  eventSource.onerror = function (event) {
    console.error('SSE connection error:', event);
    const connectionStatusEl = document.getElementById('connectionStatus');
    if (connectionStatusEl) {
      connectionStatusEl.textContent = 'SSE Connection Error - Reconnecting...';
      connectionStatusEl.className = 'connection-status--warning';
    }

    isTracking = false;
    attemptReconnection();
  };

  eventSource.addEventListener('connected', function (event) {
    const data = JSON.parse(event.data);
    const connectionStatusEl = document.getElementById('connectionStatus');
    if (connectionStatusEl) {
      connectionStatusEl.textContent = 'OEE hourly data system connected';
      connectionStatusEl.className = 'connection-status--success';
    }

    reconnectAttempts = 0;
  });

  eventSource.addEventListener('oee_data', function (event) {
    const data = JSON.parse(event.data);

    stats = data.stats;
    oeeData = data.oee_data;
    window.oeeData = data.oee_data;

    updateStatCardsFromAPI(stats);
    updateTableFromAPI(oeeData);

    const lastUpdateEl = document.getElementById('lastUpdateTime');
    if (lastUpdateEl) {
      lastUpdateEl.textContent = `Last updated: ${data.timestamp}`;
    }
  });

  eventSource.addEventListener('heartbeat', function (event) {
    const data = JSON.parse(event.data);

    const connectionStatusEl = document.getElementById('connectionStatus');
    if (connectionStatusEl) {
      connectionStatusEl.textContent = `Connection maintained (Active records: ${data.active_machines})`;
    }
  });

  eventSource.addEventListener('error', function (event) {
    const data = JSON.parse(event.data);
    console.error('SSE error:', data);
  });

  eventSource.addEventListener('disconnected', function (event) {
    const data = JSON.parse(event.data);
    const connectionStatusEl = document.getElementById('connectionStatus');
    if (connectionStatusEl) {
      connectionStatusEl.textContent = 'OEE Hourly Data System Connection Closed';
    }
  });
}

function formatPercentage(value) {
  const num = parseFloat(value);
  if (isNaN(num)) return '0%';

  // 정수인지 확인 (소수점 이하가 0인지)
  return (num % 1 === 0 ? Math.floor(num) : num) + '%';
}

function updateStatCardsFromAPI(statsData) {
  if (!statsData) return;

  const elements = {
    'overallOee': formatPercentage(statsData.overall_oee || '0'),
    'availability': formatPercentage(statsData.availability || '0'),
    'performance': formatPercentage(statsData.performance || '0'),
    'quality': formatPercentage(statsData.quality || '0'),
    'currentShiftOee': formatPercentage(statsData.current_shift_oee || '0'),
    'previousDayOee': formatPercentage(statsData.previous_day_oee || '0')
  };

  for (const [elementId, value] of Object.entries(elements)) {
    const element = document.getElementById(elementId);
    if (element) {
      element.textContent = value;
    }
  }
}

function updateTableFromAPI(oeeDataList) {
  const tbody = document.getElementById('oeeDataBody');

  if (!tbody) {
    console.error('oeeDataBody element not found.');
    return;
  }

  totalItems = oeeDataList.length;

  if (oeeDataList.length === 0) {
    const visibleColCount = columnConfig.filter(col => col.visible).length;
    tbody.innerHTML = `
      <tr>
        <td colspan="${visibleColCount}" class="data-table-centered">
          <div class="fiori-alert fiori-alert--info">
            <strong>ℹ️ Information:</strong> No OEE hourly data matching the selected conditions.
          </div>
        </td>
      </tr>
    `;
    renderPagination();
    return;
  }

  const startIndex = (currentPage - 1) * itemsPerPage;
  const endIndex = startIndex + itemsPerPage;
  const paginatedData = oeeDataList.slice(startIndex, endIndex);

  tbody.innerHTML = '';
  paginatedData.forEach(oee => {
    const row = document.createElement('tr');

    columnConfig.forEach(col => {
      const td = document.createElement('td');
      td.dataset.columnKey = col.key;

      if (!col.visible) {
        td.classList.add('hidden-column');
      }

      // Machine No 컬럼을 sticky로 고정
      if (col.key === 'machine_no') {
        td.classList.add('sticky-column');
      }

      // 데이터 렌더링
      let value = oee[col.key] !== undefined && oee[col.key] !== null ? oee[col.key] : '-';

      // 특정 컬럼에 대한 커스텀 렌더링
      if (col.key === 'oee' || col.key === 'availabilty_rate' || col.key === 'productivity_rate' || col.key === 'quality_rate') {
        if (value !== '-') {
          let badgeClass = 'fiori-badge';
          const numValue = parseFloat(value);

          if (col.key === 'oee') {
            if (numValue >= 85) {
              badgeClass += ' fiori-badge--success';
            } else if (numValue >= 70) {
              badgeClass += ' fiori-badge--warning';
            } else {
              badgeClass += ' fiori-badge--error';
            }
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
      } else {
        td.textContent = value;
      }

      row.appendChild(td);
    });

    tbody.appendChild(row);
  });

  renderPagination();
}

function renderPagination() {
  const paginationContainer = document.getElementById('pagination-controls');
  if (!paginationContainer) return;

  const totalPages = Math.ceil(totalItems / itemsPerPage);

  if (totalPages <= 1) {
    paginationContainer.innerHTML = '';
    return;
  }

  const startItem = totalItems === 0 ? 0 : ((currentPage - 1) * itemsPerPage) + 1;
  const endItem = Math.min(currentPage * itemsPerPage, totalItems);

  let paginationHTML = `
    <div class="fiori-pagination__info">
      ${startItem}-${endItem} / ${totalItems} items (${itemsPerPage} per page)
    </div>
    <div class="fiori-pagination__buttons">
  `;

  paginationHTML += `
    <button class="fiori-pagination__button ${currentPage === 1 ? 'fiori-pagination__button--disabled' : ''}"
            onclick="changePage(${currentPage - 1})" ${currentPage === 1 ? 'disabled' : ''}>
      ←
    </button>
  `;

  const startPage = Math.max(1, currentPage - 2);
  const endPage = Math.min(totalPages, currentPage + 2);

  if (startPage > 1) {
    paginationHTML += `<button class="fiori-pagination__button" onclick="changePage(1)">1</button>`;
    if (startPage > 2) {
      paginationHTML += `<span class="fiori-pagination__ellipsis">...</span>`;
    }
  }

  for (let i = startPage; i <= endPage; i++) {
    paginationHTML += `
      <button class="fiori-pagination__button ${i === currentPage ? 'fiori-pagination__button--active' : ''}"
              onclick="changePage(${i})">
        ${i}
      </button>
    `;
  }

  if (endPage < totalPages) {
    if (endPage < totalPages - 1) {
      paginationHTML += `<span class="fiori-pagination__ellipsis">...</span>`;
    }
    paginationHTML += `<button class="fiori-pagination__button" onclick="changePage(${totalPages})">${totalPages}</button>`;
  }

  paginationHTML += `
    <button class="fiori-pagination__button ${currentPage === totalPages ? 'fiori-pagination__button--disabled' : ''}"
            onclick="changePage(${currentPage + 1})" ${currentPage === totalPages ? 'disabled' : ''}>
      →
    </button>
  `;

  paginationHTML += '</div>';
  paginationContainer.innerHTML = paginationHTML;
}

function changePage(newPage) {
  const totalPages = Math.ceil(totalItems / itemsPerPage);

  if (newPage < 1 || newPage > totalPages || newPage === currentPage) {
    return;
  }

  currentPage = newPage;
  updateTableFromAPI(oeeData);
}

function stopTracking() {
  if (!isTracking) {
    return;
  }

  if (eventSource) {
    eventSource.close();
    eventSource = null;
  }

  isTracking = false;

  document.getElementById('connectionStatus').textContent = 'OEE Hourly Data System Connection Closed';

}

async function restartRealTimeMonitoring() {
  currentPage = 1;

  if (isTracking) {
    stopTracking();
    await new Promise(resolve => setTimeout(resolve, 100));
  }

  reconnectAttempts = 0;
  await startAutoTracking();
}

async function attemptReconnection() {
  // 페이지가 언로드 중이면 재연결하지 않음
  if (isPageUnloading) {
    console.log('⚠️ Log OEE Hourly: 페이지 언로드 중 - 재연결 취소');
    return;
  }

  if (reconnectAttempts >= maxReconnectAttempts) {
    return;
  }

  reconnectAttempts++;
  const delayMs = Math.min(1000 * Math.pow(2, reconnectAttempts - 1), 10000);


  if (eventSource) {
    eventSource.close();
    eventSource = null;
  }

  setTimeout(async () => {
    // 재연결 전에 다시 한번 페이지 상태 확인
    if (isPageUnloading) {
      console.log('⚠️ Log OEE Hourly: 페이지 언로드 중 - 재연결 취소');
      return;
    }

    try {
      await startAutoTracking();
    } catch (error) {
      console.error(`Reconnection attempt ${reconnectAttempts} failed:`, error);
      attemptReconnection();
    }
  }, delayMs);
}

async function handleDateRangeChange(startDate, endDate, label) {
  const timeRangeSelect = document.getElementById('timeRangeSelect');
  if (timeRangeSelect) {
    let customOption = timeRangeSelect.querySelector('option[value="custom"]');
    if (!customOption) {
      customOption = document.createElement('option');
      customOption.value = 'custom';
      customOption.textContent = 'Custom Selection';
      timeRangeSelect.appendChild(customOption);
    }
    timeRangeSelect.value = 'custom';
  }


  if (isTracking) {
    stopTracking();
    setTimeout(async () => await startAutoTracking(), 1000);
  } else {
    await startAutoTracking();
  }
}

async function handleTimeRangeChange(event) {
  const timeRange = event.target.value;

  if (timeRange !== 'custom') {
    let startDate, endDate;

    switch (timeRange) {
      case 'today':
        startDate = moment().startOf('day');
        endDate = moment().endOf('day');
        break;
      case 'yesterday':
        startDate = moment().subtract(1, 'days').startOf('day');
        endDate = moment().subtract(1, 'days').endOf('day');
        break;
      case '1w':
        startDate = moment().subtract(7, 'days').startOf('day');
        endDate = moment().endOf('day');
        break;
      case '1m':
        startDate = moment().subtract(30, 'days').startOf('day');
        endDate = moment().endOf('day');
        break;
      default:
        startDate = moment().startOf('day');
        endDate = moment().endOf('day');
    }

    $('#dateRangePicker').data('daterangepicker').setStartDate(startDate);
    $('#dateRangePicker').data('daterangepicker').setEndDate(endDate);
    $('#dateRangePicker').val(startDate.format('YYYY-MM-DD') + ' ~ ' + endDate.format('YYYY-MM-DD'));
  }

  await restartRealTimeMonitoring();
}

function exportData() {
  if (!oeeData || oeeData.length === 0) {
    return;
  }

  try {
    // Get current filter parameters
    const filters = getFilterParams();

    // Build export URL with filter parameters
    const params = new URLSearchParams();

    if (filters.factory_filter) {
      params.append('factory_filter', filters.factory_filter);
    }
    if (filters.line_filter) {
      params.append('line_filter', filters.line_filter);
    }
    if (filters.machine_filter) {
      params.append('machine_filter', filters.machine_filter);
    }
    if (filters.shift_filter) {
      params.append('shift_filter', filters.shift_filter);
    }
    if (filters.start_date) {
      params.append('start_date', filters.start_date);
    }
    if (filters.end_date) {
      params.append('end_date', filters.end_date);
    }

    // Create download URL
    const exportUrl = `proc/log_oee_hourly_export.php?${params.toString()}`;

    // Open in new window to trigger download
    window.open(exportUrl, '_blank');

  } catch (error) {
    console.error('Export error:', error);
  }
}

async function refreshData() {
  if (isTracking) {
    stopTracking();
    setTimeout(async () => await startAutoTracking(), 1000);
  } else {
    await startAutoTracking();
  }

}

function toggleStatsDisplay() {
  const statsGrid = document.getElementById('statsGrid');
  const toggleBtn = document.getElementById('toggleStatsBtn');

  if (!statsGrid || !toggleBtn) {
    console.error('Stats grid or toggle button not found.');
    return;
  }

  if (statsGrid.classList.contains('hidden')) {
    statsGrid.classList.remove('hidden');
    toggleBtn.textContent = '📊 Hide Stats';
  } else {
    statsGrid.classList.add('hidden');
    toggleBtn.textContent = '📊 Show Stats';
  }
}

// Global functions for pagination
window.changePage = changePage;
