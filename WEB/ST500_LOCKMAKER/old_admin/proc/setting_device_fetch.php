<?php
## ST-500 Renewal (2022.02.23 Start. dev@suntech.asia & hamani@naver.com)

require_once ($_SERVER['DOCUMENT_ROOT'] . '/st500/lib/database.php');

## Custom Field value
$searchByCountry = isset($_POST['searchByCountry']) ? $_POST['searchByCountry'] : '';
$searchBystate = isset($_POST['searchBystate']) ? $_POST['searchBystate'] : '';

## Read value
$searchValue = isset($_POST['search']['value']) ? $_POST['search']['value'] : '';
$columnIndex = isset($_POST['order'][0]['column']) ? $_POST['order'][0]['column'] : '';
$columnName = isset($_POST['columns'][$columnIndex]['data']) ? $_POST['columns'][$columnIndex]['data'] : '';
$columnSortOrder = isset($_POST['order'][0]['dir']) ? $_POST['order'][0]['dir'] : '';

$rowPerPage = isset($_POST['length']) ? $_POST['length'] : -1; // Rows display per page
$start = isset($_POST['start']) ? $_POST['start'] : 0;
$draw = isset($_POST["draw"]) ? $_POST["draw"] : 1;

## Date search value
// unused.

## Search
$qry = array();

if ($searchByCountry) {
  $qry[] = "a.country_idx = '{$searchByCountry}'";
}
// if ($searchBystate)   { $qry[] = "a.state = '{$searchBystate}'"; }
if ($searchBystate == 'A') {
  $qry[] = "a.state in ('Y', 'N')";
} else {
  $qry[] = "a.state = '{$searchBystate}'";
}
if ($searchValue) {
  $qry[] = "(a.device_id LIKE '%{$searchValue}%' OR a.name LIKE '%{$searchValue}%')";
}

$where = ($qry) ? ' WHERE ' . implode(' AND ', $qry) : '';
$order = ($columnIndex) ? " ORDER BY {$columnName} {$columnSortOrder}" : " ORDER BY a.idx DESC";
$limit = ($rowPerPage != -1) ? " LIMIT {$start}, {$rowPerPage}" : '';

## Fetch records
// 용우 버전
/*
$stmt = $pdo->prepare(
  "SELECT a.idx, a.device_id, a.name, a.state AS status, a.country_idx, LEFT(a.reg_date,10) as reg_date, LEFT(a.update_date,10) as update_date, IFNULL(b.country_name,'') AS country_name, b.flag_name 
   FROM `st500_device` AS a
   LEFT JOIN `country` AS b ON b.idx = a.country_idx"
  . $where . $order . $limit
);
$stmt->execute();
$result = $stmt->fetchAll(PDO::FETCH_ASSOC);
//$filtered_rows = $stmt->rowCount();  // Total number of records with filtering
*/

// 김이사님 버전
$stmt = $pdo->prepare(
  "SELECT a.idx, a.device_id, a.name, a.state as status, a.country_idx, LEFT(a.reg_date,10) as reg_date, LEFT(a.update_date,10) as update_date, IFNULL(b.country_name,'') AS country_name, b.flag_name 
   FROM `data_smart_device` AS a
   LEFT JOIN `country` AS b ON b.idx = a.country_idx"
  . $where . $order . $limit
);
$stmt->execute();
$result = $stmt->fetchAll(PDO::FETCH_ASSOC);

## Total number of records with filtering
/* $stmt = $pdo->prepare(
  "SELECT a.idx, a.device_id, a.name, a.state AS status, a.country_idx, LEFT(a.reg_date,10) as reg_date, LEFT(a.update_date,10) as update_date, IFNULL(b.country_name,'') AS country_name, b.flag_name 
   FROM `st500_device` AS a
   LEFT JOIN `country` AS b ON b.idx = a.country_idx"
   .$where.$order
  ); */


$stmt = $pdo->prepare(
  "SELECT a.idx FROM `data_smart_device` AS a
  LEFT JOIN `country` AS b ON b.idx = a.country_idx"
  . $where . $order
);
$stmt->execute();
$filtered_rows = $stmt->rowCount();

## Total number of records without filtering

// 용우 버전
/* $stmt = $pdo->prepare("SELECT count(*) FROM `st500_device` WHERE state in ('Y', 'N')");
$stmt->execute();
$total_all_records = $stmt->fetchColumn(); */


// 김이사님 버전
$stmt = $pdo->prepare("SELECT count(*) FROM `data_smart_device` WHERE state in ('Y', 'N')");
$stmt->execute();
$total_all_records = $stmt->fetchColumn();

$no = $total_all_records - $start;
foreach ($result as $row => $value) {
  $result[$row]['no'] = $no--;
  $result[$row]['status'] = $value['status'] == 'Y' ? 'used' : 'unused';
  // $result[$row]['country_name'] = ($value['country_name']!='') ? '<img src="assets/images/flags/'.$value['flag_name'].'.png" />'.' '.$value['country_name']:'';
  $result[$row]['country_name'] = ($value['country_name'] != '') ? '<img src="assets/images/flags/' . $value['flag_name'] . '.png" />' : '';
}

## Response
$response = array(
  "draw" => intval($draw),
  "iTotalRecords" => $total_all_records,
  "iTotalDisplayRecords" => $filtered_rows,
  "aaData" => $result
);

echo json_encode($response);
exit();