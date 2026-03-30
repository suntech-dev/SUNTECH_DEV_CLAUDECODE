# CLAUDE.md — ROBOT_SIMULATION 프로젝트 규칙

> 이 파일은 `C:\SUNTECH_DEV_CLAUDECODE\WEB\ROBOT_SIMULATION` 프로젝트에 적용되는 규칙입니다.

---

## JavaScript 코드 스타일 가이드

### 인코딩
- **인코딩**: UTF-8 (BOM 없음)

### 기본 규칙
- **들여쓰기**: 4칸 스페이스
- **코드**: 영문 작성, 핵심 섹션만 한글 주석
- 이모지 사용 안함 (사용자가 명시적으로 요청한 경우에만 허용)
- 최대한 간결한 코드로 유지보수성을 높임
- 빌드 시스템 없음 (순수 HTML/CSS/JS)
- 외부 프레임워크 없음 — CDN 라이브러리만 허용

### 외부 CDN 라이브러리 (고정)
```html
<script src="https://cdn.jsdelivr.net/npm/jspdf@2.5.1/dist/jspdf.umd.min.js"></script>
<script src="https://cdn.jsdelivr.net/npm/jspdf-autotable@3.8.2/dist/jspdf.plugin.autotable.min.js"></script>
<script src="https://cdn.jsdelivr.net/npm/chart.js@4.4.1/dist/chart.umd.min.js"></script>
```

### 스크립트 로드 순서 (index.html — 변경 금지)
```
config.js → engine.js → quickCalc.js → renderer.js →
realtimeCalc.js → charts.js → scenario.js → pdf.js → main.js
```

### 전역 객체 패턴
- 각 모듈은 전역 객체/클래스로 선언 (`Config`, `SimulationEngine`, `Renderer` 등)
- `Config.MACHINE_IDS` 배열을 in-place 수정하여 모든 모듈에 자동 반영

---

## 버전 히스토리 관리 (VERSION_HISTORY.md)

- 버전 폴더명 규칙: `ROBOT_SIMULATION_V{번호}`
- 변경사항 발생 시 `VERSION_HISTORY.md` 업데이트 필수
- 날짜 형식: `YYYY-MM-DD`
- 신규 버전 작업 시 이전 버전 상태를 `구버전`으로 변경

---

## README.md 버전별 업데이트 규칙

- 새 버전 폴더 추가 시 README.md 섹션 2(버전 구조)와 섹션 10(버전 이력 테이블) 갱신
- 기능 변경 시 섹션 4(소스코드 상세 분석) 내용 반영
- README.html도 README.md와 동기화 유지

---

## .md → .html 변환 스타일 가이드

### 테마 & 색상 (CSS 변수 — 변경 금지)
```css
--bg:         #0d1117;
--surface:    #161b22;
--surface2:   #21262d;
--border:     #30363d;
--text:       #e6edf3;
--text-muted: #8b949e;
--accent:     #58a6ff;
--accent2:    #3fb950;
--warn:       #d29922;
--danger:     #f85149;
```

### 레이아웃 규칙
- 좌측 고정 사이드바 TOC + 우측 메인 콘텐츠 (`flex` 레이아웃)
- 모바일(<=768px): 햄버거 버튼으로 사이드바 토글, 오버레이 처리
- `IntersectionObserver`로 현재 섹션 TOC 자동 하이라이트

### UI 컴포넌트 규칙
| 요소      | 처리 방식                                |
| --------- | ---------------------------------------- |
| 섹션 제목 | 번호 뱃지(`.num`) + 하단 border          |
| 테이블    | `.tbl-wrap` 가로 스크롤 처리             |
| 코드 블록 | `pre > code`, 다크 배경, 모노스페이스    |
| 수치/상태 | 카드 + 상태 뱃지(`.tag.ok/.warn/.error`) |
| 버전 이력 | `.timeline` + `.timeline-item`           |
| 아키텍처  | CSS Grid -- ASCII art 사용 금지          |

---

## 로컬 개발 환경

- **로컬 URL**: `http://localhost/dev/ROBOT_SIMULATION/ROBOT_SIMULATION_V1/`
- **경로 → URL 변환**: `WEB\` = `http://localhost/dev/` (1:1 대응)
- 서버 사이드 없음 — 브라우저에서 직접 HTML 파일 열람 가능

---

## 기존 프로젝트 참조 경로

| 프로젝트        | 위치                      | 참고 용도              |
| --------------- | ------------------------- | ---------------------- |
| OEE_SCI         | `WEB\OEE_SCI\`            | README/HTML 템플릿     |
| ROBOT_SIM V1    | `WEB\ROBOT_SIMULATION\ROBOT_SIMULATION_V1\` | 현재 운영 버전 |
