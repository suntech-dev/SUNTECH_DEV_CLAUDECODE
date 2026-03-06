/**
 * renderer.js - DOM-based visualization renderer
 * Robot movement animation, machine state colors, stats update
 * Dynamic support for 2/3 machine modes
 */
const Renderer = (() => {
    const POSITIONS = {
        table: { x: 50, y: 82 },
        robot_center: { x: 50, y: 50 },
        1: { x: 12, y: 50 },
        2: { x: 50, y: 22 },
        3: { x: 88, y: 50 },
    };

    let robotEl = null;

    /**
     * Update positions based on machine count
     * 2-machine mode: M2 moves to right (88%, 50%)
     * 3-machine mode: M2 at top (50%, 22%)
     */
    function setMachineCount(count) {
        if (count === 2) {
            POSITIONS[2] = { x: 88, y: 50 };
            delete POSITIONS[3];
        } else {
            POSITIONS[2] = { x: 50, y: 22 };
            POSITIONS[3] = { x: 88, y: 50 };
        }
    }

    function init() {
        robotEl = document.getElementById('node-robot');
    }

    let skipRobotPosition = false;

    function setSkipRobotPosition(val) {
        skipRobotPosition = val;
    }

    /**
     * Update full state
     */
    function updateState(state) {
        updateElapsedTime(state.time);
        updateRobotStatus(state.robot);
        if (!skipRobotPosition) {
            updateRobotPosition(state.robot);
        }
        updateMachines(state.machines, state.time);
        updateRealtimeStats(state.machines);
        if (state.palletInfo) {
            updatePalletInfo(state.palletInfo);
        }
    }

    /**
     * Update pallet info
     */
    function updatePalletInfo(palletInfo) {
        const readyCountEl = document.getElementById('pallet-ready-count');
        const timerEl = document.getElementById('pallet-timer');
        const remainingEl = document.getElementById('pallet-prep-remaining');

        if (readyCountEl) {
            readyCountEl.textContent = palletInfo.readyCount;
        }

        if (timerEl && remainingEl) {
            if (palletInfo.isPreparing && palletInfo.prepRemaining !== null) {
                timerEl.style.display = 'inline';
                remainingEl.textContent = palletInfo.prepRemaining.toFixed(1);
            } else {
                timerEl.style.display = 'none';
            }
        }
    }

    /**
     * Display elapsed time
     */
    function updateElapsedTime(time) {
        const el = document.getElementById('sim-elapsed');
        if (time < 60) {
            el.textContent = time.toFixed(1) + 's';
        } else if (time < 3600) {
            const min = Math.floor(time / 60);
            const sec = (time % 60).toFixed(0);
            el.textContent = `${min}min ${sec}s`;
        } else {
            const hr = Math.floor(time / 3600);
            const min = Math.floor((time % 3600) / 60);
            const sec = Math.floor(time % 60);
            el.textContent = `${hr}hr ${min}min ${sec}s`;
        }
    }

    /**
     * Update robot status text only (no position change)
     */
    function updateRobotStatus(robot) {
        const statusEl = document.getElementById('status-robot');
        const robotNode = document.getElementById('node-robot');

        switch (robot.state) {
            case RobotState.AT_TABLE_IDLE:
                statusEl.textContent = 'Idle';
                robotNode.classList.remove('traveling');
                break;
            case RobotState.TRAVELING_TO_MACHINE:
                statusEl.textContent = `-> M${robot.targetMachine}`;
                robotNode.classList.add('traveling');
                break;
            case RobotState.TRAVELING_TO_TABLE:
                statusEl.textContent = `<- Table`;
                robotNode.classList.add('traveling');
                break;
        }
    }

    /**
     * Update robot position (for quick calc / default mode)
     */
    function updateRobotPosition(robot) {
        switch (robot.state) {
            case RobotState.AT_TABLE_IDLE:
                moveRobotTo(POSITIONS.table);
                break;
            case RobotState.TRAVELING_TO_MACHINE:
                moveRobotToward(POSITIONS.table, POSITIONS[robot.targetMachine], 0.7);
                break;
            case RobotState.TRAVELING_TO_TABLE:
                moveRobotToward(POSITIONS[robot.targetMachine], POSITIONS.table, 0.7);
                break;
        }
    }

    /**
     * Move robot to specific position
     */
    function moveRobotTo(pos) {
        if (!robotEl) return;
        robotEl.style.left = pos.x + '%';
        robotEl.style.top = pos.y + '%';
    }

    /**
     * Move robot to interpolated position between two points
     */
    function moveRobotToward(from, to, progress) {
        if (!robotEl) return;
        const x = from.x + (to.x - from.x) * progress;
        const y = from.y + (to.y - from.y) * progress;
        robotEl.style.left = x + '%';
        robotEl.style.top = y + '%';
    }

    /**
     * Update machine states
     */
    function updateMachines(machines) {
        for (const id of Config.MACHINE_IDS) {
            const m = machines[id];
            const nodeEl = document.getElementById(`node-m${id}`);
            const statusEl = document.getElementById(`status-m${id}`);
            const progressEl = document.getElementById(`progress-m${id}`);

            nodeEl.className = 'sim-node machine-node';
            switch (m.state) {
                case MachineState.EMPTY:
                    nodeEl.classList.add('state-empty');
                    if (m.currentWaitTime > 0) {
                        statusEl.textContent = `Wait ${m.currentWaitTime.toFixed(0)}s`;
                    } else {
                        statusEl.textContent = 'Idle';
                    }
                    progressEl.style.width = '0%';
                    break;

                case MachineState.SEWING:
                    nodeEl.classList.add('state-sewing');
                    statusEl.innerHTML = `Sewing ${Math.round(m.progress * 100)}%<br><span style="font-size:0.85em;opacity:0.8">${m.sewingElapsed.toFixed(1)} / ${m.sewingTotal.toFixed(1)}s</span>`;
                    progressEl.style.width = (m.progress * 100) + '%';
                    progressEl.style.background = '';
                    break;

                case MachineState.DONE_WAITING:
                    nodeEl.classList.add('state-done');
                    statusEl.textContent = `Done Wait ${m.currentWaitTime.toFixed(0)}s`;
                    progressEl.style.width = '100%';
                    progressEl.style.background = '#f59e0b';
                    break;
            }
        }
    }

    /**
     * Update realtime stats bar
     */
    function updateRealtimeStats(machines) {
        for (const id of Config.MACHINE_IDS) {
            const m = machines[id];
            const totalWait = m.totalWaitTime + (m.currentWaitTime || 0);
            document.getElementById(`rt-m${id}-prod`).textContent = m.produced;
            document.getElementById(`rt-m${id}-wait`).textContent = totalWait.toFixed(1);
        }
    }

    /**
     * Reset simulation view to initial state
     */
    function resetView() {
        for (const id of Config.MACHINE_IDS) {
            const nodeEl = document.getElementById(`node-m${id}`);
            nodeEl.className = 'sim-node machine-node state-empty';
            document.getElementById(`status-m${id}`).textContent = 'Idle';
            document.getElementById(`progress-m${id}`).style.width = '0%';
            document.getElementById(`progress-m${id}`).style.background = '';
            document.getElementById(`rt-m${id}-prod`).textContent = '0';
            document.getElementById(`rt-m${id}-wait`).textContent = '0.0';
        }
        document.getElementById('status-robot').textContent = 'Idle';
        document.getElementById('sim-elapsed').textContent = '0.0s';
        moveRobotTo(POSITIONS.table);

        const readyCountEl = document.getElementById('pallet-ready-count');
        const timerEl = document.getElementById('pallet-timer');
        if (readyCountEl) readyCountEl.textContent = '0';
        if (timerEl) timerEl.style.display = 'none';
    }

    return { init, updateState, resetView, setSkipRobotPosition, POSITIONS, setMachineCount };
})();
