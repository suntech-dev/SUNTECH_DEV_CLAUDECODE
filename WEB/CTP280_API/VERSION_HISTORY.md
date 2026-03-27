# CTP280_API 버전 이력

> 최초 작성: 2026-03-27
> 마지막 업데이트: 2026-03-27

---

## CTP280_API (현재 운영 버전)

| 항목 | 내용 |
| ---- | ---- |
| 버전 식별자 | CTP280_API |
| 프로젝트 폴더 | `WEB\CTP280_API` |
| 작업일 | 2026-03-10 (초기 구축) |
| 상태 | 운영 중 |
| 기반 | OEE_SCI API 구조 기반 신규 구축 |

### 기술 스택

| 구분 | 기술 |
| ---- | ---- |
| 서버 언어 | PHP 7.4+ |
| 데이터베이스 | MySQL 5.7+ |
| DB 접근 | PDO (prepared statement) |
| API 방식 | REST (GET/POST, JSON 응답) |
| 타임존 | Asia/Jakarta |
| 대상 장비 | CTP280 패턴재봉기, 자수기 (EMBROIDERY_S) |

### 주요 기능

- IoT 장비 등록 및 갱신 (`start`)
- 재봉기 생산수량 수신 (`send_pCount`)
- 자수기 생산 데이터 수신 (`send_eCount`)
- 안돈 경보 발생/완료 처리 (`send_andon_warning` / `send_andon_completed`)
- 비가동 경보 발생/완료 처리 (`send_downtime_warning` / `send_downtime_completed`)
- 불량 경보 발생 (`send_defective_warning`)
- 안돈/비가동/불량 목록 조회 (`get_andonList`, `get_downtimeList`, `get_defectiveList`)
- 서버 시간 조회 (`get_dateTime`)
- 자수기 데이터 로그 뷰어 (`log_embroidery.php`)

### 변경 이력

| 날짜 | 내용 |
| ---- | ---- |
| 2026-03-10 | CTP280_API 초기 구조 생성 (api/sewing.php 라우터, lib/ 공통 라이브러리) |
| 2026-03-10 | `start`, `send_pCount`, 안돈/비가동/불량 관련 API 엔드포인트 구현 |
| 2026-03-10 | `api/api_test/` HTML 테스트 페이지 구성 |
| 2026-03-26 | `api/sewing/send_eCount.php` 자수기 전용 API 추가 (`data_embroidery` 테이블) |
| 2026-03-27 | `log_embroidery.php` 자수기 데이터 로그 뷰어 신규 생성 |
| 2026-03-27 | `log_embroidery.php` — `info_machine` JOIN으로 `machine_no` 필터 추가 |
| 2026-03-27 | `CLAUDE.md`, `README.md`, `VERSION_HISTORY.md` 문서화 |
