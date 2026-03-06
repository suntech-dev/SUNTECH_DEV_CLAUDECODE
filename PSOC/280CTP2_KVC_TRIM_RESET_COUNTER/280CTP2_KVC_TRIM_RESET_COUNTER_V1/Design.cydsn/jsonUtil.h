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
    
#define READ_JSON_VALUE(buff,index) sprintf(buff,"%.*s",t[index + 1].end - t[index + 1].start,g_JSONString + t[index + 1].start);
    
char* replaceWord(const char* s, const char* oldW, const char* newW);
void removeBackSlash(char *str);
void refineIPAddress(char *str); // 주소가 아닌 것들을 삭제 한다.
void removeWhiteSpace(char *str); // white space를 제거한다.
void removeDoubleSlash(char *str);
char *GetJSON_Token_String(jsmntok_t *tok, const char * g_JSONString);

#endif    
/* [] END OF FILE */
