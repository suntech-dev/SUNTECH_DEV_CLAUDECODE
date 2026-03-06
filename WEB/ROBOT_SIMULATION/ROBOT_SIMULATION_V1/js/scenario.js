/**
 * scenario.js - 시나리오 비교 기능 모듈
 * 시나리오 저장, 삭제, 비교 테이블/차트 관리
 */
const ScenarioManager = (() => {
    const MAX_SCENARIOS = 5;
    let scenarios = [];
    let comparisonCharts = {};

    /**
     * 현재 설정값과 결과를 시나리오로 저장
     */
    function saveScenario(name, settings, results) {
        if (scenarios.length >= MAX_SCENARIOS) {
            alert(`최대 ${MAX_SCENARIOS}개까지 저장할 수 있습니다. 기존 시나리오를 삭제해주세요.`);
            return false;
        }

        const scenario = {
            id: Date.now().toString(),
            name: name || `시나리오 ${scenarios.length + 1}`,
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

        // 각 재봉기별 결과 복사
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
     * PPH 계산
     */
    function calculatePPH(totalProduced, simDuration, workerCount) {
        const hours = simDuration / 3600;
        return hours > 0 ? Math.round((totalProduced / hours / workerCount) * 10) / 10 : 0;
    }

    /**
     * 시나리오 삭제
     */
    function deleteScenario(id) {
        scenarios = scenarios.filter(s => s.id !== id);
        updateComparisonUI();
    }

    /**
     * 모든 시나리오 삭제
     */
    function clearAllScenarios() {
        scenarios = [];
        updateComparisonUI();
    }

    /**
     * 시나리오 목록 반환
     */
    function getScenarios() {
        return scenarios;
    }

    /**
     * 비교 UI 업데이트
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
            emptyMessage.textContent = '비교하려면 2개 이상의 시나리오를 저장하세요.';
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
     * 시나리오 목록 렌더링 (삭제 버튼 포함)
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
                <span class="scenario-mode">${scenario.mode}대 모드</span>
                <span class="scenario-production">생산: ${scenario.results.totalProduced}개</span>
                <button class="btn-scenario-delete" data-id="${scenario.id}" title="삭제">×</button>
            `;

            listContainer.appendChild(item);
        });

        // 삭제 버튼 이벤트
        listContainer.querySelectorAll('.btn-scenario-delete').forEach(btn => {
            btn.addEventListener('click', (e) => {
                const id = e.target.dataset.id;
                if (confirm('이 시나리오를 삭제하시겠습니까?')) {
                    deleteScenario(id);
                }
            });
        });
    }

    /**
     * 비교 테이블 렌더링
     */
    function renderComparisonTable() {
        const tableHead = document.getElementById('scenario-table-head');
        const tableBody = document.getElementById('scenario-table-body');

        if (!tableHead || !tableBody) return;

        // 테이블 헤더
        let headHtml = '<tr><th class="metric-col">지표</th>';
        scenarios.forEach((s, i) => {
            headHtml += `<th class="scenario-col scenario-color-${i + 1}">${s.name}</th>`;
        });
        headHtml += '<th class="best-col">최적</th></tr>';
        tableHead.innerHTML = headHtml;

        // 비교 지표 정의
        const metrics = [
            { key: 'totalProduced', label: '총 생산량', unit: '개', higher: true },
            { key: 'pph', label: 'PPH', unit: '', higher: true },
            { key: 'robotUtilization', label: '로봇 가동률', unit: '%', higher: true },
            { key: 'machineUtilization', label: '재봉기 가동률', unit: '%', higher: true },
            { key: 'totalWaitTime', label: '총 대기시간', unit: '초', higher: false }
        ];

        // 테이블 바디
        let bodyHtml = '';
        metrics.forEach(metric => {
            bodyHtml += '<tr>';
            bodyHtml += `<td class="metric-col">${metric.label}</td>`;

            let values = [];
            scenarios.forEach((s, i) => {
                let value;
                if (metric.key === 'totalWaitTime') {
                    // 모든 재봉기의 대기시간 합산
                    value = Object.values(s.results.machines).reduce((sum, m) => sum + m.totalWaitTime, 0);
                } else {
                    value = s.results[metric.key];
                }
                values.push({ value, index: i, name: s.name });
            });

            // 최적값 찾기
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

        // 설정값 비교 추가
        const settingMetrics = [
            { key: 'workerCount', label: '작업인원', unit: '명' },
            { key: 'palletPrepTime', label: 'Pallet 준비시간', unit: '초' },
            { key: 'palletCount', label: 'Pallet 수량', unit: '개' }
        ];

        bodyHtml += '<tr class="settings-separator"><td colspan="' + (scenarios.length + 2) + '">설정값</td></tr>';

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
     * 비교 차트 렌더링
     */
    function renderComparisonCharts() {
        const colors = [
            'rgba(59, 130, 246, 0.8)',   // 파랑
            'rgba(16, 185, 129, 0.8)',   // 초록
            'rgba(245, 158, 11, 0.8)',   // 주황
            'rgba(239, 68, 68, 0.8)',    // 빨강
            'rgba(139, 92, 246, 0.8)'    // 보라
        ];

        const borderColors = [
            'rgba(59, 130, 246, 1)',
            'rgba(16, 185, 129, 1)',
            'rgba(245, 158, 11, 1)',
            'rgba(239, 68, 68, 1)',
            'rgba(139, 92, 246, 1)'
        ];

        // 기존 차트 파괴
        Object.values(comparisonCharts).forEach(chart => {
            if (chart) chart.destroy();
        });
        comparisonCharts = {};

        const labels = scenarios.map(s => s.name);

        // 1. 생산량 비교 차트
        const productionCtx = document.getElementById('chart-scenario-production');
        if (productionCtx) {
            comparisonCharts.production = new Chart(productionCtx, {
                type: 'bar',
                data: {
                    labels: labels,
                    datasets: [{
                        label: '총 생산량',
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

        // 2. PPH 비교 차트
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

        // 3. 가동률 비교 차트 (레이더)
        const utilCtx = document.getElementById('chart-scenario-util');
        if (utilCtx) {
            comparisonCharts.util = new Chart(utilCtx, {
                type: 'radar',
                data: {
                    labels: ['로봇 가동률', '재봉기 가동률', '생산 효율'],
                    datasets: scenarios.map((s, i) => ({
                        label: s.name,
                        data: [
                            s.results.robotUtilization,
                            s.results.machineUtilization,
                            Math.min(100, s.results.pph * 2) // PPH를 0-100 스케일로 변환
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
     * 시나리오 저장 다이얼로그 열기
     */
    function openSaveDialog(settings, results) {
        const defaultName = `시나리오 ${scenarios.length + 1}`;
        const name = prompt('시나리오 이름을 입력하세요:', defaultName);

        if (name === null) return; // 취소

        if (saveScenario(name || defaultName, settings, results)) {
            // 저장 성공 - 비교 섹션으로 스크롤
            const section = document.getElementById('scenario-comparison-section');
            if (section) {
                section.scrollIntoView({ behavior: 'smooth', block: 'start' });
            }
        }
    }

    /**
     * 차트 파괴 (모드 변경 시)
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