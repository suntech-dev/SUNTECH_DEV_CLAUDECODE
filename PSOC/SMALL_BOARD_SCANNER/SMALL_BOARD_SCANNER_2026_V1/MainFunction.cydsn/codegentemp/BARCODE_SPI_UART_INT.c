/***************************************************************************//**
* \file BARCODE_SPI_UART_INT.c
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

#include "BARCODE_PVT.h"
#include "BARCODE_SPI_UART_PVT.h"
#include "cyapicallbacks.h"

#if (BARCODE_SCB_IRQ_INTERNAL)
/*******************************************************************************
* Function Name: BARCODE_SPI_UART_ISR
****************************************************************************//**
*
*  Handles the Interrupt Service Routine for the SCB SPI or UART modes.
*
*******************************************************************************/
CY_ISR(BARCODE_SPI_UART_ISR)
{
#if (BARCODE_INTERNAL_RX_SW_BUFFER_CONST)
    uint32 locHead;
#endif /* (BARCODE_INTERNAL_RX_SW_BUFFER_CONST) */

#if (BARCODE_INTERNAL_TX_SW_BUFFER_CONST)
    uint32 locTail;
#endif /* (BARCODE_INTERNAL_TX_SW_BUFFER_CONST) */

#ifdef BARCODE_SPI_UART_ISR_ENTRY_CALLBACK
    BARCODE_SPI_UART_ISR_EntryCallback();
#endif /* BARCODE_SPI_UART_ISR_ENTRY_CALLBACK */

    if (NULL != BARCODE_customIntrHandler)
    {
        BARCODE_customIntrHandler();
    }

    #if(BARCODE_CHECK_SPI_WAKE_ENABLE)
    {
        /* Clear SPI wakeup source */
        BARCODE_ClearSpiExtClkInterruptSource(BARCODE_INTR_SPI_EC_WAKE_UP);
    }
    #endif

    #if (BARCODE_CHECK_RX_SW_BUFFER)
    {
        if (BARCODE_CHECK_INTR_RX_MASKED(BARCODE_INTR_RX_NOT_EMPTY))
        {
            do
            {
                /* Move local head index */
                locHead = (BARCODE_rxBufferHead + 1u);

                /* Adjust local head index */
                if (BARCODE_INTERNAL_RX_BUFFER_SIZE == locHead)
                {
                    locHead = 0u;
                }

                if (locHead == BARCODE_rxBufferTail)
                {
                    #if (BARCODE_CHECK_UART_RTS_CONTROL_FLOW)
                    {
                        /* There is no space in the software buffer - disable the
                        * RX Not Empty interrupt source. The data elements are
                        * still being received into the RX FIFO until the RTS signal
                        * stops the transmitter. After the data element is read from the
                        * buffer, the RX Not Empty interrupt source is enabled to
                        * move the next data element in the software buffer.
                        */
                        BARCODE_INTR_RX_MASK_REG &= ~BARCODE_INTR_RX_NOT_EMPTY;
                        break;
                    }
                    #else
                    {
                        /* Overflow: through away received data element */
                        (void) BARCODE_RX_FIFO_RD_REG;
                        BARCODE_rxBufferOverflow = (uint8) BARCODE_INTR_RX_OVERFLOW;
                    }
                    #endif
                }
                else
                {
                    /* Store received data */
                    BARCODE_PutWordInRxBuffer(locHead, BARCODE_RX_FIFO_RD_REG);

                    /* Move head index */
                    BARCODE_rxBufferHead = locHead;
                }
            }
            while(0u != BARCODE_GET_RX_FIFO_ENTRIES);

            BARCODE_ClearRxInterruptSource(BARCODE_INTR_RX_NOT_EMPTY);
        }
    }
    #endif


    #if (BARCODE_CHECK_TX_SW_BUFFER)
    {
        if (BARCODE_CHECK_INTR_TX_MASKED(BARCODE_INTR_TX_NOT_FULL))
        {
            do
            {
                /* Check for room in TX software buffer */
                if (BARCODE_txBufferHead != BARCODE_txBufferTail)
                {
                    /* Move local tail index */
                    locTail = (BARCODE_txBufferTail + 1u);

                    /* Adjust local tail index */
                    if (BARCODE_TX_BUFFER_SIZE == locTail)
                    {
                        locTail = 0u;
                    }

                    /* Put data into TX FIFO */
                    BARCODE_TX_FIFO_WR_REG = BARCODE_GetWordFromTxBuffer(locTail);

                    /* Move tail index */
                    BARCODE_txBufferTail = locTail;
                }
                else
                {
                    /* TX software buffer is empty: complete transfer */
                    BARCODE_DISABLE_INTR_TX(BARCODE_INTR_TX_NOT_FULL);
                    break;
                }
            }
            while (BARCODE_SPI_UART_FIFO_SIZE != BARCODE_GET_TX_FIFO_ENTRIES);

            BARCODE_ClearTxInterruptSource(BARCODE_INTR_TX_NOT_FULL);
        }
    }
    #endif

#ifdef BARCODE_SPI_UART_ISR_EXIT_CALLBACK
    BARCODE_SPI_UART_ISR_ExitCallback();
#endif /* BARCODE_SPI_UART_ISR_EXIT_CALLBACK */

}

#endif /* (BARCODE_SCB_IRQ_INTERNAL) */


/* [] END OF FILE */
