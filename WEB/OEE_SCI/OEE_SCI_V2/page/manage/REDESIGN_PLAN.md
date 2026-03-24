# manage 페이지 1920x1080 풀스크린 리디자인 계획

> 최초 작성: 2026-03-24
> 모델 페이지: `page/data/ai_dashboard_5.php`
> 마지막 업데이트: 2026-03-24 (햄버거 드로어 공통 include 분리)

---

## 리디자인 원칙

### 핵심 규칙

1. **풀스크린 고정** — `html, body { height:100vh; overflow:hidden; }` 필수
2. **nav-fiori.php 제거** — 햄버거 드로어로 교체 (ai_dashboard_5.css 참조)
3. **Stats 카드 제거** — 모든 manage 페이지에서 통계 카드 사용 안함
4. **Footer 제거**
5. **파일명 규칙** — 변경 파일에만 `_2` 접미사 부여, 백엔드 proc 파일은 변경 없으면 원본 재사용
6. **들여쓰기** — 4칸 스페이스

---

## 레이아웃 구조 (공통 템플릿)

```
┌────────────────────────────────────────────────────────────┐ 52px
│ [☰]  {페이지 제목}      [검색] [필터들] [추가버튼] [새로고침] │ signage-header
└────────────────────────────────────────────────────────────┘
┌────────────────────────────────────────────────────────────┐ calc(100vh - 52px)
│ {테이블 제목}                      [퀵필터 버튼들]           │ card header ~42px
├────────────────────────────────────────────────────────────┤
│                                                            │
│  테이블 (thead 고정, tbody 내부 스크롤)                      │ flex:1
│                                                            │
├────────────────────────────────────────────────────────────┤
│ [페이지네이션]                                              │ ~40px
└────────────────────────────────────────────────────────────┘
```

---

## CSS 공통 패턴

```css
/* 전체 고정 */
html, body { height: 100vh; overflow: hidden; margin: 0; padding: 0; }

/* 메인 영역 */
.manage-main {
  height: calc(100vh - 52px);
  display: flex;
  flex-direction: column;
  padding: 6px;
  box-sizing: border-box;
  gap: 0;
}

/* 테이블 카드 풀높이 */
.manage-main .fiori-card {
  flex: 1;
  display: flex;
  flex-direction: column;
  overflow: hidden;
  margin-bottom: 0;
}
.manage-main .fiori-card__content {
  flex: 1;
  overflow-y: auto;
  padding: 0;
}

/* 페이지네이션 */
.manage-main .fiori-pagination {
  flex-shrink: 0;
  padding: 6px 0 0;
}
```

### signage-header (ai_dashboard_5.css 동일 패턴)

```css
.signage-header {
  height: 52px;
  background: var(--sap-surface-1);
  border-bottom: 1px solid var(--sap-border-neutral);
  display: flex;
  align-items: center;
  padding: 0 12px;
  gap: 10px;
  flex-shrink: 0;
  box-sizing: border-box;
}

/* 헤더 내 select: 고정 width 필수 (flex:1 환경에서 늘어남 방지) */
.signage-header .fiori-select {
  height: 30px;
  font-size: var(--sap-font-size-sm);
  padding: 0 6px;
  width: 130px;
  flex-shrink: 0;
}

/* 헤더 내 버튼 compact */
.signage-header .fiori-btn {
  height: 30px;
  padding: 0 10px;
  font-size: var(--sap-font-size-sm);
}

/* 헤더 내 검색창: search-icon 제거 → padding-left override */
.signage-header .search-container { max-width: 220px; }
.signage-header .search-input {
  height: 30px;
  padding-left: 0.5rem;
  font-size: var(--sap-font-size-sm);
}
```

### 햄버거 드로어 (ai_dashboard_5.css 동일 패턴)

- `.nav-drawer-btn`, `.nav-drawer-overlay`, `.nav-drawer` 스타일은 ai_dashboard_5.css 참조
- PHP에서 드로어 HTML + 스크립트 블록 복사 (ai_dashboard_5.php 참조)

---

## Excel Export (PhpSpreadsheet) 경로 규칙

proc 파일에서 export 기능을 구현할 때 `vendor/autoload.php` 경로에 주의한다.

```
proc 파일 위치: page/manage/proc/{파일}.php
vendor 위치:    OEE_SCI_V2/lib/vendor/autoload.php
```

**올바른 경로** (`../../../` — 3단계 상위):
```php
// export 요청일 때만 조건부 로드
if (isset($_GET['export']) && $_GET['export'] === 'true') {
    require_once __DIR__ . '/../../../lib/vendor/autoload.php';
    exportXxx($pdo);
    exit;
}
```

**잘못된 경로** (2단계 → `page/lib/...` 로 해석되어 파일 없음 오류):
```php
require_once __DIR__ . '/../../lib/vendor/autoload.php'; // ❌ 틀림
```

**`use` 선언 위치**: 파일 최상단(전역 스코프)에 작성, 함수 안에 넣으면 파싱 오류 발생.

```php
use PhpOffice\PhpSpreadsheet\Spreadsheet;
use PhpOffice\PhpSpreadsheet\Writer\Xlsx;
use PhpOffice\PhpSpreadsheet\Style\Fill;
use PhpOffice\PhpSpreadsheet\Style\Alignment;
use PhpOffice\PhpSpreadsheet\Style\Border;
```

> `use`는 compile-time alias이므로 autoload require 전에 선언해도 무방.
> export 분기가 아닌 일반 CRUD 요청에서는 autoload가 로드되지 않으므로 실제 클래스 사용 전에 로드 확인 필요.

---

## JS 변경 규칙

- Stats 관련 함수 **완전 제거**: `initStatisticsUpdater`, `updateStatistics`, `fetchAllStatistics`, `animateNumber`
- `initAdvancedFeatures`에서 `initStatisticsUpdater()` 호출 제거
- `initStatsToggle` 임포트/호출 제거
- 백엔드 apiEndpoint는 원본 proc 파일 그대로 참조

---

## PHP 공통 구조

```php
<?php
$page_title = '{페이지명}';
$page_css_files = [
  '../../assets/css/fiori-page.css',
  'css/{파일명}_2.css',
];
require_once(__DIR__ . '/../../inc/head.php');
/* nav-fiori.php 제거 */
?>

<?php $nav_active = '{키}'; require_once(__DIR__ . '/../../inc/nav-drawer-manage.php'); ?>

<!-- Signage Header -->
<div class="signage-header">
  <button id="navDrawerBtn" class="nav-drawer-btn">&#9776;</button>
  <span class="signage-header__title">{페이지명}</span>
  <div class="signage-header__filters">
    <!-- 검색, 필터, 버튼들 -->
  </div>
</div>

<!-- Main -->
<div class="manage-main">
  <div class="fiori-card">
    <div class="fiori-card__header">...</div>
    <div class="fiori-card__content fiori-p-0">
      <table class="fiori-table">...</table>
    </div>
  </div>
  <div id="pagination-controls" class="fiori-pagination"></div>
</div>

<!-- Modal -->
...

<!-- JS -->
<script type="module">
  import { createResourceManager } from '../../assets/js/resource-manager.js';
  import { initAdvancedFeatures, {페이지}Config } from './js/{파일명}_2.js';
  document.addEventListener('DOMContentLoaded', function() {
    const resourceManager = createResourceManager({페이지}Config);
    setTimeout(() => initAdvancedFeatures(resourceManager), 100);
  });
</script>
```

### $nav_active 키 매핑

| 파일                        | $nav_active 값   |
| --------------------------- | ---------------- |
| `info_factory_2.php`        | `factory`        |
| `info_line_2.php`           | `line`           |
| `info_machine_model_2.php`  | `machine_model`  |
| `info_machine_2.php`        | `machine`        |
| `info_design_process_2.php` | `design_process` |
| `info_andon_2.php`          | `andon`          |
| `info_downtime_2.php`       | `downtime`       |
| `info_defective_2.php`      | `defective`      |
| `info_rate_color_2.php`     | `rate_color`     |
| `info_worktime_2.php`       | `worktime`       |

---

## 진행 상황

| 페이지              | PHP                         | CSS                         | JS                         | proc        | 완료 | 확인 |
| ------------------- | --------------------------- | --------------------------- | -------------------------- | ----------- | ---- | ---- |
| info_factory        | `info_factory_2.php`        | `info_factory_2.css`        | `info_factory_2.js`        | 원본 재사용 | [x]  | [x]  |
| info_line           | `info_line_2.php`           | `info_line_2.css`           | `info_line_2.js`           | 원본 재사용 | [x]  | [x]  |
| info_machine_model  | `info_machine_model_2.php`  | `info_machine_model_2.css`  | `info_machine_model_2.js`  | 원본 재사용 | [x]  | [x]  |
| info_machine        | `info_machine_2.php`        | `info_machine_2.css`        | `info_machine_2.js`        | 원본 재사용 | [x]  | [x]  |
| info_design_process | `info_design_process_2.php` | `info_design_process_2.css` | `info_design_process_2.js` | 원본 재사용 | [x]  | [x]  |
| info_andon          | `info_andon_2.php`          | `info_andon_2.css`          | `info_andon_2.js`          | 원본 재사용 | [x]  | [x]  |
| info_downtime       | `info_downtime_2.php`       | `info_downtime_2.css`       | `info_downtime_2.js`       | 원본 재사용 | [x]  | [x]  |
| info_defective      | `info_defective_2.php`      | `info_defective_2.css`      | `info_defective_2.js`      | 원본 재사용 | [x]  | [x]  |
| info_rate_color     | `info_rate_color_2.php`     | `info_rate_color_2.css`     | `info_rate_color_2.js`     | 원본 재사용 | [x]  | [x]  |
| info_worktime       | `info_worktime_2.php`       | `info_worktime_2.css`       | 원본 인라인 유지           | 원본 재사용 | [x]  | [x]  |

> `info_worktime_2.php`는 `worktime_head.php` 유지 (별도 보존), signage-header + 햄버거 드로어만 추가. JS는 별도 파일 분리 없이 인라인 유지.

---

## 페이지별 특이사항

| 페이지              | 특이사항                                                                                                                                                                                                   |
| ------------------- | ---------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------- |
| info_factory        | 기준 페이지, 가장 단순한 구조                                                                                                                                                                              |
| info_line           | factory 필터 드롭다운 있음 (factory 연계)                                                                                                                                                                  |
| info_machine        | factory + line 연계 필터 2개 (`ajax_factory_line.php` 사용)                                                                                                                                                |
| info_machine_model  | 독립 구조 (연계 필터 없음)                                                                                                                                                                                 |
| info_design_process | machine 연계 + `process_machine_mapping.php` 사용                                                                                                                                                          |
| info_andon          | 독립 구조                                                                                                                                                                                                  |
| info_downtime       | 독립 구조                                                                                                                                                                                                  |
| info_defective      | 독립 구조                                                                                                                                                                                                  |
| info_rate_color     | 색상 picker UI 포함 가능성 있음                                                                                                                                                                            |
| info_worktime       | `worktime_head.php` 보존, signage-header + 드로어만 추가. `worktime_style.css`의 `button:not(:disabled){color:#fff}` 전역 규칙 때문에 버튼 셀렉터는 반드시 `button.nav-drawer-btn` 형태로 특이도 확보 필요 |

---

## 변경 이력

| 날짜       | 내용                                                                                                                                                                                                                                                                                                                                                     |
| ---------- | -------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------- |
| 2026-03-24 | 문서 최초 작성, 리디자인 원칙 및 공통 패턴 정의                                                                                                                                                                                                                                                                                                          |
| 2026-03-24 | info_factory_2.php / css / js 생성 완료, 사용자 확인 완료                                                                                                                                                                                                                                                                                                |
| 2026-03-24 | info_line_2.php / css / js 생성 완료, 사용자 확인 완료 (factory 필터 드롭다운 포함, stats 제거)                                                                                                                                                                                                                                                          |
| 2026-03-24 | CSS 버그 수정: `.signage-header .fiori-select { width:130px; flex-shrink:0 }` — select가 flex:1 환경에서 늘어나는 문제                                                                                                                                                                                                                                   |
| 2026-03-24 | info_machine_model_2.php / css / js 생성 완료 (stats 완전 제거, 독립 구조)                                                                                                                                                                                                                                                                               |
| 2026-03-24 | info_machine_2.php / css / js 생성 완료 (stats 완전 제거, factory+line+machine 연계 필터 유지)                                                                                                                                                                                                                                                           |
| 2026-03-24 | info_andon_2.php / css / js 생성 완료 (stats 완전 제거, spectrum 색상 피커 유지, 독립 구조)                                                                                                                                                                                                                                                              |
| 2026-03-24 | info_defective_2.php / css / js 생성 완료 (stats 완전 제거, shortcut 검증 유지, 독립 구조)                                                                                                                                                                                                                                                               |
| 2026-03-24 | info_downtime_2.php / css / js 생성 완료 (stats 완전 제거, shortcut 중복 검증 유지, 독립 구조)                                                                                                                                                                                                                                                           |
| 2026-03-24 | info_rate_color_2.php / css / js 생성 완료 (stats 없음, jQuery+ion.rangeSlider+spectrum.js 유지, colorPaletteModal 추가)                                                                                                                                                                                                                                 |
| 2026-03-24 | info_rate_color_2.css 수정: .range-slider-wrapper padding 28px/20px, overflow:visible — irs-from/irs-to 레이블 카드 이탈 수정, 사용자 확인 완료                                                                                                                                                                                                          |
| 2026-03-24 | info_design_process_2.php / css / js 생성 완료 (stats 완전 제거, factory+line 연계 필터 유지, SOP 파일업로드·이미지 미리보기 유지, Process-Machine 매핑 패널 is-open 토글 방식으로 재구성)                                                                                                                                                               |
| 2026-03-24 | info_design_process_2.css 수정: Set Process 패널 열릴 때 flex:1 전체 차지 + 테이블 카드 숨김 (panel-open 클래스), 컨테이너 max-height 제거 → height:100% — 드래그 공간 확보, 사용자 확인 완료                                                                                                                                                            |
| 2026-03-24 | info_worktime_2.php / css 생성 완료 (worktime_head.php 보존, nav-fiori.php 제거, signage-header + 햄버거 드로어 추가, JS 인라인 유지)                                                                                                                                                                                                                    |
| 2026-03-24 | info_worktime_2.css 수정: signage-header + drawer 색상을 다크 테마에서 Fiori 라이트 테마 실측값으로 변경 (surface-1=#fff, text-primary=#32363b, border=#d6dae3, accent=#0070f2)                                                                                                                                                                          |
| 2026-03-24 | info_worktime_2.css 수정: 햄버거 버튼 색상 버그 수정 — `worktime_style.css`의 `button:not(:disabled){color:#fff}` (특이도 0,1,1)이 `.nav-drawer-btn`(0,1,0)을 덮어쓰던 문제, `button.nav-drawer-btn` 셀렉터로 특이도 동급 후순위 override, 사용자 확인 완료                                                                                              |
| 2026-03-24 | 햄버거 드로어 공통 include 분리 — `inc/nav-drawer-manage.php` 생성, 전체 10개 `_2.php`에서 드로어 HTML + IIFE 스크립트 제거 후 `<?php $nav_active = '...'; require_once(...)` 1줄로 교체. `$nav_active` 변수로 활성 링크 자동 제어. `info_factory_2.php` 오타(`info_andon.php_2`) 수정, `info_worktime_2.php` 등 모니터링/리포트 링크 `_2` 버전으로 통일 |
| 2026-03-24 | 드로어 버그 수정 — `nav-drawer-manage.php` 스크립트를 `DOMContentLoaded`로 감쌈. include가 signage-header보다 먼저 실행되어 `navDrawerBtn`이 DOM에 없는 상태에서 `getElementById`가 null 반환하던 문제, 사용자 확인 완료                                                                                                                                 |
| 2026-03-24 | `machine.php` export 경로 버그 수정 — `proc` 기준 `../../lib/vendor/autoload.php`(2단계 → `page/lib/...`)를 `../../../lib/vendor/autoload.php`(3단계 → `OEE_SCI_V2/lib/...`)로 수정. 다른 proc 파일에 export 추가 시 동일 규칙 적용 필요                                                                                                                 |
