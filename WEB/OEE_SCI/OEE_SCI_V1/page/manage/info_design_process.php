<?php
// 페이지별 설정
$page_title = 'Design Process Management';
$page_css_files = [
  '../../assets/css/fiori-page.css',
  'css/info_design_process.css'
];

// 공통 헤더 로드
require_once(__DIR__ . '/../../inc/head.php');
require_once(__DIR__ . '/../../inc/nav-fiori.php');
?>

  <div class="fiori-container">
    <main>
      
      <!-- 페이지 헤더 -->
      <div class="fiori-main-header">
        <div>
          <h2>Design Process Management</h2>
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
          <select id="statusFilterSelect" class="fiori-select" style="width: auto; min-width: 150px;">
            <option value=""> All Status</option>
            <option value="Y">✅ Used Only</option>
            <option value="N">⚠️ Unused Only</option>
          </select>
          <div class="fiori-dropdown">
            <button id="columnToggleBtn" class="fiori-btn fiori-btn--secondary"> Columns</button>
            <div id="columnToggleDropdown" class="fiori-dropdown__content">
              <!-- JS로 컬럼 체크박스 생성 -->
            </div>
          </div>
          <button id="addBtn" class="fiori-btn fiori-btn--primary"> Add Process</button>
          <button id="refreshBtn" class="fiori-btn fiori-btn--tertiary">🔄 Refresh</button>
          <!-- <button id="toggleStatsBtn" class="fiori-btn fiori-btn--secondary">📊 Show Stats</button> -->
          <button id="toggleSetProcessBtn" class="fiori-btn fiori-btn--secondary">⚙️ Set Process</button>
        </div>
      </div>

      <!-- 통계 요약 카드 -->
      <div class="factory-stats-grid" id="statsGrid" style="display: none;">
        <div class="stat-card">
          <div class="stat-value" id="totalCount">-</div>
          <div class="stat-label">📊 Total Processes</div>
        </div>
        <div class="stat-card stat-card--success">
          <div class="stat-value" id="activeCount">-</div>
          <div class="stat-label">✅ Used</div>
        </div>
        <div class="stat-card stat-card--warning">
          <div class="stat-value" id="totalNeedMC">-</div>
          <div class="stat-label">⚙️ Total Need MC</div>
          <div class="stat-description">std_mc_needed sum</div>
        </div>
        <div class="stat-card stat-card--info">
          <div class="stat-value" id="agvNeedMC">-</div>
          <div class="stat-label">🤖 AGV Need MC</div>
          <div class="stat-description">std_mc_needed > 0 average</div>
        </div>
      </div>

      <!-- Process-Machine 매핑 영역 -->
      <div class="fiori-section" id="processMachineSection" style="display: none;">
        <div class="fiori-card">
          <div class="fiori-card__header">
            <div>
              <h3 class="fiori-card__title">⚙️ Process-Machine Assignment</h3>
              <p class="fiori-card__subtitle">Assign Machines to Processes by dragging and dropping.</p>
            </div>
            <div class="fiori-header-actions">
              <button id="saveProcessMachineBtn" class="fiori-btn fiori-btn--primary">💾 Save Changes</button>
              <button id="cancelProcessMachineBtn" class="fiori-btn fiori-btn--tertiary">✕ Close</button>
            </div>
          </div>

          <!-- Line 필터 버튼 바 -->
          <div class="line-filter-bar" id="lineFilterBar">
            <span class="line-filter-label">📍 Filter by Line:</span>
            <!-- JavaScript로 동적 생성 -->
          </div>

          <div class="fiori-card__content">
            <div id="processMachineContainer" class="process-machine-container">
              <!-- JavaScript로 동적 생성 -->
            </div>
          </div>
        </div>
      </div>

      <!-- 데이터 테이블 섹션 -->
      <div class="fiori-section">
        <div class="fiori-card">
          <div class="fiori-card__header">
            <div>
              <h3 class="fiori-card__title">Design Process List</h3>
            </div>
            <!-- 빠른 액션 바를 헤더 오른쪽으로 이동 -->
            <div class="quick-actions">
              <button class="quick-action-btn active" data-filter="all"> All</button>
              <button class="quick-action-btn" data-filter="used">✅ Used</button>
              <button class="quick-action-btn" data-filter="unused">⚠️ Unused</button>
              <!-- <button class="quick-action-btn" data-filter="recent">🕒 Recent</button> -->
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
              <!-- <div style="font-size: 3rem; margin-bottom: var(--sap-spacing-md);">🔄</div> -->
              <p>There are no design processes matching your search criteria.</p>
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

  <!-- SAP Fiori Design Process Modal -->
  <div id="resourceModal" class="fiori-modal">
    <div class="fiori-modal__content">
      <div class="fiori-card">
        <div class="fiori-card__header">
          <div>
            <h3 class="fiori-card__title" id="modalTitle"> Design Process Information</h3>
            <p class="fiori-card__subtitle">Enter and edit design process information</p>
          </div>
          <button type="button" class="fiori-btn fiori-btn--tertiary fiori-btn--sm close" 
                  style="position: absolute; top: var(--sap-spacing-md); right: var(--sap-spacing-md);">
            ✕
          </button>
        </div>
        <div class="fiori-card__content">
          <form id="resourceForm" class="fiori-form" enctype="multipart/form-data">
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
                <label for="factory_idx" class="fiori-form__label fiori-form__label--required">Factory</label>
                <select id="factory_idx" name="factory_idx" class="fiori-select" required>
                  <option value="">Please select a factory</option>
                  <!-- JS로 채워짐 -->
                </select>
                <div class="fiori-form__help">Please select a factory</div>
              </div>

              <div class="fiori-form__group">
                <label for="line_idx" class="fiori-form__label fiori-form__label--required">Line</label>
                <select id="line_idx" name="line_idx" class="fiori-select" required disabled>
                  <option value="">Please select a line</option>
                  <!-- JS로 채워짐 -->
                </select>
                <div class="fiori-form__help">Select line after selecting factory</div>
              </div>

              <div class="fiori-form__group">
                <label for="model_name" class="fiori-form__label">Model Name</label>
                <input type="text" id="model_name" name="model_name" class="fiori-input"
                       placeholder="Enter product model name">
                <div class="fiori-form__help">Enter the product model name for this process (e.g., AIR MAX MOTO 2K (W))</div>
              </div>

              <div class="fiori-form__group">
                <label for="design_process" class="fiori-form__label fiori-form__label--required">Design Process Name</label>
                <input type="text" id="design_process" name="design_process" class="fiori-input" required
                       placeholder="Please enter the design process name">
                <div class="fiori-form__help">Please enter a unique design process name (2-50 characters)</div>
              </div>
              
              <div class="fiori-form__group">
                <label for="std_mc_needed" class="fiori-form__label">Standard MC Needed</label>
                <input type="number" id="std_mc_needed" name="std_mc_needed" class="fiori-input" value="1" min="1" 
                       placeholder="Enter the number of machines needed">
                <div class="fiori-form__help">Please enter the number of standard machines needed for this process (default: 1)</div>
              </div>
              
              <div class="fiori-form__group">
                <label for="status" class="fiori-form__label">Status</label>
                <select id="status" name="status" class="fiori-select">
                  <option value="Y">✅ Used</option>
                  <option value="N">⚠️ Unused</option>
                </select>
                <div class="fiori-form__help">Please select the current usage status of the design process.</div>
              </div>
            </div>
            
            <!-- Step 2: 추가 정보 -->
            <div class="form-section" data-section="2" style="display: none;">
              <h4 style="margin-bottom: var(--sap-spacing-md); color: var(--sap-text-primary);">Additional information</h4>
              
              <div class="fiori-form__group">
                <label for="file_upload" class="fiori-form__label">SOP File Upload</label>
                
                <!-- 기존 파일 정보 표시 영역 -->
                <div id="existingFileInfo" style="display: none; margin-bottom: var(--sap-spacing-sm); padding: var(--sap-spacing-sm); background: var(--sap-surface-2); border-radius: var(--sap-radius-sm); border-left: 3px solid var(--sap-status-info);">
                  <div style="display: flex; align-items: center; justify-content: space-between;">
                    <div>
                      <span style="font-size: var(--sap-font-size-sm); color: var(--sap-text-secondary);">Current file:</span>
                      <br>
                      <span id="existingFileName" style="font-weight: 500; color: var(--sap-text-primary);"></span>
                    </div>
                    <button type="button" id="viewExistingFileBtn" class="fiori-btn fiori-btn--tertiary fiori-btn--sm" style="min-width: auto;">
                      👁️ View
                    </button>
                  </div>
                </div>
                
                <input type="file" id="file_upload" name="file_upload" class="fiori-input" 
                       accept=".jpg,.jpeg,.png">
                <div class="fiori-form__help">Upload SOP file (JPG, JPEG, PNG only, max 10MB). Leave empty to keep existing file.</div>
              </div>
              
              <div class="fiori-form__group">
                <label for="remark" class="fiori-form__label">Remark</label>
                <textarea id="remark" name="remark" class="fiori-input" rows="3" 
                         placeholder=""></textarea>
                <div class="fiori-form__help">You can record design process information.</div>
              </div>              
              
              <!-- Step 3: Review -->
              <div style="padding: var(--sap-spacing-md); background: var(--sap-surface-2); border-radius: var(--sap-radius-md); margin-top: var(--sap-spacing-md);">
                <h5 style="margin: 0 0 var(--sap-spacing-sm) 0; color: var(--sap-text-primary);">💡 Confirm input information</h5>
                <div style="font-size: var(--sap-font-size-sm); color: var(--sap-text-secondary);">
                  <div><strong>Process name:</strong> <span id="previewName">-</span></div>
                  <div><strong>Standard MC needed:</strong> <span id="previewStdMc">-</span></div>
                  <div><strong>Status:</strong> <span id="previewStatus">-</span></div>
                  <div><strong>File:</strong> <span id="previewFile">-</span></div>
                </div>
              </div>
            </div>
            
            <div class="fiori-form__actions">
              <button type="button" class="fiori-btn fiori-btn--tertiary" id="modalCloseBtn">Cancel</button>
              <button type="button" class="fiori-btn fiori-btn--secondary" id="prevStep" style="display: none;">← Previous</button>
              <button type="button" class="fiori-btn fiori-btn--primary" id="nextStep">Next →</button>
              <button type="submit" class="fiori-btn fiori-btn--primary" id="submitBtn" style="display: none;">💾 Save Process</button>
            </div>
          </form>
        </div>
      </div>
    </div>
  </div>

  <!-- SOP 이미지 미리보기 모달 -->
  <div id="imagePreviewModal" class="fiori-modal" style="z-index: 9999;">
    <div class="fiori-modal__content" style="max-width: 90vw; max-height: 90vh; width: auto; height: auto;">
      <div class="fiori-card" style="height: 100%; display: flex; flex-direction: column;">
        <div class="fiori-card__header">
          <div>
            <h3 class="fiori-card__title">🖼️ SOP File Preview</h3>
            <p class="fiori-card__subtitle" id="imageFileName">Image preview</p>
          </div>
          <button type="button" class="fiori-btn fiori-btn--tertiary fiori-btn--sm" 
                  id="imageModalClose"
                  style="position: absolute; top: var(--sap-spacing-md); right: var(--sap-spacing-md);">
            ✕
          </button>
        </div>
        <div class="fiori-card__content" style="flex: 1; display: flex; align-items: center; justify-content: center; padding: var(--sap-spacing-lg);">
          <img id="previewImage" src="" alt="SOP Preview" 
               style="max-width: 100%; max-height: 100%; object-fit: contain; border-radius: var(--sap-radius-md); box-shadow: 0 4px 12px rgba(0, 0, 0, 0.15);">
        </div>
        <div class="fiori-card__content" style="padding-top: 0; text-align: center;">
          <button type="button" class="fiori-btn fiori-btn--primary" id="imageModalCloseBtn">
            ✔️ Close
          </button>
        </div>
      </div>
    </div>
  </div>

  <script type="module">
    // Resource Manager 및 디자인프로세스 모듈 가져오기
    import { createResourceManager } from '../../assets/js/resource-manager.js';
    import { initAdvancedFeatures, designProcessConfig } from './js/info_design_process.js';
    import { initStatsToggle } from '../../assets/js/manage/stats-toggle.js';

    // 전역 변수로 resourceManager 선언
    let resourceManager;

    // DOM 로딩 완료 후 초기화
    document.addEventListener('DOMContentLoaded', function() {
      // Initialize ResourceManager

      // ResourceManager 생성 - DOMContentLoaded 내부에서 호출하므로 즉시 실행
      resourceManager = createResourceManager(designProcessConfig);

      // ResourceManager 초기화 완료를 기다린 후 고급 기능 초기화
      // 짧은 지연을 두어 ResourceManager가 완전히 초기화되도록 함
      setTimeout(() => {
        initAdvancedFeatures(resourceManager);

        // SOP 파일 업로드 및 Preview 기능 초기화
        initSopFileUpload();

        // Process-Machine 매핑 기능 초기화
        initProcessMachineMapping();
      }, 100);

      // Stats toggle 초기화 추가
      initStatsToggle();
    });
    
    // SOP 파일 업로드 기능
    function initSopFileUpload() {
      const fileInput = document.getElementById('file_upload');
      const imageModalClose = document.getElementById('imageModalClose');
      const imageModalCloseBtn = document.getElementById('imageModalCloseBtn');
      const imagePreviewModal = document.getElementById('imagePreviewModal');
      const existingFileInfo = document.getElementById('existingFileInfo');
      const existingFileName = document.getElementById('existingFileName');
      const viewExistingFileBtn = document.getElementById('viewExistingFileBtn');
      const previewImage = document.getElementById('previewImage');
      const imageFileName = document.getElementById('imageFileName');
      
      let currentExistingFile = null; // 현재 데이터의 기존 파일
      
      // 파일 입력 변경 이벤트 (기본 검증만 수행)
      fileInput.addEventListener('change', function(e) {
        const file = e.target.files[0];
        if (file) {
          // JPG, JPEG, PNG 파일만 허용
          const allowedTypes = ['image/jpeg', 'image/jpg', 'image/png'];
          if (!allowedTypes.includes(file.type)) {
            alert('올바르지 않은 파일 형식입니다. JPG, JPEG, PNG 파일만 업로드 가능합니다.');
            fileInput.value = '';
            return;
          }
          
          // 10MB 크기 제한 확인
          const maxSize = 10 * 1024 * 1024; // 10MB
          if (file.size > maxSize) {
            alert('파일 크기가 10MB를 초과합니다.');
            fileInput.value = '';
            return;
          }
        }
      });
      
      // 기존 파일 보기 버튼 클릭 이벤트
      if (viewExistingFileBtn) {
        viewExistingFileBtn.addEventListener('click', function() {
          if (currentExistingFile) {
            previewImage.src = `../../upload/sop/${currentExistingFile}`;
            imageFileName.textContent = currentExistingFile;
            imagePreviewModal.style.display = 'flex';
            
            setTimeout(() => {
              imagePreviewModal.classList.add('show');
            }, 10);
          }
        });
      }
      
      // 이미지 미리보기 모달 닫기 이벤트
      function closeImagePreviewModal() {
        imagePreviewModal.classList.remove('show');
        setTimeout(() => {
          imagePreviewModal.style.display = 'none';
        }, 300); // 애니메이션 시간에 맞춰 지연
      }
      
      [imageModalClose, imageModalCloseBtn].forEach(btn => {
        btn.addEventListener('click', closeImagePreviewModal);
      });
      
      // 모달 배경 클릭으로 닫기
      imagePreviewModal.addEventListener('click', function(e) {
        if (e.target === imagePreviewModal) {
          closeImagePreviewModal();
        }
      });
      
      // 기존 파일 정보 업데이트 함수
      window.updateExistingFileInfo = function(filename) {
        currentExistingFile = filename;
        
        if (filename && filename.trim() !== '') {
          existingFileName.textContent = filename;
          existingFileInfo.style.display = 'block';
        } else {
          existingFileInfo.style.display = 'none';
        }
      };
      
      // 기존 파일 정보 초기화 함수
      window.resetExistingFileInfo = function() {
        currentExistingFile = null;
        existingFileInfo.style.display = 'none';
        fileInput.value = '';
      };
    }

    // Process-Machine 매핑 기능 초기화
    function initProcessMachineMapping() {
      const toggleBtn = document.getElementById('toggleSetProcessBtn');
      const section = document.getElementById('processMachineSection');
      const container = document.getElementById('processMachineContainer');
      const lineFilterBar = document.getElementById('lineFilterBar');
      const saveBtn = document.getElementById('saveProcessMachineBtn');
      const cancelBtn = document.getElementById('cancelProcessMachineBtn');

      let mappingData = null;
      let currentAssignments = {}; // {machine_idx: design_process_idx}
      let selectedLine = null; // 현재 선택된 Line (line_name)

      // 자동 스크롤 관련 변수
      let autoScrollInterval = null;
      const SCROLL_ZONE_SIZE = 80; // 스크롤 트리거 영역 크기 (px)
      const SCROLL_SPEED = 10; // 스크롤 속도 (px per frame)

      // 토글 버튼 클릭 이벤트
      toggleBtn.addEventListener('click', async function() {
        if (section.style.display === 'none') {
          section.style.display = 'block';
          toggleBtn.textContent = '⚙️ Hide Process';

          // 데이터 로드 및 렌더링
          await loadMappingData();
        } else {
          section.style.display = 'none';
          toggleBtn.textContent = '⚙️ Set Process';
        }
      });

      // 취소 버튼
      cancelBtn.addEventListener('click', function() {
        section.style.display = 'none';
        toggleBtn.textContent = '⚙️ Set Process';
      });

      // 저장 버튼
      saveBtn.addEventListener('click', async function() {
        await saveMachineAssignments();
      });

      // 데이터 로드 함수
      async function loadMappingData() {
        try {
          const response = await fetch('proc/process_machine_mapping.php?action=get_mapping');
          const result = await response.json();

          if (result.success) {
            mappingData = result.data;

            // 현재 할당 상태를 저장
            currentAssignments = {};
            mappingData.machines.forEach(machine => {
              currentAssignments[machine.idx] = machine.design_process_idx || 0;
            });

            // Line 필터 바 렌더링
            renderLineFilterBar();

            // UI 렌더링
            renderMappingUI();
          } else {
            alert('데이터 로드 실패: ' + result.message);
          }
        } catch (error) {
          console.error('Error loading mapping data:', error);
          alert('데이터 로드 중 오류가 발생했습니다.');
        }
      }

      // Line 필터 바 렌더링 함수
      function renderLineFilterBar() {
        if (!mappingData) return;

        // 고유한 Line 목록 추출 (Factory + Line 조합)
        const lineMap = new Map();
        mappingData.machines.forEach(machine => {
          const lineKey = `${machine.factory_name || 'N/A'}-${machine.line_name || 'N/A'}`;
          const lineDisplay = `${machine.factory_name || 'N/A'} / ${machine.line_name || 'N/A'}`;

          if (!lineMap.has(lineKey)) {
            lineMap.set(lineKey, {
              key: lineKey,
              display: lineDisplay,
              factory: machine.factory_name || 'N/A',
              line: machine.line_name || 'N/A',
              count: 0
            });
          }
          lineMap.get(lineKey).count++;
        });

        const lines = Array.from(lineMap.values()).sort((a, b) =>
          a.display.localeCompare(b.display)
        );

        // Line 필터 버튼 생성
        const filterBarContent = lineFilterBar.querySelector('.line-filter-label');

        // 기존 버튼들 제거 (label 제외)
        while (filterBarContent.nextSibling) {
          lineFilterBar.removeChild(filterBarContent.nextSibling);
        }

        // 각 Line별 버튼 추가
        lines.forEach((lineInfo, index) => {
          const btn = document.createElement('button');
          btn.className = 'line-filter-btn';
          btn.dataset.lineKey = lineInfo.key;
          btn.innerHTML = `
            📍 ${lineInfo.display}
            <span class="line-filter-count">${lineInfo.count}</span>
          `;
          btn.addEventListener('click', () => selectLine(lineInfo.key));
          lineFilterBar.appendChild(btn);

          // 첫 번째 Line을 기본 선택
          if (index === 0 && selectedLine === null) {
            selectedLine = lineInfo.key;
            btn.classList.add('active');
          }
        });
      }

      // Line 선택 함수
      function selectLine(lineKey) {
        selectedLine = lineKey;

        // 버튼 활성화 상태 업데이트
        document.querySelectorAll('.line-filter-btn').forEach(btn => {
          if (btn.dataset.lineKey === lineKey) {
            btn.classList.add('active');
          } else {
            btn.classList.remove('active');
          }
        });

        // UI 다시 렌더링
        renderMappingUI();
      }

      // Line별 Machine 필터링 함수
      function getFilteredMachines() {
        if (!selectedLine) return [];

        return mappingData.machines.filter(machine => {
          const lineKey = `${machine.factory_name || 'N/A'}-${machine.line_name || 'N/A'}`;
          return lineKey === selectedLine;
        });
      }

      // Line별 Process 필터링 함수
      function getFilteredProcesses() {
        if (!selectedLine) return [];

        return mappingData.processes.filter(process => {
          const lineKey = `${process.factory_name || 'N/A'}-${process.line_name || 'N/A'}`;
          return lineKey === selectedLine;
        });
      }

      // UI 렌더링 함수
      function renderMappingUI() {
        if (!mappingData) return;

        container.innerHTML = '';

        // 현재 선택된 Line의 Process와 Machine만 가져오기
        const filteredProcesses = getFilteredProcesses();
        const filteredMachines = getFilteredMachines();

        // 각 process별로 row 생성 (필터링된 process만)
        filteredProcesses.forEach(process => {
          const row = createProcessRow(process, filteredMachines);
          container.appendChild(row);
        });

        // Empty row 생성 (미할당 머신들)
        const emptyRow = createEmptyRow(filteredMachines);
        container.appendChild(emptyRow);
      }

      // Process Row 생성
      function createProcessRow(process, filteredMachines) {
        const row = document.createElement('div');
        row.className = 'process-row';
        row.dataset.processIdx = process.idx;

        // Process 박스
        const processBox = document.createElement('div');
        processBox.className = 'process-box';
        processBox.dataset.processIdx = process.idx;
        processBox.dataset.stdMcNeeded = process.std_mc_needed;

        // 현재 할당된 machine 개수 계산 (필터링된 Machine 중에서)
        const currentMachineCount = filteredMachines.filter(m =>
          parseInt(currentAssignments[m.idx]) === parseInt(process.idx)
        ).length;
        const availableSlots = process.std_mc_needed - currentMachineCount;
        const isFull = currentMachineCount >= process.std_mc_needed;

        // View 모드
        const viewDiv = document.createElement('div');
        viewDiv.className = 'process-box-view';
        viewDiv.innerHTML = `
          <div class="process-box-header">
            <div class="process-name">${escapeHtml(process.design_process)}</div>
            <div class="process-capacity">
              Capacity: <span class="process-capacity-badge ${isFull ? 'full' : ''}">${process.std_mc_needed}</span>
            </div>
            <div class="machine-count">
              Assigned: <span class="machine-count-current">${currentMachineCount}</span> /
              Available: <span class="machine-count-available ${isFull ? 'full' : ''}">${availableSlots}</span>
            </div>
          </div>
        `;

        // Edit 모드
        const editDiv = document.createElement('div');
        editDiv.className = 'process-box-edit';
        editDiv.innerHTML = `
          <input type="text" class="fiori-input" value="${escapeHtml(process.design_process)}" placeholder="Process Name">
          <input type="number" class="fiori-input" value="${process.std_mc_needed}" min="1" placeholder="Capacity">
          <div class="edit-actions">
            <button class="fiori-btn fiori-btn--primary fiori-btn--sm save-process">Save</button>
            <button class="fiori-btn fiori-btn--tertiary fiori-btn--sm cancel-edit">Cancel</button>
          </div>
        `;

        processBox.appendChild(viewDiv);
        processBox.appendChild(editDiv);

        // Process 박스 클릭 이벤트 (편집 모드)
        processBox.addEventListener('click', function(e) {
          if (!processBox.classList.contains('editing') && e.target === processBox || e.target.closest('.process-box-view')) {
            processBox.classList.add('editing');
          }
        });

        // 저장 버튼
        const saveProcessBtn = editDiv.querySelector('.save-process');
        saveProcessBtn.addEventListener('click', async function(e) {
          e.stopPropagation();
          await saveProcessInfo(processBox, process.idx);
        });

        // 취소 버튼
        const cancelEditBtn = editDiv.querySelector('.cancel-edit');
        cancelEditBtn.addEventListener('click', function(e) {
          e.stopPropagation();
          processBox.classList.remove('editing');
          // 원래 값으로 복원
          editDiv.querySelectorAll('input')[0].value = process.design_process;
          editDiv.querySelectorAll('input')[1].value = process.std_mc_needed;
        });

        row.appendChild(processBox);

        // Machines Container (드롭존)
        const machinesContainer = document.createElement('div');
        machinesContainer.className = 'machines-container drop-zone';
        machinesContainer.dataset.processIdx = process.idx;
        machinesContainer.dataset.stdMcNeeded = process.std_mc_needed;

        // 해당 process에 할당된 machine들 렌더링 (필터링된 Machine 중에서)
        const assignedMachines = filteredMachines.filter(m =>
          parseInt(currentAssignments[m.idx]) === parseInt(process.idx)
        );

        assignedMachines.forEach(machine => {
          const machineBox = createMachineBox(machine);
          machinesContainer.appendChild(machineBox);
        });

        // 드롭존 이벤트
        setupDropZone(machinesContainer);

        row.appendChild(machinesContainer);

        return row;
      }

      // Empty Row 생성
      function createEmptyRow(filteredMachines) {
        const row = document.createElement('div');
        row.className = 'process-row';

        // Empty 박스
        const emptyBox = document.createElement('div');
        emptyBox.className = 'process-box empty';
        emptyBox.textContent = 'Unassigned';

        row.appendChild(emptyBox);

        // Machines Container (미할당 머신들)
        const machinesContainer = document.createElement('div');
        machinesContainer.className = 'machines-container drop-zone';
        machinesContainer.dataset.processIdx = '0';

        const unassignedMachines = filteredMachines.filter(m =>
          !currentAssignments[m.idx] || parseInt(currentAssignments[m.idx]) === 0
        );

        unassignedMachines.forEach(machine => {
          const machineBox = createMachineBox(machine);
          machinesContainer.appendChild(machineBox);
        });

        // 드롭존 이벤트
        setupDropZone(machinesContainer);

        row.appendChild(machinesContainer);

        return row;
      }

      // Machine Box 생성
      function createMachineBox(machine) {
        const box = document.createElement('div');
        box.className = 'machine-box';
        box.draggable = true;
        box.dataset.machineIdx = machine.idx;
        box.dataset.machineNo = machine.machine_no;

        // Factory와 Line 정보를 포함한 구조화된 HTML
        box.innerHTML = `
          <div class="machine-box-info">
            <div class="machine-box-location">
              <div class="machine-box-factory">${escapeHtml(machine.factory_name || 'N/A')}</div>
              <div class="machine-box-line">${escapeHtml(machine.line_name || 'N/A')}</div>
            </div>
            <div class="machine-box-number">🧵 ${escapeHtml(machine.machine_no)}</div>
          </div>
        `;

        // 드래그 이벤트
        box.addEventListener('dragstart', handleDragStart);
        box.addEventListener('dragend', handleDragEnd);

        return box;
      }

      // 자동 스크롤 시작 함수
      function startAutoScroll(direction) {
        // 이미 스크롤 중이면 중복 방지
        if (autoScrollInterval) return;

        autoScrollInterval = setInterval(() => {
          if (direction === 'up') {
            container.scrollTop -= SCROLL_SPEED;
          } else if (direction === 'down') {
            container.scrollTop += SCROLL_SPEED;
          }
        }, 16); // ~60fps
      }

      // 자동 스크롤 중지 함수
      function stopAutoScroll() {
        if (autoScrollInterval) {
          clearInterval(autoScrollInterval);
          autoScrollInterval = null;
        }
      }

      // 전역 드래그오버 핸들러 (자동 스크롤을 위한)
      function handleGlobalDragOver(e) {
        if (!container) return;

        const containerRect = container.getBoundingClientRect();
        const mouseY = e.clientY;

        // Container의 viewport 내에서 보이는 영역만 계산
        // (container가 viewport 밖으로 벗어난 경우를 처리)
        const visibleTop = Math.max(containerRect.top, 0);
        const visibleBottom = Math.min(containerRect.bottom, window.innerHeight);

        // 마우스가 컨테이너의 visible 영역 내에 있는지 확인
        if (mouseY >= visibleTop && mouseY <= visibleBottom) {
          // 상단 스크롤 존: visible 영역의 상단에서 SCROLL_ZONE_SIZE 이내
          if (mouseY - visibleTop < SCROLL_ZONE_SIZE) {
            startAutoScroll('up');
          }
          // 하단 스크롤 존: visible 영역의 하단에서 SCROLL_ZONE_SIZE 이내
          else if (visibleBottom - mouseY < SCROLL_ZONE_SIZE) {
            startAutoScroll('down');
          }
          // 스크롤 영역 벗어나면 중지
          else {
            stopAutoScroll();
          }
        } else {
          // 마우스가 컨테이너의 visible 영역 밖에 있으면 스크롤 중지
          stopAutoScroll();
        }
      }

      // 드롭존 설정
      function setupDropZone(dropZone) {
        dropZone.addEventListener('dragover', handleDragOver);
        dropZone.addEventListener('dragleave', handleDragLeave);
        dropZone.addEventListener('drop', handleDrop);
      }

      // 드래그 시작
      function handleDragStart(e) {
        e.currentTarget.classList.add('dragging');
        e.dataTransfer.effectAllowed = 'move';
        e.dataTransfer.setData('text/html', e.currentTarget.innerHTML);
        e.dataTransfer.setData('machine_idx', e.currentTarget.dataset.machineIdx);

        // 전역 dragover 이벤트 리스너 추가 (자동 스크롤을 위해)
        document.addEventListener('dragover', handleGlobalDragOver);
      }

      // 드래그 종료
      function handleDragEnd(e) {
        e.currentTarget.classList.remove('dragging');
        // 자동 스크롤 중지
        stopAutoScroll();

        // 전역 dragover 이벤트 리스너 제거
        document.removeEventListener('dragover', handleGlobalDragOver);
      }

      // 드래그 오버
      function handleDragOver(e) {
        if (e.preventDefault) {
          e.preventDefault();
        }
        e.dataTransfer.dropEffect = 'move';
        e.currentTarget.classList.add('drag-over');

        // 자동 스크롤은 전역 handleGlobalDragOver에서 처리
        return false;
      }

      // 드래그 떠남
      function handleDragLeave(e) {
        e.currentTarget.classList.remove('drag-over');
        // 드롭존을 완전히 벗어났을 때 자동 스크롤 중지
        // (다른 드롭존으로 이동할 수 있으므로 즉시 중지하지 않음)
      }

      // 드롭
      function handleDrop(e) {
        if (e.stopPropagation) {
          e.stopPropagation();
        }
        e.preventDefault();

        // 자동 스크롤 중지
        stopAutoScroll();

        // 전역 dragover 이벤트 리스너 제거 (안전을 위해)
        document.removeEventListener('dragover', handleGlobalDragOver);

        const dropZone = e.currentTarget;
        dropZone.classList.remove('drag-over');

        const machineIdx = e.dataTransfer.getData('machine_idx');
        const targetProcessIdx = parseInt(dropZone.dataset.processIdx || 0);
        const stdMcNeeded = parseInt(dropZone.dataset.stdMcNeeded || 999);

        // 드래그한 요소 찾기
        const draggedElement = document.querySelector(`.machine-box[data-machine-idx="${machineIdx}"]`);

        if (!draggedElement) return;

        // std_mc_needed 제한 체크 (empty가 아닌 경우)
        // Line별로 체크: 현재 선택된 Line의 Machine만 카운트
        if (targetProcessIdx !== 0) {
          // 현재 필터링된 Machine 중 해당 Process에 할당된 개수
          const filteredMachines = getFilteredMachines();
          const currentCountInLine = filteredMachines.filter(m =>
            parseInt(currentAssignments[m.idx]) === targetProcessIdx
          ).length;

          // 드래그 중인 Machine이 이미 해당 Process에 할당되어 있는지 확인
          const isAlreadyAssigned = parseInt(currentAssignments[machineIdx]) === targetProcessIdx;

          // 새로 할당하려는 경우에만 capacity 체크
          if (!isAlreadyAssigned && currentCountInLine >= stdMcNeeded) {
            alert(`The maximum number of Machine allocations for this Process in the current line is ${stdMcNeeded}.`);
            return;
          }
        }

        // Machine을 새로운 드롭존으로 이동
        dropZone.appendChild(draggedElement);

        // 할당 정보 업데이트
        currentAssignments[machineIdx] = targetProcessIdx;

        // Machine count 실시간 업데이트
        updateAllMachineCounts();

        return false;
      }

      // 모든 Process의 Machine Count 업데이트
      function updateAllMachineCounts() {
        // 필터링된 Machine 목록 가져오기
        const filteredMachines = getFilteredMachines();

        // 모든 process-row 순회
        document.querySelectorAll('.process-row').forEach(row => {
          const processIdx = parseInt(row.dataset.processIdx);

          // Empty row는 제외
          if (!processIdx && processIdx !== 0) return;

          const processBox = row.querySelector('.process-box:not(.empty)');
          if (!processBox) return;

          const stdMcNeeded = parseInt(processBox.dataset.stdMcNeeded);
          const dropZone = row.querySelector('.drop-zone');

          // 현재 할당된 machine 개수 계산 (현재 Line에서만)
          const currentMachineCount = dropZone.querySelectorAll('.machine-box').length;
          const availableSlots = stdMcNeeded - currentMachineCount;
          const isFull = currentMachineCount >= stdMcNeeded;

          // UI 업데이트
          const currentCountSpan = processBox.querySelector('.machine-count-current');
          const availableCountSpan = processBox.querySelector('.machine-count-available');
          const capacityBadge = processBox.querySelector('.process-capacity-badge');

          if (currentCountSpan) {
            currentCountSpan.textContent = currentMachineCount;
          }

          if (availableCountSpan) {
            availableCountSpan.textContent = availableSlots;

            // Available slots이 0이면 full 클래스 추가
            if (isFull) {
              availableCountSpan.classList.add('full');
            } else {
              availableCountSpan.classList.remove('full');
            }
          }

          if (capacityBadge) {
            // Capacity가 꽉 찼으면 full 클래스 추가
            if (isFull) {
              capacityBadge.classList.add('full');
            } else {
              capacityBadge.classList.remove('full');
            }
          }
        });
      }

      // Process 정보 저장
      async function saveProcessInfo(processBox, processIdx) {
        const inputs = processBox.querySelectorAll('input');
        const newName = inputs[0].value.trim();
        const newCapacity = parseInt(inputs[1].value);

        if (!newName) {
          alert('Process 이름을 입력해주세요.');
          return;
        }

        if (newCapacity < 1) {
          alert('Capacity는 최소 1 이상이어야 합니다.');
          return;
        }

        try {
          const formData = new FormData();
          formData.append('action', 'update_process');
          formData.append('idx', processIdx);
          formData.append('design_process', newName);
          formData.append('std_mc_needed', newCapacity);

          const response = await fetch('proc/process_machine_mapping.php', {
            method: 'POST',
            body: formData
          });

          const result = await response.json();

          if (result.success) {
            alert('Process 정보가 업데이트되었습니다.');
            processBox.classList.remove('editing');

            // UI 업데이트
            processBox.querySelector('.process-name').textContent = newName;
            processBox.querySelector('.process-capacity-badge').textContent = newCapacity;
            processBox.dataset.stdMcNeeded = newCapacity;

            // 드롭존의 capacity도 업데이트
            const dropZone = processBox.parentElement.querySelector('.drop-zone');
            if (dropZone) {
              dropZone.dataset.stdMcNeeded = newCapacity;
            }

            // mappingData 업데이트
            const process = mappingData.processes.find(p => p.idx === processIdx);
            if (process) {
              process.design_process = newName;
              process.std_mc_needed = newCapacity;
            }

            // Machine count 업데이트 (capacity가 변경되었으므로)
            updateAllMachineCounts();
          } else {
            alert('저장 실패: ' + result.message);
          }
        } catch (error) {
          console.error('Error saving process:', error);
          alert('저장 중 오류가 발생했습니다.');
        }
      }

      // Machine 할당 저장
      async function saveMachineAssignments() {
        try {
          // 할당 정보를 배열로 변환
          const assignments = Object.keys(currentAssignments).map(machineIdx => ({
            machine_idx: parseInt(machineIdx),
            design_process_idx: parseInt(currentAssignments[machineIdx])
          }));

          const formData = new FormData();
          formData.append('action', 'update_machine_assignments');
          formData.append('assignments', JSON.stringify(assignments));

          const response = await fetch('proc/process_machine_mapping.php', {
            method: 'POST',
            body: formData
          });

          const result = await response.json();

          if (result.success) {
            alert(`Machine 할당이 저장되었습니다. (${result.updated_count}개)`);

            // ResourceManager 새로고침하여 메인 테이블도 업데이트
            if (resourceManager && resourceManager.loadData) {
              await resourceManager.loadData();
            }
          } else {
            alert('저장 실패: ' + result.message);
          }
        } catch (error) {
          console.error('Error saving assignments:', error);
          alert('저장 중 오류가 발생했습니다.');
        }
      }

      // HTML 이스케이프 함수
      function escapeHtml(text) {
        const div = document.createElement('div');
        div.textContent = text;
        return div.innerHTML;
      }
    }

    // All common styles handled by fiori-page.css
  </script>

</body>
</html>