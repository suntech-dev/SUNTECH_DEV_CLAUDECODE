<?php
// 현재 페이지가 어느 폴더에 있는지 자동 감지
$script_dir = dirname($_SERVER['SCRIPT_NAME']);
$current_page = basename($_SERVER['SCRIPT_NAME']);

// 프로젝트 루트 동적 계산
// 페이지 구조: {project_root}/page/{category}/file.php
// /page/{category}/ 패턴이면 2단계 위가 프로젝트 루트
if (preg_match('#/page/[^/]+$#', $script_dir)) {
  $project_root = dirname(dirname($script_dir));
  $nav_assets_path = '../../assets';
} else {
  $project_root = dirname($script_dir);
  $nav_assets_path = '../assets';
}

$base_path      = $project_root . '/page/manage';
$data_base_path = $project_root . '/page/data';
?>

<!-- SAP Fiori CSS 파일 추가 -->
<link rel="stylesheet" href="<?php echo $nav_assets_path; ?>/css/fiori-design-tokens.css">
<link rel="stylesheet" href="<?php echo $nav_assets_path; ?>/css/fiori-navigation.css">

<!-- SAP Fiori 네비게이션 -->
<nav class="fiori-app-bar">
  <div class="fiori-app-bar__container">
    <a href="#" class="fiori-app-bar__brand">
      OEE SYSTEM
    </a>
    <ul class="fiori-nav-menu">

      <!-- Setting 그룹 -->
      <li class="fiori-nav-menu__item fiori-nav-menu__item--dropdown <?php echo (in_array($current_page, ['info_factory_2.php', 'info_line_2.php', 'info_machine_model_2.php', 'info_machine_2.php', 'info_design_process_2.php', 'info_andon_2.php', 'info_downtime_2.php', 'info_defective_2.php', 'info_worktime_2.php', 'info_rate_color_2.php'])) ? 'fiori-nav-menu__item--active' : ''; ?>">
        <a href="#" class="fiori-nav-menu__link">
          Setting
        </a>
        <ul class="fiori-dropdown-menu">
          <li class="fiori-dropdown-menu__item <?php echo ($current_page == 'info_factory_2.php') ? 'fiori-dropdown-menu__item--active' : ''; ?>">
            <a href="<?php echo $base_path; ?>/info_factory_2.php" class="fiori-dropdown-menu__link">
              Factory
            </a>
          </li>
          <li class="fiori-dropdown-menu__item <?php echo ($current_page == 'info_line_2.php') ? 'fiori-dropdown-menu__item--active' : ''; ?>">
            <a href="<?php echo $base_path; ?>/info_line_2.php" class="fiori-dropdown-menu__link">
              Line
            </a>
          </li>
          <div class="fiori-dropdown-menu__divider"></div>
          <li class="fiori-dropdown-menu__item <?php echo ($current_page == 'info_machine_model_2.php') ? 'fiori-dropdown-menu__item--active' : ''; ?>">
            <a href="<?php echo $base_path; ?>/info_machine_model_2.php" class="fiori-dropdown-menu__link">
              Machine Model
            </a>
          </li>
          <li class="fiori-dropdown-menu__item <?php echo ($current_page == 'info_machine_2.php') ? 'fiori-dropdown-menu__item--active' : ''; ?>">
            <a href="<?php echo $base_path; ?>/info_machine_2.php" class="fiori-dropdown-menu__link">
              Machine
            </a>
          </li>
          <li class="fiori-dropdown-menu__item <?php echo ($current_page == 'info_design_process_2.php') ? 'fiori-dropdown-menu__item--active' : ''; ?>">
            <a href="<?php echo $base_path; ?>/info_design_process_2.php" class="fiori-dropdown-menu__link">
              Design Process
            </a>
          </li>
          <div class="fiori-dropdown-menu__divider"></div>
          <li class="fiori-dropdown-menu__item <?php echo ($current_page == 'info_andon_2.php') ? 'fiori-dropdown-menu__item--active' : ''; ?>">
            <a href="<?php echo $base_path; ?>/info_andon_2.php" class="fiori-dropdown-menu__link">
              Andon
            </a>
          </li>
          <li class="fiori-dropdown-menu__item <?php echo ($current_page == 'info_downtime_2.php') ? 'fiori-dropdown-menu__item--active' : ''; ?>">
            <a href="<?php echo $base_path; ?>/info_downtime_2.php" class="fiori-dropdown-menu__link">
              Downtime
            </a>
          </li>
          <li class="fiori-dropdown-menu__item <?php echo ($current_page == 'info_defective_2.php') ? 'fiori-dropdown-menu__item--active' : ''; ?>">
            <a href="<?php echo $base_path; ?>/info_defective_2.php" class="fiori-dropdown-menu__link">
              Defective
            </a>
          </li>
          <div class="fiori-dropdown-menu__divider"></div>
          <li class="fiori-dropdown-menu__item <?php echo ($current_page == 'info_rate_color_2.php') ? 'fiori-dropdown-menu__item--active' : ''; ?>">
            <a href="<?php echo $base_path; ?>/info_rate_color_2.php" class="fiori-dropdown-menu__link">
              Rate Color
            </a>
          </li>
          <li class="fiori-dropdown-menu__item <?php echo ($current_page == 'info_worktime_2.php') ? 'fiori-dropdown-menu__item--active' : ''; ?>">
            <a href="<?php echo $base_path; ?>/info_worktime_2.php" class="fiori-dropdown-menu__link">
              Work Time
            </a>
          </li>
        </ul>
      </li>

      <!-- Monitoring 그룹 -->
      <li class="fiori-nav-menu__item fiori-nav-menu__item--dropdown <?php echo (in_array($current_page, ['data_oee_2.php', 'data_andon_2.php', 'data_downtime_2.php', 'data_defective_2.php'])) ? 'fiori-nav-menu__item--active' : ''; ?>">
        <a href="#" class="fiori-nav-menu__link">
          Monitoring
        </a>
        <ul class="fiori-dropdown-menu">
          <li class="fiori-dropdown-menu__item <?php echo ($current_page == 'data_oee_2.php') ? 'fiori-dropdown-menu__item--active' : ''; ?>">
            <a href="<?php echo $data_base_path; ?>/data_oee_2.php" class="fiori-dropdown-menu__link">
              OEE Monitoring
            </a>
          </li>
          <li class="fiori-dropdown-menu__item <?php echo ($current_page == 'data_andon_2.php') ? 'fiori-dropdown-menu__item--active' : ''; ?>">
            <a href="<?php echo $data_base_path; ?>/data_andon_2.php" class="fiori-dropdown-menu__link">
              Andon Monitoring
            </a>
          </li>
          <li class="fiori-dropdown-menu__item <?php echo ($current_page == 'data_downtime_2.php') ? 'fiori-dropdown-menu__item--active' : ''; ?>">
            <a href="<?php echo $data_base_path; ?>/data_downtime_2.php" class="fiori-dropdown-menu__link">
              Downtime Monitoring
            </a>
          </li>
          <li class="fiori-dropdown-menu__item <?php echo ($current_page == 'data_defective_2.php') ? 'fiori-dropdown-menu__item--active' : ''; ?>">
            <a href="<?php echo $data_base_path; ?>/data_defective_2.php" class="fiori-dropdown-menu__link">
              Defective Monitoring
            </a>
          </li>
        </ul>
      </li>

      <!-- OEE Report 그룹 -->
      <li class="fiori-nav-menu__item fiori-nav-menu__item--dropdown <?php echo (in_array($current_page, ['log_oee_2.php', 'log_oee_hourly_2.php', 'log_oee_row_2.php'])) ? 'fiori-nav-menu__item--active' : ''; ?>">
        <a href="#" class="fiori-nav-menu__link">
          Report
        </a>
        <ul class="fiori-dropdown-menu">
          <li class="fiori-dropdown-menu__item <?php echo ($current_page == 'log_oee_2.php') ? 'fiori-dropdown-menu__item--active' : ''; ?>">
            <a href="<?php echo $data_base_path; ?>/log_oee_2.php" class="fiori-dropdown-menu__link">
              OEE Report by Shift
            </a>
          </li>
          <li class="fiori-dropdown-menu__item <?php echo ($current_page == 'log_oee_hourly_2.php') ? 'fiori-dropdown-menu__item--active' : ''; ?>">
            <a href="<?php echo $data_base_path; ?>/log_oee_hourly_2.php" class="fiori-dropdown-menu__link">
              OEE Report by Hourly
            </a>
          </li>
          <li class="fiori-dropdown-menu__item <?php echo ($current_page == 'log_oee_row_2.php') ? 'fiori-dropdown-menu__item--active' : ''; ?>">
            <a href="<?php echo $data_base_path; ?>/log_oee_row_2.php" class="fiori-dropdown-menu__link">
              OEE Report by Row data
            </a>
          </li>
        </ul>
      </li>

      <!-- Dashboard (단독) -->
      <li class="fiori-nav-menu__item <?php echo ($current_page == 'dashboard_2.php') ? 'fiori-nav-menu__item--active' : ''; ?>">
        <a href="<?php echo $data_base_path; ?>/dashboard_2.php" class="fiori-nav-menu__link">
          Dashboard
        </a>
      </li>

      <!-- AI Dashboard (단독) -->
      <li class="fiori-nav-menu__item <?php echo ($current_page == 'ai_dashboard_5.php') ? 'fiori-nav-menu__item--active' : ''; ?>">
        <a href="<?php echo $data_base_path; ?>/ai_dashboard_5.php" class="fiori-nav-menu__link">
          AI Dashboard
        </a>
      </li>

    </ul>
  </div>
</nav>
