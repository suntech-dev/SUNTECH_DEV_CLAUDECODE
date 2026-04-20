# 06. 화면 상세 설명

## 화면 목록

| 경로 | 파일 | 설명 |
|---|---|---|
| `/#/` | `HomeView.vue` | 메인 화면 (상태 표시, 진입 관문) |
| `/#/make` | `LockMakeView.vue` | 코드 생성 화면 |
| `/#/setting` | `SettingView.vue` | 사용자 이름 설정 화면 |

---

## HomeView.vue (`/#/`)

### 역할
- 상단에 SUNTECH 로고 표시
- 사용자 이름, 디바이스 ID(앞 16자 + …), 승인 상태 표시
- 승인되지 않은 상태에서 MAKE 버튼 비활성화 (접근 차단)
- 앱 시작 시 자동 등록 + 폴링 시작
- 오류 상태 시 오류 메시지 박스 + RETRY 버튼 표시
- 최초 접속 시 홈 화면 추가 안내 팝업 (InstallBanner)

### 주요 UI 요소

| UI 요소 | 표시 내용 |
|---|---|
| SUNTECH 로고 (`logo-topbar`) | 상단 좌측, `logo_suntech.png`, width 80px, opacity 0.85 |
| 자물쇠 SVG 아이콘 (`hero-icon`) | Blue glow 효과 (`rgba(88,166,255,0.4)`), 브랜드 아이콘 |
| ST500 타이틀 (`hero-title`) | 그라디언트 텍스트 (`#e6edf3 → #58a6ff → #bc8cff`), 48px, weight 900 |
| LockMaker 서브타이틀 | 12px, muted, letter-spacing 5px |
| 정보 카드 | NAME / DEVICE / STATUS 3행, border-radius 12px |
| DEVICE 표시 | `shortDeviceId` = UUID 앞 16자 + `…` (monospace) |
| STATUS 배지 | 컬러 원형 점 + 상태 텍스트 (pill 형태) |
| 오류 박스 | `status === 'error'` 일 때만 표시, 붉은 테두리 메시지 |
| MAKE 버튼 | 전체 너비 56px, 파란 그라디언트, 잠금 아이콘 포함 |
| RETRY 버튼 | `status === 'error'` 일 때만 표시, 노란 outline 48px |
| SETTING 버튼 | 전체 너비 48px, outline 스타일 |
| FINISH 버튼 | 최하단 ghost 텍스트 버튼 |

### STATUS 배지 색상 (GitHub Dark 팔레트)

| 상태 | CSS 클래스 | 배지 배경 | 텍스트/점 색 | 점 애니메이션 |
|---|---|---|---|---|
| INIT | `.badge-init` | `rgba(139,148,158,0.12)` | `#8b949e` | 없음 |
| WAITING | `.badge-waiting` | `rgba(210,153,34,0.12)` | `#d29922` | pulse 1.4초 반복 |
| APPROVED | `.badge-approved` | `rgba(63,185,80,0.12)` | `#3fb950` | 없음 |
| ERROR | `.badge-error` | `rgba(248,81,73,0.12)` | `#f85149` | 없음 |

### 생명주기

```javascript
onMounted(() => store.init())
onUnmounted(() => store.stopPolling())
```

### 라우팅

```javascript
goMake()    → router.push('/make')
goSetting() → router.push('/setting')
finish()    → window.history.back()
```

---

## LockMakeView.vue (`/#/make`)

### 역할
- Old Code + Lock Day 입력받아 New Code 산출
- 알고리즘 실행 (클라이언트 사이드)
- store 미사용 (완전히 독립적인 화면)

### 주요 UI 요소

| UI 요소 | 설명 |
|---|---|
| 상단 바 (height 56px) | 뒤로가기 버튼(36px) + "LockMaker" 타이틀(20px, bold) |
| OLD CODE 카드 | 숫자 입력 필드 (8~9자리), font-size 24px, letter-spacing 3px |
| LOCK DAY 카드 | 숫자 입력 + UnLock 커스텀 토글 스위치 |
| NEW CODE 카드 | 결과 표시 (생성 전: `—` 회색 / 생성 후: 초록 `#3fb950`) |
| MAKE 버튼 | 파란 그라디언트, 전체 너비, height 54px |
| BACK 버튼 | outline 스타일, height 46px |

### UnLock 토글 스위치

```vue
<!-- 커스텀 toggle (기본 checkbox 숨김) -->
<label class="unlock-toggle">
  <input v-model="isUnlock" type="checkbox" class="sr-only" />
  <span class="toggle-track" :class="{ 'toggle-on': isUnlock }">
    <span class="toggle-thumb" />
  </span>
  UnLock
</label>
```
- `isUnlock === true` → lockDay 입력 비활성화, `day = 151` 고정

### 유효성 검사

```javascript
if (code.length < 8)   → '8자리 이상 입력' 토스트
if (!lockDay && !unlock) → '잠금 일수 입력' 토스트
```

### 라우팅

```javascript
goBack() → router.push('/')
```

---

## SettingView.vue (`/#/setting`)

### 역할
- 사용자 이름 입력 및 저장
- 저장 시 서버에 디바이스 등록 요청 (`saveName → register`)

### 주요 UI 요소

| UI 요소 | 설명 |
|---|---|
| 상단 바 | 뒤로가기 버튼 + "Information" 타이틀 |
| NAME 입력 카드 | 현재 이름 자동 로드, 최대 30자 |
| SAVE 버튼 | Cyan 그라디언트, 저장 + 0.8초 후 홈 이동 |
| BACK 버튼 | outline 스타일 |

### 동작 흐름

```
이름 입력 → [SAVE]
  → store.saveName(name)
    → localStorage 저장
    → register() → 서버 send_device 호출
  → '저장되었습니다' 토스트
  → 0.8초 후 HomeView로 이동
```

---

## InstallBanner 컴포넌트

### 파일 위치
`src/components/InstallBanner.vue`  
`src/composables/useInstallPrompt.js`

### 역할
PWA 홈 화면 추가를 유도하는 bottom sheet 팝업.  
플랫폼 감지 후 적절한 안내 방식 선택.

### 표시 조건

| 조건 | 동작 |
|---|---|
| 이미 standalone 모드로 실행 중 | 표시 안 함 |
| `lm_install_dismissed` 키가 localStorage에 있음 | 표시 안 함 |
| `beforeinstallprompt` 이벤트 발생 (Android/Chrome/Edge) | native 설치 팝업 표시 |
| iOS Safari 감지 | 수동 안내 팝업 표시 |

### 플랫폼별 UI

**Android / PC Chrome / PC Edge (native)**
- "설치하기" 버튼 → `deferredPrompt.prompt()` 호출
- "나중에" 버튼 → dismiss (localStorage 저장)

**iOS Safari**
- 3단계 수동 안내:
  1. Safari 하단 공유 버튼 탭
  2. '홈 화면에 추가' 선택
  3. 오른쪽 상단 '추가' 탭
- "확인" 버튼 → dismiss

### dismiss 동작
```javascript
function dismiss() {
  dismissed.value = true
  localStorage.setItem('lm_install_dismissed', '1')
}
```
한번 dismiss하면 영구적으로 다시 표시 안 됨.

### 애니메이션
- `<Teleport to="body">` — 앱 스크롤 컨테이너 외부에 렌더링
- 슬라이드 업: `cubic-bezier(0.34, 1.56, 0.64, 1)` (bounce 효과)
- 오버레이 배경 클릭 시 dismiss

---

## 공통 디자인 시스템 (GitHub Dark)

> v1.1.0 리디자인 적용 (2026-04-18). CSS 변수는 `src/style.css`에 정의.

### CSS 변수 팔레트

| CSS 변수 | 색상값 | 용도 |
|---|---|---|
| `--bg` | `#0d1117` | 페이지 배경 (GitHub Dark 기본) |
| `--surface` | `#161b22` | 카드·상단바 배경 |
| `--surface2` | `#21262d` | 비활성 버튼·입력필드 배경 |
| `--border` | `#30363d` | 구분선·테두리 |
| `--text` | `#e6edf3` | 기본 텍스트 |
| `--text-muted` | `#8b949e` | 보조 텍스트 |
| `--accent` | `#58a6ff` | 포인트 컬러 (파란색, MAKE 버튼 등) |
| `--accent2` | `#3fb950` | 승인 상태 (초록색) |
| `--warn` | `#d29922` | 대기 상태 (노란색) |
| `--danger` | `#f85149` | 오류 상태 (빨간색) |
| `--purple` | `#bc8cff` | Hero 타이틀 그라디언트 끝 |
| `--cyan` | `#39d5b6` | (정의됨, 현재 미사용) |

### 배경

```css
/* 단색 배경 (그라디언트 없음) */
background: var(--bg); /* = #0d1117 */
```

### 카드

```css
background: var(--surface);  /* #161b22 — glassmorphism 없음 */
border: 1px solid var(--border);  /* #30363d */
border-radius: 12px;
/* backdrop-filter 없음 — 기본 카드는 불투명 */
```

> InstallBanner의 `.sheet`는 예외: `background: #0f2033`, `border-radius: 24px 24px 0 0`

### 버튼 규격

| 버튼 종류 | 높이 | 스타일 |
|---|---|---|
| Primary MAKE (HomeView) | 56px | `linear-gradient(135deg, #58a6ff → #3b82f6)` + glow |
| Primary MAKE/SAVE (LockMake/Setting) | 54px | 동일 그라디언트 |
| Secondary SETTING | 48px | `transparent` border outline |
| Secondary BACK | 46px | `transparent` border outline |
| RETRY | 48px | `rgba(210,153,34,0.08)` 노란 outline |
| FINISH | auto | ghost 텍스트, color `#6e7681` |

### Hero 타이틀 그라디언트

```css
background: linear-gradient(135deg, #e6edf3 0%, #58a6ff 55%, #bc8cff 100%);
-webkit-background-clip: text;
-webkit-text-fill-color: transparent;
```

### 토스트 메시지

```vue
<transition name="toast">
  <div v-if="toastMsg" class="toast">{{ toastMsg }}</div>
</transition>
```
- `position: fixed; bottom: 36px; left: 50%`
- 다크 배경 (`rgba(15,23,42,0.96)`) + 흰 글씨
- 0.3초 fade + 8px slide-up 효과
- 2초 후 자동 사라짐
