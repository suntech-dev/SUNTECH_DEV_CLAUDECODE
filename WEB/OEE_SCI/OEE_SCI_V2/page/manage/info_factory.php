<?php
// 페이지별 설정
$page_title = 'Factory Management';
$page_css_files = ['../../assets/css/fiori-page.css', 'css/info_factory.css'];
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
          <h2>Factory Management</h2>
        </div>
        <div class="fiori-header-actions">
          <div class="search-container">
            <div class="search-icon">🔍</div>
            <input type="text" id="realTimeSearch" class="fiori-input search-input" placeholder="">
            <button class="search-clear" id="searchClear" title="Clear search words">✕</button>
          </div>
          <select id="statusFilterSelect" class="fiori-select" style="width: auto; min-width: 150px;">
            <option value="">All Status</option>
            <option value="Y" selected>✅ Used Only</option>
            <option value="N">⚠️ Unused Only</option>
          </select>
          <div class="fiori-dropdown">
            <button id="columnToggleBtn" class="fiori-btn fiori-btn--secondary"> Columns</button>
            <div id="columnToggleDropdown" class="fiori-dropdown__content">
              <!-- JS로 컬럼 체크박스 생성 -->
            </div>
          </div>
          <button id="addBtn" class="fiori-btn fiori-btn--primary"> Add Factory</button>
          <button id="refreshBtn" class="fiori-btn fiori-btn--tertiary">🔄 Refresh</button>
          <!-- <button id="toggleStatsBtn" class="fiori-btn fiori-btn--secondary">📊 Show Stats</button> -->
        </div>
      </div>

      <!-- 통계 요약 카드 -->
      <div class="factory-stats-grid" id="statsGrid" style="display: none;">
        <div class="stat-card stat-card--info">
          <div class="stat-value" id="totalCount">-</div>
          <div class="stat-label">Total Factory</div>
        </div>
        <div class="stat-card stat-card--green">
          <div class="stat-value" id="totalLines">-</div>
          <div class="stat-label">Total Lines</div>
        </div>
        <div class="stat-card stat-card--warning">
          <div class="stat-value" id="totalMachines">-</div>
          <div class="stat-label">Total Machines</div>
        </div>
        <div class="stat-card stat-card--purple">
          <div class="stat-value" id="totalMP">-</div>
          <div class="stat-label">Total MP</div>
        </div>
      </div>

      <!-- 데이터 테이블 섹션 -->
      <div class="fiori-section">
        <div class="fiori-card">
          <div class="fiori-card__header">
            <div>
              <h3 class="fiori-card__title">Factory List</h3>
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
                </thead>
                <tbody id="tableBody">
                </tbody>
              </table>
            </div>
            <div id="noDataMessage" class="text-center" style="padding: var(--sap-spacing-xl); color: var(--sap-text-secondary); display: none;">
              <p>No matching factories found.</p>
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

  <!-- SAP Fiori Factory Modal -->
  <div id="resourceModal" class="fiori-modal">
    <div class="fiori-modal__content">
      <div class="fiori-card">
        <div class="fiori-card__header">
          <div>
            <h3 class="fiori-card__title" id="modalTitle">Factory Information</h3>
            <p class="fiori-card__subtitle">Enter and edit factory information</p>
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
                <label for="factory_name" class="fiori-form__label fiori-form__label--required">Factory Name</label>
                <input type="text" id="factory_name" name="factory_name" class="fiori-input" required 
                       placeholder="Please enter the factory name">
                <div class="fiori-form__help">Please enter a unique factory name (2-50 characters)</div>
              </div>
              
              <div class="fiori-form__group">
                <label for="status" class="fiori-form__label">Status</label>
                <select id="status" name="status" class="fiori-select">
                  <option value="Y">✅ Used</option>
                  <option value="N">⚠️ Unused</option>
                </select>
                <div class="fiori-form__help">Please select the current usage status of the factory.</div>
              </div>
            </div>
            
            <!-- Step 2: 추가 정보 -->
            <div class="form-section" data-section="2" style="display: none;">
              <h4 style="margin-bottom: var(--sap-spacing-md); color: var(--sap-text-primary);">Additional information</h4>
              
              <div class="fiori-form__group">
                <label for="remark" class="fiori-form__label">Remark</label>
                <textarea id="remark" name="remark" class="fiori-input" rows="3"></textarea>
                <div class="fiori-form__help">You can record factory information.</div>
              </div>              
              
              <!-- Step 3: Review -->
              <div style="padding: var(--sap-spacing-md); background: var(--sap-surface-2); border-radius: var(--sap-radius-md); margin-top: var(--sap-spacing-md);">
                <h5 style="margin: 0 0 var(--sap-spacing-sm) 0; color: var(--sap-text-primary);">💡 Confirm Information</h5>
                <div style="font-size: var(--sap-font-size-sm); color: var(--sap-text-secondary);">
                  <div><strong>Factory name:</strong> <span id="previewName">-</span></div>
                  <div><strong>Status:</strong> <span id="previewStatus">-</span></div>
                </div>
              </div>
            </div>
            
            <div class="fiori-form__actions">
              <button type="button" class="fiori-btn fiori-btn--tertiary" id="modalCloseBtn">Cancel</button>
              <button type="button" class="fiori-btn fiori-btn--secondary" id="prevStep" style="display: none;">← Previous</button>
              <button type="button" class="fiori-btn fiori-btn--primary" id="nextStep">Next →</button>
              <button type="submit" class="fiori-btn fiori-btn--primary" id="submitBtn" style="display: none;">💾 Save</button>
            </div>
          </form>
        </div>
      </div>
    </div>
  </div>


  <script type="module">
    // Resource Manager 및 팩토리 모듈 가져오기
    import { createResourceManager } from '../../assets/js/resource-manager.js';
    import { initAdvancedFeatures, factoryConfig } from './js/info_factory.js';
    import { initStatsToggle } from '../../assets/js/manage/stats-toggle.js';

    // 전역 변수로 resourceManager 선언
    let resourceManager;

    // DOM 로딩 완료 후 초기화
    document.addEventListener('DOMContentLoaded', function() {
        
      resourceManager = createResourceManager(factoryConfig);
      
      setTimeout(() => {
        initAdvancedFeatures(resourceManager);
      }, 100);
  
      // Stats toggle 초기화 추가
      initStatsToggle();
    });
    
  </script>

</body>
</html>