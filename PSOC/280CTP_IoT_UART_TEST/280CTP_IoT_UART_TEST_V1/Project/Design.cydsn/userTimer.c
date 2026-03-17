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
#include "lib/UI.h"
#include "userTimer.h"
#include "main.h"

///////////////////////////////////////////////////////////////
// 1ms에 관련된 것들
///////////////////////////////////////////////////////////////
uint8 g_bIsElapsed_1ms = FALSE;

void OneMilliSecond_ISR()
{
    g_bIsElapsed_1ms = TRUE;
    BuzzerTimer();
}

void OneMilliSecond_MainLoop()
{
    if(g_bIsElapsed_1ms)
    {
        g_bIsElapsed_1ms = FALSE;
    }
}

///////////////////////////////////////////////////////////////
// 1s에 관련된 것들
///////////////////////////////////////////////////////////////
uint8 g_bIsElapsed_1s = FALSE;

void OneSecond_ISR()
{
    g_bIsElapsed_1s = TRUE;
}

void OneSecond_MainLoop()
{
    if(g_bIsElapsed_1s)
    {
        g_bIsElapsed_1s = FALSE;
    }
}

///////////////////////////////////////////////////////////////
// 1s 카운터 — sysTick 1ms 카운터를 1000 단위로 래핑
///////////////////////////////////////////////////////////////
uint8 registerCounter_1s(uint32 maxCount)
{
    return registerCounter_1ms(maxCount * 1000u);
}

uint8 isFinishCounter_1s(uint16 index)
{
    return isFinishCounter_1ms(index);
}

///////////////////////////////////////////////////////////////
// 초기화
///////////////////////////////////////////////////////////////
void initTimer()
{
    setSysTick();
}
/* [] END OF FILE */
