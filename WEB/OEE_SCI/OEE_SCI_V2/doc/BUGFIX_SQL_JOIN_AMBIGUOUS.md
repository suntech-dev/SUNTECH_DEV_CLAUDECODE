# 버그픽스: SQL JOIN Ambiguous Column 오류

**파일:** `page/data/proc/dashboard_stream_2.php`  
**발생일:** 2026-04-14  
**심각도:** 🔴 Critical — OEE / Production 데이터 전체 0% 표시

---

## 증상

- 대시보드 Last Updated 시간은 정상 갱신되지만 OEE 게이지·차트 데이터가 모두 0%
- 필터를 변경해도 데이터가 바뀌지 않음 (Full Page Refresh 시에는 정상)
- SSE 스트림 응답에 다음 오류 포함:

```json
"oee": {
  "error": "SQLSTATE[23000]: Integrity constraint violation: 1052 Column 'factory_idx' in where clause is ambiguous"
},
"production": {
  "error": "SQLSTATE[23000]: Integrity constraint violation: 1052 Column 'factory_idx' in where clause is ambiguous"
}
```

---

## 원인 분석

### 트리거: factory 필터 자동 선택 기능 추가 (2026-04-13)

`dashboard_2.js`에서 factory_idx=99(Inventory) 제외 후 단일 factory 자동 선택 기능을 추가함에 따라, 이전까지 항상 빈값(`factory_filter=`)이었던 필터에 실제 값(`factory_filter=104`)이 처음으로 전달되기 시작했다.

### SQL WHERE 절의 컬럼 Ambiguous 문제

`getOEEData()` 및 `getProductionData()` 함수의 쿼리 구조:

```sql
SELECT ...
FROM data_oee do
LEFT JOIN info_machine m ON do.machine_idx = m.idx
WHERE factory_idx = ?    -- ← 두 테이블 모두 factory_idx 컬럼 보유 → Ambiguous!
  AND work_date = ?
  AND m.status = 'Y'
```

`data_oee`와 `info_machine` **양쪽 모두 `factory_idx` 컬럼을 보유**하므로, `factory_filter` 값이 있을 때만 WHERE 절에 `factory_idx = ?`가 추가되어 MySQL이 어느 테이블의 컬럼인지 판별 불가 오류 발생.

> 이전에는 `factory_filter`가 항상 빈값이어서 WHERE 절에 `factory_idx` 조건이 생성되지 않았고, 따라서 오류가 드러나지 않았다.

### 영향받은 함수 및 위치

| 함수 | 쿼리 테이블 | 문제 코드 위치 |
|---|---|---|
| `sendDashboardData()` | — | line 1214, 1218 |
| `getOEEData()` | `data_oee do` + `info_machine m` | `parseDashboardFilterParams()` 호출 |
| `getPreviousOEEData()` | `data_oee do` + `info_machine m` | line 90~106, 127, 132 |
| `getProductionData()` (hourly) | `data_oee_rows_hourly doh` + `info_machine m` | `parseDashboardFilterParams()` 호출 |
| `getProductionData()` (timeline) | `data_oee_rows_hourly doh` + `info_machine m` | line 973, 978, 986~999 |

---

## 수정 내용

### 1. `sendDashboardData()` — parseDashboardFilterParams alias 추가

```php
// 수정 전
$oeeFilter        = parseDashboardFilterParams();   // alias 없음 → WHERE factory_idx = ?
$productionFilter = parseDashboardFilterParams();   // alias 없음 → WHERE factory_idx = ?

// 수정 후
$oeeFilter        = parseDashboardFilterParams('do');  // WHERE do.factory_idx = ?
$productionFilter = parseDashboardFilterParams('doh'); // WHERE doh.factory_idx = ?
```

### 2. `getPreviousOEEData()` — 직접 빌드 WHERE 절에 `do.` prefix 추가

```php
// 수정 전
$where_clauses[] = 'factory_idx = ?';
$where_clauses[] = 'line_idx = ?';
$where_clauses[] = 'machine_idx = ?';
$where_clauses[] = 'shift_idx = ?';
$where_clauses[] = 'work_date BETWEEN ? AND ?';
$where_clauses[] = 'work_date = ?';

// 수정 후
$where_clauses[] = 'do.factory_idx = ?';
$where_clauses[] = 'do.line_idx = ?';
$where_clauses[] = 'do.machine_idx = ?';
$where_clauses[] = 'do.shift_idx = ?';
$where_clauses[] = 'do.work_date BETWEEN ? AND ?';
$where_clauses[] = 'do.work_date = ?';
```

### 3. `getProductionData()` timeline 섹션 — `doh.` prefix 추가

```php
// 수정 전 (날짜 필터)
$timeline_where_clauses[] = 'work_date = ?';
$timeline_where_clauses[] = 'work_date BETWEEN ? AND ?';

// 수정 전 (필터 조건)
$timeline_where_clauses[] = 'factory_idx = ?';
$timeline_where_clauses[] = 'line_idx = ?';
$timeline_where_clauses[] = 'machine_idx = ?';
$timeline_where_clauses[] = 'shift_idx = ?';

// 수정 후
$timeline_where_clauses[] = 'doh.work_date = ?';
$timeline_where_clauses[] = 'doh.work_date BETWEEN ? AND ?';
$timeline_where_clauses[] = 'doh.factory_idx = ?';
$timeline_where_clauses[] = 'doh.line_idx = ?';
$timeline_where_clauses[] = 'doh.machine_idx = ?';
$timeline_where_clauses[] = 'doh.shift_idx = ?';
```

---

## 핵심 원칙 (재발 방지)

> **`info_machine` 테이블은 `factory_idx`, `line_idx` 컬럼을 보유하고 있다.**  
> `data_oee`, `data_oee_rows_hourly` 등을 `info_machine m`과 JOIN할 때는  
> **반드시 WHERE 절의 모든 컬럼에 테이블 alias prefix를 붙여야 한다.**

### alias 규칙 정리

| 테이블 | alias | WHERE prefix |
|---|---|---|
| `data_oee` | `do` | `do.factory_idx`, `do.work_date`, ... |
| `data_oee_rows_hourly` | `doh` | `doh.factory_idx`, `doh.work_date`, ... |
| `data_andon` | `da` | `da.factory_idx`, ... |
| `data_downtime` | `dd` | `dd.factory_idx`, ... |
| `data_defective` | `dd` | `dd.factory_idx`, ... |
| `info_machine` | `m` | (JOIN 대상, WHERE 필터 대상 아님) |

### `parseDashboardFilterParams()` 호출 시 반드시 alias 전달

```php
// ✅ 올바른 사용
$oeeFilter        = parseDashboardFilterParams('do');
$productionFilter = parseDashboardFilterParams('doh');
$andonFilter      = parseDashboardFilterParams('da');
$downtimeFilter   = parseDashboardFilterParams('dd');
$defectiveFilter  = parseDashboardFilterParams('dd');

// ❌ 잘못된 사용 — JOIN 쿼리에서 ambiguous 오류 발생
$filter = parseDashboardFilterParams();
```

---

## 디버깅 방법

필터 값이 전달될 때만 오류가 발생하므로, curl로 실제 스트림 응답을 확인하는 것이 가장 빠르다:

```bash
curl -s --max-time 6 \
  "http://[서버IP]/OEE_SCI/OEE_SCI_V2/page/data/proc/dashboard_stream_2.php\
?factory_filter=104&shift_filter=1&start_date=2026-04-13&end_date=2026-04-13" \
  | head -c 500
```

응답에 `"error":"SQLSTATE[23000]..."` 가 있으면 JOIN 쿼리의 컬럼 prefix 누락을 의심.

---

## 전체 파일 SQL JOIN 안전성 검사 결과 (2026-04-14)

| 파일 | JOIN 여부 | 결과 |
|---|---|---|
| `dashboard_stream_2.php` | `info_machine m` JOIN | ✅ 수정 완료 |
| `data_oee_stream_2.php` | 있음 | ✅ prefix 정상 |
| `data_andon_stream_2.php` | 있음 | ✅ prefix 정상 |
| `data_downtime_stream_2.php` | 있음 | ✅ prefix 정상 |
| `data_defective_stream_2.php` | JOIN 없는 단독 쿼리 | ✅ 안전 |
| AI proc 파일들 | JOIN 없음 | ✅ 안전 |
| export / log 파일들 | 없거나 안전 | ✅ 안전 |
