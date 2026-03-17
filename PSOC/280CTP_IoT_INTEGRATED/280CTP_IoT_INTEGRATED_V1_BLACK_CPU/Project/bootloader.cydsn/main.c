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
#include "project.h"

unsigned long int timerCount =0;

void SysTickISRCallback(void)
{ 
    timerCount++;
    
    if((timerCount % 100) == 0) LED_Write(~LED_Read());    
}

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

int main(void)
{
    CyGlobalIntEnable; /* Enable global interrupts. */
 
    initSysTick();
   
    Bootloader_Start();

    for(;;)
    {
        /* Place your application code here. */
    }
}

/* [] END OF FILE */
