/**
 * Defective Main Module
 * Main initialization and management for defective page
 */

export function initAdvancedFeatures(resourceManager) {
  initRealTimeSearch(resourceManager);
  initQuickActions();
  initModalSteps();
  initStatisticsUpdater();
}

export const defectiveConfig = {
  resourceName: 'Defective',
  apiEndpoint: 'proc/defective.php',
  entityId: 'idx',

  columnConfig: [
    { key: 'no', label: 'NO.', sortable: false, render: (item, index) => `<span style="color: var(--sap-text-secondary);">${index}</span>` },
    { key: 'idx', label: 'IDX', sortable: true, sortKey: 'idx', visible: false },
    {
      key: 'defective_name',
      label: 'Defective Name',
      sortable: true,
      sortKey: 'defective_name',
      render: (item) => {
        return `<span class="defective-name-badge">${item.defective_name}</span>`;
      }
    },
    {
      key: 'defective_shortcut',
      label: 'Shortcut',
      sortable: true,
      sortKey: 'defective_shortcut',
      render: (item) => {
        if (item.defective_shortcut && item.defective_shortcut.trim()) {
          const shortcut = item.defective_shortcut.trim();
          const className = shortcut.length > 10 ? 'shortcut-badge long' : 'shortcut-badge';
          return `<code class="${className}">${shortcut}</code>`;
        } else {
          return '<span style="color: var(--sap-text-secondary);">No shortcut</span>';
        }
      }
    },
    {
      key: 'total_count',
      label: 'Total Count',
      sortable: true,
      sortKey: 'total_count',
      render: (item) => {
        const count = item.total_count || 0;
        return `<span style="color: var(--sap-text-primary); font-weight: 500;">${count}</span>`;
      },
      width: '100px'
    },
    {
      key: 'usage_rate',
      label: 'Rate',
      sortable: true,
      sortKey: 'usage_rate',
      render: (item) => {
        const rate = item.usage_rate || 0;
        const color = rate > 30 ? 'var(--sap-status-error)' :
          rate > 15 ? 'var(--sap-status-warning)' :
            'var(--sap-status-success)';
        return `<span style="color: ${color}; font-weight: 500;">${rate}%</span>`;
      },
      width: '80px'
    },
    {
      key: 'status',
      label: '📊 Status',
      sortable: true,
      sortKey: 'status',
      render: (item) => item.status === 'Y' ?
        '<span class="defective-status-used">✅ Used</span>' :
        '<span class="defective-status-unused">⏸️ Unused</span>'
    },
    {
      key: 'remark',
      label: '📝 Remark',
      sortable: true,
      sortKey: 'remark',
      render: (item) => item.remark || '<span style="color: var(--sap-text-secondary);">No remarks</span>'
    }
  ],

  filterConfig: [
    {
      elementId: 'statusFilterSelect',
      paramName: 'status_filter',
      stateKey: 'statusFilter'
    }
  ],

  onTableRender: (data) => {
    // 통계는 별도 API로 처리하므로 여기서는 테이블 관련 작업만
    const tableRows = document.querySelectorAll('#tableBody tr');
    tableRows.forEach(row => {
      row.classList.add('defective-table-row');
    });

    const tableCount = document.getElementById('tableCount');
    if (tableCount) {
      tableCount.textContent = `${data.length} defectives`;
    }
  }
};

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
    performSearch('', resourceManager);
    searchInput.focus();
  });
}

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

export function initQuickActions() {
  const quickFilters = document.querySelectorAll('.quick-action-btn');

  quickFilters.forEach(filter => {
    filter.addEventListener('click', () => {
      quickFilters.forEach(f => f.classList.remove('active'));
      filter.classList.add('active');

      const filterType = filter.dataset.filter;
      applyQuickFilter(filterType);
    });
  });
}

export function applyQuickFilter(filterType) {
  const tableRows = document.querySelectorAll('#tableBody tr');
  let visibleCount = 0;

  tableRows.forEach(row => {
    let show = true;

    switch (filterType) {
      case 'all':
        show = true;
        break;
      case 'active':
        show = row.textContent.includes('✅ Used');
        break;
      case 'inactive':
        show = row.textContent.includes('⏸️ Unused');
        break;
      case 'with-shortcuts':
        const shortcutCell = row.cells[3];
        show = shortcutCell && shortcutCell.textContent.trim() && !shortcutCell.textContent.includes('No shortcut');
        break;
      case 'long-shortcuts':
        const longShortcutCell = row.cells[3];
        const shortcutText = longShortcutCell ? longShortcutCell.textContent.trim() : '';
        show = shortcutText && shortcutText.length >= 10;
        break;
    }

    if (show) {
      row.style.display = '';
      visibleCount++;
    } else {
      row.style.display = 'none';
    }
  });

  const tableCount = document.getElementById('tableCount');
  if (tableCount) {
    tableCount.textContent = `${visibleCount} defectives`;
  }
}

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
 * 통계 업데이트 함수 - 새로운 defective 통계 로직
 */
export function updateStatistics() {
  updateDefectiveStatistics();
}

/**
 * Defective 통계 업데이트 함수 - API 기반 접근 방식
 * 필터 값을 서버에 전달하여 필터링된 통계 받아오기
 */
async function updateDefectiveStatistics() {
  const statsGrid = document.getElementById('statsGrid');
  if (!statsGrid) return;

  try {
    // 현재 필터 값 가져오기
    const statusFilter = document.getElementById('statusFilterSelect')?.value || '';

    // 1. info_defective 테이블에서 defective 종류 목록 가져오기 (필터 적용)
    let defectiveApiUrl = 'proc/defective.php';
    if (statusFilter) {
      defectiveApiUrl += `?status_filter=${encodeURIComponent(statusFilter)}`;
    }

    const defectiveListResponse = await fetch(defectiveApiUrl);
    const defectiveListResult = await defectiveListResponse.json();

    let defectiveTypes = [];
    if (defectiveListResult.success && defectiveListResult.data) {
      defectiveTypes = defectiveListResult.data.map(item => ({
        defective_name: item.defective_name,
        defective_shortcut: item.defective_shortcut || item.defective_name
      }));
    }

    // 2. data_defective 테이블에서 Warning 상태 수량 가져오기 (필터 적용)
    let warningApiUrl = 'proc/defective.php?for=warning-stats';
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
    updateStatCards(totalWarningCount, defectiveTypes, warningCounts);

  } catch (error) {
    console.error('통계 업데이트 중 오류:', error);
    // 오류 시 기본 값으로 표시
    updateStatCards(0, [], {});
  }
}

/**
 * 통계 카드 UI 업데이트
 */
function updateStatCards(totalWarningCount, defectiveTypes, warningCounts) {
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

  // 2. 각 defective_name별 Warning 카드 생성
  defectiveTypes.forEach(defective => {
    const warningCount = warningCounts[defective.defective_name] || 0;
    const cardElement = createWarningStatCard(defective.defective_name, warningCount);
    statsGrid.appendChild(cardElement);
  });
}

/**
 * Warning 통계 카드 생성
 */
function createWarningStatCard(defectiveName, count) {
  const card = document.createElement('div');
  card.className = 'stat-card stat-card--warning';
  card.innerHTML = `
    <div class="stat-value" id="warning-${defectiveName.toLowerCase().replace(/\s+/g, '-')}">${count}</div>
    <div class="stat-label">⚠️ ${defectiveName}</div>
  `;
  return card;
}

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
    }
  });

  prevBtn.addEventListener('click', () => {
    currentStep--;
    showStep(currentStep);
  });

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
    step.title = `Go to Step ${index + 1}`;
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
      const defectiveName = document.getElementById('defective_name').value.trim();
      const defectiveShortcut = document.getElementById('defective_shortcut').value.trim();

      if (!defectiveName) {
        alert('Please enter the defective name.');
        return false;
      }

      try {
        const currentIdx = document.getElementById('resourceId').value || null;
        const apiUrl = `proc/defective.php?for=check-duplicate&defective_name=${encodeURIComponent(defectiveName)}${currentIdx ? `&current_idx=${currentIdx}` : ''}`;

        const response = await fetch(apiUrl);
        const result = await response.json();

        if (!result.success) {
          alert(result.message);
          return false;
        }
      } catch (error) {
        alert('An error occurred while checking for name duplicates. Please try again later.');
        return false;
      }

      if (defectiveShortcut) {
        try {
          const currentIdx = document.getElementById('resourceId').value || null;
          const apiUrl = `proc/defective.php?for=check-duplicate-shortcut&defective_shortcut=${encodeURIComponent(defectiveShortcut)}${currentIdx ? `&current_idx=${currentIdx}` : ''}`;

          const response = await fetch(apiUrl);
          const result = await response.json();

          if (!result.success) {
            alert(result.message);
            return false;
          }
        } catch (error) {
          alert('An error occurred while checking for shortcut duplicates. Please try again later.');
          return false;
        }
      }
    }
    return true;
  }

  function updatePreview() {
    const defectiveName = document.getElementById('defective_name').value || '-';
    const defectiveShortcut = document.getElementById('defective_shortcut').value || '-';
    const statusText = document.getElementById('status').selectedOptions[0]?.textContent || '-';

    const previewName = document.getElementById('previewName');
    const previewShortcut = document.getElementById('previewShortcut');
    const previewStatus = document.getElementById('previewStatus');

    if (previewName) previewName.textContent = defectiveName;
    if (previewShortcut) previewShortcut.textContent = defectiveShortcut;
    if (previewStatus) previewStatus.textContent = statusText;
  }

  const defectiveNameInput = document.getElementById('defective_name');
  const defectiveShortcutInput = document.getElementById('defective_shortcut');
  const statusSelect = document.getElementById('status');

  if (defectiveNameInput) defectiveNameInput.addEventListener('input', updatePreview);
  if (defectiveShortcutInput) defectiveShortcutInput.addEventListener('input', updatePreview);
  if (statusSelect) statusSelect.addEventListener('change', updatePreview);

  const addBtn = document.getElementById('addBtn');
  if (addBtn) {
    addBtn.addEventListener('click', () => {
      currentStep = 1;
      showStep(1);
    });
  }
}

export function animateNumber(id, target, suffix = '') {
  const element = document.getElementById(id);
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