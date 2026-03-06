/**
 * realtimeCalc.js - 실시간 시뮬레이션 모듈
 * requestAnimationFrame 기반 루프 + 속도 제어 + 엔진/렌더러 연동
 * + 타임라인 seek 기능 + cargo 시각화
 */
const RealtimeCalc = (() => {
    let engine = null;
    let animationId = null;
    let isRunning = false;
    let isPaused = false;
    let speed = 1;
    let lastTimestamp = null;
    let simTime = 0;
    let maxDuration = 0; // 0 = 무제한
    let onTimeLimit = null; // 시간 초과 콜백
    let onTimeUpdate = null; // 매 프레임 시간 업데이트 콜백

    // 시뮬레이션 재생성을 위한 초기 설정 보관
    let savedMachineConfigs = null;
    let savedPalletPrepTime = 0;
    let savedPalletCount = 5;

    // 타임라인 드래그 중 여부 (tick 루프와의 충돌 방지)
    let isSeeking = false;

    // 로봇 애니메이션 보간용 상태
    let robotAnim = {
        fromPos: null,
        toPos: null,
        startTime: 0,
        duration: 0,
        active: false,
    };

    // 로봇 운반 상태: 'none' | 'new' | 'done'
    let cargoState = 'none';

    function setDuration(seconds) {
        maxDuration = seconds;
    }

    function setOnTimeLimit(callback) {
        onTimeLimit = callback;
    }

    function setOnTimeUpdate(callback) {
        onTimeUpdate = callback;
    }

    function start(machineConfigs, palletPrepTime, palletCount) {
        // 초기 설정 보관 (seekTo에서 재사용)
        savedMachineConfigs = machineConfigs;
        savedPalletPrepTime = palletPrepTime;
        savedPalletCount = palletCount;

        engine = new SimulationEngine(machineConfigs, palletPrepTime, palletCount);
        engine.start();
        simTime = 0;
        lastTimestamp = null;
        isRunning = true;
        isPaused = false;
        isSeeking = false;
        robotAnim = { fromPos: null, toPos: null, startTime: 0, duration: 0, active: false };
        cargoState = 'none';
        updateCargoDisplay();

        // 이벤트 콜백 - 로봇 이동 애니메이션 설정
        engine.onEvent = handleEngineEvent;

        Renderer.init();
        Renderer.resetView();
        Renderer.setSkipRobotPosition(true); // 실시간 모드에서는 애니메이션이 위치 제어

        animationId = requestAnimationFrame(tick);
    }

    function pause() {
        if (!isRunning || isPaused) return;
        isPaused = true;
        if (animationId) {
            cancelAnimationFrame(animationId);
            animationId = null;
        }
        lastTimestamp = null;
    }

    function resume() {
        if (!isRunning || !isPaused) return;
        isPaused = false;
        lastTimestamp = null;
        animationId = requestAnimationFrame(tick);
    }

    function stop() {
        isRunning = false;
        isPaused = false;
        isSeeking = false;
        if (animationId) {
            cancelAnimationFrame(animationId);
            animationId = null;
        }

        Renderer.setSkipRobotPosition(false); // 위치 제어 복원

        let results = null;
        if (engine) {
            // 최종 결과 표시
            results = engine.getResults();
            QuickCalc.displayResults(results, Config.readBasicSettings().workerCount);
        }
        return results;
    }

    function setSpeed(newSpeed) {
        speed = newSpeed;
    }

    function getSpeed() {
        return speed;
    }

    function isActive() {
        return isRunning;
    }

    function getSimTime() {
        return simTime;
    }

    function getMaxDuration() {
        return maxDuration;
    }

    /**
     * 타임라인 seek - 특정 시점으로 이동
     * @param {number} targetTime - 이동할 시뮬레이션 시간 (초)
     */
    function seekTo(targetTime) {
        if (!isRunning || !savedMachineConfigs) return;

        // 범위 제한
        targetTime = Math.max(0, Math.min(targetTime, maxDuration > 0 ? maxDuration : Infinity));

        const wasPlaying = !isPaused;

        // 애니메이션 루프 중지
        if (animationId) {
            cancelAnimationFrame(animationId);
            animationId = null;
        }

        if (targetTime < simTime) {
            // 뒤로 되감기: 엔진을 새로 생성하고 targetTime까지 실행
            engine = new SimulationEngine(savedMachineConfigs, savedPalletPrepTime, savedPalletCount);
            engine.start();
            engine.onEvent = handleEngineEvent;

            // 이벤트 콜백을 일시적으로 비활성화 (seek 중 애니메이션 트리거 방지)
            const origCallback = engine.onEvent;
            engine.onEvent = null;

            // 빠른 실행으로 targetTime까지 진행
            while (engine.eventQueue.length > 0) {
                const nextEvent = engine.eventQueue[0];
                if (nextEvent.time > targetTime) break;
                engine.processEvent(engine.popNextEvent());
            }
            engine.currentTime = targetTime;

            // 콜백 복원
            engine.onEvent = origCallback;
        } else {
            // 앞으로 건너뛰기
            const origCallback = engine.onEvent;
            engine.onEvent = null;
            engine.advanceTo(targetTime);
            engine.onEvent = origCallback;
        }

        simTime = targetTime;

        // 로봇 애니메이션 리셋 (seek 후 보간 없이 현재 위치로 스냅)
        robotAnim.active = false;

        // 엔진 상태에서 cargo 상태 유추
        if (engine.robot.state === RobotState.TRAVELING_TO_MACHINE) {
            cargoState = engine.robot.isUnloadOnly ? 'none' : 'new';
        } else if (engine.robot.state === RobotState.TRAVELING_TO_TABLE) {
            cargoState = engine.robot.carryingCompleted ? 'done' : 'none';
        } else {
            cargoState = 'none';
        }
        updateCargoDisplay();

        // 렌더러 상태 업데이트
        const state = engine.getState();
        Renderer.updateState(state);
        updateRobotStats(state);

        // 로봇 위치 스냅
        snapRobotPosition();

        // 타임라인 업데이트
        if (onTimeUpdate) {
            onTimeUpdate(simTime);
        }

        // 재생 중이었으면 다시 시작
        lastTimestamp = null;
        if (wasPlaying) {
            isPaused = false;
            animationId = requestAnimationFrame(tick);
        }
    }

    /**
     * 로봇을 현재 엔진 상태에 맞는 위치로 스냅
     */
    function snapRobotPosition() {
        const robotNode = document.getElementById('node-robot');
        const positions = Renderer.POSITIONS;

        switch (engine.robot.state) {
            case RobotState.AT_TABLE_IDLE:
                robotNode.style.left = positions.table.x + '%';
                robotNode.style.top = positions.table.y + '%';
                break;
            case RobotState.TRAVELING_TO_MACHINE:
            case RobotState.TRAVELING_TO_TABLE: {
                // 이동 중이면 목적지 근처에 배치
                const target = engine.robot.state === RobotState.TRAVELING_TO_MACHINE
                    ? positions[engine.robot.targetMachine]
                    : positions.table;
                const from = engine.robot.state === RobotState.TRAVELING_TO_MACHINE
                    ? positions.table
                    : positions[engine.robot.targetMachine];
                // 중간 지점에 배치
                const x = from.x + (target.x - from.x) * 0.5;
                const y = from.y + (target.y - from.y) * 0.5;
                robotNode.style.left = x + '%';
                robotNode.style.top = y + '%';
                break;
            }
        }
    }

    /**
     * 로봇 통계 실시간 업데이트 (seek 후에도 사용)
     */
    function updateRobotStats(state) {
        let busyTime = state.robotBusyTime;
        if (state.robot.state !== RobotState.AT_TABLE_IDLE) {
            busyTime += (simTime - engine.lastRobotBusyStart);
        }
        const rtBusyEl = document.getElementById('rt-robot-busy');
        if (rtBusyEl) rtBusyEl.textContent = busyTime.toFixed(1);

        const idleTime = simTime - busyTime;
        const rtIdleEl = document.getElementById('rt-robot-idle');
        if (rtIdleEl) rtIdleEl.textContent = idleTime.toFixed(1);
    }

    /**
     * 엔진 이벤트 핸들러 - 로봇 애니메이션 트리거
     */
    function handleEngineEvent(event) {
        const positions = Renderer.POSITIONS;

        switch (event.type) {
            case 'ROBOT_DEPART': {
                // 로봇이 테이블에서 재봉기로 출발
                const machineId = event.machineId;
                const isInitial = event.description.includes('초기 적재');
                const isUnloadOnly = event.description.includes('Unload-only');
                const duration = isUnloadOnly
                    ? engine.config[machineId].returnTime
                    : isInitial
                        ? engine.config[machineId].initLoadTime
                        : engine.config[machineId].unloadLoadTime;
                robotAnim = {
                    fromPos: positions.table,
                    toPos: positions[machineId],
                    startTime: event.time,
                    duration: duration,
                    active: true,
                };
                // Cargo 상태: Unload-only는 빈 손, 나머지는 새 pallet
                cargoState = isUnloadOnly ? 'none' : 'new';
                updateCargoDisplay();
                break;
            }
            case 'UNLOAD_LOAD':
            case 'UNLOAD_ONLY': {
                // 재봉기에서 완료품 회수 → 테이블로 복귀
                const machineId = event.machineId;
                robotAnim = {
                    fromPos: positions[machineId],
                    toPos: positions.table,
                    startTime: event.time,
                    duration: engine.config[machineId].returnTime,
                    active: true,
                };
                cargoState = 'done';
                updateCargoDisplay();
                break;
            }
            case 'INIT_LOAD': {
                // 초기 적재 완료 → 빈 손으로 복귀
                const machineId = event.machineId;
                robotAnim = {
                    fromPos: positions[machineId],
                    toPos: positions.table,
                    startTime: event.time,
                    duration: engine.config[machineId].returnTime,
                    active: true,
                };
                cargoState = 'none';
                updateCargoDisplay();
                break;
            }
            case 'RETURN':
            case 'PRODUCED': {
                // 로봇 테이블 도착
                robotAnim.active = false;
                cargoState = 'none';
                updateCargoDisplay();
                break;
            }
        }
    }

    /**
     * Cargo 인디케이터 DOM 업데이트
     */
    function updateCargoDisplay() {
        const cargoEl = document.getElementById('robot-cargo');
        const iconEl = document.getElementById('cargo-icon');
        if (!cargoEl || !iconEl) return;

        if (cargoState === 'none') {
            cargoEl.style.display = 'none';
        } else {
            cargoEl.style.display = '';
            iconEl.className = 'cargo-icon';
            if (cargoState === 'new') {
                iconEl.classList.add('carrying-new');
                iconEl.textContent = 'P';
            } else if (cargoState === 'done') {
                iconEl.classList.add('carrying-done');
                iconEl.textContent = '\u2713';
            }
        }
    }

    /**
     * 애니메이션 프레임 루프
     */
    function tick(timestamp) {
        if (!isRunning || isSeeking) return;

        if (lastTimestamp === null) {
            lastTimestamp = timestamp;
        }

        const realDeltaMs = timestamp - lastTimestamp;
        lastTimestamp = timestamp;

        // 시뮬레이션 시간 진행 (속도 배수 적용)
        const simDelta = (realDeltaMs / 1000) * speed;
        simTime += simDelta;

        // 시간 제한 체크
        if (maxDuration > 0 && simTime >= maxDuration) {
            simTime = maxDuration;
            engine.advanceTo(simTime);
            // 최종 상태 렌더링
            const state = engine.getState();
            Renderer.updateState(state);
            Renderer.updateElapsedTime(simTime);
            updateRobotStats(state);
            // 타임라인 업데이트
            if (onTimeUpdate) onTimeUpdate(simTime);
            // 자동 정지
            if (onTimeLimit) {
                onTimeLimit();
            }
            return;
        }

        // 엔진 이벤트 처리
        engine.advanceTo(simTime);

        // 상태 가져오기
        const state = engine.getState();

        // 렌더러 업데이트 (재봉기 상태, 통계, 로봇 상태 텍스트)
        Renderer.updateState(state);

        // 로봇 통계 실시간 표시
        updateRobotStats(state);

        // 타임라인 업데이트 콜백
        if (onTimeUpdate) {
            onTimeUpdate(simTime);
        }

        // 로봇 애니메이션 보간 (렌더러 이후 실행하여 위치 덮어쓰기 방지)
        const robotNode = document.getElementById('node-robot');
        const statusRobot = document.getElementById('status-robot');

        if (robotAnim.active && robotAnim.fromPos && robotAnim.toPos) {
            const elapsed = simTime - robotAnim.startTime;
            let progress = Math.min(1, Math.max(0, elapsed / robotAnim.duration));

            // 이동 경과시간 표시
            if (statusRobot) {
                const travelElapsed = Math.max(0, elapsed).toFixed(1);
                const statusText = statusRobot.textContent.replace(/ \([\d.]+초\)$/, '');
                statusRobot.textContent = statusText + ` (${travelElapsed}초)`;
            }

            progress = easeInOutCubic(progress);

            const x = robotAnim.fromPos.x + (robotAnim.toPos.x - robotAnim.fromPos.x) * progress;
            const y = robotAnim.fromPos.y + (robotAnim.toPos.y - robotAnim.fromPos.y) * progress;
            robotNode.style.left = x + '%';
            robotNode.style.top = y + '%';

            if (progress >= 1) {
                robotAnim.active = false;
            }
        } else {
            // 애니메이션 비활성 → 로봇을 자재테이블에 배치
            const positions = Renderer.POSITIONS;
            robotNode.style.left = positions.table.x + '%';
            robotNode.style.top = positions.table.y + '%';
        }

        animationId = requestAnimationFrame(tick);
    }

    /**
     * easing 함수 - 부드러운 이동
     */
    function easeInOutCubic(t) {
        return t < 0.5
            ? 4 * t * t * t
            : 1 - Math.pow(-2 * t + 2, 3) / 2;
    }

    return {
        start, stop, pause, resume,
        setSpeed, getSpeed, isActive,
        setDuration, setOnTimeLimit, setOnTimeUpdate,
        seekTo, getSimTime, getMaxDuration,
    };
})();
