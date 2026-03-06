/**
 * F13 — AI Production Optimization
 * Renders line-level OEE bottleneck analysis results as prioritized cards
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

  // ── Load optimization data ────────────────────────────────────
  async function loadOptimization() {
    const container = document.getElementById('aiOptList');
    if (container) {
      container.innerHTML = '<div class="ai-empty-state"><span class="ai-spinner"></span> Analyzing optimization opportunities...</div>';
    }

    const qs = buildQS(getFilters());
    try {
      const res  = await fetch(`proc/ai_optimization.php${qs ? '?' + qs : ''}`);
      const data = await res.json();

      if (data.code !== '00') return;
      renderSummary(data.summary);
      renderOpportunities(data.opportunities);
    } catch (e) {
      console.warn('[AI Optimization]', e);
      if (container) container.innerHTML = '<div class="ai-empty-state">Failed to load data</div>';
    }
  }

  // ── Render summary ────────────────────────────────────────────
  function renderSummary(s) {
    const el = document.getElementById('aiOptSummary');
    if (!el || !s) return;

    if (s.msg) { el.textContent = s.msg; return; }

    el.innerHTML = `
      <span>Total Lines: <strong>${s.total_lines}</strong></span>
      <span>Below Target:
        <strong style="color:var(--sap-negative);">${s.lines_below_target}</strong>
      </span>
      <span>Avg OEE: <strong>${s.global_avg_oee}%</strong></span>
      <span>Best OEE:
        <strong style="color:var(--sap-positive);">${s.best_oee}%</strong>
      </span>
      <span>Target OEE: <strong>${s.target_oee}%</strong></span>
    `;
  }

  // ── Render opportunity cards ──────────────────────────────────
  function renderOpportunities(opps) {
    const container = document.getElementById('aiOptList');
    if (!container) return;

    if (!opps?.length) {
      container.innerHTML = '<div class="ai-empty-state">✅ All lines are meeting the OEE target (85%).</div>';
      return;
    }

    const PCOLORS = { P1: '#da1e28', P2: '#e67e22', P3: '#0070f2' };
    const TRENDS  = { improving: { icon: '↑', color: 'var(--sap-positive)' },
                      declining: { icon: '↓', color: 'var(--sap-negative)' },
                      stable:    { icon: '→', color: 'var(--sap-text-secondary)' } };
    const BICONS  = { availability: '⚙️', performance: '🏃', quality: '🔬' };

    container.innerHTML = opps.map((op, i) => {
      const pc     = PCOLORS[op.priority] || '#888';
      const tr     = TRENDS[op.trend]     || TRENDS.stable;
      const bicon  = BICONS[op.bottleneck] || '⚠️';
      const barCur = Math.min(100, op.current_oee);
      const barPot = Math.min(100, op.potential_oee);
      const barTgt = Math.min(100, op.target_oee);
      const suggs  = op.suggestions.map(s => `<li>${s}</li>`).join('');

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
                <strong style="color:${pc};">-${op.oee_gap}%p</strong> below target ${op.target_oee}%
              </span>
            </div>
          </div>

          <!-- OEE bars (Current / Potential / Target) -->
          <div class="ai-opt-bar-wrap">
            <div class="ai-opt-bar-row">
              <span class="ai-opt-bar-label">Current</span>
              <div class="ai-opt-bar-track">
                <div class="ai-opt-bar-fill" style="width:${barCur}%; background:${pc};"></div>
                <div class="ai-opt-bar-target" style="left:${barTgt}%;"></div>
              </div>
              <span class="ai-opt-bar-val">${op.current_oee}%</span>
            </div>
            <div class="ai-opt-bar-row">
              <span class="ai-opt-bar-label">Potential</span>
              <div class="ai-opt-bar-track">
                <div class="ai-opt-bar-fill ai-opt-bar-fill--potential"
                     style="width:${barPot}%;"></div>
                <div class="ai-opt-bar-target" style="left:${barTgt}%;"></div>
              </div>
              <span class="ai-opt-bar-val" style="color:var(--sap-informative);">
                ${op.potential_oee}%
                <small>(+${op.potential_gain}%)</small>
              </span>
            </div>
          </div>

          <!-- Bottleneck component -->
          <div class="ai-opt-bottleneck">
            ${bicon} <strong>Bottleneck:</strong>
            ${op.bottleneck_label} ${op.bottleneck_current}%
            → target ${op.bottleneck_target}%
            (gap <strong style="color:${pc};">${op.bottleneck_gap}%p</strong>)
          </div>

          <!-- OEE component values -->
          <div class="ai-opt-components">
            <span class="${op.bottleneck === 'availability' ? 'ai-opt-comp--highlight' : ''}">
              ⚙️ Avail. ${op.avg_avail}%
            </span>
            <span class="${op.bottleneck === 'performance'  ? 'ai-opt-comp--highlight' : ''}">
              🏃 Perf. ${op.avg_perf}%
            </span>
            <span class="${op.bottleneck === 'quality'      ? 'ai-opt-comp--highlight' : ''}">
              🔬 Quality ${op.avg_quality}%
            </span>
            <span style="color:var(--sap-text-secondary); font-size:0.75rem;">
              🏆 vs. best line -${op.vs_best}%p
            </span>
          </div>

          <!-- Improvement suggestions (expandable) -->
          <details class="ai-opt-suggestions">
            <summary>💡 Improvement Suggestions (${op.suggestions.length})</summary>
            <ul>${suggs}</ul>
          </details>
        </div>
      `;
    }).join('');
  }

  // ── Init ─────────────────────────────────────────────────────
  document.addEventListener('DOMContentLoaded', () => {
    setTimeout(loadOptimization, 2200);

    ['factoryFilterSelect', 'factoryLineFilterSelect', 'factoryLineMachineFilterSelect']
      .forEach(id => {
        const el = document.getElementById(id);
        if (el) el.addEventListener('change', () => setTimeout(loadOptimization, 500));
      });

    const btn = document.getElementById('aiRefreshBtn');
    if (btn) btn.addEventListener('click', loadOptimization);
  });

})();
