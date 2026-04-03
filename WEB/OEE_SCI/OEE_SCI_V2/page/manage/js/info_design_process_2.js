
/**
 * info_design_process_2.js — 공정 설계(Design Process) 관리 페이지 전용 모듈 (ES Module)
 *
 * 내보내기(export):
 *   - designProcessConfig : ResourceManager 설정 객체
 *   - initAdvancedFeatures, initRealTimeSearch, initQuickActions,
 *     initFilePreviewIcons, showImagePreviewModal,
 *     initModalSteps, initDeleteButtons
 *
 * 특이사항:
 *   - apiEndpoint: '../manage/proc/design_process.php' (다른 파일과 달리 상위 경로 사용)
 *     → 이 JS 파일이 다른 위치에서도 로드될 수 있음을 시사
 *   - actions 컬럼: ResourceManager 통합 삭제 대신 개별 삭제 버튼(.delete-btn) 직접 처리
 *   - initQuickActions(): DOM 행 직접 숨김/표시 (API 재요청 없음, 다른 파일과 다름)
 *   - MutationObserver: resourceModal classList 변화 감지 → 모달 열림 시 Step 1 자동 리셋
 *   - beforeEdit hook: 수정 시 factory_idx/line_idx 설정 + 라인 드롭다운 갱신
 *
 * 모달 스텝 (2단계):
 *   Step 1: 공장 선택 + 라인 선택 + 공정명 입력 + 중복 확인
 *     (공장+라인 조합 내 중복 체크)
 *   Step 2: Standard MC + 상태 + SOP 파일 + 미리보기
 */

/**
 * 고급 기능 초기화 진입점
 * ResourceManager 초기화 완료 후 호출되어 모든 부가 기능을 활성화함
 */
export function initAdvancedFeatures(resourceManager) {
    initRealTimeSearch(resourceManager);    // 실시간 검색 초기화
    initQuickActions();                     // 빠른 필터 버튼 초기화 (DOM 직접 필터링)
    initModalSteps();                       // 2단계 모달 스텝 초기화
    initFilePreviewIcons();                 // SOP 파일 미리보기 아이콘 클릭 이벤트
    initDeleteButtons(resourceManager);     // 삭제 버튼 이벤트 등록
}

/**
 * 공정 설계 리소스 설정 객체
 *
 * 컬럼 설명:
 *   no            : 순번 (index+1 기반, 정렬 불가)
 *   idx           : DB PK (숨김, dp.idx 정렬 키)
 *   factory_name  : 소속 공장명 (없으면 N/A)
 *   line_name     : 소속 라인명 (없으면 N/A)
 *   model_name    : 모델명 (없으면 N/A)
 *   design_process: 공정명 (.process-name-badge 스타일)
 *   std_mc_needed : 표준 필요 기계 수 (.process-mc-badge 스타일, '{N} MC' 형식)
 *   fname         : SOP 파일명 (있으면 미리보기 아이콘 + 파일명, 없으면 'No file')
 *                   .file-preview-icon 클릭 → showImagePreviewModal() 호출
 *   status        : 사용 여부 배지
 *   remark        : 비고
 *   actions       : 삭제 버튼 (.delete-btn, data-idx, data-name 속성)
 *
 * filterConfig:
 *   - factoryFilterSelect: 공장 필터 (resets: ['lineFilter'] → 공장 변경 시 라인 필터 초기화)
 *   - factoryLineFilterSelect: 라인 필터 (공장 선택 후 동적 갱신)
 *   - statusFilterSelect: 사용 여부 필터
 *
 * onTableRender:
 *   테이블 렌더링 완료 시 모든 행에 'process-table-row' 클래스 추가 (CSS 스타일링용)
 *
 * beforeInit:
 *   ResourceManager 초기화 전 loadFactoryOptions() 호출 (공장 목록 로드)
 *
 * beforeEdit:
 *   수정 모달 열기 전 factory_idx/line_idx 값 설정 + 라인 드롭다운 동적 갱신
 *   - line_idx disabled 해제 후 updateLineOptions()로 해당 공장의 라인 목록 로드
 *   - model_name 입력 필드 값 설정
 *   - file_upload 초기화 + window.updateExistingFileInfo() 호출 (기존 파일 정보 표시)
 */
export const designProcessConfig = {
    resourceName: 'Design Process',
    // 주의: 상대 경로가 '../manage/proc/...'로 다른 파일들과 다름
    apiEndpoint: '../manage/proc/design_process.php',
    entityId: 'idx',

    columnConfig: [
        {
            key: 'no',
            label: 'NO.',
            sortable: false,
            // 다른 파일과 달리 index+1로 1-based 표시
            render: (item, index) => `<span style="color: var(--sap-text-secondary);">${index + 1}</span>`
        },
        { key: 'idx', label: 'IDX', sortable: true, sortKey: 'dp.idx', visible: false },
        {
            key: 'factory_name',
            label: 'Factory',
            sortable: true,
            sortKey: 'f.factory_name',
            render: (item) => item.factory_name
                ? `<span style="color: var(--sap-text-primary);">${item.factory_name}</span>`
                : '<span style="color: var(--sap-text-secondary);">N/A</span>'
        },
        {
            key: 'line_name',
            label: 'Line',
            sortable: true,
            sortKey: 'l.line_name',
            render: (item) => item.line_name
                ? `<span style="color: var(--sap-text-primary);">${item.line_name}</span>`
                : '<span style="color: var(--sap-text-secondary);">N/A</span>'
        },
        {
            key: 'model_name',
            label: 'Model Name',
            sortable: true,
            sortKey: 'dp.model_name',
            render: (item) => item.model_name
                ? `<span style="color: var(--sap-text-primary);">${item.model_name}</span>`
                : '<span style="color: var(--sap-text-secondary);">N/A</span>'
        },
        {
            key: 'design_process',
            label: 'Design Process',
            sortable: true,
            sortKey: 'dp.design_process',
            // 공정명은 배지 스타일로 강조 표시
            render: (item) => `<span class="process-name-badge">${item.design_process}</span>`
        },
        {
            key: 'std_mc_needed',
            label: 'Standard MC',
            sortable: true,
            sortKey: 'dp.std_mc_needed',
            // 필요 기계 수는 '{N} MC' 형식의 배지로 표시
            render: (item) => {
                const count = parseInt(item.std_mc_needed) || 0;
                return `<span class="process-mc-badge">${count} MC</span>`;
            }
        },
        {
            key: 'fname',
            label: 'SOP',
            sortable: true,
            sortKey: 'dp.fname',
            render: (item) => {
                if (item.fname) {
                    // SOP 파일이 있으면 미리보기 아이콘(📄) + 파일명 표시
                    // .file-preview-icon: initFilePreviewIcons()에서 이벤트 위임으로 처리
                    return `
                        <span class="process-file-badge">
                            <span class="file-preview-icon" data-filename="${item.fname}" title="Click to preview" style="cursor: pointer; color: var(--sap-brand-primary);">&#128196;</span>
                            ${item.fname}
                        </span>`;
                }
                return '<span style="color: var(--sap-text-secondary);">No file</span>';
            }
        },
        {
            key: 'status',
            label: 'Status',
            sortable: true,
            sortKey: 'dp.status',
            render: (item) => item.status === 'Y'
                ? '<span class="fiori-badge fiori-badge--success">Used</span>'
                : '<span class="fiori-badge fiori-badge--warning">Unused</span>'
        },
        {
            key: 'remark',
            label: 'Remark',
            sortable: true,
            sortKey: 'dp.remark',
            render: (item) => item.remark || '<span style="color: var(--sap-text-secondary);">-</span>'
        },
        {
            key: 'actions',
            label: 'Delete',
            sortable: false,
            // ResourceManager 통합 삭제 대신 개별 .delete-btn으로 직접 삭제 처리
            // data-idx, data-name 속성으로 삭제 대상 식별
            render: (item) => `
                <button
                    class="fiori-btn fiori-btn--tertiary fiori-btn--sm delete-btn"
                    data-idx="${item.idx}"
                    data-name="${item.design_process}"
                    title="Delete"
                    style="min-width: auto; padding: 4px 8px;"
                >&#128465;</button>
            `
        }
    ],

    filterConfig: [
        {
            elementId: 'factoryFilterSelect',
            paramName: 'factory_filter',
            stateKey: 'factoryFilter',
            resets: ['lineFilter']  // 공장 변경 시 라인 필터 자동 초기화
        },
        {
            elementId: 'factoryLineFilterSelect',
            paramName: 'line_filter',
            stateKey: 'lineFilter'
        },
        {
            elementId: 'statusFilterSelect',
            paramName: 'status_filter',
            stateKey: 'statusFilter'
        }
    ],

    // 테이블 렌더링 완료 시 모든 행에 CSS 클래스 추가
    onTableRender: (data) => {
        const tableRows = document.querySelectorAll('#tableBody tr');
        tableRows.forEach(row => row.classList.add('process-table-row'));
    },

    // ResourceManager 초기화 전 공장 목록 로드
    async beforeInit(api) {
        await loadFactoryOptions(api);
    },

    /**
     * 수정 모달 열기 전 처리 (beforeEdit hook)
     * 1. factory_idx 드롭다운에 현재 데이터의 공장 선택
     * 2. line_idx 드롭다운 활성화 + 해당 공장의 라인 목록 로드 + 현재 라인 선택
     * 3. model_name 입력 필드에 현재 값 설정
     * 4. file_upload 초기화 + window.updateExistingFileInfo() 호출 (기존 파일 정보 표시)
     *    - setTimeout(100): 모달 DOM 렌더링 완료 대기
     */
    async beforeEdit(api, data) {
        const factorySelect = document.getElementById('factory_idx');
        if (factorySelect && data.factory_idx) {
            factorySelect.value = data.factory_idx;
            const lineSelect = document.getElementById('line_idx');
            if (lineSelect) {
                lineSelect.disabled = false;
                // 해당 공장의 라인 목록 로드 + 현재 라인(data.line_idx) 선택 유지
                await updateLineOptions(api, data.factory_idx, 'line_idx', 'Please select a line', data.line_idx);
            }
        }

        const modelNameInput = document.getElementById('model_name');
        if (modelNameInput) modelNameInput.value = data.model_name || '';

        // 100ms 지연: 모달 DOM이 완전히 렌더링된 후 파일 정보 업데이트
        setTimeout(() => {
            const fileInput = document.getElementById('file_upload');
            if (fileInput) fileInput.value = '';
            // window.updateExistingFileInfo: PHP에서 인라인으로 정의된 전역 함수
            // 기존 파일명을 표시 영역에 설정 (새 파일 미선택 시 기존 파일 유지)
            if (window.updateExistingFileInfo) window.updateExistingFileInfo(data.fname);
        }, 100);
    }
};

/**
 * 실시간 검색 초기화 (300ms 디바운스)
 * 다른 파일과 달리 performSearch 함수를 분리하지 않고 인라인으로 처리
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
        searchTimeout = setTimeout(() => {
            // resourceManager 상태 업데이트 후 데이터 재로드
            if (resourceManager && resourceManager.state) {
                resourceManager.state.searchQuery = value;
                resourceManager.state.currentPage = 1;
            }
            if (resourceManager && resourceManager.loadData) resourceManager.loadData();
        }, 300);
    });

    searchClear.addEventListener('click', () => {
        searchInput.value = '';
        searchClear.classList.remove('visible');
        if (resourceManager && resourceManager.state) {
            resourceManager.state.searchQuery = '';
            resourceManager.state.currentPage = 1;
        }
        if (resourceManager && resourceManager.loadData) resourceManager.loadData();
        searchInput.focus();
    });
}

/**
 * 빠른 필터 버튼 초기화
 *
 * 다른 파일과의 차이점:
 *   다른 파일: statusFilterSelect.value 변경 → change 이벤트 → API 재요청
 *   이 파일: #tableBody tr를 직접 순회하여 행을 숨김/표시 (API 재요청 없음)
 *
 * 'used' 필터: 'Used'를 포함하고 'Unused'를 포함하지 않는 행만 표시
 * 'unused' 필터: 'Unused'를 포함하는 행만 표시
 * 기타(all): 모든 행 표시
 *
 * 주의: DOM 텍스트 기반 필터링이므로 i18n 변경 시 수정 필요
 */
export function initQuickActions() {
    const quickFilters = document.querySelectorAll('.quick-action-btn');
    quickFilters.forEach(filter => {
        filter.addEventListener('click', () => {
            quickFilters.forEach(f => f.classList.remove('active'));
            filter.classList.add('active');
            const filterType = filter.dataset.filter;
            const tableRows = document.querySelectorAll('#tableBody tr');
            tableRows.forEach(row => {
                let show = true;
                // 'used': 'Used' 포함하되 'Unused' 미포함 행 (Unused는 Used도 포함하므로 제외 필요)
                if (filterType === 'used')   show = row.textContent.includes('Used') && !row.textContent.includes('Unused');
                if (filterType === 'unused') show = row.textContent.includes('Unused');
                row.style.display = show ? '' : 'none';
            });
        });
    });
}

/**
 * SOP 파일 미리보기 아이콘 클릭 이벤트 초기화
 * 이벤트 위임: #tableBody에 단일 click 리스너 등록 → .file-preview-icon 클릭 시 처리
 * (동적으로 생성되는 테이블 행의 요소이므로 위임 방식 사용)
 *
 * e.stopPropagation/preventDefault: 행 클릭(수정 모달 열기) 동작과 충돌 방지
 */
export function initFilePreviewIcons() {
    const tableBody = document.getElementById('tableBody');
    if (!tableBody) return;

    tableBody.addEventListener('click', (e) => {
        const previewIcon = e.target.closest('.file-preview-icon');
        if (!previewIcon) return;
        e.stopPropagation();   // 이벤트 버블링 차단 (행 클릭 이벤트 방지)
        e.preventDefault();
        const filename = previewIcon.dataset.filename;
        if (filename) showImagePreviewModal(filename);
    });
}

/**
 * SOP 이미지 미리보기 모달 표시
 * 이미지 경로: '../../upload/sop/{filename}' (현재 페이지 기준 상대 경로)
 *
 * onerror 처리: 이미지 로드 실패 시 알림 후 모달 즉시 닫기
 *   (파일이 존재하지 않거나 경로 오류 시)
 *
 * @param {string} filename  표시할 SOP 파일명 (경로 제외, 파일명만)
 */
export function showImagePreviewModal(filename) {
    const imagePreviewModal = document.getElementById('imagePreviewModal');
    const previewImage = document.getElementById('previewImage');
    const imageFileName = document.getElementById('imageFileName');
    if (!imagePreviewModal || !previewImage || !imageFileName) return;

    previewImage.src = `../../upload/sop/${filename}`;  // 업로드 디렉토리 기준 경로
    imageFileName.textContent = filename;
    imagePreviewModal.style.display = 'flex';
    setTimeout(() => imagePreviewModal.classList.add('show'), 10);

    // 이미지 로드 실패 시 모달 닫기
    previewImage.onerror = function () {
        alert('Image could not be loaded.');
        imagePreviewModal.style.display = 'none';
    };
}

/**
 * 모달 다단계 스텝 초기화 (2단계)
 *
 * Step 1: 공장 선택 + 라인 선택 + 공정명 입력 + 중복 확인
 *   - factory_idx: 공장 선택 (필수)
 *   - line_idx: 라인 선택 (필수, 공장 선택 후 활성화)
 *   - design_process: 공정명 입력 (필수)
 *   - 중복 확인: 공장+라인 조합 내 동일 공정명 체크
 *     → '../manage/proc/design_process.php?for=check-duplicate'
 *
 * Step 2: Standard MC + 상태 + SOP 파일 업로드 + 미리보기
 *   - std_mc_needed, status, file_upload
 *   - previewName, previewStdMc, previewStatus, previewFile
 *
 * addBtn 클릭 처리:
 *   - Step 1로 리셋
 *   - factory_idx 초기화, line_idx 초기화 + disabled
 *   - window.resetExistingFileInfo() 호출 (기존 파일 표시 초기화)
 *
 * MutationObserver:
 *   - resourceModal의 classList 변화 감지
 *   - 'show' 클래스 추가 시(= 모달 열림 시) currentStep=1로 자동 리셋
 *   - 목적: 수정 후 닫기 → 재열기 시 항상 Step 1부터 시작
 */
export function initModalSteps() {
    const nextBtn      = document.getElementById('nextStep');
    const prevBtn      = document.getElementById('prevStep');
    const submitBtn    = document.getElementById('submitBtn');
    const stepIndicator = document.getElementById('stepIndicator');
    if (!nextBtn || !prevBtn || !submitBtn || !stepIndicator) return;

    let currentStep = 1;
    const totalSteps = 2;

    // 다음 스텝 버튼: 현재 스텝 검증 통과 시 다음 스텝으로 이동
    nextBtn.addEventListener('click', async () => {
        if (await validateStep(currentStep)) {
            currentStep++;
            showStep(currentStep);
        }
    });

    // 이전 스텝 버튼: 검증 없이 이전 스텝으로 즉시 이동
    prevBtn.addEventListener('click', () => {
        currentStep--;
        showStep(currentStep);
    });

    // 스텝 인디케이터 클릭 직접 이동
    document.querySelectorAll('.form-step').forEach((step, index) => {
        step.style.cursor = 'pointer';
        step.addEventListener('click', async () => {
            const target = index + 1;
            if (target < currentStep) {
                // 이전 스텝: 즉시 이동
                currentStep = target;
                showStep(currentStep);
            } else if (target > currentStep && await validateStep(currentStep)) {
                // 이후 스텝: 현재 스텝 검증 후 이동 (다른 파일과 달리 단일 검증만 수행)
                currentStep = target;
                showStep(currentStep);
            }
        });
    });

    /** 해당 스텝 섹션 표시 및 버튼 가시성 제어 */
    function showStep(step) {
        document.querySelectorAll('.form-section').forEach(s => {
            s.style.display = s.dataset.section == step ? 'block' : 'none';
        });
        prevBtn.style.display   = step > 1 ? 'inline-block' : 'none';
        nextBtn.style.display   = step < totalSteps ? 'inline-block' : 'none';
        submitBtn.style.display = step === totalSteps ? 'inline-block' : 'none';
        stepIndicator.textContent = `Step ${step} of ${totalSteps}`;
        document.querySelectorAll('.form-step').forEach(s => {
            s.classList.toggle('active', parseInt(s.dataset.step) <= step);
        });
        updatePreview();
    }

    /**
     * 스텝 유효성 검사
     * Step 1만 검증 (step !== 1이면 항상 true 반환)
     *
     * Step 1 검증 순서:
     *   1. factory_idx 선택 여부 확인 (필수)
     *   2. line_idx 선택 여부 확인 (필수)
     *   3. design_process 입력 여부 확인 (필수)
     *   4. 중복 확인 API 호출 (공장+라인+공정명 조합)
     *      수정 모드: current_idx 파라미터로 자기 자신 제외
     */
    async function validateStep(step) {
        if (step !== 1) return true;

        const factorySelect = document.getElementById('factory_idx');
        const lineSelect    = document.getElementById('line_idx');
        const designProcess = document.getElementById('design_process').value.trim();

        if (!factorySelect || !factorySelect.value) {
            alert('Please select a factory.');
            if (factorySelect) factorySelect.focus();
            return false;
        }
        if (!lineSelect || !lineSelect.value) {
            alert('Please select a line.');
            if (lineSelect) lineSelect.focus();
            return false;
        }
        if (!designProcess) {
            alert('Please enter the design process name.');
            return false;
        }

        // 중복 확인: 같은 공장+라인 내 동일 공정명 체크
        try {
            const currentIdx = document.getElementById('resourceId').value || null;
            const url = `../manage/proc/design_process.php?for=check-duplicate&design_process=${encodeURIComponent(designProcess)}&factory_idx=${factorySelect.value}&line_idx=${lineSelect.value}${currentIdx ? `&current_idx=${currentIdx}` : ''}`;
            const res = await fetch(url);
            const result = await res.json();
            if (!result.success) {
                alert(result.message);
                return false;
            }
        } catch (e) {
            alert('An error occurred while checking for duplicates.');
            return false;
        }
        return true;
    }

    /**
     * Step 2 미리보기 업데이트
     * Step 1/2 입력값을 미리보기 요소에 실시간 반영
     *   - previewName   : design_process 입력값
     *   - previewStdMc  : std_mc_needed 입력값
     *   - previewStatus : status 선택 텍스트
     *   - previewFile   : 선택된 파일명 (미선택 시 '-')
     */
    function updatePreview() {
        const designProcess = document.getElementById('design_process').value || '-';
        const stdMcNeeded   = document.getElementById('std_mc_needed').value || '-';
        const statusText    = document.getElementById('status').selectedOptions[0]?.textContent || '-';
        const fileInput     = document.getElementById('file_upload');
        // 파일 선택 여부에 따라 파일명 또는 '-' 표시
        const fileName      = fileInput && fileInput.files.length > 0 ? fileInput.files[0].name : '-';

        const previewName   = document.getElementById('previewName');
        const previewStdMc  = document.getElementById('previewStdMc');
        const previewStatus = document.getElementById('previewStatus');
        const previewFile   = document.getElementById('previewFile');

        if (previewName)   previewName.textContent   = designProcess;
        if (previewStdMc)  previewStdMc.textContent  = stdMcNeeded;
        if (previewStatus) previewStatus.textContent = statusText;
        if (previewFile)   previewFile.textContent   = fileName;
    }

    // 입력 변경 시 미리보기 실시간 반영
    ['design_process', 'std_mc_needed'].forEach(id => {
        const el = document.getElementById(id);
        if (el) el.addEventListener('input', updatePreview);
    });
    const statusSel  = document.getElementById('status');
    const fileInput  = document.getElementById('file_upload');
    if (statusSel) statusSel.addEventListener('change', updatePreview);
    if (fileInput)  fileInput.addEventListener('change', updatePreview);

    // 신규 등록 버튼 처리
    const addBtn = document.getElementById('addBtn');
    if (addBtn) {
        addBtn.addEventListener('click', () => {
            currentStep = 1;
            showStep(1);
            // 공장/라인 드롭다운 초기화
            const factorySelect = document.getElementById('factory_idx');
            const lineSelect    = document.getElementById('line_idx');
            if (factorySelect) factorySelect.value = '';
            if (lineSelect) {
                lineSelect.value = '';
                lineSelect.disabled = true;  // 공장 선택 전까지 라인 선택 불가
            }
            // window.resetExistingFileInfo: PHP 인라인 전역 함수 (기존 파일 표시 초기화)
            if (window.resetExistingFileInfo) window.resetExistingFileInfo();
        });
    }

    /**
     * MutationObserver: resourceModal 클래스 변화 감지
     * 모달에 'show' 클래스가 추가될 때(= 모달 열릴 때) Step 1로 자동 리셋
     * 목적: 수정 모달 닫기 후 다시 열 때 항상 Step 1부터 시작하도록 보장
     * 참고: addBtn 클릭은 별도로 처리하므로 신규/수정 모두 Step 1에서 시작
     */
    const resourceModal = document.getElementById('resourceModal');
    if (resourceModal) {
        new MutationObserver((mutations) => {
            mutations.forEach((m) => {
                if (m.type === 'attributes' && m.attributeName === 'class' && resourceModal.classList.contains('show')) {
                    currentStep = 1;
                    showStep(1);
                }
            });
        }).observe(resourceModal, { attributes: true });  // classList 변화만 감지
    }
}

/**
 * 삭제 버튼 이벤트 초기화
 * ResourceManager의 통합 삭제 기능 대신 개별 삭제 버튼(.delete-btn) 직접 처리
 * 이벤트 위임: #tableBody에 단일 click 리스너 등록 (동적 생성 행 처리)
 *
 * 처리 순서:
 *   1. .delete-btn 클릭 감지 (e.stopPropagation으로 행 클릭 이벤트 방지)
 *   2. data-idx, data-name으로 삭제 대상 확인
 *   3. confirm 다이얼로그 (비가역 작업 경고)
 *   4. fetch DELETE → '../manage/proc/design_process.php?id={idx}'
 *   5. 성공 시 resourceManager.loadData()로 목록 갱신
 *
 * @param {object} resourceManager  ResourceManager 인스턴스 (loadData 메서드 사용)
 */
export function initDeleteButtons(resourceManager) {
    const tableBody = document.getElementById('tableBody');
    if (!tableBody) return;

    tableBody.addEventListener('click', async (e) => {
        const deleteBtn = e.target.closest('.delete-btn');
        if (!deleteBtn) return;
        e.stopPropagation();  // 행 클릭(수정 모달 열기) 이벤트 방지
        e.preventDefault();

        const idx         = deleteBtn.dataset.idx;
        const processName = deleteBtn.dataset.name;

        // 비가역 작업 확인 다이얼로그
        if (!confirm(`Are you sure you want to delete this Design Process?\n\nProcess: ${processName}\n\nThis action is irreversible.`)) return;

        try {
            const response = await fetch(`../manage/proc/design_process.php?id=${idx}`, { method: 'DELETE' });
            const result   = await response.json();
            if (result.success) {
                alert('Design Process deleted successfully.');
                // 삭제 성공 후 목록 갱신
                if (resourceManager && resourceManager.loadData) await resourceManager.loadData();
            } else {
                alert('Deletion failed: ' + result.message);
            }
        } catch (e) {
            alert('An error occurred while deleting.');
        }
    });
}

/**
 * 공장 옵션 로드 (beforeInit에서 호출)
 * api.getAll({ for: 'factories' })로 공장 목록을 가져와
 * 목록 필터 드롭다운(factoryFilterSelect)과 모달 드롭다운(factory_idx)에 채움
 *
 * factoryFilterSelect 변경 이벤트:
 *   → updateLineOptions(api, factoryId, 'factoryLineFilterSelect', 'All Lines')
 *   → 목록 페이지의 라인 필터 드롭다운 동적 갱신
 *
 * factory_idx 변경 이벤트:
 *   → line_idx 활성화/비활성화
 *   → updateLineOptions(api, factoryId, 'line_idx', 'Please select a line')
 *   → 모달의 라인 선택 드롭다운 동적 갱신
 *
 * 초기화 완료 후: factoryLineFilterSelect를 'All Lines' 상태로 초기 로드
 *
 * @param {object} api  ResourceManager의 api 객체 (getAll 메서드 사용)
 */
async function loadFactoryOptions(api) {
    try {
        const response = await api.getAll({ for: 'factories' });
        if (!response.success || !response.data) return;

        const factoryFilterSelect = document.getElementById('factoryFilterSelect');
        const modalFactorySelect  = document.getElementById('factory_idx');

        // 목록 페이지 공장 필터 드롭다운 초기화
        if (factoryFilterSelect) {
            factoryFilterSelect.innerHTML = '<option value="">All Factories</option>';
            response.data.forEach(f => {
                factoryFilterSelect.innerHTML += `<option value="${f.idx}">${f.factory_name}</option>`;
            });
            // 공장 선택 시 목록 페이지의 라인 필터 연쇄 갱신
            factoryFilterSelect.addEventListener('change', (e) => {
                updateLineOptions(api, e.target.value, 'factoryLineFilterSelect', 'All Lines');
            });
        }

        // 모달 내 공장 선택 드롭다운 초기화
        if (modalFactorySelect) {
            modalFactorySelect.innerHTML = '<option value="">Please select a factory</option>';
            response.data.forEach(f => {
                modalFactorySelect.innerHTML += `<option value="${f.idx}">${f.factory_name}</option>`;
            });
            // 모달 공장 선택 시 라인 드롭다운 연쇄 갱신
            modalFactorySelect.addEventListener('change', (e) => {
                const lineSelect = document.getElementById('line_idx');
                if (lineSelect) {
                    // 공장 미선택 시 라인 드롭다운 비활성화
                    lineSelect.disabled = !e.target.value;
                    if (!e.target.value) lineSelect.value = '';
                }
                updateLineOptions(api, e.target.value, 'line_idx', 'Please select a line');
            });
        }

        // 페이지 로드 시 factoryLineFilterSelect를 전체 라인 목록으로 초기화
        await updateLineOptions(api, '', 'factoryLineFilterSelect', 'All Lines');
    } catch (e) {
        console.error('Failed to load factory options:', e);
    }
}

/**
 * 라인 드롭다운 동적 갱신
 * 공장 선택 변경 시 해당 공장의 라인 목록을 API에서 가져와 드롭다운을 갱신
 *
 * 경로: '../manage/proc/design_process.php?for=lines[&factory_id={id}]'
 *   - factory_id 없으면 전체 라인 조회 (목록 페이지 필터용)
 *   - factory_id 있으면 해당 공장 라인만 조회 (모달용)
 *
 * preserveValue:
 *   수정 모드에서 기존 선택값을 유지하기 위해 사용
 *   갱신 후 해당 값의 option이 존재하면 자동으로 선택
 *
 * disabled 처리:
 *   - 로딩 중 disabled=true (사용자 조작 방지)
 *   - 완료 후 disabled=false (성공/실패 관계없이)
 *   - 오류 시 초기 텍스트 option만 남기고 활성화
 *
 * @param {object} api             ResourceManager의 api 객체 (미사용, fetch 직접 호출)
 * @param {string} factoryId       선택된 공장 idx (빈 문자열이면 전체 조회)
 * @param {string} lineElementId   갱신할 select 요소의 ID
 * @param {string} initialText     기본 option 텍스트 ('All Lines' 또는 'Please select a line')
 * @param {string|null} preserveValue  수정 시 유지할 기존 선택값 (null이면 유지 안 함)
 */
async function updateLineOptions(api, factoryId, lineElementId, initialText, preserveValue = null) {
    const lineSelect = document.getElementById(lineElementId);
    if (!lineSelect) return;

    lineSelect.disabled = true;  // 로딩 중 비활성화
    try {
        let url = '../manage/proc/design_process.php?for=lines';
        if (factoryId) url += `&factory_id=${factoryId}`;

        const response = await fetch(url);
        const res      = await response.json();

        if (res.success) {
            // 갱신 전 현재 선택값 보존 (preserveValue가 있으면 우선 사용)
            const current = preserveValue || lineSelect.value;
            lineSelect.innerHTML = `<option value="">${initialText}</option>`;
            res.data.forEach(line => {
                lineSelect.innerHTML += `<option value="${line.idx}">${line.line_name}</option>`;
            });
            // 보존할 값이 존재하면 자동 선택 (수정 모드에서 기존 라인 유지)
            if (current && lineSelect.querySelector(`option[value="${current}"]`)) {
                lineSelect.value = current;
            }
        } else {
            lineSelect.innerHTML = `<option value="">${initialText}</option>`;
        }
    } catch (e) {
        lineSelect.innerHTML = `<option value="">${initialText}</option>`;
    }
    lineSelect.disabled = false;  // 로딩 완료 후 활성화
}
