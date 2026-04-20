# 11. 관리페이지 (Admin)

> 최초 작성: 2026-04-17
> 마지막 업데이트: 2026-04-17

---

## 개요

LockMaker PWA 디바이스 승인·관리를 위한 웹 관리페이지.  
기존 `old_admin`을 완전 리뉴얼. 모바일 최적화, 신규 DB 테이블 적용.

---

## 구 관리페이지 분석 (`old_admin`)

### 문제점

| 항목 | 문제 | 신규 개선 |
|---|---|---|
| DB 연결 | 운영 서버 IP·비밀번호 하드코딩 | `.env` 파일 분리 |
| SQL Injection | 필터 값 직접 문자열 삽입 | Prepared Statement 전환 |
| 테이블 | `data_smart_device` + `country` 조인 | `lm_device` (country 없음) |
| 모바일 | PC 전용 레이아웃 | Bootstrap 5 반응형 |
| 경로 | `$_SERVER['DOCUMENT_ROOT'].'/st500/'` 하드코딩 | `__DIR__` 상대경로 |
| 로그 | `st500_logs` | `lm_logs` |

### 기능 인벤토리

| 구 페이지 | 기능 | 신규 처리 |
|---|---|---|
| `setting_device.php` | 디바이스 조회·상태 변경 | `device.php` (재구현) |
| `log.php` | API 로그 조회 | `log.php` (재구현) |
| `parameter.php`, `para.php` | 파라미터 관리 | 제거 (신규 앱 불필요) |
| `data_smartDevice.php` | 구 장치 목록 | 제거 (레거시) |
| `personal_private.php` | 개인정보 | 제거 |

---

## 신규 파일 구조

```
WEB/ST500_LOCKMAKER/admin/
├── index.php    ← device.php로 redirect (1줄)
├── device.php   ← 디바이스 관리 (완전 자기 포함: AJAX + HTML + CSS + JS)
├── log.php      ← API 로그 조회 (완전 자기 포함: AJAX + HTML + CSS + JS)
├── .env         ← DB 설정 (로컬: localhost/root/무)
└── lib/
    ├── db.php       ← PDO 연결 (.env 로드, PHP 7.4 호환)
    └── helpers.php  ← json_ok(), json_fail(), now_datetime()
```

> 각 페이지가 완전히 자기 포함(self-contained) 구조.  
> `?ajax=1` 파라미터로 AJAX 엔드포인트를 같은 파일에서 처리.  
> `?action=update` POST 로 디바이스 상태 변경 처리 (device.php 내부).

---

## 기술 스택

| 항목 | 값 |
|---|---|
| 서버 | PHP 7.4.33 + MySQL 5.7.44 (Laragon) |
| UI | **순수 CSS** (외부 프레임워크 없음) |
| JS | **순수 JS** (fetch API, 외부 라이브러리 없음) |
| 스타일 참조 | `WEB/CTP280_API/log_embroidery.php` 다크 테마 |
| 빌드 시스템 | 없음 (순수 PHP) |

---

## 화면 구성

### 공통 네비게이션 (반응형)

```
[ST-500 LOCKMAKER ADMIN]    [DEVICE] [LOG]  [≡ 모바일 햄버거]
```

- sticky-top 고정
- 모바일: 햄버거 → 드롭다운 메뉴
- 현재 페이지 nav-link active 처리

### device.php — 디바이스 관리

```
┌─ 필터 바 ─────────────────────────────────────────┐
│  [● ALL]  [○ 대기 N]  [○ 승인 Y]  [○ 삭제 D]     │
│  [Search box]                    [REFRESH]         │
└────────────────────────────────────────────────────┘
┌─ 테이블 ───────────────────────────────────────────┐
│  NO │ DEVICE ID │ NAME │ STATUS │ REG DATE │ ACTION │
│  1  │ uuid...   │ 홍길동│ 대기   │ 2026-04  │ [편집] │
│  2  │ uuid...   │ 이철수│ 승인   │ 2026-04  │ [편집] │
└────────────────────────────────────────────────────┘
```

- STATUS 뱃지: 대기=노란색, 승인=초록색, 삭제=회색
- ACTION [편집] 클릭 → 모달 (state 변경 셀렉트)
- 모바일: DEVICE ID 칼럼 축소, DataTables responsive

### 편집 모달

```
┌─ EDIT DEVICE ──────────────────────────┐
│  IDX: 1                                │
│  DEVICE ID: 11111111-1111-4...  (읽기전용)│
│  NAME: 홍길동                   (읽기전용)│
│  STATUS: [대기▼]   ← 변경 가능         │
│                          [SAVE] [Close] │
└────────────────────────────────────────┘
```

### log.php — API 로그

```
┌─ 필터 바 ─────────────────────────────────────────┐
│  [코드 선택▼]    [Search box]        [REFRESH]    │
└────────────────────────────────────────────────────┘
┌─ 테이블 ───────────────────────────────────────────┐
│  NO │ CODE │ REQUEST │ RESPONSE │ IP │ DATE        │
└────────────────────────────────────────────────────┘
```

---

## 접속 URL

| 환경 | URL |
|---|---|
| 로컬 (Laragon) | `http://localhost/dev/ST500_LOCKMAKER/admin/` |
| 운영 서버 | `http://49.247.27.154/st500/admin/` (배포 후 — 경로 확인 필요) |

---

## 보안 고려사항

| 항목 | 현황 | 향후 |
|---|---|---|
| 인증 | 없음 (로컬 개발) | 로그인 세션 추가 예정 |
| SQL Injection | Prepared Statement 완전 적용 | 유지 |
| DB 인증정보 | `.env` 분리, 코드 노출 없음 | 유지 |
| CSRF | 미적용 | 추후 토큰 추가 |
| 접근 제어 | 미적용 | IP 화이트리스트 또는 Basic Auth 검토 |

---

## 구현 이력

| 날짜 | 내용 |
|---|---|
| 2026-04-17 | 신규 관리페이지 `admin/` 초기 구현 (device, log 2개 화면) |
