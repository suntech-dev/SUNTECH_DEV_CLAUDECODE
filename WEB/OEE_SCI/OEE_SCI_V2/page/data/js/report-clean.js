/* ===============================
   AI Report JavaScript - Clean Version
   실제 데이터 로딩만 유지, 가상 데이터 생성 코드 제거
   =============================== */

class AIReportSystem {
  constructor() {
    // 기본 설정
    this.charts = {};
    this.eventSource = null;
    this.isLoading = false;
    this.lastUpdateTime = null;
    
    // 필터 설정
    this.filters = {
      factory: '',
      line: '',
      machine: '',
      timeRange: 'today',
      dateRange: '',
      shift: ''
    };
    
    // AI 분석 데이터
    this.aiData = {};
    
    // 성능 최적화 설정
    this.performanceOptimization = {
      enabled: true,
      updateInterval: 5000,
      maxDataPoints: 1000
    };

    // 실시간 업데이트 설정
    this.realTimeSettings = {
      enabled: true,
      interval: 5000,
      maxRetries: 3,
      currentRetries: 0
    };

    console.log('🤖 AI Report System initializing...');
    this.init();
  }

  init() {
    this.initDateRangePicker();
    this.initFilterSystem();
    this.initEventListeners();
    this.initCharts();
    this.loadInitialData();
    this.startRealTimeUpdates();
    
    console.log('✅ AI Report System initialized successfully');
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
    // 실제 필터 데이터 로드 (API 호출)
    await this.loadFilterOptions();
    this.setupFilterDependencies();
  }

  async loadFilterOptions() {
    try {
      // 공장 목록 로드
      const factoryResponse = await fetch('../api/sewing.php?code=get_factories');
      if (factoryResponse.ok) {
        const factoryData = await factoryResponse.json();
        if (factoryData.code === '00') {
          this.populateFilterSelect('factorySelect', factoryData.data);
        }
      }
    } catch (error) {
      console.error('❌ Failed to load filter options:', error);
    }
  }

  populateFilterSelect(selectId, options) {
    const select = document.getElementById(selectId);
    if (!select || !options) return;

    select.innerHTML = '<option value="">All</option>';
    options.forEach(option => {
      const optionElement = document.createElement('option');
      optionElement.value = option.idx || option.id;
      optionElement.textContent = option.name || option.factory_name || option.line_name;
      select.appendChild(optionElement);
    });
  }

  setupFilterDependencies() {
    // 필터 변경 이벤트 리스너 설정
    const filterSelects = ['factorySelect', 'lineSelect', 'machineSelect', 'timeRangeSelect', 'shiftSelect'];
    
    filterSelects.forEach(selectId => {
      const element = document.getElementById(selectId);
      if (element) {
        element.addEventListener('change', async (e) => {
          const filterKey = selectId.replace('Select', '').toLowerCase();
          
          if (filterKey === 'timerange') {
            this.filters.timeRange = e.target.value;
          } else {
            this.filters[filterKey] = e.target.value;
          }
          
          // 종속 필터 업데이트
          if (filterKey === 'factory') {
            await this.updateLineOptions(e.target.value);
          } else if (filterKey === 'line') {
            await this.updateMachineOptions(e.target.value);
          }
          
          this.onFiltersChanged();
        });
      }
    });
  }

  async updateLineOptions(factoryId) {
    if (!factoryId) {
      this.populateFilterSelect('lineSelect', []);
      this.populateFilterSelect('machineSelect', []);
      return;
    }

    try {
      const response = await fetch(`../api/sewing.php?code=get_lines&factory_id=${factoryId}`);
      if (response.ok) {
        const data = await response.json();
        if (data.code === '00') {
          this.populateFilterSelect('lineSelect', data.data);
        }
      }
      this.populateFilterSelect('machineSelect', []);
    } catch (error) {
      console.error('❌ Failed to load line options:', error);
    }
  }

  async updateMachineOptions(lineId) {
    if (!lineId) {
      this.populateFilterSelect('machineSelect', []);
      return;
    }

    try {
      const response = await fetch(`../api/sewing.php?code=get_machines&line_id=${lineId}`);
      if (response.ok) {
        const data = await response.json();
        if (data.code === '00') {
          this.populateFilterSelect('machineSelect', data.data);
        }
      }
    } catch (error) {
      console.error('❌ Failed to load machine options:', error);
    }
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

    // 페이지 가시성 변경 처리
    document.addEventListener('visibilitychange', () => {
      if (document.hidden) {
        this.pauseRealTimeUpdates();
      } else {
        this.resumeRealTimeUpdates();
      }
    });

    // 창 크기 변경 시 차트 리사이즈
    window.addEventListener('resize', () => {
      this.resizeCharts();
    });
  }

  initCharts() {
    console.log('📊 Initializing charts...');
    
    // Chart.js 기본 설정
    Chart.defaults.font.family = 'var(--sap-font-family)';
    Chart.defaults.color = 'var(--sap-text-primary)';
    
    this.initKPIGauges();
    this.initAdvancedCharts();
    
    console.log('✅ Charts initialized successfully');
  }

  initKPIGauges() {
    // KPI 게이지 차트들 초기화
    const gaugeConfigs = [
      { id: 'oeeGauge', label: 'OEE', color: '#0070f2' },
      { id: 'availabilityGauge', label: 'Availability', color: '#30914c' },
      { id: 'performanceGauge', label: 'Performance', color: '#e26b0a' },
      { id: 'qualityGauge', label: 'Quality', color: '#00d4aa' }
    ];

    gaugeConfigs.forEach(config => {
      const canvas = document.getElementById(config.id);
      if (canvas) {
        this.charts[config.id] = this.createGaugeChart(canvas, 0, config.label, config.color);
      }
    });
  }

  createGaugeChart(canvas, value, label, color) {
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

  initAdvancedCharts() {
    // 고급 차트들 초기화
    this.initHeatMapChart();
    this.initPredictiveChart();
    this.initCorrelationChart();
    this.initParetoChart();
  }

  initHeatMapChart() {
    const ctx = document.getElementById('performanceHeatMap');
    if (!ctx) return;

    this.charts.heatMap = new Chart(ctx, {
      type: 'scatter',
      data: {
        datasets: [{
          label: 'Performance Heatmap',
          data: [],
          backgroundColor: (context) => this.getHeatMapColor(context),
          pointRadius: 8
        }]
      },
      options: {
        responsive: true,
        maintainAspectRatio: false,
        plugins: {
          legend: { display: false },
          title: { display: true, text: 'Machine Performance Heatmap' }
        },
        scales: {
          x: { 
            type: 'linear',
            title: { display: true, text: 'Time (Hours)' }
          },
          y: { 
            title: { display: true, text: 'Machine ID' }
          }
        }
      }
    });
  }

  getHeatMapColor(context) {
    const value = context.parsed?.y || 0;
    const alpha = 0.8;
    if (value >= 85) return `rgba(48, 145, 76, ${alpha})`;
    if (value >= 70) return `rgba(255, 193, 7, ${alpha})`;
    if (value >= 50) return `rgba(255, 152, 0, ${alpha})`;
    return `rgba(244, 67, 54, ${alpha})`;
  }

  initPredictiveChart() {
    const ctx = document.getElementById('predictiveAnalysisChart');
    if (!ctx) return;

    this.charts.predictive = new Chart(ctx, {
      type: 'line',
      data: {
        labels: [],
        datasets: [{
          label: 'Actual',
          data: [],
          borderColor: '#0070f2',
          backgroundColor: 'rgba(0, 112, 242, 0.1)',
          tension: 0.4
        }, {
          label: 'Predicted',
          data: [],
          borderColor: '#e26b0a',
          backgroundColor: 'rgba(226, 107, 10, 0.1)',
          borderDash: [5, 5],
          tension: 0.4
        }]
      },
      options: {
        responsive: true,
        maintainAspectRatio: false,
        plugins: {
          legend: { display: true },
          title: { display: true, text: 'OEE Trend & Prediction' }
        },
        scales: {
          y: {
            beginAtZero: true,
            max: 100,
            title: { display: true, text: 'OEE (%)' }
          }
        }
      }
    });
  }

  initCorrelationChart() {
    const ctx = document.getElementById('correlationMatrix');
    if (!ctx) return;

    this.charts.correlation = new Chart(ctx, {
      type: 'scatter',
      data: {
        datasets: [{
          label: 'Correlation Matrix',
          data: [],
          backgroundColor: (context) => this.getCorrelationColor(context),
          pointRadius: 15
        }]
      },
      options: {
        responsive: true,
        maintainAspectRatio: false,
        plugins: {
          legend: { display: false },
          title: { display: true, text: 'Factor Correlation Matrix' }
        },
        scales: {
          x: { 
            type: 'linear',
            title: { display: true, text: 'Factor X' }
          },
          y: { 
            title: { display: true, text: 'Factor Y' }
          }
        }
      }
    });
  }

  getCorrelationColor(context) {
    const correlation = context.raw?.z || 0;
    const intensity = Math.abs(correlation);
    const alpha = 0.7;
    
    if (correlation > 0) {
      return `rgba(48, 145, 76, ${intensity * alpha})`;
    } else {
      return `rgba(244, 67, 54, ${intensity * alpha})`;
    }
  }

  initParetoChart() {
    const ctx = document.getElementById('paretoChart');
    if (!ctx) return;

    this.charts.pareto = new Chart(ctx, {
      type: 'bar',
      data: {
        labels: [],
        datasets: [{
          label: 'Frequency',
          data: [],
          backgroundColor: '#0070f2',
          yAxisID: 'y'
        }, {
          label: 'Cumulative %',
          data: [],
          type: 'line',
          borderColor: '#e26b0a',
          backgroundColor: 'rgba(226, 107, 10, 0.1)',
          yAxisID: 'y1'
        }]
      },
      options: {
        responsive: true,
        maintainAspectRatio: false,
        plugins: {
          title: { display: true, text: 'Pareto Analysis' }
        },
        scales: {
          y: {
            type: 'linear',
            display: true,
            position: 'left',
            title: { display: true, text: 'Frequency' }
          },
          y1: {
            type: 'linear',
            display: true,
            position: 'right',
            title: { display: true, text: 'Cumulative %' },
            grid: { drawOnChartArea: false },
            min: 0,
            max: 100
          }
        }
      }
    });
  }

  // ===============================
  // 데이터 로딩 및 업데이트
  // ===============================

  async loadInitialData() {
    console.log('📡 Loading initial data...');
    this.showLoadingState(true);
    
    try {
      // 실제 API에서 데이터 로드
      const response = await fetch('../api/sewing.php?code=get_report_data');
      if (response.ok) {
        const data = await response.json();
        if (data.code === '00') {
          this.aiData = data.data;
          this.updateDashboardUI();
        } else {
          throw new Error(data.msg || 'Failed to load data');
        }
      } else {
        throw new Error('Network error');
      }
    } catch (error) {
      console.error('❌ Failed to load initial data:', error);
      this.showError('Failed to load data. Please try again.');
    } finally {
      this.showLoadingState(false);
    }
  }

  async startAIAnalysis() {
    console.log('🤖 Starting AI analysis...');
    
    const button = document.getElementById('startAIAnalysisBtn');
    if (button) {
      button.textContent = 'Analyzing...';
      button.disabled = true;
    }
    
    try {
      // 실제 AI 분석 API 호출
      const response = await fetch('../api/sewing.php?code=start_ai_analysis', {
        method: 'POST',
        headers: {
          'Content-Type': 'application/json'
        },
        body: JSON.stringify(this.filters)
      });
      
      if (response.ok) {
        const data = await response.json();
        if (data.code === '00') {
          this.updateAIInsights(data.data);
        }
      }
    } catch (error) {
      console.error('❌ AI analysis failed:', error);
      this.showError('AI analysis failed. Please try again.');
    } finally {
      if (button) {
        button.textContent = 'Start AI Analysis';
        button.disabled = false;
      }
    }
  }

  startRealTimeUpdates() {
    if (!this.realTimeSettings.enabled) return;
    
    console.log('🔄 Starting real-time updates...');
    this.connectToStream();
  }

  connectToStream() {
    if (this.eventSource) {
      this.eventSource.close();
    }
    
    const params = new URLSearchParams(this.filters).toString();
    const streamUrl = `proc/report_stream.php?${params}`;
    
    this.eventSource = new EventSource(streamUrl);
    
    this.eventSource.onopen = () => {
      console.log('✅ SSE connection established');
      this.realTimeSettings.currentRetries = 0;
    };
    
    this.eventSource.addEventListener('ai_data', (event) => {
      const data = JSON.parse(event.data);
      this.updateRealTimeData(data);
    });
    
    this.eventSource.addEventListener('heartbeat', (event) => {
      const data = JSON.parse(event.data);
      this.updateLastUpdateTime(data.timestamp);
    });
    
    this.eventSource.onerror = (error) => {
      console.error('❌ SSE error:', error);
      this.handleStreamError();
    };
  }

  handleStreamError() {
    if (this.realTimeSettings.currentRetries < this.realTimeSettings.maxRetries) {
      this.realTimeSettings.currentRetries++;
      console.log(`🔄 Retrying SSE connection (${this.realTimeSettings.currentRetries}/${this.realTimeSettings.maxRetries})`);
      
      setTimeout(() => {
        this.connectToStream();
      }, 5000 * this.realTimeSettings.currentRetries);
    } else {
      console.error('❌ Max retries reached. Stopping real-time updates.');
      this.showError('Real-time updates stopped. Please refresh the page.');
    }
  }

  updateRealTimeData(data) {
    this.aiData = data;
    this.updateDashboardUI();
    this.updateCharts();
    this.updateLastUpdateTime(data.timestamp);
  }

  pauseRealTimeUpdates() {
    if (this.eventSource) {
      this.eventSource.close();
      this.eventSource = null;
    }
  }

  resumeRealTimeUpdates() {
    if (this.realTimeSettings.enabled && !this.eventSource) {
      this.connectToStream();
    }
  }

  // ===============================
  // UI 업데이트 함수들
  // ===============================

  updateDashboardUI() {
    this.updateKPICards();
    this.updateInsightCards();
    this.updateDataTables();
    this.updateCharts();
  }

  updateKPICards() {
    const stats = this.aiData.integrated_stats || {};
    const oeeStats = stats.oee_stats || {};
    
    // OEE 게이지 업데이트
    if (this.charts.oeeGauge && oeeStats.avg_oee !== undefined) {
      const value = Math.round(oeeStats.avg_oee);
      this.updateGaugeChart(this.charts.oeeGauge, value);
    }
    
    // 다른 KPI 게이지들도 업데이트
    if (this.charts.availabilityGauge && oeeStats.avg_availability !== undefined) {
      const value = Math.round(oeeStats.avg_availability);
      this.updateGaugeChart(this.charts.availabilityGauge, value);
    }
  }

  updateGaugeChart(chart, value) {
    chart.data.datasets[0].data = [value, 100 - value];
    chart.update('none');
  }

  updateInsightCards() {
    const aiInsights = this.aiData.ai_insights || {};
    const predictions = this.aiData.predictions || {};
    const anomalies = this.aiData.anomalies || [];
    const recommendations = this.aiData.recommendations || [];
    
    // AI 인사이트 업데이트
    this.updateElement('performanceScore', aiInsights.performance_score);
    this.updateElement('efficiencyTrend', aiInsights.efficiency_trends?.direction);
    
    // 예측 데이터 업데이트  
    this.updateElement('oeeForecasts7d', predictions.oee_forecast_7d);
    this.updateElement('oeeTrend7d', predictions.trend_7d);
    this.updateElement('oeeForecasts30d', predictions.oee_forecast_30d);
    this.updateElement('oeeTrend30d', predictions.trend_30d);
    this.updateElement('equipmentRisk', predictions.equipment_risk);
    
    // 이상징후 업데이트
    this.updateAnomaliesList(anomalies);
    
    // 추천사항 업데이트
    this.updateRecommendationsList(recommendations);
  }

  updateElement(elementId, value) {
    const element = document.getElementById(elementId);
    if (element && value !== undefined) {
      element.textContent = value;
    }
  }

  updateAnomaliesList(anomalies) {
    const container = document.getElementById('anomalyList');
    if (!container) return;
    
    if (anomalies.length === 0) {
      container.innerHTML = '<div class="no-anomalies">No anomalies detected</div>';
      return;
    }
    
    container.innerHTML = anomalies.map(anomaly => `
      <div class="anomaly-item">
        <div class="anomaly-header">
          <span class="anomaly-type">${anomaly.type}</span>
          <span class="anomaly-severity--${anomaly.severity.toLowerCase()}">${anomaly.severity}</span>
        </div>
        <div class="anomaly-description">${anomaly.description}</div>
        <div class="anomaly-timestamp">${anomaly.timestamp}</div>
      </div>
    `).join('');
  }

  updateRecommendationsList(recommendations) {
    const container = document.getElementById('recommendationList');
    if (!container) return;
    
    if (recommendations.length === 0) {
      container.innerHTML = '<div class="no-recommendations">No recommendations available</div>';
      return;
    }
    
    container.innerHTML = recommendations.map(rec => `
      <div class="recommendation-item">
        <span class="recommendation-priority">${rec.priority}</span>
        <span>${rec.text}</span>
      </div>
    `).join('');
  }

  updateDataTables() {
    const integratedData = this.aiData.integrated_data || [];
    const tableBody = document.querySelector('#integratedDataTable tbody');
    
    if (tableBody) {
      tableBody.innerHTML = integratedData.map(row => `
        <tr>
          <td>${row.machine_no}</td>
          <td>${row.oee}%</td>
          <td>${row.availability}%</td>
          <td>${row.performance}%</td>
          <td>${row.quality}%</td>
          <td>${row.active_issues}</td>
          <td><span class="status-indicator--${row.status}">${row.status}</span></td>
        </tr>
      `).join('');
    }
  }

  updateCharts() {
    this.updateHeatMapChart();
    this.updatePredictiveChart();
    this.updateCorrelationChart();
    this.updateParetoChart();
  }

  updateHeatMapChart() {
    if (!this.charts.heatMap || !this.aiData.heatmap_data) return;
    
    this.charts.heatMap.data.datasets[0].data = this.aiData.heatmap_data;
    this.charts.heatMap.update('none');
  }

  updatePredictiveChart() {
    if (!this.charts.predictive || !this.aiData.trend_data) return;
    
    const trendData = this.aiData.trend_data;
    this.charts.predictive.data.labels = trendData.labels || [];
    this.charts.predictive.data.datasets[0].data = trendData.actual || [];
    this.charts.predictive.data.datasets[1].data = trendData.predicted || [];
    this.charts.predictive.update('none');
  }

  updateCorrelationChart() {
    if (!this.charts.correlation || !this.aiData.correlation_matrix) return;
    
    this.charts.correlation.data.datasets[0].data = this.aiData.correlation_matrix;
    this.charts.correlation.update('none');
  }

  updateParetoChart() {
    if (!this.charts.pareto || !this.aiData.pareto_data) return;
    
    const paretoData = this.aiData.pareto_data;
    this.charts.pareto.data.labels = paretoData.labels || [];
    this.charts.pareto.data.datasets[0].data = paretoData.frequency || [];
    this.charts.pareto.data.datasets[1].data = paretoData.cumulative || [];
    this.charts.pareto.update('none');
  }

  // ===============================
  // 이벤트 핸들러들
  // ===============================

  onFiltersChanged() {
    console.log('🔄 Filters changed:', this.filters);
    this.refreshData();
  }

  refreshData() {
    console.log('🔄 Refreshing data...');
    this.loadInitialData();
    this.restartRealTimeUpdates();
  }

  restartRealTimeUpdates() {
    this.pauseRealTimeUpdates();
    setTimeout(() => {
      this.startRealTimeUpdates();
    }, 1000);
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
    
    // UI 리셋
    document.getElementById('factorySelect').value = '';
    document.getElementById('lineSelect').value = '';
    document.getElementById('machineSelect').value = '';
    document.getElementById('timeRangeSelect').value = 'today';
    document.getElementById('shiftSelect').value = '';
    document.getElementById('dateRangePicker').value = '';
    
    this.refreshData();
  }

  switchDataTab(tabName) {
    document.querySelectorAll('.tab-btn').forEach(btn => btn.classList.remove('active'));
    document.querySelectorAll('.data-table-wrapper').forEach(wrapper => wrapper.classList.remove('active'));
    
    document.querySelector(`[data-tab="${tabName}"]`).classList.add('active');
    document.getElementById(`${tabName}Table`).classList.add('active');
  }

  updateHeatMap(metric) {
    console.log(`🔥 Updating heat map metric to: ${metric}`);
    // 히트맵 메트릭 업데이트 로직은 실제 API 호출로 대체
  }

  togglePrediction() {
    console.log('🔮 Toggling prediction visibility');
    // 예측 표시/숨기기 로직
  }

  updateParetoChart(category) {
    console.log(`📊 Updating Pareto chart category to: ${category}`);
    // 파레토 차트 카테고리 업데이트 로직은 실제 API 호출로 대체
  }

  exportReport() {
    console.log('📄 Exporting report...');
    // 실제 리포트 내보내기는 API 호출로 처리
    window.open('../api/sewing.php?code=export_report&' + new URLSearchParams(this.filters).toString());
  }

  // ===============================
  // 유틸리티 함수들
  // ===============================

  updateLastUpdateTime(timestamp) {
    const element = document.getElementById('lastAnalysisTime');
    if (element) {
      const time = timestamp ? new Date(timestamp) : new Date();
      element.textContent = `Last Analysis: ${time.toLocaleTimeString()}`;
    }
  }

  showLoadingState(isLoading, message = '') {
    this.isLoading = isLoading;
    
    const loadingIndicator = document.getElementById('loadingIndicator');
    if (loadingIndicator) {
      if (isLoading) {
        loadingIndicator.style.display = 'block';
        if (message) {
          loadingIndicator.textContent = message;
        }
      } else {
        loadingIndicator.style.display = 'none';
      }
    }
  }

  showError(message) {
    console.error('❌', message);
    
    const errorContainer = document.getElementById('errorContainer');
    if (errorContainer) {
      errorContainer.innerHTML = `
        <div class="error-message">
          <i class="sap-icon sap-icon--error"></i>
          <span>${message}</span>
          <button onclick="this.parentElement.parentElement.style.display='none'">×</button>
        </div>
      `;
      errorContainer.style.display = 'block';
      
      // 5초 후 자동 숨김
      setTimeout(() => {
        errorContainer.style.display = 'none';
      }, 5000);
    }
  }

  resizeCharts() {
    Object.values(this.charts).forEach(chart => {
      if (chart && typeof chart.resize === 'function') {
        chart.resize();
      }
    });
  }

  destroy() {
    // 정리 작업
    this.pauseRealTimeUpdates();
    
    Object.values(this.charts).forEach(chart => {
      if (chart && typeof chart.destroy === 'function') {
        chart.destroy();
      }
    });
    
    this.charts = {};
    this.aiData = {};
    
    console.log('🗑️ AI Report System destroyed');
  }
}

// 전역 변수
let aiReportSystem;

// 페이지 로드 시 초기화
document.addEventListener('DOMContentLoaded', function() {
  console.log('🚀 AI Report System loading...');
  aiReportSystem = new AIReportSystem();
});

// 페이지 언로드 시 정리
window.addEventListener('beforeunload', function() {
  if (aiReportSystem) {
    aiReportSystem.destroy();
  }
});