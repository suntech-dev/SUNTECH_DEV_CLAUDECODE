<?php
## ST-500 Renewal (2022.02.23 Start. dev@suntech.asia & hamani@naver.com)

## 업로드할 텍스트 파일은 반드시 UTF-8 로 인코딩하세요.
## JSON 파일 포맷에 오류가 있으면 들어가다가 실패합니다.

require_once($_SERVER['DOCUMENT_ROOT'] . '/st500/inc/head.php');
require_once($_SERVER['DOCUMENT_ROOT'] . '/st500/lib/database.php');

## last upload date
$stmt = $pdo->prepare("SELECT update_date FROM `st500_parameter` limit 1");
$stmt->execute();
$data = $stmt->fetch(PDO::FETCH_ASSOC);
$last_upload_data = $data['update_date'];
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
            <h2 class="text-primary">PARAMETER</h2>
          </div>
        </div>

        <div class="row pt-2 mb-2">
          <div class="col-auto">
            <h3>UPLOAD</h3>
          </div>

          <div class="col-auto ms-auto text-end mt-n1">
            <!-- refresh btn -->
            <button class="btn btn-sm btn-primary shadow-sm me-1" id="reload">
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

        <!-- parameter upload card -->
        <div class="card">
          <div class="card-body">

            <form method="post" id="modal_form_2" name="modal_form_2" class="row row-cols-md-auto align-items-center"
              enctype="multipart/form-data">

              <div class="col-12">
                <div class="input-group mb-2 mr-sm-2">
                  <input type="file" name="parameterFile" id="parameterFile" class="form-control" />
                </div>
              </div>

              <div class="col-12">
                <button type="submit" name="action2" id="action2" class="btn btn-danger mb-2"
                  value="Add2">UPLOAD</button>
              </div>
            <div class="col-12">
              <p class="mb-0">LAST UPLOAD : <?=$last_upload_data?></p>
            </div>
            </form>
          </div>
        </div>

        <div class="row pt-2 mb-3">
        </div>

        <div class="row pt-2 mb-1">
          <div class="col-auto">
            <h3>LIST</h3>
          </div>
        </div>

        <!-- parameter list card -->
        <!-- <div class="card"> -->
        <!-- <div class="card-body"> -->


        <div class="row">
          <!-- status filter -->
          <div class="col-md-6 mb-3">
            <!-- all -->
            <label class="form-check form-check-inline">
              <input class="form-check-input" type="radio" name="searchByCountry" value="K" checked />
              <span class="form-check-label">Korean</span>
            </label>
            <!-- used -->
            <label class="form-check form-check-inline">
              <input class="form-check-input" type="radio" name="searchByCountry" value="E" />
              <span class="form-check-label">English</span>
            </label>
            <!-- unused -->
            <label class="form-check form-check-inline">
              <input class="form-check-input" type="radio" name="searchByCountry" value="I" />
              <span class="form-check-label">Indonesian</span>
            </label>
          </div>
        </div>
        <!-- datatables -->
        <table id="data_table" class="table table-striped table-bordered" style="width:100%">
          <thead>
            <tr>
              <th>IDX</th>
              <th>COUNTRY</th>
              <th>NUM</th>
              <th>TEXT</th>
              <!-- <th>UPDATE DATE</th> -->
            </tr>
          </thead>
        </table>
        <!-- </div> -->

        <!-- modal -->
        <!-- END  modal -->

        <!-- footer -->
        <?php include_once($_SERVER['DOCUMENT_ROOT'] . '/st500/inc/footer.php'); ?>
        <!-- </div> -->
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
        url: 'proc/parameter_fetch.php',
        data: function(data) {
          /* Read values */
          var searchCountry = $('input:radio[name=searchByCountry]:checked').val();

          /* Append to data */
          data.searchByCountry = searchCountry;
          console.log(data);
        },
      },
      columns: [{
          data: 'idx',
        },
        {
          data: 'country',
        },
        {
          data: 'num',
        },
        {
          data: 'text',
        },
        /* {
          data: 'update_date',
        }, */
      ],
      /* 특정 조건에 따라 cell style 변경 */
      createdRow: function(row, data, dataIndex) {},
      /* column 조작 */
      columnDefs: [{
        /* targets: '_all',
        createdCell: function(td, cellData, rowData, rowIndex, colIndex) {
          var txt = cellData;
          if (colIndex == 3) {
            txt = "<span style='display:inline-block; width:50%; height:15px; background-color:" +
              cellData + "'></span>";
          }
          $(td).html('<a href="javascript:;" class="status status_' + rowData.status +
            ' update" name="update" idx="' + rowData.idx +
            '" data-toggle="modal" data-target="#userModal" data-backdrop="static">' + txt + '</a>');
        } */
      }, {
        targets: [3],
        orderable: false
      }],
    });
    dataTable.column(0).visible(false); // idx 항목 감추기
    dataTable.column(1).visible(false);

    /* search button */
    $("input[name='searchByCountry']").change(function() {
      dataTable.draw();
    });
  });

  /* modal submit */
  $(document).on('submit', '#modal_form_2', function(event) {
    event.preventDefault();
    $.ajax({
      url: "proc/parameter_insert.php",
      method: 'POST',
      data: new FormData(this),
      contentType: false,
      processData: false,
      dataType: 'json',
      success: function(data) {
        if (data.error) {
          alert(data.error);
        } else {
          alert(data.msg);
          $('#modal_form_2')[0].reset();
          $('#data_table').DataTable().state.clear();
          location.reload();
        }
      }
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