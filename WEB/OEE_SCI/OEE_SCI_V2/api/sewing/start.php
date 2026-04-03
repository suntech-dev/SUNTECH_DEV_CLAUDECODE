<?php
## New CPU 패턴재봉기 & 일반재봉기 통합 버전 & 안돈, 경광등
## Machine 전원 On 이후 wifi 연결되고 "req_interval" 값마다 'info_machine' 테이블의 데이터 조회
## parameter : machine_no(IoT 컨피그 프로그램에서 입력한 값), mac, ip, ver(소스 버전)
## 응답 결과에서 "device_no", "target", "req_interval" 을 Machine 에 저장

// ─────────────────────────────────────────────────────────────────────
// OEE 공통 헬퍼 라이브러리 로드
// ─────────────────────────────────────────────────────────────────────
// 공통 헬퍼 라이브러리 로드
require_once(__DIR__ . '/../../lib/api_helper.lib.php');

// 현재 서버 시간 (등록·업데이트 날짜 기록에 사용)
$today = date('Y-m-d H:i:s');
// 안돈 목록 갱신 주기 (초 단위): 기기가 이 간격마다 목록을 다시 요청
$req_interval = '3600';    // andon list 갱신 시간
// info_machine 테이블에 target 값이 없는 신규 등록 기기에 부여하는 기본 목표 생산량
$default_target = '777';   // 'info_machine' 테이블에 등록한 target 이 없을때

// API 헬퍼 초기화
$apiHelper = new ApiHelper($pdo);

// ─────────────────────────────────────────────────────────────────────
// 요청 파라미터 수신
// machine_no : IoT 컨피그 프로그램에서 설정한 기계 식별 번호
// app_ver    : 펌웨어/앱 버전 문자열 (예: Integrated_REV_9.6)
// ip         : 기기의 현재 IP 주소 (네트워크 추적용)
// ─────────────────────────────────────────────────────────────────────
// 기본 파라미터 수신
$machine_no = !empty($_REQUEST['machine_no']) ? trim($_REQUEST['machine_no']) : 'Empty';
$app_ver = !empty($_REQUEST['ver']) ? trim($_REQUEST['ver']) : '';
$ip = !empty($_REQUEST['ip']) ? trim($_REQUEST['ip']) : '99';

// MAC 주소 검증 및 처리 (공통 함수 사용)
$mac = $apiHelper->validateAndProcessMac($_REQUEST['mac'] ?? '');

// ─────────────────────────────────────────────────────────────────────
// MAC 주소로 기존 등록 여부 조회
// info_machine 테이블에서 mac 일치 레코드 확인
// - 조회 대상: idx(PK), machine_no(기계번호), target(목표 생산량)
// ─────────────────────────────────────────────────────────────────────
// MAC 주소로 machine 조회 (공통 함수 사용 - 간단한 조회만 필요)
try {
    $stmt = $pdo->prepare("SELECT idx, machine_no, target FROM `info_machine` WHERE mac = ?");
    $stmt->execute([$mac]);
    $machine_data = $stmt->fetch(PDO::FETCH_ASSOC);
} catch (PDOException $e) {
    error_log("Database error in start.php: " . $e->getMessage());
    jsonReturn($apiHelper->createResponse('99', 'Database connection error'));
}

// ─────────────────────────────────────────────────────────────────────
// 분기 처리: 기존 등록 기기 vs 신규 등록 기기
// ─────────────────────────────────────────────────────────────────────

// 이미 등록된 machine 정보가 있으면 ip, app_ver 업데이트
if ($machine_data) {

    // 기존 등록 기기: DB에 저장된 machine_no·target 값을 그대로 사용
    $idx = $machine_data['idx'];
    $machine_no = $machine_data['machine_no'];
    $target = $machine_data['target'];

    // 현재 IP와 앱 버전, 마지막 접속 시각을 갱신
    $stmt = $pdo->prepare("UPDATE `info_machine` SET ip = ?, app_ver = ?, update_date = ? WHERE idx = ?");
    $result = $stmt->execute([$ip, $app_ver, $today, $idx]);

    if ($result) {
        // 성공: 기기에 전달할 설정값 응답 (machine_no, target, req_interval)
        $response = array('code' => '00', 'machine_no' => $machine_no, 'target' => $target, 'req_interval' => $req_interval);
    } else {
        $response = array('code' => '99', 'msg' => 'Failed to update machine data');
    }
} else {

    // ─────────────────────────────────────────────────────────────────
    // 신규 등록: info_machine 테이블에 새 레코드 삽입
    // - factory_idx = 99, line_idx = 99: 미배정(INVENTORY) 상태로 초기 등록
    //   관리자가 나중에 실제 공장·라인에 배정하기 전까지 이 값 유지
    // - target = default_target: 관리자 배정 전 임시 목표 생산량
    // ─────────────────────────────────────────────────────────────────
    // 등록된 장비 정보가 없으면 신규 등록
    $factory_idx = '99';
    $line_idx = '99';

    $stmt = $pdo->prepare(
        "INSERT INTO `info_machine`(factory_idx, line_idx, machine_no, target, app_ver, ip, mac, reg_date, update_date)
    VALUES (?, ?, ?, ?, ?, ?, ?, ?, ?)
  "
    );
    $result = $stmt->execute([$factory_idx, $line_idx, $machine_no, $default_target, $app_ver, $ip, $mac, $today, $today]);

    $last_id = $pdo->lastInsertId(); // 신규 삽입된 레코드의 PK (현재는 사용하지 않음)

    if ($result) {
        // 신규 등록 성공: 기기에 임시 목표 생산량과 갱신 주기 전달
        $response = array('code' => '00', 'machine_no' => $machine_no, 'target' => $default_target, 'req_interval' => $req_interval);
    } else {
        $response = array('code' => '99', 'msg' => 'Failed to register machine');
    }
}

// ─────────────────────────────────────────────────────────────────────
// API 호출 로그 저장
// logs_api_start 테이블에 요청·응답 이력 기록
// ─────────────────────────────────────────────────────────────────────
// API 호출 로그 저장 (공통 함수 사용)
$apiHelper->logApiCall('logs_api_start', 'start', $machine_no, $mac, $_REQUEST, $response, $today);

/*
## API Endpoint
start.php

## Example Request
http://49.247.26.228/OEE_SCI/OEE_SCI_V2/api/sewing.php?code=start&machine_no=TEST01&mac=84:72:07:50:37:73&ip=192.168.0.26&ver=Integrated_REV_9.6

## Example Response
{
  "code": "00",
  "machine_no": "TEST01",
  "target": "777",
  "req_interval": "3600"
}
*/
