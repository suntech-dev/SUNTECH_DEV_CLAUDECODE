<?php
$page_title = 'Line Management';
$page_css_files = [
  '../../assets/css/fiori-page.css',
  'css/info_line_2.css',
];

require_once(__DIR__ . '/../../inc/head.php');
/* nav-fiori.php 제거 */
?>

<?php $nav_active = 'line'; require_once(__DIR__ . '/../../inc/nav-drawer-manage.php'); ?>

<!-- Signage Header -->
<div class="signage-header">
  <button id="navDrawerBtn" class="nav-drawer-btn" aria-label="Menu">&#9776;</button>
  <span class="signage-header__title">Line Management</span>

  <div class="signage-header__filters">
    <div class="search-container">
      <input type="text" id="realTimeSearch" class="fiori-input search-input" placeholder="Search line...">
      <button class="search-clear" id="searchClear" title="Clear search">&#10005;</button>
    </div>
    <select id="factoryFilterSelect" class="fiori-select">
      <!-- JS로 채워짐 -->
    </select>
    <select id="statusFilterSelect" class="fiori-select">
      <option value="">All Status</option>
      <option value="Y">Used Only</option>
      <option value="N">Unused Only</option>
    </select>
    <div class="fiori-dropdown">
      <button id="columnToggleBtn" class="fiori-btn fiori-btn--secondary">Columns</button>
      <div id="columnToggleDropdown" class="fiori-dropdown__content"></div>
    </div>
    <button id="addBtn" class="fiori-btn fiori-btn--primary">Add Line</button>
    <button id="refreshBtn" class="fiori-btn fiori-btn--tertiary">Refresh</button>
  </div>
</div>

<!-- Main -->
<div class="manage-main">

  <div class="fiori-card">
    <div class="fiori-card__header">
      <div>
        <h3 class="fiori-card__title">Line List</h3>
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
        <p>No matching lines found.</p>
      </div>
    </div>
  </div>

  <div id="pagination-controls" class="fiori-pagination"></div>

</div><!-- /manage-main -->


<!-- Line Modal -->
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

          <!-- Step 1: Basic information -->
          <div class="form-section" data-section="1">
            <h4 style="margin-bottom: var(--sap-spacing-md); color: var(--sap-text-primary);">Basic information</h4>

            <div class="fiori-form__group">
              <label for="factory_idx" class="fiori-form__label fiori-form__label--required">Factory Name</label>
              <select id="factory_idx" name="factory_idx" class="fiori-select" required>
                <!-- JS로 채워짐 -->
              </select>
              <div class="fiori-form__help">Select the factory where this line is located</div>
            </div>

            <div class="fiori-form__group">
              <label for="line_name" class="fiori-form__label fiori-form__label--required">Line Name</label>
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

          <!-- Step 2: Additional information -->
          <div class="form-section" data-section="2" style="display: none;">
            <h4 style="margin-bottom: var(--sap-spacing-md); color: var(--sap-text-primary);">Additional information</h4>

            <div class="fiori-form__group">
              <label for="mp" class="fiori-form__label fiori-form__label--required">Man Power</label>
              <input type="number" id="mp" name="mp" class="fiori-input" required value="0" min="0">
              <div class="fiori-form__help">Number of workers assigned to this line</div>
            </div>

            <div class="fiori-form__group">
              <label for="line_target" class="fiori-form__label fiori-form__label--required">Line Target</label>
              <input type="number" id="line_target" name="line_target" class="fiori-input" required value="0" min="0">
              <div class="fiori-form__help">Daily production target for this line</div>
            </div>
          </div>

          <!-- Step 3: Remark & Review -->
          <div class="form-section" data-section="3" style="display: none;">
            <h4 style="margin-bottom: var(--sap-spacing-md); color: var(--sap-text-primary);">Remark Information</h4>

            <div class="fiori-form__group">
              <label for="remark" class="fiori-form__label">Remark</label>
              <textarea id="remark" name="remark" class="fiori-input" rows="3"></textarea>
              <div class="fiori-form__help">You can record line information.</div>
            </div>

            <div style="padding: var(--sap-spacing-md); background: var(--sap-surface-2); border-radius: var(--sap-radius-md); margin-top: var(--sap-spacing-md);">
              <h5 style="margin: 0 0 var(--sap-spacing-sm) 0; color: var(--sap-text-primary);">Confirm Information</h5>
              <div style="font-size: var(--sap-font-size-sm); color: var(--sap-text-secondary);">
                <div><strong>Factory name:</strong> <span id="previewName">-</span></div>
                <div><strong>Line Name:</strong> <span id="previewLine">-</span></div>
                <div><strong>Man Power:</strong> <span id="previewMP">-</span></div>
                <div><strong>Line Target:</strong> <span id="previewTarget">-</span></div>
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
  import { createResourceManager } from '../../assets/js/resource-manager.js';
  import { initAdvancedFeatures, lineConfig } from './js/info_line_2.js';

  document.addEventListener('DOMContentLoaded', function() {
    const resourceManager = createResourceManager(lineConfig);
    setTimeout(() => initAdvancedFeatures(resourceManager), 100);
  });
</script>


</body>
</html>
