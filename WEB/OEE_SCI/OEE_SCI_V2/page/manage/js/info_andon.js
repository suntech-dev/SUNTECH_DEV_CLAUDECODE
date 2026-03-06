/**
 * INFO_ANDON.JS - Page-specific JavaScript for Andon Management
 * Main initialization and integrated management for Andon page
 */

/**
 * Spectrum Color Picker Configuration
 */
export const fioriColorPalette = [
  ["#0070f2", "#1e88e5", "#00d4aa", "#0093c7"],
  ["#30914c", "#da1e28", "#e26b0a", "#8e44ad"],
  ["#32363b", "#4a5568", "#6b7884", "#8b95a1"],
  ["#2563eb", "#059669", "#dc2626", "#ffa500"]
];

/**
 * Initialize Spectrum Color Picker
 */
export function initColorPicker() {
  const colorInput = $('#color');
  const colorPreviewBox = $('#colorPreviewBox');
  
  if (colorInput.length) {
    colorInput.spectrum({
      type: "component",
      showInput: true,
      showInitial: true,
      allowEmpty: true,
      showAlpha: false,
      disabled: false,
      showPalette: true,
      showPaletteOnly: false,
      togglePaletteOnly: false,
      showSelectionPalette: true,
      hideAfterPaletteSelect: true,
      clickoutFiresChange: true,
      color: "#0070f2",
      palette: fioriColorPalette,
      localStorageKey: "spectrum.andon.color",
      maxSelectionSize: 10,
      preferredFormat: "hex",
      
      move: function(color) {
        if (color) {
          const hexColor = color.toHexString();
          colorPreviewBox.css('background-color', hexColor);
        }
      },
      
      change: function(color) {
        if (color) {
          const hexColor = color.toHexString();
          colorPreviewBox.css('background-color', hexColor);
          
          const event = new Event('input', { bubbles: true });
          document.getElementById('color').dispatchEvent(event);
        } else {
          colorPreviewBox.css('background-color', '#ffffff');
        }
      }
    });
    
    const initialColor = colorInput.val() || '#0070f2';
    colorPreviewBox.css('background-color', initialColor);
  }
}

/**
 * Update color from external source
 */
window.updateColorFromSpectrum = function(hexColor) {
  const colorInput = $('#color');
  const colorPreviewBox = $('#colorPreviewBox');
  
  if (hexColor) {
    colorInput.spectrum('set', hexColor);
    colorPreviewBox.css('background-color', hexColor);
  }
};

/**
 * Initialize page-specific features
 */
export function initPageFeatures(resourceManager) {
  setTimeout(() => {
    initColorPicker();
  }, 100);
  
  setTimeout(() => {
    initAdvancedFeatures(resourceManager);
  }, 200);
}

/**
 * 고급 기능 초기화 함수
 * 모든 안돈 페이지 관련 기능들을 초기화
 * @param {Object} resourceManager - ResourceManager 인스턴스
 */
export function initAdvancedFeatures(resourceManager) {
  console.log('Andon 페이지 고급 기능 초기화 시작...');
  
  // 각 모듈 초기화
  initRealTimeSearch(resourceManager);
  initQuickActions();
  initModalSteps();
  initStatisticsUpdater();
  
  
  // 전역에서 테스트 가능하도록 함수 등록
  window.testAndonDuplicateCheck = async function() {
    const andonName = document.getElementById('andon_name').value.trim();
    console.log('🧪 테스트: 중복 검사 직접 호출, andon_name:', andonName);
    
    if (!andonName) {
      alert('테스트를 위해 andon name을 먼저 입력하세요.');
      return;
    }
    
    try {
      const response = await fetch(`proc/info_andon.php?for=check-duplicate&andon_name=${encodeURIComponent(andonName)}`);
      const result = await response.json();
      console.log('🧪 테스트 결과:', result);
      alert(`테스트 결과: ${result.success ? '사용 가능' : result.message}`);
    } catch (error) {
      console.error('🧪 테스트 오류:', error);
      alert('테스트 중 오류 발생: ' + error.message);
    }
  };
}


/**
 * 안돈 페이지 설정 객체
 */
export const andonConfig = {
  resourceName: 'Andon',
  apiEndpoint: 'proc/andon.php',
  entityId: 'idx',
  
  // 데이터 테이블의 컬럼 정의
  columnConfig: [
    {
      key: 'no',
      label: 'NO.',
      sortable: false,
      render: (item, index) => `<span style="color: var(--sap-text-secondary);">${index}</span>`,
      width: '60px'
    },
    {
      key: 'idx',
      label: 'IDX',
      sortable: true,
      sortKey: 'idx',
      visible: false
    },
    { 
      key: 'andon_name', 
      label: 'Andon Name', 
      sortable: true, 
      sortKey: 'andon_name',
      render: (item) => {        
        return `
          <div style="display: flex; align-items: center; gap: var(--sap-spacing-xs);">
            <div>
              <strong style="color: var(--sap-text-primary);">${item.andon_name}</strong>
            </div>
          </div>
        `;
      }
    },
    { 
      key: 'total_count', 
      label: 'Total', 
      sortable: true, 
      sortKey: 'total_count',
      render: (item) => {
        const count = item.total_count || 0;
        return `<span style="color: var(--sap-text-primary); font-weight: 500;">${count}</span>`;
      },
      width: '80px'
    },
    { 
      key: 'completed_count', 
      label: 'Completed', 
      sortable: true, 
      sortKey: 'completed_count',
      render: (item) => {
        const count = item.completed_count || 0;
        return `<span style="color: var(--sap-status-success); font-weight: 500;">${count}</span>`;
      },
      width: '100px'
    },
    { 
      key: 'warning_count', 
      label: 'Warning', 
      sortable: true, 
      sortKey: 'warning_count',
      render: (item) => {
        const count = item.warning_count || 0;
        return `<span style="color: var(--sap-status-error); font-weight: 500;">${count}</span>`;
      },
      width: '90px'
    },
    { 
      key: 'warning_rate', 
      label: 'Warning %', 
      sortable: true, 
      sortKey: 'warning_rate',
      render: (item) => {
        const rate = item.warning_rate || 0;
        const color = rate > 50 ? 'var(--sap-status-error)' : 
                     rate > 20 ? 'var(--sap-status-warning)' : 
                     'var(--sap-status-success)';
        return `<span style="color: ${color}; font-weight: 500;">${rate}%</span>`;
      },
      width: '100px'
    },
    { 
      key: 'status', 
      label: 'Status', 
      sortable: true, 
      sortKey: 'status', 
      render: (item) => {
        const isActive = item.status === 'Y';
        return `
          <span class="fiori-badge fiori-badge--${isActive ? 'success' : 'warning'}" 
                style="display: inline-flex; align-items: center; gap: var(--sap-spacing-xs);">
            ${isActive ? '✅ Used' : '⚠️ Unused'}
          </span>
        `;
      },
      width: '120px'
    },
    { 
      key: 'color', 
      label: 'Color', 
      sortable: true, 
      sortKey: 'color',
      render: (item) => {
        const color = item.color;
        if (!color || color === '' || color === null) {
          // 색상이 없을 때 기본 표시
          return `<div style="width: 50%; height: 20px; border: 1px solid var(--sap-border-neutral); border-radius: var(--sap-radius-sm); background: var(--sap-surface-2); display: flex; align-items: center; justify-content: center; font-size: var(--sap-font-size-xs); color: var(--sap-text-secondary);">N/A</div>`;
        }
        
        // 실제 색상이 있을 때 - 색상 박스만 표시
        return `<div style="width: 50%; height: 20px; border: 1px solid var(--sap-border-neutral); border-radius: var(--sap-radius-sm); background: ${color}; box-shadow: 0 1px 2px rgba(0,0,0,0.1);" title="${color}"></div>`;
      },
      width: '80px'
    },
    { 
      key: 'remark', 
      label: 'Remark', 
      sortable: true, 
      sortKey: 'remark',
      render: (item) => {
        const remark = item.remark || '-';
        return `<span style="color: ${remark === '-' ? 'var(--sap-text-secondary)' : 'var(--sap-text-primary)'};">${remark}</span>`;
      }
    }
  ],
  
  // 필터 UI 설정
  filterConfig: [
    {
      elementId: 'statusFilterSelect',
      paramName: 'status_filter',
      stateKey: 'statusFilter'
    }
  ]
};


/**
 * Andon Search Module
 * 안돈 페이지 실시간 검색 기능 관리
 */

/**
 * 실시간 검색 기능 초기화
 * @param {Object} resourceManager - ResourceManager 인스턴스
 */
export function initRealTimeSearch(resourceManager) {
  const searchInput = document.getElementById('realTimeSearch');
  const searchClear = document.getElementById('searchClear');
  let searchTimeout;
  
  if (!searchInput || !searchClear) {
    console.error("I can't find the search input field or clear button.");
    return;
  }
  
  searchInput.addEventListener('input', (e) => {
    const value = e.target.value.trim();
    
    // Clear 버튼 표시/숨김
    if (value) {
      searchClear.classList.add('visible');
    } else {
      searchClear.classList.remove('visible');
    }
    
    // 디바운스 검색
    clearTimeout(searchTimeout);
    searchTimeout = setTimeout(() => {
      performSearch(value, resourceManager);
    }, 300);
  });
  
  searchClear.addEventListener('click', () => {
    searchInput.value = '';
    searchClear.classList.remove('visible');
    // 검증 메시지 지우기
    if (window.FioriInteractions && typeof window.FioriInteractions.clearFieldMessage === 'function') {
      window.FioriInteractions.clearFieldMessage(searchInput);
    }
    performSearch('', resourceManager);
    searchInput.focus();
  });
    
    // Enter 키 즉시 검색
    searchInput.addEventListener('keydown', (e) => {
      if (e.key === 'Enter') {
        clearTimeout(searchTimeout);
        performSearch(searchInput.value.trim(), resourceManager);
      }
    });
    
    // Refresh 버튼 클릭 시 검증 메시지 지우기
    const refreshBtn = document.getElementById('refreshBtn');
    if (refreshBtn) {
      refreshBtn.addEventListener('click', () => {
        // 모든 검증 메시지 지우기
        if (window.FioriInteractions && typeof window.FioriInteractions.clearAllFieldMessages === 'function') {
          window.FioriInteractions.clearAllFieldMessages();
        }
      });
    }
}

/**
 * 검색 실행 함수
 * @param {string} query - 검색어
 * @param {Object} resourceManager - ResourceManager 인스턴스
 */
export function performSearch(query, resourceManager) {
  // ResourceManager의 상태에 검색어 저장
  if (resourceManager && resourceManager.state) {
    resourceManager.state.searchQuery = query;
    // 검색 시 첫 페이지로 이동
    resourceManager.state.currentPage = 1;
  }

  // 서버에서 검색 결과를 가져오기 위해 데이터 다시 로드
  if (resourceManager && resourceManager.loadData) {
    resourceManager.loadData();
  }
}


/**
 * Andon Filters Module
 * 안돈 페이지 필터 및 빠른 액션 기능 관리
 */

/**
 * 빠른 액션 필터 기능 초기화
 */
export function initQuickActions() {
  const actionBtns = document.querySelectorAll('.quick-action-btn');
    
  actionBtns.forEach(btn => {
    btn.addEventListener('click', () => {
      // 활성 상태 변경
      actionBtns.forEach(b => b.classList.remove('active'));
      btn.classList.add('active');
      
      // 필터 적용
      const filter = btn.dataset.filter;
      applyQuickFilter(filter);
    });
  });
}

/**
 * 빠른 필터 적용 함수
 * @param {string} filter - 필터 타입 ('all', 'used', 'unused')
 */
export function applyQuickFilter(filter) {
  const statusSelect = document.getElementById('statusFilterSelect');
  
  if (!statusSelect) {
    console.error('상태 필터 선택 박스를 찾을 수 없습니다.');
    return;
  }
  
  switch(filter) {
    case 'used':
      statusSelect.value = 'Y';
      break;
    case 'unused':
      statusSelect.value = 'N';
      break;
    case 'all':
    default:
      statusSelect.value = '';
        break;
  }
  
  // 필터 적용 트리거
  statusSelect.dispatchEvent(new Event('change'));
}


/**
 * Andon Modal Module
 * 안돈 페이지 모달 단계 기능 관리
 */

/**
 * 모달 단계 기능 초기화
 */
export function initModalSteps() {
  console.log('🚀 initModalSteps 호출됨');
  const nextBtn = document.getElementById('nextStep');
  const prevBtn = document.getElementById('prevStep');
  const submitBtn = document.getElementById('submitBtn');
  const stepIndicator = document.getElementById('stepIndicator');
  
  console.log('🔍 버튼 요소들 확인:', { nextBtn, prevBtn, submitBtn, stepIndicator });
  
  if (!nextBtn || !prevBtn || !submitBtn || !stepIndicator) {
    console.error('❌ 모달 버튼 또는 단계 표시기를 찾을 수 없습니다.');
    return;
  }
  
  let currentStep = 1;
  const totalSteps = 2;
  
  nextBtn.addEventListener('click', async () => {
    console.log('🔄 Next 버튼 클릭됨, currentStep:', currentStep);
    if (await validateCurrentStep(currentStep)) {
      console.log('✅ 검증 통과, 다음 단계로 이동');
      currentStep++;
      showStep(currentStep);
    } else {
      console.log('❌ 검증 실패, 단계 이동 차단');
    }
  });
  
  prevBtn.addEventListener('click', () => {
    currentStep--;
    showStep(currentStep);
  });
  
  // 폼 스텝 아이콘 클릭 이벤트 추가
  const formSteps = document.querySelectorAll('.form-step');
  formSteps.forEach((step, index) => {
    step.addEventListener('click', async () => {
      const targetStep = index + 1;
      
      // 현재 스텝에서 클릭한 스텝으로 이동
      if (targetStep < currentStep) {
        // 뒤로 이동하는 경우 검증 없이 바로 이동
        currentStep = targetStep;
        showStep(currentStep);
      } else if (targetStep > currentStep) {
        // 앞으로 이동하는 경우 중간 단계들을 검증하면서 이동
        let isValid = true;
        let tempStep = currentStep;
        
        while (tempStep < targetStep && isValid) {
          isValid = await validateCurrentStep(tempStep);
          if (isValid) {
            tempStep++;
          }
        }
        
        if (isValid) {
          currentStep = targetStep;
          showStep(currentStep);
        }
      }
      // 같은 스텝 클릭 시 아무 작업 없음
    });
    
    // 클릭 가능한 스타일 추가
    step.style.cursor = 'pointer';
    step.title = `Step ${index + 1}로 이동`;
  });
  
  /**
   * 단계 표시/숨김 함수
   * @param {number} step - 표시할 단계 번호
   */
  function showStep(step) {
    // 섹션 표시/숨김
    document.querySelectorAll('.form-section').forEach(section => {
      section.style.display = section.dataset.section == step ? 'block' : 'none';
    });
    
    // 버튼 상태 변경
    prevBtn.style.display = step > 1 ? 'inline-block' : 'none';
    nextBtn.style.display = step < totalSteps ? 'inline-block' : 'none';
    submitBtn.style.display = step === totalSteps ? 'inline-block' : 'none';
    
    // 단계 표시기 업데이트
    stepIndicator.textContent = `Step ${step} of ${totalSteps}`;
    updateStepIndicators(step);
    updatePreview();
  }
  
  /**
   * 단계 인디케이터 업데이트
   * @param {number} activeStep - 현재 활성 단계
   */
  function updateStepIndicators(activeStep) {
    document.querySelectorAll('.form-step').forEach(step => {
      const stepNum = parseInt(step.dataset.step);
      step.classList.toggle('active', stepNum <= activeStep);
    });
  }
  
  /**
   * 현재 단계 검증
   * @param {number} step - 검증할 단계 번호
   * @returns {Promise<boolean>} - 검증 성공 여부
   */
  async function validateCurrentStep(step) {
    console.log('🔍 validateCurrentStep 호출됨, step:', step);
    if (step === 1) {
      const andonName = document.getElementById('andon_name').value.trim();
      console.log('📝 입력된 andon name:', andonName);
      
      if (!andonName) {
        alert('Please enter the andon name.');
        return false;
      }
      
      // 중복 andon 이름 검사
      try {
        const currentIdx = document.getElementById('resourceId').value || null;
        const apiUrl = `proc/andon.php?for=check-duplicate&andon_name=${encodeURIComponent(andonName)}${currentIdx ? `&current_idx=${currentIdx}` : ''}`;
        console.log('🌐 API 호출 URL:', apiUrl);
        
        const response = await fetch(apiUrl);
        const result = await response.json();
        console.log('📨 서버 응답:', result);
        
        if (!result.success) {
          alert(result.message);
          return false;
        }
        
        console.log('✅ 중복 검사 통과');
      } catch (error) {
        console.error('❌ 중복 확인 중 오류:', error);
        alert('An error occurred while checking for duplicates. Please try again later.');
        return false;
      }
    }
    return true;
  }
  
  /**
   * 미리보기 업데이트
   */
  function updatePreview() {
    const andonName = document.getElementById('andon_name').value || '-';
    const color = document.getElementById('color').value || '-';
    const statusText = document.getElementById('status').selectedOptions[0]?.textContent || '-';
    
    const previewName = document.getElementById('previewName');
    const previewColor = document.getElementById('previewColor');
    const previewStatus = document.getElementById('previewStatus');
    
    if (previewName) previewName.textContent = andonName;
    if (previewColor) previewColor.textContent = color;
    if (previewStatus) previewStatus.textContent = statusText;
  }
  
  // 입력값 변경 시 미리보기 업데이트
  const andonNameInput = document.getElementById('andon_name');
  const colorInput = document.getElementById('color');
  const statusSelect = document.getElementById('status');
  
  if (andonNameInput) andonNameInput.addEventListener('input', updatePreview);
  if (colorInput) colorInput.addEventListener('input', updatePreview);
  if (statusSelect) statusSelect.addEventListener('change', updatePreview);
  
  // 모달 열릴 때 초기화
  const addBtn = document.getElementById('addBtn');
  if (addBtn) {
    addBtn.addEventListener('click', () => {
      currentStep = 1;
      showStep(1);
      
      // spectrum 컬러 피커 초기화
      const colorInput = $('#color');
      if (colorInput.length && typeof colorInput.spectrum === 'function') {
        colorInput.spectrum('set', '#0070f2');
        $('#colorPreviewBox').css('background-color', '#0070f2');
      }
    });
  }
  
  // 전역에서 폼 데이터 로드 시 spectrum 업데이트 함수
  window.updateAndonFormSpectrum = function(data) {
    if (data && data.color) {
      const colorInput = $('#color');
      if (colorInput.length && typeof colorInput.spectrum === 'function') {
        colorInput.spectrum('set', data.color);
        $('#colorPreviewBox').css('background-color', data.color);
        console.log('📝 Spectrum 컬러 업데이트:', data.color);
      }
    }
  };
}


/**
 * Andon Statistics Module
 * 안돈 페이지 통계 기능 관리
 */

/**
 * 통계 업데이터 초기화
 */
export function initStatisticsUpdater() {
  // 초기 통계 로드
  updateStatistics();
  
  // 데이터 변경 시 통계 업데이트  
  const tableBody = document.getElementById('tableBody');
  if (tableBody) {
    const observer = new MutationObserver(() => {
      updateStatistics();
    });
    
    observer.observe(tableBody, {
      childList: true,
      subtree: true
    });
  }
  
  // 필터 변경 시 통계 업데이트
  const statusFilterSelect = document.getElementById('statusFilterSelect');
  if (statusFilterSelect) {
    statusFilterSelect.addEventListener('change', () => {
      // 필터 적용 후 통계 업데이트
      setTimeout(() => {
        updateStatistics();
      }, 100);
    });
  }
  
  // 새로고침 버튼 클릭 시 통계 업데이트
  const refreshBtn = document.getElementById('refreshBtn');
  if (refreshBtn) {
    refreshBtn.addEventListener('click', () => {
      setTimeout(() => {
        updateStatistics();
      }, 500);
    });
  }
}

/**
 * 통계 업데이트 함수 - 새로운 andon 통계 로직
 */
export function updateStatistics() {
  updateAndonStatistics();
}

/**
 * Andon 통계 업데이트 함수 - API 기반 접근 방식
 * 필터 값을 서버에 전달하여 필터링된 통계 받아오기
 */
async function updateAndonStatistics() {
  const statsGrid = document.getElementById('statsGrid');
  if (!statsGrid) return;
  
  try {
    // 현재 필터 값 가져오기
    const statusFilter = document.getElementById('statusFilterSelect')?.value || '';
    
    // 1. info_andon 테이블에서 andon 종류 목록 가져오기 (필터 적용)
    let andonApiUrl = 'proc/andon.php';
    if (statusFilter) {
      andonApiUrl += `?status_filter=${encodeURIComponent(statusFilter)}`;
    }
    
    const andonListResponse = await fetch(andonApiUrl);
    const andonListResult = await andonListResponse.json();
    
    let andonTypes = [];
    if (andonListResult.success && andonListResult.data) {
      andonTypes = andonListResult.data.map(item => ({
        andon_name: item.andon_name,
        color: item.color || '#6b7884'
      }));
    }
    
    // 2. data_andon 테이블에서 Warning 상태 수량 가져오기 (필터 적용)
    let warningApiUrl = 'proc/andon.php?for=warning-stats';
    if (statusFilter) {
      warningApiUrl += `&status_filter=${encodeURIComponent(statusFilter)}`;
    }
    
    const warningStatsResponse = await fetch(warningApiUrl);
    const warningStatsResult = await warningStatsResponse.json();
    
    let warningCounts = {};
    let totalWarningCount = 0;
    
    if (warningStatsResult.success && warningStatsResult.data) {
      warningCounts = warningStatsResult.data;
      // 전체 Warning 수량 계산
      totalWarningCount = Object.values(warningCounts).reduce((sum, count) => sum + count, 0);
    }
    
    // 3. 통계 카드 업데이트
    updateStatCards(totalWarningCount, andonTypes, warningCounts);
    
  } catch (error) {
    console.error('통계 업데이트 중 오류:', error);
    // 오류 시 기본 값으로 표시
    updateStatCards(0, [], {});
  }
}

/**
 * 통계 카드 UI 업데이트
 */
function updateStatCards(totalWarningCount, andonTypes, warningCounts) {
  const statsGrid = document.getElementById('statsGrid');
  if (!statsGrid) {
    console.error('statsGrid를 찾을 수 없습니다.');
    return;
  }
  
  // 기존 동적 카드들 제거 (첫 번째 총 Warning 카드 제외)
  const dynamicCards = statsGrid.querySelectorAll('.stat-card:not(:first-child)');
  dynamicCards.forEach(card => card.remove());
  
  // 1. 전체 Warning 수량 업데이트
  animateNumber('totalWarningCount', totalWarningCount);
  
  // 2. 각 andon_name별 Warning 카드 생성
  andonTypes.forEach(andon => {
    const warningCount = warningCounts[andon.andon_name] || 0;
    const cardElement = createWarningStatCard(andon.andon_name, warningCount, andon.color);
    statsGrid.appendChild(cardElement);
  });
}

/**
 * Warning 통계 카드 생성
 */
function createWarningStatCard(andonName, count, color) {
  const card = document.createElement('div');
  card.className = 'stat-card stat-card--warning';
  card.innerHTML = `
    <div class="stat-value" id="warning-${andonName.toLowerCase()}">${count}</div>
    <div class="stat-label">
      <div style="display: flex; align-items: center; gap: var(--sap-spacing-xs);">
        <div style="width: 12px; height: 12px; border-radius: 50%; background: ${color}; border: 1px solid var(--sap-border-neutral);"></div>
        <span>⚠️ ${andonName}</span>
      </div>
    </div>
  `;
  return card;
}

/**
 * 숫자 애니메이션 함수
 * @param {string} elementId - 애니메이션을 적용할 엘리먼트 ID
 * @param {number} target - 목표 숫자
 * @param {string} suffix - 숫자 뒤에 붙을 접미사 (기본값: '')
 */
export function animateNumber(elementId, target, suffix = '') {
  const element = document.getElementById(elementId);
  if (!element) {
    console.error(`엘리먼트를 찾을 수 없습니다: ${elementId}`);
    return;
  }
  
  const current = parseInt(element.textContent) || 0;
  const difference = target - current;
  const duration = 1000;
  const steps = 20;
  const stepValue = difference / steps;
  const stepTime = duration / steps;
  
  let step = 0;
  const timer = setInterval(() => {
    step++;
    const value = Math.round(current + (stepValue * step));
    element.textContent = value + suffix;
    
    if (step >= steps) {
      clearInterval(timer);
      element.textContent = target + suffix;
    }
  }, stepTime);
}