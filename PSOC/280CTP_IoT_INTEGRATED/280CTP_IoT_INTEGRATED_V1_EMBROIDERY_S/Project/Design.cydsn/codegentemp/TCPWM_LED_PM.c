/*******************************************************************************
* File Name: TCPWM_LED_PM.c
* Version 2.10
*
* Description:
*  This file contains the setup, control, and status commands to support
*  the component operations in the low power mode.
*
* Note:
*  None
*
********************************************************************************
* Copyright 2013-2015, Cypress Semiconductor Corporation.  All rights reserved.
* You may use this file only in accordance with the license, terms, conditions,
* disclaimers, and limitations in the end user license agreement accompanying
* the software package with which this file was provided.
*******************************************************************************/

#include "TCPWM_LED.h"

static TCPWM_LED_BACKUP_STRUCT TCPWM_LED_backup;


/*******************************************************************************
* Function Name: TCPWM_LED_SaveConfig
********************************************************************************
*
* Summary:
*  All configuration registers are retention. Nothing to save here.
*
* Parameters:
*  None
*
* Return:
*  None
*
*******************************************************************************/
void TCPWM_LED_SaveConfig(void)
{

}


/*******************************************************************************
* Function Name: TCPWM_LED_Sleep
********************************************************************************
*
* Summary:
*  Stops the component operation and saves the user configuration.
*
* Parameters:
*  None
*
* Return:
*  None
*
*******************************************************************************/
void TCPWM_LED_Sleep(void)
{
    if(0u != (TCPWM_LED_BLOCK_CONTROL_REG & TCPWM_LED_MASK))
    {
        TCPWM_LED_backup.enableState = 1u;
    }
    else
    {
        TCPWM_LED_backup.enableState = 0u;
    }

    TCPWM_LED_Stop();
    TCPWM_LED_SaveConfig();
}


/*******************************************************************************
* Function Name: TCPWM_LED_RestoreConfig
********************************************************************************
*
* Summary:
*  All configuration registers are retention. Nothing to restore here.
*
* Parameters:
*  None
*
* Return:
*  None
*
*******************************************************************************/
void TCPWM_LED_RestoreConfig(void)
{

}


/*******************************************************************************
* Function Name: TCPWM_LED_Wakeup
********************************************************************************
*
* Summary:
*  Restores the user configuration and restores the enable state.
*
* Parameters:
*  None
*
* Return:
*  None
*
*******************************************************************************/
void TCPWM_LED_Wakeup(void)
{
    TCPWM_LED_RestoreConfig();

    if(0u != TCPWM_LED_backup.enableState)
    {
        TCPWM_LED_Enable();
    }
}


/* [] END OF FILE */
