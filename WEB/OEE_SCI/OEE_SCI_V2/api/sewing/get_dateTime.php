<?php
## New CPU 패턴재봉기 & 일반재봉기 통합 버전 & 안돈, 경광등 ##
## 현재 시간을 조회

// 공통 헬퍼 라이브러리 로드
require_once(__DIR__ . '/../../lib/api_helper.lib.php');

// API 헬퍼 초기화
$apiHelper = new ApiHelper($pdo);

$today = date('Y-m-d H:i:s'); // 현재 날짜 및 시간

// 응답 데이터 구성 (공통 함수 사용)
$response = $apiHelper->createResponse_onlyItems(['datetime' => $today]);

/*
## API
get_dateTime.php

## API 예시
http://49.247.26.228/2025/sci/new/api/index.php?code=get_dateTime

## 응답결과 예시
{
  "datetime": "2025-03-31 10:39:59"
}
*/