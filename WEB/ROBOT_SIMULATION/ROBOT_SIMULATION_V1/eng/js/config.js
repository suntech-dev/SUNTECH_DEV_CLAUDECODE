/**
 * config.js - Configuration management module
 * Input reading, validation, default values
 * Dynamic support for 2/3 machine modes
 */
const Config = (() => {
    const MACHINE_IDS = [1, 2, 3];
    let machineCount = 3;

    /**
     * Set machine count (2 or 3)
     * Modifies MACHINE_IDS array in-place so all references auto-update
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
            if (m.initLoadTime <= 0) errors.push(`Machine ${id}: Initial load time must be greater than 0.`);
            if (m.unloadLoadTime <= 0) errors.push(`Machine ${id}: Unload & Load time must be greater than 0.`);
            if (m.returnTime <= 0) errors.push(`Machine ${id}: Return time must be greater than 0.`);
            if (m.sewingTime <= 0) errors.push(`Machine ${id}: Sewing time must be greater than 0.`);
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
     * Read basic settings (Workers, Pallet Prep Time, Pallet Count)
     */
    function readBasicSettings() {
        const activePalletBtn = document.querySelector('.pallet-count-btn.active');
        const palletCount = activePalletBtn ? parseInt(activePalletBtn.dataset.palletCount) : 5;

        return {
            workerCount: parseInt(document.getElementById('worker-count').value) || 2,
            palletPrepTime: parseFloat(document.getElementById('pallet-prep-time').value) || 0,
            palletCount: palletCount,
        };
    }

    /**
     * Robot speed preset data (excluding sewing time - independent of robot speed)
     * Only initLoadTime, unloadLoadTime, returnTime change with speed
     * Supports 1 decimal place, interpolation between 75-85-100 ranges
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
     * Return work time values for each machine at given speed (%)
     * Linear interpolation between 75-85, 85-100 ranges
     * Returns values with 1 decimal place
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
