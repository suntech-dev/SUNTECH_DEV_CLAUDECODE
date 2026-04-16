/**
 * ============================================================
 * 파일명: ai_dashboard_2_ina.js
 * 목적: AI Intelligence Dashboard 메인 컨트롤러 (인도네시아어 버전)
 *       ai_dashboard_2.js 기반, UI 텍스트를 인도네시아어로 교체
 * ============================================================
 */

/* ── 차트 색상 팔레트 상수 ──────────────────────────────────────
 * 다크 테마 기반 색상 정의 (primary: SAP 브랜드 블루, forecast: 주황, 등)
 */
const AI_COLORS = {
    primary: '#0070f2',
    forecast: '#f5a623',
    ci: 'rgba(245,166,35,0.15)',
    danger: '#f85149',
    warning: '#d29922',
    normal: '#3fb950',
    info: '#58a6ff',
    chartGrid: 'rgba(255,255,255,0.08)',
    chartText: '#8b949e',
};

/* ── Actual OEE 상시 라벨 플러그인 ────────────────────────────────
 * Actual OEE solid 라인의 각 데이터 포인트 위에 % 값을 항상 표시
 * - datasets[0] 이 'Actual OEE' 인 경우에만 동작
 * - null 포인트는 건너뜀 (예측 구간과 혼재 방지)
 */
const actualDataLabelsPlugin = {
    id: 'actualDataLabels',
    afterDatasetsDraw(chart) {
        if (!chart.data.datasets.length) return;
        if (chart.data.datasets[0].label !== 'OEE Aktual') return;
        const meta = chart.getDatasetMeta(0);
        const ctx = chart.ctx;
        const chartArea = chart.chartArea;
        ctx.save();
        ctx.font = 'bold 11px Arial, sans-serif';
        ctx.textAlign = 'center';
        meta.data.forEach(function (point, index) {
            const value = chart.data.datasets[0].data[index];
            if (value === null || value === undefined) return;
            const label = value + '%';
            const tw = ctx.measureText(label).width;
            const bw = tw + 8;   // 배경 박스 너비
            const bh = 16;       // 배경 박스 높이
            const bx = point.x - bw / 2;
            // 포인트가 차트 상단 근처면 라벨을 아래에, 아니면 위에 표시
            const above = point.y - chartArea.top > bh + 6;
            const by = above ? point.y - bh - 6 : point.y + 6;
            // 반투명 배경 박스 (어두운 네이비)
            ctx.shadowColor = 'transparent';
            ctx.fillStyle = 'rgba(10, 25, 55, 0.82)';
            ctx.beginPath();
            ctx.rect(bx, by, bw, bh);
            ctx.fill();
            // 테두리 (primary 컬러, 연하게)
            ctx.strokeStyle = 'rgba(0, 112, 242, 0.6)';
            ctx.lineWidth = 1;
            ctx.stroke();
            // 텍스트
            ctx.fillStyle = '#e6edf3';   // 흰색 계열로 대비 강화
            ctx.textBaseline = 'middle';
            ctx.fillText(label, point.x, by + bh / 2);
        });
        ctx.restore();
    }
};

/* 자동 새로고침 주기 (밀리초): 60초마다 전체 데이터 갱신 */
const REFRESH_INTERVAL = 60000;

/* 전역 상태 객체: 현재 선택된 필터값 및 차트 인스턴스 보관 */
let aiState = {
    factory: '',
    line: '',
    machine: '',
    forecastChart: null,
    refreshTimer: null,
};

// ============================================================
// Sistem Filter
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
        aiState.factory = $(this).val();
        // 하위 필터(라인, 기계) 초기화
        aiState.line = '';
        aiState.machine = '';
        // 라인/기계 드롭다운 초기화 및 비활성화
        $('#factoryLineFilterSelect').html('<option value="">Semua Lini</option>').prop('disabled', true);
        $('#factoryLineMachineFilterSelect').html('<option value="">Semua Mesin</option>').prop('disabled', true);

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
                $('#factoryLineMachineFilterSelect').html('<option value="">Semua Mesin</option>');
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
        $('#factoryLineMachineFilterSelect').html('<option value="">Semua Mesin</option>').prop('disabled', true);
        
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
// Segarkan Semua
// ============================================================
/**
 * 모든 AI 대시보드 패널 데이터를 일괄 갱신
 * - OEE 예측, 이상 감지, 예방정비 위험도, 마지막 업데이트 시간, 라인 건강지수 서브타이틀 갱신
 */
function refreshAll() {
    loadPrediction();
    loadAnomalyDetection();
    loadMaintenanceRisk();
    updateLastUpdateTime();
    updateLineHealthSubtitle();
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
    $('#aiLastUpdateTime').text('Diperbarui: ' + hh + ':' + mm + ':' + ss);
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
        today: 'rata-rata OEE 7 hari per lini',
        yesterday: 'rata-rata OEE 7 hari per lini',
        '7d': 'rata-rata OEE 7 hari per lini',
        '30d': 'rata-rata OEE 30 hari per lini',
    };
    // 현재 선택값에 맞는 레이블 결정 (없으면 기본값 사용)
    const label = sel ? (rangeMap[sel.value] || 'rata-rata OEE 7 hari per lini') : 'rata-rata OEE 7 hari per lini';
    // ai-health-subtitle 요소에 텍스트 설정
    const el = document.querySelector('.ai-health-subtitle');
    if (el) el.textContent = 'Berdasarkan ' + label;
}

// ============================================================
// 1. Prediksi OEE
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
            $('#aiPredSub').text('Rata-rata 4 Jam ke Depan: ' + forecastAvg + '%');
        } else {
            // 예측 데이터 없음
            $('#aiPredSub').text('Data tidak mencukupi');
        }

        // 트렌드 배지 클래스/텍스트 매핑 정의
        const trendMap = {
            up: { cls: 'ai-trend-badge--up', text: 'Tren Naik' },
            down: { cls: 'ai-trend-badge--down', text: 'Tren Turun' },
            stable: { cls: 'ai-trend-badge--stable', text: 'Stabil' },
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
        $('#aiPredSub').text('Kesalahan API');
    });
}

// ============================================================
// 1-1. Grafik Prediksi OEE
// ============================================================
function renderForecastChart(predData) {
    const canvas = document.getElementById('aiOeeForecastChart');
    if (!canvas) return;

    const todayArr = predData.today_data || [];
    const actualLabels = todayArr.map(function (d) { return d.label; });
    const actualValues = todayArr.map(function (d) { return d.oee; });

    const forecastArr = predData.forecast || [];
    const forecastLabels = forecastArr.map(function (f) { return f.label; });
    const forecastValues = forecastArr.map(function (f) { return f.oee; });
    const ciUpper = forecastArr.map(function (f) { return f.upper; });
    const ciLower = forecastArr.map(function (f) { return f.lower; });

    // 연결점: 마지막 실제값 → 예측 첫 점 이음
    const connectOee = predData.current_oee || null;
    // 연결 라벨: current_hour가 있으면 "HH:00" 형식, 없으면 "Now"
    const connectLabel = predData.current_hour !== undefined
        ? String(predData.current_hour).padStart(2, '0') + ':00' : 'Sekarang';

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
            label: 'OEE Aktual',
            data: actualFull,
            borderColor: AI_COLORS.primary,
            backgroundColor: 'transparent',
            borderWidth: 2.5,
            borderDash: [],
            pointBackgroundColor: AI_COLORS.primary,
            pointRadius: 3,
            tension: 0.3,
            order: 0,
            spanGaps: false,
        });
    }

    // AI Forecast dashed 라인 데이터셋
    datasets.push({
        label: 'Prediksi AI',
        data: forecastFull,
        borderColor: AI_COLORS.forecast,
        backgroundColor: 'transparent',
        borderDash: [6, 4],
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

    datasets.push({
        label: 'CI Atas',
        data: ciUpperFull,
        borderColor: 'transparent',
        backgroundColor: AI_COLORS.ci,
        fill: '+1',
        pointRadius: 0,
        tension: 0.3,
        order: 2,
        spanGaps: false,
    });

    datasets.push({
        label: 'CI Bawah',
        data: ciLowerFull,
        borderColor: 'transparent',
        backgroundColor: AI_COLORS.ci,
        fill: false,
        pointRadius: 0,
        tension: 0.3,
        order: 3,
        spanGaps: false,
    });

    const chartData = { labels: allLabels, datasets: datasets };

    const options = {
        responsive: true,
        maintainAspectRatio: false,
        interaction: { mode: 'index', intersect: false },
        plugins: {
            legend: { display: false },
            tooltip: {
                backgroundColor: '#1c2128',
                borderColor: '#30363d',
                borderWidth: 1,
                titleColor: '#e6edf3',
                bodyColor: '#8b949e',
                callbacks: {
                    label: function (ctx) {
                        if (ctx.raw === null || ctx.raw === undefined) return null;
                        if (ctx.dataset.label === 'CI Atas') return 'CI Atas: ' + ctx.parsed.y + '%';
                        if (ctx.dataset.label === 'CI Bawah') return 'CI Bawah: ' + ctx.parsed.y + '%';
                        return ctx.dataset.label + ': ' + ctx.parsed.y + '%';
                    }
                }
            }
        },
        scales: {
            x: {
                grid: { color: AI_COLORS.chartGrid },
                ticks: { color: AI_COLORS.chartText, font: { size: 11 } },
            },
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
        aiState.forecastChart.update('none');
    } else {
        // 차트 최초 생성 (actualDataLabelsPlugin: Actual OEE 포인트 위 % 상시 표시)
        aiState.forecastChart = new Chart(canvas.getContext('2d'), {
            type: 'line',
            data: chartData,
            options: options,
            plugins: [actualDataLabelsPlugin],
        });
    }
}

// ============================================================
// 2. Deteksi Anomali
// ============================================================
function loadAnomalyDetection() {
    $.getJSON('proc/ai_anomaly_2.php', getFilterParams(), function (data) {
        if (data.code !== '00') { renderAnomalyEmpty('Kesalahan API'); return; }

        const summary = data.summary || {};
        $('#aiAnomalyTotal').text(summary.total || 0);

        if (summary.critical > 0) {
            $('#aiAnomalySub').text(summary.critical + ' kritis · ' + summary.warning + ' peringatan');
            $('#aiAnomalyCriticalBadge').show().text(summary.critical + ' KRITIS')
                .removeClass('ai-status-badge--warning ai-status-badge--normal').addClass('ai-status-badge--danger');
        } else if (summary.warning > 0) {
            $('#aiAnomalySub').text(summary.warning + ' peringatan terdeteksi');
            $('#aiAnomalyCriticalBadge').show().text(summary.warning + ' PERINGATAN')
                .removeClass('ai-status-badge--danger ai-status-badge--normal').addClass('ai-status-badge--warning');
        } else {
            $('#aiAnomalySub').text('Semua mesin normal');
            $('#aiAnomalyCriticalBadge').hide();
        }

        if (summary.total > 0) {
            $('#aiAnomalyHeaderCount').show();
            $('#aiAnomalyHeaderText').text(summary.total + ' anomali terdeteksi');
        } else {
            $('#aiAnomalyHeaderCount').hide();
        }

        renderAnomalyList(data.anomalies || [], data.cascade_alerts || []);
    }).fail(function () { renderAnomalyEmpty('Gagal memuat'); });
}

function renderAnomalyList(anomalies, cascadeAlerts) {
    const $list = $('#aiAnomalyList').empty();

    cascadeAlerts.forEach(function (alert) {
        $list.append(
            '<div class="ai-anomaly-item ai-anomaly-item--critical" style="grid-template-columns:1fr;">' +
            '<div>' +
            '<div class="ai-anomaly-item__machine">Peringatan Cascade: ' + escHtml(alert.line_name) + '</div>' +
            '<div class="ai-anomaly-item__line">' + escHtml(alert.message) + '</div>' +
            '</div>' +
            '</div>'
        );
    });

    if (anomalies.length === 0 && cascadeAlerts.length === 0) {
        $list.append(
            '<div class="ai-empty-state">' +
            '<div class="ai-empty-state__icon">&#10003;</div>' +
            '<div>Tidak ada anomali terdeteksi</div>' +
            '<div style="font-size:0.75rem;">Semua mesin beroperasi dalam rentang normal</div>' +
            '</div>'
        );
        return;
    }

    anomalies.forEach(function (item) {
        const severityClass = 'ai-anomaly-item--' + (item.severity === 'critical' ? 'critical' : 'warning');
        const oeeColor = item.current_oee < 60 ? AI_COLORS.danger : AI_COLORS.warning;

        const detailsHtml = (item.details || []).map(function (d) {
            return '<div style="font-size:0.72rem;color:' + (d.severity === 'critical' ? AI_COLORS.danger : AI_COLORS.warning) + ';">' +
                escHtml(d.type) + ': ' + d.value + '% (rata-rata ' + d.baseline + '%, Z=' + d.z_score + ')' +
                '</div>';
        }).join('');

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

function renderAnomalyEmpty(msg) {
    $('#aiAnomalyList').html(
        '<div class="ai-empty-state"><div class="ai-empty-state__icon">!</div><div>' + escHtml(msg) + '</div></div>'
    );
}

// ============================================================
// 3. Risiko Perawatan Prediktif
// ============================================================
function loadMaintenanceRisk() {
    $.getJSON('proc/ai_maintenance_dash_2.php', getFilterParams(), function (data) {
        if (data.code !== '00') { renderMaintenanceEmpty('Kesalahan API'); return; }

        const summary = data.summary || {};
        const totalHighRisk = (summary.danger || 0) + (summary.warning || 0);
        $('#aiMaintDanger').text(totalHighRisk);
        $('#aiMaintSub').text((summary.danger || 0) + ' bahaya · ' + (summary.warning || 0) + ' waspada');

        if (summary.danger > 0) {
            $('#aiMaintWarnBadge').show().text(summary.danger + ' BAHAYA')
                .removeClass('ai-status-badge--warning').addClass('ai-status-badge--danger');
        } else if (summary.warning > 0) {
            $('#aiMaintWarnBadge').show().text(summary.warning + ' WASPADA')
                .removeClass('ai-status-badge--danger').addClass('ai-status-badge--warning');
        } else {
            $('#aiMaintWarnBadge').hide();
        }

        renderMaintenanceList(data.machines || []);
    }).fail(function () { renderMaintenanceEmpty('Gagal memuat'); });
}

function renderMaintenanceList(machines) {
    const $list = $('#aiMaintenanceList').empty();

    if (machines.length === 0) {
        $list.html(
            '<div class="ai-empty-state">' +
            '<div class="ai-empty-state__icon">&#10003;</div>' +
            '<div>Tidak ada data risiko perawatan</div>' +
            '</div>'
        );
        return;
    }

    machines.forEach(function (m, idx) {
        const levelMap = {
            danger: { badgeCls: 'ai-status-badge--danger', barCls: 'ai-risk-bar--danger', label: 'BAHAYA' },
            warning: { badgeCls: 'ai-status-badge--warning', barCls: 'ai-risk-bar--warning', label: 'WASPADA' },
            normal: { badgeCls: 'ai-status-badge--normal', barCls: 'ai-risk-bar--normal', label: 'NORMAL' },
        };
        const lv = levelMap[m.risk_level] || levelMap.normal;
        const barWidth = Math.min(100, m.risk_score) + '%';

        let detailHtml = '';
        if (m.details) {
            const d = m.details;
            const parts = [];
            if (d.avg_oee_7d !== undefined) parts.push('Rata-rata OEE: ' + d.avg_oee_7d + '%');
            if (d.recent_30d_cnt !== undefined) parts.push('DT(30h): ' + d.recent_30d_cnt);
            if (d.hours_since_last_dt !== undefined) parts.push('DT Terakhir: ' + d.hours_since_last_dt + 'J lalu');
            detailHtml = '<div style="font-size:0.72rem;color:' + AI_COLORS.chartText + ';margin-top:2px;">' + parts.join(' | ') + '</div>';
        }

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

function renderMaintenanceEmpty(msg) {
    $('#aiMaintenanceList').html(
        '<div class="ai-empty-state"><div class="ai-empty-state__icon">!</div><div>' + escHtml(msg) + '</div></div>'
    );
}

// ============================================================
// 4. Indeks Kesehatan Lini
// ============================================================
function loadLineHealth() {
    $.getJSON('proc/ai_maintenance_dash_2.php', Object.assign({}, getFilterParams(), { limit: 50 }), function (data) {
        if (data.code !== '00') return;

        const lineMap = {};
        (data.machines || []).forEach(function (m) {
            const line = m.line_name;
            if (!lineMap[line]) lineMap[line] = { oees: [], risks: [] };
            if (m.details && m.details.avg_oee_7d !== undefined) {
                lineMap[line].oees.push(m.details.avg_oee_7d);
            }
            lineMap[line].risks.push(m.risk_score);
        });

        const lines = Object.keys(lineMap).sort((a, b) =>
            a.localeCompare(b, undefined, { numeric: true, sensitivity: 'base' })
        );
        if (lines.length === 0) {
            $('#aiHealthList').html('<div class="ai-empty-state"><div>Tidak ada data lini</div></div>');
            $('#aiHealthAvg').text('--');
            return;
        }

        let totalHealth = 0;
        let healthCount = 0;

        const $list = $('#aiHealthList').empty();
        lines.forEach(function (line) {
            const entry = lineMap[line];
            let health = 0;
            if (entry.oees.length > 0) {
                health = entry.oees.reduce((a, b) => a + b, 0) / entry.oees.length;
            } else {
                const avgRisk = entry.risks.reduce((a, b) => a + b, 0) / entry.risks.length;
                health = Math.max(0, 100 - avgRisk);
            }
            health = Math.round(health * 10) / 10;
            totalHealth += health;
            healthCount++;

            let barColor, badgeCls, badgeLabel;
            if (health >= 80) { barColor = AI_COLORS.normal; badgeCls = 'ai-status-badge--normal'; badgeLabel = 'Normal'; }
            else if (health >= 60) { barColor = AI_COLORS.warning; badgeCls = 'ai-status-badge--warning'; badgeLabel = 'Waspada'; }
            else { barColor = AI_COLORS.danger; badgeCls = 'ai-status-badge--danger'; badgeLabel = 'Bahaya'; }

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

        const avgHealth = healthCount > 0 ? (totalHealth / healthCount).toFixed(1) : '--';
        $('#aiHealthAvg').text(avgHealth !== '--' ? avgHealth + '%' : '--');
        $('#aiHealthSub').text(lines.length + ' lini dipantau');

        const avgVal = parseFloat(avgHealth);
        if (!isNaN(avgVal)) {
            let cls, label;
            if (avgVal >= 80) { cls = 'ai-status-badge--normal'; label = 'SEHAT'; }
            else if (avgVal >= 60) { cls = 'ai-status-badge--warning'; label = 'WASPADA'; }
            else { cls = 'ai-status-badge--danger'; label = 'BERISIKO'; }
            $('#aiHealthStatusBadge').show()
                .removeClass('ai-status-badge--normal ai-status-badge--warning ai-status-badge--danger')
                .addClass(cls).text(label);
        }
    });
}

// ============================================================
// Utilitas
// ============================================================
function escHtml(str) {
    if (!str) return '';
    return String(str)
        .replace(/&/g, '&amp;')
        .replace(/</g, '&lt;')
        .replace(/>/g, '&gt;')
        .replace(/"/g, '&quot;');
}

// ============================================================
// Inisialisasi
// ============================================================
$(document).ready(function () {
    initFilterSystem();
    refreshAll();

    aiState.refreshTimer = setInterval(function () {
        refreshAll();
    }, REFRESH_INTERVAL);
});
