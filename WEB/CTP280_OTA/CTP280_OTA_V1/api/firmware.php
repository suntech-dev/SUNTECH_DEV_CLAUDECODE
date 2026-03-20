<?php
/**
 * CTP280 OTA - 펌웨어 청크 반환 (HEX 인코딩)
 * GET /ota/api/firmware.php?offset=0&size=400
 * Response: {"offset":0,"bytes":400,"hex":"AABBCC..."}
 *
 * PSoC WiFi 버퍼 = 2048 bytes 제한으로 size 최대 400 bytes (hex 800자 + JSON 오버헤드)
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
$size    = min(400, max(1, intval($_GET['size'] ?? 400)));  // 최대 400 bytes
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
