/**
 * Rate Color Management JavaScript
 * SAP Fiori Style Rate Color Management System
 */

// 전역 변수 - 5단계 시스템 (Stage 5 추가)
let rateColorConfig = {
  stage1: { start_rate: 0, end_rate: 25, color: '#6b7884' },      // Stage 1: 0% < rate ≤ 상한값
  stage2: { start_rate: 25, end_rate: 50, color: '#da1e28' },     // Stage 2: Stage1 상한값 < rate ≤ 50%
  stage3: { start_rate: 50, end_rate: 80, color: '#e26b0a' },     // Stage 3: 50% < rate ≤ 80%
  stage4: { start_rate: 80, end_rate: 100, color: '#30914c' },    // Stage 4: 80% < rate ≤ 100%
  stage5: { start_rate: 100, end_rate: 999, color: '#0070f2' }    // Stage 5: 100% 초과 (슬라이더 없음)
};

// 현재 선택된 단계
let currentSelectedStage = null;

// 업데이트 상태 추적 (딜레이 해결용)
let updateInProgress = {
  stage1: false,
  stage2: false,
  stage3: false,
  stage4: false
};

// 48개 SAP Fiori 호환 컬러 팔레트
const colorPalette = [
  // Primary 원색 (16개) - 빨강, 초록, 파랑, 노랑, 주황, 보라 계열
  '#ff0000', '#e53e3e', '#dc2626', '#b91c1c',
  '#00ff00', '#38a169', '#059669', '#047857',
  '#0000ff', '#3182ce', '#2b6cb0', '#2c5282',
  '#ffff00', '#ecc94b', '#d69e2e', '#b7791f',
  
  // SAP 브랜드 색상 (4개)
  '#0070f2', '#1e88e5', '#00d4aa', '#0093c7',
  
  // 상태 색상 (8개)
  '#30914c', '#65b565', '#da1e28', '#ff4757',
  '#e26b0a', '#ff8c42', '#8e44ad', '#9b59b6',
  
  // 회색 계열 (8개)
  '#32363b', '#4a5568', '#6b7884', '#8b95a1',
  '#a0aec0', '#cbd5e0', '#e2e8f0', '#f7fafc',
  
  // 보조 색상 (8개)
  '#2563eb', '#3b82f6', '#06b6d4', '#0891b2',
  '#059669', '#10b981', '#dc2626', '#ef4444',
  
  // 액센트 색상 (8개) - 주황, 보라 확장
  '#ffa500', '#ff8c00', '#ed8936', '#dd6b20',
  '#800080', '#9b59b6', '#8e44ad', '#7c3aed'
];

/**
 * 페이지 초기화
 */
document.addEventListener('DOMContentLoaded', function() {
  
  // Ion Range Slider 초기화
  initializeRangeSliders();
  
  // 버튼 이벤트 바인딩
  initializeButtons();
  
  // 기존 설정 로드 (Spectrum 초기화보다 먼저)
  loadExistingConfig();
  
});

/**
 * Ion Range Slider 초기화 - 5단계 고정 시스템
 */
function initializeRangeSliders() {
  
  // Stage 1: 0% < rate ≤ 상한값 (범위 시각화, 상한값만 조정 가능)
  $("#stage1Range").ionRangeSlider({
    type: "double",
    min: 0,
    max: 100,
    from: 0,  // 시작점 고정
    to: rateColorConfig.stage1.end_rate,  // 상한값 조정 가능
    step: 1,
    postfix: "%",
    skin: "fiori",
    grid: true,
    grid_num: 10,
    from_fixed: true,  // 시작점 고정
    onChange: function (data) {
      
      // Stage 1 업데이트
      rateColorConfig.stage1.start_rate = 0;  // 항상 0%에서 시작
      rateColorConfig.stage1.end_rate = data.to;
      
      // Stage 2 연동 업데이트 (Stage 2 start_rate = Stage 1 end_rate)
      rateColorConfig.stage2.start_rate = data.to;
      updateLinkedStageSlider(2, 'start', data.to);
      
      // 역방향 연동: Stage 1 end_rate 변경 시 Stage 2 start_rate도 자동 업데이트
      // (이미 위에서 처리됨)
      
      // UI 업데이트 (범위 변경만, 색상은 건드리지 않음)
      updateStageTitle(1, data.to);
      updateStageTitle(2, rateColorConfig.stage2.end_rate);
      updatePreview(); // 하단 미리보기만 업데이트
      validateStageSystem();
    }
  });
  
  // Stage 2: Stage1 상한값 < rate ≤ 상한값 (범위 시각화, 상한값만 조정 가능)
  $("#stage2Range").ionRangeSlider({
    type: "double",
    min: 0,
    max: 100,
    from: rateColorConfig.stage2.start_rate,  // Stage 1 end_rate
    to: rateColorConfig.stage2.end_rate,      // 상한값 조정 가능
    step: 1,
    postfix: "%",
    skin: "fiori",
    grid: true,
    grid_num: 10,
    from_fixed: false,  // 시작점 조정 가능하도록 변경
    onChange: function (data) {
      if (updateInProgress.stage2) {
        return;
      }
      updateInProgress.stage2 = true;
      
      // Stage 2 업데이트
      const oldFromValue = rateColorConfig.stage2.start_rate;
      const oldToValue = rateColorConfig.stage2.end_rate;
      
      rateColorConfig.stage2.start_rate = data.from;
      rateColorConfig.stage2.end_rate = data.to;
      
      if (oldFromValue !== data.from) {
        rateColorConfig.stage1.end_rate = data.from;
        updateLinkedStageSlider(1, 'end', data.from);
        updateStageTitle(1, data.from);
      }
      
      if (oldToValue !== data.to) {
        rateColorConfig.stage3.start_rate = data.to;
        updateLinkedStageSlider(3, 'start', data.to);
        updateStageTitle(3, rateColorConfig.stage3.end_rate);
      }
      
      // UI 업데이트
      updateStageTitle(2, data.to);
      updatePreview();
      validateStageSystem();
      
      // 업데이트 완료 플래그 해제
      updateInProgress.stage2 = false;
    }
  });
  
  // Stage 3: Stage2 상한값 < rate ≤ 상한값 (범위 시각화, 상한값만 조정 가능)
  $("#stage3Range").ionRangeSlider({
    type: "double",
    min: 0,
    max: 100,
    from: rateColorConfig.stage3.start_rate,  // Stage 2 end_rate
    to: rateColorConfig.stage3.end_rate,      // 상한값 조정 가능
    step: 1,
    postfix: "%",
    skin: "fiori",
    grid: true,
    grid_num: 10,
    from_fixed: false,  // 시작점 조정 가능하도록 변경
    onChange: function (data) {
      console.log(`🎯 Stage 3 범위 변경: ${data.from}% < rate ≤ ${data.to}%`);
      
      // 업데이트 진행 중 체크 (즉시 반응형)
      if (updateInProgress.stage3) {
        console.log('🔄 Stage 3 업데이트 진행 중이므로 스킵');
        return;
      }
      updateInProgress.stage3 = true;
      
      // Stage 3 업데이트
      const oldFromValue = rateColorConfig.stage3.start_rate;
      const oldToValue = rateColorConfig.stage3.end_rate;
      
      rateColorConfig.stage3.start_rate = data.from;
      rateColorConfig.stage3.end_rate = data.to;
      
      // from 값 변경 시: Stage 2의 to 업데이트
      if (oldFromValue !== data.from) {
        console.log(`🔗 Stage 3 from 변경: ${oldFromValue}% → ${data.from}% (Stage 2 to 연동)`);
        rateColorConfig.stage2.end_rate = data.from;
        updateLinkedStageSlider(2, 'end', data.from);
        updateStageTitle(2, data.from);
      }
      
      // to 값 변경 시: Stage 4의 from 업데이트
      if (oldToValue !== data.to) {
        console.log(`🔗 Stage 3 to 변경: ${oldToValue}% → ${data.to}% (Stage 4 from 연동)`);
        rateColorConfig.stage4.start_rate = data.to;
        updateLinkedStageSlider(4, 'start', data.to);
        updateStageTitle(4, 100);
      }
      
      // UI 업데이트
      updateStageTitle(3, data.to);
      updatePreview();
      validateStageSystem();
      
      // 업데이트 완료 플래그 해제
      updateInProgress.stage3 = false;
    }
  });
  
  // Stage 4: Stage3 상한값 < rate ≤ 100% (범위 시각화, 조정 불가)
  $("#stage4Range").ionRangeSlider({
    type: "double",
    min: 0,
    max: 100,
    from: rateColorConfig.stage4.start_rate,  // Stage 3 end_rate
    to: 100,  // 100% 고정
    step: 1,
    postfix: "%",
    skin: "fiori",
    grid: true,
    grid_num: 10,
    disable: true  // 전체 비활성화 - 시각화만
  });
  
  // Stage 5: 100% 초과, 슬라이더 없음 (색상 팔레트만 사용)
  console.log('📌 Stage 5는 100% 초과 고정값으로 슬라이더 없음');
  
  console.log('✅ 5단계 Range Slider 초기화 완료 (Stage 5는 슬라이더 없음)');
}

/**
 * 연동된 스테이지 슬라이더 업데이트 (4단계 시스템) - 즉시 업데이트
 */
function updateLinkedStageSlider(stageNum, position, value) {
  const sliderId = `#stage${stageNum}Range`;
  const slider = $(sliderId).data("ionRangeSlider");
  
  if (!slider) {
    console.warn(`⚠️ ${sliderId} 슬라이더를 찾을 수 없음`);
    return;
  }
  
  // 비활성화된 슬라이더도 시각적 업데이트는 수행 (Stage 4의 경우)
  if (slider.options.disable && stageNum !== 4) {
    console.log(`🔒 Stage ${stageNum} 슬라이더는 비활성화되어 있어 업데이트 스킵`);
    return;
  }
  
  // 무한 루프 방지를 위한 즉시 플래그
  const stageKey = `stage${stageNum}`;
  if (updateInProgress[stageKey]) {
    console.log(`🔄 ${sliderId} 업데이트 중이므로 스킵`);
    return;
  }
  updateInProgress[stageKey] = true;
  
  try {
    if (stageNum >= 1 && stageNum <= 4) {
      // Stage 1~4: double 슬라이더
      const updateObj = {};
      if (position === 'start') {
        console.log(`🔗 Stage ${stageNum} 연동 업데이트: 시작값 = ${value}%`);
        updateObj.from = value;
        updateObj.to = rateColorConfig[`stage${stageNum}`].end_rate;
        if (stageNum === 4) {
          updateObj.to = 100;  // Stage 4 상한값은 항상 100%
        }
      } else if (position === 'end') {
        console.log(`🔗 Stage ${stageNum} 연동 업데이트: 상한값 = ${value}%`);
        updateObj.from = rateColorConfig[`stage${stageNum}`].start_rate;
        updateObj.to = value;
      }
      
      // 즉시 업데이트 (딜레이 없음)
      slider.update(updateObj);
    }
    
  } catch (error) {
    console.error(`❌ Stage ${stageNum} 슬라이더 업데이트 오류:`, error);
  } finally {
    // 즉시 플래그 해제 (딜레이 제거)
    updateInProgress[stageKey] = false;
  }
}


/**
 * 단계 선택 시스템 초기화
 */
function initializeStageSelection() {
  
  // 각 단계의 컬러 미리보기 클릭 이벤트
  document.querySelectorAll('.selected-color-preview').forEach(preview => {
    const stageCard = preview.closest('.rate-stage-card');
    const stageNum = stageCard.dataset.stage;
    
    preview.addEventListener('click', function(e) {
      e.preventDefault();
      selectStageForColorChange(stageNum);
    });
  });
  
  console.log('✅ 단계 선택 시스템 초기화 완료');
}

/**
 * 단계 선택 및 모달 팔레트 표시
 */
function selectStageForColorChange(stageNum) {
  console.log(`🎯 Stage ${stageNum} 선택됨 - 모달 팔레트 표시`);
  
  currentSelectedStage = stageNum;
  showColorPaletteModal(stageNum);
}

/**
 * 컬러 팔레트 모달 표시
 */
function showColorPaletteModal(stageNum) {
  console.log(`🎨 Stage ${stageNum} 컬러 팔레트 모달 표시`);
  
  const currentColor = rateColorConfig[`stage${stageNum}`].color;
  
  // 기존 모달 사용
  const modal = document.getElementById('colorPaletteModal');
  const modalTitle = modal.querySelector('.fiori-card__title');
  modalTitle.textContent = `🎨 Stage ${stageNum} Color Selection`;
  
  // 모달 팔레트 생성
  createModalColorPalette(stageNum, currentColor);
  
  // 모달 표시
  modal.style.display = 'flex';
  setTimeout(() => {
    modal.classList.add('show');
  }, 10);
  
  // 닫기 이벤트 (기존 이벤트 제거 후 재등록)
  const closeBtn = modal.querySelector('.modal-close');
  const backdrop = modal.querySelector('.fiori-modal__backdrop');
  
  // 기존 이벤트 리스너 제거
  closeBtn.replaceWith(closeBtn.cloneNode(true));
  backdrop.replaceWith(backdrop.cloneNode(true));
  
  // 새 이벤트 리스너 추가
  modal.querySelector('.modal-close').addEventListener('click', () => {
    closeColorPaletteModal();
  });
  
  modal.querySelector('.fiori-modal__backdrop').addEventListener('click', () => {
    closeColorPaletteModal();
  });
}

function closeColorPaletteModal() {
  const modal = document.getElementById('colorPaletteModal');
  modal.classList.remove('show');
  setTimeout(() => {
    modal.style.display = 'none';
  }, 300);
  clearStageSelection();
}

/**
 * 모달 내 컬러 팔레트 생성
 */
function createModalColorPalette(stageNum, currentColor) {
  const paletteContainer = document.getElementById('modalColorPalette');
  if (!paletteContainer) return;
  
  // 32개 색상으로 팔레트 생성
  paletteContainer.innerHTML = colorPalette.map(color => `
    <span class="modal-color-item ${color === currentColor ? 'selected' : ''}" 
          title="${color}" 
          data-color="${color}">
      <span class="color-inner" style="background-color: ${color};"></span>
    </span>
  `).join('');
  
  // 색상 선택 이벤트
  paletteContainer.addEventListener('click', function(e) {
    const colorItem = e.target.closest('.modal-color-item');
    if (!colorItem) return;
    
    const selectedColor = colorItem.dataset.color;
    console.log(`Stage ${stageNum} 색상 선택: ${selectedColor}`);
    
    // 설정 업데이트
    rateColorConfig[`stage${stageNum}`].color = selectedColor;
    
    // UI 업데이트 (특정 stage만 업데이트)
    console.log(`🔧 Stage ${stageNum} 색상 변경 시작 - 다른 Stage는 영향받지 않아야 함`);
    updateSelectedColorPreview(stageNum, selectedColor);
    updateSingleStagePreview(stageNum);
    console.log(`✅ Stage ${stageNum} 색상 변경 완료 - 다른 Stage 확인 필요`);
    
    // 모달 닫기
    closeColorPaletteModal();
  });
}

/**
 * 단계 선택 해제
 */
function clearStageSelection() {
  currentSelectedStage = null;
}


/**
 * 버튼 이벤트 초기화
 */
function initializeButtons() {
  console.log('🔘 버튼 이벤트 바인딩 중...');
  
  // 저장 버튼
  document.getElementById('saveBtn').addEventListener('click', saveConfiguration);
  
  
  // 미리보기 버튼
  document.getElementById('previewBtn').addEventListener('click', showPreviewModal);
  
  // 테스트 버튼
  document.getElementById('testBtn').addEventListener('click', testColorSystem);
  
  console.log('✅ 버튼 이벤트 바인딩 완료');
}

/**
 * 단계별 제목 업데이트 - 5단계 고정 시스템
 */
function updateStageTitle(stage, endValue) {
  const titleElement = document.querySelector(`[data-stage="${stage}"] .rate-stage-title`);
  if (!titleElement) return;
  
  const config = rateColorConfig[`stage${stage}`];
  if (!config) return;
  
  switch (stage) {
    case 1:
      // Stage 1: 0% < rate ≤ end_rate
      titleElement.textContent = `Stage 1: 0% < rate ≤ ${endValue}%`;
      break;
    case 2:
      // Stage 2: start_rate < rate ≤ end_rate
      titleElement.textContent = `Stage 2: ${config.start_rate}% < rate ≤ ${endValue}%`;
      break;
    case 3:
      // Stage 3: start_rate < rate ≤ end_rate
      titleElement.textContent = `Stage 3: ${config.start_rate}% < rate ≤ ${endValue}%`;
      break;
    case 4:
      // Stage 4: start_rate < rate ≤ 100% (고정)
      titleElement.textContent = `Stage 4: ${config.start_rate}% < rate ≤ 100%`;
      break;
    case 5:
      // Stage 5: 100% 초과 (고정)
      titleElement.textContent = `Stage 5: Over 100%`;
      break;
    default:
      console.warn(`⚠️ 알 수 없는 stage: ${stage}`);
  }
}

/**
 * 선택된 색상 미리보기 업데이트 - data 속성 기반 격리 방식
 */
function updateSelectedColorPreview(stage, color) {
  console.log(`🎨 DATA 속성 기반 색상 업데이트 - Stage ${stage}: ${color}`);
  
  try {
    // data 속성을 사용하여 정확한 요소 선택
    const targetColorBox = document.querySelector(`[data-stage-color="${stage}"]`);
    const targetColorCode = document.querySelector(`[data-stage-code="${stage}"]`);
    
    console.log(`🔍 요소 선택 결과:`, {
      targetColorBox: targetColorBox ? '✅ 발견' : '❌ 없음',
      targetColorCode: targetColorCode ? '✅ 발견' : '❌ 없음',
      targetParent: targetColorBox?.parentElement?.parentElement?.getAttribute('data-stage')
    });
    
    if (targetColorBox && targetColorCode) {
      // 1. BEFORE 상태 저장 (다른 Stage들)
      const otherStagesState = {};
      for (let i = 1; i <= 5; i++) {
        if (i != stage) {
          const otherBox = document.querySelector(`[data-stage-color="${i}"]`);
          if (otherBox) {
            otherStagesState[i] = getComputedStyle(otherBox).backgroundColor;
          }
        }
      }
      
      // 2. 대상 Stage 색상 업데이트
      targetColorBox.style.cssText = '';
      targetColorBox.style.backgroundColor = color;
      targetColorBox.style.setProperty('background-color', color, 'important');
      targetColorCode.textContent = color;
      
      console.log(`✅ Stage ${stage} 색상 적용 완료: ${color}`);
      
      // 3. 즉시 다른 Stage들 검증
      setTimeout(() => {
        console.log('🔍 다른 Stage들 상태 검증:');
        let anyChanged = false;
        
        for (let i = 1; i <= 5; i++) {
          if (i != stage) {
            const otherBox = document.querySelector(`[data-stage-color="${i}"]`);
            if (otherBox) {
              const currentColor = getComputedStyle(otherBox).backgroundColor;
              console.log(`📍 Stage ${i}: ${currentColor} (원래: ${otherStagesState[i]})`);
              
              if (currentColor !== otherStagesState[i]) {
                console.log(`🚨 Stage ${i} 색상이 바뀜! ${otherStagesState[i]} → ${currentColor}`);
                anyChanged = true;
              }
            }
          }
        }
        
        if (!anyChanged) {
          console.log('✅ 모든 다른 Stage 색상이 정상 유지됨');
        }
      }, 50);
      
    } else {
      console.error(`❌ Stage ${stage} data 속성 요소를 찾을 수 없음`);
    }
    
    // 설정 업데이트
    rateColorConfig[`stage${stage}`].color = color;
    
  } catch (error) {
    console.error('❌ 색상 업데이트 오류:', error);
  }
}

/**
 * 5단계 시스템 유효성 검사 (Stage 5 포함)
 */
function validateStageSystem() {
  const stages = rateColorConfig;
  
  // Stage 1: 0%에서 시작 검증
  if (stages.stage1.start_rate !== 0) {
    showAlert('error', '❌ Error: Stage 1 start value must be 0%.');
    return false;
  }
  
  // Stage 4: end_rate 최대값 100% 검증
  if (stages.stage4.end_rate !== 100) {
    showAlert('warning', '⚠️ Warning: Stage 4 end value is not 100%. Adjusting to 100%.');
    rateColorConfig.stage4.end_rate = 100;
  }
  
  // Stage 5: 100% 초과 검증 및 자동 설정
  if (stages.stage5.start_rate !== 100 || stages.stage5.end_rate !== 999) {
    console.log('🔧 Stage 5 값 자동 수정: 100% ~ 999% (100% 초과 구간)');
    rateColorConfig.stage5.start_rate = 100;
    rateColorConfig.stage5.end_rate = 999;
  }
  
  // 단계 간 연속성 검증 (Stage 1~4)
  for (let i = 1; i <= 3; i++) {
    const currentStage = stages[`stage${i}`];
    const nextStage = stages[`stage${i + 1}`];
    
    // 현재 단계의 end_rate와 다음 단계의 start_rate가 일치해야 함
    if (currentStage.end_rate !== nextStage.start_rate) {
      showAlert('error', `❌ Error: Stage ${i} end value (${currentStage.end_rate}%) does not match Stage ${i + 1} start value (${nextStage.start_rate}%).`);
      return false;
    }
    
    // 단계 내에서 start_rate < end_rate 검증
    if (currentStage.start_rate >= currentStage.end_rate) {
      showAlert('error', `❌ Error: Stage ${i} start value (${currentStage.start_rate}%) is greater than or equal to end value (${currentStage.end_rate}%).`);
      return false;
    }
  }
  
  // Stage 4와 Stage 5 연결 검증
  if (stages.stage4.end_rate !== stages.stage5.start_rate) {
    showAlert('error', `❌ Error: Stage 4 end value (${stages.stage4.end_rate}%) does not match Stage 5 start value (${stages.stage5.start_rate}%).`);
    return false;
  }
  
  console.log('✅ 5단계 시스템 유효성 검사 통과');
  return true;
}

/**
 * 5단계 시스템 전체 미리보기 업데이트 (Stage 5 포함) - 하단 미리보기만 업데이트
 */
function updatePreview() {
  console.log('📊 updatePreview 호출 - 하단 미리보기만 업데이트 (헤더 색상박스는 건드리지 않음)');
  const previewContainer = document.getElementById('previewContainer');
  if (!previewContainer) return;
  
  previewContainer.innerHTML = '';
  
  // 5단계 순서대로 미리보기 생성
  for (let i = 1; i <= 5; i++) {
    const stage = rateColorConfig[`stage${i}`];
    console.log(`📊 미리보기 카드 생성: Stage ${i} - ${stage.color}`);
    
    const previewCard = document.createElement('div');
    previewCard.className = 'fiori-card';
    previewCard.id = `preview-stage-${i}`;
    previewCard.style.cssText = `
      border-left: 4px solid ${stage.color};
      background: ${stage.color}10;
    `;
    
    // 단계별 범위 텍스트 생성
    let rangeText = '';
    switch (i) {
      case 1:
        rangeText = `0% < rate ≤ ${stage.end_rate}%`;
        break;
      case 2:
        rangeText = `${stage.start_rate}% < rate ≤ ${stage.end_rate}%`;
        break;
      case 3:
        rangeText = `${stage.start_rate}% < rate ≤ ${stage.end_rate}%`;
        break;
      case 4:
        rangeText = `${stage.start_rate}% < rate ≤ 100%`;
        break;
      case 5:
        rangeText = `Over 100%`;
        break;
    }
    
    previewCard.innerHTML = `
      <div class="fiori-card__content" style="padding: var(--sap-spacing-md);">
        <div style="display: flex; align-items: center; gap: var(--sap-spacing-sm);">
          <div class="stage-color-box" style="width: 24px; height: 24px; background: ${stage.color}; border-radius: var(--sap-radius-sm);"></div>
          <div>
            <div style="font-weight: var(--sap-font-weight-medium);">Stage ${i}</div>
            <div class="stage-range-text" style="color: var(--sap-text-secondary); font-size: var(--sap-font-size-sm);">${rangeText}</div>
          </div>
        </div>
        <div class="stage-color-code" style="margin-top: var(--sap-spacing-sm); font-size: var(--sap-font-size-xs); color: var(--sap-text-tertiary);">
          ${stage.color}
        </div>
      </div>
    `;
    
    previewContainer.appendChild(previewCard);
  }
  
  console.log('✅ 5단계 미리보기 업데이트 완료');
}

/**
 * 특정 단계만 미리보기 업데이트
 */
function updateSingleStagePreview(stageNum) {
  const previewCard = document.getElementById(`preview-stage-${stageNum}`);
  if (!previewCard) return;
  
  const stage = rateColorConfig[`stage${stageNum}`];
  
  // 색상 박스 업데이트
  const colorBox = previewCard.querySelector('.stage-color-box');
  if (colorBox) {
    colorBox.style.background = stage.color;
  }
  
  // 색상 코드 업데이트
  const colorCode = previewCard.querySelector('.stage-color-code');
  if (colorCode) {
    colorCode.textContent = stage.color;
  }
  
  // 카드 테두리 색상 업데이트
  previewCard.style.borderLeft = `4px solid ${stage.color}`;
  previewCard.style.background = `${stage.color}10`;
  
  console.log(`✅ Stage ${stageNum} 미리보기만 업데이트됨`);
}

/**
 * 5단계 시스템 설정 저장 (Stage 5 포함)
 */
async function saveConfiguration() {
  console.log('💾 5단계 Rate Color 설정 저장 중... (Stage 5 포함)');
  
  // 5단계 시스템 유효성 검사
  if (!validateStageSystem()) {
    console.error('❌ 유효성 검사 실패로 저장 중단');
    return;
  }
  
  const saveBtn = document.getElementById('saveBtn');
  const originalText = saveBtn.innerHTML;
  saveBtn.innerHTML = '⏳ Saving...';
  saveBtn.disabled = true;
  
  try {
    // 5단계 시스템 데이터 준비 (Stage 5 포함)
    const configData = {
      config: rateColorConfig,
      validation: 'required',
      stage_count: 5
    };
    
    console.log('📤 저장 데이터 (Stage 5 포함):', configData);
    
    // API 호출
    const response = await fetch('./proc/rate_color.php?action=save', {
      method: 'POST',
      headers: { 'Content-Type': 'application/json' },
      body: JSON.stringify(configData)
    });
    
    if (!response.ok) {
      throw new Error(`HTTP Error: ${response.status}`);
    }
    
    const result = await response.json();
    console.log('📥 API 응답:', result);
    
    if (result.code === '00') {
      showAlert('success', `✅ 5-stage Rate Color settings saved successfully. (${result.data.saved_count} stages)`);
      console.log('✅ 5단계 설정 저장 완료 (Stage 5 포함)');
      
      // 로컬 백업도 업데이트
      localStorage.setItem('rateColorConfig', JSON.stringify(rateColorConfig));
    } else {
      throw new Error(result.msg || 'Save failed.');
    }
    
  } catch (error) {
    console.error('❌ 저장 오류:', error);
    showAlert('error', `❌ Error occurred during save: ${error.message}`);
    
    // 백업으로 로컬 스토리지에 저장
    try {
      localStorage.setItem('rateColorConfig', JSON.stringify(rateColorConfig));
      showAlert('info', 'ℹ️ Settings saved as local backup.');
      console.log('📦 로컬 백업 저장 완료');
    } catch (backupError) {
      console.error('❌ 로컬 백업도 실패:', backupError);
    }
  } finally {
    saveBtn.innerHTML = originalText;
    saveBtn.disabled = false;
  }
}


/**
 * 컬러 팔레트 리셋 (모달 시스템용)
 */
function resetColorPalettes() {
  // 각 단계별 색상 미리보기 업데이트
  Object.keys(rateColorConfig).forEach(stageKey => {
    const stageNum = stageKey.replace('stage', '');
    const defaultColor = rateColorConfig[stageKey].color;
    updateSelectedColorPreview(stageNum, defaultColor);
    updateSingleStagePreview(stageNum);
  });
  
  // 단계 선택 해제
  clearStageSelection();
}

/**
 * 범위만 업데이트 (색상은 건드리지 않음)
 */
function updateRangesOnly() {
  console.log('📏 범위만 업데이트 - 색상은 변경하지 않음');
  
  // 단계별 제목 업데이트
  updateStageTitle(1, rateColorConfig.stage1.end_rate);
  updateStageTitle(2, rateColorConfig.stage2.end_rate);
  updateStageTitle(3, rateColorConfig.stage3.end_rate);
  updateStageTitle(4, rateColorConfig.stage4.end_rate);
  
  // 전체 미리보기 업데이트 (하단 미리보기만)
  updatePreview();
}

/**
 * 모든 미리보기 업데이트 (레거시 - 사용하지 않음)
 */
function updateAllPreviews() {
  console.log('⚠️ updateAllPreviews 호출됨 - 사용하지 않아야 함');
  
  // 단계별 제목 업데이트
  updateStageTitle(1, rateColorConfig.stage1.end_rate);
  updateStageTitle(2, rateColorConfig.stage2.end_rate);
  updateStageTitle(3, rateColorConfig.stage3.end_rate);
  updateStageTitle(4, rateColorConfig.stage4.end_rate);
  
  // 전체 미리보기 업데이트
  updatePreview();
}

/**
 * 5단계 시스템 설정 로드
 */
async function loadExistingConfig() {
  console.log('📥 5단계 Rate Color 설정 로드 중...');
  
  try {
    // API에서 5단계 설정 로드
    const response = await fetch('./proc/rate_color.php?action=config');
    
    if (!response.ok) {
      throw new Error(`HTTP Error: ${response.status}`);
    }
    
    const responseText = await response.text();
    console.log('📥 API 원시 응답 길이:', responseText.length);
    
    if (!responseText || responseText.trim() === '') {
      throw new Error('API response is empty');
    }
    
    // JSON 파싱
    let result;
    try {
      result = JSON.parse(responseText);
    } catch (parseError) {
      console.error('❌ JSON 파싱 오류:', parseError);
      throw new Error('Invalid JSON format response');
    }
    
    if (result.code === '00' && result.data) {
      // 5단계 데이터 유효성 검사
      const loadedConfig = result.data;
      const requiredStages = ['stage1', 'stage2', 'stage3', 'stage4', 'stage5'];
      
      const isValidConfig = requiredStages.every(stage => 
        loadedConfig[stage] && 
        typeof loadedConfig[stage].start_rate === 'number' &&
        typeof loadedConfig[stage].end_rate === 'number' &&
        typeof loadedConfig[stage].color === 'string'
      );
      
      if (isValidConfig) {
        rateColorConfig = loadedConfig;
        console.log('✅ 데이터베이스에서 5단계 설정 로드 완료:', rateColorConfig);
        showAlert('success', '✅ Settings loaded successfully.');
      } else {
        throw new Error('Loaded settings do not match 5-stage format');
      }
    } else {
      throw new Error(result.msg || 'Settings load failed');
    }
    
    // UI 컴포넌트 업데이트 (범위만)
    updateUIFromConfig();
    
    // Spectrum 컬러피커 초기화 (데이터베이스 로드 후)
    initializeSpectrumColorPickers();
    
    // 초기 색상 로드 (한 번만)
    initializeStageColors();
    
    console.log('✅ 5단계 설정 로드 및 UI 업데이트 완료');
    
  } catch (error) {
    console.error('❌ 설정 로드 오류:', error);
    showAlert('warning', `⚠️ Settings load failed: ${error.message}. Using default values.`);
    
    // 로컬 스토리지 백업 시도
    try {
      const savedConfig = localStorage.getItem('rateColorConfig');
      if (savedConfig) {
        const backupConfig = JSON.parse(savedConfig);
        rateColorConfig = backupConfig;
        updateUIFromConfig();
        initializeSpectrumColorPickers();
        initializeStageColors();
        showAlert('info', 'ℹ️ Settings restored from local backup.');
        console.log('📦 로컬 백업에서 복구 완료');
      } else {
        // 완전한 기본값 사용
        console.log('🔄 기본 5단계 설정 사용');
        updateUIFromConfig();
        initializeSpectrumColorPickers();
        initializeStageColors();
      }
    } catch (backupError) {
      console.error('❌ 백업 복구도 실패:', backupError);
      showAlert('error', '❌ All recovery attempts failed. Please refresh the page.');
    }
  }
}

/**
 * 초기 Stage 색상 설정 (로드시에만 사용) - data 속성 기반
 */
function initializeStageColors() {
  console.log('🎨 DATA 속성 기반 초기 Stage 색상 설정 중...');
  
  // 각 Stage를 data 속성으로 독립적으로 설정
  for (let i = 1; i <= 5; i++) {
    const stageConfig = rateColorConfig[`stage${i}`];
    const colorPicker = document.getElementById(`stage${i}ColorPicker`);
    const colorBox = colorPicker; // Color picker input serves as both box and code
    
    console.log(`🔍 Stage ${i} 요소 검색:`, {
      colorPicker: colorPicker ? '✅ 발견' : '❌ 없음',
      parent: colorPicker?.parentElement?.parentElement?.getAttribute('data-stage')
    });
    
    if (colorPicker) {
      console.log(`🎯 Stage ${i} 초기화: ${stageConfig.color}`);
      
      // Color picker 값 설정
      colorPicker.value = stageConfig.color;
      
      console.log(`✅ Stage ${i} 초기 색상 설정 완료: ${stageConfig.color}`);
    } else {
      console.error(`❌ Stage ${i} ColorPicker 요소를 찾을 수 없음`);
    }
  }
  
  console.log('✅ 모든 Stage DATA 기반 초기 색상 설정 완료');
}

/**
 * 5단계 시스템 설정에서 UI 업데이트 (범위만, 색상 제외)
 */
function updateUIFromConfig() {
  console.log('📏 updateUIFromConfig 호출 - 범위만 업데이트, 색상은 건드리지 않음');
  
  try {
    // 5단계 슬라이더 값 업데이트 (Stage 5는 슬라이더 없음)
    const sliders = {
      stage1: $("#stage1Range").data("ionRangeSlider"),  // double: 0%, end_rate
      stage2: $("#stage2Range").data("ionRangeSlider"),  // double: start_rate, end_rate
      stage3: $("#stage3Range").data("ionRangeSlider"),  // double: start_rate, end_rate
      stage4: $("#stage4Range").data("ionRangeSlider")   // double: start_rate, 100% (비활성화)
    };
    
    // Stage 1: 0% ~ end_rate (from 고정, to 조정 가능)
    if (sliders.stage1) {
      sliders.stage1.update({ 
        from: 0,
        to: rateColorConfig.stage1.end_rate
      });
    }
    
    // Stage 2: start_rate ~ end_rate (from 고정, to 조정 가능)
    if (sliders.stage2) {
      sliders.stage2.update({ 
        from: rateColorConfig.stage2.start_rate,
        to: rateColorConfig.stage2.end_rate 
      });
    }
    
    // Stage 3: start_rate ~ end_rate (from 고정, to 조정 가능)
    if (sliders.stage3) {
      sliders.stage3.update({ 
        from: rateColorConfig.stage3.start_rate,
        to: rateColorConfig.stage3.end_rate 
      });
    }
    
    // Stage 4: start_rate ~ 100% (전체 고정)
    if (sliders.stage4) {
      sliders.stage4.update({ 
        from: rateColorConfig.stage4.start_rate,
        to: 100
      });
      console.log(`🔗 Stage 4 UI 업데이트: ${rateColorConfig.stage4.start_rate}% ~ 100%`);
    }
    
    // 범위만 업데이트 (색상은 초기 로드시에만)
    updateAllStagesTitle();
    updatePreview();
    
    console.log('ℹ️ updateUIFromConfig: 범위만 업데이트, 색상은 건드리지 않음');
    
    console.log('✅ 5단계 UI 업데이트 완료');
    
  } catch (error) {
    console.error('❌ UI 업데이트 오류:', error);
    showAlert('error', `❌ Error during UI update: ${error.message}`);
  }
}

/**
 * 모든 단계 제목 업데이트 (Stage 5 포함)
 */
function updateAllStagesTitle() {
  updateStageTitle(1, rateColorConfig.stage1.end_rate);
  updateStageTitle(2, rateColorConfig.stage2.end_rate);
  updateStageTitle(3, rateColorConfig.stage3.end_rate);
  updateStageTitle(4, rateColorConfig.stage4.end_rate);
  updateStageTitle(5, rateColorConfig.stage5.end_rate);
}

/**
 * 미리보기 모달 표시
 */
function showPreviewModal() {
  console.log('👁 미리보기 모달 표시');
  
  // 기존 모달 사용
  const modal = document.getElementById('previewModal');
  if (!modal) {
    console.error('❌ previewModal 요소를 찾을 수 없습니다');
    return;
  }
  
  // 모달 내용 업데이트
  updateModalPreview();
  updateSampleRatesContainer();
  
  // 모달 표시
  modal.style.display = 'flex';
  setTimeout(() => {
    modal.classList.add('show');
  }, 10);
  
  // 닫기 이벤트 (기존 이벤트 제거 후 재등록)
  const closeBtn = modal.querySelector('.modal-close');
  const backdrop = modal.querySelector('.fiori-modal__backdrop');
  
  // 기존 이벤트 리스너 제거
  closeBtn.replaceWith(closeBtn.cloneNode(true));
  backdrop.replaceWith(backdrop.cloneNode(true));
  
  // 새 이벤트 리스너 추가
  modal.querySelector('.modal-close').addEventListener('click', () => {
    closePreviewModal();
  });
  
  modal.querySelector('.fiori-modal__backdrop').addEventListener('click', () => {
    closePreviewModal();
  });
}

/**
 * 미리보기 모달 닫기
 */
function closePreviewModal() {
  const modal = document.getElementById('previewModal');
  modal.classList.remove('show');
  setTimeout(() => {
    modal.style.display = 'none';
  }, 300);
}

/**
 * 샘플 Rate 컨테이너 업데이트
 */
function updateSampleRatesContainer() {
  const container = document.getElementById('sampleRatesContainer');
  if (!container) return;
  
  container.innerHTML = generateSampleRates();
}

/**
 * 샘플 Rate 값들 생성
 */
function generateSampleRates() {
  const sampleRates = [0, 25, 50, 65, 80, 95, 100, 105, 120];
  
  return sampleRates.map(rate => {
    const color = getRateColor(rate);
    return `
      <div class="sample-rate-box" style="background: ${color};">
        ${rate}%
      </div>
    `;
  }).join('');
}

/**
 * Rate 값에 따른 색상 반환 - 5단계 시스템 (Stage 5 포함)
 */
function getRateColor(rate) {
  const stages = rateColorConfig;
  
  // Stage 1: 0% < rate ≤ stage1.end_rate
  if (rate > stages.stage1.start_rate && rate <= stages.stage1.end_rate) {
    return stages.stage1.color;
  }
  
  // Stage 2: stage1.end_rate < rate ≤ stage2.end_rate
  if (rate > stages.stage1.end_rate && rate <= stages.stage2.end_rate) {
    return stages.stage2.color;
  }
  
  // Stage 3: stage2.end_rate < rate ≤ stage3.end_rate
  if (rate > stages.stage2.end_rate && rate <= stages.stage3.end_rate) {
    return stages.stage3.color;
  }
  
  // Stage 4: stage3.end_rate < rate ≤ 100%
  if (rate > stages.stage3.end_rate && rate <= 100) {
    return stages.stage4.color;
  }
  
  // Stage 5: 100% 초과
  if (rate > 100) {
    return stages.stage5.color;
  }
  
  // 0% 정확히 해당하는 경우 (예외 상황)
  if (rate === 0) {
    console.warn(`⚠️ Rate 0%는 어느 단계에도 해당하지 않음. Stage 1 색상 사용.`);
    return stages.stage1.color;
  }
  
  // 기본값 (예외 상황)
  console.warn(`⚠️ Rate ${rate}%에 해당하는 단계를 찾을 수 없음. 기본 색상 사용.`);
  return '#6b7884';
}

/**
 * 모달 미리보기 업데이트
 */
function updateModalPreview() {
  const container = document.getElementById('modalPreviewContainer');
  if (!container) return;
  
  container.innerHTML = '';
  
  Object.keys(rateColorConfig).forEach(stageKey => {
    const stage = rateColorConfig[stageKey];
    const stageNum = stageKey.replace('stage', '');
    
    let rangeText = '';
    if (stageNum === '1') {
      rangeText = `0% ~ ${stage.end_rate}%`;
    } else if (stageNum === '4') {
      rangeText = `${stage.start_rate}% < rate (Over 100% included)`;
    } else {
      rangeText = `${stage.start_rate}% < rate ≤ ${stage.end_rate}%`;
    }
    
    const previewItem = document.createElement('div');
    previewItem.style.cssText = `
      display: flex;
      align-items: center;
      gap: var(--sap-spacing-md);
      padding: var(--sap-spacing-md);
      border: 1px solid var(--sap-border-neutral);
      border-radius: var(--sap-radius-sm);
      margin-bottom: var(--sap-spacing-sm);
      background: ${stage.color}08;
    `;
    
    previewItem.innerHTML = `
      <div style="width: 40px; height: 40px; background: ${stage.color}; border-radius: var(--sap-radius-sm); border: 1px solid var(--sap-border-neutral);"></div>
      <div style="flex: 1;">
        <div style="font-weight: var(--sap-font-weight-medium); margin-bottom: var(--sap-spacing-xs);">
          Stage ${stageNum}: ${rangeText}
        </div>
        <div style="color: var(--sap-text-secondary); font-size: var(--sap-font-size-sm);">
          Color: ${stage.color}
        </div>
      </div>
    `;
    
    container.appendChild(previewItem);
  });
}

/**
 * 색상 시스템 테스트
 */
function testColorSystem() {
  console.log('🧪 Rate Color 시스템 테스트 시작');
  
  const testValues = [0, 25, 50, 65, 80, 95, 100, 105, 120];
  const testResults = [];
  
  testValues.forEach(rate => {
    const color = getRateColor(rate);
    testResults.push({ rate, color });
    console.log(`Rate ${rate}% → Color ${color}`);
  });
  
  showAlert('info', `🧪 Test completed: ${testResults.length} values verified`);
  console.log('✅ 색상 시스템 테스트 완료:', testResults);
}

/**
 * 알림 메시지 표시
 */
function showAlert(type, message) {
  // 기존 알림 제거
  const existingAlert = document.querySelector('.rate-color-alert');
  if (existingAlert) {
    existingAlert.remove();
  }
  
  // 새 알림 생성
  const alert = document.createElement('div');
  alert.className = `fiori-alert fiori-alert--${type} rate-color-alert`;
  alert.style.cssText = `
    position: fixed;
    top: 20px;
    right: 20px;
    z-index: 9999;
    max-width: 400px;
    animation: slideInRight 0.3s ease;
  `;
  alert.innerHTML = message;
  
  document.body.appendChild(alert);
  
  // 3초 후 자동 제거
  setTimeout(() => {
    if (alert.parentNode) {
      alert.style.animation = 'slideOutRight 0.3s ease';
      setTimeout(() => alert.remove(), 300);
    }
  }, 3000);
}

/**
 * Rate Color 설정 내보내기 (개발자용)
 */
function exportConfiguration() {
  const exportData = {
    timestamp: new Date().toISOString(),
    version: '1.0',
    config: rateColorConfig
  };
  
  const blob = new Blob([JSON.stringify(exportData, null, 2)], { type: 'application/json' });
  const url = URL.createObjectURL(blob);
  
  const a = document.createElement('a');
  a.href = url;
  a.download = `rate-color-config-${new Date().toISOString().split('T')[0]}.json`;
  a.click();
  
  URL.revokeObjectURL(url);
  console.log('📁 Rate Color 설정 내보내기 완료');
}

/**
 * CSS 애니메이션 추가
 */
const style = document.createElement('style');
style.textContent = `
@keyframes slideInRight {
  from { transform: translateX(100%); opacity: 0; }
  to { transform: translateX(0); opacity: 1; }
}

@keyframes slideOutRight {
  from { transform: translateX(0); opacity: 1; }
  to { transform: translateX(100%); opacity: 0; }
}
`;
document.head.appendChild(style);

/**
 * Spectrum 컬러피커 초기화
 */
function initializeSpectrumColorPickers() {
  console.log('🎨 Spectrum 컬러피커 초기화 중...');
  
  // 기존 Spectrum 인스턴스 제거 (재초기화 시)
  for (let stage = 1; stage <= 5; stage++) {
    const pickerElement = $(`#stage${stage}ColorPicker`);
    if (pickerElement.length && pickerElement.spectrum) {
      try {
        pickerElement.spectrum('destroy');
        console.log(`🔄 Stage ${stage} 기존 Spectrum 인스턴스 제거`);
      } catch (e) {
        console.log(`ℹ️ Stage ${stage} Spectrum 인스턴스 없음 (첫 초기화)`);
      }
    }
  }
  
  // 사용할 색상 팔레트 (SAP Fiori 호환)
  const fioriColorPalette = [
    // SAP 브랜드 색상
    ["#0070f2", "#1e88e5", "#2196f3", "#42a5f5"],
    // 상태 색상
    ["#30914c", "#4caf50", "#66bb6a", "#81c784"],
    ["#da1e28", "#f44336", "#ef5350", "#e57373"], 
    ["#e26b0a", "#ff9800", "#ffa726", "#ffb74d"],
    // 회색 계열
    ["#32363b", "#4a5568", "#6b7884", "#8b95a1"],
    ["#a0aec0", "#cbd5e0", "#e2e8f0", "#f7fafc"],
    // 추가 색상
    ["#800080", "#9b59b6", "#8e44ad", "#7c3aed"],
    ["#ffa500", "#ff8c00", "#ed8936", "#dd6b20"]
  ];
  
  // 각 Stage별 Spectrum 컬러피커 초기화
  for (let stage = 1; stage <= 5; stage++) {
    const pickerElement = $(`#stage${stage}ColorPicker`);
    
    if (pickerElement.length) {
      console.log(`🎯 Stage ${stage} Spectrum 컬러피커 초기화...`);
      
      pickerElement.spectrum({
        type: "component",
        showInput: true,
        showInitial: true,
        allowEmpty: false,
        showAlpha: false,
        disabled: false,
        showPalette: true,
        showPaletteOnly: false,
        togglePaletteOnly: false,
        showSelectionPalette: true,
        hideAfterPaletteSelect: true,
        clickoutFiresChange: true,
        color: rateColorConfig[`stage${stage}`].color,
        palette: fioriColorPalette,
        localStorageKey: `spectrum.stage${stage}`,
        maxSelectionSize: 10,
        preferredFormat: "hex",
        chooseText: "Select",
        cancelText: "Cancel",
        clearText: "Reset",
        
        // 색상 변경 이벤트 (실시간)
        move: function(color) {
          const hexColor = color.toHexString();
          console.log(`🎨 Stage ${stage} 색상 이동: ${hexColor}`);
        },
        
        // 색상 선택 완료 이벤트
        change: function(color) {
          const hexColor = color.toHexString();
          console.log(`✅ Stage ${stage} 색상 변경 완료: ${hexColor}`);
          
          // 설정 업데이트 (다른 Stage에 영향 없음)
          rateColorConfig[`stage${stage}`].color = hexColor;
          
          // 미리보기 업데이트
          updatePreview();
          
          console.log(`📝 Stage ${stage} 설정 저장됨: ${hexColor}`);
        },
        
        // 모달 표시 전 이벤트
        beforeShow: function(color) {
          console.log(`🔍 Stage ${stage} 컬러피커 열기: ${color.toHexString()}`);
          return true;
        },
        
        // 모달 표시 이벤트
        show: function(color) {
          console.log(`👁 Stage ${stage} 컬러피커 표시됨`);
        },
        
        // 모달 숨김 이벤트
        hide: function(color) {
          console.log(`🙈 Stage ${stage} 컬러피커 숨겨짐`);
        }
      });
      
      console.log(`✅ Stage ${stage} Spectrum 초기화 완료`);
    } else {
      console.error(`❌ Stage ${stage} 컬러피커 요소를 찾을 수 없음`);
    }
  }
  
  console.log('✅ 모든 Spectrum 컬러피커 초기화 완료');
}

// 전역 함수로 내보내기
window.rateColorManager = {
  getRateColor,
  saveConfiguration,
  exportConfiguration,
  testColorSystem,
  initializeSpectrumColorPickers
};