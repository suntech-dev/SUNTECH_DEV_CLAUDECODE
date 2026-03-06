/* ========================================
 *
 * Copyright YOUR COMPANY, THE YEAR
 * All Rights Reserved
 * UNPUBLISHED, LICENSED SOFTWARE.
 *
 * CONFIDENTIAL AND PROPRIETARY INFORMATION
 * WHICH IS THE PROPERTY OF your company.
 *
 * ========================================
*/
#ifndef _WIFI_LOOP_H_
#define _WIFI_LOOP_H_
#include "main.h"   
    
//#define USE_WIFI    
    
#define MAX_WIFI_RECEIVE_BUFFER 2048    
#define MAX_STRING_SSID  40    
//#define DEFUALT_SERVER_IP "192.168.38.72" // 업체
//#define DEFUALT_SERVER_IP "115.68.227.31" // SUNTECH



/////////////////////////////////////////////////////////
//////////// NETWORK ////////////////////////////////////
/////////////////////////////////////////////////////////
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

/////////////////////////////////////////////////////////
//////////// Access Point ///////////////////////////////
/////////////////////////////////////////////////////////
typedef struct {
    int16 RSSI;
    char SSID[MAX_STRING_SSID];
    char MAC[18];
} ACCESS_POINT;

#define MAX_NO_OF_ACCESS_POINT 40
extern ACCESS_POINT g_APs[MAX_NO_OF_ACCESS_POINT];
extern uint16 g_SizeOfAPs;
extern uint8 g_wifi_cmd;

void appendAP(char *str);
/////////////////////////////////////////////////////////

void initWIFI();    
void wifiLoop();    

void wifi_printf(const char *fmt, ...);
uint8 wifi_receive_data();
uint8 wifiConnectAP();
void printNetworkInfo();
void printWifiBuffer();
void clearWifiBuffer();

/////////////////////////////////////////////////////////
//////////// WIFI CMD ///////////////////////////////////
/////////////////////////////////////////////////////////
void wifi_cmd(uint16 cmd);
void wifi_cmd_http(char *url);

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
};


#endif    
/* [] END OF FILE */
