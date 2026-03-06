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
#include "main.h"

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
/* [] END OF FILE */
