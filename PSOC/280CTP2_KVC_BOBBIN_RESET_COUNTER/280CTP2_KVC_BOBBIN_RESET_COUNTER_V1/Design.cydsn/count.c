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

uint8 g_bTrimCountComplete = FALSE;  // 2024.05.20

void SetCountLoop()
{

    if(RESET_KEY_Read() == FALSE && g_ptrCount->count != 0)
    {
        CyDelay(50);
  
        if(RESET_KEY_Read() == FALSE) 
        {
           //       printf("Reset..\r\n");
            g_ptrCount->count = 0;
            SaveInternalFlash();   
            g_updateCountMenu = TRUE;
            g_bTrimCountComplete = FALSE;
            RESERVED_OUT_1_Write(0);
            Buzzer(BUZZER_STOP, 0);
            return;
        }
    }
    
    if(g_bTrimCountComplete == FALSE && g_ptrCount->count == g_ptrMachineParameter->setTrimCount) // 2024.05.20
    {
        Buzzer(BUZZER_WARNING_CONTINUOUS, 0);
        RESERVED_OUT_1_Write(1);
        g_bTrimCountComplete = TRUE; // 2024.05.20
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
