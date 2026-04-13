/**
 * ============================================================
 * 파일명: ai_dashboard_2.js
 * 목적: AI Intelligence Dashboard 메인 컨트롤러
 *       - OEE 예측 차트 (실제 + 예측 + 신뢰구간) 렌더링
 *       - 이상 감지(Anomaly Detection) 카드 렌더링
 *       - 예방정비 위험도(Maintenance Risk) 목록 렌더링
 *       - 라인 건강지수(Line Health) 목록 렌더링
 *       - 공장/라인/기계 필터 시스템 운용
 *       - 60초마다 전체 데이터 자동 갱신
 * v4(ai_dashboard.js) 대비 수정:
 *  - renderForecastChart(): today_data 기반 Actual OEE solid 라인 추가
 *  - loadMaintenanceRisk() / loadLineHealth(): proc/ai_maintenance_dash_2.php 호출
 *  - updateLineHealthSubtitle(): date_range 에 맞게 서브타이틀 동적 변경
 * ============================================================
 */

/* ── 차트 색상 팔레트 상수 ──────────────────────────────────────
 * 다크 테마 기반 색상 정의 (primary: SAP 브랜드 블루, forecast: 주황, 등)
 */
const AI_COLORS = {
    primary: '#0070f2',       // 실제 OEE 라인 색상 (SAP 블루)
    forecast: '#f5a623',      // AI 예측선 색상 (주황)
    ci: 'rgba(245,166,35,0.15)', // 신뢰구간 영역 채우기 색상 (반투명 주황)
    danger: '#f85149',        // 위험 표시 색상 (빨강)
    warning: '#d29922',       // 경고 표시 색상 (노랑)
    normal: '#3fb950',        // 정상 표시 색상 (초록)
    info: '#58a6ff',          // 정보 표시 색상 (하늘)
    chartGrid: 'rgba(255,255,255,0.08)', // 차트 격자선 색상 (연한 흰색)
    chartText: '#8b949e',     // 차트 축 텍스트 색상 (회색)
};

/* 자동 새로고침 주기 (밀리초): 60초마다 전체 데이터 갱신 */
const REFRESH_INTERVAL = 60000;

/* 전역 상태 객체: 현재 선택된 필터값 및 차트 인스턴스 보관 */
let aiState = {
    factory: '',          // 선택된 공장 idx
    line: '',             // 선택된 라인 idx
    machine: '',          // 선택된 기계 idx
    forecastChart: null,  // Chart.js OEE 예측 차트 인스턴스 (재사용/업데이트용)
    refreshTimer: null,   // setInterval 타이머 핸들
};

// ============================================================
// 필터 시스템
// ============================================================
/**
 * 공장/라인/기계 필터 드롭다운 초기화 및 이벤트 리스너 설정
 * - 페이지 로드 시 공장 목록을 AJAX로 조회하여 select에 삽입
 * - 공장 선택 시 해당 공장의 라인 목록 로드 (연쇄 필터링)
 * - 라인 선택 시 해당 라인의 기계 목록 로드 (연쇄 필터링)
 * - 각 필터 변경 시 refreshAll() 호출하여 전체 데이터 갱신
 */
function initFilterSystem() {
    // 공장 목록 AJAX 조회 후 select에 옵션 추가
    $.getJSON('../manage/proc/factory.php?status_filter=Y', function (res) {
        if (!res.success || !res.data) return;
        var factories = res.data.filter(function (item) { return Number(item.idx) !== 99; });
        factories.forEach(function (item) {
            $('#factoryFilterSelect').append(
                $('<option>').val(item.idx).text(item.factory_name)
            );
        });
        if (factories.length === 1) {
            $('#factoryFilterSelect').val(factories[0].idx).trigger('change');
        }
    });

    // 공장 선택 변경 이벤트 리스너
    $('#factoryFilterSelect').on('change', function () {
        // 선택된 공장값을 전역 상태에 저장
        aiState.factory = $(this).val();
        // 하위 필터(라인, 기계) 초기화
        aiState.line = '';
        aiState.machine = '';
        // 라인/기계 드롭다운 초기화 및 비활성화
        $('#factoryLineFilterSelect').html('<option value="">All Line</option>').prop('disabled', true);
        $('#factoryLineMachineFilterSelect').html('<option value="">All Machine</option>').prop('disabled', true);

        // 공장이 선택된 경우 해당 공장의 라인/기계 목록 조회
        if (aiState.factory) {
            $.getJSON('../manage/proc/line.php', { factory_filter: aiState.factory }, function (res) {
                if (!res.success || !res.data) return;
                // 라인 드롭다운 활성화 및 옵션 추가
                $('#factoryLineFilterSelect').prop('disabled', false);
                res.data.forEach(function (item) {
                    $('#factoryLineFilterSelect').append(
                        $('<option>').val(item.idx).text(item.line_name)
                    );
                });
            });
            // 공장 선택 시 기계 드롭다운도 해당 공장 기준으로 활성화
            $.getJSON('../manage/proc/machine.php', { factory_filter: aiState.factory }, function (res) {
                if (!res.success || !res.data) return;
                $('#factoryLineMachineFilterSelect').html('<option value="">All Machine</option>');
                res.data.forEach(function (item) {
                    $('#factoryLineMachineFilterSelect').append(
                        $('<option>').val(item.idx).text(item.machine_no)
                    );
                });
                $('#factoryLineMachineFilterSelect').prop('disabled', false);
            });
        }
        // 필터 변경 후 전체 데이터 갱신
        refreshAll();
    });

    // 라인 선택 변경 이벤트 리스너
    $('#factoryLineFilterSelect').on('change', function () {
        // 선택된 라인값을 전역 상태에 저장, 기계 초기화
        aiState.line = $(this).val();
        aiState.machine = '';
        // 기계 드롭다운 초기화 및 비활성화
        $('#factoryLineMachineFilterSelect').html('<option value="">All Machine</option>').prop('disabled', true);

        // 라인이 선택된 경우 해당 라인의 기계 목록 조회
        if (aiState.line) {
            const params = { line_filter: aiState.line };
            // 공장 필터도 함께 전달 (존재하는 경우)
            if (aiState.factory) params.factory_filter = aiState.factory;
            $.getJSON('../manage/proc/machine.php', params, function (res) {
                if (!res.success || !res.data) return;
                // 기계 드롭다운 활성화 및 옵션 추가
                $('#factoryLineMachineFilterSelect').prop('disabled', false);
                res.data.forEach(function (item) {
                    $('#factoryLineMachineFilterSelect').append(
                        $('<option>').val(item.idx).text(item.machine_no)
                    );
                });
            });
        }
        // 필터 변경 후 전체 데이터 갱신
        refreshAll();
    });

    // 기계 선택 변경 이벤트 리스너
    $('#factoryLineMachineFilterSelect').on('change', function () {
        // 선택된 기계값을 전역 상태에 저장 후 전체 데이터 갱신
        aiState.machine = $(this).val();
        refreshAll();
    });

    // 수동 새로고침 버튼 클릭 이벤트 리스너
    $('#aiRefreshBtn').on('click', function () { refreshAll(); });
}

/**
 * 현재 선택된 필터 파라미터 객체 반환
 * - 값이 있는 필터만 포함 (빈 문자열 제외)
 * @returns {Object} API 호출에 사용할 필터 파라미터 객체
 */
function getFilterParams() {
    const p = {};
    if (aiState.factory) p.factory_filter = aiState.factory;
    if (aiState.line) p.line_filter = aiState.line;
    if (aiState.machine) p.machine_filter = aiState.machine;
    return p;
}

// ============================================================
// 전체 새로고침
// ============================================================
/**
 * 모든 AI 대시보드 패널 데이터를 일괄 갱신
 * - OEE 예측, 이상 감지, 예방정비 위험도, 마지막 업데이트 시간, 라인 건강지수 서브타이틀 갱신
 */
function refreshAll() {
    loadPrediction();         // OEE 예측 데이터 로드
    loadAnomalyDetection();   // 이상 감지 데이터 로드
    loadMaintenanceRisk();    // 예방정비 위험도 데이터 로드
    updateLastUpdateTime();   // 마지막 업데이트 시간 표시 갱신
    updateLineHealthSubtitle(); // 라인 건강지수 서브타이틀 갱신
}

/**
 * 화면 상단의 마지막 업데이트 시간 텍스트 갱신
 * - 현재 로컬 시간을 HH:MM:SS 형식으로 표시
 */
function updateLastUpdateTime() {
    const now = new Date();
    const hh = String(now.getHours()).padStart(2, '0');
    const mm = String(now.getMinutes()).padStart(2, '0');
    const ss = String(now.getSeconds()).padStart(2, '0');
    $('#aiLastUpdateTime').text('Updated: ' + hh + ':' + mm + ':' + ss);
}

// Line Health 서브타이틀 동적 업데이트
/**
 * dateRangeSelect 값에 따라 Line Health 패널의 서브타이틀 텍스트 동적 변경
 * - today/yesterday/7d → '7-day OEE average per line'
 * - 30d → '30-day OEE average per line'
 */
function updateLineHealthSubtitle() {
    // dateRangeSelect 선택 요소 참조
    const sel = document.getElementById('dateRangeSelect');
    // 날짜 범위 값에 따른 서브타이틀 레이블 매핑
    const rangeMap = {
        today: '7-day OEE average per line',
        yesterday: '7-day OEE average per line',
        '7d': '7-day OEE average per line',
        '30d': '30-day OEE average per line',
    };
    // 현재 선택값에 맞는 레이블 결정 (없으면 기본값 사용)
    const label = sel ? (rangeMap[sel.value] || '7-day OEE average per line') : '7-day OEE average per line';
    // ai-health-subtitle 요소에 텍스트 설정
    const el = document.querySelector('.ai-health-subtitle');
    if (el) el.textContent = 'Based on ' + label;
}

// ============================================================
// 1. OEE 예측 (기본 구현 — ai_dashboard_2.php 에서 오버라이드)
// ============================================================
/**
 * OEE 예측 데이터를 API에서 로드하여 화면 갱신
 * - proc/ai_oee_prediction_dash_2.php 호출
 * - 현재 OEE, 예측 평균, 트렌드 배지, 예측 차트 업데이트
 * - 성공 후 loadLineHealth() 연쇄 호출
 */
function loadPrediction() {
    // OEE 예측 API 호출 (필터 파라미터 포함)
    $.getJSON('proc/ai_oee_prediction_dash_2.php', getFilterParams(), function (data) {
        if (data.code !== '00') return;

        // 현재 OEE 값 표시 (null이면 '--' 출력)
        const curOee = data.current_oee !== null ? data.current_oee : '--';
        $('#aiPredCurrentOee').text(curOee !== '--' ? curOee + '%' : '--');

        // 예측 배열이 있으면 4시간 평균 계산하여 서브텍스트 표시
        let forecastAvg = '--';
        if (data.forecast && data.forecast.length > 0) {
            const sum = data.forecast.reduce((s, f) => s + f.oee, 0);
            forecastAvg = (sum / data.forecast.length).toFixed(1);
            $('#aiPredSub').text('Next 4H avg: ' + forecastAvg + '%');
        } else {
            // 예측 데이터 없음
            $('#aiPredSub').text('Insufficient data');
        }

        // 트렌드 배지 클래스/텍스트 매핑 정의
        const trendMap = {
            up: { cls: 'ai-trend-badge--up', text: 'Trending Up' },
            down: { cls: 'ai-trend-badge--down', text: 'Trending Down' },
            stable: { cls: 'ai-trend-badge--stable', text: 'Stable' },
        };
        // API 응답의 trend 값에 맞는 배지 스타일 적용
        const trend = trendMap[data.trend] || trendMap.stable;
        $('#aiPredTrendBadge')
            .removeClass('ai-trend-badge--up ai-trend-badge--down ai-trend-badge--stable')
            .addClass(trend.cls)
            .text(trend.text);

        // 예측 차트 렌더링
        renderForecastChart(data);
        // 라인 건강지수 데이터 로드 (예측 데이터 로드 완료 후 연쇄 호출)
        loadLineHealth();
    }).fail(function () {
        // API 호출 실패 시 오류 메시지 표시
        $('#aiPredSub').text('API error');
    });
}

// ============================================================
// 1-1. OEE 예측 차트 — Actual OEE solid 라인 추가
// ============================================================
/**
 * Chart.js를 사용하여 OEE 예측 차트를 렌더링
 * - Actual OEE: 오늘 실제 데이터 기반 solid 라인 (파란색)
 * - AI Forecast: 예측 데이터 dashed 라인 (주황색)
 * - CI Upper/Lower: 예측 신뢰구간 영역 채우기
 * - 차트가 이미 존재하면 data/options를 교체하고 update()만 호출 (성능 최적화)
 * @param {Object} predData - OEE 예측 API 응답 데이터
 */
function renderForecastChart(predData) {
    const canvas = document.getElementById('aiOeeForecastChart');
    if (!canvas) return;

    // 오늘 실제 OEE 데이터 (today_data: [{hour, label, oee}])
    const todayArr = predData.today_data || [];
    const actualLabels = todayArr.map(function (d) { return d.label; });
    const actualValues = todayArr.map(function (d) { return d.oee; });

    // 예측 데이터
    const forecastArr = predData.forecast || [];
    const forecastLabels = forecastArr.map(function (f) { return f.label; });
    const forecastValues = forecastArr.map(function (f) { return f.oee; });
    // 신뢰구간 상단/하단 배열 추출
    const ciUpper = forecastArr.map(function (f) { return f.upper; });
    const ciLower = forecastArr.map(function (f) { return f.lower; });

    // 연결점: 마지막 실제값 → 예측 첫 점 이음
    const connectOee = predData.current_oee || null;
    // 연결 라벨: current_hour가 있으면 "HH:00" 형식, 없으면 "Now"
    const connectLabel = predData.current_hour !== undefined
        ? String(predData.current_hour).padStart(2, '0') + ':00' : 'Now';

    // 실제 라벨과 예측 라벨 합치기 (중복 연결점 방지)
    const allLabels = actualLabels.length > 0
        ? [...actualLabels, ...forecastLabels]
        : (connectOee ? [connectLabel, ...forecastLabels] : forecastLabels);

    // 각 구간 길이 계산
    const nActual = actualLabels.length;
    const nForecast = forecastLabels.length;
    const nTotal = allLabels.length;

    // Actual OEE: 실제 구간만 값, 나머지 null (예측 구간은 라인 끊김)
    const actualFull = actualValues.length > 0
        ? [...actualValues, ...Array(nForecast).fill(null)]
        : [];

    // AI Forecast: 실제 구간은 null, 연결점부터 예측값 배치
    let forecastFull, ciUpperFull, ciLowerFull;
    if (nActual > 0 && connectOee !== null) {
        // 실제 마지막 포인트와 예측을 연결 (nActual-1 위치에 connectOee 중복)
        forecastFull = [...Array(nActual - 1).fill(null), connectOee, ...forecastValues];
        ciUpperFull = [...Array(nActual - 1).fill(null), connectOee, ...ciUpper];
        ciLowerFull = [...Array(nActual - 1).fill(null), connectOee, ...ciLower];
    } else if (connectOee !== null) {
        // 실제 데이터 없음: 현재값부터 시작
        forecastFull = [connectOee, ...forecastValues];
        ciUpperFull = [connectOee, ...ciUpper];
        ciLowerFull = [connectOee, ...ciLower];
    } else {
        // 연결점도 없음: 예측값만 사용
        forecastFull = forecastValues;
        ciUpperFull = ciUpper;
        ciLowerFull = ciLower;
    }

    // Chart.js 데이터셋 배열 구성
    const datasets = [];

    // Actual OEE solid 라인 데이터셋 (실제 데이터가 있는 경우만 추가)
    if (actualFull.length > 0) {
        datasets.push({
            label: 'Actual OEE',
            data: actualFull,
            borderColor: AI_COLORS.primary,       // 파란색 실선
            backgroundColor: 'transparent',
            borderWidth: 2.5,
            borderDash: [],                        // 실선 (점선 없음)
            pointBackgroundColor: AI_COLORS.primary,
            pointRadius: 3,
            tension: 0.3,
            order: 0,          // 가장 위에 렌더링
            spanGaps: false,   // null 구간에서 라인 끊김
        });
    }

    // AI Forecast dashed 라인 데이터셋
    datasets.push({
        label: 'AI Forecast',
        data: forecastFull,
        borderColor: AI_COLORS.forecast,           // 주황색 점선
        backgroundColor: 'transparent',
        borderDash: [6, 4],                        // 점선 패턴: 6px 선 + 4px 공백
        borderWidth: 2.5,
        pointBackgroundColor: AI_COLORS.forecast,
        // null 값인 포인트는 반지름 0으로 숨김
        pointRadius: (ctx) => {
            const v = ctx.raw;
            return (v !== null && v !== undefined) ? 4 : 0;
        },
        tension: 0.3,
        order: 1,
        spanGaps: false,
    });

    // 신뢰구간 상단 데이터셋 (CI Upper → CI Lower 방향으로 채우기)
    datasets.push({
        label: 'CI Upper',
        data: ciUpperFull,
        borderColor: 'transparent',
        backgroundColor: AI_COLORS.ci,  // 반투명 주황 영역
        fill: '+1',                     // 다음 데이터셋(CI Lower)까지 채우기
        pointRadius: 0,
        tension: 0.3,
        order: 2,
        spanGaps: false,
    });

    // 신뢰구간 하단 데이터셋
    datasets.push({
        label: 'CI Lower',
        data: ciLowerFull,
        borderColor: 'transparent',
        backgroundColor: AI_COLORS.ci,
        fill: false,
        pointRadius: 0,
        tension: 0.3,
        order: 3,
        spanGaps: false,
    });

    // 최종 차트 데이터 객체 조합
    const chartData = { labels: allLabels, datasets: datasets };

    // Chart.js 옵션 설정 (다크 테마, 인터랙티브 툴팁)
    const options = {
        responsive: true,
        maintainAspectRatio: false,
        // 마우스 인터랙션: 동일 인덱스의 모든 데이터셋 동시 표시
        interaction: { mode: 'index', intersect: false },
        plugins: {
            legend: { display: false },  // 범례 숨김 (별도 커스텀 범례 사용)
            tooltip: {
                backgroundColor: '#1c2128',
                borderColor: '#30363d',
                borderWidth: 1,
                titleColor: '#e6edf3',
                bodyColor: '#8b949e',
                callbacks: {
                    // null 값인 항목은 툴팁에서 제외, 단위(%) 추가
                    label: function (ctx) {
                        if (ctx.raw === null || ctx.raw === undefined) return null;
                        if (ctx.dataset.label === 'CI Upper') return 'CI Upper: ' + ctx.parsed.y + '%';
                        if (ctx.dataset.label === 'CI Lower') return 'CI Lower: ' + ctx.parsed.y + '%';
                        return ctx.dataset.label + ': ' + ctx.parsed.y + '%';
                    }
                }
            }
        },
        scales: {
            // x축: 다크 테마 격자 및 텍스트 색상
            x: {
                grid: { color: AI_COLORS.chartGrid },
                ticks: { color: AI_COLORS.chartText, font: { size: 11 } },
            },
            // y축: 0~100% 고정 범위, % 단위 표시
            y: {
                min: 0, max: 100,
                grid: { color: AI_COLORS.chartGrid },
                ticks: {
                    color: AI_COLORS.chartText,
                    font: { size: 11 },
                    callback: (v) => v + '%',
                }
            }
        }
    };

    // 차트 인스턴스 재사용: 이미 있으면 data/options 교체 후 update() 호출
    if (aiState.forecastChart) {
        aiState.forecastChart.data = chartData;
        aiState.forecastChart.options = options;
        aiState.forecastChart.update('none');  // 애니메이션 없이 즉시 갱신
    } else {
        // 차트 최초 생성
        aiState.forecastChart = new Chart(canvas.getContext('2d'), {
            type: 'line',
            data: chartData,
            options: options,
        });
    }
}

// ============================================================
// 2. 이상 감지
// ============================================================
/**
 * 이상 감지 데이터를 API에서 로드하여 화면 갱신
 * - proc/ai_anomaly_2.php 호출
 * - 총 이상 수, critical/warning 요약 배지, 이상 목록 카드 업데이트
 */
function loadAnomalyDetection() {
    // 이상 감지 API 호출 (필터 파라미터 포함)
    $.getJSON('proc/ai_anomaly_2.php', getFilterParams(), function (data) {
        if (data.code !== '00') { renderAnomalyEmpty('API error'); return; }

        // 요약 데이터에서 총 이상 수 추출
        const summary = data.summary || {};
        $('#aiAnomalyTotal').text(summary.total || 0);

        // 심각도에 따라 배지 스타일/텍스트 결정
        if (summary.critical > 0) {
            // CRITICAL 상태: 빨간 배지
            $('#aiAnomalySub').text(summary.critical + ' critical · ' + summary.warning + ' warning');
            $('#aiAnomalyCriticalBadge').show().text(summary.critical + ' CRITICAL')
                .removeClass('ai-status-badge--warning ai-status-badge--normal').addClass('ai-status-badge--danger');
        } else if (summary.warning > 0) {
            // WARNING 상태: 노란 배지
            $('#aiAnomalySub').text(summary.warning + ' warnings detected');
            $('#aiAnomalyCriticalBadge').show().text(summary.warning + ' WARNING')
                .removeClass('ai-status-badge--danger ai-status-badge--normal').addClass('ai-status-badge--warning');
        } else {
            // 정상 상태: 배지 숨김
            $('#aiAnomalySub').text('All machines normal');
            $('#aiAnomalyCriticalBadge').hide();
        }

        // 이상 감지 헤더 카운트 표시 여부 결정
        if (summary.total > 0) {
            $('#aiAnomalyHeaderCount').show();
            $('#aiAnomalyHeaderText').text(summary.total + ' anomalies detected');
        } else {
            $('#aiAnomalyHeaderCount').hide();
        }

        // 이상 목록 및 연쇄 알림 렌더링
        renderAnomalyList(data.anomalies || [], data.cascade_alerts || []);
    }).fail(function () { renderAnomalyEmpty('Failed to load'); });
}

/**
 * 이상 감지 목록과 연쇄 알림 카드를 DOM에 렌더링
 * @param {Array} anomalies - 기계별 이상 감지 항목 배열
 * @param {Array} cascadeAlerts - 라인 단위 연쇄 알림 배열
 */
function renderAnomalyList(anomalies, cascadeAlerts) {
    // 목록 컨테이너 참조 및 초기화
    const $list = $('#aiAnomalyList').empty();

    // 연쇄 알림이 있으면 최상단에 표시
    cascadeAlerts.forEach(function (alert) {
        $list.append(
            '<div class="ai-anomaly-item ai-anomaly-item--critical" style="grid-template-columns:1fr;">' +
            '<div>' +
            '<div class="ai-anomaly-item__machine">Cascade Alert: ' + escHtml(alert.line_name) + '</div>' +
            '<div class="ai-anomaly-item__line">' + escHtml(alert.message) + '</div>' +
            '</div>' +
            '</div>'
        );
    });

    // 이상 감지 및 연쇄 알림 모두 없으면 빈 상태 메시지 표시
    if (anomalies.length === 0 && cascadeAlerts.length === 0) {
        $list.append(
            '<div class="ai-empty-state">' +
            '<div class="ai-empty-state__icon">&#10003;</div>' +
            '<div>No anomalies detected</div>' +
            '<div style="font-size:0.75rem;">All machines operating within normal range</div>' +
            '</div>'
        );
        return;
    }

    // 각 이상 감지 항목을 카드 형태로 렌더링
    anomalies.forEach(function (item) {
        // 심각도에 따른 CSS 클래스 결정
        const severityClass = 'ai-anomaly-item--' + (item.severity === 'critical' ? 'critical' : 'warning');
        // OEE 60% 미만이면 위험색, 이상이면 경고색
        const oeeColor = item.current_oee < 60 ? AI_COLORS.danger : AI_COLORS.warning;

        // 이상 상세 정보 HTML 생성 (측정값, 기준값, Z-score 포함)
        const detailsHtml = (item.details || []).map(function (d) {
            return '<div style="font-size:0.72rem;color:' + (d.severity === 'critical' ? AI_COLORS.danger : AI_COLORS.warning) + ';">' +
                escHtml(d.type) + ': ' + d.value + '% (avg ' + d.baseline + '%, Z=' + d.z_score + ')' +
                '</div>';
        }).join('');

        // 이상 감지 카드 HTML 삽입
        $list.append(
            '<div class="ai-anomaly-item ' + severityClass + '">' +
            '<div>' +
            '<div class="ai-anomaly-item__machine">' + escHtml(item.machine_no) + '</div>' +
            '<div class="ai-anomaly-item__line">' + escHtml(item.line_name) + '</div>' +
            detailsHtml +
            '</div>' +
            '<div></div>' +
            '<div class="ai-anomaly-item__oee" style="color:' + oeeColor + ';">' + item.current_oee + '%</div>' +
            '</div>'
        );
    });
}

/**
 * 이상 감지 목록에 오류/빈 상태 메시지 표시
 * @param {string} msg - 표시할 메시지 문자열
 */
function renderAnomalyEmpty(msg) {
    $('#aiAnomalyList').html(
        '<div class="ai-empty-state"><div class="ai-empty-state__icon">!</div><div>' + escHtml(msg) + '</div></div>'
    );
}

// ============================================================
// 3. 예방정비 위험도 — ai_maintenance_dash_2.php 호출
// ============================================================
/**
 * 예방정비 위험도 데이터를 API에서 로드하여 화면 갱신
 * - proc/ai_maintenance_dash_2.php 호출
 * - danger/warning 기계 수 집계 및 배지 업데이트
 * - 기계별 위험도 목록 카드 렌더링
 */
function loadMaintenanceRisk() {
    // 예방정비 위험도 API 호출 (필터 파라미터 포함)
    $.getJSON('proc/ai_maintenance_dash_2.php', getFilterParams(), function (data) {
        if (data.code !== '00') { renderMaintenanceEmpty('API error'); return; }

        // 요약 데이터에서 위험도 집계
        const summary = data.summary || {};
        // danger + warning 기계 수를 합산하여 총 고위험 수 표시
        const totalHighRisk = (summary.danger || 0) + (summary.warning || 0);
        $('#aiMaintDanger').text(totalHighRisk);
        $('#aiMaintSub').text((summary.danger || 0) + ' danger · ' + (summary.warning || 0) + ' caution');

        // 위험도 수준에 따라 경고 배지 스타일/텍스트 결정
        if (summary.danger > 0) {
            // DANGER 상태: 빨간 배지
            $('#aiMaintWarnBadge').show().text(summary.danger + ' DANGER')
                .removeClass('ai-status-badge--warning').addClass('ai-status-badge--danger');
        } else if (summary.warning > 0) {
            // CAUTION 상태: 노란 배지
            $('#aiMaintWarnBadge').show().text(summary.warning + ' CAUTION')
                .removeClass('ai-status-badge--danger').addClass('ai-status-badge--warning');
        } else {
            // 정상 상태: 배지 숨김
            $('#aiMaintWarnBadge').hide();
        }

        // 기계별 위험도 목록 렌더링
        renderMaintenanceList(data.machines || []);
    }).fail(function () { renderMaintenanceEmpty('Failed to load'); });
}

/**
 * 기계별 예방정비 위험도 카드 목록을 DOM에 렌더링
 * - 위험도 레벨(danger/warning/normal)별 색상 및 배지 적용
 * - 위험도 점수 바(progress bar) 시각화
 * - 상세 정보(평균 OEE, 다운타임 횟수, 마지막 다운타임 경과 시간) 표시
 * @param {Array} machines - 기계별 위험도 데이터 배열
 */
function renderMaintenanceList(machines) {
    const $list = $('#aiMaintenanceList').empty();

    // 데이터가 없으면 빈 상태 메시지 표시
    if (machines.length === 0) {
        $list.html(
            '<div class="ai-empty-state">' +
            '<div class="ai-empty-state__icon">&#10003;</div>' +
            '<div>No maintenance risk data</div>' +
            '</div>'
        );
        return;
    }

    // 각 기계별 위험도 카드 생성
    machines.forEach(function (m, idx) {
        // 위험도 레벨별 배지/바 CSS 클래스 및 레이블 매핑
        const levelMap = {
            danger: { badgeCls: 'ai-status-badge--danger', barCls: 'ai-risk-bar--danger', label: 'DANGER' },
            warning: { badgeCls: 'ai-status-badge--warning', barCls: 'ai-risk-bar--warning', label: 'CAUTION' },
            normal: { badgeCls: 'ai-status-badge--normal', barCls: 'ai-risk-bar--normal', label: 'NORMAL' },
        };
        const lv = levelMap[m.risk_level] || levelMap.normal;
        // 위험도 점수를 0~100% 범위로 제한하여 바 너비 계산
        const barWidth = Math.min(100, m.risk_score) + '%';

        // 상세 정보 HTML 구성 (있는 경우만)
        let detailHtml = '';
        if (m.details) {
            const d = m.details;
            const parts = [];
            if (d.avg_oee_7d !== undefined) parts.push('Avg OEE: ' + d.avg_oee_7d + '%');
            if (d.recent_30d_cnt !== undefined) parts.push('DT(30d): ' + d.recent_30d_cnt);
            if (d.hours_since_last_dt !== undefined) parts.push('Last DT: ' + d.hours_since_last_dt + 'H ago');
            detailHtml = '<div style="font-size:0.72rem;color:' + AI_COLORS.chartText + ';margin-top:2px;">' + parts.join(' | ') + '</div>';
        }

        // 기계 위험도 카드 HTML 삽입 (순위, 기계번호, 라인, 배지, 위험도 바 포함)
        $list.append(
            '<div class="ai-maintenance-item">' +
            '<div class="ai-maintenance-item__header">' +
            '<div>' +
            '<span style="font-size:0.7rem;color:' + AI_COLORS.chartText + ';">#' + (idx + 1) + ' </span>' +
            '<span class="ai-maintenance-item__machine">' + escHtml(m.machine_no) + '</span>' +
            '<span class="ai-maintenance-item__line" style="margin-left:6px;">(' + escHtml(m.line_name) + ')</span>' +
            '</div>' +
            '<span class="ai-status-badge ' + lv.badgeCls + '">' + lv.label + ' ' + m.risk_score + '</span>' +
            '</div>' +
            detailHtml +
            '<div class="ai-risk-bar-wrap" style="margin-top:6px;">' +
            '<div class="ai-risk-bar ' + lv.barCls + '" style="width:' + barWidth + ';"></div>' +
            '</div>' +
            '</div>'
        );
    });
}

/**
 * 예방정비 목록에 오류/빈 상태 메시지 표시
 * @param {string} msg - 표시할 메시지 문자열
 */
function renderMaintenanceEmpty(msg) {
    $('#aiMaintenanceList').html(
        '<div class="ai-empty-state"><div class="ai-empty-state__icon">!</div><div>' + escHtml(msg) + '</div></div>'
    );
}

// ============================================================
// 4. 라인 건강지수 — ai_maintenance_dash_2.php 호출
// ============================================================
/**
 * 라인별 건강지수(Health Index)를 계산하고 화면에 표시
 * - proc/ai_maintenance_dash_2.php 에서 기계 데이터 조회 (최대 50개)
 * - 기계별 avg_oee_7d 평균을 라인 건강지수로 활용
 *   (avg_oee_7d가 없으면 risk_score 기반으로 역산)
 * - 건강지수 >= 80: Normal(초록), >= 60: Caution(노랑), < 60: Danger(빨강)
 * - 전체 평균 건강지수 및 모니터링 라인 수 업데이트
 */
function loadLineHealth() {
    // 라인 건강지수 API 호출 (limit=50으로 충분한 기계 데이터 수집)
    $.getJSON('proc/ai_maintenance_dash_2.php', Object.assign({}, getFilterParams(), { limit: 50 }), function (data) {
        if (data.code !== '00') return;

        // 라인별 OEE 및 위험도 점수 집계 맵 구성
        const lineMap = {};
        (data.machines || []).forEach(function (m) {
            const line = m.line_name;
            if (!lineMap[line]) lineMap[line] = { oees: [], risks: [] };
            // avg_oee_7d가 있으면 OEE 배열에 추가
            if (m.details && m.details.avg_oee_7d !== undefined) {
                lineMap[line].oees.push(m.details.avg_oee_7d);
            }
            // 위험도 점수 항상 추가 (OEE 없을 때 역산용)
            lineMap[line].risks.push(m.risk_score);
        });

        // 라인명 알파벳 정렬
        const lines = Object.keys(lineMap).sort();
        if (lines.length === 0) {
            // 라인 데이터 없음
            $('#aiHealthList').html('<div class="ai-empty-state"><div>No line data available</div></div>');
            $('#aiHealthAvg').text('--');
            return;
        }

        // 전체 건강지수 합산용 변수
        let totalHealth = 0;
        let healthCount = 0;

        const $list = $('#aiHealthList').empty();
        // 각 라인별 건강지수 계산 및 렌더링
        lines.forEach(function (line) {
            const entry = lineMap[line];
            let health = 0;
            if (entry.oees.length > 0) {
                // avg_oee_7d 평균을 건강지수로 사용
                health = entry.oees.reduce((a, b) => a + b, 0) / entry.oees.length;
            } else {
                // OEE 데이터 없으면 위험도 점수 역산 (100 - 평균 risk_score)
                const avgRisk = entry.risks.reduce((a, b) => a + b, 0) / entry.risks.length;
                health = Math.max(0, 100 - avgRisk);
            }
            // 소수점 1자리로 반올림
            health = Math.round(health * 10) / 10;
            totalHealth += health;
            healthCount++;

            // 건강지수에 따른 색상/배지 결정
            let barColor, badgeCls, badgeLabel;
            if (health >= 80) { barColor = AI_COLORS.normal; badgeCls = 'ai-status-badge--normal'; badgeLabel = 'Normal'; }
            else if (health >= 60) { barColor = AI_COLORS.warning; badgeCls = 'ai-status-badge--warning'; badgeLabel = 'Caution'; }
            else { barColor = AI_COLORS.danger; badgeCls = 'ai-status-badge--danger'; badgeLabel = 'Danger'; }

            // 라인 건강지수 아이템 HTML 삽입 (라인명, 건강 바, 퍼센트, 배지)
            $list.append(
                '<div class="ai-health-item">' +
                '<div class="ai-health-item__line" title="' + escHtml(line) + '">' + escHtml(line) + '</div>' +
                '<div class="ai-health-bar-wrap">' +
                '<div class="ai-health-bar" style="width:' + health + '%;background:' + barColor + ';"></div>' +
                '</div>' +
                '<div class="ai-health-item__pct">' + health + '%</div>' +
                '<span class="ai-status-badge ' + badgeCls + '">' + badgeLabel + '</span>' +
                '</div>'
            );
        });

        // 전체 평균 건강지수 계산 및 표시
        const avgHealth = healthCount > 0 ? (totalHealth / healthCount).toFixed(1) : '--';
        $('#aiHealthAvg').text(avgHealth !== '--' ? avgHealth + '%' : '--');
        $('#aiHealthSub').text(lines.length + ' lines monitored');

        // 평균 건강지수에 따른 전체 상태 배지 업데이트
        const avgVal = parseFloat(avgHealth);
        if (!isNaN(avgVal)) {
            let cls, label;
            if (avgVal >= 80) { cls = 'ai-status-badge--normal'; label = 'HEALTHY'; }
            else if (avgVal >= 60) { cls = 'ai-status-badge--warning'; label = 'CAUTION'; }
            else { cls = 'ai-status-badge--danger'; label = 'AT RISK'; }
            // 전체 상태 배지 표시 및 스타일 적용
            $('#aiHealthStatusBadge').show()
                .removeClass('ai-status-badge--normal ai-status-badge--warning ai-status-badge--danger')
                .addClass(cls).text(label);
        }
    });
}

// ============================================================
// 유틸리티
// ============================================================
/**
 * HTML 특수문자 이스케이프 처리 (XSS 방지)
 * - &, <, >, " 문자를 HTML 엔티티로 변환
 * @param {*} str - 이스케이프할 문자열 (null/undefined 허용)
 * @returns {string} 이스케이프된 안전한 HTML 문자열
 */
function escHtml(str) {
    if (!str) return '';
    return String(str)
        .replace(/&/g, '&amp;')
        .replace(/</g, '&lt;')
        .replace(/>/g, '&gt;')
        .replace(/"/g, '&quot;');
}

// ============================================================
// 초기화
// ============================================================
/**
 * DOM 로드 완료 후 대시보드 초기화
 * - 필터 시스템 초기화 (공장/라인/기계 드롭다운)
 * - 전체 데이터 최초 로드
 * - REFRESH_INTERVAL(60초) 마다 자동 갱신 타이머 설정
 */
$(document).ready(function () {
    // 필터 시스템 초기화 (드롭다운 생성 및 이벤트 바인딩)
    initFilterSystem();
    // 최초 데이터 로드
    refreshAll();

    // 60초마다 자동 갱신 타이머 설정 (타이머 핸들을 aiState에 보관)
    aiState.refreshTimer = setInterval(function () {
        refreshAll();
    }, REFRESH_INTERVAL);
});
