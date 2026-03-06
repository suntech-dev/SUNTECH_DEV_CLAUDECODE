/* ===============================
   AI Report JavaScript - Simple Version
   Based on dashboard_total.html approach
   =============================== */

class SimpleAIReportSystem {
  constructor() {
    this.charts = {};
    this.data = {};
    this.filters = {
      factory: '',
      line: '',
      machine: '',
      timeRange: 'today',
      dateRange: '',
      shift: ''
    };
    
    console.log('🤖 Simple AI Report System initializing...');
    this.init();
  }

  init() {
    this.initDateRangePicker();
    this.initFilterSystem();
    this.initEventListeners();
    this.initCharts();
    this.loadInitialData();
    this.startRealTimeUpdates();
    
    console.log('✅ Simple AI Report System initialized successfully');
  }

  initDateRangePicker() {
    const dateRangePicker = $('#dateRangePicker');
    if (dateRangePicker.length) {
      dateRangePicker.daterangepicker({
        autoUpdateInput: false,
        locale: {
          cancelLabel: 'Clear',
          format: 'YYYY-MM-DD'
        },
        ranges: {
          'Today': [moment(), moment()],
          'Yesterday': [moment().subtract(1, 'days'), moment().subtract(1, 'days')],
          'Last 7 Days': [moment().subtract(6, 'days'), moment()],
          'Last 30 Days': [moment().subtract(29, 'days'), moment()]
        }
      });

      dateRangePicker.on('apply.daterangepicker', (ev, picker) => {
        dateRangePicker.val(picker.startDate.format('YYYY-MM-DD') + ' - ' + picker.endDate.format('YYYY-MM-DD'));
        this.filters.dateRange = picker.startDate.format('YYYY-MM-DD') + ',' + picker.endDate.format('YYYY-MM-DD');
        this.onFiltersChanged();
      });

      dateRangePicker.on('cancel.daterangepicker', () => {
        dateRangePicker.val('');
        this.filters.dateRange = '';
        this.onFiltersChanged();
      });
    }
  }

  async initFilterSystem() {
    // 간단한 필터 시스템 설정
    this.setupFilterDependencies();
  }

  setupFilterDependencies() {
    // 필터 변경 이벤트
    const filterSelects = ['timeRangeSelect', 'shiftSelect'];
    filterSelects.forEach(selectId => {
      const element = document.getElementById(selectId);
      if (element) {
        element.addEventListener('change', (e) => {
          const filterKey = selectId.replace('Select', '').toLowerCase();
          this.filters[filterKey === 'timerange' ? 'timeRange' : filterKey] = e.target.value;
          this.onFiltersChanged();
        });
      }
    });
  }

  initEventListeners() {
    // 액션 버튼 이벤트 리스너들
    document.getElementById('startAIAnalysisBtn')?.addEventListener('click', () => this.startAIAnalysis());
    document.getElementById('exportReportBtn')?.addEventListener('click', () => this.exportReport());
    document.getElementById('refreshDataBtn')?.addEventListener('click', () => this.refreshData());
    document.getElementById('resetFiltersBtn')?.addEventListener('click', () => this.resetFilters());

    // 차트 컨트롤 이벤트 리스너들
    document.getElementById('heatmapMetric')?.addEventListener('change', (e) => this.updateHeatMap(e.target.value));
    document.getElementById('togglePredictionBtn')?.addEventListener('click', () => this.togglePrediction());
    document.getElementById('paretoCategory')?.addEventListener('change', (e) => this.updateParetoChart(e.target.value));

    // 테이블 탭 이벤트 리스너들
    document.querySelectorAll('.tab-btn').forEach(btn => {
      btn.addEventListener('click', (e) => this.switchDataTab(e.target.dataset.tab));
    });
  }

  initCharts() {
    console.log('📊 Initializing simple charts...');
    
    Chart.defaults.font.family = 'var(--sap-font-family)';
    Chart.defaults.color = 'var(--sap-text-primary)';
    
    this.initKPIGauges();
    this.initSimpleCharts();
    
    console.log('✅ Charts initialized successfully');
  }

  initKPIGauges() {
    // KPI 게이지 초기화 (dashboard_total.html 방식)
    const kpiData = this.generateOEEData();
    
    this.charts.oeeGauge = this.createGaugeChart('oeeGauge', kpiData.overall, 'OEE', '#0070f2');
    this.charts.availabilityGauge = this.createGaugeChart('availabilityGauge', kpiData.availability, 'Availability', '#30914c');
    this.charts.performanceGauge = this.createGaugeChart('performanceGauge', kpiData.performance, 'Performance', '#e26b0a');
    this.charts.qualityGauge = this.createGaugeChart('qualityGauge', kpiData.quality, 'Quality', '#00d4aa');
  }

  createGaugeChart(canvasId, value, label, color) {
    const canvas = document.getElementById(canvasId);
    if (!canvas) return null;
    
    const ctx = canvas.getContext('2d');
    
    return new Chart(ctx, {
      type: 'doughnut',
      data: {
        datasets: [{
          data: [value, 100 - value],
          backgroundColor: [color, 'rgba(200, 200, 200, 0.2)'],
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
          
          ctx.font = 'bold 2rem Arial';
          ctx.fillStyle = color;
          ctx.fillText(`${value}%`, centerX, centerY);
          
          ctx.restore();
        }
      }]
    });
  }

  initSimpleCharts() {
    // 간단한 차트들 초기화
    this.initProductionChart();
    this.initAndonChart();
    this.initHeatMapChart();
  }

  initProductionChart() {
    const ctx = document.getElementById('productionTrendChart');
    if (!ctx) return;

    const productionData = this.generateProductionData();
    
    this.charts.production = new Chart(ctx, {
      type: 'line',
      data: {
        labels: productionData.hours,
        datasets: [{
          label: 'Production',
          data: productionData.production,
          borderColor: '#30914c',
          backgroundColor: 'rgba(48, 145, 76, 0.1)',
          tension: 0.4,
          fill: true,
          pointRadius: 4,
          borderWidth: 2
        }]
      },
      options: {
        responsive: true,
        maintainAspectRatio: false,
        plugins: {
          legend: { display: true },
          tooltip: {
            callbacks: {
              label: function(context) {
                return `Production: ${context.parsed.y}EA`;
              }
            }
          }
        },
        scales: {
          y: {
            beginAtZero: true,
            title: { display: true, text: 'Production (EA)' }
          },
          x: {
            title: { display: true, text: 'Time' }
          }
        }
      }
    });
  }

  initAndonChart() {
    const ctx = document.getElementById('andonOccurrenceChart');
    if (!ctx) return;

    const andonData = this.generateAndonData();
    
    this.charts.andon = new Chart(ctx, {
      type: 'bar',
      data: {
        labels: andonData.map(d => d.type),
        datasets: [{
          label: 'Count',
          data: andonData.map(d => d.count),
          backgroundColor: ['#da1e28', '#e26b0a', '#7c3aed'],
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
            title: { display: true, text: 'Count' }
          },
          x: {
            title: { display: true, text: 'Andon Type' }
          }
        }
      }
    });
  }

  initHeatMapChart() {
    const ctx = document.getElementById('performanceHeatMap');
    if (!ctx) return;

    // 간단한 히트맵 스타일 차트
    const heatmapData = this.generateHeatMapData();
    
    this.charts.heatmap = new Chart(ctx, {
      type: 'scatter',
      data: {
        datasets: [{
          label: 'Performance Heatmap',
          data: heatmapData,
          backgroundColor: (context) => {
            const value = context.parsed.y || 0;
            const alpha = 0.8;
            if (value >= 85) return `rgba(48, 145, 76, ${alpha})`;
            if (value >= 70) return `rgba(255, 193, 7, ${alpha})`;
            if (value >= 50) return `rgba(255, 152, 0, ${alpha})`;
            return `rgba(244, 67, 54, ${alpha})`;
          },
          pointRadius: 8
        }]
      },
      options: {
        responsive: true,
        maintainAspectRatio: false,
        plugins: {
          legend: { display: false }
        },
        scales: {
          x: {
            type: 'linear',
            title: { display: true, text: 'Time (Hours)' }
          },
          y: {
            title: { display: true, text: 'Machine Number' }
          }
        }
      }
    });
  }

  // ===============================
  // 간단한 데이터 생성 함수들
  // ===============================
  
  generateOEEData() {
    const availability = Math.random() * (95 - 85) + 85;
    const performance = Math.random() * (98 - 90) + 90;
    const quality = Math.random() * (99 - 92) + 92;
    const overall = (availability * performance * quality) / 10000;
    
    return {
      availability: Math.round(availability * 10) / 10,
      performance: Math.round(performance * 10) / 10,
      quality: Math.round(quality * 10) / 10,
      overall: Math.round(overall * 10) / 10
    };
  }

  generateProductionData() {
    const hours = [];
    const production = [];
    
    for (let i = 8; i < 20; i++) {
      hours.push(`${i}:00`);
      production.push(Math.floor(45 + Math.random() * 50));
    }
    
    return { hours, production };
  }

  generateAndonData() {
    const types = ['Machine', 'Process', 'Quality'];
    return types.map(type => ({
      type: type,
      count: Math.floor(Math.random() * 20) + 5
    }));
  }

  generateHeatMapData() {
    const data = [];
    for (let i = 1; i <= 5; i++) {
      for (let j = 8; j < 20; j++) {
        data.push({
          x: j,
          y: i,
          machineNo: `M${i}`,
          performance: Math.floor(50 + Math.random() * 50)
        });
      }
    }
    return data;
  }

  generateMachineData() {
    const machines = ['MACHINE-001', 'MACHINE-002', 'MACHINE-003', 'MACHINE-004', 'MACHINE-005'];
    return machines.map(machineId => {
      const oee = Math.random() * (90 - 60) + 60;
      return {
        machine_id: machineId,
        oee: Math.round(oee * 10) / 10,
        availability: Math.round((85 + Math.random() * 10) * 10) / 10,
        performance: Math.round((80 + Math.random() * 15) * 10) / 10,
        quality: Math.round((90 + Math.random() * 8) * 10) / 10,
        status: Math.random() > 0.8 ? 'Warning' : 'Running',
        active_issues: Math.floor(Math.random() * 3)
      };
    });
  }

  // ===============================
  // 이벤트 핸들러들
  // ===============================

  async loadInitialData() {
    console.log('📡 Loading initial data...');
    this.data = {
      oee: this.generateOEEData(),
      production: this.generateProductionData(),
      machines: this.generateMachineData(),
      andon: this.generateAndonData(),
      lastUpdate: new Date().toISOString()
    };
    this.updateDashboardUI();
  }

  async startAIAnalysis() {
    console.log('🤖 Starting AI analysis...');
    document.getElementById('startAIAnalysisBtn').textContent = 'Analyzing...';
    
    // 간단한 분석 시뮬레이션
    setTimeout(() => {
      this.updatePredictiveInsights();
      this.updateAnomalyDetection();
      this.updateRecommendations();
      document.getElementById('startAIAnalysisBtn').textContent = 'Start AI Analysis';
      console.log('✅ AI analysis completed');
    }, 2000);
  }

  updatePredictiveInsights() {
    const oeeData = this.data.oee;
    
    document.getElementById('oeeForecasts7d').textContent = `${(oeeData.overall + (Math.random() - 0.5) * 5).toFixed(1)}%`;
    document.getElementById('oeeTrend7d').textContent = Math.random() > 0.5 ? '↗ Improving' : '↘ Declining';
    
    document.getElementById('oeeForecasts30d').textContent = `${(oeeData.overall + (Math.random() - 0.5) * 8).toFixed(1)}%`;
    document.getElementById('oeeTrend30d').textContent = Math.random() > 0.5 ? '↗ Improving' : '→ Stable';
    
    document.getElementById('equipmentRisk').textContent = Math.random() > 0.7 ? 'High' : Math.random() > 0.3 ? 'Medium' : 'Low';
    document.getElementById('riskTrend').textContent = '95% Confidence';
    
    document.getElementById('predictionStatus').textContent = 'Completed';
    document.getElementById('predictionStatus').className = 'status-badge status-badge--success';
  }

  updateAnomalyDetection() {
    const anomalies = Math.floor(Math.random() * 3);
    document.getElementById('anomalyStatus').textContent = `${anomalies} Anomalies`;
    
    const anomalyList = document.getElementById('anomalyList');
    if (anomalies === 0) {
      anomalyList.innerHTML = '<div class="no-anomalies">No anomalies detected in current data range</div>';
    } else {
      const anomalyItems = [];
      for (let i = 0; i < anomalies; i++) {
        const severity = Math.random() > 0.6 ? 'high' : 'medium';
        const machine = `MACHINE-00${Math.floor(Math.random() * 5) + 1}`;
        anomalyItems.push(`
          <div class="anomaly-item">
            <div class="anomaly-header">
              <span class="anomaly-type">Performance Anomaly</span>
              <span class="anomaly-severity--${severity}">${severity.toUpperCase()}</span>
            </div>
            <div class="anomaly-description">${machine}: Performance drop detected</div>
            <div class="anomaly-meta">
              <span class="anomaly-machine">${machine}</span>
              <span class="anomaly-confidence">Confidence: ${(85 + Math.random() * 10).toFixed(0)}%</span>
            </div>
          </div>
        `);
      }
      anomalyList.innerHTML = anomalyItems.join('');
    }
  }

  updateRecommendations() {
    const recommendations = [
      'Optimize machine maintenance schedule for MACHINE-003',
      'Improve quality control process in Production Line A',
      'Reduce setup time by implementing quick changeover techniques',
      'Monitor temperature fluctuations in Quality Station'
    ];
    
    const shuffled = recommendations.sort(() => 0.5 - Math.random());
    const selected = shuffled.slice(0, Math.floor(Math.random() * 3) + 1);
    
    const recommendationList = document.getElementById('recommendationList');
    if (selected.length === 0) {
      recommendationList.innerHTML = '<div class="no-recommendations">No specific recommendations at this time</div>';
    } else {
      const items = selected.map((rec, idx) => `
        <div class="recommendation-item">
          <span class="recommendation-priority">P${idx + 1}</span>
          <span>${rec}</span>
        </div>
      `).join('');
      recommendationList.innerHTML = items;
    }
    
    document.getElementById('recommendationStatus').textContent = `${selected.length} Active`;
  }

  onFiltersChanged() {
    console.log('🔄 Filters changed:', this.filters);
    this.refreshData();
  }

  refreshData() {
    console.log('🔄 Refreshing data...');
    this.loadInitialData();
    this.updateCharts();
  }

  resetFilters() {
    this.filters = {
      factory: '',
      line: '',
      machine: '',
      timeRange: 'today',
      dateRange: '',
      shift: ''
    };
    
    // Reset UI
    document.getElementById('timeRangeSelect').value = 'today';
    document.getElementById('shiftSelect').value = '';
    document.getElementById('dateRangePicker').value = '';
    
    this.refreshData();
  }

  updateCharts() {
    const oeeData = this.generateOEEData();
    
    // 게이지 차트 업데이트
    if (this.charts.oeeGauge) {
      this.charts.oeeGauge.data.datasets[0].data = [oeeData.overall, 100 - oeeData.overall];
      this.charts.oeeGauge.update('none');
    }
    
    if (this.charts.availabilityGauge) {
      this.charts.availabilityGauge.data.datasets[0].data = [oeeData.availability, 100 - oeeData.availability];
      this.charts.availabilityGauge.update('none');
    }
    
    if (this.charts.performanceGauge) {
      this.charts.performanceGauge.data.datasets[0].data = [oeeData.performance, 100 - oeeData.performance];
      this.charts.performanceGauge.update('none');
    }
    
    if (this.charts.qualityGauge) {
      this.charts.qualityGauge.data.datasets[0].data = [oeeData.quality, 100 - oeeData.quality];
      this.charts.qualityGauge.update('none');
    }
    
    // 다른 차트들도 업데이트
    this.updateProductionChart();
    this.updateAndonChart();
  }

  updateProductionChart() {
    if (this.charts.production) {
      const productionData = this.generateProductionData();
      this.charts.production.data.datasets[0].data = productionData.production;
      this.charts.production.update('none');
    }
  }

  updateAndonChart() {
    if (this.charts.andon) {
      const andonData = this.generateAndonData();
      this.charts.andon.data.datasets[0].data = andonData.map(d => d.count);
      this.charts.andon.update('none');
    }
  }

  updateDashboardUI() {
    this.updateDataTable();
    this.updateLastUpdateTime();
  }

  updateDataTable() {
    const machineData = this.data.machines;
    const tableBody = document.querySelector('#integratedDataTable tbody');
    
    if (tableBody) {
      tableBody.innerHTML = machineData.map(machine => `
        <tr>
          <td>${machine.machine_id}</td>
          <td>${machine.oee}%</td>
          <td>${machine.availability}%</td>
          <td>${machine.performance}%</td>
          <td>${machine.quality}%</td>
          <td>${machine.active_issues}</td>
          <td><span class="status-indicator--${machine.status.toLowerCase()}">${machine.status}</span></td>
        </tr>
      `).join('');
    }
  }

  updateLastUpdateTime() {
    const now = new Date();
    const timeString = now.toLocaleTimeString('ko-KR', { 
      hour: '2-digit', 
      minute: '2-digit', 
      second: '2-digit' 
    });
    
    const element = document.getElementById('lastAnalysisTime');
    if (element) {
      element.textContent = `Last Analysis: ${timeString}`;
    }
  }

  switchDataTab(tabName) {
    // 탭 전환
    document.querySelectorAll('.tab-btn').forEach(btn => btn.classList.remove('active'));
    document.querySelectorAll('.data-table-wrapper').forEach(wrapper => wrapper.classList.remove('active'));
    
    document.querySelector(`[data-tab="${tabName}"]`).classList.add('active');
    document.getElementById(`${tabName}Table`).classList.add('active');
  }

  updateHeatMap(metric) {
    console.log(`🔥 Updating heat map metric to: ${metric}`);
    // 히트맵 업데이트 로직
  }

  togglePrediction() {
    console.log('🔮 Toggling prediction visibility');
    // 예측 표시/숨기기 로직
  }

  updateParetoChart(category) {
    console.log(`📊 Updating Pareto chart category to: ${category}`);
    // 파레토 차트 업데이트 로직
  }

  exportReport() {
    console.log('📄 Exporting report...');
    alert('리포트 내보내기 기능은 데모 버전에서는 제한됩니다.');
  }

  startRealTimeUpdates() {
    // 실시간 업데이트 (30초 간격)
    setInterval(() => {
      this.refreshData();
    }, 30000);
    
    console.log('🔄 Real-time updates started');
  }
}

// 전역 변수
let simpleAIReportSystem;

// 페이지 로드 시 초기화
document.addEventListener('DOMContentLoaded', function() {
  console.log('🚀 Simple AI Report System loading...');
  simpleAIReportSystem = new SimpleAIReportSystem();
});