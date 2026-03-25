<?php
/**
 * CTP280 OTA - 펌웨어 청크 반환 (HEX 인코딩)
 * GET /ota/api/firmware.php?offset=0&size=1800
 * Response: {"offset":0,"bytes":1800,"hex":"AABBCC..."}
 *
 * PSoC WiFi 버퍼 = 4096 bytes / size 최대 1800 bytes (hex 3600자 + JSON 오버헤드 ~3660)
 */
header('Content-Type: application/json');
header('Access-Control-Allow-Origin: *');

$firmwareFile = __DIR__ . '/../firmware/latest.bin';

if (!file_exists($firmwareFile)) {
    http_response_code(404);
    echo json_encode(['error' => 'firmware not found']);
    exit;
}

$offset  = max(0, intval($_GET['offset'] ?? 0));
$size    = min(580, max(1, intval($_GET['size'] ?? 580)));   // 최대 580 bytes (ICT WiFi 모듈 HTTPBODY 한계 ~1231 bytes 대응)
$total   = filesize($firmwareFile);

if ($offset >= $total) {
    http_response_code(400);
    echo json_encode(['error' => 'offset out of range', 'total' => $total]);
    exit;
}

// 실제 읽을 수 있는 바이트 수 조정
$readSize = min($size, $total - $offset);

$fp   = fopen($firmwareFile, 'rb');
fseek($fp, $offset);
$data = fread($fp, $readSize);
fclose($fp);

$hex = strtoupper(bin2hex($data));

echo json_encode([
    'offset' => $offset,
    'bytes'  => $readSize,
    'total'  => $total,
    'hex'    => $hex
]);
