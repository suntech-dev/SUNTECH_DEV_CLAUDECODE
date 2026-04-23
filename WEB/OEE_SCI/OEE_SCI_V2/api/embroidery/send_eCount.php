<?php
// 자수기 OEE 시스템 — 생산 카운트 API
//
// 자수기(EMBROIDERY_S)에서 패킷 단위로 수신하는 생산 데이터를 처리합니다.
// 재봉기(send_pCount)와의 핵심 차이점:
//   - actual_qty : 이번 패킷의 완료 수량 (누적값 아님) → 서버가 DB에 누적
//   - ct         : 사이클타임 (초, 정수)
//   - tb         : 실끊김 횟수 → thread_breakage 로 별도 누적 (defective 미포함)
//   - mrt        : 모터동작시간 (초, 정수) → motor_run_time 으로 별도 누적
//   - pair_info / pair_count : 자수기 미사용 → 0 고정
//   - 리셋 감지 : 불필요 (기계는 항상 패킷 수량만 전송)
//
// OEE 계산: 재봉기와 동일 공정 설정(info_design_process) 사용

require_once(__DIR__ . '/../../lib/api_helper.lib.php');

// ─────────────────────────────────────────────────────────────────────
// 헬퍼 함수
// ─────────────────────────────────────────────────────────────────────

/**
 * 생산 데이터 기반 OEE 지표 계산 (재봉기와 동일 로직)
 */
function calculateOeeMetrics(PDO $pdo, string $mac, string $work_date, int $shift_idx, int $runtime_sec, float $planned_work_time_hour, int $std_mc_needed, int $target, int $actual_output): array
{
    $stmt = $pdo->prepare(
        "SELECT
      (SELECT SUM(duration_sec) FROM data_downtime WHERE mac = :mac AND work_date = :work_date AND shift_idx = :shift_idx) as downtime_duration_sum,
      (SELECT COUNT(*) FROM data_defective WHERE mac = :mac2 AND work_date = :work_date2 AND shift_idx = :shift_idx2) as defective_count"
    );
    $stmt->execute([
        ':mac'        => $mac,        ':work_date'        => $work_date,        ':shift_idx'        => $shift_idx,
        ':mac2'       => $mac,        ':work_date2'       => $work_date,        ':shift_idx2'       => $shift_idx,
    ]);
    $result = $stmt->fetch(PDO::FETCH_ASSOC);

    $m = [];
    $m['downtime']  = $result['downtime_duration_sum'] ?? 0;
    $m['defective'] = $result['defective_count'] ?? 0;

    $productive = max(0, $runtime_sec - $m['downtime']);
    $m['productive_runtime'] = $productive;
    $m['availabilty_rate']   = $runtime_sec > 0 ? min(round(($productive / $runtime_sec) * 100, 1), 100) : 0;

    $tpd = $target;
    $tph = ($planned_work_time_hour > 0) ? ($tpd / $planned_work_time_hour) : 0;
    $m['target_line_per_day']  = $tpd;
    $m['target_line_per_hour'] = $tph;
    $m['target_mc_per_day']    = ($std_mc_needed > 0) ? ($tpd / $std_mc_needed) : 0;
    $m['target_mc_per_hour']   = ($std_mc_needed > 0) ? ($tph / $std_mc_needed) : 0;
    $m['cycletime']            = ($m['target_mc_per_hour'] > 0) ? (3600 / $m['target_mc_per_hour']) : 0;
    $m['theoritical_output']   = ($m['cycletime'] > 0) ? ($productive / $m['cycletime']) : 0;

    $m['productivity_rate'] = ($m['theoritical_output'] > 0)
        ? min(round(($actual_output / $m['theoritical_output']) * 100, 1), 200) : 0;

    $m['actual_a_grade'] = max(0, $actual_output - $m['defective']);
    $m['quality_rate']   = ($actual_output > 0)
        ? min(round(($m['actual_a_grade'] / $actual_output) * 100, 1), 100) : 0;

    $m['oee'] = round(($m['availabilty_rate'] * $m['productivity_rate'] * $m['quality_rate']) / 10000, 2);

    return $m;
}

/**
 * 자수기 OEE 테이블 UPSERT
 *
 * 재봉기(upsertOeeRecord)와 달리:
 * - 리셋 감지 없음 (서버가 누적 관리)
 * - thread_breakage, motor_run_time 컬럼 추가 누적
 * - actual_output 은 호출마다 packet_qty 만큼 증가
 *
 * @param PDO        $pdo
 * @param string     $table         대상 테이블명
 * @param array|null $existing      기존 레코드
 * @param array      $base_params   buildOeeBaseParams() 결과
 * @param array      $oee_metrics   현재 OEE 절댓값
 * @param int        $runtime_sec   현재 경과 근무 시간 (초)
 * @param int        $actual_output 누적 생산량 (이번 패킷 포함)
 * @param int        $packet_qty    이번 패킷의 완료 수량
 * @param int        $thread_break  이번 패킷의 실끊김 횟수
 * @param int        $motor_run     이번 패킷의 모터동작시간 (초)
 */
function upsertEmbOeeRecord(
    PDO $pdo, string $table, ?array $existing,
    array $base_params, array $oee_metrics,
    int $runtime_sec, int $actual_output,
    int $packet_qty, int $thread_break, int $motor_run
): void {
    $tbl = "`{$table}`";

    if ($existing) {
        // ── 정상 누적 업데이트 ────────────────────────────────────────
        $update_base = array_intersect_key($base_params, array_flip([
            ':time_update', ':planned_work_time', ':work_hour', ':pair_info', ':pair_count',
            ':target_line_per_day', ':target_line_per_hour', ':target_mc_per_day', ':target_mc_per_hour',
            ':cycletime', ':update_date',
        ]));

        $stmt = $pdo->prepare(
            "UPDATE {$tbl} SET
          time_update = :time_update, planned_work_time = :planned_work_time,
          runtime = runtime + :runtime_delta,
          productive_runtime = productive_runtime + :productive_delta,
          downtime = :downtime,
          availabilty_rate = :availabilty_rate,
          target_line_per_day = :target_line_per_day, target_line_per_hour = :target_line_per_hour,
          target_mc_per_day = :target_mc_per_day, target_mc_per_hour = :target_mc_per_hour,
          cycletime = :cycletime,
          pair_info = :pair_info, pair_count = :pair_count,
          theoritical_output = :theoritical_output,
          actual_output = actual_output + :packet_qty,
          productivity_rate = :productivity_rate,
          defective = :defective,
          actual_a_grade = :actual_a_grade,
          quality_rate = :quality_rate, oee = :oee,
          thread_breakage = thread_breakage + :thread_break,
          motor_run_time = motor_run_time + :motor_run,
          update_date = :update_date, work_hour = :work_hour
        WHERE idx = :idx"
        );
        $stmt->execute($update_base + [
            ':runtime_delta'      => max(0, $runtime_sec - (int)($existing['runtime'] ?? 0)),
            ':productive_delta'   => max(0, $oee_metrics['productive_runtime'] - (int)($existing['productive_runtime'] ?? 0)),
            ':downtime'           => $oee_metrics['downtime'],
            ':availabilty_rate'   => $oee_metrics['availabilty_rate'],
            ':theoritical_output' => $oee_metrics['theoritical_output'],
            ':packet_qty'         => $packet_qty,
            ':productivity_rate'  => $oee_metrics['productivity_rate'],
            ':defective'          => $oee_metrics['defective'],
            ':actual_a_grade'     => $oee_metrics['actual_a_grade'],
            ':quality_rate'       => $oee_metrics['quality_rate'],
            ':oee'                => $oee_metrics['oee'],
            ':thread_break'       => $thread_break,
            ':motor_run'          => $motor_run,
            ':idx'                => $existing['idx'],
        ]);
    } else {
        // ── 첫 번째 레코드 INSERT ─────────────────────────────────────
        $stmt = $pdo->prepare(
            "INSERT INTO {$tbl}
        (work_date, time_update, shift_idx, factory_idx, factory_name, line_idx, line_name,
         mac, machine_idx, machine_no, process_name, planned_work_time,
         runtime, productive_runtime, downtime, availabilty_rate,
         target_line_per_day, target_line_per_hour, target_mc_per_day, target_mc_per_hour,
         cycletime, pair_info, pair_count, theoritical_output, actual_output,
         productivity_rate, defective, actual_a_grade, quality_rate, oee,
         thread_breakage, motor_run_time,
         reg_date, update_date, work_hour)
       VALUES
        (:work_date, :time_update, :shift_idx, :factory_idx, :factory_name, :line_idx, :line_name,
         :mac, :machine_idx, :machine_no, :process_name, :planned_work_time,
         :runtime, :productive_runtime, :downtime, :availabilty_rate,
         :target_line_per_day, :target_line_per_hour, :target_mc_per_day, :target_mc_per_hour,
         :cycletime, :pair_info, :pair_count, :theoritical_output, :actual_output,
         :productivity_rate, :defective, :actual_a_grade, :quality_rate, :oee,
         :thread_break, :motor_run,
         :reg_date, :update_date, :work_hour)"
        );
        $stmt->execute($base_params + [
            ':runtime'            => $runtime_sec,
            ':productive_runtime' => $oee_metrics['productive_runtime'],
            ':downtime'           => $oee_metrics['downtime'],
            ':availabilty_rate'   => $oee_metrics['availabilty_rate'],
            ':theoritical_output' => $oee_metrics['theoritical_output'],
            ':actual_output'      => $packet_qty,
            ':productivity_rate'  => $oee_metrics['productivity_rate'],
            ':defective'          => $oee_metrics['defective'],
            ':actual_a_grade'     => $oee_metrics['actual_a_grade'],
            ':quality_rate'       => $oee_metrics['quality_rate'],
            ':oee'                => $oee_metrics['oee'],
            ':thread_break'       => $thread_break,
            ':motor_run'          => $motor_run,
        ]);
    }
}

/**
 * 자수기 전용 EMB 테이블 UPSERT (OEE 계산 없음)
 *
 * data_oee_emb / data_oee_rows_hourly_emb 에서 공용으로 사용.
 * - cycle_time  : 마지막 수신 CT 값으로 덮어씀 (누적 아님)
 * - thread_breakage / motor_run_time : 누적(+)
 * - actual_output : 누적(+packet_qty)
 * - runtime : 기존 대비 delta 만큼 누적
 */
function upsertEmbPureRecord(
    PDO $pdo, string $table, ?array $existing,
    array $base_ctx,
    int $runtime_sec, int $planned_work_time,
    int $packet_qty, int $cycle_time, int $thread_break, int $motor_run,
    string $today, string $time_update, int $work_hour
): void {
    $tbl = "`{$table}`";

    if ($existing) {
        $stmt = $pdo->prepare(
            "UPDATE {$tbl} SET
              time_update       = :time_update,
              planned_work_time = :planned_work_time,
              runtime           = runtime + :runtime_delta,
              actual_output     = actual_output + :packet_qty,
              cycle_time        = :cycle_time,
              thread_breakage   = thread_breakage + :thread_break,
              motor_run_time    = motor_run_time + :motor_run,
              work_hour         = :work_hour,
              update_date       = :update_date
             WHERE idx = :idx"
        );
        $stmt->execute([
            ':time_update'       => $time_update,
            ':planned_work_time' => $planned_work_time,
            ':runtime_delta'     => max(0, $runtime_sec - (int)($existing['runtime'] ?? 0)),
            ':packet_qty'        => $packet_qty,
            ':cycle_time'        => $cycle_time,
            ':thread_break'      => $thread_break,
            ':motor_run'         => $motor_run,
            ':work_hour'         => $work_hour,
            ':update_date'       => $today,
            ':idx'               => $existing['idx'],
        ]);
    } else {
        $stmt = $pdo->prepare(
            "INSERT INTO {$tbl}
             (work_date, time_update, shift_idx, factory_idx, factory_name, line_idx, line_name,
              mac, machine_idx, machine_no, process_name,
              planned_work_time, runtime, actual_output,
              cycle_time, thread_breakage, motor_run_time,
              pair_info, pair_count, work_hour, reg_date, update_date)
             VALUES
             (:work_date, :time_update, :shift_idx, :factory_idx, :factory_name, :line_idx, :line_name,
              :mac, :machine_idx, :machine_no, :process_name,
              :planned_work_time, :runtime, :actual_output,
              :cycle_time, :thread_break, :motor_run,
              0, 0, :work_hour, :reg_date, :update_date)"
        );
        $stmt->execute([
            ':work_date'         => $base_ctx['work_date'],
            ':time_update'       => $time_update,
            ':shift_idx'         => $base_ctx['shift_idx'],
            ':factory_idx'       => $base_ctx['factory_idx'],
            ':factory_name'      => $base_ctx['factory_name'],
            ':line_idx'          => $base_ctx['line_idx'],
            ':line_name'         => $base_ctx['line_name'],
            ':mac'               => $base_ctx['mac'],
            ':machine_idx'       => $base_ctx['machine_idx'],
            ':machine_no'        => $base_ctx['machine_no'],
            ':process_name'      => $base_ctx['process_name'],
            ':planned_work_time' => $planned_work_time,
            ':runtime'           => $runtime_sec,
            ':actual_output'     => $packet_qty,
            ':cycle_time'        => $cycle_time,
            ':thread_break'      => $thread_break,
            ':motor_run'         => $motor_run,
            ':work_hour'         => $work_hour,
            ':reg_date'          => $today,
            ':update_date'       => $today,
        ]);
    }
}

// ─────────────────────────────────────────────────────────────────────
// 메인 처리
// ─────────────────────────────────────────────────────────────────────
$today       = date('Y-m-d H:i:s');
$time_update = date('H:i:s');
$work_hour   = (int)date('H');
$apiHelper   = new ApiHelper($pdo);

// ─────────────────────────────────────────────────────────────────────
// 요청 파라미터 수신
// - mac        : 자수기 MAC 주소
// - actual_qty : 이번 패킷 완료 수량 (정수)
// - ct         : 사이클타임 (초, 정수)
// - tb         : 실끊김 횟수 (정수)
// - mrt        : 모터동작시간 (초, 정수)
// ─────────────────────────────────────────────────────────────────────
$mac        = $apiHelper->validateAndProcessMac($_REQUEST['mac'] ?? '');
$packet_qty = max(0, (int)trim($_REQUEST['actual_qty'] ?? 0));
$cycle_time = max(0, (int)trim($_REQUEST['ct']         ?? 0)); // 참고용 (현재 OEE 계산에 미사용)
$thread_break = max(0, (int)trim($_REQUEST['tb']       ?? 0));
$motor_run    = max(0, (int)trim($_REQUEST['mrt']      ?? 0));

// ─────────────────────────────────────────────────────────────────────
// 기계 정보 조회 (type='E' 검증 포함)
// ─────────────────────────────────────────────────────────────────────
$stmt = $pdo->prepare(
    "SELECT a.*, b.factory_name, c.line_name, c.line_target
     FROM info_machine AS a
     LEFT JOIN info_factory AS b ON b.idx = a.factory_idx
     LEFT JOIN info_line    AS c ON c.idx = a.line_idx
     WHERE a.mac = ? AND a.type = 'E'"
);
$stmt->execute([$mac]);
$machine_data = $stmt->fetch(PDO::FETCH_ASSOC);

if (!$machine_data) {
    jsonReturn(['code' => '99', 'msg' => 'Embroidery machine not found for MAC address']);
}

$factory_idx  = $machine_data['factory_idx'];
$factory_name = $machine_data['factory_name'];
$line_idx     = $machine_data['line_idx'];
$line_name    = $machine_data['line_name'];
$machine_idx  = $machine_data['idx'];
$machine_no   = $machine_data['machine_no'];
$target       = (int)($machine_data['line_target'] ?? 0);

// ─────────────────────────────────────────────────────────────────────
// INVENTORY 머신 체크
// ─────────────────────────────────────────────────────────────────────
if ($line_idx == 99) {
    jsonReturn(['code' => '00', 'msg' => 'Skipped: Embroidery machine in INVENTORY (line_idx=99)']);
}

// ─────────────────────────────────────────────────────────────────────
// 디자인 공정 확인 (재봉기와 동일 설정 공유)
// ─────────────────────────────────────────────────────────────────────
$design_process_idx = $machine_data['design_process_idx'] ?? 0;
if ($design_process_idx == 0) {
    jsonReturn(['code' => '99', 'msg' => 'Design process not configured for this embroidery machine']);
}

$stmt = $pdo->prepare("SELECT std_mc_needed, design_process FROM info_design_process WHERE idx = ? AND status = 'Y'");
$stmt->execute([$design_process_idx]);
$design_process_data = $stmt->fetch(PDO::FETCH_ASSOC);

if (!$design_process_data) {
    jsonReturn(['code' => '99', 'msg' => 'Design process not found or inactive']);
}

$process_name  = $design_process_data['design_process'];
$std_mc_needed = (int)($design_process_data['std_mc_needed'] ?? 1);

// ─────────────────────────────────────────────────────────────────────
// 현재 시프트 정보 조회
// ─────────────────────────────────────────────────────────────────────
$current_shift_info = $apiHelper->getCurrentShiftInfo($factory_idx, $line_idx, $today);
if (!$current_shift_info) {
    jsonReturn(['code' => '99', 'msg' => 'Not during working hours']);
}

// ─────────────────────────────────────────────────────────────────────
// 작업 시간 지표 계산
// ─────────────────────────────────────────────────────────────────────
$worktime         = new Worktime($pdo);
$all_shifts       = $worktime->getDayShift($current_shift_info['date'], $factory_idx, $line_idx);
$worktime_metrics = calculateWorktimeMetrics($current_shift_info, $all_shifts, $today);

$work_date         = $current_shift_info['date'];
$shift_idx         = $current_shift_info['shift_idx'];
$planned_work_time = $worktime_metrics['net_work_minutes'] * 60;
$planned_wt_hour   = ($worktime_metrics['net_work_minutes'] > 0) ? ($worktime_metrics['net_work_minutes'] / 60) : 1;
$runtime_sec       = $worktime_metrics['actual_passed_work_seconds'];

// ─────────────────────────────────────────────────────────────────────
// 현재 누적 actual_output 조회 (서버가 누적 관리)
// data_oee 의 기존 actual_output + 이번 패킷 수량 = 새 누적 total
// ─────────────────────────────────────────────────────────────────────
$stmt = $pdo->prepare(
    "SELECT * FROM data_oee WHERE mac = ? AND work_date = ? AND shift_idx = ? AND process_name = ? ORDER BY idx ASC LIMIT 1"
);
$stmt->execute([$mac, $work_date, $shift_idx, $process_name]);
$oee_data = $stmt->fetch(PDO::FETCH_ASSOC) ?: null;

$prev_actual_output = $oee_data ? (int)$oee_data['actual_output'] : 0;
$actual_output      = $prev_actual_output + $packet_qty; // 새 누적 total

// ─────────────────────────────────────────────────────────────────────
// OEE 지표 계산 (누적 actual_output 기준)
// ─────────────────────────────────────────────────────────────────────
$oee_metrics = calculateOeeMetrics(
    $pdo, $mac, $work_date, $shift_idx,
    $runtime_sec, $planned_wt_hour, $std_mc_needed, $target, $actual_output
);

// ─────────────────────────────────────────────────────────────────────
// 공통 파라미터 구성 (pair는 0 고정)
// ─────────────────────────────────────────────────────────────────────
$base_ctx = [
    'work_date'    => $work_date,
    'shift_idx'    => $shift_idx,
    'factory_idx'  => $factory_idx,
    'factory_name' => $factory_name,
    'line_idx'     => $line_idx,
    'line_name'    => $line_name,
    'mac'          => $mac,
    'machine_idx'  => $machine_idx,
    'machine_no'   => $machine_no,
    'process_name' => $process_name,
];

$base_params = [
    ':work_date'            => $work_date,
    ':shift_idx'            => $shift_idx,
    ':factory_idx'          => $factory_idx,
    ':factory_name'         => $factory_name,
    ':line_idx'             => $line_idx,
    ':line_name'            => $line_name,
    ':mac'                  => $mac,
    ':machine_idx'          => $machine_idx,
    ':machine_no'           => $machine_no,
    ':process_name'         => $process_name,
    ':time_update'          => $time_update,
    ':planned_work_time'    => $planned_work_time,
    ':work_hour'            => $work_hour,
    ':pair_info'            => 0,   // 자수기 미사용
    ':pair_count'           => 0,   // 자수기 미사용
    ':target_line_per_day'  => $oee_metrics['target_line_per_day'],
    ':target_line_per_hour' => $oee_metrics['target_line_per_hour'],
    ':target_mc_per_day'    => $oee_metrics['target_mc_per_day'],
    ':target_mc_per_hour'   => $oee_metrics['target_mc_per_hour'],
    ':cycletime'            => $oee_metrics['cycletime'],
    ':reg_date'             => $today,
    ':update_date'          => $today,
];

// ─────────────────────────────────────────────────────────────────────
// ── data_oee UPSERT ──────────────────────────────────────────────────
// ─────────────────────────────────────────────────────────────────────
upsertEmbOeeRecord(
    $pdo, 'data_oee', $oee_data,
    $base_params, $oee_metrics,
    $runtime_sec, $actual_output,
    $packet_qty, $thread_break, $motor_run
);

$response = ['code' => '00', 'msg' => $oee_data
    ? 'Embroidery OEE data updated successfully'
    : 'Embroidery OEE data inserted successfully'];

// ─────────────────────────────────────────────────────────────────────
// ── data_oee_rows INSERT (항상 삽입 — 스냅샷 이력)
// ─────────────────────────────────────────────────────────────────────
$stmt = $pdo->prepare(
    "INSERT INTO `data_oee_rows`
    (work_date, time_update, shift_idx, factory_idx, factory_name, line_idx, line_name,
     mac, machine_idx, machine_no, process_name, planned_work_time,
     runtime, productive_runtime, downtime, availabilty_rate,
     target_line_per_day, target_line_per_hour, target_mc_per_day, target_mc_per_hour,
     cycletime, pair_info, pair_count, theoritical_output, actual_output,
     productivity_rate, defective, actual_a_grade, quality_rate, oee,
     thread_breakage, motor_run_time, reg_date, work_hour)
   VALUES
    (:work_date, :time_update, :shift_idx, :factory_idx, :factory_name, :line_idx, :line_name,
     :mac, :machine_idx, :machine_no, :process_name, :planned_work_time,
     :runtime, :productive_runtime, :downtime, :availabilty_rate,
     :target_line_per_day, :target_line_per_hour, :target_mc_per_day, :target_mc_per_hour,
     :cycletime, :pair_info, :pair_count, :theoritical_output, :actual_output,
     :productivity_rate, :defective, :actual_a_grade, :quality_rate, :oee,
     :thread_break, :motor_run, :reg_date, :work_hour)"
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
    ':thread_break'       => $thread_break,
    ':motor_run'          => $motor_run,
];
unset($rows_params[':update_date']);
$stmt->execute($rows_params);

// ─────────────────────────────────────────────────────────────────────
// ── data_oee_rows_hourly UPSERT ──────────────────────────────────────
// ─────────────────────────────────────────────────────────────────────
$stmt = $pdo->prepare(
    "SELECT * FROM `data_oee_rows_hourly` WHERE work_date = ? AND shift_idx = ? AND mac = ? AND process_name = ? AND work_hour = ?"
);
$stmt->execute([$work_date, $shift_idx, $mac, $process_name, $work_hour]);
$hourly_data = $stmt->fetch(PDO::FETCH_ASSOC) ?: null;

upsertEmbOeeRecord(
    $pdo, 'data_oee_rows_hourly', $hourly_data,
    $base_params, $oee_metrics,
    $runtime_sec, $actual_output,
    $packet_qty, $thread_break, $motor_run
);

// ─────────────────────────────────────────────────────────────────────
// ── EMB 전용 테이블 저장 (OEE 계산 없음) ──────────────────────────
// ─────────────────────────────────────────────────────────────────────

// data_oee_emb UPSERT
$stmt = $pdo->prepare(
    "SELECT * FROM `data_oee_emb` WHERE mac = ? AND work_date = ? AND shift_idx = ? AND process_name = ? LIMIT 1"
);
$stmt->execute([$mac, $work_date, $shift_idx, $process_name]);
$emb_daily = $stmt->fetch(PDO::FETCH_ASSOC) ?: null;

upsertEmbPureRecord(
    $pdo, 'data_oee_emb', $emb_daily,
    $base_ctx,
    $runtime_sec, $planned_work_time,
    $packet_qty, $cycle_time, $thread_break, $motor_run,
    $today, $time_update, $work_hour
);

// data_oee_rows_emb INSERT (항상 삽입)
$stmt = $pdo->prepare(
    "INSERT INTO `data_oee_rows_emb`
     (work_date, time_update, shift_idx, factory_idx, factory_name, line_idx, line_name,
      mac, machine_idx, machine_no, process_name,
      planned_work_time, runtime, actual_output, packet_qty,
      cycle_time, thread_breakage, motor_run_time,
      pair_info, pair_count, work_hour, reg_date)
     VALUES
     (:work_date, :time_update, :shift_idx, :factory_idx, :factory_name, :line_idx, :line_name,
      :mac, :machine_idx, :machine_no, :process_name,
      :planned_work_time, :runtime, :actual_output, :packet_qty,
      :cycle_time, :thread_break, :motor_run,
      0, 0, :work_hour, :reg_date)"
);
$stmt->execute([
    ':work_date'         => $work_date,
    ':time_update'       => $time_update,
    ':shift_idx'         => $shift_idx,
    ':factory_idx'       => $factory_idx,
    ':factory_name'      => $factory_name,
    ':line_idx'          => $line_idx,
    ':line_name'         => $line_name,
    ':mac'               => $mac,
    ':machine_idx'       => $machine_idx,
    ':machine_no'        => $machine_no,
    ':process_name'      => $process_name,
    ':planned_work_time' => $planned_work_time,
    ':runtime'           => $runtime_sec,
    ':actual_output'     => $actual_output,
    ':packet_qty'        => $packet_qty,
    ':cycle_time'        => $cycle_time,
    ':thread_break'      => $thread_break,
    ':motor_run'         => $motor_run,
    ':work_hour'         => $work_hour,
    ':reg_date'          => $today,
]);

// data_oee_rows_hourly_emb UPSERT
$stmt = $pdo->prepare(
    "SELECT * FROM `data_oee_rows_hourly_emb` WHERE mac = ? AND work_date = ? AND shift_idx = ? AND process_name = ? AND work_hour = ? LIMIT 1"
);
$stmt->execute([$mac, $work_date, $shift_idx, $process_name, $work_hour]);
$emb_hourly = $stmt->fetch(PDO::FETCH_ASSOC) ?: null;

upsertEmbPureRecord(
    $pdo, 'data_oee_rows_hourly_emb', $emb_hourly,
    $base_ctx,
    $runtime_sec, $planned_work_time,
    $packet_qty, $cycle_time, $thread_break, $motor_run,
    $today, $time_update, $work_hour
);

// ─────────────────────────────────────────────────────────────────────
// API 호출 로그 저장 및 응답 반환
// ─────────────────────────────────────────────────────────────────────
$apiHelper->logApiCall('logs_api_send_ecount', 'send_eCount', $machine_no, $mac, $_REQUEST, $response, $today);
jsonReturn($response);

/*
## API 엔드포인트
embroidery/send_eCount.php (via embroidery.php?code=send_eCount)

## 요청 파라미터
- mac        : 자수기 MAC 주소  (예: 84:72:07:50:AA:BB)
- actual_qty : 이번 패킷 완료 수량  (예: 1)
- ct         : 사이클타임 (초, 정수)  (예: 45)
- tb         : 실끊김 횟수  (예: 0)
- mrt        : 모터동작시간 (초, 정수)  (예: 40)

## 요청 예시
http://SERVER/OEE_SCI/OEE_SCI_V2/api/embroidery.php?code=send_eCount&mac=84:72:07:50:37:AE&actual_qty=1&ct=45&tb=0&mrt=40

## 응답 예시
{
  "code": "00",
  "msg": "Embroidery OEE data updated successfully"
}
*/
