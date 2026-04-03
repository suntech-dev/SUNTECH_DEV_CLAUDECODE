<?php
/*
 * ============================================================
 * 파일명  : data_defective_2.php
 * 목  적  : 불량(Defective) 데이터 모니터링 페이지
 *           실시간 불량 데이터 테이블, 통계 카드, 분석 차트를
 *           사이니지 레이아웃으로 표시한다.
 * 레이아웃 구성:
 *   - Row A: 통계 카드 6개 (기본 숨김, Show Stats 버튼으로 토글)
 *   - Row B: 현재 활성 불량 목록 + 불량 유형/상태 분석 차트 2개 (기본 숨김)
 *   - Row C: 불량 발생 추세·기계별·라인별 차트 3개 (기본 숨김)
 *   - Row D: 실시간 불량 데이터 테이블 (기본 표시)
 *   - Row E: 페이지네이션 컨트롤
 * 연관 파일:
 *   - css/data_defective_2.css   : 불량 모니터링 전용 스타일
 *   - js/data_defective_2.js     : 불량 데이터 로딩·렌더링·필터 로직
 *   - inc/signage_filters.php    : 공장·라인·기계·날짜 필터 HTML 조각
 * ============================================================
 */

// 브라우저 탭 및 페이지 제목 설정
$page_title = 'Defective Data Monitoring';

// 페이지에서 사용할 CSS 파일 목록
$page_css_files = [
    '../../assets/css/fiori-page.css',       // Fiori 공통 페이지 스타일
    '../../assets/css/daterangepicker.css',   // 날짜 범위 선택기 스타일
    'css/data_defective_2.css',               // 불량 모니터링 전용 스타일
];

// 공통 헤드 파일 포함 (HTML head 태그, CSS 로드, meta 태그 등)
require_once(__DIR__ . '/../../inc/head.php');
?>

<?php
// 네비게이션 컨텍스트 및 활성 메뉴 설정
$nav_context = 'data';
$nav_active = 'defective_m'; // 네비게이션에서 'defective_m' 메뉴 항목을 활성화
// 사이드 드로어(네비게이션 메뉴) 포함
require_once(__DIR__ . '/../../inc/nav-drawer-manage.php'); ?>

<!-- Signage Header -->
<!-- 사이니지 상단 헤더: 메뉴 버튼, 타이틀, 필터 및 제어 버튼들 -->
<div class="signage-header">
    <!-- 좌측 햄버거 메뉴 버튼 (네비게이션 드로어 토글) -->
    <button id="navDrawerBtn" class="nav-drawer-btn" aria-label="Menu">&#9776;</button>
    <span class="signage-header__title">Defective Monitoring</span>

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

<!-- Defective Signage Main -->
<!-- 불량 모니터링 메인 컨텐츠 영역 -->
<div class="defective-signage-main" id="defectiveSignageMain">

    <!-- Row A: Stats (기본 hidden) -->
    <!-- 통계 카드 행: Show Stats 버튼 클릭 시 표시됨 -->
    <div id="defectiveRowStats" class="defective-row defective-row--stats hidden">
        <div class="defective-stats-grid">
            <!-- 현재 활성(경보 중) 불량 건수 카드 (빨간색) -->
            <div class="stat-card stat-card--red">
                <div class="stat-value" id="activeDefectives">-</div>
                <div class="stat-label">Active Defectives</div>
            </div>
            <!-- 현재 교대(Shift) 불량 건수 카드 (파란색) -->
            <div class="stat-card stat-card--info">
                <div class="stat-value" id="currentShiftDefective">-</div>
                <div class="stat-label">Current Shift Defective</div>
            </div>
            <!-- 불량으로 영향 받은 기계 수 카드 (주황색) -->
            <div class="stat-card stat-card--warning">
                <div class="stat-value" id="affectedMachinesDefective">-</div>
                <div class="stat-label">Affected Machines</div>
            </div>
            <!-- 불량률(%) 카드 (마룬색) -->
            <div class="stat-card stat-card--maroon">
                <div class="stat-value" id="defectiveRate">-</div>
                <div class="stat-label">Defective Rate (%)</div>
            </div>
            <!-- 품질률(%) 카드 (초록색) -->
            <div class="stat-card stat-card--success">
                <div class="stat-value" id="qualityScore">-</div>
                <div class="stat-label">Quality Rate (%)</div>
            </div>
            <!-- 전체 불량 건수 카드 (기본 스타일) -->
            <div class="stat-card">
                <div class="stat-value" id="totalDefectiveCount">-</div>
                <div class="stat-label">Total Count</div>
            </div>
        </div>
    </div>

    <!-- Row B: Charts Top — Active Defectives(2fr) + 차트 2개(3fr) (기본 hidden) -->
    <!-- 차트 상단 행: Show Charts 버튼 클릭 시 표시됨 -->
    <div id="defectiveRowChartsTop" class="defective-row defective-row--charts-top hidden">
        <div class="defective-charts-top-grid">

            <!-- 좌: Currently Active Defectives -->
            <!-- 현재 경보 중인 불량 목록 카드 (2fr 너비) -->
            <div class="fiori-card">
                <div class="fiori-card__header">
                    <div class="card-title-row">
                        <h3 class="fiori-card__title">Currently Active Defectives</h3>
                    </div>
                    <!-- 실시간 모니터링 상태 표시 -->
                    <div class="real-time-status">
                        <div class="status-dot"></div>
                        <!-- 활성 불량 건수 표시 (JS에서 업데이트) -->
                        <span id="activeDefectiveCount">0 active defectives</span>
                    </div>
                </div>
                <div class="fiori-card__content">
                    <!-- 활성 불량 항목 목록 (JS에서 동적으로 렌더링) -->
                    <div class="defective-active-list" id="activeDefectivesContainer">
                        <!-- 기본 상태: 활성 불량 없음 메시지 -->
                        <div class="fiori-alert fiori-alert--info">
                            <strong>Information:</strong> There are currently no active Defectives.
                        </div>
                    </div>
                </div>
            </div>

            <!-- 우: 차트 2개 -->
            <!-- 불량 유형 분석 차트 + 불량 상태 분포 차트 쌍 (3fr 너비) -->
            <div class="defective-charts-pair">
                <!-- 불량 유형별 발생 빈도 분석 차트 카드 -->
                <div class="fiori-card">
                    <div class="fiori-card__header">
                        <div class="card-title-row">
                            <h3 class="fiori-card__title">Defective Type Analysis</h3>
                            <span class="card-subtitle-inline">Frequency by defective type</span>
                        </div>
                    </div>
                    <div class="fiori-card__content">
                        <!-- 불량 유형별 차트 캔버스 (Chart.js) -->
                        <div class="chart-container"><canvas id="defectiveTypeChart"></canvas></div>
                    </div>
                </div>
                <!-- 불량 상태 분포 차트 카드 (경보 중 vs 처리 완료) -->
                <div class="fiori-card">
                    <div class="fiori-card__header">
                        <div class="card-title-row">
                            <h3 class="fiori-card__title">Defective Status Distribution</h3>
                            <span class="card-subtitle-inline">Warning vs Completed</span>
                        </div>
                    </div>
                    <div class="fiori-card__content">
                        <!-- 불량 상태 분포 차트 캔버스 -->
                        <div class="chart-container"><canvas id="defectiveStatusChart"></canvas></div>
                    </div>
                </div>
            </div>

        </div>
    </div>

    <!-- Row C: Charts Bottom — 3 Trend 차트 (기본 hidden) -->
    <!-- 차트 하단 행: Show Charts 시 표시되는 트렌드·비교 차트 3개 -->
    <div id="defectiveRowChartsBottom" class="defective-row defective-row--charts-bottom hidden">
        <div class="defective-charts-trio">

            <!-- 불량 발생 건수 시간별/일별 추세 차트 -->
            <div class="fiori-card">
                <div class="fiori-card__header">
                    <div class="card-title-row">
                        <h3 class="fiori-card__title">Defective Count Trend</h3>
                        <span class="card-subtitle-inline">Hourly/Daily defective occurrence</span>
                    </div>
                </div>
                <div class="fiori-card__content">
                    <!-- 불량 추세 차트 캔버스 -->
                    <div class="chart-container"><canvas id="defectiveTrendChart"></canvas></div>
                </div>
            </div>

            <!-- 기계별 불량 건수 비교 차트 -->
            <div class="fiori-card">
                <div class="fiori-card__header">
                    <div class="card-title-row">
                        <h3 class="fiori-card__title">Machine Defective Comparison</h3>
                        <span class="card-subtitle-inline">Defective count by machine</span>
                    </div>
                </div>
                <div class="fiori-card__content">
                    <!-- 기계별 불량 비교 차트 캔버스 -->
                    <div class="chart-container"><canvas id="defectiveMachineChart"></canvas></div>
                </div>
            </div>

            <!-- 라인별 불량 성과 비교 차트 (타이틀·서브타이틀은 JS에서 동적 업데이트) -->
            <div class="fiori-card" id="defectiveLineCard">
                <div class="fiori-card__header">
                    <div class="card-title-row">
                        <!-- 카드 타이틀: JS에서 라인명/조건에 따라 동적 변경 -->
                        <h3 class="fiori-card__title" id="defectiveLineCardTitle">Line Defective Performance</h3>
                        <!-- 카드 서브타이틀: JS에서 동적 변경 -->
                        <span class="card-subtitle-inline" id="defectiveLineCardSubtitle">Defective count comparison by production line</span>
                    </div>
                </div>
                <div class="fiori-card__content">
                    <!-- 라인별 불량 성과 차트 캔버스 -->
                    <div class="chart-container"><canvas id="defectiveLineChart"></canvas></div>
                </div>
            </div>

        </div>
    </div>

    <!-- Row D: Real-time Defective Table -->
    <!-- 실시간 불량 데이터 테이블 행 (페이지 로드 시 기본 표시) -->
    <div id="defectiveRowTable" class="defective-row defective-row--table">
        <div class="fiori-card">
            <div class="fiori-card__header">
                <h3 class="fiori-card__title">Real-time Defective Data</h3>
                <!-- 마지막 업데이트 시간 및 연결 상태 표시 -->
                <div class="real-time-status">
                    <div class="status-dot"></div>
                    <span id="lastUpdateTime">Last updated: -</span>
                    <span id="connectionStatus" class="connection-status-info">Connection ready...</span>
                </div>
            </div>
            <div class="fiori-card__content">
                <div class="defective-table-wrap">
                    <!-- 불량 데이터 테이블 -->
                    <table class="fiori-table" id="defectiveDataTable">
                        <!-- 테이블 헤더: 기계번호·공장라인·교대·불량유형·발생시간·경과시간·작업일·상세 -->
                        <thead class="fiori-table__header">
                            <tr>
                                <th>Machine No</th>
                                <th>Factory/Line</th>
                                <th>Shift</th>
                                <th>Defective Type</th>
                                <th>Occurrence Time</th>
                                <th>Elapsed Time</th>
                                <th>Work Date</th>
                                <th>Detail</th>
                            </tr>
                        </thead>
                        <!-- 테이블 바디: JS(data_defective_2.js)에서 동적으로 행 생성 -->
                        <tbody id="defectiveDataBody">
                            <tr>
                                <td colspan="8" class="data-table-centered">
                                    <div class="fiori-alert fiori-alert--info">
                                        <strong>Information:</strong> Loading real-time Defective data. Automatic monitoring is in progress.
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
    <div id="defectiveRowPagination" class="defective-row defective-row--pagination">
        <div id="pagination-controls" class="fiori-pagination"></div>
    </div>

</div><!-- /defective-signage-main -->


<!-- Defective Detail Modal -->
<!-- 불량 상세 정보 팝업 모달: 테이블에서 Detail 버튼 클릭 시 표시 -->
<div id="defectiveDetailModal" class="fiori-modal">
    <!-- 배경(backdrop) 클릭 시 모달 닫기 -->
    <div class="fiori-modal__backdrop" onclick="closeDefectiveDetailModal()"></div>
    <div class="fiori-modal__content">
        <div class="fiori-card">
            <div class="fiori-card__header">
                <h3 class="fiori-card__title">Defective Details</h3>
                <!-- X 버튼으로 모달 닫기 -->
                <button class="fiori-btn fiori-btn--icon" onclick="closeDefectiveDetailModal()"><span>&#10005;</span></button>
            </div>
            <div class="fiori-card__content">
                <!-- 모달 상세 정보 그리드: 기본정보·시간정보·작업정보·추가정보 4섹션 -->
                <div class="defective-detail-grid">
                    <!-- 기본 정보 섹션: 기계번호, 공장/라인, 불량유형, 상태 -->
                    <div class="defective-detail-section">
                        <h4 class="defective-detail-section-title">Basic Information</h4>
                        <div class="defective-detail-row"><span class="defective-detail-label">Machine Number:</span><span class="defective-detail-value" id="modal-machine-no">-</span></div>
                        <div class="defective-detail-row"><span class="defective-detail-label">Factory/Line:</span><span class="defective-detail-value" id="modal-factory-line">-</span></div>
                        <div class="defective-detail-row"><span class="defective-detail-label">Defective Type:</span><span class="defective-detail-value" id="modal-defective-type">-</span></div>
                        <div class="defective-detail-row"><span class="defective-detail-label">Status:</span><span class="defective-detail-value" id="modal-status">-</span></div>
                    </div>
                    <!-- 시간 정보 섹션: 발생시간, 경과시간, 작업일 -->
                    <div class="defective-detail-section">
                        <h4 class="defective-detail-section-title">Time Information</h4>
                        <div class="defective-detail-row"><span class="defective-detail-label">Occurrence Time:</span><span class="defective-detail-value" id="modal-reg-date">-</span></div>
                        <div class="defective-detail-row"><span class="defective-detail-label">Elapsed Time:</span><span class="defective-detail-value" id="modal-elapsed-time">-</span></div>
                        <div class="defective-detail-row"><span class="defective-detail-label">Work Date:</span><span class="defective-detail-value" id="modal-work-date">-</span></div>
                    </div>
                    <!-- 작업 정보 섹션: 교대(Shift), 불량 색상 표시 -->
                    <div class="defective-detail-section">
                        <h4 class="defective-detail-section-title">Work Information</h4>
                        <div class="defective-detail-row"><span class="defective-detail-label">Shift:</span><span class="defective-detail-value" id="modal-shift">-</span></div>
                        <div class="defective-detail-row">
                            <span class="defective-detail-label">Defective Color:</span>
                            <span class="defective-detail-value" id="modal-defective-color">
                                <!-- 불량 색상 인디케이터 및 색상명 표시 -->
                                <span class="defective-color-indicator" id="modal-color-indicator"></span>
                                <span id="modal-color-value">Default Color</span>
                            </span>
                        </div>
                    </div>
                    <!-- 추가 정보 섹션: DB ID, 등록일시 (전체 너비) -->
                    <div class="defective-detail-section defective-detail-section--full">
                        <h4 class="defective-detail-section-title">Additional Information</h4>
                        <div class="defective-detail-row"><span class="defective-detail-label">Database ID:</span><span class="defective-detail-value" id="modal-idx">-</span></div>
                        <div class="defective-detail-row"><span class="defective-detail-label">Registration Date:</span><span class="defective-detail-value" id="modal-created-at">-</span></div>
                    </div>
                </div>
            </div>
            <!-- 모달 푸터: 닫기 및 단건 내보내기 버튼 -->
            <div class="fiori-card__footer">
                <div class="defective-detail-actions">
                    <!-- 모달 닫기 버튼 -->
                    <button class="fiori-btn fiori-btn--secondary" onclick="closeDefectiveDetailModal()">Close</button>
                    <!-- 현재 불량 단건 내보내기 버튼 (JS: exportSingleDefective()) -->
                    <button class="fiori-btn fiori-btn--primary" onclick="exportSingleDefective()">Export</button>
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
<!-- 불량 데이터 모니터링 메인 JS (데이터 로딩, 차트, 테이블, 페이지네이션 처리) -->
<script src="js/data_defective_2.js"></script>


</body>

</html>
