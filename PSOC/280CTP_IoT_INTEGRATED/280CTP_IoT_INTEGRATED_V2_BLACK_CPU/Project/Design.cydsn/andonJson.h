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
#ifndef _ANDON_JSON_H_
#define _ANDON_JSON_H_
    
#define DEBUG_ANDON_JSON
    
#include "main.h"

void andonJsonParsor(int type, char *jsonString, int16 sizeOfJson);
extern uint8 g_uReplyResult;
extern uint32 g_lastSendTime;
#endif    
/* [] END OF FILE */
