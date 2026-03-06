/***************************************************************************//**
* \file OP_ST500_PM.c
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

#include "OP_ST500.h"
#include "OP_ST500_PVT.h"

#if(OP_ST500_SCB_MODE_I2C_INC)
    #include "OP_ST500_I2C_PVT.h"
#endif /* (OP_ST500_SCB_MODE_I2C_INC) */

#if(OP_ST500_SCB_MODE_EZI2C_INC)
    #include "OP_ST500_EZI2C_PVT.h"
#endif /* (OP_ST500_SCB_MODE_EZI2C_INC) */

#if(OP_ST500_SCB_MODE_SPI_INC || OP_ST500_SCB_MODE_UART_INC)
    #include "OP_ST500_SPI_UART_PVT.h"
#endif /* (OP_ST500_SCB_MODE_SPI_INC || OP_ST500_SCB_MODE_UART_INC) */


/***************************************
*   Backup Structure declaration
***************************************/

#if(OP_ST500_SCB_MODE_UNCONFIG_CONST_CFG || \
   (OP_ST500_SCB_MODE_I2C_CONST_CFG   && (!OP_ST500_I2C_WAKE_ENABLE_CONST))   || \
   (OP_ST500_SCB_MODE_EZI2C_CONST_CFG && (!OP_ST500_EZI2C_WAKE_ENABLE_CONST)) || \
   (OP_ST500_SCB_MODE_SPI_CONST_CFG   && (!OP_ST500_SPI_WAKE_ENABLE_CONST))   || \
   (OP_ST500_SCB_MODE_UART_CONST_CFG  && (!OP_ST500_UART_WAKE_ENABLE_CONST)))

    OP_ST500_BACKUP_STRUCT OP_ST500_backup =
    {
        0u, /* enableState */
    };
#endif


/*******************************************************************************
* Function Name: OP_ST500_Sleep
****************************************************************************//**
*
*  Prepares the OP_ST500 component to enter Deep Sleep.
*  The “Enable wakeup from Deep Sleep Mode” selection has an influence on this 
*  function implementation:
*  - Checked: configures the component to be wakeup source from Deep Sleep.
*  - Unchecked: stores the current component state (enabled or disabled) and 
*    disables the component. See SCB_Stop() function for details about component 
*    disabling.
*
*  Call the OP_ST500_Sleep() function before calling the 
*  CyPmSysDeepSleep() function. 
*  Refer to the PSoC Creator System Reference Guide for more information about 
*  power management functions and Low power section of this document for the 
*  selected mode.
*
*  This function should not be called before entering Sleep.
*
*******************************************************************************/
void OP_ST500_Sleep(void)
{
#if(OP_ST500_SCB_MODE_UNCONFIG_CONST_CFG)

    if(OP_ST500_SCB_WAKE_ENABLE_CHECK)
    {
        if(OP_ST500_SCB_MODE_I2C_RUNTM_CFG)
        {
            OP_ST500_I2CSaveConfig();
        }
        else if(OP_ST500_SCB_MODE_EZI2C_RUNTM_CFG)
        {
            OP_ST500_EzI2CSaveConfig();
        }
    #if(!OP_ST500_CY_SCBIP_V1)
        else if(OP_ST500_SCB_MODE_SPI_RUNTM_CFG)
        {
            OP_ST500_SpiSaveConfig();
        }
        else if(OP_ST500_SCB_MODE_UART_RUNTM_CFG)
        {
            OP_ST500_UartSaveConfig();
        }
    #endif /* (!OP_ST500_CY_SCBIP_V1) */
        else
        {
            /* Unknown mode */
        }
    }
    else
    {
        OP_ST500_backup.enableState = (uint8) OP_ST500_GET_CTRL_ENABLED;

        if(0u != OP_ST500_backup.enableState)
        {
            OP_ST500_Stop();
        }
    }

#else

    #if (OP_ST500_SCB_MODE_I2C_CONST_CFG && OP_ST500_I2C_WAKE_ENABLE_CONST)
        OP_ST500_I2CSaveConfig();

    #elif (OP_ST500_SCB_MODE_EZI2C_CONST_CFG && OP_ST500_EZI2C_WAKE_ENABLE_CONST)
        OP_ST500_EzI2CSaveConfig();

    #elif (OP_ST500_SCB_MODE_SPI_CONST_CFG && OP_ST500_SPI_WAKE_ENABLE_CONST)
        OP_ST500_SpiSaveConfig();

    #elif (OP_ST500_SCB_MODE_UART_CONST_CFG && OP_ST500_UART_WAKE_ENABLE_CONST)
        OP_ST500_UartSaveConfig();

    #else

        OP_ST500_backup.enableState = (uint8) OP_ST500_GET_CTRL_ENABLED;

        if(0u != OP_ST500_backup.enableState)
        {
            OP_ST500_Stop();
        }

    #endif /* defined (OP_ST500_SCB_MODE_I2C_CONST_CFG) && (OP_ST500_I2C_WAKE_ENABLE_CONST) */

#endif /* (OP_ST500_SCB_MODE_UNCONFIG_CONST_CFG) */
}


/*******************************************************************************
* Function Name: OP_ST500_Wakeup
****************************************************************************//**
*
*  Prepares the OP_ST500 component for Active mode operation after 
*  Deep Sleep.
*  The “Enable wakeup from Deep Sleep Mode” selection has influence on this 
*  function implementation:
*  - Checked: restores the component Active mode configuration.
*  - Unchecked: enables the component if it was enabled before enter Deep Sleep.
*
*  This function should not be called after exiting Sleep.
*
*  \sideeffect
*   Calling the OP_ST500_Wakeup() function without first calling the 
*   OP_ST500_Sleep() function may produce unexpected behavior.
*
*******************************************************************************/
void OP_ST500_Wakeup(void)
{
#if(OP_ST500_SCB_MODE_UNCONFIG_CONST_CFG)

    if(OP_ST500_SCB_WAKE_ENABLE_CHECK)
    {
        if(OP_ST500_SCB_MODE_I2C_RUNTM_CFG)
        {
            OP_ST500_I2CRestoreConfig();
        }
        else if(OP_ST500_SCB_MODE_EZI2C_RUNTM_CFG)
        {
            OP_ST500_EzI2CRestoreConfig();
        }
    #if(!OP_ST500_CY_SCBIP_V1)
        else if(OP_ST500_SCB_MODE_SPI_RUNTM_CFG)
        {
            OP_ST500_SpiRestoreConfig();
        }
        else if(OP_ST500_SCB_MODE_UART_RUNTM_CFG)
        {
            OP_ST500_UartRestoreConfig();
        }
    #endif /* (!OP_ST500_CY_SCBIP_V1) */
        else
        {
            /* Unknown mode */
        }
    }
    else
    {
        if(0u != OP_ST500_backup.enableState)
        {
            OP_ST500_Enable();
        }
    }

#else

    #if (OP_ST500_SCB_MODE_I2C_CONST_CFG  && OP_ST500_I2C_WAKE_ENABLE_CONST)
        OP_ST500_I2CRestoreConfig();

    #elif (OP_ST500_SCB_MODE_EZI2C_CONST_CFG && OP_ST500_EZI2C_WAKE_ENABLE_CONST)
        OP_ST500_EzI2CRestoreConfig();

    #elif (OP_ST500_SCB_MODE_SPI_CONST_CFG && OP_ST500_SPI_WAKE_ENABLE_CONST)
        OP_ST500_SpiRestoreConfig();

    #elif (OP_ST500_SCB_MODE_UART_CONST_CFG && OP_ST500_UART_WAKE_ENABLE_CONST)
        OP_ST500_UartRestoreConfig();

    #else

        if(0u != OP_ST500_backup.enableState)
        {
            OP_ST500_Enable();
        }

    #endif /* (OP_ST500_I2C_WAKE_ENABLE_CONST) */

#endif /* (OP_ST500_SCB_MODE_UNCONFIG_CONST_CFG) */
}


/* [] END OF FILE */
