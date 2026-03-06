/**
 * engine.js - 이벤트 기반 시뮬레이션 엔진
 * 빠른계산과 실시간계산 모두 이 엔진을 공유하여 결과 일관성 보장
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
 * 시뮬레이션 엔진 클래스
 * 이벤트 큐 기반으로 시간순 처리
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
            carryingCompleted: false,  // 완료품을 가지고 있는지
            isInitialLoad: false,      // 초기 적재 중인지
            isUnloadOnly: false,       // Unload-only 모드인지
        };

        // Machine states
        this.machines = {};
        for (const id of Config.MACHINE_IDS) {
            this.machines[id] = {
                state: MachineState.EMPTY,
                sewingRemaining: 0,
                sewingStartTime: 0,
                doneTime: 0,           // 재봉 완료 시점 (대기시간 계산용)
                emptyStartTime: 0,     // EMPTY 상태 시작 시점 (초기 적재 대기시간 계산용)
                totalWaitTime: 0,
                produced: 0,
            };
        }

        // Stats
        this.robotBusyTime = 0;
        this.lastRobotBusyStart = 0;

        // Pallet 준비 상태 추적 (새로운 큐 시스템)
        const machineCount = Config.MACHINE_IDS.length;

        // 초기 준비된 pallet = 재봉기 수량 (모두 t=0에 준비 완료)
        this.readyPalletsQueue = [];
        for (let i = 0; i < machineCount; i++) {
            this.readyPalletsQueue.push(0);  // ready at t=0
        }

        // 추가 pallet = palletCount - machineCount (아직 준비 시작 안함)
        this.extraPalletsRemaining = Math.max(0, this.palletCount - machineCount);

        this.totalPickups = 0;         // 테이블에서 pallet pick up 횟수

        // 콜백 (실시간 모드에서 사용)
        this.onEvent = null;
        this.onStateChange = null;
    }

    /**
     * 정렬된 배열에 값 삽입 (시간순)
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
     * 이벤트 큐에 이벤트 추가 (시간순 정렬 유지)
     */
    addEvent(time, type, data = {}) {
        const event = { time, type, data };
        // 삽입 정렬 (이벤트 수가 적으므로 충분)
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
     * 다음 이벤트를 꺼내서 처리
     */
    popNextEvent() {
        return this.eventQueue.shift();
    }

    /**
     * 이벤트 로그에 기록
     */
    log(time, type, machineId, description) {
        this.eventLog.push({ time: Math.round(time * 100) / 100, type, machineId, description });
        if (this.onEvent) {
            this.onEvent({ time, type, machineId, description });
        }
    }

    /**
     * 시뮬레이션 초기화 - 첫 번째 이벤트 등록
     */
    start() {
        this.reset();
        // 로봇이 테이블에서 시작
        this.addEvent(0, EventType.ROBOT_READY_AT_TABLE);
        this.log(0, 'INIT', null, '시뮬레이션 시작 - 로봇 자재테이블 대기');
    }

    /**
     * 로봇 스케줄링 - 다음 목적지 결정
     * 반환: { machineId, isInitial } 또는 null (대기 필요)
     */
    scheduleNext() {
        // 1순위: DONE_WAITING 상태인 재봉기 (가장 오래 기다린 순)
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

        // 2순위: EMPTY 상태인 재봉기 (번호순)
        for (const id of Config.MACHINE_IDS) {
            if (this.machines[id].state === MachineState.EMPTY) {
                return { machineId: id, isInitial: true };
            }
        }

        // 3순위: 모두 SEWING 중 → 가장 먼저 끝날 재봉기 시점까지 대기
        return null;
    }

    /**
     * 가장 먼저 끝날 재봉기의 완료 시점 찾기
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
     * 단일 이벤트 처리
     * @returns {boolean} 이벤트를 처리했으면 true
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
     * ROBOT_READY_AT_TABLE 처리
     * 로봇이 테이블에 있고, 다음 작업 결정
     */
    _handleRobotReadyAtTable() {
        // 이미 출발한 경우 중복 이벤트 무시
        if (this.robot.state !== RobotState.AT_TABLE_IDLE) return;

        const schedule = this.scheduleNext();

        // Pallet 준비 여부 확인 (큐 시스템)
        let hasPallet = false;
        if (this.readyPalletsQueue.length > 0) {
            const nextReadyTime = this.readyPalletsQueue[0];
            if (this.currentTime >= nextReadyTime) {
                hasPallet = true;
            } else {
                // Pallet 아직 준비 안됨
                // DONE_WAITING 재봉기가 있으면 Unload-only 모드로 갈 수 있음
                if (schedule && !schedule.isInitial) {
                    // Unload-only 모드: pallet 없이 가서 unload만 수행
                    this._departUnloadOnly(schedule.machineId);
                    return;
                }
                // 그 외에는 pallet 준비 대기
                const waitTime = nextReadyTime - this.currentTime;
                this.addEvent(nextReadyTime, EventType.ROBOT_READY_AT_TABLE);
                this.log(this.currentTime, 'PALLET_WAIT', null,
                    `Pallet 준비 대기 (${waitTime.toFixed(1)}초 후 준비 완료)`);
                return;
            }
        } else {
            // 준비된 pallet이 없음 (모두 재봉기에 있거나 준비 중)
            // DONE_WAITING 재봉기가 있으면 Unload-only 모드로 갈 수 있음
            if (schedule && !schedule.isInitial) {
                this._departUnloadOnly(schedule.machineId);
                return;
            }
            // 그 외에는 다음 pallet 도착 대기
            this.log(this.currentTime, 'PALLET_WAIT', null,
                `사용 가능한 Pallet 없음 - 대기`);
            return;
        }

        if (schedule) {
            const { machineId, isInitial } = schedule;
            const cfg = this.config[machineId];
            const travelTime = isInitial ? cfg.initLoadTime : cfg.unloadLoadTime;

            // Pallet 사용 (큐에서 제거)
            this.readyPalletsQueue.shift();
            this.totalPickups++;

            // 추가 pallet이 남아있으면 준비 시작
            if (this.extraPalletsRemaining > 0) {
                this.extraPalletsRemaining--;
                const readyTime = this.currentTime + this.palletPrepTime;
                this._insertSorted(this.readyPalletsQueue, readyTime);
                this.log(this.currentTime, 'PALLET_PREP_START', null,
                    `추가 Pallet 준비 시작 (${this.palletPrepTime}초 후 완료)`);
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
                ? `로봇 → 재봉기 ${machineId} (초기 적재, ${travelTime}초)`
                : `로봇 → 재봉기 ${machineId} (UL&Load, ${travelTime}초)`;
            this.log(this.currentTime, 'ROBOT_DEPART', machineId, desc);

        } else {
            // 모두 재봉 중 → 가장 먼저 끝날 시점에 재확인
            this.robot.state = RobotState.AT_TABLE_IDLE;
            const earliestDone = this.findEarliestSewingDone();
            if (earliestDone < Infinity) {
                // MACHINE_DONE_SEWING 이벤트가 이미 큐에 있을 수 있으므로
                // 그 이벤트 처리 후 ROBOT_READY_AT_TABLE이 다시 트리거됨
                this.log(this.currentTime, 'ROBOT_WAIT', null,
                    `로봇 대기 - 다음 완료 예정: ${earliestDone.toFixed(1)}초`);
            }
        }
    }

    /**
     * Unload-only 모드로 재봉기로 출발
     * Pallet이 없지만 DONE_WAITING 재봉기가 있을 때 사용
     */
    _departUnloadOnly(machineId) {
        const cfg = this.config[machineId];
        // Unload-only: returnTime을 사용 (빈 손으로 가는 시간)
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
            `로봇 → 재봉기 ${machineId} (Unload-only, ${travelTime}초)`);
    }

    /**
     * ROBOT_ARRIVED_AT_MACHINE 처리
     * 로봇이 재봉기에 도착 → unload/load 완료
     */
    _handleRobotArrivedAtMachine(data) {
        const { machineId, isInitial, isUnloadOnly } = data;
        const machine = this.machines[machineId];
        const cfg = this.config[machineId];

        if (isUnloadOnly && machine.state === MachineState.DONE_WAITING) {
            // Unload-only 모드: unload만 수행, load 없음
            const waitTime = this.currentTime - machine.doneTime;
            machine.totalWaitTime += waitTime;
            machine.produced++;
            this.robot.carryingCompleted = true;

            this.log(this.currentTime, 'UNLOAD_ONLY', machineId,
                `재봉기 ${machineId}: 완료품 unload (대기 ${waitTime.toFixed(1)}초, 누적: ${machine.produced}개)`);

            // 재봉기는 EMPTY 상태로 (새 작업물 load 안함)
            machine.state = MachineState.EMPTY;
            machine.emptyStartTime = this.currentTime;  // EMPTY 시작 시점 기록

            // 로봇 복귀
            this.robot.state = RobotState.TRAVELING_TO_TABLE;
            this.addEvent(
                this.currentTime + cfg.returnTime,
                EventType.ROBOT_RETURNED_TO_TABLE,
                { machineId, hasCompleted: true }
            );
            return;
        }

        if (!isInitial && machine.state === MachineState.DONE_WAITING) {
            // 대기시간 계산 (재봉 완료 시점부터 지금까지)
            const waitTime = this.currentTime - machine.doneTime;
            machine.totalWaitTime += waitTime;
            machine.produced++;  // unload 시점에 생산 카운트
            this.robot.carryingCompleted = true;

            this.log(this.currentTime, 'UNLOAD_LOAD', machineId,
                `재봉기 ${machineId}: 완료품 unload + 새 작업물 load (대기 ${waitTime.toFixed(1)}초, 누적: ${machine.produced}개)`);
        } else if (isInitial) {
            // 초기 적재 대기시간: EMPTY 상태가 된 시점부터 현재까지
            // (최초 적재: emptyStartTime=0, Unload-only 후 재적재: emptyStartTime=unload 시점)
            const waitTime = this.currentTime - machine.emptyStartTime;
            machine.totalWaitTime += waitTime;
            this.robot.carryingCompleted = false;
            this.log(this.currentTime, 'INIT_LOAD', machineId,
                `재봉기 ${machineId}: 초기 적재 완료 (대기 ${waitTime.toFixed(1)}초)`);
        }

        // 재봉기 시작
        machine.state = MachineState.SEWING;
        machine.sewingRemaining = cfg.sewingTime;
        machine.sewingStartTime = this.currentTime;

        // 재봉 완료 이벤트 등록
        this.addEvent(
            this.currentTime + cfg.sewingTime,
            EventType.MACHINE_DONE_SEWING,
            { machineId }
        );

        this.log(this.currentTime, 'SEWING_START', machineId,
            `재봉기 ${machineId}: 재봉 시작 (${cfg.sewingTime}초)`);

        // 로봇 복귀
        this.robot.state = RobotState.TRAVELING_TO_TABLE;
        this.addEvent(
            this.currentTime + cfg.returnTime,
            EventType.ROBOT_RETURNED_TO_TABLE,
            { machineId, hasCompleted: this.robot.carryingCompleted }
        );
    }

    /**
     * ROBOT_RETURNED_TO_TABLE 처리
     * 로봇이 테이블에 복귀
     */
    _handleRobotReturnedToTable(data) {
        const { machineId, hasCompleted } = data;

        // 로봇 가동시간 누적
        this.robotBusyTime += (this.currentTime - this.lastRobotBusyStart);

        if (hasCompleted) {
            this.log(this.currentTime, 'PRODUCED', machineId,
                `재봉기 ${machineId}: 완료품 자재테이블 도착`);

            // 빈 pallet 도착 → 자재 준비 시작 (준비 시간 후 사용 가능)
            const readyTime = this.currentTime + this.palletPrepTime;
            this._insertSorted(this.readyPalletsQueue, readyTime);
            if (this.palletPrepTime > 0) {
                this.log(this.currentTime, 'PALLET_PREP_START', null,
                    `Pallet 자재 준비 시작 (${this.palletPrepTime}초 후 완료)`);
            }
        } else {
            this.log(this.currentTime, 'RETURN', machineId,
                `로봇 테이블 복귀 (초기 적재 - 완료품 없음)`);
        }

        this.robot.state = RobotState.AT_TABLE_IDLE;
        this.robot.targetMachine = null;
        this.robot.carryingCompleted = false;

        // 즉시 다음 스케줄링
        this.addEvent(this.currentTime, EventType.ROBOT_READY_AT_TABLE);
    }

    /**
     * MACHINE_DONE_SEWING 처리
     * 재봉기 재봉 완료
     */
    _handleMachineDoneSewing(data) {
        const { machineId } = data;
        const machine = this.machines[machineId];

        machine.state = MachineState.DONE_WAITING;
        machine.doneTime = this.currentTime;
        machine.sewingRemaining = 0;

        this.log(this.currentTime, 'SEWING_DONE', machineId,
            `재봉기 ${machineId}: 재봉 완료 - 로봇 대기 시작`);

        // 로봇이 현재 테이블에서 대기 중이면 즉시 스케줄링 트리거
        if (this.robot.state === RobotState.AT_TABLE_IDLE) {
            this.addEvent(this.currentTime, EventType.ROBOT_READY_AT_TABLE);
        }
    }

    /**
     * 특정 시간까지 시뮬레이션 실행 (빠른계산용)
     */
    runUntil(endTime) {
        this.start();

        while (this.eventQueue.length > 0) {
            const nextEvent = this.eventQueue[0];
            if (nextEvent.time > endTime) break;
            this.processEvent(this.popNextEvent());
        }

        // 최종 시간 설정
        this.currentTime = endTime;
        // 참고: DONE_WAITING 대기시간은 getResults()에서 currentWaitTime으로 합산됨
    }

    /**
     * 다음 이벤트까지 진행 (실시간 모드에서 사용)
     * @param {number} upToTime - 이 시간까지만 처리
     * @returns {boolean} 이벤트를 처리했으면 true
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
     * 현재 상태 스냅샷 반환
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
                        ? this.currentTime   // 초기 적재 대기: 시작부터 현재까지
                        : 0,
            };
        }

        // Pallet 정보 추가
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
     * 최종 결과 반환
     */
    getResults() {
        const state = this.getState();
        let totalProduced = 0;

        const machineResults = {};
        let totalMachineSewingTime = 0;

        for (const id of Config.MACHINE_IDS) {
            const m = state.machines[id];
            const totalWait = m.totalWaitTime + (m.currentWaitTime || 0);

            // 재봉기 실제 가동시간 계산: 완료된 사이클 + 현재 진행 중인 사이클
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

        // 로봇이 아직 작업 중이면 가동시간 추가
        let busyTime = this.robotBusyTime;
        if (this.robot.state !== RobotState.AT_TABLE_IDLE) {
            busyTime += (this.currentTime - this.lastRobotBusyStart);
        }

        // 재봉기 총 가동률 = 전체 재봉시간 / (시뮬레이션시간 × 재봉기 수) × 100
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
