/* ========================================
 *
 * Copyright Suntech, 2023.02.01
 * All Rights Reserved
 * UNPUBLISHED, LICENSED SOFTWARE.
 *
 * CONFIDENTIAL AND PROPRIETARY INFORMATION
 * WHICH IS THE PROPERTY OF Suntech.
 *
 * ========================================
*/
#include "USBJsonConfig.h"
#include "jsonUtil.h"
#include "lib/utility.h"
#include "lib/widget.h"
#include "lib/WIFI.h"
#include "lib/server.h"
#include "lib/externalFlash.h"
#include "lib/menu.h"

static char g_usbCmd[100];         /* USBJsonConfig.c 내부 전용 */
uint8 JSON_Config(char *buffer, int16 size);

//uint8 g_USBRequest = REQ_NONE;

static CONFIG_META g_ConfigMeta;   /* USBJsonConfig.c 내부 전용 */

void USB_CommandProcess();

void initUSBJsonParsor()
{
    initUSB();
}

void usbJsonParsorLoop()
{
    usbLoop();
    
    if(g_size_USB_ReceiveBuffer == 0) return;
            
    if(JSON_Config((char *) getUSB_ReceiveBuffer(), g_size_USB_ReceiveBuffer) == TRUE) g_size_USB_ReceiveBuffer = 0; 
    
    USB_CommandProcess();
}

void USB_CommandProcess()
{
    int nLoop=0;

    switch(g_ConfigMeta.uRequestType)
    {
        case REQ_SYSTEM_CMD:
        {
            if(g_network.bWifiReady != TRUE) return;            
                        
            //////////////////////////////////
            if(strcmp(g_usbCmd,"connect")==0)
            //////////////////////////////////            
            {
                ShowMessage("AP SCANNING..");

                clearWifiBuffer();
                wifi_cmd(WIFI_CMD_AP_SCAN);
                while(g_wifi_cmd != WIFI_CMD_IDLE)
                {
                    usbLoop();
                    if(wifi_receive_data()==TRUE)
                    {
                        wifi_get_response();
                        clearWifiBuffer();
                    }
                }
                ShowMessage("SCANED %dth",g_SizeOfAPs);
                SendConfigData();
                break;
            }
            //////////////////////////////////
            if(strcmp(g_usbCmd,"reboot")==0)
            ////////////////////////////////// 
            {
                //Bootloadable_Load();
                CySoftwareReset();
                break;
            }
            //////////////////////////////////
            if(strcmp(g_usbCmd,"disconnect")==0)
            ////////////////////////////////// 
            {
                reflashMenu();
                break;            
            }       
            //////////////////////////////////
            if(strcmp(g_usbCmd,"firmwareUpgrade")==0)
            ////////////////////////////////// 
            {
                #ifdef BOOT_LOADERBLE_EXIST                                    
                    printf("Firmware Upgrade\r\n");
                    CyDelay(100);
                    Bootloadable_Load();
                    CySoftwareReset();
                #endif
                break;                   
            }            
        }
        case REQ_READ_CONFIG:
        {
            printf("Read Confdig\r\n");
            printConfig();
            SendConfigData();
        }
        break;
        case REQ_WRITE_CONFIG:
        {
            unsigned int len;
            removeWhiteSpace(g_ConfigMeta.url);   // remove white space
            removeDoubleSlash(g_ConfigMeta.url);  // remove double slash('/')
            len = strlen(g_ConfigMeta.url);
            if(len > 0 && g_ConfigMeta.url[len-1] == '/') g_ConfigMeta.url[len-1] = '\0'; // remove last slash

            /* IP+경로 통합 저장 (예: "192.168.38.72/2025/sci/new") */
            strncpy(g_ptrServer->host, g_ConfigMeta.url, sizeof(g_ptrServer->host) - 1);
            g_ptrServer->host[sizeof(g_ptrServer->host) - 1] = '\0';
            
            if(g_ptrServer->port == 0) g_ptrServer->port = 80;            
            
            printConfig();
            SendConfigData();
            ShowMessage("Saving..");            
            SaveExternalFlashConfig();
            
            reflashMenu();
    
            if(g_ConfigMeta.SSIDIndex != 0xFF) // SSID를 변경 했다면 WIFI모듈에 SSID 정보를 넣는다.
            {
                ShowMessage("Setting SSID");
                clearWifiBuffer();
                wifi_cmd(WIFI_CMD_FACTORY_RESET);

                while(g_wifi_cmd != WIFI_CMD_IDLE)
                {
                    usbLoop();
                    if(wifi_receive_data()==TRUE)
                    {
                        wifi_get_response();
                        clearWifiBuffer();
                    }
                }
                ShowMessage("SSID Done..");    
                CySoftwareReset();
            }                                
        }
        break;        
    }
    
    g_ConfigMeta.uRequestType = REQ_NONE;
}

void SendConfigData()
{
    printf_USB("{\"response\":\"config\",");
    
    printf_USB("\"SSID\":\"%s\",",g_ptrServer->SSID);
      
    int ssid_index = -1;   
    printf_USB("\"ssidList\":[");
    for(int i=0; i< g_SizeOfAPs; i++) 
    {
        int quality = 2 * (g_APs[i].RSSI + 100);
        if(i < g_SizeOfAPs-1) printf_USB("\"%.*s(%d%%)\",",40,g_APs[i].SSID, quality);
        else                  printf_USB("\"%.*s(%d%%)\"],",40,g_APs[i].SSID, quality);
        
        if(strcmp(g_ptrServer->SSID,g_APs[i].SSID) == 0) ssid_index = i;
    }
    printf_USB("\"SSID_Index\":\"%d\",",ssid_index);    
    printf_USB("\"password\":\"%s\",",g_ptrServer->password);
    printf_USB("\"serverIP\":\"%s\",",g_ptrServer->host);
    printf_USB("\"serverPort\":\"%d\",",g_ptrServer->port);
    printf_USB("\"deviceName\":\"%s\"}",g_ptrServer->deviceName);   
    
}

uint8 JSON_Config(char *buffer, int16 size)
{
    int i;
    int r;
    jsmn_parser p;
    jsmntok_t t[80]; /* 최대 토큰 수: 80 (10개 항목 JSON = 73 tokens, 여유 7) */
    
    jsmn_init(&p);
    r = jsmn_parse(&p, buffer, size, t, sizeof(t) / sizeof(t[0]));
    
    if (r < 0) {
    //    DEBUG_printf("Failed to parse JSON: %d\r\n", r);
        return FALSE;
    }
    
    /* Assume the top-level element is an object */
    if (r < 1 || t[0].type != JSMN_OBJECT) {
    //    DEBUG_printf("Object expected type: %d\r\n", t[0].type);
        return FALSE;
    }
    
    char buff[100];

    g_ConfigMeta.uRequestType = 0;
    
    g_ConfigMeta.SSIDIndex = 0xFF;
                                           
    for (i = 1; i < r; i++)
    {   
        if     (jsoneq(buffer, &t[i], "request") == 0)
        {
            sprintf(buff,"%.*s",t[i + 1].end - t[i + 1].start,buffer + t[i + 1].start);

            // AT Command ///////////////////////
            if(strcmp(buff,"sysCmd") == 0)           g_ConfigMeta.uRequestType = REQ_SYSTEM_CMD;            
            // AT Command ///////////////////////
            else if(strcmp(buff,"atCmd") == 0)       g_ConfigMeta.uRequestType = REQ_AT_CMD;
            // Read Config ///////////////////////
            else if(strcmp(buff,"readConfig") == 0)  g_ConfigMeta.uRequestType = REQ_READ_CONFIG;
            // Write Config ///////////////////////
            else if(strcmp(buff,"writeConfig") == 0) g_ConfigMeta.uRequestType = REQ_WRITE_CONFIG;
            
            continue;
        }
        /////////////////////////// REQ_SYSTEM_CMD //////////////////////////////
        else if(g_ConfigMeta.uRequestType == REQ_SYSTEM_CMD)
        {
            if     (jsoneq(buffer, &t[i], "cmd") == 0)
            {
                sprintf(g_usbCmd,"%.*s",t[i + 1].end - t[i + 1].start,buffer + t[i + 1].start);
                if(strcmp(g_usbCmd,"connect") == 0) printf_USB("{\"response\":\"connected\"}");

            }
        }
        else if(g_ConfigMeta.uRequestType == REQ_WRITE_CONFIG)
        {
                if     (jsoneq(buffer, &t[i], "SSIDIndex") == 0)
            {
                sprintf(buff,"%.*s",t[i + 1].end - t[i + 1].start,buffer + t[i + 1].start);
                g_ConfigMeta.SSIDIndex = atoi(buff);                
                strcpy(g_ptrServer->SSID,g_APs[g_ConfigMeta.SSIDIndex].SSID);                
            } 
            else if     (jsoneq(buffer, &t[i], "withMAC") == 0)
            {
                sprintf(buff,"%.*s",t[i + 1].end - t[i + 1].start,buffer + t[i + 1].start); 
                g_ConfigMeta.bWithMAC = atoi(buff);                   
            }
            else if     (jsoneq(buffer, &t[i], "password") == 0)
            {
                sprintf(g_ptrServer->password,"%.*s",t[i + 1].end - t[i + 1].start,buffer + t[i + 1].start);                
            }            
            else if     (jsoneq(buffer, &t[i], "serverIP") == 0)
            {
                sprintf(g_ConfigMeta.url,"%.*s",t[i + 1].end - t[i + 1].start,buffer + t[i + 1].start);                
            }              
            else if     (jsoneq(buffer, &t[i], "serverPort") == 0)
            {
                sprintf(buff,"%.*s",t[i + 1].end - t[i + 1].start,buffer + t[i + 1].start);    
                g_ptrServer->port = atoi(buff);
            }     
            else if     (jsoneq(buffer, &t[i], "deviceName") == 0)
            {
                sprintf(g_ptrServer->deviceName,"%.*s",t[i + 1].end - t[i + 1].start,buffer + t[i + 1].start);                
            }            
        }
    }
    
    return TRUE;
}
/* [] END OF FILE */
