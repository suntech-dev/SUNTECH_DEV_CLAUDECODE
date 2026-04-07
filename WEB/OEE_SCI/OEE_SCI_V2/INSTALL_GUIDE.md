# OEE_SCI_V2 고객사 서버 설치 가이드

> 작성일: 2026-04-07  
> 대상 서버: 고객사 운영 서버  
> 로컬 개발환경: Laragon (Windows)

---

## 1. 시스템 요구사항

| 항목 | 최소 요구사항 | 권장 |
|------|--------------|------|
| PHP | 7.4 이상 | 8.0 이상 |
| MySQL | 5.7 이상 | 8.0 이상 |
| Apache | 2.4 이상 | — |
| PHP 확장 | `pdo`, `pdo_mysql`, `zip`, `mbstring`, `xml`, `gd` | — |

---

## 2. 파일 업로드

### 업로드할 폴더 (전체)

```
OEE_SCI_V2/
├── api/
├── assets/
├── doc/
├── inc/
├── index.php
├── lib/              ← vendor/ 포함하여 모두 업로드
├── logs/             ← 빈 폴더 생성 필요 (쓰기 권한 부여)
├── opcache.php
├── page/
└── upload/           ← 빈 폴더 생성 필요 (쓰기 권한 부여)
```

### 업로드 제외 파일

```
OEE_SCI_V2/.env       ← 로컬 개발용 설정, 서버에는 별도 생성
```

> `.env` 파일은 로컬 것을 그대로 올리지 말 것.  
> 서버용 `.env`를 **Step 3**에서 별도로 생성한다.

---

## 3. .env 파일 생성 (필수)

서버의 `OEE_SCI_V2/` 루트에 `.env` 파일을 아래 내용으로 새로 생성한다.

```env
DB_HOST=localhost
DB_USERNAME=root
DB_PASSWORD=고객사_MySQL_비밀번호
DB_NAME=sci_2025_new
```

### 설정 항목 설명

| 항목 | 설명 | 기본값 (config.php) |
|------|------|---------------------|
| `DB_HOST` | MySQL 서버 주소 (서버 내부에서는 localhost) | `49.247.26.228` |
| `DB_USERNAME` | MySQL 계정 | `root` |
| `DB_PASSWORD` | MySQL 비밀번호 | `suntech9304!` |
| `DB_NAME` | 데이터베이스 이름 | `sci_2025_new` |

> **주의**: `.env` 파일이 없으면 `config.php`의 기본값(`49.247.26.228`, `suntech9304!`)이 사용된다.  
> 반드시 `.env` 파일을 생성하여 기본값을 덮어써야 한다.

---

## 4. 데이터베이스 설정

### DB 이름 — 변경 불필요

코드(`lib/config.php`)의 기본 DB명이 `sci_2025_new`이므로,  
**고객사 서버의 DB 이름을 `sci_2025_new` 그대로 유지한다.**

```
oee_sci_v2 (로컬 개발용) ≠ sci_2025_new (고객사 운영용)
```

DB 이름을 바꾸려면 `.env`의 `DB_NAME` 값을 변경하면 된다.

### DB 비교 결과 (로컬 oee_sci_v2 vs 고객사 sci_2025_new)

| 항목 | 결과 |
|------|------|
| 테이블 수 | 28개 (완전 동일) |
| 컬럼 구조 | 완전 동일 |
| 인덱스 | 차이 1건 (아래 참고) |

#### 인덱스 차이 — 고객사 DB가 더 최신

`data_oee_rows_hourly` 테이블:

| 인덱스명 | 컬럼 | 로컬 oee_sci_v2 | 고객사 sci_2025_new |
|----------|------|-----------------|---------------------|
| `idx_hourly_date_hour` | `work_date`, `work_hour` | **없음** | **있음** |

→ 고객사 DB(`sci_2025_new`)에 성능 인덱스가 추가된 상태. **추가 조치 불필요.**

---

## 5. 디렉토리 권한 설정

아래 두 폴더는 PHP가 파일을 기록해야 하므로 쓰기 권한 부여.

```bash
chmod 775 logs/
chmod 775 upload/
```

Windows 서버의 경우 IIS/Apache 실행 계정에 쓰기 권한을 부여한다.

---

## 6. 타임존 확인

`lib/db.php`에 타임존이 중앙 설정되어 있다:

```php
date_default_timezone_set('Asia/Jakarta');
```

MySQL 서버의 타임존도 동일하게 맞추는 것을 권장한다:

```sql
-- MySQL 세션 타임존 확인
SELECT @@global.time_zone, @@session.time_zone;

-- my.cnf 또는 my.ini에 추가 (권장)
[mysqld]
default-time-zone = '+07:00'
```

---

## 7. Apache 설정

### DocumentRoot 설정 예시

```apache
<VirtualHost *:80>
    ServerName 고객사도메인.com
    DocumentRoot "/var/www/html/OEE_SCI_V2"
    
    <Directory "/var/www/html/OEE_SCI_V2">
        AllowOverride All
        Require all granted
    </Directory>
</VirtualHost>
```

### .htaccess

현재 프로젝트에 `.htaccess`가 없는 경우, 필요시 아래를 루트에 추가:

```apache
Options -Indexes
```

> 디렉토리 목록 노출 방지 (보안)

---

## 8. 배포 후 동작 확인 체크리스트

### 기본 동작

- [ ] `index.php` 접속 확인 (메인 대시보드 로드)
- [ ] `lib/db.php` → `lib/config.php` → `.env` 순서로 DB 연결 확인
- [ ] `logs/` 폴더에 DB 에러 로그 기록 여부 확인

### DB 연결 확인

브라우저에서 아래 API 호출 후 `code: "00"` 응답 확인:
```
/api/sewing/get_dateTime.php
```

### 페이지별 확인

| 페이지 | URL 경로 |
|--------|----------|
| 메인 대시보드 | `/page/data/dashboard_2.php` |
| OEE 데이터 | `/page/data/data_oee_2.php` |
| 다운타임 | `/page/data/data_downtime_2.php` |
| 공장 관리 | `/page/manage/info_factory_2.php` |
| 라인 관리 | `/page/manage/info_line_2.php` |
| 설비 관리 | `/page/manage/info_machine_2.php` |
| 근무시간 관리 | `/page/manage/info_worktime_2.php` |

### AI 대시보드

- [ ] `/page/data/ai_dashboard_2.php` 접속 확인
- [ ] Export 기능 (Excel 다운로드) 동작 확인 — `phpspreadsheet` 의존

---

## 9. 보안 체크리스트

- [ ] `.env` 파일 웹 접근 차단 확인 (`.htaccess`에 `Deny from all` 추가 권장)
- [ ] `logs/` 폴더 웹 접근 차단
- [ ] `opcache.php` 접근 제한 (운영 환경에서 삭제 권장)
- [ ] `config.php`의 하드코딩 비밀번호가 `.env`로 덮어씌워지는지 확인

### .env 웹 접근 차단 (.htaccess)

```apache
<Files ".env">
    Require all denied
</Files>

<Files "opcache.php">
    Require all denied
</Files>

<DirectoryMatch "^(.*/)?logs/">
    Require all denied
</DirectoryMatch>
```

---

## 10. 트러블슈팅

### DB 연결 실패 시

1. `logs/db_errors.log` 확인
2. `.env` 파일 존재 여부 및 내용 확인
3. MySQL 서비스 실행 여부 확인
4. MySQL 계정 권한 확인:
   ```sql
   GRANT ALL PRIVILEGES ON sci_2025_new.* TO 'root'@'localhost';
   FLUSH PRIVILEGES;
   ```

### PHP 에러 발생 시

```bash
# PHP 에러 로그 확인 (Apache)
tail -f /var/log/apache2/error.log

# PHP 확장 설치 여부 확인
php -m | grep -E "pdo|zip|mbstring|xml|gd"
```

### 엑셀 Export 안 될 때

`phpspreadsheet`가 `zip`, `xml`, `gd` PHP 확장을 필요로 한다.

```bash
# Ubuntu/Debian
sudo apt install php-zip php-xml php-gd

# CentOS/RHEL
sudo yum install php-zip php-xml php-gd
```

---

## 11. 로컬 oee_sci_v2에 인덱스 동기화 (선택)

로컬 개발 DB와 고객사 DB의 인덱스를 일치시키려면 아래 SQL 실행:

```sql
-- 로컬 oee_sci_v2에 누락된 인덱스 추가
ALTER TABLE `oee_sci_v2`.`data_oee_rows_hourly`
ADD INDEX `idx_hourly_date_hour` (`work_date`, `work_hour`);
```
