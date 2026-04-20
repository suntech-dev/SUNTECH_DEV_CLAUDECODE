<?php
// 공통 헬퍼

function now_datetime(): string {
    return date('Y-m-d H:i:s');
}

function json_ok(string $msg = 'success', array $extra = []): void {
    header('Content-Type: application/json; charset=utf-8');
    echo json_encode(array_merge(['result' => 'ok', 'msg' => $msg], $extra), JSON_UNESCAPED_UNICODE);
    exit;
}

function json_fail(string $msg): void {
    header('Content-Type: application/json; charset=utf-8');
    echo json_encode(['result' => 'fail', 'error' => $msg], JSON_UNESCAPED_UNICODE);
    exit;
}

// DataTables 서버사이드 응답 형식 (DataTables 1.10+)
function dt_response(int $draw, int $total, int $filtered, array $data): void {
    header('Content-Type: application/json; charset=utf-8');
    echo json_encode([
        'draw'            => $draw,
        'recordsTotal'    => $total,
        'recordsFiltered' => $filtered,
        'data'            => $data,
    ], JSON_UNESCAPED_UNICODE);
    exit;
}

// DataTables POST 파라미터 파싱
function dt_params(): array {
    return [
        'draw'        => (int)($_POST['draw']  ?? 1),
        'start'       => (int)($_POST['start'] ?? 0),
        'length'      => (int)($_POST['length'] ?? 25),
        'search'      => trim($_POST['search']['value'] ?? ''),
        'col_idx'     => (int)($_POST['order'][0]['column'] ?? 0),
        'col_dir'     => strtolower($_POST['order'][0]['dir'] ?? 'desc') === 'asc' ? 'ASC' : 'DESC',
        'col_name'    => $_POST['columns'][$_POST['order'][0]['column'] ?? 0]['data'] ?? '',
    ];
}
