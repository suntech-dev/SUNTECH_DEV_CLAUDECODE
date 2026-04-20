<?php

/*
 * ============================================================
 * 파일명  : ai_dashboard_2.php
 * 목  적  : 1920×1080 사이니지(Signage) 전용 AI 인텔리전스 대시보드
 *           AI 기반 OEE 예측, 이상 감지(Anomaly Detection), 예측 정비,
 *           생산 최적화(Production Optimization) 정보를 한 화면에 표시한다.
 * 개선 이력:
 *   - ai_dashboard_4.php 개선본
 *   - [P1] Real-time OEE 카드: current_oee 항상 오늘 기준, min/max 클램핑 적용
 *   - [P1] OEE Forecast 차트: Actual OEE solid 라인 추가 (today_data 활용)
 *   - [P2] CI 범위 0~100% 방지: seasonal_std 상한 15% 적용
 *   - [P2] Predictive Maintenance: OEE 100% 초과 값 SQL 클램핑으로 오판정 수정
 *   - [P3] Production Optimization: date_range 파라미터 연동
 *   - [P3] Line Health Index: 서브타이틀을 date_range에 맞게 동적 표시
 * 연관 파일:
 *   - css/ai_dashboard_2.css      : 사이니지 AI 대시보드 스타일
 *   - js/ai_dashboard_2.js        : AI 차트·카드 렌더링 로직
 *   - js/ai_optimization_2.js     : 생산 최적화 분석 렌더링 로직
 *   - proc/ai_oee_prediction_dash_2.php : OEE 예측 API 엔드포인트
 *   - proc/ai_maintenance_dash_2.php    : 예측 정비 API 엔드포인트
 *   - proc/ai_optimization_2.php        : 생산 최적화 API 엔드포인트
 * ============================================================
 */

/**
 * ai_dashboard_2.php — 1920x1080 Signage AI Dashboard
 * ai_dashboard_4.php 개선본
 *
 * 버그 수정:
 *  [P1] Real-time OEE 카드: current_oee 항상 오늘 기준, min/max 클램핑 적용
 *  [P1] OEE Forecast 차트: Actual OEE solid 라인 추가 (today_data 활용)
 *  [P2] CI 범위 0~100% 방지: seasonal_std 상한 15% 적용
 *  [P2] Predictive Maintenance: OEE 100% 초과 값 SQL 클램핑으로 오판정 수정
 *  [P3] Production Optimization: date_range 파라미터 연동
 *  [P3] Line Health Index: 서브타이틀을 date_range에 맞게 동적 표시
 *
 * 연관 파일:
 *  - css/ai_dashboard_2.css
 *  - js/ai_dashboard_2.js
 *  - js/ai_optimization_2.js
 *  - proc/ai_oee_prediction_dash_2.php
 *  - proc/ai_maintenance_dash_2.php
 *  - proc/ai_optimization_2.php
 */

// 브라우저 탭 및 페이지 제목 설정
$page_title = 'AI Intelligence Dashboard - Signage';

// 페이지에서 사용할 CSS 파일 목록 (fiori 공통 + 대시보드 전용 스타일)
$page_css_files = [
    '../../assets/css/fiori-page.css',  // Fiori 공통 페이지 스타일
    'css/dashboard.css',                 // 대시보드 기본 스타일
    'css/ai_dashboard.css',              // AI 대시보드 공통 스타일
    'css/ai_dashboard_2.css',            // AI 대시보드 사이니지 전용 스타일
];

// 공통 헤드 파일 포함 (CSS 로드, meta 태그 등)
require_once(__DIR__ . '/../../inc/head.php');
/* nav-fiori.php 제거 */
?>

<?php
// 네비게이션 컨텍스트 및 활성 메뉴 설정
// - nav_context: 어느 메뉴 그룹인지 구분 (data/manage)
// - nav_active : 현재 활성화된 메뉴 항목 식별자
$nav_context = 'data';
$nav_active = 'ai_dashboard';
// 사이드 드로어(네비게이션 메뉴) 포함
require_once(__DIR__ . '/../../inc/nav-drawer-manage.php'); ?>

<!-- Signage Header -->
<!-- 사이니지 상단 헤더: 메뉴 버튼, 제목, 필터 셀렉트, 상태 표시, 실시간 시계 포함 -->
<div class="signage-header">
    <!-- 좌측 햄버거 메뉴 버튼 (네비게이션 드로어 토글) -->
    <button id="navDrawerBtn" class="nav-drawer-btn" aria-label="Menu">&#9776;</button>
    <!-- 페이지 타이틀 및 AI 뱃지 -->
    <span class="signage-header__title">
        AI Intelligence Dashboard
        <span class="ai-badge">AI POWERED</span>
    </span>

    <!-- 필터 영역: 공장·라인·기계 셀렉트 및 날짜 범위 선택 -->
    <div class="signage-header__filters">
        <!-- 공장 선택 드롭다운 (JS에서 동적으로 옵션 채움) -->
        <select id="factoryFilterSelect" class="fiori-select">
            <option value="">All Factory</option>
        </select>
        <!-- 라인 선택 드롭다운 (공장 선택 후 활성화) -->
        <select id="factoryLineFilterSelect" class="fiori-select" disabled>
            <option value="">All Line</option>
        </select>
        <!-- 기계 선택 드롭다운 (라인 선택 후 활성화) -->
        <select id="factoryLineMachineFilterSelect" class="fiori-select" disabled>
            <option value="">All Machine</option>
        </select>
        <!-- 날짜 범위 선택: today / yesterday / 7일 / 30일 -->
        <select id="dateRangeSelect" class="fiori-select date-range-select">
            <option value="today" selected>Today</option>
            <option value="yesterday">Yesterday</option>
            <option value="7d">Last 7 Days</option>
            <option value="30d">Last 30 Days</option>
        </select>
        <!-- 수동 새로고침 버튼 -->
        <button id="aiRefreshBtn" class="fiori-btn fiori-btn--tertiary">Refresh</button>
        <!-- 리포트 내보내기 버튼 (클릭 시 exportModal 표시) -->
        <button id="exportReportBtn" class="fiori-btn fiori-btn--ghost">Export</button>
        <!-- 도움말 페이지 링크 (새 탭 열기) -->
        <a href="ai_dashboard_manual.html" target="_blank" class="fiori-btn fiori-btn--ghost" style="text-decoration:none;">Help</a>
    </div>

    <!-- 마지막 업데이트 시간 및 AI 활성 상태 표시 -->
    <div class="signage-header__status">
        <div class="ai-pulse-dot"></div>
        <span id="aiLastUpdateTime">Initializing...</span>
    </div>

    <!-- 우측 실시간 시계 (HH:MM:SS 형식, JS로 갱신) -->
    <div class="signage-header__clock" id="signageClock"></div>
</div>

<!-- AI Signage Main: 4행 CSS Grid -->
<!-- 전체 대시보드 레이아웃을 4개 행(Row A~D)으로 구성 -->
<div class="ai-signage-main">

    <!-- Row A: AI Summary 카드 5개 -->
    <!-- 상단 요약 카드 영역: 실시간 OEE, 4시간 예측, 이상 감지, 고위험 기계, 라인 건강도 -->
    <div class="ai-signage-row-a">
        <div class="ai-summary-grid">

            <!-- Real-time OEE -->
            <!-- 카드 1: 현재 시간 기준 실시간 OEE 값 표시 (항상 오늘 날짜 기준) -->
            <div class="ai-summary-card ai-summary-card--realtime">
                <div>
                    <!-- LIVE 표시 도트 애니메이션 -->
                    <div class="ai-realtime-live">
                        <div class="ai-realtime-live__dot"></div>
                        LIVE
                    </div>
                    <span class="ai-summary-card__label" style="margin-top:2px;">Real-time OEE</span>
                </div>
                <!-- OEE 수치 표시 영역 (JS에서 동적으로 업데이트) -->
                <div class="ai-summary-card__value" id="aiRealtimeOee">--</div>
                <!-- 현재 시간대 서브 텍스트 + OEE 상태 뱃지 (같은 줄) -->
                <div class="ai-summary-card__sub-row">
                    <div class="ai-summary-card__sub" id="aiRealtimeSub"><span class="ai-spinner"></span></div>
                    <span id="aiRealtimeBadge" class="ai-status-badge" style="display:none;"></span>
                </div>
            </div>

            <!-- Next 4H AI Forecast -->
            <!-- 카드 2: 향후 4시간 AI OEE 예측값 및 신뢰 구간(CI) 표시 -->
            <div class="ai-summary-card ai-summary-card--prediction">
                <span class="ai-summary-card__label">Next 4H AI Forecast</span>
                <!-- 향후 4시간 평균 예측 OEE 값 -->
                <div class="ai-summary-card__value" id="aiPredForecastOee">--</div>
                <!-- CI 범위 서브 텍스트 + 추세 뱃지 (같은 줄) -->
                <div class="ai-summary-card__sub-row">
                    <div class="ai-summary-card__sub" id="aiPredSub"><span class="ai-spinner"></span></div>
                    <span class="ai-trend-badge ai-trend-badge--stable" id="aiPredTrendBadge">--</span>
                </div>
            </div>

            <!-- Anomaly Detection -->
            <!-- 카드 3: Z-Score 기반 실시간 이상 감지 건수 표시 -->
            <div class="ai-summary-card ai-summary-card--anomaly">
                <span class="ai-summary-card__label">Anomaly Detection</span>
                <!-- 감지된 전체 이상 건수 -->
                <div class="ai-summary-card__value" id="aiAnomalyTotal">--</div>
                <!-- 이상 감지 요약 서브 텍스트 + Critical 뱃지 (같은 줄) -->
                <div class="ai-summary-card__sub-row">
                    <div class="ai-summary-card__sub" id="aiAnomalySub"><span class="ai-spinner"></span></div>
                    <span id="aiAnomalyCriticalBadge" class="ai-status-badge" style="display:none;"></span>
                </div>
            </div>

            <!-- High-Risk Machines -->
            <!-- 카드 4: 예측 정비 위험 점수 기반 고위험 기계 수 표시 -->
            <div class="ai-summary-card ai-summary-card--maintenance">
                <span class="ai-summary-card__label">High-Risk Machines</span>
                <!-- 위험 등급 기계 수 -->
                <div class="ai-summary-card__value" id="aiMaintDanger">--</div>
                <!-- 경고/위험 기계 요약 서브 텍스트 + Warning 뱃지 (같은 줄) -->
                <div class="ai-summary-card__sub-row">
                    <div class="ai-summary-card__sub" id="aiMaintSub"><span class="ai-spinner"></span></div>
                    <span id="aiMaintWarnBadge" class="ai-status-badge ai-status-badge--warning" style="display:none;"></span>
                </div>
            </div>

            <!-- Line Health Index -->
            <!-- 카드 5: 라인별 OEE 평균을 기반으로 계산된 라인 건강도 평균 표시 -->
            <div class="ai-summary-card ai-summary-card--health">
                <span class="ai-summary-card__label">Line Health Index (Avg)</span>
                <!-- 전체 라인 건강도 평균 수치 -->
                <div class="ai-summary-card__value" id="aiHealthAvg">--</div>
                <!-- 건강도 서브 텍스트 + 상태 뱃지 (같은 줄) -->
                <div class="ai-summary-card__sub-row">
                    <div class="ai-summary-card__sub" id="aiHealthSub"><span class="ai-spinner"></span></div>
                    <span id="aiHealthStatusBadge" class="ai-status-badge" style="display:none;"></span>
                </div>
            </div>

        </div>
    </div><!-- /ai-signage-row-a -->

    <!-- Row B: OEE Forecast (2fr) + Anomaly Detection (1fr) -->
    <!-- 중상단 행: 좌측에 OEE 추세+예측 차트, 우측에 이상 감지 목록 -->
    <div class="ai-signage-row-b">

        <!-- OEE 추세 및 AI 예측 차트 카드 -->
        <div class="fiori-card">
            <div class="fiori-card__header">
                <div class="card-title-row">
                    <h3 class="fiori-card__title fiori-text-primary">OEE Trend & AI Forecast</h3>
                    <!-- 범례 설명: 실선=실제, 점선=AI 예측, 90% 신뢰구간 -->
                    <span class="card-subtitle-inline fiori-text-secondary">Solid = Actual &nbsp;/&nbsp; Dashed = AI Forecast (90% CI)</span>
                </div>
            </div>
            <div class="fiori-card__content">
                <!-- OEE 예측 차트 캔버스 (Chart.js 렌더링 대상) -->
                <div class="ai-prediction-chart-wrap">
                    <canvas id="aiOeeForecastChart"></canvas>
                </div>
                <!-- 차트 범례: Actual OEE / AI Forecast / Confidence Interval -->
                <div class="ai-chart-legend">
                    <div class="ai-chart-legend__item">
                        <div class="ai-chart-legend__dot ai-chart-legend__dot--actual"></div>
                        <span>Actual OEE</span>
                    </div>
                    <div class="ai-chart-legend__item">
                        <div class="ai-chart-legend__dot ai-chart-legend__dot--forecast"></div>
                        <span>AI Forecast</span>
                    </div>
                    <div class="ai-chart-legend__item">
                        <div class="ai-chart-legend__dot ai-chart-legend__dot--ci" style="width:20px;height:8px;border-radius:2px;"></div>
                        <span>Confidence Interval (90%)</span>
                    </div>
                </div>
            </div>
        </div>

        <!-- 이상 감지 카드: Z-Score 기반 실시간 이상 목록 표시 -->
        <div class="fiori-card">
            <div class="fiori-card__header">
                <div class="card-title-row">
                    <h3 class="fiori-card__title fiori-text-primary">Anomaly Detection</h3>
                    <span class="card-subtitle-inline fiori-text-secondary">Z-Score based real-time detection</span>
                </div>
                <!-- 이상 건수 카운터 (이상이 있을 때만 표시) -->
                <div id="aiAnomalyHeaderCount" class="ai-last-update" style="display:none;">
                    <div class="ai-pulse-dot" style="background:var(--sap-negative);"></div>
                    <span id="aiAnomalyHeaderText"></span>
                </div>
            </div>
            <div class="fiori-card__content">
                <!-- 이상 감지 항목 목록 (JS에서 동적으로 채움) -->
                <div class="ai-anomaly-list" id="aiAnomalyList">
                    <div class="ai-empty-state">
                        <div class="ai-spinner"></div>
                        <span>Analyzing anomalies...</span>
                    </div>
                </div>
            </div>
        </div>

    </div><!-- /ai-signage-row-b -->

    <!-- Row C: Line Health (1fr) + Predictive Maintenance (1fr) -->
    <!-- 중하단 행: 라인 건강도 인덱스 목록과 예측 정비 위험 점수 목록 -->
    <div class="ai-signage-row-c">

        <!-- 라인 건강도 인덱스 카드 -->
        <div class="fiori-card">
            <div class="fiori-card__header">
                <div class="card-title-row">
                    <h3 class="fiori-card__title fiori-text-primary">Line Health Index</h3>
                    <!-- .ai-health-subtitle 는 JS 에서 date_range 에 맞게 동적으로 업데이트 -->
                    <!-- 서브타이틀은 date_range 파라미터에 따라 JS가 동적으로 변경 -->
                    <span class="card-subtitle-inline ai-health-subtitle">Based on 7-day OEE average per line</span>
                </div>
            </div>
            <div class="fiori-card__content">
                <!-- 라인별 건강도 목록 (JS에서 동적으로 렌더링) -->
                <div class="ai-health-list" id="aiHealthList">
                    <div class="ai-empty-state">
                        <div class="ai-spinner"></div>
                        <span>Calculating health index...</span>
                    </div>
                </div>
            </div>
        </div>

        <!-- 예측 정비(Predictive Maintenance) 카드 -->
        <div class="fiori-card">
            <div class="fiori-card__header">
                <div class="card-title-row">
                    <h3 class="fiori-card__title fiori-text-primary">Predictive Maintenance</h3>
                    <!-- 위험 점수 내림차순으로 기계 목록 표시 -->
                    <span class="card-subtitle-inline fiori-text-secondary">Machines ranked by risk score (high to low)</span>
                </div>
            </div>
            <div class="fiori-card__content">
                <!-- 기계별 위험 점수 목록 (JS에서 동적으로 렌더링) -->
                <div class="ai-maintenance-list" id="aiMaintenanceList">
                    <div class="ai-empty-state">
                        <div class="ai-spinner"></div>
                        <span>Calculating risk scores...</span>
                    </div>
                </div>
            </div>
        </div>

    </div><!-- /ai-signage-row-c -->

    <!-- Row D: AI Streaming (1fr) + Production Optimization (2fr) -->
    <!-- 하단 행: 좌측에 실시간 AI 이벤트 스트림, 우측에 생산 최적화 분석 결과 -->
    <div class="ai-signage-row-d">

        <!-- 실시간 AI 스트리밍 피드 카드 -->
        <div class="fiori-card">
            <div class="fiori-card__header">
                <div class="card-title-row">
                    <h3 class="fiori-card__title fiori-text-primary">
                        Real-time AI Streaming
                        <span class="ai-badge">LIVE</span>
                    </h3>
                    <!-- 이상·다운타임·정비 위험 이벤트 실시간 수신 -->
                    <span class="card-subtitle-inline fiori-text-secondary">Anomaly, downtime, maintenance risk events</span>
                </div>
                <!-- 스트리밍 연결 상태 및 수신 이벤트 수 표시 -->
                <div class="ai-last-update">
                    <div class="ai-pulse-dot" id="aiStreamDot" style="background:#e67e22;"></div>
                    <span id="aiStreamStatus">Connecting...</span>
                    <span id="aiStreamCount" style="font-size:0.75rem;color:var(--sap-text-secondary);margin-left:6px;">0 events</span>
                </div>
            </div>
            <div class="fiori-card__content">
                <!-- AI 스트림 이벤트 피드 (ai_stream_monitor_2.js가 SSE 수신 후 DOM 삽입) -->
                <div id="aiStreamFeed" class="ai-stream-feed">
                    <div class="ai-stream-empty">
                        <span class="ai-spinner"></span> Connecting to AI stream...
                    </div>
                </div>
            </div>
        </div>

        <!-- 생산 최적화 분석 카드 -->
        <div class="fiori-card">
            <div class="fiori-card__header">
                <div class="card-title-row">
                    <h3 class="fiori-card__title fiori-text-primary">
                        Production Optimization
                        <span class="ai-badge">AI</span>
                    </h3>
                    <!-- 분석 기간이 date_range 에 따라 달라짐을 명시 -->
                    <!-- OEE 병목 분석 결과 및 개선 기회를 date_range 기반으로 표시 -->
                    <span class="card-subtitle-inline fiori-text-secondary">OEE bottleneck analysis · Improvement opportunities (period by filter)</span>
                </div>
                <!-- 최적화 요약 바 (AI 분석 결과 간략 표시) -->
                <div id="aiOptSummary" class="ai-opt-summary-bar"></div>
            </div>
            <div class="fiori-card__content">
                <!-- 최적화 항목 목록 (ai_optimization_2.js에서 동적 렌더링) -->
                <div id="aiOptList" class="ai-opt-list">
                    <div class="ai-empty-state">
                        <span class="ai-spinner"></span> Analyzing optimization opportunities...
                    </div>
                </div>
            </div>
        </div>

    </div><!-- /ai-signage-row-d -->

</div><!-- /ai-signage-main -->

<!-- JavaScript Libraries -->
<!-- 외부 라이브러리: Chart.js, jQuery, Moment.js, 공통 JS -->
<script src="../../assets/js/chart.js"></script>
<script src="../../assets/js/jquery-3.6.1.min.js"></script>
<script src="../../assets/js/moment.min.js"></script>
<script src="../../assets/js/common.js"></script>

<!-- AI Dashboard JS -->
<!-- AI 대시보드 핵심 로직, 스트림 모니터, 최적화 분석 스크립트 -->
<script src="js/ai_dashboard_2.js?v=<?php echo filemtime(__DIR__.'/js/ai_dashboard_2.js'); ?>"></script>
<script src="js/ai_stream_monitor_2.js"></script>
<script src="js/ai_optimization_2.js"></script>

<!-- 날짜 필터 getFilterParams() 확장 -->
<!-- dateRangeSelect 값을 getFilterParams()에 date_range 키로 추가하고,
     날짜 변경 시 refreshAll() 호출 -->
<script>
    (function() {
        // 원본 getFilterParams 함수를 보존한 후, date_range 파라미터를 추가하는 방식으로 오버라이드
        var _origGetFilterParams = getFilterParams;
        getFilterParams = function() {
            var p = _origGetFilterParams();
            // dateRangeSelect 선택값을 date_range 파라미터로 추가
            var sel = document.getElementById('dateRangeSelect');
            if (sel && sel.value) p.date_range = sel.value;
            return p;
        };
        // 날짜 범위 변경 시 전체 대시보드 새로고침
        document.getElementById('dateRangeSelect').addEventListener('change', function() {
            if (typeof refreshAll === 'function') refreshAll();
        });
    })();
</script>

<!-- loadPrediction() 오버라이드 — ai_oee_prediction_5.php 호출 -->
<!-- ai_dashboard_2.js의 loadPrediction()을 재정의하여
     proc/ai_oee_prediction_dash_2.php 엔드포인트를 사용하도록 변경 -->
<script>
    (function() {
        // loadPrediction 함수 재정의 (즉시 실행 함수로 스코프 격리)
        loadPrediction = function() {
            // AI OEE 예측 API 호출 (필터 파라미터 포함)
            $.getJSON('proc/ai_oee_prediction_dash_2.php', getFilterParams(), function(data) {
                // API 응답 코드 검증 (비정상 코드 시 오류 메시지 표시 후 종료)
                if (data.code !== '00') {
                    $('#aiRealtimeSub').text('API error');
                    $('#aiPredSub').text('API error');
                    return;
                }

                /* ── 카드1: Real-time OEE (항상 오늘 기준, 클램핑 완료) ── */
                // current_oee 값이 존재하면 float로 파싱, 없으면 null 처리
                var curOee = (data.current_oee !== null && data.current_oee !== undefined) ?
                    parseFloat(data.current_oee) : null;

                if (curOee !== null) {
                    // OEE 수치에 따라 색상 클래스 결정: 85% 이상=good, 60~85%=warning, 60% 미만=danger
                    var colorClass = curOee >= 85 ? 'ai-oee-good' : (curOee >= 60 ? 'ai-oee-warning' : 'ai-oee-danger');
                    $('#aiRealtimeOee')
                        .text(curOee + '%')
                        .removeClass('ai-oee-good ai-oee-warning ai-oee-danger')
                        .addClass(colorClass);

                    // OEE 상태 뱃지 텍스트 및 CSS 클래스 결정
                    var badgeText = curOee >= 85 ? 'GOOD' : (curOee >= 60 ? 'WARNING' : 'CRITICAL');
                    var badgeCls = curOee >= 85 ? 'ai-status-badge--normal' : (curOee >= 60 ? 'ai-status-badge--warning' : 'ai-status-badge--danger');
                    $('#aiRealtimeBadge')
                        .show()
                        .text(badgeText)
                        .removeClass('ai-status-badge--normal ai-status-badge--warning ai-status-badge--danger')
                        .addClass(badgeCls);
                } else {
                    // OEE 데이터가 없을 경우 기본값 표시 및 뱃지 숨김
                    $('#aiRealtimeOee').text('--').removeClass('ai-oee-good ai-oee-warning ai-oee-danger');
                    $('#aiRealtimeBadge').hide();
                }

                // 현재 시간대 레이블 생성 (예: "Current: 14:00")
                var hourLabel = (data.current_hour !== undefined && data.current_hour !== null) ?
                    'Current: ' + String(data.current_hour).padStart(2, '0') + ':00' :
                    'Current hour';
                $('#aiRealtimeSub').text(hourLabel);

                /* ── 카드2: Next 4H AI Forecast ──────────────── */
                // 향후 4시간 예측 데이터 집계 변수 초기화
                var forecastAvg = null;
                var ciMin = null,
                    ciMax = null;

                // 예측 데이터가 존재하는 경우 평균 및 신뢰구간 계산
                if (data.forecast && data.forecast.length > 0) {
                    var sum = 0;
                    // 예측값 합산
                    data.forecast.forEach(function(f) {
                        sum += parseFloat(f.oee);
                    });
                    // 예측 평균 OEE (소수점 1자리)
                    forecastAvg = (sum / data.forecast.length).toFixed(1);

                    // 신뢰구간 하한 최솟값
                    ciMin = Math.min.apply(null, data.forecast.map(function(f) {
                        return parseFloat(f.lower);
                    })).toFixed(1);
                    // 신뢰구간 상한 최댓값
                    ciMax = Math.max.apply(null, data.forecast.map(function(f) {
                        return parseFloat(f.upper);
                    })).toFixed(1);

                    // 예측 OEE 수치 및 CI 범위 표시
                    $('#aiPredForecastOee').text(forecastAvg + '%');
                    $('#aiPredSub').text('CI: ' + ciMin + '% ~ ' + ciMax + '%');
                } else {
                    // 예측 데이터 부족 시 기본 메시지 표시
                    $('#aiPredForecastOee').text('--');
                    $('#aiPredSub').text('Insufficient data');
                }

                /* ── 트렌드 배지 ──────────────────────────────── */
                // 트렌드 방향별 CSS 클래스 및 텍스트 매핑
                var trendMap = {
                    up: {
                        cls: 'ai-trend-badge--up',
                        text: 'Trending Up'
                    },
                    down: {
                        cls: 'ai-trend-badge--down',
                        text: 'Trending Down'
                    },
                    stable: {
                        cls: 'ai-trend-badge--stable',
                        text: 'Stable'
                    },
                };
                // API 응답의 trend 값으로 뱃지 업데이트 (미정의 시 stable 기본값)
                var trend = trendMap[data.trend] || trendMap.stable;
                $('#aiPredTrendBadge')
                    .removeClass('ai-trend-badge--up ai-trend-badge--down ai-trend-badge--stable')
                    .addClass(trend.cls)
                    .text(trend.text);

                /* ── OEE Forecast 차트 (today_data 포함) & 라인 건강지수 ── */
                // 예측 차트 렌더링 (실제 데이터 + 예측 데이터 + CI 밴드)
                renderForecastChart(data);
                // 라인 건강도 인덱스 별도 API 호출 및 렌더링
                loadLineHealth();

            }).fail(function() {
                // AJAX 요청 실패 시 오류 메시지 표시
                $('#aiRealtimeSub').text('API error');
                $('#aiPredSub').text('API error');
            });
        };
    })();
</script>

<!-- Export Report Modal -->
<!-- 리포트 내보내기 팝업 모달: 기간 프리셋 선택 또는 커스텀 날짜 범위 지정 -->
<div id="exportModal" style="display:none;position:fixed;inset:0;z-index:9999;background:rgba(0,0,0,.6);align-items:center;justify-content:center;">
    <div style="background:#161b22;border:1px solid #30363d;border-radius:10px;padding:24px 28px;min-width:340px;box-shadow:0 8px 32px #000a;">
        <div style="font-size:1rem;font-weight:600;color:#58a6ff;margin-bottom:12px;">Export Report — Select Period</div>

        <!-- 언어 선택 -->
        <div style="display:flex;gap:6px;margin-bottom:16px;align-items:center;">
            <span style="font-size:.8rem;color:#8b949e;margin-right:4px;">Language:</span>
            <button class="export-lang fiori-btn fiori-btn--emphasized" data-lang="en" style="font-size:.8rem;padding:3px 12px;">ENG</button>
            <button class="export-lang fiori-btn fiori-btn--tertiary"   data-lang="ko" style="font-size:.8rem;padding:3px 12px;">KOR</button>
        </div>

        <!-- 기간 프리셋 버튼 목록 (Today / Yesterday / Last 7 Days / Last 30 Days) -->
        <div style="display:flex;gap:8px;margin-bottom:16px;flex-wrap:wrap;">
            <button class="export-preset fiori-btn fiori-btn--tertiary" data-range="today">Today</button>
            <button class="export-preset fiori-btn fiori-btn--tertiary" data-range="yesterday">Yesterday</button>
            <button class="export-preset fiori-btn fiori-btn--tertiary" data-range="1w">Last 7 Days</button>
            <button class="export-preset fiori-btn fiori-btn--tertiary" data-range="1m">Last 30 Days</button>
        </div>

        <!-- 커스텀 날짜 범위 입력 영역 -->
        <div style="font-size:.8rem;color:#8b949e;margin-bottom:6px;">Custom Range</div>
        <div style="display:flex;gap:8px;align-items:center;margin-bottom:20px;">
            <!-- 시작 날짜 입력 -->
            <input type="date" id="exportDateFrom" class="fiori-input" style="flex:1;background:#21262d;color:#e6edf3;border:1px solid #30363d;border-radius:4px;padding:5px 8px;">
            <span style="color:#8b949e;">~</span>
            <!-- 종료 날짜 입력 -->
            <input type="date" id="exportDateTo" class="fiori-input" style="flex:1;background:#21262d;color:#e6edf3;border:1px solid #30363d;border-radius:4px;padding:5px 8px;">
        </div>

        <!-- 취소/내보내기 버튼 -->
        <div style="display:flex;gap:8px;justify-content:flex-end;">
            <button id="exportCancelBtn" class="fiori-btn fiori-btn--ghost">Cancel</button>
            <button id="exportConfirmBtn" class="fiori-btn fiori-btn--primary">Export</button>
        </div>
    </div>
</div>

<script>
    (function() {
        // 내보내기 모달 DOM 요소 참조
        var modal = document.getElementById('exportModal');

        /**
         * fmtDate: Date 객체를 YYYY-MM-DD 형식 문자열로 변환
         * @param {Date} d
         * @returns {string}
         */
        function fmtDate(d) {
            return d.toISOString().slice(0, 10);
        }

        /**
         * calcRange: 기간 키워드를 받아 시작·종료 날짜 객체 반환
         * @param {string} range - 'today' | 'yesterday' | '1w' | '1m'
         * @returns {{from: string, to: string}}
         */
        function calcRange(range) {
            var now = new Date(),
                ms = 86400000; // 하루를 밀리초로 표현
            // 기간 매핑 테이블: 각 키에 대해 from/to 날짜 계산
            var map = {
                today: {
                    from: fmtDate(now),
                    to: fmtDate(now)
                },
                yesterday: {
                    from: fmtDate(new Date(now - ms)),
                    to: fmtDate(new Date(now - ms))
                },
                '1w': {
                    from: fmtDate(new Date(now - 6 * ms)),  // 6일 전 ~ 오늘 (총 7일)
                    to: fmtDate(now)
                },
                '1m': {
                    from: fmtDate(new Date(now - 29 * ms)), // 29일 전 ~ 오늘 (총 30일)
                    to: fmtDate(now)
                },
            };
            return map[range] || map['today'];
        }

        /**
         * setPreset: 프리셋 버튼 클릭 시 날짜 입력 필드에 자동으로 날짜를 채우고
         *            선택된 프리셋 버튼을 강조 표시
         * @param {string} range - 프리셋 키워드
         */
        function setPreset(range) {
            var r = calcRange(range);
            // 날짜 입력 필드에 계산된 날짜 설정
            document.getElementById('exportDateFrom').value = r.from;
            document.getElementById('exportDateTo').value = r.to;
            // 프리셋 버튼 강조 토글 (선택된 것만 emphasized)
            document.querySelectorAll('.export-preset').forEach(function(b) {
                b.classList.toggle('fiori-btn--emphasized', b.dataset.range === range);
                b.classList.toggle('fiori-btn--tertiary', b.dataset.range !== range);
            });
        }

        // 언어 버튼 토글
        document.querySelectorAll('.export-lang').forEach(function(btn) {
            btn.addEventListener('click', function() {
                document.querySelectorAll('.export-lang').forEach(function(b) {
                    b.classList.remove('fiori-btn--emphasized');
                    b.classList.add('fiori-btn--tertiary');
                });
                this.classList.remove('fiori-btn--tertiary');
                this.classList.add('fiori-btn--emphasized');
            });
        });

        // "Export" 버튼 클릭 시 모달 열기 (기본 프리셋: today)
        document.getElementById('exportReportBtn').addEventListener('click', function() {
            setPreset('today');
            modal.style.display = 'flex';
        });

        // 프리셋 버튼 각각에 클릭 이벤트 바인딩
        document.querySelectorAll('.export-preset').forEach(function(btn) {
            btn.addEventListener('click', function() {
                setPreset(this.dataset.range);
            });
        });

        // 날짜 직접 변경 시 프리셋 버튼 강조 해제 (커스텀 범위로 간주)
        ['exportDateFrom', 'exportDateTo'].forEach(function(id) {
            document.getElementById(id).addEventListener('change', function() {
                document.querySelectorAll('.export-preset').forEach(function(b) {
                    b.classList.remove('fiori-btn--emphasized');
                    b.classList.add('fiori-btn--tertiary');
                });
            });
        });

        // "Export" 확인 버튼 클릭: 날짜 검증 후 리포트 내보내기 API 새 탭으로 호출
        document.getElementById('exportConfirmBtn').addEventListener('click', function() {
            var from = document.getElementById('exportDateFrom').value;
            var to = document.getElementById('exportDateTo').value;
            // 날짜 미입력 시 경고 후 종료
            if (!from || !to) {
                alert('Please select a date range.');
                return;
            }
            // 필터 파라미터에 커스텀 범위 추가 후 내보내기 URL 구성
            var p = getFilterParams();
            p.range = 'custom';
            p.date_from = from;
            p.date_to = to;
            // 선택된 언어 파라미터 추가
            var activeLang = document.querySelector('.export-lang.fiori-btn--emphasized');
            p.lang = activeLang ? activeLang.dataset.lang : 'en';
            // 새 탭에서 ai_report_export_2.php 호출 (HTML 리포트 다운로드 처리)
            window.open('proc/ai_report_export_2.php?' + new URLSearchParams(p), '_blank');
            modal.style.display = 'none';
        });

        // "Cancel" 버튼 클릭 시 모달 닫기
        document.getElementById('exportCancelBtn').addEventListener('click', function() {
            modal.style.display = 'none';
        });

        // 모달 배경(backdrop) 클릭 시 모달 닫기
        modal.addEventListener('click', function(e) {
            if (e.target === modal) modal.style.display = 'none';
        });
    })();
</script>

<!-- 실시간 시계 -->
<!-- 헤더 우측에 HH:MM:SS 형식의 시계를 1초마다 갱신 -->
<script>
    (function() {
        /**
         * updateClock: 현재 시각을 읽어 #signageClock 요소에 HH:MM:SS 형식으로 표시
         */
        function updateClock() {
            var now = new Date();
            var h = String(now.getHours()).padStart(2, '0');
            var m = String(now.getMinutes()).padStart(2, '0');
            var s = String(now.getSeconds()).padStart(2, '0');
            var el = document.getElementById('signageClock');
            if (el) el.textContent = h + ':' + m + ':' + s;
        }
        // 페이지 로드 즉시 시계 표시
        updateClock();
        // 1초(1000ms) 간격으로 시계 갱신
        setInterval(updateClock, 1000);
    })();
</script>

</body>

</html>
