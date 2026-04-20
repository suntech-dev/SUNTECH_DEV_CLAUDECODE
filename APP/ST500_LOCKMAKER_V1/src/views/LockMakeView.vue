<template>
    <div class="page">
        <!-- 상단 바 -->
        <div class="top-bar">
            <button class="back-btn" @click="goBack">
                <svg viewBox="0 0 20 20" fill="currentColor" class="back-icon">
                    <path fill-rule="evenodd" d="M12.707 5.293a1 1 0 010 1.414L9.414 10l3.293 3.293a1 1 0 01-1.414 1.414l-4-4a1 1 0 010-1.414l4-4a1 1 0 011.414 0z" clip-rule="evenodd"/>
                </svg>
            </button>
            <span class="top-title">LockMaker</span>
            <span class="top-spacer" />
        </div>

        <div class="fields">
            <!-- OLD CODE -->
            <div class="field-card">
                <label class="field-label">OLD CODE</label>
                <input
                    v-model="oldCode"
                    class="field-input"
                    type="tel"
                    maxlength="9"
                    placeholder="8~9자리 숫자"
                    inputmode="numeric"
                    pattern="[0-9]*"
                />
            </div>

            <!-- LOCK DAY -->
            <div class="field-card">
                <label class="field-label">LOCK DAY</label>
                <div class="day-row">
                    <input
                        v-model="lockDay"
                        class="field-input"
                        type="tel"
                        maxlength="3"
                        placeholder="잠금 일수"
                        inputmode="numeric"
                        pattern="[0-9]*"
                        :disabled="isUnlock"
                    />
                    <label class="unlock-toggle">
                        <input v-model="isUnlock" type="checkbox" class="sr-only" />
                        <span class="toggle-track" :class="{ 'toggle-on': isUnlock }">
                            <span class="toggle-thumb" />
                        </span>
                        <span class="toggle-label">UnLock</span>
                    </label>
                </div>
            </div>

            <!-- NEW CODE -->
            <div class="field-card result-card" :class="{ 'result-ready': newCode }">
                <label class="field-label">NEW CODE</label>
                <div class="result-display">{{ newCode || '—' }}</div>
            </div>
        </div>

        <!-- 버튼 -->
        <div class="btn-stack">
            <button class="btn-primary" @click="generate">MAKE</button>
            <button class="btn-outline" @click="goBack">BACK</button>
        </div>

        <!-- 토스트 -->
        <transition name="toast">
            <div v-if="toastMsg" class="toast">{{ toastMsg }}</div>
        </transition>
    </div>
</template>

<script setup>
import { ref, watch } from 'vue'
import { useRouter } from 'vue-router'

const router   = useRouter()
const oldCode  = ref('')
const lockDay  = ref('')
const isUnlock = ref(false)
const newCode  = ref('')
const toastMsg = ref('')

watch(isUnlock, (val) => {
    if (val) lockDay.value = ''
})

function showToast(msg) {
    toastMsg.value = msg
    setTimeout(() => { toastMsg.value = '' }, 2000)
}

/**
 * 잠금 코드 생성 알고리즘
 * 원본: lockmake.kt MakeCalNum()
 * TODO: 보안 강화 시 이 계산을 서버 API로 이전할 것
 */
function calcNum(n, coeffs) {
    const [a, b, c, d] = coeffs
    const i1 = Math.floor(n / 100)
    const i2 = Math.floor((n % 100) / 10)
    const i3 = n % 10
    return Math.floor(i1 * a + i2 * b + i3 * c + (i1 * 100 + i2 * 10 + i3) * d) % 1000
}

function generate() {
    const code = oldCode.value.trim()
    if (code.length < 8) {
        showToast('OLD CODE를 8자리 이상 입력하세요')
        return
    }

    let day
    if (isUnlock.value) {
        day = 151
    } else {
        if (!lockDay.value) {
            showToast('잠금 일수를 입력하세요')
            return
        }
        day = parseInt(lockDay.value, 10)
    }

    const codeNum = parseInt(code, 10)
    const n1 = Math.floor(codeNum / 1000000)
    const rem = codeNum - n1 * 1000000
    const n2 = Math.floor(rem / 1000)
    const n3 = rem % 1000

    let c1 = calcNum(n1, [3.305982, 2.358196, 1.141059, 6.78213])
    let c2 = calcNum(n2, [3.219283, 1.153023, 2.019283, 8.23143])
    let c3 = calcNum(n3, [1.113569, 9.123123, 7.213213, 6.12374])

    c1 = (c1 + day) % 1000
    c2 = (c2 + day) % 1000
    c3 = (c3 + day) % 1000

    const result = c1 * 1000000 + c2 * 1000 + c3
    newCode.value = result < 100000000
        ? '0' + String(result)
        : String(result)
}

function goBack() { router.push('/') }
</script>

<style scoped>
.page {
    display: flex;
    flex-direction: column;
    align-items: center;
    min-height: 100dvh;
    background: var(--bg);
    padding-bottom: 48px;
}

/* --- 상단 바 --- */
.top-bar {
    width: 100%;
    height: 56px;
    display: flex;
    align-items: center;
    justify-content: space-between;
    padding: 0 16px;
    background: var(--surface);
    border-bottom: 1px solid var(--border);
    flex-shrink: 0;
}

.back-btn {
    width: 36px;
    height: 36px;
    background: var(--surface2);
    border: 1px solid var(--border);
    border-radius: 8px;
    cursor: pointer;
    display: flex;
    align-items: center;
    justify-content: center;
    color: var(--text-muted);
    transition: background 0.15s, color 0.15s;
}
.back-btn:hover  { background: #2d333b; color: var(--text); }
.back-btn:active { background: #373e47; }

.back-icon { width: 18px; height: 18px; }

.top-title {
    /* font-size: 16px; */
    font-size: 20px;
    font-weight: 700;
    color: var(--text);
    letter-spacing: 0.5px;
}

.top-spacer { width: 36px; }

/* --- Fields --- */
.fields {
    width: 100%;
    padding: 24px 20px 16px;
    display: flex;
    flex-direction: column;
    gap: 12px;
}

.field-card {
    background: var(--surface);
    border: 1px solid var(--border);
    border-radius: 10px;
    padding: 16px 18px;
}

.field-label {
    display: block;
    font-size: 12px;
    font-weight: 700;
    letter-spacing: 1px;
    text-transform: uppercase;
    color: #adbac7;
    margin-bottom: 10px;
}

.field-input {
    width: 100%;
    background: var(--surface2);
    border: 1px solid var(--border);
    border-radius: 8px;
    padding: 12px 14px;
    font-size: 24px;
    font-weight: 600;
    color: var(--text);
    outline: none;
    letter-spacing: 3px;
    transition: border-color 0.2s, box-shadow 0.2s;
}
.field-input::placeholder {
    color: #6e7681;
    font-size: 14px;
    letter-spacing: 0;
    font-weight: 400;
}
.field-input:focus {
    border-color: var(--accent);
    box-shadow: 0 0 0 3px rgba(88, 166, 255, 0.12);
}
.field-input:disabled {
    opacity: 0.3;
    cursor: not-allowed;
}

/* Day row */
.day-row {
    display: flex;
    align-items: center;
    gap: 14px;
}
.day-row .field-input { flex: 1; }

/* Toggle */
.unlock-toggle {
    display: flex;
    align-items: center;
    gap: 8px;
    cursor: pointer;
    flex-shrink: 0;
}

.sr-only {
    position: absolute;
    width: 1px;
    height: 1px;
    overflow: hidden;
    clip: rect(0, 0, 0, 0);
}

.toggle-track {
    width: 42px;
    height: 22px;
    background: var(--surface2);
    border: 1px solid var(--border);
    border-radius: 999px;
    position: relative;
    flex-shrink: 0;
    transition: background 0.25s, border-color 0.25s;
}
.toggle-track.toggle-on {
    background: var(--accent);
    border-color: var(--accent);
}

.toggle-thumb {
    position: absolute;
    top: 2px;
    left: 2px;
    width: 16px;
    height: 16px;
    border-radius: 50%;
    background: #fff;
    box-shadow: 0 1px 3px rgba(0, 0, 0, 0.4);
    transition: transform 0.25s;
}
.toggle-on .toggle-thumb { transform: translateX(20px); }

.toggle-label {
    font-size: 13px;
    font-weight: 600;
    color: var(--text-muted);
    white-space: nowrap;
}

/* Result */
.result-card {
    border-color: var(--border);
}
.result-ready {
    border-color: rgba(63, 185, 80, 0.4);
    background: rgba(63, 185, 80, 0.06);
}

.result-display {
    font-size: 32px;
    font-weight: 800;
    letter-spacing: 5px;
    color: #6e7681;
    padding: 6px 0;
    font-family: 'Courier New', monospace;
    transition: color 0.3s;
}
.result-ready .result-display { color: var(--accent2); }

/* --- Buttons --- */
.btn-stack {
    display: flex;
    flex-direction: column;
    width: 100%;
    padding: 0 20px;
    gap: 10px;
}

.btn-primary {
    width: 100%;
    height: 54px;
    background: linear-gradient(135deg, var(--accent) 0%, #3b82f6 100%);
    color: #fff;
    border: none;
    border-radius: 10px;
    font-size: 17px;
    font-weight: 800;
    letter-spacing: 3px;
    cursor: pointer;
    box-shadow: 0 4px 20px rgba(88, 166, 255, 0.3);
    transition: opacity 0.2s, transform 0.15s, box-shadow 0.2s;
}
.btn-primary:hover {
    box-shadow: 0 6px 28px rgba(88, 166, 255, 0.45);
}
.btn-primary:active {
    transform: scale(0.97);
    opacity: 0.88;
}

.btn-outline {
    width: 100%;
    height: 46px;
    background: transparent;
    border: 1px solid var(--border);
    color: var(--text-muted);
    border-radius: 10px;
    font-size: 14px;
    font-weight: 600;
    letter-spacing: 2px;
    cursor: pointer;
    transition: background 0.15s, color 0.15s, border-color 0.15s;
}
.btn-outline:hover {
    background: var(--surface);
    border-color: #484f58;
    color: var(--text);
}
.btn-outline:active {
    background: var(--surface2);
}

/* --- Toast --- */
.toast {
    position: fixed;
    bottom: 36px;
    left: 50%;
    transform: translateX(-50%);
    background: var(--surface2);
    border: 1px solid var(--border);
    color: var(--text);
    padding: 11px 26px;
    border-radius: 20px;
    font-size: 14px;
    z-index: 100;
    white-space: nowrap;
    box-shadow: 0 8px 24px rgba(0, 0, 0, 0.5);
}
.toast-enter-active, .toast-leave-active { transition: opacity 0.3s, transform 0.3s; }
.toast-enter-from, .toast-leave-to { opacity: 0; transform: translateX(-50%) translateY(8px); }
</style>
