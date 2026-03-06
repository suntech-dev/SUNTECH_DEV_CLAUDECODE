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
    memset(g_ptrServer,0,sizeof(SERVER_INFO));
        
    strcpy(g_ptrServer->IP,DEFAULT_SERVER_IP);    
    g_ptrServer->port = DEFAULT_SERVER_PORT;
    
    strcpy(g_ptrServer->deviceName,DEFAULT_DEVICE_NAME);
}

void ValidationConfigServer()
{
    if(strlen(g_ptrServer->IP) == 0)
    {
        SetDefaultConfigServer();
    }
}

void printConfig()
{
    if(g_ptrServer->port == 0) g_ptrServer->port = 80;
 
    printf("\r\nConfig :\r\n");
    printf("\t SSID       : %s\r\n",g_ptrServer->SSID);
    printf("\t Password   : %s\r\n",g_ptrServer->password);
    printf("\t Server IP  : %s\r\n",g_ptrServer->IP);
    printf("\t Server path: %s\r\n",g_ptrServer->path);    
    printf("\t Server Port: %d\r\n",g_ptrServer->port);
    printf("\t Device Name: %s\r\n",g_ptrServer->deviceName);      
}
/* [] END OF FILE */
