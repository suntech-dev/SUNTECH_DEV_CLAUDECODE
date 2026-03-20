# IoT INTEGRATED 펌웨어 버전 히스토리

> 대상 MCU: Cypress PSoC 4 (CY8C42xx 시리즈, Cortex-M0)
> 개발툴: PSoC Creator 4.x / ARM GCC 5.4.1
> 역할: 재봉기 IoT 통합 모니터링 — ANDON 연동, 생산 카운팅, 가동/불량 관리

---

## 280CTP_IoT_INTEGRATED_V2_BLACK_CPU

| 항목 | 내용 |
|------|------|
| **버전 식별자** | `BLACK_CPU V2` |
| **프로젝트 폴더** | `280CTP_IoT_INTEGRATED_V2_BLACK_CPU/` |
| **워크스페이스** | `Project/Design.cydsn/` |
| **기반 버전** | `280CTP_IoT_INTEGRATED_V1_BLACK_CPU` (V1_BLACK_CPU 복사본) |
| **개발 목적** | OTA(Over-The-Air) 무선 펌웨어 업데이트 기능 추가 |
| **작업 시작일** | 2026-03-19 |
| **코드 수정 완료** | 2026-03-19 ✅ |
| **상태** | ✅ 빌드 완료 — OTA 2-Stage 자동 체크 + 수동 메뉴 구현 |

### 메모리 사용량

> 2026-03-20 빌드 결과 (PSoC Creator 실측 — bootloader Build 후 Design Build)
> Flash Row Size: **256 bytes** / Bootloadable Placement Address: **0x4200** (row 66)

**Bootloader 단독**

| 영역 | 사용 | 전체 | 점유율 | 상태 |
|------|------|------|--------|------|
| Flash | **16,749 bytes** | 262,144 bytes | **6.4%** | ✅ |
| Flash (Application) | 16,493 bytes | — | — | |
| Flash (Metadata) | 256 bytes | — | — | |
| SRAM | **1,956 bytes** | 32,768 bytes | **6.0%** | ✅ |
| Stack | 1,024 bytes | — | — | |
| Heap | 128 bytes | — | — | |

**Design (Application) — PSoC Creator 기준 전체 표시**

| 영역 | 사용 | 전체 | 점유율 | 상태 |
|------|------|------|--------|------|
| Flash | **134,408 bytes** | 262,144 bytes | **51.3%** | ✅ 양호 |
| Flash (Bootloader 영역) | 16,640 bytes | — | 6.4% | |
| Flash (Application) | **117,512 bytes** | — | **44.8%** | |
| Flash (Metadata) | 256 bytes | — | 0.1% | |
| **SRAM** | **22,972 bytes** | **32,768 bytes** | **70.1%** | ✅ 안정 |
| Stack | 2,048 bytes | — | — | |
| Heap | 1,024 bytes | — | — | |

### V1_BLACK_CPU 대비 변경 사항

| 항목 | V1_BLACK_CPU | V2_BLACK_CPU (OTA 추가) |
|---|---|---|
| `PROJECT_FIRMWARE_VERSION` | `"BLACK_CPU V1"` | `"BLACK_CPU V2"` |
| OTA 수동 업데이트 메뉴 | ❌ 없음 | ✅ `otaMenu.c/h` 추가 |
| OTA 자동 버전 체크 (2-Stage) | ❌ 없음 | ✅ 부팅 후 20초 → 24시간 주기 |
| `DEFAULT_OTA_API_PATH` | ❌ 없음 | ✅ `server.h`에 `/CTP280_OTA/CTP280_OTA_V1/api` 추가 |
| `WIFI_CMD_OTA_VERSION` | ❌ 없음 | ✅ `lib/WIFI.c` 케이스 추가 |
| `WIFI_CMD_OTA_AUTO` | ❌ 없음 | ✅ `lib/WIFI.c` 케이스 추가 |
| `WIFI_CMD_OTA_CHUNK` | ❌ 없음 | ✅ `lib/WIFI.c` 케이스 추가 |
| `_wifi_send_httpget_ota()` | ❌ 없음 | ✅ OTA 전용 HTTP GET (pathPart 무시, IP만 추출) |
| `otaDrawUpdateBadge()` | ❌ 없음 | ✅ `DrawHeader()`에서 호출 — 업데이트 배지 표시 |
| `otaAutoCheckInit()` | ❌ 없음 | ✅ `initAndon()`에서 호출 — 20초 카운트다운 시작 |
| `otaAutoCheckLoop()` | ❌ 없음 | ✅ `wifiLoop()`에서 호출 — 타이머 감시 + 요청 트리거 |
| W25QXX Flash Sector 30 | 미사용 | ✅ OTA 제어 블록 (`OTA_FLAG_BLOCK`) 저장 |
| W25QXX Flash Sector 32~ | 미사용 | ✅ OTA 펌웨어 데이터 저장 |

### 하드웨어 구성

V1_BLACK_CPU와 동일하며 아래 항목 추가 활용:

| 컴포넌트 | 상태 | 비고 |
|---------|------|------|
| `SPIM_FLASH` (W25Qxx) | ✅ 확장 사용 | Sector 30 (OTA 플래그), Sector 32~ (펌웨어 데이터) |
| `WIFI` (UART_WIFI) | ✅ 사용 확장 | OTA HTTP GET 명령 3종 추가 |
| `LCD` (ST7789V) | ✅ 사용 확장 | OTA 메뉴 화면 + 헤더 업데이트 배지 |

### 주요 기능

V1_BLACK_CPU의 모든 기능 포함 + 아래 OTA 기능 추가:

- **OTA 2-Stage 자동 버전 체크**:
  - Stage 1: 부팅 후 20초 딜레이 (`OTA_AUTO_INIT_DELAY_MS`) → ANDON 초기화 완료 대기 후 자동 요청
  - Stage 2: 응답 수신 후 24시간 주기 (`OTA_AUTO_PERIOD_MS`) 반복 체크
  - 신버전 감지 시 `g_otaUpdateAvailable = TRUE` → LCD 헤더에 배지 아이콘 표시
- **OTA 수동 업데이트 메뉴**: `doOtaUpdate()` — 버전 확인 → 다운로드 → Flash 쓰기 → 재부팅
- **OTA 전용 URL 처리**: `_wifi_send_httpget_ota()` — host의 pathPart(ANDON 경로) 무시하고 IP만 추출하여 OTA 절대경로 직접 사용
- **OTA API 엔드포인트**: `DEFAULT_OTA_API_PATH` = `/CTP280_OTA/CTP280_OTA_V1/api`
  - 버전 확인: `/version.php`
  - 청크 다운로드: `/firmware.php?offset={N}&size={400}`
  - 완료 보고: `/status.php?mac={MAC}&status=done&version={VER}`
- **OTA 청크 크기**: `OTA_CHUNK_SIZE = 400` bytes (hex 800자 + JSON 오버헤드 < 2048 버퍼)
- **OTA Flash 구조**: Sector 30 = `OTA_FLAG_BLOCK` (36 bytes), Sector 32~ = 펌웨어 바이너리

### 수정된 파일 목록

| 파일 | 수정 내용 |
|------|---------|
| `otaMenu.c` | 신규 추가 — OTA 수동 메뉴 + 자동 체크 상태 머신 전체 구현 |
| `otaMenu.h` | 신규 추가 — OTA 상수, 구조체, 상태 enum, 공개 API 선언 |
| `lib/WIFI.c` | `WIFI_CMD_OTA_VERSION/AUTO/CHUNK` 케이스 추가, `_wifi_send_httpget_ota()` 추가, `otaAutoCheckLoop()` 호출 추가, `resetCounter_1ms()` 추가 (수정 4) |
| `lib/widget.c` | `DrawHeader()`에 `otaDrawUpdateBadge()` 호출 추가 |
| `lib/widget.h` | `IDX_SCROLL_UP = 0xFD`, `IDX_SCROLL_DOWN = 0xFE` — child index 충돌 방지 (수정 3) |
| `lib/server.h` | `DEFAULT_OTA_API_PATH "/CTP280_OTA/CTP280_OTA_V1/api"` 추가 |
| `lib/manageMenu.c` | `manageMenuCreate()` — `otaUpdate` 노드를 root 마지막 자식(index 6)으로 이동 |
| `Design.cyprj` | `otaMenu.c` (SOURCE_C), `otaMenu.h` (HEADER) 프로젝트 등록 |
| `bootloader Debug/` | V1_BLACK_CPU에서 `bootloader.hex`(578KB), `bootloader.elf`(765KB) 복사 |
| `bootloader.cydsn` TopDesign | **SPIM_FLASH** (SPI Master v2.50) 컴포넌트 추가 |
| `bootloader.cydsn` TopDesign | **Ctrl_MEM_SS** (Digital Output Pin) 컴포넌트 추가 |
| `bootloader.cyprj` | `w25qxx.c` 소스 등록, `Additional Include Directories = ..\Design.cydsn\lib` 추가 |
| `bootloader.cydsn/main.c` | `CyFlash_WriteRow` → `CySysFlashWriteRow` (PSoC4 올바른 API) |
| `bootloader.cydsn/main.c` | `BOOTLOADABLE_BASE_ROW` = `66u` (0x4200 / 256, Flash Row = 256 bytes) |
| `Design.cydsn` TopDesign | Bootloadable **Placement address** `0x4000` → `0x4200` |

### 빌드 주의사항

- **⚠ 빌드 순서 필수**: `bootloader` 먼저 Build → `Design` Build 순서 지켜야 함. **"Clean and Build All"은 절대 사용 금지** — 프로젝트 순서가 Design 먼저라 bootloader.hex/elf 없는 상태에서 Design 빌드 시도하여 실패
- **Flash Row Size**: 이 PSoC4 디바이스의 Flash Row = **256 bytes** (PSoC4200M 기준). Bootloadable Placement Address와 BOOTLOADABLE_BASE_ROW는 반드시 256의 배수로 설정
- **Bootloadable Placement address**: V2 bootloader에 SPIM_FLASH+w25qxx 추가로 bootloader가 16KB 초과 → Placement address 0x4000 → **0x4200** (row 66) 으로 상향 조정됨
- **`jsoneq()` 반환값**: `jsmn.h`의 `jsoneq()`는 일치 시 `0` 반환 — `== 0` 으로 비교할 것 (`jsonKeyMatch()`는 이 프로젝트에 없음)
- **색상 상수**: `ST7789V.h` 기준 `GREY` → `LIGHTGREY`, `DARK_GREY` → `DARKGREY`
- **OTA HTTP GET**: `_wifi_send_httpget()` 사용 금지 — pathPart 자동 추가로 URL 오류 발생. 반드시 `_wifi_send_httpget_ota()` 사용

### 변경 이력

#### 2026-03-20 (V2_BLACK_CPU — OTA 부트로더 완성 + 빌드 성공)

**빌드 성공**: Flash 98,244 bytes (37.5%) / SRAM 19,704 bytes (60.1%)
> ⚠ 이전에 기록된 "46.8% / 68.8%"는 실수로 V1_BLACK_CPU를 빌드한 잘못된 수치였음

**[수정] V2 OTA 부트로더 미완성 문제** — 4단계 순차 해결:

1. **`w25qxx.h` include 경로 누락** (`bootloader.cyprj`)
   - `Additional Include Directories` = `..\Design.cydsn\lib` 추가
   - PSoC Creator는 외부 경로의 소스 파일 디렉토리를 include 경로에 자동 추가하지 않음

2. **`CyFlash_WriteRow` 미정의** (`bootloader/main.c:113`)
   - PSoC4 정확한 API: `CySysFlashWriteRow()` (PSoC 3/5용 `CyFlash_WriteRow`는 존재하지 않음)
   - 수정: `CyFlash_WriteRow(...)` → `CySysFlashWriteRow(...)`

3. **Bootloadable Placement address 충돌** (Design.cydsn TopDesign)
   - V2 bootloader에 SPIM_FLASH + w25qxx 추가로 bootloader가 row 64 (0x4000-0x40FF) 침범
   - Placement address `0x4000` → `0x4200` 으로 변경 (row 66 = 두 row 마진 확보)

4. **`BOOTLOADABLE_BASE_ROW` 오류** (`bootloader/main.c`)
   - Flash Row = **256 bytes** (PSoC4200M) 이므로: `0x4200 / 256 = 66`
   - 최초 설정값 `128u` → `66u` 로 수정

**[수정] bootloader.hex/elf 누락** (`bootloader.cydsn/CortexM0/ARM_GCC_541/Debug/`)
- **증상**: `Clean and Build all projects` 시 `Bootloadable. The referenced Bootloader is invalid. bootloader.hex/elf: The path does not exist`
- **근본 원인**: V2 bootloader의 `main.c`가 `w25qxx.h`를 포함하나 프로젝트에 미등록 → bootloader 빌드 실패 → hex/elf 미생성
- **전체 해결**: 위 4단계 수정 후 bootloader 빌드 성공 → Design 빌드 성공

#### 2026-03-20 (V2_BLACK_CPU — OTA 버전 확인 즉시 타임아웃 버그 수정)

**[수정 4] `setCountMax_1ms` 직후 `isFinishCounter_1ms` 즉시 TRUE 반환 버그** (`lib/WIFI.c`)

- **증상**: MENU → OTA UPDATE 진입 시 서버 응답 대기 없이 즉시 "OTA Error Bad version data" 표시. 시리얼 로그에서 `[WIFI] CMD 9 timeout -> IDLE` 이 HTTP 응답(`*ICT*HTTPGET:OK`) 보다 먼저 출력됨
- **원인 — sysTick.c 구현 특성**:
  ```c
  // sysTick.c setCountMax_1ms 구현
  void setCountMax_1ms(uint16 index, uint32 maxCount)
  {
      g_timerCounter_1ms[index].current = 0;   // ← 즉시 0으로 설정
      g_timerCounter_1ms[index].max     = maxCount;
  }
  // isFinishCounter_1ms: current==0 → TRUE 반환
  ```
  `setCountMax_1ms`는 `current=0`으로 설정. SysTick ISR이 `current==0`을 감지해 `max`로 재로드하기 전에 메인루프가 `wifiLoop()`를 실행하면 `isFinishCounter_1ms()==TRUE` → 즉시 타임아웃 발동
- **실행 흐름 (버그)**:
  ```
  MenuLoop() → requestVersion() → wifi_cmd_ota_version() → setCountMax(15000) → current=0
  ↓ (1ms ISR 아직 미실행 — 메인루프가 ISR보다 빠름)
  wifiLoop() → isFinishCounter(g_index_Wifi_Test): current==0 → TRUE
  → "[WIFI] CMD 9 timeout → IDLE" → otaHandleVersionResponse("{}", 2u)
  → r=1 (빈 객체 {}), 루프 미실행 → g_otaLatestVersion[0]=='\0' → "Bad version data"
  ↓ (이후 실제 HTTP 응답 도착하지만 g_wifi_cmd=IDLE → 무시)
  ```
- **수정**: `_wifi_send_httpget()`, `_wifi_send_httpget_ota()`, `wifi_cmd()` 세 함수에 `resetCounter_1ms(g_index_Wifi_Test)` 추가
  ```c
  /* 수정 전 */
  g_wifi_cmd = cmd;
  setCountMax_1ms(g_index_Wifi_Test, timeoutMs);

  /* 수정 후 */
  g_wifi_cmd = cmd;
  setCountMax_1ms(g_index_Wifi_Test, timeoutMs);
  resetCounter_1ms(g_index_Wifi_Test);  // current=max 보장 (current=0 즉시발동 방지)
  ```
- **근거**: `resetCounter_1ms`는 `current=max`로 즉시 설정 → ISR 실행 여부와 무관하게 타이머가 정상적으로 카운트다운 시작. `wifiConnectAP()` case 1에서 이미 이 패턴 사용 (`wifi_cmd(GET_MAC)` 후 `resetCounter_1ms()` 명시적 호출)

---

#### 2026-03-20 (V2_BLACK_CPU — OTA UPDATE 메뉴 네비게이션 버그 수정)

**[수정 3] OTA UPDATE 터치 시 상위 페이지로 스크롤되는 버그** (`lib/widget.h`, `lib/manageMenu.c`)

- **증상**: LCD → MENU → 스크롤 → OTA UPDATE 터치 시 OTA 화면에 진입하지 않고 리스트가 페이지 0으로 스크롤되어 SETTINGS 항목이 보이는 화면으로 돌아감 (사용자 인식: "SETTING으로 돌아간다")
- **원인**: `widget.h` enum의 `IDX_SCROLL_UP = 6` 값이 `manageMenuCreate()` root 메뉴의 OTA UPDATE child index(6)와 충돌. `doListMenuPage()` switch 문에서 index 6을 `case IDX_SCROLL_UP`으로 매칭하여 `curPage--` (페이지 스크롤 업) 실행
- **수정**: `IDX_SCROLL_UP = 0xFD`, `IDX_SCROLL_DOWN = 0xFE`로 변경 — child index 최대값(~14)과 `NO_CLICK(0xFF)` 사이 안전한 값으로 이동
  ```c
  /* 수정 전 */
  IDX_SCROLL_UP,    /* = 6 */
  IDX_SCROLL_DOWN   /* = 7 */

  /* 수정 후 */
  IDX_SCROLL_UP   = 0xFD,   /* child index(최대 ~24)와 NO_CLICK(0xFF) 충돌 방지 */
  IDX_SCROLL_DOWN = 0xFE
  ```
- **관련**: `manageMenuCreate()` — `otaUpdate` 노드를 root 마지막 자식(index 6)으로 이동. 이전 위치에서는 index 6과 충돌이 없었으나, 이동 후 충돌 발생하여 잠재적 설계 결함이 노출됨
- **근본 원인 분석**: `getIndexOfClickedListMenu()`가 리스트 아이템 클릭 시 절대 child index를 반환하고, 스크롤 버튼 클릭 시 `IDX_SCROLL_UP/DOWN` enum 값을 반환하는 설계에서, 두 값 공간이 겹칠 경우 오동작 발생. `NO_CLICK = 0xFF` 기준으로 스크롤 상수를 고정값으로 이동하여 해결

---

#### 2026-03-20 (V2_BLACK_CPU — OTA 관련 버그 수정 2건)

**[수정 1] WIFI INFO 정보 미표시 버그** (`lib/WIFI.c`)
- **증상**: LCD → MENU → WIFI INFO 화면에 RSSI, IP 등 정보가 표시되지 않음
- **원인**: `wifi_get_response()`에서 OTA HTTP 응답(`WIFI_CMD_OTA_VERSION/AUTO/CHUNK`)도 `andonResponse()`로 전달되어 ANDON 상태 오염 → `andonLoop()` 오작동 → RSSI 체크 블록 미실행 → `g_network.RSSI = INT16_MIN` 유지
- **수정**: `andonResponse()` 호출 조건에 `g_wifi_cmd == WIFI_CMD_HTTP` 가드 추가
  ```c
  /* 수정 전 */
  if(g_sizeOfHttpText > 0)
  { andonResponse(...); g_sizeOfHttpText = 0; }

  /* 수정 후 */
  if(g_sizeOfHttpText > 0 && g_wifi_cmd == WIFI_CMD_HTTP)
  { andonResponse(...); }
  g_sizeOfHttpText = 0;
  g_ptrHttpReceivedText = NULL;
  ```

**[수정 2] OTA UPDATE 터치 시 디바이스 먹통** (`otaMenu.c:210`)
- **증상**: LCD → MENU → OTA UPDATE 터치 시 디바이스 완전 응답 없음
- **원인**: `SetDrawListButtons(NULL, ...)` 호출 → ARM Cortex-M0 HardFault (`menu->noOfDisplayButton` NULL 포인터 역참조)
- **수정**: `NULL` → `&g_ListMenu` (전역 LIST_MENU 구조체 포인터 전달)
  ```c
  /* 수정 전 */
  SetDrawListButtons(NULL, thisMenu->nodeName, NULL, 0, BUTTON_STYLE_LIST);
  /* 수정 후 */
  SetDrawListButtons(&g_ListMenu, thisMenu->nodeName, NULL, 0, BUTTON_STYLE_LIST);
  ```

**[추가] OTA 웹 관리 페이지 HEX→BIN 자동 변환 기능** (`WEB/CTP280_OTA/CTP280_OTA_V1/index.html`)
- `Design.hex` 파일을 직접 업로드하면 브라우저에서 Intel HEX → BIN 자동 변환 후 서버 전송
- `arm-none-eabi-objcopy` 도구 없이 웹 페이지에서 직접 처리

#### 2026-03-19 (V2_BLACK_CPU — OTA 기능 구현 완료)
- V1_BLACK_CPU 기반으로 V2_BLACK_CPU 신규 버전 분리
- `otaMenu.c/h` 신규 작성:
  - `OTA_FLAG_BLOCK` 구조체 (W25QXX Sector 30 저장)
  - `OTA_STATE` / `OTA_AUTO_STATE` 상태 머신
  - 수동 메뉴: `initOtaMenu()`, `doOtaUpdate()`
  - 자동 체크: `otaAutoCheckInit()`, `otaAutoCheckLoop()`, `otaHandleAutoVersionResponse()`
  - 응답 콜백: `otaHandleVersionResponse()`, `otaHandleChunkResponse()`
  - 헤더 배지: `otaDrawUpdateBadge()`
- `lib/WIFI.c` 수정:
  - `_wifi_send_httpget_ota()` 추가 (host에서 IP만 추출, pathPart 무시)
  - `wifi_cmd_ota_version()`, `wifi_cmd_ota_auto()`, `wifi_cmd_ota_chunk()` 추가
  - `wifi_get_response()` switch에 `WIFI_CMD_OTA_VERSION/AUTO/CHUNK` 케이스 추가
  - `wifiLoop()` `andonLoop()==FALSE` 블록에 `otaAutoCheckLoop()` 추가
- `lib/widget.c` 수정: `DrawHeader()`에 `otaDrawUpdateBadge()` 추가
- `lib/server.h` 수정: `DEFAULT_OTA_API_PATH` 상수 추가
- `Design.cyprj` 수정: `otaMenu.c/h` XML 직접 등록
- `bootloader Debug/` 복사: V1_BLACK_CPU에서 `bootloader.hex/elf` 복사 (최초 빌드용)
- 빌드 에러 수정 이력:
  - `lib/jsonUtil.h: No such file or directory` → `jsonUtil.h`는 루트에 위치, include 경로 수정
  - `jsonKeyMatch` undeclared → `jsoneq(...) == 0` 패턴으로 교체 (6곳)
  - `GREY`, `DARK_GREY` undeclared → `LIGHTGREY`, `DARKGREY`로 교체
  - bootloader hex/elf not found → V1_BLACK_CPU에서 PowerShell Copy-Item으로 복사
  - OTA URL `/ota/` 경로 → `_wifi_send_httpget_ota()` + `DEFAULT_OTA_API_PATH` 분리 구현

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

> 2026-03-17 코드 수정 후 빌드 결과 (실측)

| 영역 | 사용 | 전체 | 점유율 | 상태 |
|------|------|------|--------|------|
| Flash (전체) | 122,812 bytes | 262,144 bytes | 46.8% | ✅ 양호 |
| Flash (Bootloader) | 13,568 bytes | — | 5.2% | |
| Flash (Application) | 108,988 bytes | — | 41.6% | |
| Flash (Metadata) | 256 bytes | — | 0.1% | |
| **SRAM** | **22,532 bytes** | **32,768 bytes** | **68.8%** | ✅ 안정 |
| Stack | 2,048 bytes | — | — | |
| Heap | 1,024 bytes | — | — | |

> V1 대비 변화: Flash -8,504 bytes (-3.3%), SRAM -112 bytes (68.8% vs 69.1%)
> ※ ADC_SAR_Seq TopDesign 제거 후 추가 절감 예상

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
