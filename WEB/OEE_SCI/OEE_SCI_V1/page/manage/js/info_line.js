/**
 * Line Main Module
 * 라인 페이지 메인 초기화 및 통합 관리
 */

/**
 * 고급 기능 초기화 함수
 * 모든 라인 페이지 관련 기능들을 초기화
 * @param {Object} resourceManager - ResourceManager 인스턴스
 */
export function initAdvancedFeatures(resourceManager) {
  console.log('Line 페이지 고급 기능 초기화 시작...');
  
  // updateStatistics 함수를 전역으로 등록 (resource-manager에서 호출할 수 있도록)
  window.updateStatistics = updateStatistics;
  
  // 각 모듈 초기화
  initRealTimeSearch(resourceManager);
  initQuickActions();
  initModalSteps();
  initStatisticsUpdater();
  
  console.log('Line 페이지 고급 기능 초기화 완료');
}


/**
 * 라인 페이지 설정 객체
 */
export const lineConfig = {
  resourceName: 'Line',
  apiEndpoint: 'proc/line.php',
  entityId: 'idx',
  
  // 초기화 후 팩토리 데이터 로드
  async beforeInit(api) {
    await loadFactoryOptions(api);
  },
  
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
      sortKey: 'l.idx', 
      visible: false
    },
    {
      key: 'factory_name',
      label: 'Factory Name',
      sortable: true,
      sortKey: 'f.factory_name'
    },
    { 
      key: 'line_name',
      label: 'Line Name',
      sortable: true,
      sortKey: 'l.line_name'
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
      key: 'mp', 
      label: 'MP', 
      sortable: true, 
      sortKey: 'l.mp',
      render: (item) => {
        const mp = parseInt(item.mp) || 0;
        let indicator = '';
        let className = '';
        if (mp >= 10) {
          indicator = '🔴';
          className = 'mp-high';
        } else if (mp >= 10) {
          indicator = '🟡';
          className = 'mp-medium';
        } else {
          indicator = '🟢';
          className = 'mp-low';
        }
        return `
          <span class="mp-indicator ${className}">${indicator} ${mp}</span>`;
      }
    },
    { 
      key: 'line_target', 
      label: 'Target', 
      sortable: true, 
      sortKey: 'l.line_target',
      render: (item) => {
        const target = parseInt(item.line_target) || 0;
        return `
          <span style="display: inline-flex; align-items: center; gap: var(--sap-spacing-xs);">
            ${target.toLocaleString()}
          </span>
        `;
      }
    },
    { 
      key: 'status', 
      label: 'Status', 
      sortable: true, 
      sortKey: 'l.status', 
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
    },
    { 
      elementId: 'factoryFilterSelect',
      paramName: 'factory_filter',
      stateKey: 'factoryFilter'
    }
  ],
};


/**
 * Factory Search Module
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
 * Factory Filters Module
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
 * Line Modal Module
 * 팩토리 페이지 모달 단계 기능 관리
 */

/**
 * 모달 단계 기능 초기화
 */
export function initModalSteps() {
  const nextBtn = document.getElementById('nextStep');
  const prevBtn = document.getElementById('prevStep');
  const submitBtn = document.getElementById('submitBtn');
  const stepIndicator = document.getElementById('stepIndicator');
  
  if (!nextBtn || !prevBtn || !submitBtn || !stepIndicator) {
    console.error('모달 버튼 또는 단계 표시기를 찾을 수 없습니다.');
    return;
  }
  
  let currentStep = 1;
  const totalSteps = 3;
  
  nextBtn.addEventListener('click', async () => {
    if (await validateCurrentStep(currentStep)) {
      currentStep++;
      showStep(currentStep);
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
    if (step === 1) {
      const factory = document.getElementById('factory_idx').value;
      const lineName = document.getElementById('line_name').value.trim();
      
      if (!factory || !lineName) {
        alert('Please fill in all required fields in Basic Information.');
        return false;
      }
      
      // 중복 line_name 이름 검사
      try {
        const currentIdx = document.getElementById('resourceId').value || null;
        const response = await fetch(`proc/line.php?for=check-duplicate&factory_idx=${factory}&line_name=${encodeURIComponent(lineName)}${currentIdx ? `&current_idx=${currentIdx}` : ''}`);
        const result = await response.json();
        
        if (!result.success) {
          alert(result.message);
          return false;
        }
      } catch (error) {
        console.error('중복 확인 중 오류:', error);
        alert('An error occurred while checking for duplicates. Please try again later.');
        return false;
      }
    }
    if (step === 2) {
      const mp = document.getElementById('mp').value;
      const target = document.getElementById('line_target').value;
        if (!mp || mp < 0 || !target || target < 0) {
        alert('Please enter valid values for Man Power and Line Target.');
        return false;
      }
    }
    return true;
  }
  
  /**
   * 미리보기 업데이트
   */
  function updatePreview() {
    const factorySelect = document.getElementById('factory_idx');
    const factoryName = factorySelect.selectedOptions[0]?.textContent || '-';
    const lineName = document.getElementById('line_name').value || '-';    
    const mp = document.getElementById('mp').value || '-';
    const target = document.getElementById('line_target').value || '-';
    const statusText = document.getElementById('status').selectedOptions[0]?.textContent || '-';
    
    const previewName = document.getElementById('previewName');
    const previewLine = document.getElementById('previewLine');
    const previewMP = document.getElementById('previewMP');
    const previewTarget = document.getElementById('previewTarget');
    const previewStatus = document.getElementById('previewStatus');
    
    if (previewName) previewName.textContent = factoryName;
    if (previewLine) previewLine.textContent = lineName;
    if (previewMP) previewMP.textContent = mp;
    if (previewTarget) previewTarget.textContent = target;
    if (previewStatus) previewStatus.textContent = statusText;
  }
  
  // 입력값 변경 시 미리보기 업데이트
  const factorySelect = document.getElementById('factory_idx');
  const lineNameInput = document.getElementById('line_name');
  const mpInput = document.getElementById('mp');
  const targetInput = document.getElementById('line_target');
  const statusSelect = document.getElementById('status');
  
  if (factorySelect) factorySelect.addEventListener('change', updatePreview);
  if (lineNameInput) lineNameInput.addEventListener('input', updatePreview);
  if (mpInput) mpInput.addEventListener('input', updatePreview);
  if (targetInput) targetInput.addEventListener('input', updatePreview);
  if (statusSelect) statusSelect.addEventListener('change', updatePreview);
  
  // 모달 열릴 때 초기화
  const addBtn = document.getElementById('addBtn');
  if (addBtn) {
    addBtn.addEventListener('click', () => {
      currentStep = 1;
      showStep(1);
    });
  }
}


/**
 * Line Statistics Module
 * 팩토리 페이지 통계 기능 관리
 */

/**
 * 통계 업데이터 초기화
 */
export function initStatisticsUpdater() {
  // 초기 통계 로드
  updateStatistics();
  
  // 필터 변경 시에도 통계 업데이트
  const statusFilter = document.getElementById('statusFilterSelect');
  if (statusFilter) {
    statusFilter.addEventListener('change', updateStatistics);
  }
  
  const factoryFilter = document.getElementById('factoryFilterSelect');
  if (factoryFilter) {
    factoryFilter.addEventListener('change', updateStatistics);
  }
  
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
  } else {
    console.error('Table body not found');
  }
}

/**
 * 통계 업데이트 함수
 */
export function updateStatistics() {
  // 모든 통계를 API를 통해 일관되게 가져오기
  fetchAllStatistics();
}

export async function fetchAllStatistics() {
  try {
    // 현재 필터 상태 가져오기
    const factoryFilter = document.getElementById('factoryFilterSelect')?.value || '';
    const statusFilter = document.getElementById('statusFilterSelect')?.value || '';
    
    // API를 통해 모든 통계 데이터 가져오기
    const params = new URLSearchParams();
    if (factoryFilter) params.append('factory_filter', factoryFilter);
    if (statusFilter) params.append('status_filter', statusFilter);
    
    const response = await fetch(`proc/line.php?for=statistics&${params.toString()}`);
    const result = await response.json();
    
    if (result.success && result.data) {
      const stats = result.data;
      animateNumber('totalCount', stats.total_lines || 0);
      animateNumber('totalMachines', stats.total_machines || 0);
      animateNumber('totalManpower', stats.total_manpower || 0);
      animateNumber('totalTarget', stats.total_target || 0);
      animateNumber('targetPerMan', stats.target_per_man || 0);
    } else {
      // API 에러 시 fallback
      animateNumber('totalCount', 0);
      animateNumber('totalMachines', 0);
      animateNumber('totalManpower', 0);
      animateNumber('totalTarget', 0);
      animateNumber('targetPerMan', 0);
    }
  } catch (error) {
    console.error('Failed to fetch statistics:', error);
    // 에러 시 fallback
    animateNumber('totalCount', 0);
    animateNumber('totalMachines', 0);
    animateNumber('totalManpower', 0);
    animateNumber('totalTarget', 0);
    animateNumber('targetPerMan', 0);
  }
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

/**
 * 팩토리 필터 옵션을 로드합니다.
 * @param {Object} api - API 핸들러
 */
async function loadFactoryOptions(api) {
  try {
    console.log('🏢 팩토리 목록 로딩 중...');
    const response = await fetch('proc/line.php?for=factories');
    const result = await response.json();
    
    if (result.success && result.data) {
      const factorySelect = document.getElementById('factoryFilterSelect');
      const modalFactorySelect = document.getElementById('factory_idx');
      
      if (factorySelect) {
        // 필터용 선택 박스에 옵션 추가
        factorySelect.innerHTML = '<option value="">All Factories</option>';
        result.data.forEach(factory => {
          factorySelect.innerHTML += `<option value="${factory.idx}">${factory.factory_name}</option>`;
        });
        console.log('✅ 팩토리 필터 옵션 로드 완료');
      }
      
      if (modalFactorySelect) {
        // 모달 폼용 선택 박스에 옵션 추가
        modalFactorySelect.innerHTML = '<option value="">Select Factory</option>';
        result.data.forEach(factory => {
          modalFactorySelect.innerHTML += `<option value="${factory.idx}">${factory.factory_name}</option>`;
        });
        console.log('✅ 모달 팩토리 옵션 로드 완료');
      }
    } else {
      console.error('❌ 팩토리 목록 로드 실패:', result.message);
    }
  } catch (error) {
    console.error('❌ 팩토리 옵션 로드 에러:', error);
  }
}