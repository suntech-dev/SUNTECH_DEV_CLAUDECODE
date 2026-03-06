/***************************************************************************//**
* \file WIFI_PM.c
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

#include "WIFI.h"
#include "WIFI_PVT.h"

#if(WIFI_SCB_MODE_I2C_INC)
    #include "WIFI_I2C_PVT.h"
#endif /* (WIFI_SCB_MODE_I2C_INC) */

#if(WIFI_SCB_MODE_EZI2C_INC)
    #include "WIFI_EZI2C_PVT.h"
#endif /* (WIFI_SCB_MODE_EZI2C_INC) */

#if(WIFI_SCB_MODE_SPI_INC || WIFI_SCB_MODE_UART_INC)
    #include "WIFI_SPI_UART_PVT.h"
#endif /* (WIFI_SCB_MODE_SPI_INC || WIFI_SCB_MODE_UART_INC) */


/***************************************
*   Backup Structure declaration
***************************************/

#if(WIFI_SCB_MODE_UNCONFIG_CONST_CFG || \
   (WIFI_SCB_MODE_I2C_CONST_CFG   && (!WIFI_I2C_WAKE_ENABLE_CONST))   || \
   (WIFI_SCB_MODE_EZI2C_CONST_CFG && (!WIFI_EZI2C_WAKE_ENABLE_CONST)) || \
   (WIFI_SCB_MODE_SPI_CONST_CFG   && (!WIFI_SPI_WAKE_ENABLE_CONST))   || \
   (WIFI_SCB_MODE_UART_CONST_CFG  && (!WIFI_UART_WAKE_ENABLE_CONST)))

    WIFI_BACKUP_STRUCT WIFI_backup =
    {
        0u, /* enableState */
    };
#endif


/*******************************************************************************
* Function Name: WIFI_Sleep
****************************************************************************//**
*
*  Prepares the WIFI component to enter Deep Sleep.
*  The “Enable wakeup from Deep Sleep Mode” selection has an influence on this 
*  function implementation:
*  - Checked: configures the component to be wakeup source from Deep Sleep.
*  - Unchecked: stores the current component state (enabled or disabled) and 
*    disables the component. See SCB_Stop() function for details about component 
*    disabling.
*
*  Call the WIFI_Sleep() function before calling the 
*  CyPmSysDeepSleep() function. 
*  Refer to the PSoC Creator System Reference Guide for more information about 
*  power management functions and Low power section of this document for the 
*  selected mode.
*
*  This function should not be called before entering Sleep.
*
*******************************************************************************/
void WIFI_Sleep(void)
{
#if(WIFI_SCB_MODE_UNCONFIG_CONST_CFG)

    if(WIFI_SCB_WAKE_ENABLE_CHECK)
    {
        if(WIFI_SCB_MODE_I2C_RUNTM_CFG)
        {
            WIFI_I2CSaveConfig();
        }
        else if(WIFI_SCB_MODE_EZI2C_RUNTM_CFG)
        {
            WIFI_EzI2CSaveConfig();
        }
    #if(!WIFI_CY_SCBIP_V1)
        else if(WIFI_SCB_MODE_SPI_RUNTM_CFG)
        {
            WIFI_SpiSaveConfig();
        }
        else if(WIFI_SCB_MODE_UART_RUNTM_CFG)
        {
            WIFI_UartSaveConfig();
        }
    #endif /* (!WIFI_CY_SCBIP_V1) */
        else
        {
            /* Unknown mode */
        }
    }
    else
    {
        WIFI_backup.enableState = (uint8) WIFI_GET_CTRL_ENABLED;

        if(0u != WIFI_backup.enableState)
        {
            WIFI_Stop();
        }
    }

#else

    #if (WIFI_SCB_MODE_I2C_CONST_CFG && WIFI_I2C_WAKE_ENABLE_CONST)
        WIFI_I2CSaveConfig();

    #elif (WIFI_SCB_MODE_EZI2C_CONST_CFG && WIFI_EZI2C_WAKE_ENABLE_CONST)
        WIFI_EzI2CSaveConfig();

    #elif (WIFI_SCB_MODE_SPI_CONST_CFG && WIFI_SPI_WAKE_ENABLE_CONST)
        WIFI_SpiSaveConfig();

    #elif (WIFI_SCB_MODE_UART_CONST_CFG && WIFI_UART_WAKE_ENABLE_CONST)
        WIFI_UartSaveConfig();

    #else

        WIFI_backup.enableState = (uint8) WIFI_GET_CTRL_ENABLED;

        if(0u != WIFI_backup.enableState)
        {
            WIFI_Stop();
        }

    #endif /* defined (WIFI_SCB_MODE_I2C_CONST_CFG) && (WIFI_I2C_WAKE_ENABLE_CONST) */

#endif /* (WIFI_SCB_MODE_UNCONFIG_CONST_CFG) */
}


/*******************************************************************************
* Function Name: WIFI_Wakeup
****************************************************************************//**
*
*  Prepares the WIFI component for Active mode operation after 
*  Deep Sleep.
*  The “Enable wakeup from Deep Sleep Mode” selection has influence on this 
*  function implementation:
*  - Checked: restores the component Active mode configuration.
*  - Unchecked: enables the component if it was enabled before enter Deep Sleep.
*
*  This function should not be called after exiting Sleep.
*
*  \sideeffect
*   Calling the WIFI_Wakeup() function without first calling the 
*   WIFI_Sleep() function may produce unexpected behavior.
*
*******************************************************************************/
void WIFI_Wakeup(void)
{
#if(WIFI_SCB_MODE_UNCONFIG_CONST_CFG)

    if(WIFI_SCB_WAKE_ENABLE_CHECK)
    {
        if(WIFI_SCB_MODE_I2C_RUNTM_CFG)
        {
            WIFI_I2CRestoreConfig();
        }
        else if(WIFI_SCB_MODE_EZI2C_RUNTM_CFG)
        {
            WIFI_EzI2CRestoreConfig();
        }
    #if(!WIFI_CY_SCBIP_V1)
        else if(WIFI_SCB_MODE_SPI_RUNTM_CFG)
        {
            WIFI_SpiRestoreConfig();
        }
        else if(WIFI_SCB_MODE_UART_RUNTM_CFG)
        {
            WIFI_UartRestoreConfig();
        }
    #endif /* (!WIFI_CY_SCBIP_V1) */
        else
        {
            /* Unknown mode */
        }
    }
    else
    {
        if(0u != WIFI_backup.enableState)
        {
            WIFI_Enable();
        }
    }

#else

    #if (WIFI_SCB_MODE_I2C_CONST_CFG  && WIFI_I2C_WAKE_ENABLE_CONST)
        WIFI_I2CRestoreConfig();

    #elif (WIFI_SCB_MODE_EZI2C_CONST_CFG && WIFI_EZI2C_WAKE_ENABLE_CONST)
        WIFI_EzI2CRestoreConfig();

    #elif (WIFI_SCB_MODE_SPI_CONST_CFG && WIFI_SPI_WAKE_ENABLE_CONST)
        WIFI_SpiRestoreConfig();

    #elif (WIFI_SCB_MODE_UART_CONST_CFG && WIFI_UART_WAKE_ENABLE_CONST)
        WIFI_UartRestoreConfig();

    #else

        if(0u != WIFI_backup.enableState)
        {
            WIFI_Enable();
        }

    #endif /* (WIFI_I2C_WAKE_ENABLE_CONST) */

#endif /* (WIFI_SCB_MODE_UNCONFIG_CONST_CFG) */
}


/* [] END OF FILE */
