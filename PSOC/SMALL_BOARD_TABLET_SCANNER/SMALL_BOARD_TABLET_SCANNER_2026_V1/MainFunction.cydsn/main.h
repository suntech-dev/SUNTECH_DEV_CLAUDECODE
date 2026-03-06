/* ========================================
 *
 * Copyright SUNTECH, 2018-2026
 * All Rights Reserved
 * UNPUBLISHED, LICENSED SOFTWARE.
 *
 * CONFIDENTIAL AND PROPRIETARY INFORMATION
 * WHICH IS THE PROPERTY OF SUNTECH.
 *
 * ========================================
*/
#ifndef __MAIN_H__
#define __MAIN_H__

#include "project.h"
#include <stdio.h>

void initSysTick(void);
void init(void);
void BootloaderStart(void);

extern unsigned long int timerCount;

#define UART_BUF_SIZE         (256u)
#define TIMEOUT_MS_UART       (50u)
#define COUNTER_MIN_MS        (200u)
#define COUNTER_MAX_MS        (1900u)

#define SCAN_TRIGER_ORDER_STR "$$$$#99900035;%%%%"
#define SCAN_MODE_CMD_STR     "$$$$#99900304;%%%%"

#define FIRMWARE_VERSION      "TABLET_SCANNER_SMALL_BOARD_2026_V1"

#endif
/* [] END OF FILE */
