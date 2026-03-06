/***************************************************************************//**
* \file port_MONITORING_SPI_UART_INT.c
* \version 4.0
*
* \brief
*  This file provides the source code to the Interrupt Service Routine for
*  the SCB Component in SPI and UART modes.
*
* Note:
*
********************************************************************************
* \copyright
* Copyright 2013-2017, Cypress Semiconductor Corporation.  All rights reserved.
* You may use this file only in accordance with the license, terms, conditions,
* disclaimers, and limitations in the end user license agreement accompanying
* the software package with which this file was provided.
*******************************************************************************/

#include "port_MONITORING_PVT.h"
#include "port_MONITORING_SPI_UART_PVT.h"
#include "cyapicallbacks.h"

#if (port_MONITORING_SCB_IRQ_INTERNAL)
/*******************************************************************************
* Function Name: port_MONITORING_SPI_UART_ISR
****************************************************************************//**
*
*  Handles the Interrupt Service Routine for the SCB SPI or UART modes.
*
*******************************************************************************/
CY_ISR(port_MONITORING_SPI_UART_ISR)
{
#if (port_MONITORING_INTERNAL_RX_SW_BUFFER_CONST)
    uint32 locHead;
#endif /* (port_MONITORING_INTERNAL_RX_SW_BUFFER_CONST) */

#if (port_MONITORING_INTERNAL_TX_SW_BUFFER_CONST)
    uint32 locTail;
#endif /* (port_MONITORING_INTERNAL_TX_SW_BUFFER_CONST) */

#ifdef port_MONITORING_SPI_UART_ISR_ENTRY_CALLBACK
    port_MONITORING_SPI_UART_ISR_EntryCallback();
#endif /* port_MONITORING_SPI_UART_ISR_ENTRY_CALLBACK */

    if (NULL != port_MONITORING_customIntrHandler)
    {
        port_MONITORING_customIntrHandler();
    }

    #if(port_MONITORING_CHECK_SPI_WAKE_ENABLE)
    {
        /* Clear SPI wakeup source */
        port_MONITORING_ClearSpiExtClkInterruptSource(port_MONITORING_INTR_SPI_EC_WAKE_UP);
    }
    #endif

    #if (port_MONITORING_CHECK_RX_SW_BUFFER)
    {
        if (port_MONITORING_CHECK_INTR_RX_MASKED(port_MONITORING_INTR_RX_NOT_EMPTY))
        {
            do
            {
                /* Move local head index */
                locHead = (port_MONITORING_rxBufferHead + 1u);

                /* Adjust local head index */
                if (port_MONITORING_INTERNAL_RX_BUFFER_SIZE == locHead)
                {
                    locHead = 0u;
                }

                if (locHead == port_MONITORING_rxBufferTail)
                {
                    #if (port_MONITORING_CHECK_UART_RTS_CONTROL_FLOW)
                    {
                        /* There is no space in the software buffer - disable the
                        * RX Not Empty interrupt source. The data elements are
                        * still being received into the RX FIFO until the RTS signal
                        * stops the transmitter. After the data element is read from the
                        * buffer, the RX Not Empty interrupt source is enabled to
                        * move the next data element in the software buffer.
                        */
                        port_MONITORING_INTR_RX_MASK_REG &= ~port_MONITORING_INTR_RX_NOT_EMPTY;
                        break;
                    }
                    #else
                    {
                        /* Overflow: through away received data element */
                        (void) port_MONITORING_RX_FIFO_RD_REG;
                        port_MONITORING_rxBufferOverflow = (uint8) port_MONITORING_INTR_RX_OVERFLOW;
                    }
                    #endif
                }
                else
                {
                    /* Store received data */
                    port_MONITORING_PutWordInRxBuffer(locHead, port_MONITORING_RX_FIFO_RD_REG);

                    /* Move head index */
                    port_MONITORING_rxBufferHead = locHead;
                }
            }
            while(0u != port_MONITORING_GET_RX_FIFO_ENTRIES);

            port_MONITORING_ClearRxInterruptSource(port_MONITORING_INTR_RX_NOT_EMPTY);
        }
    }
    #endif


    #if (port_MONITORING_CHECK_TX_SW_BUFFER)
    {
        if (port_MONITORING_CHECK_INTR_TX_MASKED(port_MONITORING_INTR_TX_NOT_FULL))
        {
            do
            {
                /* Check for room in TX software buffer */
                if (port_MONITORING_txBufferHead != port_MONITORING_txBufferTail)
                {
                    /* Move local tail index */
                    locTail = (port_MONITORING_txBufferTail + 1u);

                    /* Adjust local tail index */
                    if (port_MONITORING_TX_BUFFER_SIZE == locTail)
                    {
                        locTail = 0u;
                    }

                    /* Put data into TX FIFO */
                    port_MONITORING_TX_FIFO_WR_REG = port_MONITORING_GetWordFromTxBuffer(locTail);

                    /* Move tail index */
                    port_MONITORING_txBufferTail = locTail;
                }
                else
                {
                    /* TX software buffer is empty: complete transfer */
                    port_MONITORING_DISABLE_INTR_TX(port_MONITORING_INTR_TX_NOT_FULL);
                    break;
                }
            }
            while (port_MONITORING_SPI_UART_FIFO_SIZE != port_MONITORING_GET_TX_FIFO_ENTRIES);

            port_MONITORING_ClearTxInterruptSource(port_MONITORING_INTR_TX_NOT_FULL);
        }
    }
    #endif

#ifdef port_MONITORING_SPI_UART_ISR_EXIT_CALLBACK
    port_MONITORING_SPI_UART_ISR_ExitCallback();
#endif /* port_MONITORING_SPI_UART_ISR_EXIT_CALLBACK */

}

#endif /* (port_MONITORING_SCB_IRQ_INTERNAL) */


/* [] END OF FILE */
