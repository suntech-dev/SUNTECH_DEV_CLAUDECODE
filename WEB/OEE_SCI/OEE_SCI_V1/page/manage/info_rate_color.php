<?php
// Page-specific settings
$page_title = 'Rate Color Management';
$page_css_files = ['../../assets/css/fiori-style.css', '../../assets/css/ion.rangeSlider.min.css', '../../assets/css/spectrum.css', 'css/info_rate_color.css'];
$page_styles = '';

// Load common header
require_once(__DIR__ . '/../../inc/head.php');
require_once(__DIR__ . '/../../inc/nav-fiori.php');
?>

  <div class="fiori-container">
    <main>
      
      <!-- Page header -->
      <div class="fiori-main-header">
        <div>
          <h2>🎨 Rate Color Management</h2>
          <!-- <p style="color: var(--sap-text-secondary); margin: var(--sap-spacing-xs) 0 0 0;">
            Configure and manage rate-based color stages
          </p> -->
        </div>
        <div class="fiori-header-actions">
          <!-- <button id="resetBtn" class="fiori-btn fiori-btn--tertiary">🔄 Reset</button> -->
          <button id="previewBtn" class="fiori-btn fiori-btn--secondary">👁 Preview</button>
          <button id="saveBtn" class="fiori-btn fiori-btn--primary">💾 Save Changes</button>
        </div>
      </div>

      <!-- Rate Color Configuration Section -->
      <div class="fiori-section">
        <div class="fiori-card">
          <div class="fiori-card__header">
            <h3 class="fiori-card__title">📊 Rate Range and Color Settings</h3>
            <div class="quick-actions">
              <span class="fiori-badge fiori-badge--info">5 Stages</span>
            </div>
          </div>
          <div class="fiori-card__content">
            
            <!-- Rate Stage 1: 0% < rate ≤ upper limit -->
            <div class="rate-stage-card" data-stage="1">
              <div class="rate-stage-header">
                <div class="rate-stage-title">Stage 1: 0% < rate ≤ 25%</div>
                <div class="selected-color-preview">
                  <input type="text" id="stage1ColorPicker" class="spectrum-colorpicker" value="#6b7884" data-stage="1" />
                </div>
              </div>
              <div class="rate-range-container">
                <!-- <label for="stage1Range" style="color: var(--sap-text-secondary); font-size: var(--sap-font-size-sm);">
                  Adjust upper limit (Current: 0% < rate ≤ 25%)
                </label> -->
                <div class="range-slider-wrapper">
                  <input type="text" id="stage1Range" class="fiori-range-slider" 
                         data-type="double" 
                         data-min="0" 
                         data-max="100" 
                         data-from="0" 
                         data-to="25" 
                         data-postfix="%" 
                         data-skin="fiori" />
                </div>
              </div>
            </div>

            <!-- Rate Stage 2: 0% < rate <= 50% -->
            <div class="rate-stage-card" data-stage="2">
              <div class="rate-stage-header">
                <div class="rate-stage-title">Stage 2: 0% < rate ≤ 50%</div>
                <div class="selected-color-preview">
                  <input type="text" id="stage2ColorPicker" class="spectrum-colorpicker" value="#da1e28" data-stage="2" />
                </div>
              </div>
              <div class="rate-range-container">
                <!-- <label for="stage2Range" style="color: var(--sap-text-secondary); font-size: var(--sap-font-size-sm);">
                  Adjust upper limit (Current: 25% < rate ≤ 50%)
                </label> -->
                <div class="range-slider-wrapper">
                  <input type="text" id="stage2Range" class="fiori-range-slider" 
                         data-type="double" 
                         data-min="0" 
                         data-max="100" 
                         data-from="25" 
                         data-to="50" 
                         data-postfix="%" 
                         data-skin="fiori" />
                </div>
              </div>
            </div>

            <!-- Rate Stage 3: 50% < rate <= 80% -->
            <div class="rate-stage-card" data-stage="3">
              <div class="rate-stage-header">
                <div class="rate-stage-title">Stage 3: 50% < rate ≤ 80%</div>
                <div class="selected-color-preview">
                  <input type="text" id="stage3ColorPicker" class="spectrum-colorpicker" value="#e26b0a" data-stage="3" />
                </div>
              </div>
              <div class="rate-range-container">
                <!-- <label for="stage3Range" style="color: var(--sap-text-secondary); font-size: var(--sap-font-size-sm);">
                  Adjust upper limit (Current: 50% < rate ≤ 80%)
                </label> -->
                <div class="range-slider-wrapper">
                  <input type="text" id="stage3Range" class="fiori-range-slider" 
                         data-type="double" 
                         data-min="0" 
                         data-max="100" 
                         data-from="50" 
                         data-to="80" 
                         data-postfix="%" 
                         data-skin="fiori" />
                </div>
              </div>
            </div>

            <!-- Rate Stage 4: 80% < rate <= 100% -->
            <div class="rate-stage-card" data-stage="4">
              <div class="rate-stage-header">
                <div class="rate-stage-title">Stage 4: 80% < rate ≤ 100%</div>
                <div class="selected-color-preview">
                  <input type="text" id="stage4ColorPicker" class="spectrum-colorpicker" value="#30914c" data-stage="4" />
                </div>
              </div>
              <div class="rate-range-container">
                <label for="stage4Range" style="color: var(--sap-text-secondary); font-size: var(--sap-font-size-sm);">
                  Fixed value (80% < rate ≤ 100%)
                </label>
                <div class="range-slider-wrapper">
                  <input type="text" id="stage4Range" class="fiori-range-slider" 
                         data-type="double" 
                         data-min="0" 
                         data-max="100" 
                         data-from="80" 
                         data-to="100" 
                         data-postfix="%" 
                         data-skin="fiori" 
                         data-disable="true" />
                </div>
              </div>
            </div>

            <!-- Rate Stage 5: Over 100% -->
            <div class="rate-stage-card" data-stage="5">
              <div class="rate-stage-header">
                <div class="rate-stage-title">Stage 5: Over 100%</div>
                <div class="selected-color-preview">
                  <input type="text" id="stage5ColorPicker" class="spectrum-colorpicker" value="#0070f2" data-stage="5" />
                </div>
              </div>
              <div class="rate-range-container">
                <!-- <label style="color: var(--sap-text-secondary); font-size: var(--sap-font-size-sm);">
                  Fixed range: Over 100% (No slider)
                </label> -->
                <div style="padding: var(--sap-spacing-md); background: var(--sap-surface-2); border-radius: var(--sap-radius-sm); text-align: center; color: var(--sap-text-secondary);">
                  📊 Applied to all values exceeding 100%
                </div>
              </div>
            </div>


          </div>
        </div>
      </div>

      <!-- Current Settings Preview -->
      <div class="fiori-section">
        <div class="fiori-card">
          <div class="fiori-card__header">
            <h3 class="fiori-card__title">🎯 Rate Color Preview</h3>
            <div class="quick-actions">
              <button id="testBtn" class="fiori-btn fiori-btn--tertiary">🧪 Test Colors</button>
            </div>
          </div>
          <div class="fiori-card__content">
            <div id="previewContainer" style="display: grid; grid-template-columns: repeat(auto-fit, minmax(200px, 1fr)); gap: var(--sap-spacing-md);">
              <!-- Dynamically generated by JS -->
            </div>
          </div>
        </div>
      </div>

    </main>
  </div>

  <!-- Preview Modal -->
  <div id="previewModal" class="fiori-modal">
    <div class="fiori-modal__backdrop"></div>
    <div class="fiori-modal__content" style="max-width: 600px;">
      <div class="fiori-card">
        <div class="fiori-card__header">
          <h3 class="fiori-card__title">🎯 Rate Color System Preview</h3>
          <button class="fiori-btn fiori-btn--tertiary modal-close">✕</button>
        </div>
        <div class="fiori-card__content">
          <div id="modalPreviewContainer">
            <!-- Dynamically generated by JavaScript -->
          </div>
        </div>
      </div>
    </div>
  </div>

  <footer class="fiori-footer">
    <p>&copy; 2025 SUNTECH. All Rights Reserved.</p>
  </footer>

  <!-- Ion Range Slider Library -->
   
  <!-- <script src="https://code.jquery.com/jquery-3.6.0.min.js"></script> -->
  <script src="../../assets/js/jquery-3.6.1.min.js"></script>
  <script src="../../assets/js/ion.rangeSlider.min.js"></script>
  <script src="../../assets/js/spectrum.js"></script>
  
  <!-- Rate Color Dedicated Script -->
  <script src="./js/info_rate_color.js"></script>

</body>
</html>