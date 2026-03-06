/***************************************************************************//**
* \file OP_ST500_SPI_UART_PVT.h
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

#if !defined(CY_SCB_SPI_UART_PVT_OP_ST500_H)
#define CY_SCB_SPI_UART_PVT_OP_ST500_H

#include "OP_ST500_SPI_UART.h"


/***************************************
*     Internal Global Vars
***************************************/

#if (OP_ST500_INTERNAL_RX_SW_BUFFER_CONST)
    extern volatile uint32  OP_ST500_rxBufferHead;
    extern volatile uint32  OP_ST500_rxBufferTail;
    
    /**
    * \addtogroup group_globals
    * @{
    */
    
    /** Sets when internal software receive buffer overflow
     *  was occurred.
    */  
    extern volatile uint8   OP_ST500_rxBufferOverflow;
    /** @} globals */
#endif /* (OP_ST500_INTERNAL_RX_SW_BUFFER_CONST) */

#if (OP_ST500_INTERNAL_TX_SW_BUFFER_CONST)
    extern volatile uint32  OP_ST500_txBufferHead;
    extern volatile uint32  OP_ST500_txBufferTail;
#endif /* (OP_ST500_INTERNAL_TX_SW_BUFFER_CONST) */

#if (OP_ST500_INTERNAL_RX_SW_BUFFER)
    extern volatile uint8 OP_ST500_rxBufferInternal[OP_ST500_INTERNAL_RX_BUFFER_SIZE];
#endif /* (OP_ST500_INTERNAL_RX_SW_BUFFER) */

#if (OP_ST500_INTERNAL_TX_SW_BUFFER)
    extern volatile uint8 OP_ST500_txBufferInternal[OP_ST500_TX_BUFFER_SIZE];
#endif /* (OP_ST500_INTERNAL_TX_SW_BUFFER) */


/***************************************
*     Private Function Prototypes
***************************************/

void OP_ST500_SpiPostEnable(void);
void OP_ST500_SpiStop(void);

#if (OP_ST500_SCB_MODE_SPI_CONST_CFG)
    void OP_ST500_SpiInit(void);
#endif /* (OP_ST500_SCB_MODE_SPI_CONST_CFG) */

#if (OP_ST500_SPI_WAKE_ENABLE_CONST)
    void OP_ST500_SpiSaveConfig(void);
    void OP_ST500_SpiRestoreConfig(void);
#endif /* (OP_ST500_SPI_WAKE_ENABLE_CONST) */

void OP_ST500_UartPostEnable(void);
void OP_ST500_UartStop(void);

#if (OP_ST500_SCB_MODE_UART_CONST_CFG)
    void OP_ST500_UartInit(void);
#endif /* (OP_ST500_SCB_MODE_UART_CONST_CFG) */

#if (OP_ST500_UART_WAKE_ENABLE_CONST)
    void OP_ST500_UartSaveConfig(void);
    void OP_ST500_UartRestoreConfig(void);
#endif /* (OP_ST500_UART_WAKE_ENABLE_CONST) */


/***************************************
*         UART API Constants
***************************************/

/* UART RX and TX position to be used in OP_ST500_SetPins() */
#define OP_ST500_UART_RX_PIN_ENABLE    (OP_ST500_UART_RX)
#define OP_ST500_UART_TX_PIN_ENABLE    (OP_ST500_UART_TX)

/* UART RTS and CTS position to be used in  OP_ST500_SetPins() */
#define OP_ST500_UART_RTS_PIN_ENABLE    (0x10u)
#define OP_ST500_UART_CTS_PIN_ENABLE    (0x20u)


/***************************************
* The following code is DEPRECATED and
* must not be used.
***************************************/

/* Interrupt processing */
#define OP_ST500_SpiUartEnableIntRx(intSourceMask)  OP_ST500_SetRxInterruptMode(intSourceMask)
#define OP_ST500_SpiUartEnableIntTx(intSourceMask)  OP_ST500_SetTxInterruptMode(intSourceMask)
uint32  OP_ST500_SpiUartDisableIntRx(void);
uint32  OP_ST500_SpiUartDisableIntTx(void);


#endif /* (CY_SCB_SPI_UART_PVT_OP_ST500_H) */


/* [] END OF FILE */
