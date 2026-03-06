/**
 * quickCalc.js - 빠른계산 모듈
 * 엔진을 렌더링 없이 3600초(1시간) 즉시 실행
 */
const QuickCalc = (() => {
    let SIM_DURATION = 3600; // 기본값 1시간

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
        if (s < 60) return s + '초';
        if (s < 3600) return Math.floor(s / 60) + '분 ' + (s % 60) + '초';
        const h = Math.floor(s / 3600);
        const m = Math.floor((s % 3600) / 60);
        return h + '시간 ' + m + '분 ' + (s % 60) + '초';
    }

    function displayResults(results, workerCount) {
        const section = document.getElementById('results-section');
        section.style.display = '';

        // 각 재봉기별 결과
        for (const id of Config.MACHINE_IDS) {
            const m = results.machines[id];
            document.getElementById(`res-m${id}-produced`).textContent = m.produced;
            const maxEl = document.getElementById(`res-m${id}-max`);
            if (maxEl) maxEl.textContent = m.theoreticalMax;
            document.getElementById(`res-m${id}-wait`).textContent = m.totalWaitTime;
        }

        // 종합
        document.getElementById('res-total-produced').textContent = results.totalProduced;
        document.getElementById('res-robot-util').textContent = results.robotUtilization + '%';
        document.getElementById('res-robot-busy').textContent = formatTime(results.robotBusyTime);
        document.getElementById('res-sim-time').textContent = formatTime(results.simTime);

        // 새 지표: PPH, 재봉기 총 가동률, 재봉기 총 가동시간
        const workers = workerCount || Config.readBasicSettings().workerCount;
        const simTimeHours = results.simTime / 3600;
        const pph = simTimeHours > 0
            ? Math.round((results.totalProduced / simTimeHours / workers) * 10) / 10
            : 0;
        document.getElementById('res-pph').textContent = pph;
        document.getElementById('res-machine-util').textContent = results.machineUtilization + '%';
        document.getElementById('res-machine-busy').textContent = formatTime(results.totalMachineSewingTime);

        // 차트 업데이트
        if (typeof Charts !== 'undefined') {
            Charts.updateCharts(results);
        }

        // 이벤트 로그 표시
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
