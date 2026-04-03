<?php
/**
 * info_design_process_2.php — 디자인 공정(Design Process) 관리 페이지
 *
 * 디자인 공정 마스터 데이터 CRUD + Process-Machine 매핑 페이지 (SAP Fiori 스타일)
 *
 * 사용 모듈:
 *   - resource-manager.js         : CRUD, 페이지네이션, 정렬, 필터 통합 관리
 *   - info_design_process_2.js    : designProcessConfig + 검색/필터/모달 스텝 기능
 *     - beforeInit: loadFactoryOptions()로 factoryFilterSelect + factory_idx 드롭다운 초기화
 *     - beforeEdit: factory_idx 설정 → updateLineOptions → model_name 설정
 *                   + setTimeout(100) → window.updateExistingFileInfo(filename)
 *     - apiEndpoint: '../manage/proc/design_process.php' (상위 경로 주의)
 *
 * 모달 구조 (2단계):
 *   Step 1: factory_idx(공장) + line_idx(라인, disabled→공장 선택 후 활성화)
 *           + model_name + design_process(필수) + std_mc_needed(기본=1) + status
 *   Step 2: existingFileInfo(기존 파일 표시) + file_upload(SOP 이미지) + remark
 *           + 확인 미리보기 (previewName/previewStdMc/previewStatus/previewFile)
 *
 * 인라인 스크립트 기능 (ES Module):
 *   - initSopFileUpload()            : SOP 파일 업로드/미리보기 초기화
 *   - initProcessMachineMapping()    : Process-Machine 드래그앤드롭 매핑 패널 초기화
 *
 * 글로벌 함수 (window.*):
 *   - window.updateExistingFileInfo(filename): 수정 모달 열기 시 기존 파일명 표시
 *   - window.resetExistingFileInfo()         : 신규 등록 시 기존 파일 정보 초기화
 *
 * CSS 로드 순서:
 *   1. fiori-page.css             : 공통 SAP Fiori 레이아웃/컴포넌트
 *   2. info_design_process_2.css  : 디자인 공정 관리 페이지 전용 스타일
 *
 * 필터 연쇄:
 *   factoryFilterSelect → factoryLineFilterSelect (공장 선택 시 라인 필터 갱신)
 *   statusFilterSelect: 기본값 'Y'(Used Only)
 *
 * 특이사항:
 *   - enctype="multipart/form-data": SOP 파일 업로드를 위해 필요
 *   - toggleSetProcessBtn: 패널 토글 (.is-open / panel-open)
 *   - Process-Machine 매핑: proc/process_machine_mapping.php API 사용
 *   - escapeHtml(): div.textContent 방식의 XSS 방지
 */
$page_title = 'Design Process Management';
$page_css_files = [
    '../../assets/css/fiori-page.css',
    'css/info_design_process_2.css',
];

require_once(__DIR__ . '/../../inc/head.php');
/* nav-fiori.php 제거 */
?>

<?php
// 네비게이션 드로어: design_process 메뉴 항목을 active 상태로 표시
$nav_active = 'design_process'; require_once(__DIR__ . '/../../inc/nav-drawer-manage.php');
?>

<!-- Signage Header: 상단 네비게이션 바 -->
<div class="signage-header">
    <!-- 햄버거 버튼: 좌측 네비게이션 드로어 열기/닫기 -->
    <button id="navDrawerBtn" class="nav-drawer-btn" aria-label="Menu">&#9776;</button>
    <span class="signage-header__title">Design Process Management</span>

    <!-- 헤더 필터 영역: 검색, 공장/라인 필터, 상태 필터, 컬럼 토글, 각종 버튼 -->
    <div class="signage-header__filters">
        <!-- 실시간 검색 입력창 + 초기화(X) 버튼 -->
        <div class="search-container">
            <input type="text" id="realTimeSearch" class="fiori-input search-input" placeholder="Search process...">
            <button class="search-clear" id="searchClear" title="Clear search">&#10005;</button>
        </div>
        <!-- 공장 필터 드롭다운: beforeInit → loadFactoryOptions()에서 동적으로 채워짐 -->
        <!-- resets: ['lineFilter'] → 공장 변경 시 라인 필터 자동 초기화 -->
        <select id="factoryFilterSelect" class="fiori-select">
            <!-- JS로 채워짐 -->
        </select>
        <!-- 라인 필터 드롭다운: 공장 선택 후 연쇄 갱신 (JS로 채워짐) -->
        <select id="factoryLineFilterSelect" class="fiori-select">
            <!-- JS로 채워짐 -->
        </select>
        <!-- 사용 여부 필터: 기본값 'Y'(Used Only) -->
        <select id="statusFilterSelect" class="fiori-select">
            <option value="">All Status</option>
            <option value="Y" selected>Used Only</option>
            <option value="N">Unused Only</option>
        </select>
        <!-- 컬럼 표시/숨김 토글 드롭다운 (ResourceManager에서 자동 생성) -->
        <div class="fiori-dropdown">
            <button id="columnToggleBtn" class="fiori-btn fiori-btn--secondary">Columns</button>
            <div id="columnToggleDropdown" class="fiori-dropdown__content"></div>
        </div>
        <!-- 신규 등록 버튼: addBtn → initModalSteps()에서 Step 1 리셋 + resetExistingFileInfo() 호출 -->
        <button id="addBtn" class="fiori-btn fiori-btn--primary">Add Process</button>
        <!-- toggleSetProcessBtn: Process-Machine 매핑 패널 토글 -->
        <!-- 클릭 시 panel.classList.toggle('is-open') + loadMappingData() 호출 -->
        <button id="toggleSetProcessBtn" class="fiori-btn fiori-btn--secondary">Set Process</button>
        <!-- 새로고침 버튼: ResourceManager에서 자동으로 loadData() 연결 -->
        <button id="refreshBtn" class="fiori-btn fiori-btn--tertiary">Refresh</button>
    </div>
</div>

<!-- Main: 매핑 패널 + 테이블 카드 영역 -->
<div class="manage-main">

    <!-- Process-Machine 매핑 패널 (토글, 기본값: 숨김) -->
    <!-- toggleSetProcessBtn 클릭 시 'is-open' 클래스 추가 → CSS로 슬라이드 다운 표시 -->
    <!-- manage-main에 'panel-open' 클래스 추가 → 테이블 영역 높이 조정 -->
    <div class="process-machine-panel" id="processMachinePanel">
        <div class="fiori-card">
            <div class="fiori-card__header">
                <div>
                    <h3 class="fiori-card__title">Process-Machine Assignment</h3>
                    <p class="fiori-card__subtitle">Assign Machines to Processes by dragging and dropping.</p>
                </div>
                <div style="display:flex; gap:6px;">
                    <!-- saveProcessMachineBtn: saveMachineAssignments() → POST proc/process_machine_mapping.php -->
                    <button id="saveProcessMachineBtn" class="fiori-btn fiori-btn--primary">Save Changes</button>
                    <!-- cancelProcessMachineBtn: 패널 닫기 ('is-open'/'panel-open' 클래스 제거) -->
                    <button id="cancelProcessMachineBtn" class="fiori-btn fiori-btn--tertiary">Close</button>
                </div>
            </div>

            <!-- 라인 필터 버튼 바: renderLineFilterBar()에서 공장/라인 조합별 버튼 동적 생성 -->
            <!-- 버튼 클릭 → selectLine(lineKey) → renderMappingUI()로 해당 라인 기계만 표시 -->
            <div class="line-filter-bar" id="lineFilterBar">
                <span class="line-filter-label">Filter by Line:</span>
                <!-- JS로 동적 생성 -->
            </div>

            <div class="fiori-card__content" style="padding:0; overflow:hidden;">
                <!-- processMachineContainer: 공정별 행 + 기계 박스 드래그앤드롭 영역 -->
                <!-- 각 행: .process-row → .process-box(공정 정보) + .machines-container.drop-zone(기계 박스) -->
                <!-- Unassigned 행: design_process_idx=0인 기계들을 모아 표시 -->
                <div id="processMachineContainer" class="process-machine-container">
                    <!-- JS로 동적 생성 -->
                </div>
            </div>
        </div>
    </div>

    <!-- 데이터 테이블 카드: ResourceManager가 동적으로 채움 -->
    <div class="fiori-card">
        <div class="fiori-card__header">
            <div>
                <h3 class="fiori-card__title">Design Process List</h3>
            </div>
            <!-- 빠른 필터 버튼: initQuickActions()에서 DOM 행 직접 숨김/표시 (API 재요청 없음) -->
            <!-- 'used': row.textContent에 'Used' 포함 && 'Unused' 미포함인 행만 표시 -->
            <div class="quick-actions">
                <button class="quick-action-btn active" data-filter="all">All</button>
                <button class="quick-action-btn" data-filter="used">Used</button>
                <button class="quick-action-btn" data-filter="unused">Unused</button>
            </div>
        </div>

        <div class="fiori-card__content fiori-p-0">
            <div style="overflow-x: auto; height: 100%;">
                <!-- ResourceManager가 tableHeader(thead)/tableBody(tbody)를 동적으로 채움 -->
                <!-- tableBody: 이벤트 위임으로 .file-preview-icon 클릭 + .delete-btn 클릭 처리 -->
                <table class="fiori-table">
                    <thead id="tableHeader" class="fiori-table__header"></thead>
                    <tbody id="tableBody"></tbody>
                </table>
            </div>
            <!-- 검색/필터 결과 없을 때 표시되는 안내 메시지 -->
            <div id="noDataMessage" class="text-center" style="padding: var(--sap-spacing-xl); color: var(--sap-text-secondary); display: none;">
                <p>No matching design processes found.</p>
            </div>
        </div>
    </div>

    <!-- 페이지네이션 컨트롤: ResourceManager가 동적으로 생성 -->
    <div id="pagination-controls" class="fiori-pagination"></div>

</div><!-- /manage-main -->


<!-- Design Process Modal: 디자인 공정 등록/수정 다이얼로그 (2단계) -->
<!-- ResourceManager가 행 클릭(수정) 또는 addBtn 클릭(신규) 시 'show' 클래스를 추가하여 표시 -->
<div id="resourceModal" class="fiori-modal">
    <div class="fiori-modal__content">
        <div class="fiori-card">
            <div class="fiori-card__header">
                <div>
                    <!-- modalTitle: 신규 등록 시 'Add Design Process', 수정 시 'Edit Design Process' -->
                    <h3 class="fiori-card__title" id="modalTitle">Design Process Information</h3>
                    <p class="fiori-card__subtitle">Enter and edit design process information</p>
                </div>
                <!-- 모달 닫기 버튼 (.close 클래스): ResourceManager에서 이벤트 처리 -->
                <button type="button" class="fiori-btn fiori-btn--tertiary fiori-btn--sm close"
                        style="position: absolute; top: var(--sap-spacing-md); right: var(--sap-spacing-md);">
                    &#10005;
                </button>
            </div>
            <div class="fiori-card__content">
                <!-- enctype="multipart/form-data": SOP 파일 업로드를 위해 필요 -->
                <form id="resourceForm" class="fiori-form" enctype="multipart/form-data">
                    <!-- 숨김 필드: 수정 시 대상 레코드의 PK 값 저장 (신규 시 비어있음) -->
                    <input type="hidden" id="resourceId" name="idx">

                    <!-- 2단계 스텝 인디케이터: .form-step 요소 + stepIndicator 텍스트 -->
                    <div style="display: flex; justify-content: space-between; align-items: center; margin-bottom: var(--sap-spacing-lg); padding-bottom: var(--sap-spacing-md); border-bottom: 1px solid var(--sap-border-subtle);">
                        <div style="display: flex; gap: var(--sap-spacing-sm); align-items: center;">
                            <div class="form-step active" data-step="1">1</div>
                            <div class="form-step-line"></div>
                            <div class="form-step" data-step="2">2</div>
                        </div>
                        <div style="font-size: var(--sap-font-size-sm); color: var(--sap-text-secondary);">
                            <span id="stepIndicator">Step 1 of 2</span>
                        </div>
                    </div>

                    <!-- Step 1: 기본 정보 -->
                    <!-- validateCurrentStep Step1: factory_idx + line_idx + design_process 필수 + 중복 확인 -->
                    <div class="form-section" data-section="1">
                        <h4 style="margin-bottom: var(--sap-spacing-md); color: var(--sap-text-primary);">Basic information</h4>

                        <div class="fiori-form__group">
                            <label for="factory_idx" class="fiori-form__label fiori-form__label--required">Factory</label>
                            <!-- loadFactoryOptions()에서 공장 목록으로 채워짐 -->
                            <!-- 선택 시 updateLineOptions()로 line_idx 연쇄 갱신 -->
                            <select id="factory_idx" name="factory_idx" class="fiori-select" required>
                                <option value="">Please select a factory</option>
                            </select>
                            <div class="fiori-form__help">Please select a factory</div>
                        </div>

                        <div class="fiori-form__group">
                            <label for="line_idx" class="fiori-form__label fiori-form__label--required">Line</label>
                            <!-- factory_idx 선택 전: disabled=true -->
                            <!-- factory_idx 선택 후: updateLineOptions()로 활성화 + 라인 목록 갱신 -->
                            <!-- updateLineOptions에서 '../manage/proc/design_process.php?for=lines' 경로 사용 -->
                            <select id="line_idx" name="line_idx" class="fiori-select" required disabled>
                                <option value="">Please select a line</option>
                            </select>
                            <div class="fiori-form__help">Select line after selecting factory</div>
                        </div>

                        <div class="fiori-form__group">
                            <label for="model_name" class="fiori-form__label">Model Name</label>
                            <!-- 선택 입력: 제품 모델명 (예: AIR MAX MOTO 2K (W)) -->
                            <!-- beforeEdit에서 data.model_name 값으로 초기화 -->
                            <input type="text" id="model_name" name="model_name" class="fiori-input"
                                   placeholder="Enter product model name">
                            <div class="fiori-form__help">Enter the product model name (e.g., AIR MAX MOTO 2K (W))</div>
                        </div>

                        <div class="fiori-form__group">
                            <label for="design_process" class="fiori-form__label fiori-form__label--required">Design Process Name</label>
                            <!-- validateCurrentStep에서 중복 확인 API 호출 (공장+라인+공정명 조합) -->
                            <input type="text" id="design_process" name="design_process" class="fiori-input" required
                                   placeholder="Please enter the design process name">
                            <div class="fiori-form__help">Please enter a unique design process name (2-50 characters)</div>
                        </div>

                        <div class="fiori-form__group">
                            <label for="std_mc_needed" class="fiori-form__label">Standard MC Needed</label>
                            <!-- 표준 필요 기계 수: 1 이상, 기본값 1 -->
                            <!-- Process-Machine 매핑에서 수용 가능 기계 수의 기준값 -->
                            <input type="number" id="std_mc_needed" name="std_mc_needed" class="fiori-input" value="1" min="1"
                                   placeholder="Enter the number of machines needed">
                            <div class="fiori-form__help">Number of standard machines needed for this process (default: 1)</div>
                        </div>

                        <div class="fiori-form__group">
                            <label for="status" class="fiori-form__label">Status</label>
                            <select id="status" name="status" class="fiori-select">
                                <option value="Y">Used</option>
                                <option value="N">Unused</option>
                            </select>
                            <div class="fiori-form__help">Please select the current usage status.</div>
                        </div>
                    </div>

                    <!-- Step 2: 추가 정보 (SOP 파일 + 비고 + 확인 미리보기) -->
                    <div class="form-section" data-section="2" style="display: none;">
                        <h4 style="margin-bottom: var(--sap-spacing-md); color: var(--sap-text-primary);">Additional information</h4>

                        <div class="fiori-form__group">
                            <label for="file_upload" class="fiori-form__label">SOP File Upload</label>

                            <!-- existingFileInfo: 수정 시 기존 SOP 파일 정보 표시 패널 -->
                            <!-- window.updateExistingFileInfo(filename)에서 display:block으로 전환 -->
                            <!-- viewExistingFileBtn 클릭 → imagePreviewModal 열기 -->
                            <div id="existingFileInfo" style="display: none; margin-bottom: var(--sap-spacing-sm); padding: var(--sap-spacing-sm); background: var(--sap-surface-2); border-radius: var(--sap-radius-sm); border-left: 3px solid var(--sap-status-info);">
                                <div style="display: flex; align-items: center; justify-content: space-between;">
                                    <div>
                                        <span style="font-size: var(--sap-font-size-sm); color: var(--sap-text-secondary);">Current file:</span><br>
                                        <!-- existingFileName: 기존 파일명 텍스트 표시 -->
                                        <span id="existingFileName" style="font-weight: 500; color: var(--sap-text-primary);"></span>
                                    </div>
                                    <!-- viewExistingFileBtn: 클릭 시 ../../upload/sop/{filename} 경로로 이미지 미리보기 -->
                                    <button type="button" id="viewExistingFileBtn" class="fiori-btn fiori-btn--tertiary fiori-btn--sm" style="min-width: auto;">
                                        View
                                    </button>
                                </div>
                            </div>

                            <!-- file_upload: JPG/JPEG/PNG 파일만 허용, 최대 10MB -->
                            <!-- fileInput.change 이벤트: MIME 타입 + 파일 크기 검증 -->
                            <input type="file" id="file_upload" name="file_upload" class="fiori-input"
                                   accept=".jpg,.jpeg,.png">
                            <div class="fiori-form__help">Upload SOP file (JPG, JPEG, PNG only, max 10MB). Leave empty to keep existing file.</div>
                        </div>

                        <div class="fiori-form__group">
                            <label for="remark" class="fiori-form__label">Remark</label>
                            <textarea id="remark" name="remark" class="fiori-input" rows="3"></textarea>
                            <div class="fiori-form__help">You can record design process information.</div>
                        </div>

                        <!-- 확인 미리보기 패널: updatePreview()에서 Step1 입력값 반영 -->
                        <div style="padding: var(--sap-spacing-md); background: var(--sap-surface-2); border-radius: var(--sap-radius-md); margin-top: var(--sap-spacing-md);">
                            <h5 style="margin: 0 0 var(--sap-spacing-sm) 0; color: var(--sap-text-primary);">Confirm input information</h5>
                            <div style="font-size: var(--sap-font-size-sm); color: var(--sap-text-secondary);">
                                <!-- previewName: design_process 입력값 반영 -->
                                <div><strong>Process name:</strong> <span id="previewName">-</span></div>
                                <!-- previewStdMc: std_mc_needed 입력값 반영 -->
                                <div><strong>Standard MC needed:</strong> <span id="previewStdMc">-</span></div>
                                <!-- previewStatus: status 선택 텍스트 반영 -->
                                <div><strong>Status:</strong> <span id="previewStatus">-</span></div>
                                <!-- previewFile: 선택한 파일명 또는 '(existing)' 반영 -->
                                <div><strong>File:</strong> <span id="previewFile">-</span></div>
                            </div>
                        </div>
                    </div>

                    <!-- 폼 액션 버튼 -->
                    <div class="fiori-form__actions">
                        <!-- modalCloseBtn: ResourceManager에서 모달 닫기 처리 -->
                        <button type="button" class="fiori-btn fiori-btn--tertiary" id="modalCloseBtn">Cancel</button>
                        <!-- prevStep: Step > 1일 때만 표시, 클릭 시 이전 스텝으로 이동 -->
                        <button type="button" class="fiori-btn fiori-btn--secondary" id="prevStep" style="display: none;">Previous</button>
                        <!-- nextStep: Step < totalSteps일 때 표시, 클릭 시 현재 스텝 검증 후 다음 스텝 이동 -->
                        <button type="button" class="fiori-btn fiori-btn--primary" id="nextStep">Next</button>
                        <!-- submitBtn: 마지막 스텝에서만 표시, 폼 제출 (ResourceManager가 처리) -->
                        <button type="submit" class="fiori-btn fiori-btn--primary" id="submitBtn" style="display: none;">Save Process</button>
                    </div>
                </form>
            </div>
        </div>
    </div>
</div>


<!-- SOP 이미지 미리보기 모달: z-index:9999 (resourceModal 위에 표시) -->
<!-- viewExistingFileBtn 클릭 또는 tableBody 내 .file-preview-icon 클릭 시 표시 -->
<div id="imagePreviewModal" class="fiori-modal" style="z-index: 9999;">
    <div class="fiori-modal__content" style="max-width: 90vw; max-height: 90vh; width: auto; height: auto;">
        <div class="fiori-card" style="height: 100%; display: flex; flex-direction: column;">
            <div class="fiori-card__header">
                <div>
                    <h3 class="fiori-card__title">SOP File Preview</h3>
                    <!-- imageFileName: 미리보기 중인 파일명 텍스트 -->
                    <p class="fiori-card__subtitle" id="imageFileName">Image preview</p>
                </div>
                <!-- imageModalClose: 헤더 X 버튼 → closeImagePreviewModal() 호출 -->
                <button type="button" class="fiori-btn fiori-btn--tertiary fiori-btn--sm" id="imageModalClose"
                        style="position: absolute; top: var(--sap-spacing-md); right: var(--sap-spacing-md);">
                    &#10005;
                </button>
            </div>
            <div class="fiori-card__content" style="flex: 1; display: flex; align-items: center; justify-content: center; padding: var(--sap-spacing-lg);">
                <!-- previewImage: src="../../upload/sop/{filename}" 로 동적 설정 -->
                <!-- onerror 처리: showImagePreviewModal()에서 이미지 로드 실패 시 오류 메시지 표시 -->
                <img id="previewImage" src="" alt="SOP Preview"
                     style="max-width: 100%; max-height: 100%; object-fit: contain; border-radius: var(--sap-radius-md); box-shadow: 0 4px 12px rgba(0,0,0,0.15);">
            </div>
            <div class="fiori-card__content" style="padding-top: 0; text-align: center;">
                <!-- imageModalCloseBtn: 하단 Close 버튼 → closeImagePreviewModal() 호출 -->
                <button type="button" class="fiori-btn fiori-btn--primary" id="imageModalCloseBtn">Close</button>
            </div>
        </div>
    </div>
</div>


<script type="module">
    /**
     * ES Module 진입점
     *
     * 초기화 순서:
     * 1. createResourceManager(designProcessConfig): CRUD/테이블/페이지네이션 초기화
     * 2. setTimeout 100ms 후:
     *    a. initAdvancedFeatures(resourceManager): 검색/필터/모달 스텝/MutationObserver 초기화
     *    b. initSopFileUpload(): SOP 파일 업로드 + 이미지 미리보기 모달 초기화
     *    c. initProcessMachineMapping(resourceManager): 드래그앤드롭 매핑 패널 초기화
     *    (100ms 지연: ResourceManager DOM 렌더링 완료 + 모달 요소 준비 보장)
     */
    import { createResourceManager } from '../../assets/js/resource-manager.js';
    import { initAdvancedFeatures, designProcessConfig } from './js/info_design_process_2.js';

    document.addEventListener('DOMContentLoaded', function() {
        const resourceManager = createResourceManager(designProcessConfig);

        setTimeout(() => {
            initAdvancedFeatures(resourceManager);
            initSopFileUpload();
            initProcessMachineMapping(resourceManager);
        }, 100);
    });

    /**
     * initSopFileUpload — SOP 파일 업로드 + 이미지 미리보기 모달 초기화
     *
     * 처리 내용:
     *   - fileInput.change: MIME 타입(jpg/jpeg/png) 및 파일 크기(10MB) 검증
     *   - viewExistingFileBtn.click: currentExistingFile 경로로 미리보기 모달 열기
     *   - closeImagePreviewModal: 애니메이션 300ms 후 display:none 처리
     *   - imagePreviewModal 배경 클릭: 모달 닫기
     *
     * 전역 함수 노출 (window.*):
     *   - window.updateExistingFileInfo(filename): beforeEdit에서 호출, 기존 파일명 표시
     *   - window.resetExistingFileInfo(): addBtn 클릭 시 호출, 기존 파일 정보 초기화
     */
    function initSopFileUpload() {
        const fileInput          = document.getElementById('file_upload');
        const imageModalClose    = document.getElementById('imageModalClose');
        const imageModalCloseBtn = document.getElementById('imageModalCloseBtn');
        const imagePreviewModal  = document.getElementById('imagePreviewModal');
        const existingFileInfo   = document.getElementById('existingFileInfo');
        const existingFileName   = document.getElementById('existingFileName');
        const viewExistingFileBtn = document.getElementById('viewExistingFileBtn');
        const previewImage       = document.getElementById('previewImage');
        const imageFileName      = document.getElementById('imageFileName');

        // currentExistingFile: 현재 수정 중인 레코드의 기존 SOP 파일명 (클로저 변수)
        let currentExistingFile = null;

        // 파일 선택 이벤트: MIME 타입 + 파일 크기 검증
        fileInput.addEventListener('change', function(e) {
            const file = e.target.files[0];
            if (!file) return;
            const allowed = ['image/jpeg', 'image/jpg', 'image/png'];
            if (!allowed.includes(file.type)) {
                alert('Only JPG, JPEG, PNG files are allowed.');
                fileInput.value = '';
                return;
            }
            // 10MB 초과 시 선택 취소
            if (file.size > 10 * 1024 * 1024) {
                alert('File size exceeds 10MB.');
                fileInput.value = '';
            }
        });

        // 기존 파일 보기 버튼: ../../upload/sop/{currentExistingFile} 경로로 미리보기 모달 열기
        if (viewExistingFileBtn) {
            viewExistingFileBtn.addEventListener('click', function() {
                if (currentExistingFile) {
                    previewImage.src = `../../upload/sop/${currentExistingFile}`;
                    imageFileName.textContent = currentExistingFile;
                    imagePreviewModal.style.display = 'flex';
                    // 10ms 후 'show' 클래스 추가: CSS 트랜지션(fade-in) 트리거
                    setTimeout(() => imagePreviewModal.classList.add('show'), 10);
                }
            });
        }

        /**
         * closeImagePreviewModal — 이미지 미리보기 모달 닫기
         * 'show' 클래스 제거 → CSS 트랜지션 시작 → 300ms 후 display:none
         */
        function closeImagePreviewModal() {
            imagePreviewModal.classList.remove('show');
            setTimeout(() => { imagePreviewModal.style.display = 'none'; }, 300);
        }

        // 헤더 X 버튼 + 하단 Close 버튼 모두 closeImagePreviewModal에 연결
        [imageModalClose, imageModalCloseBtn].forEach(btn => btn.addEventListener('click', closeImagePreviewModal));
        // 모달 배경(fiori-modal 자체) 클릭 시 닫기
        imagePreviewModal.addEventListener('click', function(e) {
            if (e.target === imagePreviewModal) closeImagePreviewModal();
        });

        /**
         * window.updateExistingFileInfo — 수정 모달 열기 시 기존 파일 정보 표시
         * @param {string} filename - 기존 SOP 파일명 (빈 문자열이면 숨김 처리)
         * beforeEdit에서 setTimeout(100) 후 호출 (ResourceManager 모달 채우기 완료 후)
         */
        window.updateExistingFileInfo = function(filename) {
            currentExistingFile = filename;
            if (filename && filename.trim() !== '') {
                existingFileName.textContent = filename;
                existingFileInfo.style.display = 'block';
            } else {
                existingFileInfo.style.display = 'none';
            }
        };

        /**
         * window.resetExistingFileInfo — 신규 등록 시 기존 파일 정보 초기화
         * addBtn 클릭 이벤트 핸들러에서 호출
         */
        window.resetExistingFileInfo = function() {
            currentExistingFile = null;
            existingFileInfo.style.display = 'none';
            fileInput.value = '';
        };
    }

    /**
     * initProcessMachineMapping — Process-Machine 드래그앤드롭 매핑 패널 초기화
     * @param {object} resourceManager - ResourceManager 인스턴스 (저장 후 loadData() 호출용)
     *
     * 상태 변수:
     *   - mappingData: API에서 로드한 {processes, machines} 데이터
     *   - currentAssignments: { machine_idx: design_process_idx } 현재 할당 상태 (UI 변경 즉시 반영)
     *   - selectedLine: 현재 선택된 라인 키 (공장명-라인명 조합)
     *   - autoScrollInterval: 드래그 중 상하 자동 스크롤 인터벌 ID
     *
     * 자동 스크롤 상수:
     *   - SCROLL_ZONE_SIZE: 80px — 마우스가 컨테이너 상/하단 80px 이내 진입 시 스크롤 시작
     *   - SCROLL_SPEED: 10px/16ms — 자동 스크롤 속도 (약 625px/sec)
     */
    function initProcessMachineMapping(resourceManager) {
        const toggleBtn    = document.getElementById('toggleSetProcessBtn');
        const panel        = document.getElementById('processMachinePanel');
        const managMain    = panel.closest('.manage-main');
        const container    = document.getElementById('processMachineContainer');
        const lineFilterBar = document.getElementById('lineFilterBar');
        const saveBtn      = document.getElementById('saveProcessMachineBtn');
        const cancelBtn    = document.getElementById('cancelProcessMachineBtn');

        let mappingData        = null;
        let currentAssignments = {};
        let selectedLine       = null;

        let autoScrollInterval = null;
        const SCROLL_ZONE_SIZE = 80;
        const SCROLL_SPEED     = 10;

        // toggleSetProcessBtn: 패널 열기/닫기 토글
        // 열기 시: 'is-open'/'panel-open' 클래스 추가 + loadMappingData() 호출
        // 닫기 시: 클래스 제거 + 버튼 텍스트 원복
        toggleBtn.addEventListener('click', async function() {
            if (!panel.classList.contains('is-open')) {
                panel.classList.add('is-open');
                managMain.classList.add('panel-open');
                toggleBtn.textContent = 'Hide Process';
                await loadMappingData();
            } else {
                panel.classList.remove('is-open');
                managMain.classList.remove('panel-open');
                toggleBtn.textContent = 'Set Process';
            }
        });

        // cancelBtn: 패널 닫기 (저장하지 않고 닫기)
        cancelBtn.addEventListener('click', function() {
            panel.classList.remove('is-open');
            managMain.classList.remove('panel-open');
            toggleBtn.textContent = 'Set Process';
        });

        // saveBtn: saveMachineAssignments() → POST proc/process_machine_mapping.php
        saveBtn.addEventListener('click', async function() {
            await saveMachineAssignments();
        });

        /**
         * loadMappingData — 매핑 데이터 API 로드
         * GET proc/process_machine_mapping.php?action=get_mapping
         * 응답: { success: true, data: { processes: [...], machines: [...] } }
         * 성공 시: currentAssignments 초기화 + renderLineFilterBar() + renderMappingUI()
         */
        async function loadMappingData() {
            try {
                const response = await fetch('proc/process_machine_mapping.php?action=get_mapping');
                const result   = await response.json();

                if (result.success) {
                    mappingData = result.data;
                    // currentAssignments 초기화: 각 기계의 현재 공정 할당값 설정
                    currentAssignments = {};
                    mappingData.machines.forEach(machine => {
                        currentAssignments[machine.idx] = machine.design_process_idx || 0;
                    });
                    renderLineFilterBar();
                    renderMappingUI();
                } else {
                    alert('Failed to load data: ' + result.message);
                }
            } catch (error) {
                console.error('Error loading mapping data:', error);
                alert('An error occurred while loading data.');
            }
        }

        /**
         * renderLineFilterBar — 라인 필터 버튼 바 렌더링
         * mappingData.machines에서 공장명-라인명 조합을 추출하여 고유 라인 목록 생성
         * 첫 번째 라인을 자동 선택 (selectedLine === null 인 경우)
         */
        function renderLineFilterBar() {
            if (!mappingData) return;

            // lineMap: 공장-라인 조합별 표시명 + 기계 수 집계
            const lineMap = new Map();
            mappingData.machines.forEach(machine => {
                const lineKey     = `${machine.factory_name || 'N/A'}-${machine.line_name || 'N/A'}`;
                const lineDisplay = `${machine.factory_name || 'N/A'} / ${machine.line_name || 'N/A'}`;
                if (!lineMap.has(lineKey)) {
                    lineMap.set(lineKey, { key: lineKey, display: lineDisplay, count: 0 });
                }
                lineMap.get(lineKey).count++;
            });

            const lines = Array.from(lineMap.values()).sort((a, b) => a.display.localeCompare(b.display));

            // 기존 버튼 제거 (라벨 텍스트 노드만 유지)
            const filterBarLabel = lineFilterBar.querySelector('.line-filter-label');
            while (filterBarLabel.nextSibling) lineFilterBar.removeChild(filterBarLabel.nextSibling);

            lines.forEach((lineInfo, index) => {
                const btn = document.createElement('button');
                btn.className        = 'line-filter-btn';
                btn.dataset.lineKey  = lineInfo.key;
                // 라인명 + 기계 수 뱃지 표시
                btn.innerHTML        = `${lineInfo.display} <span class="line-filter-count">${lineInfo.count}</span>`;
                btn.addEventListener('click', () => selectLine(lineInfo.key));
                lineFilterBar.appendChild(btn);

                // 첫 번째 라인 자동 선택
                if (index === 0 && selectedLine === null) {
                    selectedLine = lineInfo.key;
                    btn.classList.add('active');
                }
            });
        }

        /**
         * selectLine — 라인 필터 선택 및 UI 갱신
         * @param {string} lineKey - '공장명-라인명' 조합 키
         * 선택된 버튼에 'active' 클래스 추가, 나머지 제거 후 renderMappingUI() 호출
         */
        function selectLine(lineKey) {
            selectedLine = lineKey;
            document.querySelectorAll('.line-filter-btn').forEach(btn => {
                btn.classList.toggle('active', btn.dataset.lineKey === lineKey);
            });
            renderMappingUI();
        }

        /**
         * getFilteredMachines — 선택된 라인의 기계 목록 반환
         * selectedLine을 기준으로 mappingData.machines 필터링
         */
        function getFilteredMachines() {
            if (!selectedLine) return [];
            return mappingData.machines.filter(m => `${m.factory_name || 'N/A'}-${m.line_name || 'N/A'}` === selectedLine);
        }

        /**
         * getFilteredProcesses — 선택된 라인의 공정 목록 반환
         * selectedLine을 기준으로 mappingData.processes 필터링
         */
        function getFilteredProcesses() {
            if (!selectedLine) return [];
            return mappingData.processes.filter(p => `${p.factory_name || 'N/A'}-${p.line_name || 'N/A'}` === selectedLine);
        }

        /**
         * renderMappingUI — 매핑 UI 전체 재렌더링
         * 필터된 공정별 행(createProcessRow) + 미배정 행(createEmptyRow) 생성
         */
        function renderMappingUI() {
            if (!mappingData) return;
            container.innerHTML = '';
            getFilteredProcesses().forEach(process => {
                container.appendChild(createProcessRow(process, getFilteredMachines()));
            });
            // 마지막에 Unassigned 행 추가 (design_process_idx=0인 기계들)
            container.appendChild(createEmptyRow(getFilteredMachines()));
        }

        /**
         * createProcessRow — 공정 행 DOM 생성
         * @param {object} process - 공정 데이터 { idx, design_process, std_mc_needed, ... }
         * @param {Array} filteredMachines - 현재 라인의 기계 목록
         *
         * 구조:
         *   .process-row[data-process-idx]
         *     .process-box (클릭 → .editing 클래스 토글)
         *       .process-box-view (공정명/capacity/배정 수 표시)
         *       .process-box-edit (공정명/capacity 수정 폼)
         *     .machines-container.drop-zone (드롭 존, 배정된 기계 박스들)
         */
        function createProcessRow(process, filteredMachines) {
            const row = document.createElement('div');
            row.className         = 'process-row';
            row.dataset.processIdx = process.idx;

            const processBox = document.createElement('div');
            processBox.className         = 'process-box';
            processBox.dataset.processIdx = process.idx;
            processBox.dataset.stdMcNeeded = process.std_mc_needed;

            // 현재 배정 기계 수 + 여유 슬롯 계산
            const currentMachineCount = filteredMachines.filter(m =>
                parseInt(currentAssignments[m.idx]) === parseInt(process.idx)
            ).length;
            const availableSlots = process.std_mc_needed - currentMachineCount;
            const isFull         = currentMachineCount >= process.std_mc_needed;

            // view 모드: 공정명 + capacity + 배정 현황 표시
            const viewDiv = document.createElement('div');
            viewDiv.className = 'process-box-view';
            viewDiv.innerHTML = `
                <div class="process-box-header">
                    <div class="process-name">${escapeHtml(process.design_process)}</div>
                    <div class="process-capacity">Capacity: <span class="process-capacity-badge ${isFull ? 'full' : ''}">${process.std_mc_needed}</span></div>
                    <div class="machine-count">Assigned: <span class="machine-count-current">${currentMachineCount}</span> / Available: <span class="machine-count-available ${isFull ? 'full' : ''}">${availableSlots}</span></div>
                </div>`;

            // edit 모드: 공정명 + capacity 수정 인풋 + 저장/취소 버튼
            const editDiv = document.createElement('div');
            editDiv.className = 'process-box-edit';
            editDiv.innerHTML = `
                <input type="text" class="fiori-input" value="${escapeHtml(process.design_process)}" placeholder="Process Name">
                <input type="number" class="fiori-input" value="${process.std_mc_needed}" min="1" placeholder="Capacity">
                <div class="edit-actions">
                    <button class="fiori-btn fiori-btn--primary fiori-btn--sm save-process">Save</button>
                    <button class="fiori-btn fiori-btn--tertiary fiori-btn--sm cancel-edit">Cancel</button>
                </div>`;

            processBox.appendChild(viewDiv);
            processBox.appendChild(editDiv);

            // process-box-view 클릭 → 편집 모드 진입 (.editing 클래스 추가)
            processBox.addEventListener('click', function(e) {
                if (!processBox.classList.contains('editing') && (e.target === processBox || e.target.closest('.process-box-view'))) {
                    processBox.classList.add('editing');
                }
            });

            // save-process: saveProcessInfo() → POST proc/process_machine_mapping.php?action=update_process
            editDiv.querySelector('.save-process').addEventListener('click', async function(e) {
                e.stopPropagation();
                await saveProcessInfo(processBox, process.idx);
            });

            // cancel-edit: 편집 취소 → 원래 값으로 복원
            editDiv.querySelector('.cancel-edit').addEventListener('click', function(e) {
                e.stopPropagation();
                processBox.classList.remove('editing');
                editDiv.querySelectorAll('input')[0].value = process.design_process;
                editDiv.querySelectorAll('input')[1].value = process.std_mc_needed;
            });

            row.appendChild(processBox);

            // drop-zone: 기계 박스 드롭 영역
            const machinesContainer = document.createElement('div');
            machinesContainer.className          = 'machines-container drop-zone';
            machinesContainer.dataset.processIdx  = process.idx;
            machinesContainer.dataset.stdMcNeeded = process.std_mc_needed;

            // 현재 이 공정에 배정된 기계 박스들 추가
            filteredMachines.filter(m =>
                parseInt(currentAssignments[m.idx]) === parseInt(process.idx)
            ).forEach(machine => machinesContainer.appendChild(createMachineBox(machine)));

            setupDropZone(machinesContainer);
            row.appendChild(machinesContainer);
            return row;
        }

        /**
         * createEmptyRow — 미배정(Unassigned) 행 생성
         * @param {Array} filteredMachines - 현재 라인의 기계 목록
         * design_process_idx = 0 (또는 null/undefined)인 기계들을 모아 표시
         */
        function createEmptyRow(filteredMachines) {
            const row = document.createElement('div');
            row.className = 'process-row';

            const emptyBox = document.createElement('div');
            emptyBox.className   = 'process-box empty';
            emptyBox.textContent = 'Unassigned';
            row.appendChild(emptyBox);

            const machinesContainer = document.createElement('div');
            machinesContainer.className         = 'machines-container drop-zone';
            machinesContainer.dataset.processIdx = '0'; // 미배정 = 0

            filteredMachines.filter(m =>
                !currentAssignments[m.idx] || parseInt(currentAssignments[m.idx]) === 0
            ).forEach(machine => machinesContainer.appendChild(createMachineBox(machine)));

            setupDropZone(machinesContainer);
            row.appendChild(machinesContainer);
            return row;
        }

        /**
         * createMachineBox — 기계 박스 DOM 생성 (드래그 가능)
         * @param {object} machine - 기계 데이터 { idx, machine_no, factory_name, line_name }
         * draggable=true + dragstart/dragend 이벤트 등록
         */
        function createMachineBox(machine) {
            const box = document.createElement('div');
            box.className          = 'machine-box';
            box.draggable          = true;
            box.dataset.machineIdx = machine.idx;
            box.dataset.machineNo  = machine.machine_no;
            box.innerHTML = `
                <div class="machine-box-info">
                    <div class="machine-box-location">
                        <div class="machine-box-factory">${escapeHtml(machine.factory_name || 'N/A')}</div>
                        <div class="machine-box-line">${escapeHtml(machine.line_name || 'N/A')}</div>
                    </div>
                    <div class="machine-box-number">${escapeHtml(machine.machine_no)}</div>
                </div>`;
            box.addEventListener('dragstart', handleDragStart);
            box.addEventListener('dragend', handleDragEnd);
            return box;
        }

        /**
         * startAutoScroll — 자동 스크롤 시작
         * @param {string} direction - 'up' 또는 'down'
         * 16ms 인터벌로 SCROLL_SPEED(10px)씩 스크롤 (이미 실행 중이면 중복 시작 방지)
         */
        function startAutoScroll(direction) {
            if (autoScrollInterval) return;
            autoScrollInterval = setInterval(() => {
                container.scrollTop += direction === 'up' ? -SCROLL_SPEED : SCROLL_SPEED;
            }, 16);
        }

        /**
         * stopAutoScroll — 자동 스크롤 정지
         * clearInterval 후 autoScrollInterval = null 초기화
         */
        function stopAutoScroll() {
            if (autoScrollInterval) { clearInterval(autoScrollInterval); autoScrollInterval = null; }
        }

        /**
         * handleGlobalDragOver — 전역 dragover 이벤트 핸들러 (자동 스크롤 트리거)
         * 드래그 중 마우스가 processMachineContainer 상/하단 SCROLL_ZONE_SIZE(80px) 이내에 있으면
         * 해당 방향으로 자동 스크롤 시작, 그 외 구간에서는 정지
         */
        function handleGlobalDragOver(e) {
            if (!container) return;
            const rect        = container.getBoundingClientRect();
            const visibleTop  = Math.max(rect.top, 0);
            const visibleBot  = Math.min(rect.bottom, window.innerHeight);
            const mouseY      = e.clientY;
            if (mouseY >= visibleTop && mouseY <= visibleBot) {
                if (mouseY - visibleTop < SCROLL_ZONE_SIZE)      startAutoScroll('up');
                else if (visibleBot - mouseY < SCROLL_ZONE_SIZE) startAutoScroll('down');
                else stopAutoScroll();
            } else {
                stopAutoScroll();
            }
        }

        /**
         * setupDropZone — 드롭 존에 이벤트 리스너 등록
         * @param {HTMLElement} dropZone - 드롭 존 요소 (.machines-container.drop-zone)
         */
        function setupDropZone(dropZone) {
            dropZone.addEventListener('dragover',  handleDragOver);
            dropZone.addEventListener('dragleave', handleDragLeave);
            dropZone.addEventListener('drop',      handleDrop);
        }

        /**
         * handleDragStart — 드래그 시작 이벤트 핸들러
         * .dragging 클래스 추가 + machine_idx를 dataTransfer에 저장
         * 전역 dragover 이벤트 등록 (자동 스크롤 감지)
         */
        function handleDragStart(e) {
            e.currentTarget.classList.add('dragging');
            e.dataTransfer.effectAllowed = 'move';
            e.dataTransfer.setData('machine_idx', e.currentTarget.dataset.machineIdx);
            document.addEventListener('dragover', handleGlobalDragOver);
        }

        /**
         * handleDragEnd — 드래그 종료 이벤트 핸들러
         * .dragging 클래스 제거 + 자동 스크롤 정지 + 전역 dragover 이벤트 제거
         */
        function handleDragEnd(e) {
            e.currentTarget.classList.remove('dragging');
            stopAutoScroll();
            document.removeEventListener('dragover', handleGlobalDragOver);
        }

        /**
         * handleDragOver — 드롭 존 위에서 드래그 중 이벤트 핸들러
         * preventDefault()로 드롭 허용 + 'drag-over' 클래스로 시각적 피드백
         */
        function handleDragOver(e) {
            e.preventDefault();
            e.dataTransfer.dropEffect = 'move';
            e.currentTarget.classList.add('drag-over');
            return false;
        }

        /**
         * handleDragLeave — 드롭 존에서 드래그 이탈 이벤트 핸들러
         * 'drag-over' 클래스 제거
         */
        function handleDragLeave(e) {
            e.currentTarget.classList.remove('drag-over');
        }

        /**
         * handleDrop — 드롭 이벤트 핸들러 (기계 배정 처리)
         * 1. dataTransfer에서 machine_idx, dropZone에서 targetProcessIdx 추출
         * 2. targetProcessIdx !== 0이면 std_mc_needed 초과 여부 검사 (라인 내 현재 배정 수 기준)
         * 3. 검사 통과 시: draggedElement를 dropZone으로 이동 + currentAssignments 업데이트
         * 4. updateAllMachineCounts()로 모든 행의 배정 카운터 갱신
         */
        function handleDrop(e) {
            e.stopPropagation();
            e.preventDefault();
            stopAutoScroll();
            document.removeEventListener('dragover', handleGlobalDragOver);

            const dropZone       = e.currentTarget;
            dropZone.classList.remove('drag-over');

            const machineIdx        = e.dataTransfer.getData('machine_idx');
            const targetProcessIdx  = parseInt(dropZone.dataset.processIdx || 0);
            const stdMcNeeded       = parseInt(dropZone.dataset.stdMcNeeded || 999);
            const draggedElement    = document.querySelector(`.machine-box[data-machine-idx="${machineIdx}"]`);

            if (!draggedElement) return;

            // targetProcessIdx !== 0: 실제 공정 드롭 존 → 용량 초과 검사
            if (targetProcessIdx !== 0) {
                // 라인 기준으로 현재 배정 수 계산 (전체 machines 기준이 아님)
                const filteredMachines = getFilteredMachines();
                const currentCountInLine = filteredMachines.filter(m =>
                    parseInt(currentAssignments[m.idx]) === targetProcessIdx
                ).length;
                const isAlreadyAssigned = parseInt(currentAssignments[machineIdx]) === targetProcessIdx;
                if (!isAlreadyAssigned && currentCountInLine >= stdMcNeeded) {
                    alert(`The maximum number of Machine allocations for this Process in the current line is ${stdMcNeeded}.`);
                    return;
                }
            }

            // 기계 박스를 대상 드롭 존으로 이동 + currentAssignments 업데이트
            dropZone.appendChild(draggedElement);
            currentAssignments[machineIdx] = targetProcessIdx;
            updateAllMachineCounts();
            return false;
        }

        /**
         * updateAllMachineCounts — 모든 공정 행의 배정 카운터 DOM 갱신
         * 드롭 후 호출: 각 행의 현재 배정 수 / 여유 슬롯 / 'full' 클래스 업데이트
         */
        function updateAllMachineCounts() {
            document.querySelectorAll('.process-row').forEach(row => {
                const processIdx = parseInt(row.dataset.processIdx);
                const processBox = row.querySelector('.process-box:not(.empty)');
                if (!processBox) return;

                const stdMcNeeded         = parseInt(processBox.dataset.stdMcNeeded);
                const dropZone            = row.querySelector('.drop-zone');
                const currentMachineCount = dropZone.querySelectorAll('.machine-box').length;
                const availableSlots      = stdMcNeeded - currentMachineCount;
                const isFull              = currentMachineCount >= stdMcNeeded;

                const currentCountSpan  = processBox.querySelector('.machine-count-current');
                const availableCountSpan = processBox.querySelector('.machine-count-available');
                const capacityBadge     = processBox.querySelector('.process-capacity-badge');

                if (currentCountSpan)   currentCountSpan.textContent  = currentMachineCount;
                if (availableCountSpan) { availableCountSpan.textContent = availableSlots; availableCountSpan.classList.toggle('full', isFull); }
                if (capacityBadge)      capacityBadge.classList.toggle('full', isFull);
            });
        }

        /**
         * saveProcessInfo — 공정 이름/용량 인라인 수정 저장
         * @param {HTMLElement} processBox - 편집 중인 .process-box 요소
         * @param {number} processIdx - 공정 PK (idx)
         *
         * POST proc/process_machine_mapping.php:
         *   action=update_process, idx, design_process, std_mc_needed
         * 성공 시: .editing 제거 + 뷰 모드 DOM 업데이트 + mappingData 로컬 갱신 + updateAllMachineCounts()
         */
        async function saveProcessInfo(processBox, processIdx) {
            const inputs      = processBox.querySelectorAll('input');
            const newName     = inputs[0].value.trim();
            const newCapacity = parseInt(inputs[1].value);

            if (!newName)       { alert('Please enter a process name.'); return; }
            if (newCapacity < 1) { alert('Capacity must be at least 1.'); return; }

            try {
                const formData = new FormData();
                formData.append('action',       'update_process');
                formData.append('idx',          processIdx);
                formData.append('design_process', newName);
                formData.append('std_mc_needed', newCapacity);

                const response = await fetch('proc/process_machine_mapping.php', { method: 'POST', body: formData });
                const result   = await response.json();

                if (result.success) {
                    processBox.classList.remove('editing');
                    // 뷰 모드 DOM 즉시 업데이트
                    processBox.querySelector('.process-name').textContent         = newName;
                    processBox.querySelector('.process-capacity-badge').textContent = newCapacity;
                    processBox.dataset.stdMcNeeded = newCapacity;
                    const dropZone = processBox.parentElement.querySelector('.drop-zone');
                    if (dropZone) dropZone.dataset.stdMcNeeded = newCapacity;
                    // mappingData 로컬 갱신 (다음 renderMappingUI 호출 시 반영)
                    const process = mappingData.processes.find(p => p.idx === processIdx);
                    if (process) { process.design_process = newName; process.std_mc_needed = newCapacity; }
                    updateAllMachineCounts();
                } else {
                    alert('Save failed: ' + result.message);
                }
            } catch (error) {
                alert('An error occurred while saving.');
            }
        }

        /**
         * saveMachineAssignments — 현재 UI의 기계-공정 배정 현황 전체 저장
         * currentAssignments 객체를 [{machine_idx, design_process_idx}] 배열로 변환
         * POST proc/process_machine_mapping.php:
         *   action=update_machine_assignments, assignments(JSON 문자열)
         * 성공 시: 저장 수량 알림 + resourceManager.loadData()로 테이블 갱신
         */
        async function saveMachineAssignments() {
            try {
                const assignments = Object.keys(currentAssignments).map(machineIdx => ({
                    machine_idx:        parseInt(machineIdx),
                    design_process_idx: parseInt(currentAssignments[machineIdx])
                }));

                const formData = new FormData();
                formData.append('action',      'update_machine_assignments');
                formData.append('assignments', JSON.stringify(assignments));

                const response = await fetch('proc/process_machine_mapping.php', { method: 'POST', body: formData });
                const result   = await response.json();

                if (result.success) {
                    alert(`Machine assignments saved. (${result.updated_count} items)`);
                    // 테이블 데이터 갱신 (배정 변경 사항 반영)
                    if (resourceManager && resourceManager.loadData) await resourceManager.loadData();
                } else {
                    alert('Save failed: ' + result.message);
                }
            } catch (error) {
                alert('An error occurred while saving.');
            }
        }

        /**
         * escapeHtml — XSS 방지용 HTML 이스케이프
         * @param {string} text - 이스케이프할 텍스트
         * div.textContent 방식: 브라우저 내장 이스케이프 기능 활용
         * innerHTML로 삽입되는 모든 사용자 데이터에 적용
         */
        function escapeHtml(text) {
            const div = document.createElement('div');
            div.textContent = text;
            return div.innerHTML;
        }
    }
</script>


</body>
</html>
