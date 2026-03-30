# SKILLS_LIST.md — Claude Code 커스텀 스킬 목록

> 최초 작성: 2026-03-27
> 스킬 파일 위치: `C:\Users\luvsd\.claude\commands\`
> 네임스페이스: `sc` (범용), `suntech` (이 프로젝트 전용)

---

## 요약

| 구분 | 개수 | 설명 |
|------|------|------|
| `sc:` 스킬 | 8개 | 범용 코드 작업 (분석, 구현, 개선 등) |
| `suntech:` 스킬 | 3개 | SUNTECH 프로젝트 전용 자동화 |
| **합계** | **11개** | |

---

## sc: 스킬 (범용)

### /sc:analyze — 코드 분석

```
/sc:analyze [대상] [--focus quality|security|performance|architecture] [--depth quick|deep]
```

**용도**: 코드 품질·보안·성능·아키텍처 4개 도메인 종합 분석. 심각도 등급 및 개선 로드맵 제공.

**이 프로젝트 활용 예시**:
```
/sc:analyze PSOC/280CTP_IoT_INTEGRATED --focus security
/sc:analyze WEB/OEE_SCI/OEE_SCI_V2 --focus performance
/sc:analyze WEB/CTP280_API --focus quality --depth deep
```

**MCP 의존**: 없음 (항상 동작)

---

### /sc:cleanup — 코드 정리

```
/sc:cleanup [대상] [--type code|imports|files|all] [--safe|--aggressive]
```

**용도**: 데드코드 제거, 미사용 import 정리, 프로젝트 구조 최적화. 기능 손실 없이 안전하게 실행.

**이 프로젝트 활용 예시**:
```
/sc:cleanup PSOC/280CTP_IoT_INTEGRATED --type code --safe   # 전역 변수 50개 정리
/sc:cleanup WEB/OEE_SCI/OEE_SCI_V2 --type all
```

**MCP 의존**: sequential, context7 (없어도 기본 동작)

---

### /sc:design — 아키텍처·API 설계

```
/sc:design [대상] [--type architecture|api|component|database] [--format diagram|spec|code]
```

**용도**: 시스템 아키텍처, API 스펙, DB 스키마 설계 문서 생성. 신규 기능 추가 전 설계 단계에 사용.

**이 프로젝트 활용 예시**:
```
/sc:design CTP280_API --type api --format spec        # API 스펙 문서 생성
/sc:design OEE_SCI --type database --format diagram   # DB 스키마 다이어그램
```

**MCP 의존**: 없음 (항상 동작)

---

### /sc:explain — 코드·개념 설명

```
/sc:explain [대상] [--level basic|intermediate|advanced] [--context domain]
```

**용도**: 복잡한 코드·시스템 동작을 단계별로 설명. 수준별(초급/중급/고급) 맞춤 설명 생성.

**이 프로젝트 활용 예시**:
```
/sc:explain PSOC/280CTP_IoT_INTEGRATED --level intermediate  # PSoC WiFi 통신 흐름 설명
/sc:explain WEB/OEE_SCI/OEE_SCI_V2/lib/stream_helper.lib.php  # SSE 라이브러리 설명
/sc:explain WEB/ROBOT_SIMULATION/ROBOT_SIMULATION_V1/js/engine.js --level advanced
```

**MCP 의존**: sequential, context7 (없어도 기본 동작)

---

### /sc:git — Git 작업

```
/sc:git [operation] [--smart-commit] [--interactive]
```

**용도**: git 상태 분석, 변경사항 기반 커밋 메시지 자동 생성, 브랜치 관리. Conventional Commit 형식 적용.

**이 프로젝트 활용 예시**:
```
/sc:git commit --smart-commit   # 변경사항 분석 후 커밋 메시지 자동 생성
/sc:git status                  # 저장소 상태 요약 및 다음 단계 안내
```

**MCP 의존**: 없음 (항상 동작)

---

### /sc:implement — 기능 구현

```
/sc:implement [기능 설명] [--type component|api|service|feature] [--safe] [--with-tests]
```

**용도**: 신규 기능 코드 구현. 프로젝트 타입(PHP/C/JS) 자동 감지 후 해당 언어 베스트 프랙티스 적용.

**이 프로젝트 활용 예시**:
```
/sc:implement send_eCount API --type api           # 자수기 카운트 API 구현
/sc:implement OEE 대시보드 위젯 --type component   # PHP+JS 위젯 구현
/sc:implement PSoC WiFi 재연결 로직 --safe         # 임베디드 C 구현
```

**MCP 의존**: context7, sequential, magic, playwright (없어도 기본 동작)

---

### /sc:improve — 코드 개선

```
/sc:improve [대상] [--type quality|performance|maintainability|style] [--safe]
```

**용도**: 코드 품질·성능·유지보수성 체계적 개선. 리팩토링, 기술 부채 해소, 보안 강화.

**이 프로젝트 활용 예시**:
```
/sc:improve PSOC/280CTP_IoT_INTEGRATED --type performance  # SRAM 추가 최적화
/sc:improve WEB/OEE_SCI/OEE_SCI_V2 --type maintainability  # PHP 구조 개선
/sc:improve WEB/ROBOT_SIMULATION --type quality --safe      # JS 모듈 품질 개선
```

**MCP 의존**: sequential, context7 (없어도 기본 동작)

---

### /sc:troubleshoot — 버그 진단 및 해결

```
/sc:troubleshoot [문제 설명] [--type bug|build|performance|deployment] [--trace] [--fix]
```

**용도**: 버그·빌드 오류·성능 이슈 원인 분석 및 해결. 로그 분석, 스택 트레이스 추적.

**이 프로젝트 활용 예시**:
```
/sc:troubleshoot "WiFi 부팅 시 신호강도 0" --type bug --trace   # PSoC 버그 분석
/sc:troubleshoot "SSE 스트림 끊김" --type bug --fix             # PHP SSE 버그 수정
/sc:troubleshoot "OEE 계산값 오류" --type bug                   # 로직 오류 추적
```

**MCP 의존**: 없음 (항상 동작)

---

## suntech: 스킬 (SUNTECH 프로젝트 전용)

> 이 프로젝트의 CLAUDE.md 규칙을 원본으로 참조하여 동작합니다.
> 스킬 파일에 규칙을 중복 정의하지 않아 CLAUDE.md 수정 시 자동 반영됩니다.

---

### /suntech:init — 프로젝트 초기화

```
/suntech:init [프로젝트명]
```

**용도**: 새 프로젝트 폴더에 `CLAUDE.md`, `README.md`, `VERSION_HISTORY.md` 3개 파일 자동 생성.
프로젝트 타입(PSoC4/웹/정적 JS) 자동 판별 후 해당 템플릿 적용.

**활용 예시**:
```
/suntech:init OEE_SCI_V3
/suntech:init ROBOT_SIMULATION_V2
/suntech:init 280CTP_IoT_INTEGRATED_V2
```

**규칙 원본**: `PSOC/CLAUDE.md` 또는 `WEB/CLAUDE.md` (프로젝트 타입에 따라)

**스킬 파일**: `C:\Users\luvsd\.claude\commands\suntech\init.md`

---

### /suntech:doc — MD → HTML 문서 변환

```
/suntech:doc [대상 md 파일]
```

**용도**: `.md` 파일을 다크 테마 단일 HTML 문서로 변환.
좌측 고정 사이드바 TOC, IntersectionObserver 현재 섹션 하이라이트, 모바일 반응형 포함.
인수 없이 호출 시 현재 디렉토리의 `README.md` 대상.

**활용 예시**:
```
/suntech:doc README.md
/suntech:doc WEB/OEE_SCI/AI_STRATEGY_V2.md
/suntech:doc PSOC/280CTP_IoT_INTEGRATED/SRAM_최적화_분석보고서.md
```

**규칙 원본**: 가장 가까운 `CLAUDE.md`의 `.md → .html 변환 스타일 가이드` 섹션

**스킬 파일**: `C:\Users\luvsd\.claude\commands\suntech\doc.md`

---

### /suntech:shot — Playwright JPEG 스크린샷

```
/suntech:shot [URL] [--width 1920] [--height 1080] [--full]
```

**용도**: 로컬 URL을 Playwright headless Chromium으로 캡처하여 JPEG 저장.
빠른 UI 검증, 개발 중 화면 확인, 변경 전후 비교에 사용.

**활용 예시**:
```
/suntech:shot http://localhost/dev/OEE_SCI/OEE_SCI_V2/
/suntech:shot http://localhost/dev/ROBOT_SIMULATION/ROBOT_SIMULATION_V1/ --full
/suntech:shot http://localhost/dev/CTP280_API/
```

**저장 경로**: `C:\SUNTECH_DEV_CLAUDECODE\.palywright_screen_shot\{파일명}.jpeg`

**규칙 원본**: `WEB/CLAUDE.md`의 "Playwright 실행 환경" 및 "인라인 스크립트 템플릿" 섹션

**스킬 파일**: `C:\Users\luvsd\.claude\commands\suntech\shot.md`

---

## 스킬 선택 가이드

```
새 프로젝트 폴더 생성         →  /suntech:init
문서를 HTML로 변환            →  /suntech:doc
UI 화면 캡처/검증             →  /suntech:shot

코드 버그 추적                →  /sc:troubleshoot
코드 분석 (품질/보안/성능)    →  /sc:analyze
코드 개선·리팩토링            →  /sc:improve
데드코드·구조 정리            →  /sc:cleanup
신규 기능 구현                →  /sc:implement
설계 문서 작성                →  /sc:design
코드·개념 설명                →  /sc:explain
git 커밋 메시지 생성          →  /sc:git
```
