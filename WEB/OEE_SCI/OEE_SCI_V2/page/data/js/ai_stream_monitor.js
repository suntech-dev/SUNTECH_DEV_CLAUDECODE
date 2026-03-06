/**
 * F12 — AI 실시간 스트리밍 분석 모니터
 * ai_dashboard.php 스트림 피드 패널에 SSE 이벤트 실시간 표시
 */

(function () {
  'use strict';

  const MAX_EVENTS = 15;
  let es            = null;
  let reconnectTimer= null;
  let eventCount    = 0;

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

  // ── 이벤트 타입 레이블 ────────────────────────────────────────
  const TYPE_LABELS = {
    anomaly:          'OEE Anomaly Detected',
    downtime_new:     'Active Downtime',
    maintenance_risk: 'Maintenance Risk Alert',
    quality_alert:    'Quality Alert',
  };

  const TYPE_ICONS = {
    anomaly:          '⚠️',
    downtime_new:     '🔴',
    maintenance_risk: '🔧',
    quality_alert:    '🔬',
    status:           'ℹ️',
  };

  const SEVERITY_COLORS = {
    danger:  '#da1e28',
    warning: '#e67e22',
    info:    '#0070f2',
    normal:  '#30914c',
  };

  // ── 이벤트 카드 렌더링 ────────────────────────────────────────
  function renderEvent(type, data) {
    const feed = document.getElementById('aiStreamFeed');
    if (!feed) return;

    // 빈 상태 제거
    feed.querySelector('.ai-stream-empty')?.remove();

    // 하트비트 — 상태 도트만 갱신
    if (type === 'heartbeat') {
      const dot = document.getElementById('aiStreamDot');
      if (dot) { dot.title = `Last checked: ${data.timestamp}`; dot.style.background = '#30914c'; }
      return;
    }

    // 연결/해제 상태 처리
    if (type === 'connected' || type === 'disconnected') {
      updateStatus(type === 'connected');
      return;
    }

    // 초기 "이상 없음" 안내
    if (type === 'status') {
      const el = document.createElement('div');
      el.className = 'ai-stream-empty';
      el.textContent = '✅ ' + (data.message || '');
      feed.prepend(el);
      return;
    }

    const sev   = data.severity || 'info';
    const color = SEVERITY_COLORS[sev] || SEVERITY_COLORS.info;
    const icon  = TYPE_ICONS[type] || '📡';
    const label = TYPE_LABELS[type] || type;

    const card = document.createElement('div');
    card.className = 'ai-stream-event';
    card.style.cssText = `border-left:3px solid ${color};`;
    card.innerHTML = `
      <div class="ai-stream-event__header">
        <span class="ai-stream-event__icon">${icon}</span>
        <span class="ai-stream-event__type">${label}</span>
        <span class="ai-stream-event__time">${data.timestamp || ''}</span>
        <span class="ai-status-badge" style="background:${color};color:#fff;font-size:0.6rem;padding:1px 5px;">${sev.toUpperCase()}</span>
      </div>
      <div class="ai-stream-event__msg">${data.message || ''}</div>
      ${data.machine_no
        ? `<div class="ai-stream-event__meta">${data.line_name || ''} / ${data.machine_no}</div>`
        : ''}
    `;

    feed.prepend(card);
    eventCount++;

    // 카운터 갱신
    const ctr = document.getElementById('aiStreamCount');
    if (ctr) ctr.textContent = `${eventCount} events`;

    // 최대 개수 초과 시 오래된 항목 제거
    const cards = feed.querySelectorAll('.ai-stream-event');
    if (cards.length > MAX_EVENTS) cards[cards.length - 1].remove();
  }

  // ── 연결 상태 표시 ────────────────────────────────────────────
  function updateStatus(connected) {
    const statusEl = document.getElementById('aiStreamStatus');
    const dotEl    = document.getElementById('aiStreamDot');
    if (statusEl) statusEl.textContent = connected ? 'Connected' : 'Reconnecting...';
    if (dotEl)    dotEl.style.background = connected ? '#30914c' : '#e67e22';
  }

  // ── SSE 연결 ──────────────────────────────────────────────────
  function connect() {
    if (es) { es.close(); es = null; }
    clearTimeout(reconnectTimer);

    const qs  = buildQS(getFilters());
    const url = `proc/ai_stream_analysis.php${qs ? '?' + qs : ''}`;

    es = new EventSource(url);

    const HANDLED_EVENTS = [
      'connected', 'anomaly', 'downtime_new', 'maintenance_risk',
      'quality_alert', 'status', 'heartbeat', 'disconnected',
    ];
    HANDLED_EVENTS.forEach(type => {
      es.addEventListener(type, e => {
        try { renderEvent(type, JSON.parse(e.data)); } catch (_) {}
      });
    });

    es.addEventListener('disconnected', () => {
      es.close(); es = null;
      updateStatus(false);
      reconnectTimer = setTimeout(connect, 5000);
    });

    es.onerror = () => {
      updateStatus(false);
      es.close(); es = null;
      reconnectTimer = setTimeout(connect, 15000);
    };
  }

  // ── 피드 초기화 후 재연결 ─────────────────────────────────────
  function reconnect() {
    eventCount = 0;
    const feed = document.getElementById('aiStreamFeed');
    if (feed) feed.innerHTML = '<div class="ai-stream-empty"><span class="ai-spinner"></span> Connecting...</div>';
    connect();
  }

  // ── 초기화 ───────────────────────────────────────────────────
  document.addEventListener('DOMContentLoaded', () => {
    connect();

    ['factoryFilterSelect', 'factoryLineFilterSelect', 'factoryLineMachineFilterSelect']
      .forEach(id => {
        const el = document.getElementById(id);
        if (el) el.addEventListener('change', reconnect);
      });

    const btn = document.getElementById('aiRefreshBtn');
    if (btn) btn.addEventListener('click', reconnect);
  });

})();
