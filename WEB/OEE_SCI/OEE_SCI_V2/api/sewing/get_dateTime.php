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
// 응답 데이터 구성
// createResponse_onlyItems(): code 없이 데이터만 포함하는 응답 배열 생성
// 반환 형식: { "datetime": "YYYY-MM-DD HH:MM:SS" }
// ─────────────────────────────────────────────────────────────────────
$response = $apiHelper->createResponse_onlyItems(['datetime' => $today]);

/*
## API
get_dateTime.php

## API 예시
http://49.247.26.228/OEE_SCI/OEE_SCI_V2/api/index.php?code=get_dateTime

## 응답결과 예시
{
  "datetime": "2025-03-31 10:39:59"
}
*/
