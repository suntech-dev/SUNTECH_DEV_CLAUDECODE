<?php
/**
 * proc/set_work_time_insert.php — 근무 시간 설정 저장/수정/로드 처리
 *
 * operation 파라미터에 따라 동작 분기:
 *   (없음)  : idx가 있으면 기존 설정 데이터 로드, oneday가 있으면 해당 날짜 shift 로드
 *   Add     : 기간(Period) 타입 추가 (kind=1)
 *   AddW    : 요일(Week) 타입 추가 (kind=2)
 *   AddD    : 일별(Day) 타입 추가 (kind=3)
 *   Edit    : 기간(Period) 타입 수정
 *   EditW   : 요일(Week) 타입 수정
 *   EditD   : 일별(Day) 타입 수정
 *
 * Shift 구조:
 *   info_work_time: 근무 설정 헤더 (기간, 공장/라인, kind, week_yn 등)
 *   info_work_time_shift: Shift별 시간 상세 (shift_idx 1~3, available_stime, planned1~5 시간)
 *   - available_stime/etime: 근무 가능 시간 (총 근무 시간)
 *   - planned1~5_stime/etime: 계획 휴식/정지 시간 (OEE 가용성 계산에 사용)
 *   - over_time: 연장 근무 여부
 */
// CSG version (2023.03.09 Start. dev@suntech.asia)
require_once(__DIR__ . '/../../../lib/db.php');
require_once(__DIR__ . '/../../../lib/worktime_common.php');

// 요청 시각 기반 날짜/시간 (REQUEST_TIME: 요청 수신 시각, 처리 중 시간 변동 방지)
$now_date  = date('Y-m-d', $_SERVER['REQUEST_TIME']);
$now_time  = date('H:i:s', $_SERVER['REQUEST_TIME']);
$date_time = $now_date . ' ' . $now_time;

// POST 파라미터 수신 및 기본값 설정
$idx          = isset($_POST['idx'])          ? trim($_POST['idx'])          : '';
$oneday       = isset($_POST['oneday'])       ? trim($_POST['oneday'])       : '';
$operation    = isset($_POST['operation'])    ? trim($_POST['operation'])    : '';
$factory_idx  = isset($_POST['factory_idx'])  ? trim($_POST['factory_idx'])  : '';
$line_idx     = isset($_POST['line_idx'])     ? trim($_POST['line_idx'])     : '';
$status       = isset($_POST['status'])       ? trim($_POST['status'])       : '';
$remark       = isset($_POST['remark'])       ? trim($_POST['remark'])       : '';
$period       = isset($_POST['period'])       ? trim($_POST['period'])       : '';

// Shift 시간 배열: 인덱스 = shift_idx (1~3), 값 = 'HH:MM' 형식 시간 문자열
$week            = isset($_POST['week'])            ? $_POST['week']            : [];
$available_stime = isset($_POST['available_stime']) ? $_POST['available_stime'] : [];
$available_etime = isset($_POST['available_etime']) ? $_POST['available_etime'] : [];

// 계획 정지 시간: planned1~5 각각 shift별 배열로 수신
$planned1_stime = isset($_POST['planned1_stime']) ? $_POST['planned1_stime'] : [];
$planned2_stime = isset($_POST['planned2_stime']) ? $_POST['planned2_stime'] : [];
$planned3_stime = isset($_POST['planned3_stime']) ? $_POST['planned3_stime'] : [];
$planned4_stime = isset($_POST['planned4_stime']) ? $_POST['planned4_stime'] : [];
$planned5_stime = isset($_POST['planned5_stime']) ? $_POST['planned5_stime'] : [];
$planned1_etime = isset($_POST['planned1_etime']) ? $_POST['planned1_etime'] : [];
$planned2_etime = isset($_POST['planned2_etime']) ? $_POST['planned2_etime'] : [];
$planned3_etime = isset($_POST['planned3_etime']) ? $_POST['planned3_etime'] : [];
$planned4_etime = isset($_POST['planned4_etime']) ? $_POST['planned4_etime'] : [];
$planned5_etime = isset($_POST['planned5_etime']) ? $_POST['planned5_etime'] : [];
$over_time      = isset($_POST['over_time'])      ? $_POST['over_time']      : [];

// ---------------------------------------------------------------
// 데이터 로딩 (operation 값이 없을 때: 수정 폼 데이터 조회)
// ---------------------------------------------------------------
if (!$operation) {
    if ($idx) {
        // idx로 기존 근무 시간 설정 단건 조회
        $stmt = $pdo->prepare("SELECT * FROM `info_work_time` WHERE idx=? LIMIT 1");
        $stmt->execute([$idx]);
        $result = $stmt->fetch(PDO::FETCH_ASSOC);

        if (!$result) jsonFail('Data not found');

        // Shift 데이터 추가 조회 (shift_idx 1~3 순서)
        $result['shift'] = [];

        $stmt = $pdo->prepare("SELECT * FROM `info_work_time_shift` WHERE work_time_idx=? ORDER BY shift_idx");
        $stmt->execute([$result['idx']]);
        $shifts = $stmt->fetchAll(PDO::FETCH_ASSOC);

        // shift_idx를 키로 하는 연관 배열로 재구성
        foreach ($shifts as $row) {
            $result['shift'][$row['shift_idx']] = $row;
        }

        // shift_idx 1~3 중 DB에 없는 shift는 빈 기본값으로 채움 (폼 초기화 오류 방지)
        for ($i = 1; $i <= 3; $i++) {
            if (!isset($result['shift'][$i])) {
                $result['shift'][$i] = buildEmptyShift();
            } elseif ($result['shift'][$i]['over_time'] == '0') {
                // over_time=0은 연장 없음 → 빈 문자열로 변환 (UI에서 빈 값 표시)
                $result['shift'][$i]['over_time'] = '';
            }
        }
        echo json_encode($result);
        exit();

    } elseif ($oneday) {
        // 특정 날짜(YYYY-MM-DD)의 Day 타입 shift 조회 (일별 캘린더 클릭 시)
        if (!preg_match("/^([0-9]{4})-([0-9]{2})-([0-9]{2})$/", $oneday)) jsonFail('Invalid date format');
        if (!$factory_idx || $factory_idx == '-1') jsonFail('There is no building');
        if (!$line_idx || $line_idx == '-1') jsonFail('There is no line');

        // Worktime 라이브러리로 해당 날짜의 Day shift 조회
        require_once(__DIR__ . '/../../../lib/worktime.lib.php');
        $worktime = new Worktime($pdo);
        $result   = $worktime->getDayShift($oneday, $factory_idx, $line_idx);

        // Day 타입(kind=3) 설정이 없으면 신규 등록용 기본값 구성
        if (!$result || $result['kind'] != '3') {
            $result = [
                'idx'         => '',
                'factory_idx' => $factory_idx,
                'line_idx'    => $line_idx,
                'work_sdate'  => $oneday,
                'work_edate'  => $oneday,
                'kind'        => '3',
                'week_yn'     => '0000000',  // Day 타입은 요일 설정 불필요 → 전부 0
                'status'      => 'Y',
                'remark'      => '',
                'reg_date'    => '',
                'update_date' => '',
            ];
        }

        // shift 1~3 중 없는 항목은 빈 기본값으로 채움
        for ($i = 1; $i <= 3; $i++) {
            if (!isset($result['shift'][$i])) {
                $result['shift'][$i] = buildEmptyShift();
            } elseif ($result['shift'][$i]['over_time'] == '0') {
                $result['shift'][$i]['over_time'] = '';
            }
        }
        echo json_encode($result);
        exit();

    } else {
        jsonFail('Incorrect use');
    }
}

// ---------------------------------------------------------------
// 입력값 검사 (operation이 있는 경우: ADD/EDIT 처리 전 공통 검증)
// ---------------------------------------------------------------
if ($factory_idx == '-1') jsonFail('There is no building');
if ($line_idx == '-1')    jsonFail('There is no line');

// factory_idx/line_idx가 없으면 0으로 설정 (전체 공장/라인 적용)
if (!$factory_idx) $factory_idx = '0';
if (!$line_idx)    $line_idx    = '0';

if (!$period) jsonFail('Please enter the period');
// Shift 1의 available_stime/etime은 필수 (나머지 shift는 선택)
if (!$available_stime[1] || !$available_etime[1]) jsonFail('Please enter available time');

// period 파싱: "YYYY-MM-DD ~ YYYY-MM-DD" 형식에서 시작/종료 날짜 추출
$period_arr = explode('~', $period);
if (count($period_arr) != 2) jsonFail('Error entering the period');

$sdate = trim($period_arr[0]);
$edate = trim($period_arr[1]);

// 모든 시간 값 유효성 검사: HH:MM 형식, 범위 검사 (00:00~23:59)
// 24:00은 00:00으로 변환 (자정 처리)
foreach ($available_stime as $k => $v) {
    $available_stime[$k] = timeCheck($available_stime[$k]);
    $available_etime[$k] = timeCheck($available_etime[$k]);
    $planned1_stime[$k]  = timeCheck($planned1_stime[$k]);
    $planned1_etime[$k]  = timeCheck($planned1_etime[$k]);
    $planned2_stime[$k]  = timeCheck($planned2_stime[$k]);
    $planned2_etime[$k]  = timeCheck($planned2_etime[$k]);
    $planned3_stime[$k]  = timeCheck($planned3_stime[$k]);
    $planned3_etime[$k]  = timeCheck($planned3_etime[$k]);
    $planned4_stime[$k]  = timeCheck($planned4_stime[$k]);
    $planned4_etime[$k]  = timeCheck($planned4_etime[$k]);
    $planned5_stime[$k]  = timeCheck($planned5_stime[$k]);
    $planned5_etime[$k]  = timeCheck($planned5_etime[$k]);
}

// ---------------------------------------------------------------
// ADD (신규 등록)
// ---------------------------------------------------------------
if ($operation === 'Add' || $operation === 'AddW' || $operation === 'AddD') {
    $week_yn = '';
    $kind    = '';

    if ($operation === 'AddW') {
        // 요일 타입: week 배열을 7자리 이진 문자열로 변환 (예: 월~금 → '0111110')
        $week_yn = buildWeekYn($week);
        $kind    = '2';
    } elseif ($operation === 'AddD') {
        // 일별 타입: 요일 설정 없음 → week_yn = '0000000'
        $week_yn = '0000000';
        $kind    = '3';
    } else {
        // 기간 타입: 요일 설정 없음 → week_yn = '0000000'
        $week_yn = '0000000';
        $kind    = '1';
    }

    // info_work_time 헤더 레코드 INSERT (named placeholder 사용)
    $stmt = $pdo->prepare(
        "INSERT INTO `info_work_time`
            (factory_idx, line_idx, work_sdate, work_edate, kind, status, remark, reg_date, update_date, week_yn)
         VALUES
            (:factory_idx, :line_idx, :work_sdate, :work_edate, :kind, :status, :remark, :reg_date, :update_date, :week_yn)"
    );
    $stmt->execute([
        ':factory_idx' => $factory_idx,
        ':line_idx'    => $line_idx,
        ':work_sdate'  => $sdate,
        ':work_edate'  => $edate,
        ':kind'        => $kind,
        ':status'      => $status,
        ':remark'      => $remark,
        ':reg_date'    => $date_time,
        ':update_date' => $date_time,
        ':week_yn'     => $week_yn,
    ]);
    // 방금 INSERT한 work_time의 idx를 shift INSERT에 사용
    $last_id = $pdo->lastInsertId();

    // Shift 상세 데이터 INSERT (shift_idx 1~3)
    insertShifts($pdo, $last_id, $available_stime, $available_etime,
        $planned1_stime, $planned1_etime, $planned2_stime, $planned2_etime,
        $planned3_stime, $planned3_etime, $planned4_stime, $planned4_etime,
        $planned5_stime, $planned5_etime, $over_time);

    jsonSuccess('Data Inserted');

// ---------------------------------------------------------------
// EDIT (수정)
// ---------------------------------------------------------------
} elseif ($operation === 'Edit' || $operation === 'EditW' || $operation === 'EditD') {
    if (!$idx) jsonFail('Enter the idx value');

    // 요일/일별/기간 타입에 따라 week_yn 값 결정
    $week_yn = '';
    if ($operation === 'EditW') {
        // 요일 타입: 새로운 요일 선택값으로 week_yn 재구성
        $week_yn = buildWeekYn($week);
    } elseif ($operation === 'EditD') {
        $week_yn = '0000000';
    } else {
        $week_yn = '0000000';
    }

    // info_work_time 헤더 레코드 UPDATE
    $stmt = $pdo->prepare(
        "UPDATE `info_work_time`
         SET factory_idx = ?, line_idx = ?, work_sdate = ?, work_edate = ?, status = ?, remark = ?, update_date = ?, week_yn = ?
         WHERE idx = ?"
    );
    $result = $stmt->execute([$factory_idx, $line_idx, $sdate, $edate, $status, $remark, $date_time, $week_yn, $idx]);
    if (!$result) jsonFail('Failed to update data');

    // Shift 상세 데이터 UPSERT (존재하면 UPDATE, 없으면 INSERT)
    upsertShifts($pdo, $idx, $available_stime, $available_etime,
        $planned1_stime, $planned1_etime, $planned2_stime, $planned2_etime,
        $planned3_stime, $planned3_etime, $planned4_stime, $planned4_etime,
        $planned5_stime, $planned5_etime, $over_time);

    jsonSuccess('Data Updated');

} else {
    jsonFail('Incorrect use');
}


// ---------------------------------------------------------------
// 헬퍼 함수
// ---------------------------------------------------------------

/**
 * 빈 shift 기본값 반환
 * - DB에 shift 레코드가 없는 경우 폼 초기화용 빈 구조체 반환
 * - 모든 시간 필드를 빈 문자열로 초기화하여 UI에서 빈 입력란으로 표시
 */
function buildEmptyShift(): array {
    return [
        'available_stime' => '', 'available_etime' => '',
        'planned1_stime'  => '', 'planned1_etime'  => '',
        'planned2_stime'  => '', 'planned2_etime'  => '',
        'planned3_stime'  => '', 'planned3_etime'  => '',
        'planned4_stime'  => '', 'planned4_etime'  => '',
        'planned5_stime'  => '', 'planned5_etime'  => '',
        'over_time'       => '',
    ];
}

/**
 * 요일 선택 배열 → week_yn 문자열 변환
 * - $week: ['0'=>'Y', '1'=>'N', ...] 형태의 배열 (0=일요일 ~ 6=토요일)
 * - 결과: '0111110' 형태 7자리 이진 문자열 (1=선택된 요일, 0=미선택)
 * - 아무 요일도 선택하지 않으면 오류 반환 (최소 1요일 필수)
 */
function buildWeekYn(array $week): string {
    $week_yn = '';
    $cnt     = 0;
    for ($i = 0; $i < 7; $i++) {
        if (isset($week[$i]) && $week[$i] === 'Y') {
            $week_yn .= '1';
            $cnt++;
        } else {
            $week_yn .= '0';
        }
    }
    // 최소 1개 요일 선택 강제
    if ($cnt === 0) jsonFail('Choose the day of the day of the week');
    return $week_yn;
}

/**
 * Shift 데이터 INSERT (신규 등록용)
 * - available_stime 배열 인덱스를 shift_idx로 사용 (인덱스 = 1, 2, 3)
 * - over_time: 빈 값이나 없는 경우 0으로 저장 (NULL 방지)
 * - available_second, planned1~3_second, work_minute: 초기값 0
 *   (이후 배치/워커 프로세스에서 실제 초/분으로 갱신됨)
 * - named placeholder 사용으로 긴 파라미터 목록 가독성 향상
 */
function insertShifts(
    PDO $pdo, $work_time_idx,
    array $available_stime, array $available_etime,
    array $p1s, array $p1e, array $p2s, array $p2e,
    array $p3s, array $p3e, array $p4s, array $p4e,
    array $p5s, array $p5e, array $over_time
): void {
    $stmt = $pdo->prepare(
        "INSERT INTO `info_work_time_shift`
            (work_time_idx, shift_idx, available_stime, available_etime,
             planned1_stime, planned1_etime, planned2_stime, planned2_etime,
             planned3_stime, planned3_etime, planned4_stime, planned4_etime,
             planned5_stime, planned5_etime,
             over_time, available_second, planned1_second, planned2_second, planned3_second, work_minute)
         VALUES
            (:work_time_idx, :shift_idx, :available_stime, :available_etime,
             :planned1_stime, :planned1_etime, :planned2_stime, :planned2_etime,
             :planned3_stime, :planned3_etime, :planned4_stime, :planned4_etime,
             :planned5_stime, :planned5_etime,
             :over_time, 0, 0, 0, 0, 0)"
    );
    foreach ($available_stime as $index => $value) {
        // over_time: 값이 없거나 빈 문자열이면 0으로 저장
        $ot = (!isset($over_time[$index]) || !$over_time[$index]) ? 0 : $over_time[$index];
        $stmt->execute([
            ':work_time_idx'   => $work_time_idx,
            ':shift_idx'       => $index,
            ':available_stime' => $available_stime[$index],
            ':available_etime' => $available_etime[$index],
            ':planned1_stime'  => $p1s[$index], ':planned1_etime' => $p1e[$index],
            ':planned2_stime'  => $p2s[$index], ':planned2_etime' => $p2e[$index],
            ':planned3_stime'  => $p3s[$index], ':planned3_etime' => $p3e[$index],
            ':planned4_stime'  => $p4s[$index], ':planned4_etime' => $p4e[$index],
            ':planned5_stime'  => $p5s[$index], ':planned5_etime' => $p5e[$index],
            ':over_time'       => $ot,
        ]);
    }
}

/**
 * Shift 데이터 UPSERT (수정용: 존재하면 UPDATE, 없으면 INSERT)
 * - 수정 시 shift 수가 변경될 수 있어 INSERT ON DUPLICATE 대신 체크 후 분기
 * - check_stmt: work_time_idx + shift_idx로 기존 레코드 존재 확인
 * - 기존 있으면: UPDATE (idx로 정확히 해당 레코드만 수정)
 * - 기존 없으면: INSERT (새 shift 추가, available_second 등 계산 컬럼은 0으로 초기화)
 * - update 실패 시 jsonFail로 즉시 오류 반환
 */
function upsertShifts(
    PDO $pdo, $work_time_idx,
    array $available_stime, array $available_etime,
    array $p1s, array $p1e, array $p2s, array $p2e,
    array $p3s, array $p3e, array $p4s, array $p4e,
    array $p5s, array $p5e, array $over_time
): void {
    // 존재 확인용 prepared statement (shift별로 재사용)
    $check_stmt  = $pdo->prepare("SELECT idx FROM `info_work_time_shift` WHERE work_time_idx=? AND shift_idx=? LIMIT 1");
    $update_stmt = $pdo->prepare(
        "UPDATE `info_work_time_shift`
         SET available_stime=?, available_etime=?,
             planned1_stime=?, planned1_etime=?, planned2_stime=?, planned2_etime=?,
             planned3_stime=?, planned3_etime=?, planned4_stime=?, planned4_etime=?,
             planned5_stime=?, planned5_etime=?, over_time=?
         WHERE idx=?"
    );
    $insert_stmt = $pdo->prepare(
        "INSERT INTO `info_work_time_shift`
            (work_time_idx, shift_idx, available_stime, available_etime,
             planned1_stime, planned1_etime, planned2_stime, planned2_etime,
             planned3_stime, planned3_etime, planned4_stime, planned4_etime,
             planned5_stime, planned5_etime,
             over_time, available_second, planned1_second, planned2_second, planned3_second, work_minute)
         VALUES
            (:work_time_idx, :shift_idx, :available_stime, :available_etime,
             :planned1_stime, :planned1_etime, :planned2_stime, :planned2_etime,
             :planned3_stime, :planned3_etime, :planned4_stime, :planned4_etime,
             :planned5_stime, :planned5_etime,
             :over_time, 0, 0, 0, 0, 0)"
    );

    foreach ($available_stime as $index => $value) {
        $ot = (!isset($over_time[$index]) || !$over_time[$index]) ? 0 : $over_time[$index];

        // 해당 shift_idx의 기존 레코드 확인
        $check_stmt->execute([$work_time_idx, $index]);
        $existing = $check_stmt->fetch(PDO::FETCH_ASSOC);

        if ($existing) {
            // 기존 레코드 있음: idx 기준으로 UPDATE
            $result = $update_stmt->execute([
                $available_stime[$index], $available_etime[$index],
                $p1s[$index], $p1e[$index], $p2s[$index], $p2e[$index],
                $p3s[$index], $p3e[$index], $p4s[$index], $p4e[$index],
                $p5s[$index], $p5e[$index], $ot,
                $existing['idx'],  // WHERE idx= 조건에 기존 레코드 idx 사용
            ]);
            if (!$result) jsonFail('Failed to update shift-' . $index . ' data');
        } else {
            // 기존 레코드 없음: 새 shift INSERT
            $insert_stmt->execute([
                ':work_time_idx'   => $work_time_idx,
                ':shift_idx'       => $index,
                ':available_stime' => $available_stime[$index],
                ':available_etime' => $available_etime[$index],
                ':planned1_stime'  => $p1s[$index], ':planned1_etime' => $p1e[$index],
                ':planned2_stime'  => $p2s[$index], ':planned2_etime' => $p2e[$index],
                ':planned3_stime'  => $p3s[$index], ':planned3_etime' => $p3e[$index],
                ':planned4_stime'  => $p4s[$index], ':planned4_etime' => $p4e[$index],
                ':planned5_stime'  => $p5s[$index], ':planned5_etime' => $p5e[$index],
                ':over_time'       => $ot,
            ]);
        }
    }
}

/**
 * 시간 문자열 유효성 검사 및 정규화
 * - 빈 문자열: 그대로 반환 (선택적 시간 필드)
 * - '24:00': '00:00'으로 변환 (자정 표현 정규화)
 * - HH:MM 형식 검증 (정규식)
 * - 시(0~23), 분(0~59) 범위 검사
 * - 오류 시 jsonFail()로 즉시 응답 종료
 */
function timeCheck(string $time = ''): string {
    if ($time === '24:00') return '00:00';  // 자정 → 00:00 변환
    if ($time) {
        // HH:MM 형식 검증 (두 자리 숫자 : 두 자리 숫자)
        if (!preg_match("/^([0-9]{2})\:([0-9]{2})$/", $time)) jsonFail('Time format is invalid');
        [$hour, $min] = explode(':', $time);
        $hour = (int)$hour;
        $min  = (int)$min;
        if ($hour < 0 || $hour > 23) jsonFail('Time input is incorrect');
        if ($min  < 0 || $min  > 59) jsonFail('Time input is incorrect');
    }
    return $time;
}
