<?php
// 페이지별 설정
$page_title = 'Line Management';
$page_css_files = ['../../assets/css/fiori-page.css', 'css/info_line.css'];
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
          <h2>Line Management</h2>
          <!-- <p style="color: var(--sap-text-secondary); margin: var(--sap-spacing-xs) 0 0 0;">
            공장 정보 관리 및 현황 모니터링
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
            <!-- Filled by JS -->
          </select>
          <select id="statusFilterSelect" class="fiori-select" style="width: auto; min-width: 150px;">
            <option value="" selected> All Status</option>
            <option value="Y">✅ Used Only</option>
            <option value="N">⚠️ Unused Only</option>
          </select>
          <div class="fiori-dropdown">
            <button id="columnToggleBtn" class="fiori-btn fiori-btn--secondary"> Columns</button>
            <div id="columnToggleDropdown" class="fiori-dropdown__content">
              <!-- JS로 컬럼 체크박스 생성 --> 
            </div>                 
          </div>
          <button id="addBtn" class="fiori-btn fiori-btn--primary"> Add Line</button>
          <button id="refreshBtn" class="fiori-btn fiori-btn--tertiary">🔄 Refresh</button>
          <!-- <button id="toggleStatsBtn" class="fiori-btn fiori-btn--secondary">📊 Show Stats</button> -->
        </div>
      </div>

      <!-- 통계 요약 카드 -->
      <div class="factory-stats-grid" id="statsGrid" style="display: none;">
        <div class="stat-card stat-card--info">
          <div class="stat-value" id="totalCount">-</div>
          <div class="stat-label"> Total Lines</div>
        </div>
        <div class="stat-card stat-card--green">
          <div class="stat-value" id="totalMachines">0</div>
          <div class="stat-label"> Total Machines</div>
        </div>
        <div class="stat-card stat-card--warning">
          <div class="stat-value" id="totalManpower">0</div>
          <div class="stat-label"> Total Manpower</div>
        </div>
        <div class="stat-card stat-card--purple">
          <div class="stat-value" id="totalTarget">0</div>
          <div class="stat-label"> Total Target</div>
        </div>
        <div class="stat-card stat-card--accent">
          <div class="stat-value" id="targetPerMan">0</div>
          <div class="stat-label"> Target per Man</div>
        </div>
      </div>

      <!-- 데이터 테이블 섹션 -->
      <div class="fiori-section">
        <div class="fiori-card">
          <div class="fiori-card__header">
            <div>
              <h3 class="fiori-card__title">Line List</h3>
              <!-- <p class="fiori-card__subtitle">전체 공장 목록 및 상태 관리</p> -->
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
              <!-- <div style="font-size: 3rem; margin-bottom: var(--sap-spacing-md);">🏭</div> -->
              <p>There are no line matching your search criteria.</p>
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

  <!-- SAP Fiori Line Modal -->
  <div id="resourceModal" class="fiori-modal">
    <div class="fiori-modal__content">
      <div class="fiori-card" style="max-height: calc(90vh - 2rem); overflow-y: auto; display: flex; flex-direction: column;">
        <div class="fiori-card__header">
          <div>
            <h3 class="fiori-card__title" id="modalTitle"> Line Information</h3>
            <p class="fiori-card__subtitle">Enter and edit factory information</p>
          </div>
          <button type="button" class="fiori-btn fiori-btn--tertiary fiori-btn--sm close" 
                  style="position: absolute; top: var(--sap-spacing-md); right: var(--sap-spacing-md);">
            ✕
          </button>
        </div>
        <div class="fiori-card__content" style="flex: 1; overflow-y: auto; max-height: calc(90vh - 8rem);">
          <form id="resourceForm" class="fiori-form">
            <input type="hidden" id="resourceId" name="idx">
            
            <!-- Progress indicator -->
            <div class="progress-indicator">
              <div class="progress-steps">
                <div class="form-step active" data-step="1">1</div>
                <div class="form-step-line"></div>
                <div class="form-step" data-step="2">2</div>
                <div class="form-step-line"></div>
                <div class="form-step" data-step="3">3</div>
              </div>
              <div class="step-counter">
                <span id="stepIndicator">Step 1 of 3</span>
              </div>
            </div>
            
            <!-- Step 1: 필수 정보 -->
            <div class="form-section" data-section="1">
              <h4 style="margin-bottom: var(--sap-spacing-md); color: var(--sap-text-primary);">Basic information</h4>
              
              <div class="fiori-form__group">
                <label for="factory_name" class="fiori-form__label fiori-form__label--required">Factory Name</label>
                <div class="fiori-form-group">
                  <select id="factory_idx" name="factory_idx" class="fiori-select" required>
                    <!-- Filled by JS -->
                  </select>
                  <div class="fiori-form__help">Select the factory where this line is located</div>
                </div>
              </div>              
              
              <div class="fiori-form__group">
                <label for="line_name" class="fiori-form__label fiori-form__label--required">Line Name</label>
                <input type="text" id="line_name" name="line_name" class="fiori-input" required 
                       placeholder="Please enter the line name">
                <div class="fiori-form__help">Please enter a unique line name (2-50 characters)</div>
              </div>
              
              <div class="fiori-form__group">
                <label for="status" class="fiori-form__label">Status</label>
                <select id="status" name="status" class="fiori-select">
                  <option value="Y">✅ Used</option>
                  <option value="N">⚠️ Unused</option>
                </select>
                <div class="fiori-form__help">Please select the current usage status of the line.</div>
              </div>
            </div>
            
            <!-- Step 2: 추가 정보 -->       
            <div class="form-section" data-section="2" style="display: none;">
              <h4 style="margin-bottom: var(--sap-spacing-md); color: var(--sap-text-primary);">Additional information</h4>             
              
              <div class="fiori-form__group">
                <label for="mp" class="fiori-form__label fiori-form__label--required">Man Power</label>
                <input type="number" id="mp" name="mp" class="fiori-input" required value="0" min="0">
                <div class="fiori-form__help">Number of workers assigned to this line</div>
              </div>

              <div class="fiori-form__group">
                <label for="line_target" class="fiori-form__label fiori-form__label--required">Line Target</label>
                <input type="number" id="line_target" name="line_target" class="fiori-input" required value="0" min="0">
                <div class="fiori-form__help">Daily production target for this line</div>
              </div>
            </div>  
            
            <!-- Step 3: 비고 정보 -->
            <div class="form-section" data-section="3" style="display: none;">
              <h4 style="margin-bottom: var(--sap-spacing-md); color: var(--sap-text-primary);">Remark Information</h4>
              
              <div class="fiori-form__group">
                <label for="remark" class="fiori-form__label">Remark</label>
                <textarea id="remark" name="remark" class="fiori-input" rows="3" 
                         placeholder=""></textarea>
                <div class="fiori-form__help">You can record line information.</div>
              </div>   

              <!-- Step 3: Review -->
              <div style="padding: var(--sap-spacing-md); background: var(--sap-surface-2); border-radius: var(--sap-radius-md); margin-top: var(--sap-spacing-md);">
                <h5 style="margin: 0 0 var(--sap-spacing-sm) 0; color: var(--sap-text-primary);">💡 Confirm input information</h5>
                <div style="font-size: var(--sap-font-size-sm); color: var(--sap-text-secondary);">
                  <div><strong>Factory name:</strong> <span id="previewName">-</span></div>
                  <div><strong>Line Name:</strong> <span id="previewLine">-</span></div>
                  <div><strong>Man Power:</strong> <span id="previewMP">-</span></div>
                  <div><strong>Line Target:</strong> <span id="previewTarget">-</span></div>
                  <div><strong>Status:</strong> <span id="previewStatus">-</span></div>
                </div>
              </div>
            </div>
            
            <div class="fiori-form__actions">
              <button type="button" class="fiori-btn fiori-btn--tertiary" id="modalCloseBtn">Cancel</button>
              <button type="button" class="fiori-btn fiori-btn--secondary" id="prevStep" style="display: none;">← Previous</button>
              <button type="button" class="fiori-btn fiori-btn--primary" id="nextStep">Next →</button>
              <button type="submit" class="fiori-btn fiori-btn--primary" id="submitBtn" style="display: none;">💾 Save Factory</button>
            </div>
          </form>
        </div>
      </div>
    </div>
  </div>


  <script type="module">
    // Resource Manager 및 라인 모듈 가져오기
    import { createResourceManager } from '../../assets/js/resource-manager.js';
    import { initAdvancedFeatures, lineConfig } from './js/info_line.js';
    import { initStatsToggle } from '../../assets/js/manage/stats-toggle.js';

    // 전역 변수로 resourceManager 선언
    let resourceManager;

    // DOM 로딩 완료 후 초기화
    document.addEventListener('DOMContentLoaded', function() {
      // DOM 로딩 완료, 초기화 시작
      
      // ResourceManager 생성 - DOMContentLoaded 내부에서 호출하므로 즉시 실행
      resourceManager = createResourceManager(lineConfig);
      
      // ResourceManager 초기화 완료를 기다린 후 고급 기능 초기화
      // 짧은 지연을 두어 ResourceManager가 완전히 초기화되도록 함
      setTimeout(() => {
        initAdvancedFeatures(resourceManager);
      }, 100);
  
      // Stats toggle 초기화 추가
      initStatsToggle();
    });
    
    // fiori-page.css에서 모든 공통 스타일 처리됨
    // Line 페이지 초기화 완료
  </script>

</body>
</html>