# CTP280 OTA V1 — 사용 가이드

> 최초 작성: 2026-03-20
> 서버 배포 경로: `http://49.247.26.228/CTP280_OTA/CTP280_OTA_V1/`
> 관리 웹 페이지: `http://49.247.26.228/CTP280_OTA/CTP280_OTA_V1/index.html`

---

## 목차

1. [펌웨어 업로드 절차](#1-펌웨어-업로드-절차)
2. [디렉토리 구조](#2-디렉토리-구조)
3. [API 엔드포인트 레퍼런스](#3-api-엔드포인트-레퍼런스)
4. [db/version.json 구조](#4-dbversionjson-구조)
5. [버전 규칙 및 OTA 트리거 조건](#5-버전-규칙-및-ota-트리거-조건)
6. [서버 파일 권한 설정](#6-서버-파일-권한-설정)
7. [트러블슈팅](#7-트러블슈팅)

---

## 1. 펌웨어 업로드 절차

> 새 펌웨어를 빌드하고 디바이스가 자동 업데이트받도록 하는 전체 순서

### 1단계 — package.h 버전 변경

```c
// PSoC 프로젝트: Design.cydsn/package.h
#define PROJECT_FIRMWARE_VERSION "V2.0.1"   // 기존 V2.0.0 → 새 버전으로 변경
```

> ⚠️ **이 단계를 반드시 먼저 해야 합니다.**
> 펌웨어 바이너리 안에 이 버전 문자열이 포함되어, 업데이트 후 디바이스가 서버에 새 버전을 보고합니다.
> 버전을 변경하지 않으면 업데이트 후에도 디바이스가 구버전을 보고 → OTA가 무한 반복됩니다.

### 2단계 — PSoC Creator 빌드

```
PSoC Creator → Build → Clean and Build Design
```

빌드 결과물 경로:
```
Design.cydsn/CortexM0/ARM_GCC_541/Debug/Design.hex    ← 업로드할 파일
```

### 3단계 — 웹 페이지에서 업로드

관리 웹 페이지 접속:

```
http://49.247.26.228/CTP280_OTA/CTP280_OTA_V1/index.html
```

**업로드 폼 입력:**

| 입력 항목 | 내용 |
|----------|------|
| 버전 | `V2.0.1` (package.h에서 설정한 버전과 동일하게) |
| 파일 선택 | `Design.hex` 선택 (`.hex` 선택 시 자동 BIN 변환) |
| 릴리즈 노트 | 변경 내용 요약 (선택 사항) |

**업로드 버튼 클릭 시 동작:**
1. `.hex` 파일 감지 → 브라우저에서 Intel HEX → BIN 변환 (서버 변환 불필요)
2. `firmware/latest.bin` 저장
3. `db/version.json` 자동 갱신 (버전, 파일 크기, CRC16-CCITT)
4. 성공 메시지: `✓ 업로드 완료: V2.0.1 (xxx.x KB, CRC: 0x????)`

### 4단계 — 디바이스 자동 업데이트 확인

디바이스는 다음 두 가지 방식으로 업데이트를 수신합니다:

| 방식 | 트리거 | 조건 |
|------|--------|------|
| 자동 체크 | 부팅 후 20초 → 24시간 주기 | 서버 버전 > 현재 버전 시 LCD 헤더에 "UPD" 배지 표시 |
| 수동 업데이트 | LCD → MENU → OTA UPDATE | 버전 확인 후 다운로드 진행 |

---

## 2. 디렉토리 구조

```
CTP280_OTA_V1/
├── index.html              # 펌웨어 관리 웹 페이지
│                           # - 서버 최신 버전 표시
│                           # - 펌웨어 업로드 (HEX/BIN 지원)
│                           # - 디바이스 업데이트 현황 테이블
│
├── api/
│   ├── version.php         # GET → 최신 버전 정보 반환
│   ├── firmware.php        # GET ?offset=N&size=400 → 청크 반환
│   ├── upload.php          # POST multipart → 펌웨어 저장 + version.json 갱신
│   ├── status.php          # GET ?mac=&status=done&version= → 완료 기록
│   └── devices.php         # GET → 디바이스 목록 반환 (관리 웹용)
│
├── db/
│   ├── version.json        # 현재 서버 버전 정보 (자동 관리, 직접 편집 가능)
│   └── devices.json        # 디바이스 접속/업데이트 이력 (자동 관리)
│
├── firmware/
│   └── latest.bin          # 서버의 최신 펌웨어 바이너리 (업로드 시 덮어씀)
│
├── .htaccess               # 디렉토리 인덱싱 차단
├── db/.htaccess            # DB JSON 파일 직접 URL 접근 차단
└── firmware/.htaccess      # 펌웨어 파일 직접 URL 접근 차단
```

---

## 3. API 엔드포인트 레퍼런스

### 3.1 version.php — 버전 확인

디바이스 펌웨어가 OTA 자동 체크 및 수동 업데이트 시 호출합니다.

**요청:**
```
GET /CTP280_OTA/CTP280_OTA_V1/api/version.php
```

**응답:**
```json
{
  "version": "V2.0.1",
  "size": 108988,
  "crc": 12345
}
```

| 필드 | 타입 | 설명 |
|------|------|------|
| `version` | string | 최신 버전 (형식: `V{major}.{minor}.{patch}`) |
| `size` | number | 펌웨어 전체 크기 (bytes) |
| `crc` | number | CRC16-CCITT |

---

### 3.2 firmware.php — 청크 다운로드

디바이스가 수동 업데이트 진행 중 400 bytes 단위로 호출합니다.

**요청:**
```
GET /CTP280_OTA/CTP280_OTA_V1/api/firmware.php?offset=0&size=400
```

| 파라미터 | 설명 |
|----------|------|
| `offset` | 바이트 오프셋 (0부터 시작) |
| `size` | 청크 크기 (최대 400 bytes 고정) |

**응답:**
```json
{
  "offset": 0,
  "bytes": 400,
  "total": 108988,
  "hex": "AABB...CCDD"
}
```

| 필드 | 타입 | 설명 |
|------|------|------|
| `offset` | number | 현재 오프셋 |
| `bytes` | number | 이 응답에 포함된 실제 바이트 수 |
| `total` | number | 전체 파일 크기 |
| `hex` | string | 바이너리의 HEX 문자열 (400 bytes → 800 hex 문자) |

> **청크 크기 제한 이유**: PSoC WiFi 수신 버퍼 = 2048 bytes.
> 400 bytes = hex 800자 + JSON 오버헤드 ≈ 850자 → 2048 버퍼 이하 안전.

---

### 3.3 upload.php — 펌웨어 업로드

관리 웹 페이지(`index.html`)의 업로드 버튼이 호출합니다. 직접 호출도 가능합니다.

**요청:**
```
POST /CTP280_OTA/CTP280_OTA_V1/api/upload.php
Content-Type: multipart/form-data
```

| 필드 | 타입 | 필수 | 설명 |
|------|------|------|------|
| `firmware` | file | ✓ | 펌웨어 바이너리 파일 (`.bin`) |
| `version` | string | ✓ | 버전 문자열 (형식: `V2.0.1`) |
| `notes` | string | - | 릴리즈 노트 (선택) |

**응답 (성공):**
```json
{
  "status": "ok",
  "version": "V2.0.1",
  "size": 108988,
  "crc": 12345
}
```

**저장 내용:**
- `firmware/latest.bin` ← 업로드된 바이너리
- `db/version.json` ← 버전 정보 자동 갱신

---

### 3.4 status.php — 업데이트 완료 보고

디바이스가 다운로드 완료 후 호출합니다. 응답은 무시됩니다 (로깅 목적).

**요청:**
```
GET /CTP280_OTA/CTP280_OTA_V1/api/status.php?mac=AA:BB:CC:DD:EE:FF&status=done&version=V2.0.1
```

---

### 3.5 devices.php — 디바이스 목록

관리 웹 페이지가 30초 주기로 호출합니다.

**요청:**
```
GET /CTP280_OTA/CTP280_OTA_V1/api/devices.php
```

**응답:**
```json
{
  "devices": [
    {
      "mac": "AA:BB:CC:DD:EE:FF",
      "version": "V2.0.0",
      "status": "done",
      "is_latest": false,
      "updated_at": "2026-03-20 10:30:00",
      "ip": "192.168.1.10"
    }
  ]
}
```

---

## 4. db/version.json 구조

업로드 시 자동 생성/갱신됩니다. 필요 시 수동 편집도 가능합니다.

```json
{
  "version": "V2.0.1",
  "size": 108988,
  "crc": 12345,
  "uploaded_at": "2026-03-20 10:00:00",
  "filename": "firmware.bin",
  "notes": "버그 수정: WIFI INFO, OTA 먹통 수정"
}
```

> **수동 편집 시 주의**: `size`와 `crc`는 실제 `firmware/latest.bin` 파일과 일치해야 합니다.
> 불일치 시 디바이스 다운로드 후 CRC 검증 실패로 부트로더가 업데이트를 거부할 수 있습니다.

---

## 5. 버전 규칙 및 OTA 트리거 조건

### 버전 형식

```
V{major}.{minor}.{patch}
```

예시: `V2.0.0`, `V2.0.1`, `V2.1.0`, `V3.0.0`

### OTA 트리거 조건

디바이스 펌웨어의 `compareVersion()` 함수 기준:

```
서버 버전 > 현재 디바이스 버전  →  OTA 다운로드 시작
서버 버전 = 현재 디바이스 버전  →  "Already latest!" (업데이트 없음)
서버 버전 < 현재 디바이스 버전  →  "Up to date" (업데이트 없음)
```

### 버전 비교 알고리즘

`compareVersion("V2.0.1", "V2.0.0")`:
1. `V` 제거 → `"2.0.1"`, `"2.0.0"`
2. `.`으로 분리 → `[2,0,1]`, `[2,0,0]`
3. major → minor → patch 순서로 수치 비교
4. `2.0.1 > 2.0.0` → 양수 반환 → OTA 시작

---

## 6. 서버 파일 권한 설정

Linux/Apache 서버 배포 시 아래 권한 설정 필요:

```bash
# 웹 서버 소유권 설정
chown -R www-data:www-data /var/www/html/CTP280_OTA/

# 디렉토리 권한
chmod 755 CTP280_OTA_V1/
chmod 755 CTP280_OTA_V1/api/
chmod 755 CTP280_OTA_V1/db/
chmod 755 CTP280_OTA_V1/firmware/

# 파일 권한
chmod 644 CTP280_OTA_V1/index.html
chmod 644 CTP280_OTA_V1/api/*.php
chmod 666 CTP280_OTA_V1/db/version.json     # 웹 서버가 쓰기 필요
chmod 666 CTP280_OTA_V1/db/devices.json     # 웹 서버가 쓰기 필요
chmod 666 CTP280_OTA_V1/firmware/latest.bin # 웹 서버가 쓰기 필요
```

> **Linux 대소문자 주의**: 서버 경로는 대소문자를 정확히 구분합니다.
> 디바이스 펌웨어의 `DEFAULT_OTA_API_PATH`와 실제 서버 폴더명이 정확히 일치해야 합니다.

---

## 7. 트러블슈팅

### 7.1 업로드 500 Internal Server Error

업로드 시 500 에러가 발생하면 브라우저 콘솔에서 아래와 같은 오류가 표시됩니다:

```
upload.php:1  Failed to load resource: the server responded with a status of 500
POST http://49.247.26.228/CTP280_OTA/CTP280_OTA_V1/api/upload.php 500 (Internal Server Error)
```

`upload.php`(2026-03-20 개선)는 상세 오류 메시지를 JSON으로 반환합니다.
브라우저 콘솔의 `업로드 실패:` 뒤 메시지로 원인을 특정하세요.

#### 원인별 해결책

**① `firmware 디렉토리 쓰기 권한 없음` 또는 `db 디렉토리 쓰기 권한 없음`**

가장 빈번한 원인. 서버(Linux)에 SSH 접속 후:

```bash
cd /var/www/html/CTP280_OTA/CTP280_OTA_V1/

# 디렉토리 권한 설정 (없으면 upload.php가 자동 생성 시도)
chmod 775 firmware/ db/
chown www-data:www-data firmware/ db/

# 이미 파일이 있을 경우
chmod 664 firmware/latest.bin db/version.json
chown www-data:www-data firmware/latest.bin db/version.json
```

**② `PHP 업로드 오류: php.ini upload_max_filesize 초과`**

업로드할 BIN 파일이 PHP 기본 제한(보통 2MB~8MB)을 넘는 경우.
`CTP280_OTA_V1/.htaccess`에 다음을 추가:

```apache
php_value upload_max_filesize 256M
php_value post_max_size 256M
```

또는 서버 `php.ini`에서:

```ini
upload_max_filesize = 256M
post_max_size = 256M
```

**③ `PHP 업로드 오류: PHP 임시 디렉토리 없음`**

서버의 PHP `upload_tmp_dir`이 설정되지 않은 경우. `php.ini`에서:

```ini
upload_tmp_dir = /tmp
```

#### upload.php 상세 오류 메시지 참조

| 메시지 | 원인 |
|--------|------|
| `firmware 디렉토리 쓰기 권한 없음` | `firmware/` chmod 775 필요 |
| `db 디렉토리 쓰기 권한 없음` | `db/` chmod 775 필요 |
| `디렉토리 생성 실패` | 상위 디렉토리 권한 문제 |
| `move_uploaded_file 실패` | `firmware/` 쓰기 불가 |
| `version.json 저장 실패` | `db/` 쓰기 불가 |
| `php.ini upload_max_filesize 초과` | .htaccess 또는 php.ini 크기 제한 수정 |
| `PHP 임시 디렉토리 없음` | php.ini `upload_tmp_dir` 설정 |

### 7.2 기타 문제

| 증상 | 원인 | 해결책 |
|------|------|--------|
| OTA UPDATE → `Bad version data` | 서버에 `version.json` 없음 / `version.php` 경로 오류 | 펌웨어를 한 번 업로드하여 `version.json` 생성 |
| 업로드 후에도 디바이스가 계속 업데이트 시도 | `package.h` 버전을 변경하지 않고 빌드 | `package.h` → 버전 변경 → 재빌드 → 재업로드 |
| HEX 파일 업로드 시 크기가 너무 크게 표시됨 | `.hex` 텍스트 파일 자체 업로드됨 (변환 안됨) | 브라우저에서 JavaScript 허용 여부 확인 |
| `404 Not Found` (API 호출 시) | 서버 경로 대소문자 불일치 | `DEFAULT_OTA_API_PATH` 값과 서버 실제 경로 비교 |
| 디바이스가 "Already latest!"만 표시 | 서버 버전 ≤ 디바이스 버전 | 서버에 더 높은 버전 업로드 필요 |

---

*Copyright SUNTECH, 2026*
