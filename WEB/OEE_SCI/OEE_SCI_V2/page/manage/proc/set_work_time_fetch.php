<?php

/**
 * proc/set_work_time_fetch.php — 근무 시간 설정 목록 조회 (DataTables 서버사이드)
 * POST 방식으로 DataTables draw/search/order 파라미터를 수신하여
 * info_work_time 테이블 데이터를 JSON으로 반환
 *
 * 근무 시간(Work Time) 종류 (kind):
 *   1 = Period  : 특정 기간(기간제) 적용
 *   2 = Week    : 요일 반복 적용 (week_yn으로 요일 지정)
 *   3 = Day     : 특정 날짜 1일 적용
 *
 * 파라미터:
 *   searchBystate   : 사용 여부 필터 (Y/N)
 *   searchByFactory : 공장 idx 필터
 *   searchByLine    : 라인 idx 필터
 *   search[value]   : remark LIKE 검색
 *   order[0][column]: DataTables 정렬 컬럼 인덱스
 *   columns[n][data]: 컬럼명 (화이트리스트 매핑)
 *   order[0][dir]   : ASC/DESC
 *   draw            : DataTables 요청 식별자 (그대로 반환)
 *   kind            : 특정 kind만 필터링
 *
 * 응답: DataTables 서버사이드 형식 { draw, iTotalRecords, iTotalDisplayRecords, aaData }
 */
require_once(__DIR__ . '/../../../lib/db.php');
require_once(__DIR__ . '/../../../lib/worktime_common.php');

// POST 파라미터 수신 및 기본값 설정
$searchBystate   = isset($_POST['searchBystate'])   ? $_POST['searchBystate']   : '';
$searchByFactory = isset($_POST['searchByFactory']) ? $_POST['searchByFactory'] : '';
$searchByLine    = isset($_POST['searchByLine'])    ? $_POST['searchByLine']    : '';

// DataTables 검색/정렬 파라미터
$searchValue     = isset($_POST['search']['value'])              ? $_POST['search']['value']              : '';
$columnIndex     = isset($_POST['order'][0]['column'])            ? $_POST['order'][0]['column']            : '';
$columnName      = isset($_POST['columns'][$columnIndex]['data']) ? $_POST['columns'][$columnIndex]['data'] : '';
$columnSortOrder = isset($_POST['order'][0]['dir'])               ? strtoupper($_POST['order'][0]['dir'])   : 'ASC';

$draw = isset($_POST['draw']) ? (int)$_POST['draw'] : 1;
$kind = isset($_POST['kind']) ? $_POST['kind'] : 0;

// ORDER BY 화이트리스트: DataTables 컬럼명 → 실제 DB 컬럼 별칭 매핑
// SQL 인젝션 방지를 위해 사전 허용된 컬럼만 정렬 가능
$valid_sort_columns = [
    'idx'          => 'a.idx',
    'period'       => 'a.work_sdate',   // 기간 표시는 work_sdate 기준 정렬
    'locate'       => 'b.factory_name', // 공장명 기준 정렬
    'factory_name' => 'b.factory_name',
    'line_name'    => 'c.line_name',
    'kind'         => 'a.kind',
    'status'       => 'a.status',
    'remark'       => 'a.remark',
    'reg_date'     => 'a.reg_date',
];
// 화이트리스트에 없는 컬럼명은 기본값 'a.kind'로 대체 (SQL 인젝션 방지)
$orderColumn = $valid_sort_columns[$columnName] ?? 'a.kind';
if (!in_array($columnSortOrder, ['ASC', 'DESC'])) {
    $columnSortOrder = 'ASC';
}

// WHERE 절 파라미터 바인딩 구성
$qry    = [];
$params = [];

// kind 필터: 특정 종류(기간/요일/일별)만 표시
if ($kind) {
    $qry[]    = 'a.kind = ?';
    $params[] = $kind;
}

// 공장/라인 필터:
// - 특정 공장+라인 조합: factory_idx=? AND line_idx=?
// - 전체 공장 적용(factory_idx=0): OR a.factory_idx = 0
// - 특정 공장의 전체 라인(line_idx=0): OR a.factory_idx=? AND a.line_idx=0
if ($searchByFactory) {
    $qry[]    = '(a.factory_idx = ? AND a.line_idx = ? OR a.factory_idx = 0 OR a.factory_idx = ? AND a.line_idx = 0)';
    $params[] = $searchByFactory;
    $params[] = $searchByLine;
    $params[] = $searchByFactory;
}

// 사용 여부 필터: Y(사용중) / N(미사용)
if ($searchBystate) {
    $qry[]    = 'a.status = ?';
    $params[] = $searchBystate;
}

// remark 필드 LIKE 검색 (비고 내용으로 검색)
if ($searchValue) {
    $qry[]    = '(a.remark LIKE ?)';
    $params[] = '%' . $searchValue . '%';
}

$where = $qry ? ' WHERE ' . implode(' AND ', $qry) : '';
// DataTables에서 컬럼 선택 시 해당 컬럼 정렬, 아닐 때는 kind→reg_date 기본 정렬
$order = $columnIndex !== '' ? " ORDER BY {$orderColumn} {$columnSortOrder}" : ' ORDER BY kind ASC, reg_date ASC';

// 메인 쿼리:
// - period: work_sdate ~ work_edate 하나의 문자열로 CONCAT
// - locate: 공장명 > 라인명 형식 (NULL이면 'all' 표시)
// - factory_name / line_name: NULL이면 '- ALL -' (전체 적용 설정 표시용)
// - LEFT JOIN: factory/line 설정이 없는 경우(전체 적용)도 조회 포함
$stmt = $pdo->prepare(
    "SELECT
        a.idx,
        CONCAT(work_sdate, ' ~ ', work_edate) AS period,
        CONCAT(IFNULL(b.factory_name, 'all'), ' > ', IFNULL(c.line_name, 'all')) AS locate,
        IFNULL(b.factory_name, '- ALL -') AS factory_name,
        IFNULL(c.line_name, '- ALL -') AS line_name,
        a.kind, a.status, a.remark,
        LEFT(a.reg_date, 10) AS reg_date,
        week_yn
     FROM `info_work_time` AS a
     LEFT JOIN `info_factory` AS b ON a.factory_idx = b.idx
     LEFT JOIN `info_line`    AS c ON a.line_idx    = c.idx
     {$where}{$order}"
);
$stmt->execute($params);
$result        = $stmt->fetchAll(PDO::FETCH_ASSOC);
$filtered_rows = count($result);

// 요일 이름 배열 (인덱스 0=일요일 ~ 6=토요일)
$week_arr_short = ['Su', 'Mo', 'Tu', 'We', 'Th', 'Fr', 'Sa'];

/**
 * week_yn 문자열을 HTML 포맷으로 변환
 * - week_yn: 7자리 이진 문자열 (예: '0111110' = 월~금)
 * - '1' 위치의 요일: 일반 텍스트 (활성 요일)
 * - '0' 위치의 요일: 회색(#6a6a6a) span 처리 (비활성 요일)
 * - 결과: "Su <span style="color:#6a6a6a">Mo</span> Tu We Th Fr Sa" 형태
 */
function buildWeekYnHtml(string $week_yn, array $week_arr_short): string
{
    $html = '';
    $len  = strlen($week_yn);
    for ($i = 0; $i < $len; $i++) {
        if (substr($week_yn, $i, 1) === '1') {
            // 활성 요일: 일반 텍스트로 표시
            $html .= $week_arr_short[$i] . ' ';
        } else {
            // 비활성 요일: 회색으로 흐리게 표시
            $html .= '<span style="color:#6a6a6a">' . $week_arr_short[$i] . '</span> ';
        }
    }
    return $html;
}

// 역순 번호 계산용: 전체 결과 수 기준으로 no 내림차순 부여
$no = $filtered_rows;

// 결과 후처리: 각 레코드에 표시용 필드 추가/변환
foreach ($result as $row => $value) {
    // no: 역순 번호 (마지막 항목이 1번)
    $result[$row]['no']     = $no--;
    // status: DB 'Y'/'N' → 표시용 'used'/'unused' 변환
    $result[$row]['status'] = ($value['status'] === 'Y') ? 'used' : 'unused';
    $result[$row]['week_yn_names'] = '';

    switch ($value['kind']) {
        case '2':
            // Week 타입: week_yn을 요일 HTML로 변환하여 week_yn_names에 저장
            $result[$row]['week_yn_names'] = buildWeekYnHtml($value['week_yn'], $week_arr_short);
            // kind 파라미터가 2이면 'Period'로 표시, 아니면 'Week'
            $result[$row]['kind'] = ($kind == 2) ? 'Period' : 'Week';
            break;
        case '3':
            // Day 타입: period에서 날짜 부분(10자)만 추출 (시간 제거)
            $result[$row]['period'] = substr($value['period'], 0, 10);
            $result[$row]['kind']   = 'Day';
            break;
        case '1':
            // Period 타입: kind 레이블만 변경
            $result[$row]['kind'] = 'Period';
            break;
        default:
            $result[$row]['kind'] = 'None';
    }
}

// 전체 레코드 수 (필터 없이): DataTables의 iTotalDisplayRecords에 사용
// (DataTables가 "N건 중 M건 표시" 형태로 표기하는 데 필요)
$total_all_records = (int)$pdo->query("SELECT COUNT(*) FROM `info_work_time`")->fetchColumn();

// DataTables 서버사이드 응답 형식
echo json_encode([
    'draw'                 => $draw,               // 요청 식별자 (그대로 반환)
    'iTotalRecords'        => $filtered_rows,       // 필터 적용 후 결과 수
    'iTotalDisplayRecords' => $total_all_records,  // 전체 레코드 수 (필터 없이)
    'aaData'               => $result,             // 실제 데이터 배열
]);
exit();
