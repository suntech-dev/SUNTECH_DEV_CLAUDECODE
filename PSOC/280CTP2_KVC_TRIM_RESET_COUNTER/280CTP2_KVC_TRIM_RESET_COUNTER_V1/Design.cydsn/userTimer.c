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
#include "lib/sysTick.h"
#include "lib/RealTimeClock.h"
#include "lib/UI.h"
#include "userTimer.h"
#include "main.h"

///////////////////////////////////////////////////////////////
// 1ms에 관련된 것들  //////////////////////////////////////////////
///////////////////////////////////////////////////////////////
uint8 g_bIsElapsed_1ms = FALSE;

// sysTick.h에서 사용 1ms마다 인터럽트에서 호출된다. (정확한 시간)
void OneMilliSecond_ISR() 
{
    g_bIsElapsed_1ms = TRUE;
    
    BuzzerTimer();    
}

uint8 isElasped_1ms()
{
    return g_bIsElapsed_1ms;
}

// main loop에서 1ms 마다 작동하는 함수 (delay 발생할 수 있음, 정확한 시간이 아님)
void OneMilliSecond_MainLoop() 
{
    if(g_bIsElapsed_1ms)
    {
        g_bIsElapsed_1ms = FALSE;    
    }
}

///////////////////////////////////////////////////////////////
// 1s에 관련된 것들  ///////////////////////////////////////////////
///////////////////////////////////////////////////////////////
uint8 g_bIsElapsed_1s  = FALSE;
void OneSecond_ISR() 
{
    g_bIsElapsed_1s = TRUE;    
}

uint8 isElasped_1s()
{
    return g_bIsElapsed_1s;
}

// main loop에서 1s 마다 작동하는 함수 (delay 발생할 수 있음, 정확한 시간이 아님)
void OneSecond_MainLoop() 
{
    if(g_bIsElapsed_1s)
    {
        g_bIsElapsed_1s = FALSE;
        
        RTC_Update();
    }    
}
///////////////////////////////////////////////////////////////
// 초기화  ///////////////////////////////////////////////
///////////////////////////////////////////////////////////////
void initTimer()
{
    setSysTick();
    initRealTimeClock();
}
/* [] END OF FILE */
