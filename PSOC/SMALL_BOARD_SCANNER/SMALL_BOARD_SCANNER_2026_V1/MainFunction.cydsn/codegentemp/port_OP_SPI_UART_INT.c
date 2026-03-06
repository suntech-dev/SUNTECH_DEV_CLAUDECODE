/***************************************************************************//**
* \file port_OP_SPI_UART_INT.c
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

#include "port_OP_PVT.h"
#include "port_OP_SPI_UART_PVT.h"
#include "cyapicallbacks.h"

#if (port_OP_SCB_IRQ_INTERNAL)
/*******************************************************************************
* Function Name: port_OP_SPI_UART_ISR
****************************************************************************//**
*
*  Handles the Interrupt Service Routine for the SCB SPI or UART modes.
*
*******************************************************************************/
CY_ISR(port_OP_SPI_UART_ISR)
{
#if (port_OP_INTERNAL_RX_SW_BUFFER_CONST)
    uint32 locHead;
#endif /* (port_OP_INTERNAL_RX_SW_BUFFER_CONST) */

#if (port_OP_INTERNAL_TX_SW_BUFFER_CONST)
    uint32 locTail;
#endif /* (port_OP_INTERNAL_TX_SW_BUFFER_CONST) */

#ifdef port_OP_SPI_UART_ISR_ENTRY_CALLBACK
    port_OP_SPI_UART_ISR_EntryCallback();
#endif /* port_OP_SPI_UART_ISR_ENTRY_CALLBACK */

    if (NULL != port_OP_customIntrHandler)
    {
        port_OP_customIntrHandler();
    }

    #if(port_OP_CHECK_SPI_WAKE_ENABLE)
    {
        /* Clear SPI wakeup source */
        port_OP_ClearSpiExtClkInterruptSource(port_OP_INTR_SPI_EC_WAKE_UP);
    }
    #endif

    #if (port_OP_CHECK_RX_SW_BUFFER)
    {
        if (port_OP_CHECK_INTR_RX_MASKED(port_OP_INTR_RX_NOT_EMPTY))
        {
            do
            {
                /* Move local head index */
                locHead = (port_OP_rxBufferHead + 1u);

                /* Adjust local head index */
                if (port_OP_INTERNAL_RX_BUFFER_SIZE == locHead)
                {
                    locHead = 0u;
                }

                if (locHead == port_OP_rxBufferTail)
                {
                    #if (port_OP_CHECK_UART_RTS_CONTROL_FLOW)
                    {
                        /* There is no space in the software buffer - disable the
                        * RX Not Empty interrupt source. The data elements are
                        * still being received into the RX FIFO until the RTS signal
                        * stops the transmitter. After the data element is read from the
                        * buffer, the RX Not Empty interrupt source is enabled to
                        * move the next data element in the software buffer.
                        */
                        port_OP_INTR_RX_MASK_REG &= ~port_OP_INTR_RX_NOT_EMPTY;
                        break;
                    }
                    #else
                    {
                        /* Overflow: through away received data element */
                        (void) port_OP_RX_FIFO_RD_REG;
                        port_OP_rxBufferOverflow = (uint8) port_OP_INTR_RX_OVERFLOW;
                    }
                    #endif
                }
                else
                {
                    /* Store received data */
                    port_OP_PutWordInRxBuffer(locHead, port_OP_RX_FIFO_RD_REG);

                    /* Move head index */
                    port_OP_rxBufferHead = locHead;
                }
            }
            while(0u != port_OP_GET_RX_FIFO_ENTRIES);

            port_OP_ClearRxInterruptSource(port_OP_INTR_RX_NOT_EMPTY);
        }
    }
    #endif


    #if (port_OP_CHECK_TX_SW_BUFFER)
    {
        if (port_OP_CHECK_INTR_TX_MASKED(port_OP_INTR_TX_NOT_FULL))
        {
            do
            {
                /* Check for room in TX software buffer */
                if (port_OP_txBufferHead != port_OP_txBufferTail)
                {
                    /* Move local tail index */
                    locTail = (port_OP_txBufferTail + 1u);

                    /* Adjust local tail index */
                    if (port_OP_TX_BUFFER_SIZE == locTail)
                    {
                        locTail = 0u;
                    }

                    /* Put data into TX FIFO */
                    port_OP_TX_FIFO_WR_REG = port_OP_GetWordFromTxBuffer(locTail);

                    /* Move tail index */
                    port_OP_txBufferTail = locTail;
                }
                else
                {
                    /* TX software buffer is empty: complete transfer */
                    port_OP_DISABLE_INTR_TX(port_OP_INTR_TX_NOT_FULL);
                    break;
                }
            }
            while (port_OP_SPI_UART_FIFO_SIZE != port_OP_GET_TX_FIFO_ENTRIES);

            port_OP_ClearTxInterruptSource(port_OP_INTR_TX_NOT_FULL);
        }
    }
    #endif

#ifdef port_OP_SPI_UART_ISR_EXIT_CALLBACK
    port_OP_SPI_UART_ISR_ExitCallback();
#endif /* port_OP_SPI_UART_ISR_EXIT_CALLBACK */

}

#endif /* (port_OP_SCB_IRQ_INTERNAL) */


/* [] END OF FILE */
