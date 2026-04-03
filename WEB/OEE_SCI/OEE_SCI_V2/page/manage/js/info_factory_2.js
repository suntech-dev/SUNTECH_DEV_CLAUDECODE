
/**
 * info_factory_2.js — 공장(Factory) 관리 페이지 전용 설정 및 기능 모듈 (ES Module)
 *
 * 내보내기(export):
 *   - factoryConfig : ResourceManager에 전달하는 설정 객체 (API, 컬럼, 필터 정의)
 *   - initAdvancedFeatures(resourceManager) : 실시간 검색, 빠른 필터, 모달 스텝 초기화
 *   - initRealTimeSearch, performSearch, initQuickActions, applyQuickFilter, initModalSteps
 *
 * 모달 스텝:
 *   Step 1: 공장명 입력 + 중복 확인 (API 비동기 체크)
 *   Step 2: 상태/비고 입력 + 미리보기 (previewName, previewStatus)
 */

/**
 * 고급 기능 초기화 진입점
 * - resourceManager: 공통 CRUD 프레임워크 인스턴스
 */
export function initAdvancedFeatures(resourceManager) {
    initRealTimeSearch(resourceManager);
    initQuickActions();
    initModalSteps();
}

/**
 * 공장 리소스 설정 객체
 * - resourceName : API 통신 및 알림 메시지에 사용되는 리소스 이름
 * - apiEndpoint  : 백엔드 REST API 경로
 * - entityId     : 기본 키 컬럼명 (수정/삭제 시 식별자)
 * - columnConfig : 테이블 컬럼 정의 (key, label, sortable, render, width, visible)
 * - filterConfig : 필터 드롭다운 연결 설정 (elementId → paramName → stateKey)
 *
 * 컬럼 설명:
 *   no           : 순번 (내림차순, 정렬 불가)
 *   idx          : DB Primary Key (숨김, 내부 식별용)
 *   factory_name : 공장명 (굵게 표시)
 *   line_count   : 소속 라인 수 (집계값, 정렬 가능)
 *   machine_count: 소속 기계 수 (집계값, 정렬 가능)
 *   total_mp     : 총 인원(Man Power) 수 (집계값)
 *   status       : 사용 여부 (Y→Used 초록 배지 / N→Unused 노랑 배지)
 *   remark       : 비고 (없으면 '-' 회색으로 표시)
 */
export const factoryConfig = {
    resourceName: 'Factory',
    apiEndpoint: 'proc/factory.php',
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
            label: 'IDX',
            sortable: true,
            sortKey: 'idx',
            visible: false  // 화면에 표시하지 않음 (내부 식별용)
        },
        {
            key: 'factory_name',
            label: 'Factory Name',
            sortable: true,
            sortKey: 'factory_name',
            render: (item) => `
        <div style="display: flex; align-items: center; gap: var(--sap-spacing-xs);">
          <strong style="color: var(--sap-text-primary);">${item.factory_name}</strong>
        </div>
      `
        },
        {
            key: 'line_count',
            label: 'Lines',
            sortable: true,
            sortKey: 'line_count',
            render: (item) => {
                const count = item.line_count || 0;
                return `<span style="color: var(--sap-text-primary); font-weight: 500;">${count}</span>`;
            },
            width: '80px'
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
            key: 'total_mp',
            label: 'MP',
            sortable: true,
            sortKey: 'total_mp',
            render: (item) => {
                const mp = item.total_mp || 0;
                return `<span style="color: var(--sap-text-primary); font-weight: 500;">${mp}</span>`;
            },
            width: '70px'
        },
        {
            key: 'status',
            label: 'Status',
            sortable: true,
            sortKey: 'status',
            render: (item) => {
                const isActive = item.status === 'Y';
                // fiori-badge: SAP Fiori 스타일 배지 컴포넌트
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
                // 비고 없으면 회색, 있으면 기본 텍스트 색상
                return `<span style="color: ${remark === '-' ? 'var(--sap-text-secondary)' : 'var(--sap-text-primary)'};">${remark}</span>`;
            }
        }
    ],

    // 필터 드롭다운 연결: elementId 변경 시 paramName으로 API 파라미터 전송, stateKey로 상태 추적
    filterConfig: [
        {
            elementId: 'statusFilterSelect',
            paramName: 'status_filter',
            stateKey: 'statusFilter'
        }
    ]
};


/**
 * 실시간 검색 초기화
 * - 입력 시 300ms 디바운스 후 검색 실행 (과도한 API 요청 방지)
 * - X 버튼 클릭 시 검색어 초기화 및 재검색
 * - Enter 키: 디바운스 취소 후 즉시 검색
 * - searchClear 버튼: 검색어가 있을 때만 'visible' 클래스 추가로 표시
 */
export function initRealTimeSearch(resourceManager) {
    const searchInput = document.getElementById('realTimeSearch');
    const searchClear = document.getElementById('searchClear');
    let searchTimeout;

    if (!searchInput || !searchClear) return;

    // 입력 이벤트: 300ms 디바운스 검색
    searchInput.addEventListener('input', (e) => {
        const value = e.target.value.trim();
        searchClear.classList.toggle('visible', !!value);
        clearTimeout(searchTimeout);
        searchTimeout = setTimeout(() => performSearch(value, resourceManager), 300);
    });

    // 검색어 초기화 버튼
    searchClear.addEventListener('click', () => {
        searchInput.value = '';
        searchClear.classList.remove('visible');
        performSearch('', resourceManager);
        searchInput.focus();
    });

    // Enter 키: 즉시 검색 (디바운스 타이머 취소)
    searchInput.addEventListener('keydown', (e) => {
        if (e.key === 'Enter') {
            clearTimeout(searchTimeout);
            performSearch(searchInput.value.trim(), resourceManager);
        }
    });
}

/**
 * 검색 실행
 * - resourceManager.state.searchQuery 업데이트 후 1페이지로 이동
 * - resourceManager.loadData() 호출로 API 재요청
 */
export function performSearch(query, resourceManager) {
    if (resourceManager && resourceManager.state) {
        resourceManager.state.searchQuery = query;
        resourceManager.state.currentPage = 1;  // 검색 시 첫 페이지로 리셋
    }
    if (resourceManager && resourceManager.loadData) {
        resourceManager.loadData();
    }
}


/**
 * 빠른 필터 버튼 초기화 (Used / Unused / All)
 * - .quick-action-btn 클릭 시 active 클래스 토글 + applyQuickFilter 호출
 * - btn.dataset.filter: 'used' | 'unused' | 'all' 값 사용
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
 * - statusFilterSelect 값 변경 후 change 이벤트 발생
 * - ResourceManager가 change 이벤트를 감지하여 자동으로 API 재요청
 * - map: 버튼 data-filter 값 → select option value 매핑
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
 * Step 1: 기본 정보 (공장명 입력 + 중복 확인)
 * Step 2: 상세 설정 (상태, 비고 + 미리보기)
 *
 * 동작 방식:
 * - nextBtn: validateCurrentStep() 통과 시 다음 스텝으로 이동
 * - prevBtn: 이전 스텝으로 즉시 이동 (검증 없음)
 * - .form-step 클릭: 이전 스텝은 즉시 이동, 이후 스텝은 중간 스텝 검증 후 이동
 * - showStep(step): data-section 속성으로 해당 섹션만 표시, 나머지 숨김
 * - updatePreview(): factory_name / status 변경 시 Step 2 미리보기 자동 갱신
 * - addBtn 클릭 시 Step 1로 리셋 (신규 등록 모달 열릴 때)
 */
export function initModalSteps() {
    const nextBtn = document.getElementById('nextStep');
    const prevBtn = document.getElementById('prevStep');
    const submitBtn = document.getElementById('submitBtn');
    const stepIndicator = document.getElementById('stepIndicator');

    if (!nextBtn || !prevBtn || !submitBtn || !stepIndicator) return;

    let currentStep = 1;
    const totalSteps = 2;

    // 다음 스텝 버튼: 현재 스텝 검증 통과 시에만 이동
    nextBtn.addEventListener('click', async () => {
        if (await validateCurrentStep(currentStep)) {
            currentStep++;
            showStep(currentStep);
        }
    });

    // 이전 스텝 버튼: 검증 없이 즉시 이동
    prevBtn.addEventListener('click', () => {
        currentStep--;
        showStep(currentStep);
    });

    // 스텝 인디케이터 클릭으로 직접 이동
    document.querySelectorAll('.form-step').forEach((step, index) => {
        step.style.cursor = 'pointer';
        step.title = `Step ${index + 1}`;
        step.addEventListener('click', async () => {
            const target = index + 1;
            if (target < currentStep) {
                // 이전 스텝: 즉시 이동
                currentStep = target;
                showStep(currentStep);
            } else if (target > currentStep) {
                // 이후 스텝: 중간 스텝 모두 검증 후 이동
                let valid = true, tmp = currentStep;
                while (tmp < target && valid) {
                    valid = await validateCurrentStep(tmp);
                    if (valid) tmp++;
                }
                if (valid) { currentStep = target; showStep(currentStep); }
            }
        });
    });

    /** 해당 스텝 표시 및 버튼 가시성 제어 */
    function showStep(step) {
        // data-section 값이 현재 step과 일치하는 섹션만 표시
        document.querySelectorAll('.form-section').forEach(s => {
            s.style.display = s.dataset.section == step ? 'block' : 'none';
        });
        prevBtn.style.display = step > 1 ? 'inline-block' : 'none';
        nextBtn.style.display = step < totalSteps ? 'inline-block' : 'none';
        submitBtn.style.display = step === totalSteps ? 'inline-block' : 'none';
        stepIndicator.textContent = `Step ${step} of ${totalSteps}`;
        // 완료된 스텝 인디케이터 active 표시 (현재 스텝 이하는 모두 active)
        document.querySelectorAll('.form-step').forEach(s => {
            s.classList.toggle('active', parseInt(s.dataset.step) <= step);
        });
        updatePreview();
    }

    /**
     * 현재 스텝 유효성 검사
     * Step 1: 공장명 필수 + 실시간 중복 확인 API 호출
     * - current_idx: 수정 모드에서 자기 자신 제외
     */
    async function validateCurrentStep(step) {
        if (step === 1) {
            const factoryName = document.getElementById('factory_name').value.trim();
            if (!factoryName) { alert('Please enter the factory name.'); return false; }
            try {
                const currentIdx = document.getElementById('resourceId').value || null;
                const url = `proc/factory.php?for=check-duplicate&factory_name=${encodeURIComponent(factoryName)}${currentIdx ? `&current_idx=${currentIdx}` : ''}`;
                const result = await fetch(url).then(r => r.json());
                if (!result.success) { alert(result.message); return false; }
            } catch {
                alert('An error occurred while checking for duplicates.');
                return false;
            }
        }
        return true;
    }

    /** Step 2 미리보기 업데이트: factory_name / status 변경 시 호출 */
    function updatePreview() {
        const name = document.getElementById('factory_name')?.value || '-';
        const status = document.getElementById('status')?.selectedOptions[0]?.textContent || '-';
        const pName = document.getElementById('previewName');
        const pStat = document.getElementById('previewStatus');
        if (pName) pName.textContent = name;
        if (pStat) pStat.textContent = status;
    }

    // 입력 변경 시 미리보기 실시간 반영
    document.getElementById('factory_name')?.addEventListener('input', updatePreview);
    document.getElementById('status')?.addEventListener('change', updatePreview);

    // 신규 등록 버튼 클릭 시 Step 1로 리셋
    document.getElementById('addBtn')?.addEventListener('click', () => {
        currentStep = 1;
        showStep(1);
    });
}
