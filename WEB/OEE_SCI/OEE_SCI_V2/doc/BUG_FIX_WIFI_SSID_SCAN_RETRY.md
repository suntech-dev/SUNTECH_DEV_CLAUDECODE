# BUG FIX: WiFi SSID 스캔 실패 — 저장 SSID 재탐색 로직 추가

## 상태: ❌ Phase 2 현장 테스트 실패 — UART 로그 확인 후 재검토 필요 (2026-04-24)

---

## 버그 요약

| 항목 | 내용 |
|---|---|
| 심각도 | **HIGH** |
| 발견일 | 2026-04-24 |
| 영향 범위 | EMBROIDERY_S_115200 WiFi 연결 불안정 현장 전체 |
| 증상 | 부팅 시 WiFi 연결 실패 — 재부팅 반복 시 가끔 성공 |

---

## 근본 원인

### AT 커맨드 매뉴얼 분석 결과 (WF5000 v6.2)

- `AT*ICT*SCAN` — 파라미터 없음, 모든 채널 스캔
- `AT*ICT*SCONN=<essid> [passphrase]` — 모듈이 내부적으로 랜덤 순서 스캔 후 접속

WF5000 모듈은 부팅 시 저장된 프로파일로 자동 접속(`auto-connect`)을 시도한다.  
이 내부 스캔이 확률적으로 동작하여, ISCI가 탐색 범위를 벗어나면 `ICT*ASSOCIATED: 2` (AP not found)를 반환한다.

### 기존 코드 문제 (`WIFI.c` — `wifiConnectAP()` case 2)

```c
case 2: printf("AP is not found\r\n"); break;  // ← 출력만 하고 영구 대기!
```

`ICT*ASSOCIATED: 2` 수신 시 재시도 없이 case 2에서 영구 대기.  
사용자가 재부팅을 반복해야 하는 이유.

### `MAX_NO_OF_ACCESS_POINT = 10` 과는 무관

이 상수는 USB 설정 도구의 AP 목록 표시 버퍼일 뿐이며 실제 WiFi 접속에 영향 없음.

---

## 해결 방안: Option 2 — 저장 SSID 우선 재스캔

```
부팅 자동접속 실패 (ICT*ASSOCIATED: 2)
 │
 └─ AT*ICT*SCAN 실행
     ├─ ICT*SCAN:OK → g_SizeOfAPs = 0 (리셋)
     ├─ ICT*SCANIND: → appendAP() 누적
     └─ ICT*SCANRESULT → 저장 SSID 검색
         ├─ 찾음 → AT*ICT*SCONN=SSID password → case 2로 복귀
         └─ 못 찾음 → 재스캔 (최대 3회)
             └─ 3회 후에도 없음 → SCONN fallback
```

### Phase 2 ✅ — 채널 지정 직접 접속 (2026-04-24 구현)

스캔 결과에서 채널 정보를 추출 후 `AT*ICT*ASSOCIATE=SSID {channel}` 사용.  
SCONN은 모듈 내부에서 랜덤 순서로 재스캔하기 때문에 실패 가능.  
ASSOCIATE는 지정 채널로 직접 접속 → 모듈 내부 랜덤 스캔 생략.

---

## 변경 파일

| # | 파일 | 상태 |
|---|---|---|
| 1 | `PSOC/.../EMBROIDERY_S_115200/lib/WIFI.c` | 🔧 구현 중 |

---

## 상세 변경 내용

### `wifiConnectAP()` 수정

**추가 변수:**
```c
static uint8 g_scanRetryCount = 0;
```

**case 2 수정 — AP not found 시 스캔 트리거:**
```c
case 2:
    printf("AP is not found\r\n");
    printf("[WIFI] Scanning for %s...\r\n", g_ptrServer->SSID);
    g_scanRetryCount = 0;
    g_SizeOfAPs = 0;
    WIFI_CMD("AT*ICT*SCAN");
    nLoop = 4;  // 새 스캔 케이스로 이동
    break;
```

**WIFI.h — `ACCESS_POINT` 구조체에 `channel` 추가:**
```c
typedef struct {
    int16 RSSI;
    uint8 channel;   // ← Phase 2 추가
    char SSID[MAX_STRING_SSID];
    char MAC[18];
} ACCESS_POINT;
```

**`appendAP()` — 채널 파싱 추가:**

`ICT*SCANIND:` 응답 형식: `SSID BSSID(AA:BB:CC:DD:EE:FF) CHANNEL AUTHMODE CIPHER RSSI`

```c
// get Channel
ptr++;
char chanBuf[8] = {0};
while(*ptr != ' ') chanBuf[i++] = *ptr++;
ptrAP->channel = (uint8)atoi(chanBuf);
// Skip AuthMode, Cipher
ptr++; while(*ptr != ' ') ptr++;
ptr++; while(*ptr != ' ') ptr++;
// get RSSI
ptr++;
ptrAP->RSSI = atoi(ptr);
```

**case 4 — 발견 시 `ASSOCIATE` 채널 직접 지정 (Phase 2):**
```c
if(foundIdx >= 0)
{
    uint8 ch = g_APs[foundIdx].channel;
    printf("[WIFI] [%s] found CH=%d! Associating...\r\n", g_ptrServer->SSID, ch);
    wifi_printf("AT*ICT*ASSOCIATE=%s %d\r\n", g_ptrServer->SSID, ch);  // SCONN → ASSOCIATE
    g_scanRetryCount = 0;
    nLoop = 2;
}
else if(g_scanRetryCount < 3)
{
    g_scanRetryCount++;
    WIFI_CMD("AT*ICT*SCAN");  // 재스캔
}
else
{
    // 3회 실패 시 SCONN fallback
    wifi_printf("AT*ICT*SCONN=%s %s\r\n", g_ptrServer->SSID, g_ptrServer->password);
    g_scanRetryCount = 0;
    nLoop = 2;
}
```

---

## 테스트 체크리스트

| 시나리오 | 기대 결과 | 확인 |
|---|---|---|
| 정상 부팅 (ISCI 자동접속 성공) | 기존과 동일, case 4 진입 안 함 | ⬜ |
| ISCI 자동접속 실패 → 1회 스캔에서 발견 | 스캔 후 SCONN, 재부팅 없이 연결 | ⬜ |
| ISCI 2~3회 스캔 후 발견 | 재스캔 후 연결 | ⬜ |
| ISCI 3회 스캔에도 없음 (AP 범위 밖) | fallback SCONN 시도 후 실패 | ⬜ |
| UART 로그: 재스캔 횟수 확인 | `[WIFI] not found, retry scan N/3` 출력 | ⬜ |

---

## 구현 진행 상황

- [x] 원인 분석 (2026-04-24)
- [x] AT 커맨드 매뉴얼 분석 (2026-04-24)
- [x] WIFI.c 코드 수정 Phase 1 — SCONN 재시도 (2026-04-24)
- [x] Phase 1 빌드·현장 테스트 — **연결 실패** (2026-04-24)
- [x] WIFI.c Phase 2 — ASSOCIATE 채널 직접 접속 구현 (2026-04-24)
  - WIFI.h: ACCESS_POINT에 channel 필드 추가
  - appendAP(): ICT*SCANIND 파싱에서 채널 값 추출
  - case 4: 발견 시 AT*ICT*ASSOCIATE=SSID channel 사용
- [x] Phase 2 빌드·현장 테스트 — **연결 실패** (2026-04-24)
- [ ] **UART 케이블 연결 후 로그 캡처** → 원인 재분석
- [ ] Phase 3 대응

---

## 미해결 — 다음 세션 재개 시작점

### UART 로그 확인 체크리스트

케이블 연결 후 아래 순서로 로그를 확인한다.

**① case 4 진입 여부**
```
[WIFI] AP not found. Scanning for [iSCi]...
```
이 줄이 안 나오면 → `ICT*ASSOCIATED: 2` 자체가 수신되지 않거나, `wifiConnectAP()` 가 호출되지 않는 것.  
`wifiLoop()` 에서 `g_network.isConnectAP` 가 TRUE 로 잘못 세팅됐을 가능성.

**② SCANIND 원문 캡처**
```
LOOP[4]:ICT*SCANIND: ??? ]
```
이 줄의 `???` 부분을 그대로 캡처한다.  
→ `appendAP()` 파싱이 이 형식을 기준으로 만들어졌기 때문에 형식이 다르면 SSID/채널 파싱 모두 틀림.

**③ AP 파싱 결과 확인**
```
[AP] iSCi MAC=AA:BB:CC:DD:EE:FF CH=6 RSSI=-65
```
- SSID가 `iSCi` 가 아닌 다른 값 → `i-3` 트리밍 위치 오류
- `CH=0` → 채널 파싱 필드 위치 오류
- 이 줄 자체가 없음 → `ICT*SCANIND:` 수신 안 됨 또는 `appendAP()` 조기 리턴

**④ ASSOCIATE 명령 결과**
```
[WIFI] [iSCi] found CH=6! Associating...
```
이후 `ICT*ASSOCIATED: ?` 응답값 확인. 2가 다시 나오면 ASSOCIATE 명령 자체가 WF5000에서 지원되지 않거나 문법이 다른 것.

### 의심 원인 후보 (우선순위 순)

| 순위 | 의심 원인 | 확인 방법 |
|---|---|---|
| 1 | `ICT*SCANIND:` 형식이 예상과 달라 SSID 파싱 실패 → `iSCi` 못 찾음 | UART ② 캡처 |
| 2 | `AT*ICT*ASSOCIATE` 명령이 WF5000 펌웨어에서 미지원 | UART ④ 응답 확인 |
| 3 | case 4 자체 미진입 — `wifiConnectAP()` 가 불리지 않음 | UART ① 확인 |
| 4 | SSID 대소문자 불일치 (`iSCi` vs `iSCI` 등) | UART ③ SSID 값 확인 |
