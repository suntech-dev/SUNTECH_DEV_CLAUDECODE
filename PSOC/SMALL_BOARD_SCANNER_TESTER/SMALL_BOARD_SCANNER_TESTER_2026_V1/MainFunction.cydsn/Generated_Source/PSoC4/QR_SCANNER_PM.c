/***************************************************************************//**
* \file QR_SCANNER_PM.c
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

#include "QR_SCANNER.h"
#include "QR_SCANNER_PVT.h"

#if(QR_SCANNER_SCB_MODE_I2C_INC)
    #include "QR_SCANNER_I2C_PVT.h"
#endif /* (QR_SCANNER_SCB_MODE_I2C_INC) */

#if(QR_SCANNER_SCB_MODE_EZI2C_INC)
    #include "QR_SCANNER_EZI2C_PVT.h"
#endif /* (QR_SCANNER_SCB_MODE_EZI2C_INC) */

#if(QR_SCANNER_SCB_MODE_SPI_INC || QR_SCANNER_SCB_MODE_UART_INC)
    #include "QR_SCANNER_SPI_UART_PVT.h"
#endif /* (QR_SCANNER_SCB_MODE_SPI_INC || QR_SCANNER_SCB_MODE_UART_INC) */


/***************************************
*   Backup Structure declaration
***************************************/

#if(QR_SCANNER_SCB_MODE_UNCONFIG_CONST_CFG || \
   (QR_SCANNER_SCB_MODE_I2C_CONST_CFG   && (!QR_SCANNER_I2C_WAKE_ENABLE_CONST))   || \
   (QR_SCANNER_SCB_MODE_EZI2C_CONST_CFG && (!QR_SCANNER_EZI2C_WAKE_ENABLE_CONST)) || \
   (QR_SCANNER_SCB_MODE_SPI_CONST_CFG   && (!QR_SCANNER_SPI_WAKE_ENABLE_CONST))   || \
   (QR_SCANNER_SCB_MODE_UART_CONST_CFG  && (!QR_SCANNER_UART_WAKE_ENABLE_CONST)))

    QR_SCANNER_BACKUP_STRUCT QR_SCANNER_backup =
    {
        0u, /* enableState */
    };
#endif


/*******************************************************************************
* Function Name: QR_SCANNER_Sleep
****************************************************************************//**
*
*  Prepares the QR_SCANNER component to enter Deep Sleep.
*  The “Enable wakeup from Deep Sleep Mode” selection has an influence on this 
*  function implementation:
*  - Checked: configures the component to be wakeup source from Deep Sleep.
*  - Unchecked: stores the current component state (enabled or disabled) and 
*    disables the component. See SCB_Stop() function for details about component 
*    disabling.
*
*  Call the QR_SCANNER_Sleep() function before calling the 
*  CyPmSysDeepSleep() function. 
*  Refer to the PSoC Creator System Reference Guide for more information about 
*  power management functions and Low power section of this document for the 
*  selected mode.
*
*  This function should not be called before entering Sleep.
*
*******************************************************************************/
void QR_SCANNER_Sleep(void)
{
#if(QR_SCANNER_SCB_MODE_UNCONFIG_CONST_CFG)

    if(QR_SCANNER_SCB_WAKE_ENABLE_CHECK)
    {
        if(QR_SCANNER_SCB_MODE_I2C_RUNTM_CFG)
        {
            QR_SCANNER_I2CSaveConfig();
        }
        else if(QR_SCANNER_SCB_MODE_EZI2C_RUNTM_CFG)
        {
            QR_SCANNER_EzI2CSaveConfig();
        }
    #if(!QR_SCANNER_CY_SCBIP_V1)
        else if(QR_SCANNER_SCB_MODE_SPI_RUNTM_CFG)
        {
            QR_SCANNER_SpiSaveConfig();
        }
        else if(QR_SCANNER_SCB_MODE_UART_RUNTM_CFG)
        {
            QR_SCANNER_UartSaveConfig();
        }
    #endif /* (!QR_SCANNER_CY_SCBIP_V1) */
        else
        {
            /* Unknown mode */
        }
    }
    else
    {
        QR_SCANNER_backup.enableState = (uint8) QR_SCANNER_GET_CTRL_ENABLED;

        if(0u != QR_SCANNER_backup.enableState)
        {
            QR_SCANNER_Stop();
        }
    }

#else

    #if (QR_SCANNER_SCB_MODE_I2C_CONST_CFG && QR_SCANNER_I2C_WAKE_ENABLE_CONST)
        QR_SCANNER_I2CSaveConfig();

    #elif (QR_SCANNER_SCB_MODE_EZI2C_CONST_CFG && QR_SCANNER_EZI2C_WAKE_ENABLE_CONST)
        QR_SCANNER_EzI2CSaveConfig();

    #elif (QR_SCANNER_SCB_MODE_SPI_CONST_CFG && QR_SCANNER_SPI_WAKE_ENABLE_CONST)
        QR_SCANNER_SpiSaveConfig();

    #elif (QR_SCANNER_SCB_MODE_UART_CONST_CFG && QR_SCANNER_UART_WAKE_ENABLE_CONST)
        QR_SCANNER_UartSaveConfig();

    #else

        QR_SCANNER_backup.enableState = (uint8) QR_SCANNER_GET_CTRL_ENABLED;

        if(0u != QR_SCANNER_backup.enableState)
        {
            QR_SCANNER_Stop();
        }

    #endif /* defined (QR_SCANNER_SCB_MODE_I2C_CONST_CFG) && (QR_SCANNER_I2C_WAKE_ENABLE_CONST) */

#endif /* (QR_SCANNER_SCB_MODE_UNCONFIG_CONST_CFG) */
}


/*******************************************************************************
* Function Name: QR_SCANNER_Wakeup
****************************************************************************//**
*
*  Prepares the QR_SCANNER component for Active mode operation after 
*  Deep Sleep.
*  The “Enable wakeup from Deep Sleep Mode” selection has influence on this 
*  function implementation:
*  - Checked: restores the component Active mode configuration.
*  - Unchecked: enables the component if it was enabled before enter Deep Sleep.
*
*  This function should not be called after exiting Sleep.
*
*  \sideeffect
*   Calling the QR_SCANNER_Wakeup() function without first calling the 
*   QR_SCANNER_Sleep() function may produce unexpected behavior.
*
*******************************************************************************/
void QR_SCANNER_Wakeup(void)
{
#if(QR_SCANNER_SCB_MODE_UNCONFIG_CONST_CFG)

    if(QR_SCANNER_SCB_WAKE_ENABLE_CHECK)
    {
        if(QR_SCANNER_SCB_MODE_I2C_RUNTM_CFG)
        {
            QR_SCANNER_I2CRestoreConfig();
        }
        else if(QR_SCANNER_SCB_MODE_EZI2C_RUNTM_CFG)
        {
            QR_SCANNER_EzI2CRestoreConfig();
        }
    #if(!QR_SCANNER_CY_SCBIP_V1)
        else if(QR_SCANNER_SCB_MODE_SPI_RUNTM_CFG)
        {
            QR_SCANNER_SpiRestoreConfig();
        }
        else if(QR_SCANNER_SCB_MODE_UART_RUNTM_CFG)
        {
            QR_SCANNER_UartRestoreConfig();
        }
    #endif /* (!QR_SCANNER_CY_SCBIP_V1) */
        else
        {
            /* Unknown mode */
        }
    }
    else
    {
        if(0u != QR_SCANNER_backup.enableState)
        {
            QR_SCANNER_Enable();
        }
    }

#else

    #if (QR_SCANNER_SCB_MODE_I2C_CONST_CFG  && QR_SCANNER_I2C_WAKE_ENABLE_CONST)
        QR_SCANNER_I2CRestoreConfig();

    #elif (QR_SCANNER_SCB_MODE_EZI2C_CONST_CFG && QR_SCANNER_EZI2C_WAKE_ENABLE_CONST)
        QR_SCANNER_EzI2CRestoreConfig();

    #elif (QR_SCANNER_SCB_MODE_SPI_CONST_CFG && QR_SCANNER_SPI_WAKE_ENABLE_CONST)
        QR_SCANNER_SpiRestoreConfig();

    #elif (QR_SCANNER_SCB_MODE_UART_CONST_CFG && QR_SCANNER_UART_WAKE_ENABLE_CONST)
        QR_SCANNER_UartRestoreConfig();

    #else

        if(0u != QR_SCANNER_backup.enableState)
        {
            QR_SCANNER_Enable();
        }

    #endif /* (QR_SCANNER_I2C_WAKE_ENABLE_CONST) */

#endif /* (QR_SCANNER_SCB_MODE_UNCONFIG_CONST_CFG) */
}


/* [] END OF FILE */
