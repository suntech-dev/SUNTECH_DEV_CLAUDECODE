<?php
/**
 * proc/ajax_factory_line.php — 공장 선택 시 라인 드롭다운 옵션 동적 반환 (AJAX용)
 *
 * 파라미터:
 *   factoryData : 선택된 공장 idx (없으면 0)
 *   init        : 드롭다운 첫 번째 항목 유형
 *                 - 'AllLine' : "Select Line" + "All Line" 두 항목 추가
 *                 - 'Line'    : "Line" 단일 항목 추가 (기본값)
 *                 - 'NoItem'  : 첫 항목 없음
 *                 - 그 외    : "All Lines" 추가
 *   kind        : factoryData가 없을 때 처리 방식
 *                 - 'ALL' : 전체 라인 반환 (공장명 > 라인명 형식)
 *
 * 응답: HTML <option> 태그 문자열 (echo 직접 출력)
 * — XSS 방지를 위해 htmlspecialchars(ENT_QUOTES) 적용
 */

require_once(__DIR__ . '/../../../lib/db.php');

// POST 파라미터 수신 및 기본값 설정
$factoryData = isset($_POST['factoryData']) ? (int)$_POST['factoryData'] : 0;
$init        = isset($_POST['init']) ? trim($_POST['init']) : 'Line';
$kind        = isset($_POST['kind']) ? trim($_POST['kind']) : '';

// init 값에 따라 드롭다운 첫 번째 항목(들) 출력
if ($init === 'AllLine') {
    // 'All Line' 선택 가능 형태 (라인 전체 조회 옵션 포함)
    echo '<option value="-1">Select Line</option>';
    echo '<option value="0">All Line</option>';
} elseif ($init === 'Line') {
    // 기본 "Line" 플레이스홀더 (선택 안내용)
    echo '<option value="">Line</option>';
} elseif ($init !== 'NoItem') {
    // 그 외 init 값 → "All Lines" 항목 추가
    echo '<option value="">All Lines</option>';
}
// init === 'NoItem'이면 첫 항목 없이 바로 데이터 옵션 출력

if ($factoryData) {
    // 특정 공장이 선택된 경우: 해당 공장의 활성 라인만 조회
    $qry = $pdo->prepare(
        "SELECT idx, line_name FROM `info_line`
         WHERE factory_idx = ? AND status = 'Y'
         ORDER BY line_name ASC"
    );
    $qry->execute([$factoryData]);
    $line_data = $qry->fetchAll(PDO::FETCH_ASSOC);

    // XSS 방지: idx와 line_name 모두 htmlspecialchars 처리
    foreach ($line_data as $row) {
        echo '<option value="' . htmlspecialchars($row['idx'], ENT_QUOTES) . '">'
            . htmlspecialchars($row['line_name'], ENT_QUOTES)
            . '</option>';
    }
} elseif ($kind === 'ALL') {
    // 공장 선택 없이 kind='ALL'인 경우: 전체 활성 라인을 "공장명 > 라인명" 형식으로 반환
    $stmt = $pdo->prepare(
        "SELECT a.idx, a.line_name, b.factory_name
         FROM `info_line` AS a
         INNER JOIN `info_factory` AS b ON b.idx = a.factory_idx
         WHERE a.status = 'Y'
         ORDER BY a.line_name ASC, b.factory_name ASC"
    );
    $stmt->execute();
    $line_data = $stmt->fetchAll(PDO::FETCH_ASSOC);

    // "공장명 > 라인명" 형식으로 표시하여 어느 공장 소속인지 구분 가능하게 함
    foreach ($line_data as $row) {
        echo '<option value="' . htmlspecialchars($row['idx'], ENT_QUOTES) . '">'
            . htmlspecialchars($row['factory_name'], ENT_QUOTES) . ' > '
            . htmlspecialchars($row['line_name'], ENT_QUOTES)
            . '</option>';
    }
}
