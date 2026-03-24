<?php
$page_title = 'Machine Management';
$page_css_files = [
    '../../assets/css/fiori-page.css',
    'css/info_machine_2.css',
];

require_once(__DIR__ . '/../../inc/head.php');
/* nav-fiori.php 제거 */
?>

<?php $nav_active = 'machine'; require_once(__DIR__ . '/../../inc/nav-drawer-manage.php'); ?>

<!-- Signage Header -->
<div class="signage-header">
    <button id="navDrawerBtn" class="nav-drawer-btn" aria-label="Menu">&#9776;</button>
    <span class="signage-header__title">Machine Management</span>

    <div class="signage-header__filters">
        <div class="search-container">
            <input type="text" id="realTimeSearch" class="fiori-input search-input" placeholder="Search machine...">
            <button class="search-clear" id="searchClear" title="Clear search">&#10005;</button>
        </div>
        <select id="factoryFilterSelect" class="fiori-select">
            <!-- JS로 채워짐 -->
        </select>
        <select id="factoryLineFilterSelect" class="fiori-select">
            <!-- JS로 채워짐 -->
        </select>
        <select id="factoryLineMachineFilterSelect" class="fiori-select machine-filter-select">
            <!-- JS로 채워짐 -->
        </select>
        <select id="statusFilterSelect" class="fiori-select">
            <option value="">All Status</option>
            <option value="Y">Used Only</option>
            <option value="N">Unused Only</option>
        </select>
        <select id="typeFilterSelect" class="fiori-select type-filter-select">
            <option value="">All Types</option>
            <option value="P" selected>Computer Sewing</option>
            <option value="E">Embroidery</option>
        </select>
        <div class="fiori-dropdown">
            <button id="columnToggleBtn" class="fiori-btn fiori-btn--secondary">Columns</button>
            <div id="columnToggleDropdown" class="fiori-dropdown__content"></div>
        </div>
        <button id="excelDownloadBtn" class="fiori-btn fiori-btn--secondary">Export</button>
        <button id="refreshBtn" class="fiori-btn fiori-btn--tertiary">Refresh</button>
    </div>
</div>

<!-- Main -->
<div class="manage-main">

    <div class="fiori-card">
        <div class="fiori-card__header">
            <div>
                <h3 class="fiori-card__title">Machine List</h3>
            </div>
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


<!-- Machine Modal -->
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

                    <!-- Step indicator -->
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

                    <!-- Step 1: Location Information -->
                    <div class="form-section" data-section="1">
                        <h4 style="margin-bottom: var(--sap-spacing-md); color: var(--sap-text-primary);">Location Information</h4>

                        <div class="fiori-form__group">
                            <label for="factory_idx" class="fiori-form__label fiori-form__label--required">Factory</label>
                            <select id="factory_idx" name="factory_idx" class="fiori-select" required>
                                <option value="">Please select a factory</option>
                            </select>
                            <div class="fiori-form__help">Select the factory first</div>
                        </div>

                        <div class="fiori-form__group">
                            <label for="line_idx" class="fiori-form__label fiori-form__label--required">Line</label>
                            <select id="line_idx" name="line_idx" class="fiori-select" required disabled>
                                <option value="">Please select a line</option>
                            </select>
                            <div class="fiori-form__help">Select a line after choosing a factory</div>
                        </div>

                        <div class="fiori-form__group">
                            <label for="machine_no" class="fiori-form__label fiori-form__label--required">Machine Name</label>
                            <input type="text" id="machine_no" name="machine_no" class="fiori-input" required
                                   placeholder="Enter machine name">
                            <div class="fiori-form__help">Enter the machine name (2-50 characters)</div>
                        </div>
                    </div>

                    <!-- Step 2: Machine Settings -->
                    <div class="form-section" data-section="2" style="display: none;">
                        <h4 style="margin-bottom: var(--sap-spacing-md); color: var(--sap-text-primary);">Machine Settings</h4>

                        <div class="fiori-form__group">
                            <label for="type" class="fiori-form__label fiori-form__label--required">Machine Type</label>
                            <select id="type" name="type" class="fiori-select" required>
                                <option value="">Please select machine type</option>
                                <option value="P">Computer Sewing Machine</option>
                                <option value="E">Embroidery Machine</option>
                            </select>
                            <div class="fiori-form__help">Select the machine type</div>
                        </div>

                        <div class="fiori-form__group">
                            <label for="machine_model_idx" class="fiori-form__label fiori-form__label--required">Machine Model</label>
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

                    <!-- Step 3: Production Settings -->
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
                        <button type="submit" class="fiori-btn fiori-btn--primary" id="submitBtn" style="display: none;">Update Machine</button>
                    </div>
                </form>
            </div>
        </div>
    </div>
</div>


<script type="module">
    import { createResourceManager } from '../../assets/js/resource-manager.js';
    import { initAdvancedFeatures, machineConfig } from './js/info_machine_2.js';

    document.addEventListener('DOMContentLoaded', function() {
        const resourceManager = createResourceManager(machineConfig);
        setTimeout(() => initAdvancedFeatures(resourceManager), 100);
    });
</script>


</body>
</html>
