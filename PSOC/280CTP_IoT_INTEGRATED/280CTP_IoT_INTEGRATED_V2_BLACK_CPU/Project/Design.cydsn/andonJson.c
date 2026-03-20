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
#include "andonJson.h"
#include "jsonUtil.h"
#include "andonApi.h"
#include "count.h"
#include "lib/RealTimeClock.h"
#include "lib/widget.h"
#include "lib/externalFlash.h"
#include "userProjectPatternSewing.h"
#include "lib/sysTick.h"
#include "WarningLight.h"
#include "downtime.h"
#include "defective.h"

uint32 g_lastSendTime;

uint8 andonCurrentTimeParsing  (char *jsonString, int16 sizeOfJson);
uint8 andonGetDeviceName       (char *jsonString, int16 sizeOfJson);
uint8 andonStart               (char *jsonString, int16 sizeOfJson);
uint8 andonRequestItem         (char *jsonString, int16 sizeOfJson);
uint8 andonSendCount           (char *jsonString, int16 sizeOfJson);
uint8 andonMenuParsing         (char *jsonString, int16 sizeOfJson);
uint8 andonRequestTargetParsing(char *jsonString, int16 sizeOfJson); 

void andonJsonParsor(int type, char *jsonString, int16 sizeOfJson)
{
    //printf("> %d, %s\r\n",type,jsonString);
    switch(type)
    {
        case ANDON_CURRENT_TIME:       andonCurrentTimeParsing     (jsonString, sizeOfJson); break;
        case ANDON_GET_DEVICE_NAME:    if(andonGetDeviceName (jsonString, sizeOfJson) == TRUE) makeRequestTarget(); break;   
        case ANDON_START:              andonStart                  (jsonString, sizeOfJson);
                                       //makeAndonList();
                                       break;
        case ANDON_REQUEST_ITEM:       andonRequestItem (jsonString, sizeOfJson); 
                                       makeAndonList();
                                       break;
        case ANDON_SEND_SEWING_COUNT:  //makeAndonStart();                            
        case ANDON_SEND_PATTERN_COUNT: andonSendCount              (jsonString, sizeOfJson); 
                                       ///makeAndonStart();
        break;        
        case ANDON_REQUEST_LIST:       andonMenuParsing            (jsonString, sizeOfJson); break;
        case ANDON_REQUEST_TEXT_LIST:  andonMenuParsing            (jsonString, sizeOfJson); break; 
        case ANDON_REQUEST_TARGET:     andonRequestTargetParsing   (jsonString, sizeOfJson); 
                                       if(isInTopMenu()) g_TopMenuNode->func(NULL,REFLASH);
                                       break; 
                                    
        case DOWNTIME_REQUEST_LIST:     
        case DOWNTIME_REQUEST_ITEM:    downTimeParsing             (jsonString, sizeOfJson);  break;
        case DEFECTIVE_REQUEST_LIST:     
        case DEFECTIVE_REQUEST_ITEM:   defectiveParsing             (jsonString, sizeOfJson);  break;           
//        case DOWNTIME_REQUEST_LIST:    downTimeParsing             (jsonString, sizeOfJson); break;  
//        case DOWNTIME_REQUEST_ITEM:    downTimeRequestItem         (jsonString, sizeOfJson);  break;

    }
}

uint8 andonCurrentTimeParsing(char *jsonString, int16 sizeOfJson)
{
    int i;    
    int r;
    jsmn_parser p;
    jsmntok_t t[80]; /* 최대 토큰 수: 80 (10개 항목 JSON = 73 tokens, 여유 7) */

    jsmn_init(&p);
    r = jsmn_parse(&p, jsonString, sizeOfJson, t, sizeof(t) / sizeof(t[0]));
    
    if (r < 0) {
        #ifdef DEBUG_ANDON_JSON
            printf("Failed to parse JSON: %d\r\n", r);
        #endif
        return FALSE;
    }
    
    /* Assume the top-level element is an object */
    if (r < 1 || t[0].type != JSMN_OBJECT) {
        #ifdef DEBUG_ANDON_JSON
            printf("Object expected type: %d\r\n", t[0].type);
        #endif        
        return FALSE;
    }

    for (i = 1; i < r; i++)
    {   
        if (jsoneq(jsonString, &t[i], "datetime") == 0)
        {
            RTC_SetUnixTime(g_ptrMachineParameter->lastPowerOnDateTime);
            
            uint32 lastPowerOnDate = RTC_GetDate(), currentDate;
   
            setCurrentTime(GetJSON_Token_String(&t[i + 1], jsonString));
            
            currentDate  = RTC_GetDate();
               
            if(RTC_GetDay(lastPowerOnDate) != RTC_GetDay(currentDate))
            {
                if(g_ptrMachineParameter->bAutoReset) ResetCount();             
            }
            
            g_ptrMachineParameter->lastPowerOnDateTime = RTC_GetUnixTime();

            break;
        }
    }
    if(isInTopMenu()) reflashMenu();
    //g_TopMenuNode->func(NULL,REFLASH);
    SaveExternalFlashConfig();
    return TRUE;
}

uint8 andonGetDeviceName(char *jsonString, int16 sizeOfJson)
{
    int i;
    int r;
    jsmn_parser p;
    jsmntok_t t[80]; /* 최대 토큰 수: 80 (10개 항목 JSON = 73 tokens, 여유 7) */
    
    jsmn_init(&p);
    r = jsmn_parse(&p, jsonString, sizeOfJson, t, sizeof(t) / sizeof(t[0]));
    
    if (r < 0) {
        #ifdef DEBUG_ANDON_JSON
            printf("Failed to parse JSON: %d\r\n", r);
        #endif
        return FALSE;
    }
    
    /* Assume the top-level element is an object */
    if (r < 1 || t[0].type != JSMN_OBJECT) {
        #ifdef DEBUG_ANDON_JSON
            printf("Object expected type: %d\r\n", t[0].type);
        #endif     
        return FALSE;
    }
    
    for (i = 1; i < r; i++)
    {   
        //if (jsoneq(jsonString, &t[i], "device_no") == 0)
        if (jsoneq(jsonString, &t[i], "machine_no") == 0)
        {
            sprintf(g_Info.DeviceNo,"%.*s",t[i + 1].end - t[i + 1].start,jsonString + t[i + 1].start); 
            if(strlen(g_Info.DeviceNo) == 0) strcpy(g_Info.DeviceNo,"IoT Device");
        }
        else if(jsoneq(jsonString, &t[i], "install_type") == 0)
        {
            sprintf(g_Info.InstallType,"%.*s",t[i + 1].end - t[i + 1].start,jsonString + t[i + 1].start);            
        }
        else if(jsoneq(jsonString, &t[i], "line_name") == 0)
        {
            sprintf(g_Info.LineName,"%.*s",t[i + 1].end - t[i + 1].start,jsonString + t[i + 1].start);            
        }              
        //else if(jsoneq(jsonString, &t[i], "device_idx") == 0)
        else if(jsoneq(jsonString, &t[i], "machine_idx") == 0)
        {
            char buff[20];
            sprintf(buff,"%.*s",t[i + 1].end - t[i + 1].start,jsonString + t[i + 1].start);
            g_Info.DeviceIdx = atoi(buff);
        }
    }
    
    //printf("andonGetDeviceName() : %s, %s, %d\r\n",g_Info.DeviceNo,g_Info.InstallType,g_Info.DeviceIdx);
    SetTitleBarText(g_Info.DeviceNo);
    DrawTitle();
    g_Info.bUpdateDeviceInfo = TRUE;
    
    return TRUE;
}

uint8 andonSendCount(char *jsonString, int16 sizeOfJson)
{
    int i;
    int r;
    jsmn_parser p;
    jsmntok_t t[80]; /* 최대 토큰 수: 80 (10개 항목 JSON = 73 tokens, 여유 7) */
    
    jsmn_init(&p);
    r = jsmn_parse(&p, jsonString, sizeOfJson, t, sizeof(t) / sizeof(t[0]));
  
    if (r < 0) {
        #ifdef DEBUG_ANDON_JSON
            printf("Failed to parse JSON: %d\r\n", r);
        #endif
        return FALSE;
    }
    
    /* Assume the top-level element is an object */
    if (r < 1 || t[0].type != JSMN_OBJECT) {
        #ifdef DEBUG_ANDON_JSON
            printf("Object expected type: %d\r\n", t[0].type);
        #endif     
        return FALSE;
    }
    
    //printf("andonSendCount():%s",jsonString);
    char buff[256];
    for (i = 1; i < r; i++)
    {   
        if     (jsoneq(jsonString, &t[i], "code") == 0)
        {
            sprintf(buff,"%.*s",t[i + 1].end - t[i + 1].start,jsonString + t[i + 1].start);      
            if(strcmp(buff,"00") == 0)
            {
                return TRUE;
            }
        }
    }
        
    return TRUE;    
}

uint8 andonStart(char *jsonString, int16 sizeOfJson)
{
    int i;
    int r;
    jsmn_parser p;
    jsmntok_t t[80]; /* 최대 토큰 수: 80 (10개 항목 JSON = 73 tokens, 여유 7) */
        
    jsmn_init(&p);
    r = jsmn_parse(&p, jsonString, sizeOfJson, t, sizeof(t) / sizeof(t[0]));
    
    if (r < 0) {
        #ifdef DEBUG_ANDON_JSON
            printf("Failed to parse JSON: %d\r\n", r);
        #endif
        return FALSE;
    }
    
    /* Assume the top-level element is an object */
    if (r < 1 || t[0].type != JSMN_OBJECT) {
        #ifdef DEBUG_ANDON_JSON
            printf("Object expected type: %d\r\n", t[0].type);
        #endif     
        return FALSE;
    }
    char buff[20];
    for (i = 1; i < r; i++)
    {   
        //if     (jsoneq(jsonString, &t[i], "device_no") == 0)
        if     (jsoneq(jsonString, &t[i], "machine_no") == 0)
        {
            sprintf(g_Info.DeviceNo,"%.*s",t[i + 1].end - t[i + 1].start,jsonString + t[i + 1].start); 
            if(strlen(g_Info.DeviceNo) == 0) strcpy(g_Info.DeviceNo,"IoT Device");
        }
        else if (jsoneq(jsonString, &t[i], "target") == 0)
        {    
            sprintf(buff,"%.*s",t[i + 1].end - t[i + 1].start,jsonString + t[i + 1].start); 
            setTarget(atoi(buff));   
            g_bTargetReceived = TRUE;
            g_lastSendTime = RTC_GetUnixTime();
        }
        else if (jsoneq(jsonString, &t[i], "req_interval") == 0)
        {                        
            sprintf(buff,"%.*s",t[i + 1].end - t[i + 1].start,jsonString + t[i + 1].start);
            g_Info.AndonListRequestTime = atoi(buff);
            
            setCountMax_1ms(g_index_request_interval, g_Info.AndonListRequestTime * 1000);
            g_bReceivedAndonStart = TRUE;
        }
    
        /*else if(jsoneq(jsonString, &t[i], "install_type") == 0)
        {
            sprintf(g_Info.InstallType,"%.*s",t[i + 1].end - t[i + 1].start,jsonString + t[i + 1].start);            
        }
        else if(jsoneq(jsonString, &t[i], "line_name") == 0)
        {
            sprintf(g_Info.LineName,"%.*s",t[i + 1].end - t[i + 1].start,jsonString + t[i + 1].start);            
        }              
        else if(jsoneq(jsonString, &t[i], "device_idx") == 0)
        {
            char buff[20];
            sprintf(buff,"%.*s",t[i + 1].end - t[i + 1].start,jsonString + t[i + 1].start);
            g_Info.DeviceIdx = atoi(buff);
        }*/
    }
    
    //printf("andonGetDeviceName() : %s, %s, %d\r\n",g_Info.DeviceNo,g_Info.InstallType,g_Info.DeviceIdx);
    SetTitleBarText(g_Info.DeviceNo);
    DrawTitle();
    
    if(isInTopMenu()) reflashMenu();
    //g_TopMenuNode->func(NULL,REFLASH);

    g_Info.bUpdateDeviceInfo = TRUE;
    
    return TRUE;
}

uint8 andonRequestItem(char *jsonString, int16 sizeOfJson)
{
    int i;
    int r;
    jsmn_parser p;
    jsmntok_t t[80]; /* 최대 토큰 수: 80 (10개 항목 JSON = 73 tokens, 여유 7) */
        
    jsmn_init(&p);
    r = jsmn_parse(&p, jsonString, sizeOfJson, t, sizeof(t) / sizeof(t[0]));
    
    if (r < 0) {
        #ifdef DEBUG_ANDON_JSON
            printf("Failed to parse JSON: %d\r\n", r);
        #endif
        return FALSE;
    }
    
    /* Assume the top-level element is an object */
    if (r < 1 || t[0].type != JSMN_OBJECT) {
        #ifdef DEBUG_ANDON_JSON
            printf("Object expected type: %d\r\n", t[0].type);
        #endif     
        return FALSE;
    }
    char buff[20];
    for (i = 1; i < r; i++)
    {   
    }
 
    return TRUE;
}
uint8 andonMenuParsing(char *jsonString, int16 sizeOfJson)
{
    int i;
    int r;
    jsmn_parser p;
    jsmntok_t t[80]; /* 최대 토큰 수: 80 (10개 항목 JSON = 73 tokens, 여유 7) */
    
    jsmn_init(&p);
   // printf("%s\r\n",jsonString);
    r = jsmn_parse(&p, jsonString, sizeOfJson, t, sizeof(t) / sizeof(t[0]));
    
    
    if (r < 0) {
        #ifdef DEBUG_ANDON_JSON        
            printf("Failed to parse JSON: %d\r\n", r);
        #endif        
        return FALSE;
    }
    
    /* Assume the top-level element is an object */
    if (r < 1 || t[0].type != JSMN_OBJECT) {
        #ifdef DEBUG_ANDON_JSON        
            printf("Object expected type: %d , %d\r\n", t[0].type, r);
        #endif              
        return FALSE;
    }

    g_AndonLists.bUpdated = FALSE;    
    char tmpBuff[20], textBuff[20], status[10];    
    int16 index=-1, idx, incr=1;
    ANDON_LIST_ITEM *item;
    
    for(i=0; i < t[0].size; i++)
    {
        if(incr >= r) break;
        
        incr++;
            
        if(t[incr].type == JSMN_ARRAY)
        {
            int arraySize = t[incr].size;

            for(int j=0; j < arraySize; j++)
            {
                incr++;        
                     
                if(t[incr].type == JSMN_OBJECT)
                {
                    int objSize = t[incr].size;
                
                    item = &g_AndonLists.item[++index];  
                    for(int k=0; k < objSize; k++)
                    {    
                        incr++;

                        //if (jsoneq(jsonString, &t[incr], "andon_text_idx") == 0)
                        if (jsoneq(jsonString, &t[incr], "andon_idx") == 0)
                        {
                            incr++;   
                            sprintf(tmpBuff,"%.*s",t[incr].end - t[incr].start,jsonString + t[incr].start);
                            idx = atoi(tmpBuff);
                             if(item->idx != idx)
                            {
                                item->bUpdated = TRUE;
                                g_AndonLists.bUpdated = TRUE; 
                            }
                            item->idx = idx;
                        }
                        //else if (jsoneq(jsonString, &t[incr], "andon_text") == 0)
                        else if (jsoneq(jsonString, &t[incr], "andon_name") == 0)
                        {
                            incr++;                               
                            sprintf(textBuff,"%.*s",t[incr].end - t[incr].start,jsonString + t[incr].start);
                            
                            if(strcmp(item->text, textBuff) != 0)
                            {
                                item->bUpdated = TRUE;
                                g_AndonLists.bUpdated = TRUE; 
                            }
                            
                            strcpy(item->text, textBuff);

                            g_uReplyResult = RR_SUCCESS;                
                        }
                        else if (jsoneq(jsonString, &t[incr], "not_completed_qty") == 0)
                        {
                            incr++;                               
                            sprintf(textBuff,"%.*s",t[incr].end - t[incr].start,jsonString + t[incr].start);
                            int16_t not_completed_qty = atoi(textBuff);
                            if(item->not_completed_qty !=  atoi(textBuff) )
                            {
                                item->bUpdated = TRUE;
                                g_AndonLists.bUpdated = TRUE; 
                            }
                            
                            item->not_completed_qty = not_completed_qty;
                            
                            g_uReplyResult = RR_SUCCESS;                
                        }
 
                        else if (jsoneq(jsonString, &t[incr], "warning_blink") == 0)
                        {
                            incr++;                               
                            sprintf(textBuff,"%.*s",t[incr].end - t[incr].start,jsonString + t[incr].start);
                            
                            if(strcmp(item->text, textBuff) != 0) item->bUpdated = TRUE;
                              
                            WarningLightSet(item->idx, item->not_completed_qty > 0 ? TRUE : FALSE, atoi(textBuff));
 
                            g_uReplyResult = RR_SUCCESS;                
                        }
                    }
                }
            }
        }
    }
    
    if( g_AndonLists.noOfList != index+1) g_AndonLists.bUpdated = TRUE; 
    g_AndonLists.noOfList = index+1;

    return TRUE;    
}

uint8 andonRequestTargetParsing(char *jsonString, int16 sizeOfJson)
{
    int i;
    int r;
    jsmn_parser p;
    jsmntok_t t[80]; /* 최대 토큰 수: 80 (10개 항목 JSON = 73 tokens, 여유 7) */
    
    jsmn_init(&p);
    r = jsmn_parse(&p, jsonString, sizeOfJson, t, sizeof(t) / sizeof(t[0]));
  
    if (r < 0) {
        #ifdef DEBUG_ANDON_JSON
            printf("Failed to parse JSON: %d\r\n", r);
        #endif
        return FALSE;
    }

    /* Assume the top-level element is an object */
    if (r < 1 || t[0].type != JSMN_OBJECT) {
        #ifdef DEBUG_ANDON_JSON
            printf("Object expected type: %d\r\n", t[0].type);
        #endif     
        return FALSE;
    }
 
    char buff[256];
    for (i = 1; i < r; i++)
    {   
        if     (jsoneq(jsonString, &t[i], "code") == 0)
        {
            sprintf(buff,"%.*s",t[i + 1].end - t[i + 1].start,jsonString + t[i + 1].start);      
            if(strcmp(buff,"00") != 0)
            {
                return FALSE;
            }
        }
        else  if(jsoneq(jsonString, &t[i], "target") == 0)
        {
            sprintf(buff,"%.*s",t[i + 1].end - t[i + 1].start,jsonString + t[i + 1].start); 
            setTarget(atoi(buff));   
            g_bTargetReceived = TRUE;
            g_lastSendTime = RTC_GetUnixTime();
        }
    }
    
    return TRUE;    
}

uint8 andonTextListParsing(char *jsonString, int16 sizeOfJson)
{
    int i;
    int r;
    jsmn_parser p;
    jsmntok_t t[80]; /* 최대 토큰 수: 80 (10개 항목 JSON = 73 tokens, 여유 7) */
    
    jsmn_init(&p);
    
    r = jsmn_parse(&p, jsonString, sizeOfJson, t, sizeof(t) / sizeof(t[0]));
    
     printf("%s\r\n",jsonString);
    
//    if (r < 0) {
//        DEBUG_printf("Failed to parse JSON: %d\r\n", r);
//        return FALSE;
//    }
//    
//    /* Assume the top-level element is an object */
//    if (r < 1 || t[0].type != JSMN_OBJECT) {
//        DEBUG_printf("Object expected type: %d , %d\r\n", t[0].type, r);
//        return FALSE;
//    }
//
//    int oldReceivedCount = getReceivedMessageCount(), newReceivedCount;
//    
//    g_noMessage = 0;
//    g_ListAlarm = FALSE;
//    g_bTextListUpdated = TRUE;
//   // char tmpBuff[20], textBuff[20], status[10];  
//    
//    char buff[10];
//    int16 index=-1, idx, incr=1;
//
//    MESSAGE *item=NULL;
// 
//    for(i=0; i < t[0].size; i++)
//    {
//        if(incr >= r) break;
//        
//        incr++;
//            
//        if(t[incr].type == JSMN_ARRAY)
//        {
//            int arraySize = t[incr].size;
//
//            for(int j=0; j < arraySize; j++)
//            {
//                incr++;        
//                     
//                if(t[incr].type == JSMN_OBJECT)
//                {
//                    int objSize = t[incr].size;
//                                    
//                    item = &g_Message[++index];
//                    item->isRead=FALSE;
//                    if(index >= MAX_MESSAGE)
//                    {
//                        index--;
//                        break;
//                    }
//                    
//                    for(int k=0; k < objSize; k++)
//                    {    
//                        incr++;
//                        if (jsoneq(jsonString, &t[incr], "text_idx") == 0)
//                        {
//                            incr++;   
//                            sprintf(item->text_idx,"%.*s",t[incr].end - t[incr].start,jsonString + t[incr].start);
//                        }
//                        if (jsoneq(jsonString, &t[incr], "type") == 0)
//                        {
//                            incr++;   
//                            sprintf(item->type,"%.*s",t[incr].end - t[incr].start,jsonString + t[incr].start);
//                        }                        
//                        else if (jsoneq(jsonString, &t[incr], "date") == 0)
//                        {
//                            incr++;   
//                            sprintf(item->date,"%.*s",t[incr].end - t[incr].start,jsonString + t[incr].start);           
//                        }
//                        else if (jsoneq(jsonString, &t[incr], "time") == 0)
//                        {
//                            incr++;   
//                            sprintf(item->time,"%.*s",t[incr].end - t[incr].start,jsonString + t[incr].start);           
//                        }
//                        else if (jsoneq(jsonString, &t[incr], "read_yn") == 0)
//                        {
//                            incr++;   
//                            
//                            sprintf(buff,"%.*s",t[incr].end - t[incr].start,jsonString + t[incr].start);
//                            item->isRead = TRUE;  
//                            
//                            if(strcmp(buff,"N")==0 && strcmp(item->type,"R") == 0)
//                            {
//                                item->isRead = FALSE;
//                                g_ListAlarm = TRUE;
//                            }
//                        }                        
//                        else
//                        {
//                            g_uReplyResult = RR_FAILURE;
//                          //  DEBUG_printf("JSON_MenuParsing()>> %s\r\n",g_JSONString);
//                        }
//                    }
//                }
//            }
//        }
//    }
//    
//    newReceivedCount = getReceivedMessageCount();
//    if(g_noMessage != newReceivedCount)  g_bLED1_Flickering = TRUE;
//
//    g_noMessage = (uint16) (index+1);

    return TRUE;    
}
/* [] END OF FILE */
