/***************************************************************************//**
* \file port_OP_SPI_UART_PVT.h
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

#if !defined(CY_SCB_SPI_UART_PVT_port_OP_H)
#define CY_SCB_SPI_UART_PVT_port_OP_H

#include "port_OP_SPI_UART.h"


/***************************************
*     Internal Global Vars
***************************************/

#if (port_OP_INTERNAL_RX_SW_BUFFER_CONST)
    extern volatile uint32  port_OP_rxBufferHead;
    extern volatile uint32  port_OP_rxBufferTail;
    
    /**
    * \addtogroup group_globals
    * @{
    */
    
    /** Sets when internal software receive buffer overflow
     *  was occurred.
    */  
    extern volatile uint8   port_OP_rxBufferOverflow;
    /** @} globals */
#endif /* (port_OP_INTERNAL_RX_SW_BUFFER_CONST) */

#if (port_OP_INTERNAL_TX_SW_BUFFER_CONST)
    extern volatile uint32  port_OP_txBufferHead;
    extern volatile uint32  port_OP_txBufferTail;
#endif /* (port_OP_INTERNAL_TX_SW_BUFFER_CONST) */

#if (port_OP_INTERNAL_RX_SW_BUFFER)
    extern volatile uint8 port_OP_rxBufferInternal[port_OP_INTERNAL_RX_BUFFER_SIZE];
#endif /* (port_OP_INTERNAL_RX_SW_BUFFER) */

#if (port_OP_INTERNAL_TX_SW_BUFFER)
    extern volatile uint8 port_OP_txBufferInternal[port_OP_TX_BUFFER_SIZE];
#endif /* (port_OP_INTERNAL_TX_SW_BUFFER) */


/***************************************
*     Private Function Prototypes
***************************************/

void port_OP_SpiPostEnable(void);
void port_OP_SpiStop(void);

#if (port_OP_SCB_MODE_SPI_CONST_CFG)
    void port_OP_SpiInit(void);
#endif /* (port_OP_SCB_MODE_SPI_CONST_CFG) */

#if (port_OP_SPI_WAKE_ENABLE_CONST)
    void port_OP_SpiSaveConfig(void);
    void port_OP_SpiRestoreConfig(void);
#endif /* (port_OP_SPI_WAKE_ENABLE_CONST) */

void port_OP_UartPostEnable(void);
void port_OP_UartStop(void);

#if (port_OP_SCB_MODE_UART_CONST_CFG)
    void port_OP_UartInit(void);
#endif /* (port_OP_SCB_MODE_UART_CONST_CFG) */

#if (port_OP_UART_WAKE_ENABLE_CONST)
    void port_OP_UartSaveConfig(void);
    void port_OP_UartRestoreConfig(void);
#endif /* (port_OP_UART_WAKE_ENABLE_CONST) */


/***************************************
*         UART API Constants
***************************************/

/* UART RX and TX position to be used in port_OP_SetPins() */
#define port_OP_UART_RX_PIN_ENABLE    (port_OP_UART_RX)
#define port_OP_UART_TX_PIN_ENABLE    (port_OP_UART_TX)

/* UART RTS and CTS position to be used in  port_OP_SetPins() */
#define port_OP_UART_RTS_PIN_ENABLE    (0x10u)
#define port_OP_UART_CTS_PIN_ENABLE    (0x20u)


/***************************************
* The following code is DEPRECATED and
* must not be used.
***************************************/

/* Interrupt processing */
#define port_OP_SpiUartEnableIntRx(intSourceMask)  port_OP_SetRxInterruptMode(intSourceMask)
#define port_OP_SpiUartEnableIntTx(intSourceMask)  port_OP_SetTxInterruptMode(intSourceMask)
uint32  port_OP_SpiUartDisableIntRx(void);
uint32  port_OP_SpiUartDisableIntTx(void);


#endif /* (CY_SCB_SPI_UART_PVT_port_OP_H) */


/* [] END OF FILE */
