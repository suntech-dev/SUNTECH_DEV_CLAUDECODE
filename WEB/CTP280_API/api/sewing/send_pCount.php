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
// - 이를 통해 이전 근무시간의 잘못된 큰 데이터가 누적되는 것을 방지

require_once(__DIR__ . '/../../lib/api_helper.lib.php');

// 생산 데이터 기반 OEE 지표 계산
function calculateOeeMetrics(PDO $pdo, string $mac, string $work_date, int $shift_idx, int $runtime_sec, float $planned_work_time_hour, int $std_mc_needed, int $target, int $actual_output): array
{

    $metrics = [];

    // 다운타임 및 불량 데이터 조회
    $stmt = $pdo->prepare(
        "SELECT
      (SELECT SUM(duration_sec) FROM data_downtime WHERE mac = :mac AND work_date = :work_date AND shift_idx = :shift_idx) as downtime_duration_sum,
      (SELECT COUNT(*) FROM data_defective WHERE mac = :mac AND work_date = :work_date AND shift_idx = :shift_idx) as defective_count"
    );
    $stmt->execute([':mac' => $mac, ':work_date' => $work_date, ':shift_idx' => $shift_idx]);
    $result = $stmt->fetch(PDO::FETCH_ASSOC);

    $downtime_duration_sum = $result['downtime_duration_sum'] ?? 0;
    $metrics['defective'] = $result['defective_count'] ?? 0;

    // 가동률 지표 계산
    $metrics['downtime'] = $downtime_duration_sum;
    $productive_runtime_sec = $runtime_sec - $downtime_duration_sum;
    // 음수 방지
    $productive_runtime_sec = max(0, $productive_runtime_sec);
    $metrics['productive_runtime'] = $productive_runtime_sec;

    // 가동률 계산 (runtime이 매우 짧으면 비현실적인 값 방지)
    if ($runtime_sec > 0) {
        $raw_availabilty_rate = round((($productive_runtime_sec / $runtime_sec) * 100), 1);
        $metrics['availabilty_rate'] = min($raw_availabilty_rate, 100);
    } else {
        $metrics['availabilty_rate'] = 0;
    }

    // 생산 목표 계산
    $target_line_per_day = $target;
    $target_line_per_hour = ($planned_work_time_hour > 0) ? ($target_line_per_day / $planned_work_time_hour) : 0;
    $target_mc_per_day = ($std_mc_needed > 0) ? ($target_line_per_day / $std_mc_needed) : 0;
    $target_mc_per_hour = ($std_mc_needed > 0) ? ($target_line_per_hour / $std_mc_needed) : 0;
    $metrics['target_line_per_day'] = $target_line_per_day;
    $metrics['target_line_per_hour'] = $target_line_per_hour;
    $metrics['target_mc_per_day'] = $target_mc_per_day;
    $metrics['target_mc_per_hour'] = $target_mc_per_hour;

    // 사이클 타임 및 이론적 생산량 계산
    $metrics['cycletime'] = ($target_mc_per_hour > 0) ? (3600 / $target_mc_per_hour) : 0;
    $metrics['theoritical_output'] = ($metrics['cycletime'] > 0) ? ($productive_runtime_sec / $metrics['cycletime']) : 0;

    // 성능률 및 품질률 계산
    if ($metrics['theoritical_output'] > 0) {
        $raw_productivity_rate = round((($actual_output / $metrics['theoritical_output']) * 100), 1);
        // 비율 상한선: 최대 200% (비현실적으로 높은 값 방지)
        $metrics['productivity_rate'] = min($raw_productivity_rate, 200);
    } else {
        $metrics['productivity_rate'] = 0;
    }

    $metrics['actual_a_grade'] = max(0, $actual_output - $metrics['defective']);

    // 품질률 계산 (100% 초과 방지)
    if ($actual_output > 0) {
        $raw_quality_rate = round((($metrics['actual_a_grade'] / $actual_output) * 100), 1);
        $metrics['quality_rate'] = min($raw_quality_rate, 100);
    } else {
        $metrics['quality_rate'] = 0;
    }

    // 최종 OEE 계산
    $metrics['oee'] = round((($metrics['availabilty_rate'] * $metrics['productivity_rate'] * $metrics['quality_rate']) / 10000), 2);

    return $metrics;
}

$today = date('Y-m-d H:i:s');
$time_update = date('H:i:s');
$work_hour = date('H');
$apiHelper = new ApiHelper($pdo);

// MAC 주소 검증 및 처리
$mac = $apiHelper->validateAndProcessMac($_REQUEST['mac'] ?? '');

// 요청 파라미터 가져오기
$pair_info = trim($_REQUEST['pi'] ?? 0);
$pair_count = trim($_REQUEST['pc'] ?? 0);
$design_no = trim($_REQUEST['design_no'] ?? 0);
$actual_output = trim($_REQUEST['sc'] ?? 0);
$first_sewing_time = trim($_REQUEST['ct'] ?? 0);
$first_sewing_time = ($first_sewing_time > 0) ? (floor(($first_sewing_time / 1000) * 10) / 10) : 0;

// 기계 정보 및 공장, 라인 데이터 조회
$stmt = $pdo->prepare(
    "SELECT a.*, b.factory_name, c.line_name, c.line_target
   FROM info_machine AS a
   LEFT JOIN info_factory AS b ON b.idx = a.factory_idx
   LEFT JOIN info_line AS c ON c.idx = a.line_idx
   WHERE a.mac = ?"
);
$stmt->execute([$mac]);
$machine_data = $stmt->fetch(PDO::FETCH_ASSOC);

if (!$machine_data) {
    jsonReturn(['code' => '99', 'msg' => 'Machine not found for MAC address']);
}

$factory_idx = $machine_data['factory_idx'];
$factory_name = $machine_data['factory_name'];
$line_idx = $machine_data['line_idx'];
$line_name = $machine_data['line_name'];
$machine_idx = $machine_data['idx'];
$machine_no = $machine_data['machine_no'];
$target = $machine_data['line_target'] ?? 0;

// INVENTORY 머신 체크 (line_idx = 99는 데이터 저장 안 함)
if ($line_idx == 99) {
    jsonReturn(['code' => '00', 'msg' => 'Skipped: Machine in INVENTORY (line_idx=99)']);
}

// 기계의 design_process_idx 확인
$design_process_idx = $machine_data['design_process_idx'] ?? 0;

if ($design_process_idx == 0) {
    jsonReturn(['code' => '99', 'msg' => 'Design process not configured for this machine']);
}

// 디자인 공정 정보 조회 (info_design_process에서 직접 조회)
$stmt = $pdo->prepare(
    "SELECT std_mc_needed, design_process
   FROM info_design_process
   WHERE idx = ? AND status = 'Y'"
);
$stmt->execute([$design_process_idx]);
$design_process_data = $stmt->fetch(PDO::FETCH_ASSOC);

if (!$design_process_data) {
    jsonReturn(['code' => '99', 'msg' => 'Design process not found or inactive']);
}

$process_name = $design_process_data['design_process'];
$std_mc_needed = $design_process_data['std_mc_needed'] ?? 1;

// 현재 시프트 정보 조회
$current_shift_info = $apiHelper->getCurrentShiftInfo($factory_idx, $line_idx, $today);

if (!$current_shift_info) {
    jsonReturn(['code' => '99', 'msg' => 'Not during working hours']);
}

// 작업 시간 지표 계산
$worktime = new Worktime($pdo);
$all_shifts_on_date = $worktime->getDayShift($current_shift_info['date'], $factory_idx, $line_idx);
$worktime_metrics = calculateWorktimeMetrics($current_shift_info, $all_shifts_on_date, $today);

$work_date = $current_shift_info['date'];
$shift_idx = $current_shift_info['shift_idx'];
$shift_start_datetime = $work_date . ' ' . $current_shift_info['available_stime'] . ':00';
$planned_work_time = $worktime_metrics['net_work_minutes'] * 60;
$planned_work_time_hour = ($worktime_metrics['net_work_minutes'] > 0) ? ($worktime_metrics['net_work_minutes'] / 60) : 1;
$runtime_sec = $worktime_metrics['actual_passed_work_seconds'];

// 리셋 감지: data_oee_rows에서 현재 근무시간의 가장 최근 actual_output 조회
$stmt = $pdo->prepare("SELECT actual_output FROM data_oee_rows WHERE mac = ? AND work_date = ? AND shift_idx = ? AND process_name = ? ORDER BY idx DESC LIMIT 1");
$stmt->execute([$mac, $work_date, $shift_idx, $process_name]);
$latest_row = $stmt->fetch(PDO::FETCH_ASSOC);
$is_reset = false;

// 작업자가 기계를 리셋한 경우 감지 (실수로 리셋 안한 상태에서 늦게 리셋)
// 새로 들어온 actual_output이 이전보다 10개 이상 적으면 리셋으로 판단
if ($latest_row && $actual_output < ($latest_row['actual_output'] - 10)) {
    $is_reset = true;
}

// 현재 공정의 기존 OEE 데이터 조회
$stmt = $pdo->prepare("SELECT * FROM data_oee WHERE mac = ? AND work_date = ? AND shift_idx = ? AND process_name = ? ORDER BY idx ASC LIMIT 1");
$stmt->execute([$mac, $work_date, $shift_idx, $process_name]);
$oee_data = $stmt->fetch(PDO::FETCH_ASSOC);

// OEE 누적 지표 계산
$oee_metrics = calculateOeeMetrics($pdo, $mac, $work_date, $shift_idx, $runtime_sec, $planned_work_time_hour, $std_mc_needed, $target, $actual_output);

// 공정별 델타 계산
if ($oee_data) {
    // UPDATE 케이스: 기존 레코드 대비 증가분 계산
    $process_actual_output = max(0, $actual_output - $oee_data['actual_output']);
    $process_defective = max(0, $oee_metrics['defective'] - $oee_data['defective']);
    $process_downtime = max(0, $oee_metrics['downtime'] - $oee_data['downtime']);
    $process_productive_runtime = max(0, $oee_metrics['productive_runtime'] - $oee_data['productive_runtime']);
    $process_theoritical_output = max(0, $oee_metrics['theoritical_output'] - $oee_data['theoritical_output']);
    $process_actual_a_grade = max(0, $process_actual_output - $process_defective);
    $process_runtime = max(0, $runtime_sec - $oee_data['runtime']);

    // 공정별 비율은 누적값 기준으로 계산 (델타가 아님!)
    $process_availabilty_rate = $oee_metrics['availabilty_rate'];
    $process_productivity_rate = $oee_metrics['productivity_rate'];
    $process_quality_rate = $oee_metrics['quality_rate'];
    $process_oee = $oee_metrics['oee'];
} else {
    // INSERT 케이스: 이전 공정 대비 델타 계산
    $stmt = $pdo->prepare(
        "SELECT * FROM data_oee
     WHERE mac = ? AND work_date = ? AND shift_idx = ? AND process_name != ?
     ORDER BY idx DESC LIMIT 1"
    );
    $stmt->execute([$mac, $work_date, $shift_idx, $process_name]);
    $prev_process_data = $stmt->fetch(PDO::FETCH_ASSOC);

    if ($prev_process_data) {
        // 이전 공정 데이터가 있으면 델타 계산
        $process_actual_output = max(0, $actual_output - $prev_process_data['actual_output']);
        $process_defective = max(0, $oee_metrics['defective'] - $prev_process_data['defective']);
        $process_downtime = max(0, $oee_metrics['downtime'] - $prev_process_data['downtime']);
        $process_productive_runtime = max(0, $oee_metrics['productive_runtime'] - $prev_process_data['productive_runtime']);
        $process_theoritical_output = max(0, $oee_metrics['theoritical_output'] - $prev_process_data['theoritical_output']);
        $process_actual_a_grade = max(0, $process_actual_output - $process_defective);
        $process_runtime = max(0, $runtime_sec - $prev_process_data['runtime']);

        // 공정별 비율은 누적값 기준으로 계산 (델타가 아님!)
        $process_availabilty_rate = $oee_metrics['availabilty_rate'];
        $process_productivity_rate = $oee_metrics['productivity_rate'];
        $process_quality_rate = $oee_metrics['quality_rate'];
        $process_oee = $oee_metrics['oee'];
    } else {
        // 첫 공정이면 누적값 그대로 사용
        $process_actual_output = $actual_output;
        $process_defective = $oee_metrics['defective'];
        $process_downtime = $oee_metrics['downtime'];
        $process_productive_runtime = $oee_metrics['productive_runtime'];
        $process_theoritical_output = $oee_metrics['theoritical_output'];
        $process_actual_a_grade = $oee_metrics['actual_a_grade'];
        $process_runtime = $runtime_sec;
        $process_availabilty_rate = $oee_metrics['availabilty_rate'];
        $process_productivity_rate = $oee_metrics['productivity_rate'];
        $process_quality_rate = $oee_metrics['quality_rate'];
        $process_oee = $oee_metrics['oee'];
    }
}

if ($oee_data) {
    if ($is_reset) {
        // 리셋 감지: 절대값으로 설정 (작업자가 기계 리셋 후 새로 시작)
        $stmt = $pdo->prepare(
            "UPDATE data_oee SET
        time_update = ?, planned_work_time = ?,
        runtime = ?,
        productive_runtime = ?,
        downtime = ?,
        availabilty_rate = ?,
        target_line_per_day = ?, target_line_per_hour = ?, target_mc_per_day = ?,
        target_mc_per_hour = ?, cycletime = ?, pair_info = ?, pair_count = ?,
        theoritical_output = ?,
        actual_output = ?,
        productivity_rate = ?,
        defective = ?,
        actual_a_grade = ?,
        quality_rate = ?, oee = ?,
        update_date = ?, work_hour = ?
      WHERE idx = ?"
        );
        $stmt->execute([
            $time_update,
            $planned_work_time,
            $runtime_sec,
            $oee_metrics['productive_runtime'],
            $oee_metrics['downtime'],
            $oee_metrics['availabilty_rate'],
            $oee_metrics['target_line_per_day'],
            $oee_metrics['target_line_per_hour'],
            $oee_metrics['target_mc_per_day'],
            $oee_metrics['target_mc_per_hour'],
            $oee_metrics['cycletime'],
            $pair_info,
            $pair_count,
            $oee_metrics['theoritical_output'],
            $actual_output,
            $oee_metrics['productivity_rate'],
            $oee_metrics['defective'],
            $oee_metrics['actual_a_grade'],
            $oee_metrics['quality_rate'],
            $oee_metrics['oee'],
            $today,
            $work_hour,
            $oee_data['idx']
        ]);
        $response = ['code' => '00', 'msg' => 'OEE data updated successfully (reset detected)'];
    } else {
        // 정상: 델타 값을 기존 값에 더함
        // 주의: actual_a_grade는 파생 계산값이므로 누적값으로 직접 SET
        $stmt = $pdo->prepare(
            "UPDATE data_oee SET
        time_update = ?, planned_work_time = ?,
        runtime = runtime + ?,
        productive_runtime = productive_runtime + ?,
        downtime = downtime + ?,
        availabilty_rate = ?,
        target_line_per_day = ?, target_line_per_hour = ?, target_mc_per_day = ?,
        target_mc_per_hour = ?, cycletime = ?, pair_info = ?, pair_count = ?,
        theoritical_output = theoritical_output + ?,
        actual_output = actual_output + ?,
        productivity_rate = ?,
        defective = defective + ?,
        actual_a_grade = ?,
        quality_rate = ?, oee = ?,
        update_date = ?, work_hour = ?
      WHERE idx = ?"
        );
        $stmt->execute([
            $time_update,
            $planned_work_time,
            $process_runtime,
            $process_productive_runtime,
            $process_downtime,
            $process_availabilty_rate,
            $oee_metrics['target_line_per_day'],
            $oee_metrics['target_line_per_hour'],
            $oee_metrics['target_mc_per_day'],
            $oee_metrics['target_mc_per_hour'],
            $oee_metrics['cycletime'],
            $pair_info,
            $pair_count,
            $process_theoritical_output,
            $process_actual_output,
            $process_productivity_rate,
            $process_defective,
            $oee_metrics['actual_a_grade'],
            $process_quality_rate,
            $process_oee,
            $today,
            $work_hour,
            $oee_data['idx']
        ]);
        $response = ['code' => '00', 'msg' => 'OEE data updated successfully'];
    }
} else {
    // 새로운 OEE 데이터 삽입 (델타 값 저장)
    // 참고: 초기 6S 다운타임 기록 제거 (중복/과도한 다운타임 방지)
    // 다운타임은 별도 API(send_andon_warning 등)를 통해 명시적으로 기록해야 함

    $stmt = $pdo->prepare(
        "INSERT INTO `data_oee`
      (work_date, time_update, shift_idx, factory_idx, factory_name, line_idx, line_name, mac, machine_idx, machine_no, process_name, planned_work_time, runtime, productive_runtime, downtime, availabilty_rate, target_line_per_day, target_line_per_hour, target_mc_per_day, target_mc_per_hour, cycletime, pair_info, pair_count, theoritical_output, actual_output, productivity_rate, defective, actual_a_grade, quality_rate, oee, reg_date, update_date, work_hour)
    VALUES
      (:work_date, :time_update, :shift_idx, :factory_idx, :factory_name, :line_idx, :line_name, :mac, :machine_idx, :machine_no, :process_name, :planned_work_time, :runtime, :productive_runtime, :downtime, :availabilty_rate, :target_line_per_day, :target_line_per_hour, :target_mc_per_day, :target_mc_per_hour, :cycletime, :pair_info, :pair_count, :theoritical_output, :actual_output, :productivity_rate, :defective, :actual_a_grade, :quality_rate, :oee, :reg_date, :update_date, :work_hour)"
    );
    $stmt->execute([
        ':work_date' => $work_date,
        ':time_update' => $time_update,
        ':shift_idx' => $shift_idx,
        ':factory_idx' => $factory_idx,
        ':factory_name' => $factory_name,
        ':line_idx' => $line_idx,
        ':line_name' => $line_name,
        ':mac' => $mac,
        ':machine_idx' => $machine_idx,
        ':machine_no' => $machine_no,
        ':process_name' => $process_name,
        ':planned_work_time' => $planned_work_time,
        ':runtime' => $process_runtime,
        ':productive_runtime' => $process_productive_runtime,
        ':downtime' => $process_downtime,
        ':availabilty_rate' => $process_availabilty_rate,
        ':target_line_per_day' => $oee_metrics['target_line_per_day'],
        ':target_line_per_hour' => $oee_metrics['target_line_per_hour'],
        ':target_mc_per_day' => $oee_metrics['target_mc_per_day'],
        ':target_mc_per_hour' => $oee_metrics['target_mc_per_hour'],
        ':cycletime' => $oee_metrics['cycletime'],
        ':pair_info' => $pair_info,
        ':pair_count' => $pair_count,
        ':theoritical_output' => $process_theoritical_output,
        ':actual_output' => $process_actual_output,
        ':productivity_rate' => $process_productivity_rate,
        ':defective' => $process_defective,
        ':actual_a_grade' => $process_actual_a_grade,
        ':quality_rate' => $process_quality_rate,
        ':oee' => $process_oee,
        ':reg_date' => $today,
        ':update_date' => $today,
        ':work_hour' => $work_hour
    ]);
    $response = ['code' => '00', 'msg' => 'OEE data inserted successfully'];
}

// data_oee_rows 테이블에 항상 INSERT (누적값 저장)
$stmt = $pdo->prepare(
    "INSERT INTO `data_oee_rows`
    (work_date, time_update, shift_idx, factory_idx, factory_name, line_idx, line_name, mac, machine_idx, machine_no, process_name, planned_work_time, runtime, productive_runtime, downtime, availabilty_rate, target_line_per_day, target_line_per_hour, target_mc_per_day, target_mc_per_hour, cycletime, pair_info, pair_count, theoritical_output, actual_output, productivity_rate, defective, actual_a_grade, quality_rate, oee, reg_date, work_hour)
  VALUES
    (:work_date, :time_update, :shift_idx, :factory_idx, :factory_name, :line_idx, :line_name, :mac, :machine_idx, :machine_no, :process_name, :planned_work_time, :runtime, :productive_runtime, :downtime, :availabilty_rate, :target_line_per_day, :target_line_per_hour, :target_mc_per_day, :target_mc_per_hour, :cycletime, :pair_info, :pair_count, :theoritical_output, :actual_output, :productivity_rate, :defective, :actual_a_grade, :quality_rate, :oee, :reg_date, :work_hour)"
);
$stmt->execute([
    ':work_date' => $work_date,
    ':time_update' => $time_update,
    ':shift_idx' => $shift_idx,
    ':factory_idx' => $factory_idx,
    ':factory_name' => $factory_name,
    ':line_idx' => $line_idx,
    ':line_name' => $line_name,
    ':mac' => $mac,
    ':machine_idx' => $machine_idx,
    ':machine_no' => $machine_no,
    ':process_name' => $process_name,
    ':planned_work_time' => $planned_work_time,
    ':runtime' => $runtime_sec,
    ':productive_runtime' => $oee_metrics['productive_runtime'],
    ':downtime' => $oee_metrics['downtime'],
    ':availabilty_rate' => $oee_metrics['availabilty_rate'],
    ':target_line_per_day' => $oee_metrics['target_line_per_day'],
    ':target_line_per_hour' => $oee_metrics['target_line_per_hour'],
    ':target_mc_per_day' => $oee_metrics['target_mc_per_day'],
    ':target_mc_per_hour' => $oee_metrics['target_mc_per_hour'],
    ':cycletime' => $oee_metrics['cycletime'],
    ':pair_info' => $pair_info,
    ':pair_count' => $pair_count,
    ':theoritical_output' => $oee_metrics['theoritical_output'],
    ':actual_output' => $actual_output,
    ':productivity_rate' => $oee_metrics['productivity_rate'],
    ':defective' => $oee_metrics['defective'],
    ':actual_a_grade' => $oee_metrics['actual_a_grade'],
    ':quality_rate' => $oee_metrics['quality_rate'],
    ':oee' => $oee_metrics['oee'],
    ':reg_date' => $today,
    ':work_hour' => $work_hour
]);

// data_oee_rows_hourly 테이블 UPSERT (work_date & shift_idx & mac & process_name & work_hour 기준)
$stmt = $pdo->prepare("SELECT * FROM `data_oee_rows_hourly` WHERE work_date = ? AND shift_idx = ? AND mac = ? AND process_name = ? AND work_hour = ?");
$stmt->execute([$work_date, $shift_idx, $mac, $process_name, $work_hour]);
$hourly_data = $stmt->fetch(PDO::FETCH_ASSOC);

// 시간별 델타 계산
if ($hourly_data) {
    // UPDATE 케이스: 기존 레코드 대비 증가분 계산
    $hourly_actual_output = max(0, $actual_output - $hourly_data['actual_output']);
    $hourly_defective = max(0, $oee_metrics['defective'] - $hourly_data['defective']);
    $hourly_downtime = max(0, $oee_metrics['downtime'] - $hourly_data['downtime']);
    $hourly_productive_runtime = max(0, $oee_metrics['productive_runtime'] - $hourly_data['productive_runtime']);
    $hourly_theoritical_output = max(0, $oee_metrics['theoritical_output'] - $hourly_data['theoritical_output']);
    $hourly_actual_a_grade = max(0, $hourly_actual_output - $hourly_defective);
    $hourly_runtime = max(0, $runtime_sec - $hourly_data['runtime']);

    // 시간별 비율은 누적값 기준으로 계산 (델타가 아님!)
    $hourly_availabilty_rate = $oee_metrics['availabilty_rate'];
    $hourly_productivity_rate = $oee_metrics['productivity_rate'];
    $hourly_quality_rate = $oee_metrics['quality_rate'];
    $hourly_oee = $oee_metrics['oee'];
} else {
    // INSERT 케이스: 이전 시간 대비 델타 계산
    $stmt = $pdo->prepare(
        "SELECT * FROM `data_oee_rows_hourly`
     WHERE work_date = ? AND shift_idx = ? AND mac = ? AND process_name = ? AND work_hour < ?
     ORDER BY work_hour DESC LIMIT 1"
    );
    $stmt->execute([$work_date, $shift_idx, $mac, $process_name, $work_hour]);
    $prev_hourly_data = $stmt->fetch(PDO::FETCH_ASSOC);

    if ($prev_hourly_data) {
        // 이전 시간 데이터가 있으면 델타 계산
        $hourly_actual_output = max(0, $actual_output - $prev_hourly_data['actual_output']);
        $hourly_defective = max(0, $oee_metrics['defective'] - $prev_hourly_data['defective']);
        $hourly_downtime = max(0, $oee_metrics['downtime'] - $prev_hourly_data['downtime']);
        $hourly_productive_runtime = max(0, $oee_metrics['productive_runtime'] - $prev_hourly_data['productive_runtime']);
        $hourly_theoritical_output = max(0, $oee_metrics['theoritical_output'] - $prev_hourly_data['theoritical_output']);
        $hourly_actual_a_grade = max(0, $hourly_actual_output - $hourly_defective);
        $hourly_runtime = max(0, $runtime_sec - $prev_hourly_data['runtime']);

        // 시간별 비율은 누적값 기준으로 계산 (델타가 아님!)
        $hourly_availabilty_rate = $oee_metrics['availabilty_rate'];
        $hourly_productivity_rate = $oee_metrics['productivity_rate'];
        $hourly_quality_rate = $oee_metrics['quality_rate'];
        $hourly_oee = $oee_metrics['oee'];
    } else {
        // 첫 시간이면 누적값 그대로 사용
        $hourly_actual_output = $actual_output;
        $hourly_defective = $oee_metrics['defective'];
        $hourly_downtime = $oee_metrics['downtime'];
        $hourly_productive_runtime = $oee_metrics['productive_runtime'];
        $hourly_theoritical_output = $oee_metrics['theoritical_output'];
        $hourly_actual_a_grade = $oee_metrics['actual_a_grade'];
        $hourly_runtime = $runtime_sec;
        $hourly_availabilty_rate = $oee_metrics['availabilty_rate'];
        $hourly_productivity_rate = $oee_metrics['productivity_rate'];
        $hourly_quality_rate = $oee_metrics['quality_rate'];
        $hourly_oee = $oee_metrics['oee'];
    }
}

if ($hourly_data) {
    if ($is_reset) {
        // 리셋 감지: 절대값으로 설정 (작업자가 기계 리셋 후 새로 시작)
        $stmt = $pdo->prepare(
            "UPDATE `data_oee_rows_hourly` SET
        time_update = :time_update,
        planned_work_time = :planned_work_time,
        runtime = :runtime,
        productive_runtime = :productive_runtime,
        downtime = :downtime,
        availabilty_rate = :availabilty_rate,
        target_line_per_day = :target_line_per_day,
        target_line_per_hour = :target_line_per_hour,
        target_mc_per_day = :target_mc_per_day,
        target_mc_per_hour = :target_mc_per_hour,
        cycletime = :cycletime,
        pair_info = :pair_info,
        pair_count = :pair_count,
        theoritical_output = :theoritical_output,
        actual_output = :actual_output,
        productivity_rate = :productivity_rate,
        defective = :defective,
        actual_a_grade = :actual_a_grade,
        quality_rate = :quality_rate,
        oee = :oee,
        update_date = :update_date
      WHERE idx = :idx"
        );
        $stmt->execute([
            ':time_update' => $time_update,
            ':planned_work_time' => $planned_work_time,
            ':runtime' => $runtime_sec,
            ':productive_runtime' => $oee_metrics['productive_runtime'],
            ':downtime' => $oee_metrics['downtime'],
            ':availabilty_rate' => $oee_metrics['availabilty_rate'],
            ':target_line_per_day' => $oee_metrics['target_line_per_day'],
            ':target_line_per_hour' => $oee_metrics['target_line_per_hour'],
            ':target_mc_per_day' => $oee_metrics['target_mc_per_day'],
            ':target_mc_per_hour' => $oee_metrics['target_mc_per_hour'],
            ':cycletime' => $oee_metrics['cycletime'],
            ':pair_info' => $pair_info,
            ':pair_count' => $pair_count,
            ':theoritical_output' => $oee_metrics['theoritical_output'],
            ':actual_output' => $actual_output,
            ':productivity_rate' => $oee_metrics['productivity_rate'],
            ':defective' => $oee_metrics['defective'],
            ':actual_a_grade' => $oee_metrics['actual_a_grade'],
            ':quality_rate' => $oee_metrics['quality_rate'],
            ':oee' => $oee_metrics['oee'],
            ':update_date' => $today,
            ':idx' => $hourly_data['idx']
        ]);
    } else {
        // 정상: 델타 값을 기존 값에 더함
        // 주의: actual_a_grade는 파생 계산값이므로 누적값으로 직접 SET
        $stmt = $pdo->prepare(
            "UPDATE `data_oee_rows_hourly` SET
        time_update = :time_update,
        planned_work_time = :planned_work_time,
        runtime = runtime + :runtime,
        productive_runtime = productive_runtime + :productive_runtime,
        downtime = downtime + :downtime,
        availabilty_rate = :availabilty_rate,
        target_line_per_day = :target_line_per_day,
        target_line_per_hour = :target_line_per_hour,
        target_mc_per_day = :target_mc_per_day,
        target_mc_per_hour = :target_mc_per_hour,
        cycletime = :cycletime,
        pair_info = :pair_info,
        pair_count = :pair_count,
        theoritical_output = theoritical_output + :theoritical_output,
        actual_output = actual_output + :actual_output,
        productivity_rate = :productivity_rate,
        defective = defective + :defective,
        actual_a_grade = :actual_a_grade,
        quality_rate = :quality_rate,
        oee = :oee,
        update_date = :update_date
      WHERE idx = :idx"
        );
        $stmt->execute([
            ':time_update' => $time_update,
            ':planned_work_time' => $planned_work_time,
            ':runtime' => $hourly_runtime,
            ':productive_runtime' => $hourly_productive_runtime,
            ':downtime' => $hourly_downtime,
            ':availabilty_rate' => $hourly_availabilty_rate,
            ':target_line_per_day' => $oee_metrics['target_line_per_day'],
            ':target_line_per_hour' => $oee_metrics['target_line_per_hour'],
            ':target_mc_per_day' => $oee_metrics['target_mc_per_day'],
            ':target_mc_per_hour' => $oee_metrics['target_mc_per_hour'],
            ':cycletime' => $oee_metrics['cycletime'],
            ':pair_info' => $pair_info,
            ':pair_count' => $pair_count,
            ':theoritical_output' => $hourly_theoritical_output,
            ':actual_output' => $hourly_actual_output,
            ':productivity_rate' => $hourly_productivity_rate,
            ':defective' => $hourly_defective,
            ':actual_a_grade' => $oee_metrics['actual_a_grade'],
            ':quality_rate' => $hourly_quality_rate,
            ':oee' => $hourly_oee,
            ':update_date' => $today,
            ':idx' => $hourly_data['idx']
        ]);
    }
} else {
    // 새로운 데이터 삽입 (델타 값 저장)
    $stmt = $pdo->prepare(
        "INSERT INTO `data_oee_rows_hourly`
      (work_date, time_update, shift_idx, factory_idx, factory_name, line_idx, line_name, mac, machine_idx, machine_no, process_name, planned_work_time, runtime, productive_runtime, downtime, availabilty_rate, target_line_per_day, target_line_per_hour, target_mc_per_day, target_mc_per_hour, cycletime, pair_info, pair_count, theoritical_output, actual_output, productivity_rate, defective, actual_a_grade, quality_rate, oee, reg_date, update_date, work_hour)
    VALUES
      (:work_date, :time_update, :shift_idx, :factory_idx, :factory_name, :line_idx, :line_name, :mac, :machine_idx, :machine_no, :process_name, :planned_work_time, :runtime, :productive_runtime, :downtime, :availabilty_rate, :target_line_per_day, :target_line_per_hour, :target_mc_per_day, :target_mc_per_hour, :cycletime, :pair_info, :pair_count, :theoritical_output, :actual_output, :productivity_rate, :defective, :actual_a_grade, :quality_rate, :oee, :reg_date, :update_date, :work_hour)"
    );
    $stmt->execute([
        ':work_date' => $work_date,
        ':time_update' => $time_update,
        ':shift_idx' => $shift_idx,
        ':factory_idx' => $factory_idx,
        ':factory_name' => $factory_name,
        ':line_idx' => $line_idx,
        ':line_name' => $line_name,
        ':mac' => $mac,
        ':machine_idx' => $machine_idx,
        ':machine_no' => $machine_no,
        ':process_name' => $process_name,
        ':planned_work_time' => $planned_work_time,
        ':runtime' => $hourly_runtime,
        ':productive_runtime' => $hourly_productive_runtime,
        ':downtime' => $hourly_downtime,
        ':availabilty_rate' => $hourly_availabilty_rate,
        ':target_line_per_day' => $oee_metrics['target_line_per_day'],
        ':target_line_per_hour' => $oee_metrics['target_line_per_hour'],
        ':target_mc_per_day' => $oee_metrics['target_mc_per_day'],
        ':target_mc_per_hour' => $oee_metrics['target_mc_per_hour'],
        ':cycletime' => $oee_metrics['cycletime'],
        ':pair_info' => $pair_info,
        ':pair_count' => $pair_count,
        ':theoritical_output' => $hourly_theoritical_output,
        ':actual_output' => $hourly_actual_output,
        ':productivity_rate' => $hourly_productivity_rate,
        ':defective' => $hourly_defective,
        ':actual_a_grade' => $hourly_actual_a_grade,
        ':quality_rate' => $hourly_quality_rate,
        ':oee' => $hourly_oee,
        ':reg_date' => $today,
        ':update_date' => $today,
        ':work_hour' => $work_hour
    ]);
}


// API 호출 로깅
$apiHelper->logApiCall('logs_api_send_pCount', 'send_pCount', $machine_no, $mac, $_REQUEST, $response, $today);

// 최종 응답 반환
jsonReturn($response);

/*
## API 엔드포인트
send_pCount.php

## 요청 예시
http://49.247.26.228/2025/sci/new/api/sewing.php?code=send_pCount&mac=84:72:07:50:37:73&design_no=A001&sc=100&pi=1&pc=50

## 응답 예시
{
  "code": "00",
  "msg": "OEE data updated successfully"
}
*/