<?php
## ST-500 Renewal (2022.02.23 Start. dev@suntech.asia & hamani@naver.com)

require_once($_SERVER['DOCUMENT_ROOT'] . '/st500/lib/database.php');
require_once($_SERVER['DOCUMENT_ROOT'] . '/st500/lib/common.php');

$now_date = date('Y-m-d', $_SERVER['REQUEST_TIME']);    // korea date type
$now_time = date('H:i:s', $_SERVER['REQUEST_TIME']);
$date_time = $now_date." ".$now_time;

if (isset($_FILES) && isset($_FILES['parameterFile'])) {
	if ($_FILES['parameterFile']['error']==0 && is_uploaded_file($_FILES['parameterFile']['tmp_name'])) {
		// $file_name = $_SERVER['DOCUMENT_ROOT'] . '/st500/upload/parameter/' . $_FILES['parameterFile']['name'];
		$file_name = $_SERVER['DOCUMENT_ROOT'] . '/st500/parameter/' . $_FILES['parameterFile']['name'];
		if (file_exists($file_name)) @unlink($file_name);
		if (!move_uploaded_file($_FILES['parameterFile']['tmp_name'], $file_name)) {
			jsonFail('Upload fail');
		}

        ## 기존 table data 지우기.
        $stmt = $pdo->prepare("TRUNCATE `st500_parameter`");
        $stmt->execute();

        $contents = file_get_contents($file_name);
        $json = json_decode($contents, true);
        /* foreach($json as $k) {
            $stmt = $pdo->prepare("INSERT INTO `st500_parameter` (num, text, update_date) VALUES (?, ?, ?)");
            $result = $stmt->execute([$k['num'], $k['text'], $date_time]);
            if (!$result) jsonFail('Failed to insert data');
        } */
        foreach($json as $k) {
            if($k['text'] != 'NONE' && $k['num'] > '0000' && $k['num'] < '1000') {
                $k['text'] = str_replace("\n", "<br>", $k['text']);
                $stmt = $pdo->prepare("INSERT INTO `st500_parameter` (num, text, update_date) VALUES (?, ?, ?)");
                $result = $stmt->execute([$k['num'], $k['text'], $date_time]);
                if (!$result) jsonFail('Failed to insert data');
            }
        }
        jsonSuccess('Success');
	}
}
jsonFail('Upload fail');