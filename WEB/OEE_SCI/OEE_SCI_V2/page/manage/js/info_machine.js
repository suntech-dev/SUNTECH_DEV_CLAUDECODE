/**
 * Machine Main Module
 * 기계 페이지 메인 초기화 및 통합 관리
 */

/**
 * 고급 기능 초기화 함수
 * 모든 기계 페이지 관련 기능들을 초기화
 * @param {Object} resourceManager - ResourceManager 인스턴스
 */
export function initAdvancedFeatures(resourceManager) {
  console.log('Machine 페이지 고급 기능 초기화 시작...');
  
  // typeFilterSelect 기본값 강제 설정
  const typeFilterSelect = document.getElementById('typeFilterSelect');
  if (typeFilterSelect && typeFilterSelect.value === 'P') {
    console.log('🔧 typeFilterSelect 기본값 "P" 감지, 강제 필터 적용');
    // ResourceManager 상태 업데이트
    if (resourceManager && resourceManager.state) {
      resourceManager.state.typeFilter = 'P';
      console.log('🔧 resourceManager.state.typeFilter = "P" 설정 완료');
      // 데이터 새로고침
      setTimeout(() => {
        if (resourceManager.loadData) {
          console.log('🔧 데이터 새로고침 실행');
          resourceManager.loadData();
        }
      }, 100);
    }
  }
  
  // updateStatistics 함수를 전역으로 등록 (resource-manager에서 호출할 수 있도록)
  window.updateStatistics = updateStatistics;
  
  // 각 모듈 초기화
  initRealTimeSearch(resourceManager);
  initQuickActions();
  initModalSteps();
  initStatisticsUpdater();
  
  console.log('Machine 페이지 고급 기능 초기화 완료');
}

/**
 * 기계 페이지 설정 객체
 */
export const machineConfig = {
  resourceName: 'Machine',
  apiEndpoint: 'proc/machine.php',
  entityId: 'idx',
  
  // 초기화 후 팩토리 데이터 로드
  async beforeInit(api) {
    await loadFactoryOptions(api);
    await loadModelOptions(api);
    await loadLineOptions(api);
    await loadMachineOptions(api);
  },
  
  // 컬럼 정의 (향상된 렌더링, JOIN된 테이블의 컬럼 포함)
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
      sortKey: 'm.idx', 
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
      key: 'machine_model_name',
      label: 'Model',
      sortable: true,
      sortKey: 'mm.machine_model_name'
    },
    {
      key: 'machine_no',
      label: 'Machine No',
      sortable: true,
      sortKey: 'm.machine_no'
    },
    {
      key: 'design_process',
      label: 'Design Process',
      sortable: true,
      sortKey: 'dp.design_process',
      render: (item) => {
        return item.design_process ? `<span style="color: var(--sap-text-primary); font-weight: 500;">${item.design_process}</span>` : '<span style="color: var(--sap-text-secondary);">Not assigned</span>';
      }
    },
    {
      key: 'mac',
      label: 'MAC Address',
      sortable: true,
      sortKey: 'm.mac',
      render: (item) => {
        return item.mac ? `<code style="font-family: monospace; background: var(--sap-surface-3); padding: 2px 6px; border-radius: 4px;">${item.mac}</code>` : '<span style="color: var(--sap-text-secondary);">Not set</span>';
      }
    },
    { 
      key: 'type', 
      label: 'Type', 
      sortable: true, 
      sortKey: 'm.type',
      render: (item) => {
        if (item.type === 'P') {
          return '<span style="color: var(--sap-brand-primary); font-weight: 600;">Computer Sewing Machine</span>';
        } else if (item.type === 'E') {
          return '<span style="color: var(--sap-status-warning); font-weight: 600;">Embroidery Machine</span>';
        }
        return '<span style="color: var(--sap-text-secondary);">Unknown</span>';
      }
    },
    { 
      key: 'target', 
      label: 'Target', 
      sortable: true, 
      sortKey: 'm.target',
      render: (item) => {
        const target = parseInt(item.target) || 0;
        return target.toLocaleString();
      }
    },
    { 
      key: 'status', 
      label: 'Status', 
      sortable: true, 
      sortKey: 'm.status', 
      render: (item) => item.status === 'Y' ? 
        '<span style="color: var(--sap-status-success); font-weight: 600;">✅ Used</span>' : 
        '<span style="color: var(--sap-status-error); font-weight: 600;">⚠️ Unused</span>'
    },
    { 
      key: 'app_ver', 
      label: 'APP VERSION', 
      sortable: true, 
      sortKey: 'm.app_ver'
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
      elementId: 'typeFilterSelect',
      paramName: 'type_filter',
      stateKey: 'typeFilter'
    },
    { 
      elementId: 'factoryFilterSelect',
      paramName: 'factory_filter',
      stateKey: 'factoryFilter',
      resets: ['lineFilter', 'machineFilter']
    },
    { 
      elementId: 'factoryLineFilterSelect',
      paramName: 'line_filter',
      stateKey: 'lineFilter',
      resets: ['machineFilter']
    },
    { 
      elementId: 'factoryLineMachineFilterSelect',
      paramName: 'machine_filter',
      stateKey: 'machineFilter'
    }
  ],
};

/**
 * 실시간 검색 기능 초기화
 * @param {Object} resourceManager - ResourceManager 인스턴스
 */
export function initRealTimeSearch(resourceManager) {
  const searchInput = document.getElementById('realTimeSearch');
  const searchClear = document.getElementById('searchClear');
  let searchTimeout;
  
  if (!searchInput || !searchClear) {
    console.error('검색 입력 필드 또는 클리어 버튼을 찾을 수 없습니다.');
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
    performSearch('', resourceManager);
    searchInput.focus();
  });
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
 * Initialize quick action buttons
 */
export function initQuickActions() {
  const actionBtns = document.querySelectorAll('.quick-action-btn');
  
  actionBtns.forEach(btn => {
    btn.addEventListener('click', () => {
      // Change active state
      actionBtns.forEach(b => b.classList.remove('active'));
      btn.classList.add('active');
      
      // Apply filter
      const filter = btn.dataset.filter;
      applyQuickFilter(filter);
    });
  });
}

/**
 * Apply quick filter function
 * @param {string} filter - Filter type ('all', 'used', 'unused')
 */
export function applyQuickFilter(filter) {
  const statusSelect = document.getElementById('statusFilterSelect');
  
  if (!statusSelect) {
    console.error('Status filter select box not found.');
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
      // Step 1: Factory, Line, Machine Name 검증
      const factorySelect = document.getElementById('factory_idx');
      const lineSelect = document.getElementById('line_idx');
      const machineNo = document.getElementById('machine_no');
      
      if (factorySelect && !factorySelect.value) {
        alert('공장을 선택해주세요.');
        factorySelect.focus();
        return false;
      }
      
      if (lineSelect && !lineSelect.value) {
        alert('라인을 선택해주세요.');
        lineSelect.focus();
        return false;
      }
      
      if (machineNo && !machineNo.value.trim()) {
        alert('기계 이름을 입력해주세요.');
        machineNo.focus();
        return false;
      }
      
      // 중복 기계 번호 검사
      try {
        const resourceIdEl = document.getElementById('resourceId');
        const currentIdx = resourceIdEl ? resourceIdEl.value : null;
        const apiUrl = `proc/machine.php?for=check-duplicate&machine_no=${encodeURIComponent(machineNo.value.trim())}${currentIdx ? `&current_idx=${currentIdx}` : ''}`;
        
        const response = await fetch(apiUrl);
        const result = await response.json();
        
        if (!result.success) {
          alert(result.message);
          return false;
        }
        
      } catch (error) {
        console.error('❌ 중복 확인 중 오류:', error);
        alert('An error occurred while checking for duplicates. Please try again later.');
        return false;
      }
    }
    if (step === 2) {
      // Step 2: Type, Machine Model, Status 검증
      const typeSelect = document.getElementById('type');
      const modelSelect = document.getElementById('machine_model_idx');
      const statusSelect = document.getElementById('status');
      
      if (typeSelect && !typeSelect.value) {
        alert('기계 타입을 선택해주세요.');
        typeSelect.focus();
        return false;
      }
      
      if (modelSelect && !modelSelect.value) {
        alert('기계 모델을 선택해주세요.');
        modelSelect.focus();
        return false;
      }
      
      if (statusSelect && !statusSelect.value) {
        alert('상태를 선택해주세요.');
        statusSelect.focus();
        return false;
      }
    }
    if (step === 3) {
      // Step 3: Target, Remark 검증 (선택사항)
      // 필수 필드가 아니므로 기본 검증만 수행
      const target = document.getElementById('target');
      
      if (target && target.value && isNaN(parseInt(target.value))) {
        alert('목표 생산량에는 숫자만 입력해주세요.');
        target.focus();
        return false;
      }
    }
    return true;
  }
  
  /**
   * 미리보기 업데이트
   */
  function updatePreview() {
    // Step 3는 더 이상 Preview 단계가 아니므로 updatePreview 기능을 간소화
    // 기존 updatePreview 역할을 단계별로 완료 후 자동으로 다음 단계로 이동할 수 있도록 개선
    // Preview functionality simplified
  }
  
  // Auto-save functionality removed for simplicity
}

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
  } else {
    console.error('테이블 바디를 찾을 수 없습니다.');
  }
}

/**
 * 통계 업데이트 함수
 */
export function updateStatistics(data) {
  // 만약 data가 전달되었다면 그것을 사용, 아니면 DOM에서 추출
  if (data && Array.isArray(data)) {
    calculateStatsFromData(data);
  } else {
    calculateStatsFromDOM();
  }
}

/**
 * 전달받은 데이터 배열로부터 통계 계산
 * @param {Array} data - 기계 데이터 배열
 */
function calculateStatsFromData(data) {
  const totalMachines = data.length;
  // const uniqueModels = [...new Set(data.map(m => m.machine_model_name))].length;
  // const activeMachines = data.filter(machine => machine.status === 'Y').length;
  // const inactive = totalMachines - activeMachines;
  const patternMachines = data.filter(machine => machine.type === 'P').length;
  const embroideryMachines = data.filter(machine => machine.type === 'E').length;
  
  // target > 0인 머신만 계산
  const machinesWithTarget = data.filter(machine => (parseInt(machine.target) || 0) > 0);
  const totalTargets = machinesWithTarget.reduce((sum, machine) => sum + (parseInt(machine.target) || 0), 0);
  const avgTarget = machinesWithTarget.length > 0 ? Math.round(totalTargets / machinesWithTarget.length) : 0;

  // 애니메이션과 함께 업데이트
  animateNumber('totalCount', totalMachines);
  // animateNumber('totalModels', uniqueModels);
  // animateNumber('activeMachines', activeMachines);
  // animateNumber('inactiveCount', inactive);
  animateNumber('patternMachines', patternMachines);
  animateNumber('embroideryMachines', embroideryMachines);
  animateNumber('totalTarget', totalTargets);
  animateNumber('avgTarget', avgTarget);
}

/**
 * DOM에서 통계 계산 (fallback용)
 */
function calculateStatsFromDOM() {
  const tableBody = document.getElementById('tableBody');
  if (!tableBody) {
    console.error('테이블 바디를 찾을 수 없습니다.');
    return;
  }
  
  const rows = Array.from(tableBody.querySelectorAll('tr')).filter(row => row.style.display !== 'none');
  
  const totalMachines = rows.length;
  const models = new Set();
  let activeMachines = 0;
  let patternMachines = 0;
  let embroideryMachines = 0;
  let totalTargets = 0;
  let machinesWithTargetCount = 0; // target > 0인 머신 개수
  
  rows.forEach(row => {
    const cells = row.querySelectorAll('td');
    const rowText = row.textContent;
    
    // 모델명 수집
    cells.forEach(cell => {
      if (cell.textContent && !cell.textContent.includes('✅') && !cell.textContent.includes('⚠️') && !cell.textContent.includes('🧵') && !cell.textContent.includes('🎨')) {
        const text = cell.textContent.trim();
        if (text && text !== '-' && !text.match(/^\d+$/) && !text.includes(':')) {
          models.add(text);
        }
      }
    });
    
    if (rowText.includes('✅ Used')) activeMachines++;
    if (rowText.includes('Computer Sewing Machine')) patternMachines++;
    if (rowText.includes('Embroidery Machine')) embroideryMachines++;
    
    // 타겟 숫자 추출
    const targetMatch = rowText.match(/(\d{1,3}(?:,\d{3})*)/g);
    if (targetMatch) {
      const target = parseInt(targetMatch[targetMatch.length - 1].replace(/,/g, ''));
      if (!isNaN(target) && target > 0) {
        totalTargets += target;
        machinesWithTargetCount++;
      }
    }
  });
  
  const avgTarget = machinesWithTargetCount > 0 ? Math.round(totalTargets / machinesWithTargetCount) : 0;

  // 애니메이션과 함께 업데이트
  animateNumber('totalCount', totalMachines);
  // animateNumber('totalModels', models.size);
  // animateNumber('activeMachines', activeMachines);
  animateNumber('patternMachines', patternMachines);
  animateNumber('embroideryMachines', embroideryMachines);
  animateNumber('totalTarget', totalTargets);
  animateNumber('avgTarget', avgTarget);
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
    const response = await api.getAll({ for: 'factories' });
    
    if (response.success && response.data) {
      const factorySelect = document.getElementById('factoryFilterSelect');
      const modalFactorySelect = document.getElementById('factory_idx');
      
      if (factorySelect) {
        factorySelect.innerHTML = '<option value="">All Factories</option>';
        response.data.forEach(factory => {
          factorySelect.innerHTML += `<option value="${factory.idx}">${factory.factory_name}</option>`;
        });
      }
      
      if (modalFactorySelect) {
        modalFactorySelect.innerHTML = '<option value="">Select Factory</option>';
        response.data.forEach(factory => {
          modalFactorySelect.innerHTML += `<option value="${factory.idx}">${factory.factory_name}</option>`;
        });
      }
      
      // Factory 선택 시 Line 목록 업데이트 (필터용)
      if (factorySelect) {
        factorySelect.addEventListener('change', (e) => {
          updateLineOptions(api, e.target.value, 'factoryLineFilterSelect', 'All Lines');
        });
      }
      
      // Factory 선택 시 Line 목록 업데이트 (모달용)
      if (modalFactorySelect) {
        modalFactorySelect.addEventListener('change', (e) => {
          const lineSelect = document.getElementById('line_idx');
          const preserveLineValue = e.preserveLineValue;
          
          if (lineSelect) {
            if (!preserveLineValue) {
              lineSelect.value = '';
            }
            lineSelect.disabled = !e.target.value;
          }
          
          updateLineOptions(api, e.target.value, 'line_idx', 'Please select a line', preserveLineValue);
        });
      }
      
      // Line 선택 시 Machine Model 자동 필터링 (선택사항)
      const modalLineSelect = document.getElementById('line_idx');
      if (modalLineSelect) {
        modalLineSelect.addEventListener('change', (e) => {
          // Additional logic for machine model filtering can be added here
        });
      }
      
      // Line 선택 시 Machine 목록 업데이트
      const lineFilterSelect = document.getElementById('factoryLineFilterSelect');
      if (lineFilterSelect) {
        lineFilterSelect.addEventListener('change', (e) => {
          updateMachineOptions(api, factorySelect.value, e.target.value, 'factoryLineMachineFilterSelect', 'All Machines');
        });
      }
    } else {
      console.error('❌ 팩토리 목록 로드 실패:', response.message);
    }
  } catch (error) {
    console.error('❌ 팩토리 옵션 로드 에러:', error);
  }
}

/**
 * 모델 필터 옵션을 로드합니다.
 * @param {Object} api - API 핸들러
 */
async function loadModelOptions(api) {
  try {
    console.log('🏷️ 모델 목록 로딩 중...');
    const response = await api.getAll({ for: 'models' });
    
    if (response.success && response.data) {
      const modalModelSelect = document.getElementById('machine_model_idx');
      
      if (modalModelSelect) {
        modalModelSelect.innerHTML = '<option value="">Select Model</option>';
        response.data.forEach(model => {
          modalModelSelect.innerHTML += `<option value="${model.idx}">${model.machine_model_name}</option>`;
        });
        console.log('✅ 모델 옵션 로드 완료');
      }
    } else {
      console.error('❌ 모델 목록 로드 실패:', response.message);
    }
  } catch (error) {
    console.error('❌ 모델 옵션 로드 에러:', error);
  }
}

/**
 * 라인 필터 옵션을 로드합니다.
 * @param {Object} api - API 핸들러
 */
async function loadLineOptions(api) {
  try {
    console.log('🏭 라인 목록 로딩 중...');
    const response = await api.getAll({ for: 'lines' });
    
    if (response.success && response.data) {
      const lineFilterSelect = document.getElementById('factoryLineFilterSelect');
      
      if (lineFilterSelect) {
        lineFilterSelect.innerHTML = '<option value="">All Lines</option>';
        response.data.forEach(line => {
          lineFilterSelect.innerHTML += `<option value="${line.idx}">${line.line_name}</option>`;
        });
        console.log('✅ 라인 필터 옵션 로드 완료');
      }
    } else {
      console.error('❌ 라인 목록 로드 실패:', response.message);
    }
  } catch (error) {
    console.error('❌ 라인 옵션 로드 에러:', error);
  }
}

/**
 * 머신 필터 옵션을 로드합니다.
 * @param {Object} api - API 핸들러
 */
async function loadMachineOptions(api) {
  try {
    console.log('🔧 머신 목록 로딩 중...');
    const response = await api.getAll({});

    if (response.success && response.data) {
      const machineFilterSelect = document.getElementById('factoryLineMachineFilterSelect');

      if (machineFilterSelect) {
        machineFilterSelect.innerHTML = '<option value="">All Machines</option>';
        response.data.forEach(machine => {
          machineFilterSelect.innerHTML += `<option value="${machine.idx}">${machine.machine_no} (${machine.machine_model_name || 'No Model'})</option>`;
        });
        console.log('✅ 머신 필터 옵션 로드 완료');
      }
    } else {
      console.error('❌ 머신 목록 로드 실패:', response.message);
    }
  } catch (error) {
    console.error('❌ 머신 옵션 로드 에러:', error);
  }
}

/**
 * Update line dropdown content based on Factory ID
 * @param {Object} api - API handler object
 * @param {string} factoryId - Selected Factory ID
 * @param {string} lineElementId - Line SELECT element ID to update
 * @param {string} initialText - Default text to display
 * @param {string} preserveValue - Optional value to preserve/restore
 */
async function updateLineOptions(api, factoryId, lineElementId, initialText, preserveValue = null) {
  const lineSelect = document.getElementById(lineElementId);
  
  if (!lineSelect) {
    console.error(`Line select element not found: ${lineElementId}`);
    return;
  }
  
  lineSelect.disabled = true;
  
  try {
    console.log(`🔄 Line 옵션 업데이트 중... Factory ID: ${factoryId}`);
    
    // API URL을 직접 구성해서 GET 요청으로 보냄
    let url = 'proc/machine.php?for=lines';
    if (factoryId) {
      url += `&factory_id=${factoryId}`;
    }
    
    const response = await fetch(url);
    const res = await response.json();
    
    if (res.success) {
      const currentValue = preserveValue || lineSelect.value;
      
      lineSelect.innerHTML = `<option value="">${initialText}</option>`;
      res.data.forEach(line => {
        lineSelect.innerHTML += `<option value="${line.idx}">${line.line_name}</option>`;
      });
      
      if (currentValue && lineSelect.querySelector(`option[value="${currentValue}"]`)) {
        lineSelect.value = currentValue;
        
        setTimeout(() => {
          const changeEvent = new Event('change', { bubbles: true });
          lineSelect.dispatchEvent(changeEvent);
        }, 50);
      }
      
    } else {
      lineSelect.innerHTML = `<option value="">${initialText}</option>`;
      console.log(`Failed to load line list: ${res.message}`);
    }
    
    // 마지막에 disabled 해제
    lineSelect.disabled = false;
    
  } catch (error) {
    console.error(`Failed to load lines for factory ${factoryId}:`, error);
    lineSelect.innerHTML = `<option value="">${initialText}</option>`;
    lineSelect.disabled = false;
  }
}


/**
 * Factory ID와 Line ID에 따라 Machine 드롭다운 메뉴의 내용을 비동기적으로 업데이트하는 함수
 * @param {Object} api - API 핸들러 객체
 * @param {string} factoryId - 선택된 Factory의 ID
 * @param {string} lineId - 선택된 Line의 ID
 * @param {string} machineElementId - 업데이트할 Machine SELECT 요소의 ID
 * @param {string} initialText - 기본으로 표시될 텍스트
 */
async function updateMachineOptions(api, factoryId, lineId, machineElementId, initialText) {
  const machineSelect = document.getElementById(machineElementId);
  machineSelect.disabled = true;

  try {
    const params = {};
    if (factoryId) {
      params.factory_filter = factoryId;
    }
    if (lineId) {
      params.line_filter = lineId;
    }

    const res = await api.getAll(params);
    if (res.success) {
      machineSelect.innerHTML = `<option value="">${initialText}</option>`;
      res.data.forEach(machine => {
        machineSelect.innerHTML += `<option value="${machine.idx}">${machine.machine_no} (${machine.machine_model_name || 'No Model'})</option>`;
      });
      console.log(`✅ 머신 목록 로드 완료 - Factory: ${factoryId}, Line: ${lineId}, Count: ${res.data.length}`);
    } else {
      machineSelect.innerHTML = `<option value="">${initialText}</option>`;
      console.log(`⚠️ 머신 목록 로드 실패: ${res.message}`);
    }
    machineSelect.disabled = false;
  } catch (error) {
    console.error(`Failed to load machines for factory ${factoryId}, line ${lineId}:`, error);
    machineSelect.innerHTML = `<option value="">${initialText}</option>`;
    machineSelect.disabled = false;
  }
}