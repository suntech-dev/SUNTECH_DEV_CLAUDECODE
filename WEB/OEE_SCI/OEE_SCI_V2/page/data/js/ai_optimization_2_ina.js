/**
 * ============================================================
 * 파일명: ai_optimization_2_ina.js
 * 목적: AI Production Optimization 분석 (인도네시아어 버전)
 *       ai_optimization_2.js 기반, UI 텍스트를 인도네시아어로 교체
 * ============================================================
 */

(function () {
    'use strict';

    function getFilters() {
        return {
            factory_filter: document.getElementById('factoryFilterSelect')?.value || '',
            line_filter: document.getElementById('factoryLineFilterSelect')?.value || '',
            machine_filter: document.getElementById('factoryLineMachineFilterSelect')?.value || '',
            date_range: document.getElementById('dateRangeSelect')?.value || 'today',
        };
    }

    function buildQS(obj) {
        return Object.entries(obj)
            .filter(([, v]) => v !== '')
            .map(([k, v]) => `${encodeURIComponent(k)}=${encodeURIComponent(v)}`)
            .join('&');
    }

    async function loadOptimization() {
        const container = document.getElementById('aiOptList');
        if (container) {
            container.innerHTML = '<div class="ai-empty-state"><span class="ai-spinner"></span> Menganalisis peluang optimasi...</div>';
        }

        const qs = buildQS(getFilters());
        try {
            const res = await fetch(`proc/ai_optimization_2.php${qs ? '?' + qs : ''}`);
            const data = await res.json();

            if (data.code !== '00') return;
            renderSummary(data.summary);
            renderOpportunities(data.opportunities);
        } catch (e) {
            console.warn('[AI Optimization INA]', e);
            if (container) container.innerHTML = '<div class="ai-empty-state">Gagal memuat data</div>';
        }
    }

    function renderSummary(s) {
        const el = document.getElementById('aiOptSummary');
        if (!el || !s) return;

        if (s.msg) { el.textContent = s.msg; return; }

        const dayLabel = s.analysis_days ? s.analysis_days + 'h' : '14h'; // eslint-disable-line no-unused-vars
        el.innerHTML = `
      <span>Total Line: <strong>${s.total_lines}</strong></span>
      <span>Di Bawah Target:
        <strong style="color:var(--sap-negative);">${s.lines_below_target}</strong>
      </span>
      <span>Rata-rata OEE: <strong>${s.global_avg_oee}%</strong></span>
      <span>OEE Terbaik:
        <strong style="color:var(--sap-positive);">${s.best_oee}%</strong>
      </span>
      <span>Target OEE: <strong>${s.target_oee}%</strong></span>
    `;
    }

    function renderOpportunities(opps) {
        const container = document.getElementById('aiOptList');
        if (!container) return;

        if (!opps?.length) {
            container.innerHTML = '<div class="ai-empty-state">Semua line memenuhi target OEE (85%).</div>';
            return;
        }

        const PCOLORS = { P1: '#da1e28', P2: '#e67e22', P3: '#0070f2' };
        const TRENDS = {
            improving: { icon: '&#8593;', color: 'var(--sap-positive)' },
            declining: { icon: '&#8595;', color: 'var(--sap-negative)' },
            stable: { icon: '&#8594;', color: 'var(--sap-text-secondary)' },
        };
        const BICONS = { availability: '&#9881;', performance: '&#9654;', quality: '&#128300;' };

        container.innerHTML = opps.map((op) => {
            const pc = PCOLORS[op.priority] || '#888';
            const tr = TRENDS[op.trend] || TRENDS.stable;
            const bicon = BICONS[op.bottleneck] || '&#9888;';
            const barCur = Math.min(100, op.current_oee);
            const barPot = Math.min(100, op.potential_oee);
            const barTgt = Math.min(100, op.target_oee);
            const suggs = op.suggestions.map(s => `<li>${s}</li>`).join('');

            const botHighlight = (key) => op.bottleneck === key ? 'ai-opt-comp--highlight' : '';

            return `
        <div class="ai-opt-card" style="border-left:4px solid ${pc};">
          <div class="ai-opt-card__header">
            <div class="ai-opt-card__title">
              <span class="ai-status-badge" style="background:${pc};color:#fff;">${op.priority}</span>
              <strong>${op.factory_name} / ${op.line_name}</strong>
              <span style="color:${tr.color}; font-size:0.9rem;">${tr.icon}</span>
            </div>
            <div class="ai-opt-card__oee">
              <span style="font-size:1.25rem; font-weight:700; color:${pc};">${op.current_oee}%</span>
              <span style="color:var(--sap-text-secondary); font-size:0.78rem;">
                <strong style="color:${pc};">-${op.oee_gap}%p</strong> di bawah target ${op.target_oee}%
              </span>
            </div>
          </div>

          <div class="ai-opt-bar-wrap">
            <div class="ai-opt-bar-row">
              <span class="ai-opt-bar-label">Saat Ini</span>
              <div class="ai-opt-bar-track">
                <div class="ai-opt-bar-fill" style="width:${barCur}%; background:${pc};"></div>
                <div class="ai-opt-bar-target" style="left:${barTgt}%;"></div>
              </div>
              <span class="ai-opt-bar-val">${op.current_oee}%</span>
            </div>
            <div class="ai-opt-bar-row">
              <span class="ai-opt-bar-label">Potensi</span>
              <div class="ai-opt-bar-track">
                <div class="ai-opt-bar-fill ai-opt-bar-fill--potential" style="width:${barPot}%;"></div>
                <div class="ai-opt-bar-target" style="left:${barTgt}%;"></div>
              </div>
              <span class="ai-opt-bar-val" style="color:var(--sap-informative);">
                ${op.potential_oee}% <small>(+${op.potential_gain}%)</small>
              </span>
            </div>
          </div>

          <div class="ai-opt-bottleneck">
            ${bicon} <strong>Hambatan:</strong>
            ${op.bottleneck_label} ${op.bottleneck_current}%
            &rarr; target ${op.bottleneck_target}%
            (selisih <strong style="color:${pc};">${op.bottleneck_gap}%p</strong>)
          </div>

          <div class="ai-opt-components">
            <span class="${botHighlight('availability')}">&#9881; Ketersediaan ${op.avg_avail}%</span>
            <span class="${botHighlight('performance')}">&#9654; Kinerja ${op.avg_perf}%</span>
            <span class="${botHighlight('quality')}">&#128300; Kualitas ${op.avg_quality}%</span>
            <span style="color:var(--sap-text-secondary); font-size:0.75rem;">
              &#127942; vs. line terbaik -${op.vs_best}%p
            </span>
          </div>

          <details class="ai-opt-suggestions">
            <summary>&#128161; Saran Perbaikan (${op.suggestions.length})</summary>
            <ul>${suggs}</ul>
          </details>
        </div>
      `;
        }).join('');
    }

    document.addEventListener('DOMContentLoaded', () => {
        setTimeout(loadOptimization, 2200);

        ['factoryFilterSelect', 'factoryLineFilterSelect', 'factoryLineMachineFilterSelect', 'dateRangeSelect']
            .forEach(id => {
                const el = document.getElementById(id);
                if (el) el.addEventListener('change', () => setTimeout(loadOptimization, 500));
            });

        const btn = document.getElementById('aiRefreshBtn');
        if (btn) btn.addEventListener('click', loadOptimization);
    });

})();
