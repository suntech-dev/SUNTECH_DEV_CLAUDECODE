# CLAUDE.md — OEE_SCI 프로젝트 규칙

> 이 파일은 `C:\SUNTECH_DEV_CLAUDECODE\WEB\OEE_SCI` 프로젝트에 적용되는 규칙입니다.

---

## PHP 코드 스타일 가이드

### 인코딩
- **인코딩**: UTF-8 (BOM 없음)

### 보안 (추가 규칙)
- 동적 테이블명은 화이트리스트(`in_array`) 검증 후 사용
- `$_REQUEST`, `$_GET`, `$_POST` 입력값은 반드시 검증/trim 처리

### API 응답 형식
```php
// 성공
echo json_encode(['code' => '00', 'msg' => 'success', ...]);

// 실패
echo json_encode(['code' => '99', 'msg' => 'error message']);
```

### SSE (Server-Sent Events) 패턴
```php
header('Content-Type: text/event-stream');
header('Cache-Control: no-cache');
header('Connection: keep-alive');
if (ob_get_level()) ob_end_clean();
echo "data: " . json_encode($data) . "\n\n";
flush();
```

### SSE Stream 공통 라이브러리 (`lib/stream_helper.lib.php`)

모든 SSE stream 파일은 아래 공통 라이브러리를 사용한다. 로컬 중복 정의 금지.

```php
require_once(__DIR__ . '/../../../lib/stream_helper.lib.php');

// sendSSEData($eventType, $data)
sendSSEData('oee_data', $payload);

// parseFilterParams($tableAlias, $dateColumn, $isDateOnly, $defaultInterval)
// 반환: ['where_sql' => string, 'params' => array]
$filter = parseFilterParams('do', 'work_date', true, '7 DAY');
$stmt = $pdo->prepare("SELECT ... FROM data_oee do" . $filter['where_sql']);
$stmt->execute($filter['params']);

// getWorkHoursForDate($pdo, $targetDate) — Worktime 클래스 래퍼
$hours = getWorkHoursForDate($pdo, $targetDate);
```

**파일별 parseFilterParams 호출 규칙:**

| 파일 | tableAlias | dateColumn | isDateOnly | defaultInterval |
| ---- | ---------- | ---------- | ---------- | --------------- |
| data_oee_stream | `do` | `work_date` | true | `7 DAY` |
| log_oee_stream | `do` | `work_date` | true | `7 DAY` |
| log_oee_hourly_stream | `doh` | `work_date` | true | `7 DAY` |
| log_oee_row_stream | `dor` | `work_date` | true | `7 DAY` |
| oee_report_stream | `do` | `work_date` | true | `7 DAY` |
| data_downtime_stream | `dd` | `reg_date` | false | `2 DAY` |
| data_andon_stream | `da` | `reg_date` | false | `2 DAY` |
| data_defective_stream | `dd` | `reg_date` | false | `2 DAY` |

**주의**: `dashboard_stream.php`는 날짜 로직이 다르므로 내부 `parseDashboardFilterParams` 사용 (stream_helper의 `parseFilterParams`와 인터페이스 불일치).

### 타임존 설정 규칙
- **`date_default_timezone_set('Asia/Jakarta')`는 `lib/db.php` 단 한 곳에만 선언한다.**
- 개별 파일에 중복 선언 금지 — `require_once db.php` 시 자동 적용됨
- 클래스 생성자에서 `$pdo`가 필요한 경우: `global $pdo; $this->pdo = $pdo;`
- `lib/worktime_database.php`는 삭제됨 — `lib/db.php`로 대체

### 파일 경로 규칙
- 절대경로: `__DIR__ . '/../../lib/db.php'` 형식 사용

---

## 헤더 파일 분리 규칙 (`inc/head.php` vs `inc/worktime_head.php`)

### 사용 구분

| 헤더 파일 | 사용 페이지 | CSS/JS 스택 |
| --------- | ----------- | ----------- |
| `inc/head.php` | 모든 일반 관리/데이터 페이지 | SAP Fiori CSS + `fiori-advanced-interactions.js` |
| `inc/worktime_head.php` | 근무시간 관리 페이지 (`info_worktime.php` 등) | jQuery DataTables + daterangepicker + `worktime_style.css` |

### worktime_head.php — 보존 대상

- **절대 수정/삭제/통합 금지**
- 수년간 사용자가 다듬어온 캘린더·팝업 UI(`daterangepicker`, 다크 테마)가 이 파일에 묶여 있음
- Fiori 디자인 시스템으로 동등한 수준의 UI 재구현이 불가능하여 별도 유지

### common.js 충돌 여부 (분석 완료 — 2026-03-07)

- `assets/js/common.js`는 `worktime_head.php`에서만 로드됨 (jQuery `$()` 의존)
- `fiori-advanced-interactions.js`는 `head.php`에서만 로드됨 (순수 JS IIFE 패턴)
- 두 환경은 완전 분리 — **직접 충돌 없음** (동일 페이지에서 혼재 구조 없음)
- `common.js`의 전역 함수(`showDialogWindow`, `hideDialogWindow` 등)는 Fiori 네임스페이스와 충돌하지 않음

---

## 버전 히스토리 관리 (`VERSION_HISTORY.md`)

- 버전 히스토리 파일: `VERSION_HISTORY.md` (프로젝트 루트 `OEE_SCI/`)
- **새 버전 폴더가 생성되거나 새 버전 작업이 시작될 때**, `VERSION_HISTORY.md`에 해당 버전 섹션을 자동으로 추가해야 한다.
- 섹션 형식은 기존 버전과 동일한 구조를 유지한다:

```markdown
## OEE_SCI_V{번호}

| 항목          | 내용 |
| ------------- | ---- |
| 버전 식별자   | OEE_SCI_V{번호} |
| 프로젝트 폴더 | OEE_SCI\OEE_SCI_V{번호} |
| 작업일        | YYYY-MM-DD |
| 상태          | 개발 중 / 운영 배포 권장 / 구버전 |

### 기술 스택
### 주요 기능
### 변경 이력
```

- 버전 식별 규칙: 폴더명 `OEE_SCI_V{번호}` 기준
- 변경 이력은 날짜(YYYY-MM-DD) 및 변경 내용을 항목별로 기술

---

## README.md 버전별 업데이트 규칙

새 버전이 추가될 때 `README.md`의 아래 항목을 함께 갱신해야 한다.

### 1. 문서 헤더 메타데이터 (파일 최상단)

```
> 분석 버전: OEE_SCI_V{번호}
> 마지막 업데이트: YYYY-MM-DD
```

### 2. 섹션 2 — 버전 구조

새 버전 폴더(`OEE_SCI_V{번호}/`)를 폴더 트리에 추가.

### 3. 섹션 10 — 버전 이력 테이블

새 버전에서 변경된 날짜·내용을 행으로 추가:
```
| YYYY-MM-DD | 변경 내용 요약 |
```

### 4. 섹션 11 — 관련 파일 경로 빠른 참조

파일 경로의 버전 폴더명을 새 버전으로 갱신.

---

## `.md` -> `.html` 변환 스타일 가이드

`*.md` 파일을 HTML로 변환할 때 아래 규칙을 일관되게 적용한다.

### 레이아웃
- 좌측 고정 사이드바 TOC + 우측 메인 콘텐츠 (`flex` 레이아웃)
- 모바일(<=768px): 사이드바 -> 햄버거 버튼으로 토글, 오버레이 처리
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
| 요소      | 처리 방식                                         |
| --------- | ------------------------------------------------- |
| 섹션 제목 | 번호 뱃지(`.num`) + 하단 border                   |
| 테이블    | `.tbl-wrap`으로 감싸 가로 스크롤 처리             |
| 코드 블록 | `pre > code`, 다크 배경, 모노스페이스 폰트        |
| 수치/상태 | 카드(`.card`) + 상태 뱃지(`.tag.ok/.warn/.error`) |
| 버전 이력 | `.timeline` + `.timeline-item`                    |
| 아키텍처  | CSS Grid -- ASCII art 사용 금지                   |

### 파일 명명
- 변환 결과 파일명: 원본 md 파일명 그대로 확장자만 `.html`로 변경
