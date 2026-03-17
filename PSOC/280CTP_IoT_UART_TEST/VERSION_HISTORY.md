# 280CTP_IoT_UART_TEST 펌웨어 버전 히스토리

> 대상 MCU: Cypress PSoC 4 (CY8C42xx 시리즈, Cortex-M0)
> 개발툴: PSoC Creator 4.x / ARM GCC 5.4.1
> 역할: UART 통신 테스트 — 외부 MCU 데이터 수신 → LCD 표시 + USB 출력

---

## 280CTP_IoT_UART_TEST_V1

| 항목 | 내용 |
|------|------|
| **버전 식별자** | `UART_TEST V1` |
| **프로젝트 폴더** | `280CTP_IoT_UART_TEST_V1/` |
| **워크스페이스** | `Project/Design.cydsn/` |
| **기반 버전** | `280CTP_IoT_INTEGRATED_V1` (IoT_SCI_2025.07.23 Rev.9.8.3 기반) |
| **작업 시작일** | 2026-03-12 |
| **마지막 빌드** | 2026-03-13 |
| **상태** | 기능 구현 완료 |

### 메모리 사용량

> 2026-03-13 최종 빌드 기준 (폰트 정리 + Font8x16 적용 후)

| 영역 | 사용량 | 비율 | 목표 | 상태 |
|------|--------|------|------|------|
| **Flash** | 56,228 / 262,144 bytes | **21.4%** | 60% 이하 | ✅ |
| **SRAM** | 11,004 / 32,768 bytes | **33.6%** | 70% 이하 | ✅ |
| Stack | 2,048 bytes | — | — | — |
| Heap | 1,024 bytes | — | — | — |

> Flash 상세: Bootloader 13,568 B + Application 42,404 B + Metadata 256 B = 56,228 B

#### SRAM 최적화 이력

| 시점 | SRAM | 비율 | 내용 |
|------|------|------|------|
| 초기 (기반 코드) | 17,524 B | 53.5% | 버퍼 2048 × 6 |
| 최적화 완료 | 11,004 B | **33.6%** | 버퍼 1024 × 6, 불필요 코드 제거 |

### 하드웨어 구성

| 컴포넌트 | 역할 | 상태 |
|---------|------|------|
| `UART` | 외부 MCU 데이터 수신 (폴링) | ✅ 활성 |
| `USBUART` | PC로 수신 데이터 전송 (CDC) | ✅ 활성 |
| `SPIM_LCD` | ST7789V LCD (320×240) | ✅ 활성 |
| `I2C_TC` | FT5x46 터치 컨트롤러 (스크롤 버튼) | ✅ 활성 |
| `LCD_Backlight` | LCD 백라이트 | ✅ 활성 |
| `LED1_R/G/B` | 상태 표시 LED | ✅ 활성 |
| `LED2_R/G/B` | 상태 표시 LED | ✅ 활성 |
| `WIFI` (UART_WIFI) | WiFi 모듈 | **비활성** |
| `TrimPin` / `TC_INT` | 재봉기 카운트 | **미사용** |
| `ADC_SAR_Seq` | 전류 센서 | **미사용** |

### 주요 기능

- **UART 수신**: ✅ — `uartJson.c` 폴링 방식, '\n' 기준 한 줄 확정, 링 버퍼 5줄 보관
- **LCD 뷰어**: ✅ — 헤더/바디/푸터 레이아웃, Font8x16, 8줄 표시, 수신 중 라이브 녹색 표시
- **스크롤**: ✅ — UP/DOWN/CLEAR 버튼, 자동 스크롤(최신 줄), 수동 스크롤 시 자동 스크롤 OFF
- **USB 출력**: ✅ — `printf_USB()` 로 수신 완료 라인을 USB CDC 전송
- **LED 상태**: ✅ — 1초 주기 RED 점멸 (수신 대기 표시)

### 폰트 현황

| 폰트 | 크기 | 상태 |
|------|------|------|
| `SmallFont8x12` | 1,144 B | ✅ 유지 (보조) |
| `Font8x16` | 1,524 B | ✅ **신규 추가** — UART 뷰어 전체 적용 |
| `AlibriNumBold32x48` | 1,924 B | 제거 |
| `Arial_round_16x24` | 4,564 B | 제거 |
| `DotMatrix_M_16x22` | ~2,700 B | 제거 |
| `Grotesk16x32` | ~2,700 B | 제거 |
| `ArialNumFontPlus32x50` | ~500 B | 제거 |
| `SevenSegNumFontPlusPlus32x50` | 2,604 B | 제거 |

### 변경 이력

#### 2026-03-14 (기능 추가 — WHITE 표시 전환 + 타임아웃 flush + 부저 피드백)

**`uartJson.c` 변경사항**

- `#include "lib/LEDControl.h"` 추가

- **모든 데이터 WHITE 표시**
  - 오버플로우 시 `saveRecord(FALSE)` (RED) → `saveRecord(TRUE)` (WHITE) 로 변경
  - 이제 `\n`/`\r` 수신, 오버플로우, 타임아웃 flush 세 가지 모두 WHITE 표시

- **타임아웃 flush 신규 구현** — `\n` 없는 데이터도 LCD 표시 가능
  ```c
  #define UART_IDLE_FLUSH_COUNT  200u   // 200 루프 무수신 시 미완성 버퍼를 WHITE로 확정
  ```
  - `uartJsonLoop()` while 루프 종료 후 idle 카운터 증가
  - 수신 있으면 카운터 리셋, 200 루프 도달 시 현재 버퍼를 WHITE로 `saveRecord(TRUE)` 강제 커밋

- **터치 버튼 부저 피드백 신규 추가**
  ```c
  Buzzer(BUZZER_CLICK, 0);  // UP / DOWN / CLEAR 버튼 터치 시 ~50ms 비프
  ```
  - 푸터 영역(하단 40px) 터치 확인 후에만 울림 (화면 다른 곳 터치 시 무반응)

**`main.c` 변경사항**

- **LED1 색상/동작 변경**
  - 기존: `LED_RED` + 깜박임 (1초 주기 점멸, heartbeat)
  - 변경: `LED_GREEN` + 상시점등 (전원 인디케이터)
  ```c
  // g_uLED1_Color = LED_RED;
  g_uLED1_Color = LED_GREEN;
  // g_bLED1_Flickering = TRUE;   ← 주석 처리 → 상시점등
  ```

**참고 — LED2 (미적용)**
- `uartJson.c` 내 LED2 Green 깜박임 코드 추가됐으나 현재 주석 처리 상태
- 필요 시 주석 해제하여 UART RX 수신 인디케이터로 활성화 가능
  ```c
  // g_uLED2_Color      = LED_GREEN;
  // g_bLED2_Flickering = TRUE;
  ```

---

#### 2026-03-13 (최적화 + 기능 완성)

**SRAM 최적화**
- `uartJson.c` — `UART_BUFFER_SIZE` 2048 → **1024** (임시 수신 버퍼)
- `uartJson.c` — `UART_VIEW_REC_LEN` 2048 → **1024** (링 버퍼 레코드 크기)
- SRAM 절감: 6,144 bytes (53.5% → 33.6%)

**불필요 파일 제거 (Flash + 빌드 정리)**
- `lib/button.c` / `lib/button.h` — 미사용 UI 위젯, 삭제
- `lib/widget.c` / `lib/widget.h` — 미사용 UI 위젯, 삭제
- `USBUARTConfig.c` — .cyprj 미포함 + 미사용, 삭제
- `Design.cyprj` — 위 파일 참조 블록 제거

**폰트 교체 및 정리**
- `lib/fonts.h` — `Font8x16` (IBM VGA 8×16, 1,524 bytes) **신규 추가**
- `lib/fonts.h` — 미사용 폰트 6종 제거 (~14,992 bytes Flash 절감)
- `uartJson.c` — `SmallFont8x12` → `Font8x16` 전체 교체 (5곳)
- `uartJson.c` — `UART_VIEW_LINE_H` 14 → **18** (폰트 높이 16 + 여백 2)

**LCD 뷰어 구현 완성** (`uartJson.c`)
- `uartJsonLoop()` — UART 폴링 수신, '\n'/'\r' 기준 한 줄 확정, 링 버퍼 저장
- `uartJsonDrawScreen()` — 헤더/바디/푸터 레이아웃, 수신 라인 렌더링, 라이브 라인 녹색 표시
- `uartJsonHandleTouch()` — UP/DOWN/CLEAR 버튼 터치 처리, 자동 스크롤 제어
- `printf_USB()` — 수신 라인을 USB CDC로 실시간 전송

#### 2026-03-12 (V1 초기 구성)

- `280CTP_IoT_INTEGRATED_V1` 프로젝트 전체 복사
- `setup.c` WiFi 관련 코드 주석 처리 (`UART_WIFI_Start`, `WIFI_EN_Write`, `WIFI_Init`)
- `main.c` 불필요한 루프 제거 (WiFi, ANDON, 전류센서, TrimPin, 카운팅 등)
- `uartJson.c` / `uartJson.h` 신규 작성 (UART 수신 + LCD 뷰어 기본 구조)
- `userTimer.c` / `userTimer.h` 신규 작성 (1ms / 1s 타이머)
- CLAUDE.md, README.md, VERSION_HISTORY.md 초기 문서 작성

---

*Copyright SUNTECH, 2023-2026*
