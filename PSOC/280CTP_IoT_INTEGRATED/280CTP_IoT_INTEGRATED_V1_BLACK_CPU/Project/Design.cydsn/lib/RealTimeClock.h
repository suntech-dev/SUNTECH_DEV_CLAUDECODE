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
#ifndef __REAL_TIME_CLOCK_H__
#define __REAL_TIME_CLOCK_H__
#include "main.h"
    
void OneSecond_ISR();

void initRealTimeClock();
void PrintRTC();

void setCurrentTime(char *data);
void SetDateTime(uint16 year, uint16 month, uint16 day, uint16 hour, uint16 minute, uint16 second);

extern uint8 g_bIsAvaliableDateTime;

//////////////////////////////////////////////////////////////////////////
// 1 second에 관련된 것들 /////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////// 

void OneSecond_MainLoop();      // main loop에서 1s 마다 작동하는 함수

 // 1s count의 목표에 도달 했는지 여부 , index는 ONE_SECOND_COUNTER enum값
uint8 isFinishCounter_1s(uint16 index);
// 현재 counter 값을 알아낸다. 
// 주의 counter is Decreasing..

// 1s timer counter 등로
uint8 registerCounter_1s(uint16 maxCount);
void resetCounter_1s(uint16 index);

// 현재 counter 값을 알아낸다. 
// 주의 counter is Decreasing..
uint16 getCurrentTimerCounter_1s(uint8 index);

#endif
/* [] END OF FILE */
