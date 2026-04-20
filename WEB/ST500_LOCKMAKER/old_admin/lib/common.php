<?php
function jsonSuccess($msg='success') {
  $msg = array('msg' => $msg);
  echo json_encode($msg, JSON_UNESCAPED_UNICODE);
  exit();
}

function jsonFail($msg='fail') {
  $msg = array('error' => $msg);
  echo json_encode($msg, JSON_UNESCAPED_UNICODE);
  exit();
}
