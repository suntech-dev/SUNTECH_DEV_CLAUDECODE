/**
 * AI Intelligence Dashboard JavaScript
 * Phase 1: Statistical AI (OEE Prediction, Anomaly Detection, Predictive Maintenance)
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

// 새로고침 주기 (ms)
const REFRESH_INTERVAL = 60000;

let aiState = {
  factory: '',
  line:    '',
  machine: '',
  forecastChart: null,
  refreshTimer:  null,
};

// ============================================================
// 필터 시스템 (Factory → Line → Machine 연계)
// dashboard.js 와 동일한 엔드포인트 사용
// ============================================================
function initFilterSystem() {
  // Factory 목록 로드
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

  $('#aiRefreshBtn').on('click', function() {
    refreshAll();
  });
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
}

function updateLastUpdateTime() {
  const now = new Date();
  const hh = String(now.getHours()).padStart(2,'0');
  const mm = String(now.getMinutes()).padStart(2,'0');
  const ss = String(now.getSeconds()).padStart(2,'0');
  $('#aiLastUpdateTime').text('Updated: ' + hh + ':' + mm + ':' + ss);
}

// ============================================================
// 1. OEE 예측
// ============================================================
function loadPrediction() {
  $.getJSON('proc/ai_oee_prediction.php', getFilterParams(), function(data) {
    if (data.code !== '00') return;

    // 요약 카드 업데이트
    const curOee = data.current_oee !== null ? data.current_oee : '--';
    $('#aiPredCurrentOee').text(curOee !== '--' ? curOee + '%' : '--');

    // 예측값 평균
    let forecastAvg = '--';
    if (data.forecast && data.forecast.length > 0) {
      const sum = data.forecast.reduce((s, f) => s + f.oee, 0);
      forecastAvg = (sum / data.forecast.length).toFixed(1);
      $('#aiPredSub').text('Next 4H avg: ' + forecastAvg + '%');
    } else {
      $('#aiPredSub').text('Insufficient data');
    }

    // 트렌드 배지
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

    // 예측 차트 업데이트
    renderForecastChart(data);

    // 라인 건강지수도 예측 데이터 이후 업데이트
    loadLineHealth();
  }).fail(function() {
    $('#aiPredSub').text('API error');
  });
}

// ============================================================
// 1-1. OEE 예측 차트 (Chart.js)
// ============================================================
function renderForecastChart(predData) {
  const canvas = document.getElementById('aiOeeForecastChart');
  if (!canvas) return;

  // 오늘 실제 데이터 (dummy: 예측 API에 today_data 없으면 빈 배열 사용)
  const actualLabels   = [];
  const actualValues   = [];
  const forecastLabels = [];
  const forecastValues = [];
  const ciUpper        = [];
  const ciLower        = [];

  // 예측 데이터
  if (predData.forecast) {
    predData.forecast.forEach(function(f) {
      forecastLabels.push(f.label);
      forecastValues.push(f.oee);
      ciUpper.push(f.upper);
      ciLower.push(f.lower);
    });
  }

  // 마지막 실제값과 예측 첫 점을 이어줄 연결 포인트
  const connectOee = predData.current_oee || null;
  const connectLabel = predData.current_hour !== undefined
    ? String(predData.current_hour).padStart(2, '0') + ':00'
    : 'Now';

  // 전체 라벨: 연결점 + 예측
  const allLabels = connectOee ? [connectLabel, ...forecastLabels] : forecastLabels;
  const forecastFull = connectOee ? [connectOee, ...forecastValues] : forecastValues;
  const ciUpperFull  = connectOee ? [connectOee, ...ciUpper]  : ciUpper;
  const ciLowerFull  = connectOee ? [connectOee, ...ciLower]  : ciLower;

  const chartData = {
    labels: allLabels,
    datasets: [
      {
        label: 'AI Forecast',
        data: forecastFull,
        borderColor: AI_COLORS.forecast,
        backgroundColor: 'transparent',
        borderDash: [6, 4],
        borderWidth: 2.5,
        pointBackgroundColor: AI_COLORS.forecast,
        pointRadius: (ctx) => ctx.dataIndex === 0 ? 5 : 4,
        tension: 0.3,
        order: 1,
      },
      {
        label: 'CI Upper',
        data: ciUpperFull,
        borderColor: 'transparent',
        backgroundColor: AI_COLORS.ci,
        fill: '+1',
        pointRadius: 0,
        tension: 0.3,
        order: 2,
      },
      {
        label: 'CI Lower',
        data: ciLowerFull,
        borderColor: 'transparent',
        backgroundColor: AI_COLORS.ci,
        fill: false,
        pointRadius: 0,
        tension: 0.3,
        order: 3,
      },
    ]
  };

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
    if (data.code !== '00') {
      renderAnomalyEmpty('API error');
      return;
    }

    const summary = data.summary || {};
    $('#aiAnomalyTotal').text(summary.total || 0);

    if (summary.critical > 0) {
      $('#aiAnomalySub').text(summary.critical + ' critical · ' + summary.warning + ' warning');
      $('#aiAnomalyCriticalBadge')
        .show()
        .text(summary.critical + ' CRITICAL')
        .removeClass('ai-status-badge--warning ai-status-badge--normal')
        .addClass('ai-status-badge--danger');
    } else if (summary.warning > 0) {
      $('#aiAnomalySub').text(summary.warning + ' warnings detected');
      $('#aiAnomalyCriticalBadge')
        .show()
        .text(summary.warning + ' WARNING')
        .removeClass('ai-status-badge--danger ai-status-badge--normal')
        .addClass('ai-status-badge--warning');
    } else {
      $('#aiAnomalySub').text('All machines normal');
      $('#aiAnomalyCriticalBadge').hide();
    }

    // 이상 감지 헤더
    if (summary.total > 0) {
      $('#aiAnomalyHeaderCount').show();
      $('#aiAnomalyHeaderText').text(summary.total + ' anomalies detected');
    } else {
      $('#aiAnomalyHeaderCount').hide();
    }

    renderAnomalyList(data.anomalies || [], data.cascade_alerts || []);
  }).fail(function() {
    renderAnomalyEmpty('Failed to load');
  });
}

function renderAnomalyList(anomalies, cascadeAlerts) {
  const $list = $('#aiAnomalyList').empty();

  // 연쇄 이상 경고 먼저 표시
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
// 3. 예방정비 위험도
// ============================================================
function loadMaintenanceRisk() {
  $.getJSON('proc/ai_maintenance.php', getFilterParams(), function(data) {
    if (data.code !== '00') {
      renderMaintenanceEmpty('API error');
      return;
    }

    const summary = data.summary || {};
    $('#aiMaintDanger').text(summary.danger || 0);
    $('#aiMaintSub').text((summary.warning || 0) + ' caution · ' + (summary.normal || 0) + ' normal');

    if (summary.warning > 0) {
      $('#aiMaintWarnBadge').show().text(summary.warning + ' CAUTION');
    } else {
      $('#aiMaintWarnBadge').hide();
    }

    renderMaintenanceList(data.machines || []);
  }).fail(function() {
    renderMaintenanceEmpty('Failed to load');
  });
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
// 4. 라인 건강지수 (예방정비 데이터 재활용)
// ============================================================
function loadLineHealth() {
  // 라인별 평균 OEE 7일 조회
  $.getJSON('proc/ai_maintenance.php', Object.assign({}, getFilterParams(), { limit: 50 }), function(data) {
    if (data.code !== '00') return;

    // 머신 → 라인별로 OEE 집계
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
      $('#aiHealthList').html(
        '<div class="ai-empty-state"><div>No line data available</div></div>'
      );
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
        // OEE 없으면 위험도 역산
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

    // 요약 카드 평균 건강지수 업데이트
    const avgHealth = healthCount > 0 ? (totalHealth / healthCount).toFixed(1) : '--';
    $('#aiHealthAvg').text(avgHealth !== '--' ? avgHealth + '%' : '--');
    $('#aiHealthSub').text(lines.length + ' lines monitored');

    // 건강지수 상태 배지
    const avgVal = parseFloat(avgHealth);
    if (!isNaN(avgVal)) {
      let cls, label;
      if (avgVal >= 80)      { cls = 'ai-status-badge--normal';  label = 'HEALTHY'; }
      else if (avgVal >= 60) { cls = 'ai-status-badge--warning'; label = 'CAUTION'; }
      else                   { cls = 'ai-status-badge--danger';  label = 'AT RISK'; }
      $('#aiHealthStatusBadge').show().removeClass('ai-status-badge--normal ai-status-badge--warning ai-status-badge--danger').addClass(cls).text(label);
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

  // 자동 새로고침
  aiState.refreshTimer = setInterval(function() {
    refreshAll();
  }, REFRESH_INTERVAL);
});
