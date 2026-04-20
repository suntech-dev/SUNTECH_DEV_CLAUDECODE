<?php
## ST-500 Renewal (2022.02.23 Start. dev@suntech.asia & hamani@naver.com)

require_once($_SERVER['DOCUMENT_ROOT'] . '/st500/inc/head.php');
require_once($_SERVER['DOCUMENT_ROOT'] . '/st500/lib/database.php');

## code list
$stmt = $pdo->prepare("SELECT DISTINCT(code) FROM `st500_logs` ORDER BY code ASC");
$stmt->execute();
$code_data = $stmt->fetchAll(PDO::FETCH_ASSOC);
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
            <h2 class="text-primary">API LOG</h2>
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
              <!-- code filter -->
              <div class="col-md-2 mb-3">
                <select class="form-select" id="searchByCode">
                  <option value="">- All Code -</option>
                  <option value="get_device">디바이스 조회</option>
                  <option value="send_device">디바이스 등록</option>

                  <!-- <?php foreach ($code_data as $row) { ?>
                  <option value="<?php echo ($row['code']); ?>">
                    <?=$row['code']?>
                  </option>
                  <?php } ?> -->
                </select>
              </div>

              <!-- date filter -->
              <!-- <div class="col-md-2 mb-3">
                <div class="input-group">
                  <span class="input-group-text"><i class="xi-calendar-check xi-x"></i></span>
                  <input class="form-control input-datepicker form-control daterange input-daterange-datepicker"
                    type="text" name="period" id="period" value="" autocomplete="off" placeholder="select period" />
                  <input type="text" name="oneday" id="oneday" class="form-control" style="display:none" maxlength="10"
                    readonly="readonly" autocomplete="off" />
                </div>
              </div>
            </div> -->

            <!-- datatables -->
            <table id="data_table" class="table table-striped table-bordered" style="width:100%">
              <thead>
                <tr>
                  <th>IDX</th>
                  <th>NO</th>
                  <th>CODE</th>
                  <th>REQUEST DATA</th>
                  <th>RESPONSE DATA</th>
                  <!-- <th>REG IP</th> -->
                  <th>REG DATE</th>
                </tr>
              </thead>
            </table>
          </div>

          <!-- modal -->
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
        url: 'proc/log_fetch.php',
        data: function(data) {
          /* Read values */
          var searchCode = $('#searchByCode').val();
          var searchPeriod = $('#searchByPeriod').val();

          /* Append to data */
          data.searchByCode = searchCode;
          data.searchByPeriod = searchPeriod;
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
          data: 'code',
        },
        {
          data: 'log_data',
        },
        {
          data: 'log_result',
        },
        /* {
          data: 'reg_ip',
        }, */
        {
          data: 'reg_date',
        },
      ],
      /* 특정 조건에 따라 cell style 변경 */
      createdRow: function(row, data, dataIndex) {},
      /* column 조작 */
      columnDefs: [{
        /* targets: '_all',
        createdCell: function(td, cellData, rowData, rowIndex, colIndex) {
          $(td).html('<a href="javascript:;" class="status status_' + rowData.status +
            ' update" name="update" idx="' + rowData.idx +
            '" data-toggle="modal" data-target="#userModal" data-backdrop="static">' + cellData + '</a>'
          );
        } */
      }, {
        targets: [1],
        orderable: false
      }],
    });
    dataTable.column(0).visible(false); // idx 항목 감추기

    /* search button */
    $('#searchByCode').change(function() {
      dataTable.draw();
    });

    $('#searchByPeriod').change(function() {
      dataTable.draw();
    });
  });

  /* reload 버튼 누르면 datatables 초기화 후 새로고침 */
  $("#reload").on("click", function() {
    $('#data_table').DataTable().state.clear();
    location.reload();
  });
  </script>

</body>

</html>