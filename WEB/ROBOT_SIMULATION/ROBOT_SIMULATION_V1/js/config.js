/**
 * config.js - 설정값 관리 모듈
 * 입력값 읽기, 유효성 검증, 기본값 제공
 * 재봉기 2대/3대 모드 동적 지원
 */
const Config = (() => {
    const MACHINE_IDS = [1, 2, 3];
    let machineCount = 3;

    /**
     * 재봉기 수량 설정 (2 또는 3)
     * MACHINE_IDS 배열을 in-place로 수정하여 모든 참조가 자동 반영됨
     */
    function setMachineCount(count) {
        machineCount = count;
        MACHINE_IDS.length = 0;
        if (count === 2) {
            MACHINE_IDS.push(1, 2);
        } else {
            MACHINE_IDS.push(1, 2, 3);
        }
    }

    function getMachineCount() {
        return machineCount;
    }

    function readInputs() {
        const machines = {};
        for (const id of MACHINE_IDS) {
            machines[id] = {
                initLoadTime: parseFloat(document.getElementById(`m${id}-init`).value) || 15,
                unloadLoadTime: parseFloat(document.getElementById(`m${id}-ul`).value) || 20,
                returnTime: parseFloat(document.getElementById(`m${id}-return`).value) || 10,
                sewingTime: parseFloat(document.getElementById(`m${id}-sewing`).value) || 60,
            };
        }
        return machines;
    }

    function validate(machines) {
        const errors = [];
        for (const id of MACHINE_IDS) {
            const m = machines[id];
            if (m.initLoadTime <= 0) errors.push(`재봉기 ${id}: 초기 적재 시간은 0보다 커야 합니다.`);
            if (m.unloadLoadTime <= 0) errors.push(`재봉기 ${id}: Unload & Load 시간은 0보다 커야 합니다.`);
            if (m.returnTime <= 0) errors.push(`재봉기 ${id}: 복귀 시간은 0보다 커야 합니다.`);
            if (m.sewingTime <= 0) errors.push(`재봉기 ${id}: 재봉 시간은 0보다 커야 합니다.`);
        }
        return errors;
    }

    function getConfig() {
        const machines = readInputs();
        const errors = validate(machines);
        if (errors.length > 0) {
            return { valid: false, errors, machines: null };
        }
        return { valid: true, errors: [], machines };
    }

    function getTheoreticalMax(sewingTime, totalSeconds) {
        return Math.floor(totalSeconds / sewingTime);
    }

    /**
     * 기본 설정 읽기 (작업인원, Pallet 준비시간, Pallet 수량)
     */
    function readBasicSettings() {
        // Pallet 수량은 버튼에서 active 상태인 값 읽기
        const activePalletBtn = document.querySelector('.pallet-count-btn.active');
        const palletCount = activePalletBtn ? parseInt(activePalletBtn.dataset.palletCount) : 5;

        return {
            workerCount: parseInt(document.getElementById('worker-count').value) || 2,
            palletPrepTime: parseFloat(document.getElementById('pallet-prep-time').value) || 0,
            palletCount: palletCount,
        };
    }

    /**
     * 로봇 속도별 하드코딩 데이터 (재봉시간 제외 - 로봇 속도와 무관)
     * initLoadTime, unloadLoadTime, returnTime만 속도에 따라 변함
     * 소수점 1자리까지 지원, 75-85-100 구간별 보간
     */
    const SPEED_DATA = {
        75: {
            1: { initLoadTime: 12.5, unloadLoadTime: 17.5, returnTime: 10.5 },
            2: { initLoadTime: 13.5, unloadLoadTime: 18.5, returnTime: 11.5 },
            3: { initLoadTime: 12.5, unloadLoadTime: 17.5, returnTime: 10.5 },
        },
        85: {
            1: { initLoadTime: 11, unloadLoadTime: 15.5, returnTime: 9 },
            2: { initLoadTime: 11.8, unloadLoadTime: 16.3, returnTime: 9.8 },
            3: { initLoadTime: 11, unloadLoadTime: 15.5, returnTime: 9 },
        },
        100: {
            1: { initLoadTime: 8.5, unloadLoadTime: 13, returnTime: 7.5 },
            2: { initLoadTime: 9, unloadLoadTime: 13.5, returnTime: 8 },
            3: { initLoadTime: 8.5, unloadLoadTime: 13, returnTime: 7.5 },
        }
    };

    /**
     * 주어진 속도(%)에 대한 각 재봉기의 작업 시간값 반환
     * 75-85, 85-100 구간별 선형 보간으로 더 정확한 예측
     * 소수점 1자리까지 반환
     */
    function getSpeedValues(speedPercent) {
        if (speedPercent === 75) return SPEED_DATA[75];
        if (speedPercent === 85) return SPEED_DATA[85];
        if (speedPercent === 100) return SPEED_DATA[100];

        let lowSpeed, highSpeed;
        if (speedPercent < 85) {
            lowSpeed = 75;
            highSpeed = 85;
        } else {
            lowSpeed = 85;
            highSpeed = 100;
        }

        const ratio = (speedPercent - lowSpeed) / (highSpeed - lowSpeed);
        const result = {};
        for (const machineId of [1, 2, 3]) {
            const low = SPEED_DATA[lowSpeed][machineId];
            const high = SPEED_DATA[highSpeed][machineId];
            result[machineId] = {
                initLoadTime: Math.round((low.initLoadTime + (high.initLoadTime - low.initLoadTime) * ratio) * 10) / 10,
                unloadLoadTime: Math.round((low.unloadLoadTime + (high.unloadLoadTime - low.unloadLoadTime) * ratio) * 10) / 10,
                returnTime: Math.round((low.returnTime + (high.returnTime - low.returnTime) * ratio) * 10) / 10,
            };
        }
        return result;
    }

    return { MACHINE_IDS, getConfig, getTheoreticalMax, setMachineCount, getMachineCount, readBasicSettings, getSpeedValues, SPEED_DATA };
})();
