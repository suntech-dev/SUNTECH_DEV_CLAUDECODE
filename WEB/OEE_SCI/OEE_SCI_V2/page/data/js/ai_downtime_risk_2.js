/**
 * AI Downtime Risk — 다운타임 테이블에 기계별 AI 위험도 열 추가
 * MutationObserver로 tbody 변화 감지 → ai_maintenance.php 위험도 매핑
 */

(function () {
  'use strict';

  let riskMap = {}; // { machine_no: { risk_level, risk_score } }

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

  // ── 위험도 데이터 로드 ────────────────────────────────────────────
  async function loadRiskData() {
    const qs = buildQS(getFilters());
    try {
      const res  = await fetch(`proc/ai_maintenance_2.php?${qs}`);
      const data = await res.json();

      riskMap = {};
      if (data.code === '00' && data.machines?.length) {
        data.machines.forEach(m => {
          riskMap[m.machine_no] = {
            risk_level: m.risk_level,
            risk_score: m.risk_score,
          };
        });
      }
      applyRiskToAllRows();
    } catch (e) {
      console.warn('[AI Downtime Risk]', e);
    }
  }

  // ── 모든 행에 위험도 적용 ─────────────────────────────────────────
  function applyRiskToAllRows() {
    const tbody = document.getElementById('downtimeDataBody');
    if (!tbody) return;
    tbody.querySelectorAll('tr').forEach(addRiskCell);
  }

  // ── 단일 행에 위험도 셀 삽입 ─────────────────────────────────────
  function addRiskCell(row) {
    if (row.dataset.aiRiskDone === '1') return;

    // 로딩/빈 행 (colspan 있는 단일 셀 행) 건너뜀
    if (row.cells.length < 10) return;

    const machineNo = row.cells[0]?.textContent?.trim() || '';
    const risk      = riskMap[machineNo];

    const td = document.createElement('td');
    td.className    = 'fiori-table__cell';
    td.style.cssText = 'text-align:center; white-space:nowrap;';

    if (risk) {
      const colors = { danger: '#da1e28', warning: '#e67e22', normal: '#30914c' };
      const labels = { danger: 'DANGER', warning: 'CAUTION', normal: 'NORMAL' };
      const color  = colors[risk.risk_level] || '#888';
      const label  = labels[risk.risk_level] || '-';
      td.innerHTML = `
        <span class="ai-status-badge" style="background:${color}; color:#fff;">${label}</span>
        <div style="font-size:0.65rem; color:var(--sap-text-secondary); margin-top:2px;">${risk.risk_score}%</div>`;
    } else {
      td.innerHTML = '<span style="color:var(--sap-text-secondary); font-size:0.75rem;">-</span>';
    }

    // DETAIL 열 (마지막) 바로 앞에 삽입
    row.insertBefore(td, row.cells[row.cells.length - 1]);
    row.dataset.aiRiskDone = '1';
  }

  // ── MutationObserver로 tbody 변화 감지 ───────────────────────────
  function observeTableBody() {
    const tbody = document.getElementById('downtimeDataBody');
    if (!tbody) return;

    new MutationObserver(mutations => {
      mutations.forEach(m => {
        m.addedNodes.forEach(node => {
          if (node.nodeName === 'TR') addRiskCell(node);
        });
      });
    }).observe(tbody, { childList: true });
  }

  // ── 초기화 ────────────────────────────────────────────────────────
  document.addEventListener('DOMContentLoaded', () => {
    observeTableBody();
    setTimeout(loadRiskData, 1200);

    // 필터 변경 시 재조회 (기존 행 risk-done 플래그 리셋 후 재적용)
    ['factoryFilterSelect', 'factoryLineFilterSelect', 'factoryLineMachineFilterSelect']
      .forEach(id => {
        const el = document.getElementById(id);
        if (el) el.addEventListener('change', () => {
          // 플래그 초기화 후 재로드
          document.querySelectorAll('#downtimeDataBody tr[data-ai-risk-done]')
            .forEach(r => delete r.dataset.aiRiskDone);
          setTimeout(loadRiskData, 500);
        });
      });

    // Refresh 버튼
    const btn = document.getElementById('refreshBtn');
    if (btn) btn.addEventListener('click', () => {
      document.querySelectorAll('#downtimeDataBody tr[data-ai-risk-done]')
        .forEach(r => delete r.dataset.aiRiskDone);
      setTimeout(loadRiskData, 600);
    });

    // 2분마다 자동 갱신
    setInterval(loadRiskData, 120000);
  });

})();
