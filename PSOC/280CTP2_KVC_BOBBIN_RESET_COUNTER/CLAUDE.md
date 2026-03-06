# CLAUDE.md — 280CTP2_KVC_BOBBIN_RESET_COUNTER

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
- `vsprintf()` 등 가변 인자 포맷 함수는 **`vsnprintf(buf, sizeof(buf), ...)`** 형태로 교체
- 고정 크기 스택 버퍼는 최대 예상 출력 길이 + 여유 (최소 64바이트 권장)
- `ShowMessage()` 등 LCD 출력 버퍼: 포맷 결과 + NULL 고려하여 충분히 설정

### 포인터 & NULL 안전성
- `strtok()`, `malloc()` 등 포인터를 반환하는 함수 결과는 **반드시 NULL 체크** 후 사용
- 포인터 선언 형식: `type *name` (스페이스는 포인터 기호 앞)

### 상수 정의
- **매직 넘버 금지**: 코드 내 숫자 리터럴은 `#define` 상수로 분리
  - 디바운싱 딜레이: `#define DEBOUNCE_MS (50u)`
  - 최대 트림 설정값: `#define MAX_SET_TRIM_COUNT (99u)`
  - 버튼 텍스트 제한: `#define MAX_NUM_BUTTON_STRING (24u)`
- unsigned 상수에는 `u` 접미사 붙이기: `50u`, `99u`

### 변수 및 함수 선언
- **미사용 변수/함수 즉시 제거** (주석 처리 후 방치 금지)
- 전역 변수 선언 시 타입·변수명 컬럼 정렬 (가독성)
- 빈 매개변수 함수는 `void` 명시: `void func(void)`
- 헤더 파일에 전역 변수 **인스턴스 선언 금지** — `extern` 선언 + `.c` 파일 정의 분리
- 조건부 컴파일(`#ifdef`) 내·외부에 동일 이름의 함수/변수 중복 정의 금지

### checksum 구현 규칙
- `checkSum()` 루프는 반드시 `i=0`에서 시작 (현재 `i=1` 버그 — 첫 바이트 누락)
- 무결성 검증 코드 주석 처리 금지 (`IsAvaliableInternalFalsh()` 워터마크/CRC 검사 활성화 유지)

### RESET 로직 규칙
- RESET 조건: `RESET_KEY_Read() == FALSE && g_ptrCount->count == g_ptrMachineParameter->setTrimCount`
- `count != 0` 조건으로의 완화 **금지** (부정 리셋 보안 취약점)
- 디바운싱은 `CyDelay()` 대신 `isFinishCounter_1ms()` 기반 비차단 방식 사용 권장

### LCD UI 코딩 규칙
- 현재 디스플레이 방향: **LANDSCAPE** 고정 (`setDisplayDirection(DISPLAY_DIRECTION_LANDSCAPE)`)
- 타이틀: `"KVC Bobbin Reset"` (변경 시 README.md 반영)
- 버튼 텍스트 길이 제한: `MAX_NUM_BUTTON_STRING 24` — 초과 시 상수 조정 후 문서 기록

### Copyright
- 모든 소스 파일 상단에 Copyright 블록 포함
- 형식: `Copyright SUNTECH, YYYY`

---

## 버전 히스토리 관리 (`VERSION_HISTORY.md`)

- 버전 히스토리 파일: `VERSION_HISTORY.md` (프로젝트 루트)
- **새 버전 폴더가 생성되거나 새 버전 작업이 시작될 때**, `VERSION_HISTORY.md`에 해당 버전 섹션을 추가한다.
- 섹션 형식:

```
## 280CTP2_KVC_BOBBIN_RESET_COUNTER_VN

| 항목 | 내용 |
...

### 메모리 사용량
### 하드웨어 구성
### 주요 기능
### 변경 이력
```

- 버전 식별 규칙: 폴더명 `280CTP2_KVC_BOBBIN_RESET_COUNTER_V{번호}` 기준
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

### 4. 섹션 8 — 발견된 이슈 및 개선 이력

수정된 이슈는 완료 처리, 신규 이슈 추가.

### 5. 섹션 11 — 버전 이력 테이블

```
| YYYY-MM-DD | V{번호} | 변경 내용 요약 |
```

### 6. 섹션 12 — 관련 파일 경로 빠른 참조

파일 경로의 버전 폴더명을 새 버전으로 갱신.

---

## `.md` → `.html` 변환 스타일 가이드

`README.md`를 HTML로 변환할 때 **`README.html`을 기준 템플릿**으로 삼아 아래 규칙을 일관되게 적용한다.

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
