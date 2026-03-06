<?php
## CSG version (2023.03.09 Start. dev@suntech.asia)
## Bootstrap 사용 안함. Factory, Zone, Line 적용.
require_once(__DIR__ . '/../../../lib/worktime_database.php');
require_once(__DIR__ . '/../../../lib/worktime_common.php');
// require_once('../lib/database.php');
// require_once('../lib/common.php');

$now_date = date('d-m-Y', $_SERVER['REQUEST_TIME']);  // indonesia date type
$now_year_month = date('Y-m-d', $_SERVER['REQUEST_TIME']);

## Read value
$year  = isset($_POST['year']) ? $_POST['year'] : date('Y');
$month = isset($_POST['month']) ? $_POST['month'] : date('n');

$factory_id = isset($_POST['factory_id']) ? $_POST['factory_id'] : '';
$line_id   = isset($_POST['line_id']) ? $_POST['line_id'] : '';

if (empty($factory_id) || empty($line_id)) {
//   jsonFail('Invalid data');
}

$year = (int)$year;
$month = (int)$month;

if ($month < 1 || $month > 12) {
  jsonFail('Invalid month data');
}

## 배열 초기화
$mon_arr = array('None', 'January', 'February', 'March', 'April', 'May', 'June', 'July', 'August', 'September', 'October', 'November', 'December');
$week_arr = array('Sunday', 'Monday', 'Tuesday', 'Wednesday', 'Thursday', 'Friday', 'Saturday');
$week_arr_short = array('Sun', 'Mon', 'Tue', 'Wed', 'Thu', 'Fri', 'Sat');

## 선택달 영문
$monthly_name = isset($mon_arr[$month]) ? $mon_arr[$month] : $mon_arr[date('n')];

## 연도-월 : 뒤에 날짜만 붙여서 사용하기 위해 미리 만들어둠.
$year_month = $year.'-'.sprintf("%02d-", $month); // 2022-02-

## 첫째날
$first_date = $year_month.'01';	                  // 첫째날포맷 : 2022-03-01

$last_day = date('t', strtotime($first_date));	  // 마지막날 : 31
$last_date = $year_month.$last_day;               // 마지막날포맷 : 2022-03-31

$blank_cnt = date('w', strtotime($first_date));		          // 1일이 시작되는 요일 : 0~6 값 (0:일요일, 6:토요일)


## 시프트 정보를 위해 기본 날짜 정보 세팅
$days = array();

for ($i=1; $i<=$last_day; $i++) {
  $days[$i] = array('info'=>'', 'cls'=>'', 'kind'=>'');
}

$work_time = array();

## 해당 달의 Worktime 전체 가져오기 (선택한 Factory와 Line 의값만 가져온다)
$stmt = $pdo->prepare(
    "SELECT idx, work_sdate, work_edate, kind, week_yn
  FROM `info_work_time`
  WHERE
    status = 'Y'
    AND (
      factory_idx = :factory_idx AND line_idx = :line_idx OR
      factory_idx = 0 OR
      factory_idx = :factory_idx AND line_idx = 0
    ) AND
    work_sdate <= :work_edate AND
    work_edate >= :work_sdate
  ORDER BY kind ASC, reg_date ASC");

$stmt->execute(array(
  ':factory_idx' => $factory_id,
  ':line_idx' => $line_id,
  ':work_sdate' => $first_date,
  ':work_edate' => $last_date,
));
$work_time = $stmt->fetchAll(PDO::FETCH_ASSOC);


foreach ($work_time as $row) {

  ## 데이터의 시작날짜와 끝날짜가 현재 달에 맞는지 검사한다.
  ## 달이 넘어가면 현재 날짜의 시작과 끝날짜에 맞추는 작업을 한다.
//  $start = (int)substr($row['work_sdate'], 8, 2);
//  $end = (int)substr($row['work_edate'], 8, 2);

  $sdate = explode('-',$row['work_sdate']);
  $edate = explode('-',$row['work_edate']);

  if (count($sdate)!=3 || count($edate)!=3) continue;   // 날짜 형식이 잘못된 경우
  if ($row['kind']=='2') {     // 기간안에 요일별 세팅
    if (strlen($row['week_yn']) != 7) continue;   // 요일별 플래그 7개가 아니면 잘못된 경우
  }

  if ((int)$sdate[0] < $year) $start = 1;         // 해당 Row의 시작되는 날짜 구함
  else if ((int)$sdate[1] < $month) $start = 1;
  else $start = (int)$sdate[2];

  if ((int)$edate[0] > $year) $end = $last_day;   // 해당 Row의 끝나는 날짜 구함
  else if ((int)$edate[1] > $month) $end = $last_day;
  else $end = (int)$edate[2];


  ## shift time 가져오기
  $stmt = $pdo->prepare(
    "SELECT idx, shift_idx, available_stime, available_etime, over_time
    FROM `info_work_time_shift`
    WHERE
      work_time_idx = :work_time_idx
    ORDER BY shift_idx ASC");
  $stmt->execute(array(
    ':work_time_idx' => $row['idx']
  ));
  $shift_time = $stmt->fetchAll(PDO::FETCH_ASSOC);


  ## 해당일의 텍스트를 만든다.
  $info = '';
  foreach ($shift_time as $row2) {
    if ($row2['available_stime'] && $row2['available_etime']) {
      $info .= "S-{$row2['shift_idx']} / {$row2['available_stime']} ~ {$row2['available_etime']}<br />";

            if ($row2['over_time'] && $row2['over_time'] > 0) {
                $info .= "<span class='overtime'>(+{$row2['over_time']}m)</span><br />";
            }
    }
  }


  ## 날짜별 텍스트 데이터 세팅

  if ($row['kind']=='1') {              // 기간 세팅
    for ($i=$start; $i<=$end; $i++) {
      $days[$i]['info'] = $info;
      $days[$i]['kind'] = 'P';
      $days[$i]['cls'] = 'work_time_idx_'.$row['idx'];
    }

  } else if ($row['kind']=='3') {       // 날짜 세팅
    $days[$start]['info'] = $info;
    $days[$start]['kind'] = 'D';
    $days[$start]['cls'] = 'work_time_idx_'.$row['idx'];

  } else if ($row['kind']=='2') {       // 기간안에 요일별 세팅

    $tmp_date = $year_month . sprintf("%02d", $start);	  // 현재 데이터의 시작날짜 : 2022-01-08
    $tmp_week = date('w', strtotime($tmp_date));		  // 현재 데이터의 시작날짜 요일 : 0~6값 (0:일요일, 6:토요일)

    $w_flag = array();

    for ($i=0; $i<7; $i++) {
      $w_flag[$i] = substr($row['week_yn'], $i, 1);
    }
    for ($i=$start; $i<=$end; $i++) {
      if (isset($w_flag[$tmp_week]) && $w_flag[$tmp_week]=='1') {
        $days[$i]['info'] = $info;
        $days[$i]['kind'] = 'W';
        $days[$i]['cls'] = 'work_time_idx_'.$row['idx'];
      }
      $tmp_week = ($tmp_week >= 6) ? 0 : $tmp_week+1;
    }
  }
}

$html = '
<div>
    <div class="fc-toolbar">
        <div class="fc-toolbar-chunk">
        <!--Current : ' . $year . '-->
        Now Date : ' . $now_date . '
        </div>
        <div class="fc-toolbar-chunk">' . $monthly_name . ' - ' . $year . '</div>
        <div class="fc-toolbar-chunk">
        <div class="btn-group">
            <button type="button" title="Previous month" aria-pressed="false" class="fc-prev-button btn btn-primary mb-2" onclick="prevMonth()"><span class="fa fa-chevron-left"><</span></button>
            <button type="button" title="Next month" aria-pressed="false" class="fc-next-button btn btn-primary mb-2" onclick="nextMonth()"><span class="fa fa-chevron-right">></span></button>
            <button type="button" title="This month" aria-pressed="false" class="fc-today-button btn btn-primary mb-2" onclick="thisMonth()">Today</button>
        </div>
        </div>
    </div>
</div>
<table id="calendar" class="table-bordered">
    <colgroup>
        <col style="width: 14%;">
        <col style="width: 14%;">
        <col style="width: 14%;">
        <col style="width: 14%;">
        <col style="width: 14%;">
        <col style="width: 14%;">
        <col style="width: 14%;">
    </colgroup>
    <thead>
        <tr>';

    // 요일 표시
    foreach ($week_arr_short as $row) {
      $html .= "<th>{$row}</th>";
    }

$html .= "
    </tr>
  </thead>
  <tbody>
    <tr>";

    $day = 1;

    // 첫줄 빈칸
    for($i=0; $i<$blank_cnt; $i++) {
      $html .= "<td>&nbsp;</td>";
    }
    for($i=$blank_cnt; $i<7; $i++) {
      $nday = sprintf("%02d", $day);
      if ($now_year_month==$year_month.$nday) $days[$day]['cls'] .= ' today';
      $html .= "<td class='day_cell ".$days[$day]['cls']."'><div class='day_name'><a href='javascript:;' class='updateDay' date='".$year_month.$nday."'>{$day}</a></div><div class='day_kind'>".$days[$day]['kind']."</div>".$days[$day]['info']."</td>";
      $day++;
    }

$html .= "
    </tr>";

    // 날자 채움
    while(true) {
      $html .= "<tr>";
      for($i=0; $i<7; $i++) {
        $nday = sprintf("%02d", $day);
        if ($now_year_month==$year_month.$nday && isset($days[$day])) $days[$day]['cls'] .= ' today';
        $html .= "<td class='day_cell ".($days[$day]['cls'] ?? '')."'><div class='day_name'>".($day > $last_day ? '&nbsp;':"<a href='javascript:;' class='updateDay' date='".$year_month.$nday."'>".$day."</a>")."</div><div class='day_kind'>".($days[$day]['kind'] ?? '')."</div>".($days[$day]['info'] ?? '')."</td>";
        $day++;
      }
      $html .= "</tr>";
      if ($day > $last_day) break;
    }

$html .= "
  </tbody>
</table>";

//$html .= "<pre>".print_r($work_time,true).print_r($days, true)."</pre>";

$result = array(
  'msg' => 'ok',
  'year' => $year,
  'month' => $month,

    'factory_id' => $factory_id,
    'line_id' => $line_id,
    'first_date' => $first_date,
    'last_date' => $last_date,
    'work_time' => $work_time,

  'html' => $html
);

echo json_encode($result, JSON_UNESCAPED_UNICODE);