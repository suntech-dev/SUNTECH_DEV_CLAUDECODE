<?php
## ST-500 Renewal (2022.02.23 Start. dev@suntech.asia & hamani@naver.com)

require_once($_SERVER['DOCUMENT_ROOT'] . '/st500/inc/head.php');
require_once($_SERVER['DOCUMENT_ROOT'] . '/st500/lib/database.php');

## country list
$stmt = $pdo->prepare("SELECT idx, country_name FROM `country` ORDER BY country_name ASC");
$stmt->execute();
$country_data = $stmt->fetchAll(PDO::FETCH_ASSOC);
?>

<body data-theme="dark" data-layout="fluid">
  <div class="main">

    <!-- nav menu -->
    <?php include_once "inc/nav.php";?>

    <main class="content">
      <div class="container-fluid p-0">
        <!-- title -->
        <div class="row pt-2 mb-2">
          <div class="col-auto">
            <h2 class="text-primary">SETTING DEVICE</h2>
          </div>
        </div>

        <div class="row pt-2 mb-1">
          <div class="col-auto">
            <h3>LIST</h3>
          </div>

          <div class="col-auto ms-auto text-end mt-n1">
            <!-- refresh btn -->
            <button class="btn btn-sm btn-primary" id="reload">
              <!-- <i class="align-middle" data-feather="refresh-cw"></i> -->
                <svg xmlns="http://www.w3.org/2000/svg" width="24" height="24" viewBox="0 0 24 24" fill="none"
                    stroke="currentColor" stroke-width="2" stroke-linecap="round" stroke-linejoin="round"
                    class="feather feather-refresh-cw align-middle">
                    <polyline points="23 4 23 10 17 10"></polyline>
                    <polyline points="1 20 1 14 7 14"></polyline>
                    <path d="M3.51 9a9 9 0 0 1 14.85-3.36L23 10M1 14l4.64 4.36A9 9 0 0 0 20.49 15"></path>
                </svg> REFRESH
            </button>
          </div>
        </div>

        <!-- card -->
        <div class="card">
          <div class="card-body">
            <div class="row">
              <!-- country filter -->
              <div class="col-md-2 mb-3">
                <select class="form-select" id="searchByCountry">
                  <option value="">- Country -</option>
                  <?php foreach ($country_data as $row) { ?>
                  <option value="<?php echo ($row['idx']); ?>">
                    <?=$row['country_name']?>
                  </option>
                  <?php } ?>
                </select>
              </div>
              <!-- status filter -->
              <div class="col-md-6 mb-3">
                <!-- all -->
                <label class="form-check form-check-inline">
                  <input class="form-check-input" type="radio" name="searchBystate" value="A" checked />
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
            <table id="data_table" class="table table-striped table-bordered" style="width:100%">
              <thead>
                <tr>
                  <th>IDX</th>
                  <th>NO</th>
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

          <!-- modal -->
          <div class="modal fade" id="userModal" data-bs-backdrop="static" tabindex="-1" role="dialog"
            aria-hidden="true">
            <div class="modal-dialog modal-md" role="document">
              <form method="post" id="modal_form" enctype="multipart/form-data">

                <!-- modal content-->
                <div class="modal-content">
                  <div class="modal-header">
                    <h4 class="modal-title">EDIT DEVICE</h4>
                    <button type="button" class="btn-close" data-bs-dismiss="modal" aria-label="Close"></button>
                  </div>

                  <!-- modal body -->
                  <div class="modal-body m-0">
                    <div class="form-group">
                      <label>IDX</label>
                      <input type="text" class="form-control" id="data_idx" disabled>
                    </div>

                    <div class="form-group pt-3">
                      <label>DEVICE ID</label>
                      <input type="text" class="form-control" id="device_id" disabled>
                    </div>

                    <div class="form-group pt-3">
                      <label>COUNTRY</label>
                      <select name="country_idx" id="country_idx" class="form-select">
                        <?php foreach ($country_data as $row) { ?>
                        <option value="<?=$row['idx']?>">
                          <?=$row['country_name']?>
                        </option>
                        <?php } ?>
                      </select>
                    </div>

                    <div class="form-group pt-3">
                      <label>STATUS</label>
                      <select name="status" id="status" class="form-select">
                        <option value='Y'>Used</option>
                        <option value='N'>Unused</option>
                        <option value='D'>Delete</option>
                      </select>
                    </div>

                    <!-- modal footer-->
                    <div class="modal-footer">
                      <input type="hidden" name="idx" id="idx" />
                      <input type="hidden" name="operation" id="operation" />
                      <input type="submit" name="action" id="action" class="btn btn-success btn-md" value="Add" />
                      <button type="button" class="btn btn-secondary btn-md" data-bs-dismiss="modal">Close</button>
                    </div>
                  </div>
                </div>
              </form>
            </div>
          </div>
          <!-- END  modal -->

          <!-- footer -->
          <?php include_once($_SERVER['DOCUMENT_ROOT'] . '/st500/inc/footer.php'); ?>
        </div>
      </div>
    </main>
  </div>

  <!-- main JS -->
  <script src="assets/js/app.js"></script>
  <!-- Datatable ColReorder JS ver 1.5.4 -->
  <script src="assets/js/dataTables.colReorder.min.js"></script>

  <script>
  $(document).ready(function() {
    /* datatables */
    var dataTable = $('#data_table').DataTable({
      language: {
        'info': '_START_ to _END_. Total _TOTAL_.',
        'infoFiltered': '',
        'lengthMenu': 'Show _MENU_'
      },
      stateSave: true,
      // fixedHeader: true,
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
      searching: true, // Search box
      ajax: {
        url: 'proc/setting_device_fetch.php',
        data: function(data) {
          /* Read values */
          var searchCountry = $('#searchByCountry').val();
          var searchStatus = $('input:radio[name=searchBystate]:checked').val();

          /* Append to data */
          data.searchByCountry = searchCountry;
          data.searchBystate = searchStatus;
          console.log(data);
        },
      },
      columns: [{
          data: 'idx',
        },
        {
          data: 'no',
        },
        {
          data: 'device_id',
        },
        {
          data: 'country_name',
        },
        {
          data: 'name',
        },
        {
          data: 'status',
        },
        {
          data: 'reg_date',
        },
        {
          data: 'update_date',
        },
      ],
      /* 특정 조건에 따라 cell style 변경 */
      createdRow: function(row, data, dataIndex) {},
      /* column 조작 */
      columnDefs: [{
        targets: '_all',
        createdCell: function(td, cellData, rowData, rowIndex, colIndex) {
          $(td).html('<a href="javascript:;" class="status status_' + rowData.status +
            ' update" name="update" idx="' + rowData.idx +
            '" data-toggle="modal" data-target="#userModal" data-backdrop="static">' + cellData + '</a>'
          );
        }
      }, {
        targets: [1],
        orderable: false
      }],
    });
    dataTable.column(0).visible(false); // idx 항목 감추기

    /* search button */
    $('#searchByCountry').change(function() {
      dataTable.draw();
    });

    $("input[name='searchBystate']").change(function() {
      dataTable.draw();
    });

    /* edit modal */
    $(document).on('click', '.update', function() {
      var idx = $(this).attr("idx");
      $.ajax({
        url: "proc/setting_device_insert.php",
        method: "POST",
        data: {
          idx: idx
        },
        dataType: "json",
        success: function(data) {
          if (data.error) {
            alert(data.error);
          } else {
            $('#userModal').modal('show');
            $('#data_idx').val(idx);
            $('#idx').val(idx);
            $('#device_id').val(data.device_id);
            $('#country_idx').val(data.country_idx);
            $('#status').val(data.status);
            $('.modal-title').text("EDIT DEVICE");
            $('#action').val("Edit");
            $('#operation').val("Edit");
          }
        }
      })
    });
  });

  /* modal submit */
  $(document).on('submit', '#modal_form', function(event) {
    event.preventDefault();
    var country_idx = $('#country_idx').val();
    var status = $('#status').val();
    var remark = $('#remark').val();
    if (status != '') {
      $.ajax({
        url: "proc/setting_device_insert.php",
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
            $('#userModal').modal('hide');
            location.reload(); // submit 이후 datatables 새로고침
          }
        }
      });
    } else {
      alert("Missing required items.");
    }
  });

  /* reload 버튼 누르면 datatables 초기화 후 새로고침 */
  $("#reload").on("click", function() {
    $('#data_table').DataTable().state.clear();
    location.reload();
  });
  </script>

</body>

</html>