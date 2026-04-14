# BUG FIX: EMBROIDERY_S — 자수기 UART 패킷 전부 소실

## 상태: 🧪 테스트 대기 중 (코드 구현 완료, 빌드 미실시)

---

## 버그 요약

| 항목 | 내용 |
|---|---|
| 심각도 | **CRITICAL** |
| 발견일 | 2026-04-12 |
| 영향 범위 | `280CTP_IoT_INTEGRATED_V1_EMBROIDERY_S` 프로젝트 전체 |
| 증상 | 자수기 실기기 연결 시 카운트 증가 없음 — 데이터 패킷 전부 수신 불가 |
| 단독 테스트 | PC에서 `2;82;0;75;` 단독 전송 시 정상 동작 (문제 재현 안 됨) |

---

## 근본 원인

### 자수기 UART 출력 구조

자수기는 데이터 패킷 직전에 **MCSTATUS 줄을 전송할 수 있다** (항상은 아님).

```
MCSTATUS Periodic Data : 0x41,0xff\r\n   ← hex 값은 매번 달라짐
2;82;0;75;\r\n                           ← 실제 데이터 패킷
MCSTATUS Periodic Data : 0xAB,0x12\r\n
2;81;0;76;\r\n
...
```

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
- [x] `uartJson.c` 수정 완료 (2026-04-12)
  - [x] `\r\n` 수신 시 버퍼 리셋 추가
  - [x] 패킷 중간 문자 필터 추가
  - [x] `printf("PARSE: ...")` 제거
- [x] `EMBROIDERY_PLAN.md` 업데이트 완료 (2026-04-12)
- [ ] **PSoC Creator 빌드**
- [ ] **실기기 플래싱 및 자수기 연결 테스트**
- [ ] 최종 확인 후 이 파일 상태를 ✅ 완료로 변경

---

## 테스트 체크리스트

| 시나리오 | 기대 결과 | 확인 |
|---|---|---|
| PC에서 `2;82;0;75;` 단독 전송 | 카운트 증가, 서버 전송 정상 | ⬜ |
| 자수기 전원 ON 후 데이터 수신 | 카운트 증가, 에러 없음 | ⬜ |
| MCSTATUS 직후 패킷 수신 | 파싱 성공, 데이터 정상 | ⬜ |
| 연속 패킷 (2번째, 3번째) | 모두 카운트 증가 | ⬜ |
| LCD UART TEST 뷰어 표시 | 수신 문자열 정상 표시 | ⬜ |
