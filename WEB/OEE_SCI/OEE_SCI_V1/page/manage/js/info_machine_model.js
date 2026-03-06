/**
 * Machine Model Module
 * Main initialization and management for machine model pages
 */

// 모듈 imports

/**
 * Advanced features initialization function
 * Initializes all machine model page related features
 * @param {Object} resourceManager - ResourceManager instance
 */
export function initAdvancedFeatures(resourceManager) {
  
  // Initialize modules
  initRealTimeSearch(resourceManager);
  initQuickActions();
  initModalSteps();
  initStatisticsUpdater();
  
}


/**
 * Machine model page configuration object
 */
export const machineModelConfig = {
  resourceName: 'Machine_Model',
  apiEndpoint: 'proc/machine_model.php',
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
      label: 'ID', 
      sortable: true, 
      sortKey: 'idx', 
      visible: false 
    },
    { 
      key: 'machine_model_name', 
      label: 'Machine Model Name', 
      sortable: true, 
      sortKey: 'machine_model_name',
      render: (item) => {
        return `
          <div style="display: flex; align-items: center; gap: var(--sap-spacing-xs);">
            <div>
              <strong style="color: var(--sap-text-primary);">${item.machine_model_name}</strong>
            </div>
          </div>
        `;
      }
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
      key: 'type', 
      label: 'Type', 
      sortable: true, 
      sortKey: 'type',
      visible: true,
      render: (item) => {
        if (item.type === 'P') {
          return '<span style="color: var(--sap-brand-primary); font-weight: 600;">Computer Sewing Machine</span>';
        } else if (item.type === 'E') {
          return '<span style="color: var(--sap-status-warning); font-weight: 600;">Embroidery Machine</span>';
        }
        return '<span style="color: var(--sap-text-secondary);">Unknown</span>';
      },
      width: '200px'
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
 * Machine Model Search Module
 * 팩토리 페이지 실시간 검색 기능 관리
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
    // Clear validation messages
    if (window.FioriInteractions && typeof window.FioriInteractions.clearFieldMessage === 'function') {
      window.FioriInteractions.clearFieldMessage(searchInput);
    }
    performSearch('', resourceManager);
    searchInput.focus();
  });
  
  // Immediate search on Enter key
  searchInput.addEventListener('keydown', (e) => {
    if (e.key === 'Enter') {
      clearTimeout(searchTimeout);
      performSearch(searchInput.value.trim(), resourceManager);
    }
  });
  
  // Clear validation messages on refresh
  const refreshBtn = document.getElementById('refreshBtn');
  if (refreshBtn) {
    refreshBtn.addEventListener('click', () => {
      // Clear all validation messages
      if (window.FioriInteractions && typeof window.FioriInteractions.clearAllFieldMessages === 'function') {
        window.FioriInteractions.clearAllFieldMessages();
      }
    });
  }
}

/**
 * Search execution function
 * @param {string} query - Search query
 * @param {Object} resourceManager - ResourceManager instance
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
 * Machine Model Filters Module
 * 팩토리 페이지 필터 및 빠른 액션 기능 관리
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
  
  // Trigger filter application
  statusSelect.dispatchEvent(new Event('change'));
}


/**
 * Machine Model Modal Module
 * Machine model page modal step feature management
 */

/**
 * Modal step feature initialization
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
  
  // Add form step icon click event
  const formSteps = document.querySelectorAll('.form-step');
  formSteps.forEach((step, index) => {
    step.addEventListener('click', async () => {
      const targetStep = index + 1;
      
      // Move from current step to clicked step
      if (targetStep < currentStep) {
        // Move backward without validation
        currentStep = targetStep;
        showStep(currentStep);
      } else if (targetStep > currentStep) {
        // Validate intermediate steps when moving forward
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
      // No action on same step click
    });
    
    // Add clickable style
    step.style.cursor = 'pointer';
    step.title = `Move to Step ${index + 1}`;
  });
  
  /**
   * 단계 표시/숨김 함수
   * @param {number} step - 표시할 단계 번호
   */
  function showStep(step) {
    // Show/hide sections
    document.querySelectorAll('.form-section').forEach(section => {
      section.style.display = section.dataset.section == step ? 'block' : 'none';
    });
    
    // Change button states
    prevBtn.style.display = step > 1 ? 'inline-block' : 'none';
    nextBtn.style.display = step < totalSteps ? 'inline-block' : 'none';
    submitBtn.style.display = step === totalSteps ? 'inline-block' : 'none';
    
    // Update step indicator
    stepIndicator.textContent = `Step ${step} of ${totalSteps}`;
    updateStepIndicators(step);
    updatePreview();
  }
  
  /**
   * Update step indicators
   * @param {number} activeStep - Currently active step
   */
  function updateStepIndicators(activeStep) {
    document.querySelectorAll('.form-step').forEach(step => {
      const stepNum = parseInt(step.dataset.step);
      step.classList.toggle('active', stepNum <= activeStep);
    });
  }
  
  /**
   * Validate current step
   * @param {number} step - Step number to validate
   * @returns {Promise<boolean>} - Validation success status
   */
  async function validateCurrentStep(step) {
    
    if (step === 1) {
      const machineModelName = document.getElementById('machine_model_name').value.trim();
      
      if (!machineModelName) {
        alert('Please enter the Machine Model name.');
        return false;
      }
      
      // Check duplicate machine model name
      try {
        const currentIdx = document.getElementById('resourceId').value || null;
        
        const apiUrl = `proc/machine_model.php?for=check-duplicate&machine_model_name=${encodeURIComponent(machineModelName)}${currentIdx ? `&current_idx=${currentIdx}` : ''}`;
        
        const response = await fetch(apiUrl);
        const result = await response.json();
        
        if (!result.success) {
          alert(result.message);
          return false;
        }
        
      } catch (error) {
        console.error('Error checking duplicate:', error);
        alert('An error occurred while checking for duplicates. Please try again later.');
        return false;
      }
    }
    return true;
  }
  
  /**
   * Update preview
   */
  function updatePreview() {
    const machineModelName = document.getElementById('machine_model_name').value || '-';
    const statusText = document.getElementById('status').selectedOptions[0]?.textContent || '-';
    
    const previewName = document.getElementById('previewName');
    const previewStatus = document.getElementById('previewStatus');
    
    if (previewName) previewName.textContent = machineModelName;
    if (previewStatus) previewStatus.textContent = statusText;
  }
  
  // Update preview when input values change
  const machineModelNameInput = document.getElementById('machine_model_name');
  const statusSelect = document.getElementById('status');
  
  if (machineModelNameInput) machineModelNameInput.addEventListener('input', updatePreview);
  if (statusSelect) statusSelect.addEventListener('change', updatePreview);
  
  // Initialize when modal opens
  const addBtn = document.getElementById('addBtn');
  if (addBtn) {
    addBtn.addEventListener('click', () => {
      currentStep = 1;
      showStep(1);
    });
  }
}


/**
 * Machine Model Statistics Module
 * Machine model page statistics feature management
 */

/**
 * Initialize statistics updater
 */
export function initStatisticsUpdater() {
  // Load initial statistics
  updateStatistics();
  
  // Update statistics when data changes
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
  
  // Update statistics when filters change
  const statusFilterSelect = document.getElementById('statusFilterSelect');
  if (statusFilterSelect) {
    statusFilterSelect.addEventListener('change', () => {
      // Delay to ensure the filter has been applied to the table data
      setTimeout(() => {
        updateStatistics();
      }, 100);
    });
  }
  
  // Update statistics when refresh button is clicked
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
 * Statistics update function
 */
export function updateStatistics() {
  // Update statistics using API-based approach
  updateMachineModelCards();
}

/**
 * Update machine model cards based on statistics API
 */
export function updateMachineModelCards() {
  const statsGrid = document.getElementById('statsGrid');
  if (!statsGrid) return;
  
  // Get current filter value
  const statusFilter = document.getElementById('statusFilterSelect')?.value || '';
  
  // Build API URL with filter
  let apiUrl = 'proc/machine_model.php?for=statistics';
  if (statusFilter) {
    apiUrl += `&status_filter=${encodeURIComponent(statusFilter)}`;
  }
  
  // Fetch statistics from API
  fetch(apiUrl)
    .then(response => response.json())
    .then(result => {
      if (result.success) {
        const { total_models, model_stats } = result.data;
        
        // Update total models count
        animateNumber('totalModelsCount', total_models);
        
        // Remove existing dynamic cards (keep only the first total card)
        const existingCards = statsGrid.querySelectorAll('.stat-card');
        for (let i = 1; i < existingCards.length; i++) {
          existingCards[i].remove();
        }
        
        // Add cards for each machine model with actual machine counts
        model_stats.forEach(stat => {
          const card = document.createElement('div');
          card.className = 'stat-card stat-card--info';
          
          // Determine icon based on machine type
          const typeIcon = stat.type === 'P' ? '' : '';
          
          card.innerHTML = `
            <div class="stat-value">${stat.machine_count}</div>
            <div class="stat-label">${typeIcon} ${stat.machine_model_name}</div>
          `;
          statsGrid.appendChild(card);
        });
      } else {
        console.error('Failed to fetch statistics:', result.message);
      }
    })
    .catch(error => {
      console.error('Error fetching statistics:', error);
    });
}

/**
 * Number animation function
 * @param {string} elementId - Element ID to apply animation to
 * @param {number} target - Target number
 * @param {string} suffix - Suffix to append after number (default: '')
 */
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