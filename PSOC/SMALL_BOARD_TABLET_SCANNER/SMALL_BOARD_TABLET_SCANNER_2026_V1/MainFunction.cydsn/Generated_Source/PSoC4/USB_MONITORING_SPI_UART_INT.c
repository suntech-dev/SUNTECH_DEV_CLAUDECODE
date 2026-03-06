/***************************************************************************//**
* \file USB_MONITORING_SPI_UART_INT.c
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

#include "USB_MONITORING_PVT.h"
#include "USB_MONITORING_SPI_UART_PVT.h"
#include "cyapicallbacks.h"

#if (USB_MONITORING_SCB_IRQ_INTERNAL)
/*******************************************************************************
* Function Name: USB_MONITORING_SPI_UART_ISR
****************************************************************************//**
*
*  Handles the Interrupt Service Routine for the SCB SPI or UART modes.
*
*******************************************************************************/
CY_ISR(USB_MONITORING_SPI_UART_ISR)
{
#if (USB_MONITORING_INTERNAL_RX_SW_BUFFER_CONST)
    uint32 locHead;
#endif /* (USB_MONITORING_INTERNAL_RX_SW_BUFFER_CONST) */

#if (USB_MONITORING_INTERNAL_TX_SW_BUFFER_CONST)
    uint32 locTail;
#endif /* (USB_MONITORING_INTERNAL_TX_SW_BUFFER_CONST) */

#ifdef USB_MONITORING_SPI_UART_ISR_ENTRY_CALLBACK
    USB_MONITORING_SPI_UART_ISR_EntryCallback();
#endif /* USB_MONITORING_SPI_UART_ISR_ENTRY_CALLBACK */

    if (NULL != USB_MONITORING_customIntrHandler)
    {
        USB_MONITORING_customIntrHandler();
    }

    #if(USB_MONITORING_CHECK_SPI_WAKE_ENABLE)
    {
        /* Clear SPI wakeup source */
        USB_MONITORING_ClearSpiExtClkInterruptSource(USB_MONITORING_INTR_SPI_EC_WAKE_UP);
    }
    #endif

    #if (USB_MONITORING_CHECK_RX_SW_BUFFER)
    {
        if (USB_MONITORING_CHECK_INTR_RX_MASKED(USB_MONITORING_INTR_RX_NOT_EMPTY))
        {
            do
            {
                /* Move local head index */
                locHead = (USB_MONITORING_rxBufferHead + 1u);

                /* Adjust local head index */
                if (USB_MONITORING_INTERNAL_RX_BUFFER_SIZE == locHead)
                {
                    locHead = 0u;
                }

                if (locHead == USB_MONITORING_rxBufferTail)
                {
                    #if (USB_MONITORING_CHECK_UART_RTS_CONTROL_FLOW)
                    {
                        /* There is no space in the software buffer - disable the
                        * RX Not Empty interrupt source. The data elements are
                        * still being received into the RX FIFO until the RTS signal
                        * stops the transmitter. After the data element is read from the
                        * buffer, the RX Not Empty interrupt source is enabled to
                        * move the next data element in the software buffer.
                        */
                        USB_MONITORING_INTR_RX_MASK_REG &= ~USB_MONITORING_INTR_RX_NOT_EMPTY;
                        break;
                    }
                    #else
                    {
                        /* Overflow: through away received data element */
                        (void) USB_MONITORING_RX_FIFO_RD_REG;
                        USB_MONITORING_rxBufferOverflow = (uint8) USB_MONITORING_INTR_RX_OVERFLOW;
                    }
                    #endif
                }
                else
                {
                    /* Store received data */
                    USB_MONITORING_PutWordInRxBuffer(locHead, USB_MONITORING_RX_FIFO_RD_REG);

                    /* Move head index */
                    USB_MONITORING_rxBufferHead = locHead;
                }
            }
            while(0u != USB_MONITORING_GET_RX_FIFO_ENTRIES);

            USB_MONITORING_ClearRxInterruptSource(USB_MONITORING_INTR_RX_NOT_EMPTY);
        }
    }
    #endif


    #if (USB_MONITORING_CHECK_TX_SW_BUFFER)
    {
        if (USB_MONITORING_CHECK_INTR_TX_MASKED(USB_MONITORING_INTR_TX_NOT_FULL))
        {
            do
            {
                /* Check for room in TX software buffer */
                if (USB_MONITORING_txBufferHead != USB_MONITORING_txBufferTail)
                {
                    /* Move local tail index */
                    locTail = (USB_MONITORING_txBufferTail + 1u);

                    /* Adjust local tail index */
                    if (USB_MONITORING_TX_BUFFER_SIZE == locTail)
                    {
                        locTail = 0u;
                    }

                    /* Put data into TX FIFO */
                    USB_MONITORING_TX_FIFO_WR_REG = USB_MONITORING_GetWordFromTxBuffer(locTail);

                    /* Move tail index */
                    USB_MONITORING_txBufferTail = locTail;
                }
                else
                {
                    /* TX software buffer is empty: complete transfer */
                    USB_MONITORING_DISABLE_INTR_TX(USB_MONITORING_INTR_TX_NOT_FULL);
                    break;
                }
            }
            while (USB_MONITORING_SPI_UART_FIFO_SIZE != USB_MONITORING_GET_TX_FIFO_ENTRIES);

            USB_MONITORING_ClearTxInterruptSource(USB_MONITORING_INTR_TX_NOT_FULL);
        }
    }
    #endif

#ifdef USB_MONITORING_SPI_UART_ISR_EXIT_CALLBACK
    USB_MONITORING_SPI_UART_ISR_ExitCallback();
#endif /* USB_MONITORING_SPI_UART_ISR_EXIT_CALLBACK */

}

#endif /* (USB_MONITORING_SCB_IRQ_INTERNAL) */


/* [] END OF FILE */
