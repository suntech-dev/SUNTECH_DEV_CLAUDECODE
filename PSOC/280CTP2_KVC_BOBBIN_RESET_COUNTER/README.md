# 280CTP2_KVC_BOBBIN_RESET_COUNTER 프로젝트 분석 문서

> 최초 작성: 2026-03-05
> 분석 버전: V1 (초기 구현)
> 마지막 코드 개선: 2026-03-05
> 마지막 업데이트: 2026-03-05

---

## 1. 프로젝트 개요

이 프로젝트는 **Cypress PSoC 4** 기반의 **보빈 리셋 카운터(Bobbin Reset Counter)** 펌웨어입니다.
봉제 기계의 **보빈(실패) 교체 주기를 카운팅**하고, 설정된 트림 횟수 도달 시 부저 알림과 외부 신호를 출력하는 생산 관리 시스템입니다.
LCD 터치 디스플레이를 통해 현재 카운트와 목표값을 직관적으로 표시하며, 물리적 RESET 버튼으로 카운터를 초기화할 수 있습니다.

### 주요 기능 요약

| 기능 | 설명 |
|------|------|
| 트림 카운터 | 외부 TC_INT 신호를 ISR로 감지하여 카운트 자동 증가 |
| 목표값 설정 | SET 메뉴 → Trim Count 를 통한 목표 트림 횟수 설정 (1~99) |
| 알림 출력 | 목표 도달 시 부저 경보 + RESERVED_OUT_1 외부 신호 HIGH |
| 카운터 리셋 | RESET 버튼 또는 터치 RESET 버튼으로 카운터 초기화 |
| 데이터 저장 | 내부 EEPROM에 카운트 저장 (전원 차단 후 복원) |
| LCD UI | ST7789V (320x240, LANDSCAPE) 터치 디스플레이 |
| WiFi 통신 | ESP 기반 WiFi 모듈 연동 (현재 미활성화 상태) |
| USB 디버그 | USB CDC UART 통한 디버그 출력 |

---

## 2. 버전 구조

```
280CTP2_KVC_BOBBIN_RESET_COUNTER/
├── 280CTP2_KVC_BOBBIN_RESET_COUNTER_V1/    # V1 - 초기 구현 (2026.03)
│   ├── Design.cydsn/                        # 메인 애플리케이션
│   │   ├── main.c / main.h                  # 메인 진입점, 공통 정의
│   │   ├── setup.c / setup.h                # 하드웨어 초기화
│   │   ├── count.c / count.h                # 트림 카운터 로직 + ISR
│   │   ├── uart.c / uart.h                  # UART 디버그 출력
│   │   ├── package.h                        # 프로젝트 타입 선택 매크로
│   │   ├── userProjectCounter.c/h           # MACHINE_PARAMETER 정의
│   │   ├── userMenuCounter.c                # LCD UI 메뉴 (TOP/SET/RESET)
│   │   ├── menuDesign.h                     # 메뉴 트리 생성 인터페이스
│   │   ├── productCounter.h                 # (예약, 미사용)
│   │   ├── lib/                             # 공통 라이브러리
│   │   │   ├── ST7789V.c/h                  # LCD SPI 드라이버
│   │   │   ├── FT5x46.c/h                   # 터치 컨트롤러 드라이버
│   │   │   ├── menu.c/h                     # 메뉴 트리 시스템
│   │   │   ├── widget.c/h                   # LCD UI 위젯
│   │   │   ├── UI.c/h                       # UI 공통 기능
│   │   │   ├── button.c/h                   # 버튼 렌더링
│   │   │   ├── sysTick.c/h                  # 1ms SysTick ISR
│   │   │   ├── internalFlash.c/h            # 내부 EEPROM (Em_EEPROM)
│   │   │   ├── externalFlash.c/h            # 외부 SPI Flash (W25Qxx)
│   │   │   ├── LEDControl.c/h               # RGB LED 제어
│   │   │   ├── WIFI.c/h                     # WiFi 모듈 통신
│   │   │   ├── server.c/h                   # 서버 통신
│   │   │   ├── andonMenu.c/h                # 안돈 메뉴
│   │   │   ├── manageMenu.c/h               # 관리 메뉴
│   │   │   ├── RealTimeClock.c/h            # RTC 기능
│   │   │   ├── IODefine.h / IODefine.c      # I/O 핀 매핑 정의
│   │   │   ├── IOUtil.c/h                   # I/O 엣지 검출 유틸리티
│   │   │   ├── w25qxx.c/h                   # W25Qxx Flash 드라이버
│   │   │   └── config.h                     # WiFi 설정 구조체
│   │   ├── jsmn-master/                     # JSON 파서 라이브러리
│   │   └── Generated_Source/PSoC4/          # PSoC Creator 자동 생성 코드
│   ├── bootloader.cydsn/                    # USB 부트로더
│   └── *.cyprj / *.cywrk                    # PSoC Creator 프로젝트 파일
│
├── CLAUDE.md                                # 코딩 규칙 문서 (이 파일과 같은 위치)
├── README.md                                # 이 문서
├── README.html                              # HTML 버전 문서
└── VERSION_HISTORY.md                       # 버전별 변경 이력
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
| **컴파일러** | ARM GCC 5.4-2016-q2-update |
| **LCD** | ST7789V (320×240, SPI 인터페이스, LANDSCAPE 고정) |
| **터치** | FT5x46 (I2C 기반 커패시티브 터치) |
| **외부 Flash** | W25Qxx (SPI, 설정 저장용) |
| **WiFi** | ESP 계열 UART 연결 모듈 (현재 미활성화) |

### 하드웨어 컴포넌트 (PSoC Creator)

| 컴포넌트 | 타입 | 역할 |
|---------|------|------|
| `TC_INT` | GPIO 입력 + ISR | 트림 카운트 신호 입력 (상승 엣지) |
| `TC_RESET` | GPIO 입력 | 물리적 RESET 버튼 (LOW = 누름) |
| `RESET_KEY` | GPIO 입력 | 물리적 리셋 키 |
| `BUZZER` | GPIO 출력 | 부저 알림음 |
| `SPIM_LCD` | SPI Master | ST7789V LCD 인터페이스 |
| `I2C_TC` | I2C Master | FT5x46 터치 컨트롤러 |
| `SPIM_FLASH` | SPI Master | W25Qxx 외부 Flash |
| `UART` | SCB UART | 디버그 출력 |
| `WIFI` | SCB UART | WiFi 모듈 통신 |
| `USBUART` | USB CDC | USB 가상 직렬 포트 |
| `RESERVED_OUT_1` | GPIO 출력 | 트림 완료 외부 신호 |
| `RESERVED_OUT_2` | GPIO 출력 | 예약 출력 |
| `RESERVED_IN_1/2` | GPIO 입력 | 예약 입력 |
| `LED1_R/G` | GPIO 출력 | LED1 RGB |
| `LED2_R/G/B` | GPIO 출력 | LED2 RGB |
| `LCD_Backlight` | GPIO 출력 | LCD 백라이트 |
| `TCPWM_LED` | PWM | LED 밝기 제어 |

---

## 4. 소스코드 상세 분석

### 4.1 메인 루프 구조

```c
main()
├── CySysClkWcoStart()       // WCO 클럭 시작
├── CyGlobalIntEnable        // 전역 인터럽트 활성화
├── SetUp()
│   ├── BUZZER/LCD/Touch 초기화
│   ├── SPIM_LCD_Start() → ST7789V_Init()
│   ├── I2C_TC_Start() → TouchHardwareInit()
│   ├── SPIM_FLASH_Start()
│   ├── UART_Start()
│   ├── initLEDControl()     (중복 호출 이슈 존재)
│   ├── initTimer() / initUSBJsonParsor() / initWIFI()
│   ├── registerCounter_1s(1)
│   ├── initUserProject()    // MACHINE_PARAMETER 포인터 설정
│   ├── initServer() / initExternalFlash() / initInternalFlash()
│   ├── initCount()          // ISR 등록 + 카운터 초기화
│   └── initMenu()           // 메뉴 트리 생성
└── for(;;)
    ├── OneMilliSecond_MainLoop()   // 1ms 폴링 (I/O, 타이머)
    ├── OneSecond_MainLoop()        // 1s 폴링
    ├── usbJsonParsorLoop()         // USB JSON 수신 처리
    ├── wifiLoop()                  // WiFi 통신 처리
    ├── LED_OneSecondControl()      // LED 점멸 제어 (1s 주기)
    ├── MenuLoop()                  // LCD UI 메뉴 처리
    └── SetCountLoop()              // 트림 카운터 로직
```

### 4.2 트림 카운터 동작 흐름

```
TC_INT 핀 상승 엣지 (봉제기 트림 신호)
    └── CY_ISR(Trim_Interrupt_Routine)
            ├── g_updateTrimCount = TRUE
            └── g_ptrCount->count++

SetCountLoop() [메인 루프 폴링]
    ├── [RESET 조건] RESET_KEY_Read() == LOW && count != 0
    │       ├── CyDelay(50) 디바운싱
    │       ├── g_ptrCount->count = 0
    │       ├── SaveInternalFlash()
    │       ├── RESERVED_OUT_1_Write(0)
    │       └── Buzzer(BUZZER_STOP, 0)
    ├── [알림 조건] count == setTrimCount → 부저 + RESERVED_OUT_1 HIGH
    └── [저장 조건] g_updateTrimCount → SaveInternalFlash() + g_updateCountMenu = TRUE
```

### 4.3 메뉴 트리 구조

```
ROOT (TOP MENU)         ← doTopMenu()
├── Unlock with Key     ← doUnlockWithKey()   [터치 클릭 시 부모로 복귀]
└── Trim Count          ← doTargetInfoMenu()  [목표 트림 횟수 설정: +1/+10]
```

**TOP MENU 화면 구성**:
- 좌측 상단: `SET` 버튼 (빨강, Grotesk16x32 폰트) — 서브메뉴 진입
- 좌측 하단: `RESET` 버튼 (파랑) — 카운터 직접 초기화
- 우측 상단: 목표 트림값 (노랑, ArialNumFontPlus32x50 대형 폰트)
- 우측 하단: 현재 카운트 (초록, 대형 폰트)

### 4.4 데이터 저장 구조

```
내부 EEPROM (Em_EEPROM, 128 bytes)
└── INTERNAL_CONFIG (128 bytes)
    ├── watermark   [2 bytes]  : 0x3039 (12345)
    ├── data        [124 bytes]: COUNT 구조체 저장
    │   └── COUNT.count [2 bytes]: 현재 트림 카운트
    └── checksum    [2 bytes]

외부 SPI Flash (W25Qxx, 마지막 섹터)
└── EXTERNAL_CONFIG
    ├── watermark   [20 bytes] : "SUNTECH IOT"
    ├── data        [200 bytes]: 서버/화면 설정 (CONFIG, EXTERNAL_MISC_CONFIG)
    ├── userData    [300 bytes]: MACHINE_PARAMETER 저장
    │   └── setTrimCount [2 bytes]: 목표 트림 횟수 (1~99)
    └── CRC         [2 bytes]
```

### 4.5 타이머 시스템

- **SysTick (1ms ISR)**: `SysTickISRCallback_1ms()` → `registerCounter_1ms()` 로 최대 20개 타이머 관리
- **1초 카운터**: `registerCounter_1s()` → LED 제어, 1초 주기 작업
- `isFinishCounter_1ms(index)` / `isFinishCounter_1s(index)` 로 타이머 완료 여부 폴링

---

## 5. 프로젝트 특화 분석

### 5.1 조건부 컴파일 구조 (package.h)

```c
//#define PROJECT_NAME1
#define USER_PROJECT_TRIM_COUNT            // ← 현재 활성 모드
//#define USER_PROJECT_PATTERN_SEWING_MACHINE
```

| 매크로 | 활성화 코드 범위 |
|--------|----------------|
| `USER_PROJECT_TRIM_COUNT` | `count.c`, `userProjectCounter.c/h`, `userMenuCounter.c` 내 해당 `#ifdef` 블록 |
| `USER_PROJECT_PATTERN_SEWING_MACHINE` | 재봉기 패턴 머신용 복잡한 카운팅 구조체 + ISR (현재 비활성) |

### 5.2 IOUtil 프레임워크 (미활성화 상태)

`lib/IODefine.h` / `lib/IOUtil.h` 는 입출력 엣지 검출 및 타이머 추상화 프레임워크를 제공하나, 현재 `IODefine.c`의 `getSensor()`, `defineIO()`, `doOutput()` 내 모든 실제 핀 접근 코드가 주석 처리된 상태. `IOUtil` 구조체(`SIOUtil`, `SIOTUtil`)도 실질적으로 미사용.

### 5.3 WiFi/서버 기능 현황

`WIFI.c`, `server.c`, `USBJsonConfig`, `jsmn-master` 등 IoT 관련 코드가 포함되어 있으나, `widget.c`의 `DrawHeader()`에서 `#ifdef USE_WIFI` 조건으로 WiFi 표시가 제어됨. `USE_WIFI`가 정의되지 않아 WiFi 아이콘은 표시되지 않으나 초기화 코드(`initWIFI()`)는 여전히 실행됨.

---

## 6. 안돈(Andon) 시스템 연동

`andonMenu.c`, `andonApi.h` 등 안돈 시스템 연동 파일이 포함되어 있으나 현재 `USER_PROJECT_TRIM_COUNT` 모드에서는 안돈 API 직접 호출이 없음. 향후 서버 전송 기능 활성화 시 연동 예정으로 보임.

---

## 7. 부트로더 구조

```
bootloader.cydsn/
├── Generated_Source/PSoC4/
│   ├── Bootloader.c          # 부트로더 메인
│   ├── UART.c                # UART 통신
│   └── CyFlash.c             # Flash 쓰기
└── bootloader_datasheet.pdf

부트로더 프로토콜: UART 기반
업데이트 방법: UART 포트를 통한 펌웨어 다운로드
```

> 참고: 280CTP2_KVC_TRIM_RESET_COUNTER_V2의 경우 USB HID 기반 `Bootloadable_Load()`를 사용했으나, 이 프로젝트의 부트로더는 UART 기반으로 구성됨.

---

## 8. 발견된 이슈 및 개선 이력

### 8.1 미해결 이슈 (V1 기준)

| 우선순위 | 심각도 | 파일 | 이슈 | 위험 |
|---------|--------|------|------|------|
| 1 | **심각** | `count.c:33` | ISR 접근 변수(`g_updateTrimCount`, `g_ptrCount->count`)에 `volatile` 미선언 | 컴파일러 최적화로 ISR 갱신 무시 가능 |
| 2 | **심각** | `uart.c:22` | `vsprintf(buff, fmt, ap)` — 1KB 스택 버퍼 + 크기 제한 없음 | 스택 오버플로우, 메모리 손상 |
| 3 | **높음** | `count.c:104` | `CY_ISR(Trim_Interrupt_Routine)` 중복 정의 (`#ifdef` 내·외 각 1회) | 두 매크로 동시 활성화 시 컴파일 오류 |
| 4 | **높음** | `externalFlash.c:98` | `checkSum()` 루프 `i=1` 시작 — 첫 번째 바이트 누락 | CRC 무결성 오류 |
| 5 | **높음** | `internalFlash.c:20` | 워터마크/CRC 검증 코드 전체 주석 처리 → 항상 `TRUE` 반환 | 손상된 EEPROM 데이터 그대로 사용 |
| 6 | **중간** | `main.h:59` | `struct sParameter Param;` 헤더 파일에 전역 인스턴스 선언 | 다중 include 시 중복 정의 링크 오류 |
| 7 | **중간** | `setup.c:54,65` | `initLEDControl()` 두 번 연속 호출 | TCPWM 이중 Start 가능 |
| 8 | **낮음** | `count.c:65` | `CyDelay(50)` busy-wait 디바운싱 — 메인 루프 50ms 차단 | 반응성 저하 |
| 9 | **낮음** | `main.h:22` / `package.h:27` | 버전 정보 불일치 (`20211101(0.0.8)` vs `0.0.1`) | 버전 추적 혼란 |
| 10 | **낮음** | `IODefine.c` | `getSensor()`, `defineIO()`, `doOutput()` 전체 주석 처리 | Dead code |

### 8.2 수정 이력

| 날짜 | 내용 |
|------|------|
| 2026-03-05 | V1 초기 분석 — 이슈 목록 최초 작성 |

---

## 9. 테스트 시나리오

### 9.1 기본 카운터 테스트

| 시나리오 | 절차 | 기대 결과 |
|---------|------|---------|
| 카운트 증가 | TC_INT 핀에 상승 엣지 신호 인가 | LCD 현재 카운트 +1 표시 |
| 목표 도달 알림 | 카운트가 setTrimCount에 도달 | 부저 경보 + RESERVED_OUT_1 HIGH |
| RESET 버튼 | 카운트 > 0 상태에서 RESET_KEY LOW | 카운트 0으로 초기화, EEPROM 저장 |
| 전원 차단/복원 | 임의 카운트 상태에서 전원 OFF/ON | EEPROM에서 카운트 복원 |

### 9.2 목표값 설정 테스트

| 시나리오 | 절차 | 기대 결과 |
|---------|------|---------|
| 목표 증가 (+1) | doTargetInfoMenu → 상승 버튼 1단계 | setTrimCount +1 |
| 목표 증가 (+10) | doTargetInfoMenu → 상승 버튼 2단계 | setTrimCount +10 |
| 목표 저장 | SAVE 버튼 터치 | External Flash 저장, 메뉴 복귀 |
| 최대값 제한 | 99 초과 시도 | 99에서 증가 안 됨 |

### 9.3 LED 동작 테스트

| 시나리오 | 기대 결과 |
|---------|---------|
| 전원 ON | LED1 녹색 점등 (고정) |
| WiFi 신호 강도별 | LED1 색상 변화 (미연결: 현재 상태 유지) |

---

## 10. 개발 환경 설정

| 항목 | 내용 |
|------|------|
| **IDE** | PSoC Creator 4.x |
| **컴파일러** | ARM GCC 5.4-2016-q2-update |
| **디버거** | MiniProg3 또는 KitProg |
| **플래시 도구** | PSoC Creator 내장 프로그래머 |
| **시리얼 모니터** | 115200 bps, 8N1 |
| **USB CDC** | Windows: usbser.sys 드라이버 |

### 빌드 절차

1. PSoC Creator에서 `Design.cydsn` 워크스페이스 열기
2. `package.h`에서 활성 프로젝트 매크로 확인 (`USER_PROJECT_TRIM_COUNT`)
3. Build → Generate Application
4. Build → Build `Design`
5. Program/Debug → Program

---

## 11. 버전 이력 테이블

| 날짜 | 버전 | 상태 | 변경 내용 |
|------|------|------|---------|
| 2026-03-05 | V1 | 개발 중 | 초기 구현 — 트림 카운터 + LCD UI + WiFi 구조 |

---

## 12. 관련 파일 경로 빠른 참조

| 파일 | 경로 |
|------|------|
| 메인 로직 | `280CTP2_KVC_BOBBIN_RESET_COUNTER_V1/Design.cydsn/main.c` |
| 카운터 ISR | `280CTP2_KVC_BOBBIN_RESET_COUNTER_V1/Design.cydsn/count.c` |
| UI 메뉴 | `280CTP2_KVC_BOBBIN_RESET_COUNTER_V1/Design.cydsn/userMenuCounter.c` |
| 프로젝트 타입 | `280CTP2_KVC_BOBBIN_RESET_COUNTER_V1/Design.cydsn/package.h` |
| 머신 파라미터 | `280CTP2_KVC_BOBBIN_RESET_COUNTER_V1/Design.cydsn/userProjectCounter.c` |
| 내부 EEPROM | `280CTP2_KVC_BOBBIN_RESET_COUNTER_V1/Design.cydsn/lib/internalFlash.c` |
| 외부 Flash | `280CTP2_KVC_BOBBIN_RESET_COUNTER_V1/Design.cydsn/lib/externalFlash.c` |
| LED 제어 | `280CTP2_KVC_BOBBIN_RESET_COUNTER_V1/Design.cydsn/lib/LEDControl.c` |
| SysTick | `280CTP2_KVC_BOBBIN_RESET_COUNTER_V1/Design.cydsn/lib/sysTick.c` |
| I/O 정의 | `280CTP2_KVC_BOBBIN_RESET_COUNTER_V1/Design.cydsn/lib/IODefine.h` |

---

*Copyright SUNTECH, 2023-2026*
