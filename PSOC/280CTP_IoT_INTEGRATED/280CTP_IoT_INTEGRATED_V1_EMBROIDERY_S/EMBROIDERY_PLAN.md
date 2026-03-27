# EMBROIDERY_PLAN.md
> 작성일: 2026-03-26
> 마지막 업데이트: 2026-03-27 (LCD UART TEST 헤더 타이틀 잔류 버그 수정 — DrawHeader() 복원)
> 기준 버전: 280CTP_IoT_INTEGRATED_V1_BLACK_CPU → V1_EMBROIDERY_S 파생
> 목적: 컴퓨터자수기(Computer Embroidery Machine) 전용 펌웨어 적응 계획
> 상태: **구현 완료 + 버그 수정 완료** ✓

---

## 1. 배경 및 목표

`280CTP_IoT_INTEGRATED_V1_EMBROIDERY_S`는 `BLACK_CPU` 버전의 복사본을 기반으로,
**UART 신호 포맷**과 **수집 데이터 항목**을 자수기에 맞게 변경하는 프로젝트이다.

하드웨어(PCB, PSoC4, LCD, WiFi, BUZZER 등)는 동일하게 유지된다.

---

## 2. UART 프로토콜 차이

| 항목 | BLACK_CPU | EMBROIDERY_S |
|------|-----------|--------------|
| 포맷 | JSON 오브젝트 `{...}` | 세미콜론 구분 텍스트 |
| 예시 | `{"cmd":"count","value":1,"ct":300,...}` | `1;300;200;300;` |
| 종료 조건 | `}` 문자 | 4번째 `;` 문자 |
| 파서 | `jsmn` 라이브러리 기반 | 직접 구현 — `strtok` 또는 `;` 카운터 방식 |

### 2-1. 자수기 데이터 필드 정의

```
순서;값의_의미 → 파라메타명 (타입, 최대값)
──────────────────────────────────────────
1번 ; 생산수량           → actual_qty           (uint16, max 10000)
2번 ; 싸이클타임(ms)     → cycle_time           (uint32, max 3600000)
3번 ; 실끊어진횟수       → thread_breakage_qty  (uint16, max 1000)
4번 ; 실제자수기동작시간  → motor_runtime        (uint32, max 3600000)
```

> **주의**: `cycle_time`과 `motor_runtime`의 최대값 3,600,000은 uint16 범위(65535)를 초과한다.
> 기존 COUNT 구조체 패턴(H/L 분리 또는 별도 uint32 누산 필드)을 그대로 따라야 한다.

---

## 3. 수정 대상 파일 목록

| 파일 | 변경 규모 | 변경 이유 |
|------|-----------|-----------|
| `uartJson.c` | **대규모** | JSON → 세미콜론 파서 전면 교체 |
| `count.h` | 보통 | `embThreadBreakageQty` 필드 추가 |
| `count.c` | 소규모 | `ResetCount()` 에 새 필드 초기화 추가 |
| `package.h` | 소규모 | 펌웨어 버전명 변경 |
| `userMenuPatternSewingMachine.c` | 보통 | LCD Page 구조 변경 (Page 2 삭제, Page 1 항목 수정) |
| `andonApi.c` | 소규모 | `makeAndonPatternCount()` 전송 파라메타 수정 |
| `uartTestMenu.c` (**신규**) | 신규 생성 | UART TEST 뷰어 — LCD 표시 전용 |
| `uartTestMenu.h` (**신규**) | 신규 생성 | `uartTestAddLine()` / `doUartTestMenu()` 선언 |
| `uartJson.c` | 소규모 추가 | 파싱 성공 시 `uartTestAddLine()` 미러 호출 추가 |

**수정 불필요 파일**: `uartJson.h`, `uartProcess.c`, 하드웨어 관련 Generated_Source, LCD/메뉴 파일

---

## 4. 파일별 상세 변경 계획

---

### 4-1. `package.h` (소규모)

#### 변경 전
```c
#define USER_PROJECT_PATTERN_SEWING_MACHINE
#define PROJECT_FIRMWARE_VERSION "BLACK_CPU V1"
```

#### 변경 후
```c
#define USER_PROJECT_PATTERN_SEWING_MACHINE
#define PROJECT_FIRMWARE_VERSION "EMBROIDERY_S V1"
```

---

### 4-2. `count.h` — COUNT 구조체 필드 추가 (보통)

**추가 위치**: 구조체 `pattern` 계열 필드 **끝 부분** (기존 `patternSPM` 이후)

> **경고**: count.h 주석에 "중간에 uint32를 넣으면 죽는다" 기재됨.
> FLASH 직렬화 구조이므로 **반드시 끝에만 추가**, 기존 필드 순서 절대 변경 금지.

```c
// 기존 마지막 필드
uint16 patternSPM;             // spm : stitching Speed

// 추가 (자수기 전용)
uint16 embThreadBreakageQty    ; // tbq : thread breakage quantity (실끊어진횟수)
uint16 embThreadBreakageQtySumH; // tbqs : Sum High
uint16 embThreadBreakageQtySumL; // tbqs : Sum Low
```

> **cycle_time / motor_runtime 처리 방침 — ms → 0.1초 단위 변환**:
>
> 수신값(ms) ÷ 100 = 0.1초 단위 정수 → uint16 저장
>
> | 예시 | ms 수신값 | ÷100 저장값 | 표시 (÷10.0) |
> |------|-----------|-------------|--------------|
> | 최대 | 3,600,000 | 36,000      | 3600.0 s     |
> | 1분  | 60,000    | 600         | 60.0 s       |
>
> - 저장: `patternCycleTime = (uint16)(cycle_time_ms / 100u);`  (단위: 0.1초)
> - 표시: `patternCycleTime / 10.0f` → 소수점 1자리 초(s)
> - 최대 저장값 36,000 < uint16 상한 65,535 → **uint16으로 안전** ✓
> - `patternMotorRunTime`도 동일 변환 적용

---

### 4-3. `count.c` — ResetCount() 수정 (소규모)

`ResetCount()` 함수에 새 필드 초기화 라인 추가:

```c
// 기존 ResetCount() 끝 부분에 추가
g_ptrCount->embThreadBreakageQty     = 0;
g_ptrCount->embThreadBreakageQtySumH = 0;
g_ptrCount->embThreadBreakageQtySumL = 0;
```

---

### 4-4. `uartJson.c` — 파서 전면 교체 (대규모)

#### 현재 구조 (JSON 파서)
```
uartJsonLoop()
  ├── '{' 문자로 시작 감지
  ├── '}' 문자로 종료 감지
  └── uartJsonParsor() → jsmn 파싱 → 키:값 순서 처리
```

#### 변경 후 구조 (세미콜론 파서)

```
uartEmbLoop()  ← 함수명 변경 또는 유지 (uartJsonLoop 시그니처 유지 권장)
  ├── 버퍼에 문자 누적
  ├── ';' 감지 시 필드 카운터 증가
  ├── 4번째 ';' 수신 → uartEmbParsor() 호출
  └── 파싱 실패 또는 완료 시 버퍼 초기화
```

#### 파서 로직 상세 (의사코드)

```c
/* 새 파서 로직 */
static char g_UART_buff[UART_BUFFER_SIZE];
static int  g_UART_buff_index = 0;

uint8 uartJsonLoop()  /* 함수명은 count.c 호출부 호환을 위해 유지 */
{
    COUNT *ptrCount = getCount();
    while (UART_SpiUartGetRxBufferSize() > 0)
    {
        char c = UART_UartGetChar();
        if (c == '\0' || c == '\r' || c == '\n') continue;

        if (g_UART_buff_index >= UART_BUFFER_SIZE - 1)
        {
            g_UART_buff_index = 0;
            continue;
        }
        g_UART_buff[g_UART_buff_index++] = c;

        if (c == ';')
        {
            // 세미콜론 4개 = 4번째 필드 종료 → 파싱 시도
            int semicolonCount = 0;
            for (int k = 0; k < g_UART_buff_index; k++)
            {
                if (g_UART_buff[k] == ';') semicolonCount++;
            }

            if (semicolonCount >= 4)
            {
                g_UART_buff[g_UART_buff_index] = '\0';
                if (uartEmbParsor(ptrCount))
                {
                    /* UART TEST 미러 */
                    if (g_bUartTestMode) { uartTestAddLine(g_UART_buff); }

                    /* ① 서버 전송 먼저 (patternCount = 이번 actual_qty, 리셋 전) */
                    g_updateCountMenu = TRUE;
                    makeAndonPatternCount();

                    /* ② 누산 후 리셋 */
                    ADD_CONVERT_TO_4BYTE(ptrCount->patternActualH, ptrCount->patternActualL,
                                         ptrCount->patternCount);
                    ptrCount->patternCount = 0;

                    ForcefullyMarkDowntimeAsComplete();
                    SaveInternalFlash();
                    g_UART_buff_index = 0;
                    return TRUE;
                }
                g_UART_buff_index = 0;
                memset(g_UART_buff, 0, UART_BUFFER_SIZE);
            }
        }
    }
    return FALSE;
}
```

#### uartEmbParsor() 구현

```c
/* 형식: "actual_qty;cycle_time;thread_breakage_qty;motor_runtime;" */
char uartEmbParsor(COUNT *ptrCount)
{
    char tempBuf[UART_BUFFER_SIZE];
    snprintf(tempBuf, sizeof(tempBuf), "%s", g_UART_buff);

    char *token;
    char *rest = tempBuf;
    int fieldIndex = 0;

    uint16 actual_qty           = 0;
    uint32 cycle_time           = 0;
    uint16 thread_breakage_qty  = 0;
    uint32 motor_runtime        = 0;

    while ((token = strtok_r(rest, ";", &rest)) != NULL && fieldIndex < 4)
    {
        /* NULL 체크 및 범위 클램핑 */
        switch (fieldIndex)
        {
            case 0: actual_qty          = (uint16) atoi(token); break;
            case 1: cycle_time          = (uint32) atol(token); break;
            case 2: thread_breakage_qty = (uint16) atoi(token); break;
            case 3: motor_runtime       = (uint32) atol(token); break;
        }
        fieldIndex++;
    }

    if (fieldIndex < 4) return FALSE;  /* 필드 수 부족 → 무효 */

    /* 범위 유효성 검사 */
    if (actual_qty          > 10000)   return FALSE;
    if (cycle_time          > 3600000) return FALSE;
    if (thread_breakage_qty > 1000)    return FALSE;
    if (motor_runtime       > 3600000) return FALSE;

    /* COUNT 구조체에 할당 */
    ptrCount->patternCount        += actual_qty;

    /* cycle_time: ms → 0.1초 단위 변환 (÷100), 최대 36000 → uint16 안전 */
    ptrCount->patternCycleTime     = (uint16)(cycle_time / 100u);
    ADD_CONVERT_TO_4BYTE(ptrCount->patternCycleTimeSumH, ptrCount->patternCycleTimeSumL,
                         ptrCount->patternCycleTime);

    /* thread_breakage_qty: 단위 변환 없음 (최대 1000, uint16 안전) */
    ptrCount->embThreadBreakageQty = thread_breakage_qty;
    ADD_CONVERT_TO_4BYTE(ptrCount->embThreadBreakageQtySumH, ptrCount->embThreadBreakageQtySumL,
                         ptrCount->embThreadBreakageQty);

    /* motor_runtime: ms → 0.1초 단위 변환 (÷100), 최대 36000 → uint16 안전 */
    ptrCount->patternMotorRunTime  = (uint16)(motor_runtime / 100u);
    ADD_CONVERT_TO_4BYTE(ptrCount->patternMotorRunTimeSumH, ptrCount->patternMotorRunTimeSumL,
                         ptrCount->patternMotorRunTime);

    return TRUE;
}
```

> **jsmn 관련**: `uartJson.c` 상단의 `#include "jsonUtil.h"` 및 jsmn 관련 include는
> 파서 교체 후 **제거** 한다. `lib/jsmn.c`는 `andonJson.c`(서버 응답 파싱)에서 여전히 사용되므로
> 프로젝트에서 완전 삭제 금지 — `uartJson.c`에서만 include 제거.

---

### 4-5. `userMenuPatternSewingMachine.c` — LCD 메뉴 수정 (보통)

#### 변경 전후 Page 구조 비교

| | BLACK_CPU | EMBROIDERY_S |
|-|-----------|--------------|
| Page 0 | Target / Actual / Rate | Target / Actual / Rate (동일) |
| Page 1 | CT / RT / Rate(%) | CT / MRT / TB |
| Page 2 | SQ / SL / SPI | **삭제** |

#### 1) `g_patternString[]` 배열 수정

```c
// 변경 전
char *g_patternString[] = {"Target", "Actual", "Rate", "CT", "RT", "SQ", "SL", "SPI"};
uint8 g_noPatternString = 8;

// 변경 후
char *g_patternString[] = {"Target", "Actual", "Rate", "CT", "MRT", "TB"};
uint8 g_noPatternString = 6;
```

#### 2) `TopMenuPageUpdate()` — Page 1 라벨 수정, Page 2 케이스 삭제

```c
case 1:
    SetButtonText(&menu->btn[1], g_patternString[3]);  // "CT"
    SetButtonText(&menu->btn[3], g_patternString[4]);  // "MRT"  ← "RT" → "MRT"
    SetButtonText(&menu->btn[5], g_patternString[5]);  // "TB"   ← "Rate" → "TB"
    break;
// case 2: 삭제
```

#### 3) `TopMenuPageUpdateValue()` — Page 1 값 수정, Page 2 케이스 삭제

```c
case 1:
    // CT: 누산합 ÷ 10 → 정수 초 (소수점 없음)
    SetButtonText(&menu->btn[2], "%lu",
        (uint32)CONVERT_TO_4BYTE(ptrCount->patternCycleTimeSumH,
                                 ptrCount->patternCycleTimeSumL) / 10u);

    // MRT: 누산합 ÷ 10 → 정수 초 (소수점 없음)
    SetButtonText(&menu->btn[4], "%lu",
        (uint32)CONVERT_TO_4BYTE(ptrCount->patternMotorRunTimeSumH,
                                 ptrCount->patternMotorRunTimeSumL) / 10u);

    // TB: 실끊어진횟수 누산합
    SetButtonText(&menu->btn[6], "%lu",
        (uint32)CONVERT_TO_4BYTE(ptrCount->embThreadBreakageQtySumH,
                                 ptrCount->embThreadBreakageQtySumL));
    break;
// case 2: 삭제
// persent = TRUE 라인도 case 1에서 제거 (% 기호 불필요)
```

#### 4) 페이지 이동 상한값 수정

```c
// 변경 전
if (page < 2) page++;
// 변경 후
if (page < 1) page++;
```

#### 5) `DisplayDoTopMenu()` — 상단 헤더 텍스트 수정

```c
// 변경 전
SetDrawMonitoringMenu(menu, "%d : %3.1f-Pair",
    g_ptrMachineParameter->patternPairCount, g_ptrMachineParameter->patternPair/10.);

// 변경 후
SetDrawMonitoringMenu(menu, "EMBROIDERY");
```

#### 6) `menuCreate()` — PAIR(S) INFO 메뉴 노드 삭제, TARGET 유지

```c
// 삭제 대상 (페어 개념 제거)
MENUNODE *pairsInfoMenuNode = createMENUNODE(root, "PAIR(S) INFO", &doPairsInfoMenu);

// 유지 (LCD 직접 설정 + 서버 수신 모두 활성)
MENUNODE *targetInfoMenuNode  = createMENUNODE(root, "TARGET",  &doTargetInfoMenu);
MENUNODE *actualtInfoMenuNode = createMENUNODE(root, "ACTUAL",  &doActualInfoMenu);
```

> Target은 LCD 메뉴 직접 입력과 서버 `get_target` API 수신 **둘 다 유지**한다.
> `makeRequestTarget()` → `andonRequestTargetParsing()` 플로우 변경 없음.

---

### 4-6. `andonApi.c` — `makeAndonPatternCount()` 수정 (소규모) ✓ 완료

`send_pCount` 대신 자수기 전용 엔드포인트 `send_eCount`를 사용한다.
서버 파일: `C:\SUNTECH_DEV_CLAUDECODE\WEB\CTP280_API\send_eCount.php` (추후 신규 작성)

> **`actual_qty` 처리 방침**:
> `makeAndonPatternCount()`는 `uartJson.c`에서 `ADD_CONVERT_TO_4BYTE` 리셋 **전에** 호출된다.
> 따라서 `ptrCount->patternCount`가 이번 패킷의 `actual_qty` 값을 그대로 보유한다.

#### 변경 전
```c
enQueueANDON_printf(ANDON_SEND_PATTERN_COUNT,
    "send_pCount&mac=%s&pc=%u&pi=%0.1f&sc=%0.1f&design_no=%u&ct=%u\r\n",
    ...
);
```

#### 변경 후 (구현 완료)
```c
/* 자수기 UART 패킷 수신 시 서버 전송 — EMBROIDERY_S 전용 */
void makeAndonPatternCount()
{
    COUNT *ptrCount = getCount();

    enQueueANDON_printf(ANDON_SEND_PATTERN_COUNT,
        "send_eCount&mac=%s&actual_qty=%u&sc=%0.1f&ct=%0.1f&tb=%u&mrt=%0.1f\r\n",
        g_network.MAC,
        ptrCount->patternCount,                                                             // actual_qty : 이번 패킷 완료 수량 (리셋 전)
        (uint32) CONVERT_TO_4BYTE(ptrCount->patternActualH, ptrCount->patternActualL) / 10., // sc : 시프트 누산 실적 (÷10 → 소수점 1자리)
        ptrCount->patternCycleTime / 10.,                                                   // ct : 싸이클타임 (0.1s → 소수점 1자리 초)
        ptrCount->embThreadBreakageQty,                                                     // tb : 실끊김 수량
        ptrCount->patternMotorRunTime / 10.                                                 // mrt: 모터동작시간 (0.1s → 소수점 1자리 초)
    );
}
```

> `send_eCount.php` 작성 시 수신 파라메타: `mac`, `actual_qty`, `sc`, `ct`, `tb`, `mrt`

---

### 4-7. `uartTestMenu.c` / `uartTestMenu.h` — UART TEST 메뉴 신규 생성

> 참조 소스: `C:\SUNTECH_DEV_CLAUDECODE\PSOC\280CTP_IoT_UART_TEST\280CTP_IoT_UART_TEST_V1\Project\Design.cydsn\uartJson.c`

#### 핵심 설계 원칙 — 미러(Mirror) 방식

UART RX 버퍼는 한 번 읽으면 소비된다. 테스트 뷰어가 직접 읽으면 파서가 데이터를 잃는다.

**해결**: 파싱과 서버 전송은 항상 정상 동작하고, 파싱 성공 시 수신 문자열을 테스트 뷰어에 복사만 한다.

```
자수기 ──UART──▶ uartJsonLoop()   ← 항상 실행 (파싱 + 서버 전송 유지)
                      │
                 파싱 성공 &
              g_bUartTestMode==TRUE
                      │
                      ▼
              uartTestAddLine()   ← 수신 문자열 복사
                      │
                      ▼
              uartTestDrawScreen() ← LCD 표시
```

- 파싱/서버 전송 **중단 없음**
- UART 데이터 **소실 없음**
- `PatternCountLoop()` **수정 불필요**
- 테스트 뷰어는 **LCD 표시 전용** (UART 직접 읽기 없음)

---

#### `uartTestMenu.h`

```c
#ifndef _UART_TEST_MENU_H_
#define _UART_TEST_MENU_H_
#include "main.h"

extern uint8 g_bUartTestMode;  /* TRUE: 테스트 뷰어 활성 */

void uartTestAddLine(const char *line);   /* uartJsonLoop()에서 호출 */
int  doUartTestMenu(void *this, uint8 reflash);

#endif
```

---

#### `uartJson.c` — 미러 호출 추가 (소규모) ✓ 완료

파싱 성공 후 `g_UART_buff`를 테스트 뷰어에 복사한다.

```c
#include "uartTestMenu.h"

/* uartEmbParsor() 성공 직후, 버퍼 초기화 전에 추가 */
if (uartEmbParsor(ptrCount))
{
    /* 테스트 모드 활성 시 수신 문자열 미러 */
    if (g_bUartTestMode)
    {
        uartTestAddLine(g_UART_buff);   /* LCD에 표시 */
    }

    /* ※ 실제 구현 순서:
       1. g_updateCountMenu = TRUE;
       2. makeAndonPatternCount();         ← patternCount(actual_qty) 리셋 전에 호출
       3. ADD_CONVERT_TO_4BYTE(...);       ← 리셋 전 patternCount 값으로 서버 전송 완료 후 누산
       4. ptrCount->patternCount = 0;
    */

    /* 이후 정상 처리 계속 */
    ADD_CONVERT_TO_4BYTE(...);
    ...
}
```

---

#### `uartTestMenu.c` 구조

```c
#include "uartTestMenu.h"
#include "lib/widget.h"
#include "lib/UI.h"

/* ── 뷰어 상수 (UART_TEST 프로젝트와 동일) ── */
#define UART_VIEW_MAX_REC        5
#define UART_VIEW_REC_LEN     1024
#define UART_VIEW_HEADER_H      40
#define UART_VIEW_FOOTER_H      40
#define UART_VIEW_LINE_H        18
#define UART_VIEW_FONT_W         8

/* ── 전역 플래그 ── */
uint8 g_bUartTestMode = FALSE;

/* ── 내부 상태 (static) ── */
static char   g_recBuf[UART_VIEW_MAX_REC][UART_VIEW_REC_LEN];
static uint8  g_recHead   = 0;
static uint8  g_recCount  = 0;
static uint16 g_recvCount = 0;
static int16  g_scrollLine   = 0;
static uint8  g_bNeedRedraw  = TRUE;
static uint8  g_touchHandled = FALSE;
static uint8  g_autoScroll   = TRUE;

/* ── uartTestAddLine(): uartJson.c 에서 미러 호출 ── */
void uartTestAddLine(const char *line)
{
    uint8 writeIdx;
    if (g_recCount < UART_VIEW_MAX_REC)
    {
        writeIdx = (g_recHead + g_recCount) % UART_VIEW_MAX_REC;
        g_recCount++;
    }
    else
    {
        writeIdx  = g_recHead;
        g_recHead = (g_recHead + 1) % UART_VIEW_MAX_REC;
    }
    strncpy(g_recBuf[writeIdx], line, UART_VIEW_REC_LEN - 1);
    g_recBuf[writeIdx][UART_VIEW_REC_LEN - 1] = '\0';
    g_recvCount++;

    if (g_autoScroll)
    {
        uint16 totalLines = calcTotalLines();
        uint16 visLines   = getVisibleLines();
        g_scrollLine = (totalLines > visLines)
                       ? (int16)(totalLines - visLines) : 0;
    }
    g_bNeedRedraw = TRUE;
}

/* ── 메인 메뉴 함수 ── */
int doUartTestMenu(void *this, uint8 reflash)
{
    switch (reflash)
    {
        case TRUE:   /* 화면 진입 */
            g_bUartTestMode = TRUE;
            memset(g_recBuf, 0, sizeof(g_recBuf));
            g_recHead = g_recCount = g_recvCount = 0;
            g_scrollLine  = 0;
            g_autoScroll  = TRUE;
            g_bNeedRedraw = TRUE;
            SetDrawBottomButtons("QUIT", "", BUTTON_STYLE_R_GREEN, BUTTON_STYLE_DARKGREY);
            break;

        case FALSE:  /* 반복 호출 */
        {
            TOUCH tc = GetTouch();
            if (tc.isClick)
            {
                if (getIndexOfClickedButton(&tc, g_btnBottom, 2) == BOTTOM_LEFT)
                {
                    g_bUartTestMode = FALSE;
                    return MENU_RETURN_PARENT;
                }
            }
            uartTestDrawScreen();    /* LCD 갱신 */
            uartTestHandleTouch();   /* UP / DOWN / CLEAR 터치 */
            break;
        }
    }
    return MENU_RETURN_THIS;
}
```

> `uartTestDrawScreen()`, `uartTestHandleTouch()`, `calcTotalLines()`, `getVisibleLines()`, `getCharsPerLine()` 는
> UART_TEST 프로젝트의 동명 함수를 **함수명 그대로** `uartTestMenu.c` 내 `static` 함수로 이식한다.
> 헤더 타이틀 문자열만 `"EMBROIDERY UART RX TEST  [%u]"` 로 변경한다.

---

#### `lib/manageMenu.c` — UART TEST 메뉴 노드 추가 ✓ 완료

> **확인 결과**: RESTART 노드는 `userMenuPatternSewingMachine.c`가 아니라 `lib/manageMenu.c`의
> `manageMenuCreate()` 함수 내에 있었다. 따라서 아래와 같이 `lib/manageMenu.c`에 추가하였다.

```c
/* lib/manageMenu.c 상단 */
#include "../uartTestMenu.h"

/* manageMenuCreate() 내부 — RESTART 노드 바로 다음 */
MENUNODE *restart  = createMENUNODE(root, "RESTART",   &doRestart);
MENUNODE *uartTest = createMENUNODE(root, "UART TEST", &doUartTestMenu);  /* ← 추가 */
```

---

## 5. 주요 리스크 및 고려사항

| 번호 | 리스크 | 영향도 | 대응 방안 |
|------|--------|--------|-----------|
| R-1 | `cycle_time` / `motor_runtime` uint16 오버플로우 | ~~높음~~ **해결됨** | ms ÷ 100 → 0.1초 단위 저장, 최대값 36,000 < 65,535 ✓ |
| R-2 | COUNT 구조체 FLASH 직렬화 레이아웃 파괴 | 매우 높음 | 새 필드는 반드시 구조체 끝(andonEntry 이전)에만 추가 |
| R-3 | `strtok_r` PSoC4 컴파일러 지원 여부 | 보통 | 미지원 시 `strtok` 사용 또는 직접 `;` 스캔 루프로 대체 |
| R-4 | 자수기 UART 보레이트/프레이밍 차이 | 보통 | BLACK_CPU와 동일 보레이트 사용 가정, 실기기 연결 후 확인 필요 |
| R-5 | 서버 API `send_pCount` 신규 파라메타(`tb`) 미인식 | 낮음 | 서버 측 OEE_SCI 웹 백엔드에서 `tb` 파라메타 수신 처리 추가 필요 |

---

## 6. 변경 불필요 파일 (검증 완료)

- `uartProcess.c` — `UART_Start()` 호출만 포함, 변경 없음
- `uartJson.h` — 함수 시그니처 `uint8 uartJsonLoop()` 유지
- `count.c` 의 `SetCountLoop()`, `PatternCountLoop()` — 로직 변경 없음
- `userProjectPatternSewing.c/h` — 자수기는 `PATTERN_MACHINE` 타입 그대로 사용
- `menuDesign.c` — 변경 없음 (`userMenuPatternSewingMachine.c`는 수정 대상)
- `andonJson.c` — 서버 응답 JSON 파싱, 변경 없음
- `lib/jsmn.c` — 서버 응답 파싱에서 계속 사용, 유지

---

## 7. 작업 순서 (구현 완료 현황)

```
[✓] Step 1. package.h                       — 버전명 "EMBROIDERY_S V1" 변경 완료
[✓] Step 2. count.h                         — embThreadBreakageQty / SumH / SumL 3개 필드 추가 완료
[✓] Step 3. count.c                         — ResetCount() 초기화 3줄 추가 완료
[✓] Step 4. uartJson.c                      — 세미콜론 파서 전면 교체 완료
                                               ※ makeAndonPatternCount() 호출 순서:
                                                  ADD_CONVERT 리셋 전에 호출 (actual_qty 보존)
[✓] Step 5. userMenuPatternSewingMachine.c  — LCD Page 구조 수정 완료
                                               (Page 2 삭제, Page 1 CT/MRT/TB, 헤더 "EMBROIDERY")
[✓] Step 6. uartTestMenu.c / .h (신규)      — UART TEST 뷰어 구현 완료
                                               lib/manageMenu.c 에 노드 추가 (RESTART 다음)
[  ] Step 7. count.c PatternCountLoop()     — 테스트 모드 스킵 불필요 (미러 방식으로 해결됨)
[✓] Step 8. andonApi.c                      — makeAndonPatternCount() → send_eCount 완료
[✓] Step 9. 빌드 검증                       — PSoC Creator 빌드 성공 (2026-03-26 11:08:01)
                                               ※ Design.cyprj 에 uartTestMenu.c 미등록 → 링커 에러 발생
                                                  (undefined reference to doUartTestMenu / uartTestAddLine / g_bUartTestMode)
                                                  → Design.cyprj 에 SOURCE_C 엔트리 추가 후 해결
[  ] Step 10. USB 디버그 테스트              — PC에서 `1;300000;2;250000;\r\n` 전송 테스트 (미실시)
[✓] Step 11. UART TEST 메뉴 테스트          — MENU → UART TEST 진입 → LCD 표시 확인
                                               버그 발견: QUIT → MENU 복귀 시 헤더 타이틀 잔류 → 수정 완료 (섹션 11 참조)
[  ] Step 12. 실기기 연결 테스트             — 자수기 UART 연결 후 실데이터 수신 확인 (미실시)
```

---

## 8. 미결 확인 필요 사항 (사용자 확인 요청)

| 항목 | 질문 |
|------|------|
| ~~Q-1~~ | ~~`cycle_time` 단위 확인~~ | **확인 완료**: ms 단위. ÷100 → 0.1초 단위 uint16 저장으로 해결 |
| ~~Q-2~~ | ~~`actual_qty` 증분 여부 확인~~ | **확인 완료**: 증분값(delta). 완료 시마다 해당 수량만 전송. `patternCount += actual_qty` 누산 로직 그대로 사용 ✓ |
| ~~Q-3~~ | ~~UART 종료 문자 확인~~ | **확인 완료**: 마지막 `;` 이후 `\r\n` 붙음. 파서 루프 첫 줄 `\r\n` 필터로 자동 처리 ✓ |
| ~~Q-4~~ | ~~서버 API 파라메타 호환 여부~~ | **확인 완료**: `send_pCount` 재사용 안 함. 자수기 전용 `send_eCount.php` 신규 작성 예정 (`C:\SUNTECH_DEV_CLAUDECODE\WEB\CTP280_API\send_eCount.php`) |
| ~~Q-5~~ | ~~UART 보레이트 확인~~ | **확인 완료**: 115200 bps — Black CPU와 동일, 변경 불필요 ✓ |

---

## 9. 빌드 결과 (최초 성공 빌드)

> 빌드 일시: 2026-03-26 11:08:01

| 항목 | 사용량 | 전체 | 점유율 |
|------|--------|------|--------|
| Flash (전체) | 119,860 bytes | 262,144 bytes | 45.7% |
| ㄴ Bootloader | 13,568 bytes | — | — |
| ㄴ Application | 106,036 bytes | — | — |
| ㄴ Metadata | 256 bytes | — | — |
| **SRAM** | **27,708 bytes** | **32,768 bytes** | **84.6%** |
| ㄴ Stack | 2,048 bytes | — | — |
| ㄴ Heap | 1,024 bytes | — | — |

### SRAM 여유 분석

SRAM 84.6% 사용 중 — 여유 5,060 bytes. 운용에는 문제없으나 향후 기능 추가 시 주의 필요.

주요 원인: `uartTestMenu.c` 링버퍼 `g_recBuf[5][1024]` = **5,120 bytes** (SRAM의 약 15.6%)

### SRAM 절약 방안 (필요 시 적용)

링버퍼 크기를 절반으로 축소:

```c
// uartTestMenu.c
// 변경 전
#define UART_VIEW_MAX_REC     5
#define UART_VIEW_REC_LEN  1024   // → g_recBuf = 5 × 1024 = 5,120 bytes

// 변경 후 (절약 시)
#define UART_VIEW_MAX_REC     5
#define UART_VIEW_REC_LEN   512   // → g_recBuf = 5 × 512  = 2,560 bytes (약 2,560 bytes 회수)
```

> 자수기 UART 패킷 형식: `"1;300000;2;250000;\r\n"` (최대 약 30자)
> REC_LEN 512 로도 충분히 수용 가능.
> SRAM 여유가 3,072 bytes 이하로 떨어지면 적용 검토.

---

## 10. 향후 개선 검토 사항

---

### 10-1. UART 원본 데이터 저장 (방법 A) — 미구현, 필요 시 적용

> **배경**: `log_embroidery.php` 뷰어에서 UART DATA 컬럼을 표시하기 위해 두 가지 방법을 검토함.
> 현재는 **방법 B(역산 재구성)** 를 적용 중. 정확한 원본 보존이 필요할 경우 방법 A로 전환한다.

#### 현재 적용: 방법 B (역산 재구성, 변경 없음)

`log_embroidery.php` JS에서 저장된 컬럼값으로 UART 문자열을 재구성한다.

```javascript
// actual_qty=18, ct=60.0s, tb=0, mrt=65.0s → "18;60000;0;65000;"
`${r.actual_qty};${Math.round(r.ct * 1000)};${r.tb};${Math.round(r.mrt * 1000)};`
```

- DB/펌웨어/API 변경 없음
- `ct * 1000`, `mrt * 1000` 역산: ms → /100 → /10 변환이 정수 배수이므로 손실 없음 ✓
- 단점: float 연산 누적 오차가 생길 수 있는 엣지 케이스 (비정상 수신값) 에서 원본과 다를 수 있음

---

#### 미래 전환 방안: 방법 A (원본 저장)

UART에서 수신한 원본 문자열을 그대로 DB에 저장한다.

##### 1단계 — DB: `data_embroidery` 테이블에 컬럼 추가

```sql
ALTER TABLE data_embroidery
    ADD COLUMN `uart_raw` VARCHAR(64) NULL COMMENT 'UART 원본 수신 문자열' AFTER `mrt`;
```

##### 2단계 — 서버: `send_eCount.php` 파라미터 추가

```php
// 파라미터: mac, actual_qty, ct, tb, mrt, uart_raw
$uart_raw = substr(trim($_REQUEST['uart_raw'] ?? ''), 0, 64);  // 최대 64자 제한

$stmt = $pdo->prepare(
    "INSERT INTO data_embroidery (mac, actual_qty, ct, tb, mrt, uart_raw)
     VALUES (?, ?, ?, ?, ?, ?)"
);
$stmt->execute([$mac, $actual_qty, $ct, $tb, $mrt, $uart_raw]);
```

##### 3단계 — 펌웨어: `andonApi.c` — `makeAndonPatternCount()` 수정

`g_UART_buff` (파싱 전 원본 문자열)를 HTTP 파라미터로 추가한다.

```c
/* uartJson.c 에서 uartEmbParsor() 성공 후 makeAndonPatternCount() 호출 전,
   g_UART_buff 를 andonApi 쪽으로 전달하는 방법 두 가지: */

/* 방법 A-1: makeAndonPatternCount(g_UART_buff) 인자 전달 */
void makeAndonPatternCount(const char *uart_raw)
{
    COUNT *ptrCount = getCount();
    enQueueANDON_printf(ANDON_SEND_PATTERN_COUNT,
        "send_eCount&mac=%s&actual_qty=%u&ct=%0.1f&tb=%u&mrt=%0.1f&uart_raw=%s\r\n",
        g_network.MAC,
        ptrCount->patternCount,
        ptrCount->patternCycleTime / 10.,
        ptrCount->embThreadBreakageQty,
        ptrCount->patternMotorRunTime / 10.,
        uart_raw                            // 원본 문자열 (예: "18;60000;0;65000;")
    );
}

/* 방법 A-2: 전역 변수 경유 (함수 시그니처 유지) */
// andonApi.h 에 extern char g_lastUartRaw[]; 선언
// uartJson.c 에서 파싱 성공 시: strncpy(g_lastUartRaw, g_UART_buff, sizeof(g_lastUartRaw)-1);
// makeAndonPatternCount() 내부에서 g_lastUartRaw 직접 참조
```

> **권장**: 방법 A-2(전역 변수)가 기존 호출부(`count.c`)를 수정하지 않아 안전하다.

##### 4단계 — 뷰어: `log_embroidery.php` 수정

- PHP SELECT 쿼리에 `uart_raw` 추가
- JS `renderTable`에서 역산 코드 제거 후 `r.uart_raw` 직접 표시

##### 주의사항

| 항목 | 내용 |
|------|------|
| SRAM 영향 | `g_lastUartRaw[64]` 추가 시 64 bytes 증가 (현재 여유 5,060 bytes, 문제 없음) |
| URL 길이 | `uart_raw=18;60000;0;65000;` 추가 시 약 26자. AT 명령 총 길이 영향 없음 |
| `;` URL 인코딩 | `enQueueANDON_printf` 에서 `;` 를 `%3B` 로 인코딩하지 않으면 서버 파싱 오류 가능. `uart_raw` 파라미터는 마지막에 배치하고 서버에서 `rawurldecode()` 처리 권장 |

---

## 11. 발견된 버그 및 수정 이력

### 11-1. echo 루프백에 의한 2번째 패킷 파싱 실패 (2026-03-27 수정)

**버그**: 2초 이상 간격으로 2개 이상 패킷 전송 시 2번째 패킷부터 파싱 실패

**원인**: `[DBG]` `printf` 출력이 UART TX → echo → RX로 돌아와 버퍼에 세미콜론 4개 포함 문자열 오염
→ 가짜 4SC 트리거 → `uartEmbParsor` 실패 → 버퍼 클리어 → 잔류 바이트로 인해 다음 패킷 파싱 불가

**수정 (`uartJson.c`)**:
1. `g_UART_buff_index == 0` 시 `'0'`~`'9'` 외 바이트 무시 (index=0 숫자 필터)
2. `[DBG] 4SC found:` printf 블록 제거
3. `[DBG] uartJsonLoop:` printf 블록 제거

---

### 11-2. UART TEST → MENU 복귀 시 타이틀 잔류 (2026-03-27 수정)

**버그**: LCD → MENU → UART TEST → QUIT → MENU 복귀 시 상단 헤더에 "EMBROIDERY UART RX TEST" 타이틀이 그대로 잔류

**원인**:
- `uartTestDrawScreen()`이 표준 헤더 영역(`y:0~39`)을 `VIEW_HDR_BG`(진한 파랑, `CONVERT565(0,0,100)`)으로 덮어씀
- `UART_VIEW_HEADER_H = 40` = `DEFAULT_TOP_TITLE_HEIGHT = 40` → 표준 헤더와 크기 일치, 정확히 겹쳐 덮어씀
- MENU 복귀 시 `doListMenuPage`(`reflash=TRUE`)는 바디 영역(`y≥41`)만 갱신하고 `DrawHeader()`를 호출하지 않음
- 결과: y=0~39 영역이 UART TEST 커스텀 헤더 상태 그대로 잔류

**수정 (`uartTestMenu.c` 1줄 추가)**:

```c
// QUIT 처리 블록 (case FALSE)
g_bUartTestMode = FALSE;
Buzzer(BUZZER_CLICK, 0);
DrawHeader(); /* UART TEST가 덮어쓴 표준 헤더 영역(y:0~39) 복원 */
return MENU_RETURN_PARENT;
```

- `DrawHeader()` — WiFi 아이콘 + TitleBar 버튼 + 하단 구분선 재드로우
- `widget.h`에 이미 선언되어 있어 추가 include 불필요

---

*계획 작성: 2026-03-26 | 마지막 수정: 2026-03-27 | 280CTP_IoT_INTEGRATED_V1_EMBROIDERY_S 전용*
