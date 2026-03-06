/**
 * main.js - 메인 컨트롤러
 * 앱 초기화, 모드 선택, 이벤트 바인딩, UI 모드 전환
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
     * Pallet 수량 기본값 설정 (모드별)
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
    let currentRobotSpeed = 'manual'; // 'manual' | '75' | '80' | ... | '100'

    robotSpeedButtons.forEach(btn => {
        btn.addEventListener('click', () => {
            const speed = btn.dataset.robotSpeed;
            currentRobotSpeed = speed;

            // 활성 버튼 전환
            robotSpeedButtons.forEach(b => b.classList.remove('active'));
            btn.classList.add('active');

            if (speed === 'manual') {
                // 직접입력: readonly 해제
                setInputsReadonly(false);
            } else {
                // 속도 선택: 값 자동 입력 + readonly 설정
                const speedValues = Config.getSpeedValues(parseInt(speed));
                applySpeedValues(speedValues);
                setInputsReadonly(true);
            }
        });
    });

    /**
     * 속도 선택 값을 input 필드에 적용 (재봉시간 제외)
     */
    function applySpeedValues(speedValues) {
        for (const id of Config.MACHINE_IDS) {
            const vals = speedValues[id];
            if (!vals) continue;
            document.getElementById(`m${id}-init`).value = vals.initLoadTime;
            document.getElementById(`m${id}-ul`).value = vals.unloadLoadTime;
            document.getElementById(`m${id}-return`).value = vals.returnTime;
            // sewingTime은 로봇 속도와 무관 → 건드리지 않음
        }
    }

    /**
     * 로봇 속도 관련 input 필드의 readonly 상태 설정
     * 재봉시간(sewing)은 항상 수정 가능
     */
    function setInputsReadonly(readonly) {
        for (const id of [1, 2, 3]) {
            document.getElementById(`m${id}-init`).readOnly = readonly;
            document.getElementById(`m${id}-ul`).readOnly = readonly;
            document.getElementById(`m${id}-return`).readOnly = readonly;
            // sewing은 항상 수정 가능
        }
    }

    // --- State ---
    let mode = 'idle'; // 'idle' | 'quick' | 'realtime'
    let lastResults = null;
    let lastMachineConfigs = null;
    let savedM2Values = null; // 2대 모드 전환 시 m2 원본 값 보관용

    // --- Duration Selector ---

    // 분 select 옵션 초기화 (00~59)
    function populateMinutes(max) {
        const currentVal = parseInt(simMinutes.value) || 0;
        simMinutes.innerHTML = '';
        for (let i = 0; i <= max; i++) {
            const opt = document.createElement('option');
            opt.value = i;
            opt.textContent = String(i).padStart(2, '0') + '분';
            simMinutes.appendChild(opt);
        }
        // 이전 값 유지 (범위 내일 경우)
        if (currentVal <= max) {
            simMinutes.value = currentVal;
        } else {
            simMinutes.value = 0;
        }
    }

    // 초기 분 옵션 생성 (1시간이 기본이므로 0~59)
    populateMinutes(59);

    // 선택한 시간을 초 단위로 반환
    function getSelectedDuration() {
        const hours = parseInt(simHours.value);
        const minutes = parseInt(simMinutes.value);
        return hours * 3600 + minutes * 60;
    }

    // 총 시뮬레이션 시간 표시 업데이트
    function updateDurationDisplay() {
        const hours = parseInt(simHours.value);
        const minutes = parseInt(simMinutes.value);
        const hText = hours + '시간';
        const mText = String(minutes).padStart(2, '0') + '분';
        durationTotal.textContent = hText + ' ' + mText;
    }

    // 시간 select 변경 이벤트
    simHours.addEventListener('change', () => {
        const hours = parseInt(simHours.value);
        if (hours === 24) {
            // 24시간 선택 시 분은 00만 가능
            populateMinutes(0);
        } else {
            populateMinutes(59);
        }
        updateDurationDisplay();
    });

    // 분 select 변경 이벤트
    simMinutes.addEventListener('change', () => {
        updateDurationDisplay();
    });

    // --- Mode Selection ---

    modeBtn2.addEventListener('click', () => selectMode(2));
    modeBtn3.addEventListener('click', () => selectMode(3));

    function goToModeSelection() {
        // 실시간 실행 중이면 먼저 중지
        if (mode === 'realtime') {
            stopRealtime();
        }
        // 결과 초기화
        lastResults = null;
        lastMachineConfigs = null;
        resultsSection.style.display = 'none';
        logSection.style.display = 'none';
        simView.style.display = 'none';
        speedGroup.style.display = 'none';

        // 차트 초기화
        if (typeof Charts !== 'undefined') {
            Charts.destroyAllCharts();
        }

        // 시나리오 차트 초기화
        if (typeof ScenarioManager !== 'undefined') {
            ScenarioManager.destroyCharts();
        }

        // 모드 선택 화면으로 복귀
        appWrapper.style.display = 'none';
        modeSelection.style.display = 'flex';
    }

    btnModeChange.addEventListener('click', goToModeSelection);

    /**
     * 모드 선택 처리 - UI 및 설정 업데이트
     */
    function selectMode(count) {
        // 설정 업데이트
        Config.setMachineCount(count);
        Renderer.setMachineCount(count);

        // Pallet 수량 기본값 설정 (2대: 5개, 3대: 6개)
        setPalletCountDefault(count);

        // UI 전환
        modeSelection.style.display = 'none';
        appWrapper.style.display = '';

        // 헤더 서브타이틀 업데이트
        const subtitleText = `Co-robot 1대 + 컴퓨터재봉기 ${count}대 작업 시뮬레이터 `;
        headerSubtitle.innerHTML = '';
        headerSubtitle.appendChild(document.createTextNode(subtitleText));
        const changeBtn = document.createElement('button');
        changeBtn.className = 'btn-mode-change';
        changeBtn.id = 'btn-mode-change';
        changeBtn.textContent = '모드 변경';
        changeBtn.addEventListener('click', goToModeSelection);
        headerSubtitle.appendChild(changeBtn);

        headerSubtitle.appendChild(document.createTextNode(' '));
        const manualBtn = document.createElement('button');
        manualBtn.className = 'btn-mode-change btn-manual';
        manualBtn.textContent = '메뉴얼';
        manualBtn.addEventListener('click', openManual);
        headerSubtitle.appendChild(manualBtn);

        // 입력 섹션 업데이트
        const inputGrid = document.getElementById('input-grid');
        const machine3Input = document.getElementById('machine3-input');
        const m2Label = document.getElementById('m2-input-label');

        if (count === 2) {
            // 2대 모드: 재봉기 2가 3번 위치(우측)로 이동하므로 m3 값을 m2에 적용
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
            m2Label.textContent = '🟢 재봉기 2 (우측)';
        } else {
            // 3대 모드: m2 원래 값 복원
            if (savedM2Values) {
                document.getElementById('m2-init').value = savedM2Values.init;
                document.getElementById('m2-ul').value = savedM2Values.ul;
                document.getElementById('m2-return').value = savedM2Values.return;
                document.getElementById('m2-sewing').value = savedM2Values.sewing;
                savedM2Values = null;
            }

            inputGrid.classList.remove('mode-2');
            machine3Input.style.display = '';
            m2Label.textContent = '🟢 재봉기 2 (상단)';
        }

        // 결과 그리드 업데이트
        const resultsGrid = document.getElementById('results-grid');
        const resultCardM3 = document.getElementById('result-card-m3');

        if (count === 2) {
            resultsGrid.classList.add('mode-2');
            resultCardM3.style.display = 'none';
        } else {
            resultsGrid.classList.remove('mode-2');
            resultCardM3.style.display = '';
        }

        // 실시간 통계 바 업데이트
        const realtimeStats = document.getElementById('realtime-stats');
        const statM3Item = document.getElementById('stat-m3-item');

        if (count === 2) {
            realtimeStats.classList.add('mode-2');
            statM3Item.style.display = 'none';
        } else {
            realtimeStats.classList.remove('mode-2');
            statM3Item.style.display = '';
        }

        // 시뮬레이션 아레나 업데이트
        setupSimArena(count);

        // 이전 결과 초기화
        resultsSection.style.display = 'none';
        logSection.style.display = 'none';
        simView.style.display = 'none';
        speedGroup.style.display = 'none';
        lastResults = null;
        lastMachineConfigs = null;
    }

    /**
     * 시뮬레이션 아레나 노드 배치 업데이트
     */
    function setupSimArena(count) {
        const nodeM2 = document.getElementById('node-m2');
        const nodeM3 = document.getElementById('node-m3');
        const pathM2Top = document.getElementById('path-m2-top');
        const simM2Label = document.getElementById('sim-m2-label');

        if (count === 2) {
            // M2를 우측 위치로 이동 (M3 위치와 동일)
            nodeM2.style.top = '50%';
            nodeM2.style.left = 'auto';
            nodeM2.style.right = '30px';
            nodeM2.style.transform = 'translateY(-50%)';
            simM2Label.textContent = '재봉기 2';

            // M3 숨김
            nodeM3.style.display = 'none';

            // 상단 경로 숨김
            pathM2Top.style.display = 'none';
        } else {
            // M2를 상단 위치로 복원
            nodeM2.style.top = '40px';
            nodeM2.style.left = '50%';
            nodeM2.style.right = '';
            nodeM2.style.transform = 'translateX(-50%)';
            simM2Label.textContent = '재봉기 2';

            // M3 표시
            nodeM3.style.display = '';

            // 상단 경로 표시
            pathM2Top.style.display = '';
        }
    }

    // --- Event Handlers ---

    // 빠른계산
    btnQuick.addEventListener('click', () => {
        if (mode === 'realtime') {
            stopRealtime();
        }

        const cfg = Config.getConfig();
        if (!cfg.valid) {
            alert('입력값 오류:\n' + cfg.errors.join('\n'));
            return;
        }

        const basicSettings = Config.readBasicSettings();

        mode = 'quick';
        simView.style.display = 'none';
        speedGroup.style.display = 'none';

        // 선택한 시간 적용
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

    // --- 타임라인 컨트롤 ---
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

    // 슬라이더 드래그 시작
    timelineSlider.addEventListener('mousedown', () => { timelineDragging = true; });
    timelineSlider.addEventListener('touchstart', () => { timelineDragging = true; });

    // 슬라이더 값 변경 (드래그 중 미리보기)
    timelineSlider.addEventListener('input', () => {
        const time = parseFloat(timelineSlider.value);
        timelineCurrent.textContent = formatTimelineTime(time);
        updateTimelineSliderBackground();
    });

    // 슬라이더 드래그 완료 → seek 실행
    timelineSlider.addEventListener('change', () => {
        timelineDragging = false;
        const time = parseFloat(timelineSlider.value);
        if (RealtimeCalc.isActive()) {
            RealtimeCalc.seekTo(time);
        }
    });
    timelineSlider.addEventListener('mouseup', () => { timelineDragging = false; });
    timelineSlider.addEventListener('touchend', () => { timelineDragging = false; });

    // -10초 / +10초 버튼
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

    // 실시간계산
    btnRealtime.addEventListener('click', () => {
        if (mode === 'realtime') {
            stopRealtime();
        }

        const cfg = Config.getConfig();
        if (!cfg.valid) {
            alert('입력값 오류:\n' + cfg.errors.join('\n'));
            return;
        }

        mode = 'realtime';

        // UI 전환
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

        // 선택한 시간 제한 설정
        const basicSettings = Config.readBasicSettings();
        const duration = getSelectedDuration();
        RealtimeCalc.setDuration(duration);
        RealtimeCalc.setOnTimeLimit(() => {
            stopRealtime();
        });

        // 타임라인 초기화
        initTimeline(duration);
        RealtimeCalc.setOnTimeUpdate(updateTimeline);

        // 속도 슬라이더 초기값 동기화
        applySpeed(1);
        speedButtons.forEach(b => b.classList.remove('active'));
        const btn1x = document.querySelector('.btn-speed[data-speed="1"]');
        if (btn1x) btn1x.classList.add('active');

        // 시뮬레이션 시작 (palletPrepTime, palletCount 전달)
        RealtimeCalc.start(cfg.machines, basicSettings.palletPrepTime, basicSettings.palletCount);
    });

    // 일시정지
    btnPause.addEventListener('click', () => {
        RealtimeCalc.pause();
        btnPause.disabled = true;
        btnPause.style.display = 'none';
        btnResume.disabled = false;
        btnResume.style.display = '';
    });

    // 다시시작
    btnResume.addEventListener('click', () => {
        RealtimeCalc.resume();
        btnResume.disabled = true;
        btnResume.style.display = 'none';
        btnPause.disabled = false;
        btnPause.style.display = '';
    });

    // 멈춤
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
            // 실시간 모드 종료 후 결과 표시 및 차트 업데이트
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

    // --- 속도 제어 ---
    const speedSlider = document.getElementById('speed-slider');
    const speedDisplay = document.getElementById('speed-display');

    // 로그 스케일 매핑: slider(0~100) ↔ speed(0.1~100)
    function sliderToSpeed(val) {
        return Math.round(Math.pow(10, (val / 50) - 1) * 100) / 100;
    }
    function speedToSlider(speed) {
        return Math.round((Math.log10(speed) + 1) * 50);
    }
    function formatSpeed(speed) {
        if (speed >= 10) return Math.round(speed) + 'x';
        if (speed >= 1) return speed.toFixed(1) + 'x';
        return speed.toFixed(2) + 'x';
    }
    function applySpeed(speed) {
        RealtimeCalc.setSpeed(speed);
        speedDisplay.textContent = formatSpeed(speed);
        speedSlider.value = speedToSlider(speed);
    }

    // 프리셋 버튼
    speedButtons.forEach(btn => {
        btn.addEventListener('click', () => {
            const spd = parseFloat(btn.dataset.speed);
            applySpeed(spd);
            speedButtons.forEach(b => b.classList.remove('active'));
            btn.classList.add('active');
        });
    });

    // 슬라이더
    speedSlider.addEventListener('input', () => {
        const speed = sliderToSpeed(parseInt(speedSlider.value));
        RealtimeCalc.setSpeed(speed);
        speedDisplay.textContent = formatSpeed(speed);
        // 프리셋 버튼 활성 해제
        speedButtons.forEach(b => b.classList.remove('active'));
    });

    // PDF 저장
    btnPdf.addEventListener('click', () => {
        if (!lastResults || !lastMachineConfigs) {
            alert('먼저 시뮬레이션을 실행해 주세요.');
            return;
        }
        PdfExport.exportPdf(lastResults, lastMachineConfigs);
    });

    // 시나리오 저장
    btnSaveScenario.addEventListener('click', () => {
        if (!lastResults || !lastMachineConfigs) {
            alert('먼저 시뮬레이션을 실행해 주세요.');
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

    // 시나리오 모두 삭제
    btnClearScenarios.addEventListener('click', () => {
        const scenarios = ScenarioManager.getScenarios();
        if (scenarios.length === 0) {
            alert('저장된 시나리오가 없습니다.');
            return;
        }
        if (confirm(`${scenarios.length}개의 시나리오를 모두 삭제하시겠습니까?`)) {
            ScenarioManager.clearAllScenarios();
            updateScenarioCount();
        }
    });

    // 시나리오 개수 업데이트
    function updateScenarioCount() {
        const countEl = document.getElementById('scenario-count');
        if (countEl) {
            const count = ScenarioManager.getScenarios().length;
            countEl.textContent = `(${count}/${ScenarioManager.MAX_SCENARIOS})`;
        }
    }

    // 이벤트 로그 토글
    btnToggleLog.addEventListener('click', () => {
        logContainer.classList.toggle('collapsed');
    });

    // --- 메뉴얼 모달 ---
    function openManual() {
        manualModal.classList.add('active');
    }

    function closeManual() {
        manualModal.classList.remove('active');
    }

    btnManual.addEventListener('click', openManual);
    modalClose.addEventListener('click', closeManual);

    // 오버레이 클릭 시 닫기
    manualModal.addEventListener('click', (e) => {
        if (e.target === manualModal) closeManual();
    });

    // ESC 키로 닫기
    document.addEventListener('keydown', (e) => {
        if (e.key === 'Escape' && manualModal.classList.contains('active')) {
            closeManual();
        }
    });

    // --- 초기화: 기본 로봇 속도(85%) 적용 ---
    // HTML에서 active로 설정된 버튼의 속도값을 적용하고 readonly 설정
    const initialSpeedBtn = document.querySelector('.robot-speed-btn.active');
    if (initialSpeedBtn && initialSpeedBtn.dataset.robotSpeed !== 'manual') {
        currentRobotSpeed = initialSpeedBtn.dataset.robotSpeed;
        const speedValues = Config.getSpeedValues(parseInt(currentRobotSpeed));
        applySpeedValues(speedValues);
        setInputsReadonly(true);
    }
});
