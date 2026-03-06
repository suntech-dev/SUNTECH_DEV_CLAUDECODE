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
/* [] END OF FILE */
#include "WIFI.h"
#include "andonApi.h"
#include "sysTick.h"
#include "widget.h"
#include "server.h"
#include "utility.h"
#include "LEDControl.h"

NETWORK      g_network;

uint8 g_WIFI_ReceiveBuffer[MAX_WIFI_RECEIVE_BUFFER+1];
uint16 g_size_WIFI_ReceiveBuffer=0;

ACCESS_POINT g_APs[MAX_NO_OF_ACCESS_POINT];
uint16 g_SizeOfAPs;

char *g_ptrHttpReceivedText=NULL;
uint16 g_sizeOfHttpText;

uint8 g_wifi_cmd = WIFI_CMD_IDLE;

uint8 g_index_WifiStrength;

#define WIFI_CMD(str) wifi_printf("%s\r\n",str)

void initWIFI()
{  
    //////////////////////////////////////////////////////////////
    memset(&g_network,0,sizeof(g_network));       
    g_network.RSSI = INT16_MIN;
    g_network.bWifiReady = FALSE;
    
    UART_WIFI_Start();    
    WIFI_EN_Write(1);
    CyDelay(20); // Minimum 10ms
    WIFI_RESET_Write(1);

    g_index_WifiStrength = registerCounter_1ms(5000);
}

void wifiLoop()
{
    /*static uint8 bReset = TRUE;
    
    if(g_network.RSSI == INT16_MIN || g_network.RSSI == 0)
    {
        g_bLED1_Flickering = TRUE;
    } else {
        g_bLED1_Flickering = FALSE;        
    }
    
    // AP에서 IP를 받았고, WIFI Module에 CMD처리가 없고, 안돈 cmd가 있다면 처리한다.
    if(g_network.isConnectAP && g_wifi_cmd == WIFI_CMD_IDLE)
    {
        if(andonLoop() == FALSE) // 안돈에서 처리할 것이 없다면
        {
            if(isFinishCounter_1ms(g_index_WifiStrength)) // Wifi Strength 체크를 5sec마다 수행한다.
            {
                resetCounter_1ms(g_index_WifiStrength);
                wifi_cmd(WIFI_CMD_RECEIVED_STRENGTH);               
            }
        }
    }
    
    if(wifi_receive_data())
    {    
        if(g_network.isConnectAP)  // IDLE
        {
            wifi_get_response();
            if(g_sizeOfHttpText > 0)
            {
                andonResponse(g_ptrHttpReceivedText,g_sizeOfHttpText);
                
                g_sizeOfHttpText = 0;
                g_ptrHttpReceivedText=NULL;
            }
        }
        else
        {
            wifiConnectAP(bReset);
        }
        
        clearWifiBuffer();
    }*/
}

#define STRSTR_WIFI_BUFFER(str) strstr((char*) g_WIFI_ReceiveBuffer,str)
    
// 할당된 IP를 찾는다.
uint8 wifiConnectAP()
{    
    int i=0;    
    static uint8 nLoop = 0;
        
    switch(nLoop)
    {
        case 0: // Wifi Ready Check
            if(STRSTR_WIFI_BUFFER("*ICT*DEVICEREADY") != NULL)            
            {
                printf("Ready Wifi\r\n");
                g_network.isConnectAP = FALSE;
                wifi_cmd(WIFI_CMD_GET_MAC); // request MAC                
                nLoop++;
            }
            break;
        case 1: // get Mac
            if(STRSTR_WIFI_BUFFER("*ICT*MAC:OK") != NULL)            
            {
                char *p = STRSTR_WIFI_BUFFER(" "); p++;
                while(*p != CR)  g_network.MAC[i++]  = *p++;                  
               // printNetworkInfo();   
                g_network.bWifiReady = TRUE;
            } else 
                printf("WIFI_RESP_GET_MAC ERROR\r\n");   
                              
            nLoop++;
            break;            
        case 2: // get IP
            if(STRSTR_WIFI_BUFFER("*ICT*IPALLOCATED:") != NULL)            
            {
                char *p = STRSTR_WIFI_BUFFER(":"); p++;
                
                while(*p != ' ') g_network.IPv4[i++]       = *p++; p++; i=0; // IPv4
                while(*p != ' ') g_network.SubnetMask[i++] = *p++; p++; i=0; // Subnet Mask                    
                while(*p != ' ') g_network.Gateway[i++]    = *p++; p++; i=0; // Gateway
                while(*p != CR)  g_network.DNSServer[i++]  = *p++; p++; i=0; // DNS Server                    
//                printNetworkInfo();                   

                nLoop++;
                WIFI_CMD("AT*ICT*MIB=0"); break;
            }
            break;
            case 3:
                if(STRSTR_WIFI_BUFFER("*ICT*MIB:OK") != NULL)
                {
                    char * p = STRSTR_WIFI_BUFFER(" "); p++;
                    strcpy(g_network.SSID,p);
                } else
                    printf("WIFI_CMD_RECEIVED_STRENGTH ERROR\r\n");
                           
                g_network.isConnectAP = TRUE;  
                
                wifi_cmd(WIFI_CMD_RECEIVED_STRENGTH);
                resetCounter_1ms(g_index_WifiStrength);
                
                initAndon();
                nLoop = 0;
                    
            break;

    }    
    return nLoop;
}

uint8 wifi_receive_data()
{
    uint8 c;   
    while(UART_WIFI_SpiUartGetRxBufferSize() > 0 )
    {
        if((c = UART_WIFI_UartGetChar()) > 0)
        {
            g_WIFI_ReceiveBuffer[g_size_WIFI_ReceiveBuffer++] = c;
            g_WIFI_ReceiveBuffer[g_size_WIFI_ReceiveBuffer] = 0;
            if(c == LF) return TRUE;
        }
    }
    return FALSE;
}

uint8 *getWifiBuffer() {return g_WIFI_ReceiveBuffer;};

void wifi_cmd(uint16 cmd)
{    
    switch(cmd)
    {
        case WIFI_CMD_GET_MAC:           WIFI_CMD("AT*ICT*MAC=?");      break;
        case WIFI_CMD_RECEIVED_STRENGTH: WIFI_CMD("AT*ICT*MIB=112");    break;
        case WIFI_CMD_AP_SCAN:           WIFI_CMD("AT*ICT*SCAN");       break;
        case WIFI_CMD_IPCONFIG:          WIFI_CMD("AT*ICT*IPCONFIG=1"); break; 
        case WIFI_CMD_CONNECT_AP:        WIFI_CMD("AT*ICT*IPCONFIG=1"); break; 
        case WIFI_CMD_FACTORY_RESET:     WIFI_CMD("AT*ICT*FACRESET=0"); break;         
    }
    g_wifi_cmd = cmd;
}

void wifi_cmd_http(char *url)
{ 
    wifi_printf("AT*ICT*HTTPGET=http://%s:%d%s%s\r\n",g_ptrServer->IP,g_ptrServer->port,g_ptrServer->path,getReplaceBlank(url));     
   // printf("AT*ICT*HTTPGET=http://%s:%d%s%s\r\n",g_ptrServer->IP,g_ptrServer->port,g_ptrServer->path,getReplaceBlank(url));    
         
    g_wifi_cmd = WIFI_CMD_HTTP;    
}

void wifi_get_response()
{
    int i=0;
    char *p = NULL;
    
    switch(g_wifi_cmd)
    {
        //////////////////////////////////////////////////////////////////////////////////
        // Get Mac Address ///////////////////////////////////////////////////////////////
        //////////////////////////////////////////////////////////////////////////////////       
        case WIFI_CMD_GET_MAC:
        {
            if(STRSTR_WIFI_BUFFER("*ICT*MAC:OK") != NULL)            
            {
                p = STRSTR_WIFI_BUFFER(" "); p++;
                while(*p != CR)  g_network.MAC[i++]  = *p++;   
                
                 //printNetworkInfo();                
            } else 
                printf("WIFI_RESP_GET_MAC ERROR\r\n");
            
            g_wifi_cmd = WIFI_CMD_IDLE;
        }
        break;   
        //////////////////////////////////////////////////////////////////////////////////
        // get Received signal strength indication ///////////////////////////////////////
        //////////////////////////////////////////////////////////////////////////////////        
        case WIFI_CMD_RECEIVED_STRENGTH:
            if(STRSTR_WIFI_BUFFER("*ICT*MIB:OK") != NULL)
            {
                p = STRSTR_WIFI_BUFFER(" "); p++;
                g_network.RSSI = atoi(p);
                DrawWifi();
            } else
                printf("WIFI_CMD_RECEIVED_STRENGTH ERROR\r\n");

            g_wifi_cmd = WIFI_CMD_IDLE;    
        break;
        //////////////////////////////////////////////////////////////////////////////////            
        // get AP Scan list //////////////////////////////////////////////////////////////
        //////////////////////////////////////////////////////////////////////////////////            
        case WIFI_CMD_AP_SCAN:
                printWifiBuffer();                
            if(STRSTR_WIFI_BUFFER("*ICT*SCAN:OK") != NULL)
            {
                g_SizeOfAPs = 0;
            }
            else if(STRSTR_WIFI_BUFFER("*ICT*SCANIND:") != NULL)
            {

                p = STRSTR_WIFI_BUFFER(" "); p++;
                appendAP(p);
            }
            else if(STRSTR_WIFI_BUFFER("*ICT*SCANRESULT") != NULL)
            {
                g_wifi_cmd = WIFI_CMD_IDLE;
            } else
                printf("WIFI_CMD_AP_SCAN ERROR\r\n");
        break;
        //////////////////////////////////////////////////////////////////////////////////            
        // Set IP Config /////////////////////////////////////////////////////////////////
        //////////////////////////////////////////////////////////////////////////////////   
        case WIFI_CMD_FACTORY_RESET:
            {
                printWifiBuffer();   
                if(STRSTR_WIFI_BUFFER("*ICT*FACRESET:OK") != NULL)
                {
                    wifi_cmd(WIFI_CMD_IPCONFIG);
                    break;
                }
            }
            break;
        //////////////////////////////////////////////////////////////////////////////////            
        // Set IP Config /////////////////////////////////////////////////////////////////
        //////////////////////////////////////////////////////////////////////////////////            
        case WIFI_CMD_IPCONFIG:
                printWifiBuffer();                
            if(STRSTR_WIFI_BUFFER("*ICT*IPCONFIG:OK") != NULL)
            {
                wifi_printf("AT*ICT*SCONN=%s %s\r\n",g_ptrServer->SSID, g_ptrServer->password);
                printf("AT*ICT*SCONN=%s %s\r\n",g_ptrServer->SSID, g_ptrServer->password);                
            }
            else if(STRSTR_WIFI_BUFFER("*ICT*SCONN:OK") != NULL)
            {
                WIFI_RESET_Write(0);                
                CyDelay(20);
                WIFI_RESET_Write(1);   
                CyDelay(20);            
               CySoftwareReset();
            }               
            else if(STRSTR_WIFI_BUFFER("*ICT*IPRELEASED") != NULL)
            {
            }            
            else if(STRSTR_WIFI_BUFFER("*ICT*SSL_CLOSED:OK") != NULL)
            {
            }
            else if(STRSTR_WIFI_BUFFER("*ICT*DISASSOCIATED") != NULL)
            {
            }    
            else if(STRSTR_WIFI_BUFFER("*ICT*ASSOCIATED:0") != NULL)
            {
            }               
            else if(STRSTR_WIFI_BUFFER("*ICT*IPALLOCATED:") != NULL)
            {
                g_wifi_cmd = WIFI_CMD_IDLE;
            }  
            else
            {
            //    ShowMessage("ACCESS FAILUE");
                printf("WIFI_CMD_IPCONFIG ERROR\r\n");
            }
        break;                
        //////////////////////////////////////////////////////////////////////////////////            
        // request HTTP //////////////////////////////////////////////////////////////////
        //////////////////////////////////////////////////////////////////////////////////            
        case WIFI_CMD_HTTP:
            if     (STRSTR_WIFI_BUFFER("*ICT*HTTPGET:OK") != NULL)
            {
            }
            else if(STRSTR_WIFI_BUFFER("*ICT*HTTPGET:ERROR") != NULL)
            {
                printf("ICT*HTTPGET:ERROR\r\n");
                g_wifi_cmd = WIFI_CMD_IDLE;
            }
            else if(STRSTR_WIFI_BUFFER("*ICT*HTTPBODY:") != NULL)
            {
              
                p = STRSTR_WIFI_BUFFER(":"); p++;
                g_sizeOfHttpText = atoi(p); // No. of Received Data
            
                p = STRSTR_WIFI_BUFFER(" "); p++;

                g_ptrHttpReceivedText=p;    // pointer of Received Data
            }
            else if(STRSTR_WIFI_BUFFER("*ICT*HTTPCLOSE:OK") != NULL)
            {
                g_wifi_cmd = WIFI_CMD_IDLE;     
            }            
    }
}

void wifi_printf(const char *fmt, ...)
{
    va_list ap;
    char buff[1024];
    
    va_start(ap, fmt);

    vsprintf(buff, fmt, ap);
    va_end(ap);
    
    uint16 nLength = strlen(buff);
    for(int i=0; i < nLength; i++)
    {
        while(UART_WIFI_SpiUartGetTxBufferSize());
        UART_WIFI_UartPutChar(buff[i]);
    }
}

void clearWifiBuffer() { g_size_WIFI_ReceiveBuffer = 0; }


void printWifiBuffer()
{
    if(g_size_WIFI_ReceiveBuffer >0 ) printf("%s\r\n",g_WIFI_ReceiveBuffer);
}

void printNetworkInfo()
{                                
    printf("IPv4        : %s\r\n",g_network.IPv4);
    printf("Subnet Mask : %s\r\n",g_network.SubnetMask);
    printf("Gateway     : %s\r\n",g_network.Gateway);
    printf("DNS Server  : %s\r\n",g_network.DNSServer);
    printf("MAC Address : %s\r\n",g_network.MAC); 
}

void appendAP(char *str)
{
    char *ptr = str;
    
    if(ptr[0] == ' ') return; // SSID가 없는 경우가 있다.
 
    if(g_SizeOfAPs > MAX_NO_OF_ACCESS_POINT - 2) return;
    
    int i=0;
    ACCESS_POINT *ptrAP = &g_APs[g_SizeOfAPs++];

    // get SSID /////////////////////////////////
    while(*ptr != ' ') ptrAP->SSID[i++] = *ptr++;
    ptrAP->SSID[i] = 0;
    ptrAP->SSID[MAX_STRING_SSID-1] = 0;
 
    // get MAC Address /////////////////////////
    while(*ptr != ':') ptr++; // ':'을 먼저 찾고 앞으로 2자 이동한다.
    ptr--;
    ptr--;    
 
    i=0;   
    while(*ptr != ' ') ptrAP->MAC[i++] = *ptr++;
    ptrAP->MAC[i] = 0;
    
    // Skip 
    ptr++;    
    while(*ptr != ' ') ptr++;
    // Skip  
    ptr++;    
    while(*ptr != ' ') ptr++;
    // Skip     
    ptr++;    
    while(*ptr != ' ') ptr++;
 
    // get MAC Address /////////////////////////    
    ptr++;
    ptrAP->RSSI = atoi(ptr);    
    
//    printf(">%s %s %d\r\n",ptrAP->SSID, ptrAP->MAC, ptrAP->RSSI);
}