<?php
## CSG version (2023.03.09 Start. dev@suntech.asia)
require_once(__DIR__ . '/../../../lib/db.php');
require_once(__DIR__ . '/../../../lib/worktime_common.php');

$now_date = date('Y-m-d', $_SERVER['REQUEST_TIME']);    // korea date type
$now_time = date('H:i:s', $_SERVER['REQUEST_TIME']);
$date_time = $now_date." ".$now_time;

## edit modal values 
//$idx = $_POST['idx'];
$idx          = isset($_POST['idx']) ? trim($_POST['idx']) : '';
$oneday       = isset($_POST['oneday']) ? trim($_POST['oneday']) : '';
$operation    = isset($_POST['operation']) ? trim($_POST['operation']) : '';
$factory_idx  = isset($_POST['factory_idx']) ? trim($_POST['factory_idx']) : '';
$line_idx     = isset($_POST['line_idx']) ? trim($_POST['line_idx']) : '';
$status       = isset($_POST['status']) ? trim($_POST['status']) : '';
$remark       = isset($_POST['remark']) ? trim($_POST['remark']) : '';
$period       = isset($_POST['period']) ? trim($_POST['period']) : '';

$week            = isset($_POST['week']) ? $_POST['week'] : array();
$available_stime = isset($_POST['available_stime']) ? $_POST['available_stime'] : array();
$available_etime = isset($_POST['available_etime']) ? $_POST['available_etime'] : array();

$planned1_stime = isset($_POST['planned1_stime']) ? $_POST['planned1_stime'] : array();
$planned2_stime = isset($_POST['planned2_stime']) ? $_POST['planned2_stime'] : array();
$planned3_stime = isset($_POST['planned3_stime']) ? $_POST['planned3_stime'] : array();
$planned4_stime = isset($_POST['planned4_stime']) ? $_POST['planned4_stime'] : array();
$planned5_stime = isset($_POST['planned5_stime']) ? $_POST['planned5_stime'] : array();
$planned1_etime = isset($_POST['planned1_etime']) ? $_POST['planned1_etime'] : array();
$planned2_etime = isset($_POST['planned2_etime']) ? $_POST['planned2_etime'] : array();
$planned3_etime = isset($_POST['planned3_etime']) ? $_POST['planned3_etime'] : array();
$planned4_etime = isset($_POST['planned4_etime']) ? $_POST['planned4_etime'] : array();
$planned5_etime = isset($_POST['planned5_etime']) ? $_POST['planned5_etime'] : array();
$over_time      = isset($_POST['over_time']) ? $_POST['over_time'] : array();

## operation 값이 비어있으면 데이터를 로딩
if (!$operation) {

  ## idx 값이 있으면 Period, Week 정보중 하나
  ## oneday 값이 있으면 Day 정보
  if ($idx) {
    $stmt = $pdo->prepare("SELECT * FROM `info_work_time` WHERE idx=? LIMIT 1");
    $stmt->execute([$idx]);
    $result = $stmt->fetch(PDO::FETCH_ASSOC);

    if (!$result) jsonFail('Data not found');

    $result['shift'] = array();

  //  $result['period'] = $result['work_sdate'].' ~ '.$result['work_edate'];

    $stmt = $pdo->prepare("SELECT * FROM `info_work_time_shift` WHERE work_time_idx=? ORDER BY shift_idx");
    $stmt->execute([$result['idx']]);
    $shift = $stmt->fetchAll(PDO::FETCH_ASSOC);

    foreach($shift as $row) {
      $result['shift'][$row['shift_idx']] = $row;
    }

    // 로딩된 데이터가 없으면 빈값으로 채운다.
    for ($i=1; $i<=3; $i++) {
      if (!isset($result['shift'][$i])) {
        $result['shift'][$i]['available_stime'] = '';
        $result['shift'][$i]['available_etime'] = '';
        $result['shift'][$i]['planned1_stime'] = '';
        $result['shift'][$i]['planned1_etime'] = '';
        $result['shift'][$i]['planned2_stime'] = '';
        $result['shift'][$i]['planned2_etime'] = '';
        $result['shift'][$i]['planned3_stime'] = '';
        $result['shift'][$i]['planned3_etime'] = '';
        $result['shift'][$i]['planned4_stime'] = '';
        $result['shift'][$i]['planned4_etime'] = '';
        $result['shift'][$i]['planned5_stime'] = '';
        $result['shift'][$i]['planned5_etime'] = '';
        $result['shift'][$i]['over_time'] = '';
      } else {
        if ($result['shift'][$i]['over_time'] == '0') $result['shift'][$i]['over_time'] = '';
      }
    }
    echo json_encode($result);
    exit();

  ## oneday 값이 있으면 Day 정보
  } else if ($oneday) {

    if ( !preg_match("/^([0-9]{4})-([0-9]{2})-([0-9]{2})$/", $oneday) ) jsonFail('Invalid date format');
    if (!$factory_idx || $factory_idx=='-1') jsonFail('There is no building');
    if (!$line_idx || $line_idx=='-1') jsonFail('There is no line');

    require_once(__DIR__ . '/../../../lib/worktime.lib.php');
    $worktime = new Worktime($pdo);

    $result = $worktime->getDayShift($oneday, $factory_idx, $line_idx);

    if (!$result) $result = array();

    ## 데이터 없거나
    ## 데이터가 있을경우 자신의 데이터가 아니고 다른 타입에서 가져왔을수도 있다.
    ## 다른 타입에서 가져온 경우라면 새로 등록해야 한다는 의미이므로 기본값으로 세팅
    if (!$result || $result['kind']!='3') {
        $result['idx'] = '';
        $result['factory_idx'] = $factory_idx;
        $result['line_idx'] = $line_idx;
        $result['work_sdate'] = $oneday;
        $result['work_edate'] = $oneday;
        $result['kind'] = '3';
        $result['week_yn'] = '0000000';
        $result['status'] = 'Y';
        $result['remark'] = '';
        $result['reg_date'] = '';
        $result['update_date'] = '';
    }

    // 로딩된 데이터가 없으면 빈값으로 채운다.
    for ($i=1; $i<=3; $i++) {
      if (!isset($result['shift'][$i])) {
        $result['shift'][$i]['available_stime'] = '';
        $result['shift'][$i]['available_etime'] = '';
        $result['shift'][$i]['planned1_stime'] = '';
        $result['shift'][$i]['planned1_etime'] = '';
        $result['shift'][$i]['planned2_stime'] = '';
        $result['shift'][$i]['planned2_etime'] = '';
        $result['shift'][$i]['planned3_stime'] = '';
        $result['shift'][$i]['planned3_etime'] = '';
        $result['shift'][$i]['planned4_stime'] = '';
        $result['shift'][$i]['planned4_etime'] = '';
        $result['shift'][$i]['planned5_stime'] = '';
        $result['shift'][$i]['planned5_etime'] = '';
        $result['shift'][$i]['over_time'] = '';
      } else {
        if ($result['shift'][$i]['over_time'] == '0') $result['shift'][$i]['over_time'] = '';
      }
    }
    echo json_encode($result);
    exit();

  } else {
    jsonFail('Incorrect use');
  }
}

## 데이터 검사
if ($factory_idx=='-1') jsonFail('There is no building');
if ($line_idx=='-1') jsonFail('There is no line');

if (!$factory_idx) $factory_idx = '0';
if (!$line_idx)    $line_idx = '0';

// 기간 선택검사
if (!$period) jsonFail('Please enter the period');

// Shift-1 은 필수 항목이므로 입력값이 없으면 오류
if (!$available_stime[1] || !$available_etime[1]) jsonFail('Please enter available time');

// 기간에서 시작일, 종료일 분리
$period_arr = explode('~', $period);
if (count($period_arr)!=2) jsonFail('Error entering the period');

$sdate = trim($period_arr[0]);  // 시작일
$edate = trim($period_arr[1]);  // 종료일


// 시간 검사
// 시간형식이 99:99 인지 검사한다.
// 24:00 입력은 00:00 으로 변환한다.
// 시간이 23시 이상이거나 분이 59분 이상이면 오류를 리턴한다.

foreach($available_stime as $k => $v) {
  $available_stime[$k] = timeCheck($available_stime[$k]);
  $available_etime[$k] = timeCheck($available_etime[$k]);
  $planned1_stime[$k] = timeCheck($planned1_stime[$k]);
  $planned1_etime[$k] = timeCheck($planned1_etime[$k]);
  $planned2_stime[$k] = timeCheck($planned2_stime[$k]);
  $planned2_etime[$k] = timeCheck($planned2_etime[$k]);
  $planned3_stime[$k] = timeCheck($planned3_stime[$k]);
  $planned3_etime[$k] = timeCheck($planned3_etime[$k]);
  $planned4_stime[$k] = timeCheck($planned4_stime[$k]);
  $planned4_etime[$k] = timeCheck($planned4_etime[$k]);
  $planned5_stime[$k] = timeCheck($planned5_stime[$k]);
  $planned5_etime[$k] = timeCheck($planned5_etime[$k]);
}


## add modal data insert
## 기간 추가, 기간별 요일 추가
if($operation=="Add" || $operation=="AddW" || $operation=="AddD") {

  $last_id = '';
  $week_yn = '';
  $kind = '';

  if ($operation=="AddW") {
    $cnt = 0;
    for ($i=0; $i<7; $i++) {
      if (isset($week[$i]) && $week[$i]=='Y') {
        $week_yn .= '1'; $cnt++;
      } else {
        $week_yn .= '0';
      }
    }
    if ($cnt==0) {
      jsonFail('Choose the day of the day of the week');
    }
    $kind = '2';

  } else if($operation=="AddD") {
    $week_yn = '0000000';
    $kind = '3';

  } else if($operation=="Add") {
    $week_yn = '0000000';
    $kind = '1';
  }

  ## 레코드 생성
  $stmt = $pdo->prepare(
    "INSERT INTO `info_work_time`
        (factory_idx, line_idx, work_sdate, work_edate, kind, status, remark, reg_date, update_date, week_yn) 
     VALUES
        (:factory_idx, :line_idx, :work_sdate, :work_edate, :kind, :status, :remark, :reg_date, :update_date, :week_yn)");
  $result = $stmt->execute(
    array(
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
    )
  );
  $last_id = $pdo->lastInsertId();


  // Shift 데이터 저장
  foreach($available_stime as $index => $value) {

      $ot = (!isset($over_time[$index]) || !$over_time[$index]) ? 0 : $over_time[$index];

      $stmt = $pdo->prepare(
        "INSERT INTO `info_work_time_shift`
            (work_time_idx, shift_idx, available_stime, available_etime,
             planned1_stime, planned1_etime, planned2_stime, planned2_etime, planned3_stime, planned3_etime, planned4_stime, planned4_etime, planned5_stime, planned5_etime,
             over_time, available_second, planned1_second, planned2_second, planned3_second, work_minute) 
         VALUES
            (:work_time_idx, :shift_idx, :available_stime, :available_etime,
             :planned1_stime, :planned1_etime, :planned2_stime, :planned2_etime, :planned3_stime, :planned3_etime, :planned4_stime, :planned4_etime, :planned5_stime, :planned5_etime,
             :over_time, :available_second, :planned1_second, :planned2_second, :planned3_second, :work_minute)");
      $result = $stmt->execute(
        array(
          ':work_time_idx'   => $last_id,
          ':shift_idx'       => $index,
          ':available_stime' => $available_stime[$index],
          ':available_etime' => $available_etime[$index],
          ':planned1_stime'  => $planned1_stime[$index],
          ':planned1_etime'  => $planned1_etime[$index],
          ':planned2_stime'  => $planned2_stime[$index],
          ':planned2_etime'  => $planned2_etime[$index],
          ':planned3_stime'  => $planned3_stime[$index],
          ':planned3_etime'  => $planned3_etime[$index],
          ':planned4_stime'  => $planned4_stime[$index],
          ':planned4_etime'  => $planned4_etime[$index],
          ':planned5_stime'  => $planned5_stime[$index],
          ':planned5_etime'  => $planned5_etime[$index],
          ':over_time'       => $ot,
          ':available_second' => 0,
          ':planned1_second'  => 0,
          ':planned2_second'  => 0,
          ':planned3_second'  => 0,
          ':work_minute'  => 0,
        )
      );
  }
  jsonSuccess('Data Inserted');

## edit modal data update

} else if($operation=="Edit" || $operation=="EditW" || $operation=="EditD") {

  if(!$idx) jsonFail('Enter the idx value');  // idx 값이 존재하지 않으면 오류

  $week_yn = '';

  if ($operation=="EditW") {
    $cnt = 0;
    for ($i=0; $i<7; $i++) {
      if (isset($week[$i]) && $week[$i]=='Y') {
        $week_yn .= '1'; $cnt++;
      } else {
        $week_yn .= '0';
      }
    }
    if ($cnt==0) {
      jsonFail('Choose the day of the day of the week');
    }

  } else if ($operation=="EditD") {
    $week_yn = '0000000';

  } else if($operation=="Edit") {
    $week_yn = '0000000';
  }

  // 대표정보 수정
  $stmt = $pdo->prepare(
    "UPDATE `info_work_time`
     SET factory_idx = ?, line_idx = ?, work_sdate = ?, work_edate = ?, status = ?, remark = ?, update_date = ?, week_yn = ?
     WHERE idx = ?");
  $result = $stmt->execute(array($factory_idx, $line_idx, $sdate, $edate, $status, $remark, $date_time, $week_yn, $idx));

  if(!$result) jsonFail('Failed to update data');

  // Shift 데이터 저장
  foreach($available_stime as $index => $value) {

    $ot = (!isset($over_time[$index]) || !$over_time[$index]) ? 0 : $over_time[$index];

    $stmt = $pdo->prepare("SELECT idx FROM `info_work_time_shift` WHERE work_time_idx=? AND shift_idx=? LIMIT 1");
    $stmt->execute([$idx, $index]);
    $result = $stmt->fetch(PDO::FETCH_ASSOC);

    if ($result) {  // 존재하면 수정
      $shift_idx = $result['idx'];

      $stmt = $pdo->prepare(
        "UPDATE `info_work_time_shift`
         SET available_stime = ?, available_etime = ?, planned1_stime = ?, planned1_etime = ?, planned2_stime = ?, planned2_etime = ?, planned3_stime = ?, planned3_etime = ?, planned4_stime = ?, planned4_etime = ?, planned5_stime = ?, planned5_etime = ?, over_time = ?
         WHERE idx = ?");

      $result = $stmt->execute(array($available_stime[$index], $available_etime[$index], $planned1_stime[$index], $planned1_etime[$index], $planned2_stime[$index], $planned2_etime[$index], $planned3_stime[$index], $planned3_etime[$index], $planned4_stime[$index], $planned4_etime[$index], $planned5_stime[$index], $planned5_etime[$index], $ot, $shift_idx));

      if(!$result) jsonFail('Failed to update shift-'.$index.' data');

    } else {  // 없으면 추가


      $stmt = $pdo->prepare(
        "INSERT INTO `info_work_time_shift`
            (work_time_idx, shift_idx, available_stime, available_etime,
             planned1_stime, planned1_etime, planned2_stime, planned2_etime, planned3_stime, planned3_etime, planned4_stime, planned4_etime, planned5_stime, planned5_etime,
             over_time, available_second, planned1_second, planned2_second, planned3_second, work_minute) 
         VALUES
            (:work_time_idx, :shift_idx, :available_stime, :available_etime,
             :planned1_stime, :planned1_etime, :planned2_stime, :planned2_etime, :planned3_stime, :planned3_etime, :planned4_stime, :planned4_etime, :planned5_stime, :planned5_etime,
             :over_time, :available_second, :planned1_second, :planned2_second, :planned3_second, :work_minute)");
      $result = $stmt->execute(
        array(
          ':work_time_idx'   => $idx,
          ':shift_idx'       => $index,
          ':available_stime' => $available_stime[$index],
          ':available_etime' => $available_etime[$index],
          ':planned1_stime'  => $planned1_stime[$index],
          ':planned1_etime'  => $planned1_etime[$index],
          ':planned2_stime'  => $planned2_stime[$index],
          ':planned2_etime'  => $planned2_etime[$index],
          ':planned3_stime'  => $planned3_stime[$index],
          ':planned3_etime'  => $planned3_etime[$index],
          ':planned4_stime'  => $planned4_stime[$index],
          ':planned4_etime'  => $planned4_etime[$index],
          ':planned5_stime'  => $planned5_stime[$index],
          ':planned5_etime'  => $planned5_etime[$index],
          ':over_time'       => $ot,
          ':available_second' => 0,
          ':planned1_second'  => 0,
          ':planned2_second'  => 0,
          ':planned3_second'  => 0,
          ':work_minute'  => 0,
        )
      );
    }
  }
  jsonSuccess('Data Updated');

} else {
  jsonFail('Incorrect use');
}

function timeCheck($time='') {
  if ($time == '24:00') return '00:00';
  if ($time) {
    if (!preg_match("/^([0-9]{2})\:([0-9]{2})$/", $time) ) jsonFail('Time format is invalid');
    $tmp = explode(":", $time);
    if (count($tmp) != 2) jsonFail('Time input is incorrect');
    $hour = round($tmp[0]);
    $min = round($tmp[1]);
    if ($hour<0 || $hour>23) jsonFail('Time input is incorrect');
    if ($min<0 || $min>59) jsonFail('Time input is incorrect');
  }
  return $time;
}