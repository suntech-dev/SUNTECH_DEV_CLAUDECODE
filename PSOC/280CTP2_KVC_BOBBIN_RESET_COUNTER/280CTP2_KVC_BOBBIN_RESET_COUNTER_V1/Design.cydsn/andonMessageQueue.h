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
#ifndef __ANDON_MESSAGE_QUEUE_H__
#define __ANDON_MESSAGE_QUEUE_H__
#include "main.h"
    
#define MAX_ANDON_MESSAGE_QUEUE 5
    
typedef struct {
    char message[256];
    uint16 type;
} ANDON_QUEUE;


int isFullQueueANDON(void);
int isEmptyQueueANDON(void);
short enQueueANDON(ANDON_QUEUE data);
short enQueueANDON_printf(uint16 type, const char *fmt, ...);
ANDON_QUEUE *deQueueANDON(void);
ANDON_QUEUE *getFrontQueueANDON(void);
ANDON_QUEUE *getFrontNextQueueANDON(int i);
ANDON_QUEUE *getRearQueueANDON(void);
short getSizeQueueANDON(void);
void PrintInfoQueueANDON(void);

#endif
/* [] END OF FILE */
