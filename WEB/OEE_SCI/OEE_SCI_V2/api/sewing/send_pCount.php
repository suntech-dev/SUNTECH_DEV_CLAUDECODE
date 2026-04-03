<?php
// OEE 모니터링 시스템 생산 카운트 API
// 생산 카운트 데이터를 처리하고 OEE 지표를 계산
// 필수 파라미터: mac, sc (생산 카운트)
// OEE 데이터 삽입/업데이트 및 지표 계산 처리
// design_no 파라미터는 하위 호환성을 위해 받지만 더 이상 사용되지 않음
//
// 리셋 감지 기능:
// - 작업자가 근무시간 시작 시 기계 데이터를 리셋하지 않은 경우를 감지
// - 새로 들어온 actual_output(sc)이 이전 값보다 10개 이상 작으면 리셋으로 판단
// - 리셋 감지 시 data_oee 및 data_oee_rows_hourly 테이블의 누적 데이터를 현재 값으로 교체

require_once(__DIR__ . '/../../lib/api_helper.lib.php');

// ─────────────────────────────────────────────────────────────────────
// 헬퍼 함수
// ─────────────────────────────────────────────────────────────────────

/**
 * 생산 데이터 기반 OEE 지표 계산
 *
 * DB에서 비가동(downtime) 누적 시간과 불량(defective) 건수를 조회한 후
 * OEE 3대 지표(가용성·생산성·품질)와 파생 지표를 계산하여 배열로 반환합니다.
 *
 * OEE 계산 공식:
 *   가용성(Availability) = (runtime - downtime) / runtime × 100
 *   이론적 생산량(Theoritical) = productive_runtime / cycletime
 *   생산성(Productivity)  = actual_output / theoritical_output × 100
 *   품질(Quality)         = (actual_output - defective) / actual_output × 100
 *   OEE = 가용성 × 생산성 × 품질 / 10000
 *
 * @param PDO    $pdo                 데이터베이스 연결
 * @param string $mac                 기계 MAC 주소
 * @param string $work_date           작업 날짜 (YYYY-MM-DD)
 * @param int    $shift_idx           시프트 번호
 * @param int    $runtime_sec         실제 경과 근무 시간 (초)
 * @param float  $planned_work_time_hour 계획 근무 시간 (시간 단위)
 * @param int    $std_mc_needed       공정 표준 기계 대수
 * @param int    $target              일일 라인 목표 생산량
 * @param int    $actual_output       현재 실제 생산 수량
 * @return array OEE 지표 배열
 */
function calculateOeeMetrics(PDO $pdo, string $mac, string $work_date, int $shift_idx, int $runtime_sec, float $planned_work_time_hour, int $std_mc_needed, int $target, int $actual_output): array
{
    // 해당 기계·날짜·시프트의 비가동 누적 시간과 불량 건수를 한 번에 조회
    $stmt = $pdo->prepare(
        "SELECT
      (SELECT SUM(duration_sec) FROM data_downtime WHERE mac = :mac AND work_date = :work_date AND shift_idx = :shift_idx) as downtime_duration_sum,
      (SELECT COUNT(*) FROM data_defective WHERE mac = :mac AND work_date = :work_date AND shift_idx = :shift_idx) as defective_count"
    );
    $stmt->execute([':mac' => $mac, ':work_date' => $work_date, ':shift_idx' => $shift_idx]);
    $result = $stmt->fetch(PDO::FETCH_ASSOC);

    $m = [];
    $m['downtime']  = $result['downtime_duration_sum'] ?? 0; // 비가동 누적 시간 (초)
    $m['defective'] = $result['defective_count'] ?? 0;       // 불량 건수

    // 가동 시간: 실제 경과 시간에서 비가동 시간을 제외한 순수 생산 시간
    $productive = max(0, $runtime_sec - $m['downtime']);
    $m['productive_runtime'] = $productive;
    // 가용성: 100% 초과 불가 (max=100)
    $m['availabilty_rate']   = $runtime_sec > 0 ? min(round(($productive / $runtime_sec) * 100, 1), 100) : 0;

    // ── 목표 생산량 기반 속도 지표 계산 ──────────────────────────────
    $tpd = $target; // 라인 일일 목표 생산량 (Total Planned per Day)
    // 시간당 목표 생산량: 계획 근무 시간이 0 이면 0으로 처리
    $tph = ($planned_work_time_hour > 0) ? ($tpd / $planned_work_time_hour) : 0;
    $m['target_line_per_day']  = $tpd;
    $m['target_line_per_hour'] = $tph;
    // 기계 한 대당 목표 생산량: std_mc_needed(표준 기계 대수)로 나눔
    $m['target_mc_per_day']    = ($std_mc_needed > 0) ? ($tpd / $std_mc_needed) : 0;
    $m['target_mc_per_hour']   = ($std_mc_needed > 0) ? ($tph / $std_mc_needed) : 0;
    // 사이클 타임(초): 기계 한 대가 제품 하나를 생산하는 데 걸리는 이론적 시간
    $m['cycletime']            = ($m['target_mc_per_hour'] > 0) ? (3600 / $m['target_mc_per_hour']) : 0;
    // 이론적 생산량: 가동 시간 동안 사이클 타임 기준으로 생산 가능한 이론 수량
    $m['theoritical_output']   = ($m['cycletime'] > 0) ? ($productive / $m['cycletime']) : 0;

    // 생산성: 실제 생산량 / 이론적 생산량 × 100 (최대 200% — 초과 생산 허용)
    $m['productivity_rate'] = ($m['theoritical_output'] > 0)
        ? min(round(($actual_output / $m['theoritical_output']) * 100, 1), 200) : 0;

    // 품질: A등급(양품) 수량 / 실제 생산 수량 × 100 (최대 100%)
    $m['actual_a_grade'] = max(0, $actual_output - $m['defective']);
    $m['quality_rate']   = ($actual_output > 0)
        ? min(round(($m['actual_a_grade'] / $actual_output) * 100, 1), 100) : 0;

    // OEE 종합 = 가용성 × 생산성 × 품질 / 10000 (백분율 곱이므로 10000으로 나눔)
    $m['oee'] = round(($m['availabilty_rate'] * $m['productivity_rate'] * $m['quality_rate']) / 10000, 2);

    return $m;
}

/**
 * 기준 레코드 대비 OEE 델타(증분) 계산
 *
 * data_oee·data_oee_rows_hourly 테이블에 누적 저장할 때
 * 이번 호출에서 "얼마나 증가했는지"를 계산합니다.
 *
 * $baseline 이 null이면 이전 기준 레코드가 없으므로 절댓값(첫 레코드)을 그대로 반환합니다.
 * $baseline 이 있으면 현재값 - 이전값(baseline) = 증분(delta) 을 계산합니다.
 * max(0, ...) 처리로 음수 델타 방지 (리셋 감지 전 경계 케이스 보호).
 *
 * 주의: 비율 지표(availabilty_rate, productivity_rate, quality_rate, oee)는
 *        누적하지 않고 항상 현재 절댓값을 사용합니다.
 *
 * @param array      $oee_metrics   calculateOeeMetrics() 결과 (현재 절댓값)
 * @param array|null $baseline      이전 기준 레코드 (null이면 첫 번째 레코드)
 * @param int        $runtime_sec   현재 총 경과 근무 시간 (초)
 * @param int        $actual_output 현재 총 실제 생산 수량
 * @return array 증분 데이터 배열
 */
function calculateDeltaMetrics(array $oee_metrics, ?array $baseline, int $runtime_sec, int $actual_output): array
{
    // 기준 레코드가 없으면(첫 번째 레코드) 절댓값 그대로 반환
    if (!$baseline) {
        return [
            'actual_output'      => $actual_output,
            'defective'          => $oee_metrics['defective'],
            'downtime'           => $oee_metrics['downtime'],
            'productive_runtime' => $oee_metrics['productive_runtime'],
            'theoritical_output' => $oee_metrics['theoritical_output'],
            'actual_a_grade'     => $oee_metrics['actual_a_grade'],
            'runtime'            => $runtime_sec,
            'availabilty_rate'   => $oee_metrics['availabilty_rate'],
            'productivity_rate'  => $oee_metrics['productivity_rate'],
            'quality_rate'       => $oee_metrics['quality_rate'],
            'oee'                => $oee_metrics['oee'],
        ];
    }
    // 생산량 증분: 현재 누적 - 기준 누적 (음수 방지)
    $d_out = max(0, $actual_output - $baseline['actual_output']);
    // 불량 증분: 현재 불량 수 - 기준 불량 수 (음수 방지)
    $d_def = max(0, $oee_metrics['defective'] - $baseline['defective']);
    return [
        'actual_output'      => $d_out,
        'defective'          => $d_def,
        'downtime'           => max(0, $oee_metrics['downtime'] - $baseline['downtime']),
        'productive_runtime' => max(0, $oee_metrics['productive_runtime'] - $baseline['productive_runtime']),
        'theoritical_output' => max(0, $oee_metrics['theoritical_output'] - $baseline['theoritical_output']),
        // 양품 증분: 생산량 증분 - 불량 증분 (파생값)
        'actual_a_grade'     => max(0, $d_out - $d_def),
        'runtime'            => max(0, $runtime_sec - $baseline['runtime']),
        // 비율 지표는 증분 없이 현재 절댓값 사용
        'availabilty_rate'   => $oee_metrics['availabilty_rate'],
        'productivity_rate'  => $oee_metrics['productivity_rate'],
        'quality_rate'       => $oee_metrics['quality_rate'],
        'oee'                => $oee_metrics['oee'],
    ];
}

/**
 * INSERT/UPDATE에 공통으로 쓰이는 식별자·메타 파라미터 배열 생성
 *
 * data_oee, data_oee_rows_hourly 두 테이블의 UPSERT 쿼리에서
 * 공통으로 사용하는 바인딩 파라미터를 한 번에 구성합니다.
 * 쿼리별 고유 파라미터(runtime, actual_output 등)는 호출 측에서 병합합니다.
 *
 * @param array  $b                 base context 배열 (work_date, shift_idx, factory/line/machine 정보)
 * @param string $time_update       현재 시각 (HH:MM:SS) — data_oee의 time_update 컬럼
 * @param int    $planned_work_time 계획 근무 시간 (초)
 * @param int    $work_hour         현재 시각의 시(0~23) — 시간대별 집계용
 * @param mixed  $pair_info         페어링 정보 (pi 파라미터)
 * @param mixed  $pair_count        페어링 카운트 (pc 파라미터)
 * @param array  $m                 calculateOeeMetrics() 결과 (목표 생산량 지표 포함)
 * @param string $today             현재 날짜+시각 (reg_date, update_date 기록용)
 * @return array PDO 바인딩 파라미터 배열
 */
function buildOeeBaseParams(array $b, string $time_update, int $planned_work_time, int $work_hour, $pair_info, $pair_count, array $m, string $today): array
{
    return [
        ':work_date' => $b['work_date'],
        ':shift_idx'    => $b['shift_idx'],
        ':factory_idx' => $b['factory_idx'],
        ':factory_name' => $b['factory_name'],
        ':line_idx'    => $b['line_idx'],
        ':line_name'    => $b['line_name'],
        ':mac'         => $b['mac'],
        ':machine_idx'  => $b['machine_idx'],
        ':machine_no'  => $b['machine_no'],
        ':process_name' => $b['process_name'],
        ':time_update'       => $time_update,
        ':planned_work_time' => $planned_work_time,
        ':work_hour'         => $work_hour,
        ':pair_info'         => $pair_info,
        ':pair_count'        => $pair_count,
        ':target_line_per_day'  => $m['target_line_per_day'],
        ':target_line_per_hour' => $m['target_line_per_hour'],
        ':target_mc_per_day'    => $m['target_mc_per_day'],
        ':target_mc_per_hour'   => $m['target_mc_per_hour'],
        ':cycletime'            => $m['cycletime'],
        ':reg_date'    => $today,
        ':update_date' => $today,
    ];
}

/**
 * OEE 테이블 UPSERT (data_oee / data_oee_rows_hourly 공용)
 *
 * 레코드 존재 여부($existing)와 리셋 감지 여부($is_reset)에 따라 3가지 동작을 수행합니다.
 *
 * [케이스 1] $existing=null (레코드 없음) → INSERT (새 레코드 삽입, 델타값 사용)
 * [케이스 2] $existing=있음, $is_reset=false → UPDATE 누적 (기존값 + 델타)
 *   - runtime, productive_runtime, downtime, theoritical_output, actual_output, defective 는 누적
 *   - actual_a_grade 는 파생값이므로 현재 절댓값으로 SET
 *   - 비율 지표(availabilty_rate, productivity_rate 등)는 현재 절댓값으로 SET
 * [케이스 3] $existing=있음, $is_reset=true → UPDATE 절댓값 덮어쓰기 (리셋 감지 시)
 *   - 모든 필드를 현재 절댓값으로 교체
 *
 * @param PDO        $pdo         데이터베이스 연결
 * @param string     $table       대상 테이블명 (data_oee 또는 data_oee_rows_hourly)
 * @param array|null $existing    기존 레코드 (null이면 INSERT)
 * @param bool       $is_reset    리셋 감지 여부 (true이면 절댓값으로 덮어쓰기)
 * @param array      $base_params buildOeeBaseParams() 결과 (공통 파라미터)
 * @param array      $delta       calculateDeltaMetrics() 결과 (증분 데이터)
 * @param array      $oee_metrics calculateOeeMetrics() 결과 (현재 절댓값)
 * @param int        $runtime_sec 현재 총 경과 근무 시간 (초)
 * @param int        $actual_output 현재 총 실제 생산 수량
 */
function upsertOeeRecord(PDO $pdo, string $table, ?array $existing, bool $is_reset, array $base_params, array $delta, array $oee_metrics, int $runtime_sec, int $actual_output): void
{
    $tbl = "`{$table}`";

    if ($existing) {
        if ($is_reset) {
            // ── 리셋 감지: 모든 컬럼을 현재 절댓값으로 덮어쓰기 ──────────────
            // 기기가 카운터를 리셋했으므로 누적값을 버리고 현재값으로 교체
            $stmt = $pdo->prepare(
                "UPDATE {$tbl} SET
          time_update = :time_update, planned_work_time = :planned_work_time,
          runtime = :runtime, productive_runtime = :productive_runtime, downtime = :downtime,
          availabilty_rate = :availabilty_rate,
          target_line_per_day = :target_line_per_day, target_line_per_hour = :target_line_per_hour,
          target_mc_per_day = :target_mc_per_day, target_mc_per_hour = :target_mc_per_hour,
          cycletime = :cycletime, pair_info = :pair_info, pair_count = :pair_count,
          theoritical_output = :theoritical_output, actual_output = :actual_output,
          productivity_rate = :productivity_rate, defective = :defective,
          actual_a_grade = :actual_a_grade, quality_rate = :quality_rate, oee = :oee,
          update_date = :update_date, work_hour = :work_hour
        WHERE idx = :idx"
            );
            $stmt->execute($base_params + [
                ':runtime'            => $runtime_sec,
                ':productive_runtime' => $oee_metrics['productive_runtime'],
                ':downtime'           => $oee_metrics['downtime'],
                ':availabilty_rate'   => $oee_metrics['availabilty_rate'],
                ':theoritical_output' => $oee_metrics['theoritical_output'],
                ':actual_output'      => $actual_output,
                ':productivity_rate'  => $oee_metrics['productivity_rate'],
                ':defective'          => $oee_metrics['defective'],
                ':actual_a_grade'     => $oee_metrics['actual_a_grade'],
                ':quality_rate'       => $oee_metrics['quality_rate'],
                ':oee'                => $oee_metrics['oee'],
                ':idx'                => $existing['idx'],
            ]);
        } else {
            // ── 정상 업데이트: 기존값에 증분(delta)을 더해서 누적 ─────────────
            // 주의: actual_a_grade는 파생값이므로 누적 절댓값으로 SET
            $stmt = $pdo->prepare(
                "UPDATE {$tbl} SET
          time_update = :time_update, planned_work_time = :planned_work_time,
          runtime = runtime + :runtime,
          productive_runtime = productive_runtime + :productive_runtime,
          downtime = downtime + :downtime,
          availabilty_rate = :availabilty_rate,
          target_line_per_day = :target_line_per_day, target_line_per_hour = :target_line_per_hour,
          target_mc_per_day = :target_mc_per_day, target_mc_per_hour = :target_mc_per_hour,
          cycletime = :cycletime, pair_info = :pair_info, pair_count = :pair_count,
          theoritical_output = theoritical_output + :theoritical_output,
          actual_output = actual_output + :actual_output,
          productivity_rate = :productivity_rate,
          defective = defective + :defective,
          actual_a_grade = :actual_a_grade,
          quality_rate = :quality_rate, oee = :oee,
          update_date = :update_date, work_hour = :work_hour
        WHERE idx = :idx"
            );
            $stmt->execute($base_params + [
                ':runtime'            => $delta['runtime'],
                ':productive_runtime' => $delta['productive_runtime'],
                ':downtime'           => $delta['downtime'],
                ':availabilty_rate'   => $delta['availabilty_rate'],
                ':theoritical_output' => $delta['theoritical_output'],
                ':actual_output'      => $delta['actual_output'],
                ':productivity_rate'  => $delta['productivity_rate'],
                ':defective'          => $delta['defective'],
                ':actual_a_grade'     => $oee_metrics['actual_a_grade'], // 파생값: 절댓값으로 SET
                ':quality_rate'       => $delta['quality_rate'],
                ':oee'                => $delta['oee'],
                ':idx'                => $existing['idx'],
            ]);
        }
    } else {
        // ── 신규 INSERT: 첫 번째 레코드 (델타 = 절댓값) ──────────────────
        $stmt = $pdo->prepare(
            "INSERT INTO {$tbl}
        (work_date, time_update, shift_idx, factory_idx, factory_name, line_idx, line_name,
         mac, machine_idx, machine_no, process_name, planned_work_time,
         runtime, productive_runtime, downtime, availabilty_rate,
         target_line_per_day, target_line_per_hour, target_mc_per_day, target_mc_per_hour,
         cycletime, pair_info, pair_count, theoritical_output, actual_output,
         productivity_rate, defective, actual_a_grade, quality_rate, oee,
         reg_date, update_date, work_hour)
       VALUES
        (:work_date, :time_update, :shift_idx, :factory_idx, :factory_name, :line_idx, :line_name,
         :mac, :machine_idx, :machine_no, :process_name, :planned_work_time,
         :runtime, :productive_runtime, :downtime, :availabilty_rate,
         :target_line_per_day, :target_line_per_hour, :target_mc_per_day, :target_mc_per_hour,
         :cycletime, :pair_info, :pair_count, :theoritical_output, :actual_output,
         :productivity_rate, :defective, :actual_a_grade, :quality_rate, :oee,
         :reg_date, :update_date, :work_hour)"
        );
        $stmt->execute($base_params + [
            ':runtime'            => $delta['runtime'],
            ':productive_runtime' => $delta['productive_runtime'],
            ':downtime'           => $delta['downtime'],
            ':availabilty_rate'   => $delta['availabilty_rate'],
            ':theoritical_output' => $delta['theoritical_output'],
            ':actual_output'      => $delta['actual_output'],
            ':productivity_rate'  => $delta['productivity_rate'],
            ':defective'          => $delta['defective'],
            ':actual_a_grade'     => $delta['actual_a_grade'],
            ':quality_rate'       => $delta['quality_rate'],
            ':oee'                => $delta['oee'],
        ]);
    }
}

// ─────────────────────────────────────────────────────────────────────
// 메인 처리
// ─────────────────────────────────────────────────────────────────────

// 현재 날짜·시각 관련 변수 초기화
$today       = date('Y-m-d H:i:s'); // 전체 날짜+시각 (DB 저장용)
$time_update = date('H:i:s');       // 시각만 (data_oee.time_update 컬럼용)
$work_hour   = date('H');           // 현재 시(0~23) — 시간대별 집계 키
$apiHelper   = new ApiHelper($pdo);

// ─────────────────────────────────────────────────────────────────────
// 요청 파라미터 수신
// - mac: 재봉기 MAC 주소 (기계 식별 키)
// - pi : pair_info  — 페어 생산 정보 (설정에 따라 사용)
// - pc : pair_count — 페어 카운트
// - sc : 실제 생산 카운트 (actual_output) — OEE 계산의 핵심 입력값
// ─────────────────────────────────────────────────────────────────────
// MAC 주소 검증 및 처리
$mac = $apiHelper->validateAndProcessMac($_REQUEST['mac'] ?? '');

// 요청 파라미터
$pair_info     = trim($_REQUEST['pi'] ?? 0);
$pair_count    = trim($_REQUEST['pc'] ?? 0);
$actual_output = (int)trim($_REQUEST['sc'] ?? 0); // 정수 변환: 소수점 카운트 방지

// ─────────────────────────────────────────────────────────────────────
// 기계 정보 조회 (확장 버전)
// 기본 info_machine 외에 공장명(factory_name), 라인명(line_name),
// 라인 목표 생산량(line_target)도 JOIN으로 함께 조회
// ─────────────────────────────────────────────────────────────────────
// 기계 정보 조회 (line_target, factory_name, line_name 포함)
$stmt = $pdo->prepare(
    "SELECT a.*, b.factory_name, c.line_name, c.line_target
   FROM info_machine AS a
   LEFT JOIN info_factory AS b ON b.idx = a.factory_idx
   LEFT JOIN info_line    AS c ON c.idx = a.line_idx
   WHERE a.mac = ?"
);
$stmt->execute([$mac]);
$machine_data = $stmt->fetch(PDO::FETCH_ASSOC);

if (!$machine_data) {
    jsonReturn(['code' => '99', 'msg' => 'Machine not found for MAC address']);
}

$factory_idx  = $machine_data['factory_idx'];
$factory_name = $machine_data['factory_name'];
$line_idx     = $machine_data['line_idx'];
$line_name    = $machine_data['line_name'];
$machine_idx  = $machine_data['idx'];
$machine_no   = $machine_data['machine_no'];
// 라인 목표 생산량: OEE 생산성·사이클타임 계산의 기준값
$target       = (int)($machine_data['line_target'] ?? 0);

// ─────────────────────────────────────────────────────────────────────
// INVENTORY 머신 체크 (line_idx = 99는 데이터 저장 안 함)
// ─────────────────────────────────────────────────────────────────────
// INVENTORY 머신 체크 (line_idx = 99는 데이터 저장 안 함)
if ($line_idx == 99) {
    jsonReturn(['code' => '00', 'msg' => 'Skipped: Machine in INVENTORY (line_idx=99)']);
}

// ─────────────────────────────────────────────────────────────────────
// 디자인 공정 정보 조회
// info_design_process 테이블: 기계에 배정된 봉제 공정 정보
// - std_mc_needed: 해당 공정에 필요한 표준 기계 대수 (OEE 속도 지표 계산용)
// - design_process: 공정 이름 (process_name 으로 저장)
// design_process_idx = 0 이면 공정 미설정 → 데이터 저장 불가
// ─────────────────────────────────────────────────────────────────────
// 디자인 공정 확인
$design_process_idx = $machine_data['design_process_idx'] ?? 0;
if ($design_process_idx == 0) {
    jsonReturn(['code' => '99', 'msg' => 'Design process not configured for this machine']);
}

$stmt = $pdo->prepare("SELECT std_mc_needed, design_process FROM info_design_process WHERE idx = ? AND status = 'Y'");
$stmt->execute([$design_process_idx]);
$design_process_data = $stmt->fetch(PDO::FETCH_ASSOC);

if (!$design_process_data) {
    jsonReturn(['code' => '99', 'msg' => 'Design process not found or inactive']);
}

$process_name  = $design_process_data['design_process'];          // 공정명 (data_oee 집계 키)
$std_mc_needed = (int)($design_process_data['std_mc_needed'] ?? 1); // 표준 기계 대수 (0이면 1로 처리)

// ─────────────────────────────────────────────────────────────────────
// 현재 시프트 정보 조회
// 근무 시간 외에는 데이터 저장 불필요 → null이면 오류 응답
// ─────────────────────────────────────────────────────────────────────
// 현재 시프트 정보 조회
$current_shift_info = $apiHelper->getCurrentShiftInfo($factory_idx, $line_idx, $today);
if (!$current_shift_info) {
    jsonReturn(['code' => '99', 'msg' => 'Not during working hours']);
}

// ─────────────────────────────────────────────────────────────────────
// 작업 시간 지표 계산 (Worktime 클래스 활용)
// - net_work_minutes      : 순 근무 시간 (분) — 휴식 시간 제외
// - actual_passed_work_seconds: 현재 시프트 시작부터 경과한 실제 근무 시간 (초)
//   → runtime_sec 으로 사용 (가용성·생산성 계산의 분모)
// ─────────────────────────────────────────────────────────────────────
// 작업 시간 지표 계산
$worktime         = new Worktime($pdo);
$all_shifts       = $worktime->getDayShift($current_shift_info['date'], $factory_idx, $line_idx);
$worktime_metrics = calculateWorktimeMetrics($current_shift_info, $all_shifts, $today);

$work_date         = $current_shift_info['date'];
$shift_idx         = $current_shift_info['shift_idx'];
$planned_work_time = $worktime_metrics['net_work_minutes'] * 60;         // 초 단위로 변환
$planned_wt_hour   = ($worktime_metrics['net_work_minutes'] > 0) ? ($worktime_metrics['net_work_minutes'] / 60) : 1;
$runtime_sec       = $worktime_metrics['actual_passed_work_seconds'];    // 실제 경과 근무 초

// ─────────────────────────────────────────────────────────────────────
// 리셋 감지
// data_oee_rows 에서 이 기계·날짜·시프트·공정의 가장 최근 생산량을 조회
// 현재 actual_output 이 이전 값보다 10 이상 작으면 → 기기 카운터 리셋으로 판단
// is_reset=true 이면 upsertOeeRecord 에서 누적이 아닌 덮어쓰기 처리
// ─────────────────────────────────────────────────────────────────────
// 리셋 감지
$stmt = $pdo->prepare("SELECT actual_output FROM data_oee_rows WHERE mac = ? AND work_date = ? AND shift_idx = ? AND process_name = ? ORDER BY idx DESC LIMIT 1");
$stmt->execute([$mac, $work_date, $shift_idx, $process_name]);
$latest_row = $stmt->fetch(PDO::FETCH_ASSOC);
$is_reset   = $latest_row && ($actual_output < ($latest_row['actual_output'] - 10));

// ─────────────────────────────────────────────────────────────────────
// OEE 지표 계산 (현재 절댓값 기준)
// ─────────────────────────────────────────────────────────────────────
// OEE 지표 계산
$oee_metrics = calculateOeeMetrics($pdo, $mac, $work_date, $shift_idx, $runtime_sec, $planned_wt_hour, $std_mc_needed, $target, $actual_output);

// ─────────────────────────────────────────────────────────────────────
// UPSERT 공통 파라미터 구성
// 기계·라인·공장 메타데이터와 OEE 목표 지표를 하나의 배열로 묶음
// ─────────────────────────────────────────────────────────────────────
// 기준 파라미터 배열 (INSERT/UPDATE 공용)
$base_ctx = [
    'work_date' => $work_date,
    'shift_idx' => $shift_idx,
    'factory_idx' => $factory_idx,
    'factory_name' => $factory_name,
    'line_idx' => $line_idx,
    'line_name' => $line_name,
    'mac' => $mac,
    'machine_idx' => $machine_idx,
    'machine_no' => $machine_no,
    'process_name' => $process_name,
];
$base_params = buildOeeBaseParams($base_ctx, $time_update, $planned_work_time, $work_hour, $pair_info, $pair_count, $oee_metrics, $today);

// ─────────────────────────────────────────────────────────────────────
// ── data_oee UPSERT ──────────────────────────────────────────────────
// data_oee: 시프트 단위 OEE 집계 테이블 (기계·날짜·시프트·공정당 1행)
//
// UPSERT 전략:
// 1. 기존 레코드 조회 (ORDER BY idx ASC LIMIT 1 — 첫 번째 레코드를 기준으로 업데이트)
// 2. 없으면 → 이전 공정 레코드를 baseline으로 사용해 delta 계산 후 INSERT
//    (동일 기계·날짜·시프트에서 공정이 바뀐 경우 연속성 유지)
// 3. 있으면 → 기존 레코드에 delta 누적 (또는 리셋 시 절댓값 덮어쓰기)
// ─────────────────────────────────────────────────────────────────────
// ── data_oee UPSERT ──────────────────────────────────────────────────
$stmt = $pdo->prepare("SELECT * FROM data_oee WHERE mac = ? AND work_date = ? AND shift_idx = ? AND process_name = ? ORDER BY idx ASC LIMIT 1");
$stmt->execute([$mac, $work_date, $shift_idx, $process_name]);
$oee_data = $stmt->fetch(PDO::FETCH_ASSOC);

if (!$oee_data) {
    // 이전 공정 데이터 조회 (INSERT 시 델타 계산용)
    // 같은 기계·날짜·시프트에서 공정명이 다른 가장 최근 레코드를 baseline으로 사용
    $stmt = $pdo->prepare("SELECT * FROM data_oee WHERE mac = ? AND work_date = ? AND shift_idx = ? AND process_name != ? ORDER BY idx DESC LIMIT 1");
    $stmt->execute([$mac, $work_date, $shift_idx, $process_name]);
    $prev_process_data = $stmt->fetch(PDO::FETCH_ASSOC) ?: null;
} else {
    $prev_process_data = null; // 기존 레코드가 있으면 이전 공정 데이터 불필요
}

// 델타 계산을 위한 baseline 결정: 현재 공정 레코드 → 이전 공정 레코드 → null(첫 레코드)
$process_baseline = $oee_data ?? $prev_process_data;
$process_delta    = calculateDeltaMetrics($oee_metrics, $process_baseline, $runtime_sec, $actual_output);

upsertOeeRecord($pdo, 'data_oee', $oee_data, $is_reset, $base_params, $process_delta, $oee_metrics, $runtime_sec, $actual_output);

// 응답 메시지: INSERT/UPDATE/리셋 여부에 따라 다른 메시지 반환
$response = ['code' => '00', 'msg' => $oee_data
    ? ($is_reset ? 'OEE data updated successfully (reset detected)' : 'OEE data updated successfully')
    : 'OEE data inserted successfully'];

// ─────────────────────────────────────────────────────────────────────
// ── data_oee_rows INSERT (항상 삽입, 누적 감사 로그) ─────────────────
// data_oee_rows: 매 API 호출마다 레코드를 추가하는 상세 이력 테이블
// - 업데이트가 아닌 항상 INSERT → 시간대별 생산량 변화 추적 가능
// - update_date 컬럼이 없으므로 base_params에서 제거 후 삽입
// - 절댓값 저장 (delta 아님): 각 시점의 누적 상태를 스냅샷으로 보존
// ─────────────────────────────────────────────────────────────────────
// ── data_oee_rows INSERT (항상 삽입, 누적 감사 로그) ────
$stmt = $pdo->prepare(
    "INSERT INTO `data_oee_rows`
    (work_date, time_update, shift_idx, factory_idx, factory_name, line_idx, line_name,
     mac, machine_idx, machine_no, process_name, planned_work_time,
     runtime, productive_runtime, downtime, availabilty_rate,
     target_line_per_day, target_line_per_hour, target_mc_per_day, target_mc_per_hour,
     cycletime, pair_info, pair_count, theoritical_output, actual_output,
     productivity_rate, defective, actual_a_grade, quality_rate, oee, reg_date, work_hour)
   VALUES
    (:work_date, :time_update, :shift_idx, :factory_idx, :factory_name, :line_idx, :line_name,
     :mac, :machine_idx, :machine_no, :process_name, :planned_work_time,
     :runtime, :productive_runtime, :downtime, :availabilty_rate,
     :target_line_per_day, :target_line_per_hour, :target_mc_per_day, :target_mc_per_hour,
     :cycletime, :pair_info, :pair_count, :theoritical_output, :actual_output,
     :productivity_rate, :defective, :actual_a_grade, :quality_rate, :oee, :reg_date, :work_hour)"
);
$rows_params = $base_params + [
    ':runtime'            => $runtime_sec,
    ':productive_runtime' => $oee_metrics['productive_runtime'],
    ':downtime'           => $oee_metrics['downtime'],
    ':availabilty_rate'   => $oee_metrics['availabilty_rate'],
    ':theoritical_output' => $oee_metrics['theoritical_output'],
    ':actual_output'      => $actual_output,
    ':productivity_rate'  => $oee_metrics['productivity_rate'],
    ':defective'          => $oee_metrics['defective'],
    ':actual_a_grade'     => $oee_metrics['actual_a_grade'],
    ':quality_rate'       => $oee_metrics['quality_rate'],
    ':oee'                => $oee_metrics['oee'],
];
unset($rows_params[':update_date']); // data_oee_rows 테이블에는 update_date 컬럼 없음
$stmt->execute($rows_params);

// ─────────────────────────────────────────────────────────────────────
// ── data_oee_rows_hourly UPSERT ──────────────────────────────────────
// data_oee_rows_hourly: 시간대별(work_hour) OEE 집계 테이블
// 기계·날짜·시프트·공정·시간대(work_hour)당 1행 유지
//
// UPSERT 전략:
// 1. 현재 work_hour 레코드 조회
// 2. 없으면 → 이전 시간대(work_hour < 현재) 레코드를 baseline으로 delta 계산 후 INSERT
//    (시간대가 바뀌었을 때 이전 시간대 누적값을 기준으로 이번 시간대 증분 계산)
// 3. 있으면 → 기존 시간대 레코드에 delta 누적 (또는 리셋 시 절댓값 덮어쓰기)
// ─────────────────────────────────────────────────────────────────────
// ── data_oee_rows_hourly UPSERT ──────────────────────────────────────
$stmt = $pdo->prepare("SELECT * FROM `data_oee_rows_hourly` WHERE work_date = ? AND shift_idx = ? AND mac = ? AND process_name = ? AND work_hour = ?");
$stmt->execute([$work_date, $shift_idx, $mac, $process_name, $work_hour]);
$hourly_data = $stmt->fetch(PDO::FETCH_ASSOC);

if (!$hourly_data) {
    // 이전 시간 데이터 조회 (INSERT 시 델타 계산용)
    // work_hour < 현재 시간대 중 가장 최근 레코드를 baseline으로 사용
    $stmt = $pdo->prepare("SELECT * FROM `data_oee_rows_hourly` WHERE work_date = ? AND shift_idx = ? AND mac = ? AND process_name = ? AND work_hour < ? ORDER BY work_hour DESC LIMIT 1");
    $stmt->execute([$work_date, $shift_idx, $mac, $process_name, $work_hour]);
    $prev_hourly_data = $stmt->fetch(PDO::FETCH_ASSOC) ?: null;
} else {
    $prev_hourly_data = null; // 현재 시간대 레코드가 있으면 이전 시간대 데이터 불필요
}

// 시간대별 delta 계산 (현재 시간대 baseline 결정)
$hourly_baseline = $hourly_data ?? $prev_hourly_data;
$hourly_delta    = calculateDeltaMetrics($oee_metrics, $hourly_baseline, $runtime_sec, $actual_output);

upsertOeeRecord($pdo, 'data_oee_rows_hourly', $hourly_data, $is_reset, $base_params, $hourly_delta, $oee_metrics, $runtime_sec, $actual_output);

// ─────────────────────────────────────────────────────────────────────
// API 호출 로그 저장 및 응답 반환
// ─────────────────────────────────────────────────────────────────────
// API 호출 로깅 및 응답 반환
$apiHelper->logApiCall('logs_api_send_pCount', 'send_pCount', $machine_no, $mac, $_REQUEST, $response, $today);
jsonReturn($response);

/*
## API 엔드포인트
send_pCount.php

## 요청 예시
http://49.247.26.228/OEE_SCI/OEE_SCI_V2/api/sewing.php?code=send_pCount&mac=84:72:07:50:37:73&design_no=A001&sc=100&pi=1&pc=50

## 응답 예시
{
  "code": "00",
  "msg": "OEE data updated successfully"
}
*/
