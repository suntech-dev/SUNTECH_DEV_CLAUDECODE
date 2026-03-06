
export function initAdvancedFeatures(resourceManager) {
  
  initRealTimeSearch(resourceManager);
  initQuickActions();
  initModalSteps();
  initStatisticsUpdater();
  
  
}

/**
 * 테마 토글 기능 (Light Mode 전용)
 * 다크 모드가 제거되어 Light Mode만 사용
 */
function initThemeToggle() {
  const themeToggle = document.getElementById('themeToggle');
  if (!themeToggle) {
    // 테마 토글 버튼이 없는 경우는 정상 (제거됨)
    return;
  }
  
  const iconEl = themeToggle.querySelector('.icon');
  const textEl = themeToggle.querySelector('.text');
  
  if (!iconEl || !textEl) {
    return;
  }
  
}

/**
 * 팩토리 페이지 설정 객체
 */
export const factoryConfig = {
  resourceName: 'Factory',
  apiEndpoint: 'proc/factory.php',
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
      key: 'factory_name', 
      label: 'Factory Name', 
      sortable: true, 
      sortKey: 'factory_name',
      render: (item) => {
        return `
          <div style="display: flex; align-items: center; gap: var(--sap-spacing-xs);">
            <div>
              <strong style="color: var(--sap-text-primary);">${item.factory_name}</strong>
            </div>
          </div>
        `;
      }
    },
    { 
      key: 'line_count', 
      label: 'Lines', 
      sortable: true, 
      sortKey: 'line_count',
      render: (item) => {
        const count = item.line_count || 0;
        return `<span style="color: var(--sap-text-primary); font-weight: 500;">${count}</span>`;
      },
      width: '80px'
    },
    { 
      key: 'machine_count', 
      label: 'Machines', 
      sortable: true, 
      sortKey: 'machine_count',
      render: (item) => {
        const count = item.machine_count || 0;
        return `<span style="color: var(--sap-text-primary); font-weight: 500;">${count}</span>`;
      },
      width: '90px'
    },
    { 
      key: 'total_mp', 
      label: 'MP', 
      sortable: true, 
      sortKey: 'total_mp',
      render: (item) => {
        const mp = item.total_mp || 0;
        return `<span style="color: var(--sap-text-primary); font-weight: 500;">${mp}</span>`;
      },
      width: '70px'
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
 * Factory Search Module
 * 팩토리 페이지 실시간 검색 기능 관리
 */

export function initRealTimeSearch(resourceManager) {
  const searchInput = document.getElementById('realTimeSearch');
  const searchClear = document.getElementById('searchClear');
  let searchTimeout;
  
  if (!searchInput || !searchClear) {
    return;
  }
  
  searchInput.addEventListener('input', (e) => {
    const value = e.target.value.trim();
    
    if (value) {
      searchClear.classList.add('visible');
    } else {
      searchClear.classList.remove('visible');
    }
    
    clearTimeout(searchTimeout);
    searchTimeout = setTimeout(() => {
      performSearch(value, resourceManager);
    }, 300);
  });
  
  searchClear.addEventListener('click', () => {
    searchInput.value = '';
    searchClear.classList.remove('visible');
    if (window.FioriInteractions && typeof window.FioriInteractions.clearFieldMessage === 'function') {
      window.FioriInteractions.clearFieldMessage(searchInput);
    }
    performSearch('', resourceManager);
    searchInput.focus();
  });
  
  searchInput.addEventListener('keydown', (e) => {
    if (e.key === 'Enter') {
      clearTimeout(searchTimeout);
      performSearch(searchInput.value.trim(), resourceManager);
    }
  });
  
  const refreshBtn = document.getElementById('refreshBtn');
  if (refreshBtn) {
    refreshBtn.addEventListener('click', () => {
      if (window.FioriInteractions && typeof window.FioriInteractions.clearAllFieldMessages === 'function') {
        window.FioriInteractions.clearAllFieldMessages();
      }
    });
  }
}

/**
 * 검색 실행 함수 (서버 측 검색)
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
 * Factory Filters Module
 * 팩토리 페이지 필터 및 빠른 액션 기능 관리
 */

export function initQuickActions() {
  const actionBtns = document.querySelectorAll('.quick-action-btn');
  
  actionBtns.forEach(btn => {
    btn.addEventListener('click', () => {
      actionBtns.forEach(b => b.classList.remove('active'));
      btn.classList.add('active');
      
      const filter = btn.dataset.filter;
      applyQuickFilter(filter);
    });
  });
}

export function applyQuickFilter(filter) {
  const statusSelect = document.getElementById('statusFilterSelect');
  
  if (!statusSelect) {
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
  
  statusSelect.dispatchEvent(new Event('change'));
}


/**
 * Factory Modal Module
 * 팩토리 페이지 모달 단계 기능 관리
 */

export function initModalSteps() {
  const nextBtn = document.getElementById('nextStep');
  const prevBtn = document.getElementById('prevStep');
  const submitBtn = document.getElementById('submitBtn');
  const stepIndicator = document.getElementById('stepIndicator');
  
  
  if (!nextBtn || !prevBtn || !submitBtn || !stepIndicator) {
    return;
  }
  
  let currentStep = 1;
  const totalSteps = 2;
  
  nextBtn.addEventListener('click', async () => {
    if (await validateCurrentStep(currentStep)) {
      currentStep++;
      showStep(currentStep);
    } else {
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
        currentStep = targetStep;
        showStep(currentStep);
      } else if (targetStep > currentStep) {
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
    });
    
    step.style.cursor = 'pointer';
    step.title = `Step ${index + 1}로 이동`;
  });
  
  function showStep(step) {
    document.querySelectorAll('.form-section').forEach(section => {
      section.style.display = section.dataset.section == step ? 'block' : 'none';
    });
    
    prevBtn.style.display = step > 1 ? 'inline-block' : 'none';
    nextBtn.style.display = step < totalSteps ? 'inline-block' : 'none';
    submitBtn.style.display = step === totalSteps ? 'inline-block' : 'none';
    
    stepIndicator.textContent = `Step ${step} of ${totalSteps}`;
    updateStepIndicators(step);
    updatePreview();
  }
  
  function updateStepIndicators(activeStep) {
    document.querySelectorAll('.form-step').forEach(step => {
      const stepNum = parseInt(step.dataset.step);
      step.classList.toggle('active', stepNum <= activeStep);
    });
  }
  
  async function validateCurrentStep(step) {
    if (step === 1) {
      const factoryName = document.getElementById('factory_name').value.trim();
      
      if (!factoryName) {
        alert('Please enter the factory name.');
        return false;
      }
      
      try {
        const currentIdx = document.getElementById('resourceId').value || null;
        const apiUrl = `proc/factory.php?for=check-duplicate&factory_name=${encodeURIComponent(factoryName)}${currentIdx ? `&current_idx=${currentIdx}` : ''}`;
        
        const response = await fetch(apiUrl);
        const result = await response.json();
        
        if (!result.success) {
          alert(result.message);
          return false;
        }
        
      } catch (error) {
        alert('An error occurred while checking for duplicates. Please try again later.');
        return false;
      }
    }
    return true;
  }
  
  function updatePreview() {
    const factoryName = document.getElementById('factory_name').value || '-';
    const statusText = document.getElementById('status').selectedOptions[0]?.textContent || '-';
    
    const previewName = document.getElementById('previewName');
    const previewStatus = document.getElementById('previewStatus');
    
    if (previewName) previewName.textContent = factoryName;
    if (previewStatus) previewStatus.textContent = statusText;
  }
  
  const factoryNameInput = document.getElementById('factory_name');
  const statusSelect = document.getElementById('status');
  
  if (factoryNameInput) factoryNameInput.addEventListener('input', updatePreview);
  if (statusSelect) statusSelect.addEventListener('change', updatePreview);
  
  const addBtn = document.getElementById('addBtn');
  if (addBtn) {
    addBtn.addEventListener('click', () => {
      currentStep = 1;
      showStep(1);
    });
  }
}


/**
 * Factory Statistics Module
 * 팩토리 페이지 통계 기능 관리
 */

export function initStatisticsUpdater() {
  updateStatistics();
  
  // 필터 변경 시에도 통계 업데이트
  const statusFilter = document.getElementById('statusFilterSelect');
  if (statusFilter) {
    statusFilter.addEventListener('change', updateStatistics);
  }
  
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
}

export function updateStatistics() {
  // 모든 통계를 API를 통해 일관되게 가져오기
  fetchAllStatistics();
}

export async function fetchAllStatistics() {
  try {
    // 현재 필터 상태 가져오기
    const statusFilter = document.getElementById('statusFilterSelect')?.value || '';
    
    // API 를 통해 모든 통계 데이터 가져오기
    const response = await fetch(`proc/factory.php?for=statistics&status_filter=${statusFilter}`);
    const result = await response.json();
    
    if (result.success && result.data) {
      const stats = result.data;
      animateNumber('totalCount', stats.total_factories || 0);
      animateNumber('totalLines', stats.total_lines || 0);
      animateNumber('totalMachines', stats.total_machines || 0);
      animateNumber('totalMP', stats.total_mp || 0);
    } else {
      // API 에러 시 fallback
      animateNumber('totalCount', 0);
      animateNumber('totalLines', 0);
      animateNumber('totalMachines', 0);
      animateNumber('totalMP', 0);
    }
  } catch (error) {
    console.error('Failed to fetch statistics:', error);
    // 에러 시 fallback
    animateNumber('totalCount', 0);
    animateNumber('totalLines', 0);
    animateNumber('totalMachines', 0);
    animateNumber('totalMP', 0);
  }
}

export function animateNumber(elementId, target, suffix = '') {
  const element = document.getElementById(elementId);
  if (!element) {
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