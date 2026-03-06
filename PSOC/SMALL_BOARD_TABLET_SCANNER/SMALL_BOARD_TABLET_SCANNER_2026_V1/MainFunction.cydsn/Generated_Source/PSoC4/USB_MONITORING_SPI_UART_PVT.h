/***************************************************************************//**
* \file USB_MONITORING_SPI_UART_PVT.h
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

#if !defined(CY_SCB_SPI_UART_PVT_USB_MONITORING_H)
#define CY_SCB_SPI_UART_PVT_USB_MONITORING_H

#include "USB_MONITORING_SPI_UART.h"


/***************************************
*     Internal Global Vars
***************************************/

#if (USB_MONITORING_INTERNAL_RX_SW_BUFFER_CONST)
    extern volatile uint32  USB_MONITORING_rxBufferHead;
    extern volatile uint32  USB_MONITORING_rxBufferTail;
    
    /**
    * \addtogroup group_globals
    * @{
    */
    
    /** Sets when internal software receive buffer overflow
     *  was occurred.
    */  
    extern volatile uint8   USB_MONITORING_rxBufferOverflow;
    /** @} globals */
#endif /* (USB_MONITORING_INTERNAL_RX_SW_BUFFER_CONST) */

#if (USB_MONITORING_INTERNAL_TX_SW_BUFFER_CONST)
    extern volatile uint32  USB_MONITORING_txBufferHead;
    extern volatile uint32  USB_MONITORING_txBufferTail;
#endif /* (USB_MONITORING_INTERNAL_TX_SW_BUFFER_CONST) */

#if (USB_MONITORING_INTERNAL_RX_SW_BUFFER)
    extern volatile uint8 USB_MONITORING_rxBufferInternal[USB_MONITORING_INTERNAL_RX_BUFFER_SIZE];
#endif /* (USB_MONITORING_INTERNAL_RX_SW_BUFFER) */

#if (USB_MONITORING_INTERNAL_TX_SW_BUFFER)
    extern volatile uint8 USB_MONITORING_txBufferInternal[USB_MONITORING_TX_BUFFER_SIZE];
#endif /* (USB_MONITORING_INTERNAL_TX_SW_BUFFER) */


/***************************************
*     Private Function Prototypes
***************************************/

void USB_MONITORING_SpiPostEnable(void);
void USB_MONITORING_SpiStop(void);

#if (USB_MONITORING_SCB_MODE_SPI_CONST_CFG)
    void USB_MONITORING_SpiInit(void);
#endif /* (USB_MONITORING_SCB_MODE_SPI_CONST_CFG) */

#if (USB_MONITORING_SPI_WAKE_ENABLE_CONST)
    void USB_MONITORING_SpiSaveConfig(void);
    void USB_MONITORING_SpiRestoreConfig(void);
#endif /* (USB_MONITORING_SPI_WAKE_ENABLE_CONST) */

void USB_MONITORING_UartPostEnable(void);
void USB_MONITORING_UartStop(void);

#if (USB_MONITORING_SCB_MODE_UART_CONST_CFG)
    void USB_MONITORING_UartInit(void);
#endif /* (USB_MONITORING_SCB_MODE_UART_CONST_CFG) */

#if (USB_MONITORING_UART_WAKE_ENABLE_CONST)
    void USB_MONITORING_UartSaveConfig(void);
    void USB_MONITORING_UartRestoreConfig(void);
#endif /* (USB_MONITORING_UART_WAKE_ENABLE_CONST) */


/***************************************
*         UART API Constants
***************************************/

/* UART RX and TX position to be used in USB_MONITORING_SetPins() */
#define USB_MONITORING_UART_RX_PIN_ENABLE    (USB_MONITORING_UART_RX)
#define USB_MONITORING_UART_TX_PIN_ENABLE    (USB_MONITORING_UART_TX)

/* UART RTS and CTS position to be used in  USB_MONITORING_SetPins() */
#define USB_MONITORING_UART_RTS_PIN_ENABLE    (0x10u)
#define USB_MONITORING_UART_CTS_PIN_ENABLE    (0x20u)


/***************************************
* The following code is DEPRECATED and
* must not be used.
***************************************/

/* Interrupt processing */
#define USB_MONITORING_SpiUartEnableIntRx(intSourceMask)  USB_MONITORING_SetRxInterruptMode(intSourceMask)
#define USB_MONITORING_SpiUartEnableIntTx(intSourceMask)  USB_MONITORING_SetTxInterruptMode(intSourceMask)
uint32  USB_MONITORING_SpiUartDisableIntRx(void);
uint32  USB_MONITORING_SpiUartDisableIntTx(void);


#endif /* (CY_SCB_SPI_UART_PVT_USB_MONITORING_H) */


/* [] END OF FILE */
