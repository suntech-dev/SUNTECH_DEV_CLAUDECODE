<?php
// 페이지별 설정
$page_title = 'Machine Model Management';
$page_css_files = ['../../assets/css/fiori-page.css', 'css/info_machine_model.css'];

// 공통 헤더 로드
require_once(__DIR__ . '/../../inc/head.php');
require_once(__DIR__ . '/../../inc/nav-fiori.php');
?>

  <div class="fiori-container">
    <main>
      
      <!-- 페이지 헤더 -->
      <div class="fiori-main-header">
        <div>
          <h2>Machine Model Management</h2>
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
          <select id="statusFilterSelect" class="fiori-select" style="width: auto; min-width: 150px;">
            <option value=""> All Status</option>
            <option value="Y" selected>✅ Used Only</option>
            <option value="N">⚠️ Unused Only</option>
          </select>
          <div class="fiori-dropdown">
            <button id="columnToggleBtn" class="fiori-btn fiori-btn--secondary"> Columns</button>
            <div id="columnToggleDropdown" class="fiori-dropdown__content">
              <!-- JS로 컬럼 체크박스 생성 -->
            </div>
          </div>
          <button id="addBtn" class="fiori-btn fiori-btn--primary"> Add Machine Model</button>
          <button id="refreshBtn" class="fiori-btn fiori-btn--tertiary">🔄 Refresh</button>
          <!-- <button id="toggleStatsBtn" class="fiori-btn fiori-btn--secondary">📊 Show Stats</button> -->
        </div>
      </div>

      <!-- 통계 요약 카드 -->
      <div class="factory-stats-grid" id="statsGrid" style="display: none;">
        <div class="stat-card">
          <div class="stat-value" id="totalModelsCount">-</div>
          <div class="stat-label">Total Machine Models</div>
        </div>
        <!-- Dynamic machine model cards will be inserted here -->
      </div>

      <!-- 데이터 테이블 섹션 -->
      <div class="fiori-section">
        <div class="fiori-card">
          <div class="fiori-card__header">
            <div>
              <h3 class="fiori-card__title">Machine Model List</h3>
            </div>
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
              <p>There are no machine models matching your search criteria.</p>
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

  <!-- SAP Fiori Machine Model Modal -->
  <div id="resourceModal" class="fiori-modal">
    <div class="fiori-modal__content" style="max-height: 90vh; overflow-y: auto;">
      <div class="fiori-card">
        <div class="fiori-card__header">
          <div>
            <h3 class="fiori-card__title" id="modalTitle"> Machine Model Information</h3>
            <p class="fiori-card__subtitle">Enter and edit machine model information</p>
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
              </div>
              <div style="font-size: var(--sap-font-size-sm); color: var(--sap-text-secondary);">
                <span id="stepIndicator">Step 1 of 2</span>
              </div>
            </div>
            
            <!-- Step 1: 필수 정보 -->
            <div class="form-section" data-section="1">
              <h4 style="margin-bottom: var(--sap-spacing-md); color: var(--sap-text-primary);">Basic information</h4>
              
              <div class="fiori-form__group">
                <label for="machine_model_name" class="fiori-form__label fiori-form__label--required">Machine Model Name</label>
                <input type="text" id="machine_model_name" name="machine_model_name" class="fiori-input" required 
                       placeholder="Please enter the machine model name">
                <div class="fiori-form__help">Please enter a unique machine model name (2-50 characters)</div>
              </div>
              
              <div class="fiori-form__group">
                <label for="type" class="fiori-form__label fiori-form__label--required">Machine Type</label>
                <select id="type" name="type" class="fiori-select" required>
                  <option value="P">Computer Sewing Machine</option>
                  <option value="E">Embroidery Machine</option>
                </select>
                <div class="fiori-form__help">Please select the machine type</div>
              </div>
              
              <div class="fiori-form__group">
                <label for="status" class="fiori-form__label">Status</label>
                <select id="status" name="status" class="fiori-select">
                  <option value="Y">✅ Used</option>
                  <option value="N">⚠️ Unused</option>
                </select>
                <div class="fiori-form__help">Please select the current usage status of the machine model.</div>
              </div>
            </div>
            
            <!-- Step 2: 추가 정보 -->
            <div class="form-section" data-section="2" style="display: none;">
              <h4 style="margin-bottom: var(--sap-spacing-md); color: var(--sap-text-primary);">Additional information</h4>
              
              <div class="fiori-form__group">
                <label for="remark" class="fiori-form__label">Remark</label>
                <textarea id="remark" name="remark" class="fiori-input" rows="3" 
                         placeholder="Additional information about machine model"></textarea>
                <div class="fiori-form__help">You can record machine model information.</div>
              </div>              
              
              <!-- Step 3: Review -->
              <div style="padding: var(--sap-spacing-md); background: var(--sap-surface-2); border-radius: var(--sap-radius-md); margin-top: var(--sap-spacing-md);">
                <h5 style="margin: 0 0 var(--sap-spacing-sm) 0; color: var(--sap-text-primary);">💡 Confirm input information</h5>
                <div style="font-size: var(--sap-font-size-sm); color: var(--sap-text-secondary);">
                  <div><strong>Machine Model name:</strong> <span id="previewName">-</span></div>
                  <div><strong>Status:</strong> <span id="previewStatus">-</span></div>
                </div>
              </div>
            </div>
            
            <div class="fiori-form__actions">
              <button type="button" class="fiori-btn fiori-btn--tertiary" id="modalCloseBtn">Cancel</button>
              <button type="button" class="fiori-btn fiori-btn--secondary" id="prevStep" style="display: none;">← Previous</button>
              <button type="button" class="fiori-btn fiori-btn--primary" id="nextStep">Next →</button>
              <button type="submit" class="fiori-btn fiori-btn--primary" id="submitBtn" style="display: none;">💾 Save Machine Model</button>
            </div>
          </form>
        </div>
      </div>
    </div>
  </div>

  <script type="module">
    // Resource Manager 및 기계 모델 모듈 가져오기
    import { createResourceManager } from '../../assets/js/resource-manager.js';
    import { initAdvancedFeatures, machineModelConfig } from './js/info_machine_model.js';
    import { initStatsToggle } from '../../assets/js/manage/stats-toggle.js';

    // 전역 변수로 resourceManager 선언
    let resourceManager;

    // DOM 로딩 완료 후 초기화
    document.addEventListener('DOMContentLoaded', function() {
      
      // ResourceManager 생성
      resourceManager = createResourceManager(machineModelConfig);
      
      // 고급 기능 초기화
      setTimeout(() => {
        initAdvancedFeatures(resourceManager);
      }, 100);
  
      // Stats toggle 초기화 추가
      initStatsToggle();
    });
  </script>

</body>
</html>