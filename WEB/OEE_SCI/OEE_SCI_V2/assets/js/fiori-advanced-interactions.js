/*!
 * SAP Fiori Advanced Interactions
 * 고급 인터랙션, 접근성, 사용성 향상을 위한 JavaScript 라이브러리
 * Version: 1.0.0
 */

(function (global) {
  'use strict';

  // 메인 FioriInteractions 객체
  const FioriInteractions = {
    version: '1.0.0',

    // 초기화 플래그
    initialized: false,

    // 설정 옵션
    config: {
      animations: true,
      accessibility: true,
      rippleEffect: true,
      tooltips: true,
      notifications: true,
      autoSave: false,
      language: 'ko'
    },

    // 초기화 함수
    init: function (options = {}) {
      if (this.initialized) return;

      // 설정 병합
      Object.assign(this.config, options);

      // 핵심 기능 초기화
      this.initAccessibility();
      this.initAnimations();
      this.initInteractions();
      this.initNotifications();
      this.initKeyboardNavigation();
      this.initFormEnhancements();
      this.initTableEnhancements();
      this.initTooltips();
      this.initPerformanceMetrics();

      this.initialized = true;
      console.log('🎨 SAP Fiori Advanced Interactions v' + this.version + ' initialized');
    },

    // 브라우저 호환성을 위한 closest() 폴리필
    findClosest: function (element, selector) {
      // 네이티브 closest() 메서드가 있으면 사용
      if (element.closest && typeof element.closest === 'function') {
        return element.closest(selector);
      }

      // 폴리필 구현: 부모 요소들을 순회하며 선택자에 매치되는 요소 찾기
      let parent = element;
      while (parent && parent !== document) {
        if (this.matches(parent, selector)) {
          return parent;
        }
        parent = parent.parentElement;
      }
      return null;
    },

    // 브라우저 호환성을 위한 matches() 폴리필
    matches: function (element, selector) {
      // 네이티브 matches 메서드들 확인
      const matchesMethod = element.matches ||
        element.webkitMatchesSelector ||
        element.mozMatchesSelector ||
        element.msMatchesSelector;

      if (matchesMethod) {
        return matchesMethod.call(element, selector);
      }

      // 최후의 수단: querySelectorAll을 사용한 매칭
      const matches = (element.document || element.ownerDocument).querySelectorAll(selector);
      let i = matches.length;
      while (--i >= 0 && matches.item(i) !== element) { }
      return i > -1;
    },

    // 접근성 기능 초기화
    initAccessibility: function () {
      if (!this.config.accessibility) return;

      // 스킵 링크 추가
      this.addSkipLinks();

      // ARIA 라벨 자동 추가
      this.addAriaLabels();

      // 포커스 트래핑
      this.initFocusTrapping();

      // 스크린 리더 알림
      this.initScreenReaderAnnouncements();

      console.log('♿ 접근성 기능이 활성화되었습니다.');
    },

    // 스킵 링크 추가
    addSkipLinks: function () {
      const skipLink = document.createElement('a');
      skipLink.href = '#main-content';
      skipLink.className = 'fiori-skip-link';
      skipLink.textContent = '메인 콘텐츠로 건너뛰기';
      skipLink.setAttribute('accesskey', 's');

      document.body.insertAdjacentElement('afterbegin', skipLink);

      // 메인 콘텐츠 영역 식별
      const main = document.querySelector('main, [role="main"], .fiori-container');
      if (main && !main.id) {
        main.id = 'main-content';
      }
    },

    // ARIA 라벨 자동 추가
    addAriaLabels: function () {
      // 버튼에 적절한 라벨 추가
      document.querySelectorAll('.fiori-btn:not([aria-label]):not([aria-labelledby])').forEach(btn => {
        const text = btn.textContent.trim();
        if (text) {
          btn.setAttribute('aria-label', text);
        }
      });

      // 테이블 캡션 추가
      document.querySelectorAll('.fiori-table:not([aria-label]):not(caption)').forEach(table => {
        const section = this.findClosest(table, 'section');
        const title = section?.querySelector('.demo-title, .fiori-section__title');
        if (title) {
          table.setAttribute('aria-label', title.textContent.trim());
        }
      });

      // 폼 필드 연결
      document.querySelectorAll('.fiori-input, .fiori-select').forEach(input => {
        const label = input.previousElementSibling;
        if (label && label.tagName === 'LABEL' && !label.getAttribute('for')) {
          const id = input.id || 'field-' + Math.random().toString(36).substr(2, 9);
          input.id = id;
          label.setAttribute('for', id);
        }
      });
    },

    // 포커스 트래핑 (모달용)
    initFocusTrapping: function () {
      document.addEventListener('keydown', (e) => {
        if (e.key !== 'Tab') return;

        const modal = document.querySelector('.fiori-modal.show');
        if (!modal) return;

        const focusableElements = modal.querySelectorAll(
          'button, [href], input, select, textarea, [tabindex]:not([tabindex="-1"])'
        );

        const firstElement = focusableElements[0];
        const lastElement = focusableElements[focusableElements.length - 1];

        if (e.shiftKey && document.activeElement === firstElement) {
          e.preventDefault();
          lastElement.focus();
        } else if (!e.shiftKey && document.activeElement === lastElement) {
          e.preventDefault();
          firstElement.focus();
        }
      });
    },

    // 스크린 리더 알림
    initScreenReaderAnnouncements: function () {
      // 라이브 영역 생성
      if (!document.getElementById('live-region')) {
        const liveRegion = document.createElement('div');
        liveRegion.id = 'live-region';
        liveRegion.className = 'fiori-sr-only';
        liveRegion.setAttribute('aria-live', 'polite');
        liveRegion.setAttribute('aria-atomic', 'true');
        document.body.appendChild(liveRegion);
      }
    },

    // 스크린 리더에 메시지 알림
    announceToScreenReader: function (message, priority = 'polite') {
      const liveRegion = document.getElementById('live-region');
      if (liveRegion) {
        liveRegion.setAttribute('aria-live', priority);
        liveRegion.textContent = message;

        // 짧은 지연 후 클리어 (중복 알림 방지)
        setTimeout(() => {
          liveRegion.textContent = '';
        }, 1000);
      }
    },



    // 애니메이션 초기화
    initAnimations: function () {
      if (!this.config.animations) return;

      // Intersection Observer로 스크롤 애니메이션
      const observerOptions = {
        threshold: 0.1,
        rootMargin: '0px 0px -50px 0px'
      };

      const observer = new IntersectionObserver((entries) => {
        entries.forEach(entry => {
          if (entry.isIntersecting) {
            entry.target.classList.add('fiori-fade-in');
            observer.unobserve(entry.target);
          }
        });
      }, observerOptions);

      // 애니메이션 대상 요소들
      document.querySelectorAll('.demo-section, .fiori-card, .demo-card-advanced').forEach(el => {
        el.style.opacity = '0';
        el.style.transform = 'translateY(20px)';
        el.style.transition = 'opacity 0.6s ease, transform 0.6s ease';
        observer.observe(el);
      });

      // CSS 애니메이션 클래스 정의
      const animationCSS = `
        .fiori-fade-in {
          opacity: 1 !important;
          transform: translateY(0) !important;
        }
      `;

      const style = document.createElement('style');
      style.textContent = animationCSS;
      document.head.appendChild(style);
    },

    // 인터랙션 초기화
    initInteractions: function () {
      // 리플 효과
      if (this.config.rippleEffect) {
        this.initRippleEffect();
      }

      // 카드 호버 효과
      this.initCardEffects();

      // 드롭다운 관리
      this.initDropdowns();

      // 모달 관리
      this.initModals();
    },

    // 리플 효과 초기화
    initRippleEffect: function () {
      document.addEventListener('click', (e) => {
        const button = this.findClosest(e.target, '.fiori-btn');
        if (!button) return;

        const ripple = document.createElement('div');
        const rect = button.getBoundingClientRect();
        const size = Math.max(rect.width, rect.height);
        const x = e.clientX - rect.left - size / 2;
        const y = e.clientY - rect.top - size / 2;

        ripple.style.cssText = `
          position: absolute;
          width: ${size}px;
          height: ${size}px;
          left: ${x}px;
          top: ${y}px;
          background: rgba(255, 255, 255, 0.3);
          border-radius: 50%;
          transform: scale(0);
          animation: ripple 0.6s ease-out;
          pointer-events: none;
          z-index: 1;
        `;

        button.style.position = 'relative';
        button.style.overflow = 'hidden';
        button.appendChild(ripple);

        setTimeout(() => ripple.remove(), 600);
      });

      // 리플 애니메이션 CSS
      if (!document.getElementById('ripple-styles')) {
        const rippleCSS = document.createElement('style');
        rippleCSS.id = 'ripple-styles';
        rippleCSS.textContent = `
          @keyframes ripple {
            from { transform: scale(0); opacity: 1; }
            to { transform: scale(1); opacity: 0; }
          }
        `;
        document.head.appendChild(rippleCSS);
      }
    },

    // 카드 효과 초기화
    initCardEffects: function () {
      document.querySelectorAll('.fiori-card, .demo-card-advanced, .demo-interactive-card').forEach(card => {
        card.addEventListener('mouseenter', function () {
          this.style.zIndex = '10';
        });

        card.addEventListener('mouseleave', function () {
          this.style.zIndex = '1';
        });
      });
    },

    // 드롭다운 초기화
    initDropdowns: function () {
      document.addEventListener('click', (e) => {
        const dropdown = this.findClosest(e.target, '.fiori-dropdown');

        if (dropdown) {
          const content = dropdown.querySelector('.fiori-dropdown__content');
          const isOpen = content.classList.contains('show');

          // 다른 열린 드롭다운 닫기
          document.querySelectorAll('.fiori-dropdown__content.show').forEach(other => {
            if (other !== content) {
              other.classList.remove('show');
            }
          });

          // 현재 드롭다운 토글
          content.classList.toggle('show', !isOpen);

          // 접근성 알림
          const action = isOpen ? '닫혔습니다' : '열렸습니다';
          this.announceToScreenReader(`드롭다운이 ${action}`);

        } else {
          // 외부 클릭시 모든 드롭다운 닫기
          document.querySelectorAll('.fiori-dropdown__content.show').forEach(content => {
            content.classList.remove('show');
          });
        }
      });

      // ESC 키로 드롭다운 닫기
      document.addEventListener('keydown', (e) => {
        if (e.key === 'Escape') {
          document.querySelectorAll('.fiori-dropdown__content.show').forEach(content => {
            content.classList.remove('show');
          });
        }
      });
    },

    // 모달 초기화
    initModals: function () {
      // 모달 열기 버튼
      document.addEventListener('click', (e) => {
        const trigger = this.findClosest(e.target, '[data-modal-target]');
        if (trigger) {
          const modalId = trigger.dataset.modalTarget;
          const modal = document.getElementById(modalId) || document.querySelector(modalId);
          if (modal) {
            this.openModal(modal);
          }
        }

        // 모달 닫기
        const closeBtn = this.findClosest(e.target, '.fiori-modal .close, .fiori-modal [data-modal-close]');
        if (closeBtn) {
          const modal = this.findClosest(closeBtn, '.fiori-modal');
          if (modal) {
            this.closeModal(modal);
          }
        }

        // 배경 클릭으로 닫기 비활성화 (다른 영역 클릭 시 모달이 닫히지 않도록 변경됨)
        // if (e.target.classList.contains('fiori-modal')) {
        //   this.closeModal(e.target);
        // }
      });

      // ESC 키로 모달 닫기
      document.addEventListener('keydown', (e) => {
        if (e.key === 'Escape') {
          const openModal = document.querySelector('.fiori-modal.show');
          if (openModal) {
            this.closeModal(openModal);
          }
        }
      });
    },

    // 모달 열기
    openModal: function (modal) {
      modal.classList.add('show');
      document.body.style.overflow = 'hidden';

      // 첫 번째 포커스 가능한 요소에 포커스
      const firstFocusable = modal.querySelector('button, [href], input, select, textarea, [tabindex]:not([tabindex="-1"])');
      if (firstFocusable) {
        setTimeout(() => firstFocusable.focus(), 100);
      }

      this.announceToScreenReader('모달이 열렸습니다');
    },

    // 모달 닫기
    closeModal: function (modal) {
      modal.classList.remove('show');
      document.body.style.overflow = '';
      this.announceToScreenReader('모달이 닫혔습니다');
    },

    // 알림 시스템 초기화
    initNotifications: function () {
      if (!this.config.notifications) return;

      // 알림 컨테이너 생성
      if (!document.getElementById('notification-container')) {
        const container = document.createElement('div');
        container.id = 'notification-container';
        container.style.cssText = `
          position: fixed;
          top: 20px;
          right: 20px;
          z-index: 10000;
          display: flex;
          flex-direction: column;
          gap: var(--sap-spacing-sm);
          max-width: 400px;
        `;
        document.body.appendChild(container);
      }
    },

    // 알림 표시
    showNotification: function (message, type = 'info', duration = 5000) {
      const container = document.getElementById('notification-container');
      if (!container) return;

      const notification = document.createElement('div');
      notification.className = `fiori-alert fiori-alert--${type}`;
      notification.style.cssText = `
        border-radius: var(--sap-radius-lg);
        box-shadow: var(--sap-shadow-3);
        backdrop-filter: blur(10px);
        animation: slideInRight 0.3s ease;
        position: relative;
        cursor: pointer;
      `;

      const icons = {
        success: '✅',
        warning: '⚠️',
        error: '❌',
        info: 'ℹ️'
      };

      notification.innerHTML = `
        <div style="display: flex; align-items: center; gap: var(--sap-spacing-sm);">
          <span style="font-size: var(--sap-font-size-lg);">${icons[type]}</span>
          <span style="flex: 1;">${message}</span>
          <button style="background: none; border: none; font-size: var(--sap-font-size-lg); cursor: pointer; opacity: 0.7;">&times;</button>
        </div>
      `;

      container.appendChild(notification);

      // 닫기 기능
      const closeBtn = notification.querySelector('button');
      const closeNotification = () => {
        notification.style.animation = 'slideOutRight 0.3s ease';
        setTimeout(() => notification.remove(), 300);
      };

      closeBtn.onclick = closeNotification;
      notification.onclick = closeNotification;

      // 자동 닫기
      if (duration > 0) {
        setTimeout(closeNotification, duration);
      }

      // 접근성 알림
      this.announceToScreenReader(`알림: ${message}`, 'assertive');

      // 애니메이션 CSS 추가
      if (!document.getElementById('notification-animations')) {
        const animCSS = document.createElement('style');
        animCSS.id = 'notification-animations';
        animCSS.textContent = `
          @keyframes slideInRight {
            from { transform: translateX(100%); opacity: 0; }
            to { transform: translateX(0); opacity: 1; }
          }
          @keyframes slideOutRight {
            from { transform: translateX(0); opacity: 1; }
            to { transform: translateX(100%); opacity: 0; }
          }
        `;
        document.head.appendChild(animCSS);
      }
    },


    // 키보드 네비게이션 초기화
    initKeyboardNavigation: function () {
      // Tab 순서 관리
      this.manageFocusOrder();

      // 화살표 키 네비게이션 (테이블, 그리드용)
      this.initArrowNavigation();

      // 단축키 등록
      this.registerShortcuts();
    },

    // 포커스 순서 관리
    manageFocusOrder: function () {
      // 동적으로 추가되는 요소들의 tabindex 관리
      const observer = new MutationObserver((mutations) => {
        mutations.forEach(mutation => {
          mutation.addedNodes.forEach(node => {
            if (node.nodeType === Node.ELEMENT_NODE) {
              const focusableElements = node.querySelectorAll?.(
                'button:not([disabled]), [href], input:not([disabled]), select:not([disabled]), textarea:not([disabled]), [tabindex]:not([tabindex="-1"])'
              );

              focusableElements?.forEach((el, index) => {
                if (!el.hasAttribute('tabindex') || el.tabIndex === 0) {
                  // 자연스러운 tab 순서 유지
                }
              });
            }
          });
        });
      });

      observer.observe(document.body, { childList: true, subtree: true });
    },

    // 화살표 키 네비게이션
    initArrowNavigation: function () {
      document.addEventListener('keydown', (e) => {
        // 테이블 네비게이션
        if (['ArrowUp', 'ArrowDown', 'ArrowLeft', 'ArrowRight'].includes(e.key)) {
          const cell = this.findClosest(e.target, 'td, th');
          if (cell) {
            e.preventDefault();
            this.navigateTableCell(cell, e.key);
          }

          // 그리드 네비게이션
          const gridItem = this.findClosest(e.target, '.demo-interactive-grid > *');
          if (gridItem) {
            e.preventDefault();
            this.navigateGridItem(gridItem, e.key);
          }
        }
      });
    },

    // 테이블 셀 네비게이션
    navigateTableCell: function (currentCell, direction) {
      const row = currentCell.parentElement;
      const table = this.findClosest(row, 'table');
      const cellIndex = Array.from(row.children).indexOf(currentCell);
      const rowIndex = Array.from(table.rows).indexOf(row);

      let targetCell = null;

      switch (direction) {
        case 'ArrowLeft':
          targetCell = row.cells[cellIndex - 1];
          break;
        case 'ArrowRight':
          targetCell = row.cells[cellIndex + 1];
          break;
        case 'ArrowUp':
          targetCell = table.rows[rowIndex - 1]?.cells[cellIndex];
          break;
        case 'ArrowDown':
          targetCell = table.rows[rowIndex + 1]?.cells[cellIndex];
          break;
      }

      if (targetCell) {
        const focusable = targetCell.querySelector('button, [href], input, select, textarea, [tabindex]:not([tabindex="-1"])') || targetCell;
        focusable.focus();
      }
    },

    // 그리드 아이템 네비게이션
    navigateGridItem: function (currentItem, direction) {
      const grid = currentItem.parentElement;
      const items = Array.from(grid.children);
      const currentIndex = items.indexOf(currentItem);
      const columns = this.getGridColumns(grid);

      let targetIndex = -1;

      switch (direction) {
        case 'ArrowLeft':
          targetIndex = currentIndex - 1;
          break;
        case 'ArrowRight':
          targetIndex = currentIndex + 1;
          break;
        case 'ArrowUp':
          targetIndex = currentIndex - columns;
          break;
        case 'ArrowDown':
          targetIndex = currentIndex + columns;
          break;
      }

      if (targetIndex >= 0 && targetIndex < items.length) {
        items[targetIndex].focus();
      }
    },

    // 그리드 컬럼 수 계산
    getGridColumns: function (grid) {
      const gridStyle = window.getComputedStyle(grid);
      const columns = gridStyle.getPropertyValue('grid-template-columns').split(' ');
      return columns.length;
    },

    // 단축키 등록
    registerShortcuts: function () {
      document.addEventListener('keydown', (e) => {
        // Alt + S: 메인 콘텐츠로 이동
        if (e.altKey && e.key === 's') {
          e.preventDefault();
          const mainContent = document.getElementById('main-content') || document.querySelector('main');
          if (mainContent) {
            mainContent.focus();
            mainContent.scrollIntoView({ behavior: 'smooth' });
          }
        }


      });
    },

    // 폼 개선사항 초기화
    initFormEnhancements: function () {
      // 실시간 유효성 검사
      this.initRealTimeValidation();

      // 자동 저장 기능
      if (this.config.autoSave) {
        this.initAutoSave();
      }

      // 폼 진행 상황 표시
      this.initFormProgress();
    },

    // 실시간 유효성 검사
    initRealTimeValidation: function () {
      document.addEventListener('input', (e) => {
        const input = e.target;
        if (!input.matches('.fiori-input, .fiori-select')) return;

        // 기본 HTML5 유효성 검사
        const isValid = input.checkValidity();

        // 클래스 업데이트
        input.classList.remove('fiori-input--error', 'fiori-input--success');
        input.classList.add(isValid ? 'fiori-input--success' : 'fiori-input--error');

        // 오류 메시지 표시
        this.updateFieldMessage(input, isValid);
      });

      document.addEventListener('blur', (e) => {
        const input = e.target;
        if (!input.matches('.fiori-input, .fiori-select')) return;

        // 더 엄격한 유효성 검사
        this.validateField(input);
      });
    },

    // 필드 유효성 검사
    validateField: function (input) {
      const value = input.value.trim();
      const type = input.type;
      let isValid = input.checkValidity();
      let message = '';

      // 추가 유효성 검사 규칙
      if (type === 'email' && value && !this.isValidEmail(value)) {
        isValid = false;
        message = '올바른 이메일 형식이 아닙니다.';
      }

      if (input.hasAttribute('required') && !value) {
        isValid = false;
        message = '이 필드는 필수입니다.';
      }

      // UI 업데이트
      input.classList.remove('fiori-input--error', 'fiori-input--success');
      input.classList.add(isValid ? 'fiori-input--success' : 'fiori-input--error');

      this.updateFieldMessage(input, isValid, message);

      return isValid;
    },

    // 이메일 유효성 검사
    isValidEmail: function (email) {
      const emailRegex = /^[^\s@]+@[^\s@]+\.[^\s@]+$/;
      return emailRegex.test(email);
    },

    // 필드 메시지 업데이트
    updateFieldMessage: function (input, isValid, message = '') {
      let messageEl = input.parentElement.querySelector('.field-message');

      if (!messageEl) {
        messageEl = document.createElement('div');
        messageEl.className = 'field-message';
        messageEl.style.cssText = `
          font-size: var(--sap-font-size-xs);
          transition: all 0.3s ease;
        `;
        input.parentElement.appendChild(messageEl);
      }

      if (!isValid && message) {
        messageEl.textContent = message;
        messageEl.style.color = 'var(--sap-status-error)';
        messageEl.setAttribute('role', 'alert');
      } else if (isValid) {
        // messageEl.textContent = '✓ 올바른 형식입니다.';
        messageEl.textContent = '';
        messageEl.style.color = '';
        messageEl.removeAttribute('role');
      } else {
        messageEl.textContent = '';
        messageEl.removeAttribute('role');
      }
    },

    // 검증 메시지 지우기
    clearFieldMessage: function (input) {
      const messageEl = input.parentElement.querySelector('.field-message');
      if (messageEl) {
        messageEl.textContent = '';
        messageEl.removeAttribute('role');
      }
    },

    // 모든 검증 메시지 지우기
    clearAllFieldMessages: function (container = document) {
      const messageElements = container.querySelectorAll('.field-message');
      messageElements.forEach(messageEl => {
        messageEl.textContent = '';
        messageEl.removeAttribute('role');
      });
    },

    // 자동 저장 초기화
    initAutoSave: function () {
      let saveTimeout;

      document.addEventListener('input', (e) => {
        const input = e.target;
        if (!input.matches('.fiori-input, .fiori-select, textarea')) return;

        // 이전 타이머 제거
        clearTimeout(saveTimeout);

        // 새 타이머 설정 (2초 후 저장)
        saveTimeout = setTimeout(() => {
          this.autoSaveForm(this.findClosest(input, 'form'));
        }, 2000);
      });
    },

    // 폼 자동 저장
    autoSaveForm: function (form) {
      if (!form) return;

      const formData = new FormData(form);
      const data = {};
      formData.forEach((value, key) => {
        data[key] = value;
      });

      // localStorage에 저장 (실제로는 서버로 전송)
      const formId = form.id || 'form-' + Date.now();
      localStorage.setItem('autosave-' + formId, JSON.stringify(data));

      // 저장 알림
      this.showNotification('폼이 자동 저장되었습니다', 'success', 2000);
    },

    // 폼 진행 상황 초기화
    initFormProgress: function () {
      document.querySelectorAll('form').forEach(form => {
        const requiredFields = form.querySelectorAll('[required]');
        if (requiredFields.length > 0) {
          this.addProgressIndicator(form, requiredFields.length);
        }
      });
    },

    // 진행 상황 표시기 추가
    addProgressIndicator: function (form, totalFields) {
      const progressBar = document.createElement('div');
      progressBar.className = 'form-progress';
      progressBar.style.cssText = `
        width: 100%;
        height: 4px;
        background: var(--sap-surface-2);
        border-radius: 2px;
        margin-bottom: var(--sap-spacing-lg);
        overflow: hidden;
      `;

      const progressFill = document.createElement('div');
      progressFill.style.cssText = `
        width: 0%;
        height: 100%;
        background: var(--sap-brand-primary);
        border-radius: 2px;
        transition: width 0.3s ease;
      `;

      progressBar.appendChild(progressFill);
      form.insertBefore(progressBar, form.firstChild);

      // 진행률 업데이트 함수
      const updateProgress = () => {
        const requiredFields = form.querySelectorAll('[required]');
        const filledFields = Array.from(requiredFields).filter(field =>
          field.value.trim() !== '' && field.checkValidity()
        );

        const progress = (filledFields.length / requiredFields.length) * 100;
        progressFill.style.width = progress + '%';

        // 완료시 알림
        if (progress === 100) {
          progressFill.style.background = 'var(--sap-status-success)';
          this.announceToScreenReader('모든 필수 필드가 완료되었습니다');
        } else {
          progressFill.style.background = 'var(--sap-brand-primary)';
        }
      };

      // 이벤트 리스너 등록
      form.addEventListener('input', updateProgress);
      form.addEventListener('change', updateProgress);

      // 초기 진행률 계산
      updateProgress();
    },

    // 테이블 개선사항 초기화
    initTableEnhancements: function () {
      document.querySelectorAll('.fiori-table').forEach(table => {
        // 정렬 기능 추가
        this.addTableSorting(table);

        // 행 선택 기능
        this.addRowSelection(table);

        // 키보드 네비게이션 (이미 initArrowNavigation에서 처리됨)
      });
    },

    // 테이블 정렬 기능
    addTableSorting: function (table) {
      const headers = table.querySelectorAll('th');

      headers.forEach((header, index) => {
        if (!header.hasAttribute('data-sortable') || header.dataset.sortable === 'false') {
          return;
        }

        header.style.cursor = 'pointer';
        header.setAttribute('role', 'button');
        header.setAttribute('tabindex', '0');

        // 정렬 표시기 추가
        const sortIcon = document.createElement('span');
        sortIcon.className = 'sort-icon';
        sortIcon.innerHTML = ' ↕️';
        sortIcon.style.fontSize = 'var(--sap-font-size-xs)';
        header.appendChild(sortIcon);

        header.addEventListener('click', () => this.sortTable(table, index));
        header.addEventListener('keydown', (e) => {
          if (e.key === 'Enter' || e.key === ' ') {
            e.preventDefault();
            this.sortTable(table, index);
          }
        });
      });
    },

    // 테이블 정렬 실행
    sortTable: function (table, columnIndex) {
      const tbody = table.querySelector('tbody');
      const rows = Array.from(tbody.rows);
      const header = table.querySelectorAll('th')[columnIndex];
      const sortIcon = header.querySelector('.sort-icon');

      // 현재 정렬 방향 확인
      const currentSort = header.dataset.sort || 'none';
      const newSort = currentSort === 'asc' ? 'desc' : 'asc';

      // 모든 헤더의 정렬 상태 초기화
      table.querySelectorAll('th').forEach(h => {
        h.dataset.sort = 'none';
        const icon = h.querySelector('.sort-icon');
        if (icon) icon.innerHTML = ' ↕️';
      });

      // 현재 헤더 정렬 상태 설정
      header.dataset.sort = newSort;
      sortIcon.innerHTML = newSort === 'asc' ? ' ↑' : ' ↓';

      // 행 정렬
      rows.sort((a, b) => {
        const aValue = a.cells[columnIndex].textContent.trim();
        const bValue = b.cells[columnIndex].textContent.trim();

        const aNum = parseFloat(aValue.replace(/[^0-9.-]/g, ''));
        const bNum = parseFloat(bValue.replace(/[^0-9.-]/g, ''));

        let comparison;
        if (!isNaN(aNum) && !isNaN(bNum)) {
          comparison = aNum - bNum;
        } else {
          comparison = aValue.localeCompare(bValue, 'ko', { numeric: true });
        }

        return newSort === 'asc' ? comparison : -comparison;
      });

      // 정렬된 행 다시 추가
      rows.forEach(row => tbody.appendChild(row));

      // 정렬 완료 알림
      this.announceToScreenReader(`테이블이 ${header.textContent.trim()} 컬럼으로 ${newSort === 'asc' ? '오름차순' : '내림차순'} 정렬되었습니다`);
    },

    // 행 선택 기능
    addRowSelection: function (table) {
      const tbody = table.querySelector('tbody');
      if (!tbody) return;

      tbody.addEventListener('click', (e) => {
        const row = this.findClosest(e.target, 'tr');
        if (row && !this.findClosest(e.target, 'button, a, input')) {
          row.classList.toggle('selected');

          // 선택된 행 수 업데이트
          const selectedCount = tbody.querySelectorAll('tr.selected').length;
          this.announceToScreenReader(`${selectedCount}개 행이 선택되었습니다`);
        }
      });

      // 선택된 행 스타일 추가
      if (!document.getElementById('row-selection-styles')) {
        const selectionCSS = document.createElement('style');
        selectionCSS.id = 'row-selection-styles';
        selectionCSS.textContent = `
          .fiori-table tbody tr.selected {
            background-color: rgba(0, 112, 242, 0.1) !important;
            border-left: 3px solid var(--sap-brand-primary) !important;
          }
        `;
        document.head.appendChild(selectionCSS);
      }
    },

    // 툴팁 초기화
    initTooltips: function () {
      if (!this.config.tooltips) return;

      // 툴팁 컨테이너 생성
      if (!document.getElementById('tooltip-container')) {
        const container = document.createElement('div');
        container.id = 'tooltip-container';
        container.style.cssText = `
          position: absolute;
          background: var(--sap-surface-1);
          color: var(--sap-text-primary);
          border: 1px solid var(--sap-border-neutral);
          border-radius: var(--sap-radius-sm);
          padding: var(--sap-spacing-xs) var(--sap-spacing-sm);
          font-size: var(--sap-font-size-xs);
          box-shadow: var(--sap-shadow-2);
          z-index: 10000;
          pointer-events: none;
          opacity: 0;
          transition: opacity 0.2s ease;
          max-width: 200px;
        `;
        document.body.appendChild(container);
      }

      // 툴팁 이벤트 처리
      document.addEventListener('mouseenter', (e) => {
        // closest() 메서드가 없는 브라우저를 위한 폴리필 사용
        const element = this.findClosest(e.target, '[title], [data-tooltip]');
        if (element) {
          const tooltip = element.getAttribute('title') || element.getAttribute('data-tooltip');
          if (tooltip) {
            this.showTooltip(tooltip, e.pageX, e.pageY);
            // title 속성 임시 제거 (중복 표시 방지)
            if (element.hasAttribute('title')) {
              element.dataset.originalTitle = element.title;
              element.removeAttribute('title');
            }
          }
        }
      }, true);

      document.addEventListener('mouseleave', (e) => {
        // closest() 메서드가 없는 브라우저를 위한 폴리필 사용
        const element = this.findClosest(e.target, '[data-original-title], [data-tooltip]');
        if (element) {
          this.hideTooltip();
          // title 속성 복원
          if (element.dataset.originalTitle) {
            element.title = element.dataset.originalTitle;
            delete element.dataset.originalTitle;
          }
        }
      }, true);

      document.addEventListener('mousemove', (e) => {
        const tooltip = document.getElementById('tooltip-container');
        if (tooltip.style.opacity === '1') {
          this.positionTooltip(e.pageX, e.pageY);
        }
      });
    },

    // 툴팁 표시
    showTooltip: function (text, x, y) {
      const tooltip = document.getElementById('tooltip-container');
      tooltip.textContent = text;
      tooltip.style.opacity = '1';
      this.positionTooltip(x, y);
    },

    // 툴팁 숨기기
    hideTooltip: function () {
      const tooltip = document.getElementById('tooltip-container');
      tooltip.style.opacity = '0';
    },

    // 툴팁 위치 조정
    positionTooltip: function (x, y) {
      const tooltip = document.getElementById('tooltip-container');
      const rect = tooltip.getBoundingClientRect();

      let left = x + 10;
      let top = y - rect.height - 10;

      // 화면 경계 체크 및 조정
      if (left + rect.width > window.innerWidth) {
        left = x - rect.width - 10;
      }

      if (top < 0) {
        top = y + 10;
      }

      tooltip.style.left = left + 'px';
      tooltip.style.top = top + 'px';
    },

    // 성능 메트릭 초기화
    initPerformanceMetrics: function () {
      // 성능 메트릭 카드에 애니메이션 효과
      const observerOptions = {
        threshold: 0.5,
        rootMargin: '0px'
      };

      const metricsObserver = new IntersectionObserver((entries) => {
        entries.forEach(entry => {
          if (entry.isIntersecting) {
            const metric = entry.target;
            const progressAttr = metric.getAttribute('style');
            const progressMatch = progressAttr?.match(/--metric-progress:\s*(\d+)%/);

            if (progressMatch) {
              const targetProgress = parseInt(progressMatch[1]);
              this.animateMetricProgress(metric, targetProgress);
            }
          }
        });
      }, observerOptions);

      document.querySelectorAll('.performance-metric').forEach(metric => {
        metricsObserver.observe(metric);
      });
    },

    // 메트릭 프로그레스 애니메이션
    animateMetricProgress: function (element, targetProgress) {
      const valueElement = element.querySelector('.metric-value');
      if (!valueElement) return;

      const originalText = valueElement.textContent;
      const isPercentage = originalText.includes('%');
      const targetValue = parseFloat(originalText);

      let currentValue = 0;
      const duration = 1000;
      const steps = 60;
      const increment = targetValue / steps;
      const stepDuration = duration / steps;

      const animation = setInterval(() => {
        currentValue += increment;

        if (currentValue >= targetValue) {
          currentValue = targetValue;
          clearInterval(animation);
        }

        valueElement.textContent = isPercentage ?
          currentValue.toFixed(1) + '%' :
          Math.round(currentValue).toString();
      }, stepDuration);

      // 프로그레스 바 애니메이션
      setTimeout(() => {
        element.style.setProperty('--metric-progress', targetProgress + '%');
      }, 100);
    }
  };

  // 전역 객체로 노출
  global.FioriInteractions = FioriInteractions;

  // DOMContentLoaded 시 자동 초기화
  if (document.readyState === 'loading') {
    document.addEventListener('DOMContentLoaded', () => {
      FioriInteractions.init();
    });
  } else {
    FioriInteractions.init();
  }

})(window);