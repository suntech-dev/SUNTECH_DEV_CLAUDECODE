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
  - 디바운싱 딜레이: `#define DEBOUNCE_MS (50u)`
  - 경고 표시 시간: `#define WARNING_DISPLAY_MS (2000u)`
  - 버튼 텍스트 제한: `#define MAX_NUM_BUTTON_STRING (24u)`
- **메시지 문자열**도 `#define` 상수로 분리 (하드코딩 금지)
- unsigned 상수에는 `u` 접미사 붙이기: `50u`, `2000u`

### 변수 및 함수 선언
- **미사용 변수/함수 즉시 제거** (주석 처리 후 방치 금지)
- 전역 변수 선언 시 타입·변수명 컬럼 정렬 (가독성)
- 빈 매개변수 함수는 `void` 명시: `void func(void)`
- 함수 원형(prototype)과 정의의 매개변수 목록 일치

### 버퍼 안전성
- `ShowMessage()` 등 가변 인자 포맷 함수에서 버퍼 크기는 **충분히 크게** 설정
  - 포맷 문자열 + 최대 숫자 길이를 계산 후 여유 있게 설정 (최소 64바이트 권장)
- 배열 인덱스 증가 전 경계 검사: `if (count < BUF_SIZE - 1) buf[count++] = ch;`

### LCD UI 코딩 규칙
- **WiFi 기능 비활성화**: 이 프로젝트는 WiFi 미사용 — `DrawWifi()` 호출 및 WiFi 초기화 코드 제거 유지
- `g_TitleBar` 너비는 `g_SCREEN_WIDTH - 1` (전체 너비) 사용 — WiFi 영역 할당 금지
- 버튼 텍스트 길이 제한: `MAX_NUM_BUTTON_STRING 24` — 타이틀 텍스트 길이 초과 시 상수 조정
- 경고 메시지 표시 후 **반드시** 화면 복원: `EraseBlankAreaWithoutHeader()` → `g_updateCountMenu = 2`

### RESET 로직 규칙
- RESET 조건은 **반드시** `count == setTrimCount` (Target 도달 시에만 리셋)
- `count != 0` 조건으로의 완화 **금지** (부정 리셋 보안 취약점)
- Target 미달성 RESET 시도: 경고음 + LCD 메시지 + 2초 대기 + 화면 복원 시퀀스 유지

### Copyright
- 모든 소스 파일 상단에 Copyright 블록 포함
- 형식: `Copyright SUNTECH, 2018-YYYY`

---

## 버전 히스토리 관리 (`VERSION_HISTORY.md`)

- 버전 히스토리 파일: `VERSION_HISTORY.md` (프로젝트 루트)
- **새 버전 폴더가 생성되거나 새 버전 작업이 시작될 때**, `VERSION_HISTORY.md`에 해당 버전 섹션을 자동으로 추가해야 한다.
- 섹션 형식은 기존 버전과 동일한 구조를 유지한다:

```
## 280CTP2_KVC_TRIM_RESET_COUNTER_VN

| 항목 | 내용 |
...

### 메모리 사용량
### 하드웨어 구성
### 주요 기능
### 변경 이력
```

- 버전 식별 규칙: 폴더명 `280CTP2_KVC_TRIM_RESET_COUNTER_V{번호}` 기준
- 변경 이력은 날짜(YYYY-MM-DD) 및 변경 내용을 항목별로 기술
- 메모리 사용량은 빌드 성공 후 PSoC Creator 빌드 리포트에서 확인하여 기록

---

## README.md 버전별 업데이트 규칙

새 버전이 추가될 때 `README.md`의 아래 항목을 함께 갱신해야 한다.

### 1. 문서 헤더 메타데이터 (파일 최상단)

```
> 분석 버전: V{번호} (설명)
> 마지막 빌드: YYYY-MM-DD (성공)
> 마지막 코드 개선: YYYY-MM-DD
```

### 2. 섹션 4 — 버전 비교 테이블

새 버전의 기능 차이를 V{이전} vs V{신규} 형식으로 비교 행 추가.

### 3. 섹션 4.2 — 메모리 사용량 비교

빌드 리포트 기준으로 신규 버전의 SRAM/Flash 수치를 프로그레스 바 포함 갱신.

### 4. 섹션 11 — 버전 이력 테이블

새 버전에서 변경된 날짜·내용을 행으로 추가:

```
| YYYY-MM-DD | V{번호} 설명 | 변경 내용 요약 |
```

### 5. 섹션 12 — 관련 파일 경로 빠른 참조

파일 경로의 버전 폴더명을 새 버전으로 갱신.

---

## `.md` → `.html` 변환 스타일 가이드

`*.md` 파일을 HTML로 변환할 때 **`README.html`을 기준 템플릿**으로 삼아 아래 규칙을 일관되게 적용한다.

### 레이아웃
- 좌측 고정 사이드바 TOC + 우측 메인 콘텐츠 (`flex` 레이아웃)
- 모바일(≤768px): 사이드바 → 햄버거 버튼으로 토글, 오버레이 처리
- 사이드바 TOC: `IntersectionObserver`로 현재 섹션 자동 하이라이트

### 테마 & 색상 (CSS 변수)
- 다크 테마 고정 (GitHub 스타일): `--bg:#0d1117`, `--accent:#58a6ff`, `--accent2:#3fb950`
- 폰트: `-apple-system`, `'Noto Sans KR'` (본문) / `'Cascadia Code'`, `Consolas` (코드)

### UI 컴포넌트 규칙
| 요소 | 처리 방식 |
|------|-----------|
| 섹션 제목 | 번호 뱃지(`.num`) + 하단 border |
| 테이블 | `.tbl-wrap`으로 감싸 가로 스크롤 처리 |
| 코드 블록 | `pre > code`, 다크 배경, 모노스페이스 폰트 |
| 수치/상태 | 카드(`.card`) + 프로그레스 바, 상태 뱃지(`.tag.ok/.warn/.error`) |
| 버전 비교 | `.compare-grid` 2열 그리드 (`.compare-card.v1` / `.compare-card.v2`) |
| 메모리 바 | `.mem-bar` + `.mem-fill.ok/.warn/.danger` |

### 파일 명명
- 변환 결과 파일명: 원본 md 파일명 그대로 확장자만 `.html`로 변경 (예: `README.md` → `README.html`)
