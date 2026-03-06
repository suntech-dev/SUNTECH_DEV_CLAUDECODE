/**
 * scenario.js - Scenario comparison module
 * Save, delete, comparison table/chart management
 */
const ScenarioManager = (() => {
    const MAX_SCENARIOS = 5;
    let scenarios = [];
    let comparisonCharts = {};

    /**
     * Save current settings and results as scenario
     */
    function saveScenario(name, settings, results) {
        if (scenarios.length >= MAX_SCENARIOS) {
            alert(`Maximum ${MAX_SCENARIOS} scenarios can be saved. Please delete existing scenarios.`);
            return false;
        }

        const scenario = {
            id: Date.now().toString(),
            name: name || `Scenario ${scenarios.length + 1}`,
            timestamp: Date.now(),
            mode: Config.getMachineCount(),
            settings: {
                workerCount: settings.workerCount,
                palletPrepTime: settings.palletPrepTime,
                palletCount: settings.palletCount,
                simDuration: settings.simDuration,
                machines: JSON.parse(JSON.stringify(settings.machines))
            },
            results: {
                totalProduced: results.totalProduced,
                pph: calculatePPH(results.totalProduced, settings.simDuration, settings.workerCount),
                robotUtilization: results.robotUtilization,
                machineUtilization: results.machineUtilization,
                robotBusyTime: results.robotBusyTime,
                totalMachineSewingTime: results.totalMachineSewingTime,
                simTime: results.simTime,
                machines: {}
            }
        };

        for (const id of Config.MACHINE_IDS) {
            scenario.results.machines[id] = {
                produced: results.machines[id].produced,
                totalWaitTime: results.machines[id].totalWaitTime
            };
        }

        scenarios.push(scenario);
        updateComparisonUI();
        return true;
    }

    /**
     * Calculate PPH
     */
    function calculatePPH(totalProduced, simDuration, workerCount) {
        const hours = simDuration / 3600;
        return hours > 0 ? Math.round((totalProduced / hours / workerCount) * 10) / 10 : 0;
    }

    /**
     * Delete scenario
     */
    function deleteScenario(id) {
        scenarios = scenarios.filter(s => s.id !== id);
        updateComparisonUI();
    }

    /**
     * Delete all scenarios
     */
    function clearAllScenarios() {
        scenarios = [];
        updateComparisonUI();
    }

    /**
     * Return scenario list
     */
    function getScenarios() {
        return scenarios;
    }

    /**
     * Update comparison UI
     */
    function updateComparisonUI() {
        const section = document.getElementById('scenario-comparison-section');
        const tableBody = document.getElementById('scenario-table-body');
        const emptyMessage = document.getElementById('scenario-empty-message');
        const comparisonContent = document.getElementById('scenario-comparison-content');

        if (!section) return;

        if (scenarios.length === 0) {
            section.style.display = 'none';
            return;
        }

        section.style.display = '';

        if (scenarios.length < 2) {
            emptyMessage.style.display = '';
            emptyMessage.textContent = 'Save 2 or more scenarios to compare.';
            comparisonContent.style.display = 'none';
            renderScenarioList();
            return;
        }

        emptyMessage.style.display = 'none';
        comparisonContent.style.display = '';

        renderComparisonTable();
        renderComparisonCharts();
        renderScenarioList();
    }

    /**
     * Render scenario list (with delete buttons)
     */
    function renderScenarioList() {
        const listContainer = document.getElementById('scenario-list');
        if (!listContainer) return;

        listContainer.innerHTML = '';

        scenarios.forEach((scenario, index) => {
            const item = document.createElement('div');
            item.className = 'scenario-list-item';

            const colorClass = `scenario-color-${index + 1}`;

            item.innerHTML = `
                <span class="scenario-color-indicator ${colorClass}"></span>
                <span class="scenario-name">${scenario.name}</span>
                <span class="scenario-mode">${scenario.mode}-Machine</span>
                <span class="scenario-production">Prod: ${scenario.results.totalProduced}</span>
                <button class="btn-scenario-delete" data-id="${scenario.id}" title="Delete">&times;</button>
            `;

            listContainer.appendChild(item);
        });

        listContainer.querySelectorAll('.btn-scenario-delete').forEach(btn => {
            btn.addEventListener('click', (e) => {
                const id = e.target.dataset.id;
                if (confirm('Delete this scenario?')) {
                    deleteScenario(id);
                }
            });
        });
    }

    /**
     * Render comparison table
     */
    function renderComparisonTable() {
        const tableHead = document.getElementById('scenario-table-head');
        const tableBody = document.getElementById('scenario-table-body');

        if (!tableHead || !tableBody) return;

        // Table header
        let headHtml = '<tr><th class="metric-col">Metric</th>';
        scenarios.forEach((s, i) => {
            headHtml += `<th class="scenario-col scenario-color-${i + 1}">${s.name}</th>`;
        });
        headHtml += '<th class="best-col">Best</th></tr>';
        tableHead.innerHTML = headHtml;

        // Comparison metrics
        const metrics = [
            { key: 'totalProduced', label: 'Total Production', unit: '', higher: true },
            { key: 'pph', label: 'PPH', unit: '', higher: true },
            { key: 'robotUtilization', label: 'Robot Utilization', unit: '%', higher: true },
            { key: 'machineUtilization', label: 'Machine Utilization', unit: '%', higher: true },
            { key: 'totalWaitTime', label: 'Total Wait Time', unit: 's', higher: false }
        ];

        // Table body
        let bodyHtml = '';
        metrics.forEach(metric => {
            bodyHtml += '<tr>';
            bodyHtml += `<td class="metric-col">${metric.label}</td>`;

            let values = [];
            scenarios.forEach((s, i) => {
                let value;
                if (metric.key === 'totalWaitTime') {
                    value = Object.values(s.results.machines).reduce((sum, m) => sum + m.totalWaitTime, 0);
                } else {
                    value = s.results[metric.key];
                }
                values.push({ value, index: i, name: s.name });
            });

            const sortedValues = [...values].sort((a, b) =>
                metric.higher ? b.value - a.value : a.value - b.value
            );
            const bestValue = sortedValues[0].value;

            values.forEach((v, i) => {
                const isBest = v.value === bestValue;
                const cellClass = isBest ? 'best-value' : '';
                bodyHtml += `<td class="scenario-col ${cellClass}">${v.value}${metric.unit}</td>`;
            });

            bodyHtml += `<td class="best-col">${sortedValues[0].name}</td>`;
            bodyHtml += '</tr>';
        });

        // Settings comparison
        const settingMetrics = [
            { key: 'workerCount', label: 'Workers', unit: '' },
            { key: 'palletPrepTime', label: 'Pallet Prep Time', unit: 's' },
            { key: 'palletCount', label: 'Pallet Count', unit: '' }
        ];

        bodyHtml += '<tr class="settings-separator"><td colspan="' + (scenarios.length + 2) + '">Settings</td></tr>';

        settingMetrics.forEach(metric => {
            bodyHtml += '<tr class="settings-row">';
            bodyHtml += `<td class="metric-col">${metric.label}</td>`;

            scenarios.forEach(s => {
                bodyHtml += `<td class="scenario-col">${s.settings[metric.key]}${metric.unit}</td>`;
            });

            bodyHtml += '<td class="best-col">-</td>';
            bodyHtml += '</tr>';
        });

        tableBody.innerHTML = bodyHtml;
    }

    /**
     * Render comparison charts
     */
    function renderComparisonCharts() {
        const colors = [
            'rgba(59, 130, 246, 0.8)',
            'rgba(16, 185, 129, 0.8)',
            'rgba(245, 158, 11, 0.8)',
            'rgba(239, 68, 68, 0.8)',
            'rgba(139, 92, 246, 0.8)'
        ];

        const borderColors = [
            'rgba(59, 130, 246, 1)',
            'rgba(16, 185, 129, 1)',
            'rgba(245, 158, 11, 1)',
            'rgba(239, 68, 68, 1)',
            'rgba(139, 92, 246, 1)'
        ];

        Object.values(comparisonCharts).forEach(chart => {
            if (chart) chart.destroy();
        });
        comparisonCharts = {};

        const labels = scenarios.map(s => s.name);

        // 1. Production comparison chart
        const productionCtx = document.getElementById('chart-scenario-production');
        if (productionCtx) {
            comparisonCharts.production = new Chart(productionCtx, {
                type: 'bar',
                data: {
                    labels: labels,
                    datasets: [{
                        label: 'Total Production',
                        data: scenarios.map(s => s.results.totalProduced),
                        backgroundColor: scenarios.map((_, i) => colors[i]),
                        borderColor: scenarios.map((_, i) => borderColors[i]),
                        borderWidth: 1
                    }]
                },
                options: {
                    responsive: true,
                    maintainAspectRatio: false,
                    plugins: {
                        legend: { display: false }
                    },
                    scales: {
                        y: { beginAtZero: true }
                    }
                }
            });
        }

        // 2. PPH comparison chart
        const pphCtx = document.getElementById('chart-scenario-pph');
        if (pphCtx) {
            comparisonCharts.pph = new Chart(pphCtx, {
                type: 'bar',
                data: {
                    labels: labels,
                    datasets: [{
                        label: 'PPH',
                        data: scenarios.map(s => s.results.pph),
                        backgroundColor: scenarios.map((_, i) => colors[i]),
                        borderColor: scenarios.map((_, i) => borderColors[i]),
                        borderWidth: 1
                    }]
                },
                options: {
                    responsive: true,
                    maintainAspectRatio: false,
                    plugins: {
                        legend: { display: false }
                    },
                    scales: {
                        y: { beginAtZero: true }
                    }
                }
            });
        }

        // 3. Utilization comparison chart (radar)
        const utilCtx = document.getElementById('chart-scenario-util');
        if (utilCtx) {
            comparisonCharts.util = new Chart(utilCtx, {
                type: 'radar',
                data: {
                    labels: ['Robot Utilization', 'Machine Utilization', 'Production Efficiency'],
                    datasets: scenarios.map((s, i) => ({
                        label: s.name,
                        data: [
                            s.results.robotUtilization,
                            s.results.machineUtilization,
                            Math.min(100, s.results.pph * 2)
                        ],
                        backgroundColor: colors[i].replace('0.8', '0.2'),
                        borderColor: borderColors[i],
                        borderWidth: 2,
                        pointBackgroundColor: borderColors[i]
                    }))
                },
                options: {
                    responsive: true,
                    maintainAspectRatio: false,
                    scales: {
                        r: {
                            beginAtZero: true,
                            max: 100
                        }
                    }
                }
            });
        }
    }

    /**
     * Open save scenario dialog
     */
    function openSaveDialog(settings, results) {
        const defaultName = `Scenario ${scenarios.length + 1}`;
        const name = prompt('Enter scenario name:', defaultName);

        if (name === null) return;

        if (saveScenario(name || defaultName, settings, results)) {
            const section = document.getElementById('scenario-comparison-section');
            if (section) {
                section.scrollIntoView({ behavior: 'smooth', block: 'start' });
            }
        }
    }

    /**
     * Destroy charts (on mode change)
     */
    function destroyCharts() {
        Object.values(comparisonCharts).forEach(chart => {
            if (chart) chart.destroy();
        });
        comparisonCharts = {};
    }

    return {
        saveScenario,
        deleteScenario,
        clearAllScenarios,
        getScenarios,
        updateComparisonUI,
        openSaveDialog,
        destroyCharts,
        MAX_SCENARIOS
    };
})();
