<?php
/**
 * Rate Color Management API Handler
 * SAP Fiori Based Rate Color Management API
 */

header('Content-Type: application/json; charset=utf-8');
require_once(__DIR__ . '/../../../lib/db.php');

// CORS 설정
header('Access-Control-Allow-Origin: *');
header('Access-Control-Allow-Methods: GET, POST, PUT, DELETE');
header('Access-Control-Allow-Headers: Content-Type');

/**
 * Rate Color 테이블 생성
 */
function createRateColorTable($pdo) {
  $sql = "CREATE TABLE IF NOT EXISTS info_rate_color (
    idx INT(11) NOT NULL PRIMARY KEY,
    start_rate INT(11) NOT NULL DEFAULT 0,
    end_rate INT(11) NOT NULL DEFAULT 0,
    color VARCHAR(7) NOT NULL DEFAULT '#6b7884',
    created_at DATETIME DEFAULT CURRENT_TIMESTAMP,
    updated_at DATETIME DEFAULT CURRENT_TIMESTAMP ON UPDATE CURRENT_TIMESTAMP
  ) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_unicode_ci";
  
  $pdo->exec($sql);
}

try {
  $method = $_SERVER['REQUEST_METHOD'];
  $action = $_GET['action'] ?? '';
  
  switch ($method) {
    case 'GET':
      handleGetRequest($pdo, $action);
      break;
    case 'POST':
      handlePostRequest($pdo, $action);
      break;
    case 'PUT':
      handlePutRequest($pdo, $action);
      break;
    case 'DELETE':
      handleDeleteRequest($pdo, $action);
      break;
    default:
      throw new Exception('지원하지 않는 HTTP 메소드입니다.');
  }
  
} catch (Exception $e) {
  error_log("Rate Color API Error: " . $e->getMessage());
  echo json_encode([
    'code' => '99',
    'msg' => $e->getMessage(),
    'data' => null
  ]);
}

/**
 * GET 요청 처리
 */
function handleGetRequest($pdo, $action) {
  switch ($action) {
    case 'list':
    case '':
      getRateColorList($pdo);
      break;
    case 'config':
      getRateColorConfig($pdo);
      break;
    default:
      throw new Exception('알 수 없는 GET 액션입니다.');
  }
}

/**
 * POST 요청 처리
 */
function handlePostRequest($pdo, $action) {
  $input = json_decode(file_get_contents('php://input'), true);
  
  switch ($action) {
    case 'save':
      saveRateColorConfig($pdo, $input);
      break;
    case 'reset':
      resetRateColorConfig($pdo);
      break;
    default:
      throw new Exception('알 수 없는 POST 액션입니다.');
  }
}

/**
 * Rate Color 목록 조회
 */
function getRateColorList($pdo) {
  $sql = "SELECT * FROM info_rate_color ORDER BY idx ASC";
  $stmt = $pdo->prepare($sql);
  $stmt->execute();
  $result = $stmt->fetchAll(PDO::FETCH_ASSOC);
  
  echo json_encode([
    'code' => '00',
    'msg' => 'ok',
    'data' => $result
  ]);
}

/**
 * Rate Color 설정 조회 (5단계 고정 시스템)
 */
function getRateColorConfig($pdo) {
  try {
    // 테이블 생성 (존재하지 않는 경우)
    createRateColorTable($pdo);
    
    $sql = "SELECT * FROM info_rate_color ORDER BY idx ASC";
    $stmt = $pdo->prepare($sql);
    $stmt->execute();
    $data = $stmt->fetchAll(PDO::FETCH_ASSOC);
    
    // 기본 5단계 설정 정의
    $defaultStages = [
      1 => ['start_rate' => 0, 'end_rate' => 25, 'color' => '#6b7884'],      // Stage 1: 0% (고정)
      2 => ['start_rate' => 25, 'end_rate' => 50, 'color' => '#da1e28'],     // Stage 2: 양쪽 슬라이더
      3 => ['start_rate' => 50, 'end_rate' => 80, 'color' => '#e26b0a'],     // Stage 3: 양쪽 슬라이더  
      4 => ['start_rate' => 80, 'end_rate' => 100, 'color' => '#30914c'],    // Stage 4: 100% (고정)
      5 => ['start_rate' => 100, 'end_rate' => 999, 'color' => '#0070f2']    // Stage 5: 100% 초과 (고정)
    ];
    
    // 데이터베이스 데이터가 있으면 병합, 없으면 기본값 사용
    $config = [];
    for ($i = 1; $i <= 5; $i++) {
      $dbData = null;
      foreach ($data as $row) {
        if ((int)$row['idx'] === $i) {
          $dbData = $row;
          break;
        }
      }
      
      $stageKey = "stage{$i}";
      if ($dbData) {
        // 데이터베이스 데이터 사용
        $config[$stageKey] = [
          'start_rate' => (int)$dbData['start_rate'],
          'end_rate' => (int)$dbData['end_rate'],
          'color' => $dbData['color']
        ];
      } else {
        // 기본값 사용
        $config[$stageKey] = $defaultStages[$i];
      }
    }
    
    echo json_encode([
      'code' => '00',
      'msg' => 'ok',
      'data' => $config
    ]);
    
  } catch (Exception $e) {
    error_log("Rate Color Config Error: " . $e->getMessage());
    
    // 오류 발생 시 완전한 기본 설정 반환
    $defaultConfig = [
      'stage1' => ['start_rate' => 0, 'end_rate' => 25, 'color' => '#6b7884'],
      'stage2' => ['start_rate' => 25, 'end_rate' => 50, 'color' => '#da1e28'],
      'stage3' => ['start_rate' => 50, 'end_rate' => 80, 'color' => '#e26b0a'],
      'stage4' => ['start_rate' => 80, 'end_rate' => 100, 'color' => '#30914c'],
      'stage5' => ['start_rate' => 100, 'end_rate' => 999, 'color' => '#0070f2']
    ];
    
    echo json_encode([
      'code' => '00',
      'msg' => '기본 설정 반환 (오류 복구)',
      'data' => $defaultConfig
    ]);
  }
}

/**
 * Rate Color 설정 저장 (5단계 시스템 유효성 검사 포함)
 */
function saveRateColorConfig($pdo, $input) {
  if (!isset($input['config'])) {
    throw new Exception('설정 데이터가 제공되지 않았습니다.');
  }
  
  $config = $input['config'];
  
  // 5단계 시스템 유효성 검사
  $validationResult = validateStageSystem($config);
  if (!$validationResult['valid']) {
    throw new Exception($validationResult['error']);
  }
  
  try {
    $pdo->beginTransaction();
    
    // 테이블 생성 (존재하지 않는 경우)
    createRateColorTable($pdo);
    
    // 기존 데이터 삭제
    $stmt = $pdo->prepare("DELETE FROM info_rate_color");
    $stmt->execute();
    
    // 5단계 데이터 순서대로 삽입
    $sql = "INSERT INTO info_rate_color (idx, start_rate, end_rate, color) VALUES (?, ?, ?, ?)";
    $stmt = $pdo->prepare($sql);
    
    for ($i = 1; $i <= 5; $i++) {
      $stageKey = "stage{$i}";
      if (isset($config[$stageKey])) {
        $stageData = $config[$stageKey];
        $stmt->execute([
          $i,
          (int)$stageData['start_rate'],
          (int)$stageData['end_rate'],
          $stageData['color']
        ]);
      }
    }
    
    $pdo->commit();
    
    echo json_encode([
      'code' => '00',
      'msg' => 'Rate Color 설정이 성공적으로 저장되었습니다.',
      'data' => [
        'saved_count' => 5,
        'validation' => 'passed',
        'stages' => array_keys($config)
      ]
    ]);
    
  } catch (Exception $e) {
    $pdo->rollBack();
    throw new Exception('설정 저장 중 오류가 발생했습니다: ' . $e->getMessage());
  }
}

/**
 * Rate Color 설정 기본값으로 리셋
 */
function resetRateColorConfig($pdo) {
  try {
    $pdo->beginTransaction();
    
    // 기존 데이터 삭제
    $stmt = $pdo->prepare("DELETE FROM info_rate_color");
    $stmt->execute();
    
    // 기본 설정 삽입
    $defaultConfig = [
      [1, 0, 0, '#6b7884'],       // 1단계: 0%
      [2, 0, 50, '#da1e28'],      // 2단계: 0% < rate <= 50%
      [3, 50, 80, '#e26b0a'],     // 3단계: 50% < rate <= 80%
      [4, 80, 100, '#30914c'],    // 4단계: 80% < rate <= 100%
      [5, 100, 999, '#0070f2']    // 5단계: 100% < rate
    ];
    
    $sql = "INSERT INTO info_rate_color (idx, start_rate, end_rate, color) VALUES (?, ?, ?, ?)";
    $stmt = $pdo->prepare($sql);
    
    foreach ($defaultConfig as $config) {
      $stmt->execute($config);
    }
    
    $pdo->commit();
    
    echo json_encode([
      'code' => '00',
      'msg' => 'Rate Color 설정이 기본값으로 리셋되었습니다.',
      'data' => ['reset_count' => count($defaultConfig)]
    ]);
    
  } catch (Exception $e) {
    $pdo->rollBack();
    throw new Exception('설정 리셋 중 오류가 발생했습니다: ' . $e->getMessage());
  }
}

/**
 * 5단계 시스템 유효성 검사 함수
 */
function validateStageSystem($config) {
  // 필수 단계 존재 확인
  for ($i = 1; $i <= 5; $i++) {
    $stageKey = "stage{$i}";
    if (!isset($config[$stageKey])) {
      return ['valid' => false, 'error' => "Stage {$i} 설정이 누락되었습니다."];
    }
    
    $stage = $config[$stageKey];
    if (!isset($stage['start_rate']) || !isset($stage['end_rate']) || !isset($stage['color'])) {
      return ['valid' => false, 'error' => "Stage {$i} 필수 필드가 누락되었습니다."];
    }
  }
  
  // 단계별 규칙 검증
  $stages = $config;
  
  // Stage 1: start_rate는 항상 0이어야 함
  if ((int)$stages['stage1']['start_rate'] !== 0) {
    return ['valid' => false, 'error' => 'Stage 1의 시작값은 반드시 0%이어야 합니다.'];
  }
  
  // Stage 4: end_rate 최대값 100% 검증
  if ((int)$stages['stage4']['end_rate'] > 100) {
    return ['valid' => false, 'error' => 'Stage 4의 끝값은 100%를 초과할 수 없습니다.'];
  }
  
  // Stage 5: 동적 시작값 설정 (Stage 4 end_rate 기준)
  $stage5StartRate = max((int)$stages['stage4']['end_rate'], 100);
  if ((int)$stages['stage5']['start_rate'] !== $stage5StartRate || (int)$stages['stage5']['end_rate'] !== 999) {
    // Stage 5 값 자동 수정
    $stages['stage5']['start_rate'] = $stage5StartRate;
    $stages['stage5']['end_rate'] = 999;
    error_log("Stage 5 자동 수정: {$stage5StartRate}% ~ 999%");
  }
  
  // 단계 간 연속성 검증
  for ($i = 1; $i <= 4; $i++) {
    $currentStage = $stages["stage{$i}"];
    $nextStage = $stages["stage" . ($i + 1)];
    
    // 현재 단계의 end_rate와 다음 단계의 start_rate가 일치해야 함
    if ((int)$currentStage['end_rate'] !== (int)$nextStage['start_rate']) {
      return ['valid' => false, 'error' => "Stage {$i}의 끝값과 Stage " . ($i + 1) . "의 시작값이 일치하지 않습니다."];
    }
    
    // 단계 내에서 start_rate < end_rate 검증 (Stage 5 제외)
    if ($i < 5 && (int)$currentStage['start_rate'] >= (int)$currentStage['end_rate']) {
      return ['valid' => false, 'error' => "Stage {$i}의 시작값이 끝값보다 크거나 같습니다."];
    }
  }
  
  // 색상 유효성 검사
  for ($i = 1; $i <= 5; $i++) {
    $color = $stages["stage{$i}"]['color'];
    if (!preg_match('/^#[0-9a-fA-F]{6}$/', $color)) {
      return ['valid' => false, 'error' => "Stage {$i}의 색상이 올바른 HEX 형식이 아닙니다."];
    }
  }
  
  return ['valid' => true, 'error' => null];
}

/**
 * Rate 값에 따른 색상 반환 (유틸리티 함수)
 */
function getRateColorForValue($pdo, $rate) {
  $sql = "SELECT color FROM info_rate_color 
          WHERE ? >= start_rate AND ? < end_rate 
          OR (? = 0 AND idx = 1)
          OR (? >= 100 AND idx = 5)
          ORDER BY idx ASC LIMIT 1";
  
  $stmt = $pdo->prepare($sql);
  $stmt->execute([$rate, $rate, $rate, $rate]);
  $result = $stmt->fetch(PDO::FETCH_ASSOC);
  
  return $result ? $result['color'] : '#6b7884'; // 기본 색상
}

/**
 * PUT 요청 처리 (개별 Stage 업데이트)
 */
function handlePutRequest($pdo, $action) {
  $input = json_decode(file_get_contents('php://input'), true);
  
  switch ($action) {
    case 'stage':
      updateSingleStage($pdo, $input);
      break;
    default:
      throw new Exception('알 수 없는 PUT 액션입니다.');
  }
}

/**
 * 개별 Stage 업데이트
 */
function updateSingleStage($pdo, $input) {
  if (!isset($input['idx']) || !isset($input['start_rate']) || !isset($input['end_rate']) || !isset($input['color'])) {
    throw new Exception('필수 필드가 누락되었습니다.');
  }
  
  $sql = "UPDATE info_rate_color SET start_rate = ?, end_rate = ?, color = ? WHERE idx = ?";
  $stmt = $pdo->prepare($sql);
  $result = $stmt->execute([
    (int)$input['start_rate'],
    (int)$input['end_rate'],
    $input['color'],
    (int)$input['idx']
  ]);
  
  if ($result) {
    echo json_encode([
      'code' => '00',
      'msg' => 'Stage 설정이 업데이트되었습니다.',
      'data' => ['updated_idx' => $input['idx']]
    ]);
  } else {
    throw new Exception('Stage 업데이트에 실패했습니다.');
  }
}

/**
 * DELETE 요청 처리
 */
function handleDeleteRequest($pdo, $action) {
  switch ($action) {
    case 'all':
      clearAllRateColors($pdo);
      break;
    default:
      throw new Exception('알 수 없는 DELETE 액션입니다.');
  }
}

/**
 * 모든 Rate Color 설정 삭제
 */
function clearAllRateColors($pdo) {
  $stmt = $pdo->prepare("DELETE FROM info_rate_color");
  $result = $stmt->execute();
  
  if ($result) {
    echo json_encode([
      'code' => '00',
      'msg' => '모든 Rate Color 설정이 삭제되었습니다.',
      'data' => ['deleted_count' => $stmt->rowCount()]
    ]);
  } else {
    throw new Exception('Rate Color 설정 삭제에 실패했습니다.');
  }
}

?>