/**
 * ============================================================
 * 파일명: ai_oee_overlay_2.js
 * 목적: AI OEE Overlay — OEE Trend 차트에 AI 예측선 + 신뢰구간 오버레이
 *       - data_oee.js의 updateOeeTrendChart 함수를 monkey-patch하여
 *         실제 OEE 차트 위에 AI 예측 데이터 오버레이
 *       - proc/ai_oee_prediction_2.php 에서 forecast, upper, lower 데이터 조회
 *       - CI Upper(신뢰구간 상단)와 CI Lower(신뢰구간 하단) 사이를 반투명 채우기
 *       - AI Forecast 라인은 주황 점선으로 표시
 *       - 트렌드 방향 배지(up/down/stable)도 함께 업데이트
 * ============================================================
 */

/* IIFE(즉시 실행 함수)로 전역 스코프 오염 방지 */
(function () {
    'use strict';

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
            .filter(([, v]) => v !== '')  // 빈 값 필터링
            .map(([k, v]) => `${encodeURIComponent(k)}=${encodeURIComponent(v)}`)
            .join('&');
    }

    // ── 예측 오버레이 적용 ────────────────────────────────────────────
    /**
     * OEE Trend 차트에 AI 예측 오버레이(신뢰구간 + 예측선)를 비동기로 적용
     * - charts.oeeTrend 전역 차트 인스턴스가 없으면 즉시 종료
     * - proc/ai_oee_prediction_2.php API에서 forecast 배열 조회
     * - 기존 AI 오버레이 데이터셋(_aiOverlay 플래그)을 제거 후 새로 추가
     * - 기존 라벨과 겹치지 않는 예측 라벨을 차트 레이블에 추가
     * - 기존 데이터셋의 길이를 allLabels에 맞춰 null로 패딩
     */
    async function applyPredictionOverlay() {
        /* 전역 charts 객체 또는 oeeTrend 차트 인스턴스가 없으면 종료 */
        if (typeof charts === 'undefined' || !charts.oeeTrend) return;

        /* 현재 필터를 쿼리스트링으로 변환 */
        const qs = buildQS(getFilters());
        try {
            /* OEE 예측 API 호출 */
            const res = await fetch(`proc/ai_oee_prediction_2.php?${qs}`);
            const data = await res.json();

            /* 응답 코드가 '00'이 아니거나 forecast 배열이 없으면 종료 */
            if (data.code !== '00' || !data.forecast?.length) return;

            /* 차트 인스턴스 참조 */
            const chart = charts.oeeTrend;

            // 기존 AI 오버레이 데이터셋 제거
            /* _aiOverlay 플래그가 있는 데이터셋만 필터링하여 제거 */
            chart.data.datasets = chart.data.datasets.filter(d => !d._aiOverlay);

            /* 현재 차트의 기존 레이블 배열 복사 */
            const existingLabels = [...chart.data.labels];
            /* 예측 데이터의 레이블 배열 추출 */
            const forecastLabels = data.forecast.map(f => f.label);

            // 기존 라벨과 겹치지 않는 예측 라벨만 추가
            const newLabels = forecastLabels.filter(l => !existingLabels.includes(l));
            /* 기존 라벨 + 새 예측 라벨 합산 */
            const allLabels = [...existingLabels, ...newLabels];
            chart.data.labels = allLabels;

            // 기존 데이터셋을 새 길이에 맞게 null로 패딩
            /* 기존 데이터셋이 새 라벨 수보다 짧으면 null로 채워 길이를 맞춤 */
            chart.data.datasets.forEach(ds => {
                while (ds.data.length < allLabels.length) ds.data.push(null);
            });

            /**
             * allLabels 기준으로 예측 데이터 배열 생성 헬퍼
             * - 레이블에 해당하는 예측값이 있으면 사용, 없으면 null
             * @param {string} key - forecast 객체에서 추출할 키 (oee/upper/lower)
             * @returns {Array} 레이블 순서에 맞는 예측값 배열
             */
            const makeSeries = (key) => allLabels.map(label => {
                const f = data.forecast.find(x => x.label === label);
                return f ? f[key] : null;  // 해당 레이블의 예측값 또는 null
            });

            /* OEE 예측값, 신뢰구간 상단, 신뢰구간 하단 배열 각각 생성 */
            const forecastData = makeSeries('oee');
            const upperData = makeSeries('upper');
            const lowerData = makeSeries('lower');

            // CI 상단 (fill → CI 하단)
            /* 신뢰구간 상단 데이터셋: CI Lower 방향으로 면적 채우기 */
            chart.data.datasets.push({
                label: 'CI Upper',
                data: upperData,
                borderColor: 'transparent',          // 테두리 숨김
                backgroundColor: 'rgba(245,166,35,0.15)', // 반투명 주황 면적
                fill: '+1',                           // 바로 다음 데이터셋(CI Lower)까지 채우기
                pointRadius: 0,                       // 포인트 마커 숨김
                tension: 0.4,
                _aiOverlay: true,                     // AI 오버레이 식별 플래그
            });

            // CI 하단
            /* 신뢰구간 하단 데이터셋: fill 없음(채우기는 CI Upper에서 처리) */
            chart.data.datasets.push({
                label: 'CI Lower',
                data: lowerData,
                borderColor: 'transparent',
                backgroundColor: 'transparent',
                fill: false,
                pointRadius: 0,
                tension: 0.4,
                _aiOverlay: true,
            });

            // 예측선 (점선 주황)
            /* AI Forecast 주선: 주황색 점선(borderDash), 포인트 마커 표시 */
            chart.data.datasets.push({
                label: 'AI Forecast',
                data: forecastData,
                borderColor: '#f5a623',               // 주황색
                backgroundColor: 'transparent',
                borderDash: [6, 4],                   // 6px 선 + 4px 공백 점선 패턴
                borderWidth: 2,
                pointRadius: 4,
                pointBackgroundColor: '#f5a623',
                tension: 0.4,
                fill: false,
                _aiOverlay: true,
            });

            /* 애니메이션 없이 차트 즉시 갱신 */
            chart.update('none');

            // 트렌드 뱃지 업데이트
            /* 예측 트렌드 방향에 따라 배지 업데이트 */
            updateTrendBadge(data);

        } catch (e) {
            /* 네트워크 오류 또는 JSON 파싱 실패 시 경고 로그만 출력 */
            console.warn('[AI OEE Overlay]', e);
        }
    }

    // ── 트렌드 방향 뱃지 ──────────────────────────────────────────────
    /**
     * API 응답의 trend 값에 따라 OEE 트렌드 방향 배지(aiOeeTrendBadge)를 업데이트
     * - up: 상승 트렌드 (↑ Trending Up)
     * - down: 하락 트렌드 (↓ Trending Down)
     * - stable: 안정 트렌드 (→ Stable)
     * @param {Object} data - OEE 예측 API 응답 객체 (trend 필드 포함)
     */
    function updateTrendBadge(data) {
        /* 트렌드 배지 DOM 요소 참조 */
        let badge = document.getElementById('aiOeeTrendBadge');
        if (!badge) return;

        /* 트렌드 방향별 CSS 클래스 및 텍스트 매핑 */
        const trendMap = {
            up: { cls: 'ai-trend-badge--up', text: '↑ Trending Up' },
            down: { cls: 'ai-trend-badge--down', text: '↓ Trending Down' },
            stable: { cls: 'ai-trend-badge--stable', text: '→ Stable' },
        };
        /* 알 수 없는 트렌드 값은 stable로 처리 */
        const t = trendMap[data.trend] || trendMap.stable;
        badge.className = 'ai-trend-badge ' + t.cls;
        badge.textContent = t.text;
    }

    // ── updateOeeTrendChart 함수 패치 ────────────────────────────────
    /**
     * DOMContentLoaded 이벤트 시 updateOeeTrendChart 함수를 monkey-patch하여
     * 기존 차트 업데이트 함수 호출 후 자동으로 AI 예측 오버레이 적용
     *
     * 처리 순서:
     * 1. setInterval로 updateOeeTrendChart 전역 함수가 정의될 때까지 100ms마다 폴링
     * 2. 함수 발견 시 clearInterval로 폴링 중단
     * 3. 원본 함수를 래핑하여 오버라이드: 원본 실행 후 400ms 딜레이 후 overlay 적용
     * 4. Refresh 버튼 클릭 시 600ms 딜레이 후 overlay 재적용
     * 5. 공장/라인/기계 필터 변경 시 600ms 딜레이 후 overlay 재적용
     */
    document.addEventListener('DOMContentLoaded', () => {
        /* updateOeeTrendChart 함수 정의를 100ms마다 폴링하여 대기 */
        const wait = setInterval(() => {
            if (typeof updateOeeTrendChart === 'function') {
                /* 함수 발견: 폴링 중단 */
                clearInterval(wait);

                /* 원본 updateOeeTrendChart 함수 보존 */
                const original = window.updateOeeTrendChart;
                /* monkey-patch: 원본 함수 실행 후 AI overlay 추가 적용 */
                window.updateOeeTrendChart = function (trendStats) {
                    /* 원본 함수 호출 (this 컨텍스트 보존) */
                    original.call(this, trendStats);
                    /* 차트 렌더링 완료 후 400ms 딜레이로 overlay 적용 */
                    setTimeout(applyPredictionOverlay, 400);
                };
            }
        }, 100);  // 100ms마다 폴링

        // Refresh 버튼에도 훅
        /* Refresh 버튼 클릭 시 600ms 후 overlay 재적용 */
        const btn = document.getElementById('refreshBtn');
        if (btn) btn.addEventListener('click', () => setTimeout(applyPredictionOverlay, 600));

        // 필터 변경 시 재조회
        /* 각 필터 select 요소에 change 이벤트 리스너 등록 */
        ['factoryFilterSelect', 'factoryLineFilterSelect', 'factoryLineMachineFilterSelect']
            .forEach(id => {
                const el = document.getElementById(id);
                /* 필터 변경 후 600ms 딜레이로 overlay 재적용 */
                if (el) el.addEventListener('change', () => setTimeout(applyPredictionOverlay, 600));
            });
    });

})();
