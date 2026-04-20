# 07. PWA 설정 및 배포

## PWA 구성 요소

| 파일 | 역할 |
|---|---|
| `vite.config.js` | VitePWA 플러그인 설정 (manifest, workbox, base 경로) |
| `index.html` | Apple PWA 메타태그 + `apple-touch-icon` 링크 |
| `public/icons/icon-192.png` | 홈 화면 아이콘 192×192 (**완료**) |
| `public/icons/icon-512.png` | 스플래시 아이콘 512×512 (**완료**) |
| `src/components/InstallBanner.vue` | 홈 화면 추가 안내 팝업 UI |
| `src/composables/useInstallPrompt.js` | 플랫폼 감지 + 설치 로직 |
| `dist/manifest.webmanifest` | 빌드 시 자동 생성 |
| `dist/sw.js` | Service Worker (Workbox 자동 생성) |

---

## manifest 설정

```javascript
// vite.config.js
manifest: {
  name: 'ST500 LockMaker',
  short_name: 'LockMaker',
  description: 'SUNTECH 산업용 기계 잠금 코드 생성기',
  theme_color: '#00BCD4',
  background_color: '#ECECEC',
  display: 'standalone',       // 브라우저 UI 없이 앱처럼 표시
  orientation: 'portrait',     // 세로 고정
  start_url: '/',
  icons: [
    { src: 'icons/icon-192.png', sizes: '192x192', type: 'image/png' },
    { src: 'icons/icon-512.png', sizes: '512x512', type: 'image/png' }
  ]
}
```

---

## 아이콘 파일 (**완료**)

`public/icons/` 폴더에 두 파일 생성 완료:

| 파일명 | 크기 | 생성 방법 |
|---|---|---|
| `icon-192.png` | 192×192px | Python Pillow로 생성 (자물쇠 아이콘 + SUNTECH 로고) |
| `icon-512.png` | 512×512px | Python Pillow로 생성 (자물쇠 아이콘 + SUNTECH 로고) |

디자인: `#00BCD4` Cyan 배경 + 흰색 자물쇠 SVG + 하단 SUNTECH 로고

### iOS apple-touch-icon (**완료**)

```html
<!-- index.html <head> — 추가 완료 -->
<link rel="apple-touch-icon" href="/icons/icon-192.png" />
```

---

## Service Worker (Workbox)

```javascript
// vite.config.js workbox 설정
registerType: 'autoUpdate',  // 새 버전 감지 시 자동 업데이트
workbox: {
  globPatterns: ['**/*.{js,css,html,ico,png,svg}'],  // 프리캐시 파일
  runtimeCaching: [{
    urlPattern: /^https:\/\/.+\/api\//,  // HTTPS API만 캐싱
    handler: 'NetworkFirst',              // 네트워크 우선, 실패 시 캐시
    options: { networkTimeoutSeconds: 5 }
  }]
}
```

> ⚠️ 현재 API 서버가 HTTP이므로 runtimeCaching 정규식(`^https://`)에 매칭 안 됨.  
> HTTPS 전환 후 자동으로 캐싱 적용됨.

---

## 설치 안내 팝업 (InstallBanner) (**완료**)

최초 접속 시 자동으로 플랫폼을 감지하여 적절한 안내를 표시함:

| 플랫폼 | 감지 방법 | 안내 방식 |
|---|---|---|
| Android Chrome / PC Chrome·Edge | `beforeinstallprompt` 이벤트 | "설치하기" 버튼 → 네이티브 설치 다이얼로그 |
| iOS Safari | UserAgent 감지 | 3단계 수동 안내 (공유 버튼 → 홈 화면에 추가) |
| 이미 설치됨 (`standalone` 모드) | `display-mode: standalone` 감지 | 팝업 표시 안 함 |
| dismiss 후 재방문 | `localStorage['lm_install_dismissed']` | 팝업 표시 안 함 |

---

## iOS 홈 화면 추가 방법 (사용자 안내)

```
1. Safari에서 앱 URL 접속
2. 하단 공유 버튼(□↑) 탭
3. "홈 화면에 추가" 선택
4. 이름 확인 후 "추가" 탭
```

→ 이후 홈 화면에서 일반 앱처럼 실행 (브라우저 UI 없음, `display: standalone` 효과)

## Android 홈 화면 추가 방법

```
1. Chrome에서 앱 URL 접속
2. 주소창 우측 메뉴(⋮) 탭
3. "홈 화면에 추가" 또는 "앱 설치" 선택
```

---

## 빌드 명령

```bash
# 개발 서버 실행
npm run dev

# 프로덕션 빌드
npm run build

# 빌드 결과 로컬 미리보기
npm run preview
```

### 빌드 결과물 (`dist/` 폴더)

```
dist/
├── index.html              (메인 HTML)
├── manifest.webmanifest    (PWA 매니페스트)
├── sw.js                   (Service Worker)
├── workbox-*.js            (Workbox 런타임)
├── registerSW.js           (SW 등록 스크립트)
└── assets/
    ├── index-*.js          (번들된 JS)
    └── index-*.css         (번들된 CSS)
```

빌드 크기 (참고):
- JS: ~115KB (gzip: ~44KB)
- CSS: ~14KB (gzip: ~2.7KB)

---

## 웹서버 배포

### 기존 서버에 배포 (권장)

```bash
# 빌드
npm run build

# dist/ 폴더 내용을 웹서버 디렉토리에 복사
# 예: FTP 업로드 또는 SSH scp
scp -r dist/* root@49.247.27.154:/var/www/html/st500/lockmaker/
```

배포 URL: `http://49.247.27.154/st500/lockmaker`

### Apache 서버 설정 (.htaccess)

Hash 히스토리 사용 중이므로 서버 설정 불필요.  
(HTML5 History 방식이라면 아래 설정 필요했을 것)

```apache
# 불필요 (Hash History 사용 중이므로)
# RewriteRule ^(?!.*\.).*$ /index.html [L]
```

---

## 환경변수 관리

```
.env                        ← 개발 환경 (git에 포함, 민감정보 주의)
.env.production             ← 프로덕션 환경 (HTTPS URL로 변경)
```

```bash
# .env (개발)
VITE_API_BASE_URL=http://49.247.27.154/api/st500/st500_api.php

# .env.production (배포)
VITE_API_BASE_URL=https://도메인/api/st500/st500_api.php
```

> `npm run build` 시 `.env.production`이 `.env`보다 우선 적용됨.

---

## HTTPS 전환 체크리스트

- [ ] 서버에 SSL 인증서 발급 (Let's Encrypt 무료 가능)
- [ ] `.env.production` 생성 → `HTTPS://...` URL 설정
- [ ] `vite.config.js` `usesCleartextTraffic` 관련 설정 제거 (이미 없음)
- [ ] `index.html`에 CSP(Content Security Policy) 헤더 추가 검토
- [ ] workbox runtimeCaching 정규식 동작 확인
