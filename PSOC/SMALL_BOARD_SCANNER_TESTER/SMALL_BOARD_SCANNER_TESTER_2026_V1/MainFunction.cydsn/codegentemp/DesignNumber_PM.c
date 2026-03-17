/*******************************************************************************
* File Name: DesignNumber_PM.c
* Version 1.80
*
* Description:
*  This file contains the setup, control, and status commands to support 
*  the component operation in the low power mode. 
*
* Note:
*
********************************************************************************
* Copyright 2015, Cypress Semiconductor Corporation.  All rights reserved.
* You may use this file only in accordance with the license, terms, conditions, 
* disclaimers, and limitations in the end user license agreement accompanying 
* the software package with which this file was provided.
*******************************************************************************/

#include "DesignNumber.h"

/* Check for removal by optimization */
#if !defined(DesignNumber_Sync_ctrl_reg__REMOVED)

static DesignNumber_BACKUP_STRUCT  DesignNumber_backup = {0u};

    
/*******************************************************************************
* Function Name: DesignNumber_SaveConfig
********************************************************************************
*
* Summary:
*  Saves the control register value.
*
* Parameters:
*  None
*
* Return:
*  None
*
*******************************************************************************/
void DesignNumber_SaveConfig(void) 
{
    DesignNumber_backup.controlState = DesignNumber_Control;
}


/*******************************************************************************
* Function Name: DesignNumber_RestoreConfig
********************************************************************************
*
* Summary:
*  Restores the control register value.
*
* Parameters:
*  None
*
* Return:
*  None
*
*
*******************************************************************************/
void DesignNumber_RestoreConfig(void) 
{
     DesignNumber_Control = DesignNumber_backup.controlState;
}


/*******************************************************************************
* Function Name: DesignNumber_Sleep
********************************************************************************
*
* Summary:
*  Prepares the component for entering the low power mode.
*
* Parameters:
*  None
*
* Return:
*  None
*
*******************************************************************************/
void DesignNumber_Sleep(void) 
{
    DesignNumber_SaveConfig();
}


/*******************************************************************************
* Function Name: DesignNumber_Wakeup
********************************************************************************
*
* Summary:
*  Restores the component after waking up from the low power mode.
*
* Parameters:
*  None
*
* Return:
*  None
*
*******************************************************************************/
void DesignNumber_Wakeup(void)  
{
    DesignNumber_RestoreConfig();
}

#endif /* End check for removal by optimization */


/* [] END OF FILE */
