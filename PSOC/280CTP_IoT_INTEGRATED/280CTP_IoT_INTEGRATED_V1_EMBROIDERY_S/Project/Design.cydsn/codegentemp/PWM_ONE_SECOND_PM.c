/*******************************************************************************
* File Name: PWM_ONE_SECOND_PM.c
* Version 3.30
*
* Description:
*  This file provides the power management source code to API for the
*  PWM.
*
* Note:
*
********************************************************************************
* Copyright 2008-2014, Cypress Semiconductor Corporation.  All rights reserved.
* You may use this file only in accordance with the license, terms, conditions,
* disclaimers, and limitations in the end user license agreement accompanying
* the software package with which this file was provided.
*******************************************************************************/

#include "PWM_ONE_SECOND.h"

static PWM_ONE_SECOND_backupStruct PWM_ONE_SECOND_backup;


/*******************************************************************************
* Function Name: PWM_ONE_SECOND_SaveConfig
********************************************************************************
*
* Summary:
*  Saves the current user configuration of the component.
*
* Parameters:
*  None
*
* Return:
*  None
*
* Global variables:
*  PWM_ONE_SECOND_backup:  Variables of this global structure are modified to
*  store the values of non retention configuration registers when Sleep() API is
*  called.
*
*******************************************************************************/
void PWM_ONE_SECOND_SaveConfig(void) 
{

    #if(!PWM_ONE_SECOND_UsingFixedFunction)
        #if(!PWM_ONE_SECOND_PWMModeIsCenterAligned)
            PWM_ONE_SECOND_backup.PWMPeriod = PWM_ONE_SECOND_ReadPeriod();
        #endif /* (!PWM_ONE_SECOND_PWMModeIsCenterAligned) */
        PWM_ONE_SECOND_backup.PWMUdb = PWM_ONE_SECOND_ReadCounter();
        #if (PWM_ONE_SECOND_UseStatus)
            PWM_ONE_SECOND_backup.InterruptMaskValue = PWM_ONE_SECOND_STATUS_MASK;
        #endif /* (PWM_ONE_SECOND_UseStatus) */

        #if(PWM_ONE_SECOND_DeadBandMode == PWM_ONE_SECOND__B_PWM__DBM_256_CLOCKS || \
            PWM_ONE_SECOND_DeadBandMode == PWM_ONE_SECOND__B_PWM__DBM_2_4_CLOCKS)
            PWM_ONE_SECOND_backup.PWMdeadBandValue = PWM_ONE_SECOND_ReadDeadTime();
        #endif /*  deadband count is either 2-4 clocks or 256 clocks */

        #if(PWM_ONE_SECOND_KillModeMinTime)
             PWM_ONE_SECOND_backup.PWMKillCounterPeriod = PWM_ONE_SECOND_ReadKillTime();
        #endif /* (PWM_ONE_SECOND_KillModeMinTime) */

        #if(PWM_ONE_SECOND_UseControl)
            PWM_ONE_SECOND_backup.PWMControlRegister = PWM_ONE_SECOND_ReadControlRegister();
        #endif /* (PWM_ONE_SECOND_UseControl) */
    #endif  /* (!PWM_ONE_SECOND_UsingFixedFunction) */
}


/*******************************************************************************
* Function Name: PWM_ONE_SECOND_RestoreConfig
********************************************************************************
*
* Summary:
*  Restores the current user configuration of the component.
*
* Parameters:
*  None
*
* Return:
*  None
*
* Global variables:
*  PWM_ONE_SECOND_backup:  Variables of this global structure are used to
*  restore the values of non retention registers on wakeup from sleep mode.
*
*******************************************************************************/
void PWM_ONE_SECOND_RestoreConfig(void) 
{
        #if(!PWM_ONE_SECOND_UsingFixedFunction)
            #if(!PWM_ONE_SECOND_PWMModeIsCenterAligned)
                PWM_ONE_SECOND_WritePeriod(PWM_ONE_SECOND_backup.PWMPeriod);
            #endif /* (!PWM_ONE_SECOND_PWMModeIsCenterAligned) */

            PWM_ONE_SECOND_WriteCounter(PWM_ONE_SECOND_backup.PWMUdb);

            #if (PWM_ONE_SECOND_UseStatus)
                PWM_ONE_SECOND_STATUS_MASK = PWM_ONE_SECOND_backup.InterruptMaskValue;
            #endif /* (PWM_ONE_SECOND_UseStatus) */

            #if(PWM_ONE_SECOND_DeadBandMode == PWM_ONE_SECOND__B_PWM__DBM_256_CLOCKS || \
                PWM_ONE_SECOND_DeadBandMode == PWM_ONE_SECOND__B_PWM__DBM_2_4_CLOCKS)
                PWM_ONE_SECOND_WriteDeadTime(PWM_ONE_SECOND_backup.PWMdeadBandValue);
            #endif /* deadband count is either 2-4 clocks or 256 clocks */

            #if(PWM_ONE_SECOND_KillModeMinTime)
                PWM_ONE_SECOND_WriteKillTime(PWM_ONE_SECOND_backup.PWMKillCounterPeriod);
            #endif /* (PWM_ONE_SECOND_KillModeMinTime) */

            #if(PWM_ONE_SECOND_UseControl)
                PWM_ONE_SECOND_WriteControlRegister(PWM_ONE_SECOND_backup.PWMControlRegister);
            #endif /* (PWM_ONE_SECOND_UseControl) */
        #endif  /* (!PWM_ONE_SECOND_UsingFixedFunction) */
    }


/*******************************************************************************
* Function Name: PWM_ONE_SECOND_Sleep
********************************************************************************
*
* Summary:
*  Disables block's operation and saves the user configuration. Should be called
*  just prior to entering sleep.
*
* Parameters:
*  None
*
* Return:
*  None
*
* Global variables:
*  PWM_ONE_SECOND_backup.PWMEnableState:  Is modified depending on the enable
*  state of the block before entering sleep mode.
*
*******************************************************************************/
void PWM_ONE_SECOND_Sleep(void) 
{
    #if(PWM_ONE_SECOND_UseControl)
        if(PWM_ONE_SECOND_CTRL_ENABLE == (PWM_ONE_SECOND_CONTROL & PWM_ONE_SECOND_CTRL_ENABLE))
        {
            /*Component is enabled */
            PWM_ONE_SECOND_backup.PWMEnableState = 1u;
        }
        else
        {
            /* Component is disabled */
            PWM_ONE_SECOND_backup.PWMEnableState = 0u;
        }
    #endif /* (PWM_ONE_SECOND_UseControl) */

    /* Stop component */
    PWM_ONE_SECOND_Stop();

    /* Save registers configuration */
    PWM_ONE_SECOND_SaveConfig();
}


/*******************************************************************************
* Function Name: PWM_ONE_SECOND_Wakeup
********************************************************************************
*
* Summary:
*  Restores and enables the user configuration. Should be called just after
*  awaking from sleep.
*
* Parameters:
*  None
*
* Return:
*  None
*
* Global variables:
*  PWM_ONE_SECOND_backup.pwmEnable:  Is used to restore the enable state of
*  block on wakeup from sleep mode.
*
*******************************************************************************/
void PWM_ONE_SECOND_Wakeup(void) 
{
     /* Restore registers values */
    PWM_ONE_SECOND_RestoreConfig();

    if(PWM_ONE_SECOND_backup.PWMEnableState != 0u)
    {
        /* Enable component's operation */
        PWM_ONE_SECOND_Enable();
    } /* Do nothing if component's block was disabled before */

}


/* [] END OF FILE */
