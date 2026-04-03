/**
 * ============================================================
 * 파일명: ai_optimization_2.js
 * 목적: F13 — AI Production Optimization (생산 최적화 분석)
 *       - proc/ai_optimization_2.php API를 호출하여 OEE 목표 미달 라인 분석
 *       - 우선순위(P1/P2/P3)별로 개선 기회(opportunities) 카드 렌더링
 *       - 현재 OEE, 잠재 OEE, 보틀넥(병목) 요소, 개선 제안 표시
 *       - 공장/라인/기계 필터 및 날짜 범위 필터 지원
 *       - 페이지 로드 2.2초 후 최초 로드, 이후 필터 변경 시 재조회
 * v4(ai_optimization.js) 대비 수정:
 *  - API 엔드포인트: proc/ai_optimization_2.php
 *  - getFilters(): date_range 파라미터 추가 (dateRangeSelect 값 반영)
 *  - Refresh 버튼 이벤트에 dateRangeSelect change 감지 추가
 * ============================================================
 */

/* IIFE(즉시 실행 함수)로 전역 스코프 오염 방지 */
(function () {
    'use strict';

    /**
     * 현재 선택된 공장/라인/기계/날짜 범위 필터값을 객체로 반환
     * - dateRangeSelect: 날짜 범위 선택 ('today' 기본값)
     * @returns {Object} API 호출에 사용할 필터 파라미터 객체
     */
    function getFilters() {
        return {
            factory_filter: document.getElementById('factoryFilterSelect')?.value || '',
            line_filter: document.getElementById('factoryLineFilterSelect')?.value || '',
            machine_filter: document.getElementById('factoryLineMachineFilterSelect')?.value || '',
            date_range: document.getElementById('dateRangeSelect')?.value || 'today',  // 날짜 범위 (기본: today)
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

    /**
     * proc/ai_optimization_2.php API를 호출하여 최적화 분석 데이터를 로드하고 화면에 표시
     * - 로드 중에는 스피너 로딩 메시지 표시
     * - 응답 code가 '00'인 경우만 summary 및 opportunities 렌더링
     * - 오류 발생 시 에러 메시지 표시
     */
    async function loadOptimization() {
        /* 최적화 결과 목록 컨테이너에 로딩 스피너 표시 */
        const container = document.getElementById('aiOptList');
        if (container) {
            container.innerHTML = '<div class="ai-empty-state"><span class="ai-spinner"></span> Analyzing optimization opportunities...</div>';
        }

        /* 필터를 쿼리스트링으로 변환 */
        const qs = buildQS(getFilters());
        try {
            /* AI 최적화 분석 API 호출 */
            const res = await fetch(`proc/ai_optimization_2.php${qs ? '?' + qs : ''}`);
            const data = await res.json();

            /* 응답 코드가 '00'이 아니면 처리 중단 */
            if (data.code !== '00') return;
            /* 요약 정보 렌더링 */
            renderSummary(data.summary);
            /* 개선 기회 목록 렌더링 */
            renderOpportunities(data.opportunities);
        } catch (e) {
            /* 네트워크 오류 또는 JSON 파싱 실패 시 에러 메시지 표시 */
            console.warn('[AI Optimization]', e);
            if (container) container.innerHTML = '<div class="ai-empty-state">Failed to load data</div>';
        }
    }

    /**
     * 최적화 분석 요약(summary) 데이터를 aiOptSummary 영역에 렌더링
     * - 총 라인 수, 목표 미달 라인 수, 평균 OEE, 최고 OEE, 목표 OEE 표시
     * - summary.msg가 있으면 메시지 텍스트만 표시 (데이터 없음 상황)
     * @param {Object} s - API 응답의 summary 객체
     */
    function renderSummary(s) {
        /* 요약 표시 DOM 요소 참조 */
        const el = document.getElementById('aiOptSummary');
        if (!el || !s) return;

        /* summary.msg가 있으면 단순 텍스트만 표시 (분석 불가 상황) */
        if (s.msg) { el.textContent = s.msg; return; }

        /* 분석 기간 레이블 생성 (analysis_days 없으면 기본값 '14d')
         * ※ dayLabel은 현재 innerHTML에 직접 삽입되지 않지만,
         *    향후 서브타이틀 또는 툴팁 표시용으로 유보된 변수입니다. (기존 코드 원본 유지) */
        const dayLabel = s.analysis_days ? s.analysis_days + 'd' : '14d'; // eslint-disable-line no-unused-vars
        /* 요약 통계 HTML 생성 및 삽입 */
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

    /**
     * 개선 기회(opportunities) 목록을 aiOptList 컨테이너에 카드 형태로 렌더링
     * - 기회가 없으면 목표 달성 메시지 표시
     * - 우선순위(P1/P2/P3)별 색상으로 카드 좌측 테두리 구분
     * - 현재 OEE와 잠재 OEE 바(progress bar) 시각화
     * - 보틀넥(병목) 요소 강조 표시
     * - 개선 제안은 <details>로 접어두기 가능
     * @param {Array} opps - API 응답의 opportunities 배열
     */
    function renderOpportunities(opps) {
        /* 개선 기회 목록 컨테이너 참조 */
        const container = document.getElementById('aiOptList');
        if (!container) return;

        /* 개선 기회가 없으면 목표 달성 메시지 표시 */
        if (!opps?.length) {
            container.innerHTML = '<div class="ai-empty-state">All lines are meeting the OEE target (85%).</div>';
            return;
        }

        /* 우선순위별 배지 색상 (P1: 빨강/위험, P2: 주황/경고, P3: 파랑/정보) */
        const PCOLORS = { P1: '#da1e28', P2: '#e67e22', P3: '#0070f2' };
        /* 트렌드 방향별 아이콘 및 색상 (improving: 초록 상승, declining: 빨강 하락, stable: 회색 유지) */
        const TRENDS = {
            improving: { icon: '&#8593;', color: 'var(--sap-positive)' },
            declining: { icon: '&#8595;', color: 'var(--sap-negative)' },
            stable: { icon: '&#8594;', color: 'var(--sap-text-secondary)' },
        };
        /* 보틀넥 유형별 아이콘 (availability: 톱니바퀴, performance: 재생, quality: 확대경) */
        const BICONS = { availability: '&#9881;', performance: '&#9654;', quality: '&#128300;' };

        /* 각 개선 기회를 카드 HTML로 변환 후 join으로 합치기 */
        container.innerHTML = opps.map((op) => {
            /* 우선순위 색상 (알 수 없으면 회색) */
            const pc = PCOLORS[op.priority] || '#888';
            /* 트렌드 방향 (알 수 없으면 stable) */
            const tr = TRENDS[op.trend] || TRENDS.stable;
            /* 보틀넥 아이콘 (알 수 없으면 경고 기호) */
            const bicon = BICONS[op.bottleneck] || '&#9888;';
            /* 바 너비를 0~100% 범위로 제한 */
            const barCur = Math.min(100, op.current_oee);
            const barPot = Math.min(100, op.potential_oee);
            const barTgt = Math.min(100, op.target_oee);
            /* 개선 제안 목록을 <li> 태그로 변환 */
            const suggs = op.suggestions.map(s => `<li>${s}</li>`).join('');

            /**
             * 보틀넥 요소의 CSS 클래스를 반환하는 헬퍼
             * - 현재 보틀넥 요소이면 highlight 클래스 적용
             * @param {string} key - 비교할 보틀넥 키 (availability/performance/quality)
             * @returns {string} CSS 클래스명 또는 빈 문자열
             */
            const botHighlight = (key) => op.bottleneck === key ? 'ai-opt-comp--highlight' : '';

            /* 개선 기회 카드 HTML 생성 */
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
                <div class="ai-opt-bar-fill ai-opt-bar-fill--potential" style="width:${barPot}%;"></div>
                <div class="ai-opt-bar-target" style="left:${barTgt}%;"></div>
              </div>
              <span class="ai-opt-bar-val" style="color:var(--sap-informative);">
                ${op.potential_oee}% <small>(+${op.potential_gain}%)</small>
              </span>
            </div>
          </div>

          <div class="ai-opt-bottleneck">
            ${bicon} <strong>Bottleneck:</strong>
            ${op.bottleneck_label} ${op.bottleneck_current}%
            &rarr; target ${op.bottleneck_target}%
            (gap <strong style="color:${pc};">${op.bottleneck_gap}%p</strong>)
          </div>

          <div class="ai-opt-components">
            <span class="${botHighlight('availability')}">&#9881; Avail. ${op.avg_avail}%</span>
            <span class="${botHighlight('performance')}">&#9654; Perf. ${op.avg_perf}%</span>
            <span class="${botHighlight('quality')}">&#128300; Quality ${op.avg_quality}%</span>
            <span style="color:var(--sap-text-secondary); font-size:0.75rem;">
              &#127942; vs. best line -${op.vs_best}%p
            </span>
          </div>

          <details class="ai-opt-suggestions">
            <summary>&#128161; Improvement Suggestions (${op.suggestions.length})</summary>
            <ul>${suggs}</ul>
          </details>
        </div>
      `;
        }).join('');
    }

    /**
     * DOMContentLoaded 이벤트 시 실행되는 초기화 블록
     * 1. 2.2초 딜레이 후 최초 최적화 분석 데이터 로드
     *    (다른 모듈 초기화가 완료될 때까지 대기)
     * 2. 공장/라인/기계/날짜범위 필터 변경 시 0.5초 딜레이 후 재조회
     * 3. aiRefreshBtn 클릭 시 즉시 재조회
     */
    document.addEventListener('DOMContentLoaded', () => {
        /* 페이지 로드 후 2.2초 딜레이로 최초 데이터 로드 (다른 모듈 초기화 대기) */
        setTimeout(loadOptimization, 2200);

        /* 공장/라인/기계/날짜범위 필터 변경 시 0.5초 딜레이 후 재조회 */
        ['factoryFilterSelect', 'factoryLineFilterSelect', 'factoryLineMachineFilterSelect', 'dateRangeSelect']
            .forEach(id => {
                const el = document.getElementById(id);
                /* 각 필터 select 요소에 change 이벤트 리스너 등록 */
                if (el) el.addEventListener('change', () => setTimeout(loadOptimization, 500));
            });

        /* AI Refresh 버튼 클릭 시 즉시 최적화 분석 재조회 */
        const btn = document.getElementById('aiRefreshBtn');
        if (btn) btn.addEventListener('click', loadOptimization);
    });

})();
