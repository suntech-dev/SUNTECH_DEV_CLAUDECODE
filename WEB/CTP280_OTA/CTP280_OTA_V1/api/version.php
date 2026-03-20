<?php
/**
 * CTP280 OTA - 최신 버전 정보 반환
 * GET /ota/api/version.php
 * Response: {"version":"V2.0.1","size":30720,"crc":12345}
 */
header('Content-Type: application/json');
header('Access-Control-Allow-Origin: *');

$versionFile = __DIR__ . '/../db/version.json';

if (!file_exists($versionFile)) {
    http_response_code(404);
    echo json_encode(['error' => 'version file not found']);
    exit;
}

$info = json_decode(file_get_contents($versionFile), true);

if (!$info) {
    http_response_code(500);
    echo json_encode(['error' => 'invalid version data']);
    exit;
}

echo json_encode([
    'version' => $info['version'] ?? 'V2.0.0',
    'size'    => (int)($info['size'] ?? 0),
    'crc'     => (int)($info['crc']  ?? 0)
]);
