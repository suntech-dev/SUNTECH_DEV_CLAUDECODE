<?php
## 김도완 이사님께서 21년 3월 29일 요청한 내용으로 수정
## get_device 전송 시 log 는 항상 남아야 하고, approve 또는 not approve 만 응답결과 나오게 한다.
## 2021.04.14 mes.suntech.asia 로 변경

$device_id = get_request('device_id');
//$name	    = get_request('name');

//print_r($device_id);

if ($device_id) {

	$qry = $pdo->prepare("SELECT *
							FROM data_smart_device
							WHERE device_id = :device_id");
	$qry->bindValue(":device_id", $device_id);
	$qry->execute();

	//$count = $qry -> fetchColumn();
	$data = $qry->fetch(PDO::FETCH_ASSOC);

	// if ($data['status'] == 'Y') {
	if ($data['state'] == 'Y') {

		$result['code'] = '00';
		$result['msg'] = 'approve';

	} else {

		$result['code'] = '99';
		$result['msg'] = 'not approved';

	}

} else {

	$result['code'] = '99';
	$result['msg'] = '등록한 디바이스가 없습니다.';

}