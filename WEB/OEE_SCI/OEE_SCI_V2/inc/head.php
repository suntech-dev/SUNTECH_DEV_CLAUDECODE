<?php
/**
 * 공통 HTML Head 템플릿
 * 모든 관리 페이지에서 사용하는 공통 헤더 구성요소
 * 
 * 사용 변수:
 * - $page_title: 페이지 제목 (기본값: 'Management')
 * - $page_css_files: 추가 CSS 파일 배열
 * - $page_styles: 페이지별 인라인 CSS 스타일
 */

$page_title = $page_title ?? 'Management';
$page_css_files = $page_css_files ?? [];
$page_styles = $page_styles ?? '';

// 현재 스크립트의 디렉토리 경로를 기반으로 assets 경로 설정
$script_dir = dirname($_SERVER['SCRIPT_NAME']);
if (strpos($script_dir, '/manage') !== false) {
    $assets_path = '../../assets';
    $base_path = '../..';
} elseif (strpos($script_dir, '/data') !== false) {
    $assets_path = '../../assets';
    $base_path = '../..';
} else {
    $assets_path = '../assets';
    $base_path = '..';
}
?>
<!DOCTYPE html>
<html lang="en">
<head>
  <meta charset="UTF-8">
  <meta name="viewport" content="width=device-width, initial-scale=1.0, user-scalable=no">
  <!-- <title><?php echo $page_title; ?> - SAP Fiori</title> -->
  <title><?php echo $page_title; ?> - SunTech</title>
  
  <meta name="theme-color" content="#0070f2">

  <!-- favicons -->
  <link rel="icon" type="image/png" sizes="32x32" href="<?php echo $assets_path; ?>/images/favicons/suntech_blue.png">
  <link rel="icon" type="image/png" sizes="16x16" href="<?php echo $assets_path; ?>/images/favicons/suntech_blue.png">
  <link href="<?php echo $assets_path; ?>/images/favicons/suntech_blue.png" rel="shortcut icon">
  <!-- <link href="../favicon.ico" rel="icon" type="image/x-icon"> -->
  
  <!-- 기본 CSS 스타일시트 -->
  <link rel="stylesheet" href="<?php echo $assets_path; ?>/css/fiori-style.css">
  <link rel="stylesheet" href="<?php echo $assets_path; ?>/css/fiori-mobile.css">
  
  <!-- 페이지별 CSS 파일 -->
  <?php if (!empty($page_css_files)): ?>
    <?php foreach ($page_css_files as $css_file): ?>
      <link rel="stylesheet" href="<?php echo $css_file; ?>">
    <?php endforeach; ?>
  <?php endif; ?>
  
  <!-- JavaScript 라이브러리 -->
  <script src="<?php echo $assets_path; ?>/js/fiori-advanced-interactions.js"></script>
  <script src="<?php echo $assets_path; ?>/js/mobile-interactions.js"></script>
  
  <!-- 페이지별 인라인 스타일 -->
  <?php if (!empty($page_styles)): ?>
    <style>
      <?php echo $page_styles; ?>
    </style>
  <?php endif; ?>
</head>
<body>