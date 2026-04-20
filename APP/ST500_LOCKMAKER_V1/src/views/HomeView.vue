<template>
    <div class="page">
        <!-- Hero -->
        <div class="hero">
            <div class="hero-glow" />
            <div class="hero-icon">
                <svg viewBox="0 0 64 72" fill="none" xmlns="http://www.w3.org/2000/svg">
                    <path d="M14 32V23C14 13.611 21.611 6 31 6h2c9.389 0 17 7.611 17 17v9"
                          stroke="currentColor" stroke-width="5" stroke-linecap="round"/>
                    <rect x="4" y="30" width="56" height="38" rx="9" fill="currentColor"/>
                    <circle cx="32" cy="48" r="5.5" fill="#0d1117" opacity="0.9"/>
                    <rect x="29.5" y="51.5" width="5" height="8" rx="1.5" fill="#0d1117" opacity="0.9"/>
                </svg>
            </div>
            <h1 class="hero-title">ST500</h1>
            <p class="hero-sub">LockMaker</p>
        </div>

        <!-- Info Card -->
        <div class="card">
            <div class="card-row">
                <span class="row-label">NAME</span>
                <span class="row-value">{{ store.userName || '(미설정)' }}</span>
            </div>
            <div class="card-divider" />
            <div class="card-row">
                <span class="row-label">DEVICE</span>
                <span class="row-value mono">{{ shortDeviceId }}</span>
            </div>
            <div class="card-divider" />
            <div class="card-row">
                <span class="row-label">STATUS</span>
                <span class="status-badge" :class="statusBadgeClass">
                    <span class="badge-dot" />
                    {{ store.statusLabel }}
                </span>
            </div>
        </div>

        <!-- 오류 메시지 -->
        <div v-if="store.status === 'error'" class="error-box">
            {{ store.errorMsg || '서버 오류가 발생했습니다.' }}
        </div>

        <!-- 버튼 영역 -->
        <div class="btn-stack">
            <button class="btn-make" :disabled="!store.isApproved" @click="goMake">
                <svg class="btn-icon" viewBox="0 0 20 20" fill="currentColor">
                    <path fill-rule="evenodd" d="M5 9V7a5 5 0 0110 0v2a2 2 0 012 2v5a2 2 0 01-2 2H5a2 2 0 01-2-2v-5a2 2 0 012-2zm8-2v2H7V7a3 3 0 016 0z" clip-rule="evenodd"/>
                </svg>
                MAKE
            </button>
            <button v-if="store.status === 'error'" class="btn-retry" @click="store.retry()">
                RETRY
            </button>
            <button class="btn-setting" @click="goSetting">SETTING</button>
        </div>

        <button class="btn-finish" @click="finish">FINISH</button>
    </div>

    <InstallBanner />
</template>

<script setup>
import { computed, onMounted, onUnmounted } from 'vue'
import { useRouter } from 'vue-router'
import { useDeviceStore } from '@/stores/device.js'
import InstallBanner from '@/components/InstallBanner.vue'

const store  = useDeviceStore()
const router = useRouter()

const shortDeviceId = computed(() =>
    store.deviceId.length > 16
        ? store.deviceId.substring(0, 16) + '…'
        : store.deviceId
)

const statusBadgeClass = computed(() => ({
    'badge-init':     store.status === 'init',
    'badge-waiting':  store.status === 'waiting',
    'badge-approved': store.status === 'approved',
    'badge-error':    store.status === 'error',
}))

function goMake()    { router.push('/make') }
function goSetting() { router.push('/setting') }
function finish()    { window.history.back() }

onMounted(() => store.init())
onUnmounted(() => store.stopPolling())
</script>

<style scoped>
.page {
    display: flex;
    flex-direction: column;
    align-items: center;
    min-height: 100dvh;
    background: var(--bg);
    padding: 0 20px 48px;
}

/* --- Hero --- */
.hero {
    display: flex;
    flex-direction: column;
    align-items: center;
    padding: 56px 0 44px;
    position: relative;
    width: 100%;
}

.hero-glow {
    position: absolute;
    top: -20px;
    left: 50%;
    transform: translateX(-50%);
    width: 400px;
    height: 400px;
    background: radial-gradient(circle, rgba(88, 166, 255, 0.08) 0%, transparent 65%);
    pointer-events: none;
}

.hero-icon {
    width: 68px;
    height: 68px;
    color: var(--accent);
    margin-bottom: 22px;
    filter: drop-shadow(0 0 16px rgba(88, 166, 255, 0.4));
}

.hero-title {
    margin: 0;
    font-size: 48px;
    font-weight: 900;
    letter-spacing: 10px;
    background: linear-gradient(135deg, #e6edf3 0%, #58a6ff 55%, #bc8cff 100%);
    -webkit-background-clip: text;
    -webkit-text-fill-color: transparent;
    background-clip: text;
}

.hero-sub {
    margin: 8px 0 0;
    font-size: 12px;
    color: var(--text-muted);
    letter-spacing: 5px;
    text-transform: uppercase;
    font-weight: 500;
}

/* --- Info Card --- */
.card {
    width: 100%;
    background: var(--surface);
    border: 1px solid var(--border);
    border-radius: 12px;
    padding: 4px 0;
    margin-bottom: 16px;
}

.card-row {
    display: flex;
    align-items: center;
    padding: 16px 20px;
    gap: 16px;
}

.card-divider {
    height: 1px;
    background: var(--border);
    margin: 0 20px;
}

.row-label {
    font-size: 12px;
    font-weight: 700;
    letter-spacing: 1px;
    text-transform: uppercase;
    color: #adbac7;
    width: 64px;
    flex-shrink: 0;
}

.row-value {
    font-size: 16px;
    color: var(--text);
    font-weight: 500;
    overflow: hidden;
    text-overflow: ellipsis;
    white-space: nowrap;
}

.mono {
    font-family: 'Courier New', monospace;
    font-size: 13px;
    color: var(--text-muted);
    letter-spacing: 0.5px;
}

/* --- Status Badge --- */
.status-badge {
    display: inline-flex;
    align-items: center;
    gap: 7px;
    font-size: 13px;
    font-weight: 600;
    padding: 5px 13px;
    border-radius: 999px;
    border: 1px solid transparent;
}

.badge-dot {
    width: 7px;
    height: 7px;
    border-radius: 50%;
    flex-shrink: 0;
}

.badge-init {
    background: rgba(139, 148, 158, 0.12);
    border-color: rgba(139, 148, 158, 0.25);
    color: var(--text-muted);
}
.badge-init .badge-dot { background: var(--text-muted); }

.badge-waiting {
    background: rgba(210, 153, 34, 0.12);
    border-color: rgba(210, 153, 34, 0.3);
    color: var(--warn);
}
.badge-waiting .badge-dot {
    background: var(--warn);
    animation: pulse-dot 1.4s ease-in-out infinite;
}

.badge-approved {
    background: rgba(63, 185, 80, 0.12);
    border-color: rgba(63, 185, 80, 0.3);
    color: var(--accent2);
}
.badge-approved .badge-dot { background: var(--accent2); }

.badge-error {
    background: rgba(248, 81, 73, 0.12);
    border-color: rgba(248, 81, 73, 0.3);
    color: var(--danger);
}
.badge-error .badge-dot { background: var(--danger); }

@keyframes pulse-dot {
    0%, 100% { opacity: 1; transform: scale(1); }
    50%       { opacity: 0.3; transform: scale(0.7); }
}

/* --- Error Box --- */
.error-box {
    width: 100%;
    background: rgba(248, 81, 73, 0.08);
    border: 1px solid rgba(248, 81, 73, 0.3);
    border-radius: 10px;
    color: var(--danger);
    font-size: 13px;
    line-height: 1.5;
    padding: 12px 16px;
    margin-bottom: 16px;
}

/* --- Buttons --- */
.btn-stack {
    display: flex;
    flex-direction: column;
    width: 100%;
    gap: 10px;
    margin-bottom: 20px;
}

.btn-make {
    width: 100%;
    height: 56px;
    background: linear-gradient(135deg, var(--accent) 0%, #3b82f6 100%);
    color: #fff;
    border: none;
    border-radius: 10px;
    font-size: 17px;
    font-weight: 800;
    letter-spacing: 3px;
    cursor: pointer;
    display: flex;
    align-items: center;
    justify-content: center;
    gap: 10px;
    box-shadow: 0 4px 20px rgba(88, 166, 255, 0.3);
    transition: opacity 0.2s, transform 0.15s, box-shadow 0.2s;
}
.btn-make:not(:disabled):hover {
    box-shadow: 0 6px 28px rgba(88, 166, 255, 0.45);
}
.btn-make:not(:disabled):active {
    transform: scale(0.97);
    opacity: 0.88;
}
.btn-make:disabled {
    background: var(--surface2);
    color: var(--border);
    box-shadow: none;
    cursor: not-allowed;
    border: 1px solid var(--border);
}

.btn-icon {
    width: 20px;
    height: 20px;
}

.btn-retry {
    width: 100%;
    height: 48px;
    background: rgba(210, 153, 34, 0.08);
    border: 1px solid rgba(210, 153, 34, 0.35);
    color: var(--warn);
    border-radius: 10px;
    font-size: 14px;
    font-weight: 700;
    letter-spacing: 2px;
    cursor: pointer;
    transition: background 0.2s;
}
.btn-retry:active { background: rgba(210, 153, 34, 0.16); }

.btn-setting {
    width: 100%;
    height: 48px;
    background: transparent;
    border: 1px solid var(--border);
    color: var(--text-muted);
    border-radius: 10px;
    font-size: 14px;
    font-weight: 600;
    letter-spacing: 2px;
    cursor: pointer;
    transition: background 0.2s, border-color 0.2s, color 0.2s;
}
.btn-setting:hover {
    background: var(--surface);
    border-color: #484f58;
    color: var(--text);
}
.btn-setting:active {
    background: var(--surface2);
}

.btn-finish {
    background: none;
    border: none;
    color: #6e7681;
    font-size: 12px;
    font-weight: 500;
    letter-spacing: 2px;
    text-transform: uppercase;
    cursor: pointer;
    padding: 8px 24px;
    transition: color 0.2s;
}
.btn-finish:hover { color: var(--text-muted); }
.btn-finish:active { color: var(--text); }
</style>
