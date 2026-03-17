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

#if !defined(CY_SCB_PVT_UART_WIFI_H)
#define CY_SCB_PVT_UART_WIFI_H

#include "UART_WIFI.h"


/***************************************
*     Private Function Prototypes
***************************************/

/* APIs to service INTR_I2C_EC register */
#define UART_WIFI_SetI2CExtClkInterruptMode(interruptMask) UART_WIFI_WRITE_INTR_I2C_EC_MASK(interruptMask)
#define UART_WIFI_ClearI2CExtClkInterruptSource(interruptMask) UART_WIFI_CLEAR_INTR_I2C_EC(interruptMask)
#define UART_WIFI_GetI2CExtClkInterruptSource()                (UART_WIFI_INTR_I2C_EC_REG)
#define UART_WIFI_GetI2CExtClkInterruptMode()                  (UART_WIFI_INTR_I2C_EC_MASK_REG)
#define UART_WIFI_GetI2CExtClkInterruptSourceMasked()          (UART_WIFI_INTR_I2C_EC_MASKED_REG)

#if (!UART_WIFI_CY_SCBIP_V1)
    /* APIs to service INTR_SPI_EC register */
    #define UART_WIFI_SetSpiExtClkInterruptMode(interruptMask) \
                                                                UART_WIFI_WRITE_INTR_SPI_EC_MASK(interruptMask)
    #define UART_WIFI_ClearSpiExtClkInterruptSource(interruptMask) \
                                                                UART_WIFI_CLEAR_INTR_SPI_EC(interruptMask)
    #define UART_WIFI_GetExtSpiClkInterruptSource()                 (UART_WIFI_INTR_SPI_EC_REG)
    #define UART_WIFI_GetExtSpiClkInterruptMode()                   (UART_WIFI_INTR_SPI_EC_MASK_REG)
    #define UART_WIFI_GetExtSpiClkInterruptSourceMasked()           (UART_WIFI_INTR_SPI_EC_MASKED_REG)
#endif /* (!UART_WIFI_CY_SCBIP_V1) */

#if(UART_WIFI_SCB_MODE_UNCONFIG_CONST_CFG)
    extern void UART_WIFI_SetPins(uint32 mode, uint32 subMode, uint32 uartEnableMask);
#endif /* (UART_WIFI_SCB_MODE_UNCONFIG_CONST_CFG) */


/***************************************
*     Vars with External Linkage
***************************************/

#if (UART_WIFI_SCB_IRQ_INTERNAL)
#if !defined (CY_REMOVE_UART_WIFI_CUSTOM_INTR_HANDLER)
    extern cyisraddress UART_WIFI_customIntrHandler;
#endif /* !defined (CY_REMOVE_UART_WIFI_CUSTOM_INTR_HANDLER) */
#endif /* (UART_WIFI_SCB_IRQ_INTERNAL) */

extern UART_WIFI_BACKUP_STRUCT UART_WIFI_backup;

#if(UART_WIFI_SCB_MODE_UNCONFIG_CONST_CFG)
    /* Common configuration variables */
    extern uint8 UART_WIFI_scbMode;
    extern uint8 UART_WIFI_scbEnableWake;
    extern uint8 UART_WIFI_scbEnableIntr;

    /* I2C configuration variables */
    extern uint8 UART_WIFI_mode;
    extern uint8 UART_WIFI_acceptAddr;

    /* SPI/UART configuration variables */
    extern volatile uint8 * UART_WIFI_rxBuffer;
    extern uint8   UART_WIFI_rxDataBits;
    extern uint32  UART_WIFI_rxBufferSize;

    extern volatile uint8 * UART_WIFI_txBuffer;
    extern uint8   UART_WIFI_txDataBits;
    extern uint32  UART_WIFI_txBufferSize;

    /* EZI2C configuration variables */
    extern uint8 UART_WIFI_numberOfAddr;
    extern uint8 UART_WIFI_subAddrSize;
#endif /* (UART_WIFI_SCB_MODE_UNCONFIG_CONST_CFG) */

#if (! (UART_WIFI_SCB_MODE_I2C_CONST_CFG || \
        UART_WIFI_SCB_MODE_EZI2C_CONST_CFG))
    extern uint16 UART_WIFI_IntrTxMask;
#endif /* (! (UART_WIFI_SCB_MODE_I2C_CONST_CFG || \
              UART_WIFI_SCB_MODE_EZI2C_CONST_CFG)) */


/***************************************
*        Conditional Macro
****************************************/

#if(UART_WIFI_SCB_MODE_UNCONFIG_CONST_CFG)
    /* Defines run time operation mode */
    #define UART_WIFI_SCB_MODE_I2C_RUNTM_CFG     (UART_WIFI_SCB_MODE_I2C      == UART_WIFI_scbMode)
    #define UART_WIFI_SCB_MODE_SPI_RUNTM_CFG     (UART_WIFI_SCB_MODE_SPI      == UART_WIFI_scbMode)
    #define UART_WIFI_SCB_MODE_UART_RUNTM_CFG    (UART_WIFI_SCB_MODE_UART     == UART_WIFI_scbMode)
    #define UART_WIFI_SCB_MODE_EZI2C_RUNTM_CFG   (UART_WIFI_SCB_MODE_EZI2C    == UART_WIFI_scbMode)
    #define UART_WIFI_SCB_MODE_UNCONFIG_RUNTM_CFG \
                                                        (UART_WIFI_SCB_MODE_UNCONFIG == UART_WIFI_scbMode)

    /* Defines wakeup enable */
    #define UART_WIFI_SCB_WAKE_ENABLE_CHECK       (0u != UART_WIFI_scbEnableWake)
#endif /* (UART_WIFI_SCB_MODE_UNCONFIG_CONST_CFG) */

/* Defines maximum number of SCB pins */
#if (!UART_WIFI_CY_SCBIP_V1)
    #define UART_WIFI_SCB_PINS_NUMBER    (7u)
#else
    #define UART_WIFI_SCB_PINS_NUMBER    (2u)
#endif /* (!UART_WIFI_CY_SCBIP_V1) */

#endif /* (CY_SCB_PVT_UART_WIFI_H) */


/* [] END OF FILE */
