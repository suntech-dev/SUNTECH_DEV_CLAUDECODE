# 280CTP_IoT_UART_TEST 프로젝트 분석 문서

> 최초 작성: 2026-03-12
> 기반 프로젝트: 280CTP_IoT_INTEGRATED_V1
> 목적: UART 통신 테스트 전용 버전
> 마지막 업데이트: 2026-03-14

---

## 1. 프로젝트 개요

이 프로젝트는 **280CTP_IoT_INTEGRATED_V1**을 기반으로 만든 **UART 통신 테스트 전용 펌웨어**입니다.
다른 MCU와의 UART 통신이 정상적으로 작동하는지 확인하기 위한 목적으로 제작되었으며,
WiFi, ANDON, 생산 카운팅 등 운영 기능은 모두 제거합니다.

### 핵심 기능 요약

| 기능 | 설명 | 상태 |
|------|------|------|
| **UART 수신** | 외부 MCU로부터 데이터 수신 (폴링 방식) | ✅ **완료** (`uartJson.c`) |
| **LCD 뷰어** | 수신 라인을 ST7789V LCD에 표시 + 스크롤 | ✅ **완료** (`uartJsonDrawScreen`) |
| **스크롤** | UP/DOWN/CLEAR 버튼으로 수신 이력 탐색 | ✅ **완료** (`uartJsonHandleTouch`) |
| **USB 출력** | 수신 라인을 PC(USB CDC)로 동시 전송 | ✅ **완료** (`printf_USB`) |
| **LED 상태** | 수신 중 LED 점멸로 통신 상태 시각화 | ✅ **완료** |
| **WiFi** | 사용 안 함 | ✅ **비활성화 완료** |
| **ANDON/카운팅** | 사용 안 함 | ✅ **비활성화 완료** |

---

## 2. 버전 구조

```
280CTP_IoT_UART_TEST/
├── 280CTP_IoT_UART_TEST_V1/                   # V1 - UART 테스트 버전 (2026.03)
│   └── Project/
│       ├── Design.cydsn/                        # 메인 애플리케이션 펌웨어
│       │   ├── main.c / main.h                  # 메인 진입점
│       │   ├── setup.c / setup.h                # 하드웨어 초기화 (WiFi 비활성화)
│       │   ├── uartJson.c / uartJson.h          # UART 수신 + LCD 뷰어 + USB 출력 (핵심)
│       │   ├── userTimer.c / userTimer.h        # 1ms / 1s 타이머 루프
│       │   └── lib/                             # 공통 라이브러리
│       │       ├── ST7789V.c/h                  # LCD SPI 드라이버
│       │       ├── FT5x46.c/h                   # 터치 컨트롤러 (스크롤 버튼)
│       │       ├── USB.c/h                      # USB CDC (데이터 출력)
│       │       ├── UI.c/h                       # LCD 그리기 유틸리티
│       │       ├── IODefine.h / IODefine.c      # I/O 핀 매핑
│       │       ├── IOUtil.c/h                   # I/O 엣지 검출 유틸리티
│       │       ├── sysTick.c/h                  # 1ms SysTick ISR
│       │       ├── LEDControl.c/h               # RGB LED 제어
│       │       └── fonts.h / image.h            # LCD 폰트/이미지 (const 필수)
│       └── bootloader.cydsn/                    # UART 기반 부트로더
├── CLAUDE.md                                    # 코딩 규칙 문서
├── README.md                                    # 이 문서
└── VERSION_HISTORY.md                          # 버전별 변경 이력
```

---

## 3. 하드웨어 정보

| 항목 | 내용 |
|------|------|
| **MCU** | Cypress PSoC 4 (CY8C42xx 시리즈) |
| **코어** | ARM Cortex-M0 |
| **Flash** | 262,144 bytes (256KB) |
| **SRAM** | 32,768 bytes (32KB) |
| **개발툴** | PSoC Creator 4.x |
| **컴파일러** | ARM GCC 5.4.1 |
| **LCD** | ST7789V (320×240, SPI 인터페이스) |
| **터치** | FT5x46 (I2C 기반 커패시티브 터치) |

### 사용 하드웨어 컴포넌트

| 컴포넌트 | 타입 | 역할 | 상태 |
|---------|------|------|------|
| `UART` | SCB UART | 외부 MCU 데이터 수신 | **활성** |
| `USBUART` | USB CDC | PC로 수신 데이터 출력 | **활성** |
| `SPIM_LCD` | SPI Master | ST7789V LCD | **활성** |
| `I2C_TC` | I2C Master | FT5x46 터치 (스크롤 버튼) | **활성** |
| `LCD_Backlight` | GPIO 출력 | LCD 백라이트 | **활성** |
| `LED1_R/G/B` | GPIO 출력 | 상태 표시 LED | **활성** |
| `LED2_R/G/B` | GPIO 출력 | 상태 표시 LED | **활성** |
| `BUZZER` | GPIO 출력 | 알림음 | 선택적 |
| `WIFI` (UART_WIFI) | SCB UART | WiFi 모듈 | **비활성** |
| `TrimPin` | GPIO + ISR | 재봉기 트림 신호 | **미사용** |
| `TC_INT` | GPIO + ISR | 추가 카운트 입력 | **미사용** |
| `ADC_SAR_Seq` | ADC | 전류 센서 | **미사용** |

---

## 4. 소스코드 상세 분석

### 4.1 main.c 루프 구조

```c
main()
├── CySysClkWcoStart()           // WCO 클럭 시작
├── CyGlobalIntEnable            // 전역 인터럽트 활성화
├── SetUp()                      // 하드웨어 초기화
├── LCD_Backlight_Write(1)       // 백라이트 ON
├── LED1 GREEN 상시점등 (전원 인디케이터)
└── for(;;)
    ├── OneMilliSecond_MainLoop()  // 1ms 타이머 루프
    ├── OneSecond_MainLoop()       // 1s 타이머 루프
    ├── usbLoop()                  // USB CDC enumeration 처리
    ├── uartJsonLoop()             // UART 수신 + 링 버퍼 저장
    ├── [1s 이벤트]
    │   └── LED_OneSecondControl() // LED 1초 주기 제어
    ├── uartJsonDrawScreen()       // LCD 뷰어 화면 갱신
    └── uartJsonHandleTouch()      // UP/DOWN/CLEAR 터치 처리
```

### 4.2 uartJson.c — UART 수신 + LCD 뷰어 + USB 출력

이 파일이 이 프로젝트의 핵심 구현체입니다.

#### 버퍼 구조

```c
#define UART_BUFFER_SIZE   1024   // 수신 임시 버퍼 (한 줄 단위)
#define UART_VIEW_MAX_REC    5    // 링 버퍼 최대 보관 라인 수
#define UART_VIEW_REC_LEN  1024   // 라인당 최대 저장 문자 수

static char g_UART_buff[UART_BUFFER_SIZE];           // 임시 수신 버퍼
static char g_recBuf[UART_VIEW_MAX_REC][UART_VIEW_REC_LEN]; // 링 버퍼 (5KB)
```

> SRAM 사용: `g_UART_buff`(1KB) + `g_recBuf`(5KB) = **6KB**
> 이전(2KB × 6 = 12KB) 대비 **6KB 절감**

#### 수신 흐름

```
UART FIFO 폴링
  → '\n' 또는 '\r' 수신 시 → WHITE 확정 → saveRecord()
  → 1023자 오버플로우 시  → WHITE 확정 → saveRecord() (강제 커밋)
  → 200 루프 무수신 시    → WHITE 확정 → saveRecord() (타임아웃 flush)
  → 링 버퍼에 저장 (5줄 순환)
  → 자동 스크롤: 최신 라인으로 이동
  ※ 에코 없음 / 모든 케이스 WHITE 표시
```

#### LCD 뷰어 레이아웃 (320×240)

```
[y:  0 ~  39]  헤더: "UART RX  [N줄]"  (진한 파란색 배경, Font8x16)
[y: 40 ~ 199]  바디: 수신 텍스트 (스크롤 가능, 최대 8줄 표시)
[y:200 ~ 239]  푸터: [  UP  ] [  DOWN  ] [  CLEAR  ] (3분할 버튼)
```

| 설정값 | 내용 |
|--------|------|
| `UART_VIEW_LINE_H` | 18px (Font8x16 높이 16 + 여백 2) |
| `UART_VIEW_FONT_W` | 8px → 한 줄 최대 40자 |
| 가시 라인 수 | (240 - 40 - 40) ÷ 18 = **8줄** |

#### 터치 버튼 동작

| 버튼 | 영역 (x) | 동작 |
|------|---------|------|
| UP | 0 ~ 79 | 스크롤 1칸 위 (자동 스크롤 OFF) |
| DOWN | 80 ~ 159 | 스크롤 1칸 아래 (최하단 도달 시 자동 스크롤 ON) |
| CLEAR | 160 ~ 239 | 모든 수신 이력 초기화 |

> 세 버튼 모두 터치 시 **부저 ~50ms 피드백** (`BUZZER_CLICK`) 울림

### 4.3 fonts.h — 폰트 현황

| 폰트 | 크기 | 용도 | 상태 |
|------|------|------|------|
| `SmallFont8x12` | 1,144 B | 소형 표시 (보조) | 유지 |
| `Font8x16` | 1,524 B | UART 뷰어 전체 | **신규 추가 (2026-03-13)** |
| `AlibriNumBold32x48` | 1,924 B | 미사용 | **제거 (2026-03-13)** |
| `Arial_round_16x24` | 4,564 B | 미사용 | **제거 (2026-03-13)** |
| `DotMatrix_M_16x22` | ~2,700 B | 미사용 | **제거 (2026-03-13)** |
| `Grotesk16x32` | ~2,700 B | 미사용 | **제거 (2026-03-13)** |
| `ArialNumFontPlus32x50` | ~500 B | 미사용 | **제거 (2026-03-13)** |
| `SevenSegNumFontPlusPlus32x50` | 2,604 B | 미사용 | **제거 (2026-03-13)** |

> 폰트 정리로 Flash 약 **14,992 bytes 절감** 예상

### 4.4 지원 최대 UART 수신 데이터 크기

실측한 최대 JSON 데이터 크기:

```json
{"items": [
  {"downtime_idx":"1","downtime_name":"EDIT PROGRAM","not_completed_qty":"0"},
  ...10개 항목...
]}
```

- 항목당 최대 78 bytes × 10 + wrapper 13 bytes + 구분자 9 bytes = **802 bytes**
- 버퍼 크기 1024 bytes로 여유 있게 처리 가능

---

## 5. 메모리 사용량

### V1 최종 빌드 결과 (2026-03-13)

| 영역 | 사용량 | 비율 | 목표 | 상태 |
|------|--------|------|------|------|
| **Flash** | 56,228 / 262,144 bytes | **21.4%** | 60% 이하 | ✅ |
| **SRAM** | 11,004 / 32,768 bytes | **33.6%** | 70% 이하 | ✅ |
| Stack | 2,048 bytes | — | — | — |
| Heap | 1,024 bytes | — | — | — |

> Flash 상세: Bootloader 13,568 B + Application 42,404 B + Metadata 256 B

### SRAM 최적화 이력

| 시점 | SRAM | 비율 | 변경 내용 |
|------|------|------|---------|
| 최초 (기반 코드) | 17,524 B | 53.5% | 초기 상태 |
| 버퍼 최적화 후 | 11,004 B | 33.6% | `g_UART_buff` + `g_recBuf` 각 2048→1024로 축소 (6KB 절감) |

---

## 6. 코드 규모

| 파일 | 역할 | 비고 |
|------|------|------|
| `uartJson.c` | UART 수신 + LCD 뷰어 + USB 출력 | 핵심 파일 |
| `main.c` | 메인 루프 | 간결한 구조 |
| `setup.c` | 하드웨어 초기화 | WiFi 비활성화 완료 |
| `userTimer.c` | 1ms / 1s 타이머 | 유지 |
| `lib/UI.c` | LCD 그리기 유틸 | LCD_printf, FillRect 등 |
| `lib/USB.c` | USB CDC 출력 | printf_USB 포함 |
| `lib/fonts.h` | 폰트 정의 | SmallFont8x12, Font8x16 두 개만 유지 |

---

## 7. 개발 환경 설정

| 항목 | 내용 |
|------|------|
| **IDE** | PSoC Creator 4.x |
| **컴파일러** | ARM GCC 5.4.1 |
| **디버거** | MiniProg3 또는 KitProg |
| **UART 설정** | 115200 bps, 8N1 |
| **USB CDC** | Windows: usbser.sys 드라이버 |

### 빌드 절차

1. PSoC Creator에서 `Design.cydsn` 열기
2. Build → Generate Application
3. Build → Build `Design`
4. Program/Debug → Program

### UART 통신 확인 절차

1. 외부 MCU와 UART 연결 (TX → RX, GND 공통)
2. 280CTP_IoT_UART_TEST 전원 ON
3. LCD 화면에 "UART RX [0]" 헤더 표시 확인
4. 외부 MCU에서 데이터 전송 → LCD 바디에 수신 텍스트 표시 확인
5. PC에서 USB CDC 시리얼 모니터 열기 → 동일 데이터 수신 확인
6. UP/DOWN 버튼으로 스크롤, CLEAR 버튼으로 초기화 확인

---

## 8. 알려진 이슈

| 심각도 | 파일 | 이슈 | 상태 |
|--------|------|------|------|
| **낮음** | `lib/image.h` | `const` 적용 여부 확인 필요 (SRAM 과다 사용 방지) | 확인 필요 |

> 이전에 존재하던 주요 이슈(LCD 뷰어 미구현, USB 미연결, WiFi 미제거 등)는 모두 해결 완료.

---

## 9. 버전 이력 테이블

| 날짜 | 버전 | 상태 | 변경 내용 |
|------|------|------|---------|
| 2026-03-12 | V1 | 초기 구성 | 280CTP_IoT_INTEGRATED_V1 기반 복사, WiFi 주석 처리 |
| 2026-03-13 | V1 | 기능 구현 완료 | UART 뷰어 + USB 출력 + SRAM 최적화 + 폰트 정리 |
| 2026-03-14 | V1 | 기능 추가 | WHITE 표시 전환 + 타임아웃 flush + 버튼 부저 피드백 + LED1 GREEN 상시점등 |

---

## 10. 관련 파일 경로 빠른 참조

| 파일 | 경로 |
|------|------|
| 메인 로직 | `280CTP_IoT_UART_TEST_V1/Project/Design.cydsn/main.c` |
| UART 뷰어 (핵심) | `280CTP_IoT_UART_TEST_V1/Project/Design.cydsn/uartJson.c` |
| 하드웨어 초기화 | `280CTP_IoT_UART_TEST_V1/Project/Design.cydsn/setup.c` |
| 폰트 정의 | `280CTP_IoT_UART_TEST_V1/Project/Design.cydsn/lib/fonts.h` |
| LCD 드라이버 | `280CTP_IoT_UART_TEST_V1/Project/Design.cydsn/lib/ST7789V.c` |
| 터치 드라이버 | `280CTP_IoT_UART_TEST_V1/Project/Design.cydsn/lib/FT5x46.c` |
| USB 통신 | `280CTP_IoT_UART_TEST_V1/Project/Design.cydsn/lib/USB.c` |
| 기반 프로젝트 | `../280CTP_IoT_INTEGRATED/280CTP_IoT_INTEGRATED_V1/` |

---

*Copyright SUNTECH, 2023-2026*
