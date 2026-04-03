<?php
/**
 * info_defective_2.php — 불량(Defective) 관리 페이지
 *
 * 불량 마스터 데이터 CRUD 페이지 (SAP Fiori 스타일)
 *
 * 사용 모듈:
 *   - resource-manager.js  : CRUD, 페이지네이션, 정렬, 필터 통합 관리
 *   - info_defective_2.js  : defectiveConfig(컬럼/필터 설정) + 검색/필터/모달 스텝 기능
 *
 * 모달 구조 (2단계):
 *   Step 1: defective_name(필수 + 중복확인) + defective_shortcut(선택, maxlength=10)
 *   Step 2: status + remark(비고) + 확인 미리보기 (previewName, previewShortcut, previewStatus)
 *
 * CSS 로드 순서:
 *   1. fiori-page.css      : 공통 SAP Fiori 레이아웃/컴포넌트
 *   2. info_defective_2.css: 불량 관리 페이지 전용 스타일
 *
 * 특이사항:
 *   - defective_shortcut: 선택 입력 (optional) — 입력 시에만 중복 확인 API 2차 호출
 *   - status 선택은 Step2에 위치 (info_andon_2.php는 Step1에 위치)
 */
$page_title = 'Defective Management';
$page_css_files = [
    '../../assets/css/fiori-page.css',
    'css/info_defective_2.css',
];

require_once(__DIR__ . '/../../inc/head.php');
/* nav-fiori.php 제거 */
?>

<?php
// 네비게이션 드로어: defective 메뉴 항목을 active 상태로 표시
$nav_active = 'defective'; require_once(__DIR__ . '/../../inc/nav-drawer-manage.php');
?>

<!-- Signage Header: 상단 네비게이션 바 -->
<div class="signage-header">
    <!-- 햄버거 버튼: 좌측 네비게이션 드로어 열기/닫기 -->
    <button id="navDrawerBtn" class="nav-drawer-btn" aria-label="Menu">&#9776;</button>
    <span class="signage-header__title">Defective Management</span>

    <!-- 헤더 필터 영역: 검색, 상태 필터, 컬럼 토글, 추가/새로고침 버튼 -->
    <div class="signage-header__filters">
        <!-- 실시간 검색 입력창 + 초기화(X) 버튼 -->
        <!-- initRealTimeSearch()에서 input/keydown/searchClear 이벤트 등록 -->
        <div class="search-container">
            <input type="text" id="realTimeSearch" class="fiori-input search-input" placeholder="Search defective...">
            <button class="search-clear" id="searchClear" title="Clear search">&#10005;</button>
        </div>
        <!-- 사용 여부 필터: ResourceManager filterConfig에서 statusFilterSelect를 감지하여 데이터 재로드 -->
        <select id="statusFilterSelect" class="fiori-select">
            <option value="">All Status</option>
            <option value="Y">Used Only</option>
            <option value="N">Unused Only</option>
        </select>
        <!-- 컬럼 표시/숨김 토글 드롭다운 (ResourceManager에서 자동 생성) -->
        <div class="fiori-dropdown">
            <button id="columnToggleBtn" class="fiori-btn fiori-btn--secondary">Columns</button>
            <div id="columnToggleDropdown" class="fiori-dropdown__content"></div>
        </div>
        <!-- 신규 등록 버튼: addBtn ID → initModalSteps()에서 Step 1 리셋 트리거 -->
        <button id="addBtn" class="fiori-btn fiori-btn--primary">Add Defective</button>
        <!-- 새로고침 버튼: ResourceManager에서 자동으로 loadData() 연결 -->
        <button id="refreshBtn" class="fiori-btn fiori-btn--tertiary">Refresh</button>
    </div>
</div>

<!-- Main: 테이블 카드 영역 -->
<div class="manage-main">

    <div class="fiori-card">
        <div class="fiori-card__header">
            <div>
                <h3 class="fiori-card__title">Defective List</h3>
            </div>
            <!-- 빠른 필터 버튼: initQuickActions()에서 이벤트 등록 -->
            <!-- data-filter 속성값이 applyQuickFilter()의 인자로 전달됨 -->
            <div class="quick-actions">
                <button class="quick-action-btn active" data-filter="all">All</button>
                <button class="quick-action-btn" data-filter="used">Used</button>
                <button class="quick-action-btn" data-filter="unused">Unused</button>
            </div>
        </div>

        <div class="fiori-card__content fiori-p-0">
            <div style="overflow-x: auto; height: 100%;">
                <!-- ResourceManager가 tableHeader(thead)/tableBody(tbody)를 동적으로 채움 -->
                <table class="fiori-table">
                    <thead id="tableHeader" class="fiori-table__header"></thead>
                    <tbody id="tableBody"></tbody>
                </table>
            </div>
            <!-- 검색/필터 결과 없을 때 표시되는 안내 메시지 -->
            <div id="noDataMessage" class="text-center" style="padding: var(--sap-spacing-xl); color: var(--sap-text-secondary); display: none;">
                <p>No matching defectives found.</p>
            </div>
        </div>
    </div>

    <!-- 페이지네이션 컨트롤: ResourceManager가 동적으로 생성 -->
    <div id="pagination-controls" class="fiori-pagination"></div>

</div><!-- /manage-main -->


<!-- Defective Modal: 불량 등록/수정 다이얼로그 (2단계) -->
<!-- ResourceManager가 행 클릭(수정) 또는 addBtn 클릭(신규) 시 'show' 클래스를 추가하여 표시 -->
<div id="resourceModal" class="fiori-modal">
    <div class="fiori-modal__content">
        <div class="fiori-card">
            <div class="fiori-card__header">
                <div>
                    <!-- modalTitle: 신규 등록 시 'Add Defective', 수정 시 'Edit Defective' -->
                    <h3 class="fiori-card__title" id="modalTitle">Defective Information</h3>
                    <p class="fiori-card__subtitle">Enter and edit defective information</p>
                </div>
                <!-- 모달 닫기 버튼 (.close 클래스): ResourceManager에서 이벤트 처리 -->
                <button type="button" class="fiori-btn fiori-btn--tertiary fiori-btn--sm close"
                        style="position: absolute; top: var(--sap-spacing-md); right: var(--sap-spacing-md);">
                    &#10005;
                </button>
            </div>
            <div class="fiori-card__content">
                <form id="resourceForm" class="fiori-form">
                    <!-- 숨김 필드: 수정 시 대상 레코드의 PK 값 저장 (신규 시 비어있음) -->
                    <input type="hidden" id="resourceId" name="idx">

                    <!-- 2단계 스텝 인디케이터: .form-step 요소 + stepIndicator 텍스트 -->
                    <!-- data-step 속성으로 현재 스텝 이하 항목을 active 처리 -->
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

                    <!-- Step 1: 기본 정보 (불량명 + 단축 코드) -->
                    <!-- validateCurrentStep Step1: defective_name 필수 + 중복 확인 API 호출 -->
                    <!-- defective_shortcut 입력 시 2차 중복 확인 API 호출 -->
                    <div class="form-section" data-section="1">
                        <h4 style="margin-bottom: var(--sap-spacing-md); color: var(--sap-text-primary);">Basic information</h4>

                        <div class="fiori-form__group">
                            <label for="defective_name" class="fiori-form__label fiori-form__label--required">Defective Name</label>
                            <!-- validateCurrentStep에서 중복 확인 API 1차 호출 -->
                            <input type="text" id="defective_name" name="defective_name" class="fiori-input" required
                                   placeholder="Please enter the defective name">
                            <div class="fiori-form__help">Please enter a unique defective name (2-50 characters)</div>
                        </div>

                        <div class="fiori-form__group">
                            <label for="defective_shortcut" class="fiori-form__label">Shortcut Code</label>
                            <!-- 선택 입력: 값이 있을 때만 validateCurrentStep에서 2차 중복 확인 API 호출 -->
                            <!-- maxlength="10": 단축 코드 최대 10자 제한 -->
                            <input type="text" id="defective_shortcut" name="defective_shortcut" class="fiori-input"
                                   placeholder="Enter shortcut code (optional)" maxlength="10">
                            <div class="fiori-form__help">Optional shortcut code for quick defective selection (max 10 characters)</div>
                        </div>
                    </div>

                    <!-- Step 2: 추가 정보 (상태 + 비고 + 확인 미리보기) -->
                    <!-- status가 Step2에 위치 (info_andon_2.php와 다름) -->
                    <div class="form-section" data-section="2" style="display: none;">
                        <h4 style="margin-bottom: var(--sap-spacing-md); color: var(--sap-text-primary);">Additional information</h4>

                        <div class="fiori-form__group">
                            <label for="status" class="fiori-form__label">Status</label>
                            <!-- previewStatus: 미리보기에 선택된 option의 textContent가 표시됨 -->
                            <select id="status" name="status" class="fiori-select">
                                <option value="Y">Used</option>
                                <option value="N">Unused</option>
                            </select>
                            <div class="fiori-form__help">Please select the current usage status of the defective.</div>
                        </div>

                        <div class="fiori-form__group">
                            <label for="remark" class="fiori-form__label">Remark</label>
                            <textarea id="remark" name="remark" class="fiori-input" rows="3"></textarea>
                            <div class="fiori-form__help">You can record defective information.</div>
                        </div>

                        <!-- 확인 미리보기 패널: updatePreview()에서 Step1 입력값 반영 -->
                        <div style="padding: var(--sap-spacing-md); background: var(--sap-surface-2); border-radius: var(--sap-radius-md); margin-top: var(--sap-spacing-md);">
                            <h5 style="margin: 0 0 var(--sap-spacing-sm) 0; color: var(--sap-text-primary);">Confirm Information</h5>
                            <div style="font-size: var(--sap-font-size-sm); color: var(--sap-text-secondary);">
                                <!-- previewName: defective_name 입력값 반영 -->
                                <div><strong>Defective name:</strong> <span id="previewName">-</span></div>
                                <!-- previewShortcut: defective_shortcut 입력값 반영 (없으면 '-') -->
                                <div><strong>Shortcut code:</strong> <span id="previewShortcut">-</span></div>
                                <!-- previewStatus: status 선택 텍스트 반영 -->
                                <div><strong>Status:</strong> <span id="previewStatus">-</span></div>
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
                        <button type="submit" class="fiori-btn fiori-btn--primary" id="submitBtn" style="display: none;">Save</button>
                    </div>
                </form>
            </div>
        </div>
    </div>
</div>


<script type="module">
    /**
     * ES Module 진입점
     *
     * 초기화 순서:
     * 1. createResourceManager(defectiveConfig): CRUD/테이블/페이지네이션 초기화
     * 2. setTimeout 100ms 후 initAdvancedFeatures(resourceManager): 검색/필터/모달 기능 초기화
     *    (100ms 지연: ResourceManager DOM 렌더링 완료 보장)
     */
    import { createResourceManager } from '../../assets/js/resource-manager.js';
    import { initAdvancedFeatures, defectiveConfig } from './js/info_defective_2.js';

    document.addEventListener('DOMContentLoaded', function() {
        const resourceManager = createResourceManager(defectiveConfig);
        setTimeout(() => initAdvancedFeatures(resourceManager), 100);
    });
</script>


</body>
</html>
