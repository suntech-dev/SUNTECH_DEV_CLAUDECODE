# CLAUDE.md — WEB 프로젝트 공통 규칙

---

## 코드 작성 공통 규칙

- 최대한 간결한 코드로 만들어서 유지보수성을 높이자.
- 모든 경로는 `$_SERVER['SCRIPT_NAME']` 기반 동적 경로 사용.
- 최대한 외부 CDN 사용.
- 이모지 사용 안함 (사용자가 명시적으로 요청한 경우에만 허용).
- **들여쓰기**: 2칸 스페이스
- **코드**: 영문 작성, 핵심 섹션만 한글 주석
- **인코딩**: 코드 내 한글 주석 최소화 (인코딩 문제 방지)

---

## Core Libraries

- `lib/config.php`: Database configuration
- `lib/db.php`: PDO connection handling

---

## Development Rules

### Database Operations

```php
// Always use prepared statements
$stmt = $pdo->prepare("SELECT * FROM table WHERE column = ?");
$stmt->execute([$value]);
$result = $stmt->fetch(PDO::FETCH_ASSOC);
```

### Security

- **모든 쿼리는 PDO prepared statement 사용 필수**
- 환경변수(`$_ENV`) 우선 사용, .env 파일 로드 지원
- DB 인증정보 하드코딩 금지 (config.php에만 fallback 허용)

---

## UI/UX Design

- **Design System**: SAP Fiori Horizon Light theme only
- **UI Framework**: SAP Fiori Design System
- PHP 7.4+, MySQL 5.7+ 필수
- 빌드 시스템 없음 (순수 PHP)
- 외부 프레임워크 없음

---

## 새 프로젝트 폴더 초기화 규칙

### 트리거 조건

다음 중 하나라도 해당되면 **아래 4개 파일을 자동으로 생성**한다:

1. 새 프로젝트 폴더가 생성된 후 **처음으로 분석/작업 요청**이 들어올 때
2. 프로젝트 폴더에 `CLAUDE.md`, `README.md`, `README.html`, `VERSION_HISTORY.md` 중 **하나라도 없을 때**
3. 사용자가 "새 프로젝트 시작", "초기화", "문서 만들어줘", "필수 4개 파일들" 등을 요청할 때

### 생성 대상 위치

```
C:\SUNTECH_DEV_CLAUDECODE\WEB\{프로젝트명}\       <- 이 레벨에만 생성
    CLAUDE.md
    README.md
    README.html
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
## PHP 코드 스타일 가이드
## 버전 히스토리 관리 (VERSION_HISTORY.md)
## README.md 버전별 업데이트 규칙
## .md -> .html 변환 스타일 가이드
```

**기준 템플릿**: `C:\SUNTECH_DEV_CLAUDECODE\WEB\OEE_SCI\CLAUDE.md`

**버전 폴더명 규칙**: `{프로젝트명}_V{번호}` 형식으로 자동 반영

---

### 2. README.md

**목적**: 프로젝트 전체 분석 문서 (한글)

**필수 섹션 (번호 순서)**:
```
1. 프로젝트 개요 (주요 기능 표)
2. 버전 구조 (폴더 트리)
3. 기술 스택
4. 소스코드 상세 분석
5. API 구조 (해당 시)
6. 데이터베이스 구조 (해당 시)
7. 로컬 개발 환경 설정
8. 발견된 이슈 및 개선 이력
9. 테스트 시나리오
10. 버전 이력 테이블
11. 관련 파일 경로 빠른 참조
```

**기준 템플릿**: `C:\SUNTECH_DEV_CLAUDECODE\WEB\OEE_SCI\README.md`

**작성 언어**: 한글 (코드 블록, 변수명, 파일명 제외)

---

### 3. README.html

**목적**: README.md의 HTML 변환본 — 브라우저에서 바로 열람 가능한 문서

**레이아웃 규칙**:
- 좌측 고정 사이드바 TOC + 우측 메인 콘텐츠 (`flex` 레이아웃)
- 모바일(<=768px): 햄버거 버튼으로 사이드바 토글, 오버레이 처리
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
| 요소      | 처리 방식                                |
| --------- | ---------------------------------------- |
| 섹션 제목 | 번호 뱃지(`.num`) + 하단 border          |
| 테이블    | `.tbl-wrap` 가로 스크롤 처리             |
| 코드 블록 | `pre > code`, 다크 배경, 모노스페이스    |
| 수치/상태 | 카드 + 상태 뱃지(`.tag.ok/.warn/.error`) |
| 버전 이력 | `.timeline` + `.timeline-item`           |
| 아키텍처  | CSS Grid -- ASCII art 사용 금지          |

**기준 템플릿**: `C:\SUNTECH_DEV_CLAUDECODE\WEB\OEE_SCI\README.html`

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

### 기술 스택
### 주요 기능
### 변경 이력        (날짜별 항목)
```

**기준 템플릿**: `C:\SUNTECH_DEV_CLAUDECODE\WEB\OEE_SCI\VERSION_HISTORY.md`

---

## 로컬 개발 환경 (Laragon)

- **PHP**: 7.4.33 (`C:\laragon\bin\php\php-7.4.33-nts-Win32-vc15-x64\`)
- **MySQL**: Laragon 내장 MySQL (root / 비밀번호 없음)
- **Xdebug**: 포트 9003, `xdebug.start_with_request=yes`
- **로컬 URL**: `http://localhost/dev/{프로젝트명}/{버전폴더}/` 또는 `http://{프로젝트명}.test`
- **Laragon 매핑**: `C:\laragon\www\dev` → `C:\SUNTECH_DEV_CLAUDECODE\WEB` (junction)
- **환경변수**: 프로젝트 루트에 `.env` 파일로 DB 설정 오버라이드

**.env 파일 예시 (로컬 개발용)**:
```
DB_HOST=localhost
DB_USERNAME=root
DB_PASSWORD=
DB_NAME={데이터베이스명}
```

---

## 문서 작성 공통 규칙

- **언어**: 모든 `.md` 파일은 **한글**로 작성 (코드, 변수명, 파일명 제외)
- **날짜 형식**: `YYYY-MM-DD`
- **문서 헤더**: 각 md 파일 최상단에 `> 최초 작성:`, `> 분석 버전:`, `> 마지막 업데이트:` 포함

---

## 기존 프로젝트 참조 경로

| 프로젝트 | 위치           | 참고 용도                    |
| -------- | -------------- | ---------------------------- |
| OEE_SCI  | `WEB\OEE_SCI\` | PHP/MySQL 웹 프로젝트 템플릿 |