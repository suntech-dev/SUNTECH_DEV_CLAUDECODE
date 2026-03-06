/**
 * charts.js - Chart.js 기반 결과 시각화 모듈
 * 시뮬레이션 결과를 다양한 차트로 표시
 */
const Charts = (() => {
    // 차트 인스턴스 저장 (재생성 시 파괴 필요)
    let productionChart = null;
    let waittimeChart = null;
    let robotUtilChart = null;
    let machineUtilChart = null;

    // 재봉기별 색상
    const MACHINE_COLORS = {
        1: { bg: 'rgba(59, 130, 246, 0.7)', border: '#3b82f6' },  // Blue
        2: { bg: 'rgba(34, 197, 94, 0.7)', border: '#22c55e' },   // Green
        3: { bg: 'rgba(239, 68, 68, 0.7)', border: '#ef4444' },   // Red
    };

    /**
     * 모든 차트 초기화/업데이트
     * @param {Object} results - engine.getResults() 결과
     */
    function updateCharts(results) {
        const machineIds = Config.MACHINE_IDS;

        updateProductionChart(results, machineIds);
        updateWaittimeChart(results, machineIds);
        updateRobotUtilChart(results);
        updateMachineUtilChart(results, machineIds);
    }

    /**
     * 재봉기별 생산량 막대 차트
     */
    function updateProductionChart(results, machineIds) {
        const ctx = document.getElementById('chart-production');
        if (!ctx) return;

        const labels = machineIds.map(id => `재봉기 ${id}`);
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
                    label: '생산량',
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
                            label: (context) => `생산량: ${context.raw}개`
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
                            text: '생산량 (개)'
                        }
                    }
                }
            }
        });
    }

    /**
     * 재봉기별 대기시간 막대 차트
     */
    function updateWaittimeChart(results, machineIds) {
        const ctx = document.getElementById('chart-waittime');
        if (!ctx) return;

        const labels = machineIds.map(id => `재봉기 ${id}`);
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
                    label: '대기시간',
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
                            label: (context) => `대기시간: ${context.raw.toFixed(1)}초`
                        }
                    }
                },
                scales: {
                    y: {
                        beginAtZero: true,
                        title: {
                            display: true,
                            text: '대기시간 (초)'
                        }
                    }
                }
            }
        });
    }

    /**
     * 로봇 시간 활용 도넛 차트
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
                labels: ['가동시간', '대기시간'],
                datasets: [{
                    data: [busyTime, idleTime],
                    backgroundColor: [
                        'rgba(124, 58, 237, 0.8)',  // Purple (Robot)
                        'rgba(148, 163, 184, 0.5)'  // Gray
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
                                return `${context.label}: ${context.raw.toFixed(1)}초 (${percent}%)`;
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
     * 재봉기 시간 활용 도넛 차트
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
                labels: ['재봉시간', '유휴시간'],
                datasets: [{
                    data: [totalSewingTime, idleTime],
                    backgroundColor: [
                        'rgba(34, 197, 94, 0.8)',   // Green (Sewing)
                        'rgba(148, 163, 184, 0.5)' // Gray
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
                                return `${context.label}: ${context.raw.toFixed(1)}초 (${percent}%)`;
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
     * 모든 차트 파괴 (모드 변경 시 등)
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
