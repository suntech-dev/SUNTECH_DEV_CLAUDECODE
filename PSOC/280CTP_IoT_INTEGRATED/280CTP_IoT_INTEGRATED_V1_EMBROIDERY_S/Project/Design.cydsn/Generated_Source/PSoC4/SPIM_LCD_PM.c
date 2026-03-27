/*******************************************************************************
* File Name: SPIM_LCD_PM.c
* Version 2.50
*
* Description:
*  This file contains the setup, control and status commands to support
*  component operations in low power mode.
*
* Note:
*
********************************************************************************
* Copyright 2008-2015, Cypress Semiconductor Corporation.  All rights reserved.
* You may use this file only in accordance with the license, terms, conditions,
* disclaimers, and limitations in the end user license agreement accompanying
* the software package with which this file was provided.
*******************************************************************************/

#include "SPIM_LCD_PVT.h"

static SPIM_LCD_BACKUP_STRUCT SPIM_LCD_backup =
{
    SPIM_LCD_DISABLED,
    SPIM_LCD_BITCTR_INIT,
};


/*******************************************************************************
* Function Name: SPIM_LCD_SaveConfig
********************************************************************************
*
* Summary:
*  Empty function. Included for consistency with other components.
*
* Parameters:
*  None.
*
* Return:
*  None.
*
*******************************************************************************/
void SPIM_LCD_SaveConfig(void) 
{

}


/*******************************************************************************
* Function Name: SPIM_LCD_RestoreConfig
********************************************************************************
*
* Summary:
*  Empty function. Included for consistency with other components.
*
* Parameters:
*  None.
*
* Return:
*  None.
*
*******************************************************************************/
void SPIM_LCD_RestoreConfig(void) 
{

}


/*******************************************************************************
* Function Name: SPIM_LCD_Sleep
********************************************************************************
*
* Summary:
*  Prepare SPIM Component goes to sleep.
*
* Parameters:
*  None.
*
* Return:
*  None.
*
* Global Variables:
*  SPIM_LCD_backup - modified when non-retention registers are saved.
*
* Reentrant:
*  No.
*
*******************************************************************************/
void SPIM_LCD_Sleep(void) 
{
    /* Save components enable state */
    SPIM_LCD_backup.enableState = ((uint8) SPIM_LCD_IS_ENABLED);

    SPIM_LCD_Stop();
}


/*******************************************************************************
* Function Name: SPIM_LCD_Wakeup
********************************************************************************
*
* Summary:
*  Prepare SPIM Component to wake up.
*
* Parameters:
*  None.
*
* Return:
*  None.
*
* Global Variables:
*  SPIM_LCD_backup - used when non-retention registers are restored.
*  SPIM_LCD_txBufferWrite - modified every function call - resets to
*  zero.
*  SPIM_LCD_txBufferRead - modified every function call - resets to
*  zero.
*  SPIM_LCD_rxBufferWrite - modified every function call - resets to
*  zero.
*  SPIM_LCD_rxBufferRead - modified every function call - resets to
*  zero.
*
* Reentrant:
*  No.
*
*******************************************************************************/
void SPIM_LCD_Wakeup(void) 
{
    #if(SPIM_LCD_RX_SOFTWARE_BUF_ENABLED)
        SPIM_LCD_rxBufferFull  = 0u;
        SPIM_LCD_rxBufferRead  = 0u;
        SPIM_LCD_rxBufferWrite = 0u;
    #endif /* (SPIM_LCD_RX_SOFTWARE_BUF_ENABLED) */

    #if(SPIM_LCD_TX_SOFTWARE_BUF_ENABLED)
        SPIM_LCD_txBufferFull  = 0u;
        SPIM_LCD_txBufferRead  = 0u;
        SPIM_LCD_txBufferWrite = 0u;
    #endif /* (SPIM_LCD_TX_SOFTWARE_BUF_ENABLED) */

    /* Clear any data from the RX and TX FIFO */
    SPIM_LCD_ClearFIFO();

    /* Restore components block enable state */
    if(0u != SPIM_LCD_backup.enableState)
    {
        SPIM_LCD_Enable();
    }
}


/* [] END OF FILE */
