<?php
/**
 * nav-drawer-manage.php
 * Hamburger drawer for manage _2 pages
 *
 * Usage (before require_once):
 *   $nav_active = 'factory'; // factory|line|machine_model|machine|design_process|andon|downtime|defective|rate_color|worktime
 */
$_nav = $nav_active ?? '';
function _nav_cls(string $key, string $active): string {
    return 'nav-drawer__link' . ($key === $active ? ' nav-drawer__link--active' : '');
}
?>
<div id="navDrawerOverlay" class="nav-drawer-overlay"></div>
<div id="navDrawer" class="nav-drawer">
    <div class="nav-drawer__header">OEE SYSTEM</div>
    <nav class="nav-drawer__menu">
        <div class="nav-drawer__group">
            <div class="nav-drawer__group-title">Setting</div>
            <a href="info_factory_2.php"        class="<?= _nav_cls('factory',        $_nav) ?>">Factory</a>
            <a href="info_line_2.php"            class="<?= _nav_cls('line',           $_nav) ?>">Line</a>
            <a href="info_machine_model_2.php"   class="<?= _nav_cls('machine_model',  $_nav) ?>">Machine Model</a>
            <a href="info_machine_2.php"         class="<?= _nav_cls('machine',        $_nav) ?>">Machine</a>
            <a href="info_design_process_2.php"  class="<?= _nav_cls('design_process', $_nav) ?>">Design Process</a>
            <a href="info_andon_2.php"           class="<?= _nav_cls('andon',          $_nav) ?>">Andon</a>
            <a href="info_downtime_2.php"        class="<?= _nav_cls('downtime',       $_nav) ?>">Downtime</a>
            <a href="info_defective_2.php"       class="<?= _nav_cls('defective',      $_nav) ?>">Defective</a>
            <a href="info_rate_color_2.php"      class="<?= _nav_cls('rate_color',     $_nav) ?>">Rate Color</a>
            <a href="info_worktime_2.php"        class="<?= _nav_cls('worktime',       $_nav) ?>">Work Time</a>
        </div>
        <div class="nav-drawer__divider"></div>
        <div class="nav-drawer__group">
            <div class="nav-drawer__group-title">Monitoring</div>
            <a href="../data/data_oee_2.php"       class="nav-drawer__link">OEE Monitoring</a>
            <a href="../data/data_andon_2.php"     class="nav-drawer__link">Andon Monitoring</a>
            <a href="../data/data_downtime_2.php"  class="nav-drawer__link">Downtime Monitoring</a>
            <a href="../data/data_defective_2.php" class="nav-drawer__link">Defective Monitoring</a>
        </div>
        <div class="nav-drawer__divider"></div>
        <div class="nav-drawer__group">
            <div class="nav-drawer__group-title">Report</div>
            <a href="../data/log_oee_2.php"         class="nav-drawer__link">OEE Report by Shift</a>
            <a href="../data/log_oee_hourly_2.php"  class="nav-drawer__link">OEE Report by Hourly</a>
            <a href="../data/log_oee_row_2.php"     class="nav-drawer__link">OEE Report by Row data</a>
        </div>
        <div class="nav-drawer__divider"></div>
        <a href="../data/dashboard_2.php"    class="nav-drawer__link">Dashboard</a>
        <a href="../data/ai_dashboard_5.php" class="nav-drawer__link">AI Dashboard v5</a>
    </nav>
</div>

<script>
document.addEventListener('DOMContentLoaded', function() {
    var btn     = document.getElementById('navDrawerBtn');
    var drawer  = document.getElementById('navDrawer');
    var overlay = document.getElementById('navDrawerOverlay');
    function open()  { drawer.classList.add('is-open');    overlay.classList.add('is-open'); }
    function close() { drawer.classList.remove('is-open'); overlay.classList.remove('is-open'); }
    btn.addEventListener('click', function() { drawer.classList.contains('is-open') ? close() : open(); });
    overlay.addEventListener('click', close);
});
</script>
