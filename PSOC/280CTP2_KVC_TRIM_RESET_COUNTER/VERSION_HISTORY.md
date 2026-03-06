# KVC TRIM RESET COUNTER 펌웨어 버전 히스토리

> 대상 MCU: Cypress PSoC 4 (CY8C42xx 시리즈, Cortex-M0)
> 개발툴: PSoC Creator 4.x / ARM GCC 5.4
> 역할: 생산 라인 Trim Count 목표 관리 및 부정 리셋 방지 시스템

---

## 280CTP2_KVC_TRIM_RESET_COUNTER_V1

| 항목 | 내용 |
|------|------|
| **버전 식별자** | `KVC Trim Reset V1` |
| **프로젝트 폴더** | `280CTP2_KVC_TRIM_RESET_COUNTER_V1/` |
| **워크스페이스** | `Design.cydsn/` |
| **작업일** | 2025-12-25 |
| **상태** | 보안 취약점 존재 — 운영 환경 미권장 (V2로 교체) |

### 메모리 사용량

| 영역 | 사용 | 전체 | 점유율 |
|------|------|------|--------|
| Flash (Bootloader) | 13,312 bytes | 262,144 bytes | - |
| Flash (Application) | 80,900 bytes | 262,144 bytes | - |
| Flash (합계) | 94,212 bytes | 262,144 bytes | 35.9% |
| **SRAM** | **약 19,788 bytes** | **32,768 bytes** | **60.4%** |

> 참고: WiFi/서버/JSON 기능 포함 상태의 추정값. V2 SRAM 최적화 보고서 기준.

### 하드웨어 구성

| 컴포넌트 | 역할 |
|---------|------|
| `TC_INT` | Trim Count 신호 입력 |
| `TC_RESET` | 물리적 RESET 버튼 |
| `BUZZER` | 알림음 출력 |
| `SPIM_LCD` | ST7789V LCD SPI (320x240) |
| `I2C_TC` | 터치 컨트롤러 |
| `UART` | 디버그 출력 |
| `RESERVED_OUT_1` | Trim 완료 외부 신호 |

### 주요 기능

- **RESET 조건 강화 (Phase 1)**: `count != 0` → `count == setTrimCount` 변경으로 Target 도달 시에만 RESET 허용
- **LCD 경고 메시지 (Phase 2)**: Target 미달성 RESET 시도 시 "Target Not Reached!\n%d/%d" 표시 + 경고음
- **WiFi/JSON/서버 기능 포함**: jsmn-master 라이브러리, WIFI, server, andonApi 등 IoT 코드 존재 (미사용)

### 알려진 문제점 (V2에서 수정됨)

| 심각도 | 파일 | 문제 |
|--------|------|------|
| **HIGH** | `lib/widget.c` | `ShowMessage()` 버퍼 20바이트 — 포맷 결과 오버플로우 가능 |
| **MEDIUM** | `lib/button.c` | 버튼 텍스트 16글자 제한 — 타이틀 표시 잘림 |
| **MEDIUM** | `lib/widget.c` | WiFi 영역이 TitleBar 25% 차지 (불필요한 공간 낭비) |
| **LOW** | `count.c` | 경고음 자동 정지 없음 (계속 울림) |
| **LOW** | `userMenuCounter.c` | 경고 메시지 후 화면 잔상 |

### 변경 이력

#### 2025-12-25 (초기 보안 개선)
- Phase 1: RESET 조건 강화 — `count != 0` → `count == setTrimCount`
- Phase 2: Target 미달성 시 LCD 경고 메시지 + 경고음 추가
  - `ShowMessage("Target Not Reached!\n%d/%d", ...)` — 1.5초 표시
  - `Buzzer(BUZZER_WARNING, 100)` 호출

---

## 280CTP2_KVC_TRIM_RESET_COUNTER_V2

| 항목 | 내용 |
|------|------|
| **버전 식별자** | `KVC Trim Reset V2.1` |
| **프로젝트 폴더** | `280CTP2_KVC_TRIM_RESET_COUNTER_V2/` |
| **워크스페이스** | `Design.cydsn/` |
| **작업일** | 2025-12-29 ~ 2025-12-30 |
| **상태** | 운영 배포 권장 버전 — 전 기능 검증 완료 |

### 메모리 사용량

| 영역 | 사용 | 전체 | 점유율 |
|------|------|------|--------|
| Flash (Bootloader) | 13,312 bytes | 262,144 bytes | - |
| Flash (Application) | 64,400 bytes | 262,144 bytes | - |
| Flash (합계) | 77,712 bytes | 262,144 bytes | 29.7% |
| **SRAM** | **11,532 bytes** | **32,768 bytes** | **35.2%** |

> V1 대비 SRAM **8,256 bytes (41.7%) 절감**, Flash **16,500 bytes (17.5%) 절감**

### 하드웨어 구성

V1과 동일 (TC_INT, TC_RESET, BUZZER, SPIM_LCD, I2C_TC, UART, RESERVED_OUT_1)

> 참고: WIFI, UART_WIFI PSoC Creator 하드웨어 컴포넌트는 Generated_Source에 잔존하나, 소프트웨어 코드에서 완전 제거되어 SRAM 영향 없음.

### 주요 기능

- **RESET 조건 강화 (Phase 1)**: V1 동일 — `count == setTrimCount`
- **경고 메시지 개선 (Phase 2→4)**:
  - 메시지 간소화: `"Not Reached! %d/%d"` (단일 라인)
  - `ShowMessage()` 버퍼 크기 `20` → `64` (오버플로우 수정)
  - 경고음 2초 후 자동 정지: `Buzzer(BUZZER_STOP, 0)`
  - 화면 복원: `EraseBlankAreaWithoutHeader()` + `g_updateCountMenu = 2` (전체 갱신)
- **LCD 타이틀 바 확장 (Phase 3)**: WiFi 비활성화로 TitleBar `256px → 319px` (+25%)
- **SET 메뉴 부트로더 진입 (Phase 5)**:
  - `EnterBootloaderMode()` 함수 — `Bootloadable_Load()` + `CySoftwareReset()`
  - 메뉴: SET → USB Update → "Going to UPDATE?" 확인 → 부트로더 진입
- **3단계 계층 메뉴 (Phase 5)**:
  - ROOT → SET Submenu → Set Target / USB Update
- **UI 개선 (Phase 6)**: 버튼 100% 너비 사용, 텍스트 중앙 정렬, 아웃라인 제거
- **버튼 텍스트 확장 (Phase 3)**: `MAX_NUM_BUTTON_STRING 16 → 24` (+50%)
- **SRAM 최적화**: WiFi/서버/JSON 완전 제거 — 17개 파일 삭제

### 변경 이력

#### 2025-12-29 (Phase 3~4, SRAM 최적화)
- Phase 3: WiFi 초기화·DrawWifi() 비활성화, TitleBar 전체 너비 사용
- Phase 3: `MAX_NUM_BUTTON_STRING` 16 → 24
- Phase 4: `ShowMessage()` 버퍼 `char buff[20]` → `char buff[64]`
- Phase 4: 경고 메시지 간소화 `"Not Reached! %d/%d"`
- Phase 4: 경고음 2초 자동 정지 추가
- Phase 4: `EraseBlankAreaWithoutHeader()` + `g_updateCountMenu = 2` 화면 복원
- SRAM 최적화: WiFi/서버/JSON 관련 17개 파일 삭제
  - `lib/WIFI.c/h`, `lib/server.c/h`, `lib/jsmn.c/h`
  - `USBJsonConfig`, `USBUARTConfig`, `jsonUtil`, `uartJson`, `andonJson`, `andonApi`
  - `jsmn-master/` 디렉토리 전체
- SRAM 최적화: WiFi 이미지 5개 (`image_wifi_0~4`) 삭제 (약 8KB)
- SRAM 결과: **19,788 bytes (60.4%) → 11,532 bytes (35.2%)** — 8,256 bytes 절감

#### 2025-12-30 (Phase 5~6, 최종 완성)
- Phase 5: `EnterBootloaderMode()` 함수 구현 (`main.c`, `main.h`)
- Phase 5: SET 서브메뉴 화면 추가 (`doSetSubmenu`, `DisplayDoSetSubmenu`)
- Phase 5: USB Update 확인 화면 추가 (`doUSBUpdate`, `DisplayDoUSBUpdate`)
- Phase 5: 메뉴 트리 3단계 계층 구성 (`menuCreate()`)
- Phase 5: 타이틀 텍스트 `"KVC Trim Reset V2.1"` 설정
- Phase 6: 버튼 크기 화면 100% 사용 (각 50%)
- Phase 6: USB Update 확인 텍스트 수직 중앙 정렬 (`centerY` 계산)
- Phase 6: 아웃라인 색상 BLACK으로 퍼플 라인 제거
- 기능 테스트 완료: LCD, Trim Target 설정, Trim 카운트, Reset 버튼, LED 제어

---

*Copyright SUNTECH, 2018-2025*
