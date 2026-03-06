/**
 * engine.js - Event-driven simulation engine
 * Shared by both Quick Calc and Realtime Calc for consistent results
 */

// --- Event Types ---
const EventType = {
    ROBOT_READY_AT_TABLE: 'ROBOT_READY_AT_TABLE',
    ROBOT_ARRIVED_AT_MACHINE: 'ROBOT_ARRIVED_AT_MACHINE',
    ROBOT_RETURNED_TO_TABLE: 'ROBOT_RETURNED_TO_TABLE',
    MACHINE_DONE_SEWING: 'MACHINE_DONE_SEWING',
};

// --- Robot States ---
const RobotState = {
    AT_TABLE_IDLE: 'AT_TABLE_IDLE',
    TRAVELING_TO_MACHINE: 'TRAVELING_TO_MACHINE',
    TRAVELING_TO_TABLE: 'TRAVELING_TO_TABLE',
};

// --- Machine States ---
const MachineState = {
    EMPTY: 'EMPTY',
    SEWING: 'SEWING',
    DONE_WAITING: 'DONE_WAITING',
};

/**
 * Simulation engine class
 * Event queue-based chronological processing
 */
class SimulationEngine {
    constructor(machineConfigs, palletPrepTime, palletCount) {
        this.config = machineConfigs;
        this.palletPrepTime = palletPrepTime || 0;
        this.palletCount = palletCount || 5;
        this.reset();
    }

    reset() {
        this.currentTime = 0;
        this.eventQueue = [];
        this.eventLog = [];

        // Robot state
        this.robot = {
            state: RobotState.AT_TABLE_IDLE,
            targetMachine: null,
            carryingCompleted: false,
            isInitialLoad: false,
            isUnloadOnly: false,
        };

        // Machine states
        this.machines = {};
        for (const id of Config.MACHINE_IDS) {
            this.machines[id] = {
                state: MachineState.EMPTY,
                sewingRemaining: 0,
                sewingStartTime: 0,
                doneTime: 0,
                emptyStartTime: 0,
                totalWaitTime: 0,
                produced: 0,
            };
        }

        // Stats
        this.robotBusyTime = 0;
        this.lastRobotBusyStart = 0;

        // Pallet tracking (queue system)
        const machineCount = Config.MACHINE_IDS.length;

        // Initially ready pallets = machine count (all ready at t=0)
        this.readyPalletsQueue = [];
        for (let i = 0; i < machineCount; i++) {
            this.readyPalletsQueue.push(0);
        }

        // Extra pallets = palletCount - machineCount (not yet started)
        this.extraPalletsRemaining = Math.max(0, this.palletCount - machineCount);

        this.totalPickups = 0;

        // Callbacks (used in realtime mode)
        this.onEvent = null;
        this.onStateChange = null;
    }

    /**
     * Insert value into sorted array (chronological)
     */
    _insertSorted(arr, value) {
        let inserted = false;
        for (let i = 0; i < arr.length; i++) {
            if (value < arr[i]) {
                arr.splice(i, 0, value);
                inserted = true;
                break;
            }
        }
        if (!inserted) {
            arr.push(value);
        }
    }

    /**
     * Add event to queue (maintains chronological order)
     */
    addEvent(time, type, data = {}) {
        const event = { time, type, data };
        let inserted = false;
        for (let i = 0; i < this.eventQueue.length; i++) {
            if (time < this.eventQueue[i].time) {
                this.eventQueue.splice(i, 0, event);
                inserted = true;
                break;
            }
        }
        if (!inserted) {
            this.eventQueue.push(event);
        }
    }

    /**
     * Pop and return next event
     */
    popNextEvent() {
        return this.eventQueue.shift();
    }

    /**
     * Log event
     */
    log(time, type, machineId, description) {
        this.eventLog.push({ time: Math.round(time * 100) / 100, type, machineId, description });
        if (this.onEvent) {
            this.onEvent({ time, type, machineId, description });
        }
    }

    /**
     * Initialize simulation - register first event
     */
    start() {
        this.reset();
        this.addEvent(0, EventType.ROBOT_READY_AT_TABLE);
        this.log(0, 'INIT', null, 'Simulation started - Robot at material table');
    }

    /**
     * Robot scheduling - determine next destination
     * Returns: { machineId, isInitial } or null (need to wait)
     */
    scheduleNext() {
        // Priority 1: DONE_WAITING machines (longest wait first)
        let longestWaitId = null;
        let longestWaitSince = Infinity;

        for (const id of Config.MACHINE_IDS) {
            const m = this.machines[id];
            if (m.state === MachineState.DONE_WAITING) {
                if (m.doneTime < longestWaitSince) {
                    longestWaitSince = m.doneTime;
                    longestWaitId = id;
                }
            }
        }

        if (longestWaitId !== null) {
            return { machineId: longestWaitId, isInitial: false };
        }

        // Priority 2: EMPTY machines (by number order)
        for (const id of Config.MACHINE_IDS) {
            if (this.machines[id].state === MachineState.EMPTY) {
                return { machineId: id, isInitial: true };
            }
        }

        // Priority 3: All SEWING - wait until earliest completion
        return null;
    }

    /**
     * Find earliest sewing completion time
     */
    findEarliestSewingDone() {
        let earliest = Infinity;
        for (const id of Config.MACHINE_IDS) {
            const m = this.machines[id];
            if (m.state === MachineState.SEWING) {
                const doneAt = m.sewingStartTime + this.config[id].sewingTime;
                if (doneAt < earliest) {
                    earliest = doneAt;
                }
            }
        }
        return earliest;
    }

    /**
     * Process single event
     * @returns {boolean} true if event was processed
     */
    processEvent(event) {
        this.currentTime = event.time;

        switch (event.type) {
            case EventType.ROBOT_READY_AT_TABLE:
                this._handleRobotReadyAtTable();
                break;

            case EventType.ROBOT_ARRIVED_AT_MACHINE:
                this._handleRobotArrivedAtMachine(event.data);
                break;

            case EventType.ROBOT_RETURNED_TO_TABLE:
                this._handleRobotReturnedToTable(event.data);
                break;

            case EventType.MACHINE_DONE_SEWING:
                this._handleMachineDoneSewing(event.data);
                break;
        }

        if (this.onStateChange) {
            this.onStateChange(this.getState());
        }

        return true;
    }

    /**
     * ROBOT_READY_AT_TABLE handler
     * Robot is at table, decide next action
     */
    _handleRobotReadyAtTable() {
        if (this.robot.state !== RobotState.AT_TABLE_IDLE) return;

        const schedule = this.scheduleNext();

        // Check pallet availability (queue system)
        let hasPallet = false;
        if (this.readyPalletsQueue.length > 0) {
            const nextReadyTime = this.readyPalletsQueue[0];
            if (this.currentTime >= nextReadyTime) {
                hasPallet = true;
            } else {
                if (schedule && !schedule.isInitial) {
                    this._departUnloadOnly(schedule.machineId);
                    return;
                }
                const waitTime = nextReadyTime - this.currentTime;
                this.addEvent(nextReadyTime, EventType.ROBOT_READY_AT_TABLE);
                this.log(this.currentTime, 'PALLET_WAIT', null,
                    `Pallet wait (ready in ${waitTime.toFixed(1)}s)`);
                return;
            }
        } else {
            if (schedule && !schedule.isInitial) {
                this._departUnloadOnly(schedule.machineId);
                return;
            }
            this.log(this.currentTime, 'PALLET_WAIT', null,
                `No pallet available - waiting`);
            return;
        }

        if (schedule) {
            const { machineId, isInitial } = schedule;
            const cfg = this.config[machineId];
            const travelTime = isInitial ? cfg.initLoadTime : cfg.unloadLoadTime;

            // Use pallet (remove from queue)
            this.readyPalletsQueue.shift();
            this.totalPickups++;

            // Start preparing extra pallet if available
            if (this.extraPalletsRemaining > 0) {
                this.extraPalletsRemaining--;
                const readyTime = this.currentTime + this.palletPrepTime;
                this._insertSorted(this.readyPalletsQueue, readyTime);
                this.log(this.currentTime, 'PALLET_PREP_START', null,
                    `Extra pallet prep started (ready in ${this.palletPrepTime}s)`);
            }

            this.robot.state = RobotState.TRAVELING_TO_MACHINE;
            this.robot.targetMachine = machineId;
            this.robot.isInitialLoad = isInitial;
            this.robot.isUnloadOnly = false;
            this.robot.carryingCompleted = false;
            this.lastRobotBusyStart = this.currentTime;

            this.addEvent(
                this.currentTime + travelTime,
                EventType.ROBOT_ARRIVED_AT_MACHINE,
                { machineId, isInitial, isUnloadOnly: false }
            );

            const desc = isInitial
                ? `Robot -> Machine ${machineId} (Initial Load, ${travelTime}s)`
                : `Robot -> Machine ${machineId} (UL&Load, ${travelTime}s)`;
            this.log(this.currentTime, 'ROBOT_DEPART', machineId, desc);

        } else {
            this.robot.state = RobotState.AT_TABLE_IDLE;
            const earliestDone = this.findEarliestSewingDone();
            if (earliestDone < Infinity) {
                this.log(this.currentTime, 'ROBOT_WAIT', null,
                    `Robot idle - next completion at: ${earliestDone.toFixed(1)}s`);
            }
        }
    }

    /**
     * Depart to machine in Unload-only mode
     * Used when no pallet available but DONE_WAITING machine exists
     */
    _departUnloadOnly(machineId) {
        const cfg = this.config[machineId];
        const travelTime = cfg.returnTime;

        this.robot.state = RobotState.TRAVELING_TO_MACHINE;
        this.robot.targetMachine = machineId;
        this.robot.isInitialLoad = false;
        this.robot.isUnloadOnly = true;
        this.robot.carryingCompleted = false;
        this.lastRobotBusyStart = this.currentTime;

        this.addEvent(
            this.currentTime + travelTime,
            EventType.ROBOT_ARRIVED_AT_MACHINE,
            { machineId, isInitial: false, isUnloadOnly: true }
        );

        this.log(this.currentTime, 'ROBOT_DEPART', machineId,
            `Robot -> Machine ${machineId} (Unload-only, ${travelTime}s)`);
    }

    /**
     * ROBOT_ARRIVED_AT_MACHINE handler
     * Robot arrived at machine -> unload/load complete
     */
    _handleRobotArrivedAtMachine(data) {
        const { machineId, isInitial, isUnloadOnly } = data;
        const machine = this.machines[machineId];
        const cfg = this.config[machineId];

        if (isUnloadOnly && machine.state === MachineState.DONE_WAITING) {
            const waitTime = this.currentTime - machine.doneTime;
            machine.totalWaitTime += waitTime;
            machine.produced++;
            this.robot.carryingCompleted = true;

            this.log(this.currentTime, 'UNLOAD_ONLY', machineId,
                `Machine ${machineId}: Unload completed (wait ${waitTime.toFixed(1)}s, total: ${machine.produced})`);

            machine.state = MachineState.EMPTY;
            machine.emptyStartTime = this.currentTime;

            this.robot.state = RobotState.TRAVELING_TO_TABLE;
            this.addEvent(
                this.currentTime + cfg.returnTime,
                EventType.ROBOT_RETURNED_TO_TABLE,
                { machineId, hasCompleted: true }
            );
            return;
        }

        if (!isInitial && machine.state === MachineState.DONE_WAITING) {
            const waitTime = this.currentTime - machine.doneTime;
            machine.totalWaitTime += waitTime;
            machine.produced++;
            this.robot.carryingCompleted = true;

            this.log(this.currentTime, 'UNLOAD_LOAD', machineId,
                `Machine ${machineId}: Unload + Load new (wait ${waitTime.toFixed(1)}s, total: ${machine.produced})`);
        } else if (isInitial) {
            const waitTime = this.currentTime - machine.emptyStartTime;
            machine.totalWaitTime += waitTime;
            this.robot.carryingCompleted = false;
            this.log(this.currentTime, 'INIT_LOAD', machineId,
                `Machine ${machineId}: Initial load complete (wait ${waitTime.toFixed(1)}s)`);
        }

        // Start sewing
        machine.state = MachineState.SEWING;
        machine.sewingRemaining = cfg.sewingTime;
        machine.sewingStartTime = this.currentTime;

        this.addEvent(
            this.currentTime + cfg.sewingTime,
            EventType.MACHINE_DONE_SEWING,
            { machineId }
        );

        this.log(this.currentTime, 'SEWING_START', machineId,
            `Machine ${machineId}: Sewing started (${cfg.sewingTime}s)`);

        // Robot returns
        this.robot.state = RobotState.TRAVELING_TO_TABLE;
        this.addEvent(
            this.currentTime + cfg.returnTime,
            EventType.ROBOT_RETURNED_TO_TABLE,
            { machineId, hasCompleted: this.robot.carryingCompleted }
        );
    }

    /**
     * ROBOT_RETURNED_TO_TABLE handler
     * Robot returned to table
     */
    _handleRobotReturnedToTable(data) {
        const { machineId, hasCompleted } = data;

        this.robotBusyTime += (this.currentTime - this.lastRobotBusyStart);

        if (hasCompleted) {
            this.log(this.currentTime, 'PRODUCED', machineId,
                `Machine ${machineId}: Completed product arrived at table`);

            const readyTime = this.currentTime + this.palletPrepTime;
            this._insertSorted(this.readyPalletsQueue, readyTime);
            if (this.palletPrepTime > 0) {
                this.log(this.currentTime, 'PALLET_PREP_START', null,
                    `Pallet material prep started (ready in ${this.palletPrepTime}s)`);
            }
        } else {
            this.log(this.currentTime, 'RETURN', machineId,
                `Robot returned to table (initial load - no completed product)`);
        }

        this.robot.state = RobotState.AT_TABLE_IDLE;
        this.robot.targetMachine = null;
        this.robot.carryingCompleted = false;

        this.addEvent(this.currentTime, EventType.ROBOT_READY_AT_TABLE);
    }

    /**
     * MACHINE_DONE_SEWING handler
     * Machine sewing complete
     */
    _handleMachineDoneSewing(data) {
        const { machineId } = data;
        const machine = this.machines[machineId];

        machine.state = MachineState.DONE_WAITING;
        machine.doneTime = this.currentTime;
        machine.sewingRemaining = 0;

        this.log(this.currentTime, 'SEWING_DONE', machineId,
            `Machine ${machineId}: Sewing complete - waiting for robot`);

        if (this.robot.state === RobotState.AT_TABLE_IDLE) {
            this.addEvent(this.currentTime, EventType.ROBOT_READY_AT_TABLE);
        }
    }

    /**
     * Run simulation until specified time (for Quick Calc)
     */
    runUntil(endTime) {
        this.start();

        while (this.eventQueue.length > 0) {
            const nextEvent = this.eventQueue[0];
            if (nextEvent.time > endTime) break;
            this.processEvent(this.popNextEvent());
        }

        this.currentTime = endTime;
    }

    /**
     * Advance to next event (used in Realtime mode)
     * @param {number} upToTime - process only up to this time
     * @returns {boolean} true if event was processed
     */
    advanceTo(upToTime) {
        let processed = false;
        while (this.eventQueue.length > 0) {
            const nextEvent = this.eventQueue[0];
            if (nextEvent.time > upToTime) break;
            this.processEvent(this.popNextEvent());
            processed = true;
        }
        this.currentTime = upToTime;
        return processed;
    }

    /**
     * Return current state snapshot
     */
    getState() {
        const state = {
            time: this.currentTime,
            robot: { ...this.robot },
            machines: {},
            robotBusyTime: this.robotBusyTime,
        };

        for (const id of Config.MACHINE_IDS) {
            const m = this.machines[id];
            const cfg = this.config[id];
            let progress = 0;
            if (m.state === MachineState.SEWING) {
                const elapsed = this.currentTime - m.sewingStartTime;
                progress = Math.min(1, elapsed / cfg.sewingTime);
            }

            const sewingElapsed = m.state === MachineState.SEWING
                ? this.currentTime - m.sewingStartTime : 0;

            state.machines[id] = {
                state: m.state,
                produced: m.produced,
                totalWaitTime: m.totalWaitTime,
                progress,
                sewingElapsed,
                sewingTotal: cfg.sewingTime,
                currentWaitTime: m.state === MachineState.DONE_WAITING
                    ? this.currentTime - m.doneTime
                    : m.state === MachineState.EMPTY
                        ? this.currentTime
                        : 0,
            };
        }

        // Pallet info
        const readyCount = this.readyPalletsQueue.filter(t => t <= this.currentTime).length;
        const preparingPallets = this.readyPalletsQueue.filter(t => t > this.currentTime);
        const nextPrepTime = preparingPallets.length > 0 ? Math.min(...preparingPallets) : null;
        const prepRemaining = nextPrepTime !== null ? Math.max(0, nextPrepTime - this.currentTime) : null;

        state.palletInfo = {
            readyCount,
            prepRemaining,
            isPreparing: prepRemaining !== null && prepRemaining > 0,
        };

        return state;
    }

    /**
     * Return final results
     */
    getResults() {
        const state = this.getState();
        let totalProduced = 0;

        const machineResults = {};
        let totalMachineSewingTime = 0;

        for (const id of Config.MACHINE_IDS) {
            const m = state.machines[id];
            const totalWait = m.totalWaitTime + (m.currentWaitTime || 0);

            let machineSewingTime = this.machines[id].produced * this.config[id].sewingTime;
            if (this.machines[id].state === MachineState.SEWING) {
                machineSewingTime += (this.currentTime - this.machines[id].sewingStartTime);
            }

            machineResults[id] = {
                produced: m.produced,
                totalWaitTime: Math.round(totalWait * 10) / 10,
                theoreticalMax: Config.getTheoreticalMax(this.config[id].sewingTime, this.currentTime),
                sewingTime: Math.round(machineSewingTime * 10) / 10,
            };
            totalProduced += m.produced;
            totalMachineSewingTime += machineSewingTime;
        }

        let busyTime = this.robotBusyTime;
        if (this.robot.state !== RobotState.AT_TABLE_IDLE) {
            busyTime += (this.currentTime - this.lastRobotBusyStart);
        }

        const machineCount = Config.MACHINE_IDS.length;
        const machineUtilization = this.currentTime > 0
            ? Math.round((totalMachineSewingTime / (this.currentTime * machineCount)) * 1000) / 10
            : 0;

        return {
            simTime: this.currentTime,
            totalProduced,
            robotBusyTime: Math.round(busyTime * 10) / 10,
            robotUtilization: this.currentTime > 0
                ? Math.round((busyTime / this.currentTime) * 1000) / 10
                : 0,
            totalMachineSewingTime: Math.round(totalMachineSewingTime * 10) / 10,
            machineUtilization,
            totalPickups: this.totalPickups,
            machines: machineResults,
            eventLog: this.eventLog,
        };
    }
}
