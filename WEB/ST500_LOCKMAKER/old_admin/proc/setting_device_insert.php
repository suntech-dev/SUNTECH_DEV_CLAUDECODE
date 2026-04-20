<?php
## ST-500 Renewal (2022.02.23 Start. dev@suntech.asia & hamani@naver.com)

require_once ($_SERVER['DOCUMENT_ROOT'] . '/st500/lib/database.php');
require_once ($_SERVER['DOCUMENT_ROOT'] . '/st500/lib/common.php');

$now_date = date('Y-m-d', $_SERVER['REQUEST_TIME']);    // korea date type
$now_time = date('H:i:s', $_SERVER['REQUEST_TIME']);
$date_time = $now_date . " " . $now_time;

## edit modal values 
$idx = isset($_POST['idx']) ? trim($_POST['idx']) : '';
$operation = isset($_POST['operation']) ? trim($_POST['operation']) : '';
$device_id = isset($_POST['device_id']) ? trim($_POST['device_id']) : '';
$country_idx = isset($_POST['country_idx']) ? trim($_POST['country_idx']) : 9;
$status = isset($_POST['status']) ? trim($_POST['status']) : '';

## operation 값이 비어있으면 데이터를 로딩
if ($idx && !$operation) {
  // $stmt = $pdo->prepare("SELECT * FROM `st500_device` WHERE idx=? LIMIT 1");
  //용우 버전
  /* $stmt = $pdo->prepare("SELECT idx, device_id, country_idx, state AS status FROM `st500_device` WHERE idx=? LIMIT 1"); */
  //김이사 버전
  $stmt = $pdo->prepare("SELECT idx, device_id, country_idx, state AS status FROM `data_smart_device` WHERE idx=? LIMIT 1");

  $stmt->execute([$idx]);
  $result = $stmt->fetch(PDO::FETCH_ASSOC);

  if (!$result)
    jsonFail('Data not found');

  echo json_encode($result);
  exit();
}

## edit modal data update
if ($operation == "Edit") {

  if (!$idx)
    jsonFail('Enter the idx value');  // idx 값이 존재하지 않으면 오류

  $stmt = $pdo->prepare(
    "UPDATE `data_smart_device` 
     SET country_idx = ?, state = ?, update_date = ?
     WHERE idx = ?"
  );

  $result = $stmt->execute(array($country_idx, $status, $date_time, $idx));

  if (!$result)
    jsonFail('Failed to update data');

  jsonSuccess('Data Updated');

} else {

  jsonFail('Incorrect use');
}