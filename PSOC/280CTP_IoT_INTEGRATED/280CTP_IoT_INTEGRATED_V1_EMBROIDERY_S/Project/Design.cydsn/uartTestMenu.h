/* ========================================
 *
 * Copyright Suntech, 2026.03.26
 * All Rights Reserved
 * UNPUBLISHED, LICENSED SOFTWARE.
 *
 * CONFIDENTIAL AND PROPRIETARY INFORMATION
 * WHICH IS THE PROPERTY OF Suntech.
 *
 * ========================================
*/
#ifndef _UART_TEST_MENU_H_
#define _UART_TEST_MENU_H_

#include "main.h"

extern uint8 g_bUartTestMode;  /* TRUE: UART TEST 뷰어 활성 */

void uartTestAddLine(const char *line);  /* uartJson.c 에서 미러 호출 */
int  doUartTestMenu(void *this, uint8 reflash);

#endif
/* [] END OF FILE */
