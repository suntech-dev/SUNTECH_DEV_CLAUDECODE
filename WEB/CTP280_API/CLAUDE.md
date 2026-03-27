# CLAUDE.md — CTP280_API 프로젝트 규칙

> 이 파일은 `C:\SUNTECH_DEV_CLAUDECODE\WEB\CTP280_API` 프로젝트에 적용되는 규칙입니다.
> 상위 규칙: `C:\SUNTECH_DEV_CLAUDECODE\WEB\CLAUDE.md` (공통 규칙 상속)

---

## PHP 코드 스타일 가이드

### 인코딩
- **인코딩**: UTF-8 (BOM 없음)

### 보안
- 모든 쿼리는 PDO prepared statement 필수
- API 코드는 화이트리스트(`in_array`) 방식으로 검증 — `api/sewing.php` 참조
- 동적 파라미터는 반드시 `trim()` + 형변환(int/float/string) 후 사용
- MAC 주소 검증은 `ApiHelper::validateAndProcessMac()` 공통 함수 사용

### API 응답 형식

```php
// 성공
jsonReturn(['code' => '00', 'msg' => 'OK', ...]);

// 실패
jsonReturn(['code' => '99', 'msg' => 'error message'], 400);
```

### 타임존
- **`date_default_timezone_set('Asia/Jakarta')`는 `lib/db.php` 단 한 곳에만 선언**
- 개별 API 파일에 중복 선언 금지

### 파일 경로 규칙
- 절대경로: `__DIR__ . '/../../lib/db.php'` 형식 사용
- API 서브파일은 `$pdo`, `jsonReturn()` 을 sewing.php 에서 상속받아 사용

---

## API 구조 규칙

### 라우터 패턴 (`api/sewing.php`)
- `?code=` 파라미터로 서브파일 분기
- 허용 코드는 `$allowedCodes` 배열에 명시 (화이트리스트)
- 서브파일: `api/sewing/{code}.php`
- 서브파일은 반드시 `$response` 배열 설정 후 종료

### 신규 API 추가 절차
1. `api/sewing/{code}.php` 파일 생성
2. `api/sewing.php` 의 `$allowedCodes` 배열에 코드 추가
3. `api/api_test/{code}.html` 테스트 파일 생성
4. `api/get_api_list.php` 에 API 설명 추가 (해당 시)

---

## 공통 라이브러리 (`lib/`)

| 파일 | 역할 |
| ---- | ---- |
| `config.php` | DB 접속 정보 (환경변수 우선) |
| `db.php` | PDO 연결 + 타임존 설정 |
| `api_helper.lib.php` | `ApiHelper` 클래스 — MAC 검증, 응답 생성, 로그 기록 |
| `database_helper.lib.php` | DB 공통 헬퍼 |
| `validator.lib.php` | 입력값 검증 유틸리티 |
| `get_shift.lib.php` | 근무 교대 계산 |
| `statistics.lib.php` | 통계 연산 |
| `stream_helper.lib.php` | SSE 공통 헬퍼 |
| `worktime.lib.php` | 근무시간 관련 로직 |
| `worktime_common.php` | 근무시간 공통 함수 |

---

## 데이터베이스 테이블 규칙

- DB명: `ctp280_api_test`
- 주요 테이블: `info_machine`, `data_embroidery`, `logs_api_start`
- `info_machine` 테이블: `mac` 컬럼으로 장비 식별 (PK 역할)
- `data_embroidery` 테이블: 자수기 생산 데이터 (`mac`, `actual_qty`, `ct`, `tb`, `mrt`, `created_at`)

---

## 버전 히스토리 관리 (`VERSION_HISTORY.md`)

- 버전 히스토리 파일: `VERSION_HISTORY.md` (프로젝트 루트 `CTP280_API/`)
- 신규 기능 추가 또는 API 변경 시 `VERSION_HISTORY.md` 에 날짜·내용 추가

---

## README.md 업데이트 규칙

- API 추가/변경 시 README.md 섹션 5 (API 구조) 동기화
- DB 테이블 변경 시 README.md 섹션 6 (데이터베이스 구조) 동기화

---

## `.md` -> `.html` 변환 스타일 가이드

상위 `WEB/CLAUDE.md` 의 변환 규칙을 그대로 따른다.
(다크 테마, 좌측 사이드바 TOC, CSS 변수 고정)
