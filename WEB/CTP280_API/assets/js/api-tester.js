/**
 * API 테스터 - 재봉기 OEE 시스템의 API를 쉽게 테스트할 수 있는 도구
 * 초보자 설명: 이 파일은 서버의 API들을 웹 페이지에서 쉽게 테스트할 수 있게 해주는 도구입니다.
 */

// 페이지가 완전히 로드된 후 실행되는 코드
document.addEventListener('DOMContentLoaded', function () {
  
  console.log('API 테스터가 시작되었습니다.');
  
  // HTML에서 필요한 요소들을 찾아서 변수에 저장
  // getElementById: HTML에서 특정 ID를 가진 요소를 찾는 함수
  const viewer = document.getElementById('viewer');              // API 결과를 보여줄 영역
  const buttonsContainer = document.getElementById('api-buttons-container'); // 버튼들이 들어갈 영역
  const errorContainer = document.getElementById('error-container');         // 오류 메시지를 보여줄 영역

  // 서버에서 사용 가능한 API 목록을 가져와서 동적으로 버튼을 만듭니다
  console.log('서버에서 API 목록을 가져오는 중...');
  
  fetch('./api/get_api_list.php')
    .then(function(response) {
      // fetch: 서버에 데이터를 요청하는 함수
      // .then: 요청이 완료되면 실행되는 코드
      
      // 서버 응답이 정상인지 확인
      if (!response.ok) {
        throw new Error('서버에서 오류가 발생했습니다. 상태코드: ' + response.status);
      }
      
      console.log('API 목록을 성공적으로 받았습니다.');
      return response.json(); // JSON 형태로 데이터 변환
    })
    .then(function(data) {
      // 받아온 API 데이터를 사용해서 버튼들을 만들어 화면에 표시
      console.log('받아온 API 데이터:', data);
      
      // data 객체의 각 그룹(예: '조회 API', '전송 API')별로 처리
      for (const groupName in data) {
        // 해당 그룹에 API가 하나라도 있는지 확인
        if (data[groupName].length > 0) {
          
          // fieldset: 관련된 요소들을 묶어주는 HTML 태그
          const fieldset = document.createElement('fieldset');
          const legend = document.createElement('legend');
          legend.textContent = groupName; // 그룹 제목 설정
          fieldset.appendChild(legend);

          // 버튼들을 격자 형태로 배치할 컨테이너
          const buttonGrid = document.createElement('div');
          buttonGrid.className = 'button-grid';

          // 각 API에 대해 버튼을 하나씩 생성
          data[groupName].forEach(function(api) {
            // 새로운 버튼 요소 생성
            const button = document.createElement('button');
            button.textContent = api.name; // 버튼에 표시될 텍스트
            
            // 버튼을 클릭했을 때 실행될 함수 지정
            // 화살표 함수(=>)를 일반 함수로 변경하여 초보자가 이해하기 쉽게
            button.onclick = function() {
              loadPage(api.path, button);
            };
            
            // 만든 버튼을 버튼 그리드에 추가
            buttonGrid.appendChild(button);
            
            console.log('API 버튼 생성됨:', api.name);
          });

          // 버튼 그리드를 fieldset에 추가
          fieldset.appendChild(buttonGrid);
          // fieldset을 전체 컨테이너에 추가
          buttonsContainer.appendChild(fieldset);
        }
      }
      // 모든 버튼이 생성되면 첫 번째 버튼을 자동으로 클릭해서 첫 페이지 로드
      const firstButton = buttonsContainer.querySelector('button');
      if (firstButton) {
        console.log('첫 번째 API 버튼을 자동으로 클릭합니다.');
        firstButton.click();
      } else {
        // API 버튼이 하나도 없는 경우 오류 메시지 표시
        console.log('사용 가능한 API가 없습니다.');
        displayError('사용 가능한 API가 없습니다.');
      }
    })
    .catch(function(error) {
      // fetch 요청이 실패했거나 오류가 발생한 경우 처리
      console.error('API 목록을 가져오는 중 오류 발생:', error);
      displayError('API 목록을 불러오는데 실패했습니다. 서버 상태를 확인해주세요.');
    });

  /**
   * 특정 API 테스트 페이지를 로드하여 화면에 표시하는 함수
   * 초보자 설명: 버튼을 클릭하면 해당 API 테스트 페이지를 가져와서 화면에 보여주는 함수입니다.
   * @param {string} path - 로드할 페이지의 경로 (예: '../api/api_test/start.html')
   * @param {HTMLElement} element - 클릭된 버튼 요소
   */
  function loadPage(path, element) {
    // 경로가 없으면 함수를 종료
    if (!path) {
      console.log('페이지 경로가 없습니다.');
      return;
    }

    console.log('페이지 로딩 중:', path);
    
    // 로딩 중 메시지를 표시
    viewer.innerHTML = '<p style="text-align: center; padding: 2rem; color: #666;">페이지를 불러오는 중...</p>';

    // 서버에서 HTML 페이지를 가져옵니다
    fetch(path)
      .then(function(response) {
        // 응답이 정상인지 확인
        if (!response.ok) {
          throw new Error('페이지를 불러올 수 없습니다. 상태코드: ' + response.status);
        }
        return response.text(); // HTML 텍스트로 변환
      })
      .then(function(htmlString) {
        console.log('페이지 로딩 완료:', path);
        // DOMParser: HTML 문자열을 실제 HTML 요소로 변환하는 도구
        const parser = new DOMParser();
        const doc = parser.parseFromString(htmlString, 'text/html');
        
        // 가져온 페이지의 body 내용을 viewer에 삽입
        viewer.innerHTML = doc.body.innerHTML;

        // 초보자 설명: HTML 페이지 안에 있는 자바스크립트도 실행할 수 있도록 처리
        // 가져온 HTML에 포함된 스크립트들을 실행 가능한 상태로 만들어야 합니다.
        const scriptsToExecute = Array.from(viewer.getElementsByTagName('script'));
        
        console.log('실행할 스크립트 개수:', scriptsToExecute.length);
        
        // 각 스크립트를 새로 생성해서 실행 가능하게 만듭니다
        scriptsToExecute.forEach(function(oldScript) {
          // 새로운 스크립트 요소 생성
          const newScript = document.createElement('script');
          
          // 원본 스크립트의 모든 속성들을 새 스크립트에 복사
          // (예: type, async 등의 속성)
          Array.from(oldScript.attributes).forEach(function(attr) {
            newScript.setAttribute(attr.name, attr.value);
          });
          
          // 스크립트의 실제 코드 내용을 복사
          newScript.textContent = oldScript.textContent;
          
          // 원본 스크립트를 새로운 스크립트로 교체 (이렇게 해야 실행됨)
          oldScript.parentNode.replaceChild(newScript, oldScript);
        });
      })
      .catch(function(error) {
        // 페이지 로딩 중 오류가 발생한 경우
        console.error('페이지 로딩 오류:', error);
        viewer.innerHTML = '<p style="color: red; text-align: center; padding: 2rem;">페이지를 불러오는 중 오류가 발생했습니다: ' + error.message + '</p>';
      });

    // 버튼 스타일 업데이트: 현재 선택된 버튼을 강조 표시
    
    // 먼저 모든 버튼에서 'active' 클래스를 제거 (선택 해제)
    const allButtons = document.querySelectorAll('#api-buttons-container button');
    allButtons.forEach(function(btn) {
      btn.classList.remove('active');
    });
    
    // 클릭된 버튼에만 'active' 클래스를 추가 (선택 상태로 만듦)
    if (element) {
      element.classList.add('active');
      console.log('활성 버튼 변경:', element.textContent);
    }
  }

  /**
   * 사용자에게 오류 메시지를 표시하는 함수
   * 초보자 설명: 뭔가 문제가 생겼을 때 사용자가 알 수 있도록 메시지를 보여주는 함수입니다.
   * @param {string} message - 화면에 표시할 오류 메시지
   */
  function displayError(message) {
    console.log('오류 메시지 표시:', message);
    
    // 버튼 컨테이너를 숨기고 오류 메시지를 표시
    buttonsContainer.style.display = 'none';  // 버튼들을 화면에서 숨김
    errorContainer.textContent = message;      // 오류 메시지 텍스트 설정
    errorContainer.style.display = 'block';    // 오류 메시지를 화면에 보여줌
  }
  
  console.log('API 테스터 초기화 완료');
});
