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
#include "lib/RealTimeClock.h"
#include "lib/widget.h"

uint8 andonCurrentTimeParsing(char *jsonString, int16 sizeOfJson);
uint8 andonGetDeviceName     (char *jsonString, int16 sizeOfJson);
uint8 andonSendCount         (char *jsonString, int16 sizeOfJson);
uint8 andonMenuParsing       (char *jsonString, int16 sizeOfJson);
   
void andonJsonParsor(int type, char *jsonString, int16 sizeOfJson)
{
    switch(type)
    {
        case ANDON_CURRENT_TIME:       andonCurrentTimeParsing(jsonString, sizeOfJson); break;
        case ANDON_GET_DEVICE_NAME:    andonGetDeviceName     (jsonString, sizeOfJson); break;
        case ANDON_SEND_SEWING_COUNT:
        case ANDON_SEND_PATTERN_COUNT: andonSendCount         (jsonString, sizeOfJson); break;        
        case ANDON_REQUEST_LIST:       andonMenuParsing       (jsonString, sizeOfJson); break;
        case ANDON_REQUEST_TEXT_LIST:  andonMenuParsing       (jsonString, sizeOfJson); break;            
    }
}

uint8 andonCurrentTimeParsing(char *jsonString, int16 sizeOfJson)
{
    int i;    
    int r;
    jsmn_parser p;
    jsmntok_t t[128]; /* We expect no more than 128 tokens */  
    
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
            setCurrentTime(GetJSON_Token_String(&t[i + 1], jsonString));
            g_uReplyResult = RR_SUCCESS;
            break;
        }
    }
    
    return TRUE;
}

uint8 andonGetDeviceName(char *jsonString, int16 sizeOfJson)
{
    int i;
    int r;
    jsmn_parser p;
    jsmntok_t t[128]; /* We expect no more than 128 tokens */  
    
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
        if     (jsoneq(jsonString, &t[i], "device_no") == 0)
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
        else if(jsoneq(jsonString, &t[i], "device_idx") == 0)
        {
            char buff[20];
            sprintf(buff,"%.*s",t[i + 1].end - t[i + 1].start,jsonString + t[i + 1].start);
            g_Info.DeviceIdx = atoi(buff);
        }
    }
    
 //   printf("andonGetDeviceName() : %s, %s, %d\r\n",g_Info.DeviceNo,g_Info.InstallType,g_Info.DeviceIdx);
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
    jsmntok_t t[128]; /* We expect no more than 128 tokens */  
    
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
    
 //   printf("andonSendCount():%s",jsonString);
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

uint8 andonMenuParsing(char *jsonString, int16 sizeOfJson)
{
    int i;
    int r;
    jsmn_parser p;
    jsmntok_t t[128]; /* We expect no more than 128 tokens */  
    
    jsmn_init(&p);
    printf("%s\r\n",jsonString);
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

    g_Lists.bUpdated = FALSE;    
    char tmpBuff[20], textBuff[20], status[10];    
    int16 index=-1, idx, incr=1;
    LIST_ITEM *item;
    
    uint8 bLEDAlarm = FALSE;
           
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
                
                    item = &g_Lists.item[++index];  
                    for(int k=0; k < objSize; k++)
                    {    
                        incr++;
                        if (jsoneq(jsonString, &t[incr], "idx") == 0)
                        {
                            incr++;   
                            sprintf(tmpBuff,"%.*s",t[incr].end - t[incr].start,jsonString + t[incr].start);
                            idx = atoi(tmpBuff);
                             if(item->idx != idx) item->bUpdated = TRUE; 
                            item->idx = idx;
                        }
                        else if (jsoneq(jsonString, &t[incr], "text") == 0)
                        {
                            incr++;                               
                            sprintf(textBuff,"%.*s",t[incr].end - t[incr].start,jsonString + t[incr].start);
                            
                            if(strcmp(item->text, textBuff) != 0) item->bUpdated = TRUE;
                            strcpy(item->text, textBuff);
                            
                            g_uReplyResult = RR_SUCCESS;                
                        }
                        else if (jsoneq(jsonString, &t[incr], "not_completed_qty") == 0)
                        {
                            incr++;                               
                            sprintf(textBuff,"%.*s",t[incr].end - t[incr].start,jsonString + t[incr].start);
                            
                            if(strcmp(item->text, textBuff) != 0) item->bUpdated = TRUE;
                            
                            item->not_completed_qty = atoi(GetJSON_Token_String(&t[incr], jsonString));
                 
                            g_uReplyResult = RR_SUCCESS;                
                        }                        
//                        else if(jsoneq(g_JSONString, &t[incr], "process_no") == 0)
//                        {
//                            incr++;                                 
//                            item->process_no = atoi(GetJSON_Token_String(&t[incr], g_JSONString));
//                        }             
                        else if     (jsoneq(jsonString, &t[incr], "status") == 0)
                        {
                            incr++;                               
                            sprintf(status,"%.*s",t[incr].end - t[incr].start,jsonString + t[incr].start);
                          //  item->uStat = LIS_INIT;
                            
                            if(strcmp("C",status)==0 || strlen(status) == 0)
                            {
                                if(item->status != LIS_C)
                                {
                                    item->status = LIS_C;
                                    item->bUpdated = TRUE;
                                }
                            }
                            else if(strcmp("P",status)==0)
                            {
                                if(item->status != LIS_P)
                                {
                                    item->status = LIS_P;
                                    item->bUpdated = TRUE;
                                }
                                bLEDAlarm = TRUE;
                            }
                            else if(strcmp("W",status)==0)
                            {
                                if(item->status != LIS_W)
                                {
                                    item->status = LIS_W;
                                    item->bUpdated = TRUE;
                                }
                                bLEDAlarm = TRUE;                    
                            }
                            else
                            {
                                g_uReplyResult = RR_FAILURE;
                              //  DEBUG_printf("JSON_MenuParsing()>> %s\r\n",g_JSONString);
                            }
                        }
                    }
                }
            }
        }
        else if(t[incr].type == JSMN_STRING)
        {      
            if (jsoneq(jsonString, &t[incr], "andon_list_request_interval") == 0)
            {
                incr++;                               
                sprintf(tmpBuff,"%.*s",t[incr].end - t[incr].start,jsonString + t[incr].start);
                
                g_Info.AndonListRequestTime = atoi(tmpBuff);  
                
               // g_Info.AndonListRequestTime = 10;
            }
            else if (jsoneq(jsonString, &t[incr], "notice_index") == 0)
            {
                incr++;                               
                sprintf(tmpBuff,"%.*s",t[incr].end - t[incr].start,jsonString + t[incr].start);
                
                g_Info.notice_index = atoi(tmpBuff);               
            } else {
                incr++;                 
            }
        }     
    
    }
    
    
//    if(index != -1) g_bRecivedLists = TRUE;
//        
//    if( g_Lists.noOfList != index+1) g_Lists.bUpdated = TRUE; 
//    g_Lists.noOfList = index+1;
//
//    if(bLEDAlarm)
//    {
//        g_uLED2_Color      = LED_RED;
//        g_bLED2_Flickering =    TRUE;        
//    }
//    else 
//    {
//        g_uLED2_Color      = LED_BLUE;
//        g_bLED2_Flickering =    FALSE;             
//    }
//  //  if(g_Lists.updated) initLists();
    
    return TRUE;    
}

uint8 andonTextListParsing(char *jsonString, int16 sizeOfJson)
{
    int i;
    int r;
    jsmn_parser p;
    jsmntok_t t[128]; /* We expect no more than 128 tokens */  
    
    jsmn_init(&p);
    
    r = jsmn_parse(&p, jsonString, sizeOfJson, t, sizeof(t) / sizeof(t[0]));
    
     printf("%s\r\n",jsonString);
//    
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
