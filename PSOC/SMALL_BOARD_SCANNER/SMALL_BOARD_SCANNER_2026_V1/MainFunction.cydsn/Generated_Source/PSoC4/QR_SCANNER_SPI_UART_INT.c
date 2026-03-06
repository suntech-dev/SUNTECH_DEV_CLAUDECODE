/***************************************************************************//**
* \file QR_SCANNER_SPI_UART_INT.c
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

#include "QR_SCANNER_PVT.h"
#include "QR_SCANNER_SPI_UART_PVT.h"
#include "cyapicallbacks.h"

#if (QR_SCANNER_SCB_IRQ_INTERNAL)
/*******************************************************************************
* Function Name: QR_SCANNER_SPI_UART_ISR
****************************************************************************//**
*
*  Handles the Interrupt Service Routine for the SCB SPI or UART modes.
*
*******************************************************************************/
CY_ISR(QR_SCANNER_SPI_UART_ISR)
{
#if (QR_SCANNER_INTERNAL_RX_SW_BUFFER_CONST)
    uint32 locHead;
#endif /* (QR_SCANNER_INTERNAL_RX_SW_BUFFER_CONST) */

#if (QR_SCANNER_INTERNAL_TX_SW_BUFFER_CONST)
    uint32 locTail;
#endif /* (QR_SCANNER_INTERNAL_TX_SW_BUFFER_CONST) */

#ifdef QR_SCANNER_SPI_UART_ISR_ENTRY_CALLBACK
    QR_SCANNER_SPI_UART_ISR_EntryCallback();
#endif /* QR_SCANNER_SPI_UART_ISR_ENTRY_CALLBACK */

    if (NULL != QR_SCANNER_customIntrHandler)
    {
        QR_SCANNER_customIntrHandler();
    }

    #if(QR_SCANNER_CHECK_SPI_WAKE_ENABLE)
    {
        /* Clear SPI wakeup source */
        QR_SCANNER_ClearSpiExtClkInterruptSource(QR_SCANNER_INTR_SPI_EC_WAKE_UP);
    }
    #endif

    #if (QR_SCANNER_CHECK_RX_SW_BUFFER)
    {
        if (QR_SCANNER_CHECK_INTR_RX_MASKED(QR_SCANNER_INTR_RX_NOT_EMPTY))
        {
            do
            {
                /* Move local head index */
                locHead = (QR_SCANNER_rxBufferHead + 1u);

                /* Adjust local head index */
                if (QR_SCANNER_INTERNAL_RX_BUFFER_SIZE == locHead)
                {
                    locHead = 0u;
                }

                if (locHead == QR_SCANNER_rxBufferTail)
                {
                    #if (QR_SCANNER_CHECK_UART_RTS_CONTROL_FLOW)
                    {
                        /* There is no space in the software buffer - disable the
                        * RX Not Empty interrupt source. The data elements are
                        * still being received into the RX FIFO until the RTS signal
                        * stops the transmitter. After the data element is read from the
                        * buffer, the RX Not Empty interrupt source is enabled to
                        * move the next data element in the software buffer.
                        */
                        QR_SCANNER_INTR_RX_MASK_REG &= ~QR_SCANNER_INTR_RX_NOT_EMPTY;
                        break;
                    }
                    #else
                    {
                        /* Overflow: through away received data element */
                        (void) QR_SCANNER_RX_FIFO_RD_REG;
                        QR_SCANNER_rxBufferOverflow = (uint8) QR_SCANNER_INTR_RX_OVERFLOW;
                    }
                    #endif
                }
                else
                {
                    /* Store received data */
                    QR_SCANNER_PutWordInRxBuffer(locHead, QR_SCANNER_RX_FIFO_RD_REG);

                    /* Move head index */
                    QR_SCANNER_rxBufferHead = locHead;
                }
            }
            while(0u != QR_SCANNER_GET_RX_FIFO_ENTRIES);

            QR_SCANNER_ClearRxInterruptSource(QR_SCANNER_INTR_RX_NOT_EMPTY);
        }
    }
    #endif


    #if (QR_SCANNER_CHECK_TX_SW_BUFFER)
    {
        if (QR_SCANNER_CHECK_INTR_TX_MASKED(QR_SCANNER_INTR_TX_NOT_FULL))
        {
            do
            {
                /* Check for room in TX software buffer */
                if (QR_SCANNER_txBufferHead != QR_SCANNER_txBufferTail)
                {
                    /* Move local tail index */
                    locTail = (QR_SCANNER_txBufferTail + 1u);

                    /* Adjust local tail index */
                    if (QR_SCANNER_TX_BUFFER_SIZE == locTail)
                    {
                        locTail = 0u;
                    }

                    /* Put data into TX FIFO */
                    QR_SCANNER_TX_FIFO_WR_REG = QR_SCANNER_GetWordFromTxBuffer(locTail);

                    /* Move tail index */
                    QR_SCANNER_txBufferTail = locTail;
                }
                else
                {
                    /* TX software buffer is empty: complete transfer */
                    QR_SCANNER_DISABLE_INTR_TX(QR_SCANNER_INTR_TX_NOT_FULL);
                    break;
                }
            }
            while (QR_SCANNER_SPI_UART_FIFO_SIZE != QR_SCANNER_GET_TX_FIFO_ENTRIES);

            QR_SCANNER_ClearTxInterruptSource(QR_SCANNER_INTR_TX_NOT_FULL);
        }
    }
    #endif

#ifdef QR_SCANNER_SPI_UART_ISR_EXIT_CALLBACK
    QR_SCANNER_SPI_UART_ISR_ExitCallback();
#endif /* QR_SCANNER_SPI_UART_ISR_EXIT_CALLBACK */

}

#endif /* (QR_SCANNER_SCB_IRQ_INTERNAL) */


/* [] END OF FILE */
