/***************************************************************************//**
* \file MONITORING_TABLET_SPI_UART_INT.c
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

#include "MONITORING_TABLET_PVT.h"
#include "MONITORING_TABLET_SPI_UART_PVT.h"
#include "cyapicallbacks.h"

#if (MONITORING_TABLET_SCB_IRQ_INTERNAL)
/*******************************************************************************
* Function Name: MONITORING_TABLET_SPI_UART_ISR
****************************************************************************//**
*
*  Handles the Interrupt Service Routine for the SCB SPI or UART modes.
*
*******************************************************************************/
CY_ISR(MONITORING_TABLET_SPI_UART_ISR)
{
#if (MONITORING_TABLET_INTERNAL_RX_SW_BUFFER_CONST)
    uint32 locHead;
#endif /* (MONITORING_TABLET_INTERNAL_RX_SW_BUFFER_CONST) */

#if (MONITORING_TABLET_INTERNAL_TX_SW_BUFFER_CONST)
    uint32 locTail;
#endif /* (MONITORING_TABLET_INTERNAL_TX_SW_BUFFER_CONST) */

#ifdef MONITORING_TABLET_SPI_UART_ISR_ENTRY_CALLBACK
    MONITORING_TABLET_SPI_UART_ISR_EntryCallback();
#endif /* MONITORING_TABLET_SPI_UART_ISR_ENTRY_CALLBACK */

    if (NULL != MONITORING_TABLET_customIntrHandler)
    {
        MONITORING_TABLET_customIntrHandler();
    }

    #if(MONITORING_TABLET_CHECK_SPI_WAKE_ENABLE)
    {
        /* Clear SPI wakeup source */
        MONITORING_TABLET_ClearSpiExtClkInterruptSource(MONITORING_TABLET_INTR_SPI_EC_WAKE_UP);
    }
    #endif

    #if (MONITORING_TABLET_CHECK_RX_SW_BUFFER)
    {
        if (MONITORING_TABLET_CHECK_INTR_RX_MASKED(MONITORING_TABLET_INTR_RX_NOT_EMPTY))
        {
            do
            {
                /* Move local head index */
                locHead = (MONITORING_TABLET_rxBufferHead + 1u);

                /* Adjust local head index */
                if (MONITORING_TABLET_INTERNAL_RX_BUFFER_SIZE == locHead)
                {
                    locHead = 0u;
                }

                if (locHead == MONITORING_TABLET_rxBufferTail)
                {
                    #if (MONITORING_TABLET_CHECK_UART_RTS_CONTROL_FLOW)
                    {
                        /* There is no space in the software buffer - disable the
                        * RX Not Empty interrupt source. The data elements are
                        * still being received into the RX FIFO until the RTS signal
                        * stops the transmitter. After the data element is read from the
                        * buffer, the RX Not Empty interrupt source is enabled to
                        * move the next data element in the software buffer.
                        */
                        MONITORING_TABLET_INTR_RX_MASK_REG &= ~MONITORING_TABLET_INTR_RX_NOT_EMPTY;
                        break;
                    }
                    #else
                    {
                        /* Overflow: through away received data element */
                        (void) MONITORING_TABLET_RX_FIFO_RD_REG;
                        MONITORING_TABLET_rxBufferOverflow = (uint8) MONITORING_TABLET_INTR_RX_OVERFLOW;
                    }
                    #endif
                }
                else
                {
                    /* Store received data */
                    MONITORING_TABLET_PutWordInRxBuffer(locHead, MONITORING_TABLET_RX_FIFO_RD_REG);

                    /* Move head index */
                    MONITORING_TABLET_rxBufferHead = locHead;
                }
            }
            while(0u != MONITORING_TABLET_GET_RX_FIFO_ENTRIES);

            MONITORING_TABLET_ClearRxInterruptSource(MONITORING_TABLET_INTR_RX_NOT_EMPTY);
        }
    }
    #endif


    #if (MONITORING_TABLET_CHECK_TX_SW_BUFFER)
    {
        if (MONITORING_TABLET_CHECK_INTR_TX_MASKED(MONITORING_TABLET_INTR_TX_NOT_FULL))
        {
            do
            {
                /* Check for room in TX software buffer */
                if (MONITORING_TABLET_txBufferHead != MONITORING_TABLET_txBufferTail)
                {
                    /* Move local tail index */
                    locTail = (MONITORING_TABLET_txBufferTail + 1u);

                    /* Adjust local tail index */
                    if (MONITORING_TABLET_TX_BUFFER_SIZE == locTail)
                    {
                        locTail = 0u;
                    }

                    /* Put data into TX FIFO */
                    MONITORING_TABLET_TX_FIFO_WR_REG = MONITORING_TABLET_GetWordFromTxBuffer(locTail);

                    /* Move tail index */
                    MONITORING_TABLET_txBufferTail = locTail;
                }
                else
                {
                    /* TX software buffer is empty: complete transfer */
                    MONITORING_TABLET_DISABLE_INTR_TX(MONITORING_TABLET_INTR_TX_NOT_FULL);
                    break;
                }
            }
            while (MONITORING_TABLET_SPI_UART_FIFO_SIZE != MONITORING_TABLET_GET_TX_FIFO_ENTRIES);

            MONITORING_TABLET_ClearTxInterruptSource(MONITORING_TABLET_INTR_TX_NOT_FULL);
        }
    }
    #endif

#ifdef MONITORING_TABLET_SPI_UART_ISR_EXIT_CALLBACK
    MONITORING_TABLET_SPI_UART_ISR_ExitCallback();
#endif /* MONITORING_TABLET_SPI_UART_ISR_EXIT_CALLBACK */

}

#endif /* (MONITORING_TABLET_SCB_IRQ_INTERNAL) */


/* [] END OF FILE */
