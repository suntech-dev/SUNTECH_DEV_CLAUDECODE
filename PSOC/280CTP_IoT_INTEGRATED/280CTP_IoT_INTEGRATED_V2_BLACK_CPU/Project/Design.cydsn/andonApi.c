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
#include "otaMenu.h"

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

/* 패턴 New CPU 보드의 Uart 신호. (디바이스 4pin. 1번,2번) */
void makeAndonPatternCount()
{
    COUNT *ptrCount = getCount();    
    uint32 pts =   GetWorkingTimeCount();
    ResetWorkingTimeCount();

    //enQueueANDON_printf(ANDON_SEND_PATTERN_COUNT,"send_pCount&mac=%s&d_idx=%u&pc=%u&pi=%0.1f&tg=%u&sc=%0.1f&no=%u&et=%u&ets=%lu&&ct=%u&cts=%lu&mrt=%u&mrts=%lu&sq=%u&sqs=%lu&sl=%u&sls=%lu&tc=%u&tcs=%lu&spm=%u&pts=%lu\r\n",
    //enQueueANDON_printf(ANDON_SEND_PATTERN_COUNT,"send_pCount&mac=%s&pc=%u&pi=%0.1f&tg=%u&sc=%0.1f&no=%u&et=%u&ets=%lu&&ct=%u&cts=%lu&mrt=%u&mrts=%lu&sq=%u&sqs=%lu&sl=%u&sls=%lu&tc=%u&tcs=%lu&spm=%u&pts=%lu\r\n",
    enQueueANDON_printf(ANDON_SEND_PATTERN_COUNT,"send_pCount&mac=%s&pc=%u&pi=%0.1f&sc=%0.1f&design_no=%u&ct=%u\r\n",
        g_network.MAC,
        //g_Info.DeviceIdx,
        g_ptrMachineParameter->patternPairCount,                                                            // pc  : pair Count        
        g_ptrMachineParameter->patternPair / 10.,                                                           // pi  : pair info
        //g_ptrMachineParameter->patternTarget,                                                               // tg  : target
        (uint32) CONVERT_TO_4BYTE(ptrCount->patternActualH,ptrCount->patternActualL) / 10.,                 // sc  : shift actual
        ptrCount->patternNo,
        //ptrCount->patternEmergencyTime,                                                                     // et  : emergency time
        //(uint32) CONVERT_TO_4BYTE(ptrCount->patternEmergencyTimeSumH,ptrCount->patternEmergencyTimeSumL),   // ets : emergency time sum
        ptrCount->patternCycleTime                                                                         // ct  : cycle time
        //(uint32) CONVERT_TO_4BYTE(ptrCount->patternCycleTimeSumH,ptrCount->patternCycleTimeSumL),           // cts : cycle time sum
        //ptrCount->patternMotorRunTime,                                                                      // mrt : motor run time 
        //(uint32) CONVERT_TO_4BYTE(ptrCount->patternMotorRunTimeSumH, ptrCount->patternMotorRunTimeSumL),    // mrts: motor run time sum
        //ptrCount->patternNoStitch,                                                                          // sq  : no of stitch
        //(uint32) CONVERT_TO_4BYTE(ptrCount->patternNoStitchSumH, ptrCount->patternNoStitchSumL),            // sqs : no of stitch sum
        //ptrCount->patternStitchLength,                                                                      // sl  : stitch Length
        //(uint32) CONVERT_TO_4BYTE(ptrCount->patternStitchLengthSumH,ptrCount->patternStitchLengthSumL),     // sls : stitch Length Sum          
        //ptrCount->patternTrimCount,                                                                         // tc  : trim count
        //(uint32) CONVERT_TO_4BYTE(ptrCount->patternTrimCountSumH,ptrCount->patternTrimCountSumL),           // tcs : trim count sum
        //ptrCount->patternSPM,                                                                               // spm : stitching Speed  
        //pts                                                                                                 // pts : powor on time (reflash)                                                                                                            
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

    /* 2-Stage OTA 자동 체크 시작:
     * 위 5개 요청이 모두 완료될 때까지 20초 대기 후 버전 확인     */
    otaAutoCheckInit();
}

/* [] END OF FILE */
