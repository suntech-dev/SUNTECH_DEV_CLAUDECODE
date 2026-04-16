# BUG FIX: EMBROIDERY_S — 자수기 UART 패킷 전부 소실

## 상태: 🔧 3차 수정 완료 (UART TEST 진단 강화, 빌드 필요) — 미해결 원인 추가 조사 중

---

## 버그 요약

| 항목 | 내용 |
|---|---|
| 심각도 | **CRITICAL** |
| 발견일 | 2026-04-12 |
| 2차 발견일 | 2026-04-15 |
| 영향 범위 | `280CTP_IoT_INTEGRATED_V1_EMBROIDERY_S` 프로젝트 전체 |
| 증상 | 자수기 실기기 연결 시 카운트 증가 없음, LCD UART TEST 화면 텍스트 미출력 |
| 단독 테스트 | PC에서 `2;82;0;75;` 단독 전송 시 정상 동작 (문제 재현 안 됨) |
| 1차 원인 | MCSTATUS 줄이 다음 패킷과 합쳐져 오염 → 수정 완료 |
| 2차 원인 (가설, 틀림) | ~~프로토콜 불일치: 후행 `;` 없음~~ → **실제로는 후행 `;` 있음** (2026-04-15 사용자 확인) |
| 미해결 원인 | **1차 수정 후에도 실패** — 파싱 로직은 정상이나 UART 수신 자체 문제 의심 |

---

## 자수기 실제 프로토콜 (2026-04-15 확인)

### 데이터 포맷

```
actual_qty;cycle_time_s;thread_breakage_qty;motor_runtime_s;\r\n
```

**후행 `;` 있음** (사용자 2026-04-15 직접 확인). 예시:
```
2;82;0;75;
20;187;1;175;
12;182;0;153;
16;172;2;131;
```

### UART 출력 순서

부팅 시 1회 긴 텍스트 → MCSTATUS → 자수 완료마다 데이터 패킷

```
MCSTATUS Periodic Data : 0x41,0xff\r\n   ← hex 값은 매번 달라짐
2;82;0;75;\r\n                           ← 실제 데이터 패킷 (후행 ; 있음)
MCSTATUS Periodic Data : 0xAB,0x12\r\n
20;187;1;175;\r\n
...
```

---

## 근본 원인

### 1차 원인 (2026-04-12 수정 완료)

자수기는 데이터 패킷 직전에 **MCSTATUS 줄을 전송할 수 있다** (항상은 아님).

### 버그 코드 (`uartJson.c` 수정 전)

```c
// ❌ 수정 전: \r\n 은 버퍼 유지한 채 skip
if (c == '\0' || c == '\r' || c == '\n') continue;

// ❌ 수정 전: index=0 에서만 비숫자 차단, 중간 문자 필터 없음
if (g_UART_buff_index == 0 && (c < '0' || c > '9')) continue;
```

### 오염 경로 (3단계)

```
MCSTATUS Periodic Data : 0x41,0xff\r\n
  'M' → index=0, 비숫자 → 스킵 ✓
  '0' (from 0x41) → digit → 버퍼 시작: "0", index=1
  'x','4','1',',','0','x','f','f' → 중간 필터 없음 → 버퍼 누적
  '\r','\n' → continue (버퍼 유지! "0x41,0xff" 남음)

2;82;0;75;\r\n  ← 실제 패킷
  '2' → index=9, 그냥 붙음 → 버퍼: "0x41,0xff2"
  ';','8','2',';','0',';','7','5',';' → 세미콜론 4개 → 파싱 시도!
  field[0] = atoi("0x41,0xff2") = 0  ← atoi는 'x'에서 멈춰 0 반환
  actual_qty == 0 → return FALSE → 버퍼 리셋

→ 패킷 소실! MCSTATUS가 매 패킷 전에 오므로 전 패킷 소실
```

### hex 값이 달라지는 경우 추가 케이스

```
0x12,0x34 처럼 끝이 숫자로 끝나는 경우:
  '3','4' → 버퍼: "34" (중간 필터는 통과함 — 숫자이므로)
  '\r' → 버퍼 유지 (수정 전)
  다음 줄 '2;82;...' → "342;82;0;75;" → atoi("342")=342 → 오파싱
```

---

## 미해결 원인 분석 (2026-04-15)

### 문제 재정의

- 자수기는 `2;82;0;75;\r\n` (후행 `;` **있음**) 형태로 전송
- 1차 수정 코드에서 `2;82;0;75;\r\n`은 4번째 `;` 수신 시 **파싱 성공해야 정상**
- 그런데 1차 수정 플래싱 후 실기기 테스트 → LCD UART TEST 아무것도 표시 안 됨

→ **파싱 로직 자체는 올바름. 데이터가 파서에 도달하지 못하는 것으로 의심.**

### 유력 가설: 부팅 메시지 UART RX 버퍼 오버플로우

자수기 부팅 메시지는 약 500~700바이트의 텍스트를 한 번에 전송한다.  
PSoC UART 소프트웨어 RX 버퍼 크기가 부팅 메시지보다 작은 경우:

1. 부팅 메시지 수신 중 버퍼 오버플로우 발생
2. PSoC `UART_rxBufferOverflow` 플래그 세트
3. 이후 ISR이 소프트웨어 버퍼 쓰기를 중단하거나 카운터가 오동작
4. `UART_SpiUartGetRxBufferSize()` 가 0을 반환
5. `uartJsonLoop()` 의 `while (UART_SpiUartGetRxBufferSize() > 0)` 진입 불가
6. 자수 완료 데이터 패킷 수신 자체가 불가능 → UART TEST 화면 아무것도 표시 안 됨

### 플래싱 후 UART TEST 화면으로 진단하는 방법

현재 3차 수정 코드에 진단용 `uartTestAddLine` 호출이 추가되어 있어,  
다음 플래싱 후 UART TEST 화면 결과로 원인을 구분할 수 있다.

| UART TEST 화면 결과 | 의미 | 다음 조치 |
|---|---|---|
| `2;82;0;75;` 표시됨 | 파싱 성공 ✅ 해결 | 완료 |
| 잘못된 내용 표시 (예: `342;82;0;75;`) | 파싱 됐지만 MCSTATUS 오염 잔류 | 1차 수정 재검토 |
| **아무것도 표시 안 됨** | **UART 수신 자체 불가 → 버퍼 오버플로우** | PSoC UART RX 버퍼 크기 확대 |

### 버퍼 오버플로우 의심 시 조치

PSoC Creator에서 UART 컴포넌트(SCB) 설정 변경:

- **RX Buffer Size**: 현재 값 확인 후 **1024** 이상으로 증가
- 자수기 부팅 메시지 700바이트 + 여유 → 1024 이상 권장

---

## 해결 방안: 3단계 필터

### 수정 1 — `\r\n` 수신 시 버퍼 리셋 (줄 경계 격리)

```c
// ✅ 수정 후
if (c == '\r' || c == '\n' || c == '\0')
{
    if (g_UART_buff_index > 0)
    {
        g_UART_buff_index = 0;
        memset(g_UART_buff, 0, UART_BUFFER_SIZE);
    }
    continue;
}
```

> 유효 패킷은 4번째 `;` 수신 시 이미 파싱·리셋 완료되므로 `\r\n` 도달 시 `index=0` → 이 블록 미실행 → 영향 없음 ✓

### 수정 2 — 패킷 시작 필터 (유지)

```c
if (g_UART_buff_index == 0 && (c < '0' || c > '9')) continue;
```

### 수정 3 — 패킷 중간 문자 필터 (핵심 추가)

```c
// ✅ 수정 후: 숫자·';' 외 문자 즉시 리셋
if (g_UART_buff_index > 0 && (c < '0' || c > '9') && c != ';')
{
    g_UART_buff_index = 0;
    memset(g_UART_buff, 0, UART_BUFFER_SIZE);
    continue;
}
```

### 수정 4 — `printf` 제거 (echo 루프백 방지)

```c
// ❌ 제거: uartEmbParsor() 내 UART TX → echo → RX 오염 원인
// printf("PARSE: actual_qty=%u ct=%lu tb=%u mrt=%lu\r\n", ...);
```

---

## 수정 후 전체 흐름 검증

### MCSTATUS hex 값이 달라도 모두 처리

| MCSTATUS 값 | 처리 흐름 | 결과 |
|------------|-----------|------|
| `0x41,0xff` | '0'→버퍼; 'x'→중간필터RESET; '4','1'→버퍼; ','→RESET; '0'→버퍼; 'x'→RESET; 'f','f'→스킵; `\r\n`→index=0→무시 | 클린 ✓ |
| `0xAB,0xCD` | '0'→버퍼; 'x'→RESET; 'A','B'→스킵; ','→스킵; '0'→버퍼; 'x'→RESET; 'C','D'→스킵; `\r\n`→무시 | 클린 ✓ |
| `0x12,0x34` | '0'→버퍼; 'x'→RESET; '1','2'→버퍼; ','→RESET; '0'→버퍼; 'x'→RESET; '3','4'→버퍼; `\r\n`→**RESET** | 클린 ✓ |

### 실제 데이터 패킷 처리

```
(MCSTATUS 처리 후 버퍼 클린 상태)
2;82;0;75;\r\n
  '2' → digit → 버퍼: "2"
  ';' → 유효, count=1
  '8','2' → "2;82"
  ';' → count=2
  '0',';' → count=3
  '7','5',';' → count=4 → 파싱!
  field[0]="2" → actual_qty=2 ✓
  field[1]="82" → cycle_time_s=82 ✓
  field[2]="0" → thread_breakage_qty=0 ✓
  field[3]="75" → motor_runtime_s=75 ✓
  → 파싱 성공, return TRUE ✓
```

---

## 변경 파일 목록

| # | 파일 | 종류 | 상태 |
|---|---|---|---|
| 1 | `PSOC/.../Design.cydsn/uartJson.c` | 펌웨어 C | ✅ 코드 완료 |

**전체 경로**: `C:\SUNTECH_DEV_CLAUDECODE\PSOC\280CTP_IoT_INTEGRATED\280CTP_IoT_INTEGRATED_V1_EMBROIDERY_S\Project\Design.cydsn\uartJson.c`

---

## 상세 변경 내용 (`uartJson.c`)

### Before (수정 전)

```c
if (c == '\0' || c == '\r' || c == '\n') continue;   // 버퍼 유지

if (g_UART_buff_index == 0 && (c < '0' || c > '9')) continue;

// [중간 문자 필터 없음]
```

### After (수정 후)

```c
// ① 줄 끝 → 버퍼 리셋 (MCSTATUS hex 잔류 제거)
if (c == '\r' || c == '\n' || c == '\0')
{
    if (g_UART_buff_index > 0)
    {
        g_UART_buff_index = 0;
        memset(g_UART_buff, 0, UART_BUFFER_SIZE);
    }
    continue;
}

// ② 패킷 시작: 숫자만 허용
if (g_UART_buff_index == 0 && (c < '0' || c > '9')) continue;

// ③ 패킷 중간: 숫자·';' 외 즉시 리셋 (0x?? 오염 차단)
if (g_UART_buff_index > 0 && (c < '0' || c > '9') && c != ';')
{
    g_UART_buff_index = 0;
    memset(g_UART_buff, 0, UART_BUFFER_SIZE);
    continue;
}
```

### printf 제거

```c
// uartEmbParsor() 내 — 제거됨
// printf("PARSE: actual_qty=%u ct=%lu tb=%u mrt=%lu\r\n", ...);
```

---

## 주의 사항

- **유효 패킷 영향 없음**: `2;82;0;75;\r\n` 형태의 유효 패킷은 4번째 `;` 도달 시 파싱+리셋이 먼저 일어남. `\r\n` 도달 시 `index=0` 상태이므로 `\r\n` 리셋 블록은 실행되지 않음.
- **빌드 필요**: PSoC Creator 에서 빌드 후 `.hex`/`.cyacd` 플래싱 필요.
- **EMBROIDERY_PLAN.md 섹션 11-3** 에 동일 내용 기록됨.

---

## 구현 진행 상황

- [x] 버그 원인 분석 완료 (2026-04-12)
- [x] `uartJson.c` 1차 수정 완료 (2026-04-12)
  - [x] `\r\n` 수신 시 버퍼 리셋 추가
  - [x] 패킷 중간 문자 필터 추가
  - [x] `printf("PARSE: ...")` 제거
- [x] `EMBROIDERY_PLAN.md` 업데이트 완료 (2026-04-12)
- [x] 빌드 후 실기기 테스트 (2026-04-15) → LCD UART TEST 텍스트 미출력 확인
- [x] **2차 원인 분석 시도** (2026-04-15) — ⚠️ 가설 오류
  - 당시 가설: 후행 `;` 없음 → 1차 수정이 `\r\n`에서 버퍼 버림
  - **실제 확인 결과**: 자수기는 `2;82;0;75;` (후행 `;` 있음) 전송 → 가설 틀림
  - 2차 수정 코드(sc==3 보완)는 방어적 코드로 유지 (해롭지 않음)
- [x] `uartJson.c` 2차 수정 완료 (2026-04-15)
  - [x] `\r\n` 핸들러에 sc==3 보완 파싱 추가 (후행 `;` 없는 경우 방어용으로 유지)
- [x] **3차 원인 재분석** (2026-04-15) — 미해결 원인 규명
  - 후행 `;` 있어도 실패 → 파싱 코드 외 문제
  - **유력 가설**: 자수기 부팅 메시지(500~700바이트)가 PSoC UART RX 소프트웨어 버퍼를 오버플로우 → `UART_SpiUartGetRxBufferSize()` 이상 동작 → `uartJsonLoop()` 진입 불가
  - 다음 플래싱 후 UART TEST 화면으로 진단 예정
- [x] `uartJson.c` 3차 수정 완료 (2026-04-15) — UART TEST 진단 강화
  - [x] `\r\n` 핸들러: sc==3 파싱 실패 시에도 `uartTestAddLine` 호출
  - [x] `\r\n` 핸들러: sc가 3이 아니더라도 세미콜론 ≥1이면 `uartTestAddLine` 호출
  - [x] `;` 핸들러(sc≥4): 파싱 실패 시에도 `uartTestAddLine` 호출
  - **목적**: 파싱 성패와 관계없이 세미콜론 포함 라인을 UART TEST에 표시 → 수신 여부·포맷 육안 확인
- [ ] **PSoC Creator 빌드 (재빌드)**
- [ ] **실기기 플래싱 및 UART TEST 화면 진단**
  - 아무것도 표시 안 됨 → PSoC UART RX Buffer Size 1024 이상으로 확대
  - 내용 표시됨 → 파싱 정상 또는 오염 내용 확인
- [ ] (필요 시) PSoC Creator UART 컴포넌트 RX Buffer Size 확대 후 재빌드·플래싱
- [ ] 최종 확인 후 이 파일 상태를 ✅ 완료로 변경

---

## 테스트 체크리스트

### 1단계: 플래싱 직후 진단 (가장 중요)

| 시나리오 | 기대 결과 | 확인 |
|---|---|---|
| 자수기 연결, 자수 완료 후 UART TEST 화면 | **뭔가 표시됨** (내용 불문) | ⬜ |
| UART TEST 화면에 `2;82;0;75;` 표시 | 파싱 성공 ✅ | ⬜ |
| UART TEST 화면에 아무것도 표시 안 됨 | UART 수신 불가 → RX Buffer 확대 필요 | ⬜ |

### 2단계: 파싱 정상 확인

| 시나리오 | 기대 결과 | 확인 |
|---|---|---|
| PC에서 `2;82;0;75;` 단독 전송 | 카운트 증가, 서버 전송 정상 | ⬜ |
| PC에서 `2;82;0;75` (후행 `;` 없음) 전송 | 카운트 증가 (sc==3 보완 처리) | ⬜ |
| MCSTATUS 직후 패킷 수신 | 파싱 성공, 데이터 정상 | ⬜ |
| 연속 패킷 (2번째, 3번째) | 모두 카운트 증가 | ⬜ |
