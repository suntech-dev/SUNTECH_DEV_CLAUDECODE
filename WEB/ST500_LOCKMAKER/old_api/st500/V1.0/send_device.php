<?php
## 김도완 이사님께서 21년 3월 29일 요청한 내용으로 수정
## send_device 전송 시 log 는 항상 남아야 하고, success 또는 failed 만 응답결과 나오게 한다.
## name 중복 시 업데이트 처리한다.
## 2021.04.14 mes.suntech.asia 로 변경

$now_datetime = date('Y-m-d H:i:s', $_SERVER['REQUEST_TIME']);
$remain_date = date('Y-m-d H:i:s', strtotime('+3 months'));

$device_id = get_request('device_id');
$name = get_request('name');
//$brand    	= get_request('brand');
//$model	  	= get_request('model');
//$version    	= get_request('version');

//$result['item1'] = array();
//$result['item1'] = array();

//$reg_date = NOW_DATE_TIME;
//$update_date = NOW_DATE_TIME;

## device_id 주소로 이미 등록된 장비가 있는지? 조회
if ($device_id) {

	$qry = $pdo->prepare
	("SELECT COUNT(0) AS cnt
			FROM data_smart_device
			WHERE device_id = :device_id");
	$qry->bindValue(":device_id", $device_id);
	$qry->execute();
	$device_count = $qry->fetchColumn();


	## 이미 등록된 장비가 있으면
	if ($device_count > 0) {

		$qry = $pdo->prepare
		("UPDATE data_smart_device
				SET name = :name, update_date = :update_date
				WHERE device_id = :device_id");
		$qry->bindValue(":device_id", $device_id);
		$qry->bindValue(":update_date", $now_datetime);
		$qry->bindValue(":name", $name);
		$qry->execute();

		$result['code'] = '00';
		$result['msg'] = 'success';

	} else {

		## device insert
		$qry = $pdo->prepare
		("INSERT INTO data_smart_device (device_id, name, reg_date, remain_date)
				VALUES (:device_id, :name, :reg_date, :remain_date)");
		$qry->bindValue(":device_id", $device_id);
		$qry->bindValue(":reg_date", $now_datetime);
		$qry->bindValue(":remain_date", $remain_date);
		$qry->bindValue(":name", $name);
		$qry->execute();

		$count = $qry->rowCount();

		if ($count > 0) {

			$result['code'] = '00';
			$result['msg'] = 'success';

		} else {

			$result['code'] = '99';
			$result['msg'] = 'failed';

		}

	}

} else {

	$result['code'] = '99';
	$result['msg'] = 'failed';

}