/***************************************************************************//**
* \file BARCODE_PM.c
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

#include "BARCODE.h"
#include "BARCODE_PVT.h"

#if(BARCODE_SCB_MODE_I2C_INC)
    #include "BARCODE_I2C_PVT.h"
#endif /* (BARCODE_SCB_MODE_I2C_INC) */

#if(BARCODE_SCB_MODE_EZI2C_INC)
    #include "BARCODE_EZI2C_PVT.h"
#endif /* (BARCODE_SCB_MODE_EZI2C_INC) */

#if(BARCODE_SCB_MODE_SPI_INC || BARCODE_SCB_MODE_UART_INC)
    #include "BARCODE_SPI_UART_PVT.h"
#endif /* (BARCODE_SCB_MODE_SPI_INC || BARCODE_SCB_MODE_UART_INC) */


/***************************************
*   Backup Structure declaration
***************************************/

#if(BARCODE_SCB_MODE_UNCONFIG_CONST_CFG || \
   (BARCODE_SCB_MODE_I2C_CONST_CFG   && (!BARCODE_I2C_WAKE_ENABLE_CONST))   || \
   (BARCODE_SCB_MODE_EZI2C_CONST_CFG && (!BARCODE_EZI2C_WAKE_ENABLE_CONST)) || \
   (BARCODE_SCB_MODE_SPI_CONST_CFG   && (!BARCODE_SPI_WAKE_ENABLE_CONST))   || \
   (BARCODE_SCB_MODE_UART_CONST_CFG  && (!BARCODE_UART_WAKE_ENABLE_CONST)))

    BARCODE_BACKUP_STRUCT BARCODE_backup =
    {
        0u, /* enableState */
    };
#endif


/*******************************************************************************
* Function Name: BARCODE_Sleep
****************************************************************************//**
*
*  Prepares the BARCODE component to enter Deep Sleep.
*  The “Enable wakeup from Deep Sleep Mode” selection has an influence on this 
*  function implementation:
*  - Checked: configures the component to be wakeup source from Deep Sleep.
*  - Unchecked: stores the current component state (enabled or disabled) and 
*    disables the component. See SCB_Stop() function for details about component 
*    disabling.
*
*  Call the BARCODE_Sleep() function before calling the 
*  CyPmSysDeepSleep() function. 
*  Refer to the PSoC Creator System Reference Guide for more information about 
*  power management functions and Low power section of this document for the 
*  selected mode.
*
*  This function should not be called before entering Sleep.
*
*******************************************************************************/
void BARCODE_Sleep(void)
{
#if(BARCODE_SCB_MODE_UNCONFIG_CONST_CFG)

    if(BARCODE_SCB_WAKE_ENABLE_CHECK)
    {
        if(BARCODE_SCB_MODE_I2C_RUNTM_CFG)
        {
            BARCODE_I2CSaveConfig();
        }
        else if(BARCODE_SCB_MODE_EZI2C_RUNTM_CFG)
        {
            BARCODE_EzI2CSaveConfig();
        }
    #if(!BARCODE_CY_SCBIP_V1)
        else if(BARCODE_SCB_MODE_SPI_RUNTM_CFG)
        {
            BARCODE_SpiSaveConfig();
        }
        else if(BARCODE_SCB_MODE_UART_RUNTM_CFG)
        {
            BARCODE_UartSaveConfig();
        }
    #endif /* (!BARCODE_CY_SCBIP_V1) */
        else
        {
            /* Unknown mode */
        }
    }
    else
    {
        BARCODE_backup.enableState = (uint8) BARCODE_GET_CTRL_ENABLED;

        if(0u != BARCODE_backup.enableState)
        {
            BARCODE_Stop();
        }
    }

#else

    #if (BARCODE_SCB_MODE_I2C_CONST_CFG && BARCODE_I2C_WAKE_ENABLE_CONST)
        BARCODE_I2CSaveConfig();

    #elif (BARCODE_SCB_MODE_EZI2C_CONST_CFG && BARCODE_EZI2C_WAKE_ENABLE_CONST)
        BARCODE_EzI2CSaveConfig();

    #elif (BARCODE_SCB_MODE_SPI_CONST_CFG && BARCODE_SPI_WAKE_ENABLE_CONST)
        BARCODE_SpiSaveConfig();

    #elif (BARCODE_SCB_MODE_UART_CONST_CFG && BARCODE_UART_WAKE_ENABLE_CONST)
        BARCODE_UartSaveConfig();

    #else

        BARCODE_backup.enableState = (uint8) BARCODE_GET_CTRL_ENABLED;

        if(0u != BARCODE_backup.enableState)
        {
            BARCODE_Stop();
        }

    #endif /* defined (BARCODE_SCB_MODE_I2C_CONST_CFG) && (BARCODE_I2C_WAKE_ENABLE_CONST) */

#endif /* (BARCODE_SCB_MODE_UNCONFIG_CONST_CFG) */
}


/*******************************************************************************
* Function Name: BARCODE_Wakeup
****************************************************************************//**
*
*  Prepares the BARCODE component for Active mode operation after 
*  Deep Sleep.
*  The “Enable wakeup from Deep Sleep Mode” selection has influence on this 
*  function implementation:
*  - Checked: restores the component Active mode configuration.
*  - Unchecked: enables the component if it was enabled before enter Deep Sleep.
*
*  This function should not be called after exiting Sleep.
*
*  \sideeffect
*   Calling the BARCODE_Wakeup() function without first calling the 
*   BARCODE_Sleep() function may produce unexpected behavior.
*
*******************************************************************************/
void BARCODE_Wakeup(void)
{
#if(BARCODE_SCB_MODE_UNCONFIG_CONST_CFG)

    if(BARCODE_SCB_WAKE_ENABLE_CHECK)
    {
        if(BARCODE_SCB_MODE_I2C_RUNTM_CFG)
        {
            BARCODE_I2CRestoreConfig();
        }
        else if(BARCODE_SCB_MODE_EZI2C_RUNTM_CFG)
        {
            BARCODE_EzI2CRestoreConfig();
        }
    #if(!BARCODE_CY_SCBIP_V1)
        else if(BARCODE_SCB_MODE_SPI_RUNTM_CFG)
        {
            BARCODE_SpiRestoreConfig();
        }
        else if(BARCODE_SCB_MODE_UART_RUNTM_CFG)
        {
            BARCODE_UartRestoreConfig();
        }
    #endif /* (!BARCODE_CY_SCBIP_V1) */
        else
        {
            /* Unknown mode */
        }
    }
    else
    {
        if(0u != BARCODE_backup.enableState)
        {
            BARCODE_Enable();
        }
    }

#else

    #if (BARCODE_SCB_MODE_I2C_CONST_CFG  && BARCODE_I2C_WAKE_ENABLE_CONST)
        BARCODE_I2CRestoreConfig();

    #elif (BARCODE_SCB_MODE_EZI2C_CONST_CFG && BARCODE_EZI2C_WAKE_ENABLE_CONST)
        BARCODE_EzI2CRestoreConfig();

    #elif (BARCODE_SCB_MODE_SPI_CONST_CFG && BARCODE_SPI_WAKE_ENABLE_CONST)
        BARCODE_SpiRestoreConfig();

    #elif (BARCODE_SCB_MODE_UART_CONST_CFG && BARCODE_UART_WAKE_ENABLE_CONST)
        BARCODE_UartRestoreConfig();

    #else

        if(0u != BARCODE_backup.enableState)
        {
            BARCODE_Enable();
        }

    #endif /* (BARCODE_I2C_WAKE_ENABLE_CONST) */

#endif /* (BARCODE_SCB_MODE_UNCONFIG_CONST_CFG) */
}


/* [] END OF FILE */
