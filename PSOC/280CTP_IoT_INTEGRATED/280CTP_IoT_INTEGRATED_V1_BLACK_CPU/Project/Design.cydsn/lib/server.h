
#ifndef _SERVER_H_
#define _SERVER_H_

/*
 * 현장 고정값 (하드코딩 우선 적용)
 * ─────────────────────────────────────────────────────────────
 * 값이 비어있지 않으면 → 외부 플래시 값을 덮어쓰고 하드코딩 우선 사용
 * 값이 ""(빈 문자열)이면 → 외부 플래시 저장값을 그대로 사용
 * ─────────────────────────────────────────────────────────────
 */
//#define DEFAULT_SERVER_HOST  "49.247.26.228/CTP280_API"  /* SUNTECH TEST. IP+경로 통합. 비우면 외부 플래시 사용: "" */
//#define DEFAULT_SERVER_HOST  "49.247.26.228/OEE_SCI/OEE_SCI_V2"  /* SUNTECH TEST. IP+경로 통합. 비우면 외부 플래시 사용: "" */
#define DEFAULT_SERVER_HOST  "192.168.38.72/OEE_SCI/OEE_SCI_V2"  /* SCI. IP+경로 통합. 비우면 외부 플래시 사용: "" */
#define DEFAULT_SERVER_PORT  80                             /* 0이면 외부 플래시 사용 */
#define DEFAULT_DEVICE_NAME  "SUNTECH IoT"                 /* 비우면 외부 플래시 사용: "" */

//#define DEFAULT_SSID         "SUNTECH-CORING"                /* SUNTECH. 비워두면 외부 플래시(USB 설정 도구) 사용 */
//#define DEFAULT_PASSWORD     "12345678"                /* SUNTECH. 비워두면 외부 플래시(USB 설정 도구) 사용 */
#define DEFAULT_SSID         "iSCi"                /* SCI. 비워두면 외부 플래시(USB 설정 도구) 사용 */
#define DEFAULT_PASSWORD     "iotsci1234"                /* SCI. 비워두면 외부 플래시(USB 설정 도구) 사용 */

/* API 엔드포인트 — andonApi.h에서 이동하여 한 곳에서 관리 */
#define DEFAULT_API_ENDPOINT "/api/sewing.php"

typedef struct {
    char        host[50];       /* IP/도메인 + 경로 (예: "192.168.38.72/2025/sci/new") */
    unsigned short port;
    char       SSID[40];
    char   password[20];    
    char deviceName[20];
    
    unsigned short deviceIndex;
} SERVER_INFO;

void initServer();    
void SetDefaultConfigServer();
void ValidationConfigServer();
void printConfig();

extern SERVER_INFO *g_ptrServer;

#endif
/* [] END OF FILE */