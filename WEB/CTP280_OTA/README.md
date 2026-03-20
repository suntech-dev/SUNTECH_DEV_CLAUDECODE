# CTP280 OTA 웹 서버

> 최초 작성: 2026-03-20
> 역할: CTP280 IoT 디바이스의 Over-The-Air 펌웨어 업데이트 서버

---

## 개요

이 웹 서버는 **CTP280 IoT 디바이스(PSoC 4, Cortex-M0)**의 무선 펌웨어 업데이트(OTA)를 담당합니다.

- **관리 웹 페이지**: `CTP280_OTA_V1/index.html` — 펌웨어 업로드, 디바이스 현황 조회
- **REST API**: `CTP280_OTA_V1/api/` — 디바이스가 HTTP GET으로 직접 호출하는 엔드포인트
- **배포 서버**: `http://49.247.26.228/CTP280_OTA/`

---

## 디렉토리 구조

```
CTP280_OTA/
└── CTP280_OTA_V1/              # OTA 서버 V1
    ├── index.html              # 펌웨어 관리 웹 페이지 (업로드 + 디바이스 현황)
    ├── api/
    │   ├── version.php         # 디바이스 → 버전 확인 (GET)
    │   ├── firmware.php        # 디바이스 → 청크 다운로드 (GET)
    │   ├── upload.php          # 관리자 → 펌웨어 업로드 (POST)
    │   ├── status.php          # 디바이스 → 업데이트 완료 보고 (GET)
    │   └── devices.php         # 관리 웹 → 디바이스 목록 조회 (GET)
    ├── db/
    │   ├── version.json        # 최신 버전 정보 저장 (업로드 시 자동 갱신)
    │   └── devices.json        # 디바이스 접속 이력 저장
    ├── firmware/
    │   └── latest.bin          # 최신 펌웨어 바이너리 (업로드 시 덮어씀)
    ├── .htaccess               # 디렉토리 인덱싱 차단
    └── db/.htaccess            # DB 파일 직접 접근 차단
```

---

## 버전 이력

| 버전 | 날짜 | 내용 |
|------|------|------|
| V1 | 2026-03-19 | 최초 구현 — 버전 확인, 청크 다운로드, 업로드, 디바이스 현황 |
| V1 | 2026-03-20 | `index.html` HEX→BIN 자동 변환 기능 추가 (Intel HEX 클라이언트 파싱) |
| V1 | 2026-03-20 | `upload.php` 서버 500 오류 개선 — 디렉토리 자동 생성, 상세 오류 메시지 반환 |

---

## 관련 펌웨어 설정

디바이스 펌웨어(`lib/server.h`)의 OTA 경로 설정:

```c
#define DEFAULT_OTA_API_PATH "/CTP280_OTA/CTP280_OTA_V1/api"
```

---

*Copyright SUNTECH, 2026*
