
export function initAdvancedFeatures(resourceManager) {
    // typeFilterSelect 기본값 'P' 강제 적용
    const typeFilterSelect = document.getElementById('typeFilterSelect');
    if (typeFilterSelect && typeFilterSelect.value === 'P') {
        if (resourceManager && resourceManager.state) {
            resourceManager.state.typeFilter = 'P';
            setTimeout(() => {
                if (resourceManager.loadData) resourceManager.loadData();
            }, 100);
        }
    }

    initRealTimeSearch(resourceManager);
    initQuickActions();
    initModalSteps();
}

export const machineConfig = {
    resourceName: 'Machine',
    apiEndpoint: 'proc/machine.php',
    entityId: 'idx',

    async beforeInit(api) {
        await loadFactoryOptions(api);
        await loadModelOptions(api);
        await loadLineOptions(api);
        await loadMachineOptions(api);
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
            render: (item) => item.design_process
                ? `<span style="color: var(--sap-text-primary); font-weight: 500;">${item.design_process}</span>`
                : '<span style="color: var(--sap-text-secondary);">Not assigned</span>'
        },
        {
            key: 'mac',
            label: 'MAC Address',
            sortable: true,
            sortKey: 'm.mac',
            render: (item) => item.mac
                ? `<code style="font-family: monospace; background: var(--sap-surface-3); padding: 2px 6px; border-radius: 4px;">${item.mac}</code>`
                : '<span style="color: var(--sap-text-secondary);">Not set</span>'
        },
        {
            key: 'type',
            label: 'Type',
            sortable: true,
            sortKey: 'm.type',
            render: (item) => {
                if (item.type === 'P') return '<span style="color: var(--sap-brand-primary); font-weight: 600;">Computer Sewing Machine</span>';
                if (item.type === 'E') return '<span style="color: var(--sap-status-warning); font-weight: 600;">Embroidery Machine</span>';
                return '<span style="color: var(--sap-text-secondary);">Unknown</span>';
            }
        },
        {
            key: 'target',
            label: 'Target',
            sortable: true,
            sortKey: 'm.target',
            render: (item) => (parseInt(item.target) || 0).toLocaleString()
        },
        {
            key: 'status',
            label: 'Status',
            sortable: true,
            sortKey: 'm.status',
            render: (item) => {
                const isActive = item.status === 'Y';
                return `<span class="fiori-badge fiori-badge--${isActive ? 'success' : 'warning'}">${isActive ? 'Used' : 'Unused'}</span>`;
            },
            width: '90px'
        },
        {
            key: 'app_ver',
            label: 'APP VERSION',
            sortable: true,
            sortKey: 'm.app_ver'
        }
    ],

    filterConfig: [
        { elementId: 'statusFilterSelect',           paramName: 'status_filter',  stateKey: 'statusFilter' },
        { elementId: 'typeFilterSelect',             paramName: 'type_filter',    stateKey: 'typeFilter' },
        { elementId: 'factoryFilterSelect',          paramName: 'factory_filter', stateKey: 'factoryFilter', resets: ['lineFilter', 'machineFilter'] },
        { elementId: 'factoryLineFilterSelect',      paramName: 'line_filter',    stateKey: 'lineFilter',    resets: ['machineFilter'] },
        { elementId: 'factoryLineMachineFilterSelect', paramName: 'machine_filter', stateKey: 'machineFilter' }
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
    }

    async function validateCurrentStep(step) {
        if (step === 1) {
            const factorySelect = document.getElementById('factory_idx');
            const lineSelect    = document.getElementById('line_idx');
            const machineNo     = document.getElementById('machine_no');

            if (factorySelect && !factorySelect.value) {
                alert('Please select a factory.');
                factorySelect.focus();
                return false;
            }
            if (lineSelect && !lineSelect.value) {
                alert('Please select a line.');
                lineSelect.focus();
                return false;
            }
            if (machineNo && !machineNo.value.trim()) {
                alert('Please enter the machine name.');
                machineNo.focus();
                return false;
            }
            try {
                const currentIdx = document.getElementById('resourceId').value || null;
                const url = `proc/machine.php?for=check-duplicate&machine_no=${encodeURIComponent(machineNo.value.trim())}${currentIdx ? `&current_idx=${currentIdx}` : ''}`;
                const result = await fetch(url).then(r => r.json());
                if (!result.success) { alert(result.message); return false; }
            } catch {
                alert('An error occurred while checking for duplicates.');
                return false;
            }
        }
        if (step === 2) {
            const typeSelect  = document.getElementById('type');
            const modelSelect = document.getElementById('machine_model_idx');

            if (typeSelect && !typeSelect.value) {
                alert('Please select the machine type.');
                typeSelect.focus();
                return false;
            }
            if (modelSelect && !modelSelect.value) {
                alert('Please select the machine model.');
                modelSelect.focus();
                return false;
            }
        }
        if (step === 3) {
            const target = document.getElementById('target');
            if (target && target.value && isNaN(parseInt(target.value))) {
                alert('Please enter a number for target quantity.');
                target.focus();
                return false;
            }
        }
        return true;
    }
}


async function loadFactoryOptions(api) {
    try {
        const response = await api.getAll({ for: 'factories' });
        if (!response.success || !response.data) return;

        const factorySelect      = document.getElementById('factoryFilterSelect');
        const modalFactorySelect = document.getElementById('factory_idx');

        if (factorySelect) {
            factorySelect.innerHTML = '<option value="">All Factories</option>';
            response.data.forEach(f => {
                factorySelect.innerHTML += `<option value="${f.idx}">${f.factory_name}</option>`;
            });
            factorySelect.addEventListener('change', (e) => {
                updateLineOptions(api, e.target.value, 'factoryLineFilterSelect', 'All Lines');
            });
        }

        if (modalFactorySelect) {
            modalFactorySelect.innerHTML = '<option value="">Select Factory</option>';
            response.data.forEach(f => {
                modalFactorySelect.innerHTML += `<option value="${f.idx}">${f.factory_name}</option>`;
            });
            modalFactorySelect.addEventListener('change', (e) => {
                const lineSelect = document.getElementById('line_idx');
                if (lineSelect) {
                    lineSelect.value    = '';
                    lineSelect.disabled = !e.target.value;
                }
                updateLineOptions(api, e.target.value, 'line_idx', 'Please select a line');
            });
        }

        const lineFilterSelect = document.getElementById('factoryLineFilterSelect');
        if (lineFilterSelect) {
            lineFilterSelect.addEventListener('change', (e) => {
                const factoryVal = factorySelect ? factorySelect.value : '';
                updateMachineOptions(api, factoryVal, e.target.value, 'factoryLineMachineFilterSelect', 'All Machines');
            });
        }
    } catch (e) {
        console.error('Factory options load error:', e);
    }
}

async function loadModelOptions(api) {
    try {
        const response = await api.getAll({ for: 'models' });
        if (!response.success || !response.data) return;

        const modelSelect = document.getElementById('machine_model_idx');
        if (modelSelect) {
            modelSelect.innerHTML = '<option value="">Select Model</option>';
            response.data.forEach(m => {
                modelSelect.innerHTML += `<option value="${m.idx}">${m.machine_model_name}</option>`;
            });
        }
    } catch (e) {
        console.error('Model options load error:', e);
    }
}

async function loadLineOptions(api) {
    try {
        const response = await api.getAll({ for: 'lines' });
        if (!response.success || !response.data) return;

        const lineFilterSelect = document.getElementById('factoryLineFilterSelect');
        if (lineFilterSelect) {
            lineFilterSelect.innerHTML = '<option value="">All Lines</option>';
            response.data.forEach(l => {
                lineFilterSelect.innerHTML += `<option value="${l.idx}">${l.line_name}</option>`;
            });
        }
    } catch (e) {
        console.error('Line options load error:', e);
    }
}

async function loadMachineOptions(api) {
    try {
        const response = await api.getAll({});
        if (!response.success || !response.data) return;

        const machineFilterSelect = document.getElementById('factoryLineMachineFilterSelect');
        if (machineFilterSelect) {
            machineFilterSelect.innerHTML = '<option value="">All Machines</option>';
            response.data.forEach(m => {
                machineFilterSelect.innerHTML += `<option value="${m.idx}">${m.machine_no} (${m.machine_model_name || 'No Model'})</option>`;
            });
        }
    } catch (e) {
        console.error('Machine options load error:', e);
    }
}

async function updateLineOptions(api, factoryId, lineElementId, initialText, preserveValue = null) {
    const lineSelect = document.getElementById(lineElementId);
    if (!lineSelect) return;

    lineSelect.disabled = true;
    try {
        let url = 'proc/machine.php?for=lines';
        if (factoryId) url += `&factory_id=${factoryId}`;

        const res = await fetch(url).then(r => r.json());
        const currentValue = preserveValue || lineSelect.value;

        lineSelect.innerHTML = `<option value="">${initialText}</option>`;
        if (res.success && res.data) {
            res.data.forEach(l => {
                lineSelect.innerHTML += `<option value="${l.idx}">${l.line_name}</option>`;
            });
            if (currentValue && lineSelect.querySelector(`option[value="${currentValue}"]`)) {
                lineSelect.value = currentValue;
                setTimeout(() => lineSelect.dispatchEvent(new Event('change', { bubbles: true })), 50);
            }
        }
    } catch (e) {
        console.error('updateLineOptions error:', e);
        lineSelect.innerHTML = `<option value="">${initialText}</option>`;
    }
    lineSelect.disabled = false;
}

async function updateMachineOptions(api, factoryId, lineId, machineElementId, initialText) {
    const machineSelect = document.getElementById(machineElementId);
    if (!machineSelect) return;

    machineSelect.disabled = true;
    try {
        const params = {};
        if (factoryId) params.factory_filter = factoryId;
        if (lineId)    params.line_filter    = lineId;

        const res = await api.getAll(params);
        machineSelect.innerHTML = `<option value="">${initialText}</option>`;
        if (res.success && res.data) {
            res.data.forEach(m => {
                machineSelect.innerHTML += `<option value="${m.idx}">${m.machine_no} (${m.machine_model_name || 'No Model'})</option>`;
            });
        }
    } catch (e) {
        console.error('updateMachineOptions error:', e);
        machineSelect.innerHTML = `<option value="">${initialText}</option>`;
    }
    machineSelect.disabled = false;
}
