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

#if !defined(CY_SCB_PVT_BARCODE_H)
#define CY_SCB_PVT_BARCODE_H

#include "BARCODE.h"


/***************************************
*     Private Function Prototypes
***************************************/

/* APIs to service INTR_I2C_EC register */
#define BARCODE_SetI2CExtClkInterruptMode(interruptMask) BARCODE_WRITE_INTR_I2C_EC_MASK(interruptMask)
#define BARCODE_ClearI2CExtClkInterruptSource(interruptMask) BARCODE_CLEAR_INTR_I2C_EC(interruptMask)
#define BARCODE_GetI2CExtClkInterruptSource()                (BARCODE_INTR_I2C_EC_REG)
#define BARCODE_GetI2CExtClkInterruptMode()                  (BARCODE_INTR_I2C_EC_MASK_REG)
#define BARCODE_GetI2CExtClkInterruptSourceMasked()          (BARCODE_INTR_I2C_EC_MASKED_REG)

#if (!BARCODE_CY_SCBIP_V1)
    /* APIs to service INTR_SPI_EC register */
    #define BARCODE_SetSpiExtClkInterruptMode(interruptMask) \
                                                                BARCODE_WRITE_INTR_SPI_EC_MASK(interruptMask)
    #define BARCODE_ClearSpiExtClkInterruptSource(interruptMask) \
                                                                BARCODE_CLEAR_INTR_SPI_EC(interruptMask)
    #define BARCODE_GetExtSpiClkInterruptSource()                 (BARCODE_INTR_SPI_EC_REG)
    #define BARCODE_GetExtSpiClkInterruptMode()                   (BARCODE_INTR_SPI_EC_MASK_REG)
    #define BARCODE_GetExtSpiClkInterruptSourceMasked()           (BARCODE_INTR_SPI_EC_MASKED_REG)
#endif /* (!BARCODE_CY_SCBIP_V1) */

#if(BARCODE_SCB_MODE_UNCONFIG_CONST_CFG)
    extern void BARCODE_SetPins(uint32 mode, uint32 subMode, uint32 uartEnableMask);
#endif /* (BARCODE_SCB_MODE_UNCONFIG_CONST_CFG) */


/***************************************
*     Vars with External Linkage
***************************************/

#if (BARCODE_SCB_IRQ_INTERNAL)
#if !defined (CY_REMOVE_BARCODE_CUSTOM_INTR_HANDLER)
    extern cyisraddress BARCODE_customIntrHandler;
#endif /* !defined (CY_REMOVE_BARCODE_CUSTOM_INTR_HANDLER) */
#endif /* (BARCODE_SCB_IRQ_INTERNAL) */

extern BARCODE_BACKUP_STRUCT BARCODE_backup;

#if(BARCODE_SCB_MODE_UNCONFIG_CONST_CFG)
    /* Common configuration variables */
    extern uint8 BARCODE_scbMode;
    extern uint8 BARCODE_scbEnableWake;
    extern uint8 BARCODE_scbEnableIntr;

    /* I2C configuration variables */
    extern uint8 BARCODE_mode;
    extern uint8 BARCODE_acceptAddr;

    /* SPI/UART configuration variables */
    extern volatile uint8 * BARCODE_rxBuffer;
    extern uint8   BARCODE_rxDataBits;
    extern uint32  BARCODE_rxBufferSize;

    extern volatile uint8 * BARCODE_txBuffer;
    extern uint8   BARCODE_txDataBits;
    extern uint32  BARCODE_txBufferSize;

    /* EZI2C configuration variables */
    extern uint8 BARCODE_numberOfAddr;
    extern uint8 BARCODE_subAddrSize;
#endif /* (BARCODE_SCB_MODE_UNCONFIG_CONST_CFG) */

#if (! (BARCODE_SCB_MODE_I2C_CONST_CFG || \
        BARCODE_SCB_MODE_EZI2C_CONST_CFG))
    extern uint16 BARCODE_IntrTxMask;
#endif /* (! (BARCODE_SCB_MODE_I2C_CONST_CFG || \
              BARCODE_SCB_MODE_EZI2C_CONST_CFG)) */


/***************************************
*        Conditional Macro
****************************************/

#if(BARCODE_SCB_MODE_UNCONFIG_CONST_CFG)
    /* Defines run time operation mode */
    #define BARCODE_SCB_MODE_I2C_RUNTM_CFG     (BARCODE_SCB_MODE_I2C      == BARCODE_scbMode)
    #define BARCODE_SCB_MODE_SPI_RUNTM_CFG     (BARCODE_SCB_MODE_SPI      == BARCODE_scbMode)
    #define BARCODE_SCB_MODE_UART_RUNTM_CFG    (BARCODE_SCB_MODE_UART     == BARCODE_scbMode)
    #define BARCODE_SCB_MODE_EZI2C_RUNTM_CFG   (BARCODE_SCB_MODE_EZI2C    == BARCODE_scbMode)
    #define BARCODE_SCB_MODE_UNCONFIG_RUNTM_CFG \
                                                        (BARCODE_SCB_MODE_UNCONFIG == BARCODE_scbMode)

    /* Defines wakeup enable */
    #define BARCODE_SCB_WAKE_ENABLE_CHECK       (0u != BARCODE_scbEnableWake)
#endif /* (BARCODE_SCB_MODE_UNCONFIG_CONST_CFG) */

/* Defines maximum number of SCB pins */
#if (!BARCODE_CY_SCBIP_V1)
    #define BARCODE_SCB_PINS_NUMBER    (7u)
#else
    #define BARCODE_SCB_PINS_NUMBER    (2u)
#endif /* (!BARCODE_CY_SCBIP_V1) */

#endif /* (CY_SCB_PVT_BARCODE_H) */


/* [] END OF FILE */
