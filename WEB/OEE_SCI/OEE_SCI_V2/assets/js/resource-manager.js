/**
 * @file Generic resource manager for creating dynamic data tables with CRUD operations.
 * @version 1.2.0
 * @description 이 파일은 데이터 관리 페이지(factory, line, machine 등)의 공통 로직을 처리하는 범용 모듈입니다.
 *              API 통신, 데이터 테이블 렌더링, 페이지네이션, 필터링, 정렬, 모달을 이용한 추가/수정/삭제 기능을 담당합니다.
 */

// --- API Abstraction ---
/**
 * API 엔드포인트와 통신하는 핸들러 객체를 생성합니다.
 * @param {string} endpoint - 통신할 기본 API 경로 (예: 'proc/factory.php')
 * @returns {object} getAll, getOne, create, update, delete, export 메서드를 포함하는 API 핸들러 객체
 */
function createApiHandler(endpoint) {
  /**
   * fetch API를 사용하여 서버에 비동기 요청을 보냅니다.
   * @param {string} url - 요청할 URL
   * @param {object} options - fetch에 전달할 옵션 (method, body 등)
   * @returns {Promise<object>} 서버로부터 받은 JSON 데이터
   */
  async function request(url, options = {}) {
    try {
      const response = await fetch(url, options);
      if (!response.ok) {
        const errorData = await response.json().catch(() => ({ message: `HTTP error! Status: ${response.status}` }));
        throw new Error(errorData.message);
      }
      const contentType = response.headers.get("content-type");
      if (contentType && contentType.includes("application/json")) {
        return response.json();
      }
      return { success: true };
    } catch (error) {
      throw error;
    }
  }

  // 각 CRUD 작업에 매핑되는 메서드들을 반환합니다.
  return {
    getAll: (params) => request(`${endpoint}?${new URLSearchParams(params)}`), // 모든 데이터 조회 (페이지네이션, 필터링, 정렬 파라미터 포함)
    getOne: (id) => request(`${endpoint}?id=${id}`), // 특정 데이터 1개 조회
    create: (data) => {
      // FormData인 경우 그대로 전송, 아닌 경우 URLSearchParams로 변환
      const body = data instanceof FormData ? data : new URLSearchParams(data);
      return request(endpoint, { method: 'POST', body });
    }, // 새 데이터 생성
    update: (id, data) => { // 특정 데이터 수정
        let body;
        if (data instanceof FormData) {
          // FormData인 경우 _method와 ID를 추가
          data.append('_method', 'PUT');
          if (!data.has('idx') && !data.has('id')) {
            data.append('idx', id);
          }
          body = data;
        } else {
          // 일반 객체인 경우 기존 로직 유지
          const params = new URLSearchParams(data);
          params.append('_method', 'PUT');
          if (!data.idx && !data.id) {
            params.append('idx', id);
          }
          body = params;
        }
        return request(endpoint, { 
            method: 'POST', 
            body 
        });
    },
    // delete: (id) => request(`${endpoint}?id=${id}`, { method: 'DELETE' }), // 삭제 기능 제거됨
    export: (params) => { // 엑셀로 내보내기
      const exportParams = new URLSearchParams(params);
      exportParams.append('export', 'true');
      window.location.href = `${endpoint}?${exportParams}`;
    },
  };
}

// --- Main Factory Function ---
/**
 * 데이터 관리 페이지의 전체 기능을 생성하고 초기화하는 메인 함수입니다.
 * @param {object} config - 페이지의 동작을 정의하는 설정 객체
 */
export function createResourceManager(config) {
  // ResourceManager 인스턴스를 즉시 생성해서 반환
  const resourceManagerInstance = {
    loadData: null,
    api: null,
    state: null,
    refreshData: null,
    openModal: null,
    closeModal: null,
    isInitialized: false
  };

  // DOM이 이미 로드되었으면 즉시 실행, 아니면 DOMContentLoaded 이벤트를 기다림
  function initializeResourceManager() {
    // 설정 객체에서 필요한 값들을 추출합니다.
    const { resourceName, apiEndpoint, columnConfig, entityId, filterConfig = [], beforeInit, afterInit } = config;
    
    // API 핸들러를 생성합니다.
    const api = createApiHandler(apiEndpoint);

    // --- 1. State Management ---
    // 페이지의 상태(현재 페이지, 정렬 기준 등)를 관리하는 객체입니다.
    const state = {
      currentPage: 1,
      rowsPerPage: 10,
      sortColumn: columnConfig.find(c => c.sortable)?.sortKey || entityId, // 정렬 가능한 첫 번째 열을 기본 정렬 기준으로 설정
      sortOrder: 'asc',
      totalPages: 1,
      visibleColumns: new Set(), // 화면에 보여지는 컬럼들을 관리
      searchQuery: '', // 검색어 상태 추가
    };
    // 설정에 따라 필터 상태를 초기화합니다.
    filterConfig.forEach(f => {
      const element = document.getElementById(f.elementId);
      // HTML의 selected 속성을 우선 확인하고, 없으면 defaultValue 사용
      if (element && element.value) {
        state[f.stateKey] = element.value;
        console.log(`🔍 [${f.elementId}] 초기값 설정: "${element.value}" -> state.${f.stateKey}`);
      } else {
        state[f.stateKey] = f.defaultValue || '';
        console.log(`🔍 [${f.elementId}] 기본값 설정: "${f.defaultValue || ''}" -> state.${f.stateKey}`);
      }
    });

    // 컬럼 보이기/숨기기 상태를 localStorage에 저장하기 위한 키
    const COLUMN_STATE_KEY = `${resourceName.toLowerCase()}_column_visibility`;

    // --- 2. DOM Element Caching ---
    // 자주 사용하는 HTML 요소들을 미리 찾아와서 변수에 저장합니다.
    const elements = {
        tableHeader: document.getElementById('tableHeader'),
        tableBody: document.getElementById('tableBody'),
        paginationControls: document.getElementById('pagination-controls'),
        modal: document.getElementById('resourceModal'),
        addBtn: document.getElementById('addBtn'),
        excelDownloadBtn: document.getElementById('excelDownloadBtn'),
        refreshBtn: document.getElementById('refreshBtn'),
        resourceForm: document.getElementById('resourceForm'),
        modalTitle: document.getElementById('modalTitle'),
        resourceIdInput: document.getElementById('resourceId'),
        columnToggleBtn: document.getElementById('columnToggleBtn'),
        columnToggleDropdown: document.getElementById('columnToggleDropdown'),
    };
    elements.closeModalBtn = elements.modal.querySelector('.close');
    elements.modalCloseBtn = document.getElementById('modalCloseBtn');

    // --- 3. Column Visibility --- 
    /**
     * 현재 컬럼 보이기/숨기기 상태를 브라우저의 localStorage에 저장합니다.
     */
    function saveColumnState() {
      localStorage.setItem(COLUMN_STATE_KEY, JSON.stringify(Array.from(state.visibleColumns)));
    }

    /**
     * localStorage에서 컬럼 상태를 불러옵니다. 저장된 상태가 없으면 모든 컬럼을 보이도록 설정합니다.
     */
    function loadColumnState() {
      const savedState = localStorage.getItem(COLUMN_STATE_KEY);
      const defaultVisible = columnConfig.filter(c => c.visible !== false).map(c => c.key);
      state.visibleColumns = new Set(savedState ? JSON.parse(savedState) : defaultVisible);
    }

    /**
     * state에 저장된 `visibleColumns`를 기반으로 실제 테이블의 컬럼들을 보이거나 숨깁니다.
     */
    function applyColumnVisibility() {
      // 실제 렌더링되는 컬럼들만 필터링 (visible이 false가 아닌 컬럼들)
      const visibleConfigColumns = columnConfig.filter(col => col.visible !== false);
      
      visibleConfigColumns.forEach((col, domIndex) => {
        const isVisible = state.visibleColumns.has(col.key);
        // domIndex + 1을 사용하여 실제 DOM 위치에 맞춰 선택
        document.querySelectorAll(`#tableHeader th:nth-child(${domIndex + 1}), #tableBody td:nth-child(${domIndex + 1})`).forEach(el => {
          el.style.display = isVisible ? '' : 'none';
        });
      });
    }

    /**
     * 컬럼 보이기/숨기기를 제어하는 드롭다운 메뉴를 렌더링합니다.
     */
    function renderColumnToggler() {
      elements.columnToggleDropdown.innerHTML = '';
      
      // 드롭다운 헤더와 닫기 버튼 추가
      elements.columnToggleDropdown.insertAdjacentHTML('beforeend', `
        <div style="display: flex; justify-content: space-between; align-items: center; margin-bottom: var(--sap-spacing-sm); padding-bottom: var(--sap-spacing-xs); border-bottom: 1px solid var(--sap-border-subtle);">
          <strong style="color: var(--sap-text-primary);">Show/Hide Columns</strong>
          <button type="button" class="dropdown-close-btn" style="background: none; border: none; cursor: pointer; color: var(--sap-text-secondary); font-size: 18px; padding: 2px;" title="닫기">✕</button>
        </div>
      `);
      
      // visible이 false인 컬럼은 Show/Hide Columns 목록에서 제외
      columnConfig.filter(col => col.visible !== false).forEach(col => {
        const isChecked = state.visibleColumns.has(col.key);
        elements.columnToggleDropdown.insertAdjacentHTML('beforeend', `
          <label style="display: flex; align-items: center; gap: var(--sap-spacing-xs); padding: var(--sap-spacing-xs) 0; cursor: pointer;">
            <input type="checkbox" class="column-toggle-checkbox" data-key="${col.key}" ${isChecked ? 'checked' : ''}>
            <span>${col.label}</span>
          </label>
        `);
      });
    }

    // --- 4. Rendering Functions ---
    /**
     * 데이터 테이블의 헤더를 렌더링합니다.
     */
    function renderTableHeader() {
      elements.tableHeader.innerHTML = `<tr>${columnConfig.filter(col => col.visible !== false).map(col => {
        const sortableClass = col.sortable ? 'sortable' : '';
        const sortKey = col.sortable ? `data-sort="${col.sortKey}"` : '';
        return `<th class="${sortableClass}" ${sortKey}>${col.label}</th>`;
      }).join('')}</tr>`;
    }

    /**
     * 서버로부터 데이터를 가져와서 테이블과 페이지네이션을 렌더링합니다.
     */
    async function fetchData() {
      // 데이터 가져오기 시작

      const params = {
        page: state.currentPage,
        limit: state.rowsPerPage,
        sort: state.sortColumn,
        order: state.sortOrder,
        search: state.searchQuery, // 검색어 파라미터 추가
      };
      // 필터 상태를 API 파라미터에 추가합니다.
      filterConfig.forEach(f => {
        params[f.paramName] = state[f.stateKey];
        console.log(`🚀 API 파라미터 추가: ${f.paramName} = "${state[f.stateKey]}"`);
      });

      console.log('🔗 API 요청 파라미터:', params);

      // API 요청 파라미터 구성 완료

      try {
        const result = await api.getAll(params);
        // API 응답 수신
        
        if (result.success) {
          // 데이터 처리 중
          
          await renderTable(result.data);
          if (result.pagination) {
            renderPagination(result.pagination);
          } else {
            elements.paginationControls.innerHTML = '';
          }
          updateSortIndicators();
          applyColumnVisibility();
          
          // 테이블 렌더링 완료
        } else {
          // API 응답 에러 처리
          const visibleColumnCount = columnConfig.filter(col => col.visible !== false).length;
          elements.tableBody.innerHTML = `<tr><td colspan="${visibleColumnCount}" class="text-center">${result.message}</td></tr>`;
        }
      } catch (error) {
        // API 요청 에러 처리
        elements.tableBody.innerHTML = `<tr><td colspan="${columnConfig.length}" class="text-center">Error loading data: ${error.message}</td></tr>`;
      }
    }

    /**
     * 주어진 데이터로 테이블의 본문(tbody)을 렌더링합니다.
     * @param {Array} data - 렌더링할 데이터 배열
     */
    async function renderTable(data) {
      const { tableBody } = elements;
      // renderTable 호출
      
      tableBody.innerHTML = '';
      if (!data || data.length === 0) {
        const visibleColumnCount = columnConfig.filter(col => col.visible !== false).length;
        // 빈 데이터 메시지 표시
        tableBody.innerHTML = `<tr><td colspan="${visibleColumnCount}" class="text-center">No data available.</td></tr>`;
        
        // 빈 데이터일 때도 콜백 실행
        if (config.onTableRender && typeof config.onTableRender === 'function') {
          config.onTableRender([]);
        }
        
        // 레거시 지원을 위한 전역 함수 호출
        if (typeof window.updateStatistics === 'function') {
          try {
            await window.updateStatistics([]);
          } catch (error) {
            console.error('Statistics update error:', error);
          }
        }
        return;
      }
      
      // 데이터 렌더링 시작
      const startNumber = (state.currentPage - 1) * state.rowsPerPage + 1;
      try {
        const rows = data.map((item, index) => {
          // 행 렌더링 중
          
          const cells = columnConfig.filter(col => col.visible !== false).map(col => {
            let content = '';
            try {
              if (col.render) {
                content = col.render(item, startNumber + index);
              } else {
                content = escapeHtml(item[col.key]);
              }
              // 컬럼 렌더링
            } catch (renderError) {
              // 컬럼 렌더링 오류
              content = 'Error';
            }
            return `<td>${content}</td>`;
          });
          
          return `<tr class="data-row" data-id="${item[entityId]}" style="cursor: pointer;">${cells.join('')}</tr>`;
        });
        
        tableBody.innerHTML = rows.join('');
        // 테이블 HTML 설정 완료
        
      } catch (renderError) {
        // 테이블 렌더링 오류
        tableBody.innerHTML = `<tr><td colspan="6" class="text-center">Rendering error: ${renderError.message}</td></tr>`;
      }
      
      // 데이터 렌더링 후 콜백 실행
      if (config.onTableRender && typeof config.onTableRender === 'function') {
        config.onTableRender(data);
      }
      
      // 레거시 지원을 위한 전역 함수 호출
      if (typeof window.updateStatistics === 'function') {
        try {
          await window.updateStatistics(data);
        } catch (error) {
          console.error('Statistics update error:', error);
        }
      }
    }

    /**
     * 페이지네이션 컨트롤 UI를 렌더링합니다.
     * @param {object} pagination - 페이지네이션 정보 (total_records, current_page, total_pages)
     */
    function renderPagination(pagination = {}) {
      const { total_records = 0, current_page = 1, total_pages = 1 } = pagination;
      state.totalPages = total_pages;
      elements.paginationControls.innerHTML = '';
      if (total_records === 0) return;

      // SAP Fiori 스타일 버튼 생성 함수
      const createButton = (text, page, disabled = false, isActive = false) => {
        const btn = document.createElement('button');
        btn.innerHTML = text;
        btn.dataset.page = page;
        btn.disabled = disabled;
        btn.className = `fiori-pagination__button ${isActive ? 'fiori-pagination__button--active' : ''} ${disabled ? 'fiori-pagination__button--disabled' : ''}`;
        return btn;
      };

      // 정보 표시
      const info = document.createElement('div');
      info.className = 'fiori-pagination__info';
      // info.textContent = `${current_page} / ${total_pages} 페이지 (총 ${total_records}개 항목)`;
      info.textContent = `${current_page} / ${total_pages} page (${total_records} items total)`;
      elements.paginationControls.appendChild(info);

      // 버튼 컨테이너 생성
      const buttonsContainer = document.createElement('div');
      buttonsContainer.className = 'fiori-pagination__buttons';

      // 이전 버튼
      // buttonsContainer.appendChild(createButton('‹ 이전', current_page - 1, current_page <= 1));
      buttonsContainer.appendChild(createButton('‹ Prev', current_page - 1, current_page <= 1));
      
      // 페이지 번호 버튼들 (최대 7개까지만 표시)
      const startPage = Math.max(1, current_page - 3);
      const endPage = Math.min(total_pages, current_page + 3);
      
      if (startPage > 1) {
        buttonsContainer.appendChild(createButton('1', 1, false, false));
        if (startPage > 2) {
          const ellipsis = document.createElement('span');
          ellipsis.className = 'fiori-pagination__ellipsis';
          ellipsis.textContent = '…';
          buttonsContainer.appendChild(ellipsis);
        }
      }
      
      for (let i = startPage; i <= endPage; i++) {
        buttonsContainer.appendChild(createButton(i, i, false, i === current_page));
      }
      
      if (endPage < total_pages) {
        if (endPage < total_pages - 1) {
          const ellipsis = document.createElement('span');
          ellipsis.className = 'fiori-pagination__ellipsis';
          ellipsis.textContent = '…';
          buttonsContainer.appendChild(ellipsis);
        }
        buttonsContainer.appendChild(createButton(total_pages, total_pages, false, false));
      }
      
      // 다음 버튼
      // buttonsContainer.appendChild(createButton('다음 ›', current_page + 1, current_page >= total_pages));
      buttonsContainer.appendChild(createButton('Next ›', current_page + 1, current_page >= total_pages));
      
      // 버튼 컨테이너를 메인 컨테이너에 추가
      elements.paginationControls.appendChild(buttonsContainer);
      
      // 활성 버튼을 기준으로 중앙 정렬 적용
      centerActiveButton();
    }

    /**
     * 활성 버튼을 화면 중앙에 배치하도록 스크롤 조정
     */
    function centerActiveButton() {
      const container = elements.paginationControls;
      const buttonsContainer = container.querySelector('.fiori-pagination__buttons');
      const activeButton = container.querySelector('.fiori-pagination__button--active');
      
      if (!activeButton || !buttonsContainer) return;
      
      // DOM이 완전히 렌더링된 후에 실행
      requestAnimationFrame(() => {
        setTimeout(() => {
          // 버튼 컨테이너와 활성 버튼의 위치 정보 가져오기
          const buttonsRect = buttonsContainer.getBoundingClientRect();
          const activeRect = activeButton.getBoundingClientRect();
          
          // 활성 버튼의 중앙 위치 (버튼 컨테이너 기준)
          const activeCenter = activeRect.left + activeRect.width / 2;
          
          // 버튼 컨테이너의 중앙 위치 (화면 기준)
          const buttonsCenter = buttonsRect.left + buttonsRect.width / 2;
          
          // 스크롤해야 할 거리 계산
          const scrollOffset = activeCenter - buttonsCenter;
          
          // 현재 스크롤 위치에서 오프셋만큼 이동
          const newScrollLeft = buttonsContainer.scrollLeft + scrollOffset;
          
          // 스크롤 범위 제한 (0 이상, 최대 스크롤 가능 범위 이하)
          const maxScroll = Math.max(0, buttonsContainer.scrollWidth - buttonsContainer.clientWidth);
          const finalScrollLeft = Math.max(0, Math.min(newScrollLeft, maxScroll));
          
          // 부드러운 스크롤 적용
          buttonsContainer.scrollTo({
            left: finalScrollLeft,
            behavior: 'smooth'
          });
        }, 100); // 레이아웃 계산 완료를 위한 충분한 지연
      });
    }

    /**
     * 테이블 헤더에 현재 정렬 상태(오름차순/내림차순)를 시각적으로 표시합니다.
     */
    function updateSortIndicators() {
      document.querySelectorAll('th.sortable').forEach(th => {
        th.classList.remove('asc', 'desc');
        if (th.dataset.sort === state.sortColumn) {
          th.classList.add(state.sortOrder.toLowerCase());
        }
      });
    }

    // --- 5. Event Handlers & Listeners ---
    /**
     * 추가/수정 모달 창을 엽니다.
     * @param {string} title - 모달 창의 제목
     * @param {object|null} entity - 수정할 데이터 객체. 추가일 경우 null.
     */
    function openModal(title, entity = null) {
      elements.resourceForm.reset();
      elements.modalTitle.textContent = title;
      elements.resourceIdInput.value = entity ? entity[entityId] : '';
      
      // 엔티티 데이터가 있을 때 폼 필드에 값 설정
      if (entity) {
        for (const key in entity) {
          const input = elements.resourceForm.elements[key];
          if (input) {
            input.value = entity[key];
            
            // factory_idx 필드에 값이 설정된 후 change 이벤트 트리거 (3단 연동용)
            if (key === 'factory_idx' && input.value) {
              // line_idx 값도 저장해두기 (후에 복원용)
              const lineValue = entity['line_idx'];
              
              // 짧은 지연 후 change 이벤트 발생시켜 Line 업데이트
              setTimeout(() => {
                const changeEvent = new Event('change', { bubbles: true });
                // 복원할 line 값을 이벤트 데이터로 전달
                changeEvent.preserveLineValue = lineValue;
                input.dispatchEvent(changeEvent);
                console.log(`🔄 factory_idx change 이벤트 트리거됨: ${input.value}, Line 복원 값: ${lineValue}`);
              }, 100);
            }
            
            // line_idx 필드에 값이 설정된 후 change 이벤트 트리거 (추가 연동용)
            if (key === 'line_idx' && input.value) {
              setTimeout(() => {
                const changeEvent = new Event('change', { bubbles: true });
                input.dispatchEvent(changeEvent);
                console.log(`🔄 line_idx change 이벤트 트리거됨: ${input.value}`);
              }, 200);
            }
          }
        }
        
        // Spectrum 컬러 피커 값 설정 (전역 함수 호출)
        if (typeof window.updateAndonFormSpectrum === 'function') {
          setTimeout(() => {
            window.updateAndonFormSpectrum(entity);
          }, 300);
        }
      }
      
      // SAP Fiori 모달 시스템: .show 클래스를 추가해서 모달을 표시
      elements.modal.classList.add('show');
    }

    /**
     * 모달 창을 닫습니다.
     */
    function closeModal() { 
      // SAP Fiori 모달 시스템: .show 클래스를 제거해서 모달을 숨김
      elements.modal.classList.remove('show'); 
    }

    /**
     * HTML 문자열을 이스케이프 처리하여 XSS 공격을 방지합니다.
     * @param {string} unsafe - 이스케이프 처리할 문자열
     * @returns {string} 이스케이프 처리된 안전한 문자열
     */
    function escapeHtml(unsafe) {
      return unsafe?.toString()
        .replace(/&/g, "&amp;").replace(/</g, "&lt;").replace(/>/g, "&gt;")
        .replace(/"/g, "&quot;").replace(/'/g, "&#039;") ?? '';
    }

    /**
     * 페이지의 모든 주요 이벤트 리스너를 설정합니다.
     */
    function setupEventListeners() {
        // 모달의 '닫기' 버튼 클릭 시 (X 버튼)
        elements.closeModalBtn.addEventListener('click', closeModal);
        
        // 모달의 'Close' 버튼 클릭 시 (폼 하단 버튼)
        if (elements.modalCloseBtn) {
            elements.modalCloseBtn.addEventListener('click', closeModal);
        }
        
        // 'Add New' 버튼 클릭 시 새 데이터 추가 모달 열기
        if (elements.addBtn) {
            elements.addBtn.addEventListener('click', () => {
                openModal(`Add New ${resourceName}`);
            });
        }
        
        // 'Refresh' 버튼 클릭 시 필터 조건 유지하며 데이터 새로고침
        if (elements.refreshBtn) {
            elements.refreshBtn.addEventListener('click', () => {
                fetchData(); // 현재 필터 상태를 유지한채 데이터만 다시 가져오기
            });
        }
        
        // '컬럼' 버튼 클릭 시 드롭다운 토글
        elements.columnToggleBtn.addEventListener('click', (e) => { 
            e.stopPropagation(); 
            elements.columnToggleDropdown.classList.toggle('show'); 
        });
        
        // 컬럼 드롭다운에서 체크박스 변경 시
        elements.columnToggleDropdown.addEventListener('change', (e) => {
            if (e.target.classList.contains('column-toggle-checkbox')) {
                e.stopPropagation(); // 체크박스 클릭 시 드롭다운이 닫히지 않도록 이벤트 전파 중단
                const key = e.target.dataset.key;
                if (e.target.checked) state.visibleColumns.add(key); else state.visibleColumns.delete(key);
                applyColumnVisibility();
                saveColumnState();
            }
        });

        // 드롭다운 내부 클릭 시 닫히지 않도록 방지
        elements.columnToggleDropdown.addEventListener('click', (e) => {
            e.stopPropagation(); // 드롭다운 내부 클릭 시 이벤트 전파 중단
        });

        // 드롭다운 닫기 버튼 클릭 시
        elements.columnToggleDropdown.addEventListener('click', (e) => {
            if (e.target.classList.contains('dropdown-close-btn')) {
                elements.columnToggleDropdown.classList.remove('show');
            }
        });

        // '엑셀 다운로드' 버튼 클릭 시 (버튼이 존재하는 경우에만)
        if (elements.excelDownloadBtn) {
            elements.excelDownloadBtn.addEventListener('click', () => {
                const params = { sort: state.sortColumn, order: state.sortOrder };
                filterConfig.forEach(f => { params[f.paramName] = state[f.stateKey]; });
                api.export(params);
            });
        }

        // 테이블 헤더 클릭으로 정렬 기능 수행
        elements.tableHeader.addEventListener('click', e => {
            const header = e.target.closest('th.sortable');
            if (!header) return;
            const sortColumn = header.dataset.sort;
            state.sortOrder = (state.sortColumn === sortColumn && state.sortOrder === 'asc') ? 'desc' : 'asc';
            state.sortColumn = sortColumn;
            state.currentPage = 1;
            fetchData();
        });

        // 페이지네이션 컨트롤 클릭 시
        elements.paginationControls.addEventListener('click', e => {
            const page = e.target.dataset.page;
            if (page && state.currentPage !== +page) { 
                state.currentPage = +page; 
                fetchData().then(() => {
                    // 데이터 로딩 후 중앙 정렬 다시 적용
                    centerActiveButton();
                });
            }
        });

        // 모달 폼 제출 시 (추가/수정)
        elements.resourceForm.addEventListener('submit', async (e) => {
            e.preventDefault();
            const formData = new FormData(e.target);
            
            // 파일 업로드 감지
            const fileInput = e.target.querySelector('input[type="file"]');
            const hasFileUpload = fileInput && fileInput.files && fileInput.files.length > 0;
            
            let data;
            
            if (hasFileUpload || e.target.enctype === 'multipart/form-data') {
                // multipart 폼인 경우 항상 FormData 사용
                data = formData;
            } else {
                // 일반 데이터만 있는 경우 plain object로 변환
                data = {};
                for (let [key, value] of formData.entries()) {
                    data[key] = value;
                }
            }
            
            const id = elements.resourceIdInput.value;
            
            try {
                const result = id ? await api.update(id, data) : await api.create(data);
                
                if (result.success) { 
                    closeModal(); 
                    fetchData();
                } else { 
                    alert(`Error: ${result.message}`); 
                }
            } catch (error) { 
                alert(`Request error: ${error.message}`); 
            }
        });

        // 테이블 row 클릭 시 edit modal 열기
        elements.tableBody.addEventListener('click', async (e) => {
            // 파일 미리보기 아이콘 클릭인 경우 무시
            if (e.target.classList.contains('file-preview-icon')) {
                console.log('resource-manager: 파일 미리보기 아이콘 클릭 감지, 이벤트 무시');
                return;
            }

            // 삭제 버튼 클릭인 경우 무시
            if (e.target.closest('.delete-btn')) {
                console.log('resource-manager: 삭제 버튼 클릭 감지, 이벤트 무시');
                return;
            }

            // 데이터 row 클릭 감지 (NO. 컬럼이나 빈 데이터 row는 제외)
            const dataRow = e.target.closest('.data-row');
            if (!dataRow) return;
            
            const id = dataRow.dataset.id;
            if (!id) return;
            
            console.log('resource-manager: 테이블 행 클릭, 수정 모달 열기');
            
            try {
                const result = await api.getOne(id);
                if (result.success) {
                    // beforeEdit 콜백이 있으면 먼저 실행합니다.
                    if (config.beforeEdit) {
                        await config.beforeEdit(api, result.data);
                    }
                    openModal(`Edit ${resourceName}`, result.data);
                } else alert(`Error: ${result.message}`);
            } catch (error) { alert(`Request error: ${error.message}`); }
        });

        // 컬럼 드롭다운 외부 클릭 시 닫기
        window.addEventListener('click', (e) => {
            if (!elements.columnToggleBtn.contains(e.target) && !elements.columnToggleDropdown.contains(e.target)) {
                elements.columnToggleDropdown.classList.remove('show');
            }
        });

        // 모달 배경 클릭 시 닫기 비활성화 (다른 영역 클릭 시 모달이 닫히지 않도록 변경됨)
        // elements.modal.addEventListener('click', (e) => {
        //     if (e.target === elements.modal) {
        //         closeModal();
        //     }
        // });

        // 설정에 정의된 모든 커스텀 필터에 대해 이벤트 리스너 설정
        filterConfig.forEach(f => {
            const filterElement = document.getElementById(f.elementId);
            if (filterElement) {
                filterElement.addEventListener('change', (e) => {
                    state[f.stateKey] = e.target.value;
                    state.currentPage = 1;

                    // 종속 필터 리셋 로직: 이 필터가 변경될 때 다른 필터를 초기화해야 하는 경우
                    if (f.resets && Array.isArray(f.resets)) {
                        f.resets.forEach(stateKeyToReset => {
                            const filterToReset = filterConfig.find(c => c.stateKey === stateKeyToReset);
                            if (filterToReset) {
                                state[filterToReset.stateKey] = filterToReset.defaultValue || '';
                                const elementToReset = document.getElementById(filterToReset.elementId);
                                if (elementToReset) {
                                    elementToReset.value = filterToReset.defaultValue || '';
                                }
                            }
                        });
                    }

                    fetchData();
                });
            }
        });
    }

    // --- 6. Initialization ---
    /**
     * 리소스 관리자 전체를 초기화하고 실행합니다.
     */
    async function initialize() {
        // 설정에 beforeInit 함수가 있으면 먼저 실행 (비동기 작업이 끝날 때까지 기다림)
        if (config.beforeInit) {
            await config.beforeInit(api);
        }
        // 나머지 초기화 작업 수행
        loadColumnState();
        renderTableHeader();
        renderColumnToggler();
        setupEventListeners();
        fetchData();
        // 설정에 afterInit 함수가 있으면 마지막에 실행
        if (config.afterInit) {
            config.afterInit(api);
        }
    }

    // 초기화 함수 실행
    initialize();
    
    // resourceManagerInstance에 실제 함수들 할당
    resourceManagerInstance.loadData = fetchData;
    resourceManagerInstance.api = api;
    resourceManagerInstance.state = state;
    resourceManagerInstance.refreshData = fetchData;
    resourceManagerInstance.openModal = openModal;
    resourceManagerInstance.closeModal = closeModal;
    resourceManagerInstance.isInitialized = true;
    
    // ResourceManager 초기화 완료
  }

  // DOM이 이미 로드되었는지 확인하고 적절한 시점에 초기화 실행
  if (document.readyState === 'loading') {
    // DOM이 아직 로드 중이면 DOMContentLoaded 이벤트를 기다림
    document.addEventListener('DOMContentLoaded', initializeResourceManager);
  } else {
    // DOM이 이미 로드되었으면 즉시 실행
    initializeResourceManager();
  }
  
  // 인스턴스를 즉시 반환 (함수들은 초기화 후 할당됨)
  return resourceManagerInstance;
}