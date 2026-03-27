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

#if !defined(CY_SCB_PVT_WIFI_H)
#define CY_SCB_PVT_WIFI_H

#include "WIFI.h"


/***************************************
*     Private Function Prototypes
***************************************/

/* APIs to service INTR_I2C_EC register */
#define WIFI_SetI2CExtClkInterruptMode(interruptMask) WIFI_WRITE_INTR_I2C_EC_MASK(interruptMask)
#define WIFI_ClearI2CExtClkInterruptSource(interruptMask) WIFI_CLEAR_INTR_I2C_EC(interruptMask)
#define WIFI_GetI2CExtClkInterruptSource()                (WIFI_INTR_I2C_EC_REG)
#define WIFI_GetI2CExtClkInterruptMode()                  (WIFI_INTR_I2C_EC_MASK_REG)
#define WIFI_GetI2CExtClkInterruptSourceMasked()          (WIFI_INTR_I2C_EC_MASKED_REG)

#if (!WIFI_CY_SCBIP_V1)
    /* APIs to service INTR_SPI_EC register */
    #define WIFI_SetSpiExtClkInterruptMode(interruptMask) \
                                                                WIFI_WRITE_INTR_SPI_EC_MASK(interruptMask)
    #define WIFI_ClearSpiExtClkInterruptSource(interruptMask) \
                                                                WIFI_CLEAR_INTR_SPI_EC(interruptMask)
    #define WIFI_GetExtSpiClkInterruptSource()                 (WIFI_INTR_SPI_EC_REG)
    #define WIFI_GetExtSpiClkInterruptMode()                   (WIFI_INTR_SPI_EC_MASK_REG)
    #define WIFI_GetExtSpiClkInterruptSourceMasked()           (WIFI_INTR_SPI_EC_MASKED_REG)
#endif /* (!WIFI_CY_SCBIP_V1) */

#if(WIFI_SCB_MODE_UNCONFIG_CONST_CFG)
    extern void WIFI_SetPins(uint32 mode, uint32 subMode, uint32 uartEnableMask);
#endif /* (WIFI_SCB_MODE_UNCONFIG_CONST_CFG) */


/***************************************
*     Vars with External Linkage
***************************************/

#if (WIFI_SCB_IRQ_INTERNAL)
#if !defined (CY_REMOVE_WIFI_CUSTOM_INTR_HANDLER)
    extern cyisraddress WIFI_customIntrHandler;
#endif /* !defined (CY_REMOVE_WIFI_CUSTOM_INTR_HANDLER) */
#endif /* (WIFI_SCB_IRQ_INTERNAL) */

extern WIFI_BACKUP_STRUCT WIFI_backup;

#if(WIFI_SCB_MODE_UNCONFIG_CONST_CFG)
    /* Common configuration variables */
    extern uint8 WIFI_scbMode;
    extern uint8 WIFI_scbEnableWake;
    extern uint8 WIFI_scbEnableIntr;

    /* I2C configuration variables */
    extern uint8 WIFI_mode;
    extern uint8 WIFI_acceptAddr;

    /* SPI/UART configuration variables */
    extern volatile uint8 * WIFI_rxBuffer;
    extern uint8   WIFI_rxDataBits;
    extern uint32  WIFI_rxBufferSize;

    extern volatile uint8 * WIFI_txBuffer;
    extern uint8   WIFI_txDataBits;
    extern uint32  WIFI_txBufferSize;

    /* EZI2C configuration variables */
    extern uint8 WIFI_numberOfAddr;
    extern uint8 WIFI_subAddrSize;
#endif /* (WIFI_SCB_MODE_UNCONFIG_CONST_CFG) */

#if (! (WIFI_SCB_MODE_I2C_CONST_CFG || \
        WIFI_SCB_MODE_EZI2C_CONST_CFG))
    extern uint16 WIFI_IntrTxMask;
#endif /* (! (WIFI_SCB_MODE_I2C_CONST_CFG || \
              WIFI_SCB_MODE_EZI2C_CONST_CFG)) */


/***************************************
*        Conditional Macro
****************************************/

#if(WIFI_SCB_MODE_UNCONFIG_CONST_CFG)
    /* Defines run time operation mode */
    #define WIFI_SCB_MODE_I2C_RUNTM_CFG     (WIFI_SCB_MODE_I2C      == WIFI_scbMode)
    #define WIFI_SCB_MODE_SPI_RUNTM_CFG     (WIFI_SCB_MODE_SPI      == WIFI_scbMode)
    #define WIFI_SCB_MODE_UART_RUNTM_CFG    (WIFI_SCB_MODE_UART     == WIFI_scbMode)
    #define WIFI_SCB_MODE_EZI2C_RUNTM_CFG   (WIFI_SCB_MODE_EZI2C    == WIFI_scbMode)
    #define WIFI_SCB_MODE_UNCONFIG_RUNTM_CFG \
                                                        (WIFI_SCB_MODE_UNCONFIG == WIFI_scbMode)

    /* Defines wakeup enable */
    #define WIFI_SCB_WAKE_ENABLE_CHECK       (0u != WIFI_scbEnableWake)
#endif /* (WIFI_SCB_MODE_UNCONFIG_CONST_CFG) */

/* Defines maximum number of SCB pins */
#if (!WIFI_CY_SCBIP_V1)
    #define WIFI_SCB_PINS_NUMBER    (7u)
#else
    #define WIFI_SCB_PINS_NUMBER    (2u)
#endif /* (!WIFI_CY_SCBIP_V1) */

#endif /* (CY_SCB_PVT_WIFI_H) */


/* [] END OF FILE */
