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

#if !defined(CY_SCB_PVT_MONITORING_TABLET_H)
#define CY_SCB_PVT_MONITORING_TABLET_H

#include "MONITORING_TABLET.h"


/***************************************
*     Private Function Prototypes
***************************************/

/* APIs to service INTR_I2C_EC register */
#define MONITORING_TABLET_SetI2CExtClkInterruptMode(interruptMask) MONITORING_TABLET_WRITE_INTR_I2C_EC_MASK(interruptMask)
#define MONITORING_TABLET_ClearI2CExtClkInterruptSource(interruptMask) MONITORING_TABLET_CLEAR_INTR_I2C_EC(interruptMask)
#define MONITORING_TABLET_GetI2CExtClkInterruptSource()                (MONITORING_TABLET_INTR_I2C_EC_REG)
#define MONITORING_TABLET_GetI2CExtClkInterruptMode()                  (MONITORING_TABLET_INTR_I2C_EC_MASK_REG)
#define MONITORING_TABLET_GetI2CExtClkInterruptSourceMasked()          (MONITORING_TABLET_INTR_I2C_EC_MASKED_REG)

#if (!MONITORING_TABLET_CY_SCBIP_V1)
    /* APIs to service INTR_SPI_EC register */
    #define MONITORING_TABLET_SetSpiExtClkInterruptMode(interruptMask) \
                                                                MONITORING_TABLET_WRITE_INTR_SPI_EC_MASK(interruptMask)
    #define MONITORING_TABLET_ClearSpiExtClkInterruptSource(interruptMask) \
                                                                MONITORING_TABLET_CLEAR_INTR_SPI_EC(interruptMask)
    #define MONITORING_TABLET_GetExtSpiClkInterruptSource()                 (MONITORING_TABLET_INTR_SPI_EC_REG)
    #define MONITORING_TABLET_GetExtSpiClkInterruptMode()                   (MONITORING_TABLET_INTR_SPI_EC_MASK_REG)
    #define MONITORING_TABLET_GetExtSpiClkInterruptSourceMasked()           (MONITORING_TABLET_INTR_SPI_EC_MASKED_REG)
#endif /* (!MONITORING_TABLET_CY_SCBIP_V1) */

#if(MONITORING_TABLET_SCB_MODE_UNCONFIG_CONST_CFG)
    extern void MONITORING_TABLET_SetPins(uint32 mode, uint32 subMode, uint32 uartEnableMask);
#endif /* (MONITORING_TABLET_SCB_MODE_UNCONFIG_CONST_CFG) */


/***************************************
*     Vars with External Linkage
***************************************/

#if (MONITORING_TABLET_SCB_IRQ_INTERNAL)
#if !defined (CY_REMOVE_MONITORING_TABLET_CUSTOM_INTR_HANDLER)
    extern cyisraddress MONITORING_TABLET_customIntrHandler;
#endif /* !defined (CY_REMOVE_MONITORING_TABLET_CUSTOM_INTR_HANDLER) */
#endif /* (MONITORING_TABLET_SCB_IRQ_INTERNAL) */

extern MONITORING_TABLET_BACKUP_STRUCT MONITORING_TABLET_backup;

#if(MONITORING_TABLET_SCB_MODE_UNCONFIG_CONST_CFG)
    /* Common configuration variables */
    extern uint8 MONITORING_TABLET_scbMode;
    extern uint8 MONITORING_TABLET_scbEnableWake;
    extern uint8 MONITORING_TABLET_scbEnableIntr;

    /* I2C configuration variables */
    extern uint8 MONITORING_TABLET_mode;
    extern uint8 MONITORING_TABLET_acceptAddr;

    /* SPI/UART configuration variables */
    extern volatile uint8 * MONITORING_TABLET_rxBuffer;
    extern uint8   MONITORING_TABLET_rxDataBits;
    extern uint32  MONITORING_TABLET_rxBufferSize;

    extern volatile uint8 * MONITORING_TABLET_txBuffer;
    extern uint8   MONITORING_TABLET_txDataBits;
    extern uint32  MONITORING_TABLET_txBufferSize;

    /* EZI2C configuration variables */
    extern uint8 MONITORING_TABLET_numberOfAddr;
    extern uint8 MONITORING_TABLET_subAddrSize;
#endif /* (MONITORING_TABLET_SCB_MODE_UNCONFIG_CONST_CFG) */

#if (! (MONITORING_TABLET_SCB_MODE_I2C_CONST_CFG || \
        MONITORING_TABLET_SCB_MODE_EZI2C_CONST_CFG))
    extern uint16 MONITORING_TABLET_IntrTxMask;
#endif /* (! (MONITORING_TABLET_SCB_MODE_I2C_CONST_CFG || \
              MONITORING_TABLET_SCB_MODE_EZI2C_CONST_CFG)) */


/***************************************
*        Conditional Macro
****************************************/

#if(MONITORING_TABLET_SCB_MODE_UNCONFIG_CONST_CFG)
    /* Defines run time operation mode */
    #define MONITORING_TABLET_SCB_MODE_I2C_RUNTM_CFG     (MONITORING_TABLET_SCB_MODE_I2C      == MONITORING_TABLET_scbMode)
    #define MONITORING_TABLET_SCB_MODE_SPI_RUNTM_CFG     (MONITORING_TABLET_SCB_MODE_SPI      == MONITORING_TABLET_scbMode)
    #define MONITORING_TABLET_SCB_MODE_UART_RUNTM_CFG    (MONITORING_TABLET_SCB_MODE_UART     == MONITORING_TABLET_scbMode)
    #define MONITORING_TABLET_SCB_MODE_EZI2C_RUNTM_CFG   (MONITORING_TABLET_SCB_MODE_EZI2C    == MONITORING_TABLET_scbMode)
    #define MONITORING_TABLET_SCB_MODE_UNCONFIG_RUNTM_CFG \
                                                        (MONITORING_TABLET_SCB_MODE_UNCONFIG == MONITORING_TABLET_scbMode)

    /* Defines wakeup enable */
    #define MONITORING_TABLET_SCB_WAKE_ENABLE_CHECK       (0u != MONITORING_TABLET_scbEnableWake)
#endif /* (MONITORING_TABLET_SCB_MODE_UNCONFIG_CONST_CFG) */

/* Defines maximum number of SCB pins */
#if (!MONITORING_TABLET_CY_SCBIP_V1)
    #define MONITORING_TABLET_SCB_PINS_NUMBER    (7u)
#else
    #define MONITORING_TABLET_SCB_PINS_NUMBER    (2u)
#endif /* (!MONITORING_TABLET_CY_SCBIP_V1) */

#endif /* (CY_SCB_PVT_MONITORING_TABLET_H) */


/* [] END OF FILE */
