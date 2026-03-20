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
#define OTA_AUTO_INIT_DELAY_MS  20000ul  /* 부팅 후 최초 체크 딜레이: 20초 (ANDON 초기화 완료 대기) */
#define OTA_AUTO_PERIOD_MS      86400000ul /* 24시간마다 자동 체크 (24*60*60*1000) */

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
