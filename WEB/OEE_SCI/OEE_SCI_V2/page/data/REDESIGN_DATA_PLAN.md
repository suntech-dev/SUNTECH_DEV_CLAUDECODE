# data 페이지 1920x1080 풀스크린 리디자인 계획

> 최초 작성: 2026-03-24
> 마지막 업데이트: 2026-03-30

---

## 새 컨텍스트 시작 프롬프트 (복붙용)

새 대화를 시작할 때 아래 템플릿을 그대로 붙여넣기 하면 된다.
`{대상 페이지}` 부분만 교체한다.

```
/sc:implement {대상 페이지} 리디자인

다음 파일들을 먼저 읽어줘:
1. C:\SUNTECH_DEV_CLAUDECODE\WEB\OEE_SCI\OEE_SCI_V2\page\data\REDESIGN_DATA_PLAN.md  ← 설계 문서
2. C:\SUNTECH_DEV_CLAUDECODE\WEB\OEE_SCI\OEE_SCI_V2\page\data\data_oee_2.php         ← 완성 모델 HTML
3. C:\SUNTECH_DEV_CLAUDECODE\WEB\OEE_SCI\OEE_SCI_V2\page\data\css\data_oee_2.css     ← 완성 모델 CSS
4. C:\SUNTECH_DEV_CLAUDECODE\WEB\OEE_SCI\OEE_SCI_V2\page\data\js\data_oee_2.js       ← 완성 모델 JS
5. C:\SUNTECH_DEV_CLAUDECODE\WEB\OEE_SCI\OEE_SCI_V2\page\data\{원본 페이지}.php      ← 이식할 원본

읽은 후 REDESIGN_DATA_PLAN.md 의 설계 원칙 + data_oee_2 파일 구조를 그대로 따라서
{대상 페이지}_2.php, css/{대상 페이지}_2.css, js/{대상 페이지}_2.js 를 새로 만들어줘.
구현 후 Playwright로 레이아웃 검증까지 해줘.
```

### 예시 (data_andon)

```
/sc:implement data_andon 리디자인

다음 파일들을 먼저 읽어줘:
1. C:\SUNTECH_DEV_CLAUDECODE\WEB\OEE_SCI\OEE_SCI_V2\page\data\REDESIGN_DATA_PLAN.md
2. C:\SUNTECH_DEV_CLAUDECODE\WEB\OEE_SCI\OEE_SCI_V2\page\data\data_oee_2.php
3. C:\SUNTECH_DEV_CLAUDECODE\WEB\OEE_SCI\OEE_SCI_V2\page\data\css\data_oee_2.css
4. C:\SUNTECH_DEV_CLAUDECODE\WEB\OEE_SCI\OEE_SCI_V2\page\data\js\data_oee_2.js
5. C:\SUNTECH_DEV_CLAUDECODE\WEB\OEE_SCI\OEE_SCI_V2\page\data\data_andon.php

읽은 후 REDESIGN_DATA_PLAN.md 의 설계 원칙 + data_oee_2 파일 구조를 그대로 따라서
data_andon_2.php, css/data_andon_2.css, js/data_andon_2.js 를 새로 만들어줘.
구현 후 Playwright로 레이아웃 검증까지 해줘.
```

---

## 핵심 규칙

1. **Footer 제거**
2. **파일명 규칙** — 변경 파일에만 `_2` 접미사 부여, 백엔드 proc 파일은 변경 없으면 원본 재사용
3. **들여쓰기** — 4칸 스페이스
 
## 레이아웃 모델: ai_dashboard_5.php Row Grid 방식

`ai_dashboard_5.php` + `css/ai_dashboard_5.css` 를 **기준 모델**로 삼는다.

### ai_dashboard_5의 핵심 패턴

```css
/* 메인 컨테이너: CSS Grid + fr 단위 */
.ai-signage-main {
  height: calc(100vh - 52px);
  display: grid;
  grid-template-rows: 15fr 29fr 28fr 28fr;  /* 고정 비율 행 */
  gap: 5px;
  padding: 5px;
  overflow: hidden;
}

/* 각 행: display:grid + min-height:0 (필수) */
.ai-signage-row-b {
  display: grid;
  grid-template-columns: 2fr 1fr;
  gap: 5px;
  min-height: 0;
}

/* 카드: height:100% + overflow:hidden + flex column */
.ai-signage-main .fiori-card {
  height: 100%;
  display: flex;
  flex-direction: column;
  overflow: hidden;
  margin-bottom: 0;
}

/* 카드 컨텐츠: flex:1 + min-height:0 + overflow:hidden (핵심) */
.ai-signage-main .fiori-card__content {
  flex: 1;
  min-height: 0;
  display: flex;
  flex-direction: column;
  overflow: hidden;
}
```

### data_oee_2.php 와의 차이점

| 항목               | ai_dashboard_5       | data_oee_2                                                   |
| ------------------ | -------------------- | ------------------------------------------------------------ |
| 행 가시성          | 항상 표시 (4행 고정) | 토글 가능 (stats/charts/table)                               |
| grid-template-rows | 고정 fr 비율         | JS가 동적으로 계산·설정                                      |
| 행 수              | 4                    | 최대 5 (stats, charts-top, charts-bottom, table, pagination) |

---

## data_oee_2 레이아웃 설계

### HTML 구조 (행 기반)

```
.oee-signage-main  (CSS Grid, id="oeeSignageMain")
  ├── #oeeRowStats        .oee-row  (Row A: Stats 6카드 — 기본 hidden)
  ├── #oeeRowChartsTop    .oee-row  (Row B: Components Details + 2차트 — 기본 hidden)
  ├── #oeeRowChartsBottom .oee-row  (Row C: 3 Trend 차트 — 기본 hidden)
  ├── #oeeRowTable        .oee-row  (Row D: Real-time 테이블 — 기본 표시)
  └── #oeeRowPagination   .oee-row  (Row E: 페이지네이션 — auto)
```

### 각 행 내부 구조

| 행            | 내부 레이아웃                                                          | 컬럼              |
| ------------- | ---------------------------------------------------------------------- | ----------------- |
| Stats         | `.oee-stats-grid` (grid)                                               | 6×1fr             |
| Charts-Top    | `.oee-charts-top-grid` (grid) → 좌:Details 우:`.oee-charts-pair`(grid) | 2fr+3fr / 1fr+1fr |
| Charts-Bottom | `.oee-charts-trio` (grid)                                              | 1fr+1fr+1fr       |
| Table         | `.fiori-card`                                                          | flex column       |
| Pagination    | `#pagination-controls`                                                 | —                 |

### 토글 레이아웃 동적 계산 (updateLayout)

```javascript
function updateLayout() {
  const main = document.getElementById('oeeSignageMain');
  const rows = [];

  if (!document.getElementById('oeeRowStats').classList.contains('hidden'))        rows.push('auto');
  if (!document.getElementById('oeeRowChartsTop').classList.contains('hidden'))    rows.push('1fr');
  if (!document.getElementById('oeeRowChartsBottom').classList.contains('hidden')) rows.push('1fr');
  if (!document.getElementById('oeeRowTable').classList.contains('hidden'))        rows.push('1fr');
  rows.push('auto'); // 페이지네이션 항상

  main.style.gridTemplateRows = rows.join(' ');

  // Chart.js ResizeObserver 트리거
  setTimeout(() => Object.values(charts).forEach(c => c && c.resize()), 50);
}
```

**결과 (각 상태별 grid-template-rows):**

| 상태                         | gridTemplateRows        |
| ---------------------------- | ----------------------- |
| 테이블만 (기본)              | `1fr auto`              |
| Stats + 테이블               | `auto 1fr auto`         |
| Charts + 테이블              | `1fr 1fr 1fr auto`      |
| Stats + Charts + 테이블      | `auto 1fr 1fr 1fr auto` |
| Charts만 (테이블 숨김)       | `1fr 1fr auto`          |
| Stats + Charts (테이블 숨김) | `auto 1fr 1fr auto`     |

---

## CSS 패턴 (ai_dashboard_5 그대로 적용)

```css
/* 메인 컨테이너 */
.oee-signage-main {
  height: calc(100vh - 52px);
  display: grid;
  grid-template-rows: 1fr auto; /* JS가 덮어씀 */
  gap: 5px;
  padding: 5px;
  box-sizing: border-box;
  overflow: hidden;
}

/* 행 기본 */
.oee-row { min-height: 0; overflow: hidden; }
.oee-row.hidden { display: none; }

/* 카드 */
.oee-signage-main .fiori-card {
  height: 100%;
  display: flex;
  flex-direction: column;
  overflow: hidden;
  margin-bottom: 0;
}

/* 카드 헤더 */
.oee-signage-main .fiori-card__header {
  flex-shrink: 0;
  padding: 5px 10px;
}

/* 카드 컨텐츠 */
.oee-signage-main .fiori-card__content {
  flex: 1;
  min-height: 0;
  display: flex;
  flex-direction: column;
  overflow: hidden;
  padding: 6px 8px;
}

/* 차트 컨테이너 */
.oee-signage-main .chart-container {
  flex: 1;
  min-height: 0;
  overflow: hidden;
  position: relative;
}
```

**핵심 원칙:**
- `overflow: hidden` 이 모든 레벨(행, 카드, 컨텐츠, 차트컨테이너)에 적용
- Canvas가 컨테이너를 부풀리지 못함 → Chart.js ResizeObserver 정상 동작
- JavaScript 리사이즈 핵 불필요

---

## 차트 목록 (data_oee_2)

| Canvas ID              | 종류     | 위치          |
| ---------------------- | -------- | ------------- |
| `oeeComponentChart`    | Radar    | Row B 우측 좌 |
| `oeeGradeChart`        | Doughnut | Row B 우측 우 |
| `oeeTrendChart`        | Line     | Row C 좌      |
| `productionTrendChart` | Bar      | Row C 중      |
| `machineOeeChart`      | Bar      | Row C 우      |

---

## 파일 구성

| 파일                   | 역할                           |
| ---------------------- | ------------------------------ |
| `data_oee_2.php`       | HTML 구조 (Row Grid 방식)      |
| `css/data_oee_2.css`   | ai_dashboard_5 패턴 CSS        |
| `js/data_oee_2.js`     | 데이터/SSE + updateLayout 토글 |
| `js/ai_oee_overlay_2.js` | AI 예측 오버레이 (재사용)      |

**제거:** `data_oee.js` (구 버전, data_oee_2.js로 대체)

---

## 테이블 컬럼 고정 (Sticky Column) 구현 패턴

> `_2` 레이아웃은 모든 레벨에 `overflow: hidden`이 적용되어 있어 CSS `position: sticky`가 Chromium에서 작동하지 않음.
> **JS scroll 리스너 + `transform: translateX`** 방식으로 대체 구현.

### 원인 분석

```
.oee-signage-main           overflow: hidden  ← grid 컨테이너
  └── .oee-row              overflow: hidden  ← 행
        └── .fiori-card     overflow: hidden  ← 카드
              └── .fiori-card__content  overflow: hidden  ← 카드 컨텐츠
                    └── .oee-table-wrap  overflow-x: auto  ← 실제 스크롤 컨테이너
```

Chromium에서 `overflow: hidden` 조상이 있으면 `position: sticky` scroll container 감지 실패 → 고정 안됨.
`overflow: clip` 으로 변경해도 이 케이스에서는 동일하게 미작동 확인.

### CSS 설정

```css
/* overflow: clip + min-width: 0 으로 너비 제약 유지 */
.log-row-main { width: 100%; overflow: clip; }
.log-row      { min-width: 0; overflow: clip; }
.log-row-main .fiori-card         { min-width: 0; overflow: clip; }
.log-row-main .fiori-card__content { min-width: 0; overflow: clip; }

/* border-collapse: separate 필수 (fiori-components.css가 collapse 전역 설정) */
.oee-table-wrap > .fiori-table {
    border-collapse: separate !important;
    border-spacing: 0 !important;
}

/* sticky-column: position:relative + will-change (JS가 transform 제어) */
.oee-table-wrap th.sticky-column,
.oee-table-wrap td.sticky-column {
    position: relative;
    z-index: 10;
    background-color: var(--sap-surface-1) !important;
    box-shadow: 2px 0 4px rgba(0, 0, 0, .1);
    border-right: 2px solid var(--sap-border-neutral);
    white-space: nowrap;
    will-change: transform;
}
.oee-table-wrap thead th.sticky-column { z-index: 11; background-color: var(--sap-surface-2) !important; }
```

### JS 구현 (`initStickyColumnsScroll`)

```javascript
function initStickyColumnsScroll() {
    const wrap = document.querySelector('.oee-table-wrap');
    if (!wrap) return;
    let _stickyEls = [];
    let _naturalOffsets = [];

    function _captureStickyOffsets() {
        const prevScroll = wrap.scrollLeft;
        if (prevScroll !== 0) wrap.scrollLeft = 0;      // scroll=0 상태에서 offsetLeft 측정
        _stickyEls = Array.from(wrap.querySelectorAll('.sticky-column'));
        _naturalOffsets = _stickyEls.map(el => el.offsetLeft);
        if (prevScroll !== 0) wrap.scrollLeft = prevScroll;
        _updatePositions();
    }

    function _updatePositions() {
        const sl = wrap.scrollLeft;
        _stickyEls.forEach((el, i) => {
            const nat = _naturalOffsets[i] || 0;
            const shift = sl > nat ? sl - nat : 0;
            el.style.transform = shift > 0 ? `translateX(${shift}px)` : '';
        });
    }

    let _ticking = false;
    wrap.addEventListener('scroll', () => {
        if (!_ticking) {
            requestAnimationFrame(() => { _updatePositions(); _ticking = false; });
            _ticking = true;
        }
    }, { passive: true });

    wrap._refreshStickyColumns = _captureStickyOffsets;
    setTimeout(_captureStickyOffsets, 300);  // 초기 렌더링 후 offset 캡처
}
```

**동작 원리:** `셀 뷰포트 위치 = wrapLeft + naturalOffset - scrollLeft + translateX`
→ `translateX = scrollLeft - naturalOffset` 적용 시 항상 `wrapLeft` 에 고정.

### 필수 호출 위치

| 위치 | 이유 |
|------|------|
| `DOMContentLoaded` 마지막 | 초기 offset 캡처 |
| `updateTableFromAPI` 마지막 | 페이지 전환 후 행이 바뀌면 offset 재측정 필요 |

```javascript
// DOMContentLoaded
initStickyColumnsScroll();

// updateTableFromAPI 마지막 줄
document.querySelector('.oee-table-wrap')?._refreshStickyColumns?.();
```

### Playwright 검증 결과 (2026-03-25)

```json
{
  "ok": true,
  "scrollLeft": 400,
  "wrapLeft": 6,
  "tdLeft": 6,
  "diff": 0,
  "tdTransform": "translateX(399px)"
}
```
3개 페이지 모두 (`log_oee_row_2`, `log_oee_hourly_2`, `log_oee_2`) STICKY 정상 작동 확인.

---

## 진행 상황

| 페이지         | PHP | CSS | JS  | 완료 |
| -------------- | --- | --- | --- | ---- |
| data_oee (v2)  | [x] | [x] | [x] | [x]  |
| data_andon     | [x] | [x] | [x] | [x]  |
| data_downtime  | [x] | [x] | [x] | [x]  |
| data_defective | [x] | [x] | [x] | [x]  |
| log_oee        | [x] | [x] | [x] | [x]  |
| log_oee_hourly | [x] | [x] | [x] | [x]  |
| log_oee_row    | [x] | [x] | [x] | [x]  |
| dashboard      | [x] | [x] | [x] | [x]  |

---

## 변경 이력

| 날짜       | 내용                                                                                                                                           |
| ---------- | ---------------------------------------------------------------------------------------------------------------------------------------------- |
| 2026-03-24 | 문서 최초 작성 (manage와 분리)                                                                                                                 |
| 2026-03-24 | nav-drawer 공용화 방안 확정, data_oee_2.php 초기 구현                                                                                          |
| 2026-03-25 | 레이아웃 모델을 ai_dashboard_5.php Row Grid 방식으로 전면 재설계. updateLayout() 동적 gridTemplateRows 방식으로 전환. data_oee_2.js 신규 작성. |
| 2026-03-25 | Playwright 검증 완료 — 6개 토글 상태 모두 정상. 어떤 상태도 뷰포트(1080px) 초과 없음. 차트 5개 정상 렌더링 확인. |
| 2026-03-25 | data_downtime_2 구현 완료. data_oee_2 Row Grid 구조 그대로 적용. 차트 5개(dtTypeChart, dtStatusChart, dtTrendChart, dtLineChart, dtDurationChart). Playwright 6단계 검증 PASS. |
| 2026-03-25 | data_defective_2 구현 완료. data_oee_2 Row Grid 구조 그대로 적용. 차트 5개(defectiveTypeChart, defectiveStatusChart, defectiveTrendChart, defectiveMachineChart, defectiveLineChart). Active Defectives 실시간 리스트 + 경과시간 타이머 포함. Playwright 레이아웃 검증 PASS (1080px 초과 없음). |
| 2026-03-25 | log_oee_row_2 구현 완료. data_oee_2 Row Grid 구조 적용. 차트 없음 (Stats Row + Table Row). 컬럼 토글(33개 컬럼, hidden-column/sticky-column), SSE proc/log_oee_row_stream.php 재사용. Playwright 6단계 검증 PASS (1080px 초과 없음). |
| 2026-03-25 | log_oee_hourly_2 구현 완료. log_oee_row_2 구조 그대로 적용. 네임스페이스 logRow→logHourly로 변경. 컬럼 34개(update_date 추가). SSE proc/log_oee_hourly_stream.php 재사용. Playwright 6단계 검증 PASS (모든 토글 상태 1080px 초과 없음, Stats+Table 동시표시 정상). |
| 2026-03-25 | log_oee_2 구현 완료. log_oee_row_2 구조 그대로 적용. 네임스페이스 logRow→logOee로 변경. 컬럼 33개(work_hour 제거, update_date 추가, idx visible:false). SSE proc/log_oee_stream.php 재사용. Playwright 6단계 검증 PASS (모든 토글 상태 1080px 초과 없음). |
| 2026-03-25 | 테이블 컬럼 고정(sticky column) 구현 완료. CSS `position: sticky` 미작동(overflow:hidden 부모 체인 원인) → JS scroll 리스너 + `transform: translateX` 방식으로 해결. log_oee_row_2, log_oee_hourly_2, log_oee_2 3개 페이지 모두 적용. Playwright 검증: scrollLeft=400 에서 tdLeft=wrapLeft(diff=0) 확인. |
| 2026-03-30 | dashboard_2 (사이니지 전용 대시보드) 기능 점검 및 버그 3건 수정. `proc/dashboard_stream_2.php` 수정. Playwright로 2026-03-06 데이터 검증 완료. ▶ 상세는 PERFORMANCE_OPTIMIZATION.md `5-4` 참조. |
