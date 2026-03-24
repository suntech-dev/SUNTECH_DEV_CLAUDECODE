
export function initAdvancedFeatures(resourceManager) {
  initRealTimeSearch(resourceManager);
  initQuickActions();
  initModalSteps();
}

export const lineConfig = {
  resourceName: 'Line',
  apiEndpoint: 'proc/line.php',
  entityId: 'idx',

  async beforeInit(api) {
    await loadFactoryOptions(api);
  },

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
      sortKey: 'l.line_name',
      render: (item) => `<strong style="color: var(--sap-text-primary);">${item.line_name}</strong>`
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
        return `<span style="color: var(--sap-text-primary); font-weight: 500;">${mp}</span>`;
      },
      width: '70px'
    },
    {
      key: 'line_target',
      label: 'Target',
      sortable: true,
      sortKey: 'l.line_target',
      render: (item) => {
        const target = parseInt(item.line_target) || 0;
        return `<span style="color: var(--sap-text-primary);">${target.toLocaleString()}</span>`;
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
            ${isActive ? 'Used' : 'Unused'}
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
  ]
};


export function initRealTimeSearch(resourceManager) {
  const searchInput = document.getElementById('realTimeSearch');
  const searchClear = document.getElementById('searchClear');
  let searchTimeout;

  if (!searchInput || !searchClear) return;

  searchInput.addEventListener('input', (e) => {
    const value = e.target.value.trim();
    searchClear.classList.toggle('visible', !!value);
    clearTimeout(searchTimeout);
    searchTimeout = setTimeout(() => performSearch(value, resourceManager), 300);
  });

  searchClear.addEventListener('click', () => {
    searchInput.value = '';
    searchClear.classList.remove('visible');
    performSearch('', resourceManager);
    searchInput.focus();
  });

  searchInput.addEventListener('keydown', (e) => {
    if (e.key === 'Enter') {
      clearTimeout(searchTimeout);
      performSearch(searchInput.value.trim(), resourceManager);
    }
  });
}

export function performSearch(query, resourceManager) {
  if (resourceManager && resourceManager.state) {
    resourceManager.state.searchQuery = query;
    resourceManager.state.currentPage = 1;
  }
  if (resourceManager && resourceManager.loadData) {
    resourceManager.loadData();
  }
}


export function initQuickActions() {
  const actionBtns = document.querySelectorAll('.quick-action-btn');
  actionBtns.forEach(btn => {
    btn.addEventListener('click', () => {
      actionBtns.forEach(b => b.classList.remove('active'));
      btn.classList.add('active');
      applyQuickFilter(btn.dataset.filter);
    });
  });
}

export function applyQuickFilter(filter) {
  const statusSelect = document.getElementById('statusFilterSelect');
  if (!statusSelect) return;
  const map = { used: 'Y', unused: 'N', all: '' };
  statusSelect.value = map[filter] ?? '';
  statusSelect.dispatchEvent(new Event('change'));
}


export function initModalSteps() {
  const nextBtn       = document.getElementById('nextStep');
  const prevBtn       = document.getElementById('prevStep');
  const submitBtn     = document.getElementById('submitBtn');
  const stepIndicator = document.getElementById('stepIndicator');

  if (!nextBtn || !prevBtn || !submitBtn || !stepIndicator) return;

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

  document.querySelectorAll('.form-step').forEach((step, index) => {
    step.style.cursor = 'pointer';
    step.title = `Step ${index + 1}`;
    step.addEventListener('click', async () => {
      const target = index + 1;
      if (target < currentStep) {
        currentStep = target;
        showStep(currentStep);
      } else if (target > currentStep) {
        let valid = true, tmp = currentStep;
        while (tmp < target && valid) {
          valid = await validateCurrentStep(tmp);
          if (valid) tmp++;
        }
        if (valid) { currentStep = target; showStep(currentStep); }
      }
    });
  });

  function showStep(step) {
    document.querySelectorAll('.form-section').forEach(s => {
      s.style.display = s.dataset.section == step ? 'block' : 'none';
    });
    prevBtn.style.display   = step > 1            ? 'inline-block' : 'none';
    nextBtn.style.display   = step < totalSteps   ? 'inline-block' : 'none';
    submitBtn.style.display = step === totalSteps ? 'inline-block' : 'none';
    stepIndicator.textContent = `Step ${step} of ${totalSteps}`;
    document.querySelectorAll('.form-step').forEach(s => {
      s.classList.toggle('active', parseInt(s.dataset.step) <= step);
    });
    updatePreview();
  }

  async function validateCurrentStep(step) {
    if (step === 1) {
      const factory  = document.getElementById('factory_idx').value;
      const lineName = document.getElementById('line_name').value.trim();
      if (!factory || !lineName) {
        alert('Please fill in all required fields in Basic Information.');
        return false;
      }
      try {
        const currentIdx = document.getElementById('resourceId').value || null;
        const url = `proc/line.php?for=check-duplicate&factory_idx=${factory}&line_name=${encodeURIComponent(lineName)}${currentIdx ? `&current_idx=${currentIdx}` : ''}`;
        const result = await fetch(url).then(r => r.json());
        if (!result.success) { alert(result.message); return false; }
      } catch {
        alert('An error occurred while checking for duplicates.');
        return false;
      }
    }
    if (step === 2) {
      const mp     = document.getElementById('mp').value;
      const target = document.getElementById('line_target').value;
      if (!mp || mp < 0 || !target || target < 0) {
        alert('Please enter valid values for Man Power and Line Target.');
        return false;
      }
    }
    return true;
  }

  function updatePreview() {
    const factorySelect = document.getElementById('factory_idx');
    const factoryName   = factorySelect?.selectedOptions[0]?.textContent || '-';
    const lineName      = document.getElementById('line_name')?.value || '-';
    const mp            = document.getElementById('mp')?.value || '-';
    const target        = document.getElementById('line_target')?.value || '-';
    const statusText    = document.getElementById('status')?.selectedOptions[0]?.textContent || '-';

    const pName   = document.getElementById('previewName');
    const pLine   = document.getElementById('previewLine');
    const pMP     = document.getElementById('previewMP');
    const pTarget = document.getElementById('previewTarget');
    const pStatus = document.getElementById('previewStatus');

    if (pName)   pName.textContent   = factoryName;
    if (pLine)   pLine.textContent   = lineName;
    if (pMP)     pMP.textContent     = mp;
    if (pTarget) pTarget.textContent = target;
    if (pStatus) pStatus.textContent = statusText;
  }

  document.getElementById('factory_idx')?.addEventListener('change', updatePreview);
  document.getElementById('line_name')?.addEventListener('input', updatePreview);
  document.getElementById('mp')?.addEventListener('input', updatePreview);
  document.getElementById('line_target')?.addEventListener('input', updatePreview);
  document.getElementById('status')?.addEventListener('change', updatePreview);

  document.getElementById('addBtn')?.addEventListener('click', () => {
    currentStep = 1;
    showStep(1);
  });
}


async function loadFactoryOptions() {
  try {
    const response = await fetch('proc/line.php?for=factories');
    const result   = await response.json();

    if (result.success && result.data) {
      const filterSelect = document.getElementById('factoryFilterSelect');
      const modalSelect  = document.getElementById('factory_idx');

      if (filterSelect) {
        filterSelect.innerHTML = '<option value="">All Factories</option>';
        result.data.forEach(f => {
          filterSelect.innerHTML += `<option value="${f.idx}">${f.factory_name}</option>`;
        });
      }

      if (modalSelect) {
        modalSelect.innerHTML = '<option value="">Select Factory</option>';
        result.data.forEach(f => {
          modalSelect.innerHTML += `<option value="${f.idx}">${f.factory_name}</option>`;
        });
      }
    }
  } catch (error) {
    console.error('Factory options load error:', error);
  }
}
