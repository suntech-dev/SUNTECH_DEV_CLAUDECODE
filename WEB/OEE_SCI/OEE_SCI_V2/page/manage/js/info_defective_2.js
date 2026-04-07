
/**
 * info_defective_2.js — 불량 유형(Defective) 관리 페이지 전용 모듈 (ES Module)
 *
 * 내보내기(export):
 *   - defectiveConfig : ResourceManager 설정 객체
 *   - initAdvancedFeatures, initRealTimeSearch, performSearch,
 *     initQuickActions, applyQuickFilter, initModalSteps
 *
 * 불량 유형(Defective): 생산 중 발생하는 불량 종류 마스터 데이터
 *
 * 모달 스텝 (2단계):
 *   Step 1: 불량명 + shortcut 입력 + 이중 중복 확인
 *     - defective_name 중복 확인 (전체 고유)
 *     - defective_shortcut 중복 확인 (입력한 경우에만, 선택 입력)
 *   Step 2: 상태/비고 + 미리보기 (previewName, previewShortcut, previewStatus)
 *
 * shortcut: 장치에서 불량 입력 시 단축키로 사용되는 코드 (길이 > 10 시 long 클래스 추가)
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
 * 불량 유형 리소스 설정 객체
 *
 * 컬럼 설명:
 *   no                 : 순번
 *   idx                : DB PK (숨김)
 *   defective_name     : 불량명 (.defective-name-badge 스타일 적용)
 *   defective_shortcut : 단축키 (code 스타일 + 길이 > 10이면 long 클래스)
 *   total_count        : 총 발생 건수
 *   usage_rate         : 점유율 % (30% 초과=빨강, 15% 초과=주황, 이하=초록)
 *   status             : 사용 여부 (.defective-status-used / .defective-status-unused)
 *   remark             : 비고
 */
export const defectiveConfig = {
    resourceName: 'Defective',
    apiEndpoint: 'proc/defective.php',
    entityId: 'idx',

    columnConfig: [
        {
            key: 'no',
            label: 'NO.',
            sortable: false,
            render: (item, index) => `<span style="color: var(--sap-text-secondary);">${index}</span>`,
            width: '60px'
        },
        { key: 'idx', label: 'IDX', sortable: true, sortKey: 'idx', visible: false },
        {
            key: 'defective_name',
            label: 'Defective Name',
            sortable: true,
            sortKey: 'defective_name',
            render: (item) => `<span class="defective-name-badge">${item.defective_name}</span>`
        },
        {
            key: 'defective_shortcut',
            label: 'Shortcut',
            sortable: true,
            sortKey: 'defective_shortcut',
            render: (item) => {
                if (item.defective_shortcut && item.defective_shortcut.trim()) {
                    const s = item.defective_shortcut.trim();
                    // 단축키 길이가 11자 초과하면 'long' 클래스 추가 (LCD 최대 11자 기준)
                    return `<code class="shortcut-badge${s.length > 11 ? ' long' : ''}">${s}</code>`;
                }
                return '<span style="color: var(--sap-text-secondary);">No shortcut</span>';
            }
        },
        {
            key: 'total_count',
            label: 'Total Count',
            sortable: true,
            sortKey: 'total_count',
            render: (item) => `<span style="color: var(--sap-text-primary); font-weight: 500;">${item.total_count || 0}</span>`,
            width: '100px'
        },
        {
            key: 'usage_rate',
            label: 'Rate',
            sortable: true,
            sortKey: 'usage_rate',
            render: (item) => {
                const rate = item.usage_rate || 0;
                // 점유율 3단계 색상: >30% 빨강, >15% 주황, 이하 초록
                const color = rate > 30 ? 'var(--sap-status-error)' :
                    rate > 15 ? 'var(--sap-status-warning)' : 'var(--sap-status-success)';
                return `<span style="color: ${color}; font-weight: 500;">${rate}%</span>`;
            },
            width: '80px'
        },
        {
            key: 'status',
            label: 'Status',
            sortable: true,
            sortKey: 'status',
            render: (item) => item.status === 'Y' ?
                '<span class="defective-status-used">Used</span>' :
                '<span class="defective-status-unused">Unused</span>',
            width: '100px'
        },
        {
            key: 'remark',
            label: 'Remark',
            sortable: true,
            sortKey: 'remark',
            render: (item) => item.remark ||
                '<span style="color: var(--sap-text-secondary);">No remarks</span>'
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
 * Step 1 검증 순서:
 * 1. defective_name 입력 확인 (필수)
 * 2. defective_name 중복 확인 API (proc/defective.php?for=check-duplicate)
 * 3. defective_shortcut이 입력된 경우에만 shortcut 중복 확인 API
 *    (proc/defective.php?for=check-duplicate-shortcut)
 *    → shortcut은 선택 입력이므로 비어있으면 중복 체크 생략
 */
export function initModalSteps() {
    const nextBtn       = document.getElementById('nextStep');
    const prevBtn       = document.getElementById('prevStep');
    const submitBtn     = document.getElementById('submitBtn');
    const stepIndicator = document.getElementById('stepIndicator');

    if (!nextBtn || !prevBtn || !submitBtn || !stepIndicator) return;

    let currentStep = 1;
    const totalSteps = 2;

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
        updatePreview();
    }

    /**
     * Step 1 이중 중복 확인:
     * 1차: 불량명 중복 (필수)
     * 2차: shortcut 중복 (shortcut 입력된 경우에만)
     */
    async function validateCurrentStep(step) {
        if (step === 1) {
            const name = document.getElementById('defective_name').value.trim();
            if (!name) { alert('Please enter the defective name.'); return false; }

            // 1차 중복 확인: 불량명
            try {
                const idx = document.getElementById('resourceId').value || null;
                const url = `proc/defective.php?for=check-duplicate&defective_name=${encodeURIComponent(name)}${idx ? `&current_idx=${idx}` : ''}`;
                const result = await fetch(url).then(r => r.json());
                if (!result.success) { alert(result.message); return false; }
            } catch {
                alert('An error occurred while checking for duplicates.');
                return false;
            }

            // 2차 중복 확인: shortcut (입력된 경우에만)
            const shortcut = document.getElementById('defective_shortcut').value.trim();
            if (shortcut) {
                try {
                    const idx = document.getElementById('resourceId').value || null;
                    const url = `proc/defective.php?for=check-duplicate-shortcut&defective_shortcut=${encodeURIComponent(shortcut)}${idx ? `&current_idx=${idx}` : ''}`;
                    const result = await fetch(url).then(r => r.json());
                    if (!result.success) { alert(result.message); return false; }
                } catch {
                    alert('An error occurred while checking for shortcut duplicates.');
                    return false;
                }
            }
        }
        return true;
    }

    /** Step 2 미리보기 업데이트 */
    function updatePreview() {
        const name     = document.getElementById('defective_name')?.value || '-';
        const shortcut = document.getElementById('defective_shortcut')?.value || '-';
        const status   = document.getElementById('status')?.selectedOptions[0]?.textContent || '-';
        const pName    = document.getElementById('previewName');
        const pShort   = document.getElementById('previewShortcut');
        const pStatus  = document.getElementById('previewStatus');
        if (pName)   pName.textContent   = name;
        if (pShort)  pShort.textContent  = shortcut;
        if (pStatus) pStatus.textContent = status;
    }

    document.getElementById('defective_name')?.addEventListener('input', updatePreview);
    document.getElementById('defective_shortcut')?.addEventListener('input', updatePreview);
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
