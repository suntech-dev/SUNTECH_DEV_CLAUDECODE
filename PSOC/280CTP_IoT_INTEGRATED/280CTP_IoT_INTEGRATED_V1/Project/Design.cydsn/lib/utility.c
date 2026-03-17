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
#include "utility.h"
#include  <ctype.h>

int HexToDec(char c)
{
    int u = toupper ((int) c);
    if(u < 'A') return u-'0';
    return u-'A' + 10;
}

char hexToChar(char *str)
{
    int a = HexToDec(str[0]);
    int b = HexToDec(str[1]);
    int c = a << 4 | b;
    
    return (char) c;
}

uint8 convertHexToDecimal(uint8 hex)
{
    if     (hex  <= '9') return hex-'0';
    else if(hex  <= 'F') return (hex-'A') + 10;    
    else if(hex  <= 'f') return (hex-'a') + 10;
    return 0;
}

char *url_encode(const char *src) {
    static char ret[512];
    int len = strlen(src);
    int idx=0;
    for(int i=0; i < len; i++)
    {
        char c =src[i];
        if(c==' ' || c == '#')
        {
            ret[idx++]='%';
            ret[idx++]='2';
            ret[idx++]='0';             
        }
        else
            ret[idx++] = c;
    }
    ret[idx] = 0;
    return ret;
}

/*
char *getReplaceBlank(char *text)
{
    static char ret[512];
    int len = strlen(text);
    int idx=0;
    for(int i=0; i < len; i++)
    {
        char c =text[i];
        if(c==' ')
        {
            ret[idx++]='%';
            ret[idx++]='2';
            ret[idx++]='0';             
        }
        else
            ret[idx++] = c;
    }
    ret[idx] = 0;
    return ret;
}
*/

/* [] END OF FILE */
