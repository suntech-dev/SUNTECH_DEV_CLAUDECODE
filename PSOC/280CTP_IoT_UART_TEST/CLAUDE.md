# CLAUDE.md — 280CTP_IoT_UART_TEST

> 기반: 280CTP_IoT_INTEGRATED CLAUDE.md 코딩 규칙 상속
> 추가 규칙: UART 테스트 버전 전용

---

## 프로젝트 목적

이 버전은 **UART 통신 테스트 전용**입니다.
WiFi, ANDON, 카운팅, 패턴 관리 기능은 모두 비활성화하고,
다음 3가지 핵심 기능만 구현합니다.

1. **UART 수신** — 외부 MCU로부터 데이터 수신
2. **LCD 표시** — 수신 데이터를 화면에 출력 + Up/Down 스크롤
3. **USB 출력** — 수신 데이터를 PC(USB CDC)로 동시 전송

---

## C 코드 스타일 가이드 (PSoC4 임베디드 C)

### 들여쓰기 & 중괄호

- **들여쓰기**: 스페이스 4칸 (탭 사용 금지)
- **중괄호**: Allman 스타일 — 함수 및 제어문 모두 중괄호를 다음 줄에 배치
- **단문 제어문도 반드시 중괄호** 사용

### 타입 안전성

- `char` 타입을 배열 인덱스나 카운터로 사용 **금지** → `uint8_t` 사용
- ISR에서 접근하는 전역 변수는 **반드시 `volatile`** 선언
- 버퍼 인덱스는 `uint16_t` 이상 사용 (1000 바이트 버퍼 시 `uint16` 필요)

### 버퍼 안전성

- `sprintf()` 대신 **`snprintf(buf, sizeof(buf), ...)`** 사용
- `vsprintf()` 대신 **`vsnprintf(buf, sizeof(buf), ...)`** 사용
  - `uart.c`의 `DEBUG_printf`/`DEBUG_Prompt_printf` 수정 대상
- UART 수신 버퍼 크기 초과 시 반드시 인덱스 리셋 처리

### 포인터 & NULL 안전성

- 포인터 반환 함수 결과는 반드시 NULL 체크
- 포인터 선언 형식: `type *name`

---

## UART 수신 규칙

> `uartJson.c`에 UART JSON 수신 및 파싱이 이미 **완전히 구현**되어 있습니다.
> 호출 경로: `main.c → CountFunc() → PatternCountLoop() → uartJsonLoop() → uartJsonParsor()`

- UART 버퍼 크기: `UART_BUFFER_SIZE` 매크로로 정의 (현재 **512 bytes** — `uartJson.c:21`)
- 수신 방식: **폴링 방식** (`UART_SpiUartGetRxBufferSize()`) — ISR 방식이 아님
- JSON 시작 감지: `{` 또는 `[` 문자로 판단, `}` 수신 시 파싱 실행
- 버퍼 오버플로우 방지 로직 적용 중:
  ```c
  if (g_UART_buff_index >= UART_BUFFER_SIZE - 1) { g_UART_buff_index = 0; continue; }
  ```
- **LCD 표시 코드**: `uartJsonParsor()` 내부에 주석 처리 상태 (`uartJson.c:103~122`)
  → 활성화 또는 스크롤 지원 뷰어로 재구현 필요
- **USB 출력 연결**: `DEBUG_printf(g_UART_buff)` 호출을 `uartJson.c`에 추가해야 함

---

## USB 출력 규칙

- USB CDC 출력은 `UART_USB_PutString()` 사용
- 포맷 출력은 `uart.c`의 `DEBUG_printf()` 사용
- USB CDC 초기화 전에 출력 시도 금지 (부팅 대기 필요)
  ```c
  // USB CDC 준비 대기 예시
  while (!USBUART_CDCIsReady()) {}
  ```
- 대용량 데이터 출력 시 TX 버퍼 확인 후 전송

---

## LCD 뷰어 규칙

- LCD 해상도: 320×240 (ST7789V, SPI)
- 폰트/이미지 배열에는 반드시 `const` 유지 — FLASH 배치를 위해
  ```c
  static const uint16_t image_xxx[] = { ... };  // const 필수
  ```
- 스크롤 위치 변수명: `g_scrollOffset` (static, 파일 범위)
- 화면 갱신 플래그: `g_bNeedRedraw` (volatile)
- 라인 버퍼 최대 크기는 SRAM 여유를 고려하여 결정
  - 현재 SRAM 목표: 32,768 bytes의 70% 이하

---

## 변수 및 함수 선언 규칙

- **전역 변수 최소화**: 파일 범위 `static` 우선 사용
- **미사용 변수/함수 즉시 제거** (주석 처리 후 방치 금지)
  - 기반 버전(INTEGRATED_V1)에서 가져온 WiFi/ANDON 관련 코드는 완전히 제거
- ISR 공유 변수: `volatile` + `static` 조합
- 빈 매개변수 함수: `void func(void)` 형태로 명시

---

## 비활성화/제거 대상 코드

이 버전에서는 다음 코드를 제거하거나 완전히 비활성화해야 합니다.

| 기능 | 파일 | 처리 방법 |
|------|------|---------|
| WiFi 초기화 | `setup.c` | `initWIFI()` 호출 제거 |
| WiFi 루프 | `main.c` | `wifiLoop()` 제거 |
| ANDON API | `main.c`, `andonApi.c` | 호출 제거 |
| USB JSON 파서 | `main.c` | `usbJsonParsorLoop()` 제거 |
| 전류 센서 | `main.c` | `currentSensorRoutine()` 제거 |
| 가동 시간 카운팅 | `main.c` | `WorkingTimeCount()` 제거 |
| TrimPin 처리 | `main.c` | 제거 |

> **주의**: `CountFunc()` 는 제거하면 안 됩니다.
> `PatternCountLoop() → uartJsonLoop()` 호출 체인의 진입점으로 **UART 수신에 필수**입니다.

---

## 메모리 관리 규칙

- `lib/image.h`, `lib/fonts.h` 배열에는 반드시 `const` 유지
- WiFi 관련 대형 버퍼 (`g_WIFI_ReceiveBuffer[2048]`) 비활성화로 SRAM 절감
- SRAM 목표: 32,768 bytes의 70% 이하 유지
- UART 수신 버퍼(1000 bytes)를 추가하더라도 전체 사용량 70% 이하 유지 목표

---

## 버전 히스토리 관리 (`VERSION_HISTORY.md`)

- 버전 히스토리 파일: `VERSION_HISTORY.md` (프로젝트 루트)
- 새 작업이 시작될 때 `VERSION_HISTORY.md`에 해당 버전 섹션 추가
- 섹션 형식:

```
## 280CTP_IoT_UART_TEST_VN

| 항목 | 내용 |
...

### 메모리 사용량
### 하드웨어 구성
### 주요 기능
### 변경 이력
```

---

## README.md 버전별 업데이트 규칙

새 버전이 추가될 때 `README.md`의 아래 항목을 함께 갱신한다.

1. 문서 헤더 메타데이터 (분석 버전, 마지막 빌드일)
2. 섹션 2 — 버전 구조 폴더 트리
3. 섹션 8 — 알려진 이슈 (수정 완료 처리)
4. 섹션 9 — 버전 이력 테이블

---

## Copyright

- 모든 소스 파일 상단에 Copyright 블록 포함
- 형식: `Copyright SUNTECH, YYYY`
