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

#ifndef _SERVER_H_
#define _SERVER_H_
    
#define DEFAULT_SERVER_IP    "49.247.26.228"
#define DEFAULT_SERVER_PATH  "csg"    
#define DEFAULT_SERVER_PORT  80
#define DEFAULT_DEVICE_NAME  "IoT Device"    

typedef struct {
    char         IP[16];
    char path      [32];
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
