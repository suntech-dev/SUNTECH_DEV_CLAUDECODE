<?php
/**
 * info_machine_2.php — 기계(Machine) 관리 페이지
 *
 * 기계 마스터 데이터 CRUD 페이지 (SAP Fiori 스타일)
 *
 * 사용 모듈:
 *   - resource-manager.js : CRUD, 페이지네이션, 정렬, 필터 통합 관리
 *   - info_machine_2.js   : machineConfig + 검색/필터/모달 스텝 기능
 *     - beforeInit: 4개 드롭다운 로드 (공장/라인/기계/기계모델)
 *
 * 모달 구조 (3단계):
 *   Step 1: factory_idx(공장) + line_idx(라인) + machine_no(기계명) — 위치 정보
 *   Step 2: type(기계 타입) + machine_model_idx(모델) + status — 기계 설정
 *   Step 3: target(시간당 목표) + remark — 생산 설정
 *
 * 연쇄 드롭다운 필터:
 *   factoryFilterSelect → factoryLineFilterSelect → factoryLineMachineFilterSelect
 *   (resets 속성으로 상위 필터 변경 시 하위 필터 자동 초기화)
 *
 * 특이사항:
 *   - typeFilterSelect: 기본값 'P'(Computer Sewing) — beforeInit에서 100ms 지연 후 설정
 *   - excelDownloadBtn: PhpSpreadsheet Excel 내보내기 (현재 필터 조건 유지)
 *   - submitBtn 텍스트: 'Update Machine' (수정 위주 페이지)
 */
$page_title = 'Machine Management';
$page_css_files = [
    '../../assets/css/fiori-page.css',
    'css/info_machine_2.css',
];

require_once(__DIR__ . '/../../inc/head.php');
/* nav-fiori.php 제거 */
?>

<?php
// 네비게이션 드로어: machine 메뉴 항목을 active 상태로 표시
$nav_active = 'machine'; require_once(__DIR__ . '/../../inc/nav-drawer-manage.php');
?>

<!-- Signage Header: 상단 네비게이션 바 -->
<div class="signage-header">
    <button id="navDrawerBtn" class="nav-drawer-btn" aria-label="Menu">&#9776;</button>
    <span class="signage-header__title">Machine Management</span>

    <div class="signage-header__filters">
        <!-- 실시간 검색 + 초기화 버튼 -->
        <div class="search-container">
            <input type="text" id="realTimeSearch" class="fiori-input search-input" placeholder="Search machine...">
            <button class="search-clear" id="searchClear" title="Clear search">&#10005;</button>
        </div>
        <!-- 공장 필터: JS로 채워짐 (beforeInit에서 로드) -->
        <!-- resets: ['factoryLineFilter', 'factoryLineMachineFilter'] → 공장 변경 시 하위 필터 초기화 -->
        <select id="factoryFilterSelect" class="fiori-select">
            <!-- JS로 채워짐 -->
        </select>
        <!-- 라인 필터: 공장 선택 후 연쇄 갱신 (JS로 채워짐) -->
        <!-- resets: ['factoryLineMachineFilter'] → 라인 변경 시 기계 필터 초기화 -->
        <select id="factoryLineFilterSelect" class="fiori-select">
            <!-- JS로 채워짐 -->
        </select>
        <!-- 기계 필터: 공장+라인 선택 후 연쇄 갱신 (JS로 채워짐) -->
        <select id="factoryLineMachineFilterSelect" class="fiori-select machine-filter-select">
            <!-- JS로 채워짐 -->
        </select>
        <!-- 사용 여부 필터 -->
        <select id="statusFilterSelect" class="fiori-select">
            <option value="">All Status</option>
            <option value="Y">Used Only</option>
            <option value="N">Unused Only</option>
        </select>
        <!-- 기계 타입 필터: 기본값 'P'(Computer Sewing) — beforeInit에서 100ms 지연 후 설정 -->
        <select id="typeFilterSelect" class="fiori-select type-filter-select">
            <option value="" selected>All Types</option>
            <option value="P">Computer Sewing</option>
            <option value="E">Embroidery</option>
        </select>
        <!-- 컬럼 표시/숨김 토글 -->
        <div class="fiori-dropdown">
            <button id="columnToggleBtn" class="fiori-btn fiori-btn--secondary">Columns</button>
            <div id="columnToggleDropdown" class="fiori-dropdown__content"></div>
        </div>
        <!-- Excel 내보내기 버튼: 현재 필터 조건을 유지하여 PhpSpreadsheet로 다운로드 -->
        <button id="excelDownloadBtn" class="fiori-btn fiori-btn--secondary">Export</button>
        <button id="refreshBtn" class="fiori-btn fiori-btn--tertiary">Refresh</button>
    </div>
</div>

<!-- Main: 테이블 카드 영역 -->
<div class="manage-main">

    <div class="fiori-card">
        <div class="fiori-card__header">
            <div>
                <h3 class="fiori-card__title">Machine List</h3>
            </div>
            <!-- 빠른 필터: statusFilterSelect → change 이벤트 → ResourceManager 재로드 -->
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
                <p>No matching machines found.</p>
            </div>
        </div>
    </div>

    <div id="pagination-controls" class="fiori-pagination"></div>

</div><!-- /manage-main -->


<!-- Machine Modal: 기계 등록/수정 다이얼로그 (3단계) -->
<div id="resourceModal" class="fiori-modal">
    <div class="fiori-modal__content">
        <div class="fiori-card">
            <div class="fiori-card__header">
                <div>
                    <h3 class="fiori-card__title" id="modalTitle">Machine Information Edit</h3>
                    <p class="fiori-card__subtitle">Edit machine information in 3 steps</p>
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

                    <!-- Step 1: 위치 정보 (공장 → 라인 연쇄 선택 + 기계명) -->
                    <!-- validateCurrentStep Step1: factory_idx + line_idx + machine_no 필수 + 중복 확인 -->
                    <div class="form-section" data-section="1">
                        <h4 style="margin-bottom: var(--sap-spacing-md); color: var(--sap-text-primary);">Location Information</h4>

                        <div class="fiori-form__group">
                            <label for="factory_idx" class="fiori-form__label fiori-form__label--required">Factory</label>
                            <!-- beforeInit에서 공장 목록으로 채워짐 -->
                            <!-- 선택 시 updateLineOptions()로 line_idx 연쇄 갱신 -->
                            <select id="factory_idx" name="factory_idx" class="fiori-select" required>
                                <option value="">Please select a factory</option>
                            </select>
                            <div class="fiori-form__help">Select the factory first</div>
                        </div>

                        <div class="fiori-form__group">
                            <label for="line_idx" class="fiori-form__label fiori-form__label--required">Line</label>
                            <!-- 공장 선택 전: disabled=true, 공장 선택 후 활성화 -->
                            <!-- factory_idx 변경 이벤트에서 updateLineOptions()로 동적 갱신 -->
                            <select id="line_idx" name="line_idx" class="fiori-select" required disabled>
                                <option value="">Please select a line</option>
                            </select>
                            <div class="fiori-form__help">Select a line after choosing a factory</div>
                        </div>

                        <div class="fiori-form__group">
                            <label for="machine_no" class="fiori-form__label fiori-form__label--required">Machine Name</label>
                            <!-- 전체 고유 중복 확인 (공장/라인 범위 아님) -->
                            <input type="text" id="machine_no" name="machine_no" class="fiori-input" required
                                   placeholder="Enter machine name">
                            <div class="fiori-form__help">Enter the machine name (2-50 characters)</div>
                        </div>
                    </div>

                    <!-- Step 2: 기계 설정 (타입 + 모델 + 상태) -->
                    <!-- validateCurrentStep Step2: type + machine_model_idx 필수 -->
                    <div class="form-section" data-section="2" style="display: none;">
                        <h4 style="margin-bottom: var(--sap-spacing-md); color: var(--sap-text-primary);">Machine Settings</h4>

                        <div class="fiori-form__group">
                            <label for="type" class="fiori-form__label fiori-form__label--required">Machine Type</label>
                            <!-- 타입 선택 시 updateMachineModelOptions()로 machine_model_idx 연쇄 갱신 -->
                            <select id="type" name="type" class="fiori-select" required>
                                <option value="">Please select machine type</option>
                                <option value="P">Computer Sewing Machine</option>
                                <option value="E">Embroidery Machine</option>
                            </select>
                            <div class="fiori-form__help">Select the machine type</div>
                        </div>

                        <div class="fiori-form__group">
                            <label for="machine_model_idx" class="fiori-form__label fiori-form__label--required">Machine Model</label>
                            <!-- type 선택 후 해당 타입의 모델 목록으로 동적 갱신 -->
                            <select id="machine_model_idx" name="machine_model_idx" class="fiori-select" required>
                                <option value="">Please select machine model</option>
                            </select>
                            <div class="fiori-form__help">Select the machine model</div>
                        </div>

                        <div class="fiori-form__group">
                            <label for="status" class="fiori-form__label">Status</label>
                            <select id="status" name="status" class="fiori-select">
                                <option value="Y">Used</option>
                                <option value="N">Unused</option>
                            </select>
                            <div class="fiori-form__help">Select the machine usage status</div>
                        </div>
                    </div>

                    <!-- Step 3: 생산 설정 (시간당 목표 + 비고) -->
                    <div class="form-section" data-section="3" style="display: none;">
                        <h4 style="margin-bottom: var(--sap-spacing-md); color: var(--sap-text-primary);">Production Settings</h4>

                        <div class="fiori-form__group">
                            <label for="target" class="fiori-form__label">Target Quantity (per hour)</label>
                            <input type="number" id="target" name="target" class="fiori-input" min="0" step="1"
                                   placeholder="Enter target quantity per hour">
                            <div class="fiori-form__help">Enter the hourly production target</div>
                        </div>

                        <div class="fiori-form__group">
                            <label for="remark" class="fiori-form__label">Remark</label>
                            <textarea id="remark" name="remark" class="fiori-input" rows="3"
                                      placeholder="Enter any notes about this machine"></textarea>
                            <div class="fiori-form__help">Additional information or notes about the machine</div>
                        </div>
                    </div>

                    <div class="fiori-form__actions">
                        <button type="button" class="fiori-btn fiori-btn--tertiary" id="modalCloseBtn">Cancel</button>
                        <button type="button" class="fiori-btn fiori-btn--secondary" id="prevStep" style="display: none;">Previous</button>
                        <button type="button" class="fiori-btn fiori-btn--primary" id="nextStep">Next</button>
                        <!-- submitBtn 텍스트: 'Update Machine' (수정 위주 작업) -->
                        <button type="submit" class="fiori-btn fiori-btn--primary" id="submitBtn" style="display: none;">Update Machine</button>
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
     * machineConfig.beforeInit: 4개 드롭다운 비동기 로드
     *   - 공장/라인/기계 필터 드롭다운 (목록 페이지용)
     *   - 공장/라인 모달 드롭다운 (등록/수정 폼용)
     *   - typeFilterSelect 기본값 'P' 설정 (100ms 지연)
     *
     * initAdvancedFeatures: 검색/필터/3단계 모달 스텝 초기화
     */
    import { createResourceManager } from '../../assets/js/resource-manager.js';
    import { initAdvancedFeatures, machineConfig } from './js/info_machine_2.js';

    document.addEventListener('DOMContentLoaded', function() {
        const resourceManager = createResourceManager(machineConfig);
        setTimeout(() => initAdvancedFeatures(resourceManager), 100);
    });
</script>


</body>
</html>
