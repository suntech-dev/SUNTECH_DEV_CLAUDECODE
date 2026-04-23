-- ============================================================
-- 자수기(EMB) 전용 테이블 3개 생성 DDL
-- OEE_SCI_V2 — data_oee_emb / data_oee_rows_emb / data_oee_rows_hourly_emb
-- 실행 환경: 라라곤(개발) 및 운영 서버 공통
-- ============================================================

-- 1. data_oee_emb (일별 UPSERT — 교대별 자수기 집계)
CREATE TABLE IF NOT EXISTS `data_oee_emb` (
  `idx`              int(11)      NOT NULL AUTO_INCREMENT,
  `work_date`        date         NOT NULL                     COMMENT '작업일',
  `time_update`      time         DEFAULT NULL                 COMMENT '마지막 갱신 시각',
  `shift_idx`        tinyint(4)   DEFAULT NULL                 COMMENT '근무조',
  `factory_idx`      int(11)      DEFAULT NULL                 COMMENT '공장 idx',
  `factory_name`     varchar(100) DEFAULT NULL                 COMMENT '공장명',
  `line_idx`         int(11)      DEFAULT NULL                 COMMENT '라인 idx',
  `line_name`        varchar(100) DEFAULT NULL                 COMMENT '라인명',
  `mac`              varchar(30)  DEFAULT NULL                 COMMENT 'MAC 주소',
  `machine_idx`      int(11)      DEFAULT NULL                 COMMENT '기계 idx',
  `machine_no`       varchar(50)  DEFAULT NULL                 COMMENT '기계 번호',
  `process_name`     varchar(100) DEFAULT NULL                 COMMENT '공정명',
  `planned_work_time` int(11)     DEFAULT 0                   COMMENT '계획 근무시간(초)',
  `runtime`          int(11)      DEFAULT 0                    COMMENT '경과 근무시간(초)',
  `actual_output`    int(11)      DEFAULT 0                    COMMENT '누적 생산량',
  `cycle_time`       int(11)      DEFAULT 0                    COMMENT '마지막 수신 CT(초)',
  `thread_breakage`  int(11)      DEFAULT 0                    COMMENT '누적 실끊김 횟수',
  `motor_run_time`   int(11)      DEFAULT 0                    COMMENT '누적 모터동작시간(초)',
  `pair_info`        int(11)      DEFAULT 0                    COMMENT '예비(자수기 현재 미사용)',
  `pair_count`       int(11)      DEFAULT 0                    COMMENT '예비(자수기 현재 미사용)',
  `work_hour`        tinyint(4)   DEFAULT NULL                 COMMENT '마지막 갱신 시각대(0~23)',
  `reg_date`         datetime     DEFAULT NULL                 COMMENT '최초 등록 일시',
  `update_date`      datetime     DEFAULT NULL                 COMMENT '최종 수정 일시',
  PRIMARY KEY (`idx`),
  UNIQUE KEY `uk_emb_daily` (`mac`,`work_date`,`shift_idx`,`process_name`),
  KEY `idx_work_date`  (`work_date`),
  KEY `idx_mac`        (`mac`),
  KEY `idx_factory`    (`factory_idx`),
  KEY `idx_line`       (`line_idx`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4 COMMENT='자수기 전용 일별 집계';

-- 2. data_oee_rows_emb (패킷 수신 때마다 INSERT — 스냅샷 이력)
CREATE TABLE IF NOT EXISTS `data_oee_rows_emb` (
  `idx`              int(11)      NOT NULL AUTO_INCREMENT,
  `work_date`        date         NOT NULL                     COMMENT '작업일',
  `time_update`      time         DEFAULT NULL                 COMMENT '수신 시각',
  `shift_idx`        tinyint(4)   DEFAULT NULL                 COMMENT '근무조',
  `factory_idx`      int(11)      DEFAULT NULL                 COMMENT '공장 idx',
  `factory_name`     varchar(100) DEFAULT NULL                 COMMENT '공장명',
  `line_idx`         int(11)      DEFAULT NULL                 COMMENT '라인 idx',
  `line_name`        varchar(100) DEFAULT NULL                 COMMENT '라인명',
  `mac`              varchar(30)  DEFAULT NULL                 COMMENT 'MAC 주소',
  `machine_idx`      int(11)      DEFAULT NULL                 COMMENT '기계 idx',
  `machine_no`       varchar(50)  DEFAULT NULL                 COMMENT '기계 번호',
  `process_name`     varchar(100) DEFAULT NULL                 COMMENT '공정명',
  `planned_work_time` int(11)     DEFAULT 0                   COMMENT '계획 근무시간(초)',
  `runtime`          int(11)      DEFAULT 0                    COMMENT '경과 근무시간(초)',
  `actual_output`    int(11)      DEFAULT 0                    COMMENT '이 시점 누적 생산량',
  `packet_qty`       int(11)      DEFAULT 0                    COMMENT '이번 패킷 수량',
  `cycle_time`       int(11)      DEFAULT 0                    COMMENT '이번 패킷 CT(초)',
  `thread_breakage`  int(11)      DEFAULT 0                    COMMENT '이번 패킷 실끊김 횟수',
  `motor_run_time`   int(11)      DEFAULT 0                    COMMENT '이번 패킷 모터동작시간(초)',
  `pair_info`        int(11)      DEFAULT 0                    COMMENT '예비(자수기 현재 미사용)',
  `pair_count`       int(11)      DEFAULT 0                    COMMENT '예비(자수기 현재 미사용)',
  `work_hour`        tinyint(4)   DEFAULT NULL                 COMMENT '수신 시각대(0~23)',
  `reg_date`         datetime     DEFAULT NULL                 COMMENT '등록 일시',
  PRIMARY KEY (`idx`),
  KEY `idx_work_date`  (`work_date`),
  KEY `idx_mac`        (`mac`),
  KEY `idx_factory`    (`factory_idx`),
  KEY `idx_line`       (`line_idx`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4 COMMENT='자수기 전용 패킷 스냅샷';

-- 3. data_oee_rows_hourly_emb (시간대별 UPSERT)
CREATE TABLE IF NOT EXISTS `data_oee_rows_hourly_emb` (
  `idx`              int(11)      NOT NULL AUTO_INCREMENT,
  `work_date`        date         NOT NULL                     COMMENT '작업일',
  `time_update`      time         DEFAULT NULL                 COMMENT '마지막 갱신 시각',
  `shift_idx`        tinyint(4)   DEFAULT NULL                 COMMENT '근무조',
  `work_hour`        tinyint(4)   NOT NULL                     COMMENT '시간대(0~23)',
  `factory_idx`      int(11)      DEFAULT NULL                 COMMENT '공장 idx',
  `factory_name`     varchar(100) DEFAULT NULL                 COMMENT '공장명',
  `line_idx`         int(11)      DEFAULT NULL                 COMMENT '라인 idx',
  `line_name`        varchar(100) DEFAULT NULL                 COMMENT '라인명',
  `mac`              varchar(30)  DEFAULT NULL                 COMMENT 'MAC 주소',
  `machine_idx`      int(11)      DEFAULT NULL                 COMMENT '기계 idx',
  `machine_no`       varchar(50)  DEFAULT NULL                 COMMENT '기계 번호',
  `process_name`     varchar(100) DEFAULT NULL                 COMMENT '공정명',
  `planned_work_time` int(11)     DEFAULT 0                   COMMENT '계획 근무시간(초)',
  `runtime`          int(11)      DEFAULT 0                    COMMENT '경과 근무시간(초)',
  `actual_output`    int(11)      DEFAULT 0                    COMMENT '해당 시간대 누적 생산량',
  `cycle_time`       int(11)      DEFAULT 0                    COMMENT '해당 시간대 마지막 CT(초)',
  `thread_breakage`  int(11)      DEFAULT 0                    COMMENT '해당 시간대 누적 실끊김',
  `motor_run_time`   int(11)      DEFAULT 0                    COMMENT '해당 시간대 누적 모터동작시간(초)',
  `pair_info`        int(11)      DEFAULT 0                    COMMENT '예비(자수기 현재 미사용)',
  `pair_count`       int(11)      DEFAULT 0                    COMMENT '예비(자수기 현재 미사용)',
  `reg_date`         datetime     DEFAULT NULL                 COMMENT '최초 등록 일시',
  `update_date`      datetime     DEFAULT NULL                 COMMENT '최종 수정 일시',
  PRIMARY KEY (`idx`),
  UNIQUE KEY `uk_emb_hourly` (`mac`,`work_date`,`shift_idx`,`process_name`,`work_hour`),
  KEY `idx_work_date`  (`work_date`),
  KEY `idx_mac`        (`mac`),
  KEY `idx_factory`    (`factory_idx`),
  KEY `idx_line`       (`line_idx`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4 COMMENT='자수기 전용 시간대별 집계';
