# CLAUDE.md

## C 코드 스타일 가이드 (PSoC4 임베디드 C)

### 들여쓰기 & 중괄호
- **들여쓰기**: 스페이스 4칸 (탭 사용 금지)
- **중괄호**: Allman 스타일 — 함수 및 제어문(`if`, `for`, `while`) 모두 중괄호를 다음 줄에 배치
- **단문 제어문도 반드시 중괄호** 사용 (`if (x) foo();` → `if (x) { foo(); }`)

### 타입 안전성
- `char` 타입을 배열 인덱스나 카운터로 사용 **금지** → `uint8_t` (또는 PSoC `uint8`) 사용
- `unsigned int`를 카운터로 사용할 때 오버플로우 가능성 검토

### 포인터 & NULL 안전성
- `strtok()`, `malloc()` 등 포인터를 반환하는 함수 결과는 **반드시 NULL 체크** 후 사용
- 포인터 선언 형식: `type *name` (스페이스는 포인터 기호 앞)

### 상수 정의
- **매직 넘버 금지**: 코드 내 숫자 리터럴은 `#define` 상수로 분리
  - 버퍼 크기: `#define UART_BUF_SIZE (256u)`
  - 타임아웃: `#define TIMEOUT_MS_UART (50u)`
  - 임계값: `#define COUNTER_MIN_MS (200u)` / `#define COUNTER_MAX_MS (1900u)`
- **명령 문자열**도 `#define` 상수로 분리 (하드코딩 금지)
  - `#define SCAN_TRIGER_ORDER_STR "$$$$#99900035;%%%%"`
  - `#define SCAN_MODE_CMD_STR     "$$$$#99900304;%%%%"`
- unsigned 상수에는 `u` 접미사 붙이기: `256u`, `1000u`

### 변수 및 함수 선언
- **미사용 변수/함수 즉시 제거** (주석 처리 후 방치 금지)
- 전역 변수 선언 시 타입·변수명 컬럼 정렬 (가독성)
- 빈 매개변수 함수는 `void` 명시: `void func(void)`
- 함수 원형(prototype)과 정의의 매개변수 목록 일치

### 버퍼 안전성
- UART 수신 루프에서 **항상 버퍼에서 문자를 소비** (읽지 않으면 무한루프)
- 배열 인덱스 증가 전 경계 검사: `if (count < BUF_SIZE - 1) buf[count++] = ch;`
- 버퍼 초과 데이터는 폐기 처리

### 포트 역할 주의사항 (이 프로젝트 전용)
- `MONITORING` → **태블릿** (JSON 출력): `MONITORING_UartPutString()`
- `USB_OP` → **ST-500 Touch OP** (트리거/응답): `USB_OP_UartPutString()`, `USB_OP_UartGetChar()`
- `BARCODE` → **2D 바코드 스캐너**: `BARCODE_UartPutChar()`, `BARCODE_UartGetChar()`
- ⚠️ SMALL_BOARD_SCANNER와 Monitoring/OP 포트 역할이 **반대**이므로 혼동 주의

### Copyright
- 모든 소스 파일 상단에 Copyright 블록 포함
- 형식: `Copyright SUNTECH, 2018-YYYY`

---

## 버전 히스토리 관리 (`VERSION_HISTORY.md`)

- 버전 히스토리 파일: `VERSION_HISTORY.md` (프로젝트 루트)
- **새 버전 폴더가 생성되거나 새 버전 작업이 시작될 때**, `VERSION_HISTORY.md` 에 해당 버전 섹션을 자동으로 추가해야 한다.
- 섹션 형식은 기존 버전과 동일한 구조를 유지한다:

```
## SMALL_BOARD_TABLET_SCANNER_YYYY_VN

| 항목 | 내용 |
...

### 메모리 사용량
### UART 포트 구성
### 주요 기능
### 변경 이력
```

- 버전 식별 규칙: 폴더명 `SMALL_BOARD_TABLET_SCANNER_{연도}_V{번호}` 기준
- 변경 이력은 날짜(YY-MM-DD) 및 변경 내용을 항목별로 기술
- 메모리 사용량은 빌드 성공 후 PSoC Creator 빌드 리포트에서 확인하여 기록

---

## README.md 버전별 업데이트 규칙

새 버전이 추가될 때 `README.md` 의 아래 항목을 함께 갱신해야 한다.

### 1. 문서 헤더 메타데이터 (파일 최상단)

```
> 분석 버전: SMALL_BOARD_TABLET_SCANNER_{연도}_V{번호}
> 마지막 빌드: YYYY-MM-DD (성공)
> 마지막 코드 개선: YYYY-MM-DD
```

### 2. 섹션 2 — 하드웨어 정보

Flash/SRAM 사용량이 변경된 경우 해당 수치를 빌드 리포트 기준으로 갱신.

### 3. 섹션 3 — 프로젝트 구조

새 버전 폴더(`SMALL_BOARD_TABLET_SCANNER_{연도}_V{번호}/`)를 트리 구조에 추가.

### 4. 섹션 10 — 버전 이력 테이블

새 버전에서 변경된 날짜·내용을 행으로 추가:

```
| YYYY-MM-DD | 변경 내용 요약 |
```

### 5. 섹션 11 — 관련 파일 경로 빠른 참조

파일 경로의 버전 폴더명(`SMALL_BOARD_TABLET_SCANNER_YYYY_VN`)을 새 버전으로 갱신.

---

## `.md` → `.html` 변환 스타일 가이드

`*.md` 파일을 HTML로 변환할 때 **`c:\SUNTECH_DEV_CLAUDECODE\PSOC\SMALL_BOARD_TABLET_SCANNER\README.html`을 기준 템플릿**으로 삼아 아래 규칙을 일관되게 적용한다.

### 레이아웃
- 좌측 고정 사이드바 TOC + 우측 메인 콘텐츠 (`flex` 레이아웃)
- 모바일(≤900px): 사이드바 → 햄버거 버튼으로 토글, 오버레이 처리
- 사이드바 TOC: `IntersectionObserver`로 현재 섹션 자동 하이라이트

### 테마 & 색상 (CSS 변수)
- 다크 테마 고정 (GitHub 스타일): `--bg:#0d1117`, `--accent:#58a6ff`, `--accent2:#3fb950`
- 폰트: `-apple-system`, `'Noto Sans KR'` (본문) / `'JetBrains Mono'`, `Consolas` (코드)

### UI 컴포넌트 규칙
| 요소                | 처리 방식                                                       |
| ------------------- | --------------------------------------------------------------- |
| 섹션 제목           | 번호 뱃지(`.num`) + 하단 border                                 |
| 테이블              | `.table-wrap`으로 감싸 가로 스크롤 처리                         |
| 코드 블록           | `pre > code`, 다크 배경, 모노스페이스 폰트                      |
| 수치/상태           | 카드(`.card`) + 프로그레스 바, 상태 뱃지(`.status-ok/high/med`) |
| 타임라인            | `.timeline` + `.timeline-item` (버전 이력 등)                   |
| 아키텍처 다이어그램 | CSS Grid 6열 (`comp-grid-6`) — ASCII art 사용 금지              |

### 파일 명명
- 변환 결과 파일명: 원본 md 파일명 그대로 확장자만 `.html`로 변경 (예: `README.md` → `README.html`)
