<?php
// 공통 헬퍼 함수

// 클라이언트 IP (IPv6 포함, 최대 45자)
function get_client_ip(): string {
    foreach (['HTTP_CLIENT_IP', 'HTTP_X_FORWARDED_FOR', 'REMOTE_ADDR'] as $key) {
        if (!empty($_SERVER[$key])) {
            $ip = trim(explode(',', $_SERVER[$key])[0]);
            if (filter_var($ip, FILTER_VALIDATE_IP)) return substr($ip, 0, 45);
        }
    }
    return '';
}

// 현재 일시 (MySQL DATETIME 형식)
function now_datetime(): string {
    return date('Y-m-d H:i:s');
}

// GET/POST 파라미터 취득
function get_param(string $key, string $default = ''): string {
    return isset($_GET[$key]) ? trim($_GET[$key])
         : (isset($_POST[$key]) ? trim($_POST[$key]) : $default);
}

// JSON 응답 출력 후 종료
function json_response(array $data, int $status = 200): void {
    http_response_code($status);
    header('Content-Type: application/json; charset=utf-8');
    header('Access-Control-Allow-Origin: *');
    echo json_encode($data, JSON_UNESCAPED_UNICODE);
    exit;
}

// API 로그 기록
function write_log(PDO $pdo, string $code, array $data, array $result): void {
    $stmt = $pdo->prepare(
        "INSERT INTO lm_logs (code, log_data, log_result, reg_ip, reg_date)
         VALUES (?, ?, ?, ?, ?)"
    );
    $stmt->execute([
        $code,
        json_encode($data,   JSON_UNESCAPED_UNICODE),
        json_encode($result, JSON_UNESCAPED_UNICODE),
        get_client_ip(),
        now_datetime(),
    ]);
}
