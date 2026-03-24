<?php
$page_title = 'Downtime Management';
$page_css_files = [
  '../../assets/css/fiori-page.css',
  'css/info_downtime_2.css',
];

require_once(__DIR__ . '/../../inc/head.php');
/* nav-fiori.php 제거 */
?>

<?php $nav_active = 'downtime'; require_once(__DIR__ . '/../../inc/nav-drawer-manage.php'); ?>

<!-- Signage Header -->
<div class="signage-header">
  <button id="navDrawerBtn" class="nav-drawer-btn" aria-label="Menu">&#9776;</button>
  <span class="signage-header__title">Downtime Management</span>

  <div class="signage-header__filters">
    <div class="search-container">
      <input type="text" id="realTimeSearch" class="fiori-input search-input" placeholder="Search downtime...">
      <button class="search-clear" id="searchClear" title="Clear search">&#10005;</button>
    </div>
    <select id="statusFilterSelect" class="fiori-select">
      <option value="">All Status</option>
      <option value="Y">Used Only</option>
      <option value="N">Unused Only</option>
    </select>
    <div class="fiori-dropdown">
      <button id="columnToggleBtn" class="fiori-btn fiori-btn--secondary">Columns</button>
      <div id="columnToggleDropdown" class="fiori-dropdown__content"></div>
    </div>
    <button id="addBtn" class="fiori-btn fiori-btn--primary">Add Downtime</button>
    <button id="refreshBtn" class="fiori-btn fiori-btn--tertiary">Refresh</button>
  </div>
</div>

<!-- Main -->
<div class="manage-main">

  <div class="fiori-card">
    <div class="fiori-card__header">
      <div>
        <h3 class="fiori-card__title">Downtime List</h3>
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
        <p>No matching downtimes found.</p>
      </div>
    </div>
  </div>

  <div id="pagination-controls" class="fiori-pagination"></div>

</div><!-- /manage-main -->


<!-- Downtime Modal -->
<div id="resourceModal" class="fiori-modal">
  <div class="fiori-modal__content">
    <div class="fiori-card">
      <div class="fiori-card__header">
        <div>
          <h3 class="fiori-card__title" id="modalTitle">Downtime Information</h3>
          <p class="fiori-card__subtitle">Enter and edit downtime information</p>
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
            </div>
            <div style="font-size: var(--sap-font-size-sm); color: var(--sap-text-secondary);">
              <span id="stepIndicator">Step 1 of 2</span>
            </div>
          </div>

          <!-- Step 1 -->
          <div class="form-section" data-section="1">
            <h4 style="margin-bottom: var(--sap-spacing-md); color: var(--sap-text-primary);">Basic information</h4>

            <div class="fiori-form__group">
              <label for="downtime_name" class="fiori-form__label fiori-form__label--required">Downtime Name</label>
              <input type="text" id="downtime_name" name="downtime_name" class="fiori-input" required
                     placeholder="Please enter the downtime name">
              <div class="fiori-form__help">Please enter a unique downtime name (2-50 characters)</div>
            </div>

            <div class="fiori-form__group">
              <label for="downtime_shortcut" class="fiori-form__label">Shortcut Code</label>
              <input type="text" id="downtime_shortcut" name="downtime_shortcut" class="fiori-input"
                     placeholder="Enter shortcut code (optional)" maxlength="10">
              <div class="fiori-form__help">Optional shortcut code for quick downtime selection (max 10 characters)</div>
            </div>
          </div>

          <!-- Step 2 -->
          <div class="form-section" data-section="2" style="display: none;">
            <h4 style="margin-bottom: var(--sap-spacing-md); color: var(--sap-text-primary);">Additional information</h4>

            <div class="fiori-form__group">
              <label for="status" class="fiori-form__label">Status</label>
              <select id="status" name="status" class="fiori-select">
                <option value="Y">Used</option>
                <option value="N">Unused</option>
              </select>
              <div class="fiori-form__help">Please select the current usage status of the downtime.</div>
            </div>

            <div class="fiori-form__group">
              <label for="remark" class="fiori-form__label">Remark</label>
              <textarea id="remark" name="remark" class="fiori-input" rows="3"
                        placeholder="You can record downtime information."></textarea>
            </div>

            <div style="padding: var(--sap-spacing-md); background: var(--sap-surface-2); border-radius: var(--sap-radius-md); margin-top: var(--sap-spacing-md);">
              <h5 style="margin: 0 0 var(--sap-spacing-sm) 0; color: var(--sap-text-primary);">Confirm Information</h5>
              <div style="font-size: var(--sap-font-size-sm); color: var(--sap-text-secondary);">
                <div><strong>Downtime name:</strong> <span id="previewName">-</span></div>
                <div><strong>Shortcut code:</strong> <span id="previewShortcut">-</span></div>
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
  import { initAdvancedFeatures, downtimeConfig } from './js/info_downtime_2.js';

  document.addEventListener('DOMContentLoaded', function() {
    const resourceManager = createResourceManager(downtimeConfig);
    setTimeout(() => initAdvancedFeatures(resourceManager), 100);
  });
</script>


</body>
</html>
