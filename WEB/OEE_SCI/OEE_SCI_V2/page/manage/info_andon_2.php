<?php
/**
 * info_andon_2.php — 안돈(Andon) 관리 페이지
 *
 * 안돈 마스터 데이터 CRUD 페이지 (SAP Fiori 스타일)
 *
 * 사용 모듈:
 *   - resource-manager.js : CRUD, 페이지네이션, 정렬, 필터 통합 관리
 *   - info_andon_2.js     : andonConfig(컬럼/필터 설정) + 검색/필터/모달 스텝 기능
 *
 * 모달 구조 (2단계):
 *   Step 1: andon_name(필수 + 중복확인) + color(Spectrum 컬러피커) + status
 *   Step 2: remark(비고) + 확인 미리보기 (previewName, previewColor, previewStatus)
 *
 * CSS 로드 순서:
 *   1. fiori-page.css     : 공통 SAP Fiori 레이아웃/컴포넌트
 *   2. spectrum.css       : Spectrum 컬러피커 플러그인 스타일
 *   3. info_andon_2.css   : 안돈 관리 페이지 전용 스타일
 *
 * 스크립트 로드 순서 (모듈 외 의존성):
 *   1. jquery-3.6.1.min.js : jQuery (Spectrum 플러그인 의존성)
 *   2. spectrum.js         : Spectrum 컬러피커 플러그인
 *   3. ES Module           : resource-manager.js + info_andon_2.js (import)
 *
 * 특이사항:
 *   - Spectrum 컬러피커: jQuery 기반 플러그인 → ES Module 로드 전에 jQuery/spectrum.js 먼저 로드
 *   - color 입력: class="spectrum-colorpicker" → info_andon_2.js에서 $().spectrum() 초기화
 *   - colorPreviewBox: 선택된 색상을 실시간으로 미리보는 div 박스
 */
$page_title = 'Andon Management';
$page_css_files = [
    '../../assets/css/fiori-page.css',
    '../../assets/css/spectrum.css',
    'css/info_andon_2.css',
];

require_once(__DIR__ . '/../../inc/head.php');
/* nav-fiori.php 제거 */
?>

<?php
// 네비게이션 드로어: andon 메뉴 항목을 active 상태로 표시
$nav_active = 'andon'; require_once(__DIR__ . '/../../inc/nav-drawer-manage.php');
?>

<!-- Signage Header: 상단 네비게이션 바 -->
<div class="signage-header">
    <!-- 햄버거 버튼: 좌측 네비게이션 드로어 열기/닫기 -->
    <button id="navDrawerBtn" class="nav-drawer-btn" aria-label="Menu">&#9776;</button>
    <span class="signage-header__title">Andon Management</span>

    <!-- 헤더 필터 영역: 검색, 상태 필터, 컬럼 토글, 추가/새로고침 버튼 -->
    <div class="signage-header__filters">
        <!-- 실시간 검색 입력창 + 초기화(X) 버튼 -->
        <!-- initRealTimeSearch()에서 input/keydown/searchClear 이벤트 등록 -->
        <div class="search-container">
            <input type="text" id="realTimeSearch" class="fiori-input search-input" placeholder="Search andon...">
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
        <button id="addBtn" class="fiori-btn fiori-btn--primary">Add Andon</button>
        <!-- 새로고침 버튼: ResourceManager에서 자동으로 loadData() 연결 -->
        <button id="refreshBtn" class="fiori-btn fiori-btn--tertiary">Refresh</button>
    </div>
</div>

<!-- Main: 테이블 카드 영역 -->
<div class="manage-main">

    <div class="fiori-card">
        <div class="fiori-card__header">
            <div>
                <h3 class="fiori-card__title">Andon List</h3>
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
                <p>No matching andon found.</p>
            </div>
        </div>
    </div>

    <!-- 페이지네이션 컨트롤: ResourceManager가 동적으로 생성 -->
    <div id="pagination-controls" class="fiori-pagination"></div>

</div><!-- /manage-main -->


<!-- Andon Modal: 안돈 등록/수정 다이얼로그 (2단계) -->
<!-- ResourceManager가 행 클릭(수정) 또는 addBtn 클릭(신규) 시 'show' 클래스를 추가하여 표시 -->
<div id="resourceModal" class="fiori-modal">
    <div class="fiori-modal__content">
        <div class="fiori-card">
            <div class="fiori-card__header">
                <div>
                    <!-- modalTitle: 신규 등록 시 'Add Andon', 수정 시 'Edit Andon' -->
                    <h3 class="fiori-card__title" id="modalTitle">Andon Information</h3>
                    <p class="fiori-card__subtitle">Enter and edit andon information</p>
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

                    <!-- Step 1: 기본 정보 (안돈명 + 색상 + 상태) -->
                    <!-- validateCurrentStep Step1: andon_name 필수 + 중복 확인 API 호출 -->
                    <div class="form-section" data-section="1">
                        <h4 style="margin-bottom: var(--sap-spacing-md); color: var(--sap-text-primary);">Basic information</h4>

                        <div class="fiori-form__group">
                            <label for="andon_name" class="fiori-form__label fiori-form__label--required">Andon Name</label>
                            <!-- validateCurrentStep에서 중복 확인 API 호출 -->
                            <input type="text" id="andon_name" name="andon_name" class="fiori-input" required
                                   placeholder="Please enter the andon name">
                            <div class="fiori-form__help">Please enter a unique andon name (2-50 characters)</div>
                        </div>

                        <div class="fiori-form__group">
                            <label for="color" class="fiori-form__label">Color</label>
                            <div style="display: flex; align-items: center; gap: var(--sap-spacing-sm);">
                                <!-- class="spectrum-colorpicker": info_andon_2.js에서 $().spectrum()으로 초기화 -->
                                <!-- 컬러피커 선택 시 color 입력값 + colorPreviewBox 배경색 업데이트 -->
                                <input type="text" id="color" name="color" class="spectrum-colorpicker fiori-input"
                                       placeholder="Select color" maxlength="10" style="flex: 1;">
                                <!-- colorPreviewBox: 선택된 색상을 background-color로 시각화 -->
                                <div class="color-preview-box" id="colorPreviewBox"
                                     style="width: 40px; height: 32px; border: 1px solid var(--sap-border-neutral); border-radius: var(--sap-radius-sm); background: #ffffff;">
                                </div>
                            </div>
                            <div class="fiori-form__help">Click to select color using color picker</div>
                        </div>

                        <div class="fiori-form__group">
                            <label for="status" class="fiori-form__label">Status</label>
                            <!-- previewStatus: Step2 미리보기에 선택된 option의 textContent가 표시됨 -->
                            <select id="status" name="status" class="fiori-select">
                                <option value="Y">Used</option>
                                <option value="N">Unused</option>
                            </select>
                            <div class="fiori-form__help">Please select the current usage status of the andon.</div>
                        </div>
                    </div>

                    <!-- Step 2: 추가 정보 + 확인 미리보기 -->
                    <div class="form-section" data-section="2" style="display: none;">
                        <h4 style="margin-bottom: var(--sap-spacing-md); color: var(--sap-text-primary);">Additional information</h4>

                        <div class="fiori-form__group">
                            <label for="remark" class="fiori-form__label">Remark</label>
                            <textarea id="remark" name="remark" class="fiori-input" rows="3"></textarea>
                            <div class="fiori-form__help">You can record andon information.</div>
                        </div>

                        <!-- 확인 미리보기 패널: updatePreview()에서 실시간 갱신 -->
                        <div style="padding: var(--sap-spacing-md); background: var(--sap-surface-2); border-radius: var(--sap-radius-md); margin-top: var(--sap-spacing-md);">
                            <h5 style="margin: 0 0 var(--sap-spacing-sm) 0; color: var(--sap-text-primary);">Confirm Information</h5>
                            <div style="font-size: var(--sap-font-size-sm); color: var(--sap-text-secondary);">
                                <!-- previewName: andon_name 입력값 반영 -->
                                <div><strong>Andon name:</strong> <span id="previewName">-</span></div>
                                <!-- previewColor: color 입력값 반영 (색상 코드 텍스트) -->
                                <div><strong>Color:</strong> <span id="previewColor">-</span></div>
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


<!-- jQuery: Spectrum 컬러피커 플러그인의 의존성 -->
<!-- ES Module보다 먼저 로드해야 info_andon_2.js에서 $().spectrum() 사용 가능 -->
<script src="../../assets/js/jquery-3.6.1.min.js"></script>
<!-- Spectrum 컬러피커 플러그인: #color.spectrum-colorpicker 요소에 적용 -->
<script src="../../assets/js/spectrum.js"></script>

<script type="module">
    /**
     * ES Module 진입점
     *
     * 초기화 순서:
     * 1. createResourceManager(andonConfig): CRUD/테이블/페이지네이션 초기화
     * 2. setTimeout 100ms 후 initAdvancedFeatures(resourceManager): 검색/필터/모달 기능 초기화
     *    - Spectrum 컬러피커 초기화 포함 ($('#color').spectrum())
     *    - 100ms 지연: ResourceManager DOM 렌더링 완료 보장
     */
    import { createResourceManager } from '../../assets/js/resource-manager.js';
    import { initAdvancedFeatures, andonConfig } from './js/info_andon_2.js';

    document.addEventListener('DOMContentLoaded', function() {
        const resourceManager = createResourceManager(andonConfig);
        setTimeout(() => initAdvancedFeatures(resourceManager), 100);
    });
</script>


</body>
</html>
