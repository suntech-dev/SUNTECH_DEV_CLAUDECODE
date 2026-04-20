<template>
    <div class="page">
        <!-- 상단 바 -->
        <div class="top-bar">
            <button class="back-btn" @click="goBack">
                <svg viewBox="0 0 20 20" fill="currentColor" class="back-icon">
                    <path fill-rule="evenodd" d="M12.707 5.293a1 1 0 010 1.414L9.414 10l3.293 3.293a1 1 0 01-1.414 1.414l-4-4a1 1 0 010-1.414l4-4a1 1 0 011.414 0z" clip-rule="evenodd"/>
                </svg>
            </button>
            <span class="top-title">Information</span>
            <span class="top-spacer" />
        </div>

        <div class="content">
            <div class="field-card">
                <label class="field-label" for="input-name">NAME</label>
                <input
                    id="input-name"
                    v-model="inputName"
                    class="field-input"
                    type="text"
                    placeholder="이름을 입력하세요"
                    maxlength="30"
                    autocomplete="off"
                />
            </div>

            <div class="btn-stack">
                <button class="btn-primary" @click="save">SAVE</button>
                <button class="btn-outline" @click="goBack">BACK</button>
            </div>
        </div>

        <!-- 토스트 -->
        <transition name="toast">
            <div v-if="toastMsg" class="toast">{{ toastMsg }}</div>
        </transition>
    </div>
</template>

<script setup>
import { ref, onMounted } from 'vue'
import { useRouter } from 'vue-router'
import { useDeviceStore } from '@/stores/device.js'

const router    = useRouter()
const store     = useDeviceStore()
const inputName = ref('')
const toastMsg  = ref('')

onMounted(() => {
    inputName.value = store.userName
})

function showToast(msg) {
    toastMsg.value = msg
    setTimeout(() => { toastMsg.value = '' }, 2000)
}

async function save() {
    if (!inputName.value.trim()) {
        showToast('이름을 입력해주세요')
        return
    }
    await store.saveName(inputName.value.trim())
    showToast('저장되었습니다')
    setTimeout(() => router.push('/'), 800)
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
    font-size: 16px;
    font-weight: 700;
    color: var(--text);
    letter-spacing: 0.5px;
}

.top-spacer { width: 36px; }

/* --- Content --- */
.content {
    width: 100%;
    padding: 28px 20px 0;
    display: flex;
    flex-direction: column;
    gap: 16px;
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
    padding: 13px 14px;
    font-size: 18px;
    font-weight: 500;
    color: var(--text);
    outline: none;
    transition: border-color 0.2s, box-shadow 0.2s;
}
.field-input::placeholder {
    color: #6e7681;
    font-size: 15px;
    font-weight: 400;
}
.field-input:focus {
    border-color: var(--accent);
    box-shadow: 0 0 0 3px rgba(88, 166, 255, 0.12);
}

/* --- Buttons --- */
.btn-stack {
    display: flex;
    flex-direction: column;
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
