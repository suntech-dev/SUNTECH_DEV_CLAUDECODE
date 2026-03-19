# OEE_SCI 프로젝트 분석 문서

> 최초 작성: 2026-03-06
> 분석 버전: OEE_SCI_V2
> 마지막 업데이트: 2026-03-19 (AI Dashboard v4 · v5 신규 생성 및 버그 수정)

---

## 1. 프로젝트 개요

이 프로젝트는 **SAP Fiori 기반 OEE(Overall Equipment Effectiveness) 통합 모니터링 시스템**입니다.
인도네시아 봉제 공장의 **패턴재봉기(IoT 장비)** 및 **일반재봉기**를 대상으로 실시간 생산 효율, 비가동, 불량, 안돈 데이터를 수집하고 모니터링합니다.

### 주요 기능 요약

| 기능 | 설명 |
| ---- | ---- |
| OEE 모니터링 | 가용성(Availability), 성능(Performance), 품질(Quality) 실시간 계산 및 표시 |
| SSE 실시간 스트리밍 | Server-Sent Events 기반 데이터 스트리밍 (폴링 없음) |
| IoT API 통신 | 재봉기 장비의 MAC 주소 기반 자동 등록 및 데이터 수신 |
| 비가동 관리 | 비가동 사유 등록, 완료 처리, 지속 시간 자동 계산 |
| 안돈 관리 | 경보 발생 및 완료 처리 |
| 불량 관리 | 불량 유형별 등록 및 통계 |
| 마스터 데이터 관리 | 공장, 라인, 기계, 모델, 근무시간 설정 |
| 엑셀 내보내기 | PhpSpreadsheet 기반 데이터 내보내기 |
| 대시보드 | 종합 현황 페이지 |
| 보고서 | OEE 리포트 생성 |

---

## 2. 버전 구조

```
OEE_SCI/
├── CLAUDE.md                       # 프로젝트 코딩 규칙
├── README.md                       # 이 문서
├── README.html                     # HTML 변환 문서
├── VERSION_HISTORY.md              # 버전 이력
└── OEE_SCI_V1/                     # 첫 번째 버전
    ├── index.php                   # 진입점 (리다이렉트)
    ├── opcache.php                 # OPcache 상태 확인
    ├── .env                        # 환경변수 (로컬 개발용, git 제외)
    ├── api/                        # IoT 장비 API
    │   ├── sewing.php              # API 라우터
    │   └── sewing/                 # API 개별 엔드포인트
    │       ├── start.php           # 장비 등록/갱신
    │       ├── get_andonList.php   # 안돈 목록 조회
    │       ├── get_downtimeList.php # 비가동 목록 조회
    │       ├── get_defectiveList.php # 불량 목록 조회
    │       ├── get_dateTime.php    # 서버 시간 조회
    │       ├── send_pCount.php     # 생산량 전송 + OEE 계산
    │       ├── send_andon_*.php    # 안돈 발생/완료
    │       ├── send_downtime_*.php # 비가동 발생/완료
    │       └── send_defective_warning.php # 불량 발생
    ├── assets/                     # 정적 자원
    │   ├── css/                    # Fiori CSS, DateRangePicker CSS
    │   └── js/                     # jQuery, Chart.js, 공통 JS 모듈
    ├── dashboard/                  # 대시보드 (개발 중)
    ├── inc/                        # 공통 include 파일
    │   ├── head.php                # HTML head + CSS 로드
    │   ├── nav-fiori.php           # 사이드바 네비게이션
    │   └── foot.php                # 공통 푸터
    ├── lib/                        # 비즈니스 로직 라이브러리
    │   ├── config.php              # DB 설정 (.env 우선)
    │   ├── db.php                  # PDO 연결 + 에러 처리
    │   ├── api_helper.lib.php      # API 공통 헬퍼 클래스
    │   ├── database_helper.lib.php # DB 조작 헬퍼 클래스
    │   ├── worktime.lib.php        # 근무시간 Worktime 클래스
    │   ├── worktime_common.php     # 근무시간 공통 함수
    │   ├── worktime_database.php   # 근무시간 DB 조작
    │   ├── get_shift.lib.php       # 현재 교대조 탐색
    │   └── validator.lib.php       # 입력값 검증 클래스
    ├── logs/                       # 오류 및 API 로그 (자동 생성)
    ├── page/
    │   ├── data/                   # 데이터 조회 페이지
    │   │   ├── data_oee.php        # OEE 모니터링
    │   │   ├── data_andon.php      # 안돈 현황
    │   │   ├── data_downtime.php   # 비가동 현황
    │   │   ├── data_defective.php  # 불량 현황
    │   │   ├── dashboard.php       # 통합 대시보드
    │   │   ├── log_oee.php         # OEE 로그
    │   │   ├── log_oee_hourly.php  # 시간별 OEE 로그
    │   │   ├── log_oee_row.php     # 행별 OEE 로그
    │   │   ├── report.php          # 보고서
    │   │   ├── proc/               # SSE 스트리밍 + 엑셀 내보내기
    │   │   ├── js/                 # 페이지별 JavaScript
    │   │   └── css/                # 페이지별 CSS
    │   └── manage/                 # 마스터 데이터 관리 페이지
    │       ├── info_factory.php    # 공장 관리
    │       ├── info_line.php       # 라인 관리
    │       ├── info_machine.php    # 기계 관리
    │       ├── info_machine_model.php # 모델 관리
    │       ├── info_andon.php      # 안돈 유형 관리
    │       ├── info_downtime.php   # 비가동 유형 관리
    │       ├── info_defective.php  # 불량 유형 관리
    │       ├── info_design_process.php # 디자인 공정 관리
    │       ├── info_rate_color.php # OEE 색상 기준 관리
    │       ├── info_worktime.php   # 근무시간 관리
    │       ├── proc/               # CRUD 처리 (AJAX)
    │       ├── js/                 # 페이지별 JavaScript
    │       └── css/                # 페이지별 CSS
    └── upload/                     # 파일 업로드 디렉토리
```

---

## 3. 기술 스택

| 구분 | 기술 | 버전 |
| ---- | ---- | ---- |
| 서버 언어 | PHP | 7.4+ |
| 데이터베이스 | MySQL | 5.7+ |
| DB 접속 | PDO | - |
| UI 프레임워크 | SAP Fiori Horizon Light | - |
| 차트 라이브러리 | Chart.js | CDN |
| 날짜 라이브러리 | moment.js + daterangepicker | 번들 |
| jQuery | jQuery | 3.6.1 |
| 엑셀 내보내기 | PhpSpreadsheet | Composer 설치 |
| 실시간 통신 | Server-Sent Events (SSE) | - |
| 타임존 | Asia/Jakarta (UTC+7) | - |
| 운영 서버 | 49.247.26.228 | /var/www/html |

---

## 4. 소스코드 상세 분석

### 4.1 lib/config.php — DB 설정

- `.env` 파일이 있으면 우선 로드 (KEY=VALUE 형식)
- 환경변수 `DB_HOST`, `DB_USERNAME`, `DB_PASSWORD`, `DB_NAME`으로 설정 오버라이드
- 설정값 없을 경우 기본값(운영서버 IP) 사용

### 4.2 lib/db.php — PDO 연결

- 타임존 `Asia/Jakarta` 설정
- PDO 예외 모드(`ERRMODE_EXCEPTION`) 활성화
- 연결 실패 시 `logs/db_errors.log` 기록 후 JSON 오류 응답

### 4.3 lib/api_helper.lib.php — API 공통 헬퍼

OEE 관련 모든 API에서 사용하는 통합 클래스:

| 메서드 | 기능 |
| ------ | ---- |
| `validateAndProcessMac()` | MAC 주소 검증 및 정제 |
| `getMachineInfo()` | MAC으로 기계 정보 조회 (공장/라인 JOIN) |
| `getCurrentShiftInfo()` | 현재 교대조 조회 |
| `calculateShiftMetrics()` | 근무시간 지표 계산 |
| `calculateOeeMetrics()` | OEE 계산 (가용성, 성능, 품질) |
| `saveOeeData()` | OEE 데이터 INSERT/UPDATE |
| `logApiCall()` | API 호출 로그 저장 (화이트리스트 검증 포함) |
| `getStatusList()` | 안돈/비가동/불량 목록 조회 |
| `insertWarningData()` | Warning 데이터 삽입 |
| `updateCompletedStatus()` | 완료 처리 |

### 4.4 OEE 계산 공식

```
가용성(A) = (가동시간 - 비가동시간) / 가동시간 x 100
성능(P)   = 실제 생산량 / 이론적 생산량 x 100
품질(Q)   = 양품 수량 / 실제 생산량 x 100
OEE       = A x P x Q / 10000
```

### 4.5 assets/js/resource-manager.js — 범용 CRUD 모듈

관리 페이지 공통 CRUD 처리 모듈:
- 테이블 렌더링, 페이지네이션, 검색, 정렬
- 모달 폼 관리, 유효성 검사
- 각 페이지별 `config` 객체로 설정 주입

---

## 5. API 구조

기본 URL: `http://{서버}/2025/sci/new/api/sewing.php?code={엔드포인트}`

| 엔드포인트 | 파일 | 설명 |
| ---------- | ---- | ---- |
| `start` | start.php | 장비 전원 ON 등록/갱신 |
| `get_andonList` | get_andonList.php | 안돈 유형 목록 조회 |
| `get_downtimeList` | get_downtimeList.php | 비가동 유형 목록 조회 |
| `get_defectiveList` | get_defectiveList.php | 불량 유형 목록 조회 |
| `get_dateTime` | get_dateTime.php | 서버 날짜/시간 + 교대조 정보 |
| `send_pCount` | send_pCount.php | 생산량 전송 + OEE 자동 계산 저장 |
| `send_andon_warning` | send_andon_warning.php | 안돈 발생 |
| `send_andon_completed` | send_andon_completed.php | 안돈 완료 |
| `send_downtime_warning` | send_downtime_warning.php | 비가동 발생 |
| `send_downtime_completed` | send_downtime_completed.php | 비가동 완료 |
| `send_defective_warning` | send_defective_warning.php | 불량 발생 |

공통 응답 형식:
```json
{"code": "00", "msg": "success"}
{"code": "99", "msg": "오류 메시지"}
```

---

## 6. 데이터베이스 구조

데이터베이스명: `sci_2025_new`

### 마스터 테이블

| 테이블명 | 설명 |
| -------- | ---- |
| `info_factory` | 공장 정보 |
| `info_line` | 라인 정보 |
| `info_machine` | 기계 정보 (MAC, IP, 모델, 목표 생산량) |
| `info_machine_model` | 기계 모델 |
| `info_andon` | 안돈 유형 |
| `info_downtime` | 비가동 유형 |
| `info_defective` | 불량 유형 |
| `info_design_process` | 디자인 공정 |
| `info_rate_color` | OEE 색상 기준 |
| `info_work_time` | 근무시간 마스터 |
| `info_work_time_shift` | 교대조 시간 설정 |

### 데이터 테이블

| 테이블명 | 설명 |
| -------- | ---- |
| `data_oee` | OEE 집계 데이터 (교대조별) |
| `data_oee_rows` | OEE 행별 추적 데이터 |
| `data_andon` | 안돈 발생 이력 |
| `data_downtime` | 비가동 이력 (duration_sec 포함) |
| `data_defective` | 불량 이력 |

### API 로그 테이블

| 테이블명 | 설명 |
| -------- | ---- |
| `logs_api_start` | 장비 등록 로그 |
| `logs_api_send_pCount` | 생산량 전송 로그 |
| `logs_api_*` | 각 API 호출 로그 |

---

## 7. 로컬 개발 환경 설정

### 사전 준비

1. **Laragon** 실행 (PHP 7.4.33, MySQL)
2. 프로젝트 폴더를 Laragon www 경로에 연결 (심볼릭 링크 또는 복사)
3. Laragon MySQL에 `sci_2025_new` 데이터베이스 생성 후 스키마/데이터 임포트

### .env 파일 설정 (OEE_SCI_V1 루트에 생성)

```
DB_HOST=localhost
DB_USERNAME=root
DB_PASSWORD=
DB_NAME=sci_2025_new
```

### 로컬 접속 URL

```
http://localhost/dev/OEE_SCI/OEE_SCI_V1/
```

또는 Laragon 가상호스트 설정 시:
```
http://oee-sci.test/
```

### VSCode 디버깅 설정

1. `F5` 키 -> `Listen for Xdebug (Local - Laragon)` 선택
2. 코드 좌측 클릭으로 브레이크포인트 설정
3. 브라우저에서 페이지 접근 시 디버깅 시작

---

## 8. 발견된 이슈 및 개선 이력

| 날짜 | 구분 | 내용 |
| ---- | ---- | ---- |
| 2026-03-06 | 이슈 | `worktime.lib.php`의 `getDayShift()`에서 SQL 문자열 직접 삽입 (factory_id) — prepared statement 전환 권장 |
| 2026-03-06 | 이슈 | `lib/config.php` 기본값에 운영 DB 인증정보 포함 — .env 파일 필수 사용 권장 |
| 2026-03-06 | 개선 | `api_helper.lib.php`로 API 공통 로직 통합 |
| 2026-03-06 | 개선 | `resource-manager.js`로 관리 페이지 CRUD 공통화 |
| 2026-03-06 | 개선 | SSE 스트리밍 도입으로 실시간 데이터 표시 |
| 2026-03-06 | 개선 | `index.php` 하드코딩 경로 → `$_SERVER['SCRIPT_NAME']` 기반 동적 경로로 수정 |
| 2026-03-06 | 개선 | `.env` 파일 생성으로 로컬 DB 연결 지원 |
| 2026-03-06 | 개선 | `C:\laragon\www\dev` junction을 `C:\SUNTECH_DEV_CLAUDECODE\WEB`으로 재연결 — 모든 WEB 프로젝트 로컬 접근 가능 |
| 2026-03-06 | 개선 | `inc/nav-fiori.php` 하드코딩 경로 `/2025/sci/new/` → `preg_match` 기반 동적 `$project_root` 계산으로 수정 (설치 위치 무관) |
| 2026-03-06 | 개선 | `inc/head.php` PWA manifest 링크 제거, `manifest.json` 삭제 (PWA 미사용 확정) |

---

## 9. 테스트 시나리오

### API 테스트

```
# 장비 등록 테스트
.../api/sewing.php?code=start&machine_no=TEST01&mac=84:72:07:50:37:73&ip=192.168.0.1&ver=Test_1.0

# 생산량 전송 테스트
.../api/sewing.php?code=send_pCount&mac=84:72:07:50:37:73&pCount=100
```

### 브라우저 테스트

1. 공장 관리 페이지 접속 -> 공장 추가/수정/삭제 확인
2. 라인, 기계 관리 동일 패턴 확인
3. OEE 모니터링 페이지에서 SSE 연결 확인
4. 필터(공장/라인/기계/날짜/교대조) 동작 확인
5. Excel 내보내기 다운로드 확인

---

## 10. 버전 이력 테이블

| 날짜 | 버전 | 주요 변경 내용 |
| ---- | ---- | -------------- |
| 2026-03-06 | OEE_SCI_V1 | 최초 분석 문서 작성, 기본 기능 구현 완료 |
| 2026-03-06 | OEE_SCI_V1 | nav-fiori.php 동적 경로 수정, PWA manifest 제거 |
| 2026-03-06 | OEE_SCI_V2 | V1 기반 V2 생성, AI 통계 엔진 + AI Dashboard 구현 (F5~F13) |
| 2026-03-07 | OEE_SCI_V2 | SSE stream 성능 최적화, 공통 라이브러리 통합, 타임존 중앙화 (Phase 2~4) |
| 2026-03-08 | OEE_SCI_V2 | 1920×1080 사이니지 전용 dashboard_2.php + ai_dashboard_2.php 신규 생성 |
| 2026-03-08 | OEE_SCI_V2 | F11 AI 리포트 엔진(ai_report_engine.php) + HTML Export(ai_report_export.php) 구현 |
| 2026-03-08 | OEE_SCI_V2 | ai_dashboard_2.php Export 버튼 + 날짜 선택 모달 추가, linearRegression 버그 수정 |
| 2026-03-09 | OEE_SCI_V2 | ai_dashboard_3.php + ai_dashboard_3.css 신규 생성 — 사이니지 AI 대시보드 V3 (Grid 비율 개선, Row D 잘림 해소) |
| 2026-03-09 | OEE_SCI_V2 | dashboard_2.php Row A 레이아웃 개선 (OEE 4지표 전체 너비, Andon→Row B 이동) |
| 2026-03-09 | OEE_SCI_V2 | dashboard_2.php + ai_dashboard_3.php 햄버거 슬라이드 드로어 메뉴 추가 (전체 네비게이션, active 하이라이트) |
| 2026-03-19 | OEE_SCI_V2 | **AI Dashboard v4** 신규 생성 — 5카드 Row A (Real-time OEE LIVE 카드 추가), 날짜 필터 연동, Playwright 버그 분석 5건 발견 |
| 2026-03-19 | OEE_SCI_V2 | **AI Dashboard v5** 신규 생성 — v4 버그 5건 전체 수정 (_5 접미사 파일 분리, current_oee 클램핑, Actual OEE 차트 라인 추가, date_range 전 API 연동) |

---

## 11. 관련 파일 경로 빠른 참조

| 파일 | 경로 |
| ---- | ---- |
| DB 설정 | `OEE_SCI_V1\lib\config.php` |
| PDO 연결 | `OEE_SCI_V1\lib\db.php` |
| API 헬퍼 | `OEE_SCI_V1\lib\api_helper.lib.php` |
| 공통 JS 모듈 | `OEE_SCI_V1\assets\js\resource-manager.js` |
| OEE 모니터링 | `OEE_SCI_V1\page\data\data_oee.php` |
| OEE 스트리밍 API | `OEE_SCI_V1\page\data\proc\data_oee_stream.php` |
| 공장 관리 | `OEE_SCI_V1\page\manage\info_factory.php` |
| 생산량 수신 API | `OEE_SCI_V1\api\sewing\send_pCount.php` |
| 장비 등록 API | `OEE_SCI_V1\api\sewing\start.php` |
| 근무시간 클래스 | `OEE_SCI_V1\lib\worktime.lib.php` |
| 로컬 환경 가이드 | `C:\SUNTECH_DEV_CLAUDECODE\WEB\local-dev-environment.md` |
| AI Dashboard | `OEE_SCI_V2\page\data\ai_dashboard.php` |
| AI Dashboard (사이니지 V2) | `OEE_SCI_V2\page\data\ai_dashboard_2.php` |
| AI Dashboard (사이니지 V3) | `OEE_SCI_V2\page\data\ai_dashboard_3.php` |
| AI Dashboard (사이니지 V4) | `OEE_SCI_V2\page\data\ai_dashboard_4.php` |
| AI Dashboard (사이니지 V5 — 권장) | `OEE_SCI_V2\page\data\ai_dashboard_5.php` |
| 통합 대시보드 (사이니지) | `OEE_SCI_V2\page\data\dashboard_2.php` |
| AI 통계 엔진 | `OEE_SCI_V2\lib\statistics.lib.php` |
| SSE 공통 헬퍼 | `OEE_SCI_V2\lib\stream_helper.lib.php` |
