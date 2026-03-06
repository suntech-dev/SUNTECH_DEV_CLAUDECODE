export function initAdvancedFeatures(resourceManager) {
  initRealTimeSearch(resourceManager);
  initQuickActions();
  initModalSteps();
  initStatisticsUpdater();
}

export const downtimeConfig = {
  resourceName: 'Downtime',
  apiEndpoint: 'proc/downtime.php',
  entityId: 'idx',
  
  columnConfig: [
    { key: 'no', label: 'NO.', sortable: false, render: (item, index) => `<span style="color: var(--sap-text-secondary);">${index}</span>` },
    { key: 'idx', label: 'IDX', sortable: true, sortKey: 'idx', visible: false },
    { 
      key: 'downtime_name', 
      label: '⏱️ Downtime Name', 
      sortable: true, 
      sortKey: 'downtime_name',
      render: (item) => {
        const type = item.downtime_type || 'Planned';
        let typeClass = 'downtime-planned';
        let typeIcon = '';
        
        switch (type.toLowerCase()) {
          case 'critical':
            typeClass = 'downtime-critical';
            typeIcon = '🚨';
            break;
          case 'unplanned':
            typeClass = 'downtime-unplanned';
            typeIcon = '⚠️';
            break;
        }
        
        return `<span class="downtime-name-badge ${typeClass}">${typeIcon} ${item.downtime_name}</span>`;
      }
    },
    { 
      key: 'downtime_shortcut', 
      label: '⌨️ Shortcut', 
      sortable: true, 
      sortKey: 'downtime_shortcut',
      render: (item) => {
        if (item.downtime_shortcut && item.downtime_shortcut.trim()) {
          const shortcut = item.downtime_shortcut.trim();
          const className = shortcut.length > 10 ? 'shortcut-badge long' : 'shortcut-badge';
          return `<code class="${className}">${shortcut}</code>`;
        } else {
          return '<span style="color: var(--sap-text-secondary);">No shortcut</span>';
        }
      }
    },
    { 
      key: 'total_count', 
      label: '📊 Total', 
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
      label: '✅ Completed', 
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
      label: '⚠️ Warning', 
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
      label: '📈 Warning %', 
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
      label: '📊 Status', 
      sortable: true, 
      sortKey: 'status', 
      render: (item) => item.status === 'Y' ? 
        '<span class="downtime-status-used">✅ Used</span>' : 
        '<span class="downtime-status-unused">⏸️ Unused</span>'
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
      row.classList.add('downtime-table-row');
    });
    
    const tableCount = document.getElementById('tableCount');
    if (tableCount) {
      tableCount.textContent = `${data.length} downtimes`;
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
  const quickFilters = document.querySelectorAll('.quick-filter');
  
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
      case 'critical':
        show = row.textContent.includes('🚨 Critical');
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
    tableCount.textContent = `${visibleCount} downtimes`;
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
 * 통계 업데이트 함수 - 새로운 downtime 통계 로직
 */
export function updateStatistics() {
  updateDowntimeStatistics();
}

/**
 * Downtime 통계 업데이트 함수 - API 기반 접근 방식
 * 필터 값을 서버에 전달하여 필터링된 통계 받아오기
 */
async function updateDowntimeStatistics() {
  const statsGrid = document.getElementById('statsGrid');
  if (!statsGrid) return;

  try {
    // 현재 필터 값 가져오기
    const statusFilter = document.getElementById('statusFilterSelect')?.value || '';
    
    // 1. info_downtime 테이블에서 downtime 종류 목록 가져오기 (필터 적용)
    let downtimeApiUrl = 'proc/downtime.php';
    if (statusFilter) {
      downtimeApiUrl += `?status_filter=${encodeURIComponent(statusFilter)}`;
    }
    
    const downtimeListResponse = await fetch(downtimeApiUrl);
    const downtimeListResult = await downtimeListResponse.json();
    
    let downtimeTypes = [];
    if (downtimeListResult.success && downtimeListResult.data) {
      downtimeTypes = downtimeListResult.data.map(item => ({
        downtime_name: item.downtime_name,
        downtime_shortcut: item.downtime_shortcut || item.downtime_name
      }));
    }
    
    // 2. data_downtime 테이블에서 Warning 상태 수량 가져오기 (필터 적용)
    let warningApiUrl = 'proc/downtime.php?for=warning-stats';
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
    updateStatCards(totalWarningCount, downtimeTypes, warningCounts);
    
  } catch (error) {
    console.error('통계 업데이트 중 오류:', error);
    // 오류 시 기본 값으로 표시
    updateStatCards(0, [], {});
  }
}

/**
 * 통계 카드 UI 업데이트
 */
function updateStatCards(totalWarningCount, downtimeTypes, warningCounts) {
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
  
  // 2. 각 downtime_name별 Warning 카드 생성
  downtimeTypes.forEach(downtime => {
    const warningCount = warningCounts[downtime.downtime_name] || 0;
    const cardElement = createWarningStatCard(downtime.downtime_name, warningCount, downtime.downtime_shortcut);
    statsGrid.appendChild(cardElement);
  });
}

/**
 * Warning 통계 카드 생성
 */
function createWarningStatCard(downtimeName, count, shortcut) {
  const card = document.createElement('div');
  card.className = 'stat-card stat-card--warning';
  card.innerHTML = `
    <div class="stat-value" id="warning-${downtimeName.toLowerCase().replace(/\s+/g, '-')}">${count}</div>
    <div class="stat-label">⚠️ ${downtimeName}</div>
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
      const downtimeName = document.getElementById('downtime_name').value.trim();
      const downtimeShortcut = document.getElementById('downtime_shortcut').value.trim();
      
      if (!downtimeName) {
        alert('Please enter the downtime name.');
        return false;
      }
      
      try {
        const currentIdx = document.getElementById('resourceId').value || null;
        const apiUrl = `proc/downtime.php?for=check-duplicate&downtime_name=${encodeURIComponent(downtimeName)}${currentIdx ? `&current_idx=${currentIdx}` : ''}`;
        
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
      
      if (downtimeShortcut) {
        try {
          const currentIdx = document.getElementById('resourceId').value || null;
          const apiUrl = `proc/downtime.php?for=check-duplicate-shortcut&downtime_shortcut=${encodeURIComponent(downtimeShortcut)}${currentIdx ? `&current_idx=${currentIdx}` : ''}`;
          
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
    const downtimeName = document.getElementById('downtime_name').value || '-';
    const downtimeShortcut = document.getElementById('downtime_shortcut').value || '-';
    const statusText = document.getElementById('status').selectedOptions[0]?.textContent || '-';
    
    const previewName = document.getElementById('previewName');
    const previewShortcut = document.getElementById('previewShortcut');
    const previewStatus = document.getElementById('previewStatus');
    
    if (previewName) previewName.textContent = downtimeName;
    if (previewShortcut) previewShortcut.textContent = downtimeShortcut;
    if (previewStatus) previewStatus.textContent = statusText;
  }
  
  const downtimeNameInput = document.getElementById('downtime_name');
  const downtimeShortcutInput = document.getElementById('downtime_shortcut');
  const statusSelect = document.getElementById('status');
  
  if (downtimeNameInput) downtimeNameInput.addEventListener('input', updatePreview);
  if (downtimeShortcutInput) downtimeShortcutInput.addEventListener('input', updatePreview);
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