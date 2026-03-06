/**
 * main.js - Main controller
 * App initialization, mode selection, event binding, UI mode switching
 */
document.addEventListener('DOMContentLoaded', () => {
    // --- DOM References ---
    const modeSelection = document.getElementById('mode-selection');
    const appWrapper = document.getElementById('app-wrapper');
    const modeBtn2 = document.getElementById('mode-btn-2');
    const modeBtn3 = document.getElementById('mode-btn-3');
    const btnModeChange = document.getElementById('btn-mode-change');
    const headerSubtitle = document.getElementById('header-subtitle');

    const manualModal = document.getElementById('manual-modal');
    const modalClose = document.getElementById('modal-close');
    const btnManual = document.getElementById('btn-manual');

    const btnQuick = document.getElementById('btn-quick');
    const btnRealtime = document.getElementById('btn-realtime');
    const btnPause = document.getElementById('btn-pause');
    const btnResume = document.getElementById('btn-resume');
    const btnStop = document.getElementById('btn-stop');
    const btnPdf = document.getElementById('btn-pdf');
    const btnSaveScenario = document.getElementById('btn-save-scenario');
    const btnClearScenarios = document.getElementById('btn-clear-scenarios');
    const speedGroup = document.getElementById('speed-group');
    const speedButtons = document.querySelectorAll('.btn-speed');
    const simView = document.getElementById('sim-view');
    const resultsSection = document.getElementById('results-section');
    const logSection = document.getElementById('log-section');
    const btnToggleLog = document.getElementById('btn-toggle-log');
    const logContainer = document.getElementById('log-container');
    const simHours = document.getElementById('sim-hours');
    const simMinutes = document.getElementById('sim-minutes');
    const durationTotal = document.getElementById('duration-total');

    // --- Pallet Count Selector ---
    const palletCountButtons = document.querySelectorAll('.pallet-count-btn');

    palletCountButtons.forEach(btn => {
        btn.addEventListener('click', () => {
            palletCountButtons.forEach(b => b.classList.remove('active'));
            btn.classList.add('active');
        });
    });

    /**
     * Set pallet count default by mode
     */
    function setPalletCountDefault(machineCount) {
        const defaultCount = machineCount === 2 ? 5 : 6;
        palletCountButtons.forEach(btn => {
            btn.classList.remove('active');
            if (parseInt(btn.dataset.palletCount) === defaultCount) {
                btn.classList.add('active');
            }
        });
    }

    // --- Robot Speed Selector ---
    const robotSpeedButtons = document.querySelectorAll('.robot-speed-btn');
    let currentRobotSpeed = 'manual';

    robotSpeedButtons.forEach(btn => {
        btn.addEventListener('click', () => {
            const speed = btn.dataset.robotSpeed;
            currentRobotSpeed = speed;

            robotSpeedButtons.forEach(b => b.classList.remove('active'));
            btn.classList.add('active');

            if (speed === 'manual') {
                setInputsReadonly(false);
            } else {
                const speedValues = Config.getSpeedValues(parseInt(speed));
                applySpeedValues(speedValues);
                setInputsReadonly(true);
            }
        });
    });

    /**
     * Apply speed values to input fields (excluding sewing time)
     */
    function applySpeedValues(speedValues) {
        for (const id of Config.MACHINE_IDS) {
            const vals = speedValues[id];
            if (!vals) continue;
            document.getElementById(`m${id}-init`).value = vals.initLoadTime;
            document.getElementById(`m${id}-ul`).value = vals.unloadLoadTime;
            document.getElementById(`m${id}-return`).value = vals.returnTime;
        }
    }

    /**
     * Set readonly state for robot speed related input fields
     * Sewing time is always editable
     */
    function setInputsReadonly(readonly) {
        for (const id of [1, 2, 3]) {
            document.getElementById(`m${id}-init`).readOnly = readonly;
            document.getElementById(`m${id}-ul`).readOnly = readonly;
            document.getElementById(`m${id}-return`).readOnly = readonly;
        }
    }

    // --- State ---
    let mode = 'idle';
    let lastResults = null;
    let lastMachineConfigs = null;
    let savedM2Values = null;

    // --- Duration Selector ---

    function populateMinutes(max) {
        const currentVal = parseInt(simMinutes.value) || 0;
        simMinutes.innerHTML = '';
        for (let i = 0; i <= max; i++) {
            const opt = document.createElement('option');
            opt.value = i;
            opt.textContent = String(i).padStart(2, '0') + ' min';
            simMinutes.appendChild(opt);
        }
        if (currentVal <= max) {
            simMinutes.value = currentVal;
        } else {
            simMinutes.value = 0;
        }
    }

    populateMinutes(59);

    function getSelectedDuration() {
        const hours = parseInt(simHours.value);
        const minutes = parseInt(simMinutes.value);
        return hours * 3600 + minutes * 60;
    }

    function updateDurationDisplay() {
        const hours = parseInt(simHours.value);
        const minutes = parseInt(simMinutes.value);
        const hText = hours + ' hr';
        const mText = String(minutes).padStart(2, '0') + ' min';
        durationTotal.textContent = hText + ' ' + mText;
    }

    simHours.addEventListener('change', () => {
        const hours = parseInt(simHours.value);
        if (hours === 24) {
            populateMinutes(0);
        } else {
            populateMinutes(59);
        }
        updateDurationDisplay();
    });

    simMinutes.addEventListener('change', () => {
        updateDurationDisplay();
    });

    // --- Mode Selection ---

    modeBtn2.addEventListener('click', () => selectMode(2));
    modeBtn3.addEventListener('click', () => selectMode(3));

    function goToModeSelection() {
        if (mode === 'realtime') {
            stopRealtime();
        }
        lastResults = null;
        lastMachineConfigs = null;
        resultsSection.style.display = 'none';
        logSection.style.display = 'none';
        simView.style.display = 'none';
        speedGroup.style.display = 'none';

        if (typeof Charts !== 'undefined') {
            Charts.destroyAllCharts();
        }

        if (typeof ScenarioManager !== 'undefined') {
            ScenarioManager.destroyCharts();
        }

        appWrapper.style.display = 'none';
        modeSelection.style.display = 'flex';
    }

    btnModeChange.addEventListener('click', goToModeSelection);

    /**
     * Mode selection handler - UI and settings update
     */
    function selectMode(count) {
        Config.setMachineCount(count);
        Renderer.setMachineCount(count);

        setPalletCountDefault(count);

        modeSelection.style.display = 'none';
        appWrapper.style.display = '';

        // Update header subtitle
        const subtitleText = `Co-robot 1 + Sewing Machine ${count} Simulator `;
        headerSubtitle.innerHTML = '';
        headerSubtitle.appendChild(document.createTextNode(subtitleText));
        const changeBtn = document.createElement('button');
        changeBtn.className = 'btn-mode-change';
        changeBtn.id = 'btn-mode-change';
        changeBtn.textContent = 'Change Mode';
        changeBtn.addEventListener('click', goToModeSelection);
        headerSubtitle.appendChild(changeBtn);

        headerSubtitle.appendChild(document.createTextNode(' '));
        const manualBtn = document.createElement('button');
        manualBtn.className = 'btn-mode-change btn-manual';
        manualBtn.textContent = 'Manual';
        manualBtn.addEventListener('click', openManual);
        headerSubtitle.appendChild(manualBtn);

        // Update input section
        const inputGrid = document.getElementById('input-grid');
        const machine3Input = document.getElementById('machine3-input');
        const m2Label = document.getElementById('m2-input-label');

        if (count === 2) {
            savedM2Values = {
                init: document.getElementById('m2-init').value,
                ul: document.getElementById('m2-ul').value,
                return: document.getElementById('m2-return').value,
                sewing: document.getElementById('m2-sewing').value,
            };
            document.getElementById('m2-init').value = document.getElementById('m3-init').value;
            document.getElementById('m2-ul').value = document.getElementById('m3-ul').value;
            document.getElementById('m2-return').value = document.getElementById('m3-return').value;
            document.getElementById('m2-sewing').value = document.getElementById('m3-sewing').value;

            inputGrid.classList.add('mode-2');
            machine3Input.style.display = 'none';
            m2Label.textContent = '\u{1f7e2} Machine 2 (Right)';
        } else {
            if (savedM2Values) {
                document.getElementById('m2-init').value = savedM2Values.init;
                document.getElementById('m2-ul').value = savedM2Values.ul;
                document.getElementById('m2-return').value = savedM2Values.return;
                document.getElementById('m2-sewing').value = savedM2Values.sewing;
                savedM2Values = null;
            }

            inputGrid.classList.remove('mode-2');
            machine3Input.style.display = '';
            m2Label.textContent = '\u{1f7e2} Machine 2 (Top)';
        }

        // Update results grid
        const resultsGrid = document.getElementById('results-grid');
        const resultCardM3 = document.getElementById('result-card-m3');

        if (count === 2) {
            resultsGrid.classList.add('mode-2');
            resultCardM3.style.display = 'none';
        } else {
            resultsGrid.classList.remove('mode-2');
            resultCardM3.style.display = '';
        }

        // Update realtime stats bar
        const realtimeStats = document.getElementById('realtime-stats');
        const statM3Item = document.getElementById('stat-m3-item');

        if (count === 2) {
            realtimeStats.classList.add('mode-2');
            statM3Item.style.display = 'none';
        } else {
            realtimeStats.classList.remove('mode-2');
            statM3Item.style.display = '';
        }

        // Update simulation arena
        setupSimArena(count);

        // Reset previous results
        resultsSection.style.display = 'none';
        logSection.style.display = 'none';
        simView.style.display = 'none';
        speedGroup.style.display = 'none';
        lastResults = null;
        lastMachineConfigs = null;
    }

    /**
     * Update simulation arena node layout
     */
    function setupSimArena(count) {
        const nodeM2 = document.getElementById('node-m2');
        const nodeM3 = document.getElementById('node-m3');
        const pathM2Top = document.getElementById('path-m2-top');
        const simM2Label = document.getElementById('sim-m2-label');

        if (count === 2) {
            nodeM2.style.top = '50%';
            nodeM2.style.left = 'auto';
            nodeM2.style.right = '30px';
            nodeM2.style.transform = 'translateY(-50%)';
            simM2Label.textContent = 'Machine 2';

            nodeM3.style.display = 'none';
            pathM2Top.style.display = 'none';
        } else {
            nodeM2.style.top = '40px';
            nodeM2.style.left = '50%';
            nodeM2.style.right = '';
            nodeM2.style.transform = 'translateX(-50%)';
            simM2Label.textContent = 'Machine 2';

            nodeM3.style.display = '';
            pathM2Top.style.display = '';
        }
    }

    // --- Event Handlers ---

    // Quick Calc
    btnQuick.addEventListener('click', () => {
        if (mode === 'realtime') {
            stopRealtime();
        }

        const cfg = Config.getConfig();
        if (!cfg.valid) {
            alert('Input error:\n' + cfg.errors.join('\n'));
            return;
        }

        const basicSettings = Config.readBasicSettings();

        mode = 'quick';
        simView.style.display = 'none';
        speedGroup.style.display = 'none';

        QuickCalc.setDuration(getSelectedDuration());

        const results = QuickCalc.run(cfg.machines, basicSettings.palletPrepTime, basicSettings.palletCount);
        QuickCalc.displayResults(results, basicSettings.workerCount);

        lastResults = results;
        lastMachineConfigs = cfg.machines;

        btnQuick.disabled = false;
        btnRealtime.disabled = false;
        btnStop.disabled = true;
        mode = 'idle';
    });

    // --- Timeline Controls ---
    const timelineSlider = document.getElementById('timeline-slider');
    const timelineCurrent = document.getElementById('timeline-current');
    const timelineTotal = document.getElementById('timeline-total');
    const btnSeekBack = document.getElementById('btn-seek-back');
    const btnSeekForward = document.getElementById('btn-seek-forward');
    let timelineDragging = false;

    function formatTimelineTime(seconds) {
        const h = Math.floor(seconds / 3600);
        const m = Math.floor((seconds % 3600) / 60);
        const s = Math.floor(seconds % 60);
        if (h > 0) return `${h}:${String(m).padStart(2, '0')}:${String(s).padStart(2, '0')}`;
        return `${m}:${String(s).padStart(2, '0')}`;
    }

    function updateTimelineSliderBackground() {
        const val = parseFloat(timelineSlider.value);
        const max = parseFloat(timelineSlider.max);
        const pct = max > 0 ? (val / max) * 100 : 0;
        timelineSlider.style.background = `linear-gradient(to right, var(--color-primary) ${pct}%, #e2e8f0 ${pct}%)`;
    }

    function initTimeline(duration) {
        timelineSlider.max = duration;
        timelineSlider.value = 0;
        timelineCurrent.textContent = formatTimelineTime(0);
        timelineTotal.textContent = formatTimelineTime(duration);
        updateTimelineSliderBackground();
    }

    function updateTimeline(currentTime) {
        if (timelineDragging) return;
        timelineSlider.value = currentTime;
        timelineCurrent.textContent = formatTimelineTime(currentTime);
        updateTimelineSliderBackground();
    }

    // Slider drag start
    timelineSlider.addEventListener('mousedown', () => { timelineDragging = true; });
    timelineSlider.addEventListener('touchstart', () => { timelineDragging = true; });

    // Slider value change (preview during drag)
    timelineSlider.addEventListener('input', () => {
        const time = parseFloat(timelineSlider.value);
        timelineCurrent.textContent = formatTimelineTime(time);
        updateTimelineSliderBackground();
    });

    // Slider drag complete -> execute seek
    timelineSlider.addEventListener('change', () => {
        timelineDragging = false;
        const time = parseFloat(timelineSlider.value);
        if (RealtimeCalc.isActive()) {
            RealtimeCalc.seekTo(time);
        }
    });
    timelineSlider.addEventListener('mouseup', () => { timelineDragging = false; });
    timelineSlider.addEventListener('touchend', () => { timelineDragging = false; });

    // -10s / +10s buttons
    btnSeekBack.addEventListener('click', () => {
        if (!RealtimeCalc.isActive()) return;
        const newTime = Math.max(0, RealtimeCalc.getSimTime() - 10);
        RealtimeCalc.seekTo(newTime);
    });

    btnSeekForward.addEventListener('click', () => {
        if (!RealtimeCalc.isActive()) return;
        const max = RealtimeCalc.getMaxDuration();
        const newTime = Math.min(max > 0 ? max : Infinity, RealtimeCalc.getSimTime() + 10);
        RealtimeCalc.seekTo(newTime);
    });

    // Realtime Calc
    btnRealtime.addEventListener('click', () => {
        if (mode === 'realtime') {
            stopRealtime();
        }

        const cfg = Config.getConfig();
        if (!cfg.valid) {
            alert('Input error:\n' + cfg.errors.join('\n'));
            return;
        }

        mode = 'realtime';

        resultsSection.style.display = 'none';
        logSection.style.display = 'none';
        simView.style.display = '';
        speedGroup.style.display = 'flex';

        btnQuick.disabled = true;
        btnRealtime.disabled = true;
        btnPause.disabled = false;
        btnPause.style.display = '';
        btnResume.disabled = true;
        btnResume.style.display = 'none';
        btnStop.disabled = false;

        // Set time limit
        const basicSettings = Config.readBasicSettings();
        const duration = getSelectedDuration();
        RealtimeCalc.setDuration(duration);
        RealtimeCalc.setOnTimeLimit(() => {
            stopRealtime();
        });

        // Initialize timeline
        initTimeline(duration);
        RealtimeCalc.setOnTimeUpdate(updateTimeline);

        // Sync speed slider to initial value
        applySpeed(1);
        speedButtons.forEach(b => b.classList.remove('active'));
        const btn1x = document.querySelector('.btn-speed[data-speed="1"]');
        if (btn1x) btn1x.classList.add('active');

        // Start simulation (pass palletPrepTime, palletCount)
        RealtimeCalc.start(cfg.machines, basicSettings.palletPrepTime, basicSettings.palletCount);
    });

    // Pause
    btnPause.addEventListener('click', () => {
        RealtimeCalc.pause();
        btnPause.disabled = true;
        btnPause.style.display = 'none';
        btnResume.disabled = false;
        btnResume.style.display = '';
    });

    // Resume
    btnResume.addEventListener('click', () => {
        RealtimeCalc.resume();
        btnResume.disabled = true;
        btnResume.style.display = 'none';
        btnPause.disabled = false;
        btnPause.style.display = '';
    });

    // Stop
    btnStop.addEventListener('click', () => {
        stopRealtime();
    });

    function stopRealtime() {
        const cfg = Config.getConfig();
        const basicSettings = Config.readBasicSettings();
        const results = RealtimeCalc.stop();
        if (results && cfg.valid) {
            lastResults = results;
            lastMachineConfigs = cfg.machines;
            QuickCalc.displayResults(results, basicSettings.workerCount);
        }
        mode = 'idle';

        btnQuick.disabled = false;
        btnRealtime.disabled = false;
        btnPause.disabled = true;
        btnPause.style.display = 'none';
        btnResume.disabled = true;
        btnResume.style.display = 'none';
        btnStop.disabled = true;
    }

    // --- Speed Control (log scale slider + preset buttons) ---
    const speedSlider = document.getElementById('speed-slider');
    const speedDisplay = document.getElementById('speed-display');

    // Log scale mapping: slider(0~100) -> speed(0.1~100)
    function sliderToSpeed(val) {
        return Math.pow(10, (val / 50) - 1);
    }

    function speedToSlider(speed) {
        return (Math.log10(speed) + 1) * 50;
    }

    function formatSpeed(speed) {
        if (speed >= 10) return Math.round(speed) + 'x';
        if (speed >= 1) return speed.toFixed(1) + 'x';
        return speed.toFixed(2) + 'x';
    }

    function applySpeed(spd) {
        RealtimeCalc.setSpeed(spd);
        speedSlider.value = speedToSlider(spd);
        speedDisplay.textContent = formatSpeed(spd);
    }

    // Preset buttons
    speedButtons.forEach(btn => {
        btn.addEventListener('click', () => {
            const spd = parseFloat(btn.dataset.speed);
            applySpeed(spd);

            speedButtons.forEach(b => b.classList.remove('active'));
            btn.classList.add('active');
        });
    });

    // Speed slider
    speedSlider.addEventListener('input', () => {
        const spd = sliderToSpeed(parseFloat(speedSlider.value));
        RealtimeCalc.setSpeed(spd);
        speedDisplay.textContent = formatSpeed(spd);

        // Deactivate preset buttons
        speedButtons.forEach(b => b.classList.remove('active'));
    });

    // PDF Save
    btnPdf.addEventListener('click', () => {
        if (!lastResults || !lastMachineConfigs) {
            alert('Please run a simulation first.');
            return;
        }
        PdfExport.exportPdf(lastResults, lastMachineConfigs);
    });

    // Scenario Save
    btnSaveScenario.addEventListener('click', () => {
        if (!lastResults || !lastMachineConfigs) {
            alert('Please run a simulation first.');
            return;
        }

        const basicSettings = Config.readBasicSettings();
        const settings = {
            workerCount: basicSettings.workerCount,
            palletPrepTime: basicSettings.palletPrepTime,
            palletCount: basicSettings.palletCount,
            simDuration: getSelectedDuration(),
            machines: lastMachineConfigs
        };

        ScenarioManager.openSaveDialog(settings, lastResults);
        updateScenarioCount();
    });

    // Clear All Scenarios
    btnClearScenarios.addEventListener('click', () => {
        const scenarios = ScenarioManager.getScenarios();
        if (scenarios.length === 0) {
            alert('No saved scenarios.');
            return;
        }
        if (confirm(`Delete all ${scenarios.length} scenarios?`)) {
            ScenarioManager.clearAllScenarios();
            updateScenarioCount();
        }
    });

    // Update scenario count
    function updateScenarioCount() {
        const countEl = document.getElementById('scenario-count');
        if (countEl) {
            const count = ScenarioManager.getScenarios().length;
            countEl.textContent = `(${count}/${ScenarioManager.MAX_SCENARIOS})`;
        }
    }

    // Event log toggle
    btnToggleLog.addEventListener('click', () => {
        logContainer.classList.toggle('collapsed');
    });

    // --- Manual Modal ---
    function openManual() {
        manualModal.classList.add('active');
    }

    function closeManual() {
        manualModal.classList.remove('active');
    }

    btnManual.addEventListener('click', openManual);
    modalClose.addEventListener('click', closeManual);

    manualModal.addEventListener('click', (e) => {
        if (e.target === manualModal) closeManual();
    });

    document.addEventListener('keydown', (e) => {
        if (e.key === 'Escape' && manualModal.classList.contains('active')) {
            closeManual();
        }
    });

    // --- Initialize: Apply default robot speed (85%) ---
    const initialSpeedBtn = document.querySelector('.robot-speed-btn.active');
    if (initialSpeedBtn && initialSpeedBtn.dataset.robotSpeed !== 'manual') {
        currentRobotSpeed = initialSpeedBtn.dataset.robotSpeed;
        const speedValues = Config.getSpeedValues(parseInt(currentRobotSpeed));
        applySpeedValues(speedValues);
        setInputsReadonly(true);
    }
});
