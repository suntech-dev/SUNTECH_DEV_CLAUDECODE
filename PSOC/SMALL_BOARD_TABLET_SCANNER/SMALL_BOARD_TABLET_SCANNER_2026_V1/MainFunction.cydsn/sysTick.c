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

/* Function Prototypes */
void SysTickISRCallback(void);

unsigned long int timerCount =0;

void initSysTick()
{
    CySysTickStart();
    uint32 i;
    
    /* Find unused callback slot and assign the callback. */
    for (i = 0u; i < CY_SYS_SYST_NUM_OF_CALLBACKS; ++i)
    {
        if (CySysTickGetCallback(i) == NULL)
        {
            /* Set callback */
            CySysTickSetCallback(i, SysTickISRCallback);
            break;
        }
    }
}

void SysTickISRCallback(void)
{ 
    timerCount++;
    
    if((timerCount % 1000) == 0) LED_Write(~LED_Read());    
}

/* [] END OF FILE */
