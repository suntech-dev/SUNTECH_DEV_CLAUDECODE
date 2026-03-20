/* ========================================
 *
 * Copyright Suntech, 2023.04.16
 * All Rights Reserved
 * UNPUBLISHED, LICENSED SOFTWARE.
 *
 * CONFIDENTIAL AND PROPRIETARY INFORMATION
 * WHICH IS THE PROPERTY OF Suntech.
 *
 * ========================================
*/
#include "uartJson.h"
#include "jsonUtil.h"
#include "count.h"
#include "andonApi.h"
#include "userProjectPatternSewing.h"
#include "lib/internalFlash.h"
#include "lib/widget.h"
#include "downtime.h"

#define UART_BUFFER_SIZE 512

static char g_UART_buff[UART_BUFFER_SIZE];  /* uartJson.c 내부 전용 */
static int  g_UART_buff_index = 0;

char uartJsonParsor();

uint8 uartJsonLoop()
{    
   COUNT *ptrCount = getCount();   
    while(UART_SpiUartGetRxBufferSize() > 0 )
    {
        char c = UART_UartGetChar();

        if(c == '\0') continue;
        
        if(g_UART_buff_index==0)
        {
            if(!(c == '{' || c == '[')) continue;
        }
        
        if(g_UART_buff_index >= UART_BUFFER_SIZE - 1) { g_UART_buff_index = 0; continue; }
        g_UART_buff[g_UART_buff_index++] = c;
        if(c == '}')
        {
            g_UART_buff[g_UART_buff_index] = '\0';
                
            if(uartJsonParsor())
            {                        
                while(ptrCount->patternCount >= g_ptrMachineParameter->patternPairCount)
                {
                    ptrCount->patternCount -= g_ptrMachineParameter->patternPairCount;
                    
                    ADD_CONVERT_TO_4BYTE(ptrCount->patternActualH,ptrCount->patternActualL,g_ptrMachineParameter->patternPair);
                    uint32 tmp = CONVERT_TO_4BYTE(ptrCount->patternActualH, ptrCount->patternActualL) + g_ptrMachineParameter->patternPair;
                }
    
                Reg_Pattern_Write(1);
                
                g_updateCountMenu = TRUE;     
                makeAndonPatternCount();
               
                ForcefullyMarkDowntimeAsComplete();
               
                SaveInternalFlash();
                
                CyDelay(50);
                Reg_Pattern_Write(0);
                
                g_UART_buff_index = 0;
                return TRUE;
            }
                        
            g_UART_buff_index = 0;
            memset(g_UART_buff,0,UART_BUFFER_SIZE);            
        }        
    }
    return FALSE;
}

char uartJsonParsor()
{
    int i;
    int r;
    jsmn_parser p;
    jsmntok_t t[80]; /* 최대 토큰 수: 80 (10개 항목 JSON = 73 tokens, 여유 7) */
    
    jsmn_init(&p);
    r = jsmn_parse(&p, g_UART_buff, strlen(g_UART_buff), t, sizeof(t) / sizeof(t[0]));
    
    if (r < 0) {
    //    DEBUG_printf("Failed to parse JSON: %d\r\n", r);
        return FALSE;
    }
    
    /* Assume the top-level element is an object */
    if (r < 1 || t[0].type != JSMN_OBJECT) {
    //    DEBUG_printf("Object expected type: %d\r\n", t[0].type);
        return FALSE;
    }

//    uint16 len = strlen(g_UART_buff);
//
//    int16 x = 0;
//    int16 y = GetBodyArea().top+ 40;
//    
//    for(uint16 i=0; i < len; i++)
//    {
//        char c =  g_UART_buff[i];
//        printf("%c",c);
//        if(c == ' ' || c == '\r' || c == '\n') continue;
//
//        if(x >= g_SCREEN_WIDTH - 10)
//        {
//            x = 0;
//            y += 20;
//        }
//        if(y > GetBodyArea().bottom + 20) y = GetBodyArea().top;
//        
//        LCD_DrawFont(x,y,WHITE,BLACK,SmallFont8x12,c);
//        x += 10;
//    }
    COUNT *ptrCount = getCount();   
    char buff[100];
    
    for (i = 1; i < r; i++)
    {   
        if     (jsoneq(g_UART_buff, &t[i], "cmd") == 0)
        {
            sprintf(buff,"%.*s",t[i + 1].end - t[i + 1].start,g_UART_buff + t[i + 1].start);

            if     (strcmp(buff,"count") != 0) return FALSE;
        }    
        else if     (jsoneq(g_UART_buff, &t[i], "value") == 0)
        {      
            sprintf(buff,"%.*s",t[i + 1].end - t[i + 1].start,g_UART_buff + t[i + 1].start);
            
            ptrCount->patternCount += atoi(buff);
        }         
        else if     (jsoneq(g_UART_buff, &t[i], "no") == 0)
        {      
            sprintf(buff,"%.*s",t[i + 1].end - t[i + 1].start,g_UART_buff + t[i + 1].start);
            
            ptrCount->patternNo = atoi(buff);
        }           
        else if     (jsoneq(g_UART_buff, &t[i], "ct") == 0)
        {      
            sprintf(buff,"%.*s",t[i + 1].end - t[i + 1].start,g_UART_buff + t[i + 1].start);
            
            ptrCount->patternCycleTime = atoi(buff);
            
//       LCD_printf(0,50, WHITE, BLACK, Grotesk16x32, "CT:%u", g_ptrCount->patternCycleTime);   
//        LCD_printf(0,80, WHITE, BLACK, Grotesk16x32, "%s", buff);   
        
            ADD_CONVERT_TO_4BYTE(ptrCount->patternCycleTimeSumH,ptrCount->patternCycleTimeSumL,ptrCount->patternCycleTime);
        }
        else if     (jsoneq(g_UART_buff, &t[i], "et") == 0)
        {     
            sprintf(buff,"%.*s",t[i + 1].end - t[i + 1].start,g_UART_buff + t[i + 1].start);
            
            ptrCount->patternEmergencyTime = atoi(buff);
            ADD_CONVERT_TO_4BYTE(ptrCount->patternEmergencyTimeSumH,ptrCount->patternEmergencyTimeSumL,ptrCount->patternEmergencyTime);
        }            
        else if     (jsoneq(g_UART_buff, &t[i], "mrt") == 0)
        {     
            sprintf(buff,"%.*s",t[i + 1].end - t[i + 1].start,g_UART_buff + t[i + 1].start);
            
            ptrCount->patternMotorRunTime = atoi(buff);
            ADD_CONVERT_TO_4BYTE(ptrCount->patternMotorRunTimeSumH,ptrCount->patternMotorRunTimeSumL,ptrCount->patternMotorRunTime);
        }        
        else if     (jsoneq(g_UART_buff, &t[i], "sq") == 0)
        {     
            sprintf(buff,"%.*s",t[i + 1].end - t[i + 1].start,g_UART_buff + t[i + 1].start);
            
            ptrCount->patternNoStitch = atoi(buff);
            ADD_CONVERT_TO_4BYTE(ptrCount->patternNoStitchSumH,ptrCount->patternNoStitchSumL,ptrCount->patternNoStitch);            
        }
        else if     (jsoneq(g_UART_buff, &t[i], "sl") == 0)
        {     
            sprintf(buff,"%.*s",t[i + 1].end - t[i + 1].start,g_UART_buff + t[i + 1].start);
            
            ptrCount->patternStitchLength = atoi(buff);
            
//        LCD_printf(0,150, WHITE, BLACK, Grotesk16x32, "SL:%u", g_ptrCount->patternStitchLength);   
//        LCD_printf(0,180, WHITE, BLACK, Grotesk16x32, "%s", buff);   
        
                      //  printf("--->%s : %u\r\n",buff,g_ptrCount->patternStitchLength);
            ADD_CONVERT_TO_4BYTE(ptrCount->patternStitchLengthSumH,ptrCount->patternStitchLengthSumL,ptrCount->patternStitchLength);            
        }
        else if     (jsoneq(g_UART_buff, &t[i], "tc") == 0)
        {     
            sprintf(buff,"%.*s",t[i + 1].end - t[i + 1].start,g_UART_buff + t[i + 1].start);
            
            ptrCount->patternTrimCount = atoi(buff);
            
            ADD_CONVERT_TO_4BYTE(ptrCount->patternTrimCountSumH,ptrCount->patternTrimCountSumL,ptrCount->patternTrimCount);                
        }           
        else if     (jsoneq(g_UART_buff, &t[i], "spm") == 0)
        {     
            sprintf(buff,"%.*s",t[i + 1].end - t[i + 1].start,g_UART_buff + t[i + 1].start);
            ptrCount->patternSPM = atoi(buff);
        }

    }
    return TRUE;
}
/* [] END OF FILE */
