<?php
// 공통 헤더 로드
// require_once(__DIR__ . '/../inc/head.php');
require_once(__DIR__ . '/../../inc/worktime_head.php');
require_once(__DIR__ . '/../../inc/nav-fiori.php');

## 탭메뉴용 factory list
$stmt = $pdo->prepare("SELECT idx, factory_name FROM `info_factory` WHERE status = 'Y' ORDER BY factory_name ASC");
$stmt->execute();
$factory_data = $stmt->fetchAll(PDO::FETCH_ASSOC);

## 탭메뉴용 Line list
foreach ($factory_data as $key => $val) {
  $stmt = $pdo->prepare("SELECT idx, line_name FROM `info_line` WHERE status = 'Y' AND factory_idx = ? ORDER BY line_name ASC");
  $stmt->execute([$val['idx']]);

  $line_data = $stmt->fetchAll(PDO::FETCH_ASSOC);

  // Line 정보가 있는 Factory를 찾을때 까지 반복한다.
  if ($stmt->rowCount() > 0) {
    ## Line 정보가 있으므로 첫번째 값을 current 값으로 지정하고 빠져나간다.
    break;
  }
}

## 탭메뉴용 Line list
/* if (isset($factory_data[0])) {
    $stmt = $pdo->prepare("SELECT idx, line_name FROM `info_line` WHERE status = 'Y' AND factory_idx = ? ORDER BY line_name ASC");
    $stmt->execute([$factory_data[0]['idx']]);
    $line_data = $stmt->fetchAll(PDO::FETCH_ASSOC);
} else {
    $line_data = array();
} */

// echo '<pre>'; print_r($factory_data); print_r($line_data); echo '</pre>';
?>


<style type="text/css">
    /*
.daterangepicker {
    background-color: #293042;
    border-color: #696e7b;
}
.daterangepicker .calendar-table {
    background-color: #293042;
    border-color: #293042;
}
*/
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

    /* 시간 입력 필드 유효성 검사 스타일 */
    .form-control.input-time.is-invalid {
        border-color: #dc3545;
        box-shadow: 0 0 0 0.2rem rgba(220, 53, 69, 0.25);
        background-color: rgba(220, 53, 69, 0.1);
    }

    .form-control.input-time.is-invalid:focus {
        border-color: #dc3545;
        box-shadow: 0 0 0 0.2rem rgba(220, 53, 69, 0.25);
    }

    /* 시간 입력 필드 성공 상태 */
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

    /* SAP Fiori 헤더 스타일 적용 */
    .fiori-main-header h2 {
        color: #0070f2;  /* SAP brand primary */
        font-weight: 600;  /* semibold */
        margin: 0 0 4px 0;
        letter-spacing: -0.025em;
        font-size: 1.5rem;  /* xl size */
    }
</style>

  <!-- <div class="fiori-container">
    <main> -->
      
      <!-- 페이지 헤더 -->

<div class="container">
      <!-- 페이지 헤더 -->
      <div class="fiori-main-header">
        <div>
          <h2>Work Time Management</h2>
        </div>
      </div>

    <ul class="card-title">
        <li>
            <!-- <h3 class="text-primary">LIST</h3> -->
        </li>
        <li class="area-buttons">
            <!-- refresh btn -->
            <button class="btn btn-sm btn-primary me-1" id="reload">
                <svg xmlns="http://www.w3.org/2000/svg" width="24" height="24" viewBox="0 0 24 24" fill="none"
                    stroke="currentColor" stroke-width="2" stroke-linecap="round" stroke-linejoin="round"
                    class="feather feather-refresh-cw align-middle">
                    <polyline points="23 4 23 10 17 10"></polyline>
                    <polyline points="1 20 1 14 7 14"></polyline>
                    <path d="M3.51 9a9 9 0 0 1 14.85-3.36L23 10M1 14l4.64 4.36A9 9 0 0 0 20.49 15"></path>
                </svg>
                <span class="align-middle"> REFRESH</span>
            </button>
            <!-- add btn -->
            <!-- <button class="btn btn-sm btn-danger addData" id="add_button">
                <svg xmlns="http://www.w3.org/2000/svg" width="24" height="24" viewBox="0 0 24 24" fill="none"
                    stroke="currentColor" stroke-width="2" stroke-linecap="round" stroke-linejoin="round"
                    class="feather feather-plus-circle align-middle me-0">
                    <circle cx="12" cy="12" r="10"></circle>
                    <line x1="12" y1="8" x2="12" y2="16"></line>
                    <line x1="8" y1="12" x2="16" y2="12"></line>
                </svg>
                <span class="align-middle"> ADD WORK TIME PERIOD</span>
            </button> -->
            <button class="btn btn-sm btn-danger addData" id="add_button2">
                <svg xmlns="http://www.w3.org/2000/svg" width="24" height="24" viewBox="0 0 24 24" fill="none"
                    stroke="currentColor" stroke-width="2" stroke-linecap="round" stroke-linejoin="round"
                    class="feather feather-plus-circle align-middle me-0">
                    <circle cx="12" cy="12" r="10"></circle>
                    <line x1="12" y1="8" x2="12" y2="16"></line>
                    <line x1="8" y1="12" x2="16" y2="12"></line>
                </svg>
                <span class="align-middle"> ADD WORK TIME</span>
            </button>
        </li>
    </ul>

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
                    <!-- NOTE -->
                </div>
            </div>
        </div>
    </div>
    <!-- </main> -->
</div>

  <!-- </div> -->

  
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

  <!-- <footer class="fiori-footer">
    <p>&copy; 2025 SUNTECH. All Rights Reserved.</p>
  </footer> -->

<script>
    var year = <?= date('Y') ?>;
    var month = <?= date('n') ?>;

    // jQuery가 로드될 때까지 대기
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

        // 시간 입력 필드용 스마트 포맷팅 기능 (1111 → 11:11)
        function initSmartTimeFormatting() {
            $('.form-control.input-time[data-mask="99:99"]').each(function() {
                const $input = $(this);
                
                // 기존 마스크 제거하고 새로운 로직 적용
                $input.off('.smartTime');
                
                // 입력 이벤트 처리 (실시간 포맷팅)
                $input.on('input.smartTime', function(e) {
                    const cursorPos = this.selectionStart;
                    let rawValue = this.value;
                    
                    // 숫자만 추출
                    const digitsOnly = rawValue.replace(/[^0-9]/g, '');
                    
                    let formattedValue = '';
                    
                    if (digitsOnly.length === 0) {
                        formattedValue = '';
                    } else if (digitsOnly.length === 1) {
                        formattedValue = digitsOnly;
                    } else if (digitsOnly.length === 2) {
                        formattedValue = digitsOnly;
                    } else if (digitsOnly.length === 3) {
                        // 123 → 12:3
                        formattedValue = digitsOnly.charAt(0) + digitsOnly.charAt(1) + ':' + digitsOnly.charAt(2);
                    } else if (digitsOnly.length >= 4) {
                        // 1111 → 11:11, 11111 → 11:11 (4자리만 사용)
                        const hours = digitsOnly.charAt(0) + digitsOnly.charAt(1);
                        const minutes = digitsOnly.charAt(2) + digitsOnly.charAt(3);
                        
                        const hourNum = parseInt(hours);
                        const minuteNum = parseInt(minutes);
                        
                        // 시간 유효성 검사
                        if (hourNum <= 23 && minuteNum <= 59) {
                            formattedValue = hours + ':' + minutes;
                        } else {
                            // 유효하지 않으면 이전 값 유지
                            return;
                        }
                    }
                    
                    // 값 업데이트
                    if (formattedValue !== rawValue) {
                        this.value = formattedValue;
                        
                        // 커서 위치 조정
                        let newCursorPos = formattedValue.length;
                        if (formattedValue.length > 2 && cursorPos <= 2) {
                            newCursorPos = cursorPos;
                        }
                        this.setSelectionRange(newCursorPos, newCursorPos);
                    }
                });
                
                // 키 입력 제한 (숫자와 콜론만 허용)
                $input.on('keypress.smartTime', function(e) {
                    const char = String.fromCharCode(e.which);
                    if (!/[0-9:]/.test(char) && !e.ctrlKey && !e.metaKey) {
                        e.preventDefault();
                    }
                });
                
                // 붙여넣기 이벤트 처리
                $input.on('paste.smartTime', function(e) {
                    setTimeout(() => {
                        $(this).trigger('input.smartTime');
                    }, 10);
                });
                
                // 포커스 아웃 시 최종 포맷팅 및 유효성 검사
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
                                // 올바른 형식으로 패딩
                                this.value = String(hour).padStart(2, '0') + ':' + String(minute).padStart(2, '0');
                                $(this).removeClass('is-invalid');
                                $(this).removeAttr('title');
                            } else {
                                // 유효하지 않은 시간
                                $(this).addClass('is-invalid');
                                $(this).attr('title', '올바른 시간 형식을 입력하세요 (00:00 ~ 23:59)');
                            }
                        } else {
                            $(this).addClass('is-invalid');
                            $(this).attr('title', '올바른 시간 형식을 입력하세요 (HH:MM)');
                        }
                    } else {
                        // 콜론이 없는 경우 - 숫자만 있다면 오류 표시
                        const digitsOnly = value.replace(/[^0-9]/g, '');
                        if (digitsOnly.length > 0) {
                            $(this).addClass('is-invalid');
                            $(this).attr('title', '시간 형식이 올바르지 않습니다. 예: 1111 → 11:11');
                        }
                    }
                });
            });
        }
        
        // 페이지 로드 시 초기화
        initSmartTimeFormatting();
        
        // 모달이 열릴 때마다 다시 초기화 (동적으로 생성된 요소들을 위해)
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

        /* modal info */
        $('#add_button').click(function() {
            $('#modal_form')[0].reset();
            $('#oneday').hide();
            $('#period').show();
            $('#period_name').text('PERIOD');
            $('#dayOfTheWeek').hide();
            $('.modal-title').text("ADD WORK TIME by PERIOD");
            $('#action').val("Add");
            $('#operation').val("Add");
            showDialogWindow(900, 750, false);
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
            // check box reset
      for (var i=0; i<7; i++) {
        $("input[name='week["+i+"]']").attr("checked", false);
            }
            showDialogWindow(900, 750, false);
        });

        getMonth();


    /* modal -> Factory & Line select box */
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

        /* list table mouse event */
        $(document).on({
            mouseover: function() {
                var idx = $(this).attr('idx');
                $('.work_time_idx_'+idx).css('background-color','#000');
            },
            mouseout: function() {
                var idx = $(this).attr('idx');
                $('.work_time_idx_'+idx).css('background-color','');
            }
        }, '#data_table .status');

        /* period edit modal */
    $(document).on('click', '.update1, .updatePeriod', function() {
            var idx = $(this).attr("idx");
            $.ajax({
        url: "proc/set_work_time_insert.php",
                method: "POST",
                data: {
                    idx: idx
                },
                dataType: "json",
                success: function(data) {
                    if (data.error) {
                        alert(data.error);
                    } else {
            console.log(data);
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
                            locale: {
                                format: 'YYYY-MM-DD',
                                separator: ' ~ ',
                            }
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
      })
    });
        /* edit modal */
        $(document).on('click', '.update2, .updateWeek', function() {
            var idx = $(this).attr("idx");
            $.ajax({
        url: "proc/set_work_time_insert.php",
                method: "POST",
                data: {
                    idx: idx
                },
                dataType: "json",
                success: function(data) {
                    if (data.error) {
                        alert(data.error);
                    } else {
            console.log(data);
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
                            locale: {
                                format: 'YYYY-MM-DD',
                                separator: ' ~ ',
                            }
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

                        // check box
                        var len = data.week_yn.length;

            for (var i=0; i<len; i++) {
              if (data.week_yn.substr(i, 1)=='1') {
                $("input[name='week["+i+"]']").attr("checked", true);
              } else {
                $("input[name='week["+i+"]']").attr("checked", false);
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
      })
    });
        /* day edit modal */
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
                        console.log(data);
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
            if (data.idx=='') {
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
                            locale: {
                                format: 'YYYY-MM-DD',
                                separator: ' ~ ',
                            }
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
      })
    });

        /* modal submit */
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
                                if (data.error) { // 실패
                                    alert(data.error);
                                } else {
                                    //alert(data.msg); // save 후 alert 메세지.
                                    $('#modal_form')[0].reset();
                                    $('#userModal').hide();
                                    hideDialogWindow();
                                    if (typeof dataTable != "undefined") {  // submit 이후 datatables 새로고침
                                        dataTable.draw();
                                    }
                                    getMonth();   // 달력 갱신
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

        /* datatables */
        var dataTable = $('#data_table').DataTable({
            //stateSave: true,
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
          display: $.fn.dataTable.Responsive.display.childRow // rows detail
        }
      },
            ajax: {
                url: 'proc/set_work_time_fetch.php',
        data: function(data) {
                    /* Read values */
                    var searchStatus = $('input:radio[name=searchBystate]:checked').val();
                    var searchFactory = $('#searchByFactory').val();
                    var searchLine = $('#searchByLine').val();

                    /* Append to data */
                    data.searchByFactory = searchFactory;
                    data.searchByLine = searchLine;
                    data.searchBystate = searchStatus;
                    data.kind = 0;
                    console.log(data);
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
            /* 특정 조건에 따라 cell style 변경 */
      createdRow: function(row, data, dataIndex) {},
            /* column 조작 */
            columnDefs: [
                {
                    targets: '_all',
                    createdCell: function(td, cellData, rowData, rowIndex, colIndex) {
                        $(td).html('<a href="javascript:;" class="status status_' + rowData.status +
                            ' update' + rowData.kind + '" name="update" idx="' + rowData.idx +
                            '" date="'+rowData.period+'" data-toggle="modal" data-target="#userModal" data-backdrop="static">' + cellData + '</a>');
                    }
                },{
                    targets: [1,4],
                    orderable: false
                }
            ],
        });
        dataTable.column(0).visible(false);   // idx 항목 감추기

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
        })
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

  /* refresh button */
  /* $("#reload").on("click", function() {
    // $('#data_table').DataTable().state.clear();
    location.reload();
  }); */
        $("#reload").click(function () {
            // $('#data_table').DataTable().state.clear();
            location.reload();
        });
        
        }); // $(document).ready 닫기
    }); // waitForJQuery 닫기
</script>
</body>
</html>