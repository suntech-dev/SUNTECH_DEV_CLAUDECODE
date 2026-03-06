/***************************************************************************//**
* \file .h
* \version 4.0
*
* \brief
*  This private file provides constants and parameter values for the
*  SCB Component.
*  Please do not use this file or its content in your project.
*
* Note:
*
********************************************************************************
* \copyright
* Copyright 2013-2017, Cypress Semiconductor Corporation. All rights reserved.
* You may use this file only in accordance with the license, terms, conditions,
* disclaimers, and limitations in the end user license agreement accompanying
* the software package with which this file was provided.
*******************************************************************************/

#if !defined(CY_SCB_PVT_QR_SCANNER_H)
#define CY_SCB_PVT_QR_SCANNER_H

#include "QR_SCANNER.h"


/***************************************
*     Private Function Prototypes
***************************************/

/* APIs to service INTR_I2C_EC register */
#define QR_SCANNER_SetI2CExtClkInterruptMode(interruptMask) QR_SCANNER_WRITE_INTR_I2C_EC_MASK(interruptMask)
#define QR_SCANNER_ClearI2CExtClkInterruptSource(interruptMask) QR_SCANNER_CLEAR_INTR_I2C_EC(interruptMask)
#define QR_SCANNER_GetI2CExtClkInterruptSource()                (QR_SCANNER_INTR_I2C_EC_REG)
#define QR_SCANNER_GetI2CExtClkInterruptMode()                  (QR_SCANNER_INTR_I2C_EC_MASK_REG)
#define QR_SCANNER_GetI2CExtClkInterruptSourceMasked()          (QR_SCANNER_INTR_I2C_EC_MASKED_REG)

#if (!QR_SCANNER_CY_SCBIP_V1)
    /* APIs to service INTR_SPI_EC register */
    #define QR_SCANNER_SetSpiExtClkInterruptMode(interruptMask) \
                                                                QR_SCANNER_WRITE_INTR_SPI_EC_MASK(interruptMask)
    #define QR_SCANNER_ClearSpiExtClkInterruptSource(interruptMask) \
                                                                QR_SCANNER_CLEAR_INTR_SPI_EC(interruptMask)
    #define QR_SCANNER_GetExtSpiClkInterruptSource()                 (QR_SCANNER_INTR_SPI_EC_REG)
    #define QR_SCANNER_GetExtSpiClkInterruptMode()                   (QR_SCANNER_INTR_SPI_EC_MASK_REG)
    #define QR_SCANNER_GetExtSpiClkInterruptSourceMasked()           (QR_SCANNER_INTR_SPI_EC_MASKED_REG)
#endif /* (!QR_SCANNER_CY_SCBIP_V1) */

#if(QR_SCANNER_SCB_MODE_UNCONFIG_CONST_CFG)
    extern void QR_SCANNER_SetPins(uint32 mode, uint32 subMode, uint32 uartEnableMask);
#endif /* (QR_SCANNER_SCB_MODE_UNCONFIG_CONST_CFG) */


/***************************************
*     Vars with External Linkage
***************************************/

#if (QR_SCANNER_SCB_IRQ_INTERNAL)
#if !defined (CY_REMOVE_QR_SCANNER_CUSTOM_INTR_HANDLER)
    extern cyisraddress QR_SCANNER_customIntrHandler;
#endif /* !defined (CY_REMOVE_QR_SCANNER_CUSTOM_INTR_HANDLER) */
#endif /* (QR_SCANNER_SCB_IRQ_INTERNAL) */

extern QR_SCANNER_BACKUP_STRUCT QR_SCANNER_backup;

#if(QR_SCANNER_SCB_MODE_UNCONFIG_CONST_CFG)
    /* Common configuration variables */
    extern uint8 QR_SCANNER_scbMode;
    extern uint8 QR_SCANNER_scbEnableWake;
    extern uint8 QR_SCANNER_scbEnableIntr;

    /* I2C configuration variables */
    extern uint8 QR_SCANNER_mode;
    extern uint8 QR_SCANNER_acceptAddr;

    /* SPI/UART configuration variables */
    extern volatile uint8 * QR_SCANNER_rxBuffer;
    extern uint8   QR_SCANNER_rxDataBits;
    extern uint32  QR_SCANNER_rxBufferSize;

    extern volatile uint8 * QR_SCANNER_txBuffer;
    extern uint8   QR_SCANNER_txDataBits;
    extern uint32  QR_SCANNER_txBufferSize;

    /* EZI2C configuration variables */
    extern uint8 QR_SCANNER_numberOfAddr;
    extern uint8 QR_SCANNER_subAddrSize;
#endif /* (QR_SCANNER_SCB_MODE_UNCONFIG_CONST_CFG) */

#if (! (QR_SCANNER_SCB_MODE_I2C_CONST_CFG || \
        QR_SCANNER_SCB_MODE_EZI2C_CONST_CFG))
    extern uint16 QR_SCANNER_IntrTxMask;
#endif /* (! (QR_SCANNER_SCB_MODE_I2C_CONST_CFG || \
              QR_SCANNER_SCB_MODE_EZI2C_CONST_CFG)) */


/***************************************
*        Conditional Macro
****************************************/

#if(QR_SCANNER_SCB_MODE_UNCONFIG_CONST_CFG)
    /* Defines run time operation mode */
    #define QR_SCANNER_SCB_MODE_I2C_RUNTM_CFG     (QR_SCANNER_SCB_MODE_I2C      == QR_SCANNER_scbMode)
    #define QR_SCANNER_SCB_MODE_SPI_RUNTM_CFG     (QR_SCANNER_SCB_MODE_SPI      == QR_SCANNER_scbMode)
    #define QR_SCANNER_SCB_MODE_UART_RUNTM_CFG    (QR_SCANNER_SCB_MODE_UART     == QR_SCANNER_scbMode)
    #define QR_SCANNER_SCB_MODE_EZI2C_RUNTM_CFG   (QR_SCANNER_SCB_MODE_EZI2C    == QR_SCANNER_scbMode)
    #define QR_SCANNER_SCB_MODE_UNCONFIG_RUNTM_CFG \
                                                        (QR_SCANNER_SCB_MODE_UNCONFIG == QR_SCANNER_scbMode)

    /* Defines wakeup enable */
    #define QR_SCANNER_SCB_WAKE_ENABLE_CHECK       (0u != QR_SCANNER_scbEnableWake)
#endif /* (QR_SCANNER_SCB_MODE_UNCONFIG_CONST_CFG) */

/* Defines maximum number of SCB pins */
#if (!QR_SCANNER_CY_SCBIP_V1)
    #define QR_SCANNER_SCB_PINS_NUMBER    (7u)
#else
    #define QR_SCANNER_SCB_PINS_NUMBER    (2u)
#endif /* (!QR_SCANNER_CY_SCBIP_V1) */

#endif /* (CY_SCB_PVT_QR_SCANNER_H) */


/* [] END OF FILE */
