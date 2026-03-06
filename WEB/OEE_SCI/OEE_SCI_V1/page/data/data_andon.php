<?php
$page_title = 'Andon Data Monitoring';
$page_css_files = ['../../assets/css/fiori-page.css', '../../assets/css/daterangepicker.css', 'css/data_andon.css'];

require_once(__DIR__ . '/../../inc/head.php');
require_once(__DIR__ . '/../../inc/nav-fiori.php');
?>

<div class="fiori-container">
  <main>
    
    <!-- Andon 정보 박스 -->
    <!-- <div class="andon-hero-box">
      <h2>🚨 Andon 데이터 실시간 모니터링</h2>
      <p>생산 라인의 안돈(Andon) 경고 및 완료 상황을 실시간으로 모니터링합니다. 데이터베이스 변경 시 즉시 화면에 반영되어 신속한 대응이 가능합니다.</p>
      <div class="real-time-status">
        <div class="status-dot"></div>
        <span id="connectionStatus">Andon 시스템 연결 준비됨</span>
      </div>
    </div> -->

    <!-- 페이지 헤더 -->
    <div class="fiori-main-header">
      <div>
        <!-- <h2>🚨 Andon 데이터 모니터링</h2> -->
        <h2>Andon Monitoring</h2>
        <!-- <p style="color: var(--sap-text-secondary); margin: var(--sap-spacing-xs) 0 0 0;">
          실시간 안돈 경고 및 해결 상황 모니터링
        </p> -->
      </div>
      
      <!-- 3단계 연동 필터링 시스템을 헤더 안으로 이동 -->
      <div class="fiori-card__content filter-header-content">
        <div class="filter-section">
          <div>
            <!-- <label for="factoryFilterSelect" class="fiori-label">공장 선택</label> -->
            <select id="factoryFilterSelect" class="fiori-select">
              <option value="">All Factory</option>
              <!-- JS로 동적 로딩 -->
            </select>
          </div>
          <div>
            <!-- <label for="factoryLineFilterSelect" class="fiori-label">라인 선택</label> -->
            <select id="factoryLineFilterSelect" class="fiori-select" disabled>
              <option value="">All line</option>
              <!-- JS로 동적 로딩 -->
            </select>
          </div>
          <div>
            <!-- <label for="factoryLineMachineFilterSelect" class="fiori-label">기계 선택</label> -->
            <select id="factoryLineMachineFilterSelect" class="fiori-select" disabled>
              <option value="">All Machine</option>
              <!-- JS로 동적 로딩 -->
            </select>
          </div>
          <div>
            <!-- <label for="timeRangeSelect" class="fiori-label">시간 범위</label> -->
            <select id="timeRangeSelect" class="fiori-select">
              <option value="today" selected>today</option>
              <option value="yesterday">yseterday</option>
              <option value="1w">last week</option>
              <option value="1m">last month</option>
            </select>
          </div>
          <div>
            <!-- <label for="dateRangePicker" class="fiori-label">조회 기간</label> -->
            <input type="text" id="dateRangePicker" class="fiori-input" readonly placeholder="Select date range" />
          </div>
          <div>
            <!-- <label for="shiftSelect" class="fiori-label">교대 선택</label> -->
            <select id="shiftSelect" class="fiori-select">
              <option value="">All Shift</option>
              <option value="1">Shift 1</option>
              <option value="2">Shift 2</option>
              <option value="3">Shift 3</option>
            </select>
          </div>
          <div>
            <button id="excelDownloadBtn" class="fiori-btn fiori-btn--primary"> Export</button>
            <button id="refreshBtn" class="fiori-btn fiori-btn--tertiary">🔄 Refresh</button>
            <button id="toggleStatsBtn" class="fiori-btn fiori-btn--secondary">Hide Stats</button>
            <button id="toggleChartsBtn" class="fiori-btn fiori-btn--secondary">📈 Hide Charts</button>
          </div>
          <!-- <div style="display: flex; align-items: flex-end;">
            <label class="fiori-label" style="opacity: 0; pointer-events: none;">액션</label>
            <div class="fiori-header-actions">
              <button id="exportDataBtn" class="fiori-btn fiori-btn--tertiary">📊 데이터 내보내기</button>
              <button id="refreshBtn" class="fiori-btn fiori-btn--tertiary">🔄 새로고침</button>
            </div>
          </div> -->
        </div>
      </div>
    </div>

    <!-- Andon 통계 Stat Cards (6개) -->
    <div class="andon-monitoring-grid" id="statsGrid">
      <div class="stat-card stat-card--red">
        <div class="stat-value" id="totalAndons">-</div>
        <div class="stat-label">🚨 Total Andon Count</div>
        <!-- <div class="stat-description">전체 안돈 발생 건수</div> -->
      </div>
      <div class="stat-card stat-card--rose">
        <div class="stat-value" id="activeWarnings">-</div>
        <div class="stat-label">Active Warnings</div>
        <!-- <div class="stat-description">현재 해결되지 않은 경고</div> -->
      </div>
      <div class="stat-card stat-card--info">
        <div class="stat-value" id="currentShiftCount">-</div>
        <div class="stat-label">Current Shift Count</div>
        <!-- <div class="stat-description">현재 교대에 발생한 안돈 수량</div> -->
      </div>
      <div class="stat-card stat-card--warning">
        <div class="stat-value" id="affectedMachines">-</div>
        <div class="stat-label">Affected Machine Count</div>
        <!-- <div class="stat-description">안돈이 발생한 기계 수</div> -->
      </div>
      <div class="stat-card stat-card--maroon">
        <div class="stat-value" id="urgentWarnings">-</div>
        <div class="stat-label">Unresolved Over 5min</div>
        <!-- <div class="stat-description">5분 이상 미해결 경고</div> -->
      </div>
      <div class="stat-card stat-card--success">
        <div class="stat-value" id="avgCompletedTime">-</div>
        <div class="stat-label">Avg Completion Time</div>
        <!-- <div class="stat-description">안돈 완료까지의 평균 시간</div> -->
      </div>
      
      <!-- 나중에 사용할 수 있게 주석 처리된 코드들 -->
      <!-- <div class="stat-card stat-card--success">
        <div class="stat-value" id="completedAndons">-</div>
        <div class="stat-label">✅ 해결 완료</div>
        <div class="stat-description">완료된 안돈 건수</div>
      </div> -->
      
      <!-- <div class="stat-card stat-card--accent">
        <div class="stat-value" id="andonTypes">-</div>
        <div class="stat-label">📋 안돈 유형</div>
        <div class="stat-description">사용된 안돈 유형 수</div>
      </div> -->
      
      <!-- 사용하지 않는 기존 카드들 
      <div class="stat-card stat-card--success">
        <div class="stat-value" id="todayAndons">-</div>
        <div class="stat-label">📅 오늘 발생</div>
        <div class="stat-description">금일 발생한 안돈 건수</div>
      </div> -->
    </div>

    <!-- 메인 레이아웃: 활성 안돈 (50%) + 차트 (50%) -->
    <div class="andon-main-layout">
      <!-- 활성 안돈 현황 (50% width) -->
      <div class="fiori-section andon-active-section">
        <div class="fiori-card">
          <div class="fiori-card__header">
            <h3 class="fiori-card__title">🔥 Currently active Andon</h3>
            <!-- <p class="fiori-card__subtitle">현재 해결되지 않은 안돈 경고들 - 즉시 조치가 필요합니다</p> -->
            <div class="real-time-status real-time-status-header">
              <div class="status-dot"></div>
              <span id="activeAndonCount">0 active alerts</span>
            </div>
          </div>
          <div class="fiori-card__content">
            <div id="activeAndonsContainer">
              <div class="fiori-alert fiori-alert--info">
                <strong>ℹ️ Information:</strong> There are currently no active Andon. Real-time monitoring is active.
              </div>
            </div>
          </div>
        </div>
      </div>

      <!-- 안돈 유형별 분석 차트 (50% width) -->
      <div class="fiori-section">
        <div class="fiori-card andon-chart-small">
          <div class="fiori-card__header">
            <h3 class="fiori-card__title">📊 Analysis by Andon type</h3>
            <p class="fiori-card__subtitle">Frequency of occurrence by Andon type</p>
          </div>
          <div class="fiori-card__content">
            <div class="chart-container">
              <canvas id="andonTypeChart"></canvas>
            </div>
          </div>
        </div>
      </div>
    </div>

    <!-- 안돈 발생 추이 차트 (100% width - 새로운 행) -->
    <div class="fiori-section andon-trend-chart-full" id="andonTrendSection" style="margin-bottom: var(--sap-spacing-xl);">
      <div class="fiori-card andon-chart-small">
        <div class="fiori-card__header">
          <h3 class="fiori-card__title">📈 Andon occurrence trend</h3>
          <p class="fiori-card__subtitle">Hourly change in the number of Andon occurrences</p>
        </div>
        <div class="fiori-card__content">
          <div class="chart-container">
            <canvas id="andonTrendChart"></canvas>
          </div>
        </div>
      </div>
    </div>

    <!-- 추가 차트들 (하단에 전체 너비로 배치) -->
    <!-- <div class="andon-additional-charts"> -->
      <!-- 안돈 상태 분포 차트 -->
      <!-- <div class="fiori-section">
        <div class="fiori-card">
          <div class="fiori-card__header">
            <h3 class="fiori-card__title">🔧 안돈 상태 분포</h3>
            <p class="fiori-card__subtitle">경고/완료 상태 비율</p>
          </div>
          <div class="fiori-card__content">
            <div class="chart-container">
              <canvas id="andonStatusChart"></canvas>
            </div>
          </div>
        </div>
      </div> -->

      <!-- 해결 시간 분석 차트 -->
      <!-- <div class="fiori-section">
        <div class="fiori-card">
          <div class="fiori-card__header">
            <h3 class="fiori-card__title">⏰ 해결 시간 분석</h3>
            <p class="fiori-card__subtitle">안돈 해결 시간 분포</p>
          </div>
          <div class="fiori-card__content">
            <div class="chart-container">
              <canvas id="resolutionTimeChart"></canvas>
            </div>
          </div>
        </div>
      </div>
    </div> -->

    <!-- 실시간 안돈 데이터 테이블 -->
    <div class="fiori-section">
      <div class="fiori-card">
        <div class="fiori-card__header">
          <h3 class="fiori-card__title">📋 Real-time Andon data</h3>
          <!-- <p class="fiori-card__subtitle">3단계 필터(공장→라인→기계) 및 날짜 조건에 맞는 안돈 발생 및 해결 기록</p> -->
          <div class="real-time-status">
            <div class="status-dot"></div>
            <span id="lastUpdateTime">Last updated: -</span>
            <span id="connectionStatus" class="connection-status-info">Connection ready...</span>
          </div>
        </div>
        <div class="fiori-card__content fiori-p-0">
          <div class="data-table-wrapper">
            <table class="fiori-table" id="andonDataTable">
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
              <tbody id="andonDataBody">
                <!-- 실시간 데이터가 여기에 로딩됩니다 -->
                <tr>
                  <td colspan="10" class="data-table-centered">
                    <div class="fiori-alert fiori-alert--info">
                      <strong>ℹ️ Information:</strong> Loading real-time Andon data. Automatic monitoring is in progress.
                    </div>
                  </td>
                </tr>
              </tbody>
            </table>
          </div>
        </div>
      </div>
      
      <!-- 페이지네이션 -->
      <div id="pagination-controls" class="fiori-pagination"></div>
    </div>

    <!-- 실시간 알림 영역 -->
    <!-- <div class="fiori-section">
      <div class="fiori-card">
        <div class="fiori-card__header">
          <h3 class="fiori-card__title">🚨 실시간 안돈 알림</h3>
          <p class="fiori-card__subtitle">안돈 발생, 해결 및 시스템 상태 알림</p>
        </div>
        <div class="fiori-card__content">
          <div id="alertsContainer" style="max-height: 300px; overflow-y: auto;">
            <div class="fiori-alert fiori-alert--info">
              <strong>ℹ️ 시스템 준비:</strong> 안돈 추적 시스템이 준비되었습니다.
            </div>
          </div>
        </div>
      </div>
    </div> -->

  </main>
</div>

<!-- Andon 상세 정보 모달 -->
<div id="andonDetailModal" class="fiori-modal">
  <div class="fiori-modal__backdrop" onclick="closeAndonDetailModal()"></div>
  <div class="fiori-modal__content">
    <div class="fiori-card">
      <div class="fiori-card__header">
        <h3 class="fiori-card__title">🔍 Andon Details</h3>
        <button class="fiori-btn fiori-btn--icon" onclick="closeAndonDetailModal()">
          <span>✕</span>
        </button>
      </div>
      <div class="fiori-card__content">
        <div class="andon-detail-grid">
          <!-- 기본 정보 섹션 -->
          <div class="andon-detail-section">
            <h4 class="andon-detail-section-title">📋 Basic Information</h4>
            <div class="andon-detail-row">
              <span class="andon-detail-label">Machine Number:</span>
              <span class="andon-detail-value" id="modal-machine-no">-</span>
            </div>
            <div class="andon-detail-row">
              <span class="andon-detail-label">Factory/Line:</span>
              <span class="andon-detail-value" id="modal-factory-line">-</span>
            </div>
            <div class="andon-detail-row">
              <span class="andon-detail-label">Andon Type:</span>
              <span class="andon-detail-value" id="modal-andon-type">-</span>
            </div>
            <div class="andon-detail-row">
              <span class="andon-detail-label">Status:</span>
              <span class="andon-detail-value" id="modal-status">-</span>
            </div>
          </div>
          
          <!-- 시간 정보 섹션 -->
          <div class="andon-detail-section">
            <h4 class="andon-detail-section-title">⏰ Time Information</h4>
            <div class="andon-detail-row">
              <span class="andon-detail-label">Occurrence Time:</span>
              <span class="andon-detail-value" id="modal-reg-date">-</span>
            </div>
            <div class="andon-detail-row">
              <span class="andon-detail-label">Resolution Time:</span>
              <span class="andon-detail-value" id="modal-update-date">-</span>
            </div>
            <div class="andon-detail-row">
              <span class="andon-detail-label">Duration:</span>
              <span class="andon-detail-value" id="modal-duration">-</span>
            </div>
            <div class="andon-detail-row">
              <span class="andon-detail-label">Work Date:</span>
              <span class="andon-detail-value" id="modal-work-date">-</span>
            </div>
          </div>
          
          <!-- 작업 정보 섹션 -->
          <div class="andon-detail-section">
            <h4 class="andon-detail-section-title">🏭 Work Information</h4>
            <div class="andon-detail-row">
              <span class="andon-detail-label">Shift:</span>
              <span class="andon-detail-value" id="modal-shift">-</span>
            </div>
            <div class="andon-detail-row">
              <span class="andon-detail-label">Andon Color:</span>
              <span class="andon-detail-value" id="modal-andon-color">
                <span class="andon-color-indicator" id="modal-color-indicator"></span>
                <span id="modal-color-value">Default Color</span>
              </span>
            </div>
          </div>
          
          <!-- 추가 정보 섹션 -->
          <div class="andon-detail-section andon-detail-section--full">
            <h4 class="andon-detail-section-title">📝 Additional Information</h4>
            <div class="andon-detail-row">
              <span class="andon-detail-label">Database ID:</span>
              <span class="andon-detail-value" id="modal-idx">-</span>
            </div>
            <div class="andon-detail-row">
              <span class="andon-detail-label">Registration Date:</span>
              <span class="andon-detail-value" id="modal-created-at">-</span>
            </div>
          </div>
        </div>
      </div>
      <div class="fiori-card__footer">
        <div class="andon-detail-actions">
          <button class="fiori-btn fiori-btn--secondary" onclick="closeAndonDetailModal()">Close</button>
          <button class="fiori-btn fiori-btn--primary" onclick="exportSingleAndon()">📊 Export</button>
        </div>
      </div>
    </div>
  </div>
</div>

<footer class="fiori-footer">
  <p>&copy; 2025 SUNTECH. All Rights Reserved.</p>
</footer>

<!-- Chart.js 라이브러리 -->
<!-- <script src="https://cdn.jsdelivr.net/npm/chart.js"></script> -->
<script src="../../assets/js/chart.js"></script>

<!-- jQuery 및 DateRangePicker 라이브러리 -->
<script src="../../assets/js/jquery-3.6.1.min.js"></script>
<script src="../../assets/js/moment.min.js"></script>
<script src="../../assets/js/daterangepicker.js"></script>

<script src="js/data_andon.js"></script>

</body>
</html>