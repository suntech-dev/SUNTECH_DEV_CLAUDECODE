<?php

/**
 * API 헬퍼 라이브러리
 * 모든 API 파일에서 공통으로 사용되는 기능들을 통합 관리합니다.
 * 
 * 주요 기능:
 * - MAC 주소 검증 및 처리
 * - 장비 정보 조회
 * - 근무시간 처리
 * - API 로깅
 * - 공통 응답 처리
 * 
 * 사용법:
 * $apiHelper = new ApiHelper($pdo);
 * $apiHelper->validateMac($mac);
 */

require_once(__DIR__ . '/validator.lib.php');
require_once(__DIR__ . '/database_helper.lib.php');
require_once(__DIR__ . '/worktime.lib.php');
require_once(__DIR__ . '/get_shift.lib.php');

class ApiHelper
{
    private $pdo;
    private $validator;
    private $dbHelper;
    private $worktime;

    public function __construct($pdo)
    {
        $this->pdo = $pdo;
        $this->validator = new InputValidator();
        $this->dbHelper = new DatabaseHelper($pdo);
        $this->worktime = new Worktime($pdo);
    }

    /**
     * MAC 주소 파라미터를 받아서 검증하고 처리하는 통합 함수
     * 모든 API에서 반복되는 MAC 주소 처리를 일원화
     * 
     * @param string $mac_param $_REQUEST에서 받은 MAC 주소
     * @return string 정제된 MAC 주소 (대문자, 17자)
     * @throws Exception MAC 주소가 유효하지 않으면 JSON 응답 후 종료
     */
    public function validateAndProcessMac($mac_param = null)
    {
        // MAC 주소 파라미터 수신
        $mac = !empty($mac_param) ? trim($mac_param) : (!empty($_REQUEST['mac']) ? trim($_REQUEST['mac']) : '');

        // 대문자로 변환
        $mac = strtoupper($mac);

        // MAC 주소 유효성 검사
        if (empty($mac) || $mac == '99') {
            jsonReturn(['code' => '99', 'msg' => 'MAC address is required.']);
        }

        if (!$this->validator->validateMac($mac)) {
            jsonReturn(['code' => '99', 'msg' => 'Incorrect MAC address format. Expected: XX:XX:XX:XX:XX:XX']);
        }

        return $mac;
    }

    /**
     * MAC 주소로 장비 정보를 조회하는 통합 함수
     * 공장, 라인 정보도 함께 JOIN해서 가져옴
     * 
     * @param string $mac MAC 주소
     * @param bool $includeFactoryLine 공장/라인 정보 포함 여부
     * @return array 장비 정보 (없으면 JSON 응답 후 종료)
     */
    public function getMachineInfo($mac, $includeFactoryLine = true)
    {
        if ($includeFactoryLine) {
            $machine_data = $this->dbHelper->getMachineWithFactoryAndLine($mac);
        } else {
            $machine_data = $this->dbHelper->getMachineByMac($mac);
        }

        if (!$machine_data) {
            jsonReturn(['code' => '99', 'msg' => 'No matching machine found for the provided MAC address.']);
        }

        return $machine_data;
    }

    /**
     * 현재 근무시간 정보를 조회하는 통합 함수
     * 모든 근무시간 관련 API에서 공통으로 사용
     * 
     * @param int $factory_idx 공장 인덱스
     * @param int $line_idx 라인 인덱스
     * @param string $current_datetime 현재 시간 (Y-m-d H:i:s)
     * @return array|null 현재 근무시간 정보
     */
    public function getCurrentShiftInfo($factory_idx, $line_idx, $current_datetime)
    {
        return findCurrentShift($this->pdo, $this->worktime, $factory_idx, $line_idx, $current_datetime);
    }

    /**
     * 근무시간 지표를 계산하는 통합 함수
     * 
     * @param array $current_shift_info 현재 근무조 정보
     * @param string $current_datetime 현재 시간
     * @return array 계산된 근무시간 지표
     */
    public function calculateShiftMetrics($current_shift_info, $current_datetime)
    {
        if (!$current_shift_info) {
            return null;
        }

        $all_shifts_on_date = $this->worktime->getDayShift(
            $current_shift_info['date'],
            $current_shift_info['factory_idx'],
            $current_shift_info['line_idx']
        );

        return calculateWorktimeMetrics($current_shift_info, $all_shifts_on_date, $current_datetime);
    }

    /**
     * 안돈 정보를 조회하는 통합 함수
     * 
     * @param int $andon_idx 안돈 인덱스
     * @return array 안돈 정보 (없으면 JSON 응답 후 종료)
     */
    public function getAndonInfo($andon_idx)
    {
        if (empty($andon_idx) || $andon_idx == '0') {
            jsonReturn(['code' => '99', 'msg' => 'Andon index is required.']);
        }

        $andon_data = $this->dbHelper->getAndonInfo($andon_idx);

        if (!$andon_data) {
            jsonReturn(['code' => '99', 'msg' => 'No matching andon information found in database.']);
        }

        return $andon_data;
    }

    /**
     * 비가동 정보를 조회하는 통합 함수
     * 
     * @param int $downtime_idx 비가동 인덱스
     * @return array 비가동 정보 (없으면 JSON 응답 후 종료)
     */
    public function getDowntimeInfo($downtime_idx)
    {
        if (empty($downtime_idx) || $downtime_idx == '0') {
            jsonReturn(['code' => '99', 'msg' => 'Downtime index is required.']);
        }

        $downtime_data = $this->dbHelper->getDowntimeInfo($downtime_idx);

        if (!$downtime_data) {
            jsonReturn(['code' => '99', 'msg' => 'No matching downtime information found in database.']);
        }

        return $downtime_data;
    }

    /**
     * 불량 정보를 조회하는 통합 함수
     * 
     * @param int $defective_idx 불량 인덱스
     * @return array 불량 정보 (없으면 JSON 응답 후 종료)
     */
    public function getDefectiveInfo($defective_idx)
    {
        if (empty($defective_idx) || $defective_idx == '0') {
            jsonReturn(['code' => '99', 'msg' => 'Defective index is required.']);
        }

        $defective_data = $this->dbHelper->getDefectiveInfo($defective_idx);

        if (!$defective_data) {
            jsonReturn(['code' => '99', 'msg' => 'No matching defective information found in database.']);
        }

        return $defective_data;
    }

    /**
     * 디자인 공정 정보를 조회하는 통합 함수
     * 
     * @param string $design_no 디자인 번호
     * @return array 디자인 공정 정보
     */
    public function getDesignProcessInfo($design_no)
    {
        return $this->dbHelper->getDesignProcessInfo($design_no);
    }

    /**
     * API 호출 로그를 저장하는 통합 함수
     * 모든 API에서 통일된 로깅 처리
     * 
     * @param string $table_name 로그 테이블명 (logs_api_*)
     * @param string $gubun API 구분자
     * @param string $machine_no 장비 번호
     * @param string $mac MAC 주소
     * @param array $request_data 요청 데이터 ($_REQUEST)
     * @param array $response_data 응답 데이터
     * @param string $reg_date 등록 시간
     */
    public function logApiCall($table_name, $gubun, $machine_no, $mac, $request_data, $response_data, $reg_date)
    {
        try {
            $log_result = json_encode($response_data, JSON_UNESCAPED_UNICODE | JSON_UNESCAPED_SLASHES);
            $log_data = json_encode($request_data, JSON_UNESCAPED_UNICODE | JSON_UNESCAPED_SLASHES);

            // 동적으로 테이블명을 사용하여 로그 저장 (보안상 검증된 테이블명만 사용)
            $allowed_tables = [
                'logs_api_start',
                'logs_api_get_andonlist',
                'logs_api_get_downtimelist',
                'logs_api_get_defectivelist',
                'logs_api_get_datetime',
                'logs_api_send_andon_warning',
                'logs_api_send_andon_completed',
                'logs_api_send_downtime_warning',
                'logs_api_send_downtime_completed',
                'logs_api_send_defective_warning',
                'logs_api_send_pCount'
            ];

            if (!in_array($table_name, $allowed_tables)) {
                error_log("Invalid log table name: " . $table_name);
                return false;
            }

            $stmt = $this->pdo->prepare(
                "INSERT INTO `{$table_name}` (gubun, machine_no, mac, data, result, reg_date) VALUES (?, ?, ?, ?, ?, ?)"
            );

            return $stmt->execute([$gubun, $machine_no, $mac, $log_data, $log_result, $reg_date]);
        } catch (PDOException $e) {
            error_log("API log error: " . $e->getMessage());
            return false;
        }
    }

    /**
     * Warning → Completed 경과 시간 계산
     * send_andon_completed, send_downtime_completed에서 공통 사용
     *
     * @param string $reg_date Warning 등록 시간 (Y-m-d H:i:s)
     * @param string $today    현재 시간 (Y-m-d H:i:s)
     * @return array [$duration_sec, $duration_str]  예: [125, '2m 5s ']
     */
    public function calculateDuration(string $reg_date, string $today): array
    {
        if (empty($reg_date) || $reg_date === '0000-00-00 00:00:00') {
            return [0, ''];
        }
        $diff = max(0, strtotime($today) - strtotime($reg_date));
        $str  = '';
        $h    = floor($diff / 3600);
        if ($h  > 0) $str .= $h  . 'h ';
        $m    = floor(($diff % 3600) / 60);
        if ($m  > 0) $str .= $m  . 'm ';
        $s    = $diff % 60;
        if ($s  > 0) $str .= $s  . 's ';
        return [$diff, $str];
    }

    /**
     * 안돈/비가동/불량 목록 조회를 위한 통합 함수
     * get_andonList, get_downtimeList, get_defectiveList에서 공통 사용
     * 
     * @param string $type 'andon', 'downtime', 'defective'
     * @param string $mac MAC 주소
     * @param array $machine_data 장비 정보
     * @param string $work_date 작업일 (필터링용)
     * @param int $shift_idx 교대 인덱스 (필터링용)
     * @return array 목록 데이터
     */
    public function getStatusList($type, $mac, $machine_data, $work_date = null, $shift_idx = null)
    {
        $today = date('Y-m-d H:i:s');
        $date = date('Y-m-d');

        // 타입별 테이블과 컬럼 설정
        $table_config = [
            'andon' => [
                'info_table' => 'info_andon',
                'data_table' => 'data_andon',
                'idx_column' => 'andon_idx',
                'name_column' => 'andon_name',
                'result_prefix' => 'andon'
            ],
            'downtime' => [
                'info_table' => 'info_downtime',
                'data_table' => 'data_downtime',
                'idx_column' => 'downtime_idx',
                // 'name_column' => 'downtime_name',
                'name_column' => 'downtime_shortcut',
                'result_prefix' => 'downtime'
            ],
            'defective' => [
                'info_table' => 'info_defective',
                'data_table' => 'data_defective',
                'idx_column' => 'defective_idx',
                // 'name_column' => 'defective_name', 
                'name_column' => 'defective_shortcut',
                'result_prefix' => 'defective'
            ]
        ];

        if (!isset($table_config[$type])) {
            return [];
        }

        $config = $table_config[$type];
        $machine_type = $machine_data['type'] ?? 'Machine';
        $line_idx = $machine_data['line_idx'];

        // 장비 타입에 따라 JOIN 조건과 파라미터 설정
        $join_conditions = [];
        $params = [];

        if ($machine_type == 'W' || $machine_type == 'Warning') {
            $join_conditions[] = "b.line_idx = ?";
            $params[] = $line_idx;
        } else {
            $join_conditions[] = "b.mac = ?";
            $params[] = $mac;
        }

        // 작업일과 교대 필터링 추가
        if ($work_date && $shift_idx) {
            $join_conditions[] = "b.work_date = ?";
            $join_conditions[] = "b.shift_idx = ?";
            $params[] = $work_date;
            $params[] = $shift_idx;
        }

        $join_condition = implode(' AND ', $join_conditions);

        // 목록 조회 쿼리 (work_date, shift_idx 필터링 포함)
        $sql = "SELECT 
              a.idx, 
              a.{$config['name_column']},  
              COUNT(if(b.status <> 'Completed', b.status, NULL)) AS not_completed_qty
            FROM `{$config['info_table']}` AS a 
            LEFT JOIN `{$config['data_table']}` AS b ON a.idx = b.{$config['idx_column']} AND {$join_condition}
            WHERE a.status = 'Y'
            GROUP BY a.idx, a.{$config['name_column']}
            ORDER BY a.idx ASC";

        $stmt = $this->pdo->prepare($sql);
        $stmt->execute($params);
        $result = $stmt->fetchAll(PDO::FETCH_ASSOC);

        $items = [];
        $warning_blink = '0'; // 기본값

        if ($config['info_table'] == 'info_andon') {
            foreach ($result as $value) {
                $items[] = [
                    $config['result_prefix'] . '_idx' => $value['idx'],
                    $config['result_prefix'] . '_name' => $value[$config['name_column']],
                    'not_completed_qty' => $value['not_completed_qty'],
                    'warning_blink' => $warning_blink
                ];
            }
        } else {
            foreach ($result as $value) {
                $items[] = [
                    $config['result_prefix'] . '_idx' => $value['idx'],
                    $config['result_prefix'] . '_name' => $value[$config['name_column']],
                    'not_completed_qty' => $value['not_completed_qty']
                    // 'warning_blink' => $warning_blink
                ];
            }
        }

        /* foreach ($result as $value) {
      $items[] = [
        $config['result_prefix'] . '_idx' => $value['idx'],
        $config['result_prefix'] . '_name' => $value[$config['name_column']],
        'not_completed_qty' => $value['not_completed_qty']
        // 'warning_blink' => $warning_blink
      ];
    } */

        return $items;
    }

    /**
     * Warning 데이터를 삽입하는 통합 함수
     * andon, downtime, defective warning에서 공통 사용
     * 
     * @param string $type 'andon', 'downtime', 'defective'
     * @param array $data 삽입할 데이터
     * @return bool 성공 여부
     */
    public function insertWarningData($type, $data)
    {
        $table_mapping = [
            'andon' => 'data_andon',
            'downtime' => 'data_downtime',
            'defective' => 'data_defective'
        ];

        if (!isset($table_mapping[$type])) {
            return false;
        }

        $table = $table_mapping[$type];

        return $this->dbHelper->insert($table, $data);
    }

    /**
     * Completed 데이터를 업데이트하는 통합 함수
     * andon, downtime completed에서 공통 사용
     * 
     * @param string $type 'andon', 'downtime'
     * @param string $mac MAC 주소
     * @param int $idx 인덱스 (andon_idx 또는 downtime_idx)
     * @param string $today 현재 시간
     * @return bool 성공 여부
     */
    public function updateCompletedStatus($type, $mac, $idx, $today)
    {
        $table_mapping = [
            'andon' => ['table' => 'data_andon', 'idx_column' => 'andon_idx'],
            'downtime' => ['table' => 'data_downtime', 'idx_column' => 'downtime_idx']
        ];

        if (!isset($table_mapping[$type])) {
            return false;
        }

        $config = $table_mapping[$type];
        $active_record = $this->dbHelper->findActiveRecord($mac, $idx, $config['table']);

        if (!$active_record) {
            return false;
        }

        // 비가동의 경우 지속 시간 계산
        $update_data = ['status' => 'Completed', 'update_date' => $today];

        if ($type === 'downtime') {
            $reg_datetime = new DateTime($active_record['reg_date']);
            $current_datetime = new DateTime($today);
            $duration_sec = $current_datetime->getTimestamp() - $reg_datetime->getTimestamp();

            $update_data['duration_sec'] = $duration_sec;
            $update_data['duration_his'] = secondsToHis($duration_sec);
        }

        return $this->dbHelper->update($config['table'], $update_data, ['idx' => $active_record['idx']]);
    }

    /**
     * OEE 관련 데이터 조회를 위한 통합 함수
     * 
     * @param string $mac MAC 주소
     * @param string $work_date 작업일 
     * @param int $shift_idx 교대 인덱스
     * @return array OEE 계산용 데이터
     */
    public function getOeeCalculationData($mac, $work_date, $shift_idx)
    {
        return $this->dbHelper->getOeeCalculationData($mac, $work_date, $shift_idx);
    }

    /**
     * 공통 JSON 응답 헬퍼 함수
     * 성공/실패 응답을 일관된 형태로 반환
     * 
     * @param string $code 응답 코드 ('00' = 성공, '99' = 실패)
     * @param string $msg 메시지
     * @param array $data 추가 데이터 (선택사항)
     * @return array 응답 배열
     */
    public function createResponse($code, $msg, $data = null)
    {
        $response = ['code' => $code, 'msg' => $msg];

        if ($data !== null) {
            if (is_array($data)) {
                $response = array_merge($response, $data);
            } else {
                $response['data'] = $data;
            }
        }

        return $response;
    }

    /**
     * 공통 JSON 응답 헬퍼 함수
     * 성공/실패 없이 items 만 일관된 형태로 반환
     * 
     * get_andonList.php
     * get_downtimeList.php
     * get_defectiveList.php
     * get_dateTime.php   * 
     * 
     * @param array $data 추가 데이터
     * @return array 응답 배열
     */
    public function createResponse_onlyItems($data = null)
    {
        if ($data !== null) {
            if (is_array($data)) {
                $response = ($data);
            } else {
                $response['data'] = $data;
            }
        }

        return $response;
    }

    /**
     * OEE 지표를 계산하는 통합 함수 (send_pCount.php에서 사용)
     * 
     * @param string $mac MAC 주소
     * @param string $work_date 작업일
     * @param int $shift_idx 교대 인덱스
     * @param int $runtime_sec 경과 시간 (초)
     * @param float $planned_work_time_hour 계획된 근무 시간 (시간)
     * @param int $std_mc_needed 표준 필요 장비 수
     * @param int $target 목표 생산량
     * @param int $actual_output 실제 생산량
     * @return array 계산된 OEE 지표 배열
     */
    public function calculateOeeMetrics($mac, $work_date, $shift_idx, $runtime_sec, $planned_work_time_hour, $std_mc_needed, $target, $actual_output)
    {
        $metrics = [];

        // downtime 및 defective 수량을 한 번의 쿼리로 조회 (DatabaseHelper 사용)
        $oee_data = $this->dbHelper->getOeeCalculationData($mac, $work_date, $shift_idx);

        $downtime_duration_sum = $oee_data['downtime_duration_sum'] ?? 0;
        $metrics['defective'] = $oee_data['defective_count'] ?? 0;

        // 가동 시간 관련 계산 (단위: 초)
        $metrics['downtime'] = $downtime_duration_sum;
        $productive_runtime_sec = $runtime_sec - $downtime_duration_sum;
        $metrics['productive_runtime'] = $productive_runtime_sec;
        $metrics['availabilty_rate'] = ($runtime_sec > 0) ? round((($productive_runtime_sec / $runtime_sec) * 100), 1) : 0;

        // 생산량 목표 관련 계산
        $target_line_per_day = $target;
        $target_line_per_hour = ($planned_work_time_hour > 0) ? ($target_line_per_day / $planned_work_time_hour) : 0;
        $target_mc_per_day = ($std_mc_needed > 0) ? ($target_line_per_day / $std_mc_needed) : 0;
        $target_mc_per_hour = ($std_mc_needed > 0) ? ($target_line_per_hour / $std_mc_needed) : 0;
        $metrics['target_line_per_day'] = $target_line_per_day;
        $metrics['target_line_per_hour'] = $target_line_per_hour;
        $metrics['target_mc_per_day'] = $target_mc_per_day;
        $metrics['target_mc_per_hour'] = $target_mc_per_hour;

        // 사이클 타임 및 이론적 생산량 계산
        $metrics['cycletime'] = ($target_mc_per_hour > 0) ? (3600 / $target_mc_per_hour) : 0;
        $metrics['theoritical_output'] = ($metrics['cycletime'] > 0) ? ($productive_runtime_sec / $metrics['cycletime']) : 0;

        // 생산 효율 및 품질 관련 계산
        $metrics['productivity_rate'] = ($metrics['theoritical_output'] > 0) ? round((($actual_output / $metrics['theoritical_output']) * 100), 1) : 0;
        $metrics['actual_a_grade'] = $actual_output - $metrics['defective'];
        $metrics['quality_rate'] = ($actual_output > 0) ? round((($metrics['actual_a_grade'] / $actual_output) * 100), 1) : 0;

        // 최종 OEE 계산
        $metrics['oee'] = round((($metrics['availabilty_rate'] * $metrics['productivity_rate'] * $metrics['quality_rate']) / 10000), 2);

        return $metrics;
    }

    /**
     * OEE 데이터를 데이터베이스에 저장하는 통합 함수
     * 
     * @param array $oee_data OEE 데이터 배열
     * @param bool $is_update 업데이트 여부 (false면 INSERT)
     * @param int $existing_idx 기존 데이터 인덱스 (업데이트 시 사용)
     * @return bool 성공 여부
     */
    public function saveOeeData($oee_data, $is_update = false, $existing_idx = null)
    {
        if ($is_update && $existing_idx) {
            // UPDATE 쿼리 실행
            unset($oee_data['reg_date']); // UPDATE 시 reg_date 제외
            return $this->dbHelper->update('data_oee', $oee_data, ['idx' => $existing_idx]);
        } else {
            // INSERT 쿼리 실행
            return $this->dbHelper->insert('data_oee', $oee_data);
        }
    }

    /**
     * OEE 추적 데이터를 저장하는 함수
     * 
     * @param array $tracking_data 추적 데이터 배열
     * @return bool 성공 여부
     */
    public function saveOeeTrackingData($tracking_data)
    {
        return $this->dbHelper->insert('data_oee_rows', $tracking_data);
    }

    /**
     * 첫 번째 다운타임을 기록하는 함수 (send_pCount.php에서 사용)
     * 
     * @param array $downtime_data 다운타임 데이터
     * @return bool 성공 여부
     */
    public function insertFirstDowntime($downtime_data)
    {
        return $this->dbHelper->insert('data_downtime', $downtime_data);
    }

    /**
     * 초를 시:분:초 형태로 변환하는 헬퍼 함수
     * 
     * @param int $seconds 초
     * @return string HH:MM:SS 형태의 시간
     */
    public function secondsToHis($seconds)
    {
        if (function_exists('secondsToHis')) {
            return secondsToHis($seconds);
        }

        // 함수가 없는 경우 직접 계산
        $hours = floor($seconds / 3600);
        $minutes = floor(($seconds % 3600) / 60);
        $secs = $seconds % 60;

        return sprintf('%02d:%02d:%02d', $hours, $minutes, $secs);
    }

    /**
     * 다중 API 작업을 배치로 처리하는 헬퍼 함수
     * 여러 개의 warning이나 completed 작업을 한 번에 처리
     * 
     * @param array $operations 작업 배열
     * @return array 결과 배열
     */
    public function batchOperations($operations)
    {
        $results = [];

        try {
            $this->pdo->beginTransaction();

            foreach ($operations as $operation) {
                $results[] = $this->executeOperation($operation);
            }

            $this->pdo->commit();
            return $results;
        } catch (Exception $e) {
            $this->pdo->rollback();
            error_log("Batch operation error: " . $e->getMessage());
            return ['error' => $e->getMessage()];
        }
    }

    /**
     * 개별 작업을 실행하는 내부 함수
     * 
     * @param array $operation 작업 정의
     * @return mixed 작업 결과
     */
    private function executeOperation($operation)
    {
        $type = $operation['type'];
        $params = $operation['params'];

        switch ($type) {
            case 'insert_warning':
                return $this->insertWarningData($params['data_type'], $params['data']);

            case 'update_completed':
                return $this->updateCompletedStatus($params['data_type'], $params['mac'], $params['idx'], $params['today']);

            case 'insert_oee':
                return $this->saveOeeData($params['data']);

            default:
                throw new Exception("Unknown operation type: " . $type);
        }
    }

    /**
     * 캐시 초기화 함수
     * DatabaseHelper의 캐시를 초기화
     */
    public function clearCache()
    {
        $this->dbHelper->clearCache();
    }

    /**
     * 현재 시간 기준 표준화된 날짜/시간 반환
     * 
     * @param string $format 날짜 형식 (기본: 'Y-m-d H:i:s')
     * @return string 현재 시간
     */
    public function getCurrentTime($format = 'Y-m-d H:i:s')
    {
        return date($format);
    }

    /**
     * API 응답 시간 측정을 위한 헬퍼 함수들
     */
    private $start_time;

    public function startTimer()
    {
        $this->start_time = microtime(true);
    }

    public function getExecutionTime()
    {
        if ($this->start_time) {
            return round((microtime(true) - $this->start_time) * 1000, 2); // 밀리초 단위
        }
        return 0;
    }

    /**
     * 디버그 정보를 포함한 응답 생성
     * 개발 환경에서만 사용
     * 
     * @param string $code 응답 코드
     * @param string $msg 메시지
     * @param array $data 데이터
     * @param bool $include_debug 디버그 정보 포함 여부
     * @return array 응답 배열
     */
    public function createDebugResponse($code, $msg, $data = null, $include_debug = false)
    {
        $response = $this->createResponse($code, $msg, $data);

        if ($include_debug && ($_ENV['APP_DEBUG'] ?? false)) {
            $response['debug'] = [
                'execution_time_ms' => $this->getExecutionTime(),
                'memory_usage_mb' => round(memory_get_usage(true) / 1024 / 1024, 2),
                'peak_memory_mb' => round(memory_get_peak_usage(true) / 1024 / 1024, 2),
                'timestamp' => $this->getCurrentTime()
            ];
        }

        return $response;
    }
}
