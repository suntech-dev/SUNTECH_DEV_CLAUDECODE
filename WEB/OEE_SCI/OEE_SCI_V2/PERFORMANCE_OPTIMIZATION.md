> 최초 작성: 2026-03-07
> 분석 버전: OEE_SCI_V2
> 마지막 업데이트: 2026-03-07 (Phase 4 완료: 타임존 중앙화 + report_stream 버그 수정)

# OEE_SCI_V2 성능 최적화 계획

---

## 1. 현황 분석 요약

### 로딩이 느린 근본 원인

| 원인 | 해당 파일 | 심각도 |
|------|-----------|--------|
| SSE 사이클당 다중 순차 쿼리 (최대 7회) | data_oee_stream.php | 높음 |
| 동일 데이터 중복 집계 쿼리 | data_oee_stream.php, log_oee_stream.php | 높음 |
| N+1 쿼리 패턴 (type stats 2회 분리 조회) | data_downtime_stream.php, data_andon_stream.php | 중간 |
| log_oee 해시 버그 (항상 전송) | log_oee_stream.php | 높음 |
| 기본 LIMIT 1000 (log_oee 전체 전송) | log_oee_stream.php | 중간 |
| 공통 함수 중복 정의 (파일마다 복사) | 모든 stream 파일 | 중간 |
| DB 인덱스 미확인 | 모든 테이블 | 높음 |

---

## 2. 파일별 문제 상세 분석

### 2-1. `data_oee_stream.php` — SSE 사이클당 7회 쿼리

```
[현재: 5초마다 실행되는 쿼리 목록]
1. getOeeData()           → SELECT ... FROM data_oee (LIMIT 100)
2. getOeeStats()          → SELECT AVG/SUM ... FROM data_oee (전체 집계)
3. getOeeDetails()        → SELECT AVG/SUM ... FROM data_oee (2번과 거의 동일)
4. getOeeTrendStats()     → SELECT ... GROUP BY work_date/work_hour
5. getOeeComponentStats() → SELECT AVG/SUM ... FROM data_oee (2번의 부분집합)
6. getProductionTrendStats() → 4번과 동일한 날짜 판정 로직 + workHours 중복 호출
7. getMachineOeeStats()   → SELECT ... GROUP BY line/machine
```

**문제점:**
- `getOeeStats` + `getOeeDetails` + `getOeeComponentStats` = 동일 테이블, 동일 WHERE, 다른 집계 → **1개 쿼리로 통합 가능**
- `getOeeTrendStats` + `getProductionTrendStats` = 날짜 판정 로직 동일, `getWorkHoursForDate()` 2번 호출 → **1개 쿼리로 통합 가능**
- 최적화 후: 7회 → **4회로 축소**

---

### 2-2. `log_oee_stream.php` — 해시 버그 + LIMIT 1000

```php
// 현재 코드 (버그): count > 0 이면 항상 전송
if ($currentDataHash !== $lastDataHash || count($oeeDataLog) > 0) {
  sendSSEData('oee_data', $responseData); // 매 5초마다 1000건 전송!
}
```

**문제점:**
- `|| count($oeeDataLog) > 0` 조건 때문에 데이터가 있으면 **변화 없어도 항상** 전송
- LIMIT 1000 = 기본 조회량이 data_oee_stream의 10배
- 매 5초마다 1000건 JSON 직렬화 + 전송 → 네트워크/CPU 낭비

**수정:**
```php
// 수정 후: 해시 변화 시에만 전송
if ($currentDataHash !== $lastDataHash) {
  sendSSEData('oee_data', $responseData);
}
```

---

### 2-3. `data_downtime_stream.php` / `data_andon_stream.php` — N+1 쿼리

```php
// 현재: getDowntimeTypeStats() 내부에서 2번 쿼리
$allDowntimesQuery = "SELECT downtime_name FROM info_downtime WHERE status = 'Y'";
// ... 실행 ...
$dataQuery = "SELECT dd.downtime_name, COUNT(*) ... FROM data_downtime dd ... GROUP BY ...";
// ... 실행 ...
// PHP에서 left join 역할 수동으로 수행
```

**문제점:**
- 2개의 별도 쿼리 + PHP 루프 = DB에서 LEFT JOIN 한 번으로 해결 가능

**최적화:**
```sql
-- 1개 쿼리로 통합
SELECT
  id.downtime_name,
  COALESCE(COUNT(dd.idx), 0) as count,
  COALESCE(SUM(CASE WHEN dd.status = 'Warning' THEN 1 ELSE 0 END), 0) as warning_count,
  COALESCE(SUM(CASE WHEN dd.status = 'Completed' THEN 1 ELSE 0 END), 0) as completed_count,
  COALESCE(SUM(dd.duration_sec), 0) as total_duration_sec
FROM info_downtime id
LEFT JOIN data_downtime dd
  ON id.downtime_name = dd.downtime_name
  AND {filter_conditions}
WHERE id.status = 'Y'
GROUP BY id.downtime_name
ORDER BY id.downtime_name ASC
```

---

### 2-4. 공통 함수 중복 정의

아래 함수들이 모든 stream 파일마다 **동일 코드로 복사** 존재:

| 함수 | 복사된 파일 수 |
|------|--------------|
| `getWorkHoursForDate()` | 3개 (oee, downtime, andon) |
| `parseFilterParams()` | 5개 (별칭만 다름: `do.`, `dd.`, `da.`) |
| `sendSSEData()` | 5개 |
| shift 시간 계산 로직 | 3개 |

**해결책:** `lib/stream_helper.lib.php` 공통 라이브러리 생성

---

### 2-5. DB 인덱스 확인 필요 (예상 누락)

성능에 가장 큰 영향을 미치는 쿼리 조건:

```sql
-- data_oee: 주요 WHERE 조건
work_date, factory_idx, line_idx, machine_idx, shift_idx

-- data_downtime / data_andon / data_defective: 주요 WHERE 조건
reg_date, factory_idx, line_idx, machine_idx, shift_idx, status
```

**확인 명령:**
```sql
SHOW INDEX FROM data_oee;
SHOW INDEX FROM data_downtime;
SHOW INDEX FROM data_andon;
SHOW INDEX FROM data_defective;
```

**권장 복합 인덱스:**
```sql
-- data_oee
ALTER TABLE data_oee ADD INDEX idx_oee_main (work_date, factory_idx, line_idx, machine_idx);
ALTER TABLE data_oee ADD INDEX idx_oee_machine_status (machine_idx);

-- data_downtime
ALTER TABLE data_downtime ADD INDEX idx_dt_main (reg_date, factory_idx, line_idx, machine_idx);
ALTER TABLE data_downtime ADD INDEX idx_dt_status (status, reg_date);

-- data_andon
ALTER TABLE data_andon ADD INDEX idx_andon_main (reg_date, factory_idx, line_idx, machine_idx);
ALTER TABLE data_andon ADD INDEX idx_andon_status (status, reg_date);
```

---

## 3. 최적화 우선순위 & 실행 계획

### Phase 1 — 즉시 효과 (버그 수정 + 고영향)
> 목표: 체감 로딩 속도 개선

| 순서 | 파일 | 작업 내용 | 예상 효과 |
|------|------|-----------|-----------|
| 1-A | `log_oee_stream.php` | 해시 버그 수정 + LIMIT 조정 | 매 5초 1000건 전송 제거 |
| 1-B | `data_oee_stream.php` | stats/details/component 쿼리 통합 (7→4개) | SSE 응답시간 40% 단축 예상 |
| 1-C | DB | 인덱스 확인 및 추가 | 쿼리 실행계획 개선 |

### Phase 2 — 코드 품질 + 중간 영향
> 목표: 일관성 및 유지보수성

| 순서 | 파일 | 작업 내용 | 예상 효과 |
|------|------|-----------|-----------|
| 2-A | `data_downtime_stream.php` | Type stats N+1 → 단일 LEFT JOIN | 쿼리 1개 제거 |
| 2-B | `data_andon_stream.php` | Type stats N+1 → 단일 LEFT JOIN | 쿼리 1개 제거 |
| 2-C | `data_defective_stream.php` | 동일 패턴 적용 | 일관성 |

### Phase 3 — 공통화 (기술 부채 해소)
> 목표: 코드 중복 제거

| 순서 | 파일 | 작업 내용 |
|------|------|-----------|
| 3-A | `lib/stream_helper.lib.php` (신규) | getWorkHoursForDate, sendSSEData, parseShiftTime 공통화 |
| 3-B | 모든 stream 파일 | stream_helper.lib.php require_once 로 대체 |

---

## 4. 최적화 패턴 (첫 번째 파일 기준)

**첫 번째 최적화 대상: `log_oee_stream.php`** (Phase 1-A)

이유:
- 코드가 상대적으로 단순 (쿼리 2개: 데이터 + 통계)
- 버그(해시 조건)가 명확하여 수정이 단순
- 효과가 즉각적이고 측정 가능

이 파일에서 확립할 최적화 패턴:
1. **해시 조건 패턴**: `$currentDataHash !== $lastDataHash` (변화 시에만 전송)
2. **성능 로깅 패턴**: `microtime(true)` 으로 각 쿼리 시간 측정
3. **SSE 헬퍼 함수 패턴**: `sendSSEData()` 표준화
4. **에러 처리 패턴**: 각 쿼리 독립 try-catch (한 쿼리 실패해도 나머지 전송)

이 패턴을 이후 파일들에 일관되게 적용.

---

## 5. 진행 상태 추적

| Phase | 파일 | 상태 | 완료일 |
|-------|------|------|--------|
| 1-A | log_oee_stream.php | 완료 | 2026-03-07 |
| 1-B | data_oee_stream.php | 완료 | 2026-03-07 |
| 1-C | DB 인덱스 확인/추가 | 대기 | - |
| 2-A | data_downtime_stream.php | 완료 | 2026-03-07 |
| 2-B | data_andon_stream.php | 완료 | 2026-03-07 |
| 2-C | data_defective_stream.php | 완료 | 2026-03-07 |
| 3-A | lib/stream_helper.lib.php 신규 | 완료 | 2026-03-07 |
| 3-B | 모든 stream 파일 공통화 (9개 파일) | 완료 | 2026-03-07 |
| 3-C | log_oee_hourly/row 해시 버그 수정 | 완료 | 2026-03-07 |
| 4-A | date_default_timezone_set 중앙화 (27→1곳, lib/db.php) | 완료 | 2026-03-07 |
| 4-B | lib/worktime_database.php → lib/db.php 대체 후 삭제 | 완료 | 2026-03-07 |
| 4-C | report_stream.php getDBConnection() 미선언 버그 수정 | 완료 | 2026-03-07 |

---

## 5-1. Phase 1-B 완료 기록 (2026-03-07)

### 작업 내용
- `getOeeStats` + `getOeeDetails` + `getOeeComponentStats` → `getOeeAggregated()` 1개 쿼리로 통합
- `getOeeTrendStats` + `getProductionTrendStats` → `getTrendStats()` 1개 쿼리 + `getWorkHoursForDate()` 1회 호출로 통합
- 쿼리 수: **7개 → 4개** (SSE 사이클당)

### 실측 성능 (curl 테스트)
```
oeeData:    1.81ms
aggregated: 3.84ms
trends:     4.77ms
machineOee: 2.39ms
total:      12.83ms
```

### SSE 500 오류 근본 원인 및 해결

**원인:** mod_fcgid 기본 `FcgidOutputBufferSize = 65536 bytes`
PHP `flush()` 호출 시 데이터가 mod_fcgid 내부 버퍼(64KB)에 갇혀 클라이언트에게 전달되지 않음.
SSE long-polling 스크립트는 종료되지 않으므로 버퍼가 영구적으로 비워지지 않아 HTTP 500 반환.

**해결:** `C:/laragon/etc/apache2/fcgid.conf`에 추가:
```
FcgidOutputBufferSize 0
```
→ PHP `flush()` 시 mod_fcgid가 즉시 클라이언트에 전달 (Transfer-Encoding: chunked)

**결과:**
- 기존: HTTP 500, 클라이언트 0 bytes 수신
- 수정 후: HTTP 200 OK, SSE 이벤트 실시간 스트리밍 정상 작동

---

## 5-2. Phase 2-B/2-C/3 완료 기록 (2026-03-07)

### Phase 2-B: data_andon_stream.php
- `getAndonTypeStats()`: 2쿼리 + PHP merge → `info_andon LEFT JOIN data_andon` 1쿼리
- JOIN 키: `ia.idx = da.andon_idx`, `color` 컬럼 포함

### Phase 2-C: data_defective_stream.php
- `getDefectiveTypeStats()`: 2쿼리 + PHP merge → `info_defective LEFT JOIN data_defective` 1쿼리
- JOIN 키: `id.idx = dd.defective_idx`
- fallback (DISTINCT) 쿼리 제거

### Phase 3-A: lib/stream_helper.lib.php 신규 생성
공통 함수 3개 추출:

| 함수 | 설명 |
|------|------|
| `sendSSEData($eventType, $data)` | SSE 이벤트 출력 + flush |
| `parseFilterParams($tableAlias, $dateColumn, $isDateOnly, $defaultInterval)` | 파라미터화된 WHERE 빌더 |
| `getWorkHoursForDate($pdo, $targetDate)` | 시프트 시작/종료 시간 계산 |

### Phase 3-B: 5개 stream 파일 공통화
각 파일에서 중복 정의 제거 후 `require_once` 추가:

| 파일 | parseFilterParams 호출 |
|------|------------------------|
| data_oee_stream.php | `parseFilterParams('do', 'work_date', true,  '7 DAY')` |
| log_oee_stream.php  | `parseFilterParams('do', 'work_date', true,  '7 DAY')` |
| data_downtime_stream.php | `parseFilterParams('dd', 'reg_date',  false, '2 DAY')` |
| data_andon_stream.php    | `parseFilterParams('da', 'reg_date',  false, '2 DAY')` |
| data_defective_stream.php | `parseFilterParams('dd', 'reg_date',  false, '2 DAY')` |

제거된 중복 코드: 약 **350줄** (파일당 약 70줄 × 5파일)

---

## 5-3. Phase 4 완료 기록 (2026-03-07)

### 배경
`date_default_timezone_set('Asia/Jakarta')` 가 27개 파일에 중복 선언되어 있었음.
오타 버그(`Asia/Jajarta`) 1건 포함.

### Phase 4-A: 타임존 중앙화
- **유지**: `lib/db.php:5` (주석 추가 — "중앙 설정, 개별 파일 중복 선언 금지")
- **제거**: proc 22개, inc 1개, lib 2개 → 총 **26개 파일**에서 제거
- **오타 수정**: `lib/get_shift.lib.php` `'Asia/Jajarta'` → `'Asia/Jakarta'` (타임존 설정 자체도 제거)

### Phase 4-B: worktime_database.php 통합
- `lib/worktime_database.php` — `lib/db.php`와 내용 동일 (PDO 연결 + config.php)
- 에러 처리만 상이 (echo+exit vs 로그+JSON 응답) → `db.php`가 더 정교
- 사용 파일 5개 경로 변경 후 `worktime_database.php` 삭제

| 파일 | 변경 내용 |
|------|-----------|
| `inc/worktime_head.php` | `../../lib/worktime_database.php` → `__DIR__.'/../lib/db.php'` + 타임존 제거 |
| `page/manage/proc/ajax_factory_line.php` | `worktime_database.php` → `db.php` |
| `page/manage/proc/set_work_time_insert.php` | 동일 |
| `page/manage/proc/set_work_time_fetch.php` | 동일 |
| `page/manage/proc/set_work_time_fetch_month.php` | 동일 |

### Phase 4-C: report_stream.php 버그 수정
- `getDBConnection()` 함수가 프로젝트 어디에도 선언되지 않아 Fatal error 발생
- `global $pdo; $this->pdo = $pdo;` 로 수정 (db.php 전역 변수 직접 참조)
- 중복 null 체크 `if (!$this->pdo) die(...)` 제거 (db.php에서 이미 처리)

---

## 6. 참고 — 최적화 전후 비교 (예상)

### data_oee_stream.php SSE 사이클 (5초마다)

| 항목 | 최적화 전 | 최적화 후 |
|------|-----------|-----------|
| 쿼리 횟수 | 7회 | 4회 |
| 중복 집계 | stats + details + component (3회) | 1회 통합 |
| workHours 호출 | 2회 (trend + production) | 1회 |
| 예상 총 쿼리 시간 | ~800ms ~ 2000ms | ~400ms ~ 800ms |

### log_oee_stream.php SSE 사이클 (5초마다)

| 항목 | 최적화 전 | 최적화 후 |
|------|-----------|-----------|
| 해시 조건 | 항상 전송 (버그) | 변화 시에만 전송 |
| 기본 데이터 전송량 | 매 5초 1000건 | 변화 시에만 |
| 네트워크 트래픽 | 상시 high | 대폭 감소 |
