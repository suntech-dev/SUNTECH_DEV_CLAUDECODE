/**
 * realtimeCalc.js - Realtime simulation module
 * requestAnimationFrame-based loop + speed control + engine/renderer integration
 * + timeline seek + cargo visualization
 */
const RealtimeCalc = (() => {
    let engine = null;
    let animationId = null;
    let isRunning = false;
    let isPaused = false;
    let speed = 1;
    let lastTimestamp = null;
    let simTime = 0;
    let maxDuration = 0; // 0 = unlimited
    let onTimeLimit = null; // time limit callback
    let onTimeUpdate = null; // per-frame time update callback

    // Saved initial settings for re-simulation on seek
    let savedMachineConfigs = null;
    let savedPalletPrepTime = 0;
    let savedPalletCount = 5;

    // Timeline drag flag (prevent tick loop conflict)
    let isSeeking = false;

    // Robot animation interpolation state
    let robotAnim = {
        fromPos: null,
        toPos: null,
        startTime: 0,
        duration: 0,
        active: false,
    };

    // Robot cargo state: 'none' | 'new' | 'done'
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
        // Save initial settings (reused by seekTo)
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

        // Event callback - robot animation setup
        engine.onEvent = handleEngineEvent;

        Renderer.init();
        Renderer.resetView();
        Renderer.setSkipRobotPosition(true); // animation controls position in realtime mode

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

        Renderer.setSkipRobotPosition(false); // restore position control

        let results = null;
        if (engine) {
            // Display final results
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
     * Timeline seek - move to a specific time
     * @param {number} targetTime - target simulation time (seconds)
     */
    function seekTo(targetTime) {
        if (!isRunning || !savedMachineConfigs) return;

        // Clamp range
        targetTime = Math.max(0, Math.min(targetTime, maxDuration > 0 ? maxDuration : Infinity));

        const wasPlaying = !isPaused;

        // Stop animation loop
        if (animationId) {
            cancelAnimationFrame(animationId);
            animationId = null;
        }

        if (targetTime < simTime) {
            // Rewind: create new engine and run to targetTime
            engine = new SimulationEngine(savedMachineConfigs, savedPalletPrepTime, savedPalletCount);
            engine.start();
            engine.onEvent = handleEngineEvent;

            // Temporarily disable event callback (prevent animation triggers during seek)
            const origCallback = engine.onEvent;
            engine.onEvent = null;

            // Fast-forward to targetTime
            while (engine.eventQueue.length > 0) {
                const nextEvent = engine.eventQueue[0];
                if (nextEvent.time > targetTime) break;
                engine.processEvent(engine.popNextEvent());
            }
            engine.currentTime = targetTime;

            // Restore callback
            engine.onEvent = origCallback;
        } else {
            // Forward skip
            const origCallback = engine.onEvent;
            engine.onEvent = null;
            engine.advanceTo(targetTime);
            engine.onEvent = origCallback;
        }

        simTime = targetTime;

        // Reset robot animation (snap to current position without interpolation)
        robotAnim.active = false;

        // Infer cargo state from engine state
        if (engine.robot.state === RobotState.TRAVELING_TO_MACHINE) {
            cargoState = engine.robot.isUnloadOnly ? 'none' : 'new';
        } else if (engine.robot.state === RobotState.TRAVELING_TO_TABLE) {
            cargoState = engine.robot.carryingCompleted ? 'done' : 'none';
        } else {
            cargoState = 'none';
        }
        updateCargoDisplay();

        // Update renderer state
        const state = engine.getState();
        Renderer.updateState(state);
        updateRobotStats(state);

        // Snap robot position
        snapRobotPosition();

        // Timeline update
        if (onTimeUpdate) {
            onTimeUpdate(simTime);
        }

        // Resume if was playing
        lastTimestamp = null;
        if (wasPlaying) {
            isPaused = false;
            animationId = requestAnimationFrame(tick);
        }
    }

    /**
     * Snap robot to correct position based on engine state
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
                // If traveling, place at midpoint
                const target = engine.robot.state === RobotState.TRAVELING_TO_MACHINE
                    ? positions[engine.robot.targetMachine]
                    : positions.table;
                const from = engine.robot.state === RobotState.TRAVELING_TO_MACHINE
                    ? positions.table
                    : positions[engine.robot.targetMachine];
                // Place at midpoint
                const x = from.x + (target.x - from.x) * 0.5;
                const y = from.y + (target.y - from.y) * 0.5;
                robotNode.style.left = x + '%';
                robotNode.style.top = y + '%';
                break;
            }
        }
    }

    /**
     * Update robot stats in realtime (also used after seek)
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
     * Engine event handler - trigger robot animation
     */
    function handleEngineEvent(event) {
        const positions = Renderer.POSITIONS;

        switch (event.type) {
            case 'ROBOT_DEPART': {
                // Robot departing from table to machine
                const machineId = event.machineId;
                const isInitial = event.description.includes('Initial Load');
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
                // Cargo state: Unload-only = empty hands, others = new pallet
                cargoState = isUnloadOnly ? 'none' : 'new';
                updateCargoDisplay();
                break;
            }
            case 'UNLOAD_LOAD':
            case 'UNLOAD_ONLY': {
                // Retrieved completed item from machine -> return to table
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
                // Initial load complete -> return empty-handed
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
                // Robot arrived at table
                robotAnim.active = false;
                cargoState = 'none';
                updateCargoDisplay();
                break;
            }
        }
    }

    /**
     * Cargo indicator DOM update
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
     * Animation frame loop
     */
    function tick(timestamp) {
        if (!isRunning || isSeeking) return;

        if (lastTimestamp === null) {
            lastTimestamp = timestamp;
        }

        const realDeltaMs = timestamp - lastTimestamp;
        lastTimestamp = timestamp;

        // Advance simulation time (apply speed multiplier)
        const simDelta = (realDeltaMs / 1000) * speed;
        simTime += simDelta;

        // Time limit check
        if (maxDuration > 0 && simTime >= maxDuration) {
            simTime = maxDuration;
            engine.advanceTo(simTime);
            // Final state render
            const state = engine.getState();
            Renderer.updateState(state);
            Renderer.updateElapsedTime(simTime);
            updateRobotStats(state);
            // Timeline update
            if (onTimeUpdate) onTimeUpdate(simTime);
            // Auto stop
            if (onTimeLimit) {
                onTimeLimit();
            }
            return;
        }

        // Process engine events
        engine.advanceTo(simTime);

        // Get state
        const state = engine.getState();

        // Update renderer (machine states, stats, robot status text)
        Renderer.updateState(state);

        // Realtime robot stats
        updateRobotStats(state);

        // Timeline update callback
        if (onTimeUpdate) {
            onTimeUpdate(simTime);
        }

        // Robot animation interpolation (run after renderer to override position)
        const robotNode = document.getElementById('node-robot');
        const statusRobot = document.getElementById('status-robot');

        if (robotAnim.active && robotAnim.fromPos && robotAnim.toPos) {
            const elapsed = simTime - robotAnim.startTime;
            let progress = Math.min(1, Math.max(0, elapsed / robotAnim.duration));

            // Show travel elapsed time
            if (statusRobot) {
                const travelElapsed = Math.max(0, elapsed).toFixed(1);
                const statusText = statusRobot.textContent.replace(/ \([\d.]+s\)$/, '');
                statusRobot.textContent = statusText + ` (${travelElapsed}s)`;
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
            // Animation inactive -> place robot at material table
            const positions = Renderer.POSITIONS;
            robotNode.style.left = positions.table.x + '%';
            robotNode.style.top = positions.table.y + '%';
        }

        animationId = requestAnimationFrame(tick);
    }

    /**
     * Easing function - smooth movement
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
