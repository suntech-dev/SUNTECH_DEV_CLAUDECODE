-- ST500 LockMaker PWA 신규 DB 스키마
-- DB: suntech_st500
-- 생성일: 2026-04-17
-- MySQL 5.7.44 (Laragon)
--
-- 기존 테이블 참고:
--   data_smart_device  → 구 Android 앱 디바이스 테이블 (MyISAM, 레거시)
--   st500_device       → 동일 구조 복사본
--   st500_logs         → 구 API 로그
--
-- 신규 테이블: lm_device, lm_logs
-- 기존 테이블은 건드리지 않고 병행 운영

USE suntech_st500;

-- --------------------------------------------------
-- 디바이스 테이블 (신규 PWA용)
-- 기존 data_smart_device 대비 변경점:
--   - device_id: VARCHAR(36) — UUID v4 고정 길이
--   - brand/model/version 제거 — 웹앱에서 불필요
--   - state: Y=승인, N=대기, D=삭제(비활성)
--   - ENGINE: InnoDB (트랜잭션 지원)
--   - CHARSET: utf8mb4 (이모지 포함 완전한 유니코드)
-- --------------------------------------------------
CREATE TABLE IF NOT EXISTS `lm_device` (
    `idx`         INT(11)      NOT NULL AUTO_INCREMENT COMMENT '일련번호',
    `device_id`   VARCHAR(36)  NOT NULL COMMENT 'UUID v4 — 브라우저 crypto.randomUUID()',
    `name`        VARCHAR(100) NOT NULL DEFAULT '' COMMENT '사용자 이름',
    `state`       ENUM('Y','N','D') NOT NULL DEFAULT 'N' COMMENT 'Y:승인, N:대기, D:삭제',
    `reg_date`    DATETIME     NOT NULL COMMENT '최초 등록일시',
    `update_date` DATETIME     DEFAULT NULL COMMENT '마지막 수정일시',
    PRIMARY KEY (`idx`),
    UNIQUE KEY `uq_device_id` (`device_id`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4 COMMENT='LockMaker PWA 디바이스';

-- --------------------------------------------------
-- API 로그 테이블 (신규)
-- reg_ip: VARCHAR(45) — IPv6 대응 (기존 VARCHAR(20)은 IPv6 불가)
-- --------------------------------------------------
CREATE TABLE IF NOT EXISTS `lm_logs` (
    `idx`        INT(11)      NOT NULL AUTO_INCREMENT COMMENT '일련번호',
    `code`       VARCHAR(50)  DEFAULT NULL COMMENT 'API 코드 (send_device, get_device 등)',
    `log_data`   TEXT         COMMENT '요청 파라미터 (JSON)',
    `log_result` TEXT         COMMENT '응답 결과 (JSON)',
    `reg_ip`     VARCHAR(45)  DEFAULT NULL COMMENT '요청 클라이언트 IP',
    `reg_date`   DATETIME     NOT NULL COMMENT '로그 기록 일시',
    PRIMARY KEY (`idx`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4 COMMENT='LockMaker PWA API 로그';
