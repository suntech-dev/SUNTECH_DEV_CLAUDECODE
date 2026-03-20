<?php
/**
 * CTP280 OTA - 펌웨어 파일 업로드
 * POST /ota/api/upload.php
 * Form data: firmware(file), version(string)
 * Response: {"status":"ok","version":"V2.0.1","size":30720,"crc":12345}
 */
header('Content-Type: application/json');
header('Access-Control-Allow-Origin: *');

if ($_SERVER['REQUEST_METHOD'] !== 'POST') {
    http_response_code(405);
    echo json_encode(['error' => 'POST only']);
    exit;
}

// PHP 업로드 에러 코드 → 사람이 읽을 수 있는 메시지
$uploadErrors = [
    UPLOAD_ERR_INI_SIZE   => 'php.ini upload_max_filesize 초과 (' . ini_get('upload_max_filesize') . ')',
    UPLOAD_ERR_FORM_SIZE  => 'HTML form MAX_FILE_SIZE 초과',
    UPLOAD_ERR_PARTIAL    => '파일이 부분적으로만 업로드됨',
    UPLOAD_ERR_NO_FILE    => '파일이 업로드되지 않음',
    UPLOAD_ERR_NO_TMP_DIR => 'PHP 임시 디렉토리 없음',
    UPLOAD_ERR_CANT_WRITE => 'PHP 임시 디렉토리 쓰기 실패',
    UPLOAD_ERR_EXTENSION  => 'PHP 확장에 의해 업로드 중단됨',
];

if (!isset($_FILES['firmware'])) {
    http_response_code(400);
    echo json_encode([
        'error'        => 'firmware 파일 없음',
        'post_max_size' => ini_get('post_max_size'),
        'content_length' => $_SERVER['CONTENT_LENGTH'] ?? 'unknown',
    ]);
    exit;
}

$uploadErrCode = $_FILES['firmware']['error'];
if ($uploadErrCode !== UPLOAD_ERR_OK) {
    http_response_code(400);
    echo json_encode([
        'error'      => 'PHP 업로드 오류: ' . ($uploadErrors[$uploadErrCode] ?? "code=$uploadErrCode"),
        'upload_max_filesize' => ini_get('upload_max_filesize'),
        'post_max_size'       => ini_get('post_max_size'),
    ]);
    exit;
}

$version = trim($_POST['version'] ?? '');
if (empty($version)) {
    http_response_code(400);
    echo json_encode(['error' => 'version string required (e.g. V2.0.1)']);
    exit;
}

// 버전 형식 검증: V{숫자}.{숫자}.{숫자}
if (!preg_match('/^[Vv]\d+\.\d+\.\d+$/', $version)) {
    http_response_code(400);
    echo json_encode(['error' => 'invalid version format. Use V{major}.{minor}.{patch} (e.g. V2.0.1)']);
    exit;
}

// 필요한 디렉토리 자동 생성
$firmwareDir = __DIR__ . '/../firmware';
$dbDir       = __DIR__ . '/../db';

foreach ([$firmwareDir, $dbDir] as $dir) {
    if (!is_dir($dir)) {
        if (!mkdir($dir, 0755, true)) {
            http_response_code(500);
            echo json_encode(['error' => "디렉토리 생성 실패: $dir"]);
            exit;
        }
    }
}

// firmware/ 쓰기 권한 확인
if (!is_writable($firmwareDir)) {
    http_response_code(500);
    echo json_encode([
        'error' => 'firmware 디렉토리 쓰기 권한 없음',
        'path'  => $firmwareDir,
        'tip'   => 'chmod 775 firmware/ && chown www-data:www-data firmware/',
    ]);
    exit;
}

// db/ 쓰기 권한 확인
if (!is_writable($dbDir)) {
    http_response_code(500);
    echo json_encode([
        'error' => 'db 디렉토리 쓰기 권한 없음',
        'path'  => $dbDir,
        'tip'   => 'chmod 775 db/ && chown www-data:www-data db/',
    ]);
    exit;
}

$targetPath  = $firmwareDir . '/latest.bin';
$uploadedTmp = $_FILES['firmware']['tmp_name'];

if (!move_uploaded_file($uploadedTmp, $targetPath)) {
    http_response_code(500);
    echo json_encode([
        'error'  => 'move_uploaded_file 실패',
        'target' => $targetPath,
        'writable' => is_writable($firmwareDir) ? 'yes' : 'no',
    ]);
    exit;
}

// 파일 크기 및 CRC16-CCITT 계산
$data = file_get_contents($targetPath);
$size = strlen($data);
$crc  = crc16_ccitt($data);

// version.json 갱신
$versionFile = $dbDir . '/version.json';
$versionInfo = [
    'version'     => strtoupper($version),
    'size'        => $size,
    'crc'         => $crc,
    'uploaded_at' => date('Y-m-d H:i:s'),
    'filename'    => $_FILES['firmware']['name'],
    'notes'       => $_POST['notes'] ?? ''
];

if (file_put_contents($versionFile, json_encode($versionInfo, JSON_PRETTY_PRINT)) === false) {
    http_response_code(500);
    echo json_encode([
        'error' => 'version.json 저장 실패',
        'path'  => $versionFile,
        'tip'   => 'chmod 664 db/version.json 또는 db/ 디렉토리 쓰기 권한 확인',
    ]);
    exit;
}

echo json_encode([
    'status'  => 'ok',
    'version' => $versionInfo['version'],
    'size'    => $size,
    'crc'     => $crc
]);

// ─── CRC16-CCITT (0xFFFF init, 0x1021 poly) ──────────────────────
function crc16_ccitt(string $data): int
{
    $crc = 0xFFFF;
    for ($i = 0; $i < strlen($data); $i++) {
        $crc ^= (ord($data[$i]) << 8);
        for ($j = 0; $j < 8; $j++) {
            $crc = ($crc & 0x8000) ? (($crc << 1) ^ 0x1021) : ($crc << 1);
            $crc &= 0xFFFF;
        }
    }
    return $crc;
}
