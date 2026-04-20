# 08. TODO 및 개선 로드맵

## 현재 상태 (v1.0.0 초기 구조)

v1.0.0은 기존 Android 앱의 기능을 1:1로 PWA로 이식한 버전.  
빌드·실행은 정상이나 아래 항목들을 순차적으로 개선해야 함.

---

## 즉시 처리 (배포 전 필수)

### ✅ T-1. PWA 아이콘 파일 생성 (완료)
- `public/icons/icon-192.png`, `public/icons/icon-512.png` 생성 완료
- Python Pillow로 생성: Cyan 배경 + 자물쇠 SVG + SUNTECH 로고

### ✅ T-2. iOS apple-touch-icon 링크 추가 (완료)
```html
<!-- index.html — 추가 완료 -->
<link rel="apple-touch-icon" href="/icons/icon-192.png" />
```

### T-3. 알고리즘 검증 테스트
- Kotlin 원본과 JavaScript 이식 코드의 계산 결과 동일 여부 확인
- 테스트 케이스: 알려진 Old Code → New Code 쌍으로 비교
- **현황**: 코드 이식 완료, 실기기 검증 미완료

---

## 단기 개선 (1~2주)

### T-4. .env.production 파일 생성
```bash
# .env.production
VITE_API_BASE_URL=https://[HTTPS도메인]/api/st500/st500_api.php
```
- HTTPS 서버 도메인 결정 후 생성

### T-5. 승인 만료 처리
- 현재: 한번 APPROVED가 되면 영구 유지 (앱 재시작까지)
- 개선: 서버에서 승인 취소 가능하도록 → 폴링 중에 상태 재확인 로직 추가

### ✅ T-6. 네트워크 오류 사용자 피드백 개선 (완료)
- 오류 시 붉은 메시지 박스 + RETRY 버튼 표시 (`HomeView.vue`)
- `stores/device.js` — `retry()` 함수 추가
- 오류 메시지 한글화: `Failed to fetch` → "서버에 연결할 수 없습니다"

### ✅ T-7. 홈 화면 추가 안내 팝업 (완료)
- `InstallBanner.vue` + `useInstallPrompt.js` 구현
- Android/Chrome/Edge: 네이티브 설치 버튼
- iOS Safari: 3단계 수동 안내
- 이미 설치됨(standalone) 또는 dismiss 후: 팝업 표시 안 함

---

## 중기 개선 (1개월)

### T-8. HTTPS 전환
- **우선순위**: 높음 (보안 필수)
- **작업**: 서버 SSL 인증서 발급, `.env.production` 설정
- **효과**: 데이터 암호화, Service Worker 완전 동작, iOS PWA 신뢰성 향상

### T-9. 잠금 코드 알고리즘 서버 이전 (보안)
- **현재**: 클라이언트 JS에 알고리즘 노출 → 브라우저 개발자 도구로 완전 탈취 가능
- **개선**: PHP API 엔드포인트 추가
  ```
  POST /api/st500/st500_api.php
  body: { code: "generate_lock", device_id, old_code, lock_day }
  → response: { new_code: "012345678" }
  ```
- **주의**: 서버 이전 시 `LockMakeView.vue`의 `generate()` 함수 → API 호출로 교체

### T-10. 다국어(i18n) 지원
- 현재 한국어/영어 혼재 (UI는 영어, 일부 메시지는 한국어)
- `vue-i18n` 도입 검토
- **대상 언어**: 한국어, 영어, 인도네시아어 (공장 위치 고려)

### T-11. favicon 및 스플래시 화면
- 현재 기본 Vite favicon (SVG)
- SUNTECH 브랜드 favicon 교체 필요
- `public/favicon.ico` 또는 `public/favicon.svg`

---

## 장기 개선 (3개월+)

### T-12. 관리자 페이지 (별도 구현)
- 서버 관리자가 디바이스 승인/거부할 수 있는 웹 인터페이스
- 현재: 서버 측 관리 방식 미확인 (기존 앱 운영 방식 승계)

### T-13. 코드 이력 관리
- 생성된 New Code를 날짜별로 로컬 저장 (localStorage)
- 재생성 없이 이전 코드 조회 기능

### T-14. 다중 디바이스 프로필
- 한 스마트폰에서 여러 사용자 이름 전환
- 현재는 1기기 = 1이름 고정

### T-15. 오프라인 모드 (제한적)
- 이전에 생성한 코드 로컬 캐싱
- 서버 연결 없이 최근 이력 조회 가능

---

## 알려진 버그 / 제한 사항

| # | 설명 | 심각도 | 상태 |
|---|---|---|---|
| ~~B-1~~ | ~~브라우저 캐시 초기화 시 디바이스 ID 변경 → 서버 재승인 필요~~ | ~~MEDIUM~~ | ✅ 해결 (v1.2.0 핑거프린트) |
| B-2 | 폴링 중 앱을 백그라운드로 보내면 iOS Safari에서 타이머 슬로우다운 | LOW | 알려진 iOS 제한 |
| B-3 | 알고리즘 float 정밀도 차이 → Kotlin toLong()과 JS Math.floor() 결과 불일치 가능 | HIGH | 검증 필요 |
| B-4 | HTTP 서버 → HTTPS PWA에서 Mixed Content 차단 가능 | HIGH | HTTPS 전환으로 해결 |

---

## 버전 히스토리

| 버전 | 날짜 | 내용 |
|---|---|---|
| v1.0.0 | 2026-04-17 | 초기 구조 구축 (Android 앱 PWA 이식) |
| v1.1.0 | 2026-04-18 | UI 전면 리디자인 — GitHub Dark 팔레트 적용, SUNTECH 로고 추가, T-1/2/6/7 완료, base 경로 수정 |
| v1.2.0 | 2026-04-20 | Device ID 안정화 — 하드웨어 핑거프린트 + IndexedDB 이중 저장 (B-1 해결) |

---

## 참고 파일

- 원본 Android 앱: `APP/lockmaker_211206/`
- 원본 분석 리포트: `APP/lockmaker_211206/ANALYSIS_REPORT.md`
- 기술 선택 분석: `APP/lockmaker_211206/RENEWAL_TECH_ANALYSIS.md`
