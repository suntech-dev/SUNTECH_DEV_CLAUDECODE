<?php
## ST-500 Renewal (2022.02.23 Start. dev@suntech.asia & hamani@naver.com)

date_default_timezone_set('Asia/Seoul');
// date_default_timezone_set('Asia/Jakarta');

$servername = "115.68.227.31";
$username = "root";
$password = "suntech9304!";
// $dbname = "suntech_bbs";
$dbname = "suntech_st500";

try {

  $pdo = new PDO("mysql:host={$servername};dbname={$dbname};charset=utf8", $username, $password);
  $pdo->setAttribute(PDO::ATTR_ERRMODE, PDO::ERRMODE_EXCEPTION);

} catch (PDOException $e) {

  echo "Connection failed" . "<br>" . $e->getMessage() . "<br>" . $e->getfile() . "(" . $e->getline() . ")";

}