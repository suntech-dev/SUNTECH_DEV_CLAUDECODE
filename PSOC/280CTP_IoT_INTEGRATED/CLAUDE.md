# CLAUDE.md — 280CTP_IoT_INTEGRATED

## C 코드 스타일 가이드 (PSoC4 임베디드 C)

### 들여쓰기 & 중괄호
- **들여쓰기**: 스페이스 4칸 (탭 사용 금지)
- **중괄호**: Allman 스타일 — 함수 및 제어문(`if`, `for`, `while`) 모두 중괄호를 다음 줄에 배치
- **단문 제어문도 반드시 중괄호** 사용 (`if (x) foo();` → `if (x) { foo(); }`)

### 타입 안전성
- `char` 타입을 배열 인덱스나 카운터로 사용 **금지** → `uint8_t` (또는 PSoC `uint8`) 사용
- `unsigned int`를 카운터로 사용할 때 오버플로우 가능성 검토
- ISR에서 접근하는 전역 변수는 **반드시 `volatile`** 선언

### 버퍼 안전성
- `sprintf()` 대신 **`snprintf(buf, sizeof(buf), ...)`** 사용 — URL 조합 포함
- `vsprintf()` 등 가변 인자 포맷 함수는 **`vsnprintf(buf, sizeof(buf), ...)`** 형태로 교체
- `UART_BUFFER_SIZE` (512), `MAX_WIFI_RECEIVE_BUFFER` (1024~2048) 초과 입력 방지
- JSON 파싱 전 응답 크기 검증 필수 (`jsmntok_t t[128]` 토큰 수 초과 주의)

### 포인터 & NULL 안전성
- `strtok()`, `malloc()` 등 포인터를 반환하는 함수 결과는 **반드시 NULL 체크** 후 사용
- `andonResponse()` 등 JSON 수신 콜백에서 파싱 실패 시 조기 반환 처리
- 포인터 선언 형식: `type *name` (스페이스는 포인터 기호 앞)

### 상수 정의
- **매직 넘버 금지**: 코드 내 숫자 리터럴은 `#define` 상수로 분리
  - `#define MAX_LIST 5` — ANDON 리스트 최대 항목 수
  - `#define MAX_COL_NOTICE 14`, `#define MAX_ROW_NOTICE 20` — 공지 배열 크기
  - `#define WIFI_STRENGTH_CHECK_TIME 60000` — WiFi 강도 체크 주기(ms)
- unsigned 상수에는 `u` 접미사 붙이기: `50u`, `99u`

### 변수 및 함수 선언
- **전역 변수 최소화**: 현재 50+ 개의 전역 변수 — 새로운 전역 변수 추가 자제
- **미사용 변수/함수 즉시 제거** (주석 처리 후 방치 금지)
- 전역 변수 선언 시 타입·변수명 컬럼 정렬 (가독성)
- 빈 매개변수 함수는 `void` 명시: `void func(void)`
- 헤더 파일에 전역 변수 **인스턴스 선언 금지** — `extern` 선언 + `.c` 파일 정의 분리
- `static const char g_strServeURL[]` 선언은 헤더에 두어 중복 정의 방지

### ANDON API 규칙
- 서버 API URL: `andonApi.h`의 `g_strServeURL` 상수만 사용 (`/api/sewing.php`)
- URL 조합: `sprintf` → `snprintf` 사용하여 버퍼 오버플로우 방지
  ```c
  // 금지
  sprintf(url, "%s?code=%s", g_strServeURL, aq->message);
  // 권장
  snprintf(url, sizeof(url), "%s?code=%s", g_strServeURL, aq->message);
  ```
- `aq->message` 등 외부 입력값은 URL 조합 전 길이 검증
- HTTP 요청 실패 시 재시도 횟수 제한 필수

### WiFi 통신 규칙
- **블로킹 루프 금지**: `while(g_wifi_cmd != WIFI_CMD_IDLE)` 패턴에서 타임아웃 반드시 설정
- WiFi 수신 버퍼: `g_WIFI_ReceiveBuffer[MAX_WIFI_RECEIVE_BUFFER+1]` — 인덱스 범위 검사
- SSID/Password는 외부 플래시(`externalFlash.c`)에만 저장 — 코드 내 하드코딩 금지
- 서버 IP, 포트, 경로도 외부 플래시 `CONFIG` 구조체로 관리

### JSON 파싱 규칙
- `jsmn_parse()` 반환값 반드시 체크 (음수 = 에러)
- 토큰 배열 크기: `jsmntok_t t[128]` — 128 초과 응답 시 잘림 발생, 필요 시 증가
- 파싱 후 키 탐색 시 `jsmneq()` 또는 `strncmp()` 사용 (NULL 종료 보장)

### checksum 구현 규칙
- `checkSum()` 루프는 반드시 `i=0`에서 시작 (첫 바이트 누락 방지)
- `externalFlashCRC()` 사용 시 CRC 검증 비활성화 금지

### 서버 설정 규칙
- `server.h`의 `DEFAULT_SERVER_IP`, `DEFAULT_SERVER_PATH`, `DEFAULT_SERVER_PORT` 는 **초기 기본값**만
- 실 운영 환경에서는 반드시 USB 설정 도구(`SuntechIoTConfig_V1`)로 외부 플래시에 저장
- HTTP → HTTPS 전환 시 포트 80 → 443으로 변경, SSL 라이브러리 추가 필요

### 메모리 관리 규칙
- `lib/image.h`, `lib/fonts.h` 의 배열에는 반드시 `const` 유지 — FLASH 배치를 위해
- `MAX_NO_OF_ACCESS_POINT` 현재 40 → 실제 환경에 맞게 10~15로 축소 권장
- SRAM 목표: 32,768 bytes의 70% 이하 유지 (최적화 후 71.5% 달성됨)

### 조건부 컴파일 구조 (package.h)
- 현재 활성 매크로: `USER_PROJECT_PATTERN_SEWING_MACHINE`
- 매크로 변경 시 `package.h` 수정 후 전체 재빌드 필요
- `//#define USE_CURRENT_SENSOR_FOR_COUNTTING` — 전류 센서 카운팅은 기본 비활성화

### Copyright
- 모든 소스 파일 상단에 Copyright 블록 포함
- 형식: `Copyright SUNTECH, YYYY`

---

## 버전 히스토리 관리 (`VERSION_HISTORY.md`)

- 버전 히스토리 파일: `VERSION_HISTORY.md` (프로젝트 루트)
- **새 버전 폴더가 생성되거나 새 버전 작업이 시작될 때**, `VERSION_HISTORY.md`에 해당 버전 섹션을 추가한다.
- 섹션 형식:

```
## 280CTP_IoT_INTEGRATED_VN

| 항목 | 내용 |
...

### 메모리 사용량
### 하드웨어 구성
### 주요 기능
### 변경 이력
```

- 버전 식별 규칙: 폴더명 `280CTP_IoT_INTEGRATED_V{번호}` 기준
- 변경 이력은 날짜(YYYY-MM-DD) 및 변경 내용을 항목별로 기술
- 메모리 사용량은 빌드 성공 후 PSoC Creator 빌드 리포트에서 확인하여 기록

---

## README.md 버전별 업데이트 규칙

새 버전이 추가될 때 `README.md`의 아래 항목을 함께 갱신한다.

### 1. 문서 헤더 메타데이터 (파일 최상단)

```
> 분석 버전: V{번호} (설명)
> 마지막 빌드: YYYY-MM-DD (성공)
> 마지막 코드 개선: YYYY-MM-DD
```

### 2. 섹션 2 — 버전 구조 폴더 트리

새 버전 폴더를 트리에 추가.

### 3. 섹션 4 — 버전 비교 테이블

새 버전의 기능 차이를 V{이전} vs V{신규} 형식으로 비교 행 추가.

### 4. 섹션 9 — 발견된 이슈 및 개선 이력

수정된 이슈는 완료 처리, 신규 이슈 추가.

### 5. 섹션 12 — 버전 이력 테이블

```
| YYYY-MM-DD | V{번호} | 변경 내용 요약 |
```

### 6. 섹션 13 — 관련 파일 경로 빠른 참조

파일 경로의 버전 폴더명을 새 버전으로 갱신.

---

## `.md` → `.html` 변환 스타일 가이드

`README.md`를 HTML로 변환할 때 아래 규칙을 일관되게 적용한다.

### 레이아웃
- 좌측 고정 사이드바 TOC + 우측 메인 콘텐츠 (`flex` 레이아웃)
- 모바일(≤768px): 사이드바 → 햄버거 버튼으로 토글, 오버레이 처리
- 사이드바 TOC: `IntersectionObserver`로 현재 섹션 자동 하이라이트

### 테마 & 색상 (CSS 변수 — 변경 금지)
```css
--bg:         #0d1117;
--surface:    #161b22;
--surface2:   #21262d;
--border:     #30363d;
--text:       #e6edf3;
--text-muted: #8b949e;
--accent:     #58a6ff;
--accent2:    #3fb950;
--warn:       #d29922;
--danger:     #f85149;
```

### UI 컴포넌트 규칙

| 요소 | 처리 방식 |
|------|-----------|
| 섹션 제목 | 번호 뱃지(`.num`) + 하단 border |
| 테이블 | `.tbl-wrap`으로 감싸 가로 스크롤 처리 |
| 코드 블록 | `pre > code`, 다크 배경, 모노스페이스 폰트 |
| 수치/상태 | 카드(`.card`) + 프로그레스 바, 상태 뱃지(`.tag.ok/.warn/.error`) |
| 버전 비교 | `.compare-grid` 2열 그리드 |
| 메모리 바 | `.mem-bar` + `.mem-fill.ok/.warn/.danger` |
| 이슈 심각도 | `.tag.error` (심각), `.tag.warn` (높음), `.tag.ok` (낮음) |
