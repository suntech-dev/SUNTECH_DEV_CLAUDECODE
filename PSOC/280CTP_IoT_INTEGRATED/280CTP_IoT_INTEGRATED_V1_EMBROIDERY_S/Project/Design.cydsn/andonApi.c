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
#include "andonApi.h"
#include "andonJson.h"
#include "andonMessageQueue.h"
#include "lib/WIFI.h"
#include "lib/server.h"
#include "lib/utility.h"
#include "userProjectPatternSewing.h"
#include "count.h"
#include "package.h"
#include "lib/sysTick.h"
#include "WarningLight.h"
#include "downtime.h"
#include "defective.h"

ANDON_LISTS g_AndonLists;
uint8       g_bDeviceInfoUpdate=FALSE;
uint8       g_uReplyResult=RR_INIT;
static uint16 g_uAndonRequestType = ANDON_NONE;  /* andonApi.c 내부에서만 사용 */
uint8       g_bTargetReceived = FALSE;
uint8       g_index_request_interval=0;
uint8       g_bReceivedAndonStart = 0;

INFO  g_Info = {
    .nNoOfLineForNotice =0, 
};

uint8 andonLoop()
{
    if(!isEmptyQueueANDON())
    {
        ANDON_QUEUE *aq = deQueueANDON();
        
        char url[512];
        snprintf(url, sizeof(url), "%s?code=%s", DEFAULT_API_ENDPOINT, aq->message);

        g_uAndonRequestType = aq->type;

        wifi_cmd_http(url);
        
        return TRUE;
    }
    
    if(g_bReceivedAndonStart)
    {
        if(isFinishCounter_1ms(g_index_request_interval)) // Wifi Strength 체크를 5sec마다 수행한다.
        {
            resetCounter_1ms(g_index_request_interval);
            
            if(g_ptrMachineParameter->andon_enable==TRUE) makeAndonList();             
            
            return TRUE;
        }
        
        WarningLight();
    }
   
    return FALSE;
}


void andonResponse(char *ptrText, int16 sizeOfText)
{
    andonJsonParsor(g_uAndonRequestType, ptrText, sizeOfText);
    g_uAndonRequestType = ANDON_NONE;
}

void makeAndonCurrentTimeRequest()
{
    enQueueANDON_printf(ANDON_CURRENT_TIME,"get_dateTime");
}

void makeAndonStart()
{
    //enQueueANDON_printf(ANDON_START,"start&mac=%s&name=%s&ip=%s&ver=%s",
    enQueueANDON_printf(ANDON_START,"start&mac=%s&machine_no=%s&ip=%s&ver=%s",
        g_network.MAC,
        g_ptrServer->deviceName,
        g_network.IPv4,
        PROJECT_FIRMWARE_VERSION
    ); 
}

void makeAndonList()
{
    enQueueANDON_printf(ANDON_REQUEST_LIST,"get_andonList&mac=%s",
        g_network.MAC
    );   
}

void makeAndonWarningRequest(int idx)
{
    //enQueueANDON_printf(ANDON_REQUEST_ITEM,"send_andon_warning&mac=%s&andon_text_idx=%d",
    enQueueANDON_printf(ANDON_REQUEST_ITEM,"send_andon_warning&mac=%s&andon_idx=%d",
        g_network.MAC,
        idx
    );  
}

void makeAndonCompleteRequest(int idx)
{
    //enQueueANDON_printf(ANDON_REQUEST_ITEM,"send_andon_completed&mac=%s&andon_text_idx=%d",
    enQueueANDON_printf(ANDON_REQUEST_ITEM,"send_andon_completed&mac=%s&andon_idx=%d",
        g_network.MAC,
        idx);  
}

/* 자수기 UART 패킷 수신 시 서버 전송 — EMBROIDERY_S 전용 */
void makeAndonPatternCount()
{
    COUNT *ptrCount = getCount();

    enQueueANDON_printf(ANDON_SEND_PATTERN_COUNT,
        "send_eCount&mac=%s&actual_qty=%u&ct=%0.1f&tb=%u&mrt=%0.1f\r\n",
        g_network.MAC,
        ptrCount->patternCount,             // actual_qty : 이번 패킷 완료 수량
        ptrCount->patternCycleTime / 10.,   // ct  : 싸이클타임 (0.1s 단위 → 소수점 1자리 초)
        ptrCount->embThreadBreakageQty,     // tb  : 실끊김 수량
        ptrCount->patternMotorRunTime / 10. // mrt : 모터동작시간 (0.1s 단위 → 소수점 1자리 초)
    );
}

/* 현 버전에서 사용하지 않는 함수들 Start. */
void makeAndonDevice()
{
    enQueueANDON_printf(ANDON_GET_DEVICE_NAME,"send_device&mac=%s&name=%s&ip=%s&ver=%s",g_network.MAC,g_ptrServer->deviceName,g_network.IPv4,PROJECT_FIRMWARE_VERSION);
}

void makeRequestTarget()
{
    enQueueANDON_printf(ANDON_REQUEST_TARGET,"get_target&device_idx=%d",g_Info.DeviceIdx);
}

void makeAndonTextList()
{
    enQueueANDON_printf(ANDON_REQUEST_TEXT_LIST,"get_textList&device_idx=%d",g_ptrServer->deviceIndex);
}
/* 현 버전에서 사용하지 않는 함수들 End */

void initAndon()
{
    g_index_request_interval = registerCounter_1ms(5000);
    makeAndonCurrentTimeRequest();
    makeAndonStart();
    makeAndonList();
    makeDownTimeList();
    makeDefectiveList();
    //makeAndonTextList();
}

/* [] END OF FILE */
