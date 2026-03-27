<?php
// 자수기 생산 데이터 수신 API — EMBROIDERY_S 전용
// 파라미터: mac, actual_qty, ct, tb, mrt
// machine 등록 여부 / 근무시간 여부 관계없이 무조건 저장 (테스트 모드)

require_once(__DIR__ . '/../../lib/api_helper.lib.php');

$apiHelper = new ApiHelper($pdo);

// MAC 주소 검증 및 정규화
$mac = $apiHelper->validateAndProcessMac($_REQUEST['mac'] ?? '');

// 파라미터 수신
$actual_qty = (int)   trim($_REQUEST['actual_qty'] ?? 0);
$ct         = (float) trim($_REQUEST['ct']         ?? 0);
$tb         = (int)   trim($_REQUEST['tb']         ?? 0);
$mrt        = (float) trim($_REQUEST['mrt']        ?? 0);

// 범위 유효성 검사
if ($actual_qty < 0 || $actual_qty > 10000) {
    jsonReturn(['code' => '99', 'msg' => 'actual_qty out of range']);
}
if ($tb < 0 || $tb > 1000) {
    jsonReturn(['code' => '99', 'msg' => 'tb out of range']);
}
if ($ct < 0 || $ct > 3600) {
    jsonReturn(['code' => '99', 'msg' => 'ct out of range']);
}
if ($mrt < 0 || $mrt > 3600) {
    jsonReturn(['code' => '99', 'msg' => 'mrt out of range']);
}

// data_embroidery 테이블에 INSERT
$stmt = $pdo->prepare(
    "INSERT INTO data_embroidery (mac, actual_qty, ct, tb, mrt)
     VALUES (?, ?, ?, ?, ?)"
);
$stmt->execute([$mac, $actual_qty, $ct, $tb, $mrt]);

jsonReturn(['code' => '00', 'msg' => 'OK', 'id' => $pdo->lastInsertId()]);
