/**
 * quickCalc.js - Quick calculation module
 * Runs the engine without rendering for instant results
 */
const QuickCalc = (() => {
    let SIM_DURATION = 3600; // Default 1 hour

    function setDuration(seconds) {
        SIM_DURATION = seconds;
    }

    function getDuration() {
        return SIM_DURATION;
    }

    function run(machineConfigs, palletPrepTime, palletCount) {
        const engine = new SimulationEngine(machineConfigs, palletPrepTime, palletCount);
        engine.runUntil(SIM_DURATION);
        return engine.getResults();
    }

    function formatTime(seconds) {
        const s = Math.round(seconds);
        if (s < 60) return s + 's';
        if (s < 3600) return Math.floor(s / 60) + 'min ' + (s % 60) + 's';
        const h = Math.floor(s / 3600);
        const m = Math.floor((s % 3600) / 60);
        return h + 'hr ' + m + 'min ' + (s % 60) + 's';
    }

    function displayResults(results, workerCount) {
        const section = document.getElementById('results-section');
        section.style.display = '';

        // Per-machine results
        for (const id of Config.MACHINE_IDS) {
            const m = results.machines[id];
            document.getElementById(`res-m${id}-produced`).textContent = m.produced;
            const maxEl = document.getElementById(`res-m${id}-max`);
            if (maxEl) maxEl.textContent = m.theoreticalMax;
            document.getElementById(`res-m${id}-wait`).textContent = m.totalWaitTime;
        }

        // Summary
        document.getElementById('res-total-produced').textContent = results.totalProduced;
        document.getElementById('res-robot-util').textContent = results.robotUtilization + '%';
        document.getElementById('res-robot-busy').textContent = formatTime(results.robotBusyTime);
        document.getElementById('res-sim-time').textContent = formatTime(results.simTime);

        // New metrics: PPH, Machine Utilization, Machine Active Time
        const workers = workerCount || Config.readBasicSettings().workerCount;
        const simTimeHours = results.simTime / 3600;
        const pph = simTimeHours > 0
            ? Math.round((results.totalProduced / simTimeHours / workers) * 10) / 10
            : 0;
        document.getElementById('res-pph').textContent = pph;
        document.getElementById('res-machine-util').textContent = results.machineUtilization + '%';
        document.getElementById('res-machine-busy').textContent = formatTime(results.totalMachineSewingTime);

        // Update charts
        if (typeof Charts !== 'undefined') {
            Charts.updateCharts(results);
        }

        // Display event log
        displayEventLog(results.eventLog);
    }

    function displayEventLog(logs) {
        const section = document.getElementById('log-section');
        section.style.display = '';

        const tbody = document.getElementById('log-body');
        tbody.innerHTML = '';

        for (const entry of logs) {
            const tr = document.createElement('tr');

            const tdTime = document.createElement('td');
            tdTime.textContent = entry.time.toFixed(1);
            tr.appendChild(tdTime);

            const tdType = document.createElement('td');
            tdType.textContent = entry.type;
            tdType.className = entry.machineId ? '' : 'log-robot';
            tr.appendChild(tdType);

            const tdMachine = document.createElement('td');
            if (entry.machineId) {
                tdMachine.textContent = `M${entry.machineId}`;
                tdMachine.className = `log-m${entry.machineId}`;
            } else {
                tdMachine.textContent = '-';
            }
            tr.appendChild(tdMachine);

            const tdDesc = document.createElement('td');
            tdDesc.textContent = entry.description;
            tr.appendChild(tdDesc);

            tbody.appendChild(tr);
        }
    }

    return { run, displayResults, setDuration, getDuration };
})();
