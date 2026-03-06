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
#ifndef __REAL_TIME_CLOCK_H__
#define __REAL_TIME_CLOCK_H__
#include "main.h"
    
void initRealTimeClock();
void PrintRTC();

uint8 isPassedOneSecond();
uint32 getElapedTime();

void setCurrentTime(char *data);
void SetDateTime(uint16 year, uint16 month, uint16 day, uint16 hour, uint16 minute, uint16 second);

extern uint8 g_bIsAvaliableDateTime;

#endif
/* [] END OF FILE */
