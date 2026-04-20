# 02. 아키텍처 및 폴더 구조

## 전체 폴더 구조

```
ST500_LOCKMAKER_V1/
├── index.html                  ← 진입점 (PWA 메타태그 포함)
├── vite.config.js              ← 빌드 설정 (@ 별칭, PWA 플러그인)
├── package.json                ← 의존성 (vue, pinia, vue-router, vite-plugin-pwa)
├── .env                        ← 환경변수 (VITE_API_BASE_URL)
├── .gitignore
├── CLAUDE.md                   ← Claude AI 핵심 기억 파일
│
├── doc/                        ← 프로젝트 문서
│   ├── 01_overview.md
│   ├── 02_architecture.md
│   ├── 03_api.md
│   ├── 04_algorithm.md
│   ├── 05_state_store.md
│   ├── 06_screens.md
│   ├── 07_pwa_deploy.md
│   └── 08_roadmap.md
│
├── public/
│   └── icons/
│       ├── icon-192.png        ← PWA 홈 화면 아이콘 192×192 (Python/Pillow 생성)
│       └── icon-512.png        ← PWA 스플래시 아이콘 512×512 (Python/Pillow 생성)
│
├── src/
│   ├── main.js                 ← 앱 진입점 (Pinia + Router 등록)
│   ├── App.vue                 ← 루트 컴포넌트 (<RouterView /> 래퍼)
│   ├── style.css               ← 전역 스타일 (GitHub Dark 테마 리셋, #0d1117 배경, CSS 변수)
│   │
│   ├── router/
│   │   └── index.js            ← Vue Router (Hash 히스토리, 3개 경로)
│   │
│   ├── services/
│   │   └── api.js              ← 서버 HTTP 통신 모듈 (fetch 래퍼)
│   │
│   ├── stores/
│   │   └── device.js           ← Pinia 스토어 (디바이스 상태, 폴링, 재시도 로직)
│   │
│   ├── composables/
│   │   └── useInstallPrompt.js ← PWA 설치 팝업 로직 (iOS/Android/PC 분기)
│   │
│   ├── components/
│   │   └── InstallBanner.vue   ← 홈 화면 추가 안내 팝업 (bottom sheet)
│   │
│   └── views/
│       ├── HomeView.vue        ← 메인 화면 (상태 표시, MAKE/SETTING 버튼)
│       ├── LockMakeView.vue    ← 코드 생성 화면 (알고리즘 실행)
│       └── SettingView.vue     ← 설정 화면 (이름 입력·저장)
│
└── dist/                       ← 빌드 결과물 (npm run build 생성)
    ├── index.html
    ├── manifest.webmanifest
    ├── sw.js                   ← Service Worker (PWA 오프라인 지원)
    ├── workbox-*.js
    └── assets/
```

## 데이터 흐름

```
[브라우저]
    │
    ├─ localStorage
    │     ├── lm_device_id    (핑거프린트 UUID, 1순위)
    │     └── lm_user_name    (사용자 이름)
    │
    ├─ IndexedDB (lm_store)
    │     └── device_id       (lm_device_id 백업, 2순위)
    │
    └─ [Vue App]
           │
           ├─ [stores/device.js] ← Pinia 전역 상태
           │        │
           │        ├─ deviceId, userName, status, errorMsg
           │        ├─ isApproved (computed)
           │        ├─ statusLabel (computed)
           │        └─ 함수: init(), saveName(), register(), retry(), startPolling(), stopPolling()
           │                   │
           │                   └─ [services/api.js]
           │                           │
           │                           ├─ registerDevice(deviceId, name)  → POST-like GET
           │                           └─ getDeviceStatus(deviceId)       → GET 폴링
           │                                   │
           │                                   └─ [HTTP] → 서버 (49.247.27.154)
           │
           └─ [router/index.js] ← Hash 히스토리
                    ├── /#/          → HomeView.vue
                    ├── /#/make      → LockMakeView.vue
                    └── /#/setting   → SettingView.vue
```

## 상태 머신

```
앱 시작
  │
  ├─ userName 없음 → STATUS.INIT → 사용자를 /setting으로 유도 (MAKE 비활성)
  │
  └─ userName 있음
        │
        └─ register() 호출
              │
              ├─ 성공 (msg='success') → STATUS.WAITING + startPolling()
              │         │
              │         └─ 1초 폴링 getDeviceStatus()
              │                 │
              │                 └─ msg='approve' → STATUS.APPROVED → MAKE 버튼 활성화
              │
              └─ 실패 (네트워크 오류) → STATUS.ERROR
                        │
                        └─ retry() 호출 → STATUS.INIT → register() 재시도
```

## 컴포넌트 관계

```
App.vue
  └─ RouterView
        ├─ HomeView.vue
        │     ├─ logo-topbar (logo_suntech.png — ${baseUrl}로 로드)
        │     ├─ useDeviceStore() ← 상태 읽기 + init(), retry() 호출
        │     │     └─ shortDeviceId: UUID 앞 16자 + '…' 표시
        │     └─ InstallBanner.vue
        │           └─ useInstallPrompt() ← iOS/Android/PC 설치 팝업 처리
        │
        ├─ LockMakeView.vue
        │     └─ 독립 (store 불사용, 알고리즘 로컬 계산)
        │
        └─ SettingView.vue
              └─ useDeviceStore() ← saveName() 호출
```

## 중요 설계 결정 사항

### 1. Hash History 라우터 사용
```javascript
// router/index.js
history: createWebHashHistory()
```
- URL 형태: `https://도메인/#/make`
- **이유**: 서버 사이드 라우팅 설정 없이 정적 파일 서버에서 동작
- PWA를 웹서버에 단순 파일 복사로 배포 가능

### 2. 디바이스 ID 전략 (하드웨어 핑거프린트)

```javascript
// stores/device.js — resolveDeviceId()
// 1순위: localStorage / 2순위: IndexedDB / 3순위: 핑거프린트 재계산
```

**핑거프린트 구성**: CPU 코어수 + RAM + 화면해상도 + OS + 타임존 + Canvas + WebGL  
→ SHA-256 해시 → UUID 형식 문자열

- **목적**: MAC 주소처럼 기기 고유 ID 역할 (브라우저 보안 정책상 MAC 주소 직접 접근 불가)
- **안정성**: localStorage + IndexedDB 이중 저장 → 한쪽 초기화되어도 복구
- **완전 초기화 시**: 핑거프린트 재계산 → **동일 기기·브라우저라면 동일 ID 복원**
- Android ID 대체: 네이티브 권한 없이 고유 식별자 구현

### 3. `@` 경로 별칭
```javascript
// vite.config.js
alias: { '@': fileURLToPath(new URL('./src', import.meta.url)) }
```
- `@/views/HomeView.vue` = `src/views/HomeView.vue`
- 상대경로(`../../`) 없이 절대경로처럼 import 가능

### 4. 환경변수로 API URL 관리
```
.env → VITE_API_BASE_URL=http://localhost/dev/ST500_LOCKMAKER/api/index.php
```
- 코드에 URL 하드코딩 금지
- 배포 환경별로 `.env.production` 별도 관리 가능

### 5. Vite base 경로 설정
```javascript
// vite.config.js
base: '/app/ST500_LOCKMAKER_V1/'
```
- Laragon 배포 경로(`/app/ST500_LOCKMAKER_V1/`)와 일치시키기 위해 필수
- 개발 서버 URL: `http://localhost:5173/app/ST500_LOCKMAKER_V1/`
- 미설정 시 `/assets/...` 경로로 자산 요청 → 404 발생
