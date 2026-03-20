# OTA 구현 가이드 — CTP280 IoT 시리즈

> 최초 작성: 2026-03-19
> 기준 구현: `280CTP_IoT_INTEGRATED_V2_BLACK_CPU`
> 목적: 다른 버전에서 OTA 기능을 **동일하게 재현**하기 위한 완전한 구현 참조 문서

---

## 목차

1. [OTA 아키텍처 개요](#1-ota-아키텍처-개요)
2. [신규 파일 목록](#2-신규-파일-목록)
3. [기존 파일 수정 목록](#3-기존-파일-수정-목록)
4. [otaMenu.h — 헤더 전체](#4-otamenuh--헤더-전체)
5. [otaMenu.c — 핵심 코드 스니펫](#5-otamenuc--핵심-코드-스니펫)
6. [lib/WIFI.c 수정 내용](#6-libwific-수정-내용)
7. [lib/widget.c 수정 내용](#7-libwidgetc-수정-내용)
8. [lib/server.h 수정 내용](#8-libserverh-수정-내용)
9. [Design.cyprj 등록 방법](#9-designcyprj-등록-방법)
10. [웹서버 API 구조 및 펌웨어 업로드 절차](#10-웹서버-api-구조-및-펌웨어-업로드-절차)
11. [W25QXX Flash 메모리 레이아웃](#11-w25qxx-flash-메모리-레이아웃)
12. [2-Stage 자동 체크 흐름](#12-2-stage-자동-체크-흐름)
13. [이식 시 주의사항](#13-이식-시-주의사항)
14. [빌드 에러 및 해결책](#14-빌드-에러-및-해결책)

---

## 1. OTA 아키텍처 개요

```
[OTA 전체 흐름]

■ 자동 체크 (백그라운드)
  부팅 → initAndon() → otaAutoCheckInit()
       ↓ 20초 대기 (ANDON 초기화 완료 대기)
  wifiLoop() → otaAutoCheckLoop() → wifi_cmd_ota_auto(url)
       ↓ WiFi 응답
  WIFI_CMD_OTA_AUTO 케이스 → otaHandleAutoVersionResponse()
       ↓ 신버전 감지 시
  g_otaUpdateAvailable = TRUE → DrawHeader() → otaDrawUpdateBadge() [헤더 배지]
       ↓ 24시간 후 반복

■ 수동 업데이트 (메뉴)
  사용자 → OTA 메뉴 진입 → doOtaUpdate(this, TRUE)
       ↓ WiFi 연결 확인
  requestVersion() → wifi_cmd_ota_version(url)
       ↓ WIFI_CMD_OTA_VERSION 케이스 → otaHandleVersionResponse()
  버전 비교 → [최신] "Up to date" 표시 or [신버전] SHOW_INFO 화면
       ↓ 사용자 UPDATE 버튼
  requestNextChunk() × N → wifi_cmd_ota_chunk(url)
       ↓ WIFI_CMD_OTA_CHUNK 케이스 → otaHandleChunkResponse()
  hex 디코딩 → W25qxx_WriteSector(Sector 32~) → CRC 누적
       ↓ 전체 수신 완료
  writeOtaFlag() → W25qxx_WriteSector(Sector 30) → CySoftwareReset()
       ↓
  부트로더 부팅 → OTA_FLAG_BLOCK 감지 → 외부Flash → 내부Flash 프로그래밍

■ 핵심 설계 원칙
  - OTA HTTP GET: _wifi_send_httpget_ota() 전용 함수 사용
    (일반 _wifi_send_httpget()은 host의 pathPart를 URL에 자동 추가하므로 OTA에 사용 금지)
  - OTA API 경로: DEFAULT_OTA_API_PATH (server.h) 절대경로 기반
  - ANDON API 경로: DEFAULT_API_ENDPOINT — OTA와 완전 독립
```

---

## 2. 신규 파일 목록

이 파일들을 `Design.cydsn/` 루트에 생성한다.

| 파일 | 위치 | 설명 |
|------|------|------|
| `otaMenu.c` | `Design.cydsn/` | OTA 수동 메뉴 + 자동 체크 상태 머신 전체 |
| `otaMenu.h` | `Design.cydsn/` | OTA 상수, 구조체, 상태 enum, 공개 API |

---

## 3. 기존 파일 수정 목록

| 파일 | 수정 내용 | 관련 섹션 |
|------|---------|---------|
| `lib/WIFI.c` | OTA 전용 HTTP GET 함수, 3개 WiFi 커맨드 케이스, wifiLoop 훅 | §6 |
| `lib/widget.c` | `DrawHeader()`에 `otaDrawUpdateBadge()` 호출 추가 | §7 |
| `lib/server.h` | `DEFAULT_OTA_API_PATH` 상수 추가 | §8 |
| `Design.cyprj` | `otaMenu.c`, `otaMenu.h` XML 직접 등록 | §9 |
| `andonApi.c` | `initAndon()` 마지막에 `otaAutoCheckInit()` 호출 추가 | §12 |

---

## 4. otaMenu.h — 헤더 전체

```c
/* ========================================
 * CTP280 OTA Update Menu
 * Copyright Suntech, 2026
 * ========================================
*/
#ifndef _OTA_MENU_H_
#define _OTA_MENU_H_

#include "main.h"
#include "lib/menu.h"

/* ─── OTA 설정 상수 ─────────────────────────────────────────── */
#define OTA_CHUNK_SIZE          400u    /* 청크 크기: hex 800자 + JSON 오버헤드 < 2048 버퍼 */
#define OTA_FLAG_SECTOR         30u     /* OTA 제어 블록 저장 섹터 */
#define OTA_FIRMWARE_SECTOR     32u     /* OTA 펌웨어 데이터 시작 섹터 */
#define OTA_FLAG_MAGIC          "OTAFLG"

/* 2-Stage 자동 체크 타이머 설정 */
#define OTA_AUTO_INIT_DELAY_MS  20000ul  /* 부팅 후 최초 체크 딜레이: 20초 */
#define OTA_AUTO_PERIOD_MS      86400000ul /* 24시간마다 자동 체크 */

/* ─── OTA 제어 블록 (W25QXX Sector 30에 저장) ───────────────── */
typedef struct {
    char   magic[8];       /* "OTAFLG\0" */
    char   version[16];    /* 예: "V2.0.1" */
    uint32 firmwareSize;   /* 전체 펌웨어 크기 (bytes) */
    uint16 crc;            /* CRC16-CCITT */
    uint8  status;         /* 0xFF=없음, 0x01=업데이트 대기, 0x02=적용 완료 */
    uint8  reserved[5];
} OTA_FLAG_BLOCK;          /* 36 bytes */

/* ─── 수동 메뉴 상태 머신 ────────────────────────────────────── */
typedef enum {
    OTA_STATE_IDLE = 0,
    OTA_STATE_CHECK_VERSION,
    OTA_STATE_WAITING_VERSION,
    OTA_STATE_SHOW_INFO,
    OTA_STATE_UP_TO_DATE,
    OTA_STATE_DOWNLOADING,
    OTA_STATE_WAITING_CHUNK,
    OTA_STATE_WRITE_FLAG,
    OTA_STATE_COMPLETE,
    OTA_STATE_ERROR,
} OTA_STATE;

/* ─── 자동 체크 상태 ─────────────────────────────────────────── */
typedef enum {
    OTA_AUTO_IDLE = 0,         /* 비활성 */
    OTA_AUTO_WAIT_INIT,        /* 초기 딜레이 대기 (20초) */
    OTA_AUTO_REQUESTING,       /* HTTP 요청 발송 → WiFi 응답 대기 */
    OTA_AUTO_WAIT_PERIODIC,    /* 24시간 대기 */
} OTA_AUTO_STATE;

/* ─── 공개 변수 ─────────────────────────────────────────────── */
extern OTA_STATE      g_otaState;
extern OTA_AUTO_STATE g_otaAutoState;
extern uint8          g_otaUpdateAvailable;   /* TRUE = 새 버전 있음 → 헤더 배지 표시 */
extern char           g_otaLatestVersion[16];
extern uint32         g_otaFirmwareSize;
extern uint32         g_otaDownloadOffset;
extern uint16         g_otaCRC;

/* ─── 수동 메뉴 함수 ─────────────────────────────────────────── */
void initOtaMenu(void);
int  doOtaUpdate(void *this, uint8 reflash);

/* ─── 2-Stage 자동 체크 함수 ──────────────────────────────────
 * otaAutoCheckInit() : initAndon() 에서 호출 → 20초 카운트다운 시작
 * otaAutoCheckLoop() : wifiLoop() 에서 호출 → 타이머 감시 + 요청 트리거
 * otaDrawUpdateBadge(): DrawHeader() 에서 호출 → 업데이트 있으면 아이콘 표시  */
void otaAutoCheckInit(void);
void otaAutoCheckLoop(void);
void otaDrawUpdateBadge(void);

/* ─── WiFi 응답 콜백 (WIFI.c에서 호출) ──────────────────────── */
void otaHandleVersionResponse    (char *json, uint16 size); /* 수동 메뉴용 */
void otaHandleChunkResponse      (char *json, uint16 size); /* 다운로드용  */
void otaHandleAutoVersionResponse(char *json, uint16 size); /* 자동 체크용 */

#endif /* _OTA_MENU_H_ */
/* [] END OF FILE */
```

---

## 5. otaMenu.c — 핵심 코드 스니펫

### 5.1 파일 상단 include / 전역 변수

```c
#include "otaMenu.h"
#include "lib/widget.h"
#include "lib/WIFI.h"
#include "lib/w25qxx.h"
#include "lib/server.h"
#include "lib/sysTick.h"
#include "lib/jsmn.h"
#include "package.h"

OTA_STATE      g_otaState          = OTA_STATE_IDLE;
OTA_AUTO_STATE g_otaAutoState      = OTA_AUTO_IDLE;
uint8          g_otaUpdateAvailable = FALSE;

char   g_otaLatestVersion[16] = {0};
uint32 g_otaFirmwareSize      = 0;
uint32 g_otaDownloadOffset    = 0;
uint16 g_otaCRC               = 0;

static uint8 g_otaAutoTimerIdx = 0xFF;   /* 0xFF = 미등록 */
static uint8 g_chunkBuf[OTA_CHUNK_SIZE];
```

### 5.2 HTTP 요청 함수들

```c
static void requestVersion(void)
{
    char url[80];
    snprintf(url, sizeof(url), "%s/version.php", DEFAULT_OTA_API_PATH);
    wifi_cmd_ota_version(url);
    g_otaState = OTA_STATE_WAITING_VERSION;
}

static void requestNextChunk(void)
{
    char url[120];
    snprintf(url, sizeof(url), "%s/firmware.php?offset=%lu&size=%u",
             DEFAULT_OTA_API_PATH, g_otaDownloadOffset, OTA_CHUNK_SIZE);
    wifi_cmd_ota_chunk(url);
    g_otaState = OTA_STATE_WAITING_CHUNK;
}
```

### 5.3 OTA 플래그 기록

```c
static void writeOtaFlag(void)
{
    OTA_FLAG_BLOCK flag;
    memset(&flag, 0xFF, sizeof(flag));
    memcpy(flag.magic, OTA_FLAG_MAGIC, 7u);
    flag.magic[7]     = '\0';
    strncpy(flag.version, g_otaLatestVersion, 15u);
    flag.version[15]  = '\0';
    flag.firmwareSize = g_otaFirmwareSize;
    flag.crc          = g_otaCRC;
    flag.status       = 0x01u;   /* 0x01 = 업데이트 대기 */

    W25qxx_EraseSector(OTA_FLAG_SECTOR);
    W25qxx_WriteSector((uint8*)&flag, OTA_FLAG_SECTOR, 0u, sizeof(OTA_FLAG_BLOCK));
}
```

### 5.4 자동 체크 — 초기화 (initAndon()에서 호출)

```c
void otaAutoCheckInit(void)
{
    g_otaAutoTimerIdx = registerCounter_1ms();   /* sysTick 타이머 등록 */
    setCountMax_1ms(g_otaAutoTimerIdx, OTA_AUTO_INIT_DELAY_MS);
    g_otaAutoState = OTA_AUTO_WAIT_INIT;
}
```

### 5.5 자동 체크 — 루프 (wifiLoop()에서 호출)

```c
void otaAutoCheckLoop(void)
{
    char url[80];

    if(g_otaAutoTimerIdx == 0xFF) return;

    switch(g_otaAutoState)
    {
        case OTA_AUTO_WAIT_INIT:
            if(!isFinishCounter_1ms(g_otaAutoTimerIdx)) break;
            if(!g_network.isConnectAP) break;          /* WiFi 미연결 시 대기 */

            snprintf(url, sizeof(url), "%s/version.php", DEFAULT_OTA_API_PATH);
            wifi_cmd_ota_auto(url);
            g_otaAutoState = OTA_AUTO_REQUESTING;
        break;

        case OTA_AUTO_REQUESTING:
            /* WIFI.c의 WIFI_CMD_OTA_AUTO 케이스에서 처리 후
               otaHandleAutoVersionResponse() 호출 → OTA_AUTO_WAIT_PERIODIC 전환 */
        break;

        case OTA_AUTO_WAIT_PERIODIC:
            if(!isFinishCounter_1ms(g_otaAutoTimerIdx)) break;
            if(!g_network.isConnectAP) break;

            snprintf(url, sizeof(url), "%s/version.php", DEFAULT_OTA_API_PATH);
            wifi_cmd_ota_auto(url);
            g_otaAutoState = OTA_AUTO_REQUESTING;
        break;

        default:
        break;
    }
}
```

### 5.6 자동 버전 응답 처리

```c
void otaHandleAutoVersionResponse(char *json, uint16 size)
{
    jsmn_parser p;
    jsmntok_t   t[32];
    int         r, i;

    jsmn_init(&p);
    r = jsmn_parse(&p, json, size, t, 32);
    if(r < 0)
    {
        g_otaAutoState = OTA_AUTO_WAIT_PERIODIC;
        setCountMax_1ms(g_otaAutoTimerIdx, OTA_AUTO_PERIOD_MS);
        return;
    }

    for(i = 1; i < r - 1; i++)
    {
        if(jsoneq(json, &t[i], "version") == 0)
        {
            uint16 len = (uint16)(t[i+1].end - t[i+1].start);
            if(len >= sizeof(g_otaLatestVersion)) len = sizeof(g_otaLatestVersion) - 1u;
            strncpy(g_otaLatestVersion, json + t[i+1].start, len);
            g_otaLatestVersion[len] = '\0';
            i++;
        }
    }

    /* 현재 버전보다 최신이면 배지 표시 */
    if(compareVersion(g_otaLatestVersion, PROJECT_FIRMWARE_VERSION) > 0)
        g_otaUpdateAvailable = TRUE;

    g_otaAutoState = OTA_AUTO_WAIT_PERIODIC;
    setCountMax_1ms(g_otaAutoTimerIdx, OTA_AUTO_PERIOD_MS);
}
```

> **주의**: `compareVersion()`은 `otaMenu.c` 내부 static 함수. 포맷: `"V{major}.{minor}.{patch}"`

### 5.7 헤더 배지 표시

```c
void otaDrawUpdateBadge(void)
{
    if(!g_otaUpdateAvailable) return;
    /* 우측 상단 헤더 영역에 "UPD" 텍스트 배지 표시 */
    LCD_printf(g_SCREEN_WIDTH - 45, 5, YELLOW, BLACK, SmallFont8x12, "UPD");
}
```

### 5.8 버전 JSON 응답 처리 (수동 메뉴용)

```c
void otaHandleVersionResponse(char *json, uint16 size)
{
    jsmn_parser p;
    jsmntok_t   t[32];
    int         r, i;

    jsmn_init(&p);
    r = jsmn_parse(&p, json, size, t, 32);
    if(r < 0) { g_otaState = OTA_STATE_ERROR; return; }

    for(i = 1; i < r - 1; i++)
    {
        if(jsoneq(json, &t[i], "version") == 0)
        {
            uint16 len = (uint16)(t[i+1].end - t[i+1].start);
            if(len >= sizeof(g_otaLatestVersion)) len = sizeof(g_otaLatestVersion) - 1u;
            strncpy(g_otaLatestVersion, json + t[i+1].start, len);
            g_otaLatestVersion[len] = '\0';
            i++;
        }
        else if(jsoneq(json, &t[i], "size") == 0)
        {
            char tmp[16] = {0};
            uint16 len = (uint16)(t[i+1].end - t[i+1].start);
            if(len >= sizeof(tmp)) len = sizeof(tmp) - 1u;
            strncpy(tmp, json + t[i+1].start, len);
            g_otaFirmwareSize = (uint32)atol(tmp);
            i++;
        }
    }

    if(compareVersion(g_otaLatestVersion, PROJECT_FIRMWARE_VERSION) <= 0)
    {
        g_otaState = OTA_STATE_UP_TO_DATE;
    }
    else
    {
        g_otaState = OTA_STATE_SHOW_INFO;
        displayVersionInfo();
    }
}
```

### 5.9 청크 응답 처리

```c
void otaHandleChunkResponse(char *json, uint16 size)
{
    jsmn_parser p;
    jsmntok_t   t[16];
    int         r, i;
    char       *hexData   = NULL;
    uint16      hexLen    = 0u;

    jsmn_init(&p);
    r = jsmn_parse(&p, json, size, t, 16);
    if(r < 0) { g_otaState = OTA_STATE_ERROR; return; }

    for(i = 1; i < r - 1; i++)
    {
        if(jsoneq(json, &t[i], "data") == 0)
        {
            hexData = json + t[i+1].start;
            hexLen  = (uint16)(t[i+1].end - t[i+1].start);
            i++;
        }
    }

    if(hexData == NULL || hexLen == 0u || (hexLen & 1u) != 0u)
    {
        g_otaState = OTA_STATE_ERROR;
        return;
    }

    {
        uint16 byteCount = hexLen / 2u;
        uint32 sector    = OTA_FIRMWARE_SECTOR + (g_otaDownloadOffset / 4096ul);
        uint32 offset    = g_otaDownloadOffset % 4096ul;
        uint16 k;

        hexDecode(hexData, g_chunkBuf, byteCount);

        /* 섹터 경계에서만 Erase */
        if(offset == 0u)
            W25qxx_EraseSector((uint32)sector);

        W25qxx_WriteSector(g_chunkBuf, (uint32)sector, offset, byteCount);

        /* CRC 누적 */
        for(k = 0; k < byteCount; k++)
            g_otaCRC = crc16Update(g_otaCRC, g_chunkBuf[k]);

        g_otaDownloadOffset += byteCount;
    }

    if(g_otaDownloadOffset >= g_otaFirmwareSize)
    {
        /* 다운로드 완료 → 상태 전송 → 플래그 기록 */
        char statusUrl[120];
        snprintf(statusUrl, sizeof(statusUrl),
            "%s/status.php?mac=%s&status=done&version=%s",
            DEFAULT_OTA_API_PATH, g_network.MAC, g_otaLatestVersion);
        wifi_cmd_ota_version(statusUrl);   /* 단순 GET — 응답 무시 */

        writeOtaFlag();
        g_otaState = OTA_STATE_WRITE_FLAG;
    }
    else
    {
        displayProgress();
        requestNextChunk();
    }
}
```

---

## 6. lib/WIFI.c 수정 내용

### 6.1 WIFI_CMD 열거형에 추가 (WIFI.h)

```c
/* WIFI.h 또는 WIFI.c 내 열거형에 아래 3개 추가 */
typedef enum {
    WIFI_CMD_IDLE = 0,
    /* ... 기존 항목들 ... */
    WIFI_CMD_OTA_VERSION,   /* 수동 버전 확인 */
    WIFI_CMD_OTA_AUTO,      /* 자동 버전 확인 */
    WIFI_CMD_OTA_CHUNK,     /* 펌웨어 청크 다운로드 */
} WIFI_CMD;
```

### 6.2 OTA 전용 HTTP GET 함수 (핵심)

```c
/* ANDON의 _wifi_send_httpget()과 달리 host에서 IP만 추출 — pathPart 무시 */
static void _wifi_send_httpget_ota(const char *absolutePath, uint8 cmd, uint16 timeoutMs)
{
    char  ipPart[50];
    char *slash = strchr(g_ptrServer->host, '/');

    if(slash != NULL)
    {
        int ipLen = (int)(slash - g_ptrServer->host);
        strncpy(ipPart, g_ptrServer->host, ipLen);
        ipPart[ipLen] = '\0';
    }
    else
    {
        strncpy(ipPart, g_ptrServer->host, sizeof(ipPart) - 1);
        ipPart[sizeof(ipPart) - 1] = '\0';
    }

    wifi_printf("AT*ICT*HTTPGET=http://%s:%d%s\r\n",
                ipPart, g_ptrServer->port, absolutePath);
    g_wifi_cmd = cmd;
    setCountMax_1ms(g_index_Wifi_Test, timeoutMs);
}
```

> **왜 별도 함수인가?** `_wifi_send_httpget()`은 `g_ptrServer->host` 전체를 URL에 붙인다.
> host = `"49.247.26.228/CTP280_API"` 일 때 ANDON URL은 올바르게 형성되지만
> OTA URL은 `http://49.247.26.228/CTP280_API/CTP280_OTA/CTP280_OTA_V1/api/version.php` 처럼
> `/CTP280_API` pathPart가 삽입되어 404 오류가 발생한다.
> OTA는 IP만 추출하여 절대경로를 직접 조합해야 한다.

### 6.3 공개 WiFi 명령 함수 3개

```c
void wifi_cmd_ota_version(const char *absolutePath)
{
    _wifi_send_httpget_ota(absolutePath, WIFI_CMD_OTA_VERSION, 5000u);
}

void wifi_cmd_ota_auto(const char *absolutePath)
{
    _wifi_send_httpget_ota(absolutePath, WIFI_CMD_OTA_AUTO, 5000u);
}

void wifi_cmd_ota_chunk(const char *absolutePath)
{
    _wifi_send_httpget_ota(absolutePath, WIFI_CMD_OTA_CHUNK, 10000u);
}
```

> 이 3개 함수 선언을 `lib/WIFI.h`에도 추가한다:
> ```c
> void wifi_cmd_ota_version(const char *absolutePath);
> void wifi_cmd_ota_auto   (const char *absolutePath);
> void wifi_cmd_ota_chunk  (const char *absolutePath);
> ```

### 6.4 wifi_get_response() switch 케이스 추가

```c
case WIFI_CMD_OTA_VERSION:
    if(isFinishCounter_1ms(g_index_Wifi_Test))
    {
        otaHandleVersionResponse("{}", 2u);
        break;
    }
    if(STRSTR_WIFI_BUFFER("ICT*HTTPGET:OK"))
    {
        resetCounter_1ms(g_index_Wifi_Test);
    }
    else if(STRSTR_WIFI_BUFFER("ICT*HTTPGET:ERROR"))
    {
        otaHandleVersionResponse("{}", 2u);
    }
    else if(STRSTR_WIFI_BUFFER("ICT*HTTPBODY:"))
    {
        char *body = strstr(g_WIFI_ReceiveBuffer, "ICT*HTTPBODY:");
        if(body != NULL)
        {
            body += 13;  /* "ICT*HTTPBODY:" 길이 */
            uint16 bodyLen = (uint16)strlen(body);
            otaHandleVersionResponse(body, bodyLen);
        }
    }
    else if(STRSTR_WIFI_BUFFER("ICT*HTTPCLOSE:OK"))
    {
        g_wifi_cmd = WIFI_CMD_IDLE;
    }
break;

case WIFI_CMD_OTA_AUTO:
    if(isFinishCounter_1ms(g_index_Wifi_Test))
    {
        otaHandleAutoVersionResponse("{}", 2u);
        break;
    }
    if(STRSTR_WIFI_BUFFER("ICT*HTTPGET:OK"))
    {
        resetCounter_1ms(g_index_Wifi_Test);
    }
    else if(STRSTR_WIFI_BUFFER("ICT*HTTPGET:ERROR"))
    {
        otaHandleAutoVersionResponse("{}", 2u);
    }
    else if(STRSTR_WIFI_BUFFER("ICT*HTTPBODY:"))
    {
        char *body = strstr(g_WIFI_ReceiveBuffer, "ICT*HTTPBODY:");
        if(body != NULL)
        {
            body += 13;
            uint16 bodyLen = (uint16)strlen(body);
            otaHandleAutoVersionResponse(body, bodyLen);
        }
    }
    else if(STRSTR_WIFI_BUFFER("ICT*HTTPCLOSE:OK"))
    {
        g_wifi_cmd = WIFI_CMD_IDLE;
    }
break;

case WIFI_CMD_OTA_CHUNK:
    if(isFinishCounter_1ms(g_index_Wifi_Test))
    {
        otaHandleChunkResponse("{}", 2u);
        break;
    }
    if(STRSTR_WIFI_BUFFER("ICT*HTTPGET:OK"))
    {
        resetCounter_1ms(g_index_Wifi_Test);
    }
    else if(STRSTR_WIFI_BUFFER("ICT*HTTPGET:ERROR"))
    {
        otaHandleChunkResponse("{}", 2u);
    }
    else if(STRSTR_WIFI_BUFFER("ICT*HTTPBODY:"))
    {
        char *body = strstr(g_WIFI_ReceiveBuffer, "ICT*HTTPBODY:");
        if(body != NULL)
        {
            body += 13;
            uint16 bodyLen = (uint16)strlen(body);
            otaHandleChunkResponse(body, bodyLen);
        }
    }
    else if(STRSTR_WIFI_BUFFER("ICT*HTTPCLOSE:OK"))
    {
        g_wifi_cmd = WIFI_CMD_IDLE;
    }
break;
```

### 6.5 wifiLoop()에 자동 체크 훅 추가

```c
void wifiLoop(void)
{
    /* ... 기존 코드 ... */

    if(andonLoop() == FALSE)
    {
        /* ANDON이 요청 없을 때 → OTA 자동 체크 기회 제공 */
        otaAutoCheckLoop();
    }
}
```

---

## 7. lib/widget.c 수정 내용

```c
#include "../otaMenu.h"    /* DrawHeader()에서 otaDrawUpdateBadge() 호출 */

void DrawHeader(void)
{
    DrawWifi();
    DrawTitle();
    otaDrawUpdateBadge();   /* OTA 업데이트 있을 때 헤더에 배지 표시 */
    DrawHorizontalLine(0, g_SCREEN_WIDTH - 1, DEFAULT_TOP_TITLE_HEIGHT - 1, WHITE);
}
```

---

## 8. lib/server.h 수정 내용

```c
/* 기존 상수들 아래에 추가 */

/* OTA 서버 경로 — ANDON API(pathPart)와 독립된 절대 경로 */
#define DEFAULT_OTA_API_PATH "/CTP280_OTA/CTP280_OTA_V1/api"
```

> `DEFAULT_OTA_API_PATH`는 IP 없이 경로만 포함한다.
> 실제 HTTP 요청 시 `_wifi_send_httpget_ota()`가 `g_ptrServer->host`에서 IP를 추출하여 조합.

---

## 9. Design.cyprj 등록 방법

PSoC Creator GUI 없이 `.cyprj` XML 파일을 직접 편집한다.

### 9.1 SOURCE_C 등록

`defective.c` 항목을 찾아 **그 바로 다음**에 `otaMenu.c` 항목 추가:

```xml
<SOURCE_C NAME="defective.c" FULL_PATH="%5C%5C...defective.c"/>
<SOURCE_C NAME="otaMenu.c" FULL_PATH="%5C%5C...otaMenu.c"/>
```

> `FULL_PATH` 값은 실제 절대경로를 URL 인코딩(`%5C` = `\`)으로 표기한다.
> 기존 다른 파일의 `FULL_PATH` 형식을 복사하여 파일명만 변경하면 된다.

### 9.2 HEADER 등록

`andonApi.h` 항목을 찾아 **그 바로 다음**에 `otaMenu.h` 항목 추가:

```xml
<HEADER NAME="andonApi.h" FULL_PATH="%5C%5C...andonApi.h"/>
<HEADER NAME="otaMenu.h" FULL_PATH="%5C%5C...otaMenu.h"/>
```

---

## 10. 웹서버 API 구조 및 펌웨어 업로드 절차

OTA 웹서버 배포 경로: `http://{서버IP}/CTP280_OTA/CTP280_OTA_V1/api/`

### 10.0 펌웨어 업로드 절차 (신규 버전 배포 시)

> **관리 웹 페이지**: `http://49.247.26.228/CTP280_OTA/CTP280_OTA_V1/index.html`
> **로컬 소스 경로**: `C:\SUNTECH_DEV_CLAUDECODE\WEB\CTP280_OTA\CTP280_OTA_V1\index.html`

#### 순서 (반드시 이 순서 준수)

**① `package.h` 버전 변경 → 빌드**

```c
// Design.cydsn/package.h
#define PROJECT_FIRMWARE_VERSION "V2.0.1"   // 이전 버전보다 높게 설정
```

PSoC Creator → Build → Clean and Build Design

빌드 결과물: `Design.cydsn/CortexM0/ARM_GCC_541/Debug/Design.hex`

**② 웹 페이지에서 업로드**

```
http://49.247.26.228/CTP280_OTA/CTP280_OTA_V1/index.html
```

| 입력 항목 | 값 |
|----------|-----|
| 버전 입력란 | `V2.0.1` ← package.h와 동일한 버전 |
| 파일 선택 | `Design.hex` (`.hex` 선택 시 BIN 자동 변환됨) |
| 릴리즈 노트 | 변경 내용 요약 (선택) |

**HEX → BIN 자동 변환 기능 (2026-03-20 추가):**
- `.hex` 파일 선택 시 주황색 안내 표시: "Intel HEX 파일 감지 → BIN으로 자동 변환됩니다"
- 업로드 버튼 클릭 → 브라우저에서 Intel HEX 파싱 → 이진 변환 → 서버에 BIN 전송
- `arm-none-eabi-objcopy` 도구 없이 브라우저에서 직접 처리

**③ 디바이스 자동 업데이트 확인**

| 방식 | 타이밍 |
|------|--------|
| 자동 체크 | 부팅 후 20초 → 이후 24시간 주기 |
| 수동 | LCD → MENU → OTA UPDATE |

> **주의**: `package.h`를 변경하지 않고 서버에 같은 버전을 올리면 디바이스는 업데이트하지 않습니다 (`compareVersion() <= 0`).
> 반드시 서버 버전이 디바이스 현재 버전보다 높아야 OTA가 시작됩니다.

---

### 10.1 version.php — 버전 확인

**요청**: `GET /CTP280_OTA/CTP280_OTA_V1/api/version.php`

**응답 (JSON)**:
```json
{
  "version": "V2.1.0",
  "size": 108988
}
```

| 필드 | 타입 | 설명 |
|------|------|------|
| `version` | string | 최신 펌웨어 버전 (형식: `V{major}.{minor}.{patch}`) |
| `size` | number | 전체 펌웨어 크기 (bytes) |

### 10.2 firmware.php — 청크 다운로드

**요청**: `GET /CTP280_OTA/CTP280_OTA_V1/api/firmware.php?offset={N}&size={400}`

| 파라미터 | 설명 |
|----------|------|
| `offset` | 바이트 오프셋 (0부터 시작) |
| `size` | 청크 크기 (고정 400 bytes) |

**응답 (JSON)**:
```json
{
  "data": "AABB...CCDD"
}
```

| 필드 | 타입 | 설명 |
|------|------|------|
| `data` | string | 펌웨어 바이너리의 HEX 문자열 (400 bytes → 800 hex 문자) |

> **청크 크기 결정 이유**: 400 bytes = hex 800자 + JSON 오버헤드 ≈ 850자 < 2048 버퍼

### 10.3 status.php — 완료 보고

**요청**: `GET /CTP280_OTA/CTP280_OTA_V1/api/status.php?mac={MAC}&status=done&version={VER}`

응답은 무시됨 (단순 로깅 목적).

---

## 11. W25QXX Flash 메모리 레이아웃

```
W25Qxx SPI Flash (4KB 섹터 단위)
├── Sector  0~15 : 외부 플래시 설정 (CONFIG, MACHINE_PARAMETER 등)
├── Sector 16~29 : 예약 / 미사용
├── Sector 30    : OTA_FLAG_BLOCK (36 bytes) ← OTA 제어 블록
│   ├── magic[8]       "OTAFLG\0"
│   ├── version[16]    최신 버전 문자열
│   ├── firmwareSize   uint32 — 전체 크기 (bytes)
│   ├── crc            uint16 — CRC16-CCITT
│   └── status         0xFF=없음, 0x01=업데이트 대기, 0x02=적용 완료
├── Sector 31    : 예약
└── Sector 32~   : OTA 펌웨어 데이터 (청크 순서대로 기록)
    ├── Sector 32: offset 0    ~ 4095
    ├── Sector 33: offset 4096 ~ 8191
    └── ...
```

> **Erase 시점**: 각 섹터의 첫 청크(`offset % 4096 == 0`)에서만 `W25qxx_EraseSector()` 호출.
> 나머지 청크는 동일 섹터에 연속 기록.

---

## 12. 2-Stage 자동 체크 흐름

```
[부팅]
  ↓
initAndon() 마지막 줄:
  otaAutoCheckInit()
    ├── registerCounter_1ms() → g_otaAutoTimerIdx 등록
    ├── setCountMax_1ms(idx, 20000ul) → 20초 타이머 시작
    └── g_otaAutoState = OTA_AUTO_WAIT_INIT

  ↓ 20초 경과 (ANDON 초기화 완료 대기)

wifiLoop() → andonLoop() == FALSE → otaAutoCheckLoop()
  OTA_AUTO_WAIT_INIT:
    isFinishCounter_1ms() == TRUE
    && g_network.isConnectAP == TRUE
    → wifi_cmd_ota_auto("/CTP280_OTA/.../version.php")
    → g_otaAutoState = OTA_AUTO_REQUESTING

  ↓ WiFi 응답 수신

WIFI_CMD_OTA_AUTO 케이스:
  ICT*HTTPBODY: {"version":"V2.1.0","size":108988}
  → otaHandleAutoVersionResponse()
    ├── 버전 파싱 → g_otaLatestVersion
    ├── compareVersion(latest, current) > 0 → g_otaUpdateAvailable = TRUE
    ├── setCountMax_1ms(idx, 86400000ul) → 24시간 타이머 재설정
    └── g_otaAutoState = OTA_AUTO_WAIT_PERIODIC

  ↓ g_otaUpdateAvailable = TRUE

DrawHeader() 매 프레임:
  otaDrawUpdateBadge()
  → g_otaUpdateAvailable == TRUE → LCD 헤더에 "UPD" 배지 표시

  ↓ 24시간 경과

OTA_AUTO_WAIT_PERIODIC:
  → wifi_cmd_ota_auto() 재요청 → 반복
```

---

## 13. 이식 시 주의사항

### 13.1 jsoneq() 사용법

이 프로젝트에는 `jsonKeyMatch()`가 **없다**. `jsmn.h`의 `jsoneq()` 사용.

```c
/* 잘못된 사용 */
if(jsonKeyMatch(json, &t[i], "version")) { ... }

/* 올바른 사용 — jsoneq() 일치 시 0 반환 */
if(jsoneq(json, &t[i], "version") == 0) { ... }
```

### 13.2 색상 상수

`ST7789V.h` 기준으로 사용할 것:

| 잘못된 이름 | 올바른 이름 |
|------------|------------|
| `GREY` | `LIGHTGREY` (0xC618) |
| `DARK_GREY` | `DARKGREY` (0x7BEF) |

### 13.3 bootloader hex/elf

V2를 새로 분기할 때 `bootloader.cydsn/CortexM0/ARM_GCC_541/Debug/` 폴더에
`bootloader.hex`, `bootloader.elf` 파일이 없으면 빌드 실패.

이전 버전(V1_BLACK_CPU 등)에서 복사:
```powershell
$src = "경로\V1_BLACK_CPU\Project\bootloader.cydsn\CortexM0\ARM_GCC_541\Debug"
$dst = "경로\V2_BLACK_CPU\Project\bootloader.cydsn\CortexM0\ARM_GCC_541\Debug"
Copy-Item "$src\bootloader.hex" "$dst\bootloader.hex"
Copy-Item "$src\bootloader.elf" "$dst\bootloader.elf"
```

### 13.4 `andonApi.c`에 `otaAutoCheckInit()` 호출 추가

```c
/* andonApi.c — initAndon() 함수 맨 마지막에 추가 */
#include "otaMenu.h"   /* 파일 상단에 추가 */

void initAndon(void)
{
    /* ... 기존 ANDON 초기화 코드 ... */

    otaAutoCheckInit();   /* OTA 자동 체크 시작 (20초 후 첫 요청) */
}
```

### 13.5 widget.h IDX_SCROLL_UP/DOWN 값 확인 (⚠ 중요)

`lib/widget.h` enum의 `IDX_SCROLL_UP`, `IDX_SCROLL_DOWN` 값이 메뉴 child index와 충돌하지 않아야 한다.

```c
/* ❌ 원래 값 — child index(0, 1, 2...) 범위와 겹침 */
IDX_SCROLL_UP,    /* = 6 */
IDX_SCROLL_DOWN   /* = 7 */

/* ✅ 수정된 값 — NO_CLICK(0xFF) 바로 아래 고정값 사용 */
IDX_SCROLL_UP   = 0xFD,
IDX_SCROLL_DOWN = 0xFE
```

root 메뉴에 자식 노드가 6개 이상 있으면 child index 6이 `IDX_SCROLL_UP = 6`과 충돌한다.
이식 후 메뉴에 노드를 추가할 때 반드시 이 값이 올바르게 설정되었는지 확인한다.

---

### 13.6 OTA URL 경로 — 서버 환경에 맞게 수정

`lib/server.h`의 `DEFAULT_OTA_API_PATH`를 실제 서버 배포 경로로 변경:

```c
/* 예시: /CTP280_OTA/CTP280_OTA_V1/api */
#define DEFAULT_OTA_API_PATH "/CTP280_OTA/CTP280_OTA_V1/api"
```

Linux 서버는 **대소문자 구분**. 서버의 실제 폴더명과 정확히 일치시킬 것.

### 13.7 메모리 영향

> 실측 기준: V2_BLACK_CPU (2026-03-20 Clean and Build)

| 영역 | V1_BLACK_CPU | V2_BLACK_CPU (OTA 추가) | 증감 |
|------|-------------|------------------------|------|
| Flash (Application) | 108,988 bytes (41.6%) | 108,988 bytes (41.6%) | **±0** |
| Flash (전체) | 122,812 bytes (46.8%) | 122,812 bytes (46.8%) | **±0** |
| SRAM | 22,532 bytes (68.8%) | 22,532 bytes (68.8%) | **±0** |

> OTA 기능(otaMenu.c ~400줄) 추가에도 불구하고 메모리 증가 없음.
> 이유: `w25qxx.c`는 V1_BLACK_CPU에도 이미 포함되어 있었으므로 OTA 전용 신규 Flash 증가분이 미미함.

| 항목 | 설계 예상 | 실제 결과 |
|------|----------|---------|
| Flash | `otaMenu.c` ~400줄 → +8~10KB 예상 | ±0 (링커 최적화) |
| SRAM | `g_chunkBuf[400]` → +400 bytes 예상 | ±0 (측정치 동일) |
| Stack | `jsmntok_t t[32]` (지역) → +512 bytes 일시적 | 해당 없음 (정적 측정) |

SRAM 제약이 있을 경우 `OTA_CHUNK_SIZE`를 줄이면 `g_chunkBuf` 크기 감소 (단, 서버 `size` 파라미터도 동일하게 조정 필요).

---

## 14. 빌드 에러 및 해결책

| 에러 메시지 | 원인 | 해결책 |
|------------|------|--------|
| `lib/jsonUtil.h: No such file or directory` | `jsonUtil.h`가 `lib/`가 아닌 루트에 위치 | `#include "jsonUtil.h"` (경로 없이) |
| `'jsonKeyMatch' undeclared` | 이 프로젝트에 `jsonKeyMatch()` 없음 | `jsoneq(...) == 0` 패턴으로 교체 |
| `'GREY' undeclared` | `ST7789V.h`에 `GREY` 정의 없음 | `LIGHTGREY` 사용 |
| `'DARK_GREY' undeclared` | `ST7789V.h`에 `DARK_GREY` 정의 없음 | `DARKGREY` 사용 |
| `bootloader.hex: No such file` | V2 최초 분기 시 Debug 폴더 비어있음 | V1_BLACK_CPU Debug 폴더에서 PowerShell Copy-Item |
| OTA URL에 `/CTP280_API` 삽입됨 | `_wifi_send_httpget()` 사용 시 pathPart 자동 추가 | `_wifi_send_httpget_ota()` 전용 함수 사용 |
| `registerCounter_1ms` undeclared in otaMenu.c | `sysTick.h` 미포함 | `#include "lib/sysTick.h"` 추가 |
| OTA UPDATE 터치 시 메뉴 스크롤 업 또는 다른 메뉴로 이동 | `widget.h` `IDX_SCROLL_UP` 값이 OTA UPDATE child index와 충돌 | `lib/widget.h` enum에서 `IDX_SCROLL_UP = 0xFD`, `IDX_SCROLL_DOWN = 0xFE`로 설정 확인 |

---

*Copyright SUNTECH, 2026*
