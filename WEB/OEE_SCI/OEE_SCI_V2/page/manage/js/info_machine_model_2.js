
/**
 * info_machine_model_2.js — 기계 모델(Machine Model) 관리 페이지 전용 모듈 (ES Module)
 *
 * 내보내기(export):
 *   - machineModelConfig : ResourceManager 설정 객체
 *   - initAdvancedFeatures, initRealTimeSearch, performSearch,
 *     initQuickActions, applyQuickFilter, initModalSteps
 *
 * 모달 스텝 (2단계):
 *   Step 1: 기본 정보 (모델명 입력 + 중복 확인)
 *   Step 2: 상태/비고 + 미리보기 (previewName, previewStatus)
 *
 * 기계 모델 타입:
 *   P = Computer Sewing Machine (패턴재봉기, 파란색 표시)
 *   E = Embroidery Machine (자수기, 주황 표시)
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
 * 기계 모델 리소스 설정 객체
 *
 * 컬럼 설명:
 *   no                : 순번 (정렬 불가)
 *   idx               : DB PK (숨김)
 *   machine_model_name: 모델명 (굵게 표시)
 *   machine_count     : 해당 모델 기계 수 (집계값)
 *   type              : P=Computer Sewing Machine(파란색) / E=Embroidery Machine(주황)
 *   status            : 사용 여부 배지
 *   remark            : 비고
 */
export const machineModelConfig = {
    resourceName: 'Machine_Model',
    apiEndpoint: 'proc/machine_model.php',
    entityId: 'idx',

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
            label: 'ID',
            sortable: false,
            sortKey: 'idx',
            visible: false
        },
        {
            key: 'machine_model_name',
            label: 'Machine Model Name',
            sortable: true,
            sortKey: 'machine_model_name',
            render: (item) => `
        <div style="display: flex; align-items: center; gap: var(--sap-spacing-xs);">
          <strong style="color: var(--sap-text-primary);">${item.machine_model_name}</strong>
        </div>
      `
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
            key: 'type',
            label: 'Type',
            sortable: true,
            sortKey: 'type',
            render: (item) => {
                // 타입별 색상 구분: P=파란색(브랜드), E=주황(경고색)
                if (item.type === 'P') return '<span style="color: var(--sap-brand-primary); font-weight: 600;">Computer Sewing Machine</span>';
                if (item.type === 'E') return '<span style="color: var(--sap-status-warning); font-weight: 600;">Embroidery Machine</span>';
                return '<span style="color: var(--sap-text-secondary);">Unknown</span>';
            },
            width: '200px'
        },
        {
            key: 'status',
            label: 'Status',
            sortable: true,
            sortKey: 'status',
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
 * 모달 다단계 스텝 초기화 (2단계)
 *
 * Step 1: 기본 정보 (모델명 + 중복 확인)
 * Step 2: 상세 설정 + 미리보기
 *
 * 주의: prevBtn은 'prevBtn' ID로 조회하나 없을 수도 있어 optional chaining 사용
 * (machine_model 모달은 prevStep ID를 가질 수 있으므로 showStep 내에서 별도 조회)
 */
export function initModalSteps() {
    const nextBtn = document.getElementById('nextStep');
    const prevBtn = document.getElementById('prevBtn');  // 일부 페이지에서는 다른 ID 사용
    const submitBtn = document.getElementById('submitBtn');
    const stepIndicator = document.getElementById('stepIndicator');

    if (!nextBtn || !submitBtn || !stepIndicator) return;

    let currentStep = 1;
    const totalSteps = 2;

    nextBtn.addEventListener('click', async () => {
        if (await validateCurrentStep(currentStep)) {
            currentStep++;
            showStep(currentStep);
        }
    });

    // prevBtn이 있는 경우에만 이벤트 등록 (방어적 처리)
    if (prevBtn) {
        prevBtn.addEventListener('click', () => {
            currentStep--;
            showStep(currentStep);
        });
    }

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
        // prevStep ID로 이전 버튼 조회 (prevBtn과 별개로 처리)
        const prev = document.getElementById('prevStep');
        if (prev) prev.style.display = step > 1 ? 'inline-block' : 'none';
        nextBtn.style.display = step < totalSteps ? 'inline-block' : 'none';
        submitBtn.style.display = step === totalSteps ? 'inline-block' : 'none';
        stepIndicator.textContent = `Step ${step} of ${totalSteps}`;
        document.querySelectorAll('.form-step').forEach(s => {
            s.classList.toggle('active', parseInt(s.dataset.step) <= step);
        });
        updatePreview();
    }

    /**
     * Step 1: 모델명 필수 + 중복 확인 API
     */
    async function validateCurrentStep(step) {
        if (step === 1) {
            const name = document.getElementById('machine_model_name').value.trim();
            if (!name) { alert('Please enter the Machine Model name.'); return false; }
            try {
                const currentIdx = document.getElementById('resourceId').value || null;
                const url = `proc/machine_model.php?for=check-duplicate&machine_model_name=${encodeURIComponent(name)}${currentIdx ? `&current_idx=${currentIdx}` : ''}`;
                const result = await fetch(url).then(r => r.json());
                if (!result.success) { alert(result.message); return false; }
            } catch {
                alert('An error occurred while checking for duplicates.');
                return false;
            }
        }
        return true;
    }

    /** Step 2 미리보기 업데이트 */
    function updatePreview() {
        const name = document.getElementById('machine_model_name')?.value || '-';
        const status = document.getElementById('status')?.selectedOptions[0]?.textContent || '-';
        const pName = document.getElementById('previewName');
        const pStat = document.getElementById('previewStatus');
        if (pName) pName.textContent = name;
        if (pStat) pStat.textContent = status;
    }

    document.getElementById('machine_model_name')?.addEventListener('input', updatePreview);
    document.getElementById('status')?.addEventListener('change', updatePreview);

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
