
/**
 * info_downtime_2.js — 다운타임(Downtime) 유형 관리 페이지 전용 모듈 (ES Module)
 *
 * 내보내기(export):
 *   - downtimeConfig : ResourceManager 설정 객체
 *   - initAdvancedFeatures, initRealTimeSearch, performSearch,
 *     initQuickActions, applyQuickFilter, initModalSteps
 *
 * 다운타임(Downtime): 생산 중 발생하는 비가동 원인 유형 마스터 데이터
 *
 * 모달 스텝 (2단계):
 *   Step 1: 다운타임명 + shortcut 입력 + 이중 중복 확인
 *     - downtime_name 중복 확인 (전체 고유)
 *     - downtime_shortcut 중복 확인 (입력한 경우에만, 선택 입력)
 *   Step 2: 상태/비고 + 미리보기 (previewName, previewShortcut, previewStatus)
 *
 * 구조는 info_defective_2.js와 동일하나 대상 리소스가 다름
 */

/**
 * 고급 기능 초기화 진입점
 * ResourceManager 초기화 완료 후 호출되어 검색·필터·모달 기능을 활성화함
 */
export function initAdvancedFeatures(resourceManager) {
    initRealTimeSearch(resourceManager);  // 실시간 검색 초기화
    initQuickActions();                   // 빠른 필터 버튼 초기화
    initModalSteps();                     // 2단계 모달 스텝 초기화
}

/**
 * 다운타임 유형 리소스 설정 객체
 *
 * 컬럼 설명:
 *   no                : 순번 (정렬 불가, --sap-text-secondary 색상)
 *   idx               : DB PK (숨김, sortable)
 *   downtime_name     : 다운타임명 (굵게 표시)
 *   downtime_shortcut : 단축키 (code 스타일, 없으면 '-' 표시)
 *   status            : 사용 여부 (fiori-badge 배지, Y=success/N=warning)
 *   remark            : 비고 (없으면 '-', 색상 구분)
 */
export const downtimeConfig = {
    resourceName: 'Downtime',       // ResourceManager에서 표시/로깅용 이름
    apiEndpoint: 'proc/downtime.php',  // CRUD API 엔드포인트
    entityId: 'idx',                // 행 식별자 (PK 컬럼명)

    columnConfig: [
        {
            key: 'no',
            label: 'NO.',
            sortable: false,
            // 순번은 index(0-based) 기반으로 표시, 색상을 secondary로 흐리게 처리
            render: (item, index) => `<span style="color: var(--sap-text-secondary);">${index}</span>`,
            width: '60px'
        },
        { key: 'idx', label: 'IDX', sortable: false, sortKey: 'idx', visible: false },
        {
            key: 'downtime_name',
            label: 'Downtime Name',
            sortable: true,
            sortKey: 'downtime_name',
            // 다운타임명은 굵게 강조하여 주요 식별 정보임을 명확히 함
            render: (item) => `<strong style="color: var(--sap-text-primary);">${item.downtime_name}</strong>`
        },
        {
            key: 'downtime_shortcut',
            label: 'Shortcut',
            sortable: true,
            sortKey: 'downtime_shortcut',
            render: (item) => {
                if (item.downtime_shortcut && item.downtime_shortcut.trim()) {
                    const s = item.downtime_shortcut.trim();
                    // 단축키 길이가 13자 초과하면 'long' 클래스 추가 (LCD 최대 13자 기준)
                    return `<code class="shortcut-badge${s.length > 13 ? ' long' : ''}">${s}</code>`;
                }
                return '<span style="color: var(--sap-text-secondary);">-</span>';
            }
        },
        {
            key: 'status',
            label: 'Status',
            sortable: true,
            sortKey: 'status',
            render: (item) => {
                const isActive = item.status === 'Y';
                // 사용 중(Y)=success(초록), 미사용(N)=warning(주황) 배지
                return `<span class="fiori-badge fiori-badge--${isActive ? 'success' : 'warning'}">${isActive ? 'Used' : 'Unused'}</span>`;
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
                // 비고가 있으면 기본 텍스트 색상, 없으면 secondary 색상으로 흐리게 표시
                return `<span style="color: ${remark === '-' ? 'var(--sap-text-secondary)' : 'var(--sap-text-primary)'};">${remark}</span>`;
            }
        }
    ],

    // 필터: 사용 여부 드롭다운 (statusFilterSelect)
    filterConfig: [
        {
            elementId: 'statusFilterSelect',  // HTML 요소 ID
            paramName: 'status_filter',        // API 요청 파라미터명
            stateKey: 'statusFilter'           // ResourceManager 내부 상태 키
        }
    ]
};


/**
 * 실시간 검색 초기화 (300ms 디바운스)
 *
 * - input 이벤트: 300ms 대기 후 performSearch 호출 (타이핑 중 불필요한 요청 방지)
 * - searchClear 버튼: 입력값 있을 때만 표시(visible 클래스 토글), 클릭 시 검색 초기화
 * - Enter 키: 디바운스 없이 즉시 검색 실행
 */
export function initRealTimeSearch(resourceManager) {
    const searchInput = document.getElementById('realTimeSearch');
    const searchClear = document.getElementById('searchClear');
    let searchTimeout;  // 디바운스용 타이머 ID

    // 요소가 없으면 초기화 중단
    if (!searchInput || !searchClear) return;

    searchInput.addEventListener('input', (e) => {
        const value = e.target.value.trim();
        // 입력값 유무에 따라 X(초기화) 버튼 표시/숨김
        searchClear.classList.toggle('visible', !!value);
        clearTimeout(searchTimeout);
        // 300ms 후 검색 실행 (연속 입력 시 마지막 입력만 처리)
        searchTimeout = setTimeout(() => performSearch(value, resourceManager), 300);
    });

    searchClear.addEventListener('click', () => {
        searchInput.value = '';
        searchClear.classList.remove('visible');
        performSearch('', resourceManager);  // 빈 쿼리로 전체 목록 재로드
        searchInput.focus();                  // UX: 클리어 후 포커스 유지
    });

    searchInput.addEventListener('keydown', (e) => {
        if (e.key === 'Enter') {
            clearTimeout(searchTimeout);  // 디바운스 취소
            performSearch(searchInput.value.trim(), resourceManager);  // 즉시 검색
        }
    });
}

/**
 * 검색 실행
 * ResourceManager 상태에 검색어를 설정하고 1페이지로 리셋 후 데이터를 재로드함
 */
export function performSearch(query, resourceManager) {
    if (resourceManager && resourceManager.state) {
        resourceManager.state.searchQuery = query;  // 검색어 상태 업데이트
        resourceManager.state.currentPage = 1;       // 검색 시 1페이지로 리셋
    }
    if (resourceManager && resourceManager.loadData) {
        resourceManager.loadData();  // 변경된 검색 조건으로 데이터 재요청
    }
}


/**
 * 빠른 필터 버튼 초기화 (Used / Unused / All)
 * .quick-action-btn 버튼 클릭 시 active 클래스 전환 후 해당 필터를 statusFilterSelect에 반영
 */
export function initQuickActions() {
    const actionBtns = document.querySelectorAll('.quick-action-btn');
    actionBtns.forEach(btn => {
        btn.addEventListener('click', () => {
            // 기존 active 버튼 초기화 후 현재 버튼 활성화
            actionBtns.forEach(b => b.classList.remove('active'));
            btn.classList.add('active');
            applyQuickFilter(btn.dataset.filter);  // data-filter 속성값으로 필터 적용
        });
    });
}

/**
 * 빠른 필터 적용
 * filter 값('used'|'unused'|'all')을 statusFilterSelect의 value에 매핑하여 change 이벤트 발생
 * → ResourceManager가 change 이벤트를 감지하여 자동으로 데이터 재로드
 */
export function applyQuickFilter(filter) {
    const statusSelect = document.getElementById('statusFilterSelect');
    if (!statusSelect) return;
    const map = { used: 'Y', unused: 'N', all: '' };  // 필터명 → API 파라미터값 매핑
    statusSelect.value = map[filter] ?? '';
    statusSelect.dispatchEvent(new Event('change'));  // ResourceManager 필터 리스너 트리거
}


/**
 * 모달 다단계 스텝 초기화 (2단계)
 *
 * Step 1: 다운타임명 + shortcut 입력 + 이중 중복 확인
 *   1차: downtime_name 중복 확인 (필수)
 *   2차: downtime_shortcut 중복 확인 (shortcut 입력된 경우에만)
 *     → shortcut은 선택 입력이므로 비어있으면 중복 체크 생략
 *
 * Step 2: 상태/비고 + 미리보기 (previewName, previewShortcut, previewStatus)
 *
 * 스텝 인디케이터(.form-step) 직접 클릭으로 이전/이후 스텝 이동 가능
 * (이후 스텝으로의 직접 이동은 현재 스텝부터 대상 스텝까지 순차 검증 필요)
 */
export function initModalSteps() {
    const nextBtn = document.getElementById('nextStep');
    const prevBtn = document.getElementById('prevStep');
    const submitBtn = document.getElementById('submitBtn');
    const stepIndicator = document.getElementById('stepIndicator');

    // 필수 요소가 없으면 초기화 중단
    if (!nextBtn || !prevBtn || !submitBtn || !stepIndicator) return;

    let currentStep = 1;    // 현재 활성 스텝 (1-based)
    const totalSteps = 2;   // 전체 스텝 수

    // 다음 스텝 버튼: 현재 스텝 검증 통과 시 다음 스텝으로 이동
    nextBtn.addEventListener('click', async () => {
        if (await validateCurrentStep(currentStep)) {
            currentStep++;
            showStep(currentStep);
        }
    });

    // 이전 스텝 버튼: 검증 없이 이전 스텝으로 즉시 이동
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
                // 이전 스텝으로의 이동: 검증 없이 즉시 이동
                currentStep = target;
                showStep(currentStep);
            } else if (target > currentStep) {
                // 이후 스텝으로의 이동: 현재 스텝부터 대상 전 스텝까지 순차 검증
                let valid = true, tmp = currentStep;
                while (tmp < target && valid) {
                    valid = await validateCurrentStep(tmp);
                    if (valid) tmp++;
                }
                if (valid) { currentStep = target; showStep(currentStep); }
            }
        });
    });

    /**
     * 해당 스텝 섹션 표시 및 버튼 가시성 업데이트
     * - data-section 속성이 현재 스텝과 일치하는 .form-section만 표시
     * - 첫 스텝: prevBtn 숨김, 마지막 스텝: nextBtn 숨김 + submitBtn 표시
     */
    function showStep(step) {
        document.querySelectorAll('.form-section').forEach(s => {
            s.style.display = s.dataset.section == step ? 'block' : 'none';
        });
        prevBtn.style.display = step > 1 ? 'inline-block' : 'none';
        nextBtn.style.display = step < totalSteps ? 'inline-block' : 'none';
        submitBtn.style.display = step === totalSteps ? 'inline-block' : 'none';
        stepIndicator.textContent = `Step ${step} of ${totalSteps}`;
        // 현재 스텝 이하의 인디케이터를 모두 active로 표시 (완료된 스텝 시각화)
        document.querySelectorAll('.form-step').forEach(s => {
            s.classList.toggle('active', parseInt(s.dataset.step) <= step);
        });
        updatePreview();  // 스텝 전환 시 미리보기 갱신
    }

    /**
     * Step 1 이중 중복 확인:
     *   1차: 다운타임명(downtime_name) 중복 확인 (필수)
     *     → proc/downtime.php?for=check-duplicate
     *   2차: shortcut(downtime_shortcut) 중복 확인 (입력된 경우에만)
     *     → proc/downtime.php?for=check-duplicate-shortcut
     *     → shortcut은 선택 입력이므로 비어있으면 2차 체크 생략
     *
     * 수정 모드: resourceId가 있으면 current_idx 파라미터 추가
     *   → 자기 자신은 중복 판정에서 제외
     */
    async function validateCurrentStep(step) {
        if (step === 1) {
            const downtimeName = document.getElementById('downtime_name').value.trim();
            if (!downtimeName) { alert('Please enter the downtime name.'); return false; }

            // 1차 중복 확인: 다운타임명 (전체 고유)
            try {
                const currentIdx = document.getElementById('resourceId').value || null;
                const url = `proc/downtime.php?for=check-duplicate&downtime_name=${encodeURIComponent(downtimeName)}${currentIdx ? `&current_idx=${currentIdx}` : ''}`;
                const result = await fetch(url).then(r => r.json());
                if (!result.success) { alert(result.message); return false; }
            } catch {
                alert('An error occurred while checking for duplicates.');
                return false;
            }

            // 2차 중복 확인: shortcut (입력된 경우에만)
            const downtimeShortcut = document.getElementById('downtime_shortcut').value.trim();
            if (downtimeShortcut) {
                try {
                    const currentIdx = document.getElementById('resourceId').value || null;
                    const url = `proc/downtime.php?for=check-duplicate-shortcut&downtime_shortcut=${encodeURIComponent(downtimeShortcut)}${currentIdx ? `&current_idx=${currentIdx}` : ''}`;
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

    /**
     * Step 2 미리보기 업데이트
     * Step 1 입력값을 Step 2의 미리보기 요소에 실시간 반영
     *   - previewName    : downtime_name 입력값
     *   - previewShortcut: downtime_shortcut 입력값
     *   - previewStatus  : status 선택 텍스트
     */
    function updatePreview() {
        const name = document.getElementById('downtime_name')?.value || '-';
        const shortcut = document.getElementById('downtime_shortcut')?.value || '-';
        const status = document.getElementById('status')?.selectedOptions[0]?.textContent || '-';
        const pName = document.getElementById('previewName');
        const pShort = document.getElementById('previewShortcut');
        const pStat = document.getElementById('previewStatus');
        if (pName) pName.textContent = name;
        if (pShort) pShort.textContent = shortcut;
        if (pStat) pStat.textContent = status;
    }

    // 입력 변경 시 미리보기 실시간 반영
    document.getElementById('downtime_name')?.addEventListener('input', updatePreview);
    document.getElementById('downtime_shortcut')?.addEventListener('input', updatePreview);
    document.getElementById('status')?.addEventListener('change', updatePreview);

    // 신규 등록 버튼 클릭 시 Step 1로 초기화
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
