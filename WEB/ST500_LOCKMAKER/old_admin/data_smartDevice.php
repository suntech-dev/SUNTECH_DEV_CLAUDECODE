<?php
## ST-500 Renewal (2022.02.23 Start. dev@suntech.asia)
## ST-500 login 기능 추가 (2023.05.24. dev@suntech.asia)

require_once ('./inc/head.php');

## country list
$stmt = $pdo->prepare("SELECT idx, country_name FROM `country` ORDER BY country_name ASC");
$stmt->execute();
$country_data = $stmt->fetchAll(PDO::FETCH_ASSOC);
?>

<div class="container">
    <!-- title -->
    <div class="page-title">
        <h2 class="text-primary">DATA SMART DEVICE</h2>
    </div>

    <ul class="card-title">
        <li>
            <!--<h3>LIST</h3>-->
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
            <!-- excel down -->
            <!-- <button class="btn btn-sm btn-success me-1" id="excel_down" onclick="comingSoon()">
                <svg xmlns="http://www.w3.org/2000/svg" width="24" height="24" viewBox="0 0 24 24" fill="none"
                    stroke="currentColor" stroke-width="2" stroke-linecap="round" stroke-linejoin="round"
                    class="feather feather-arrow-down-circle align-middle me-0">
                    <circle cx="12" cy="12" r="10"></circle>
                    <polyline points="8 12 12 16 16 12"></polyline>
                    <line x1="12" y1="8" x2="12" y2="16"></line>
                </svg>
                <span class="align-middle"> EXCEL DOWN</span>
            </button> -->
            <!-- add btn -->
            <!-- <button class="btn btn-sm btn-danger addData" id="add_button" data-bs-toggle="modal"
                data-bs-target="#userModal">
                <svg xmlns="http://www.w3.org/2000/svg" width="24" height="24" viewBox="0 0 24 24" fill="none"
                    stroke="currentColor" stroke-width="2" stroke-linecap="round" stroke-linejoin="round"
                    class="feather feather-plus-circle align-middle me-0">
                    <circle cx="12" cy="12" r="10"></circle>
                    <line x1="12" y1="8" x2="12" y2="16"></line>
                    <line x1="8" y1="12" x2="16" y2="12"></line>
                </svg>
                <span class="align-middle"> ADD</span>
            </button> -->
        </li>
    </ul>

    <div class="card">
        <div class="card-body">
            <div class="filter-row">
                <!-- factory filter -->
                <div class="col col-m">
                    <select class="form-select" id="searchByFactory">
                        <option value="">All Countries</option>
                        <?php foreach ($country_data as $row) { ?>
                            <option value="<?= $row['idx'] ?>">
                                <?= $row['country_name'] ?>
                            </option>
                        <?php } ?>
                    </select>
                </div>
                <!-- status filter -->
                <div class="col">
                    <!-- all -->
                    <label class="form-check form-check-inline">
                        <input class="form-check-input" type="radio" name="searchBystate" value="" checked />
                        <span class="form-check-label">ALL</span>
                    </label>
                    <!-- used -->
                    <label class="form-check form-check-inline">
                        <input class="form-check-input" type="radio" name="searchBystate" value="Y" />
                        <span class="form-check-label">USED</span>
                    </label>
                    <!-- unused -->
                    <label class="form-check form-check-inline">
                        <input class="form-check-input" type="radio" name="searchBystate" value="N" />
                        <span class="form-check-label">UNUSED</span>
                    </label>
                </div>
            </div>
            <!-- datatables -->
            <table id="data_table" class="display cell-border nowrap" style="width:100%">
                <thead>
                    <tr>
                        <th>IDX</th>
                        <th>No.</th>
                        <th>DEVICE ID</th>
                        <th>COUNTRY</th>
                        <th>NAME</th>
                        <th>STATUS</th>
                        <th>REG DATE</th>
                        <th>UPDATE DATE</th>
                    </tr>
                </thead>
            </table>
        </div>

        <?php require_once ('./inc/copyright.php'); ?>
    </div>
</div>

<!-- modal -->
<div id="userModal" tabindex="-1">
    <div class="modal-dialog">
        <form method="post" id="modal_form" enctype="multipart/form-data">
            <!-- modal content-->
            <div class="modal-content">
                <div class="modal-header">
                    <h4 class="modal-title">EDIT SMART DEVICE</h4>
                </div>

                <!-- modal body -->
                <div class="modal-body">
                    <div class="form-group">
                        <div class="row">
                            <label>IDX</label>
                        </div>
                        <div class="row">
                            <input type="text" class="form-control" name="data_idx" id="data_idx" disabled />
                        </div>
                    </div>

                    <div class="form-group">
                        <div class="row">
                            <label>DEVICE ID</label>
                        </div>
                        <div class="row">
                            <input type="text" class="form-control" name="device_id" id="device_id" disabled />
                        </div>
                    </div>

                    <div class="form-group">
                        <div class="row">
                            <label>COUNTRY</label>
                        </div>
                        <div class="row">
                            <select name="country_idx" id="country_idx" class="form-select">
                                <?php foreach ($country_data as $row) { ?>
                                    <option value="<?= $row['idx'] ?>"> <?= $row['country_name'] ?> </option>
                                <?php } ?>
                            </select>
                        </div>
                    </div>

                    <div class="form-group">
                        <div class="row">
                            <label>STATUS</label>
                        </div>
                        <div class="row">
                            <select name="status" id="status" class="form-select">
                                <option value='Y'>Used</option>
                                <option value='N'>Unused</option>
                                <option value='D'>Delete</option>
                            </select>
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
<!-- END  modal -->

<script>
    $(document).ready(function () {
        /* datatables */
        var dataTable = $('#data_table').DataTable({
            language: {
                info: '_START_ to _END_. Total _TOTAL_.',
                infoFiltered: '',
                lengthMenu: 'Show _MENU_',
                search: 'Search (Factory or Zone or Remark) : '
            },
            stateSave: true,
            order: [],
            processing: true,
            serverSide: true,
            serverMethod: 'post',
            colReorder: true,
            ajax: {
                url: 'proc/setting_zone_fetch.php',
                data: function (data) {
                    data.searchByFactory = $('#searchByFactory').val();
                    data.searchBystate = $('input:radio[name=searchBystate]:checked').val();
                },
            },
            columns: [
                { data: 'idx', },
                { data: 'no', },
                { data: 'factory_name', },
                { data: 'zone_name', },
                { data: 'status', },
                { data: 'remark', },
                { data: 'update_date', },
            ],
            // 특정 조건에 따라 cell style 변경
            createdRow: function (row, data, dataIndex) { },
            // column 조작
            columnDefs: [{
                targets: '_all',
                createdCell: function (td, cellData, rowData, rowIndex, colIndex) {
                    $(td).html('<a href="javascript:;" class="status status_' + rowData.status + ' update" idx="' + rowData.idx + '">' + cellData + '</a>');
                }
            }, {
                targets: [1],
                orderable: false
            }],
        });
        dataTable.column(0).visible(false);   // idx 항목 감추기

        /* search button */
        $('#searchByFactory').change(function () {
            dataTable.draw();
        });
        $("input[name='searchBystate']").change(function () {
            dataTable.draw();
        });

        /* add modal */
        $('#add_button').click(function () {
            $('#modal_form')[0].reset();
            $('.modal-title').text("ADD ZONE");
            $('#action').val("Add");
            $('#operation').val("Add");
            $('#Duplicate_msg').html('');
            showDialogWindow(580, 520, false);
        });

        /* excel down button */
        $("#excel_down").click(function () {
            /*if (!dataTable.data().count()) {
                alert('There is no data to export.');
            } else {
                var data = dataTable.ajax.params(); 
                var params = $.param(data);
                document.location.href = 'proc/setting_zone_fetch.php?' + params + '&excelDown=Y';
            }*/
        });

        /* refresh button */
        $("#reload").click(function () {
            $('#data_table').DataTable().state.clear();
            location.reload();
        });

        /* edit modal */
        $(document).on('click', '.update', function () {
            var idx = $(this).attr("idx");
            $.ajax({
                url: "proc/setting_zone_fetch.php",
                method: "POST",
                data: {
                    operation: 'get',
                    idx: idx
                },
                dataType: "json",
                success: function (data) {
                    if (data.error) {
                        alert(data.error);
                    } else {
                        $('#data_idx').val(idx);
                        $('#idx').val(idx);
                        $('#factory_idx').val(data.factory_idx);
                        $('#zone_name').val(data.zone_name);
                        $('#status').val(data.status);
                        $('#remark').val(data.remark);
                        $('.modal-title').text("EDIT ZONE");
                        $('#action').val("Edit");
                        $('#operation').val("Edit");
                        $('#Duplicate_msg').html('');

                        showDialogWindow(580, 520, false);
                    }
                }
            });
        });

        /* modal submit */
        $(document).on('submit', '#modal_form', function (event) {
            event.preventDefault();
            var zone_name = $('#zone_name').val();
            var status = $('#status').val();
            if (zone_name != '' && status != '') {
                $.ajax({
                    url: "proc/setting_zone_insert.php",
                    method: 'POST',
                    data: new FormData(this),
                    contentType: false,
                    processData: false,
                    dataType: 'json',
                    success: function (data) {
                        if (data.error) {
                            alert(data.error);
                        } else {
                            $('#modal_form')[0].reset();
                            $('#userModal').hide();
                            location.reload(); // submit 이후 datatables 새로고침
                        }
                    }
                });
            } else {
                alert("Missing required items.");
            }
        });

        /* 중복체크 처리 */
        $("#Duplicate_check").on("click", function () {
            var factory_idx = $('#factory_idx').val();
            var zone_name = $('#zone_name').val();
            if (zone_name != '') {
                $.ajax({
                    url: "proc/setting_zone_insert.php",
                    method: 'POST',
                    data: $('#modal_form').serialize() + "&check=Y",
                    dataType: 'json',
                    success: function (data) {
                        if (data.error) {
                            $('#Duplicate_msg').html(data.error); // exist
                            $('#Duplicate_msg').removeClass('text-primary');
                            $('#Duplicate_msg').addClass('text-warning');
                        } else {
                            $('#Duplicate_msg').html(data.msg); // available
                            $('#Duplicate_msg').removeClass('text-warning');
                            $('#Duplicate_msg').addClass('text-primary');
                        }
                    },
                    error: function (res) {
                        alert('' + res.status + ' ' + res.statusText);
                    }
                });
            } else {
                alert("Enter the factory name");
            }
        });
    });
</script>
<?php require_once ('./inc/footer.php'); ?>