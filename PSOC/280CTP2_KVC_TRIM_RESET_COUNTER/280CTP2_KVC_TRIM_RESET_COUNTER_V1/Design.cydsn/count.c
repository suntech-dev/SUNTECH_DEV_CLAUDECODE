/* ========================================
 *
 * Copyright Suntech, 2023.04.13
 * All Rights Reserved
 * UNPUBLISHED, LICENSED SOFTWARE.
 *
 * CONFIDENTIAL AND PROPRIETARY INFORMATION
 * WHICH IS THE PROPERTY OF Suntech.
 *
 * ========================================
*/
#include "main.h"
#include "count.h"
#include "lib/internalFlash.h"
#include "userProjectCounter.h"
#include "userProjectPatternSewing.h"
#include "andonApi.h"
#include "uartJson.h"
#include "lib/UI.h"
#include "lib/widget.h"

uint8 g_updateTrimCount=FALSE;
uint8 g_updateCountMenu=FALSE;
//uint16 g_trimCount=0;
COUNT *g_ptrCount = NULL;

#ifdef USER_PROJECT_PATTERN_SEWING_MACHINE
void (*CountFunc)() = &SewingCountLoop;
void PatternCountLoop();
void SewingCountLoop();
#endif


CY_ISR(Trim_Interrupt_Routine)
{
    g_updateTrimCount = TRUE;

    g_ptrCount->count++;
}

void initCount()
{
    if(sizeof(COUNT) > sizeof(g_internalFlash.data))
    {
        printf("Wrong... COUNT size is large than g_internalFlash.data\r\n");        
    }    
    g_ptrCount = (COUNT *) &g_internalFlash.data;  
    
    trim_Int_StartEx(&Trim_Interrupt_Routine); 
    
    if(g_ptrCount->count == g_ptrMachineParameter->setTrimCount)
    {
        g_ptrCount->count = 0;
        RESERVED_OUT_1_Write(0);
    }    
}

#ifdef USER_PROJECT_TRIM_COUNT


void SetCountLoop()
{
    static uint8 bTrimCountComplete = FALSE;
    
    // RESET 버튼은 Target에 도달했을 때만 작동 (부정 리셋 방지)
    if(RESET_KEY_Read() == FALSE && g_ptrCount->count == g_ptrMachineParameter->setTrimCount)
    {
        CyDelay(50);

        if(RESET_KEY_Read() == FALSE)
        {
           //       printf("Reset..\r\n");
            g_ptrCount->count = 0;
            SaveInternalFlash();
            g_updateCountMenu = TRUE;
            bTrimCountComplete = FALSE;
            RESERVED_OUT_1_Write(0);
            Buzzer(BUZZER_STOP, 0);
            return;
        }
    }
    // Target 미달성 시 RESET 버튼을 누르면 LCD 경고 메시지 + 경고음
    else if(RESET_KEY_Read() == FALSE && g_ptrCount->count > 0 && g_ptrCount->count < g_ptrMachineParameter->setTrimCount)
    {
        CyDelay(50);
        if(RESET_KEY_Read() == FALSE)
        {
            // LCD에 경고 메시지 표시 (현재 카운트 / 목표 카운트)
            ShowMessage("Target Not Reached!\n%d/%d", g_ptrCount->count, g_ptrMachineParameter->setTrimCount);
            CyDelay(1500);  // 1.5초 동안 메시지 표시

            // 짧은 경고음으로 Target 미달성 알림
            Buzzer(BUZZER_WARNING, 100);

            // 메뉴 화면 갱신
            g_updateCountMenu = TRUE;
        }
    }
    
    if(bTrimCountComplete == FALSE && g_ptrCount->count == g_ptrMachineParameter->setTrimCount)
    {
        Buzzer(BUZZER_WARNING_CONTINUOUS, 0);
        RESERVED_OUT_1_Write(1);
        bTrimCountComplete = TRUE;
    }
    
    if(g_updateTrimCount == FALSE) return;
 
    g_updateTrimCount = FALSE;
    
    printf(".");   
    if(g_ptrMachineParameter->setTrimCount < g_ptrCount->count)
    {
        g_ptrCount->count = g_ptrMachineParameter->setTrimCount;
    }

    SaveInternalFlash();   
    g_updateCountMenu = TRUE;
}

#endif

#ifdef USER_PROJECT_PATTERN_SEWING_MACHINE  
uint16 g_Count = 0;
CY_ISR(Trim_Interrupt_Routine)
{
    g_updateTrimCount = TRUE;
//    g_trimCount++;
    g_ptrCount->sewingTrimCount++;
    g_Count++;
}


void SewingCountLoop()
{
    if(g_updateTrimCount == FALSE) return;
    
    

    uint8 bSend = FALSE;
 //   uint32 actual =  CONVERT_TO_4BYTE(g_ptrCount->sewingActualH, g_ptrCount->sewingActualL) /10;
    
    ADD_CONVERT_TO_4BYTE(g_ptrCount->sewingTrimCountSumH,g_ptrCount->sewingTrimCountSumL,g_Count); g_Count = 0;
    
 //   printf("\r\n%u %u %lu\r\n",g_ptrCount->sewingTrimCount,g_ptrMachineParameter->sewingPairTrim,(uint32) CONVERT_TO_4BYTE(g_ptrCount->sewingTrimCountSumH,g_ptrCount->sewingTrimCountSumL));
    while(g_ptrCount->sewingTrimCount >= g_ptrMachineParameter->sewingPairTrim)
    {
        g_ptrCount->sewingTrimCount -= g_ptrMachineParameter->sewingPairTrim;
        
        ADD_CONVERT_TO_4BYTE(g_ptrCount->sewingActualH,g_ptrCount->sewingActualL,g_ptrMachineParameter->sewingPair);
        bSend = TRUE;
    }

    if(bSend == TRUE)
    {
        g_updateCountMenu = TRUE;
        makeAndonSewingCount();
        SaveInternalFlash();              
    }
    
//    if(CONVERT_TO_4BYTE(g_ptrCount->sewingActualH, g_ptrCount->sewingActualL)/10 > actual)
//    {
//        g_updateCountMenu = TRUE;
//        makeAndonSewingCount();
//        SaveInternalFlash();              
//    }
 
    g_updateTrimCount = FALSE;        
}

void PatternCountLoop()
{
    if(uartJsonLoop()==TRUE)
    {
        g_updateCountMenu = TRUE;
    }
}
#endif  

/* [] END OF FILE */
