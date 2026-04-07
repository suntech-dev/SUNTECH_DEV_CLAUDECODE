<?php

/**
 * ============================================================
 * 파일명: ai_optimization_2.php
 * 목  적: AI 생산 최적화 제안 API v5
 *         라인별 OEE 분석 → 병목 요인 파악 → 개선 제안 생성
 *
 * v4 대비 수정 사항:
 *  - date_range 파라미터 지원 추가 (기존 고정 14일 → 동적)
 *    · today / yesterday → 14일 분석 (기본 동작 유지)
 *    · 7d              → 7일
 *    · 30d             → 30일
 *  - HAVING 최소 데이터 수를 date_range에 맞게 조정
 *    (기간이 짧을수록 최소 건수 완화)
 *
 * 동작 개요:
 *   1) date_range에 따라 분석 기간(7/14/30일) 결정
 *   2) 라인별 OEE·가용성·성능·품질 평균 및 트렌드(전반기 vs 후반기) 산출
 *   3) OEE < 85%(목표) 미달 라인만 대상으로 최적화 기회 분석
 *   4) 가용성·성능·품질 갭(gap) 중 가장 큰 항목 = 병목 요인 선정
 *   5) 병목 요인별 개선 제안(generateSuggestions) 반환
 *
 * Method: GET
 * Params:
 *   factory_filter, line_filter (optional, machine_filter 없음)
 *   date_range: today|yesterday|7d|30d (default: today)
 * Response JSON 구조:
 *   {
 *     code: '00' | 'ERR',
 *     opportunities: [
 *       {
 *         line_name, factory_name,
 *         current_oee, target_oee(85), oee_gap,
 *         potential_oee, potential_gain,
 *         avg_avail, avg_perf, avg_quality,
 *         bottleneck: 'availability'|'performance'|'quality',
 *         bottleneck_label, bottleneck_current, bottleneck_target, bottleneck_gap,
 *         trend: 'improving'|'declining'|'stable',
 *         priority: 'P1'(gap>=20) | 'P2'(gap>=10) | 'P3',
 *         data_count, suggestions: [개선 제안 문자열 배열],
 *         vs_best: 최고 라인 대비 OEE 차이
 *       }
 *     ],
 *     summary: { total_lines, lines_below_target, global_avg_oee, best_oee, target_oee, analysis_days }
 *   }
 *
 * 상수 정의:
 *   OEE_TARGET   = 85.0%  (OEE 목표치)
 *   AVAIL_TARGET = 90.0%  (가용성 목표치)
 *   PERF_TARGET  = 90.0%  (성능 목표치)
 *   QUAL_TARGET  = 99.0%  (품질 목표치)
 * ============================================================
 */

// DB 연결 라이브러리 로드 (statistics 불필요 — 순수 통계는 DB에서 처리)
require_once(__DIR__ . '/../../../lib/db.php');

// JSON 응답 헤더 설정
header('Content-Type: application/json');
header('Cache-Control: no-cache');

// ── GET 파라미터 수집 ────────────────────────────────────────────
$factory_filter = isset($_GET['factory_filter']) ? trim($_GET['factory_filter']) : '';
$line_filter    = isset($_GET['line_filter'])    ? trim($_GET['line_filter'])    : '';
$date_range     = isset($_GET['date_range'])     ? trim($_GET['date_range'])     : 'today';

// ── date_range → 분석 기간 및 최소 데이터 건수 결정 ─────────────
// interval_days : SQL INTERVAL에 사용할 분석 기간(일)
// min_data_cnt  : HAVING 최소 데이터 건수 (기간이 짧을수록 완화)
switch ($date_range) {
    case '7d': // 최근 7일 분석, 최소 3건 이상
        $interval_days = 7;
        $min_data_cnt  = 3;
        break;
    case '30d': // 최근 30일 분석, 최소 7건 이상
        $interval_days = 30;
        $min_data_cnt  = 7;
        break;
    default: // today, yesterday → 14일 분석, 최소 5건 이상 (기본 동작)
        $interval_days = 14;
        $min_data_cnt  = 5;
}

// ── 필터 조건 구성 ───────────────────────────────────────────────
$where_parts = [];
$params      = [];

if ($factory_filter !== '') {
    $where_parts[] = 'do.factory_idx = ?'; // 공장 필터
    $params[] = $factory_filter;
}
if ($line_filter    !== '') {
    $where_parts[] = 'do.line_idx = ?';   // 라인 필터
    $params[] = $line_filter;
}

// 조건이 있으면 'AND ...' 형태로 결합
$where_sql = $where_parts ? 'AND ' . implode(' AND ', $where_parts) : '';

// ── OEE 목표치 상수 정의 ─────────────────────────────────────────
define('OEE_TARGET',   85.0); // 전체 OEE 목표치 (%)
define('AVAIL_TARGET', 90.0); // 가용성(Availability) 목표치 (%)
define('PERF_TARGET',  90.0); // 성능(Performance) 목표치 (%)
define('QUAL_TARGET',  99.0); // 품질(Quality) 목표치 (%)

try {
    // 전반기/후반기 비교를 위한 절반 기간 계산
    // (예: 14일이면 $half = 7일 → 최근 7일 vs 이전 7일)
    $half = intdiv($interval_days, 2);

    // ════════════════════════════════════════════════════════
    // STEP 1. 라인별 OEE·가용성·성능·품질 집계 쿼리
    //   - avg_oee, avg_avail, avg_perf, avg_quality: 기간 전체 평균
    //   - recent_half_oee: 최근 절반 기간 평균 OEE (트렌드 비교용)
    //   - prev_half_oee  : 이전 절반 기간 평균 OEE (트렌드 비교용)
    //   - HAVING COUNT(*) >= min_data_cnt: 데이터 부족 라인 제외
    //   - ORDER BY avg_oee ASC: OEE가 낮은 라인 우선 (최적화 필요 순)
    //
    //   바인딩 파라미터 순서:
    //     [$half, $interval_days, $half+1, $interval_days, $min_data_cnt, ...$params]
    //     1: recent_half_oee 시작 기준 (INTERVAL half DAY)
    //     2: prev_half_oee 시작 기준 (INTERVAL interval_days DAY)
    //     3: prev_half_oee 종료 기준 (INTERVAL half+1 DAY)
    //     4: 전체 WHERE 절 기준 (INTERVAL interval_days DAY)
    //     5: HAVING 최소 건수
    // ════════════════════════════════════════════════════════
    $sql = "
    SELECT
      il.idx                        AS line_idx,
      il.line_name,
      ift.factory_name,
      AVG(do.oee)                   AS avg_oee,          -- 기간 전체 평균 OEE
      AVG(do.availabilty_rate)      AS avg_avail,        -- 기간 전체 평균 가용성
      AVG(do.productivity_rate)     AS avg_perf,         -- 기간 전체 평균 성능
      AVG(do.quality_rate)          AS avg_quality,      -- 기간 전체 평균 품질
      COUNT(*)                      AS data_count,       -- 데이터 건수
      AVG(CASE WHEN do.work_date >= DATE_SUB(CURDATE(), INTERVAL ? DAY)
               THEN do.oee ELSE NULL END) AS recent_half_oee, -- 최근 절반 기간 평균 OEE
      AVG(CASE WHEN do.work_date BETWEEN DATE_SUB(CURDATE(), INTERVAL ? DAY)
                                     AND DATE_SUB(CURDATE(), INTERVAL ? DAY)
               THEN do.oee ELSE NULL END) AS prev_half_oee   -- 이전 절반 기간 평균 OEE
    FROM data_oee do
    JOIN info_line    il  ON do.line_idx = il.idx
    JOIN info_factory ift ON do.factory_idx = ift.idx
    WHERE do.work_date >= DATE_SUB(CURDATE(), INTERVAL ? DAY)
      $where_sql
    GROUP BY il.idx, il.line_name, ift.factory_name
    HAVING COUNT(*) >= ?
    ORDER BY avg_oee ASC
  ";

    $stmt = $pdo->prepare($sql);
    // 바인딩 파라미터: 기간 관련 5개 + 필터 파라미터
    $stmt->execute(array_merge([$half, $interval_days, $half + 1, $interval_days, $min_data_cnt], $params));
    $lines = $stmt->fetchAll(PDO::FETCH_ASSOC);

    // 데이터가 없으면 빈 결과 반환
    if (empty($lines)) {
        echo json_encode([
            'code' => '00',
            'opportunities' => [],
            'summary' => ['msg' => 'Data shortage (requires ' . $min_data_cnt . ' or more records within ' . $interval_days . ' days)'],
        ], JSON_UNESCAPED_UNICODE);
        exit;
    }

    // ── 전체 통계 계산 ───────────────────────────────────────
    $all_oee        = array_column($lines, 'avg_oee');
    $best_oee       = (float)max($all_oee);         // 전체 라인 중 최고 OEE
    $global_avg_oee = (float)array_sum($all_oee) / count($all_oee); // 전체 라인 평균 OEE

    $opportunities = []; // 최적화 기회 결과 배열

    // ════════════════════════════════════════════════════════
    // STEP 2. 라인별 최적화 기회 분석
    //   - OEE 목표치(85%) 미달 라인만 처리 (달성 라인은 건너뜀)
    //   - 병목 요인: 가용성·성능·품질 중 목표 대비 갭 비율이 가장 큰 항목
    //   - 잠재 향상폭(potential_gain): 병목 개선 시 80% 달성 가정
    // ════════════════════════════════════════════════════════
    foreach ($lines as $line) {
        $avg_oee     = round((float)$line['avg_oee'],     1);
        $avg_avail   = round((float)$line['avg_avail'],   1);
        $avg_perf    = round((float)$line['avg_perf'],    1);
        $avg_quality = round((float)$line['avg_quality'], 1);

        // OEE 목표 갭 계산 (목표치 85% - 현재 OEE)
        $oee_gap = round(OEE_TARGET - $avg_oee, 1);
        // 목표 달성 라인은 건너뜀 (갭이 0 이하)
        if ($oee_gap <= 0) continue;

        // ── 각 요소별 갭(현재 vs 목표) 계산 ────────────────────
        $gaps = [
            'availability' => ['gap' => AVAIL_TARGET - $avg_avail, 'current' => $avg_avail, 'target' => AVAIL_TARGET, 'label' => 'Availability'],
            'performance'  => ['gap' => PERF_TARGET  - $avg_perf,  'current' => $avg_perf,  'target' => PERF_TARGET,  'label' => 'Performance'],
            'quality'      => ['gap' => QUAL_TARGET  - $avg_quality, 'current' => $avg_quality, 'target' => QUAL_TARGET, 'label' => 'Quality'],
        ];

        // ── 정규화된 갭 비율로 병목 요인 선정 ──────────────────
        // 갭을 목표치로 나눠 비율화 → 가장 큰 비율의 요소 = 병목
        $norm_gaps = [];
        foreach ($gaps as $key => $g) {
            $norm_gaps[$key] = $g['target'] > 0 ? ($g['gap'] / $g['target']) : 0;
        }
        arsort($norm_gaps); // 내림차순 정렬 (갭 비율 큰 순)
        $bottleneck_key = array_key_first($norm_gaps); // 가장 큰 갭 비율의 요소
        $bottleneck     = $gaps[$bottleneck_key];      // 병목 요인 상세 정보

        // ── 잠재 개선 효과 계산 ──────────────────────────────────
        // fix_ratio: 병목 갭 중 달성 가능 비율 (0~1)
        // potential_gain: 현재 OEE × fix_ratio × 0.8 (80% 실현 가정)
        // potential_oee : 현재 OEE + 잠재 향상폭 (최대 99%)
        $fix_ratio      = min(1.0, max(0, $bottleneck['gap']) / $bottleneck['target']);
        $potential_gain = round($avg_oee * $fix_ratio * 0.8, 1);
        $potential_oee  = min(99.0, $avg_oee + $potential_gain);

        // ── 트렌드 분석: 최근 절반 기간 vs 이전 절반 기간 OEE 비교 ─
        $recent = (float)($line['recent_half_oee'] ?? $avg_oee);
        $prev   = (float)($line['prev_half_oee']   ?? $avg_oee);
        // 1%p 이상 차이 있을 때만 improving/declining 판정
        $trend  = $recent > $prev + 1 ? 'improving' : ($recent < $prev - 1 ? 'declining' : 'stable');

        // ── 우선순위 결정 ─────────────────────────────────────────
        // P1: OEE 갭 20%p 이상 (즉각 조치 필요)
        // P2: OEE 갭 10%p 이상 (우선 개선)
        // P3: OEE 갭 10%p 미만 (모니터링)
        $priority    = $oee_gap >= 20 ? 'P1' : ($oee_gap >= 10 ? 'P2' : 'P3');
        // 병목 요인별 구체적 개선 제안 생성
        $suggestions = generateSuggestions($bottleneck_key, $bottleneck['current'], $bottleneck['target']);

        $opportunities[] = [
            'line_name'          => $line['line_name'],
            'factory_name'       => $line['factory_name'],
            'current_oee'        => $avg_oee,             // 현재 평균 OEE
            'target_oee'         => OEE_TARGET,           // 목표 OEE (85%)
            'oee_gap'            => $oee_gap,             // OEE 갭 (목표 - 현재)
            'potential_oee'      => round($potential_oee, 1), // 개선 후 예상 OEE
            'potential_gain'     => $potential_gain,      // 잠재 OEE 향상폭 (%p)
            'avg_avail'          => $avg_avail,           // 평균 가용성
            'avg_perf'           => $avg_perf,            // 평균 성능
            'avg_quality'        => $avg_quality,         // 평균 품질
            'bottleneck'         => $bottleneck_key,      // 병목 요인 키
            'bottleneck_label'   => $bottleneck['label'], // 병목 요인 레이블
            'bottleneck_current' => round($bottleneck['current'], 1), // 병목 현재값
            'bottleneck_target'  => $bottleneck['target'],            // 병목 목표값
            'bottleneck_gap'     => round(max(0, $bottleneck['gap']), 1), // 병목 갭
            'trend'              => $trend,               // 트렌드 방향
            'priority'           => $priority,            // 우선순위
            'data_count'         => (int)$line['data_count'], // 분석에 사용된 데이터 건수
            'suggestions'        => $suggestions,         // 개선 제안 목록
            'vs_best'            => round($best_oee - $avg_oee, 1), // 최고 라인 대비 OEE 차이
        ];
    }

    // ── 우선순위 정렬: P1 > P2 > P3, 같은 우선순위 내에서는 OEE 갭 큰 순 ──
    usort($opportunities, function ($a, $b) {
        if ($a['priority'] !== $b['priority']) return strcmp($a['priority'], $b['priority']);
        return $b['oee_gap'] <=> $a['oee_gap'];
    });

    // ── 최종 JSON 응답 출력 ──────────────────────────────────
    echo json_encode([
        'code'          => '00',
        'opportunities' => $opportunities, // 최적화 기회 배열
        'summary' => [
            'total_lines'        => count($lines),            // 전체 라인 수
            'lines_below_target' => count($opportunities),    // 목표 미달 라인 수
            'global_avg_oee'     => round($global_avg_oee, 1), // 전체 평균 OEE
            'best_oee'           => round($best_oee, 1),     // 최고 라인 OEE
            'target_oee'         => OEE_TARGET,              // 목표 OEE
            'analysis_days'      => $interval_days,           // 분석 기간 (일)
        ],
    ], JSON_UNESCAPED_UNICODE);
} catch (PDOException $e) {
    // DB 오류 발생 시 오류 코드와 메시지 반환
    echo json_encode(['code' => 'ERR', 'message' => $e->getMessage()]);
}

/**
 * generateSuggestions — 병목 요인별 구체적 개선 제안 생성
 *
 * @param string $bottleneck  병목 요인 키 ('availability'|'performance'|'quality')
 * @param float  $current     현재 병목 값 (%)
 * @param float  $target      병목 목표값 (%)
 * @return array              개선 제안 문자열 배열 (2~4개)
 *
 * 제안 세분화 기준:
 *   - 가용성: gap >= 20 → 즉각 집중 조치 (4가지)
 *             gap >= 10 → 중간 조치 (3가지)
 *             그 외    → 모니터링 수준 (2가지)
 *   - 성능:   gap >= 20 → 즉각 집중 조치 (4가지)
 *             gap >= 10 → 중간 조치 (3가지)
 *             그 외    → 모니터링 수준 (2가지)
 *   - 품질:   gap >= 5  → 즉각 집중 조치 (4가지)
 *             gap >= 2  → 중간 조치 (3가지)
 *             그 외    → 모니터링 수준 (2가지)
 */
function generateSuggestions(string $bottleneck, float $current, float $target): array
{
    $gap = $target - $current; // 목표 대비 갭 계산

    // ── 가용성 병목 개선 제안 ──────────────────────────────────
    if ($bottleneck === 'availability') {
        if ($gap >= 20) return [
            '비계획 다운타임 상위 3건 원인 집중 분석 필요',
            '예방정비 주기 단축 검토 (현재 고장 빈도 이상)',
            '교대 전·후 점검 루틴 강화 및 표준화',
            '소모품/부품 재고 및 교체 이력 점검',
        ];
        if ($gap >= 10) return [
            '주요 비계획 다운타임 유형 파레토 분석',
            '설비 청소·윤활 주기 재검토',
            '빠른 복구를 위한 대기 부품 재고 확보',
        ];
        return ['다운타임 기록 정확도 향상 (계획 vs 비계획 구분)', '단기 고장 재발 패턴 모니터링'];
    }

    // ── 성능 병목 개선 제안 ────────────────────────────────────
    if ($bottleneck === 'performance') {
        if ($gap >= 20) return [
            '사이클 타임 목표 대비 실제 편차 시간대별 분석',
            '작업자 표준 작업 절차(SOP) 재정립 및 교육',
            '원자재·부품 공급 지연에 따른 미세 정지 집계',
            '설비 속도 제한 요인 파악 (재료 공급, 품질 체크 등)',
        ];
        if ($gap >= 10) return [
            '마이크로 정지 발생 시간대 집중 분석',
            '설비 속도 저하 구간 원인 조사',
            '오퍼레이터 작업 동선 최적화 검토',
        ];
        return ['속도 저하 패턴 주간 리포트 작성', '오퍼레이터 작업 피드백 수집 및 개선'];
    }

    // ── 품질 병목 개선 제안 (availability·performance 외 나머지) ─
    if ($gap >= 5) return [
        '주요 불량 유형 파레토 분석 (80% 차지 유형 파악)',
        '불량 다발 시간대·교대·머신 교차 분석',
        '공정 파라미터 (온도/장력/속도) 최적값 재설정',
        '원자재 수입검사 기준 강화',
    ];
    if ($gap >= 2) return [
        '불량 다발 시간대 집중 모니터링',
        '작업자별 불량률 편차 확인',
        '원자재·소모품 품질 수입검사 강화',
    ];
    return ['불량 데이터 입력 정확성 검증', '불량 감소 Best Practice 라인 간 수평 전개'];
}
