<?php
/**
 * AI 생산 최적화 제안 API v5
 * v4 대비 수정:
 *  - date_range 파라미터 지원 추가 (기존 고정 14일 → 동적)
 *    · today / yesterday → 14일 분석 (기본 동작 유지)
 *    · 7d              → 7일
 *    · 30d             → 30일
 *  - HAVING 최소 데이터 수를 date_range에 맞게 조정
 */

require_once(__DIR__ . '/../../../lib/db.php');

header('Content-Type: application/json');
header('Cache-Control: no-cache');

$factory_filter = isset($_GET['factory_filter']) ? trim($_GET['factory_filter']) : '';
$line_filter    = isset($_GET['line_filter'])    ? trim($_GET['line_filter'])    : '';
$date_range     = isset($_GET['date_range'])     ? trim($_GET['date_range'])     : 'today';

// date_range → 분석 기간
switch ($date_range) {
  case '7d':
    $interval_days = 7;
    $min_data_cnt  = 3;
    break;
  case '30d':
    $interval_days = 30;
    $min_data_cnt  = 7;
    break;
  default: // today, yesterday
    $interval_days = 14;
    $min_data_cnt  = 5;
}

$where_parts = [];
$params      = [];

if ($factory_filter !== '') { $where_parts[] = 'do.factory_idx = ?'; $params[] = $factory_filter; }
if ($line_filter    !== '') { $where_parts[] = 'do.line_idx = ?';    $params[] = $line_filter; }

$where_sql = $where_parts ? 'AND ' . implode(' AND ', $where_parts) : '';

define('OEE_TARGET',   85.0);
define('AVAIL_TARGET', 90.0);
define('PERF_TARGET',  90.0);
define('QUAL_TARGET',  99.0);

try {
  $half = intdiv($interval_days, 2);

  $sql = "
    SELECT
      il.idx                        AS line_idx,
      il.line_name,
      ift.factory_name,
      AVG(do.oee)                   AS avg_oee,
      AVG(do.availabilty_rate)      AS avg_avail,
      AVG(do.productivity_rate)     AS avg_perf,
      AVG(do.quality_rate)          AS avg_quality,
      COUNT(*)                      AS data_count,
      AVG(CASE WHEN do.work_date >= DATE_SUB(CURDATE(), INTERVAL ? DAY)
               THEN do.oee ELSE NULL END) AS recent_half_oee,
      AVG(CASE WHEN do.work_date BETWEEN DATE_SUB(CURDATE(), INTERVAL ? DAY)
                                     AND DATE_SUB(CURDATE(), INTERVAL ? DAY)
               THEN do.oee ELSE NULL END) AS prev_half_oee
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
  $stmt->execute(array_merge([$half, $interval_days, $half + 1, $interval_days, $min_data_cnt], $params));
  $lines = $stmt->fetchAll(PDO::FETCH_ASSOC);

  if (empty($lines)) {
    echo json_encode([
      'code' => '00',
      'opportunities' => [],
      'summary' => ['msg' => '데이터 부족 (' . $interval_days . '일 내 ' . $min_data_cnt . '건 이상 필요)'],
    ], JSON_UNESCAPED_UNICODE);
    exit;
  }

  $all_oee        = array_column($lines, 'avg_oee');
  $best_oee       = (float)max($all_oee);
  $global_avg_oee = (float)array_sum($all_oee) / count($all_oee);

  $opportunities = [];

  foreach ($lines as $line) {
    $avg_oee    = round((float)$line['avg_oee'],    1);
    $avg_avail  = round((float)$line['avg_avail'],  1);
    $avg_perf   = round((float)$line['avg_perf'],   1);
    $avg_quality= round((float)$line['avg_quality'],1);

    $oee_gap = round(OEE_TARGET - $avg_oee, 1);
    if ($oee_gap <= 0) continue;

    $gaps = [
      'availability' => ['gap' => AVAIL_TARGET - $avg_avail, 'current' => $avg_avail, 'target' => AVAIL_TARGET, 'label' => '가용성'],
      'performance'  => ['gap' => PERF_TARGET  - $avg_perf,  'current' => $avg_perf,  'target' => PERF_TARGET,  'label' => '성능률'],
      'quality'      => ['gap' => QUAL_TARGET  - $avg_quality,'current' => $avg_quality,'target' => QUAL_TARGET, 'label' => '품질률'],
    ];

    $norm_gaps = [];
    foreach ($gaps as $key => $g) {
      $norm_gaps[$key] = $g['target'] > 0 ? ($g['gap'] / $g['target']) : 0;
    }
    arsort($norm_gaps);
    $bottleneck_key = array_key_first($norm_gaps);
    $bottleneck     = $gaps[$bottleneck_key];

    $fix_ratio      = min(1.0, max(0, $bottleneck['gap']) / $bottleneck['target']);
    $potential_gain = round($avg_oee * $fix_ratio * 0.8, 1);
    $potential_oee  = min(99.0, $avg_oee + $potential_gain);

    $recent = (float)($line['recent_half_oee'] ?? $avg_oee);
    $prev   = (float)($line['prev_half_oee']   ?? $avg_oee);
    $trend  = $recent > $prev + 1 ? 'improving' : ($recent < $prev - 1 ? 'declining' : 'stable');

    $priority    = $oee_gap >= 20 ? 'P1' : ($oee_gap >= 10 ? 'P2' : 'P3');
    $suggestions = generateSuggestions($bottleneck_key, $bottleneck['current'], $bottleneck['target']);

    $opportunities[] = [
      'line_name'          => $line['line_name'],
      'factory_name'       => $line['factory_name'],
      'current_oee'        => $avg_oee,
      'target_oee'         => OEE_TARGET,
      'oee_gap'            => $oee_gap,
      'potential_oee'      => round($potential_oee, 1),
      'potential_gain'     => $potential_gain,
      'avg_avail'          => $avg_avail,
      'avg_perf'           => $avg_perf,
      'avg_quality'        => $avg_quality,
      'bottleneck'         => $bottleneck_key,
      'bottleneck_label'   => $bottleneck['label'],
      'bottleneck_current' => round($bottleneck['current'], 1),
      'bottleneck_target'  => $bottleneck['target'],
      'bottleneck_gap'     => round(max(0, $bottleneck['gap']), 1),
      'trend'              => $trend,
      'priority'           => $priority,
      'data_count'         => (int)$line['data_count'],
      'suggestions'        => $suggestions,
      'vs_best'            => round($best_oee - $avg_oee, 1),
    ];
  }

  usort($opportunities, function ($a, $b) {
    if ($a['priority'] !== $b['priority']) return strcmp($a['priority'], $b['priority']);
    return $b['oee_gap'] <=> $a['oee_gap'];
  });

  echo json_encode([
    'code'          => '00',
    'opportunities' => $opportunities,
    'summary' => [
      'total_lines'        => count($lines),
      'lines_below_target' => count($opportunities),
      'global_avg_oee'     => round($global_avg_oee, 1),
      'best_oee'           => round($best_oee, 1),
      'target_oee'         => OEE_TARGET,
      'analysis_days'      => $interval_days,
    ],
  ], JSON_UNESCAPED_UNICODE);

} catch (PDOException $e) {
  echo json_encode(['code' => 'ERR', 'message' => $e->getMessage()]);
}

function generateSuggestions(string $bottleneck, float $current, float $target): array {
  $gap = $target - $current;

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
