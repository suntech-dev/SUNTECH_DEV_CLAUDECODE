/***************************************************************************//**
* \file USB_MONITORING_SPI_UART.c
* \version 4.0
*
* \brief
*  This file provides the source code to the API for the SCB Component in
*  SPI and UART modes.
*
* Note:
*
*******************************************************************************
* \copyright
* Copyright 2013-2017, Cypress Semiconductor Corporation.  All rights reserved.
* You may use this file only in accordance with the license, terms, conditions,
* disclaimers, and limitations in the end user license agreement accompanying
* the software package with which this file was provided.
*******************************************************************************/

#include "USB_MONITORING_PVT.h"
#include "USB_MONITORING_SPI_UART_PVT.h"

/***************************************
*        SPI/UART Private Vars
***************************************/

#if(USB_MONITORING_INTERNAL_RX_SW_BUFFER_CONST)
    /* Start index to put data into the software receive buffer.*/
    volatile uint32 USB_MONITORING_rxBufferHead;
    /* Start index to get data from the software receive buffer.*/
    volatile uint32 USB_MONITORING_rxBufferTail;
    /**
    * \addtogroup group_globals
    * \{
    */
    /** Sets when internal software receive buffer overflow
    *  was occurred.
    */
    volatile uint8  USB_MONITORING_rxBufferOverflow;
    /** \} globals */
#endif /* (USB_MONITORING_INTERNAL_RX_SW_BUFFER_CONST) */

#if(USB_MONITORING_INTERNAL_TX_SW_BUFFER_CONST)
    /* Start index to put data into the software transmit buffer.*/
    volatile uint32 USB_MONITORING_txBufferHead;
    /* Start index to get data from the software transmit buffer.*/
    volatile uint32 USB_MONITORING_txBufferTail;
#endif /* (USB_MONITORING_INTERNAL_TX_SW_BUFFER_CONST) */

#if(USB_MONITORING_INTERNAL_RX_SW_BUFFER)
    /* Add one element to the buffer to receive full packet. One byte in receive buffer is always empty */
    volatile uint8 USB_MONITORING_rxBufferInternal[USB_MONITORING_INTERNAL_RX_BUFFER_SIZE];
#endif /* (USB_MONITORING_INTERNAL_RX_SW_BUFFER) */

#if(USB_MONITORING_INTERNAL_TX_SW_BUFFER)
    volatile uint8 USB_MONITORING_txBufferInternal[USB_MONITORING_TX_BUFFER_SIZE];
#endif /* (USB_MONITORING_INTERNAL_TX_SW_BUFFER) */


#if(USB_MONITORING_RX_DIRECTION)
    /*******************************************************************************
    * Function Name: USB_MONITORING_SpiUartReadRxData
    ****************************************************************************//**
    *
    *  Retrieves the next data element from the receive buffer.
    *   - RX software buffer is disabled: Returns data element retrieved from
    *     RX FIFO. Undefined data will be returned if the RX FIFO is empty.
    *   - RX software buffer is enabled: Returns data element from the software
    *     receive buffer. Zero value is returned if the software receive buffer
    *     is empty.
    *
    * \return
    *  Next data element from the receive buffer. 
    *  The amount of data bits to be received depends on RX data bits selection 
    *  (the data bit counting starts from LSB of return value).
    *
    * \globalvars
    *  USB_MONITORING_rxBufferHead - the start index to put data into the 
    *  software receive buffer.
    *  USB_MONITORING_rxBufferTail - the start index to get data from the 
    *  software receive buffer.
    *
    *******************************************************************************/
    uint32 USB_MONITORING_SpiUartReadRxData(void)
    {
        uint32 rxData = 0u;

    #if (USB_MONITORING_INTERNAL_RX_SW_BUFFER_CONST)
        uint32 locTail;
    #endif /* (USB_MONITORING_INTERNAL_RX_SW_BUFFER_CONST) */

        #if (USB_MONITORING_CHECK_RX_SW_BUFFER)
        {
            if (USB_MONITORING_rxBufferHead != USB_MONITORING_rxBufferTail)
            {
                /* There is data in RX software buffer */

                /* Calculate index to read from */
                locTail = (USB_MONITORING_rxBufferTail + 1u);

                if (USB_MONITORING_INTERNAL_RX_BUFFER_SIZE == locTail)
                {
                    locTail = 0u;
                }

                /* Get data from RX software buffer */
                rxData = USB_MONITORING_GetWordFromRxBuffer(locTail);

                /* Change index in the buffer */
                USB_MONITORING_rxBufferTail = locTail;

                #if (USB_MONITORING_CHECK_UART_RTS_CONTROL_FLOW)
                {
                    /* Check if RX Not Empty is disabled in the interrupt */
                    if (0u == (USB_MONITORING_INTR_RX_MASK_REG & USB_MONITORING_INTR_RX_NOT_EMPTY))
                    {
                        /* Enable RX Not Empty interrupt source to continue
                        * receiving data into software buffer.
                        */
                        USB_MONITORING_INTR_RX_MASK_REG |= USB_MONITORING_INTR_RX_NOT_EMPTY;
                    }
                }
                #endif

            }
        }
        #else
        {
            /* Read data from RX FIFO */
            rxData = USB_MONITORING_RX_FIFO_RD_REG;
        }
        #endif

        return (rxData);
    }


    /*******************************************************************************
    * Function Name: USB_MONITORING_SpiUartGetRxBufferSize
    ****************************************************************************//**
    *
    *  Returns the number of received data elements in the receive buffer.
    *   - RX software buffer disabled: returns the number of used entries in
    *     RX FIFO.
    *   - RX software buffer enabled: returns the number of elements which were
    *     placed in the receive buffer. This does not include the hardware RX FIFO.
    *
    * \return
    *  Number of received data elements.
    *
    * \globalvars
    *  USB_MONITORING_rxBufferHead - the start index to put data into the 
    *  software receive buffer.
    *  USB_MONITORING_rxBufferTail - the start index to get data from the 
    *  software receive buffer.
    *
    *******************************************************************************/
    uint32 USB_MONITORING_SpiUartGetRxBufferSize(void)
    {
        uint32 size;
    #if (USB_MONITORING_INTERNAL_RX_SW_BUFFER_CONST)
        uint32 locHead;
    #endif /* (USB_MONITORING_INTERNAL_RX_SW_BUFFER_CONST) */

        #if (USB_MONITORING_CHECK_RX_SW_BUFFER)
        {
            locHead = USB_MONITORING_rxBufferHead;

            if(locHead >= USB_MONITORING_rxBufferTail)
            {
                size = (locHead - USB_MONITORING_rxBufferTail);
            }
            else
            {
                size = (locHead + (USB_MONITORING_INTERNAL_RX_BUFFER_SIZE - USB_MONITORING_rxBufferTail));
            }
        }
        #else
        {
            size = USB_MONITORING_GET_RX_FIFO_ENTRIES;
        }
        #endif

        return (size);
    }


    /*******************************************************************************
    * Function Name: USB_MONITORING_SpiUartClearRxBuffer
    ****************************************************************************//**
    *
    *  Clears the receive buffer and RX FIFO.
    *
    * \globalvars
    *  USB_MONITORING_rxBufferHead - the start index to put data into the 
    *  software receive buffer.
    *  USB_MONITORING_rxBufferTail - the start index to get data from the 
    *  software receive buffer.
    *
    *******************************************************************************/
    void USB_MONITORING_SpiUartClearRxBuffer(void)
    {
        #if (USB_MONITORING_CHECK_RX_SW_BUFFER)
        {
            /* Lock from component interruption */
            USB_MONITORING_DisableInt();

            /* Flush RX software buffer */
            USB_MONITORING_rxBufferHead = USB_MONITORING_rxBufferTail;
            USB_MONITORING_rxBufferOverflow = 0u;

            USB_MONITORING_CLEAR_RX_FIFO;
            USB_MONITORING_ClearRxInterruptSource(USB_MONITORING_INTR_RX_ALL);

            #if (USB_MONITORING_CHECK_UART_RTS_CONTROL_FLOW)
            {
                /* Enable RX Not Empty interrupt source to continue receiving
                * data into software buffer.
                */
                USB_MONITORING_INTR_RX_MASK_REG |= USB_MONITORING_INTR_RX_NOT_EMPTY;
            }
            #endif
            
            /* Release lock */
            USB_MONITORING_EnableInt();
        }
        #else
        {
            USB_MONITORING_CLEAR_RX_FIFO;
        }
        #endif
    }

#endif /* (USB_MONITORING_RX_DIRECTION) */


#if(USB_MONITORING_TX_DIRECTION)
    /*******************************************************************************
    * Function Name: USB_MONITORING_SpiUartWriteTxData
    ****************************************************************************//**
    *
    *  Places a data entry into the transmit buffer to be sent at the next available
    *  bus time.
    *  This function is blocking and waits until there is space available to put the
    *  requested data in the transmit buffer.
    *
    *  \param txDataByte: the data to be transmitted.
    *   The amount of data bits to be transmitted depends on TX data bits selection 
    *   (the data bit counting starts from LSB of txDataByte).
    *
    * \globalvars
    *  USB_MONITORING_txBufferHead - the start index to put data into the 
    *  software transmit buffer.
    *  USB_MONITORING_txBufferTail - start index to get data from the software
    *  transmit buffer.
    *
    *******************************************************************************/
    void USB_MONITORING_SpiUartWriteTxData(uint32 txData)
    {
    #if (USB_MONITORING_INTERNAL_TX_SW_BUFFER_CONST)
        uint32 locHead;
    #endif /* (USB_MONITORING_INTERNAL_TX_SW_BUFFER_CONST) */

        #if (USB_MONITORING_CHECK_TX_SW_BUFFER)
        {
            /* Put data directly into the TX FIFO */
            if ((USB_MONITORING_txBufferHead == USB_MONITORING_txBufferTail) &&
                (USB_MONITORING_SPI_UART_FIFO_SIZE != USB_MONITORING_GET_TX_FIFO_ENTRIES))
            {
                /* TX software buffer is empty: put data directly in TX FIFO */
                USB_MONITORING_TX_FIFO_WR_REG = txData;
            }
            /* Put data into TX software buffer */
            else
            {
                /* Head index to put data */
                locHead = (USB_MONITORING_txBufferHead + 1u);

                /* Adjust TX software buffer index */
                if (USB_MONITORING_TX_BUFFER_SIZE == locHead)
                {
                    locHead = 0u;
                }

                /* Wait for space in TX software buffer */
                while (locHead == USB_MONITORING_txBufferTail)
                {
                }

                /* TX software buffer has at least one room */

                /* Clear old status of INTR_TX_NOT_FULL. It sets at the end of transfer when TX FIFO is empty. */
                USB_MONITORING_ClearTxInterruptSource(USB_MONITORING_INTR_TX_NOT_FULL);

                USB_MONITORING_PutWordInTxBuffer(locHead, txData);

                USB_MONITORING_txBufferHead = locHead;

                /* Check if TX Not Full is disabled in interrupt */
                if (0u == (USB_MONITORING_INTR_TX_MASK_REG & USB_MONITORING_INTR_TX_NOT_FULL))
                {
                    /* Enable TX Not Full interrupt source to transmit from software buffer */
                    USB_MONITORING_INTR_TX_MASK_REG |= (uint32) USB_MONITORING_INTR_TX_NOT_FULL;
                }
            }
        }
        #else
        {
            /* Wait until TX FIFO has space to put data element */
            while (USB_MONITORING_SPI_UART_FIFO_SIZE == USB_MONITORING_GET_TX_FIFO_ENTRIES)
            {
            }

            USB_MONITORING_TX_FIFO_WR_REG = txData;
        }
        #endif
    }


    /*******************************************************************************
    * Function Name: USB_MONITORING_SpiUartPutArray
    ****************************************************************************//**
    *
    *  Places an array of data into the transmit buffer to be sent.
    *  This function is blocking and waits until there is a space available to put
    *  all the requested data in the transmit buffer. The array size can be greater
    *  than transmit buffer size.
    *
    * \param wrBuf: pointer to an array of data to be placed in transmit buffer. 
    *  The width of the data to be transmitted depends on TX data width selection 
    *  (the data bit counting starts from LSB for each array element).
    * \param count: number of data elements to be placed in the transmit buffer.
    *
    * \globalvars
    *  USB_MONITORING_txBufferHead - the start index to put data into the 
    *  software transmit buffer.
    *  USB_MONITORING_txBufferTail - start index to get data from the software
    *  transmit buffer.
    *
    *******************************************************************************/
    void USB_MONITORING_SpiUartPutArray(const uint8 wrBuf[], uint32 count)
    {
        uint32 i;

        for (i=0u; i < count; i++)
        {
            USB_MONITORING_SpiUartWriteTxData((uint32) wrBuf[i]);
        }
    }


    /*******************************************************************************
    * Function Name: USB_MONITORING_SpiUartGetTxBufferSize
    ****************************************************************************//**
    *
    *  Returns the number of elements currently in the transmit buffer.
    *   - TX software buffer is disabled: returns the number of used entries in
    *     TX FIFO.
    *   - TX software buffer is enabled: returns the number of elements currently
    *     used in the transmit buffer. This number does not include used entries in
    *     the TX FIFO. The transmit buffer size is zero until the TX FIFO is
    *     not full.
    *
    * \return
    *  Number of data elements ready to transmit.
    *
    * \globalvars
    *  USB_MONITORING_txBufferHead - the start index to put data into the 
    *  software transmit buffer.
    *  USB_MONITORING_txBufferTail - start index to get data from the software
    *  transmit buffer.
    *
    *******************************************************************************/
    uint32 USB_MONITORING_SpiUartGetTxBufferSize(void)
    {
        uint32 size;
    #if (USB_MONITORING_INTERNAL_TX_SW_BUFFER_CONST)
        uint32 locTail;
    #endif /* (USB_MONITORING_INTERNAL_TX_SW_BUFFER_CONST) */

        #if (USB_MONITORING_CHECK_TX_SW_BUFFER)
        {
            /* Get current Tail index */
            locTail = USB_MONITORING_txBufferTail;

            if (USB_MONITORING_txBufferHead >= locTail)
            {
                size = (USB_MONITORING_txBufferHead - locTail);
            }
            else
            {
                size = (USB_MONITORING_txBufferHead + (USB_MONITORING_TX_BUFFER_SIZE - locTail));
            }
        }
        #else
        {
            size = USB_MONITORING_GET_TX_FIFO_ENTRIES;
        }
        #endif

        return (size);
    }


    /*******************************************************************************
    * Function Name: USB_MONITORING_SpiUartClearTxBuffer
    ****************************************************************************//**
    *
    *  Clears the transmit buffer and TX FIFO.
    *
    * \globalvars
    *  USB_MONITORING_txBufferHead - the start index to put data into the 
    *  software transmit buffer.
    *  USB_MONITORING_txBufferTail - start index to get data from the software
    *  transmit buffer.
    *
    *******************************************************************************/
    void USB_MONITORING_SpiUartClearTxBuffer(void)
    {
        #if (USB_MONITORING_CHECK_TX_SW_BUFFER)
        {
            /* Lock from component interruption */
            USB_MONITORING_DisableInt();

            /* Flush TX software buffer */
            USB_MONITORING_txBufferHead = USB_MONITORING_txBufferTail;

            USB_MONITORING_INTR_TX_MASK_REG &= (uint32) ~USB_MONITORING_INTR_TX_NOT_FULL;
            USB_MONITORING_CLEAR_TX_FIFO;
            USB_MONITORING_ClearTxInterruptSource(USB_MONITORING_INTR_TX_ALL);

            /* Release lock */
            USB_MONITORING_EnableInt();
        }
        #else
        {
            USB_MONITORING_CLEAR_TX_FIFO;
        }
        #endif
    }

#endif /* (USB_MONITORING_TX_DIRECTION) */


/*******************************************************************************
* Function Name: USB_MONITORING_SpiUartDisableIntRx
****************************************************************************//**
*
*  Disables the RX interrupt sources.
*
*  \return
*   Returns the RX interrupt sources enabled before the function call.
*
*******************************************************************************/
uint32 USB_MONITORING_SpiUartDisableIntRx(void)
{
    uint32 intSource;

    intSource = USB_MONITORING_GetRxInterruptMode();

    USB_MONITORING_SetRxInterruptMode(USB_MONITORING_NO_INTR_SOURCES);

    return (intSource);
}


/*******************************************************************************
* Function Name: USB_MONITORING_SpiUartDisableIntTx
****************************************************************************//**
*
*  Disables TX interrupt sources.
*
*  \return
*   Returns TX interrupt sources enabled before function call.
*
*******************************************************************************/
uint32 USB_MONITORING_SpiUartDisableIntTx(void)
{
    uint32 intSourceMask;

    intSourceMask = USB_MONITORING_GetTxInterruptMode();

    USB_MONITORING_SetTxInterruptMode(USB_MONITORING_NO_INTR_SOURCES);

    return (intSourceMask);
}


#if(USB_MONITORING_SCB_MODE_UNCONFIG_CONST_CFG)
    /*******************************************************************************
    * Function Name: USB_MONITORING_PutWordInRxBuffer
    ****************************************************************************//**
    *
    *  Stores a byte/word into the RX buffer.
    *  Only available in the Unconfigured operation mode.
    *
    *  \param index:      index to store data byte/word in the RX buffer.
    *  \param rxDataByte: byte/word to store.
    *
    *******************************************************************************/
    void USB_MONITORING_PutWordInRxBuffer(uint32 idx, uint32 rxDataByte)
    {
        /* Put data in buffer */
        if (USB_MONITORING_ONE_BYTE_WIDTH == USB_MONITORING_rxDataBits)
        {
            USB_MONITORING_rxBuffer[idx] = ((uint8) rxDataByte);
        }
        else
        {
            USB_MONITORING_rxBuffer[(uint32)(idx << 1u)]      = LO8(LO16(rxDataByte));
            USB_MONITORING_rxBuffer[(uint32)(idx << 1u) + 1u] = HI8(LO16(rxDataByte));
        }
    }


    /*******************************************************************************
    * Function Name: USB_MONITORING_GetWordFromRxBuffer
    ****************************************************************************//**
    *
    *  Reads byte/word from RX buffer.
    *  Only available in the Unconfigured operation mode.
    *
    *  \return
    *   Returns byte/word read from RX buffer.
    *
    *******************************************************************************/
    uint32 USB_MONITORING_GetWordFromRxBuffer(uint32 idx)
    {
        uint32 value;

        if (USB_MONITORING_ONE_BYTE_WIDTH == USB_MONITORING_rxDataBits)
        {
            value = USB_MONITORING_rxBuffer[idx];
        }
        else
        {
            value  = (uint32) USB_MONITORING_rxBuffer[(uint32)(idx << 1u)];
            value |= (uint32) ((uint32)USB_MONITORING_rxBuffer[(uint32)(idx << 1u) + 1u] << 8u);
        }

        return (value);
    }


    /*******************************************************************************
    * Function Name: USB_MONITORING_PutWordInTxBuffer
    ****************************************************************************//**
    *
    *  Stores byte/word into the TX buffer.
    *  Only available in the Unconfigured operation mode.
    *
    *  \param idx:        index to store data byte/word in the TX buffer.
    *  \param txDataByte: byte/word to store.
    *
    *******************************************************************************/
    void USB_MONITORING_PutWordInTxBuffer(uint32 idx, uint32 txDataByte)
    {
        /* Put data in buffer */
        if (USB_MONITORING_ONE_BYTE_WIDTH == USB_MONITORING_txDataBits)
        {
            USB_MONITORING_txBuffer[idx] = ((uint8) txDataByte);
        }
        else
        {
            USB_MONITORING_txBuffer[(uint32)(idx << 1u)]      = LO8(LO16(txDataByte));
            USB_MONITORING_txBuffer[(uint32)(idx << 1u) + 1u] = HI8(LO16(txDataByte));
        }
    }


    /*******************************************************************************
    * Function Name: USB_MONITORING_GetWordFromTxBuffer
    ****************************************************************************//**
    *
    *  Reads byte/word from the TX buffer.
    *  Only available in the Unconfigured operation mode.
    *
    *  \param idx: index to get data byte/word from the TX buffer.
    *
    *  \return
    *   Returns byte/word read from the TX buffer.
    *
    *******************************************************************************/
    uint32 USB_MONITORING_GetWordFromTxBuffer(uint32 idx)
    {
        uint32 value;

        if (USB_MONITORING_ONE_BYTE_WIDTH == USB_MONITORING_txDataBits)
        {
            value = (uint32) USB_MONITORING_txBuffer[idx];
        }
        else
        {
            value  = (uint32) USB_MONITORING_txBuffer[(uint32)(idx << 1u)];
            value |= (uint32) ((uint32) USB_MONITORING_txBuffer[(uint32)(idx << 1u) + 1u] << 8u);
        }

        return (value);
    }

#endif /* (USB_MONITORING_SCB_MODE_UNCONFIG_CONST_CFG) */


/* [] END OF FILE */
