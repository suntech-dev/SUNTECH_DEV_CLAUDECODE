<?php
/**
 * 공통 HTML Footer 템플릿
 * 모든 페이지에서 사용하는 공통 푸터 구성요소
 * 
 * 사용 변수:
 * - $page_js_files: 추가 JavaScript 파일 배열
 * - $page_scripts: 페이지별 인라인 JavaScript 코드
 */

$page_js_files = $page_js_files ?? [];
$page_scripts = $page_scripts ?? '';
?>

<!-- 페이지별 JavaScript 파일 -->
<?php if (!empty($page_js_files)): ?>
  <?php foreach ($page_js_files as $js_file): ?>
    <script src="<?php echo $js_file; ?>"></script>
  <?php endforeach; ?>
<?php endif; ?>

<!-- 페이지별 인라인 스크립트 -->
<?php if (!empty($page_scripts)): ?>
  <script>
    <?php echo $page_scripts; ?>
  </script>
<?php endif; ?>

</body>
</html>