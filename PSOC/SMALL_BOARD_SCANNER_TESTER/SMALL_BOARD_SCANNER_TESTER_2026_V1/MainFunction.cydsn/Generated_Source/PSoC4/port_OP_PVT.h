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

#if !defined(CY_SCB_PVT_port_OP_H)
#define CY_SCB_PVT_port_OP_H

#include "port_OP.h"


/***************************************
*     Private Function Prototypes
***************************************/

/* APIs to service INTR_I2C_EC register */
#define port_OP_SetI2CExtClkInterruptMode(interruptMask) port_OP_WRITE_INTR_I2C_EC_MASK(interruptMask)
#define port_OP_ClearI2CExtClkInterruptSource(interruptMask) port_OP_CLEAR_INTR_I2C_EC(interruptMask)
#define port_OP_GetI2CExtClkInterruptSource()                (port_OP_INTR_I2C_EC_REG)
#define port_OP_GetI2CExtClkInterruptMode()                  (port_OP_INTR_I2C_EC_MASK_REG)
#define port_OP_GetI2CExtClkInterruptSourceMasked()          (port_OP_INTR_I2C_EC_MASKED_REG)

#if (!port_OP_CY_SCBIP_V1)
    /* APIs to service INTR_SPI_EC register */
    #define port_OP_SetSpiExtClkInterruptMode(interruptMask) \
                                                                port_OP_WRITE_INTR_SPI_EC_MASK(interruptMask)
    #define port_OP_ClearSpiExtClkInterruptSource(interruptMask) \
                                                                port_OP_CLEAR_INTR_SPI_EC(interruptMask)
    #define port_OP_GetExtSpiClkInterruptSource()                 (port_OP_INTR_SPI_EC_REG)
    #define port_OP_GetExtSpiClkInterruptMode()                   (port_OP_INTR_SPI_EC_MASK_REG)
    #define port_OP_GetExtSpiClkInterruptSourceMasked()           (port_OP_INTR_SPI_EC_MASKED_REG)
#endif /* (!port_OP_CY_SCBIP_V1) */

#if(port_OP_SCB_MODE_UNCONFIG_CONST_CFG)
    extern void port_OP_SetPins(uint32 mode, uint32 subMode, uint32 uartEnableMask);
#endif /* (port_OP_SCB_MODE_UNCONFIG_CONST_CFG) */


/***************************************
*     Vars with External Linkage
***************************************/

#if (port_OP_SCB_IRQ_INTERNAL)
#if !defined (CY_REMOVE_port_OP_CUSTOM_INTR_HANDLER)
    extern cyisraddress port_OP_customIntrHandler;
#endif /* !defined (CY_REMOVE_port_OP_CUSTOM_INTR_HANDLER) */
#endif /* (port_OP_SCB_IRQ_INTERNAL) */

extern port_OP_BACKUP_STRUCT port_OP_backup;

#if(port_OP_SCB_MODE_UNCONFIG_CONST_CFG)
    /* Common configuration variables */
    extern uint8 port_OP_scbMode;
    extern uint8 port_OP_scbEnableWake;
    extern uint8 port_OP_scbEnableIntr;

    /* I2C configuration variables */
    extern uint8 port_OP_mode;
    extern uint8 port_OP_acceptAddr;

    /* SPI/UART configuration variables */
    extern volatile uint8 * port_OP_rxBuffer;
    extern uint8   port_OP_rxDataBits;
    extern uint32  port_OP_rxBufferSize;

    extern volatile uint8 * port_OP_txBuffer;
    extern uint8   port_OP_txDataBits;
    extern uint32  port_OP_txBufferSize;

    /* EZI2C configuration variables */
    extern uint8 port_OP_numberOfAddr;
    extern uint8 port_OP_subAddrSize;
#endif /* (port_OP_SCB_MODE_UNCONFIG_CONST_CFG) */

#if (! (port_OP_SCB_MODE_I2C_CONST_CFG || \
        port_OP_SCB_MODE_EZI2C_CONST_CFG))
    extern uint16 port_OP_IntrTxMask;
#endif /* (! (port_OP_SCB_MODE_I2C_CONST_CFG || \
              port_OP_SCB_MODE_EZI2C_CONST_CFG)) */


/***************************************
*        Conditional Macro
****************************************/

#if(port_OP_SCB_MODE_UNCONFIG_CONST_CFG)
    /* Defines run time operation mode */
    #define port_OP_SCB_MODE_I2C_RUNTM_CFG     (port_OP_SCB_MODE_I2C      == port_OP_scbMode)
    #define port_OP_SCB_MODE_SPI_RUNTM_CFG     (port_OP_SCB_MODE_SPI      == port_OP_scbMode)
    #define port_OP_SCB_MODE_UART_RUNTM_CFG    (port_OP_SCB_MODE_UART     == port_OP_scbMode)
    #define port_OP_SCB_MODE_EZI2C_RUNTM_CFG   (port_OP_SCB_MODE_EZI2C    == port_OP_scbMode)
    #define port_OP_SCB_MODE_UNCONFIG_RUNTM_CFG \
                                                        (port_OP_SCB_MODE_UNCONFIG == port_OP_scbMode)

    /* Defines wakeup enable */
    #define port_OP_SCB_WAKE_ENABLE_CHECK       (0u != port_OP_scbEnableWake)
#endif /* (port_OP_SCB_MODE_UNCONFIG_CONST_CFG) */

/* Defines maximum number of SCB pins */
#if (!port_OP_CY_SCBIP_V1)
    #define port_OP_SCB_PINS_NUMBER    (7u)
#else
    #define port_OP_SCB_PINS_NUMBER    (2u)
#endif /* (!port_OP_CY_SCBIP_V1) */

#endif /* (CY_SCB_PVT_port_OP_H) */


/* [] END OF FILE */
