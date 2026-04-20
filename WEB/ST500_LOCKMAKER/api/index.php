<?php
// ST500 LockMaker API 라우터
// URL: /dev/ST500_LOCKMAKER/api/index.php?code={코드}&...
// MySQL 5.7.44 / PHP 7.4.33

header('Access-Control-Allow-Origin: *');
header('Access-Control-Allow-Methods: GET, POST, OPTIONS');
header('Access-Control-Allow-Headers: Content-Type');

if ($_SERVER['REQUEST_METHOD'] === 'OPTIONS') {
    http_response_code(204);
    exit;
}

$code = isset($_GET['code']) ? trim($_GET['code']) : '';

$routes = [
    'send_device' => __DIR__ . '/v1/send_device.php',
    'get_device'  => __DIR__ . '/v1/get_device.php',
];

if (isset($routes[$code])) {
    require $routes[$code];
} else {
    header('Content-Type: application/json; charset=utf-8');
    echo json_encode([['code' => '99', 'msg' => 'unknown_code']], JSON_UNESCAPED_UNICODE);
}
