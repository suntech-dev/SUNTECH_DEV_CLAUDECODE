<?php

// require_once('../lib/db.php');
// require_once('../lib/worktime.lib.php');
// require_once('./db.php');
// require_once('./worktime.lib.php');

/**
 * 주어진 날짜와 시간, 설비 정보를 바탕으로 현재 진행 중인 근무조(Shift) 정보를 찾습니다.
 * 야간 근무조를 고려하여 어제와 오늘 근무 목록을 모두 확인합니다.
 *
 * @param PDO $pdo PDO 데이터베이스 연결 객체
 * @param Worktime $worktime Worktime 라이브러리 객체
 * @param string $factory_idx 공장 인덱스
 * @param string $line_idx 라인 인덱스
 * @param string $current_datetime_str 현재 날짜 및 시간 (Y-m-d H:i:s 형식)
 * @return array|null 현재 근무조 정보를 담은 배열 또는 찾지 못한 경우 null
 */

function findCurrentShift(PDO $pdo, Worktime $worktime, string $factory_idx, string $line_idx, string $current_datetime_str): ?array
{
    $current_date = substr($current_datetime_str, 0, 10);
    $prev_date = date('Y-m-d', strtotime($current_date . ' -1 day'));
    $current_millis = strtotime($current_datetime_str);

    // 어제와 오늘 근무 데이터를 가져옵니다.
    $shifts_data = [];
    $prev_shifts = $worktime->getDayShift($prev_date, $factory_idx, $line_idx);
    if ($prev_shifts) {
        $shifts_data[$prev_date] = $prev_shifts['shift'];
    }
    $today_shifts = $worktime->getDayShift($current_date, $factory_idx, $line_idx);
    if ($today_shifts) {
        $shifts_data[$current_date] = $today_shifts['shift'];
    }

    $all_shifts = [];
    // 가져온 근무 데이터를 순회하며 처리 가능한 형태로 정규화합니다.
    foreach ($shifts_data as $date => $shifts) {
        $first_shift_stime_str = '';
        foreach ($shifts as $shift) {
            if (empty($shift['available_stime']) || empty($shift['available_etime'])) {
                continue;
            }

            // 첫 번째 근무조의 시작 시간을 기준으로 날짜 변경 여부를 판단합니다.
            if ($first_shift_stime_str === '') {
                $first_shift_stime_str = $date . ' ' . $shift['available_stime'] . ':00';
            }

            $work_stime_str = $date . ' ' . $shift['available_stime'] . ':00';
            $work_etime_str = $date . ' ' . $shift['available_etime'] . ':00';

            // 잔업(over_time)이 있으면 종료 시간에 추가합니다.
            if ($shift['over_time']) {
                $work_etime_str = date('Y-m-d H:i:s', strtotime($work_etime_str . ' +' . $shift['over_time'] . ' minutes'));
            }

            // 시작 시간이 첫 근무조의 시작 시간보다 이전이면 다음 날로 처리합니다. (야간 근무)
            if ($first_shift_stime_str > $work_stime_str) {
                $work_stime_str = date('Y-m-d H:i:s', strtotime($work_stime_str . ' +1 day'));
            }
            // 종료 시간이 첫 근무조의 시작 시간보다 이전이거나, 시작 시간보다 종료 시간이 앞서는 경우 다음 날로 처리합니다.
            if ($first_shift_stime_str > $work_etime_str || $work_stime_str >= $work_etime_str) {
                $work_etime_str = date('Y-m-d H:i:s', strtotime($work_etime_str . ' +1 day'));
            }

            // 정규화된 근무 정보를 배열에 추가합니다.
            $all_shifts[] = [
                'shift_idx' => $shift['shift_idx'],
                'shift_name' => 'SHIFT ' . $shift['shift_idx'],
                'date' => $date,
                'available_stime' => $shift['available_stime'],
                'available_etime_orig' => $shift['available_etime'], // OT 미포함 종료시간
                'available_etime' => substr($work_etime_str, 11, 5), // OT 포함 종료시간
                'over_time' => $shift['over_time'],
                'work_stime' => $work_stime_str,
                'work_etime' => $work_etime_str,
            ];
        }
    }

    // 모든 근무조 중에서 현재 시간에 해당하는 근무조를 찾습니다.
    foreach ($all_shifts as $shift) {
        if (strtotime($shift['work_stime']) <= $current_millis && $current_millis < strtotime($shift['work_etime'])) {
            return $shift;
        }
    }

    return null;
}

/**
 * 주어진 근무조 정보를 바탕으로 각종 근무 시간 지표를 계산합니다.
 *
 * @param array $current_shift_info 현재 근무조 정보
 * @param array $all_shifts_on_date 현재 근무조가 속한 날짜의 모든 근무조 정보
 * @param string $current_datetime_str 현재 날짜 및 시간
 * @return array 계산된 근무 시간 지표
 */
function calculateWorktimeMetrics(array $current_shift_info, array $all_shifts_on_date, string $current_datetime_str): array
{
    $metrics = [];
    $current_millis = strtotime($current_datetime_str);
    $shift_details = $all_shifts_on_date['shift'][$current_shift_info['shift_idx']];

    // --- 기본 정보 계산 ---
    $metrics['planned_work_hours_str'] = $current_shift_info['available_stime'] . ' ~ ' . $current_shift_info['available_etime'];
    $metrics['over_time_seconds'] = $current_shift_info['over_time'] * 60;

    // --- 경과 시간 계산 ---
    $passed_work_seconds = 0;
    $work_start_millis = strtotime($current_shift_info['work_stime']);
    if ($current_millis >= $work_start_millis) {
        $passed_work_seconds = $current_millis - $work_start_millis;
    }
    $metrics['passed_work_seconds'] = $passed_work_seconds;

    // --- 휴식 시간 계산 (경과 및 전체) ---
    $total_passed_break_seconds = 0;
    $total_break_seconds = 0;
    $first_shift_stime_str = $current_shift_info['date'] . ' ' . $all_shifts_on_date['shift'][array_key_first($all_shifts_on_date['shift'])]['available_stime'] . ':00';

    for ($i = 1; $i <= 5; $i++) {
        $stime_key = 'planned' . $i . '_stime';
        $etime_key = 'planned' . $i . '_etime';

        if (!empty($shift_details[$stime_key]) && !empty($shift_details[$etime_key])) {
            $break_s_str = $current_shift_info['date'] . ' ' . $shift_details[$stime_key] . ':00';
            $break_e_str = $current_shift_info['date'] . ' ' . $shift_details[$etime_key] . ':00';

            if ($first_shift_stime_str > $break_s_str) $break_s_str = date('Y-m-d H:i:s', strtotime($break_s_str . ' +1 day'));
            if ($first_shift_stime_str > $break_e_str || $break_s_str >= $break_e_str) $break_e_str = date('Y-m-d H:i:s', strtotime($break_e_str . ' +1 day'));

            $break_s_millis = strtotime($break_s_str);
            $break_e_millis = strtotime($break_e_str);

            $total_break_seconds += ($break_e_millis - $break_s_millis);

            if ($current_millis >= $break_e_millis) {
                $total_passed_break_seconds += ($break_e_millis - $break_s_millis);
            } elseif ($current_millis > $break_s_millis && $current_millis < $break_e_millis) {
                $total_passed_break_seconds += ($current_millis - $break_s_millis);
            }
        }
    }
    $metrics['passed_break_seconds'] = $total_passed_break_seconds;

    // --- 전체 시간 요약 계산 ---
    $total_work_seconds = strtotime($current_shift_info['work_etime']) - $work_start_millis;
    $metrics['total_work_minutes'] = round($total_work_seconds / 60);
    $metrics['total_break_minutes'] = round($total_break_seconds / 60);
    $metrics['net_work_minutes'] = $metrics['total_work_minutes'] - $metrics['total_break_minutes'];
    $metrics['actual_passed_work_seconds'] = $passed_work_seconds - $total_passed_break_seconds;

    return $metrics;
}

/**
 * 초를 "Hh Mm Ss" 형식의 문자열로 변환합니다.
 *
 * @param integer $seconds 변환할 초
 * @return string 형식화된 시간 문자열
 */
function secondsToHis(int $seconds): string
{
    $h = floor($seconds / 3600);
    $m = floor(($seconds % 3600) / 60);
    $s = $seconds % 60;
    return sprintf('%dh %dm %ds', $h, $m, $s);
}
