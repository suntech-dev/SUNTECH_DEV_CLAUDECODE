# OEE_SCI 버전 이력

> 최초 작성: 2026-03-06
> 마지막 업데이트: 2026-03-06 (수정: 2026-03-06 — info_worktime.php 버그 수정)

---

## OEE_SCI_V1

| 항목 | 내용 |
| ---- | ---- |
| 버전 식별자 | OEE_SCI_V1 |
| 프로젝트 폴더 | `WEB\OEE_SCI\OEE_SCI_V1` |
| 작업일 | 2026-03-06 |
| 상태 | 운영 배포 권장 |
| 타임존 | Asia/Jakarta (UTC+7) |
| 데이터베이스 | sci_2025_new |

### 기술 스택

| 구분 | 기술 |
| ---- | ---- |
| 서버 언어 | PHP 7.4+ |
| 데이터베이스 | MySQL 5.7+ |
| UI 프레임워크 | SAP Fiori Horizon Light |
| 차트 | Chart.js |
| 엑셀 내보내기 | PhpSpreadsheet |
| 실시간 통신 | Server-Sent Events (SSE) |
| jQuery | 3.6.1 |
| moment.js + daterangepicker | 번들 내장 |

### 주요 기능

- OEE 실시간 모니터링 (가용성, 성능, 품질)
- IoT 재봉기 API: MAC 주소 기반 장비 자동 등록 및 생산량 수신
- SSE 기반 실시간 데이터 스트리밍
- 안돈 / 비가동 / 불량 관리 (발생/완료 처리)
- 마스터 데이터 관리: 공장, 라인, 기계, 모델, 근무시간
- 시간별/교대조별 OEE 로그
- Excel 엑셀 내보내기
- OEE 보고서 생성

### 변경 이력

| 날짜 | 내용 |
| ---- | ---- |
| 2026-03-06 | 프로젝트 최초 분석 문서 작성 |
| 2026-03-06 | `api_helper.lib.php`로 API 공통 로직 통합 완료 |
| 2026-03-06 | `resource-manager.js`로 관리 페이지 CRUD 공통화 완료 |
| 2026-03-06 | SSE 실시간 스트리밍 구현 완료 |
| 2026-03-06 | PhpSpreadsheet 기반 엑셀 내보내기 구현 완료 |
| 2026-03-06 | `index.php` 하드코딩 경로 → `$_SERVER['SCRIPT_NAME']` 기반 동적 경로로 수정 |
| 2026-03-06 | `.env` 파일 생성 — 로컬 DB 연결 지원 (DB_HOST=localhost) |
| 2026-03-06 | `C:\laragon\www\dev` junction 재연결 → `C:\SUNTECH_DEV_CLAUDECODE\WEB` |
| 2026-03-06 | `inc/nav-fiori.php` 하드코딩 경로(`/2025/sci/new/`) → `preg_match` 기반 동적 `$project_root` 계산으로 수정 |
| 2026-03-06 | `inc/head.php` PWA manifest 링크 제거 |
| 2026-03-06 | `manifest.json` 삭제 (PWA 미사용 확정) |
| 2026-03-06 | `lib/worktime_database.php` — 하드코딩 DB 인증 제거, `config.php` require로 교체 |
| 2026-03-06 | `page/manage/proc/set_work_time_fetch_month.php` — PHP Notice 수정: 달력 빈 셀(32~35일) `Undefined offset` → `isset()` + `??` null coalescing 적용 |
| 2026-03-06 | `page/manage/info_worktime.php` — include 순서 수정 (nav-fiori.php가 body 열리기 전에 출력되던 버그), `window.prevMonth/nextMonth/thisMonth` 전역 노출 |
| 2026-03-06 | `page/manage/proc/set_work_time_insert.php` — 하드코딩 경로 `/samho/lib/worktime.lib.php` → `__DIR__ . '/../../../lib/worktime.lib.php'` 수정, 달력 날짜 클릭 모달 정상 동작 |
