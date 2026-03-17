/***************************************************************************//**
* \file OP_ST500_SPI_UART.c
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

#include "OP_ST500_PVT.h"
#include "OP_ST500_SPI_UART_PVT.h"

/***************************************
*        SPI/UART Private Vars
***************************************/

#if(OP_ST500_INTERNAL_RX_SW_BUFFER_CONST)
    /* Start index to put data into the software receive buffer.*/
    volatile uint32 OP_ST500_rxBufferHead;
    /* Start index to get data from the software receive buffer.*/
    volatile uint32 OP_ST500_rxBufferTail;
    /**
    * \addtogroup group_globals
    * \{
    */
    /** Sets when internal software receive buffer overflow
    *  was occurred.
    */
    volatile uint8  OP_ST500_rxBufferOverflow;
    /** \} globals */
#endif /* (OP_ST500_INTERNAL_RX_SW_BUFFER_CONST) */

#if(OP_ST500_INTERNAL_TX_SW_BUFFER_CONST)
    /* Start index to put data into the software transmit buffer.*/
    volatile uint32 OP_ST500_txBufferHead;
    /* Start index to get data from the software transmit buffer.*/
    volatile uint32 OP_ST500_txBufferTail;
#endif /* (OP_ST500_INTERNAL_TX_SW_BUFFER_CONST) */

#if(OP_ST500_INTERNAL_RX_SW_BUFFER)
    /* Add one element to the buffer to receive full packet. One byte in receive buffer is always empty */
    volatile uint8 OP_ST500_rxBufferInternal[OP_ST500_INTERNAL_RX_BUFFER_SIZE];
#endif /* (OP_ST500_INTERNAL_RX_SW_BUFFER) */

#if(OP_ST500_INTERNAL_TX_SW_BUFFER)
    volatile uint8 OP_ST500_txBufferInternal[OP_ST500_TX_BUFFER_SIZE];
#endif /* (OP_ST500_INTERNAL_TX_SW_BUFFER) */


#if(OP_ST500_RX_DIRECTION)
    /*******************************************************************************
    * Function Name: OP_ST500_SpiUartReadRxData
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
    *  OP_ST500_rxBufferHead - the start index to put data into the 
    *  software receive buffer.
    *  OP_ST500_rxBufferTail - the start index to get data from the 
    *  software receive buffer.
    *
    *******************************************************************************/
    uint32 OP_ST500_SpiUartReadRxData(void)
    {
        uint32 rxData = 0u;

    #if (OP_ST500_INTERNAL_RX_SW_BUFFER_CONST)
        uint32 locTail;
    #endif /* (OP_ST500_INTERNAL_RX_SW_BUFFER_CONST) */

        #if (OP_ST500_CHECK_RX_SW_BUFFER)
        {
            if (OP_ST500_rxBufferHead != OP_ST500_rxBufferTail)
            {
                /* There is data in RX software buffer */

                /* Calculate index to read from */
                locTail = (OP_ST500_rxBufferTail + 1u);

                if (OP_ST500_INTERNAL_RX_BUFFER_SIZE == locTail)
                {
                    locTail = 0u;
                }

                /* Get data from RX software buffer */
                rxData = OP_ST500_GetWordFromRxBuffer(locTail);

                /* Change index in the buffer */
                OP_ST500_rxBufferTail = locTail;

                #if (OP_ST500_CHECK_UART_RTS_CONTROL_FLOW)
                {
                    /* Check if RX Not Empty is disabled in the interrupt */
                    if (0u == (OP_ST500_INTR_RX_MASK_REG & OP_ST500_INTR_RX_NOT_EMPTY))
                    {
                        /* Enable RX Not Empty interrupt source to continue
                        * receiving data into software buffer.
                        */
                        OP_ST500_INTR_RX_MASK_REG |= OP_ST500_INTR_RX_NOT_EMPTY;
                    }
                }
                #endif

            }
        }
        #else
        {
            /* Read data from RX FIFO */
            rxData = OP_ST500_RX_FIFO_RD_REG;
        }
        #endif

        return (rxData);
    }


    /*******************************************************************************
    * Function Name: OP_ST500_SpiUartGetRxBufferSize
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
    *  OP_ST500_rxBufferHead - the start index to put data into the 
    *  software receive buffer.
    *  OP_ST500_rxBufferTail - the start index to get data from the 
    *  software receive buffer.
    *
    *******************************************************************************/
    uint32 OP_ST500_SpiUartGetRxBufferSize(void)
    {
        uint32 size;
    #if (OP_ST500_INTERNAL_RX_SW_BUFFER_CONST)
        uint32 locHead;
    #endif /* (OP_ST500_INTERNAL_RX_SW_BUFFER_CONST) */

        #if (OP_ST500_CHECK_RX_SW_BUFFER)
        {
            locHead = OP_ST500_rxBufferHead;

            if(locHead >= OP_ST500_rxBufferTail)
            {
                size = (locHead - OP_ST500_rxBufferTail);
            }
            else
            {
                size = (locHead + (OP_ST500_INTERNAL_RX_BUFFER_SIZE - OP_ST500_rxBufferTail));
            }
        }
        #else
        {
            size = OP_ST500_GET_RX_FIFO_ENTRIES;
        }
        #endif

        return (size);
    }


    /*******************************************************************************
    * Function Name: OP_ST500_SpiUartClearRxBuffer
    ****************************************************************************//**
    *
    *  Clears the receive buffer and RX FIFO.
    *
    * \globalvars
    *  OP_ST500_rxBufferHead - the start index to put data into the 
    *  software receive buffer.
    *  OP_ST500_rxBufferTail - the start index to get data from the 
    *  software receive buffer.
    *
    *******************************************************************************/
    void OP_ST500_SpiUartClearRxBuffer(void)
    {
        #if (OP_ST500_CHECK_RX_SW_BUFFER)
        {
            /* Lock from component interruption */
            OP_ST500_DisableInt();

            /* Flush RX software buffer */
            OP_ST500_rxBufferHead = OP_ST500_rxBufferTail;
            OP_ST500_rxBufferOverflow = 0u;

            OP_ST500_CLEAR_RX_FIFO;
            OP_ST500_ClearRxInterruptSource(OP_ST500_INTR_RX_ALL);

            #if (OP_ST500_CHECK_UART_RTS_CONTROL_FLOW)
            {
                /* Enable RX Not Empty interrupt source to continue receiving
                * data into software buffer.
                */
                OP_ST500_INTR_RX_MASK_REG |= OP_ST500_INTR_RX_NOT_EMPTY;
            }
            #endif
            
            /* Release lock */
            OP_ST500_EnableInt();
        }
        #else
        {
            OP_ST500_CLEAR_RX_FIFO;
        }
        #endif
    }

#endif /* (OP_ST500_RX_DIRECTION) */


#if(OP_ST500_TX_DIRECTION)
    /*******************************************************************************
    * Function Name: OP_ST500_SpiUartWriteTxData
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
    *  OP_ST500_txBufferHead - the start index to put data into the 
    *  software transmit buffer.
    *  OP_ST500_txBufferTail - start index to get data from the software
    *  transmit buffer.
    *
    *******************************************************************************/
    void OP_ST500_SpiUartWriteTxData(uint32 txData)
    {
    #if (OP_ST500_INTERNAL_TX_SW_BUFFER_CONST)
        uint32 locHead;
    #endif /* (OP_ST500_INTERNAL_TX_SW_BUFFER_CONST) */

        #if (OP_ST500_CHECK_TX_SW_BUFFER)
        {
            /* Put data directly into the TX FIFO */
            if ((OP_ST500_txBufferHead == OP_ST500_txBufferTail) &&
                (OP_ST500_SPI_UART_FIFO_SIZE != OP_ST500_GET_TX_FIFO_ENTRIES))
            {
                /* TX software buffer is empty: put data directly in TX FIFO */
                OP_ST500_TX_FIFO_WR_REG = txData;
            }
            /* Put data into TX software buffer */
            else
            {
                /* Head index to put data */
                locHead = (OP_ST500_txBufferHead + 1u);

                /* Adjust TX software buffer index */
                if (OP_ST500_TX_BUFFER_SIZE == locHead)
                {
                    locHead = 0u;
                }

                /* Wait for space in TX software buffer */
                while (locHead == OP_ST500_txBufferTail)
                {
                }

                /* TX software buffer has at least one room */

                /* Clear old status of INTR_TX_NOT_FULL. It sets at the end of transfer when TX FIFO is empty. */
                OP_ST500_ClearTxInterruptSource(OP_ST500_INTR_TX_NOT_FULL);

                OP_ST500_PutWordInTxBuffer(locHead, txData);

                OP_ST500_txBufferHead = locHead;

                /* Check if TX Not Full is disabled in interrupt */
                if (0u == (OP_ST500_INTR_TX_MASK_REG & OP_ST500_INTR_TX_NOT_FULL))
                {
                    /* Enable TX Not Full interrupt source to transmit from software buffer */
                    OP_ST500_INTR_TX_MASK_REG |= (uint32) OP_ST500_INTR_TX_NOT_FULL;
                }
            }
        }
        #else
        {
            /* Wait until TX FIFO has space to put data element */
            while (OP_ST500_SPI_UART_FIFO_SIZE == OP_ST500_GET_TX_FIFO_ENTRIES)
            {
            }

            OP_ST500_TX_FIFO_WR_REG = txData;
        }
        #endif
    }


    /*******************************************************************************
    * Function Name: OP_ST500_SpiUartPutArray
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
    *  OP_ST500_txBufferHead - the start index to put data into the 
    *  software transmit buffer.
    *  OP_ST500_txBufferTail - start index to get data from the software
    *  transmit buffer.
    *
    *******************************************************************************/
    void OP_ST500_SpiUartPutArray(const uint8 wrBuf[], uint32 count)
    {
        uint32 i;

        for (i=0u; i < count; i++)
        {
            OP_ST500_SpiUartWriteTxData((uint32) wrBuf[i]);
        }
    }


    /*******************************************************************************
    * Function Name: OP_ST500_SpiUartGetTxBufferSize
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
    *  OP_ST500_txBufferHead - the start index to put data into the 
    *  software transmit buffer.
    *  OP_ST500_txBufferTail - start index to get data from the software
    *  transmit buffer.
    *
    *******************************************************************************/
    uint32 OP_ST500_SpiUartGetTxBufferSize(void)
    {
        uint32 size;
    #if (OP_ST500_INTERNAL_TX_SW_BUFFER_CONST)
        uint32 locTail;
    #endif /* (OP_ST500_INTERNAL_TX_SW_BUFFER_CONST) */

        #if (OP_ST500_CHECK_TX_SW_BUFFER)
        {
            /* Get current Tail index */
            locTail = OP_ST500_txBufferTail;

            if (OP_ST500_txBufferHead >= locTail)
            {
                size = (OP_ST500_txBufferHead - locTail);
            }
            else
            {
                size = (OP_ST500_txBufferHead + (OP_ST500_TX_BUFFER_SIZE - locTail));
            }
        }
        #else
        {
            size = OP_ST500_GET_TX_FIFO_ENTRIES;
        }
        #endif

        return (size);
    }


    /*******************************************************************************
    * Function Name: OP_ST500_SpiUartClearTxBuffer
    ****************************************************************************//**
    *
    *  Clears the transmit buffer and TX FIFO.
    *
    * \globalvars
    *  OP_ST500_txBufferHead - the start index to put data into the 
    *  software transmit buffer.
    *  OP_ST500_txBufferTail - start index to get data from the software
    *  transmit buffer.
    *
    *******************************************************************************/
    void OP_ST500_SpiUartClearTxBuffer(void)
    {
        #if (OP_ST500_CHECK_TX_SW_BUFFER)
        {
            /* Lock from component interruption */
            OP_ST500_DisableInt();

            /* Flush TX software buffer */
            OP_ST500_txBufferHead = OP_ST500_txBufferTail;

            OP_ST500_INTR_TX_MASK_REG &= (uint32) ~OP_ST500_INTR_TX_NOT_FULL;
            OP_ST500_CLEAR_TX_FIFO;
            OP_ST500_ClearTxInterruptSource(OP_ST500_INTR_TX_ALL);

            /* Release lock */
            OP_ST500_EnableInt();
        }
        #else
        {
            OP_ST500_CLEAR_TX_FIFO;
        }
        #endif
    }

#endif /* (OP_ST500_TX_DIRECTION) */


/*******************************************************************************
* Function Name: OP_ST500_SpiUartDisableIntRx
****************************************************************************//**
*
*  Disables the RX interrupt sources.
*
*  \return
*   Returns the RX interrupt sources enabled before the function call.
*
*******************************************************************************/
uint32 OP_ST500_SpiUartDisableIntRx(void)
{
    uint32 intSource;

    intSource = OP_ST500_GetRxInterruptMode();

    OP_ST500_SetRxInterruptMode(OP_ST500_NO_INTR_SOURCES);

    return (intSource);
}


/*******************************************************************************
* Function Name: OP_ST500_SpiUartDisableIntTx
****************************************************************************//**
*
*  Disables TX interrupt sources.
*
*  \return
*   Returns TX interrupt sources enabled before function call.
*
*******************************************************************************/
uint32 OP_ST500_SpiUartDisableIntTx(void)
{
    uint32 intSourceMask;

    intSourceMask = OP_ST500_GetTxInterruptMode();

    OP_ST500_SetTxInterruptMode(OP_ST500_NO_INTR_SOURCES);

    return (intSourceMask);
}


#if(OP_ST500_SCB_MODE_UNCONFIG_CONST_CFG)
    /*******************************************************************************
    * Function Name: OP_ST500_PutWordInRxBuffer
    ****************************************************************************//**
    *
    *  Stores a byte/word into the RX buffer.
    *  Only available in the Unconfigured operation mode.
    *
    *  \param index:      index to store data byte/word in the RX buffer.
    *  \param rxDataByte: byte/word to store.
    *
    *******************************************************************************/
    void OP_ST500_PutWordInRxBuffer(uint32 idx, uint32 rxDataByte)
    {
        /* Put data in buffer */
        if (OP_ST500_ONE_BYTE_WIDTH == OP_ST500_rxDataBits)
        {
            OP_ST500_rxBuffer[idx] = ((uint8) rxDataByte);
        }
        else
        {
            OP_ST500_rxBuffer[(uint32)(idx << 1u)]      = LO8(LO16(rxDataByte));
            OP_ST500_rxBuffer[(uint32)(idx << 1u) + 1u] = HI8(LO16(rxDataByte));
        }
    }


    /*******************************************************************************
    * Function Name: OP_ST500_GetWordFromRxBuffer
    ****************************************************************************//**
    *
    *  Reads byte/word from RX buffer.
    *  Only available in the Unconfigured operation mode.
    *
    *  \return
    *   Returns byte/word read from RX buffer.
    *
    *******************************************************************************/
    uint32 OP_ST500_GetWordFromRxBuffer(uint32 idx)
    {
        uint32 value;

        if (OP_ST500_ONE_BYTE_WIDTH == OP_ST500_rxDataBits)
        {
            value = OP_ST500_rxBuffer[idx];
        }
        else
        {
            value  = (uint32) OP_ST500_rxBuffer[(uint32)(idx << 1u)];
            value |= (uint32) ((uint32)OP_ST500_rxBuffer[(uint32)(idx << 1u) + 1u] << 8u);
        }

        return (value);
    }


    /*******************************************************************************
    * Function Name: OP_ST500_PutWordInTxBuffer
    ****************************************************************************//**
    *
    *  Stores byte/word into the TX buffer.
    *  Only available in the Unconfigured operation mode.
    *
    *  \param idx:        index to store data byte/word in the TX buffer.
    *  \param txDataByte: byte/word to store.
    *
    *******************************************************************************/
    void OP_ST500_PutWordInTxBuffer(uint32 idx, uint32 txDataByte)
    {
        /* Put data in buffer */
        if (OP_ST500_ONE_BYTE_WIDTH == OP_ST500_txDataBits)
        {
            OP_ST500_txBuffer[idx] = ((uint8) txDataByte);
        }
        else
        {
            OP_ST500_txBuffer[(uint32)(idx << 1u)]      = LO8(LO16(txDataByte));
            OP_ST500_txBuffer[(uint32)(idx << 1u) + 1u] = HI8(LO16(txDataByte));
        }
    }


    /*******************************************************************************
    * Function Name: OP_ST500_GetWordFromTxBuffer
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
    uint32 OP_ST500_GetWordFromTxBuffer(uint32 idx)
    {
        uint32 value;

        if (OP_ST500_ONE_BYTE_WIDTH == OP_ST500_txDataBits)
        {
            value = (uint32) OP_ST500_txBuffer[idx];
        }
        else
        {
            value  = (uint32) OP_ST500_txBuffer[(uint32)(idx << 1u)];
            value |= (uint32) ((uint32) OP_ST500_txBuffer[(uint32)(idx << 1u) + 1u] << 8u);
        }

        return (value);
    }

#endif /* (OP_ST500_SCB_MODE_UNCONFIG_CONST_CFG) */


/* [] END OF FILE */
