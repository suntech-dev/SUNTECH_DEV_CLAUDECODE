<template>
    <Teleport to="body">
        <Transition name="banner">
            <div v-if="promptType" class="overlay" @click.self="dismiss">
                <div class="sheet">
                    <!-- 앱 아이디 -->
                    <div class="sheet-header">
                        <div class="app-icon">
                            <svg viewBox="0 0 64 72" fill="none" xmlns="http://www.w3.org/2000/svg">
                                <path d="M14 32V23C14 13.611 21.611 6 31 6h2c9.389 0 17 7.611 17 17v9"
                                      stroke="currentColor" stroke-width="5.5" stroke-linecap="round"/>
                                <rect x="4" y="30" width="56" height="38" rx="9" fill="currentColor"/>
                                <circle cx="32" cy="48" r="5.5" fill="#0d1b2a" opacity="0.85"/>
                                <rect x="29.5" y="51.5" width="5" height="8" rx="1.5" fill="#0d1b2a" opacity="0.85"/>
                            </svg>
                        </div>
                        <div>
                            <div class="app-name">ST500 LockMaker</div>
                            <div class="app-sub">SUNTECH</div>
                        </div>
                    </div>

                    <!-- iOS Safari 안내 -->
                    <template v-if="promptType === 'ios'">
                        <p class="sheet-desc">
                            홈 화면에 추가하면 앱처럼 바로 실행할 수 있습니다.
                        </p>
                        <div class="steps">
                            <div class="step">
                                <span class="step-num">1</span>
                                <span class="step-text">Safari 하단의
                                    <span class="highlight">
                                        <!-- iOS 공유 아이콘 -->
                                        <svg viewBox="0 0 20 20" fill="currentColor" class="share-icon">
                                            <path d="M13 8V2H7v6H2l8 8 8-8h-5zM0 18h20v2H0v-2z"/>
                                        </svg>
                                        공유
                                    </span>
                                    버튼 탭
                                </span>
                            </div>
                            <div class="step">
                                <span class="step-num">2</span>
                                <span class="step-text"><span class="highlight">홈 화면에 추가</span> 선택</span>
                            </div>
                            <div class="step">
                                <span class="step-num">3</span>
                                <span class="step-text">오른쪽 상단 <span class="highlight">추가</span> 탭</span>
                            </div>
                        </div>
                        <button class="btn-primary" @click="dismiss">확인</button>
                    </template>

                    <!-- Android / Chrome / Edge 네이티브 설치 -->
                    <template v-else>
                        <p class="sheet-desc">
                            홈 화면에 추가하면 앱처럼 바로 실행할 수 있습니다.
                        </p>
                        <button class="btn-primary" @click="install">설치하기</button>
                        <button class="btn-ghost" @click="dismiss">나중에</button>
                    </template>
                </div>
            </div>
        </Transition>
    </Teleport>
</template>

<script setup>
import { useInstallPrompt } from '@/composables/useInstallPrompt.js'

const { promptType, install, dismiss } = useInstallPrompt()
</script>

<style scoped>
.overlay {
    position: fixed;
    inset: 0;
    background: rgba(0, 0, 0, 0.6);
    backdrop-filter: blur(4px);
    -webkit-backdrop-filter: blur(4px);
    z-index: 200;
    display: flex;
    align-items: flex-end;
    /* #app max-width 맞춤 */
    max-width: 540px;
    left: 50%;
    transform: translateX(-50%);
}

.sheet {
    width: 100%;
    background: #0f2033;
    border: 1px solid rgba(255, 255, 255, 0.12);
    border-bottom: none;
    border-radius: 24px 24px 0 0;
    padding: 28px 24px 40px;
    display: flex;
    flex-direction: column;
    gap: 16px;
}

/* 앱 헤더 */
.sheet-header {
    display: flex;
    align-items: center;
    gap: 16px;
    margin-bottom: 4px;
}

.app-icon {
    width: 56px;
    height: 56px;
    background: linear-gradient(135deg, #06b6d4, #0284c7);
    border-radius: 14px;
    display: flex;
    align-items: center;
    justify-content: center;
    color: #fff;
    padding: 10px;
    flex-shrink: 0;
    box-shadow: 0 4px 16px rgba(6, 182, 212, 0.3);
}

.app-name {
    font-size: 17px;
    font-weight: 700;
    color: #f1f5f9;
}

.app-sub {
    font-size: 12px;
    color: #64748b;
    margin-top: 2px;
    letter-spacing: 1px;
}

/* 설명 */
.sheet-desc {
    margin: 0;
    font-size: 14px;
    color: #94a3b8;
    line-height: 1.6;
}

/* iOS 단계 안내 */
.steps {
    display: flex;
    flex-direction: column;
    gap: 12px;
    background: rgba(255, 255, 255, 0.04);
    border: 1px solid rgba(255, 255, 255, 0.07);
    border-radius: 12px;
    padding: 16px;
}

.step {
    display: flex;
    align-items: center;
    gap: 12px;
}

.step-num {
    width: 24px;
    height: 24px;
    background: rgba(6, 182, 212, 0.15);
    color: #06b6d4;
    border-radius: 50%;
    font-size: 12px;
    font-weight: 700;
    display: flex;
    align-items: center;
    justify-content: center;
    flex-shrink: 0;
}

.step-text {
    font-size: 14px;
    color: #cbd5e1;
    line-height: 1.4;
    display: flex;
    align-items: center;
    gap: 4px;
    flex-wrap: wrap;
}

.highlight {
    color: #06b6d4;
    font-weight: 600;
    display: inline-flex;
    align-items: center;
    gap: 3px;
}

.share-icon {
    width: 14px;
    height: 14px;
}

/* 버튼 */
.btn-primary {
    width: 100%;
    height: 54px;
    background: linear-gradient(135deg, #06b6d4 0%, #0284c7 100%);
    color: #fff;
    border: none;
    border-radius: 14px;
    font-size: 16px;
    font-weight: 700;
    letter-spacing: 2px;
    cursor: pointer;
    box-shadow: 0 4px 18px rgba(6, 182, 212, 0.35);
    transition: opacity 0.2s, transform 0.15s;
}
.btn-primary:active {
    transform: scale(0.97);
    opacity: 0.88;
}

.btn-ghost {
    width: 100%;
    height: 46px;
    background: transparent;
    border: none;
    color: #475569;
    font-size: 14px;
    font-weight: 500;
    cursor: pointer;
    letter-spacing: 1px;
}

/* 슬라이드 업 애니메이션 */
.banner-enter-active { transition: opacity 0.3s, transform 0.35s cubic-bezier(0.34, 1.56, 0.64, 1); }
.banner-leave-active { transition: opacity 0.25s, transform 0.25s ease-in; }
.banner-enter-from   { opacity: 0; transform: translateX(-50%) translateY(100%); }
.banner-leave-to     { opacity: 0; transform: translateX(-50%) translateY(100%); }
.banner-enter-to     { opacity: 1; transform: translateX(-50%) translateY(0); }
</style>
