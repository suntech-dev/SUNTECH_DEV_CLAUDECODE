<?php
## New CPU 패턴재봉기 & 일반재봉기 통합 버전 & 안돈, 경광등
## Machine 전원 On 이후 wifi 연결되고 "req_interval" 값마다 'info_machine' 테이블의 데이터 조회
## parameter : machine_no(IoT 컨피그 프로그램에서 입력한 값), mac, ip, ver(소스 버전)
## 응답 결과에서 "device_no", "target", "req_interval" 을 Machine 에 저장

// 공통 헬퍼 라이브러리 로드
require_once(__DIR__ . '/../../lib/api_helper.lib.php');

$today = date('Y-m-d H:i:s');
$req_interval = '3600';    // andon list 갱신 시간
$default_target = '777';   // 'info_machine' 테이블에 등록한 target 이 없을때

// API 헬퍼 초기화
$apiHelper = new ApiHelper($pdo);

// 기본 파라미터 수신
$machine_no = !empty($_REQUEST['machine_no']) ? trim($_REQUEST['machine_no']) : 'Empty';
$app_ver = !empty($_REQUEST['ver']) ? trim($_REQUEST['ver']) : '';
$ip = !empty($_REQUEST['ip']) ? trim($_REQUEST['ip']) : '99';

// MAC 주소 검증 및 처리 (공통 함수 사용)
$mac = $apiHelper->validateAndProcessMac($_REQUEST['mac'] ?? '');

// MAC 주소로 machine 조회 (공통 함수 사용 - 간단한 조회만 필요)
try {
  $stmt = $pdo->prepare("SELECT idx, machine_no, target FROM `info_machine` WHERE mac = ?");
  $stmt->execute([$mac]);
  $machine_data = $stmt->fetch(PDO::FETCH_ASSOC);
} catch (PDOException $e) {
  error_log("Database error in start.php: " . $e->getMessage());
  jsonReturn($apiHelper->createResponse('99', 'Database connection error'));
}

// 이미 등록된 machine 정보가 있으면 ip, app_ver 업데이트
if ($machine_data) {

  $idx = $machine_data['idx'];
  $machine_no = $machine_data['machine_no'];
  $target = $machine_data['target'];

  $stmt = $pdo->prepare("UPDATE `info_machine` SET ip = ?, app_ver = ?, update_date = ? WHERE idx = ?");
  $result = $stmt->execute([$ip, $app_ver, $today, $idx]);

  if ($result) {
    $response = array('code' => '00', 'machine_no' => $machine_no, 'target' => $target, 'req_interval' => $req_interval);
  } else {
    $response = array('code' => '99', 'msg' => 'Failed to update machine data');
  }
} else {

  // 등록된 장비 정보가 없으면 신규 등록
  $factory_idx = '99';
  $line_idx = '99';

  $stmt = $pdo->prepare(
    "INSERT INTO `info_machine`(factory_idx, line_idx, machine_no, target, app_ver, ip, mac, reg_date, update_date)
    VALUES (?, ?, ?, ?, ?, ?, ?, ?, ?)
  "
  );
  $result = $stmt->execute([$factory_idx, $line_idx, $machine_no, $default_target, $app_ver, $ip, $mac, $today, $today]);

  $last_id = $pdo->lastInsertId();

  if ($result) {
    $response = array('code' => '00', 'machine_no' => $machine_no, 'target' => $default_target, 'req_interval' => $req_interval);
  } else {
    $response = array('code' => '99', 'msg' => 'Failed to register machine');
  }
}

// API 호출 로그 저장 (공통 함수 사용)
$apiHelper->logApiCall('logs_api_start', 'start', $machine_no, $mac, $_REQUEST, $response, $today);

/*
## API Endpoint
start.php

## Example Request
http://49.247.26.228/2025/sci/new/api/sewing.php?code=start&machine_no=TEST01&mac=84:72:07:50:37:73&ip=192.168.0.26&ver=Integrated_REV_9.6

## Example Response
{
  "code": "00",
  "machine_no": "TEST01",
  "target": "777",
  "req_interval": "3600"
}
*/