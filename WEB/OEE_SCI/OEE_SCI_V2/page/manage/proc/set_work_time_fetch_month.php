<?php
/**
 * proc/set_work_time_fetch_month.php — 월별 근무 시간 캘린더 HTML 생성
 *
 * info_worktime_2.php의 getMonth() 함수에서 POST로 호출됨
 * 선택한 공장/라인/연도/월에 해당하는 근무 시간 설정을 읽어
 * HTML <table> 캘린더를 생성하여 JSON 응답 내 'html' 키로 반환
 *
 * 파라미터 (POST):
 *   year        : 조회 연도 (기본: 현재 연도)
 *   month       : 조회 월 1~12 (기본: 현재 월)
 *   factory_id  : 공장 idx (없으면 전체)
 *   line_id     : 라인 idx (없으면 전체)
 *
 * 근무 시간 종류 (kind):
 *   1 (P) = Period : 지정 기간 동안 매일 적용
 *   2 (W) = Week   : 지정 기간 중 특정 요일에만 적용 (week_yn 7자리 플래그)
 *   3 (D) = Day    : 특정 날짜 1일에만 적용
 *
 * 캘린더 셀 클래스:
 *   .work_time_idx_{idx} : 해당 근무 시간 설정 idx (클릭 시 수정 폼 연결)
 *   .today               : 오늘 날짜 셀 강조
 *
 * 응답 JSON:
 *   { msg, year, month, factory_id, line_id, first_date, last_date, work_time[], html }
 */
require_once(__DIR__ . '/../../../lib/db.php');
require_once(__DIR__ . '/../../../lib/worktime_common.php');

// 요청 수신 시각 기준 오늘 날짜 (인도네시아 형식 + ISO 형식 2종 생성)
$now_date = date('d-m-Y', $_SERVER['REQUEST_TIME']);  // indonesia date type
$now_year_month = date('Y-m-d', $_SERVER['REQUEST_TIME']);

## Read value
// POST 파라미터 수신: 연도/월/공장/라인 (없으면 현재 날짜값 사용)
$year  = isset($_POST['year']) ? $_POST['year'] : date('Y');
$month = isset($_POST['month']) ? $_POST['month'] : date('n');

$factory_id = isset($_POST['factory_id']) ? $_POST['factory_id'] : '';
$line_id   = isset($_POST['line_id']) ? $_POST['line_id'] : '';

if (empty($factory_id) || empty($line_id)) {
    //   jsonFail('Invalid data');
}

$year = (int)$year;
$month = (int)$month;

// 월 범위 유효성 검사 (1~12)
if ($month < 1 || $month > 12) {
    jsonFail('Invalid month data');
}

## 배열 초기화
// 월 이름 배열 (1-based: [0]='None' 더미, [1]='January', ..., [12]='December')
$mon_arr = array('None', 'January', 'February', 'March', 'April', 'May', 'June', 'July', 'August', 'September', 'October', 'November', 'December');
// 요일 풀네임/약자 배열 (0-based: [0]=일요일)
$week_arr = array('Sunday', 'Monday', 'Tuesday', 'Wednesday', 'Thursday', 'Friday', 'Saturday');
$week_arr_short = array('Sun', 'Mon', 'Tue', 'Wed', 'Thu', 'Fri', 'Sat');

## 선택달 영문
// 캘린더 헤더에 표시할 월 영문명 (예: "March")
$monthly_name = isset($mon_arr[$month]) ? $mon_arr[$month] : $mon_arr[date('n')];

## 연도-월 : 뒤에 날짜만 붙여서 사용하기 위해 미리 만들어둠.
// 예: "2022-02-" → 이후 sprintf("%02d", $day)와 결합하여 "2022-02-01" 형식 완성
$year_month = $year . '-' . sprintf("%02d-", $month); // 2022-02-

## 첫째날
$first_date = $year_month . '01';                      // 첫째날포맷 : 2022-03-01

$last_day = date('t', strtotime($first_date));      // 마지막날 : 31
$last_date = $year_month . $last_day;               // 마지막날포맷 : 2022-03-31

// date('w'): 1일이 시작되는 요일 인덱스 (0=일요일, 6=토요일) → 캘린더 첫 줄 빈 셀 수
$blank_cnt = date('w', strtotime($first_date));                  // 1일이 시작되는 요일 : 0~6 값 (0:일요일, 6:토요일)


## 시프트 정보를 위해 기본 날짜 정보 세팅
// $days: 1~last_day 인덱스로 각 날짜의 캘린더 표시 정보 저장
// info: 시프트 시간 텍스트, cls: CSS 클래스 문자열, kind: P/W/D 종류 표시 코드
$days = array();

for ($i = 1; $i <= $last_day; $i++) {
    $days[$i] = array('info' => '', 'cls' => '', 'kind' => '');
}

$work_time = array();

## 해당 달의 Worktime 전체 가져오기 (선택한 Factory와 Line 의값만 가져온다)
// 조건:
//   status='Y': 활성 설정만
//   factory/line 조합 우선순위: (공장+라인 정확히 일치) OR (공장=0, 전체 공장) OR (공장 일치, 라인=0 전체 라인)
//   기간 겹침 조건: work_sdate <= 이번달 마지막날 AND work_edate >= 이번달 첫날
//   ORDER BY kind ASC: Period(1) → Week(2) → Day(3) 순으로 적용 (Day가 가장 높은 우선순위)
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
  ORDER BY kind ASC, reg_date ASC"
);

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

    // 날짜 문자열을 연-월-일 배열로 분리 (예: "2022-03-15" → [2022, 03, 15])
    $sdate = explode('-', $row['work_sdate']);
    $edate = explode('-', $row['work_edate']);

    if (count($sdate) != 3 || count($edate) != 3) continue;   // 날짜 형식이 잘못된 경우
    if ($row['kind'] == '2') {     // 기간안에 요일별 세팅
        if (strlen($row['week_yn']) != 7) continue;   // 요일별 플래그 7개가 아니면 잘못된 경우
    }

    // 시작일 계산: 설정 시작이 이번 달 이전이면 → 1일부터 표시
    if ((int)$sdate[0] < $year) $start = 1;         // 해당 Row의 시작되는 날짜 구함
    else if ((int)$sdate[1] < $month) $start = 1;
    else $start = (int)$sdate[2];

    // 종료일 계산: 설정 종료가 이번 달 이후이면 → 말일까지 표시
    if ((int)$edate[0] > $year) $end = $last_day;   // 해당 Row의 끝나는 날짜 구함
    else if ((int)$edate[1] > $month) $end = $last_day;
    else $end = (int)$edate[2];


    ## shift time 가져오기
    // 해당 근무 시간 설정(work_time_idx)에 속한 시프트(Shift) 1~3개 조회
    $stmt = $pdo->prepare(
        "SELECT idx, shift_idx, available_stime, available_etime, over_time
    FROM `info_work_time_shift`
    WHERE
      work_time_idx = :work_time_idx
    ORDER BY shift_idx ASC"
    );
    $stmt->execute(array(
        ':work_time_idx' => $row['idx']
    ));
    $shift_time = $stmt->fetchAll(PDO::FETCH_ASSOC);


    ## 해당일의 텍스트를 만든다.
    // 캘린더 셀 안에 표시할 시프트 시간 텍스트 생성
    // 예: "S-1 / 08:00 ~ 17:00<br />(+30m)<br />"
    $info = '';
    foreach ($shift_time as $row2) {
        if ($row2['available_stime'] && $row2['available_etime']) {
            // S-{시프트번호} / 시작시간 ~ 종료시간 형식 표시
            $info .= "S-{$row2['shift_idx']} / {$row2['available_stime']} ~ {$row2['available_etime']}<br />";

            // 연장 근무 시간이 있으면 (+분) 형식으로 추가 표시
            if ($row2['over_time'] && $row2['over_time'] > 0) {
                $info .= "<span class='overtime'>(+{$row2['over_time']}m)</span><br />";
            }
        }
    }


    ## 날짜별 텍스트 데이터 세팅
    // kind별로 $days 배열의 해당 날짜(들)에 info/kind/cls 적용

    if ($row['kind'] == '1') {              // 기간 세팅
        // Period: start~end 범위의 모든 날짜에 동일한 설정 적용
        for ($i = $start; $i <= $end; $i++) {
            $days[$i]['info'] = $info;
            $days[$i]['kind'] = 'P';
            $days[$i]['cls'] = 'work_time_idx_' . $row['idx'];
        }
    } else if ($row['kind'] == '3') {       // 날짜 세팅
        // Day: 특정 날짜($start) 1일에만 적용
        $days[$start]['info'] = $info;
        $days[$start]['kind'] = 'D';
        $days[$start]['cls'] = 'work_time_idx_' . $row['idx'];
    } else if ($row['kind'] == '2') {       // 기간안에 요일별 세팅
        // Week: start~end 범위에서 week_yn[요일인덱스]='1'인 날짜에만 적용
        $tmp_date = $year_month . sprintf("%02d", $start);      // 현재 데이터의 시작날짜 : 2022-01-08
        $tmp_week = date('w', strtotime($tmp_date));          // 현재 데이터의 시작날짜 요일 : 0~6값 (0:일요일, 6:토요일)

        // week_yn 7자리 문자열을 인덱스 배열로 변환 (예: "1100101" → ['0'=>'1','1'=>'1','2'=>'0',...])
        $w_flag = array();

        for ($i = 0; $i < 7; $i++) {
            $w_flag[$i] = substr($row['week_yn'], $i, 1);
        }
        for ($i = $start; $i <= $end; $i++) {
            // 해당 날짜의 요일이 w_flag에서 '1'이면 적용
            if (isset($w_flag[$tmp_week]) && $w_flag[$tmp_week] == '1') {
                $days[$i]['info'] = $info;
                $days[$i]['kind'] = 'W';
                $days[$i]['cls'] = 'work_time_idx_' . $row['idx'];
            }
            // 요일 순환: 토요일(6) 다음은 일요일(0)
            $tmp_week = ($tmp_week >= 6) ? 0 : $tmp_week + 1;
        }
    }
}

// 캘린더 HTML 생성 시작
// 구조: fc-toolbar(네비게이션) + <table id="calendar">(7열 캘린더)
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

// 요일 헤더 행: Sun~Sat 약자 표시
foreach ($week_arr_short as $row) {
    $html .= "<th>{$row}</th>";
}

$html .= "
    </tr>
  </thead>
  <tbody>
    <tr>";

$day = 1;

// 첫줄 빈칸: 1일 이전 요일 위치에 해당하는 빈 셀 삽입
for ($i = 0; $i < $blank_cnt; $i++) {
    $html .= "<td>&nbsp;</td>";
}
// 첫째 주 나머지 날짜 채우기 (blank_cnt 이후 ~ 6번 인덱스까지)
for ($i = $blank_cnt; $i < 7; $i++) {
    $nday = sprintf("%02d", $day);
    // 오늘 날짜면 'today' CSS 클래스 추가
    if ($now_year_month == $year_month . $nday) $days[$day]['cls'] .= ' today';
    // .updateDay: info_worktime_2.php의 클릭 핸들러가 이 class로 수정 모달 트리거
    // date 속성: "YYYY-MM-DD" 형식 (수정 시 날짜 식별에 사용)
    $html .= "<td class='day_cell " . $days[$day]['cls'] . "'><div class='day_name'><a href='javascript:;' class='updateDay' date='" . $year_month . $nday . "'>{$day}</a></div><div class='day_kind'>" . $days[$day]['kind'] . "</div>" . $days[$day]['info'] . "</td>";
    $day++;
}

$html .= "
    </tr>";

// 나머지 주 채우기: $day > $last_day가 될 때까지 7셀씩 행 생성
while (true) {
    $html .= "<tr>";
    for ($i = 0; $i < 7; $i++) {
        $nday = sprintf("%02d", $day);
        // 오늘 날짜이고 유효한 날짜면 'today' 클래스 추가
        if ($now_year_month == $year_month . $nday && isset($days[$day])) $days[$day]['cls'] .= ' today';
        // $day > $last_day: 말일 이후 빈 셀은 &nbsp;로 채움
        $html .= "<td class='day_cell " . ($days[$day]['cls'] ?? '') . "'><div class='day_name'>" . ($day > $last_day ? '&nbsp;' : "<a href='javascript:;' class='updateDay' date='" . $year_month . $nday . "'>" . $day . "</a>") . "</div><div class='day_kind'>" . ($days[$day]['kind'] ?? '') . "</div>" . ($days[$day]['info'] ?? '') . "</td>";
        $day++;
    }
    $html .= "</tr>";
    if ($day > $last_day) break;
}

$html .= "
  </tbody>
</table>";

//$html .= "<pre>".print_r($work_time,true).print_r($days, true)."</pre>";

// 최종 JSON 응답 구성
// html: info_worktime_2.php의 getMonth()에서 $('#monthDiv').html(res.html)로 직접 삽입됨
$result = array(
    'msg' => 'ok',
    'year' => $year,
    'month' => $month,

    'factory_id' => $factory_id,
    'line_id' => $line_id,
    'first_date' => $first_date,
    'last_date' => $last_date,
    'work_time' => $work_time,   // 원시 근무 시간 데이터 (디버깅/추가 처리용)

    'html' => $html              // 캘린더 HTML 문자열
);

// JSON_UNESCAPED_UNICODE: 한글/특수문자를 \uXXXX로 변환하지 않고 그대로 출력
echo json_encode($result, JSON_UNESCAPED_UNICODE);
