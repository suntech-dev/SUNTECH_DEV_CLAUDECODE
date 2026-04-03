<?php

/*
 * ============================================================
 * 파일명  : dashboard_2.php
 * 목  적  : 1920×1080 사이니지(Signage) 전용 OEE 메인 대시보드
 *           OEE 4대 지표(가동률·성능률·품질률·종합OEE) 게이지,
 *           다운타임·불량·안돈 발생 차트, OEE 추세 및 타임라인,
 *           생산 히트맵을 한 화면에 표시한다.
 * 특징:
 *   - nav 제거, 슬림 헤더 55px 적용 (사이니지 최적화)
 *   - CSS Grid 4행 고정 레이아웃 (overflow: hidden)
 *   - js/dashboard_2.js 100% 재사용
 * 연관 파일:
 *   - css/dashboard.css       : 대시보드 공통 스타일
 *   - css/dashboard_2.css     : 사이니지 전용 레이아웃 스타일
 *   - js/dashboard_2.js       : 대시보드 데이터 로딩·차트 렌더링 로직
 *   - inc/signage_filters.php : 공장·라인·기계·날짜 필터 HTML 조각
 * ============================================================
 */

/**
 * dashboard_2.php — 1920x1080 Signage Dashboard
 * dashboard.php 의 사이니지 전용 버전
 * - nav 제거, 슬림 헤더 55px
 * - CSS Grid 4행 고정 레이아웃 (overflow: hidden)
 * - js/dashboard_2.js 100% 재사용
 */

// 브라우저 탭 및 페이지 제목 설정
$page_title = 'SCI OEE Dashboard - Signage';

// 페이지에서 사용할 CSS 파일 목록
$page_css_files = [
    '../../assets/css/fiori-page.css',       // Fiori 공통 페이지 스타일
    '../../assets/css/daterangepicker.css',   // 날짜 범위 선택기 스타일
    'css/dashboard.css',                       // 대시보드 공통 스타일
    'css/dashboard_2.css',                     // 사이니지 전용 레이아웃 스타일
];

// 공통 헤드 파일 포함 (HTML head 태그, CSS 로드 등)
require_once(__DIR__ . '/../../inc/head.php');
/* nav-fiori.php 제거 — 사이니지에는 네비게이션 불필요 */
?>

<?php
// 네비게이션 컨텍스트 및 활성 메뉴 설정
$nav_context = 'data';
$nav_active = 'oee_dashboard';
// 사이드 드로어(네비게이션 메뉴) 포함
require_once(__DIR__ . '/../../inc/nav-drawer-manage.php'); ?>

<!-- Signage Header (nav 대체, 52px) -->
<!-- 사이니지 상단 헤더: 메뉴 버튼, OEE DASHBOARD 타이틀, 필터, 상태 표시, 시계 -->
<div class="signage-header">
    <!-- 좌측 햄버거 메뉴 버튼 (네비게이션 드로어 토글) -->
    <button id="navDrawerBtn" class="nav-drawer-btn" aria-label="Menu">&#9776;</button>
    <span class="signage-header__title">OEE DASHBOARD</span>

    <!-- 필터 영역: 공장·라인·기계·날짜 필터 및 새로고침 버튼 -->
    <div class="signage-header__filters">
        <!-- 공통 사이니지 필터 (inc/signage_filters.php): 공장·라인·기계·날짜 선택기 포함 -->
        <?php include __DIR__ . '/inc/signage_filters.php'; ?>
        <!-- 수동 새로고침 버튼 -->
        <button id="refreshBtn" class="fiori-btn fiori-btn--tertiary">Refresh</button>
    </div>

    <!-- 마지막 업데이트 시간 및 연결 상태 표시 -->
    <div class="signage-header__status">
        <div class="status-dot"></div>
        <span id="lastUpdateTime">--:--:--</span>
    </div>

    <!-- 우측 실시간 시계 (HH:MM:SS 형식, JS로 갱신) -->
    <div class="signage-header__clock" id="signageClock"></div>
</div>

<!-- Signage Main: 4행 CSS Grid -->
<!-- 전체 대시보드를 Row A~D 4개 행으로 구성 -->
<div class="signage-main">

    <!-- Row A: OEE 4 metrics (3fr) + Active Andon Feed (1fr) -->
    <!-- 상단 행: OEE 4대 지표 게이지 카드 -->
    <div class="signage-row-a">

        <!-- OEE 4 metrics -->
        <!-- 가동률·성능률·품질률·종합OEE 게이지 카드 4개 -->
        <div class="signage-oee-metrics">

            <!-- Availability -->
            <!-- 가동률 게이지 카드: Downtime 및 Planned Time 진행 막대 포함 -->
            <div class="oee-metric-card oee-metric-card--availability">
                <div class="oee-metric-label">Availability</div>
                <!-- 원형 게이지 캔버스 (Chart.js 도넛 차트) -->
                <div class="oee-metric-gauge">
                    <canvas id="availabilityGauge" width="150" height="150"></canvas>
                </div>
                <!-- 가동률 세부 지표: 다운타임 및 계획 시간 진행 바 -->
                <div class="availability-metrics" style="width:100%">
                    <div class="metric-row">
                        <span class="metric-label" style="font-size:0.8rem">Downtime</span>
                        <!-- 다운타임 수치 (JS에서 업데이트) -->
                        <span class="metric-value" id="runtime-value">-</span>
                    </div>
                    <!-- 다운타임 진행률 바 -->
                    <div class="metric-progress">
                        <div class="progress-bar" id="runtime-progress" style="width:0%"></div>
                    </div>
                    <div class="metric-row">
                        <span class="metric-label" style="font-size:0.8rem">Planned Time</span>
                        <!-- 계획 시간 수치 (JS에서 업데이트) -->
                        <span class="metric-value" id="planned-time-value">-</span>
                    </div>
                    <!-- 계획 시간 진행률 바 -->
                    <div class="metric-progress">
                        <div class="progress-bar" id="planned-progress" style="width:0%"></div>
                    </div>
                </div>
                <!-- 전일 대비 변화량 표시 -->
                <div class="oee-metric-change">
                    <span id="availabilityTrend">-</span>
                    <span id="availabilityChange">vs Last Day</span>
                </div>
            </div>

            <!-- Performance -->
            <!-- 성능률 게이지 카드: 실제 생산량 및 이론 생산량 진행 막대 포함 -->
            <div class="oee-metric-card oee-metric-card--performance">
                <div class="oee-metric-label">Performance</div>
                <!-- 원형 게이지 캔버스 -->
                <div class="oee-metric-gauge">
                    <canvas id="performanceGauge" width="150" height="150"></canvas>
                </div>
                <!-- 성능률 세부 지표: 실제 생산량 및 이론 생산량 -->
                <div class="performance-metrics" style="width:100%">
                    <div class="metric-row">
                        <span class="metric-label" style="font-size:0.8rem">Actual Output</span>
                        <!-- 실제 생산량 수치 -->
                        <span class="metric-value" id="actual-output-value">-</span>
                    </div>
                    <!-- 실제 생산량 진행률 바 -->
                    <div class="metric-progress">
                        <div class="progress-bar" id="actual-output-progress" style="width:0%"></div>
                    </div>
                    <div class="metric-row">
                        <span class="metric-label" style="font-size:0.8rem">Theoretical Output</span>
                        <!-- 이론 생산량 수치 -->
                        <span class="metric-value" id="theoretical-output-value">-</span>
                    </div>
                    <!-- 이론 생산량 진행률 바 -->
                    <div class="metric-progress">
                        <div class="progress-bar" id="theoretical-output-progress" style="width:0%"></div>
                    </div>
                </div>
                <!-- 전일 대비 변화량 표시 -->
                <div class="oee-metric-change">
                    <span id="performanceTrend">-</span>
                    <span id="performanceChange">vs Last Day</span>
                </div>
            </div>

            <!-- Quality -->
            <!-- 품질률 게이지 카드: 양품 및 불량품 진행 막대 포함 -->
            <div class="oee-metric-card oee-metric-card--quality">
                <div class="oee-metric-label">Quality</div>
                <!-- 원형 게이지 캔버스 -->
                <div class="oee-metric-gauge">
                    <canvas id="qualityGauge" width="150" height="150"></canvas>
                </div>
                <!-- 품질률 세부 지표: 양품·불량품 수량 -->
                <div class="quality-metrics" style="width:100%">
                    <div class="metric-row">
                        <span class="metric-label" style="font-size:0.8rem">Good Products</span>
                        <!-- 양품 수량 -->
                        <span class="metric-value" id="good-products-value">-</span>
                    </div>
                    <!-- 양품 진행률 바 -->
                    <div class="metric-progress">
                        <div class="progress-bar" id="good-products-progress" style="width:0%"></div>
                    </div>
                    <div class="metric-row">
                        <span class="metric-label" style="font-size:0.8rem">Defective Products</span>
                        <!-- 불량품 수량 -->
                        <span class="metric-value" id="defective-products-value">-</span>
                    </div>
                    <!-- 불량품 진행률 바 -->
                    <div class="metric-progress">
                        <div class="progress-bar" id="defective-products-progress" style="width:0%"></div>
                    </div>
                </div>
                <!-- 전일 대비 변화량 표시 -->
                <div class="oee-metric-change">
                    <span id="qualityTrend">-</span>
                    <span id="qualityChange">vs Last Day</span>
                </div>
            </div>

            <!-- Overall OEE -->
            <!-- 종합 OEE 게이지 카드: 가동률×성능률×품질률 종합 수치 -->
            <div class="oee-metric-card oee-metric-card--overall">
                <div class="oee-metric-label">OEE</div>
                <!-- 원형 게이지 캔버스 -->
                <div class="oee-metric-gauge">
                    <canvas id="overallGauge" width="150" height="150"></canvas>
                </div>
                <!-- 전일 대비 OEE 변화량 표시 -->
                <div class="oee-metric-change">
                    <span id="overallTrend">-</span>
                    <span id="overallChange">vs Last Day</span>
                </div>
            </div>

        </div><!-- /signage-oee-metrics -->

    </div><!-- /signage-row-a -->

    <!-- Row B: Downtime / Defective / Andon Warning Qty / Currently active Andon -->
    <!-- 중상단 행: 다운타임·불량·안돈 유형별 발생 차트 및 현재 활성 안돈 피드 -->
    <div class="signage-row-b">

        <!-- 다운타임 유형별 발생 차트 카드 -->
        <div class="fiori-card">
            <div class="fiori-card__header">
                <h3 class="fiori-card__title fiori-text-primary">Downtime</h3>
                <p class="fiori-card__subtitle fiori-text-secondary">By type</p>
            </div>
            <div class="fiori-card__content">
                <div style="flex:1; position:relative; min-height:0;">
                    <!-- 다운타임 발생 건수 차트 캔버스 -->
                    <canvas id="downtimeOccurrenceChart"></canvas>
                </div>
            </div>
        </div>

        <!-- 불량 유형별 발생 차트 카드 -->
        <div class="fiori-card">
            <div class="fiori-card__header">
                <h3 class="fiori-card__title fiori-text-primary">Defective</h3>
                <p class="fiori-card__subtitle fiori-text-secondary">By type</p>
            </div>
            <div class="fiori-card__content">
                <div style="flex:1; position:relative; min-height:0;">
                    <!-- 불량 발생 건수 차트 캔버스 -->
                    <canvas id="defectiveOccurrenceChart"></canvas>
                </div>
            </div>
        </div>

        <!-- 안돈 경보 유형별 발생 건수 차트 카드 -->
        <div class="fiori-card">
            <div class="fiori-card__header">
                <h3 class="fiori-card__title fiori-text-primary">Andon Warning Qty</h3>
                <p class="fiori-card__subtitle fiori-text-secondary">By type</p>
            </div>
            <div class="fiori-card__content">
                <div style="flex:1; position:relative; min-height:0;">
                    <!-- 안돈 발생 건수 차트 캔버스 -->
                    <canvas id="andonOccurrenceChart"></canvas>
                </div>
            </div>
        </div>

        <!-- Weekly Andon Warning (hidden — moved to Row B slot 4 with Andon Feed) -->
        <!-- 주간 안돈 경보 트렌드 차트 (현재 숨김 처리 — Row B 4번 슬롯으로 이동됨) -->
        <div style="display:none;">
            <canvas id="weeklyAndonTrendChart"></canvas>
        </div>

        <!-- Currently active Andon (moved from Row A) -->
        <!-- 현재 활성화된 안돈 경보 실시간 피드 카드 -->
        <div class="fiori-card">
            <div class="fiori-card__header">
                <h3 class="fiori-card__title fiori-text-primary">Currently active Andon</h3>
                <!-- 활성 안돈 건수 및 실시간 상태 표시 -->
                <div class="real-time-status real-time-status-header">
                    <div class="status-dot"></div>
                    <span id="activeAndonCount">0 active alerts</span>
                </div>
            </div>
            <div class="fiori-card__content">
                <!-- 안돈 경보 피드 목록 (JS에서 실시간으로 업데이트) -->
                <div id="andonAlarmFeed" style="flex:1; display:flex; flex-direction:column; overflow-y:auto;">
                    <!-- 기본 상태: 활성 안돈 없음 메시지 -->
                    <div class="fiori-alert fiori-alert--info">
                        <strong>Info:</strong> No active Andon. Real-time monitoring active.
                    </div>
                </div>
            </div>
        </div>

    </div><!-- /signage-row-b -->

    <!-- Row C: OEE Trend (2fr) + OEE Timeline (1fr) -->
    <!-- 중하단 행: 시간별 OEE 추세 차트와 OEE 타임라인 바 -->
    <div class="signage-row-c">

        <!-- OEE 추세 차트 카드 (시간별 OEE 변화 라인 차트) -->
        <div class="fiori-card">
            <div class="fiori-card__header">
                <h3 class="fiori-card__title fiori-text-primary">OEE Trend</h3>
                <p class="fiori-card__subtitle fiori-text-secondary">Hourly OEE trend</p>
            </div>
            <div class="fiori-card__content">
                <div style="flex:1; position:relative; min-height:0;">
                    <!-- OEE 추세 차트 캔버스 -->
                    <canvas id="oeeTrendChart"></canvas>
                </div>
            </div>
        </div>

        <!-- OEE 타임라인 카드: 시간대별 OEE 색상 바 시각화 -->
        <div class="fiori-card">
            <div class="fiori-card__header">
                <h3 class="fiori-card__title fiori-text-primary">OEE Timeline</h3>
                <!-- 서브타이틀은 JS에서 동적으로 업데이트 -->
                <p class="fiori-card__subtitle fiori-text-secondary" id="timelineSubtitle">Hourly OEE timeline</p>
            </div>
            <div class="fiori-card__content">
                <!-- 타임라인 컨테이너 -->
                <div class="production-timeline">
                    <!-- 시간 헤더 (JS에서 시간 레이블 생성) -->
                    <div class="timeline-header" id="timelineHeader"></div>
                    <div class="timeline-bars">
                        <!-- 생산 타임라인 바 (각 시간대별 OEE 색상으로 채워짐) -->
                        <div class="timeline-bar" id="productionTimeline"></div>
                    </div>
                    <!-- 타임라인 색상 범례 -->
                    <div class="timeline-legend">
                        <div class="legend-item">
                            <div style="padding:4px; color:var(--sap-text-secondary); font-size:10px;">Loading legend...</div>
                        </div>
                    </div>
                </div>
            </div>
        </div>

    </div><!-- /signage-row-c -->

    <!-- Row D: Production Heatmap (full width) -->
    <!-- 하단 행: 생산 히트맵 (요일×시간대별 생산량 분포) -->
    <div class="signage-row-d">

        <!-- 생산 히트맵 카드 -->
        <div class="fiori-card">
            <div class="fiori-card__header">
                <h3 class="fiori-card__title fiori-text-primary">Production Heatmap</h3>
                <p class="fiori-card__subtitle fiori-text-secondary">Production distribution by day and time</p>
            </div>
            <div class="fiori-card__content">
                <div class="heatmap">
                    <!-- 히트맵 그리드: 요일×시간 셀 (JS에서 동적으로 생성) -->
                    <div class="heatmap-grid" id="productionHeatmap"></div>
                    <!-- 시간 축 레이블 -->
                    <div class="heatmap-axis">
                        <span>08:00</span>
                        <span>12:00</span>
                        <span>16:00</span>
                        <span>20:00</span>
                    </div>
                </div>
            </div>
        </div>

    </div><!-- /signage-row-d -->

</div><!-- /signage-main -->

<!-- JavaScript Libraries -->
<!-- 외부 라이브러리: Chart.js, jQuery, Moment.js, DateRangePicker -->
<script src="../../assets/js/chart.js"></script>
<script src="../../assets/js/jquery-3.6.1.min.js"></script>
<script src="../../assets/js/moment.min.js"></script>
<script src="../../assets/js/daterangepicker.js"></script>

<!-- Dashboard JS (100% 재사용) -->
<!-- 대시보드 데이터 로딩, 차트 렌더링, 필터 이벤트 처리 모두 포함 -->
<script src="js/dashboard_2.js"></script>

<!-- 사이니지 전용: 실시간 시계 -->
<!-- 1초마다 헤더 우측 시계를 HH:MM:SS 형식으로 갱신 -->
<script>
    (function() {
        /**
         * updateClock: 현재 시각을 #signageClock 요소에 표시
         */
        function updateClock() {
            var now = new Date();
            var h = String(now.getHours()).padStart(2, '0');
            var m = String(now.getMinutes()).padStart(2, '0');
            var s = String(now.getSeconds()).padStart(2, '0');
            var el = document.getElementById('signageClock');
            if (el) el.textContent = h + ':' + m + ':' + s;
        }
        // 초기 시계 표시
        updateClock();
        // 1초(1000ms) 간격으로 시계 갱신
        setInterval(updateClock, 1000);
    })();
</script>

</body>

</html>
