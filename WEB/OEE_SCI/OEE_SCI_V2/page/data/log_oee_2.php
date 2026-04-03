<?php
/*
 * ============================================================
 * 파일명  : log_oee_2.php
 * 목  적  : OEE 데이터 로그(일별 집계) 페이지
 *           날짜 범위 필터 기반으로 일별 OEE 집계 데이터를
 *           테이블로 표시하고, Excel/CSV 내보내기를 지원한다.
 * 레이아웃 구성:
 *   - Row A: OEE 통계 카드 (기본 숨김, Show Stats 버튼으로 토글)
 *   - Row B: OEE 데이터 로그 테이블 (컬럼 표시/숨김 토글 가능)
 *   - Row C: 페이지네이션 컨트롤
 * 특징:
 *   - Columns 드롭다운: 컬럼 표시/숨김을 동적으로 제어 (JS)
 *   - 테이블 헤더는 JS(log_oee_2.js)에서 동적으로 생성
 * 연관 파일:
 *   - css/log_oee_2.css          : OEE 로그 페이지 전용 스타일
 *   - js/log_oee_2.js            : OEE 로그 데이터 로딩·렌더링·컬럼 토글 로직
 *   - inc/signage_filters.php    : 공장·라인·기계·날짜 필터 HTML 조각
 *   - inc/oee_stats_grid.php     : OEE 통계 카드 그리드 HTML 조각
 * ============================================================
 */

// 브라우저 탭 및 페이지 제목 설정
$page_title = 'OEE Data Log';

// 페이지에서 사용할 CSS 파일 목록
$page_css_files = [
    '../../assets/css/fiori-page.css',       // Fiori 공통 페이지 스타일
    '../../assets/css/daterangepicker.css',   // 날짜 범위 선택기 스타일
    'css/log_oee_2.css',                      // OEE 로그 페이지 전용 스타일
];

// 공통 헤드 파일 포함 (HTML head 태그, CSS 로드, meta 태그 등)
require_once(__DIR__ . '/../../inc/head.php');
?>

<?php
// 네비게이션 컨텍스트 및 활성 메뉴 설정
$nav_context = 'data';
$nav_active = 'log_oee'; // 네비게이션에서 'log_oee' 메뉴 항목을 활성화
// 사이드 드로어(네비게이션 메뉴) 포함
require_once(__DIR__ . '/../../inc/nav-drawer-manage.php'); ?>

<!-- Signage Header -->
<!-- 사이니지 상단 헤더: 메뉴 버튼, 타이틀, 필터 및 제어 버튼들 -->
<div class="signage-header">
    <!-- 좌측 햄버거 메뉴 버튼 (네비게이션 드로어 토글) -->
    <button id="navDrawerBtn" class="nav-drawer-btn" aria-label="Menu">&#9776;</button>
    <span class="signage-header__title">OEE Data Log</span>

    <!-- 필터 및 뷰 제어 버튼 영역 -->
    <div class="signage-header__filters">
        <!-- 공통 사이니지 필터 (공장·라인·기계·날짜 선택기) -->
        <?php include __DIR__ . '/inc/signage_filters.php'; ?>

        <!-- 컬럼 표시/숨김 드롭다운 컨테이너 -->
        <div class="log-oee-dropdown">
            <!-- 컬럼 토글 버튼 (클릭 시 아래 드롭다운 표시) -->
            <button id="columnToggleBtn" class="fiori-btn fiori-btn--secondary">Columns</button>
            <!-- 컬럼 선택 드롭다운 (JS에서 체크박스 목록 동적 생성) -->
            <div id="columnToggleDropdown" class="log-oee-dropdown__content"></div>
        </div>

        <!-- 통계 행 토글 버튼 (Show Stats / Hide Stats) -->
        <button id="toggleStatsBtn" class="fiori-btn fiori-btn--secondary">Show Stats</button>
        <!-- 테이블 행 토글 버튼 (주석 처리 — 현재 비활성화) -->
        <!-- <button id="toggleDataBtn" class="fiori-btn fiori-btn--secondary">Hide Table</button> -->
        <!-- Excel/CSV 내보내기 버튼 -->
        <button id="excelDownloadBtn" class="fiori-btn fiori-btn--secondary">Export</button>
        <!-- 수동 새로고침 버튼 -->
        <button id="refreshBtn" class="fiori-btn fiori-btn--tertiary">Refresh</button>
    </div>
</div>

<!-- Log OEE Signage Main -->
<!-- OEE 로그 메인 컨텐츠 영역 -->
<div class="log-oee-main" id="logOeeMain">

    <!-- Row A: Stats (기본 hidden) -->
    <!-- OEE 통계 카드 행: Show Stats 버튼 클릭 시 표시됨 -->
    <div id="logOeeStats" class="log-oee log-oee--stats hidden">
        <div class="oee-stats-grid">
            <!-- OEE 통계 카드 그리드 HTML 조각 포함 (공통 컴포넌트) -->
            <?php include __DIR__ . '/inc/oee_stats_grid.php'; ?>
        </div>
    </div>

    <!-- Row B: Table -->
    <!-- OEE 일별 집계 데이터 테이블 행 (항상 표시) -->
    <div id="logOeeTable" class="log-oee log-oee--table">
        <div class="fiori-card">
            <div class="fiori-card__header">
                <h3 class="fiori-card__title">OEE Data Log</h3>
                <!-- 마지막 업데이트 시간 및 연결 상태 표시 -->
                <div class="real-time-status">
                    <div class="status-dot"></div>
                    <span id="lastUpdateTime">Last updated: -</span>
                    <span id="connectionStatus" class="connection-status-info">Connection ready...</span>
                </div>
            </div>
            <div class="fiori-card__content">
                <div class="oee-table-wrap">
                    <!-- OEE 로그 데이터 테이블 -->
                    <table class="fiori-table" id="oeeDataTable">
                        <thead class="fiori-table__header">
                            <!-- 테이블 헤더 행: JS(log_oee_2.js)에서 컬럼 토글에 따라 동적으로 생성 -->
                            <tr id="tableHeaderRow">
                                <!-- JS로 헤더 생성 -->
                            </tr>
                        </thead>
                        <!-- 테이블 바디: JS(log_oee_2.js)에서 데이터 스트림으로 행 생성 -->
                        <tbody id="oeeDataBody">
                            <tr>
                                <!-- 초기 로딩 메시지: 최대 35컬럼 colspan 적용 -->
                                <td colspan="35" class="data-table-centered">
                                    <div class="fiori-alert fiori-alert--info">
                                        <strong>Information:</strong> Loading OEE data log. Please wait...
                                    </div>
                                </td>
                            </tr>
                        </tbody>
                    </table>
                </div>
            </div>
        </div>
    </div>

    <!-- Row C: Pagination -->
    <!-- 페이지네이션 컨트롤 행 (JS에서 동적으로 버튼 생성) -->
    <div id="logOeePagination" class="log-oee log-oee--pagination">
        <div id="pagination-controls" class="fiori-pagination"></div>
    </div>

</div><!-- /log-oee-main -->


<!-- 외부 라이브러리: jQuery, Moment.js, DateRangePicker -->
<!-- Chart.js는 이 페이지에서 미사용 (테이블 전용 페이지) -->
<script src="../../assets/js/jquery-3.6.1.min.js"></script>
<script src="../../assets/js/moment.min.js"></script>
<script src="../../assets/js/daterangepicker.js"></script>
<!-- OEE 로그 데이터 메인 JS (데이터 로딩, 테이블 렌더링, 컬럼 토글, 페이지네이션 처리) -->
<script src="js/log_oee_2.js"></script>


</body>

</html>
