<?php
/*
 * ============================================================
 * 파일명  : data_downtime_2.php
 * 목  적  : 다운타임(Downtime) 데이터 모니터링 페이지
 *           실시간 다운타임 데이터 테이블, 통계 카드, 분석 차트를
 *           사이니지 레이아웃으로 표시한다.
 * 레이아웃 구성:
 *   - Row A: 통계 카드 6개 (기본 숨김, Show Stats 버튼으로 토글)
 *   - Row B: 다운타임 요약 상세 + 유형/상태 분석 차트 2개 (기본 숨김)
 *   - Row C: 다운타임 추세·라인별·지속시간 분포 차트 3개 (기본 숨김)
 *   - Row D: 실시간 다운타임 데이터 테이블 (기본 표시)
 *   - Row E: 페이지네이션 컨트롤
 * 특징:
 *   - AI Risk 컬럼: AI 위험 점수를 ai_downtime_risk_2.js가 표 안에 표시
 * 연관 파일:
 *   - css/data_downtime_2.css    : 다운타임 모니터링 전용 스타일
 *   - js/data_downtime_2.js      : 다운타임 데이터 로딩·렌더링·필터 로직
 *   - js/ai_downtime_risk_2.js   : AI 기반 다운타임 위험 점수 렌더링 로직
 *   - inc/signage_filters.php    : 공장·라인·기계·날짜 필터 HTML 조각
 * ============================================================
 */

// 브라우저 탭 및 페이지 제목 설정
$page_title = 'Downtime Data Monitoring';

// 페이지에서 사용할 CSS 파일 목록
$page_css_files = [
    '../../assets/css/fiori-page.css',       // Fiori 공통 페이지 스타일
    '../../assets/css/daterangepicker.css',   // 날짜 범위 선택기 스타일
    'css/data_downtime_2.css',                // 다운타임 모니터링 전용 스타일
];

// 공통 헤드 파일 포함 (HTML head 태그, CSS 로드, meta 태그 등)
require_once(__DIR__ . '/../../inc/head.php');
?>

<?php
// 네비게이션 컨텍스트 및 활성 메뉴 설정
$nav_context = 'data';
$nav_active = 'downtime_m'; // 네비게이션에서 'downtime_m' 메뉴 항목을 활성화
// 사이드 드로어(네비게이션 메뉴) 포함
require_once(__DIR__ . '/../../inc/nav-drawer-manage.php'); ?>

<!-- Signage Header -->
<!-- 사이니지 상단 헤더: 메뉴 버튼, 타이틀, 필터 및 제어 버튼들 -->
<div class="signage-header">
    <!-- 좌측 햄버거 메뉴 버튼 (네비게이션 드로어 토글) -->
    <button id="navDrawerBtn" class="nav-drawer-btn" aria-label="Menu">&#9776;</button>
    <span class="signage-header__title">Downtime Monitoring</span>

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

<!-- Downtime Signage Main -->
<!-- 다운타임 모니터링 메인 컨텐츠 영역 -->
<div class="dt-signage-main" id="dtSignageMain">

    <!-- Row A: Stats (기본 hidden) -->
    <!-- 통계 카드 행: Show Stats 버튼 클릭 시 표시됨 -->
    <div id="dtRowStats" class="dt-row dt-row--stats hidden">
        <div class="dt-stats-grid">
            <!-- 전체 다운타임 건수/시간 카드 (빨간색) -->
            <div class="stat-card stat-card--red">
                <div class="stat-value" id="totalDowntime">-</div>
                <div class="stat-label">Total Downtime</div>
            </div>
            <!-- 현재 활성(진행 중) 다운타임 건수 카드 (로즈색) -->
            <div class="stat-card stat-card--rose">
                <div class="stat-value" id="activeDowntimes">-</div>
                <div class="stat-label">Active Downtimes</div>
            </div>
            <!-- 현재 교대(Shift) 다운타임 건수 카드 (파란색) -->
            <div class="stat-card stat-card--info">
                <div class="stat-value" id="currentShiftDowntime">-</div>
                <div class="stat-label">Current Shift Downtime</div>
            </div>
            <!-- 다운타임으로 영향 받은 기계 수 카드 (주황색) -->
            <div class="stat-card stat-card--warning">
                <div class="stat-value" id="affectedMachinesDowntime">-</div>
                <div class="stat-label">Affected Machines</div>
            </div>
            <!-- 30분 이상 장시간 다운타임 건수 카드 (마룬색) -->
            <div class="stat-card stat-card--maroon">
                <div class="stat-value" id="longDowntimes">-</div>
                <div class="stat-label">Long Downtimes (&gt;30min)</div>
            </div>
            <!-- 평균 다운타임 처리 완료 시간 카드 (초록색) -->
            <div class="stat-card stat-card--success">
                <div class="stat-value" id="avgDowntimeResolution">-</div>
                <div class="stat-label">Avg Resolution Time</div>
            </div>
        </div>
    </div>

    <!-- Row B: Charts Top — Summary Details(2fr) + 차트 2개(3fr) (기본 hidden) -->
    <!-- 차트 상단 행: Show Charts 버튼 클릭 시 표시됨 -->
    <div id="dtRowChartsTop" class="dt-row dt-row--charts-top hidden">
        <div class="dt-charts-top-grid">

            <!-- 좌: Downtime Summary Details -->
            <!-- 다운타임 요약 상세 정보 카드 (2fr 너비): 전체·교대·기계·처리 성과 4개 항목 -->
            <div class="fiori-card">
                <div class="fiori-card__header">
                    <div class="card-title-row">
                        <h3 class="fiori-card__title">Downtime Summary Details</h3>
                    </div>
                    <!-- 실시간 모니터링 상태 표시 -->
                    <div class="real-time-status">
                        <div class="status-dot"></div>
                        <span id="dtLiveStatus">Real-time monitoring active</span>
                    </div>
                </div>
                <div class="fiori-card__content">
                    <!-- 다운타임 요약 상세 목록 컨테이너 (JS에서 데이터 업데이트) -->
                    <div class="dt-details-list" id="dtDetailsContainer">
                        <!-- 항목 1: 전체 다운타임 (활성·완료 건수 포함) -->
                        <div class="dt-component-item">
                            <div class="dt-component-info">
                                <div class="dt-component-name">Total Downtime</div>
                                <div class="dt-component-details">
                                    <!-- 활성 다운타임 건수 -->
                                    <span class="dt-detail-item">Active: <span id="dtDetailWarningCount">-</span></span>
                                    <!-- 처리 완료 다운타임 건수 -->
                                    <span class="dt-detail-item">Completed: <span id="dtDetailCompletedCount">-</span></span>
                                </div>
                            </div>
                            <!-- 전체 요약 값 표시 -->
                            <div class="dt-component-value"><span id="dtTotalDetail">-</span></div>
                        </div>
                        <!-- 항목 2: 현재 교대 다운타임 영향 (교대 건수·장기 다운타임 포함) -->
                        <div class="dt-component-item">
                            <div class="dt-component-info">
                                <div class="dt-component-name">Current Shift Impact</div>
                                <div class="dt-component-details">
                                    <!-- 현재 교대 다운타임 건수 -->
                                    <span class="dt-detail-item">Shift Count: <span id="dtDetailShiftCount">-</span></span>
                                    <!-- 30분 이상 장시간 다운타임 건수 -->
                                    <span class="dt-detail-item">Long Downtimes: <span id="dtDetailLongDowntimes">-</span></span>
                                </div>
                            </div>
                            <!-- 교대 영향 요약 값 -->
                            <div class="dt-component-value"><span id="dtShiftDetail">-</span></div>
                        </div>
                        <!-- 항목 3: 기계 가동 가능성 (영향 기계 수·최대 지속 시간 포함) -->
                        <div class="dt-component-item">
                            <div class="dt-component-info">
                                <div class="dt-component-name">Machine Availability</div>
                                <div class="dt-component-details">
                                    <!-- 영향 받은 기계 수 -->
                                    <span class="dt-detail-item">Affected: <span id="dtDetailAffectedMachines">-</span></span>
                                    <!-- 최대 다운타임 지속 시간 -->
                                    <span class="dt-detail-item">Max Duration: <span id="dtDetailMaxDuration">-</span></span>
                                </div>
                            </div>
                            <!-- 기계 가동 가능성 요약 값 -->
                            <div class="dt-component-value"><span id="dtMachineDetail">-</span></div>
                        </div>
                        <!-- 항목 4: 처리 성과 (평균 처리 시간·30분 초과 건수 포함) -->
                        <div class="dt-component-item">
                            <div class="dt-component-info">
                                <div class="dt-component-name">Resolution Performance</div>
                                <div class="dt-component-details">
                                    <!-- 평균 처리 완료 시간 -->
                                    <span class="dt-detail-item">Avg Time: <span id="dtDetailAvgResolution">-</span></span>
                                    <!-- 30분 이상 초과 건수 -->
                                    <span class="dt-detail-item">Over 30min: <span id="dtDetailOver30">-</span></span>
                                </div>
                            </div>
                            <!-- 처리 성과 요약 값 -->
                            <div class="dt-component-value"><span id="dtResolutionDetail">-</span></div>
                        </div>
                    </div>
                </div>
            </div>

            <!-- 우: 차트 2개 -->
            <!-- 다운타임 유형 분석 차트 + 상태 분포 차트 쌍 (3fr 너비) -->
            <div class="dt-charts-pair">
                <!-- 다운타임 유형별 지속 시간 분석 차트 카드 -->
                <div class="fiori-card">
                    <div class="fiori-card__header">
                        <div class="card-title-row">
                            <h3 class="fiori-card__title">Downtime Type Analysis</h3>
                            <span class="card-subtitle-inline">Duration by type (minutes)</span>
                        </div>
                    </div>
                    <div class="fiori-card__content">
                        <!-- 다운타임 유형 분석 차트 캔버스 (Chart.js) -->
                        <div class="chart-container"><canvas id="dtTypeChart"></canvas></div>
                    </div>
                </div>
                <!-- 다운타임 상태 분포 차트 카드 (활성 vs 완료) -->
                <div class="fiori-card">
                    <div class="fiori-card__header">
                        <div class="card-title-row">
                            <h3 class="fiori-card__title">Downtime Status</h3>
                            <span class="card-subtitle-inline">Active vs Completed</span>
                        </div>
                    </div>
                    <div class="fiori-card__content">
                        <!-- 다운타임 상태 분포 차트 캔버스 -->
                        <div class="chart-container"><canvas id="dtStatusChart"></canvas></div>
                    </div>
                </div>
            </div>

        </div>
    </div>

    <!-- Row C: Charts Bottom — 3 Trend 차트 (기본 hidden) -->
    <!-- 차트 하단 행: Show Charts 시 표시되는 추세·라인별·지속시간 분포 차트 3개 -->
    <div id="dtRowChartsBottom" class="dt-row dt-row--charts-bottom hidden">
        <div class="dt-charts-trio">

            <!-- 시간별 다운타임 발생 추세 차트 (경보 중/완료 구분) -->
            <div class="fiori-card">
                <div class="fiori-card__header">
                    <div class="card-title-row">
                        <h3 class="fiori-card__title">Downtime Trend</h3>
                        <span class="card-subtitle-inline">Hourly occurrence (Warning / Completed)</span>
                    </div>
                </div>
                <div class="fiori-card__content">
                    <!-- 다운타임 추세 차트 캔버스 -->
                    <div class="chart-container"><canvas id="dtTrendChart"></canvas></div>
                </div>
            </div>

            <!-- 라인별 다운타임 비교 차트 (타이틀·서브타이틀은 JS에서 동적 업데이트) -->
            <div class="fiori-card">
                <div class="fiori-card__header">
                    <div class="card-title-row">
                        <!-- 카드 타이틀: JS에서 조건에 따라 동적 변경 -->
                        <h3 class="fiori-card__title" id="dtLineChartTitle">Line Downtime</h3>
                        <!-- 카드 서브타이틀: JS에서 동적 변경 -->
                        <span class="card-subtitle-inline" id="dtLineChartSubtitle">Downtime comparison by line</span>
                    </div>
                </div>
                <div class="fiori-card__content">
                    <!-- 라인별 다운타임 비교 차트 캔버스 -->
                    <div class="chart-container"><canvas id="dtLineChart"></canvas></div>
                </div>
            </div>

            <!-- 다운타임 지속 시간 구간별 분포 차트 (Duration Bucket) -->
            <div class="fiori-card">
                <div class="fiori-card__header">
                    <div class="card-title-row">
                        <h3 class="fiori-card__title">Duration Distribution</h3>
                        <span class="card-subtitle-inline">Downtime duration buckets</span>
                    </div>
                </div>
                <div class="fiori-card__content">
                    <!-- 다운타임 지속 시간 분포 차트 캔버스 -->
                    <div class="chart-container"><canvas id="dtDurationChart"></canvas></div>
                </div>
            </div>

        </div>
    </div>

    <!-- Row D: Real-time Downtime Table -->
    <!-- 실시간 다운타임 데이터 테이블 행 (페이지 로드 시 기본 표시) -->
    <div id="dtRowTable" class="dt-row dt-row--table">
        <div class="fiori-card">
            <div class="fiori-card__header">
                <h3 class="fiori-card__title">Real-time Downtime Data</h3>
                <!-- 마지막 업데이트 시간 및 연결 상태 표시 -->
                <div class="real-time-status">
                    <div class="status-dot"></div>
                    <span id="lastUpdateTime">Last updated: -</span>
                    <span id="connectionStatus" class="connection-status-info">Connection ready...</span>
                </div>
            </div>
            <div class="fiori-card__content">
                <div class="dt-table-wrap">
                    <!-- 다운타임 데이터 테이블 -->
                    <table class="fiori-table" id="downtimeDataTable">
                        <!-- 테이블 헤더: 기계번호·공장라인·교대·다운타임유형·상태·발생시간·처리시간·소요시간·작업일·AI위험도·상세 -->
                        <thead class="fiori-table__header">
                            <tr>
                                <th>Machine No</th>
                                <th>Factory/Line</th>
                                <th>Shift</th>
                                <th>Downtime Type</th>
                                <th>Status</th>
                                <th>Occurrence Time</th>
                                <th>Resolution Time</th>
                                <th>Duration</th>
                                <th>Work Date</th>
                                <!-- AI 위험도 컬럼: ai_downtime_risk_2.js에서 위험 점수를 표시 -->
                                <th>AI Risk <span class="ai-badge" style="font-size:0.6rem;padding:1px 6px;">AI</span></th>
                                <th>DETAIL</th>
                            </tr>
                        </thead>
                        <!-- 테이블 바디: JS(data_downtime_2.js)에서 동적으로 행 생성 -->
                        <tbody id="downtimeDataBody">
                            <tr>
                                <td colspan="11" class="data-table-centered">
                                    <div class="fiori-alert fiori-alert--info">
                                        <strong>Information:</strong> Loading real-time Downtime data. Automatic monitoring is in progress.
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
    <div id="dtRowPagination" class="dt-row dt-row--pagination">
        <div id="pagination-controls" class="fiori-pagination"></div>
    </div>

</div><!-- /dt-signage-main -->


<!-- Downtime Detail Modal -->
<!-- 다운타임 상세 정보 팝업 모달: 테이블에서 DETAIL 버튼 클릭 시 표시 -->
<div id="downtimeDetailModal" class="fiori-modal">
    <!-- 배경(backdrop) 클릭 시 모달 닫기 -->
    <div class="fiori-modal__backdrop" onclick="closeDowntimeDetailModal()"></div>
    <div class="fiori-modal__content">
        <div class="fiori-card">
            <div class="fiori-card__header">
                <h3 class="fiori-card__title">Downtime Details</h3>
                <!-- X 버튼으로 모달 닫기 -->
                <button class="fiori-btn fiori-btn--icon" onclick="closeDowntimeDetailModal()"><span>&#10005;</span></button>
            </div>
            <div class="fiori-card__content">
                <!-- 모달 상세 정보 그리드: 기본정보·시간정보·작업정보·추가정보 4섹션 -->
                <div class="dt-detail-grid">
                    <!-- 기본 정보 섹션: 기계번호, 공장/라인, 다운타임유형, 상태 -->
                    <div class="dt-detail-section">
                        <h4 class="dt-detail-section-title">Basic Information</h4>
                        <div class="dt-detail-row"><span class="dt-detail-label">Machine Number:</span><span class="dt-detail-value" id="modal-machine-no">-</span></div>
                        <div class="dt-detail-row"><span class="dt-detail-label">Factory/Line:</span><span class="dt-detail-value" id="modal-factory-line">-</span></div>
                        <div class="dt-detail-row"><span class="dt-detail-label">Downtime Type:</span><span class="dt-detail-value" id="modal-downtime-type">-</span></div>
                        <div class="dt-detail-row"><span class="dt-detail-label">Status:</span><span class="dt-detail-value" id="modal-status">-</span></div>
                    </div>
                    <!-- 시간 정보 섹션: 발생시간, 처리시간, 소요시간, 작업일 -->
                    <div class="dt-detail-section">
                        <h4 class="dt-detail-section-title">Time Information</h4>
                        <div class="dt-detail-row"><span class="dt-detail-label">Occurrence Time:</span><span class="dt-detail-value" id="modal-reg-date">-</span></div>
                        <div class="dt-detail-row"><span class="dt-detail-label">Resolution Time:</span><span class="dt-detail-value" id="modal-update-date">-</span></div>
                        <div class="dt-detail-row"><span class="dt-detail-label">Duration:</span><span class="dt-detail-value" id="modal-duration">-</span></div>
                        <div class="dt-detail-row"><span class="dt-detail-label">Work Date:</span><span class="dt-detail-value" id="modal-work-date">-</span></div>
                    </div>
                    <!-- 작업 정보 섹션: 교대(Shift), 다운타임 색상 표시 -->
                    <div class="dt-detail-section">
                        <h4 class="dt-detail-section-title">Work Information</h4>
                        <div class="dt-detail-row"><span class="dt-detail-label">Shift:</span><span class="dt-detail-value" id="modal-shift">-</span></div>
                        <div class="dt-detail-row"><span class="dt-detail-label">Downtime Color:</span><span class="dt-detail-value" id="modal-downtime-color"><span class="dt-color-indicator" id="modal-color-indicator"></span><span id="modal-color-value">Default Color</span></span></div>
                    </div>
                    <!-- 추가 정보 섹션: DB ID, 등록일시 (전체 너비) -->
                    <div class="dt-detail-section dt-detail-section--full">
                        <h4 class="dt-detail-section-title">Additional Information</h4>
                        <div class="dt-detail-row"><span class="dt-detail-label">Database ID:</span><span class="dt-detail-value" id="modal-idx">-</span></div>
                        <div class="dt-detail-row"><span class="dt-detail-label">Registration Date:</span><span class="dt-detail-value" id="modal-created-at">-</span></div>
                    </div>
                </div>
            </div>
            <!-- 모달 푸터: 닫기 및 단건 내보내기 버튼 -->
            <div class="fiori-card__footer">
                <div class="dt-detail-actions">
                    <!-- 모달 닫기 버튼 -->
                    <button class="fiori-btn fiori-btn--secondary" onclick="closeDowntimeDetailModal()">Close</button>
                    <!-- 현재 다운타임 단건 내보내기 버튼 (JS: exportSingleDowntime()) -->
                    <button class="fiori-btn fiori-btn--primary" onclick="exportSingleDowntime()">Export</button>
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
<!-- 다운타임 데이터 모니터링 메인 JS (데이터 로딩, 차트, 테이블, 페이지네이션 처리) -->
<script src="js/data_downtime_2.js"></script>
<!-- AI 다운타임 위험 점수 렌더링 JS (테이블 AI Risk 컬럼 업데이트) -->
<script src="js/ai_downtime_risk_2.js"></script>


</body>

</html>
