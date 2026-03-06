let eventSource = null;
let isTracking = false;
let charts = {};
let andonData = [];
let reconnectAttempts = 0;
let maxReconnectAttempts = 3;
let stats = {};
let activeAndons = [];
let elapsedTimeTimer = null;
let durationUpdateTimer = null;
let currentPage = 1;
let itemsPerPage = 10;
let totalItems = 0;
let isPageUnloading = false; // 페이지 언로드 상태 추적

Chart.defaults.color = '#1a1a1a';
Chart.defaults.borderColor = '#e8eaed';
const chartColors = {
  warning: '#e26b0a',
  error: '#da1e28',
  success: '#30914c',
  info: '#7c3aed',
  primary: '#0070f2',
  secondary: '#1e88e5',
  accent: '#00d4aa'
};

document.addEventListener('DOMContentLoaded', async function () {
  try {
    initDateRangePicker();
    await initFilterSystem();
    initCharts();
    setupEventListeners();
    await loadInitialData();
    await startAutoTracking();
  } catch (error) {
    console.error('Initialization error:', error);
  }
});

// 페이지 언로드 시 SSE 연결 및 타이머 종료 (브라우저 동시 연결 수 제한 문제 해결)
// 우선순위: beforeunload > pagehide > visibilitychange

// 1. beforeunload - 가장 먼저 실행되어 페이지 언로드 플래그 설정
window.addEventListener('beforeunload', () => {
  console.log('Andon: beforeunload - 페이지 언로드 시작');
  isPageUnloading = true; // 플래그 설정으로 재연결 방지

  if (eventSource) {
    eventSource.close();
    eventSource = null;
    isTracking = false;
  }
  if (elapsedTimeTimer) {
    clearInterval(elapsedTimeTimer);
    elapsedTimeTimer = null;
  }
  if (durationUpdateTimer) {
    clearInterval(durationUpdateTimer);
    durationUpdateTimer = null;
  }
});

// 2. pagehide - 브라우저가 페이지를 숨길 때 (모바일에서 중요)
window.addEventListener('pagehide', () => {
  console.log('Andon: pagehide - 페이지 숨김');
  isPageUnloading = true;

  if (eventSource) {
    eventSource.close();
    eventSource = null;
    isTracking = false;
  }
  if (elapsedTimeTimer) {
    clearInterval(elapsedTimeTimer);
    elapsedTimeTimer = null;
  }
  if (durationUpdateTimer) {
    clearInterval(durationUpdateTimer);
    durationUpdateTimer = null;
  }
});

// 3. visibilitychange - 탭 전환이나 페이지 숨김
document.addEventListener('visibilitychange', () => {
  if (document.hidden) {
    // 페이지가 숨겨질 때 SSE 연결과 타이머를 즉시 정리
    console.log('Andon: 페이지 숨김 - SSE 연결 및 타이머 종료');

    if (eventSource && isTracking) {
      eventSource.close();
      eventSource = null;
      isTracking = false;
    }
    if (elapsedTimeTimer) {
      clearInterval(elapsedTimeTimer);
      elapsedTimeTimer = null;
    }
    if (durationUpdateTimer) {
      clearInterval(durationUpdateTimer);
      durationUpdateTimer = null;
    }
  } else {
    // 페이지가 다시 보일 때만 재연결 (언로드 중이 아닐 때만)
    console.log('Andon: 페이지 표시 - SSE 재연결 시도');

    if (!isPageUnloading && !isTracking && eventSource === null) {
      reconnectAttempts = 0;
      startAutoTracking();
    }
  }
});

// 4. 페이지 포커스 복원 시 - 언로드 플래그 리셋
window.addEventListener('focus', () => {
  if (isPageUnloading) {
    console.log('Andon: 페이지 포커스 - 언로드 플래그 리셋');
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
      startDate: moment().subtract(6, 'days'),
      endDate: moment(),
      showDropdowns: true,
      showWeekNumbers: true,
      alwaysShowCalendars: true
    }, function (start, end, label) {
      handleDateRangeChange(start, end, label);
    });

    const initialStart = moment().startOf('day');
    const initialEnd = moment().endOf('day');
    $('#dateRangePicker').val(initialStart.format('YYYY-MM-DD') + ' ~ ' + initialEnd.format('YYYY-MM-DD'));

  } catch (error) {
    console.error('DateRangePicker error:', error);
  }
}

async function initFilterSystem() {
  try {
    await loadFactoryOptions();
    await loadLineOptions();
    await loadMachineOptions();
  } catch (error) {
    console.error('Filter system error:', error);
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
      console.error('Factory list load failed:', res.message || 'Unknown error');
    }
  } catch (error) {
    console.error('Factory options error:', error);
  }
}

/**
 * Line 옵션 초기 로딩
 */
async function loadLineOptions() {
  try {
    console.log('🏭 라인 목록 로딩 중...');

    const response = await fetch('../manage/proc/line.php?status_filter=Y');
    const res = await response.json();

    if (res.success && res.data) {
      const lineSelect = document.getElementById('factoryLineFilterSelect');

      if (lineSelect) {
        lineSelect.innerHTML = '<option value="">All Line</option>';
        res.data.forEach(line => {
          lineSelect.innerHTML += `<option value="${line.idx}">${line.line_name}</option>`;
        });

        // Line 선택 시 Machine 목록 업데이트 이벤트 리스너
        lineSelect.addEventListener('change', async (e) => {
          console.log(`🏭 Line 필터 변경됨: ${e.target.value}`);
          const factoryId = document.getElementById('factoryFilterSelect').value;
          await updateMachineOptions(factoryId, e.target.value);

          // 필터 변경에 따른 실시간 모니터링 재시작
          await restartRealTimeMonitoring();
        });

        console.log('✅ 라인 필터 옵션 로드 완료');
      }
    } else {
      console.error('❌ 라인 목록 로드 실패:', res.message || 'Unknown error');
    }
  } catch (error) {
    console.error('❌ 라인 옵션 로드 에러:', error);
  }
}

/**
 * Machine 옵션 초기 로딩 (실제 API 연동)
 */
async function loadMachineOptions() {
  try {
    console.log('🔧 머신 목록 로딩 중...');

    // 실제 API 연동
    const response = await fetch('../manage/proc/machine.php');
    const res = await response.json();

    if (res.success && res.data) {
      const machineSelect = document.getElementById('factoryLineMachineFilterSelect');

      if (machineSelect) {
        machineSelect.innerHTML = '<option value="">All Machine</option>';
        res.data.forEach(machine => {
          machineSelect.innerHTML += `<option value="${machine.idx}">${machine.machine_no} (${machine.machine_model_name || 'No Model'})</option>`;
        });
        console.log('✅ 머신 필터 옵션 로드 완료');
      }
    } else {
      console.error('❌ 머신 목록 로드 실패:', res.message || 'Unknown error');
    }
  } catch (error) {
    console.error('❌ 머신 옵션 로드 에러:', error);
  }
}

/**
 * Line 옵션 업데이트 (Factory 선택에 따라)
 */
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
      console.error('Line update failed:', res.message || 'Unknown error');
    }

    lineSelect.disabled = false;

    // Machine 필터도 다시 활성화
    const machineSelect = document.getElementById('factoryLineMachineFilterSelect');
    if (factoryId) {
      machineSelect.disabled = false;
      updateMachineOptions(factoryId, '');
    }
  } catch (error) {
    console.error('Line options update error:', error);
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
      console.error('Machine update failed:', res.message || 'Unknown error');
    }

    machineSelect.disabled = false;
  } catch (error) {
    console.error('Machine options update error:', error);
    machineSelect.innerHTML = '<option value="">All Machine</option>';
    machineSelect.disabled = false;
  }
}

/**
 * Initialize Charts (Andon Specific)
 */
function initCharts() {
  console.log('📊 Initializing Andon charts...');

  try {
    // 1. Andon occurrence trend chart
    charts.andonTrend = createAndonTrendChart();

    // 2. Andon status distribution chart (commented - for future use)
    // charts.andonStatus = createAndonStatusChart();

    // 3. Andon type analysis chart
    charts.andonType = createAndonTypeChart();

    // 4. Resolution time analysis chart (commented - for future use)
    // charts.resolutionTime = createResolutionTimeChart();

    console.log('✅ Active Andon chart initialization completed (trend, by type)');
  } catch (error) {
    console.error('❌ Chart initialization error:', error);
  }
}

/**
 * Create Andon Occurrence Trend Chart
 */
function createAndonTrendChart() {
  const ctx = document.getElementById('andonTrendChart').getContext('2d');
  return new Chart(ctx, {
    type: 'line',
    data: {
      labels: [], // 실제 데이터로 업데이트
      datasets: [
        {
          label: '⚠️ Warning',
          data: [], // 실제 데이터로 업데이트
          borderColor: chartColors.error,
          backgroundColor: chartColors.error + '20',
          fill: true,
          tension: 0.4,
          pointBackgroundColor: chartColors.error,
          pointBorderColor: chartColors.error,
          pointRadius: 4,
          pointHoverRadius: 6
        },
        {
          label: '✅ Completed',
          data: [], // 실제 데이터로 업데이트
          borderColor: chartColors.success,
          backgroundColor: chartColors.success + '20',
          fill: true,
          tension: 0.4,
          pointBackgroundColor: chartColors.success,
          pointBorderColor: chartColors.success,
          pointRadius: 4,
          pointHoverRadius: 6
        }
      ]
    },
    options: {
      responsive: true,
      maintainAspectRatio: false,
      plugins: {
        legend: {
          display: true,
          position: 'top'
        },
        tooltip: {
          mode: 'index',
          intersect: false,
          backgroundColor: 'rgba(255, 255, 255, 0.95)',
          titleColor: '#333',
          bodyColor: '#666',
          borderColor: '#ddd',
          borderWidth: 1,
          cornerRadius: 8,
          displayColors: true
        }
      },
      scales: {
        y: {
          beginAtZero: true,
          title: {
            display: true,
            text: 'Andon Count'
          },
          ticks: {
            stepSize: 1,
            precision: 0
          },
          grid: {
            color: 'rgba(0, 0, 0, 0.1)'
          }
        },
        x: {
          title: {
            display: true,
            text: 'Time/Period'
          },
          grid: {
            color: 'rgba(0, 0, 0, 0.1)'
          }
        }
      },
      interaction: {
        mode: 'nearest',
        axis: 'x',
        intersect: false
      }
    }
  });
}

/**
 * Create Andon Status Distribution Chart (Commented - For Future Use)
 */
/*
function createAndonStatusChart() {
  const ctx = document.getElementById('andonStatusChart').getContext('2d');
  return new Chart(ctx, {
    type: 'doughnut',
    data: {
      labels: ['경고 (미해결)', '완료 (해결됨)'],
      datasets: [{
        data: [15, 85],
        backgroundColor: [
          chartColors.error,
          chartColors.success
        ],
        borderWidth: 2
      }]
    },
    options: {
      responsive: true,
      maintainAspectRatio: false,
      plugins: {
        legend: {
          position: 'bottom'
        }
      }
    }
  });
}
*/

/**
 * Create Andon Type Analysis Chart
 */
function createAndonTypeChart() {
  const ctx = document.getElementById('andonTypeChart').getContext('2d');
  return new Chart(ctx, {
    type: 'bar',
    data: {
      labels: [], // 실제 데이터로 업데이트
      datasets: [{
        label: 'Andon Occurrence Count',
        data: [], // 실제 데이터로 업데이트
        backgroundColor: [
          chartColors.warning,
          chartColors.error,
          chartColors.primary,
          chartColors.info,
          chartColors.accent,
          chartColors.secondary
        ],
        borderColor: [
          chartColors.warning,
          chartColors.error,
          chartColors.primary,
          chartColors.info,
          chartColors.accent,
          chartColors.secondary
        ],
        borderWidth: 1
      }]
    },
    options: {
      responsive: true,
      maintainAspectRatio: false,
      plugins: {
        legend: {
          display: false
        }
      },
      scales: {
        y: {
          beginAtZero: true,
          title: {
            display: true,
            text: 'Andon Count'
          },
          ticks: {
            stepSize: 1,
            precision: 0
          }
        },
        x: {
          title: {
            display: true,
            text: 'Andon Type'
          }
        }
      }
    }
  });
}

/**
 * Create Resolution Time Analysis Chart (Commented - For Future Use)
 */
/*
function createResolutionTimeChart() {
  const ctx = document.getElementById('resolutionTimeChart').getContext('2d');
  return new Chart(ctx, {
    type: 'pie',
    data: {
      labels: ['1분 이내', '1-5분', '5-15분', '15분 이상'],
      datasets: [{
        data: [35, 40, 20, 5],
        backgroundColor: [
          chartColors.success,
          chartColors.info,
          chartColors.warning,
          chartColors.error
        ]
      }]
    },
    options: {
      responsive: true,
      maintainAspectRatio: false,
      plugins: {
        legend: {
          position: 'bottom'
        }
      }
    }
  });
}
*/

/**
 * Setup Event Listeners
 */
function setupEventListeners() {
  console.log('🎧 Setting up event listeners...');


  // Data export button (removed as commented section)
  const excelDownloadBtn = document.getElementById('excelDownloadBtn');
  if (excelDownloadBtn) {
    excelDownloadBtn.addEventListener('click', exportData);
    console.log('✅ excelDownloadBtn event listener added');
  } else {
    console.log('⚠️ Unable to find excelDownloadBtn element.');
  }

  // Refresh button
  const refreshBtn = document.getElementById('refreshBtn');
  if (refreshBtn) {
    refreshBtn.addEventListener('click', refreshData);
    console.log('✅ refreshBtn event listener added');
  } else {
    console.log('⚠️ Unable to find refreshBtn element.');
  }

  // Toggle stats button
  const toggleStatsBtn = document.getElementById('toggleStatsBtn');
  if (toggleStatsBtn) {
    toggleStatsBtn.addEventListener('click', toggleStatsDisplay);
  }

  // Toggle charts button
  const toggleChartsBtn = document.getElementById('toggleChartsBtn');
  if (toggleChartsBtn) {
    toggleChartsBtn.addEventListener('click', toggleChartsDisplay);
  }

  // Time range filter change
  const timeRangeSelect = document.getElementById('timeRangeSelect');
  if (timeRangeSelect) {
    timeRangeSelect.addEventListener('change', handleTimeRangeChange);
    console.log('✅ timeRangeSelect event listener added');
  } else {
    console.log('⚠️ Unable to find timeRangeSelect element.');
  }

  // Machine filter change (restart real-time monitoring)
  const factoryLineMachineFilterSelect = document.getElementById('factoryLineMachineFilterSelect');
  if (factoryLineMachineFilterSelect) {
    factoryLineMachineFilterSelect.addEventListener('change', async (e) => {
      console.log(`🔧 Machine filter changed: ${e.target.value}`);
      // 필터 변경에 따른 실시간 모니터링 재시작
      await restartRealTimeMonitoring();
    });
    console.log('✅ factoryLineMachineFilterSelect event listener added');
  } else {
    console.log('⚠️ Unable to find factoryLineMachineFilterSelect element.');
  }

  // Shift filter change (restart real-time monitoring)
  const shiftSelect = document.getElementById('shiftSelect');
  if (shiftSelect) {
    shiftSelect.addEventListener('change', async (e) => {
      console.log(`⏰ Shift filter changed: ${e.target.value}`);
      // 필터 변경에 따른 실시간 모니터링 재시작
      await restartRealTimeMonitoring();
    });
    console.log('✅ shiftSelect event listener added');
  } else {
    console.log('⚠️ Unable to find shiftSelect element.');
  }

  console.log('✅ All event listeners setup completed');
}

/**
 * 초기 데이터 로딩 (실제 API 연동 없이 빈 상태로 시작)
 */
async function loadInitialData() {
  console.log('📋 초기 Andon 데이터 준비 중...');

  try {
    // 초기에는 연결 전 상태로 빈 데이터 표시
    displayEmptyState();

    console.log('✅ 초기 Andon 데이터 준비 완료');
  } catch (error) {
    console.error('❌ 초기 데이터 준비 오류:', error);
  }
}

/**
 * 빈 상태 표시 (데이터 로딩 전)
 */
function displayEmptyState() {
  // Stat Cards를 빈 상태로 초기화 (null 체크 포함) - 6개 항목으로 변경
  const statElements = [
    'totalAndons', 'activeWarnings', 'currentShiftCount',
    'affectedMachines', 'urgentWarnings', 'avgCompletedTime'
    // 'completedAndons', // 나중에 사용할 수 있게 주석 처리
    // 'andonTypes'      // 나중에 사용할 수 있게 주석 처리
    // 'todayAndons', 'avgResolutionTime' // 사용하지 않는 항목들
  ];

  statElements.forEach(elementId => {
    const element = document.getElementById(elementId);
    if (element) {
      element.textContent = '-';
    } else {
      console.warn(`⚠️ 초기화 시 요소를 찾을 수 없습니다: ${elementId}`);
    }
  });

  // 활성 안돈 업데이트
  const activeCountEl = document.getElementById('activeAndonCount');
  if (activeCountEl) {
    activeCountEl.textContent = '0 active alerts';
  } else {
    console.warn('⚠️ activeAndonCount 요소를 찾을 수 없습니다.');
  }

  // 테이블 초기 메시지 유지
  const tbody = document.getElementById('andonDataBody');
  tbody.innerHTML = `
    <tr>
      <td colspan="10" style="text-align: center; padding: var(--sap-spacing-xl);">
        <div class="fiori-alert fiori-alert--info">
          <strong>ℹ️ Information:</strong> Loading real-time Andon data. Automatic monitoring is in progress.
        </div>
      </td>
    </tr>
  `;
}


/**
 * 자동 실시간 모니터링 시작 (페이지 로드 시 자동 호출)
 */
async function startAutoTracking() {
  if (isTracking) {
    console.log('⚠️ 이미 모니터링 중입니다.');
    return;
  }

  console.log('🔄 Andon 자동 실시간 모니터링 시작...');

  try {
    // 새로운 연결 시도이므로 재연결 횟수는 건드리지 않음 (자동 재연결과 구분)
    // 필터 파라미터 구성
    const filters = getFilterParams();
    const params = new URLSearchParams(filters);

    const sseUrl = `proc/data_andon_stream.php?${params.toString()}`;

    // 실제 SSE 연결
    eventSource = new EventSource(sseUrl);

    // SSE 이벤트 리스너 설정
    setupSSEEventListeners();

    isTracking = true;

    // 연결 상태 업데이트
    const connectionStatusEl = document.getElementById('connectionStatus');
    if (connectionStatusEl) {
      connectionStatusEl.textContent = 'Andon 시스템 실시간 연결됨';
    }


  } catch (error) {
    console.error('❌ 자동 SSE 연결 오류:', error);

    // 연결 상태 업데이트
    const connectionStatusEl = document.getElementById('connectionStatus');
    if (connectionStatusEl) {
      connectionStatusEl.textContent = 'Connection failed';
      connectionStatusEl.className = 'connection-status--error';
    }
  }
}

/**
 * 필터 파라미터 구성
 */
function getFilterParams() {
  const factoryFilter = document.getElementById('factoryFilterSelect').value;
  const lineFilter = document.getElementById('factoryLineFilterSelect').value;
  const machineFilter = document.getElementById('factoryLineMachineFilterSelect').value;
  const shiftFilter = document.getElementById('shiftSelect').value;

  // DateRangePicker 값 파싱
  const dateRange = $('#dateRangePicker').val();
  let startDate = '', endDate = '';

  if (dateRange && dateRange.includes(' ~ ')) {
    const dates = dateRange.split(' ~ ');
    startDate = dates[0];
    endDate = dates[1];
  }

  // shift_idx 변수 할당
  let shift_idx = '';
  if (shiftFilter) {
    shift_idx = shiftFilter; // "1", "2", "3" 값을 그대로 사용
  }

  return {
    factory_filter: factoryFilter,
    line_filter: lineFilter,
    machine_filter: machineFilter,
    shift_filter: shift_idx,
    start_date: startDate,
    end_date: endDate,
    limit: 100
  };
}

/**
 * SSE 이벤트 리스너 설정
 */
function setupSSEEventListeners() {
  // 일반 연결 오류 처리
  eventSource.onerror = function (event) {
    console.error('❌ SSE 연결 오류:', event);
    const connectionStatusEl = document.getElementById('connectionStatus');
    if (connectionStatusEl) {
      connectionStatusEl.textContent = 'SSE 연결 오류 - 재연결 시도 중...';
      connectionStatusEl.className = 'connection-status--warning';
    }

    // 추적 상태를 false로 설정하여 재연결 준비
    isTracking = false;

    // 자동 재연결 시도
    attemptReconnection();
  };

  // 연결 성공
  eventSource.addEventListener('connected', function (event) {
    const data = JSON.parse(event.data);
    console.log('✅ SSE 연결 성공:', data);
    const connectionStatusEl = document.getElementById('connectionStatus');
    if (connectionStatusEl) {
      connectionStatusEl.textContent = 'Andon system connected';
      connectionStatusEl.className = 'connection-status--success';
    }

    // 재연결 시도 횟수 초기화 (성공적으로 연결됨)
    reconnectAttempts = 0;

  });

  // 메인 안돈 데이터
  eventSource.addEventListener('andon_data', function (event) {
    const data = JSON.parse(event.data);

    // 데이터 업데이트 (전역 변수로 저장)
    stats = data.stats;
    activeAndons = data.active_andons;
    andonData = data.andon_data;
    window.andonData = data.andon_data; // 전역 접근 가능하도록

    // UI 업데이트
    updateStatCardsFromAPI(stats);
    updateActiveAndonsDisplay(activeAndons);
    updateTableFromAPI(andonData);
    updateChartsFromAPI(data);

    // 마지막 업데이트 시간 표시
    const lastUpdateEl = document.getElementById('lastUpdateTime');
    if (lastUpdateEl) {
      lastUpdateEl.textContent = `Last updated: ${data.timestamp}`;
    } else {
      console.warn('⚠️ lastUpdateTime 요소를 찾을 수 없습니다.');
    }
  });

  // Heartbeat (조용한 연결 유지)
  eventSource.addEventListener('heartbeat', function (event) {
    const data = JSON.parse(event.data);

    // 연결 상태 업데이트
    const connectionStatusEl = document.getElementById('connectionStatus');
    if (connectionStatusEl) {
      connectionStatusEl.textContent = `Connection maintained (Active warnings: ${data.active_warnings})`;
    }
  });

  // 오류 처리
  eventSource.addEventListener('error', function (event) {
    const data = JSON.parse(event.data);
    console.error('❌ SSE 오류:', data);
  });

  // 연결 종료
  eventSource.addEventListener('disconnected', function (event) {
    const data = JSON.parse(event.data);
    console.log('🔌 SSE 연결 종료:', data);
    const connectionStatusEl = document.getElementById('connectionStatus');
    if (connectionStatusEl) {
      connectionStatusEl.textContent = 'Andon 시스템 연결 종료됨';
    }
  });
}

/**
 * API에서 받은 통계 데이터로 Stat Cards 업데이트
 */
function updateStatCardsFromAPI(statsData) {
  if (!statsData) return;

  // DOM 요소들이 존재하는지 확인 후 업데이트 - 6개 항목으로 변경
  const elements = {
    'totalAndons': statsData.total_count || '0',
    'activeWarnings': statsData.warning_count || '0',
    'currentShiftCount': statsData.current_shift_count || '0', // 현재 shift 발생 수량
    'affectedMachines': statsData.affected_machines || '0',
    'urgentWarnings': statsData.urgent_warnings_count || '0', // 5분이상 미해결 수량
    'avgCompletedTime': statsData.avg_completed_time || '-' // 평균 completed 시간
    // 나중에 사용할 수 있게 주석 처리
    // 'completedAndons': statsData.completed_count || '0',
    // 'andonTypes': statsData.andon_types_used || '0'
    // 사용하지 않는 항목들
    // 'avgResolutionTime': statsData.avg_duration_display || '-',
    // 'todayAndons': statsData.today_count || '0'
  };

  for (const [elementId, value] of Object.entries(elements)) {
    const element = document.getElementById(elementId);
    if (element) {
      element.textContent = value;
    } else {
      console.warn(`⚠️ 요소를 찾을 수 없습니다: ${elementId}`);
    }
  }
}

/**
 * 활성 안돈 표시 업데이트
 */
function updateActiveAndonsDisplay(activeAndonsList) {
  const container = document.getElementById('activeAndonsContainer');
  const countDisplay = document.getElementById('activeAndonCount');

  if (countDisplay) {
    countDisplay.textContent = `${activeAndonsList.length} active alerts`;
  } else {
    console.warn('⚠️ activeAndonCount 요소를 찾을 수 없습니다.');
  }

  if (!container) {
    console.warn('⚠️ activeAndonsContainer 요소를 찾을 수 없습니다.');
    return;
  }

  if (activeAndonsList.length === 0) {
    container.innerHTML = `
      <div class="fiori-alert fiori-alert--success">
        <strong>✅ Good:</strong> There are currently no active Andons. All systems are operating normally.
      </div>
    `;
    // 활성 안돈이 없으면 타이머 정지
    stopElapsedTimeTimer();
  } else {
    let html = '';
    activeAndonsList.forEach(andon => {
      // 초기 경과 시간 계산 (실시간 업데이트는 타이머가 처리)
      const initialElapsed = calculateElapsedTime(andon.reg_date);

      // 안돈 색상을 테두리와 배경색에 적용 (기본값: SAP 브랜드 색상)
      const borderColor = andon.andon_color || '#0070f2';

      // 연한 배경색 생성 (투명도 10%)
      const backgroundColor = `${borderColor}1A`; // HEX 색상에 알파 값 추가 (10% 투명도)

      const borderStyle = `border: 2px dashed ${borderColor}; background-color: ${backgroundColor};`;

      html += `
        <div class="active-andon-item" style="${borderStyle}">
          <div class="andon-machine-info">
            <div class="andon-machine-name">${andon.andon_name}</div>
            <div class="andon-location">
              <span class="andon-location-item">${andon.factory_name}</span>
              <span class="andon-location-item">/</span>
              <span class="andon-location-item">${andon.line_name}</span>
              <span class="andon-location-item">/</span>
              <span class="andon-location-item">${andon.machine_no}</span>
            </div>
          </div>
          <div class="andon-elapsed-time">
            <span>⏰ ${initialElapsed}</span>
          </div>
        </div>
      `;
    });
    container.innerHTML = html;

    // 활성 안돈이 있으면 실시간 타이머 시작
    startElapsedTimeTimer();
  }
}

/**
 * API 데이터로 테이블 업데이트
 */
function updateTableFromAPI(andonDataList) {
  const tbody = document.getElementById('andonDataBody');

  if (!tbody) {
    console.error('❌ andonDataBody 요소를 찾을 수 없습니다.');
    return;
  }

  // 전체 데이터 수 업데이트
  totalItems = andonDataList.length;

  if (andonDataList.length === 0) {
    tbody.innerHTML = `
      <tr>
        <td colspan="10" style="text-align: center; padding: var(--sap-spacing-xl);">
          <div class="fiori-alert fiori-alert--info">
            <strong>ℹ️ Information:</strong> No Andon data matching the selected conditions.
          </div>
        </td>
      </tr>
    `;
    // 빈 페이지네이션 표시
    renderPagination();
    return;
  }

  // 페이지네이션을 위한 데이터 분할
  const startIndex = (currentPage - 1) * itemsPerPage;
  const endIndex = startIndex + itemsPerPage;
  const paginatedData = andonDataList.slice(startIndex, endIndex);

  tbody.innerHTML = '';
  paginatedData.forEach(andon => {
    const row = document.createElement('tr');

    let statusBadge = '';
    if (andon.status === 'Warning') {
      statusBadge = '<span class="fiori-badge fiori-badge--error">⚠️ Warning</span>';
    } else {
      statusBadge = '<span class="fiori-badge fiori-badge--success">✅ Completed</span>';
    }

    // 교대 표시 (shift_idx 값에 따라)
    let shiftDisplay = '-';
    if (andon.shift_idx) {
      shiftDisplay = `Shift ${andon.shift_idx}`;
    }

    // Duration 셀 처리 - Warning 상태일 경우 실시간 업데이트 적용
    let durationCell = '';
    if (andon.status === 'Warning' && andon.reg_date) {
      // Warning 상태 - 실시간 Duration 계산 및 빨간색 표시
      const initialDuration = calculateElapsedTime(andon.reg_date);
      durationCell = `<td class="duration-in-progress" data-start-time="${andon.reg_date}">${initialDuration}</td>`;
    } else {
      // Completed 상태 - 고정된 Duration 표시
      durationCell = `<td>${andon.duration_display || andon.duration_his || '-'}</td>`;
    }

    row.innerHTML = `
      <td>${andon.machine_no || '-'}</td>
      <td>${(andon.factory_name || '') + ' / ' + (andon.line_name || '')}</td>
      <td>${shiftDisplay}</td>
      <td>${andon.andon_name || '-'}</td>
      <td>${statusBadge}</td>
      <td>${andon.reg_date || '-'}</td>
      <td>${andon.update_date || '-'}</td>
      ${durationCell}
      <td>${andon.work_date || '-'}</td>
      <td>
        <button class="fiori-btn fiori-btn--tertiary andon-details-btn"
                style="padding: 0.25rem 0.5rem; font-size: 0.75rem;"
                data-andon-data='${JSON.stringify(andon).replace(/'/g, "&#39;")}'>
          🔍
        </button>
      </td>
    `;

    tbody.appendChild(row);
  });

  // 페이지네이션 렌더링
  renderPagination();

  // Details 버튼에 이벤트 리스너 추가
  setupDetailsButtonListeners();

  // Duration 실시간 업데이트 타이머 시작/재시작
  startDurationUpdateTimer();
}

/**
 * Details 버튼에 이벤트 리스너 설정
 */
function setupDetailsButtonListeners() {
  const detailsButtons = document.querySelectorAll('.andon-details-btn');

  detailsButtons.forEach(button => {
    // 기존 이벤트 리스너 제거 (중복 방지)
    button.removeEventListener('click', handleDetailsButtonClick);

    // 새 이벤트 리스너 추가
    button.addEventListener('click', handleDetailsButtonClick);
  });

  console.log(`✅ Details 버튼 이벤트 리스너 설정 완료: ${detailsButtons.length}개`);
}

/**
 * Details 버튼 클릭 처리 함수
 * @param {Event} event - 클릭 이벤트
 */
function handleDetailsButtonClick(event) {
  event.preventDefault();
  event.stopPropagation();

  const button = event.target.closest('.andon-details-btn');
  if (button) {
    openAndonDetailModal(button);
  }
}

/**
 * 페이지네이션 렌더링 함수
 */
function renderPagination() {
  const paginationContainer = document.getElementById('pagination-controls');
  if (!paginationContainer) return;

  const totalPages = Math.ceil(totalItems / itemsPerPage);

  if (totalPages <= 1) {
    paginationContainer.innerHTML = '';
    return;
  }

  // 페이지네이션 정보 생성
  const startItem = totalItems === 0 ? 0 : ((currentPage - 1) * itemsPerPage) + 1;
  const endItem = Math.min(currentPage * itemsPerPage, totalItems);

  let paginationHTML = `
    <div class="fiori-pagination__info">
      ${startItem}-${endItem} / ${totalItems} items (${itemsPerPage} per page)
    </div>
    <div class="fiori-pagination__buttons">
  `;

  // 이전 버튼
  paginationHTML += `
    <button class="fiori-pagination__button ${currentPage === 1 ? 'fiori-pagination__button--disabled' : ''}" 
            onclick="changePage(${currentPage - 1})" ${currentPage === 1 ? 'disabled' : ''}>
      ←
    </button>
  `;

  // 페이지 번호 버튼들
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

  // 다음 버튼
  paginationHTML += `
    <button class="fiori-pagination__button ${currentPage === totalPages ? 'fiori-pagination__button--disabled' : ''}" 
            onclick="changePage(${currentPage + 1})" ${currentPage === totalPages ? 'disabled' : ''}>
      →
    </button>
  `;

  paginationHTML += '</div>';
  paginationContainer.innerHTML = paginationHTML;
}

/**
 * 페이지 변경 함수
 */
function changePage(newPage) {
  const totalPages = Math.ceil(totalItems / itemsPerPage);

  if (newPage < 1 || newPage > totalPages || newPage === currentPage) {
    return;
  }

  currentPage = newPage;

  // 현재 안돈 데이터로 테이블 다시 렌더링
  updateTableFromAPI(andonData);

  console.log(`📄 페이지 변경: ${currentPage}/${totalPages}`);
}

/**
 * 차트 데이터 업데이트
 */
function updateChartsFromAPI(apiData) {
  try {

    // 1. 안돈 유형별 차트 업데이트
    if (apiData.andon_type_stats && charts.andonType) {
      updateAndonTypeChart(apiData.andon_type_stats);
    }

    // 2. 안돈 발생 추이 차트 업데이트

    if (apiData.andon_trend_stats && charts.andonTrend) {
      updateAndonTrendChart(apiData.andon_trend_stats);
    } else {
      console.warn('⚠️ 안돈 추이 차트 업데이트 건너뜀:', {
        hasAndonTrendStats: !!apiData.andon_trend_stats,
        hasChart: !!charts.andonTrend
      });
    }

    // 3. 기타 차트들 (주석 처리됨 - 추후 사용)
    // if (apiData.andon_status_stats && charts.andonStatus) {
    //   updateAndonStatusChart(apiData.andon_status_stats);
    // }
    // if (apiData.resolution_time_stats && charts.resolutionTime) {
    //   updateResolutionTimeChart(apiData.resolution_time_stats);
    // }

  } catch (error) {
    console.error('❌ 차트 업데이트 중 오류:', error);
  }
}

/**
 * 안돈 유형별 차트 데이터 업데이트
 */
function updateAndonTypeChart(andonTypeStats) {
  // console.log('🔍 updateAndonTypeChart 호출됨:', {  // 주석 처리 - 나중에 사용 가능
  //   hasChart: !!charts.andonType,
  //   hasData: !!andonTypeStats,
  //   dataLength: andonTypeStats ? andonTypeStats.length : 0,
  //   data: andonTypeStats
  // });

  if (!charts.andonType) {
    console.error('❌ charts.andonType이 초기화되지 않았습니다.');
    return;
  }

  if (!andonTypeStats || andonTypeStats.length === 0) {
    console.warn('⚠️ andonTypeStats 데이터가 없거나 비어있습니다.');
    return;
  }

  try {
    // andon_name으로 정렬 (Downtime과 일관성 유지)
    const sortedStats = [...andonTypeStats].sort((a, b) => {
      const nameA = (a.andon_name || 'Unclassified').toLowerCase();
      const nameB = (b.andon_name || 'Unclassified').toLowerCase();
      return nameA.localeCompare(nameB);
    });

    // 안돈 이름, 발생 건수, 색상 정보 추출 (정렬된 순서)
    const labels = sortedStats.map(item => item.andon_name || 'Unclassified');
    const counts = sortedStats.map(item => parseInt(item.count) || 0);
    const colors = sortedStats.map(item => item.andon_color);

    console.log('🔍 차트 데이터 추출 (정렬됨):', {
      labels: labels,
      counts: counts,
      colors: colors
    });

    // 실제 안돈 색상 사용, 없으면 예비 색상 사용
    const backgroundColors = [];
    const borderColors = [];
    const fallbackColors = ['#0070f2', '#da1e28', '#e26b0a', '#30914c', '#8e44ad', '#1e88e5'];

    for (let i = 0; i < labels.length; i++) {
      const andonColor = colors[i] || fallbackColors[i % fallbackColors.length];
      backgroundColors.push(andonColor);
      borderColors.push(andonColor);
    }

    // 차트 데이터 업데이트
    charts.andonType.data.labels = labels;
    charts.andonType.data.datasets[0].data = counts;
    charts.andonType.data.datasets[0].backgroundColor = backgroundColors;
    charts.andonType.data.datasets[0].borderColor = borderColors;

    // 차트 다시 그리기
    charts.andonType.update('none'); // 애니메이션 없이 업데이트

    console.log(`✅ 안돈 유형별 차트 업데이트 완료: ${labels.length}개 유형`);

  } catch (error) {
    console.error('❌ 안돈 유형별 차트 업데이트 오류:', error);
  }
}

/**
 * 안돈 발생 추이 차트 데이터 업데이트
 */
function updateAndonTrendChart(andonTrendStats) {
  console.log('🔍 안돈 추이 차트 업데이트 시작:', andonTrendStats);

  if (!charts.andonTrend) {
    console.error('❌ charts.andonTrend가 초기화되지 않았습니다.');
    return;
  }

  if (!andonTrendStats) {
    console.warn('⚠️ andonTrendStats가 없습니다.');
    charts.andonTrend.data.labels = [];
    charts.andonTrend.data.datasets[0].data = [];
    charts.andonTrend.data.datasets[1].data = [];
    charts.andonTrend.update('none');
    return;
  }

  if (!andonTrendStats.data || andonTrendStats.data.length === 0) {
    console.warn('⚠️ 안돈 추이 데이터가 비어있습니다.');

    // 대체 로직: andonData에서 간단한 추이 생성
    if (window.andonData && window.andonData.length > 0) {
      console.log('🔄 Generate simple trend with existing Andon data:', window.andonData.length + ' items');
      generateSimpleTrendFromAndonData();
      return;
    }

    // 완전히 비어있는 경우
    charts.andonTrend.data.labels = ['No Data'];
    charts.andonTrend.data.datasets[0].data = [0];
    charts.andonTrend.data.datasets[1].data = [0];
    charts.andonTrend.update('none');
    return;
  }

  try {
    // Handle new API response structure with work_hours
    let trendData = andonTrendStats.data || andonTrendStats;
    let workHours = null;
    let viewType = 'hourly';

    // Check if response has new structure (data + work_hours)
    if (andonTrendStats && typeof andonTrendStats === 'object' && andonTrendStats.data) {
      trendData = andonTrendStats.data;
      workHours = andonTrendStats.work_hours;
      viewType = andonTrendStats.view_type || 'hourly';
    }

    console.log('🔍 추이 차트 원본 데이터:', trendData);
    console.log('🔍 뷰 타입:', viewType);
    console.log('🔍 근무시간 정보:', workHours);

    // Generate labels based on view type and work hours
    let labels = [];

    if (viewType === 'hourly' && workHours && workHours.start_time && workHours.end_time) {
      // For 1 day or less: show full work hours range
      labels = generateWorkHoursLabels(
        workHours.start_time,
        workHours.end_time,
        workHours.start_minutes,
        workHours.end_minutes
      );
    } else if (viewType === 'daily') {
      // Daily view: use display_label directly (e.g., "10/10", "10/14")
      labels = trendData.map(item => item.display_label || item.time_label);
    } else {
      // Hourly view without work_hours: format datetime to hour labels
      labels = trendData.map(item => {
        const originalLabel = item.time_label || item.display_label;

        // Format datetime: "2025-10-13 09:00:00" → "09H"
        if (originalLabel && originalLabel.includes(':')) {
          const match = originalLabel.match(/^(\d{4}-\d{2}-\d{2})\s+(\d{2}):/);
          if (match) {
            return `${match[2]}H`;
          }
        }

        return originalLabel;
      });
    }

    // Map counts to labels
    const warningCountsMap = {};
    const completedCountsMap = {};

    trendData.forEach(item => {
      let labelKey = '';

      if (viewType === 'daily') {
        // Daily view: use display_label as key
        labelKey = item.display_label || item.time_label;
      } else {
        // Hourly view: extract hour label from time_label
        const originalLabel = item.time_label || item.display_label;
        if (originalLabel && originalLabel.includes(':')) {
          const match = originalLabel.match(/^(\d{4}-\d{2}-\d{2})\s+(\d{2}):/);
          if (match) {
            labelKey = `${match[2]}H`;
          }
        }
      }

      if (labelKey) {
        warningCountsMap[labelKey] = parseInt(item.warning_count) || 0;
        completedCountsMap[labelKey] = parseInt(item.completed_count) || 0;
      }
    });

    // Fill in counts array with 0 for missing hours
    // Warning should show cumulative count (warning + completed)
    const completedCounts = labels.map(label => completedCountsMap[label] || 0);
    const warningCounts = labels.map(label => {
      const warning = warningCountsMap[label] || 0;
      const completed = completedCountsMap[label] || 0;
      return warning + completed; // Warning = Warning + Completed (누적값)
    });

    console.log('📊 차트 라벨:', labels);
    console.log('📊 Warning 카운트:', warningCounts);
    console.log('📊 Completed 카운트:', completedCounts);

    // X축 제목을 뷰 타입에 따라 변경
    const xAxisTitle = viewType === 'hourly' ? 'Time (within 1 day)' : 'Date (more than 1 day)';

    // 차트 데이터 업데이트 - Warning/Completed 상태 구분 표시
    charts.andonTrend.data.labels = labels;
    charts.andonTrend.data.datasets[0].data = warningCounts; // Warning 상태만
    charts.andonTrend.data.datasets[1].data = completedCounts; // Completed 상태만

    // 데이터셋 라벨 및 스타일 업데이트
    charts.andonTrend.data.datasets[0].label = '⚠️ Warning';
    charts.andonTrend.data.datasets[0].borderColor = chartColors.error;
    charts.andonTrend.data.datasets[0].backgroundColor = chartColors.error + '20';

    charts.andonTrend.data.datasets[1].label = '✅ Completed';
    charts.andonTrend.data.datasets[1].borderColor = chartColors.success;
    charts.andonTrend.data.datasets[1].backgroundColor = chartColors.success + '20';

    // X축 제목 업데이트
    charts.andonTrend.options.scales.x.title.text = xAxisTitle;

    // 차트 다시 그리기 (부드러운 애니메이션과 함께)
    charts.andonTrend.update('show');

    console.log(`✅ 안돈 발생 추이 차트 업데이트 완료: ${labels.length}개 포인트 (${viewType} view)`);

  } catch (error) {
    console.error('❌ 안돈 발생 추이 차트 업데이트 오류:', error);
  }
}

/**
 * Generate hourly labels for work hours range
 * @param {string} startTime - Start time in HH:mm format (e.g., "07:00")
 * @param {string} endTime - End time in HH:mm format (e.g., "22:00")
 * @param {number} startMinutes - Optional: start time in minutes since midnight
 * @param {number} endMinutes - Optional: end time in minutes since midnight
 * @return {array} - Array of hour labels (e.g., ["07H", "08H", ..., "22H"])
 */
function generateWorkHoursLabels(startTime, endTime, startMinutes = null, endMinutes = null) {
  const labels = [];

  let startHour, endHour;

  // Use minutes if provided for more accurate handling
  if (startMinutes !== null && endMinutes !== null) {
    startHour = Math.floor(startMinutes / 60);
    endHour = Math.floor(endMinutes / 60);
  } else {
    // Fallback to parsing time strings
    startHour = parseInt(startTime.split(':')[0]);
    endHour = parseInt(endTime.split(':')[0]);

    // Handle overnight shifts (e.g., 22:00 to 06:00)
    if (endHour < startHour) {
      endHour += 24;
    }
  }

  // Generate hourly labels
  for (let hour = startHour; hour <= endHour; hour++) {
    const displayHour = hour % 24;
    labels.push(`${displayHour.toString().padStart(2, '0')}H`);
  }

  return labels;
}

/**
 * 기존 안돈 데이터로 간단한 추이 차트 생성 (대체 로직)
 */
function generateSimpleTrendFromAndonData() {
  if (!charts.andonTrend || !window.andonData || window.andonData.length === 0) {
    return;
  }

  try {
    // 날짜별로 안돈 데이터 그룹화
    const dateGroups = {};

    window.andonData.forEach(item => {
      if (!item.work_date) return;

      const date = item.work_date;
      if (!dateGroups[date]) {
        dateGroups[date] = {
          total: 0,
          warning: 0,
          completed: 0
        };
      }

      dateGroups[date].total++;
      if (item.status === 'Warning') {
        dateGroups[date].warning++;
      } else if (item.status === 'Completed') {
        dateGroups[date].completed++;
      }
    });

    // 정렬된 날짜 목록 생성
    const sortedDates = Object.keys(dateGroups).sort();
    const labels = sortedDates.map(date => {
      const d = new Date(date);
      return (d.getMonth() + 1) + '/' + d.getDate();
    });

    const totalCounts = sortedDates.map(date => dateGroups[date].total);
    const completedCounts = sortedDates.map(date => dateGroups[date].completed);

    // 차트 데이터 업데이트
    charts.andonTrend.data.labels = labels;
    charts.andonTrend.data.datasets[0].data = totalCounts;
    charts.andonTrend.data.datasets[1].data = completedCounts;

    // 데이터셋 라벨 업데이트
    charts.andonTrend.data.datasets[0].label = 'Andon Occurrence';
    charts.andonTrend.data.datasets[1].label = 'Andon Resolution';

    // X축 제목 업데이트
    charts.andonTrend.options.scales.x.title.text = 'Date';

    // 차트 다시 그리기
    charts.andonTrend.update('show');

    console.log(`✅ 기존 데이터로 추이 차트 생성 완료: ${labels.length}개 날짜`);

  } catch (error) {
    console.error('❌ 간단한 추이 차트 생성 오류:', error);
  }
}

/**
 * 실시간 모니터링 정지
 */
function stopTracking() {
  if (!isTracking) {
    console.log('⚠️ 모니터링 중이 아닙니다.');
    return;
  }

  console.log('⏹️ Andon 실시간 모니터링 정지...');

  if (eventSource) {
    eventSource.close();
    eventSource = null;
  }

  // 경과 시간 타이머 정지
  stopElapsedTimeTimer();

  // Duration 실시간 업데이트 타이머 정지
  stopDurationUpdateTimer();

  isTracking = false;

  // 연결 상태 업데이트
  document.getElementById('connectionStatus').textContent = 'Andon 시스템 연결 종료됨';

}

/**
 * 필터 변경 시 실시간 모니터링 재시작
 * 3단 연동 필터(공장-라인-기계) 중 하나라도 변경되면 새로운 필터 조건으로 모니터링 재시작
 */
async function restartRealTimeMonitoring() {
  console.log('🔄 필터 변경으로 인한 실시간 모니터링 재시작...');

  // 페이지를 1로 리셋
  currentPage = 1;

  // 기존 모니터링 중지
  if (isTracking) {
    stopTracking();
    // 잠깐 대기 (연결 정리 시간)
    await new Promise(resolve => setTimeout(resolve, 100));
  }

  // 재연결 시도 횟수 초기화 (필터 변경으로 인한 재시작이므로)
  reconnectAttempts = 0;

  // 새로운 필터 조건으로 모니터링 재시작
  await startAutoTracking();

  console.log('✅ 필터 변경에 따른 실시간 모니터링 재시작 완료');
}

/**
 * SSE 연결 오류 시 자동 재연결
 */
async function attemptReconnection() {
  // 페이지가 언로드 중이면 재연결하지 않음
  if (isPageUnloading) {
    console.log('⚠️ Andon: 페이지 언로드 중 - 재연결 취소');
    return;
  }

  if (reconnectAttempts >= maxReconnectAttempts) {
    console.log('❌ 최대 재연결 시도 횟수에 도달했습니다. 수동 재시작이 필요합니다.');
    return;
  }

  reconnectAttempts++;
  const delayMs = Math.min(1000 * Math.pow(2, reconnectAttempts - 1), 10000); // 지수 백오프 (최대 10초)

  console.log(`🔄 SSE 자동 재연결 시도 ${reconnectAttempts}/${maxReconnectAttempts} (${delayMs}ms 후)`);

  // 기존 연결 정리
  if (eventSource) {
    eventSource.close();
    eventSource = null;
  }

  // 지연 후 재연결 시도
  setTimeout(async () => {
    // 재연결 전에 다시 한번 페이지 상태 확인
    if (isPageUnloading) {
      console.log('⚠️ Andon: 페이지 언로드 중 - 재연결 취소');
      return;
    }

    try {
      await startAutoTracking();
    } catch (error) {
      console.error(`❌ 재연결 시도 ${reconnectAttempts} 실패:`, error);
      // 다음 재연결 시도
      attemptReconnection();
    }
  }, delayMs);
}

/**
 * 경과 시간 실시간 계산 함수 (Jakarta 타임존 기준)
 * @param {string} startTime - 시작 시간 (YYYY-MM-DD HH:mm:ss 형식, Jakarta 타임존)
 * @return {string} - 포맷된 경과 시간 (예: "5m 30s", "1h 15m 30s")
 */
function calculateElapsedTime(startTime) {
  // Jakarta 타임존으로 현재 시간 가져오기
  const nowJakarta = new Date(new Date().toLocaleString('en-US', { timeZone: 'Asia/Jakarta' }));

  // 시작 시간 파싱 (데이터베이스에서 Jakarta 시간으로 저장된 값)
  const start = new Date(startTime.replace(' ', 'T'));

  const diffMs = nowJakarta - start;

  if (diffMs < 0) return '0s';

  const totalSeconds = Math.floor(diffMs / 1000);
  const hours = Math.floor(totalSeconds / 3600);
  const minutes = Math.floor((totalSeconds % 3600) / 60);
  const seconds = totalSeconds % 60;

  let result = '';
  if (hours > 0) result += hours + 'h ';
  if (minutes > 0) result += minutes + 'm ';
  if (seconds > 0 || result === '') result += seconds + 's';

  return result.trim();
}

/**
 * 활성 안돈의 경과 시간을 실시간 업데이트
 */
function updateElapsedTimes() {
  const activeAndonItems = document.querySelectorAll('.active-andon-item');

  activeAndonItems.forEach((item, index) => {
    if (activeAndons[index] && activeAndons[index].reg_date) {
      const elapsedSpan = item.querySelector('.andon-elapsed-time span');
      if (elapsedSpan) {
        const elapsed = calculateElapsedTime(activeAndons[index].reg_date);
        elapsedSpan.textContent = `⏰ ${elapsed}`;
      }
    }
  });
}

/**
 * 경과 시간 타이머 시작
 */
function startElapsedTimeTimer() {
  // 기존 타이머가 있으면 정리
  if (elapsedTimeTimer) {
    clearInterval(elapsedTimeTimer);
  }

  // 1초마다 경과 시간 업데이트
  elapsedTimeTimer = setInterval(updateElapsedTimes, 1000);
}

/**
 * 경과 시간 타이머 정지
 */
function stopElapsedTimeTimer() {
  if (elapsedTimeTimer) {
    clearInterval(elapsedTimeTimer);
    elapsedTimeTimer = null;
  }
}

/**
 * 테이블의 "in progress" Duration 실시간 업데이트
 */
function updateInProgressDurations() {
  const inProgressCells = document.querySelectorAll('.duration-in-progress');

  inProgressCells.forEach(cell => {
    const startTime = cell.getAttribute('data-start-time');
    if (startTime) {
      const elapsed = calculateElapsedTime(startTime);
      cell.textContent = elapsed;
    }
  });
}

/**
 * Duration 실시간 업데이트 타이머 시작
 */
function startDurationUpdateTimer() {
  // 기존 타이머가 있으면 정리
  if (durationUpdateTimer) {
    clearInterval(durationUpdateTimer);
  }

  // 1초마다 in-progress duration 업데이트
  durationUpdateTimer = setInterval(updateInProgressDurations, 1000);
}

/**
 * Duration 실시간 업데이트 타이머 정지
 */
function stopDurationUpdateTimer() {
  if (durationUpdateTimer) {
    clearInterval(durationUpdateTimer);
    durationUpdateTimer = null;
  }
}

/**
 * 날짜 범위 변경 처리
 */
async function handleDateRangeChange(startDate, endDate, label) {
  console.log(`📅 날짜 범위 변경: ${startDate.format('YYYY-MM-DD')} ~ ${endDate.format('YYYY-MM-DD')} (${label})`);

  // 시간 범위 select를 'custom'으로 변경
  const timeRangeSelect = document.getElementById('timeRangeSelect');
  if (timeRangeSelect) {
    let customOption = timeRangeSelect.querySelector('option[value="custom"]');
    if (!customOption) {
      customOption = document.createElement('option');
      customOption.value = 'custom';
      customOption.textContent = '📅 직접 선택';
      timeRangeSelect.appendChild(customOption);
    }
    timeRangeSelect.value = 'custom';
  }

  // 알림 표시

  // 실시간 추적 중이면 연결 재시작
  if (isTracking) {
    stopTracking();
    setTimeout(async () => await startAutoTracking(), 1000);
  } else {
    // 자동 추적 재시작
    await startAutoTracking();
  }
}

/**
 * 시간 범위 변경 처리
 */
async function handleTimeRangeChange(event) {
  const timeRange = event.target.value;
  console.log(`⏰ 시간 범위 변경: ${timeRange}`);

  // 시간 범위가 'custom'이 아닌 경우 DateRangePicker 업데이트
  if (timeRange !== 'custom') {
    let startDate, endDate;

    switch (timeRange) {
      case 'today':
        // 오늘 (00:00 ~ 23:59)
        startDate = moment().startOf('day');
        endDate = moment().endOf('day');
        break;
      case 'yesterday':
        // 어제 (00:00 ~ 23:59)
        startDate = moment().subtract(1, 'days').startOf('day');
        endDate = moment().subtract(1, 'days').endOf('day');
        break;
      case '1w':
        // 최근 일주일 (7일 전 00:00 ~ 오늘 23:59)
        startDate = moment().subtract(7, 'days').startOf('day');
        endDate = moment().endOf('day');
        break;
      case '1m':
        // 최근 한달 (30일 전 00:00 ~ 오늘 23:59)
        startDate = moment().subtract(30, 'days').startOf('day');
        endDate = moment().endOf('day');
        break;
      default:
        // 기본값: 오늘
        startDate = moment().startOf('day');
        endDate = moment().endOf('day');
    }

    // DateRangePicker 값 업데이트
    $('#dateRangePicker').data('daterangepicker').setStartDate(startDate);
    $('#dateRangePicker').data('daterangepicker').setEndDate(endDate);
    $('#dateRangePicker').val(startDate.format('YYYY-MM-DD') + ' ~ ' + endDate.format('YYYY-MM-DD'));
  }

  // 시간 범위 변경에 따른 실시간 모니터링 재시작
  await restartRealTimeMonitoring();
}

/**
 * Export data to Excel with current filters
 */
function exportData() {
  try {
    // Get current filter parameters
    const filters = getFilterParams();

    // Build query parameters for export
    const exportParams = new URLSearchParams();

    // Add filter parameters
    if (filters.factory_filter) {
      exportParams.append('factory_filter', filters.factory_filter);
    }
    if (filters.line_filter) {
      exportParams.append('line_filter', filters.line_filter);
    }
    if (filters.machine_filter) {
      exportParams.append('machine_filter', filters.machine_filter);
    }
    if (filters.shift_filter) {
      exportParams.append('shift_filter', filters.shift_filter);
    }
    if (filters.start_date) {
      exportParams.append('start_date', filters.start_date);
    }
    if (filters.end_date) {
      exportParams.append('end_date', filters.end_date);
    }

    // Construct export URL
    const exportUrl = `proc/data_andon_export.php?${exportParams.toString()}`;

    // Trigger download by redirecting to export URL
    window.location.href = exportUrl;


  } catch (error) {
    console.error('Export error:', error);
  }
}

/**
 * 데이터 새로고침
 */
async function refreshData() {
  console.log('🔄 Andon 데이터 새로고침...');

  // 실시간 추적 중이면 연결 재시작
  if (isTracking) {
    stopTracking();
    setTimeout(async () => await startAutoTracking(), 1000);
  } else {
    // 자동 추적 재시작
    await startAutoTracking();
  }

}

// 토스트 알림 애니메이션 및 스타일은 CSS 파일에서 정의됨

// ============================================================================
// Andon 상세 모달 기능
// ============================================================================

/**
 * Andon 상세 모달 열기 (테이블의 Details 버튼 클릭 시 호출)
 * @param {HTMLElement} buttonElement - 클릭된 Details 버튼 요소
 */
function openAndonDetailModal(buttonElement) {
  try {
    // 버튼에서 Andon 데이터 추출
    const andonDataJson = buttonElement.getAttribute('data-andon-data');
    if (!andonDataJson) {
      console.error('❌ Andon 데이터를 찾을 수 없습니다.');
      return;
    }

    const andonData = JSON.parse(andonDataJson);
    console.log('🔍 Andon 상세 모달 열기:', andonData);

    // 모달에 데이터 표시
    populateAndonModal(andonData);

    // 모달 표시
    const modal = document.getElementById('andonDetailModal');
    if (modal) {
      modal.classList.add('show');
      // body 스크롤 방지
      document.body.style.overflow = 'hidden';
    } else {
      console.error('❌ 모달 요소를 찾을 수 없습니다.');
    }

  } catch (error) {
    console.error('❌ 모달 열기 오류:', error);
  }
}

/**
 * Andon 상세 모달 닫기
 */
function closeAndonDetailModal() {
  console.log('❌ Andon 상세 모달 닫기');

  const modal = document.getElementById('andonDetailModal');
  if (modal) {
    modal.classList.add('hide');

    // 애니메이션 완료 후 모달 숨기기
    setTimeout(() => {
      modal.classList.remove('show', 'hide');
      // body 스크롤 복원
      document.body.style.overflow = '';
    }, 300);
  }
}

/**
 * 모달에 Andon 데이터 채우기
 * @param {Object} andonData - 표시할 Andon 데이터
 */
function populateAndonModal(andonData) {
  console.log('📝 모달 데이터 채우기:', andonData);

  try {
    // 기본 정보
    setModalValue('modal-machine-no', andonData.machine_no || '-');
    setModalValue('modal-factory-line', `${andonData.factory_name || '-'} / ${andonData.line_name || '-'}`);
    setModalValue('modal-andon-type', andonData.andon_name || '-');

    // 상태 표시 (배지 형태로)
    const statusElement = document.getElementById('modal-status');
    if (statusElement) {
      if (andonData.status === 'Warning') {
        statusElement.innerHTML = '<span class="fiori-badge fiori-badge--error">⚠️ Warning</span>';
      } else if (andonData.status === 'Completed') {
        statusElement.innerHTML = '<span class="fiori-badge fiori-badge--success">✅ Completed</span>';
      } else {
        statusElement.textContent = andonData.status || '-';
      }
    }

    // 시간 정보
    setModalValue('modal-reg-date', andonData.reg_date || '-');
    setModalValue('modal-update-date', andonData.update_date || '-');
    setModalValue('modal-duration', andonData.duration_display || andonData.duration_his || '-');
    setModalValue('modal-work-date', andonData.work_date || '-');

    // 작업 정보
    const shiftDisplay = andonData.shift_idx ? `Shift ${andonData.shift_idx}` : '-';
    setModalValue('modal-shift', shiftDisplay);

    // Andon 색상 표시
    const colorIndicator = document.getElementById('modal-color-indicator');
    const colorValue = document.getElementById('modal-color-value');
    if (colorIndicator && colorValue) {
      if (andonData.andon_color) {
        colorIndicator.style.backgroundColor = andonData.andon_color;
        colorValue.textContent = andonData.andon_color;
      } else {
        colorIndicator.style.backgroundColor = '#cccccc';
        colorValue.textContent = '기본 색상';
      }
    }

    // 추가 정보
    setModalValue('modal-idx', andonData.idx || '-');
    setModalValue('modal-created-at', andonData.created_at || andonData.reg_date || '-');

    console.log('✅ 모달 데이터 채우기 완료');

  } catch (error) {
    console.error('❌ 모달 데이터 채우기 오류:', error);
  }
}

/**
 * 모달 요소에 값 설정하는 도우미 함수
 * @param {string} elementId - 요소 ID
 * @param {string} value - 설정할 값
 */
function setModalValue(elementId, value) {
  const element = document.getElementById(elementId);
  if (element) {
    element.textContent = value;
  } else {
    console.warn(`⚠️ 모달 요소를 찾을 수 없습니다: ${elementId}`);
  }
}

/**
 * 단일 Andon 데이터 내보내기 (모달에서 호출)
 */
function exportSingleAndon() {
  console.log('📊 단일 Andon 데이터 내보내기...');

  // TODO: 실제 단일 Andon 데이터 내보내기 구현

  // 모달 닫기
  closeAndonDetailModal();
}

/**
 * ESC 키로 모달 닫기 이벤트 리스너
 */
document.addEventListener('keydown', function (event) {
  if (event.key === 'Escape') {
    const modal = document.getElementById('andonDetailModal');
    if (modal && modal.classList.contains('show')) {
      closeAndonDetailModal();
    }
  }
});

/**
 * Toggle stats display
 */
function toggleStatsDisplay() {
  const statsGrid = document.getElementById('statsGrid');
  const toggleBtn = document.getElementById('toggleStatsBtn');

  if (!statsGrid || !toggleBtn) {
    console.error('Stats grid or toggle button not found.');
    return;
  }

  if (statsGrid.classList.contains('hidden')) {
    // Show stats
    statsGrid.classList.remove('hidden');
    toggleBtn.textContent = '📊 Hide Stats';
  } else {
    // Hide stats
    statsGrid.classList.add('hidden');
    toggleBtn.textContent = '📊 Show Stats';
  }
}

/**
 * Toggle charts display
 */
function toggleChartsDisplay() {
  const andonMainLayout = document.querySelector('.andon-main-layout');
  const andonTrendSection = document.getElementById('andonTrendSection');
  const toggleBtn = document.getElementById('toggleChartsBtn');

  if (!toggleBtn) {
    console.error('Toggle charts button not found.');
    return;
  }

  if (!andonMainLayout && !andonTrendSection) {
    console.error('Chart sections not found.');
    return;
  }

  // 두 섹션 중 하나라도 보이는 상태라면 "보이는 상태"로 간주
  const isHidden = (andonMainLayout && andonMainLayout.classList.contains('hidden')) ||
    (andonTrendSection && andonTrendSection.classList.contains('hidden'));

  if (isHidden) {
    // Show charts
    if (andonMainLayout) {
      andonMainLayout.classList.remove('hidden');
    }
    if (andonTrendSection) {
      andonTrendSection.classList.remove('hidden');
    }
    toggleBtn.textContent = '📈 Hide Charts';
  } else {
    // Hide charts
    if (andonMainLayout) {
      andonMainLayout.classList.add('hidden');
    }
    if (andonTrendSection) {
      andonTrendSection.classList.add('hidden');
    }
    toggleBtn.textContent = '📈 Show Charts';
  }
}

/**
 * 전역 함수로 등록 (HTML onclick에서 호출 가능하도록)
 */
window.openAndonDetailModal = openAndonDetailModal;
window.closeAndonDetailModal = closeAndonDetailModal;
window.exportSingleAndon = exportSingleAndon;