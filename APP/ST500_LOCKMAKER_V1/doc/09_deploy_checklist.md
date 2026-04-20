# 09. 테스트 및 배포 절차 체크리스트

> 최초 작성: 2026-04-17
> 마지막 업데이트: 2026-04-17

배포할 때마다 이 문서를 순서대로 따라간다.  
각 단계를 완료하면 `[x]`로 체크.

---

## 환경 구성도

```
[소스 코드]                     [테스트]                    [운영]
 VS Code 편집
    │
    ├─ npm run dev ──────────► localhost:5173        (Vite, 핫 리로드)
    │
    └─ npm run build
           │
           └─ dist/ 복사 ────► localhost/app/        (Laragon, PWA 동작 확인)
                                ST500_LOCKMAKER_V1/
                                      │
                                      └─ scp ──────► 실제 서버 (115.68.227.31)
```

---

## PHASE 1 — 개발 서버 (Vite) 기능 테스트

> 목적: UI/UX, 라우팅, 알고리즘 계산 빠른 검증. 핫 리로드 활용.

```bash
cd C:\SUNTECH_DEV_CLAUDECODE\APP\ST500_LOCKMAKER_V1
npm run dev
# → http://localhost:5173/
```

### 기능 체크리스트

- [ ] **HomeView** 진입 시 NAME/DEVICE ID/STATUS 표시 정상
- [ ] **STATUS**: 이름 미설정 → "Not registered" 표시
- [ ] **SETTING 버튼** → `/setting` 화면 이동
- [ ] **이름 입력 후 SAVE** → 저장 토스트 표시 → 홈 복귀
- [ ] **STATUS**: 이름 저장 후 → "Wait approval..." 로 변경
- [ ] **MAKE 버튼**: 미승인 상태에서 disabled (클릭 불가)
- [ ] **알고리즘 검증**: 아래 샘플 코드로 계산 결과 확인

    | Old Code | Lock Day | 예상 New Code |
    |---|---|---|
    | (원본 앱에서 테스트 후 채워넣기) | | |

- [ ] **UnLock 체크박스**: 체크 시 DAY 입력 비활성화, 151로 계산
- [ ] **BACK 버튼** (각 화면): 홈으로 정상 복귀
- [ ] **FINISH 버튼**: 브라우저 뒤로 가기 동작

### 콘솔 오류 확인

- [ ] 브라우저 개발자 도구 Console 탭 — 빨간 에러 없음
- [ ] Network 탭 — API 요청 정상 (또는 서버 미연결 시 ERROR 상태 처리 확인)

---

## PHASE 2 — Laragon 로컬 서버 PWA 테스트

> 목적: 실제 브라우저 PWA 동작 확인 (Service Worker, 홈 화면 추가, 오프라인).  
> Vite dev server에서는 Service Worker가 비활성화되므로 반드시 이 단계 수행.

### 2-1. 빌드 및 Laragon 배포

```bash
# 프로젝트 폴더에서
cd C:\SUNTECH_DEV_CLAUDECODE\APP\ST500_LOCKMAKER_V1

# 빌드 (에러 없이 완료 확인)
npm run build
```

빌드 성공 확인 사항:
- [ ] `✓ built in xxx ms` 메시지 출력
- [ ] `dist/sw.js`, `dist/manifest.webmanifest` 파일 생성 확인

```bash
# Laragon www에 복사 (PowerShell)
Copy-Item -Recurse -Force dist\* C:\laragon\www\app\ST500_LOCKMAKER_V1\

# 또는 Git Bash
cp -r dist/* /c/laragon/www/app/ST500_LOCKMAKER_V1/
```

- [ ] Laragon이 실행 중인지 확인 (트레이 아이콘)
- [ ] 복사 대상 폴더 존재 확인 (`C:\laragon\www\app\ST500_LOCKMAKER_V1\`)

접속 URL: `http://localhost/app/ST500_LOCKMAKER_V1/`

### 2-2. PWA 동작 확인 (Chrome)

- [ ] `http://localhost/app/ST500_LOCKMAKER_V1/` 접속
- [ ] 주소창 우측 설치 아이콘(⊕) 또는 "앱 설치" 프롬프트 표시
- [ ] DevTools → Application → Service Workers → 상태 `activated and running`
- [ ] DevTools → Application → Manifest → 아이콘, 이름 정상 표시
- [ ] 기능 전체 재검증 (PHASE 1 체크리스트 반복)

### 2-3. iOS Safari 실기기 테스트 (선택 — 기기 있을 때)

> Laragon 서버를 같은 Wi-Fi 네트워크에서 접속.  
> Laragon이 `--host` 옵션 없이도 로컬 IP로 접근 가능.

```bash
# 내 PC의 로컬 IP 확인
ipconfig | findstr "IPv4"
# → 예: 192.168.1.100
```

접속 URL: `http://192.168.1.100/app/ST500_LOCKMAKER_V1/`

- [ ] iPhone Safari에서 위 URL 접속 정상
- [ ] 화면 레이아웃 깨짐 없음 (max-width 540px, 세로 모드)
- [ ] 키보드 올라올 때 레이아웃 밀림 없음
- [ ] 홈 화면에 추가 → 아이콘 표시 (아이콘 파일 준비된 경우)
- [ ] 홈 화면에서 실행 → 주소창 없는 standalone 모드

### 2-4. Android Chrome 실기기 테스트 (선택)

- [ ] Chrome에서 URL 접속 → "앱 설치" 배너 표시
- [ ] 설치 후 앱 서랍에서 실행 확인

---

## PHASE 3 — 운영 서버 배포

> 목적: 실제 서버(`115.68.227.31`)에 배포.

### 3-1. 배포 전 최종 확인

- [ ] `.env.production` 파일 존재 여부 확인
    ```
    VITE_API_BASE_URL=https://[운영 도메인]/api/st500/st500_api.php
    ```
    > 아직 HTTP만 있으면 `.env`의 HTTP URL로 배포 (보안 주의)

- [ ] `vite.config.js` 변경 사항 있으면 반영 확인
- [ ] `public/icons/icon-192.png`, `icon-512.png` 존재 확인
- [ ] `index.html` apple-touch-icon 태그 포함 확인

### 3-2. 프로덕션 빌드

```bash
cd C:\SUNTECH_DEV_CLAUDECODE\APP\ST500_LOCKMAKER_V1
npm run build
```

- [ ] 빌드 에러 없음
- [ ] `dist/` 폴더 내용 최신화 확인

### 3-3. 서버 업로드

```bash
# SCP로 dist/ 내용 업로드
scp -r dist/* root@115.68.227.31:/var/www/html/lockmaker/

# 또는 포트 지정 시
scp -P 22 -r dist/* root@115.68.227.31:/var/www/html/lockmaker/
```

- [ ] 업로드 완료 (오류 없음)
- [ ] 서버 파일 권한 확인 (필요 시 `chmod 644`)

### 3-4. 운영 서버 동작 확인

접속 URL: `http://115.68.227.31/lockmaker/` (또는 도메인)

- [ ] 메인 화면 정상 로드
- [ ] 기능 전체 빠른 검증 (PHASE 1 체크리스트)
- [ ] 브라우저 Console 에러 없음
- [ ] Service Worker 등록 확인 (DevTools → Application)

---

## PHASE 4 — 배포 후 정리

- [ ] 버전 히스토리 업데이트 (`doc/08_roadmap.md` 버전 이력 테이블)
- [ ] 완료된 TODO 항목 `doc/08_roadmap.md`에서 체크
- [ ] 변경 사항 Git 커밋

```bash
git add .
git commit -m "feat(ST500_LOCKMAKER_V1): 배포 v1.x.x - 변경 내용 요약"
```

---

## 빠른 참조 — 서버 정보

| 항목 | 값 |
|---|---|
| 운영 서버 IP | `115.68.227.31` |
| SSH 포트 | 22 |
| SSH 사용자 | root |
| 서버 배포 경로 | `/var/www/html/lockmaker/` |
| 운영 URL | `http://115.68.227.31/lockmaker/` |
| Laragon 배포 경로 | `C:\laragon\www\app\ST500_LOCKMAKER_V1\` |
| Laragon 테스트 URL | `http://localhost/app/ST500_LOCKMAKER_V1/` |
| Vite dev URL | `http://localhost:5173/` |

---

## 트러블슈팅

### 빌드 후 Laragon에서 흰 화면만 보임

```
원인: dist/ 복사 경로 또는 Laragon www 폴더 문제
확인: http://localhost/app/ST500_LOCKMAKER_V1/index.html 직접 접속
해결: 대상 폴더 경로 재확인, Laragon 재시작
```

### Service Worker 오래된 버전이 계속 실행됨

```
해결: Chrome DevTools → Application → Service Workers → "Unregister"
      그 후 페이지 새로고침 (Shift+F5)
```

### iOS에서 홈 화면 추가 후 앱이 흰 화면

```
원인: start_url 또는 scope 설정 문제, 또는 파일 경로 불일치
확인: manifest.webmanifest의 start_url이 실제 접속 경로와 일치하는지 확인
```

### API 서버 연결 안 됨 (CORS 오류)

```
원인: 서버에서 CORS 헤더 미설정
확인: DevTools Network 탭 → API 요청 → Response Headers에 Access-Control-Allow-Origin 없음
해결: 서버 PHP 파일에 header("Access-Control-Allow-Origin: *") 추가
```

### Mixed Content 차단 (HTTPS 환경)

```
원인: HTTPS 페이지에서 HTTP API 호출
해결: .env.production에 HTTPS API URL 설정 후 재빌드·재배포
```
