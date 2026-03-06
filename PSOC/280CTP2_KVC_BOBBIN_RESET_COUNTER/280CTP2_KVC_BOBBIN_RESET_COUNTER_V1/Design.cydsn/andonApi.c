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
                                
LISTS g_Lists;
uint8 g_bDeviceInfoUpdate=FALSE;
uint8 g_uReplyResult=RR_INIT;
uint16 g_uAndonRequestType;

uint16 g_uAndonRequestType=ANDON_NONE;

INFO  g_Info = {
    .nNoOfLineForNotice =0, 
};

uint8 andonLoop()
{
    if(!isEmptyQueueANDON())
    {
        ANDON_QUEUE *aq = deQueueANDON();
        
        char url[512];
        sprintf(url,"%s?code=%s",g_strServeURL,aq->message);

        g_uAndonRequestType = aq->type;
        
        wifi_cmd_http(url);
        return TRUE;
    }
    return FALSE;
}

void makeAndonCurrentTimeRequest()
{
    enQueueANDON_printf(ANDON_CURRENT_TIME,"get_dateTime");
}

void makeAndonDevice()
{
    enQueueANDON_printf(ANDON_GET_DEVICE_NAME,"send_device&mac=%s&name=%s&ip=%s&ver=%s",g_network.MAC,g_ptrServer->deviceName,g_network.IPv4,PROJECT_FIRMWARE_VERSION); 
}

void makeAndonList()
{
    enQueueANDON_printf(ANDON_REQUEST_LIST,"get_andonList&device_idx=%d",g_ptrServer->deviceIndex);   
}

void makeAndonTextList()
{
    enQueueANDON_printf(ANDON_REQUEST_TEXT_LIST,"get_textList&device_idx=%d",g_ptrServer->deviceIndex);   
}

void andonResponse(char *ptrText, int16 sizeOfText)
{
    andonJsonParsor(g_uAndonRequestType, ptrText, sizeOfText);
    g_uAndonRequestType = ANDON_NONE;
}

void makeAndonSewingCount()
{
//    enQueueANDON_printf(ANDON_SEND_SEWING_COUNT,"send _cCount&d_idx=%u&pi=%u&tg=%u&sc=%u&ct=%u&cts=%lu&mrt=%u&mrts=%lu&sq=%u&sqs=%lu&tc=%u&tcs=%lu",
//        g_Info.DeviceIdx,
//        (g_ptrMachineParameter->sewingPair/g_ptrMachineParameter->sewingPairTrim) / 10 , // pi  : pair info
//        g_ptrMachineParameter->sewingTarget,// tg  : target
//        g_ptrCount->sewingActual,           // sc  : shift actual
//        g_ptrCount->sewingCycleTime,        // ct  : cycle time
//        g_ptrCount->sewingCycleTimeSum,     // cts : cycle time sum
//        g_ptrCount->sewingMotorRunTime,     // mrt : motor run time 
//        g_ptrCount->sewingMotorRunTimeSum,  // mrts: motor run time sum
//        g_ptrCount->sewingNoStitch,         // sq  : no of stitch
//        g_ptrCount->sewingNoStitchSum,      // sqs : no of stitch sum
//        g_ptrCount->sewingTrimCount,        // tc  : trim count
//        g_ptrCount->sewingTrimCountSum      // tcs : trim count sum
//        );     
#ifdef USER_PROJECT_PATTERN_SEWING_MACHINE      
    enQueueANDON_printf(ANDON_SEND_SEWING_COUNT,"send_cCount&d_idx=%u&pi=%0.1f&tg=%u&sc=%0.1f&ct=%u&cts=%lu&mrt=%u&mrts=%lu&sq=%u&sqs=%lu&tc=%u&tcs=%lu",
        g_Info.DeviceIdx,
 //       g_ptrMachineParameter->sewingPairTrim,                                                             // pc  : pair Trim        
        g_ptrMachineParameter->sewingPair / 10. ,                                                          // pi  : pair info
        g_ptrMachineParameter->sewingTarget,                                                               // tg  : target
        (uint32) CONVERT_TO_4BYTE(g_ptrCount->sewingActualH,g_ptrCount->sewingActualL) / 10.,               // sc  : shift actual
        g_ptrCount->sewingCycleTime,                                                                       // ct  : cycle time
        (uint32) CONVERT_TO_4BYTE(g_ptrCount->sewingCycleTimeSumH, g_ptrCount->sewingCycleTimeSumL),       // cts : cycle time sum
        g_ptrCount->sewingMotorRunTime,                                                                    // mrt : motor run time 
        (uint32) CONVERT_TO_4BYTE(g_ptrCount->sewingMotorRunTimeSumH, g_ptrCount->sewingMotorRunTimeSumL), // mrts: motor run time sum
        g_ptrCount->sewingNoStitch,                                                                        // sq  : no of stitch
        (uint32) CONVERT_TO_4BYTE(g_ptrCount->sewingNoStitchSumH,g_ptrCount->sewingNoStitchSumL),          // sqs : no of stitch sum
        g_ptrMachineParameter->sewingPairTrim,                                                                       // tc  : trim count
        (uint32) CONVERT_TO_4BYTE(g_ptrCount->sewingTrimCountSumH,g_ptrCount->sewingTrimCountSumL)         // tcs : trim count sum
        ); 
#endif    
//    printf("send_cCount&d_idx=%u&pi=%0.1f&tg=%u&sc=%0.1f&ct=%u&cts=%lu&mrt=%u&mrts=%lu&sq=%u&sqs=%lu&tc=%u&tcs=%lu",
//        g_Info.DeviceIdx,
// //       g_ptrMachineParameter->sewingPairTrim,                                                             // pc  : pair Trim        
//        g_ptrMachineParameter->sewingPair / 10. ,                                                          // pi  : pair info
//        g_ptrMachineParameter->sewingTarget,                                                               // tg  : target
//        (uint32) CONVERT_TO_4BYTE(g_ptrCount->sewingActualH,g_ptrCount->sewingActualL) / 10.,               // sc  : shift actual
//        g_ptrCount->sewingCycleTime,                                                                       // ct  : cycle time
//        (uint32) CONVERT_TO_4BYTE(g_ptrCount->sewingCycleTimeSumH, g_ptrCount->sewingCycleTimeSumL),       // cts : cycle time sum
//        g_ptrCount->sewingMotorRunTime,                                                                    // mrt : motor run time 
//        (uint32) CONVERT_TO_4BYTE(g_ptrCount->sewingMotorRunTimeSumH, g_ptrCount->sewingMotorRunTimeSumL), // mrts: motor run time sum
//        g_ptrCount->sewingNoStitch,                                                                        // sq  : no of stitch
//        (uint32) CONVERT_TO_4BYTE(g_ptrCount->sewingNoStitchSumH,g_ptrCount->sewingNoStitchSumL),          // sqs : no of stitch sum
//        g_ptrMachineParameter->sewingPairTrim,                                                                       // tc  : trim count
//        (uint32) CONVERT_TO_4BYTE(g_ptrCount->sewingTrimCountSumH,g_ptrCount->sewingTrimCountSumL)         // tcs : trim count sum
//        ); 
    
//    printf("send_cCount&d_idx=%u&pc=%u&pi=%3.1f&tg=%u&sc=%lu&ct=%u&cts=%lu&mrt=%u&mrts=%lu&sq=%u&sqs=%lu&tc=%u&tcs=%lu",
//        g_Info.DeviceIdx,
//        g_ptrMachineParameter->sewingPairTrim,                                                             // pc  : pair Trim        
//        g_ptrMachineParameter->sewingPair / 10. ,                                                          // pi  : pair info
//        g_ptrMachineParameter->sewingTarget,                                                               // tg  : target
//        (uint32) CONVERT_TO_4BYTE(g_ptrCount->sewingActualH,g_ptrCount->sewingActualL) / 10,               // sc  : shift actual
//        g_ptrCount->sewingCycleTime,                                                                       // ct  : cycle time
//        (uint32) CONVERT_TO_4BYTE(g_ptrCount->sewingCycleTimeSumH, g_ptrCount->sewingCycleTimeSumL),       // cts : cycle time sum
//        g_ptrCount->sewingMotorRunTime,                                                                    // mrt : motor run time 
//        (uint32) CONVERT_TO_4BYTE(g_ptrCount->sewingMotorRunTimeSumH, g_ptrCount->sewingMotorRunTimeSumL), // mrts: motor run time sum
//        g_ptrCount->sewingNoStitch,                                                                        // sq  : no of stitch
//        (uint32) CONVERT_TO_4BYTE(g_ptrCount->sewingNoStitchSumH,g_ptrCount->sewingNoStitchSumL),          // sqs : no of stitch sum
//        g_ptrCount->sewingTrimCount,                                                                       // tc  : trim count
//        (uint32) CONVERT_TO_4BYTE(g_ptrCount->sewingTrimCountSumH,g_ptrCount->sewingTrimCountSumL)         // tcs : trim count sum
//        );      
     
}
#ifdef USER_PROJECT_PATTERN_SEWING_MACHINE  
void makeAndonPatternCount()
{
       printf("send_pCount&d_idx=%u&pc=%u&pi=%0.1f&tg=%u&sc=%0.1f&no=%u&et=%u&ets=%lu&&ct=%u&cts=%lu&mrt=%u&mrts=%lu&sq=%u&sqs=%lu&sl=%u&sls=%lu&tc=%u&tcs=%lu&spm=%u\r\n",
        g_Info.DeviceIdx,
        g_ptrMachineParameter->patternPairCount,                                                             // pc  : pair Count        
        g_ptrMachineParameter->patternPair / 10.,                                                            // pi  : pair info
        g_ptrMachineParameter->patternTarget,                                                                // tg  : target
        (uint32) CONVERT_TO_4BYTE(g_ptrCount->patternActualH,g_ptrCount->patternActualL) / 10.,              // sc  : shift actual
        g_ptrCount->patternNo,
        g_ptrCount->patternEmergencyTime,                                                                    // et  : emergency time
        (uint32) CONVERT_TO_4BYTE(g_ptrCount->patternEmergencyTimeSumH,g_ptrCount->patternEmergencyTimeSumL),// ets : emergency time sum
        g_ptrCount->patternCycleTime,                                                                        // ct  : cycle time
        (uint32) CONVERT_TO_4BYTE(g_ptrCount->patternCycleTimeSumH,g_ptrCount->patternCycleTimeSumL),        // cts : cycle time sum
        g_ptrCount->patternMotorRunTime,                                                                     // mrt : motor run time 
        (uint32) CONVERT_TO_4BYTE(g_ptrCount->patternMotorRunTimeSumH, g_ptrCount->patternMotorRunTimeSumL), // mrts: motor run time sum
        g_ptrCount->patternNoStitch,                                                                         // sq  : no of stitch
        (uint32) CONVERT_TO_4BYTE(g_ptrCount->patternNoStitchSumH, g_ptrCount->patternNoStitchSumL),         // sqs : no of stitch sum
        g_ptrCount->patternStitchLength,                                                                     // sl  : stitch Length
        (uint32) CONVERT_TO_4BYTE(g_ptrCount->patternStitchLengthSumH,g_ptrCount->patternStitchLengthSumL),  // sls : stitch Length Sum          
        g_ptrCount->patternTrimCount,                                                                        // tc  : trim count
        (uint32) CONVERT_TO_4BYTE(g_ptrCount->patternTrimCountSumH,g_ptrCount->patternTrimCountSumL),        // tcs : trim count sum
        g_ptrCount->patternSPM                                                                               // spm : stitching Speed  
        ); 

    enQueueANDON_printf(ANDON_SEND_PATTERN_COUNT,"send_pCount&d_idx=%u&pc=%u&pi=%0.1f&tg=%u&sc=%0.1f&no=%u&et=%u&ets=%lu&&ct=%u&cts=%lu&mrt=%u&mrts=%lu&sq=%u&sqs=%lu&sl=%u&sls=%lu&tc=%u&tcs=%lu&spm=%u\r\n",
        g_Info.DeviceIdx,
        g_ptrMachineParameter->patternPairCount,                                                             // pc  : pair Count        
        g_ptrMachineParameter->patternPair / 10.,                                                            // pi  : pair info
        g_ptrMachineParameter->patternTarget,                                                                // tg  : target
        (uint32) CONVERT_TO_4BYTE(g_ptrCount->patternActualH,g_ptrCount->patternActualL) / 10.,              // sc  : shift actual
        g_ptrCount->patternNo,
        g_ptrCount->patternEmergencyTime,                                                                    // et  : emergency time
        (uint32) CONVERT_TO_4BYTE(g_ptrCount->patternEmergencyTimeSumH,g_ptrCount->patternEmergencyTimeSumL),// ets : emergency time sum
        g_ptrCount->patternCycleTime,                                                                        // ct  : cycle time
        (uint32) CONVERT_TO_4BYTE(g_ptrCount->patternCycleTimeSumH,g_ptrCount->patternCycleTimeSumL),        // cts : cycle time sum
        g_ptrCount->patternMotorRunTime,                                                                     // mrt : motor run time 
        (uint32) CONVERT_TO_4BYTE(g_ptrCount->patternMotorRunTimeSumH, g_ptrCount->patternMotorRunTimeSumL), // mrts: motor run time sum
        g_ptrCount->patternNoStitch,                                                                         // sq  : no of stitch
        (uint32) CONVERT_TO_4BYTE(g_ptrCount->patternNoStitchSumH, g_ptrCount->patternNoStitchSumL),         // sqs : no of stitch sum
        g_ptrCount->patternStitchLength,                                                                     // sl  : stitch Length
        (uint32) CONVERT_TO_4BYTE(g_ptrCount->patternStitchLengthSumH,g_ptrCount->patternStitchLengthSumL),  // sls : stitch Length Sum          
        g_ptrCount->patternTrimCount,                                                                        // tc  : trim count
        (uint32) CONVERT_TO_4BYTE(g_ptrCount->patternTrimCountSumH,g_ptrCount->patternTrimCountSumL),        // tcs : trim count sum
        g_ptrCount->patternSPM                                                                               // spm : stitching Speed  
        );    
}
#endif
void initAndon()
{
    makeAndonCurrentTimeRequest();
    makeAndonDevice();    
   // makeAndonList();
   // makeAndonTextList();
}

/* [] END OF FILE */
