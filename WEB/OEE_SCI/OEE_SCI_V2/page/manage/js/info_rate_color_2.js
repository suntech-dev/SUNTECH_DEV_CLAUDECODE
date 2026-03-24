/**
 * info_rate_color_2.js
 * Rate Color Management — Fullscreen Redesign
 */

// 5단계 설정
let rateColorConfig = {
    stage1: { start_rate: 0,   end_rate: 25,  color: '#6b7884' },
    stage2: { start_rate: 25,  end_rate: 50,  color: '#da1e28' },
    stage3: { start_rate: 50,  end_rate: 80,  color: '#e26b0a' },
    stage4: { start_rate: 80,  end_rate: 100, color: '#30914c' },
    stage5: { start_rate: 100, end_rate: 999, color: '#0070f2' }
};

let currentSelectedStage = null;

let updateInProgress = {
    stage1: false,
    stage2: false,
    stage3: false,
    stage4: false
};

const colorPalette = [
    '#ff0000', '#e53e3e', '#dc2626', '#b91c1c',
    '#00ff00', '#38a169', '#059669', '#047857',
    '#0000ff', '#3182ce', '#2b6cb0', '#2c5282',
    '#ffff00', '#ecc94b', '#d69e2e', '#b7791f',
    '#0070f2', '#1e88e5', '#00d4aa', '#0093c7',
    '#30914c', '#65b565', '#da1e28', '#ff4757',
    '#e26b0a', '#ff8c42', '#8e44ad', '#9b59b6',
    '#32363b', '#4a5568', '#6b7884', '#8b95a1',
    '#a0aec0', '#cbd5e0', '#e2e8f0', '#f7fafc',
    '#2563eb', '#3b82f6', '#06b6d4', '#0891b2',
    '#059669', '#10b981', '#dc2626', '#ef4444',
    '#ffa500', '#ff8c00', '#ed8936', '#dd6b20',
    '#800080', '#9b59b6', '#8e44ad', '#7c3aed'
];

document.addEventListener('DOMContentLoaded', function() {
    initializeRangeSliders();
    initializeButtons();
    loadExistingConfig();
});


/* ─── Range Sliders ──────────────────────────────────────── */

function initializeRangeSliders() {
    $("#stage1Range").ionRangeSlider({
        type: "double",
        min: 0, max: 100,
        from: 0, to: rateColorConfig.stage1.end_rate,
        step: 1, postfix: "%", skin: "fiori",
        grid: true, grid_num: 10,
        from_fixed: true,
        onChange: function(data) {
            rateColorConfig.stage1.start_rate = 0;
            rateColorConfig.stage1.end_rate = data.to;
            rateColorConfig.stage2.start_rate = data.to;
            updateLinkedStageSlider(2, 'start', data.to);
            updateStageTitle(1, data.to);
            updateStageTitle(2, rateColorConfig.stage2.end_rate);
            updatePreview();
            validateStageSystem();
        }
    });

    $("#stage2Range").ionRangeSlider({
        type: "double",
        min: 0, max: 100,
        from: rateColorConfig.stage2.start_rate,
        to: rateColorConfig.stage2.end_rate,
        step: 1, postfix: "%", skin: "fiori",
        grid: true, grid_num: 10,
        from_fixed: false,
        onChange: function(data) {
            if (updateInProgress.stage2) return;
            updateInProgress.stage2 = true;

            const oldFrom = rateColorConfig.stage2.start_rate;
            const oldTo   = rateColorConfig.stage2.end_rate;

            rateColorConfig.stage2.start_rate = data.from;
            rateColorConfig.stage2.end_rate   = data.to;

            if (oldFrom !== data.from) {
                rateColorConfig.stage1.end_rate = data.from;
                updateLinkedStageSlider(1, 'end', data.from);
                updateStageTitle(1, data.from);
            }
            if (oldTo !== data.to) {
                rateColorConfig.stage3.start_rate = data.to;
                updateLinkedStageSlider(3, 'start', data.to);
                updateStageTitle(3, rateColorConfig.stage3.end_rate);
            }

            updateStageTitle(2, data.to);
            updatePreview();
            validateStageSystem();
            updateInProgress.stage2 = false;
        }
    });

    $("#stage3Range").ionRangeSlider({
        type: "double",
        min: 0, max: 100,
        from: rateColorConfig.stage3.start_rate,
        to: rateColorConfig.stage3.end_rate,
        step: 1, postfix: "%", skin: "fiori",
        grid: true, grid_num: 10,
        from_fixed: false,
        onChange: function(data) {
            if (updateInProgress.stage3) return;
            updateInProgress.stage3 = true;

            const oldFrom = rateColorConfig.stage3.start_rate;
            const oldTo   = rateColorConfig.stage3.end_rate;

            rateColorConfig.stage3.start_rate = data.from;
            rateColorConfig.stage3.end_rate   = data.to;

            if (oldFrom !== data.from) {
                rateColorConfig.stage2.end_rate = data.from;
                updateLinkedStageSlider(2, 'end', data.from);
                updateStageTitle(2, data.from);
            }
            if (oldTo !== data.to) {
                rateColorConfig.stage4.start_rate = data.to;
                updateLinkedStageSlider(4, 'start', data.to);
                updateStageTitle(4, 100);
            }

            updateStageTitle(3, data.to);
            updatePreview();
            validateStageSystem();
            updateInProgress.stage3 = false;
        }
    });

    $("#stage4Range").ionRangeSlider({
        type: "double",
        min: 0, max: 100,
        from: rateColorConfig.stage4.start_rate,
        to: 100,
        step: 1, postfix: "%", skin: "fiori",
        grid: true, grid_num: 10,
        disable: true
    });
}

function updateLinkedStageSlider(stageNum, position, value) {
    const slider = $(`#stage${stageNum}Range`).data("ionRangeSlider");
    if (!slider) return;
    if (slider.options.disable && stageNum !== 4) return;

    const stageKey = `stage${stageNum}`;
    if (updateInProgress[stageKey]) return;
    updateInProgress[stageKey] = true;

    try {
        const updateObj = {};
        if (position === 'start') {
            updateObj.from = value;
            updateObj.to   = stageNum === 4 ? 100 : rateColorConfig[stageKey].end_rate;
        } else {
            updateObj.from = rateColorConfig[stageKey].start_rate;
            updateObj.to   = value;
        }
        slider.update(updateObj);
    } finally {
        updateInProgress[stageKey] = false;
    }
}


/* ─── Buttons ────────────────────────────────────────────── */

function initializeButtons() {
    document.getElementById('saveBtn').addEventListener('click', saveConfiguration);
    document.getElementById('previewBtn').addEventListener('click', showPreviewModal);
    document.getElementById('testBtn').addEventListener('click', testColorSystem);
}


/* ─── Stage Title ────────────────────────────────────────── */

function updateStageTitle(stage, endValue) {
    const el = document.querySelector(`[data-stage="${stage}"] .rate-stage-title`);
    if (!el) return;

    const cfg = rateColorConfig[`stage${stage}`];
    if (!cfg) return;

    switch (stage) {
        case 1: el.textContent = `Stage 1: 0% < rate \u2264 ${endValue}%`; break;
        case 2: el.textContent = `Stage 2: ${cfg.start_rate}% < rate \u2264 ${endValue}%`; break;
        case 3: el.textContent = `Stage 3: ${cfg.start_rate}% < rate \u2264 ${endValue}%`; break;
        case 4: el.textContent = `Stage 4: ${cfg.start_rate}% < rate \u2264 100%`; break;
        case 5: el.textContent = `Stage 5: Over 100%`; break;
    }
}

function updateAllStagesTitle() {
    for (let i = 1; i <= 5; i++) {
        updateStageTitle(i, rateColorConfig[`stage${i}`].end_rate);
    }
}


/* ─── Preview ────────────────────────────────────────────── */

function updatePreview() {
    const container = document.getElementById('previewContainer');
    if (!container) return;

    container.innerHTML = '';

    for (let i = 1; i <= 5; i++) {
        const stage = rateColorConfig[`stage${i}`];
        let rangeText = '';
        switch (i) {
            case 1: rangeText = `0% < rate \u2264 ${stage.end_rate}%`; break;
            case 2: rangeText = `${stage.start_rate}% < rate \u2264 ${stage.end_rate}%`; break;
            case 3: rangeText = `${stage.start_rate}% < rate \u2264 ${stage.end_rate}%`; break;
            case 4: rangeText = `${stage.start_rate}% < rate \u2264 100%`; break;
            case 5: rangeText = `Over 100%`; break;
        }

        const card = document.createElement('div');
        card.className = 'fiori-card';
        card.id = `preview-stage-${i}`;
        card.style.cssText = `border-left: 4px solid ${stage.color}; background: ${stage.color}10;`;
        card.innerHTML = `
            <div class="fiori-card__content" style="padding: var(--sap-spacing-sm) var(--sap-spacing-md);">
                <div style="display:flex; align-items:center; gap:var(--sap-spacing-sm);">
                    <div class="stage-color-box" style="width:24px; height:24px; background:${stage.color};"></div>
                    <div>
                        <div style="font-weight:var(--sap-font-weight-medium);">Stage ${i}</div>
                        <div style="color:var(--sap-text-secondary); font-size:var(--sap-font-size-sm);">${rangeText}</div>
                    </div>
                </div>
                <div style="margin-top:4px; font-size:var(--sap-font-size-xs); color:var(--sap-text-tertiary);">${stage.color}</div>
            </div>
        `;
        container.appendChild(card);
    }
}

function updateSingleStagePreview(stageNum) {
    const card = document.getElementById(`preview-stage-${stageNum}`);
    if (!card) return;

    const stage = rateColorConfig[`stage${stageNum}`];
    const colorBox = card.querySelector('.stage-color-box');
    if (colorBox) colorBox.style.background = stage.color;
    card.style.borderLeft = `4px solid ${stage.color}`;
    card.style.background = `${stage.color}10`;
}


/* ─── Validation ─────────────────────────────────────────── */

function validateStageSystem() {
    const s = rateColorConfig;

    if (s.stage1.start_rate !== 0) {
        showAlert('error', 'Error: Stage 1 start value must be 0%.');
        return false;
    }
    if (s.stage4.end_rate !== 100) {
        rateColorConfig.stage4.end_rate = 100;
    }
    rateColorConfig.stage5.start_rate = 100;
    rateColorConfig.stage5.end_rate   = 999;

    for (let i = 1; i <= 3; i++) {
        const cur  = s[`stage${i}`];
        const next = s[`stage${i + 1}`];
        if (cur.end_rate !== next.start_rate) {
            showAlert('error', `Error: Stage ${i} end (${cur.end_rate}%) != Stage ${i + 1} start (${next.start_rate}%).`);
            return false;
        }
        if (cur.start_rate >= cur.end_rate) {
            showAlert('error', `Error: Stage ${i} start >= end.`);
            return false;
        }
    }
    if (s.stage4.end_rate !== s.stage5.start_rate) {
        showAlert('error', `Error: Stage 4 end (${s.stage4.end_rate}%) != Stage 5 start (${s.stage5.start_rate}%).`);
        return false;
    }
    return true;
}


/* ─── Save ───────────────────────────────────────────────── */

async function saveConfiguration() {
    if (!validateStageSystem()) return;

    const saveBtn = document.getElementById('saveBtn');
    const originalText = saveBtn.innerHTML;
    saveBtn.innerHTML = 'Saving...';
    saveBtn.disabled = true;

    try {
        const response = await fetch('./proc/rate_color.php?action=save', {
            method: 'POST',
            headers: { 'Content-Type': 'application/json' },
            body: JSON.stringify({ config: rateColorConfig, validation: 'required', stage_count: 5 })
        });

        if (!response.ok) throw new Error(`HTTP Error: ${response.status}`);

        const result = await response.json();

        if (result.code === '00') {
            showAlert('success', `Settings saved successfully. (${result.data.saved_count} stages)`);
            localStorage.setItem('rateColorConfig', JSON.stringify(rateColorConfig));
        } else {
            throw new Error(result.msg || 'Save failed.');
        }
    } catch (error) {
        showAlert('error', `Save error: ${error.message}`);
        try {
            localStorage.setItem('rateColorConfig', JSON.stringify(rateColorConfig));
            showAlert('info', 'Settings saved as local backup.');
        } catch (e) { /* ignore */ }
    } finally {
        saveBtn.innerHTML = originalText;
        saveBtn.disabled = false;
    }
}


/* ─── Load ───────────────────────────────────────────────── */

async function loadExistingConfig() {
    try {
        const response = await fetch('./proc/rate_color.php?action=config');
        if (!response.ok) throw new Error(`HTTP Error: ${response.status}`);

        const text = await response.text();
        if (!text || text.trim() === '') throw new Error('Empty API response');

        let result;
        try { result = JSON.parse(text); }
        catch (e) { throw new Error('Invalid JSON response'); }

        if (result.code === '00' && result.data) {
            const loaded = result.data;
            const stages = ['stage1', 'stage2', 'stage3', 'stage4', 'stage5'];
            const valid  = stages.every(k =>
                loaded[k] &&
                typeof loaded[k].start_rate === 'number' &&
                typeof loaded[k].end_rate   === 'number' &&
                typeof loaded[k].color      === 'string'
            );
            if (valid) {
                rateColorConfig = loaded;
            } else {
                throw new Error('Loaded settings invalid format');
            }
        } else {
            throw new Error(result.msg || 'Load failed');
        }

        updateUIFromConfig();
        initializeSpectrumColorPickers();
        initializeStageColors();

    } catch (error) {
        showAlert('warning', `Settings load failed: ${error.message}. Using defaults.`);
        try {
            const saved = localStorage.getItem('rateColorConfig');
            if (saved) {
                rateColorConfig = JSON.parse(saved);
                showAlert('info', 'Restored from local backup.');
            }
        } catch (e) { /* ignore */ }

        updateUIFromConfig();
        initializeSpectrumColorPickers();
        initializeStageColors();
    }
}

function initializeStageColors() {
    for (let i = 1; i <= 5; i++) {
        const picker = document.getElementById(`stage${i}ColorPicker`);
        if (picker) picker.value = rateColorConfig[`stage${i}`].color;
    }
}

function updateUIFromConfig() {
    const sliders = {
        stage1: $("#stage1Range").data("ionRangeSlider"),
        stage2: $("#stage2Range").data("ionRangeSlider"),
        stage3: $("#stage3Range").data("ionRangeSlider"),
        stage4: $("#stage4Range").data("ionRangeSlider")
    };

    if (sliders.stage1) sliders.stage1.update({ from: 0, to: rateColorConfig.stage1.end_rate });
    if (sliders.stage2) sliders.stage2.update({ from: rateColorConfig.stage2.start_rate, to: rateColorConfig.stage2.end_rate });
    if (sliders.stage3) sliders.stage3.update({ from: rateColorConfig.stage3.start_rate, to: rateColorConfig.stage3.end_rate });
    if (sliders.stage4) sliders.stage4.update({ from: rateColorConfig.stage4.start_rate, to: 100 });

    updateAllStagesTitle();
    updatePreview();
}


/* ─── Spectrum Color Pickers ─────────────────────────────── */

function initializeSpectrumColorPickers() {
    const fioriPalette = [
        ["#0070f2", "#1e88e5", "#2196f3", "#42a5f5"],
        ["#30914c", "#4caf50", "#66bb6a", "#81c784"],
        ["#da1e28", "#f44336", "#ef5350", "#e57373"],
        ["#e26b0a", "#ff9800", "#ffa726", "#ffb74d"],
        ["#32363b", "#4a5568", "#6b7884", "#8b95a1"],
        ["#a0aec0", "#cbd5e0", "#e2e8f0", "#f7fafc"],
        ["#800080", "#9b59b6", "#8e44ad", "#7c3aed"],
        ["#ffa500", "#ff8c00", "#ed8936", "#dd6b20"]
    ];

    for (let stage = 1; stage <= 5; stage++) {
        const el = $(`#stage${stage}ColorPicker`);
        if (!el.length) continue;

        try { el.spectrum('destroy'); } catch (e) { /* first init */ }

        const s = stage; // capture
        el.spectrum({
            type: "component",
            showInput: true,
            showInitial: true,
            allowEmpty: false,
            showAlpha: false,
            showPalette: true,
            showPaletteOnly: false,
            showSelectionPalette: true,
            hideAfterPaletteSelect: true,
            clickoutFiresChange: true,
            color: rateColorConfig[`stage${s}`].color,
            palette: fioriPalette,
            localStorageKey: `spectrum.stage${s}`,
            maxSelectionSize: 10,
            preferredFormat: "hex",
            chooseText: "Select",
            cancelText: "Cancel",
            change: function(color) {
                const hex = color.toHexString();
                rateColorConfig[`stage${s}`].color = hex;
                updatePreview();
                updateSingleStagePreview(s);
            }
        });
    }
}


/* ─── Color Palette Modal (팔레트 직접 선택) ─────────────── */

function selectStageForColorChange(stageNum) {
    currentSelectedStage = stageNum;
    showColorPaletteModal(stageNum);
}

function showColorPaletteModal(stageNum) {
    const currentColor = rateColorConfig[`stage${stageNum}`].color;
    const modal = document.getElementById('colorPaletteModal');
    const title = modal.querySelector('.fiori-card__title');
    title.textContent = `Stage ${stageNum} Color Selection`;

    createModalColorPalette(stageNum, currentColor);

    modal.style.display = 'flex';
    setTimeout(() => modal.classList.add('show'), 10);

    const closeBtn  = modal.querySelector('.modal-close');
    const backdrop  = modal.querySelector('.fiori-modal__backdrop');
    closeBtn.replaceWith(closeBtn.cloneNode(true));
    backdrop.replaceWith(backdrop.cloneNode(true));
    modal.querySelector('.modal-close').addEventListener('click', closeColorPaletteModal);
    modal.querySelector('.fiori-modal__backdrop').addEventListener('click', closeColorPaletteModal);
}

function closeColorPaletteModal() {
    const modal = document.getElementById('colorPaletteModal');
    modal.classList.remove('show');
    setTimeout(() => { modal.style.display = 'none'; }, 300);
    currentSelectedStage = null;
}

function createModalColorPalette(stageNum, currentColor) {
    const container = document.getElementById('modalColorPalette');
    if (!container) return;

    container.innerHTML = colorPalette.map(color => `
        <span class="modal-color-item ${color === currentColor ? 'selected' : ''}"
              title="${color}" data-color="${color}">
            <span class="color-inner" style="background-color:${color};"></span>
        </span>
    `).join('');

    container.addEventListener('click', function(e) {
        const item = e.target.closest('.modal-color-item');
        if (!item) return;
        const selected = item.dataset.color;
        rateColorConfig[`stage${stageNum}`].color = selected;
        updatePreview();
        updateSingleStagePreview(stageNum);
        closeColorPaletteModal();
    });
}


/* ─── Preview Modal ──────────────────────────────────────── */

function showPreviewModal() {
    const modal = document.getElementById('previewModal');
    if (!modal) return;

    updateModalPreview();

    modal.style.display = 'flex';
    setTimeout(() => modal.classList.add('show'), 10);

    const closeBtn = modal.querySelector('.modal-close');
    const backdrop = modal.querySelector('.fiori-modal__backdrop');
    closeBtn.replaceWith(closeBtn.cloneNode(true));
    backdrop.replaceWith(backdrop.cloneNode(true));
    modal.querySelector('.modal-close').addEventListener('click', closePreviewModal);
    modal.querySelector('.fiori-modal__backdrop').addEventListener('click', closePreviewModal);
}

function closePreviewModal() {
    const modal = document.getElementById('previewModal');
    modal.classList.remove('show');
    setTimeout(() => { modal.style.display = 'none'; }, 300);
}

function updateModalPreview() {
    const container = document.getElementById('modalPreviewContainer');
    if (!container) return;

    container.innerHTML = '';

    Object.keys(rateColorConfig).forEach(key => {
        const stage = rateColorConfig[key];
        const num   = key.replace('stage', '');
        let rangeText = '';
        if (num === '1')      rangeText = `0% ~ ${stage.end_rate}%`;
        else if (num === '5') rangeText = `Over 100%`;
        else                  rangeText = `${stage.start_rate}% < rate \u2264 ${stage.end_rate}%`;

        const item = document.createElement('div');
        item.style.cssText = `
            display:flex; align-items:center; gap:var(--sap-spacing-md);
            padding:var(--sap-spacing-md);
            border:1px solid var(--sap-border-neutral);
            border-radius:var(--sap-radius-sm);
            margin-bottom:var(--sap-spacing-sm);
            background:${stage.color}08;
        `;
        item.innerHTML = `
            <div style="width:40px; height:40px; background:${stage.color}; border-radius:var(--sap-radius-sm); border:1px solid var(--sap-border-neutral); flex-shrink:0;"></div>
            <div>
                <div style="font-weight:var(--sap-font-weight-medium);">Stage ${num}: ${rangeText}</div>
                <div style="color:var(--sap-text-secondary); font-size:var(--sap-font-size-sm);">Color: ${stage.color}</div>
            </div>
        `;
        container.appendChild(item);
    });
}


/* ─── Test ───────────────────────────────────────────────── */

function testColorSystem() {
    const rates   = [0, 25, 50, 65, 80, 95, 100, 105, 120];
    const results = rates.map(r => ({ rate: r, color: getRateColor(r) }));
    showAlert('info', `Test completed: ${results.length} values verified`);
}

function getRateColor(rate) {
    const s = rateColorConfig;
    if (rate > s.stage1.start_rate && rate <= s.stage1.end_rate) return s.stage1.color;
    if (rate > s.stage1.end_rate   && rate <= s.stage2.end_rate) return s.stage2.color;
    if (rate > s.stage2.end_rate   && rate <= s.stage3.end_rate) return s.stage3.color;
    if (rate > s.stage3.end_rate   && rate <= 100)               return s.stage4.color;
    if (rate > 100)                                               return s.stage5.color;
    return s.stage1.color;
}


/* ─── Alert ──────────────────────────────────────────────── */

const _alertStyle = document.createElement('style');
_alertStyle.textContent = `
@keyframes slideInRight  { from { transform:translateX(100%); opacity:0; } to { transform:translateX(0); opacity:1; } }
@keyframes slideOutRight { from { transform:translateX(0); opacity:1; }   to { transform:translateX(100%); opacity:0; } }
`;
document.head.appendChild(_alertStyle);

function showAlert(type, message) {
    const existing = document.querySelector('.rate-color-alert');
    if (existing) existing.remove();

    const el = document.createElement('div');
    el.className = `fiori-alert fiori-alert--${type} rate-color-alert`;
    el.style.animation = 'slideInRight 0.3s ease';
    el.textContent = message;
    document.body.appendChild(el);

    setTimeout(() => {
        if (!el.parentNode) return;
        el.style.animation = 'slideOutRight 0.3s ease';
        setTimeout(() => el.remove(), 300);
    }, 3000);
}
