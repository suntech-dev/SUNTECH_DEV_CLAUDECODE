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

#if !defined(CY_SCB_PVT_port_MONITORING_H)
#define CY_SCB_PVT_port_MONITORING_H

#include "port_MONITORING.h"


/***************************************
*     Private Function Prototypes
***************************************/

/* APIs to service INTR_I2C_EC register */
#define port_MONITORING_SetI2CExtClkInterruptMode(interruptMask) port_MONITORING_WRITE_INTR_I2C_EC_MASK(interruptMask)
#define port_MONITORING_ClearI2CExtClkInterruptSource(interruptMask) port_MONITORING_CLEAR_INTR_I2C_EC(interruptMask)
#define port_MONITORING_GetI2CExtClkInterruptSource()                (port_MONITORING_INTR_I2C_EC_REG)
#define port_MONITORING_GetI2CExtClkInterruptMode()                  (port_MONITORING_INTR_I2C_EC_MASK_REG)
#define port_MONITORING_GetI2CExtClkInterruptSourceMasked()          (port_MONITORING_INTR_I2C_EC_MASKED_REG)

#if (!port_MONITORING_CY_SCBIP_V1)
    /* APIs to service INTR_SPI_EC register */
    #define port_MONITORING_SetSpiExtClkInterruptMode(interruptMask) \
                                                                port_MONITORING_WRITE_INTR_SPI_EC_MASK(interruptMask)
    #define port_MONITORING_ClearSpiExtClkInterruptSource(interruptMask) \
                                                                port_MONITORING_CLEAR_INTR_SPI_EC(interruptMask)
    #define port_MONITORING_GetExtSpiClkInterruptSource()                 (port_MONITORING_INTR_SPI_EC_REG)
    #define port_MONITORING_GetExtSpiClkInterruptMode()                   (port_MONITORING_INTR_SPI_EC_MASK_REG)
    #define port_MONITORING_GetExtSpiClkInterruptSourceMasked()           (port_MONITORING_INTR_SPI_EC_MASKED_REG)
#endif /* (!port_MONITORING_CY_SCBIP_V1) */

#if(port_MONITORING_SCB_MODE_UNCONFIG_CONST_CFG)
    extern void port_MONITORING_SetPins(uint32 mode, uint32 subMode, uint32 uartEnableMask);
#endif /* (port_MONITORING_SCB_MODE_UNCONFIG_CONST_CFG) */


/***************************************
*     Vars with External Linkage
***************************************/

#if (port_MONITORING_SCB_IRQ_INTERNAL)
#if !defined (CY_REMOVE_port_MONITORING_CUSTOM_INTR_HANDLER)
    extern cyisraddress port_MONITORING_customIntrHandler;
#endif /* !defined (CY_REMOVE_port_MONITORING_CUSTOM_INTR_HANDLER) */
#endif /* (port_MONITORING_SCB_IRQ_INTERNAL) */

extern port_MONITORING_BACKUP_STRUCT port_MONITORING_backup;

#if(port_MONITORING_SCB_MODE_UNCONFIG_CONST_CFG)
    /* Common configuration variables */
    extern uint8 port_MONITORING_scbMode;
    extern uint8 port_MONITORING_scbEnableWake;
    extern uint8 port_MONITORING_scbEnableIntr;

    /* I2C configuration variables */
    extern uint8 port_MONITORING_mode;
    extern uint8 port_MONITORING_acceptAddr;

    /* SPI/UART configuration variables */
    extern volatile uint8 * port_MONITORING_rxBuffer;
    extern uint8   port_MONITORING_rxDataBits;
    extern uint32  port_MONITORING_rxBufferSize;

    extern volatile uint8 * port_MONITORING_txBuffer;
    extern uint8   port_MONITORING_txDataBits;
    extern uint32  port_MONITORING_txBufferSize;

    /* EZI2C configuration variables */
    extern uint8 port_MONITORING_numberOfAddr;
    extern uint8 port_MONITORING_subAddrSize;
#endif /* (port_MONITORING_SCB_MODE_UNCONFIG_CONST_CFG) */

#if (! (port_MONITORING_SCB_MODE_I2C_CONST_CFG || \
        port_MONITORING_SCB_MODE_EZI2C_CONST_CFG))
    extern uint16 port_MONITORING_IntrTxMask;
#endif /* (! (port_MONITORING_SCB_MODE_I2C_CONST_CFG || \
              port_MONITORING_SCB_MODE_EZI2C_CONST_CFG)) */


/***************************************
*        Conditional Macro
****************************************/

#if(port_MONITORING_SCB_MODE_UNCONFIG_CONST_CFG)
    /* Defines run time operation mode */
    #define port_MONITORING_SCB_MODE_I2C_RUNTM_CFG     (port_MONITORING_SCB_MODE_I2C      == port_MONITORING_scbMode)
    #define port_MONITORING_SCB_MODE_SPI_RUNTM_CFG     (port_MONITORING_SCB_MODE_SPI      == port_MONITORING_scbMode)
    #define port_MONITORING_SCB_MODE_UART_RUNTM_CFG    (port_MONITORING_SCB_MODE_UART     == port_MONITORING_scbMode)
    #define port_MONITORING_SCB_MODE_EZI2C_RUNTM_CFG   (port_MONITORING_SCB_MODE_EZI2C    == port_MONITORING_scbMode)
    #define port_MONITORING_SCB_MODE_UNCONFIG_RUNTM_CFG \
                                                        (port_MONITORING_SCB_MODE_UNCONFIG == port_MONITORING_scbMode)

    /* Defines wakeup enable */
    #define port_MONITORING_SCB_WAKE_ENABLE_CHECK       (0u != port_MONITORING_scbEnableWake)
#endif /* (port_MONITORING_SCB_MODE_UNCONFIG_CONST_CFG) */

/* Defines maximum number of SCB pins */
#if (!port_MONITORING_CY_SCBIP_V1)
    #define port_MONITORING_SCB_PINS_NUMBER    (7u)
#else
    #define port_MONITORING_SCB_PINS_NUMBER    (2u)
#endif /* (!port_MONITORING_CY_SCBIP_V1) */

#endif /* (CY_SCB_PVT_port_MONITORING_H) */


/* [] END OF FILE */
