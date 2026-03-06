/**
 * Design Process Main Module
 * Main initialization and integrated management for design process page
 */

/**
 * 고급 기능 초기화 함수
 * 모든 디자인 프로세스 페이지 관련 기능들을 초기화
 * @param {Object} resourceManager - ResourceManager 인스턴스
 */
export function initAdvancedFeatures(resourceManager) {
  // Initialize all design process modules
  initRealTimeSearch(resourceManager);
  initQuickActions();
  initModalSteps();
  initStatisticsUpdater();
  initFilePreviewIcons();
  initDeleteButtons(resourceManager);
}

/**
 * 디자인 프로세스 페이지 설정 객체
 */
export const designProcessConfig = {
  resourceName: 'Design Process',
  apiEndpoint: '../manage/proc/design_process.php',
  entityId: 'idx',

  // 데이터 테이블의 컬럼 정의
  columnConfig: [
    { key: 'no', label: 'NO.', sortable: false, render: (item, index) => `<span style="color: var(--sap-text-secondary);">${index + 1}</span>` },
    { key: 'idx', label: 'IDX', sortable: true, sortKey: 'dp.idx', visible: false },
    {
      key: 'factory_name',
      label: 'Factory',
      sortable: true,
      sortKey: 'f.factory_name',
      render: (item) => {
        return item.factory_name ? `<span style="color: var(--sap-text-primary);">${item.factory_name}</span>` : '<span style="color: var(--sap-text-secondary);">N/A</span>';
      }
    },
    {
      key: 'line_name',
      label: 'Line',
      sortable: true,
      sortKey: 'l.line_name',
      render: (item) => {
        return item.line_name ? `<span style="color: var(--sap-text-primary);">${item.line_name}</span>` : '<span style="color: var(--sap-text-secondary);">N/A</span>';
      }
    },
    {
      key: 'model_name',
      label: 'Model Name',
      sortable: true,
      sortKey: 'dp.model_name',
      render: (item) => {
        return item.model_name ? `<span style="color: var(--sap-text-primary);">${item.model_name}</span>` : '<span style="color: var(--sap-text-secondary);">N/A</span>';
      }
    },
    {
      key: 'design_process',
      label: 'Design Process',
      sortable: true,
      sortKey: 'dp.design_process',
      render: (item) => {
        return `<span class="process-name-badge">${item.design_process}</span>`;
      }
    },
    {
      key: 'std_mc_needed',
      label: 'Standard MC',
      sortable: true,
      sortKey: 'dp.std_mc_needed',
      render: (item) => {
        const count = parseInt(item.std_mc_needed) || 0;
        return `<span class="process-mc-badge">${count} MC</span>`;
      }
    },
    {
      key: 'fname',
      label: '📁 SOP',
      sortable: true,
      sortKey: 'dp.fname',
      render: (item) => {
        if (item.fname) {
          return `
            <span class="process-file-badge">
              <span class="file-preview-icon" data-filename="${item.fname}" title="Click to preview image" style="cursor: pointer; color: var(--sap-brand-primary);">📄</span>
              ${item.fname}
            </span>`;
        }
        return '<span style="color: var(--sap-text-secondary);">No file</span>';
      }
    },
    {
      key: 'status',
      label: 'Status',
      sortable: true,
      sortKey: 'dp.status',
      render: (item) => item.status === 'Y' ?
        '<span class="process-status-used">✅ Used</span>' :
        '<span class="process-status-unused">⏸️ Unused</span>'
    },
    {
      key: 'remark',
      label: 'Remark',
      sortable: true,
      sortKey: 'dp.remark',
      render: (item) => item.remark || '<span style="color: var(--sap-text-secondary);">No remarks</span>'
    },
    {
      key: 'actions',
      label: 'Delete',
      sortable: false,
      render: (item) => {
        return `
          <button
            class="fiori-btn fiori-btn--tertiary fiori-btn--sm delete-btn"
            data-idx="${item.idx}"
            data-name="${item.design_process}"
            title="Delete this process"
            style="min-width: auto; padding: 4px 8px;"
          >
          🗑️
          </button>
        `;
      }
    }
  ],

  // 필터 UI 설정
  filterConfig: [
    {
      elementId: 'factoryFilterSelect',
      paramName: 'factory_filter',
      stateKey: 'factoryFilter',
      resets: ['lineFilter']
    },
    {
      elementId: 'factoryLineFilterSelect',
      paramName: 'line_filter',
      stateKey: 'lineFilter'
    },
    {
      elementId: 'statusFilterSelect',
      paramName: 'status_filter',
      stateKey: 'statusFilter'
    }
  ],

  // 테이블 행 렌더링 후 호출되는 콜백 함수
  onTableRender: (data) => {
    updateDesignProcessStatistics(data);

    // 테이블 행에 CSS 클래스 추가
    const tableRows = document.querySelectorAll('#tableBody tr');
    tableRows.forEach(row => {
      row.classList.add('process-table-row');
    });

    // Update table count
    const tableCount = document.getElementById('tableCount');
    if (tableCount) {
      tableCount.textContent = `${data.length} processes`;
    }
  },

  // ResourceManager 초기화 전에 실행될 비동기 함수
  async beforeInit(api) {
    console.log('beforeInit 실행 중, API 테스트 시작');

    // API 테스트: 디자인 프로세스 데이터 확인
    try {
      const testResult = await api.getAll({});
      console.log('API 테스트 결과:', testResult);

      if (testResult.success && testResult.data) {
        console.log('API에서 받은 데이터 개수:', testResult.data.length);
        if (testResult.data.length > 0) {
          console.log('첫 번째 데이터 예시:', testResult.data[0]);
        }
      }
    } catch (error) {
      console.error('API 테스트 오류:', error);
    }

    // Factory 필터 옵션 로드
    await loadFactoryOptions(api);
  },

  // 수정 모달 열기 전에 실행될 콜백 함수
  async beforeEdit(api, data) {
    console.log('beforeEdit 호출됨, 데이터:', data);

    // Factory 선택
    const factorySelect = document.getElementById('factory_idx');
    if (factorySelect && data.factory_idx) {
      factorySelect.value = data.factory_idx;

      // Factory 선택 후 Line 옵션 로드
      const lineSelect = document.getElementById('line_idx');
      if (lineSelect) {
        lineSelect.disabled = false;
        await updateLineOptions(api, data.factory_idx, 'line_idx', 'Please select a line', data.line_idx);
      }
    }

    // Model Name 설정
    const modelNameInput = document.getElementById('model_name');
    if (modelNameInput) {
      modelNameInput.value = data.model_name || '';
    }

    // 모달이 열린 후 기존 파일 정보 업데이트
    setTimeout(() => {
      // 파일 입력 필드 초기화
      const fileInput = document.getElementById('file_upload');
      if (fileInput) {
        fileInput.value = '';
      }

      // 기존 파일 정보 업데이트
      if (window.updateExistingFileInfo) {
        window.updateExistingFileInfo(data.fname);
      }
    }, 100);
  }
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
 * 빠른 액션 필터 기능 초기화
 */
export function initQuickActions() {
  const quickFilters = document.querySelectorAll('.quick-action-btn');

  quickFilters.forEach(filter => {
    filter.addEventListener('click', () => {
      // 모든 필터에서 active 클래스 제거
      quickFilters.forEach(f => f.classList.remove('active'));
      // 클릭된 필터에 active 클래스 추가
      filter.classList.add('active');

      const filterType = filter.dataset.filter;
      applyQuickFilter(filterType);
    });
  });
}

/**
 * 빠른 필터 적용 함수
 */
export function applyQuickFilter(filterType) {
  const tableRows = document.querySelectorAll('#tableBody tr');
  let visibleCount = 0;

  tableRows.forEach(row => {
    let show = true;

    switch (filterType) {
      case 'all':
        show = true;
        break;
      case 'used':
        show = row.textContent.includes('✅ Used');
        break;
      case 'unused':
        show = row.textContent.includes('⏸️ Unused');
        break;
      case 'recent':
        // 최근 추가된 프로세스 (예: 최근 5개)
        const rowIndex = Array.from(tableRows).indexOf(row);
        show = rowIndex < 5;
        break;
    }

    if (show) {
      row.style.display = '';
      visibleCount++;
    } else {
      row.style.display = 'none';
    }
  });

  // 테이블 카운트 업데이트
  const tableCount = document.getElementById('tableCount');
  if (tableCount) {
    tableCount.textContent = `${visibleCount} processes`;
  }
}

/**
 * 통계 업데이터 초기화
 */
export function initStatisticsUpdater() {
  updateDesignProcessStatistics();

  const tableBody = document.getElementById('tableBody');
  if (tableBody) {
    const observer = new MutationObserver(() => {
      updateDesignProcessStatistics();
    });

    observer.observe(tableBody, {
      childList: true,
      subtree: true
    });
  }
}

/**
 * 디자인 프로세스 통계 업데이트 함수
 */
export function updateDesignProcessStatistics(data) {
  console.log('통계 업데이트 함수 호출됨:', data);

  const tableBody = document.getElementById('tableBody');
  if (!tableBody) {
    console.error('테이블 바디를 찾을 수 없습니다.');
    return;
  }

  const rows = Array.from(tableBody.querySelectorAll('tr')).filter(row => row.style.display !== 'none');

  if (data && Array.isArray(data)) {
    console.log('데이터 배열 길이:', data.length);

    // Total Processes
    const totalProcesses = data.length;

    // Used (status === 'Y')
    const usedProcesses = data.filter(process => process.status === 'Y').length;

    // Total Need MC: std_mc_needed 합계
    const totalNeedMC = data.reduce((sum, process) => {
      return sum + (parseInt(process.std_mc_needed) || 0);
    }, 0);

    // AGV Need MC: std_mc_needed > 0인 데이터들의 std_mc_needed 평균
    const validProcesses = data.filter(process => {
      const stdMc = parseInt(process.std_mc_needed) || 0;
      return stdMc > 0;
    });

    let agvNeedMC = 0;
    if (validProcesses.length > 0) {
      const totalValidMC = validProcesses.reduce((sum, process) => {
        return sum + (parseInt(process.std_mc_needed) || 0);
      }, 0);
      agvNeedMC = Math.round((totalValidMC / validProcesses.length) * 10) / 10; // 소수점 1자리까지
    }

    console.log('통계 계산 결과:', {
      totalProcesses,
      usedProcesses,
      totalNeedMC,
      agvNeedMC
    });

    // 애니메이션과 함께 숫자 업데이트
    animateNumber('totalCount', totalProcesses);
    animateNumber('activeCount', usedProcesses);
    animateNumber('totalNeedMC', totalNeedMC);
    animateNumber('agvNeedMC', agvNeedMC);
  } else {
    console.log('데이터가 없거나 배열이 아닙니다:', data);
  }
}

/**
 * 숫자 애니메이션 함수
 * @param {string} id - 엘리먼트 ID
 * @param {number} target - 목표값
 * @param {string} suffix - 접미사
 */
export function animateNumber(id, target, suffix = '') {
  const element = document.getElementById(id);
  if (!element) return;

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
 * 파일 미리보기 아이콘 클릭 이벤트 초기화 (이벤트 위임 사용)
 */
export function initFilePreviewIcons() {
  // 기존 이벤트 리스너가 있으면 제거
  const tableBody = document.getElementById('tableBody');
  if (!tableBody) {
    console.error('tableBody를 찾을 수 없습니다.');
    return;
  }

  // 이벤트 위임을 사용하여 동적으로 생성되는 요소에도 이벤트 적용
  tableBody.addEventListener('click', (e) => {
    // 클릭된 요소 또는 부모 요소가 .file-preview-icon 클래스를 가지고 있는지 확인
    const previewIcon = e.target.closest('.file-preview-icon');

    if (previewIcon) {
      e.stopPropagation(); // 테이블 행 클릭 이벤트 방지
      e.preventDefault();

      const filename = previewIcon.dataset.filename;
      console.log('파일 미리보기 아이콘 클릭:', filename);

      if (filename) {
        showImagePreviewModal(filename);
      } else {
        console.error('파일명이 없습니다.');
      }
    }
  });

  console.log('파일 미리보기 이벤트 위임 설정 완료');
}

/**
 * 이미지 미리보기 모달 표시
 * @param {string} filename - 미리보기할 파일명
 */
export function showImagePreviewModal(filename) {
  console.log('showImagePreviewModal 호출됨:', filename);

  const imagePreviewModal = document.getElementById('imagePreviewModal');
  const previewImage = document.getElementById('previewImage');
  const imageFileName = document.getElementById('imageFileName');

  console.log('모달 요소 확인:', {
    imagePreviewModal: !!imagePreviewModal,
    previewImage: !!previewImage,
    imageFileName: !!imageFileName
  });

  if (!imagePreviewModal || !previewImage || !imageFileName) {
    console.error('이미지 미리보기 모달 요소를 찾을 수 없습니다.');
    return;
  }

  // 이미지 경로 설정 및 모달 표시
  const imagePath = `../../upload/sop/${filename}`;
  console.log('이미지 경로:', imagePath);

  previewImage.src = imagePath;
  imageFileName.textContent = filename;

  // 모달 표시 (두 단계: display + show 클래스)
  imagePreviewModal.style.display = 'flex';

  // 짧은 지연 후 show 클래스 추가 (애니메이션 위해)
  setTimeout(() => {
    imagePreviewModal.classList.add('show');
    console.log('모달 show 클래스 추가 완료');
  }, 10);

  console.log('모달 표시 완료');

  // 이미지 로드 오류 처리
  previewImage.onerror = function () {
    console.error('이미지 로드 실패:', imagePath);
    alert('이미지를 불러올 수 없습니다. 파일이 존재하지 않거나 손상되었을 수 있습니다.');
    imagePreviewModal.style.display = 'none';
  };
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
      const factorySelect = document.getElementById('factory_idx');
      const lineSelect = document.getElementById('line_idx');
      const designProcess = document.getElementById('design_process').value.trim();
      console.log('📝 입력된 Design Process name:', designProcess);

      // Factory 검증
      if (!factorySelect || !factorySelect.value) {
        alert('Please select a factory.');
        if (factorySelect) factorySelect.focus();
        return false;
      }

      // Line 검증
      if (!lineSelect || !lineSelect.value) {
        alert('Please select a line.');
        if (lineSelect) lineSelect.focus();
        return false;
      }

      // Design Process 이름 검증
      if (!designProcess) {
        alert('Please enter the design process name.');
        return false;
      }

      // 중복 디자인 프로세스 이름 검사 (같은 Factory/Line 내에서)
      try {
        const currentIdx = document.getElementById('resourceId').value || null;
        const factoryIdx = factorySelect.value;
        const lineIdx = lineSelect.value;

        const apiUrl = `../manage/proc/design_process.php?for=check-duplicate&design_process=${encodeURIComponent(designProcess)}&factory_idx=${factoryIdx}&line_idx=${lineIdx}${currentIdx ? `&current_idx=${currentIdx}` : ''}`;
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
    const designProcess = document.getElementById('design_process').value || '-';
    const stdMcNeeded = document.getElementById('std_mc_needed').value || '-';
    const statusText = document.getElementById('status').selectedOptions[0]?.textContent || '-';
    const fileInput = document.getElementById('file_upload');
    const fileName = fileInput && fileInput.files.length > 0 ? fileInput.files[0].name : '-';

    const previewName = document.getElementById('previewName');
    const previewStdMc = document.getElementById('previewStdMc');
    const previewStatus = document.getElementById('previewStatus');
    const previewFile = document.getElementById('previewFile');

    if (previewName) previewName.textContent = designProcess;
    if (previewStdMc) previewStdMc.textContent = stdMcNeeded;
    if (previewStatus) previewStatus.textContent = statusText;
    if (previewFile) previewFile.textContent = fileName;
  }

  // 입력값 변경 시 미리보기 업데이트
  const designProcessInput = document.getElementById('design_process');
  const stdMcInput = document.getElementById('std_mc_needed');
  const statusSelect = document.getElementById('status');
  const fileInput = document.getElementById('file_upload');

  if (designProcessInput) designProcessInput.addEventListener('input', updatePreview);
  if (stdMcInput) stdMcInput.addEventListener('input', updatePreview);
  if (statusSelect) statusSelect.addEventListener('change', updatePreview);
  if (fileInput) fileInput.addEventListener('change', updatePreview);

  // 모달 열릴 때 초기화
  const addBtn = document.getElementById('addBtn');
  if (addBtn) {
    addBtn.addEventListener('click', () => {
      currentStep = 1;
      showStep(1);

      // Factory와 Line 초기화
      const factorySelect = document.getElementById('factory_idx');
      const lineSelect = document.getElementById('line_idx');
      if (factorySelect) factorySelect.value = '';
      if (lineSelect) {
        lineSelect.value = '';
        lineSelect.disabled = true;
      }

      // 새 추가 모달일 때 기존 파일 정보 숨기기
      if (window.resetExistingFileInfo) {
        window.resetExistingFileInfo();
      }
    });
  }

  // 테이블 row 클릭으로 수정 모달 열릴 때도 초기화
  const resourceModal = document.getElementById('resourceModal');
  if (resourceModal) {
    // MutationObserver를 사용하여 모달이 열릴 때 초기화
    const observer = new MutationObserver((mutations) => {
      mutations.forEach((mutation) => {
        if (mutation.type === 'attributes' && mutation.attributeName === 'class') {
          if (resourceModal.classList.contains('show')) {
            // 모달이 열릴 때 Step 1로 초기화
            currentStep = 1;
            showStep(1);
          }
        }
      });
    });

    observer.observe(resourceModal, { attributes: true });
  }
}

/**
 * 삭제 버튼 초기화 (이벤트 위임 사용)
 * @param {Object} resourceManager - ResourceManager 인스턴스
 */
export function initDeleteButtons(resourceManager) {
  const tableBody = document.getElementById('tableBody');
  if (!tableBody) {
    console.error('tableBody를 찾을 수 없습니다.');
    return;
  }

  // 이벤트 위임을 사용하여 동적으로 생성되는 삭제 버튼에 이벤트 적용
  tableBody.addEventListener('click', async (e) => {
    // 삭제 버튼 클릭 확인
    const deleteBtn = e.target.closest('.delete-btn');
    if (!deleteBtn) return;

    e.stopPropagation(); // 테이블 행 클릭 이벤트 방지
    e.preventDefault();

    const idx = deleteBtn.dataset.idx;
    const processName = deleteBtn.dataset.name;

    console.log('삭제 버튼 클릭:', { idx, processName });

    // Fiori 스타일 확인 대화상자
    const confirmed = confirm(
      `Are you sure you want to delete this Design Process??\n\n` +
      `Process Name: ${processName}\n\n` +
      `This action is irreversible.`
    );

    if (!confirmed) {
      console.log('삭제 취소됨');
      return;
    }

    // 삭제 API 호출
    try {
      console.log('삭제 API 호출 시작:', idx);

      const response = await fetch(`../manage/proc/design_process.php?id=${idx}`, {
        method: 'DELETE'
      });

      const result = await response.json();
      console.log('삭제 API 응답:', result);

      if (result.success) {
        alert('Design Process has been successfully deleted.');

        // 테이블 새로고침
        if (resourceManager && resourceManager.loadData) {
          await resourceManager.loadData();
        }
      } else {
        alert('Deletion failed: ' + result.message);
      }
    } catch (error) {
      console.error('An error occurred while deleting:', error);
      alert('An error occurred while deleting. Please try again..');
    }
  });

  console.log('삭제 버튼 이벤트 위임 설정 완료');
}

/**
 * Factory 필터 옵션을 로드합니다.
 * @param {Object} api - API 핸들러
 */
async function loadFactoryOptions(api) {
  try {
    const response = await api.getAll({ for: 'factories' });

    if (response.success && response.data) {
      const factoryFilterSelect = document.getElementById('factoryFilterSelect');
      const modalFactorySelect = document.getElementById('factory_idx');

      // 필터용 Factory 선택 박스
      if (factoryFilterSelect) {
        factoryFilterSelect.innerHTML = '<option value=""> All Factories</option>';
        response.data.forEach(factory => {
          factoryFilterSelect.innerHTML += `<option value="${factory.idx}">${factory.factory_name}</option>`;
        });

        // Factory 선택 시 Line 목록 업데이트 (필터용)
        factoryFilterSelect.addEventListener('change', (e) => {
          updateLineOptions(api, e.target.value, 'factoryLineFilterSelect', ' All Lines');
        });
      }

      // 모달용 Factory 선택 박스
      if (modalFactorySelect) {
        modalFactorySelect.innerHTML = '<option value="">Please select a factory</option>';
        response.data.forEach(factory => {
          modalFactorySelect.innerHTML += `<option value="${factory.idx}">${factory.factory_name}</option>`;
        });

        // Factory 선택 시 Line 목록 업데이트 (모달용)
        modalFactorySelect.addEventListener('change', (e) => {
          const lineSelect = document.getElementById('line_idx');
          if (lineSelect) {
            lineSelect.disabled = !e.target.value;
            if (!e.target.value) {
              lineSelect.value = '';
            }
          }
          updateLineOptions(api, e.target.value, 'line_idx', 'Please select a line');
        });
      }

      // Line 선택 박스 초기 로드 (All Lines)
      await updateLineOptions(api, '', 'factoryLineFilterSelect', ' All Lines');
    } else {
      console.error('❌ Factory 목록 로드 실패:', response.message);
    }
  } catch (error) {
    console.error('❌ Factory 옵션 로드 에러:', error);
  }
}

/**
 * Line 드롭다운 내용을 Factory ID에 따라 업데이트합니다.
 * @param {Object} api - API 핸들러 객체
 * @param {string} factoryId - 선택된 Factory ID
 * @param {string} lineElementId - Line SELECT 요소의 ID
 * @param {string} initialText - 기본으로 표시될 텍스트
 * @param {string} preserveValue - 복원할 값 (선택사항)
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
    let url = '../manage/proc/design_process.php?for=lines';
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