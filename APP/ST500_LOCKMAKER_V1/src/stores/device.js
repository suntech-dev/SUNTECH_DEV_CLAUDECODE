import { defineStore } from 'pinia'
import { ref, computed } from 'vue'
import { registerDevice, getDeviceStatus } from '@/services/api.js'

// ─── 하드웨어 핑거프린트 ──────────────────────────────────────────────────────
// MAC 주소 대신 브라우저에서 사용 가능한 기기 고유 신호를 조합해
// SHA-256 해시로 결정론적 UUID를 생성한다. localStorage가 삭제되어도
// 동일 기기·브라우저라면 항상 동일한 ID가 재생성된다.

function _canvasSignal() {
  try {
    const c = document.createElement('canvas')
    const ctx = c.getContext('2d')
    ctx.textBaseline = 'top'
    ctx.font = '14px Arial'
    ctx.fillStyle = '#f60'
    ctx.fillRect(125, 1, 62, 20)
    ctx.fillStyle = '#069'
    ctx.fillText('ST500-LM\u{1F512}', 2, 15)
    ctx.fillStyle = 'rgba(102,204,0,0.7)'
    ctx.fillText('ST500-LM\u{1F512}', 4, 17)
    return c.toDataURL().slice(-60)
  } catch { return '' }
}

function _webglSignal() {
  try {
    const c = document.createElement('canvas')
    const gl = c.getContext('webgl') || c.getContext('experimental-webgl')
    if (!gl) return ''
    const ext = gl.getExtension('WEBGL_debug_renderer_info')
    return ext ? (gl.getParameter(ext.UNMASKED_RENDERER_WEBGL) || '') : ''
  } catch { return '' }
}

async function _sha256hex(str) {
  // crypto.subtle은 HTTPS 또는 localhost에서만 동작
  if (crypto.subtle) {
    const buf = await crypto.subtle.digest('SHA-256', new TextEncoder().encode(str))
    return Array.from(new Uint8Array(buf)).map(b => b.toString(16).padStart(2, '0')).join('')
  }
  // HTTP fallback: djb2 hash를 64자 hex로 확장
  let h = 5381
  for (let i = 0; i < str.length; i++) h = (Math.imul(h, 33) ^ str.charCodeAt(i)) >>> 0
  return h.toString(16).padStart(8, '0').repeat(8)
}

function _hexToUUID(hex) {
  return [
    hex.slice(0, 8),
    hex.slice(8, 12),
    '4' + hex.slice(13, 16),
    (parseInt(hex[16], 16) & 3 | 8).toString(16) + hex.slice(17, 20),
    hex.slice(20, 32),
  ].join('-')
}

async function computeFingerprint() {
  const signals = [
    navigator.hardwareConcurrency ?? 0,
    navigator.deviceMemory ?? 0,
    screen.width,
    screen.height,
    screen.colorDepth,
    navigator.platform ?? '',
    navigator.language ?? '',
    Intl.DateTimeFormat().resolvedOptions().timeZone ?? '',
    _canvasSignal(),
    _webglSignal(),
  ]
  return _hexToUUID(await _sha256hex(signals.join('||')))
}

// ─── IndexedDB 백업 저장소 ────────────────────────────────────────────────────
// localStorage가 초기화되어도 IndexedDB에서 복구한다.

const _IDB = 'lm_store'
const _KEY = 'device_id'

function _idbOpen() {
  return new Promise((resolve, reject) => {
    const req = indexedDB.open(_IDB, 1)
    req.onupgradeneeded = e => e.target.result.createObjectStore('kv')
    req.onsuccess = e => resolve(e.target.result)
    req.onerror   = () => reject(req.error)
  })
}

async function idbGet() {
  try {
    const db = await _idbOpen()
    return new Promise(resolve => {
      const r = db.transaction('kv', 'readonly').objectStore('kv').get(_KEY)
      r.onsuccess = () => resolve(r.result ?? null)
      r.onerror   = () => resolve(null)
    })
  } catch { return null }
}

async function idbSet(value) {
  try {
    const db = await _idbOpen()
    return new Promise(resolve => {
      const r = db.transaction('kv', 'readwrite').objectStore('kv').put(value, _KEY)
      r.onsuccess = () => resolve()
      r.onerror   = () => resolve()
    })
  } catch {}
}

// ─── Device ID 결정 (3단계 우선순위) ─────────────────────────────────────────
// 1순위: localStorage  (가장 빠름)
// 2순위: IndexedDB     (localStorage 초기화 시 복구)
// 3순위: 핑거프린트 재계산 (둘 다 없을 때 — 동일 기기면 항상 같은 값)

async function resolveDeviceId() {
  const ls = localStorage.getItem('lm_device_id')
  if (ls) {
    idbSet(ls)
    return ls
  }

  const idb = await idbGet()
  if (idb) {
    localStorage.setItem('lm_device_id', idb)
    return idb
  }

  let fp
  try {
    fp = await computeFingerprint()
  } catch {
    // computeFingerprint 실패 시 (예외적 환경) crypto.getRandomValues fallback
    const bytes = new Uint8Array(16)
    crypto.getRandomValues(bytes)
    bytes[6] = (bytes[6] & 0x0f) | 0x40
    bytes[8] = (bytes[8] & 0x3f) | 0x80
    const h = Array.from(bytes).map(b => b.toString(16).padStart(2, '0')).join('')
    fp = _hexToUUID(h)
  }
  localStorage.setItem('lm_device_id', fp)
  idbSet(fp)
  return fp
}

// ─── 상태 상수 ────────────────────────────────────────────────────────────────

export const STATUS = {
  INIT:     'init',     // 초기 (등록 전)
  WAITING:  'waiting',  // 등록 완료, 승인 대기
  APPROVED: 'approved', // 승인 완료 → MAKE 버튼 활성화
  ERROR:    'error',    // 서버 통신 오류
}

// ─── Pinia Store ──────────────────────────────────────────────────────────────

export const useDeviceStore = defineStore('device', () => {
  const deviceId  = ref('')
  const userName  = ref(localStorage.getItem('lm_user_name') ?? '')
  const status    = ref(STATUS.INIT)
  const errorMsg  = ref('')
  let   pollTimer = null

  const isApproved  = computed(() => status.value === STATUS.APPROVED)
  const statusLabel = computed(() => {
    switch (status.value) {
      case STATUS.INIT:     return 'Not registered'
      case STATUS.WAITING:  return 'Wait approval...'
      case STATUS.APPROVED: return 'OK'
      case STATUS.ERROR:    return 'Server error'
      default:              return ''
    }
  })

  async function saveName(name) {
    if (!name?.trim()) return
    userName.value = name.trim()
    localStorage.setItem('lm_user_name', userName.value)
    await register()
  }

  async function register() {
    try {
      const res = await registerDevice(deviceId.value, userName.value)
      if (res?.msg === 'success') {
        status.value = STATUS.WAITING
        startPolling()
      }
    } catch (e) {
      status.value   = STATUS.ERROR
      errorMsg.value = e.message?.includes('Failed to fetch')
        ? '서버에 연결할 수 없습니다'
        : (e.message ?? '알 수 없는 오류')
    }
  }

  async function retry() {
    errorMsg.value = ''
    status.value   = STATUS.INIT
    await register()
  }

  function startPolling() {
    stopPolling()
    pollTimer = setInterval(async () => {
      try {
        const res = await getDeviceStatus(deviceId.value)
        if (res?.msg === 'approve') {
          status.value = STATUS.APPROVED
          stopPolling()
        }
      } catch { /* 다음 사이클에 재시도 */ }
    }, 1000)
  }

  function stopPolling() {
    if (pollTimer) { clearInterval(pollTimer); pollTimer = null }
  }

  async function init() {
    // Device ID를 비동기로 결정 (핑거프린트 계산 포함)
    deviceId.value = await resolveDeviceId()
    if (!userName.value) return
    await register()
  }

  return {
    deviceId, userName, status, errorMsg,
    isApproved, statusLabel,
    saveName, init, retry, stopPolling,
  }
})
