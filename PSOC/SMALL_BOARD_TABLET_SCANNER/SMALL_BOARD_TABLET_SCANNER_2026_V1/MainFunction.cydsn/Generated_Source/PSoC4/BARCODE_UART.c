/***************************************************************************//**
* \file BARCODE_UART.c
* \version 4.0
*
* \brief
*  This file provides the source code to the API for the SCB Component in
*  UART mode.
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

#include "BARCODE_PVT.h"
#include "BARCODE_SPI_UART_PVT.h"
#include "cyapicallbacks.h"

#if (BARCODE_UART_WAKE_ENABLE_CONST && BARCODE_UART_RX_WAKEUP_IRQ)
    /**
    * \addtogroup group_globals
    * \{
    */
    /** This global variable determines whether to enable Skip Start
    * functionality when BARCODE_Sleep() function is called:
    * 0 – disable, other values – enable. Default value is 1.
    * It is only available when Enable wakeup from Deep Sleep Mode is enabled.
    */
    uint8 BARCODE_skipStart = 1u;
    /** \} globals */
#endif /* (BARCODE_UART_WAKE_ENABLE_CONST && BARCODE_UART_RX_WAKEUP_IRQ) */

#if(BARCODE_SCB_MODE_UNCONFIG_CONST_CFG)

    /***************************************
    *  Configuration Structure Initialization
    ***************************************/

    const BARCODE_UART_INIT_STRUCT BARCODE_configUart =
    {
        BARCODE_UART_SUB_MODE,
        BARCODE_UART_DIRECTION,
        BARCODE_UART_DATA_BITS_NUM,
        BARCODE_UART_PARITY_TYPE,
        BARCODE_UART_STOP_BITS_NUM,
        BARCODE_UART_OVS_FACTOR,
        BARCODE_UART_IRDA_LOW_POWER,
        BARCODE_UART_MEDIAN_FILTER_ENABLE,
        BARCODE_UART_RETRY_ON_NACK,
        BARCODE_UART_IRDA_POLARITY,
        BARCODE_UART_DROP_ON_PARITY_ERR,
        BARCODE_UART_DROP_ON_FRAME_ERR,
        BARCODE_UART_WAKE_ENABLE,
        0u,
        NULL,
        0u,
        NULL,
        BARCODE_UART_MP_MODE_ENABLE,
        BARCODE_UART_MP_ACCEPT_ADDRESS,
        BARCODE_UART_MP_RX_ADDRESS,
        BARCODE_UART_MP_RX_ADDRESS_MASK,
        (uint32) BARCODE_SCB_IRQ_INTERNAL,
        BARCODE_UART_INTR_RX_MASK,
        BARCODE_UART_RX_TRIGGER_LEVEL,
        BARCODE_UART_INTR_TX_MASK,
        BARCODE_UART_TX_TRIGGER_LEVEL,
        (uint8) BARCODE_UART_BYTE_MODE_ENABLE,
        (uint8) BARCODE_UART_CTS_ENABLE,
        (uint8) BARCODE_UART_CTS_POLARITY,
        (uint8) BARCODE_UART_RTS_POLARITY,
        (uint8) BARCODE_UART_RTS_FIFO_LEVEL,
        (uint8) BARCODE_UART_RX_BREAK_WIDTH
    };


    /*******************************************************************************
    * Function Name: BARCODE_UartInit
    ****************************************************************************//**
    *
    *  Configures the BARCODE for UART operation.
    *
    *  This function is intended specifically to be used when the BARCODE
    *  configuration is set to “Unconfigured BARCODE” in the customizer.
    *  After initializing the BARCODE in UART mode using this function,
    *  the component can be enabled using the BARCODE_Start() or
    * BARCODE_Enable() function.
    *  This function uses a pointer to a structure that provides the configuration
    *  settings. This structure contains the same information that would otherwise
    *  be provided by the customizer settings.
    *
    *  \param config: pointer to a structure that contains the following list of
    *   fields. These fields match the selections available in the customizer.
    *   Refer to the customizer for further description of the settings.
    *
    *******************************************************************************/
    void BARCODE_UartInit(const BARCODE_UART_INIT_STRUCT *config)
    {
        uint32 pinsConfig;

        if (NULL == config)
        {
            CYASSERT(0u != 0u); /* Halt execution due to bad function parameter */
        }
        else
        {
            /* Get direction to configure UART pins: TX, RX or TX+RX */
            pinsConfig  = config->direction;

        #if !(BARCODE_CY_SCBIP_V0 || BARCODE_CY_SCBIP_V1)
            /* Add RTS and CTS pins to configure */
            pinsConfig |= (0u != config->rtsRxFifoLevel) ? (BARCODE_UART_RTS_PIN_ENABLE) : (0u);
            pinsConfig |= (0u != config->enableCts)      ? (BARCODE_UART_CTS_PIN_ENABLE) : (0u);
        #endif /* !(BARCODE_CY_SCBIP_V0 || BARCODE_CY_SCBIP_V1) */

            /* Configure pins */
            BARCODE_SetPins(BARCODE_SCB_MODE_UART, config->mode, pinsConfig);

            /* Store internal configuration */
            BARCODE_scbMode       = (uint8) BARCODE_SCB_MODE_UART;
            BARCODE_scbEnableWake = (uint8) config->enableWake;
            BARCODE_scbEnableIntr = (uint8) config->enableInterrupt;

            /* Set RX direction internal variables */
            BARCODE_rxBuffer      =         config->rxBuffer;
            BARCODE_rxDataBits    = (uint8) config->dataBits;
            BARCODE_rxBufferSize  =         config->rxBufferSize;

            /* Set TX direction internal variables */
            BARCODE_txBuffer      =         config->txBuffer;
            BARCODE_txDataBits    = (uint8) config->dataBits;
            BARCODE_txBufferSize  =         config->txBufferSize;

            /* Configure UART interface */
            if(BARCODE_UART_MODE_IRDA == config->mode)
            {
                /* OVS settings: IrDA */
                BARCODE_CTRL_REG  = ((0u != config->enableIrdaLowPower) ?
                                                (BARCODE_UART_GET_CTRL_OVS_IRDA_LP(config->oversample)) :
                                                (BARCODE_CTRL_OVS_IRDA_OVS16));
            }
            else
            {
                /* OVS settings: UART and SmartCard */
                BARCODE_CTRL_REG  = BARCODE_GET_CTRL_OVS(config->oversample);
            }

            BARCODE_CTRL_REG     |= BARCODE_GET_CTRL_BYTE_MODE  (config->enableByteMode)      |
                                             BARCODE_GET_CTRL_ADDR_ACCEPT(config->multiprocAcceptAddr) |
                                             BARCODE_CTRL_UART;

            /* Configure sub-mode: UART, SmartCard or IrDA */
            BARCODE_UART_CTRL_REG = BARCODE_GET_UART_CTRL_MODE(config->mode);

            /* Configure RX direction */
            BARCODE_UART_RX_CTRL_REG = BARCODE_GET_UART_RX_CTRL_MODE(config->stopBits)              |
                                        BARCODE_GET_UART_RX_CTRL_POLARITY(config->enableInvertedRx)          |
                                        BARCODE_GET_UART_RX_CTRL_MP_MODE(config->enableMultiproc)            |
                                        BARCODE_GET_UART_RX_CTRL_DROP_ON_PARITY_ERR(config->dropOnParityErr) |
                                        BARCODE_GET_UART_RX_CTRL_DROP_ON_FRAME_ERR(config->dropOnFrameErr)   |
                                        BARCODE_GET_UART_RX_CTRL_BREAK_WIDTH(config->breakWidth);

            if(BARCODE_UART_PARITY_NONE != config->parity)
            {
               BARCODE_UART_RX_CTRL_REG |= BARCODE_GET_UART_RX_CTRL_PARITY(config->parity) |
                                                    BARCODE_UART_RX_CTRL_PARITY_ENABLED;
            }

            BARCODE_RX_CTRL_REG      = BARCODE_GET_RX_CTRL_DATA_WIDTH(config->dataBits)       |
                                                BARCODE_GET_RX_CTRL_MEDIAN(config->enableMedianFilter) |
                                                BARCODE_GET_UART_RX_CTRL_ENABLED(config->direction);

            BARCODE_RX_FIFO_CTRL_REG = BARCODE_GET_RX_FIFO_CTRL_TRIGGER_LEVEL(config->rxTriggerLevel);

            /* Configure MP address */
            BARCODE_RX_MATCH_REG     = BARCODE_GET_RX_MATCH_ADDR(config->multiprocAddr) |
                                                BARCODE_GET_RX_MATCH_MASK(config->multiprocAddrMask);

            /* Configure RX direction */
            BARCODE_UART_TX_CTRL_REG = BARCODE_GET_UART_TX_CTRL_MODE(config->stopBits) |
                                                BARCODE_GET_UART_TX_CTRL_RETRY_NACK(config->enableRetryNack);

            if(BARCODE_UART_PARITY_NONE != config->parity)
            {
               BARCODE_UART_TX_CTRL_REG |= BARCODE_GET_UART_TX_CTRL_PARITY(config->parity) |
                                                    BARCODE_UART_TX_CTRL_PARITY_ENABLED;
            }

            BARCODE_TX_CTRL_REG      = BARCODE_GET_TX_CTRL_DATA_WIDTH(config->dataBits)    |
                                                BARCODE_GET_UART_TX_CTRL_ENABLED(config->direction);

            BARCODE_TX_FIFO_CTRL_REG = BARCODE_GET_TX_FIFO_CTRL_TRIGGER_LEVEL(config->txTriggerLevel);

        #if !(BARCODE_CY_SCBIP_V0 || BARCODE_CY_SCBIP_V1)
            BARCODE_UART_FLOW_CTRL_REG = BARCODE_GET_UART_FLOW_CTRL_CTS_ENABLE(config->enableCts) | \
                                            BARCODE_GET_UART_FLOW_CTRL_CTS_POLARITY (config->ctsPolarity)  | \
                                            BARCODE_GET_UART_FLOW_CTRL_RTS_POLARITY (config->rtsPolarity)  | \
                                            BARCODE_GET_UART_FLOW_CTRL_TRIGGER_LEVEL(config->rtsRxFifoLevel);
        #endif /* !(BARCODE_CY_SCBIP_V0 || BARCODE_CY_SCBIP_V1) */

            /* Configure interrupt with UART handler but do not enable it */
            CyIntDisable    (BARCODE_ISR_NUMBER);
            CyIntSetPriority(BARCODE_ISR_NUMBER, BARCODE_ISR_PRIORITY);
            (void) CyIntSetVector(BARCODE_ISR_NUMBER, &BARCODE_SPI_UART_ISR);

            /* Configure WAKE interrupt */
        #if(BARCODE_UART_RX_WAKEUP_IRQ)
            CyIntDisable    (BARCODE_RX_WAKE_ISR_NUMBER);
            CyIntSetPriority(BARCODE_RX_WAKE_ISR_NUMBER, BARCODE_RX_WAKE_ISR_PRIORITY);
            (void) CyIntSetVector(BARCODE_RX_WAKE_ISR_NUMBER, &BARCODE_UART_WAKEUP_ISR);
        #endif /* (BARCODE_UART_RX_WAKEUP_IRQ) */

            /* Configure interrupt sources */
            BARCODE_INTR_I2C_EC_MASK_REG = BARCODE_NO_INTR_SOURCES;
            BARCODE_INTR_SPI_EC_MASK_REG = BARCODE_NO_INTR_SOURCES;
            BARCODE_INTR_SLAVE_MASK_REG  = BARCODE_NO_INTR_SOURCES;
            BARCODE_INTR_MASTER_MASK_REG = BARCODE_NO_INTR_SOURCES;
            BARCODE_INTR_RX_MASK_REG     = config->rxInterruptMask;
            BARCODE_INTR_TX_MASK_REG     = config->txInterruptMask;

            /* Configure TX interrupt sources to restore. */
            BARCODE_IntrTxMask = LO16(BARCODE_INTR_TX_MASK_REG);

            /* Clear RX buffer indexes */
            BARCODE_rxBufferHead     = 0u;
            BARCODE_rxBufferTail     = 0u;
            BARCODE_rxBufferOverflow = 0u;

            /* Clear TX buffer indexes */
            BARCODE_txBufferHead = 0u;
            BARCODE_txBufferTail = 0u;
        }
    }

#else

    /*******************************************************************************
    * Function Name: BARCODE_UartInit
    ****************************************************************************//**
    *
    *  Configures the SCB for the UART operation.
    *
    *******************************************************************************/
    void BARCODE_UartInit(void)
    {
        /* Configure UART interface */
        BARCODE_CTRL_REG = BARCODE_UART_DEFAULT_CTRL;

        /* Configure sub-mode: UART, SmartCard or IrDA */
        BARCODE_UART_CTRL_REG = BARCODE_UART_DEFAULT_UART_CTRL;

        /* Configure RX direction */
        BARCODE_UART_RX_CTRL_REG = BARCODE_UART_DEFAULT_UART_RX_CTRL;
        BARCODE_RX_CTRL_REG      = BARCODE_UART_DEFAULT_RX_CTRL;
        BARCODE_RX_FIFO_CTRL_REG = BARCODE_UART_DEFAULT_RX_FIFO_CTRL;
        BARCODE_RX_MATCH_REG     = BARCODE_UART_DEFAULT_RX_MATCH_REG;

        /* Configure TX direction */
        BARCODE_UART_TX_CTRL_REG = BARCODE_UART_DEFAULT_UART_TX_CTRL;
        BARCODE_TX_CTRL_REG      = BARCODE_UART_DEFAULT_TX_CTRL;
        BARCODE_TX_FIFO_CTRL_REG = BARCODE_UART_DEFAULT_TX_FIFO_CTRL;

    #if !(BARCODE_CY_SCBIP_V0 || BARCODE_CY_SCBIP_V1)
        BARCODE_UART_FLOW_CTRL_REG = BARCODE_UART_DEFAULT_FLOW_CTRL;
    #endif /* !(BARCODE_CY_SCBIP_V0 || BARCODE_CY_SCBIP_V1) */

        /* Configure interrupt with UART handler but do not enable it */
    #if(BARCODE_SCB_IRQ_INTERNAL)
        CyIntDisable    (BARCODE_ISR_NUMBER);
        CyIntSetPriority(BARCODE_ISR_NUMBER, BARCODE_ISR_PRIORITY);
        (void) CyIntSetVector(BARCODE_ISR_NUMBER, &BARCODE_SPI_UART_ISR);
    #endif /* (BARCODE_SCB_IRQ_INTERNAL) */

        /* Configure WAKE interrupt */
    #if(BARCODE_UART_RX_WAKEUP_IRQ)
        CyIntDisable    (BARCODE_RX_WAKE_ISR_NUMBER);
        CyIntSetPriority(BARCODE_RX_WAKE_ISR_NUMBER, BARCODE_RX_WAKE_ISR_PRIORITY);
        (void) CyIntSetVector(BARCODE_RX_WAKE_ISR_NUMBER, &BARCODE_UART_WAKEUP_ISR);
    #endif /* (BARCODE_UART_RX_WAKEUP_IRQ) */

        /* Configure interrupt sources */
        BARCODE_INTR_I2C_EC_MASK_REG = BARCODE_UART_DEFAULT_INTR_I2C_EC_MASK;
        BARCODE_INTR_SPI_EC_MASK_REG = BARCODE_UART_DEFAULT_INTR_SPI_EC_MASK;
        BARCODE_INTR_SLAVE_MASK_REG  = BARCODE_UART_DEFAULT_INTR_SLAVE_MASK;
        BARCODE_INTR_MASTER_MASK_REG = BARCODE_UART_DEFAULT_INTR_MASTER_MASK;
        BARCODE_INTR_RX_MASK_REG     = BARCODE_UART_DEFAULT_INTR_RX_MASK;
        BARCODE_INTR_TX_MASK_REG     = BARCODE_UART_DEFAULT_INTR_TX_MASK;

        /* Configure TX interrupt sources to restore. */
        BARCODE_IntrTxMask = LO16(BARCODE_INTR_TX_MASK_REG);

    #if(BARCODE_INTERNAL_RX_SW_BUFFER_CONST)
        BARCODE_rxBufferHead     = 0u;
        BARCODE_rxBufferTail     = 0u;
        BARCODE_rxBufferOverflow = 0u;
    #endif /* (BARCODE_INTERNAL_RX_SW_BUFFER_CONST) */

    #if(BARCODE_INTERNAL_TX_SW_BUFFER_CONST)
        BARCODE_txBufferHead = 0u;
        BARCODE_txBufferTail = 0u;
    #endif /* (BARCODE_INTERNAL_TX_SW_BUFFER_CONST) */
    }
#endif /* (BARCODE_SCB_MODE_UNCONFIG_CONST_CFG) */


/*******************************************************************************
* Function Name: BARCODE_UartPostEnable
****************************************************************************//**
*
*  Restores HSIOM settings for the UART output pins (TX and/or RTS) to be
*  controlled by the SCB UART.
*
*******************************************************************************/
void BARCODE_UartPostEnable(void)
{
#if (BARCODE_SCB_MODE_UNCONFIG_CONST_CFG)
    #if (BARCODE_TX_SDA_MISO_PIN)
        if (BARCODE_CHECK_TX_SDA_MISO_PIN_USED)
        {
            /* Set SCB UART to drive the output pin */
            BARCODE_SET_HSIOM_SEL(BARCODE_TX_SDA_MISO_HSIOM_REG, BARCODE_TX_SDA_MISO_HSIOM_MASK,
                                           BARCODE_TX_SDA_MISO_HSIOM_POS, BARCODE_TX_SDA_MISO_HSIOM_SEL_UART);
        }
    #endif /* (BARCODE_TX_SDA_MISO_PIN_PIN) */

    #if !(BARCODE_CY_SCBIP_V0 || BARCODE_CY_SCBIP_V1)
        #if (BARCODE_RTS_SS0_PIN)
            if (BARCODE_CHECK_RTS_SS0_PIN_USED)
            {
                /* Set SCB UART to drive the output pin */
                BARCODE_SET_HSIOM_SEL(BARCODE_RTS_SS0_HSIOM_REG, BARCODE_RTS_SS0_HSIOM_MASK,
                                               BARCODE_RTS_SS0_HSIOM_POS, BARCODE_RTS_SS0_HSIOM_SEL_UART);
            }
        #endif /* (BARCODE_RTS_SS0_PIN) */
    #endif /* !(BARCODE_CY_SCBIP_V0 || BARCODE_CY_SCBIP_V1) */

#else
    #if (BARCODE_UART_TX_PIN)
         /* Set SCB UART to drive the output pin */
        BARCODE_SET_HSIOM_SEL(BARCODE_TX_HSIOM_REG, BARCODE_TX_HSIOM_MASK,
                                       BARCODE_TX_HSIOM_POS, BARCODE_TX_HSIOM_SEL_UART);
    #endif /* (BARCODE_UART_TX_PIN) */

    #if (BARCODE_UART_RTS_PIN)
        /* Set SCB UART to drive the output pin */
        BARCODE_SET_HSIOM_SEL(BARCODE_RTS_HSIOM_REG, BARCODE_RTS_HSIOM_MASK,
                                       BARCODE_RTS_HSIOM_POS, BARCODE_RTS_HSIOM_SEL_UART);
    #endif /* (BARCODE_UART_RTS_PIN) */
#endif /* (BARCODE_SCB_MODE_UNCONFIG_CONST_CFG) */

    /* Restore TX interrupt sources. */
    BARCODE_SetTxInterruptMode(BARCODE_IntrTxMask);
}


/*******************************************************************************
* Function Name: BARCODE_UartStop
****************************************************************************//**
*
*  Changes the HSIOM settings for the UART output pins (TX and/or RTS) to keep
*  them inactive after the block is disabled. The output pins are controlled by
*  the GPIO data register. Also, the function disables the skip start feature
*  to not cause it to trigger after the component is enabled.
*
*******************************************************************************/
void BARCODE_UartStop(void)
{
#if(BARCODE_SCB_MODE_UNCONFIG_CONST_CFG)
    #if (BARCODE_TX_SDA_MISO_PIN)
        if (BARCODE_CHECK_TX_SDA_MISO_PIN_USED)
        {
            /* Set GPIO to drive output pin */
            BARCODE_SET_HSIOM_SEL(BARCODE_TX_SDA_MISO_HSIOM_REG, BARCODE_TX_SDA_MISO_HSIOM_MASK,
                                           BARCODE_TX_SDA_MISO_HSIOM_POS, BARCODE_TX_SDA_MISO_HSIOM_SEL_GPIO);
        }
    #endif /* (BARCODE_TX_SDA_MISO_PIN_PIN) */

    #if !(BARCODE_CY_SCBIP_V0 || BARCODE_CY_SCBIP_V1)
        #if (BARCODE_RTS_SS0_PIN)
            if (BARCODE_CHECK_RTS_SS0_PIN_USED)
            {
                /* Set output pin state after block is disabled */
                BARCODE_uart_rts_spi_ss0_Write(BARCODE_GET_UART_RTS_INACTIVE);

                /* Set GPIO to drive output pin */
                BARCODE_SET_HSIOM_SEL(BARCODE_RTS_SS0_HSIOM_REG, BARCODE_RTS_SS0_HSIOM_MASK,
                                               BARCODE_RTS_SS0_HSIOM_POS, BARCODE_RTS_SS0_HSIOM_SEL_GPIO);
            }
        #endif /* (BARCODE_RTS_SS0_PIN) */
    #endif /* !(BARCODE_CY_SCBIP_V0 || BARCODE_CY_SCBIP_V1) */

#else
    #if (BARCODE_UART_TX_PIN)
        /* Set GPIO to drive output pin */
        BARCODE_SET_HSIOM_SEL(BARCODE_TX_HSIOM_REG, BARCODE_TX_HSIOM_MASK,
                                       BARCODE_TX_HSIOM_POS, BARCODE_TX_HSIOM_SEL_GPIO);
    #endif /* (BARCODE_UART_TX_PIN) */

    #if (BARCODE_UART_RTS_PIN)
        /* Set output pin state after block is disabled */
        BARCODE_rts_Write(BARCODE_GET_UART_RTS_INACTIVE);

        /* Set GPIO to drive output pin */
        BARCODE_SET_HSIOM_SEL(BARCODE_RTS_HSIOM_REG, BARCODE_RTS_HSIOM_MASK,
                                       BARCODE_RTS_HSIOM_POS, BARCODE_RTS_HSIOM_SEL_GPIO);
    #endif /* (BARCODE_UART_RTS_PIN) */

#endif /* (BARCODE_SCB_MODE_UNCONFIG_CONST_CFG) */

#if (BARCODE_UART_WAKE_ENABLE_CONST)
    /* Disable skip start feature used for wakeup */
    BARCODE_UART_RX_CTRL_REG &= (uint32) ~BARCODE_UART_RX_CTRL_SKIP_START;
#endif /* (BARCODE_UART_WAKE_ENABLE_CONST) */

    /* Store TX interrupt sources (exclude level triggered). */
    BARCODE_IntrTxMask = LO16(BARCODE_GetTxInterruptMode() & BARCODE_INTR_UART_TX_RESTORE);
}


/*******************************************************************************
* Function Name: BARCODE_UartSetRxAddress
****************************************************************************//**
*
*  Sets the hardware detectable receiver address for the UART in the
*  Multiprocessor mode.
*
*  \param address: Address for hardware address detection.
*
*******************************************************************************/
void BARCODE_UartSetRxAddress(uint32 address)
{
     uint32 matchReg;

    matchReg = BARCODE_RX_MATCH_REG;

    matchReg &= ((uint32) ~BARCODE_RX_MATCH_ADDR_MASK); /* Clear address bits */
    matchReg |= ((uint32)  (address & BARCODE_RX_MATCH_ADDR_MASK)); /* Set address  */

    BARCODE_RX_MATCH_REG = matchReg;
}


/*******************************************************************************
* Function Name: BARCODE_UartSetRxAddressMask
****************************************************************************//**
*
*  Sets the hardware address mask for the UART in the Multiprocessor mode.
*
*  \param addressMask: Address mask.
*   - Bit value 0 – excludes bit from address comparison.
*   - Bit value 1 – the bit needs to match with the corresponding bit
*     of the address.
*
*******************************************************************************/
void BARCODE_UartSetRxAddressMask(uint32 addressMask)
{
    uint32 matchReg;

    matchReg = BARCODE_RX_MATCH_REG;

    matchReg &= ((uint32) ~BARCODE_RX_MATCH_MASK_MASK); /* Clear address mask bits */
    matchReg |= ((uint32) (addressMask << BARCODE_RX_MATCH_MASK_POS));

    BARCODE_RX_MATCH_REG = matchReg;
}


#if(BARCODE_UART_RX_DIRECTION)
    /*******************************************************************************
    * Function Name: BARCODE_UartGetChar
    ****************************************************************************//**
    *
    *  Retrieves next data element from receive buffer.
    *  This function is designed for ASCII characters and returns a char where
    *  1 to 255 are valid characters and 0 indicates an error occurred or no data
    *  is present.
    *  - RX software buffer is disabled: Returns data element retrieved from RX
    *    FIFO.
    *  - RX software buffer is enabled: Returns data element from the software
    *    receive buffer.
    *
    *  \return
    *   Next data element from the receive buffer. ASCII character values from
    *   1 to 255 are valid. A returned zero signifies an error condition or no
    *   data available.
    *
    *  \sideeffect
    *   The errors bits may not correspond with reading characters due to
    *   RX FIFO and software buffer usage.
    *   RX software buffer is enabled: The internal software buffer overflow
    *   is not treated as an error condition.
    *   Check BARCODE_rxBufferOverflow to capture that error condition.
    *
    *******************************************************************************/
    uint32 BARCODE_UartGetChar(void)
    {
        uint32 rxData = 0u;

        /* Reads data only if there is data to read */
        if (0u != BARCODE_SpiUartGetRxBufferSize())
        {
            rxData = BARCODE_SpiUartReadRxData();
        }

        if (BARCODE_CHECK_INTR_RX(BARCODE_INTR_RX_ERR))
        {
            rxData = 0u; /* Error occurred: returns zero */
            BARCODE_ClearRxInterruptSource(BARCODE_INTR_RX_ERR);
        }

        return (rxData);
    }


    /*******************************************************************************
    * Function Name: BARCODE_UartGetByte
    ****************************************************************************//**
    *
    *  Retrieves the next data element from the receive buffer, returns the
    *  received byte and error condition.
    *   - The RX software buffer is disabled: returns the data element retrieved
    *     from the RX FIFO. Undefined data will be returned if the RX FIFO is
    *     empty.
    *   - The RX software buffer is enabled: returns data element from the
    *     software receive buffer.
    *
    *  \return
    *   Bits 7-0 contain the next data element from the receive buffer and
    *   other bits contain the error condition.
    *   - BARCODE_UART_RX_OVERFLOW - Attempt to write to a full
    *     receiver FIFO.
    *   - BARCODE_UART_RX_UNDERFLOW    Attempt to read from an empty
    *     receiver FIFO.
    *   - BARCODE_UART_RX_FRAME_ERROR - UART framing error detected.
    *   - BARCODE_UART_RX_PARITY_ERROR - UART parity error detected.
    *
    *  \sideeffect
    *   The errors bits may not correspond with reading characters due to
    *   RX FIFO and software buffer usage.
    *   RX software buffer is enabled: The internal software buffer overflow
    *   is not treated as an error condition.
    *   Check BARCODE_rxBufferOverflow to capture that error condition.
    *
    *******************************************************************************/
    uint32 BARCODE_UartGetByte(void)
    {
        uint32 rxData;
        uint32 tmpStatus;

        #if (BARCODE_CHECK_RX_SW_BUFFER)
        {
            BARCODE_DisableInt();
        }
        #endif

        if (0u != BARCODE_SpiUartGetRxBufferSize())
        {
            /* Enables interrupt to receive more bytes: at least one byte is in
            * buffer.
            */
            #if (BARCODE_CHECK_RX_SW_BUFFER)
            {
                BARCODE_EnableInt();
            }
            #endif

            /* Get received byte */
            rxData = BARCODE_SpiUartReadRxData();
        }
        else
        {
            /* Reads a byte directly from RX FIFO: underflow is raised in the
            * case of empty. Otherwise the first received byte will be read.
            */
            rxData = BARCODE_RX_FIFO_RD_REG;


            /* Enables interrupt to receive more bytes. */
            #if (BARCODE_CHECK_RX_SW_BUFFER)
            {

                /* The byte has been read from RX FIFO. Clear RX interrupt to
                * not involve interrupt handler when RX FIFO is empty.
                */
                BARCODE_ClearRxInterruptSource(BARCODE_INTR_RX_NOT_EMPTY);

                BARCODE_EnableInt();
            }
            #endif
        }

        /* Get and clear RX error mask */
        tmpStatus = (BARCODE_GetRxInterruptSource() & BARCODE_INTR_RX_ERR);
        BARCODE_ClearRxInterruptSource(BARCODE_INTR_RX_ERR);

        /* Puts together data and error status:
        * MP mode and accept address: 9th bit is set to notify mark.
        */
        rxData |= ((uint32) (tmpStatus << 8u));

        return (rxData);
    }


    #if !(BARCODE_CY_SCBIP_V0 || BARCODE_CY_SCBIP_V1)
        /*******************************************************************************
        * Function Name: BARCODE_UartSetRtsPolarity
        ****************************************************************************//**
        *
        *  Sets active polarity of RTS output signal.
        *  Only available for PSoC 4100 BLE / PSoC 4200 BLE / PSoC 4100M / PSoC 4200M /
        *  PSoC 4200L / PSoC 4000S / PSoC 4100S / PSoC Analog Coprocessor devices.
        *
        *  \param polarity: Active polarity of RTS output signal.
        *   - BARCODE_UART_RTS_ACTIVE_LOW  - RTS signal is active low.
        *   - BARCODE_UART_RTS_ACTIVE_HIGH - RTS signal is active high.
        *
        *******************************************************************************/
        void BARCODE_UartSetRtsPolarity(uint32 polarity)
        {
            if(0u != polarity)
            {
                BARCODE_UART_FLOW_CTRL_REG |= (uint32)  BARCODE_UART_FLOW_CTRL_RTS_POLARITY;
            }
            else
            {
                BARCODE_UART_FLOW_CTRL_REG &= (uint32) ~BARCODE_UART_FLOW_CTRL_RTS_POLARITY;
            }
        }


        /*******************************************************************************
        * Function Name: BARCODE_UartSetRtsFifoLevel
        ****************************************************************************//**
        *
        *  Sets level in the RX FIFO for RTS signal activation.
        *  While the RX FIFO has fewer entries than the RX FIFO level the RTS signal
        *  remains active, otherwise the RTS signal becomes inactive.
        *  Only available for PSoC 4100 BLE / PSoC 4200 BLE / PSoC 4100M / PSoC 4200M /
        *  PSoC 4200L / PSoC 4000S / PSoC 4100S / PSoC Analog Coprocessor devices.
        *
        *  \param level: Level in the RX FIFO for RTS signal activation.
        *   The range of valid level values is between 0 and RX FIFO depth - 1.
        *   Setting level value to 0 disables RTS signal activation.
        *
        *******************************************************************************/
        void BARCODE_UartSetRtsFifoLevel(uint32 level)
        {
            uint32 uartFlowCtrl;

            uartFlowCtrl = BARCODE_UART_FLOW_CTRL_REG;

            uartFlowCtrl &= ((uint32) ~BARCODE_UART_FLOW_CTRL_TRIGGER_LEVEL_MASK); /* Clear level mask bits */
            uartFlowCtrl |= ((uint32) (BARCODE_UART_FLOW_CTRL_TRIGGER_LEVEL_MASK & level));

            BARCODE_UART_FLOW_CTRL_REG = uartFlowCtrl;
        }
    #endif /* !(BARCODE_CY_SCBIP_V0 || BARCODE_CY_SCBIP_V1) */

#endif /* (BARCODE_UART_RX_DIRECTION) */


#if(BARCODE_UART_TX_DIRECTION)
    /*******************************************************************************
    * Function Name: BARCODE_UartPutString
    ****************************************************************************//**
    *
    *  Places a NULL terminated string in the transmit buffer to be sent at the
    *  next available bus time.
    *  This function is blocking and waits until there is a space available to put
    *  requested data in transmit buffer.
    *
    *  \param string: pointer to the null terminated string array to be placed in the
    *   transmit buffer.
    *
    *******************************************************************************/
    void BARCODE_UartPutString(const char8 string[])
    {
        uint32 bufIndex;

        bufIndex = 0u;

        /* Blocks the control flow until all data has been sent */
        while(string[bufIndex] != ((char8) 0))
        {
            BARCODE_UartPutChar((uint32) string[bufIndex]);
            bufIndex++;
        }
    }


    /*******************************************************************************
    * Function Name: BARCODE_UartPutCRLF
    ****************************************************************************//**
    *
    *  Places byte of data followed by a carriage return (0x0D) and line feed
    *  (0x0A) in the transmit buffer.
    *  This function is blocking and waits until there is a space available to put
    *  all requested data in transmit buffer.
    *
    *  \param txDataByte: the data to be transmitted.
    *
    *******************************************************************************/
    void BARCODE_UartPutCRLF(uint32 txDataByte)
    {
        BARCODE_UartPutChar(txDataByte);  /* Blocks control flow until all data has been sent */
        BARCODE_UartPutChar(0x0Du);       /* Blocks control flow until all data has been sent */
        BARCODE_UartPutChar(0x0Au);       /* Blocks control flow until all data has been sent */
    }


    #if !(BARCODE_CY_SCBIP_V0 || BARCODE_CY_SCBIP_V1)
        /*******************************************************************************
        * Function Name: BARCODESCB_UartEnableCts
        ****************************************************************************//**
        *
        *  Enables usage of CTS input signal by the UART transmitter.
        *  Only available for PSoC 4100 BLE / PSoC 4200 BLE / PSoC 4100M / PSoC 4200M /
        *  PSoC 4200L / PSoC 4000S / PSoC 4100S / PSoC Analog Coprocessor devices.
        *
        *******************************************************************************/
        void BARCODE_UartEnableCts(void)
        {
            BARCODE_UART_FLOW_CTRL_REG |= (uint32)  BARCODE_UART_FLOW_CTRL_CTS_ENABLE;
        }


        /*******************************************************************************
        * Function Name: BARCODE_UartDisableCts
        ****************************************************************************//**
        *
        *  Disables usage of CTS input signal by the UART transmitter.
        *  Only available for PSoC 4100 BLE / PSoC 4200 BLE / PSoC 4100M / PSoC 4200M /
        *  PSoC 4200L / PSoC 4000S / PSoC 4100S / PSoC Analog Coprocessor devices.
        *
        *******************************************************************************/
        void BARCODE_UartDisableCts(void)
        {
            BARCODE_UART_FLOW_CTRL_REG &= (uint32) ~BARCODE_UART_FLOW_CTRL_CTS_ENABLE;
        }


        /*******************************************************************************
        * Function Name: BARCODE_UartSetCtsPolarity
        ****************************************************************************//**
        *
        *  Sets active polarity of CTS input signal.
        *  Only available for PSoC 4100 BLE / PSoC 4200 BLE / PSoC 4100M / PSoC 4200M /
        *  PSoC 4200L / PSoC 4000S / PSoC 4100S / PSoC Analog Coprocessor devices.
        *
        * \param
        * polarity: Active polarity of CTS output signal.
        *   - BARCODE_UART_CTS_ACTIVE_LOW  - CTS signal is active low.
        *   - BARCODE_UART_CTS_ACTIVE_HIGH - CTS signal is active high.
        *
        *******************************************************************************/
        void BARCODE_UartSetCtsPolarity(uint32 polarity)
        {
            if (0u != polarity)
            {
                BARCODE_UART_FLOW_CTRL_REG |= (uint32)  BARCODE_UART_FLOW_CTRL_CTS_POLARITY;
            }
            else
            {
                BARCODE_UART_FLOW_CTRL_REG &= (uint32) ~BARCODE_UART_FLOW_CTRL_CTS_POLARITY;
            }
        }
    #endif /* !(BARCODE_CY_SCBIP_V0 || BARCODE_CY_SCBIP_V1) */


    /*******************************************************************************
    * Function Name: BARCODE_UartSendBreakBlocking
    ****************************************************************************//**
    *
    * Sends a break condition (logic low) of specified width on UART TX line.
    * Blocks until break is completed. Only call this function when UART TX FIFO
    * and shifter are empty.
    *
    * \param breakWidth
    * Width of break condition. Valid range is 4 to 16 bits.
    *
    * \note
    * Before sending break all UART TX interrupt sources are disabled. The state
    * of UART TX interrupt sources is restored before function returns.
    *
    * \sideeffect
    * If this function is called while there is data in the TX FIFO or shifter that
    * data will be shifted out in packets the size of breakWidth.
    *
    *******************************************************************************/
    void BARCODE_UartSendBreakBlocking(uint32 breakWidth)
    {
        uint32 txCtrlReg;
        uint32 txIntrReg;

        /* Disable all UART TX interrupt source and clear UART TX Done history */
        txIntrReg = BARCODE_GetTxInterruptMode();
        BARCODE_SetTxInterruptMode(0u);
        BARCODE_ClearTxInterruptSource(BARCODE_INTR_TX_UART_DONE);

        /* Store TX CTRL configuration */
        txCtrlReg = BARCODE_TX_CTRL_REG;

        /* Set break width */
        BARCODE_TX_CTRL_REG = (BARCODE_TX_CTRL_REG & (uint32) ~BARCODE_TX_CTRL_DATA_WIDTH_MASK) |
                                        BARCODE_GET_TX_CTRL_DATA_WIDTH(breakWidth);

        /* Generate break */
        BARCODE_TX_FIFO_WR_REG = 0u;

        /* Wait for break completion */
        while (0u == (BARCODE_GetTxInterruptSource() & BARCODE_INTR_TX_UART_DONE))
        {
        }

        /* Clear all UART TX interrupt sources to  */
        BARCODE_ClearTxInterruptSource(BARCODE_INTR_TX_ALL);

        /* Restore TX interrupt sources and data width */
        BARCODE_TX_CTRL_REG = txCtrlReg;
        BARCODE_SetTxInterruptMode(txIntrReg);
    }
#endif /* (BARCODE_UART_TX_DIRECTION) */


#if (BARCODE_UART_WAKE_ENABLE_CONST)
    /*******************************************************************************
    * Function Name: BARCODE_UartSaveConfig
    ****************************************************************************//**
    *
    *  Clears and enables an interrupt on a falling edge of the Rx input. The GPIO
    *  interrupt does not track in the active mode, therefore requires to be
    *  cleared by this API.
    *
    *******************************************************************************/
    void BARCODE_UartSaveConfig(void)
    {
    #if (BARCODE_UART_RX_WAKEUP_IRQ)
        /* Set SKIP_START if requested (set by default). */
        if (0u != BARCODE_skipStart)
        {
            BARCODE_UART_RX_CTRL_REG |= (uint32)  BARCODE_UART_RX_CTRL_SKIP_START;
        }
        else
        {
            BARCODE_UART_RX_CTRL_REG &= (uint32) ~BARCODE_UART_RX_CTRL_SKIP_START;
        }

        /* Clear RX GPIO interrupt status and pending interrupt in NVIC because
        * falling edge on RX line occurs while UART communication in active mode.
        * Enable interrupt: next interrupt trigger should wakeup device.
        */
        BARCODE_CLEAR_UART_RX_WAKE_INTR;
        BARCODE_RxWakeClearPendingInt();
        BARCODE_RxWakeEnableInt();
    #endif /* (BARCODE_UART_RX_WAKEUP_IRQ) */
    }


    /*******************************************************************************
    * Function Name: BARCODE_UartRestoreConfig
    ****************************************************************************//**
    *
    *  Disables the RX GPIO interrupt. Until this function is called the interrupt
    *  remains active and triggers on every falling edge of the UART RX line.
    *
    *******************************************************************************/
    void BARCODE_UartRestoreConfig(void)
    {
    #if (BARCODE_UART_RX_WAKEUP_IRQ)
        /* Disable interrupt: no more triggers in active mode */
        BARCODE_RxWakeDisableInt();
    #endif /* (BARCODE_UART_RX_WAKEUP_IRQ) */
    }


    #if (BARCODE_UART_RX_WAKEUP_IRQ)
        /*******************************************************************************
        * Function Name: BARCODE_UART_WAKEUP_ISR
        ****************************************************************************//**
        *
        *  Handles the Interrupt Service Routine for the SCB UART mode GPIO wakeup
        *  event. This event is configured to trigger on a falling edge of the RX line.
        *
        *******************************************************************************/
        CY_ISR(BARCODE_UART_WAKEUP_ISR)
        {
        #ifdef BARCODE_UART_WAKEUP_ISR_ENTRY_CALLBACK
            BARCODE_UART_WAKEUP_ISR_EntryCallback();
        #endif /* BARCODE_UART_WAKEUP_ISR_ENTRY_CALLBACK */

            BARCODE_CLEAR_UART_RX_WAKE_INTR;

        #ifdef BARCODE_UART_WAKEUP_ISR_EXIT_CALLBACK
            BARCODE_UART_WAKEUP_ISR_ExitCallback();
        #endif /* BARCODE_UART_WAKEUP_ISR_EXIT_CALLBACK */
        }
    #endif /* (BARCODE_UART_RX_WAKEUP_IRQ) */
#endif /* (BARCODE_UART_RX_WAKEUP_IRQ) */


/* [] END OF FILE */
