
/**
 * info_line_2.js — 라인(Line) 관리 페이지 전용 설정 및 기능 모듈 (ES Module)
 *
 * 내보내기(export):
 *   - lineConfig : ResourceManager에 전달하는 설정 객체 (API, 컬럼, 필터, beforeInit 정의)
 *   - initAdvancedFeatures(resourceManager) : 실시간 검색, 빠른 필터, 모달 스텝 초기화
 *   - initRealTimeSearch, performSearch, initQuickActions, applyQuickFilter, initModalSteps
 *
 * 모달 스텝 (3단계):
 *   Step 1: 기본 정보 (공장 선택, 라인명 입력 + 중복 확인)
 *   Step 2: 수치 정보 (MP, Target 입력 + 범위 검증)
 *   Step 3: 상태/비고 + 미리보기 (factory, line, mp, target, status 표시)
 *
 * beforeInit:
 *   - ResourceManager 초기화 전에 loadFactoryOptions() 호출
 *   - 공장 필터 드롭다운과 모달 공장 선택 드롭다운을 API로 채움
 */

/**
 * 고급 기능 초기화 진입점
 */
export function initAdvancedFeatures(resourceManager) {
    initRealTimeSearch(resourceManager);
    initQuickActions();
    initModalSteps();
}

/**
 * 라인 리소스 설정 객체
 *
 * beforeInit: ResourceManager 초기화 이전에 실행되는 비동기 훅
 *   - loadFactoryOptions(): 공장 목록을 미리 로드하여 필터/모달 드롭다운 초기화
 *
 * 컬럼 설명:
 *   no           : 순번 (정렬 불가)
 *   idx          : DB PK (숨김)
 *   factory_name : 소속 공장명
 *   line_name    : 라인명 (굵게 표시)
 *   machine_count: 소속 기계 수 (집계값)
 *   mp           : Man Power (인원 수)
 *   line_target  : 일일 목표 생산량 (toLocaleString으로 천단위 콤마)
 *   status       : 사용 여부 배지
 *   remark       : 비고
 *
 * filterConfig:
 *   - statusFilterSelect: 사용 여부 필터
 *   - factoryFilterSelect: 공장 필터 (라인 목록을 공장별로 필터링)
 */
export const lineConfig = {
    resourceName: 'Line',
    apiEndpoint: 'proc/line.php',
    entityId: 'idx',

    // ResourceManager 초기화 전 공장 옵션 로드 (필터/모달 드롭다운에 공장 목록 채우기)
    async beforeInit(api) {
        await loadFactoryOptions(api);
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
            sortable: false,
            sortKey: 'l.idx',
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
            sortKey: 'l.line_name',
            render: (item) => `<strong style="color: var(--sap-text-primary);">${item.line_name}</strong>`
        },
        {
            key: 'machine_count',
            label: 'Machines',
            sortable: true,
            sortKey: 'machine_count',
            render: (item) => {
                const count = item.machine_count || 0;
                return `<span style="color: var(--sap-text-primary); font-weight: 500;">${count}</span>`;
            },
            width: '90px'
        },
        {
            key: 'mp',
            label: 'MP',
            sortable: true,
            sortKey: 'l.mp',
            render: (item) => {
                const mp = parseInt(item.mp) || 0;
                return `<span style="color: var(--sap-text-primary); font-weight: 500;">${mp}</span>`;
            },
            width: '70px'
        },
        {
            key: 'line_target',
            label: 'Target',
            sortable: true,
            sortKey: 'l.line_target',
            render: (item) => {
                const target = parseInt(item.line_target) || 0;
                // toLocaleString(): 천단위 콤마 표시 (예: 1,200)
                return `<span style="color: var(--sap-text-primary);">${target.toLocaleString()}</span>`;
            }
        },
        {
            key: 'status',
            label: 'Status',
            sortable: true,
            sortKey: 'l.status',
            render: (item) => {
                const isActive = item.status === 'Y';
                return `
          <span class="fiori-badge fiori-badge--${isActive ? 'success' : 'warning'}"
                style="display: inline-flex; align-items: center; gap: var(--sap-spacing-xs);">
            ${isActive ? 'Used' : 'Unused'}
          </span>
        `;
            },
            width: '120px'
        },
        {
            key: 'remark',
            label: 'Remark',
            sortable: true,
            sortKey: 'remark',
            render: (item) => {
                const remark = item.remark || '-';
                return `<span style="color: ${remark === '-' ? 'var(--sap-text-secondary)' : 'var(--sap-text-primary)'};">${remark}</span>`;
            }
        }
    ],

    filterConfig: [
        {
            elementId: 'statusFilterSelect',
            paramName: 'status_filter',
            stateKey: 'statusFilter'
        },
        {
            elementId: 'factoryFilterSelect',
            paramName: 'factory_filter',
            stateKey: 'factoryFilter'
        }
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
 * 검색 실행 (searchQuery 업데이트 후 1페이지로 리셋하여 데이터 재로드)
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
 * 빠른 필터 버튼 초기화 (Used / Unused / All)
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
 * 빠른 필터 적용 (statusFilterSelect 값 변경 → change 이벤트 발생 → 자동 재로드)
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
 *   - line_name: 라인명 입력 (필수)
 *   - 중복 확인: proc/line.php?for=check-duplicate (factory_idx + line_name 조합 중복 체크)
 *
 * Step 2: 수치 정보
 *   - mp: Man Power (필수, 0 이상)
 *   - line_target: 일일 목표 생산량 (필수, 0 이상)
 *
 * Step 3: 미리보기
 *   - previewName(공장명), previewLine(라인명), previewMP, previewTarget, previewStatus
 *   - factory_idx / line_name / mp / line_target / status 변경 시 실시간 반영
 */
export function initModalSteps() {
    const nextBtn = document.getElementById('nextStep');
    const prevBtn = document.getElementById('prevStep');
    const submitBtn = document.getElementById('submitBtn');
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

    // 스텝 인디케이터 클릭 직접 이동 (이전 스텝은 즉시, 이후 스텝은 검증 후)
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

    /** 해당 스텝만 표시하고 버튼 가시성 업데이트 */
    function showStep(step) {
        document.querySelectorAll('.form-section').forEach(s => {
            s.style.display = s.dataset.section == step ? 'block' : 'none';
        });
        prevBtn.style.display = step > 1 ? 'inline-block' : 'none';
        nextBtn.style.display = step < totalSteps ? 'inline-block' : 'none';
        submitBtn.style.display = step === totalSteps ? 'inline-block' : 'none';
        stepIndicator.textContent = `Step ${step} of ${totalSteps}`;
        document.querySelectorAll('.form-step').forEach(s => {
            s.classList.toggle('active', parseInt(s.dataset.step) <= step);
        });
        updatePreview();
    }

    /**
     * 스텝별 유효성 검사
     * Step 1: 공장 선택 + 라인명 입력 + 중복 확인 API
     * Step 2: MP, Target 양수 검증
     */
    async function validateCurrentStep(step) {
        if (step === 1) {
            const factory = document.getElementById('factory_idx').value;
            const lineName = document.getElementById('line_name').value.trim();
            if (!factory || !lineName) {
                alert('Please fill in all required fields in Basic Information.');
                return false;
            }
            try {
                const currentIdx = document.getElementById('resourceId').value || null;
                // 중복 확인: 같은 공장 내 동일 라인명 체크
                const url = `proc/line.php?for=check-duplicate&factory_idx=${factory}&line_name=${encodeURIComponent(lineName)}${currentIdx ? `&current_idx=${currentIdx}` : ''}`;
                const result = await fetch(url).then(r => r.json());
                if (!result.success) { alert(result.message); return false; }
            } catch {
                alert('An error occurred while checking for duplicates.');
                return false;
            }
        }
        if (step === 2) {
            const mp = document.getElementById('mp').value;
            const target = document.getElementById('line_target').value;
            if (!mp || mp < 0 || !target || target < 0) {
                alert('Please enter valid values for Man Power and Line Target.');
                return false;
            }
        }
        return true;
    }

    /** Step 3 미리보기 업데이트 */
    function updatePreview() {
        const factorySelect = document.getElementById('factory_idx');
        const factoryName = factorySelect?.selectedOptions[0]?.textContent || '-';
        const lineName = document.getElementById('line_name')?.value || '-';
        const mp = document.getElementById('mp')?.value || '-';
        const target = document.getElementById('line_target')?.value || '-';
        const statusText = document.getElementById('status')?.selectedOptions[0]?.textContent || '-';

        const pName = document.getElementById('previewName');
        const pLine = document.getElementById('previewLine');
        const pMP = document.getElementById('previewMP');
        const pTarget = document.getElementById('previewTarget');
        const pStatus = document.getElementById('previewStatus');

        if (pName) pName.textContent = factoryName;
        if (pLine) pLine.textContent = lineName;
        if (pMP) pMP.textContent = mp;
        if (pTarget) pTarget.textContent = target;
        if (pStatus) pStatus.textContent = statusText;
    }

    // 입력 변경 시 미리보기 실시간 반영
    document.getElementById('factory_idx')?.addEventListener('change', updatePreview);
    document.getElementById('line_name')?.addEventListener('input', updatePreview);
    document.getElementById('mp')?.addEventListener('input', updatePreview);
    document.getElementById('line_target')?.addEventListener('input', updatePreview);
    document.getElementById('status')?.addEventListener('change', updatePreview);

    // 신규 등록 버튼 클릭 시 Step 1로 리셋
    document.getElementById('addBtn')?.addEventListener('click', () => {
        currentStep = 1;
        showStep(1);
    });

    // 수정 모달 열릴 때 Step 1로 자동 리셋
    const resourceModal = document.getElementById('resourceModal');
    if (resourceModal) {
        new MutationObserver((mutations) => {
            mutations.forEach((m) => {
                if (m.type === 'attributes' && m.attributeName === 'class'
                    && resourceModal.classList.contains('show')) {
                    currentStep = 1;
                    showStep(1);
                }
            });
        }).observe(resourceModal, { attributes: true });
    }
}


/**
 * 공장 옵션 로드 (proc/line.php?for=factories)
 * - factoryFilterSelect: 목록 페이지 공장 필터 드롭다운
 * - factory_idx: 모달 내 공장 선택 드롭다운
 * - beforeInit에서 호출되어 페이지 초기화 시 한 번만 실행
 */
async function loadFactoryOptions() {
    try {
        const response = await fetch('proc/line.php?for=factories');
        const result = await response.json();

        if (result.success && result.data) {
            const filterSelect = document.getElementById('factoryFilterSelect');
            const modalSelect = document.getElementById('factory_idx');

            // 목록 필터 드롭다운 채우기
            if (filterSelect) {
                filterSelect.innerHTML = '<option value="">All Factories</option>';
                result.data.forEach(f => {
                    filterSelect.innerHTML += `<option value="${f.idx}">${f.factory_name}</option>`;
                });
            }

            // 모달 공장 선택 드롭다운 채우기
            if (modalSelect) {
                modalSelect.innerHTML = '<option value="">Select Factory</option>';
                result.data.forEach(f => {
                    modalSelect.innerHTML += `<option value="${f.idx}">${f.factory_name}</option>`;
                });
            }
        }
    } catch (error) {
        console.error('Factory options load error:', error);
    }
}
