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

#if !defined(CY_SCB_PVT_USB_OP_H)
#define CY_SCB_PVT_USB_OP_H

#include "USB_OP.h"


/***************************************
*     Private Function Prototypes
***************************************/

/* APIs to service INTR_I2C_EC register */
#define USB_OP_SetI2CExtClkInterruptMode(interruptMask) USB_OP_WRITE_INTR_I2C_EC_MASK(interruptMask)
#define USB_OP_ClearI2CExtClkInterruptSource(interruptMask) USB_OP_CLEAR_INTR_I2C_EC(interruptMask)
#define USB_OP_GetI2CExtClkInterruptSource()                (USB_OP_INTR_I2C_EC_REG)
#define USB_OP_GetI2CExtClkInterruptMode()                  (USB_OP_INTR_I2C_EC_MASK_REG)
#define USB_OP_GetI2CExtClkInterruptSourceMasked()          (USB_OP_INTR_I2C_EC_MASKED_REG)

#if (!USB_OP_CY_SCBIP_V1)
    /* APIs to service INTR_SPI_EC register */
    #define USB_OP_SetSpiExtClkInterruptMode(interruptMask) \
                                                                USB_OP_WRITE_INTR_SPI_EC_MASK(interruptMask)
    #define USB_OP_ClearSpiExtClkInterruptSource(interruptMask) \
                                                                USB_OP_CLEAR_INTR_SPI_EC(interruptMask)
    #define USB_OP_GetExtSpiClkInterruptSource()                 (USB_OP_INTR_SPI_EC_REG)
    #define USB_OP_GetExtSpiClkInterruptMode()                   (USB_OP_INTR_SPI_EC_MASK_REG)
    #define USB_OP_GetExtSpiClkInterruptSourceMasked()           (USB_OP_INTR_SPI_EC_MASKED_REG)
#endif /* (!USB_OP_CY_SCBIP_V1) */

#if(USB_OP_SCB_MODE_UNCONFIG_CONST_CFG)
    extern void USB_OP_SetPins(uint32 mode, uint32 subMode, uint32 uartEnableMask);
#endif /* (USB_OP_SCB_MODE_UNCONFIG_CONST_CFG) */


/***************************************
*     Vars with External Linkage
***************************************/

#if (USB_OP_SCB_IRQ_INTERNAL)
#if !defined (CY_REMOVE_USB_OP_CUSTOM_INTR_HANDLER)
    extern cyisraddress USB_OP_customIntrHandler;
#endif /* !defined (CY_REMOVE_USB_OP_CUSTOM_INTR_HANDLER) */
#endif /* (USB_OP_SCB_IRQ_INTERNAL) */

extern USB_OP_BACKUP_STRUCT USB_OP_backup;

#if(USB_OP_SCB_MODE_UNCONFIG_CONST_CFG)
    /* Common configuration variables */
    extern uint8 USB_OP_scbMode;
    extern uint8 USB_OP_scbEnableWake;
    extern uint8 USB_OP_scbEnableIntr;

    /* I2C configuration variables */
    extern uint8 USB_OP_mode;
    extern uint8 USB_OP_acceptAddr;

    /* SPI/UART configuration variables */
    extern volatile uint8 * USB_OP_rxBuffer;
    extern uint8   USB_OP_rxDataBits;
    extern uint32  USB_OP_rxBufferSize;

    extern volatile uint8 * USB_OP_txBuffer;
    extern uint8   USB_OP_txDataBits;
    extern uint32  USB_OP_txBufferSize;

    /* EZI2C configuration variables */
    extern uint8 USB_OP_numberOfAddr;
    extern uint8 USB_OP_subAddrSize;
#endif /* (USB_OP_SCB_MODE_UNCONFIG_CONST_CFG) */

#if (! (USB_OP_SCB_MODE_I2C_CONST_CFG || \
        USB_OP_SCB_MODE_EZI2C_CONST_CFG))
    extern uint16 USB_OP_IntrTxMask;
#endif /* (! (USB_OP_SCB_MODE_I2C_CONST_CFG || \
              USB_OP_SCB_MODE_EZI2C_CONST_CFG)) */


/***************************************
*        Conditional Macro
****************************************/

#if(USB_OP_SCB_MODE_UNCONFIG_CONST_CFG)
    /* Defines run time operation mode */
    #define USB_OP_SCB_MODE_I2C_RUNTM_CFG     (USB_OP_SCB_MODE_I2C      == USB_OP_scbMode)
    #define USB_OP_SCB_MODE_SPI_RUNTM_CFG     (USB_OP_SCB_MODE_SPI      == USB_OP_scbMode)
    #define USB_OP_SCB_MODE_UART_RUNTM_CFG    (USB_OP_SCB_MODE_UART     == USB_OP_scbMode)
    #define USB_OP_SCB_MODE_EZI2C_RUNTM_CFG   (USB_OP_SCB_MODE_EZI2C    == USB_OP_scbMode)
    #define USB_OP_SCB_MODE_UNCONFIG_RUNTM_CFG \
                                                        (USB_OP_SCB_MODE_UNCONFIG == USB_OP_scbMode)

    /* Defines wakeup enable */
    #define USB_OP_SCB_WAKE_ENABLE_CHECK       (0u != USB_OP_scbEnableWake)
#endif /* (USB_OP_SCB_MODE_UNCONFIG_CONST_CFG) */

/* Defines maximum number of SCB pins */
#if (!USB_OP_CY_SCBIP_V1)
    #define USB_OP_SCB_PINS_NUMBER    (7u)
#else
    #define USB_OP_SCB_PINS_NUMBER    (2u)
#endif /* (!USB_OP_CY_SCBIP_V1) */

#endif /* (CY_SCB_PVT_USB_OP_H) */


/* [] END OF FILE */
