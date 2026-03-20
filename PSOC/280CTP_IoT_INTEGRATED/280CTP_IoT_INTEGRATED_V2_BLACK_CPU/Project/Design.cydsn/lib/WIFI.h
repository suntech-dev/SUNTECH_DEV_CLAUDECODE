
#ifndef _WIFI_LOOP_H_
#define _WIFI_LOOP_H_
#include "main.h"   
    
#define MAX_WIFI_RECEIVE_BUFFER 2048    // 2026-03-09: 1024→2048 (최대 JSON 응답 ~1,087 bytes 대응)
#define MAX_STRING_SSID  40    
//#define DEFUALT_SERVER_IP "192.168.38.72" // 업체
//#define DEFUALT_SERVER_IP "115.68.227.31" // SUNTECH


/* NETWORK */
typedef struct {
    uint8 isConnectAP;
    int16 RSSI;
    
    char SSID[MAX_STRING_SSID];
    
    char IPv4[16];
    char SubnetMask[16];
    char Gateway[16];
    char DNSServer[16];
    char MAC[18];
 
    uint8 bWifiReady;
} NETWORK; // wifi module에서 받는 정보

extern NETWORK g_network;


/* Access Point */
typedef struct {
    int16 RSSI;
    char SSID[MAX_STRING_SSID];
    char MAC[18];
} ACCESS_POINT;

#define MAX_NO_OF_ACCESS_POINT 10  // 최적화: 40 → 10 (SRAM 1,800 bytes 절감)
extern ACCESS_POINT g_APs[MAX_NO_OF_ACCESS_POINT];
extern uint16 g_SizeOfAPs;
extern uint8 g_wifi_cmd;

void appendAP(char *str);

void initWIFI();    
void wifiLoop();    

void wifi_printf(const char *fmt, ...);
uint8 wifi_receive_data();
uint8 wifiConnectAP();
void printNetworkInfo();
void printWifiBuffer();
void clearWifiBuffer();


/* WIFI CMD */
void wifi_cmd(uint16 cmd);
void wifi_cmd_http(char *url);
void wifi_cmd_ota_version(char *url);   /* OTA 수동 메뉴: 버전 체크 HTTP GET */
void wifi_cmd_ota_chunk  (char *url);   /* OTA 수동 메뉴: 청크 다운로드 HTTP GET */
void wifi_cmd_ota_auto   (char *url);   /* OTA 자동 체크: 백그라운드 버전 확인 */

void wifi_get_response();

enum WIFI_CMD_ENUM {
//    WIFI_READY =0,
//    WIFI_IP_ALLOCATED,
    WIFI_CMD_IDLE = 0,
    WIFI_CMD_GET_MAC,
    WIFI_CMD_RECEIVED_STRENGTH,
    WIFI_CMD_AP_SCAN,
    WIFI_CMD_IPCONFIG,
    WIFI_CMD_HTTP,
    WIFI_CMD_CONNECT_AP,
    WIFI_CMD_FACTORY_RESET,
    WIFI_CMD_NETWORK_STATUS,
    WIFI_CMD_OTA_VERSION,   /* OTA 수동: 서버 버전 확인 (메뉴 진입 시) */
    WIFI_CMD_OTA_CHUNK,     /* OTA 수동: 펌웨어 청크 수신 */
    WIFI_CMD_OTA_AUTO,      /* OTA 자동: 백그라운드 버전 확인 (20s/24h 주기) */
};

char * STRSTR_WIFI_BUFFER(char *str);

#define WIFI_STRENGTH_CHECK_TIME 60000
#endif
/* [] END OF FILE */