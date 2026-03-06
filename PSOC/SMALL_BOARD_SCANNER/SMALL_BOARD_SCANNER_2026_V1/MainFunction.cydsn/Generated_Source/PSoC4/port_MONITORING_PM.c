/***************************************************************************//**
* \file port_MONITORING_PM.c
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

#include "port_MONITORING.h"
#include "port_MONITORING_PVT.h"

#if(port_MONITORING_SCB_MODE_I2C_INC)
    #include "port_MONITORING_I2C_PVT.h"
#endif /* (port_MONITORING_SCB_MODE_I2C_INC) */

#if(port_MONITORING_SCB_MODE_EZI2C_INC)
    #include "port_MONITORING_EZI2C_PVT.h"
#endif /* (port_MONITORING_SCB_MODE_EZI2C_INC) */

#if(port_MONITORING_SCB_MODE_SPI_INC || port_MONITORING_SCB_MODE_UART_INC)
    #include "port_MONITORING_SPI_UART_PVT.h"
#endif /* (port_MONITORING_SCB_MODE_SPI_INC || port_MONITORING_SCB_MODE_UART_INC) */


/***************************************
*   Backup Structure declaration
***************************************/

#if(port_MONITORING_SCB_MODE_UNCONFIG_CONST_CFG || \
   (port_MONITORING_SCB_MODE_I2C_CONST_CFG   && (!port_MONITORING_I2C_WAKE_ENABLE_CONST))   || \
   (port_MONITORING_SCB_MODE_EZI2C_CONST_CFG && (!port_MONITORING_EZI2C_WAKE_ENABLE_CONST)) || \
   (port_MONITORING_SCB_MODE_SPI_CONST_CFG   && (!port_MONITORING_SPI_WAKE_ENABLE_CONST))   || \
   (port_MONITORING_SCB_MODE_UART_CONST_CFG  && (!port_MONITORING_UART_WAKE_ENABLE_CONST)))

    port_MONITORING_BACKUP_STRUCT port_MONITORING_backup =
    {
        0u, /* enableState */
    };
#endif


/*******************************************************************************
* Function Name: port_MONITORING_Sleep
****************************************************************************//**
*
*  Prepares the port_MONITORING component to enter Deep Sleep.
*  The “Enable wakeup from Deep Sleep Mode” selection has an influence on this 
*  function implementation:
*  - Checked: configures the component to be wakeup source from Deep Sleep.
*  - Unchecked: stores the current component state (enabled or disabled) and 
*    disables the component. See SCB_Stop() function for details about component 
*    disabling.
*
*  Call the port_MONITORING_Sleep() function before calling the 
*  CyPmSysDeepSleep() function. 
*  Refer to the PSoC Creator System Reference Guide for more information about 
*  power management functions and Low power section of this document for the 
*  selected mode.
*
*  This function should not be called before entering Sleep.
*
*******************************************************************************/
void port_MONITORING_Sleep(void)
{
#if(port_MONITORING_SCB_MODE_UNCONFIG_CONST_CFG)

    if(port_MONITORING_SCB_WAKE_ENABLE_CHECK)
    {
        if(port_MONITORING_SCB_MODE_I2C_RUNTM_CFG)
        {
            port_MONITORING_I2CSaveConfig();
        }
        else if(port_MONITORING_SCB_MODE_EZI2C_RUNTM_CFG)
        {
            port_MONITORING_EzI2CSaveConfig();
        }
    #if(!port_MONITORING_CY_SCBIP_V1)
        else if(port_MONITORING_SCB_MODE_SPI_RUNTM_CFG)
        {
            port_MONITORING_SpiSaveConfig();
        }
        else if(port_MONITORING_SCB_MODE_UART_RUNTM_CFG)
        {
            port_MONITORING_UartSaveConfig();
        }
    #endif /* (!port_MONITORING_CY_SCBIP_V1) */
        else
        {
            /* Unknown mode */
        }
    }
    else
    {
        port_MONITORING_backup.enableState = (uint8) port_MONITORING_GET_CTRL_ENABLED;

        if(0u != port_MONITORING_backup.enableState)
        {
            port_MONITORING_Stop();
        }
    }

#else

    #if (port_MONITORING_SCB_MODE_I2C_CONST_CFG && port_MONITORING_I2C_WAKE_ENABLE_CONST)
        port_MONITORING_I2CSaveConfig();

    #elif (port_MONITORING_SCB_MODE_EZI2C_CONST_CFG && port_MONITORING_EZI2C_WAKE_ENABLE_CONST)
        port_MONITORING_EzI2CSaveConfig();

    #elif (port_MONITORING_SCB_MODE_SPI_CONST_CFG && port_MONITORING_SPI_WAKE_ENABLE_CONST)
        port_MONITORING_SpiSaveConfig();

    #elif (port_MONITORING_SCB_MODE_UART_CONST_CFG && port_MONITORING_UART_WAKE_ENABLE_CONST)
        port_MONITORING_UartSaveConfig();

    #else

        port_MONITORING_backup.enableState = (uint8) port_MONITORING_GET_CTRL_ENABLED;

        if(0u != port_MONITORING_backup.enableState)
        {
            port_MONITORING_Stop();
        }

    #endif /* defined (port_MONITORING_SCB_MODE_I2C_CONST_CFG) && (port_MONITORING_I2C_WAKE_ENABLE_CONST) */

#endif /* (port_MONITORING_SCB_MODE_UNCONFIG_CONST_CFG) */
}


/*******************************************************************************
* Function Name: port_MONITORING_Wakeup
****************************************************************************//**
*
*  Prepares the port_MONITORING component for Active mode operation after 
*  Deep Sleep.
*  The “Enable wakeup from Deep Sleep Mode” selection has influence on this 
*  function implementation:
*  - Checked: restores the component Active mode configuration.
*  - Unchecked: enables the component if it was enabled before enter Deep Sleep.
*
*  This function should not be called after exiting Sleep.
*
*  \sideeffect
*   Calling the port_MONITORING_Wakeup() function without first calling the 
*   port_MONITORING_Sleep() function may produce unexpected behavior.
*
*******************************************************************************/
void port_MONITORING_Wakeup(void)
{
#if(port_MONITORING_SCB_MODE_UNCONFIG_CONST_CFG)

    if(port_MONITORING_SCB_WAKE_ENABLE_CHECK)
    {
        if(port_MONITORING_SCB_MODE_I2C_RUNTM_CFG)
        {
            port_MONITORING_I2CRestoreConfig();
        }
        else if(port_MONITORING_SCB_MODE_EZI2C_RUNTM_CFG)
        {
            port_MONITORING_EzI2CRestoreConfig();
        }
    #if(!port_MONITORING_CY_SCBIP_V1)
        else if(port_MONITORING_SCB_MODE_SPI_RUNTM_CFG)
        {
            port_MONITORING_SpiRestoreConfig();
        }
        else if(port_MONITORING_SCB_MODE_UART_RUNTM_CFG)
        {
            port_MONITORING_UartRestoreConfig();
        }
    #endif /* (!port_MONITORING_CY_SCBIP_V1) */
        else
        {
            /* Unknown mode */
        }
    }
    else
    {
        if(0u != port_MONITORING_backup.enableState)
        {
            port_MONITORING_Enable();
        }
    }

#else

    #if (port_MONITORING_SCB_MODE_I2C_CONST_CFG  && port_MONITORING_I2C_WAKE_ENABLE_CONST)
        port_MONITORING_I2CRestoreConfig();

    #elif (port_MONITORING_SCB_MODE_EZI2C_CONST_CFG && port_MONITORING_EZI2C_WAKE_ENABLE_CONST)
        port_MONITORING_EzI2CRestoreConfig();

    #elif (port_MONITORING_SCB_MODE_SPI_CONST_CFG && port_MONITORING_SPI_WAKE_ENABLE_CONST)
        port_MONITORING_SpiRestoreConfig();

    #elif (port_MONITORING_SCB_MODE_UART_CONST_CFG && port_MONITORING_UART_WAKE_ENABLE_CONST)
        port_MONITORING_UartRestoreConfig();

    #else

        if(0u != port_MONITORING_backup.enableState)
        {
            port_MONITORING_Enable();
        }

    #endif /* (port_MONITORING_I2C_WAKE_ENABLE_CONST) */

#endif /* (port_MONITORING_SCB_MODE_UNCONFIG_CONST_CFG) */
}


/* [] END OF FILE */
