/***************************************************************************//**
* \file WIFI_UART.c
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

#include "WIFI_PVT.h"
#include "WIFI_SPI_UART_PVT.h"
#include "cyapicallbacks.h"

#if (WIFI_UART_WAKE_ENABLE_CONST && WIFI_UART_RX_WAKEUP_IRQ)
    /**
    * \addtogroup group_globals
    * \{
    */
    /** This global variable determines whether to enable Skip Start
    * functionality when WIFI_Sleep() function is called:
    * 0 – disable, other values – enable. Default value is 1.
    * It is only available when Enable wakeup from Deep Sleep Mode is enabled.
    */
    uint8 WIFI_skipStart = 1u;
    /** \} globals */
#endif /* (WIFI_UART_WAKE_ENABLE_CONST && WIFI_UART_RX_WAKEUP_IRQ) */

#if(WIFI_SCB_MODE_UNCONFIG_CONST_CFG)

    /***************************************
    *  Configuration Structure Initialization
    ***************************************/

    const WIFI_UART_INIT_STRUCT WIFI_configUart =
    {
        WIFI_UART_SUB_MODE,
        WIFI_UART_DIRECTION,
        WIFI_UART_DATA_BITS_NUM,
        WIFI_UART_PARITY_TYPE,
        WIFI_UART_STOP_BITS_NUM,
        WIFI_UART_OVS_FACTOR,
        WIFI_UART_IRDA_LOW_POWER,
        WIFI_UART_MEDIAN_FILTER_ENABLE,
        WIFI_UART_RETRY_ON_NACK,
        WIFI_UART_IRDA_POLARITY,
        WIFI_UART_DROP_ON_PARITY_ERR,
        WIFI_UART_DROP_ON_FRAME_ERR,
        WIFI_UART_WAKE_ENABLE,
        0u,
        NULL,
        0u,
        NULL,
        WIFI_UART_MP_MODE_ENABLE,
        WIFI_UART_MP_ACCEPT_ADDRESS,
        WIFI_UART_MP_RX_ADDRESS,
        WIFI_UART_MP_RX_ADDRESS_MASK,
        (uint32) WIFI_SCB_IRQ_INTERNAL,
        WIFI_UART_INTR_RX_MASK,
        WIFI_UART_RX_TRIGGER_LEVEL,
        WIFI_UART_INTR_TX_MASK,
        WIFI_UART_TX_TRIGGER_LEVEL,
        (uint8) WIFI_UART_BYTE_MODE_ENABLE,
        (uint8) WIFI_UART_CTS_ENABLE,
        (uint8) WIFI_UART_CTS_POLARITY,
        (uint8) WIFI_UART_RTS_POLARITY,
        (uint8) WIFI_UART_RTS_FIFO_LEVEL,
        (uint8) WIFI_UART_RX_BREAK_WIDTH
    };


    /*******************************************************************************
    * Function Name: WIFI_UartInit
    ****************************************************************************//**
    *
    *  Configures the WIFI for UART operation.
    *
    *  This function is intended specifically to be used when the WIFI
    *  configuration is set to “Unconfigured WIFI” in the customizer.
    *  After initializing the WIFI in UART mode using this function,
    *  the component can be enabled using the WIFI_Start() or
    * WIFI_Enable() function.
    *  This function uses a pointer to a structure that provides the configuration
    *  settings. This structure contains the same information that would otherwise
    *  be provided by the customizer settings.
    *
    *  \param config: pointer to a structure that contains the following list of
    *   fields. These fields match the selections available in the customizer.
    *   Refer to the customizer for further description of the settings.
    *
    *******************************************************************************/
    void WIFI_UartInit(const WIFI_UART_INIT_STRUCT *config)
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

        #if !(WIFI_CY_SCBIP_V0 || WIFI_CY_SCBIP_V1)
            /* Add RTS and CTS pins to configure */
            pinsConfig |= (0u != config->rtsRxFifoLevel) ? (WIFI_UART_RTS_PIN_ENABLE) : (0u);
            pinsConfig |= (0u != config->enableCts)      ? (WIFI_UART_CTS_PIN_ENABLE) : (0u);
        #endif /* !(WIFI_CY_SCBIP_V0 || WIFI_CY_SCBIP_V1) */

            /* Configure pins */
            WIFI_SetPins(WIFI_SCB_MODE_UART, config->mode, pinsConfig);

            /* Store internal configuration */
            WIFI_scbMode       = (uint8) WIFI_SCB_MODE_UART;
            WIFI_scbEnableWake = (uint8) config->enableWake;
            WIFI_scbEnableIntr = (uint8) config->enableInterrupt;

            /* Set RX direction internal variables */
            WIFI_rxBuffer      =         config->rxBuffer;
            WIFI_rxDataBits    = (uint8) config->dataBits;
            WIFI_rxBufferSize  =         config->rxBufferSize;

            /* Set TX direction internal variables */
            WIFI_txBuffer      =         config->txBuffer;
            WIFI_txDataBits    = (uint8) config->dataBits;
            WIFI_txBufferSize  =         config->txBufferSize;

            /* Configure UART interface */
            if(WIFI_UART_MODE_IRDA == config->mode)
            {
                /* OVS settings: IrDA */
                WIFI_CTRL_REG  = ((0u != config->enableIrdaLowPower) ?
                                                (WIFI_UART_GET_CTRL_OVS_IRDA_LP(config->oversample)) :
                                                (WIFI_CTRL_OVS_IRDA_OVS16));
            }
            else
            {
                /* OVS settings: UART and SmartCard */
                WIFI_CTRL_REG  = WIFI_GET_CTRL_OVS(config->oversample);
            }

            WIFI_CTRL_REG     |= WIFI_GET_CTRL_BYTE_MODE  (config->enableByteMode)      |
                                             WIFI_GET_CTRL_ADDR_ACCEPT(config->multiprocAcceptAddr) |
                                             WIFI_CTRL_UART;

            /* Configure sub-mode: UART, SmartCard or IrDA */
            WIFI_UART_CTRL_REG = WIFI_GET_UART_CTRL_MODE(config->mode);

            /* Configure RX direction */
            WIFI_UART_RX_CTRL_REG = WIFI_GET_UART_RX_CTRL_MODE(config->stopBits)              |
                                        WIFI_GET_UART_RX_CTRL_POLARITY(config->enableInvertedRx)          |
                                        WIFI_GET_UART_RX_CTRL_MP_MODE(config->enableMultiproc)            |
                                        WIFI_GET_UART_RX_CTRL_DROP_ON_PARITY_ERR(config->dropOnParityErr) |
                                        WIFI_GET_UART_RX_CTRL_DROP_ON_FRAME_ERR(config->dropOnFrameErr)   |
                                        WIFI_GET_UART_RX_CTRL_BREAK_WIDTH(config->breakWidth);

            if(WIFI_UART_PARITY_NONE != config->parity)
            {
               WIFI_UART_RX_CTRL_REG |= WIFI_GET_UART_RX_CTRL_PARITY(config->parity) |
                                                    WIFI_UART_RX_CTRL_PARITY_ENABLED;
            }

            WIFI_RX_CTRL_REG      = WIFI_GET_RX_CTRL_DATA_WIDTH(config->dataBits)       |
                                                WIFI_GET_RX_CTRL_MEDIAN(config->enableMedianFilter) |
                                                WIFI_GET_UART_RX_CTRL_ENABLED(config->direction);

            WIFI_RX_FIFO_CTRL_REG = WIFI_GET_RX_FIFO_CTRL_TRIGGER_LEVEL(config->rxTriggerLevel);

            /* Configure MP address */
            WIFI_RX_MATCH_REG     = WIFI_GET_RX_MATCH_ADDR(config->multiprocAddr) |
                                                WIFI_GET_RX_MATCH_MASK(config->multiprocAddrMask);

            /* Configure RX direction */
            WIFI_UART_TX_CTRL_REG = WIFI_GET_UART_TX_CTRL_MODE(config->stopBits) |
                                                WIFI_GET_UART_TX_CTRL_RETRY_NACK(config->enableRetryNack);

            if(WIFI_UART_PARITY_NONE != config->parity)
            {
               WIFI_UART_TX_CTRL_REG |= WIFI_GET_UART_TX_CTRL_PARITY(config->parity) |
                                                    WIFI_UART_TX_CTRL_PARITY_ENABLED;
            }

            WIFI_TX_CTRL_REG      = WIFI_GET_TX_CTRL_DATA_WIDTH(config->dataBits)    |
                                                WIFI_GET_UART_TX_CTRL_ENABLED(config->direction);

            WIFI_TX_FIFO_CTRL_REG = WIFI_GET_TX_FIFO_CTRL_TRIGGER_LEVEL(config->txTriggerLevel);

        #if !(WIFI_CY_SCBIP_V0 || WIFI_CY_SCBIP_V1)
            WIFI_UART_FLOW_CTRL_REG = WIFI_GET_UART_FLOW_CTRL_CTS_ENABLE(config->enableCts) | \
                                            WIFI_GET_UART_FLOW_CTRL_CTS_POLARITY (config->ctsPolarity)  | \
                                            WIFI_GET_UART_FLOW_CTRL_RTS_POLARITY (config->rtsPolarity)  | \
                                            WIFI_GET_UART_FLOW_CTRL_TRIGGER_LEVEL(config->rtsRxFifoLevel);
        #endif /* !(WIFI_CY_SCBIP_V0 || WIFI_CY_SCBIP_V1) */

            /* Configure interrupt with UART handler but do not enable it */
            CyIntDisable    (WIFI_ISR_NUMBER);
            CyIntSetPriority(WIFI_ISR_NUMBER, WIFI_ISR_PRIORITY);
            (void) CyIntSetVector(WIFI_ISR_NUMBER, &WIFI_SPI_UART_ISR);

            /* Configure WAKE interrupt */
        #if(WIFI_UART_RX_WAKEUP_IRQ)
            CyIntDisable    (WIFI_RX_WAKE_ISR_NUMBER);
            CyIntSetPriority(WIFI_RX_WAKE_ISR_NUMBER, WIFI_RX_WAKE_ISR_PRIORITY);
            (void) CyIntSetVector(WIFI_RX_WAKE_ISR_NUMBER, &WIFI_UART_WAKEUP_ISR);
        #endif /* (WIFI_UART_RX_WAKEUP_IRQ) */

            /* Configure interrupt sources */
            WIFI_INTR_I2C_EC_MASK_REG = WIFI_NO_INTR_SOURCES;
            WIFI_INTR_SPI_EC_MASK_REG = WIFI_NO_INTR_SOURCES;
            WIFI_INTR_SLAVE_MASK_REG  = WIFI_NO_INTR_SOURCES;
            WIFI_INTR_MASTER_MASK_REG = WIFI_NO_INTR_SOURCES;
            WIFI_INTR_RX_MASK_REG     = config->rxInterruptMask;
            WIFI_INTR_TX_MASK_REG     = config->txInterruptMask;

            /* Configure TX interrupt sources to restore. */
            WIFI_IntrTxMask = LO16(WIFI_INTR_TX_MASK_REG);

            /* Clear RX buffer indexes */
            WIFI_rxBufferHead     = 0u;
            WIFI_rxBufferTail     = 0u;
            WIFI_rxBufferOverflow = 0u;

            /* Clear TX buffer indexes */
            WIFI_txBufferHead = 0u;
            WIFI_txBufferTail = 0u;
        }
    }

#else

    /*******************************************************************************
    * Function Name: WIFI_UartInit
    ****************************************************************************//**
    *
    *  Configures the SCB for the UART operation.
    *
    *******************************************************************************/
    void WIFI_UartInit(void)
    {
        /* Configure UART interface */
        WIFI_CTRL_REG = WIFI_UART_DEFAULT_CTRL;

        /* Configure sub-mode: UART, SmartCard or IrDA */
        WIFI_UART_CTRL_REG = WIFI_UART_DEFAULT_UART_CTRL;

        /* Configure RX direction */
        WIFI_UART_RX_CTRL_REG = WIFI_UART_DEFAULT_UART_RX_CTRL;
        WIFI_RX_CTRL_REG      = WIFI_UART_DEFAULT_RX_CTRL;
        WIFI_RX_FIFO_CTRL_REG = WIFI_UART_DEFAULT_RX_FIFO_CTRL;
        WIFI_RX_MATCH_REG     = WIFI_UART_DEFAULT_RX_MATCH_REG;

        /* Configure TX direction */
        WIFI_UART_TX_CTRL_REG = WIFI_UART_DEFAULT_UART_TX_CTRL;
        WIFI_TX_CTRL_REG      = WIFI_UART_DEFAULT_TX_CTRL;
        WIFI_TX_FIFO_CTRL_REG = WIFI_UART_DEFAULT_TX_FIFO_CTRL;

    #if !(WIFI_CY_SCBIP_V0 || WIFI_CY_SCBIP_V1)
        WIFI_UART_FLOW_CTRL_REG = WIFI_UART_DEFAULT_FLOW_CTRL;
    #endif /* !(WIFI_CY_SCBIP_V0 || WIFI_CY_SCBIP_V1) */

        /* Configure interrupt with UART handler but do not enable it */
    #if(WIFI_SCB_IRQ_INTERNAL)
        CyIntDisable    (WIFI_ISR_NUMBER);
        CyIntSetPriority(WIFI_ISR_NUMBER, WIFI_ISR_PRIORITY);
        (void) CyIntSetVector(WIFI_ISR_NUMBER, &WIFI_SPI_UART_ISR);
    #endif /* (WIFI_SCB_IRQ_INTERNAL) */

        /* Configure WAKE interrupt */
    #if(WIFI_UART_RX_WAKEUP_IRQ)
        CyIntDisable    (WIFI_RX_WAKE_ISR_NUMBER);
        CyIntSetPriority(WIFI_RX_WAKE_ISR_NUMBER, WIFI_RX_WAKE_ISR_PRIORITY);
        (void) CyIntSetVector(WIFI_RX_WAKE_ISR_NUMBER, &WIFI_UART_WAKEUP_ISR);
    #endif /* (WIFI_UART_RX_WAKEUP_IRQ) */

        /* Configure interrupt sources */
        WIFI_INTR_I2C_EC_MASK_REG = WIFI_UART_DEFAULT_INTR_I2C_EC_MASK;
        WIFI_INTR_SPI_EC_MASK_REG = WIFI_UART_DEFAULT_INTR_SPI_EC_MASK;
        WIFI_INTR_SLAVE_MASK_REG  = WIFI_UART_DEFAULT_INTR_SLAVE_MASK;
        WIFI_INTR_MASTER_MASK_REG = WIFI_UART_DEFAULT_INTR_MASTER_MASK;
        WIFI_INTR_RX_MASK_REG     = WIFI_UART_DEFAULT_INTR_RX_MASK;
        WIFI_INTR_TX_MASK_REG     = WIFI_UART_DEFAULT_INTR_TX_MASK;

        /* Configure TX interrupt sources to restore. */
        WIFI_IntrTxMask = LO16(WIFI_INTR_TX_MASK_REG);

    #if(WIFI_INTERNAL_RX_SW_BUFFER_CONST)
        WIFI_rxBufferHead     = 0u;
        WIFI_rxBufferTail     = 0u;
        WIFI_rxBufferOverflow = 0u;
    #endif /* (WIFI_INTERNAL_RX_SW_BUFFER_CONST) */

    #if(WIFI_INTERNAL_TX_SW_BUFFER_CONST)
        WIFI_txBufferHead = 0u;
        WIFI_txBufferTail = 0u;
    #endif /* (WIFI_INTERNAL_TX_SW_BUFFER_CONST) */
    }
#endif /* (WIFI_SCB_MODE_UNCONFIG_CONST_CFG) */


/*******************************************************************************
* Function Name: WIFI_UartPostEnable
****************************************************************************//**
*
*  Restores HSIOM settings for the UART output pins (TX and/or RTS) to be
*  controlled by the SCB UART.
*
*******************************************************************************/
void WIFI_UartPostEnable(void)
{
#if (WIFI_SCB_MODE_UNCONFIG_CONST_CFG)
    #if (WIFI_TX_SDA_MISO_PIN)
        if (WIFI_CHECK_TX_SDA_MISO_PIN_USED)
        {
            /* Set SCB UART to drive the output pin */
            WIFI_SET_HSIOM_SEL(WIFI_TX_SDA_MISO_HSIOM_REG, WIFI_TX_SDA_MISO_HSIOM_MASK,
                                           WIFI_TX_SDA_MISO_HSIOM_POS, WIFI_TX_SDA_MISO_HSIOM_SEL_UART);
        }
    #endif /* (WIFI_TX_SDA_MISO_PIN_PIN) */

    #if !(WIFI_CY_SCBIP_V0 || WIFI_CY_SCBIP_V1)
        #if (WIFI_RTS_SS0_PIN)
            if (WIFI_CHECK_RTS_SS0_PIN_USED)
            {
                /* Set SCB UART to drive the output pin */
                WIFI_SET_HSIOM_SEL(WIFI_RTS_SS0_HSIOM_REG, WIFI_RTS_SS0_HSIOM_MASK,
                                               WIFI_RTS_SS0_HSIOM_POS, WIFI_RTS_SS0_HSIOM_SEL_UART);
            }
        #endif /* (WIFI_RTS_SS0_PIN) */
    #endif /* !(WIFI_CY_SCBIP_V0 || WIFI_CY_SCBIP_V1) */

#else
    #if (WIFI_UART_TX_PIN)
         /* Set SCB UART to drive the output pin */
        WIFI_SET_HSIOM_SEL(WIFI_TX_HSIOM_REG, WIFI_TX_HSIOM_MASK,
                                       WIFI_TX_HSIOM_POS, WIFI_TX_HSIOM_SEL_UART);
    #endif /* (WIFI_UART_TX_PIN) */

    #if (WIFI_UART_RTS_PIN)
        /* Set SCB UART to drive the output pin */
        WIFI_SET_HSIOM_SEL(WIFI_RTS_HSIOM_REG, WIFI_RTS_HSIOM_MASK,
                                       WIFI_RTS_HSIOM_POS, WIFI_RTS_HSIOM_SEL_UART);
    #endif /* (WIFI_UART_RTS_PIN) */
#endif /* (WIFI_SCB_MODE_UNCONFIG_CONST_CFG) */

    /* Restore TX interrupt sources. */
    WIFI_SetTxInterruptMode(WIFI_IntrTxMask);
}


/*******************************************************************************
* Function Name: WIFI_UartStop
****************************************************************************//**
*
*  Changes the HSIOM settings for the UART output pins (TX and/or RTS) to keep
*  them inactive after the block is disabled. The output pins are controlled by
*  the GPIO data register. Also, the function disables the skip start feature
*  to not cause it to trigger after the component is enabled.
*
*******************************************************************************/
void WIFI_UartStop(void)
{
#if(WIFI_SCB_MODE_UNCONFIG_CONST_CFG)
    #if (WIFI_TX_SDA_MISO_PIN)
        if (WIFI_CHECK_TX_SDA_MISO_PIN_USED)
        {
            /* Set GPIO to drive output pin */
            WIFI_SET_HSIOM_SEL(WIFI_TX_SDA_MISO_HSIOM_REG, WIFI_TX_SDA_MISO_HSIOM_MASK,
                                           WIFI_TX_SDA_MISO_HSIOM_POS, WIFI_TX_SDA_MISO_HSIOM_SEL_GPIO);
        }
    #endif /* (WIFI_TX_SDA_MISO_PIN_PIN) */

    #if !(WIFI_CY_SCBIP_V0 || WIFI_CY_SCBIP_V1)
        #if (WIFI_RTS_SS0_PIN)
            if (WIFI_CHECK_RTS_SS0_PIN_USED)
            {
                /* Set output pin state after block is disabled */
                WIFI_uart_rts_spi_ss0_Write(WIFI_GET_UART_RTS_INACTIVE);

                /* Set GPIO to drive output pin */
                WIFI_SET_HSIOM_SEL(WIFI_RTS_SS0_HSIOM_REG, WIFI_RTS_SS0_HSIOM_MASK,
                                               WIFI_RTS_SS0_HSIOM_POS, WIFI_RTS_SS0_HSIOM_SEL_GPIO);
            }
        #endif /* (WIFI_RTS_SS0_PIN) */
    #endif /* !(WIFI_CY_SCBIP_V0 || WIFI_CY_SCBIP_V1) */

#else
    #if (WIFI_UART_TX_PIN)
        /* Set GPIO to drive output pin */
        WIFI_SET_HSIOM_SEL(WIFI_TX_HSIOM_REG, WIFI_TX_HSIOM_MASK,
                                       WIFI_TX_HSIOM_POS, WIFI_TX_HSIOM_SEL_GPIO);
    #endif /* (WIFI_UART_TX_PIN) */

    #if (WIFI_UART_RTS_PIN)
        /* Set output pin state after block is disabled */
        WIFI_rts_Write(WIFI_GET_UART_RTS_INACTIVE);

        /* Set GPIO to drive output pin */
        WIFI_SET_HSIOM_SEL(WIFI_RTS_HSIOM_REG, WIFI_RTS_HSIOM_MASK,
                                       WIFI_RTS_HSIOM_POS, WIFI_RTS_HSIOM_SEL_GPIO);
    #endif /* (WIFI_UART_RTS_PIN) */

#endif /* (WIFI_SCB_MODE_UNCONFIG_CONST_CFG) */

#if (WIFI_UART_WAKE_ENABLE_CONST)
    /* Disable skip start feature used for wakeup */
    WIFI_UART_RX_CTRL_REG &= (uint32) ~WIFI_UART_RX_CTRL_SKIP_START;
#endif /* (WIFI_UART_WAKE_ENABLE_CONST) */

    /* Store TX interrupt sources (exclude level triggered). */
    WIFI_IntrTxMask = LO16(WIFI_GetTxInterruptMode() & WIFI_INTR_UART_TX_RESTORE);
}


/*******************************************************************************
* Function Name: WIFI_UartSetRxAddress
****************************************************************************//**
*
*  Sets the hardware detectable receiver address for the UART in the
*  Multiprocessor mode.
*
*  \param address: Address for hardware address detection.
*
*******************************************************************************/
void WIFI_UartSetRxAddress(uint32 address)
{
     uint32 matchReg;

    matchReg = WIFI_RX_MATCH_REG;

    matchReg &= ((uint32) ~WIFI_RX_MATCH_ADDR_MASK); /* Clear address bits */
    matchReg |= ((uint32)  (address & WIFI_RX_MATCH_ADDR_MASK)); /* Set address  */

    WIFI_RX_MATCH_REG = matchReg;
}


/*******************************************************************************
* Function Name: WIFI_UartSetRxAddressMask
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
void WIFI_UartSetRxAddressMask(uint32 addressMask)
{
    uint32 matchReg;

    matchReg = WIFI_RX_MATCH_REG;

    matchReg &= ((uint32) ~WIFI_RX_MATCH_MASK_MASK); /* Clear address mask bits */
    matchReg |= ((uint32) (addressMask << WIFI_RX_MATCH_MASK_POS));

    WIFI_RX_MATCH_REG = matchReg;
}


#if(WIFI_UART_RX_DIRECTION)
    /*******************************************************************************
    * Function Name: WIFI_UartGetChar
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
    *   Check WIFI_rxBufferOverflow to capture that error condition.
    *
    *******************************************************************************/
    uint32 WIFI_UartGetChar(void)
    {
        uint32 rxData = 0u;

        /* Reads data only if there is data to read */
        if (0u != WIFI_SpiUartGetRxBufferSize())
        {
            rxData = WIFI_SpiUartReadRxData();
        }

        if (WIFI_CHECK_INTR_RX(WIFI_INTR_RX_ERR))
        {
            rxData = 0u; /* Error occurred: returns zero */
            WIFI_ClearRxInterruptSource(WIFI_INTR_RX_ERR);
        }

        return (rxData);
    }


    /*******************************************************************************
    * Function Name: WIFI_UartGetByte
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
    *   - WIFI_UART_RX_OVERFLOW - Attempt to write to a full
    *     receiver FIFO.
    *   - WIFI_UART_RX_UNDERFLOW    Attempt to read from an empty
    *     receiver FIFO.
    *   - WIFI_UART_RX_FRAME_ERROR - UART framing error detected.
    *   - WIFI_UART_RX_PARITY_ERROR - UART parity error detected.
    *
    *  \sideeffect
    *   The errors bits may not correspond with reading characters due to
    *   RX FIFO and software buffer usage.
    *   RX software buffer is enabled: The internal software buffer overflow
    *   is not treated as an error condition.
    *   Check WIFI_rxBufferOverflow to capture that error condition.
    *
    *******************************************************************************/
    uint32 WIFI_UartGetByte(void)
    {
        uint32 rxData;
        uint32 tmpStatus;

        #if (WIFI_CHECK_RX_SW_BUFFER)
        {
            WIFI_DisableInt();
        }
        #endif

        if (0u != WIFI_SpiUartGetRxBufferSize())
        {
            /* Enables interrupt to receive more bytes: at least one byte is in
            * buffer.
            */
            #if (WIFI_CHECK_RX_SW_BUFFER)
            {
                WIFI_EnableInt();
            }
            #endif

            /* Get received byte */
            rxData = WIFI_SpiUartReadRxData();
        }
        else
        {
            /* Reads a byte directly from RX FIFO: underflow is raised in the
            * case of empty. Otherwise the first received byte will be read.
            */
            rxData = WIFI_RX_FIFO_RD_REG;


            /* Enables interrupt to receive more bytes. */
            #if (WIFI_CHECK_RX_SW_BUFFER)
            {

                /* The byte has been read from RX FIFO. Clear RX interrupt to
                * not involve interrupt handler when RX FIFO is empty.
                */
                WIFI_ClearRxInterruptSource(WIFI_INTR_RX_NOT_EMPTY);

                WIFI_EnableInt();
            }
            #endif
        }

        /* Get and clear RX error mask */
        tmpStatus = (WIFI_GetRxInterruptSource() & WIFI_INTR_RX_ERR);
        WIFI_ClearRxInterruptSource(WIFI_INTR_RX_ERR);

        /* Puts together data and error status:
        * MP mode and accept address: 9th bit is set to notify mark.
        */
        rxData |= ((uint32) (tmpStatus << 8u));

        return (rxData);
    }


    #if !(WIFI_CY_SCBIP_V0 || WIFI_CY_SCBIP_V1)
        /*******************************************************************************
        * Function Name: WIFI_UartSetRtsPolarity
        ****************************************************************************//**
        *
        *  Sets active polarity of RTS output signal.
        *  Only available for PSoC 4100 BLE / PSoC 4200 BLE / PSoC 4100M / PSoC 4200M /
        *  PSoC 4200L / PSoC 4000S / PSoC 4100S / PSoC Analog Coprocessor devices.
        *
        *  \param polarity: Active polarity of RTS output signal.
        *   - WIFI_UART_RTS_ACTIVE_LOW  - RTS signal is active low.
        *   - WIFI_UART_RTS_ACTIVE_HIGH - RTS signal is active high.
        *
        *******************************************************************************/
        void WIFI_UartSetRtsPolarity(uint32 polarity)
        {
            if(0u != polarity)
            {
                WIFI_UART_FLOW_CTRL_REG |= (uint32)  WIFI_UART_FLOW_CTRL_RTS_POLARITY;
            }
            else
            {
                WIFI_UART_FLOW_CTRL_REG &= (uint32) ~WIFI_UART_FLOW_CTRL_RTS_POLARITY;
            }
        }


        /*******************************************************************************
        * Function Name: WIFI_UartSetRtsFifoLevel
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
        void WIFI_UartSetRtsFifoLevel(uint32 level)
        {
            uint32 uartFlowCtrl;

            uartFlowCtrl = WIFI_UART_FLOW_CTRL_REG;

            uartFlowCtrl &= ((uint32) ~WIFI_UART_FLOW_CTRL_TRIGGER_LEVEL_MASK); /* Clear level mask bits */
            uartFlowCtrl |= ((uint32) (WIFI_UART_FLOW_CTRL_TRIGGER_LEVEL_MASK & level));

            WIFI_UART_FLOW_CTRL_REG = uartFlowCtrl;
        }
    #endif /* !(WIFI_CY_SCBIP_V0 || WIFI_CY_SCBIP_V1) */

#endif /* (WIFI_UART_RX_DIRECTION) */


#if(WIFI_UART_TX_DIRECTION)
    /*******************************************************************************
    * Function Name: WIFI_UartPutString
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
    void WIFI_UartPutString(const char8 string[])
    {
        uint32 bufIndex;

        bufIndex = 0u;

        /* Blocks the control flow until all data has been sent */
        while(string[bufIndex] != ((char8) 0))
        {
            WIFI_UartPutChar((uint32) string[bufIndex]);
            bufIndex++;
        }
    }


    /*******************************************************************************
    * Function Name: WIFI_UartPutCRLF
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
    void WIFI_UartPutCRLF(uint32 txDataByte)
    {
        WIFI_UartPutChar(txDataByte);  /* Blocks control flow until all data has been sent */
        WIFI_UartPutChar(0x0Du);       /* Blocks control flow until all data has been sent */
        WIFI_UartPutChar(0x0Au);       /* Blocks control flow until all data has been sent */
    }


    #if !(WIFI_CY_SCBIP_V0 || WIFI_CY_SCBIP_V1)
        /*******************************************************************************
        * Function Name: WIFISCB_UartEnableCts
        ****************************************************************************//**
        *
        *  Enables usage of CTS input signal by the UART transmitter.
        *  Only available for PSoC 4100 BLE / PSoC 4200 BLE / PSoC 4100M / PSoC 4200M /
        *  PSoC 4200L / PSoC 4000S / PSoC 4100S / PSoC Analog Coprocessor devices.
        *
        *******************************************************************************/
        void WIFI_UartEnableCts(void)
        {
            WIFI_UART_FLOW_CTRL_REG |= (uint32)  WIFI_UART_FLOW_CTRL_CTS_ENABLE;
        }


        /*******************************************************************************
        * Function Name: WIFI_UartDisableCts
        ****************************************************************************//**
        *
        *  Disables usage of CTS input signal by the UART transmitter.
        *  Only available for PSoC 4100 BLE / PSoC 4200 BLE / PSoC 4100M / PSoC 4200M /
        *  PSoC 4200L / PSoC 4000S / PSoC 4100S / PSoC Analog Coprocessor devices.
        *
        *******************************************************************************/
        void WIFI_UartDisableCts(void)
        {
            WIFI_UART_FLOW_CTRL_REG &= (uint32) ~WIFI_UART_FLOW_CTRL_CTS_ENABLE;
        }


        /*******************************************************************************
        * Function Name: WIFI_UartSetCtsPolarity
        ****************************************************************************//**
        *
        *  Sets active polarity of CTS input signal.
        *  Only available for PSoC 4100 BLE / PSoC 4200 BLE / PSoC 4100M / PSoC 4200M /
        *  PSoC 4200L / PSoC 4000S / PSoC 4100S / PSoC Analog Coprocessor devices.
        *
        * \param
        * polarity: Active polarity of CTS output signal.
        *   - WIFI_UART_CTS_ACTIVE_LOW  - CTS signal is active low.
        *   - WIFI_UART_CTS_ACTIVE_HIGH - CTS signal is active high.
        *
        *******************************************************************************/
        void WIFI_UartSetCtsPolarity(uint32 polarity)
        {
            if (0u != polarity)
            {
                WIFI_UART_FLOW_CTRL_REG |= (uint32)  WIFI_UART_FLOW_CTRL_CTS_POLARITY;
            }
            else
            {
                WIFI_UART_FLOW_CTRL_REG &= (uint32) ~WIFI_UART_FLOW_CTRL_CTS_POLARITY;
            }
        }
    #endif /* !(WIFI_CY_SCBIP_V0 || WIFI_CY_SCBIP_V1) */


    /*******************************************************************************
    * Function Name: WIFI_UartSendBreakBlocking
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
    void WIFI_UartSendBreakBlocking(uint32 breakWidth)
    {
        uint32 txCtrlReg;
        uint32 txIntrReg;

        /* Disable all UART TX interrupt source and clear UART TX Done history */
        txIntrReg = WIFI_GetTxInterruptMode();
        WIFI_SetTxInterruptMode(0u);
        WIFI_ClearTxInterruptSource(WIFI_INTR_TX_UART_DONE);

        /* Store TX CTRL configuration */
        txCtrlReg = WIFI_TX_CTRL_REG;

        /* Set break width */
        WIFI_TX_CTRL_REG = (WIFI_TX_CTRL_REG & (uint32) ~WIFI_TX_CTRL_DATA_WIDTH_MASK) |
                                        WIFI_GET_TX_CTRL_DATA_WIDTH(breakWidth);

        /* Generate break */
        WIFI_TX_FIFO_WR_REG = 0u;

        /* Wait for break completion */
        while (0u == (WIFI_GetTxInterruptSource() & WIFI_INTR_TX_UART_DONE))
        {
        }

        /* Clear all UART TX interrupt sources to  */
        WIFI_ClearTxInterruptSource(WIFI_INTR_TX_ALL);

        /* Restore TX interrupt sources and data width */
        WIFI_TX_CTRL_REG = txCtrlReg;
        WIFI_SetTxInterruptMode(txIntrReg);
    }
#endif /* (WIFI_UART_TX_DIRECTION) */


#if (WIFI_UART_WAKE_ENABLE_CONST)
    /*******************************************************************************
    * Function Name: WIFI_UartSaveConfig
    ****************************************************************************//**
    *
    *  Clears and enables an interrupt on a falling edge of the Rx input. The GPIO
    *  interrupt does not track in the active mode, therefore requires to be
    *  cleared by this API.
    *
    *******************************************************************************/
    void WIFI_UartSaveConfig(void)
    {
    #if (WIFI_UART_RX_WAKEUP_IRQ)
        /* Set SKIP_START if requested (set by default). */
        if (0u != WIFI_skipStart)
        {
            WIFI_UART_RX_CTRL_REG |= (uint32)  WIFI_UART_RX_CTRL_SKIP_START;
        }
        else
        {
            WIFI_UART_RX_CTRL_REG &= (uint32) ~WIFI_UART_RX_CTRL_SKIP_START;
        }

        /* Clear RX GPIO interrupt status and pending interrupt in NVIC because
        * falling edge on RX line occurs while UART communication in active mode.
        * Enable interrupt: next interrupt trigger should wakeup device.
        */
        WIFI_CLEAR_UART_RX_WAKE_INTR;
        WIFI_RxWakeClearPendingInt();
        WIFI_RxWakeEnableInt();
    #endif /* (WIFI_UART_RX_WAKEUP_IRQ) */
    }


    /*******************************************************************************
    * Function Name: WIFI_UartRestoreConfig
    ****************************************************************************//**
    *
    *  Disables the RX GPIO interrupt. Until this function is called the interrupt
    *  remains active and triggers on every falling edge of the UART RX line.
    *
    *******************************************************************************/
    void WIFI_UartRestoreConfig(void)
    {
    #if (WIFI_UART_RX_WAKEUP_IRQ)
        /* Disable interrupt: no more triggers in active mode */
        WIFI_RxWakeDisableInt();
    #endif /* (WIFI_UART_RX_WAKEUP_IRQ) */
    }


    #if (WIFI_UART_RX_WAKEUP_IRQ)
        /*******************************************************************************
        * Function Name: WIFI_UART_WAKEUP_ISR
        ****************************************************************************//**
        *
        *  Handles the Interrupt Service Routine for the SCB UART mode GPIO wakeup
        *  event. This event is configured to trigger on a falling edge of the RX line.
        *
        *******************************************************************************/
        CY_ISR(WIFI_UART_WAKEUP_ISR)
        {
        #ifdef WIFI_UART_WAKEUP_ISR_ENTRY_CALLBACK
            WIFI_UART_WAKEUP_ISR_EntryCallback();
        #endif /* WIFI_UART_WAKEUP_ISR_ENTRY_CALLBACK */

            WIFI_CLEAR_UART_RX_WAKE_INTR;

        #ifdef WIFI_UART_WAKEUP_ISR_EXIT_CALLBACK
            WIFI_UART_WAKEUP_ISR_ExitCallback();
        #endif /* WIFI_UART_WAKEUP_ISR_EXIT_CALLBACK */
        }
    #endif /* (WIFI_UART_RX_WAKEUP_IRQ) */
#endif /* (WIFI_UART_RX_WAKEUP_IRQ) */


/* [] END OF FILE */
