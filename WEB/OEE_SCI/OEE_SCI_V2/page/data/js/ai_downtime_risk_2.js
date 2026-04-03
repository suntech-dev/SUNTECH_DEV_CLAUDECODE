/**
 * ============================================================
 * 파일명: ai_downtime_risk_2.js
 * 목적: AI Downtime Risk — 다운타임 테이블에 기계별 AI 위험도 열 추가
 *       - MutationObserver로 tbody 변화 감지 → ai_maintenance_2.php 위험도 매핑
 *       - 각 행에 DANGER / CAUTION / NORMAL 배지와 위험도 점수(%) 표시
 *       - 공장/라인/기계 필터 변경 및 Refresh 버튼 클릭 시 위험도 재조회
 *       - 2분(120초)마다 자동 갱신
 * ============================================================
 */

/* IIFE(즉시 실행 함수)로 전역 스코프 오염 방지 */
(function () {
    'use strict';

    /* 기계번호(machine_no)를 키로 위험도 정보를 보관하는 맵 */
    // { machine_no: { risk_level, risk_score } }
    let riskMap = {};

    /**
     * 현재 선택된 공장/라인/기계 필터값을 객체로 반환
     * - optional chaining(?.)으로 요소가 없을 경우 빈 문자열 반환
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
     * - 빈 문자열 값을 가진 키는 제외하여 불필요한 파라미터 전송 방지
     * @param {Object} obj - 변환할 파라미터 객체
     * @returns {string} URL 쿼리스트링 (예: "factory_filter=1&line_filter=2")
     */
    function buildQS(obj) {
        return Object.entries(obj)
            .filter(([, v]) => v !== '')  // 빈 값 제외
            .map(([k, v]) => `${encodeURIComponent(k)}=${encodeURIComponent(v)}`)
            .join('&');
    }

    // ── 위험도 데이터 로드 ────────────────────────────────────────────
    /**
     * proc/ai_maintenance_2.php API를 호출하여 기계별 위험도 데이터를 riskMap에 저장
     * - fetch API로 비동기 조회 (async/await 사용)
     * - 응답 code가 '00'이고 machines 배열이 있는 경우만 riskMap 갱신
     * - 조회 완료 후 applyRiskToAllRows()를 호출하여 테이블에 즉시 반영
     */
    async function loadRiskData() {
        /* 현재 필터값을 쿼리스트링으로 변환 */
        const qs = buildQS(getFilters());
        try {
            /* AI 유지보수 위험도 API 호출 */
            const res = await fetch(`proc/ai_maintenance_2.php?${qs}`);
            const data = await res.json();

            /* riskMap 초기화 후 새 데이터 매핑 */
            riskMap = {};
            if (data.code === '00' && data.machines?.length) {
                /* 각 기계별 위험도 레벨과 점수를 riskMap에 저장 */
                data.machines.forEach(m => {
                    riskMap[m.machine_no] = {
                        risk_level: m.risk_level,
                        risk_score: m.risk_score,
                    };
                });
            }
            /* riskMap 갱신 완료 후 테이블 모든 행에 위험도 적용 */
            applyRiskToAllRows();
        } catch (e) {
            /* 네트워크 오류 또는 JSON 파싱 실패 시 경고 로그만 출력 */
            console.warn('[AI Downtime Risk]', e);
        }
    }

    // ── 모든 행에 위험도 적용 ─────────────────────────────────────────
    /**
     * downtimeDataBody의 모든 <tr> 행을 순회하며 위험도 셀 삽입
     * - tbody가 없으면 즉시 종료
     */
    function applyRiskToAllRows() {
        /* 다운타임 데이터 테이블 tbody 요소 참조 */
        const tbody = document.getElementById('downtimeDataBody');
        if (!tbody) return;
        /* 각 행에 addRiskCell 함수 적용 */
        tbody.querySelectorAll('tr').forEach(addRiskCell);
    }

    // ── 단일 행에 위험도 셀 삽입 ─────────────────────────────────────
    /**
     * 테이블의 단일 <tr> 행에 AI 위험도 <td> 셀을 삽입
     * - data-ai-risk-done 플래그로 중복 삽입 방지
     * - colspan을 가진 로딩/빈 행(셀 수 10개 미만)은 건너뜀
     * - 위험도 데이터가 없는 기계는 '-' 표시
     * @param {HTMLTableRowElement} row - 위험도 셀을 삽입할 테이블 행
     */
    function addRiskCell(row) {
        /* 이미 위험도 셀이 추가된 행은 건너뜀 */
        if (row.dataset.aiRiskDone === '1') return;

        // 로딩/빈 행 (colspan 있는 단일 셀 행) 건너뜀
        if (row.cells.length < 10) return;

        /* 행의 첫 번째 셀에서 기계번호 추출 */
        const machineNo = row.cells[0]?.textContent?.trim() || '';
        /* riskMap에서 해당 기계번호의 위험도 데이터 조회 */
        const risk = riskMap[machineNo];

        /* 위험도 셀 생성 */
        const td = document.createElement('td');
        td.className = 'fiori-table__cell';
        td.style.cssText = 'text-align:center; white-space:nowrap;';

        if (risk) {
            /* 위험도 레벨별 배경색 및 레이블 매핑 */
            const colors = { danger: '#da1e28', warning: '#e67e22', normal: '#30914c' };
            const labels = { danger: 'DANGER', warning: 'CAUTION', normal: 'NORMAL' };
            const color = colors[risk.risk_level] || '#888';  // 알 수 없는 레벨은 회색
            const label = labels[risk.risk_level] || '-';
            /* 위험도 배지 HTML 및 점수 표시 */
            td.innerHTML = `
        <span class="ai-status-badge" style="background:${color}; color:#fff;">${label}</span>
        <div style="font-size:0.65rem; color:var(--sap-text-secondary); margin-top:2px;">${risk.risk_score}%</div>`;
        } else {
            /* riskMap에 해당 기계 데이터 없음: '-' 표시 */
            td.innerHTML = '<span style="color:var(--sap-text-secondary); font-size:0.75rem;">-</span>';
        }

        // DETAIL 열 (마지막) 바로 앞에 삽입
        row.insertBefore(td, row.cells[row.cells.length - 1]);
        /* 중복 삽입 방지 플래그 설정 */
        row.dataset.aiRiskDone = '1';
    }

    // ── MutationObserver로 tbody 변화 감지 ───────────────────────────
    /**
     * MutationObserver를 사용하여 tbody에 새 행이 추가될 때마다 자동으로 위험도 셀 삽입
     * - SSE/AJAX로 테이블 행이 동적으로 추가되는 경우에도 위험도 표시 보장
     * - childList: true 옵션으로 직계 자식 노드 변화만 감지
     */
    function observeTableBody() {
        /* downtimeDataBody tbody 요소 참조 */
        const tbody = document.getElementById('downtimeDataBody');
        if (!tbody) return;

        /* MutationObserver 생성: tbody에 TR 추가 시 addRiskCell 호출 */
        new MutationObserver(mutations => {
            mutations.forEach(m => {
                /* 각 변이(mutation)에서 추가된 노드 확인 */
                m.addedNodes.forEach(node => {
                    /* 추가된 노드가 TR 요소인 경우만 위험도 셀 삽입 */
                    if (node.nodeName === 'TR') addRiskCell(node);
                });
            });
        }).observe(tbody, { childList: true });  // 직계 자식 변화 감지
    }

    // ── 초기화 ────────────────────────────────────────────────────────
    /**
     * DOMContentLoaded 이벤트 시 실행되는 초기화 블록
     * 1. MutationObserver 설정 (테이블 동적 행 감지)
     * 2. 1.2초 딜레이 후 최초 위험도 데이터 로드 (테이블 초기 렌더링 대기)
     * 3. 공장/라인/기계 필터 변경 시 위험도 재조회
     * 4. Refresh 버튼 클릭 시 위험도 재조회
     * 5. 2분(120,000ms)마다 자동 갱신 setInterval 설정
     */
    document.addEventListener('DOMContentLoaded', () => {
        /* MutationObserver 등록으로 동적 행 추가 감지 시작 */
        observeTableBody();
        /* 테이블 초기 렌더링이 완료된 이후(1.2초 후) 위험도 데이터 최초 로드 */
        setTimeout(loadRiskData, 1200);

        // 필터 변경 시 재조회 (기존 행 risk-done 플래그 리셋 후 재적용)
        ['factoryFilterSelect', 'factoryLineFilterSelect', 'factoryLineMachineFilterSelect']
            .forEach(id => {
                const el = document.getElementById(id);
                /* 각 필터 select 요소에 change 이벤트 리스너 등록 */
                if (el) el.addEventListener('change', () => {
                    // 플래그 초기화 후 재로드
                    /* 기존 행의 ai-risk-done 플래그를 모두 삭제하여 재처리 가능하게 함 */
                    document.querySelectorAll('#downtimeDataBody tr[data-ai-risk-done]')
                        .forEach(r => delete r.dataset.aiRiskDone);
                    /* 0.5초 딜레이 후 위험도 재조회 (필터 변경 후 테이블 갱신 대기) */
                    setTimeout(loadRiskData, 500);
                });
            });

        // Refresh 버튼
        /* Refresh 버튼 클릭 시 플래그 초기화 후 위험도 재조회 */
        const btn = document.getElementById('refreshBtn');
        if (btn) btn.addEventListener('click', () => {
            /* 기존 행의 ai-risk-done 플래그 삭제 */
            document.querySelectorAll('#downtimeDataBody tr[data-ai-risk-done]')
                .forEach(r => delete r.dataset.aiRiskDone);
            /* 0.6초 딜레이 후 위험도 재조회 */
            setTimeout(loadRiskData, 600);
        });

        // 2분마다 자동 갱신
        /* setInterval로 120초(2분)마다 위험도 데이터 자동 갱신 */
        setInterval(loadRiskData, 120000);
    });

})();
