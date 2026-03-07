<?php
## CSG version (2023.03.09 Start. dev@suntech.asia)
## Bootstrap 사용 안함. Factory, Zone, Line 적용.
require_once(__DIR__ . '/../../../lib/db.php');
require_once(__DIR__ . '/../../../lib/worktime_common.php');
// require_once('../lib/database.php');
// require_once('../lib/common.php');

## Custom Field value
$searchBystate    = isset($_POST['searchBystate']) ? $_POST['searchBystate'] : '';
$searchByFactory  = isset($_POST['searchByFactory']) ? $_POST['searchByFactory'] : '';
$searchByLine     = isset($_POST['searchByLine']) ? $_POST['searchByLine'] : '';

## Read value
$searchValue      = isset($_POST['search']['value']) ? $_POST['search']['value'] : '';
$columnIndex      = isset($_POST['order'][0]['column']) ? $_POST['order'][0]['column'] : '';
$columnName       = isset($_POST['columns'][$columnIndex]['data']) ? $_POST['columns'][$columnIndex]['data'] : '';
$columnSortOrder  = isset($_POST['order'][0]['dir']) ? $_POST['order'][0]['dir'] : '';

## 선택한 factory와 line의 전체 데이터를 읽어야 하므로 limit 절은 필요없어짐.
//$rowPerPage       = isset($_POST['length']) ? $_POST['length'] : -1; // Rows display per page
//$start            = isset($_POST['start']) ? $_POST['start'] : 0;

$draw             = isset($_POST["draw"]) ? $_POST["draw"] : 1;
$kind             = isset($_POST["kind"]) ? $_POST["kind"] : 0;

## Search
$qry = array();

if ($kind) { $qry[] = "a.kind = '{$kind}'"; }
if ($searchByFactory) { $qry[] = "(a.factory_idx = '{$searchByFactory}' AND a.line_idx = '{$searchByLine}' OR a.factory_idx = 0 OR a.factory_idx = '{$searchByFactory}' AND a.line_idx = 0)"; }
if ($searchBystate) { $qry[] = "a.status = '{$searchBystate}'"; }
if ($searchValue)   { $qry[] = "(a.remark LIKE '%{$searchValue}%')"; }

$where = ($qry) ? ' WHERE '.implode(' AND ', $qry) : '';
$order = ($columnIndex) ? " ORDER BY {$columnName} {$columnSortOrder}" : " ORDER BY kind ASC, reg_date ASC";
//$limit = ($rowPerPage != -1) ? " LIMIT {$start}, {$rowPerPage}" : '';   limit 절은 필요없어짐.
$limit = '';

## Fetch records
## 작업자가 선택한 Factory와 Line의 정보만 가져오는 쿼리로 변경
$stmt = $pdo->prepare(
  "SELECT
      a.idx, concat(work_sdate,' ~ ',work_edate) as period, concat(ifnull(b.factory_name, 'all'),' > ',ifnull(c.line_name, 'all')) as locate, ifnull(b.factory_name, '- ALL -') as factory_name, ifnull(c.line_name, '- ALL -') as line_name, a.kind, a.status, a.remark, LEFT(a.reg_date,10) as reg_date, week_yn
   FROM `info_work_time` AS a
      LEFT JOIN `info_factory` AS b ON a.factory_idx=b.idx
      LEFT JOIN `info_line` AS c ON a.line_idx=c.idx".$where.$order.$limit);

/*
$stmt = $pdo->prepare(
  "SELECT
      a.idx, concat(work_sdate,' ~ ',work_edate) as period, concat(ifnull(b.factory_name, 'all'),' > ',ifnull(c.line_name, 'all')) as locate, ifnull(b.factory_name, '- ALL -') as factory_name, ifnull(c.line_name, '- ALL -') as line_name, a.kind, a.status, a.remark, LEFT(a.reg_date,10) as reg_date, week_yn
   FROM `info_work_time` AS a
      LEFT JOIN `info_factory` AS b ON a.factory_idx=b.idx
      LEFT JOIN `info_line` AS c ON a.line_idx=c.idx".$where.$order.$limit);
*/

$stmt->execute();
$result = $stmt->fetchAll(PDO::FETCH_ASSOC);
$filtered_rows = $stmt->rowCount();  // Total number of records with filtering

$no = $filtered_rows;

if ($kind==2) {   // 요일별

  $week_arr_short = array('Su', 'Mo', 'Tu', 'We', 'Th', 'Fr', 'Sa');

  foreach($result as $row => $value) {
    $result[$row]['no'] = $no--;
    $result[$row]['status'] = $value['status']=='Y' ? 'used':'unused';
    $result[$row]['week_yn_names'] = '';

    $len = strlen($value['week_yn']);

    for ($i=0; $i<$len; $i++) {
      if (substr($value['week_yn'], $i, 1)=='1') {
        $result[$row]['week_yn_names'] .= $week_arr_short[$i].' ';
      } else {
        $result[$row]['week_yn_names'] .= '<span style="color:#6a6a6a">'.$week_arr_short[$i].'</span> ';
      }
    }
    $result[$row]['kind'] = 'Period';
  }

} else if ($kind==0) {    // 기간별

  $week_arr_short = array('Su', 'Mo', 'Tu', 'We', 'Th', 'Fr', 'Sa');

  foreach($result as $row => $value) {
    $result[$row]['no'] = $no--;
    $result[$row]['status'] = $value['status']=='Y' ? 'used':'unused';
    $result[$row]['week_yn_names'] = '';

    if ($value['kind']=='2') {
      $len = strlen($value['week_yn']);

      for ($i=0; $i<$len; $i++) {
        if (substr($value['week_yn'], $i, 1)=='1') {
          $result[$row]['week_yn_names'] .= $week_arr_short[$i].' ';
        } else {
          $result[$row]['week_yn_names'] .= '<span style="color:#6a6a6a">'.$week_arr_short[$i].'</span> ';
        }
      }
      $result[$row]['kind'] = 'Week';

    } else if ($value['kind']=='3') {
      $result[$row]['period'] = substr($value['period'], 0, 10);
      $result[$row]['kind'] = 'Day';

    } else if ($value['kind']=='1') {
      $result[$row]['kind'] = 'Period';

    } else {
      $result[$row]['kind'] = 'None';
    }
  }
}


## Total number of records without filtering
$stmt = $pdo->prepare("SELECT count(*) FROM `info_work_time`");
$stmt->execute();
//$total_all_records = $stmt->fetch(PDO::FETCH_NUM);
$total_all_records = $stmt->fetchColumn();

## Response
$response = array(
  "draw"                  => intval($draw),
  "iTotalRecords"         => $filtered_rows,
  "iTotalDisplayRecords"  => $total_all_records,
  "aaData"                => $result
);

echo json_encode($response);
exit();