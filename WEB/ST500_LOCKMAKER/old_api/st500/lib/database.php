<?php

## 2023.05.23일까지 사용한 소스
## PDO
/* $servername = "localhost";
$username = "root";
$password = "suntech9304!";
// $dbname = "suntech_bbs";
$dbname = "suntech_st500";

try {

	$pdo = new PDO("mysql:host=$servername;dbname=$dbname;charset=utf8", $username, $password);
	$pdo->setAttribute(PDO::ATTR_ERRMODE, PDO::ERRMODE_EXCEPTION);

} catch (PDOException $e) {

	echo "Connection failed" . "<br>" . $e->getMessage() . "<br>" . $e->getfile() . "(" . $e->getline() . ")";

} */



## ST-500 Renewal (2022.02.23 Start. dev@suntech.asia)
## ST-500 login 기능 추가 (2023.05.24. dev@suntech.asia)

date_default_timezone_set('Asia/Seoul');
$dbserver = "localhost";
$dbuser = "root";
$dbpass = "suntech9304!";
$dbname = "suntech_st500";

try {
	$pdo = new PDO("mysql:host={$dbserver};dbname={$dbname};charset=utf8", $dbuser, $dbpass);
	$pdo->setAttribute(PDO::ATTR_ERRMODE, PDO::ERRMODE_EXCEPTION);

} catch (PDOException $e) {
	echo "Connection failed" . "<br>" . $e->getMessage() . "<br>" . $e->getfile() . "(" . $e->getline() . ")";
}