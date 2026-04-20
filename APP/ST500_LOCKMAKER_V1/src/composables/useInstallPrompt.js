import { ref, onMounted, onUnmounted, computed } from 'vue'

const DISMISSED_KEY = 'lm_install_dismissed'

export function useInstallPrompt() {
    const deferredPrompt = ref(null)
    const dismissed = ref(localStorage.getItem(DISMISSED_KEY) === '1')

    // 이미 standalone 모드로 실행 중이면 표시 안 함
    const isStandalone =
        window.matchMedia('(display-mode: standalone)').matches ||
        ('standalone' in window.navigator && window.navigator.standalone === true)

    // iOS Safari 감지 (Chrome/Firefox on iOS 제외)
    const isIOS = /iPad|iPhone|iPod/.test(navigator.userAgent) && !window.MSStream
    const isIOSSafari = isIOS && /Safari/.test(navigator.userAgent) && !/CriOS|FxiOS/.test(navigator.userAgent)

    // 'native': Android/Chrome/Edge — beforeinstallprompt 이벤트 사용
    // 'ios'   : iOS Safari — 수동 안내 필요
    // null    : 표시 불필요
    const promptType = computed(() => {
        if (dismissed.value || isStandalone) return null
        if (deferredPrompt.value) return 'native'
        if (isIOSSafari) return 'ios'
        return null
    })

    function onBeforeInstallPrompt(e) {
        e.preventDefault()
        deferredPrompt.value = e
    }

    onMounted(() => {
        window.addEventListener('beforeinstallprompt', onBeforeInstallPrompt)
    })

    onUnmounted(() => {
        window.removeEventListener('beforeinstallprompt', onBeforeInstallPrompt)
    })

    async function install() {
        if (!deferredPrompt.value) return
        deferredPrompt.value.prompt()
        const { outcome } = await deferredPrompt.value.userChoice
        deferredPrompt.value = null
        if (outcome === 'accepted') dismiss()
    }

    function dismiss() {
        dismissed.value = true
        localStorage.setItem(DISMISSED_KEY, '1')
    }

    return { promptType, install, dismiss }
}
