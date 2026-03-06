<?php
## New CPU 패턴재봉기 & Andon 디바이스

date_default_timezone_set('Asia/Jakarta');

require_once(__DIR__ . '/config.php');

try {
    $pdo = new PDO("mysql:host={$servername};dbname={$dbname};charset=utf8", $username, $password);
    $pdo->setAttribute(PDO::ATTR_ERRMODE, PDO::ERRMODE_EXCEPTION);
} catch (PDOException $e) {
    echo "Connection failed" . "<br>" . $e->getMessage() . "<br>" . $e->getFile() . "(" . $e->getLine() . ")";
    exit();
}