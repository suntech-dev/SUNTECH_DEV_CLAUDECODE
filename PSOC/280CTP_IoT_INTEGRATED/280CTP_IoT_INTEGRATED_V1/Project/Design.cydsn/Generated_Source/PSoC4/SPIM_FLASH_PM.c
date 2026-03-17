/*******************************************************************************
* File Name: SPIM_FLASH_PM.c
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

#include "SPIM_FLASH_PVT.h"

static SPIM_FLASH_BACKUP_STRUCT SPIM_FLASH_backup =
{
    SPIM_FLASH_DISABLED,
    SPIM_FLASH_BITCTR_INIT,
};


/*******************************************************************************
* Function Name: SPIM_FLASH_SaveConfig
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
void SPIM_FLASH_SaveConfig(void) 
{

}


/*******************************************************************************
* Function Name: SPIM_FLASH_RestoreConfig
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
void SPIM_FLASH_RestoreConfig(void) 
{

}


/*******************************************************************************
* Function Name: SPIM_FLASH_Sleep
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
*  SPIM_FLASH_backup - modified when non-retention registers are saved.
*
* Reentrant:
*  No.
*
*******************************************************************************/
void SPIM_FLASH_Sleep(void) 
{
    /* Save components enable state */
    SPIM_FLASH_backup.enableState = ((uint8) SPIM_FLASH_IS_ENABLED);

    SPIM_FLASH_Stop();
}


/*******************************************************************************
* Function Name: SPIM_FLASH_Wakeup
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
*  SPIM_FLASH_backup - used when non-retention registers are restored.
*  SPIM_FLASH_txBufferWrite - modified every function call - resets to
*  zero.
*  SPIM_FLASH_txBufferRead - modified every function call - resets to
*  zero.
*  SPIM_FLASH_rxBufferWrite - modified every function call - resets to
*  zero.
*  SPIM_FLASH_rxBufferRead - modified every function call - resets to
*  zero.
*
* Reentrant:
*  No.
*
*******************************************************************************/
void SPIM_FLASH_Wakeup(void) 
{
    #if(SPIM_FLASH_RX_SOFTWARE_BUF_ENABLED)
        SPIM_FLASH_rxBufferFull  = 0u;
        SPIM_FLASH_rxBufferRead  = 0u;
        SPIM_FLASH_rxBufferWrite = 0u;
    #endif /* (SPIM_FLASH_RX_SOFTWARE_BUF_ENABLED) */

    #if(SPIM_FLASH_TX_SOFTWARE_BUF_ENABLED)
        SPIM_FLASH_txBufferFull  = 0u;
        SPIM_FLASH_txBufferRead  = 0u;
        SPIM_FLASH_txBufferWrite = 0u;
    #endif /* (SPIM_FLASH_TX_SOFTWARE_BUF_ENABLED) */

    /* Clear any data from the RX and TX FIFO */
    SPIM_FLASH_ClearFIFO();

    /* Restore components block enable state */
    if(0u != SPIM_FLASH_backup.enableState)
    {
        SPIM_FLASH_Enable();
    }
}


/* [] END OF FILE */
