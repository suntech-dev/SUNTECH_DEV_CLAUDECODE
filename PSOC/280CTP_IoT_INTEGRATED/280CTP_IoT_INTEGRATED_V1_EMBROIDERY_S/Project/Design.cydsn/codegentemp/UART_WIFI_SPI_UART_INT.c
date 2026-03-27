/***************************************************************************//**
* \file UART_WIFI_SPI_UART_INT.c
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

#include "UART_WIFI_PVT.h"
#include "UART_WIFI_SPI_UART_PVT.h"
#include "cyapicallbacks.h"

#if (UART_WIFI_SCB_IRQ_INTERNAL)
/*******************************************************************************
* Function Name: UART_WIFI_SPI_UART_ISR
****************************************************************************//**
*
*  Handles the Interrupt Service Routine for the SCB SPI or UART modes.
*
*******************************************************************************/
CY_ISR(UART_WIFI_SPI_UART_ISR)
{
#if (UART_WIFI_INTERNAL_RX_SW_BUFFER_CONST)
    uint32 locHead;
#endif /* (UART_WIFI_INTERNAL_RX_SW_BUFFER_CONST) */

#if (UART_WIFI_INTERNAL_TX_SW_BUFFER_CONST)
    uint32 locTail;
#endif /* (UART_WIFI_INTERNAL_TX_SW_BUFFER_CONST) */

#ifdef UART_WIFI_SPI_UART_ISR_ENTRY_CALLBACK
    UART_WIFI_SPI_UART_ISR_EntryCallback();
#endif /* UART_WIFI_SPI_UART_ISR_ENTRY_CALLBACK */

    if (NULL != UART_WIFI_customIntrHandler)
    {
        UART_WIFI_customIntrHandler();
    }

    #if(UART_WIFI_CHECK_SPI_WAKE_ENABLE)
    {
        /* Clear SPI wakeup source */
        UART_WIFI_ClearSpiExtClkInterruptSource(UART_WIFI_INTR_SPI_EC_WAKE_UP);
    }
    #endif

    #if (UART_WIFI_CHECK_RX_SW_BUFFER)
    {
        if (UART_WIFI_CHECK_INTR_RX_MASKED(UART_WIFI_INTR_RX_NOT_EMPTY))
        {
            do
            {
                /* Move local head index */
                locHead = (UART_WIFI_rxBufferHead + 1u);

                /* Adjust local head index */
                if (UART_WIFI_INTERNAL_RX_BUFFER_SIZE == locHead)
                {
                    locHead = 0u;
                }

                if (locHead == UART_WIFI_rxBufferTail)
                {
                    #if (UART_WIFI_CHECK_UART_RTS_CONTROL_FLOW)
                    {
                        /* There is no space in the software buffer - disable the
                        * RX Not Empty interrupt source. The data elements are
                        * still being received into the RX FIFO until the RTS signal
                        * stops the transmitter. After the data element is read from the
                        * buffer, the RX Not Empty interrupt source is enabled to
                        * move the next data element in the software buffer.
                        */
                        UART_WIFI_INTR_RX_MASK_REG &= ~UART_WIFI_INTR_RX_NOT_EMPTY;
                        break;
                    }
                    #else
                    {
                        /* Overflow: through away received data element */
                        (void) UART_WIFI_RX_FIFO_RD_REG;
                        UART_WIFI_rxBufferOverflow = (uint8) UART_WIFI_INTR_RX_OVERFLOW;
                    }
                    #endif
                }
                else
                {
                    /* Store received data */
                    UART_WIFI_PutWordInRxBuffer(locHead, UART_WIFI_RX_FIFO_RD_REG);

                    /* Move head index */
                    UART_WIFI_rxBufferHead = locHead;
                }
            }
            while(0u != UART_WIFI_GET_RX_FIFO_ENTRIES);

            UART_WIFI_ClearRxInterruptSource(UART_WIFI_INTR_RX_NOT_EMPTY);
        }
    }
    #endif


    #if (UART_WIFI_CHECK_TX_SW_BUFFER)
    {
        if (UART_WIFI_CHECK_INTR_TX_MASKED(UART_WIFI_INTR_TX_NOT_FULL))
        {
            do
            {
                /* Check for room in TX software buffer */
                if (UART_WIFI_txBufferHead != UART_WIFI_txBufferTail)
                {
                    /* Move local tail index */
                    locTail = (UART_WIFI_txBufferTail + 1u);

                    /* Adjust local tail index */
                    if (UART_WIFI_TX_BUFFER_SIZE == locTail)
                    {
                        locTail = 0u;
                    }

                    /* Put data into TX FIFO */
                    UART_WIFI_TX_FIFO_WR_REG = UART_WIFI_GetWordFromTxBuffer(locTail);

                    /* Move tail index */
                    UART_WIFI_txBufferTail = locTail;
                }
                else
                {
                    /* TX software buffer is empty: complete transfer */
                    UART_WIFI_DISABLE_INTR_TX(UART_WIFI_INTR_TX_NOT_FULL);
                    break;
                }
            }
            while (UART_WIFI_SPI_UART_FIFO_SIZE != UART_WIFI_GET_TX_FIFO_ENTRIES);

            UART_WIFI_ClearTxInterruptSource(UART_WIFI_INTR_TX_NOT_FULL);
        }
    }
    #endif

#ifdef UART_WIFI_SPI_UART_ISR_EXIT_CALLBACK
    UART_WIFI_SPI_UART_ISR_ExitCallback();
#endif /* UART_WIFI_SPI_UART_ISR_EXIT_CALLBACK */

}

#endif /* (UART_WIFI_SCB_IRQ_INTERNAL) */


/* [] END OF FILE */
