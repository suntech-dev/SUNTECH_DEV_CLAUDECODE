/**
 * Work Time Manager
 * Work Time 관리 시스템을 위한 전용 매니저
 * org/set_work_time.php의 기능을 완전히 재구현하여 SAP Fiori 스타일로 구현
 */

import { createResourceManager } from './resource-manager.js';

/**
 * Work Time Manager를 생성하고 초기화하는 함수
 * @param {Object} config - Work Time 설정 객체
 */
export function createWorkTimeManager(config) {
  // 기본 ResourceManager 생성
  const resourceManager = createResourceManager(config);
  
  // Work Time 전용 기능 추가
  const workTimeManager = new WorkTimeManager(resourceManager, config);
  
  // 전역적으로 접근 가능하게 설정
  window.workTimeManagerInstance = workTimeManager;
  
  return workTimeManager;
}

class WorkTimeManager {
  constructor(resourceManager, config) {
    this.resourceManager = resourceManager;
    this.config = config;
    this.currentYear = new Date().getFullYear();
    this.currentMonth = new Date().getMonth() + 1; // JavaScript month는 0부터 시작
    
    // 초기화
    this.init();
  }

  /**
   * Work Time Manager 초기화
   */
  init() {
    this.initCalendar();
    this.initCustomDatePicker();
    this.initTimeMask();
    this.loadFactoryData();
    this.setupEventListeners();
    this.initInteractiveCalendar();
    
    // resource-manager가 먼저 데이터를 로드한 후 캘린더 로드
    setTimeout(() => {
      this.loadCalendarData();
    }, 500);
  }

  /**
   * Interactive Calendar 초기화
   */
  initInteractiveCalendar() {
    this.calendarWorkTimeData = {};
    this.selectedDateRange = { start: null, end: null };
    
    // Initialize template system
    this.initTemplateButtons();
    this.initQuickEditIntegration();
  }

  /**
   * Template buttons 초기화
   */
  initTemplateButtons() {
    const templateButtons = document.querySelectorAll('.fiori-btn--template');
    
    templateButtons.forEach(btn => {
      btn.addEventListener('click', () => {
        const template = btn.dataset.template;
        this.applyWorkTimeTemplate(template);
        
        // Visual feedback
        templateButtons.forEach(b => b.classList.remove('active'));
        btn.classList.add('active');
        
        setTimeout(() => {
          btn.classList.remove('active');
        }, 1000);
      });
    });
  }

  /**
   * Quick Edit와 기존 시스템 통합
   */
  initQuickEditIntegration() {
    // Quick edit save 버튼을 연동
    const quickApplyBtn = document.getElementById('quickApplyBtn');
    if (quickApplyBtn) {
      quickApplyBtn.addEventListener('click', () => {
        this.saveQuickEdit();
      });
    }
  }

  /**
   * 캘린더 초기화
   */
  initCalendar() {
    // 캘린더 네비게이션 이벤트
    const prevBtn = document.getElementById('prevMonthBtn');
    const nextBtn = document.getElementById('nextMonthBtn');
    const todayBtn = document.getElementById('todayBtn');
    
    if (prevBtn) {
      prevBtn.addEventListener('click', () => {
        this.navigateMonth(-1);
      });
    }
    
    if (nextBtn) {
      nextBtn.addEventListener('click', () => {
        this.navigateMonth(1);
      });
    }
    
    if (todayBtn) {
      todayBtn.addEventListener('click', () => {
        this.goToToday();
      });
    }
    
    // Interactive calendar 이벤트 초기화
    this.initCalendarDragSelection();
  }

  /**
   * 캘린더 드래그 선택 기능 초기화
   */
  initCalendarDragSelection() {
    let isDragging = false;
    let startDate = null;
    
    // Calendar interaction events will be attached after calendar rendering
    this.attachCalendarEvents = () => {
      const calendarDays = document.querySelectorAll('.calendar-day:not(.calendar-day--other-month)');
      
      calendarDays.forEach(day => {
        // Single click selection
        day.addEventListener('click', (e) => {
          e.preventDefault();
          this.selectCalendarDate(day);
        });
        
        // Drag selection start
        day.addEventListener('mousedown', (e) => {
          e.preventDefault();
          isDragging = true;
          startDate = day.dataset.date;
          this.clearCalendarSelection();
          day.classList.add('calendar-day--selected');
        });
        
        // Drag selection continue
        day.addEventListener('mouseenter', () => {
          if (isDragging && startDate) {
            this.updateDragSelection(startDate, day.dataset.date);
          }
          
          if (!day.classList.contains('calendar-day--selected')) {
            day.classList.add('calendar-day--hover');
          }
        });
        
        day.addEventListener('mouseleave', () => {
          day.classList.remove('calendar-day--hover');
        });
      });
      
      // Drag selection end
      document.addEventListener('mouseup', () => {
        if (isDragging) {
          isDragging = false;
          const selectedDates = document.querySelectorAll('.calendar-day--selected');
          if (selectedDates.length > 0) {
            this.showQuickEditForSelection();
          }
        }
      });
    };
  }

  /**
   * 커스텀 날짜 선택기 구현
   */
  initCustomDatePicker() {
    const periodInput = document.getElementById('period');
    
    if (!periodInput) {
      console.error('period 입력 필드를 찾을 수 없습니다');
      return;
    }

    // 현재 날짜 정보
    this.selectedStartDate = null;
    this.selectedEndDate = null;
    this.currentPickerDate = new Date();

    // period 입력 필드 클릭 시 커스텀 캘린더 표시
    periodInput.addEventListener('click', (e) => {
      e.preventDefault();
      this.showCustomDatePicker();
    });

    // 읽기 전용으로 설정하여 직접 입력 방지
    periodInput.readOnly = true;
  }

  /**
   * 커스텀 날짜 선택기 표시
   */
  showCustomDatePicker() {
    // 기존 캘린더가 있으면 제거
    const existingPicker = document.querySelector('.custom-date-picker');
    if (existingPicker) {
      existingPicker.remove();
      return;
    }

    // 편집 모드에서 선택된 날짜가 있으면 해당 월로 이동
    if (this.selectedStartDate) {
      this.currentPickerDate = new Date(this.selectedStartDate);
    }

    // 커스텀 캘린더 생성
    const picker = this.createCustomCalendar();
    
    // 모달에 추가
    const modalContent = document.querySelector('#resourceModal .fiori-card__content');
    
    if (modalContent) {
      modalContent.appendChild(picker);
      
      // 기존 선택된 날짜가 있으면 표시
      if (this.selectedStartDate || this.selectedEndDate) {
        setTimeout(() => {
          this.updateCalendarWithSelectedDates(picker);
        }, 20);
      }
      
      // 중앙 위치 설정
      setTimeout(() => {
        this.centerCustomPicker(picker);
      }, 10);
    }
  }

  /**
   * 커스텀 캘린더 DOM 생성 (SAP Fiori 스타일)
   */
  createCustomCalendar() {
    const picker = document.createElement('div');
    picker.className = 'custom-date-picker fiori-card';
    
    const year = this.currentPickerDate.getFullYear();
    const month = this.currentPickerDate.getMonth();
    
    picker.innerHTML = `
      <div class="custom-picker-header">
        <button type="button" class="fiori-btn fiori-btn--ghost picker-nav-btn" data-nav="prev">
          ◀
        </button>
        <h3 class="picker-title">${year}년 ${month + 1}월</h3>
        <button type="button" class="fiori-btn fiori-btn--ghost picker-nav-btn" data-nav="next">
          ▶
        </button>
      </div>
      <div class="custom-picker-body">
        <div class="picker-weekdays">
          <div class="picker-weekday">일</div>
          <div class="picker-weekday">월</div>
          <div class="picker-weekday">화</div>
          <div class="picker-weekday">수</div>
          <div class="picker-weekday">목</div>
          <div class="picker-weekday">금</div>
          <div class="picker-weekday">토</div>
        </div>
        <div class="picker-days">
          ${this.generateCalendarDays(year, month)}
        </div>
      </div>
      <div class="custom-picker-footer">
        <button type="button" class="fiori-btn fiori-btn--secondary picker-cancel">취소</button>
        <button type="button" class="fiori-btn fiori-btn--primary picker-apply">적용</button>
      </div>
    `;

    // 이벤트 리스너 추가
    this.attachPickerEvents(picker);
    
    return picker;
  }

  /**
   * 캘린더 날짜 생성
   */
  generateCalendarDays(year, month) {
    const firstDay = new Date(year, month, 1);
    const lastDay = new Date(year, month + 1, 0);
    const startDate = new Date(firstDay);
    startDate.setDate(startDate.getDate() - firstDay.getDay()); // 일요일부터 시작

    let daysHTML = '';
    
    for (let i = 0; i < 42; i++) { // 6주 * 7일
      const currentDate = new Date(startDate);
      currentDate.setDate(startDate.getDate() + i);
      
      const isCurrentMonth = currentDate.getMonth() === month;
      const isToday = this.isToday(currentDate);
      const dateString = this.formatDate(currentDate);
      
      let classes = ['picker-day'];
      if (!isCurrentMonth) classes.push('picker-day-other');
      if (isToday) classes.push('picker-day-today');
      
      daysHTML += `
        <div class="${classes.join(' ')}" data-date="${dateString}">
          ${currentDate.getDate()}
        </div>
      `;
    }
    
    return daysHTML;
  }

  /**
   * 커스텀 캘린더 이벤트 리스너
   */
  attachPickerEvents(picker) {
    // 네비게이션 버튼
    picker.querySelectorAll('.picker-nav-btn').forEach(btn => {
      btn.addEventListener('click', (e) => {
        const direction = e.currentTarget.dataset.nav;
        if (direction === 'prev') {
          this.currentPickerDate.setMonth(this.currentPickerDate.getMonth() - 1);
        } else {
          this.currentPickerDate.setMonth(this.currentPickerDate.getMonth() + 1);
        }
        this.updateCalendarDisplay(picker);
      });
    });

    // 날짜 선택
    picker.addEventListener('click', (e) => {
      if (e.target.classList.contains('picker-day') && 
          !e.target.classList.contains('picker-day-other')) {
        this.selectDate(e.target, picker);
      }
    });

    // 취소 버튼
    picker.querySelector('.picker-cancel').addEventListener('click', () => {
      this.hideCustomDatePicker();
    });

    // 적용 버튼
    picker.querySelector('.picker-apply').addEventListener('click', () => {
      this.applySelectedDates();
      this.hideCustomDatePicker();
    });

    // 외부 클릭으로 닫기
    setTimeout(() => {
      document.addEventListener('click', this.handleOutsideClick.bind(this), { once: true });
    }, 10);
  }

  /**
   * 날짜 선택 처리 (범위 선택 가능)
   */
  selectDate(dayElement, picker) {
    const dateString = dayElement.dataset.date;
    const selectedDate = new Date(dateString);

    // 기존 선택 초기화
    picker.querySelectorAll('.picker-day').forEach(day => {
      day.classList.remove('picker-day-selected', 'picker-day-start', 'picker-day-end', 'picker-day-range');
    });

    if (!this.selectedStartDate || (this.selectedStartDate && this.selectedEndDate)) {
      // 새로운 범위 시작
      this.selectedStartDate = selectedDate;
      this.selectedEndDate = null;
      dayElement.classList.add('picker-day-selected', 'picker-day-start');
    } else if (this.selectedStartDate && !this.selectedEndDate) {
      // 범위 종료
      if (selectedDate >= this.selectedStartDate) {
        this.selectedEndDate = selectedDate;
      } else {
        this.selectedEndDate = this.selectedStartDate;
        this.selectedStartDate = selectedDate;
      }
      this.highlightDateRange(picker);
    }
  }

  /**
   * 선택된 날짜 범위 하이라이트
   */
  highlightDateRange(picker) {
    if (!this.selectedStartDate || !this.selectedEndDate) return;

    const startTime = this.selectedStartDate.getTime();
    const endTime = this.selectedEndDate.getTime();

    picker.querySelectorAll('.picker-day').forEach(day => {
      const dayTime = new Date(day.dataset.date).getTime();
      
      if (dayTime === startTime) {
        day.classList.add('picker-day-selected', 'picker-day-start');
      } else if (dayTime === endTime) {
        day.classList.add('picker-day-selected', 'picker-day-end');
      } else if (dayTime > startTime && dayTime < endTime) {
        day.classList.add('picker-day-selected', 'picker-day-range');
      }
    });
  }

  /**
   * 캘린더 표시 업데이트
   */
  updateCalendarDisplay(picker) {
    const year = this.currentPickerDate.getFullYear();
    const month = this.currentPickerDate.getMonth();
    
    picker.querySelector('.picker-title').textContent = `${year}년 ${month + 1}월`;
    picker.querySelector('.picker-days').innerHTML = this.generateCalendarDays(year, month);
    
    // 선택된 날짜가 있으면 다시 표시
    if (this.selectedStartDate || this.selectedEndDate) {
      setTimeout(() => {
        this.updateCalendarWithSelectedDates(picker);
      }, 10);
    }
  }

  /**
   * 선택된 날짜를 캘린더에 표시
   */
  updateCalendarWithSelectedDates(picker) {
    if (!this.selectedStartDate) return;

    const startTime = this.selectedStartDate.getTime();
    const endTime = this.selectedEndDate ? this.selectedEndDate.getTime() : startTime;

    picker.querySelectorAll('.picker-day').forEach(day => {
      if (day.classList.contains('picker-day-other')) return;
      
      const dayTime = new Date(day.dataset.date).getTime();
      
      // 기존 선택 클래스 제거
      day.classList.remove('picker-day-selected', 'picker-day-start', 'picker-day-end', 'picker-day-range');
      
      if (dayTime === startTime && dayTime === endTime) {
        // 시작일과 종료일이 같은 경우
        day.classList.add('picker-day-selected', 'picker-day-start');
      } else if (dayTime === startTime) {
        day.classList.add('picker-day-selected', 'picker-day-start');
      } else if (dayTime === endTime) {
        day.classList.add('picker-day-selected', 'picker-day-end');
      } else if (dayTime > startTime && dayTime < endTime) {
        day.classList.add('picker-day-selected', 'picker-day-range');
      }
    });
  }

  /**
   * 선택된 날짜 적용
   */
  applySelectedDates() {
    const periodInput = document.getElementById('period');
    if (!periodInput || !this.selectedStartDate) return;

    const startStr = this.formatDate(this.selectedStartDate);
    const endStr = this.selectedEndDate ? this.formatDate(this.selectedEndDate) : startStr;
    
    periodInput.value = `${startStr} ~ ${endStr}`;
  }

  /**
   * 커스텀 캘린더 숨기기
   */
  hideCustomDatePicker() {
    const picker = document.querySelector('.custom-date-picker');
    if (picker) {
      picker.remove();
    }
  }

  /**
   * 커스텀 캘린더 중앙 위치 설정
   */
  centerCustomPicker(picker) {
    const modalContent = document.querySelector('#resourceModal .fiori-card__content');
    if (!modalContent) return;

    picker.style.position = 'absolute';
    picker.style.zIndex = '10000';
    picker.style.left = '50%';
    picker.style.top = '50%';
    picker.style.transform = 'translate(-50%, -50%)';
  }

  /**
   * 외부 클릭 처리
   */
  handleOutsideClick(e) {
    const picker = document.querySelector('.custom-date-picker');
    const periodInput = document.getElementById('period');
    
    if (picker && !picker.contains(e.target) && e.target !== periodInput) {
      this.hideCustomDatePicker();
    } else if (picker) {
      // 다시 등록
      setTimeout(() => {
        document.addEventListener('click', this.handleOutsideClick.bind(this), { once: true });
      }, 10);
    }
  }

  /**
   * Work Time Templates Definition
   */
  getWorkTimeTemplates() {
    return {
      standard: {
        name: '표준 근무',
        available: [{ start: '09:00', end: '18:00' }],
        planned1: [{ start: '10:00', end: '10:20' }],
        planned2: [{ start: '12:00', end: '12:20' }],
        overtime: [30]
      },
      shift2: {
        name: '2교대 근무',
        available: [
          { start: '08:00', end: '20:00' },
          { start: '20:00', end: '08:00' }
        ],
        planned1: [
          { start: '10:00', end: '10:15' },
          { start: '22:00', end: '22:15' }
        ],
        planned2: [
          { start: '15:00', end: '15:15' },
          { start: '03:00', end: '03:15' }
        ],
        overtime: [60, 60]
      },
      shift3: {
        name: '3교대 근무',
        available: [
          { start: '08:00', end: '16:00' },
          { start: '16:00', end: '00:00' },
          { start: '00:00', end: '08:00' }
        ],
        planned1: [
          { start: '10:00', end: '10:10' },
          { start: '18:00', end: '18:10' },
          { start: '02:00', end: '02:10' }
        ],
        planned2: [
          { start: '14:00', end: '14:10' },
          { start: '22:00', end: '22:10' },
          { start: '06:00', end: '06:10' }
        ],
        overtime: [30, 30, 30]
      }
    };
  }

  /**
   * Work Time Template 적용
   */
  applyWorkTimeTemplate(templateKey) {
    const template = this.getWorkTimeTemplates()[templateKey];
    if (!template) return;

    const selectedDates = document.querySelectorAll('.calendar-day--selected');

    if (selectedDates.length === 0) {
      this.showNotification('날짜를 먼저 선택해주세요.', 'warning');
      return;
    }

    // Apply template to selected dates
    selectedDates.forEach(dayEl => {
      const dateStr = dayEl.dataset.date;
      this.applyTemplateToDate(dateStr, template);
    });

    this.showNotification(`${template.name} 템플릿이 적용되었습니다.`, 'success');
    this.refreshCalendarDisplay();
  }

  /**
   * 날짜별 템플릿 적용
   */
  applyTemplateToDate(dateStr, template) {
    // Update local calendar data
    this.calendarWorkTimeData[dateStr] = {
      shift_count: template.available.length,
      status: 'Y',
      kind: '3', // Day type
      template_applied: template.name,
      template_data: template
    };

    // Update visual indicator
    const dayElement = document.querySelector(`[data-date="${dateStr}"]`);
    if (dayElement) {
      dayElement.classList.add('calendar-day--complete');
      
      // Add shift count display
      let summaryEl = dayElement.querySelector('.work-time-summary');
      if (!summaryEl) {
        summaryEl = document.createElement('div');
        summaryEl.className = 'work-time-summary';
        const contentEl = dayElement.querySelector('.calendar-day-content');
        if (contentEl) {
          contentEl.appendChild(summaryEl);
        }
      }
      summaryEl.textContent = `${template.available.length}교대`;
    }
  }

  /**
   * Calendar 날짜 선택
   */
  selectCalendarDate(dayElement) {
    const dateStr = dayElement.dataset.date;
    const selectedDate = new Date(dateStr);

    // Clear previous selections
    this.clearCalendarSelection();

    // Mark as selected
    dayElement.classList.add('calendar-day--selected');

    // Show quick edit panel
    this.showQuickEditPanel(dateStr, selectedDate);
  }

  /**
   * Calendar 선택 해제
   */
  clearCalendarSelection() {
    document.querySelectorAll('.calendar-day--selected').forEach(day => {
      day.classList.remove('calendar-day--selected');
    });
  }

  /**
   * 드래그 선택 업데이트
   */
  updateDragSelection(startDateStr, endDateStr) {
    const startDate = new Date(startDateStr);
    const endDate = new Date(endDateStr);

    let [rangeStart, rangeEnd] = startDate <= endDate ? [startDate, endDate] : [endDate, startDate];

    const calendarDays = document.querySelectorAll('.calendar-day');

    calendarDays.forEach(day => {
      const dayDate = new Date(day.dataset.date);

      if (dayDate >= rangeStart && dayDate <= rangeEnd && 
          !day.classList.contains('calendar-day--other-month')) {
        day.classList.add('calendar-day--selected');
      } else {
        day.classList.remove('calendar-day--selected');
      }
    });
  }

  /**
   * Quick Edit Panel 표시
   */
  showQuickEditPanel(dateStr, selectedDate) {
    const quickEditPanel = document.getElementById('quickEditPanel');
    const quickEditTitle = document.getElementById('quickEditTitle');

    if (!quickEditPanel || !quickEditTitle) return;

    const dateDisplay = selectedDate.toLocaleDateString('ko-KR', {
      year: 'numeric',
      month: 'long', 
      day: 'numeric',
      weekday: 'short'
    });

    quickEditTitle.textContent = `📅 ${dateDisplay} 근무시간`;

    // Load existing work time data for the date
    this.loadWorkTimeForQuickEdit(dateStr);

    quickEditPanel.style.display = 'block';
    quickEditPanel.scrollIntoView({ behavior: 'smooth', block: 'nearest' });
  }

  /**
   * Quick Edit 선택 영역 처리
   */
  showQuickEditForSelection() {
    const selectedDates = document.querySelectorAll('.calendar-day--selected');
    if (selectedDates.length === 1) {
      // Single date selection
      this.selectCalendarDate(selectedDates[0]);
    } else if (selectedDates.length > 1) {
      // Multiple date selection
      this.showBulkEditOptions(selectedDates);
    }
  }

  /**
   * 일괄 편집 옵션 표시
   */
  showBulkEditOptions(selectedDates) {
    const dateCount = selectedDates.length;
    const quickEditTitle = document.getElementById('quickEditTitle');

    if (quickEditTitle) {
      quickEditTitle.textContent = `📅 ${dateCount}개 날짜 일괄 편집`;
    }

    const quickEditPanel = document.getElementById('quickEditPanel');
    if (quickEditPanel) {
      quickEditPanel.style.display = 'block';
      quickEditPanel.scrollIntoView({ behavior: 'smooth', block: 'nearest' });
    }
  }

  /**
   * Quick Edit용 근무시간 데이터 로드
   */
  loadWorkTimeForQuickEdit(dateStr) {
    const workTimeInfo = this.calendarWorkTimeData[dateStr];

    if (workTimeInfo && workTimeInfo.template_data) {
      const template = workTimeInfo.template_data;
      
      // Populate quick edit inputs
      const shift1Start = document.getElementById('quickShift1Start');
      const shift1End = document.getElementById('quickShift1End');
      const shift2Start = document.getElementById('quickShift2Start');
      const shift2End = document.getElementById('quickShift2End');

      if (template.available[0]) {
        if (shift1Start) shift1Start.value = template.available[0].start;
        if (shift1End) shift1End.value = template.available[0].end;
      }
      
      if (template.available[1]) {
        if (shift2Start) shift2Start.value = template.available[1].start;
        if (shift2End) shift2End.value = template.available[1].end;
      }
    }
  }

  /**
   * Quick Edit 저장
   */
  async saveQuickEdit() {
    const selectedDates = document.querySelectorAll('.calendar-day--selected');
    const shift1Start = document.getElementById('quickShift1Start').value;
    const shift1End = document.getElementById('quickShift1End').value;
    const shift2Start = document.getElementById('quickShift2Start').value;
    const shift2End = document.getElementById('quickShift2End').value;

    if (!shift1Start || !shift1End) {
      this.showNotification('최소 1교대 시간은 입력해야 합니다.', 'error');
      return;
    }

    // Save work time for selected dates
    const promises = [];
    selectedDates.forEach(dayEl => {
      const dateStr = dayEl.dataset.date;
      const promise = this.saveWorkTimeForDate(dateStr, {
        shift1: { start: shift1Start, end: shift1End },
        shift2: { start: shift2Start, end: shift2End }
      });
      promises.push(promise);
    });

    try {
      await Promise.all(promises);
      this.hideQuickEditPanel();
      this.showNotification('근무시간이 저장되었습니다.', 'success');
      
      // Refresh data
      this.refreshData();
    } catch (error) {
      this.showNotification('저장 중 오류가 발생했습니다.', 'error');
      console.error('Save error:', error);
    }
  }

  /**
   * 날짜별 근무시간 저장
   */
  async saveWorkTimeForDate(dateStr, shiftData) {
    const factoryIdx = document.getElementById('factoryFilterSelect').value || '0';
    const lineIdx = document.getElementById('lineFilterSelect').value || '0';
    
    const formData = new URLSearchParams({
      operation: 'AddD',
      factory_idx: factoryIdx,
      line_idx: lineIdx,
      period: `${dateStr} ~ ${dateStr}`,
      status: 'Y',
      'available_stime[1]': shiftData.shift1.start,
      'available_etime[1]': shiftData.shift1.end,
      'available_stime[2]': shiftData.shift2.start || '',
      'available_etime[2]': shiftData.shift2.end || ''
    });

    const response = await fetch(this.config.apiEndpoint, {
      method: 'POST',
      body: formData
    });
    
    const result = await response.json();
    
    if (!result.success) {
      throw new Error(result.message || '저장 실패');
    }
    
    return result;
  }

  /**
   * Quick Edit Panel 숨기기
   */
  hideQuickEditPanel() {
    const quickEditPanel = document.getElementById('quickEditPanel');
    if (quickEditPanel) {
      quickEditPanel.style.display = 'none';
    }
    this.clearCalendarSelection();
  }

  /**
   * 시간 입력 마스킹 초기화
   */
  initTimeMask() {
    // jQuery Mask 플러그인 사용
    if (typeof $.fn.mask !== 'undefined') {
      // 시간 입력 마스킹 (HH:MM 형식)
      $('input[data-mask="99:99"]').mask('99:99', {
        placeholder: '__:__'
      });

      // 숫자 입력 마스킹 (overtime용)
      $('input[data-mask="999"]').mask('999', {
        placeholder: '___'
      });
    }
  }

  /**
   * 이벤트 리스너 설정
   */
  setupEventListeners() {
    // Main Add 버튼 (Add WorkTime)
    const addBtn = document.getElementById('addBtn');
    if (addBtn) {
      addBtn.addEventListener('click', () => {
        this.openAddModal('week'); // 기본은 Week 타입으로 열기
      });
    }

    // Add Week 버튼  
    const addWeekBtn = document.getElementById('addWeekBtn');
    if (addWeekBtn) {
      addWeekBtn.addEventListener('click', () => {
        this.openAddModal('week');
      });
    }

    // Add Day 버튼
    const addDayBtn = document.getElementById('addDayBtn');
    if (addDayBtn) {
      addDayBtn.addEventListener('click', () => {
        this.openAddModal('day');
      });
    }

    // Factory 변경 시 Line 목록 업데이트 (모달 내부)
    const factorySelect = document.getElementById('factory_idx');
    if (factorySelect) {
      factorySelect.addEventListener('change', (e) => {
        this.loadLineData(e.target.value);
      });
    }

    // Factory 필터 변경 시 Line 필터와 캘린더, 테이블 데이터 업데이트
    const factoryFilter = document.getElementById('factoryFilterSelect');
    if (factoryFilter) {
      factoryFilter.addEventListener('change', (e) => {
        this.loadLineFilterData(e.target.value);
        this.loadCalendarData();
        // resource-manager의 loadData 호출하여 테이블도 새로고침
        if (this.resourceManager && this.resourceManager.loadData) {
          this.resourceManager.loadData();
        }
      });
    }

    // Line 필터 변경 시 캘린더와 테이블 데이터 업데이트
    const lineFilter = document.getElementById('lineFilterSelect');
    if (lineFilter) {
      lineFilter.addEventListener('change', () => {
        this.loadCalendarData();
        // resource-manager의 loadData 호출하여 테이블도 새로고침
        if (this.resourceManager && this.resourceManager.loadData) {
          this.resourceManager.loadData();
        }
      });
    }

    // Status 필터 변경 시 테이블 데이터 업데이트
    const statusFilter = document.getElementById('statusFilterSelect');
    if (statusFilter) {
      statusFilter.addEventListener('change', () => {
        // resource-manager의 loadData 호출하여 테이블 새로고침
        if (this.resourceManager && this.resourceManager.loadData) {
          this.resourceManager.loadData();
        }
      });
    }

    // 테이블의 Edit 버튼 클릭 (이벤트 위임)
    document.addEventListener('click', (e) => {
      if (e.target.classList.contains('btn-edit')) {
        const id = e.target.getAttribute('data-id');
        const kind = e.target.getAttribute('data-kind');
        this.openEditModal(id, kind);
      }
    });

    // 캘린더 날짜 클릭 (이벤트 위임)
    document.addEventListener('click', (e) => {
      // calendar-day 클래스를 클릭한 경우
      if (e.target.closest('.calendar-day') && !e.target.closest('.calendar-day--other-month')) {
        const dayCell = e.target.closest('.calendar-day');
        const date = dayCell.getAttribute('data-date');
        if (date) {
          this.openDayModal(date);
        }
      }
    });

    // 폼 제출 이벤트 리스너
    const resourceForm = document.getElementById('resourceForm');
    if (resourceForm) {
      resourceForm.addEventListener('submit', (e) => {
        this.handleFormSubmit(e);
      });
    }
  }

  /**
   * Factory 데이터 로드
   */
  async loadFactoryData() {
    try {
      const response = await fetch(`${this.config.apiEndpoint}?factories=true`);
      const data = await response.json();
      
      if (data.success) {
        this.populateFactorySelects(data.data);
      }
    } catch (error) {
      console.error('Failed to load factory data:', error);
    }
  }

  /**
   * Factory 선택 요소들 채우기
   */
  populateFactorySelects(factories) {
    const factorySelect = document.getElementById('factory_idx');
    const factoryFilter = document.getElementById('factoryFilterSelect');
    
    if (factorySelect) {
      factorySelect.innerHTML = '<option value="-1">Select Factory</option><option value="">All Factory</option>';
      factories.forEach(factory => {
        factorySelect.innerHTML += `<option value="${factory.idx}">${factory.factory_name}</option>`;
      });
    }
    
    if (factoryFilter) {
      factoryFilter.innerHTML = '<option value="">All Factories</option>';
      factories.forEach(factory => {
        factoryFilter.innerHTML += `<option value="${factory.idx}">${factory.factory_name}</option>`;
      });
    }
  }

  /**
   * Line 데이터 로드 (모달용)
   */
  async loadLineData(factoryIdx) {
    try {
      const url = `${this.config.apiEndpoint}?lines=true&factory_idx=${factoryIdx}`;
      const response = await fetch(url);
      const data = await response.json();
      
      if (data.success) {
        const lineSelect = document.getElementById('line_idx');
        if (lineSelect) {
          lineSelect.innerHTML = '<option value="-1">Select Line</option><option value="0">All Line</option>';
          
          data.data.forEach(line => {
            lineSelect.innerHTML += `<option value="${line.idx}">${line.line_name}</option>`;
          });
        }
      }
    } catch (error) {
      console.error('Failed to load line data:', error);
    }
  }

  /**
   * Line 데이터 로드 (필터용)
   */
  async loadLineFilterData(factoryIdx) {
    try {
      const url = `${this.config.apiEndpoint}?lines=true&factory_idx=${factoryIdx}`;
      const response = await fetch(url);
      const data = await response.json();
      
      if (data.success) {
        const lineFilter = document.getElementById('lineFilterSelect');
        if (lineFilter) {
          lineFilter.innerHTML = '<option value="">All Lines</option>';
          
          data.data.forEach(line => {
            lineFilter.innerHTML += `<option value="${line.idx}">${line.line_name}</option>`;
          });
        }
      }
    } catch (error) {
      console.error('Failed to load line filter data:', error);
    }
  }

  /**
   * 캘린더 데이터 로드
   */
  async loadCalendarData() {
    try {
      const factoryIdx = document.getElementById('factoryFilterSelect').value;
      const lineIdx = document.getElementById('lineFilterSelect').value;
      
      const url = `${this.config.apiEndpoint}?calendar=true&year=${this.currentYear}&month=${this.currentMonth}&factory_idx=${factoryIdx}&line_idx=${lineIdx}`;
      const response = await fetch(url);
      const data = await response.json();
      
      if (data.success) {
        // 캘린더 컨테이너가 있으면 API 데이터로 업데이트
        const calendarContainer = document.getElementById('calendarContainer');
        if (calendarContainer && data.html) {
          calendarContainer.innerHTML = data.html;
        }
        
        const calendarTitle = document.getElementById('calendarTitle');
        if (calendarTitle && data.month_name) {
          calendarTitle.textContent = `📅 ${data.month_name}`;
        }

        // Re-attach calendar events
        setTimeout(() => {
          this.attachCalendarEvents();
        }, 100);
      }
    } catch (error) {
      console.error('Failed to load calendar data:', error);
    }
  }

  /**
   * 월 네비게이션
   */
  navigateMonth(direction) {
    this.currentMonth += direction;
    
    if (this.currentMonth > 12) {
      this.currentMonth = 1;
      this.currentYear++;
    } else if (this.currentMonth < 1) {
      this.currentMonth = 12;
      this.currentYear--;
    }
    
    this.loadCalendarData();
  }

  /**
   * 오늘로 이동
   */
  goToToday() {
    this.currentYear = new Date().getFullYear();
    this.currentMonth = new Date().getMonth() + 1;
    this.loadCalendarData();
  }

  /**
   * 추가 모달 열기
   */
  openAddModal(type) {
    this.resetModal();
    
    const modal = document.getElementById('resourceModal');
    const modalTitle = document.getElementById('modalTitle');
    const weekSection = document.getElementById('weekDaysSection');
    const operationType = document.getElementById('operation');
    const periodInput = document.getElementById('period');
    
    if (type === 'week') {
      if (modalTitle) modalTitle.textContent = 'Add Work Time by Week';
      if (weekSection) weekSection.style.display = 'block';
      if (operationType) operationType.value = 'AddW';
    } else if (type === 'day') {
      if (modalTitle) modalTitle.textContent = 'Add Work Time by Day';
      if (weekSection) weekSection.style.display = 'none';
      if (operationType) operationType.value = 'AddD';
      
      // Day 타입의 경우 오늘 날짜로 기본 설정
      const today = new Date().toISOString().split('T')[0];
      if (periodInput) periodInput.value = today + ' ~ ' + today;
      
      // 커스텀 날짜 선택기를 오늘 날짜로 설정
      this.selectedStartDate = new Date(today);
      this.selectedEndDate = new Date(today);
    }
    
    if (modal) modal.style.display = 'block';
  }

  /**
   * 수정 모달 열기
   */
  async openEditModal(id, kind) {
    try {
      const response = await fetch(`${this.config.apiEndpoint}?id=${id}`);
      const data = await response.json();
      
      if (data.success) {
        this.populateEditModal(data.data, kind);
        const modal = document.getElementById('resourceModal');
        if (modal) {
          modal.style.display = 'block';
        }
      } else {
        this.showNotification('데이터 로드에 실패했습니다: ' + data.message, 'error');
      }
    } catch (error) {
      console.error('Failed to load work time for edit:', error);
      this.showNotification('데이터 로드 중 오류가 발생했습니다', 'error');
    }
  }

  /**
   * 일별 모달 열기 (캘린더 날짜 클릭)
   */
  async openDayModal(date) {
    // 해당 날짜에 기존 근무시간이 있는지 확인
    try {
      const factoryIdx = document.getElementById('factoryFilterSelect').value;
      const lineIdx = document.getElementById('lineFilterSelect').value;
      
      if (!factoryIdx || !lineIdx) {
        this.showNotification('Factory와 Line을 먼저 선택해주세요.', 'warning');
        return;
      }
      
      const url = `${this.config.apiEndpoint}?check_date=true&date=${date}&factory_idx=${factoryIdx}&line_idx=${lineIdx}`;
      const response = await fetch(url);
      const data = await response.json();
      
      if (data.success && data.exists) {
        // 기존 근무시간이 있으면 수정 모달 열기
        this.openEditModal(data.work_time_id, data.kind);
      } else {
        // 기존 근무시간이 없으면 새로운 Day 타입 추가 모달 열기
        this.openAddDayModal(date);
      }
    } catch (error) {
      console.error('Error checking work time for date:', error);
      // 오류 시 기본적으로 추가 모달 열기
      this.openAddDayModal(date);
    }
  }
  
  /**
   * 새로운 일별 근무시간 추가 모달 열기
   */
  openAddDayModal(date) {
    this.resetModal();
    
    const modal = document.getElementById('resourceModal');
    const modalTitle = document.getElementById('modalTitle');
    const weekSection = document.getElementById('weekDaysSection');
    const operationType = document.getElementById('operation');
    const periodInput = document.getElementById('period');
    
    if (modalTitle) modalTitle.textContent = 'Add Work Time by Day';
    if (weekSection) weekSection.style.display = 'none';
    if (operationType) operationType.value = 'AddD';
    if (periodInput) periodInput.value = date + ' ~ ' + date;
    
    // 커스텀 날짜 선택기를 해당 날짜로 설정
    this.selectedStartDate = new Date(date);
    this.selectedEndDate = new Date(date);
    
    if (modal) modal.style.display = 'block';
  }

  /**
   * 수정 모달 데이터 채우기
   */
  async populateEditModal(data, kind) {
    this.resetModal();
    
    const modal = document.getElementById('resourceModal');
    const modalTitle = document.getElementById('modalTitle');
    const weekSection = document.getElementById('weekDaysSection');
    const operationType = document.getElementById('operation');
    
    // 기본 정보 설정
    const resourceId = document.getElementById('resourceId');
    if (resourceId) resourceId.value = data.idx;
    
    const factorySelect = document.getElementById('factory_idx');
    const lineSelect = document.getElementById('line_idx');
    const statusSelect = document.getElementById('status');
    const remarkInput = document.getElementById('remark');
    const periodInput = document.getElementById('period');
    
    if (factorySelect) factorySelect.value = data.factory_idx || '';
    if (lineSelect) lineSelect.value = data.line_idx || '0';
    if (statusSelect) statusSelect.value = data.status;
    if (remarkInput) remarkInput.value = data.remark || '';
    
    // Kind에 따른 모달 설정
    if (kind === '2') { // Week
      if (modalTitle) modalTitle.textContent = 'Edit Work Time by Week';
      if (weekSection) weekSection.style.display = 'block';
      if (operationType) operationType.value = 'EditW';
      
      // 요일 체크박스 설정
      const weekYn = data.week_yn || '0000000';
      for (let i = 0; i < 7; i++) {
        const checkbox = document.querySelector(`input[name="week[${i}]"]`);
        if (checkbox) {
          checkbox.checked = weekYn.charAt(i) === '1';
        }
      }
    } else if (kind === '3') { // Day
      if (modalTitle) modalTitle.textContent = 'Edit Work Time by Day';
      if (weekSection) weekSection.style.display = 'none';
      if (operationType) operationType.value = 'EditD';
    } else { // Period (kind=1) - 지원 중단
      this.showNotification('Period 타입의 근무시간은 더 이상 지원되지 않습니다. Week 또는 Day 타입으로 새로 생성해주세요.', 'warning');
      return;
    }
    
    // Shift 데이터 설정
    for (let shiftIdx = 1; shiftIdx <= 3; shiftIdx++) {
      if (data.shift && data.shift[shiftIdx]) {
        const shift = data.shift[shiftIdx];
        
        const availableStimeInput = document.querySelector(`input[name="available_stime[${shiftIdx}]"]`);
        const availableEtimeInput = document.querySelector(`input[name="available_etime[${shiftIdx}]"]`);
        const planned1StimeInput = document.querySelector(`input[name="planned1_stime[${shiftIdx}]"]`);
        const planned1EtimeInput = document.querySelector(`input[name="planned1_etime[${shiftIdx}]"]`);
        const planned2StimeInput = document.querySelector(`input[name="planned2_stime[${shiftIdx}]"]`);
        const planned2EtimeInput = document.querySelector(`input[name="planned2_etime[${shiftIdx}]"]`);
        const overTimeInput = document.querySelector(`input[name="over_time[${shiftIdx}]"]`);
        
        if (availableStimeInput) availableStimeInput.value = shift.available_stime || '';
        if (availableEtimeInput) availableEtimeInput.value = shift.available_etime || '';
        if (planned1StimeInput) planned1StimeInput.value = shift.planned1_stime || '';
        if (planned1EtimeInput) planned1EtimeInput.value = shift.planned1_etime || '';
        if (planned2StimeInput) planned2StimeInput.value = shift.planned2_stime || '';
        if (planned2EtimeInput) planned2EtimeInput.value = shift.planned2_etime || '';
        if (overTimeInput) overTimeInput.value = shift.over_time || '';
      }
    }
    
    // Line 데이터 로드
    if (data.factory_idx) {
      await this.loadLineData(data.factory_idx);
      const lineSelect = document.getElementById('line_idx');
      if (lineSelect) {
        lineSelect.value = data.line_idx || '0';
      }
    }
    
    // 커스텀 날짜 선택기용 날짜 설정
    if (periodInput && data.work_sdate && data.work_edate) {
      periodInput.value = `${data.work_sdate} ~ ${data.work_edate}`;
      
      // 편집 모드를 위한 날짜 사전 설정
      this.selectedStartDate = new Date(data.work_sdate);
      this.selectedEndDate = new Date(data.work_edate);
    }
  }

  /**
   * 모달 리셋
   */
  resetModal() {
    const form = document.getElementById('resourceForm');
    if (form) {
      form.reset();
    }
    
    // 요일 체크박스 리셋
    for (let i = 0; i < 7; i++) {
      const checkbox = document.querySelector(`input[name="week[${i}]"]`);
      if (checkbox) {
        checkbox.checked = false;
      }
    }
    
    // hidden 필드 리셋
    const resourceId = document.getElementById('resourceId');
    const operation = document.getElementById('operation');
    if (resourceId) resourceId.value = '';
    if (operation) operation.value = '';
    
    // 날짜 선택 리셋
    this.selectedStartDate = null;
    this.selectedEndDate = null;
  }

  /**
   * 모달 닫기
   */
  closeModal() {
    const modal = document.getElementById('resourceModal');
    if (modal) {
      modal.style.display = 'none';
    }
    this.resetModal();
    this.hideCustomDatePicker();
  }

  /**
   * 폼 제출 처리
   */
  async handleFormSubmit(event) {
    event.preventDefault();
    
    const form = event.target;
    const formData = new FormData(form);
    const operation = document.getElementById('operation').value;
    
    // 유효성 검사
    if (!this.validateForm(formData)) {
      return;
    }
    
    try {
      const response = await fetch(this.config.apiEndpoint, {
        method: 'POST',
        body: formData
      });
      
      const result = await response.json();
      
      if (result.success) {
        this.showNotification(result.message, 'success');
        this.closeModal();
        
        // 데이터 다시 로드
        this.refreshData();
      } else {
        this.showNotification('Error: ' + result.message, 'error');
      }
    } catch (error) {
      console.error('Form submission error:', error);
      this.showNotification('저장 중 오류가 발생했습니다.', 'error');
    }
  }

  /**
   * 폼 유효성 검사
   */
  validateForm(formData) {
    const factoryIdx = formData.get('factory_idx');
    const lineIdx = formData.get('line_idx');
    const period = formData.get('period');
    const operation = formData.get('operation');
    
    if (factoryIdx === '-1') {
      this.showNotification('Factory를 선택해주세요.', 'warning');
      return false;
    }
    
    if (lineIdx === '-1') {
      this.showNotification('Line을 선택해주세요.', 'warning');
      return false;
    }
    
    if (!period || period.trim() === '') {
      this.showNotification('기간을 선택해주세요.', 'warning');
      return false;
    }
    
    // Week 타입인 경우 요일 선택 확인
    if (operation === 'AddW' || operation === 'EditW') {
      let weekSelected = false;
      for (let i = 0; i < 7; i++) {
        if (formData.get(`week[${i}]`) === 'Y') {
          weekSelected = true;
          break;
        }
      }
      
      if (!weekSelected) {
        this.showNotification('최소 하나의 요일을 선택해주세요.', 'warning');
        return false;
      }
    }
    
    // Shift-1 시간 확인
    const availableStime1 = formData.get('available_stime[1]');
    const availableEtime1 = formData.get('available_etime[1]');
    
    if (!availableStime1 || !availableEtime1) {
      this.showNotification('Shift-1 가용 시간은 필수입니다.', 'warning');
      return false;
    }
    
    return true;
  }

  /**
   * 알림 표시 (SAP Fiori 스타일)
   */
  showNotification(message, type = 'info') {
    const notification = document.createElement('div');
    notification.className = `fiori-alert fiori-alert--${type}`;
    notification.style.cssText = `
      position: fixed;
      top: 20px;
      right: 20px;
      z-index: 100000;
      min-width: 300px;
      max-width: 500px;
      animation: slideInRight 0.3s ease;
    `;
    
    notification.innerHTML = `
      <div class="fiori-alert__content">
        <span class="fiori-alert__message">${message}</span>
        <button class="fiori-alert__close" style="background: none; border: none; color: inherit; font-size: 1.2rem; cursor: pointer; margin-left: auto;">×</button>
      </div>
    `;

    document.body.appendChild(notification);

    // Auto remove after 5 seconds
    setTimeout(() => {
      if (notification.parentNode) {
        notification.remove();
      }
    }, 5000);

    // Manual close
    notification.querySelector('.fiori-alert__close').addEventListener('click', () => {
      notification.remove();
    });
  }

  /**
   * 데이터 새로고침
   */
  refreshData() {
    if (this.resourceManager && this.resourceManager.loadData) {
      this.resourceManager.loadData();
    }
    this.loadCalendarData();
  }

  /**
   * 날짜 포맷팅
   */
  formatDate(date) {
    const year = date.getFullYear();
    const month = String(date.getMonth() + 1).padStart(2, '0');
    const day = String(date.getDate()).padStart(2, '0');
    return `${year}-${month}-${day}`;
  }

  /**
   * 오늘 날짜 체크
   */
  isToday(date) {
    const today = new Date();
    return date.toDateString() === today.toDateString();
  }
}

// 추가 스타일을 동적으로 추가
const additionalStyles = `
<style>
/* 커스텀 날짜 선택기 스타일 */
.custom-date-picker {
  position: absolute !important;
  background: var(--sap-surface-1) !important;
  border: 1px solid var(--sap-border-1) !important;
  border-radius: var(--sap-border-radius) !important;
  box-shadow: var(--sap-shadow-lg) !important;
  z-index: 10000 !important;
  font-family: var(--sap-font-family) !important;
  min-width: 280px !important;
  max-width: 320px !important;
  user-select: none !important;
  color: var(--sap-text-primary) !important;
}

.custom-picker-header {
  display: flex !important;
  align-items: center !important;
  justify-content: space-between !important;
  padding: 16px 20px !important;
  border-bottom: 1px solid var(--sap-border-1) !important;
  background: var(--sap-surface-2) !important;
}

.picker-title {
  font-size: 16px !important;
  font-weight: 600 !important;
  color: var(--sap-text-primary) !important;
  margin: 0 !important;
}

.picker-nav-btn {
  padding: 4px 8px !important;
  font-size: 14px !important;
}

.custom-picker-body {
  padding: 16px 20px !important;
}

.picker-weekdays {
  display: grid !important;
  grid-template-columns: repeat(7, 1fr) !important;
  gap: 4px !important;
  margin-bottom: 8px !important;
}

.picker-weekday {
  text-align: center !important;
  font-size: 12px !important;
  font-weight: 500 !important;
  color: var(--sap-text-secondary) !important;
  padding: 8px 4px !important;
}

.picker-days {
  display: grid !important;
  grid-template-columns: repeat(7, 1fr) !important;
  gap: 2px !important;
}

.picker-day {
  text-align: center !important;
  padding: 8px 4px !important;
  font-size: 14px !important;
  color: var(--sap-text-primary) !important;
  cursor: pointer !important;
  border-radius: var(--sap-border-radius) !important;
  transition: all 0.2s ease !important;
  min-height: 32px !important;
  display: flex !important;
  align-items: center !important;
  justify-content: center !important;
}

.picker-day:hover {
  background-color: var(--sap-surface-hover) !important;
}

.picker-day-other {
  color: var(--sap-text-tertiary) !important;
  cursor: default !important;
}

.picker-day-other:hover {
  background-color: transparent !important;
}

.picker-day-today {
  background-color: var(--sap-brand-primary-light) !important;
  color: var(--sap-brand-primary) !important;
  font-weight: 600 !important;
}

.picker-day-selected {
  background-color: var(--sap-brand-primary) !important;
  color: white !important;
  font-weight: 600 !important;
}

.picker-day-start {
  background-color: var(--sap-brand-primary) !important;
  color: white !important;
}

.picker-day-end {
  background-color: var(--sap-brand-primary) !important;
  color: white !important;
}

.picker-day-range {
  background-color: var(--sap-brand-primary-light) !important;
  color: var(--sap-brand-primary) !important;
}

.custom-picker-footer {
  display: flex !important;
  justify-content: flex-end !important;
  gap: 8px !important;
  padding: 16px 20px !important;
  border-top: 1px solid var(--sap-border-1) !important;
  background: var(--sap-surface-2) !important;
}

@keyframes slideInRight {
  from { 
    opacity: 0; 
    transform: translateX(100%); 
  }
  to { 
    opacity: 1; 
    transform: translateX(0); 
  }
}
</style>
`;

// 스타일 추가
if (!document.querySelector('#work-time-picker-styles')) {
  const styleElement = document.createElement('div');
  styleElement.id = 'work-time-picker-styles';
  styleElement.innerHTML = additionalStyles;
  document.head.appendChild(styleElement);
}