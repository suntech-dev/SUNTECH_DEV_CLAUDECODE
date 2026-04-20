# 03. 서버 API 명세

> 최초 작성: 2026-04-17
> 마지막 업데이트: 2026-04-20

---

## API 서버 구성

### 로컬 개발 환경 (Laragon)

| 항목 | 값 |
|---|---|
| 서버 | Laragon (Apache + PHP 7.4.33 + **MySQL 5.7.44**) |
| 로컬 API URL | `http://localhost/dev/ST500_LOCKMAKER/api/index.php` |
| API 소스 경로 | `C:\SUNTECH_DEV_CLAUDECODE\WEB\ST500_LOCKMAKER\api\` |
| .env 설정 | `VITE_API_BASE_URL=http://localhost/dev/ST500_LOCKMAKER/api/index.php` |

### 운영 서버

| 항목 | 값 |
|---|---|
| 서버 IP | `49.247.27.154` |
| 운영 앱 URL | `http://49.247.27.154/st500/lockmaker` |
| 운영 API URL | `http://49.247.27.154/api/st500/st500_api.php` (서버 구성 후 확인) |
| 통신 | HTTP → **HTTPS 전환 필요** |

> ⚠️ 운영 API 경로는 서버 실제 구성에 따라 달라질 수 있다. 서버 설치 완료 후 확인 필수.

---

## 신규 API 파일 구조

```
WEB/ST500_LOCKMAKER/api/
├── index.php          ← 라우터 (code 파라미터로 분기)
├── .env               ← DB 설정 (DB_HOST, DB_USERNAME, DB_PASSWORD, DB_NAME)
├── lib/
│   ├── db.php         ← PDO 연결 (MySQL 5.7.44, .env 로드)
│   └── helpers.php    ← get_param(), json_response(), write_log(), get_client_ip()
├── v1/
│   ├── send_device.php
│   └── get_device.php
└── sql/
    └── schema.sql     ← lm_device, lm_logs 테이블 DDL
```

---

## 응답 형식

모든 API 응답은 JSON 배열:

```json
[{"code": "00", "msg": "success"}]
```

| code | 의미 |
|---|---|
| `00` | 성공 |
| `01` | 필수 파라미터 누락 |
| `02` | device_id 형식 오류 (UUID v4 아님) |
| `03` | 삭제된 디바이스 (state=D) |
| `04` | 디바이스 없음 (미등록) |
| `99` | 알 수 없는 API 코드 |

---

## API 목록

### 1. 디바이스 등록 (`send_device`)

디바이스를 서버에 등록하여 관리자 승인 요청.
최초 실행 시, 또는 이름 변경 후 저장 시 호출.

**요청**
```
GET index.php?code=send_device&device_id={UUID}&name={이름}
```

| 파라미터 | 타입 | 설명 |
|---|---|---|
| `code` | string | 고정값 `send_device` |
| `device_id` | string | UUID v4 (localStorage: `lm_device_id`) |
| `name` | string | 사용자 이름 |

**응답**

| msg | 의미 |
|---|---|
| `success` | 등록/업데이트 성공 → 승인 대기 상태 |
| `missing_params` | device_id 또는 name 누락 |
| `invalid_device_id` | UUID v4 형식 불일치 |
| `device_deleted` | 삭제된 디바이스 (재등록 불가) |

**클라이언트 처리** (`src/services/api.js` → `registerDevice()`)
```javascript
const res = await registerDevice(deviceId, userName)
if (res?.msg === 'success') {
    status = STATUS.WAITING
    startPolling()
}
```

---

### 2. 승인 상태 조회 (`get_device`)

관리자가 디바이스를 승인했는지 확인. 1초 간격으로 반복 조회.

**요청**
```
GET index.php?code=get_device&device_id={UUID}
```

| 파라미터 | 타입 | 설명 |
|---|---|---|
| `code` | string | 고정값 `get_device` |
| `device_id` | string | UUID v4 |

**응답**

| msg | 의미 | 클라이언트 동작 |
|---|---|---|
| `approve` | 관리자 승인 완료 | STATUS.APPROVED, 폴링 중지, MAKE 활성화 |
| `wait` | 승인 대기 중 | 계속 폴링 |
| `deleted` | 삭제된 디바이스 | STATUS.ERROR |
| `not_found` | 미등록 디바이스 | STATUS.ERROR |

**클라이언트 처리** (`src/stores/device.js` → `startPolling()`)
```javascript
setInterval(async () => {
    const res = await getDeviceStatus(deviceId)
    if (res?.msg === 'approve') {
        status = STATUS.APPROVED
        stopPolling()
    }
}, 1000)
```

---

## 에러 처리 (클라이언트)

```javascript
// src/services/api.js
async function request(params) {
    const res = await fetch(url, { ... })
    if (!res.ok) throw new Error(`HTTP ${res.status}`)
    const text = await res.text()
    if (!text || text.length < 20) throw new Error('응답 데이터 부족')
    return JSON.parse(text)
}
```

| 에러 조건 | 처리 |
|---|---|
| HTTP 4xx/5xx | `Error('HTTP 상태코드')` throw |
| 응답 길이 20 미만 | `Error('응답 데이터 부족')` throw |
| JSON 파싱 실패 | `SyntaxError` 자연 전파 |
| `register()` 실패 | `STATUS.ERROR` 설정 |
| `polling()` 실패 | 에러 무시, 다음 사이클 재시도 |

---

## 향후 API 확장 계획

코드 생성 알고리즘을 서버로 이전할 경우 추가할 API:

```
POST index.php
Body: { code: "generate_lock", device_id, old_code, lock_day }
Response: [{ code: "00", msg: "success", new_code: "012345678" }]
```

> **이유**: 클라이언트 JS 알고리즘은 브라우저 개발자 도구로 완전 노출됨.

---

## 구 API 참고 (보존, 신규 사용 안함)

| 항목 | 값 |
|---|---|
| 경로 | `WEB/ST500_LOCKMAKER/old_api/st500/st500_api.php` |
| 테이블 | `data_smart_device`, `st500_logs` |
| 비고 | 기존 Android 앱 전용, 신규 PWA는 신규 API 사용 |
