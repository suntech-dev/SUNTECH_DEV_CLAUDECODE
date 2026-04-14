<?php
/*
 * ============================================================
 * 파일명  : data_offline_2.php
 * 목  적  : 미수신(오프라인) 머신 모니터링 페이지
 *           지정 날짜에 OEE 데이터가 없는 머신 목록을 실시간으로 표시하고
 *           원인(전원 OFF / UART 케이블 단선 등)을 진단할 수 있다.
 * 레이아웃:
 *   - Row A: 요약 통계 카드 4개 (전체 활성 / 금일 데이터 있음 / 미수신 / 미연결)
 *   - Row B: 미수신 머신 테이블 (기계번호·공장·라인·타입·마지막 데이터·경과시간·원인)
 *   - Row C: 페이지네이션
 * 연관 파일:
 *   - css/data_offline_2.css             : 전용 스타일
 *   - js/data_offline_2.js               : SSE 수신·렌더링 로직
 *   - proc/data_offline_stream_2.php     : SSE 스트림 백엔드
 * ============================================================
 */

$page_title    = 'Offline Machine Monitor';
$page_css_files = [
    '../../assets/css/fiori-page.css',
    'css/data_offline_2.css',
];

require_once(__DIR__ . '/../../inc/head.php');
?>

<?php
$nav_context = 'data';
$nav_active  = 'offline_monitor';
require_once(__DIR__ . '/../../inc/nav-drawer-manage.php');
?>

<!-- Signage Header -->
<div class="signage-header">
    <button id="navDrawerBtn" class="nav-drawer-btn" aria-label="Menu">&#9776;</button>
    <span class="signage-header__title">Offline Monitor</span>

    <div class="signage-header__filters">
        <!-- 공장 필터 -->
        <select id="offlineFactoryFilter" class="fiori-select">
            <option value="">All Factory</option>
        </select>
        <!-- 라인 필터 -->
        <select id="offlineLineFilter" class="fiori-select" disabled>
            <option value="">All Line</option>
        </select>
        <!-- 기준 날짜 (기본: 오늘) -->
        <input type="date" id="offlineRefDate" class="fiori-input" title="기준 날짜: 해당 날짜에 데이터가 없는 머신을 조회합니다">
        <!-- 수동 새로고침 -->
        <button id="offlineRefreshBtn" class="fiori-btn fiori-btn--tertiary">Refresh</button>
    </div>
</div>

<!-- Main Content -->
<div class="offline-main">

    <!-- Row A: 요약 통계 카드 -->
    <div class="offline-stats-row">

        <!-- 전체 활성 머신 수 -->
        <div class="offline-stat-card offline-stat-card--total">
            <div class="offline-stat-value" id="offlineStatTotal">-</div>
            <div class="offline-stat-label">Total Active Machines</div>
        </div>

        <!-- 금일 데이터 있는 머신 수 -->
        <div class="offline-stat-card offline-stat-card--active">
            <div class="offline-stat-value" id="offlineStatActive">-</div>
            <div class="offline-stat-label">Machines With Data Today</div>
        </div>

        <!-- 미수신 머신 수 (경보) -->
        <div class="offline-stat-card offline-stat-card--offline">
            <div class="offline-stat-value" id="offlineStatOffline">-</div>
            <div class="offline-stat-label">Offline Machines (No Data)</div>
        </div>

        <!-- 한 번도 데이터 없는 머신 수 -->
        <div class="offline-stat-card offline-stat-card--never">
            <div class="offline-stat-value" id="offlineStatNever">-</div>
            <div class="offline-stat-label">Never Connected</div>
        </div>

    </div><!-- /offline-stats-row -->

    <!-- Row B: 미수신 머신 테이블 -->
    <div class="offline-table-row">
        <div class="fiori-card">
            <div class="fiori-card__header">
                <h3 class="fiori-card__title">Offline Machine List</h3>
                <!-- 기준 날짜 배지 -->
                <span class="ref-date-badge">기준 날짜: <strong id="currentRefDate">-</strong></span>
                <!-- 연결 상태 -->
                <div class="offline-status-bar" style="margin-left: auto;">
                    <div class="offline-status-dot"></div>
                    <span id="offlineConnectionStatus">Connecting...</span>
                    <span id="offlineLastUpdate" style="margin-left:8px; color:var(--sap-text-secondary);"></span>
                </div>
            </div>
            <div class="fiori-card__content">
                <div class="offline-table-wrap">
                    <table class="fiori-table" id="offlineDataTable">
                        <thead class="fiori-table__header">
                            <tr>
                                <th>Machine No</th>
                                <th>Factory</th>
                                <th>Line</th>
                                <th>Type</th>
                                <th>Last Data Time</th>
                                <th>Time Since Last Data</th>
                                <th>Probable Cause</th>
                            </tr>
                        </thead>
                        <tbody id="offlineTableBody">
                            <tr>
                                <td colspan="7">
                                    <div class="offline-empty">
                                        <div class="offline-empty__icon">&#8987;</div>
                                        <div class="offline-empty__text">데이터 로딩 중...</div>
                                        <div class="offline-empty__sub">실시간 모니터링을 시작하고 있습니다.</div>
                                    </div>
                                </td>
                            </tr>
                        </tbody>
                    </table>
                </div>
            </div>
        </div>
    </div><!-- /offline-table-row -->

    <!-- Row C: 페이지네이션 -->
    <div class="offline-pagination-row">
        <div id="offlinePagination" class="fiori-pagination"></div>
    </div>

</div><!-- /offline-main -->


<!-- 원인 분류 범례 안내 (헤더 하단 고정) -->
<!-- 별도 UI 없이 JS 배지로 직관적으로 표시 -->


<!-- JS -->
<script src="js/data_offline_2.js"></script>


</body>
</html>
