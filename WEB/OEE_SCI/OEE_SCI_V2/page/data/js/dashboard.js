/**
 * Dashboard JavaScript Module
 */

const chartColors = {
  primary: '#0070f2',
  success: '#30914c',
  warning: '#e26b0a',
  error: '#da1e28',
  info: '#7c3aed',
  light: '#f8f9fa',
  secondary: '#1e88e5',
  accent: '#00d4aa'
};

let dashboardData = {};
let gaugeCharts = {};
let eventSource = null;
let isConnected = false;

let reconnectAttempts = 0;
let maxReconnectAttempts = 3;

// Andon 알람 피드 실시간 업데이트를 위한 타이머
let alarmFeedTimer = null;
let currentAlarms = [];

// 페이지 언로드 상태 추적
let isPageUnloading = false;

class OEEGaugeChart {
  constructor(canvasId, value = 0, label = '', color = chartColors.primary) {
    this.canvas = document.getElementById(canvasId);
    if (!this.canvas) return;

    this.ctx = this.canvas.getContext('2d');
    this.value = value;
    this.label = label;
    this.color = color;
    this.chart = this.createChart();
  }

  createChart() {
    return new Chart(this.ctx, {
      type: 'doughnut',
      data: {
        datasets: [{
          data: [this.value, 100 - this.value],
          backgroundColor: [this.color, 'rgba(200, 200, 200, 0.2)'],
          borderWidth: 0,
          cutout: '75%'
        }]
      },
      options: {
        responsive: true,
        maintainAspectRatio: false,
        plugins: {
          legend: { display: false },
          tooltip: { enabled: false }
        },
        elements: {
          arc: {
            borderRadius: 8
          }
        }
      },
      plugins: [{
        afterDraw: (chart) => {
          const ctx = chart.ctx;
          const centerX = (chart.chartArea.left + chart.chartArea.right) / 2;
          const centerY = (chart.chartArea.top + chart.chartArea.bottom) / 2;

          ctx.save();
          ctx.textAlign = 'center';
          ctx.textBaseline = 'middle';

          // 값 표시 — 차트 크기에 따라 폰트 동적 조정
          const radius = (chart.chartArea.right - chart.chartArea.left) / 2;
          const fontSize = Math.max(12, Math.min(28, radius * 0.50));
          ctx.font = `bold ${fontSize}px Arial`;
          ctx.fillStyle = this.color;
          ctx.fillText(`${this.value}%`, centerX, centerY);

          ctx.restore();
        }
      }]
    });
  }

  updateValue(newValue) {
    this.value = newValue >= 100 ? 100 : Math.floor(newValue * 10) / 10;
    this.chart.data.datasets[0].data = [this.value, 100 - this.value];
    this.chart.update('none');
  }
}

class DashboardManager {
  constructor() {
    this.initializeEventListeners();
    this.initFilterSystem();
    this.initDateRangePicker();
    this.connectToStream();
  }

  initDateRangePicker() {
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
          'last month': [moment().subtract(29, 'days'), moment()]
        },
        startDate: moment().subtract(6, 'days'),
        endDate: moment(),
        showDropdowns: true,
        showWeekNumbers: true,
        alwaysShowCalendars: true
      }, (start, end, label) => {
        this.handleDateRangeChange(start, end, label);
      });

      const initialStart = moment().startOf('day');
      const initialEnd = moment().endOf('day');
      $('#dateRangePicker').val(initialStart.format('YYYY-MM-DD') + ' ~ ' + initialEnd.format('YYYY-MM-DD'));

    } catch (error) {
    }
  }

  async initFilterSystem() {
    try {
      await this.loadFactoryOptions();
      await this.loadLineOptions();
      await this.loadMachineOptions();
      this.setupFilterEventListeners();
    } catch (error) {
    }
  }

  async loadFactoryOptions() {
    try {
      const response = await fetch('../manage/proc/factory.php');
      const res = await response.json();

      if (res.success && res.data) {
        const factorySelect = document.getElementById('factoryFilterSelect');

        if (factorySelect) {
          factorySelect.innerHTML = '<option value="">All Factory</option>';
          res.data.forEach(factory => {
            factorySelect.innerHTML += `<option value="${factory.idx}">${factory.factory_name}</option>`;
          });
        }
      } else {
      }
    } catch (error) {
    }
  }

  async loadLineOptions() {
    try {
      const response = await fetch('../manage/proc/line.php');
      const res = await response.json();

      if (res.success && res.data) {
        const lineSelect = document.getElementById('factoryLineFilterSelect');

        if (lineSelect) {
          lineSelect.innerHTML = '<option value="">All Line</option>';
          res.data.forEach(line => {
            lineSelect.innerHTML += `<option value="${line.idx}">${line.line_name}</option>`;
          });
        }
      } else {
      }
    } catch (error) {
    }
  }

  async loadMachineOptions() {
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
      }
    } catch (error) {
    }
  }

  setupFilterEventListeners() {
    const factorySelect = document.getElementById('factoryFilterSelect');
    if (factorySelect) {
      factorySelect.addEventListener('change', async (e) => {
        const machineSelect = document.getElementById('factoryLineMachineFilterSelect');
        machineSelect.innerHTML = '<option value="">All Machine</option>';
        machineSelect.disabled = true;

        await this.updateLineOptions(e.target.value);
        this.restartDashboardStream();
      });
    }

    const lineSelect = document.getElementById('factoryLineFilterSelect');
    if (lineSelect) {
      lineSelect.addEventListener('change', async (e) => {
        const factoryId = document.getElementById('factoryFilterSelect').value;
        await this.updateMachineOptions(factoryId, e.target.value);
        this.restartDashboardStream();
      });
    }

    const machineSelect = document.getElementById('factoryLineMachineFilterSelect');
    if (machineSelect) {
      machineSelect.addEventListener('change', () => {
        this.restartDashboardStream();
      });
    }

    const timeRangeSelect = document.getElementById('timeRangeSelect');
    if (timeRangeSelect) {
      timeRangeSelect.addEventListener('change', this.handleTimeRangeChange.bind(this));
    }

    const shiftSelect = document.getElementById('shiftSelect');
    if (shiftSelect) {
      shiftSelect.addEventListener('change', () => {
        this.restartDashboardStream();
      });
    }

    const refreshBtn = document.getElementById('refreshBtn');
    if (refreshBtn) {
      refreshBtn.addEventListener('click', () => {
        this.restartDashboardStream();
      });
    }
  }

  async updateLineOptions(factoryId) {
    const lineSelect = document.getElementById('factoryLineFilterSelect');
    lineSelect.disabled = true;

    try {
      let url = '../manage/proc/line.php';
      if (factoryId) {
        url += `?factory_filter=${factoryId}`;
      }

      const response = await fetch(url);
      const res = await response.json();

      if (res.success) {
        lineSelect.innerHTML = '<option value="">All Line</option>';
        res.data.forEach(line => {
          lineSelect.innerHTML += `<option value="${line.idx}">${line.line_name}</option>`;
        });
      }

      lineSelect.disabled = false;

      const machineSelect = document.getElementById('factoryLineMachineFilterSelect');
      if (factoryId) {
        machineSelect.disabled = false;
        this.updateMachineOptions(factoryId, '');
      }
    } catch (error) {
      lineSelect.innerHTML = '<option value="">All Line</option>';
      lineSelect.disabled = false;
    }
  }

  async updateMachineOptions(factoryId, lineId) {
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
      }

      machineSelect.disabled = false;
    } catch (error) {
      machineSelect.innerHTML = '<option value="">All Machine</option>';
      machineSelect.disabled = false;
    }
  }

  async handleDateRangeChange(startDate, endDate, label) {
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

    this.restartDashboardStream();
  }

  async handleTimeRangeChange(event) {
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

    this.restartDashboardStream();
  }

  getFilterParams() {
    const factoryFilter = document.getElementById('factoryFilterSelect')?.value || '';
    const lineFilter = document.getElementById('factoryLineFilterSelect')?.value || '';
    const machineFilter = document.getElementById('factoryLineMachineFilterSelect')?.value || '';
    const shiftFilter = document.getElementById('shiftSelect')?.value || '';

    const dateRange = $('#dateRangePicker').val();
    let startDate = '', endDate = '';

    if (dateRange && dateRange.includes(' ~ ')) {
      const dates = dateRange.split(' ~ ');
      startDate = dates[0];
      endDate = dates[1];
    }

    return {
      factory_filter: factoryFilter,
      line_filter: lineFilter,
      machine_filter: machineFilter,
      shift_filter: shiftFilter,
      start_date: startDate,
      end_date: endDate
    };
  }

  restartDashboardStream() {
    if (eventSource) {
      eventSource.close();
      isConnected = false;
    }

    setTimeout(() => {
      this.connectToStream();
    }, 100);
  }

  initializeEventListeners() {
    // 페이지 언로드 시 SSE 연결 및 타이머 종료
    // 우선순위: beforeunload > pagehide > visibilitychange

    // 1. beforeunload - 가장 먼저 실행되어 페이지 언로드 플래그 설정
    window.addEventListener('beforeunload', () => {
      console.log('Dashboard: beforeunload - 페이지 언로드 시작');
      isPageUnloading = true; // 플래그 설정으로 재연결 방지

      if (eventSource) {
        eventSource.close();
        eventSource = null;
        isConnected = false;
      }
      if (alarmFeedTimer) {
        clearInterval(alarmFeedTimer);
        alarmFeedTimer = null;
      }
    });

    // 2. pagehide - 브라우저가 페이지를 숨길 때 (모바일에서 중요)
    window.addEventListener('pagehide', () => {
      console.log('Dashboard: pagehide - 페이지 숨김');
      isPageUnloading = true;

      if (eventSource) {
        eventSource.close();
        eventSource = null;
        isConnected = false;
      }
      if (alarmFeedTimer) {
        clearInterval(alarmFeedTimer);
        alarmFeedTimer = null;
      }
    });

    // 3. visibilitychange - 탭 전환이나 페이지 숨김
    document.addEventListener('visibilitychange', () => {
      if (document.hidden) {
        // 페이지가 숨겨질 때 SSE 연결과 타이머를 즉시 정리
        console.log('Dashboard: 페이지 숨김 - SSE 연결 및 타이머 종료');

        if (eventSource) {
          eventSource.close();
          eventSource = null;
          isConnected = false;
        }
        if (alarmFeedTimer) {
          clearInterval(alarmFeedTimer);
          alarmFeedTimer = null;
        }
      } else {
        // 페이지가 다시 보일 때만 재연결 (언로드 중이 아닐 때만)
        console.log('Dashboard: 페이지 표시 - SSE 재연결 시도');

        if (!isPageUnloading && !isConnected) {
          this.reconnectToStream();
        }
      }
    });

    // 4. 페이지 포커스 복원 시 - 언로드 플래그 리셋
    window.addEventListener('focus', () => {
      if (isPageUnloading) {
        console.log('Dashboard: 페이지 포커스 - 언로드 플래그 리셋');
        isPageUnloading = false;
      }
    });
  }

  connectToStream() {
    if (eventSource) {
      eventSource.close();
    }

    const filters = this.getFilterParams();
    const params = new URLSearchParams(filters);
    const streamUrl = `proc/dashboard_stream.php?${params.toString()}`;

    eventSource = new EventSource(streamUrl);

    eventSource.onopen = () => {
      isConnected = true;
      this.updateConnectionStatus('실시간 데이터 연결됨');
    };

    eventSource.onmessage = (event) => {
      try {
        const data = JSON.parse(event.data);
        this.handleDataUpdate(data);
      } catch (error) {
      }
    };

    eventSource.onerror = () => {
      // 페이지가 언로드 중이면 재연결하지 않음
      if (isPageUnloading) {
        console.log('⚠️ Dashboard: 페이지 언로드 중 - 재연결 취소');
        return;
      }

      isConnected = false;
      this.updateConnectionStatus('연결 재시도 중...');

      setTimeout(() => {
        // 재연결 전에 다시 한번 페이지 상태 확인
        if (isPageUnloading) {
          console.log('⚠️ Dashboard: 페이지 언로드 중 - 재연결 취소');
          return;
        }

        if (!isConnected) {
          this.reconnectToStream();
        }
      }, 5000);
    };
  }

  reconnectToStream() {
    // 페이지가 언로드 중이면 재연결하지 않음
    if (isPageUnloading) {
      console.log('⚠️ Dashboard: 페이지 언로드 중 - 재연결 취소');
      return;
    }

    this.connectToStream();
  }

  updateConnectionStatus(message) {
    const statusElements = document.querySelectorAll('.real-time-status span:not(.status-dot)');
    statusElements.forEach(el => {
      if (el.textContent.includes('연결') || el.textContent.includes('재시도')) {
        el.textContent = message;
      }
    });
  }

  handleDataUpdate(data) {
    dashboardData = data;

    if (data.oee) {
      this.updateOEEMetrics(data.oee);
    }

    if (data.andon) {
      this.updateAndonCharts(data.andon);
    }

    if (data.downtime) {
      this.updateDowntimeChart(data.downtime);
    }

    if (data.defective) {
      this.updateDefectiveChart(data.defective);
    }

    if (data.production) {
      this.updateProductionAnalysis(data.production);
    }

    this.updateLastUpdateTime();
  }

  // OEE 메트릭 업데이트
  updateOEEMetrics(oeeData) {
    // 게이지 차트 업데이트
    if (gaugeCharts.availability && oeeData.availability !== undefined) {
      gaugeCharts.availability.updateValue(oeeData.availability.value || 0);
    }
    if (gaugeCharts.performance && oeeData.performance !== undefined) {
      gaugeCharts.performance.updateValue(oeeData.performance.value || 0);
    }
    if (gaugeCharts.quality && oeeData.quality !== undefined) {
      gaugeCharts.quality.updateValue(oeeData.quality.value || 0);
    }
    if (gaugeCharts.overall && oeeData.overall !== undefined) {
      gaugeCharts.overall.updateValue(oeeData.overall.value || 0);
    }

    // 상세 메트릭 값 업데이트
    this.updateMetricValues(oeeData);
  }

  // 메트릭 값들 업데이트
  updateMetricValues(oeeData) {
    const updateElement = (id, value, suffix = '') => {
      const element = document.getElementById(id);
      if (element && value !== undefined) {
        let displayValue = value;
        if (suffix === '%' && typeof value === 'number') {
          displayValue = value >= 100 ? 100 : Math.floor(value * 10) / 10;
        }
        element.textContent = displayValue + suffix;
      }
    };

    // Availability 메트릭
    if (oeeData.availability) {
      updateElement('runtime-value', oeeData.availability.runtime, '%');
      updateElement('planned-time-value', oeeData.availability.planned_time, '%');
      updateElement('availabilityTrend', oeeData.availability.trend || '→');
      updateElement('availabilityChange',
        `${oeeData.availability.change > 0 ? '+' : ''}${oeeData.availability.change || 0}% vs Last Day`);

      // Progress bar 업데이트
      const runtimeProgress = document.getElementById('runtime-progress');
      if (runtimeProgress) {
        runtimeProgress.style.width = `${oeeData.availability.runtime || 0}%`;
      }
    }

    // Performance 메트릭
    if (oeeData.performance) {
      updateElement('actual-output-value', oeeData.performance.actual_output || 0);
      updateElement('theoretical-output-value', oeeData.performance.theoretical_output || 0);
      updateElement('performanceTrend', oeeData.performance.trend || '→');
      updateElement('performanceChange',
        `${oeeData.performance.change > 0 ? '+' : ''}${oeeData.performance.change || 0}% vs Last Day`);
    }

    // Quality 메트릭
    if (oeeData.quality) {
      updateElement('good-products-value', oeeData.quality.good_products || 0);
      updateElement('defective-products-value', oeeData.quality.defective_products || 0);
      updateElement('qualityTrend', oeeData.quality.trend || '→');
      updateElement('qualityChange',
        `${oeeData.quality.change > 0 ? '+' : ''}${oeeData.quality.change || 0}% vs Last Day`);
    }

    // Overall OEE
    if (oeeData.overall) {
      updateElement('overallTrend', oeeData.overall.trend || '→');
      updateElement('overallChange',
        `${oeeData.overall.change > 0 ? '+' : ''}${oeeData.overall.change || 0}% vs Last Day`);
    }
  }

  // Andon 차트 업데이트
  updateAndonCharts(andonData) {
    // Andon 발생수량 차트 업데이트 (data_andon.js와 일관성 유지)
    if (window.andonOccurrenceChart && andonData.occurrence) {
      // andon_name으로 정렬 (알파벳 순)
      const sortedStats = [...andonData.occurrence].sort((a, b) => {
        const nameA = (a.andon_name || 'Unclassified').toLowerCase();
        const nameB = (b.andon_name || 'Unclassified').toLowerCase();
        return nameA.localeCompare(nameB);
      });

      // 라벨, 카운트, 색상 추출
      const labels = sortedStats.map(item => item.andon_name || 'Unclassified');
      const counts = sortedStats.map(item => parseInt(item.count) || 0);
      const colors = sortedStats.map(item => item.andon_color);

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
      window.andonOccurrenceChart.data.labels = labels;
      window.andonOccurrenceChart.data.datasets[0].data = counts;
      window.andonOccurrenceChart.data.datasets[0].backgroundColor = backgroundColors;
      window.andonOccurrenceChart.data.datasets[0].borderColor = borderColors;
      window.andonOccurrenceChart.update('none');
    }

    // 주간 Andon 추이 차트 업데이트 (동적 Andon 유형 기반)
    if (window.weeklyAndonTrendChart && andonData.weekly && andonData.andon_types) {
      // andon_types가 변경되었는지 확인 (차트 재구성 필요)
      if (!this.lastAndonTypes || JSON.stringify(this.lastAndonTypes) !== JSON.stringify(andonData.andon_types)) {
        this.lastAndonTypes = andonData.andon_types;
        this.rebuildWeeklyAndonChart(andonData);
        return;
      }

      // 기존 차트 업데이트 (andon_types 동일한 경우)
      const datasets = window.weeklyAndonTrendChart.data.datasets;

      // 각 Andon 유형별 데이터 업데이트
      andonData.andon_types.forEach((andon, index) => {
        const idx = parseInt(andon.idx);
        if (andonData.weekly[idx] && datasets[index]) {
          datasets[index].data = andonData.weekly[idx];
          // 색상 업데이트
          if (andonData.weekly_colors && andonData.weekly_colors[idx]) {
            datasets[index].backgroundColor = `${andonData.weekly_colors[idx]}80`;
            datasets[index].borderColor = andonData.weekly_colors[idx];
          }
        }
      });

      // Total 데이터 업데이트
      const totalIndex = andonData.andon_types.length;
      if (andonData.weekly.total && datasets[totalIndex]) {
        datasets[totalIndex].data = andonData.weekly.total;
      }

      window.weeklyAndonTrendChart.update('none');
    }

    // 실시간 알람 피드 업데이트
    if (andonData.alarms) {
      this.updateAlarmFeed(andonData.alarms);
    }
  }

  // 실시간 알람 피드 업데이트 (data_andon.js의 updateActiveAndonsDisplay 스타일 적용)
  updateAlarmFeed(alarms) {
    const feedContainer = document.getElementById('andonAlarmFeed');
    if (!feedContainer) return;

    // activeAndonCount 업데이트
    const countDisplay = document.getElementById('activeAndonCount');
    if (countDisplay) {
      countDisplay.textContent = `${alarms ? alarms.length : 0} active alerts`;
    }

    // 전역 변수에 알람 데이터 저장 (타이머 업데이트용)
    currentAlarms = alarms || [];

    // 알람이 없는 경우 친절한 메시지 표시
    if (!alarms || alarms.length === 0) {
      feedContainer.innerHTML = `
        <div class="fiori-alert fiori-alert--success" style="margin: 0;">
          <strong>✅ Good:</strong> There are currently no active Andon alarms. All systems are operating normally.
        </div>
      `;
      // 타이머 정지
      this.stopAlarmFeedTimer();
      return;
    }

    // 알람 아이템 생성 (data_andon.js의 active-andon-item 구조 완전 동일하게 적용)
    let html = '';
    alarms.forEach(alarm => {
      // 안돈 색상을 테두리와 배경색에 적용 (기본값: SAP 브랜드 색상)
      const borderColor = alarm.andon_color || '#0070f2';
      const backgroundColor = alarm.andon_color ? `${alarm.andon_color}1A` : '#0070f21A';
      const borderStyle = `border: 2px dashed ${borderColor}; background-color: ${backgroundColor}; border-radius: var(--sap-radius-md); padding: var(--sap-spacing-md); margin-bottom: var(--sap-spacing-sm); box-shadow: 0 2px 8px ${backgroundColor};`;

      // 시간 표시
      const timeDisplay = this.getTimeAgo(alarm.timestamp);

      html += `
        <div class="active-andon-item" style="${borderStyle}" data-alarm-id="${alarm.id}" data-timestamp="${alarm.timestamp}">
          <div class="andon-machine-info">
            <div class="andon-machine-name">${alarm.andon_name}</div>
            <div class="andon-location">
              <span class="andon-location-item">${alarm.factory_name}</span>
              <span class="andon-location-item">/</span>
              <span class="andon-location-item">${alarm.line_name}</span>
              <span class="andon-location-item">/</span>
              <span class="andon-location-item">${alarm.machine_no}</span>
            </div>
          </div>
          <div class="andon-elapsed-time">
            <span class="alarm-time-display">⏰ ${timeDisplay}</span>
          </div>
        </div>
      `;
    });

    feedContainer.innerHTML = html;

    // 실시간 타이머 시작
    this.startAlarmFeedTimer();
  }

  // 알람 피드 시간 실시간 업데이트
  updateAlarmFeedTimes() {
    const alarmItems = document.querySelectorAll('.active-andon-item[data-alarm-id]');

    alarmItems.forEach((item) => {
      const timestamp = parseInt(item.getAttribute('data-timestamp'));
      if (timestamp) {
        const timeDisplay = item.querySelector('.alarm-time-display');
        if (timeDisplay) {
          timeDisplay.textContent = this.getTimeAgo(timestamp);
        }
      }
    });
  }

  // 알람 피드 타이머 시작
  startAlarmFeedTimer() {
    // 기존 타이머가 있으면 정리
    if (alarmFeedTimer) {
      clearInterval(alarmFeedTimer);
    }

    // 1초마다 시간 업데이트
    alarmFeedTimer = setInterval(() => {
      this.updateAlarmFeedTimes();
    }, 1000);
  }

  // 알람 피드 타이머 정지
  stopAlarmFeedTimer() {
    if (alarmFeedTimer) {
      clearInterval(alarmFeedTimer);
      alarmFeedTimer = null;
    }
  }

  // Downtime 차트 업데이트 (data_downtime.js와 일관성 유지)
  updateDowntimeChart(downtimeData) {
    if (window.downtimeOccurrenceChart && downtimeData.occurrence) {
      // downtime_name으로 정렬 (알파벳 순)
      const sortedStats = [...downtimeData.occurrence].sort((a, b) => {
        const nameA = (a.downtime_name || 'Unclassified').toLowerCase();
        const nameB = (b.downtime_name || 'Unclassified').toLowerCase();
        return nameA.localeCompare(nameB);
      });

      // 라벨과 지속시간(분) 추출
      const labels = sortedStats.map(item => item.downtime_name || 'Unclassified');
      const durations = sortedStats.map(item => parseFloat(item.total_duration_min) || 0);

      // 색상 배열 순환하여 사용 (data_downtime.js 패턴)
      const backgroundColors = [];
      const borderColors = [];
      const colorKeys = ['warning', 'error', 'primary', 'info', 'accent', 'secondary'];

      for (let i = 0; i < labels.length; i++) {
        const colorKey = colorKeys[i % colorKeys.length];
        backgroundColors.push(chartColors[colorKey]);
        borderColors.push(chartColors[colorKey]);
      }

      // 차트 데이터 업데이트
      window.downtimeOccurrenceChart.data.labels = labels;
      window.downtimeOccurrenceChart.data.datasets[0].data = durations; // 분 단위 지속시간
      window.downtimeOccurrenceChart.data.datasets[0].backgroundColor = backgroundColors;
      window.downtimeOccurrenceChart.data.datasets[0].borderColor = borderColors;
      window.downtimeOccurrenceChart.update('none');
    }
  }

  // Defective 차트 업데이트 (data_downtime.js와 일관성 유지)
  updateDefectiveChart(defectiveData) {
    if (window.defectiveOccurrenceChart && defectiveData.occurrence) {
      // defective_name으로 정렬 (알파벳 순)
      const sortedStats = [...defectiveData.occurrence].sort((a, b) => {
        const nameA = (a.defective_name || 'Unclassified').toLowerCase();
        const nameB = (b.defective_name || 'Unclassified').toLowerCase();
        return nameA.localeCompare(nameB);
      });

      // 라벨과 카운트 추출
      const labels = sortedStats.map(item => item.defective_name || 'Unclassified');
      const counts = sortedStats.map(item => parseInt(item.count) || 0);

      // 색상 배열 순환하여 사용 (data_downtime.js 패턴)
      const backgroundColors = [];
      const borderColors = [];
      const colorKeys = ['warning', 'error', 'primary', 'info', 'accent', 'secondary'];

      for (let i = 0; i < labels.length; i++) {
        const colorKey = colorKeys[i % colorKeys.length];
        backgroundColors.push(chartColors[colorKey]);
        borderColors.push(chartColors[colorKey]);
      }

      // 차트 데이터 업데이트
      window.defectiveOccurrenceChart.data.labels = labels;
      window.defectiveOccurrenceChart.data.datasets[0].data = counts;
      window.defectiveOccurrenceChart.data.datasets[0].backgroundColor = backgroundColors;
      window.defectiveOccurrenceChart.data.datasets[0].borderColor = borderColors;
      window.defectiveOccurrenceChart.update('none');
    }
  }

  // Production 분석 업데이트
  updateProductionAnalysis(productionData) {
    // OEE Trend 차트 업데이트
    if (productionData.oee_trend) {
      this.updateOeeTrendChart(productionData.oee_trend);
    }

    // 생산 타임라인 업데이트
    if (productionData.timeline) {
      this.updateProductionTimeline(productionData.timeline, productionData.oee_trend);
    }

    // 생산량 히트맵 업데이트 (실제 요일 레이블 전달)
    if (productionData.heatmap) {
      this.updateProductionHeatmap(productionData.heatmap, productionData.heatmap_labels);
    }

    // Rate color 범례 업데이트 (최초 1회만)
    if (productionData.rate_color_config && !this.rateColorLegendInitialized) {
      this.updateTimelineLegend(productionData.rate_color_config);
      this.rateColorLegendInitialized = true;
    }
  }

  // OEE Trend 차트 업데이트 (data_oee.js의 updateOeeTrendChart와 동일)
  updateOeeTrendChart(oeeTrendStats) {
    if (!window.oeeTrendChart) {
      return;
    }

    if (!oeeTrendStats) {
      window.oeeTrendChart.data.labels = [];
      window.oeeTrendChart.data.datasets[0].data = [];
      window.oeeTrendChart.update('none');
      return;
    }

    if (!oeeTrendStats.data || oeeTrendStats.data.length === 0) {
      window.oeeTrendChart.data.labels = ['No Data'];
      window.oeeTrendChart.data.datasets[0].data = [0];
      window.oeeTrendChart.update('none');
      return;
    }

    try {
      let trendData = oeeTrendStats.data || oeeTrendStats;
      let workHours = null;
      let viewType = 'hourly';

      if (oeeTrendStats && typeof oeeTrendStats === 'object' && oeeTrendStats.data) {
        trendData = oeeTrendStats.data;
        workHours = oeeTrendStats.work_hours;
        viewType = oeeTrendStats.view_type || 'hourly';
      }

      let labels = [];

      if (viewType === 'hourly' && workHours && workHours.start_time && workHours.end_time) {
        // 1일 이하: 근무시간 전체 범위 표시
        labels = this.generateWorkHoursLabels(
          workHours.start_time,
          workHours.end_time,
          workHours.start_minutes,
          workHours.end_minutes
        );
      } else if (viewType === 'daily') {
        // 일별 뷰: display_label 직접 사용
        labels = trendData.map(item => item.display_label || item.time_label);
      } else if (viewType === 'hourly' && !workHours) {
        // 1일 초과, 시간별 평균: "00H", "01H" 형식 직접 사용
        labels = trendData.map(item => {
          const originalLabel = item.time_label || item.display_label;
          // "00H" 형식이면 그대로 사용
          if (originalLabel && originalLabel.match(/^\d{2}H$/)) {
            return originalLabel;
          }
          // datetime 형식이면 변환
          if (originalLabel && originalLabel.includes(':')) {
            const match = originalLabel.match(/^(\d{4}-\d{2}-\d{2})\s+(\d{2}):/);
            if (match) {
              return `${match[2]}H`;
            }
          }
          return originalLabel;
        });
      } else {
        // 기타: datetime을 시간 레이블로 변환
        labels = trendData.map(item => {
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

      // OEE 값을 레이블에 매핑
      const oeeValuesMap = {};

      trendData.forEach(item => {
        let labelKey = '';

        if (viewType === 'daily') {
          labelKey = item.display_label || item.time_label;
        } else {
          const originalLabel = item.time_label || item.display_label;
          // "00H" 형식이면 그대로 사용
          if (originalLabel && originalLabel.match(/^\d{2}H$/)) {
            labelKey = originalLabel;
          } else if (originalLabel && originalLabel.includes(':')) {
            // datetime 형식이면 변환
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

      // 누락된 시간은 0으로 채우기
      const oeeValues = labels.map(label => oeeValuesMap[label] || 0);

      // x축 제목 업데이트
      let xAxisTitle = 'Time';
      if (viewType === 'hourly' && workHours) {
        xAxisTitle = 'Time (within 1 day)';
      } else if (viewType === 'hourly' && !workHours) {
        xAxisTitle = 'Hour (average across all days)';
      } else if (viewType === 'daily') {
        xAxisTitle = 'Date (more than 1 day)';
      }

      // 차트 데이터 업데이트
      window.oeeTrendChart.data.labels = labels;
      window.oeeTrendChart.data.datasets[0].data = oeeValues;
      window.oeeTrendChart.options.scales.x.title.text = xAxisTitle;
      window.oeeTrendChart.update('show');

    } catch (error) {
      console.error('OEE trend chart update error:', error);
    }
  }

  // 근무시간 범위 기반 시간 레이블 생성 (data_oee.js의 generateWorkHoursLabels와 동일)
  generateWorkHoursLabels(startTime, endTime, startMinutes = null, endMinutes = null) {
    const labels = [];

    let startHour, endHour;

    if (startMinutes !== null && endMinutes !== null) {
      startHour = Math.floor(startMinutes / 60);
      endHour = Math.floor(endMinutes / 60);
    } else {
      startHour = parseInt(startTime.split(':')[0]);
      endHour = parseInt(endTime.split(':')[0]);

      if (endHour < startHour) {
        endHour += 24;
      }
    }

    for (let hour = startHour; hour <= endHour; hour++) {
      const displayHour = hour % 24;
      labels.push(`${displayHour.toString().padStart(2, '0')}H`);
    }

    return labels;
  }

  // 생산 타임라인 업데이트
  updateProductionTimeline(timelineData, oeeTrendStats) {
    const timelineBar = document.getElementById('productionTimeline');
    const timelineHeader = document.getElementById('timelineHeader');
    const timelineSubtitle = document.getElementById('timelineSubtitle');
    if (!timelineBar || !timelineData) return;

    // 타임라인 바 세그먼트 업데이트
    timelineBar.innerHTML = timelineData.map(segment => `
      <div class="bar-segment"
           style="width: ${segment.width}%; background: ${segment.color};"
           title="${segment.title}">
      </div>
    `).join('');

    // x축 레이블 동적 생성 (OEE Trend와 일관성 유지)
    if (timelineHeader) {
      this.updateTimelineHeader(timelineHeader, oeeTrendStats);
    }

    // Subtitle 업데이트 (표시 모드에 따라 다르게)
    if (timelineSubtitle) {
      const dateRange = $('#dateRangePicker').val();

      if (dateRange && dateRange.includes(' ~ ')) {
        const dates = dateRange.split(' ~ ');
        const startDate = dates[0];
        const endDate = dates[1];

        // 날짜 차이 계산
        const start = moment(startDate);
        const end = moment(endDate);
        const daysDiff = end.diff(start, 'days');

        if (daysDiff > 1) {
          // 1일 초과: 평균 표시
          timelineSubtitle.textContent = `24-hour average OEE status (${startDate} ~ ${endDate})`;
        } else {
          // 1일 이하: 특정 날짜 표시
          timelineSubtitle.textContent = `24-hour OEE status (${endDate})`;
        }
      } else {
        // 날짜 범위 없음: 오늘 날짜 표시
        const today = moment().format('YYYY-MM-DD');
        timelineSubtitle.textContent = `24-hour OEE status (${today})`;
      }
    }
  }

  // 타임라인 헤더 업데이트 (OEE Trend와 동일한 로직)
  updateTimelineHeader(timelineHeader, oeeTrendStats) {
    if (!oeeTrendStats) {
      // 기본 레이블 (24시간)
      const labels = [];
      for (let hour = 0; hour < 24; hour++) {
        labels.push(`<span style="font-size: var(--sap-font-size-xs);">${hour.toString().padStart(2, '0')}H</span>`);
      }
      timelineHeader.innerHTML = labels.join('');
      return;
    }

    try {
      let workHours = null;
      let viewType = 'hourly';

      if (oeeTrendStats && typeof oeeTrendStats === 'object' && oeeTrendStats.data) {
        workHours = oeeTrendStats.work_hours;
        viewType = oeeTrendStats.view_type || 'hourly';
      }

      let labels = [];

      if (viewType === 'hourly' && workHours && workHours.start_time && workHours.end_time) {
        // 1일 이하: 근무시간 범위로 H 표기 (OEE Trend와 동일)
        const workHourLabels = this.generateWorkHoursLabels(
          workHours.start_time,
          workHours.end_time,
          workHours.start_minutes,
          workHours.end_minutes
        );
        labels = workHourLabels.map(label =>
          `<span style="font-size: var(--sap-font-size-xs);">${label}</span>`
        );
      } else {
        // 기본 24시간 레이블
        for (let hour = 0; hour < 24; hour++) {
          labels.push(`<span style="font-size: var(--sap-font-size-xs);">${hour.toString().padStart(2, '0')}H</span>`);
        }
      }

      timelineHeader.innerHTML = labels.join('');

    } catch (error) {
      console.error('Timeline header update error:', error);
      // 오류 발생 시 기본 레이블
      const labels = [];
      for (let hour = 0; hour < 24; hour++) {
        labels.push(`<span style="font-size: var(--sap-font-size-xs);">${hour.toString().padStart(2, '0')}H</span>`);
      }
      timelineHeader.innerHTML = labels.join('');
    }
  }

  // 생산량 히트맵 업데이트 (6일 x 24시간, OEE Timeline과 동일한 색상 로직)
  updateProductionHeatmap(heatmapData, heatmapLabels) {
    const heatmapGrid = document.getElementById('productionHeatmap');
    if (!heatmapGrid || !heatmapData) return;

    // 서버에서 전달받은 실제 요일 레이블 사용 (없으면 기본값)
    const days = heatmapLabels || ['Mon', 'Tue', 'Wed', 'Thu', 'Fri', 'Sat'];

    // 히트맵 그리드 생성 (6일 x 24시간)
    heatmapGrid.innerHTML = heatmapData.map((dayData, dayIndex) => `
      <div class="heatmap-row">
        <span class="row-label">${days[dayIndex] || ''}</span>
        ${dayData.map((cell, hourIndex) => {
      // 각 셀은 { oee, color, status_text } 구조
      const oeeValue = cell.oee !== null ? Math.round(cell.oee * 10) / 10 : null;
      const titleText = oeeValue !== null
        ? `${days[dayIndex]} ${hourIndex.toString().padStart(2, '0')}H: OEE ${oeeValue}% (${cell.status_text})`
        : `${days[dayIndex]} ${hourIndex.toString().padStart(2, '0')}H: ${cell.status_text}`;

      return `
            <div class="heatmap-cell"
                 style="background-color: ${cell.color};"
                 title="${titleText}">
            </div>
          `;
    }).join('')}
      </div>
    `).join('');

    // 히트맵 시간 축 레이블 업데이트 (0H ~ 23H 균등 배치)
    const heatmapAxis = document.querySelector('.heatmap-axis');
    if (heatmapAxis) {
      // 첫 번째 열은 row-label 자리 (빈 공간, row-label 클래스로 동일한 스타일 적용)
      const axisLabels = ['<span class="row-label"></span>'];

      // 모든 시간 레이블 생성 (0H ~ 23H, heatmap-cell과 동일한 구조)
      for (let hour = 0; hour < 24; hour++) {
        axisLabels.push(`<div class="axis-cell">${hour.toString().padStart(2, '0')}H</div>`);
      }
      heatmapAxis.innerHTML = axisLabels.join('');
    }
  }

  // 시간 경과 표시 (data_andon.js의 calculateElapsedTime 완전 동일하게 적용)
  getTimeAgo(timestamp) {
    const now = Date.now();
    const diff = Math.floor((now - timestamp) / 1000); // 초 단위로 변환

    if (diff < 0) return '0s';

    const hours = Math.floor(diff / 3600);
    const minutes = Math.floor((diff % 3600) / 60);
    const seconds = diff % 60;

    let result = '';
    if (hours > 0) result += hours + 'h ';
    if (minutes > 0) result += minutes + 'm ';
    if (seconds > 0 || result === '') result += seconds + 's';

    return result.trim();
  }

  // 최종 업데이트 시간 갱신
  updateLastUpdateTime() {
    const now = new Date();
    const timeString = now.toLocaleTimeString(undefined, {
      hour: '2-digit',
      minute: '2-digit',
      second: '2-digit',
      timeZoneName: 'short'
    });

    const element = document.getElementById('lastUpdateTime');
    if (element) {
      element.textContent = `Last updated: ${timeString}`;
    }
  }

  // Production Timeline 범례 업데이트 (info_rate_color 테이블 기반 동적 생성)
  updateTimelineLegend(rateColorConfig) {
    const legendContainer = document.querySelector('.production-timeline .timeline-legend');
    if (!legendContainer || !rateColorConfig) return;

    // 범례 초기화
    legendContainer.innerHTML = '';

    // Rate color config를 순회하며 범례 생성 (역순으로 표시 - 높은 값부터)
    const sortedConfig = [...rateColorConfig].sort((a, b) => b.start_rate - a.start_rate);

    sortedConfig.forEach(config => {
      const startRate = parseFloat(config.start_rate);
      const endRate = parseFloat(config.end_rate);
      const color = config.color;

      // 범위 텍스트 생성
      let rangeText = '';
      if (endRate > 100) {
        rangeText = `${startRate}%+`;
      } else if (startRate >= 78) {
        rangeText = `${startRate}-${endRate}%`;
      } else if (startRate >= 60) {
        rangeText = `${startRate}-${endRate}%`;
      } else if (startRate >= 40) {
        rangeText = `${startRate}-${endRate}%`;
      } else {
        rangeText = `${startRate}-${endRate}%`;
      }

      // 범례 아이템 생성
      const legendItem = document.createElement('div');
      legendItem.className = 'legend-item';
      legendItem.innerHTML = `
        <div class="legend-color" style="background: ${color};"></div>
        <span>${rangeText}</span>
      `;

      legendContainer.appendChild(legendItem);
    });

    // "데이터 없음" 범례 추가
    const noDataItem = document.createElement('div');
    noDataItem.className = 'legend-item';
    noDataItem.innerHTML = `
      <div class="legend-color" style="background: #e0e0e0;"></div>
      <span>No data</span>
    `;
    legendContainer.appendChild(noDataItem);
  }

  // Weekly Andon 차트 재구성 (Andon 유형 변경 시)
  rebuildWeeklyAndonChart(andonData) {
    if (!window.weeklyAndonTrendChart || !andonData.andon_types) return;

    // 기존 차트 파괴
    window.weeklyAndonTrendChart.destroy();

    // 새 데이터셋 구성
    const datasets = [];

    // 각 Andon 유형별 데이터셋 추가 (가로 막대)
    andonData.andon_types.forEach(andon => {
      const idx = parseInt(andon.idx);
      const color = andonData.weekly_colors[idx] || '#0070f2';

      datasets.push({
        type: 'bar',
        label: andon.andon_name,
        data: andonData.weekly[idx] || [0, 0, 0, 0, 0, 0, 0],
        backgroundColor: `${color}80`,
        borderColor: color,
        borderWidth: 1
      });
    });

    // Total 데이터셋 추가 (라인)
    datasets.push({
      type: 'line',
      label: 'Total',
      data: andonData.weekly.total || [0, 0, 0, 0, 0, 0, 0],
      borderColor: '#000000',
      backgroundColor: '#000000',
      borderWidth: 2,
      tension: 0.4,
      pointRadius: 4,
      pointHoverRadius: 6,
      order: 1
    });

    // 차트 재생성 (세로 막대 그래프)
    const ctx = document.getElementById('weeklyAndonTrendChart').getContext('2d');
    window.weeklyAndonTrendChart = new Chart(ctx, {
      type: 'bar',
      data: {
        labels: ['Mon', 'Tue', 'Wed', 'Thu', 'Fri', 'Sat', 'Sun'],
        datasets: datasets
      },
      options: {
        responsive: true,
        maintainAspectRatio: false,
        plugins: {
          legend: { display: true, position: 'top' },
          tooltip: {
            mode: 'index',
            intersect: false
          }
        },
        scales: {
          y: {
            beginAtZero: true,
            stacked: false,
            title: { display: true, text: 'Count' },
            ticks: {
              stepSize: 1,
              precision: 0
            }
          },
          x: {
            stacked: false,
            title: { display: true, text: 'Day of Week' }
          }
        }
      }
    });
  }
}

/**
 * 차트 초기화 함수들
 */
function initializeGaugeCharts() {
  // 초기 데이터로 게이지 차트 생성
  gaugeCharts.availability = new OEEGaugeChart(
    'availabilityGauge',
    0,
    'Availability',
    chartColors.success
  );

  gaugeCharts.performance = new OEEGaugeChart(
    'performanceGauge',
    0,
    'Performance',
    chartColors.warning
  );

  gaugeCharts.quality = new OEEGaugeChart(
    'qualityGauge',
    0,
    'Quality',
    '#00d4aa'
  );

  gaugeCharts.overall = new OEEGaugeChart(
    'overallGauge',
    0,
    'Overall OEE',
    chartColors.primary
  );

}

function initializeAndonCharts() {
  // Andon 발생수량 차트 (data_andon.js와 일관성 유지)
  const andonOccurrenceCtx = document.getElementById('andonOccurrenceChart').getContext('2d');
  window.andonOccurrenceChart = new Chart(andonOccurrenceCtx, {
    type: 'bar',
    data: {
      labels: [], // 동적으로 업데이트
      datasets: [{
        label: 'Warning Quantity',
        data: [], // 동적으로 업데이트
        backgroundColor: [], // 동적으로 업데이트
        borderRadius: 6,
        borderWidth: 0
      }]
    },
    options: {
      responsive: true,
      maintainAspectRatio: false,
      plugins: {
        legend: { display: false }
      },
      scales: {
        y: {
          beginAtZero: true,
          title: { display: true, text: 'Warning Quantity' },
          ticks: {
            stepSize: 1,
            precision: 0 // 정수로 표기
          }
        }
      }
    }
  });

  // 주간 Andon 추이 차트 (빈 차트로 시작, 실제 데이터 도착 시 동적 구성)
  const weeklyAndonTrendCtx = document.getElementById('weeklyAndonTrendChart').getContext('2d');
  const days = ['Mon', 'Tue', 'Wed', 'Thu', 'Fri', 'Sat', 'Sun'];

  window.weeklyAndonTrendChart = new Chart(weeklyAndonTrendCtx, {
    type: 'bar',
    data: {
      labels: days,
      datasets: [] // 빈 데이터셋으로 시작, SSE 데이터 도착 시 rebuildWeeklyAndonChart로 재구성
    },
    options: {
      responsive: true,
      maintainAspectRatio: false,
      plugins: {
        legend: { position: 'top' },
        tooltip: {
          mode: 'index',
          intersect: false
        }
      },
      scales: {
        y: {
          beginAtZero: true,
          stacked: false,
          title: { display: true, text: 'Count' },
          ticks: {
            stepSize: 1,
            precision: 0 // 정수로 표기
          }
        },
        x: {
          stacked: false,
          title: { display: true, text: 'Day of Week' }
        }
      }
    }
  });

}

function initializeDowntimeDefectiveCharts() {
  // Downtime 발생현황 차트 (data_downtime.js와 일관성 유지)
  const downtimeOccurrenceCtx = document.getElementById('downtimeOccurrenceChart').getContext('2d');
  window.downtimeOccurrenceChart = new Chart(downtimeOccurrenceCtx, {
    type: 'bar',
    data: {
      labels: [], // 동적으로 업데이트
      datasets: [{
        label: 'Total downtime duration (minutes)',
        data: [], // 동적으로 업데이트
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
        legend: { display: false },
        tooltip: {
          callbacks: {
            label: function (context) {
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
          title: { display: true, text: 'Duration (minutes)' },
          ticks: {
            // 분 단위이므로 소수점 1자리까지 표시
            callback: function (value) {
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

  // Defective 발생현황 차트 (data_downtime.js와 일관성 유지)
  const defectiveOccurrenceCtx = document.getElementById('defectiveOccurrenceChart').getContext('2d');
  window.defectiveOccurrenceChart = new Chart(defectiveOccurrenceCtx, {
    type: 'bar',
    data: {
      labels: [], // 동적으로 업데이트
      datasets: [{
        label: 'Defective occurrence quantity',
        data: [], // 동적으로 업데이트
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
        legend: { display: false }
      },
      scales: {
        y: {
          beginAtZero: true,
          title: { display: true, text: 'Defective quantity' },
          ticks: {
            stepSize: 1,
            precision: 0 // 정수로 표기
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

function initializeProductionAnalysisCharts() {
  // OEE Trend 차트 (data_oee.js의 createOeeTrendChart와 동일)
  const oeeTrendCtx = document.getElementById('oeeTrendChart').getContext('2d');
  window.oeeTrendChart = new Chart(oeeTrendCtx, {
    type: 'line',
    data: {
      labels: [],
      datasets: [{
        label: 'OEE %',
        data: [],
        borderColor: chartColors.primary,
        backgroundColor: chartColors.primary + '20',
        fill: true,
        tension: 0.4,
        pointBackgroundColor: chartColors.primary,
        pointBorderColor: chartColors.primary,
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

/**
 * Dashboard 초기화
 */
document.addEventListener('DOMContentLoaded', function () {

  // 차트들 초기화
  initializeGaugeCharts();
  initializeAndonCharts();
  initializeDowntimeDefectiveCharts();
  initializeProductionAnalysisCharts();

  // Dashboard 매니저 시작
  const dashboardManager = new DashboardManager();

});