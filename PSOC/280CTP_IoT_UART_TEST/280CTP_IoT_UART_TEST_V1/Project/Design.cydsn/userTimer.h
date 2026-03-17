/* ========================================
 *
 * Copyright YOUR COMPANY, THE YEAR
 * All Rights Reserved
 * UNPUBLISHED, LICENSED SOFTWARE.
 *
 * CONFIDENTIAL AND PROPRIETARY INFORMATION
 * WHICH IS THE PROPERTY OF your company.
 *
 * ========================================
*/
#ifndef _USER_TIMER_H_
#define _USER_TIMER_H_

#include "main.h"
#include "lib/sysTick.h"

void OneMilliSecond_MainLoop();
void OneSecond_MainLoop();
void initTimer();

uint8 registerCounter_1s(uint32 maxCount);
uint8 isFinishCounter_1s(uint16 index);

#endif
/* [] END OF FILE */
