# 05. 상태 관리 (Pinia Store)

## 파일 위치

`src/stores/device.js`

---

## 스토어 구조

```javascript
export const useDeviceStore = defineStore('device', () => { ... })
```

Pinia **Composition API 스타일** (Options API 아님).

---

## 상태(State)

| 변수 | 타입 | 초기값 | 설명 |
|---|---|---|---|
| `deviceId` | `ref<string>` | `''` → `init()`에서 비동기 결정 | 디바이스 고유 식별자 (핑거프린트 기반) |
| `userName` | `ref<string>` | localStorage 또는 `''` | 사용자 이름 |
| `status` | `ref<string>` | `STATUS.INIT` | 현재 승인 상태 |
| `errorMsg` | `ref<string>` | `''` | 마지막 오류 메시지 |
| `pollTimer` | `let` (비반응) | `null` | setInterval 핸들 |

### STATUS 상수

```javascript
export const STATUS = {
  INIT:     'init',      // 초기 (이름 미입력 또는 등록 전)
  WAITING:  'waiting',  // 서버 등록 완료, 관리자 승인 대기
  APPROVED: 'approved', // 승인 완료 → MAKE 버튼 활성화
  ERROR:    'error'     // 서버 통신 오류
}
```

---

## 계산된 상태(Computed)

| 이름 | 반환 타입 | 설명 |
|---|---|---|
| `isApproved` | `boolean` | status === 'approved' |
| `statusLabel` | `string` | 화면 표시용 한글 상태 문자열 |

```javascript
// statusLabel 매핑
STATUS.INIT     → 'Not registered'
STATUS.WAITING  → 'Wait approval...'
STATUS.APPROVED → 'OK'
STATUS.ERROR    → 'Server error'
```

---

## 함수(Actions)

### `init()`
```javascript
async function init() {
  // Device ID를 비동기로 결정 (핑거프린트 계산 포함)
  deviceId.value = await resolveDeviceId()
  if (!userName.value) return  // 이름 없으면 아무것도 안 함 → /setting 유도
  await register()
}
```
- **호출 위치**: `HomeView.vue` `onMounted()`
- `resolveDeviceId()` 실행 후 이름 확인 → 자동 등록 시도
- `deviceId`는 `init()` 호출 전까지 `''` (비동기 초기화)

---

### `saveName(name)`
```javascript
async function saveName(name) {
  if (!name?.trim()) return
  userName.value = name.trim()
  localStorage.setItem('lm_user_name', userName.value)
  await register()
}
```
- **호출 위치**: `SettingView.vue` save 버튼
- 이름 저장 + 서버 등록 동시 처리

---

### `register()` (내부)
```javascript
async function register() {
  try {
    const res = await registerDevice(deviceId.value, userName.value)
    if (res?.msg === 'success') {
      status.value = STATUS.WAITING
      startPolling()
    }
  } catch (e) {
    status.value = STATUS.ERROR
    // "Failed to fetch" → 사용자 친화적 한글 메시지로 변환
    errorMsg.value = e.message?.includes('Failed to fetch')
      ? '서버에 연결할 수 없습니다'
      : (e.message ?? '알 수 없는 오류')
  }
}
```

---

### `retry()`
```javascript
async function retry() {
  errorMsg.value = ''
  status.value = STATUS.INIT
  await register()
}
```
- **호출 위치**: `HomeView.vue` RETRY 버튼 (`store.status === 'error'` 일 때만 표시)
- STATUS.ERROR 상태에서 재등록 시도
- errorMsg 초기화 후 register() 재호출

---

### `startPolling()` / `stopPolling()`
```javascript
// 1초 간격 폴링
pollTimer = setInterval(async () => {
  const res = await getDeviceStatus(deviceId.value)
  if (res?.msg === 'approve') {
    status.value = STATUS.APPROVED
    stopPolling()
  }
}, 1000)
```
- **폴링 중지 조건**: 승인 완료 또는 `HomeView.vue` `onUnmounted()`
- 오류 발생 시 에러를 삼키고 다음 사이클에 재시도

---

## 저장소 키 및 Device ID 결정 전략

### 저장소 키

| 저장소 | 키 | 설명 |
|---|---|---|
| `localStorage` | `lm_device_id` | 핑거프린트 기반 UUID (1순위) |
| `IndexedDB` (`lm_store`) | `device_id` | localStorage 초기화 시 복구용 (2순위) |
| `localStorage` | `lm_user_name` | 사용자 이름 |

### `resolveDeviceId()` — 3단계 우선순위

```
1. localStorage에 lm_device_id 있으면 → 그대로 사용 + IndexedDB 동기화
2. IndexedDB에 device_id 있으면      → localStorage 복원 후 반환
3. 둘 다 없으면                       → 하드웨어 핑거프린트 계산 → 양쪽 저장
```

### 핑거프린트 구성 신호

```
CPU 코어수(hardwareConcurrency) + RAM(deviceMemory)
+ 화면 해상도·색상깊이 + OS 플랫폼 + 언어 + 타임존
+ Canvas 렌더링 지문 (GPU 기반)
+ WebGL UNMASKED_RENDERER (GPU 모델)
──────────────────────────────────────────────────
SHA-256 → UUID v4 형식 문자열
```

> 동일 기기 + 동일 브라우저라면 브라우저 데이터 전체 초기화 후에도 항상 동일한 ID 재생성.  
> GPU 드라이버 업데이트 등 극히 드문 경우에만 ID 변경 가능성 있음.

### localStorage / IndexedDB 삭제 시 영향

| 상황 | 결과 |
|---|---|
| localStorage만 삭제 | IndexedDB에서 복구 → 동일 ID 유지 |
| IndexedDB만 삭제 | localStorage에서 복구 → 동일 ID 유지 |
| 둘 다 삭제 | 핑거프린트 재계산 → **동일 기기라면 동일 ID 복원** |
| `lm_user_name` 삭제 | 이름 초기화 → /setting에서 재입력 |

---

## 스토어 사용법

```javascript
// 컴포넌트에서 import
import { useDeviceStore, STATUS } from '@/stores/device.js'

const store = useDeviceStore()

// 상태 읽기 (반응형)
store.userName        // ref
store.status          // ref
store.isApproved      // computed
store.statusLabel     // computed

// 액션 호출
await store.init()
await store.saveName('홍길동')
await store.retry()   // STATUS.ERROR 상태에서 재시도
store.stopPolling()
```

---

## 주의 사항

1. `pollTimer`는 `ref`가 아닌 `let` → 반응형 아님 (의도적)
2. `HomeView` `onUnmounted()`에서 반드시 `store.stopPolling()` 호출해야 메모리 누수 방지
3. `register()` 실패 시 status가 `ERROR`로 바뀜 → 재시도는 `retry()` 사용 (`init()` 재호출 불필요)
4. `localStorage` 키 `lm_install_dismissed` — InstallBanner 팝업 영구 비표시 여부 (별도 관리)
