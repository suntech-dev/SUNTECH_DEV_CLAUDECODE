# SMALL_BOARD_TABLET_SCANNER 프로젝트 분석 문서

> 최초 작성: 2026-03-04
> 분석 버전: SMALL_BOARD_TABLET_SCANNER_2026_V1
> 마지막 빌드: 2020-05-13 (성공)
> 마지막 코드 개선: 2026-03-04

---

## 1. 프로젝트 개요

이 프로젝트는 **Cypress PSoC 4** 기반의 **태블릿 스캐너 소형 통신 보드** 펌웨어입니다.
공장 자동화 생산 라인에서 **2D 바코드 스캐너**, **ST-500 Touch OP(HMI)**, **태블릿(Android OP)** 세 장치 간의 데이터를 중계하는 **통신 브리지(Communication Bridge)** 역할을 합니다.

> **⚠️ SMALL_BOARD_SCANNER와의 핵심 차이점**: 포트 역할이 반대입니다.
> - `MONITORING` 포트 → **태블릿** (JSON 출력)
> - `USB_OP` 포트 → **ST-500 Touch OP** (트리거 명령 수신 / DesignNo 송신)

### 주요 기능 요약

| 기능 | 설명 |
|------|------|
| 바코드 스캔 수신 | 2D 스캐너에서 바코드 데이터 수신 |
| 데이터 파싱 및 라우팅 | 바코드 데이터를 파싱해 Touch OP와 태블릿으로 각각 전송 |
| 스캔 트리거 명령 중계 | USB_OP(Touch OP)의 트리거 명령을 2D 스캐너에 전달 |
| 카운터 감지 | 외부 Counter 신호를 감지하여 JSON 카운트 이벤트 태블릿 전송 |
| UART 부트로더 | UART를 통한 OTA 방식 펌웨어 업그레이드 지원 |

---

## 2. 하드웨어 정보

| 항목 | 내용 |
|------|------|
| **MCU** | Cypress CY8C4245AZI-M445 |
| **코어** | ARM Cortex-M0 |
| **Flash** | 32,768 bytes (사용: 14,470 bytes / 44.2%) |
| **SRAM** | 4,096 bytes (사용: 2,660 bytes / 64.9%) |
| **패키지** | 64-TQFP |
| **개발툴** | PSoC Creator 4.4 |
| **컴파일러** | ARM GCC 5.4-2016-q2-update |

---

## 3. 프로젝트 구조

```
SMALL_BOARD_TABLET_SCANNER/
├── SMALL_BOARD_TABLET_SCANNER_2026_V1/
│   ├── bootloader.cydsn/           # PSoC 부트로더 프로젝트
│   │   ├── main.c                  # 부트로더 진입점
│   │   └── Generated_Source/       # PSoC Creator 자동 생성 코드
│   │
│   ├── MainFunction.cydsn/         # 메인 애플리케이션 프로젝트
│   │   ├── main.c                  # ★ 핵심 로직 (데이터 라우팅, 이벤트 처리)
│   │   ├── main.h                  # 전역 선언, 상수 정의, 펌웨어 버전
│   │   ├── common.c                # 초기화(init), 부트로더 진입 함수
│   │   ├── sysTick.c               # SysTick 타이머 (1ms 틱, LED 블링크)
│   │   ├── TopDesign/              # PSoC 스케매틱 (하드웨어 컴포넌트 배치)
│   │   ├── Generated_Source/PSoC4/ # PSoC Creator 자동 생성 드라이버
│   │   └── CortexM0/ARM_GCC_541/Debug/ # 빌드 출력물 (.elf, .map 등)
│   │
│   ├── Bootloader/                 # 펌웨어 업그레이드 도구 (PC용)
│   │   ├── UARTBootloaderHost.exe  # UART 부트로더 호스트 프로그램
│   │   ├── BootLoad_Utils.dll      # 의존 라이브러리
│   │   └── Prerequisites.txt       # 실행 요구사항 (.NET 4, MSVC 2010)
│   │
│   └── PatternBarcodeMonitoring.cywrk  # PSoC Creator 워크스페이스 파일
│
├── README.md
├── VERSION_HISTORY.md
└── CLAUDE.md
```

---

## 4. 시스템 아키텍처 (통신 구조)

```
┌─────────────────────────────────────────────────────────┐
│              태블릿 스캐너 통신 보드 (PSoC4)               │
│                                                         │
│  ┌─────────────┐   ┌─────────────┐   ┌───────────────┐ │
│  │  MONITORING │   │   USB_OP    │   │    BARCODE    │ │
│  │  (UART #1)  │   │  (UART #2)  │   │   (UART #3)   │ │
│  │  9600 bps   │   │  9600 bps   │   │   9600 bps    │ │
│  │   8N2       │   │    8N2      │   │     8N2       │ │
│  │ 인터럽트 RX  │   │  폴링 TX/RX │   │   폴링 RX     │ │
│  └──────┬──────┘   └──────┬──────┘   └───────┬───────┘ │
│         │                 │                   │         │
│  ┌──────┴──────┐   ┌──────┴──────┐   ┌───────┴───────┐ │
│  │   Counter   │   │Barcode_Triger│   │Pin_StartBoot  │ │
│  │ (디지털 입력) │   │ (디지털 입력) │   │  (디지털 입력) │ │
│  └─────────────┘   └─────────────┘   └───────────────┘ │
└─────────────────────────────────────────────────────────┘
         │                   │                   │
      태블릿 OP           ST-500 Touch OP      2D 스캐너
   (MONITORING Port)      (USB_OP Port)
```

### 포트별 역할

| 포트 | 연결 장치 | 방향 | 보드레이트 | 데이터 형식 |
|------|----------|------|-----------|------------|
| `MONITORING` | 태블릿 (Android) | 주로 TX | 9600 bps | 8N2, 인터럽트 기반 수신 |
| `USB_OP` | ST-500 Touch OP | 양방향 | 9600 bps | 8N2, JSON / 트리거 명령 |
| `BARCODE` | 2D 바코드 스캐너 | 양방향 | 9600 bps | 8N2, 전용 프로토콜 |

---

## 5. 소스코드 상세 분석

### 5.1 main.c - 핵심 로직

#### 전역 변수

| 변수 | 타입 | 설명 |
|------|------|------|
| `lastReceivedTime` | `unsigned long` | 마지막 데이터 수신 시각 (ms) |
| `receivedFromBarcode[256]` | `char[]` | 스캐너 수신 버퍼 (크기: 256) |
| `receivedFromBarcodeCount` | `char` | 수신 바이트 카운터 ⚠️ `uint8_t` 변경 필요 |
| `scanTrigerStart[3]` | `char[]` | 스캐너 트리거 명령 `{0x02, 0xF4, 0x03}` |
| `scanTrigerOrder[256]` | `char[]` | Touch OP 트리거 명령 문자열 ⚠️ `#define` 변경 필요 |
| `scanMode[256]` | `char[]` | Touch OP 스캔 모드 명령 문자열 ⚠️ `#define` 변경 필요 |
| `receivedScanTrigerOrder[256]` | `char[]` | Touch OP 수신 버퍼 |
| `receivedScanTrigerOrderCount` | `unsigned int` | Touch OP 수신 바이트 카운터 |

#### main.h 정의 상수 (현재)

| 상수 | 값 | 용도 |
|------|-----|------|
| `TYPE_OLD` | `1u` | 구형 OP 타입 (미사용) |
| `TYPE_NEW` | `0u` | 신형 OP 타입 (미사용) |
| `FIRMWARE_VERSION` | `"1.0.1"` | 펌웨어 버전 식별자 |

#### process_BARCODE(void) 함수

스캐너 UART 수신 버퍼에서 데이터를 읽어 `receivedFromBarcode` 버퍼에 누적합니다.
- 데이터 수신 시 `timerCount` 초기화 및 `lastReceivedTime` 갱신
- ⚠️ **버퍼 경계 검사 없음** — 오버플로우 가능성
- ⚠️ **`receivedFromBarcodeCount`가 `char` 타입** — 배열 인덱스 안전성 문제

#### process_USB_OP(void) 함수

ST-500 Touch OP로부터 명령을 수신하고 두 가지 명령을 처리합니다:
1. **트리거 명령** (`$$$$#99900035;%%%%`) → 2D 스캐너에 트리거 신호 전송 (`{0x02, 0xF4, 0x03}`)
2. **스캔 모드 조회** (`$$$$#99900304;%%%%`) → Touch OP로 스캐너 연결 확인 응답 반환
- ⚠️ **버퍼 경계 검사 없음** — 오버플로우 가능성
- ⚠️ **버퍼 미소비 시 무한루프 가능성** (RX 버퍼 항상 소비 보장 필요)

#### main() 메인 루프

```
1. Counter 신호 감지 (엣지 감지 방식)
   - 하강 엣지 (1→0): 타이머 초기화
   - 상승 엣지 (0→1): 시간 간격 계산 → 200ms~1900ms 범위이면 태블릿으로 카운트 JSON 전송

2. 타임아웃 처리 (timerCount - lastReceivedTime > 50)
   - Touch OP 수신 버퍼 초기화 (노이즈 데이터 폐기)
   - 스캐너 데이터가 있으면 파싱 후 라우팅:
     * ptrFirst (DesignNo) → ⚠️NULL 검사 없이 Touch OP 전송
     * ptrSecond/ptrThird (DesignIDX/Pieces) → ⚠️NULL 검사 없이 태블릿 JSON 전송
   - "SUNTECH" 바코드 스캔 시 부트로더 진입 (활성화 상태)

3. process_USB_OP() 호출
4. process_BARCODE() 호출
5. Pin_StartBootloader 감지 → 부트로더 진입
```

### 5.2 sysTick.c - 시스템 타이머

- `CySysTickStart()` 초기화 후 콜백 등록
- 1ms마다 `timerCount++`
- 1000ms마다 LED 토글 (상태 표시)

### 5.3 common.c - 초기화 및 유틸리티

#### init(void) 함수

```c
initSysTick();
MONITORING_Start();    // 태블릿 UART 시작
USB_OP_Start();        // ST-500 Touch OP UART 시작
BARCODE_Start();       // 스캐너 UART 시작
```

#### BootloaderStart(void) 함수

부트로더 진입 전 Monitoring 포트를 통해 현재 펌웨어 버전 및 업그레이드 준비 메시지 출력 후 `Bootloadable_Load()` 호출.

---

## 6. 통신 프로토콜 상세

### 6.1 바코드 데이터 형식

스캐너가 읽은 바코드는 슬래시(`/`)로 구분된 최대 3개의 필드:

```
{DesignNo}/{DesignIDX}/{Pieces}\r\n
```

| 필드 | 필수여부 | 전송 목적지 |
|------|---------|-----------|
| DesignNo | 필수 | ST-500 Touch OP (`USB_OP`) |
| DesignIDX | 선택 | 태블릿 (`MONITORING`, JSON) |
| Pieces | 선택 | 태블릿 (`MONITORING`, JSON) |

### 6.2 ST-500 Touch OP 전송 형식

```
@@@@!99900035;^^^^*P{DesignNo}*\r\n
```

### 6.3 태블릿 전송 형식 (JSON)

```json
// DesignIDX + Pieces 둘 다 있을 때
{"cmd" : "barcode", "value" : ["{DesignIDX}", "{Pieces}"]}

// DesignIDX만 있을 때
{"cmd" : "barcode", "value" : ["{DesignIDX}"]}

// 카운터 이벤트
{"cmd" : "count", "value" : 1}
```

### 6.4 스캐너 트리거 명령

| 방향 | 명령 | 내용 |
|------|------|------|
| Touch OP → 보드 | `$$$$#99900035;%%%%` | 스캔 트리거 요청 |
| 보드 → 스캐너 | `{0x02, 0xF4, 0x03}` | 스캔 실행 명령 |
| Touch OP → 보드 | `$$$$#99900304;%%%%` | 스캐너 연결 확인 |
| 보드 → Touch OP | `@@@@!99900304;&{FM100-M-R\|DD03AB95}^^^^` | 스캐너 확인 응답 |

---

## 7. 부트로더 구조

### 구조 (Bootloadable Architecture)

```
Flash Layout:
┌─────────────────────┐ 0x00000000
│   Bootloader        │ 6,272 bytes
│   (bootloader.cydsn)│
├─────────────────────┤
│   Application       │ 8,070 bytes
│ (MainFunction.cydsn)│
├─────────────────────┤
│   Metadata          │ 128 bytes
└─────────────────────┘ 0x00007FFF
```

### 부트로더 진입 조건

1. **내부 버튼**: `Pin_StartBootloader` 핀이 LOW일 때 자동 진입
2. **바코드 스캔**: "SUNTECH" 텍스트 스캔 시 진입 (현재 **활성화** 상태)

### 펌웨어 업그레이드 방법

1. PC에 .NET Framework 4, Visual C++ 2010 Redistributable 설치
2. `Bootloader/UARTBootloaderHost.exe` 실행
3. UART 포트 선택 (115200 bps)
4. 업로드용 `.cyacd` 파일 선택 및 업로드

---

## 8. 발견된 이슈 및 개선 이력

### 8.1 코드 버그 / 잠재적 위험 — ✅ 2026-03-04 전체 수정 완료

| 우선순위 | 파일 | 상태 | 내용 |
|---------|------|------|------|
| **HIGH** | main.c | ✅ 수정 완료 | `receivedFromBarcodeCount` `char` → `uint8_t` 변경. 배열 인덱스 타입 안전성 확보 |
| **HIGH** | main.c | ✅ 수정 완료 | `ptrFirst`, `ptrSecond`, `ptrThird` NULL 포인터 검사 추가. strtok NULL 반환 시 크래시 방지 |
| **HIGH** | main.c | ✅ 수정 완료 | UART RX 루프 내 항상 문자 소비 처리. 버퍼 미소비 시 무한루프 가능성 제거 |
| **HIGH** | main.c | ✅ 수정 완료 | 버퍼 경계 검사 추가 (`count < UART_BUF_SIZE - 1`). 버퍼 오버플로우 방어 |
| **MEDIUM** | main.c | ✅ 수정 완료 | 미사용 변수 `count`, `oldBarcodeTriger`, `oldBarcodeTrigerStat` 제거 |
| **MEDIUM** | main.c | ✅ 수정 완료 | `scanTrigerOrder`, `scanMode` 전역 배열 → `#define` 상수(`SCAN_TRIGER_ORDER_STR`, `SCAN_MODE_CMD_STR`)로 대체 |
| **MEDIUM** | main.c | ✅ 수정 완료 | 매직 넘버 `256`, `50`, `200`, `1900` → `UART_BUF_SIZE`, `TIMEOUT_MS_UART`, `COUNTER_MIN_MS`, `COUNTER_MAX_MS` 상수화 |
| **LOW** | 전체 | ✅ 수정 완료 | Copyright `YOUR COMPANY / THE YEAR` → `SUNTECH, 2018-2026` 수정 |

### 8.2 컴파일러 경고 — ✅ 전체 해소

```
[수정 전 경고]
main.c:41: warning: array subscript has type 'char' [-Wchar-subscripts]   → uint8_t 변경으로 해소
main.c:127: warning: array subscript has type 'char' [-Wchar-subscripts]  → uint8_t 변경으로 해소
main.c:89: warning: unused variable 'count' [-Wunused-variable]           → 변수 제거로 해소
main.c:88: warning: unused variable 'oldBarcodeTriger' [-Wunused-variable] → 변수 제거로 해소
```

### 8.3 아키텍처 개선 — ✅ 완료 / 🔲 향후 검토

| 항목 | 상태 | 내용 |
|------|------|------|
| 타임아웃 상수화 | ✅ 완료 | `50`, `200`, `1900` → `TIMEOUT_MS_UART`, `COUNTER_MIN_MS`, `COUNTER_MAX_MS` |
| 버퍼 크기 상수화 | ✅ 완료 | `256` 반복 사용 → `#define UART_BUF_SIZE (256u)` |
| 명령 문자열 상수화 | ✅ 완료 | 하드코딩 문자열 → `#define SCAN_TRIGER_ORDER_STR` / `SCAN_MODE_CMD_STR` |
| 수신 상태 머신(FSM) | 🔲 향후 검토 | 폴링 방식 → 인터럽트 기반 FSM 전환 시 신뢰성 향상 가능 |

---

## 9. 개발 환경 설정

### 필수 소프트웨어

- **Cypress PSoC Creator 4.4** (PSoC 프로젝트 편집 및 빌드)
- **ARM GCC 5.4-2016-q2-update** (PSoC Creator에 번들됨)
- **PSoC Programmer** 또는 **MiniProg3** (초기 플래싱용)

### 빌드 방법

1. PSoC Creator 실행
2. `SMALL_BOARD_TABLET_SCANNER_2026_V1/PatternBarcodeMonitoring.cywrk` 워크스페이스 열기
3. `bootloader` 프로젝트 먼저 빌드
4. `MainFunction` 프로젝트 빌드
5. `.cyacd` 파일 생성 확인 (`Export/` 폴더)

### UART 설정 (통신 포트 연결 시)

| 포트 | 보드레이트 | 데이터비트 | 패리티 | 스톱비트 |
|------|-----------|----------|-------|---------|
| MONITORING | 9,600 | 8 | None | 2 |
| USB_OP | 9,600 | 8 | None | 2 |
| BARCODE | 9,600 | 8 | None | 2 |
| 부트로더 업그레이드 | 115,200 | 8 | None | 1 |

---

## 10. 버전 이력

| 날짜 | 내용 |
|------|------|
| 2018-03-12 | Bootloader 적용 |
| 2018-05-14 | 2D Scanner + Touch OP 통합. 태블릿→Monitoring Port, Touch OP→USB_OP Port 구조 확립 |
| 2018-05-25 | Scanner Mode Enable 추가 |
| 2019-04-20 | PWJ 라인 적용 |
| 2020-05-13 | 마지막 빌드 성공 확인 (PSoC Creator 4.4) |
| 2026-03-04 | 코드 품질 개선: 타입 안전성(uint8_t), NULL 포인터 방어, 미사용 변수 제거, 상수 정의 분리, 버퍼 오버플로우 방어, Copyright 수정 |

---

## 11. 관련 파일 경로 빠른 참조

| 파일 | 경로 | 설명 |
|------|------|------|
| 핵심 로직 | `SMALL_BOARD_TABLET_SCANNER_2026_V1/MainFunction.cydsn/main.c` | 메인 루프 및 데이터 처리 |
| 헤더/상수 | `SMALL_BOARD_TABLET_SCANNER_2026_V1/MainFunction.cydsn/main.h` | 전역 선언, 상수 정의, 버전 |
| 초기화 | `SMALL_BOARD_TABLET_SCANNER_2026_V1/MainFunction.cydsn/common.c` | 하드웨어 초기화 |
| 타이머 | `SMALL_BOARD_TABLET_SCANNER_2026_V1/MainFunction.cydsn/sysTick.c` | 1ms SysTick 콜백 |
| 부트로더 main | `SMALL_BOARD_TABLET_SCANNER_2026_V1/bootloader.cydsn/main.c` | 부트로더 진입점 |
| 업로더 도구 | `SMALL_BOARD_TABLET_SCANNER_2026_V1/Bootloader/UARTBootloaderHost.exe` | PC 업로드 프로그램 |
