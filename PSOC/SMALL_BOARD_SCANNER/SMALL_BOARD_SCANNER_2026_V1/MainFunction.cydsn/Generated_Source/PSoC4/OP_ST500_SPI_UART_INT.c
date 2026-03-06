/***************************************************************************//**
* \file OP_ST500_SPI_UART_INT.c
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

#include "OP_ST500_PVT.h"
#include "OP_ST500_SPI_UART_PVT.h"
#include "cyapicallbacks.h"

#if (OP_ST500_SCB_IRQ_INTERNAL)
/*******************************************************************************
* Function Name: OP_ST500_SPI_UART_ISR
****************************************************************************//**
*
*  Handles the Interrupt Service Routine for the SCB SPI or UART modes.
*
*******************************************************************************/
CY_ISR(OP_ST500_SPI_UART_ISR)
{
#if (OP_ST500_INTERNAL_RX_SW_BUFFER_CONST)
    uint32 locHead;
#endif /* (OP_ST500_INTERNAL_RX_SW_BUFFER_CONST) */

#if (OP_ST500_INTERNAL_TX_SW_BUFFER_CONST)
    uint32 locTail;
#endif /* (OP_ST500_INTERNAL_TX_SW_BUFFER_CONST) */

#ifdef OP_ST500_SPI_UART_ISR_ENTRY_CALLBACK
    OP_ST500_SPI_UART_ISR_EntryCallback();
#endif /* OP_ST500_SPI_UART_ISR_ENTRY_CALLBACK */

    if (NULL != OP_ST500_customIntrHandler)
    {
        OP_ST500_customIntrHandler();
    }

    #if(OP_ST500_CHECK_SPI_WAKE_ENABLE)
    {
        /* Clear SPI wakeup source */
        OP_ST500_ClearSpiExtClkInterruptSource(OP_ST500_INTR_SPI_EC_WAKE_UP);
    }
    #endif

    #if (OP_ST500_CHECK_RX_SW_BUFFER)
    {
        if (OP_ST500_CHECK_INTR_RX_MASKED(OP_ST500_INTR_RX_NOT_EMPTY))
        {
            do
            {
                /* Move local head index */
                locHead = (OP_ST500_rxBufferHead + 1u);

                /* Adjust local head index */
                if (OP_ST500_INTERNAL_RX_BUFFER_SIZE == locHead)
                {
                    locHead = 0u;
                }

                if (locHead == OP_ST500_rxBufferTail)
                {
                    #if (OP_ST500_CHECK_UART_RTS_CONTROL_FLOW)
                    {
                        /* There is no space in the software buffer - disable the
                        * RX Not Empty interrupt source. The data elements are
                        * still being received into the RX FIFO until the RTS signal
                        * stops the transmitter. After the data element is read from the
                        * buffer, the RX Not Empty interrupt source is enabled to
                        * move the next data element in the software buffer.
                        */
                        OP_ST500_INTR_RX_MASK_REG &= ~OP_ST500_INTR_RX_NOT_EMPTY;
                        break;
                    }
                    #else
                    {
                        /* Overflow: through away received data element */
                        (void) OP_ST500_RX_FIFO_RD_REG;
                        OP_ST500_rxBufferOverflow = (uint8) OP_ST500_INTR_RX_OVERFLOW;
                    }
                    #endif
                }
                else
                {
                    /* Store received data */
                    OP_ST500_PutWordInRxBuffer(locHead, OP_ST500_RX_FIFO_RD_REG);

                    /* Move head index */
                    OP_ST500_rxBufferHead = locHead;
                }
            }
            while(0u != OP_ST500_GET_RX_FIFO_ENTRIES);

            OP_ST500_ClearRxInterruptSource(OP_ST500_INTR_RX_NOT_EMPTY);
        }
    }
    #endif


    #if (OP_ST500_CHECK_TX_SW_BUFFER)
    {
        if (OP_ST500_CHECK_INTR_TX_MASKED(OP_ST500_INTR_TX_NOT_FULL))
        {
            do
            {
                /* Check for room in TX software buffer */
                if (OP_ST500_txBufferHead != OP_ST500_txBufferTail)
                {
                    /* Move local tail index */
                    locTail = (OP_ST500_txBufferTail + 1u);

                    /* Adjust local tail index */
                    if (OP_ST500_TX_BUFFER_SIZE == locTail)
                    {
                        locTail = 0u;
                    }

                    /* Put data into TX FIFO */
                    OP_ST500_TX_FIFO_WR_REG = OP_ST500_GetWordFromTxBuffer(locTail);

                    /* Move tail index */
                    OP_ST500_txBufferTail = locTail;
                }
                else
                {
                    /* TX software buffer is empty: complete transfer */
                    OP_ST500_DISABLE_INTR_TX(OP_ST500_INTR_TX_NOT_FULL);
                    break;
                }
            }
            while (OP_ST500_SPI_UART_FIFO_SIZE != OP_ST500_GET_TX_FIFO_ENTRIES);

            OP_ST500_ClearTxInterruptSource(OP_ST500_INTR_TX_NOT_FULL);
        }
    }
    #endif

#ifdef OP_ST500_SPI_UART_ISR_EXIT_CALLBACK
    OP_ST500_SPI_UART_ISR_ExitCallback();
#endif /* OP_ST500_SPI_UART_ISR_EXIT_CALLBACK */

}

#endif /* (OP_ST500_SCB_IRQ_INTERNAL) */


/* [] END OF FILE */
