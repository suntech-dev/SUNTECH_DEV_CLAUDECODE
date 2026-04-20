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
- 사용자 이름, 디바이스 ID, 승인 상태 표시
- 승인되지 않은 상태에서 MAKE 버튼 비활성화 (접근 차단)
- 앱 시작 시 자동 등록 + 폴링 시작
- 오류 상태 시 오류 메시지 + RETRY 버튼 표시
- 최초 접속 시 홈 화면 추가 안내 팝업 (InstallBanner)

### 주요 UI 요소

| UI 요소 | 표시 내용 |
|---|---|
| 자물쇠 SVG 아이콘 | Cyan glow 효과, 브랜드 아이콘 |
| ST500 타이틀 | 대형 bold 텍스트 |
| 정보 카드 (Glassmorphism) | NAME / DEVICE / STATUS 3행 |
| STATUS 배지 | 컬러 원형 점 + 상태 텍스트 (pill 형태) |
| MAKE 버튼 | 전체 너비, Cyan 그라디언트, 잠금 아이콘 포함 |
| SETTING 버튼 | 전체 너비, outline 스타일 |
| RETRY 버튼 | `status === 'error'` 일 때만 표시, 주황색 outline |
| 오류 박스 | `status === 'error'` 일 때만 표시, 붉은 배경 메시지 |
| FINISH 버튼 | 최하단 ghost 텍스트 버튼 |

### STATUS 배지 색상

| 상태 | 배지 색 | 점 애니메이션 |
|---|---|---|
| INIT | 회색 (`#94a3b8`) | 없음 |
| WAITING | 주황 (`#fbbf24`) | pulse (1.4초 반복) |
| APPROVED | 초록 (`#34d399`) | 없음 |
| ERROR | 빨강 (`#fca5a5`) | 없음 |

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
| 상단 바 | 뒤로가기 버튼 + "LockMaker" 타이틀 |
| OLD CODE 카드 | 숫자 입력 필드 (8~9자리) |
| LOCK DAY 카드 | 숫자 입력 + UnLock 커스텀 토글 스위치 |
| NEW CODE 카드 | 결과 표시 (생성 전: `—` / 생성 후: 초록 숫자) |
| MAKE 버튼 | Cyan 그라디언트, 전체 너비 |
| BACK 버튼 | outline 스타일 |

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

## 공통 디자인 시스템 (Dark Industrial Premium)

### 배경
```css
background: linear-gradient(160deg, #0d1b2a 0%, #0c2233 55%, #0d1b2a 100%);
```

### 색상 팔레트

| 용도 | 색상 |
|---|---|
| 페이지 배경 | `#0d1b2a` (다크 네이비) |
| 브랜드 Accent | `#06b6d4` (Cyan 500) |
| 버튼 그라디언트 | `#06b6d4` → `#0284c7` |
| 텍스트 Primary | `#f1f5f9` |
| 텍스트 Secondary | `#94a3b8` |
| 텍스트 Muted | `#64748b` |
| 카드 배경 | `rgba(255,255,255,0.06)` |
| 카드 테두리 | `rgba(255,255,255,0.10)` |
| 성공 (APPROVED) | `#10b981` / `#34d399` |
| 경고 (WAITING) | `#f59e0b` / `#fbbf24` |
| 오류 (ERROR) | `#ef4444` / `#fca5a5` |

### 카드 (Glassmorphism)
```css
background: rgba(255, 255, 255, 0.06);
backdrop-filter: blur(12px);
-webkit-backdrop-filter: blur(12px);
border: 1px solid rgba(255, 255, 255, 0.10);
border-radius: 16~18px;
box-shadow: 0 8px 32px rgba(0, 0, 0, 0.35);
```

### 버튼 규격

| 버튼 종류 | 높이 | 스타일 |
|---|---|---|
| Primary (MAKE/SAVE) | 58px | Cyan 그라디언트 + glow shadow |
| Secondary (SETTING/BACK) | 50px | outline (`rgba(255,255,255,0.13)`) |
| RETRY | 50px | 주황 outline |
| FINISH | auto | ghost 텍스트 |

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
