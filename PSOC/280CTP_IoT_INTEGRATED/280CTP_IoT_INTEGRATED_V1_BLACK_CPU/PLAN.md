# 280CTP_IoT_INTEGRATED_V1_BLACK_CPU 개발 계획

> 베이스: `280CTP_IoT_INTEGRATED_V1` 복사본
> 목표: `USER_PROJECT_PATTERN_SEWING_MACHINE` 중 **PATTERN_MACHINE 전용 버전** (BLACK CPU 보드)
> 작성일: 2026-03-10
> **완료일: 2026-03-17 ✅ (코드 수정 완료 / 빌드 검증 완료)**

---

## 프로젝트 개요

`USER_PROJECT_PATTERN_SEWING_MACHINE` 은 아래 카운터 입력 방식들을 지원하는 **통합 버전**이다.

| 구분 | 카운터 입력 방식 | 하드웨어 |
|---|---|---|
| `PATTERN_MACHINE` | 초성 CPU UART 수신 (PatternCountLoop, uartJsonLoop) | UART (3.3~5V TTL High/Low 신호) |
| `SEWING_MACHINE` | TrimPin ISR 카운트 (SewingCountLoop) | TrimPin (디지털 입력, 인터럽트) |
| 공통 옵션 | 전류센서 카운트 | ADC_SAR_Seq (Port 5.5, 2pin) |

**BLACK_CPU 버전은 `PATTERN_MACHINE` 타입을 유지하되,**
**아래 카운터 입력을 제거하고, `uartJson` 은 BLACK CPU 전용 방식으로 유지한다.**

- 초성 CPU UART 3.3~5V H/L 신호 기반 `PatternCountLoop` 로직 → **제거** (uartJson 자체는 유지)
- TrimPin ISR 재봉 카운트 (`SewingCountLoop`) → **제거**
- 전류센서 (`ADC_SAR_Seq`, `currentSensor`) → **제거**

> ※ `uartJson.c/.h` 는 BLACK CPU 보드 전용 카운터 입력 방식으로 재활용 예정

---

## 제거 대상

### 1. 초성 CPU UART H/L 신호 (PatternCountLoop) 관련

| 파일 | 수정 내용 |
|---|---|
| `count.c` | `PatternCountLoop()` 내부 로직 정리 (초성 CPU 방식 제거, BLACK CPU 전용으로 재구성) |
| `count.c` | 기존 `uartJsonLoop()` 호출 방식 재검토 (BLACK CPU 전용 처리로 변경) |
| `uartJson.c/.h` | **유지** (BLACK CPU 보드 전용 카운터 입력 방식으로 재활용) |
| `userMenuPatternSewingMachine.c` | 초성 CPU 관련 수신 데이터 표시 메뉴 항목 정리 |

### 2. SEWING_MACHINE (TrimPin ISR 카운트) 관련

| 파일 | 수정 내용 |
|---|---|
| `count.c` | `SewingCountLoop()`, `Trim_Interrupt_Routine` ISR 제거 |
| `count.c` | `SetCountLoop()` 내 `SEWING_MACHINE` 분기 제거, `CountFunc` → `PatternCountLoop` 고정 |
| `count.c` | `g_bStartTrimPin`, `g_bTrimElapsedTime` 변수 제거 |
| `count.h` | `COUNT` 구조체에서 `sewing*` 멤버 필드 제거 검토 (서버 JSON 호환성 확인 후 결정) |
| `count.h` | `g_bStartTrimPin`, `g_bTrimElapsedTime` extern 선언 제거 |
| `main.c` | `g_bStartTrimPin`, `makeAndonSewingCount2()` 블록 제거 |
| `userProjectPatternSewing.h` | `MACHINE_TYPE` enum에서 `SEWING_MACHINE` 제거, `PATTERN_MACHINE` 단일 타입 고정 |
| `userProjectPatternSewing.h` | `MACHINE_PARAMETER` 구조체에서 `sewingTarget`, `sewingPairTrim`, `sewingPair` 필드 제거 |
| `userProjectPatternSewing.c` | machineType 고정 처리, sewing 관련 참조 코드 제거 |
| `userMenuPatternSewingMachine.c` | Machine Type 선택 메뉴 제거, Sewing 관련 메뉴 항목 제거 |

### 3. 전류센서 (ADC_SAR_Seq) 관련

| 파일 | 수정 내용 |
|---|---|
| `currentSensor.c` | **파일 전체 제거** |
| `currentSensor.h` | **파일 전체 제거** |
| `main.c` | `currentSensorRoutine()` 호출 제거, `#include "currentSensor.h"` 제거 |
| `package.h` | `USE_CURRENT_SENSOR_FOR_COUNTTING` define 완전 제거 |
| `userProjectPatternSewing.h` | `MACHINE_PARAMETER` 구조체에서 `current_enable`, `current_sensor_threshold` 필드 제거 |
| `userProjectPatternSewing.c` | 위 필드 참조 코드 제거 |
| `userMenuPatternSewingMachine.c` | 전류센서 메뉴 항목 (`CURRENT`, `CUR.SENSOR`, `THRESHOLD`, `MEASURE`) 제거 |
| `andonApi.c` | `current_enable` 관련 코드 정리 |
| PSoC Creator TopDesign | **ADC_SAR_Seq 컴포넌트 제거** (하드웨어 리소스 절약) |
| Generated_Source | ADC_SAR_Seq_*.c/h 파일들 PSoC Creator 재생성으로 자동 제거 |

---

## 유지 대상

- `machineType = PATTERN_MACHINE` 고정
- `uartJson.c/.h` 유지 (BLACK CPU 보드 전용 카운터 입력 방식으로 재활용)
- WIFI, LCD, USB, 버저, LED, ANDON API, 서버 통신 전체 유지
- `package.h` 의 `USER_PROJECT_PATTERN_SEWING_MACHINE` define 유지
- `PROJECT_FIRMWARE_VERSION` → `"BLACK_CPU V1"` 으로 변경

---

## firmware version 변경

```c
// package.h
#define PROJECT_FIRMWARE_VERSION "BLACK_CPU V1"
```

---

## 작업 순서 (2026-03-13 실제 코드 수정 시)

> **2026-03-17 완료 현황**

1. ~~PSoC Creator TopDesign 열어서 ADC_SAR_Seq 컴포넌트 제거 → 빌드~~ → ⚠️ **PSoC Creator에서 수동 작업 필요** (currentSensor.c는 stub으로 처리하여 ADC 없이도 컴파일 가능 상태)
2. ~~`currentSensor.c/.h` 파일 제거~~ → ✅ **stub으로 교체** (ADC 참조 제거, 빈 함수로 대체)
3. ~~`main.c` 에서 currentSensor, TrimPin(g_bStartTrimPin) 관련 코드 제거~~ → ✅ **완료**
4. ~~`package.h` 정리~~ → ✅ **완료** (`PROJECT_FIRMWARE_VERSION` = `"BLACK_CPU V1"`)
5. ~~`userProjectPatternSewing.h/c` 에서 `SEWING_MACHINE`, sewing 관련 코드 제거~~ → ✅ **완료** (구조체 필드는 flash 호환성 유지)
6. ~~`count.c/.h` 에서 SewingCountLoop, TrimPin ISR, 관련 변수 제거~~ → ✅ **완료**
7. ~~`PatternCountLoop()` BLACK CPU 전용 방식으로 재구성 (uartJson 활용)~~ → ✅ **완료** (기존 uartJsonLoop 유지, CountFunc 고정)
8. ~~`userMenuPatternSewingMachine.c` 에서 전류센서/Sewing/초성CPU 메뉴 제거~~ → ✅ **완료**
9. ~~`andonApi.c` sewing 관련 함수 제거~~ → ✅ **완료** (`makeAndonSewingCount`, `makeAndonSewingCount2`, `updateRuntimeSum` 제거)
10. ~~`History.txt` 업데이트~~ → ✅ **완료**
11. 빌드 → ✅ **성공** (Flash 46.8%, SRAM 68.8%)

### 잔여 작업 (PSoC Creator 수동)

- [x] TopDesign에서 `ADC_SAR_Seq` 컴포넌트 제거 후 재빌드 ✅ 2026-03-17
- [x] 프로젝트 소스 목록에서 `currentSensor.c` 제거 (우클릭 → Remove from Build) ✅ 2026-03-17

> **모든 작업 완료** — Flash 46.8% / SRAM 68.8% (ADC 제거 후 동일, stub이 이미 ADC 참조 없었음)

---

## 참고: V1 과 BLACK_CPU 차이 요약

| 항목 | V1 (통합) | BLACK_CPU (전용) |
|---|---|---|
| PATTERN_MACHINE 타입 | 지원 | **유지 (고정)** |
| 초성 CPU UART H/L 신호 카운트 | 지원 | **제거** |
| SEWING_MACHINE (TrimPin ISR) | 지원 | **제거** |
| 전류센서 카운트 (ADC_SAR_Seq) | 지원 (옵션) | **제거** |
| machineType 설정 | 사용자 선택 | **PATTERN_MACHINE 고정** |
| ADC_SAR_Seq | 사용 | **제거** |
