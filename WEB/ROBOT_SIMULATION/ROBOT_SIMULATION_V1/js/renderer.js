/**
 * renderer.js - DOM 기반 시각화 렌더러
 * 로봇 이동 애니메이션, 재봉기 상태 색상, 통계 업데이트
 * 재봉기 2대/3대 모드 동적 지원
 */
const Renderer = (() => {
    // 시뮬레이션 영역 내 각 노드의 위치 (비율 기반, 600px 높이 기준)
    // 로봇 이동 목표 위치 - 각 노드의 edge 근처로 설정
    const POSITIONS = {
        table: { x: 50, y: 82 },      // 하단: table top edge 근처 (bottom:40px 고려)
        robot_center: { x: 50, y: 50 },
        1: { x: 12, y: 50 },          // Machine 1 (left) - M1 right edge 근처
        2: { x: 50, y: 22 },          // Machine 2 (top): M2 bottom edge 근처 (똑바로 이동)
        3: { x: 88, y: 50 },          // Machine 3 (right) - M3 left edge 근처
    };

    let robotEl = null;

    /**
     * 재봉기 수량에 따라 위치 업데이트
     * 2대 모드: M2가 우측(88%, 50%)으로 이동
     * 3대 모드: M2가 상단(50%, 22%) - 똑바로 이동
     */
    function setMachineCount(count) {
        if (count === 2) {
            POSITIONS[2] = { x: 88, y: 50 };  // M2를 우측으로
            delete POSITIONS[3];
        } else {
            POSITIONS[2] = { x: 50, y: 22 };  // M2는 상단 (bottom edge 근처, 똑바로 이동)
            POSITIONS[3] = { x: 88, y: 50 };  // M3는 우측
        }
    }

    function init() {
        robotEl = document.getElementById('node-robot');
    }

    // 실시간 모드에서 로봇 위치를 외부(realtimeCalc)에서 제어할지 여부
    let skipRobotPosition = false;

    function setSkipRobotPosition(val) {
        skipRobotPosition = val;
    }

    /**
     * 전체 상태 업데이트
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
     * Pallet 정보 업데이트
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
     * 경과 시간 표시
     */
    function updateElapsedTime(time) {
        const el = document.getElementById('sim-elapsed');
        if (time < 60) {
            el.textContent = time.toFixed(1) + '초';
        } else if (time < 3600) {
            const min = Math.floor(time / 60);
            const sec = (time % 60).toFixed(0);
            el.textContent = `${min}분 ${sec}초`;
        } else {
            const hr = Math.floor(time / 3600);
            const min = Math.floor((time % 3600) / 60);
            const sec = Math.floor(time % 60);
            el.textContent = `${hr}시간 ${min}분 ${sec}초`;
        }
    }

    /**
     * 로봇 상태 텍스트만 업데이트 (위치 변경 없음)
     */
    function updateRobotStatus(robot) {
        const statusEl = document.getElementById('status-robot');
        const robotNode = document.getElementById('node-robot');

        switch (robot.state) {
            case RobotState.AT_TABLE_IDLE:
                statusEl.textContent = '대기';
                robotNode.classList.remove('traveling');
                break;
            case RobotState.TRAVELING_TO_MACHINE:
                statusEl.textContent = `→ M${robot.targetMachine}`;
                robotNode.classList.add('traveling');
                break;
            case RobotState.TRAVELING_TO_TABLE:
                statusEl.textContent = `← 테이블`;
                robotNode.classList.add('traveling');
                break;
        }
    }

    /**
     * 로봇 위치 업데이트 (빠른계산/기본 모드용)
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
     * 로봇을 특정 위치로 이동
     */
    function moveRobotTo(pos) {
        if (!robotEl) return;
        robotEl.style.left = pos.x + '%';
        robotEl.style.top = pos.y + '%';
    }

    /**
     * 두 위치 사이의 보간 위치로 이동
     */
    function moveRobotToward(from, to, progress) {
        if (!robotEl) return;
        const x = from.x + (to.x - from.x) * progress;
        const y = from.y + (to.y - from.y) * progress;
        robotEl.style.left = x + '%';
        robotEl.style.top = y + '%';
    }

    /**
     * 재봉기 상태 업데이트
     */
    function updateMachines(machines) {
        for (const id of Config.MACHINE_IDS) {
            const m = machines[id];
            const nodeEl = document.getElementById(`node-m${id}`);
            const statusEl = document.getElementById(`status-m${id}`);
            const progressEl = document.getElementById(`progress-m${id}`);

            // 상태 클래스
            nodeEl.className = 'sim-node machine-node';
            switch (m.state) {
                case MachineState.EMPTY:
                    nodeEl.classList.add('state-empty');
                    if (m.currentWaitTime > 0) {
                        statusEl.textContent = `대기 ${m.currentWaitTime.toFixed(0)}초`;
                    } else {
                        statusEl.textContent = '대기';
                    }
                    progressEl.style.width = '0%';
                    break;

                case MachineState.SEWING:
                    nodeEl.classList.add('state-sewing');
                    statusEl.innerHTML = `재봉 중 ${Math.round(m.progress * 100)}%<br><span style="font-size:0.85em;opacity:0.8">${m.sewingElapsed.toFixed(1)} / ${m.sewingTotal.toFixed(1)}초</span>`;
                    progressEl.style.width = (m.progress * 100) + '%';
                    progressEl.style.background = '';
                    break;

                case MachineState.DONE_WAITING:
                    nodeEl.classList.add('state-done');
                    statusEl.textContent = `완료 대기 ${m.currentWaitTime.toFixed(0)}초`;
                    progressEl.style.width = '100%';
                    progressEl.style.background = '#f59e0b';
                    break;
            }
        }
    }

    /**
     * 실시간 통계 바 업데이트
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
     * 시뮬레이션 뷰 초기 상태로 리셋
     */
    function resetView() {
        for (const id of Config.MACHINE_IDS) {
            const nodeEl = document.getElementById(`node-m${id}`);
            nodeEl.className = 'sim-node machine-node state-empty';
            document.getElementById(`status-m${id}`).textContent = '대기';
            document.getElementById(`progress-m${id}`).style.width = '0%';
            document.getElementById(`progress-m${id}`).style.background = '';
            document.getElementById(`rt-m${id}-prod`).textContent = '0';
            document.getElementById(`rt-m${id}-wait`).textContent = '0.0';
        }
        document.getElementById('status-robot').textContent = '대기';
        document.getElementById('sim-elapsed').textContent = '0.0초';
        moveRobotTo(POSITIONS.table);

        // Pallet 정보 초기화
        const readyCountEl = document.getElementById('pallet-ready-count');
        const timerEl = document.getElementById('pallet-timer');
        if (readyCountEl) readyCountEl.textContent = '0';
        if (timerEl) timerEl.style.display = 'none';
    }

    return { init, updateState, resetView, setSkipRobotPosition, POSITIONS, setMachineCount };
})();
