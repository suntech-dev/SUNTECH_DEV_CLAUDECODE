<?php
## New CPU 패턴재봉기 & 일반재봉기 통합 버전 & 안돈, 경광등 ##
## 현재 시간을 조회

// ─────────────────────────────────────────────────────────────────────
// OEE 공통 헬퍼 라이브러리 로드
// ApiHelper 클래스: 응답 생성, MAC 검증, DB 조작 등 공통 기능 제공
// ─────────────────────────────────────────────────────────────────────
require_once(__DIR__ . '/../../lib/api_helper.lib.php');

// API 헬퍼 초기화 (PDO 연결 주입)
$apiHelper = new ApiHelper($pdo);

// 서버 현재 날짜·시간 조회 (Y-m-d H:i:s 형식)
// 재봉기(IoT 기기)는 이 값을 시간 동기화에 활용합니다.
$today = date('Y-m-d H:i:s'); // 현재 날짜 및 시간

// ─────────────────────────────────────────────────────────────────────
// [BUG FIX] AUTO RESET 교대 기반 비교를 위한 shift 정보 추가
// mac 파라미터가 있으면 해당 기기의 현재 work_date + shift_idx 를 응답에 포함.
// 펌웨어는 이 값을 "work_date * 10 + shift_idx" 로 인코딩해 비교하므로
// 자정을 넘는 야간 교대(예: 21:00~06:00)에서도 잘못된 리셋이 발생하지 않는다.
// mac 이 없거나 근무 외 시간이면 work_date/shift_idx 를 생략 → 펌웨어 fallback 동작.
// ─────────────────────────────────────────────────────────────────────
$mac = !empty($_REQUEST['mac']) ? strtoupper(trim($_REQUEST['mac'])) : '';

$work_date = null;
$shift_idx = null;

if (!empty($mac)) {
    $stmt = $pdo->prepare(
        "SELECT factory_idx, line_idx FROM info_machine WHERE mac = ? AND status = 'Y' LIMIT 1"
    );
    $stmt->execute([$mac]);
    $machine = $stmt->fetch(PDO::FETCH_ASSOC);

    if ($machine) {
        $current_shift_info = $apiHelper->getCurrentShiftInfo(
            $machine['factory_idx'],
            $machine['line_idx'],
            $today
        );
        if ($current_shift_info) {
            $work_date = $current_shift_info['date'];
            $shift_idx = (int)$current_shift_info['shift_idx'];
        }
    }
}

// ─────────────────────────────────────────────────────────────────────
// 응답 데이터 구성
// createResponse_onlyItems(): code 없이 데이터만 포함하는 응답 배열 생성
// shift 정보가 있으면 work_date + shift_idx 를 함께 반환
// ─────────────────────────────────────────────────────────────────────
$response_items = ['datetime' => $today];
if ($work_date !== null && $shift_idx !== null) {
    $response_items['work_date'] = $work_date;
    $response_items['shift_idx'] = $shift_idx;
}
$response = $apiHelper->createResponse_onlyItems($response_items);

/*
## API
get_dateTime.php

## API 예시 (mac 없이 — 기존 호환)
http://49.247.26.228/OEE_SCI/OEE_SCI_V2/api/sewing.php?code=get_dateTime

## API 예시 (mac 포함 — shift 정보 반환)
http://49.247.26.228/OEE_SCI/OEE_SCI_V2/api/sewing.php?code=get_dateTime&mac=84:72:07:50:37:73

## 응답결과 예시 (근무 중)
{
  "datetime": "2026-04-09 02:30:00",
  "work_date": "2026-04-08",
  "shift_idx": 2
}

## 응답결과 예시 (근무 외 또는 mac 없음)
{
  "datetime": "2026-04-09 02:30:00"
}
*/
