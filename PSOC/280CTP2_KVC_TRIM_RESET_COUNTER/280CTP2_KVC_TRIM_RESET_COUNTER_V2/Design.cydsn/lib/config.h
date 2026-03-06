/* ========================================
 *
 * Copyright Suntech, 2023.04.10
 * All Rights Reserved
 * UNPUBLISHED, LICENSED SOFTWARE.
 *
 * CONFIDENTIAL AND PROPRIETARY INFORMATION
 * WHICH IS THE PROPERTY OF Suntech.
 *
 * ========================================
*/
#ifndef _CONFIG_H_
#define _CONFIG_H_
#include "main.h"   
    
#define MAX_WIFI_RECEIVE_BUFFER 2048    
    
typedef struct {
    char Server_URL[50];
    char Server_Path[50];
    
    char SSID[30];
    char password[30];
    uint16 port;
    
    uint16 reconnectTime;
    char deviceName[30];    
} CONFIG; // External flash에 저장될 Config 데이터

#endif    
/* [] END OF FILE */
