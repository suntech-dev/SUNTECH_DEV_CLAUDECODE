/**
 * ai_report.js — F11 AI Report System
 * Claude API 없이 규칙 기반 인사이트 렌더링
 * ai_report_engine.php 단일 API 호출로 전체 리포트 구성
 */

var rptState = {
  factory:  '',
  line:     '',
  machine:  '',
  range:    'today',
  dateFrom: '',
  dateTo:   '',
  charts:   {},
  data:     null,
};

var COLORS = {
  good:    '#3fb950',
  warning: '#d29922',
  danger:  '#f85149',
  info:    '#58a6ff',
  muted:   '#8b949e',
};

// ── 초기화 ────────────────────────────────────────────
$(function() {
  initFilters();
  initDatePicker();
  initButtons();
  initCharts();
  runAnalysis();
});

// ── 필터 시스템 (ai_dashboard.js 동일 패턴) ──────────
function initFilters() {
  $.getJSON('../manage/proc/factory.php', function(res) {
    if (!res.success || !res.data) return;
    res.data.forEach(function(item) {
      $('#factoryFilterSelect').append($('<option>').val(item.idx).text(item.factory_name));
    });
  });

  $('#factoryFilterSelect').on('change', function() {
    rptState.factory = $(this).val();
    rptState.line    = '';
    rptState.machine = '';
    $('#factoryLineFilterSelect').html('<option value="">All Line</option>').prop('disabled', true);
    $('#factoryLineMachineFilterSelect').html('<option value="">All Machine</option>').prop('disabled', true);

    if (rptState.factory) {
      $.getJSON('../manage/proc/line.php', { factory_filter: rptState.factory }, function(res) {
        if (!res.success || !res.data) return;
        $('#factoryLineFilterSelect').prop('disabled', false);
        res.data.forEach(function(item) {
          $('#factoryLineFilterSelect').append($('<option>').val(item.idx).text(item.line_name));
        });
      });
    }
  });

  $('#factoryLineFilterSelect').on('change', function() {
    rptState.line    = $(this).val();
    rptState.machine = '';
    $('#factoryLineMachineFilterSelect').html('<option value="">All Machine</option>').prop('disabled', true);

    if (rptState.line) {
      var p = { line_filter: rptState.line };
      if (rptState.factory) p.factory_filter = rptState.factory;
      $.getJSON('../manage/proc/machine.php', p, function(res) {
        if (!res.success || !res.data) return;
        $('#factoryLineMachineFilterSelect').prop('disabled', false);
        res.data.forEach(function(item) {
          $('#factoryLineMachineFilterSelect').append($('<option>').val(item.idx).text(item.machine_no));
        });
      });
    }
  });

  $('#factoryLineMachineFilterSelect').on('change', function() {
    rptState.machine = $(this).val();
  });

  $('#timeRangeSelect').on('change', function() {
    rptState.range = $(this).val();
    if (rptState.range !== 'custom') {
      rptState.dateFrom = '';
      rptState.dateTo   = '';
    }
  });
}

function initDatePicker() {
  var $dp = $('#dateRangePicker');
  if (!$dp.length || typeof $.fn.daterangepicker === 'undefined') return;

  $dp.daterangepicker({
    autoUpdateInput: false,
    locale: { cancelLabel: 'Clear', format: 'YYYY-MM-DD' },
  });

  $dp.on('apply.daterangepicker', function(ev, picker) {
    rptState.range    = 'custom';
    rptState.dateFrom = picker.startDate.format('YYYY-MM-DD');
    rptState.dateTo   = picker.endDate.format('YYYY-MM-DD');
    $(this).val(rptState.dateFrom + ' ~ ' + rptState.dateTo);
    $('#timeRangeSelect').val('custom');
  });
  $dp.on('cancel.daterangepicker', function() {
    $(this).val('');
  });
}

function initButtons() {
  $('#startAIAnalysisBtn, #refreshDataBtn').on('click', function() {
    runAnalysis();
  });

  $('#exportReportBtn').on('click', function() {
    var params = buildParams();
    var url = 'proc/ai_report_export.php?' + $.param(params);
    window.open(url, '_blank');
  });

  $('#resetFiltersBtn').on('click', function() {
    rptState = $.extend(rptState, { factory: '', line: '', machine: '', range: 'today', dateFrom: '', dateTo: '' });
    $('#factoryFilterSelect').val('');
    $('#factoryLineFilterSelect').html('<option value="">All Line</option>').prop('disabled', true);
    $('#factoryLineMachineFilterSelect').html('<option value="">All Machine</option>').prop('disabled', true);
    $('#timeRangeSelect').val('today');
    $('#dateRangePicker').val('');
    runAnalysis();
  });
}

function buildParams() {
  var p = { range: rptState.range };
  if (rptState.factory)  p.factory_filter = rptState.factory;
  if (rptState.line)     p.line_filter    = rptState.line;
  if (rptState.machine)  p.machine_filter = rptState.machine;
  if (rptState.dateFrom) p.date_from      = rptState.dateFrom;
  if (rptState.dateTo)   p.date_to        = rptState.dateTo;
  return p;
}

// ── 분석 실행 ─────────────────────────────────────────
function runAnalysis() {
  setLoadingState(true);

  $.getJSON('proc/ai_report_engine.php', buildParams(), function(res) {
    if (res.code !== '00') {
      showError(res.msg || 'Analysis failed');
      setLoadingState(false);
      return;
    }
    rptState.data = res;
    renderAll(res);
    setLoadingState(false);
    $('#lastAnalysisTime').text('Last Analysis: ' + new Date().toLocaleTimeString());
  }).fail(function() {
    showError('Server connection failed');
    setLoadingState(false);
  });
}

// ── 전체 렌더링 ───────────────────────────────────────
function renderAll(d) {
  renderKPI(d.summary, d.downtime);
  renderInsights(d.insights);
  renderPrediction(d.prediction);
  renderAnomalies(d.anomalies);
  renderMaintenance(d.maintenance);
  renderOptimization(d.optimization);
  updateCharts(d);
}

// ── KPI 게이지 렌더링 ─────────────────────────────────
function renderKPI(s, dt) {
  var items = [
    { id: 'oee',          val: s.avg_oee,    target: 85,  suffix: '%', label: 'OEE' },
    { id: 'availability', val: s.avg_avail,  target: 90,  suffix: '%', label: 'Availability' },
    { id: 'performance',  val: s.avg_perf,   target: 90,  suffix: '%', label: 'Performance' },
    { id: 'quality',      val: s.avg_quality,target: 95,  suffix: '%', label: 'Quality' },
  ];

  items.forEach(function(item) {
    var val    = parseFloat(item.val) || 0;
    var color  = val >= item.target ? COLORS.good : (val >= item.target * 0.85 ? COLORS.warning : COLORS.danger);
    var status = val >= item.target ? 'On Target' : (val >= item.target * 0.85 ? 'Near Target' : 'Below Target');

    $('#' + item.id + 'CurrentValue').text(val + item.suffix).css('color', color);
    $('#' + item.id + 'Status').text(status).css('color', color);

    var chartId = item.id + 'Gauge';
    if (rptState.charts[chartId]) {
      rptState.charts[chartId].data.datasets[0].data = [val, 100 - val];
      rptState.charts[chartId].data.datasets[0].backgroundColor = [color, 'rgba(255,255,255,0.06)'];
      rptState.charts[chartId].update('none');
    }
  });

  // 다운타임 게이지
  if (dt) {
    var dtVal = parseFloat(dt.dt_total_min) || 0;
    $('#downtimeCurrentValue').text(dtVal + ' min');
    $('#downtimeStatus').text(dt.dt_count + ' events');
    $('#downtimeTrend').text('Avg ' + (dt.dt_avg_min || '--') + ' min/event');
  }
}

// ── AI Insights 패널 ──────────────────────────────────
function renderInsights(insights) {
  if (!insights || !insights.length) return;

  // Predictive Analytics 카드 — prediction / trend 관련
  var predItems = insights.filter(function(i) { return i.text.indexOf('trend') !== -1 || i.text.indexOf('forecast') !== -1; });
  if (predItems.length) {
    var html = predItems.map(function(i) {
      return '<div class="prediction-item"><div class="prediction-trend" style="color:' + levelColor(i.level) + '">' + escHtml(i.text) + '</div></div>';
    }).join('');
    $('#predictionStatus').text('Analyzed').removeClass().addClass('status-badge status-badge--success');
    $('#predictiveContent .prediction-item').last().after(html);
  }

  // AI Recommendations 카드
  var recItems = insights.filter(function(i) { return i.text.indexOf('opportunity') !== -1 || i.text.indexOf('bottleneck') !== -1; });
  var recHtml  = recItems.length
    ? recItems.map(function(i) {
        return '<div class="recommendation-item" style="border-left:3px solid ' + levelColor(i.level) + ';padding:6px 10px;margin-bottom:6px;">' + escHtml(i.text) + '</div>';
      }).join('')
    : '<div class="no-recommendations">No optimization opportunities found for current period.</div>';
  $('#recommendationList').html(recHtml);
  $('#recommendationStatus').text('Ready').removeClass().addClass('status-badge status-badge--success');

  // Anomaly 상태 배지
  var anomItems = insights.filter(function(i) { return i.text.indexOf('Anomaly') !== -1; });
  if (anomItems.length) {
    var anomText = anomItems[0].text;
    var anomLevel = anomItems[0].level;
    $('#anomalyStatus').text(anomLevel === 'success' ? 'Clean' : 'Anomalies Found')
      .removeClass().addClass('status-badge ' + (anomLevel === 'success' ? 'status-badge--success' : 'status-badge--warning'));
  }
}

// ── Predictive Analytics 수치 렌더링 ─────────────────
function renderPrediction(pred) {
  if (!pred || !pred['7d']) {
    $('#oeeForecasts7d').text('N/A');
    $('#oeeForecasts30d').text('N/A');
    $('#predictionStatus').text('Insufficient data').removeClass().addClass('status-badge status-badge--processing');
    return;
  }
  $('#oeeForecasts7d').text(pred['7d'] + '%');
  $('#oeeTrend7d').text(pred.trend === 'improving' ? 'Upward trend' : (pred.trend === 'declining' ? 'Downward trend' : 'Stable'));
  $('#oeeForecasts30d').text(pred['30d'] + '%');
  $('#oeeTrend30d').text('Slope: ' + pred.slope + '%/day');
  $('#predictionStatus').text('Ready').removeClass().addClass('status-badge status-badge--success');
}

// ── 이상 감지 목록 ────────────────────────────────────
function renderAnomalies(anomalies) {
  if (!anomalies || !anomalies.length) {
    $('#anomalyList').html('<div class="no-anomalies">No anomalies detected in current data range.</div>');
    return;
  }
  var html = anomalies.map(function(a) {
    var color = a.severity === 'critical' ? COLORS.danger : COLORS.warning;
    return '<div class="anomaly-item" style="border-left:3px solid ' + color + ';padding:6px 10px;margin-bottom:6px;">' +
      '<strong>' + escHtml(a.machine) + '</strong> (' + escHtml(a.line) + ')' +
      ' — OEE: ' + a.cur_oee + '% (Z=' + a.z_score + ')' +
      ' <span style="color:' + color + ';font-size:0.8rem;">' + a.severity.toUpperCase() + '</span>' +
      '</div>';
  }).join('');
  $('#anomalyList').html(html);
}

// ── 예방정비 목록 (recommendationContent 재활용) ─────
function renderMaintenance(maint) {
  if (!maint || !maint.length) return;
  var html = maint.map(function(m) {
    var color = m.risk_level === 'danger' ? COLORS.danger : (m.risk_level === 'warning' ? COLORS.warning : COLORS.good);
    return '<div class="recommendation-item" style="border-left:3px solid ' + color + ';padding:6px 10px;margin-bottom:4px;">' +
      '<strong>' + escHtml(m.machine) + '</strong> (' + escHtml(m.line) + ')' +
      ' — Risk: <strong style="color:' + color + '">' + m.risk_score + '%</strong>' +
      ' | Events (60d): ' + m.dt_count +
      '</div>';
  }).join('');
  $('#equipmentRisk').text(maint.filter(function(m) { return m.risk_level === 'danger'; }).length + ' high-risk');
  $('#riskTrend').text('Top: ' + (maint[0] ? maint[0].machine + ' (' + maint[0].risk_score + '%)' : '--'));
}

// ── 최적화 기회 (Correlation 카드 영역 활용) ─────────
function renderOptimization(opt) {
  if (!opt || !opt.length) {
    $('#correlationStatus').text('No data').removeClass().addClass('status-badge status-badge--info');
    return;
  }
  var label = { availability: 'Avail', performance: 'Perf', quality: 'Qual' };
  var html = opt.map(function(o) {
    return '<div class="correlation-item">' +
      '<div class="correlation-factors">' + escHtml(o.line) + ' [' + escHtml(o.priority) + '] — ' + (label[o.bottleneck] || o.bottleneck) + '</div>' +
      '<div class="correlation-value" style="color:' + COLORS.info + '">+' + o.potential_gain + '%p</div>' +
      '</div>';
  }).join('');
  $('#correlationContent').html(html);
  $('#correlationStatus').text('Analyzed').removeClass().addClass('status-badge status-badge--success');
}

// ── 차트 초기화 ───────────────────────────────────────
function initCharts() {
  // KPI 게이지 (Doughnut)
  var gaugeIds = ['oee', 'availability', 'performance', 'quality', 'defectRate', 'downtime'];
  gaugeIds.forEach(function(id) {
    var canvas = document.getElementById(id + 'Gauge');
    if (!canvas) return;
    var ctx = canvas.getContext('2d');
    rptState.charts[id + 'Gauge'] = new Chart(ctx, {
      type: 'doughnut',
      data: {
        datasets: [{
          data: [0, 100],
          backgroundColor: [COLORS.muted, 'rgba(255,255,255,0.06)'],
          borderWidth: 0,
        }],
      },
      options: {
        responsive: true,
        maintainAspectRatio: true,
        circumference: 180,
        rotation: -90,
        cutout: '75%',
        plugins: { legend: { display: false }, tooltip: { enabled: false } },
      },
    });
  });

  // Predictive Trend Chart
  var trendCanvas = document.getElementById('predictiveTrendChart');
  if (trendCanvas) {
    rptState.charts['trend'] = new Chart(trendCanvas.getContext('2d'), {
      type: 'line',
      data: { labels: [], datasets: [
        { label: 'Actual OEE', data: [], borderColor: COLORS.info, backgroundColor: 'rgba(88,166,255,0.1)', tension: 0.3, fill: true, borderWidth: 2, pointRadius: 2 },
        { label: 'Forecast',   data: [], borderColor: COLORS.warning, borderDash: [5, 5], borderWidth: 2, pointRadius: 0 },
      ]},
      options: {
        responsive: true, maintainAspectRatio: false,
        scales: {
          x: { ticks: { color: COLORS.muted, maxTicksLimit: 8, font: { size: 10 } }, grid: { color: 'rgba(255,255,255,0.06)' } },
          y: { min: 0, max: 100, ticks: { color: COLORS.muted, font: { size: 10 } }, grid: { color: 'rgba(255,255,255,0.06)' } },
        },
        plugins: { legend: { labels: { color: COLORS.muted, font: { size: 10 } } } },
      },
    });
  }
}

function updateCharts(d) {
  // Trend 차트
  var tc = rptState.charts['trend'];
  if (tc && d.prediction && d.prediction.actuals) {
    var actuals  = d.prediction.actuals;
    var dates    = d.prediction.dates;
    var forecast = d.prediction['7d'] ? Array(actuals.length).fill(null).concat([d.prediction['7d']]) : [];
    var fDates   = dates.concat(d.prediction['7d'] ? ['Forecast'] : []);

    tc.data.labels                 = fDates;
    tc.data.datasets[0].data       = actuals.concat(d.prediction['7d'] ? [null] : []);
    tc.data.datasets[1].data       = Array(actuals.length).fill(null).concat(d.prediction['7d'] ? [d.prediction['7d']] : []);
    tc.update('none');
  }
}

// ── 유틸 ─────────────────────────────────────────────
function levelColor(level) {
  return { success: COLORS.good, warning: COLORS.warning, error: COLORS.danger, info: COLORS.info }[level] || COLORS.muted;
}

function escHtml(str) {
  return String(str).replace(/&/g, '&amp;').replace(/</g, '&lt;').replace(/>/g, '&gt;');
}

function setLoadingState(loading) {
  var $btn = $('#startAIAnalysisBtn');
  $btn.prop('disabled', loading).text(loading ? 'Analyzing...' : 'Start AI Analysis');
  if (loading) {
    $('#predictionStatus').text('Analyzing...').removeClass().addClass('status-badge status-badge--processing');
    $('#anomalyStatus').text('Analyzing...').removeClass().addClass('status-badge status-badge--processing');
    $('#correlationStatus').text('Analyzing...').removeClass().addClass('status-badge status-badge--processing');
    $('#recommendationStatus').text('Analyzing...').removeClass().addClass('status-badge status-badge--processing');
  }
}

function showError(msg) {
  $('#anomalyList').html('<div style="color:' + COLORS.danger + ';padding:12px;">' + escHtml(msg) + '</div>');
}
