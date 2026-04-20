# 10. 데이터베이스 구조

> 최초 작성: 2026-04-17
> 마지막 업데이트: 2026-04-17

---

## 환경 정보

| 항목 | 값 |
|---|---|
| DBMS | **MySQL 5.7.44** (Laragon 내장) |
| 데이터베이스명 | `suntech_st500` |
| 접속 정보 (로컬) | host=localhost, user=root, password=(없음) |
| 스키마 파일 | `WEB/ST500_LOCKMAKER/api/sql/schema.sql` |

> ⚠️ MySQL **5.7.44** 버전 고정. 8.x 문법(JSON_TABLE 등) 사용 금지.

---

## 테이블 구성 요약

| 테이블 | 용도 | 엔진 | 상태 |
|---|---|---|---|
| `data_smart_device` | 구 Android 앱 디바이스 (레거시) | MyISAM | 보존, 신규 사용 안함 |
| `st500_device` | 위와 동일 구조 복사본 | - | 보존, 신규 사용 안함 |
| `st500_logs` | 구 API 로그 | - | 보존, 신규 사용 안함 |
| `lm_device` | **신규 PWA 디바이스** | InnoDB | 운영 중 |
| `lm_logs` | **신규 API 로그** | InnoDB | 운영 중 |

---

## lm_device (신규 디바이스 테이블)

```sql
CREATE TABLE IF NOT EXISTS `lm_device` (
    `idx`         INT(11)           NOT NULL AUTO_INCREMENT COMMENT '일련번호',
    `device_id`   VARCHAR(36)       NOT NULL COMMENT 'UUID v4 (브라우저 crypto.randomUUID())',
    `name`        VARCHAR(100)      NOT NULL DEFAULT '' COMMENT '사용자 이름',
    `state`       ENUM('Y','N','D') NOT NULL DEFAULT 'N' COMMENT 'Y:승인, N:대기, D:삭제',
    `reg_date`    DATETIME          NOT NULL COMMENT '최초 등록일시',
    `update_date` DATETIME          DEFAULT NULL COMMENT '마지막 수정일시',
    PRIMARY KEY (`idx`),
    UNIQUE KEY `uq_device_id` (`device_id`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4 COMMENT='LockMaker PWA 디바이스';
```

### 컬럼 설명

| 컬럼 | 타입 | 설명 |
|---|---|---|
| `idx` | INT AUTO_INCREMENT | PK |
| `device_id` | VARCHAR(36) UNIQUE | UUID v4 — `crypto.randomUUID()` 생성, localStorage 영구 저장 |
| `name` | VARCHAR(100) | 사용자가 입력한 이름 |
| `state` | ENUM | Y=승인(코드 생성 가능), N=대기(관리자 승인 전), D=삭제(비활성) |
| `reg_date` | DATETIME | 최초 등록 일시 |
| `update_date` | DATETIME | 이름 변경 등 수정 시 갱신 |

### state 흐름

```
신규 등록 → N(대기)
         → 관리자 DB 직접 UPDATE state='Y' → Y(승인) → 코드 생성 가능
         → 관리자 DB 직접 UPDATE state='D' → D(삭제) → API 거부
```

> 현재 관리자 웹 UI 없음 — phpMyAdmin 또는 MySQL CLI로 직접 승인 처리.

---

## lm_logs (신규 로그 테이블)

```sql
CREATE TABLE IF NOT EXISTS `lm_logs` (
    `idx`        INT(11)     NOT NULL AUTO_INCREMENT COMMENT '일련번호',
    `code`       VARCHAR(50) DEFAULT NULL COMMENT 'API 코드 (send_device, get_device 등)',
    `log_data`   TEXT        COMMENT '요청 파라미터 (JSON)',
    `log_result` TEXT        COMMENT '응답 결과 (JSON)',
    `reg_ip`     VARCHAR(45) DEFAULT NULL COMMENT '요청 클라이언트 IP (IPv6 대응)',
    `reg_date`   DATETIME    NOT NULL COMMENT '로그 기록 일시',
    PRIMARY KEY (`idx`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4 COMMENT='LockMaker PWA API 로그';
```

### 컬럼 설명

| 컬럼 | 타입 | 설명 |
|---|---|---|
| `idx` | INT AUTO_INCREMENT | PK |
| `code` | VARCHAR(50) | API 종류 (send_device, get_device) |
| `log_data` | TEXT | 요청 파라미터 JSON |
| `log_result` | TEXT | 응답 결과 JSON |
| `reg_ip` | VARCHAR(45) | 클라이언트 IP (IPv4: 최대 15자, IPv6: 최대 39자 → 45자 여유) |
| `reg_date` | DATETIME | 로그 기록 일시 |

---

## 구 테이블 vs 신규 테이블 비교

| 항목 | `data_smart_device` (구) | `lm_device` (신) |
|---|---|---|
| 엔진 | MyISAM | InnoDB (트랜잭션) |
| device_id | VARCHAR(20), Android ID | VARCHAR(36), UUID v4 |
| 문자셋 | latin1 또는 utf8 | utf8mb4 (완전한 유니코드) |
| brand/model/version | 있음 | 제거 (웹앱 불필요) |
| IP 컬럼 | VARCHAR(20) (IPv6 불가) | VARCHAR(45) (IPv6 지원) |

---

## DB 초기 셋업 절차

```bash
# MySQL 5.7.44 CLI 접속
/c/laragon/bin/mysql/mysql-5.7.44-winx64/bin/mysql.exe -u root

# SQL 파일 실행
source C:/SUNTECH_DEV_CLAUDECODE/WEB/ST500_LOCKMAKER/api/sql/schema.sql;

# 테이블 확인
USE suntech_st500;
SHOW TABLES LIKE 'lm_%';
```

---

## 디바이스 수동 승인 방법 (관리자)

```sql
-- 특정 디바이스 승인
UPDATE lm_device SET state = 'Y', update_date = NOW()
WHERE device_id = 'xxxxxxxx-xxxx-4xxx-xxxx-xxxxxxxxxxxx';

-- 대기 중인 디바이스 목록 확인
SELECT idx, device_id, name, reg_date FROM lm_device WHERE state = 'N';

-- 디바이스 비활성화 (삭제)
UPDATE lm_device SET state = 'D', update_date = NOW() WHERE idx = ?;
```
