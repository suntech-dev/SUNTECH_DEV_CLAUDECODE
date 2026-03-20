<?php
/**
 * CTP280 OTA - 디바이스 업데이트 상태 보고 수신
 * GET /ota/api/status.php?mac=AA:BB:CC:DD:EE:FF&status=done&version=V2.0.1
 * Response: {"status":"ok"}
 *
 * status 값:
 *   downloading - 다운로드 시작
 *   done        - 업데이트 완료 후 재부팅
 *   error       - 오류 발생
 *   booted      - 새 펌웨어로 부팅 완료 (향후 확장)
 */
header('Content-Type: application/json');
header('Access-Control-Allow-Origin: *');

$mac     = trim($_GET['mac']     ?? 'unknown');
$status  = trim($_GET['status']  ?? 'unknown');
$version = trim($_GET['version'] ?? 'unknown');

// MAC 주소 기본 검증
if (empty($mac) || $mac === 'unknown') {
    http_response_code(400);
    echo json_encode(['error' => 'mac required']);
    exit;
}

$dbFile  = __DIR__ . '/../db/devices.json';
$devices = [];

if (file_exists($dbFile)) {
    $raw = file_get_contents($dbFile);
    $devices = json_decode($raw, true) ?? [];
}

$devices[$mac] = [
    'mac'        => $mac,
    'status'     => $status,
    'version'    => $version,
    'updated_at' => date('Y-m-d H:i:s'),
    'ip'         => $_SERVER['REMOTE_ADDR'] ?? 'unknown'
];

file_put_contents($dbFile, json_encode($devices, JSON_PRETTY_PRINT | JSON_UNESCAPED_UNICODE));

echo json_encode(['status' => 'ok', 'mac' => $mac]);
