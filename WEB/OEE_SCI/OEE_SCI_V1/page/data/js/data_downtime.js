// ============================================================================
// Downtime Data Tracking JavaScript
// ============================================================================

/**
 * Global variables and settings
 */
let eventSource = null;
let isTracking = false;
let charts = {};
let downtimeData = [];
let reconnectAttempts = 0;
let maxReconnectAttempts = 3;
let stats = {};
let activeDowntimes = [];
let elapsedTimeTimer = null;
let durationUpdateTimer = null;
let isPageUnloading = false; // 페이지 언로드 상태 추적

// Pagination variables
let currentPage = 1;
let itemsPerPage = 10;
let totalItems = 0;

// Chart.js 기본 설정
Chart.defaults.color = '#1a1a1a';
Chart.defaults.borderColor = '#e8eaed';

/**
 * Chart color palette for Downtime
 */
const chartColors = {
  warning: '#e26b0a',
  error: '#da1e28',
  success: '#30914c',
  info: '#7c3aed',
  primary: '#0070f2',
  secondary: '#1e88e5',
  accent: '#00d4aa'
};

/**
 * Page initialization
 */
document.addEventListener('DOMContentLoaded', async function () {
  console.log('Downtime monitoring system initialization started');

  try {
    initDateRangePicker();
    await initFilterSystem();
    initCharts();
    setupEventListeners();
    await loadInitialData();
    await startAutoTracking();

    console.log('Downtime monitoring system initialized');
  } catch (error) {
    console.error('Initialization error:', error);
  }
});

// 페이지 언로드 시 SSE 연결 및 타이머 종료 (브라우저 동시 연결 수 제한 문제 해결)
// 우선순위: beforeunload > pagehide > visibilitychange

// 1. beforeunload - 가장 먼저 실행되어 페이지 언로드 플래그 설정
window.addEventListener('beforeunload', () => {
  console.log('Downtime: beforeunload - 페이지 언로드 시작');
  isPageUnloading = true; // 플래그 설정으로 재연결 방지

  if (eventSource) {
    eventSource.close();
    eventSource = null;
    isTracking = false;
  }
  stopElapsedTimeTimer();
  stopDurationUpdateTimer();
});

// 2. pagehide - 브라우저가 페이지를 숨길 때 (모바일에서 중요)
window.addEventListener('pagehide', () => {
  console.log('Downtime: pagehide - 페이지 숨김');
  isPageUnloading = true;

  if (eventSource) {
    eventSource.close();
    eventSource = null;
    isTracking = false;
  }
  stopElapsedTimeTimer();
  stopDurationUpdateTimer();
});

// 3. visibilitychange - 탭 전환이나 페이지 숨김
document.addEventListener('visibilitychange', () => {
  if (document.hidden) {
    // 페이지가 숨겨질 때 SSE 연결과 타이머를 즉시 정리
    console.log('Downtime: 페이지 숨김 - SSE 연결 및 타이머 종료');

    if (eventSource && isTracking) {
      eventSource.close();
      eventSource = null;
      isTracking = false;
    }
    stopElapsedTimeTimer();
    stopDurationUpdateTimer();
  } else {
    // 페이지가 다시 보일 때만 재연결 (언로드 중이 아닐 때만)
    console.log('Downtime: 페이지 표시 - SSE 재연결 시도');

    if (!isPageUnloading && !isTracking && eventSource === null) {
      reconnectAttempts = 0;
      startAutoTracking();
    }
  }
});

// 4. 페이지 포커스 복원 시 - 언로드 플래그 리셋
window.addEventListener('focus', () => {
  if (isPageUnloading) {
    console.log('Downtime: 페이지 포커스 - 언로드 플래그 리셋');
    isPageUnloading = false;
  }
});

/**
 * Initialize DateRangePicker
 */
function initDateRangePicker() {

  try {
    // Initialize DateRangePicker with jQuery
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
      // Handle date range change
      handleDateRangeChange(start, end, label);
    });

    // Set initial values for today
    const initialStart = moment().startOf('day');
    const initialEnd = moment().endOf('day');
    $('#dateRangePicker').val(initialStart.format('YYYY-MM-DD') + ' ~ ' + initialEnd.format('YYYY-MM-DD'));

  } catch (error) {
    console.error('DateRangePicker initialization error:', error);
  }
}

/**
 * Initialize 3-level filter system
 */
async function initFilterSystem() {

  try {
    await loadFactoryOptions();
    await loadLineOptions();
    await loadMachineOptions();

  } catch (error) {
    console.error('Filter system initialization error:', error);
  }
}

/**
 * Load factory options
 */
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

        // Update line list when factory is selected
        factorySelect.addEventListener('change', async (e) => {
          // Reset machine filter
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
    console.error('Factory options load error:', error);
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
    console.log(`🔄 Line 옵션 업데이트 중... Factory ID: ${factoryId}`);

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
      console.log(`✅ Line 옵션 업데이트 완료 - Factory: ${factoryId}, Lines: ${res.data.length}개`);
    } else {
      console.error(`❌ Line 옵션 업데이트 실패 - Factory: ${factoryId}`, res.message || 'Unknown error');
    }

    lineSelect.disabled = false;

    // Machine 필터도 다시 활성화
    const machineSelect = document.getElementById('factoryLineMachineFilterSelect');
    if (factoryId) {
      machineSelect.disabled = false;
      updateMachineOptions(factoryId, ''); // 전체 기계 로드
    }
  } catch (error) {
    console.error(`❌ Line 옵션 업데이트 실패 - Factory: ${factoryId}`, error);
    lineSelect.innerHTML = '<option value="">All Line</option>';
    lineSelect.disabled = false;
  }
}

/**
 * Machine 옵션 업데이트 (Factory, Line 선택에 따라)
 */
async function updateMachineOptions(factoryId, lineId) {
  const machineSelect = document.getElementById('factoryLineMachineFilterSelect');
  machineSelect.disabled = true;

  try {
    console.log(`🔄 Machine 옵션 업데이트 중... Factory: ${factoryId}, Line: ${lineId}`);

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
      console.log(`✅ 머신 목록 로드 완료 - Factory: ${factoryId}, Line: ${lineId}, Count: ${res.data.length}`);
    } else {
      console.error(`❌ Machine 옵션 업데이트 실패 - Factory: ${factoryId}, Line: ${lineId}`, res.message || 'Unknown error');
    }

    machineSelect.disabled = false;
  } catch (error) {
    console.error(`❌ Machine 옵션 업데이트 실패 - Factory: ${factoryId}, Line: ${lineId}`, error);
    machineSelect.innerHTML = '<option value="">All Machine</option>';
    machineSelect.disabled = false;
  }
}

/**
 * 차트 초기화 (Downtime 전용)
 */
function initCharts() {
  console.log('📊 Downtime 차트 초기화 중...');

  try {
    // 1. 다운타임 발생 추이 차트
    if (document.getElementById('downtimeTrendChart')) {
      charts.downtimeTrend = createDowntimeTrendChart();
    } else {
      console.warn('⚠️ downtimeTrendChart 요소를 찾을 수 없습니다.');
    }

    // 2. 다운타임 유형별 분석 차트
    if (document.getElementById('downtimeTypeChart')) {
      charts.downtimeType = createDowntimeTypeChart();
    } else {
      console.warn('⚠️ downtimeTypeChart 요소를 찾을 수 없습니다.');
    }

    // 3. 다운타임 지속시간 분포 차트 (제거됨 - HTML에 canvas가 없음)
    // charts.downtimeDuration = createDowntimeDurationChart();

    console.log('✅ 활성 Downtime 차트 초기화 완료 (추이, 유형별)');
  } catch (error) {
    console.error('❌ 차트 초기화 오류:', error);
  }
}

/**
 * 다운타임 발생 추이 차트 생성
 */
function createDowntimeTrendChart() {
  const ctx = document.getElementById('downtimeTrendChart').getContext('2d');
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
            text: 'Downtime Quantity'
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
            text: 'Time/Date'
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
 * 다운타임 유형별 분석 차트 생성
 */
function createDowntimeTypeChart() {
  const ctx = document.getElementById('downtimeTypeChart').getContext('2d');
  return new Chart(ctx, {
    type: 'bar',
    data: {
      labels: [], // 실제 데이터로 업데이트
      datasets: [{
        label: 'Total downtime duration (minutes)',
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
        },
        tooltip: {
          callbacks: {
            label: function(context) {
              const label = context.dataset.label || '';
              const value = context.parsed.y || 0;
              return `${label}: ${value.toFixed(1)} minutes`;
            }
          }
        }
      },
      scales: {
        y: {
          beginAtZero: true,
          title: {
            display: true,
            text: 'Duration (minutes)'
          },
          ticks: {
            // 분 단위이므로 소수점 1자리까지 표시
            callback: function(value) {
              return value.toFixed(1) + ' min';
            }
          }
        },
        x: {
          title: {
            display: true,
            text: 'Downtime type'
          }
        }
      }
    }
  });
}

/**
 * 다운타임 지속시간 분포 차트 생성
 */
function createDowntimeDurationChart() {
  const ctx = document.getElementById('downtimeDurationChart').getContext('2d');
  return new Chart(ctx, {
    type: 'pie',
    data: {
      labels: ['< 5min', '5-15min', '15-30min', '30-60min', '> 1hour'],
      datasets: [{
        data: [], // 실제 데이터로 업데이트
        backgroundColor: [
          chartColors.success,
          chartColors.info,
          chartColors.warning,
          chartColors.error,
          '#8B0000' // 매우 긴 다운타임용 다크레드
        ]
      }]
    },
    options: {
      responsive: true,
      maintainAspectRatio: false,
      plugins: {
        legend: {
          position: 'bottom'
        },
        tooltip: {
          callbacks: {
            label: function (context) {
              const label = context.label || '';
              const value = context.parsed || 0;
              const total = context.dataset.data.reduce((a, b) => a + b, 0);
              const percentage = total > 0 ? Math.round((value / total) * 100) : 0;
              return `${label}: ${value} cases (${percentage}%)`;
            }
          }
        }
      }
    }
  });
}

/**
 * 이벤트 리스너 설정
 */
function setupEventListeners() {
  console.log('🎧 이벤트 리스너 설정 중...');

  // 데이터 내보내기 버튼
  const excelDownloadBtn = document.getElementById('excelDownloadBtn');
  if (excelDownloadBtn) {
    excelDownloadBtn.addEventListener('click', exportData);
    console.log('✅ excelDownloadBtn 이벤트 리스너 추가됨');
  } else {
    console.log('⚠️ excelDownloadBtn 요소를 찾을 수 없습니다.');
  }

  // 새로고침 버튼
  const refreshBtn = document.getElementById('refreshBtn');
  if (refreshBtn) {
    refreshBtn.addEventListener('click', refreshData);
    console.log('✅ refreshBtn 이벤트 리스너 추가됨');
  } else {
    console.log('⚠️ refreshBtn 요소를 찾을 수 없습니다.');
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

  // 시간 범위 필터 변경
  const timeRangeSelect = document.getElementById('timeRangeSelect');
  if (timeRangeSelect) {
    timeRangeSelect.addEventListener('change', handleTimeRangeChange);
    console.log('✅ timeRangeSelect 이벤트 리스너 추가됨');
  } else {
    console.log('⚠️ timeRangeSelect 요소를 찾을 수 없습니다.');
  }

  // 기계 필터 변경 (실시간 모니터링 재시작)
  const factoryLineMachineFilterSelect = document.getElementById('factoryLineMachineFilterSelect');
  if (factoryLineMachineFilterSelect) {
    factoryLineMachineFilterSelect.addEventListener('change', async (e) => {
      console.log(`🔧 Machine 필터 변경됨: ${e.target.value}`);
      // 필터 변경에 따른 실시간 모니터링 재시작
      await restartRealTimeMonitoring();
    });
    console.log('✅ factoryLineMachineFilterSelect 이벤트 리스너 추가됨');
  } else {
    console.log('⚠️ factoryLineMachineFilterSelect 요소를 찾을 수 없습니다.');
  }

  // 교대(Shift) 필터 변경 (실시간 모니터링 재시작)
  const shiftSelect = document.getElementById('shiftSelect');
  if (shiftSelect) {
    shiftSelect.addEventListener('change', async (e) => {
      console.log(`⏰ Shift 필터 변경됨: ${e.target.value}`);
      // 필터 변경에 따른 실시간 모니터링 재시작
      await restartRealTimeMonitoring();
    });
    console.log('✅ shiftSelect 이벤트 리스너 추가됨');
  } else {
    console.log('⚠️ shiftSelect 요소를 찾을 수 없습니다.');
  }

  console.log('✅ 모든 이벤트 리스너 설정 완료');
}

/**
 * 초기 데이터 로딩 (실제 API 연동 없이 빈 상태로 시작)
 */
async function loadInitialData() {
  console.log('📋 초기 Downtime 데이터 준비 중...');

  try {
    // 초기에는 연결 전 상태로 빈 데이터 표시
    displayEmptyState();

    console.log('✅ 초기 Downtime 데이터 준비 완료');
  } catch (error) {
    console.error('❌ 초기 데이터 준비 오류:', error);
  }
}

/**
 * 빈 상태 표시 (데이터 로딩 전)
 */
function displayEmptyState() {
  // Stat Cards를 빈 상태로 초기화 (null 체크 포함) - 6개 항목
  const statElements = [
    'totalDowntime', 'activeDowntimes', 'currentShiftDowntime',
    'affectedMachinesDowntime', 'longDowntimes', 'avgDowntimeResolution'
  ];

  statElements.forEach(elementId => {
    const element = document.getElementById(elementId);
    if (element) {
      element.textContent = '-';
    } else {
      console.warn(`⚠️ 초기화 시 요소를 찾을 수 없습니다: ${elementId}`);
    }
  });

  // 활성 다운타임 업데이트
  const activeCountEl = document.getElementById('activeDowntimeCount');
  if (activeCountEl) {
    activeCountEl.textContent = '0 active downtimes';
  } else {
    console.warn('⚠️ activeDowntimeCount 요소를 찾을 수 없습니다.');
  }

  // 테이블 초기 메시지 유지
  const tbody = document.getElementById('downtimeDataBody');
  tbody.innerHTML = `
    <tr>
      <td colspan="10" style="text-align: center; padding: var(--sap-spacing-xl);">
        <div class="fiori-alert fiori-alert--info">
          <strong>ℹ️ Information:</strong> Loading real-time Downtime data. Automatic monitoring is in progress.
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

  console.log('🔄 Downtime 자동 실시간 모니터링 시작...');

  try {
    // 새로운 연결 시도이므로 재연결 횟수는 건드리지 않음 (자동 재연결과 구분)
    // 필터 파라미터 구성
    const filters = getFilterParams();
    const params = new URLSearchParams(filters);

    const sseUrl = `proc/data_downtime_stream.php?${params.toString()}`;

    // 실제 SSE 연결
    eventSource = new EventSource(sseUrl);

    // SSE 이벤트 리스너 설정
    setupSSEEventListeners();

    isTracking = true;

    // 연결 상태 업데이트
    const connectionStatusEl = document.getElementById('connectionStatus');
    if (connectionStatusEl) {
      connectionStatusEl.textContent = 'Downtime 시스템 실시간 연결됨';
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
      connectionStatusEl.textContent = 'Downtime system connected';
      connectionStatusEl.className = 'connection-status--success';
    }

    // 재연결 시도 횟수 초기화 (성공적으로 연결됨)
    reconnectAttempts = 0;
  });

  // 메인 다운타임 데이터
  eventSource.addEventListener('downtime_data', function (event) {
    const data = JSON.parse(event.data);

    // 데이터 업데이트 (전역 변수로 저장)
    stats = data.stats;
    activeDowntimes = data.active_downtimes;
    downtimeData = data.downtime_data;
    window.downtimeData = data.downtime_data; // 전역 접근 가능하도록

    // UI 업데이트
    updateStatCardsFromAPI(stats);
    updateActiveDowntimesDisplay(activeDowntimes);
    updateTableFromAPI(downtimeData);
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
      connectionStatusEl.textContent = 'Downtime 시스템 연결 종료됨';
    }
  });
}

/**
 * API에서 받은 통계 데이터로 Stat Cards 업데이트
 */
function updateStatCardsFromAPI(statsData) {
  if (!statsData) return;

  // DOM 요소들이 존재하는지 확인 후 업데이트 - 6개 항목
  const elements = {
    'totalDowntime': statsData.total_count || '0',
    'activeDowntimes': statsData.warning_count || '0',
    'currentShiftDowntime': statsData.current_shift_count || '0',
    'affectedMachinesDowntime': statsData.affected_machines || '0',
    'longDowntimes': statsData.long_downtimes_count || '0', // 30분 이상 다운타임
    'avgDowntimeResolution': statsData.avg_completed_time || '-'
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
 * 활성 다운타임 표시 업데이트
 */
function updateActiveDowntimesDisplay(activeDowntimesList) {
  const container = document.getElementById('activeDowntimesContainer');
  const countDisplay = document.getElementById('activeDowntimeCount');

  if (countDisplay) {
    countDisplay.textContent = `${activeDowntimesList.length} active downtimes`;
  } else {
    console.warn('⚠️ activeDowntimeCount 요소를 찾을 수 없습니다.');
  }

  if (!container) {
    console.warn('⚠️ activeDowntimesContainer 요소를 찾을 수 없습니다.');
    return;
  }

  if (activeDowntimesList.length === 0) {
    container.innerHTML = `
      <div class="fiori-alert fiori-alert--success">
        <strong>✅ Good:</strong> There are currently no active Downtimes. All systems are operating normally.
      </div>
    `;
    // 활성 다운타임이 없으면 타이머 정지
    stopElapsedTimeTimer();
  } else {
    let html = '';
    activeDowntimesList.forEach(downtime => {
      // 초기 경과 시간 계산 (실시간 업데이트는 타이머가 처리)
      const initialElapsed = calculateElapsedTime(downtime.reg_date);

      html += `
        <div class="active-downtime-item">
          <div class="downtime-machine-info">
            <div class="downtime-machine-name">${downtime.downtime_name}</div>
            <div class="downtime-location">
              <span class="downtime-location-item">${downtime.factory_name}</span>
              <span class="downtime-location-item">/</span>
              <span class="downtime-location-item">${downtime.line_name}</span>
              <span class="downtime-location-item">/</span>
              <span class="downtime-location-item">${downtime.machine_no}</span>
            </div>
          </div>
          <div class="downtime-elapsed-time">
            <span>⏰ ${initialElapsed}</span>
          </div>
        </div>
      `;
    });
    container.innerHTML = html;

    // 활성 다운타임이 있으면 실시간 타이머 시작
    startElapsedTimeTimer();
  }
}

/**
 * API 데이터로 테이블 업데이트
 */
function updateTableFromAPI(downtimeDataList) {
  const tbody = document.getElementById('downtimeDataBody');

  if (!tbody) {
    console.error('❌ downtimeDataBody 요소를 찾을 수 없습니다.');
    return;
  }

  // 전체 데이터 수 업데이트
  totalItems = downtimeDataList.length;

  if (downtimeDataList.length === 0) {
    tbody.innerHTML = `
      <tr>
        <td colspan="10" style="text-align: center; padding: var(--sap-spacing-xl);">
          <div class="fiori-alert fiori-alert--info">
            <strong>ℹ️ Information:</strong> No Downtime data matching the selected conditions.
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
  const paginatedData = downtimeDataList.slice(startIndex, endIndex);

  tbody.innerHTML = '';
  paginatedData.forEach(downtime => {
    const row = document.createElement('tr');

    let statusBadge = '';
    if (downtime.status === 'Warning') {
      statusBadge = '<span class="fiori-badge fiori-badge--error">⚠️ Warning</span>';
    } else {
      statusBadge = '<span class="fiori-badge fiori-badge--success">✅ Completed</span>';
    }

    // 교대 표시 (shift_idx 값에 따라)
    let shiftDisplay = '-';
    if (downtime.shift_idx) {
      shiftDisplay = `Shift ${downtime.shift_idx}`;
    }

    // Duration 셀 처리 - Warning 상태일 경우 실시간 업데이트 적용
    let durationCell = '';
    if (downtime.status === 'Warning' && downtime.reg_date) {
      // Warning 상태 - 실시간 Duration 계산 및 빨간색 표시
      const initialDuration = calculateElapsedTime(downtime.reg_date);
      durationCell = `<td class="duration-in-progress" data-start-time="${downtime.reg_date}">${initialDuration}</td>`;
    } else {
      // Completed 상태 - 고정된 Duration 표시
      durationCell = `<td>${downtime.duration_display || downtime.duration_his || '-'}</td>`;
    }

    row.innerHTML = `
      <td>${downtime.machine_no || '-'}</td>
      <td>${(downtime.factory_name || '') + ' / ' + (downtime.line_name || '')}</td>
      <td>${shiftDisplay}</td>
      <td>${downtime.downtime_name || '-'}</td>
      <td>${statusBadge}</td>
      <td>${downtime.reg_date || '-'}</td>
      <td>${downtime.update_date || '-'}</td>
      ${durationCell}
      <td>${downtime.work_date || '-'}</td>
      <td>
        <button class="fiori-btn fiori-btn--tertiary downtime-details-btn"
                style="padding: 0.25rem 0.5rem; font-size: 0.75rem;"
                data-downtime-data='${JSON.stringify(downtime).replace(/'/g, "&#39;")}'>
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

  // 현재 다운타임 데이터로 테이블 다시 렌더링
  updateTableFromAPI(downtimeData);

  console.log(`📄 페이지 변경: ${currentPage}/${totalPages}`);
}

/**
 * 차트 데이터 업데이트
 */
function updateChartsFromAPI(apiData) {
  try {
    // 1. 다운타임 유형별 차트 업데이트
    if (apiData.downtime_type_stats && charts.downtimeType) {
      updateDowntimeTypeChart(apiData.downtime_type_stats);
    }

    // 2. 다운타임 발생 추이 차트 업데이트
    if (apiData.downtime_trend_stats && charts.downtimeTrend) {
      updateDowntimeTrendChart(apiData.downtime_trend_stats);
    } else {
      console.warn('⚠️ 다운타임 추이 차트 업데이트 건너뜀:', {
        hasDowntimeTrendStats: !!apiData.downtime_trend_stats,
        hasChart: !!charts.downtimeTrend
      });
    }

    // 3. 다운타임 지속시간 분포 차트 (제거됨 - HTML에 canvas가 없음)
    // if (apiData.downtime_duration_stats && charts.downtimeDuration) {
    //   updateDowntimeDurationChart(apiData.downtime_duration_stats);
    // }

  } catch (error) {
    console.error('❌ 차트 업데이트 중 오류:', error);
  }
}

/**
 * 다운타임 유형별 차트 데이터 업데이트
 */
function updateDowntimeTypeChart(downtimeTypeStats) {
  if (!charts.downtimeType) {
    console.error('❌ charts.downtimeType이 초기화되지 않았습니다.');
    return;
  }

  if (!downtimeTypeStats || downtimeTypeStats.length === 0) {
    console.warn('⚠️ downtimeTypeStats 데이터가 없거나 비어있습니다.');
    return;
  }

  try {
    // downtime_name 기준으로 알파벳순 정렬
    const sortedStats = [...downtimeTypeStats].sort((a, b) => {
      const nameA = (a.downtime_name || 'Unclassified').toLowerCase();
      const nameB = (b.downtime_name || 'Unclassified').toLowerCase();
      return nameA.localeCompare(nameB);
    });

    // 다운타임 이름과 지속시간(분) 추출
    const labels = sortedStats.map(item => item.downtime_name || 'Unclassified');
    const durations = sortedStats.map(item => parseFloat(item.total_duration_min) || 0);

    console.log('🔍 차트 데이터 추출 (downtime_name 기준 정렬):', {
      labels: labels,
      durations_min: durations
    });

    // 색상 배열 순환하여 사용
    const backgroundColors = [];
    const borderColors = [];
    const colorKeys = ['warning', 'error', 'primary', 'info', 'accent', 'secondary'];

    for (let i = 0; i < labels.length; i++) {
      const colorKey = colorKeys[i % colorKeys.length];
      backgroundColors.push(chartColors[colorKey]);
      borderColors.push(chartColors[colorKey]);
    }

    // 차트 데이터 업데이트
    charts.downtimeType.data.labels = labels;
    charts.downtimeType.data.datasets[0].data = durations; // 분 단위 지속시간
    charts.downtimeType.data.datasets[0].backgroundColor = backgroundColors;
    charts.downtimeType.data.datasets[0].borderColor = borderColors;

    // 차트 다시 그리기
    charts.downtimeType.update('none'); // 애니메이션 없이 업데이트

    console.log(`✅ 다운타임 유형별 차트 업데이트 완료: ${labels.length}개 유형`);

  } catch (error) {
    console.error('❌ 다운타임 유형별 차트 업데이트 오류:', error);
  }
}

/**
 * 다운타임 발생 추이 차트 데이터 업데이트
 */
function updateDowntimeTrendChart(downtimeTrendStats) {
  console.log('🔍 다운타임 추이 차트 업데이트 시작:', downtimeTrendStats);

  if (!charts.downtimeTrend) {
    console.error('❌ charts.downtimeTrend가 초기화되지 않았습니다.');
    return;
  }

  if (!downtimeTrendStats) {
    console.warn('⚠️ downtimeTrendStats가 없습니다.');
    charts.downtimeTrend.data.labels = [];
    charts.downtimeTrend.data.datasets[0].data = [];
    charts.downtimeTrend.data.datasets[1].data = [];
    charts.downtimeTrend.update('none');
    return;
  }

  if (!downtimeTrendStats.data || downtimeTrendStats.data.length === 0) {
    console.warn('⚠️ 다운타임 추이 데이터가 비어있습니다.');

    // 대체 로직: downtimeData에서 간단한 추이 생성
    if (window.downtimeData && window.downtimeData.length > 0) {
      console.log('🔄 기존 Downtime 데이터로 간단한 추이 생성:', window.downtimeData.length + ' items');
      generateSimpleTrendFromDowntimeData();
      return;
    }

    // 완전히 비어있는 경우
    charts.downtimeTrend.data.labels = ['No Data'];
    charts.downtimeTrend.data.datasets[0].data = [0];
    charts.downtimeTrend.data.datasets[1].data = [0];
    charts.downtimeTrend.update('none');
    return;
  }

  try {
    // Handle new API response structure with work_hours
    let trendData = downtimeTrendStats.data || downtimeTrendStats;
    let workHours = null;
    let viewType = 'hourly';

    // Check if response has new structure (data + work_hours)
    if (downtimeTrendStats && typeof downtimeTrendStats === 'object' && downtimeTrendStats.data) {
      trendData = downtimeTrendStats.data;
      workHours = downtimeTrendStats.work_hours;
      viewType = downtimeTrendStats.view_type || 'hourly';
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
    charts.downtimeTrend.data.labels = labels;
    charts.downtimeTrend.data.datasets[0].data = warningCounts; // Warning 상태만
    charts.downtimeTrend.data.datasets[1].data = completedCounts; // Completed 상태만

    // 데이터셋 라벨 및 스타일 업데이트
    charts.downtimeTrend.data.datasets[0].label = '⚠️ Warning';
    charts.downtimeTrend.data.datasets[0].borderColor = chartColors.error;
    charts.downtimeTrend.data.datasets[0].backgroundColor = chartColors.error + '20';

    charts.downtimeTrend.data.datasets[1].label = '✅ Completed';
    charts.downtimeTrend.data.datasets[1].borderColor = chartColors.success;
    charts.downtimeTrend.data.datasets[1].backgroundColor = chartColors.success + '20';

    // X축 제목 업데이트
    charts.downtimeTrend.options.scales.x.title.text = xAxisTitle;

    // 차트 다시 그리기 (부드러운 애니메이션과 함께)
    charts.downtimeTrend.update('show');

    console.log(`✅ 다운타임 발생 추이 차트 업데이트 완료: ${labels.length}개 포인트 (${viewType} view)`);

  } catch (error) {
    console.error('❌ 다운타임 발생 추이 차트 업데이트 오류:', error);
  }
}

/**
 * 다운타임 지속시간 분포 차트 데이터 업데이트
 */
function updateDowntimeDurationChart(downtimeDurationStats) {
  if (!charts.downtimeDuration) {
    console.error('❌ charts.downtimeDuration이 초기화되지 않았습니다.');
    return;
  }

  if (!downtimeDurationStats || downtimeDurationStats.length === 0) {
    console.warn('⚠️ downtimeDurationStats 데이터가 없거나 비어있습니다.');
    return;
  }

  try {
    // 지속시간 범위별 데이터 추출
    const durations = [
      downtimeDurationStats.under_5min || 0,
      downtimeDurationStats.min_5_15 || 0,
      downtimeDurationStats.min_15_30 || 0,
      downtimeDurationStats.min_30_60 || 0,
      downtimeDurationStats.over_1hour || 0
    ];

    // 차트 데이터 업데이트
    charts.downtimeDuration.data.datasets[0].data = durations;

    // 차트 다시 그리기
    charts.downtimeDuration.update('none');

    console.log(`✅ 다운타임 지속시간 분포 차트 업데이트 완료:`, durations);

  } catch (error) {
    console.error('❌ 다운타임 지속시간 분포 차트 업데이트 오류:', error);
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
 * 기존 다운타임 데이터로 간단한 추이 차트 생성 (대체 로직)
 */
function generateSimpleTrendFromDowntimeData() {
  if (!charts.downtimeTrend || !window.downtimeData || window.downtimeData.length === 0) {
    return;
  }

  try {
    // 날짜별로 다운타임 데이터 그룹화
    const dateGroups = {};

    window.downtimeData.forEach(item => {
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
    charts.downtimeTrend.data.labels = labels;
    charts.downtimeTrend.data.datasets[0].data = totalCounts;
    charts.downtimeTrend.data.datasets[1].data = completedCounts;

    // 데이터셋 라벨 업데이트
    charts.downtimeTrend.data.datasets[0].label = 'Downtime Occurrence';
    charts.downtimeTrend.data.datasets[1].label = 'Downtime Resolution';

    // X축 제목 업데이트
    charts.downtimeTrend.options.scales.x.title.text = 'Date';

    // 차트 다시 그리기
    charts.downtimeTrend.update('show');

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

  console.log('⏹️ Downtime 실시간 모니터링 정지...');

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
  document.getElementById('connectionStatus').textContent = 'Downtime 시스템 연결 종료됨';

  console.log('ℹ️ Downtime 실시간 모니터링이 정지되었습니다.');
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
    console.log('⚠️ 페이지 언로드 중 - 재연결 취소');
    return;
  }

  if (reconnectAttempts >= maxReconnectAttempts) {
    console.log('❌ 최대 재연결 시도 횟수에 도달했습니다. 수동 재시작이 필요합니다.');
    console.error(`❌ SSE 연결에 계속 실패했습니다 (${maxReconnectAttempts}회 시도). 네트워크 상태를 확인하거나 페이지를 새로고침해주세요.`);
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
      console.log('⚠️ 페이지 언로드 중 - 재연결 취소');
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
  // 한국 시간 기준으로 계산함.
  /* const start = new Date(startTime.replace(' ', 'T'));
  const now = new Date();
  const diffMs = now - start; */

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
 * 활성 다운타임의 경과 시간을 실시간 업데이트
 */
function updateElapsedTimes() {
  const activeDowntimeItems = document.querySelectorAll('.active-downtime-item');

  activeDowntimeItems.forEach((item, index) => {
    if (activeDowntimes[index] && activeDowntimes[index].reg_date) {
      const elapsedSpan = item.querySelector('.downtime-elapsed-time span');
      if (elapsedSpan) {
        const elapsed = calculateElapsedTime(activeDowntimes[index].reg_date);
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

  console.log(`ℹ️ 조회 기간이 ${startDate.format('YYYY-MM-DD')} ~ ${endDate.format('YYYY-MM-DD')}로 설정되었습니다.`);

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
 * 데이터 내보내기
 */
/**
 * Export data to Excel with current filters
 */
function exportData() {
  try {
    console.log('📊 Downtime 데이터 내보내기...');

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
    const exportUrl = `proc/data_downtime_export.php?${exportParams.toString()}`;

    // Trigger download by redirecting to export URL
    window.location.href = exportUrl;

    console.log('Downtime data export started. Download will begin shortly.');

  } catch (error) {
    console.error('Export error:', error);
  }
}

/**
 * 데이터 새로고침
 */
async function refreshData() {
  console.log('🔄 Downtime 데이터 새로고침...');

  // 실시간 추적 중이면 연결 재시작
  if (isTracking) {
    stopTracking();
    setTimeout(async () => await startAutoTracking(), 1000);
  } else {
    // 자동 추적 재시작
    await startAutoTracking();
  }

  console.log('🔄 Downtime 데이터가 새로고침되었습니다.');
}

/**
 * Details 버튼에 이벤트 리스너 설정
 */
function setupDetailsButtonListeners() {
  console.log('🔍 Details 버튼 이벤트 리스너 설정 시작...');
  const detailsButtons = document.querySelectorAll('.downtime-details-btn');
  console.log(`🔍 찾은 Details 버튼 수: ${detailsButtons.length}`);
  console.log('🔍 찾은 버튼들:', detailsButtons);

  detailsButtons.forEach((button, index) => {
    console.log(`🔍 버튼 ${index + 1} 설정 중:`, button);

    // 기존 이벤트 리스너 제거 (중복 방지)
    button.removeEventListener('click', handleDetailsButtonClick);

    // 새 이벤트 리스너 추가
    button.addEventListener('click', handleDetailsButtonClick);
    console.log(`✅ 버튼 ${index + 1} 이벤트 리스너 추가 완료`);
  });

  console.log(`✅ Details 버튼 이벤트 리스너 설정 완료: ${detailsButtons.length}개`);
}

/**
 * Details 버튼 클릭 처리 함수
 */
function handleDetailsButtonClick(event) {
  console.log('🔍 Details 버튼 클릭됨!', event);
  event.preventDefault();
  event.stopPropagation();

  const button = event.target.closest('.downtime-details-btn');
  console.log('🔍 클릭된 버튼:', button);

  if (button) {
    openDowntimeDetailModal(button);
  } else {
    console.error('❌ Downtime Details 버튼을 찾을 수 없습니다.');
  }
}

/**
 * Downtime 상세 모달 열기 (테이블의 Details 버튼 클릭 시 호출)
 */
function openDowntimeDetailModal(buttonElement) {
  try {
    // 버튼에서 Downtime 데이터 추출
    const downtimeDataJson = buttonElement.getAttribute('data-downtime-data');
    if (!downtimeDataJson) {
      console.error('❌ Downtime 데이터를 찾을 수 없습니다.');
      return;
    }

    const downtimeData = JSON.parse(downtimeDataJson.replace(/&#39;/g, "'"));
    console.log('🔍 Downtime 상세 모달 열기:', downtimeData);

    // 모달에 데이터 표시
    populateDowntimeModal(downtimeData);

    // 모달 표시
    const modal = document.getElementById('downtimeDetailModal');
    if (modal) {
      modal.classList.add('show');
      document.body.style.overflow = 'hidden';

      console.log('✅ Downtime 상세 모달 열기 완료');
    } else {
      console.error('❌ downtimeDetailModal 요소를 찾을 수 없습니다.');
    }
  } catch (error) {
    console.error('❌ Downtime 상세 모달 열기 오류:', error);
  }
}

/**
 * Downtime 상세 정보 모달 닫기
 */
function closeDowntimeDetailModal() {
  console.log('❌ Downtime 상세 모달 닫기');

  const modal = document.getElementById('downtimeDetailModal');
  if (modal) {
    modal.classList.remove('show');
    document.body.style.overflow = ''; // 배경 스크롤 복원

    console.log('✅ Downtime 상세 모달 닫기 완료');
  }
}

/**
 * Downtime 모달에 데이터 채우기
 */
function populateDowntimeModal(downtimeData) {
  console.log('📝 Downtime 모달 데이터 채우기:', downtimeData);

  try {
    // 기본 정보 섹션
    updateModalElement('modal-machine-no', downtimeData.machine_no || '-');
    updateModalElement('modal-factory-line',
      `${downtimeData.factory_name || '-'} / ${downtimeData.line_name || '-'}`);
    updateModalElement('modal-downtime-type', downtimeData.downtime_name || '-');

    // 상태 표시 (HTML 포함)
    const statusElement = document.getElementById('modal-status');
    if (statusElement) {
      if (downtimeData.status === 'Warning') {
        statusElement.innerHTML = '<span class="fiori-badge fiori-badge--error">⚠️ Warning</span>';
      } else {
        statusElement.innerHTML = '<span class="fiori-badge fiori-badge--success">✅ Completed</span>';
      }
    }

    // 시간 정보 섹션
    updateModalElement('modal-reg-date', downtimeData.reg_date || '-');
    updateModalElement('modal-update-date', downtimeData.update_date || '-');
    updateModalElement('modal-duration',
      downtimeData.duration_display || downtimeData.duration_his || '-');
    updateModalElement('modal-work-date', downtimeData.work_date || '-');

    // 작업 정보 섹션
    const shiftDisplay = downtimeData.shift_idx ? `Shift ${downtimeData.shift_idx}` : '-';
    updateModalElement('modal-shift', shiftDisplay);

    // Downtime 색상 정보 (downtime_color가 있는 경우)
    const colorIndicator = document.getElementById('modal-color-indicator');
    const colorValue = document.getElementById('modal-color-value');
    if (colorIndicator && colorValue) {
      if (downtimeData.downtime_color) {
        colorIndicator.style.backgroundColor = downtimeData.downtime_color;
        colorIndicator.style.display = 'inline-block';
        colorValue.textContent = downtimeData.downtime_color;
      } else {
        colorIndicator.style.display = 'none';
        colorValue.textContent = 'Default Color';
      }
    }

    // 추가 정보 섹션
    updateModalElement('modal-idx', downtimeData.idx || '-');
    updateModalElement('modal-created-at', downtimeData.reg_date || '-');

    console.log('✅ Downtime 모달 데이터 채우기 완료');
  } catch (error) {
    console.error('❌ Downtime 모달 데이터 채우기 오류:', error);
  }
}

/**
 * 모달 요소 업데이트 유틸리티 함수
 */
function updateModalElement(elementId, value) {
  const element = document.getElementById(elementId);
  if (element) {
    element.textContent = value;
  } else {
    console.warn(`⚠️ 모달 요소를 찾을 수 없습니다: ${elementId}`);
  }
}

/**
 * 단일 Downtime 데이터 내보내기
 */
function exportSingleDowntime() {
  console.log('📊 단일 Downtime 데이터 내보내기...');

  // TODO: 실제 단일 데이터 내보내기 구현
  console.log('ℹ️ 단일 Downtime 데이터 내보내기 기능은 추후 구현 예정입니다.');

  // 모달 닫기
  closeDowntimeDetailModal();
}

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
    console.log('Statistics cards are now visible.');
  } else {
    // Hide stats
    statsGrid.classList.add('hidden');
    toggleBtn.textContent = '📊 Show Stats';
    console.log('Statistics cards are now hidden.');
  }
}

/**
 * Toggle charts display
 */
function toggleChartsDisplay() {
  const downtimeRowLayout = document.querySelector('.downtime-row-layout');
  const downtimeTrendSection = document.getElementById('downtimeTrendSection');
  const toggleBtn = document.getElementById('toggleChartsBtn');

  if (!toggleBtn) {
    console.error('Toggle charts button not found.');
    return;
  }

  if (!downtimeRowLayout && !downtimeTrendSection) {
    console.error('Chart sections not found.');
    return;
  }

  // 두 섹션 중 하나라도 보이는 상태라면 "보이는 상태"로 간주
  const isHidden = (downtimeRowLayout && downtimeRowLayout.classList.contains('hidden')) ||
    (downtimeTrendSection && downtimeTrendSection.classList.contains('hidden'));

  if (isHidden) {
    // Show charts
    if (downtimeRowLayout) {
      downtimeRowLayout.classList.remove('hidden');
    }
    if (downtimeTrendSection) {
      downtimeTrendSection.classList.remove('hidden');
    }
    toggleBtn.textContent = '📈 Hide Charts';
    console.log('Charts are now visible.');
  } else {
    // Hide charts
    if (downtimeRowLayout) {
      downtimeRowLayout.classList.add('hidden');
    }
    if (downtimeTrendSection) {
      downtimeTrendSection.classList.add('hidden');
    }
    toggleBtn.textContent = '📈 Show Charts';
    console.log('Charts are now hidden.');
  }
}