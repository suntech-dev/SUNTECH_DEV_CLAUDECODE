/**
 * AI OEE Overlay — OEE Trend 차트에 AI 예측선 + 신뢰구간 오버레이
 * data_oee.js의 updateOeeTrendChart 함수를 패치하여 예측 데이터 추가
 */

(function () {
  'use strict';

  function getFilters() {
    return {
      factory_filter: document.getElementById('factoryFilterSelect')?.value  || '',
      line_filter:    document.getElementById('factoryLineFilterSelect')?.value || '',
      machine_filter: document.getElementById('factoryLineMachineFilterSelect')?.value || '',
    };
  }

  function buildQS(obj) {
    return Object.entries(obj)
      .filter(([, v]) => v !== '')
      .map(([k, v]) => `${encodeURIComponent(k)}=${encodeURIComponent(v)}`)
      .join('&');
  }

  // ── 예측 오버레이 적용 ────────────────────────────────────────────
  async function applyPredictionOverlay() {
    if (typeof charts === 'undefined' || !charts.oeeTrend) return;

    const qs = buildQS(getFilters());
    try {
      const res  = await fetch(`proc/ai_oee_prediction_2.php?${qs}`);
      const data = await res.json();

      if (data.code !== '00' || !data.forecast?.length) return;

      const chart = charts.oeeTrend;

      // 기존 AI 오버레이 데이터셋 제거
      chart.data.datasets = chart.data.datasets.filter(d => !d._aiOverlay);

      const existingLabels = [...chart.data.labels];
      const forecastLabels = data.forecast.map(f => f.label);

      // 기존 라벨과 겹치지 않는 예측 라벨만 추가
      const newLabels = forecastLabels.filter(l => !existingLabels.includes(l));
      const allLabels = [...existingLabels, ...newLabels];
      chart.data.labels = allLabels;

      // 기존 데이터셋을 새 길이에 맞게 null로 패딩
      chart.data.datasets.forEach(ds => {
        while (ds.data.length < allLabels.length) ds.data.push(null);
      });

      // 예측 값 배열 구성
      const makeSeries = (key) => allLabels.map(label => {
        const f = data.forecast.find(x => x.label === label);
        return f ? f[key] : null;
      });

      const forecastData = makeSeries('oee');
      const upperData    = makeSeries('upper');
      const lowerData    = makeSeries('lower');

      // CI 상단 (fill → CI 하단)
      chart.data.datasets.push({
        label:           'CI Upper',
        data:            upperData,
        borderColor:     'transparent',
        backgroundColor: 'rgba(245,166,35,0.15)',
        fill:            '+1',
        pointRadius:     0,
        tension:         0.4,
        _aiOverlay:      true,
      });

      // CI 하단
      chart.data.datasets.push({
        label:           'CI Lower',
        data:            lowerData,
        borderColor:     'transparent',
        backgroundColor: 'transparent',
        fill:            false,
        pointRadius:     0,
        tension:         0.4,
        _aiOverlay:      true,
      });

      // 예측선 (점선 주황)
      chart.data.datasets.push({
        label:                'AI Forecast',
        data:                 forecastData,
        borderColor:          '#f5a623',
        backgroundColor:      'transparent',
        borderDash:           [6, 4],
        borderWidth:          2,
        pointRadius:          4,
        pointBackgroundColor: '#f5a623',
        tension:              0.4,
        fill:                 false,
        _aiOverlay:           true,
      });

      chart.update('none');

      // 트렌드 뱃지 업데이트
      updateTrendBadge(data);

    } catch (e) {
      console.warn('[AI OEE Overlay]', e);
    }
  }

  // ── 트렌드 방향 뱃지 ──────────────────────────────────────────────
  function updateTrendBadge(data) {
    let badge = document.getElementById('aiOeeTrendBadge');
    if (!badge) return;

    const trendMap = {
      up:     { cls: 'ai-trend-badge--up',     text: '↑ Trending Up' },
      down:   { cls: 'ai-trend-badge--down',   text: '↓ Trending Down' },
      stable: { cls: 'ai-trend-badge--stable', text: '→ Stable' },
    };
    const t = trendMap[data.trend] || trendMap.stable;
    badge.className = 'ai-trend-badge ' + t.cls;
    badge.textContent = t.text;
  }

  // ── updateOeeTrendChart 함수 패치 ────────────────────────────────
  document.addEventListener('DOMContentLoaded', () => {
    const wait = setInterval(() => {
      if (typeof updateOeeTrendChart === 'function') {
        clearInterval(wait);

        const original = window.updateOeeTrendChart;
        window.updateOeeTrendChart = function (trendStats) {
          original.call(this, trendStats);
          setTimeout(applyPredictionOverlay, 400);
        };
      }
    }, 100);

    // Refresh 버튼에도 훅
    const btn = document.getElementById('refreshBtn');
    if (btn) btn.addEventListener('click', () => setTimeout(applyPredictionOverlay, 600));

    // 필터 변경 시 재조회
    ['factoryFilterSelect', 'factoryLineFilterSelect', 'factoryLineMachineFilterSelect']
      .forEach(id => {
        const el = document.getElementById(id);
        if (el) el.addEventListener('change', () => setTimeout(applyPredictionOverlay, 600));
      });
  });

})();
