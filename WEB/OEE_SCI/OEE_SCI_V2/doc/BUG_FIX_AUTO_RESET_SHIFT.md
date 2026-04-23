# BUG FIX: AUTO RESET — 교대 근무 날짜 경계 오작동

## 상태: 🧪 테스트 대기 중 (코드 구현 완료 — 자수기 5분 폴링 추가 2026-04-23)

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

### Phase 1 — 교대 기반 비교 (2026-04-09)

| # | 파일 | 종류 | 상태 |
|---|---|---|---|
| 1 | `WEB/OEE_SCI/OEE_SCI_V2/api/sewing/get_dateTime.php` | 서버 PHP | ✅ 완료 |
| 2 | `PSOC/.../EMBROIDERY_S/andonApi.c` | 자수기 펌웨어 | ✅ 완료 |
| 3 | `PSOC/.../EMBROIDERY_S/andonJson.c` | 자수기 펌웨어 | ✅ 완료 |

### Phase 2 — 재부팅 없는 교대 전환 감지 (2026-04-23, 자수기 전용)

| # | 파일 | 종류 | 변경 내용 |
|---|---|---|---|
| 4 | `PSOC/.../EMBROIDERY_S/andonApi.c` | 자수기 펌웨어 | 5분 폴링 타이머 등록 + `andonLoop()` 주기 호출 |
| 5 | `PSOC/.../EMBROIDERY_S/andonJson.c` | 자수기 펌웨어 | EEPROM 쓰기 최적화 (key 변경 시에만 저장) |

---

## 추가 문제 — 재부팅 없는 교대 전환 감지 (Phase 2)

### 문제

자수기는 8시간씩 3교대로 운영되며, 교대 전환 시 디바이스를 재부팅하지 않는 경우가 발생한다.  
Phase 1의 shift 비교는 **부팅 시 1회만** `get_dateTime`을 호출하므로, 디바이스가 켜진 상태로 교대를 넘기면 shift 변경을 영구히 감지하지 못한다.

### 해결 — 5분 주기 `get_dateTime` 폴링

```
initAndon() → 5분 타이머 등록 (300,000ms)

매 5분 andonLoop() 내부:
  └─ makeAndonCurrentTimeRequest() → 큐에 enqueue
       └─ andonCurrentTimeParsing()
            ├─ shift key 동일 → 아무것도 안함
            └─ shift key 변경 → ResetCount() + EEPROM 저장
```

### EEPROM 쓰기 최적화 (함께 수정)

Phase 1 코드는 폴링 응답마다 무조건 `SaveExternalFlashConfig()`를 호출하고 있었다.  
5분 폴링을 추가하면 하루 288회 불필요한 플래시 쓰기가 발생하므로, **shift key가 변경될 때만** 저장하도록 수정했다.

**Before:**
```c
g_ptrMachineParameter->lastPowerOnDateTime = currentKey;
SaveExternalFlashConfig();  // 매 응답마다 무조건 저장
```

**After:**
```c
if (lastKey != currentKey)
{
    if (g_ptrMachineParameter->bAutoReset) ResetCount();
    g_ptrMachineParameter->lastPowerOnDateTime = currentKey;
    SaveExternalFlashConfig();  // key 변경 시에만 저장
}
```

| 항목 | Before | After |
|---|---|---|
| 플래시 쓰기 (8시간 교대 중) | 96회 | 0회 |
| 플래시 쓰기 (교대 전환 시) | 1회 | 1회 |
| 하루 최대 쓰기 | 288회 | 3회 |

---

## 재봉기(SEWING) 적용 가이드

자수기(EMBROIDERY_S)와 재봉기(IoT_INTEGRATED)는 `andonApi.c` / `andonJson.c` 구조가 동일하므로 동일한 패턴을 적용할 수 있다.

### 적용 체크리스트

**`andonApi.c`**

1. 전역 변수 추가:
```c
static uint8 g_index_shift_check = 0;
```

2. `initAndon()` 에 타이머 등록 추가:
```c
g_index_shift_check = registerCounter_1ms(5UL * 60UL * 1000UL);
```

3. `andonLoop()` 의 `g_bReceivedAndonStart` 블록에 추가:
```c
if(isFinishCounter_1ms(g_index_shift_check))
    makeAndonCurrentTimeRequest();
```

**`andonJson.c`** — `andonCurrentTimeParsing()` Pass 3 수정

교대 기반 경로:
```c
// 변경 전: 무조건 저장
g_ptrMachineParameter->lastPowerOnDateTime = currentKey;
SaveExternalFlashConfig();

// 변경 후: key 변경 시에만
if (lastKey != currentKey) {
    if (g_ptrMachineParameter->bAutoReset) ResetCount();
    g_ptrMachineParameter->lastPowerOnDateTime = currentKey;
    SaveExternalFlashConfig();
}
```

Fallback 경로도 동일 패턴으로 수정:
```c
if (lastDay != currentDay) {
    if (g_ptrMachineParameter->bAutoReset) ResetCount();
    g_ptrMachineParameter->lastPowerOnDateTime = (uint32)RTC_GetUnixTime();
    SaveExternalFlashConfig();
}
```

> **주의**: `MAX_NO_MILISECOND_COUNTER = 15`, 현재 자수기 기준 4개 슬롯 사용 중.  
> 재봉기 프로젝트에서도 슬롯 여유를 확인 후 등록할 것.

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

### 3. `andonJson.c` — `andonCurrentTimeParsing()` (Phase 1)

**Before:** `lastPowerOnDateTime / 86400` 달력 날짜 비교  
**After:**  `work_date * 10 + shift_idx` 교대 기반 비교 (서버 없으면 fallback)

### 4. `andonApi.c` — 5분 폴링 추가 (Phase 2, 자수기)

```c
// 추가된 변수
static uint8 g_index_shift_check = 0;

// initAndon() 에 추가
g_index_shift_check = registerCounter_1ms(5UL * 60UL * 1000UL);

// andonLoop() 에 추가
if(isFinishCounter_1ms(g_index_shift_check))
    makeAndonCurrentTimeRequest();
```

### 5. `andonJson.c` — EEPROM 쓰기 최적화 (Phase 2, 자수기)

**Before:** shift key 변경 여부와 무관하게 매 응답마다 `SaveExternalFlashConfig()` 호출  
**After:** `lastKey != currentKey` 일 때만 `ResetCount()` + `SaveExternalFlashConfig()` 호출

---

## 주의 사항

- **구조체 변경 없음**: `MACHINE_PARAMETER`에 필드를 추가하지 않으므로 EEPROM 레이아웃/CRC 변경 없음.
- **Fallback 보장**: 서버가 구버전이어서 `work_date`/`shift_idx`를 미지원해도 기존 달력 비교로 동작.
- **최초 전환 시 1회 리셋**: 서버 업데이트 직후, 저장된 이전 Unix time 기반 값과 신규 인코딩 값이 달라 1회 리셋이 발생할 수 있음 → 정상 동작.
- **근무 외 시간**: `getCurrentShiftInfo()`가 null 반환 시 `work_date`/`shift_idx` 미포함 → fallback 동작.

---

## 구현 진행 상황

### Phase 1 — 교대 기반 비교
- [x] 분석 완료 (2026-04-09)
- [x] `get_dateTime.php` 서버 수정 (2026-04-09)
- [x] `get_dateTime.php` pre-shift window 10분 추가 (2026-04-21)
- [x] `andonApi.c` mac 파라미터 추가 (2026-04-09)
- [x] `andonJson.c` shift 기반 비교 로직 (2026-04-09)

### Phase 2 — 재부팅 없는 교대 전환 감지 (자수기)
- [x] `andonApi.c` 5분 폴링 타이머 등록 (2026-04-23)
- [x] `andonJson.c` EEPROM 쓰기 최적화 (2026-04-23)
- [ ] 통합 테스트: 2교대 자정 경계 시나리오
- [ ] 통합 테스트: 교대 전환 시 5분 이내 자동 ResetCount 시나리오
- [ ] 재봉기(IoT_INTEGRATED) 동일 패턴 적용
- [ ] 현장 배포

---

## 테스트 체크리스트

### Phase 1 — 부팅 시 교대 비교

| 시나리오 | 기대 결과 | 확인 |
|---|---|---|
| 2교대 중 자정 넘어 전원 ON | 리셋 **안 됨** | ⬜ |
| 2교대→1교대 전환 후 전원 ON | 리셋 **됨** | ⬜ |
| 근무 시작 10분 전 전원 ON | shift 정보 반환, 리셋 **안 됨** | ⬜ |
| 근무 시작 11분 전 전원 ON | shift 정보 없음, fallback 동작 | ⬜ |
| 근무 외 시간 전원 ON (fallback) | 달력 날짜 비교로 동작 | ⬜ |
| 서버 구버전 (work_date 없음) | fallback 달력 비교 | ⬜ |
| 최초 배포 직후 | 1회 리셋 후 정상 동작 | ⬜ |

### Phase 2 — 5분 폴링 (자수기)

| 시나리오 | 기대 결과 | 확인 |
|---|---|---|
| 디바이스 켠 채로 교대 전환 | 전환 후 최대 5분 이내 `ResetCount()` 자동 실행 | ⬜ |
| 같은 교대 내 5분마다 폴링 | 리셋 **안 됨**, 플래시 쓰기 **없음** | ⬜ |
| 교대 전환 후 1회 폴링 | 플래시 쓰기 **1회** 발생 | ⬜ |
| 근무 외 시간 폴링 (shift 없음) | fallback, 같은 날이면 리셋 안 됨 | ⬜ |
