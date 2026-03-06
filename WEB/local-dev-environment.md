# 로컬 개발 환경 상세 가이드

## 새 프로젝트 폴더 추가 시 질문 방법

새 프로젝트를 시작할 때 아래처럼 질문하면 바로 설정 가능:

```
"새 프로젝트 [프로젝트명] 추가해줘.
 - 로컬 폴더: C:\SUNTECH_DEV_CLAUDECODE\WEB\[프로젝트명]
 - localhost URL: http://localhost/dev/[프로젝트명]/[버전폴더]/
 - (필요시) 별도 가상호스트: [프로젝트명].test"
```

## 로컬 개발 워크플로우

1. **코드 작성**: `C:\SUNTECH_DEV_CLAUDECODE\WEB\[프로젝트명]\[버전폴더]\` 에서 직접 작업
2. **로컬 테스트**: `http://localhost/dev/[프로젝트명]/[버전폴더]/`
3. **디버깅**: `F5` → `Listen for Xdebug (Local - Laragon)` 선택
4. **브레이크포인트**: 코드 좌측 클릭 → 빨간 점 표시
5. **배포**: `Ctrl+Shift+P` → `SFTP: Upload` (WEB/.vscode/sftp.json 기준)

## Laragon 가상호스트 (Virtual Host) 추가

프로젝트별 독립 도메인 필요 시 Laragon에서 자동 생성:
- `C:\laragon\www\dev` 는 `C:\SUNTECH_DEV_CLAUDECODE\WEB` 으로 연결된 junction
- 모든 WEB 프로젝트가 자동으로 `http://localhost/dev/[프로젝트명]/[버전폴더]/` 로 접근 가능
- 별도 가상호스트 필요 시: Laragon 트레이 → `www` 폴더에 폴더 생성 → `http://[폴더명].test`

## Xdebug 설정 (php.ini)

경로: `C:\laragon\bin\php\php-7.4.33-nts-Win32-vc15-x64\php.ini`

```ini
[xdebug]
zend_extension=xdebug
xdebug.mode=debug
xdebug.start_with_request=yes
xdebug.client_host=127.0.0.1
xdebug.client_port=9003
xdebug.log_level=0
```

## launch.json (디버그 설정)

경로: `C:\SUNTECH_DEV_CLAUDECODE\WEB\.vscode\launch.json`

- **Listen for Xdebug (Local - Laragon)**: 로컬 파일 직접 디버깅
- **Listen for Xdebug (Remote - /var/www/html)**: 원격 서버 경로 매핑

## sftp.json (SFTP 연결)

경로: `C:\SUNTECH_DEV_CLAUDECODE\WEB\.vscode\sftp.json`

- host: 49.247.26.228, port: 22
- username: root
- remotePath: /var/www/html
- uploadOnSave: true

## PHP 버전 전환 방법 (필요 시)

Laragon 트레이 우클릭 → PHP → Version 선택
또는 `C:\laragon\usr\profile\default.ini` 에서 Version 수동 변경 후 재시작

## MySQL 접속

- phpMyAdmin: `http://localhost/phpmyadmin`
- HeidiSQL: Laragon 트레이 → MySQL → HeidiSQL
- 기본 계정: root / (비밀번호 없음)
