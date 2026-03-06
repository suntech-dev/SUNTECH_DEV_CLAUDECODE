<?php
/**
 * 데이터베이스 헬퍼 라이브러리
 * 성능 최적화된 쿼리와 공통 데이터베이스 작업을 제공합니다.
 * 
 * 주요 기능:
 * - 최적화된 SQL 쿼리 (SELECT * 사용 지양)
 * - 캐싱 기능
 * - 공통 CRUD 작업
 */

class DatabaseHelper 
{
  private $pdo;
  private $cache = [];
  private $cacheTimeout = 300; // 5분

  public function __construct($pdo) 
  {
    $this->pdo = $pdo;
  }

  /**
   * MAC 주소로 장비 정보 조회 (최적화된 쿼리)
   * 기존: SELECT * FROM info_machine
   * 개선: 필요한 컬럼만 선택적으로 조회
   * 
   * @param string $mac MAC 주소
   * @return array|null 장비 정보
   */
  public function getMachineByMac($mac) 
  {
    // 캐시 키 생성
    $cacheKey = "machine_" . $mac;
    
    // 캐시에서 먼저 확인
    if ($this->isCacheValid($cacheKey)) {
      return $this->cache[$cacheKey]['data'];
    }

    // 필요한 컬럼만 선택 (성능 향상)
    $sql = "SELECT 
              idx, factory_idx, line_idx, machine_no, 
              target, mac, ip, app_ver, update_date
            FROM info_machine 
            WHERE mac = ? 
            LIMIT 1";

    $stmt = $this->pdo->prepare($sql);
    $stmt->execute([$mac]);
    $result = $stmt->fetch(PDO::FETCH_ASSOC);

    // 결과를 캐시에 저장
    $this->setCache($cacheKey, $result);

    return $result;
  }

  /**
   * MAC 주소로 장비 정보와 공장/라인 정보를 한번에 조회 (JOIN 사용)
   * N+1 쿼리 문제 해결
   * 
   * @param string $mac MAC 주소
   * @return array|null 장비와 공장/라인 정보
   */
  public function getMachineWithFactoryAndLine($mac) 
  {
    $cacheKey = "machine_full_" . $mac;
    
    if ($this->isCacheValid($cacheKey)) {
      return $this->cache[$cacheKey]['data'];
    }

    // JOIN을 사용해서 한 번의 쿼리로 모든 정보 가져오기 (성능 최적화)
    $sql = "SELECT 
              m.idx, m.factory_idx, m.line_idx, 
              m.machine_no, m.target, m.mac, m.ip, m.app_ver,
              f.factory_name,
              l.line_name, l.line_target, l.mp
            FROM info_machine m
            LEFT JOIN info_factory f ON f.idx = m.factory_idx
            LEFT JOIN info_line l ON l.idx = m.line_idx  
            WHERE m.mac = ? 
            LIMIT 1";

    $stmt = $this->pdo->prepare($sql);
    $stmt->execute([$mac]);
    $result = $stmt->fetch(PDO::FETCH_ASSOC);

    $this->setCache($cacheKey, $result);

    return $result;
  }

  /**
   * OEE 계산을 위한 downtime과 defective 데이터를 한번에 조회
   * 기존: 2개의 별도 쿼리 → 1개의 통합 쿼리로 성능 향상
   * 
   * @param string $mac MAC 주소
   * @param string $workDate 작업 날짜
   * @param int $shiftIdx 교대 인덱스
   * @return array downtime과 defective 집계 데이터
   */
  public function getOeeCalculationData($mac, $workDate, $shiftIdx) 
  {
    $cacheKey = "oee_calc_{$mac}_{$workDate}_{$shiftIdx}";
    
    // OEE 계산 데이터는 짧은 캐시 시간 사용 (1분)
    if ($this->isCacheValid($cacheKey, 60)) {
      return $this->cache[$cacheKey]['data'];
    }

    // 서브쿼리를 사용해서 한 번의 쿼리로 모든 집계 데이터 가져오기
    $sql = "SELECT 
              (SELECT COALESCE(SUM(duration_sec), 0) 
               FROM data_downtime 
               WHERE mac = ? AND work_date = ? AND shift_idx = ?) as downtime_duration_sum,
              (SELECT COALESCE(COUNT(*), 0) 
               FROM data_defective 
               WHERE mac = ? AND work_date = ? AND shift_idx = ?) as defective_count";

    $stmt = $this->pdo->prepare($sql);
    $stmt->execute([$mac, $workDate, $shiftIdx, $mac, $workDate, $shiftIdx]);
    $result = $stmt->fetch(PDO::FETCH_ASSOC);

    // 결과가 null인 경우 기본값 설정
    if (!$result) {
      $result = [
        'downtime_duration_sum' => 0,
        'defective_count' => 0
      ];
    }

    $this->setCache($cacheKey, $result, 60);

    return $result;
  }

  /**
   * 디자인 공정 정보 조회 (최적화)
   *
   * @param int $designProcessIdx 디자인 공정 인덱스
   * @return array|null 디자인 공정 정보
   */
  public function getDesignProcessInfo($designProcessIdx)
  {
    $cacheKey = "design_process_" . $designProcessIdx;

    // 디자인 정보는 자주 변경되지 않으므로 긴 캐시 시간 사용 (10분)
    if ($this->isCacheValid($cacheKey, 600)) {
      return $this->cache[$cacheKey]['data'];
    }

    $sql = "SELECT
              std_mc_needed, design_process
            FROM info_design_process
            WHERE idx = ? AND status = 'Y'
            LIMIT 1";

    $stmt = $this->pdo->prepare($sql);
    $stmt->execute([$designProcessIdx]);
    $result = $stmt->fetch(PDO::FETCH_ASSOC);

    $this->setCache($cacheKey, $result, 600);

    return $result;
  }

  /**
   * 안돈 정보 조회 (최적화)
   * 
   * @param int $andonIdx 안돈 인덱스
   * @return array|null 안돈 정보
   */
  public function getAndonInfo($andonIdx) 
  {
    $cacheKey = "andon_info_" . $andonIdx;
    
    if ($this->isCacheValid($cacheKey, 600)) {
      return $this->cache[$cacheKey]['data'];
    }

    $sql = "SELECT 
              idx, andon_name, status
            FROM info_andon 
            WHERE idx = ? AND status = 'Y'
            LIMIT 1";

    $stmt = $this->pdo->prepare($sql);
    $stmt->execute([$andonIdx]);
    $result = $stmt->fetch(PDO::FETCH_ASSOC);

    $this->setCache($cacheKey, $result, 600);

    return $result;
  }

  /**
   * 비가동 정보 조회 (최적화)
   * 
   * @param int $downtimeIdx 비가동 인덱스
   * @return array|null 비가동 정보
   */
  public function getDowntimeInfo($downtimeIdx) 
  {
    $cacheKey = "downtime_info_" . $downtimeIdx;
    
    if ($this->isCacheValid($cacheKey, 600)) {
      return $this->cache[$cacheKey]['data'];
    }

    $sql = "SELECT 
              idx, downtime_name, downtime_shortcut, status
            FROM info_downtime 
            WHERE idx = ? AND status = 'Y'
            LIMIT 1";

    $stmt = $this->pdo->prepare($sql);
    $stmt->execute([$downtimeIdx]);
    $result = $stmt->fetch(PDO::FETCH_ASSOC);

    $this->setCache($cacheKey, $result, 600);

    return $result;
  }

  /**
   * 불량 정보 조회 (최적화)
   * 
   * @param int $defectiveIdx 불량 인덱스
   * @return array|null 불량 정보
   */
  public function getDefectiveInfo($defectiveIdx) 
  {
    $cacheKey = "defective_info_" . $defectiveIdx;
    
    if ($this->isCacheValid($cacheKey, 600)) {
      return $this->cache[$cacheKey]['data'];
    }

    $sql = "SELECT 
              idx, defective_name, defective_shortcut, status
            FROM info_defective 
            WHERE idx = ? AND status = 'Y'
            LIMIT 1";

    $stmt = $this->pdo->prepare($sql);
    $stmt->execute([$defectiveIdx]);
    $result = $stmt->fetch(PDO::FETCH_ASSOC);

    $this->setCache($cacheKey, $result, 600);

    return $result;
  }

  /**
   * 진행 중인 안돈/비가동 찾기 (최적화된 쿼리)
   * 
   * @param string $mac MAC 주소
   * @param int $idx 안돈 또는 비가동 인덱스
   * @param string $table 테이블명 ('data_andon' 또는 'data_downtime')
   * @return array|null 진행 중인 데이터
   */
  public function findActiveRecord($mac, $idx, $table) 
  {
    // 테이블명 검증 (보안)
    $allowedTables = ['data_andon', 'data_downtime'];
    if (!in_array($table, $allowedTables)) {
      throw new InvalidArgumentException('허용되지 않은 테이블명입니다.');
    }

    $idxColumn = ($table === 'data_andon') ? 'andon_idx' : 'downtime_idx';

    $sql = "SELECT 
              idx, reg_date, status
            FROM `{$table}` 
            WHERE mac = ? AND {$idxColumn} = ? AND status = 'Warning' 
            ORDER BY idx ASC 
            LIMIT 1";

    $stmt = $this->pdo->prepare($sql);
    $stmt->execute([$mac, $idx]);
    
    return $stmt->fetch(PDO::FETCH_ASSOC);
  }

  /**
   * 캐시 설정
   * 
   * @param string $key 캐시 키
   * @param mixed $data 캐시할 데이터
   * @param int $timeout 캐시 유지 시간 (초)
   */
  private function setCache($key, $data, $timeout = null) 
  {
    $timeout = $timeout ?? $this->cacheTimeout;
    
    $this->cache[$key] = [
      'data' => $data,
      'timestamp' => time(),
      'timeout' => $timeout
    ];
  }

  /**
   * 캐시 유효성 확인
   * 
   * @param string $key 캐시 키
   * @param int $timeout 캐시 유지 시간 (초)
   * @return bool 캐시가 유효하면 true
   */
  private function isCacheValid($key, $timeout = null) 
  {
    if (!isset($this->cache[$key])) {
      return false;
    }

    $cache = $this->cache[$key];
    $cacheTimeout = $timeout ?? $cache['timeout'];
    
    return (time() - $cache['timestamp']) < $cacheTimeout;
  }

  /**
   * 캐시 삭제
   * 
   * @param string $key 캐시 키 (null이면 전체 캐시 삭제)
   */
  public function clearCache($key = null) 
  {
    if ($key === null) {
      $this->cache = [];
    } else {
      unset($this->cache[$key]);
    }
  }

  /**
   * 안전한 INSERT 쿼리 실행
   * 
   * @param string $table 테이블명
   * @param array $data 삽입할 데이터 (컬럼명 => 값)
   * @return bool|int 성공하면 INSERT ID, 실패하면 false
   */
  public function insert($table, $data) 
  {
    if (empty($data)) {
      return false;
    }

    // 컬럼명과 값 분리
    $columns = array_keys($data);
    $values = array_values($data);
    $placeholders = str_repeat('?,', count($columns) - 1) . '?';

    // 컬럼명 이스케이프 (보안)
    $escapedColumns = array_map(function($col) {
      return "`{$col}`";
    }, $columns);

    $sql = "INSERT INTO `{$table}` (" . implode(', ', $escapedColumns) . ") VALUES ({$placeholders})";

    try {
      $stmt = $this->pdo->prepare($sql);
      $result = $stmt->execute($values);
      
      if ($result) {
        return $this->pdo->lastInsertId();
      }
      
      return false;
    } catch (PDOException $e) {
      error_log("Database INSERT error: " . $e->getMessage());
      return false;
    }
  }

  /**
   * 안전한 UPDATE 쿼리 실행
   * 
   * @param string $table 테이블명
   * @param array $data 업데이트할 데이터 (컬럼명 => 값)
   * @param array $where WHERE 조건 (컬럼명 => 값)
   * @return bool 성공하면 true
   */
  public function update($table, $data, $where) 
  {
    if (empty($data) || empty($where)) {
      return false;
    }

    // SET 절 생성
    $setParts = [];
    $setValues = [];
    foreach ($data as $column => $value) {
      $setParts[] = "`{$column}` = ?";
      $setValues[] = $value;
    }

    // WHERE 절 생성
    $whereParts = [];
    $whereValues = [];
    foreach ($where as $column => $value) {
      $whereParts[] = "`{$column}` = ?";
      $whereValues[] = $value;
    }

    $sql = "UPDATE `{$table}` SET " . implode(', ', $setParts) . " WHERE " . implode(' AND ', $whereParts);

    try {
      $stmt = $this->pdo->prepare($sql);
      return $stmt->execute(array_merge($setValues, $whereValues));
    } catch (PDOException $e) {
      error_log("Database UPDATE error: " . $e->getMessage());
      return false;
    }
  }

  /**
   * 배치 INSERT (대량 데이터 삽입 시 성능 향상)
   * 
   * @param string $table 테이블명
   * @param array $columns 컬럼명 배열
   * @param array $dataRows 데이터 행 배열
   * @param int $batchSize 배치 크기 (기본값: 100)
   * @return bool 성공하면 true
   */
  public function batchInsert($table, $columns, $dataRows, $batchSize = 100) 
  {
    if (empty($columns) || empty($dataRows)) {
      return false;
    }

    $escapedColumns = array_map(function($col) {
      return "`{$col}`";
    }, $columns);

    $columnCount = count($columns);
    $placeholderRow = '(' . str_repeat('?,', $columnCount - 1) . '?)';

    try {
      $this->pdo->beginTransaction();

      $totalRows = count($dataRows);
      for ($i = 0; $i < $totalRows; $i += $batchSize) {
        $batch = array_slice($dataRows, $i, $batchSize);
        $placeholders = str_repeat($placeholderRow . ',', count($batch) - 1) . $placeholderRow;
        
        $sql = "INSERT INTO `{$table}` (" . implode(', ', $escapedColumns) . ") VALUES {$placeholders}";
        
        $stmt = $this->pdo->prepare($sql);
        $stmt->execute(array_merge(...$batch));
      }

      $this->pdo->commit();
      return true;
    } catch (PDOException $e) {
      $this->pdo->rollback();
      error_log("Batch INSERT error: " . $e->getMessage());
      return false;
    }
  }
}