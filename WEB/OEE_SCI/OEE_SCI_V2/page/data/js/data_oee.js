// OEE Data Monitoring JavaScript

let eventSource = null;
let isTracking = false;
let charts = {};
let oeeData = [];
let reconnectAttempts = 0;
let maxReconnectAttempts = 3;
let stats = {};
let isPageUnloading = false; // 페이지 언로드 상태 추적

let currentPage = 1;
let itemsPerPage = 10;
let totalItems = 0;

Chart.defaults.color = '#1a1a1a';
Chart.defaults.borderColor = '#e8eaed';

const chartColors = {
  oee_overall: '#0070f2',
  availability: '#30914c',
  performance: '#1e88e5',
  quality: '#e26b0a',
  target: '#da1e28',
  production: '#7c3aed',
  machine1: '#00d4aa',
  machine2: '#ff6b6b',
  secondary: '#1e88e5',
  excellent: '#30914c',
  good: '#0070f2',
  fair: '#e26b0a',
  poor: '#da1e28'
};

document.addEventListener('DOMContentLoaded', async function () {
  console.log('OEE monitoring system initialization started');

  try {
    initDateRangePicker();
    await initFilterSystem();
    initCharts();
    setupEventListeners();
    await loadInitialData();
    await startAutoTracking();
    console.log('OEE monitoring system initialized');
  } catch (error) {
    console.error('Initialization error:', error);
  }
});

// 페이지 언로드 시 SSE 연결 종료 (브라우저 동시 연결 수 제한 문제 해결)
// 우선순위: beforeunload > pagehide > visibilitychange

// 1. beforeunload - 가장 먼저 실행되어 페이지 언로드 플래그 설정
window.addEventListener('beforeunload', () => {
  console.log('OEE: beforeunload - 페이지 언로드 시작');
  isPageUnloading = true; // 플래그 설정으로 재연결 방지

  if (eventSource) {
    eventSource.close();
    eventSource = null;
    isTracking = false;
  }
});

// 2. pagehide - 브라우저가 페이지를 숨길 때 (모바일에서 중요)
window.addEventListener('pagehide', () => {
  console.log('OEE: pagehide - 페이지 숨김');
  isPageUnloading = true;

  if (eventSource) {
    eventSource.close();
    eventSource = null;
    isTracking = false;
  }
});

// 3. visibilitychange - 탭 전환이나 페이지 숨김
document.addEventListener('visibilitychange', () => {
  if (document.hidden) {
    // 페이지가 숨겨질 때 SSE 연결을 즉시 정리
    console.log('OEE: 페이지 숨김 - SSE 연결 종료');

    if (eventSource && isTracking) {
      eventSource.close();
      eventSource = null;
      isTracking = false;
    }
  } else {
    // 페이지가 다시 보일 때만 재연결 (언로드 중이 아닐 때만)
    console.log('OEE: 페이지 표시 - SSE 재연결 시도');

    if (!isPageUnloading && !isTracking && eventSource === null) {
      reconnectAttempts = 0;
      startAutoTracking();
    }
  }
});

// 4. 페이지 포커스 복원 시 - 언로드 플래그 리셋
window.addEventListener('focus', () => {
  if (isPageUnloading) {
    console.log('OEE: 페이지 포커스 - 언로드 플래그 리셋');
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

function initCharts() {
  try {
    charts.oeeTrend = createOeeTrendChart();
    charts.oeeComponent = createOeeComponentChart();
    charts.productionTrend = createProductionTrendChart();
    charts.machineOee = createMachineOeeChart();
    charts.oeeGrade = createOeeGradeChart();
    charts.efficiencyMatrix = createEfficiencyMatrixChart();
  } catch (error) {
    console.error('Chart initialization error:', error);
  }
}

function createOeeTrendChart() {
  const ctx = document.getElementById('oeeTrendChart').getContext('2d');
  return new Chart(ctx, {
    type: 'line',
    data: {
      labels: [],
      datasets: [{
        label: 'OEE %',
        data: [],
        borderColor: chartColors.oee_overall,
        backgroundColor: chartColors.oee_overall + '20',
        fill: true,
        tension: 0.4,
        pointBackgroundColor: chartColors.oee_overall,
        pointBorderColor: chartColors.oee_overall,
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
          displayColors: true
        }
      },
      scales: {
        y: {
          beginAtZero: true,
          title: {
            display: true,
            text: 'OEE (%)'
          },
          ticks: {
            stepSize: 10,
            callback: function (value) {
              return value + '%';
            }
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

function createOeeComponentChart() {
  const ctx = document.getElementById('oeeComponentChart').getContext('2d');
  return new Chart(ctx, {
    type: 'radar',
    data: {
      labels: ['Availability', 'Performance', 'Quality'],
      datasets: [
        {
          label: 'Current OEE',
          data: [0, 0, 0],
          borderColor: chartColors.oee_overall,
          backgroundColor: chartColors.oee_overall + '30',
          pointBackgroundColor: chartColors.oee_overall,
          pointBorderColor: chartColors.oee_overall,
          pointRadius: 5,
          pointHoverRadius: 7
        },
        {
          label: 'Target',
          data: [100, 100, 100],
          borderColor: chartColors.target,
          backgroundColor: chartColors.target + '20',
          pointBackgroundColor: chartColors.target,
          pointBorderColor: chartColors.target,
          pointRadius: 4,
          pointHoverRadius: 6,
          borderDash: [5, 5]
        }
      ]
    },
    options: {
      responsive: true,
      maintainAspectRatio: false,
      scales: {
        r: {
          beginAtZero: true,
          max: 100,
          ticks: {
            stepSize: 20,
            callback: function (value) {
              return value + '%';
            }
          }
        }
      },
      plugins: {
        legend: {
          display: true,
          position: 'top'
        }
      }
    }
  });
}

function createProductionTrendChart() {
  const ctx = document.getElementById('productionTrendChart').getContext('2d');
  return new Chart(ctx, {
    type: 'bar',
    data: {
      labels: [],
      datasets: [
        {
          label: 'Actual Output',
          data: [],
          backgroundColor: chartColors.production + '80',
          borderColor: chartColors.production,
          borderWidth: 1,
          yAxisID: 'y'
        },
        {
          label: 'Target Output',
          data: [],
          type: 'line',
          borderColor: chartColors.target,
          backgroundColor: 'transparent',
          borderWidth: 2,
          pointRadius: 3,
          pointHoverRadius: 5,
          yAxisID: 'y',
          borderDash: [5, 5]
        }
      ]
    },
    options: {
      responsive: true,
      maintainAspectRatio: false,
      interaction: {
        intersect: false,
        mode: 'index'
      },
      plugins: {
        legend: {
          display: true,
          position: 'top'
        },
        tooltip: {
          backgroundColor: 'rgba(255, 255, 255, 0.95)',
          titleColor: '#333',
          bodyColor: '#666',
          borderColor: '#ddd',
          borderWidth: 1,
          cornerRadius: 8
        }
      },
      scales: {
        y: {
          type: 'linear',
          display: true,
          position: 'left',
          beginAtZero: true,
          title: {
            display: true,
            text: 'Production Count'
          },
          grid: {
            color: 'rgba(0, 0, 0, 0.1)'
          }
        },
        x: {
          title: {
            display: true,
            text: 'Time Period'
          },
          grid: {
            color: 'rgba(0, 0, 0, 0.1)'
          }
        }
      }
    }
  });
}

function createMachineOeeChart() {
  const ctx = document.getElementById('machineOeeChart').getContext('2d');
  return new Chart(ctx, {
    type: 'bar',
    data: {
      labels: [],
      datasets: [
        {
          label: 'Availability',
          data: [],
          backgroundColor: chartColors.availability + '80',
          borderColor: chartColors.availability,
          borderWidth: 1
        },
        {
          label: 'Performance',
          data: [],
          backgroundColor: chartColors.performance + '80',
          borderColor: chartColors.performance,
          borderWidth: 1
        },
        {
          label: 'Quality',
          data: [],
          backgroundColor: chartColors.quality + '80',
          borderColor: chartColors.quality,
          borderWidth: 1
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
          backgroundColor: 'rgba(255, 255, 255, 0.95)',
          titleColor: '#333',
          bodyColor: '#666',
          borderColor: '#ddd',
          borderWidth: 1,
          cornerRadius: 8,
          callbacks: {
            label: function (context) {
              return context.dataset.label + ': ' + context.parsed.y + '%';
            }
          }
        }
      },
      scales: {
        y: {
          beginAtZero: true,
          suggestedMax: 100,
          title: {
            display: true,
            text: 'Performance (%)'
          },
          ticks: {
            stepSize: 10,
            callback: function (value) {
              return value + '%';
            }
          },
          grid: {
            color: 'rgba(0, 0, 0, 0.1)'
          }
        },
        x: {
          title: {
            display: true,
            text: 'Line Name'
          },
          grid: {
            color: 'rgba(0, 0, 0, 0.1)'
          }
        }
      }
    }
  });
}

function createOeeGradeChart() {
  const ctx = document.getElementById('oeeGradeChart').getContext('2d');
  return new Chart(ctx, {
    type: 'doughnut',
    data: {
      labels: [' ≥85%', ' 70-84%', ' 50-69%', ' <50%'],
      datasets: [{
        label: 'OEE Performance Grade',
        data: [0, 0, 0, 0],
        backgroundColor: [
          chartColors.excellent + 'CC',
          chartColors.good + 'CC',
          chartColors.fair + 'CC',
          chartColors.poor + 'CC'
        ],
        borderColor: [
          chartColors.excellent,
          chartColors.good,
          chartColors.fair,
          chartColors.poor
        ],
        borderWidth: 2,
        hoverOffset: 10
      }]
    },
    options: {
      responsive: true,
      maintainAspectRatio: false,
      plugins: {
        legend: {
          display: true,
          position: 'bottom',
          labels: {
            padding: 20,
            usePointStyle: true,
            font: {
              size: 12
            }
          }
        },
        tooltip: {
          backgroundColor: 'rgba(255, 255, 255, 0.95)',
          titleColor: '#333',
          bodyColor: '#666',
          borderColor: '#ddd',
          borderWidth: 1,
          cornerRadius: 8,
          callbacks: {
            label: function (context) {
              const total = context.dataset.data.reduce((a, b) => a + b, 0);
              const percentage = total > 0 ? ((context.raw / total) * 100).toFixed(1) : 0;
              return context.label + ': ' + context.raw + ' machines (' + percentage + '%)';
            }
          }
        }
      },
      animation: {
        animateRotate: true,
        animateScale: true
      }
    }
  });
}

function createEfficiencyMatrixChart() {
  const ctx = document.getElementById('efficiencyMatrixChart').getContext('2d');
  return new Chart(ctx, {
    type: 'scatter',
    data: {
      datasets: [{
        label: 'Machine Efficiency',
        data: [],
        backgroundColor: function (context) {
          const value = context.raw;
          if (!value || !value.r) return chartColors.secondary + '80';

          if (value.r >= 85) return chartColors.excellent + '80';
          else if (value.r >= 70) return chartColors.good + '80';
          else if (value.r >= 50) return chartColors.fair + '80';
          else return chartColors.poor + '80';
        },
        borderColor: function (context) {
          const value = context.raw;
          if (!value || !value.r) return chartColors.secondary;

          if (value.r >= 85) return chartColors.excellent;
          else if (value.r >= 70) return chartColors.good;
          else if (value.r >= 50) return chartColors.fair;
          else return chartColors.poor;
        },
        borderWidth: 2,
        pointRadius: function (context) {
          const value = context.raw;
          if (!value || !value.r) return 8;

          return Math.max(5, Math.min(15, (value.r / 100) * 15));
        },
        pointHoverRadius: function (context) {
          const value = context.raw;
          if (!value || !value.r) return 10;

          return Math.max(7, Math.min(18, (value.r / 100) * 18));
        }
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
          backgroundColor: 'rgba(255, 255, 255, 0.95)',
          titleColor: '#333',
          bodyColor: '#666',
          borderColor: '#ddd',
          borderWidth: 1,
          cornerRadius: 8,
          callbacks: {
            title: function (context) {
              const point = context[0];
              return point.raw.machine_no || 'Machine';
            },
            label: function (context) {
              const value = context.raw;
              return [
                `Performance: ${value.x}%`,
                `Quality: ${value.y}%`,
                `Overall OEE: ${value.r}%`
              ];
            }
          }
        }
      },
      scales: {
        x: {
          type: 'linear',
          position: 'bottom',
          title: {
            display: true,
            text: 'Performance Rate (%)',
            font: {
              size: 14,
              weight: 'bold'
            }
          },
          min: 0,
          suggestedMax: 100,
          ticks: {
            stepSize: 10,
            callback: function (value) {
              return value + '%';
            }
          },
          grid: {
            color: 'rgba(0, 0, 0, 0.1)'
          }
        },
        y: {
          title: {
            display: true,
            text: 'Quality Rate (%)',
            font: {
              size: 14,
              weight: 'bold'
            }
          },
          min: 0,
          suggestedMax: 100,
          ticks: {
            stepSize: 10,
            callback: function (value) {
              return value + '%';
            }
          },
          grid: {
            color: 'rgba(0, 0, 0, 0.1)'
          }
        }
      },
      interaction: {
        intersect: false,
        mode: 'point'
      }
    }
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

  // Toggle data (table) button
  const toggleDataBtn = document.getElementById('toggleDataBtn');
  if (toggleDataBtn) {
    toggleDataBtn.addEventListener('click', toggleDataDisplay);
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

  const detailElements = [
    'runtime', 'plannedTime', 'availabilityDetail', 'actualOutput',
    'theoreticalOutput', 'performanceDetail', 'goodProducts',
    'defectiveProducts', 'qualityDetail'
  ];

  detailElements.forEach(elementId => {
    const element = document.getElementById(elementId);
    if (element) {
      element.textContent = '-';
    }
  });

  const tbody = document.getElementById('oeeDataBody');
  tbody.innerHTML = `
    <tr>
      <td colspan="10" class="data-table-centered">
        <div class="fiori-alert fiori-alert--info">
          <strong>ℹ️ Information:</strong> Loading real-time OEE data. Automatic monitoring is in progress.
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

    const sseUrl = `proc/data_oee_stream.php?${params.toString()}`;

    eventSource = new EventSource(sseUrl);
    setupSSEEventListeners();

    isTracking = true;

    const connectionStatusEl = document.getElementById('connectionStatus');
    if (connectionStatusEl) {
      connectionStatusEl.textContent = 'OEE System Real-time Connected';
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
    limit: 100
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
      connectionStatusEl.textContent = 'OEE system connected';
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
    updateOeeDetailsFromAPI(data.oee_details);
    updateTableFromAPI(oeeData);
    updateChartsFromAPI(data);

    const lastUpdateEl = document.getElementById('lastUpdateTime');
    if (lastUpdateEl) {
      lastUpdateEl.textContent = `Last updated: ${data.timestamp}`;
    }
  });

  eventSource.addEventListener('heartbeat', function (event) {
    const data = JSON.parse(event.data);

    const connectionStatusEl = document.getElementById('connectionStatus');
    if (connectionStatusEl) {
      connectionStatusEl.textContent = `Connection maintained (Active machines: ${data.active_machines})`;
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
      connectionStatusEl.textContent = 'OEE System Connection Closed';
    }
  });
}

function formatPercentage(value) {
  const num = parseFloat(value);
  if (isNaN(num)) return '0%';

  // 정수인지 확인 (소수점 이하가 0인지)
  return (num % 1 === 0 ? Math.floor(num) : num) + '%';
}

function formatDecimal(value, decimals = 2) {
  const num = parseFloat(value);
  if (isNaN(num)) return '-';

  // 소수점 자리수만큼 반올림
  return num.toFixed(decimals);
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

function updateOeeDetailsFromAPI(oeeDetails) {
  if (!oeeDetails) return;

  const elements = {
    'oeeRateDetail': formatPercentage(oeeDetails.overall_oee || '0'),
    'overallEfficiency': formatPercentage(oeeDetails.overall_oee || '0'),
    'targetAchievement': oeeDetails.target_achievement || '-',
    'runtime': oeeDetails.runtime || '-',
    'plannedTime': oeeDetails.planned_time || '-',
    'availabilityDetail': formatPercentage(oeeDetails.availability || '0'),
    'actualOutput': oeeDetails.actual_output || '-',
    'theoreticalOutput': oeeDetails.theoretical_output ? formatDecimal(oeeDetails.theoretical_output, 2) : '-',
    'performanceDetail': formatPercentage(oeeDetails.performance || '0'),
    'goodProducts': oeeDetails.good_products || '-',
    'defectiveProducts': oeeDetails.defective_products || '-',
    'qualityDetail': formatPercentage(oeeDetails.quality || '0')
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
    tbody.innerHTML = `
      <tr>
        <td colspan="10" class="data-table-centered">
          <div class="fiori-alert fiori-alert--info">
            <strong>ℹ️ Information:</strong> No OEE data matching the selected conditions.
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

    let oeeClass = 'fiori-badge';
    if (parseFloat(oee.overall_oee) >= 85) {
      oeeClass += ' fiori-badge--success';
    } else if (parseFloat(oee.overall_oee) >= 70) {
      oeeClass += ' fiori-badge--warning';
    } else {
      oeeClass += ' fiori-badge--error';
    }

    let shiftDisplay = '-';
    if (oee.shift_idx) {
      shiftDisplay = `Shift ${oee.shift_idx}`;
    }

    row.innerHTML = `
      <td>${oee.machine_no || '-'}</td>
      <td>${(oee.factory_name || '') + ' / ' + (oee.line_name || '')}</td>
      <td>${shiftDisplay}</td>
      <td><span class="${oeeClass}">${(oee.overall_oee || 0)}%</span></td>
      <td><span class="fiori-badge fiori-badge--success">${(oee.availability || 0)}%</span></td>
      <td><span class="fiori-badge fiori-badge--info">${(oee.performance || 0)}%</span></td>
      <td><span class="fiori-badge fiori-badge--warning">${(oee.quality || 0)}%</span></td>
      <td>${oee.work_date || '-'}</td>
      <td>${oee.update_date || '-'}</td>
      <td>
        <button class="fiori-btn fiori-btn--tertiary oee-details-btn" 
                style="padding: 0.25rem 0.5rem; font-size: 0.75rem;"
                data-oee-data='${JSON.stringify(oee).replace(/'/g, "&#39;")}'>
          Details
        </button>
      </td>
    `;

    tbody.appendChild(row);
  });

  renderPagination();
  setupDetailsButtonListeners();
}

function setupDetailsButtonListeners() {
  const detailsButtons = document.querySelectorAll('.oee-details-btn');

  detailsButtons.forEach(button => {
    button.removeEventListener('click', handleDetailsButtonClick);
    button.addEventListener('click', handleDetailsButtonClick);
  });
}

function handleDetailsButtonClick(event) {
  event.preventDefault();
  event.stopPropagation();

  const button = event.target.closest('.oee-details-btn');
  if (button) {
    openOeeDetailModal(button);
  }
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

function updateChartsFromAPI(apiData) {
  try {
    if (apiData.oee_trend_stats && charts.oeeTrend) {
      updateOeeTrendChart(apiData.oee_trend_stats);
    }

    if (apiData.oee_component_stats && charts.oeeComponent) {
      updateOeeComponentChart(apiData.oee_component_stats);
    }

    if (apiData.production_trend_stats && charts.productionTrend) {
      updateProductionTrendChart(apiData.production_trend_stats);
    }

    if (apiData.machine_oee_stats && charts.machineOee) {
      updateMachineOeeChart(apiData.machine_oee_stats);
    }

    if (apiData.stats && charts.oeeGrade) {
      updateOeeGradeChart(apiData.stats);
    }

    if (apiData.machine_oee_stats && charts.efficiencyMatrix) {
      updateEfficiencyMatrixChart(apiData.machine_oee_stats);
    }

  } catch (error) {
    console.error('Chart update error:', error);
  }
}

function updateOeeTrendChart(oeeTrendStats) {
  console.log('OEE trend chart update started:', oeeTrendStats);

  if (!charts.oeeTrend) {
    console.error('charts.oeeTrend not initialized.');
    return;
  }

  if (!oeeTrendStats) {
    console.warn('oeeTrendStats not available.');
    charts.oeeTrend.data.labels = [];
    charts.oeeTrend.data.datasets[0].data = [];
    charts.oeeTrend.update('none');
    return;
  }

  if (!oeeTrendStats.data || oeeTrendStats.data.length === 0) {
    console.warn('OEE trend data is empty.');

    // Fallback: generate simple trend from existing OEE data
    if (window.oeeData && window.oeeData.length > 0) {
      console.log('Generate simple trend with existing OEE data:', window.oeeData.length + ' items');
      generateSimpleTrendFromOeeData();
      return;
    }

    // Completely empty case
    charts.oeeTrend.data.labels = ['No Data'];
    charts.oeeTrend.data.datasets[0].data = [0];
    charts.oeeTrend.update('none');
    return;
  }

  try {
    // Handle new API response structure with work_hours
    let trendData = oeeTrendStats.data || oeeTrendStats;
    let workHours = null;
    let viewType = 'hourly';

    // Check if response has new structure (data + work_hours)
    if (oeeTrendStats && typeof oeeTrendStats === 'object' && oeeTrendStats.data) {
      trendData = oeeTrendStats.data;
      workHours = oeeTrendStats.work_hours;
      viewType = oeeTrendStats.view_type || 'hourly';
    }

    console.log('Trend chart original data:', trendData);
    console.log('View type:', viewType);
    console.log('Work hours info:', workHours);

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

    // Map OEE values to labels
    const oeeValuesMap = {};

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
        oeeValuesMap[labelKey] = parseFloat(item.overall_oee) || 0;
      }
    });

    // Fill in OEE values array with 0 for missing hours
    const oeeValues = labels.map(label => oeeValuesMap[label] || 0);

    console.log('Chart labels:', labels);
    console.log('OEE values:', oeeValues);

    // Update x-axis title based on view type
    const xAxisTitle = viewType === 'hourly' ? 'Time (within 1 day)' : 'Date (more than 1 day)';

    // Update chart data
    charts.oeeTrend.data.labels = labels;
    charts.oeeTrend.data.datasets[0].data = oeeValues;

    // Update x-axis title
    charts.oeeTrend.options.scales.x.title.text = xAxisTitle;

    // Redraw chart with smooth animation
    charts.oeeTrend.update('show');

    console.log(`OEE trend chart update completed: ${labels.length} points (${viewType} view)`);

  } catch (error) {
    console.error('OEE trend chart update error:', error);
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
 * Generate simple trend from existing OEE data (fallback logic)
 */
function generateSimpleTrendFromOeeData() {
  if (!charts.oeeTrend || !window.oeeData || window.oeeData.length === 0) {
    return;
  }

  try {
    // Group OEE data by date
    const dateGroups = {};

    window.oeeData.forEach(item => {
      if (!item.work_date) return;

      const date = item.work_date;
      if (!dateGroups[date]) {
        dateGroups[date] = {
          totalOee: 0,
          count: 0
        };
      }

      dateGroups[date].totalOee += parseFloat(item.overall_oee) || 0;
      dateGroups[date].count++;
    });

    // Generate sorted date list
    const sortedDates = Object.keys(dateGroups).sort();
    const labels = sortedDates.map(date => {
      const d = new Date(date);
      return (d.getMonth() + 1) + '/' + d.getDate();
    });

    const avgOeeValues = sortedDates.map(date => {
      const group = dateGroups[date];
      return group.count > 0 ? (group.totalOee / group.count) : 0;
    });

    // Update chart data
    charts.oeeTrend.data.labels = labels;
    charts.oeeTrend.data.datasets[0].data = avgOeeValues;

    // Update x-axis title
    charts.oeeTrend.options.scales.x.title.text = 'Date';

    // Redraw chart
    charts.oeeTrend.update('show');

    console.log(`Simple trend chart generated from existing data: ${labels.length} dates`);

  } catch (error) {
    console.error('Simple trend chart generation error:', error);
  }
}

function updateOeeComponentChart(oeeComponentStats) {
  if (!charts.oeeComponent || !oeeComponentStats) {
    return;
  }

  try {
    const availability = parseFloat(oeeComponentStats.availability) || 0;
    const performance = parseFloat(oeeComponentStats.performance) || 0;
    const quality = parseFloat(oeeComponentStats.quality) || 0;

    charts.oeeComponent.data.datasets[0].data = [availability, performance, quality];
    charts.oeeComponent.update('none');

  } catch (error) {
    console.error('OEE component chart update error:', error);
  }
}

function updateProductionTrendChart(productionTrendStats) {
  console.log('Production trend chart update started:', productionTrendStats);

  if (!charts.productionTrend) {
    console.error('charts.productionTrend not initialized.');
    return;
  }

  if (!productionTrendStats) {
    console.warn('productionTrendStats not available.');
    charts.productionTrend.data.labels = [];
    charts.productionTrend.data.datasets[0].data = [];
    charts.productionTrend.data.datasets[1].data = [];
    charts.productionTrend.update('none');
    return;
  }

  if (!productionTrendStats.data || productionTrendStats.data.length === 0) {
    console.warn('Production trend data is empty.');
    charts.productionTrend.data.labels = ['No Data'];
    charts.productionTrend.data.datasets[0].data = [0];
    charts.productionTrend.data.datasets[1].data = [0];
    charts.productionTrend.update('none');
    return;
  }

  try {
    // Handle new API response structure with work_hours
    let trendData = productionTrendStats.data || productionTrendStats;
    let workHours = null;
    let viewType = 'hourly';

    // Check if response has new structure (data + work_hours)
    if (productionTrendStats && typeof productionTrendStats === 'object' && productionTrendStats.data) {
      trendData = productionTrendStats.data;
      workHours = productionTrendStats.work_hours;
      viewType = productionTrendStats.view_type || 'hourly';
    }

    console.log('Production trend original data:', trendData);
    console.log('View type:', viewType);
    console.log('Work hours info:', workHours);

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

    // Map production values to labels
    const actualOutputMap = {};
    const targetOutputMap = {};

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
        actualOutputMap[labelKey] = parseInt(item.actual_output) || 0;
        targetOutputMap[labelKey] = parseInt(item.target_output) || 0;
      }
    });

    // Fill in production values array with 0 for missing hours
    const actualOutput = labels.map(label => actualOutputMap[label] || 0);
    const targetOutput = labels.map(label => targetOutputMap[label] || 0);

    console.log('Production chart labels:', labels);
    console.log('Actual output:', actualOutput);
    console.log('Target output:', targetOutput);

    // Update x-axis title based on view type
    const xAxisTitle = viewType === 'hourly' ? 'Time (within 1 day)' : 'Date (more than 1 day)';

    // Update chart data
    charts.productionTrend.data.labels = labels;
    charts.productionTrend.data.datasets[0].data = actualOutput;
    charts.productionTrend.data.datasets[1].data = targetOutput;

    // Update x-axis title
    charts.productionTrend.options.scales.x.title.text = xAxisTitle;

    // Redraw chart with smooth animation
    charts.productionTrend.update('show');

    console.log(`Production trend chart update completed: ${labels.length} points (${viewType} view)`);

  } catch (error) {
    console.error('Production trend chart update error:', error);
  }
}

function updateMachineOeeChart(machineOeeStats) {
  if (!charts.machineOee || !machineOeeStats || !Array.isArray(machineOeeStats) || machineOeeStats.length === 0) {
    return;
  }

  try {
    // Determine if showing by line or machine
    const isMachineView = machineOeeStats.length > 0 && machineOeeStats[0].hasOwnProperty('machine_no');

    // Extract labels (line_name or machine_no)
    const labels = machineOeeStats.map(item => {
      if (isMachineView) {
        return item.machine_no || 'Unknown Machine';
      } else {
        return item.line_name || 'Unknown Line';
      }
    });

    const availability = machineOeeStats.map(item => parseFloat(item.availability) || 0);
    const performance = machineOeeStats.map(item => parseFloat(item.performance) || 0);
    const quality = machineOeeStats.map(item => parseFloat(item.quality) || 0);

    // Update chart data
    charts.machineOee.data.labels = labels;
    charts.machineOee.data.datasets[0].data = availability;
    charts.machineOee.data.datasets[1].data = performance;
    charts.machineOee.data.datasets[2].data = quality;

    // Update X-axis title dynamically
    const xAxisTitle = isMachineView ? 'Machine Number' : 'Line Name';
    charts.machineOee.options.scales.x.title.text = xAxisTitle;

    // Update chart title and subtitle dynamically
    const chartTitleElement = document.querySelector('.oee-additional-charts.oee-charts-full .fiori-card__title');
    const chartSubtitleElement = document.querySelector('.oee-additional-charts.oee-charts-full .fiori-card__subtitle');

    if (chartTitleElement && chartSubtitleElement) {
      if (isMachineView) {
        chartTitleElement.innerHTML = '🔧 Machine OEE Performance';
        chartSubtitleElement.textContent = 'OEE performance comparison by machine';
      } else {
        chartTitleElement.innerHTML = '🏭 Line OEE Performance';
        chartSubtitleElement.textContent = 'OEE performance comparison by production line';
      }
    }

    charts.machineOee.update('none');

  } catch (error) {
    console.error('Line/Machine OEE chart update error:', error);
  }
}

function updateOeeGradeChart(statsData) {
  if (!charts.oeeGrade || !statsData) {
    return;
  }

  try {
    const excellentCount = parseInt(statsData.excellent_count) || 0;
    const goodCount = parseInt(statsData.good_count) || 0;
    const fairCount = parseInt(statsData.fair_count) || 0;
    const poorCount = parseInt(statsData.poor_count) || 0;

    charts.oeeGrade.data.datasets[0].data = [excellentCount, goodCount, fairCount, poorCount];
    charts.oeeGrade.update('none');

  } catch (error) {
    console.error('OEE grade chart update error:', error);
  }
}

function updateEfficiencyMatrixChart(machineOeeStats) {
  if (!charts.efficiencyMatrix || !machineOeeStats || !Array.isArray(machineOeeStats) || machineOeeStats.length === 0) {
    return;
  }

  try {
    const scatterData = machineOeeStats.map(item => {
      return {
        x: parseFloat(item.performance) || 0,
        y: parseFloat(item.quality) || 0,
        r: parseFloat(item.overall_oee) || 0,
        machine_no: item.machine_no
      };
    });

    charts.efficiencyMatrix.data.datasets[0].data = scatterData;
    charts.efficiencyMatrix.update('none');

  } catch (error) {
    console.error('Efficiency matrix chart update error:', error);
  }
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

  document.getElementById('connectionStatus').textContent = 'OEE System Connection Closed';

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
    console.log('⚠️ OEE: 페이지 언로드 중 - 재연결 취소');
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
      console.log('⚠️ OEE: 페이지 언로드 중 - 재연결 취소');
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
    const exportUrl = `proc/data_oee_export.php?${params.toString()}`;

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

// Modal Functions
function openOeeDetailModal(buttonElement) {
  try {
    const oeeDataJson = buttonElement.getAttribute('data-oee-data');
    if (!oeeDataJson) {
      console.error('OEE data not found.');
      return;
    }

    const oeeData = JSON.parse(oeeDataJson);
    populateOeeModal(oeeData);

    const modal = document.getElementById('oeeDetailModal');
    if (modal) {
      modal.classList.add('show');
      document.body.style.overflow = 'hidden';
    } else {
      console.error('Modal element not found.');
    }

  } catch (error) {
    console.error('Modal open error:', error);
  }
}

function closeOeeDetailModal() {
  const modal = document.getElementById('oeeDetailModal');
  if (modal) {
    modal.classList.add('hide');

    setTimeout(() => {
      modal.classList.remove('show', 'hide');
      document.body.style.overflow = '';
    }, 300);
  }
}

function populateOeeModal(oeeData) {
  try {
    setModalValue('modal-machine-no', oeeData.machine_no || '-');
    setModalValue('modal-factory-line', `${oeeData.factory_name || '-'} / ${oeeData.line_name || '-'}`);
    setModalValue('modal-work-date', oeeData.work_date || '-');

    const shiftDisplay = oeeData.shift_idx ? `Shift ${oeeData.shift_idx}` : '-';
    setModalValue('modal-shift', shiftDisplay);

    setModalValue('modal-overall-oee', formatPercentage(oeeData.overall_oee || 0));
    setModalValue('modal-availability', formatPercentage(oeeData.availability || 0));
    setModalValue('modal-performance', formatPercentage(oeeData.performance || 0));
    setModalValue('modal-quality', formatPercentage(oeeData.quality || 0));

    setModalValue('modal-planned-time', oeeData.planned_time || '-');
    setModalValue('modal-runtime', oeeData.runtime || '-');
    setModalValue('modal-downtime', oeeData.downtime || '-');
    setModalValue('modal-actual-output', oeeData.actual_output || '-');

    setModalValue('modal-theoretical-output', oeeData.theoretical_output ? formatDecimal(oeeData.theoretical_output, 2) : '-');
    setModalValue('modal-defective', oeeData.defective_count || '-');
    setModalValue('modal-cycletime', oeeData.cycle_time || '-');
    setModalValue('modal-update-time', oeeData.update_date || '-');

  } catch (error) {
    console.error('Modal data population error:', error);
  }
}

function setModalValue(elementId, value) {
  const element = document.getElementById(elementId);
  if (element) {
    element.textContent = value;
  }
}

function exportSingleOee() {
  closeOeeDetailModal();
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
  const chartSections = document.querySelectorAll('.oee-main-layout, .oee-additional-charts');
  const oeeChartsRow = document.getElementById('oeeChartsRow');
  const toggleBtn = document.getElementById('toggleChartsBtn');

  if (!toggleBtn) {
    console.error('Toggle charts button not found.');
    return;
  }

  if (chartSections.length === 0) {
    console.error('Chart sections not found.');
    return;
  }

  const isHidden = chartSections[0].classList.contains('hidden');

  if (isHidden) {
    // Show charts
    chartSections.forEach(section => {
      section.classList.remove('hidden');
    });
    if (oeeChartsRow) oeeChartsRow.classList.remove('hidden');
    toggleBtn.textContent = '📈 Hide Charts';
  } else {
    // Hide charts
    chartSections.forEach(section => {
      section.classList.add('hidden');
    });
    if (oeeChartsRow) oeeChartsRow.classList.add('hidden');
    toggleBtn.textContent = '📈 Show Charts';
  }

  // Chart.js 리사이즈 (레이아웃 변경 후)
  setTimeout(function () {
    Object.values(charts).forEach(function (c) { if (c) c.resize(); });
  }, 50);
}

/**
 * Toggle data table display
 */
function toggleDataDisplay() {
  const realtimeCard = document.querySelector('.oee-realtime-card');
  const paginationControls = document.getElementById('pagination-controls');
  const manageMain = document.querySelector('.manage-main');
  const toggleBtn = document.getElementById('toggleDataBtn');

  if (!realtimeCard || !toggleBtn) return;

  const isHidden = realtimeCard.classList.contains('hidden');

  if (isHidden) {
    realtimeCard.classList.remove('hidden');
    if (paginationControls) paginationControls.classList.remove('hidden');
    manageMain.classList.remove('table-hidden');
    toggleBtn.textContent = 'Hide Table';
  } else {
    realtimeCard.classList.add('hidden');
    if (paginationControls) paginationControls.classList.add('hidden');
    manageMain.classList.add('table-hidden');
    toggleBtn.textContent = 'Show Table';
  }

  // Chart.js 4.x는 ResizeObserver 기반이므로 window resize 이벤트로 리사이즈 트리거
  requestAnimationFrame(function () {
    Object.values(charts).forEach(function (c) {
      if (c && c.canvas) {
        c.canvas.style.height = '';
        c.canvas.style.width = '';
      }
    });
    window.dispatchEvent(new Event('resize'));
  });
}

document.addEventListener('keydown', function (event) {
  if (event.key === 'Escape') {
    const modal = document.getElementById('oeeDetailModal');
    if (modal && modal.classList.contains('show')) {
      closeOeeDetailModal();
    }
  }
});

window.openOeeDetailModal = openOeeDetailModal;
window.closeOeeDetailModal = closeOeeDetailModal;
window.exportSingleOee = exportSingleOee;