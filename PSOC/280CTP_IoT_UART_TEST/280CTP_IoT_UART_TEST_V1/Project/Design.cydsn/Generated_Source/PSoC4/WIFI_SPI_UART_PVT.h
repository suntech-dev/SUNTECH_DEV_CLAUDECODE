/***************************************************************************//**
* \file WIFI_SPI_UART_PVT.h
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

#if !defined(CY_SCB_SPI_UART_PVT_WIFI_H)
#define CY_SCB_SPI_UART_PVT_WIFI_H

#include "WIFI_SPI_UART.h"


/***************************************
*     Internal Global Vars
***************************************/

#if (WIFI_INTERNAL_RX_SW_BUFFER_CONST)
    extern volatile uint32  WIFI_rxBufferHead;
    extern volatile uint32  WIFI_rxBufferTail;
    
    /**
    * \addtogroup group_globals
    * @{
    */
    
    /** Sets when internal software receive buffer overflow
     *  was occurred.
    */  
    extern volatile uint8   WIFI_rxBufferOverflow;
    /** @} globals */
#endif /* (WIFI_INTERNAL_RX_SW_BUFFER_CONST) */

#if (WIFI_INTERNAL_TX_SW_BUFFER_CONST)
    extern volatile uint32  WIFI_txBufferHead;
    extern volatile uint32  WIFI_txBufferTail;
#endif /* (WIFI_INTERNAL_TX_SW_BUFFER_CONST) */

#if (WIFI_INTERNAL_RX_SW_BUFFER)
    extern volatile uint8 WIFI_rxBufferInternal[WIFI_INTERNAL_RX_BUFFER_SIZE];
#endif /* (WIFI_INTERNAL_RX_SW_BUFFER) */

#if (WIFI_INTERNAL_TX_SW_BUFFER)
    extern volatile uint8 WIFI_txBufferInternal[WIFI_TX_BUFFER_SIZE];
#endif /* (WIFI_INTERNAL_TX_SW_BUFFER) */


/***************************************
*     Private Function Prototypes
***************************************/

void WIFI_SpiPostEnable(void);
void WIFI_SpiStop(void);

#if (WIFI_SCB_MODE_SPI_CONST_CFG)
    void WIFI_SpiInit(void);
#endif /* (WIFI_SCB_MODE_SPI_CONST_CFG) */

#if (WIFI_SPI_WAKE_ENABLE_CONST)
    void WIFI_SpiSaveConfig(void);
    void WIFI_SpiRestoreConfig(void);
#endif /* (WIFI_SPI_WAKE_ENABLE_CONST) */

void WIFI_UartPostEnable(void);
void WIFI_UartStop(void);

#if (WIFI_SCB_MODE_UART_CONST_CFG)
    void WIFI_UartInit(void);
#endif /* (WIFI_SCB_MODE_UART_CONST_CFG) */

#if (WIFI_UART_WAKE_ENABLE_CONST)
    void WIFI_UartSaveConfig(void);
    void WIFI_UartRestoreConfig(void);
#endif /* (WIFI_UART_WAKE_ENABLE_CONST) */


/***************************************
*         UART API Constants
***************************************/

/* UART RX and TX position to be used in WIFI_SetPins() */
#define WIFI_UART_RX_PIN_ENABLE    (WIFI_UART_RX)
#define WIFI_UART_TX_PIN_ENABLE    (WIFI_UART_TX)

/* UART RTS and CTS position to be used in  WIFI_SetPins() */
#define WIFI_UART_RTS_PIN_ENABLE    (0x10u)
#define WIFI_UART_CTS_PIN_ENABLE    (0x20u)


/***************************************
* The following code is DEPRECATED and
* must not be used.
***************************************/

/* Interrupt processing */
#define WIFI_SpiUartEnableIntRx(intSourceMask)  WIFI_SetRxInterruptMode(intSourceMask)
#define WIFI_SpiUartEnableIntTx(intSourceMask)  WIFI_SetTxInterruptMode(intSourceMask)
uint32  WIFI_SpiUartDisableIntRx(void);
uint32  WIFI_SpiUartDisableIntTx(void);


#endif /* (CY_SCB_SPI_UART_PVT_WIFI_H) */


/* [] END OF FILE */
