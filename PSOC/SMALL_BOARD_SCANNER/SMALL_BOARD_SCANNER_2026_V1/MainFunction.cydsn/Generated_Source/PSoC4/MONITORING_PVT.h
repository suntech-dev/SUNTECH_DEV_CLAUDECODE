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

#if !defined(CY_SCB_PVT_MONITORING_H)
#define CY_SCB_PVT_MONITORING_H

#include "MONITORING.h"


/***************************************
*     Private Function Prototypes
***************************************/

/* APIs to service INTR_I2C_EC register */
#define MONITORING_SetI2CExtClkInterruptMode(interruptMask) MONITORING_WRITE_INTR_I2C_EC_MASK(interruptMask)
#define MONITORING_ClearI2CExtClkInterruptSource(interruptMask) MONITORING_CLEAR_INTR_I2C_EC(interruptMask)
#define MONITORING_GetI2CExtClkInterruptSource()                (MONITORING_INTR_I2C_EC_REG)
#define MONITORING_GetI2CExtClkInterruptMode()                  (MONITORING_INTR_I2C_EC_MASK_REG)
#define MONITORING_GetI2CExtClkInterruptSourceMasked()          (MONITORING_INTR_I2C_EC_MASKED_REG)

#if (!MONITORING_CY_SCBIP_V1)
    /* APIs to service INTR_SPI_EC register */
    #define MONITORING_SetSpiExtClkInterruptMode(interruptMask) \
                                                                MONITORING_WRITE_INTR_SPI_EC_MASK(interruptMask)
    #define MONITORING_ClearSpiExtClkInterruptSource(interruptMask) \
                                                                MONITORING_CLEAR_INTR_SPI_EC(interruptMask)
    #define MONITORING_GetExtSpiClkInterruptSource()                 (MONITORING_INTR_SPI_EC_REG)
    #define MONITORING_GetExtSpiClkInterruptMode()                   (MONITORING_INTR_SPI_EC_MASK_REG)
    #define MONITORING_GetExtSpiClkInterruptSourceMasked()           (MONITORING_INTR_SPI_EC_MASKED_REG)
#endif /* (!MONITORING_CY_SCBIP_V1) */

#if(MONITORING_SCB_MODE_UNCONFIG_CONST_CFG)
    extern void MONITORING_SetPins(uint32 mode, uint32 subMode, uint32 uartEnableMask);
#endif /* (MONITORING_SCB_MODE_UNCONFIG_CONST_CFG) */


/***************************************
*     Vars with External Linkage
***************************************/

#if (MONITORING_SCB_IRQ_INTERNAL)
#if !defined (CY_REMOVE_MONITORING_CUSTOM_INTR_HANDLER)
    extern cyisraddress MONITORING_customIntrHandler;
#endif /* !defined (CY_REMOVE_MONITORING_CUSTOM_INTR_HANDLER) */
#endif /* (MONITORING_SCB_IRQ_INTERNAL) */

extern MONITORING_BACKUP_STRUCT MONITORING_backup;

#if(MONITORING_SCB_MODE_UNCONFIG_CONST_CFG)
    /* Common configuration variables */
    extern uint8 MONITORING_scbMode;
    extern uint8 MONITORING_scbEnableWake;
    extern uint8 MONITORING_scbEnableIntr;

    /* I2C configuration variables */
    extern uint8 MONITORING_mode;
    extern uint8 MONITORING_acceptAddr;

    /* SPI/UART configuration variables */
    extern volatile uint8 * MONITORING_rxBuffer;
    extern uint8   MONITORING_rxDataBits;
    extern uint32  MONITORING_rxBufferSize;

    extern volatile uint8 * MONITORING_txBuffer;
    extern uint8   MONITORING_txDataBits;
    extern uint32  MONITORING_txBufferSize;

    /* EZI2C configuration variables */
    extern uint8 MONITORING_numberOfAddr;
    extern uint8 MONITORING_subAddrSize;
#endif /* (MONITORING_SCB_MODE_UNCONFIG_CONST_CFG) */

#if (! (MONITORING_SCB_MODE_I2C_CONST_CFG || \
        MONITORING_SCB_MODE_EZI2C_CONST_CFG))
    extern uint16 MONITORING_IntrTxMask;
#endif /* (! (MONITORING_SCB_MODE_I2C_CONST_CFG || \
              MONITORING_SCB_MODE_EZI2C_CONST_CFG)) */


/***************************************
*        Conditional Macro
****************************************/

#if(MONITORING_SCB_MODE_UNCONFIG_CONST_CFG)
    /* Defines run time operation mode */
    #define MONITORING_SCB_MODE_I2C_RUNTM_CFG     (MONITORING_SCB_MODE_I2C      == MONITORING_scbMode)
    #define MONITORING_SCB_MODE_SPI_RUNTM_CFG     (MONITORING_SCB_MODE_SPI      == MONITORING_scbMode)
    #define MONITORING_SCB_MODE_UART_RUNTM_CFG    (MONITORING_SCB_MODE_UART     == MONITORING_scbMode)
    #define MONITORING_SCB_MODE_EZI2C_RUNTM_CFG   (MONITORING_SCB_MODE_EZI2C    == MONITORING_scbMode)
    #define MONITORING_SCB_MODE_UNCONFIG_RUNTM_CFG \
                                                        (MONITORING_SCB_MODE_UNCONFIG == MONITORING_scbMode)

    /* Defines wakeup enable */
    #define MONITORING_SCB_WAKE_ENABLE_CHECK       (0u != MONITORING_scbEnableWake)
#endif /* (MONITORING_SCB_MODE_UNCONFIG_CONST_CFG) */

/* Defines maximum number of SCB pins */
#if (!MONITORING_CY_SCBIP_V1)
    #define MONITORING_SCB_PINS_NUMBER    (7u)
#else
    #define MONITORING_SCB_PINS_NUMBER    (2u)
#endif /* (!MONITORING_CY_SCBIP_V1) */

#endif /* (CY_SCB_PVT_MONITORING_H) */


/* [] END OF FILE */
