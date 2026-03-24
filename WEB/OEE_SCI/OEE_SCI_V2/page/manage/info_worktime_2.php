<?php
// worktime_head.php 유지 (jQuery DataTables + daterangepicker — 별도 보존)
require_once(__DIR__ . '/../../inc/worktime_head.php');
// nav-fiori.php 제거

## 탭메뉴용 factory list
$stmt = $pdo->prepare("SELECT idx, factory_name FROM `info_factory` WHERE status = 'Y' ORDER BY factory_name ASC");
$stmt->execute();
$factory_data = $stmt->fetchAll(PDO::FETCH_ASSOC);

## 탭메뉴용 Line list
foreach ($factory_data as $key => $val) {
  $stmt = $pdo->prepare("SELECT idx, line_name FROM `info_line` WHERE status = 'Y' AND factory_idx = ? ORDER BY line_name ASC");
  $stmt->execute([$val['idx']]);

  $line_data = $stmt->fetchAll(PDO::FETCH_ASSOC);

  if ($stmt->rowCount() > 0) {
    break;
  }
}
?>

<link rel="stylesheet" href="css/info_worktime_2.css">

<style type="text/css">
    .input-datepicker {
        font-size: .825rem;
    }

    .dropdown-menu {
        background-color: #2f3546;
    }

    .custom-checkbox {
        margin-right: 1rem;
    }

    #monthDiv table {
        border-collapse: collapse;
    }

    #calendar {
        width: 100%;
        font-size: 0.87rem;
    }

    #calendar thead th {
        padding: 4px;
        text-align: center;
        border: 1px solid #888;
        color: #a9acb3;
    }

    #calendar tbody td {
        padding: 4px;
        height: 95px;
        min-height: 95px;
        text-align: right;
        vertical-align: top;
        position: relative;
        border: 1px solid #888;
        font-size: 0.81rem;
        color: #a9acb3;
    }

    #calendar tbody td .overtime {
        color: #ad7c7c
    }

    #calendar .day_name {
        text-align: right;
        margin-bottom: 8px;
        color: #ddd;
    }

    #calendar .day_cell:hover {
        background-color: #232732;
    }

    #calendar .day_kind {
        color: #676872;
        font-size: 0.7rem;
        background-color: #272b3c;
        border-radius: 0.2rem;
        padding: 0rem 0.2rem;
        position: absolute;
        left: 4px;
        top: 18px;
    }

    #calendar .today {
        background-color: #3f444a;
    }

    .form-control.input-time.is-invalid {
        border-color: #dc3545;
        box-shadow: 0 0 0 0.2rem rgba(220, 53, 69, 0.25);
        background-color: rgba(220, 53, 69, 0.1);
    }

    .form-control.input-time.is-invalid:focus {
        border-color: #dc3545;
        box-shadow: 0 0 0 0.2rem rgba(220, 53, 69, 0.25);
    }

    .form-control.input-time:not(.is-invalid):focus {
        border-color: #198754;
        box-shadow: 0 0 0 0.2rem rgba(25, 135, 84, 0.25);
    }

    .fc-toolbar {
        overflow: hidden;
        width: 100%;
        height: 40px;
        position: relative;
    }

    .fc-toolbar-chunk:first-child {
        position: absolute;
        left: 0;
        font-size: 0.9rem;
    }

    .fc-toolbar-chunk:nth-child(2) {
        position: absolute;
        left: 50%;
        margin-left: -110px;
        width: 220px;
        text-align: center;
        color: white;
        font-size: 1.4rem;
        font-weight: 400
    }

    .fc-toolbar-chunk:last-child {
        position: absolute;
        right: 0;
    }

    .card .row {
        overflow: hidden;
        width: 100%;
    }

    .card .row>div {
        float: left;
    }

    .card .row>div:first-child {
        width: 58%;
    }

    .card .row>div:last-child {
        width: 42%;
        padding-left: 22px;
    }

    #time_list h3 {
        font-size: 1.2375rem;
        color: #fff;
        font-weight: 500;
        line-height: 1.2;
        margin-bottom: 0.5rem;
        margin-top: 0;
    }
</style>

<?php $nav_active = 'worktime'; require_once(__DIR__ . '/../../inc/nav-drawer-manage.php'); ?>

<!-- Signage Header -->
<div class="signage-header">
  <button id="navDrawerBtn" class="nav-drawer-btn" aria-label="Menu">&#9776;</button>
  <span class="signage-header__title">Work Time Management</span>
  <div class="signage-header__filters">
    <button class="btn btn-sm btn-primary me-1" id="reload">&#8635; REFRESH</button>
    <button class="btn btn-sm btn-danger addData" id="add_button2">+ ADD WORK TIME</button>
  </div>
</div>

<div class="container wt-body">

    <div class="card">
        <div class="card-body">
            <div class="row">
                <div class="col-6" id="monthDiv"></div>
                <div class="col-4" id="time_list">
                    <div>
                        <h3>WORK TIME LIST</h3>
                    </div>

                    <ul class="quick-update">
                        <li class="w-180 pad-r-20">
                    <select class="form-select" id="searchByFactory">
                      <?php foreach ($factory_data as $row) { ?>
                      <option value="<?=$row['idx'] ?>">
                        <?=$row['factory_name']?>
                      </option>
                      <?php } ?>
                    </select>
                        </li>
                        <li class="w-180 pad-r-20">
                    <select class="form-select" id="searchByLine">
                      <?php foreach ($line_data as $row) { ?>
                      <option value="<?=$row['idx']?>">
                        <?=$row['line_name']?>
                      </option>
                      <?php } ?>
                    </select>
                        </li>
                    </ul>

                    <!-- datatables : period -->
                    <table id="data_table" class="display cell-border nowrap" style="width:100% !important">
                        <thead>
                            <tr>
                                <th>IDX</th>
                                <th>NO.</th>
                                <th>PERIOD</th>
                                <th>AREA</th>
                                <th>DAY of THE WEEK</th>
                                <th>STATUS</th>
                                <th>TYPE</th>
                            </tr>
                        </thead>
                    </table>
                </div>
            </div>
        </div>
    </div>

</div>


<!-- modal -->
<div id="userModal" tabindex="-1">
    <div class="modal-dialog">
        <form method="post" id="modal_form" enctype="multipart/form-data">
            <!-- modal content-->
            <div class="modal-content">
                <div class="modal-header">
                    <h4 class="modal-title">ADD WORK TIME by PERIOD</h4>
                </div>

                <!-- modal body -->
                <div class="modal-body">
                    <div class="form-group">
                        <div class="row">
                            <div class="row-head">
                                <label>IDX</label>
                            </div>
                            <div class="row-data">
                                <div class="col col-4">
                                    <input type="text" class="form-control" name="data_idx" id="data_idx" disabled />
                                </div>
                            </div>
                        </div>
                    </div>

                    <div class="form-group">
                        <div class="row">
                            <div class="row-head">
                                <label>FACTORY / LINE</label>
                            </div>
                            <div class="row-data">
                                <div class="col col-4">
                                    <select name="factory_idx" id="factory_idx" class="form-select">
                                        <option value="-1">Select Factory</option>
                            <option value="">All Factory</option>
                                        <?php foreach ($factory_data as $row) { ?>
                            <option value="<?=$row['idx']?>">
                              <?=$row['factory_name']?>
                                            </option>
                                        <?php } ?>
                                    </select>
                                </div>
                                <div class="col col-4">
                                    <select name="line_idx" id="line_idx" class="form-select">
                                        <option value="-1">Select Line</option>
                            <option value="0">All Line</option>
                                    </select>
                                </div>
                            </div>
                        </div>
                    </div>

                    <div class="form-group">
                        <div class="row">
                            <div class="row-head">
                                <label>REMARK</label>
                            </div>
                            <div class="row-data">
                                <div class="col col-75p">
                                    <input type="text" class="form-control" name="remark" id="remark" />
                                </div>
                            </div>
                        </div>
                    </div>

                    <div class="form-group">
                        <div class="row">
                            <div class="row-head">
                                <label>STATUS</label>
                            </div>
                            <div class="row-data">
                                <div class="col col-4">
                                    <select name="status" id="status" class="form-select">
                                        <option value='Y'>Used</option>
                                        <option value='N'>Unused</option>
                                    </select>
                                </div>
                            </div>
                        </div>
                    </div>
                </div>

                <div class="modal-body div-line">
                    <div class="form-group">
                        <div class="row">
                            <div class="row-head">
                                <label id="period_name">PERIOD</label>
                            </div>
                            <div class="row-data">
                                <div class="col col-3">
                                    <input type="text"
                                        class="form-control input-datepicker daterange input-daterange-datepicker"
                                        name="period" id="period" autocomplete="off" placeholder="select period" />
                                </div>
                                <div class="col col-4">
                                    <input type="text" class="form-control" name="oneday" id="oneday"
                                        style="display:none" maxlength="10" readonly="readonly" autocomplete="off" />
                                </div>
                            </div>
                        </div>
                    </div>

                    <div class="form-group" id="dayOfTheWeek">
                        <div class="row">
                            <div class="row-head">
                                <label>DAY of THE WEEK</label>
                            </div>
                            <div class="row-data">
                                <div class="col" style="margin-top: 0.6rem;">
                                    <label class="form-check form-check-inline">
                                        <input class="form-check-input" type="checkbox" name="week[0]" value="Y" />
                                        <span class="form-check-label">Sun</span>
                                    </label>
                                    <label class="form-check form-check-inline">
                                        <input class="form-check-input" type="checkbox" name="week[1]" value="Y" />
                                        <span class="form-check-label">Mon</span>
                                    </label>
                                    <label class="form-check form-check-inline">
                                        <input class="form-check-input" type="checkbox" name="week[2]" value="Y" />
                                        <span class="form-check-label">Tues</span>
                                    </label>
                                    <label class="form-check form-check-inline">
                                        <input class="form-check-input" type="checkbox" name="week[3]" value="Y" />
                                        <span class="form-check-label">Wednes</span>
                                    </label>
                                    <label class="form-check form-check-inline">
                                        <input class="form-check-input" type="checkbox" name="week[4]" value="Y" />
                                        <span class="form-check-label">Thurs</span>
                                    </label>
                                    <label class="form-check form-check-inline">
                                        <input class="form-check-input" type="checkbox" name="week[5]" value="Y" />
                                        <span class="form-check-label">Fri</span>
                                    </label>
                                    <label class="form-check form-check-inline">
                                        <input class="form-check-input" type="checkbox" name="week[6]" value="Y" />
                                        <span class="form-check-label">Sat</span>
                                    </label>
                                </div>
                            </div>
                        </div>
                    </div>

                    <div class="form-group" style="margin-top:20px">
                        <div class="row">
                            <div class="row-head">
                                <label>&nbsp;</label>
                            </div>
                            <div class="row-data">
                                <div class="col col-4">
                                    <label class="white">Available Time</label>
                                </div>
                                <div class="col col-4">
                                    <label class="white">Planned Time ① ~ ⑤</label>
                                </div>
                            </div>
                        </div>
                        <div class="row">
                            <div class="row-head">
                                <label>SHIFT - 1</label>
                            </div>
                            <div class="row-data">
                                <div class="col col-4 pad-top-4">
                                    <input type="text" class="form-control input-time" name="available_stime[1]"
                                        data-mask="99:99" maxlength="5" />
                                    <div class="div-input-time">~</div>
                                    <input type="text" class="form-control input-time" name="available_etime[1]"
                                        data-mask="99:99" maxlength="5" />
                                </div>
                                <div class="col col-4 pad-top-4">
                                    <input type="text" class="form-control input-time" name="planned1_stime[1]"
                                        data-mask="99:99" maxlength="5" placeholder="①" />
                                    <div class="div-input-time">~</div>
                                    <input type="text" class="form-control input-time" name="planned1_etime[1]"
                                        data-mask="99:99" maxlength="5" />
                                </div>
                                <div class="col col-4 pad-top-4">
                                    <input type="text" class="form-control input-time" name="planned2_stime[1]"
                                        data-mask="99:99" maxlength="5" placeholder="②" />
                                    <div class="div-input-time">~</div>
                                    <input type="text" class="form-control input-time" name="planned2_etime[1]"
                                        data-mask="99:99" maxlength="5" />
                                </div>
                                <div class="col col-4 pad-top-4">
                                    <input type="text" class="form-control input-time" name="planned3_stime[1]"
                                        data-mask="99:99" maxlength="5" placeholder="③" />
                                    <div class="div-input-time">~</div>
                                    <input type="text" class="form-control input-time" name="planned3_etime[1]"
                                        data-mask="99:99" maxlength="5" />
                                </div>
                                <div class="col col-4 pad-top-4">&nbsp;
                                </div>
                                <div class="col col-4 pad-top-4">
                                    <input type="text" class="form-control input-time" name="planned4_stime[1]"
                                        data-mask="99:99" maxlength="5" placeholder="④" />
                                    <div class="div-input-time">~</div>
                                    <input type="text" class="form-control input-time" name="planned4_etime[1]"
                                        data-mask="99:99" maxlength="5" />
                                </div>
                                <div class="col col-4 pad-top-4">
                                    <input type="text" class="form-control input-time" name="planned5_stime[1]"
                                        data-mask="99:99" maxlength="5" placeholder="⑤" />
                                    <div class="div-input-time">~</div>
                                    <input type="text" class="form-control input-time" name="planned5_etime[1]"
                                        data-mask="99:99" maxlength="5" />
                                </div>
                                <div class="col col-4 pad-top-4">
                                    <span class="over-time">Over Time</span>
                                    <input type="text" class="form-control overtime" name="over_time[1]" data-mask="999"
                                        maxlength="3" />
                                    <span class="over-time">(Min)</span>
                                </div>
                            </div>
                        </div>
                    </div>

                    <div class="form-group">
                        <div class="row">
                            <div class="row-head">
                                <label>SHIFT - 2</label>
                            </div>
                            <div class="row-data">
                                <div class="col col-4 pad-top-4">
                                    <input type="text" class="form-control input-time" name="available_stime[2]"
                                        data-mask="99:99" />
                                    <div class="div-input-time">~</div>
                                    <input type="text" class="form-control input-time" name="available_etime[2]"
                                        data-mask="99:99" />
                                </div>
                                <div class="col col-4 pad-top-4">
                                    <input type="text" class="form-control input-time" name="planned1_stime[2]"
                                        data-mask="99:99" placeholder="①" />
                                    <div class="div-input-time">~</div>
                                    <input type="text" class="form-control input-time" name="planned1_etime[2]"
                                        data-mask="99:99" />
                                </div>
                                <div class="col col-4 pad-top-4">
                                    <input type="text" class="form-control input-time" name="planned2_stime[2]"
                                        data-mask="99:99" placeholder="②" />
                                    <div class="div-input-time">~</div>
                                    <input type="text" class="form-control input-time" name="planned2_etime[2]"
                                        data-mask="99:99" />
                                </div>
                                <div class="col col-4 pad-top-4">
                                    <input type="text" class="form-control input-time" name="planned3_stime[2]"
                                        data-mask="99:99" placeholder="③" />
                                    <div class="div-input-time">~</div>
                                    <input type="text" class="form-control input-time" name="planned3_etime[2]"
                                        data-mask="99:99" />
                                </div>
                                <div class="col col-4 pad-top-4">&nbsp;
                                </div>
                                <div class="col col-4 pad-top-4">
                                    <input type="text" class="form-control input-time" name="planned4_stime[2]"
                                        data-mask="99:99" placeholder="④" />
                                    <div class="div-input-time">~</div>
                                    <input type="text" class="form-control input-time" name="planned4_etime[2]"
                                        data-mask="99:99" />
                                </div>
                                <div class="col col-4 pad-top-4">
                                    <input type="text" class="form-control input-time" name="planned5_stime[2]"
                                        data-mask="99:99" placeholder="⑤" />
                                    <div class="div-input-time">~</div>
                                    <input type="text" class="form-control input-time" name="planned5_etime[2]"
                                        data-mask="99:99" />
                                </div>
                                <div class="col col-4 pad-top-4">
                                    <span class="over-time">Over Time</span>
                                    <input type="text" class="form-control overtime" name="over_time[2]"
                                        data-mask="999" />
                                    <span class="over-time">(Min)</span>
                                </div>
                            </div>
                        </div>
                    </div>

                    <div class="form-group">
                        <div class="row">
                            <div class="row-head">
                                <label>SHIFT - 3</label>
                            </div>
                            <div class="row-data">
                                <div class="col col-4 pad-top-4">
                                    <input type="text" class="form-control input-time" name="available_stime[3]"
                                        data-mask="99:99" />
                                    <div class="div-input-time">~</div>
                                    <input type="text" class="form-control input-time" name="available_etime[3]"
                                        data-mask="99:99" />
                                </div>
                                <div class="col col-4 pad-top-4">
                                    <input type="text" class="form-control input-time" name="planned1_stime[3]"
                                        data-mask="99:99" placeholder="①" />
                                    <div class="div-input-time">~</div>
                                    <input type="text" class="form-control input-time" name="planned1_etime[3]"
                                        data-mask="99:99" />
                                </div>
                                <div class="col col-4 pad-top-4">
                                    <input type="text" class="form-control input-time" name="planned2_stime[3]"
                                        data-mask="99:99" placeholder="②" />
                                    <div class="div-input-time">~</div>
                                    <input type="text" class="form-control input-time" name="planned2_etime[3]"
                                        data-mask="99:99" />
                                </div>
                                <div class="col col-4 pad-top-4">
                                    <input type="text" class="form-control input-time" name="planned3_stime[3]"
                                        data-mask="99:99" placeholder="③" />
                                    <div class="div-input-time">~</div>
                                    <input type="text" class="form-control input-time" name="planned3_etime[3]"
                                        data-mask="99:99" />
                                </div>
                                <div class="col col-4 pad-top-4">&nbsp;
                                </div>
                                <div class="col col-4 pad-top-4">
                                    <input type="text" class="form-control input-time" name="planned4_stime[3]"
                                        data-mask="99:99" placeholder="④" />
                                    <div class="div-input-time">~</div>
                                    <input type="text" class="form-control input-time" name="planned4_etime[3]"
                                        data-mask="99:99" />
                                </div>
                                <div class="col col-4 pad-top-4">
                                    <input type="text" class="form-control input-time" name="planned5_stime[3]"
                                        data-mask="99:99" placeholder="⑤" />
                                    <div class="div-input-time">~</div>
                                    <input type="text" class="form-control input-time" name="planned5_etime[3]"
                                        data-mask="99:99" />
                                </div>
                                <div class="col col-4 pad-top-4">
                                    <span class="over-time">Over Time</span>
                                    <input type="text" class="form-control overtime" name="over_time[3]"
                                        data-mask="999" />
                                    <span class="over-time">(Min)</span>
                                </div>
                            </div>
                        </div>
                    </div>

                </div>

                <!-- modal footer-->
                <div
                    style="position:absolute; width:100%; text-align:right; bottom:0px; padding:18px; border-top:1px solid #5a5a5a">
                    <input type="hidden" name="idx" id="idx" />
                    <input type="hidden" name="operation" id="operation" />
                    <input type="submit" name="action" id="action" class="btn btn-success btn-md" value="Add" />
                    <button type="button" class="btn btn-secondary btn-md" onclick="hideDialogWindow()"> Close </button>
                </div>
            </div>
        </form>
    </div>
</div>


<script>
    var year = <?= date('Y') ?>;
    var month = <?= date('n') ?>;

    function waitForJQuery(callback) {
        if (typeof $ !== 'undefined') {
            callback();
        } else {
            setTimeout(function() {
                waitForJQuery(callback);
            }, 50);
        }
    }

    waitForJQuery(function() {
        $(document).ready(function () {

        function initSmartTimeFormatting() {
            $('.form-control.input-time[data-mask="99:99"]').each(function() {
                const $input = $(this);

                $input.off('.smartTime');

                $input.on('input.smartTime', function(e) {
                    const cursorPos = this.selectionStart;
                    let rawValue = this.value;

                    const digitsOnly = rawValue.replace(/[^0-9]/g, '');

                    let formattedValue = '';

                    if (digitsOnly.length === 0) {
                        formattedValue = '';
                    } else if (digitsOnly.length === 1) {
                        formattedValue = digitsOnly;
                    } else if (digitsOnly.length === 2) {
                        formattedValue = digitsOnly;
                    } else if (digitsOnly.length === 3) {
                        formattedValue = digitsOnly.charAt(0) + digitsOnly.charAt(1) + ':' + digitsOnly.charAt(2);
                    } else if (digitsOnly.length >= 4) {
                        const hours = digitsOnly.charAt(0) + digitsOnly.charAt(1);
                        const minutes = digitsOnly.charAt(2) + digitsOnly.charAt(3);

                        const hourNum = parseInt(hours);
                        const minuteNum = parseInt(minutes);

                        if (hourNum <= 23 && minuteNum <= 59) {
                            formattedValue = hours + ':' + minutes;
                        } else {
                            return;
                        }
                    }

                    if (formattedValue !== rawValue) {
                        this.value = formattedValue;

                        let newCursorPos = formattedValue.length;
                        if (formattedValue.length > 2 && cursorPos <= 2) {
                            newCursorPos = cursorPos;
                        }
                        this.setSelectionRange(newCursorPos, newCursorPos);
                    }
                });

                $input.on('keypress.smartTime', function(e) {
                    const char = String.fromCharCode(e.which);
                    if (!/[0-9:]/.test(char) && !e.ctrlKey && !e.metaKey) {
                        e.preventDefault();
                    }
                });

                $input.on('paste.smartTime', function(e) {
                    setTimeout(() => {
                        $(this).trigger('input.smartTime');
                    }, 10);
                });

                $input.on('blur.smartTime', function() {
                    const value = this.value;

                    if (!value) {
                        $(this).removeClass('is-invalid');
                        return;
                    }

                    if (value.includes(':')) {
                        const parts = value.split(':');
                        if (parts.length === 2) {
                            const hour = parseInt(parts[0]);
                            const minute = parseInt(parts[1]);

                            if (!isNaN(hour) && !isNaN(minute) && hour >= 0 && hour <= 23 && minute >= 0 && minute <= 59) {
                                this.value = String(hour).padStart(2, '0') + ':' + String(minute).padStart(2, '0');
                                $(this).removeClass('is-invalid');
                                $(this).removeAttr('title');
                            } else {
                                $(this).addClass('is-invalid');
                                $(this).attr('title', 'Invalid time (00:00 ~ 23:59)');
                            }
                        } else {
                            $(this).addClass('is-invalid');
                            $(this).attr('title', 'Invalid time format (HH:MM)');
                        }
                    } else {
                        const digitsOnly = value.replace(/[^0-9]/g, '');
                        if (digitsOnly.length > 0) {
                            $(this).addClass('is-invalid');
                            $(this).attr('title', 'Invalid format. e.g. 1111 => 11:11');
                        }
                    }
                });
            });
        }

        initSmartTimeFormatting();

        $(document).on('shown.bs.modal', '#userModal', function() {
            initSmartTimeFormatting();
        });

        $('.input-datepicker').daterangepicker({
            "locale": {
                "format": "YYYY-MM-DD",
                "separator": " ~ ",
                "applyLabel": "Apply",
                "cancelLabel": "Cancel",
                "fromLabel": "From",
                "toLabel": "To",
                "customRangeLabel": "Custom",
                "weekLabel": "W",
            },
            todayHighlight: true,
            autoclose: true
        });

        $('#searchByFactory').change(function() {
            var factoryData = $(this).val();
            $.ajax({
                type: 'POST',
                url: './proc/ajax_factory_line.php',
                data: 'factoryData=' + factoryData + '&init=None',
                success: function(html) {
                    $('#searchByLine').html(html);
                    dataTable.draw();
                    getMonth();
                }
            });
        });

        $('#searchByLine').change(function() {
            dataTable.draw();
            getMonth();
        });

        $('#add_button2').click(function() {
            $('#modal_form')[0].reset();
            $('#oneday').hide();
            $('#period').show();
            $('#period_name').text('PERIOD');
            $('#dayOfTheWeek').show();
            $('.modal-title').text("ADD WORK TIME by Day Of The Week");
            $('#action').val("Add");
            $('#operation').val("AddW");
            for (var i = 0; i < 7; i++) {
                $("input[name='week[" + i + "]']").attr("checked", false);
            }
            showDialogWindow(900, 750, false);
        });

        getMonth();

        $("select[name='factory_idx']").on('change', function() {
            var factoryData = $(this).val();
            $.ajax({
                type: 'POST',
                url: './proc/ajax_factory_line.php',
                data: 'factoryData=' + factoryData + '&init=AllLine',
                success: function(html) {
                    $("#modal_form select[name='line_idx']").html(html);
                }
            });
        });

        $(document).on({
            mouseover: function() {
                var idx = $(this).attr('idx');
                $('.work_time_idx_' + idx).css('background-color', '#000');
            },
            mouseout: function() {
                var idx = $(this).attr('idx');
                $('.work_time_idx_' + idx).css('background-color', '');
            }
        }, '#data_table .status');

        $(document).on('click', '.update1, .updatePeriod', function() {
            var idx = $(this).attr("idx");
            $.ajax({
                url: "proc/set_work_time_insert.php",
                method: "POST",
                data: { idx: idx },
                dataType: "json",
                success: function(data) {
                    if (data.error) {
                        alert(data.error);
                    } else {
                        $('#oneday').hide();
                        $('#period').show();
                        $('#period_name').text('PERIOD');
                        $('#dayOfTheWeek').hide();
                        $('#data_idx').val(idx);
                        $('#idx').val(idx);
                        $('#remark').val(data.remark);
                        $('#status').val(data.status);
                        $('#period').val(data.work_sdate + ' ~ ' + data.work_edate);
                        $('.modal-title').text("EDIT WORK TIME by PERIOD");
                        $('#action').val("Edit");
                        $('#operation').val("Edit");

                        $('.input-datepicker').daterangepicker({
                            startDate: data.work_sdate,
                            endDate: data.work_edate,
                            locale: { format: 'YYYY-MM-DD', separator: ' ~ ' }
                        });

                        if (data.factory_idx == '0') {
                            $('#factory_idx').val('');
                        } else {
                            $('#factory_idx').val(data.factory_idx);
                            $.ajax({
                                type: 'POST',
                                url: './proc/ajax_factory_line.php',
                                data: 'factoryData=' + data.factory_idx + '&init=AllLine',
                                success: function(html) {
                                    $("select[name='line_idx']").html(html).promise().done(function() {
                                        if (data.line_idx != '-1') {
                                            $("select[name='line_idx']").val(data.line_idx);
                                        }
                                    });
                                }
                            });
                        }

                        $("input[name='available_stime[1]']").val(data.shift[1].available_stime);
                        $("input[name='available_etime[1]']").val(data.shift[1].available_etime);
                        $("input[name='planned1_stime[1]']").val(data.shift[1].planned1_stime);
                        $("input[name='planned1_etime[1]']").val(data.shift[1].planned1_etime);
                        $("input[name='planned2_stime[1]']").val(data.shift[1].planned2_stime);
                        $("input[name='planned2_etime[1]']").val(data.shift[1].planned2_etime);
                        $("input[name='planned3_stime[1]']").val(data.shift[1].planned3_stime);
                        $("input[name='planned3_etime[1]']").val(data.shift[1].planned3_etime);
                        $("input[name='planned4_stime[1]']").val(data.shift[1].planned4_stime);
                        $("input[name='planned4_etime[1]']").val(data.shift[1].planned4_etime);
                        $("input[name='planned5_stime[1]']").val(data.shift[1].planned5_stime);
                        $("input[name='planned5_etime[1]']").val(data.shift[1].planned5_etime);
                        $("input[name='over_time[1]']").val(data.shift[1].over_time);

                        $("input[name='available_stime[2]']").val(data.shift[2].available_stime);
                        $("input[name='available_etime[2]']").val(data.shift[2].available_etime);
                        $("input[name='planned1_stime[2]']").val(data.shift[2].planned1_stime);
                        $("input[name='planned1_etime[2]']").val(data.shift[2].planned1_etime);
                        $("input[name='planned2_stime[2]']").val(data.shift[2].planned2_stime);
                        $("input[name='planned2_etime[2]']").val(data.shift[2].planned2_etime);
                        $("input[name='planned3_stime[2]']").val(data.shift[2].planned3_stime);
                        $("input[name='planned3_etime[2]']").val(data.shift[2].planned3_etime);
                        $("input[name='planned4_stime[2]']").val(data.shift[2].planned4_stime);
                        $("input[name='planned4_etime[2]']").val(data.shift[2].planned4_etime);
                        $("input[name='planned5_stime[2]']").val(data.shift[2].planned5_stime);
                        $("input[name='planned5_etime[2]']").val(data.shift[2].planned5_etime);
                        $("input[name='over_time[2]']").val(data.shift[2].over_time);

                        $("input[name='available_stime[3]']").val(data.shift[3].available_stime);
                        $("input[name='available_etime[3]']").val(data.shift[3].available_etime);
                        $("input[name='planned1_stime[3]']").val(data.shift[3].planned1_stime);
                        $("input[name='planned1_etime[3]']").val(data.shift[3].planned1_etime);
                        $("input[name='planned2_stime[3]']").val(data.shift[3].planned2_stime);
                        $("input[name='planned2_etime[3]']").val(data.shift[3].planned2_etime);
                        $("input[name='planned3_stime[3]']").val(data.shift[3].planned3_stime);
                        $("input[name='planned3_etime[3]']").val(data.shift[3].planned3_etime);
                        $("input[name='planned4_stime[3]']").val(data.shift[3].planned4_stime);
                        $("input[name='planned4_etime[3]']").val(data.shift[3].planned4_etime);
                        $("input[name='planned5_stime[3]']").val(data.shift[3].planned5_stime);
                        $("input[name='planned5_etime[3]']").val(data.shift[3].planned5_etime);
                        $("input[name='over_time[3]']").val(data.shift[3].over_time);

                        showDialogWindow(900, 750, false);
                    }
                }
            });
        });

        $(document).on('click', '.update2, .updateWeek', function() {
            var idx = $(this).attr("idx");
            $.ajax({
                url: "proc/set_work_time_insert.php",
                method: "POST",
                data: { idx: idx },
                dataType: "json",
                success: function(data) {
                    if (data.error) {
                        alert(data.error);
                    } else {
                        $('#oneday').hide();
                        $('#period').show();
                        $('#period_name').text('PERIOD');
                        $('#dayOfTheWeek').show();
                        $('#data_idx').val(idx);
                        $('#idx').val(idx);
                        $('#remark').val(data.remark);
                        $('#status').val(data.status);
                        $('#period').val(data.work_sdate + ' ~ ' + data.work_edate);
                        $('.modal-title').text("EDIT WORK TIME by Day of The Week");
                        $('#action').val("Edit");
                        $('#operation').val("EditW");

                        $('.input-datepicker').daterangepicker({
                            startDate: data.work_sdate,
                            endDate: data.work_edate,
                            locale: { format: 'YYYY-MM-DD', separator: ' ~ ' }
                        });

                        if (data.factory_idx == '0') {
                            $('#factory_idx').val('');
                        } else {
                            $('#factory_idx').val(data.factory_idx);
                            $.ajax({
                                type: 'POST',
                                url: './proc/ajax_factory_line.php',
                                data: 'factoryData=' + data.factory_idx + '&init=AllLine',
                                success: function(html) {
                                    $("select[name='line_idx']").html(html).promise().done(function() {
                                        if (data.line_idx != '-1') {
                                            $("select[name='line_idx']").val(data.line_idx);
                                        }
                                    });
                                }
                            });
                        }

                        var len = data.week_yn.length;
                        for (var i = 0; i < len; i++) {
                            if (data.week_yn.substr(i, 1) == '1') {
                                $("input[name='week[" + i + "]']").attr("checked", true);
                            } else {
                                $("input[name='week[" + i + "]']").attr("checked", false);
                            }
                        }

                        $("input[name='available_stime[1]']").val(data.shift[1].available_stime);
                        $("input[name='available_etime[1]']").val(data.shift[1].available_etime);
                        $("input[name='planned1_stime[1]']").val(data.shift[1].planned1_stime);
                        $("input[name='planned1_etime[1]']").val(data.shift[1].planned1_etime);
                        $("input[name='planned2_stime[1]']").val(data.shift[1].planned2_stime);
                        $("input[name='planned2_etime[1]']").val(data.shift[1].planned2_etime);
                        $("input[name='planned3_stime[1]']").val(data.shift[1].planned3_stime);
                        $("input[name='planned3_etime[1]']").val(data.shift[1].planned3_etime);
                        $("input[name='planned4_stime[1]']").val(data.shift[1].planned4_stime);
                        $("input[name='planned4_etime[1]']").val(data.shift[1].planned4_etime);
                        $("input[name='planned5_stime[1]']").val(data.shift[1].planned5_stime);
                        $("input[name='planned5_etime[1]']").val(data.shift[1].planned5_etime);
                        $("input[name='over_time[1]']").val(data.shift[1].over_time);

                        $("input[name='available_stime[2]']").val(data.shift[2].available_stime);
                        $("input[name='available_etime[2]']").val(data.shift[2].available_etime);
                        $("input[name='planned1_stime[2]']").val(data.shift[2].planned1_stime);
                        $("input[name='planned1_etime[2]']").val(data.shift[2].planned1_etime);
                        $("input[name='planned2_stime[2]']").val(data.shift[2].planned2_stime);
                        $("input[name='planned2_etime[2]']").val(data.shift[2].planned2_etime);
                        $("input[name='planned3_stime[2]']").val(data.shift[2].planned3_stime);
                        $("input[name='planned3_etime[2]']").val(data.shift[2].planned3_etime);
                        $("input[name='planned4_stime[2]']").val(data.shift[2].planned4_stime);
                        $("input[name='planned4_etime[2]']").val(data.shift[2].planned4_etime);
                        $("input[name='planned5_stime[2]']").val(data.shift[2].planned5_stime);
                        $("input[name='planned5_etime[2]']").val(data.shift[2].planned5_etime);
                        $("input[name='over_time[2]']").val(data.shift[2].over_time);

                        $("input[name='available_stime[3]']").val(data.shift[3].available_stime);
                        $("input[name='available_etime[3]']").val(data.shift[3].available_etime);
                        $("input[name='planned1_stime[3]']").val(data.shift[3].planned1_stime);
                        $("input[name='planned1_etime[3]']").val(data.shift[3].planned1_etime);
                        $("input[name='planned2_stime[3]']").val(data.shift[3].planned2_stime);
                        $("input[name='planned2_etime[3]']").val(data.shift[3].planned2_etime);
                        $("input[name='planned3_stime[3]']").val(data.shift[3].planned3_stime);
                        $("input[name='planned3_etime[3]']").val(data.shift[3].planned3_etime);
                        $("input[name='planned4_stime[3]']").val(data.shift[3].planned4_stime);
                        $("input[name='planned4_etime[3]']").val(data.shift[3].planned4_etime);
                        $("input[name='planned5_stime[3]']").val(data.shift[3].planned5_stime);
                        $("input[name='planned5_etime[3]']").val(data.shift[3].planned5_etime);
                        $("input[name='over_time[3]']").val(data.shift[3].over_time);

                        showDialogWindow(900, 750, false);
                    }
                }
            });
        });

        $(document).on('click', '.update3, .updateDay', function() {
            var oneday = $(this).attr('date');
            var searchFactory = $('#searchByFactory').val();
            var searchLine = $('#searchByLine').val();
            $.ajax({
                url: "proc/set_work_time_insert.php",
                method: "POST",
                data: {
                    oneday: oneday,
                    factory_idx: searchFactory,
                    line_idx: searchLine,
                },
                dataType: "json",
                success: function(data) {
                    if (data.error) {
                        alert(data.error);
                    } else {
                        $('#modal_form')[0].reset();
                        $('#oneday').show();
                        $('#period').hide();
                        $('#period_name').text('DATE');
                        $('#dayOfTheWeek').hide();
                        $('#data_idx').val(data.idx);
                        $('#idx').val(data.idx);
                        $('#remark').val(data.remark);
                        $('#status').val(data.status);
                        $('#oneday').val(oneday);
                        if (data.idx == '') {
                            $('.modal-title').text("ADD WORK TIME by Day");
                            $('#action').val("Add");
                            $('#operation').val("AddD");
                        } else {
                            $('.modal-title').text("EDIT WORK TIME by Day");
                            $('#action').val("Edit");
                            $('#operation').val("EditD");
                        }

                        $('.input-datepicker').daterangepicker({
                            startDate: data.work_sdate,
                            endDate: data.work_edate,
                            locale: { format: 'YYYY-MM-DD', separator: ' ~ ' }
                        });

                        if (data.factory_idx == '0') {
                            $('#factory_idx').val('');
                        } else {
                            $('#factory_idx').val(data.factory_idx);
                            $.ajax({
                                type: 'POST',
                                url: './proc/ajax_factory_line.php',
                                data: 'factoryData=' + data.factory_idx + '&init=AllLine',
                                success: function(html) {
                                    $("select[name='line_idx']").html(html).promise().done(function() {
                                        if (data.line_idx != '-1') {
                                            $("select[name='line_idx']").val(data.line_idx);
                                        }
                                    });
                                }
                            });
                        }

                        $("input[name='available_stime[1]']").val(data.shift[1].available_stime);
                        $("input[name='available_etime[1]']").val(data.shift[1].available_etime);
                        $("input[name='planned1_stime[1]']").val(data.shift[1].planned1_stime);
                        $("input[name='planned1_etime[1]']").val(data.shift[1].planned1_etime);
                        $("input[name='planned2_stime[1]']").val(data.shift[1].planned2_stime);
                        $("input[name='planned2_etime[1]']").val(data.shift[1].planned2_etime);
                        $("input[name='planned3_stime[1]']").val(data.shift[1].planned3_stime);
                        $("input[name='planned3_etime[1]']").val(data.shift[1].planned3_etime);
                        $("input[name='planned4_stime[1]']").val(data.shift[1].planned4_stime);
                        $("input[name='planned4_etime[1]']").val(data.shift[1].planned4_etime);
                        $("input[name='planned5_stime[1]']").val(data.shift[1].planned5_stime);
                        $("input[name='planned5_etime[1]']").val(data.shift[1].planned5_etime);
                        $("input[name='over_time[1]']").val(data.shift[1].over_time);

                        $("input[name='available_stime[2]']").val(data.shift[2].available_stime);
                        $("input[name='available_etime[2]']").val(data.shift[2].available_etime);
                        $("input[name='planned1_stime[2]']").val(data.shift[2].planned1_stime);
                        $("input[name='planned1_etime[2]']").val(data.shift[2].planned1_etime);
                        $("input[name='planned2_stime[2]']").val(data.shift[2].planned2_stime);
                        $("input[name='planned2_etime[2]']").val(data.shift[2].planned2_etime);
                        $("input[name='planned3_stime[2]']").val(data.shift[2].planned3_stime);
                        $("input[name='planned3_etime[2]']").val(data.shift[2].planned3_etime);
                        $("input[name='planned4_stime[2]']").val(data.shift[2].planned4_stime);
                        $("input[name='planned4_etime[2]']").val(data.shift[2].planned4_etime);
                        $("input[name='planned5_stime[2]']").val(data.shift[2].planned5_stime);
                        $("input[name='planned5_etime[2]']").val(data.shift[2].planned5_etime);
                        $("input[name='over_time[2]']").val(data.shift[2].over_time);

                        $("input[name='available_stime[3]']").val(data.shift[3].available_stime);
                        $("input[name='available_etime[3]']").val(data.shift[3].available_etime);
                        $("input[name='planned1_stime[3]']").val(data.shift[3].planned1_stime);
                        $("input[name='planned1_etime[3]']").val(data.shift[3].planned1_etime);
                        $("input[name='planned2_stime[3]']").val(data.shift[3].planned2_stime);
                        $("input[name='planned2_etime[3]']").val(data.shift[3].planned2_etime);
                        $("input[name='planned3_stime[3]']").val(data.shift[3].planned3_stime);
                        $("input[name='planned3_etime[3]']").val(data.shift[3].planned3_etime);
                        $("input[name='planned4_stime[3]']").val(data.shift[3].planned4_stime);
                        $("input[name='planned4_etime[3]']").val(data.shift[3].planned4_etime);
                        $("input[name='planned5_stime[3]']").val(data.shift[3].planned5_stime);
                        $("input[name='planned5_etime[3]']").val(data.shift[3].planned5_etime);
                        $("input[name='over_time[3]']").val(data.shift[3].over_time);

                        showDialogWindow(900, 750, false);
                    }
                }
            });
        });

        $(document).on('submit', '#modal_form', function(event) {
            event.preventDefault();
            var factory_idx = $('#factory_idx').val();
            var line_idx = $('#line_idx').val();
            var period = $('#period').val();
            if (factory_idx != '-1') {
                if (line_idx != '-1') {
                    if (period != '') {
                        $.ajax({
                            url: "proc/set_work_time_insert.php",
                            method: 'POST',
                            data: new FormData(this),
                            contentType: false,
                            processData: false,
                            dataType: 'json',
                            success: function(data) {
                                if (data.error) {
                                    alert(data.error);
                                } else {
                                    $('#modal_form')[0].reset();
                                    $('#userModal').hide();
                                    hideDialogWindow();
                                    if (typeof dataTable != "undefined") {
                                        dataTable.draw();
                                    }
                                    getMonth();
                                }
                            }
                        });
                    } else {
                        alert("Missing required period.");
                    }
                } else {
                    alert("Please select line");
                }
            } else {
                alert("Please select building");
            }
        });

        var dataTable = $('#data_table').DataTable({
            paging: false,
            info: false,
            searching: false,
            order: [],
            processing: true,
            serverSide: true,
            serverMethod: 'post',
            colReorder: true,
            responsive: {
                details: {
                    display: $.fn.dataTable.Responsive.display.childRow
                }
            },
            ajax: {
                url: 'proc/set_work_time_fetch.php',
                data: function(data) {
                    var searchStatus = $('input:radio[name=searchBystate]:checked').val();
                    var searchFactory = $('#searchByFactory').val();
                    var searchLine = $('#searchByLine').val();

                    data.searchByFactory = searchFactory;
                    data.searchByLine = searchLine;
                    data.searchBystate = searchStatus;
                    data.kind = 0;
                },
            },
            columns: [
                { data: 'idx' },
                { data: 'no' },
                { data: 'period' },
                { data: 'locate' },
                { data: 'week_yn_names' },
                { data: 'status' },
                { data: 'kind' },
            ],
            createdRow: function(row, data, dataIndex) {},
            columnDefs: [
                {
                    targets: '_all',
                    createdCell: function(td, cellData, rowData, rowIndex, colIndex) {
                        $(td).html('<a href="javascript:;" class="status status_' + rowData.status +
                            ' update' + rowData.kind + '" name="update" idx="' + rowData.idx +
                            '" date="' + rowData.period + '" data-toggle="modal" data-target="#userModal" data-backdrop="static">' + cellData + '</a>');
                    }
                }, {
                    targets: [1, 4],
                    orderable: false
                }
            ],
        });
        dataTable.column(0).visible(false);

        function getMonth() {
            $.ajax({
                url: "proc/set_work_time_fetch_month.php",
                method: "POST",
                data: {
                    year: year,
                    month: month,
                    factory_id: $('#searchByFactory').val(),
                    line_id: $('#searchByLine').val()
                },
                dataType: "json",
                success: function(data) {
                    if (data.error) {
                        alert(data.error);
                    } else {
                        $('#monthDiv').html(data.html);
                    }
                }
            });
        }
        function prevMonth() {
            if (month <= 1) {
                month = 12;
                year--;
            } else {
                month--;
            }
            getMonth();
        }
        function nextMonth() {
            if (month >= 12) {
                month = 1;
                year++;
            } else {
                month++;
            }
            getMonth();
        }
        function thisMonth() {
            year = <?=date('Y')?>;
            month = <?=date('n')?>;
            getMonth();
        }
        window.prevMonth = prevMonth;
        window.nextMonth = nextMonth;
        window.thisMonth = thisMonth;

        $("#reload").click(function() {
            location.reload();
        });

        }); // $(document).ready
    }); // waitForJQuery
</script>


</body>
</html>
