<?php
// =================================================================================================================
// ## API Test Page Helper ##
// ## `api/api_test` 디렉토리의 HTML 파일 목록을 스캔하고 그룹화하여 JSON으로 반환하는 API
// =================================================================================================================

header('Content-Type: application/json');

// API 테스트 파일이 있는 디렉토리 경로
$test_dir = __DIR__ . '/api_test/';

// 디렉토리가 존재하지 않으면 오류 반환
if (!is_dir($test_dir)) {
    echo json_encode(['error' => 'api_test directory not found.']);
    exit;
}

// 디렉토리의 모든 파일을 스캔
$files = scandir($test_dir);

$api_groups = [
    'Andon' => [],
    'Defective' => [],
    'Downtime' => [],
    'General' => []
];

foreach ($files as $file) {
    // HTML 파일만 대상으로 함
    if (pathinfo($file, PATHINFO_EXTENSION) == 'html') {
        $filename = basename($file, '.html');
        $path = './api_test/' . $file;

        // 파일 이름에 포함된 키워드를 기반으로 그룹화
        if (strpos($filename, 'andon') !== false) {
            $api_groups['Andon'][] = ['name' => $filename, 'path' => $path];
        } elseif (strpos($filename, 'defective') !== false) {
            $api_groups['Defective'][] = ['name' => $filename, 'path' => $path];
        } elseif (strpos($filename, 'downtime') !== false) {
            $api_groups['Downtime'][] = ['name' => $filename, 'path' => $path];
        } else {
            $api_groups['General'][] = ['name' => $filename, 'path' => $path];
        }
    }
}

// 그룹별로 정렬 (파일명 텍스트 길이가 짧은 순서대로)
foreach ($api_groups as $key => &$group) {
    usort($group, function($a, $b) {
        return strlen($a['name']) - strlen($b['name']);
    });
}

// 최종 결과 반환
echo json_encode($api_groups, JSON_PRETTY_PRINT | JSON_UNESCAPED_SLASHES);
?>