
#include "WIFI.h"
#include "andonApi.h"
#include "sysTick.h"
#include "widget.h"
#include "server.h"
#include "utility.h"
#include "LEDControl.h"
#include "../otaMenu.h"   /* OTA 응답 콜백 */


NETWORK  g_network;

uint8 g_WIFI_ReceiveBuffer[MAX_WIFI_RECEIVE_BUFFER+1];
uint16 g_size_WIFI_ReceiveBuffer=0;

ACCESS_POINT g_APs[MAX_NO_OF_ACCESS_POINT];
uint16 g_SizeOfAPs;

char *g_ptrHttpReceivedText=NULL;
uint16 g_sizeOfHttpText;

uint8 g_wifi_cmd = WIFI_CMD_IDLE;

uint8 g_index_WifiStrength;
uint8 g_index_Wifi_Test;
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

    g_index_WifiStrength = registerCounter_1ms(WIFI_STRENGTH_CHECK_TIME);
    g_index_Wifi_Test    = registerCounter_1ms(5000);
}

void wifiLoop()
{
   // static uint8 bReset = TRUE;
    
    if(g_network.RSSI == INT16_MIN || g_network.RSSI == 0)
    {
        ///g_bLED1_Flickering = TRUE;
        g_bLED1_Flickering = FALSE;
    } else {
        
        g_bLED1_Flickering = FALSE;        
    }
    
    // AP에서 IP를 받았고, WIFI Module에 CMD처리가 없고, 안돈 cmd가 있다면 처리한다.
    /* 비-HTTP 명령 타임아웃 감시 (2026-03-19 수정)
     * OTA 명령(VERSION/AUTO/CHUNK)은 핸들러를 직접 호출하여 상태 머신을 정상 복구
     * 기타 명령은 기존처럼 IDLE로만 복구 */
    if(g_wifi_cmd != WIFI_CMD_IDLE && g_wifi_cmd != WIFI_CMD_HTTP)
    {
        if(isFinishCounter_1ms(g_index_Wifi_Test))
        {
            printf("[WIFI] CMD %d timeout -> IDLE\r\n", g_wifi_cmd);
            if(g_wifi_cmd == WIFI_CMD_OTA_VERSION)
            {
                /* OTA 수동 메뉴: 타임아웃 시 오류 화면 표시 후 복구 */
                g_wifi_cmd = WIFI_CMD_IDLE;
                otaHandleVersionResponse("{}", 2u);
            }
            else if(g_wifi_cmd == WIFI_CMD_OTA_AUTO)
            {
                /* OTA 자동 체크: 타임아웃 시 24시간 후 재시도 예약 */
                g_wifi_cmd = WIFI_CMD_IDLE;
                otaHandleAutoVersionResponse("{}", 2u);
            }
            else if(g_wifi_cmd == WIFI_CMD_OTA_CHUNK)
            {
                /* OTA 청크 다운로드: 타임아웃 시 오류 처리 */
                g_wifi_cmd = WIFI_CMD_IDLE;
                otaHandleChunkResponse("{}", 2u);
            }
            else
            {
                /* [수정 1] MIB 타임아웃 시 WifiStrength 타이머 리셋 (2026-03-10)
                 * 문제: 타임아웃 후 즉시 다음 루프에서 MIB 재전송 → CMD 2 timeout 반복 발생
                 * 해결: MIB 명령 타임아웃 시 WifiStrength 타이머도 리셋하여 다음 체크를 60초 후로 연기 */
                if(g_wifi_cmd == WIFI_CMD_RECEIVED_STRENGTH)
                    resetCounter_1ms(g_index_WifiStrength);
                g_wifi_cmd = WIFI_CMD_IDLE;
            }
        }
    }

    if(g_network.isConnectAP && g_wifi_cmd == WIFI_CMD_IDLE)
    {
        if(andonLoop() == FALSE) // 안돈에서 처리할 것이 없다면
        {
            otaAutoCheckLoop(); /* 2-Stage OTA 자동 체크 타이머 감시 */

            /* [수정 2] OTA 요청 발송 후 g_wifi_cmd가 바뀌었을 수 있으므로 IDLE 재확인
             * 문제: otaAutoCheckLoop()가 OTA HTTP GET을 보내 g_wifi_cmd=OTA_AUTO로 바꾼 직후
             *        WifiStrength 타이머도 만료돼 wifi_cmd(RECEIVED_STRENGTH)가 호출되면
             *        g_wifi_cmd가 RECEIVED_STRENGTH로 덮어쓰여 WiFi 모듈에 충돌 명령이 전송됨
             *        → RSSI 갱신 실패 → WIFI INFO 화면에 "disconnect" 표시 버그 */
            if(g_wifi_cmd == WIFI_CMD_IDLE &&
               isFinishCounter_1ms(g_index_WifiStrength))
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
            /* [수정 3] OTA HTTP 응답이 andonResponse()로 전달되는 버그 수정 (2026-03-20)
             * 문제: OTA VERSION/AUTO/CHUNK HTTPBODY 수신 시 g_sizeOfHttpText가 설정되고
             *        wifi_get_response() 리턴 후 무조건 andonResponse()가 호출됨
             *        → OTA JSON이 ANDON 파서에 전달 → ANDON 상태 오염 → andonLoop() 오작동
             *        → RSSI 체크 블록 미실행 → g_network.RSSI = INT16_MIN 유지
             *        → WIFI INFO 화면에 RSSI/WiFi 정보 표시 안됨
             * 해결: WIFI_CMD_HTTP 명령에 대한 응답만 andonResponse()로 전달 */
            if(g_sizeOfHttpText > 0 && g_wifi_cmd == WIFI_CMD_HTTP)
            {
                andonResponse(g_ptrHttpReceivedText,g_sizeOfHttpText);
            }
            g_sizeOfHttpText = 0;
            g_ptrHttpReceivedText=NULL;
        }
        else
        {
            wifiConnectAP(); // ???
        }
        
        clearWifiBuffer();
    }
}

#define STRSTR_WIFI_BUFFER(str) strstr((char*) g_WIFI_ReceiveBuffer,str)
    
// 할당된 IP를 찾는다.
uint8 wifiConnectAP()
{    
    int i=0;    
    static uint8 nLoop = 0;

    if(nLoop == 0) printf("\r\n");
    
    printf("LOOP[%d]:%s]", nLoop, g_WIFI_ReceiveBuffer);
    switch(nLoop)
    {
        case 0: // Wifi Ready Check
            if(STRSTR_WIFI_BUFFER("ICT*DEVICEREADY") != NULL)            
            {
                printf("\r\n[WIFI] Ready Wifi\r\n");
                g_network.isConnectAP = FALSE;
                wifi_cmd(WIFI_CMD_GET_MAC); // request MAC                
                nLoop++;
            }
            break;
        case 1: // get Mac
            if(STRSTR_WIFI_BUFFER("ICT*MAC:OK") != NULL)            
            {
                 printf("\r\n[WIFI] GET MAC\r\n");
                char *p = STRSTR_WIFI_BUFFER(" "); p++;
                while(*p != '\0')  g_network.MAC[i++]  = *p++;                  
               // printNetworkInfo();   
                g_network.bWifiReady = TRUE;       
                resetCounter_1ms(g_index_Wifi_Test);
            } else 
                printf("WIFI_RESP_GET_MAC ERROR\r\n");   
                              
            nLoop++;
            break;            
        case 2: // get IP
            if(STRSTR_WIFI_BUFFER("ICT*IPALLOCATED:") != NULL)            
            {
                 printf("\r\n[WIFI] GET IP\r\n");
                char *p = STRSTR_WIFI_BUFFER(":"); p++;
                
                while(*p != ' ') g_network.IPv4[i++]       = *p++; p++; i=0; // IPv4
                while(*p != ' ') g_network.SubnetMask[i++] = *p++; p++; i=0; // Subnet Mask                    
                while(*p != ' ') g_network.Gateway[i++]    = *p++; p++; i=0; // Gateway
                while(*p != '\0')  g_network.DNSServer[i++]  = *p++; p++; i=0; // DNS Server                    
                // printNetworkInfo();                   

                nLoop++;
                WIFI_CMD("AT*ICT*MIB=0"); break;
            }
            else if(STRSTR_WIFI_BUFFER("ICT*ASSOCIATED:") != NULL)            
            {
                char *p = STRSTR_WIFI_BUFFER(":"); p++;
                printf("AP Stat :");
                switch(atoi(p))
                {
                    case 0: printf("Success\r\n"); break;
                    case 1: printf("Fail\r\n"); break; 
                    case 2: printf("AP is not found\r\n"); 
                    break;   
                    case 3: printf("Connecttion is restricted"); break;
                }               
            }
            break;
            case 3:
                if(STRSTR_WIFI_BUFFER("ICT*MIB:OK") != NULL)
                {
                    printf("\r\n[WIFI] GET STRENGTH\r\n");
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
         //   printf("%c",c);
            
            if(g_size_WIFI_ReceiveBuffer >= MAX_WIFI_RECEIVE_BUFFER)
            {
                printf("Out of Memory :%d\r\n", g_size_WIFI_ReceiveBuffer);
                g_size_WIFI_ReceiveBuffer = 0;
            }
            if(c == LF)
            {
                g_WIFI_ReceiveBuffer[g_size_WIFI_ReceiveBuffer-1] = '\0';
                g_size_WIFI_ReceiveBuffer--;

                if(g_WIFI_ReceiveBuffer[g_size_WIFI_ReceiveBuffer-1] == CR)
                {
                    g_WIFI_ReceiveBuffer[g_size_WIFI_ReceiveBuffer-1] = '\0';
                    g_size_WIFI_ReceiveBuffer--;                    
                }
                
                return TRUE;
            }
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
        case WIFI_CMD_NETWORK_STATUS:    WIFI_CMD("AT*ICT*NWSTATUS=?"); break;
    }
    g_wifi_cmd = cmd;
    setCountMax_1ms(g_index_Wifi_Test, 3000); /* 모든 명령에 3초 타임아웃 설정 */
    resetCounter_1ms(g_index_Wifi_Test);       /* current=max 보장: setCountMax는 current=0으로 세팅 */
   // if(g_wifi_cmd == cmd) printf("WIFI_CMD_FACTORY_RESET\n");
}

/* ─── 내부 공통: IP+경로 분리 후 HTTP GET 전송 ─────────────────
 * host 형식: "192.168.38.72/CTP280_API"
 * AT 명령  : AT*ICT*HTTPGET=http://{ip}:{port}{/path}{url}     */
static void _wifi_send_httpget(const char *url, uint8 cmd, uint16 timeoutMs)
{
    char        ipPart[50];
    const char *pathPart = "";
    char       *slash    = strchr(g_ptrServer->host, '/');

    if(slash != NULL) {
        int ipLen = (int)(slash - g_ptrServer->host);
        strncpy(ipPart, g_ptrServer->host, ipLen);
        ipPart[ipLen] = '\0';
        pathPart = slash;
    } else {
        strncpy(ipPart, g_ptrServer->host, sizeof(ipPart) - 1);
        ipPart[sizeof(ipPart) - 1] = '\0';
    }

    wifi_printf("AT*ICT*HTTPGET=http://%s:%d%s%s\r\n",
                ipPart, g_ptrServer->port, pathPart, url_encode(url));
    printf("AT*ICT*HTTPGET=http://%s:%d%s%s\r\n",
           ipPart, g_ptrServer->port, pathPart, url_encode(url));

    g_wifi_cmd = cmd;
    setCountMax_1ms(g_index_Wifi_Test, timeoutMs);
    resetCounter_1ms(g_index_Wifi_Test); /* setCountMax는 current=0으로 세팅 → 다음 wifiLoop에서
                                          * isFinishCounter가 즉시 TRUE 반환되는 것을 방지.
                                          * resetCounter로 current=max(=timeoutMs)를 보장한다. */
}

void wifi_cmd_http(char *url)
{
    _wifi_send_httpget(url, WIFI_CMD_HTTP, 5000u);
}

/* ─── OTA 전용: host의 pathPart를 무시하고 IP + absolutePath 사용 ──
 * ANDON API: http://{ip}/{pathPart}{url}  (pathPart = /CTP280_API)
 * OTA API  : http://{ip}:{port}{absolutePath}   (pathPart 미사용)  */
static void _wifi_send_httpget_ota(const char *absolutePath, uint8 cmd, uint16 timeoutMs)
{
    char  ipPart[50];
    char *slash = strchr(g_ptrServer->host, '/');

    if(slash != NULL) {
        int ipLen = (int)(slash - g_ptrServer->host);
        strncpy(ipPart, g_ptrServer->host, ipLen);
        ipPart[ipLen] = '\0';
    } else {
        strncpy(ipPart, g_ptrServer->host, sizeof(ipPart) - 1);
        ipPart[sizeof(ipPart) - 1] = '\0';
    }

    wifi_printf("AT*ICT*HTTPGET=http://%s:%d%s\r\n",
                ipPart, g_ptrServer->port, url_encode(absolutePath));
    printf("AT*ICT*HTTPGET=http://%s:%d%s\r\n",
           ipPart, g_ptrServer->port, absolutePath);

    g_wifi_cmd = cmd;
    setCountMax_1ms(g_index_Wifi_Test, timeoutMs);
    resetCounter_1ms(g_index_Wifi_Test); /* setCountMax는 current=0으로 세팅 → 다음 wifiLoop에서
                                          * isFinishCounter가 즉시 TRUE 반환되는 것을 방지.
                                          * resetCounter로 current=max(=timeoutMs)를 보장한다. */
}

void wifi_cmd_ota_version(char *url)
{
    /* 수동 메뉴: 버전 체크, 15초 타임아웃
     * (해외 서버 응답이 5초를 초과하는 경우 확인됨 → 2026-03-19 5000→15000) */
    _wifi_send_httpget_ota(url, WIFI_CMD_OTA_VERSION, 15000u);
}

void wifi_cmd_ota_auto(char *url)
{
    /* 자동 백그라운드 체크: 15초 타임아웃, LCD 조작 없음 */
    _wifi_send_httpget_ota(url, WIFI_CMD_OTA_AUTO, 15000u);
}

void wifi_cmd_ota_chunk(char *url)
{
    /* 청크 다운로드: hex 800자 응답 → 15초 타임아웃 */
    _wifi_send_httpget_ota(url, WIFI_CMD_OTA_CHUNK, 15000u);
}

void wifi_get_response()
{
    int i=0;
    char *p = NULL;

    printf("%s - %d\r\n",g_WIFI_ReceiveBuffer, g_wifi_cmd);

    /* [수정 2] IDLE 상태에서 늦게 도착한 MIB 응답 처리 (2026-03-10)
     * 기존 문제: MIB 타임아웃(CMD 2 timeout→IDLE) 후 응답이 늦게 오면
     *            g_wifi_cmd=0(IDLE)이므로 switch 내 처리 케이스가 없어 무시됨
     *            → DrawWifi() 미호출 → g_network.RSSI = INT16_MIN(초기값) 유지
     *            → WiFi 연결됐음에도 화면에 WiFi 미연결 아이콘 표시
     * 해결: switch 진입 전 IDLE 상태의 MIB 응답을 먼저 처리하여 RSSI 갱신 및 DrawWifi() 호출
     * 롤백: 아래 if 블록 전체(6줄)를 제거하면 원복됨 */
    if(g_wifi_cmd == WIFI_CMD_IDLE && STRSTR_WIFI_BUFFER("ICT*MIB:OK") != NULL)
    {
        p = STRSTR_WIFI_BUFFER(" "); p++;
        g_network.RSSI = atoi(p);
        DrawWifi();
        return;
    }

    switch(g_wifi_cmd)
    {
        
        /* Get Mac Address */
        case WIFI_CMD_GET_MAC:
        {
            if(STRSTR_WIFI_BUFFER("ICT*MAC:OK") != NULL)            
            {
                p = STRSTR_WIFI_BUFFER(" "); p++;
                while(*p != '\0')  g_network.MAC[i++]  = *p++;   
                
                 //printNetworkInfo();                
            } else 
                printf("WIFI_RESP_GET_MAC ERROR\r\n");
            
            g_wifi_cmd = WIFI_CMD_IDLE;
        }
        break;   
        
        /* get Received signal strength indication */
        case WIFI_CMD_RECEIVED_STRENGTH:
            if(STRSTR_WIFI_BUFFER("ICT*MIB:OK") != NULL)
            {
                p = STRSTR_WIFI_BUFFER(" "); p++;
                g_network.RSSI = atoi(p);
                DrawWifi();
            } else
                printf("WIFI_CMD_RECEIVED_STRENGTH ERROR\r\n");

            g_wifi_cmd = WIFI_CMD_IDLE;    
        break;
                    
        /* get AP Scan list */
        case WIFI_CMD_AP_SCAN:
                printWifiBuffer();                
            if(STRSTR_WIFI_BUFFER("ICT*SCAN:OK") != NULL)
            {
                g_SizeOfAPs = 0;
            }
            else if(STRSTR_WIFI_BUFFER("ICT*SCANIND:") != NULL)
            {
                p = STRSTR_WIFI_BUFFER(" "); p++;
                appendAP(p);
            }
            else if(STRSTR_WIFI_BUFFER("ICT*SCANRESULT") != NULL)
            {
                g_wifi_cmd = WIFI_CMD_IDLE;
            } else
                printf("WIFI_CMD_AP_SCAN ERROR\r\n");
        break;
                
        /* Set IP Config */
        case WIFI_CMD_FACTORY_RESET:
            {
                printWifiBuffer();   
                //printf(".");
                if(STRSTR_WIFI_BUFFER("ICT*FACRESET:OK") != NULL)
                {
                    wifi_cmd(WIFI_CMD_IPCONFIG);                    
                    
                    printf("WIFI_CMD_IPCONFIG----\r\n");
                    break;
                }
            }
            break;
            
        /* Set IP Config */
        case WIFI_CMD_IPCONFIG:
            // printWifiBuffer();  
     
            if(STRSTR_WIFI_BUFFER("ICT*IPCONFIG:OK") != NULL)
            {                
                wifi_printf("AT*ICT*SCONN=%s %s\r\n",g_ptrServer->SSID, g_ptrServer->password);             
            }
            else if(STRSTR_WIFI_BUFFER("ICT*SCONN:OK") != NULL)
            {
                wifi_cmd(WIFI_CMD_NETWORK_STATUS);
            }               
            else if(STRSTR_WIFI_BUFFER("ICT*IPRELEASED") != NULL)
            {
            }            
            else if(STRSTR_WIFI_BUFFER("ICT*SSL_CLOSED:OK") != NULL)
            {
            }
            else if(STRSTR_WIFI_BUFFER("ICT*DISASSOCIATED") != NULL)
            {
            }    
            else if(STRSTR_WIFI_BUFFER("ICT*ASSOCIATED:") != NULL)
            {

            }               
            else if(STRSTR_WIFI_BUFFER("ICT*IPALLOCATED:") != NULL)
            {
                g_wifi_cmd = WIFI_CMD_IDLE;
            }  
            else
            {
            //    ShowMessage("ACCESS FAILUE");
                printf("WIFI_CMD_IPCONFIG ERROR\r\n");
            }
        break;
        case WIFI_CMD_NETWORK_STATUS:
            if(STRSTR_WIFI_BUFFER("ICT*NWSTATUS") != NULL)
            {
                CyDelay(500);
                WIFI_RESET_Write(0);                
                CyDelay(20);
                WIFI_RESET_Write(1);   
                CyDelay(20);   
               CySoftwareReset();                
            }
            break;
            /*
                       if(isFinishCounter_1ms(g_index_WifiStrength)) // Wifi Strength 체크를 5sec마다 수행한다.
            {
                resetCounter_1ms(g_index_WifiStrength);
                wifi_cmd(WIFI_CMD_RECEIVED_STRENGTH);               
            } */
        /* request HTTP */
        case WIFI_CMD_HTTP:
            if(isFinishCounter_1ms(g_index_Wifi_Test))
            {
                g_wifi_cmd = WIFI_CMD_IDLE; 
                printf("WIFI_CMD_HTTP\r\n");
            }
            
            if     (STRSTR_WIFI_BUFFER("ICT*HTTPGET:OK") != NULL)
            {
                resetCounter_1ms(g_index_Wifi_Test);
               // printf(">>>>>>>OK\r\n");
            }
            else if(STRSTR_WIFI_BUFFER("ICT*HTTPGET:ERROR") != NULL)
            {
                printf("ICT*HTTPGET:ERROR\r\n");
                g_wifi_cmd = WIFI_CMD_IDLE;
            }
            else if(STRSTR_WIFI_BUFFER("ICT*HTTPBODY:") != NULL)
            {
               printf(">BODY>%s\r\n",g_WIFI_ReceiveBuffer);
                p = STRSTR_WIFI_BUFFER(":"); p++;
                g_sizeOfHttpText = atoi(p); // No. of Received Data
            
                p = STRSTR_WIFI_BUFFER(" "); p++;

                g_ptrHttpReceivedText=p;    // pointer of Received Data
            }
            else if(STRSTR_WIFI_BUFFER("ICT*HTTPCLOSE:OK") != NULL)
            {
                g_wifi_cmd = WIFI_CMD_IDLE;
            }
        break;

        /* OTA: 서버 버전 확인 응답 */
        case WIFI_CMD_OTA_VERSION:
            if(isFinishCounter_1ms(g_index_Wifi_Test))
            {
                printf("[OTA] version check timeout\r\n");
                g_wifi_cmd = WIFI_CMD_IDLE;
                otaHandleVersionResponse("{}", 2); /* 타임아웃 시 빈 JSON으로 오류 처리 */
                break;
            }
            if(STRSTR_WIFI_BUFFER("ICT*HTTPGET:OK") != NULL)
            {
                resetCounter_1ms(g_index_Wifi_Test);
            }
            else if(STRSTR_WIFI_BUFFER("ICT*HTTPGET:ERROR") != NULL)
            {
                printf("[OTA] version HTTP error\r\n");
                g_wifi_cmd = WIFI_CMD_IDLE;
                otaHandleVersionResponse("{}", 2);
            }
            else if(STRSTR_WIFI_BUFFER("ICT*HTTPBODY:") != NULL)
            {
                p = STRSTR_WIFI_BUFFER(":"); p++;
                g_sizeOfHttpText = atoi(p);
                p = STRSTR_WIFI_BUFFER(" "); p++;
                g_ptrHttpReceivedText = p;
                printf("[OTA] version body: %s\r\n", g_ptrHttpReceivedText);
                otaHandleVersionResponse(g_ptrHttpReceivedText, g_sizeOfHttpText);
            }
            else if(STRSTR_WIFI_BUFFER("ICT*HTTPCLOSE:OK") != NULL)
            {
                g_wifi_cmd = WIFI_CMD_IDLE;
            }
        break;

        /* OTA: 펌웨어 청크 다운로드 응답 */
        case WIFI_CMD_OTA_CHUNK:
            if(isFinishCounter_1ms(g_index_Wifi_Test))
            {
                printf("[OTA] chunk timeout\r\n");
                g_wifi_cmd = WIFI_CMD_IDLE;
                otaHandleChunkResponse("{}", 2);
                break;
            }
            if(STRSTR_WIFI_BUFFER("ICT*HTTPGET:OK") != NULL)
            {
                resetCounter_1ms(g_index_Wifi_Test);
            }
            else if(STRSTR_WIFI_BUFFER("ICT*HTTPGET:ERROR") != NULL)
            {
                printf("[OTA] chunk HTTP error\r\n");
                g_wifi_cmd = WIFI_CMD_IDLE;
                otaHandleChunkResponse("{}", 2);
            }
            else if(STRSTR_WIFI_BUFFER("ICT*HTTPBODY:") != NULL)
            {
                p = STRSTR_WIFI_BUFFER(":"); p++;
                g_sizeOfHttpText = atoi(p);
                p = STRSTR_WIFI_BUFFER(" "); p++;
                g_ptrHttpReceivedText = p;
                printf("[OTA] chunk body size=%d\r\n", g_sizeOfHttpText);
                otaHandleChunkResponse(g_ptrHttpReceivedText, g_sizeOfHttpText);
            }
            else if(STRSTR_WIFI_BUFFER("ICT*HTTPCLOSE:OK") != NULL)
            {
                g_wifi_cmd = WIFI_CMD_IDLE;
            }
        break;

        /* OTA: 자동 백그라운드 버전 확인 응답 */
        case WIFI_CMD_OTA_AUTO:
            if(isFinishCounter_1ms(g_index_Wifi_Test))
            {
                printf("[OTA-AUTO] timeout\r\n");
                g_wifi_cmd = WIFI_CMD_IDLE;
                otaHandleAutoVersionResponse("{}", 2);
                break;
            }
            if(STRSTR_WIFI_BUFFER("ICT*HTTPGET:OK") != NULL)
            {
                resetCounter_1ms(g_index_Wifi_Test);
            }
            else if(STRSTR_WIFI_BUFFER("ICT*HTTPGET:ERROR") != NULL)
            {
                printf("[OTA-AUTO] HTTP error\r\n");
                g_wifi_cmd = WIFI_CMD_IDLE;
                otaHandleAutoVersionResponse("{}", 2);
            }
            else if(STRSTR_WIFI_BUFFER("ICT*HTTPBODY:") != NULL)
            {
                p = STRSTR_WIFI_BUFFER(":"); p++;
                g_sizeOfHttpText = atoi(p);
                p = STRSTR_WIFI_BUFFER(" "); p++;
                g_ptrHttpReceivedText = p;
                printf("[OTA-AUTO] body: %s\r\n", g_ptrHttpReceivedText);
                otaHandleAutoVersionResponse(g_ptrHttpReceivedText, g_sizeOfHttpText);
            }
            else if(STRSTR_WIFI_BUFFER("ICT*HTTPCLOSE:OK") != NULL)
            {
                g_wifi_cmd = WIFI_CMD_IDLE;
            }
        break;
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
        uint16 retries = 0;
        while(UART_WIFI_SpiUartGetTxBufferSize() && retries < 10000) retries++;
        UART_WIFI_UartPutChar(buff[i]);
    }
}

void clearWifiBuffer() { g_size_WIFI_ReceiveBuffer = 0; }


void printWifiBuffer()
{
    if(g_size_WIFI_ReceiveBuffer >0 ) printf("[->]%s[<-]\r\n",g_WIFI_ReceiveBuffer);
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
    //while(*ptr != ' ') ptrAP->SSID[i++] = *ptr++;
    //ptrAP->SSID[i] = 0;
    while(*ptr != ':') ptrAP->SSID[i++] = *ptr++;
    ptrAP->SSID[i-3] = 0;
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
    
    // printf(">%s %s %d\r\n",ptrAP->SSID, ptrAP->MAC, ptrAP->RSSI);
}
/* [] END OF FILE */