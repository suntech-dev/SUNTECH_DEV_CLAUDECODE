/* ========================================
 *
 * Copyright Suntech, 2023.03.30
 * All Rights Reserved
 * UNPUBLISHED, LICENSED SOFTWARE.
 *
 * CONFIDENTIAL AND PROPRIETARY INFORMATION
 * WHICH IS THE PROPERTY OF Suntech.
 *
 * ========================================
*/
#include "server.h"
#include "externalFlash.h"

SERVER_INFO *g_ptrServer;

void initServer()
{
    g_ptrServer = (SERVER_INFO *) & getExternalConfigData()[CONFIG_DATA_CONFIG];
}

void SetDefaultConfigServer()
{
    memset(g_ptrServer, 0, sizeof(SERVER_INFO));

    /* 하드코딩 값으로 초기화 (외부 플래시 완전 실패 시 사용) */
    strcpy(g_ptrServer->host,       DEFAULT_SERVER_HOST);
    g_ptrServer->port             = DEFAULT_SERVER_PORT ? DEFAULT_SERVER_PORT : 80;
    strcpy(g_ptrServer->SSID,       DEFAULT_SSID);
    strcpy(g_ptrServer->password,   DEFAULT_PASSWORD);
    strcpy(g_ptrServer->deviceName, DEFAULT_DEVICE_NAME);
}

void ValidationConfigServer()
{
    /*
     * 우선순위: 하드코딩 현장 고정값 > 외부 플래시 저장값
     * DEFAULT_* 가 비어있으면("")  → 외부 플래시 값 그대로 사용
     * DEFAULT_* 가 비어있지 않으면 → 하드코딩 값으로 덮어쓰기
     */
    if(strlen(DEFAULT_SERVER_HOST) > 0)  strcpy(g_ptrServer->host,       DEFAULT_SERVER_HOST);
    if(DEFAULT_SERVER_PORT > 0)          g_ptrServer->port             = DEFAULT_SERVER_PORT;
    if(strlen(DEFAULT_SSID) > 0)         strcpy(g_ptrServer->SSID,       DEFAULT_SSID);
    if(strlen(DEFAULT_PASSWORD) > 0)     strcpy(g_ptrServer->password,   DEFAULT_PASSWORD);
    if(strlen(DEFAULT_DEVICE_NAME) > 0)  strcpy(g_ptrServer->deviceName, DEFAULT_DEVICE_NAME);

    /* 최종 폴백: host가 비어있으면 외부 플래시도 비어있음 → 하드코딩 전체 적용 */
    if(strlen(g_ptrServer->host) == 0)   SetDefaultConfigServer();

    /* 포트 최소값 보장 */
    if(g_ptrServer->port == 0)           g_ptrServer->port = 80;
}

void printConfig()
{
    if(g_ptrServer->port == 0) g_ptrServer->port = 80;

    printf("\r\nConfig :\r\n");
    printf("\t SSID       : %s\r\n", g_ptrServer->SSID);
    printf("\t Password   : %s\r\n", g_ptrServer->password);
    printf("\t Server Host: %s\r\n", g_ptrServer->host);
    printf("\t Server Port: %d\r\n", g_ptrServer->port);
    printf("\t Device Name: %s\r\n", g_ptrServer->deviceName);
}
/* [] END OF FILE */
