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
#ifndef _USERTIMER_H_
#define _USERTIMER_H_

#include <project.h>

void setSysTick(void);
    
void OneMilliSecond_ISR();

// 1ms count의 목표에 도달 했는지 여부, index는 ONE_MILISECOND_COUNTER enum값
// 주의 : main loop의  총 수행시간이 1ms 보다 크면 목표값에 도달한 것을 놓칠 수 있다.
uint8 isFinishCounter_1ms(uint16 index);

// 1ms timer counter 등록 
uint8 registerCounter_1ms(uint32 maxCount);

// 1ms timer를 주어진 index에 maxCount 값을 셋한다.
void setCountMax_1ms(uint16 index, uint32 maxCount);
// 1ms timer를 주어진 index에 해당하는 값을 리셋한다.

void resetCounter_1ms(uint16 index);
// 현재 counter 값을 알아낸다. 
// 주의 counter is Decreasing..
uint16 getCurrentTimerCounter_1ms(uint16 index);

extern uint8 g_b1ms;

#endif
/* [] END OF FILE */
