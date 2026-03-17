# CLAUDE.md — SUNTECH_DEV_CLAUDECODE 전역 규칙

> 이 파일은 `C:\SUNTECH_DEV_CLAUDECODE` 하위 **모든 프로젝트**에 공통 적용되는 규칙입니다.
> Claude Code는 CLAUDE.md를 디렉토리 계층 순서로 읽으므로, 이 규칙은 하위 모든 폴더에서 자동 적용됩니다.

---

## 새 프로젝트 폴더 초기화 규칙

### 트리거 조건

다음 중 하나라도 해당되면 **아래 3개 파일을 자동으로 생성**한다:

1. 새 프로젝트 폴더가 생성된 후 **처음으로 분석/작업 요청**이 들어올 때
2. 프로젝트 폴더에 `CLAUDE.md`, `README.md`, `VERSION_HISTORY.md` 중 **하나라도 없을 때**
3. 사용자가 "새 프로젝트 시작", "초기화", "문서 만들어줘", "필수 파일들" 등을 요청할 때

### 생성 대상 위치

```
C:\SUNTECH_DEV_CLAUDECODE\{분류}\{프로젝트명}\   ← 이 레벨에만 생성
    CLAUDE.md
    README.md
    VERSION_HISTORY.md
```

> **주의**: V1, V2 등 하위 버전 폴더에는 생성하지 않는다.
> 프로젝트명 폴더(최상위 레벨)에만 생성한다.

---

## 파일별 생성 규칙

### 1. CLAUDE.md

**목적**: 해당 프로젝트의 코딩 규칙, 버전 관리 규칙, 문서 변환 스타일 정의

**필수 섹션**:
```
## C 코드 스타일 가이드 (프로젝트 타입에 맞게)
## 버전 히스토리 관리 (VERSION_HISTORY.md)
## README.md 버전별 업데이트 규칙
## .md → .html 변환 스타일 가이드
```

**기준 템플릿**:
- PSoC4 임베디드: `C:\SUNTECH_DEV_CLAUDECODE\PSOC\280CTP2_KVC_TRIM_RESET_COUNTER\CLAUDE.md`
- 스캐너 통신 보드: `C:\SUNTECH_DEV_CLAUDECODE\PSOC\SMALL_BOARD_SCANNER\CLAUDE.md`
- 웹 프로젝트: `C:\SUNTECH_DEV_CLAUDECODE\WEB\CLAUDE.md`

**버전 폴더명 규칙**: `{프로젝트명}_V{번호}` 형식으로 자동 반영

---

### 2. README.md

**목적**: 프로젝트 전체 분석 문서 (한글)

**필수 섹션 (번호 순서)**:
```
1. 프로젝트 개요 (주요 기능 표)
2. 버전 구조 (폴더 트리)
3. 하드웨어 정보 (PSoC4) 또는 기술 스택 (웹)
4. 버전 비교 (V1 vs V2 등, 해당 시)
5. 소스코드 상세 분석
6. (프로젝트 특화 섹션)
7. 부트로더 / 배포 구조
8. 발견된 이슈 및 개선 이력
9. 테스트 시나리오
10. 개발 환경 설정
11. 버전 이력 테이블
12. 관련 파일 경로 빠른 참조
```

**기준 템플릿**:
- `C:\SUNTECH_DEV_CLAUDECODE\PSOC\280CTP2_KVC_TRIM_RESET_COUNTER\README.md`
- `C:\SUNTECH_DEV_CLAUDECODE\PSOC\SMALL_BOARD_SCANNER\README.md`

**작성 언어**: 한글 (코드 블록 제외)

---

### 3. README.html

**목적**: README.md의 HTML 변환본 — 브라우저에서 바로 열람 가능한 문서

**레이아웃 규칙**:
- 좌측 고정 사이드바 TOC + 우측 메인 콘텐츠 (`flex` 레이아웃)
- 모바일(≤768px): 햄버거 버튼으로 사이드바 토글, 오버레이 처리
- `IntersectionObserver`로 현재 섹션 TOC 자동 하이라이트

**테마 & 색상 (CSS 변수 — 변경 금지)**:
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

**UI 컴포넌트 규칙**:
| 요소      | 처리 방식                                               |
| --------- | ------------------------------------------------------- |
| 섹션 제목 | 번호 뱃지(`.num`) + 하단 border                         |
| 테이블    | `.tbl-wrap` 가로 스크롤 처리                            |
| 코드 블록 | `pre > code`, 다크 배경, 모노스페이스                   |
| 수치/상태 | 카드 + 프로그레스 바, 상태 뱃지(`.tag.ok/.warn/.error`) |
| 버전 비교 | `.compare-grid` 2열 그리드                              |
| 메모리 바 | `.mem-bar` + `.mem-fill.ok/.warn/.danger`               |

**기준 템플릿**:
- `C:\SUNTECH_DEV_CLAUDECODE\PSOC\280CTP2_KVC_TRIM_RESET_COUNTER\README.html`
- `C:\SUNTECH_DEV_CLAUDECODE\PSOC\SMALL_BOARD_SCANNER\README.html`

---

### 4. VERSION_HISTORY.md

**목적**: 버전별 변경 이력 추적 문서

**필수 섹션 (버전당 반복)**:
```markdown
## {프로젝트명}_V{번호}

| 항목          | 내용                              |
| ------------- | --------------------------------- |
| 버전 식별자   | ...                               |
| 프로젝트 폴더 | ...                               |
| 작업일        | YYYY-MM-DD                        |
| 상태          | 개발 중 / 운영 배포 권장 / 구버전 |

### 메모리 사용량     (PSoC4인 경우)
### 기술 스택        (웹인 경우)
### 하드웨어 구성    (PSoC4인 경우)
### 주요 기능
### 변경 이력        (날짜별 항목)
```

**기준 템플릿**:
- `C:\SUNTECH_DEV_CLAUDECODE\PSOC\280CTP2_KVC_TRIM_RESET_COUNTER\VERSION_HISTORY.md`
- `C:\SUNTECH_DEV_CLAUDECODE\PSOC\SMALL_BOARD_SCANNER\VERSION_HISTORY.md`

---

## 프로젝트 타입 판별 규칙

프로젝트 폴더 분석 후 아래 기준으로 타입을 자동 판별한다:

| 판별 기준                                 | 프로젝트 타입      | 적용 템플릿                              |
| ----------------------------------------- | ------------------ | ---------------------------------------- |
| `*.cydsn`, `*.cyprj`, `*.cywrk` 파일 존재 | **PSoC4 임베디드** | SMALL_BOARD_SCANNER 또는 280CTP2 기준    |
| `package.json`, `*.ts`, `*.vue` 존재      | **웹/Node.js**     | WEB/CLAUDE.md 기준                       |
| `*.py`, `requirements.txt` 존재           | **Python**         | 범용 템플릿                              |
| 판별 불가                                 | **범용**           | 공통 구조 적용, 기술 스택 섹션 자유 기재 |

### PSoC4 임베디드 세부 판별

| 판별 기준                                                     | 세부 타입     |
| ------------------------------------------------------------- | ------------- |
| `bootloader.cydsn` + `MainFunction.cydsn` 또는 통신 관련 파일 | 통신 브리지형 |
| `Design.cydsn` + LCD/터치 관련 (`widget.c`, `button.c`)       | LCD UI 제어형 |
| `Design.cydsn` + WiFi/JSON 관련 파일                          | IoT 연결형    |

---

## 문서 작성 공통 규칙

- **언어**: 모든 `.md` 파일은 **한글**로 작성 (코드, 변수명, 파일명 제외)
- **날짜 형식**: `YYYY-MM-DD`
- **이모지**: 사용자가 명시적으로 요청한 경우에만 사용
- **버전 폴더명**: `{프로젝트명}_V{번호}` (예: `MY_PROJECT_V1`, `MY_PROJECT_V2`)
- **문서 헤더**: 각 md 파일 최상단에 `> 최초 작성:`, `> 분석 버전:`, `> 마지막 업데이트:` 포함

---

## 기존 프로젝트 참조 경로 빠른 참조

| 프로젝트                       | 위치                                   | 참고 용도                              |
| ------------------------------ | -------------------------------------- | -------------------------------------- |
| SMALL_BOARD_SCANNER            | `PSOC\SMALL_BOARD_SCANNER\`            | 통신 브리지형 PSoC4 템플릿             |
| 280CTP2_KVC_TRIM_RESET_COUNTER | `PSOC\280CTP2_KVC_TRIM_RESET_COUNTER\` | LCD UI 제어형 PSoC4 + 버전 비교 템플릿 |
