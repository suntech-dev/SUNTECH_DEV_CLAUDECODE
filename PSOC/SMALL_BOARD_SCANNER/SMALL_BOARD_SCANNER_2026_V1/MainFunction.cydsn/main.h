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
#include <stdint.h>
#include <string.h>

/* ---- 버퍼 크기 상수 ---- */
#define UART_BUF_SIZE           (256u)

/* ---- 타임아웃 / 카운터 임계값 (ms) ---- */
#define TIMEOUT_MS_UART         (50u)
#define COUNTER_MIN_MS          (200u)
#define COUNTER_MAX_MS          (1900u)

/* ---- Touch OP 수신 명령 문자열 상수 ---- */
#define SCAN_TRIGER_ORDER_STR   "$$$$#99900035;%%%%"   /* 스캔 트리거 요청 */
#define SCAN_MODE_CMD_STR       "$$$$#99900304;%%%%"   /* 스캐너 연결 확인 */

/* ---- 펌웨어 버전 ---- */
#define FIRMWARE_VERSION        "SCANNER_SMALL_BOARD_2026_V1"

void initSysTick(void);
void init(void);
void BootloaderStart(void);

extern unsigned long int timerCount;

#endif
/* [] END OF FILE */
