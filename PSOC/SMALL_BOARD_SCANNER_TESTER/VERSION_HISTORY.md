# SCANNER SMALL BOARD 펌웨어 버전 히스토리

> 대상 MCU: Cypress PSoC4 (CY8C4245AZI-M445, Cortex-M0)
> 개발툴: PSoC Creator 4.4 / ARM GCC 5.4
> 역할: 공장 생산라인 통신 브리지 (2D 스캐너 ↔ ST-500 Touch OP ↔ 태블릿)

---

## SMALL_BOARD_SCANNER_2026_V1

| 항목 | 내용 |
|------|------|
| **버전 식별자** | `2D_ST500_MonitoringPort` |
| **프로젝트 폴더** | `SMALL_BOARD_SCANNER_2026_V1/` |
| **워크스페이스** | `PatternBarcodeMonitoring.cywrk` |
| **최종 빌드일** | 2024-10-29 (성공) |
| **코드 개선일** | 2026-03-03 |

### 메모리 사용량

| 영역 | 사용 | 전체 | 점유율 |
|------|------|------|--------|
| Flash | 14,310 bytes | 32,768 bytes | 43.7% |
| SRAM  |  2,668 bytes |  4,096 bytes | 65.1% |

### UART 포트 구성 (모두 9600 bps, 8N2)

| 포트 | 연결 대상 | 방향 |
|------|-----------|------|
| `port_MONITORING` | ST-500 Touch OP | 인터럽트 RX / TX |
| `port_OP` | 태블릿 Android | TX (JSON 출력) |
| `BARCODE` | 2D 바코드 스캐너 | RX / TX |

### 주요 기능

- **바코드 수신 및 파싱**: 2D 스캐너로부터 `DesignNo/DesignIDX/Pieces` 형식의 데이터를 수신하여 파싱
  - `DesignNo` → Touch OP(`port_MONITORING`)로 전달 (`@@@@!...` 포맷)
  - `DesignIDX`, `Pieces` → 태블릿(`port_OP`)으로 JSON 전달
- **스캔 트리거 중계**: Touch OP가 스캔 명령(`$$$$#99900035;%%%%`)을 보내면 스캐너에 트리거 신호 전송
- **스캐너 연결 확인**: Touch OP의 모드 조회 명령(`$$$$#99900304;%%%%`)에 응답
- **카운터 이벤트**: 풋/핸드 스위치 입력(상승 엣지)에 따라 유효 구간(200ms~1900ms) 내 카운트 이벤트를 태블릿에 JSON 전송
- **부트로더 진입**: 내부 버튼(`Pin_StartBootloader`) LOW 감지 시 OTA 부트로더 진입 (115200 bps)
- **LED 동작 표시**: SysTick 1ms 인터럽트 기반, 1초 주기 LED 토글

### 변경 이력

#### 2020-03-10 (초기 릴리즈)
- 부트로더 + 2D 스캐너 + Touch OP 통합 구조 적용
- 하드웨어 인터럽트(HWI) 기반 Downtime 감지
- Count 핀: 2번, Power 핀: 8번
- 풋/핸드 스위치 케이블 연결 지원
- Tablet → `port_OP`, Touch OP + Scanner → `port_MONITORING` 구조 확립

#### 2021-05-28 (VJ/SCI/PWJ 적용)
- VJ, SCI, PWJ 공정 라인 적용 버전으로 업데이트
- 구형 버튼 OP 방식 제거 (ST-500 전용)
- ST-500 → Monitoring Port, Tablet → OP Port 구조 재정립

#### 2026-03-03 (코드 품질 개선)
- `receivedFromBarcodeCount` 타입을 `char` → `uint8_t` 로 변경 (배열 인덱스 타입 안전성)
- `strtok()` 반환 포인터(`ptrFirst`, `ptrSecond`, `ptrThird`) NULL 체크 추가 (크래시 방어)
- 매직 넘버 제거: 버퍼 크기·타임아웃·카운터 임계값 모두 `#define` 상수화
- 명령 문자열 `#define` 상수 분리 (`SCAN_TRIGER_ORDER_STR`, `SCAN_MODE_CMD_STR`)
- 미사용 변수(`count`, `oldBarcodeTriger`) 제거
- UART 수신 루프에서 항상 버퍼 소비 보장 (무한루프 방어)
- Copyright 연도 2026으로 갱신

---

*Copyright SUNTECH, 2018-2026*
