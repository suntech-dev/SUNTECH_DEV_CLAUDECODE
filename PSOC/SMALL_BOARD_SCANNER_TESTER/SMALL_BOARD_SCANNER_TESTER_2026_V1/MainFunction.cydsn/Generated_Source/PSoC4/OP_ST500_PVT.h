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

#if !defined(CY_SCB_PVT_OP_ST500_H)
#define CY_SCB_PVT_OP_ST500_H

#include "OP_ST500.h"


/***************************************
*     Private Function Prototypes
***************************************/

/* APIs to service INTR_I2C_EC register */
#define OP_ST500_SetI2CExtClkInterruptMode(interruptMask) OP_ST500_WRITE_INTR_I2C_EC_MASK(interruptMask)
#define OP_ST500_ClearI2CExtClkInterruptSource(interruptMask) OP_ST500_CLEAR_INTR_I2C_EC(interruptMask)
#define OP_ST500_GetI2CExtClkInterruptSource()                (OP_ST500_INTR_I2C_EC_REG)
#define OP_ST500_GetI2CExtClkInterruptMode()                  (OP_ST500_INTR_I2C_EC_MASK_REG)
#define OP_ST500_GetI2CExtClkInterruptSourceMasked()          (OP_ST500_INTR_I2C_EC_MASKED_REG)

#if (!OP_ST500_CY_SCBIP_V1)
    /* APIs to service INTR_SPI_EC register */
    #define OP_ST500_SetSpiExtClkInterruptMode(interruptMask) \
                                                                OP_ST500_WRITE_INTR_SPI_EC_MASK(interruptMask)
    #define OP_ST500_ClearSpiExtClkInterruptSource(interruptMask) \
                                                                OP_ST500_CLEAR_INTR_SPI_EC(interruptMask)
    #define OP_ST500_GetExtSpiClkInterruptSource()                 (OP_ST500_INTR_SPI_EC_REG)
    #define OP_ST500_GetExtSpiClkInterruptMode()                   (OP_ST500_INTR_SPI_EC_MASK_REG)
    #define OP_ST500_GetExtSpiClkInterruptSourceMasked()           (OP_ST500_INTR_SPI_EC_MASKED_REG)
#endif /* (!OP_ST500_CY_SCBIP_V1) */

#if(OP_ST500_SCB_MODE_UNCONFIG_CONST_CFG)
    extern void OP_ST500_SetPins(uint32 mode, uint32 subMode, uint32 uartEnableMask);
#endif /* (OP_ST500_SCB_MODE_UNCONFIG_CONST_CFG) */


/***************************************
*     Vars with External Linkage
***************************************/

#if (OP_ST500_SCB_IRQ_INTERNAL)
#if !defined (CY_REMOVE_OP_ST500_CUSTOM_INTR_HANDLER)
    extern cyisraddress OP_ST500_customIntrHandler;
#endif /* !defined (CY_REMOVE_OP_ST500_CUSTOM_INTR_HANDLER) */
#endif /* (OP_ST500_SCB_IRQ_INTERNAL) */

extern OP_ST500_BACKUP_STRUCT OP_ST500_backup;

#if(OP_ST500_SCB_MODE_UNCONFIG_CONST_CFG)
    /* Common configuration variables */
    extern uint8 OP_ST500_scbMode;
    extern uint8 OP_ST500_scbEnableWake;
    extern uint8 OP_ST500_scbEnableIntr;

    /* I2C configuration variables */
    extern uint8 OP_ST500_mode;
    extern uint8 OP_ST500_acceptAddr;

    /* SPI/UART configuration variables */
    extern volatile uint8 * OP_ST500_rxBuffer;
    extern uint8   OP_ST500_rxDataBits;
    extern uint32  OP_ST500_rxBufferSize;

    extern volatile uint8 * OP_ST500_txBuffer;
    extern uint8   OP_ST500_txDataBits;
    extern uint32  OP_ST500_txBufferSize;

    /* EZI2C configuration variables */
    extern uint8 OP_ST500_numberOfAddr;
    extern uint8 OP_ST500_subAddrSize;
#endif /* (OP_ST500_SCB_MODE_UNCONFIG_CONST_CFG) */

#if (! (OP_ST500_SCB_MODE_I2C_CONST_CFG || \
        OP_ST500_SCB_MODE_EZI2C_CONST_CFG))
    extern uint16 OP_ST500_IntrTxMask;
#endif /* (! (OP_ST500_SCB_MODE_I2C_CONST_CFG || \
              OP_ST500_SCB_MODE_EZI2C_CONST_CFG)) */


/***************************************
*        Conditional Macro
****************************************/

#if(OP_ST500_SCB_MODE_UNCONFIG_CONST_CFG)
    /* Defines run time operation mode */
    #define OP_ST500_SCB_MODE_I2C_RUNTM_CFG     (OP_ST500_SCB_MODE_I2C      == OP_ST500_scbMode)
    #define OP_ST500_SCB_MODE_SPI_RUNTM_CFG     (OP_ST500_SCB_MODE_SPI      == OP_ST500_scbMode)
    #define OP_ST500_SCB_MODE_UART_RUNTM_CFG    (OP_ST500_SCB_MODE_UART     == OP_ST500_scbMode)
    #define OP_ST500_SCB_MODE_EZI2C_RUNTM_CFG   (OP_ST500_SCB_MODE_EZI2C    == OP_ST500_scbMode)
    #define OP_ST500_SCB_MODE_UNCONFIG_RUNTM_CFG \
                                                        (OP_ST500_SCB_MODE_UNCONFIG == OP_ST500_scbMode)

    /* Defines wakeup enable */
    #define OP_ST500_SCB_WAKE_ENABLE_CHECK       (0u != OP_ST500_scbEnableWake)
#endif /* (OP_ST500_SCB_MODE_UNCONFIG_CONST_CFG) */

/* Defines maximum number of SCB pins */
#if (!OP_ST500_CY_SCBIP_V1)
    #define OP_ST500_SCB_PINS_NUMBER    (7u)
#else
    #define OP_ST500_SCB_PINS_NUMBER    (2u)
#endif /* (!OP_ST500_CY_SCBIP_V1) */

#endif /* (CY_SCB_PVT_OP_ST500_H) */


/* [] END OF FILE */
