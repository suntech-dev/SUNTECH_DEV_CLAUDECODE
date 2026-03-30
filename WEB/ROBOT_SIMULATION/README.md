# ROBOT_SIMULATION — 프로젝트 분석 문서

> 최초 작성: 2026-03-27
> 분석 버전: ROBOT_SIMULATION_V1
> 마지막 업데이트: 2026-03-27

---

## 1. 프로젝트 개요

Co-Robot 1대와 재봉기 2~3대의 통합 작업 흐름을 시뮬레이션하는 웹 애플리케이션.
순수 HTML/CSS/JavaScript로 구현되어 별도 빌드 도구나 서버 없이 동작합니다.

| 주요 기능 | 설명 |
| --------- | ---- |
| 모드 선택 | 시작 시 재봉기 수량(2대/3대) 선택 |
| 빠른계산 | 렌더링 없이 즉시 시뮬레이션 결과 계산 |
| 실시간계산 | RAF 루프 기반 애니메이션 시각화 |
| 시나리오 비교 | 다수 시나리오 저장·삭제·비교 테이블·차트 |
| 결과 차트 | 생산량/대기시간 막대차트, 가동률 도넛차트 |
| PDF 내보내기 | jsPDF + autoTable 기반 리포트 생성 |
| 로봇 속도 선택 | 75%/80%/85%/90%/95%/100% 선형 보간 |
| Pallet 큐 시스템 | 자재 준비 사이클 시뮬레이션 |
| 영문 버전 | `eng/` 폴더 내 완전 영문 UI 제공 |

---

## 2. 버전 구조

```
ROBOT_SIMULATION/
├── CLAUDE.md               # 프로젝트 코딩 규칙
├── README.md               # 이 문서
├── README.html             # README HTML 변환본
├── VERSION_HISTORY.md      # 버전 이력
└── ROBOT_SIMULATION_V1/    # 현재 운영 버전
    ├── index.html
    ├── DOCUMENT.md
    ├── css/
    │   └── style.css
    ├── js/
    │   ├── config.js
    │   ├── engine.js
    │   ├── quickCalc.js
    │   ├── renderer.js
    │   ├── realtimeCalc.js
    │   ├── charts.js
    │   ├── scenario.js
    │   ├── pdf.js
    │   └── main.js
    ├── image/
    │   ├── icon_robot.png
    │   └── pallet.png
    └── eng/                # 영문 버전
        ├── index.html
        └── js/ (동일 구성)
```

---

## 3. 기술 스택

| 구분 | 기술 |
| ---- | ---- |
| 언어 | HTML5, CSS3, JavaScript (ES6+) |
| 서버 | 없음 (순수 정적 웹) |
| 차트 | Chart.js 4.4.1 (CDN) |
| PDF | jsPDF 2.5.1 + jsPDF-autoTable 3.8.2 (CDN) |
| 빌드 | 없음 |
| 프레임워크 | 없음 |
| 시뮬레이션 방식 | 이벤트 기반 이산 사건 시뮬레이션 (Discrete Event Simulation) |

---

## 4. 소스코드 상세 분석

### 4.1 모듈 의존성

```
index.html
  └── css/style.css
  └── js/config.js       ─── 입력값 검증/제공 + 재봉기 수량 관리
        └── js/engine.js ─── 시뮬레이션 엔진 (핵심)
              ├── js/quickCalc.js    (렌더링 없는 즉시 실행)
              └── js/realtimeCalc.js (RAF 루프, 타임라인 seek)
                    └── js/renderer.js  (DOM 기반 시각화)
                          └── js/charts.js    (Chart.js 결과 차트)
                                └── js/scenario.js (시나리오 비교)
  └── js/pdf.js          ─── PDF 리포트 내보내기
  └── js/main.js         ─── 컨트롤러 (모드 선택 + 이벤트 바인딩)
```

### 4.2 config.js — 설정 관리

| 항목 | 설명 |
| ---- | ---- |
| `MACHINE_IDS` | `[1,2]` 또는 `[1,2,3]` — in-place 수정으로 전 모듈 자동 반영 |
| `getConfig()` | 입력값 읽기 + 유효성 검증 → `{ valid, errors, machines }` |
| `SPEED_DATA` | 75%/85%/100% 속도별 재봉기별 작업시간 하드코딩값 |
| `getSpeedValues(%)` | 구간 선형 보간 (75~85%, 85~100%), 소수점 1자리 반올림 |
| `readBasicSettings()` | workerCount, palletPrepTime, palletCount 반환 |

### 4.3 engine.js — 시뮬레이션 엔진

**로봇 상태:**

| 상태 | 설명 |
| ---- | ---- |
| `AT_TABLE_IDLE` | 자재테이블 대기 |
| `TRAVELING_TO_MACHINE` | 재봉기로 이동 중 |
| `TRAVELING_TO_TABLE` | 테이블로 복귀 중 |

**스케줄링 우선순위:**
1. `DONE_WAITING` 재봉기 → 가장 오래 기다린 재봉기 우선
2. `EMPTY` 재봉기 → 번호순 초기 적재
3. 모두 `SEWING` 중 → 가장 먼저 끝날 재봉기까지 대기

**Pallet 큐 시스템:**
- 초기 준비 pallet = 재봉기 수량 (t=0에 준비 완료)
- 추가 pallet = `palletCount - machineCount`
- Pallet 없음 + DONE_WAITING → Unload-only 모드 (빈 손으로 이동, unload만 수행)

### 4.4 renderer.js — 시각화

- 동적 좌표 계산 (2대/3대 모드별 재봉기 위치)
- Pallet 정보 오버레이 표시
- 로봇 이동 애니메이션 (CSS transform)

### 4.5 charts.js — 결과 차트

| 차트 | 종류 | 데이터 |
| ---- | ---- | ------ |
| 생산량 비교 | 막대차트 | 재봉기별 생산량 |
| 대기시간 비교 | 막대차트 | 재봉기별 누적 대기시간 |
| 가동률 | 도넛차트 | 로봇 가동률 % |

---

## 5. 사용자 입력 파라미터

### 기본 설정

| 항목 | ID | 기본값 |
| ---- | -- | ------ |
| 작업인원 | `worker-count` | 2명 |
| Pallet 준비시간 | `pallet-prep-time` | 58초 |
| Pallet 수량 | `.pallet-count-btn.active` | 2대:5개 / 3대:6개 |

### 재봉기별 작업시간 (85% 속도 기본값)

| 재봉기 | 초기 Load | UL&Load | 복귀 | 재봉 |
| ------ | --------- | ------- | ---- | ---- |
| M1 | 11초 | 15.5초 | 9초 | 63초 |
| M2 | 11.8초 | 16.3초 | 9.8초 | 63초 |
| M3 | 11초 | 15.5초 | 9초 | 63초 |

### 시뮬레이션 시간

- `sim-hours` (1~24시간) + `sim-minutes` (0~59분)
- 총 시간 = 시간×3600 + 분×60 (초)

---

## 6. 로컬 개발 환경 설정

```
C:\SUNTECH_DEV_CLAUDECODE\WEB\ROBOT_SIMULATION\ROBOT_SIMULATION_V1\
→ http://localhost/dev/ROBOT_SIMULATION/ROBOT_SIMULATION_V1/
```

- 서버 불필요 — 브라우저에서 `index.html` 직접 열람 가능
- Laragon 환경: `WEB\` = `http://localhost/dev/` (1:1 대응)

---

## 7. 발견된 이슈 및 개선 이력

| 날짜 | 내용 |
| ---- | ---- |
| 2026-03-27 | ROBOT_SIMULATION_V1 초기 분석 및 문서화 완료 |

---

## 8. 테스트 시나리오

| 시나리오 | 조건 | 확인 항목 |
| -------- | ---- | --------- |
| 2대 모드 기본 | 재봉기 2대, 85% 속도, 1시간 | 생산량 / 대기시간 / 가동률 정상 출력 |
| 3대 모드 기본 | 재봉기 3대, 85% 속도, 1시간 | 3대 모두 스케줄링 정상 동작 |
| 속도 변경 | 75%/100% 선택 | 작업시간 자동 입력 및 readonly 상태 |
| 직접입력 | 속도 선택 해제 | 모든 입력 필드 편집 가능 |
| Pallet 수량 | 3~7개 선택 | Pallet 큐 흐름 정상 동작 |
| 실시간 계산 | 애니메이션 모드 | 로봇 이동 시각화 및 seek 기능 |
| PDF 내보내기 | 결과 표시 후 | PDF 정상 생성 |
| 시나리오 비교 | 2개 이상 저장 | 비교 테이블 및 차트 정상 표시 |
| 영문 버전 | `eng/index.html` | UI 전체 영문 정상 표시 |

---

## 9. 버전 이력 테이블

| 버전 | 작업일 | 주요 변경 | 상태 |
| ---- | ------ | --------- | ---- |
| ROBOT_SIMULATION_V1 | 2026-03-27 | 초기 버전 — 2대/3대 모드, 이산 사건 시뮬레이션, Pallet 큐 시스템, 영문 버전 포함 | 운영 |

---

## 10. 관련 파일 경로 빠른 참조

| 파일 | 경로 |
| ---- | ---- |
| 메인 진입점 | `ROBOT_SIMULATION_V1/index.html` |
| 기술 상세 문서 | `ROBOT_SIMULATION_V1/DOCUMENT.md` |
| 시뮬레이션 엔진 | `ROBOT_SIMULATION_V1/js/engine.js` |
| 설정 관리 | `ROBOT_SIMULATION_V1/js/config.js` |
| 시각화 렌더러 | `ROBOT_SIMULATION_V1/js/renderer.js` |
| 실시간 계산 | `ROBOT_SIMULATION_V1/js/realtimeCalc.js` |
| 영문 버전 | `ROBOT_SIMULATION_V1/eng/index.html` |
| 버전 이력 | `VERSION_HISTORY.md` |
| 로컬 URL (V1) | `http://localhost/dev/ROBOT_SIMULATION/ROBOT_SIMULATION_V1/` |
