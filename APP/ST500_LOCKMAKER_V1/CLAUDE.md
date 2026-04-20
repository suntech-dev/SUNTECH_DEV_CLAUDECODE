# CLAUDE.md — ST500 LockMaker PWA

> 이 파일은 Claude AI가 컨텍스트 초기화 후에도 이 프로젝트를 즉시 파악할 수 있도록  
> 가장 중요한 정보만 압축 정리한 핵심 기억 파일입니다.

---

## 한 줄 요약

**SUNTECH 산업용 기계 잠금 코드 생성 PWA** — Android 전용 앱(`lockmaker_211206`)을  
Vue 3 + Vite PWA로 리뉴얼. iOS/Android/PC 모두 지원.

---

## 프로젝트 신원

| 항목 | 값 |
|---|---|
| 폴더 | `C:\SUNTECH_DEV_CLAUDECODE\APP\ST500_LOCKMAKER_V1` |
| 버전 | 1.0.0 |
| 스택 | Vue 3 + Vite 8 + Pinia 3 + Vue Router 4 + vite-plugin-pwa |
| 언어 | JavaScript (ES Modules, `<script setup>` Composition API) |
| 로컬 API | `http://localhost/dev/ST500_LOCKMAKER/api/index.php` |
| 운영 서버 IP | `49.247.27.154` |
| 운영 앱 URL | `http://49.247.27.154/st500/lockmaker` |
| 운영 서버 API | `http://49.247.27.154/api/st500/st500_api.php` (서버 구성 후 확인) |
| 원본 앱 | `APP/lockmaker_211206/` (Kotlin/Java, 퇴사자 작성, 문서 없음) |

---

## 코드 작성 규칙

- **들여쓰기**: 4칸 스페이스
- **코드**: 영문 작성, 핵심 섹션만 한글 주석
- **이모지**: 사용 안 함 (사용자가 명시적으로 요청한 경우에만 허용)
- **간결성**: 최대한 간결한 코드로 유지보수성 우선
- **외부 CDN**: Bootstarp, datatables 사용 금지. 순수CSS 우선 적용. 외부 CDN 사용 전에 먼저 묻고 허락 받아야 함.
- **경로**: 모든 경로는 `$_SERVER['SCRIPT_NAME']` 기반 동적 경로 사용.

---

## 파일 구조 (핵심만)

```
src/
├── main.js              ← Pinia + Router 등록
├── App.vue              ← <RouterView /> 래퍼만 있음
├── style.css            ← 전역 리셋 + max-width 540px
├── router/index.js      ← Hash History, 경로 3개
├── services/api.js      ← fetch() 래퍼 (registerDevice, getDeviceStatus)
├── stores/device.js     ← Pinia (디바이스 ID·이름·상태·폴링)
└── views/
    ├── HomeView.vue     ← 메인 (상태 표시, MAKE/SETTING)
    ├── LockMakeView.vue ← 코드 생성 알고리즘 실행
    └── SettingView.vue  ← 이름 입력·저장
```

---

## 라우팅 구조

```javascript
// Hash History 사용 — URL: /#/경로
/#/          → HomeView
/#/make      → LockMakeView
/#/setting   → SettingView
```

**Hash History를 쓰는 이유**: 서버 사이드 설정 없이 정적 파일 호스팅에서 동작.

---

## Pinia 스토어 (`stores/device.js`)

### 상태 상수
```javascript
STATUS = { INIT, WAITING, APPROVED, ERROR }
```

### 저장소 키
```
lm_device_id   → 하드웨어 핑거프린트 기반 UUID (localStorage + IndexedDB 이중 저장)
lm_user_name   → 사용자 이름
```

> **Device ID 결정 우선순위**: localStorage → IndexedDB → 핑거프린트 재계산  
> 브라우저 데이터 초기화 후에도 동일 기기·브라우저라면 항상 동일 ID 복원.

### 핵심 흐름
```
init() → resolveDeviceId() (비동기, 핑거프린트 계산 포함)
       → 이름 없으면 /setting 유도
       → register() → send_device API
           → msg='success' → STATUS.WAITING + startPolling()
               → 1초마다 get_device 폴링
               → msg='approve' → STATUS.APPROVED → MAKE 버튼 활성화
```

### 중요 주의사항
- `HomeView.vue` `onUnmounted()`에서 반드시 `store.stopPolling()` 호출해야 함
- `pollTimer`는 `ref` 아닌 `let` (비반응형 의도적)

---

## 관리페이지 (Admin)

| 항목 | 값 |
|---|---|
| 소스 경로 | `C:\SUNTECH_DEV_CLAUDECODE\WEB\ST500_LOCKMAKER\admin\` |
| 로컬 URL | `http://localhost/dev/ST500_LOCKMAKER/admin/` |
| 구 관리페이지 | `WEB/ST500_LOCKMAKER/old_admin/` (보존, 신규 사용 안함) |
| 스타일 | 순수 CSS/JS — Bootstrap/DataTables/jQuery 없음 |
| 스타일 참조 | `WEB/CTP280_API/log_embroidery.php` |
| 주요 화면 | `device.php` (디바이스 승인), `log.php` (API 로그) |
| AJAX 방식 | `?ajax=1` — 같은 파일 내 인라인 처리 |

---

## 신규 API 서버 (로컬 Laragon)

| 항목 | 값 |
|---|---|
| 소스 경로 | `C:\SUNTECH_DEV_CLAUDECODE\WEB\ST500_LOCKMAKER\api\` |
| 로컬 URL | `http://localhost/dev/ST500_LOCKMAKER/api/index.php` |
| DB | MySQL **5.7.44**, DB: `suntech_st500` |
| 주요 테이블 | `lm_device` (디바이스), `lm_logs` (API 로그) |
| 구 API 경로 | `WEB/ST500_LOCKMAKER/old_api/` (보존, 신규 사용 안함) |
| 스키마 파일 | `WEB/ST500_LOCKMAKER/api/sql/schema.sql` |

---

## API 명세 (서버)

| 용도 | 요청 |
|---|---|
| 디바이스 등록 | `GET ?code=send_device&device_id=UUID&name=이름` |
| 승인 상태 조회 | `GET ?code=get_device&device_id=UUID` |

응답 형태: `[{"code":"00","msg":"success|approve"}]`

---

## 잠금 코드 알고리즘 (`LockMakeView.vue` generate 함수)

```
Old Code(9자리) → n1/n2/n3 3분할
각 부분에 가중치 계수 적용 후 mod 1000 → c1/c2/c3
(c1+day)%1000, (c2+day)%1000, (c3+day)%1000
결과: c1*1000000 + c2*1000 + c3 (9자리 패딩)
```

가중치 계수:
- n1→c1: `[3.305982, 2.358196, 1.141059, 6.78213]`
- n2→c2: `[3.219283, 1.153023, 2.019283, 8.23143]`
- n3→c3: `[1.113569, 9.123123, 7.213213, 6.12374]`

UnLock 모드: `lockDay = 151` 고정

> ⚠️ 이 알고리즘은 클라이언트에 노출됨. 향후 PHP 서버 이전 계획.

---

## 개발 명령

```bash
# Vite 개발 서버 (http://localhost:5173) — 핫 리로드, 빠른 개발용
npm run dev

# 프로덕션 빌드 (dist/ 폴더 생성)
npm run build

# 빌드 결과 로컬 미리보기
npm run preview
```

---

## 로컬 개발 환경 (Laragon)

**개발 서버**: Laragon (Apache + PHP 7.4.33)을 정적 파일 서버로 사용.

### 경로 및 URL

| 항목 | 값 |
|---|---|
| Laragon www 루트 | `C:\laragon\www\` |
| 이 앱 배포 경로 | `C:\laragon\www\app\ST500_LOCKMAKER_V1\` |
| 로컬 접속 URL | `http://localhost/app/ST500_LOCKMAKER_V1/` |
| Vite 개발 서버 URL | `http://localhost:5173/` |

### Laragon 배포 방법 (빌드 → 복사)

```bash
# 1. 프로덕션 빌드
cd C:\SUNTECH_DEV_CLAUDECODE\APP\ST500_LOCKMAKER_V1
npm run build

# 2. dist/ 내용을 Laragon www에 복사
# (PowerShell)
Copy-Item -Recurse -Force dist\* C:\laragon\www\app\ST500_LOCKMAKER_V1\

# 또는 (Git Bash)
cp -r dist/* /c/laragon/www/app/ST500_LOCKMAKER_V1/
```

> **주의**: `dist/` 폴더를 통째로 복사하는 게 아니라, `dist/` **안의 내용**을 대상 폴더에 복사.

### 개발 방식 선택 기준

| 상황 | 사용 서버 | URL |
|---|---|---|
| UI 개발 중 (핫 리로드 필요) | Vite dev server | `http://localhost:5173/app/ST500_LOCKMAKER_V1/` |
| 실제 브라우저 PWA 동작 확인 | Laragon (빌드 후 배포) | `http://localhost/app/ST500_LOCKMAKER_V1/` |
| iOS Safari 실기기 테스트 | Laragon + 로컬 IP | `http://192.168.x.x/app/ST500_LOCKMAKER_V1/` |

> Vite dev server는 Service Worker가 동작하지 않음.  
> PWA 동작(오프라인, 홈 화면 추가) 확인은 반드시 Laragon 빌드 배포로 테스트.

### Laragon 환경 정보 (고정값 — 재확인 불필요)

| 항목 | 값 |
|---|---|
| PHP | 7.4.33 (`C:\laragon\bin\php\php-7.4.33-nts-Win32-vc15-x64\`) |
| MySQL | **5.7.44** — Laragon 내장 (root / 비밀번호 없음) |
| WEB 프로젝트 junction | `C:\laragon\www\dev` → `C:\SUNTECH_DEV_CLAUDECODE\WEB` |
| Apache 설정 | `C:\laragon\bin\apache\httpd-2.4.66-260107-Win64-VS18\conf\` |

---

## Playwright 테스트 자동화

Playwright를 사용한 브라우저 테스트는 **사용자 허락 없이 자동 진행**.

### 실행 환경 (고정값 — 재확인 불필요)

| 항목 | 값 |
|---|---|
| Node.js | v22.18.0 |
| Playwright | v1.58.2 |
| 설치 위치 | `C:\SUNTECH_DEV_CLAUDECODE\node_modules\playwright` |
| 실행 바이너리 | `C:\SUNTECH_DEV_CLAUDECODE\node_modules\.bin\playwright` |
| Chromium | `C:\Users\luvsd\AppData\Local\ms-playwright\chromium-1208` |

### 스크린샷 규칙

- **저장 경로**: `C:\SUNTECH_DEV_CLAUDECODE\.palywright_screen_shot\` (절대경로 고정)
- **파일 형식**: JPEG (`.jpeg`) — `type: 'jpeg'` 명시 필수
- 이 폴더는 `.gitignore` 등록됨

### 실행 규칙

- **작업 디렉토리**: 반드시 `C:\SUNTECH_DEV_CLAUDECODE\` 에서 `node script.js` 로 실행
- **브라우저**: `chromium.launch({ headless: true })` — 항상 headless
- 이 앱 테스트 시 URL: `http://localhost/app/ST500_LOCKMAKER_V1/` (Laragon 배포 후)

### Playwright 스크립트 템플릿

```javascript
// 실행: node script.js  (C:\SUNTECH_DEV_CLAUDECODE\ 에서)
const { chromium } = require('playwright');

(async () => {
    const browser = await chromium.launch({ headless: true });
    const page = await browser.newPage();
    await page.setViewportSize({ width: 390, height: 844 }); // iPhone 14 기준

    await page.goto('http://localhost/app/ST500_LOCKMAKER_V1/');
    await page.waitForLoadState('networkidle');

    await page.screenshot({
        path: 'C:\\SUNTECH_DEV_CLAUDECODE\\.palywright_screen_shot\\lockmaker_home.jpeg',
        type: 'jpeg'
    });

    await browser.close();
})();
```

---

## 운영 서버 직접 접속 (SSH / SFTP)

사용자 요청 시 Claude가 직접 서버에 접속하여 배포·조회·명령 실행 가능. **사용자 허락 없이 자동 진행**.

### 접속 정보 (고정값 — 재확인 불필요)

| 항목 | 값 |
|---|---|
| 서버 IP | `49.247.27.154` |
| SSH 포트 | `22` |
| SSH 사용자 | `root` |
| SSH 비밀번호 | `suntech9304!` |
| MySQL 사용자 | `root` |
| MySQL 비밀번호 | `suntech9304!` |
| MySQL DB | `suntech_st500` |
| OS | CentOS 7 |
| Apache | 2.4.6 — DocumentRoot: `/var/www/html` |
| PHP | 7.4.33 |
| MySQL | 5.7.44 |

### 접속 방법 — Python paramiko (고정값 — 재확인 불필요)

> `sshpass` 미설치. `paramiko`(pip install 완료)로 접속한다.

```python
import paramiko, io, os

def get_client():
    client = paramiko.SSHClient()
    client.set_missing_host_key_policy(paramiko.AutoAddPolicy())
    client.connect('49.247.27.154', username='root', password='suntech9304!', timeout=15)
    return client

# 명령 실행
client = get_client()
_, out, err = client.exec_command('명령어')
print((out.read() + err.read()).decode())
client.close()

# 파일 업로드 (SFTP)
client = get_client()
sftp = client.open_sftp()
sftp.put('로컬경로', '원격경로')          # 단일 파일
sftp.putfo(io.BytesIO(b'내용'), '경로')   # 메모리 → 파일
sftp.close()
client.close()
```

### 운영 서버 디렉토리 구조

```
/var/www/html/
├── api/
│   └── st500/
│       ├── st500_api.php       ← LockMaker API 진입점
│       ├── .env                ← DB 접속 정보
│       ├── lib/
│       │   ├── db.php
│       │   └── helpers.php
│       └── v1/
│           ├── send_device.php
│           └── get_device.php
└── st500/
    └── lockmaker/              ← Vue PWA dist 파일
        ├── index.html
        ├── sw.js
        ├── manifest.webmanifest
        └── assets/
```

### 운영 배포 절차 (전체)

```python
# 1. DB 테이블 생성 (최초 1회)
sftp.putfo(io.BytesIO(schema_sql_bytes), '/tmp/schema.sql')
client.exec_command('mysql -uroot -psuntech9304! < /tmp/schema.sql')

# 2. API 파일 업로드
sftp.put(local, '/var/www/html/api/st500/파일명')

# 3. Vue 빌드 (로컬 — MSYS_NO_PATHCONV 필수)
# Bash: MSYS_NO_PATHCONV=1 npm run build -- --base=/st500/lockmaker/

# 4. dist 파일 업로드
for root, dirs, files in os.walk(local_dist):
    rel = os.path.relpath(root, local_dist).replace(os.sep, '/')
    remote_dir = '/var/www/html/st500/lockmaker' if rel == '.' else '/var/www/html/st500/lockmaker/' + rel
    for f in files:
        sftp.put(os.path.join(root, f), remote_dir + '/' + f)

# 5. 동작 확인
client.exec_command('curl -s http://localhost/st500/lockmaker/')
```

### 빌드 명령 주의사항

| 환경 | 빌드 명령 | base 결과 |
|---|---|---|
| Laragon 로컬 | `npm run build` | `/app/ST500_LOCKMAKER_V1/` |
| 운영 서버 | `MSYS_NO_PATHCONV=1 npm run build -- --base=/st500/lockmaker/` | `/st500/lockmaker/` |

> `MSYS_NO_PATHCONV=1` 없이 `--base=/st500/lockmaker/` 전달 시 Git Bash가 Windows 경로로 변환 → 빌드 오류 발생.

---

## 디자인 색상 팔레트 (UI 수정 시 참조)

> **GitHub Dark 팔레트** — v1.1.0 리디자인 적용 (2026-04-18)

CSS 변수는 `src/style.css`에 정의됨.

| CSS 변수 | 색상값 | 용도 |
|---|---|---|
| `--bg` | `#0d1117` | 페이지 배경 |
| `--surface` | `#161b22` | 카드·컨테이너 배경 |
| `--surface2` | `#21262d` | 비활성 버튼 배경 |
| `--border` | `#30363d` | 구분선·테두리 |
| `--text` | `#e6edf3` | 기본 텍스트 |
| `--text-muted` | `#8b949e` | 보조 텍스트 |
| `--accent` | `#58a6ff` | 포인트 컬러 (파란색, MAKE 버튼 등) |
| `--accent2` | `#3fb950` | 승인 상태 (초록색) |
| `--warn` | `#d29922` | 대기 상태 (노란색) |
| `--danger` | `#f85149` | 오류 상태 (빨간색) |

Hero 타이틀 그라디언트: `linear-gradient(135deg, #e6edf3 → #58a6ff → #bc8cff)`

---

## 긴급 TODO (수정 필요 항목)

| 우선순위 | 항목 | 위치 |
|---|---|---|
| ✅ 완료 | PWA 아이콘 생성 (`icon-192.png`, `icon-512.png`) | `public/icons/` |
| ✅ 완료 | `apple-touch-icon` 링크 추가 | `index.html` |
| 🔴 높음 | 알고리즘 JS vs Kotlin 결과 동일 여부 실기기 검증 | `LockMakeView.vue` |
| 🟠 중간 | HTTPS 전환 후 `.env.production` 생성 | 프로젝트 루트 |
| 🟡 낮음 | 알고리즘을 PHP 서버 API로 이전 | `src/services/api.js` + 서버 |

---

## 원본 앱 대비 개선사항 (이미 완료)

- ✅ 메인스레드 네트워크 버그 → async/await 해결
- ✅ 파일 이중 열기/누수 → localStorage로 대체
- ✅ Activity 직접 인스턴스화 → Pinia store로 대체
- ✅ 서버 URL 4곳 하드코딩 → `.env` 단일 관리
- ✅ Android ID → 하드웨어 핑거프린트 UUID + localStorage/IndexedDB 이중 저장
- ✅ iOS 미지원 → PWA로 Safari 동작
- ✅ Deprecated AsyncTask → async/await
- ✅ 미사용 클래스 6개 제거 (data/ 패키지)

---

## 문서 목록

| 파일 | 내용 |
|---|---|
| `doc/01_overview.md` | 프로젝트 개요, 기술스택, 원본 앱 비교 |
| `doc/02_architecture.md` | 폴더구조, 데이터흐름, 상태머신 |
| `doc/03_api.md` | 서버 API 명세, 에러처리 |
| `doc/04_algorithm.md` | 잠금코드 알고리즘 전문, 원본 Kotlin 코드 |
| `doc/05_state_store.md` | Pinia 스토어 상세 (상태, 계산값, 함수) |
| `doc/06_screens.md` | 화면별 UI/UX, 색상 팔레트, 토스트 구현 |
| `doc/07_pwa_deploy.md` | PWA 빌드, 아이콘, Service Worker, 환경변수 |
| `doc/08_roadmap.md` | TODO 목록, 버그 추적, 버전 히스토리 |
| `doc/09_deploy_checklist.md` | **배포 시 실행 절차** (Laragon 테스트 → 운영 배포 체크리스트) |
| `doc/10_database.md` | DB 스키마 (MySQL 5.7.44, lm_device/lm_logs, 구 테이블 비교) |
| `doc/11_admin.md` | 관리페이지 명세 (순수CSS/JS, self-contained 구조) |
