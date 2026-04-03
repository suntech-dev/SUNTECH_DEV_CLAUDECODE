<?php

/**
 * proc/rate_color.php — Rate Color 5단계 색상 설정 API
 * GET    ?action=list   : info_rate_color 전체 목록 반환 (원시 데이터)
 * GET    ?action=config : stage1~stage5 키 형태로 변환된 설정 반환 (DB 없으면 기본값)
 * POST   ?action=save   : JSON body로 5단계 설정 저장 (전체 삭제 후 재삽입)
 * POST   ?action=reset  : 기본 5단계 설정으로 초기화
 * PUT    ?action=stage  : 단일 Stage 업데이트
 * DELETE ?action=all    : 모든 설정 삭제
 *
 * Rate Color: OEE / 생산율(%) 수치 범위에 따라 다른 색상을 표시하는 마스터 설정
 * 예) 0~25%: 회색, 25~50%: 빨간, 50~80%: 주황, 80~100%: 녹색, 100~999%: 파란
 *
 * 응답 형식: { code: '00'(성공)/'99'(오류), msg: string, data: any }
 */
header('Content-Type: application/json; charset=utf-8');
require_once(__DIR__ . '/../../../lib/db.php');

// CORS 헤더 (대시보드 등 다른 오리진에서 Rate Color 조회 가능하도록)
header('Access-Control-Allow-Origin: *');
header('Access-Control-Allow-Methods: GET, POST, PUT, DELETE');
header('Access-Control-Allow-Headers: Content-Type');

/**
 * 기본 5단계 Rate Color 설정 상수
 * - idx 1~5에 대응하며 start_rate~end_rate 범위에 color 지정
 * - Stage 5의 end_rate=999: "100% 초과" 구간을 의미하는 상한 없는 값
 * - DB 조회 실패 시 이 기본값을 폴백(fallback)으로 반환
 */
const DEFAULT_RATE_COLOR_STAGES = [
    1 => ['start_rate' => 0,   'end_rate' => 25,  'color' => '#6b7884'],
    2 => ['start_rate' => 25,  'end_rate' => 50,  'color' => '#da1e28'],
    3 => ['start_rate' => 50,  'end_rate' => 80,  'color' => '#e26b0a'],
    4 => ['start_rate' => 80,  'end_rate' => 100, 'color' => '#30914c'],
    5 => ['start_rate' => 100, 'end_rate' => 999, 'color' => '#0070f2'],
];

/**
 * info_rate_color 테이블 생성 (없는 경우에만)
 * - IF NOT EXISTS: 이미 테이블이 있으면 무시 (멱등성 보장)
 * - idx는 1~5 Stage 번호를 PRIMARY KEY로 사용 (AUTO_INCREMENT 아님)
 * - ON UPDATE CURRENT_TIMESTAMP: 수정 시 updated_at 자동 갱신
 */
function createRateColorTable(PDO $pdo): void
{
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

// HTTP 메서드 및 action 파라미터에 따라 처리 함수 분기
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
            throw new Exception('This is an unsupported HTTP method.');
    }
} catch (Exception $e) {
    // 오류 로그 기록 후 code='99' 형태로 응답
    error_log("Rate Color API Error: " . $e->getMessage());
    echo json_encode([
        'code' => '99',
        'msg' => $e->getMessage(),
        'data' => null
    ]);
}

/**
 * GET 요청 처리 분기
 * - action='' 또는 'list': 원시 DB 레코드 반환
 * - action='config': stage1~stage5 구조화된 형태로 반환
 */
function handleGetRequest(PDO $pdo, string $action): void
{
    switch ($action) {
        case 'list':
        case '':
            getRateColorList($pdo);
            break;
        case 'config':
            getRateColorConfig($pdo);
            break;
        default:
            throw new Exception('This is an unknown GET action.');
    }
}

/**
 * POST 요청 처리 분기
 * - action='save': JSON body { config: { stage1: {...}, ... } } 형태의 설정 저장
 * - action='reset': 기본 5단계 설정으로 초기화
 * - php://input으로 JSON body 파싱 (form-data 아닌 JSON 요청 대응)
 */
function handlePostRequest(PDO $pdo, string $action): void
{
    $input = json_decode(file_get_contents('php://input'), true);

    switch ($action) {
        case 'save':
            saveRateColorConfig($pdo, $input);
            break;
        case 'reset':
            resetRateColorConfig($pdo);
            break;
        default:
            throw new Exception('This is an unknown POST action.');
    }
}

/**
 * Rate Color 목록 조회 (원시 DB 데이터)
 * - idx ASC 정렬: Stage 1→5 순서 보장
 * - 주로 내부 확인/디버그 용도, 화면 표시에는 getRateColorConfig() 사용
 */
function getRateColorList(PDO $pdo): void
{
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
 * Rate Color 설정 조회 (stage1~stage5 구조화 형태)
 * - createRateColorTable(): 테이블이 없으면 자동 생성 (초기 설치 시 안전)
 * - DB 레코드를 idx 기준으로 인덱싱 후 stage1~stage5 키로 재구성
 * - DB에 없는 stage는 DEFAULT_RATE_COLOR_STAGES 기본값으로 채움
 * - DB 오류 시 기본값을 폴백(fallback)으로 반환 (서비스 중단 방지)
 */
function getRateColorConfig(PDO $pdo): void
{
    try {
        // 테이블 자동 생성 (없을 경우)
        createRateColorTable($pdo);

        $stmt = $pdo->prepare("SELECT * FROM info_rate_color ORDER BY idx ASC");
        $stmt->execute();
        $rows = $stmt->fetchAll(PDO::FETCH_ASSOC);

        // idx 기준으로 DB 데이터 인덱싱 (빠른 stage 번호 조회용)
        $dbByIdx = [];
        foreach ($rows as $row) {
            $dbByIdx[(int)$row['idx']] = $row;
        }

        // stage1~stage5 키 형태로 변환: DB 데이터 우선, 없으면 기본값 사용
        $config = [];
        for ($i = 1; $i <= 5; $i++) {
            $stageKey = "stage{$i}";
            if (isset($dbByIdx[$i])) {
                // DB에 해당 stage가 있으면 DB 값 사용
                $config[$stageKey] = [
                    'start_rate' => (int)$dbByIdx[$i]['start_rate'],
                    'end_rate'   => (int)$dbByIdx[$i]['end_rate'],
                    'color'      => $dbByIdx[$i]['color'],
                ];
            } else {
                // DB에 없는 stage는 상수 기본값으로 채움
                $config[$stageKey] = DEFAULT_RATE_COLOR_STAGES[$i];
            }
        }

        echo json_encode(['code' => '00', 'msg' => 'ok', 'data' => $config]);
    } catch (Exception $e) {
        // DB 오류 시 기본값 폴백 반환 (대시보드 렌더링 중단 방지)
        error_log("Rate Color Config Error: " . $e->getMessage());

        $fallback = [];
        foreach (DEFAULT_RATE_COLOR_STAGES as $i => $stage) {
            $fallback["stage{$i}"] = $stage;
        }
        echo json_encode(['code' => '00', 'msg' => 'Return default settings (error recovery)', 'data' => $fallback]);
    }
}

/**
 * Rate Color 설정 저장 (5단계 전체 교체)
 * - validateStageSystem(): 저장 전 5단계 연속성/유효성 검사 (실패 시 예외)
 * - 트랜잭션: DELETE ALL → INSERT 5건을 원자적으로 처리
 *   (중간 실패 시 rollBack()으로 이전 데이터 유지)
 * - createRateColorTable(): 테이블이 없으면 자동 생성 후 저장
 */
function saveRateColorConfig(PDO $pdo, ?array $input): void
{
    if (!isset($input['config'])) {
        throw new Exception('Configuration data was not provided.');
    }

    $config = $input['config'];

    // 5단계 시스템 유효성 검사 (연속성, 시작/끝값 규칙, HEX 색상 형식)
    $validationResult = validateStageSystem($config);
    if (!$validationResult['valid']) {
        throw new Exception($validationResult['error']);
    }

    try {
        $pdo->beginTransaction();

        // 테이블 생성 (존재하지 않는 경우)
        createRateColorTable($pdo);

        // 기존 전체 삭제 후 재삽입 (UPSERT 대신 REPLACE 방식 사용)
        $stmt = $pdo->prepare("DELETE FROM info_rate_color");
        $stmt->execute();

        // 5단계 데이터 순서대로 삽입 (idx = stage 번호 1~5)
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
            'msg' => 'Rate Color settings have been successfully saved.',
            'data' => [
                'saved_count' => 5,
                'validation' => 'passed',
                'stages' => array_keys($config)
            ]
        ]);
    } catch (Exception $e) {
        $pdo->rollBack();
        throw new Exception('An error occurred while saving settings: ' . $e->getMessage());
    }
}

/**
 * Rate Color 설정 기본값으로 초기화
 * - 트랜잭션: DELETE ALL → INSERT 5건 (기본 설정)
 * - 기본 설정 의미:
 *   idx=1: 0% (정확히 0인 경우)
 *   idx=2: 0~50% (0 초과 ~ 50% 이하)
 *   idx=3: 50~80%
 *   idx=4: 80~100%
 *   idx=5: 100~999% (초과 달성)
 */
function resetRateColorConfig(PDO $pdo): void
{
    try {
        $pdo->beginTransaction();

        // 기존 데이터 삭제
        $stmt = $pdo->prepare("DELETE FROM info_rate_color");
        $stmt->execute();

        // 기본 5단계 설정 삽입 (idx, start_rate, end_rate, color)
        $defaultConfig = [
            [1, 0, 0, '#6b7884'],       // 1단계: 0%
            [2, 0, 50, '#da1e28'],      // 2단계: 0% < rate <= 50%
            [3, 50, 80, '#e26b0a'],     // 3단계: 50% < rate <= 80%
            [4, 80, 100, '#30914c'],    // 4단계: 80% < rate <= 100%
            [5, 100, 999, '#0070f2']    // 5단계: 100% < rate (초과 달성)
        ];

        $sql = "INSERT INTO info_rate_color (idx, start_rate, end_rate, color) VALUES (?, ?, ?, ?)";
        $stmt = $pdo->prepare($sql);

        foreach ($defaultConfig as $config) {
            $stmt->execute($config);
        }

        $pdo->commit();

        echo json_encode([
            'code' => '00',
            'msg' => 'Rate Color settings have been reset to their default values.',
            'data' => ['reset_count' => count($defaultConfig)]
        ]);
    } catch (Exception $e) {
        $pdo->rollBack();
        throw new Exception('An error occurred while resetting settings: ' . $e->getMessage());
    }
}

/**
 * 5단계 Rate Color 시스템 유효성 검사
 * 검사 항목:
 * 1. stage1~stage5 모든 단계 존재 확인
 * 2. 각 단계의 start_rate / end_rate / color 필수 필드 존재 확인
 * 3. Stage 1 start_rate = 0 강제 (최소값은 항상 0%)
 * 4. Stage 4 end_rate <= 100 검증 (Stage 4는 100% 초과 불가)
 * 5. Stage 5: Stage 4 end_rate 기준으로 start_rate 자동 설정, end_rate = 999 강제
 * 6. Stage 간 연속성: 이전 단계 end_rate = 다음 단계 start_rate 일치 확인
 * 7. 단계 내 start_rate < end_rate 검증 (유효 범위 확인)
 * 8. 색상 HEX 형식 검증: #RRGGBB 형식 (정규식)
 *
 * @return array { valid: bool, error: string|null }
 */
function validateStageSystem(array $config): array
{
    // 필수 단계 존재 확인 및 필수 필드 검증
    for ($i = 1; $i <= 5; $i++) {
        $stageKey = "stage{$i}";
        if (!isset($config[$stageKey])) {
            return ['valid' => false, 'error' => "Stage {$i} setting is missing."];
        }

        $stage = $config[$stageKey];
        if (!isset($stage['start_rate']) || !isset($stage['end_rate']) || !isset($stage['color'])) {
            return ['valid' => false, 'error' => "Stage {$i} required field is missing."];
        }
    }

    $stages = $config;

    // Stage 1: start_rate는 항상 0이어야 함 (최솟값 강제)
    if ((int)$stages['stage1']['start_rate'] !== 0) {
        return ['valid' => false, 'error' => 'The starting value of Stage 1 must be 0%.'];
    }

    // Stage 4: end_rate 최대값 100% 검증 (100% 초과는 Stage 5 영역)
    if ((int)$stages['stage4']['end_rate'] > 100) {
        return ['valid' => false, 'error' => 'The end value of Stage 4 cannot exceed 100%.'];
    }

    // Stage 5: start_rate = Stage 4 end_rate 기준으로 자동 설정, end_rate = 999 강제
    // max(..., 100): Stage 4 end_rate가 100 미만이어도 Stage 5는 최소 100부터 시작
    $stage5StartRate = max((int)$stages['stage4']['end_rate'], 100);
    if ((int)$stages['stage5']['start_rate'] !== $stage5StartRate || (int)$stages['stage5']['end_rate'] !== 999) {
        // Stage 5 값 자동 수정 (에러 반환 아닌 자동 보정)
        $stages['stage5']['start_rate'] = $stage5StartRate;
        $stages['stage5']['end_rate'] = 999;
        error_log("Stage 5 Auto-correction: {$stage5StartRate}% ~ 999%");
    }

    // 단계 간 연속성 검증: 이전 단계 end_rate = 다음 단계 start_rate 일치 확인
    for ($i = 1; $i <= 4; $i++) {
        $currentStage = $stages["stage{$i}"];
        $nextStage = $stages["stage" . ($i + 1)];

        // 연속성 검사: gap(공백)이나 overlap(겹침) 없이 이어져야 함
        if ((int)$currentStage['end_rate'] !== (int)$nextStage['start_rate']) {
            return ['valid' => false, 'error' => "The end value of Stage {$i} value of Stage " . ($i + 1) . "do not match."];
        }

        // 단계 내 유효 범위: start_rate < end_rate (Stage 5 제외 - 999는 무한대 의미)
        if ($i < 5 && (int)$currentStage['start_rate'] >= (int)$currentStage['end_rate']) {
            return ['valid' => false, 'error' => "The starting value of Stage {$i} is greater than or equal to the ending value."];
        }
    }

    // 색상 유효성 검사: #RRGGBB 형식 (6자리 HEX, # 포함)
    for ($i = 1; $i <= 5; $i++) {
        $color = $stages["stage{$i}"]['color'];
        if (!preg_match('/^#[0-9a-fA-F]{6}$/', $color)) {
            return ['valid' => false, 'error' => "The color of Stage {$i} is not in a valid HEX format."];
        }
    }

    return ['valid' => true, 'error' => null];
}

/**
 * 특정 rate 값에 해당하는 색상 반환 (내부 유틸 함수)
 * - rate >= start_rate AND rate < end_rate 조건으로 해당 Stage의 색상 조회
 * - 특수 처리: rate=0이면 idx=1, rate>=100이면 idx=5
 * - 매칭 없으면 기본 회색(#6b7884) 반환
 * - 현재 이 함수는 API 엔드포인트가 아니며, 다른 PHP 파일에서 require 후 직접 호출 가능
 */
function getRateColorForValue(PDO $pdo, int $rate): string
{
    $sql = "SELECT color FROM info_rate_color
          WHERE ? >= start_rate AND ? < end_rate
          OR (? = 0 AND idx = 1)
          OR (? >= 100 AND idx = 5)
          ORDER BY idx ASC LIMIT 1";

    $stmt = $pdo->prepare($sql);
    $stmt->execute([$rate, $rate, $rate, $rate]);
    $result = $stmt->fetch(PDO::FETCH_ASSOC);

    return $result ? $result['color'] : '#6b7884'; // 매칭 없으면 기본 회색
}

/**
 * PUT 요청 처리 분기
 * - action='stage': 단일 Stage 업데이트 (idx, start_rate, end_rate, color)
 * - php://input으로 JSON body 파싱
 */
function handlePutRequest(PDO $pdo, string $action): void
{
    $input = json_decode(file_get_contents('php://input'), true);

    switch ($action) {
        case 'stage':
            updateSingleStage($pdo, $input);
            break;
        default:
            throw new Exception('This is an unknown PUT action.');
    }
}

/**
 * 단일 Stage 업데이트 (PUT ?action=stage)
 * - 전체 재저장 없이 특정 stage만 개별 수정할 때 사용
 * - idx, start_rate, end_rate, color 모두 필수
 * - 연속성 검증은 수행하지 않음 (호출 측에서 보장 필요)
 */
function updateSingleStage(PDO $pdo, ?array $input): void
{
    if (!isset($input['idx']) || !isset($input['start_rate']) || !isset($input['end_rate']) || !isset($input['color'])) {
        throw new Exception('A required field is missing.');
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
            'msg' => 'Stage settings have been updated.',
            'data' => ['updated_idx' => $input['idx']]
        ]);
    } else {
        throw new Exception('Failed to update the stage.');
    }
}

/**
 * DELETE 요청 처리 분기
 * - action='all': 모든 Rate Color 설정 삭제 (테이블 데이터만 삭제, 테이블 유지)
 */
function handleDeleteRequest(PDO $pdo, string $action): void
{
    switch ($action) {
        case 'all':
            clearAllRateColors($pdo);
            break;
        default:
            throw new Exception('This is an unknown DELETE action.');
    }
}

/**
 * 모든 Rate Color 설정 삭제 (DELETE ?action=all)
 * - rowCount(): 실제 삭제된 행 수 반환 (0이면 이미 비어있는 상태)
 * - 삭제 후 저장/리셋 없이 조회하면 기본값 폴백으로 응답됨
 */
function clearAllRateColors(PDO $pdo): void
{
    $stmt = $pdo->prepare("DELETE FROM info_rate_color");
    $result = $stmt->execute();

    if ($result) {
        echo json_encode([
            'code' => '00',
            'msg' => 'All Rate Color settings have been deleted.',
            'data' => ['deleted_count' => $stmt->rowCount()]
        ]);
    } else {
        throw new Exception('Failed to delete Rate Color settings.');
    }
}
