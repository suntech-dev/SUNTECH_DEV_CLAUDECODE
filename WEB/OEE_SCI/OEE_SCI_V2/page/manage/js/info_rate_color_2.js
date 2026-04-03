/**
 * info_rate_color_2.js
 * Rate Color Management — Fullscreen Redesign
 *
 * OEE 비율(rate) 값에 따라 색상을 5단계로 구분하는 설정 페이지 전용 스크립트
 * ES Module이 아닌 일반 스크립트로 로드됨 (export 없음)
 *
 * 의존 라이브러리:
 *   - Ion Range Slider (jQuery 플러그인): 구간 설정 슬라이더 (#stage1Range ~ #stage4Range)
 *   - Spectrum Color Picker (jQuery 플러그인): 단계별 색상 선택 (#stage1ColorPicker ~ #stage5ColorPicker)
 *
 * 5단계 구조:
 *   stage1: 0% < rate ≤ end1   (기본 회색)
 *   stage2: end1 < rate ≤ end2  (기본 빨강)
 *   stage3: end2 < rate ≤ end3  (기본 주황)
 *   stage4: end3 < rate ≤ 100%  (기본 초록, to=100 고정)
 *   stage5: 100% 초과            (기본 파랑, start=100/end=999 고정)
 *
 * 슬라이더 연쇄 업데이트 구조:
 *   stage1 to → stage2 from 자동 갱신
 *   stage2 from/to → stage1 to / stage3 from 자동 갱신
 *   stage3 from/to → stage2 to / stage4 from 자동 갱신
 *   stage4: disable=true (항상 end=100 고정, 사용자 조작 불가)
 *   stage5: 슬라이더 없음 (start=100, end=999 자동 설정)
 *
 * API 응답 형식 (다른 proc/*.php와 다름):
 *   성공: { code: '00', msg: '...', data: { ... } }
 *   실패: { code: '!= 00', msg: '...' }
 */

/** 5단계 색상 설정 전역 상태 객체 (기본값으로 초기화) */
let rateColorConfig = {
    stage1: { start_rate: 0,   end_rate: 25,  color: '#6b7884' },
    stage2: { start_rate: 25,  end_rate: 50,  color: '#da1e28' },
    stage3: { start_rate: 50,  end_rate: 80,  color: '#e26b0a' },
    stage4: { start_rate: 80,  end_rate: 100, color: '#30914c' },
    stage5: { start_rate: 100, end_rate: 999, color: '#0070f2' }
};

/** 현재 팔레트 모달에서 선택 중인 stage 번호 (null이면 모달 닫힘) */
let currentSelectedStage = null;

/**
 * 슬라이더 onChange 중복 실행 방지 플래그
 * Ion Range Slider는 update() 호출 시에도 onChange가 재발생하므로
 * 연쇄 업데이트(updateLinkedStageSlider) 중에는 해당 슬라이더의 플래그를 true로 설정하여
 * 무한 루프 방지
 */
let updateInProgress = {
    stage1: false,
    stage2: false,
    stage3: false,
    stage4: false
};

/**
 * 색상 팔레트 모달에서 직접 선택할 수 있는 색상 목록 (52색)
 * Spectrum 컬러피커 대신 팔레트 모달을 원하는 경우 사용
 */
const colorPalette = [
    '#ff0000', '#e53e3e', '#dc2626', '#b91c1c',
    '#00ff00', '#38a169', '#059669', '#047857',
    '#0000ff', '#3182ce', '#2b6cb0', '#2c5282',
    '#ffff00', '#ecc94b', '#d69e2e', '#b7791f',
    '#0070f2', '#1e88e5', '#00d4aa', '#0093c7',
    '#30914c', '#65b565', '#da1e28', '#ff4757',
    '#e26b0a', '#ff8c42', '#8e44ad', '#9b59b6',
    '#32363b', '#4a5568', '#6b7884', '#8b95a1',
    '#a0aec0', '#cbd5e0', '#e2e8f0', '#f7fafc',
    '#2563eb', '#3b82f6', '#06b6d4', '#0891b2',
    '#059669', '#10b981', '#dc2626', '#ef4444',
    '#ffa500', '#ff8c00', '#ed8936', '#dd6b20',
    '#800080', '#9b59b6', '#8e44ad', '#7c3aed'
];

/**
 * DOMContentLoaded: 페이지 로드 완료 후 초기화 순서
 * 1. initializeRangeSliders(): Ion Range Slider 초기화
 * 2. initializeButtons(): 저장/미리보기/테스트 버튼 이벤트 등록
 * 3. loadExistingConfig(): API에서 기존 설정 로드 → UI 반영
 */
document.addEventListener('DOMContentLoaded', function() {
    initializeRangeSliders();
    initializeButtons();
    loadExistingConfig();
});


/* ─── Range Sliders ──────────────────────────────────────── */

/**
 * Ion Range Slider 4개 초기화 (stage1~4)
 *
 * stage1: from_fixed=true (시작점=0 고정, 끝점만 조절)
 *   onChange: stage1.end_rate 업데이트 → stage2.start_rate 연쇄 갱신
 *
 * stage2: from/to 양방향 조절
 *   onChange: oldFrom/oldTo 비교하여 변경된 쪽만 연쇄 갱신
 *     - from 변경 → stage1 to 연쇄 갱신
 *     - to 변경 → stage3 start 연쇄 갱신
 *
 * stage3: from/to 양방향 조절
 *   onChange: oldFrom/oldTo 비교하여 변경된 쪽만 연쇄 갱신
 *     - from 변경 → stage2 to 연쇄 갱신
 *     - to 변경 → stage4 start 연쇄 갱신
 *
 * stage4: disable=true (사용자 조작 불가, to=100 고정)
 *   → stage3 to 변경 시 updateLinkedStageSlider(4, 'start', ...)로만 갱신됨
 */
function initializeRangeSliders() {
    // stage1 슬라이더: 시작점(0) 고정, 끝점만 조절
    $("#stage1Range").ionRangeSlider({
        type: "double",
        min: 0, max: 100,
        from: 0, to: rateColorConfig.stage1.end_rate,
        step: 1, postfix: "%", skin: "fiori",
        grid: true, grid_num: 10,
        from_fixed: true,  // 시작값(from=0) 고정
        onChange: function(data) {
            rateColorConfig.stage1.start_rate = 0;
            rateColorConfig.stage1.end_rate = data.to;
            // stage1 끝값 = stage2 시작값 (연쇄 갱신)
            rateColorConfig.stage2.start_rate = data.to;
            updateLinkedStageSlider(2, 'start', data.to);
            updateStageTitle(1, data.to);
            updateStageTitle(2, rateColorConfig.stage2.end_rate);
            updatePreview();
            validateStageSystem();
        }
    });

    // stage2 슬라이더: 양방향 조절, updateInProgress 플래그로 무한 루프 방지
    $("#stage2Range").ionRangeSlider({
        type: "double",
        min: 0, max: 100,
        from: rateColorConfig.stage2.start_rate,
        to: rateColorConfig.stage2.end_rate,
        step: 1, postfix: "%", skin: "fiori",
        grid: true, grid_num: 10,
        from_fixed: false,
        onChange: function(data) {
            // updateLinkedStageSlider에서 이 슬라이더를 갱신 중이면 재진입 방지
            if (updateInProgress.stage2) return;
            updateInProgress.stage2 = true;

            const oldFrom = rateColorConfig.stage2.start_rate;
            const oldTo   = rateColorConfig.stage2.end_rate;

            rateColorConfig.stage2.start_rate = data.from;
            rateColorConfig.stage2.end_rate   = data.to;

            // from이 변경되었으면 stage1 끝값도 같이 갱신 (경계 일치 유지)
            if (oldFrom !== data.from) {
                rateColorConfig.stage1.end_rate = data.from;
                updateLinkedStageSlider(1, 'end', data.from);
                updateStageTitle(1, data.from);
            }
            // to가 변경되었으면 stage3 시작값도 같이 갱신 (경계 일치 유지)
            if (oldTo !== data.to) {
                rateColorConfig.stage3.start_rate = data.to;
                updateLinkedStageSlider(3, 'start', data.to);
                updateStageTitle(3, rateColorConfig.stage3.end_rate);
            }

            updateStageTitle(2, data.to);
            updatePreview();
            validateStageSystem();
            updateInProgress.stage2 = false;
        }
    });

    // stage3 슬라이더: 양방향 조절, stage2/stage4와 연쇄
    $("#stage3Range").ionRangeSlider({
        type: "double",
        min: 0, max: 100,
        from: rateColorConfig.stage3.start_rate,
        to: rateColorConfig.stage3.end_rate,
        step: 1, postfix: "%", skin: "fiori",
        grid: true, grid_num: 10,
        from_fixed: false,
        onChange: function(data) {
            if (updateInProgress.stage3) return;
            updateInProgress.stage3 = true;

            const oldFrom = rateColorConfig.stage3.start_rate;
            const oldTo   = rateColorConfig.stage3.end_rate;

            rateColorConfig.stage3.start_rate = data.from;
            rateColorConfig.stage3.end_rate   = data.to;

            // from이 변경되었으면 stage2 끝값도 갱신
            if (oldFrom !== data.from) {
                rateColorConfig.stage2.end_rate = data.from;
                updateLinkedStageSlider(2, 'end', data.from);
                updateStageTitle(2, data.from);
            }
            // to가 변경되었으면 stage4 시작값도 갱신
            if (oldTo !== data.to) {
                rateColorConfig.stage4.start_rate = data.to;
                updateLinkedStageSlider(4, 'start', data.to);
                updateStageTitle(4, 100);  // stage4 끝값은 항상 100
            }

            updateStageTitle(3, data.to);
            updatePreview();
            validateStageSystem();
            updateInProgress.stage3 = false;
        }
    });

    // stage4 슬라이더: disable=true (사용자 조작 불가, to=100 고정)
    // stage3 to 변경 시 updateLinkedStageSlider(4, ...)로만 갱신됨
    $("#stage4Range").ionRangeSlider({
        type: "double",
        min: 0, max: 100,
        from: rateColorConfig.stage4.start_rate,
        to: 100,
        step: 1, postfix: "%", skin: "fiori",
        grid: true, grid_num: 10,
        disable: true  // 사용자 조작 불가 (stage3 to = stage4 from 이므로 자동 결정)
    });
}

/**
 * 연쇄 슬라이더 업데이트
 * 인접 슬라이더의 경계값을 프로그래밍 방식으로 갱신할 때 사용
 *
 * @param {number} stageNum  갱신할 슬라이더 번호 (1~4)
 * @param {string} position  'start'(from) 또는 'end'(to) 중 어느 쪽을 갱신할지
 * @param {number} value     새 경계값
 *
 * updateInProgress 플래그:
 *   - 이 함수가 슬라이더를 update()하면 해당 슬라이더의 onChange가 다시 발생함
 *   - 무한 루프 방지를 위해 updateInProgress[stageKey]=true 설정 후
 *     try/finally로 반드시 false로 복원
 *
 * stage4 특이사항:
 *   - disable=true이지만 위치 표시 갱신을 위해 update() 호출은 허용
 *   - 단, disable이고 stageNum!==4인 경우는 갱신 건너뜀 (예외 없음)
 */
function updateLinkedStageSlider(stageNum, position, value) {
    const slider = $(`#stage${stageNum}Range`).data("ionRangeSlider");
    if (!slider) return;
    // disable 슬라이더는 stage4만 허용 (stage1~3은 disable 상태면 건너뜀)
    if (slider.options.disable && stageNum !== 4) return;

    const stageKey = `stage${stageNum}`;
    // 이미 이 슬라이더를 갱신 중이면 재진입 방지
    if (updateInProgress[stageKey]) return;
    updateInProgress[stageKey] = true;

    try {
        const updateObj = {};
        if (position === 'start') {
            updateObj.from = value;
            // stage4는 to=100 고정, 나머지는 현재 end_rate 유지
            updateObj.to   = stageNum === 4 ? 100 : rateColorConfig[stageKey].end_rate;
        } else {
            updateObj.from = rateColorConfig[stageKey].start_rate;
            updateObj.to   = value;
        }
        slider.update(updateObj);
    } finally {
        // 예외 발생 시에도 반드시 플래그 해제
        updateInProgress[stageKey] = false;
    }
}


/* ─── Buttons ────────────────────────────────────────────── */

/**
 * 버튼 이벤트 등록
 * - saveBtn: 설정 저장 (validateStageSystem → API POST)
 * - previewBtn: 미리보기 모달 표시
 * - testBtn: 색상 시스템 테스트 (0~120% 구간 색상 확인)
 */
function initializeButtons() {
    document.getElementById('saveBtn').addEventListener('click', saveConfiguration);
    document.getElementById('previewBtn').addEventListener('click', showPreviewModal);
    document.getElementById('testBtn').addEventListener('click', testColorSystem);
}


/* ─── Stage Title ────────────────────────────────────────── */

/**
 * 단일 스테이지 타이틀 텍스트 업데이트
 * 슬라이더 값 변경 시 해당 단계의 [data-stage="N"] .rate-stage-title 요소를 갱신
 *
 * @param {number} stage     갱신할 스테이지 번호 (1~5)
 * @param {number} endValue  표시할 끝값 (stage5는 사용 안 함)
 *
 * 표시 형식:
 *   Stage 1: 0% < rate ≤ {endValue}%
 *   Stage 2: {start}% < rate ≤ {endValue}%
 *   Stage 3: {start}% < rate ≤ {endValue}%
 *   Stage 4: {start}% < rate ≤ 100%
 *   Stage 5: Over 100%
 */
function updateStageTitle(stage, endValue) {
    const el = document.querySelector(`[data-stage="${stage}"] .rate-stage-title`);
    if (!el) return;

    const cfg = rateColorConfig[`stage${stage}`];
    if (!cfg) return;

    switch (stage) {
        case 1: el.textContent = `Stage 1: 0% < rate \u2264 ${endValue}%`; break;
        case 2: el.textContent = `Stage 2: ${cfg.start_rate}% < rate \u2264 ${endValue}%`; break;
        case 3: el.textContent = `Stage 3: ${cfg.start_rate}% < rate \u2264 ${endValue}%`; break;
        case 4: el.textContent = `Stage 4: ${cfg.start_rate}% < rate \u2264 100%`; break;
        case 5: el.textContent = `Stage 5: Over 100%`; break;
    }
}

/**
 * 전체 스테이지(1~5) 타이틀 일괄 업데이트
 * loadExistingConfig 완료 후 UI 전체 동기화에 사용
 */
function updateAllStagesTitle() {
    for (let i = 1; i <= 5; i++) {
        updateStageTitle(i, rateColorConfig[`stage${i}`].end_rate);
    }
}


/* ─── Preview ────────────────────────────────────────────── */

/**
 * 우측 미리보기 패널 갱신 (#previewContainer)
 * rateColorConfig의 현재 상태를 fiori-card 형태로 동적 렌더링
 * 슬라이더 변경 또는 색상 변경 시 호출되어 실시간으로 반영
 *
 * 각 카드:
 *   - 좌측 테두리 색상 = stage 색상
 *   - 배경 색상 = stage 색상 + 10% 투명도 (${color}10 = HEX + 투명도 2자리)
 *   - 색상 박스 + 스테이지명 + 범위 텍스트 + HEX 코드 표시
 */
function updatePreview() {
    const container = document.getElementById('previewContainer');
    if (!container) return;

    container.innerHTML = '';  // 기존 카드 초기화

    for (let i = 1; i <= 5; i++) {
        const stage = rateColorConfig[`stage${i}`];
        let rangeText = '';
        // 각 스테이지별 범위 텍스트 포맷팅
        switch (i) {
            case 1: rangeText = `0% < rate \u2264 ${stage.end_rate}%`; break;
            case 2: rangeText = `${stage.start_rate}% < rate \u2264 ${stage.end_rate}%`; break;
            case 3: rangeText = `${stage.start_rate}% < rate \u2264 ${stage.end_rate}%`; break;
            case 4: rangeText = `${stage.start_rate}% < rate \u2264 100%`; break;
            case 5: rangeText = `Over 100%`; break;
        }

        const card = document.createElement('div');
        card.className = 'fiori-card';
        card.id = `preview-stage-${i}`;  // updateSingleStagePreview에서 직접 참조
        // 카드 좌측 테두리와 배경에 stage 색상 적용 (배경은 10% 투명도)
        card.style.cssText = `border-left: 4px solid ${stage.color}; background: ${stage.color}10;`;
        card.innerHTML = `
            <div class="fiori-card__content" style="padding: var(--sap-spacing-sm) var(--sap-spacing-md);">
                <div style="display:flex; align-items:center; gap:var(--sap-spacing-sm);">
                    <div class="stage-color-box" style="width:24px; height:24px; background:${stage.color};"></div>
                    <div>
                        <div style="font-weight:var(--sap-font-weight-medium);">Stage ${i}</div>
                        <div style="color:var(--sap-text-secondary); font-size:var(--sap-font-size-sm);">${rangeText}</div>
                    </div>
                </div>
                <div style="margin-top:4px; font-size:var(--sap-font-size-xs); color:var(--sap-text-tertiary);">${stage.color}</div>
            </div>
        `;
        container.appendChild(card);
    }
}

/**
 * 특정 스테이지의 미리보기 카드만 색상 갱신 (전체 재렌더 없이 부분 업데이트)
 * Spectrum 컬러피커의 change 이벤트에서 호출하여 성능 최적화
 *
 * @param {number} stageNum  갱신할 스테이지 번호 (1~5)
 */
function updateSingleStagePreview(stageNum) {
    const card = document.getElementById(`preview-stage-${stageNum}`);
    if (!card) return;

    const stage = rateColorConfig[`stage${stageNum}`];
    const colorBox = card.querySelector('.stage-color-box');
    if (colorBox) colorBox.style.background = stage.color;
    card.style.borderLeft = `4px solid ${stage.color}`;
    card.style.background = `${stage.color}10`;  // 10% 투명도 배경
}


/* ─── Validation ─────────────────────────────────────────── */

/**
 * 5단계 구간 시스템 유효성 검사
 * 저장 전 또는 슬라이더 변경 시 호출하여 데이터 정합성 검증
 *
 * 검증 규칙:
 *   1. stage1.start_rate === 0 (시작점 고정)
 *   2. stage4.end_rate !== 100이면 자동으로 100으로 교정
 *   3. stage5.start_rate = 100, stage5.end_rate = 999 강제 설정
 *   4. stage1~3: 현재 끝값 === 다음 시작값 (연속성 검사)
 *   5. stage1~3: start_rate < end_rate (단조증가 검사)
 *   6. stage4.end_rate === stage5.start_rate
 *
 * @returns {boolean} 유효하면 true, 오류 시 showAlert('error', ...) 후 false
 */
function validateStageSystem() {
    const s = rateColorConfig;

    // 규칙 1: stage1 시작값 고정 검사
    if (s.stage1.start_rate !== 0) {
        showAlert('error', 'Error: Stage 1 start value must be 0%.');
        return false;
    }
    // 규칙 2: stage4 끝값 자동 교정 (100 고정)
    if (s.stage4.end_rate !== 100) {
        rateColorConfig.stage4.end_rate = 100;
    }
    // 규칙 3: stage5 값 강제 설정 (100% 초과 구간)
    rateColorConfig.stage5.start_rate = 100;
    rateColorConfig.stage5.end_rate   = 999;

    // 규칙 4, 5: stage1~3 연속성 및 단조증가 검사
    for (let i = 1; i <= 3; i++) {
        const cur  = s[`stage${i}`];
        const next = s[`stage${i + 1}`];
        if (cur.end_rate !== next.start_rate) {
            showAlert('error', `Error: Stage ${i} end (${cur.end_rate}%) != Stage ${i + 1} start (${next.start_rate}%).`);
            return false;
        }
        if (cur.start_rate >= cur.end_rate) {
            showAlert('error', `Error: Stage ${i} start >= end.`);
            return false;
        }
    }
    // 규칙 6: stage4 끝값 === stage5 시작값
    if (s.stage4.end_rate !== s.stage5.start_rate) {
        showAlert('error', `Error: Stage 4 end (${s.stage4.end_rate}%) != Stage 5 start (${s.stage5.start_rate}%).`);
        return false;
    }
    return true;
}


/* ─── Save ───────────────────────────────────────────────── */

/**
 * 설정 저장
 * 1. validateStageSystem() 호출 → 유효성 검사 실패 시 중단
 * 2. saveBtn 비활성화 (중복 클릭 방지)
 * 3. fetch POST → proc/rate_color.php?action=save
 *    body: { config: rateColorConfig, validation: 'required', stage_count: 5 }
 * 4. 응답 code === '00': localStorage 백업 저장 후 성공 알림
 *    응답 code !== '00': 오류 메시지 표시
 * 5. 네트워크 오류 시 localStorage에만 저장 (로컬 백업)
 * 6. finally: saveBtn 원상복구
 *
 * API 응답 형식: { code: '00', msg: '...', data: { saved_count: 5 } }
 */
async function saveConfiguration() {
    if (!validateStageSystem()) return;

    const saveBtn = document.getElementById('saveBtn');
    const originalText = saveBtn.innerHTML;
    saveBtn.innerHTML = 'Saving...';
    saveBtn.disabled = true;

    try {
        const response = await fetch('./proc/rate_color.php?action=save', {
            method: 'POST',
            headers: { 'Content-Type': 'application/json' },
            body: JSON.stringify({ config: rateColorConfig, validation: 'required', stage_count: 5 })
        });

        if (!response.ok) throw new Error(`HTTP Error: ${response.status}`);

        const result = await response.json();

        if (result.code === '00') {
            showAlert('success', `Settings saved successfully. (${result.data.saved_count} stages)`);
            // 저장 성공 시 localStorage에도 백업 (오프라인/오류 시 복원용)
            localStorage.setItem('rateColorConfig', JSON.stringify(rateColorConfig));
        } else {
            throw new Error(result.msg || 'Save failed.');
        }
    } catch (error) {
        showAlert('error', `Save error: ${error.message}`);
        // API 오류 시 localStorage에만 저장 (로컬 백업)
        try {
            localStorage.setItem('rateColorConfig', JSON.stringify(rateColorConfig));
            showAlert('info', 'Settings saved as local backup.');
        } catch (e) { /* localStorage 접근 불가 무시 */ }
    } finally {
        // 성공/실패 관계없이 버튼 원상복구
        saveBtn.innerHTML = originalText;
        saveBtn.disabled = false;
    }
}


/* ─── Load ───────────────────────────────────────────────── */

/**
 * 기존 설정 로드 (페이지 초기화 시 1회 호출)
 * 1. fetch GET → proc/rate_color.php?action=config
 * 2. 응답 검증: 빈 응답·JSON 파싱 오류·형식 불일치 예외 처리
 * 3. 유효한 경우: rateColorConfig에 로드된 값 적용
 * 4. 오류 시: localStorage 백업 복원 시도 후 기본값 사용
 * 5. updateUIFromConfig(): 슬라이더 갱신
 *    initializeSpectrumColorPickers(): 컬러피커 초기화
 *    initializeStageColors(): input[type=color] 동기화
 *
 * 형식 검증: stage1~5 각각 start_rate/end_rate(number), color(string) 필수
 */
async function loadExistingConfig() {
    try {
        const response = await fetch('./proc/rate_color.php?action=config');
        if (!response.ok) throw new Error(`HTTP Error: ${response.status}`);

        // 빈 응답 체크 (DB 미초기화 상태 등)
        const text = await response.text();
        if (!text || text.trim() === '') throw new Error('Empty API response');

        let result;
        try { result = JSON.parse(text); }
        catch (e) { throw new Error('Invalid JSON response'); }

        if (result.code === '00' && result.data) {
            const loaded = result.data;
            const stages = ['stage1', 'stage2', 'stage3', 'stage4', 'stage5'];
            // 형식 검증: 5개 스테이지 모두 필수 필드 보유 확인
            const valid  = stages.every(k =>
                loaded[k] &&
                typeof loaded[k].start_rate === 'number' &&
                typeof loaded[k].end_rate   === 'number' &&
                typeof loaded[k].color      === 'string'
            );
            if (valid) {
                rateColorConfig = loaded;
            } else {
                throw new Error('Loaded settings invalid format');
            }
        } else {
            throw new Error(result.msg || 'Load failed');
        }

        // 로드 성공 시 UI 반영
        updateUIFromConfig();
        initializeSpectrumColorPickers();
        initializeStageColors();

    } catch (error) {
        showAlert('warning', `Settings load failed: ${error.message}. Using defaults.`);
        // API 실패 시 localStorage 백업에서 복원 시도
        try {
            const saved = localStorage.getItem('rateColorConfig');
            if (saved) {
                rateColorConfig = JSON.parse(saved);
                showAlert('info', 'Restored from local backup.');
            }
        } catch (e) { /* localStorage 파싱 오류 무시, 기본값 유지 */ }

        // 오류 시에도 UI는 초기화 (기본값 또는 localStorage 복원값 사용)
        updateUIFromConfig();
        initializeSpectrumColorPickers();
        initializeStageColors();
    }
}

/**
 * input[type=color] 동기화
 * stage1ColorPicker~stage5ColorPicker의 value를 rateColorConfig 색상으로 설정
 * (Spectrum 컬러피커와 별도로 존재하는 네이티브 color input 동기화용)
 */
function initializeStageColors() {
    for (let i = 1; i <= 5; i++) {
        const picker = document.getElementById(`stage${i}ColorPicker`);
        if (picker) picker.value = rateColorConfig[`stage${i}`].color;
    }
}

/**
 * 로드된 설정값으로 슬라이더 UI 전체 갱신
 * Ion Range Slider .update()로 from/to 재설정 후 타이틀과 미리보기 패널 동기화
 */
function updateUIFromConfig() {
    const sliders = {
        stage1: $("#stage1Range").data("ionRangeSlider"),
        stage2: $("#stage2Range").data("ionRangeSlider"),
        stage3: $("#stage3Range").data("ionRangeSlider"),
        stage4: $("#stage4Range").data("ionRangeSlider")
    };

    // 각 슬라이더를 현재 rateColorConfig 값으로 갱신
    if (sliders.stage1) sliders.stage1.update({ from: 0, to: rateColorConfig.stage1.end_rate });
    if (sliders.stage2) sliders.stage2.update({ from: rateColorConfig.stage2.start_rate, to: rateColorConfig.stage2.end_rate });
    if (sliders.stage3) sliders.stage3.update({ from: rateColorConfig.stage3.start_rate, to: rateColorConfig.stage3.end_rate });
    if (sliders.stage4) sliders.stage4.update({ from: rateColorConfig.stage4.start_rate, to: 100 });

    updateAllStagesTitle();  // 타이틀 텍스트 갱신
    updatePreview();          // 미리보기 패널 갱신
}


/* ─── Spectrum Color Pickers ─────────────────────────────── */

/**
 * Spectrum 컬러피커 초기화 (stage1~5)
 * 로드 성공/실패 후 항상 호출되어 최신 rateColorConfig 색상으로 초기화
 *
 * fioriPalette: SAP Fiori 계열 색상 팔레트 (8×4 배열)
 *
 * 핵심 주의사항: 클로저 변수 캡처
 *   const s = stage; // for 루프 변수를 클로저로 캡처
 *   → change 핸들러 내에서 s 사용 (루프 종료 후에도 올바른 stage 번호 유지)
 *   → stage를 직접 사용하면 모든 핸들러가 마지막 값(5)을 참조하는 버그 발생
 *
 * 설정:
 *   - showPalette + showInput: 팔레트 + 직접 입력 모두 지원
 *   - hideAfterPaletteSelect: 팔레트 색상 선택 시 자동으로 컬러피커 닫힘
 *   - localStorageKey: 최근 선택 색상 저장용 (각 stage별 독립 키)
 *   - preferredFormat: 'hex' (HEX 형식으로 반환)
 *
 * change 이벤트:
 *   - rateColorConfig[`stage${s}`].color 업데이트
 *   - updatePreview() 전체 패널 갱신
 *   - updateSingleStagePreview(s) 해당 카드만 색상 갱신 (성능 최적화)
 */
function initializeSpectrumColorPickers() {
    const fioriPalette = [
        ["#0070f2", "#1e88e5", "#2196f3", "#42a5f5"],
        ["#30914c", "#4caf50", "#66bb6a", "#81c784"],
        ["#da1e28", "#f44336", "#ef5350", "#e57373"],
        ["#e26b0a", "#ff9800", "#ffa726", "#ffb74d"],
        ["#32363b", "#4a5568", "#6b7884", "#8b95a1"],
        ["#a0aec0", "#cbd5e0", "#e2e8f0", "#f7fafc"],
        ["#800080", "#9b59b6", "#8e44ad", "#7c3aed"],
        ["#ffa500", "#ff8c00", "#ed8936", "#dd6b20"]
    ];

    for (let stage = 1; stage <= 5; stage++) {
        const el = $(`#stage${stage}ColorPicker`);
        if (!el.length) continue;

        // 재초기화 시 기존 Spectrum 인스턴스 제거 (중복 초기화 방지)
        try { el.spectrum('destroy'); } catch (e) { /* 첫 초기화 시 오류 무시 */ }

        const s = stage;  // 클로저 캡처: change 핸들러에서 올바른 stage 번호 사용
        el.spectrum({
            type: "component",
            showInput: true,         // 직접 HEX 입력 허용
            showInitial: true,       // 초기 색상 표시
            allowEmpty: false,       // 빈 값 불허
            showAlpha: false,        // 투명도 조절 숨김
            showPalette: true,       // 팔레트 표시
            showPaletteOnly: false,  // 팔레트 + 직접 선택 모두 허용
            showSelectionPalette: true,    // 최근 선택 색상 표시
            hideAfterPaletteSelect: true,  // 팔레트 선택 시 자동 닫힘
            clickoutFiresChange: true,     // 외부 클릭 시 변경 확정
            color: rateColorConfig[`stage${s}`].color,  // 현재 설정 색상으로 초기화
            palette: fioriPalette,
            localStorageKey: `spectrum.stage${s}`,  // stage별 독립 최근 선택 저장소
            maxSelectionSize: 10,    // 최근 선택 색상 최대 10개 보관
            preferredFormat: "hex",
            chooseText: "Select",
            cancelText: "Cancel",
            change: function(color) {
                const hex = color.toHexString();
                rateColorConfig[`stage${s}`].color = hex;  // s: 클로저로 캡처된 stage 번호
                updatePreview();
                updateSingleStagePreview(s);  // 전체 재렌더 대신 해당 카드만 갱신
            }
        });
    }
}


/* ─── Color Palette Modal (팔레트 직접 선택) ─────────────── */

/**
 * 팔레트 모달 열기 (특정 stage 색상 변경 시작)
 * Spectrum 컬러피커 대신 사용할 수 있는 간단한 팔레트 모달
 *
 * @param {number} stageNum  색상을 변경할 stage 번호
 */
function selectStageForColorChange(stageNum) {
    currentSelectedStage = stageNum;  // 전역 상태에 현재 선택 stage 저장
    showColorPaletteModal(stageNum);
}

/**
 * 팔레트 모달 표시
 * - 모달 타이틀에 "Stage N Color Selection" 표시
 * - createModalColorPalette()로 현재 색상 기준 팔레트 생성
 * - display:flex → classList.add('show') (CSS 트랜지션으로 fade-in)
 * - closeBtn/backdrop 이벤트: cloneNode()로 기존 리스너 제거 후 새로 등록 (중복 방지)
 */
function showColorPaletteModal(stageNum) {
    const currentColor = rateColorConfig[`stage${stageNum}`].color;
    const modal = document.getElementById('colorPaletteModal');
    const title = modal.querySelector('.fiori-card__title');
    title.textContent = `Stage ${stageNum} Color Selection`;

    createModalColorPalette(stageNum, currentColor);

    modal.style.display = 'flex';
    setTimeout(() => modal.classList.add('show'), 10);  // CSS 트랜지션 트리거

    // cloneNode로 기존 이벤트 리스너 제거 후 새로 등록 (중복 클릭 방지)
    const closeBtn  = modal.querySelector('.modal-close');
    const backdrop  = modal.querySelector('.fiori-modal__backdrop');
    closeBtn.replaceWith(closeBtn.cloneNode(true));
    backdrop.replaceWith(backdrop.cloneNode(true));
    modal.querySelector('.modal-close').addEventListener('click', closeColorPaletteModal);
    modal.querySelector('.fiori-modal__backdrop').addEventListener('click', closeColorPaletteModal);
}

/**
 * 팔레트 모달 닫기
 * classList.remove('show') → 300ms 후 display:none (CSS 트랜지션으로 fade-out)
 */
function closeColorPaletteModal() {
    const modal = document.getElementById('colorPaletteModal');
    modal.classList.remove('show');
    setTimeout(() => { modal.style.display = 'none'; }, 300);
    currentSelectedStage = null;  // 선택 stage 초기화
}

/**
 * 팔레트 모달 내 색상 아이템 렌더링 및 선택 이벤트 등록
 * colorPalette 배열의 각 색상을 .modal-color-item 스팬으로 렌더링
 * 현재 색상과 일치하면 'selected' 클래스 추가 (강조 표시)
 *
 * 이벤트 위임(delegation):
 *   container에 단일 click 리스너 등록 → .modal-color-item 클릭 시
 *   → rateColorConfig 업데이트 + 미리보기 갱신 + 모달 닫기
 *
 * @param {number} stageNum     색상을 변경할 stage 번호
 * @param {string} currentColor 현재 선택된 색상 (HEX)
 */
function createModalColorPalette(stageNum, currentColor) {
    const container = document.getElementById('modalColorPalette');
    if (!container) return;

    container.innerHTML = colorPalette.map(color => `
        <span class="modal-color-item ${color === currentColor ? 'selected' : ''}"
              title="${color}" data-color="${color}">
            <span class="color-inner" style="background-color:${color};"></span>
        </span>
    `).join('');

    container.addEventListener('click', function(e) {
        const item = e.target.closest('.modal-color-item');
        if (!item) return;
        const selected = item.dataset.color;
        rateColorConfig[`stage${stageNum}`].color = selected;
        updatePreview();
        updateSingleStagePreview(stageNum);
        closeColorPaletteModal();
    });
}


/* ─── Preview Modal ──────────────────────────────────────── */

/**
 * 미리보기 모달 표시
 * updateModalPreview()로 현재 rateColorConfig를 모달 내에 렌더링
 * closeBtn/backdrop: cloneNode로 중복 리스너 방지
 */
function showPreviewModal() {
    const modal = document.getElementById('previewModal');
    if (!modal) return;

    updateModalPreview();  // 모달 열기 전 최신 데이터로 갱신

    modal.style.display = 'flex';
    setTimeout(() => modal.classList.add('show'), 10);

    // 중복 이벤트 방지를 위해 기존 리스너 제거 후 재등록
    const closeBtn = modal.querySelector('.modal-close');
    const backdrop = modal.querySelector('.fiori-modal__backdrop');
    closeBtn.replaceWith(closeBtn.cloneNode(true));
    backdrop.replaceWith(backdrop.cloneNode(true));
    modal.querySelector('.modal-close').addEventListener('click', closePreviewModal);
    modal.querySelector('.fiori-modal__backdrop').addEventListener('click', closePreviewModal);
}

/**
 * 미리보기 모달 닫기
 */
function closePreviewModal() {
    const modal = document.getElementById('previewModal');
    modal.classList.remove('show');
    setTimeout(() => { modal.style.display = 'none'; }, 300);
}

/**
 * 미리보기 모달 내용 갱신 (#modalPreviewContainer)
 * rateColorConfig의 5단계를 각각 색상 박스 + 범위 텍스트 + HEX 코드로 표시
 * 배경: ${stage.color}08 (8% 투명도, 우측 패널보다 더 투명하게)
 */
function updateModalPreview() {
    const container = document.getElementById('modalPreviewContainer');
    if (!container) return;

    container.innerHTML = '';

    Object.keys(rateColorConfig).forEach(key => {
        const stage = rateColorConfig[key];
        const num   = key.replace('stage', '');
        let rangeText = '';
        if (num === '1')      rangeText = `0% ~ ${stage.end_rate}%`;
        else if (num === '5') rangeText = `Over 100%`;
        else                  rangeText = `${stage.start_rate}% < rate \u2264 ${stage.end_rate}%`;

        const item = document.createElement('div');
        item.style.cssText = `
            display:flex; align-items:center; gap:var(--sap-spacing-md);
            padding:var(--sap-spacing-md);
            border:1px solid var(--sap-border-neutral);
            border-radius:var(--sap-radius-sm);
            margin-bottom:var(--sap-spacing-sm);
            background:${stage.color}08;
        `;
        item.innerHTML = `
            <div style="width:40px; height:40px; background:${stage.color}; border-radius:var(--sap-radius-sm); border:1px solid var(--sap-border-neutral); flex-shrink:0;"></div>
            <div>
                <div style="font-weight:var(--sap-font-weight-medium);">Stage ${num}: ${rangeText}</div>
                <div style="color:var(--sap-text-secondary); font-size:var(--sap-font-size-sm);">Color: ${stage.color}</div>
            </div>
        `;
        container.appendChild(item);
    });
}


/* ─── Test ───────────────────────────────────────────────── */

/**
 * 색상 시스템 테스트
 * 0~120% 구간의 대표 rate 값에 대해 getRateColor()를 호출하여
 * 각 값에 매핑되는 색상을 확인함 (개발/디버깅용)
 */
function testColorSystem() {
    const rates   = [0, 25, 50, 65, 80, 95, 100, 105, 120];
    const results = rates.map(r => ({ rate: r, color: getRateColor(r) }));
    showAlert('info', `Test completed: ${results.length} values verified`);
}

/**
 * rate 값에 해당하는 색상 반환
 * rateColorConfig의 5단계 범위와 비교하여 해당 단계의 색상을 반환
 *
 * 범위 비교 로직 (하한 초과, 상한 이하):
 *   stage1: start_rate < rate ≤ stage1.end_rate
 *   stage2: stage1.end_rate < rate ≤ stage2.end_rate
 *   stage3: stage2.end_rate < rate ≤ stage3.end_rate
 *   stage4: stage3.end_rate < rate ≤ 100
 *   stage5: rate > 100
 *   기본값: stage1 색상 (rate=0 또는 범위 미매칭 시)
 *
 * @param {number} rate  OEE 비율 값 (0~100+)
 * @returns {string}     HEX 색상 코드
 */
function getRateColor(rate) {
    const s = rateColorConfig;
    if (rate > s.stage1.start_rate && rate <= s.stage1.end_rate) return s.stage1.color;
    if (rate > s.stage1.end_rate   && rate <= s.stage2.end_rate) return s.stage2.color;
    if (rate > s.stage2.end_rate   && rate <= s.stage3.end_rate) return s.stage3.color;
    if (rate > s.stage3.end_rate   && rate <= 100)               return s.stage4.color;
    if (rate > 100)                                               return s.stage5.color;
    return s.stage1.color;  // 기본값: rate=0이거나 범위 미매칭
}


/* ─── Alert ──────────────────────────────────────────────── */

/**
 * 알림 CSS 애니메이션 정의 (동적 주입)
 * - slideInRight: 우측에서 슬라이드 인 (0.3s)
 * - slideOutRight: 우측으로 슬라이드 아웃 (0.3s)
 * 스크립트 로드 시 한 번만 실행되어 document.head에 주입
 */
const _alertStyle = document.createElement('style');
_alertStyle.textContent = `
@keyframes slideInRight  { from { transform:translateX(100%); opacity:0; } to { transform:translateX(0); opacity:1; } }
@keyframes slideOutRight { from { transform:translateX(0); opacity:1; }   to { transform:translateX(100%); opacity:0; } }
`;
document.head.appendChild(_alertStyle);

/**
 * 슬라이딩 알림 메시지 표시
 * - type: 'success'|'error'|'warning'|'info' → fiori-alert--{type} 클래스
 * - 동시에 1개만 표시 (기존 알림 제거 후 새 알림 표시)
 * - 3초 후 slideOutRight 애니메이션과 함께 자동 제거
 * - 이미 제거된 경우(el.parentNode 없음) setTimeout 콜백 안전 처리
 *
 * @param {string} type    알림 유형 ('success'|'error'|'warning'|'info')
 * @param {string} message 표시할 메시지 텍스트
 */
function showAlert(type, message) {
    // 기존 알림 즉시 제거 (동시에 1개만 표시)
    const existing = document.querySelector('.rate-color-alert');
    if (existing) existing.remove();

    const el = document.createElement('div');
    el.className = `fiori-alert fiori-alert--${type} rate-color-alert`;
    el.style.animation = 'slideInRight 0.3s ease';
    el.textContent = message;
    document.body.appendChild(el);

    // 3초 후 슬라이드 아웃 후 DOM 제거
    setTimeout(() => {
        if (!el.parentNode) return;  // 이미 수동으로 제거된 경우 건너뜀
        el.style.animation = 'slideOutRight 0.3s ease';
        setTimeout(() => el.remove(), 300);  // 애니메이션 완료 후 제거
    }, 3000);
}
