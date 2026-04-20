# 01. 프로젝트 개요

## 기본 정보

| 항목 | 내용 |
|---|---|
| 프로젝트명 | ST500 LockMaker |
| 폴더명 | ST500_LOCKMAKER_V1 |
| 버전 | 1.0.0 |
| 타입 | PWA (Progressive Web App) |
| 프레임워크 | Vue 3 + Vite 8 |
| 개발 시작 | 2026-04-17 |
| 원본 앱 | lockmaker_211206 (Android, Kotlin/Java, 퇴사자 작성) |

## 프로젝트 목적

SUNTECH 컴퓨터 패턴 재봉기의 **잠금 코드(Lock Code)를 생성**하는 도구.

기존 Android 전용 앱을 **iOS + Android + PC 브라우저 모두 지원**하는 PWA로 리뉴얼.

## 핵심 비즈니스 로직

1. **디바이스 등록**: 사용자 이름 + 디바이스 UUID를 서버에 전송
2. **관리자 승인 대기**: 서버 관리자가 해당 디바이스를 승인할 때까지 1초 폴링
3. **코드 생성**: 승인 후 Old Code(기존 잠금코드) + Lock Day(잠금 일수)를 입력받아 New Code 산출
4. **UnLock**: Lock Day 대신 151을 사용 → 영구 잠금 해제 코드 생성

## 개발 배경

- 원본 앱(`lockmaker_211206`)은 퇴사자가 작성, 개발 문서 없음
- Android 전용 → iOS 사용 불가 문제 발생
- 원본 앱의 심각한 버그/보안 문제 존재 (스레드 오류, 파일 누수, 알고리즘 노출 등)
- PWA로 리뉴얼하여 크로스플랫폼 + 보안 개선

## 기술 스택

| 범주 | 기술 |
|---|---|
| 프레임워크 | Vue 3 (Composition API, `<script setup>`) |
| 빌드 도구 | Vite 8 |
| 상태 관리 | Pinia 3 |
| 라우터 | Vue Router 4 (Hash History) |
| PWA | vite-plugin-pwa 1.2 + Workbox |
| HTTP | 브라우저 내장 `fetch()` API |
| 스토리지 | `localStorage` (디바이스 ID, 사용자 이름 영구 저장) |
| 언어 | JavaScript (ES Modules) |

## 원본 앱과의 차이점

| 항목 | 원본 (Android) | 신규 (PWA) |
|---|---|---|
| 플랫폼 | Android 전용 | iOS + Android + PC |
| 언어 | Kotlin + Java | JavaScript (Vue 3) |
| 스레드 | 메인스레드 네트워크 (버그) | async/await (정상) |
| 파일 저장 | 내부 파일시스템 | localStorage |
| 디바이스 ID | Android ID | crypto.randomUUID() |
| 서버 URL | 코드 4곳 하드코딩 | .env 단일 관리 |
| 배포 방식 | APK 재빌드·재배포 | 서버 파일 교체 즉시 반영 |
| 앱스토어 | Play Store 필요 | 불필요 (브라우저 접속) |
