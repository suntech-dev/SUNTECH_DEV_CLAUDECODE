/******************************************************************************
* File Name: main.c
*
* Version: 1.00
*
* Description: This example demonstrates how to switch debounce using the 
*              Debouncer Component.
*
* Related Document: CE224719.pdf 
*
* Hardware Dependency: CY8CKIT-042 PSoC 4 PIONEER KIT
*
******************************************************************************
* Copyright (2018), Cypress Semiconductor Corporation.
******************************************************************************
* This software, including source code, documentation and related materials
* ("Software") is owned by Cypress Semiconductor Corporation (Cypress) and is
* protected by and subject to worldwide patent protection (United States and 
* foreign), United States copyright laws and international treaty provisions. 
* Cypress hereby grants to licensee a personal, non-exclusive, non-transferable
* license to copy, use, modify, create derivative works of, and compile the 
* Cypress source code and derivative works for the sole purpose of creating 
* custom software in support of licensee product, such licensee product to be
* used only in conjunction with Cypress's integrated circuit as specified in the
* applicable agreement. Any reproduction, modification, translation, compilation,
* or representation of this Software except as specified above is prohibited 
* without the express written permission of Cypress.
* 
* Disclaimer: THIS SOFTWARE IS PROVIDED AS-IS, WITH NO WARRANTY OF ANY KIND, 
* EXPRESS OR IMPLIED, INCLUDING, BUT NOT LIMITED TO, NONINFRINGEMENT, IMPLIED 
* WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
* Cypress reserves the right to make changes to the Software without notice. 
* Cypress does not assume any liability arising out of the application or use
* of Software or any product or circuit described in the Software. Cypress does
* not authorize its products for use as critical components in any products 
* where a malfunction or failure may reasonably be expected to result in 
* significant injury or death ("ACTIVE Risk Product"). By including Cypress's 
* product in a ACTIVE Risk Product, the manufacturer of such system or application
* assumes all risk of such use and in doing so indemnifies Cypress against all
* liability. Use of this Software may be limited by and subject to the applicable
* Cypress software license agreement.
*****************************************************************************/

#include <project.h>
#include <stdio.h>

/*******************************************************************************
*            Global variables
*******************************************************************************/
uint32 count = 0u;           /* # of raw (unfiltered)transitions of input pin 'SW' */
uint32 filteredCount = 0u;   /* # of filtered transitions of input pin 'SW' */

/*******************************************************************************
* Function Name: switchInt_Handler
********************************************************************************
* Summary:
*  The Interrupt Service Routine increments a global 'count' variable.
*   
* Parameters:
*  None
*
* Return:
*  None
*
*******************************************************************************/
CY_ISR(switchInt_Handler)
{
    /* Clear interrupt source and increment count */
    Control_Reg_Write(1);
    count++; 
}

/*****************************************************************************
* Function Name: debouncedInt_Handler
******************************************************************************
* Summary:
*   The Interrupt Service Routine increments a global 'filteredCount' variable.
*
* Parameters:
*   none
*
* Return:
*   None.  Filtered count is incremented.
*
*****************************************************************************/
CY_ISR(debouncedInt_Handler)
{
	/* Debouncer status reg is cleared through read and filtered count incremented*/
	filteredCount++;
} 

/*******************************************************************************
* Function Name: main
********************************************************************************
*
*  The main function performs the following actions:
*   1. Sets up the interrupt handler and UART operation.
*   2. Sources are cleared.
*   3. Local variables used are initialized.
*   4. The interrupt is disabled and renabled after getting a copy of count. 
*   5. The current and filtered count are displayed via UART to terminal.
*   6. When switch bounce has occured it is also displayed.
*
*******************************************************************************/
int main(void)
{
    char snum[32]; /* for string conversion */
    uint32 tempCount;
    uint32 tempFilteredCount;
    uint32 prevCount = 0ul;
    uint32 prevFilterCount = 0ul;

    /* Interrupt handler for switch and debounced switch */
    switch_Int_StartEx(switchInt_Handler);
    debounced_Int_StartEx(debouncedInt_Handler);
    
    /* Clears pending interrupt */
    switch_Int_ClearPending();
    debounced_Int_ClearPending();
    
    /* Clears interrupt source */
    Control_Reg_Write(1); 
    
    /* Start UART operation */
    UART_Start();
    UART_UartPutString("UART started\r\n");
    
    /* Enable global interrupts. */
	CyGlobalIntEnable; 
    
    for(;;)
    {
        /* Disables interrupt, getting a copy of value before allowing interrupts to occur */
        CyGlobalIntDisable;
        tempCount = count;
        tempFilteredCount = filteredCount;
        CyGlobalIntEnable;
 
        /* If count has changed or debounce has occured then the change is displayed */
        if(tempCount != prevCount)
        {
            UART_UartPutString("Raw Count = ");
            sprintf(snum, "%lu", tempCount);
            UART_UartPutString(snum);
        }

        /* If filtered count is changed then the change is displayed */
        if(tempFilteredCount != prevFilterCount)
        {
            UART_UartPutString(" | Debounced Count = ");
            sprintf(snum, "%lu", tempFilteredCount);
            UART_UartPutString(snum);
            UART_UartPutString("\r\n");
        }
        
        /* Previous count and filtered counts are updated */
        prevCount = tempCount;
        prevFilterCount = tempFilteredCount;
    }
}

/* [] END OF FILE */
