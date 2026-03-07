<?php
## New CPU 패턴재봉기 & Andon 디바이스
## factoryData : factory id value
## init = 옵션 리스트의 첫번째 항목의 이름 ex) None, Line, AllLine (None이면 사용안함)
## kind = factoryData 값이 없을 경우 처리할 방법. 'ALL' 이면 전체 Line 리턴

require_once(__DIR__ . '/../../../lib/db.php');

$factoryData = isset($_POST['factoryData']) ? trim($_POST['factoryData']) : '';
$init        = isset($_POST['init']) ? trim($_POST['init']) : 'Line';            // 그룹 OPTION 의 첫번째 항목 이름
$kind        = isset($_POST['kind']) ? trim($_POST['kind']) : '';

if ($init=='AllLine') {
	echo '<option value="-1">Select Line</option>'; 
	echo '<option value="0">All Line</option>'; 
}
 
if ($init=='Line') {
	echo '<option value="">Line</option>'; 
} else if ($init == 'NoItem') {
	// pass
} else {
	echo '<option value="">All Lines</option>';
}
if ($factoryData) {

	$qry = $pdo->prepare(
		"SELECT *
		FROM `info_line`
		WHERE factory_idx = ".$_POST['factoryData']." AND status = 'Y'
		ORDER BY line_name ASC"
	);
	$qry->execute();
	$line_data = $qry->fetchAll(PDO::FETCH_ASSOC);

	foreach ($line_data as $row) {  
		echo '<option value="'.$row['idx'].'">'.$row['line_name'].'</option>'; 
	}

} else {

	if ($kind=='ALL') {
		## line list
		$stmt = $pdo->prepare(
			"SELECT a.idx, a.line_name, b.factory_name 
			FROM `info_line` AS a 
				INNER JOIN `info_factory` AS b ON b.idx = a.factory_idx  
			WHERE a.status = 'Y' ORDER BY a.line_name ASC, b.factory_name ASC"
		);
		$stmt->execute();
		$line_data = $stmt->fetchAll(PDO::FETCH_ASSOC);

		foreach ($line_data as $row) {  
		echo '<option value="'.$row['idx'].'">'.$row['factory_name'].' > '.$row['line_name'].'</option>'; 
		}
	}
}