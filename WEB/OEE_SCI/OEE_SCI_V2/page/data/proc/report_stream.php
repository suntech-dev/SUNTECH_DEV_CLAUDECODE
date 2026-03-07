<?php
/* ===============================
   AI Report SSE Stream API
   실시간 AI 분석 데이터 스트리밍
   =============================== */

require_once(__DIR__ . '/../../../lib/config.php');
require_once(__DIR__ . '/../../../lib/db.php');
require_once(__DIR__ . '/../../../lib/api_helper.lib.php');
require_once(__DIR__ . '/../../../lib/worktime.lib.php');

class AIReportStreamer {
    private $pdo;
    private $filters;
    private $startTime;
    private $maxExecutionTime = 3600; // 1시간
    
    public function __construct() {
        // 데이터베이스 연결 (db.php의 전역 $pdo 사용)
        global $pdo;
        $this->pdo = $pdo;
        
        // 필터 파라미터 수집
        $this->filters = $this->collectFilters();
        
        // SSE 설정
        $this->setupSSE();
        
        // 시작 시간 기록
        $this->startTime = time();
    }
    
    private function collectFilters() {
        return [
            'factory' => $_GET['factory'] ?? '',
            'line' => $_GET['line'] ?? '',
            'machine' => $_GET['machine'] ?? '',
            'timeRange' => $_GET['timeRange'] ?? 'today',
            'dateRange' => $_GET['dateRange'] ?? '',
            'shift' => $_GET['shift'] ?? '',
            'performance' => $_GET['performance'] ?? '',
            'defectRate' => $_GET['defectRate'] ?? '',
            'downtime' => $_GET['downtime'] ?? '',
            'granularity' => $_GET['granularity'] ?? 'hourly'
        ];
    }
    
    private function setupSSE() {
        // SSE 헤더 설정
        header('Content-Type: text/event-stream');
        header('Cache-Control: no-cache');
        header('Connection: keep-alive');
        header('Access-Control-Allow-Origin: *');
        header('Access-Control-Allow-Methods: GET');
        header('Access-Control-Allow-Headers: Cache-Control');
        
        // 출력 버퍼링 비활성화
        if (ob_get_level()) ob_end_clean();
        ob_implicit_flush(true);
        
        // PHP 실행 시간 제한 해제
        set_time_limit(0);
        ini_set('max_execution_time', $this->maxExecutionTime);
    }
    
    public function streamAIData() {
        $this->sendSSEData('connected', ['message' => 'AI Report Stream connected']);
        
        $lastDataHash = '';
        $heartbeatCounter = 0;
        
        while (true) {
            // 실행 시간 체크
            if (time() - $this->startTime > $this->maxExecutionTime) {
                $this->sendSSEData('timeout', ['message' => 'Maximum execution time reached']);
                break;
            }
            
            // 클라이언트 연결 상태 체크
            if (connection_aborted()) {
                $this->sendSSEData('disconnected', ['message' => 'Client disconnected']);
                break;
            }
            
            try {
                // 통합 데이터 수집
                $integratedData = $this->getIntegratedData();
                
                // AI 분석 수행
                $aiAnalysis = $this->performAIAnalysis($integratedData);
                
                // 예측 데이터 계산
                $predictions = $this->calculatePredictions($integratedData);
                
                // 이상치 탐지
                $anomalies = $this->detectAnomalies($integratedData);
                
                // 상관관계 분석
                $correlations = $this->analyzeCorrelations($integratedData);
                
                // 추천사항 생성
                $recommendations = $this->generateRecommendations($aiAnalysis);
                
                // 응답 데이터 구성
                $responseData = [
                    'timestamp' => date('Y-m-d H:i:s'),
                    'filters' => $this->filters,
                    'integrated_stats' => $integratedData['stats'],
                    'ai_insights' => $aiAnalysis,
                    'predictions' => $predictions,
                    'anomalies' => $anomalies,
                    'correlations' => $correlations,
                    'recommendations' => $recommendations,
                    'heatmap_data' => $this->generateHeatMapData($integratedData),
                    'trend_data' => $this->generateTrendData($integratedData),
                    'correlation_matrix' => $this->generateCorrelationMatrix($correlations),
                    'pareto_data' => $this->generateParetoData($integratedData),
                    'integrated_data' => $integratedData['table_data'] ?? []
                ];
                
                // 데이터 변화 감지
                $currentDataHash = md5(json_encode($responseData));
                
                if ($currentDataHash !== $lastDataHash) {
                    $this->sendSSEData('ai_data', $responseData);
                    $lastDataHash = $currentDataHash;
                    $heartbeatCounter = 0;
                } else {
                    // 데이터 변화가 없으면 heartbeat 전송 (15초마다)
                    $heartbeatCounter++;
                    if ($heartbeatCounter >= 3) {
                        $this->sendSSEData('heartbeat', ['timestamp' => date('Y-m-d H:i:s')]);
                        $heartbeatCounter = 0;
                    }
                }
                
            } catch (Exception $e) {
                error_log("AI Report Stream error: " . $e->getMessage());
                $this->sendSSEData('error', ['message' => 'Data processing error: ' . $e->getMessage()]);
            }
            
            // 5초 대기
            sleep(5);
        }
    }
    
    private function getIntegratedData() {
        $dateRange = $this->calculateDateRange();
        $whereConditions = $this->buildWhereConditions($dateRange);
        
        $stats = [
            'oee_stats' => $this->getOEEStats($whereConditions),
            'andon_stats' => $this->getAndonStats($whereConditions),
            'defective_stats' => $this->getDefectiveStats($whereConditions),
            'downtime_stats' => $this->getDowntimeStats($whereConditions)
        ];
        
        $tableData = $this->getIntegratedTableData($whereConditions);
        
        return [
            'stats' => $stats,
            'table_data' => $tableData,
            'date_range' => $dateRange
        ];
    }
    
    private function calculateDateRange() {
        if (!empty($this->filters['dateRange'])) {
            $dates = explode(',', $this->filters['dateRange']);
            return [
                'start_date' => $dates[0],
                'end_date' => $dates[1] ?? $dates[0]
            ];
        }
        
        // timeRange에 따른 날짜 계산
        switch ($this->filters['timeRange']) {
            case '1h':
                return [
                    'start_date' => date('Y-m-d H:i:s', strtotime('-1 hour')),
                    'end_date' => date('Y-m-d H:i:s'),
                    'is_hourly' => true
                ];
            case '4h':
                return [
                    'start_date' => date('Y-m-d H:i:s', strtotime('-4 hours')),
                    'end_date' => date('Y-m-d H:i:s'),
                    'is_hourly' => true
                ];
            case '8h':
                return [
                    'start_date' => date('Y-m-d H:i:s', strtotime('-8 hours')),
                    'end_date' => date('Y-m-d H:i:s'),
                    'is_hourly' => true
                ];
            case 'today':
                return [
                    'start_date' => date('Y-m-d 00:00:00'),
                    'end_date' => date('Y-m-d 23:59:59')
                ];
            case 'yesterday':
                return [
                    'start_date' => date('Y-m-d 00:00:00', strtotime('-1 day')),
                    'end_date' => date('Y-m-d 23:59:59', strtotime('-1 day'))
                ];
            case '1w':
                return [
                    'start_date' => date('Y-m-d 00:00:00', strtotime('-7 days')),
                    'end_date' => date('Y-m-d 23:59:59')
                ];
            case '1m':
                return [
                    'start_date' => date('Y-m-d 00:00:00', strtotime('-30 days')),
                    'end_date' => date('Y-m-d 23:59:59')
                ];
            default:
                return [
                    'start_date' => date('Y-m-d 00:00:00'),
                    'end_date' => date('Y-m-d 23:59:59')
                ];
        }
    }
    
    private function buildWhereConditions($dateRange) {
        $conditions = [];
        $params = [];
        
        // 날짜 범위 조건
        if (isset($dateRange['is_hourly']) && $dateRange['is_hourly']) {
            $conditions[] = "reg_date >= ? AND reg_date <= ?";
        } else {
            $conditions[] = "work_date >= ? AND work_date <= ?";
        }
        $params[] = $dateRange['start_date'];
        $params[] = $dateRange['end_date'];
        
        // 공장 필터
        if (!empty($this->filters['factory'])) {
            $conditions[] = "factory_idx = ?";
            $params[] = $this->filters['factory'];
        }
        
        // 라인 필터
        if (!empty($this->filters['line'])) {
            $conditions[] = "line_idx = ?";
            $params[] = $this->filters['line'];
        }
        
        // 기계 필터
        if (!empty($this->filters['machine'])) {
            $conditions[] = "machine_idx = ?";
            $params[] = $this->filters['machine'];
        }
        
        // 교대 필터
        if (!empty($this->filters['shift'])) {
            $conditions[] = "shift_idx = ?";
            $params[] = $this->filters['shift'];
        }
        
        return [
            'conditions' => $conditions,
            'params' => $params,
            'where_clause' => implode(' AND ', $conditions)
        ];
    }
    
    private function getOEEStats($whereConditions) {
        try {
            $sql = "SELECT 
                        COUNT(*) as total_count,
                        AVG(oee) as avg_oee,
                        AVG(availabilty_rate) as avg_availability,
                        AVG(productivity_rate) as avg_performance,
                        AVG(quality_rate) as avg_quality,
                        MAX(oee) as max_oee,
                        MIN(oee) as min_oee,
                        COUNT(DISTINCT machine_idx) as active_machines
                    FROM data_oee";
            
            if (!empty($whereConditions['where_clause'])) {
                $sql .= " WHERE " . $whereConditions['where_clause'];
            }
            
            $stmt = $this->pdo->prepare($sql);
            $stmt->execute($whereConditions['params']);
            $result = $stmt->fetch(PDO::FETCH_ASSOC);
            
            // 성능 등급 계산
            $gradeStats = $this->calculatePerformanceGrades($whereConditions);
            
            return array_merge($result ?: [], $gradeStats);
            
        } catch (PDOException $e) {
            error_log("OEE stats query error: " . $e->getMessage());
            return [];
        }
    }
    
    private function getAndonStats($whereConditions) {
        try {
            $sql = "SELECT 
                        COUNT(*) as total_count,
                        COUNT(CASE WHEN status = 'Warning' THEN 1 END) as warning_count,
                        COUNT(CASE WHEN status = 'Completed' THEN 1 END) as completed_count,
                        COUNT(DISTINCT machine_idx) as affected_machines,
                        AVG(duration_sec) as avg_duration
                    FROM data_andon";
            
            if (!empty($whereConditions['where_clause'])) {
                $sql .= " WHERE " . $whereConditions['where_clause'];
            }
            
            $stmt = $this->pdo->prepare($sql);
            $stmt->execute($whereConditions['params']);
            
            return $stmt->fetch(PDO::FETCH_ASSOC) ?: [];
            
        } catch (PDOException $e) {
            error_log("Andon stats query error: " . $e->getMessage());
            return [];
        }
    }
    
    private function getDefectiveStats($whereConditions) {
        try {
            $sql = "SELECT 
                        COUNT(*) as total_count,
                        COUNT(CASE WHEN status = 'Warning' THEN 1 END) as warning_count,
                        COUNT(DISTINCT machine_idx) as affected_machines,
                        COUNT(DISTINCT defective_idx) as defective_types_used
                    FROM data_defective";
            
            if (!empty($whereConditions['where_clause'])) {
                $sql .= " WHERE " . $whereConditions['where_clause'];
            }
            
            $stmt = $this->pdo->prepare($sql);
            $stmt->execute($whereConditions['params']);
            
            return $stmt->fetch(PDO::FETCH_ASSOC) ?: [];
            
        } catch (PDOException $e) {
            error_log("Defective stats query error: " . $e->getMessage());
            return [];
        }
    }
    
    private function getDowntimeStats($whereConditions) {
        try {
            $sql = "SELECT 
                        COUNT(*) as total_count,
                        COUNT(CASE WHEN status = 'Warning' THEN 1 END) as warning_count,
                        COUNT(DISTINCT machine_idx) as affected_machines,
                        AVG(duration_sec) as avg_duration,
                        COUNT(CASE WHEN duration_sec > 1800 THEN 1 END) as long_downtimes_count
                    FROM data_downtime";
            
            if (!empty($whereConditions['where_clause'])) {
                $sql .= " WHERE " . $whereConditions['where_clause'];
            }
            
            $stmt = $this->pdo->prepare($sql);
            $stmt->execute($whereConditions['params']);
            
            return $stmt->fetch(PDO::FETCH_ASSOC) ?: [];
            
        } catch (PDOException $e) {
            error_log("Downtime stats query error: " . $e->getMessage());
            return [];
        }
    }
    
    private function calculatePerformanceGrades($whereConditions) {
        try {
            $sql = "SELECT 
                        COUNT(CASE WHEN oee >= 85 THEN 1 END) as excellent_count,
                        COUNT(CASE WHEN oee >= 70 AND oee < 85 THEN 1 END) as good_count,
                        COUNT(CASE WHEN oee >= 50 AND oee < 70 THEN 1 END) as fair_count,
                        COUNT(CASE WHEN oee < 50 THEN 1 END) as poor_count
                    FROM data_oee";
            
            if (!empty($whereConditions['where_clause'])) {
                $sql .= " WHERE " . $whereConditions['where_clause'];
            }
            
            $stmt = $this->pdo->prepare($sql);
            $stmt->execute($whereConditions['params']);
            
            return $stmt->fetch(PDO::FETCH_ASSOC) ?: [];
            
        } catch (PDOException $e) {
            error_log("Performance grades query error: " . $e->getMessage());
            return [];
        }
    }
    
    private function getIntegratedTableData($whereConditions) {
        try {
            $sql = "SELECT DISTINCT
                        m.machine_no,
                        COALESCE(oee.oee, 0) as oee,
                        COALESCE(oee.availabilty_rate, 0) as availability,
                        COALESCE(oee.productivity_rate, 0) as performance,
                        COALESCE(oee.quality_rate, 0) as quality,
                        (
                            COALESCE(andon_active.warning_count, 0) + 
                            COALESCE(defective_active.warning_count, 0) + 
                            COALESCE(downtime_active.warning_count, 0)
                        ) as active_issues,
                        CASE 
                            WHEN COALESCE(oee.oee, 0) >= 85 THEN 'excellent'
                            WHEN COALESCE(oee.oee, 0) >= 70 THEN 'good'
                            WHEN COALESCE(oee.oee, 0) >= 50 THEN 'fair'
                            ELSE 'poor'
                        END as status
                    FROM info_machine m
                    LEFT JOIN info_line l ON m.line_idx = l.idx
                    LEFT JOIN info_factory f ON l.factory_idx = f.idx
                    LEFT JOIN (
                        SELECT machine_idx, 
                               AVG(oee) as oee,
                               AVG(availabilty_rate) as availabilty_rate,
                               AVG(productivity_rate) as productivity_rate,
                               AVG(quality_rate) as quality_rate
                        FROM data_oee 
                        WHERE " . ($whereConditions['where_clause'] ?: '1=1') . "
                        GROUP BY machine_idx
                    ) oee ON m.idx = oee.machine_idx
                    LEFT JOIN (
                        SELECT machine_idx, COUNT(*) as warning_count
                        FROM data_andon 
                        WHERE status = 'Warning' AND " . ($whereConditions['where_clause'] ?: '1=1') . "
                        GROUP BY machine_idx
                    ) andon_active ON m.idx = andon_active.machine_idx
                    LEFT JOIN (
                        SELECT machine_idx, COUNT(*) as warning_count
                        FROM data_defective 
                        WHERE status = 'Warning' AND " . ($whereConditions['where_clause'] ?: '1=1') . "
                        GROUP BY machine_idx
                    ) defective_active ON m.idx = defective_active.machine_idx
                    LEFT JOIN (
                        SELECT machine_idx, COUNT(*) as warning_count
                        FROM data_downtime 
                        WHERE status = 'Warning' AND " . ($whereConditions['where_clause'] ?: '1=1') . "
                        GROUP BY machine_idx
                    ) downtime_active ON m.idx = downtime_active.machine_idx
                    WHERE m.status = 'active'";
            
            // 필터 조건 적용
            if (!empty($this->filters['factory'])) {
                $sql .= " AND f.idx = " . intval($this->filters['factory']);
            }
            if (!empty($this->filters['line'])) {
                $sql .= " AND l.idx = " . intval($this->filters['line']);
            }
            if (!empty($this->filters['machine'])) {
                $sql .= " AND m.idx = " . intval($this->filters['machine']);
            }
            
            $sql .= " ORDER BY m.machine_no LIMIT 100";
            
            $stmt = $this->pdo->prepare($sql);
            $stmt->execute();
            
            return $stmt->fetchAll(PDO::FETCH_ASSOC) ?: [];
            
        } catch (PDOException $e) {
            error_log("Integrated table data query error: " . $e->getMessage());
            return [];
        }
    }
    
    // ===============================
    // AI 분석 함수들
    // ===============================
    
    private function performAIAnalysis($integratedData) {
        $stats = $integratedData['stats'];
        
        return [
            'performance_score' => $this->calculateOverallPerformanceScore($stats),
            'efficiency_trends' => $this->analyzeTrends($stats),
            'bottleneck_analysis' => $this->identifyBottlenecks($stats),
            'optimization_potential' => $this->calculateOptimizationPotential($stats)
        ];
    }
    
    private function calculateOverallPerformanceScore($stats) {
        $oeeStats = $stats['oee_stats'] ?? [];
        $avgOee = floatval($oeeStats['avg_oee'] ?? 0);
        
        if ($avgOee >= 85) return 'Excellent';
        if ($avgOee >= 70) return 'Good';
        if ($avgOee >= 50) return 'Fair';
        return 'Poor';
    }
    
    private function analyzeTrends($stats) {
        // 간단한 트렌드 분석 (실제로는 더 복잡한 알고리즘 필요)
        $oeeStats = $stats['oee_stats'] ?? [];
        $avgOee = floatval($oeeStats['avg_oee'] ?? 0);
        
        return [
            'direction' => $avgOee > 75 ? 'Improving' : 'Declining',
            'confidence' => 'Medium',
            'period' => $this->filters['timeRange']
        ];
    }
    
    private function identifyBottlenecks($stats) {
        $bottlenecks = [];
        
        $andonStats = $stats['andon_stats'] ?? [];
        $defectiveStats = $stats['defective_stats'] ?? [];
        $downtimeStats = $stats['downtime_stats'] ?? [];
        
        // 안돈 문제가 많은 경우
        if (($andonStats['warning_count'] ?? 0) > 5) {
            $bottlenecks[] = [
                'type' => 'Andon Alerts',
                'severity' => 'High',
                'count' => $andonStats['warning_count']
            ];
        }
        
        // 불량률이 높은 경우
        if (($defectiveStats['warning_count'] ?? 0) > 3) {
            $bottlenecks[] = [
                'type' => 'Quality Issues',
                'severity' => 'Medium',
                'count' => $defectiveStats['warning_count']
            ];
        }
        
        // 비가동 시간이 긴 경우
        if (($downtimeStats['long_downtimes_count'] ?? 0) > 2) {
            $bottlenecks[] = [
                'type' => 'Extended Downtimes',
                'severity' => 'High',
                'count' => $downtimeStats['long_downtimes_count']
            ];
        }
        
        return $bottlenecks;
    }
    
    private function calculateOptimizationPotential($stats) {
        $oeeStats = $stats['oee_stats'] ?? [];
        $avgOee = floatval($oeeStats['avg_oee'] ?? 0);
        $maxOee = floatval($oeeStats['max_oee'] ?? 0);
        
        $potential = $maxOee - $avgOee;
        
        return [
            'potential_improvement' => round($potential, 1) . '%',
            'target_oee' => 85,
            'current_gap' => round(85 - $avgOee, 1) . '%'
        ];
    }
    
    private function calculatePredictions($integratedData) {
        $stats = $integratedData['stats'];
        $oeeStats = $stats['oee_stats'] ?? [];
        $avgOee = floatval($oeeStats['avg_oee'] ?? 0);
        
        // 간단한 예측 모델 (실제로는 더 정교한 머신러닝 모델 필요)
        $trend = rand(-5, 5) / 10; // 임시 트렌드
        
        return [
            'oee_forecast_7d' => round($avgOee + $trend, 1) . '%',
            'trend_7d' => $trend > 0 ? 'Improving ↗' : ($trend < 0 ? 'Declining ↘' : 'Stable →'),
            'oee_forecast_30d' => round($avgOee + ($trend * 4), 1) . '%',
            'trend_30d' => $trend > 0 ? 'Improving ↗' : ($trend < 0 ? 'Declining ↘' : 'Stable →'),
            'equipment_risk' => $avgOee < 60 ? 'High' : ($avgOee < 80 ? 'Medium' : 'Low'),
            'risk_trend' => 'Stable'
        ];
    }
    
    private function detectAnomalies($integratedData) {
        $anomalies = [];
        $stats = $integratedData['stats'];
        
        // OEE 이상치 감지
        $oeeStats = $stats['oee_stats'] ?? [];
        $avgOee = floatval($oeeStats['avg_oee'] ?? 0);
        $minOee = floatval($oeeStats['min_oee'] ?? 0);
        
        if ($minOee < 30 && $avgOee > 70) {
            $anomalies[] = [
                'type' => 'OEE Anomaly',
                'description' => 'Extremely low OEE detected on some machines',
                'severity' => 'High',
                'timestamp' => date('Y-m-d H:i:s')
            ];
        }
        
        // 안돈 이상 증가 감지
        $andonStats = $stats['andon_stats'] ?? [];
        if (($andonStats['warning_count'] ?? 0) > 10) {
            $anomalies[] = [
                'type' => 'Andon Spike',
                'description' => 'Unusual increase in Andon alerts',
                'severity' => 'Medium',
                'timestamp' => date('Y-m-d H:i:s')
            ];
        }
        
        return $anomalies;
    }
    
    private function analyzeCorrelations($integratedData) {
        // 간단한 상관관계 분석 (실제로는 더 정교한 통계 분석 필요)
        return [
            'oee_defect' => round(rand(-80, -30) / 100, 2), // OEE와 불량률의 부정적 상관관계
            'availability_downtime' => round(rand(-90, -70) / 100, 2), // 가용성과 비가동의 부정적 상관관계  
            'performance_quality' => round(rand(30, 70) / 100, 2) // 성능과 품질의 정적 상관관계
        ];
    }
    
    private function generateRecommendations($aiAnalysis) {
        $recommendations = [];
        
        $performanceScore = $aiAnalysis['performance_score'] ?? 'Fair';
        $bottlenecks = $aiAnalysis['bottleneck_analysis'] ?? [];
        
        if ($performanceScore === 'Poor' || $performanceScore === 'Fair') {
            $recommendations[] = [
                'priority' => 'HIGH',
                'text' => 'Focus on improving machine availability and reducing unplanned downtime'
            ];
        }
        
        foreach ($bottlenecks as $bottleneck) {
            if ($bottleneck['type'] === 'Extended Downtimes') {
                $recommendations[] = [
                    'priority' => 'HIGH',
                    'text' => 'Implement predictive maintenance to reduce extended downtimes'
                ];
            }
            
            if ($bottleneck['type'] === 'Quality Issues') {
                $recommendations[] = [
                    'priority' => 'MED',
                    'text' => 'Review quality control processes and operator training'
                ];
            }
        }
        
        if (empty($recommendations)) {
            $recommendations[] = [
                'priority' => 'LOW',
                'text' => 'Continue monitoring current performance levels'
            ];
        }
        
        return $recommendations;
    }
    
    // ===============================
    // 차트 데이터 생성 함수들
    // ===============================
    
    private function generateHeatMapData($integratedData) {
        $heatmapData = [];
        $tableData = $integratedData['table_data'] ?? [];
        
        foreach ($tableData as $index => $row) {
            $heatmapData[] = [
                'x' => $index + 1,
                'y' => intval($row['machine_no'] ?? 1),
                'z' => floatval($row['oee'] ?? 0)
            ];
        }
        
        return $heatmapData;
    }
    
    private function generateTrendData($integratedData) {
        // 샘플 트렌드 데이터 (실제로는 시계열 데이터 조회 필요)
        $labels = [];
        $actualData = [];
        $predictedData = [];
        
        $baseOee = floatval($integratedData['stats']['oee_stats']['avg_oee'] ?? 75);
        
        for ($i = 0; $i < 24; $i++) {
            $labels[] = date('H:i', strtotime("-{$i} hours"));
            $actualData[] = $baseOee + rand(-10, 10);
            if ($i < 6) { // 미래 6시간 예측
                $predictedData[] = $baseOee + rand(-5, 5);
            } else {
                $predictedData[] = null;
            }
        }
        
        return [
            'labels' => array_reverse($labels),
            'actual' => array_reverse($actualData),
            'predicted' => array_reverse($predictedData)
        ];
    }
    
    private function generateCorrelationMatrix($correlations) {
        $matrix = [];
        $factors = ['OEE', 'Defects', 'Availability', 'Performance', 'Quality'];
        
        foreach ($factors as $i => $factorX) {
            foreach ($factors as $j => $factorY) {
                if ($factorX === 'OEE' && $factorY === 'Defects') {
                    $correlation = $correlations['oee_defect'] ?? -0.6;
                } elseif ($factorX === 'Availability' && $factorY === 'Downtime') {
                    $correlation = $correlations['availability_downtime'] ?? -0.8;
                } elseif ($factorX === 'Performance' && $factorY === 'Quality') {
                    $correlation = $correlations['performance_quality'] ?? 0.5;
                } elseif ($factorX === $factorY) {
                    $correlation = 1.0;
                } else {
                    $correlation = rand(-50, 50) / 100;
                }
                
                $matrix[] = [
                    'x' => $i,
                    'y' => $j,
                    'z' => $correlation
                ];
            }
        }
        
        return $matrix;
    }
    
    private function generateParetoData($integratedData) {
        // 샘플 파레토 데이터 (실제로는 문제 유형별 집계 필요)
        $categories = ['Material Shortage', 'Equipment Setup', 'Quality Check', 'Maintenance', 'Operator Training'];
        $frequency = [45, 32, 28, 15, 10];
        
        // 누적 비율 계산
        $total = array_sum($frequency);
        $cumulative = [];
        $runningTotal = 0;
        
        foreach ($frequency as $freq) {
            $runningTotal += $freq;
            $cumulative[] = round(($runningTotal / $total) * 100, 1);
        }
        
        return [
            'labels' => $categories,
            'frequency' => $frequency,
            'cumulative' => $cumulative
        ];
    }
    
    private function sendSSEData($eventType, $data) {
        $jsonData = json_encode($data, JSON_UNESCAPED_UNICODE);
        echo "event: {$eventType}\n";
        echo "data: {$jsonData}\n\n";
        flush();
    }
}

// AI Report Streamer 실행
try {
    $aiStreamer = new AIReportStreamer();
    $aiStreamer->streamAIData();
} catch (Exception $e) {
    error_log("AI Report Stream fatal error: " . $e->getMessage());
    echo "event: error\n";
    echo "data: " . json_encode(['message' => 'Stream initialization failed']) . "\n\n";
    flush();
}
?>