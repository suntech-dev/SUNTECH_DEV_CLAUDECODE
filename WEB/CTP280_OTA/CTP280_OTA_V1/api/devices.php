<?php
/**
 * CTP280 OTA - 디바이스 목록 및 업데이트 현황 조회
 * GET /ota/api/devices.php
 * Response: 디바이스별 업데이트 상태 JSON
 */
header('Content-Type: application/json');
header('Access-Control-Allow-Origin: *');

$dbFile  = __DIR__ . '/../db/devices.json';
$verFile = __DIR__ . '/../db/version.json';

$devices = [];
$latest  = 'V2.0.0';

if (file_exists($dbFile)) {
    $raw = file_get_contents($dbFile);
    $devices = json_decode($raw, true) ?? [];
}

if (file_exists($verFile)) {
    $vInfo  = json_decode(file_get_contents($verFile), true);
    $latest = $vInfo['version'] ?? 'V2.0.0';
}

// 각 디바이스에 최신 버전 비교 결과 추가
foreach ($devices as &$dev) {
    $dev['latest_version'] = $latest;
    $dev['is_latest']      = (strtoupper($dev['version'] ?? '') === strtoupper($latest));
}
unset($dev);

echo json_encode([
    'latest_version' => $latest,
    'total_devices'  => count($devices),
    'devices'        => array_values($devices)
], JSON_PRETTY_PRINT | JSON_UNESCAPED_UNICODE);
