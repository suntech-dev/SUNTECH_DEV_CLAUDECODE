<?php
$page_title = 'Rate Color Management';
$page_css_files = [
    '../../assets/css/fiori-page.css',
    '../../assets/css/ion.rangeSlider.min.css',
    '../../assets/css/spectrum.css',
    'css/info_rate_color_2.css',
];
require_once(__DIR__ . '/../../inc/head.php');
/* nav-fiori.php 제거 */
?>

<?php $nav_active = 'rate_color'; require_once(__DIR__ . '/../../inc/nav-drawer-manage.php'); ?>

<!-- Signage Header -->
<div class="signage-header">
    <button id="navDrawerBtn" class="nav-drawer-btn" aria-label="Menu">&#9776;</button>
    <span class="signage-header__title">Rate Color Management</span>
    <div class="signage-header__filters">
        <button id="testBtn" class="fiori-btn fiori-btn--tertiary">Test Colors</button>
        <button id="previewBtn" class="fiori-btn fiori-btn--secondary">Preview</button>
        <button id="saveBtn" class="fiori-btn fiori-btn--primary">Save Changes</button>
    </div>
</div>

<!-- Main -->
<div class="rate-main">

    <!-- Stage Configuration -->
    <div class="fiori-card">
        <div class="fiori-card__header">
            <h3 class="fiori-card__title">Rate Range and Color Settings</h3>
            <div class="quick-actions">
                <span class="fiori-badge fiori-badge--info">5 Stages</span>
            </div>
        </div>
        <div class="fiori-card__content">

            <!-- Stage 1 -->
            <div class="rate-stage-card" data-stage="1">
                <div class="rate-stage-header">
                    <div class="rate-stage-title">Stage 1: 0% &lt; rate &le; 25%</div>
                    <div class="selected-color-preview">
                        <input type="text" id="stage1ColorPicker" class="spectrum-colorpicker" value="#6b7884" data-stage="1" />
                    </div>
                </div>
                <div class="rate-range-container">
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

            <!-- Stage 2 -->
            <div class="rate-stage-card" data-stage="2">
                <div class="rate-stage-header">
                    <div class="rate-stage-title">Stage 2: 25% &lt; rate &le; 50%</div>
                    <div class="selected-color-preview">
                        <input type="text" id="stage2ColorPicker" class="spectrum-colorpicker" value="#da1e28" data-stage="2" />
                    </div>
                </div>
                <div class="rate-range-container">
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

            <!-- Stage 3 -->
            <div class="rate-stage-card" data-stage="3">
                <div class="rate-stage-header">
                    <div class="rate-stage-title">Stage 3: 50% &lt; rate &le; 80%</div>
                    <div class="selected-color-preview">
                        <input type="text" id="stage3ColorPicker" class="spectrum-colorpicker" value="#e26b0a" data-stage="3" />
                    </div>
                </div>
                <div class="rate-range-container">
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

            <!-- Stage 4 -->
            <div class="rate-stage-card" data-stage="4">
                <div class="rate-stage-header">
                    <div class="rate-stage-title">Stage 4: 80% &lt; rate &le; 100%</div>
                    <div class="selected-color-preview">
                        <input type="text" id="stage4ColorPicker" class="spectrum-colorpicker" value="#30914c" data-stage="4" />
                    </div>
                </div>
                <div class="rate-range-container">
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

            <!-- Stage 5 -->
            <div class="rate-stage-card" data-stage="5">
                <div class="rate-stage-header">
                    <div class="rate-stage-title">Stage 5: Over 100%</div>
                    <div class="selected-color-preview">
                        <input type="text" id="stage5ColorPicker" class="spectrum-colorpicker" value="#0070f2" data-stage="5" />
                    </div>
                </div>
                <div class="rate-range-container">
                    <div class="range-fixed-label">Applied to all values exceeding 100%</div>
                </div>
            </div>

        </div>
    </div>

    <!-- Preview -->
    <div class="fiori-card">
        <div class="fiori-card__header">
            <h3 class="fiori-card__title">Rate Color Preview</h3>
        </div>
        <div class="fiori-card__content">
            <div id="previewContainer" class="preview-grid"></div>
        </div>
    </div>

</div><!-- /rate-main -->


<!-- Preview Modal -->
<div id="previewModal" class="fiori-modal">
    <div class="fiori-modal__backdrop"></div>
    <div class="fiori-modal__content" style="max-width: 600px;">
        <div class="fiori-card">
            <div class="fiori-card__header">
                <h3 class="fiori-card__title">Rate Color System Preview</h3>
                <button class="fiori-btn fiori-btn--tertiary modal-close">&#10005;</button>
            </div>
            <div class="fiori-card__content">
                <div id="modalPreviewContainer"></div>
            </div>
        </div>
    </div>
</div>

<!-- Color Palette Modal -->
<div id="colorPaletteModal" class="fiori-modal">
    <div class="fiori-modal__backdrop"></div>
    <div class="fiori-modal__content" style="max-width: 480px;">
        <div class="fiori-card">
            <div class="fiori-card__header">
                <h3 class="fiori-card__title">Color Selection</h3>
                <button class="fiori-btn fiori-btn--tertiary modal-close">&#10005;</button>
            </div>
            <div class="fiori-card__content">
                <div id="modalColorPalette" class="modal-palette-grid"></div>
            </div>
        </div>
    </div>
</div>


<script src="../../assets/js/jquery-3.6.1.min.js"></script>
<script src="../../assets/js/ion.rangeSlider.min.js"></script>
<script src="../../assets/js/spectrum.js"></script>
<script src="./js/info_rate_color_2.js"></script>


</body>
</html>
