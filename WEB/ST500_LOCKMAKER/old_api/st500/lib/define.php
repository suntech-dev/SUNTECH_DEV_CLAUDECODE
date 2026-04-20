<?php

	if(!function_exists('_print_r')) {

		function _print_r($array=array()) {

			echo '<xmp>';
			print_r($array);
			echo '</xmp>';

		}

	}

	/*if(!function_exists('_json_print')){
		function _json_print($result=array())
		{
			echo json_encode($result);
			exit();
		}
	}*/


	date_default_timezone_set('Asia/Seoul');

	//$dateTime = new DateTime();
	//$dateTime->format('Y-m-d H:i:s');

	define('USER_IP',		$_SERVER['REMOTE_ADDR']);
	define('NOW_DATE',		date('Y-m-d'));
	//define('YESTERDAY',		date('Y-m-d', strtotime('-1 day'));
	define('NOW_TIME',		date('H:i:s'));
	define('NOW_DATE_TIME',	date('Y-m-d H:i:s'));
	//define('NOW_DATE_TIME',	date(NOW_DATE.' '.NOW_TIME));


	define('SERVER_NAME',	$_SERVER['HTTP_HOST']);
	define('SITE_URL',		'http://'.SERVER_NAME);
	define('SITE_URL_SSL',	'http://'.SERVER_NAME);
	define('SITE_PATH',		$_SERVER['DOCUMENT_ROOT']);
	define('THIS_URL',		$_SERVER['REQUEST_URI']);
	define('REFERER_URL',	(isset($_SERVER['HTTP_REFERER'])) ? $_SERVER['HTTP_REFERER'] : '');