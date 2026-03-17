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
#include "andonMessageQueue.h"

static short g_uSizeQueueANDON  = 0;  /* 접근자 함수로만 외부 접근 */
static short g_uFrontQueueANDON = 0;
static short g_uRearQueueANDON  = 0;
static ANDON_QUEUE g_cQueue[MAX_ANDON_MESSAGE_QUEUE];

int isFullQueueANDON(void)
{
    if( (g_uRearQueueANDON + 1) % MAX_ANDON_MESSAGE_QUEUE == g_uFrontQueueANDON)
        return 1;
    else
        return 0;
}

int isEmptyQueueANDON(void)
{
    if(g_uFrontQueueANDON==g_uRearQueueANDON)
            return 1;
    else
            return 0;
}

short enQueueANDON(ANDON_QUEUE data)
{
    if(isFullQueueANDON()) return FALSE;

    g_uSizeQueueANDON++;
    g_uRearQueueANDON = (g_uRearQueueANDON + 1) % MAX_ANDON_MESSAGE_QUEUE;
    g_cQueue[g_uRearQueueANDON] = data;
    return TRUE;
}

short enQueueANDON_printf(uint16 type, const char *fmt, ...)
{
    if(isFullQueueANDON()) 
    {
        printf("isFullQueueANDON() FULL \r\n");
        return FALSE;
    }

    g_uSizeQueueANDON++;
    g_uRearQueueANDON = (g_uRearQueueANDON + 1) % MAX_ANDON_MESSAGE_QUEUE;
    
    g_cQueue[g_uRearQueueANDON].type = type;
    
    va_list ap;   
    va_start(ap, fmt);

    vsprintf(g_cQueue[g_uRearQueueANDON].message, fmt, ap);
    va_end(ap);
   
//    printf(">>:%s\r\n",g_cQueue[g_uRearQueueANDON].message);
        
    return TRUE;    
}

ANDON_QUEUE *deQueueANDON(void)
{
    g_uSizeQueueANDON--;
    g_uFrontQueueANDON = (g_uFrontQueueANDON + 1) % MAX_ANDON_MESSAGE_QUEUE;
    
    return &g_cQueue[g_uFrontQueueANDON];
}

ANDON_QUEUE *getFrontQueueANDON(void)
{
   return &g_cQueue[(g_uFrontQueueANDON + 1 ) % MAX_ANDON_MESSAGE_QUEUE ];
}

ANDON_QUEUE *getFrontNextQueueANDON(int i)
{
   return &g_cQueue[(g_uFrontQueueANDON + 1 + i) % MAX_ANDON_MESSAGE_QUEUE];
}

ANDON_QUEUE *getRearQueueANDON(void)
{
   return &g_cQueue[ g_uRearQueueANDON % MAX_ANDON_MESSAGE_QUEUE];
}

short getSizeQueueANDON(void)
{
   return g_uSizeQueueANDON;
}

void PrintInfoQueueANDON(void)
{
  // printf("Queue -> Size : %d\n", g_uSizeQueueANDON);
  // printf("Rest Size : %d\n", getRestSizeQueue());
}


/* [] END OF FILE */
