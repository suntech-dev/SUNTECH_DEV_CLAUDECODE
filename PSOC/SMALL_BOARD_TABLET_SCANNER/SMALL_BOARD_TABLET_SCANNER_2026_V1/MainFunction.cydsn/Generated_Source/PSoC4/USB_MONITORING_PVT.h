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

#if !defined(CY_SCB_PVT_USB_MONITORING_H)
#define CY_SCB_PVT_USB_MONITORING_H

#include "USB_MONITORING.h"


/***************************************
*     Private Function Prototypes
***************************************/

/* APIs to service INTR_I2C_EC register */
#define USB_MONITORING_SetI2CExtClkInterruptMode(interruptMask) USB_MONITORING_WRITE_INTR_I2C_EC_MASK(interruptMask)
#define USB_MONITORING_ClearI2CExtClkInterruptSource(interruptMask) USB_MONITORING_CLEAR_INTR_I2C_EC(interruptMask)
#define USB_MONITORING_GetI2CExtClkInterruptSource()                (USB_MONITORING_INTR_I2C_EC_REG)
#define USB_MONITORING_GetI2CExtClkInterruptMode()                  (USB_MONITORING_INTR_I2C_EC_MASK_REG)
#define USB_MONITORING_GetI2CExtClkInterruptSourceMasked()          (USB_MONITORING_INTR_I2C_EC_MASKED_REG)

#if (!USB_MONITORING_CY_SCBIP_V1)
    /* APIs to service INTR_SPI_EC register */
    #define USB_MONITORING_SetSpiExtClkInterruptMode(interruptMask) \
                                                                USB_MONITORING_WRITE_INTR_SPI_EC_MASK(interruptMask)
    #define USB_MONITORING_ClearSpiExtClkInterruptSource(interruptMask) \
                                                                USB_MONITORING_CLEAR_INTR_SPI_EC(interruptMask)
    #define USB_MONITORING_GetExtSpiClkInterruptSource()                 (USB_MONITORING_INTR_SPI_EC_REG)
    #define USB_MONITORING_GetExtSpiClkInterruptMode()                   (USB_MONITORING_INTR_SPI_EC_MASK_REG)
    #define USB_MONITORING_GetExtSpiClkInterruptSourceMasked()           (USB_MONITORING_INTR_SPI_EC_MASKED_REG)
#endif /* (!USB_MONITORING_CY_SCBIP_V1) */

#if(USB_MONITORING_SCB_MODE_UNCONFIG_CONST_CFG)
    extern void USB_MONITORING_SetPins(uint32 mode, uint32 subMode, uint32 uartEnableMask);
#endif /* (USB_MONITORING_SCB_MODE_UNCONFIG_CONST_CFG) */


/***************************************
*     Vars with External Linkage
***************************************/

#if (USB_MONITORING_SCB_IRQ_INTERNAL)
#if !defined (CY_REMOVE_USB_MONITORING_CUSTOM_INTR_HANDLER)
    extern cyisraddress USB_MONITORING_customIntrHandler;
#endif /* !defined (CY_REMOVE_USB_MONITORING_CUSTOM_INTR_HANDLER) */
#endif /* (USB_MONITORING_SCB_IRQ_INTERNAL) */

extern USB_MONITORING_BACKUP_STRUCT USB_MONITORING_backup;

#if(USB_MONITORING_SCB_MODE_UNCONFIG_CONST_CFG)
    /* Common configuration variables */
    extern uint8 USB_MONITORING_scbMode;
    extern uint8 USB_MONITORING_scbEnableWake;
    extern uint8 USB_MONITORING_scbEnableIntr;

    /* I2C configuration variables */
    extern uint8 USB_MONITORING_mode;
    extern uint8 USB_MONITORING_acceptAddr;

    /* SPI/UART configuration variables */
    extern volatile uint8 * USB_MONITORING_rxBuffer;
    extern uint8   USB_MONITORING_rxDataBits;
    extern uint32  USB_MONITORING_rxBufferSize;

    extern volatile uint8 * USB_MONITORING_txBuffer;
    extern uint8   USB_MONITORING_txDataBits;
    extern uint32  USB_MONITORING_txBufferSize;

    /* EZI2C configuration variables */
    extern uint8 USB_MONITORING_numberOfAddr;
    extern uint8 USB_MONITORING_subAddrSize;
#endif /* (USB_MONITORING_SCB_MODE_UNCONFIG_CONST_CFG) */

#if (! (USB_MONITORING_SCB_MODE_I2C_CONST_CFG || \
        USB_MONITORING_SCB_MODE_EZI2C_CONST_CFG))
    extern uint16 USB_MONITORING_IntrTxMask;
#endif /* (! (USB_MONITORING_SCB_MODE_I2C_CONST_CFG || \
              USB_MONITORING_SCB_MODE_EZI2C_CONST_CFG)) */


/***************************************
*        Conditional Macro
****************************************/

#if(USB_MONITORING_SCB_MODE_UNCONFIG_CONST_CFG)
    /* Defines run time operation mode */
    #define USB_MONITORING_SCB_MODE_I2C_RUNTM_CFG     (USB_MONITORING_SCB_MODE_I2C      == USB_MONITORING_scbMode)
    #define USB_MONITORING_SCB_MODE_SPI_RUNTM_CFG     (USB_MONITORING_SCB_MODE_SPI      == USB_MONITORING_scbMode)
    #define USB_MONITORING_SCB_MODE_UART_RUNTM_CFG    (USB_MONITORING_SCB_MODE_UART     == USB_MONITORING_scbMode)
    #define USB_MONITORING_SCB_MODE_EZI2C_RUNTM_CFG   (USB_MONITORING_SCB_MODE_EZI2C    == USB_MONITORING_scbMode)
    #define USB_MONITORING_SCB_MODE_UNCONFIG_RUNTM_CFG \
                                                        (USB_MONITORING_SCB_MODE_UNCONFIG == USB_MONITORING_scbMode)

    /* Defines wakeup enable */
    #define USB_MONITORING_SCB_WAKE_ENABLE_CHECK       (0u != USB_MONITORING_scbEnableWake)
#endif /* (USB_MONITORING_SCB_MODE_UNCONFIG_CONST_CFG) */

/* Defines maximum number of SCB pins */
#if (!USB_MONITORING_CY_SCBIP_V1)
    #define USB_MONITORING_SCB_PINS_NUMBER    (7u)
#else
    #define USB_MONITORING_SCB_PINS_NUMBER    (2u)
#endif /* (!USB_MONITORING_CY_SCBIP_V1) */

#endif /* (CY_SCB_PVT_USB_MONITORING_H) */


/* [] END OF FILE */
