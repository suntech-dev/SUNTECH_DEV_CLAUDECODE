<?php
/*
 * ============================================================
 * 파일명  : data_andon_2.php
 * 목  적  : 안돈(Andon) 데이터 모니터링 페이지
 *           실시간 안돈 경보 데이터 테이블, 통계 카드, 분석 차트를
 *           사이니지 레이아웃으로 표시한다.
 * 레이아웃 구성:
 *   - Row A: 통계 카드 6개 (기본 숨김, Show Stats 버튼으로 토글)
 *   - Row B: 현재 활성 안돈 목록 + 안돈 유형 분석 차트 (기본 숨김)
 *   - Row C: 안돈 발생 추세 차트 (기본 숨김)
 *   - Row D: 실시간 안돈 데이터 테이블 (기본 표시)
 *   - Row E: 페이지네이션 컨트롤
 * 연관 파일:
 *   - css/data_andon_2.css      : 안돈 모니터링 전용 스타일
 *   - js/data_andon_2.js        : 안돈 데이터 로딩·렌더링·필터 로직
 *   - inc/signage_filters.php   : 공장·라인·기계·날짜 필터 HTML 조각
 * ============================================================
 */

// 브라우저 탭 및 페이지 제목 설정
$page_title = 'Andon Data Monitoring';

// 페이지에서 사용할 CSS 파일 목록
$page_css_files = [
    '../../assets/css/fiori-page.css',       // Fiori 공통 페이지 스타일
    '../../assets/css/daterangepicker.css',   // 날짜 범위 선택기 스타일
    'css/data_andon_2.css',                    // 안돈 모니터링 전용 스타일
];

// 공통 헤드 파일 포함 (HTML head 태그, CSS 로드 등)
require_once(__DIR__ . '/../../inc/head.php');
?>

<?php
// 네비게이션 컨텍스트 및 활성 메뉴 설정
$nav_context = 'data';
$nav_active = 'andon_m'; // 네비게이션에서 'andon_m' 메뉴 항목을 활성화
// 사이드 드로어(네비게이션 메뉴) 포함
require_once(__DIR__ . '/../../inc/nav-drawer-manage.php'); ?>

<!-- Signage Header -->
<!-- 사이니지 상단 헤더: 메뉴 버튼, 타이틀, 필터 및 제어 버튼들 -->
<div class="signage-header">
    <!-- 좌측 햄버거 메뉴 버튼 -->
    <button id="navDrawerBtn" class="nav-drawer-btn" aria-label="Menu">&#9776;</button>
    <span class="signage-header__title">Andon Monitoring</span>

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

<!-- Andon Signage Main -->
<!-- 안돈 모니터링 메인 컨텐츠 영역 -->
<div class="andon-signage-main" id="andonSignageMain">

    <!-- Row A: Stats (기본 hidden) -->
    <!-- 통계 카드 행: Show Stats 버튼 클릭 시 표시됨 -->
    <div id="andonRowStats" class="andon-row andon-row--stats hidden">
        <div class="andon-stats-grid">
            <!-- 전체 안돈 발생 건수 카드 (빨간색) -->
            <div class="stat-card stat-card--red">
                <div class="stat-value" id="totalAndons">-</div>
                <div class="stat-label">Total Andon Count</div>
            </div>
            <!-- 현재 활성(경보 중) 안돈 건수 카드 (주황색) -->
            <div class="stat-card stat-card--warning">
                <div class="stat-value" id="activeWarnings">-</div>
                <div class="stat-label">Active Warnings</div>
            </div>
            <!-- 현재 교대(Shift) 안돈 건수 카드 (파란색) -->
            <div class="stat-card stat-card--info">
                <div class="stat-value" id="currentShiftCount">-</div>
                <div class="stat-label">Current Shift Count</div>
            </div>
            <!-- 영향 받은 기계 수 카드 (마룬색) -->
            <div class="stat-card stat-card--maroon">
                <div class="stat-value" id="affectedMachines">-</div>
                <div class="stat-label">Affected Machine Count</div>
            </div>
            <!-- 5분 이상 미처리 안돈 건수 카드 (빨간색) -->
            <div class="stat-card stat-card--red">
                <div class="stat-value" id="urgentWarnings">-</div>
                <div class="stat-label">Unresolved Over 5min</div>
            </div>
            <!-- 평균 처리 완료 시간 카드 (초록색) -->
            <div class="stat-card stat-card--success">
                <div class="stat-value" id="avgCompletedTime">-</div>
                <div class="stat-label">Avg Completion Time</div>
            </div>
        </div>
    </div>

    <!-- Row B: Charts Top — Active Andons(2fr) + Andon Type Chart(3fr) (기본 hidden) -->
    <!-- 차트 상단 행: Show Charts 버튼 클릭 시 표시됨 -->
    <div id="andonRowChartsTop" class="andon-row andon-row--charts-top hidden">
        <div class="andon-charts-top-grid">

            <!-- 좌: Currently Active Andons -->
            <!-- 현재 경보 중인 안돈 목록 카드 (2fr 너비) -->
            <div class="fiori-card">
                <div class="fiori-card__header">
                    <div class="card-title-row">
                        <h3 class="fiori-card__title">Currently Active Andon</h3>
                        <!-- 활성 안돈 건수 표시 (JS에서 업데이트) -->
                        <span class="card-subtitle-inline" id="activeAndonCount">0 active alerts</span>
                    </div>
                    <!-- 실시간 모니터링 상태 표시 -->
                    <div class="real-time-status">
                        <div class="status-dot"></div>
                        <span id="oeeLiveStatus">Real-time monitoring active</span>
                    </div>
                </div>
                <div class="fiori-card__content">
                    <!-- 활성 안돈 항목 목록 (JS에서 동적으로 렌더링) -->
                    <div class="andon-active-list" id="activeAndonsContainer">
                        <div class="fiori-alert fiori-alert--info">
                            <strong>Information:</strong> No active Andon. Real-time monitoring active.
                        </div>
                    </div>
                </div>
            </div>

            <!-- 우: Andon Type Analysis Chart -->
            <!-- 안돈 유형별 발생 분석 차트 카드 (3fr 너비) -->
            <div class="fiori-card">
                <div class="fiori-card__header">
                    <div class="card-title-row">
                        <h3 class="fiori-card__title">Analysis by Andon Type</h3>
                    </div>
                </div>
                <div class="fiori-card__content">
                    <!-- 안돈 유형 분석 차트 캔버스 (Chart.js) -->
                    <div class="chart-container">
                        <canvas id="andonTypeChart"></canvas>
                    </div>
                </div>
            </div>

        </div>
    </div>

    <!-- Row C: Charts Bottom — Andon Trend (기본 hidden) -->
    <!-- 차트 하단 행: 안돈 발생 추세 차트 (Show Charts 시 표시) -->
    <div id="andonRowChartsBottom" class="andon-row andon-row--charts-bottom hidden">
        <div class="fiori-card">
            <div class="fiori-card__header">
                <div class="card-title-row">
                    <h3 class="fiori-card__title">Andon Occurrence Trend</h3>
                </div>
            </div>
            <div class="fiori-card__content">
                <!-- 안돈 발생 추세 차트 캔버스 (시간대별·일별 추세) -->
                <div class="chart-container">
                    <canvas id="andonTrendChart"></canvas>
                </div>
            </div>
        </div>
    </div>

    <!-- Row D: Table (기본 표시) -->
    <!-- 실시간 안돈 데이터 테이블 행 (페이지 로드 시 기본 표시) -->
    <div id="andonRowTable" class="andon-row andon-row--table">
        <div class="fiori-card">
            <div class="fiori-card__header">
                <div class="card-title-row">
                    <h3 class="fiori-card__title">Real-time Andon Data</h3>
                </div>
                <!-- 마지막 업데이트 시간 및 연결 상태 표시 -->
                <div class="real-time-status">
                    <div class="status-dot"></div>
                    <span id="lastUpdateTime">Last updated: -</span>
                    <span id="connectionStatus" class="connection-status-info">Connection ready...</span>
                </div>
            </div>
            <div class="fiori-card__content">
                <div class="andon-table-wrap">
                    <!-- 안돈 데이터 테이블 -->
                    <table class="fiori-table" id="andonDataTable">
                        <!-- 테이블 헤더: 기계번호·공장라인·교대·안돈유형·상태·발생시간·처리시간·소요시간·작업일·상세 -->
                        <thead class="fiori-table__header">
                            <tr>
                                <th>Machine No</th>
                                <th>Factory/Line</th>
                                <th>Shift</th>
                                <th>Andon Type</th>
                                <th>Status</th>
                                <th>Occurrence Time</th>
                                <th>Resolution Time</th>
                                <th>Duration</th>
                                <th>Work Date</th>
                                <th>DETAIL</th>
                            </tr>
                        </thead>
                        <!-- 테이블 바디: JS(data_andon_2.js)에서 동적으로 행 생성 -->
                        <tbody id="andonDataBody">
                            <tr>
                                <td colspan="10" class="data-table-centered">
                                    <div class="fiori-alert fiori-alert--info">
                                        <strong>Information:</strong> Loading real-time Andon data.
                                    </div>
                                </td>
                            </tr>
                        </tbody>
                    </table>
                </div>
            </div>
        </div>
    </div>

    <!-- Row E: Pagination (항상 auto) -->
    <!-- 페이지네이션 컨트롤 행 (JS에서 동적으로 버튼 생성) -->
    <div id="andonRowPagination" class="andon-row">
        <div id="pagination-controls" class="fiori-pagination"></div>
    </div>

</div>

<!-- Andon Detail Modal -->
<!-- 안돈 상세 정보 팝업 모달: 테이블에서 DETAIL 버튼 클릭 시 표시 -->
<div id="andonDetailModal" class="fiori-modal">
    <!-- 배경(backdrop) 클릭 시 모달 닫기 -->
    <div class="fiori-modal__backdrop" onclick="closeAndonDetailModal()"></div>
    <div class="fiori-modal__content">
        <div class="fiori-card">
            <div class="fiori-card__header">
                <h3 class="fiori-card__title">Andon Details</h3>
                <!-- X 버튼으로 모달 닫기 -->
                <button class="fiori-btn fiori-btn--icon" onclick="closeAndonDetailModal()"><span>&#x2715;</span></button>
            </div>
            <div class="fiori-card__content">
                <!-- 모달 상세 정보 그리드: 기본정보·시간정보·작업정보·추가정보 4섹션 -->
                <div class="andon-detail-grid">
                    <!-- 기본 정보 섹션: 기계번호, 공장/라인, 안돈유형, 상태 -->
                    <div class="andon-detail-section">
                        <h4 class="andon-detail-section-title">Basic Information</h4>
                        <div class="andon-detail-row"><span class="andon-detail-label">Machine Number:</span><span class="andon-detail-value" id="modal-machine-no">-</span></div>
                        <div class="andon-detail-row"><span class="andon-detail-label">Factory/Line:</span><span class="andon-detail-value" id="modal-factory-line">-</span></div>
                        <div class="andon-detail-row"><span class="andon-detail-label">Andon Type:</span><span class="andon-detail-value" id="modal-andon-type">-</span></div>
                        <div class="andon-detail-row"><span class="andon-detail-label">Status:</span><span class="andon-detail-value" id="modal-status">-</span></div>
                    </div>
                    <!-- 시간 정보 섹션: 발생시간, 처리시간, 소요시간, 작업일 -->
                    <div class="andon-detail-section">
                        <h4 class="andon-detail-section-title">Time Information</h4>
                        <div class="andon-detail-row"><span class="andon-detail-label">Occurrence Time:</span><span class="andon-detail-value" id="modal-reg-date">-</span></div>
                        <div class="andon-detail-row"><span class="andon-detail-label">Resolution Time:</span><span class="andon-detail-value" id="modal-update-date">-</span></div>
                        <div class="andon-detail-row"><span class="andon-detail-label">Duration:</span><span class="andon-detail-value" id="modal-duration">-</span></div>
                        <div class="andon-detail-row"><span class="andon-detail-label">Work Date:</span><span class="andon-detail-value" id="modal-work-date">-</span></div>
                    </div>
                    <!-- 작업 정보 섹션: 교대(Shift), 안돈 색상 표시 -->
                    <div class="andon-detail-section">
                        <h4 class="andon-detail-section-title">Work Information</h4>
                        <div class="andon-detail-row"><span class="andon-detail-label">Shift:</span><span class="andon-detail-value" id="modal-shift">-</span></div>
                        <div class="andon-detail-row">
                            <span class="andon-detail-label">Andon Color:</span>
                            <span class="andon-detail-value" id="modal-andon-color">
                                <!-- 안돈 색상 인디케이터 및 색상명 표시 -->
                                <span class="andon-color-indicator" id="modal-color-indicator"></span>
                                <span id="modal-color-value">Default Color</span>
                            </span>
                        </div>
                    </div>
                    <!-- 추가 정보 섹션: DB ID, 등록일시 (전체 너비) -->
                    <div class="andon-detail-section andon-detail-section--full">
                        <h4 class="andon-detail-section-title">Additional Information</h4>
                        <div class="andon-detail-row"><span class="andon-detail-label">Database ID:</span><span class="andon-detail-value" id="modal-idx">-</span></div>
                        <div class="andon-detail-row"><span class="andon-detail-label">Registration Date:</span><span class="andon-detail-value" id="modal-created-at">-</span></div>
                    </div>
                </div>
            </div>
            <!-- 모달 푸터: 닫기 및 단건 내보내기 버튼 -->
            <div class="fiori-card__footer">
                <div class="andon-detail-actions">
                    <!-- 모달 닫기 버튼 -->
                    <button class="fiori-btn fiori-btn--secondary" onclick="closeAndonDetailModal()">Close</button>
                    <!-- 현재 안돈 단건 내보내기 버튼 (JS: exportSingleAndon()) -->
                    <button class="fiori-btn fiori-btn--primary" onclick="exportSingleAndon()">Export</button>
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
<!-- 안돈 데이터 모니터링 메인 JS (데이터 로딩, 차트, 테이블, 페이지네이션 처리) -->
<script src="js/data_andon_2.js"></script>

</body>

</html>
