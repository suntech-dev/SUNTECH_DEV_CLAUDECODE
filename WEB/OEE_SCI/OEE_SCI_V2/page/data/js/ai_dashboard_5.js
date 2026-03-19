/**
 * AI Intelligence Dashboard JavaScript v5
 * v4(ai_dashboard.js) 대비 수정:
 *  - renderForecastChart(): today_data 기반 Actual OEE solid 라인 추가
 *  - loadMaintenanceRisk() / loadLineHealth(): proc/ai_maintenance_5.php 호출
 *  - updateLineHealthSubtitle(): date_range 에 맞게 서브타이틀 동적 변경
 */

const AI_COLORS = {
  primary:    '#0070f2',
  forecast:   '#f5a623',
  ci:         'rgba(245,166,35,0.15)',
  danger:     '#f85149',
  warning:    '#d29922',
  normal:     '#3fb950',
  info:       '#58a6ff',
  chartGrid:  'rgba(255,255,255,0.08)',
  chartText:  '#8b949e',
};

const REFRESH_INTERVAL = 60000;

let aiState = {
  factory: '',
  line:    '',
  machine: '',
  forecastChart: null,
  refreshTimer:  null,
};

// ============================================================
// 필터 시스템
// ============================================================
function initFilterSystem() {
  $.getJSON('../manage/proc/factory.php', function(res) {
    if (!res.success || !res.data) return;
    res.data.forEach(function(item) {
      $('#factoryFilterSelect').append(
        $('<option>').val(item.idx).text(item.factory_name)
      );
    });
  });

  $('#factoryFilterSelect').on('change', function() {
    aiState.factory = $(this).val();
    aiState.line    = '';
    aiState.machine = '';
    $('#factoryLineFilterSelect').html('<option value="">All Line</option>').prop('disabled', true);
    $('#factoryLineMachineFilterSelect').html('<option value="">All Machine</option>').prop('disabled', true);

    if (aiState.factory) {
      $.getJSON('../manage/proc/line.php', { factory_filter: aiState.factory }, function(res) {
        if (!res.success || !res.data) return;
        $('#factoryLineFilterSelect').prop('disabled', false);
        res.data.forEach(function(item) {
          $('#factoryLineFilterSelect').append(
            $('<option>').val(item.idx).text(item.line_name)
          );
        });
      });
    }
    refreshAll();
  });

  $('#factoryLineFilterSelect').on('change', function() {
    aiState.line    = $(this).val();
    aiState.machine = '';
    $('#factoryLineMachineFilterSelect').html('<option value="">All Machine</option>').prop('disabled', true);

    if (aiState.line) {
      const params = { line_filter: aiState.line };
      if (aiState.factory) params.factory_filter = aiState.factory;
      $.getJSON('../manage/proc/machine.php', params, function(res) {
        if (!res.success || !res.data) return;
        $('#factoryLineMachineFilterSelect').prop('disabled', false);
        res.data.forEach(function(item) {
          $('#factoryLineMachineFilterSelect').append(
            $('<option>').val(item.idx).text(item.machine_no)
          );
        });
      });
    }
    refreshAll();
  });

  $('#factoryLineMachineFilterSelect').on('change', function() {
    aiState.machine = $(this).val();
    refreshAll();
  });

  $('#aiRefreshBtn').on('click', function() { refreshAll(); });
}

function getFilterParams() {
  const p = {};
  if (aiState.factory) p.factory_filter = aiState.factory;
  if (aiState.line)    p.line_filter    = aiState.line;
  if (aiState.machine) p.machine_filter = aiState.machine;
  return p;
}

// ============================================================
// 전체 새로고침
// ============================================================
function refreshAll() {
  loadPrediction();
  loadAnomalyDetection();
  loadMaintenanceRisk();
  updateLastUpdateTime();
  updateLineHealthSubtitle();
}

function updateLastUpdateTime() {
  const now = new Date();
  const hh = String(now.getHours()).padStart(2,'0');
  const mm = String(now.getMinutes()).padStart(2,'0');
  const ss = String(now.getSeconds()).padStart(2,'0');
  $('#aiLastUpdateTime').text('Updated: ' + hh + ':' + mm + ':' + ss);
}

// Line Health 서브타이틀 동적 업데이트 (v5 신규)
function updateLineHealthSubtitle() {
  const sel = document.getElementById('dateRangeSelect');
  const rangeMap = {
    today:     '7-day OEE average per line',
    yesterday: '7-day OEE average per line',
    '7d':      '7-day OEE average per line',
    '30d':     '30-day OEE average per line',
  };
  const label = sel ? (rangeMap[sel.value] || '7-day OEE average per line') : '7-day OEE average per line';
  const el = document.querySelector('.ai-health-subtitle');
  if (el) el.textContent = 'Based on ' + label;
}

// ============================================================
// 1. OEE 예측 (기본 구현 — ai_dashboard_5.php 에서 오버라이드)
// ============================================================
function loadPrediction() {
  $.getJSON('proc/ai_oee_prediction_5.php', getFilterParams(), function(data) {
    if (data.code !== '00') return;

    const curOee = data.current_oee !== null ? data.current_oee : '--';
    $('#aiPredCurrentOee').text(curOee !== '--' ? curOee + '%' : '--');

    let forecastAvg = '--';
    if (data.forecast && data.forecast.length > 0) {
      const sum = data.forecast.reduce((s, f) => s + f.oee, 0);
      forecastAvg = (sum / data.forecast.length).toFixed(1);
      $('#aiPredSub').text('Next 4H avg: ' + forecastAvg + '%');
    } else {
      $('#aiPredSub').text('Insufficient data');
    }

    const trendMap = {
      up:     { cls: 'ai-trend-badge--up',     text: 'Trending Up' },
      down:   { cls: 'ai-trend-badge--down',   text: 'Trending Down' },
      stable: { cls: 'ai-trend-badge--stable', text: 'Stable' },
    };
    const trend = trendMap[data.trend] || trendMap.stable;
    $('#aiPredTrendBadge')
      .removeClass('ai-trend-badge--up ai-trend-badge--down ai-trend-badge--stable')
      .addClass(trend.cls)
      .text(trend.text);

    renderForecastChart(data);
    loadLineHealth();
  }).fail(function() {
    $('#aiPredSub').text('API error');
  });
}

// ============================================================
// 1-1. OEE 예측 차트 — v5: Actual OEE solid 라인 추가
// ============================================================
function renderForecastChart(predData) {
  const canvas = document.getElementById('aiOeeForecastChart');
  if (!canvas) return;

  // 오늘 실제 OEE 데이터 (today_data: [{hour, label, oee}])
  const todayArr     = predData.today_data || [];
  const actualLabels = todayArr.map(function(d) { return d.label; });
  const actualValues = todayArr.map(function(d) { return d.oee; });

  // 예측 데이터
  const forecastArr    = predData.forecast || [];
  const forecastLabels = forecastArr.map(function(f) { return f.label; });
  const forecastValues = forecastArr.map(function(f) { return f.oee; });
  const ciUpper        = forecastArr.map(function(f) { return f.upper; });
  const ciLower        = forecastArr.map(function(f) { return f.lower; });

  // 연결점: 마지막 실제값 → 예측 첫 점 이음
  const connectOee   = predData.current_oee || null;
  const connectLabel = predData.current_hour !== undefined
    ? String(predData.current_hour).padStart(2, '0') + ':00' : 'Now';

  // 실제 라벨과 예측 라벨 합치기 (중복 연결점 방지)
  const allLabels = actualLabels.length > 0
    ? [...actualLabels, ...forecastLabels]
    : (connectOee ? [connectLabel, ...forecastLabels] : forecastLabels);

  const nActual   = actualLabels.length;
  const nForecast = forecastLabels.length;
  const nTotal    = allLabels.length;

  // Actual OEE: 실제 구간만 값, 나머지 null
  const actualFull = actualValues.length > 0
    ? [...actualValues, ...Array(nForecast).fill(null)]
    : [];

  // AI Forecast: 실제 구간은 null, 연결점부터 예측
  let forecastFull, ciUpperFull, ciLowerFull;
  if (nActual > 0 && connectOee !== null) {
    // 실제 마지막 포인트와 예측을 연결 (nActual-1 위치에 connectOee 중복)
    forecastFull = [...Array(nActual - 1).fill(null), connectOee, ...forecastValues];
    ciUpperFull  = [...Array(nActual - 1).fill(null), connectOee, ...ciUpper];
    ciLowerFull  = [...Array(nActual - 1).fill(null), connectOee, ...ciLower];
  } else if (connectOee !== null) {
    forecastFull = [connectOee, ...forecastValues];
    ciUpperFull  = [connectOee, ...ciUpper];
    ciLowerFull  = [connectOee, ...ciLower];
  } else {
    forecastFull = forecastValues;
    ciUpperFull  = ciUpper;
    ciLowerFull  = ciLower;
  }

  const datasets = [];

  // Actual OEE solid 라인 (v5 신규)
  if (actualFull.length > 0) {
    datasets.push({
      label: 'Actual OEE',
      data: actualFull,
      borderColor: AI_COLORS.primary,
      backgroundColor: 'transparent',
      borderWidth: 2.5,
      borderDash: [],
      pointBackgroundColor: AI_COLORS.primary,
      pointRadius: 3,
      tension: 0.3,
      order: 0,
      spanGaps: false,
    });
  }

  // AI Forecast dashed 라인
  datasets.push({
    label: 'AI Forecast',
    data: forecastFull,
    borderColor: AI_COLORS.forecast,
    backgroundColor: 'transparent',
    borderDash: [6, 4],
    borderWidth: 2.5,
    pointBackgroundColor: AI_COLORS.forecast,
    pointRadius: (ctx) => {
      const v = ctx.raw;
      return (v !== null && v !== undefined) ? 4 : 0;
    },
    tension: 0.3,
    order: 1,
    spanGaps: false,
  });

  // CI Upper
  datasets.push({
    label: 'CI Upper',
    data: ciUpperFull,
    borderColor: 'transparent',
    backgroundColor: AI_COLORS.ci,
    fill: '+1',
    pointRadius: 0,
    tension: 0.3,
    order: 2,
    spanGaps: false,
  });

  // CI Lower
  datasets.push({
    label: 'CI Lower',
    data: ciLowerFull,
    borderColor: 'transparent',
    backgroundColor: AI_COLORS.ci,
    fill: false,
    pointRadius: 0,
    tension: 0.3,
    order: 3,
    spanGaps: false,
  });

  const chartData = { labels: allLabels, datasets: datasets };

  const options = {
    responsive: true,
    maintainAspectRatio: false,
    interaction: { mode: 'index', intersect: false },
    plugins: {
      legend: { display: false },
      tooltip: {
        backgroundColor: '#1c2128',
        borderColor: '#30363d',
        borderWidth: 1,
        titleColor: '#e6edf3',
        bodyColor: '#8b949e',
        callbacks: {
          label: function(ctx) {
            if (ctx.raw === null || ctx.raw === undefined) return null;
            if (ctx.dataset.label === 'CI Upper') return 'CI Upper: ' + ctx.parsed.y + '%';
            if (ctx.dataset.label === 'CI Lower') return 'CI Lower: ' + ctx.parsed.y + '%';
            return ctx.dataset.label + ': ' + ctx.parsed.y + '%';
          }
        }
      }
    },
    scales: {
      x: {
        grid: { color: AI_COLORS.chartGrid },
        ticks: { color: AI_COLORS.chartText, font: { size: 11 } },
      },
      y: {
        min: 0, max: 100,
        grid: { color: AI_COLORS.chartGrid },
        ticks: {
          color: AI_COLORS.chartText,
          font: { size: 11 },
          callback: (v) => v + '%',
        }
      }
    }
  };

  if (aiState.forecastChart) {
    aiState.forecastChart.data    = chartData;
    aiState.forecastChart.options = options;
    aiState.forecastChart.update('none');
  } else {
    aiState.forecastChart = new Chart(canvas.getContext('2d'), {
      type: 'line',
      data: chartData,
      options: options,
    });
  }
}

// ============================================================
// 2. 이상 감지
// ============================================================
function loadAnomalyDetection() {
  $.getJSON('proc/ai_anomaly.php', getFilterParams(), function(data) {
    if (data.code !== '00') { renderAnomalyEmpty('API error'); return; }

    const summary = data.summary || {};
    $('#aiAnomalyTotal').text(summary.total || 0);

    if (summary.critical > 0) {
      $('#aiAnomalySub').text(summary.critical + ' critical · ' + summary.warning + ' warning');
      $('#aiAnomalyCriticalBadge').show().text(summary.critical + ' CRITICAL')
        .removeClass('ai-status-badge--warning ai-status-badge--normal').addClass('ai-status-badge--danger');
    } else if (summary.warning > 0) {
      $('#aiAnomalySub').text(summary.warning + ' warnings detected');
      $('#aiAnomalyCriticalBadge').show().text(summary.warning + ' WARNING')
        .removeClass('ai-status-badge--danger ai-status-badge--normal').addClass('ai-status-badge--warning');
    } else {
      $('#aiAnomalySub').text('All machines normal');
      $('#aiAnomalyCriticalBadge').hide();
    }

    if (summary.total > 0) {
      $('#aiAnomalyHeaderCount').show();
      $('#aiAnomalyHeaderText').text(summary.total + ' anomalies detected');
    } else {
      $('#aiAnomalyHeaderCount').hide();
    }

    renderAnomalyList(data.anomalies || [], data.cascade_alerts || []);
  }).fail(function() { renderAnomalyEmpty('Failed to load'); });
}

function renderAnomalyList(anomalies, cascadeAlerts) {
  const $list = $('#aiAnomalyList').empty();

  cascadeAlerts.forEach(function(alert) {
    $list.append(
      '<div class="ai-anomaly-item ai-anomaly-item--critical" style="grid-template-columns:1fr;">' +
        '<div>' +
          '<div class="ai-anomaly-item__machine">Cascade Alert: ' + escHtml(alert.line_name) + '</div>' +
          '<div class="ai-anomaly-item__line">' + escHtml(alert.message) + '</div>' +
        '</div>' +
      '</div>'
    );
  });

  if (anomalies.length === 0 && cascadeAlerts.length === 0) {
    $list.append(
      '<div class="ai-empty-state">' +
        '<div class="ai-empty-state__icon">&#10003;</div>' +
        '<div>No anomalies detected</div>' +
        '<div style="font-size:0.75rem;">All machines operating within normal range</div>' +
      '</div>'
    );
    return;
  }

  anomalies.forEach(function(item) {
    const severityClass = 'ai-anomaly-item--' + (item.severity === 'critical' ? 'critical' : 'warning');
    const oeeColor = item.current_oee < 60 ? AI_COLORS.danger : AI_COLORS.warning;

    const detailsHtml = (item.details || []).map(function(d) {
      return '<div style="font-size:0.72rem;color:' + (d.severity === 'critical' ? AI_COLORS.danger : AI_COLORS.warning) + ';">' +
        escHtml(d.type) + ': ' + d.value + '% (avg ' + d.baseline + '%, Z=' + d.z_score + ')' +
      '</div>';
    }).join('');

    $list.append(
      '<div class="ai-anomaly-item ' + severityClass + '">' +
        '<div>' +
          '<div class="ai-anomaly-item__machine">' + escHtml(item.machine_no) + '</div>' +
          '<div class="ai-anomaly-item__line">' + escHtml(item.line_name) + '</div>' +
          detailsHtml +
        '</div>' +
        '<div></div>' +
        '<div class="ai-anomaly-item__oee" style="color:' + oeeColor + ';">' + item.current_oee + '%</div>' +
      '</div>'
    );
  });
}

function renderAnomalyEmpty(msg) {
  $('#aiAnomalyList').html(
    '<div class="ai-empty-state"><div class="ai-empty-state__icon">!</div><div>' + escHtml(msg) + '</div></div>'
  );
}

// ============================================================
// 3. 예방정비 위험도 — v5: ai_maintenance_5.php 호출
// ============================================================
function loadMaintenanceRisk() {
  $.getJSON('proc/ai_maintenance_5.php', getFilterParams(), function(data) {
    if (data.code !== '00') { renderMaintenanceEmpty('API error'); return; }

    const summary = data.summary || {};
    const totalHighRisk = (summary.danger || 0) + (summary.warning || 0);
    $('#aiMaintDanger').text(totalHighRisk);
    $('#aiMaintSub').text((summary.danger || 0) + ' danger · ' + (summary.warning || 0) + ' caution');

    if (summary.danger > 0) {
      $('#aiMaintWarnBadge').show().text(summary.danger + ' DANGER')
        .removeClass('ai-status-badge--warning').addClass('ai-status-badge--danger');
    } else if (summary.warning > 0) {
      $('#aiMaintWarnBadge').show().text(summary.warning + ' CAUTION')
        .removeClass('ai-status-badge--danger').addClass('ai-status-badge--warning');
    } else {
      $('#aiMaintWarnBadge').hide();
    }

    renderMaintenanceList(data.machines || []);
  }).fail(function() { renderMaintenanceEmpty('Failed to load'); });
}

function renderMaintenanceList(machines) {
  const $list = $('#aiMaintenanceList').empty();

  if (machines.length === 0) {
    $list.html(
      '<div class="ai-empty-state">' +
        '<div class="ai-empty-state__icon">&#10003;</div>' +
        '<div>No maintenance risk data</div>' +
      '</div>'
    );
    return;
  }

  machines.forEach(function(m, idx) {
    const levelMap = {
      danger:  { badgeCls: 'ai-status-badge--danger',  barCls: 'ai-risk-bar--danger',  label: 'DANGER' },
      warning: { badgeCls: 'ai-status-badge--warning', barCls: 'ai-risk-bar--warning', label: 'CAUTION' },
      normal:  { badgeCls: 'ai-status-badge--normal',  barCls: 'ai-risk-bar--normal',  label: 'NORMAL' },
    };
    const lv = levelMap[m.risk_level] || levelMap.normal;
    const barWidth = Math.min(100, m.risk_score) + '%';

    let detailHtml = '';
    if (m.details) {
      const d = m.details;
      const parts = [];
      if (d.avg_oee_7d !== undefined) parts.push('Avg OEE: ' + d.avg_oee_7d + '%');
      if (d.recent_30d_cnt !== undefined) parts.push('DT(30d): ' + d.recent_30d_cnt);
      if (d.hours_since_last_dt !== undefined) parts.push('Last DT: ' + d.hours_since_last_dt + 'H ago');
      detailHtml = '<div style="font-size:0.72rem;color:' + AI_COLORS.chartText + ';margin-top:2px;">' + parts.join(' | ') + '</div>';
    }

    $list.append(
      '<div class="ai-maintenance-item">' +
        '<div class="ai-maintenance-item__header">' +
          '<div>' +
            '<span style="font-size:0.7rem;color:' + AI_COLORS.chartText + ';">#' + (idx+1) + ' </span>' +
            '<span class="ai-maintenance-item__machine">' + escHtml(m.machine_no) + '</span>' +
            '<span class="ai-maintenance-item__line" style="margin-left:6px;">(' + escHtml(m.line_name) + ')</span>' +
          '</div>' +
          '<span class="ai-status-badge ' + lv.badgeCls + '">' + lv.label + ' ' + m.risk_score + '</span>' +
        '</div>' +
        detailHtml +
        '<div class="ai-risk-bar-wrap" style="margin-top:6px;">' +
          '<div class="ai-risk-bar ' + lv.barCls + '" style="width:' + barWidth + ';"></div>' +
        '</div>' +
      '</div>'
    );
  });
}

function renderMaintenanceEmpty(msg) {
  $('#aiMaintenanceList').html(
    '<div class="ai-empty-state"><div class="ai-empty-state__icon">!</div><div>' + escHtml(msg) + '</div></div>'
  );
}

// ============================================================
// 4. 라인 건강지수 — v5: ai_maintenance_5.php 호출
// ============================================================
function loadLineHealth() {
  $.getJSON('proc/ai_maintenance_5.php', Object.assign({}, getFilterParams(), { limit: 50 }), function(data) {
    if (data.code !== '00') return;

    const lineMap = {};
    (data.machines || []).forEach(function(m) {
      const line = m.line_name;
      if (!lineMap[line]) lineMap[line] = { oees: [], risks: [] };
      if (m.details && m.details.avg_oee_7d !== undefined) {
        lineMap[line].oees.push(m.details.avg_oee_7d);
      }
      lineMap[line].risks.push(m.risk_score);
    });

    const lines = Object.keys(lineMap).sort();
    if (lines.length === 0) {
      $('#aiHealthList').html('<div class="ai-empty-state"><div>No line data available</div></div>');
      $('#aiHealthAvg').text('--');
      return;
    }

    let totalHealth = 0;
    let healthCount = 0;

    const $list = $('#aiHealthList').empty();
    lines.forEach(function(line) {
      const entry = lineMap[line];
      let health = 0;
      if (entry.oees.length > 0) {
        health = entry.oees.reduce((a, b) => a + b, 0) / entry.oees.length;
      } else {
        const avgRisk = entry.risks.reduce((a, b) => a + b, 0) / entry.risks.length;
        health = Math.max(0, 100 - avgRisk);
      }
      health = Math.round(health * 10) / 10;
      totalHealth += health;
      healthCount++;

      let barColor, badgeCls, badgeLabel;
      if (health >= 80)      { barColor = AI_COLORS.normal;  badgeCls = 'ai-status-badge--normal';  badgeLabel = 'Normal'; }
      else if (health >= 60) { barColor = AI_COLORS.warning; badgeCls = 'ai-status-badge--warning'; badgeLabel = 'Caution'; }
      else                   { barColor = AI_COLORS.danger;  badgeCls = 'ai-status-badge--danger';  badgeLabel = 'Danger'; }

      $list.append(
        '<div class="ai-health-item">' +
          '<div class="ai-health-item__line" title="' + escHtml(line) + '">' + escHtml(line) + '</div>' +
          '<div class="ai-health-bar-wrap">' +
            '<div class="ai-health-bar" style="width:' + health + '%;background:' + barColor + ';"></div>' +
          '</div>' +
          '<div class="ai-health-item__pct">' + health + '%</div>' +
          '<span class="ai-status-badge ' + badgeCls + '">' + badgeLabel + '</span>' +
        '</div>'
      );
    });

    const avgHealth = healthCount > 0 ? (totalHealth / healthCount).toFixed(1) : '--';
    $('#aiHealthAvg').text(avgHealth !== '--' ? avgHealth + '%' : '--');
    $('#aiHealthSub').text(lines.length + ' lines monitored');

    const avgVal = parseFloat(avgHealth);
    if (!isNaN(avgVal)) {
      let cls, label;
      if (avgVal >= 80)      { cls = 'ai-status-badge--normal';  label = 'HEALTHY'; }
      else if (avgVal >= 60) { cls = 'ai-status-badge--warning'; label = 'CAUTION'; }
      else                   { cls = 'ai-status-badge--danger';  label = 'AT RISK'; }
      $('#aiHealthStatusBadge').show()
        .removeClass('ai-status-badge--normal ai-status-badge--warning ai-status-badge--danger')
        .addClass(cls).text(label);
    }
  });
}

// ============================================================
// 유틸리티
// ============================================================
function escHtml(str) {
  if (!str) return '';
  return String(str)
    .replace(/&/g, '&amp;')
    .replace(/</g, '&lt;')
    .replace(/>/g, '&gt;')
    .replace(/"/g, '&quot;');
}

// ============================================================
// 초기화
// ============================================================
$(document).ready(function() {
  initFilterSystem();
  refreshAll();

  aiState.refreshTimer = setInterval(function() {
    refreshAll();
  }, REFRESH_INTERVAL);
});
