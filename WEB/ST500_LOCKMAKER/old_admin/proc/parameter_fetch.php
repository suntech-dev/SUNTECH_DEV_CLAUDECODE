<?php
## SUNTECH Documents (2022.02.23 Start. dev@suntech.asia & hamani@naver.com)

require_once($_SERVER['DOCUMENT_ROOT'] . '/st500/lib/database.php');

## Custom Field value
$searchByCountry  = isset($_POST['searchByCountry']) ? $_POST['searchByCountry'] : '';

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

if ($searchByCountry) { $qry[] = "country = '{$searchByCountry}'"; }
if ($searchValue) { $qry[] = "(num LIKE '%{$searchValue}%' OR text LIKE '%{$searchValue}%')"; }

$where = ($qry) ? ' WHERE '.implode(' AND ', $qry) : '';
$order = ($columnIndex) ? " ORDER BY {$columnName} {$columnSortOrder}" : " ORDER BY num ASC";
$limit = ($rowPerPage != -1) ? " LIMIT {$start}, {$rowPerPage}" : '';

## Fetch records
$stmt = $pdo->prepare(
  "SELECT idx, country, num, text 
   FROM `st500_parameter`"
   .$where.$order.$limit
  );
$stmt->execute();
$result = $stmt->fetchAll(PDO::FETCH_ASSOC);
// $filtered_rows = $stmt->rowCount();  // Total number of records with filtering

## Total number of records with filtering
$stmt = $pdo->prepare(
  "SELECT count(*) FROM `st500_parameter`"
   .$where.$order
  );
$stmt->execute();
$filtered_rows = $stmt->fetchColumn();  // Total number of records with filtering

## Total number of records without filtering
$stmt = $pdo->prepare("SELECT count(*) FROM `st500_parameter`");
$stmt->execute();
$total_all_records = $stmt->fetchColumn();

## Response
/* $response = array(
  "draw"                  => intval($draw),
  "iTotalRecords"         => $filtered_rows,
  "iTotalDisplayRecords"  => $total_all_records,
  "aaData"                => $result
); */

$response = array(
  "draw"                  => intval($draw),
  "iTotalRecords"         => $total_all_records,
  "iTotalDisplayRecords"  => $filtered_rows,
  "aaData"                => $result
);

echo json_encode($response);
exit();