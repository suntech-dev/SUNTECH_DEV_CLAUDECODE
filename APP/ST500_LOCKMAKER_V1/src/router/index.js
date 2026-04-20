import { createRouter, createWebHashHistory } from 'vue-router'
import HomeView    from '@/views/HomeView.vue'
import LockMakeView from '@/views/LockMakeView.vue'
import SettingView  from '@/views/SettingView.vue'

const routes = [
  { path: '/',        name: 'home',     component: HomeView },
  { path: '/make',    name: 'make',     component: LockMakeView },
  { path: '/setting', name: 'setting',  component: SettingView }
]

export default createRouter({
  // Hash 히스토리 사용 → 서버 설정 없이 PWA에서 동작
  history: createWebHashHistory(),
  routes
})
