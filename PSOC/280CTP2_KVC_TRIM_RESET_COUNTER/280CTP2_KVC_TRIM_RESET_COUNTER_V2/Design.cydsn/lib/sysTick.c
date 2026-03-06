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
#include "main.h"
#include "lib/sysTick.h"
#include "lib/timerCounter.h"

//////////////////////////////////////////////////////////////
// 1ms에 관련된 것들  //////////////////////////////////////////////
///////////////////////////////////////////////////////////////
uint8 g_uNoMiliSecondCounter=0;
TIMER_COUNTER g_timerCounter_1ms[MAX_NO_MILISECOND_COUNTER];

// 1ms count의 목표에 도달 했는지 여부, index는 ONE_MILISECOND_COUNTER enum값
// 주의 : main loop의  총 수행시간이 1ms 보다 크면 목표값에 도달한 것을 놓칠 수 있다.
uint8 isFinishCounter_1ms(uint16 index)
{
   return g_timerCounter_1ms[index].current == 0 ? TRUE : FALSE;
}

void resetCounter_1ms(uint16 index)
{
    g_timerCounter_1ms[index].current = 
    g_timerCounter_1ms[index].max;
}

// 1s timer counter 등록 
uint8 registerCounter_1ms(uint16 maxCount)
{
    if(g_uNoMiliSecondCounter < MAX_NO_MILISECOND_COUNTER-1)
    {
        g_timerCounter_1ms[g_uNoMiliSecondCounter].current = 
        g_timerCounter_1ms[g_uNoMiliSecondCounter].max = maxCount;
        
        return g_uNoMiliSecondCounter++;
    } else {
        printf("Warning : resisterCounter_1ms()\r\n"); 
    }
    return 0xFF;   
}

// 현재 counter 값을 알아낸다. 
// 주의 counter is Decreasing..
uint16 getCurrentTimerCounter_1ms(uint8 index) { return g_timerCounter_1ms[index].current; }

/*******************************************************************************
* Function Definitions
*******************************************************************************/
// 1ms 마다 인터럽트에 의한 호출되는 ISR
void SysTickISRCallback_1ms(void)
{
    OneMilliSecond_ISR(); 
    
    for(int i=0; i < g_uNoMiliSecondCounter; i++)
    {
        if(g_timerCounter_1ms[i].current == 0)
            g_timerCounter_1ms[i].current = g_timerCounter_1ms[i].max;
        else
            g_timerCounter_1ms[i].current--;            
    }
}

void setSysTick(void)
{
    uint32 i;
    
    CySysTickStart();
    
    /* Find unused callback slot and assign the callback. */
    for (i = 0u; i < CY_SYS_SYST_NUM_OF_CALLBACKS; ++i)
    {
        if (CySysTickGetCallback(i) == NULL)
        {
            /* Set callback */
            CySysTickSetCallback(i, SysTickISRCallback_1ms);
            break;
        }
    }
}

/* [] END OF FILE */
