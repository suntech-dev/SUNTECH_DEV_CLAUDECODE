<?php
ini_set('display_errors', 1);
error_reporting(E_ALL);

/**
 * Process-Machine Mapping API
 * Handles process-machine assignment operations
 */

// 공통 라이브러리 및 설정 파일 로드
require_once(__DIR__ . '/../../../lib/config.php');
require_once(__DIR__ . '/../../../lib/db.php');

// API 응답을 JSON 형식으로 설정
header('Content-Type: application/json');

// HTTP 요청 메서드 확인
$method = $_SERVER['REQUEST_METHOD'];

// POST 요청에서 _method 필드가 있는 경우 해당 값으로 메서드를 재정의
if ($method === 'POST' && isset($_POST['_method'])) {
    $method = strtoupper($_POST['_method']);
}

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
                        updateProcessInfo($pdo);
                        break;
                    case 'update_machine_assignments':
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
 * @param PDO $pdo PDO 객체
 */
function getProcessMachineMapping(PDO $pdo) {
    try {
        // 1. 모든 design_process 조회 (status = 'Y'인 것만, Factory/Line 정보 포함)
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

        // 2. 모든 machine 조회 (status = 'Y'인 것만) with factory and line names
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
        $grouped_machines = [];
        foreach ($machines as $machine) {
            $process_idx = $machine['design_process_idx'] ?? 0;
            if (!isset($grouped_machines[$process_idx])) {
                $grouped_machines[$process_idx] = [];
            }
            $grouped_machines[$process_idx][] = $machine;
        }

        // 응답 데이터 구성
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
 * @param PDO $pdo PDO 객체
 */
function updateProcessInfo(PDO $pdo) {
    $idx = (int)($_POST['idx'] ?? 0);
    $design_process = trim($_POST['design_process'] ?? '');
    $std_mc_needed = (int)($_POST['std_mc_needed'] ?? 1);

    if (empty($idx) || empty($design_process)) {
        http_response_code(400);
        echo json_encode(['success' => false, 'message' => 'Process ID and name are required.']);
        return;
    }

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

    if (!is_array($assignments)) {
        http_response_code(400);
        echo json_encode(['success' => false, 'message' => 'Invalid assignments data format.']);
        return;
    }

    try {
        $pdo->beginTransaction();

        // Prepared statement로 일괄 업데이트
        $stmt = $pdo->prepare("
            UPDATE info_machine
            SET design_process_idx = ?, update_date = NOW()
            WHERE idx = ?
        ");

        foreach ($assignments as $assignment) {
            $machine_idx = (int)($assignment['machine_idx'] ?? 0);
            $design_process_idx = (int)($assignment['design_process_idx'] ?? 0);

            if ($machine_idx > 0) {
                $stmt->execute([$design_process_idx, $machine_idx]);
            }
        }

        $pdo->commit();

        echo json_encode([
            'success' => true,
            'message' => 'Machine assignments updated successfully.',
            'updated_count' => count($assignments)
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
