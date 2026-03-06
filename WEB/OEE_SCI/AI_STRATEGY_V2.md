# OEE_SCI V2 — AI 도입 전략 설계서

> 작성일: 2026-03-06
> 대상 버전: OEE_SCI_V2
> 작성자: Claude Sonnet 4.6 (분석 보조)
> 상태: Phase 1 완료 / Phase 2 진행 중 (F5~F7 완료) / Phase 3 완료 (F12~F13 완료)

---

## 0. 구현 현황

> 마지막 업데이트: 2026-03-06

### Phase 1 — 통계 기반 AI ✅ 완료

| 기능 ID | 파일 | 설명 | 상태 |
|---------|------|------|------|
| F1 | `lib/statistics.lib.php` | 지수평활법, Z-Score, 선형회귀, 신뢰구간 통계 엔진 | ✅ 완료 |
| F2 | `page/data/proc/ai_oee_prediction.php` | 과거 30일 계절성 기반 OEE 예측 API (90% CI) | ✅ 완료 |
| F3 | `page/data/proc/ai_anomaly.php` | Z-Score + 연쇄 이상 감지 API | ✅ 완료 |
| F4 | `page/data/proc/ai_maintenance.php` | 예방정비 위험도 스코어 API (런타임 40%+다운타임 빈도 35%+OEE 불안정 25%) | ✅ 완료 |
| F4 | `page/data/ai_dashboard.php` | AI Intelligence Dashboard 신규 페이지 | ✅ 완료 |
| F4 | `page/data/js/ai_dashboard.js` | 대시보드 프론트엔드 (Chart.js 예측선+CI, 60초 자동갱신) | ✅ 완료 |
| F4 | `page/data/css/ai_dashboard.css` | AI 전용 스타일 (요약카드/뱃지/스피너/히트맵/랭킹 컴포넌트) | ✅ 완료 |
| F4 | `inc/nav-fiori.php` | 네비게이션에 "AI Dashboard" 메뉴 추가 | ✅ 완료 |

### Phase 2 — 기존 페이지 AI 강화 (진행 중)

| 기능 ID | 파일 | 설명 | 상태 |
|---------|------|------|------|
| F5 | `page/data/proc/ai_quality_sentinel.php` | 품질 파수꾼 API: 파레토 분석, 24H 히트맵, 머신 위험 랭킹, OEE Pearson 상관계수 | ✅ 완료 |
| F5 | `page/data/js/ai_quality_sentinel.js` | 파레토 콤보차트 + CSS 히트맵(6열×4행) + 머신 랭킹 카드 | ✅ 완료 |
| F5 | `page/data/data_defective.php` | AI 품질 파수꾼 섹션 삽입 | ✅ 완료 |
| F6 | `page/data/js/ai_oee_overlay.js` | OEE 트렌드 차트에 AI 예측선+신뢰구간 오버레이 (monkeypatch) | ✅ 완료 |
| F6 | `page/data/data_oee.php` | OEE 트렌드 헤더에 AI POWERED 뱃지 추가 | ✅ 완료 |
| F7 | `page/data/js/ai_downtime_risk.js` | 다운타임 테이블 AI 위험도 열 동적 주입 (MutationObserver) | ✅ 완료 |
| F7 | `page/data/data_downtime.php` | 테이블 헤더 "AI Risk" 열 추가 | ✅ 완료 |
| F8 | `lib/ai_helper.lib.php` | Claude API cURL 래퍼 | ⏳ 미구현 |
| F9 | `api/ai/chat.php` | AI 자연어 어시스턴트 REST 엔드포인트 | ⏳ 미구현 |
| F10 | `page/data/ai_dashboard.php` (채팅 패널) | AI 채팅 UI 패널 | ⏳ 미구현 |
| F11 | `page/report/report.php` | AI 리포트 생성기 (Claude API) | ⏳ 미구현 |

### Phase 3 — 고도화 ✅ 완료

| 기능 ID | 파일 | 설명 | 상태 |
|---------|------|------|------|
| F12 | `page/data/proc/ai_stream_analysis.php` | SSE 실시간 스트리밍: OEE 이상·활성 다운타임·정비 위험 초기 전송 + 15초 폴링 (5분 세션) | ✅ 완료 |
| F12 | `page/data/js/ai_stream_monitor.js` | SSE 클라이언트: 슬라이드인 카드 피드 (최대 15건), 자동 재연결 | ✅ 완료 |
| F13 | `page/data/proc/ai_optimization.php` | 생산 최적화 API: Availability/Performance/Quality 병목 분석, 잠재 OEE 추정, P1/P2/P3 우선순위 | ✅ 완료 |
| F13 | `page/data/js/ai_optimization.js` | 최적화 제안 프론트엔드: 우선순위 카드 + OEE 바 시각화 + 병목 하이라이트 + 개선 제안 | ✅ 완료 |

---

## 1. 현재 시스템 구조 요약

| 항목 | 내용 |
|------|------|
| 기술 스택 | PHP + MySQL + jQuery + Chart.js + SSE |
| UI 프레임워크 | SAP Fiori 디자인 시스템 |
| 데이터 수집 | 패턴재봉기 → REST API → MySQL |
| 실시간 처리 | Server-Sent Events (5초 폴링) |
| 핵심 데이터 | `data_oee`, `data_oee_rows_hourly`, Downtime, Defective, Andon |
| 계층 구조 | Factory → Line → Machine |

---

## 2. AI 도입 핵심 방향

V1의 `dashboard.php`는 "AI OEE CS DASHBOARD"라는 이름을 갖고 있지만 실제 AI 기능이 없다.
V2에서는 **진짜 AI Dashboard를 별도 페이지**(`ai_dashboard.php`)로 신규 구현하고,
기존 페이지들에도 AI 분석 기능을 단계적으로 추가한다.

---

## 3. AI Dashboard 신규 페이지 설계

### 파일 위치

```
page/data/ai_dashboard.php
page/data/js/ai_dashboard.js
page/data/proc/ai_oee_prediction.php
page/data/proc/ai_anomaly.php
page/data/proc/ai_maintenance.php
```

### 화면 구성 (레이아웃)

```
+--- AI OEE Intelligence Dashboard ------------------------------------------+
| [Factory] [Line] [Machine] [Today] [Date Range] [Shift] [새로고침]          |
+-------------+-------------+-------------+------------------------------------+
| OEE 예측    | 위험 기계   | 이상 감지   | AI 인사이트                        |
| 다음 4H:74% | 3대 감지    | 0건 활성    | 업데이트                           |
+-------------+-------------+-------------+------------------------------------+
|                                                                              |
|  OEE 트렌드 + AI 예측선 (실선=실제, 점선=예측)                               |
|  100% |                                                  . . .               |
|   80% |--------------------+                        +------                  |
|   60% |                    +------------------------+                        |
|       | 08H  10H  12H  14H  16H  18H  20H  [22H  24H  02H]                  |
|                                            <- 예측구간(점선) ->               |
+-------------------------------------+----------------------------------------+
| 라인별 AI 건강지수                   | 예방정비 권고 목록                      |
| Line A: ████████  82%  정상          | 1순위: M-012 (위험 89%)                |
| Line B: █████     61%  위험          |  → 예상 다음 다운타임 2.1H 후           |
| Line C: ███████   74%  주의          | 2순위: M-007 (주의 72%)                |
| Line D: ████████  80%  정상          | 3순위: M-023 (주의 65%)                |
+-------------------------------------+----------------------------------------+
|                                                                              |
|  AI 어시스턴트 (Claude API 연동)                                              |
|  [오늘 가장 문제있는 라인은?]  [이번 주 불량 원인 분석]                        |
|  [다음 교대 OEE 예측]          [예방정비 우선순위]                             |
|  +------------------------------------------------------------------------+  |
|  | 여기에 질문을 입력하세요...                                    [전송]  |  |
|  +------------------------------------------------------------------------+  |
|  AI: "현재 Line B의 OEE가 61%로 가장 낮습니다.                               |
|       M-012 머신에서 오전 9~11시 사이 다운타임 3회 발생이 주요 원인입니다..." |
+------------------------------------------------------------------------------+
```

---

## 4. AI 기능별 상세 설계

### 기능 1 — OEE 예측 (Predictive OEE)

**적용 위치**: AI Dashboard, data_oee.php 차트에 예측선 추가

**동작 방식**:
- `data_oee_rows_hourly` 테이블에서 과거 30일 시간대별 OEE 패턴 학습
- 요일 × 시간대 조합 매트릭스로 다음 4~8시간 OEE 예측
- Chart.js에 점선 예측 구간(Confidence Interval) 시각화

**알고리즘**: 지수평활법(Exponential Smoothing) + 요일 계절성 보정

**입력 데이터**:
- 최근 4주 동일 요일/시간대 OEE 데이터
- 현재 시간대 Availability, Performance, Quality 각각

**출력**:
- 다음 4시간 시간대별 예측 OEE (%)
- 신뢰구간 상한/하한

**API 파일**: `page/data/proc/ai_oee_prediction.php`

**활용 가치**: "현재 오후 2시 OEE 72% → 다음 2시간 예상 68%로 하락 예측 (월요일 오후 패턴 기반)"

---

### 기능 2 — 예측적 유지보수 (Predictive Maintenance)

**적용 위치**: AI Dashboard 위험 기계 패널, data_downtime.php에 위험 지표 열 추가

**동작 방식**:
- 머신별 다운타임 발생 주기 패턴 분석 (`data_downtime` 이력)
- 마지막 다운타임 이후 경과 시간 + 평균 주기 비교
- 위험도 스코어 (0~100) 계산

**위험도 계산식**:
```
위험도 = (현재_런타임 / 평균_고장간격)     × 가중치 A
       + (다운타임_빈도_증가율)             × 가중치 B
       + (사이클타임_이상_감지 여부)        × 가중치 C
```

**위험 등급**:
| 스코어 | 등급 | 표시 |
|--------|------|------|
| 80~100 | 위험 | 빨간색 |
| 50~79  | 주의 | 노란색 |
| 0~49   | 정상 | 초록색 |

**API 파일**: `page/data/proc/ai_maintenance.php`

---

### 기능 3 — 이상 감지 (Anomaly Detection)

**적용 위치**: AI Dashboard 실시간 알림 패널, 기존 모니터링 페이지

**동작 방식**:
- Z-Score 기반: OEE, 불량률, 다운타임 빈도가 ±2σ 벗어날 시 즉시 감지
- 패턴 이상: 이전 시간 대비 OEE -15% 이상 급락 감지
- 연쇄 이상: 동일 라인에서 30분 내 다수 머신 동시 문제 감지

**감지 대상**:
- OEE 급락 (단기 -15% 이상)
- 불량률 급증 (전일 평균 대비 +3σ 이상)
- 다운타임 빈도 이상 증가
- 동일 라인 연쇄 다운타임

**API 파일**: `page/data/proc/ai_anomaly.php`

---

### 기능 4 — AI 어시스턴트 (Claude API 연동)

**적용 위치**: AI Dashboard 하단 Q&A 패널

**동작 방식**:
1. 운영자가 자연어로 질문 입력
2. PHP 백엔드가 관련 DB 데이터 자동 수집
3. Claude API에 데이터 컨텍스트 + 질문 전송
4. 한국어 자연어 답변 반환 및 화면 표시

**지원 질문 예시**:
- "오늘 어느 라인이 가장 문제가 많나요?"
- "이번 주 불량 원인을 분석해줘"
- "다음 교대 OEE를 예측해줘"
- "예방정비가 필요한 기계 순위를 알려줘"

**API 파일**: `api/ai/chat.php`

**라이브러리**: `lib/ai_helper.lib.php` (Claude API cURL 래퍼)

**Claude API 모델**: `claude-sonnet-4-6`

---

### 기능 5 — AI 품질 파수꾼 (Quality Sentinel)

**적용 위치**: data_defective.php에 AI 분석 탭 추가

**동작 방식**:
- 불량 발생 패턴 클러스터링 (시간대 × 머신 × 불량유형)
- 파레토 분석 자동화: "불량의 80%가 이 3가지 유형에서 발생"
- 상관관계 분석: 특정 불량 유형 증가 → 이후 OEE 영향도 예측

---

### 기능 6 — AI 리포트 생성기 (Report Generator)

**적용 위치**: report.php에 "AI 분석 보고서" 버튼 추가

**동작 방식**:
- 선택 기간 데이터를 Claude API로 전송
- 경영자용 요약 보고서 자동 생성 (한국어)
- 엑셀 내보내기 시 AI 코멘트 시트 자동 포함

---

## 5. 구현 파일 구조 (실제 현황)

```
OEE_SCI_V2/
├── page/
│   └── data/
│       ├── ai_dashboard.php              # ✅ AI 통합 대시보드 (섹션1~5)
│       ├── data_defective.php            # ✅ AI 품질 파수꾼 섹션 추가 (F5)
│       ├── data_oee.php                  # ✅ AI 예측선 오버레이 뱃지 추가 (F6)
│       ├── data_downtime.php             # ✅ AI Risk 열 추가 (F7)
│       ├── css/
│       │   └── ai_dashboard.css          # ✅ AI 전용 스타일 (요약카드/히트맵/스트림/최적화)
│       ├── js/
│       │   ├── ai_dashboard.js           # ✅ 대시보드 로직 (예측/이상/정비/건강지수)
│       │   ├── ai_quality_sentinel.js    # ✅ 품질 파수꾼 프론트엔드 (F5)
│       │   ├── ai_oee_overlay.js         # ✅ OEE 트렌드 예측선 오버레이 (F6)
│       │   ├── ai_downtime_risk.js       # ✅ 다운타임 테이블 위험도 주입 (F7)
│       │   ├── ai_stream_monitor.js      # ✅ 실시간 스트리밍 피드 (F12)
│       │   └── ai_optimization.js        # ✅ 생산 최적화 카드 (F13)
│       └── proc/
│           ├── ai_oee_prediction.php     # ✅ OEE 예측 API (지수평활법+계절성)
│           ├── ai_anomaly.php            # ✅ 이상 감지 API (Z-Score)
│           ├── ai_maintenance.php        # ✅ 예방정비 위험도 API
│           ├── ai_quality_sentinel.php   # ✅ 품질 파수꾼 API (파레토+히트맵+상관)
│           ├── ai_stream_analysis.php    # ✅ 실시간 스트리밍 SSE 엔드포인트 (F12)
│           └── ai_optimization.php       # ✅ 생산 최적화 제안 API (F13)
│
├── api/
│   └── ai/
│       ├── chat.php                      # ⏳ Claude API 채팅 (F9, 미구현)
│       └── quality_analysis.php          # ⏳ 품질 분석 (미구현)
│
└── lib/
    ├── ai_helper.lib.php                 # ⏳ Claude API cURL 래퍼 (F8, 미구현)
    └── statistics.lib.php                # ✅ 통계 함수 (지수평활법/Z-Score/선형회귀)
```

---

## 6. 구현 로드맵

### Phase 1 — 통계 기반 AI (DB 데이터만 활용, 빠른 구현) ✅ 완료

| 기능 | 구현 난이도 | 비즈니스 가치 | 상태 |
|------|------------|--------------|------|
| OEE 예측 (지수평활법 + 계절성) | 중 | 높음 | ✅ 완료 |
| 이상 감지 (Z-Score + 연쇄 감지) | 중 | 매우 높음 | ✅ 완료 |
| 예방정비 위험도 스코어 | 중 | 매우 높음 | ✅ 완료 |
| AI Dashboard 신규 페이지 (라인 건강지수 포함) | 하 | 높음 | ✅ 완료 |

### Phase 2 — 기존 페이지 AI 강화 + Claude API 연동 (진행 중)

| 기능 | 구현 난이도 | 비즈니스 가치 | 상태 |
|------|------------|--------------|------|
| AI 품질 파수꾼 (파레토+히트맵+OEE 상관) | 중 | 높음 | ✅ 완료 |
| OEE 트렌드 차트 AI 예측선 오버레이 | 중 | 높음 | ✅ 완료 |
| 다운타임 테이블 AI 위험도 열 | 하 | 중 | ✅ 완료 |
| Claude API cURL 래퍼 (`ai_helper.lib.php`) | 중 | 매우 높음 | ⏳ 미구현 |
| AI 자연어 어시스턴트 (`api/ai/chat.php`) | 상 | 매우 높음 | ⏳ 미구현 |
| AI 채팅 패널 (ai_dashboard.php) | 중 | 매우 높음 | ⏳ 미구현 |
| AI 리포트 생성 (report.php) | 상 | 높음 | ⏳ 미구현 |

### Phase 3 — 고도화 ✅ 완료

| 기능 | 구현 난이도 | 비즈니스 가치 | 상태 |
|------|------------|--------------|------|
| 실시간 스트리밍 AI 분석 (SSE + 이벤트 피드) | 높음 | 매우 높음 | ✅ 완료 |
| 생산 최적화 제안 (병목 분석 + 우선순위 카드) | 높음 | 매우 높음 | ✅ 완료 |

---

## 7. 즉시 활용 가능한 기존 데이터

현재 V1 DB에 이미 존재하여 AI 학습/분석에 바로 활용 가능한 데이터:

| 테이블 | AI 활용 용도 |
|--------|-------------|
| `data_oee_rows_hourly` | 시간대별 OEE 예측 훈련 데이터 (핵심) |
| `data_downtime` + `cycletime` 컬럼 | 예방정비 위험도 계산 |
| `data_defective` 이력 | 불량 패턴 클러스터링 |
| `data_andon` 이력 | 안돈 예측 및 연쇄 발생 감지 |
| `shift_idx` + `work_date` 조합 | 교대별/요일별 패턴 분석 |

---

## 8. 기술 구현 메모

### lib/statistics.lib.php 주요 함수 목록

```php
// 이동평균
function movingAverage(array $data, int $window): array

// 지수평활법 예측
function exponentialSmoothing(array $data, float $alpha): array

// Z-Score 이상 감지
function detectAnomalies(array $data, float $threshold = 2.0): array

// 선형 회귀
function linearRegression(array $x, array $y): array  // ['slope', 'intercept', 'r2']

// 요일별 계절성 보정
function seasonalAdjust(array $data, string $date, int $hour): float
```

### lib/ai_helper.lib.php 주요 함수 목록

```php
class AiHelper {
    // Claude API 호출 (cURL)
    public function chat(string $systemPrompt, string $userMessage): string

    // 공장 현황 컨텍스트 생성
    public function buildFactoryContext(array $oeeData, array $downtimeData, array $defectiveData): string

    // OEE 분석 요약 요청
    public function analyzeOee(array $stats): string

    // 예방정비 권고 생성
    public function recommendMaintenance(array $machineRiskData): string
}
```

### Claude API 연동 패턴 (PHP cURL)

```php
// lib/ai_helper.lib.php
$payload = [
    'model' => 'claude-sonnet-4-6',
    'max_tokens' => 1024,
    'messages' => [
        ['role' => 'user', 'content' => $contextPrompt . "\n\n" . $userQuestion]
    ]
];

$ch = curl_init('https://api.anthropic.com/v1/messages');
curl_setopt($ch, CURLOPT_HTTPHEADER, [
    'x-api-key: ' . getenv('ANTHROPIC_API_KEY'),
    'anthropic-version: 2023-06-01',
    'content-type: application/json'
]);
curl_setopt($ch, CURLOPT_POSTFIELDS, json_encode($payload));
```

### .env 추가 항목 (V2)

```
ANTHROPIC_API_KEY=sk-ant-...
AI_ENABLED=true
AI_PREDICTION_DAYS=30
AI_ANOMALY_THRESHOLD=2.0
```

---

## 9. 네비게이션 메뉴 변경 계획

`inc/nav-fiori.php`에 AI 메뉴 항목 추가:

```
기존 메뉴:
  Dashboard | OEE | Downtime | Defective | Andon | Log | Report | 관리

V2 추가:
  Dashboard | [AI Dashboard] | OEE | Downtime | Defective | Andon | Log | Report | 관리
```

---

## 10. 구현 순서

### 완료된 항목

1. ✅ `lib/statistics.lib.php` — 통계 함수 구현
2. ✅ `page/data/proc/ai_oee_prediction.php` — OEE 예측 API
3. ✅ `page/data/proc/ai_anomaly.php` — 이상 감지 API
4. ✅ `page/data/proc/ai_maintenance.php` — 예방정비 위험도 API
5. ✅ `page/data/ai_dashboard.php` — AI 대시보드 HTML
6. ✅ `page/data/js/ai_dashboard.js` — 프론트엔드 로직
7. ✅ `inc/nav-fiori.php` — 메뉴 항목 추가
8. ✅ `page/data/proc/ai_quality_sentinel.php` — 품질 파수꾼 API (F5)
9. ✅ `page/data/js/ai_quality_sentinel.js` — 품질 파수꾼 프론트엔드 (F5)
10. ✅ `page/data/data_defective.php` — AI 품질 파수꾼 섹션 삽입 (F5)
11. ✅ `page/data/js/ai_oee_overlay.js` — OEE 차트 AI 예측선 오버레이 (F6)
12. ✅ `page/data/data_oee.php` — AI POWERED 뱃지 및 오버레이 JS 로드 (F6)
13. ✅ `page/data/js/ai_downtime_risk.js` — 다운타임 테이블 AI 위험도 열 (F7)
14. ✅ `page/data/data_downtime.php` — AI Risk 열 헤더 추가 (F7)

15. ✅ `page/data/proc/ai_stream_analysis.php` — SSE 스트리밍 엔드포인트 (F12)
16. ✅ `page/data/js/ai_stream_monitor.js` — 스트리밍 피드 JS (F12)
17. ✅ `page/data/proc/ai_optimization.php` — 생산 최적화 API (F13)
18. ✅ `page/data/js/ai_optimization.js` — 최적화 제안 JS (F13)

### 미구현 항목 (Phase 2 Claude API 연동)

19. ⏳ `lib/ai_helper.lib.php` — Claude API cURL 래퍼 (F8)
20. ⏳ `api/ai/chat.php` — AI 채팅 엔드포인트 (F9)
21. ⏳ AI 채팅 패널 UI — `ai_dashboard.php` (F10)
22. ⏳ AI 리포트 생성기 — `report.php` (F11)
