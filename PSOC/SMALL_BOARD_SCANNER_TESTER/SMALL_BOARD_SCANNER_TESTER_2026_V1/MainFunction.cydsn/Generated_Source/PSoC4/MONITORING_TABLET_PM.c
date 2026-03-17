/***************************************************************************//**
* \file MONITORING_TABLET_PM.c
* \version 4.0
*
* \brief
*  This file provides the source code to the Power Management support for
*  the SCB Component.
*
* Note:
*
********************************************************************************
* \copyright
* Copyright 2013-2017, Cypress Semiconductor Corporation.  All rights reserved.
* You may use this file only in accordance with the license, terms, conditions,
* disclaimers, and limitations in the end user license agreement accompanying
* the software package with which this file was provided.
*******************************************************************************/

#include "MONITORING_TABLET.h"
#include "MONITORING_TABLET_PVT.h"

#if(MONITORING_TABLET_SCB_MODE_I2C_INC)
    #include "MONITORING_TABLET_I2C_PVT.h"
#endif /* (MONITORING_TABLET_SCB_MODE_I2C_INC) */

#if(MONITORING_TABLET_SCB_MODE_EZI2C_INC)
    #include "MONITORING_TABLET_EZI2C_PVT.h"
#endif /* (MONITORING_TABLET_SCB_MODE_EZI2C_INC) */

#if(MONITORING_TABLET_SCB_MODE_SPI_INC || MONITORING_TABLET_SCB_MODE_UART_INC)
    #include "MONITORING_TABLET_SPI_UART_PVT.h"
#endif /* (MONITORING_TABLET_SCB_MODE_SPI_INC || MONITORING_TABLET_SCB_MODE_UART_INC) */


/***************************************
*   Backup Structure declaration
***************************************/

#if(MONITORING_TABLET_SCB_MODE_UNCONFIG_CONST_CFG || \
   (MONITORING_TABLET_SCB_MODE_I2C_CONST_CFG   && (!MONITORING_TABLET_I2C_WAKE_ENABLE_CONST))   || \
   (MONITORING_TABLET_SCB_MODE_EZI2C_CONST_CFG && (!MONITORING_TABLET_EZI2C_WAKE_ENABLE_CONST)) || \
   (MONITORING_TABLET_SCB_MODE_SPI_CONST_CFG   && (!MONITORING_TABLET_SPI_WAKE_ENABLE_CONST))   || \
   (MONITORING_TABLET_SCB_MODE_UART_CONST_CFG  && (!MONITORING_TABLET_UART_WAKE_ENABLE_CONST)))

    MONITORING_TABLET_BACKUP_STRUCT MONITORING_TABLET_backup =
    {
        0u, /* enableState */
    };
#endif


/*******************************************************************************
* Function Name: MONITORING_TABLET_Sleep
****************************************************************************//**
*
*  Prepares the MONITORING_TABLET component to enter Deep Sleep.
*  The “Enable wakeup from Deep Sleep Mode” selection has an influence on this 
*  function implementation:
*  - Checked: configures the component to be wakeup source from Deep Sleep.
*  - Unchecked: stores the current component state (enabled or disabled) and 
*    disables the component. See SCB_Stop() function for details about component 
*    disabling.
*
*  Call the MONITORING_TABLET_Sleep() function before calling the 
*  CyPmSysDeepSleep() function. 
*  Refer to the PSoC Creator System Reference Guide for more information about 
*  power management functions and Low power section of this document for the 
*  selected mode.
*
*  This function should not be called before entering Sleep.
*
*******************************************************************************/
void MONITORING_TABLET_Sleep(void)
{
#if(MONITORING_TABLET_SCB_MODE_UNCONFIG_CONST_CFG)

    if(MONITORING_TABLET_SCB_WAKE_ENABLE_CHECK)
    {
        if(MONITORING_TABLET_SCB_MODE_I2C_RUNTM_CFG)
        {
            MONITORING_TABLET_I2CSaveConfig();
        }
        else if(MONITORING_TABLET_SCB_MODE_EZI2C_RUNTM_CFG)
        {
            MONITORING_TABLET_EzI2CSaveConfig();
        }
    #if(!MONITORING_TABLET_CY_SCBIP_V1)
        else if(MONITORING_TABLET_SCB_MODE_SPI_RUNTM_CFG)
        {
            MONITORING_TABLET_SpiSaveConfig();
        }
        else if(MONITORING_TABLET_SCB_MODE_UART_RUNTM_CFG)
        {
            MONITORING_TABLET_UartSaveConfig();
        }
    #endif /* (!MONITORING_TABLET_CY_SCBIP_V1) */
        else
        {
            /* Unknown mode */
        }
    }
    else
    {
        MONITORING_TABLET_backup.enableState = (uint8) MONITORING_TABLET_GET_CTRL_ENABLED;

        if(0u != MONITORING_TABLET_backup.enableState)
        {
            MONITORING_TABLET_Stop();
        }
    }

#else

    #if (MONITORING_TABLET_SCB_MODE_I2C_CONST_CFG && MONITORING_TABLET_I2C_WAKE_ENABLE_CONST)
        MONITORING_TABLET_I2CSaveConfig();

    #elif (MONITORING_TABLET_SCB_MODE_EZI2C_CONST_CFG && MONITORING_TABLET_EZI2C_WAKE_ENABLE_CONST)
        MONITORING_TABLET_EzI2CSaveConfig();

    #elif (MONITORING_TABLET_SCB_MODE_SPI_CONST_CFG && MONITORING_TABLET_SPI_WAKE_ENABLE_CONST)
        MONITORING_TABLET_SpiSaveConfig();

    #elif (MONITORING_TABLET_SCB_MODE_UART_CONST_CFG && MONITORING_TABLET_UART_WAKE_ENABLE_CONST)
        MONITORING_TABLET_UartSaveConfig();

    #else

        MONITORING_TABLET_backup.enableState = (uint8) MONITORING_TABLET_GET_CTRL_ENABLED;

        if(0u != MONITORING_TABLET_backup.enableState)
        {
            MONITORING_TABLET_Stop();
        }

    #endif /* defined (MONITORING_TABLET_SCB_MODE_I2C_CONST_CFG) && (MONITORING_TABLET_I2C_WAKE_ENABLE_CONST) */

#endif /* (MONITORING_TABLET_SCB_MODE_UNCONFIG_CONST_CFG) */
}


/*******************************************************************************
* Function Name: MONITORING_TABLET_Wakeup
****************************************************************************//**
*
*  Prepares the MONITORING_TABLET component for Active mode operation after 
*  Deep Sleep.
*  The “Enable wakeup from Deep Sleep Mode” selection has influence on this 
*  function implementation:
*  - Checked: restores the component Active mode configuration.
*  - Unchecked: enables the component if it was enabled before enter Deep Sleep.
*
*  This function should not be called after exiting Sleep.
*
*  \sideeffect
*   Calling the MONITORING_TABLET_Wakeup() function without first calling the 
*   MONITORING_TABLET_Sleep() function may produce unexpected behavior.
*
*******************************************************************************/
void MONITORING_TABLET_Wakeup(void)
{
#if(MONITORING_TABLET_SCB_MODE_UNCONFIG_CONST_CFG)

    if(MONITORING_TABLET_SCB_WAKE_ENABLE_CHECK)
    {
        if(MONITORING_TABLET_SCB_MODE_I2C_RUNTM_CFG)
        {
            MONITORING_TABLET_I2CRestoreConfig();
        }
        else if(MONITORING_TABLET_SCB_MODE_EZI2C_RUNTM_CFG)
        {
            MONITORING_TABLET_EzI2CRestoreConfig();
        }
    #if(!MONITORING_TABLET_CY_SCBIP_V1)
        else if(MONITORING_TABLET_SCB_MODE_SPI_RUNTM_CFG)
        {
            MONITORING_TABLET_SpiRestoreConfig();
        }
        else if(MONITORING_TABLET_SCB_MODE_UART_RUNTM_CFG)
        {
            MONITORING_TABLET_UartRestoreConfig();
        }
    #endif /* (!MONITORING_TABLET_CY_SCBIP_V1) */
        else
        {
            /* Unknown mode */
        }
    }
    else
    {
        if(0u != MONITORING_TABLET_backup.enableState)
        {
            MONITORING_TABLET_Enable();
        }
    }

#else

    #if (MONITORING_TABLET_SCB_MODE_I2C_CONST_CFG  && MONITORING_TABLET_I2C_WAKE_ENABLE_CONST)
        MONITORING_TABLET_I2CRestoreConfig();

    #elif (MONITORING_TABLET_SCB_MODE_EZI2C_CONST_CFG && MONITORING_TABLET_EZI2C_WAKE_ENABLE_CONST)
        MONITORING_TABLET_EzI2CRestoreConfig();

    #elif (MONITORING_TABLET_SCB_MODE_SPI_CONST_CFG && MONITORING_TABLET_SPI_WAKE_ENABLE_CONST)
        MONITORING_TABLET_SpiRestoreConfig();

    #elif (MONITORING_TABLET_SCB_MODE_UART_CONST_CFG && MONITORING_TABLET_UART_WAKE_ENABLE_CONST)
        MONITORING_TABLET_UartRestoreConfig();

    #else

        if(0u != MONITORING_TABLET_backup.enableState)
        {
            MONITORING_TABLET_Enable();
        }

    #endif /* (MONITORING_TABLET_I2C_WAKE_ENABLE_CONST) */

#endif /* (MONITORING_TABLET_SCB_MODE_UNCONFIG_CONST_CFG) */
}


/* [] END OF FILE */
