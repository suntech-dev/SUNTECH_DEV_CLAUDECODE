<?php
## New CPU 패턴재봉기 & Andon 디바이스

// 타임존 중앙 설정 — 개별 파일에 중복 선언하지 말 것
date_default_timezone_set('Asia/Jakarta');

require_once('config.php');

try {
    $pdo = new PDO("mysql:host={$servername};dbname={$dbname};charset=utf8", $username, $password);
    $pdo->setAttribute(PDO::ATTR_ERRMODE, PDO::ERRMODE_EXCEPTION);
} catch (PDOException $e) {
    // 로그 기록
    $log_dir = __DIR__ . '/../logs';
    if (!is_dir($log_dir)) {
        mkdir($log_dir, 0777, true);
    }
    $log_file = $log_dir . '/db_errors.log';
    $error_message = "[" . date("Y-m-d H:i:s") . "] " . $e->getMessage() . " in " . $e->getFile() . " on line " . $e->getLine() . "\n";
    error_log($error_message, 3, $log_file);

    // JSON 에러 응답
    header('Content-Type: application/json');
    echo json_encode([
        'code' => '99',
        'msg' => 'Database connection failed.'
    ]);
    exit();
}