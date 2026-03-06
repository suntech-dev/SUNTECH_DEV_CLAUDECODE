/***************************************************************************//**
* \file WIFI_SPI_UART_INT.c
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

#include "WIFI_PVT.h"
#include "WIFI_SPI_UART_PVT.h"
#include "cyapicallbacks.h"

#if (WIFI_SCB_IRQ_INTERNAL)
/*******************************************************************************
* Function Name: WIFI_SPI_UART_ISR
****************************************************************************//**
*
*  Handles the Interrupt Service Routine for the SCB SPI or UART modes.
*
*******************************************************************************/
CY_ISR(WIFI_SPI_UART_ISR)
{
#if (WIFI_INTERNAL_RX_SW_BUFFER_CONST)
    uint32 locHead;
#endif /* (WIFI_INTERNAL_RX_SW_BUFFER_CONST) */

#if (WIFI_INTERNAL_TX_SW_BUFFER_CONST)
    uint32 locTail;
#endif /* (WIFI_INTERNAL_TX_SW_BUFFER_CONST) */

#ifdef WIFI_SPI_UART_ISR_ENTRY_CALLBACK
    WIFI_SPI_UART_ISR_EntryCallback();
#endif /* WIFI_SPI_UART_ISR_ENTRY_CALLBACK */

    if (NULL != WIFI_customIntrHandler)
    {
        WIFI_customIntrHandler();
    }

    #if(WIFI_CHECK_SPI_WAKE_ENABLE)
    {
        /* Clear SPI wakeup source */
        WIFI_ClearSpiExtClkInterruptSource(WIFI_INTR_SPI_EC_WAKE_UP);
    }
    #endif

    #if (WIFI_CHECK_RX_SW_BUFFER)
    {
        if (WIFI_CHECK_INTR_RX_MASKED(WIFI_INTR_RX_NOT_EMPTY))
        {
            do
            {
                /* Move local head index */
                locHead = (WIFI_rxBufferHead + 1u);

                /* Adjust local head index */
                if (WIFI_INTERNAL_RX_BUFFER_SIZE == locHead)
                {
                    locHead = 0u;
                }

                if (locHead == WIFI_rxBufferTail)
                {
                    #if (WIFI_CHECK_UART_RTS_CONTROL_FLOW)
                    {
                        /* There is no space in the software buffer - disable the
                        * RX Not Empty interrupt source. The data elements are
                        * still being received into the RX FIFO until the RTS signal
                        * stops the transmitter. After the data element is read from the
                        * buffer, the RX Not Empty interrupt source is enabled to
                        * move the next data element in the software buffer.
                        */
                        WIFI_INTR_RX_MASK_REG &= ~WIFI_INTR_RX_NOT_EMPTY;
                        break;
                    }
                    #else
                    {
                        /* Overflow: through away received data element */
                        (void) WIFI_RX_FIFO_RD_REG;
                        WIFI_rxBufferOverflow = (uint8) WIFI_INTR_RX_OVERFLOW;
                    }
                    #endif
                }
                else
                {
                    /* Store received data */
                    WIFI_PutWordInRxBuffer(locHead, WIFI_RX_FIFO_RD_REG);

                    /* Move head index */
                    WIFI_rxBufferHead = locHead;
                }
            }
            while(0u != WIFI_GET_RX_FIFO_ENTRIES);

            WIFI_ClearRxInterruptSource(WIFI_INTR_RX_NOT_EMPTY);
        }
    }
    #endif


    #if (WIFI_CHECK_TX_SW_BUFFER)
    {
        if (WIFI_CHECK_INTR_TX_MASKED(WIFI_INTR_TX_NOT_FULL))
        {
            do
            {
                /* Check for room in TX software buffer */
                if (WIFI_txBufferHead != WIFI_txBufferTail)
                {
                    /* Move local tail index */
                    locTail = (WIFI_txBufferTail + 1u);

                    /* Adjust local tail index */
                    if (WIFI_TX_BUFFER_SIZE == locTail)
                    {
                        locTail = 0u;
                    }

                    /* Put data into TX FIFO */
                    WIFI_TX_FIFO_WR_REG = WIFI_GetWordFromTxBuffer(locTail);

                    /* Move tail index */
                    WIFI_txBufferTail = locTail;
                }
                else
                {
                    /* TX software buffer is empty: complete transfer */
                    WIFI_DISABLE_INTR_TX(WIFI_INTR_TX_NOT_FULL);
                    break;
                }
            }
            while (WIFI_SPI_UART_FIFO_SIZE != WIFI_GET_TX_FIFO_ENTRIES);

            WIFI_ClearTxInterruptSource(WIFI_INTR_TX_NOT_FULL);
        }
    }
    #endif

#ifdef WIFI_SPI_UART_ISR_EXIT_CALLBACK
    WIFI_SPI_UART_ISR_ExitCallback();
#endif /* WIFI_SPI_UART_ISR_EXIT_CALLBACK */

}

#endif /* (WIFI_SCB_IRQ_INTERNAL) */


/* [] END OF FILE */
