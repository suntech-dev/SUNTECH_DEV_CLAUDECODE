/* info_andon_2.js — Andon Management (stats 제거, fullscreen 리디자인) */

// Spectrum color palette
const COLOR_PALETTE = [
    ["#0070f2", "#1e88e5", "#00d4aa", "#0093c7"],
    ["#30914c", "#da1e28", "#e26b0a", "#8e44ad"],
    ["#32363b", "#4a5568", "#6b7884", "#8b95a1"],
    ["#2563eb", "#059669", "#dc2626", "#ffa500"]
];

export const andonConfig = {
    resourceName: 'Andon',
    apiEndpoint: 'proc/andon.php',
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
            label: 'IDX',
            sortable: true,
            sortKey: 'idx',
            visible: false
        },
        {
            key: 'andon_name',
            label: 'Andon Name',
            sortable: true,
            sortKey: 'andon_name',
            render: (item) => `
                <div style="display: flex; align-items: center; gap: var(--sap-spacing-xs);">
                    <strong style="color: var(--sap-text-primary);">${item.andon_name}</strong>
                </div>
            `
        },
        {
            key: 'total_count',
            label: 'Total',
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
            label: 'Completed',
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
            label: 'Warning',
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
            label: 'Warning %',
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
            key: 'color',
            label: 'Color',
            sortable: true,
            sortKey: 'color',
            render: (item) => {
                const color = item.color;
                if (!color) {
                    return `<div style="width: 50%; height: 20px; border: 1px solid var(--sap-border-neutral); border-radius: var(--sap-radius-sm); background: var(--sap-surface-2); display: flex; align-items: center; justify-content: center; font-size: var(--sap-font-size-xs); color: var(--sap-text-secondary);">N/A</div>`;
                }
                return `<div style="width: 50%; height: 20px; border: 1px solid var(--sap-border-neutral); border-radius: var(--sap-radius-sm); background: ${color}; box-shadow: 0 1px 2px rgba(0,0,0,0.1);" title="${color}"></div>`;
            },
            width: '80px'
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


export function initAdvancedFeatures(resourceManager) {
    initColorPicker();
    initRealTimeSearch(resourceManager);
    initQuickActions();
    initModalSteps();
}


// ─── Color Picker ────────────────────────────────────────

function initColorPicker() {
    const colorInput = $('#color');
    const colorPreviewBox = $('#colorPreviewBox');

    if (!colorInput.length) return;

    colorInput.spectrum({
        type: 'component',
        showInput: true,
        showInitial: true,
        allowEmpty: true,
        showAlpha: false,
        showPalette: true,
        showPaletteOnly: false,
        showSelectionPalette: true,
        hideAfterPaletteSelect: true,
        clickoutFiresChange: true,
        color: '#0070f2',
        palette: COLOR_PALETTE,
        localStorageKey: 'spectrum.andon.color',
        maxSelectionSize: 10,
        preferredFormat: 'hex',

        move: function(color) {
            if (color) colorPreviewBox.css('background-color', color.toHexString());
        },

        change: function(color) {
            if (color) {
                colorPreviewBox.css('background-color', color.toHexString());
                document.getElementById('color').dispatchEvent(new Event('input', { bubbles: true }));
            } else {
                colorPreviewBox.css('background-color', '#ffffff');
            }
        }
    });

    colorPreviewBox.css('background-color', colorInput.val() || '#0070f2');
}

// 외부(resource-manager)에서 폼 데이터 로드 시 spectrum 업데이트
window.updateAndonFormSpectrum = function(data) {
    if (data && data.color) {
        const colorInput = $('#color');
        if (colorInput.length && typeof colorInput.spectrum === 'function') {
            colorInput.spectrum('set', data.color);
            $('#colorPreviewBox').css('background-color', data.color);
        }
    }
};


// ─── Real-time Search ────────────────────────────────────

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


// ─── Quick Actions ───────────────────────────────────────

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


// ─── Modal Steps ─────────────────────────────────────────

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
            const andonName = document.getElementById('andon_name').value.trim();
            if (!andonName) { alert('Please enter the andon name.'); return false; }
            try {
                const currentIdx = document.getElementById('resourceId').value || null;
                const url = `proc/andon.php?for=check-duplicate&andon_name=${encodeURIComponent(andonName)}${currentIdx ? `&current_idx=${currentIdx}` : ''}`;
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
        const name   = document.getElementById('andon_name')?.value || '-';
        const color  = document.getElementById('color')?.value || '-';
        const status = document.getElementById('status')?.selectedOptions[0]?.textContent || '-';
        const pName  = document.getElementById('previewName');
        const pColor = document.getElementById('previewColor');
        const pStat  = document.getElementById('previewStatus');
        if (pName)  pName.textContent  = name;
        if (pColor) pColor.textContent = color;
        if (pStat)  pStat.textContent  = status;
    }

    document.getElementById('andon_name')?.addEventListener('input', updatePreview);
    document.getElementById('color')?.addEventListener('input', updatePreview);
    document.getElementById('status')?.addEventListener('change', updatePreview);

    document.getElementById('addBtn')?.addEventListener('click', () => {
        currentStep = 1;
        showStep(1);
        // spectrum 초기화
        const colorInput = $('#color');
        if (colorInput.length && typeof colorInput.spectrum === 'function') {
            colorInput.spectrum('set', '#0070f2');
            $('#colorPreviewBox').css('background-color', '#0070f2');
        }
    });
}
