/***************************************************************************//**
* \file MONITORING_TABLET_SPI_UART.c
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

#include "MONITORING_TABLET_PVT.h"
#include "MONITORING_TABLET_SPI_UART_PVT.h"

/***************************************
*        SPI/UART Private Vars
***************************************/

#if(MONITORING_TABLET_INTERNAL_RX_SW_BUFFER_CONST)
    /* Start index to put data into the software receive buffer.*/
    volatile uint32 MONITORING_TABLET_rxBufferHead;
    /* Start index to get data from the software receive buffer.*/
    volatile uint32 MONITORING_TABLET_rxBufferTail;
    /**
    * \addtogroup group_globals
    * \{
    */
    /** Sets when internal software receive buffer overflow
    *  was occurred.
    */
    volatile uint8  MONITORING_TABLET_rxBufferOverflow;
    /** \} globals */
#endif /* (MONITORING_TABLET_INTERNAL_RX_SW_BUFFER_CONST) */

#if(MONITORING_TABLET_INTERNAL_TX_SW_BUFFER_CONST)
    /* Start index to put data into the software transmit buffer.*/
    volatile uint32 MONITORING_TABLET_txBufferHead;
    /* Start index to get data from the software transmit buffer.*/
    volatile uint32 MONITORING_TABLET_txBufferTail;
#endif /* (MONITORING_TABLET_INTERNAL_TX_SW_BUFFER_CONST) */

#if(MONITORING_TABLET_INTERNAL_RX_SW_BUFFER)
    /* Add one element to the buffer to receive full packet. One byte in receive buffer is always empty */
    volatile uint8 MONITORING_TABLET_rxBufferInternal[MONITORING_TABLET_INTERNAL_RX_BUFFER_SIZE];
#endif /* (MONITORING_TABLET_INTERNAL_RX_SW_BUFFER) */

#if(MONITORING_TABLET_INTERNAL_TX_SW_BUFFER)
    volatile uint8 MONITORING_TABLET_txBufferInternal[MONITORING_TABLET_TX_BUFFER_SIZE];
#endif /* (MONITORING_TABLET_INTERNAL_TX_SW_BUFFER) */


#if(MONITORING_TABLET_RX_DIRECTION)
    /*******************************************************************************
    * Function Name: MONITORING_TABLET_SpiUartReadRxData
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
    *  MONITORING_TABLET_rxBufferHead - the start index to put data into the 
    *  software receive buffer.
    *  MONITORING_TABLET_rxBufferTail - the start index to get data from the 
    *  software receive buffer.
    *
    *******************************************************************************/
    uint32 MONITORING_TABLET_SpiUartReadRxData(void)
    {
        uint32 rxData = 0u;

    #if (MONITORING_TABLET_INTERNAL_RX_SW_BUFFER_CONST)
        uint32 locTail;
    #endif /* (MONITORING_TABLET_INTERNAL_RX_SW_BUFFER_CONST) */

        #if (MONITORING_TABLET_CHECK_RX_SW_BUFFER)
        {
            if (MONITORING_TABLET_rxBufferHead != MONITORING_TABLET_rxBufferTail)
            {
                /* There is data in RX software buffer */

                /* Calculate index to read from */
                locTail = (MONITORING_TABLET_rxBufferTail + 1u);

                if (MONITORING_TABLET_INTERNAL_RX_BUFFER_SIZE == locTail)
                {
                    locTail = 0u;
                }

                /* Get data from RX software buffer */
                rxData = MONITORING_TABLET_GetWordFromRxBuffer(locTail);

                /* Change index in the buffer */
                MONITORING_TABLET_rxBufferTail = locTail;

                #if (MONITORING_TABLET_CHECK_UART_RTS_CONTROL_FLOW)
                {
                    /* Check if RX Not Empty is disabled in the interrupt */
                    if (0u == (MONITORING_TABLET_INTR_RX_MASK_REG & MONITORING_TABLET_INTR_RX_NOT_EMPTY))
                    {
                        /* Enable RX Not Empty interrupt source to continue
                        * receiving data into software buffer.
                        */
                        MONITORING_TABLET_INTR_RX_MASK_REG |= MONITORING_TABLET_INTR_RX_NOT_EMPTY;
                    }
                }
                #endif

            }
        }
        #else
        {
            /* Read data from RX FIFO */
            rxData = MONITORING_TABLET_RX_FIFO_RD_REG;
        }
        #endif

        return (rxData);
    }


    /*******************************************************************************
    * Function Name: MONITORING_TABLET_SpiUartGetRxBufferSize
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
    *  MONITORING_TABLET_rxBufferHead - the start index to put data into the 
    *  software receive buffer.
    *  MONITORING_TABLET_rxBufferTail - the start index to get data from the 
    *  software receive buffer.
    *
    *******************************************************************************/
    uint32 MONITORING_TABLET_SpiUartGetRxBufferSize(void)
    {
        uint32 size;
    #if (MONITORING_TABLET_INTERNAL_RX_SW_BUFFER_CONST)
        uint32 locHead;
    #endif /* (MONITORING_TABLET_INTERNAL_RX_SW_BUFFER_CONST) */

        #if (MONITORING_TABLET_CHECK_RX_SW_BUFFER)
        {
            locHead = MONITORING_TABLET_rxBufferHead;

            if(locHead >= MONITORING_TABLET_rxBufferTail)
            {
                size = (locHead - MONITORING_TABLET_rxBufferTail);
            }
            else
            {
                size = (locHead + (MONITORING_TABLET_INTERNAL_RX_BUFFER_SIZE - MONITORING_TABLET_rxBufferTail));
            }
        }
        #else
        {
            size = MONITORING_TABLET_GET_RX_FIFO_ENTRIES;
        }
        #endif

        return (size);
    }


    /*******************************************************************************
    * Function Name: MONITORING_TABLET_SpiUartClearRxBuffer
    ****************************************************************************//**
    *
    *  Clears the receive buffer and RX FIFO.
    *
    * \globalvars
    *  MONITORING_TABLET_rxBufferHead - the start index to put data into the 
    *  software receive buffer.
    *  MONITORING_TABLET_rxBufferTail - the start index to get data from the 
    *  software receive buffer.
    *
    *******************************************************************************/
    void MONITORING_TABLET_SpiUartClearRxBuffer(void)
    {
        #if (MONITORING_TABLET_CHECK_RX_SW_BUFFER)
        {
            /* Lock from component interruption */
            MONITORING_TABLET_DisableInt();

            /* Flush RX software buffer */
            MONITORING_TABLET_rxBufferHead = MONITORING_TABLET_rxBufferTail;
            MONITORING_TABLET_rxBufferOverflow = 0u;

            MONITORING_TABLET_CLEAR_RX_FIFO;
            MONITORING_TABLET_ClearRxInterruptSource(MONITORING_TABLET_INTR_RX_ALL);

            #if (MONITORING_TABLET_CHECK_UART_RTS_CONTROL_FLOW)
            {
                /* Enable RX Not Empty interrupt source to continue receiving
                * data into software buffer.
                */
                MONITORING_TABLET_INTR_RX_MASK_REG |= MONITORING_TABLET_INTR_RX_NOT_EMPTY;
            }
            #endif
            
            /* Release lock */
            MONITORING_TABLET_EnableInt();
        }
        #else
        {
            MONITORING_TABLET_CLEAR_RX_FIFO;
        }
        #endif
    }

#endif /* (MONITORING_TABLET_RX_DIRECTION) */


#if(MONITORING_TABLET_TX_DIRECTION)
    /*******************************************************************************
    * Function Name: MONITORING_TABLET_SpiUartWriteTxData
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
    *  MONITORING_TABLET_txBufferHead - the start index to put data into the 
    *  software transmit buffer.
    *  MONITORING_TABLET_txBufferTail - start index to get data from the software
    *  transmit buffer.
    *
    *******************************************************************************/
    void MONITORING_TABLET_SpiUartWriteTxData(uint32 txData)
    {
    #if (MONITORING_TABLET_INTERNAL_TX_SW_BUFFER_CONST)
        uint32 locHead;
    #endif /* (MONITORING_TABLET_INTERNAL_TX_SW_BUFFER_CONST) */

        #if (MONITORING_TABLET_CHECK_TX_SW_BUFFER)
        {
            /* Put data directly into the TX FIFO */
            if ((MONITORING_TABLET_txBufferHead == MONITORING_TABLET_txBufferTail) &&
                (MONITORING_TABLET_SPI_UART_FIFO_SIZE != MONITORING_TABLET_GET_TX_FIFO_ENTRIES))
            {
                /* TX software buffer is empty: put data directly in TX FIFO */
                MONITORING_TABLET_TX_FIFO_WR_REG = txData;
            }
            /* Put data into TX software buffer */
            else
            {
                /* Head index to put data */
                locHead = (MONITORING_TABLET_txBufferHead + 1u);

                /* Adjust TX software buffer index */
                if (MONITORING_TABLET_TX_BUFFER_SIZE == locHead)
                {
                    locHead = 0u;
                }

                /* Wait for space in TX software buffer */
                while (locHead == MONITORING_TABLET_txBufferTail)
                {
                }

                /* TX software buffer has at least one room */

                /* Clear old status of INTR_TX_NOT_FULL. It sets at the end of transfer when TX FIFO is empty. */
                MONITORING_TABLET_ClearTxInterruptSource(MONITORING_TABLET_INTR_TX_NOT_FULL);

                MONITORING_TABLET_PutWordInTxBuffer(locHead, txData);

                MONITORING_TABLET_txBufferHead = locHead;

                /* Check if TX Not Full is disabled in interrupt */
                if (0u == (MONITORING_TABLET_INTR_TX_MASK_REG & MONITORING_TABLET_INTR_TX_NOT_FULL))
                {
                    /* Enable TX Not Full interrupt source to transmit from software buffer */
                    MONITORING_TABLET_INTR_TX_MASK_REG |= (uint32) MONITORING_TABLET_INTR_TX_NOT_FULL;
                }
            }
        }
        #else
        {
            /* Wait until TX FIFO has space to put data element */
            while (MONITORING_TABLET_SPI_UART_FIFO_SIZE == MONITORING_TABLET_GET_TX_FIFO_ENTRIES)
            {
            }

            MONITORING_TABLET_TX_FIFO_WR_REG = txData;
        }
        #endif
    }


    /*******************************************************************************
    * Function Name: MONITORING_TABLET_SpiUartPutArray
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
    *  MONITORING_TABLET_txBufferHead - the start index to put data into the 
    *  software transmit buffer.
    *  MONITORING_TABLET_txBufferTail - start index to get data from the software
    *  transmit buffer.
    *
    *******************************************************************************/
    void MONITORING_TABLET_SpiUartPutArray(const uint8 wrBuf[], uint32 count)
    {
        uint32 i;

        for (i=0u; i < count; i++)
        {
            MONITORING_TABLET_SpiUartWriteTxData((uint32) wrBuf[i]);
        }
    }


    /*******************************************************************************
    * Function Name: MONITORING_TABLET_SpiUartGetTxBufferSize
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
    *  MONITORING_TABLET_txBufferHead - the start index to put data into the 
    *  software transmit buffer.
    *  MONITORING_TABLET_txBufferTail - start index to get data from the software
    *  transmit buffer.
    *
    *******************************************************************************/
    uint32 MONITORING_TABLET_SpiUartGetTxBufferSize(void)
    {
        uint32 size;
    #if (MONITORING_TABLET_INTERNAL_TX_SW_BUFFER_CONST)
        uint32 locTail;
    #endif /* (MONITORING_TABLET_INTERNAL_TX_SW_BUFFER_CONST) */

        #if (MONITORING_TABLET_CHECK_TX_SW_BUFFER)
        {
            /* Get current Tail index */
            locTail = MONITORING_TABLET_txBufferTail;

            if (MONITORING_TABLET_txBufferHead >= locTail)
            {
                size = (MONITORING_TABLET_txBufferHead - locTail);
            }
            else
            {
                size = (MONITORING_TABLET_txBufferHead + (MONITORING_TABLET_TX_BUFFER_SIZE - locTail));
            }
        }
        #else
        {
            size = MONITORING_TABLET_GET_TX_FIFO_ENTRIES;
        }
        #endif

        return (size);
    }


    /*******************************************************************************
    * Function Name: MONITORING_TABLET_SpiUartClearTxBuffer
    ****************************************************************************//**
    *
    *  Clears the transmit buffer and TX FIFO.
    *
    * \globalvars
    *  MONITORING_TABLET_txBufferHead - the start index to put data into the 
    *  software transmit buffer.
    *  MONITORING_TABLET_txBufferTail - start index to get data from the software
    *  transmit buffer.
    *
    *******************************************************************************/
    void MONITORING_TABLET_SpiUartClearTxBuffer(void)
    {
        #if (MONITORING_TABLET_CHECK_TX_SW_BUFFER)
        {
            /* Lock from component interruption */
            MONITORING_TABLET_DisableInt();

            /* Flush TX software buffer */
            MONITORING_TABLET_txBufferHead = MONITORING_TABLET_txBufferTail;

            MONITORING_TABLET_INTR_TX_MASK_REG &= (uint32) ~MONITORING_TABLET_INTR_TX_NOT_FULL;
            MONITORING_TABLET_CLEAR_TX_FIFO;
            MONITORING_TABLET_ClearTxInterruptSource(MONITORING_TABLET_INTR_TX_ALL);

            /* Release lock */
            MONITORING_TABLET_EnableInt();
        }
        #else
        {
            MONITORING_TABLET_CLEAR_TX_FIFO;
        }
        #endif
    }

#endif /* (MONITORING_TABLET_TX_DIRECTION) */


/*******************************************************************************
* Function Name: MONITORING_TABLET_SpiUartDisableIntRx
****************************************************************************//**
*
*  Disables the RX interrupt sources.
*
*  \return
*   Returns the RX interrupt sources enabled before the function call.
*
*******************************************************************************/
uint32 MONITORING_TABLET_SpiUartDisableIntRx(void)
{
    uint32 intSource;

    intSource = MONITORING_TABLET_GetRxInterruptMode();

    MONITORING_TABLET_SetRxInterruptMode(MONITORING_TABLET_NO_INTR_SOURCES);

    return (intSource);
}


/*******************************************************************************
* Function Name: MONITORING_TABLET_SpiUartDisableIntTx
****************************************************************************//**
*
*  Disables TX interrupt sources.
*
*  \return
*   Returns TX interrupt sources enabled before function call.
*
*******************************************************************************/
uint32 MONITORING_TABLET_SpiUartDisableIntTx(void)
{
    uint32 intSourceMask;

    intSourceMask = MONITORING_TABLET_GetTxInterruptMode();

    MONITORING_TABLET_SetTxInterruptMode(MONITORING_TABLET_NO_INTR_SOURCES);

    return (intSourceMask);
}


#if(MONITORING_TABLET_SCB_MODE_UNCONFIG_CONST_CFG)
    /*******************************************************************************
    * Function Name: MONITORING_TABLET_PutWordInRxBuffer
    ****************************************************************************//**
    *
    *  Stores a byte/word into the RX buffer.
    *  Only available in the Unconfigured operation mode.
    *
    *  \param index:      index to store data byte/word in the RX buffer.
    *  \param rxDataByte: byte/word to store.
    *
    *******************************************************************************/
    void MONITORING_TABLET_PutWordInRxBuffer(uint32 idx, uint32 rxDataByte)
    {
        /* Put data in buffer */
        if (MONITORING_TABLET_ONE_BYTE_WIDTH == MONITORING_TABLET_rxDataBits)
        {
            MONITORING_TABLET_rxBuffer[idx] = ((uint8) rxDataByte);
        }
        else
        {
            MONITORING_TABLET_rxBuffer[(uint32)(idx << 1u)]      = LO8(LO16(rxDataByte));
            MONITORING_TABLET_rxBuffer[(uint32)(idx << 1u) + 1u] = HI8(LO16(rxDataByte));
        }
    }


    /*******************************************************************************
    * Function Name: MONITORING_TABLET_GetWordFromRxBuffer
    ****************************************************************************//**
    *
    *  Reads byte/word from RX buffer.
    *  Only available in the Unconfigured operation mode.
    *
    *  \return
    *   Returns byte/word read from RX buffer.
    *
    *******************************************************************************/
    uint32 MONITORING_TABLET_GetWordFromRxBuffer(uint32 idx)
    {
        uint32 value;

        if (MONITORING_TABLET_ONE_BYTE_WIDTH == MONITORING_TABLET_rxDataBits)
        {
            value = MONITORING_TABLET_rxBuffer[idx];
        }
        else
        {
            value  = (uint32) MONITORING_TABLET_rxBuffer[(uint32)(idx << 1u)];
            value |= (uint32) ((uint32)MONITORING_TABLET_rxBuffer[(uint32)(idx << 1u) + 1u] << 8u);
        }

        return (value);
    }


    /*******************************************************************************
    * Function Name: MONITORING_TABLET_PutWordInTxBuffer
    ****************************************************************************//**
    *
    *  Stores byte/word into the TX buffer.
    *  Only available in the Unconfigured operation mode.
    *
    *  \param idx:        index to store data byte/word in the TX buffer.
    *  \param txDataByte: byte/word to store.
    *
    *******************************************************************************/
    void MONITORING_TABLET_PutWordInTxBuffer(uint32 idx, uint32 txDataByte)
    {
        /* Put data in buffer */
        if (MONITORING_TABLET_ONE_BYTE_WIDTH == MONITORING_TABLET_txDataBits)
        {
            MONITORING_TABLET_txBuffer[idx] = ((uint8) txDataByte);
        }
        else
        {
            MONITORING_TABLET_txBuffer[(uint32)(idx << 1u)]      = LO8(LO16(txDataByte));
            MONITORING_TABLET_txBuffer[(uint32)(idx << 1u) + 1u] = HI8(LO16(txDataByte));
        }
    }


    /*******************************************************************************
    * Function Name: MONITORING_TABLET_GetWordFromTxBuffer
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
    uint32 MONITORING_TABLET_GetWordFromTxBuffer(uint32 idx)
    {
        uint32 value;

        if (MONITORING_TABLET_ONE_BYTE_WIDTH == MONITORING_TABLET_txDataBits)
        {
            value = (uint32) MONITORING_TABLET_txBuffer[idx];
        }
        else
        {
            value  = (uint32) MONITORING_TABLET_txBuffer[(uint32)(idx << 1u)];
            value |= (uint32) ((uint32) MONITORING_TABLET_txBuffer[(uint32)(idx << 1u) + 1u] << 8u);
        }

        return (value);
    }

#endif /* (MONITORING_TABLET_SCB_MODE_UNCONFIG_CONST_CFG) */


/* [] END OF FILE */
