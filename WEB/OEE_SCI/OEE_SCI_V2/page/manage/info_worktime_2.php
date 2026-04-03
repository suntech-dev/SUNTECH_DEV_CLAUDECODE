<?php

/**
 * info_worktime_2.php — 근무 시간(Work Time) 관리 페이지
 *
 * 공장/라인별 근무 시간 설정 및 관리 페이지 (레거시 jQuery/DataTables UI)
 *
 * 다른 관리 페이지와의 차이점:
 *   - worktime_head.php 사용 (head.php와 다름: jQuery + DataTables + daterangepicker 포함)
 *   - ResourceManager 미사용: jQuery DataTables 서버사이드 렌더링
 *   - Bootstrap 스타일 모달 (#userModal): Fiori 스타일 모달 아님
 *   - ES Module 미사용: 인라인 <script> 방식
 *   - PHP 인라인 PDO 쿼리: factory_data + line_data 직접 조회
 *   - 인라인 CSS: 캘린더/시간 입력/타임피커 등 레거시 스타일
 *
 * 레이아웃:
 *   .container.wt-body → .card → .row
 *     왼쪽(58%): #monthDiv — 월 캘린더 (Ajax로 동적 로드)
 *     오른쪽(42%): #time_list — DataTables 목록 (#data_table)
 *
 * 3가지 근무 시간 수정 모드:
 *   kind=1 / .update1, .updatePeriod : Period 수정 (기간별 + 요일 지정 없음)
 *   kind=2 / .update2, .updateWeek   : Week 수정 (기간별 + 요일 지정 있음)
 *   kind=3 / .update3, .updateDay    : Day 수정 (특정 날짜 단일 수정)
 *
 * 모달 폼 구성:
 *   - factory_idx + line_idx: 공장/라인 선택
 *   - period (daterangepicker) / oneday (단일 날짜, 읽기 전용): kind에 따라 교체 표시
 *   - dayOfTheWeek: 요일 체크박스 (Sun~Sat, week[0]~week[6])
 *   - SHIFT-1/2/3: available_stime/etime + planned1~5_stime/etime + over_time
 *
 * 시간 입력 형식: HH:MM (99:99 마스크)
 *   - initSmartTimeFormatting(): 숫자 4자리 자동 포맷 + blur 유효성 검사
 *
 * 전역 함수 (window.*):
 *   - window.prevMonth(): 이전 달 캘린더 로드
 *   - window.nextMonth(): 다음 달 캘린더 로드
 *   - window.thisMonth(): 현재 달 캘린더 로드 (PHP date() 기준)
 *
 * API 연동:
 *   - proc/set_work_time_fetch.php  : DataTables 서버사이드 (searchByFactory/Line/kind)
 *   - proc/set_work_time_fetch_month.php: 월 캘린더 HTML 반환
 *   - proc/set_work_time_insert.php : 수정 데이터 로드 (GET, idx/oneday 파라미터)
 *                                    + 폼 제출 (POST, operation=Add/Edit/AddW/EditW/AddD/EditD)
 *   - proc/ajax_factory_line.php    : 공장 선택 시 라인 목록 HTML 반환
 */

// worktime_head.php 유지 (jQuery DataTables + daterangepicker — 별도 보존)
require_once(__DIR__ . '/../../inc/worktime_head.php');
require_once(__DIR__ . '/../../lib/config.php');
require_once(__DIR__ . '/../../lib/db.php');
// nav-fiori.php 제거

## 탭메뉴용 factory list
// 사용 중인(status='Y') 공장 목록 조회 → 헤더 searchByFactory + 모달 factory_idx 드롭다운에 사용
$stmt = $pdo->prepare("SELECT idx, factory_name FROM `info_factory` WHERE status = 'Y' ORDER BY factory_name ASC");
$stmt->execute();
$factory_data = $stmt->fetchAll(PDO::FETCH_ASSOC);

## 탭메뉴용 Line list
// factory_data의 첫 번째 공장에 해당하는 라인 목록 조회
// 라인이 1개 이상인 첫 번째 공장의 라인 데이터만 가져옴 (break로 첫 매칭 공장에서 중단)
foreach ($factory_data as $key => $val) {
    $stmt = $pdo->prepare("SELECT idx, line_name FROM `info_line` WHERE status = 'Y' AND factory_idx = ? ORDER BY line_name ASC");
    $stmt->execute([$val['idx']]);

    $line_data = $stmt->fetchAll(PDO::FETCH_ASSOC);

    // 라인이 1개 이상인 공장을 찾으면 즉시 중단 (첫 번째 공장의 첫 번째 라인을 초기값으로 사용)
    if ($stmt->rowCount() > 0) {
        break;
    }
}
?>

<!-- info_worktime_2.css: 근무 시간 관리 페이지 전용 스타일 -->
<link rel="stylesheet" href="css/info_worktime_2.css">

<style type="text/css">
    /* 날짜 입력 피커: 작은 폰트 크기 */
    .input-datepicker {
        font-size: .825rem;
    }

    /* 드롭다운 메뉴: 다크 테마 배경색 */
    .dropdown-menu {
        background-color: #2f3546;
    }

    .custom-checkbox {
        margin-right: 1rem;
    }

    /* #monthDiv 내부 테이블: 셀 간격 없음 */
    #monthDiv table {
        border-collapse: collapse;
    }

    /* #calendar: 월 캘린더 전체 너비 */
    #calendar {
        width: 100%;
        font-size: 0.87rem;
    }

    /* 캘린더 헤더 (요일 행): 중앙 정렬, 보더, 회색 텍스트 */
    #calendar thead th {
        padding: 4px;
        text-align: center;
        border: 1px solid #888;
        color: #a9acb3;
    }

    /* 캘린더 날짜 셀: 높이 95px, 우측 상단 정렬, 상대 위치 */
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

    /* 캘린더 초과 근무 텍스트: 연한 빨간색 */
    #calendar tbody td .overtime {
        color: #ad7c7c
    }

    /* 날짜 숫자: 우측 정렬, 하단 마진 */
    #calendar .day_name {
        text-align: right;
        margin-bottom: 8px;
        color: #ddd;
    }

    /* 날짜 셀 호버: 배경색 강조 */
    #calendar .day_cell:hover {
        background-color: #232732;
    }

    /* day_kind: 근무 종류 레이블 (절대 위치, 왼쪽 상단) */
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

    /* today: 오늘 날짜 셀 배경 강조 */
    #calendar .today {
        background-color: #3f444a;
    }

    /* input-time is-invalid: 시간 입력 오류 스타일 (빨간 테두리 + 음영) */
    .form-control.input-time.is-invalid {
        border-color: #dc3545;
        box-shadow: 0 0 0 0.2rem rgba(220, 53, 69, 0.25);
        background-color: rgba(220, 53, 69, 0.1);
    }

    .form-control.input-time.is-invalid:focus {
        border-color: #dc3545;
        box-shadow: 0 0 0 0.2rem rgba(220, 53, 69, 0.25);
    }

    /* input-time 유효: 포커스 시 초록 테두리 */
    .form-control.input-time:not(.is-invalid):focus {
        border-color: #198754;
        box-shadow: 0 0 0 0.2rem rgba(25, 135, 84, 0.25);
    }

    /* fc-toolbar: FullCalendar 스타일 오버라이드 (사용 중 여부 확인 필요) */
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

    /* .card .row: float 레이아웃 (캘린더 58% + 목록 42%) */
    .card .row {
        overflow: hidden;
        width: 100%;
    }

    .card .row>div {
        float: left;
    }

    /* 캘린더 영역: 58% 너비 */
    .card .row>div:first-child {
        width: 58%;
    }

    /* 목록 영역: 42% 너비, 좌측 패딩 */
    .card .row>div:last-child {
        width: 42%;
        padding-left: 22px;
    }

    /* 목록 제목 스타일 */
    #time_list h3 {
        font-size: 1.2375rem;
        color: #fff;
        font-weight: 500;
        line-height: 1.2;
        margin-bottom: 0.5rem;
        margin-top: 0;
    }
</style>

<?php
// 네비게이션 드로어: worktime 메뉴 항목을 active 상태로 표시
$nav_active = 'worktime';
require_once(__DIR__ . '/../../inc/nav-drawer-manage.php');
?>

<!-- Signage Header: 상단 네비게이션 바 -->
<div class="signage-header">
    <!-- 햄버거 버튼: 좌측 네비게이션 드로어 열기/닫기 -->
    <button id="navDrawerBtn" class="nav-drawer-btn" aria-label="Menu">&#9776;</button>
    <span class="signage-header__title">Work Time Management</span>
    <div class="signage-header__filters">
        <!-- reload: 클릭 시 location.reload() → 전체 페이지 새로고침 -->
        <button class="btn btn-sm btn-primary me-1" id="reload">&#8635; REFRESH</button>
        <!-- add_button2: 클릭 시 신규 근무 시간 추가 모달 열기 (operation=AddW) -->
        <button class="btn btn-sm btn-danger addData" id="add_button2">+ ADD WORK TIME</button>
    </div>
</div>

<!-- 메인 컨테이너: 캘린더(좌) + 목록(우) 2컬럼 레이아웃 -->
<div class="container wt-body">

    <div class="card">
        <div class="card-body">
            <div class="row">
                <!-- 월 캘린더 영역 (58%): Ajax로 동적 로드 -->
                <!-- getMonth() 호출 → POST proc/set_work_time_fetch_month.php → HTML 렌더링 -->
                <!-- 캘린더 날짜 클릭 → .update3/.updateDay 이벤트 → kind=3 수정 모달 열기 -->
                <div class="col-6" id="monthDiv"></div>

                <!-- 근무 시간 목록 영역 (42%): DataTables 서버사이드 렌더링 -->
                <div class="col-4" id="time_list">
                    <div>
                        <h3>WORK TIME LIST</h3>
                    </div>

                    <!-- 공장/라인 필터 드롭다운: 변경 시 dataTable.draw() + getMonth() 재호출 -->
                    <ul class="quick-update">
                        <li class="w-180 pad-r-20">
                            <!-- searchByFactory: PHP에서 공장 목록 렌더링, change → Ajax 라인 목록 갱신 -->
                            <select class="form-select" id="searchByFactory">
                                <?php foreach ($factory_data as $row) { ?>
                                    <option value="<?= $row['idx'] ?>">
                                        <?= $row['factory_name'] ?>
                                    </option>
                                <?php } ?>
                            </select>
                        </li>
                        <li class="w-180 pad-r-20">
                            <!-- searchByLine: 초기값은 첫 번째 공장의 라인 목록 -->
                            <!-- searchByFactory change → Ajax → html() 갱신 -->
                            <select class="form-select" id="searchByLine">
                                <?php foreach ($line_data as $row) { ?>
                                    <option value="<?= $row['idx'] ?>">
                                        <?= $row['line_name'] ?>
                                    </option>
                                <?php } ?>
                            </select>
                        </li>
                    </ul>

                    <!-- data_table: DataTables 서버사이드 렌더링 -->
                    <!-- 컬럼: IDX(숨김) / NO. / PERIOD / AREA / DAY of THE WEEK / STATUS / TYPE -->
                    <!-- createdCell: 모든 셀을 <a> 링크로 래핑 (.status, .update{kind}) -->
                    <!-- .update1/.updatePeriod: Period 수정 모달, .update2/.updateWeek: Week 수정 모달 -->
                    <!-- .update3/.updateDay: Day 수정 모달 (캘린더 날짜 클릭과 동일 경로) -->
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


<!-- Bootstrap 스타일 모달: 근무 시간 추가/수정 다이얼로그 -->
<!-- showDialogWindow(900, 750, false): CSS position 방식으로 표시 (Bootstrap 방식과 다름) -->
<!-- #modal_form submit → proc/set_work_time_insert.php POST (operation 값에 따라 처리) -->
<div id="userModal" tabindex="-1">
    <div class="modal-dialog">
        <!-- modal_form: enctype="multipart/form-data" (파일 업로드 없지만 FormData 사용) -->
        <form method="post" id="modal_form" enctype="multipart/form-data">
            <!-- modal content-->
            <div class="modal-content">
                <div class="modal-header">
                    <!-- modal-title: 수정 모드에 따라 동적으로 변경 -->
                    <!-- ADD/EDIT + by PERIOD/Day Of The Week/Day -->
                    <h4 class="modal-title">ADD WORK TIME by PERIOD</h4>
                </div>

                <!-- modal body: 공장/라인/기간/요일 설정 영역 -->
                <div class="modal-body">
                    <!-- IDX 표시 (비활성화): 현재 수정 중인 레코드 인덱스 확인용 -->
                    <div class="form-group">
                        <div class="row">
                            <div class="row-head">
                                <label>IDX</label>
                            </div>
                            <div class="row-data">
                                <div class="col col-4">
                                    <!-- data_idx: 표시용 (비활성화), idx: 실제 전송값 -->
                                    <input type="text" class="form-control" name="data_idx" id="data_idx" disabled />
                                </div>
                            </div>
                        </div>
                    </div>

                    <!-- 공장/라인 선택: factory_idx(-1=미선택, ''=전체, idx=특정 공장) -->
                    <!-- factory_idx change → Ajax → ajax_factory_line.php → line_idx 드롭다운 갱신 -->
                    <div class="form-group">
                        <div class="row">
                            <div class="row-head">
                                <label>FACTORY / LINE</label>
                            </div>
                            <div class="row-data">
                                <div class="col col-4">
                                    <!-- factory_idx: -1=미선택(폼 제출 차단), ''=전체, 실제 idx=특정 공장 -->
                                    <select name="factory_idx" id="factory_idx" class="form-select">
                                        <option value="-1">Select Factory</option>
                                        <option value="">All Factory</option>
                                        <?php foreach ($factory_data as $row) { ?>
                                            <option value="<?= $row['idx'] ?>">
                                                <?= $row['factory_name'] ?>
                                            </option>
                                        <?php } ?>
                                    </select>
                                </div>
                                <div class="col col-4">
                                    <!-- line_idx: -1=미선택(폼 제출 차단), 0=전체, 실제 idx=특정 라인 -->
                                    <!-- Ajax로 동적 채워짐 (ajax_factory_line.php) -->
                                    <select name="line_idx" id="line_idx" class="form-select">
                                        <option value="-1">Select Line</option>
                                        <option value="0">All Line</option>
                                    </select>
                                </div>
                            </div>
                        </div>
                    </div>

                    <!-- 비고 입력 -->
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

                    <!-- 사용 여부 선택 -->
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

                <!-- 기간/날짜/요일/Shift 입력 영역 -->
                <div class="modal-body div-line">
                    <!-- 기간/날짜 입력: kind에 따라 period(daterangepicker) / oneday 교체 표시 -->
                    <div class="form-group">
                        <div class="row">
                            <div class="row-head">
                                <!-- period_name: kind=3(Day) 수정 시 'DATE'로 변경, 나머지는 'PERIOD' -->
                                <label id="period_name">PERIOD</label>
                            </div>
                            <div class="row-data">
                                <div class="col col-3">
                                    <!-- period: daterangepicker 적용 (YYYY-MM-DD ~ YYYY-MM-DD 형식) -->
                                    <!-- kind=3(Day) 수정 시 hide(), oneday show() -->
                                    <input type="text"
                                        class="form-control input-datepicker daterange input-daterange-datepicker"
                                        name="period" id="period" autocomplete="off" placeholder="select period" />
                                </div>
                                <div class="col col-4">
                                    <!-- oneday: kind=3(Day) 수정 시 표시 (readonly, 캘린더 클릭값) -->
                                    <!-- 기본 hidden, add_button2/update1/update2 클릭 시 hide() -->
                                    <input type="text" class="form-control" name="oneday" id="oneday"
                                        style="display:none" maxlength="10" readonly="readonly" autocomplete="off" />
                                </div>
                            </div>
                        </div>
                    </div>

                    <!-- 요일 선택 체크박스: week[0]~week[6] (일~토) -->
                    <!-- kind=1(Period) 수정 시 hide(), kind=2(Week)/신규 시 show() -->
                    <!-- 7자리 문자열 week_yn의 각 문자(0/1)로 체크 상태 복원: week_yn.substr(i,1)=='1' -->
                    <div class="form-group" id="dayOfTheWeek">
                        <div class="row">
                            <div class="row-head">
                                <label>DAY of THE WEEK</label>
                            </div>
                            <div class="row-data">
                                <div class="col" style="margin-top: 0.6rem;">
                                    <!-- week[0]: 일요일 -->
                                    <label class="form-check form-check-inline">
                                        <input class="form-check-input" type="checkbox" name="week[0]" value="Y" />
                                        <span class="form-check-label">Sun</span>
                                    </label>
                                    <!-- week[1]: 월요일 -->
                                    <label class="form-check form-check-inline">
                                        <input class="form-check-input" type="checkbox" name="week[1]" value="Y" />
                                        <span class="form-check-label">Mon</span>
                                    </label>
                                    <!-- week[2]: 화요일 -->
                                    <label class="form-check form-check-inline">
                                        <input class="form-check-input" type="checkbox" name="week[2]" value="Y" />
                                        <span class="form-check-label">Tues</span>
                                    </label>
                                    <!-- week[3]: 수요일 -->
                                    <label class="form-check form-check-inline">
                                        <input class="form-check-input" type="checkbox" name="week[3]" value="Y" />
                                        <span class="form-check-label">Wednes</span>
                                    </label>
                                    <!-- week[4]: 목요일 -->
                                    <label class="form-check form-check-inline">
                                        <input class="form-check-input" type="checkbox" name="week[4]" value="Y" />
                                        <span class="form-check-label">Thurs</span>
                                    </label>
                                    <!-- week[5]: 금요일 -->
                                    <label class="form-check form-check-inline">
                                        <input class="form-check-input" type="checkbox" name="week[5]" value="Y" />
                                        <span class="form-check-label">Fri</span>
                                    </label>
                                    <!-- week[6]: 토요일 -->
                                    <label class="form-check form-check-inline">
                                        <input class="form-check-input" type="checkbox" name="week[6]" value="Y" />
                                        <span class="form-check-label">Sat</span>
                                    </label>
                                </div>
                            </div>
                        </div>
                    </div>

                    <!-- SHIFT 시간 입력 영역: SHIFT-1/2/3 각각 동일한 구조 반복 -->
                    <!-- input-time[data-mask="99:99"]: initSmartTimeFormatting()에서 자동 포맷 처리 -->
                    <!-- available_stime/etime[N]: N번 쉬프트 가용 시작/종료 시간 -->
                    <!-- planned1~5_stime/etime[N]: N번 쉬프트 계획 중단 시간 ①~⑤ -->
                    <!-- over_time[N]: N번 쉬프트 초과 근무 시간 (분 단위, 최대 3자리) -->
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
                                    <!-- Planned Time ①~⑤: 계획된 중단 시간 (점심, 휴식 등) -->
                                    <label class="white">Planned Time ① ~ ⑤</label>
                                </div>
                            </div>
                        </div>
                        <!-- SHIFT-1: 1번 근무 쉬프트 시간 설정 -->
                        <div class="row">
                            <div class="row-head">
                                <label>SHIFT - 1</label>
                            </div>
                            <div class="row-data">
                                <!-- 가용 시간: 쉬프트 시작~종료 전체 가용 시간 -->
                                <div class="col col-4 pad-top-4">
                                    <input type="text" class="form-control input-time" name="available_stime[1]"
                                        data-mask="99:99" maxlength="5" />
                                    <div class="div-input-time">~</div>
                                    <input type="text" class="form-control input-time" name="available_etime[1]"
                                        data-mask="99:99" maxlength="5" />
                                </div>
                                <!-- 계획 중단 ①: 첫 번째 계획 중단 시간 -->
                                <div class="col col-4 pad-top-4">
                                    <input type="text" class="form-control input-time" name="planned1_stime[1]"
                                        data-mask="99:99" maxlength="5" placeholder="①" />
                                    <div class="div-input-time">~</div>
                                    <input type="text" class="form-control input-time" name="planned1_etime[1]"
                                        data-mask="99:99" maxlength="5" />
                                </div>
                                <!-- 계획 중단 ②: 두 번째 계획 중단 시간 -->
                                <div class="col col-4 pad-top-4">
                                    <input type="text" class="form-control input-time" name="planned2_stime[1]"
                                        data-mask="99:99" maxlength="5" placeholder="②" />
                                    <div class="div-input-time">~</div>
                                    <input type="text" class="form-control input-time" name="planned2_etime[1]"
                                        data-mask="99:99" maxlength="5" />
                                </div>
                                <!-- 계획 중단 ③: 세 번째 계획 중단 시간 -->
                                <div class="col col-4 pad-top-4">
                                    <input type="text" class="form-control input-time" name="planned3_stime[1]"
                                        data-mask="99:99" maxlength="5" placeholder="③" />
                                    <div class="div-input-time">~</div>
                                    <input type="text" class="form-control input-time" name="planned3_etime[1]"
                                        data-mask="99:99" maxlength="5" />
                                </div>
                                <!-- 빈 셀: 레이아웃 정렬용 공백 -->
                                <div class="col col-4 pad-top-4">&nbsp;
                                </div>
                                <!-- 계획 중단 ④: 네 번째 계획 중단 시간 -->
                                <div class="col col-4 pad-top-4">
                                    <input type="text" class="form-control input-time" name="planned4_stime[1]"
                                        data-mask="99:99" maxlength="5" placeholder="④" />
                                    <div class="div-input-time">~</div>
                                    <input type="text" class="form-control input-time" name="planned4_etime[1]"
                                        data-mask="99:99" maxlength="5" />
                                </div>
                                <!-- 계획 중단 ⑤: 다섯 번째 계획 중단 시간 -->
                                <div class="col col-4 pad-top-4">
                                    <input type="text" class="form-control input-time" name="planned5_stime[1]"
                                        data-mask="99:99" maxlength="5" placeholder="⑤" />
                                    <div class="div-input-time">~</div>
                                    <input type="text" class="form-control input-time" name="planned5_etime[1]"
                                        data-mask="99:99" maxlength="5" />
                                </div>
                                <!-- 초과 근무: 분 단위 (최대 999분) -->
                                <div class="col col-4 pad-top-4">
                                    <span class="over-time">Over Time</span>
                                    <input type="text" class="form-control overtime" name="over_time[1]" data-mask="999"
                                        maxlength="3" />
                                    <span class="over-time">(Min)</span>
                                </div>
                            </div>
                        </div>
                    </div>

                    <!-- SHIFT-2: 2번 근무 쉬프트 (SHIFT-1과 동일한 구조, index=[2]) -->
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

                    <!-- SHIFT-3: 3번 근무 쉬프트 (SHIFT-1과 동일한 구조, index=[3]) -->
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

                <!-- 모달 푸터: 절대 위치, 하단 고정 -->
                <!-- idx: 실제 폼 전송용 레코드 PK (data_idx는 표시용) -->
                <!-- operation: Add/Edit/AddW/EditW/AddD/EditD → proc/set_work_time_insert.php에서 분기 처리 -->
                <!-- action(submit input): 'Add' 또는 'Edit' 값 표시 -->
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
    // PHP에서 현재 연/월 초기값 설정 (캘린더 초기 표시 기준)
    var year = <?= date('Y') ?>;
    var month = <?= date('n') ?>;

    /**
     * waitForJQuery — jQuery 로드 완료 대기 함수
     * @param {Function} callback - jQuery 로드 후 실행할 콜백
     * 50ms 간격으로 재귀 호출하며 jQuery 로드 여부 확인
     * worktime_head.php에서 jQuery를 async로 로드하므로 필요
     */
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
        $(document).ready(function() {

            /**
             * initSmartTimeFormatting — 시간 입력 자동 포맷팅 초기화
             * .form-control.input-time[data-mask="99:99"] 요소에 적용
             *
             * 이벤트:
             *   - input.smartTime: 숫자 4자리 → HH:MM 자동 포맷
             *     - 1자리: 그대로
             *     - 2자리: 그대로
             *     - 3자리: XX:X 형식
             *     - 4자리 이상: HH:MM (유효한 시간이면 포맷, 아니면 무시)
             *   - keypress.smartTime: 숫자/콜론 외 입력 차단
             *   - paste.smartTime: 붙여넣기 후 10ms 지연 → input 이벤트 재트리거
             *   - blur.smartTime: HH:MM 유효성 검사 → is-invalid 클래스 토글
             *     - 유효: HH:MM 제로패딩 정규화 + is-invalid 제거
             *     - 무효: is-invalid 추가 + title 속성으로 오류 메시지
             *
             * .off('.smartTime'): 재초기화 시 기존 이벤트 핸들러 중복 등록 방지
             */
            function initSmartTimeFormatting() {
                $('.form-control.input-time[data-mask="99:99"]').each(function() {
                    const $input = $(this);

                    // 기존 이벤트 핸들러 제거 (중복 등록 방지)
                    $input.off('.smartTime');

                    // 입력 이벤트: 숫자 4자리 → HH:MM 자동 포맷
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
                            // 3자리: XX:X
                            formattedValue = digitsOnly.charAt(0) + digitsOnly.charAt(1) + ':' + digitsOnly.charAt(2);
                        } else if (digitsOnly.length >= 4) {
                            // 4자리 이상: HH:MM 포맷 + 범위 검사
                            const hours = digitsOnly.charAt(0) + digitsOnly.charAt(1);
                            const minutes = digitsOnly.charAt(2) + digitsOnly.charAt(3);

                            const hourNum = parseInt(hours);
                            const minuteNum = parseInt(minutes);

                            if (hourNum <= 23 && minuteNum <= 59) {
                                formattedValue = hours + ':' + minutes;
                            } else {
                                // 유효하지 않은 시간이면 포맷 변경 없이 그대로 유지
                                return;
                            }
                        }

                        if (formattedValue !== rawValue) {
                            this.value = formattedValue;

                            // 커서 위치 복원: ':' 삽입으로 인한 커서 이동 보정
                            let newCursorPos = formattedValue.length;
                            if (formattedValue.length > 2 && cursorPos <= 2) {
                                newCursorPos = cursorPos;
                            }
                            this.setSelectionRange(newCursorPos, newCursorPos);
                        }
                    });

                    // 키입력 이벤트: 숫자/콜론 외 입력 차단
                    $input.on('keypress.smartTime', function(e) {
                        const char = String.fromCharCode(e.which);
                        if (!/[0-9:]/.test(char) && !e.ctrlKey && !e.metaKey) {
                            e.preventDefault();
                        }
                    });

                    // 붙여넣기 이벤트: 10ms 지연 후 input 이벤트 재트리거 (포맷 적용)
                    $input.on('paste.smartTime', function(e) {
                        setTimeout(() => {
                            $(this).trigger('input.smartTime');
                        }, 10);
                    });

                    // 포커스 아웃 이벤트: HH:MM 유효성 검사
                    $input.on('blur.smartTime', function() {
                        const value = this.value;

                        // 빈 값: 오류 클래스 제거 (선택 입력이므로)
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
                                    // 유효: HH:MM 제로패딩 정규화 (예: '9:5' → '09:05')
                                    this.value = String(hour).padStart(2, '0') + ':' + String(minute).padStart(2, '0');
                                    $(this).removeClass('is-invalid');
                                    $(this).removeAttr('title');
                                } else {
                                    // 무효: 범위 초과 (00:00 ~ 23:59)
                                    $(this).addClass('is-invalid');
                                    $(this).attr('title', 'Invalid time (00:00 ~ 23:59)');
                                }
                            } else {
                                // 무효: ':' 2개 이상 등 잘못된 형식
                                $(this).addClass('is-invalid');
                                $(this).attr('title', 'Invalid time format (HH:MM)');
                            }
                        } else {
                            // ':' 없이 숫자만 입력된 경우
                            const digitsOnly = value.replace(/[^0-9]/g, '');
                            if (digitsOnly.length > 0) {
                                $(this).addClass('is-invalid');
                                $(this).attr('title', 'Invalid format. e.g. 1111 => 11:11');
                            }
                        }
                    });
                });
            }

            // 초기 시간 포맷팅 적용
            initSmartTimeFormatting();

            // 모달 열릴 때마다 시간 포맷팅 재초기화 (동적으로 추가된 input에 적용)
            $(document).on('shown.bs.modal', '#userModal', function() {
                initSmartTimeFormatting();
            });

            // daterangepicker 초기화: YYYY-MM-DD 형식, 한국어 레이블 설정
            // period 입력 요소에 적용 (수정 모달 열기 시 startDate/endDate 재설정)
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

            // searchByFactory change: 공장 선택 시 라인 목록 Ajax 갱신 + 테이블/캘린더 재로드
            $('#searchByFactory').change(function() {
                var factoryData = $(this).val();
                $.ajax({
                    type: 'POST',
                    url: './proc/ajax_factory_line.php',
                    data: 'factoryData=' + factoryData + '&init=None',
                    success: function(html) {
                        // searchByLine 드롭다운 갱신
                        $('#searchByLine').html(html);
                        // 데이터 테이블 재로드
                        dataTable.draw();
                        // 캘린더 재로드
                        getMonth();
                    }
                });
            });

            // searchByLine change: 라인 선택 시 테이블/캘린더 재로드
            $('#searchByLine').change(function() {
                dataTable.draw();
                getMonth();
            });

            // add_button2 클릭: 신규 근무 시간 추가 모달 열기 (요일 지정 방식)
            // operation='AddW': 요일 기반 신규 추가
            $('#add_button2').click(function() {
                $('#modal_form')[0].reset();
                $('#oneday').hide();
                $('#period').show();
                $('#period_name').text('PERIOD');
                $('#dayOfTheWeek').show();
                $('.modal-title').text("ADD WORK TIME by Day Of The Week");
                $('#action').val("Add");
                $('#operation').val("AddW");
                // 모든 요일 체크박스 초기화
                for (var i = 0; i < 7; i++) {
                    $("input[name='week[" + i + "]']").attr("checked", false);
                }
                showDialogWindow(900, 750, false);
            });

            // 페이지 로드 시 현재 달 캘린더 초기 렌더링
            getMonth();

            // 모달 내 factory_idx change: Ajax로 라인 목록 갱신 (init=AllLine: '전체 라인' 옵션 포함)
            $("select[name='factory_idx']").on('change', function() {
                var factoryData = $(this).val();
                $.ajax({
                    type: 'POST',
                    url: './proc/ajax_factory_line.php',
                    data: 'factoryData=' + factoryData + '&init=AllLine',
                    success: function(html) {
                        // 모달 내 line_idx 드롭다운 갱신
                        $("#modal_form select[name='line_idx']").html(html);
                    }
                });
            });

            // #data_table 내 .status 요소 마우스오버: 같은 idx의 행 전체 배경 강조
            // idx 속성으로 연관된 .work_time_idx_{idx} 요소들을 그룹 하이라이트
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

            // .update1, .updatePeriod 클릭: kind=1 Period 수정 모달 열기
            // proc/set_work_time_insert.php POST {idx} → 기존 데이터 로드 → 모달 필드 채우기
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
                            // Period 수정: oneday 숨김, period 표시, dayOfTheWeek 숨김
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
                            $('#operation').val("Edit"); // kind=1 수정

                            // daterangepicker 재초기화: 기존 기간 설정
                            $('.input-datepicker').daterangepicker({
                                startDate: data.work_sdate,
                                endDate: data.work_edate,
                                locale: {
                                    format: 'YYYY-MM-DD',
                                    separator: ' ~ '
                                }
                            });

                            // factory_idx 설정: 0이면 전체('')로, 아니면 해당 공장 선택
                            if (data.factory_idx == '0') {
                                $('#factory_idx').val('');
                            } else {
                                $('#factory_idx').val(data.factory_idx);
                                // 해당 공장의 라인 목록 Ajax 로드 + line_idx 선택
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

                            // SHIFT-1 시간 값 채우기 (available + planned1~5 + over_time)
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

                            // SHIFT-2 시간 값 채우기
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

                            // SHIFT-3 시간 값 채우기
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

            // .update2, .updateWeek 클릭: kind=2 Week 수정 모달 열기 (Period + 요일 지정)
            // update1과 다른 점: dayOfTheWeek 표시, week_yn 7자리로 체크박스 복원
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
                            // Week 수정: oneday 숨김, period 표시, dayOfTheWeek 표시
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
                            $('#operation').val("EditW"); // kind=2 수정

                            // daterangepicker 재초기화
                            $('.input-datepicker').daterangepicker({
                                startDate: data.work_sdate,
                                endDate: data.work_edate,
                                locale: {
                                    format: 'YYYY-MM-DD',
                                    separator: ' ~ '
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

                            // week_yn: 7자리 문자열 (예: '1111100' = 월~금 근무)
                            // 각 자리가 '1'이면 해당 요일 체크박스 체크
                            var len = data.week_yn.length;
                            for (var i = 0; i < len; i++) {
                                if (data.week_yn.substr(i, 1) == '1') {
                                    $("input[name='week[" + i + "]']").attr("checked", true);
                                } else {
                                    $("input[name='week[" + i + "]']").attr("checked", false);
                                }
                            }

                            // SHIFT-1 시간 값 채우기
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

                            // SHIFT-2 시간 값 채우기
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

                            // SHIFT-3 시간 값 채우기
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

            // .update3, .updateDay 클릭: kind=3 Day 수정 모달 열기 (캘린더 날짜 클릭)
            // date 속성: 클릭한 날짜 (YYYY-MM-DD)
            // searchFactory/searchLine: 현재 필터된 공장/라인 기준으로 해당 날짜 데이터 조회
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
                            // Day 수정: oneday 표시, period 숨김, dayOfTheWeek 숨김
                            $('#oneday').show();
                            $('#period').hide();
                            $('#period_name').text('DATE');
                            $('#dayOfTheWeek').hide();
                            $('#data_idx').val(data.idx);
                            $('#idx').val(data.idx);
                            $('#remark').val(data.remark);
                            $('#status').val(data.status);
                            $('#oneday').val(oneday); // 클릭한 날짜 직접 설정

                            // data.idx 없으면 신규(AddD), 있으면 수정(EditD)
                            if (data.idx == '') {
                                $('.modal-title').text("ADD WORK TIME by Day");
                                $('#action').val("Add");
                                $('#operation').val("AddD"); // 특정 날짜 신규 추가
                            } else {
                                $('.modal-title').text("EDIT WORK TIME by Day");
                                $('#action').val("Edit");
                                $('#operation').val("EditD"); // 특정 날짜 수정
                            }

                            // daterangepicker 재초기화 (period는 숨겨지지만 초기화는 필요)
                            $('.input-datepicker').daterangepicker({
                                startDate: data.work_sdate,
                                endDate: data.work_edate,
                                locale: {
                                    format: 'YYYY-MM-DD',
                                    separator: ' ~ '
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

                            // SHIFT-1 시간 값 채우기
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

                            // SHIFT-2 시간 값 채우기
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

                            // SHIFT-3 시간 값 채우기
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

            // #modal_form submit: 근무 시간 저장 처리
            // 유효성 검사: factory_idx != '-1' && line_idx != '-1' && period != ''
            // POST proc/set_work_time_insert.php → FormData (multipart)
            // 성공 시: 폼 리셋 → 모달 닫기 → 테이블 재로드 → 캘린더 재로드
            $(document).on('submit', '#modal_form', function(event) {
                event.preventDefault();
                var factory_idx = $('#factory_idx').val();
                var line_idx = $('#line_idx').val();
                var period = $('#period').val();
                // 공장 미선택 차단
                if (factory_idx != '-1') {
                    // 라인 미선택 차단
                    if (line_idx != '-1') {
                        // 기간 미입력 차단
                        if (period != '') {
                            $.ajax({
                                url: "proc/set_work_time_insert.php",
                                method: 'POST',
                                data: new FormData(this), // FormData로 enctype 포함 전송
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
                                        // 데이터 테이블 갱신
                                        if (typeof dataTable != "undefined") {
                                            dataTable.draw();
                                        }
                                        // 캘린더 갱신
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

            /**
             * DataTables 초기화: 서버사이드 렌더링
             *
             * 설정:
             *   - paging: false (페이지네이션 없음, 전체 목록 표시)
             *   - serverSide: true, serverMethod: 'post'
             *   - colReorder: true (컬럼 드래그앤드롭 재정렬 가능)
             *   - responsive: childRow 방식 (좁은 화면에서 숨겨진 컬럼을 자식 행으로 표시)
             *   - ajax.data: searchByFactory/searchByLine/searchBystate/kind=0 추가 파라미터
             *   - createdCell: 모든 셀을 <a> 링크로 래핑
             *     - href: javascript:; (SPA 방식)
             *     - class: status, status_{Y/N}, update{kind}
             *     - idx: 레코드 PK
             *     - date: 기간(period)
             *
             * 컬럼:
             *   0: idx (숨김: dataTable.column(0).visible(false))
             *   1: no (순번, orderable=false)
             *   2: period (기간)
             *   3: locate (공장/라인 위치)
             *   4: week_yn_names (요일 이름 목록, orderable=false)
             *   5: status (사용 여부)
             *   6: kind (수정 모드 구분: 1/2/3)
             */
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
                        // 추가 검색 파라미터: 공장/라인 필터 + 상태 필터
                        var searchStatus = $('input:radio[name=searchBystate]:checked').val();
                        var searchFactory = $('#searchByFactory').val();
                        var searchLine = $('#searchByLine').val();

                        data.searchByFactory = searchFactory;
                        data.searchByLine = searchLine;
                        data.searchBystate = searchStatus;
                        data.kind = 0; // kind=0: 전체 조회
                    },
                },
                columns: [{
                        data: 'idx' // 숨김 컬럼 (PK)
                    },
                    {
                        data: 'no' // 순번
                    },
                    {
                        data: 'period' // 기간 (work_sdate ~ work_edate)
                    },
                    {
                        data: 'locate' // 공장/라인 위치 표시
                    },
                    {
                        data: 'week_yn_names' // 요일 이름 목록 (월,화,수...)
                    },
                    {
                        data: 'status' // 사용 여부 (Y/N)
                    },
                    {
                        data: 'kind' // 수정 모드 (1=Period, 2=Week, 3=Day)
                    },
                ],
                createdRow: function(row, data, dataIndex) {},
                columnDefs: [{
                    targets: '_all',
                    // 모든 셀을 <a> 링크로 래핑 → 클릭 시 해당 kind의 수정 모달 트리거
                    createdCell: function(td, cellData, rowData, rowIndex, colIndex) {
                        $(td).html('<a href="javascript:;" class="status status_' + rowData.status +
                            ' update' + rowData.kind + '" name="update" idx="' + rowData.idx +
                            '" date="' + rowData.period + '" data-toggle="modal" data-target="#userModal" data-backdrop="static">' + cellData + '</a>');
                    }
                }, {
                    // no(1), week_yn_names(4) 컬럼: 정렬 불가
                    targets: [1, 4],
                    orderable: false
                }],
            });
            // idx 컬럼(0번) 숨김 처리
            dataTable.column(0).visible(false);

            /**
             * getMonth — 월 캘린더 Ajax 로드
             * POST proc/set_work_time_fetch_month.php {year, month, factory_id, line_id}
             * 응답: { html: '...' } → #monthDiv에 렌더링
             * 캘린더 날짜 셀에 .update3/.updateDay 클래스 링크 포함
             */
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

            /**
             * prevMonth — 이전 달로 이동
             * 1월이면 12월로 + 연도 감소, 그 외에는 월만 감소
             */
            function prevMonth() {
                if (month <= 1) {
                    month = 12;
                    year--;
                } else {
                    month--;
                }
                getMonth();
            }

            /**
             * nextMonth — 다음 달로 이동
             * 12월이면 1월로 + 연도 증가, 그 외에는 월만 증가
             */
            function nextMonth() {
                if (month >= 12) {
                    month = 1;
                    year++;
                } else {
                    month++;
                }
                getMonth();
            }

            /**
             * thisMonth — 현재 달로 이동
             * PHP date() 기준으로 year/month 초기화 후 getMonth() 호출
             */
            function thisMonth() {
                year = <?= date('Y') ?>;
                month = <?= date('n') ?>;
                getMonth();
            }

            // 캘린더 네비게이션 버튼에서 호출할 수 있도록 window에 전역 노출
            // proc/set_work_time_fetch_month.php에서 렌더링하는 HTML 내 onclick="prevMonth()" 등에서 호출
            window.prevMonth = prevMonth;
            window.nextMonth = nextMonth;
            window.thisMonth = thisMonth;

            // reload 버튼: 클릭 시 전체 페이지 새로고침
            $("#reload").click(function() {
                location.reload();
            });

        }); // $(document).ready
    }); // waitForJQuery
</script>


</body>

</html>