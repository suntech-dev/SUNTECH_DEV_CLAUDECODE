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
#include "userProjectPatternSewing.h"
#include "andonApi.h"
#include "uartJson.h"

static uint8 g_updateTrimCount=FALSE;  /* ISR + SewingCountLoop 내부 전용 */
uint8 g_updateCountMenu=FALSE;
//uint16 g_trimCount=0;

COUNT *g_ptrCount = NULL;

static uint32 g_uWorkingTimeCount=0;   /* 접근자 함수(Get/Reset)로만 외부 접근 */

void PatternCountLoop();
void SewingCountLoop();

void (*CountFunc)() = &SewingCountLoop;

static uint16 g_Count = 0;             /* ISR + SewingCountLoop 내부 전용 */

volatile uint8_t  g_bStartTrimPin    = FALSE;  /* ISR 공유 → volatile 필수 */
volatile uint16_t g_bTrimElapsedTime = 0;      /* ISR 공유 → volatile 필수 */

CY_ISR(Trim_Interrupt_Routine)
{
    g_updateTrimCount = TRUE;
    //g_trimCount++;
    g_ptrCount->sewingTrimCount++;
    g_Count++;
    
    if(TrimPin_Read() == 0)
    {
        g_bStartTrimPin = TRUE;
        g_bTrimElapsedTime = 0;
    }
}

void initCount()
{
    if(sizeof(COUNT) > sizeof(g_internalFlash.data))
    {
        printf("Wrong... COUNT size is large than g_internalFlash.data\r\n");        
    }    
    g_ptrCount = (COUNT *) &g_internalFlash.data;  
    
    SetCountLoop();
}

void SetCountLoop()
{
    switch(g_ptrMachineParameter->machineType)
    {
        case PATTERN_MACHINE:
            CountFunc = &PatternCountLoop;
            trim_Int_Stop();
        break;
        case SEWING_MACHINE :
            CountFunc = &SewingCountLoop;
            trim_Int_StartEx(&Trim_Interrupt_Routine); 
        break;
    }
}

void SewingCountLoop()
{
    if(g_updateTrimCount == FALSE) return;

    uint8 bSend = FALSE;
    //uint32 actual =  CONVERT_TO_4BYTE(g_ptrCount->sewingActualH, g_ptrCount->sewingActualL) /10;
    
    ADD_CONVERT_TO_4BYTE(g_ptrCount->sewingTrimCountSumH,g_ptrCount->sewingTrimCountSumL,g_Count); g_Count = 0;
    
    //printf("\r\n%u %u %lu\r\n",g_ptrCount->sewingTrimCount,g_ptrMachineParameter->sewingPairTrim,(uint32) CONVERT_TO_4BYTE(g_ptrCount->sewingTrimCountSumH,g_ptrCount->sewingTrimCountSumL));
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
    
    //if(CONVERT_TO_4BYTE(g_ptrCount->sewingActualH, g_ptrCount->sewingActualL)/10 > actual)
    //{
    //  g_updateCountMenu = TRUE;
    //  makeAndonSewingCount();
    //  SaveInternalFlash();              
    //}
    
    g_updateTrimCount = FALSE;        
}

void PatternCountLoop()
{
    if(uartJsonLoop()==TRUE)
    {
        g_updateCountMenu = TRUE;
    }
}

COUNT *getCount() 
{
    return g_ptrCount;
}

void ResetCount()
{
    g_ptrCount->patternActualH = 0;
    g_ptrCount->patternActualL = 0; 
    
    // 김도형 추가.
    g_ptrCount->patternCycleTimeSumH = 0;
    g_ptrCount->patternCycleTimeSumL = 0;
    g_ptrCount->patternMotorRunTimeSumH = 0;
    g_ptrCount->patternMotorRunTimeSumL = 0;
    g_ptrCount->patternNoStitchSumH = 0;
    g_ptrCount->patternNoStitchSumL = 0;
    g_ptrCount->patternStitchLengthSumH = 0;
    g_ptrCount->patternStitchLengthSumL = 0;
    g_ptrCount->patternTrimCountSumH = 0;
    g_ptrCount->patternTrimCountSumL = 0;
    g_ptrCount->patternEmergencyTimeSumH = 0;
    g_ptrCount->patternEmergencyTimeSumL = 0;
    //                
    
    g_ptrCount->sewingActualH = 0;
    g_ptrCount->sewingActualL = 0;  
    
    SaveInternalFlash(); 
}

void WorkingTimeCount()
{
    g_uWorkingTimeCount++;
}

uint32 GetWorkingTimeCount()
{
    return g_uWorkingTimeCount;
}

void ResetWorkingTimeCount()
{
    g_uWorkingTimeCount = 0;
}
/* [] END OF FILE */
