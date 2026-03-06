/**
 * Mobile Interactions Library
 * 모바일 터치 인터페이스 및 PWA 기능 관리
 * 
 * 주요 기능:
 * - 터치 제스처 처리 (스와이프, 핀치, 탭)
 * - 햄버거 메뉴 관리
 * - 풀투리프레시 (Pull-to-refresh)
 * - PWA 설치 프롬프트
 * - 오프라인 상태 관리
 * - 가상 키보드 대응
 * - 안전 영역 (Safe Area) 처리
 */

class MobileInteractions {
  constructor(options = {}) {
    this.options = {
      // 터치 제스처 임계값
      swipeThreshold: 50,
      swipeTimeout: 300,
      
      // 풀투리프레시 설정
      pullThreshold: 80,
      pullResistance: 2.5,
      
      // 햄버거 메뉴 설정
      menuBreakpoint: 768,
      menuAnimation: true,
      
      // PWA 설정
      showInstallPrompt: true,
      installPromptDelay: 3000,
      
      // 기타 설정
      enableHapticFeedback: true,
      keyboardResizeHandler: true,
      
      ...options
    };
    
    this.state = {
      // 터치 상태
      touchStartX: 0,
      touchStartY: 0,
      touchStartTime: 0,
      isSwiping: false,
      
      // 풀투리프레시 상태
      isPulling: false,
      pullDistance: 0,
      
      // 메뉴 상태
      isMenuOpen: false,
      
      // PWA 상태
      installPrompt: null,
      isInstalled: false,
      isOnline: navigator.onLine,
      
      // 키보드 상태
      keyboardVisible: false,
      initialViewportHeight: window.innerHeight
    };
    
    this.init();
  }
  
  /**
   * 초기화 함수
   */
  init() {
    console.log('📱 Mobile Interactions 초기화 시작');
    
    this.setupEventListeners();
    this.setupPWA();
    this.setupMobileMenu();
    this.setupPullToRefresh();
    this.setupKeyboardHandler();
    this.setupHapticFeedback();
    this.detectInstallation();
    
    console.log('✅ Mobile Interactions 초기화 완료');
  }
  
  /**
   * 이벤트 리스너 설정
   */
  setupEventListeners() {
    // 터치 이벤트
    document.addEventListener('touchstart', this.handleTouchStart.bind(this), { passive: false });
    document.addEventListener('touchmove', this.handleTouchMove.bind(this), { passive: false });
    document.addEventListener('touchend', this.handleTouchEnd.bind(this), { passive: true });
    
    // 네트워크 상태 변화
    window.addEventListener('online', this.handleOnline.bind(this));
    window.addEventListener('offline', this.handleOffline.bind(this));
    
    // 화면 크기 변화
    window.addEventListener('resize', this.handleResize.bind(this));
    window.addEventListener('orientationchange', this.handleOrientationChange.bind(this));
    
    // PWA 이벤트
    window.addEventListener('beforeinstallprompt', this.handleInstallPrompt.bind(this));
    window.addEventListener('appinstalled', this.handleAppInstalled.bind(this));
  }
  
  /**
   * 터치 시작 처리
   */
  handleTouchStart(event) {
    if (!event.touches || event.touches.length !== 1) return;
    
    const touch = event.touches[0];
    this.state.touchStartX = touch.clientX;
    this.state.touchStartY = touch.clientY;
    this.state.touchStartTime = Date.now();
    this.state.isSwiping = false;
  }
  
  /**
   * 터치 이동 처리
   */
  handleTouchMove(event) {
    if (!event.touches || event.touches.length !== 1 || !this.state.touchStartTime) return;
    
    const touch = event.touches[0];
    const deltaX = touch.clientX - this.state.touchStartX;
    const deltaY = touch.clientY - this.state.touchStartY;
    const absDeltaX = Math.abs(deltaX);
    const absDeltaY = Math.abs(deltaY);
    
    // 스와이프 감지
    if (absDeltaX > 10 || absDeltaY > 10) {
      this.state.isSwiping = true;
    }
    
    // 풀투리프레시 처리
    if (this.isPullToRefreshActive() && deltaY > 0 && window.scrollY === 0) {
      event.preventDefault();
      this.handlePullToRefresh(deltaY);
    }
    
    // 메뉴 스와이프 처리 (오른쪽 가장자리에서 왼쪽으로)
    if (this.state.touchStartX > window.innerWidth - 20 && deltaX < -30) {
      this.toggleMobileMenu(true);
    }
    
    // 메뉴 닫기 스와이프 (메뉴가 열린 상태에서 오른쪽으로)
    if (this.state.isMenuOpen && deltaX > 30) {
      this.toggleMobileMenu(false);
    }
  }
  
  /**
   * 터치 종료 처리
   */
  handleTouchEnd(event) {
    if (!this.state.touchStartTime) return;
    
    const touchEndTime = Date.now();
    const touchDuration = touchEndTime - this.state.touchStartTime;
    
    // 풀투리프레시 완료 처리
    if (this.state.isPulling) {
      this.completePullToRefresh();
    }
    
    // 빠른 탭 감지 (더블 탭 등)
    if (!this.state.isSwiping && touchDuration < 300) {
      this.handleQuickTap(event);
    }
    
    // 상태 초기화
    this.state.touchStartX = 0;
    this.state.touchStartY = 0;
    this.state.touchStartTime = 0;
    this.state.isSwiping = false;
  }
  
  /**
   * 빠른 탭 처리 (접근성 향상)
   */
  handleQuickTap(event) {
    const target = event.target;
    
    // 버튼이나 링크에 포커스 제공
    if (target.matches('.fiori-btn, a, button, input, select, textarea')) {
      target.focus();
      
      // 햅틱 피드백
      this.triggerHapticFeedback('light');
    }
  }
  
  /**
   * PWA 설정
   */
  setupPWA() {
    // PWA 설치 버튼 생성
    if (this.options.showInstallPrompt) {
      this.createInstallButton();
    }
  }
  
  /**
   * PWA 설치 프롬프트 처리
   */
  handleInstallPrompt(event) {
    console.log('📲 PWA 설치 프롬프트 준비');
    
    event.preventDefault();
    this.state.installPrompt = event;
    
    // 설치 버튼 표시 (지연)
    setTimeout(() => {
      this.showInstallPrompt();
    }, this.options.installPromptDelay);
  }
  
  /**
   * PWA 설치 버튼 생성
   */
  createInstallButton() {
    const installButton = document.createElement('button');
    installButton.id = 'pwa-install-btn';
    installButton.className = 'fiori-btn fiori-btn--primary pwa-install-btn';
    installButton.innerHTML = '📱 앱 설치';
    installButton.style.display = 'none';
    installButton.style.position = 'fixed';
    installButton.style.bottom = '20px';
    installButton.style.right = '20px';
    installButton.style.zIndex = '1000';
    installButton.style.borderRadius = '50px';
    installButton.style.padding = '12px 20px';
    installButton.style.boxShadow = 'var(--sap-shadow-lg)';
    
    installButton.addEventListener('click', this.installPWA.bind(this));
    
    document.body.appendChild(installButton);
  }
  
  /**
   * PWA 설치 프롬프트 표시
   */
  showInstallPrompt() {
    const installButton = document.getElementById('pwa-install-btn');
    
    if (installButton && this.state.installPrompt && !this.state.isInstalled) {
      installButton.style.display = 'block';
      installButton.style.animation = 'fadeInUp 0.5s ease';
      
      // 알림 표시
      this.showInstallNotification();
    }
  }
  
  /**
   * PWA 설치 실행
   */
  async installPWA() {
    if (!this.state.installPrompt) return;
    
    console.log('📲 PWA 설치 시작');
    
    try {
      const result = await this.state.installPrompt.prompt();
      console.log('PWA 설치 결과:', result);
      
      if (result.outcome === 'accepted') {
        console.log('✅ 사용자가 PWA 설치를 승인했습니다');
        this.hideInstallButton();
      } else {
        console.log('❌ 사용자가 PWA 설치를 거부했습니다');
      }
      
    } catch (error) {
      console.error('❌ PWA 설치 오류:', error);
    }
    
    this.state.installPrompt = null;
  }
  
  /**
   * PWA 설치 완료 처리
   */
  handleAppInstalled(event) {
    console.log('✅ PWA 설치 완료');
    this.state.isInstalled = true;
    this.hideInstallButton();
    
    // 설치 완료 알림
    this.showNotification('앱이 성공적으로 설치되었습니다! 📱', 'success');
  }
  
  /**
   * PWA 설치 감지
   */
  detectInstallation() {
    // standalone 모드 감지
    if (window.matchMedia('(display-mode: standalone)').matches) {
      this.state.isInstalled = true;
      console.log('✅ PWA standalone 모드에서 실행 중');
    }
    
    // iOS Safari 홈 화면 추가 감지
    if (window.navigator.standalone === true) {
      this.state.isInstalled = true;
      console.log('✅ iOS PWA 모드에서 실행 중');
    }
  }
  
  /**
   * 설치 버튼 숨기기
   */
  hideInstallButton() {
    const installButton = document.getElementById('pwa-install-btn');
    if (installButton) {
      installButton.style.animation = 'fadeOut 0.5s ease';
      setTimeout(() => {
        installButton.style.display = 'none';
      }, 500);
    }
  }
  
  /**
   * 모바일 메뉴 설정
   */
  setupMobileMenu() {
    // 햄버거 메뉴 버튼 생성
    this.createMobileMenuButton();
    
    // 메뉴 오버레이 생성
    this.createMobileMenuOverlay();
    
    // 미디어 쿼리 감지
    this.setupMediaQueryListener();
  }
  
  /**
   * 햄버거 메뉴 버튼 생성
   */
  createMobileMenuButton() {
    let menuButton = document.querySelector('.mobile-menu-toggle');
    
    if (!menuButton) {
      menuButton = document.createElement('button');
      menuButton.className = 'mobile-menu-toggle';
      menuButton.innerHTML = '☰';
      menuButton.setAttribute('aria-label', '메뉴 열기');
      menuButton.style.display = 'none';
      
      menuButton.addEventListener('click', () => {
        this.toggleMobileMenu();
      });
      
      // 네비게이션 바에 추가
      const navbar = document.querySelector('.demo-navbar, .fiori-main-header');
      if (navbar) {
        navbar.appendChild(menuButton);
      }
    }
  }
  
  /**
   * 모바일 메뉴 오버레이 생성
   */
  createMobileMenuOverlay() {
    let overlay = document.querySelector('.mobile-menu-overlay');
    
    if (!overlay) {
      overlay = document.createElement('div');
      overlay.className = 'mobile-menu-overlay';
      
      overlay.addEventListener('click', () => {
        this.toggleMobileMenu(false);
      });
      
      document.body.appendChild(overlay);
    }
  }
  
  /**
   * 미디어 쿼리 리스너 설정
   */
  setupMediaQueryListener() {
    const mediaQuery = window.matchMedia(`(max-width: ${this.options.menuBreakpoint}px)`);
    
    const handleMediaChange = (e) => {
      const menuButton = document.querySelector('.mobile-menu-toggle');
      
      if (e.matches) {
        // 모바일 모드
        if (menuButton) menuButton.style.display = 'flex';
        this.showMobileMenu();
      } else {
        // 데스크톱 모드
        if (menuButton) menuButton.style.display = 'none';
        this.hideMobileMenu();
        this.state.isMenuOpen = false;
      }
    };
    
    mediaQuery.addListener(handleMediaChange);
    handleMediaChange(mediaQuery);
  }
  
  /**
   * 모바일 메뉴 토글
   */
  toggleMobileMenu(forceState = null) {
    const isOpen = forceState !== null ? forceState : !this.state.isMenuOpen;
    
    this.state.isMenuOpen = isOpen;
    
    const overlay = document.querySelector('.mobile-menu-overlay');
    const menu = document.querySelector('.mobile-menu, .navbar-nav');
    const menuButton = document.querySelector('.mobile-menu-toggle');
    
    if (isOpen) {
      // 메뉴 열기
      if (overlay) overlay.classList.add('active');
      if (menu) menu.classList.add('active');
      if (menuButton) {
        menuButton.innerHTML = '✕';
        menuButton.setAttribute('aria-label', '메뉴 닫기');
      }
      
      // 스크롤 방지
      document.body.style.overflow = 'hidden';
      
      // 햅틱 피드백
      this.triggerHapticFeedback('medium');
      
    } else {
      // 메뉴 닫기
      if (overlay) overlay.classList.remove('active');
      if (menu) menu.classList.remove('active');
      if (menuButton) {
        menuButton.innerHTML = '☰';
        menuButton.setAttribute('aria-label', '메뉴 열기');
      }
      
      // 스크롤 복원
      document.body.style.overflow = '';
      
      // 햅틱 피드백
      this.triggerHapticFeedback('light');
    }
  }
  
  /**
   * 모바일 메뉴 표시
   */
  showMobileMenu() {
    // 기존 네비게이션을 모바일 메뉴로 변환
    const existingNav = document.querySelector('.navbar-nav');
    if (existingNav && !existingNav.classList.contains('mobile-menu-converted')) {
      existingNav.classList.add('mobile-menu', 'mobile-menu-converted');
    }
  }
  
  /**
   * 모바일 메뉴 숨기기
   */
  hideMobileMenu() {
    const mobileMenu = document.querySelector('.mobile-menu');
    if (mobileMenu) {
      mobileMenu.classList.remove('active');
    }
    
    const overlay = document.querySelector('.mobile-menu-overlay');
    if (overlay) {
      overlay.classList.remove('active');
    }
  }
  
  /**
   * 풀투리프레시 설정
   */
  setupPullToRefresh() {
    // 풀투리프레시 인디케이터 생성
    this.createPullToRefreshIndicator();
  }
  
  /**
   * 풀투리프레시 인디케이터 생성
   */
  createPullToRefreshIndicator() {
    let indicator = document.querySelector('.pull-to-refresh-indicator');
    
    if (!indicator) {
      indicator = document.createElement('div');
      indicator.className = 'pull-to-refresh-indicator';
      indicator.innerHTML = '⟲';
      
      const container = document.querySelector('.fiori-container, body');
      if (container) {
        container.insertBefore(indicator, container.firstChild);
      }
    }
  }
  
  /**
   * 풀투리프레시 활성화 확인
   */
  isPullToRefreshActive() {
    return window.innerWidth <= this.options.menuBreakpoint;
  }
  
  /**
   * 풀투리프레시 처리
   */
  handlePullToRefresh(deltaY) {
    const pullDistance = Math.min(deltaY / this.options.pullResistance, this.options.pullThreshold);
    this.state.pullDistance = pullDistance;
    
    const indicator = document.querySelector('.pull-to-refresh-indicator');
    const container = document.querySelector('.fiori-container');
    
    if (indicator && container) {
      const progress = pullDistance / this.options.pullThreshold;
      
      // 인디케이터 위치 및 상태 업데이트
      indicator.style.transform = `translateY(${pullDistance}px) rotate(${progress * 360}deg)`;
      indicator.style.opacity = Math.min(progress, 1);
      
      // 컨테이너 변형
      container.style.transform = `translateY(${pullDistance * 0.5}px)`;
      
      if (pullDistance >= this.options.pullThreshold && !this.state.isPulling) {
        this.state.isPulling = true;
        indicator.innerHTML = '↻';
        this.triggerHapticFeedback('medium');
      }
    }
  }
  
  /**
   * 풀투리프레시 완료
   */
  completePullToRefresh() {
    if (!this.state.isPulling) return;
    
    console.log('🔄 풀투리프레시 실행');
    
    const indicator = document.querySelector('.pull-to-refresh-indicator');
    const container = document.querySelector('.fiori-container');
    
    if (indicator && container) {
      // 애니메이션 리셋
      indicator.style.animation = 'spin 0.5s ease';
      
      setTimeout(() => {
        indicator.style.transform = '';
        indicator.style.opacity = '';
        indicator.innerHTML = '⟲';
        container.style.transform = '';
        
        // 페이지 새로고침
        this.refreshPage();
        
      }, 500);
    }
    
    this.state.isPulling = false;
    this.state.pullDistance = 0;
  }
  
  /**
   * 페이지 새로고침
   */
  refreshPage() {
    // 현재 페이지 데이터 새로고침
    if (typeof window.refreshData === 'function') {
      window.refreshData();
    } else {
      location.reload();
    }
    
    // 성공 피드백
    setTimeout(() => {
      this.showNotification('페이지가 새로고침되었습니다', 'success');
      this.triggerHapticFeedback('success');
    }, 100);
  }
  
  /**
   * 키보드 핸들러 설정
   */
  setupKeyboardHandler() {
    if (!this.options.keyboardResizeHandler) return;
    
    // 가상 키보드 표시/숨김 감지
    let resizeTimer;
    
    window.addEventListener('resize', () => {
      clearTimeout(resizeTimer);
      resizeTimer = setTimeout(() => {
        this.handleKeyboardResize();
      }, 100);
    });
    
    // 입력 필드 포커스 처리
    document.addEventListener('focusin', this.handleInputFocus.bind(this));
    document.addEventListener('focusout', this.handleInputBlur.bind(this));
  }
  
  /**
   * 키보드 크기 변화 처리
   */
  handleKeyboardResize() {
    const currentHeight = window.innerHeight;
    const heightDiff = this.state.initialViewportHeight - currentHeight;
    
    if (heightDiff > 100) {
      // 키보드 표시됨
      if (!this.state.keyboardVisible) {
        this.state.keyboardVisible = true;
        this.handleKeyboardShow(heightDiff);
      }
    } else {
      // 키보드 숨겨짐
      if (this.state.keyboardVisible) {
        this.state.keyboardVisible = false;
        this.handleKeyboardHide();
      }
    }
  }
  
  /**
   * 키보드 표시 처리
   */
  handleKeyboardShow(keyboardHeight) {
    console.log('⌨️ 가상 키보드 표시됨:', keyboardHeight);
    
    document.body.classList.add('keyboard-visible');
    
    // 활성 입력 필드를 뷰포트에 표시
    const activeElement = document.activeElement;
    if (activeElement && activeElement.matches('input, textarea, select')) {
      setTimeout(() => {
        activeElement.scrollIntoView({ behavior: 'smooth', block: 'center' });
      }, 300);
    }
  }
  
  /**
   * 키보드 숨김 처리
   */
  handleKeyboardHide() {
    console.log('⌨️ 가상 키보드 숨겨짐');
    document.body.classList.remove('keyboard-visible');
  }
  
  /**
   * 입력 필드 포커스 처리
   */
  handleInputFocus(event) {
    const input = event.target;
    
    if (input.matches('input, textarea, select')) {
      // 입력 필드 하이라이트
      input.classList.add('input-focused');
      
      // 부드러운 스크롤
      setTimeout(() => {
        input.scrollIntoView({ behavior: 'smooth', block: 'center' });
      }, 100);
    }
  }
  
  /**
   * 입력 필드 블러 처리
   */
  handleInputBlur(event) {
    const input = event.target;
    
    if (input.matches('input, textarea, select')) {
      input.classList.remove('input-focused');
    }
  }
  
  /**
   * 햅틱 피드백 설정
   */
  setupHapticFeedback() {
    if (!this.options.enableHapticFeedback || !navigator.vibrate) return;
    
    console.log('📳 햅틱 피드백 활성화');
  }
  
  /**
   * 햅틱 피드백 실행
   */
  triggerHapticFeedback(type = 'light') {
    if (!this.options.enableHapticFeedback || !navigator.vibrate) return;
    
    const patterns = {
      light: [10],
      medium: [50],
      heavy: [100],
      success: [50, 50, 50],
      error: [100, 50, 100],
      warning: [50, 100, 50]
    };
    
    const pattern = patterns[type] || patterns.light;
    navigator.vibrate(pattern);
  }
  
  /**
   * 온라인 상태 처리
   */
  handleOnline() {
    console.log('🌐 온라인 상태');
    this.state.isOnline = true;
    
    document.body.classList.remove('offline');
    document.body.classList.add('online');
    
    this.showNotification('인터넷에 연결되었습니다', 'success');
    
    // 대기 중인 데이터 동기화
    if ('serviceWorker' in navigator && navigator.serviceWorker.ready) {
      navigator.serviceWorker.ready.then(registration => {
        if (registration.sync) {
          registration.sync.register('oee-data-sync');
        }
      });
    }
  }
  
  /**
   * 오프라인 상태 처리
   */
  handleOffline() {
    console.log('📶 오프라인 상태');
    this.state.isOnline = false;
    
    document.body.classList.remove('online');
    document.body.classList.add('offline');
    
    this.showNotification('인터넷 연결이 끊어졌습니다. 오프라인 모드로 전환됩니다.', 'warning');
  }
  
  /**
   * 화면 크기 변화 처리
   */
  handleResize() {
    // 안전 영역 재계산
    this.updateSafeArea();
    
    // 메뉴 상태 업데이트
    if (window.innerWidth > this.options.menuBreakpoint && this.state.isMenuOpen) {
      this.toggleMobileMenu(false);
    }
  }
  
  /**
   * 화면 방향 변화 처리
   */
  handleOrientationChange() {
    setTimeout(() => {
      this.state.initialViewportHeight = window.innerHeight;
      this.handleResize();
    }, 500);
  }
  
  /**
   * 안전 영역 업데이트
   */
  updateSafeArea() {
    const root = document.documentElement;
    
    // CSS 커스텀 속성으로 안전 영역 정보 제공
    if (CSS.supports('padding-top: env(safe-area-inset-top)')) {
      root.style.setProperty('--safe-area-inset-top', 'env(safe-area-inset-top)');
      root.style.setProperty('--safe-area-inset-bottom', 'env(safe-area-inset-bottom)');
      root.style.setProperty('--safe-area-inset-left', 'env(safe-area-inset-left)');
      root.style.setProperty('--safe-area-inset-right', 'env(safe-area-inset-right)');
    }
  }
  
  /**
   * 알림 표시
   */
  showNotification(message, type = 'info', duration = 3000) {
    // 기존 Fiori 알림 시스템 사용
    const notification = document.createElement('div');
    notification.className = `fiori-alert fiori-alert--${type} mobile-notification`;
    notification.textContent = message;
    notification.style.position = 'fixed';
    notification.style.top = '20px';
    notification.style.left = '50%';
    notification.style.transform = 'translateX(-50%)';
    notification.style.zIndex = '10000';
    notification.style.maxWidth = '90%';
    notification.style.animation = 'slideInDown 0.3s ease';
    
    document.body.appendChild(notification);
    
    setTimeout(() => {
      notification.style.animation = 'slideOutUp 0.3s ease';
      setTimeout(() => {
        document.body.removeChild(notification);
      }, 300);
    }, duration);
  }
  
  /**
   * PWA 설치 알림 표시
   */
  showInstallNotification() {
    const notification = document.createElement('div');
    notification.className = 'pwa-install-notification';
    notification.innerHTML = `
      <div class="pwa-notification-content">
        <div class="pwa-notification-icon">📱</div>
        <div class="pwa-notification-text">
          <strong>앱으로 설치하시겠습니까?</strong>
          <p>더 빠르고 편리한 앱 환경을 경험해보세요!</p>
        </div>
        <div class="pwa-notification-actions">
          <button class="fiori-btn fiori-btn--tertiary pwa-dismiss">나중에</button>
          <button class="fiori-btn fiori-btn--primary pwa-install">설치</button>
        </div>
      </div>
    `;
    
    notification.style.cssText = `
      position: fixed;
      bottom: 20px;
      left: 20px;
      right: 20px;
      background: var(--sap-surface-1);
      border-radius: var(--sap-radius-lg);
      padding: var(--sap-spacing-md);
      box-shadow: var(--sap-shadow-lg);
      z-index: 10000;
      animation: slideInUp 0.5s ease;
    `;
    
    // 이벤트 리스너
    notification.querySelector('.pwa-install').addEventListener('click', () => {
      this.installPWA();
      document.body.removeChild(notification);
    });
    
    notification.querySelector('.pwa-dismiss').addEventListener('click', () => {
      notification.style.animation = 'slideOutDown 0.3s ease';
      setTimeout(() => {
        document.body.removeChild(notification);
      }, 300);
    });
    
    document.body.appendChild(notification);
    
    // 10초 후 자동 제거
    setTimeout(() => {
      if (document.body.contains(notification)) {
        notification.style.animation = 'slideOutDown 0.3s ease';
        setTimeout(() => {
          document.body.removeChild(notification);
        }, 300);
      }
    }, 10000);
  }
}

// 전역 인스턴스 생성
let mobileInteractions;

// DOM 로드 완료 시 초기화
document.addEventListener('DOMContentLoaded', () => {
  mobileInteractions = new MobileInteractions({
    // 커스텀 설정 옵션
    swipeThreshold: 50,
    menuBreakpoint: 768,
    showInstallPrompt: true,
    enableHapticFeedback: true,
    keyboardResizeHandler: true
  });
});

// 전역 함수로 내보내기
window.MobileInteractions = MobileInteractions;
window.mobileInteractions = mobileInteractions;