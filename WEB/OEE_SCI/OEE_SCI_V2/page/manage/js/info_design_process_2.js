
export function initAdvancedFeatures(resourceManager) {
    initRealTimeSearch(resourceManager);
    initQuickActions();
    initModalSteps();
    initFilePreviewIcons();
    initDeleteButtons(resourceManager);
}

export const designProcessConfig = {
    resourceName: 'Design Process',
    apiEndpoint: '../manage/proc/design_process.php',
    entityId: 'idx',

    columnConfig: [
        { key: 'no', label: 'NO.', sortable: false, render: (item, index) => `<span style="color: var(--sap-text-secondary);">${index + 1}</span>` },
        { key: 'idx', label: 'IDX', sortable: true, sortKey: 'dp.idx', visible: false },
        {
            key: 'factory_name',
            label: 'Factory',
            sortable: true,
            sortKey: 'f.factory_name',
            render: (item) => item.factory_name
                ? `<span style="color: var(--sap-text-primary);">${item.factory_name}</span>`
                : '<span style="color: var(--sap-text-secondary);">N/A</span>'
        },
        {
            key: 'line_name',
            label: 'Line',
            sortable: true,
            sortKey: 'l.line_name',
            render: (item) => item.line_name
                ? `<span style="color: var(--sap-text-primary);">${item.line_name}</span>`
                : '<span style="color: var(--sap-text-secondary);">N/A</span>'
        },
        {
            key: 'model_name',
            label: 'Model Name',
            sortable: true,
            sortKey: 'dp.model_name',
            render: (item) => item.model_name
                ? `<span style="color: var(--sap-text-primary);">${item.model_name}</span>`
                : '<span style="color: var(--sap-text-secondary);">N/A</span>'
        },
        {
            key: 'design_process',
            label: 'Design Process',
            sortable: true,
            sortKey: 'dp.design_process',
            render: (item) => `<span class="process-name-badge">${item.design_process}</span>`
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
            label: 'SOP',
            sortable: true,
            sortKey: 'dp.fname',
            render: (item) => {
                if (item.fname) {
                    return `
                        <span class="process-file-badge">
                            <span class="file-preview-icon" data-filename="${item.fname}" title="Click to preview" style="cursor: pointer; color: var(--sap-brand-primary);">&#128196;</span>
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
            render: (item) => item.status === 'Y'
                ? '<span class="fiori-badge fiori-badge--success">Used</span>'
                : '<span class="fiori-badge fiori-badge--warning">Unused</span>'
        },
        {
            key: 'remark',
            label: 'Remark',
            sortable: true,
            sortKey: 'dp.remark',
            render: (item) => item.remark || '<span style="color: var(--sap-text-secondary);">-</span>'
        },
        {
            key: 'actions',
            label: 'Delete',
            sortable: false,
            render: (item) => `
                <button
                    class="fiori-btn fiori-btn--tertiary fiori-btn--sm delete-btn"
                    data-idx="${item.idx}"
                    data-name="${item.design_process}"
                    title="Delete"
                    style="min-width: auto; padding: 4px 8px;"
                >&#128465;</button>
            `
        }
    ],

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

    onTableRender: (data) => {
        const tableRows = document.querySelectorAll('#tableBody tr');
        tableRows.forEach(row => row.classList.add('process-table-row'));
    },

    async beforeInit(api) {
        await loadFactoryOptions(api);
    },

    async beforeEdit(api, data) {
        const factorySelect = document.getElementById('factory_idx');
        if (factorySelect && data.factory_idx) {
            factorySelect.value = data.factory_idx;
            const lineSelect = document.getElementById('line_idx');
            if (lineSelect) {
                lineSelect.disabled = false;
                await updateLineOptions(api, data.factory_idx, 'line_idx', 'Please select a line', data.line_idx);
            }
        }

        const modelNameInput = document.getElementById('model_name');
        if (modelNameInput) modelNameInput.value = data.model_name || '';

        setTimeout(() => {
            const fileInput = document.getElementById('file_upload');
            if (fileInput) fileInput.value = '';
            if (window.updateExistingFileInfo) window.updateExistingFileInfo(data.fname);
        }, 100);
    }
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
        searchTimeout = setTimeout(() => {
            if (resourceManager && resourceManager.state) {
                resourceManager.state.searchQuery = value;
                resourceManager.state.currentPage = 1;
            }
            if (resourceManager && resourceManager.loadData) resourceManager.loadData();
        }, 300);
    });

    searchClear.addEventListener('click', () => {
        searchInput.value = '';
        searchClear.classList.remove('visible');
        if (resourceManager && resourceManager.state) {
            resourceManager.state.searchQuery = '';
            resourceManager.state.currentPage = 1;
        }
        if (resourceManager && resourceManager.loadData) resourceManager.loadData();
        searchInput.focus();
    });
}

export function initQuickActions() {
    const quickFilters = document.querySelectorAll('.quick-action-btn');
    quickFilters.forEach(filter => {
        filter.addEventListener('click', () => {
            quickFilters.forEach(f => f.classList.remove('active'));
            filter.classList.add('active');
            const filterType = filter.dataset.filter;
            const tableRows = document.querySelectorAll('#tableBody tr');
            tableRows.forEach(row => {
                let show = true;
                if (filterType === 'used')   show = row.textContent.includes('Used') && !row.textContent.includes('Unused');
                if (filterType === 'unused') show = row.textContent.includes('Unused');
                row.style.display = show ? '' : 'none';
            });
        });
    });
}

export function initFilePreviewIcons() {
    const tableBody = document.getElementById('tableBody');
    if (!tableBody) return;

    tableBody.addEventListener('click', (e) => {
        const previewIcon = e.target.closest('.file-preview-icon');
        if (!previewIcon) return;
        e.stopPropagation();
        e.preventDefault();
        const filename = previewIcon.dataset.filename;
        if (filename) showImagePreviewModal(filename);
    });
}

export function showImagePreviewModal(filename) {
    const imagePreviewModal = document.getElementById('imagePreviewModal');
    const previewImage = document.getElementById('previewImage');
    const imageFileName = document.getElementById('imageFileName');
    if (!imagePreviewModal || !previewImage || !imageFileName) return;

    previewImage.src = `../../upload/sop/${filename}`;
    imageFileName.textContent = filename;
    imagePreviewModal.style.display = 'flex';
    setTimeout(() => imagePreviewModal.classList.add('show'), 10);

    previewImage.onerror = function () {
        alert('Image could not be loaded.');
        imagePreviewModal.style.display = 'none';
    };
}

export function initModalSteps() {
    const nextBtn      = document.getElementById('nextStep');
    const prevBtn      = document.getElementById('prevStep');
    const submitBtn    = document.getElementById('submitBtn');
    const stepIndicator = document.getElementById('stepIndicator');
    if (!nextBtn || !prevBtn || !submitBtn || !stepIndicator) return;

    let currentStep = 1;
    const totalSteps = 2;

    nextBtn.addEventListener('click', async () => {
        if (await validateStep(currentStep)) {
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
        step.addEventListener('click', async () => {
            const target = index + 1;
            if (target < currentStep) {
                currentStep = target;
                showStep(currentStep);
            } else if (target > currentStep && await validateStep(currentStep)) {
                currentStep = target;
                showStep(currentStep);
            }
        });
    });

    function showStep(step) {
        document.querySelectorAll('.form-section').forEach(s => {
            s.style.display = s.dataset.section == step ? 'block' : 'none';
        });
        prevBtn.style.display   = step > 1 ? 'inline-block' : 'none';
        nextBtn.style.display   = step < totalSteps ? 'inline-block' : 'none';
        submitBtn.style.display = step === totalSteps ? 'inline-block' : 'none';
        stepIndicator.textContent = `Step ${step} of ${totalSteps}`;
        document.querySelectorAll('.form-step').forEach(s => {
            s.classList.toggle('active', parseInt(s.dataset.step) <= step);
        });
        updatePreview();
    }

    async function validateStep(step) {
        if (step !== 1) return true;

        const factorySelect = document.getElementById('factory_idx');
        const lineSelect    = document.getElementById('line_idx');
        const designProcess = document.getElementById('design_process').value.trim();

        if (!factorySelect || !factorySelect.value) {
            alert('Please select a factory.');
            if (factorySelect) factorySelect.focus();
            return false;
        }
        if (!lineSelect || !lineSelect.value) {
            alert('Please select a line.');
            if (lineSelect) lineSelect.focus();
            return false;
        }
        if (!designProcess) {
            alert('Please enter the design process name.');
            return false;
        }

        try {
            const currentIdx = document.getElementById('resourceId').value || null;
            const url = `../manage/proc/design_process.php?for=check-duplicate&design_process=${encodeURIComponent(designProcess)}&factory_idx=${factorySelect.value}&line_idx=${lineSelect.value}${currentIdx ? `&current_idx=${currentIdx}` : ''}`;
            const res = await fetch(url);
            const result = await res.json();
            if (!result.success) {
                alert(result.message);
                return false;
            }
        } catch (e) {
            alert('An error occurred while checking for duplicates.');
            return false;
        }
        return true;
    }

    function updatePreview() {
        const designProcess = document.getElementById('design_process').value || '-';
        const stdMcNeeded   = document.getElementById('std_mc_needed').value || '-';
        const statusText    = document.getElementById('status').selectedOptions[0]?.textContent || '-';
        const fileInput     = document.getElementById('file_upload');
        const fileName      = fileInput && fileInput.files.length > 0 ? fileInput.files[0].name : '-';

        const previewName   = document.getElementById('previewName');
        const previewStdMc  = document.getElementById('previewStdMc');
        const previewStatus = document.getElementById('previewStatus');
        const previewFile   = document.getElementById('previewFile');

        if (previewName)   previewName.textContent   = designProcess;
        if (previewStdMc)  previewStdMc.textContent  = stdMcNeeded;
        if (previewStatus) previewStatus.textContent = statusText;
        if (previewFile)   previewFile.textContent   = fileName;
    }

    ['design_process', 'std_mc_needed'].forEach(id => {
        const el = document.getElementById(id);
        if (el) el.addEventListener('input', updatePreview);
    });
    const statusSel  = document.getElementById('status');
    const fileInput  = document.getElementById('file_upload');
    if (statusSel) statusSel.addEventListener('change', updatePreview);
    if (fileInput)  fileInput.addEventListener('change', updatePreview);

    const addBtn = document.getElementById('addBtn');
    if (addBtn) {
        addBtn.addEventListener('click', () => {
            currentStep = 1;
            showStep(1);
            const factorySelect = document.getElementById('factory_idx');
            const lineSelect    = document.getElementById('line_idx');
            if (factorySelect) factorySelect.value = '';
            if (lineSelect) { lineSelect.value = ''; lineSelect.disabled = true; }
            if (window.resetExistingFileInfo) window.resetExistingFileInfo();
        });
    }

    const resourceModal = document.getElementById('resourceModal');
    if (resourceModal) {
        new MutationObserver((mutations) => {
            mutations.forEach((m) => {
                if (m.type === 'attributes' && m.attributeName === 'class' && resourceModal.classList.contains('show')) {
                    currentStep = 1;
                    showStep(1);
                }
            });
        }).observe(resourceModal, { attributes: true });
    }
}

export function initDeleteButtons(resourceManager) {
    const tableBody = document.getElementById('tableBody');
    if (!tableBody) return;

    tableBody.addEventListener('click', async (e) => {
        const deleteBtn = e.target.closest('.delete-btn');
        if (!deleteBtn) return;
        e.stopPropagation();
        e.preventDefault();

        const idx         = deleteBtn.dataset.idx;
        const processName = deleteBtn.dataset.name;

        if (!confirm(`Are you sure you want to delete this Design Process?\n\nProcess: ${processName}\n\nThis action is irreversible.`)) return;

        try {
            const response = await fetch(`../manage/proc/design_process.php?id=${idx}`, { method: 'DELETE' });
            const result   = await response.json();
            if (result.success) {
                alert('Design Process deleted successfully.');
                if (resourceManager && resourceManager.loadData) await resourceManager.loadData();
            } else {
                alert('Deletion failed: ' + result.message);
            }
        } catch (e) {
            alert('An error occurred while deleting.');
        }
    });
}

async function loadFactoryOptions(api) {
    try {
        const response = await api.getAll({ for: 'factories' });
        if (!response.success || !response.data) return;

        const factoryFilterSelect = document.getElementById('factoryFilterSelect');
        const modalFactorySelect  = document.getElementById('factory_idx');

        if (factoryFilterSelect) {
            factoryFilterSelect.innerHTML = '<option value="">All Factories</option>';
            response.data.forEach(f => {
                factoryFilterSelect.innerHTML += `<option value="${f.idx}">${f.factory_name}</option>`;
            });
            factoryFilterSelect.addEventListener('change', (e) => {
                updateLineOptions(api, e.target.value, 'factoryLineFilterSelect', 'All Lines');
            });
        }

        if (modalFactorySelect) {
            modalFactorySelect.innerHTML = '<option value="">Please select a factory</option>';
            response.data.forEach(f => {
                modalFactorySelect.innerHTML += `<option value="${f.idx}">${f.factory_name}</option>`;
            });
            modalFactorySelect.addEventListener('change', (e) => {
                const lineSelect = document.getElementById('line_idx');
                if (lineSelect) {
                    lineSelect.disabled = !e.target.value;
                    if (!e.target.value) lineSelect.value = '';
                }
                updateLineOptions(api, e.target.value, 'line_idx', 'Please select a line');
            });
        }

        await updateLineOptions(api, '', 'factoryLineFilterSelect', 'All Lines');
    } catch (e) {
        console.error('Failed to load factory options:', e);
    }
}

async function updateLineOptions(api, factoryId, lineElementId, initialText, preserveValue = null) {
    const lineSelect = document.getElementById(lineElementId);
    if (!lineSelect) return;

    lineSelect.disabled = true;
    try {
        let url = '../manage/proc/design_process.php?for=lines';
        if (factoryId) url += `&factory_id=${factoryId}`;

        const response = await fetch(url);
        const res      = await response.json();

        if (res.success) {
            const current = preserveValue || lineSelect.value;
            lineSelect.innerHTML = `<option value="">${initialText}</option>`;
            res.data.forEach(line => {
                lineSelect.innerHTML += `<option value="${line.idx}">${line.line_name}</option>`;
            });
            if (current && lineSelect.querySelector(`option[value="${current}"]`)) {
                lineSelect.value = current;
            }
        } else {
            lineSelect.innerHTML = `<option value="">${initialText}</option>`;
        }
    } catch (e) {
        lineSelect.innerHTML = `<option value="">${initialText}</option>`;
    }
    lineSelect.disabled = false;
}
