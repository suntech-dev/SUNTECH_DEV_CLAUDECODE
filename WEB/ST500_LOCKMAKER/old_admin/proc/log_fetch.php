<?php
## ST-500 Renewal (2022.02.23 Start. dev@suntech.asia & hamani@naver.com)

require_once($_SERVER['DOCUMENT_ROOT'] . '/st500/lib/database.php');

## Custom Field value
$searchByCode     = isset($_POST['searchByCode']) ? $_POST['searchByCode'] : '';
$searchByPeriod   = isset($_POST['searchByPeriod']) ? $_POST['searchByPeriod'] : '';

## Read value
$searchValue      = isset($_POST['search']['value']) ? $_POST['search']['value'] : '';
$columnIndex      = isset($_POST['order'][0]['column']) ? $_POST['order'][0]['column'] : '';
$columnName       = isset($_POST['columns'][$columnIndex]['data']) ? $_POST['columns'][$columnIndex]['data'] : '';
$columnSortOrder  = isset($_POST['order'][0]['dir']) ? $_POST['order'][0]['dir'] : '';

$rowPerPage       = isset($_POST['length']) ? $_POST['length'] : -1; // Rows display per page
$start            = isset($_POST['start']) ? $_POST['start'] : 0;
$draw             = isset($_POST["draw"]) ? $_POST["draw"] : 1;

## Date search value
// unused.

## Search
$qry = array();

if ($searchByCode) { $qry[] = "code = '{$searchByCode}'"; }
if ($searchByPeriod)   { $qry[] = "reg_date = '{$searchByPeriod}'"; }
if ($searchValue)     { $qry[] = "(log_data LIKE '%{$searchValue}%' OR log_result LIKE '%{$searchValue}%' OR reg_ip LIKE '%{$searchValue}%')"; }

$where = ($qry) ? ' WHERE '.implode(' AND ', $qry) : '';
$order = ($columnIndex) ? " ORDER BY {$columnName} {$columnSortOrder}" : " ORDER BY idx DESC";
$limit = ($rowPerPage != -1) ? " LIMIT {$start}, {$rowPerPage}" : '';

## Fetch records
$stmt = $pdo->prepare(
  "SELECT idx, code, log_data, log_result, reg_ip, reg_date 
   FROM `st500_logs`"
   .$where.$order.$limit
  );
$stmt->execute();
$result = $stmt->fetchAll(PDO::FETCH_ASSOC);  
$filtered_rows = $stmt->rowCount();  // Total number of records with filtering

## Total number of records without filtering
$stmt = $pdo->prepare("SELECT count(*) FROM `st500_logs`");
$stmt->execute();
$total_all_records = $stmt->fetchColumn();

$no = $total_all_records - $start;
foreach($result as $row => $value) {
  $result[$row]['no'] = $no--;
}

## Response
$response = array(
  "draw"                  => intval($draw),
  "iTotalRecords"         => $filtered_rows,
  "iTotalDisplayRecords"  => $total_all_records,
  "aaData"                => $result
);

echo json_encode($response);
exit();