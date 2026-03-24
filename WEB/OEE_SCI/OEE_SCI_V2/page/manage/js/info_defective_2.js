
export function initAdvancedFeatures(resourceManager) {
    initRealTimeSearch(resourceManager);
    initQuickActions();
    initModalSteps();
}

export const defectiveConfig = {
    resourceName: 'Defective',
    apiEndpoint: 'proc/defective.php',
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
            key: 'defective_name',
            label: 'Defective Name',
            sortable: true,
            sortKey: 'defective_name',
            render: (item) => `<span class="defective-name-badge">${item.defective_name}</span>`
        },
        {
            key: 'defective_shortcut',
            label: 'Shortcut',
            sortable: true,
            sortKey: 'defective_shortcut',
            render: (item) => {
                if (item.defective_shortcut && item.defective_shortcut.trim()) {
                    const s = item.defective_shortcut.trim();
                    return `<code class="shortcut-badge${s.length > 10 ? ' long' : ''}">${s}</code>`;
                }
                return '<span style="color: var(--sap-text-secondary);">No shortcut</span>';
            }
        },
        {
            key: 'total_count',
            label: 'Total Count',
            sortable: true,
            sortKey: 'total_count',
            render: (item) => `<span style="color: var(--sap-text-primary); font-weight: 500;">${item.total_count || 0}</span>`,
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
                    rate > 15 ? 'var(--sap-status-warning)' : 'var(--sap-status-success)';
                return `<span style="color: ${color}; font-weight: 500;">${rate}%</span>`;
            },
            width: '80px'
        },
        {
            key: 'status',
            label: 'Status',
            sortable: true,
            sortKey: 'status',
            render: (item) => item.status === 'Y' ?
                '<span class="defective-status-used">Used</span>' :
                '<span class="defective-status-unused">Unused</span>',
            width: '100px'
        },
        {
            key: 'remark',
            label: 'Remark',
            sortable: true,
            sortKey: 'remark',
            render: (item) => item.remark ||
                '<span style="color: var(--sap-text-secondary);">No remarks</span>'
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
            const name = document.getElementById('defective_name').value.trim();
            if (!name) { alert('Please enter the defective name.'); return false; }
            try {
                const idx = document.getElementById('resourceId').value || null;
                const url = `proc/defective.php?for=check-duplicate&defective_name=${encodeURIComponent(name)}${idx ? `&current_idx=${idx}` : ''}`;
                const result = await fetch(url).then(r => r.json());
                if (!result.success) { alert(result.message); return false; }
            } catch {
                alert('An error occurred while checking for duplicates.');
                return false;
            }
            const shortcut = document.getElementById('defective_shortcut').value.trim();
            if (shortcut) {
                try {
                    const idx = document.getElementById('resourceId').value || null;
                    const url = `proc/defective.php?for=check-duplicate-shortcut&defective_shortcut=${encodeURIComponent(shortcut)}${idx ? `&current_idx=${idx}` : ''}`;
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
        const name     = document.getElementById('defective_name')?.value || '-';
        const shortcut = document.getElementById('defective_shortcut')?.value || '-';
        const status   = document.getElementById('status')?.selectedOptions[0]?.textContent || '-';
        const pName    = document.getElementById('previewName');
        const pShort   = document.getElementById('previewShortcut');
        const pStatus  = document.getElementById('previewStatus');
        if (pName)   pName.textContent   = name;
        if (pShort)  pShort.textContent  = shortcut;
        if (pStatus) pStatus.textContent = status;
    }

    document.getElementById('defective_name')?.addEventListener('input', updatePreview);
    document.getElementById('defective_shortcut')?.addEventListener('input', updatePreview);
    document.getElementById('status')?.addEventListener('change', updatePreview);

    document.getElementById('addBtn')?.addEventListener('click', () => {
        currentStep = 1;
        showStep(1);
    });
}
