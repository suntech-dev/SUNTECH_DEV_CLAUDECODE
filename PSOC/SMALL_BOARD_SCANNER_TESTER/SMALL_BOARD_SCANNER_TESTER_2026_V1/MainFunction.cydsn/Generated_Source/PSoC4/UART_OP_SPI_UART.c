/***************************************************************************//**
* \file UART_OP_SPI_UART.c
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

#include "UART_OP_PVT.h"
#include "UART_OP_SPI_UART_PVT.h"

/***************************************
*        SPI/UART Private Vars
***************************************/

#if(UART_OP_INTERNAL_RX_SW_BUFFER_CONST)
    /* Start index to put data into the software receive buffer.*/
    volatile uint32 UART_OP_rxBufferHead;
    /* Start index to get data from the software receive buffer.*/
    volatile uint32 UART_OP_rxBufferTail;
    /**
    * \addtogroup group_globals
    * \{
    */
    /** Sets when internal software receive buffer overflow
    *  was occurred.
    */
    volatile uint8  UART_OP_rxBufferOverflow;
    /** \} globals */
#endif /* (UART_OP_INTERNAL_RX_SW_BUFFER_CONST) */

#if(UART_OP_INTERNAL_TX_SW_BUFFER_CONST)
    /* Start index to put data into the software transmit buffer.*/
    volatile uint32 UART_OP_txBufferHead;
    /* Start index to get data from the software transmit buffer.*/
    volatile uint32 UART_OP_txBufferTail;
#endif /* (UART_OP_INTERNAL_TX_SW_BUFFER_CONST) */

#if(UART_OP_INTERNAL_RX_SW_BUFFER)
    /* Add one element to the buffer to receive full packet. One byte in receive buffer is always empty */
    volatile uint8 UART_OP_rxBufferInternal[UART_OP_INTERNAL_RX_BUFFER_SIZE];
#endif /* (UART_OP_INTERNAL_RX_SW_BUFFER) */

#if(UART_OP_INTERNAL_TX_SW_BUFFER)
    volatile uint8 UART_OP_txBufferInternal[UART_OP_TX_BUFFER_SIZE];
#endif /* (UART_OP_INTERNAL_TX_SW_BUFFER) */


#if(UART_OP_RX_DIRECTION)
    /*******************************************************************************
    * Function Name: UART_OP_SpiUartReadRxData
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
    *  UART_OP_rxBufferHead - the start index to put data into the 
    *  software receive buffer.
    *  UART_OP_rxBufferTail - the start index to get data from the 
    *  software receive buffer.
    *
    *******************************************************************************/
    uint32 UART_OP_SpiUartReadRxData(void)
    {
        uint32 rxData = 0u;

    #if (UART_OP_INTERNAL_RX_SW_BUFFER_CONST)
        uint32 locTail;
    #endif /* (UART_OP_INTERNAL_RX_SW_BUFFER_CONST) */

        #if (UART_OP_CHECK_RX_SW_BUFFER)
        {
            if (UART_OP_rxBufferHead != UART_OP_rxBufferTail)
            {
                /* There is data in RX software buffer */

                /* Calculate index to read from */
                locTail = (UART_OP_rxBufferTail + 1u);

                if (UART_OP_INTERNAL_RX_BUFFER_SIZE == locTail)
                {
                    locTail = 0u;
                }

                /* Get data from RX software buffer */
                rxData = UART_OP_GetWordFromRxBuffer(locTail);

                /* Change index in the buffer */
                UART_OP_rxBufferTail = locTail;

                #if (UART_OP_CHECK_UART_RTS_CONTROL_FLOW)
                {
                    /* Check if RX Not Empty is disabled in the interrupt */
                    if (0u == (UART_OP_INTR_RX_MASK_REG & UART_OP_INTR_RX_NOT_EMPTY))
                    {
                        /* Enable RX Not Empty interrupt source to continue
                        * receiving data into software buffer.
                        */
                        UART_OP_INTR_RX_MASK_REG |= UART_OP_INTR_RX_NOT_EMPTY;
                    }
                }
                #endif

            }
        }
        #else
        {
            /* Read data from RX FIFO */
            rxData = UART_OP_RX_FIFO_RD_REG;
        }
        #endif

        return (rxData);
    }


    /*******************************************************************************
    * Function Name: UART_OP_SpiUartGetRxBufferSize
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
    *  UART_OP_rxBufferHead - the start index to put data into the 
    *  software receive buffer.
    *  UART_OP_rxBufferTail - the start index to get data from the 
    *  software receive buffer.
    *
    *******************************************************************************/
    uint32 UART_OP_SpiUartGetRxBufferSize(void)
    {
        uint32 size;
    #if (UART_OP_INTERNAL_RX_SW_BUFFER_CONST)
        uint32 locHead;
    #endif /* (UART_OP_INTERNAL_RX_SW_BUFFER_CONST) */

        #if (UART_OP_CHECK_RX_SW_BUFFER)
        {
            locHead = UART_OP_rxBufferHead;

            if(locHead >= UART_OP_rxBufferTail)
            {
                size = (locHead - UART_OP_rxBufferTail);
            }
            else
            {
                size = (locHead + (UART_OP_INTERNAL_RX_BUFFER_SIZE - UART_OP_rxBufferTail));
            }
        }
        #else
        {
            size = UART_OP_GET_RX_FIFO_ENTRIES;
        }
        #endif

        return (size);
    }


    /*******************************************************************************
    * Function Name: UART_OP_SpiUartClearRxBuffer
    ****************************************************************************//**
    *
    *  Clears the receive buffer and RX FIFO.
    *
    * \globalvars
    *  UART_OP_rxBufferHead - the start index to put data into the 
    *  software receive buffer.
    *  UART_OP_rxBufferTail - the start index to get data from the 
    *  software receive buffer.
    *
    *******************************************************************************/
    void UART_OP_SpiUartClearRxBuffer(void)
    {
        #if (UART_OP_CHECK_RX_SW_BUFFER)
        {
            /* Lock from component interruption */
            UART_OP_DisableInt();

            /* Flush RX software buffer */
            UART_OP_rxBufferHead = UART_OP_rxBufferTail;
            UART_OP_rxBufferOverflow = 0u;

            UART_OP_CLEAR_RX_FIFO;
            UART_OP_ClearRxInterruptSource(UART_OP_INTR_RX_ALL);

            #if (UART_OP_CHECK_UART_RTS_CONTROL_FLOW)
            {
                /* Enable RX Not Empty interrupt source to continue receiving
                * data into software buffer.
                */
                UART_OP_INTR_RX_MASK_REG |= UART_OP_INTR_RX_NOT_EMPTY;
            }
            #endif
            
            /* Release lock */
            UART_OP_EnableInt();
        }
        #else
        {
            UART_OP_CLEAR_RX_FIFO;
        }
        #endif
    }

#endif /* (UART_OP_RX_DIRECTION) */


#if(UART_OP_TX_DIRECTION)
    /*******************************************************************************
    * Function Name: UART_OP_SpiUartWriteTxData
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
    *  UART_OP_txBufferHead - the start index to put data into the 
    *  software transmit buffer.
    *  UART_OP_txBufferTail - start index to get data from the software
    *  transmit buffer.
    *
    *******************************************************************************/
    void UART_OP_SpiUartWriteTxData(uint32 txData)
    {
    #if (UART_OP_INTERNAL_TX_SW_BUFFER_CONST)
        uint32 locHead;
    #endif /* (UART_OP_INTERNAL_TX_SW_BUFFER_CONST) */

        #if (UART_OP_CHECK_TX_SW_BUFFER)
        {
            /* Put data directly into the TX FIFO */
            if ((UART_OP_txBufferHead == UART_OP_txBufferTail) &&
                (UART_OP_SPI_UART_FIFO_SIZE != UART_OP_GET_TX_FIFO_ENTRIES))
            {
                /* TX software buffer is empty: put data directly in TX FIFO */
                UART_OP_TX_FIFO_WR_REG = txData;
            }
            /* Put data into TX software buffer */
            else
            {
                /* Head index to put data */
                locHead = (UART_OP_txBufferHead + 1u);

                /* Adjust TX software buffer index */
                if (UART_OP_TX_BUFFER_SIZE == locHead)
                {
                    locHead = 0u;
                }

                /* Wait for space in TX software buffer */
                while (locHead == UART_OP_txBufferTail)
                {
                }

                /* TX software buffer has at least one room */

                /* Clear old status of INTR_TX_NOT_FULL. It sets at the end of transfer when TX FIFO is empty. */
                UART_OP_ClearTxInterruptSource(UART_OP_INTR_TX_NOT_FULL);

                UART_OP_PutWordInTxBuffer(locHead, txData);

                UART_OP_txBufferHead = locHead;

                /* Check if TX Not Full is disabled in interrupt */
                if (0u == (UART_OP_INTR_TX_MASK_REG & UART_OP_INTR_TX_NOT_FULL))
                {
                    /* Enable TX Not Full interrupt source to transmit from software buffer */
                    UART_OP_INTR_TX_MASK_REG |= (uint32) UART_OP_INTR_TX_NOT_FULL;
                }
            }
        }
        #else
        {
            /* Wait until TX FIFO has space to put data element */
            while (UART_OP_SPI_UART_FIFO_SIZE == UART_OP_GET_TX_FIFO_ENTRIES)
            {
            }

            UART_OP_TX_FIFO_WR_REG = txData;
        }
        #endif
    }


    /*******************************************************************************
    * Function Name: UART_OP_SpiUartPutArray
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
    *  UART_OP_txBufferHead - the start index to put data into the 
    *  software transmit buffer.
    *  UART_OP_txBufferTail - start index to get data from the software
    *  transmit buffer.
    *
    *******************************************************************************/
    void UART_OP_SpiUartPutArray(const uint8 wrBuf[], uint32 count)
    {
        uint32 i;

        for (i=0u; i < count; i++)
        {
            UART_OP_SpiUartWriteTxData((uint32) wrBuf[i]);
        }
    }


    /*******************************************************************************
    * Function Name: UART_OP_SpiUartGetTxBufferSize
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
    *  UART_OP_txBufferHead - the start index to put data into the 
    *  software transmit buffer.
    *  UART_OP_txBufferTail - start index to get data from the software
    *  transmit buffer.
    *
    *******************************************************************************/
    uint32 UART_OP_SpiUartGetTxBufferSize(void)
    {
        uint32 size;
    #if (UART_OP_INTERNAL_TX_SW_BUFFER_CONST)
        uint32 locTail;
    #endif /* (UART_OP_INTERNAL_TX_SW_BUFFER_CONST) */

        #if (UART_OP_CHECK_TX_SW_BUFFER)
        {
            /* Get current Tail index */
            locTail = UART_OP_txBufferTail;

            if (UART_OP_txBufferHead >= locTail)
            {
                size = (UART_OP_txBufferHead - locTail);
            }
            else
            {
                size = (UART_OP_txBufferHead + (UART_OP_TX_BUFFER_SIZE - locTail));
            }
        }
        #else
        {
            size = UART_OP_GET_TX_FIFO_ENTRIES;
        }
        #endif

        return (size);
    }


    /*******************************************************************************
    * Function Name: UART_OP_SpiUartClearTxBuffer
    ****************************************************************************//**
    *
    *  Clears the transmit buffer and TX FIFO.
    *
    * \globalvars
    *  UART_OP_txBufferHead - the start index to put data into the 
    *  software transmit buffer.
    *  UART_OP_txBufferTail - start index to get data from the software
    *  transmit buffer.
    *
    *******************************************************************************/
    void UART_OP_SpiUartClearTxBuffer(void)
    {
        #if (UART_OP_CHECK_TX_SW_BUFFER)
        {
            /* Lock from component interruption */
            UART_OP_DisableInt();

            /* Flush TX software buffer */
            UART_OP_txBufferHead = UART_OP_txBufferTail;

            UART_OP_INTR_TX_MASK_REG &= (uint32) ~UART_OP_INTR_TX_NOT_FULL;
            UART_OP_CLEAR_TX_FIFO;
            UART_OP_ClearTxInterruptSource(UART_OP_INTR_TX_ALL);

            /* Release lock */
            UART_OP_EnableInt();
        }
        #else
        {
            UART_OP_CLEAR_TX_FIFO;
        }
        #endif
    }

#endif /* (UART_OP_TX_DIRECTION) */


/*******************************************************************************
* Function Name: UART_OP_SpiUartDisableIntRx
****************************************************************************//**
*
*  Disables the RX interrupt sources.
*
*  \return
*   Returns the RX interrupt sources enabled before the function call.
*
*******************************************************************************/
uint32 UART_OP_SpiUartDisableIntRx(void)
{
    uint32 intSource;

    intSource = UART_OP_GetRxInterruptMode();

    UART_OP_SetRxInterruptMode(UART_OP_NO_INTR_SOURCES);

    return (intSource);
}


/*******************************************************************************
* Function Name: UART_OP_SpiUartDisableIntTx
****************************************************************************//**
*
*  Disables TX interrupt sources.
*
*  \return
*   Returns TX interrupt sources enabled before function call.
*
*******************************************************************************/
uint32 UART_OP_SpiUartDisableIntTx(void)
{
    uint32 intSourceMask;

    intSourceMask = UART_OP_GetTxInterruptMode();

    UART_OP_SetTxInterruptMode(UART_OP_NO_INTR_SOURCES);

    return (intSourceMask);
}


#if(UART_OP_SCB_MODE_UNCONFIG_CONST_CFG)
    /*******************************************************************************
    * Function Name: UART_OP_PutWordInRxBuffer
    ****************************************************************************//**
    *
    *  Stores a byte/word into the RX buffer.
    *  Only available in the Unconfigured operation mode.
    *
    *  \param index:      index to store data byte/word in the RX buffer.
    *  \param rxDataByte: byte/word to store.
    *
    *******************************************************************************/
    void UART_OP_PutWordInRxBuffer(uint32 idx, uint32 rxDataByte)
    {
        /* Put data in buffer */
        if (UART_OP_ONE_BYTE_WIDTH == UART_OP_rxDataBits)
        {
            UART_OP_rxBuffer[idx] = ((uint8) rxDataByte);
        }
        else
        {
            UART_OP_rxBuffer[(uint32)(idx << 1u)]      = LO8(LO16(rxDataByte));
            UART_OP_rxBuffer[(uint32)(idx << 1u) + 1u] = HI8(LO16(rxDataByte));
        }
    }


    /*******************************************************************************
    * Function Name: UART_OP_GetWordFromRxBuffer
    ****************************************************************************//**
    *
    *  Reads byte/word from RX buffer.
    *  Only available in the Unconfigured operation mode.
    *
    *  \return
    *   Returns byte/word read from RX buffer.
    *
    *******************************************************************************/
    uint32 UART_OP_GetWordFromRxBuffer(uint32 idx)
    {
        uint32 value;

        if (UART_OP_ONE_BYTE_WIDTH == UART_OP_rxDataBits)
        {
            value = UART_OP_rxBuffer[idx];
        }
        else
        {
            value  = (uint32) UART_OP_rxBuffer[(uint32)(idx << 1u)];
            value |= (uint32) ((uint32)UART_OP_rxBuffer[(uint32)(idx << 1u) + 1u] << 8u);
        }

        return (value);
    }


    /*******************************************************************************
    * Function Name: UART_OP_PutWordInTxBuffer
    ****************************************************************************//**
    *
    *  Stores byte/word into the TX buffer.
    *  Only available in the Unconfigured operation mode.
    *
    *  \param idx:        index to store data byte/word in the TX buffer.
    *  \param txDataByte: byte/word to store.
    *
    *******************************************************************************/
    void UART_OP_PutWordInTxBuffer(uint32 idx, uint32 txDataByte)
    {
        /* Put data in buffer */
        if (UART_OP_ONE_BYTE_WIDTH == UART_OP_txDataBits)
        {
            UART_OP_txBuffer[idx] = ((uint8) txDataByte);
        }
        else
        {
            UART_OP_txBuffer[(uint32)(idx << 1u)]      = LO8(LO16(txDataByte));
            UART_OP_txBuffer[(uint32)(idx << 1u) + 1u] = HI8(LO16(txDataByte));
        }
    }


    /*******************************************************************************
    * Function Name: UART_OP_GetWordFromTxBuffer
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
    uint32 UART_OP_GetWordFromTxBuffer(uint32 idx)
    {
        uint32 value;

        if (UART_OP_ONE_BYTE_WIDTH == UART_OP_txDataBits)
        {
            value = (uint32) UART_OP_txBuffer[idx];
        }
        else
        {
            value  = (uint32) UART_OP_txBuffer[(uint32)(idx << 1u)];
            value |= (uint32) ((uint32) UART_OP_txBuffer[(uint32)(idx << 1u) + 1u] << 8u);
        }

        return (value);
    }

#endif /* (UART_OP_SCB_MODE_UNCONFIG_CONST_CFG) */


/* [] END OF FILE */
