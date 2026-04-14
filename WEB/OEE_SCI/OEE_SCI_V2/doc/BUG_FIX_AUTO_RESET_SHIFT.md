# BUG FIX: AUTO RESET — 교대 근무 날짜 경계 오작동

## 상태: 🧪 테스트 대기 중 (코드 구현 완료)

---

## 버그 요약

| 항목 | 내용 |
|---|---|
| 심각도 | **CRITICAL** |
| 발견일 | 2026-04-09 |
| 영향 범위 | 2교대 이상 운영 중인 모든 현장 |
| 증상 | 2교대(야간) 기기가 자정 이후 전원 On 시 잘못 초기화됨 |

---

## 근본 원인

### 현재 코드 (`andonJson.c:100-108`)
```c
uint32 lastDay    = g_ptrMachineParameter->lastPowerOnDateTime / 86400u;
setCurrentTime(...);
uint32 currentDay = (uint32)(RTC_GetUnixTime() / 86400u);
if(lastDay != currentDay)
{
    if(g_ptrMachineParameter->bAutoReset) ResetCount(); // ← 오작동
}
```

`/ 86400u`는 UTC **자정(00:00)**을 경계로 날짜를 구분한다.  
그러나 공장의 논리적 작업 경계는 자정이 아니라 **교대(Shift)**이다.

---

## 버그 시나리오

### 시나리오 A — 잘못된 리셋 (False Reset)
```
2교대 스케줄: 21:00 (1/1) ~ 06:00 (1/2)

22:00 Jan 1  → 기기 운전 중, lastPowerOnDateTime 저장됨
22:30 Jan 1  → 기기 전원 OFF
00:30 Jan 2  → 기기 전원 ON (여전히 2교대 진행 중)

lastDay    = Jan 1 (1/1 22:00 / 86400)
currentDay = Jan 2  ← 달력 날짜가 다름!

→ bAutoReset == ON → ResetCount() 호출 ← 잘못됨!
  (2교대는 아직 끝나지 않았음)
```

### 시나리오 B — 리셋 누락 (Missed Reset)
```
2교대가 06:00에 끝나고 07:00에 1교대 시작.

1교대 기기가 07:30에 전원 ON.
서버에서 시간 동기화 시도.

lastPowerOnDateTime는 같은 날(Jan 2) 00:30에 설정됨
lastDay    = Jan 2
currentDay = Jan 2  ← 같은 날!

→ ResetCount() 호출 안 됨 ← 잘못됨!
  (1교대 신규 시작인데 초기화 없음)
```

---

## 해결 방안: 방안 B — 서버가 work_date + shift_idx 제공

### 핵심 아이디어
달력 날짜(자정 기준) 대신, 서버가 이미 정확히 계산한 **논리적 작업일(work_date) + 교대번호(shift_idx)** 조합으로 비교한다.

### 인코딩 방식
```c
// lastPowerOnDateTime 필드를 재활용
// 구조체 변경 없음 → EEPROM CRC 호환 유지
uint32 key = work_date_YYYYMMDD * 10u + shift_idx;
// 예: work_date=2026-04-08, shift_idx=2 → 202604082
// uint32 최대값: 4294967295 >> 충분 (최대 999912319)
```

### 비교 로직
```c
// NEW: Shift 기반
if (lastPowerOnDateTime != newKey)   → ResetCount()
g_ptrMachineParameter->lastPowerOnDateTime = newKey;

// FALLBACK: 서버가 work_date/shift_idx 미지원 시 기존 방식
if (lastDay != currentDay)           → ResetCount()
```

---

## 변경 파일 목록

| # | 파일 | 종류 | 상태 |
|---|---|---|---|
| 1 | `WEB/OEE_SCI/OEE_SCI_V2/api/sewing/get_dateTime.php` | 서버 PHP | ✅ 완료 |
| 2 | `PSOC/.../andonApi.c` | 펌웨어 C | ✅ 완료 |
| 3 | `PSOC/.../andonJson.c` | 펌웨어 C | ✅ 완료 |

---

## 상세 변경 내용

### 1. `get_dateTime.php` (서버)

**Before:**
```php
$response = $apiHelper->createResponse_onlyItems(['datetime' => $today]);
// 응답: {"datetime": "2026-04-09 02:30:00"}
```

**After:**
```php
// mac으로 factory/line 조회 → 현재 shift 조회
// 응답: {"datetime": "2026-04-09 02:30:00", "work_date": "2026-04-08", "shift_idx": 2}
```

### 2. `andonApi.c` — `makeAndonCurrentTimeRequest()`

**Before:**
```c
enQueueANDON_printf(ANDON_CURRENT_TIME, "get_dateTime");
```

**After:**
```c
enQueueANDON_printf(ANDON_CURRENT_TIME, "get_dateTime&mac=%s", g_network.MAC);
```

### 3. `andonJson.c` — `andonCurrentTimeParsing()`

**Before:** `lastPowerOnDateTime / 86400` 달력 날짜 비교  
**After:**  `work_date * 10 + shift_idx` 교대 기반 비교 (서버 없으면 fallback)

---

## 주의 사항

- **구조체 변경 없음**: `MACHINE_PARAMETER`에 필드를 추가하지 않으므로 EEPROM 레이아웃/CRC 변경 없음.
- **Fallback 보장**: 서버가 구버전이어서 `work_date`/`shift_idx`를 미지원해도 기존 달력 비교로 동작.
- **최초 전환 시 1회 리셋**: 서버 업데이트 직후, 저장된 이전 Unix time 기반 값과 신규 인코딩 값이 달라 1회 리셋이 발생할 수 있음 → 정상 동작.
- **근무 외 시간**: `getCurrentShiftInfo()`가 null 반환 시 `work_date`/`shift_idx` 미포함 → fallback 동작.

---

## 구현 진행 상황

- [x] 분석 완료 (2026-04-09)
- [x] `get_dateTime.php` 서버 수정 (2026-04-09)
- [x] `andonApi.c` 펌웨어 수정 (2026-04-09)
- [x] `andonJson.c` 펌웨어 수정 (2026-04-09)
- [ ] 통합 테스트: 2교대 자정 경계 시나리오
- [ ] 통합 테스트: 1교대→2교대 교대 전환 시나리오
- [ ] 현장 배포

---

## 테스트 체크리스트

| 시나리오 | 기대 결과 | 확인 |
|---|---|---|
| 2교대 중 자정 넘어 전원 ON | 리셋 **안 됨** | ⬜ |
| 2교대→1교대 전환 후 전원 ON | 리셋 **됨** | ⬜ |
| 근무 외 시간 전원 ON (fallback) | 달력 날짜 비교로 동작 | ⬜ |
| 서버 구버전 (work_date 없음) | fallback 달력 비교 | ⬜ |
| 최초 배포 직후 | 1회 리셋 후 정상 동작 | ⬜ |
