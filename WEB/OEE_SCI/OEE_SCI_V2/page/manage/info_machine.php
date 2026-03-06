<?php
// 페이지별 설정
$page_title = 'Machine Management';
$page_css_files = ['../../assets/css/fiori-page.css', 'css/info_machine.css'];
$page_styles = '';

// 공통 헤더 로드
require_once(__DIR__ . '/../../inc/head.php');
require_once(__DIR__ . '/../../inc/nav-fiori.php');
?>

  <div class="fiori-container">
    <main>
      
      <!-- 페이지 헤더 -->
      <div class="fiori-main-header">
        <div>
          <h2>Machine Management</h2>
          <!-- <p style="color: var(--sap-text-secondary); margin: var(--sap-spacing-xs) 0 0 0;">
            기계 정보 관리 및 현황 모니터링
          </p> -->
        </div>
        <div class="fiori-header-actions">
          <div class="search-container">
            <div class="search-icon">🔍</div>
            <input 
              type="text" 
              id="realTimeSearch" 
              class="fiori-input search-input" 
              placeholder=""
            >
            <button class="search-clear" id="searchClear" title="Clear search words">✕</button>
          </div>
          <select id="factoryFilterSelect" class="fiori-select" style="width: auto; min-width: 150px;">
            <!-- JS로 채워짐 -->
          </select>
          <select id="factoryLineFilterSelect" class="fiori-select" style="width: auto; min-width: 150px;">
            <!-- JS로 채워짐 -->
          </select>
          <select id="factoryLineMachineFilterSelect" class="fiori-select" style="width: auto; min-width: 150px;">
            <!-- JS로 채워짐 -->
          </select>
          <select id="statusFilterSelect" class="fiori-select" style="width: auto; min-width: 150px;">
            <option value=""> All Status</option>
            <option value="Y">✅ Used Only</option>
            <option value="N">⚠️ Unused Only</option>
          </select>
          <select id="typeFilterSelect" class="fiori-select" style="width: auto; min-width: 150px;">
            <option value=""> 모든 기계 유형</option>
            <option value="P" selected>🧵 Computer Sewing Machine</option>
            <option value="E">🪡 Embroidery Machine</option>
          </select>
          <div class="fiori-dropdown">
            <button id="columnToggleBtn" class="fiori-btn fiori-btn--secondary"> Columns</button>
            <div id="columnToggleDropdown" class="fiori-dropdown__content">
              <!-- JS로 컬럼 체크박스 생성 -->
            </div>
          </div>
          <!-- Add 기능 제거 - 편집 전용 -->
          <button id="excelDownloadBtn" class="fiori-btn fiori-btn--secondary"> Export</button>
          <button id="refreshBtn" class="fiori-btn fiori-btn--tertiary">🔄 Refresh</button>
          <!-- <button id="toggleStatsBtn" class="fiori-btn fiori-btn--secondary">📊 Show Stats</button> -->
        </div>
      </div>

      <!-- 통계 요약 카드 -->
      <div class="factory-stats-grid" id="statsGrid" style="display: none;">
        <div class="stat-card">
          <div class="stat-value" id="totalCount">-</div>
          <div class="stat-label"> Total Machine</div>
        </div>
        <div class="stat-card stat-card--success">
          <div class="stat-value" id="patternMachines">0</div>
          <div class="stat-label"> Computer Sewing Machine</div>
        </div>
        <div class="stat-card stat-card--warning">
          <div class="stat-value" id="embroideryMachines">0</div>
          <div class="stat-label"> Embroidery Machine</div>
        </div>
        <div class="stat-card stat-card--info">
          <div class="stat-value" id="totalTarget">0</div>
          <div class="stat-label"> Total Target</div>
        </div>
        <div class="stat-card stat-card--error">
          <div class="stat-value" id="avgTarget">0</div>
          <div class="stat-label"> Avg Target</div>
        </div>
      </div>

      <!-- 데이터 테이블 섹션 -->
      <div class="fiori-section">
        <div class="fiori-card">
          <div class="fiori-card__header">
            <div>
              <h3 class="fiori-card__title">Machine List</h3>
              <!-- <p class="fiori-card__subtitle">전체 기계 목록 및 상태 관리</p> -->
            </div>
            <!-- 빠른 액션 바를 헤더 오른쪽으로 이동 -->
            <div class="quick-actions">
              <button class="quick-action-btn active" data-filter="all"> All</button>
              <button class="quick-action-btn" data-filter="used">✅ Used</button>
              <button class="quick-action-btn" data-filter="unused">⚠️ Unused</button>
            </div>
          </div>
          
          <div class="fiori-card__content fiori-p-0">
            <div style="overflow-x: auto;">
              <table class="fiori-table">
                <thead id="tableHeader" class="fiori-table__header">
                  <!-- JS로 헤더 생성 -->
                </thead>
                <tbody id="tableBody">
                  <!-- JS로 데이터 행 생성 -->
                </tbody>
              </table>
            </div>
            <div id="noDataMessage" class="text-center" style="padding: var(--sap-spacing-xl); color: var(--sap-text-secondary); display: none;">
              <!-- <div style="font-size: 3rem; margin-bottom: var(--sap-spacing-md);">🔧</div> -->
              <p>There are no machines matching your search criteria.</p>
            </div>
          </div>
        </div>
      </div>
      
      <!-- 페이지네이션 -->
      <div id="pagination-controls" class="fiori-pagination"></div>
    </main>
  </div>

  <footer class="fiori-footer">
    <p>&copy; 2025 SUNTECH. All Rights Reserved.</p>
  </footer>

  <!-- SAP Fiori Machine Modal -->
  <div id="resourceModal" class="fiori-modal">
    <div class="fiori-modal__content">
      <div class="fiori-card">
        <div class="fiori-card__header">
          <div>
            <h3 class="fiori-card__title" id="modalTitle">✏️ Machine Information Edit</h3>
            <p class="fiori-card__subtitle">Edit machine information in 3 steps</p>
          </div>
          <button type="button" class="fiori-btn fiori-btn--tertiary fiori-btn--sm close" 
                  style="position: absolute; top: var(--sap-spacing-md); right: var(--sap-spacing-md);">
            ✕
          </button>
        </div>
        <div class="fiori-card__content">
          <form id="resourceForm" class="fiori-form">
            <input type="hidden" id="resourceId" name="idx">
            
            <!-- 진행 표시기 -->
            <div style="display: flex; justify-content: space-between; align-items: center; margin-bottom: var(--sap-spacing-lg); padding-bottom: var(--sap-spacing-md); border-bottom: 1px solid var(--sap-border-subtle);">
              <div style="display: flex; gap: var(--sap-spacing-sm); align-items: center;">
                <div class="form-step active" data-step="1">1</div>
                <div class="form-step-line"></div>
                <div class="form-step" data-step="2">2</div>
                <div class="form-step-line"></div>
                <div class="form-step" data-step="3">3</div>
              </div>
              <div style="font-size: var(--sap-font-size-sm); color: var(--sap-text-secondary);">
                <span id="stepIndicator">Step 1 of 3</span>
              </div>
            </div>
            
            <!-- Step 1: Factory, Line, Machine Name 선택 -->
            <div class="form-section" data-section="1">
              <h4 style="margin-bottom: var(--sap-spacing-md); color: var(--sap-text-primary);">🏢 Location Information</h4>
              
              <div class="fiori-form__group">
                <label for="factory_idx" class="fiori-form__label fiori-form__label--required">Factory</label>
                <select id="factory_idx" name="factory_idx" class="fiori-select" required>
                  <option value="">Please select a factory</option>
                  <!-- JS로 채워짐 -->
                </select>
                <div class="fiori-form__help">먼저 공장을 선택하세요</div>
              </div>
              
              <div class="fiori-form__group">
                <label for="line_idx" class="fiori-form__label fiori-form__label--required">Line</label>
                <select id="line_idx" name="line_idx" class="fiori-select" required disabled>
                  <option value="">Please select a line</option>
                  <!-- JS로 채워짐 -->
                </select>
                <div class="fiori-form__help">공장 선택 후 라인을 선택하세요</div>
              </div>
              
              <div class="fiori-form__group">
                <label for="machine_no" class="fiori-form__label fiori-form__label--required">Machine Name</label>
                <input type="text" id="machine_no" name="machine_no" class="fiori-input" required 
                       placeholder="Machine name을 입력하세요">
                <div class="fiori-form__help">기계 이름을 입력하세요 (2-50 characters)</div>
              </div>
            </div>
            
            <!-- Step 2: Type, Machine Model, Status 선택 -->
            <div class="form-section" data-section="2" style="display: none;">
              <h4 style="margin-bottom: var(--sap-spacing-md); color: var(--sap-text-primary);">⚙️ Machine Settings</h4>
              
              <div class="fiori-form__group">
                <label for="type" class="fiori-form__label fiori-form__label--required">Machine Type</label>
                <select id="type" name="type" class="fiori-select" required>
                  <option value="">Please select machine type</option>
                  <option value="P">Computer Sewing Machine</option>
                  <option value="E">Embroidery Machine</option>
                </select>
                <div class="fiori-form__help">기계 타입을 선택하세요</div>
              </div>
              
              <div class="fiori-form__group">
                <label for="machine_model_idx" class="fiori-form__label fiori-form__label--required">Machine Model</label>
                <select id="machine_model_idx" name="machine_model_idx" class="fiori-select" required>
                  <option value="">Please select machine model</option>
                  <!-- JS로 채워짐 -->
                </select>
                <div class="fiori-form__help">기계 모델을 선택하세요</div>
              </div>
              
              <div class="fiori-form__group">
                <label for="status" class="fiori-form__label">Status</label>
                <select id="status" name="status" class="fiori-select">
                  <option value="Y">✅ Used</option>
                  <option value="N">⚠️ Unused</option>
                </select>
                <div class="fiori-form__help">기계 사용 상태를 선택하세요</div>
              </div>
            </div>
            
            <!-- Step 3: Target, Remark 입력 -->
            <div class="form-section" data-section="3" style="display: none;">
              <h4 style="margin-bottom: var(--sap-spacing-md); color: var(--sap-text-primary);">📈 Production Settings</h4>
              
              <div class="fiori-form__group">
                <label for="target" class="fiori-form__label">Target Quantity (per hour)</label>
                <input type="number" id="target" name="target" class="fiori-input" min="0" step="1"
                       placeholder="시간당 목표 생산량을 입력하세요">
                <div class="fiori-form__help">시간당 목표 생산량을 입력하세요</div>
              </div>
              
              <div class="fiori-form__group">
                <label for="remark" class="fiori-form__label">Remark</label>
                <textarea id="remark" name="remark" class="fiori-input" rows="3" 
                         placeholder="기계 관련 메모를 입력하세요"></textarea>
                <div class="fiori-form__help">기계 관련 추가 정보나 메모를 입력하세요</div>
              </div>
            </div>
            
            <div class="fiori-form__actions">
              <button type="button" class="fiori-btn fiori-btn--tertiary" id="modalCloseBtn">Cancel</button>
              <button type="button" class="fiori-btn fiori-btn--secondary" id="prevStep" style="display: none;">← Previous</button>
              <button type="button" class="fiori-btn fiori-btn--primary" id="nextStep">Next →</button>
              <button type="submit" class="fiori-btn fiori-btn--primary" id="submitBtn" style="display: none;">✏️ Update Machine</button>
            </div>
          </form>
        </div>
      </div>
    </div>
  </div>


  <script type="module">
    // Resource Manager 및 기계 모듈 가져오기
    import { createResourceManager } from '../../assets/js/resource-manager.js';
    import { initAdvancedFeatures, machineConfig } from './js/info_machine.js';
    import { initStatsToggle } from '../../assets/js/manage/stats-toggle.js';

    // 전역 변수로 resourceManager 선언
    let resourceManager;

    // DOM 로딩 완료 후 초기화
    document.addEventListener('DOMContentLoaded', function() {
      console.log('DOM 로딩 완료, 초기화 시작');
      
      // ResourceManager 생성 - DOMContentLoaded 내부에서 호출하므로 즉시 실행
      resourceManager = createResourceManager(machineConfig);
      
      // ResourceManager 초기화 완료를 기다린 후 고급 기능 초기화
      // 짧은 지연을 두어 ResourceManager가 완전히 초기화되도록 함
      setTimeout(() => {
        initAdvancedFeatures(resourceManager);
      }, 100);
  
      // Stats toggle 초기화 추가
      initStatsToggle();
    });
    
    // fiori-page.css에서 모든 공통 스타일 처리됨
    console.log('Machine 페이지 초기화 완료. 모듈형 JavaScript 로드됨.');
  </script>

</body>
</html>