<?php

/**
 * nav-drawer-manage.php
 * Shared hamburger drawer for manage _2 and data _2 pages
 *
 * Usage:
 *   $nav_active  = 'factory';  // active link key
 *   $nav_context = 'manage';   // 'manage' (default) | 'data'
 *   require_once(__DIR__ . '/../../inc/nav-drawer-manage.php');
 */
$_nav = $nav_active ?? '';
$_ctx = $nav_context ?? 'manage';
$_m   = $_ctx === 'data' ? '../manage/' : '';    // manage 링크 prefix
$_d   = $_ctx === 'data' ? '' : '../data/';      // data 링크 prefix

function _nav_cls(string $key, string $active): string
{
    return 'nav-drawer__link' . ($key === $active ? ' nav-drawer__link--active' : '');
}
?>
<div id="navDrawerOverlay" class="nav-drawer-overlay"></div>
<div id="navDrawer" class="nav-drawer">
    <div class="nav-drawer__header">OEE SYSTEM</div>
    <nav class="nav-drawer__menu">
        <div class="nav-drawer__group">
            <div class="nav-drawer__group-title">Setting</div>
            <a href="<?= $_m ?>info_factory_2.php" class="<?= _nav_cls('factory',        $_nav) ?>">Factory</a>
            <a href="<?= $_m ?>info_line_2.php" class="<?= _nav_cls('line',           $_nav) ?>">Line</a>
            <a href="<?= $_m ?>info_machine_model_2.php" class="<?= _nav_cls('machine_model',  $_nav) ?>">Machine Model</a>
            <a href="<?= $_m ?>info_machine_2.php" class="<?= _nav_cls('machine',        $_nav) ?>">Machine</a>
            <a href="<?= $_m ?>info_design_process_2.php" class="<?= _nav_cls('design_process', $_nav) ?>">Design Process</a>
            <a href="<?= $_m ?>info_andon_2.php" class="<?= _nav_cls('andon',          $_nav) ?>">Andon</a>
            <a href="<?= $_m ?>info_downtime_2.php" class="<?= _nav_cls('downtime',       $_nav) ?>">Downtime</a>
            <a href="<?= $_m ?>info_defective_2.php" class="<?= _nav_cls('defective',      $_nav) ?>">Defective</a>
            <a href="<?= $_m ?>info_rate_color_2.php" class="<?= _nav_cls('rate_color',     $_nav) ?>">Rate Color</a>
            <a href="<?= $_m ?>info_worktime_2.php" class="<?= _nav_cls('worktime',       $_nav) ?>">Work Time</a>
        </div>
        <div class="nav-drawer__divider"></div>
        <div class="nav-drawer__group">
            <div class="nav-drawer__group-title">Monitoring</div>
            <a href="<?= $_d ?>data_oee_2.php" class="<?= _nav_cls('oee',      $_nav) ?>">OEE Monitoring</a>
            <a href="<?= $_d ?>data_andon_2.php" class="<?= _nav_cls('andon_m',  $_nav) ?>">Andon Monitoring</a>
            <a href="<?= $_d ?>data_downtime_2.php" class="<?= _nav_cls('downtime_m', $_nav) ?>">Downtime Monitoring</a>
            <a href="<?= $_d ?>data_defective_2.php" class="<?= _nav_cls('defective_m',    $_nav) ?>">Defective Monitoring</a>
            <a href="<?= $_d ?>data_offline_2.php"  class="<?= _nav_cls('offline_monitor', $_nav) ?>">Offline Monitor</a>
        </div>
        <div class="nav-drawer__divider"></div>
        <div class="nav-drawer__group">
            <div class="nav-drawer__group-title">Report</div>
            <a href="<?= $_d ?>log_oee_2.php" class="<?= _nav_cls('log_oee',        $_nav) ?>">OEE Report by Shift</a>
            <a href="<?= $_d ?>log_oee_hourly_2.php" class="<?= _nav_cls('log_oee_hourly', $_nav) ?>">OEE Report by Hourly</a>
            <a href="<?= $_d ?>log_oee_row_2.php" class="<?= _nav_cls('log_oee_row',    $_nav) ?>">OEE Report by Row data</a>
        </div>
        <div class="nav-drawer__divider"></div>
        <div class="nav-drawer__group">
            <div class="nav-drawer__group-title">Dashboard</div>
            <a href="<?= $_d ?>dashboard_2.php" class="<?= _nav_cls('oee_dashboard', $_nav) ?>">OEE Dashboard</a>
            <!-- <a href="<?= $_d ?>ai_dashboard_2.php" class="nav-drawer__link">AI Dashboard</a> -->
            <a href="<?= $_d ?>ai_dashboard_2.php" class="<?= _nav_cls('ai_dashboard', $_nav) ?>">AI Dashboard</a>
        </div>
    </nav>
</div>

<script>
    document.addEventListener('DOMContentLoaded', function() {
        var btn = document.getElementById('navDrawerBtn');
        var drawer = document.getElementById('navDrawer');
        var overlay = document.getElementById('navDrawerOverlay');

        function open() {
            drawer.classList.add('is-open');
            overlay.classList.add('is-open');
        }

        function close() {
            drawer.classList.remove('is-open');
            overlay.classList.remove('is-open');
        }
        btn.addEventListener('click', function() {
            drawer.classList.contains('is-open') ? close() : open();
        });
        overlay.addEventListener('click', close);
    });
</script>