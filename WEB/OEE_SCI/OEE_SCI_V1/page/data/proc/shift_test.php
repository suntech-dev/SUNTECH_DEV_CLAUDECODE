<?php
// 간단한 shift 데이터 확인 스크립트
require_once(__DIR__ . '/../../../lib/db.php');

header('Content-Type: application/json; charset=utf-8');

try {
    // 1. 전체 데이터 조회
    $stmt = $pdo->prepare("
        SELECT idx, work_date, shift_idx, machine_no, andon_name, status, reg_date 
        FROM data_andon 
        ORDER BY reg_date DESC 
        LIMIT 10
    ");
    $stmt->execute();
    $allData = $stmt->fetchAll(PDO::FETCH_ASSOC);
    
    // 2. shift_idx별 데이터 개수
    $stmt = $pdo->prepare("
        SELECT shift_idx, COUNT(*) as count 
        FROM data_andon 
        GROUP BY shift_idx 
        ORDER BY shift_idx
    ");
    $stmt->execute();
    $shiftStats = $stmt->fetchAll(PDO::FETCH_ASSOC);
    
    // 3. shift_idx = 1인 데이터
    $stmt = $pdo->prepare("
        SELECT idx, work_date, shift_idx, machine_no, andon_name, status, reg_date 
        FROM data_andon 
        WHERE shift_idx = 1
        ORDER BY reg_date DESC 
        LIMIT 5
    ");
    $stmt->execute();
    $shift1Data = $stmt->fetchAll(PDO::FETCH_ASSOC);
    
    echo json_encode([
        'success' => true,
        'all_data' => $allData,
        'shift_stats' => $shiftStats,
        'shift_1_data' => $shift1Data
    ], JSON_UNESCAPED_UNICODE | JSON_PRETTY_PRINT);
    
} catch (Exception $e) {
    echo json_encode([
        'success' => false,
        'error' => $e->getMessage()
    ], JSON_UNESCAPED_UNICODE);
}
?>