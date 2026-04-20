<?php
// DB 연결 — PDO (MySQL 5.7.44)
// .env 파일에서 설정 로드, fallback: hardcode 없음

function load_env(string $path): void {
    if (!file_exists($path)) return;
    foreach (file($path, FILE_IGNORE_NEW_LINES | FILE_SKIP_EMPTY_LINES) as $line) {
        if (strncmp(trim($line), '#', 1) === 0) continue;
        [$key, $val] = array_map('trim', explode('=', $line, 2));
        if (!isset($_ENV[$key])) $_ENV[$key] = $val;
    }
}

function get_pdo(): PDO {
    static $pdo = null;
    if ($pdo !== null) return $pdo;

    load_env(__DIR__ . '/../.env');

    $host = $_ENV['DB_HOST']     ?? 'localhost';
    $user = $_ENV['DB_USERNAME'] ?? 'root';
    $pass = $_ENV['DB_PASSWORD'] ?? '';
    $name = $_ENV['DB_NAME']     ?? 'suntech_st500';

    $dsn = "mysql:host={$host};dbname={$name};charset=utf8mb4";
    $pdo = new PDO($dsn, $user, $pass, [
        PDO::ATTR_ERRMODE            => PDO::ERRMODE_EXCEPTION,
        PDO::ATTR_DEFAULT_FETCH_MODE => PDO::FETCH_ASSOC,
        PDO::ATTR_EMULATE_PREPARES   => false,
    ]);
    return $pdo;
}
