
export function initAdvancedFeatures(resourceManager) {
  initRealTimeSearch(resourceManager);
  initQuickActions();
  initModalSteps();
}

export const downtimeConfig = {
  resourceName: 'Downtime',
  apiEndpoint: 'proc/downtime.php',
  entityId: 'idx',

  columnConfig: [
    {
      key: 'no',
      label: 'NO.',
      sortable: false,
      render: (item, index) => `<span style="color: var(--sap-text-secondary);">${index}</span>`,
      width: '60px'
    },
    { key: 'idx', label: 'IDX', sortable: true, sortKey: 'idx', visible: false },
    {
      key: 'downtime_name',
      label: 'Downtime Name',
      sortable: true,
      sortKey: 'downtime_name',
      render: (item) => `<strong style="color: var(--sap-text-primary);">${item.downtime_name}</strong>`
    },
    {
      key: 'downtime_shortcut',
      label: 'Shortcut',
      sortable: true,
      sortKey: 'downtime_shortcut',
      render: (item) => {
        if (item.downtime_shortcut && item.downtime_shortcut.trim()) {
          return `<code style="background: var(--sap-surface-2); padding: 2px 6px; border-radius: var(--sap-radius-sm); font-size: var(--sap-font-size-sm);">${item.downtime_shortcut.trim()}</code>`;
        }
        return `<span style="color: var(--sap-text-secondary);">-</span>`;
      }
    },
    {
      key: 'status',
      label: 'Status',
      sortable: true,
      sortKey: 'status',
      render: (item) => {
        const isActive = item.status === 'Y';
        return `<span class="fiori-badge fiori-badge--${isActive ? 'success' : 'warning'}">${isActive ? 'Used' : 'Unused'}</span>`;
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
  const prevBtn       = document.getElementById('prevStep');
  const submitBtn     = document.getElementById('submitBtn');
  const stepIndicator = document.getElementById('stepIndicator');

  if (!nextBtn || !prevBtn || !submitBtn || !stepIndicator) return;

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
    prevBtn.style.display   = step > 1           ? 'inline-block' : 'none';
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
      const downtimeName = document.getElementById('downtime_name').value.trim();
      if (!downtimeName) { alert('Please enter the downtime name.'); return false; }

      try {
        const currentIdx = document.getElementById('resourceId').value || null;
        const url = `proc/downtime.php?for=check-duplicate&downtime_name=${encodeURIComponent(downtimeName)}${currentIdx ? `&current_idx=${currentIdx}` : ''}`;
        const result = await fetch(url).then(r => r.json());
        if (!result.success) { alert(result.message); return false; }
      } catch {
        alert('An error occurred while checking for duplicates.');
        return false;
      }

      const downtimeShortcut = document.getElementById('downtime_shortcut').value.trim();
      if (downtimeShortcut) {
        try {
          const currentIdx = document.getElementById('resourceId').value || null;
          const url = `proc/downtime.php?for=check-duplicate-shortcut&downtime_shortcut=${encodeURIComponent(downtimeShortcut)}${currentIdx ? `&current_idx=${currentIdx}` : ''}`;
          const result = await fetch(url).then(r => r.json());
          if (!result.success) { alert(result.message); return false; }
        } catch {
          alert('An error occurred while checking for shortcut duplicates.');
          return false;
        }
      }
    }
    return true;
  }

  function updatePreview() {
    const name     = document.getElementById('downtime_name')?.value || '-';
    const shortcut = document.getElementById('downtime_shortcut')?.value || '-';
    const status   = document.getElementById('status')?.selectedOptions[0]?.textContent || '-';
    const pName    = document.getElementById('previewName');
    const pShort   = document.getElementById('previewShortcut');
    const pStat    = document.getElementById('previewStatus');
    if (pName)  pName.textContent  = name;
    if (pShort) pShort.textContent = shortcut;
    if (pStat)  pStat.textContent  = status;
  }

  document.getElementById('downtime_name')?.addEventListener('input', updatePreview);
  document.getElementById('downtime_shortcut')?.addEventListener('input', updatePreview);
  document.getElementById('status')?.addEventListener('change', updatePreview);

  document.getElementById('addBtn')?.addEventListener('click', () => {
    currentStep = 1;
    showStep(1);
  });
}
