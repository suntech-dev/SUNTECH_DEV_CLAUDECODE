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
#include "main.h"

void init(void)
{
    initSysTick();

    port_MONITORING_Start();
    port_OP_Start();
    BARCODE_Start();
}


void BootloaderStart(void)
{
    port_MONITORING_UartPutString(__DATE__);
    port_MONITORING_UartPutString("\r\nCurrent firmware Version is ");
    port_MONITORING_UartPutString(FIRMWARE_VERSION);
    port_MONITORING_UartPutString("\r\nFirmware Upgrade Ready");
    port_MONITORING_UartPutString("(115200bps).");

    Bootloadable_Load();
}

/* [] END OF FILE */
