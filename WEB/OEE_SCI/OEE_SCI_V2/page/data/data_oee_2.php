<?php
/*
 * ============================================================
 * 파일명  : data_oee_2.php
 * 목  적  : OEE(Overall Equipment Effectiveness) 데이터 모니터링 페이지
 *           실시간 OEE 데이터 테이블, 통계 카드, 분석 차트를
 *           사이니지 레이아웃으로 표시한다.
 * 레이아웃 구성:
 *   - Row A: 통계 카드 (기본 숨김, Show Stats 버튼으로 토글, oee_stats_grid.php 포함)
 *   - Row B: OEE 구성 요소 상세 + 컴포넌트/등급 분포 차트 2개 (기본 숨김)
 *   - Row C: OEE 추세(AI 예측 포함)·타임라인·라인별 성과 차트 3개 (기본 숨김)
 *   - Row D: 실시간 OEE 데이터 테이블 (기본 표시)
 *   - Row E: 페이지네이션 컨트롤
 * 특징:
 *   - AI 기반 OEE 추세 예측 오버레이: ai_oee_overlay_2.js 사용
 * 연관 파일:
 *   - css/data_oee_2.css         : OEE 모니터링 전용 스타일
 *   - css/ai_dashboard.css       : AI 관련 공통 스타일 (뱃지, 트렌드 등)
 *   - js/data_oee_2.js           : OEE 데이터 로딩·렌더링·필터 로직
 *   - js/ai_oee_overlay_2.js     : AI OEE 예측 오버레이 렌더링
 *   - inc/signage_filters.php    : 공장·라인·기계·날짜 필터 HTML 조각
 *   - inc/oee_stats_grid.php     : OEE 통계 카드 그리드 HTML 조각
 * ============================================================
 */

// 브라우저 탭 및 페이지 제목 설정
$page_title = 'OEE Data Monitoring';

// 페이지에서 사용할 CSS 파일 목록
$page_css_files = [
    '../../assets/css/fiori-page.css',       // Fiori 공통 페이지 스타일
    '../../assets/css/daterangepicker.css',   // 날짜 범위 선택기 스타일
    'css/data_oee_2.css',                     // OEE 모니터링 전용 스타일
    'css/ai_dashboard.css',                   // AI 대시보드 공통 스타일 (뱃지, 트렌드 배지 등)
];

// 공통 헤드 파일 포함 (HTML head 태그, CSS 로드, meta 태그 등)
require_once(__DIR__ . '/../../inc/head.php');
?>

<?php
// 네비게이션 컨텍스트 및 활성 메뉴 설정
$nav_context = 'data';
$nav_active = 'oee'; // 네비게이션에서 'oee' 메뉴 항목을 활성화
// 사이드 드로어(네비게이션 메뉴) 포함
require_once(__DIR__ . '/../../inc/nav-drawer-manage.php'); ?>

<!-- Signage Header -->
<!-- 사이니지 상단 헤더: 메뉴 버튼, 타이틀, 필터 및 제어 버튼들 -->
<div class="signage-header">
    <!-- 좌측 햄버거 메뉴 버튼 (네비게이션 드로어 토글) -->
    <button id="navDrawerBtn" class="nav-drawer-btn" aria-label="Menu">&#9776;</button>
    <span class="signage-header__title">OEE Monitoring</span>

    <!-- 필터 및 뷰 제어 버튼 영역 -->
    <div class="signage-header__filters">
        <!-- 공통 사이니지 필터 (공장·라인·기계·날짜 선택기) -->
        <?php include __DIR__ . '/inc/signage_filters.php'; ?>
        <!-- 통계 행 토글 버튼 (Show Stats / Hide Stats) -->
        <button id="toggleStatsBtn" class="fiori-btn fiori-btn--secondary">Show Stats</button>
        <!-- 차트 행 토글 버튼 (Show Charts / Hide Charts) -->
        <button id="toggleChartsBtn" class="fiori-btn fiori-btn--secondary">Show Charts</button>
        <!-- 테이블 행 토글 버튼 (Hide Table / Show Table) -->
        <button id="toggleDataBtn" class="fiori-btn fiori-btn--secondary">Hide Table</button>
        <!-- Excel/CSV 내보내기 버튼 -->
        <button id="excelDownloadBtn" class="fiori-btn fiori-btn--secondary">Export</button>
        <!-- 수동 새로고침 버튼 -->
        <button id="refreshBtn" class="fiori-btn fiori-btn--tertiary">Refresh</button>
    </div>
</div>

<!-- OEE Signage Main -->
<!-- OEE 모니터링 메인 컨텐츠 영역 -->
<div class="oee-signage-main" id="oeeSignageMain">

    <!-- Row A: Stats (기본 hidden) -->
    <!-- OEE 통계 카드 행: Show Stats 버튼 클릭 시 표시됨 -->
    <div id="oeeRowStats" class="oee-row oee-row--stats hidden">
        <div class="oee-stats-grid">
            <!-- OEE 통계 카드 그리드 HTML 조각 포함 (공통 컴포넌트) -->
            <?php include __DIR__ . '/inc/oee_stats_grid.php'; ?>
        </div>
    </div>

    <!-- Row B: Charts Top — Components Details(2fr) + 차트 2개(3fr) (기본 hidden) -->
    <!-- 차트 상단 행: Show Charts 버튼 클릭 시 표시됨 -->
    <div id="oeeRowChartsTop" class="oee-row oee-row--charts-top hidden">
        <div class="oee-charts-top-grid">

            <!-- 좌: OEE Components Details -->
            <!-- OEE 구성 요소(가동률·성능률·품질률·OEE) 상세 수치 카드 (2fr 너비) -->
            <div class="fiori-card">
                <div class="fiori-card__header">
                    <div class="card-title-row">
                        <h3 class="fiori-card__title">OEE Components Details</h3>
                    </div>
                    <!-- 실시간 모니터링 상태 표시 -->
                    <div class="real-time-status">
                        <div class="status-dot"></div>
                        <span id="oeeLiveStatus">Real-time monitoring active</span>
                    </div>
                </div>
                <div class="fiori-card__content">
                    <!-- OEE 구성 요소 상세 목록 (JS에서 데이터 업데이트) -->
                    <div class="oee-details-list" id="oeeDetailsContainer">
                        <!-- 항목 1: OEE Rate (전체 효율·목표 달성률 포함) -->
                        <div class="oee-component-item">
                            <div class="oee-component-info">
                                <div class="oee-component-name">OEE Rate</div>
                                <div class="oee-component-details">
                                    <!-- 전체 효율 수치 -->
                                    <span class="oee-detail-item">Overall Efficiency: <span id="overallEfficiency">-</span></span>
                                    <!-- 목표 달성률 -->
                                    <span class="oee-detail-item">Target Achievement: <span id="targetAchievement">-</span></span>
                                </div>
                            </div>
                            <!-- OEE Rate 요약 값 -->
                            <div class="oee-component-value"><span id="oeeRateDetail">-</span></div>
                        </div>
                        <!-- 항목 2: Availability Rate (다운타임·계획 시간 포함) -->
                        <div class="oee-component-item">
                            <div class="oee-component-info">
                                <div class="oee-component-name">Availability Rate</div>
                                <div class="oee-component-details">
                                    <!-- 다운타임(분) -->
                                    <span class="oee-detail-item">Downtime: <span id="runtime">-</span>m</span>
                                    <!-- 계획 작업 시간(분) -->
                                    <span class="oee-detail-item">Planned Time: <span id="plannedTime">-</span>m</span>
                                </div>
                            </div>
                            <!-- 가동률 요약 값 -->
                            <div class="oee-component-value"><span id="availabilityDetail">-</span></div>
                        </div>
                        <!-- 항목 3: Performance Rate (실제 생산량·이론 생산량 포함) -->
                        <div class="oee-component-item">
                            <div class="oee-component-info">
                                <div class="oee-component-name">Performance Rate</div>
                                <div class="oee-component-details">
                                    <!-- 실제 생산량 -->
                                    <span class="oee-detail-item">Actual Output: <span id="actualOutput">-</span></span>
                                    <!-- 이론 생산량 -->
                                    <span class="oee-detail-item">Theoretical Output: <span id="theoreticalOutput">-</span></span>
                                </div>
                            </div>
                            <!-- 성능률 요약 값 -->
                            <div class="oee-component-value"><span id="performanceDetail">-</span></div>
                        </div>
                        <!-- 항목 4: Quality Rate (양품·불량품 수량 포함) -->
                        <div class="oee-component-item">
                            <div class="oee-component-info">
                                <div class="oee-component-name">Quality Rate</div>
                                <div class="oee-component-details">
                                    <!-- 양품 수량 -->
                                    <span class="oee-detail-item">Good Products: <span id="goodProducts">-</span></span>
                                    <!-- 불량품 수량 -->
                                    <span class="oee-detail-item">Defective: <span id="defectiveProducts">-</span></span>
                                </div>
                            </div>
                            <!-- 품질률 요약 값 -->
                            <div class="oee-component-value"><span id="qualityDetail">-</span></div>
                        </div>
                    </div>
                </div>
            </div>

            <!-- 우: 차트 2개 -->
            <!-- OEE 구성 요소 차트 + OEE 등급 분포 차트 쌍 (3fr 너비) -->
            <div class="oee-charts-pair">
                <!-- OEE 구성 요소(가동률·성능률·품질률) 비교 차트 카드 -->
                <div class="fiori-card">
                    <div class="fiori-card__header">
                        <div class="card-title-row">
                            <h3 class="fiori-card__title">OEE Components</h3>
                            <span class="card-subtitle-inline">Availability, Performance, Quality</span>
                        </div>
                    </div>
                    <div class="fiori-card__content">
                        <!-- OEE 구성 요소 차트 캔버스 (Chart.js) -->
                        <div class="chart-container"><canvas id="oeeComponentChart"></canvas></div>
                    </div>
                </div>
                <!-- OEE 등급 분포 차트 카드 (성과 등급별 분류) -->
                <div class="fiori-card">
                    <div class="fiori-card__header">
                        <div class="card-title-row">
                            <h3 class="fiori-card__title">OEE Grade Distribution</h3>
                            <span class="card-subtitle-inline">By performance grade</span>
                        </div>
                    </div>
                    <div class="fiori-card__content">
                        <!-- OEE 등급 분포 차트 캔버스 -->
                        <div class="chart-container"><canvas id="oeeGradeChart"></canvas></div>
                    </div>
                </div>
            </div>

        </div>
    </div>

    <!-- Row C: Charts Bottom — 3 Trend 차트 (기본 hidden) -->
    <!-- 차트 하단 행: Show Charts 시 표시되는 추세·타임라인·라인별 성과 차트 3개 -->
    <div id="oeeRowChartsBottom" class="oee-row oee-row--charts-bottom hidden">
        <div class="oee-charts-trio">

            <!-- OEE 시간별 추세 차트 (AI 예측 점선 포함) -->
            <div class="fiori-card">
                <div class="fiori-card__header">
                    <div class="card-title-row">
                        <h3 class="fiori-card__title">
                            OEE Trend
                            <!-- AI 기반 예측이 포함된 차트임을 뱃지로 표시 -->
                            <span class="ai-badge">AI POWERED</span>
                        </h3>
                        <span class="card-subtitle-inline">
                            Hourly OEE trend &nbsp;&middot;&nbsp; dotted line = AI prediction
                            <!-- AI 트렌드 방향 뱃지 (JS에서 표시/숨김 전환) -->
                            &nbsp;<span id="aiOeeTrendBadge" class="ai-trend-badge ai-trend-badge--stable" style="display:none;"></span>
                        </span>
                    </div>
                </div>
                <div class="fiori-card__content">
                    <!-- OEE 추세 차트 캔버스 (실제값 + AI 예측 오버레이) -->
                    <div class="chart-container"><canvas id="oeeTrendChart"></canvas></div>
                </div>
            </div>

            <!-- OEE 시간대별 타임라인 차트 -->
            <div class="fiori-card">
                <div class="fiori-card__header">
                    <div class="card-title-row">
                        <h3 class="fiori-card__title">OEE Timeline</h3>
                        <span class="card-subtitle-inline">Hourly OEE timeline</span>
                    </div>
                </div>
                <div class="fiori-card__content">
                    <!-- OEE 타임라인 차트 캔버스 -->
                    <div class="chart-container"><canvas id="productionTrendChart"></canvas></div>
                </div>
            </div>

            <!-- 라인별 OEE 성과 비교 차트 (타이틀·서브타이틀은 JS에서 동적 업데이트) -->
            <div class="fiori-card" id="oeeLineOeeCard">
                <div class="fiori-card__header">
                    <div class="card-title-row">
                        <!-- 카드 타이틀: JS에서 조건에 따라 동적 변경 -->
                        <h3 class="fiori-card__title" id="oeeLineOeeCardTitle">Line OEE Performance</h3>
                        <!-- 카드 서브타이틀: JS에서 동적 변경 -->
                        <span class="card-subtitle-inline" id="oeeLineOeeCardSubtitle">OEE performance comparison by production line</span>
                    </div>
                </div>
                <div class="fiori-card__content">
                    <!-- 라인별 OEE 성과 비교 차트 캔버스 -->
                    <div class="chart-container"><canvas id="machineOeeChart"></canvas></div>
                </div>
            </div>

        </div>
    </div>

    <!-- Row D: Real-time OEE Table -->
    <!-- 실시간 OEE 데이터 테이블 행 (페이지 로드 시 기본 표시) -->
    <div id="oeeRowTable" class="oee-row oee-row--table">
        <div class="fiori-card">
            <div class="fiori-card__header">
                <h3 class="fiori-card__title">Real-time OEE Data</h3>
                <!-- 마지막 업데이트 시간 및 연결 상태 표시 -->
                <div class="real-time-status">
                    <div class="status-dot"></div>
                    <span id="lastUpdateTime">Last updated: -</span>
                    <span id="connectionStatus" class="connection-status-info">Connection ready...</span>
                </div>
            </div>
            <div class="fiori-card__content">
                <div class="oee-table-wrap">
                    <!-- OEE 데이터 테이블 -->
                    <table class="fiori-table" id="oeeDataTable">
                        <!-- 테이블 헤더: 기계번호·공장라인·교대·종합OEE·가동률·성능률·품질률·작업일·업데이트시간·액션 -->
                        <thead class="fiori-table__header">
                            <tr>
                                <th>Machine No</th>
                                <th>Factory/Line</th>
                                <th>Shift</th>
                                <th>Overall OEE</th>
                                <th>Availability</th>
                                <th>Performance</th>
                                <th>Quality</th>
                                <th>Work Date</th>
                                <th>Update Time</th>
                                <th>Action</th>
                            </tr>
                        </thead>
                        <!-- 테이블 바디: JS(data_oee_2.js)에서 동적으로 행 생성 -->
                        <tbody id="oeeDataBody">
                            <tr>
                                <td colspan="10" class="data-table-centered">
                                    <div class="fiori-alert fiori-alert--info">
                                        <strong>Information:</strong> Loading real-time OEE data. Automatic monitoring is in progress.
                                    </div>
                                </td>
                            </tr>
                        </tbody>
                    </table>
                </div>
            </div>
        </div>
    </div>

    <!-- Row E: Pagination -->
    <!-- 페이지네이션 컨트롤 행 (JS에서 동적으로 버튼 생성) -->
    <div id="oeeRowPagination" class="oee-row oee-row--pagination">
        <div id="pagination-controls" class="fiori-pagination"></div>
    </div>

</div><!-- /oee-signage-main -->


<!-- OEE Detail Modal -->
<!-- OEE 상세 정보 팝업 모달: 테이블에서 Action 버튼 클릭 시 표시 -->
<div id="oeeDetailModal" class="fiori-modal">
    <!-- 배경(backdrop) 클릭 시 모달 닫기 -->
    <div class="fiori-modal__backdrop" onclick="closeOeeDetailModal()"></div>
    <div class="fiori-modal__content">
        <div class="fiori-card">
            <div class="fiori-card__header">
                <h3 class="fiori-card__title">OEE Details</h3>
                <!-- X 버튼으로 모달 닫기 -->
                <button class="fiori-btn fiori-btn--icon" onclick="closeOeeDetailModal()"><span>&#10005;</span></button>
            </div>
            <div class="fiori-card__content">
                <!-- 모달 상세 정보 그리드: 기본정보·OEE성과·시간생산정보·추가정보 4섹션 -->
                <div class="oee-detail-grid">
                    <!-- 기본 정보 섹션: 기계번호, 공장/라인, 작업일, 교대 -->
                    <div class="oee-detail-section">
                        <h4 class="oee-detail-section-title">Basic Information</h4>
                        <div class="oee-detail-row"><span class="oee-detail-label">Machine Number:</span><span class="oee-detail-value" id="modal-machine-no">-</span></div>
                        <div class="oee-detail-row"><span class="oee-detail-label">Factory/Line:</span><span class="oee-detail-value" id="modal-factory-line">-</span></div>
                        <div class="oee-detail-row"><span class="oee-detail-label">Work Date:</span><span class="oee-detail-value" id="modal-work-date">-</span></div>
                        <div class="oee-detail-row"><span class="oee-detail-label">Shift:</span><span class="oee-detail-value" id="modal-shift">-</span></div>
                    </div>
                    <!-- OEE 성과 섹션: 종합OEE·가동률·성능률·품질률 -->
                    <div class="oee-detail-section">
                        <h4 class="oee-detail-section-title">OEE Performance</h4>
                        <div class="oee-detail-row"><span class="oee-detail-label">Overall OEE:</span><span class="oee-detail-value" id="modal-overall-oee">-</span></div>
                        <div class="oee-detail-row"><span class="oee-detail-label">Availability:</span><span class="oee-detail-value" id="modal-availability">-</span></div>
                        <div class="oee-detail-row"><span class="oee-detail-label">Performance:</span><span class="oee-detail-value" id="modal-performance">-</span></div>
                        <div class="oee-detail-row"><span class="oee-detail-label">Quality:</span><span class="oee-detail-value" id="modal-quality">-</span></div>
                    </div>
                    <!-- 시간 및 생산 정보 섹션: 계획 시간·런타임·다운타임·실제 생산량 -->
                    <div class="oee-detail-section">
                        <h4 class="oee-detail-section-title">Time &amp; Production</h4>
                        <div class="oee-detail-row"><span class="oee-detail-label">Planned Work Time:</span><span class="oee-detail-value" id="modal-planned-time">-</span></div>
                        <div class="oee-detail-row"><span class="oee-detail-label">Runtime:</span><span class="oee-detail-value" id="modal-runtime">-</span></div>
                        <div class="oee-detail-row"><span class="oee-detail-label">Downtime:</span><span class="oee-detail-value" id="modal-downtime">-</span></div>
                        <div class="oee-detail-row"><span class="oee-detail-label">Actual Output:</span><span class="oee-detail-value" id="modal-actual-output">-</span></div>
                    </div>
                    <!-- 추가 정보 섹션: 이론 생산량·불량 수·사이클 타임·업데이트 시간 (전체 너비) -->
                    <div class="oee-detail-section oee-detail-section--full">
                        <h4 class="oee-detail-section-title">Additional Information</h4>
                        <div class="oee-detail-row"><span class="oee-detail-label">Theoretical Output:</span><span class="oee-detail-value" id="modal-theoretical-output">-</span></div>
                        <div class="oee-detail-row"><span class="oee-detail-label">Defective Count:</span><span class="oee-detail-value" id="modal-defective">-</span></div>
                        <div class="oee-detail-row"><span class="oee-detail-label">Cycle Time:</span><span class="oee-detail-value" id="modal-cycletime">-</span></div>
                        <div class="oee-detail-row"><span class="oee-detail-label">Update Time:</span><span class="oee-detail-value" id="modal-update-time">-</span></div>
                    </div>
                </div>
            </div>
            <!-- 모달 푸터: 닫기 및 단건 내보내기 버튼 -->
            <div class="fiori-card__footer">
                <div class="oee-detail-actions">
                    <!-- 모달 닫기 버튼 -->
                    <button class="fiori-btn fiori-btn--secondary" onclick="closeOeeDetailModal()">Close</button>
                    <!-- 현재 OEE 단건 내보내기 버튼 (JS: exportSingleOee()) -->
                    <button class="fiori-btn fiori-btn--primary" onclick="exportSingleOee()">Export</button>
                </div>
            </div>
        </div>
    </div>
</div>


<!-- 외부 라이브러리: Chart.js, jQuery, Moment.js, DateRangePicker -->
<script src="../../assets/js/chart.js"></script>
<script src="../../assets/js/jquery-3.6.1.min.js"></script>
<script src="../../assets/js/moment.min.js"></script>
<script src="../../assets/js/daterangepicker.js"></script>
<!-- OEE 데이터 모니터링 메인 JS (데이터 로딩, 차트, 테이블, 페이지네이션 처리) -->
<script src="js/data_oee_2.js"></script>
<!-- AI OEE 예측 오버레이 JS (OEE 추세 차트에 AI 예측 점선 렌더링) -->
<script src="js/ai_oee_overlay_2.js"></script>


</body>

</html>
