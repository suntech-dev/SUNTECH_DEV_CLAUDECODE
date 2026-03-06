/**
 * charts.js - Chart.js-based result visualization module
 * Display simulation results in various charts
 */
const Charts = (() => {
    let productionChart = null;
    let waittimeChart = null;
    let robotUtilChart = null;
    let machineUtilChart = null;

    const MACHINE_COLORS = {
        1: { bg: 'rgba(59, 130, 246, 0.7)', border: '#3b82f6' },
        2: { bg: 'rgba(34, 197, 94, 0.7)', border: '#22c55e' },
        3: { bg: 'rgba(239, 68, 68, 0.7)', border: '#ef4444' },
    };

    /**
     * Initialize/update all charts
     */
    function updateCharts(results) {
        const machineIds = Config.MACHINE_IDS;

        updateProductionChart(results, machineIds);
        updateWaittimeChart(results, machineIds);
        updateRobotUtilChart(results);
        updateMachineUtilChart(results, machineIds);
    }

    /**
     * Production by machine bar chart
     */
    function updateProductionChart(results, machineIds) {
        const ctx = document.getElementById('chart-production');
        if (!ctx) return;

        const labels = machineIds.map(id => `Machine ${id}`);
        const data = machineIds.map(id => results.machines[id].produced);
        const bgColors = machineIds.map(id => MACHINE_COLORS[id].bg);
        const borderColors = machineIds.map(id => MACHINE_COLORS[id].border);

        if (productionChart) {
            productionChart.destroy();
        }

        productionChart = new Chart(ctx, {
            type: 'bar',
            data: {
                labels: labels,
                datasets: [{
                    label: 'Production',
                    data: data,
                    backgroundColor: bgColors,
                    borderColor: borderColors,
                    borderWidth: 2,
                    borderRadius: 6,
                }]
            },
            options: {
                responsive: true,
                maintainAspectRatio: true,
                plugins: {
                    legend: { display: false },
                    tooltip: {
                        callbacks: {
                            label: (context) => `Production: ${context.raw} pcs`
                        }
                    }
                },
                scales: {
                    y: {
                        beginAtZero: true,
                        ticks: {
                            stepSize: Math.ceil(Math.max(...data) / 5) || 1
                        },
                        title: {
                            display: true,
                            text: 'Production (pcs)'
                        }
                    }
                }
            }
        });
    }

    /**
     * Wait time by machine bar chart
     */
    function updateWaittimeChart(results, machineIds) {
        const ctx = document.getElementById('chart-waittime');
        if (!ctx) return;

        const labels = machineIds.map(id => `Machine ${id}`);
        const data = machineIds.map(id => results.machines[id].totalWaitTime);
        const bgColors = machineIds.map(id => MACHINE_COLORS[id].bg);
        const borderColors = machineIds.map(id => MACHINE_COLORS[id].border);

        if (waittimeChart) {
            waittimeChart.destroy();
        }

        waittimeChart = new Chart(ctx, {
            type: 'bar',
            data: {
                labels: labels,
                datasets: [{
                    label: 'Wait Time',
                    data: data,
                    backgroundColor: bgColors,
                    borderColor: borderColors,
                    borderWidth: 2,
                    borderRadius: 6,
                }]
            },
            options: {
                responsive: true,
                maintainAspectRatio: true,
                plugins: {
                    legend: { display: false },
                    tooltip: {
                        callbacks: {
                            label: (context) => `Wait Time: ${context.raw.toFixed(1)}s`
                        }
                    }
                },
                scales: {
                    y: {
                        beginAtZero: true,
                        title: {
                            display: true,
                            text: 'Wait Time (sec)'
                        }
                    }
                }
            }
        });
    }

    /**
     * Robot time usage doughnut chart
     */
    function updateRobotUtilChart(results) {
        const ctx = document.getElementById('chart-robot-util');
        if (!ctx) return;

        const busyTime = results.robotBusyTime;
        const idleTime = Math.max(0, results.simTime - busyTime);

        if (robotUtilChart) {
            robotUtilChart.destroy();
        }

        robotUtilChart = new Chart(ctx, {
            type: 'doughnut',
            data: {
                labels: ['Active Time', 'Idle Time'],
                datasets: [{
                    data: [busyTime, idleTime],
                    backgroundColor: [
                        'rgba(124, 58, 237, 0.8)',
                        'rgba(148, 163, 184, 0.5)'
                    ],
                    borderColor: [
                        '#7c3aed',
                        '#94a3b8'
                    ],
                    borderWidth: 2,
                }]
            },
            options: {
                responsive: true,
                maintainAspectRatio: true,
                cutout: '55%',
                plugins: {
                    legend: {
                        position: 'bottom',
                        labels: {
                            padding: 12,
                            font: { size: 11 }
                        }
                    },
                    tooltip: {
                        callbacks: {
                            label: (context) => {
                                const total = busyTime + idleTime;
                                const percent = total > 0 ? ((context.raw / total) * 100).toFixed(1) : 0;
                                return `${context.label}: ${context.raw.toFixed(1)}s (${percent}%)`;
                            }
                        }
                    }
                }
            },
            plugins: [{
                id: 'centerText',
                beforeDraw: function(chart) {
                    const width = chart.width;
                    const height = chart.height;
                    const ctx = chart.ctx;
                    ctx.restore();

                    const fontSize = (height / 114).toFixed(2);
                    ctx.font = `bold ${fontSize}em Segoe UI, sans-serif`;
                    ctx.textBaseline = 'middle';
                    ctx.fillStyle = '#7c3aed';

                    const text = `${results.robotUtilization}%`;
                    const textX = Math.round((width - ctx.measureText(text).width) / 2);
                    const textY = height / 2 - 10;

                    ctx.fillText(text, textX, textY);
                    ctx.save();
                }
            }]
        });
    }

    /**
     * Machine time usage doughnut chart
     */
    function updateMachineUtilChart(results, machineIds) {
        const ctx = document.getElementById('chart-machine-util');
        if (!ctx) return;

        const totalSewingTime = results.totalMachineSewingTime;
        const totalPossibleTime = results.simTime * machineIds.length;
        const idleTime = Math.max(0, totalPossibleTime - totalSewingTime);

        if (machineUtilChart) {
            machineUtilChart.destroy();
        }

        machineUtilChart = new Chart(ctx, {
            type: 'doughnut',
            data: {
                labels: ['Sewing Time', 'Idle Time'],
                datasets: [{
                    data: [totalSewingTime, idleTime],
                    backgroundColor: [
                        'rgba(34, 197, 94, 0.8)',
                        'rgba(148, 163, 184, 0.5)'
                    ],
                    borderColor: [
                        '#22c55e',
                        '#94a3b8'
                    ],
                    borderWidth: 2,
                }]
            },
            options: {
                responsive: true,
                maintainAspectRatio: true,
                cutout: '55%',
                plugins: {
                    legend: {
                        position: 'bottom',
                        labels: {
                            padding: 12,
                            font: { size: 11 }
                        }
                    },
                    tooltip: {
                        callbacks: {
                            label: (context) => {
                                const total = totalSewingTime + idleTime;
                                const percent = total > 0 ? ((context.raw / total) * 100).toFixed(1) : 0;
                                return `${context.label}: ${context.raw.toFixed(1)}s (${percent}%)`;
                            }
                        }
                    }
                }
            },
            plugins: [{
                id: 'centerText',
                beforeDraw: function(chart) {
                    const width = chart.width;
                    const height = chart.height;
                    const ctx = chart.ctx;
                    ctx.restore();

                    const fontSize = (height / 114).toFixed(2);
                    ctx.font = `bold ${fontSize}em Segoe UI, sans-serif`;
                    ctx.textBaseline = 'middle';
                    ctx.fillStyle = '#22c55e';

                    const text = `${results.machineUtilization}%`;
                    const textX = Math.round((width - ctx.measureText(text).width) / 2);
                    const textY = height / 2 - 10;

                    ctx.fillText(text, textX, textY);
                    ctx.save();
                }
            }]
        });
    }

    /**
     * Destroy all charts (e.g., on mode change)
     */
    function destroyAllCharts() {
        if (productionChart) {
            productionChart.destroy();
            productionChart = null;
        }
        if (waittimeChart) {
            waittimeChart.destroy();
            waittimeChart = null;
        }
        if (robotUtilChart) {
            robotUtilChart.destroy();
            robotUtilChart = null;
        }
        if (machineUtilChart) {
            machineUtilChart.destroy();
            machineUtilChart = null;
        }
    }

    return {
        updateCharts,
        destroyAllCharts
    };
})();
