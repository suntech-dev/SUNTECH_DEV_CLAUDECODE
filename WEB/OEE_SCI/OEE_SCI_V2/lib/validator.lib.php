<?php
/**
 * 입력 검증 라이브러리
 * 모든 API에서 사용할 수 있는 공통 검증 함수들을 제공합니다.
 * 
 * 사용법:
 * $validator = new InputValidator();
 * if (!$validator->validateMac($mac)) {
 *   throw new Exception($validator->getLastError());
 * }
 */

class InputValidator 
{
  private $lastError = '';
  private $errors = [];

  /**
   * MAC 주소 유효성 검증
   * 형식: XX:XX:XX:XX:XX:XX (대소문자 구분 없음)
   * 
   * @param string $mac MAC 주소
   * @return bool 유효하면 true, 아니면 false
   */
  public function validateMac($mac) 
  {
    if (empty($mac)) {
      $this->lastError = 'MAC 주소가 입력되지 않았습니다.';
      return false;
    }

    // MAC 주소를 대문자로 변환하여 검증
    $mac = strtoupper(trim($mac));
    
    // 정규식으로 MAC 주소 형식 검증 (XX:XX:XX:XX:XX:XX)
    if (!preg_match('/^([0-9A-F]{2}:){5}[0-9A-F]{2}$/', $mac)) {
      $this->lastError = 'MAC 주소 형식이 올바르지 않습니다. (예: 84:72:07:50:37:73)';
      return false;
    }

    return true;
  }

  /**
   * 숫자 범위 검증
   * 
   * @param mixed $value 검증할 값
   * @param int $min 최소값
   * @param int $max 최대값
   * @param string $fieldName 필드명 (에러 메시지용)
   * @return bool
   */
  public function validateNumberRange($value, $min = 0, $max = PHP_INT_MAX, $fieldName = '값') 
  {
    // 숫자로 변환 가능한지 확인
    if (!is_numeric($value)) {
      $this->lastError = "{$fieldName}은(는) 숫자여야 합니다.";
      return false;
    }

    $numValue = (int)$value;

    // 범위 검증
    if ($numValue < $min || $numValue > $max) {
      $this->lastError = "{$fieldName}은(는) {$min}과 {$max} 사이의 값이어야 합니다.";
      return false;
    }

    return true;
  }

  /**
   * 문자열 길이 검증
   * 
   * @param string $value 검증할 문자열
   * @param int $minLength 최소 길이
   * @param int $maxLength 최대 길이
   * @param string $fieldName 필드명
   * @return bool
   */
  public function validateStringLength($value, $minLength = 1, $maxLength = 255, $fieldName = '문자열') 
  {
    $length = strlen(trim($value));

    if ($length < $minLength) {
      $this->lastError = "{$fieldName}은(는) 최소 {$minLength}자 이상이어야 합니다.";
      return false;
    }

    if ($length > $maxLength) {
      $this->lastError = "{$fieldName}은(는) 최대 {$maxLength}자 이하여야 합니다.";
      return false;
    }

    return true;
  }

  /**
   * 안전한 파일명 검증 (경로 탐색 공격 방지)
   * 
   * @param string $filename 파일명
   * @return bool
   */
  public function validateSafeFilename($filename) 
  {
    // 빈 값 체크
    if (empty(trim($filename))) {
      $this->lastError = '파일명이 비어있습니다.';
      return false;
    }

    // 위험한 문자들 체크 (경로 탐색 방지)
    $dangerousChars = ['..', '/', '\\', '<', '>', ':', '"', '|', '?', '*'];
    foreach ($dangerousChars as $char) {
      if (strpos($filename, $char) !== false) {
        $this->lastError = '파일명에 허용되지 않는 문자가 포함되어 있습니다.';
        return false;
      }
    }

    // 화이트리스트 방식: 영문, 숫자, 한글, 일부 특수문자만 허용
    if (!preg_match('/^[a-zA-Z0-9가-힣._-]+$/', $filename)) {
      $this->lastError = '파일명은 영문, 숫자, 한글, 점(.), 하이픈(-), 언더스코어(_)만 사용할 수 있습니다.';
      return false;
    }

    return true;
  }

  /**
   * IP 주소 검증
   * 
   * @param string $ip IP 주소
   * @return bool
   */
  public function validateIP($ip) 
  {
    if (!filter_var($ip, FILTER_VALIDATE_IP)) {
      $this->lastError = '올바른 IP 주소가 아닙니다.';
      return false;
    }

    return true;
  }

  /**
   * 날짜 형식 검증 (Y-m-d)
   * 
   * @param string $date 날짜 문자열
   * @return bool
   */
  public function validateDate($date) 
  {
    if (!preg_match('/^\d{4}-\d{2}-\d{2}$/', $date)) {
      $this->lastError = '날짜 형식이 올바르지 않습니다. (YYYY-MM-DD 형식 사용)';
      return false;
    }

    // 실제 날짜인지 확인
    $dateParts = explode('-', $date);
    if (!checkdate($dateParts[1], $dateParts[2], $dateParts[0])) {
      $this->lastError = '유효하지 않은 날짜입니다.';
      return false;
    }

    return true;
  }

  /**
   * 여러 필드 동시 검증
   * 
   * @param array $rules 검증 규칙 배열
   * @param array $data 검증할 데이터 배열
   * @return bool 모든 검증이 통과하면 true
   */
  public function validateMultiple($rules, $data) 
  {
    $this->errors = [];
    $allValid = true;

    foreach ($rules as $field => $rule) {
      $value = $data[$field] ?? '';
      
      switch ($rule['type']) {
        case 'mac':
          if (!$this->validateMac($value)) {
            $this->errors[$field] = $this->lastError;
            $allValid = false;
          }
          break;
          
        case 'number':
          if (!$this->validateNumberRange($value, $rule['min'] ?? 0, $rule['max'] ?? PHP_INT_MAX, $field)) {
            $this->errors[$field] = $this->lastError;
            $allValid = false;
          }
          break;
          
        case 'string':
          if (!$this->validateStringLength($value, $rule['min'] ?? 1, $rule['max'] ?? 255, $field)) {
            $this->errors[$field] = $this->lastError;
            $allValid = false;
          }
          break;
      }
    }

    return $allValid;
  }

  /**
   * 마지막 에러 메시지 반환
   * 
   * @return string
   */
  public function getLastError() 
  {
    return $this->lastError;
  }

  /**
   * 모든 에러 메시지 반환 (여러 필드 검증 시)
   * 
   * @return array
   */
  public function getAllErrors() 
  {
    return $this->errors;
  }

  /**
   * 안전한 HTML 문자열 변환 (XSS 방지)
   * 
   * @param string $input 입력 문자열
   * @return string 안전한 문자열
   */
  public function sanitizeHtml($input) 
  {
    return htmlspecialchars($input, ENT_QUOTES, 'UTF-8');
  }

  /**
   * 안전한 SQL 문자열 처리 (추가 보안 레이어)
   * 참고: PDO prepared statements가 주요 방어이며, 이는 추가 보안입니다.
   * 
   * @param string $input 입력 문자열
   * @return string 정제된 문자열
   */
  public function sanitizeString($input) 
  {
    return trim(strip_tags($input));
  }
}

/**
 * 전역 헬퍼 함수들 (기존 코드와의 호환성을 위해)
 */

/**
 * MAC 주소 검증 헬퍼 함수
 * 
 * @param string $mac MAC 주소
 * @return bool
 */
function isValidMac($mac) 
{
  $validator = new InputValidator();
  return $validator->validateMac($mac);
}

/**
 * 안전한 파일명 검증 헬퍼 함수
 * 
 * @param string $filename 파일명
 * @return bool
 */
function isSafeFilename($filename) 
{
  $validator = new InputValidator();
  return $validator->validateSafeFilename($filename);
}

/**
 * JSON 응답 생성 헬퍼 함수 (기존 jsonReturn 함수 개선)
 * 
 * @param array $data 응답 데이터
 * @param int $httpCode HTTP 상태 코드 (기본값: 200)
 */
function jsonResponse($data, $httpCode = 200) 
{
  // HTTP 상태 코드 설정
  http_response_code($httpCode);
  
  // JSON 헤더 설정
  header('Content-Type: application/json; charset=utf-8');
  
  // CORS 헤더 (필요한 경우)
  header('Access-Control-Allow-Origin: *');
  header('Access-Control-Allow-Methods: GET, POST, OPTIONS');
  header('Access-Control-Allow-Headers: Content-Type');
  
  // JSON 출력 (유니코드 안전하게)
  echo json_encode($data, JSON_UNESCAPED_UNICODE | JSON_UNESCAPED_SLASHES);
  exit();
}