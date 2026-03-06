/**
 * 재봉기 OEE 모니터링 시스템 - 메인 자바스크립트 파일
 * 초보자도 쉽게 이해할 수 있도록 한글 주석으로 설명합니다.
 */

// 시스템 시작 메시지를 콘솔에 출력 (개발자 도구에서 확인 가능)
console.log('재봉기 OEE 모니터링 시스템에 오신 것을 환영합니다!');

/**
 * 페이지가 완전히 로드된 후 실행되는 코드
 * DOMContentLoaded 이벤트: HTML이 모두 로드되고 파싱이 완료되면 실행
 */
document.addEventListener('DOMContentLoaded', function() {
  // 현재 시간을 화면에 표시하는 함수 실행
  showCurrentTime();
  
  // 10초마다 시간을 업데이트 (10000밀리초 = 10초)
  setInterval(showCurrentTime, 10000);
  
  // 시스템 상태를 확인하는 함수 실행
  checkSystemStatus();
  
  console.log('메인 자바스크립트가 성공적으로 로드되었습니다.');
});

/**
 * 현재 시간을 화면에 표시하는 함수
 * 초보자 설명: 함수는 특정 작업을 수행하는 코드 블록입니다.
 */
function showCurrentTime() {
  // 현재 날짜와 시간을 가져옵니다
  const now = new Date();
  
  // 시간을 사람이 읽기 쉬운 형태로 변환
  const timeString = now.toLocaleString('ko-KR', {
    year: 'numeric',    // 년도 (숫자로)
    month: '2-digit',   // 월 (2자리 숫자)
    day: '2-digit',     // 일 (2자리 숫자)
    hour: '2-digit',    // 시 (2자리 숫자)
    minute: '2-digit',  // 분 (2자리 숫자)
    second: '2-digit'   // 초 (2자리 숫자)
  });
  
  // HTML에서 시간을 표시할 요소를 찾습니다
  const timeElement = document.getElementById('current-time');
  
  // 요소가 존재하면 시간을 업데이트
  if (timeElement) {
    timeElement.textContent = timeString;
  }
}

/**
 * 시스템 상태를 확인하는 함수
 * API 서버가 정상 작동하는지 확인합니다.
 */
function checkSystemStatus() {
  // 서버에 간단한 요청을 보내서 상태 확인
  fetch('../api/index.php?code=get_dateTime')
    .then(function(response) {
      // 서버 응답이 정상인지 확인
      if (response.ok) {
        console.log('서버 상태: 정상');
        updateStatusDisplay('정상', 'green');
      } else {
        console.log('서버 상태: 오류 - ', response.status);
        updateStatusDisplay('오류', 'red');
      }
    })
    .catch(function(error) {
      // 네트워크 오류나 기타 문제가 발생한 경우
      console.log('서버 연결 실패:', error);
      updateStatusDisplay('연결 실패', 'red');
    });
}

/**
 * 시스템 상태 표시를 업데이트하는 함수
 * @param {string} status - 상태 텍스트 (예: '정상', '오류')
 * @param {string} color - 상태 색상 (예: 'green', 'red')
 */
function updateStatusDisplay(status, color) {
  // 상태를 표시할 HTML 요소 찾기
  const statusElement = document.getElementById('system-status');
  
  // 요소가 존재하면 상태 업데이트
  if (statusElement) {
    statusElement.textContent = '시스템 상태: ' + status;
    statusElement.style.color = color;
  }
}

/**
 * 사용자에게 알림 메시지를 표시하는 함수
 * @param {string} message - 표시할 메시지
 * @param {string} type - 메시지 타입 ('success', 'error', 'info')
 */
function showNotification(message, type) {
  // type 기본값 설정
  type = type || 'info';
  
  // 알림 요소 생성
  const notification = document.createElement('div');
  notification.className = 'notification ' + type;
  notification.textContent = message;
  
  // 페이지에 알림 추가
  document.body.appendChild(notification);
  
  // 3초 후에 알림 제거
  setTimeout(function() {
    if (notification.parentNode) {
      notification.parentNode.removeChild(notification);
    }
  }, 3000);
}

/**
 * 로컬 스토리지에 데이터를 저장하는 함수
 * 초보자 설명: 로컬 스토리지는 브라우저에 데이터를 저장하는 기능입니다.
 * @param {string} key - 저장할 데이터의 키(이름)
 * @param {any} value - 저장할 값
 */
function saveToLocalStorage(key, value) {
  try {
    // 객체나 배열은 JSON 문자열로 변환해서 저장
    const valueToStore = typeof value === 'object' ? JSON.stringify(value) : value;
    localStorage.setItem(key, valueToStore);
    console.log('데이터 저장 완료:', key);
  } catch (error) {
    console.error('데이터 저장 실패:', error);
  }
}

/**
 * 로컬 스토리지에서 데이터를 불러오는 함수
 * @param {string} key - 불러올 데이터의 키
 * @return {any} 저장된 데이터 (없으면 null)
 */
function loadFromLocalStorage(key) {
  try {
    const storedValue = localStorage.getItem(key);
    
    // 값이 없으면 null 반환
    if (storedValue === null) {
      return null;
    }
    
    // JSON 형태로 저장된 데이터라면 객체로 변환
    try {
      return JSON.parse(storedValue);
    } catch {
      // JSON이 아니면 원본 문자열 반환
      return storedValue;
    }
  } catch (error) {
    console.error('데이터 로드 실패:', error);
    return null;
  }
}