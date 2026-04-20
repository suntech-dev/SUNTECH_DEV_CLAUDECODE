<?php
// send_device — 디바이스 등록 또는 업데이트
// GET ?code=send_device&device_id=UUID&name=이름

require_once __DIR__ . '/../lib/db.php';
require_once __DIR__ . '/../lib/helpers.php';

$device_id = get_param('device_id');
$name      = get_param('name');

$input = ['device_id' => $device_id, 'name' => $name];

// 필수 파라미터 검증
if ($device_id === '' || $name === '') {
    $res = [['code' => '01', 'msg' => 'missing_params']];
    write_log(get_pdo(), 'send_device', $input, $res);
    json_response($res);
}

// UUID v4 형식 검증 (보안)
if (!preg_match('/^[0-9a-f]{8}-[0-9a-f]{4}-4[0-9a-f]{3}-[89ab][0-9a-f]{3}-[0-9a-f]{12}$/i', $device_id)) {
    $res = [['code' => '02', 'msg' => 'invalid_device_id']];
    write_log(get_pdo(), 'send_device', $input, $res);
    json_response($res);
}

$pdo = get_pdo();

// 기존 디바이스 조회
$stmt = $pdo->prepare("SELECT idx, state FROM lm_device WHERE device_id = ?");
$stmt->execute([$device_id]);
$row = $stmt->fetch();

if ($row) {
    // 삭제된 디바이스는 거부
    if ($row['state'] === 'D') {
        $res = [['code' => '03', 'msg' => 'device_deleted']];
        write_log($pdo, 'send_device', $input, $res);
        json_response($res);
    }
    // 이름 업데이트
    $stmt = $pdo->prepare("UPDATE lm_device SET name = ?, update_date = ? WHERE idx = ?");
    $stmt->execute([$name, now_datetime(), $row['idx']]);
} else {
    // 신규 등록
    $stmt = $pdo->prepare(
        "INSERT INTO lm_device (device_id, name, state, reg_date)
         VALUES (?, ?, 'N', ?)"
    );
    $stmt->execute([$device_id, $name, now_datetime()]);
}

$res = [['code' => '00', 'msg' => 'success']];
write_log($pdo, 'send_device', $input, $res);
json_response($res);
