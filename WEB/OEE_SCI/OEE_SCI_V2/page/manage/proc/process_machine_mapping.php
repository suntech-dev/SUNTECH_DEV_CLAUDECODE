<?php
/**
 * proc/process_machine_mapping.php — 공정-기계 매핑 관리 API
 * GET  ?action=get_mapping        : 공정 목록 + 기계 목록 + 기계-공정 그룹화 데이터 반환
 * POST ?action (form body)
 *   - action=update_process       : 공정명 및 표준 필요 기계 수 수정
 *   - action=update_machine_assignments : 기계의 공정 배정 일괄 업데이트
 *
 * 공정-기계 매핑: 어떤 공정(design_process)에 어떤 기계(machine)가 배정되어 있는지 관리
 * info_machine.design_process_idx 컬럼으로 연결됨
 * std_mc_needed: 해당 공정을 수행하기 위한 표준 기계 수 (OEE 계산에 사용)
 */
require_once(__DIR__ . '/../../../lib/config.php');
require_once(__DIR__ . '/../../../lib/db.php');

header('Content-Type: application/json');

// HTML Form에서 PUT 전송 불가 → _method 필드로 메서드 오버라이드
$method = $_SERVER['REQUEST_METHOD'];
if ($method === 'POST' && isset($_POST['_method'])) {
    $method = strtoupper($_POST['_method']);
}

// HTTP 메서드 및 action 파라미터에 따라 처리 함수 분기
try {
    switch ($method) {
        case 'GET':
            if (isset($_GET['action']) && $_GET['action'] === 'get_mapping') {
                getProcessMachineMapping($pdo);
            } else {
                http_response_code(400);
                echo json_encode(['success' => false, 'message' => 'Invalid action']);
            }
            break;
        case 'POST':
            if (isset($_POST['action'])) {
                switch ($_POST['action']) {
                    case 'update_process':
                        // 공정명 및 std_mc_needed 수정
                        updateProcessInfo($pdo);
                        break;
                    case 'update_machine_assignments':
                        // 기계-공정 배정 일괄 업데이트
                        updateMachineAssignments($pdo);
                        break;
                    default:
                        http_response_code(400);
                        echo json_encode(['success' => false, 'message' => 'Invalid action']);
                }
            } else {
                http_response_code(400);
                echo json_encode(['success' => false, 'message' => 'Action is required']);
            }
            break;
        default:
            http_response_code(405);
            echo json_encode(['success' => false, 'message' => 'Method Not Allowed']);
            break;
    }
} catch (PDOException $e) {
    http_response_code(500);
    echo json_encode(['success' => false, 'message' => 'Database error: ' . $e->getMessage()]);
} catch (Exception $e) {
    http_response_code(500);
    echo json_encode(['success' => false, 'message' => 'An unexpected error occurred: ' . $e->getMessage()]);
}

/**
 * Process-Machine 매핑 데이터를 조회합니다.
 * - info_design_process: 모든 프로세스 정보 (Factory/Line 포함)
 * - info_machine: 모든 머신 정보 (design_process_idx로 그룹화)
 *
 * 응답 구조:
 * {
 *   processes: [ { idx, design_process, factory_name, line_name, std_mc_needed, ... } ],
 *   machines:  [ { idx, machine_no, design_process_idx, factory_name, line_name, ... } ],
 *   grouped_machines: { [design_process_idx]: [ machine, ... ] }
 * }
 * - grouped_machines: 프론트에서 각 공정 카드에 배정된 기계 목록을 빠르게 렌더링하는 데 사용
 * - design_process_idx=0 또는 null: 아직 배정되지 않은 기계 그룹
 *
 * @param PDO $pdo PDO 객체
 */
function getProcessMachineMapping(PDO $pdo) {
    try {
        // 1. 활성 공정(status='Y') 조회 (공장명 > 라인명 > 공정명 순 정렬)
        $process_stmt = $pdo->prepare("
            SELECT
                dp.idx,
                dp.design_process,
                dp.model_name,
                dp.std_mc_needed,
                dp.status,
                dp.factory_idx,
                dp.line_idx,
                f.factory_name,
                l.line_name
            FROM info_design_process dp
            LEFT JOIN info_factory f ON dp.factory_idx = f.idx
            LEFT JOIN info_line l ON dp.line_idx = l.idx
            WHERE dp.status = 'Y'
            ORDER BY f.factory_name ASC, l.line_name ASC, dp.design_process ASC
        ");
        $process_stmt->execute();
        $processes = $process_stmt->fetchAll(PDO::FETCH_ASSOC);

        // 2. 활성 기계(status='Y') 조회 (공장명 > 라인명 > 기계번호 순 정렬)
        $machine_stmt = $pdo->prepare("
            SELECT
                m.idx,
                m.machine_no,
                m.design_process_idx,
                m.factory_idx,
                m.line_idx,
                m.mac,
                f.factory_name,
                l.line_name
            FROM info_machine m
            LEFT JOIN info_factory f ON m.factory_idx = f.idx
            LEFT JOIN info_line l ON m.line_idx = l.idx
            WHERE m.status = 'Y'
            ORDER BY f.factory_name ASC, l.line_name ASC, m.machine_no ASC
        ");
        $machine_stmt->execute();
        $machines = $machine_stmt->fetchAll(PDO::FETCH_ASSOC);

        // 3. Machine을 design_process_idx별로 그룹화
        // - 키: design_process_idx (0=미배정, 양수=해당 공정에 배정된 기계 그룹)
        // - 프론트에서 공정 카드별 기계 목록을 O(1)로 접근 가능하게 함
        $grouped_machines = [];
        foreach ($machines as $machine) {
            $process_idx = $machine['design_process_idx'] ?? 0;
            if (!isset($grouped_machines[$process_idx])) {
                $grouped_machines[$process_idx] = [];
            }
            $grouped_machines[$process_idx][] = $machine;
        }

        // 응답 데이터 구성: 3개의 데이터 집합을 한 번에 반환하여 AJAX 요청 최소화
        echo json_encode([
            'success' => true,
            'data' => [
                'processes' => $processes,
                'machines' => $machines,
                'grouped_machines' => $grouped_machines
            ]
        ]);
    } catch (Exception $e) {
        http_response_code(500);
        echo json_encode([
            'success' => false,
            'message' => 'Failed to fetch mapping data: ' . $e->getMessage()
        ]);
    }
}

/**
 * Process 정보를 업데이트합니다.
 * (design_process 이름과 std_mc_needed 값)
 *
 * - std_mc_needed >= 1 강제: 공정에는 최소 1대 이상 기계 필요
 * - 중복 이름(23000 UNIQUE 제약 오류) 시 409 반환
 *
 * @param PDO $pdo PDO 객체
 */
function updateProcessInfo(PDO $pdo) {
    $idx = (int)($_POST['idx'] ?? 0);
    $design_process = trim($_POST['design_process'] ?? '');
    $std_mc_needed = (int)($_POST['std_mc_needed'] ?? 1);

    // 필수값 검증
    if (empty($idx) || empty($design_process)) {
        http_response_code(400);
        echo json_encode(['success' => false, 'message' => 'Process ID and name are required.']);
        return;
    }

    // std_mc_needed 최솟값 1 강제
    if ($std_mc_needed < 1) {
        http_response_code(400);
        echo json_encode(['success' => false, 'message' => 'Standard MC needed must be at least 1.']);
        return;
    }

    try {
        $stmt = $pdo->prepare("
            UPDATE info_design_process
            SET design_process = ?, std_mc_needed = ?, update_date = NOW()
            WHERE idx = ?
        ");
        $stmt->execute([$design_process, $std_mc_needed, $idx]);

        echo json_encode([
            'success' => true,
            'message' => 'Process information updated successfully.'
        ]);
    } catch (PDOException $e) {
        if ($e->getCode() == '23000') {
            // UNIQUE 제약 위반: 이미 동일한 공정명 존재
            http_response_code(409);
            echo json_encode(['success' => false, 'message' => 'Process name already exists.']);
        } else {
            throw $e;
        }
    }
}

/**
 * Machine의 design_process_idx를 일괄 업데이트합니다.
 *
 * - assignments: JSON 배열 형태로 전송 (form-data 내 JSON 문자열)
 *   형식: [ { machine_idx: 1, design_process_idx: 3 }, ... ]
 * - design_process_idx=0: 기계를 공정에서 해제(미배정 상태)
 * - 트랜잭션: 여러 기계를 원자적으로 업데이트 (일부 실패 시 전체 롤백)
 * - machine_idx > 0인 항목만 처리 (유효하지 않은 idx 건너뜀)
 *
 * @param PDO $pdo PDO 객체
 */
function updateMachineAssignments(PDO $pdo) {
    // JSON 형식으로 전송된 assignments 데이터를 파싱
    $assignments_json = $_POST['assignments'] ?? '';

    if (empty($assignments_json)) {
        http_response_code(400);
        echo json_encode(['success' => false, 'message' => 'Assignments data is required.']);
        return;
    }

    $assignments = json_decode($assignments_json, true);

    // JSON 파싱 실패 또는 배열이 아닌 경우 오류 반환
    if (!is_array($assignments)) {
        http_response_code(400);
        echo json_encode(['success' => false, 'message' => 'Invalid assignments data format.']);
        return;
    }

    try {
        $pdo->beginTransaction();

        // Prepared statement 재사용으로 성능 최적화 (N건 일괄 업데이트)
        $stmt = $pdo->prepare("
            UPDATE info_machine
            SET design_process_idx = ?, update_date = NOW()
            WHERE idx = ?
        ");

        foreach ($assignments as $assignment) {
            $machine_idx = (int)($assignment['machine_idx'] ?? 0);
            $design_process_idx = (int)($assignment['design_process_idx'] ?? 0);

            // machine_idx <= 0인 항목은 유효하지 않은 데이터로 건너뜀
            if ($machine_idx > 0) {
                $stmt->execute([$design_process_idx, $machine_idx]);
            }
        }

        $pdo->commit();

        echo json_encode([
            'success' => true,
            'message' => 'Machine assignments updated successfully.',
            'updated_count' => count($assignments)  // 처리 시도한 총 항목 수 반환
        ]);
    } catch (Exception $e) {
        $pdo->rollBack();
        http_response_code(500);
        echo json_encode([
            'success' => false,
            'message' => 'Failed to update assignments: ' . $e->getMessage()
        ]);
    }
}
