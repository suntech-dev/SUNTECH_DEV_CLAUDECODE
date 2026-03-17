/***************************************************************************//**
* \file BARCODE_SPI_UART_PVT.h
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

#if !defined(CY_SCB_SPI_UART_PVT_BARCODE_H)
#define CY_SCB_SPI_UART_PVT_BARCODE_H

#include "BARCODE_SPI_UART.h"


/***************************************
*     Internal Global Vars
***************************************/

#if (BARCODE_INTERNAL_RX_SW_BUFFER_CONST)
    extern volatile uint32  BARCODE_rxBufferHead;
    extern volatile uint32  BARCODE_rxBufferTail;
    
    /**
    * \addtogroup group_globals
    * @{
    */
    
    /** Sets when internal software receive buffer overflow
     *  was occurred.
    */  
    extern volatile uint8   BARCODE_rxBufferOverflow;
    /** @} globals */
#endif /* (BARCODE_INTERNAL_RX_SW_BUFFER_CONST) */

#if (BARCODE_INTERNAL_TX_SW_BUFFER_CONST)
    extern volatile uint32  BARCODE_txBufferHead;
    extern volatile uint32  BARCODE_txBufferTail;
#endif /* (BARCODE_INTERNAL_TX_SW_BUFFER_CONST) */

#if (BARCODE_INTERNAL_RX_SW_BUFFER)
    extern volatile uint8 BARCODE_rxBufferInternal[BARCODE_INTERNAL_RX_BUFFER_SIZE];
#endif /* (BARCODE_INTERNAL_RX_SW_BUFFER) */

#if (BARCODE_INTERNAL_TX_SW_BUFFER)
    extern volatile uint8 BARCODE_txBufferInternal[BARCODE_TX_BUFFER_SIZE];
#endif /* (BARCODE_INTERNAL_TX_SW_BUFFER) */


/***************************************
*     Private Function Prototypes
***************************************/

void BARCODE_SpiPostEnable(void);
void BARCODE_SpiStop(void);

#if (BARCODE_SCB_MODE_SPI_CONST_CFG)
    void BARCODE_SpiInit(void);
#endif /* (BARCODE_SCB_MODE_SPI_CONST_CFG) */

#if (BARCODE_SPI_WAKE_ENABLE_CONST)
    void BARCODE_SpiSaveConfig(void);
    void BARCODE_SpiRestoreConfig(void);
#endif /* (BARCODE_SPI_WAKE_ENABLE_CONST) */

void BARCODE_UartPostEnable(void);
void BARCODE_UartStop(void);

#if (BARCODE_SCB_MODE_UART_CONST_CFG)
    void BARCODE_UartInit(void);
#endif /* (BARCODE_SCB_MODE_UART_CONST_CFG) */

#if (BARCODE_UART_WAKE_ENABLE_CONST)
    void BARCODE_UartSaveConfig(void);
    void BARCODE_UartRestoreConfig(void);
#endif /* (BARCODE_UART_WAKE_ENABLE_CONST) */


/***************************************
*         UART API Constants
***************************************/

/* UART RX and TX position to be used in BARCODE_SetPins() */
#define BARCODE_UART_RX_PIN_ENABLE    (BARCODE_UART_RX)
#define BARCODE_UART_TX_PIN_ENABLE    (BARCODE_UART_TX)

/* UART RTS and CTS position to be used in  BARCODE_SetPins() */
#define BARCODE_UART_RTS_PIN_ENABLE    (0x10u)
#define BARCODE_UART_CTS_PIN_ENABLE    (0x20u)


/***************************************
* The following code is DEPRECATED and
* must not be used.
***************************************/

/* Interrupt processing */
#define BARCODE_SpiUartEnableIntRx(intSourceMask)  BARCODE_SetRxInterruptMode(intSourceMask)
#define BARCODE_SpiUartEnableIntTx(intSourceMask)  BARCODE_SetTxInterruptMode(intSourceMask)
uint32  BARCODE_SpiUartDisableIntRx(void);
uint32  BARCODE_SpiUartDisableIntTx(void);


#endif /* (CY_SCB_SPI_UART_PVT_BARCODE_H) */


/* [] END OF FILE */
