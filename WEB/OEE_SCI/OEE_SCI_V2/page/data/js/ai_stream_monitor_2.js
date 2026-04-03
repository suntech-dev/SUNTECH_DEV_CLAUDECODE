/**
 * ============================================================
 * 파일명: ai_stream_monitor_2.js
 * 목적: F12 — AI 실시간 스트리밍 분석 모니터
 *       - ai_dashboard.php 스트림 피드 패널에 SSE 이벤트 실시간 표시
 *       - proc/ai_stream_analysis_2.php에 EventSource(SSE) 연결
 *       - anomaly / downtime_new / maintenance_risk / quality_alert /
 *         status / heartbeat / connected / disconnected 이벤트 처리
 *       - 최대 MAX_EVENTS(15)개 이벤트 카드 유지 (오래된 항목 자동 제거)
 *       - 연결 끊김 시 자동 재연결 (disconnected: 5초, onerror: 15초)
 *       - 공장/라인/기계 필터 변경 또는 Refresh 버튼 클릭 시 피드 초기화 후 재연결
 * ============================================================
 */

/* IIFE(즉시 실행 함수)로 전역 스코프 오염 방지 */
(function () {
    'use strict';

    /* 피드에 표시할 최대 이벤트 카드 수 */
    const MAX_EVENTS = 15;
    /* 현재 활성 EventSource 인스턴스 (재연결 시 기존 연결 닫기 위해 보관) */
    let es = null;
    /* 재연결 딜레이 타이머 핸들 */
    let reconnectTimer = null;
    /* 누적 이벤트 수 카운터 */
    let eventCount = 0;

    /**
     * 현재 선택된 공장/라인/기계 필터값을 객체로 반환
     * @returns {Object} 필터 파라미터 객체
     */
    function getFilters() {
        return {
            factory_filter: document.getElementById('factoryFilterSelect')?.value || '',
            line_filter: document.getElementById('factoryLineFilterSelect')?.value || '',
            machine_filter: document.getElementById('factoryLineMachineFilterSelect')?.value || '',
        };
    }

    /**
     * 객체를 URL 쿼리스트링으로 변환
     * - 빈 문자열 값을 가진 키는 제외
     * @param {Object} obj - 변환할 파라미터 객체
     * @returns {string} URL 쿼리스트링
     */
    function buildQS(obj) {
        return Object.entries(obj)
            .filter(([, v]) => v !== '')  // 빈 값 제외
            .map(([k, v]) => `${encodeURIComponent(k)}=${encodeURIComponent(v)}`)
            .join('&');
    }

    // ── 이벤트 타입 레이블 ────────────────────────────────────────
    /* SSE 이벤트 타입(type)에 대응하는 사람이 읽기 쉬운 레이블 매핑 */
    const TYPE_LABELS = {
        anomaly: 'OEE Anomaly Detected',
        downtime_new: 'Active Downtime',
        maintenance_risk: 'Maintenance Risk Alert',
        quality_alert: 'Quality Alert',
    };

    /* SSE 이벤트 타입별 표시 아이콘 (이모지) */
    const TYPE_ICONS = {
        anomaly: '⚠️',
        downtime_new: '🔴',
        maintenance_risk: '🔧',
        quality_alert: '🔬',
        status: 'ℹ️',
    };

    /* 심각도(severity)별 배지 배경색 매핑 */
    const SEVERITY_COLORS = {
        danger: '#da1e28',   // 빨강: 위험
        warning: '#e67e22',  // 주황: 경고
        info: '#0070f2',     // 파랑: 정보
        normal: '#30914c',   // 초록: 정상
    };

    // ── 이벤트 카드 렌더링 ────────────────────────────────────────
    /**
     * SSE 이벤트 데이터를 aiStreamFeed 컨테이너에 카드 형태로 렌더링
     * - heartbeat: 상태 도트 색상만 갱신 (카드 추가 없음)
     * - connected/disconnected: 연결 상태 표시 업데이트
     * - status: "이상 없음" 안내 메시지 표시
     * - 기타 이벤트: 이벤트 카드를 feed 최상단에 prepend
     * - MAX_EVENTS 초과 시 가장 오래된 카드(마지막 항목) 제거
     * @param {string} type - SSE 이벤트 타입명
     * @param {Object} data - 이벤트 JSON 데이터
     */
    function renderEvent(type, data) {
        /* 스트림 피드 컨테이너 참조 */
        const feed = document.getElementById('aiStreamFeed');
        if (!feed) return;

        // 빈 상태 제거
        /* 기존 "연결 중..." 또는 빈 상태 메시지 DOM 요소 제거 */
        feed.querySelector('.ai-stream-empty')?.remove();

        // 하트비트 — 상태 도트만 갱신
        /* heartbeat 이벤트: 연결 살아있음을 확인, 도트 색상을 초록으로 갱신 */
        if (type === 'heartbeat') {
            const dot = document.getElementById('aiStreamDot');
            if (dot) { dot.title = `Last checked: ${data.timestamp}`; dot.style.background = '#30914c'; }
            return;
        }

        // 연결/해제 상태 처리
        /* connected/disconnected 이벤트: 연결 상태 표시 업데이트 */
        if (type === 'connected' || type === 'disconnected') {
            updateStatus(type === 'connected');
            return;
        }

        // 초기 "이상 없음" 안내
        /* status 이벤트: "이상 없음" 안내 메시지를 피드 상단에 표시 */
        if (type === 'status') {
            const el = document.createElement('div');
            el.className = 'ai-stream-empty';
            el.textContent = '✅ ' + (data.message || '');
            feed.prepend(el);
            return;
        }

        /* 심각도 결정 (없으면 'info' 기본값) */
        const sev = data.severity || 'info';
        /* 심각도에 따른 배지 색상 (알 수 없으면 info 색상) */
        const color = SEVERITY_COLORS[sev] || SEVERITY_COLORS.info;
        /* 이벤트 타입 아이콘 (알 수 없으면 안테나 이모지) */
        const icon = TYPE_ICONS[type] || '📡';
        /* 이벤트 타입 레이블 (알 수 없으면 타입명 그대로 표시) */
        const label = TYPE_LABELS[type] || type;

        /* 이벤트 카드 DOM 요소 생성 */
        const card = document.createElement('div');
        card.className = 'ai-stream-event';
        /* 카드 좌측 테두리에 심각도 색상 적용 */
        card.style.cssText = `border-left:3px solid ${color};`;
        /* 카드 내부 HTML: 타입 아이콘, 레이블, 시간, 심각도 배지, 메시지, 기계 정보 */
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

        /* 새 카드를 피드 최상단에 추가 (최신 이벤트가 위에 표시) */
        feed.prepend(card);
        eventCount++;

        // 카운터 갱신
        /* 이벤트 카운터 표시 업데이트 */
        const ctr = document.getElementById('aiStreamCount');
        if (ctr) ctr.textContent = `${eventCount} events`;

        // 최대 개수 초과 시 오래된 항목 제거
        /* 피드의 카드 수가 MAX_EVENTS(15)를 초과하면 가장 오래된 카드(마지막) 제거 */
        const cards = feed.querySelectorAll('.ai-stream-event');
        if (cards.length > MAX_EVENTS) cards[cards.length - 1].remove();
    }

    // ── 연결 상태 표시 ────────────────────────────────────────────
    /**
     * SSE 연결 상태에 따라 상태 텍스트 및 상태 도트 색상을 업데이트
     * - connected: 텍스트 'Connected', 도트 초록색
     * - disconnected: 텍스트 'Reconnecting...', 도트 주황색
     * @param {boolean} connected - true이면 연결됨, false이면 재연결 중
     */
    function updateStatus(connected) {
        /* 연결 상태 텍스트 요소 */
        const statusEl = document.getElementById('aiStreamStatus');
        /* 연결 상태 도트(원형) 요소 */
        const dotEl = document.getElementById('aiStreamDot');
        if (statusEl) statusEl.textContent = connected ? 'Connected' : 'Reconnecting...';
        /* 연결됨: 초록(#30914c), 재연결 중: 주황(#e67e22) */
        if (dotEl) dotEl.style.background = connected ? '#30914c' : '#e67e22';
    }

    // ── SSE 연결 ──────────────────────────────────────────────────
    /**
     * proc/ai_stream_analysis_2.php에 EventSource(SSE) 연결 수립
     * - 기존 연결이 있으면 먼저 닫고 재연결 타이머도 초기화
     * - 처리할 이벤트 타입 목록을 순회하며 각각 addEventListener 등록
     * - disconnected 이벤트: 연결 닫고 5초 후 자동 재연결
     * - onerror 이벤트: 상태 갱신 후 15초 후 자동 재연결
     */
    function connect() {
        /* 기존 EventSource 연결이 있으면 닫기 */
        if (es) { es.close(); es = null; }
        /* 기존 재연결 타이머 취소 */
        clearTimeout(reconnectTimer);

        /* 현재 필터를 쿼리스트링으로 변환하여 SSE URL 구성 */
        const qs = buildQS(getFilters());
        const url = `proc/ai_stream_analysis_2.php${qs ? '?' + qs : ''}`;

        /* EventSource(SSE) 연결 생성 */
        es = new EventSource(url);

        /* 처리할 SSE 이벤트 타입 목록 */
        const HANDLED_EVENTS = [
            'connected', 'anomaly', 'downtime_new', 'maintenance_risk',
            'quality_alert', 'status', 'heartbeat', 'disconnected',
        ];
        /* 각 이벤트 타입에 리스너 등록: JSON 파싱 후 renderEvent 호출 */
        HANDLED_EVENTS.forEach(type => {
            es.addEventListener(type, e => {
                /* JSON 파싱 실패 시 무시 (try-catch로 안전 처리) */
                try { renderEvent(type, JSON.parse(e.data)); } catch (_) { }
            });
        });

        /* disconnected 이벤트: 서버 측에서 연결 종료 신호 → 5초 후 재연결 */
        es.addEventListener('disconnected', () => {
            es.close(); es = null;
            updateStatus(false);  // 상태를 'Reconnecting...'으로 변경
            reconnectTimer = setTimeout(connect, 5000);  // 5초 후 재연결
        });

        /* onerror: 네트워크 오류 또는 서버 응답 없음 → 15초 후 재연결 */
        es.onerror = () => {
            updateStatus(false);  // 상태를 'Reconnecting...'으로 변경
            es.close(); es = null;
            reconnectTimer = setTimeout(connect, 15000);  // 15초 후 재연결
        };
    }

    // ── 피드 초기화 후 재연결 ─────────────────────────────────────
    /**
     * 이벤트 카운터와 피드 DOM을 초기화한 후 SSE 재연결
     * - 필터 변경 또는 Refresh 버튼 클릭 시 호출
     * - 피드를 '연결 중...' 스피너로 리셋
     */
    function reconnect() {
        /* 이벤트 카운터 초기화 */
        eventCount = 0;
        /* 피드 컨테이너를 '연결 중...' 스피너 메시지로 리셋 */
        const feed = document.getElementById('aiStreamFeed');
        if (feed) feed.innerHTML = '<div class="ai-stream-empty"><span class="ai-spinner"></span> Connecting...</div>';
        /* SSE 재연결 */
        connect();
    }

    // ── 초기화 ───────────────────────────────────────────────────
    /**
     * DOMContentLoaded 이벤트 시 실행되는 초기화 블록
     * 1. 최초 SSE 연결 시작
     * 2. 공장/라인/기계 필터 변경 시 피드 초기화 후 재연결
     * 3. AI Refresh 버튼 클릭 시 피드 초기화 후 재연결
     */
    document.addEventListener('DOMContentLoaded', () => {
        /* 페이지 로드 시 SSE 연결 최초 시작 */
        connect();

        /* 각 필터 select 요소에 change 이벤트 리스너 등록: 필터 변경 시 재연결 */
        ['factoryFilterSelect', 'factoryLineFilterSelect', 'factoryLineMachineFilterSelect']
            .forEach(id => {
                const el = document.getElementById(id);
                if (el) el.addEventListener('change', reconnect);
            });

        /* AI Refresh 버튼 클릭 시 피드 초기화 후 재연결 */
        const btn = document.getElementById('aiRefreshBtn');
        if (btn) btn.addEventListener('click', reconnect);
    });

})();
