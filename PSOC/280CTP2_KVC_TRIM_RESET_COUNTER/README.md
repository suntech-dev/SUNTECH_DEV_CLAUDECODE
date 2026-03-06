# 280CTP2_KVC_TRIM_RESET_COUNTER 프로젝트 분석 문서

> 최초 작성: 2026-03-05
> 분석 버전: V1 (초기 구현) / V2 (최종 최적화)
> 마지막 코드 개선: 2025-12-30 (V2)
> 마지막 업데이트: 2026-03-05

---

## 1. 프로젝트 개요

이 프로젝트는 **Cypress PSoC 4** 기반의 **생산 카운터 관리 시스템** 펌웨어입니다.
공장 자동화 생산 라인에서 **Trim Count 목표(Target) 달성 여부를 관리**하고, **부정 리셋 방지** 기능을 제공합니다.
LCD 터치 디스플레이와 물리적 RESET 버튼을 통해 작업자가 생산량을 추적하고, 목표 달성 시에만 카운터를 초기화할 수 있도록 보안이 강화된 시스템입니다.

### 주요 기능 요약

| 기능 | 설명 |
|------|------|
| Trim Count 추적 | 외부 TC_INT 신호를 감지하여 카운트 자동 증가 |
| 목표(Target) 관리 | SET 메뉴를 통한 Trim Target 설정 |
| 부정 리셋 방지 | Target 도달 시에만 RESET 버튼 동작 (V1→V2 핵심 개선) |
| 경고 메시지 | Target 미달성 시 LCD 경고 + 부저 알림 |
| USB 부트로더 | SET 메뉴를 통한 USB HID 방식 펌웨어 업그레이드 |
| SRAM 최적화 | WiFi/서버/JSON 기능 제거로 메모리 41.7% 절감 (V2) |

---

## 2. 버전 구조

```
280CTP2_KVC_TRIM_RESET_COUNTER/
├── 280CTP2_KVC_TRIM_RESET_COUNTER_V1/    # V1 - 초기 구현 (2025.12.25)
│   ├── Design.cydsn/                      # 메인 애플리케이션
│   │   ├── count.c                        # RESET 로직 (Phase 1, 2)
│   │   ├── main.c                         # 메인 루프
│   │   ├── userMenuCounter.c              # UI 메뉴
│   │   ├── lib/widget.c                   # LCD UI 위젯 (WiFi 포함)
│   │   ├── lib/button.c                   # 버튼 렌더링 (16글자 제한)
│   │   ├── jsmn-master/                   # JSON 라이브러리 (V1만 존재)
│   │   └── Generated_Source/PSoC4/        # PSoC Creator 자동 생성 코드
│   ├── bootloader.cydsn/                  # USB 부트로더
│   └── 개선사항.md                         # 개선사항 문서 v1.0
│
├── 280CTP2_KVC_TRIM_RESET_COUNTER_V2/    # V2 - 완전한 최적화 (2025.12.30)
│   ├── Design.cydsn/                      # 메인 애플리케이션
│   │   ├── count.c                        # RESET 로직 최종 버전
│   │   ├── main.c                         # 메인 루프 + 부트로더 함수
│   │   ├── userMenuCounter.c              # UI 메뉴 (3단계 계층 구조)
│   │   ├── lib/widget.c                   # LCD UI 위젯 (WiFi 제거됨)
│   │   ├── lib/button.c                   # 버튼 렌더링 (24글자 제한)
│   │   └── Generated_Source/PSoC4/        # PSoC Creator 자동 생성 코드
│   ├── bootloader.cydsn/                  # USB 부트로더
│   ├── 개선사항.md                         # 개선사항 문서 v1.3
│   ├── SRAM_최적화_분석보고서.md           # SRAM 최적화 결과 보고서
│   └── Trim_Reset_최적화(...).md          # SRAM 최적화 계획 문서
│
├── README.md                              # 이 문서
└── README.html                            # HTML 버전 문서
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
| **LCD** | ST7789V (320x240, SPI 인터페이스) |
| **터치** | I2C_TC (I2C 기반 터치 컨트롤러) |

### 하드웨어 컴포넌트

| 컴포넌트 | 역할 |
|---------|------|
| `TC_INT` | Trim Count 신호 입력 (외부 카운터) |
| `TC_RESET` | 물리적 RESET 버튼 (LOW = 누름) |
| `BUZZER` | 알림 및 경고음 출력 |
| `SPIM_LCD` | ST7789V LCD SPI 인터페이스 |
| `I2C_TC` | 터치 컨트롤러 I2C |
| `UART` | 디버그/통신용 |
| `RESERVED_OUT_1` | Trim 완료 외부 신호 출력 |

---

## 4. 버전 비교 (V1 vs V2)

### 4.1 핵심 기능 비교

| 기능 | V1 (2025.12.25) | V2 (2025.12.30) |
|------|----------------|----------------|
| **RESET 조건** | `count != 0` (취약) | `count == setTrimCount` (보안 강화) |
| **경고 메시지** | "Target Not Reached!\n%d/%d" | "Not Reached! %d/%d" (간소화) |
| **메시지 표시 시간** | 1.5초 | 2초 |
| **경고음 정지** | 없음 (계속 울림) | 2초 후 자동 정지 |
| **화면 복원** | 버튼 값만 업데이트 | 전체 화면 갱신 (2단계 모드) |
| **LCD 타이틀 바** | WiFi 영역 포함 (256px) | 전체 너비 사용 (319px, +25%) |
| **버튼 텍스트 제한** | 16글자 | 24글자 (+50%) |
| **부트로더 진입** | - | SET 메뉴 기반 (2단계 확인) |
| **메뉴 구조** | 단일 레벨 | 3단계 계층 구조 |
| **WiFi 기능** | 포함 | 완전 제거 |
| **JSON 파서** | jsmn 라이브러리 포함 | 완전 제거 |
| **ShowMessage 버퍼** | `char buff[20]` | `char buff[64]` (3.2배) |

### 4.2 메모리 사용량 비교

| 항목 | V1 (최적화 전) | V2 (최적화 후) | 절감량 |
|------|--------------|--------------|-------|
| **SRAM** | 19,788 bytes (60.4%) | 11,532 bytes (35.2%) | **8,256 bytes (-41.7%)** |
| **Flash** | 94,468 bytes (36.0%) | 77,968 bytes (29.7%) | **16,500 bytes (-17.5%)** |
| **SRAM 여유** | 12,980 bytes | 21,236 bytes | +8,256 bytes |

### 4.3 코드 파일 비교

| 구분 | V1 | V2 |
|------|----|----|
| **jsmn-master/** | 포함 (JSON 라이브러리) | 제거됨 |
| **WiFi 관련 사용자 파일** | lib/WIFI.c 등 포함 | 17개 파일 제거됨 |
| **Generated WiFi 파일** | WIFI*.c/h 포함 | 하드웨어 컴포넌트 미제거 (선택사항) |
| **userMenuCounter.c** | 단일 레벨 메뉴 | 3단계 계층 메뉴 추가 |
| **main.c** | 기본 루프 | `EnterBootloaderMode()` 함수 추가 |

---

## 5. 소스코드 상세 분석

### 5.1 count.c - RESET 로직 (핵심)

#### V1 RESET 로직

```c
// V1 - 취약한 조건: 카운트가 0이 아니면 언제든 리셋 가능
if(RESET_KEY_Read() == FALSE && g_ptrCount->count != 0)
{
    CyDelay(50);
    if(RESET_KEY_Read() == FALSE)
    {
        g_ptrCount->count = 0;
        SaveInternalFlash();
        g_updateCountMenu = TRUE;
        bTrimCountComplete = FALSE;
        RESERVED_OUT_1_Write(0);
        Buzzer(BUZZER_STOP, 0);
        return;
    }
}
```

#### V2 RESET 로직 (보안 강화)

```c
// V2 - 강화된 조건: Target 도달 시에만 리셋 가능
if(RESET_KEY_Read() == FALSE &&
   g_ptrCount->count == g_ptrMachineParameter->setTrimCount)
{
    CyDelay(50);
    if(RESET_KEY_Read() == FALSE)
    {
        g_ptrCount->count = 0;
        SaveInternalFlash();
        g_updateCountMenu = TRUE;
        bTrimCountComplete = FALSE;
        RESERVED_OUT_1_Write(0);
        Buzzer(BUZZER_STOP, 0);
        return;
    }
}
// V2 추가 - Target 미달성 시 경고
else if(RESET_KEY_Read() == FALSE &&
        g_ptrCount->count > 0 &&
        g_ptrCount->count < g_ptrMachineParameter->setTrimCount)
{
    CyDelay(50);
    if(RESET_KEY_Read() == FALSE)
    {
        Buzzer(BUZZER_WARNING, 100);               // 경고음 시작
        ShowMessage("Not Reached! %d/%d",           // LCD 경고 메시지
                    g_ptrCount->count,
                    g_ptrMachineParameter->setTrimCount);
        CyDelay(2000);                              // 2초 대기
        Buzzer(BUZZER_STOP, 0);                     // 경고음 정지
        EraseBlankAreaWithoutHeader();              // 화면 지우기
        g_updateCountMenu = 2;                      // 전체 화면 갱신 요청
    }
}
```

### 5.2 userMenuCounter.c - 메뉴 시스템

#### V2 메뉴 계층 구조

```
TOP MENU (doTopMenu)
├── Count 화면 (기본)
├── SET Button 터치
│   └── SET Submenu (doSetSubmenu)
│       ├── Set Target [BLUE] → doTargetInfoMenu
│       ├── USB Update [RED]  → doUSBUpdate
│       │   └── 확인 화면: "Going to UPDATE?"
│       │       ├── CANCEL [GREEN] → 서브메뉴 복귀
│       │       └── UPDATE [RED]   → EnterBootloaderMode()
│       └── EXIT [GREEN] → 메인 화면 복귀
└── Unlock with Key (doUnlockWithKey)
```

#### V2 갱신 모드 2단계 시스템

| 모드 | 값 | 동작 |
|------|-----|------|
| 일반 갱신 | `TRUE (1)` | 버튼 값만 업데이트 (count, target 표시) |
| 전체 갱신 | `2` | `DisplayDoTopMenu()` 전체 화면 재그리기 |

### 5.3 lib/widget.c - LCD UI 위젯

#### WiFi 비활성화 (V1 → V2)

```c
// V2 - initWidget() 함수
void initWidget()
{
    /* WiFi 기능 비활성화 - TitleBar 전체 영역 사용을 위해 제거
    g_wifi_strength.rect.right = g_SCREEN_WIDTH-1;
    g_wifi_strength.rect.left  = g_wifi_strength.rect.right - 30;
    ...
    */

    // title Bar - 전체 화면 너비 사용
    g_TitleBar.rect.right = g_SCREEN_WIDTH - 1;  // 256px → 319px (+25%)
    g_TitleBar.rect.left  = 0;
}

void DrawHeader()
{
    // DrawWifi();  // V2: WiFi 비활성화
    DrawTitle();
    DrawHorizontalLine(0, g_SCREEN_WIDTH-1, DEFAULT_TOP_TITLE_HEIGHT-1, WHITE);
}
```

#### ShowMessage 버퍼 개선 (V1 → V2)

```c
// V1 - 버퍼 오버플로우 가능
void ShowMessage(const char *msg, ...) {
    char buff[20];  // "Target Not Reached!\n50/100" = 27자 → 오버플로우!
    ...
}

// V2 - 충분한 버퍼
void ShowMessage(const char *msg, ...) {
    char buff[64];  // 여유 있는 크기
    ...
}
```

### 5.4 lib/button.c - 버튼 텍스트 제한

```c
// V1: 16글자 제한 → "KVC Trim Reset V2" (18글자) 표시 불가
#define MAX_NUM_BUTTON_STRING 16

// V2: 24글자 제한 → 18글자 정상 표시
#define MAX_NUM_BUTTON_STRING 24
```

### 5.5 main.c - 부트로더 진입 (V2 추가)

```c
// V2 추가 - EnterBootloaderMode() 함수
void EnterBootloaderMode(void)
{
    printf("Entering Bootloader Mode...\r\n");
    Bootloadable_Load();    // 부트로더 메타데이터 로드
    CySoftwareReset();      // 소프트웨어 리셋 → 부트로더 모드 진입
}
```

---

## 6. V2 SRAM 최적화 상세

### 6.1 제거된 항목

| 항목 | 절감량 |
|------|-------|
| WiFi 이미지 데이터 (image_wifi_0~4) | 약 8KB |
| WiFi 수신 버퍼 (MAX_WIFI_RECEIVE_BUFFER) | 2KB |
| JSON 파서 버퍼 및 토큰 배열 | 1~2KB |
| 서버 설정 구조체 (SERVER_INFO) | 약 400 bytes |
| Access Point 목록 (g_APs[40]) | 약 2KB |
| 기타 WiFi/서버 관련 전역 변수 | 약 500 bytes |

### 6.2 제거된 파일 목록 (17개 파일 + 1개 디렉토리)

**사용자 작성 파일:**
- `lib/WIFI.c`, `lib/WIFI.h`
- `lib/server.c`, `lib/server.h`
- `lib/jsmn.c`, `lib/jsmn.h`
- `USBJsonConfig.c`, `USBJsonConfig.h`
- `USBUARTConfig.c`, `USBUARTConfig.h`
- `jsonUtil.c`, `jsonUtil.h`
- `uartJson.c`, `uartJson.h`
- `andonJson.c`, `andonJson.h`
- `andonApi.c`, `andonApi.h`
- `jsmn-master/` (디렉토리 전체)

### 6.3 최적화 결과

| 단계 | SRAM 사용량 | 사용률 | 상태 |
|------|------------|--------|------|
| V1 (최적화 전) | 19,788 bytes | 60.4% | 주의 |
| V2 (최적화 후) | 11,532 bytes | 35.2% | 안정 |
| **절감량** | **8,256 bytes** | **-25.2%p** | 41.7% 감소 |

---

## 7. 부트로더 구조

### Flash 레이아웃

```
Flash Layout:
┌─────────────────────┐ 0x00000000
│   Bootloader        │ 13,312 bytes
│   (bootloader.cydsn)│
├─────────────────────┤
│   Application       │ 64,400 bytes (V2)
│   (Design.cydsn)    │ 80,900 bytes (V1)
├─────────────────────┤
│   Metadata          │ 256 bytes
└─────────────────────┘ 0x0003FFFF
```

### 부트로더 진입 방법

**V1:**
- 별도 진입 방법 없음 (개발 중)

**V2:**
1. LCD 화면에서 **SET 버튼** 터치
2. **USB Update (RED 버튼)** 선택
3. 확인 화면에서 **UPDATE (RED 버튼)** 확인
4. `EnterBootloaderMode()` → `Bootloadable_Load()` → `CySoftwareReset()` 순서로 실행
5. USB HID 디바이스로 재연결

### 펌웨어 업그레이드 방법 (V2)

1. 장치에서 부트로더 모드 진입 (위 순서)
2. PC에서 `USBBootloaderHost.exe` 실행
3. [Connect] 클릭 → USB HID 연결 확인
4. [Open] → `design.cyacd` 파일 선택
5. [Update] → 펌웨어 전송 시작
6. 업데이트 완료 후 자동 재시작

---

## 8. 발견된 이슈 및 개선 이력

### 8.1 V1 발견 문제점

| 우선순위 | 파일 | 문제 |
|---------|------|------|
| **HIGH** | count.c | RESET 조건 취약 (`count != 0`으로 언제든 리셋 가능) |
| **HIGH** | count.c | Target 미달성 시 RESET 눌러도 아무 피드백 없음 |
| **HIGH** | lib/widget.c | ShowMessage 버퍼 부족 (`char buff[20]` → 오버플로우) |
| **MEDIUM** | lib/button.c | 버튼 텍스트 16글자 제한 ("KVC Trim Reset V2" 표시 불가) |
| **MEDIUM** | lib/widget.c | WiFi 영역이 타이틀 바 25% 차지 (불필요한 낭비) |
| **LOW** | count.c | 경고음이 자동 정지 안 됨 |
| **LOW** | userMenuCounter.c | 경고 메시지 후 화면 잔상 남음 |

### 8.2 V2 수정 내역 (Phase별)

| Phase | 수정 파일 | 내용 |
|-------|---------|------|
| **Phase 1** | count.c | RESET 조건 강화 (`count == setTrimCount`) |
| **Phase 2** | count.c, lib/widget.h | LCD 경고 메시지 + 경고음 추가 |
| **Phase 3** | lib/widget.c | WiFi 비활성화, TitleBar 전체 너비 사용 |
| **Phase 4** | lib/widget.c, count.c, userMenuCounter.c | 메시지 버퍼 증가, 경고음 정지, 화면 복원 |
| **Phase 5** | main.c, main.h, userMenuCounter.c | SET 메뉴 기반 부트로더 진입 시스템 |
| **Phase 6** | userMenuCounter.c | UI 개선 (버튼 크기, 텍스트 정렬, 아웃라인) |
| **SRAM 최적화** | 17개 파일 삭제, main.c, setup.c 등 | WiFi/서버/JSON 완전 제거 |

### 8.3 V2 향후 고려사항

| 항목 | 상태 | 내용 |
|------|------|------|
| PSoC Creator 하드웨어 컴포넌트 제거 | 선택사항 | UART_WIFI, WIFI 컴포넌트 제거 (SRAM 영향 없음) |
| 부정 리셋 시도 횟수 로깅 | 미구현 | Flash에 시도 횟수 기록 |
| 관리자 강제 리셋 기능 | 미구현 | 비밀번호 입력 후 강제 리셋 허용 |

---

## 9. 테스트 시나리오

### 테스트 환경

- **Target 설정**: 100
- **테스트 장비**: PSoC4 개발 보드 + LCD 터치 디스플레이
- **측정 항목**: LCD 표시, 부저 동작, 카운트 값, SRAM 사용량

### 주요 테스트 케이스

| 테스트 | 조건 | 기대 결과 (V2) |
|-------|------|--------------|
| **TC1** 정상 리셋 | count=100, Target=100, RESET 누름 | 카운트 0 리셋, 부저 정지, Flash 저장 |
| **TC2** 미달성 리셋 | count=50, Target=100, RESET 누름 | 카운트 유지, LCD "Not Reached! 50/100", 경고음 2초 |
| **TC3** 초기 상태 리셋 | count=0, RESET 누름 | 무반응 (아무 동작 없음) |
| **TC4** 연속 동작 | count 100→RESET→50→RESET시도→100→RESET | 순서대로 동작 |
| **TC5** 부트로더 진입 | SET → USB Update → UPDATE | 부트로더 모드 진입 |
| **TC6** 타이틀 표시 | 전원 ON | "KVC Trim Reset V2.1" 완전 표시 |

---

## 10. 개발 환경 설정

### 필수 소프트웨어

- **Cypress PSoC Creator 4.x** (PSoC 프로젝트 편집 및 빌드)
- **ARM GCC 5.4-2016-q2-update** (PSoC Creator에 번들됨)
- **PSoC Programmer** 또는 **MiniProg3** (초기 플래싱용)

### 빌드 방법

1. PSoC Creator 실행
2. `280CTP2_KVC_TRIM_RESET_COUNTER_V2/` 폴더 내 워크스페이스 열기
3. `bootloader` 프로젝트 먼저 빌드
4. `Design` 프로젝트 빌드
5. `.cyacd` 파일 생성 확인

### 권장 버전 (V2)

- V1은 보안 취약점(부정 리셋)이 있어 **운영 환경에서는 V2 사용 권장**
- V2는 모든 Phase 완료 및 기능 테스트 확인됨

---

## 11. 버전 이력

| 날짜 | 버전 | 내용 |
|------|------|------|
| 2025-12-25 | V1 (개선사항 v1.0) | Phase 1: RESET 조건 강화, Phase 2: 경고 메시지 추가 |
| 2025-12-29 | V2 시작 | Phase 3~4: 타이틀 바 확장, 메시지 시스템 개선 |
| 2025-12-29 | V2 SRAM 최적화 | WiFi/서버/JSON 완전 제거, SRAM 41.7% 절감 |
| 2025-12-30 | V2 (개선사항 v1.3) | Phase 5~6: SET 메뉴 부트로더, UI 개선 |
| 2026-03-05 | 문서 작성 | 전체 프로젝트 분석 문서 생성 |

---

## 12. 관련 파일 경로 빠른 참조

| 파일 | 경로 | 설명 |
|------|------|------|
| 핵심 RESET 로직 | `V2/Design.cydsn/count.c` | 부정 리셋 방지 + 경고 메시지 |
| 메인 루프 | `V2/Design.cydsn/main.c` | 초기화, 부트로더 진입 함수 |
| UI 메뉴 | `V2/Design.cydsn/userMenuCounter.c` | 3단계 계층 메뉴 |
| LCD 위젯 | `V2/Design.cydsn/lib/widget.c` | ShowMessage, WiFi 비활성화 |
| 버튼 렌더링 | `V2/Design.cydsn/lib/button.c` | 텍스트 길이 24글자 제한 |
| V2 개선사항 | `V2/개선사항.md` | Phase 1~6 상세 구현 내용 |
| SRAM 최적화 | `V2/SRAM_최적화_분석보고서.md` | 메모리 최적화 결과 |
| V1 개선사항 | `V1/개선사항.md` | Phase 1~2 초기 구현 |
