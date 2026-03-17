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
#include "jsonUtil.h"
#include "andonApi.h"    /* RR_SUCCESS enum */
#include "andonJson.h"   /* g_uReplyResult */

char* replaceWord(const char* s, const char* oldW,
                const char* newW)
{
    char* result;
    int i, cnt = 0;
    int newWlen = strlen(newW);
    int oldWlen = strlen(oldW);
 
    // Counting the number of times old word
    // occur in the string
    for (i = 0; s[i] != '\0'; i++) {
        if (strstr(&s[i], oldW) == &s[i]) {
            cnt++;
 
            // Jumping to index after the old word.
            i += oldWlen - 1;
        }
    }
 
    // Making new string of enough length
    result = (char*)malloc(i + cnt * (newWlen - oldWlen) + 1);
 
    i = 0;
    while (*s) {
        // compare the substring with the result
        if (strstr(s, oldW) == s) {
            strcpy(&result[i], newW);
            i += newWlen;
            s += oldWlen;
        }
        else
            result[i++] = *s++;
    }
 
    result[i] = '\0';
    return result;
}

void removeBackSlash(char *str)
{
    int len = strlen(str);
    
    for(int i=0; i < len-1; i++)
    {
        int j=i+1;
        if(str[i] == '\\' && (str[j] == '/' || str[j] == '"' || str[j] == '\'') )
        {
            for(int k=i; k < len-1; k++) str[k] = str[k+1];
            str[len--] = '\0';
            len--;
        }
    }
}

void removeWhiteSpace(char *str)
{
    uint8 nCheck = TRUE;
    while(nCheck)
    {
        nCheck = FALSE;
        unsigned int len = strlen(str);
        for(unsigned int i=0; i < len; i++)
        {
            char c = str[i];
            if(c == ' ' || c == '\t' || c == '\r' || c == '\n')
            {
                for(unsigned int j=i; j < len-1; j++) str[j] = str[j+1];
                str[len-1] = '\0';
                 nCheck = TRUE;
            }
        }
    }    
}

void removeDoubleSlash(char *str)
{
    uint8 nCheck = TRUE;
    while(nCheck)
    {
        nCheck = FALSE;
        unsigned int len = strlen(str);
        for(unsigned int i=0; i < len-1; i++)
        {
            char c = str[i];
            if(str[i] == '/' && str[i+1] == '/')
            {
                for(unsigned int j=i; j < len-1; j++) str[j] = str[j+1];
                str[len-1] = '\0';
                nCheck = TRUE;
            }
        }
    }    
}

void refineIPAddress(char *str) // 주소가 아닌 것들을 삭제 한다.
{
    uint8 nCheck = TRUE;
    while(nCheck)
    {
        nCheck = FALSE;
        unsigned int len = strlen(str);
        for(unsigned int i=0; i < len-1; i++)
        {
            char c = str[i];
            if(!((c >= '0' && c <= '9') || c == '.'))
            {
                for(unsigned int j=i; j < len-1; j++) str[j] = str[j+1];
                str[len-1] = '\0';
                nCheck = TRUE;
            }
        }
    }   
}

char *GetJSON_Token_String(jsmntok_t *tok, const char * g_JSONString)
{
    static char buff[1024];
    
    sprintf(buff,"%.*s",tok->end - tok->start, g_JSONString + tok->start);
    
    return buff;
}
/* =====================================================================
 * parseGenericList() — downtime/defective 공통 JSON 파서
 * idxKey  : "downtime_idx"  또는 "defective_idx"
 * nameKey : "downtime_name" 또는 "defective_name"
 * ===================================================================== */
uint8 parseGenericList(char *jsonString, int16 sizeOfJson,
                       GENERIC_LISTS *lists,
                       const char *idxKey, const char *nameKey)
{
    int i, r;
    jsmn_parser p;
    jsmntok_t t[80];  /* 최대 80 토큰 (10항목 기준 73개, 여유 7) */

    jsmn_init(&p);
    r = jsmn_parse(&p, jsonString, sizeOfJson, t, sizeof(t) / sizeof(t[0]));

    if (r < 0) {
        #ifdef DEBUG_ANDON_JSON
            printf("Failed to parse JSON: %d\r\n", r);
        #endif
        return FALSE;
    }
    if (r < 1 || t[0].type != JSMN_OBJECT) {
        #ifdef DEBUG_ANDON_JSON
            printf("Object expected type: %d , %d\r\n", t[0].type, r);
        #endif
        return FALSE;
    }

    lists->bUpdated = FALSE;
    char tmpBuff[20], textBuff[20];
    int16 index = -1, idx, incr = 1;
    GENERIC_LIST_ITEM *item;

    for (i = 0; i < t[0].size; i++)
    {
        if (incr >= r) break;
        incr++;

        if (t[incr].type == JSMN_ARRAY)
        {
            int arraySize = t[incr].size;

            for (int j = 0; j < arraySize; j++)
            {
                incr++;
                if (t[incr].type == JSMN_OBJECT)
                {
                    int objSize = t[incr].size;
                    item = &lists->item[++index];

                    for (int k = 0; k < objSize; k++)
                    {
                        incr++;
                        if (jsoneq(jsonString, &t[incr], idxKey) == 0)
                        {
                            incr++;
                            sprintf(tmpBuff, "%.*s", t[incr].end - t[incr].start, jsonString + t[incr].start);
                            idx = atoi(tmpBuff);
                            if (item->idx != idx) {
                                item->bUpdated   = TRUE;
                                lists->bUpdated  = TRUE;
                            }
                            item->idx = idx;
                        }
                        else if (jsoneq(jsonString, &t[incr], nameKey) == 0)
                        {
                            incr++;
                            sprintf(textBuff, "%.*s", t[incr].end - t[incr].start, jsonString + t[incr].start);
                            if (strcmp(item->text, textBuff) != 0) {
                                item->bUpdated   = TRUE;
                                lists->bUpdated  = TRUE;
                            }
                            strcpy(item->text, textBuff);
                            g_uReplyResult = RR_SUCCESS;
                        }
                        else if (jsoneq(jsonString, &t[incr], "not_completed_qty") == 0)
                        {
                            incr++;
                            sprintf(textBuff, "%.*s", t[incr].end - t[incr].start, jsonString + t[incr].start);
                            int16_t qty = atoi(textBuff);
                            item->not_completed_qty = qty;
                            if (item->not_completed_qty != atoi(textBuff)) {
                                item->bUpdated   = TRUE;
                                lists->bUpdated  = TRUE;
                                printf(">>> Update...%d-%d\r\n", index, qty);
                            }
                            g_uReplyResult = RR_SUCCESS;
                        }
                    }
                }
            }
        }
    }

    if (lists->noOfList != (uint8)(index + 1)) lists->bUpdated = TRUE;
    lists->bUpdated  = TRUE;
    lists->noOfList  = (uint8)(index + 1);
    return TRUE;
}
/* [] END OF FILE */
