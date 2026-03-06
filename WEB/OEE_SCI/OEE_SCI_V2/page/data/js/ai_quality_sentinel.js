/**
 * AI Quality Sentinel — JavaScript
 * Pareto 차트 / 시간대 히트맵 / 기계 위험 순위 / OEE 상관관계 렌더링
 */

(function () {
  'use strict';

  let paretoChart  = null;
  let refreshTimer = null;
  const REFRESH_MS = 120000; // 2분

  // ── 필터 수집 ────────────────────────────────────────────────────
  function getFilters() {
    return {
      factory_filter: document.getElementById('factoryFilterSelect')?.value || '',
      line_filter:    document.getElementById('factoryLineFilterSelect')?.value || '',
      machine_filter: document.getElementById('factoryLineMachineFilterSelect')?.value || '',
      days: 30,
    };
  }

  function buildQS(filters) {
    return Object.entries(filters)
      .filter(([, v]) => v !== '' && v !== null && v !== undefined)
      .map(([k, v]) => `${encodeURIComponent(k)}=${encodeURIComponent(v)}`)
      .join('&');
  }

  // ── 메인 로드 ────────────────────────────────────────────────────
  async function loadAIQualitySentinel() {
    const loading = document.getElementById('aiQsLoading');
    const content = document.getElementById('aiQsContent');
    if (!loading || !content) return;

    loading.style.display = 'flex';
    content.style.display  = 'none';

    try {
      const qs  = buildQS(getFilters());
      const res = await fetch(`proc/ai_quality_sentinel.php?${qs}`);
      const data = await res.json();

      if (data.code !== '00') throw new Error(data.msg || 'API 오류');

      loading.style.display = 'none';
      content.style.display = 'block';

      renderSummary(data);
      renderParetoChart(data);
      renderHourlyHeatmap(data);
      renderMachineRanking(data);
      renderCorrelation(data);

      const ts = document.getElementById('aiQsLastUpdate');
      if (ts) ts.textContent = '업데이트: ' + new Date().toLocaleTimeString('ko-KR');

    } catch (err) {
      console.error('[AI Quality Sentinel]', err);
      if (loading) {
        loading.innerHTML = '<span style="color:var(--sap-negative);">⚠️ AI 분석 로드 실패 — 잠시 후 재시도됩니다.</span>';
        loading.style.display = 'flex';
      }
    }
  }

  // ── 요약 카드 4개 ────────────────────────────────────────────────
  function renderSummary(data) {
    const grid = document.getElementById('aiQsSummaryGrid');
    if (!grid) return;

    const peakLabel = data.peak_hour !== null
      ? String(data.peak_hour).padStart(2, '0') + ':00' : '-';
    const topType = data.summary?.top_type || '-';
    const topPct  = data.pareto?.length ? data.pareto[0].pct + '%' : '-';
    const corr    = data.oee_correlation?.coefficient;
    const corrTxt = corr !== null && corr !== undefined ? corr.toFixed(2) : '-';
    const corrClr = (corr !== null && corr !== undefined && corr <= -0.3)
      ? 'var(--sap-negative)' : 'var(--sap-positive)';

    grid.innerHTML = `
      <div class="ai-summary-card">
        <span class="ai-summary-card__label">총 불량건수 (${data.analysis_days}일)</span>
        <div class="ai-summary-card__value">${Number(data.total_defects).toLocaleString()}</div>
        <div class="ai-summary-card__sub">1위: ${topType} (${topPct})</div>
      </div>
      <div class="ai-summary-card">
        <span class="ai-summary-card__label">Pareto 핵심 유형 수</span>
        <div class="ai-summary-card__value">${data.pareto_summary?.top_n_types ?? '-'}</div>
        <div class="ai-summary-card__sub">전체 불량의 80% 차지</div>
      </div>
      <div class="ai-summary-card">
        <span class="ai-summary-card__label">불량 집중 시간대</span>
        <div class="ai-summary-card__value">${peakLabel}</div>
        <div class="ai-summary-card__sub">가장 많은 불량 발생 시각</div>
      </div>
      <div class="ai-summary-card">
        <span class="ai-summary-card__label">불량↔OEE 상관계수</span>
        <div class="ai-summary-card__value" style="color:${corrClr};">${corrTxt}</div>
        <div class="ai-summary-card__sub">${getCorrelationLabel(data.oee_correlation?.insight)}</div>
      </div>
    `;
  }

  function getCorrelationLabel(insight) {
    const map = {
      strong_negative: '↑불량 → ↓OEE (강한 음의 상관)',
      weak_negative:   '↑불량 → ↓OEE (약한 음의 상관)',
      strong_positive: '양의 상관관계',
      weak_positive:   '약한 양의 상관관계',
      no_correlation:  '뚜렷한 상관관계 없음',
    };
    return map[insight] || '데이터 부족';
  }

  // ── Pareto 차트 ──────────────────────────────────────────────────
  function renderParetoChart(data) {
    const ctx = document.getElementById('aiQsParetoChart');
    if (!ctx) return;

    if (paretoChart) { paretoChart.destroy(); paretoChart = null; }

    const subtitle = document.getElementById('aiQsParetoSubtitle');
    if (subtitle && data.pareto_summary) {
      subtitle.textContent =
        `상위 ${data.pareto_summary.top_n_types}개 유형 = 전체의 80%`;
    }

    if (!data.pareto?.length) {
      ctx.parentElement.innerHTML =
        '<div class="ai-empty-state"><span>불량 유형 데이터 없음</span></div>';
      return;
    }

    const labels    = data.pareto.map(p => p.name);
    const counts    = data.pareto.map(p => p.count);
    const cumPcts   = data.pareto.map(p => p.cum_pct);
    const barColors = data.pareto.map(p =>
      p.cum_pct <= 80 ? '#da1e28' : '#e67e22'
    );

    paretoChart = new Chart(ctx, {
      type: 'bar',
      data: {
        labels,
        datasets: [
          {
            type: 'bar',
            label: '불량 건수',
            data: counts,
            backgroundColor: barColors,
            yAxisID: 'y',
            order: 2,
          },
          {
            type: 'line',
            label: '누적 비율(%)',
            data: cumPcts,
            borderColor: '#0070f2',
            backgroundColor: 'transparent',
            pointRadius: 3,
            borderWidth: 2,
            yAxisID: 'y2',
            order: 1,
          },
        ],
      },
      options: {
        responsive: true,
        maintainAspectRatio: false,
        plugins: {
          legend: { display: false },
          tooltip: {
            callbacks: {
              afterBody: (items) => {
                const idx = items[0]?.dataIndex;
                if (idx !== undefined && data.pareto[idx]) {
                  return [`누적: ${data.pareto[idx].cum_pct}%`];
                }
                return [];
              },
            },
          },
        },
        scales: {
          y: {
            beginAtZero: true,
            position: 'left',
            title: { display: true, text: '건수' },
          },
          y2: {
            beginAtZero: true,
            max: 100,
            position: 'right',
            grid: { drawOnChartArea: false },
            title: { display: true, text: '%' },
            ticks: { callback: v => v + '%' },
          },
          x: { ticks: { maxRotation: 40, font: { size: 11 } } },
        },
      },
    });
  }

  // ── 시간대 히트맵 ────────────────────────────────────────────────
  function renderHourlyHeatmap(data) {
    const container = document.getElementById('aiQsHourlyHeatmap');
    if (!container) return;

    const peakText = document.getElementById('aiQsPeakHourText');

    if (!data.hourly_heatmap?.length) {
      container.innerHTML =
        '<div class="ai-empty-state"><span>시간대 데이터 없음</span></div>';
      return;
    }

    if (peakText && data.peak_hour !== null) {
      peakText.textContent =
        `피크: ${String(data.peak_hour).padStart(2, '0')}:00 — 이 시간에 불량이 집중됩니다`;
    }

    const hourMap  = {};
    data.hourly_heatmap.forEach(h => { hourMap[h.hour] = h; });
    const maxCount = Math.max(...data.hourly_heatmap.map(h => h.count), 1);

    let html = '<div class="ai-heatmap-grid">';
    for (let h = 0; h < 24; h++) {
      const entry = hourMap[h];
      const count = entry ? entry.count : 0;
      const risk  = entry ? entry.risk  : 'none';
      const label = String(h).padStart(2, '0') + ':00';
      const pct   = count / maxCount;

      let bg = '#f0f2f5';
      if (risk === 'high')   bg = `rgba(218,30,40,${0.35 + pct * 0.55})`;
      else if (risk === 'medium') bg = `rgba(230,126,34,${0.25 + pct * 0.5})`;
      else if (risk === 'low' && count > 0) bg = `rgba(48,145,76,${0.2 + pct * 0.4})`;

      const title = entry
        ? `${label}: ${count}건, ${entry.machines_affected}대`
        : `${label}: 0건`;

      html += `
        <div class="ai-heatmap-cell" title="${title}" style="background:${bg};">
          <span class="ai-heatmap-label">${label}</span>
          <span class="ai-heatmap-count">${count > 0 ? count : ''}</span>
        </div>`;
    }
    html += '</div>';
    container.innerHTML = html;
  }

  // ── 기계별 위험 순위 ─────────────────────────────────────────────
  function renderMachineRanking(data) {
    const container = document.getElementById('aiQsMachineRanking');
    if (!container) return;

    if (!data.machine_ranking?.length) {
      container.innerHTML =
        '<div class="ai-empty-state"><span>기계 데이터 없음</span></div>';
      return;
    }

    const riskColors = {
      danger:  'var(--sap-negative)',
      warning: 'var(--sap-caution)',
      normal:  'var(--sap-positive)',
    };
    const riskLabels = { danger: 'DANGER', warning: 'CAUTION', normal: 'NORMAL' };

    let html = '';
    data.machine_ranking.forEach(m => {
      const color  = riskColors[m.risk_level] || '#888';
      const label  = riskLabels[m.risk_level]  || m.risk_level;
      const incTxt = m.increase_rate > 0
        ? `▲ +${m.increase_rate}% vs 이전 7일`
        : (m.increase_rate < 0 ? `▼ ${m.increase_rate}%` : '변화 없음');

      html += `
        <div class="ai-maintenance-item">
          <div class="ai-maintenance-item__header">
            <span style="font-weight:600; font-size:var(--sap-font-size-sm);">${m.machine_no}</span>
            <span class="ai-status-badge" style="background:${color}; color:#fff;">${label}</span>
          </div>
          <div class="ai-risk-bar-wrap">
            <div class="ai-risk-bar" style="width:${m.risk_score}%; background:${color};"></div>
          </div>
          <div style="font-size:0.72rem; color:var(--sap-text-secondary); margin-top:4px;">
            총 ${m.total_count}건 | 7일: ${m.recent_7d}건 | ${incTxt}
          </div>
          ${m.defective_types
            ? `<div style="font-size:0.70rem; color:var(--sap-text-secondary); margin-top:2px;">${m.defective_types}</div>`
            : ''}
        </div>`;
    });

    container.innerHTML = html;
  }

  // ── OEE 상관관계 ─────────────────────────────────────────────────
  function renderCorrelation(data) {
    const container = document.getElementById('aiQsCorrelation');
    if (!container) return;

    const corr = data.oee_correlation;
    if (!corr || corr.coefficient === null) {
      container.innerHTML = `
        <div class="ai-empty-state">
          <span>상관관계 분석을 위한<br>데이터가 부족합니다 (최소 5일 필요)</span>
        </div>`;
      return;
    }

    const coeff    = corr.coefficient;
    const strength = Math.abs(coeff);
    const barWidth = Math.round(strength * 100);
    const color    = coeff <= -0.3
      ? 'var(--sap-negative)'
      : (coeff >= 0.3 ? 'var(--sap-caution)' : 'var(--sap-positive)');

    const insightMap = {
      strong_negative: '불량 증가가 OEE 하락과 강하게 연관됩니다.<br>불량 관리가 OEE 개선의 핵심 레버입니다.',
      weak_negative:   '불량 증가 시 OEE가 소폭 하락하는 경향이 있습니다.',
      strong_positive: '불량과 OEE가 양의 상관관계 — 추가 분석이 필요합니다.',
      weak_positive:   '불량과 OEE 간 약한 양의 상관관계가 관찰됩니다.',
      no_correlation:  '불량과 OEE 사이에 뚜렷한 선형 관계가 없습니다.',
    };
    const insight = insightMap[corr.insight] || '-';

    container.innerHTML = `
      <div style="text-align:center; margin-bottom:var(--sap-spacing-md);">
        <div style="font-size:2.2rem; font-weight:700; color:${color};">
          ${coeff.toFixed(3)}
        </div>
        <div style="font-size:0.78rem; color:var(--sap-text-secondary);">
          Pearson 상관계수
        </div>
      </div>
      <div class="ai-risk-bar-wrap" style="margin-bottom:var(--sap-spacing-sm);">
        <div class="ai-risk-bar" style="width:${barWidth}%; background:${color};"></div>
      </div>
      <div style="font-size:0.8rem; font-weight:600; color:var(--sap-text-primary); margin-bottom:var(--sap-spacing-sm);">
        ${getCorrelationLabel(corr.insight)}
      </div>
      <div style="font-size:0.75rem; color:var(--sap-text-secondary); line-height:1.6;">
        ${insight}
      </div>
      <div style="margin-top:var(--sap-spacing-sm); font-size:0.7rem; color:var(--sap-text-secondary);">
        분석 데이터: ${corr.data_points}일
      </div>`;
  }

  // ── 자동 갱신 예약 ────────────────────────────────────────────────
  function scheduleRefresh() {
    if (refreshTimer) clearInterval(refreshTimer);
    refreshTimer = setInterval(loadAIQualitySentinel, REFRESH_MS);
  }

  // ── 초기화 ────────────────────────────────────────────────────────
  document.addEventListener('DOMContentLoaded', () => {
    // 필터 변경 시 재조회
    ['factoryFilterSelect', 'factoryLineFilterSelect', 'factoryLineMachineFilterSelect']
      .forEach(id => {
        const el = document.getElementById(id);
        if (el) el.addEventListener('change', () => setTimeout(loadAIQualitySentinel, 400));
      });

    // 전용 갱신 버튼
    const btn = document.getElementById('aiQsRefreshBtn');
    if (btn) btn.addEventListener('click', loadAIQualitySentinel);

    // 전역 Refresh 버튼에 훅
    const globalRefresh = document.getElementById('refreshBtn');
    if (globalRefresh) globalRefresh.addEventListener('click', loadAIQualitySentinel);

    // 초기 로드 (메인 SSE 연결 완료 후)
    setTimeout(loadAIQualitySentinel, 1800);
    scheduleRefresh();
  });

})();
