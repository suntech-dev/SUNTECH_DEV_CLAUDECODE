<?php
// 데이터베이스 테이블 구조 확인
require_once(__DIR__ . '/../../../lib/db.php');

try {
    // 테이블 구조 확인
    $stmt = $pdo->prepare("DESCRIBE data_andon");
    $stmt->execute();
    $columns = $stmt->fetchAll(PDO::FETCH_ASSOC);
    
    echo "data_andon 테이블 컬럼:\n";
    foreach ($columns as $column) {
        echo "- " . $column['Field'] . " (" . $column['Type'] . ")\n";
    }
    
    // shift_idx 컬럼 존재 여부 확인
    $hasShiftIdx = false;
    foreach ($columns as $column) {
        if ($column['Field'] === 'shift_idx') {
            $hasShiftIdx = true;
            break;
        }
    }
    
    echo "\nshift_idx 컬럼 존재: " . ($hasShiftIdx ? "YES" : "NO") . "\n";
    
    // 실제 데이터 샘플 조회
    $stmt = $pdo->prepare("SELECT * FROM data_andon LIMIT 3");
    $stmt->execute();
    $samples = $stmt->fetchAll(PDO::FETCH_ASSOC);
    
    echo "\n샘플 데이터:\n";
    foreach ($samples as $i => $row) {
        echo "Row " . ($i + 1) . ":\n";
        foreach ($row as $key => $value) {
            echo "  $key: $value\n";
        }
        echo "\n";
    }
    
} catch (Exception $e) {
    echo "오류: " . $e->getMessage();
}
?>