# IoT INTEGRATED 펌웨어 버전 히스토리

> 대상 MCU: Cypress PSoC 4 (CY8C42xx 시리즈, Cortex-M0)
> 개발툴: PSoC Creator 4.x / ARM GCC 5.4.1
> 역할: 재봉기 IoT 통합 모니터링 — ANDON 연동, 생산 카운팅, 가동/불량 관리

---

## 280CTP_IoT_INTEGRATED_V1_BLACK_CPU

| 항목 | 내용 |
|------|------|
| **버전 식별자** | `BLACK_CPU V1` |
| **프로젝트 폴더** | `280CTP_IoT_INTEGRATED_V1_BLACK_CPU/` |
| **워크스페이스** | `Project/Design.cydsn/` |
| **기반 버전** | `280CTP_IoT_INTEGRATED_V1` (V1 복사본) |
| **개발 목적** | PATTERN_MACHINE 전용 (BLACK CPU 보드) — SEWING/전류센서 제거 |
| **계획 수립일** | 2026-03-10 |
| **코드 수정 완료** | 2026-03-17 ✅ |
| **상태** | ✅ 완전 완료 (TopDesign ADC_SAR_Seq 제거 + currentSensor.c 프로젝트 제거 완료) |

### 메모리 사용량

> 2026-03-23 RESTART 메뉴 추가 후 빌드 결과 (실측)

| 영역 | 사용 | 전체 | 점유율 | 상태 |
|------|------|------|--------|------|
| Flash (전체) | 118,796 bytes | 262,144 bytes | 45.3% | ✅ 양호 |
| Flash (Bootloader) | 13,568 bytes | — | 5.2% | |
| Flash (Application) | 104,972 bytes | — | 40.0% | |
| Flash (Metadata) | 256 bytes | — | 0.1% | |
| **SRAM** | **22,580 bytes** | **32,768 bytes** | **68.9%** | ✅ 안정 |
| Stack | 2,048 bytes | — | — | |
| Heap | 1,024 bytes | — | — | |

> WiFi 아이콘 제거(2026-03-23) 대비 변화: Flash +176 bytes (RESTART 코드), SRAM +64 bytes (doRestart static 변수)
> V1 대비 누적 변화: Flash -13,008 bytes (-5.0%), SRAM -128 bytes

### V1 대비 변경 사항

| 항목 | V1 (통합) | BLACK_CPU V1 (전용) |
|---|---|---|
| `PROJECT_FIRMWARE_VERSION` | `"Integrated REV 9.8.3"` | `"BLACK_CPU V1"` |
| `PATTERN_MACHINE` | 지원 | **고정 (유일 타입)** |
| `SEWING_MACHINE` (TrimPin ISR) | 지원 | **제거** |
| 전류센서 (ADC_SAR_Seq) | 지원 (옵션) | **제거 (TopDesign + 프로젝트 소스 모두 제거)** |
| `machineType` 설정 | 사용자 선택 | **PATTERN_MACHINE 고정** |
| `Trim_Interrupt_Routine` ISR | 있음 | **제거** |
| `SewingCountLoop()` | 있음 | **제거** |
| `CountFunc` 초기화 | `&SewingCountLoop` | **`&PatternCountLoop` 고정** |
| `makeAndonSewingCount2()` | 있음 | **제거** |
| `currentSensor.c` | 전체 로직 | **프로젝트 소스에서 완전 제거** |

### 하드웨어 구성

V1과 동일하나 아래 항목 비활성화:

| 컴포넌트 | 상태 | 비고 |
|---------|------|------|
| `TrimPin` | ❌ 미사용 | ISR 제거됨 |
| `ADC_SAR_Seq` | ❌ 미사용 | stub 처리 / TopDesign 제거 예정 |
| `UART` (UART_WIFI) | ✅ 사용 | BLACK CPU UART 카운트 수신 |

### 주요 기능

- **PATTERN_MACHINE 전용 카운팅**: BLACK CPU 보드 UART JSON 수신 (`uartJsonLoop`)
  - `{"cmd":"count","value":1,...}` 형식 JSON 수신 → `patternActualH/L` 누적
- **ANDON 서버 연동**: HTTP GET → 생산 카운트 자동 전송 (`makeAndonPatternCount`)
- **LCD 터치 UI**: 모니터링 / ANDON / SET / RESET 메뉴 (PATTERN_MACHINE 전용 화면)
- **데이터 지속성**: 내부 EEPROM(카운트), 외부 Flash(서버/WiFi 설정)
  - ※ COUNT 구조체 sewing* 필드 유지 (internal flash 레이아웃 호환)
  - ※ MACHINE_PARAMETER sewing*/current_* 필드 유지 (external flash 레이아웃 호환)
- **USB 설정 도구 연동**: `SuntechIoTConfig_V1` C# 도구

### 수정된 파일 목록

| 파일 | 수정 내용 |
|------|---------|
| `package.h` | `PROJECT_FIRMWARE_VERSION` → `"BLACK_CPU V1"` |
| `main.c` | `currentSensorRoutine()` 전방선언/호출 제거, TrimPin `g_bStartTrimPin` 블록 제거 |
| `count.c` | `Trim_Interrupt_Routine` ISR 제거, `SewingCountLoop()` 제거, 관련 변수 제거, `CountFunc` 고정 |
| `count.h` | `g_bStartTrimPin`, `g_bTrimElapsedTime` extern 선언 제거 |
| `userProjectPatternSewing.h` | `SEWING_MACHINE` enum 제거 (구조체 필드는 flash 호환성 유지) |
| `userProjectPatternSewing.c` | `SEWING_MACHINE` 분기 제거, `machineType` 강제 `PATTERN_MACHINE` 고정 |
| `userMenuPatternSewingMachine.c` | `#include "currentSensor.h"` 제거, 활성 SEWING_MACHINE 분기 전부 제거 |
| `andonApi.c` | `makeAndonSewingCount2()`, `makeAndonSewingCount()`, `updateRuntimeSum()` 제거 |
| `andonApi.h` | 위 3개 함수 선언 제거 |
| `currentSensor.c` | ADC_SAR_Seq 참조 전부 제거 → stub 교체 |
| `History.txt` | 2026-03-17 변경 이력 기록 |

### 잔여 작업

- [x] PSoC Creator TopDesign에서 `ADC_SAR_Seq` 컴포넌트 제거 → 재빌드 ✅ 2026-03-17
- [x] 프로젝트 소스 목록에서 `currentSensor.c` 제거 (우클릭 → Remove from Build) ✅ 2026-03-17

> **모든 작업 완료** — Flash 46.8% / SRAM 68.8% (ADC stub이 이미 ADC 참조 없었으므로 메모리 수치 동일)

### 변경 이력

#### 2026-03-23 (RESTART 메뉴 추가 + 스크롤 버그 수정 + 미사용 폰트 비활성화)

**RESTART 메뉴 추가**
- `lib/manageMenu.c` `doRestart()` 함수 추가 — `doFactoryReset` 패턴과 동일한 YES/NO 확인 다이얼로그
  - 터치 흐름: LCD → MENU → RESTART → "Do you want ?" → OK → `CySoftwareReset()` 재부팅
  - QUIT 버튼: 이전 메뉴로 복귀, OK 버튼: `ShowWaitMessage()` 후 소프트웨어 리셋
- `lib/manageMenu.c` `manageMenuCreate()` 에 `createMENUNODE(root, "RESTART", &doRestart)` 추가 (WIFI INFO 다음, 마지막 위치)

**스크롤 버그 수정 (7번째+ 메뉴 터치 시 이전 페이지로 이동하는 문제)**
- `lib/widget.h` `enum BASIC_MENU_IDX` 의 `IDX_SCROLL_UP`, `IDX_SCROLL_DOWN` 값 변경
  - 변경 전: `IDX_SCROLL_UP=6`, `IDX_SCROLL_DOWN=7`
  - 변경 후: `IDX_SCROLL_UP=0x10(16)`, `IDX_SCROLL_DOWN=0x11(17)`
  - 원인: `getIndexOfClickedListMenu()`가 7번째 아이템 클릭 시 인덱스 6 반환 → `doListMenuPage` switch에서 `IDX_SCROLL_UP(=6)` 케이스로 잘못 분기 → 페이지 감소(이전 단계 이동)
  - 수정 효과: `listText` 최대 15개이므로 인덱스 최대 14, 0x10(16) 이상 값은 충돌 없음

**미사용 폰트 비활성화 (`lib/fonts.h`)**
- `#define _FONT_DotMatrix_M_16x22_`, `_FONT_AlibriNumBold32x48_`, `_FONT_ArialNumFontPlus32x50_`, `_FONT_SevenSegNumFontPlusPlus32x50_` 주석 처리
- 활성 유지: `_FONT_Grotesk16x32_`, `_FONT_SmallFont8x12_`, `_FONT_Arial_round_16x24_`
- 비고: PSoC Creator `--gc-sections` 링커가 미참조 `static const` 배열을 이미 제거하므로 Flash 크기 변화 없음, 향후 유지보수 명확화 목적

**빌드 결과**: Flash 118,620 → 118,796 bytes (+176 bytes), SRAM 22,516 → 22,580 bytes (+64 bytes)

---

#### 2026-03-23 (LCD 상단 WiFi 아이콘 제거 — Flash 최적화)
- `lib/image.h` WiFi 아이콘 배열 5개 전체 제거 (`image_wifi_0` ~ `image_wifi_4`, 약 10KB)
- `lib/widget.c` `DrawWifi()` 수정: 미연결 시 빈 텍스트 대신 빨간 `"0"` 표시
- `lib/widget.c` `initWidget()` WiFi 아이콘 좌표 섹션 제거, TitleBar 영역 확장 (176px → 208px)
- `lib/widget.c` `g_imageWifi`, `g_rectWifi` 변수 제거
- `lib/widget.h` `extern IMAGE g_imageWifi` 선언 제거
- Flash: 122,812 → 118,620 bytes (-4,192 bytes, -1.6%), SRAM: 22,532 → 22,516 bytes (-16 bytes)

#### 2026-03-17 (BLACK_CPU V1 코드 수정 완료)
- V1 통합 버전 기반으로 PATTERN_MACHINE 전용 BLACK CPU 버전 분리
- SEWING_MACHINE (TrimPin ISR), 전류센서(ADC_SAR_Seq), 초성 CPU H/L 신호 로직 제거
- `uartJson.c/.h` 유지 — BLACK CPU 보드 UART JSON 카운터 입력 방식으로 재활용
- 빌드 결과: Flash 122,812/262,144 (46.8%), SRAM 22,532/32,768 (68.8%)

---

## 280CTP_IoT_INTEGRATED_V1

| 항목 | 내용 |
|------|------|
| **버전 식별자** | `Integrated REV 9.8.3` |
| **프로젝트 폴더** | `280CTP_IoT_INTEGRATED_V1/` |
| **워크스페이스** | `Project/Design.cydsn/` |
| **기반 버전** | `IoT_SCI_2025.07.23 Rev.9.8.3` |
| **최초 작업일** | 2025-11-17 |
| **마지막 빌드** | 2026-03-10 (성공) |
| **마지막 최적화** | 2026-03-10 |
| **상태** | 운영 중 — 이슈 #1~#10 수정 완료 ✅ |

### 메모리 사용량

> 2026-03-10 이슈 #1~#10 수정 후 빌드 결과 (실측)

| 영역 | 사용 | 전체 | 점유율 | 상태 |
|------|------|------|--------|------|
| Flash (Application) | 117,492 bytes | 262,144 bytes | 50.1% | ✅ 양호 |
| **SRAM (현재)** | **22,644 bytes** | **32,768 bytes** | **69.1%** | ✅ 안정 |
| SRAM (최적화 전) | 31,612 bytes | 32,768 bytes | 96.5% | 🔴 위험 |
| Stack | 2,048 bytes | — | — | |
| Heap | 1,024 bytes | — | — | |

> 최적화 내역:
> - `lib/image.h` 13개 이미지 배열 `static uint16_t` → `static const uint16_t` (FLASH 배치)
> - `lib/WIFI.h` `MAX_NO_OF_ACCESS_POINT` 40 → 10 (1,800 bytes 절감)
> - 실제 절감량: **9,992 bytes** (SRAM 96.5% → 66.0%, -30.5%)

### 하드웨어 구성

| 컴포넌트 | 역할 |
|---------|------|
| `TrimPin` | 재봉기 트림 카운트 신호 입력 (봉제기 실 자르기 감지) |
| `TC_INT` | 추가 카운트 신호 입력 |
| `TC_RESET` | 물리적 RESET 버튼 |
| `BUZZER` | 알림음 출력 |
| `SPIM_LCD` | ST7789V LCD SPI (320×240) |
| `I2C_TC` | FT5x46 터치 컨트롤러 |
| `SPIM_FLASH` | W25Qxx 외부 Flash (설정 저장) |
| `UART` | 디버그 출력 (115200 bps) |
| `WIFI` (UART_WIFI) | ESP WiFi 모듈 통신 (활성화 상태) |
| `USBUART` | USB CDC 디버그 / 설정 |
| `WIFI_EN` | WiFi 모듈 활성화 제어 |
| `WIFI_RESET` | WiFi 모듈 리셋 |
| `LED1_R/G/B` | RGB LED 1 |
| `LED2_R/G/B` | RGB LED 2 |
| `LCD_Backlight` | LCD 백라이트 |
| `ADC_SAR_Seq` | 전류 센서 (현재 비활성화) |
| `RESERVED_OUT_1/2/3` | 예약 출력 핀 |
| `RTC` | 실시간 클럭 |

### 주요 기능

- **재봉 카운팅**: TrimPin ISR 기반 카운트 증가, 재봉 경과 시간 측정
- **ANDON 서버 연동**: HTTP GET → `/api/sewing.php` 자동 요청/응답
  - 서버 시간 동기화, 디바이스 등록, 작업 지시 조회
  - 재봉 카운트 / 패턴 카운트 / 가동 시간 자동 전송
- **패턴 관리**: 패턴별 목표/실적 집계, 패턴 전환 시 자동 저장
- **가동 중단 관리**: 중단 사유 등록 및 서버 동기화 (`downtime.c`)
- **불량 관리**: 불량 유형별 수량 등록 및 집계 (`defective.c`)
- **LCD 터치 UI**: 모니터링 / ANDON / SET / RESET 메뉴 체계
- **WiFi 자동 관리**: 60초 주기 신호 강도 체크, 자동 재연결
- **LED 상태 표시**: WiFi 신호 강도별 색상 변경, 점멸 지원
- **데이터 지속성**: 내부 EEPROM(카운트), 외부 Flash(서버/WiFi 설정) 이중 저장
- **USB 설정 도구 연동**: `SuntechIoTConfig_V1` C# 도구로 서버/WiFi 설정 변경
- **조건부 컴파일**: `package.h`의 `USER_PROJECT_PATTERN_SEWING_MACHINE` 매크로

### 알려진 문제점

| 심각도 | 파일 | 문제 |
|--------|------|------|
| ✅ 수정 | `andonApi.c:46` | `sprintf()` → `snprintf()` 교체 |
| ✅ 수정 | `uartJson.c` | UART 수신 버퍼 경계 검사 추가 |
| ✅ 수정 | `andonJson.c:73` | `jsmntok_t t[128]` → `t[80]` |
| ✅ 수정 | `server.h` | 현장 고정값 우선 폴백, host[50] 통합 |
| ✅ 수정 | `lib/WIFI.c` | WiFi 명령 3s 타임아웃 추가 |
| ✅ 수정 | `andonApi.h:118` | API URL → server.h DEFAULT_API_ENDPOINT |
| ✅ 수정 | `lib/externalFlash.c` | `checkSum()` i=1 → i=0 수정 |
| ✅ 수정 | 전체 | 전역 변수 19개 → static 전환, volatile 2개 추가 |
| ✅ 수정 | `downtime.c`, `defective.c` | JSON 파싱 로직 `parseGenericList()`로 통합 |
| **낮음** | `userMenuPatternSewingMachine.c` | 단일 파일 ~37,000 라인 |

### 변경 이력

#### 2026-03-10 (이슈 #14: WiFi MIB 타임아웃 반복 + 미연결 아이콘 버그 수정)
- **`lib/WIFI.c` [수정 1]** MIB 타임아웃 시 WifiStrength 타이머 리셋 (`wifiLoop()`)
  - 문제: CMD 2 timeout → IDLE 후 즉시 다음 루프에서 MIB 재전송 → 무한 반복
  - 해결: `g_wifi_cmd == WIFI_CMD_RECEIVED_STRENGTH` 타임아웃 시 `resetCounter_1ms(g_index_WifiStrength)` 추가 → 다음 체크 60초 후로 연기
- **`lib/WIFI.c` [수정 2]** IDLE 상태에서 늦게 도착한 MIB 응답 처리 (`wifi_get_response()`)
  - 문제: MIB 타임아웃 후 `g_wifi_cmd=0(IDLE)` 상태에서 MIB 응답 도착 → switch 케이스 없어 무시 → `DrawWifi()` 미호출 → `g_network.RSSI = INT16_MIN` 유지 → WiFi 연결됐음에도 미연결 아이콘 표시
  - 해결: switch 진입 전 IDLE 상태 MIB 응답 선처리 블록 추가 → RSSI 갱신 및 `DrawWifi()` 호출

#### 2026-03-10 (이슈 #13: 서버 경로 대소문자 수정 + API 초기화 순서 변경)
- **`lib/server.h`** `DEFAULT_SERVER_HOST` 경로 대소문자 수정
  - `"49.247.26.228/ctp280_api"` → `"49.247.26.228/CTP280_API"`
  - 원인: Linux 서버는 대소문자 구분 — 소문자 경로로 HTML 404 반환, JSON 파싱 실패
- **`andonApi.c`** `initAndon()` API 요청 순서 변경
  - 변경 전: `get_dateTime → get_downtimeList → get_defectiveList → start → get_andonList`
  - 변경 후: `get_dateTime → start → get_andonList → get_downtimeList → get_defectiveList`
  - 이유: `start` 응답에서 `target`, `req_interval` 등 핵심 설정을 먼저 수신해야 이후 동작 정상화

#### 2026-03-10 (이슈 #11: userMenuPatternSewingMachine.c 코드 정리)
- README 오기 수정: ~37,000 라인 → 실측 931 라인
- 중복 `#include "count.h"` 제거 (28번 줄)
- 주석 처리된 구버전 `doTargetInfoMenu` 130줄 완전 제거
- `doTargetInfoMenu` 빈 switch 블록 제거 — `doIncreseNumberMenu` 직접 호출로 단순화
- `doActualInfoMenu` 주석 처리된 `strTitle` 제거
- 정리 후: 931 → **763 라인** (-168줄)

#### 2026-03-10 (이슈 #10: downtime/defective JSON 파싱 중복 제거)
- `jsonUtil.h` `GENERIC_LIST_ITEM`, `GENERIC_LISTS` 공통 구조체 추가 (MAX_GENERIC_LIST=20)
- `jsonUtil.c` `parseGenericList(jsonString, sizeOfJson, lists, idxKey, nameKey)` 공통 함수 추가
- `downtime.c` `downTimeParsing()` → `parseGenericList(..., "downtime_idx", "downtime_name")` 위임 (120줄 → 4줄)
- `defective.c` `defectiveParsing()` → `parseGenericList(..., "defective_idx", "defective_name")` 위임 (120줄 → 4줄)
- Flash ~2KB 절감, 유지보수 단일 지점으로 통합

#### 2026-03-10 (빌드 에러 수정: USBJsonConfig.h extern 선언 충돌)
- `USBJsonConfig.h:34` `extern CONFIG_META g_ConfigMeta;` 제거
  - 원인: `.c`에서 `static` 정의 후 헤더의 non-static extern 선언 포함 → GCC 에러
  - `g_ConfigMeta`는 외부 파일에서 참조하지 않으므로 extern 불필요
  - `USBJsonConfig.c`의 `static CONFIG_META g_ConfigMeta;` 유지

#### 2026-03-10 (이슈 #9: 전역 변수 범위 제한 + volatile)
- **중복 선언 버그 수정**: `andonApi.c` `g_uAndonRequestType` 2중 선언 → 1개 `static`으로 통합
- **파일 범위 static 전환** (19개 변수):
  - `andonApi.c`: `g_uAndonRequestType` → static
  - `andonMessageQueue.c`: `g_uSizeQueueANDON`, `g_uFrontQueueANDON`, `g_uRearQueueANDON`, `g_cQueue[]` → static
  - `count.c`: `g_updateTrimCount`, `g_uWorkingTimeCount`, `g_Count` → static
  - `currentSensor.c`: `g_bUpdateRuntime`, `g_updateTimeTime` → static
  - `WarningLight.c`: `g_warning` → static
  - `uartJson.c`: `g_UART_buff[]`, `g_UART_buff_index` → static
  - `USBJsonConfig.c`: `g_usbCmd[]`, `g_ConfigMeta` → static
  - `lib/externalFlash.c`: `g_uSectorForConfig`, `g_uAddressForConfig` → static
  - `lib/sysTick.c`: `g_uNoMiliSecondCounter`, `g_timerCounter_1ms[]` → static
  - `lib/internalFlash.c`: `eepromReturnValue` → static
- **ISR 공유 변수 volatile 추가**:
  - `count.c/h`: `g_bStartTrimPin`, `g_bTrimElapsedTime` → `volatile` 키워드 추가

#### 2026-03-10 (이슈 #5 ~ #8: 서버 설정 재설계 + WiFi 타임아웃 + API 엔드포인트)
- `server.h` `SERVER_INFO` 구조체 `IP[16]+path[32]` → `host[50]` 통합
- `server.h` 현장 고정값 우선 적용 패턴: `DEFAULT_*` 비어있으면 외부 플래시 사용
- `server.h` `DEFAULT_API_ENDPOINT` 추가 (andonApi.h에서 이동)
- `WIFI.c` `wifi_cmd_http()` host 파싱 로직 추가
- `WIFI.c` `wifi_cmd()` 모든 명령 3s 타임아웃, `wifiLoop()` 감시 루프 추가

#### 2026-03-09 (버그 수정: 버퍼 오버플로우 3건 + CRC 버그)
- `andonApi.c:46` `sprintf` → `snprintf(url, sizeof(url), ...)` — URL 버퍼 오버플로우 방지
- `uartJson.c:42` UART 버퍼 경계 검사 추가 (`UART_BUFFER_SIZE - 1` 초과 시 리셋)
- `lib/externalFlash.c` `checkSum()` `for(i=1;...)` → `for(i=0;...)` — 첫 바이트 CRC 누락 수정

#### 2026-03-09 (버그 수정: JSMN 토큰 배열 스택 오버플로우)
- `andonJson.c`(8곳), `defective.c`, `downtime.c`(2곳), `uartJson.c`, `USBJsonConfig.c` — 총 13곳
  - `jsmntok_t t[128]` → `t[80]` (스택 사용: 2,048 → 1,280 bytes, 62.5%)
  - 이유: 128×16=2,048 bytes = 스택 전체 소진, 여유 0 bytes → 락업 위험
  - 실측 최대 토큰: 73개 (10항목 불량 리스트), 80으로 여유 7개 확보

#### 2026-03-09 (버그 수정: WiFi 수신 버퍼)
- `lib/WIFI.h` `MAX_WIFI_RECEIVE_BUFFER` 1024 → **2048** (SRAM +1,024 bytes)
  - 10개 항목 불량 리스트 JSON(~1,087 bytes)이 기존 버퍼(1,024 bytes) 초과 → 데이터 잘림 버그
  - `lib/config.h` 중복 `#define MAX_WIFI_RECEIVE_BUFFER 2048` 제거 → 주석으로 대체
- 빌드 후 SRAM: ~22,644 bytes (~69.1%) 예상

#### 2026-03-09 (1단계 SRAM 최적화 적용)
- `lib/image.h` 13개 이미지 배열 `static uint16_t` → `static const uint16_t` (FLASH 재배치)
  - `image_wifi_0/1/2/3/4`, `image_suntech`, `image_danger`
  - `image_arrow_left/right/up/down`, `image_arrow_up2/down2`
- `lib/WIFI.h` `MAX_NO_OF_ACCESS_POINT` 40 → 10 (1,800 bytes 절감)
- 실측 SRAM: 31,612 → **21,620 bytes** (96.5% → 66.0%, -9,992 bytes)
- CLAUDE.md, README.md, VERSION_HISTORY.md 초기 문서 작성

#### 2025-11-17 (최초 빌드)
- `IoT_SCI_2025.07.23 Rev.9.8.3` 기반으로 통합 버전 구성
- ARM GCC 5.4.1 빌드 성공
- 펌웨어 버전: `Integrated REV 9.8.3` (package.h)
- 빌드 결과: Flash 131,592/262,144 (50.2%), SRAM 31,612/32,768 (96.5%)

---

*Copyright SUNTECH, 2023-2026*
