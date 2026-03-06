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
#ifndef _TIMER_COUNTER_H_
#define _TIMER_COUNTER_H_

#include "main.h"
    
typedef struct {
    uint16 current;
    uint16 max;  
} TIMER_COUNTER;
    
#define MAX_NO_MILISECOND_COUNTER 10 // MILISECOND_COUNTER의 최대 갯수
#define MAX_NO_SECOND_COUNTER     10 // SECOND_COUNTER의 최대 갯수

#endif    
/* [] END OF FILE */
