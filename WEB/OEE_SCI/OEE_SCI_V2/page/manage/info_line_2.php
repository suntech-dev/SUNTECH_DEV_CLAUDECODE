<?php

/**
 * info_line_2.php — 라인(Line) 관리 페이지
 *
 * 라인 마스터 데이터 CRUD 페이지 (SAP Fiori 스타일)
 *
 * 사용 모듈:
 *   - resource-manager.js : CRUD, 페이지네이션, 정렬, 필터 통합 관리
 *   - info_line_2.js      : lineConfig + 검색/필터/모달 스텝 기능
 *     - beforeInit: loadFactoryOptions()로 factoryFilterSelect + factory_idx 드롭다운 초기화
 *
 * 모달 구조 (3단계):
 *   Step 1: factory_idx(공장, 필수) + line_name(라인명, 필수) + status
 *   Step 2: mp(Man Power) + line_target(일일 목표)
 *   Step 3: remark + 확인 미리보기 (previewName/previewLine/previewMP/previewTarget/previewStatus)
 *
 * 필터:
 *   - factoryFilterSelect: 공장별 필터 (JS로 채워짐)
 *   - statusFilterSelect: 사용 여부 필터
 */
$page_title = 'Line Management';
$page_css_files = [
    '../../assets/css/fiori-page.css',
    'css/info_line_2.css',
];

require_once(__DIR__ . '/../../inc/head.php');
/* nav-fiori.php 제거 */
?>

<?php
// 네비게이션 드로어: line 메뉴 항목을 active 상태로 표시
$nav_active = 'line';
require_once(__DIR__ . '/../../inc/nav-drawer-manage.php');
?>

<!-- Signage Header: 상단 네비게이션 바 -->
<div class="signage-header">
    <button id="navDrawerBtn" class="nav-drawer-btn" aria-label="Menu">&#9776;</button>
    <span class="signage-header__title">Line Management</span>

    <div class="signage-header__filters">
        <!-- 실시간 검색 입력창 -->
        <div class="search-container">
            <input type="text" id="realTimeSearch" class="fiori-input search-input" placeholder="Search line...">
            <button class="search-clear" id="searchClear" title="Clear search">&#10005;</button>
        </div>
        <!-- 공장 필터 드롭다운: loadFactoryOptions()에서 동적으로 채워짐 (JS로 채워짐) -->
        <select id="factoryFilterSelect" class="fiori-select">
            <!-- JS로 채워짐 -->
        </select>
        <!-- 사용 여부 필터 -->
        <select id="statusFilterSelect" class="fiori-select">
            <option value="">All Status</option>
            <option value="Y">Used Only</option>
            <option value="N">Unused Only</option>
        </select>
        <!-- 컬럼 표시/숨김 토글 -->
        <div class="fiori-dropdown">
            <button id="columnToggleBtn" class="fiori-btn fiori-btn--secondary">Columns</button>
            <div id="columnToggleDropdown" class="fiori-dropdown__content"></div>
        </div>
        <button id="addBtn" class="fiori-btn fiori-btn--primary">Add Line</button>
        <button id="refreshBtn" class="fiori-btn fiori-btn--tertiary">Refresh</button>
    </div>
</div>

<!-- Main: 테이블 카드 영역 -->
<div class="manage-main">

    <div class="fiori-card">
        <div class="fiori-card__header">
            <div>
                <h3 class="fiori-card__title">Line List</h3>
            </div>
            <!-- 빠른 필터 버튼: statusFilterSelect → change 이벤트 → ResourceManager 재로드 -->
            <div class="quick-actions">
                <button class="quick-action-btn active" data-filter="all">All</button>
                <button class="quick-action-btn" data-filter="used">Used</button>
                <button class="quick-action-btn" data-filter="unused">Unused</button>
            </div>
        </div>

        <div class="fiori-card__content fiori-p-0">
            <div style="overflow-x: auto; height: 100%;">
                <table class="fiori-table">
                    <thead id="tableHeader" class="fiori-table__header"></thead>
                    <tbody id="tableBody"></tbody>
                </table>
            </div>
            <div id="noDataMessage" class="text-center" style="padding: var(--sap-spacing-xl); color: var(--sap-text-secondary); display: none;">
                <p>No matching lines found.</p>
            </div>
        </div>
    </div>

    <div id="pagination-controls" class="fiori-pagination"></div>

</div><!-- /manage-main -->


<!-- Line Modal: 라인 등록/수정 다이얼로그 (3단계) -->
<div id="resourceModal" class="fiori-modal">
    <div class="fiori-modal__content">
        <div class="fiori-card">
            <div class="fiori-card__header">
                <div>
                    <h3 class="fiori-card__title" id="modalTitle">Line Information</h3>
                    <p class="fiori-card__subtitle">Enter and edit line information</p>
                </div>
                <button type="button" class="fiori-btn fiori-btn--tertiary fiori-btn--sm close"
                    style="position: absolute; top: var(--sap-spacing-md); right: var(--sap-spacing-md);">
                    &#10005;
                </button>
            </div>
            <div class="fiori-card__content">
                <form id="resourceForm" class="fiori-form">
                    <input type="hidden" id="resourceId" name="idx">

                    <!-- 3단계 스텝 인디케이터 -->
                    <div style="display: flex; justify-content: space-between; align-items: center; margin-bottom: var(--sap-spacing-lg); padding-bottom: var(--sap-spacing-md); border-bottom: 1px solid var(--sap-border-subtle);">
                        <div style="display: flex; gap: var(--sap-spacing-sm); align-items: center;">
                            <div class="form-step active" data-step="1">1</div>
                            <div class="form-step-line"></div>
                            <div class="form-step" data-step="2">2</div>
                            <div class="form-step-line"></div>
                            <div class="form-step" data-step="3">3</div>
                        </div>
                        <div style="font-size: var(--sap-font-size-sm); color: var(--sap-text-secondary);">
                            <span id="stepIndicator">Step 1 of 3</span>
                        </div>
                    </div>

                    <!-- Step 1: 기본 정보 (공장 선택 + 라인명 + 상태) -->
                    <!-- validateCurrentStep Step1: factory_idx + line_name 필수 + 중복 확인 API -->
                    <div class="form-section" data-section="1">
                        <h4 style="margin-bottom: var(--sap-spacing-md); color: var(--sap-text-primary);">Basic information</h4>

                        <div class="fiori-form__group">
                            <label for="factory_idx" class="fiori-form__label fiori-form__label--required">Factory Name</label>
                            <!-- loadFactoryOptions()에서 공장 목록으로 채워짐 -->
                            <select id="factory_idx" name="factory_idx" class="fiori-select" required>
                                <!-- JS로 채워짐 -->
                            </select>
                            <div class="fiori-form__help">Select the factory where this line is located</div>
                        </div>

                        <div class="fiori-form__group">
                            <label for="line_name" class="fiori-form__label fiori-form__label--required">Line Name</label>
                            <!-- 중복 확인: 같은 공장 내 동일 라인명 체크 (factory_idx + line_name 조합) -->
                            <input type="text" id="line_name" name="line_name" class="fiori-input" required
                                placeholder="Please enter the line name">
                            <div class="fiori-form__help">Please enter a unique line name (2-50 characters)</div>
                        </div>

                        <div class="fiori-form__group">
                            <label for="status" class="fiori-form__label">Status</label>
                            <select id="status" name="status" class="fiori-select">
                                <option value="Y">Used</option>
                                <option value="N">Unused</option>
                            </select>
                            <div class="fiori-form__help">Please select the current usage status of the line.</div>
                        </div>
                    </div>

                    <!-- Step 2: 수치 정보 (Man Power + 일일 목표 생산량) -->
                    <!-- validateCurrentStep Step2: mp/line_target >= 0 검증 -->
                    <div class="form-section" data-section="2" style="display: none;">
                        <h4 style="margin-bottom: var(--sap-spacing-md); color: var(--sap-text-primary);">Additional information</h4>

                        <div class="fiori-form__group">
                            <label for="mp" class="fiori-form__label fiori-form__label--required">Man Power</label>
                            <!-- 0 이상 정수, 음수 입력 시 검증 실패 -->
                            <input type="number" id="mp" name="mp" class="fiori-input" required value="0" min="0">
                            <div class="fiori-form__help">Number of workers assigned to this line</div>
                        </div>

                        <div class="fiori-form__group">
                            <label for="line_target" class="fiori-form__label fiori-form__label--required">Line Target</label>
                            <input type="number" id="line_target" name="line_target" class="fiori-input" required value="0" min="0">
                            <div class="fiori-form__help">Daily production target for this line</div>
                        </div>
                    </div>

                    <!-- Step 3: 비고 + 확인 미리보기 -->
                    <div class="form-section" data-section="3" style="display: none;">
                        <h4 style="margin-bottom: var(--sap-spacing-md); color: var(--sap-text-primary);">Remark Information</h4>

                        <div class="fiori-form__group">
                            <label for="remark" class="fiori-form__label">Remark</label>
                            <textarea id="remark" name="remark" class="fiori-input" rows="3"></textarea>
                            <div class="fiori-form__help">You can record line information.</div>
                        </div>

                        <!-- 확인 미리보기: updatePreview()에서 Step1/2 입력값 반영 -->
                        <div style="padding: var(--sap-spacing-md); background: var(--sap-surface-2); border-radius: var(--sap-radius-md); margin-top: var(--sap-spacing-md);">
                            <h5 style="margin: 0 0 var(--sap-spacing-sm) 0; color: var(--sap-text-primary);">Confirm Information</h5>
                            <div style="font-size: var(--sap-font-size-sm); color: var(--sap-text-secondary);">
                                <!-- previewName: factory_idx 선택 텍스트 -->
                                <div><strong>Factory name:</strong> <span id="previewName">-</span></div>
                                <!-- previewLine: line_name 입력값 -->
                                <div><strong>Line Name:</strong> <span id="previewLine">-</span></div>
                                <!-- previewMP: mp 입력값 -->
                                <div><strong>Man Power:</strong> <span id="previewMP">-</span></div>
                                <!-- previewTarget: line_target 입력값 -->
                                <div><strong>Line Target:</strong> <span id="previewTarget">-</span></div>
                                <!-- previewStatus: status 선택 텍스트 -->
                                <div><strong>Status:</strong> <span id="previewStatus">-</span></div>
                            </div>
                        </div>
                    </div>

                    <div class="fiori-form__actions">
                        <button type="button" class="fiori-btn fiori-btn--tertiary" id="modalCloseBtn">Cancel</button>
                        <button type="button" class="fiori-btn fiori-btn--secondary" id="prevStep" style="display: none;">Previous</button>
                        <button type="button" class="fiori-btn fiori-btn--primary" id="nextStep">Next</button>
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
     * lineConfig.beforeInit: createResourceManager 내부에서 ResourceManager 초기화 전에 호출
     *   → loadFactoryOptions(): 공장 목록 API 요청 후 factoryFilterSelect + factory_idx 드롭다운 채움
     *
     * initAdvancedFeatures: 검색/필터/3단계 모달 스텝 초기화
     */
    import {
        createResourceManager
    } from '../../assets/js/resource-manager.js';
    import {
        initAdvancedFeatures,
        lineConfig
    } from './js/info_line_2.js';

    document.addEventListener('DOMContentLoaded', function() {
        const resourceManager = createResourceManager(lineConfig);
        setTimeout(() => initAdvancedFeatures(resourceManager), 100);
    });
</script>


</body>

</html>