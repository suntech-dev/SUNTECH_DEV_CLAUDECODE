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
#include "project.h"

/* ---- Function Prototypes ---- */
void SysTickISRCallback(void);

/* ---- 전역 타이머 카운터 (1ms 단위) ---- */
unsigned long int timerCount      = 0;
unsigned long int g_timerCountWork = 0;

void initSysTick(void)
{
    uint32 i;

    CySysTickStart();

    for (i = 0u; i < CY_SYS_SYST_NUM_OF_CALLBACKS; ++i)
    {
        if (CySysTickGetCallback(i) == NULL)
        {
            CySysTickSetCallback(i, SysTickISRCallback);
            break;
        }
    }
}

void SysTickISRCallback(void)
{
    g_timerCountWork++;
    timerCount++;

    if ((timerCount % 1000u) == 0u)
    {
        LED_Write(~LED_Read());
    }
}

/* [] END OF FILE */
