# CTP280_API 프로젝트 분석 문서

> 최초 작성: 2026-03-27
> 분석 버전: CTP280_API
> 마지막 업데이트: 2026-03-27

---

## 1. 프로젝트 개요

**CTP280_API**는 인도네시아 봉제 공장의 IoT 장비(CTP280 패턴재봉기, 자수기)와 서버 간 통신을 담당하는 **REST API 백엔드**입니다.
장비가 WiFi 연결 후 MAC 주소 기반으로 자동 등록되고, 생산 데이터·이벤트 데이터를 수신하여 DB에 저장합니다.

### 주요 기능 요약

| 기능 | 설명 |
| ---- | ---- |
| 장비 등록/갱신 | MAC 주소 기반 자동 등록, IP/버전 갱신 |
| 생산수량 수신 | 재봉기(`send_pCount`) / 자수기(`send_eCount`) 생산 데이터 저장 |
| 안돈 관리 | 경보 발생 및 완료 처리 |
| 비가동 관리 | 비가동 사유 발생 및 완료 처리 |
| 불량 관리 | 불량 데이터 수신 및 저장 |
| 목록 조회 | 안돈/비가동/불량 현황 조회 |
| 서버 시간 제공 | 장비 시계 동기화용 |
| 로그 뷰어 | 자수기 데이터 실시간 조회 (`log_embroidery.php`) |

---

## 2. 폴더 구조

```
CTP280_API/
├── CLAUDE.md                       # 프로젝트 코딩 규칙
├── README.md                       # 이 문서
├── VERSION_HISTORY.md              # 버전 이력
├── index.html                      # 진입점 (API 목록 안내)
├── log_embroidery.php              # 자수기 데이터 로그 뷰어
├── api/
│   ├── sewing.php                  # API 라우터 (화이트리스트 방식)
│   ├── get_api_list.php            # API 목록 조회
│   ├── api_test/                   # API 테스트용 HTML 페이지
│   │   ├── start.html
│   │   ├── send_pCount.html
│   │   ├── send_andon_warning.html
│   │   ├── send_andon_completed.html
│   │   ├── send_downtime_warning.html
│   │   ├── send_downtime_completed.html
│   │   ├── send_defective_warning.html
│   │   ├── get_andonList.html
│   │   ├── get_downtimeList.html
│   │   ├── get_defectiveList.html
│   │   └── get_dateTime.html
│   └── sewing/                     # API 개별 엔드포인트
│       ├── start.php               # 장비 등록/갱신
│       ├── send_pCount.php         # 재봉기 생산수량 수신
│       ├── send_eCount.php         # 자수기 생산 데이터 수신
│       ├── send_andon_warning.php  # 안돈 경보 발생
│       ├── send_andon_completed.php # 안돈 완료
│       ├── send_downtime_warning.php # 비가동 발생
│       ├── send_downtime_completed.php # 비가동 완료
│       ├── send_defective_warning.php  # 불량 발생
│       ├── get_andonList.php       # 안돈 목록 조회
│       ├── get_downtimeList.php    # 비가동 목록 조회
│       ├── get_defectiveList.php   # 불량 목록 조회
│       └── get_dateTime.php        # 서버 시간 조회
├── lib/
│   ├── config.php                  # DB 접속 설정 (환경변수 우선)
│   ├── db.php                      # PDO 연결 + 타임존(Asia/Jakarta)
│   ├── api_helper.lib.php          # ApiHelper 클래스 (MAC 검증, 로그 등)
│   ├── database_helper.lib.php     # DB 공통 헬퍼
│   ├── validator.lib.php           # 입력값 검증
│   ├── get_shift.lib.php           # 근무 교대 계산
│   ├── statistics.lib.php          # 통계 연산
│   ├── stream_helper.lib.php       # SSE 공통 헬퍼
│   ├── worktime.lib.php            # 근무시간 로직
│   └── worktime_common.php         # 근무시간 공통 함수
└── assets/
    ├── css/test_api.css
    └── js/api-tester.js
```

---

## 3. 기술 스택

| 구분 | 기술 |
| ---- | ---- |
| 서버 언어 | PHP 7.4+ |
| 데이터베이스 | MySQL 5.7+ |
| DB 접근 | PDO (prepared statement 필수) |
| API 방식 | REST (GET/POST, JSON 응답) |
| 보안 | 화이트리스트 코드 검증, 보안 헤더 |
| 타임존 | Asia/Jakarta |
| 빌드 시스템 | 없음 (순수 PHP) |

---

## 4. 소스코드 상세 분석

### `api/sewing.php` — API 라우터

모든 장비 API 요청의 진입점. `?code=` 파라미터로 서브파일 분기.

```
요청 → code 파라미터 검증 → 화이트리스트 확인 → sewing/{code}.php include → $response 반환
```

- 허용 코드가 아닐 경우 403 + 보안 로그 기록
- `$pdo`, `jsonReturn()` 함수를 서브파일에서 그대로 사용

### `lib/db.php` — DB 연결

- PDO 연결 생성 + `date_default_timezone_set('Asia/Jakarta')` 선언
- 연결 실패 시 JSON 에러 응답 후 종료
- 에러 로그: `logs/db_errors.log`

### `lib/api_helper.lib.php` — ApiHelper 클래스

- `validateAndProcessMac($mac)` — MAC 주소 형식 검증 및 정규화
- `createResponse($code, $msg)` — 표준 응답 배열 생성
- `logApiCall($table, $code, $machine_no, $mac, $request, $response, $datetime)` — API 호출 로그 저장

### `api/sewing/start.php` — 장비 등록

- 파라미터: `machine_no`, `mac`, `ip`, `ver`
- `info_machine` 테이블에서 MAC으로 조회
  - 기존 장비: `ip`, `app_ver`, `update_date` 업데이트
  - 신규 장비: `factory_idx=99`, `line_idx=99` 로 신규 INSERT
- 응답: `machine_no`, `target`, `req_interval`

### `api/sewing/send_eCount.php` — 자수기 데이터 수신

- 파라미터: `mac`, `actual_qty`, `ct`, `tb`, `mrt`
- `data_embroidery` 테이블에 INSERT
- 범위 검증: `actual_qty(0~10000)`, `tb(0~1000)`, `ct/mrt(0~3600)`

---

## 5. API 구조

### 엔드포인트

```
POST/GET http://{server}/dev/CTP280_API/api/sewing.php?code={code}
```

### API 코드 목록

| code | 방향 | 설명 | 주요 파라미터 |
| ---- | ---- | ---- | ------------- |
| `start` | 장비→서버 | 장비 등록/갱신 | `machine_no`, `mac`, `ip`, `ver` |
| `send_pCount` | 장비→서버 | 재봉기 생산수량 | `mac`, `pCount`, ... |
| `send_eCount` | 장비→서버 | 자수기 생산 데이터 | `mac`, `actual_qty`, `ct`, `tb`, `mrt` |
| `send_andon_warning` | 장비→서버 | 안돈 경보 발생 | `mac`, `andon_code` |
| `send_andon_completed` | 장비→서버 | 안돈 완료 | `mac`, `andon_idx` |
| `send_downtime_warning` | 장비→서버 | 비가동 발생 | `mac`, `downtime_code` |
| `send_downtime_completed` | 장비→서버 | 비가동 완료 | `mac`, `downtime_idx` |
| `send_defective_warning` | 장비→서버 | 불량 발생 | `mac`, `defective_code` |
| `get_andonList` | 서버→장비 | 안돈 목록 조회 | `mac` |
| `get_downtimeList` | 서버→장비 | 비가동 목록 조회 | `mac` |
| `get_defectiveList` | 서버→장비 | 불량 목록 조회 | `mac` |
| `get_dateTime` | 서버→장비 | 서버 시간 조회 | (없음) |

### 응답 형식

```json
// 성공
{ "code": "00", "msg": "OK", ... }

// 실패
{ "code": "99", "msg": "에러 메시지" }
```

---

## 6. 데이터베이스 구조

- **DB명**: `ctp280_api_test`

### `info_machine` — 장비 마스터

| 컬럼 | 타입 | 설명 |
| ---- | ---- | ---- |
| `idx` | INT AUTO_INCREMENT | PK |
| `factory_idx` | INT | 공장 인덱스 |
| `line_idx` | INT | 라인 인덱스 |
| `machine_no` | VARCHAR | 기계 번호 |
| `target` | VARCHAR | 목표 수량 |
| `app_ver` | VARCHAR | 장비 앱 버전 |
| `ip` | VARCHAR | 장비 IP |
| `mac` | VARCHAR | MAC 주소 (장비 식별자) |
| `reg_date` | DATETIME | 최초 등록일 |
| `update_date` | DATETIME | 마지막 갱신일 |

### `data_embroidery` — 자수기 생산 데이터

| 컬럼 | 타입 | 설명 |
| ---- | ---- | ---- |
| `idx` | INT AUTO_INCREMENT | PK |
| `mac` | VARCHAR | MAC 주소 |
| `actual_qty` | INT | 실제 생산수량 |
| `ct` | FLOAT | Cycle Time (초) |
| `tb` | INT | Thread Break 횟수 |
| `mrt` | FLOAT | Motor Runtime (초) |
| `created_at` | DATETIME | 수신 시각 |

### `logs_api_start` — start API 호출 로그

장비 등록/갱신 API 호출 이력 저장.

---

## 7. 로컬 개발 환경 설정

```
로컬 URL: http://localhost/dev/CTP280_API/api/sewing.php?code=start&...
Laragon 경로: C:\laragon\www\dev\CTP280_API (junction → C:\SUNTECH_DEV_CLAUDECODE\WEB\CTP280_API)
PHP: 7.4.33
MySQL: Laragon 내장 (root / 비밀번호 없음)
```

**.env 파일 예시** (프로젝트 루트에 생성):
```
DB_HOST=localhost
DB_USERNAME=root
DB_PASSWORD=
DB_NAME=ctp280_api_test
```

---

## 8. 발견된 이슈 및 개선 이력

| 날짜 | 분류 | 내용 |
| ---- | ---- | ---- |
| 2026-03-27 | 기능 추가 | `log_embroidery.php` `info_machine` JOIN — `machine_no` 필터 추가 |

---

## 9. 테스트 시나리오

### API 테스트 (브라우저)

`api/api_test/` 폴더의 HTML 파일을 브라우저에서 열어 각 API 테스트 가능.

### cURL 테스트 예시

```bash
# 장비 등록
curl "http://localhost/dev/CTP280_API/api/sewing.php?code=start&machine_no=TEST01&mac=84:72:07:50:37:73&ip=192.168.0.26&ver=1.0"

# 자수기 데이터 전송
curl "http://localhost/dev/CTP280_API/api/sewing.php?code=send_eCount&mac=84:72:07:50:37:73&actual_qty=10&ct=120.5&tb=0&mrt=100.3"
```

---

## 10. 버전 이력 테이블

| 날짜 | 내용 |
| ---- | ---- |
| 2026-03-10 | CTP280_API 초기 구축 |
| 2026-03-26 | 자수기 전용 `send_eCount` API 추가 |
| 2026-03-27 | `log_embroidery.php` 신규 생성 및 `machine_no` 필터 추가 |
| 2026-03-27 | 프로젝트 문서화 (`CLAUDE.md`, `README.md`, `VERSION_HISTORY.md`) |

---

## 11. 관련 파일 경로 빠른 참조

| 파일 | 경로 |
| ---- | ---- |
| API 라우터 | `CTP280_API/api/sewing.php` |
| 자수기 데이터 수신 | `CTP280_API/api/sewing/send_eCount.php` |
| 재봉기 데이터 수신 | `CTP280_API/api/sewing/send_pCount.php` |
| 장비 등록 | `CTP280_API/api/sewing/start.php` |
| DB 연결 | `CTP280_API/lib/db.php` |
| DB 설정 | `CTP280_API/lib/config.php` |
| API 헬퍼 | `CTP280_API/lib/api_helper.lib.php` |
| 자수기 로그 뷰어 | `CTP280_API/log_embroidery.php` |
