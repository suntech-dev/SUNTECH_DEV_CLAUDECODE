/***************************************************************************//**
* \file port_OP_SPI_UART.c
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

#include "port_OP_PVT.h"
#include "port_OP_SPI_UART_PVT.h"

/***************************************
*        SPI/UART Private Vars
***************************************/

#if(port_OP_INTERNAL_RX_SW_BUFFER_CONST)
    /* Start index to put data into the software receive buffer.*/
    volatile uint32 port_OP_rxBufferHead;
    /* Start index to get data from the software receive buffer.*/
    volatile uint32 port_OP_rxBufferTail;
    /**
    * \addtogroup group_globals
    * \{
    */
    /** Sets when internal software receive buffer overflow
    *  was occurred.
    */
    volatile uint8  port_OP_rxBufferOverflow;
    /** \} globals */
#endif /* (port_OP_INTERNAL_RX_SW_BUFFER_CONST) */

#if(port_OP_INTERNAL_TX_SW_BUFFER_CONST)
    /* Start index to put data into the software transmit buffer.*/
    volatile uint32 port_OP_txBufferHead;
    /* Start index to get data from the software transmit buffer.*/
    volatile uint32 port_OP_txBufferTail;
#endif /* (port_OP_INTERNAL_TX_SW_BUFFER_CONST) */

#if(port_OP_INTERNAL_RX_SW_BUFFER)
    /* Add one element to the buffer to receive full packet. One byte in receive buffer is always empty */
    volatile uint8 port_OP_rxBufferInternal[port_OP_INTERNAL_RX_BUFFER_SIZE];
#endif /* (port_OP_INTERNAL_RX_SW_BUFFER) */

#if(port_OP_INTERNAL_TX_SW_BUFFER)
    volatile uint8 port_OP_txBufferInternal[port_OP_TX_BUFFER_SIZE];
#endif /* (port_OP_INTERNAL_TX_SW_BUFFER) */


#if(port_OP_RX_DIRECTION)
    /*******************************************************************************
    * Function Name: port_OP_SpiUartReadRxData
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
    *  port_OP_rxBufferHead - the start index to put data into the 
    *  software receive buffer.
    *  port_OP_rxBufferTail - the start index to get data from the 
    *  software receive buffer.
    *
    *******************************************************************************/
    uint32 port_OP_SpiUartReadRxData(void)
    {
        uint32 rxData = 0u;

    #if (port_OP_INTERNAL_RX_SW_BUFFER_CONST)
        uint32 locTail;
    #endif /* (port_OP_INTERNAL_RX_SW_BUFFER_CONST) */

        #if (port_OP_CHECK_RX_SW_BUFFER)
        {
            if (port_OP_rxBufferHead != port_OP_rxBufferTail)
            {
                /* There is data in RX software buffer */

                /* Calculate index to read from */
                locTail = (port_OP_rxBufferTail + 1u);

                if (port_OP_INTERNAL_RX_BUFFER_SIZE == locTail)
                {
                    locTail = 0u;
                }

                /* Get data from RX software buffer */
                rxData = port_OP_GetWordFromRxBuffer(locTail);

                /* Change index in the buffer */
                port_OP_rxBufferTail = locTail;

                #if (port_OP_CHECK_UART_RTS_CONTROL_FLOW)
                {
                    /* Check if RX Not Empty is disabled in the interrupt */
                    if (0u == (port_OP_INTR_RX_MASK_REG & port_OP_INTR_RX_NOT_EMPTY))
                    {
                        /* Enable RX Not Empty interrupt source to continue
                        * receiving data into software buffer.
                        */
                        port_OP_INTR_RX_MASK_REG |= port_OP_INTR_RX_NOT_EMPTY;
                    }
                }
                #endif

            }
        }
        #else
        {
            /* Read data from RX FIFO */
            rxData = port_OP_RX_FIFO_RD_REG;
        }
        #endif

        return (rxData);
    }


    /*******************************************************************************
    * Function Name: port_OP_SpiUartGetRxBufferSize
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
    *  port_OP_rxBufferHead - the start index to put data into the 
    *  software receive buffer.
    *  port_OP_rxBufferTail - the start index to get data from the 
    *  software receive buffer.
    *
    *******************************************************************************/
    uint32 port_OP_SpiUartGetRxBufferSize(void)
    {
        uint32 size;
    #if (port_OP_INTERNAL_RX_SW_BUFFER_CONST)
        uint32 locHead;
    #endif /* (port_OP_INTERNAL_RX_SW_BUFFER_CONST) */

        #if (port_OP_CHECK_RX_SW_BUFFER)
        {
            locHead = port_OP_rxBufferHead;

            if(locHead >= port_OP_rxBufferTail)
            {
                size = (locHead - port_OP_rxBufferTail);
            }
            else
            {
                size = (locHead + (port_OP_INTERNAL_RX_BUFFER_SIZE - port_OP_rxBufferTail));
            }
        }
        #else
        {
            size = port_OP_GET_RX_FIFO_ENTRIES;
        }
        #endif

        return (size);
    }


    /*******************************************************************************
    * Function Name: port_OP_SpiUartClearRxBuffer
    ****************************************************************************//**
    *
    *  Clears the receive buffer and RX FIFO.
    *
    * \globalvars
    *  port_OP_rxBufferHead - the start index to put data into the 
    *  software receive buffer.
    *  port_OP_rxBufferTail - the start index to get data from the 
    *  software receive buffer.
    *
    *******************************************************************************/
    void port_OP_SpiUartClearRxBuffer(void)
    {
        #if (port_OP_CHECK_RX_SW_BUFFER)
        {
            /* Lock from component interruption */
            port_OP_DisableInt();

            /* Flush RX software buffer */
            port_OP_rxBufferHead = port_OP_rxBufferTail;
            port_OP_rxBufferOverflow = 0u;

            port_OP_CLEAR_RX_FIFO;
            port_OP_ClearRxInterruptSource(port_OP_INTR_RX_ALL);

            #if (port_OP_CHECK_UART_RTS_CONTROL_FLOW)
            {
                /* Enable RX Not Empty interrupt source to continue receiving
                * data into software buffer.
                */
                port_OP_INTR_RX_MASK_REG |= port_OP_INTR_RX_NOT_EMPTY;
            }
            #endif
            
            /* Release lock */
            port_OP_EnableInt();
        }
        #else
        {
            port_OP_CLEAR_RX_FIFO;
        }
        #endif
    }

#endif /* (port_OP_RX_DIRECTION) */


#if(port_OP_TX_DIRECTION)
    /*******************************************************************************
    * Function Name: port_OP_SpiUartWriteTxData
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
    *  port_OP_txBufferHead - the start index to put data into the 
    *  software transmit buffer.
    *  port_OP_txBufferTail - start index to get data from the software
    *  transmit buffer.
    *
    *******************************************************************************/
    void port_OP_SpiUartWriteTxData(uint32 txData)
    {
    #if (port_OP_INTERNAL_TX_SW_BUFFER_CONST)
        uint32 locHead;
    #endif /* (port_OP_INTERNAL_TX_SW_BUFFER_CONST) */

        #if (port_OP_CHECK_TX_SW_BUFFER)
        {
            /* Put data directly into the TX FIFO */
            if ((port_OP_txBufferHead == port_OP_txBufferTail) &&
                (port_OP_SPI_UART_FIFO_SIZE != port_OP_GET_TX_FIFO_ENTRIES))
            {
                /* TX software buffer is empty: put data directly in TX FIFO */
                port_OP_TX_FIFO_WR_REG = txData;
            }
            /* Put data into TX software buffer */
            else
            {
                /* Head index to put data */
                locHead = (port_OP_txBufferHead + 1u);

                /* Adjust TX software buffer index */
                if (port_OP_TX_BUFFER_SIZE == locHead)
                {
                    locHead = 0u;
                }

                /* Wait for space in TX software buffer */
                while (locHead == port_OP_txBufferTail)
                {
                }

                /* TX software buffer has at least one room */

                /* Clear old status of INTR_TX_NOT_FULL. It sets at the end of transfer when TX FIFO is empty. */
                port_OP_ClearTxInterruptSource(port_OP_INTR_TX_NOT_FULL);

                port_OP_PutWordInTxBuffer(locHead, txData);

                port_OP_txBufferHead = locHead;

                /* Check if TX Not Full is disabled in interrupt */
                if (0u == (port_OP_INTR_TX_MASK_REG & port_OP_INTR_TX_NOT_FULL))
                {
                    /* Enable TX Not Full interrupt source to transmit from software buffer */
                    port_OP_INTR_TX_MASK_REG |= (uint32) port_OP_INTR_TX_NOT_FULL;
                }
            }
        }
        #else
        {
            /* Wait until TX FIFO has space to put data element */
            while (port_OP_SPI_UART_FIFO_SIZE == port_OP_GET_TX_FIFO_ENTRIES)
            {
            }

            port_OP_TX_FIFO_WR_REG = txData;
        }
        #endif
    }


    /*******************************************************************************
    * Function Name: port_OP_SpiUartPutArray
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
    *  port_OP_txBufferHead - the start index to put data into the 
    *  software transmit buffer.
    *  port_OP_txBufferTail - start index to get data from the software
    *  transmit buffer.
    *
    *******************************************************************************/
    void port_OP_SpiUartPutArray(const uint8 wrBuf[], uint32 count)
    {
        uint32 i;

        for (i=0u; i < count; i++)
        {
            port_OP_SpiUartWriteTxData((uint32) wrBuf[i]);
        }
    }


    /*******************************************************************************
    * Function Name: port_OP_SpiUartGetTxBufferSize
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
    *  port_OP_txBufferHead - the start index to put data into the 
    *  software transmit buffer.
    *  port_OP_txBufferTail - start index to get data from the software
    *  transmit buffer.
    *
    *******************************************************************************/
    uint32 port_OP_SpiUartGetTxBufferSize(void)
    {
        uint32 size;
    #if (port_OP_INTERNAL_TX_SW_BUFFER_CONST)
        uint32 locTail;
    #endif /* (port_OP_INTERNAL_TX_SW_BUFFER_CONST) */

        #if (port_OP_CHECK_TX_SW_BUFFER)
        {
            /* Get current Tail index */
            locTail = port_OP_txBufferTail;

            if (port_OP_txBufferHead >= locTail)
            {
                size = (port_OP_txBufferHead - locTail);
            }
            else
            {
                size = (port_OP_txBufferHead + (port_OP_TX_BUFFER_SIZE - locTail));
            }
        }
        #else
        {
            size = port_OP_GET_TX_FIFO_ENTRIES;
        }
        #endif

        return (size);
    }


    /*******************************************************************************
    * Function Name: port_OP_SpiUartClearTxBuffer
    ****************************************************************************//**
    *
    *  Clears the transmit buffer and TX FIFO.
    *
    * \globalvars
    *  port_OP_txBufferHead - the start index to put data into the 
    *  software transmit buffer.
    *  port_OP_txBufferTail - start index to get data from the software
    *  transmit buffer.
    *
    *******************************************************************************/
    void port_OP_SpiUartClearTxBuffer(void)
    {
        #if (port_OP_CHECK_TX_SW_BUFFER)
        {
            /* Lock from component interruption */
            port_OP_DisableInt();

            /* Flush TX software buffer */
            port_OP_txBufferHead = port_OP_txBufferTail;

            port_OP_INTR_TX_MASK_REG &= (uint32) ~port_OP_INTR_TX_NOT_FULL;
            port_OP_CLEAR_TX_FIFO;
            port_OP_ClearTxInterruptSource(port_OP_INTR_TX_ALL);

            /* Release lock */
            port_OP_EnableInt();
        }
        #else
        {
            port_OP_CLEAR_TX_FIFO;
        }
        #endif
    }

#endif /* (port_OP_TX_DIRECTION) */


/*******************************************************************************
* Function Name: port_OP_SpiUartDisableIntRx
****************************************************************************//**
*
*  Disables the RX interrupt sources.
*
*  \return
*   Returns the RX interrupt sources enabled before the function call.
*
*******************************************************************************/
uint32 port_OP_SpiUartDisableIntRx(void)
{
    uint32 intSource;

    intSource = port_OP_GetRxInterruptMode();

    port_OP_SetRxInterruptMode(port_OP_NO_INTR_SOURCES);

    return (intSource);
}


/*******************************************************************************
* Function Name: port_OP_SpiUartDisableIntTx
****************************************************************************//**
*
*  Disables TX interrupt sources.
*
*  \return
*   Returns TX interrupt sources enabled before function call.
*
*******************************************************************************/
uint32 port_OP_SpiUartDisableIntTx(void)
{
    uint32 intSourceMask;

    intSourceMask = port_OP_GetTxInterruptMode();

    port_OP_SetTxInterruptMode(port_OP_NO_INTR_SOURCES);

    return (intSourceMask);
}


#if(port_OP_SCB_MODE_UNCONFIG_CONST_CFG)
    /*******************************************************************************
    * Function Name: port_OP_PutWordInRxBuffer
    ****************************************************************************//**
    *
    *  Stores a byte/word into the RX buffer.
    *  Only available in the Unconfigured operation mode.
    *
    *  \param index:      index to store data byte/word in the RX buffer.
    *  \param rxDataByte: byte/word to store.
    *
    *******************************************************************************/
    void port_OP_PutWordInRxBuffer(uint32 idx, uint32 rxDataByte)
    {
        /* Put data in buffer */
        if (port_OP_ONE_BYTE_WIDTH == port_OP_rxDataBits)
        {
            port_OP_rxBuffer[idx] = ((uint8) rxDataByte);
        }
        else
        {
            port_OP_rxBuffer[(uint32)(idx << 1u)]      = LO8(LO16(rxDataByte));
            port_OP_rxBuffer[(uint32)(idx << 1u) + 1u] = HI8(LO16(rxDataByte));
        }
    }


    /*******************************************************************************
    * Function Name: port_OP_GetWordFromRxBuffer
    ****************************************************************************//**
    *
    *  Reads byte/word from RX buffer.
    *  Only available in the Unconfigured operation mode.
    *
    *  \return
    *   Returns byte/word read from RX buffer.
    *
    *******************************************************************************/
    uint32 port_OP_GetWordFromRxBuffer(uint32 idx)
    {
        uint32 value;

        if (port_OP_ONE_BYTE_WIDTH == port_OP_rxDataBits)
        {
            value = port_OP_rxBuffer[idx];
        }
        else
        {
            value  = (uint32) port_OP_rxBuffer[(uint32)(idx << 1u)];
            value |= (uint32) ((uint32)port_OP_rxBuffer[(uint32)(idx << 1u) + 1u] << 8u);
        }

        return (value);
    }


    /*******************************************************************************
    * Function Name: port_OP_PutWordInTxBuffer
    ****************************************************************************//**
    *
    *  Stores byte/word into the TX buffer.
    *  Only available in the Unconfigured operation mode.
    *
    *  \param idx:        index to store data byte/word in the TX buffer.
    *  \param txDataByte: byte/word to store.
    *
    *******************************************************************************/
    void port_OP_PutWordInTxBuffer(uint32 idx, uint32 txDataByte)
    {
        /* Put data in buffer */
        if (port_OP_ONE_BYTE_WIDTH == port_OP_txDataBits)
        {
            port_OP_txBuffer[idx] = ((uint8) txDataByte);
        }
        else
        {
            port_OP_txBuffer[(uint32)(idx << 1u)]      = LO8(LO16(txDataByte));
            port_OP_txBuffer[(uint32)(idx << 1u) + 1u] = HI8(LO16(txDataByte));
        }
    }


    /*******************************************************************************
    * Function Name: port_OP_GetWordFromTxBuffer
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
    uint32 port_OP_GetWordFromTxBuffer(uint32 idx)
    {
        uint32 value;

        if (port_OP_ONE_BYTE_WIDTH == port_OP_txDataBits)
        {
            value = (uint32) port_OP_txBuffer[idx];
        }
        else
        {
            value  = (uint32) port_OP_txBuffer[(uint32)(idx << 1u)];
            value |= (uint32) ((uint32) port_OP_txBuffer[(uint32)(idx << 1u) + 1u] << 8u);
        }

        return (value);
    }

#endif /* (port_OP_SCB_MODE_UNCONFIG_CONST_CFG) */


/* [] END OF FILE */
