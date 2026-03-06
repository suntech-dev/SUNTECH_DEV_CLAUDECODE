# KVC BOBBIN RESET COUNTER 펌웨어 버전 히스토리

> 대상 MCU: Cypress PSoC 4 (CY8C42xx 시리즈, Cortex-M0)
> 개발툴: PSoC Creator 4.x / ARM GCC 5.4
> 역할: 봉제 기계 보빈 교체 주기 트림 카운팅 및 알림 시스템

---

## 280CTP2_KVC_BOBBIN_RESET_COUNTER_V1

| 항목 | 내용 |
|------|------|
| **버전 식별자** | `KVC Bobbin Reset V1` |
| **프로젝트 폴더** | `280CTP2_KVC_BOBBIN_RESET_COUNTER_V1/` |
| **워크스페이스** | `Design.cydsn/` |
| **작업일** | 2026-03-05 |
| **상태** | 개발 중 — 이슈 수정 필요 |

### 메모리 사용량

> 빌드 리포트 미확인 상태 (PSoC Creator 빌드 후 갱신 필요)

| 영역 | 사용 | 전체 | 점유율 |
|------|------|------|--------|
| Flash (Bootloader) | - | 262,144 bytes | - |
| Flash (Application) | - | 262,144 bytes | - |
| **SRAM** | **미측정** | **32,768 bytes** | **미측정** |

> 참고: WiFi/JSON/서버 기능 포함 상태. `image.h` 비활성화 시 SRAM 50% 절감 가능 (주석 참조).

### 하드웨어 구성

| 컴포넌트 | 역할 |
|---------|------|
| `TC_INT` | 트림 카운트 신호 입력 (봉제기 실 자르기 감지) |
| `RESET_KEY` | 물리적 RESET 버튼 |
| `BUZZER` | 알림음 출력 |
| `SPIM_LCD` | ST7789V LCD SPI (320×240, LANDSCAPE) |
| `I2C_TC` | FT5x46 터치 컨트롤러 |
| `SPIM_FLASH` | W25Qxx 외부 Flash (설정 저장) |
| `UART` | 디버그 출력 |
| `WIFI` | WiFi 모듈 (현재 미활성화) |
| `USBUART` | USB CDC 디버그 |
| `RESERVED_OUT_1` | 트림 완료 외부 신호 출력 |
| `LED1_R/G`, `LED2_R/G/B` | RGB LED (총 2개) |
| `TCPWM_LED` | LED 밝기 PWM 제어 |

### 주요 기능

- **트림 카운터**: TC_INT ISR(`Trim_Interrupt_Routine`)로 카운트 증가, 내부 EEPROM 자동 저장
- **LCD 터치 UI**: LANDSCAPE 방향, SET/RESET 버튼 + 목표값/현재값 대형 폰트 표시
- **목표 도달 알림**: `count == setTrimCount` 시 부저 경보 + `RESERVED_OUT_1` HIGH
- **목표값 설정**: `doTargetInfoMenu()` — +1/+10 단위 조정, External Flash 저장
- **RESET 기능**: 물리 버튼(`RESET_KEY`) 또는 터치 RESET 버튼으로 카운터 초기화
- **LED 제어**: WiFi 신호 강도별 색상 변경, 점멸 지원
- **데이터 지속성**: 내부 EEPROM(카운트), 외부 Flash(머신 파라미터) 이중 저장
- **WiFi/서버 기능**: 코드 포함되어 있으나 `USE_WIFI` 미정의로 비활성화 상태
- **조건부 컴파일**: `package.h`의 `USER_PROJECT_TRIM_COUNT` 매크로로 프로젝트 타입 선택

### 알려진 문제점

| 심각도 | 파일 | 문제 |
|--------|------|------|
| **심각** | `count.c:33` | ISR 변수 `volatile` 미선언 — 컴파일러 최적화로 갱신 무시 가능 |
| **심각** | `uart.c:22` | `vsprintf()` 버퍼 크기 제한 없음 — 스택 오버플로우 위험 |
| **높음** | `count.c:104` | `CY_ISR(Trim_Interrupt_Routine)` 중복 정의 |
| **높음** | `externalFlash.c:98` | `checkSum()` `i=1` 시작 — 첫 바이트 누락 |
| **높음** | `internalFlash.c:20` | EEPROM 무결성 검증 코드 주석 처리 |
| **중간** | `main.h:59` | 전역 구조체 인스턴스 헤더 선언 |
| **중간** | `setup.c:54,65` | `initLEDControl()` 중복 호출 |
| **낮음** | `count.c:65` | `CyDelay(50)` busy-wait 디바운싱 |
| **낮음** | `main.h:22` / `package.h:27` | 버전 정보 불일치 |
| **낮음** | `IODefine.c` | I/O 매핑 전체 주석 처리 (Dead code) |

### 변경 이력

#### 2026-03-05 (초기 구현 분석)
- V1 프로젝트 최초 코드 분석
- 트림 카운터 + LCD UI + WiFi 구조 확인
- 10개 이슈 발굴 및 문서화
- CLAUDE.md, README.md, VERSION_HISTORY.md 초기 작성

---

*Copyright SUNTECH, 2023-2026*
