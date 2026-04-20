<?php
// get_device — 디바이스 승인 상태 조회
// GET ?code=get_device&device_id=UUID

require_once __DIR__ . '/../lib/db.php';
require_once __DIR__ . '/../lib/helpers.php';

$device_id = get_param('device_id');
$input     = ['device_id' => $device_id];

if ($device_id === '') {
    $res = [['code' => '01', 'msg' => 'missing_params']];
    write_log(get_pdo(), 'get_device', $input, $res);
    json_response($res);
}

$pdo  = get_pdo();
$stmt = $pdo->prepare("SELECT state FROM lm_device WHERE device_id = ?");
$stmt->execute([$device_id]);
$row  = $stmt->fetch();

if (!$row) {
    $res = [['code' => '04', 'msg' => 'not_found']];
    write_log($pdo, 'get_device', $input, $res);
    json_response($res);
}

// state → msg 매핑
$msg_map = ['Y' => 'approve', 'N' => 'wait', 'D' => 'deleted'];
$msg     = $msg_map[$row['state']] ?? 'unknown';

$res = [['code' => '00', 'msg' => $msg]];
write_log($pdo, 'get_device', $input, $res);
json_response($res);
