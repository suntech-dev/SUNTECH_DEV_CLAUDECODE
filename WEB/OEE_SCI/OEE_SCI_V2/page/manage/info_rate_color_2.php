<?php
/**
 * info_rate_color_2.php — Rate Color 관리 페이지
 *
 * 생산율(OEE Rate) 단계별 색상 범위 설정 페이지 (SAP Fiori 스타일)
 *
 * 다른 관리 페이지와의 차이점:
 *   - ResourceManager 미사용: 고정 UI (CRUD 테이블 없음)
 *   - 5단계 Rate Stage 카드: 각 단계마다 범위 슬라이더 + 색상 피커
 *   - Ion Range Slider (jQuery 플러그인): 양방향 범위 슬라이더
 *   - Spectrum (jQuery 플러그인): 색상 선택 피커
 *   - ES Module 미사용: <script src> 방식으로 직접 로드
 *
 * CSS 로드 순서:
 *   1. fiori-page.css           : 공통 SAP Fiori 레이아웃/컴포넌트
 *   2. ion.rangeSlider.min.css  : Ion Range Slider 플러그인 스타일
 *   3. spectrum.css             : Spectrum 컬러피커 플러그인 스타일
 *   4. info_rate_color_2.css    : Rate Color 관리 페이지 전용 스타일
 *
 * 스크립트 로드 순서 (의존성 순서 중요):
 *   1. jquery-3.6.1.min.js      : jQuery (Ion Range Slider + Spectrum 의존성)
 *   2. ion.rangeSlider.min.js   : Ion Range Slider 플러그인
 *   3. spectrum.js              : Spectrum 컬러피커 플러그인
 *   4. ./js/info_rate_color_2.js: 페이지 로직 (모듈 아님, DOMContentLoaded 직접 사용)
 *
 * 5단계 구성:
 *   Stage 1: 0%   ~ 25%  (from 고정=0, to 조절 가능)
 *   Stage 2: 25%  ~ 50%  (양방향 조절, Stage 1 to와 연동)
 *   Stage 3: 50%  ~ 80%  (양방향 조절, Stage 2 to와 연동)
 *   Stage 4: 80%  ~ 100% (from 조절 가능, to=100 고정, data-disable="true")
 *   Stage 5: 100% 초과    (슬라이더 없음, 고정 레이블만 표시)
 *
 * 모달:
 *   #previewModal      : Rate Color 시스템 전체 미리보기
 *   #colorPaletteModal : 색상 팔레트 선택 모달 (52색)
 */
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

<?php
// 네비게이션 드로어: rate_color 메뉴 항목을 active 상태로 표시
$nav_active = 'rate_color'; require_once(__DIR__ . '/../../inc/nav-drawer-manage.php');
?>

<!-- Signage Header: 상단 네비게이션 바 -->
<div class="signage-header">
    <!-- 햄버거 버튼: 좌측 네비게이션 드로어 열기/닫기 -->
    <button id="navDrawerBtn" class="nav-drawer-btn" aria-label="Menu">&#9776;</button>
    <span class="signage-header__title">Rate Color Management</span>
    <div class="signage-header__filters">
        <!-- testBtn: 현재 설정 기준으로 0~120% 범위 색상 렌더링 테스트 -->
        <button id="testBtn" class="fiori-btn fiori-btn--tertiary">Test Colors</button>
        <!-- previewBtn: #previewModal 열기 → 전체 Rate Color 시스템 미리보기 -->
        <button id="previewBtn" class="fiori-btn fiori-btn--secondary">Preview</button>
        <!-- saveBtn: saveConfiguration() 호출 → validateStageSystem → POST proc/rate_color.php -->
        <button id="saveBtn" class="fiori-btn fiori-btn--primary">Save Changes</button>
    </div>
</div>

<!-- Main: Rate Color 설정 영역 -->
<div class="rate-main">

    <!-- Stage 설정 카드: 5개 단계 범위 및 색상 설정 -->
    <div class="fiori-card">
        <div class="fiori-card__header">
            <h3 class="fiori-card__title">Rate Range and Color Settings</h3>
            <div class="quick-actions">
                <!-- 5 Stages 배지: 단계 수 안내 -->
                <span class="fiori-badge fiori-badge--info">5 Stages</span>
            </div>
        </div>
        <div class="fiori-card__content">

            <!-- Stage 1: 0% < rate ≤ 25% -->
            <!-- data-stage="1": info_rate_color_2.js에서 stage1 전역 객체와 매핑 -->
            <!-- from 고정(=0): initializeRangeSliders()에서 from_fixed=true 설정 -->
            <div class="rate-stage-card" data-stage="1">
                <div class="rate-stage-header">
                    <div class="rate-stage-title">Stage 1: 0% &lt; rate &le; 25%</div>
                    <div class="selected-color-preview">
                        <!-- stage1ColorPicker: Spectrum 컬러피커 초기화 대상 (jQuery) -->
                        <!-- data-stage="1": initializeSpectrumColorPickers()에서 루프 식별자로 사용 -->
                        <!-- value="#6b7884": 기본 색상 (회색 계열) -->
                        <input type="text" id="stage1ColorPicker" class="spectrum-colorpicker" value="#6b7884" data-stage="1" />
                    </div>
                </div>
                <div class="rate-range-container">
                    <div class="range-slider-wrapper">
                        <!-- stage1Range: Ion Range Slider 초기화 대상 -->
                        <!-- data-type="double": 양방향 슬라이더 (from/to 핸들 2개) -->
                        <!-- data-from="0" 고정: initializeRangeSliders()에서 from_fixed=true 처리 -->
                        <!-- data-to="25": Stage 1 초기 상한값 -->
                        <!-- data-skin="fiori": 커스텀 Fiori 스킨 적용 -->
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

            <!-- Stage 2: 25% < rate ≤ 50% -->
            <!-- from: Stage 1의 to와 연동 (Stage 1 to 변경 시 Stage 2 from 자동 갱신) -->
            <!-- to: Stage 3의 from과 연동 (Stage 2 to 변경 시 Stage 3 from 자동 갱신) -->
            <div class="rate-stage-card" data-stage="2">
                <div class="rate-stage-header">
                    <div class="rate-stage-title">Stage 2: 25% &lt; rate &le; 50%</div>
                    <div class="selected-color-preview">
                        <!-- stage2ColorPicker: Spectrum 컬러피커 초기화 대상 -->
                        <!-- value="#da1e28": 기본 색상 (빨간색 계열) -->
                        <input type="text" id="stage2ColorPicker" class="spectrum-colorpicker" value="#da1e28" data-stage="2" />
                    </div>
                </div>
                <div class="rate-range-container">
                    <div class="range-slider-wrapper">
                        <!-- stage2Range: Ion Range Slider 초기화 대상 -->
                        <!-- onChange: oldFrom/oldTo 비교로 변경된 핸들만 연쇄 갱신 -->
                        <!-- updateInProgress 플래그: 연쇄 갱신 시 무한 루프 방지 -->
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

            <!-- Stage 3: 50% < rate ≤ 80% -->
            <!-- from: Stage 2의 to와 연동, to: Stage 4의 from과 연동 -->
            <div class="rate-stage-card" data-stage="3">
                <div class="rate-stage-header">
                    <div class="rate-stage-title">Stage 3: 50% &lt; rate &le; 80%</div>
                    <div class="selected-color-preview">
                        <!-- value="#e26b0a": 기본 색상 (주황색 계열) -->
                        <input type="text" id="stage3ColorPicker" class="spectrum-colorpicker" value="#e26b0a" data-stage="3" />
                    </div>
                </div>
                <div class="rate-range-container">
                    <div class="range-slider-wrapper">
                        <!-- stage3Range: Stage 2와 동일한 연쇄 갱신 로직 -->
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

            <!-- Stage 4: 80% < rate ≤ 100% -->
            <!-- data-disable="true": to 핸들 고정(=100), from만 Stage 3 to와 연동하여 갱신 -->
            <!-- initializeRangeSliders()에서 disable_to=true 처리 -->
            <div class="rate-stage-card" data-stage="4">
                <div class="rate-stage-header">
                    <div class="rate-stage-title">Stage 4: 80% &lt; rate &le; 100%</div>
                    <div class="selected-color-preview">
                        <!-- value="#30914c": 기본 색상 (녹색 계열) -->
                        <input type="text" id="stage4ColorPicker" class="spectrum-colorpicker" value="#30914c" data-stage="4" />
                    </div>
                </div>
                <div class="rate-range-container">
                    <div class="range-slider-wrapper">
                        <!-- data-disable="true": 슬라이더 UI는 표시되지만 to 핸들 비활성화 -->
                        <!-- Stage 3 to 변경 시 Stage 4 from이 자동 갱신 (updateLinkedStageSlider 호출) -->
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

            <!-- Stage 5: 100% 초과 (Over 100%) -->
            <!-- 슬라이더 없음: .range-fixed-label로 안내 텍스트만 표시 -->
            <!-- 색상 피커만 있음: 100% 초과 시 적용할 색상 지정 -->
            <div class="rate-stage-card" data-stage="5">
                <div class="rate-stage-header">
                    <div class="rate-stage-title">Stage 5: Over 100%</div>
                    <div class="selected-color-preview">
                        <!-- value="#0070f2": 기본 색상 (파란색 계열) -->
                        <input type="text" id="stage5ColorPicker" class="spectrum-colorpicker" value="#0070f2" data-stage="5" />
                    </div>
                </div>
                <div class="rate-range-container">
                    <!-- 슬라이더 대신 고정 안내 레이블 표시 -->
                    <div class="range-fixed-label">Applied to all values exceeding 100%</div>
                </div>
            </div>

        </div>
    </div>

    <!-- Rate Color 미리보기 카드: previewContainer에 동적으로 색상 샘플 렌더링 -->
    <!-- testBtn 클릭 또는 초기 loadExistingConfig() 완료 후 갱신 -->
    <div class="fiori-card">
        <div class="fiori-card__header">
            <h3 class="fiori-card__title">Rate Color Preview</h3>
        </div>
        <div class="fiori-card__content">
            <!-- previewContainer: CSS Grid 레이아웃 (.preview-grid) -->
            <!-- 각 셀: rate 값(%) + 배경색 박스로 구성 -->
            <div id="previewContainer" class="preview-grid"></div>
        </div>
    </div>

</div><!-- /rate-main -->


<!-- Preview Modal: Rate Color 시스템 전체 미리보기 다이얼로그 -->
<!-- previewBtn 클릭 → 'show' 클래스 추가하여 표시 -->
<!-- #modalPreviewContainer: 현재 stage 설정 기준 미리보기 동적 렌더링 -->
<div id="previewModal" class="fiori-modal">
    <div class="fiori-modal__backdrop"></div>
    <div class="fiori-modal__content" style="max-width: 600px;">
        <div class="fiori-card">
            <div class="fiori-card__header">
                <h3 class="fiori-card__title">Rate Color System Preview</h3>
                <!-- modal-close: 클릭 시 'show' 클래스 제거 -->
                <button class="fiori-btn fiori-btn--tertiary modal-close">&#10005;</button>
            </div>
            <div class="fiori-card__content">
                <!-- modalPreviewContainer: previewBtn 클릭 시 현재 설정 기준 색상 샘플 렌더링 -->
                <div id="modalPreviewContainer"></div>
            </div>
        </div>
    </div>
</div>

<!-- Color Palette Modal: 색상 팔레트 선택 다이얼로그 (52색) -->
<!-- Spectrum 컬러피커 클릭 시 표시 (또는 별도 팔레트 아이콘 클릭) -->
<!-- #modalColorPalette: .modal-palette-grid 레이아웃으로 colorPalette 배열 렌더링 -->
<div id="colorPaletteModal" class="fiori-modal">
    <div class="fiori-modal__backdrop"></div>
    <div class="fiori-modal__content" style="max-width: 480px;">
        <div class="fiori-card">
            <div class="fiori-card__header">
                <h3 class="fiori-card__title">Color Selection</h3>
                <!-- modal-close: 클릭 시 'show' 클래스 제거 -->
                <button class="fiori-btn fiori-btn--tertiary modal-close">&#10005;</button>
            </div>
            <div class="fiori-card__content">
                <!-- modalColorPalette: 52색 팔레트 색상 박스 렌더링 -->
                <!-- 각 박스 클릭 → 현재 활성 stage의 색상 피커 값 업데이트 -->
                <div id="modalColorPalette" class="modal-palette-grid"></div>
            </div>
        </div>
    </div>
</div>


<!-- jQuery: Ion Range Slider + Spectrum 컬러피커 의존성 -->
<script src="../../assets/js/jquery-3.6.1.min.js"></script>
<!-- Ion Range Slider: 양방향 범위 슬라이더 플러그인 -->
<script src="../../assets/js/ion.rangeSlider.min.js"></script>
<!-- Spectrum: 색상 피커 플러그인 -->
<script src="../../assets/js/spectrum.js"></script>
<!-- info_rate_color_2.js: Rate Color 관리 페이지 로직 (ES Module 아님, DOMContentLoaded 직접 사용) -->
<!-- API 응답 형식: { code: '00', msg, data } (다른 proc/*.php와 다름) -->
<script src="./js/info_rate_color_2.js"></script>


</body>
</html>
