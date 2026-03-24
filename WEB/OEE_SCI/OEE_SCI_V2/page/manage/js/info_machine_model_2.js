
export function initAdvancedFeatures(resourceManager) {
  initRealTimeSearch(resourceManager);
  initQuickActions();
  initModalSteps();
}

export const machineModelConfig = {
  resourceName: 'Machine_Model',
  apiEndpoint: 'proc/machine_model.php',
  entityId: 'idx',

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
      render: (item) => `
        <div style="display: flex; align-items: center; gap: var(--sap-spacing-xs);">
          <strong style="color: var(--sap-text-primary);">${item.machine_model_name}</strong>
        </div>
      `
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
      render: (item) => {
        if (item.type === 'P') return '<span style="color: var(--sap-brand-primary); font-weight: 600;">Computer Sewing Machine</span>';
        if (item.type === 'E') return '<span style="color: var(--sap-status-warning); font-weight: 600;">Embroidery Machine</span>';
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
  const prevBtn       = document.getElementById('prevBtn');
  const submitBtn     = document.getElementById('submitBtn');
  const stepIndicator = document.getElementById('stepIndicator');

  if (!nextBtn || !submitBtn || !stepIndicator) return;

  let currentStep = 1;
  const totalSteps = 2;

  nextBtn.addEventListener('click', async () => {
    if (await validateCurrentStep(currentStep)) {
      currentStep++;
      showStep(currentStep);
    }
  });

  if (prevBtn) {
    prevBtn.addEventListener('click', () => {
      currentStep--;
      showStep(currentStep);
    });
  }

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
    const prev = document.getElementById('prevStep');
    if (prev) prev.style.display = step > 1 ? 'inline-block' : 'none';
    nextBtn.style.display   = step < totalSteps  ? 'inline-block' : 'none';
    submitBtn.style.display = step === totalSteps ? 'inline-block' : 'none';
    stepIndicator.textContent = `Step ${step} of ${totalSteps}`;
    document.querySelectorAll('.form-step').forEach(s => {
      s.classList.toggle('active', parseInt(s.dataset.step) <= step);
    });
    updatePreview();
  }

  async function validateCurrentStep(step) {
    if (step === 1) {
      const name = document.getElementById('machine_model_name').value.trim();
      if (!name) { alert('Please enter the Machine Model name.'); return false; }
      try {
        const currentIdx = document.getElementById('resourceId').value || null;
        const url = `proc/machine_model.php?for=check-duplicate&machine_model_name=${encodeURIComponent(name)}${currentIdx ? `&current_idx=${currentIdx}` : ''}`;
        const result = await fetch(url).then(r => r.json());
        if (!result.success) { alert(result.message); return false; }
      } catch {
        alert('An error occurred while checking for duplicates.');
        return false;
      }
    }
    return true;
  }

  function updatePreview() {
    const name   = document.getElementById('machine_model_name')?.value || '-';
    const status = document.getElementById('status')?.selectedOptions[0]?.textContent || '-';
    const pName  = document.getElementById('previewName');
    const pStat  = document.getElementById('previewStatus');
    if (pName) pName.textContent = name;
    if (pStat) pStat.textContent = status;
  }

  document.getElementById('machine_model_name')?.addEventListener('input', updatePreview);
  document.getElementById('status')?.addEventListener('change', updatePreview);

  document.getElementById('addBtn')?.addEventListener('click', () => {
    currentStep = 1;
    showStep(1);
  });
}
