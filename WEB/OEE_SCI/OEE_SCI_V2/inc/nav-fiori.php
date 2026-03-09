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
      <li class="fiori-nav-menu__item fiori-nav-menu__item--dropdown <?php echo (in_array($current_page, ['info_factory.php', 'info_line.php', 'info_machine_model.php', 'info_machine.php', 'info_design_process.php', 'info_andon.php', 'info_downtime.php', 'info_defective.php', 'info_worktime.php', 'info_rate_color.php'])) ? 'fiori-nav-menu__item--active' : ''; ?>">
        <a href="#" class="fiori-nav-menu__link">
          Setting
        </a>
        <ul class="fiori-dropdown-menu">
          <!-- <div class="fiori-dropdown-menu__group-header">기본 설정</div> -->
          <li class="fiori-dropdown-menu__item <?php echo ($current_page == 'info_factory.php') ? 'fiori-dropdown-menu__item--active' : ''; ?>">
            <a href="<?php echo $base_path; ?>/info_factory.php" class="fiori-dropdown-menu__link">
              Factory
            </a>
          </li>
          <li class="fiori-dropdown-menu__item <?php echo ($current_page == 'info_line.php') ? 'fiori-dropdown-menu__item--active' : ''; ?>">
            <a href="<?php echo $base_path; ?>/info_line.php" class="fiori-dropdown-menu__link">
              Line
            </a>
          </li>
          <div class="fiori-dropdown-menu__divider"></div>
          <!-- <div class="fiori-dropdown-menu__group-header">장비 관리</div> -->
          <li class="fiori-dropdown-menu__item <?php echo ($current_page == 'info_machine_model.php') ? 'fiori-dropdown-menu__item--active' : ''; ?>">
            <a href="<?php echo $base_path; ?>/info_machine_model.php" class="fiori-dropdown-menu__link">
              Machine Model
            </a>
          </li>
          <li class="fiori-dropdown-menu__item <?php echo ($current_page == 'info_machine.php') ? 'fiori-dropdown-menu__item--active' : ''; ?>">
            <a href="<?php echo $base_path; ?>/info_machine.php" class="fiori-dropdown-menu__link">
              Machine
            </a>
          </li>
          <!-- <div class="fiori-dropdown-menu__divider"></div> -->
          <li class="fiori-dropdown-menu__item <?php echo ($current_page == 'info_design_process.php') ? 'fiori-dropdown-menu__item--active' : ''; ?>">
            <a href="<?php echo $base_path; ?>/info_design_process.php" class="fiori-dropdown-menu__link">
              Design Process
            </a>
          </li>
          <!-- <li class="fiori-dropdown-menu__item <?php echo ($current_page == 'info_design.php') ? 'fiori-dropdown-menu__item--active' : ''; ?>">
            <a href="<?php echo $base_path; ?>/info_design.php" class="fiori-dropdown-menu__link">
              Design
            </a>
          </li> -->
          <div class="fiori-dropdown-menu__divider"></div>
          <li class="fiori-dropdown-menu__item <?php echo ($current_page == 'info_andon.php') ? 'fiori-dropdown-menu__item--active' : ''; ?>">
            <a href="<?php echo $base_path; ?>/info_andon.php" class="fiori-dropdown-menu__link">
              Andon
            </a>
          </li>
          <li class="fiori-dropdown-menu__item <?php echo ($current_page == 'info_downtime.php') ? 'fiori-dropdown-menu__item--active' : ''; ?>">
            <a href="<?php echo $base_path; ?>/info_downtime.php" class="fiori-dropdown-menu__link">
              Downtime
            </a>
          </li>
          <li class="fiori-dropdown-menu__item <?php echo ($current_page == 'info_defective.php') ? 'fiori-dropdown-menu__item--active' : ''; ?>">
            <a href="<?php echo $base_path; ?>/info_defective.php" class="fiori-dropdown-menu__link">
              Defective
            </a>
          </li>
          <div class="fiori-dropdown-menu__divider"></div>
          <li class="fiori-dropdown-menu__item <?php echo ($current_page == 'info_rate_color.php') ? 'fiori-dropdown-menu__item--active' : ''; ?>">
            <a href="<?php echo $base_path; ?>/info_rate_color.php" class="fiori-dropdown-menu__link">
              Rate Color
            </a>
          </li>
          <li class="fiori-dropdown-menu__item <?php echo ($current_page == 'info_worktime.php') ? 'fiori-dropdown-menu__item--active' : ''; ?>">
            <a href="<?php echo $base_path; ?>/info_worktime.php" class="fiori-dropdown-menu__link">
              Work Time
            </a>
          </li>
        </ul>
      </li>

      <!-- Monitoring 그룹 -->
      <li class="fiori-nav-menu__item fiori-nav-menu__item--dropdown <?php echo (in_array($current_page, ['data_oee.php', 'data_andon.php', 'data_downtime.php', 'data_defective.php'])) ? 'fiori-nav-menu__item--active' : ''; ?>">
        <a href="#" class="fiori-nav-menu__link">
          Monitoring
        </a>
        <ul class="fiori-dropdown-menu">
          <li class="fiori-dropdown-menu__item <?php echo ($current_page == 'data_oee.php') ? 'fiori-dropdown-menu__item--active' : ''; ?>">
            <a href="<?php echo $data_base_path; ?>/data_oee.php" class="fiori-dropdown-menu__link">
              OEE Monitoring
            </a>
          </li>
          <li class="fiori-dropdown-menu__item <?php echo ($current_page == 'data_andon.php') ? 'fiori-dropdown-menu__item--active' : ''; ?>">
            <a href="<?php echo $data_base_path; ?>/data_andon.php" class="fiori-dropdown-menu__link">
              Andon Monitoring
            </a>
          </li>
          <li class="fiori-dropdown-menu__item <?php echo ($current_page == 'data_downtime.php') ? 'fiori-dropdown-menu__item--active' : ''; ?>">
            <a href="<?php echo $data_base_path; ?>/data_downtime.php" class="fiori-dropdown-menu__link">
              Downtime Monitoring
            </a>
          </li>
          <li class="fiori-dropdown-menu__item <?php echo ($current_page == 'data_defective.php') ? 'fiori-dropdown-menu__item--active' : ''; ?>">
            <a href="<?php echo $data_base_path; ?>/data_defective.php" class="fiori-dropdown-menu__link">
              Defective Monitoring
            </a>
          </li>
        </ul>
      </li>

      <!-- OEE Report 그룹 -->
      <li class="fiori-nav-menu__item fiori-nav-menu__item--dropdown <?php echo (in_array($current_page, ['log_oee.php', 'log_oee_hourly.php', 'log_oee_data.php'])) ? 'fiori-nav-menu__item--active' : ''; ?>">
        <a href="#" class="fiori-nav-menu__link">
          Report
        </a>
        <ul class="fiori-dropdown-menu">
          <li class="fiori-dropdown-menu__item <?php echo ($current_page == 'log_oee.php') ? 'fiori-dropdown-menu__item--active' : ''; ?>">
            <a href="<?php echo $data_base_path; ?>/log_oee.php" class="fiori-dropdown-menu__link">
              OEE Report by Shift
            </a>
          </li>
          <li class="fiori-dropdown-menu__item <?php echo ($current_page == 'log_oee_hourly.php') ? 'fiori-dropdown-menu__item--active' : ''; ?>">
            <a href="<?php echo $data_base_path; ?>/log_oee_hourly.php" class="fiori-dropdown-menu__link">
              OEE Report by Hourly
            </a>
          </li>
          <li class="fiori-dropdown-menu__item <?php echo ($current_page == 'log_oee_data.php') ? 'fiori-dropdown-menu__item--active' : ''; ?>">
            <a href="<?php echo $data_base_path; ?>/log_oee_row.php" class="fiori-dropdown-menu__link">
              OEE Report by Row data
            </a>
          </li>
        </ul>
      </li>

      <!-- Dashboard (단독) -->
      <li class="fiori-nav-menu__item <?php echo ($current_page == 'dashboard.php') ? 'fiori-nav-menu__item--active' : ''; ?>">
        <a href="<?php echo $data_base_path; ?>/dashboard_2.php" class="fiori-nav-menu__link">
          Dashboard
        </a>
      </li>

      <!-- AI Dashboard (단독) -->
      <li class="fiori-nav-menu__item <?php echo ($current_page == 'ai_dashboard.php') ? 'fiori-nav-menu__item--active' : ''; ?>">
        <a href="<?php echo $data_base_path; ?>/ai_dashboard_3.php" class="fiori-nav-menu__link">
          AI Dashboard
        </a>
      </li>

    </ul>
  </div>
</nav>