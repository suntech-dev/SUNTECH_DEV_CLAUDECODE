<?php

/**
 * info_machine_model_2.php — 기계 모델(Machine Model) 관리 페이지
 *
 * 기계 모델 마스터 데이터 CRUD 페이지 (SAP Fiori 스타일)
 *
 * 사용 모듈:
 *   - resource-manager.js       : CRUD, 페이지네이션, 정렬, 필터 통합 관리
 *   - info_machine_model_2.js   : machineModelConfig + 검색/필터/모달 스텝 기능
 *
 * 모달 구조 (2단계):
 *   Step 1: machine_model_name(필수 + 중복확인) + type(P/E 선택) + status
 *   Step 2: remark + 확인 미리보기 (previewName, previewStatus)
 *
 * 기계 타입:
 *   P = Computer Sewing Machine (컴퓨터 재봉기)
 *   E = Embroidery Machine (자수기)
 */
$page_title = 'Machine Model Management';
$page_css_files = [
    '../../assets/css/fiori-page.css',
    'css/info_machine_model_2.css',
];

require_once(__DIR__ . '/../../inc/head.php');
/* nav-fiori.php 제거 */
?>

<?php
// 네비게이션 드로어: machine_model 메뉴 항목을 active 상태로 표시
$nav_active = 'machine_model';
require_once(__DIR__ . '/../../inc/nav-drawer-manage.php');
?>

<!-- Signage Header: 상단 네비게이션 바 -->
<div class="signage-header">
    <button id="navDrawerBtn" class="nav-drawer-btn" aria-label="Menu">&#9776;</button>
    <span class="signage-header__title">Machine Model Management</span>

    <div class="signage-header__filters">
        <!-- 실시간 검색 + 초기화 버튼 -->
        <div class="search-container">
            <input type="text" id="realTimeSearch" class="fiori-input search-input" placeholder="Search machine model...">
            <button class="search-clear" id="searchClear" title="Clear search">&#10005;</button>
        </div>
        <!-- 사용 여부 필터: 기본값 'Y'(Used Only) -->
        <select id="statusFilterSelect" class="fiori-select">
            <option value="">All Status</option>
            <option value="Y" selected>Used Only</option>
            <option value="N">Unused Only</option>
        </select>
        <!-- 컬럼 표시/숨김 토글 -->
        <div class="fiori-dropdown">
            <button id="columnToggleBtn" class="fiori-btn fiori-btn--secondary">Columns</button>
            <div id="columnToggleDropdown" class="fiori-dropdown__content"></div>
        </div>
        <button id="addBtn" class="fiori-btn fiori-btn--primary">Add Machine Model</button>
        <button id="refreshBtn" class="fiori-btn fiori-btn--tertiary">Refresh</button>
    </div>
</div>

<!-- Main: 테이블 카드 영역 -->
<div class="manage-main">

    <div class="fiori-card">
        <div class="fiori-card__header">
            <div>
                <h3 class="fiori-card__title">Machine Model List</h3>
            </div>
            <!-- 빠른 필터 버튼 -->
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
                <p>No matching machine models found.</p>
            </div>
        </div>
    </div>

    <div id="pagination-controls" class="fiori-pagination"></div>

</div><!-- /manage-main -->


<!-- Machine Model Modal: 기계 모델 등록/수정 다이얼로그 (2단계) -->
<div id="resourceModal" class="fiori-modal">
    <div class="fiori-modal__content">
        <div class="fiori-card">
            <div class="fiori-card__header">
                <div>
                    <h3 class="fiori-card__title" id="modalTitle">Machine Model Information</h3>
                    <p class="fiori-card__subtitle">Enter and edit machine model information</p>
                </div>
                <button type="button" class="fiori-btn fiori-btn--tertiary fiori-btn--sm close"
                    style="position: absolute; top: var(--sap-spacing-md); right: var(--sap-spacing-md);">
                    &#10005;
                </button>
            </div>
            <div class="fiori-card__content">
                <form id="resourceForm" class="fiori-form">
                    <input type="hidden" id="resourceId" name="idx">

                    <!-- 2단계 스텝 인디케이터 -->
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

                    <!-- Step 1: 기본 정보 (모델명 + 타입 + 상태) -->
                    <!-- validateCurrentStep: machine_model_name 중복 확인 API 호출 -->
                    <div class="form-section" data-section="1">
                        <h4 style="margin-bottom: var(--sap-spacing-md); color: var(--sap-text-primary);">Basic information</h4>

                        <div class="fiori-form__group">
                            <label for="machine_model_name" class="fiori-form__label fiori-form__label--required">Machine Model Name</label>
                            <input type="text" id="machine_model_name" name="machine_model_name" class="fiori-input" required
                                placeholder="Please enter the machine model name">
                            <div class="fiori-form__help">Please enter a unique machine model name (2-50 characters)</div>
                        </div>

                        <div class="fiori-form__group">
                            <label for="type" class="fiori-form__label fiori-form__label--required">Machine Type</label>
                            <!-- P: Computer Sewing Machine (컬럼에서 파란색으로 표시) -->
                            <!-- E: Embroidery Machine (컬럼에서 주황색으로 표시) -->
                            <select id="type" name="type" class="fiori-select" required>
                                <option value="P">Computer Sewing Machine</option>
                                <option value="E">Embroidery Machine</option>
                            </select>
                            <div class="fiori-form__help">Please select the machine type</div>
                        </div>

                        <div class="fiori-form__group">
                            <label for="status" class="fiori-form__label">Status</label>
                            <select id="status" name="status" class="fiori-select">
                                <option value="Y">Used</option>
                                <option value="N">Unused</option>
                            </select>
                            <div class="fiori-form__help">Please select the current usage status of the machine model.</div>
                        </div>
                    </div>

                    <!-- Step 2: 추가 정보 + 확인 미리보기 -->
                    <div class="form-section" data-section="2" style="display: none;">
                        <h4 style="margin-bottom: var(--sap-spacing-md); color: var(--sap-text-primary);">Additional information</h4>

                        <div class="fiori-form__group">
                            <label for="remark" class="fiori-form__label">Remark</label>
                            <textarea id="remark" name="remark" class="fiori-input" rows="3"
                                placeholder="Additional information about machine model"></textarea>
                            <div class="fiori-form__help">You can record machine model information.</div>
                        </div>

                        <!-- 확인 미리보기: updatePreview()에서 machine_model_name + status 반영 -->
                        <div style="padding: var(--sap-spacing-md); background: var(--sap-surface-2); border-radius: var(--sap-radius-md); margin-top: var(--sap-spacing-md);">
                            <h5 style="margin: 0 0 var(--sap-spacing-sm) 0; color: var(--sap-text-primary);">Confirm Information</h5>
                            <div style="font-size: var(--sap-font-size-sm); color: var(--sap-text-secondary);">
                                <div><strong>Machine Model name:</strong> <span id="previewName">-</span></div>
                                <div><strong>Status:</strong> <span id="previewStatus">-</span></div>
                            </div>
                        </div>
                    </div>

                    <!-- 폼 액션 버튼 -->
                    <!-- 주의: info_machine_model_2.js의 initModalSteps()는 prevBtn을 'prevBtn' ID로 조회하지만
               HTML에는 'prevStep' ID로 정의되어 있음 → showStep() 내에서 'prevStep'으로 재조회 -->
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
     * machineModelConfig → createResourceManager → initAdvancedFeatures 순서로 초기화
     */
    import {
        createResourceManager
    } from '../../assets/js/resource-manager.js';
    import {
        initAdvancedFeatures,
        machineModelConfig
    } from './js/info_machine_model_2.js';

    document.addEventListener('DOMContentLoaded', function() {
        const resourceManager = createResourceManager(machineModelConfig);
        setTimeout(() => initAdvancedFeatures(resourceManager), 100);
    });
</script>


</body>

</html>