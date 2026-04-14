<?php
/**
 * 자수기 OEE 시스템 — 장비 등록/갱신 API
 *
 * 자수기(EMBROIDERY_S) 전원 ON 이후 WiFi 연결 시 호출.
 * sewing/start.php와 동일한 흐름이지만, 신규 등록 시 type='E' 로 고정합니다.
 *
 * parameter : machine_no, mac, ip, ver
 * 응답      : machine_no, target, req_interval
 */

require_once(__DIR__ . '/../../lib/api_helper.lib.php');

$today        = date('Y-m-d H:i:s');
$req_interval = '3600';
$default_target = '777';

$apiHelper = new ApiHelper($pdo);

// ─────────────────────────────────────────────────────────────────────
// 요청 파라미터 수신
// ─────────────────────────────────────────────────────────────────────
$machine_no = !empty($_REQUEST['machine_no']) ? trim($_REQUEST['machine_no']) : 'Empty';
$app_ver    = !empty($_REQUEST['ver'])        ? trim($_REQUEST['ver'])        : '';
$ip         = !empty($_REQUEST['ip'])         ? trim($_REQUEST['ip'])         : '99';

$mac = $apiHelper->validateAndProcessMac($_REQUEST['mac'] ?? '');

// ─────────────────────────────────────────────────────────────────────
// MAC 주소로 기존 등록 여부 조회
// ─────────────────────────────────────────────────────────────────────
try {
    $stmt = $pdo->prepare("SELECT idx, machine_no, target, type FROM `info_machine` WHERE mac = ?");
    $stmt->execute([$mac]);
    $machine_data = $stmt->fetch(PDO::FETCH_ASSOC);
} catch (PDOException $e) {
    error_log("Database error in embroidery/start.php: " . $e->getMessage());
    jsonReturn($apiHelper->createResponse('99', 'Database connection error'));
}

if ($machine_data) {
    // ── 기존 등록 기기: ip, app_ver, type 갱신 ──────────────────────
    // type이 'P'로 잘못 등록된 경우도 'E'로 교정
    $idx        = $machine_data['idx'];
    $machine_no = $machine_data['machine_no'];
    $target     = $machine_data['target'];

    $stmt   = $pdo->prepare("UPDATE `info_machine` SET ip = ?, app_ver = ?, type = 'E', update_date = ? WHERE idx = ?");
    $result = $stmt->execute([$ip, $app_ver, $today, $idx]);

    if ($result) {
        $response = array('code' => '00', 'machine_no' => $machine_no, 'target' => $target, 'req_interval' => $req_interval);
    } else {
        $response = array('code' => '99', 'msg' => 'Failed to update embroidery machine data');
    }
} else {
    // ── 신규 등록: type='E' 명시, factory/line = 99(INVENTORY) ──────
    $factory_idx = '99';
    $line_idx    = '99';

    $stmt = $pdo->prepare(
        "INSERT INTO `info_machine`(factory_idx, line_idx, machine_no, target, app_ver, ip, mac, type, reg_date, update_date)
         VALUES (?, ?, ?, ?, ?, ?, ?, 'E', ?, ?)"
    );
    $result = $stmt->execute([$factory_idx, $line_idx, $machine_no, $default_target, $app_ver, $ip, $mac, $today, $today]);

    if ($result) {
        $response = array('code' => '00', 'machine_no' => $machine_no, 'target' => $default_target, 'req_interval' => $req_interval);
    } else {
        $response = array('code' => '99', 'msg' => 'Failed to register embroidery machine');
    }
}

// ─────────────────────────────────────────────────────────────────────
// API 호출 로그 저장 (logs_api_start 공유)
// ─────────────────────────────────────────────────────────────────────
$apiHelper->logApiCall('logs_api_start', 'embroidery_start', $machine_no, $mac, $_REQUEST, $response, $today);

/*
## API Endpoint
embroidery/start.php (via embroidery.php?code=start)

## Example Request
http://SERVER/OEE_SCI/OEE_SCI_V2/api/embroidery.php?code=start&machine_no=EMB01&mac=84:72:07:50:37:73&ip=192.168.0.26&ver=EMBROIDERY_S_REV_1.0

## Example Response
{
  "code": "00",
  "machine_no": "EMB01",
  "target": "777",
  "req_interval": "3600"
}
*/
