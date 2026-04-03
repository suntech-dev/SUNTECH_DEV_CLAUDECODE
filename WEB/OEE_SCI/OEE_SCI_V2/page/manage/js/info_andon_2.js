/**
 * info_andon_2.js — 안돈(Andon) 관리 페이지 전용 모듈 (ES Module)
 *
 * 내보내기(export):
 *   - andonConfig : ResourceManager 설정 객체
 *   - initAdvancedFeatures, initRealTimeSearch, performSearch,
 *     initQuickActions, applyQuickFilter, initModalSteps
 *
 * 안돈(Andon): 생산라인 이상 신호 경보 유형 마스터 데이터
 * 컬러 피커: jQuery Spectrum 플러그인 사용
 *
 * 모달 스텝 (2단계):
 *   Step 1: 안돈명 입력 + 중복 확인 + 색상 선택 (Spectrum Color Picker)
 *   Step 2: 상태/비고 + 미리보기 (previewName, previewColor, previewStatus)
 *
 * 전역 함수:
 *   window.updateAndonFormSpectrum(data): resource-manager에서 수정 폼 로드 시
 *   Spectrum 컬러피커 값을 data.color로 갱신하기 위해 전역으로 노출
 */

/**
 * Spectrum 컬러피커 팔레트 (4×4 형태, 4개 색상 그룹)
 * - SAP Fiori 브랜드/상태 색상 위주로 구성
 */
const COLOR_PALETTE = [
    ["#0070f2", "#1e88e5", "#00d4aa", "#0093c7"],  // 파랑 계열
    ["#30914c", "#da1e28", "#e26b0a", "#8e44ad"],  // 녹색/빨강/주황/보라
    ["#32363b", "#4a5568", "#6b7884", "#8b95a1"],  // 어두운 회색 계열
    ["#2563eb", "#059669", "#dc2626", "#ffa500"]   // 기본 상태 색상
];

/**
 * 안돈 리소스 설정 객체
 *
 * 컬럼 설명:
 *   no             : 순번
 *   idx            : DB PK (숨김)
 *   andon_name     : 안돈명
 *   total_count    : 총 발생 건수 (data_andon JOIN)
 *   completed_count: 완료 건수 (초록색)
 *   warning_count  : 경고 건수 (빨간색)
 *   warning_rate   : 경고율 % (50% 초과=빨강, 20% 초과=주황, 이하=초록)
 *   color          : 안돈 표시 색상 박스 (색상 없으면 N/A 회색 박스)
 *   status         : 사용 여부 배지
 *   remark         : 비고
 */
export const andonConfig = {
    resourceName: 'Andon',
    apiEndpoint: 'proc/andon.php',
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
            visible: false
        },
        {
            key: 'andon_name',
            label: 'Andon Name',
            sortable: true,
            sortKey: 'andon_name',
            render: (item) => `
                <div style="display: flex; align-items: center; gap: var(--sap-spacing-xs);">
                    <strong style="color: var(--sap-text-primary);">${item.andon_name}</strong>
                </div>
            `
        },
        {
            key: 'total_count',
            label: 'Total',
            sortable: true,
            sortKey: 'total_count',
            render: (item) => {
                const count = item.total_count || 0;
                return `<span style="color: var(--sap-text-primary); font-weight: 500;">${count}</span>`;
            },
            width: '80px'
        },
        {
            key: 'completed_count',
            label: 'Completed',
            sortable: true,
            sortKey: 'completed_count',
            render: (item) => {
                const count = item.completed_count || 0;
                // 완료 건수: 초록색으로 표시
                return `<span style="color: var(--sap-status-success); font-weight: 500;">${count}</span>`;
            },
            width: '100px'
        },
        {
            key: 'warning_count',
            label: 'Warning',
            sortable: true,
            sortKey: 'warning_count',
            render: (item) => {
                const count = item.warning_count || 0;
                // 경고 건수: 빨간색으로 표시 (미해결 안돈 수)
                return `<span style="color: var(--sap-status-error); font-weight: 500;">${count}</span>`;
            },
            width: '90px'
        },
        {
            key: 'warning_rate',
            label: 'Warning %',
            sortable: true,
            sortKey: 'warning_rate',
            render: (item) => {
                const rate = item.warning_rate || 0;
                // 경고율에 따른 3단계 색상: >50% 빨강, >20% 주황, 이하 초록
                const color = rate > 50 ? 'var(--sap-status-error)' :
                              rate > 20 ? 'var(--sap-status-warning)' :
                              'var(--sap-status-success)';
                return `<span style="color: ${color}; font-weight: 500;">${rate}%</span>`;
            },
            width: '100px'
        },
        {
            key: 'color',
            label: 'Color',
            sortable: true,
            sortKey: 'color',
            render: (item) => {
                const color = item.color;
                if (!color) {
                    // 색상 미설정: 회색 배경 + N/A 텍스트
                    return `<div style="width: 50%; height: 20px; border: 1px solid var(--sap-border-neutral); border-radius: var(--sap-radius-sm); background: var(--sap-surface-2); display: flex; align-items: center; justify-content: center; font-size: var(--sap-font-size-xs); color: var(--sap-text-secondary);">N/A</div>`;
                }
                // 색상 설정: 해당 색상으로 채운 박스 (title에 HEX 코드 표시)
                return `<div style="width: 50%; height: 20px; border: 1px solid var(--sap-border-neutral); border-radius: var(--sap-radius-sm); background: ${color}; box-shadow: 0 1px 2px rgba(0,0,0,0.1);" title="${color}"></div>`;
            },
            width: '80px'
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
 * 고급 기능 초기화 진입점
 * - initColorPicker: Spectrum 컬러피커 초기화 (다른 기능보다 먼저 실행)
 */
export function initAdvancedFeatures(resourceManager) {
    initColorPicker();
    initRealTimeSearch(resourceManager);
    initQuickActions();
    initModalSteps();
}


// ─── Color Picker ────────────────────────────────────────

/**
 * Spectrum 컬러피커 초기화 (#color 입력 필드에 연결)
 * - type: 'component': 입력 필드 옆에 컬러 버튼 표시
 * - showPalette: true: COLOR_PALETTE 팔레트 표시
 * - hideAfterPaletteSelect: true: 팔레트 선택 후 자동으로 닫힘
 * - localStorageKey: 최근 선택 색상을 로컬스토리지에 저장 (최대 10개)
 * - colorPreviewBox: 선택한 색상을 실시간으로 미리보기 박스에 반영
 * - move 이벤트: 컬러피커 드래그 중 실시간 미리보기 갱신
 * - change 이벤트: 색상 선택 확정 시 input 이벤트 발생 (미리보기 업데이트 연계)
 */
function initColorPicker() {
    const colorInput = $('#color');
    const colorPreviewBox = $('#colorPreviewBox');

    if (!colorInput.length) return;

    colorInput.spectrum({
        type: 'component',
        showInput: true,
        showInitial: true,
        allowEmpty: true,
        showAlpha: false,
        showPalette: true,
        showPaletteOnly: false,
        showSelectionPalette: true,
        hideAfterPaletteSelect: true,
        clickoutFiresChange: true,
        color: '#0070f2',           // 기본 색상: SAP Fiori 브랜드 파란색
        palette: COLOR_PALETTE,
        localStorageKey: 'spectrum.andon.color',
        maxSelectionSize: 10,
        preferredFormat: 'hex',

        // 드래그 중 실시간 미리보기 갱신
        move: function(color) {
            if (color) colorPreviewBox.css('background-color', color.toHexString());
        },

        // 색상 선택 확정 시 미리보기 갱신 + input 이벤트 발생 (Step 2 미리보기 연동)
        change: function(color) {
            if (color) {
                colorPreviewBox.css('background-color', color.toHexString());
                document.getElementById('color').dispatchEvent(new Event('input', { bubbles: true }));
            } else {
                colorPreviewBox.css('background-color', '#ffffff');
            }
        }
    });

    // 초기 미리보기 박스 색상 설정 (기존 값 또는 기본 파란색)
    colorPreviewBox.css('background-color', colorInput.val() || '#0070f2');
}

/**
 * 수정 모달 열릴 때 Spectrum 컬러피커 값 갱신 (전역 함수)
 * - resource-manager.js에서 폼 데이터 로드 후 이 함수를 호출하여
 *   Spectrum 컬러피커를 DB에서 불러온 color 값으로 설정
 * - window에 노출: resource-manager가 ES Module 외부에서 호출 가능하도록
 */
window.updateAndonFormSpectrum = function(data) {
    if (data && data.color) {
        const colorInput = $('#color');
        if (colorInput.length && typeof colorInput.spectrum === 'function') {
            colorInput.spectrum('set', data.color);
            $('#colorPreviewBox').css('background-color', data.color);
        }
    }
};


// ─── Real-time Search ────────────────────────────────────

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


// ─── Quick Actions ───────────────────────────────────────

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


// ─── Modal Steps ─────────────────────────────────────────

/**
 * 모달 다단계 스텝 초기화 (2단계)
 *
 * Step 1: 기본 정보
 *   - andon_name: 안돈명 (필수 + 중복 확인)
 *   - color: 색상 선택 (Spectrum 컬러피커)
 *
 * Step 2: 상세 설정 + 미리보기
 *   - previewName, previewColor, previewStatus 실시간 반영
 *
 * addBtn 클릭 시 Step 1로 리셋 + Spectrum 컬러피커를 기본 파란색으로 초기화
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
     * Step 1 검증: 안돈명 필수 + 중복 확인 API
     */
    async function validateCurrentStep(step) {
        if (step === 1) {
            const andonName = document.getElementById('andon_name').value.trim();
            if (!andonName) { alert('Please enter the andon name.'); return false; }
            try {
                const currentIdx = document.getElementById('resourceId').value || null;
                const url = `proc/andon.php?for=check-duplicate&andon_name=${encodeURIComponent(andonName)}${currentIdx ? `&current_idx=${currentIdx}` : ''}`;
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
        const name   = document.getElementById('andon_name')?.value || '-';
        const color  = document.getElementById('color')?.value || '-';
        const status = document.getElementById('status')?.selectedOptions[0]?.textContent || '-';
        const pName  = document.getElementById('previewName');
        const pColor = document.getElementById('previewColor');
        const pStat  = document.getElementById('previewStatus');
        if (pName)  pName.textContent  = name;
        if (pColor) pColor.textContent = color;
        if (pStat)  pStat.textContent  = status;
    }

    document.getElementById('andon_name')?.addEventListener('input', updatePreview);
    document.getElementById('color')?.addEventListener('input', updatePreview);
    document.getElementById('status')?.addEventListener('change', updatePreview);

    // 신규 등록 버튼 클릭: Step 1 리셋 + Spectrum 컬러피커 기본 파란색으로 초기화
    document.getElementById('addBtn')?.addEventListener('click', () => {
        currentStep = 1;
        showStep(1);
        // Spectrum 컬러피커 초기화
        const colorInput = $('#color');
        if (colorInput.length && typeof colorInput.spectrum === 'function') {
            colorInput.spectrum('set', '#0070f2');
            $('#colorPreviewBox').css('background-color', '#0070f2');
        }
    });
}
