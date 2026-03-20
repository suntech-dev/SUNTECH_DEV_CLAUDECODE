# 280CTP_IoT_INTEGRATED 프로젝트 분석 문서

> 최초 작성: 2026-03-09
> 분석 버전: V2_BLACK_CPU (OTA 무선 업데이트 추가)
> 마지막 빌드: 2026-03-20 (성공 — bootloader + Design 순차 빌드)
> 마지막 코드 개선: 2026-03-20
> 마지막 업데이트: 2026-03-20

---

## 1. 프로젝트 개요

이 프로젝트는 **Cypress PSoC 4** 기반의 **재봉기 IoT 통합 모니터링 시스템** 펌웨어입니다.
ANDON(안돈) 시스템과 연동하여 **봉제 라인의 생산 현황을 실시간으로 서버에 전송**하고,
LCD 터치 디스플레이를 통해 작업 지시, 가동 현황, 불량 관리 등을 수행하는 스마트 공장 솔루션입니다.

기반 버전: `IoT_SCI_2025.07.23 Rev.9.8.3` → 통합 버전으로 재구성

### 주요 기능 요약

| 기능 | 설명 |
|------|------|
| ANDON 연동 | HTTP GET으로 서버(`/api/sewing.php`)에 생산 데이터 자동 전송 |
| 재봉 카운팅 | 트림(Trim) 신호 인터럽트 기반 제품 수량 자동 카운팅 |
| 패턴 관리 | 패턴별 목표/실적 집계, 서버 자동 동기화 |
| 가동 중단 관리 | 가동 중단 사유 등록 및 이력 관리 (`downtime.c`) |
| 불량 관리 | 불량 유형별 수량 등록 및 집계 (`defective.c`) |
| LCD 터치 UI | ST7789V (320×240, SPI) + FT5x46 터치 컨트롤러 |
| WiFi 통신 | ESP 계열 UART WiFi 모듈로 서버 HTTP 통신 |
| USB 설정 | USB CDC 통해 JSON 기반 디바이스 설정 (`SuntechIoTConfig_V1`) |
| 외부 플래시 | W25Qxx SPI Flash — 서버 설정, WiFi 인증 정보 저장 |
| 내부 EEPROM | 카운트, 가동 시간 데이터 지속 저장 |

---

## 2. 버전 구조

```
280CTP_IoT_INTEGRATED/
├── 280CTP_IoT_INTEGRATED_V2_BLACK_CPU/          # V2 - OTA 무선 업데이트 추가 (2026.03) ← 최신
│   ├── Project/
│   │   ├── Design.cydsn/                        # 메인 애플리케이션 펌웨어
│   │   │   ├── otaMenu.c / otaMenu.h            # OTA 업데이트 메뉴 + 자동 체크 (신규)
│   │   │   ├── lib/WIFI.c                       # WiFi 통신 (OTA HTTP GET 케이스 추가)
│   │   │   ├── lib/widget.c                     # LCD 위젯 (DrawHeader OTA 배지 추가)
│   │   │   ├── lib/server.h                     # 서버 설정 (DEFAULT_OTA_API_PATH 추가)
│   │   │   └── ...                              # V1_BLACK_CPU와 동일한 나머지 파일
│   │   ├── bootloader.cydsn/                    # OTA 부트로더 (W25QXX 외부 Flash → 내부 Flash 프로그래밍)
│   │   └── *.cyprj / *.cywrk
│   └── SuntechIoTConfig_V1/                     # C# 설정 도구
│
├── 280CTP_IoT_INTEGRATED_V1_BLACK_CPU/          # V1 - PATTERN_MACHINE 전용 (2026.03)
│   ├── Project/
│   │   ├── Design.cydsn/                        # SEWING_MACHINE/전류센서 제거 버전
│   │   └── bootloader.cydsn/
│   └── SuntechIoTConfig_V1/
│
├── 280CTP_IoT_INTEGRATED_V1/                   # V1 - 통합 IoT 초기 구현 (2025.11)
│   ├── Project/
│   │   ├── Design.cydsn/                        # 메인 애플리케이션 펌웨어
│   │   │   ├── main.c / main.h                  # 메인 진입점, 공통 타입 정의
│   │   │   ├── setup.c / setup.h                # 하드웨어 초기화
│   │   │   ├── package.h                        # 프로젝트 타입 선택 매크로
│   │   │   ├── andonApi.c / andonApi.h          # ANDON HTTP API 요청 생성
│   │   │   ├── andonJson.c / andonJson.h        # ANDON JSON 응답 파싱
│   │   │   ├── andonMessageQueue.c/h            # ANDON 메시지 큐
│   │   │   ├── andonMenu.c / andonMenu.h        # ANDON UI 메뉴
│   │   │   ├── count.c / count.h                # 제품 카운팅 로직
│   │   │   ├── downtime.c / downtime.h          # 가동 중단 관리
│   │   │   ├── defective.c / defective.h        # 불량 관리
│   │   │   ├── uartJson.c / uartJson.h          # UART JSON 처리
│   │   │   ├── USBJsonConfig.c/h                # USB JSON 설정 인터페이스
│   │   │   ├── userProjectPatternSewing.c/h     # 패턴 재봉기 프로젝트 정의
│   │   │   ├── userMenuPatternSewingMachine.c   # 재봉기 패턴 메뉴 (최대 파일)
│   │   │   ├── menuDesign.h                     # 메뉴 트리 인터페이스
│   │   │   ├── currentSensor.c                  # 전류 센서 처리
│   │   │   ├── WarningLight.c/h                 # 경고 표시등 제어
│   │   │   ├── resetMenu.c/h                    # 리셋 메뉴
│   │   │   ├── jsonUtil.c                       # JSON 유틸리티
│   │   │   ├── userTimer.c/h                    # 사용자 타이머
│   │   │   └── lib/                             # 공통 라이브러리
│   │   │       ├── ST7789V.c/h                  # LCD SPI 드라이버
│   │   │       ├── FT5x46.c/h                   # 터치 컨트롤러 드라이버
│   │   │       ├── WIFI.c/h                     # WiFi 모듈 통신
│   │   │       ├── server.c/h                   # 서버 연결 관리
│   │   │       ├── menu.c/h                     # 메뉴 트리 시스템
│   │   │       ├── widget.c/h                   # LCD UI 위젯 (대형)
│   │   │       ├── UI.c/h                       # UI 공통 기능
│   │   │       ├── button.c/h                   # 버튼 렌더링
│   │   │       ├── manageMenu.c/h               # 관리 메뉴
│   │   │       ├── sysTick.c/h                  # 1ms SysTick ISR
│   │   │       ├── internalFlash.c/h            # 내부 EEPROM
│   │   │       ├── externalFlash.c/h            # 외부 SPI Flash
│   │   │       ├── LEDControl.c/h               # RGB LED 제어
│   │   │       ├── RealTimeClock.c/h            # RTC 기능
│   │   │       ├── image.h                      # LCD 이미지 데이터 (const, FLASH)
│   │   │       ├── fonts.h                      # LCD 폰트 데이터 (const, FLASH)
│   │   │       ├── config.h                     # WiFi/서버 설정 구조체
│   │   │       ├── IODefine.h / IODefine.c      # I/O 핀 매핑
│   │   │       ├── IOUtil.c/h                   # I/O 엣지 검출 유틸리티
│   │   │       └── USB.c/h                      # USB 통신
│   │   │   ├── jsmn-master/                     # JSON 파서 라이브러리 (JSMN)
│   │   │   └── Generated_Source/PSoC4/          # PSoC Creator 자동 생성 코드
│   │   ├── bootloader.cydsn/                    # UART 기반 부트로더
│   │   └── *.cyprj / *.cywrk                    # PSoC Creator 프로젝트 파일
│   └── SuntechIoTConfig_V1/                     # C# 설정 도구 (.NET 4.7.2)
│       ├── MainForm.cs                           # 메인 UI (직렬 포트 통신)
│       ├── SysCmdForm.cs                         # 시스템 명령 창
│       └── Program.cs                            # 진입점
│
├── CLAUDE.md                                     # 코딩 규칙 문서
├── README.md                                     # 이 문서
├── VERSION_HISTORY.md                            # 버전별 변경 이력
└── SRAM_최적화_분석보고서.md                       # SRAM 최적화 분석 보고서
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
| **외부 Flash** | W25Qxx (SPI, 설정 저장용) |
| **WiFi** | ESP 계열 UART 연결 모듈 (활성화 상태) |

### 하드웨어 컴포넌트 (PSoC Creator)

| 컴포넌트 | 타입 | 역할 |
|---------|------|------|
| `TrimPin` | GPIO 입력 + ISR | 트림 카운트 신호 입력 (재봉기 실 자르기) |
| `TC_INT` | GPIO 입력 + ISR | 추가 카운트 입력 |
| `TC_RESET` | GPIO 입력 | 물리적 RESET 버튼 |
| `BUZZER` | GPIO 출력 | 부저 알림음 |
| `SPIM_LCD` | SPI Master | ST7789V LCD 인터페이스 |
| `I2C_TC` | I2C Master | FT5x46 터치 컨트롤러 |
| `SPIM_FLASH` | SPI Master | W25Qxx 외부 Flash |
| `UART` | SCB UART | 디버그 출력 |
| `WIFI` (UART_WIFI) | SCB UART | WiFi 모듈 통신 |
| `USBUART` | USB CDC | USB 가상 직렬 포트 |
| `LCD_Backlight` | GPIO 출력 | LCD 백라이트 |
| `WIFI_EN` | GPIO 출력 | WiFi 모듈 활성화 제어 |
| `WIFI_RESET` | GPIO 출력 | WiFi 모듈 리셋 |
| `LED1_R/G/B` | GPIO 출력 | LED1 RGB |
| `LED2_R/G/B` | GPIO 출력 | LED2 RGB |
| `RESERVED_IN_2` | GPIO 입력 | 예약 입력 |
| `RESERVED_OUT_1/2/3` | GPIO 출력 | 예약 출력 |
| `InputPin` | GPIO 입력 | 범용 입력 핀 |
| `ADC_SAR_Seq` | ADC | 전류 센서 아날로그 입력 |
| `PWM_ONE_SECOND` | PWM | 1초 주기 타이머 |
| `TCPWM_LED` | PWM | LED 밝기 제어 |
| `Ctrl_LCD_RS` | 디지털 출력 | LCD RS 제어 |
| `Ctrl_MEM_SS` | 디지털 출력 | Flash CS 제어 |
| `RTC` | RTC | 실시간 클럭 |

---

## 4. 소스코드 상세 분석

### 4.1 메인 루프 구조

```c
main()
├── CySysClkWcoStart()           // WCO 클럭 시작
├── CyGlobalIntEnable            // 전역 인터럽트 활성화
├── SetUp()                      // 하드웨어 초기화
│   ├── LCD / Touch / Flash 초기화
│   ├── UART / WIFI / USB 초기화
│   ├── initLEDControl()
│   ├── initWIFI()
│   ├── initUserProject()        // 패턴 재봉기 파라미터 초기화
│   ├── initExternalFlash()      // 서버/WiFi 설정 로드
│   ├── initInternalFlash()      // 카운트 데이터 로드
│   ├── initAndon()              // ANDON 시스템 초기화
│   └── initMenu()               // 메뉴 트리 생성
└── for(;;)
    ├── currentSensorRoutine()       // 전류 센서 처리
    ├── OneMilliSecond_MainLoop()    // 1ms 폴링
    ├── OneSecond_MainLoop()         // 1s 폴링 (가동 시간 집계)
    ├── usbJsonParsorLoop()          // USB JSON 수신 처리
    ├── wifiLoop()                   // WiFi HTTP 통신 처리
    ├── [1s 이벤트]
    │   ├── WorkingTimeCount()       // 가동 시간 카운트
    │   ├── LED_OneSecondControl()   // LED 점멸
    │   └── TrimPin 상태 감지        // 트림 시간 측정
    ├── MenuLoop()                   // LCD UI 메뉴 처리
    └── CountFunc()                  // 카운팅 로직
```

### 4.2 ANDON 통신 흐름

```
[부팅 시 초기화 순서 — initAndon()]
1. get_dateTime      → RTC 동기화, 날짜 변경 시 카운트 자동 리셋
2. start             → 디바이스 등록, target/req_interval 수신 (핵심 설정)
3. get_andonList     → 작업 지시 리스트 조회
4. get_downtimeList  → 가동 중단 항목 리스트 조회
5. get_defectiveList → 불량 항목 리스트 조회

[운영 중 주기 요청]
makeAndonList()                    → req_interval 주기마다 작업 지시 갱신
makeAndonSewingCount()             → 재봉 카운트 전송
makeAndonPatternCount()            → 패턴 카운트 전송
updateRuntimeSum()                 → 가동 시간 합계 전송

[통신 경로]
ANDON 요청 → andonMessageQueue → wifiLoop()
→ wifi_cmd_http(url) → WiFi UART → HTTP GET
→ WiFi 수신 버퍼 → andonResponse()
→ andonJsonParsor() → 상태 갱신

[서버 엔드포인트]
GET http://{SERVER_IP}/{SERVER_PATH}/api/sewing.php?{params}
```

### 4.3 제품 카운팅 흐름

```
TrimPin 상승 엣지 (봉제기 트림 신호)
    └── ISR 처리
            ├── g_bStartTrimPin = TRUE
            └── 카운트 증가

[메인 루프 1s 이벤트]
    ├── g_bTrimElapsedTime++ (경과 시간 측정)
    └── TrimPin_Read() != 0 (핀 해제 감지)
            └── makeAndonSewingCount2(elapsed)  // 재봉 시간 포함 서버 전송

CountFunc() [메인 루프]
    ├── 패턴별 카운트 집계
    ├── 목표 대비 실적 계산
    └── internalFlash 저장
```

### 4.4 데이터 저장 구조

```
내부 EEPROM (Em_EEPROM)
└── INTERNAL_CONFIG
    ├── watermark [2 bytes]
    ├── data      [N bytes]: COUNT 구조체 (카운트, 가동 시간)
    └── checksum  [2 bytes]

외부 SPI Flash (W25Qxx)
└── EXTERNAL_CONFIG (최대 4KB)
    ├── watermark  [20 bytes] : "SUNTECH IOT"
    ├── data       [200 bytes]: CONFIG 구조체
    │   ├── Server_URL   [50]  : 서버 IP/도메인
    │   ├── Server_Path  [50]  : 서버 경로
    │   ├── SSID         [30]  : WiFi AP SSID
    │   ├── password     [30]  : WiFi 비밀번호
    │   ├── port         [2]   : 서버 포트 (기본 80)
    │   ├── reconnectTime[2]   : 재연결 주기
    │   └── deviceName   [30]  : 디바이스명
    ├── userData   [300 bytes]: MACHINE_PARAMETER (패턴, 재봉기 설정)
    └── CRC        [2 bytes]
```

### 4.5 JSON 통신 구조

**요청 예시**:
```
GET /api/sewing.php?code={device_code}&cmd=sewingCount&count={n}&elapsed={t}
```

**응답 파싱** (JSMN, 최대 80 토큰 — 10개 항목 불량 리스트 기준 73 토큰):
```json
{
  "result": "ok",
  "currentTime": "2025-11-17 10:30:00",
  "targetCount": 100,
  "andonList": [...]
}
```

### 4.6 메뉴 트리 구조

```
ROOT (TOP MENU)
├── 모니터링 화면          ← 기본 화면 (생산 현황 표시)
│   ├── 현재 카운트 / 목표
│   ├── WiFi 신호 강도
│   └── ANDON 상태
├── ANDON 메뉴            ← andonMenu.c
│   ├── 작업 지시 선택
│   ├── 가동 중단 등록
│   └── 불량 등록
├── SET 메뉴              ← userMenuPatternSewingMachine.c
│   ├── 서버 설정
│   ├── WiFi 설정
│   ├── 패턴 설정
│   └── 디바이스 정보
└── RESET 메뉴            ← resetMenu.c
    └── 카운터 초기화
```

---

## 5. 코드 규모 및 복잡도

| 파일 | 라인 수 | 비고 |
|------|---------|------|
| `lib/widget.c` | 1,089 | UI 위젯 전체 |
| `userMenuPatternSewingMachine.c` | 763 | 재봉기 메뉴 (이슈 #11 정리 완료) |
| `andonJson.c` | 616 | JSON 파싱 로직 |
| `lib/WIFI.c` | 506 | WiFi 통신 |
| `andonApi.c` | 266 | API 요청 생성 |
| `downtime.c` | 346 | 가동 중단 관리 |
| `defective.c` | 236 | 불량 관리 |
| `count.c` | 165 | 카운팅 로직 |
| `main.c` | 73 | 메인 루프 (간결) |

---

## 6. 프로젝트 특화 분석

### 6.1 조건부 컴파일 구조 (package.h)

```c
//#define PROJECT_NAME1
#define USER_PROJECT_PATTERN_SEWING_MACHINE   // ← 현재 활성 모드
//#define USE_CURRENT_SENSOR_FOR_COUNTTING    // 전류 센서 카운팅 (비활성)
```

| 매크로 | 역할 |
|--------|------|
| `USER_PROJECT_PATTERN_SEWING_MACHINE` | 패턴 재봉기 모드 활성화 |
| `USE_CURRENT_SENSOR_FOR_COUNTTING` | 전류 센서 기반 카운팅 (Port 5.5 사용 시) |

### 6.2 SRAM 최적화 현황

| 단계 | SRAM 사용량 | 사용률 | 상태 |
|------|-------------|--------|------|
| 최적화 전 | 31,612 bytes | 96.5% | 🔴 위험 |
| **1단계 최적화 후** | **21,620 bytes** | **66.0%** | ✅ 안정 |
| **WiFi 버퍼 확장 후** | **~22,644 bytes** | **~69.1%** | ✅ 안정 |
| 총 절감 | -8,968 bytes | -27.4% | |

주요 최적화/수정 내역:
- `lib/image.h` 13개 이미지 배열 `const` 추가 → FLASH 배치 (2026-03-09 완료)
- `MAX_NO_OF_ACCESS_POINT` 40 → 10 (2026-03-09 완료)
- `MAX_WIFI_RECEIVE_BUFFER` 1024 → 2048 — 실제 JSON 수신 크기(~1,087 bytes) 대응 (2026-03-09 완료)

### 6.3 WiFi/서버 설정 기본값

| 항목 | 기본값 | 파일 | 비고 |
|------|--------|------|------|
| 서버 호스트 | `49.247.26.228/CTP280_API` | `server.h` | 외부 플래시로 변경 가능 (대소문자 정확히) |
| 서버 포트 | `80` (HTTP) | `server.h` | HTTPS 전환 시 443 |
| API 엔드포인트 | `/api/sewing.php` | `server.h` | `DEFAULT_API_ENDPOINT` |
| WiFi SSID | `SUNTECH-CORING` | `server.h` | 외부 플래시로 변경 가능 |
| WiFi 강도 체크 | `60,000ms` | `WIFI.h` | `WIFI_STRENGTH_CHECK_TIME` |
| WiFi 최대 AP 수 | `10` | `WIFI.h` | 최적화 완료 (40→10) |
| WiFi 수신 버퍼 | `2048` bytes | `WIFI.h` | 수정 완료 (1024→2048) |

---

## 7. C# 설정 도구 (SuntechIoTConfig_V1)

| 항목 | 내용 |
|------|------|
| **프레임워크** | .NET Framework 4.7.2 |
| **UI** | Windows Forms |
| **통신** | SerialPort (COM 포트 자동 탐색) |
| **데이터** | JSON 기반 명령 송수신 |

### 주요 기능
1. COM 포트 자동 탐색 및 연결 (9600 / 115200 bps)
2. 서버 IP, 경로, 포트 설정 → 디바이스 외부 플래시에 저장
3. WiFi SSID / Password 설정
4. AT 명령어 직접 전송
5. 디바이스 정보 조회 (버전, 디바이스명 등)

---

## 8. 부트로더 구조

```
bootloader.cydsn/
├── Generated_Source/PSoC4/
│   ├── Bootloader.c           # 부트로더 메인
│   ├── UART.c                 # UART 통신
│   └── CyFlash.c              # Flash 쓰기

부트로더 프로토콜: UART 기반
업데이트 방법: UART 포트를 통한 펌웨어 다운로드
```

---

## 9. 발견된 이슈 및 개선 이력

### 9.1 미해결 이슈 (V1 기준)

| 우선순위 | 심각도 | 파일 | 이슈 | 위험 | 상태 |
|---------|--------|------|------|------|------|
| 1 | **높음** | `andonApi.c:46` | `sprintf(url, "%s?code=%s", ...)` — URL 인젝션 취약점, 크기 미제한 | 버퍼 오버플로우 | ✅ 수정 (snprintf) |
| 2 | **높음** | `uartJson.c` | `g_UART_buff[512]` 고정 버퍼, 오버플로우 경계 검사 없음 | 버퍼 오버플로우 | ✅ 수정 (경계 검사 추가) |
| 3 | **높음** | `andonJson.c:73` | `jsmntok_t t[128]` — 스택 2,048 bytes 전체 사용 (스택=2,048 bytes) | 스택 오버플로우 위험 | ✅ 수정 (t[128]→t[80]) |
| 4 | **높음** | `lib/WIFI.h`, `lib/config.h` | `MAX_WIFI_RECEIVE_BUFFER` 이중 정의 (1024/2048), 실제 JSON 크기 ~1,087 bytes 초과 | 데이터 잘림 | ✅ 수정 (→2048 통일) |
| 5 | **높음** | `server.h` | 서버 IP, 포트 하드코딩 (`192.168.38.72:80`) | 보안, 유지보수 | ✅ 수정 (현장 고정값 우선, 빈값이면 외부 플래시 사용) |
| 6 | **높음** | `lib/WIFI.c` | `while(g_wifi_cmd != WIFI_CMD_IDLE)` 타임아웃 없음 | 시스템 락업 | ✅ 수정 (wifi_cmd 3s 타임아웃, wifiLoop 감시 추가) |
| 7 | **중간** | `andonApi.h:118` | API URL `/api/sewing.php` 하드코딩 | 유지보수 불가 | ✅ 수정 (server.h DEFAULT_API_ENDPOINT로 이동) |
| 8 | **중간** | `lib/externalFlash.c` | `checkSum()` i=1 시작 — 첫 바이트 누락 | CRC 무결성 오류 | ✅ 수정 (i=0으로 변경) |
| 9 | **중간** | 전체 | 전역 변수 50+ 개 — 추적, 테스트 어려움 | 유지보수성 저하 | ✅ 수정 (파일 범위 static 전환 + volatile 추가) |
| 10 | **중간** | `downtime.c`, `defective.c` | JSON 파싱 로직 40% 중복 | 코드 중복 | ✅ 수정 (parseGenericList 공통 함수 추출 → jsonUtil.c) |
| 11 | **낮음** | `userMenuPatternSewingMachine.c` | 실제 931 라인 (README 오기) — 주석 코드/중복 include 제거 | 가독성 저하 | ✅ 수정 (불필요 코드 제거 → 763 라인) |
| 12 | **낮음** | `lib/WIFI.h` | `MAX_NO_OF_ACCESS_POINT 40` — 과도한 메모리 사용 | SRAM 낭비 | ✅ 수정 (→10) |
| 13 | **낮음** | `lib/image.h`, `lib/fonts.h` | `const` 미적용 시 SRAM 점유 (~14KB) | SRAM 낭비 | ✅ 수정 (const 추가) |
| 14 | **높음** | `lib/server.h` | `DEFAULT_SERVER_HOST` 소문자 경로 — Linux 서버 대소문자 구분으로 404 반환 | API 통신 전체 실패 | ✅ 수정 (`CTP280_API` 대소문자 수정) |
| 15 | **중간** | `andonApi.c` | `initAndon()` 순서 오류 — `start` 전에 `get_downtimeList/defectiveList` 호출 | `req_interval` 미수신으로 주기 설정 지연 | ✅ 수정 (순서 재정렬) |
| 16 | **중간** | `lib/WIFI.c` | MIB 타임아웃 후 `CMD 2 timeout` 반복 + WiFi 미연결 아이콘 표시 | LCD UI 오표시 | ✅ 수정 (수정 1+2 적용) |
| 17 | **높음** | `lib/WIFI.c` | OTA 응답이 `andonResponse()`로 전달되어 ANDON 상태 오염 → RSSI 미갱신 → WIFI INFO 미표시 | LCD WIFI INFO 화면 데이터 표시 불가 | ✅ 수정 (`g_wifi_cmd == WIFI_CMD_HTTP` 가드 추가) |
| 18 | **높음** | `lib/widget.h` | `IDX_SCROLL_UP = 6`이 메뉴 child index 6과 충돌 → OTA UPDATE 터치 시 스크롤 업 동작 | OTA 화면 진입 불가, 메뉴 리스트가 페이지 0으로 스크롤됨 | ✅ 수정 (`IDX_SCROLL_UP = 0xFD`, `IDX_SCROLL_DOWN = 0xFE`) |
| 19 | **높음** | `lib/WIFI.c` | `setCountMax_1ms()`가 `current=0`으로 설정 → 다음 `wifiLoop()`에서 `isFinishCounter_1ms()==TRUE` 즉시 반환 → OTA 버전 요청 직후 타임아웃 발동 | "OTA Error Bad version data" 즉시 표시, 서버 응답 무시 | ✅ 수정 (`_wifi_send_httpget()`, `_wifi_send_httpget_ota()`, `wifi_cmd()` 모두에 `resetCounter_1ms()` 추가) |

### 9.2 수정 이력

| 날짜 | 내용 |
|------|------|
| 2026-03-09 | `lib/image.h` 13개 배열 `const` 추가 — SRAM 절감 (96.5% → 66.0%) |
| 2026-03-09 | `lib/WIFI.h` `MAX_NO_OF_ACCESS_POINT` 40 → 10 — SRAM 1,800 bytes 절감 |
| 2026-03-09 | `lib/WIFI.h` `MAX_WIFI_RECEIVE_BUFFER` 1024 → 2048 — JSON ~1,087 bytes 수신 대응 |
| 2026-03-09 | `lib/config.h` 중복 `MAX_WIFI_RECEIVE_BUFFER` 정의 제거 |
| 2026-03-09 | `andonJson.c` 外 5개 파일 `jsmntok_t t[128]` → `t[80]` 수정 — 스택 768 bytes 여유 확보 |
| 2026-03-09 | `andonApi.c:46` `sprintf` → `snprintf` — URL 버퍼 오버플로우 방지 |
| 2026-03-09 | `uartJson.c:42` UART 버퍼 경계 검사 추가 — 오버플로우 방지 |
| 2026-03-09 | `lib/externalFlash.c` `checkSum()` `i=1` → `i=0` — CRC 첫 바이트 누락 수정 |
| 2026-03-10 | `server.h/c` 현장 고정값 우선 적용 — `DEFAULT_SSID/PASSWORD` 추가, 빈값이면 외부 플래시 폴백 |
| 2026-03-10 | `server.h` `IP[16]+path[32]` → `host[50]` 통합, `DEFAULT_API_ENDPOINT` 추가 (andonApi.h에서 이동) |
| 2026-03-10 | `WIFI.c` `wifi_cmd_http()` host 파싱 로직 추가, `USBJsonConfig.c` split 로직 제거 |
| 2026-03-10 | `WIFI.c` `wifi_cmd()` 모든 명령 3s 타임아웃, `wifiLoop()` 감시 루프, `wifi_printf()` TX 안전 카운터 추가 |
| 2026-03-10 | 이슈 #10 — `downTimeParsing()`/`defectiveParsing()` 중복 제거: `parseGenericList()` 공통 함수 추출 (`jsonUtil.c`), `GENERIC_LISTS`/`GENERIC_LIST_ITEM` 공통 구조체 추가 (`jsonUtil.h`), Flash ~2KB 절감 |
| 2026-03-10 | 이슈 #9 — 전역 변수 static 전환: `g_uAndonRequestType` 중복 선언 제거 + static, `g_updateTrimCount`/`g_uWorkingTimeCount`/`g_Count`/`g_uSectorForConfig`/`g_uAddressForConfig`/`g_uNoMiliSecondCounter`/`g_timerCounter_1ms`/`g_UART_buff`/`g_UART_buff_index`/`g_usbCmd`/`g_ConfigMeta`/`g_warning`/`g_bUpdateRuntime`/`g_updateTimeTime`/`eepromReturnValue`/`g_uSizeQueueANDON`/`g_uFrontQueueANDON`/`g_uRearQueueANDON`/`g_cQueue` → static 전환; `g_bStartTrimPin`/`g_bTrimElapsedTime` → volatile 추가 |
| 2026-03-10 | 이슈 #11 — `userMenuPatternSewingMachine.c` 정리: 중복 `count.h` include 제거, 주석 처리된 구버전 `doTargetInfoMenu` (130줄) 제거, 빈 switch 블록 정리 (931 → 763 라인) |
| 2026-03-10 | 이슈 #13 — `server.h` `DEFAULT_SERVER_HOST` 소문자 → 대소문자 수정 (`CTP280_API`), `andonApi.c` `initAndon()` API 순서 재정렬 (start 우선) |
| 2026-03-10 | 이슈 #14 — `WIFI.c` MIB 타임아웃 반복 방지 (수정 1) + IDLE 상태 MIB 응답 처리로 WiFi 미연결 아이콘 버그 수정 (수정 2) |
| 2026-03-09 | V1 최초 문서 분석 — 이슈 목록 작성 |

---

## 10. 테스트 시나리오

### 10.1 ANDON 통신 테스트

| 시나리오 | 절차 | 기대 결과 |
|---------|------|---------|
| 서버 연결 | 전원 ON → WiFi 연결 → 서버 요청 | ANDON Start 수신, 디바이스 등록 완료 |
| 작업 지시 조회 | makeAndonList() 자동 호출 | LCD에 작업 지시 리스트 표시 |
| 카운트 전송 | 트림 신호 인가 후 카운트 누적 | 서버에 재봉 카운트 자동 전송 |
| WiFi 단절 복구 | AP 연결 해제 → 재연결 | 60초 이내 자동 재연결 및 통신 재개 |

### 10.2 카운팅 테스트

| 시나리오 | 절차 | 기대 결과 |
|---------|------|---------|
| 트림 카운트 | TrimPin 상승 엣지 인가 | LCD 카운트 +1, EEPROM 저장 |
| 재봉 시간 측정 | TrimPin HIGH 유지 → LOW | 경과 시간 서버 전송 |
| 패턴 전환 | 새 패턴 선택 | 이전 패턴 데이터 저장, 새 패턴 카운트 초기화 |
| 전원 복원 | 임의 카운트 상태에서 OFF/ON | EEPROM에서 카운트 복원 |

### 10.3 LED 동작 테스트

| 시나리오 | 기대 결과 |
|---------|---------|
| 전원 ON | LED1 빨강 점멸 (초기화 중) |
| WiFi 연결 완료 | LED1 녹색 고정 |
| 서버 통신 중 | LED2 파랑 점멸 |
| 경고 상태 | LED 색상 변경 |

---

## 11. 개발 환경 설정

| 항목 | 내용 |
|------|------|
| **IDE** | PSoC Creator 4.x |
| **컴파일러** | ARM GCC 5.4.1 |
| **디버거** | MiniProg3 또는 KitProg |
| **플래시 도구** | PSoC Creator 내장 프로그래머 |
| **시리얼 모니터** | 115200 bps, 8N1 |
| **USB CDC** | Windows: usbser.sys 드라이버 |
| **설정 도구** | SuntechIoTConfig_V1 (C# Windows Forms) |

### 빌드 절차

1. PSoC Creator에서 `Design.cydsn` 워크스페이스 열기
2. `package.h`에서 활성 매크로 확인 (`USER_PROJECT_PATTERN_SEWING_MACHINE`)
3. Build → Generate Application
4. Build → Build `Design`
5. Program/Debug → Program

### 초기 설정 절차

1. `SuntechIoTConfig_V1` 실행
2. COM 포트 선택 후 연결 (115200 bps)
3. 서버 IP, 경로, 포트 입력 후 저장
4. WiFi SSID / Password 입력 후 저장
5. 디바이스 재시작 → 자동 서버 연결 확인

---

## 12. 버전 이력 테이블

| 날짜 | 버전 | 상태 | 변경 내용 |
|------|------|------|---------|
| 2025-11-17 | V1 | 운영 중 | 통합 버전 최초 구현 (IoT_SCI_2025.07.23 기반) |
| 2025-12-20 | V1 | 패치 | SRAM 최적화 — const 추가, 8,192 bytes 절감 |
| 2026-03-09 | V1 | 문서화 | 프로젝트 최초 문서 분석 및 이슈 목록 작성 |
| 2026-03-20 | - | 문서화 | OTA 웹 서버 가이드 신규 (`WEB/CTP280_OTA/`) |
| 2026-03-10 | V1 | 수정 | 이슈 #5~#16 전체 수정 (서버 설정, WiFi 타임아웃, MIB 버그 등) |
| 2026-03-17 | V1_BLACK_CPU | 신규 | PATTERN_MACHINE 전용 BLACK CPU 버전 분리 (SEWING/전류센서 제거) |
| 2026-03-19 | V2_BLACK_CPU | 신규 | OTA 무선 업데이트 기능 추가 (2-Stage 자동 체크 + 수동 메뉴) |
| 2026-03-20 | V2_BLACK_CPU | 버그수정 | WIFI INFO 미표시 (andonResponse 가드), OTA 먹통 (NULL→&g_ListMenu) 수정 |
| 2026-03-20 | V2_BLACK_CPU | 빌드완료 | OTA 부트로더 완성 (SPIM_FLASH+w25qxx 추가, CySysFlashWriteRow 수정, Placement 0x4200) — Flash 51.3% (134,408 bytes) / SRAM 70.1% (22,972 bytes) |
| 2026-03-20 | V2_BLACK_CPU | 버그수정 | OTA UPDATE 터치 시 스크롤 업 오동작 (widget.h IDX_SCROLL_UP=6 → 0xFD 충돌 수정), manageMenu.c otaUpdate 노드 위치 이동(index 6) |
| 2026-03-20 | V2_BLACK_CPU | 버그수정 | OTA 버전 요청 즉시 타임아웃 (setCountMax_1ms=0 → isFinishCounter 즉시 TRUE) — WIFI.c 3곳에 resetCounter_1ms 추가 |

---

## 13. 관련 파일 경로 빠른 참조

> 최신 버전(`V2_BLACK_CPU`) 기준. 동일 파일명이 V1/V1_BLACK_CPU에도 존재함.

| 파일 | 경로 |
|------|------|
| 메인 로직 | `280CTP_IoT_INTEGRATED_V2_BLACK_CPU/Project/Design.cydsn/main.c` |
| ANDON API | `280CTP_IoT_INTEGRATED_V2_BLACK_CPU/Project/Design.cydsn/andonApi.c` |
| ANDON JSON 파싱 | `280CTP_IoT_INTEGRATED_V2_BLACK_CPU/Project/Design.cydsn/andonJson.c` |
| WiFi 통신 | `280CTP_IoT_INTEGRATED_V2_BLACK_CPU/Project/Design.cydsn/lib/WIFI.c` |
| 서버 설정 | `280CTP_IoT_INTEGRATED_V2_BLACK_CPU/Project/Design.cydsn/lib/server.h` |
| **OTA 메뉴** | `280CTP_IoT_INTEGRATED_V2_BLACK_CPU/Project/Design.cydsn/otaMenu.c` |
| **OTA 헤더** | `280CTP_IoT_INTEGRATED_V2_BLACK_CPU/Project/Design.cydsn/otaMenu.h` |
| **OTA 웹 관리 페이지** | `C:\SUNTECH_DEV_CLAUDECODE\WEB\CTP280_OTA\CTP280_OTA_V1\index.html` |
| **OTA 웹 서버 가이드** | `C:\SUNTECH_DEV_CLAUDECODE\WEB\CTP280_OTA\CTP280_OTA_V1\README.md` |
| 카운팅 로직 | `280CTP_IoT_INTEGRATED_V2_BLACK_CPU/Project/Design.cydsn/count.c` |
| 가동 중단 | `280CTP_IoT_INTEGRATED_V2_BLACK_CPU/Project/Design.cydsn/downtime.c` |
| 불량 관리 | `280CTP_IoT_INTEGRATED_V2_BLACK_CPU/Project/Design.cydsn/defective.c` |
| 외부 Flash | `280CTP_IoT_INTEGRATED_V2_BLACK_CPU/Project/Design.cydsn/lib/externalFlash.c` |
| 내부 EEPROM | `280CTP_IoT_INTEGRATED_V2_BLACK_CPU/Project/Design.cydsn/lib/internalFlash.c` |
| 프로젝트 타입 | `280CTP_IoT_INTEGRATED_V2_BLACK_CPU/Project/Design.cydsn/package.h` |
| 설정 구조체 | `280CTP_IoT_INTEGRATED_V2_BLACK_CPU/Project/Design.cydsn/lib/config.h` |
| 설정 도구 | `280CTP_IoT_INTEGRATED_V2_BLACK_CPU/SuntechIoTConfig_V1/MainForm.cs` |
| **OTA 구현 가이드** | `OTA_IMPLEMENTATION_GUIDE.md` |

---

*Copyright SUNTECH, 2023-2026*
