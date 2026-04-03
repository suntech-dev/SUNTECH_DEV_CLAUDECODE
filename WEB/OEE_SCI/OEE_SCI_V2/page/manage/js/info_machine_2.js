
/**
 * info_machine_2.js — 기계(Machine) 관리 페이지 전용 모듈 (ES Module)
 *
 * 내보내기(export):
 *   - machineConfig : ResourceManager 설정 객체
 *   - initAdvancedFeatures, initRealTimeSearch, performSearch,
 *     initQuickActions, applyQuickFilter, initModalSteps
 *
 * 모달 스텝 (3단계):
 *   Step 1: 공장/라인/기계번호 입력 + 중복 확인
 *   Step 2: 타입/모델 선택
 *   Step 3: 목표 수량/상태 등 상세 설정
 *
 * beforeInit:
 *   - loadFactoryOptions: 공장 목록 + 공장 변경 시 라인 연쇄 드롭다운
 *   - loadModelOptions  : 기계 모델 목록
 *   - loadLineOptions   : 라인 목록 (필터용)
 *   - loadMachineOptions: 기계 목록 (필터용)
 *
 * 연쇄 드롭다운 (Cascading Dropdown):
 *   공장 선택 → 라인 목록 갱신 (updateLineOptions)
 *   공장+라인 선택 → 기계 목록 갱신 (updateMachineOptions)
 *
 * typeFilterSelect 기본값 처리:
 *   - 페이지 초기 로드 시 value='P'이면 state.typeFilter='P'로 강제 적용
 *   - 100ms 지연 후 loadData() 호출 (ResourceManager 초기화 완료 후 실행 보장)
 */

/**
 * 고급 기능 초기화 진입점
 * - typeFilterSelect 기본값 'P' 강제 설정: 초기 목록을 패턴재봉기(P)만 표시
 */
export function initAdvancedFeatures(resourceManager) {
    // typeFilterSelect 기본값 'P' 강제 적용
    const typeFilterSelect = document.getElementById('typeFilterSelect');
    if (typeFilterSelect && typeFilterSelect.value === 'P') {
        if (resourceManager && resourceManager.state) {
            resourceManager.state.typeFilter = 'P';
            // ResourceManager 초기화 완료 후 데이터 로드 (100ms 지연)
            setTimeout(() => {
                if (resourceManager.loadData) resourceManager.loadData();
            }, 100);
        }
    }

    initRealTimeSearch(resourceManager);
    initQuickActions();
    initModalSteps();
}

/**
 * 기계 리소스 설정 객체
 *
 * beforeInit: 4개 드롭다운 로드 (공장, 모델, 라인, 기계)
 *
 * 컬럼 설명:
 *   no                 : 순번
 *   idx                : DB PK (숨김)
 *   factory_name       : 소속 공장명
 *   line_name          : 소속 라인명
 *   machine_model_name : 모델명
 *   machine_no         : 기계 번호 (고유 식별자)
 *   design_process     : 배정된 공정 (없으면 'Not assigned' 회색)
 *   mac                : MAC 주소 (monospace code 스타일, 없으면 'Not set')
 *   type               : P=Computer Sewing(파란)/E=Embroidery(주황)
 *   target             : 일일 목표 생산량 (toLocaleString)
 *   status             : 사용 여부 배지
 *   app_ver            : 설치된 앱 버전 (OTA 관리용)
 *
 * filterConfig:
 *   - statusFilterSelect          : 사용 여부 필터
 *   - typeFilterSelect            : 타입 필터 (P/E)
 *   - factoryFilterSelect         : 공장 필터 (resets: lineFilter, machineFilter)
 *   - factoryLineFilterSelect     : 라인 필터 (resets: machineFilter)
 *   - factoryLineMachineFilterSelect: 기계 필터
 *   resets: 상위 필터 변경 시 하위 필터 상태를 자동 초기화
 */
export const machineConfig = {
    resourceName: 'Machine',
    apiEndpoint: 'proc/machine.php',
    entityId: 'idx',

    // ResourceManager 초기화 전 드롭다운 4개 순차 로드
    async beforeInit(api) {
        await loadFactoryOptions(api);
        await loadModelOptions(api);
        await loadLineOptions(api);
        await loadMachineOptions(api);
    },

    columnConfig: [
        {
            key: 'no',
            label: 'NO.',
            sortable: false,
            render: (item, index) => `<span style="color: var(--sap-text-secondary);">${index}</span>`,
            width: '60px'
        },
        {
            key: 'idx',
            label: 'IDX',
            sortable: true,
            sortKey: 'm.idx',
            visible: false
        },
        {
            key: 'factory_name',
            label: 'Factory Name',
            sortable: true,
            sortKey: 'f.factory_name'
        },
        {
            key: 'line_name',
            label: 'Line Name',
            sortable: true,
            sortKey: 'l.line_name'
        },
        {
            key: 'machine_model_name',
            label: 'Model',
            sortable: true,
            sortKey: 'mm.machine_model_name'
        },
        {
            key: 'machine_no',
            label: 'Machine No',
            sortable: true,
            sortKey: 'm.machine_no'
        },
        {
            key: 'design_process',
            label: 'Design Process',
            sortable: true,
            sortKey: 'dp.design_process',
            // 공정 배정 여부에 따라 표시 방식 분기
            render: (item) => item.design_process
                ? `<span style="color: var(--sap-text-primary); font-weight: 500;">${item.design_process}</span>`
                : '<span style="color: var(--sap-text-secondary);">Not assigned</span>'
        },
        {
            key: 'mac',
            label: 'MAC Address',
            sortable: true,
            sortKey: 'm.mac',
            // MAC 주소: 코드 스타일 배경으로 가독성 향상
            render: (item) => item.mac
                ? `<code style="font-family: monospace; background: var(--sap-surface-3); padding: 2px 6px; border-radius: 4px;">${item.mac}</code>`
                : '<span style="color: var(--sap-text-secondary);">Not set</span>'
        },
        {
            key: 'type',
            label: 'Type',
            sortable: true,
            sortKey: 'm.type',
            render: (item) => {
                if (item.type === 'P') return '<span style="color: var(--sap-brand-primary); font-weight: 600;">Computer Sewing Machine</span>';
                if (item.type === 'E') return '<span style="color: var(--sap-status-warning); font-weight: 600;">Embroidery Machine</span>';
                return '<span style="color: var(--sap-text-secondary);">Unknown</span>';
            }
        },
        {
            key: 'target',
            label: 'Target',
            sortable: true,
            sortKey: 'm.target',
            render: (item) => (parseInt(item.target) || 0).toLocaleString()
        },
        {
            key: 'status',
            label: 'Status',
            sortable: true,
            sortKey: 'm.status',
            render: (item) => {
                const isActive = item.status === 'Y';
                return `<span class="fiori-badge fiori-badge--${isActive ? 'success' : 'warning'}">${isActive ? 'Used' : 'Unused'}</span>`;
            },
            width: '90px'
        },
        {
            key: 'app_ver',
            label: 'APP VERSION',
            sortable: true,
            sortKey: 'm.app_ver'
        }
    ],

    filterConfig: [
        { elementId: 'statusFilterSelect',              paramName: 'status_filter',  stateKey: 'statusFilter' },
        { elementId: 'typeFilterSelect',                paramName: 'type_filter',    stateKey: 'typeFilter' },
        // resets: 공장 변경 시 라인/기계 필터 상태 초기화
        { elementId: 'factoryFilterSelect',             paramName: 'factory_filter', stateKey: 'factoryFilter',  resets: ['lineFilter', 'machineFilter'] },
        // resets: 라인 변경 시 기계 필터 상태 초기화
        { elementId: 'factoryLineFilterSelect',         paramName: 'line_filter',    stateKey: 'lineFilter',     resets: ['machineFilter'] },
        { elementId: 'factoryLineMachineFilterSelect',  paramName: 'machine_filter', stateKey: 'machineFilter' }
    ]
};


/**
 * 실시간 검색 초기화 (300ms 디바운스)
 */
export function initRealTimeSearch(resourceManager) {
    const searchInput = document.getElementById('realTimeSearch');
    const searchClear = document.getElementById('searchClear');
    let searchTimeout;

    if (!searchInput || !searchClear) return;

    searchInput.addEventListener('input', (e) => {
        const value = e.target.value.trim();
        searchClear.classList.toggle('visible', !!value);
        clearTimeout(searchTimeout);
        searchTimeout = setTimeout(() => performSearch(value, resourceManager), 300);
    });

    searchClear.addEventListener('click', () => {
        searchInput.value = '';
        searchClear.classList.remove('visible');
        performSearch('', resourceManager);
        searchInput.focus();
    });

    searchInput.addEventListener('keydown', (e) => {
        if (e.key === 'Enter') {
            clearTimeout(searchTimeout);
            performSearch(searchInput.value.trim(), resourceManager);
        }
    });
}

/**
 * 검색 실행
 */
export function performSearch(query, resourceManager) {
    if (resourceManager && resourceManager.state) {
        resourceManager.state.searchQuery = query;
        resourceManager.state.currentPage = 1;
    }
    if (resourceManager && resourceManager.loadData) {
        resourceManager.loadData();
    }
}


/**
 * 빠른 필터 버튼 초기화
 */
export function initQuickActions() {
    const actionBtns = document.querySelectorAll('.quick-action-btn');
    actionBtns.forEach(btn => {
        btn.addEventListener('click', () => {
            actionBtns.forEach(b => b.classList.remove('active'));
            btn.classList.add('active');
            applyQuickFilter(btn.dataset.filter);
        });
    });
}

/**
 * 빠른 필터 적용
 */
export function applyQuickFilter(filter) {
    const statusSelect = document.getElementById('statusFilterSelect');
    if (!statusSelect) return;
    const map = { used: 'Y', unused: 'N', all: '' };
    statusSelect.value = map[filter] ?? '';
    statusSelect.dispatchEvent(new Event('change'));
}


/**
 * 모달 다단계 스텝 초기화 (3단계)
 *
 * Step 1: 기본 정보
 *   - factory_idx: 공장 선택 (필수)
 *   - line_idx: 라인 선택 (필수)
 *   - machine_no: 기계번호 (필수 + 중복 확인)
 *
 * Step 2: 기계 정보
 *   - type: 기계 타입 (필수)
 *   - machine_model_idx: 모델 선택 (필수)
 *
 * Step 3: 상세 설정
 *   - target: 목표 생산량 (숫자 형식 검증)
 */
export function initModalSteps() {
    const nextBtn       = document.getElementById('nextStep');
    const prevBtn       = document.getElementById('prevStep');
    const submitBtn     = document.getElementById('submitBtn');
    const stepIndicator = document.getElementById('stepIndicator');

    if (!nextBtn || !prevBtn || !submitBtn || !stepIndicator) return;

    let currentStep = 1;
    const totalSteps = 3;

    nextBtn.addEventListener('click', async () => {
        if (await validateCurrentStep(currentStep)) {
            currentStep++;
            showStep(currentStep);
        }
    });

    prevBtn.addEventListener('click', () => {
        currentStep--;
        showStep(currentStep);
    });

    // 스텝 인디케이터 클릭 직접 이동
    document.querySelectorAll('.form-step').forEach((step, index) => {
        step.style.cursor = 'pointer';
        step.title = `Step ${index + 1}`;
        step.addEventListener('click', async () => {
            const target = index + 1;
            if (target < currentStep) {
                currentStep = target;
                showStep(currentStep);
            } else if (target > currentStep) {
                let valid = true, tmp = currentStep;
                while (tmp < target && valid) {
                    valid = await validateCurrentStep(tmp);
                    if (valid) tmp++;
                }
                if (valid) { currentStep = target; showStep(currentStep); }
            }
        });
    });

    function showStep(step) {
        document.querySelectorAll('.form-section').forEach(s => {
            s.style.display = s.dataset.section == step ? 'block' : 'none';
        });
        prevBtn.style.display   = step > 1            ? 'inline-block' : 'none';
        nextBtn.style.display   = step < totalSteps   ? 'inline-block' : 'none';
        submitBtn.style.display = step === totalSteps ? 'inline-block' : 'none';
        stepIndicator.textContent = `Step ${step} of ${totalSteps}`;
        document.querySelectorAll('.form-step').forEach(s => {
            s.classList.toggle('active', parseInt(s.dataset.step) <= step);
        });
        // Step 3에는 별도 미리보기 없음
    }

    /**
     * 스텝별 유효성 검사
     * Step 1: 공장/라인/기계번호 필수 + 중복 확인
     * Step 2: 타입/모델 필수
     * Step 3: target 숫자 형식 검증
     */
    async function validateCurrentStep(step) {
        if (step === 1) {
            const factorySelect = document.getElementById('factory_idx');
            const lineSelect    = document.getElementById('line_idx');
            const machineNo     = document.getElementById('machine_no');

            if (factorySelect && !factorySelect.value) {
                alert('Please select a factory.');
                factorySelect.focus();
                return false;
            }
            if (lineSelect && !lineSelect.value) {
                alert('Please select a line.');
                lineSelect.focus();
                return false;
            }
            if (machineNo && !machineNo.value.trim()) {
                alert('Please enter the machine name.');
                machineNo.focus();
                return false;
            }
            try {
                // 기계번호 중복 확인
                const currentIdx = document.getElementById('resourceId').value || null;
                const url = `proc/machine.php?for=check-duplicate&machine_no=${encodeURIComponent(machineNo.value.trim())}${currentIdx ? `&current_idx=${currentIdx}` : ''}`;
                const result = await fetch(url).then(r => r.json());
                if (!result.success) { alert(result.message); return false; }
            } catch {
                alert('An error occurred while checking for duplicates.');
                return false;
            }
        }
        if (step === 2) {
            const typeSelect  = document.getElementById('type');
            const modelSelect = document.getElementById('machine_model_idx');

            if (typeSelect && !typeSelect.value) {
                alert('Please select the machine type.');
                typeSelect.focus();
                return false;
            }
            if (modelSelect && !modelSelect.value) {
                alert('Please select the machine model.');
                modelSelect.focus();
                return false;
            }
        }
        if (step === 3) {
            const target = document.getElementById('target');
            if (target && target.value && isNaN(parseInt(target.value))) {
                alert('Please enter a number for target quantity.');
                target.focus();
                return false;
            }
        }
        return true;
    }
}


/**
 * 공장 목록 로드 및 연쇄 드롭다운 이벤트 설정
 * - factoryFilterSelect: 목록 페이지 공장 필터 → 변경 시 라인 필터 갱신
 * - factory_idx: 모달 공장 선택 → 변경 시 라인 선택 드롭다운 갱신 (비활성화 해제)
 * - factoryLineFilterSelect 변경 시: 공장+라인 조합으로 기계 필터 갱신
 */
async function loadFactoryOptions(api) {
    try {
        const response = await api.getAll({ for: 'factories' });
        if (!response.success || !response.data) return;

        const factorySelect      = document.getElementById('factoryFilterSelect');
        const modalFactorySelect = document.getElementById('factory_idx');

        // 목록 필터 공장 드롭다운
        if (factorySelect) {
            factorySelect.innerHTML = '<option value="">All Factories</option>';
            response.data.forEach(f => {
                factorySelect.innerHTML += `<option value="${f.idx}">${f.factory_name}</option>`;
            });
            // 공장 변경 → 라인 필터 갱신 (연쇄)
            factorySelect.addEventListener('change', (e) => {
                updateLineOptions(api, e.target.value, 'factoryLineFilterSelect', 'All Lines');
            });
        }

        // 모달 공장 선택 드롭다운
        if (modalFactorySelect) {
            modalFactorySelect.innerHTML = '<option value="">Select Factory</option>';
            response.data.forEach(f => {
                modalFactorySelect.innerHTML += `<option value="${f.idx}">${f.factory_name}</option>`;
            });
            // 공장 변경 → 라인 선택 드롭다운 갱신 + disabled 해제
            modalFactorySelect.addEventListener('change', (e) => {
                const lineSelect = document.getElementById('line_idx');
                if (lineSelect) {
                    lineSelect.value    = '';
                    lineSelect.disabled = !e.target.value;  // 공장 선택 전까지 비활성
                }
                updateLineOptions(api, e.target.value, 'line_idx', 'Please select a line');
            });
        }

        // 라인 필터 변경 → 기계 필터 갱신 (공장+라인 조합)
        const lineFilterSelect = document.getElementById('factoryLineFilterSelect');
        if (lineFilterSelect) {
            lineFilterSelect.addEventListener('change', (e) => {
                const factoryVal = factorySelect ? factorySelect.value : '';
                updateMachineOptions(api, factoryVal, e.target.value, 'factoryLineMachineFilterSelect', 'All Machines');
            });
        }
    } catch (e) {
        console.error('Factory options load error:', e);
    }
}

/**
 * 기계 모델 목록 로드 (proc/machine.php?for=models)
 * - machine_model_idx: 모달 내 모델 선택 드롭다운
 */
async function loadModelOptions(api) {
    try {
        const response = await api.getAll({ for: 'models' });
        if (!response.success || !response.data) return;

        const modelSelect = document.getElementById('machine_model_idx');
        if (modelSelect) {
            modelSelect.innerHTML = '<option value="">Select Model</option>';
            response.data.forEach(m => {
                modelSelect.innerHTML += `<option value="${m.idx}">${m.machine_model_name}</option>`;
            });
        }
    } catch (e) {
        console.error('Model options load error:', e);
    }
}

/**
 * 라인 목록 로드 (proc/machine.php?for=lines, 전체)
 * - factoryLineFilterSelect: 목록 페이지 라인 필터 드롭다운 초기 옵션 채우기
 */
async function loadLineOptions(api) {
    try {
        const response = await api.getAll({ for: 'lines' });
        if (!response.success || !response.data) return;

        const lineFilterSelect = document.getElementById('factoryLineFilterSelect');
        if (lineFilterSelect) {
            lineFilterSelect.innerHTML = '<option value="">All Lines</option>';
            response.data.forEach(l => {
                lineFilterSelect.innerHTML += `<option value="${l.idx}">${l.line_name}</option>`;
            });
        }
    } catch (e) {
        console.error('Line options load error:', e);
    }
}

/**
 * 기계 목록 로드 (전체)
 * - factoryLineMachineFilterSelect: 기계 필터 드롭다운 초기 옵션 채우기
 * - "machine_no (model_name)" 형식으로 표시하여 구분 가능하게 함
 */
async function loadMachineOptions(api) {
    try {
        const response = await api.getAll({});
        if (!response.success || !response.data) return;

        const machineFilterSelect = document.getElementById('factoryLineMachineFilterSelect');
        if (machineFilterSelect) {
            machineFilterSelect.innerHTML = '<option value="">All Machines</option>';
            response.data.forEach(m => {
                machineFilterSelect.innerHTML += `<option value="${m.idx}">${m.machine_no} (${m.machine_model_name || 'No Model'})</option>`;
            });
        }
    } catch (e) {
        console.error('Machine options load error:', e);
    }
}

/**
 * 라인 드롭다운 동적 갱신 (공장 변경 시 연쇄 호출)
 * - factoryId: 특정 공장의 라인만 조회 (없으면 전체)
 * - lineElementId: 업데이트할 select 요소 ID
 * - initialText: 첫 번째 옵션 텍스트 ("All Lines" 또는 "Please select a line")
 * - preserveValue: 갱신 후 유지할 기존 선택값 (null이면 유지 안 함)
 * - 갱신 중 disabled → 완료 후 enabled (사용자 조작 방지)
 * - 기존 선택값이 새 옵션에 존재하면 재선택 후 change 이벤트 발생 (하위 드롭다운 연쇄 갱신)
 */
async function updateLineOptions(api, factoryId, lineElementId, initialText, preserveValue = null) {
    const lineSelect = document.getElementById(lineElementId);
    if (!lineSelect) return;

    lineSelect.disabled = true;
    try {
        let url = 'proc/machine.php?for=lines';
        if (factoryId) url += `&factory_id=${factoryId}`;

        const res = await fetch(url).then(r => r.json());
        const currentValue = preserveValue || lineSelect.value;

        lineSelect.innerHTML = `<option value="">${initialText}</option>`;
        if (res.success && res.data) {
            res.data.forEach(l => {
                lineSelect.innerHTML += `<option value="${l.idx}">${l.line_name}</option>`;
            });
            // 기존 선택값이 새 목록에 있으면 재선택 + 하위 드롭다운 연쇄 갱신
            if (currentValue && lineSelect.querySelector(`option[value="${currentValue}"]`)) {
                lineSelect.value = currentValue;
                setTimeout(() => lineSelect.dispatchEvent(new Event('change', { bubbles: true })), 50);
            }
        }
    } catch (e) {
        console.error('updateLineOptions error:', e);
        lineSelect.innerHTML = `<option value="">${initialText}</option>`;
    }
    lineSelect.disabled = false;
}

/**
 * 기계 드롭다운 동적 갱신 (공장+라인 변경 시 연쇄 호출)
 * - factoryId, lineId: 복합 필터 조건
 * - machineElementId: 업데이트할 select 요소 ID
 * - api.getAll()로 서버사이드 필터링 (factory_filter, line_filter 파라미터)
 * - 갱신 중 disabled → 완료 후 enabled
 */
async function updateMachineOptions(api, factoryId, lineId, machineElementId, initialText) {
    const machineSelect = document.getElementById(machineElementId);
    if (!machineSelect) return;

    machineSelect.disabled = true;
    try {
        const params = {};
        if (factoryId) params.factory_filter = factoryId;
        if (lineId)    params.line_filter    = lineId;

        const res = await api.getAll(params);
        machineSelect.innerHTML = `<option value="">${initialText}</option>`;
        if (res.success && res.data) {
            res.data.forEach(m => {
                machineSelect.innerHTML += `<option value="${m.idx}">${m.machine_no} (${m.machine_model_name || 'No Model'})</option>`;
            });
        }
    } catch (e) {
        console.error('updateMachineOptions error:', e);
        machineSelect.innerHTML = `<option value="">${initialText}</option>`;
    }
    machineSelect.disabled = false;
}
