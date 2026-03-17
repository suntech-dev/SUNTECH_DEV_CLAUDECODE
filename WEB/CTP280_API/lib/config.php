<?php

/**
 * 데이터베이스 설정 파일
 * 보안을 위해 환경변수를 우선 사용하고, 없으면 기본값을 사용
 * 운영환경에서는 반드시 .env 파일 또는 환경변수를 설정하세요
 */

// .env 파일이 있으면 로드 (선택사항)
/* if (file_exists(__DIR__ . '/../.env')) {
  $lines = file(__DIR__ . '/../.env', FILE_IGNORE_NEW_LINES | FILE_SKIP_EMPTY_LINES);
  foreach ($lines as $line) {
    if (strpos($line, '=') !== false && substr($line, 0, 1) !== '#') {
      list($key, $value) = explode('=', $line, 2);
      $_ENV[trim($key)] = trim($value);
    }
  }
} */

// 환경변수에서 설정값 가져오기 (없으면 기본값 사용 - 개발용)
$servername = $_ENV['DB_HOST'] ?? "49.247.26.228";
$username = $_ENV['DB_USERNAME'] ?? "root";
$password = $_ENV['DB_PASSWORD'] ?? "suntech9304!"; // 운영환경에서는 반드시 변경!
$dbname = $_ENV['DB_NAME'] ?? "ctp280_api_test";

// 설정값 검증 (운영환경에서는 더 엄격하게)
if (empty($servername) || empty($username) || empty($dbname)) {
  throw new Exception('데이터베이스 설정이 완전하지 않습니다. 환경변수를 확인하세요.');
}
