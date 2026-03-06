# OEE_SCI 버전 이력

> 최초 작성: 2026-03-06
> 마지막 업데이트: 2026-03-06 (F5~F7 완료: 품질파수꾼·OEE오버레이·다운타임위험도 / F12~F13 완료: 스트리밍·최적화)

---

## OEE_SCI_V2

| 항목 | 내용 |
| ---- | ---- |
| 버전 식별자 | OEE_SCI_V2 |
| 프로젝트 폴더 | `WEB\OEE_SCI\OEE_SCI_V2` |
| 작업일 | 2026-03-06 |
| 상태 | 개발 중 |
| 기반 | OEE_SCI_V1 전체 복사 후 AI 기능 추가 |

### 기술 스택

| 구분 | 기술 |
| ---- | ---- |
| 서버 언어 | PHP 7.4+ |
| 데이터베이스 | MySQL 5.7+ |
| UI 프레임워크 | SAP Fiori Horizon Light |
| 차트 | Chart.js |
| 엑셀 내보내기 | PhpSpreadsheet |
| 실시간 통신 | Server-Sent Events (SSE) |
| AI 엔진 | Claude API (claude-sonnet-4-6) |
| AI 통계 | PHP 자체 구현 (지수평활법, Z-Score, 선형회귀) |

### 주요 기능 (V1 대비 추가)

- AI Dashboard 신규 페이지 (`ai_dashboard.php`)
- OEE 시간대별 예측 (Predictive OEE)
- 예방정비 위험도 스코어 (Predictive Maintenance)
- 실시간 이상 감지 (Anomaly Detection, Z-Score 기반)
- AI 자연어 어시스턴트 (Claude API 연동)
- AI 품질 파수꾼 (Quality Sentinel)
- AI 리포트 생성기

### 변경 이력

| 날짜 | 내용 |
| ---- | ---- |
| 2026-03-06 | OEE_SCI_V1 기반으로 V2 폴더 생성 (전체 복사) |
| 2026-03-06 | AI 도입 전략 설계서 작성 (`AI_STRATEGY_V2.md`) |
| 2026-03-06 | **[Phase 1 구현 완료]** AI 통계 엔진 `lib/statistics.lib.php` 신규 생성 (지수평활법, 이동평균, Z-Score, 선형회귀, 신뢰구간) |
| 2026-03-06 | `page/data/proc/ai_oee_prediction.php` — OEE 예측 API: 과거 30일 계절성(요일×시간대) + 지수평활법 혼합, 90% 신뢰구간 반환 |
| 2026-03-06 | `page/data/proc/ai_anomaly.php` — 이상 감지 API: Z-Score 기반 머신별 OEE/Availability/Quality 이상 감지, 연쇄 이상(동일 라인 복수 머신) 감지 포함 |
| 2026-03-06 | `page/data/proc/ai_maintenance.php` — 예방정비 위험도 API: 런타임(40%) + 다운타임 빈도 증가율(35%) + OEE 불안정도(25%) 가중 합산 스코어 |
| 2026-03-06 | `page/data/ai_dashboard.php` — AI Intelligence Dashboard 신규 페이지 |
| 2026-03-06 | `page/data/css/ai_dashboard.css` — AI Dashboard 전용 스타일 (SAP Fiori 기반, 요약카드/예측차트/이상감지/건강지수/예방정비 컴포넌트) |
| 2026-03-06 | `page/data/js/ai_dashboard.js` — AI Dashboard 클라이언트 로직 (Chart.js 예측선+신뢰구간, 60초 자동갱신, Factory→Line→Machine 연계 필터) |
| 2026-03-06 | `inc/nav-fiori.php` — 네비게이션에 "AI Dashboard" 메뉴 항목 추가 |
| 2026-03-06 | **[F5 구현 완료]** `page/data/proc/ai_quality_sentinel.php` — AI 품질 파수꾼 API: 파레토 분석(80% 룰), 24시간 히트맵, 머신 위험 랭킹(최근 7일 vs 이전 7일 증가율), Pearson OEE 상관계수 |
| 2026-03-06 | `page/data/js/ai_quality_sentinel.js` — 품질 파수꾼 프론트엔드: Chart.js 파레토 콤보 차트(막대+누적선), CSS 히트맵 그리드(6열×4행), 머신 랭킹 카드, OEE 상관계수 패널 |
| 2026-03-06 | `page/data/data_defective.php` — AI 품질 파수꾼 섹션 추가 (로딩 스피너 → 요약카드 4개 → 파레토+히트맵 → 랭킹+상관계수) |
| 2026-03-06 | `page/data/css/ai_dashboard.css` — 히트맵 셀(.ai-heatmap-grid/cell/label/count) 및 머신 랭킹 카드 스타일 추가 |
| 2026-03-06 | **[F6 구현 완료]** `page/data/js/ai_oee_overlay.js` — OEE 트렌드 차트에 AI 예측선 오버레이: `window.updateOeeTrendChart` 몽키패치, CI Upper/Lower + AI Forecast 점선 데이터셋 주입, `#aiOeeTrendBadge` 트렌드 방향 뱃지 |
| 2026-03-06 | `page/data/data_oee.php` — OEE 트렌드 카드 헤더에 AI POWERED 뱃지 및 "점선 = AI 예측" 서브타이틀 추가, `ai_oee_overlay.js` 로드 |
| 2026-03-06 | **[F7 구현 완료]** `page/data/js/ai_downtime_risk.js` — 다운타임 테이블 AI 위험도 열 동적 주입: MutationObserver로 tbody 변화 감지, `ai_maintenance.php` 위험도 매핑, DANGER/CAUTION/NORMAL 뱃지 |
| 2026-03-06 | `page/data/data_downtime.php` — 테이블 헤더에 "AI Risk" 열 추가(DETAIL 바로 앞), colspan 10→11, `ai_downtime_risk.js` 로드 |
| 2026-03-06 | **[F12 구현 완료]** `page/data/proc/ai_stream_analysis.php` — SSE 실시간 스트리밍 AI 분석 엔드포인트: 연결 시 오늘 OEE Z-Score 이상, 활성 다운타임, 정비 위험 머신 초기 이벤트 전송 → 15초 폴링으로 신규 이벤트 스트리밍 (5분 세션) |
| 2026-03-06 | `page/data/js/ai_stream_monitor.js` — SSE 클라이언트: anomaly/downtime_new/maintenance_risk 이벤트 수신 → 슬라이드인 애니메이션 카드 피드 렌더링 (최대 15건), 자동 재연결(15초) |
| 2026-03-06 | **[F13 구현 완료]** `page/data/proc/ai_optimization.php` — 생산 최적화 제안 API: 최근 14일 라인별 Availability/Performance/Quality 병목 컴포넌트 식별, 잠재 OEE 향상 추정, P1/P2/P3 우선순위 및 개선 제안 4건 반환 |
| 2026-03-06 | `page/data/js/ai_optimization.js` — 최적화 제안 프론트엔드: 우선순위 카드 + 현재/잠재 OEE 바 시각화 + 병목 하이라이트 + 개선 제안 접기/펼치기 |
| 2026-03-06 | `page/data/ai_dashboard.php` — 섹션 4(실시간 AI 스트리밍) + 섹션 5(생산 최적화 제안) 추가, `ai_stream_monitor.js` · `ai_optimization.js` 로드 |
| 2026-03-06 | `page/data/css/ai_dashboard.css` — F12 스트림 피드(.ai-stream-feed/event/event__header 등) + F13 최적화 카드(.ai-opt-card/bar-wrap/bottleneck/components/suggestions 등) 스타일 추가 |

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
