<?php
## 김도완 이사님 개발 프로젝트 전용 API.
## 2021.04.14 mes.suntech.asia 로 변경
## 2021.12.06 115.68.227.31 로 변경. --> 김도완 이사님 확인 완료

require_once($_SERVER['DOCUMENT_ROOT'] . '/api/st500/lib/database.php');
require_once($_SERVER['DOCUMENT_ROOT'] . '/api/st500/lib/define.php');

//$result = array('code'=>'00', 'msg'=>'ok');

$now_datetime = date('Y-m-d H:i:s', $_SERVER['REQUEST_TIME']);

$code = get_request('code');

$code_list = array(
    ## GET
    'get_device',
    // device state
    //'get_country',		// country info
    //'get_remain_date',	// remain date

    # SEND
    'send_device' // send device info
);

if (in_array($code, $code_list)) {

    include_once($_SERVER['DOCUMENT_ROOT'] . '/api/st500/V1.0/' . $code . ".php");

} else {

    $result = array('code' => '99', 'msg' => '일치하는 code 값 없음');
}

$result = json_encode($result, JSON_UNESCAPED_UNICODE | JSON_UNESCAPED_SLASHES);
$result = '[' . $result . ']';

header('Content-Length: ' . strlen($result));
header("Content-Type: text/plain");

echo $result;

if (true) { // 로그기록

    $_data = json_encode($_REQUEST);

    // 용우 버전
    /* $qry = $pdo->prepare
        ("INSERT INTO data_logs_api (code, log_data, log_result, reg_ip, reg_date)
         VALUES (:code, :log_data, :log_result, :reg_ip, :reg_date)"); */
    // 김이사님 버전     
    $qry = $pdo->prepare
    ("INSERT INTO st500_logs (code, log_data, log_result, reg_ip, reg_date)
    	VALUES (:code, :log_data, :log_result, :reg_ip, :reg_date)");
    $qry->bindValue(":code", $code);
    $qry->bindValue(":log_data", $_data);
    $qry->bindValue(":log_result", $result);
    $qry->bindValue(":reg_ip", USER_IP);
    $qry->bindValue(":reg_date", $now_datetime);
    $qry->execute();
}


function get_request($key, $default = '')
{

    return (isset($_REQUEST[$key])) ? trim($_REQUEST[$key]) : $default;

}