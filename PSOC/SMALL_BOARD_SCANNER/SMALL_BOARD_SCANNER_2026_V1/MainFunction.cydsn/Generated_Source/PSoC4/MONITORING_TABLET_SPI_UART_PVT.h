/***************************************************************************//**
* \file MONITORING_TABLET_SPI_UART_PVT.h
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

#if !defined(CY_SCB_SPI_UART_PVT_MONITORING_TABLET_H)
#define CY_SCB_SPI_UART_PVT_MONITORING_TABLET_H

#include "MONITORING_TABLET_SPI_UART.h"


/***************************************
*     Internal Global Vars
***************************************/

#if (MONITORING_TABLET_INTERNAL_RX_SW_BUFFER_CONST)
    extern volatile uint32  MONITORING_TABLET_rxBufferHead;
    extern volatile uint32  MONITORING_TABLET_rxBufferTail;
    
    /**
    * \addtogroup group_globals
    * @{
    */
    
    /** Sets when internal software receive buffer overflow
     *  was occurred.
    */  
    extern volatile uint8   MONITORING_TABLET_rxBufferOverflow;
    /** @} globals */
#endif /* (MONITORING_TABLET_INTERNAL_RX_SW_BUFFER_CONST) */

#if (MONITORING_TABLET_INTERNAL_TX_SW_BUFFER_CONST)
    extern volatile uint32  MONITORING_TABLET_txBufferHead;
    extern volatile uint32  MONITORING_TABLET_txBufferTail;
#endif /* (MONITORING_TABLET_INTERNAL_TX_SW_BUFFER_CONST) */

#if (MONITORING_TABLET_INTERNAL_RX_SW_BUFFER)
    extern volatile uint8 MONITORING_TABLET_rxBufferInternal[MONITORING_TABLET_INTERNAL_RX_BUFFER_SIZE];
#endif /* (MONITORING_TABLET_INTERNAL_RX_SW_BUFFER) */

#if (MONITORING_TABLET_INTERNAL_TX_SW_BUFFER)
    extern volatile uint8 MONITORING_TABLET_txBufferInternal[MONITORING_TABLET_TX_BUFFER_SIZE];
#endif /* (MONITORING_TABLET_INTERNAL_TX_SW_BUFFER) */


/***************************************
*     Private Function Prototypes
***************************************/

void MONITORING_TABLET_SpiPostEnable(void);
void MONITORING_TABLET_SpiStop(void);

#if (MONITORING_TABLET_SCB_MODE_SPI_CONST_CFG)
    void MONITORING_TABLET_SpiInit(void);
#endif /* (MONITORING_TABLET_SCB_MODE_SPI_CONST_CFG) */

#if (MONITORING_TABLET_SPI_WAKE_ENABLE_CONST)
    void MONITORING_TABLET_SpiSaveConfig(void);
    void MONITORING_TABLET_SpiRestoreConfig(void);
#endif /* (MONITORING_TABLET_SPI_WAKE_ENABLE_CONST) */

void MONITORING_TABLET_UartPostEnable(void);
void MONITORING_TABLET_UartStop(void);

#if (MONITORING_TABLET_SCB_MODE_UART_CONST_CFG)
    void MONITORING_TABLET_UartInit(void);
#endif /* (MONITORING_TABLET_SCB_MODE_UART_CONST_CFG) */

#if (MONITORING_TABLET_UART_WAKE_ENABLE_CONST)
    void MONITORING_TABLET_UartSaveConfig(void);
    void MONITORING_TABLET_UartRestoreConfig(void);
#endif /* (MONITORING_TABLET_UART_WAKE_ENABLE_CONST) */


/***************************************
*         UART API Constants
***************************************/

/* UART RX and TX position to be used in MONITORING_TABLET_SetPins() */
#define MONITORING_TABLET_UART_RX_PIN_ENABLE    (MONITORING_TABLET_UART_RX)
#define MONITORING_TABLET_UART_TX_PIN_ENABLE    (MONITORING_TABLET_UART_TX)

/* UART RTS and CTS position to be used in  MONITORING_TABLET_SetPins() */
#define MONITORING_TABLET_UART_RTS_PIN_ENABLE    (0x10u)
#define MONITORING_TABLET_UART_CTS_PIN_ENABLE    (0x20u)


/***************************************
* The following code is DEPRECATED and
* must not be used.
***************************************/

/* Interrupt processing */
#define MONITORING_TABLET_SpiUartEnableIntRx(intSourceMask)  MONITORING_TABLET_SetRxInterruptMode(intSourceMask)
#define MONITORING_TABLET_SpiUartEnableIntTx(intSourceMask)  MONITORING_TABLET_SetTxInterruptMode(intSourceMask)
uint32  MONITORING_TABLET_SpiUartDisableIntRx(void);
uint32  MONITORING_TABLET_SpiUartDisableIntTx(void);


#endif /* (CY_SCB_SPI_UART_PVT_MONITORING_TABLET_H) */


/* [] END OF FILE */
