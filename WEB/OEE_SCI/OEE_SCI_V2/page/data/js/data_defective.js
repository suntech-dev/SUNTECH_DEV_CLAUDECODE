// ============================================================================
// Defective Data Tracking JavaScript
// ============================================================================

/**
 * Global variables and settings
 */
let eventSource = null;
let isTracking = false;
let charts = {};
let defectiveData = [];
let reconnectAttempts = 0;
let maxReconnectAttempts = 3;
let stats = {};
let activeDefectives = [];
let elapsedTimeTimer = null;
let isPageUnloading = false; // 페이지 언로드 상태 추적

// Pagination variables
let currentPage = 1;
let itemsPerPage = 10;
let totalItems = 0;

// Chart.js default settings
Chart.defaults.color = '#1a1a1a';
Chart.defaults.borderColor = '#e8eaed';

/**
 * Chart color palette for Defective
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
  console.log('Defective monitoring system initialization started');

  try {
    initDateRangePicker();
    await initFilterSystem();
    initCharts();
    setupEventListeners();
    await loadInitialData();
    await startAutoTracking();

    console.log('Defective monitoring system initialized');
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
  console.log('Defective: beforeunload - 페이지 언로드 시작');
  isPageUnloading = true; // 언로드 상태 설정 (재연결 방지)

  if (eventSource) {
    eventSource.close();
    eventSource = null;
    isTracking = false;
  }
  if (elapsedTimeTimer) {
    clearInterval(elapsedTimeTimer);
    elapsedTimeTimer = null;
  }
});

// 2. pagehide - 2순위: 페이지 숨김 시 정리 (모바일 브라우저 지원)
window.addEventListener('pagehide', () => {
  console.log('Defective: pagehide - 페이지 숨김');
  isPageUnloading = true; // 언로드 상태 설정

  if (eventSource) {
    eventSource.close();
    eventSource = null;
    isTracking = false;
  }
  if (elapsedTimeTimer) {
    clearInterval(elapsedTimeTimer);
    elapsedTimeTimer = null;
  }
});

// 3. visibilitychange - 3순위: 탭 전환 시 처리
document.addEventListener('visibilitychange', () => {
  if (document.hidden) {
    console.log('Defective: visibilitychange - 페이지 숨김, SSE 연결 종료');

    // 탭 전환 시 즉시 연결 종료
    if (eventSource) {
      eventSource.close();
      eventSource = null;
      isTracking = false;
    }
    if (elapsedTimeTimer) {
      clearInterval(elapsedTimeTimer);
      elapsedTimeTimer = null;
    }
  } else {
    console.log('Defective: visibilitychange - 페이지 표시');

    // 페이지가 언로드 중이 아닐 때만 재연결
    if (!isPageUnloading && !isTracking && eventSource === null) {
      console.log('Defective: 페이지 재표시 - SSE 재연결 시도');
      reconnectAttempts = 0;
      startAutoTracking();
    } else if (isPageUnloading) {
      console.log('⚠️ Defective: 페이지 언로드 중 - 재연결 취소');
    }
  }
});

// 4. focus - 페이지 포커스 시 언로드 플래그 리셋
window.addEventListener('focus', () => {
  if (isPageUnloading) {
    console.log('Defective: 페이지 포커스 - 언로드 플래그 리셋');
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
 * Load line options
 */
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

        // Update machine list when line is selected
        lineSelect.addEventListener('change', async (e) => {
          const factoryId = document.getElementById('factoryFilterSelect').value;
          await updateMachineOptions(factoryId, e.target.value);
          await restartRealTimeMonitoring();
        });
      }
    } else {
      console.error('Line list load failed:', res.message || 'Unknown error');
    }
  } catch (error) {
    console.error('Line options load error:', error);
  }
}

/**
 * Load machine options
 */
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
      console.error('Machine list load failed:', res.message || 'Unknown error');
    }
  } catch (error) {
    console.error('Machine options load error:', error);
  }
}

/**
 * Update line options based on factory selection
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
      console.error('Line options update failed:', res.message || 'Unknown error');
    }

    lineSelect.disabled = false;

    // Re-enable machine filter
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

/**
 * Update machine options based on factory and line selection
 */
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

/**
 * Initialize charts for Defective
 */
function initCharts() {
  try {
    // 1. Defective type analysis chart
    charts.defectiveType = createDefectiveTypeChart();

    // 2. Defective rate trend chart
    charts.defectiveRate = createDefectiveRateChart();

    // 3. Machine defective rate chart
    // charts.machineDefective = createMachineDefectiveChart();

    console.log('Defective charts initialized');
  } catch (error) {
    console.error('Charts initialization error:', error);
  }
}

/**
 * Create defective type analysis chart
 */
function createDefectiveTypeChart() {
  const ctx = document.getElementById('defectiveTypeChart').getContext('2d');
  return new Chart(ctx, {
    type: 'bar',
    data: {
      labels: [],
      datasets: [{
        label: 'Defective occurrence quantity',
        data: [],
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
            text: 'Defective quantity'
          },
          ticks: {
            stepSize: 1,
            precision: 0
          }
        },
        x: {
          title: {
            display: true,
            text: 'Defective type'
          }
        }
      }
    }
  });
}

/**
 * Create defective rate trend chart
 */
function createDefectiveRateChart() {
  const ctx = document.getElementById('defectiveRateChart').getContext('2d');
  return new Chart(ctx, {
    type: 'line',
    data: {
      labels: [],
      datasets: [{
        label: 'Defective Count',
        data: [],
        borderColor: chartColors.error,
        backgroundColor: chartColors.error + '20',
        fill: true,
        tension: 0.4,
        pointBackgroundColor: chartColors.error,
        pointBorderColor: chartColors.error,
        pointRadius: 4,
        pointHoverRadius: 6
      }]
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
          displayColors: true,
          callbacks: {
            label: function (context) {
              return context.dataset.label + ': ' + context.parsed.y + ' occurrences';
            }
          }
        }
      },
      scales: {
        y: {
          beginAtZero: true,
          title: {
            display: true,
            text: 'Defective Count'
          },
          ticks: {
            stepSize: 1,
            precision: 0,
            callback: function (value) {
              return value;
            }
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
          },
          ticks: {
            autoSkip: true,
            maxRotation: 45,
            minRotation: 0
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
 * Create machine defective rate chart
 */
/* function createMachineDefectiveChart() {
  const ctx = document.getElementById('machineDefectiveChart').getContext('2d');
  return new Chart(ctx, {
    type: 'bar',
    data: {
      labels: [],
      datasets: [{
        label: 'Defective Rate (%)',
        data: [],
        backgroundColor: chartColors.warning,
        borderColor: chartColors.warning,
        borderWidth: 1
      }]
    },
    options: {
      indexAxis: 'y',
      responsive: true,
      maintainAspectRatio: false,
      plugins: {
        legend: {
          display: true,
          position: 'top'
        }
      },
      scales: {
        x: {
          beginAtZero: true,
          title: {
            display: true,
            text: 'Defective Rate (%)'
          },
          ticks: {
            callback: function (value) {
              return value + '%';
            }
          }
        },
        y: {
          title: {
            display: true,
            text: 'Machine'
          }
        }
      }
    }
  });
} */

/**
 * Setup event listeners
 */
function setupEventListeners() {
  // Export button
  const excelDownloadBtn = document.getElementById('excelDownloadBtn');
  if (excelDownloadBtn) {
    excelDownloadBtn.addEventListener('click', exportData);
  }

  // Refresh button
  const refreshBtn = document.getElementById('refreshBtn');
  if (refreshBtn) {
    refreshBtn.addEventListener('click', refreshData);
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
  }

  // Machine filter change
  const factoryLineMachineFilterSelect = document.getElementById('factoryLineMachineFilterSelect');
  if (factoryLineMachineFilterSelect) {
    factoryLineMachineFilterSelect.addEventListener('change', async (e) => {
      await restartRealTimeMonitoring();
    });
  }

  // Shift filter change
  const shiftSelect = document.getElementById('shiftSelect');
  if (shiftSelect) {
    shiftSelect.addEventListener('change', async (e) => {
      await restartRealTimeMonitoring();
    });
  }
}

/**
 * Load initial data
 */
async function loadInitialData() {
  try {
    displayEmptyState();
  } catch (error) {
    console.error('Initial data load error:', error);
  }
}

/**
 * Display empty state
 */
function displayEmptyState() {
  // Initialize stat cards
  const statElements = [
    'totalDefective', 'activeDefectives', 'currentShiftDefective',
    'affectedMachinesDefective', 'defectiveRate', 'qualityScore'
  ];

  statElements.forEach(elementId => {
    const element = document.getElementById(elementId);
    if (element) {
      element.textContent = '-';
    }
  });

  // Update active defective count
  const activeCountEl = document.getElementById('activeDefectiveCount');
  if (activeCountEl) {
    activeCountEl.textContent = '0 active defectives';
  }

  // Initial table message
  const tbody = document.getElementById('defectiveDataBody');
  tbody.innerHTML = `
    <tr>
      <td colspan="9" class="data-table-centered">
        <div class="fiori-alert fiori-alert--info">
          <strong>ℹ️ Information:</strong> Loading real-time Defective data. Automatic monitoring is in progress.
        </div>
      </td>
    </tr>
  `;
}

/**
 * Start automatic real-time tracking
 */
async function startAutoTracking() {
  if (isTracking) {
    return;
  }

  try {
    const filters = getFilterParams();
    const params = new URLSearchParams(filters);

    const sseUrl = `proc/data_defective_stream.php?${params.toString()}`;

    eventSource = new EventSource(sseUrl);

    setupSSEEventListeners();

    isTracking = true;

    const connectionStatusEl = document.getElementById('connectionStatus');
    if (connectionStatusEl) {
      connectionStatusEl.textContent = 'Defective system connected';
    }


  } catch (error) {
    console.error('SSE connection error:', error);

    const connectionStatusEl = document.getElementById('connectionStatus');
    if (connectionStatusEl) {
      connectionStatusEl.textContent = 'Connection failed';
      connectionStatusEl.className = 'connection-status--error';
    }
  }
}

/**
 * Get filter parameters
 */
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
    limit: 100
  };
}

/**
 * Setup SSE event listeners
 */
function setupSSEEventListeners() {
  // Connection error handling
  eventSource.onerror = function (event) {
    console.error('SSE connection error:', event);
    const connectionStatusEl = document.getElementById('connectionStatus');
    if (connectionStatusEl) {
      connectionStatusEl.textContent = 'SSE connection error - reconnecting...';
      connectionStatusEl.className = 'connection-status--warning';
    }

    isTracking = false;
    attemptReconnection();
  };

  // Connection success
  eventSource.addEventListener('connected', function (event) {
    const data = JSON.parse(event.data);
    const connectionStatusEl = document.getElementById('connectionStatus');
    if (connectionStatusEl) {
      connectionStatusEl.textContent = 'Defective system connected';
      connectionStatusEl.className = 'connection-status--success';
    }

    reconnectAttempts = 0;
  });

  // Main defective data
  eventSource.addEventListener('defective_data', function (event) {
    const data = JSON.parse(event.data);

    stats = data.stats;
    activeDefectives = data.active_defectives;
    defectiveData = data.defective_data;
    window.defectiveData = data.defective_data;

    updateStatCardsFromAPI(stats);
    updateActiveDefectivesDisplay(activeDefectives);
    updateTableFromAPI(defectiveData);
    updateChartsFromAPI(data);

    const lastUpdateEl = document.getElementById('lastUpdateTime');
    if (lastUpdateEl) {
      lastUpdateEl.textContent = `Last updated: ${data.timestamp}`;
    }
  });

  // Heartbeat
  eventSource.addEventListener('heartbeat', function (event) {
    const data = JSON.parse(event.data);

    const connectionStatusEl = document.getElementById('connectionStatus');
    if (connectionStatusEl) {
      connectionStatusEl.textContent = `Connection maintained (Active warnings: ${data.active_warnings})`;
    }
  });

  // Error handling
  eventSource.addEventListener('error', function (event) {
    const data = JSON.parse(event.data);
    console.error('SSE error:', data);
  });

  // Connection close
  eventSource.addEventListener('disconnected', function (event) {
    const data = JSON.parse(event.data);
    const connectionStatusEl = document.getElementById('connectionStatus');
    if (connectionStatusEl) {
      connectionStatusEl.textContent = 'Defective system disconnected';
    }
  });
}

/**
 * Format number: show integer if whole number, else show 1 decimal place
 * @param {string|number} value - The number to format
 * @return {string} - Formatted number
 */
function formatRateNumber(value) {
  if (!value || value === '-') return '-';

  const num = parseFloat(value);

  if (isNaN(num)) return '-';

  // Check if the number is an integer
  if (Number.isInteger(num)) {
    return num.toString();
  }

  // Show 1 decimal place for non-integers
  return num.toFixed(1);
}

/**
 * Update stat cards from API data
 */
function updateStatCardsFromAPI(statsData) {
  if (!statsData) return;

  // Format defective rate: show "-" only if no data (null or undefined)
  let defectiveRateDisplay = '-';
  if (statsData.defective_rate !== null && statsData.defective_rate !== undefined) {
    defectiveRateDisplay = formatRateNumber(statsData.defective_rate) + '%';
  }

  // Format quality score: show "-" only if no data (null or undefined)
  let qualityScoreDisplay = '-';
  if (statsData.quality_score !== null && statsData.quality_score !== undefined) {
    qualityScoreDisplay = formatRateNumber(statsData.quality_score) + '%';
  }

  const elements = {
    'totalDefective': statsData.total_count || '0',
    'activeDefectives': statsData.warning_count || '0',
    'currentShiftDefective': statsData.current_shift_count || '0',
    'affectedMachinesDefective': statsData.affected_machines || '0',
    'defectiveRate': defectiveRateDisplay,
    'qualityScore': qualityScoreDisplay
  };

  for (const [elementId, value] of Object.entries(elements)) {
    const element = document.getElementById(elementId);
    if (element) {
      element.textContent = value;
    }
  }
}

/**
 * Update active defectives display
 */
function updateActiveDefectivesDisplay(activeDefectivesList) {
  const container = document.getElementById('activeDefectivesContainer');
  const countDisplay = document.getElementById('activeDefectiveCount');

  if (countDisplay) {
    countDisplay.textContent = `${activeDefectivesList.length} active defectives`;
  }

  if (!container) {
    return;
  }

  if (activeDefectivesList.length === 0) {
    container.innerHTML = `
      <div class="fiori-alert fiori-alert--success">
        <strong>✅ Good:</strong> There are currently no active Defectives. All systems are operating normally.
      </div>
    `;
    // Stop elapsed time timer when no active defectives
    stopElapsedTimeTimer();
  } else {
    // Sort by registration date (most recent first) and limit to 5 items
    const sortedDefectives = [...activeDefectivesList]
      .sort((a, b) => {
        // Sort by reg_date in descending order (newest first)
        const dateA = new Date(a.reg_date);
        const dateB = new Date(b.reg_date);
        return dateB - dateA;
      })
      .slice(0, 5); // Limit to 5 items

    let html = '';
    sortedDefectives.forEach(defective => {
      // Calculate initial elapsed time (real-time updates handled by timer)
      const initialElapsed = calculateElapsedTime(defective.reg_date);

      // Apply defective color to border and background
      const borderColor = defective.defective_color || '#0070f2';
      const backgroundColor = `${borderColor}1A`; // 10% transparency
      const borderStyle = `border: 2px dashed ${borderColor}; background-color: ${backgroundColor};`;

      html += `
        <div class="active-defective-item" style="${borderStyle}">
          <div class="defective-machine-info">
            <div class="defective-machine-name">${defective.defective_name}</div>
            <div class="defective-location">
              <span class="defective-location-item">${defective.factory_name}</span>
              <span class="defective-location-item">/</span>
              <span class="defective-location-item">${defective.line_name}</span>
              <span class="defective-location-item">/</span>
              <span class="defective-location-item">${defective.machine_no}</span>
            </div>
          </div>
          <div class="defective-elapsed-time">
            <span>⏰ ${initialElapsed}</span>
          </div>
        </div>
      `;
    });

    // Add info message if there are more than 5 active defectives
    if (activeDefectivesList.length > 5) {
      html += `
        <div class="fiori-alert fiori-alert--info" style="margin-top: var(--sap-spacing-sm);">
          <strong>ℹ️ Info:</strong> Showing 5 most recent defectives out of ${activeDefectivesList.length} total active defectives.
        </div>
      `;
    }

    container.innerHTML = html;

    // Start real-time elapsed time timer when there are active defectives
    startElapsedTimeTimer();
  }
}

/**
 * Update table from API data
 */
function updateTableFromAPI(defectiveDataList) {
  const tbody = document.getElementById('defectiveDataBody');

  if (!tbody) {
    console.error('defectiveDataBody element not found.');
    return;
  }

  totalItems = defectiveDataList.length;

  if (defectiveDataList.length === 0) {
    tbody.innerHTML = `
      <tr>
        <td colspan="9" class="data-table-centered">
          <div class="fiori-alert fiori-alert--info">
            <strong>ℹ️ Information:</strong> No Defective data matching the selected conditions.
          </div>
        </td>
      </tr>
    `;
    renderPagination();
    return;
  }

  const startIndex = (currentPage - 1) * itemsPerPage;
  const endIndex = startIndex + itemsPerPage;
  const paginatedData = defectiveDataList.slice(startIndex, endIndex);

  tbody.innerHTML = '';
  paginatedData.forEach(defective => {
    const row = document.createElement('tr');

    let shiftDisplay = '-';
    if (defective.shift_idx) {
      shiftDisplay = `Shift ${defective.shift_idx}`;
    }

    const elapsedTime = defective.status === 'Warning' ?
      calculateElapsedTime(defective.reg_date) :
      (defective.duration_display || defective.duration_his || '-');

    row.innerHTML = `
      <td>${defective.machine_no || '-'}</td>
      <td>${(defective.factory_name || '') + ' / ' + (defective.line_name || '')}</td>
      <td>${shiftDisplay}</td>
      <td>${defective.defective_name || '-'}</td>
      <td>${defective.reg_date || '-'}</td>
      <td class="defective-elapsed-time-cell"
          data-status="${defective.status || ''}"
          data-reg-date="${defective.reg_date || ''}"
          data-duration="${defective.duration_display || defective.duration_his || '-'}">${elapsedTime}</td>
      <td>${defective.work_date || '-'}</td>
      <td>
        <button class="fiori-btn fiori-btn--tertiary defective-details-btn"
                style="padding: 0.25rem 0.5rem; font-size: 0.75rem;"
                data-defective-data='${JSON.stringify(defective).replace(/'/g, "&#39;")}'>
          🔍
        </button>
      </td>
    `;

    tbody.appendChild(row);
  });

  renderPagination();
  setupDetailsButtonListeners();

  // Ensure elapsed time timer is running if there are Warning status defectives in the table
  const hasWarningDefectives = paginatedData.some(d => d.status === 'Warning');
  if (hasWarningDefectives && !elapsedTimeTimer) {
    startElapsedTimeTimer();
  }
}

/**
 * Render pagination
 */
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

  // Previous button
  paginationHTML += `
    <button class="fiori-pagination__button ${currentPage === 1 ? 'fiori-pagination__button--disabled' : ''}" 
            onclick="changePage(${currentPage - 1})" ${currentPage === 1 ? 'disabled' : ''}>
      ←
    </button>
  `;

  // Page number buttons
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

  // Next button
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
 * Change page
 */
function changePage(newPage) {
  const totalPages = Math.ceil(totalItems / itemsPerPage);

  if (newPage < 1 || newPage > totalPages || newPage === currentPage) {
    return;
  }

  currentPage = newPage;
  updateTableFromAPI(defectiveData);
}

/**
 * Update charts from API data
 */
function updateChartsFromAPI(apiData) {
  try {
    // Update defective type chart
    if (apiData.defective_type_stats && charts.defectiveType) {
      updateDefectiveTypeChart(apiData.defective_type_stats);
    }

    // Update defective rate trend chart
    if (apiData.defective_rate_stats && charts.defectiveRate) {
      updateDefectiveRateChart(apiData.defective_rate_stats);
    }

    // Update machine defective chart
    /* if (apiData.machine_defective_stats && charts.machineDefective) {
      updateMachineDefectiveChart(apiData.machine_defective_stats);
    } */

  } catch (error) {
    console.error('Chart update error:', error);
  }
}

/**
 * Update defective type chart data
 */
function updateDefectiveTypeChart(defectiveTypeStats) {
  if (!charts.defectiveType) {
    return;
  }

  if (!defectiveTypeStats || defectiveTypeStats.length === 0) {
    return;
  }

  try {
    // defective_name으로 정렬 (Downtime/Andon과 일관성 유지)
    const sortedStats = [...defectiveTypeStats].sort((a, b) => {
      const nameA = (a.defective_name || 'Unclassified').toLowerCase();
      const nameB = (b.defective_name || 'Unclassified').toLowerCase();
      return nameA.localeCompare(nameB);
    });

    const labels = sortedStats.map(item => item.defective_name || 'Unclassified');
    const counts = sortedStats.map(item => parseInt(item.count) || 0);

    const backgroundColors = [];
    const borderColors = [];
    const colorKeys = ['warning', 'error', 'primary', 'info', 'accent', 'secondary'];

    for (let i = 0; i < labels.length; i++) {
      const colorKey = colorKeys[i % colorKeys.length];
      backgroundColors.push(chartColors[colorKey]);
      borderColors.push(chartColors[colorKey]);
    }

    charts.defectiveType.data.labels = labels;
    charts.defectiveType.data.datasets[0].data = counts;
    charts.defectiveType.data.datasets[0].backgroundColor = backgroundColors;
    charts.defectiveType.data.datasets[0].borderColor = borderColors;

    charts.defectiveType.update('none');

  } catch (error) {
    console.error('Defective type chart update error:', error);
  }
}

/**
 * Update defective rate trend chart data
 */
function updateDefectiveRateChart(defectiveRateStats) {
  if (!charts.defectiveRate) {
    return;
  }

  // Handle new API response structure with work_hours
  let statsData = defectiveRateStats;
  let workHours = null;
  let viewType = 'hourly';

  // Check if response has new structure (data + work_hours)
  if (defectiveRateStats && typeof defectiveRateStats === 'object' && defectiveRateStats.data) {
    statsData = defectiveRateStats.data;
    workHours = defectiveRateStats.work_hours;
    viewType = defectiveRateStats.view_type || 'hourly';
  }

  if (!statsData || statsData.length === 0) {
    return;
  }

  try {
    // Generate labels and map data based on view type
    let labels = [];
    let counts = [];

    if (viewType === 'hourly') {
      // Hourly view (1 day or less)
      if (workHours && workHours.start_time && workHours.end_time) {
        // Generate full work hours range
        labels = generateWorkHoursLabels(
          workHours.start_time,
          workHours.end_time,
          workHours.start_minutes,
          workHours.end_minutes
        );
      } else {
        // Fallback: generate labels from data
        labels = statsData.map(item => {
          const originalLabel = item.time_label || item.display_label;
          if (originalLabel && originalLabel.includes(':')) {
            const match = originalLabel.match(/^(\d{4}-\d{2}-\d{2})\s+(\d{2}):/);
            if (match) {
              return `${match[2]}H`;
            }
          }
          return originalLabel;
        });
      }

      // Map hourly counts to labels
      const countsMap = {};
      statsData.forEach(item => {
        const originalLabel = item.time_label || item.display_label;
        if (originalLabel && originalLabel.includes(':')) {
          const match = originalLabel.match(/^(\d{4}-\d{2}-\d{2})\s+(\d{2}):/);
          if (match) {
            const hourLabel = `${match[2]}H`;
            countsMap[hourLabel] = parseInt(item.defective_count) || 0;
          }
        }
      });

      // Fill in counts array with 0 for missing hours
      counts = labels.map(label => countsMap[label] || 0);

    } else {
      // Daily view (more than 1 day)
      // Use display_label directly from data (format: "MM/DD" or date string)
      labels = statsData.map(item => item.display_label || item.time_label);
      counts = statsData.map(item => parseInt(item.defective_count) || 0);
    }

    charts.defectiveRate.data.labels = labels;
    charts.defectiveRate.data.datasets[0].data = counts;

    charts.defectiveRate.update('none');

  } catch (error) {
    console.error('Defective rate chart update error:', error);
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
 * Update machine defective chart data
 */
/* function updateMachineDefectiveChart(machineDefectiveStats) {
  if (!charts.machineDefective) {
    return;
  }

  if (!machineDefectiveStats || machineDefectiveStats.length === 0) {
    return;
  }

  try {
    const labels = machineDefectiveStats.map(item => item.machine_no || 'Unknown');
    const rates = machineDefectiveStats.map(item => parseFloat(item.defective_rate) || 0);

    charts.machineDefective.data.labels = labels;
    charts.machineDefective.data.datasets[0].data = rates;

    charts.machineDefective.update('none');

  } catch (error) {
    console.error('Machine defective chart update error:', error);
  }
} */

/**
 * Stop real-time tracking
 */
function stopTracking() {
  if (!isTracking) {
    return;
  }

  if (eventSource) {
    eventSource.close();
    eventSource = null;
  }

  // Stop elapsed time timer
  stopElapsedTimeTimer();

  isTracking = false;

  document.getElementById('connectionStatus').textContent = 'Defective system disconnected';

}

/**
 * Restart real-time monitoring on filter change
 */
async function restartRealTimeMonitoring() {
  currentPage = 1;

  if (isTracking) {
    stopTracking();
    await new Promise(resolve => setTimeout(resolve, 100));
  }

  reconnectAttempts = 0;

  await startAutoTracking();
}

/**
 * Attempt SSE reconnection
 */
async function attemptReconnection() {
  // 페이지가 언로드 중이면 재연결하지 않음
  if (isPageUnloading) {
    console.log('⚠️ Defective: 페이지 언로드 중 - 재연결 취소');
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
      console.log('⚠️ Defective: 페이지 언로드 중 - 재연결 취소');
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
 * Update elapsed times for active defectives in real-time
 */
function updateElapsedTimes() {
  // Update active defectives section
  const activeDefectiveItems = document.querySelectorAll('.active-defective-item');

  // Sort activeDefectives by reg_date (most recent first) and limit to 5
  const sortedDefectives = [...activeDefectives]
    .sort((a, b) => {
      const dateA = new Date(a.reg_date);
      const dateB = new Date(b.reg_date);
      return dateB - dateA;
    })
    .slice(0, 5);

  activeDefectiveItems.forEach((item, index) => {
    if (sortedDefectives[index] && sortedDefectives[index].reg_date) {
      const elapsedSpan = item.querySelector('.defective-elapsed-time span');
      if (elapsedSpan) {
        const elapsed = calculateElapsedTime(sortedDefectives[index].reg_date);
        elapsedSpan.textContent = `⏰ ${elapsed}`;
      }
    }
  });

  // Update table elapsed time cells
  const tableCells = document.querySelectorAll('.defective-elapsed-time-cell');
  tableCells.forEach(cell => {
    const status = cell.getAttribute('data-status');
    const regDate = cell.getAttribute('data-reg-date');
    const duration = cell.getAttribute('data-duration');

    if (status === 'Warning' && regDate) {
      const elapsed = calculateElapsedTime(regDate);
      cell.textContent = elapsed;
    } else if (duration && duration !== '-') {
      cell.textContent = duration;
    }
  });

  // Update modal elapsed time if modal is open
  if (window.currentModalDefectiveData) {
    const modalElapsedTimeEl = document.getElementById('modal-elapsed-time');
    const defectiveData = window.currentModalDefectiveData;

    if (modalElapsedTimeEl && defectiveData.status === 'Warning' && defectiveData.reg_date) {
      const elapsed = calculateElapsedTime(defectiveData.reg_date);
      modalElapsedTimeEl.textContent = elapsed;
    }
  }
}

/**
 * Start elapsed time timer (updates every second)
 */
function startElapsedTimeTimer() {
  // Clear existing timer if any
  if (elapsedTimeTimer) {
    clearInterval(elapsedTimeTimer);
  }

  // Update elapsed time every 1 second
  elapsedTimeTimer = setInterval(updateElapsedTimes, 1000);
}

/**
 * Stop elapsed time timer
 */
function stopElapsedTimeTimer() {
  if (elapsedTimeTimer) {
    clearInterval(elapsedTimeTimer);
    elapsedTimeTimer = null;
  }
}

/**
 * Handle date range change
 */
async function handleDateRangeChange(startDate, endDate, label) {
  const timeRangeSelect = document.getElementById('timeRangeSelect');
  if (timeRangeSelect) {
    let customOption = timeRangeSelect.querySelector('option[value="custom"]');
    if (!customOption) {
      customOption = document.createElement('option');
      customOption.value = 'custom';
      customOption.textContent = 'Custom Range';
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

/**
 * Handle time range change
 */
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
    const exportUrl = `proc/data_defective_export.php?${exportParams.toString()}`;

    // Trigger download by redirecting to export URL
    window.location.href = exportUrl;


  } catch (error) {
    console.error('Export error:', error);
  }
}

/**
 * Refresh data
 */
async function refreshData() {
  if (isTracking) {
    stopTracking();
    setTimeout(async () => await startAutoTracking(), 1000);
  } else {
    await startAutoTracking();
  }

}

/**
 * Setup details button listeners
 */
function setupDetailsButtonListeners() {
  const detailsButtons = document.querySelectorAll('.defective-details-btn');

  detailsButtons.forEach((button) => {
    button.removeEventListener('click', handleDetailsButtonClick);
    button.addEventListener('click', handleDetailsButtonClick);
  });
}

/**
 * Handle details button click
 */
function handleDetailsButtonClick(event) {
  event.preventDefault();
  event.stopPropagation();

  const button = event.target.closest('.defective-details-btn');

  if (button) {
    openDefectiveDetailModal(button);
  } else {
    console.error('Defective Details button not found.');
  }
}

/**
 * Open defective detail modal
 */
function openDefectiveDetailModal(buttonElement) {
  try {
    const defectiveDataJson = buttonElement.getAttribute('data-defective-data');
    if (!defectiveDataJson) {
      console.error('Defective data not found.');
      return;
    }

    const defectiveData = JSON.parse(defectiveDataJson.replace(/&#39;/g, "'"));

    // Store defective data for modal elapsed time update
    window.currentModalDefectiveData = defectiveData;

    populateDefectiveModal(defectiveData);

    const modal = document.getElementById('defectiveDetailModal');
    if (modal) {
      modal.classList.add('show');
      document.body.style.overflow = 'hidden';
    } else {
      console.error('defectiveDetailModal element not found.');
    }
  } catch (error) {
    console.error('Defective detail modal open error:', error);
  }
}

/**
 * Close defective detail modal
 */
function closeDefectiveDetailModal() {
  const modal = document.getElementById('defectiveDetailModal');
  if (modal) {
    modal.classList.remove('show');
    document.body.style.overflow = '';

    // Clear stored modal data
    window.currentModalDefectiveData = null;
  }
}

/**
 * Populate defective modal with data
 */
function populateDefectiveModal(defectiveData) {
  try {
    // Basic information section
    updateModalElement('modal-machine-no', defectiveData.machine_no || '-');
    updateModalElement('modal-factory-line',
      `${defectiveData.factory_name || '-'} / ${defectiveData.line_name || '-'}`);
    updateModalElement('modal-defective-type', defectiveData.defective_name || '-');

    // Status display
    const statusElement = document.getElementById('modal-status');
    if (statusElement) {
      if (defectiveData.status === 'Warning') {
        statusElement.innerHTML = '<span class="fiori-badge fiori-badge--error">⚠️ Warning</span>';
      } else {
        statusElement.innerHTML = '<span class="fiori-badge fiori-badge--success">✅ Completed</span>';
      }
    }

    // Time information section
    updateModalElement('modal-reg-date', defectiveData.reg_date || '-');
    const elapsedTime = defectiveData.status === 'Warning' ?
      calculateElapsedTime(defectiveData.reg_date) :
      (defectiveData.duration_display || defectiveData.duration_his || '-');
    updateModalElement('modal-elapsed-time', elapsedTime);
    updateModalElement('modal-work-date', defectiveData.work_date || '-');

    // Work information section
    const shiftDisplay = defectiveData.shift_idx ? `Shift ${defectiveData.shift_idx}` : '-';
    updateModalElement('modal-shift', shiftDisplay);

    // Defective color information
    const colorIndicator = document.getElementById('modal-color-indicator');
    const colorValue = document.getElementById('modal-color-value');
    if (colorIndicator && colorValue) {
      if (defectiveData.defective_color) {
        colorIndicator.style.backgroundColor = defectiveData.defective_color;
        colorIndicator.style.display = 'inline-block';
        colorValue.textContent = defectiveData.defective_color;
      } else {
        colorIndicator.style.display = 'none';
        colorValue.textContent = 'Default Color';
      }
    }

    // Additional information section
    updateModalElement('modal-idx', defectiveData.idx || '-');
    updateModalElement('modal-created-at', defectiveData.reg_date || '-');

  } catch (error) {
    console.error('Defective modal populate error:', error);
  }
}

/**
 * Update modal element utility function
 */
function updateModalElement(elementId, value) {
  const element = document.getElementById(elementId);
  if (element) {
    element.textContent = value;
  }
}

/**
 * Export single defective data
 */
function exportSingleDefective() {
  closeDefectiveDetailModal();
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
  const defectiveRowLayout = document.querySelector('.defective-row-layout');
  const defectiveTrendSection = document.getElementById('defectiveTrendSection');
  const toggleBtn = document.getElementById('toggleChartsBtn');

  if (!toggleBtn) {
    console.error('Toggle charts button not found.');
    return;
  }

  if (!defectiveRowLayout) {
    console.error('Chart sections not found.');
    return;
  }

  const isHidden = defectiveRowLayout.classList.contains('hidden');

  if (isHidden) {
    // Show charts
    if (defectiveRowLayout) {
      defectiveRowLayout.classList.remove('hidden');
    }
    if (defectiveTrendSection) {
      defectiveTrendSection.classList.remove('hidden');
    }
    toggleBtn.textContent = '📈 Hide Charts';
  } else {
    // Hide charts
    if (defectiveRowLayout) {
      defectiveRowLayout.classList.add('hidden');
    }
    if (defectiveTrendSection) {
      defectiveTrendSection.classList.add('hidden');
    }
    toggleBtn.textContent = '📈 Show Charts';
  }
}