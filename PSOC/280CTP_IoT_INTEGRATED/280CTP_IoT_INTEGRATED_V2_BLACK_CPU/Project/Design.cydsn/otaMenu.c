/* ========================================
 * CTP280 OTA Update Menu
 * Copyright Suntech, 2026
 *
 * OTA 흐름:
 *   1. 서버 /ota/api/version.php → 최신 버전 확인
 *   2. 버전 비교 후 UPDATE/QUIT 표시
 *   3. UPDATE → /ota/api/firmware.php?offset=N&size=400 청크 다운로드
 *   4. 청크를 W25QXX Sector 32~ 에 순서대로 기록
 *   5. 완료 후 OTA 플래그 Sector 30에 기록 → CySoftwareReset()
 *   6. 부트로더가 OTA 플래그 감지 → 외부 Flash → 내부 Flash 프로그래밍
 * ========================================
*/
#include "otaMenu.h"
#include "lib/widget.h"
#include "lib/WIFI.h"
#include "lib/w25qxx.h"
#include "lib/server.h"
#include "lib/sysTick.h"
#include "lib/jsmn.h"
#include "package.h"

/* ─── 전역 변수 ────────────────────────────────────────────── */
OTA_STATE      g_otaState          = OTA_STATE_IDLE;
OTA_AUTO_STATE g_otaAutoState      = OTA_AUTO_IDLE;
uint8          g_otaUpdateAvailable = FALSE;   /* TRUE = 헤더 배지 표시 */

char      g_otaLatestVersion[16] = {0};
uint32    g_otaFirmwareSize   = 0;
uint32    g_otaDownloadOffset = 0;
uint16    g_otaCRC            = 0;

/* 자동 체크 타이머 인덱스 */
static uint8 g_otaAutoTimerIdx = 0xFF;   /* 0xFF = 미등록 */

/* 청크 디코딩용 버퍼: OTA_CHUNK_SIZE = 400 bytes */
static uint8 g_chunkBuf[OTA_CHUNK_SIZE];

/* ─── 내부 함수 선언 ─────────────────────────────────────────── */
static int8   compareVersion  (const char *a, const char *b);
static void   hexDecode       (const char *hex, uint8 *out, uint16 byteCount);
static uint16 crc16Update     (uint16 crc, uint8 byte);
static void   requestVersion  (void);
static void   requestNextChunk(void);
static void   writeOtaFlag    (void);
static void   displayVersionInfo(void);
static void   displayProgress   (void);
static void   displayError      (const char *msg);


/* ─── 버전 비교 ─────────────────────────────────────────────── */
/* 반환: 1 = a > b,  0 = a == b,  -1 = a < b
 * 형식: V{major}.{minor}.{patch}  (예: "V2.0.1")       */
static int8 compareVersion(const char *a, const char *b)
{
    int ma=0,mia=0,pa=0, mb=0,mib=0,pb=0;
    const char *pa_ = (a[0]=='V'||a[0]=='v') ? a+1 : a;
    const char *pb_ = (b[0]=='V'||b[0]=='v') ? b+1 : b;
    sscanf(pa_, "%d.%d.%d", &ma,  &mia, &pa);
    sscanf(pb_, "%d.%d.%d", &mb,  &mib, &pb);
    if(ma  != mb)  return (ma  > mb)  ? 1 : -1;
    if(mia != mib) return (mia > mib) ? 1 : -1;
    if(pa  != pb)  return (pa  > pb)  ? 1 : -1;
    return 0;
}


/* ─── HEX 디코딩 ─────────────────────────────────────────────── */
/* "AABB..." → {0xAA, 0xBB, ...}  byteCount = hex 문자열 길이 / 2 */
static void hexDecode(const char *hex, uint8 *out, uint16 byteCount)
{
    uint16 i;
    for(i = 0; i < byteCount; i++)
    {
        char  hi = hex[i*2], lo = hex[i*2+1];
        uint8 h  = (hi>='a') ? hi-'a'+10 : (hi>='A') ? hi-'A'+10 : hi-'0';
        uint8 l  = (lo>='a') ? lo-'a'+10 : (lo>='A') ? lo-'A'+10 : lo-'0';
        out[i]   = (h << 4) | l;
    }
}


/* ─── CRC16-CCITT (init=0xFFFF, poly=0x1021) ─────────────────── */
static uint16 crc16Update(uint16 crc, uint8 byte)
{
    uint8 i;
    crc ^= (uint16)byte << 8;
    for(i = 0; i < 8u; i++)
        crc = (crc & 0x8000u) ? ((crc << 1) ^ 0x1021u) : (crc << 1);
    return crc;
}


/* ─── HTTP 요청: 서버 버전 확인 ─────────────────────────────── */
static void requestVersion(void)
{
    char url[80];
    snprintf(url, sizeof(url), "%s/version.php", DEFAULT_OTA_API_PATH);
    wifi_cmd_ota_version(url);
    g_otaState = OTA_STATE_WAITING_VERSION;
}


/* ─── HTTP 요청: 다음 펌웨어 청크 ──────────────────────────── */
static void requestNextChunk(void)
{
    char url[120];
    snprintf(url, sizeof(url), "%s/firmware.php?offset=%lu&size=%u",
             DEFAULT_OTA_API_PATH, g_otaDownloadOffset, OTA_CHUNK_SIZE);
    wifi_cmd_ota_chunk(url);
    g_otaState = OTA_STATE_WAITING_CHUNK;
}


/* ─── 화면: 버전 비교 정보 표시 ─────────────────────────────── */
static void displayVersionInfo(void)
{
    uint16 y = GetBodyArea().top + 15;
    EraseBlankArea(GetBodyArea().top, FALSE);

    LCD_printf(15, y,    YELLOW, BLACK, SmallFont8x12, "Current :");
    LCD_printf(100,y,    WHITE,  BLACK, SmallFont8x12, PROJECT_FIRMWARE_VERSION);
    LCD_printf(15, y+22, YELLOW, BLACK, SmallFont8x12, "Latest  :");
    LCD_printf(100,y+22, GREEN,  BLACK, SmallFont8x12, g_otaLatestVersion);

    LCD_printf(15, y+50, LIGHTGREY, BLACK, SmallFont8x12,
        "Size: %lu bytes", g_otaFirmwareSize);

    SetDrawBottomButtons("QUIT", "UPDATE", BUTTON_STYLE_R_GREEN, BUTTON_STYLE_R_BLUE);
}


/* ─── 화면: 다운로드 진행 표시 ──────────────────────────────── */
static void displayProgress(void)
{
    uint16 y = GetBodyArea().top + 15;
    uint8  pct = 0;
    uint16 barW, maxBarW;

    if(g_otaFirmwareSize > 0)
        pct = (uint8)((g_otaDownloadOffset * 100ul) / g_otaFirmwareSize);

    EraseBlankArea(GetBodyArea().top, FALSE);
    LCD_printf(15, y,    WHITE,  BLACK, SmallFont8x12, "Downloading...");
    LCD_printf(15, y+22, WHITE,  BLACK, SmallFont8x12,
        "%lu / %lu bytes", g_otaDownloadOffset, g_otaFirmwareSize);

    /* 진행 바 */
    maxBarW = (uint16)(g_SCREEN_WIDTH - 30u);
    barW    = (uint16)((uint32)maxBarW * pct / 100u);
    DrawRectangle(15, y+48, g_SCREEN_WIDTH-15, y+62, DARKGREY);
    if(barW > 0)
        FillRectangle(15, y+48, 15+barW, y+62, GREEN);

    LCD_printf(15, y+70, WHITE, BLACK, SmallFont8x12, "%d%%", pct);
}


/* ─── 화면: 오류 표시 ────────────────────────────────────────── */
static void displayError(const char *msg)
{
    uint16 y = GetBodyArea().top + 30;
    EraseBlankArea(GetBodyArea().top, FALSE);
    LCD_printf(15, y,    RED,   BLACK, SmallFont8x12, "OTA Error:");
    LCD_printf(15, y+20, WHITE, BLACK, SmallFont8x12, msg);
    SetDrawBottomButtons("QUIT", NULL, BUTTON_STYLE_R_GREEN, BUTTON_STYLE_R_BLUE);
}


/* ─── OTA 플래그 → W25QXX Sector 30에 기록 ─────────────────── */
static void writeOtaFlag(void)
{
    OTA_FLAG_BLOCK flag;
    memset(&flag, 0xFF, sizeof(flag));
    memcpy(flag.magic, OTA_FLAG_MAGIC, 7u);
    flag.magic[7] = '\0';
    strncpy(flag.version, g_otaLatestVersion, 15u);
    flag.version[15] = '\0';
    flag.firmwareSize = g_otaFirmwareSize;
    flag.crc          = g_otaCRC;
    flag.status       = 0x01u; /* 업데이트 대기 */

    W25qxx_EraseSector(OTA_FLAG_SECTOR);
    W25qxx_WriteSector((uint8*)&flag, OTA_FLAG_SECTOR, 0u, sizeof(OTA_FLAG_BLOCK));
}


/* ─── 초기화 ─────────────────────────────────────────────────── */
void initOtaMenu(void)
{
    g_otaState          = OTA_STATE_IDLE;
    g_otaDownloadOffset = 0u;
    g_otaCRC            = 0u;
    memset(g_otaLatestVersion, 0, sizeof(g_otaLatestVersion));
}


/* ─── 메뉴 메인 함수 (manageMenu.c에서 등록) ────────────────── */
int doOtaUpdate(void *this, uint8 reflash)
{
    MENUNODE *thisMenu = (MENUNODE *)this;

    switch(reflash)
    {
        /* ── 화면 진입 ── */
        case TRUE:
        {
            initOtaMenu();

            SetDrawListButtons(&g_ListMenu, thisMenu->nodeName, NULL, 0, BUTTON_STYLE_LIST);
            EraseBlankArea(GetBodyArea().top, FALSE);

            if(!g_network.isConnectAP)
            {
                displayError("WiFi not connected");
                g_otaState = OTA_STATE_ERROR;
                break;
            }

            uint16 y = GetBodyArea().top + 30;
            LCD_printf(15, y, WHITE, BLACK, SmallFont8x12, "Checking server...");

            requestVersion();
        }
        break;

        /* ── 루프 처리 ── */
        case FALSE:
        {
            TOUCH tc = GetTouch();

            switch(g_otaState)
            {
                /* 버전 정보 표시 중 - 버튼 입력 대기 */
                case OTA_STATE_SHOW_INFO:
                {
                    if(tc.isClick == FALSE) break;
                    switch(getIndexOfClickedButton(&tc, g_btnBottom, 2))
                    {
                        case BOTTOM_LEFT:   /* QUIT */
                            g_otaState = OTA_STATE_IDLE;
                            return MENU_RETURN_PARENT;

                        case BOTTOM_RIGHT:  /* UPDATE */
                            g_otaState = OTA_STATE_DOWNLOADING;
                            displayProgress();
                            requestNextChunk();
                            break;
                    }
                }
                break;

                /* 이미 최신 / 오류 - QUIT만 */
                case OTA_STATE_UP_TO_DATE:
                case OTA_STATE_ERROR:
                {
                    if(tc.isClick == FALSE) break;
                    if(getIndexOfClickedButton(&tc, g_btnBottom, 2) == BOTTOM_LEFT)
                    {
                        g_otaState = OTA_STATE_IDLE;
                        return MENU_RETURN_PARENT;
                    }
                }
                break;

                /* 완료 → 잠시 후 재부팅 */
                case OTA_STATE_COMPLETE:
                {
                    CyDelay(2000u);
                    CySoftwareReset();
                }
                break;

                default: break;
            }
        }
        break;
    }
    return MENU_RETURN_THIS;
}


/* ─── WiFi 콜백: 버전 응답 처리 ─────────────────────────────── */
/* WIFI.c의 wifi_get_response()에서 WIFI_CMD_OTA_VERSION 케이스에서 호출 */
void otaHandleVersionResponse(char *json, uint16 size)
{
    jsmn_parser  p;
    jsmntok_t    tokens[12];
    int          r;
    int          i;

    jsmn_init(&p);
    r = jsmn_parse(&p, json, size, tokens, 12);

    if(r < 1)
    {
        displayError("Invalid JSON");
        g_otaState = OTA_STATE_ERROR;
        return;
    }

    /* JSON 파싱: {"version":"V2.0.1","size":30720,"crc":12345} */
    for(i = 1; i < r - 1; i++)
    {
        if(jsoneq(json, &tokens[i],"version") == 0)
        {
            int len = tokens[i+1].end - tokens[i+1].start;
            if(len > 15) len = 15;
            strncpy(g_otaLatestVersion, json + tokens[i+1].start, len);
            g_otaLatestVersion[len] = '\0';
        }
        else if(jsoneq(json, &tokens[i],"size") == 0)
        {
            g_otaFirmwareSize = (uint32)atol(json + tokens[i+1].start);
        }
        else if(jsoneq(json, &tokens[i],"crc") == 0)
        {
            g_otaCRC = (uint16)atoi(json + tokens[i+1].start);
        }
    }

    if(g_otaLatestVersion[0] == '\0' || g_otaFirmwareSize == 0u)
    {
        displayError("Bad version data");
        g_otaState = OTA_STATE_ERROR;
        return;
    }

    EraseBlankArea(GetBodyArea().top, FALSE);

    if(compareVersion(g_otaLatestVersion, PROJECT_FIRMWARE_VERSION) > 0)
    {
        /* 새 버전 존재 */
        g_otaState = OTA_STATE_SHOW_INFO;
        displayVersionInfo();
    }
    else
    {
        /* 이미 최신 */
        uint16 y = GetBodyArea().top + 25;
        LCD_printf(15, y,    GREEN, BLACK, SmallFont8x12, "Already latest!");
        LCD_printf(15, y+22, WHITE, BLACK, SmallFont8x12, "Ver: %s", PROJECT_FIRMWARE_VERSION);
        SetDrawBottomButtons("QUIT", NULL, BUTTON_STYLE_R_GREEN, BUTTON_STYLE_R_BLUE);
        g_otaState = OTA_STATE_UP_TO_DATE;
    }
}


/* ─── WiFi 콜백: 펌웨어 청크 처리 ──────────────────────────── */
/* WIFI.c의 wifi_get_response()에서 WIFI_CMD_OTA_CHUNK 케이스에서 호출  */
void otaHandleChunkResponse(char *json, uint16 size)
{
    jsmn_parser  p;
    jsmntok_t    tokens[8];
    int          r;
    int          i;
    char        *hexData  = NULL;
    uint16       hexLen   = 0;
    uint16       bytesCnt = 0;
    uint32       flashAddr;
    uint32       sectorOffset;
    uint32       sectorNum;
    uint16       j;

    jsmn_init(&p);
    r = jsmn_parse(&p, json, size, tokens, 8);

    if(r < 1)
    {
        displayError("Chunk JSON error");
        g_otaState = OTA_STATE_ERROR;
        return;
    }

    /* JSON 파싱: {"offset":0,"bytes":400,"hex":"AABB..."} */
    for(i = 1; i < r - 1; i++)
    {
        if(jsoneq(json, &tokens[i],"hex") == 0)
        {
            hexData = json + tokens[i+1].start;
            hexLen  = (uint16)(tokens[i+1].end - tokens[i+1].start);
            bytesCnt = hexLen / 2u;
        }
    }

    if(hexData == NULL || bytesCnt == 0u || bytesCnt > OTA_CHUNK_SIZE)
    {
        displayError("Invalid chunk data");
        g_otaState = OTA_STATE_ERROR;
        return;
    }

    /* HEX → 바이너리 변환 */
    hexDecode(hexData, g_chunkBuf, bytesCnt);

    /* CRC 누적 계산 */
    for(j = 0; j < bytesCnt; j++)
        g_otaCRC = crc16Update(g_otaCRC, g_chunkBuf[j]);  /* 누적 중에는 덮어쓰지 않음 */
    /* ※ g_otaCRC는 이 함수가 끝난 후 서버의 최종 CRC와 비교할 때 사용
     *   (현재는 서버 CRC로 검증, 계산 CRC는 부트로더에서 재검증) */

    /* W25QXX Flash에 기록 */
    /* 섹터 경계: 새 섹터 시작 시 먼저 Erase */
    sectorOffset = g_otaDownloadOffset % w25qxx.SectorSize;
    if(sectorOffset == 0u)
    {
        sectorNum = (uint32)OTA_FIRMWARE_SECTOR + (g_otaDownloadOffset / w25qxx.SectorSize);
        W25qxx_EraseSector(sectorNum);
    }

    flashAddr = (uint32)OTA_FIRMWARE_SECTOR * w25qxx.SectorSize + g_otaDownloadOffset;
    W25qxx_WritePage(g_chunkBuf,
                     flashAddr / w25qxx.PageSize,
                     (uint32)(flashAddr % w25qxx.PageSize),
                     bytesCnt);

    g_otaDownloadOffset += bytesCnt;

    /* 다운로드 완료 여부 확인 */
    if(g_otaDownloadOffset >= g_otaFirmwareSize)
    {
        /* OTA 플래그 기록 */
        writeOtaFlag();

        /* 상태 서버에 보고 */
        {
            char statusUrl[140];
            snprintf(statusUrl, sizeof(statusUrl),
                "%s/status.php?mac=%s&status=done&version=%s",
                DEFAULT_OTA_API_PATH, g_network.MAC, g_otaLatestVersion);
            wifi_cmd_http(statusUrl);
        }

        /* 완료 화면 */
        EraseBlankArea(GetBodyArea().top, FALSE);
        {
            uint16 y = GetBodyArea().top + 25;
            LCD_printf(15, y,    GREEN, BLACK, SmallFont8x12, "Download Complete!");
            LCD_printf(15, y+22, WHITE, BLACK, SmallFont8x12, "Applying update...");
            LCD_printf(15, y+44, LIGHTGREY,  BLACK, SmallFont8x12, "Device will restart.");
        }

        g_otaState = OTA_STATE_COMPLETE;
    }
    else
    {
        /* 다음 청크 요청 */
        displayProgress();
        requestNextChunk();
    }
}

/* ================================================================
 * 2-Stage 자동 OTA 버전 체크
 * ================================================================
 *
 * Stage 1: 부팅 후 20초 (ANDON 초기화 완료 대기) → 1회 자동 체크
 * Stage 2: 이후 24시간마다 반복 체크
 *
 * 결과:
 *   - 새 버전 있음 → g_otaUpdateAvailable = TRUE → 헤더 배지 표시
 *   - 이미 최신   → g_otaUpdateAvailable = FALSE
 *   - LCD 화면 변경 없음 (완전 조용한 백그라운드 동작)
 * ================================================================*/

/* ─── 자동 체크 초기화 (initAndon() 끝에서 호출) ──────────────
 * ANDON 초기화 큐(5개 요청)가 모두 처리되도록 20초 딜레이 후 체크  */
void otaAutoCheckInit(void)
{
    if(g_otaAutoTimerIdx == 0xFFu)
    {
        g_otaAutoTimerIdx = registerCounter_1ms(OTA_AUTO_INIT_DELAY_MS);
    }
    else
    {
        /* 재연결 시 초기 딜레이로 리셋 */
        setCountMax_1ms(g_otaAutoTimerIdx, OTA_AUTO_INIT_DELAY_MS);
        resetCounter_1ms(g_otaAutoTimerIdx);
    }
    g_otaAutoState = OTA_AUTO_WAIT_INIT;
    printf("[OTA AUTO] Scheduled initial check in %lu sec\r\n",
           OTA_AUTO_INIT_DELAY_MS / 1000ul);
}

/* ─── 자동 체크 루프 (wifiLoop() 에서 WiFi IDLE + ANDON 없을 때 호출) ─
 * 타이머 만료 시 HTTP 버전 요청 발송. 응답은 otaHandleAutoVersionResponse()로 */
void otaAutoCheckLoop(void)
{
    if(g_otaAutoTimerIdx == 0xFFu)    return;  /* 미초기화 */
    if(g_otaAutoState == OTA_AUTO_IDLE) return; /* 비활성 */
    if(g_otaAutoState == OTA_AUTO_REQUESTING) return; /* 응답 대기 중 */

    if(isFinishCounter_1ms(g_otaAutoTimerIdx))
    {
        char url[80];
        snprintf(url, sizeof(url), "%s/version.php", DEFAULT_OTA_API_PATH);
        wifi_cmd_ota_auto(url);
        g_otaAutoState = OTA_AUTO_REQUESTING;
        printf("[OTA AUTO] Checking version...\r\n");
    }
}

/* ─── 자동 버전 체크 응답 콜백 (WIFI.c → WIFI_CMD_OTA_AUTO 케이스에서 호출) ─
 * LCD를 건드리지 않고 g_otaUpdateAvailable 플래그만 설정                    */
void otaHandleAutoVersionResponse(char *json, uint16 size)
{
    jsmn_parser p;
    jsmntok_t   tokens[8];
    int         r, i;
    char        latestVer[16] = {0};

    jsmn_init(&p);
    r = jsmn_parse(&p, json, size, tokens, 8);

    if(r < 1)
    {
        printf("[OTA AUTO] Parse error\r\n");
        /* 다음 주기(24h)에 재시도 */
        setCountMax_1ms(g_otaAutoTimerIdx, OTA_AUTO_PERIOD_MS);
        resetCounter_1ms(g_otaAutoTimerIdx);
        g_otaAutoState = OTA_AUTO_WAIT_PERIODIC;
        return;
    }

    for(i = 1; i < r - 1; i++)
    {
        if(jsoneq(json, &tokens[i],"version") == 0)
        {
            int len = tokens[i+1].end - tokens[i+1].start;
            if(len > 15) len = 15;
            strncpy(latestVer, json + tokens[i+1].start, len);
            latestVer[len] = '\0';
        }
        else if(jsoneq(json, &tokens[i],"size") == 0)
        {
            g_otaFirmwareSize = (uint32)atol(json + tokens[i+1].start);
        }
    }

    if(latestVer[0] != '\0')
    {
        /* 버전 비교: 최신 버전이 현재보다 높으면 배지 활성화 */
        int8 cmp = compareVersion(latestVer, PROJECT_FIRMWARE_VERSION);
        if(cmp > 0)
        {
            strncpy(g_otaLatestVersion, latestVer, 15u);
            g_otaLatestVersion[15] = '\0';
            g_otaUpdateAvailable = TRUE;
            printf("[OTA AUTO] New version: %s (current: %s)\r\n",
                   g_otaLatestVersion, PROJECT_FIRMWARE_VERSION);
        }
        else
        {
            g_otaUpdateAvailable = FALSE;
            printf("[OTA AUTO] Up to date: %s\r\n", PROJECT_FIRMWARE_VERSION);
        }
    }

    /* 24시간 후 다음 체크 예약 */
    setCountMax_1ms(g_otaAutoTimerIdx, OTA_AUTO_PERIOD_MS);
    resetCounter_1ms(g_otaAutoTimerIdx);
    g_otaAutoState = OTA_AUTO_WAIT_PERIODIC;
}

/* ─── 헤더 OTA 배지 그리기 (DrawHeader() 에서 호출) ───────────
 * g_otaUpdateAvailable == TRUE 일 때만 헤더에 "▲UPD" 텍스트 표시 */
void otaDrawUpdateBadge(void)
{
    if(g_otaUpdateAvailable == FALSE) return;

    /* 헤더 영역 오른쪽: WiFi 아이콘 왼쪽 공간에 배치
     * DEFAULT_TOP_TITLE_HEIGHT = 30px (widget.h 참조)
     * x: 화면 중앙 약간 오른쪽, y: 헤더 상단              */
    uint16 x = g_SCREEN_WIDTH / 2u + 10u;
    uint16 y = 8u;

    /* 노란색 배경 사각형 */
    FillRectangle(x - 2u, y - 2u, x + 38u, y + 13u, ORANGE);

    /* "UPD" 텍스트 (SmallFont8x12: 8px wide, 12px tall) */
    LCD_DrawFont(x,      y, BLACK, ORANGE, SmallFont8x12, 'U');
    LCD_DrawFont(x + 8u, y, BLACK, ORANGE, SmallFont8x12, 'P');
    LCD_DrawFont(x +16u, y, BLACK, ORANGE, SmallFont8x12, 'D');
    LCD_DrawFont(x +24u, y, BLACK, ORANGE, SmallFont8x12, '!');
}

/* [] END OF FILE */
