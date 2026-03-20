/* ========================================
 *
 * Copyright YOUR COMPANY, THE YEAR
 * All Rights Reserved
 * UNPUBLISHED, LICENSED SOFTWARE.
 *
 * CONFIDENTIAL AND PROPRIETARY INFORMATION
 * WHICH IS THE PROPERTY OF your company.
 *
 * ========================================
*/
#ifndef _JSON_UTIL_H_
#define _JSON_UTIL_H_

#include "lib/jsmn.h"
#include "main.h"

#define READ_JSON_VALUE(buff,index) sprintf(buff,"%.*s",t[index + 1].end - t[index + 1].start,g_JSONString + t[index + 1].start);

char* replaceWord(const char* s, const char* oldW, const char* newW);
void removeBackSlash(char *str);
void refineIPAddress(char *str); // 주소가 아닌 것들을 삭제 한다.
void removeWhiteSpace(char *str); // white space를 제거한다.
void removeDoubleSlash(char *str);
char *GetJSON_Token_String(jsmntok_t *tok, const char * g_JSONString);

/* =====================================================================
 * 공통 리스트 JSON 파서 (downtime/defective 공유)
 * downtime.h / defective.h 의 *_LIST_ITEM, *_LISTS 과 레이아웃 동일
 * MAX = 20 : MAX_DOWNTIME_LIST == MAX_DEFECTIVE_LIST == 20
 * ===================================================================== */
#define MAX_GENERIC_LIST 20

typedef struct {
    uint8 bUpdated;
    uint8 status;
    char  text[32];
    int16 idx;
    int16 not_completed_qty;
} GENERIC_LIST_ITEM;

typedef struct {
    uint8 bUpdated;
    uint8 noOfList;
    uint8 uSelectIndex;
    GENERIC_LIST_ITEM item[MAX_GENERIC_LIST];
} GENERIC_LISTS;

uint8 parseGenericList(char *jsonString, int16 sizeOfJson,
                       GENERIC_LISTS *lists,
                       const char *idxKey, const char *nameKey);

#endif    
/* [] END OF FILE */
