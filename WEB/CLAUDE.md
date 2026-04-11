# CLAUDE.md — WEB 프로젝트 공통 규칙

---

## 코드 작성 공통 규칙

- 최대한 간결한 코드로 만들어서 유지보수성을 높이자.
- 모든 경로는 `$_SERVER['SCRIPT_NAME']` 기반 동적 경로 사용.
- 최대한 외부 CDN 사용.
- 이모지 사용 안함 (사용자가 명시적으로 요청한 경우에만 허용).
- **들여쓰기**: 4칸 스페이스
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

#### PDO Named Placeholder 규칙 (HY093 방지)

**SQLSTATE[HY093]: Invalid parameter number** 오류는 두 가지 원인으로 발생한다.
서버 PHP 구버전(7.0~7.3)은 로컬 PHP 7.4+보다 엄격하게 파라미터 개수를 체크한다.

**원인 1 — 동일 named placeholder 중복 사용 (서브쿼리 등)**
```php
// 잘못된 예: :mac이 SQL에 2번 등장 → 토큰 6개, 바인딩 3개 → HY093
$stmt = $pdo->prepare("
    SELECT (SELECT ... WHERE mac = :mac AND shift_idx = :shift_idx) as a,
           (SELECT ... WHERE mac = :mac AND shift_idx = :shift_idx) as b
");
$stmt->execute([':mac' => $mac, ':shift_idx' => $shift_idx]); // HY093!

// 올바른 예: 두 번째 서브쿼리에 별도 파라미터명 사용
$stmt = $pdo->prepare("
    SELECT (SELECT ... WHERE mac = :mac  AND shift_idx = :shift_idx ) as a,
           (SELECT ... WHERE mac = :mac2 AND shift_idx = :shift_idx2) as b
");
$stmt->execute([':mac' => $mac, ':shift_idx' => $shift_idx,
                ':mac2' => $mac, ':shift_idx2' => $shift_idx]);
```

**원인 2 — execute()에 SQL에 없는 extra 파라미터 전달**
```php
// 잘못된 예: $base_params(22개) + 추가(12개) = 34개인데 UPDATE SQL 토큰은 23개 → HY093
$stmt->execute($base_params + [':runtime' => $val, ':idx' => $id]);

// 올바른 예: UPDATE SQL에 실제로 있는 파라미터만 추출해서 전달
$update_params = array_intersect_key($base_params, array_flip([
    ':time_update', ':planned_work_time', ':work_hour', // SQL에 있는 것만 열거
]));
$stmt->execute($update_params + [':runtime' => $val, ':idx' => $id]);
```

> **주의**: 로컬 PHP 7.4는 extra params를 묵시적으로 무시하여 에러가 안 나도,
> 서버 PHP 7.0~7.3은 엄격하게 체크하여 HY093 발생. 항상 정확히 일치시킬 것.

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

다음 중 하나라도 해당되면 **아래 3개 파일을 자동으로 생성**한다:

1. 새 프로젝트 폴더가 생성된 후 **처음으로 분석/작업 요청**이 들어올 때
2. 프로젝트 폴더에 `CLAUDE.md`, `README.md`, `VERSION_HISTORY.md` 중 **하나라도 없을 때**
3. 사용자가 "새 프로젝트 시작", "초기화", "문서 만들어줘", "필수 파일들" 등을 요청할 때

### 생성 대상 위치

```
C:\SUNTECH_DEV_CLAUDECODE\WEB\{프로젝트명}\       <- 이 레벨에만 생성
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

## Playwright 테스트 자동화 정책

- Playwright를 사용한 브라우저 테스트(navigate, screenshot, snapshot, click, wait 등)는 **사용자 허락 없이 자동 진행**한다.
- 테스트 중 발견된 오류는 스스로 수정 후 재테스트한다.
- **스크린샷 저장 경로**: `C:\SUNTECH_DEV_CLAUDECODE\.palywright_screen_shot\` (절대경로 고정)
- **파일 형식**: JPEG (`.jpeg`)
- 이 폴더는 `.gitignore`에 등록되어 있으므로 Git 추적 제외.

### Playwright 실행 환경 (확인 불필요 — 고정값)

| 항목 | 값 |
| ---- | -- |
| Node.js | v22.18.0 |
| Playwright | v1.58.2 |
| 설치 위치 | `C:\SUNTECH_DEV_CLAUDECODE\node_modules\playwright` |
| 실행 바이너리 | `C:\SUNTECH_DEV_CLAUDECODE\node_modules\.bin\playwright` |
| Chromium | `C:\Users\luvsd\AppData\Local\ms-playwright\chromium-1208` |
| package.json | `C:\SUNTECH_DEV_CLAUDECODE\package.json` |

### Playwright 스크립트 실행 규칙

- **작업 디렉토리**: 반드시 `C:\SUNTECH_DEV_CLAUDECODE\` 에서 `node script.js` 로 실행
- **브라우저 실행**: `chromium.launch({ headless: true })` — 항상 headless
- **스크린샷**: `page.screenshot({ path: '...', type: 'jpeg' })` — type 명시 필수
- **버전 확인 금지**: 위 환경 정보가 최신값이므로 매 실행 시 버전/경로 재확인 불필요
- **임시 스크립트**: 검증 후 삭제하지 않아도 됨 (`.palywright_screen_shot/` 폴더와 함께 gitignore 처리)

### Playwright 인라인 스크립트 템플릿

```javascript
// 실행: node script.js (C:\SUNTECH_DEV_CLAUDECODE\ 에서)
const { chromium } = require('playwright');

(async () => {
    const browser = await chromium.launch({ headless: true });
    const page = await browser.newPage();
    await page.setViewportSize({ width: 1920, height: 1080 });

    await page.goto('http://localhost/dev/...');
    await page.waitForLoadState('networkidle');

    await page.screenshot({
        path: 'C:\\SUNTECH_DEV_CLAUDECODE\\.palywright_screen_shot\\name.jpeg',
        type: 'jpeg'
    });

    // 검증 예시
    const result = await page.evaluate(() => {
        const el = document.getElementById('targetId');
        return { height: el?.offsetHeight, gridTemplateRows: el?.style.gridTemplateRows };
    });
    console.log(JSON.stringify(result));

    await browser.close();
})();
```

---

## 로컬 개발 환경 (Laragon)

- **PHP**: 7.4.33 (`C:\laragon\bin\php\php-7.4.33-nts-Win32-vc15-x64\`)
- **MySQL**: Laragon 내장 MySQL (root / 비밀번호 없음)
- **Xdebug**: 포트 9003, `xdebug.start_with_request=yes`
- **로컬 URL**: `http://localhost/dev/{프로젝트명}/{버전폴더}/` 또는 `http://{프로젝트명}.test`
- **Laragon 매핑**: `C:\laragon\www\dev` → `C:\SUNTECH_DEV_CLAUDECODE\WEB` (junction)
- **SFTP**: host `49.247.26.228`, port 22, user `root`, remotePath `/var/www/html`, uploadOnSave: true

### 경로 → URL 변환 규칙 (필수)

> `WEB\` 경로에는 `dev`가 없지만, URL에는 반드시 `/dev/`가 포함된다.

```
C:\SUNTECH_DEV_CLAUDECODE\WEB\{프로젝트명}\{버전폴더}\
→ http://localhost/dev/{프로젝트명}/{버전폴더}/
```

- `WEB\` = `http://localhost/dev/` (1:1 대응)
- Bash로 URL 점검 시 `/dev/` 누락 금지
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

- **날짜 형식**: `YYYY-MM-DD`
- **문서 헤더**: 각 md 파일 최상단에 `> 최초 작성:`, `> 분석 버전:`, `> 마지막 업데이트:` 포함

---

## HTML → PDF 변환 정책

### 변환 도구
- **Chrome headless** 사용 (`C:\Program Files\Google\Chrome\Application\chrome.exe`)
- Playwright `page.pdf()` 사용 불가 — MCP Playwright는 브라우저 컨텍스트 JS만 실행 가능
- Node.js `child_process.execSync`로 Chrome headless 호출

### 변환 절차

1. **print CSS 삽입** — Node.js 스크립트에서 원본 HTML을 읽어 `</style>` 직전에 삽입 후 임시 파일로 저장
2. **Chrome headless 실행** — `--print-background` 옵션으로 배경색 포함 PDF 생성
3. **임시 파일 삭제** — 변환 후 `os.tmpdir()` 임시 HTML 삭제
4. **스크립트 삭제** — 작업 완료 후 `gen_pdf.js` 삭제 (일회성 스크립트)

### 필수 print CSS 규칙 (다크 테마 HTML 공통)

```css
@media print {
  html, body {
    background: #0d1117 !important;
    -webkit-print-color-adjust: exact !important;
    print-color-adjust: exact !important;
  }
  #sidebar, #menu-btn, #overlay { display: none !important; }
  #main { margin-left: 0 !important; }
  .hero    { padding: 40px 32px 32px !important; break-inside: avoid; }
  .content { padding: 0 32px !important; }
  section, .feature-card, .phase-card, .card, .tbl-wrap { break-inside: avoid; }
  a { color: #58a6ff !important; text-decoration: none; }
}
```

### Chrome headless 명령 옵션

```
chrome.exe
  --headless=new
  --no-sandbox
  --disable-gpu
  --print-background          ← 배경색 포함 필수
  --run-all-compositor-stages-before-draw
  --print-to-pdf="출력경로.pdf"
  "file:///임시HTML경로"
```

### 출력 파일 규칙
- PDF 저장 위치: 원본 HTML과 동일 폴더
- 파일명: 원본 HTML 파일명 그대로 확장자만 `.pdf`로 변경
  - 예: `AI_STRATEGY_V2_ENG.html` → `AI_STRATEGY_V2_ENG.pdf`

### gen_pdf.js 템플릿 (재사용)

```javascript
const fs = require('fs');
const path = require('path');
const { execSync } = require('child_process');
const os = require('os');

const SRC_HTML = 'C:\\...\\file.html';
const OUT_PDF  = SRC_HTML.replace('.html', '.pdf');
const CHROME   = 'C:\\Program Files\\Google\\Chrome\\Application\\chrome.exe';

const PRINT_CSS = `/* @media print { ... } */`;
const TMP_HTML  = path.join(os.tmpdir(), 'print_tmp.html');

try {
  let html = fs.readFileSync(SRC_HTML, 'utf8');
  html = html.replace('</style>', PRINT_CSS + '\n</style>');
  fs.writeFileSync(TMP_HTML, html, 'utf8');

  const fileUrl = 'file:///' + TMP_HTML.replace(/\\/g, '/');
  execSync(`"${CHROME}" --headless=new --no-sandbox --disable-gpu --print-background --run-all-compositor-stages-before-draw --print-to-pdf="${OUT_PDF}" "${fileUrl}"`, { timeout: 30000 });

  console.log('PDF saved:', OUT_PDF, Math.round(fs.statSync(OUT_PDF).size / 1024) + 'KB');
} finally {
  if (fs.existsSync(TMP_HTML)) fs.unlinkSync(TMP_HTML);
}
```

---

## 기존 프로젝트 참조 경로

| 프로젝트 | 위치           | 참고 용도                    |
| -------- | -------------- | ---------------------------- |
| OEE_SCI  | `WEB\OEE_SCI\` | PHP/MySQL 웹 프로젝트 템플릿 |