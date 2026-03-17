/***************************************************************************//**
* \file QR_SCANNER_SPI_UART_PVT.h
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

#if !defined(CY_SCB_SPI_UART_PVT_QR_SCANNER_H)
#define CY_SCB_SPI_UART_PVT_QR_SCANNER_H

#include "QR_SCANNER_SPI_UART.h"


/***************************************
*     Internal Global Vars
***************************************/

#if (QR_SCANNER_INTERNAL_RX_SW_BUFFER_CONST)
    extern volatile uint32  QR_SCANNER_rxBufferHead;
    extern volatile uint32  QR_SCANNER_rxBufferTail;
    
    /**
    * \addtogroup group_globals
    * @{
    */
    
    /** Sets when internal software receive buffer overflow
     *  was occurred.
    */  
    extern volatile uint8   QR_SCANNER_rxBufferOverflow;
    /** @} globals */
#endif /* (QR_SCANNER_INTERNAL_RX_SW_BUFFER_CONST) */

#if (QR_SCANNER_INTERNAL_TX_SW_BUFFER_CONST)
    extern volatile uint32  QR_SCANNER_txBufferHead;
    extern volatile uint32  QR_SCANNER_txBufferTail;
#endif /* (QR_SCANNER_INTERNAL_TX_SW_BUFFER_CONST) */

#if (QR_SCANNER_INTERNAL_RX_SW_BUFFER)
    extern volatile uint8 QR_SCANNER_rxBufferInternal[QR_SCANNER_INTERNAL_RX_BUFFER_SIZE];
#endif /* (QR_SCANNER_INTERNAL_RX_SW_BUFFER) */

#if (QR_SCANNER_INTERNAL_TX_SW_BUFFER)
    extern volatile uint8 QR_SCANNER_txBufferInternal[QR_SCANNER_TX_BUFFER_SIZE];
#endif /* (QR_SCANNER_INTERNAL_TX_SW_BUFFER) */


/***************************************
*     Private Function Prototypes
***************************************/

void QR_SCANNER_SpiPostEnable(void);
void QR_SCANNER_SpiStop(void);

#if (QR_SCANNER_SCB_MODE_SPI_CONST_CFG)
    void QR_SCANNER_SpiInit(void);
#endif /* (QR_SCANNER_SCB_MODE_SPI_CONST_CFG) */

#if (QR_SCANNER_SPI_WAKE_ENABLE_CONST)
    void QR_SCANNER_SpiSaveConfig(void);
    void QR_SCANNER_SpiRestoreConfig(void);
#endif /* (QR_SCANNER_SPI_WAKE_ENABLE_CONST) */

void QR_SCANNER_UartPostEnable(void);
void QR_SCANNER_UartStop(void);

#if (QR_SCANNER_SCB_MODE_UART_CONST_CFG)
    void QR_SCANNER_UartInit(void);
#endif /* (QR_SCANNER_SCB_MODE_UART_CONST_CFG) */

#if (QR_SCANNER_UART_WAKE_ENABLE_CONST)
    void QR_SCANNER_UartSaveConfig(void);
    void QR_SCANNER_UartRestoreConfig(void);
#endif /* (QR_SCANNER_UART_WAKE_ENABLE_CONST) */


/***************************************
*         UART API Constants
***************************************/

/* UART RX and TX position to be used in QR_SCANNER_SetPins() */
#define QR_SCANNER_UART_RX_PIN_ENABLE    (QR_SCANNER_UART_RX)
#define QR_SCANNER_UART_TX_PIN_ENABLE    (QR_SCANNER_UART_TX)

/* UART RTS and CTS position to be used in  QR_SCANNER_SetPins() */
#define QR_SCANNER_UART_RTS_PIN_ENABLE    (0x10u)
#define QR_SCANNER_UART_CTS_PIN_ENABLE    (0x20u)


/***************************************
* The following code is DEPRECATED and
* must not be used.
***************************************/

/* Interrupt processing */
#define QR_SCANNER_SpiUartEnableIntRx(intSourceMask)  QR_SCANNER_SetRxInterruptMode(intSourceMask)
#define QR_SCANNER_SpiUartEnableIntTx(intSourceMask)  QR_SCANNER_SetTxInterruptMode(intSourceMask)
uint32  QR_SCANNER_SpiUartDisableIntRx(void);
uint32  QR_SCANNER_SpiUartDisableIntTx(void);


#endif /* (CY_SCB_SPI_UART_PVT_QR_SCANNER_H) */


/* [] END OF FILE */
