<?php
/**
 * Statistics Library for AI OEE Analysis
 * 통계 기반 AI 분석 함수 모음
 *
 * Functions:
 *   - exponentialSmoothing()   : 지수평활법 (단기 예측)
 *   - movingAverage()          : 이동평균
 *   - zScore()                 : Z-Score 이상 감지
 *   - linearRegression()       : 선형 회귀 (추세 예측)
 *   - calcMean()               : 평균
 *   - calcStdDev()             : 표준편차
 *   - calcConfidenceInterval() : 신뢰구간 상하한
 */

/**
 * 지수평활법 (Exponential Smoothing)
 * 단기 시계열 예측에 사용. alpha가 클수록 최근 데이터에 민감.
 *
 * @param  array $data   시계열 값 배열 (순서대로)
 * @param  float $alpha  평활 계수 (0 < alpha < 1, 권장: 0.3)
 * @param  int   $steps  예측할 미래 스텝 수
 * @return array ['smoothed' => [], 'forecast' => []]
 */
function exponentialSmoothing(array $data, float $alpha = 0.3, int $steps = 4): array {
  if (empty($data)) return ['smoothed' => [], 'forecast' => []];

  $smoothed = [];
  $smoothed[0] = $data[0];

  for ($i = 1; $i < count($data); $i++) {
    $smoothed[$i] = $alpha * $data[$i] + (1 - $alpha) * $smoothed[$i - 1];
  }

  // 미래 예측: 마지막 평활값 기반
  $last = end($smoothed);
  $forecast = [];
  for ($i = 0; $i < $steps; $i++) {
    $forecast[] = round($last, 2);
  }

  return ['smoothed' => $smoothed, 'forecast' => $forecast];
}

/**
 * 이동평균 (Moving Average)
 *
 * @param  array $data    시계열 값 배열
 * @param  int   $window  이동 창 크기 (기본 7)
 * @param  int   $steps   예측 스텝 수
 * @return array ['ma' => [], 'forecast' => []]
 */
function movingAverage(array $data, int $window = 7, int $steps = 4): array {
  if (count($data) < $window) {
    $window = max(1, count($data));
  }

  $ma = [];
  for ($i = $window - 1; $i < count($data); $i++) {
    $slice = array_slice($data, $i - $window + 1, $window);
    $ma[] = round(array_sum($slice) / $window, 2);
  }

  // 마지막 window 데이터로 미래 예측
  $lastWindow = array_slice($data, -$window);
  $avg = array_sum($lastWindow) / count($lastWindow);
  $forecast = array_fill(0, $steps, round($avg, 2));

  return ['ma' => $ma, 'forecast' => $forecast];
}

/**
 * 평균 계산
 *
 * @param  array $data
 * @return float
 */
function calcMean(array $data): float {
  if (empty($data)) return 0.0;
  return array_sum($data) / count($data);
}

/**
 * 표준편차 계산 (모집단 표준편차)
 *
 * @param  array $data
 * @return float
 */
function calcStdDev(array $data): float {
  if (count($data) < 2) return 0.0;
  $mean = calcMean($data);
  $variance = 0.0;
  foreach ($data as $val) {
    $variance += pow($val - $mean, 2);
  }
  return sqrt($variance / count($data));
}

/**
 * Z-Score 계산 및 이상 감지
 * Z-Score = (값 - 평균) / 표준편차
 * |Z| > threshold 이면 이상(anomaly)으로 판단
 *
 * @param  array  $data       시계열 값 배열
 * @param  float  $threshold  이상 감지 임계값 (기본: 2.0 = 95% CI)
 * @return array  각 데이터 포인트별 ['value', 'z_score', 'is_anomaly']
 */
function zScore(array $data, float $threshold = 2.0): array {
  if (empty($data)) return [];

  $mean   = calcMean($data);
  $stddev = calcStdDev($data);

  $results = [];
  foreach ($data as $i => $val) {
    $z = ($stddev > 0) ? ($val - $mean) / $stddev : 0.0;
    $results[] = [
      'index'      => $i,
      'value'      => $val,
      'z_score'    => round($z, 3),
      'is_anomaly' => (abs($z) > $threshold),
      'direction'  => ($z > 0) ? 'high' : 'low',
    ];
  }

  return $results;
}

/**
 * 단일 값의 Z-Score 계산 (기존 데이터의 통계 기준)
 *
 * @param  float  $value      검사할 값
 * @param  float  $mean       기준 평균
 * @param  float  $stddev     기준 표준편차
 * @param  float  $threshold  임계값
 * @return array  ['z_score', 'is_anomaly', 'direction']
 */
function zScoreSingle(float $value, float $mean, float $stddev, float $threshold = 2.0): array {
  $z = ($stddev > 0) ? ($value - $mean) / $stddev : 0.0;
  return [
    'z_score'    => round($z, 3),
    'is_anomaly' => (abs($z) > $threshold),
    'direction'  => ($z > 0) ? 'high' : 'low',
  ];
}

/**
 * 선형 회귀 (Linear Regression)
 * y = a + b*x 형태의 추세선
 *
 * @param  array $data   y 값 배열 (x는 인덱스 0,1,2,...)
 * @param  int   $steps  예측 스텝 수
 * @return array ['slope' => b, 'intercept' => a, 'forecast' => [], 'r_squared' => float]
 */
function linearRegression(array $data, int $steps = 4): array {
  $n = count($data);
  if ($n < 2) {
    return ['slope' => 0, 'intercept' => (empty($data) ? 0 : $data[0]), 'forecast' => array_fill(0, $steps, 0), 'r_squared' => 0];
  }

  $sumX = 0; $sumY = 0; $sumXY = 0; $sumX2 = 0;
  for ($i = 0; $i < $n; $i++) {
    $sumX  += $i;
    $sumY  += $data[$i];
    $sumXY += $i * $data[$i];
    $sumX2 += $i * $i;
  }

  $denom = ($n * $sumX2 - $sumX * $sumX);
  if ($denom == 0) {
    return ['slope' => 0, 'intercept' => $sumY / $n, 'forecast' => array_fill(0, $steps, round($sumY / $n, 2)), 'r_squared' => 0];
  }

  $b = ($n * $sumXY - $sumX * $sumY) / $denom;
  $a = ($sumY - $b * $sumX) / $n;

  // R-squared
  $meanY = $sumY / $n;
  $ssTot = 0; $ssRes = 0;
  for ($i = 0; $i < $n; $i++) {
    $predicted = $a + $b * $i;
    $ssTot += pow($data[$i] - $meanY, 2);
    $ssRes += pow($data[$i] - $predicted, 2);
  }
  $rSquared = ($ssTot > 0) ? 1 - ($ssRes / $ssTot) : 0;

  // 미래 예측
  $forecast = [];
  for ($i = 0; $i < $steps; $i++) {
    $xNext = $n + $i;
    $yNext = $a + $b * $xNext;
    // OEE는 0~100 범위로 클리핑
    $forecast[] = round(max(0, min(100, $yNext)), 2);
  }

  return [
    'slope'      => round($b, 4),
    'intercept'  => round($a, 4),
    'forecast'   => $forecast,
    'r_squared'  => round(max(0, $rSquared), 4),
  ];
}

/**
 * 신뢰구간 계산 (95% CI 기본)
 *
 * @param  float $mean    평균
 * @param  float $stddev  표준편차
 * @param  float $z       Z값 (1.96 = 95%, 1.645 = 90%)
 * @return array ['lower' => float, 'upper' => float]
 */
function calcConfidenceInterval(float $mean, float $stddev, float $z = 1.96): array {
  return [
    'lower' => round(max(0, $mean - $z * $stddev), 2),
    'upper' => round(min(100, $mean + $z * $stddev), 2),
  ];
}

/**
 * 요일 × 시간대 매트릭스에서 예측값 조회
 * 과거 동일 요일/시간대 평균으로 예측
 *
 * @param  array  $matrix  ['dow_hour' => [values]] 형태
 * @param  int    $dow     요일 (0=일, 1=월 ... 6=토)
 * @param  array  $hours   예측할 시간 배열 [8, 9, 10, ...]
 * @return array  시간대별 예측값
 */
function seasonalForecast(array $matrix, int $dow, array $hours): array {
  $forecast = [];
  foreach ($hours as $hour) {
    $key = "{$dow}_{$hour}";
    if (isset($matrix[$key]) && !empty($matrix[$key])) {
      $forecast[$hour] = round(calcMean($matrix[$key]), 2);
    } else {
      // 같은 시간대 전체 요일 평균으로 fallback
      $allVals = [];
      for ($d = 0; $d < 7; $d++) {
        $k = "{$d}_{$hour}";
        if (isset($matrix[$k])) {
          $allVals = array_merge($allVals, $matrix[$k]);
        }
      }
      $forecast[$hour] = !empty($allVals) ? round(calcMean($allVals), 2) : null;
    }
  }
  return $forecast;
}
