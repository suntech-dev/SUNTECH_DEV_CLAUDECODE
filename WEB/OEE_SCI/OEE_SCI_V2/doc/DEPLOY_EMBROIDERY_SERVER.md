# 자수기 데이터 연동 — 실서버 배포 가이드

> 작성일: 2026-04-11  
> 대상 서버: 49.247.26.228 (운영 서버)  
> 대상 프로젝트: OEE_SCI_V2

---

## 배포 전 체크리스트

- [ ] 로컬 테스트 완료 (자수기 기기 또는 curl 테스트)
- [ ] DB 백업 완료
- [ ] 배포 작업자 확인

---

## 1단계 — DB 마이그레이션 (MySQL)

> 실서버 MySQL 에 접속하여 아래 SQL 을 순서대로 실행합니다.  
> 기존 데이터 손실 없음 (컬럼 추가 / 테이블 신규 생성만 수행).

```sql
-- 1-1. data_oee 자수기 전용 컬럼 추가
ALTER TABLE data_oee
  ADD COLUMN thread_breakage INT NOT NULL DEFAULT 0 COMMENT '실끊김 누적 횟수',
  ADD COLUMN motor_run_time  INT NOT NULL DEFAULT 0 COMMENT '모터동작시간 누적(초)';

-- 1-2. data_oee_rows 자수기 전용 컬럼 추가
ALTER TABLE data_oee_rows
  ADD COLUMN thread_breakage INT NOT NULL DEFAULT 0 COMMENT '실끊김 누적 횟수',
  ADD COLUMN motor_run_time  INT NOT NULL DEFAULT 0 COMMENT '모터동작시간 누적(초)';

-- 1-3. data_oee_rows_hourly 자수기 전용 컬럼 추가
ALTER TABLE data_oee_rows_hourly
  ADD COLUMN thread_breakage INT NOT NULL DEFAULT 0 COMMENT '실끊김 누적 횟수',
  ADD COLUMN motor_run_time  INT NOT NULL DEFAULT 0 COMMENT '모터동작시간 누적(초)';

-- 1-4. 자수기 API 호출 로그 테이블 신규 생성
CREATE TABLE IF NOT EXISTS `logs_api_send_ecount` (
  `idx`        int(11)      NOT NULL AUTO_INCREMENT,
  `gubun`      varchar(30)  DEFAULT NULL,
  `machine_no` varchar(20)  DEFAULT NULL,
  `mac`        varchar(17)  DEFAULT NULL,
  `data`       text         DEFAULT NULL,
  `result`     text         DEFAULT NULL,
  `reg_date`   datetime     DEFAULT NULL,
  PRIMARY KEY (`idx`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8 COMMENT='자수기 send_eCount API 호출 로그';
```

### 적용 확인
```sql
SHOW COLUMNS FROM data_oee LIKE 'thread%';
SHOW COLUMNS FROM data_oee LIKE 'motor%';
SHOW TABLES LIKE 'logs_api_send_ecount';
```

---

## 2단계 — 웹 파일 업로드

아래 파일들을 실서버 `/var/www/html/OEE_SCI/OEE_SCI_V2/` 하위에 업로드합니다.

| 로컬 경로 | 서버 경로 | 비고 |
|-----------|-----------|------|
| `WEB/OEE_SCI/OEE_SCI_V2/api/embroidery.php` | `api/embroidery.php` | 자수기 전용 라우터 (신규) |
| `WEB/OEE_SCI/OEE_SCI_V2/api/embroidery/start.php` | `api/embroidery/start.php` | 자수기 장비 등록 (신규) |
| `WEB/OEE_SCI/OEE_SCI_V2/api/embroidery/send_eCount.php` | `api/embroidery/send_eCount.php` | 자수기 생산 카운트 (신규) |

> **주의**: `api/sewing/` 폴더의 기존 파일은 **수정하지 않습니다.**  
> `api/sewing/get_dateTime.php` 는 이미 AUTO RESET 교대 기반 코드가 적용되어 있어 자수기도 공유합니다.

### 업로드 후 권한 설정 (필요 시)
```bash
chmod 644 api/embroidery.php
chmod 644 api/embroidery/start.php
chmod 644 api/embroidery/send_eCount.php
```

---

## 3단계 — 자수기 API 동작 검증

서버에서 아래 curl 명령으로 각 엔드포인트를 확인합니다.

### 3-1. start (자수기 등록)
```bash
curl "http://49.247.26.228/OEE_SCI/OEE_SCI_V2/api/embroidery.php?code=start&machine_no=EMB01&mac=AA:BB:CC:DD:EE:FF&ip=192.168.1.100&ver=EMBROIDERY_S_REV_1.0"
```
기대 응답:
```json
{"code":"00","machine_no":"EMB01","target":"777","req_interval":"3600"}
```

### 3-2. get_dateTime (AUTO RESET 교대 기반)
```bash
curl "http://49.247.26.228/OEE_SCI/OEE_SCI_V2/api/embroidery.php?code=get_dateTime&mac=AA:BB:CC:DD:EE:FF"
```
기대 응답 (근무 중):
```json
{"datetime":"2026-04-11 10:30:00","work_date":"2026-04-11","shift_idx":1}
```

### 3-3. send_eCount (생산 카운트)
```bash
curl "http://49.247.26.228/OEE_SCI/OEE_SCI_V2/api/embroidery.php?code=send_eCount&mac=AA:BB:CC:DD:EE:FF&actual_qty=1&ct=45&tb=0&mrt=40"
```
기대 응답:
```json
{"code":"00","msg":"Embroidery OEE data updated successfully"}
```

> 기계 등록(info_machine) 및 공정 설정(info_design_process)이 완료된 상태여야 합니다.

---

## 4단계 — PSOC 펌웨어 빌드 및 업데이트

> PSoC Creator 에서 `EMBROIDERY_S` 프로젝트를 빌드 후 기기에 플래시합니다.

### 수정된 소스 파일
| 파일 | 변경 내용 |
|------|-----------|
| `lib/server.h` | `DEFAULT_API_ENDPOINT` → `/api/embroidery.php` |
| `andonApi.c` | `makeAndonCurrentTimeRequest()` — mac 파라미터 추가 |
| `andonApi.c` | `makeAndonPatternCount()` — ct/mrt 단위 초 정수로 변경 (`%0.1f` → `%u`, `/10.` → `/10u`) |
| `andonJson.c` | `andonCurrentTimeParsing()` — AUTO RESET 교대 기반 비교 적용 |

### 빌드 경로
```
PSOC\280CTP_IoT_INTEGRATED\280CTP_IoT_INTEGRATED_V1_EMBROIDERY_S\
  Project\Design.cydsn\
```

### 적용 기기
- 자수기에 연결된 CTP280 보드 (EMBROIDERY_S 펌웨어 탑재)
- 기존 재봉기(BLACK_CPU)는 영향 없음

---

## 5단계 — info_machine 자수기 등록 확인

관리 화면에서 자수기 기기가 `type='E'` 로 등록되었는지 확인합니다.

```sql
SELECT machine_no, mac, type, factory_idx, line_idx, design_process_idx, status
FROM info_machine
WHERE type = 'E';
```

- 펌웨어 업데이트 후 기기 전원 ON 시 `embroidery/start.php` 가 호출되어 자동으로 `type='E'` 로 등록/갱신됩니다.
- 관리 화면에서 공장/라인/공정 배정을 완료해야 OEE 데이터가 저장됩니다 (`line_idx ≠ 99`).

---

## 롤백 계획

문제 발생 시:

1. 업로드한 파일 제거
   ```bash
   rm api/embroidery.php
   rm -rf api/embroidery/
   ```

2. DB 컬럼 제거 (데이터 보존이 필요하면 생략)
   ```sql
   ALTER TABLE data_oee DROP COLUMN thread_breakage, DROP COLUMN motor_run_time;
   ALTER TABLE data_oee_rows DROP COLUMN thread_breakage, DROP COLUMN motor_run_time;
   ALTER TABLE data_oee_rows_hourly DROP COLUMN thread_breakage, DROP COLUMN motor_run_time;
   DROP TABLE IF EXISTS logs_api_send_ecount;
   ```

3. 자수기 펌웨어를 이전 버전으로 재플래시

---

## 변경 영향 범위

| 대상 | 영향 |
|------|------|
| 기존 재봉기 데이터 | **영향 없음** — `send_pCount`, `sewing.php` 무수정 |
| 기존 DB 데이터 | **영향 없음** — 컬럼 추가만 수행, 기존값 유지 |
| 기존 재봉기 펌웨어 | **영향 없음** — BLACK_CPU 소스 무수정 |
| 자수기 기기 | 펌웨어 업데이트 필요 |
| 관리 화면 | 별도 수정 불필요 (info_machine.type 컬럼 이미 존재) |
