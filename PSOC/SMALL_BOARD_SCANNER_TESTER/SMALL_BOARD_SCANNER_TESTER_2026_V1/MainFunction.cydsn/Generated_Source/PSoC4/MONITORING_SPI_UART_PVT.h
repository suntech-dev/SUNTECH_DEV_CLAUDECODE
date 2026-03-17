/***************************************************************************//**
* \file MONITORING_SPI_UART_PVT.h
* \version 4.0
*
* \brief
*  This private file provides constants and parameter values for the
*  SCB Component in SPI and UART modes.
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

#if !defined(CY_SCB_SPI_UART_PVT_MONITORING_H)
#define CY_SCB_SPI_UART_PVT_MONITORING_H

#include "MONITORING_SPI_UART.h"


/***************************************
*     Internal Global Vars
***************************************/

#if (MONITORING_INTERNAL_RX_SW_BUFFER_CONST)
    extern volatile uint32  MONITORING_rxBufferHead;
    extern volatile uint32  MONITORING_rxBufferTail;
    
    /**
    * \addtogroup group_globals
    * @{
    */
    
    /** Sets when internal software receive buffer overflow
     *  was occurred.
    */  
    extern volatile uint8   MONITORING_rxBufferOverflow;
    /** @} globals */
#endif /* (MONITORING_INTERNAL_RX_SW_BUFFER_CONST) */

#if (MONITORING_INTERNAL_TX_SW_BUFFER_CONST)
    extern volatile uint32  MONITORING_txBufferHead;
    extern volatile uint32  MONITORING_txBufferTail;
#endif /* (MONITORING_INTERNAL_TX_SW_BUFFER_CONST) */

#if (MONITORING_INTERNAL_RX_SW_BUFFER)
    extern volatile uint8 MONITORING_rxBufferInternal[MONITORING_INTERNAL_RX_BUFFER_SIZE];
#endif /* (MONITORING_INTERNAL_RX_SW_BUFFER) */

#if (MONITORING_INTERNAL_TX_SW_BUFFER)
    extern volatile uint8 MONITORING_txBufferInternal[MONITORING_TX_BUFFER_SIZE];
#endif /* (MONITORING_INTERNAL_TX_SW_BUFFER) */


/***************************************
*     Private Function Prototypes
***************************************/

void MONITORING_SpiPostEnable(void);
void MONITORING_SpiStop(void);

#if (MONITORING_SCB_MODE_SPI_CONST_CFG)
    void MONITORING_SpiInit(void);
#endif /* (MONITORING_SCB_MODE_SPI_CONST_CFG) */

#if (MONITORING_SPI_WAKE_ENABLE_CONST)
    void MONITORING_SpiSaveConfig(void);
    void MONITORING_SpiRestoreConfig(void);
#endif /* (MONITORING_SPI_WAKE_ENABLE_CONST) */

void MONITORING_UartPostEnable(void);
void MONITORING_UartStop(void);

#if (MONITORING_SCB_MODE_UART_CONST_CFG)
    void MONITORING_UartInit(void);
#endif /* (MONITORING_SCB_MODE_UART_CONST_CFG) */

#if (MONITORING_UART_WAKE_ENABLE_CONST)
    void MONITORING_UartSaveConfig(void);
    void MONITORING_UartRestoreConfig(void);
#endif /* (MONITORING_UART_WAKE_ENABLE_CONST) */


/***************************************
*         UART API Constants
***************************************/

/* UART RX and TX position to be used in MONITORING_SetPins() */
#define MONITORING_UART_RX_PIN_ENABLE    (MONITORING_UART_RX)
#define MONITORING_UART_TX_PIN_ENABLE    (MONITORING_UART_TX)

/* UART RTS and CTS position to be used in  MONITORING_SetPins() */
#define MONITORING_UART_RTS_PIN_ENABLE    (0x10u)
#define MONITORING_UART_CTS_PIN_ENABLE    (0x20u)


/***************************************
* The following code is DEPRECATED and
* must not be used.
***************************************/

/* Interrupt processing */
#define MONITORING_SpiUartEnableIntRx(intSourceMask)  MONITORING_SetRxInterruptMode(intSourceMask)
#define MONITORING_SpiUartEnableIntTx(intSourceMask)  MONITORING_SetTxInterruptMode(intSourceMask)
uint32  MONITORING_SpiUartDisableIntRx(void);
uint32  MONITORING_SpiUartDisableIntTx(void);


#endif /* (CY_SCB_SPI_UART_PVT_MONITORING_H) */


/* [] END OF FILE */
