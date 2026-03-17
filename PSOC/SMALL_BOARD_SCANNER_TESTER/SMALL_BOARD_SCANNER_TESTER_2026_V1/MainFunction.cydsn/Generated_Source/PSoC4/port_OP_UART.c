/***************************************************************************//**
* \file port_OP_UART.c
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

#include "port_OP_PVT.h"
#include "port_OP_SPI_UART_PVT.h"
#include "cyapicallbacks.h"

#if (port_OP_UART_WAKE_ENABLE_CONST && port_OP_UART_RX_WAKEUP_IRQ)
    /**
    * \addtogroup group_globals
    * \{
    */
    /** This global variable determines whether to enable Skip Start
    * functionality when port_OP_Sleep() function is called:
    * 0 – disable, other values – enable. Default value is 1.
    * It is only available when Enable wakeup from Deep Sleep Mode is enabled.
    */
    uint8 port_OP_skipStart = 1u;
    /** \} globals */
#endif /* (port_OP_UART_WAKE_ENABLE_CONST && port_OP_UART_RX_WAKEUP_IRQ) */

#if(port_OP_SCB_MODE_UNCONFIG_CONST_CFG)

    /***************************************
    *  Configuration Structure Initialization
    ***************************************/

    const port_OP_UART_INIT_STRUCT port_OP_configUart =
    {
        port_OP_UART_SUB_MODE,
        port_OP_UART_DIRECTION,
        port_OP_UART_DATA_BITS_NUM,
        port_OP_UART_PARITY_TYPE,
        port_OP_UART_STOP_BITS_NUM,
        port_OP_UART_OVS_FACTOR,
        port_OP_UART_IRDA_LOW_POWER,
        port_OP_UART_MEDIAN_FILTER_ENABLE,
        port_OP_UART_RETRY_ON_NACK,
        port_OP_UART_IRDA_POLARITY,
        port_OP_UART_DROP_ON_PARITY_ERR,
        port_OP_UART_DROP_ON_FRAME_ERR,
        port_OP_UART_WAKE_ENABLE,
        0u,
        NULL,
        0u,
        NULL,
        port_OP_UART_MP_MODE_ENABLE,
        port_OP_UART_MP_ACCEPT_ADDRESS,
        port_OP_UART_MP_RX_ADDRESS,
        port_OP_UART_MP_RX_ADDRESS_MASK,
        (uint32) port_OP_SCB_IRQ_INTERNAL,
        port_OP_UART_INTR_RX_MASK,
        port_OP_UART_RX_TRIGGER_LEVEL,
        port_OP_UART_INTR_TX_MASK,
        port_OP_UART_TX_TRIGGER_LEVEL,
        (uint8) port_OP_UART_BYTE_MODE_ENABLE,
        (uint8) port_OP_UART_CTS_ENABLE,
        (uint8) port_OP_UART_CTS_POLARITY,
        (uint8) port_OP_UART_RTS_POLARITY,
        (uint8) port_OP_UART_RTS_FIFO_LEVEL,
        (uint8) port_OP_UART_RX_BREAK_WIDTH
    };


    /*******************************************************************************
    * Function Name: port_OP_UartInit
    ****************************************************************************//**
    *
    *  Configures the port_OP for UART operation.
    *
    *  This function is intended specifically to be used when the port_OP
    *  configuration is set to “Unconfigured port_OP” in the customizer.
    *  After initializing the port_OP in UART mode using this function,
    *  the component can be enabled using the port_OP_Start() or
    * port_OP_Enable() function.
    *  This function uses a pointer to a structure that provides the configuration
    *  settings. This structure contains the same information that would otherwise
    *  be provided by the customizer settings.
    *
    *  \param config: pointer to a structure that contains the following list of
    *   fields. These fields match the selections available in the customizer.
    *   Refer to the customizer for further description of the settings.
    *
    *******************************************************************************/
    void port_OP_UartInit(const port_OP_UART_INIT_STRUCT *config)
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

        #if !(port_OP_CY_SCBIP_V0 || port_OP_CY_SCBIP_V1)
            /* Add RTS and CTS pins to configure */
            pinsConfig |= (0u != config->rtsRxFifoLevel) ? (port_OP_UART_RTS_PIN_ENABLE) : (0u);
            pinsConfig |= (0u != config->enableCts)      ? (port_OP_UART_CTS_PIN_ENABLE) : (0u);
        #endif /* !(port_OP_CY_SCBIP_V0 || port_OP_CY_SCBIP_V1) */

            /* Configure pins */
            port_OP_SetPins(port_OP_SCB_MODE_UART, config->mode, pinsConfig);

            /* Store internal configuration */
            port_OP_scbMode       = (uint8) port_OP_SCB_MODE_UART;
            port_OP_scbEnableWake = (uint8) config->enableWake;
            port_OP_scbEnableIntr = (uint8) config->enableInterrupt;

            /* Set RX direction internal variables */
            port_OP_rxBuffer      =         config->rxBuffer;
            port_OP_rxDataBits    = (uint8) config->dataBits;
            port_OP_rxBufferSize  =         config->rxBufferSize;

            /* Set TX direction internal variables */
            port_OP_txBuffer      =         config->txBuffer;
            port_OP_txDataBits    = (uint8) config->dataBits;
            port_OP_txBufferSize  =         config->txBufferSize;

            /* Configure UART interface */
            if(port_OP_UART_MODE_IRDA == config->mode)
            {
                /* OVS settings: IrDA */
                port_OP_CTRL_REG  = ((0u != config->enableIrdaLowPower) ?
                                                (port_OP_UART_GET_CTRL_OVS_IRDA_LP(config->oversample)) :
                                                (port_OP_CTRL_OVS_IRDA_OVS16));
            }
            else
            {
                /* OVS settings: UART and SmartCard */
                port_OP_CTRL_REG  = port_OP_GET_CTRL_OVS(config->oversample);
            }

            port_OP_CTRL_REG     |= port_OP_GET_CTRL_BYTE_MODE  (config->enableByteMode)      |
                                             port_OP_GET_CTRL_ADDR_ACCEPT(config->multiprocAcceptAddr) |
                                             port_OP_CTRL_UART;

            /* Configure sub-mode: UART, SmartCard or IrDA */
            port_OP_UART_CTRL_REG = port_OP_GET_UART_CTRL_MODE(config->mode);

            /* Configure RX direction */
            port_OP_UART_RX_CTRL_REG = port_OP_GET_UART_RX_CTRL_MODE(config->stopBits)              |
                                        port_OP_GET_UART_RX_CTRL_POLARITY(config->enableInvertedRx)          |
                                        port_OP_GET_UART_RX_CTRL_MP_MODE(config->enableMultiproc)            |
                                        port_OP_GET_UART_RX_CTRL_DROP_ON_PARITY_ERR(config->dropOnParityErr) |
                                        port_OP_GET_UART_RX_CTRL_DROP_ON_FRAME_ERR(config->dropOnFrameErr)   |
                                        port_OP_GET_UART_RX_CTRL_BREAK_WIDTH(config->breakWidth);

            if(port_OP_UART_PARITY_NONE != config->parity)
            {
               port_OP_UART_RX_CTRL_REG |= port_OP_GET_UART_RX_CTRL_PARITY(config->parity) |
                                                    port_OP_UART_RX_CTRL_PARITY_ENABLED;
            }

            port_OP_RX_CTRL_REG      = port_OP_GET_RX_CTRL_DATA_WIDTH(config->dataBits)       |
                                                port_OP_GET_RX_CTRL_MEDIAN(config->enableMedianFilter) |
                                                port_OP_GET_UART_RX_CTRL_ENABLED(config->direction);

            port_OP_RX_FIFO_CTRL_REG = port_OP_GET_RX_FIFO_CTRL_TRIGGER_LEVEL(config->rxTriggerLevel);

            /* Configure MP address */
            port_OP_RX_MATCH_REG     = port_OP_GET_RX_MATCH_ADDR(config->multiprocAddr) |
                                                port_OP_GET_RX_MATCH_MASK(config->multiprocAddrMask);

            /* Configure RX direction */
            port_OP_UART_TX_CTRL_REG = port_OP_GET_UART_TX_CTRL_MODE(config->stopBits) |
                                                port_OP_GET_UART_TX_CTRL_RETRY_NACK(config->enableRetryNack);

            if(port_OP_UART_PARITY_NONE != config->parity)
            {
               port_OP_UART_TX_CTRL_REG |= port_OP_GET_UART_TX_CTRL_PARITY(config->parity) |
                                                    port_OP_UART_TX_CTRL_PARITY_ENABLED;
            }

            port_OP_TX_CTRL_REG      = port_OP_GET_TX_CTRL_DATA_WIDTH(config->dataBits)    |
                                                port_OP_GET_UART_TX_CTRL_ENABLED(config->direction);

            port_OP_TX_FIFO_CTRL_REG = port_OP_GET_TX_FIFO_CTRL_TRIGGER_LEVEL(config->txTriggerLevel);

        #if !(port_OP_CY_SCBIP_V0 || port_OP_CY_SCBIP_V1)
            port_OP_UART_FLOW_CTRL_REG = port_OP_GET_UART_FLOW_CTRL_CTS_ENABLE(config->enableCts) | \
                                            port_OP_GET_UART_FLOW_CTRL_CTS_POLARITY (config->ctsPolarity)  | \
                                            port_OP_GET_UART_FLOW_CTRL_RTS_POLARITY (config->rtsPolarity)  | \
                                            port_OP_GET_UART_FLOW_CTRL_TRIGGER_LEVEL(config->rtsRxFifoLevel);
        #endif /* !(port_OP_CY_SCBIP_V0 || port_OP_CY_SCBIP_V1) */

            /* Configure interrupt with UART handler but do not enable it */
            CyIntDisable    (port_OP_ISR_NUMBER);
            CyIntSetPriority(port_OP_ISR_NUMBER, port_OP_ISR_PRIORITY);
            (void) CyIntSetVector(port_OP_ISR_NUMBER, &port_OP_SPI_UART_ISR);

            /* Configure WAKE interrupt */
        #if(port_OP_UART_RX_WAKEUP_IRQ)
            CyIntDisable    (port_OP_RX_WAKE_ISR_NUMBER);
            CyIntSetPriority(port_OP_RX_WAKE_ISR_NUMBER, port_OP_RX_WAKE_ISR_PRIORITY);
            (void) CyIntSetVector(port_OP_RX_WAKE_ISR_NUMBER, &port_OP_UART_WAKEUP_ISR);
        #endif /* (port_OP_UART_RX_WAKEUP_IRQ) */

            /* Configure interrupt sources */
            port_OP_INTR_I2C_EC_MASK_REG = port_OP_NO_INTR_SOURCES;
            port_OP_INTR_SPI_EC_MASK_REG = port_OP_NO_INTR_SOURCES;
            port_OP_INTR_SLAVE_MASK_REG  = port_OP_NO_INTR_SOURCES;
            port_OP_INTR_MASTER_MASK_REG = port_OP_NO_INTR_SOURCES;
            port_OP_INTR_RX_MASK_REG     = config->rxInterruptMask;
            port_OP_INTR_TX_MASK_REG     = config->txInterruptMask;

            /* Configure TX interrupt sources to restore. */
            port_OP_IntrTxMask = LO16(port_OP_INTR_TX_MASK_REG);

            /* Clear RX buffer indexes */
            port_OP_rxBufferHead     = 0u;
            port_OP_rxBufferTail     = 0u;
            port_OP_rxBufferOverflow = 0u;

            /* Clear TX buffer indexes */
            port_OP_txBufferHead = 0u;
            port_OP_txBufferTail = 0u;
        }
    }

#else

    /*******************************************************************************
    * Function Name: port_OP_UartInit
    ****************************************************************************//**
    *
    *  Configures the SCB for the UART operation.
    *
    *******************************************************************************/
    void port_OP_UartInit(void)
    {
        /* Configure UART interface */
        port_OP_CTRL_REG = port_OP_UART_DEFAULT_CTRL;

        /* Configure sub-mode: UART, SmartCard or IrDA */
        port_OP_UART_CTRL_REG = port_OP_UART_DEFAULT_UART_CTRL;

        /* Configure RX direction */
        port_OP_UART_RX_CTRL_REG = port_OP_UART_DEFAULT_UART_RX_CTRL;
        port_OP_RX_CTRL_REG      = port_OP_UART_DEFAULT_RX_CTRL;
        port_OP_RX_FIFO_CTRL_REG = port_OP_UART_DEFAULT_RX_FIFO_CTRL;
        port_OP_RX_MATCH_REG     = port_OP_UART_DEFAULT_RX_MATCH_REG;

        /* Configure TX direction */
        port_OP_UART_TX_CTRL_REG = port_OP_UART_DEFAULT_UART_TX_CTRL;
        port_OP_TX_CTRL_REG      = port_OP_UART_DEFAULT_TX_CTRL;
        port_OP_TX_FIFO_CTRL_REG = port_OP_UART_DEFAULT_TX_FIFO_CTRL;

    #if !(port_OP_CY_SCBIP_V0 || port_OP_CY_SCBIP_V1)
        port_OP_UART_FLOW_CTRL_REG = port_OP_UART_DEFAULT_FLOW_CTRL;
    #endif /* !(port_OP_CY_SCBIP_V0 || port_OP_CY_SCBIP_V1) */

        /* Configure interrupt with UART handler but do not enable it */
    #if(port_OP_SCB_IRQ_INTERNAL)
        CyIntDisable    (port_OP_ISR_NUMBER);
        CyIntSetPriority(port_OP_ISR_NUMBER, port_OP_ISR_PRIORITY);
        (void) CyIntSetVector(port_OP_ISR_NUMBER, &port_OP_SPI_UART_ISR);
    #endif /* (port_OP_SCB_IRQ_INTERNAL) */

        /* Configure WAKE interrupt */
    #if(port_OP_UART_RX_WAKEUP_IRQ)
        CyIntDisable    (port_OP_RX_WAKE_ISR_NUMBER);
        CyIntSetPriority(port_OP_RX_WAKE_ISR_NUMBER, port_OP_RX_WAKE_ISR_PRIORITY);
        (void) CyIntSetVector(port_OP_RX_WAKE_ISR_NUMBER, &port_OP_UART_WAKEUP_ISR);
    #endif /* (port_OP_UART_RX_WAKEUP_IRQ) */

        /* Configure interrupt sources */
        port_OP_INTR_I2C_EC_MASK_REG = port_OP_UART_DEFAULT_INTR_I2C_EC_MASK;
        port_OP_INTR_SPI_EC_MASK_REG = port_OP_UART_DEFAULT_INTR_SPI_EC_MASK;
        port_OP_INTR_SLAVE_MASK_REG  = port_OP_UART_DEFAULT_INTR_SLAVE_MASK;
        port_OP_INTR_MASTER_MASK_REG = port_OP_UART_DEFAULT_INTR_MASTER_MASK;
        port_OP_INTR_RX_MASK_REG     = port_OP_UART_DEFAULT_INTR_RX_MASK;
        port_OP_INTR_TX_MASK_REG     = port_OP_UART_DEFAULT_INTR_TX_MASK;

        /* Configure TX interrupt sources to restore. */
        port_OP_IntrTxMask = LO16(port_OP_INTR_TX_MASK_REG);

    #if(port_OP_INTERNAL_RX_SW_BUFFER_CONST)
        port_OP_rxBufferHead     = 0u;
        port_OP_rxBufferTail     = 0u;
        port_OP_rxBufferOverflow = 0u;
    #endif /* (port_OP_INTERNAL_RX_SW_BUFFER_CONST) */

    #if(port_OP_INTERNAL_TX_SW_BUFFER_CONST)
        port_OP_txBufferHead = 0u;
        port_OP_txBufferTail = 0u;
    #endif /* (port_OP_INTERNAL_TX_SW_BUFFER_CONST) */
    }
#endif /* (port_OP_SCB_MODE_UNCONFIG_CONST_CFG) */


/*******************************************************************************
* Function Name: port_OP_UartPostEnable
****************************************************************************//**
*
*  Restores HSIOM settings for the UART output pins (TX and/or RTS) to be
*  controlled by the SCB UART.
*
*******************************************************************************/
void port_OP_UartPostEnable(void)
{
#if (port_OP_SCB_MODE_UNCONFIG_CONST_CFG)
    #if (port_OP_TX_SDA_MISO_PIN)
        if (port_OP_CHECK_TX_SDA_MISO_PIN_USED)
        {
            /* Set SCB UART to drive the output pin */
            port_OP_SET_HSIOM_SEL(port_OP_TX_SDA_MISO_HSIOM_REG, port_OP_TX_SDA_MISO_HSIOM_MASK,
                                           port_OP_TX_SDA_MISO_HSIOM_POS, port_OP_TX_SDA_MISO_HSIOM_SEL_UART);
        }
    #endif /* (port_OP_TX_SDA_MISO_PIN_PIN) */

    #if !(port_OP_CY_SCBIP_V0 || port_OP_CY_SCBIP_V1)
        #if (port_OP_RTS_SS0_PIN)
            if (port_OP_CHECK_RTS_SS0_PIN_USED)
            {
                /* Set SCB UART to drive the output pin */
                port_OP_SET_HSIOM_SEL(port_OP_RTS_SS0_HSIOM_REG, port_OP_RTS_SS0_HSIOM_MASK,
                                               port_OP_RTS_SS0_HSIOM_POS, port_OP_RTS_SS0_HSIOM_SEL_UART);
            }
        #endif /* (port_OP_RTS_SS0_PIN) */
    #endif /* !(port_OP_CY_SCBIP_V0 || port_OP_CY_SCBIP_V1) */

#else
    #if (port_OP_UART_TX_PIN)
         /* Set SCB UART to drive the output pin */
        port_OP_SET_HSIOM_SEL(port_OP_TX_HSIOM_REG, port_OP_TX_HSIOM_MASK,
                                       port_OP_TX_HSIOM_POS, port_OP_TX_HSIOM_SEL_UART);
    #endif /* (port_OP_UART_TX_PIN) */

    #if (port_OP_UART_RTS_PIN)
        /* Set SCB UART to drive the output pin */
        port_OP_SET_HSIOM_SEL(port_OP_RTS_HSIOM_REG, port_OP_RTS_HSIOM_MASK,
                                       port_OP_RTS_HSIOM_POS, port_OP_RTS_HSIOM_SEL_UART);
    #endif /* (port_OP_UART_RTS_PIN) */
#endif /* (port_OP_SCB_MODE_UNCONFIG_CONST_CFG) */

    /* Restore TX interrupt sources. */
    port_OP_SetTxInterruptMode(port_OP_IntrTxMask);
}


/*******************************************************************************
* Function Name: port_OP_UartStop
****************************************************************************//**
*
*  Changes the HSIOM settings for the UART output pins (TX and/or RTS) to keep
*  them inactive after the block is disabled. The output pins are controlled by
*  the GPIO data register. Also, the function disables the skip start feature
*  to not cause it to trigger after the component is enabled.
*
*******************************************************************************/
void port_OP_UartStop(void)
{
#if(port_OP_SCB_MODE_UNCONFIG_CONST_CFG)
    #if (port_OP_TX_SDA_MISO_PIN)
        if (port_OP_CHECK_TX_SDA_MISO_PIN_USED)
        {
            /* Set GPIO to drive output pin */
            port_OP_SET_HSIOM_SEL(port_OP_TX_SDA_MISO_HSIOM_REG, port_OP_TX_SDA_MISO_HSIOM_MASK,
                                           port_OP_TX_SDA_MISO_HSIOM_POS, port_OP_TX_SDA_MISO_HSIOM_SEL_GPIO);
        }
    #endif /* (port_OP_TX_SDA_MISO_PIN_PIN) */

    #if !(port_OP_CY_SCBIP_V0 || port_OP_CY_SCBIP_V1)
        #if (port_OP_RTS_SS0_PIN)
            if (port_OP_CHECK_RTS_SS0_PIN_USED)
            {
                /* Set output pin state after block is disabled */
                port_OP_uart_rts_spi_ss0_Write(port_OP_GET_UART_RTS_INACTIVE);

                /* Set GPIO to drive output pin */
                port_OP_SET_HSIOM_SEL(port_OP_RTS_SS0_HSIOM_REG, port_OP_RTS_SS0_HSIOM_MASK,
                                               port_OP_RTS_SS0_HSIOM_POS, port_OP_RTS_SS0_HSIOM_SEL_GPIO);
            }
        #endif /* (port_OP_RTS_SS0_PIN) */
    #endif /* !(port_OP_CY_SCBIP_V0 || port_OP_CY_SCBIP_V1) */

#else
    #if (port_OP_UART_TX_PIN)
        /* Set GPIO to drive output pin */
        port_OP_SET_HSIOM_SEL(port_OP_TX_HSIOM_REG, port_OP_TX_HSIOM_MASK,
                                       port_OP_TX_HSIOM_POS, port_OP_TX_HSIOM_SEL_GPIO);
    #endif /* (port_OP_UART_TX_PIN) */

    #if (port_OP_UART_RTS_PIN)
        /* Set output pin state after block is disabled */
        port_OP_rts_Write(port_OP_GET_UART_RTS_INACTIVE);

        /* Set GPIO to drive output pin */
        port_OP_SET_HSIOM_SEL(port_OP_RTS_HSIOM_REG, port_OP_RTS_HSIOM_MASK,
                                       port_OP_RTS_HSIOM_POS, port_OP_RTS_HSIOM_SEL_GPIO);
    #endif /* (port_OP_UART_RTS_PIN) */

#endif /* (port_OP_SCB_MODE_UNCONFIG_CONST_CFG) */

#if (port_OP_UART_WAKE_ENABLE_CONST)
    /* Disable skip start feature used for wakeup */
    port_OP_UART_RX_CTRL_REG &= (uint32) ~port_OP_UART_RX_CTRL_SKIP_START;
#endif /* (port_OP_UART_WAKE_ENABLE_CONST) */

    /* Store TX interrupt sources (exclude level triggered). */
    port_OP_IntrTxMask = LO16(port_OP_GetTxInterruptMode() & port_OP_INTR_UART_TX_RESTORE);
}


/*******************************************************************************
* Function Name: port_OP_UartSetRxAddress
****************************************************************************//**
*
*  Sets the hardware detectable receiver address for the UART in the
*  Multiprocessor mode.
*
*  \param address: Address for hardware address detection.
*
*******************************************************************************/
void port_OP_UartSetRxAddress(uint32 address)
{
     uint32 matchReg;

    matchReg = port_OP_RX_MATCH_REG;

    matchReg &= ((uint32) ~port_OP_RX_MATCH_ADDR_MASK); /* Clear address bits */
    matchReg |= ((uint32)  (address & port_OP_RX_MATCH_ADDR_MASK)); /* Set address  */

    port_OP_RX_MATCH_REG = matchReg;
}


/*******************************************************************************
* Function Name: port_OP_UartSetRxAddressMask
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
void port_OP_UartSetRxAddressMask(uint32 addressMask)
{
    uint32 matchReg;

    matchReg = port_OP_RX_MATCH_REG;

    matchReg &= ((uint32) ~port_OP_RX_MATCH_MASK_MASK); /* Clear address mask bits */
    matchReg |= ((uint32) (addressMask << port_OP_RX_MATCH_MASK_POS));

    port_OP_RX_MATCH_REG = matchReg;
}


#if(port_OP_UART_RX_DIRECTION)
    /*******************************************************************************
    * Function Name: port_OP_UartGetChar
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
    *   Check port_OP_rxBufferOverflow to capture that error condition.
    *
    *******************************************************************************/
    uint32 port_OP_UartGetChar(void)
    {
        uint32 rxData = 0u;

        /* Reads data only if there is data to read */
        if (0u != port_OP_SpiUartGetRxBufferSize())
        {
            rxData = port_OP_SpiUartReadRxData();
        }

        if (port_OP_CHECK_INTR_RX(port_OP_INTR_RX_ERR))
        {
            rxData = 0u; /* Error occurred: returns zero */
            port_OP_ClearRxInterruptSource(port_OP_INTR_RX_ERR);
        }

        return (rxData);
    }


    /*******************************************************************************
    * Function Name: port_OP_UartGetByte
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
    *   - port_OP_UART_RX_OVERFLOW - Attempt to write to a full
    *     receiver FIFO.
    *   - port_OP_UART_RX_UNDERFLOW    Attempt to read from an empty
    *     receiver FIFO.
    *   - port_OP_UART_RX_FRAME_ERROR - UART framing error detected.
    *   - port_OP_UART_RX_PARITY_ERROR - UART parity error detected.
    *
    *  \sideeffect
    *   The errors bits may not correspond with reading characters due to
    *   RX FIFO and software buffer usage.
    *   RX software buffer is enabled: The internal software buffer overflow
    *   is not treated as an error condition.
    *   Check port_OP_rxBufferOverflow to capture that error condition.
    *
    *******************************************************************************/
    uint32 port_OP_UartGetByte(void)
    {
        uint32 rxData;
        uint32 tmpStatus;

        #if (port_OP_CHECK_RX_SW_BUFFER)
        {
            port_OP_DisableInt();
        }
        #endif

        if (0u != port_OP_SpiUartGetRxBufferSize())
        {
            /* Enables interrupt to receive more bytes: at least one byte is in
            * buffer.
            */
            #if (port_OP_CHECK_RX_SW_BUFFER)
            {
                port_OP_EnableInt();
            }
            #endif

            /* Get received byte */
            rxData = port_OP_SpiUartReadRxData();
        }
        else
        {
            /* Reads a byte directly from RX FIFO: underflow is raised in the
            * case of empty. Otherwise the first received byte will be read.
            */
            rxData = port_OP_RX_FIFO_RD_REG;


            /* Enables interrupt to receive more bytes. */
            #if (port_OP_CHECK_RX_SW_BUFFER)
            {

                /* The byte has been read from RX FIFO. Clear RX interrupt to
                * not involve interrupt handler when RX FIFO is empty.
                */
                port_OP_ClearRxInterruptSource(port_OP_INTR_RX_NOT_EMPTY);

                port_OP_EnableInt();
            }
            #endif
        }

        /* Get and clear RX error mask */
        tmpStatus = (port_OP_GetRxInterruptSource() & port_OP_INTR_RX_ERR);
        port_OP_ClearRxInterruptSource(port_OP_INTR_RX_ERR);

        /* Puts together data and error status:
        * MP mode and accept address: 9th bit is set to notify mark.
        */
        rxData |= ((uint32) (tmpStatus << 8u));

        return (rxData);
    }


    #if !(port_OP_CY_SCBIP_V0 || port_OP_CY_SCBIP_V1)
        /*******************************************************************************
        * Function Name: port_OP_UartSetRtsPolarity
        ****************************************************************************//**
        *
        *  Sets active polarity of RTS output signal.
        *  Only available for PSoC 4100 BLE / PSoC 4200 BLE / PSoC 4100M / PSoC 4200M /
        *  PSoC 4200L / PSoC 4000S / PSoC 4100S / PSoC Analog Coprocessor devices.
        *
        *  \param polarity: Active polarity of RTS output signal.
        *   - port_OP_UART_RTS_ACTIVE_LOW  - RTS signal is active low.
        *   - port_OP_UART_RTS_ACTIVE_HIGH - RTS signal is active high.
        *
        *******************************************************************************/
        void port_OP_UartSetRtsPolarity(uint32 polarity)
        {
            if(0u != polarity)
            {
                port_OP_UART_FLOW_CTRL_REG |= (uint32)  port_OP_UART_FLOW_CTRL_RTS_POLARITY;
            }
            else
            {
                port_OP_UART_FLOW_CTRL_REG &= (uint32) ~port_OP_UART_FLOW_CTRL_RTS_POLARITY;
            }
        }


        /*******************************************************************************
        * Function Name: port_OP_UartSetRtsFifoLevel
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
        void port_OP_UartSetRtsFifoLevel(uint32 level)
        {
            uint32 uartFlowCtrl;

            uartFlowCtrl = port_OP_UART_FLOW_CTRL_REG;

            uartFlowCtrl &= ((uint32) ~port_OP_UART_FLOW_CTRL_TRIGGER_LEVEL_MASK); /* Clear level mask bits */
            uartFlowCtrl |= ((uint32) (port_OP_UART_FLOW_CTRL_TRIGGER_LEVEL_MASK & level));

            port_OP_UART_FLOW_CTRL_REG = uartFlowCtrl;
        }
    #endif /* !(port_OP_CY_SCBIP_V0 || port_OP_CY_SCBIP_V1) */

#endif /* (port_OP_UART_RX_DIRECTION) */


#if(port_OP_UART_TX_DIRECTION)
    /*******************************************************************************
    * Function Name: port_OP_UartPutString
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
    void port_OP_UartPutString(const char8 string[])
    {
        uint32 bufIndex;

        bufIndex = 0u;

        /* Blocks the control flow until all data has been sent */
        while(string[bufIndex] != ((char8) 0))
        {
            port_OP_UartPutChar((uint32) string[bufIndex]);
            bufIndex++;
        }
    }


    /*******************************************************************************
    * Function Name: port_OP_UartPutCRLF
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
    void port_OP_UartPutCRLF(uint32 txDataByte)
    {
        port_OP_UartPutChar(txDataByte);  /* Blocks control flow until all data has been sent */
        port_OP_UartPutChar(0x0Du);       /* Blocks control flow until all data has been sent */
        port_OP_UartPutChar(0x0Au);       /* Blocks control flow until all data has been sent */
    }


    #if !(port_OP_CY_SCBIP_V0 || port_OP_CY_SCBIP_V1)
        /*******************************************************************************
        * Function Name: port_OPSCB_UartEnableCts
        ****************************************************************************//**
        *
        *  Enables usage of CTS input signal by the UART transmitter.
        *  Only available for PSoC 4100 BLE / PSoC 4200 BLE / PSoC 4100M / PSoC 4200M /
        *  PSoC 4200L / PSoC 4000S / PSoC 4100S / PSoC Analog Coprocessor devices.
        *
        *******************************************************************************/
        void port_OP_UartEnableCts(void)
        {
            port_OP_UART_FLOW_CTRL_REG |= (uint32)  port_OP_UART_FLOW_CTRL_CTS_ENABLE;
        }


        /*******************************************************************************
        * Function Name: port_OP_UartDisableCts
        ****************************************************************************//**
        *
        *  Disables usage of CTS input signal by the UART transmitter.
        *  Only available for PSoC 4100 BLE / PSoC 4200 BLE / PSoC 4100M / PSoC 4200M /
        *  PSoC 4200L / PSoC 4000S / PSoC 4100S / PSoC Analog Coprocessor devices.
        *
        *******************************************************************************/
        void port_OP_UartDisableCts(void)
        {
            port_OP_UART_FLOW_CTRL_REG &= (uint32) ~port_OP_UART_FLOW_CTRL_CTS_ENABLE;
        }


        /*******************************************************************************
        * Function Name: port_OP_UartSetCtsPolarity
        ****************************************************************************//**
        *
        *  Sets active polarity of CTS input signal.
        *  Only available for PSoC 4100 BLE / PSoC 4200 BLE / PSoC 4100M / PSoC 4200M /
        *  PSoC 4200L / PSoC 4000S / PSoC 4100S / PSoC Analog Coprocessor devices.
        *
        * \param
        * polarity: Active polarity of CTS output signal.
        *   - port_OP_UART_CTS_ACTIVE_LOW  - CTS signal is active low.
        *   - port_OP_UART_CTS_ACTIVE_HIGH - CTS signal is active high.
        *
        *******************************************************************************/
        void port_OP_UartSetCtsPolarity(uint32 polarity)
        {
            if (0u != polarity)
            {
                port_OP_UART_FLOW_CTRL_REG |= (uint32)  port_OP_UART_FLOW_CTRL_CTS_POLARITY;
            }
            else
            {
                port_OP_UART_FLOW_CTRL_REG &= (uint32) ~port_OP_UART_FLOW_CTRL_CTS_POLARITY;
            }
        }
    #endif /* !(port_OP_CY_SCBIP_V0 || port_OP_CY_SCBIP_V1) */


    /*******************************************************************************
    * Function Name: port_OP_UartSendBreakBlocking
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
    void port_OP_UartSendBreakBlocking(uint32 breakWidth)
    {
        uint32 txCtrlReg;
        uint32 txIntrReg;

        /* Disable all UART TX interrupt source and clear UART TX Done history */
        txIntrReg = port_OP_GetTxInterruptMode();
        port_OP_SetTxInterruptMode(0u);
        port_OP_ClearTxInterruptSource(port_OP_INTR_TX_UART_DONE);

        /* Store TX CTRL configuration */
        txCtrlReg = port_OP_TX_CTRL_REG;

        /* Set break width */
        port_OP_TX_CTRL_REG = (port_OP_TX_CTRL_REG & (uint32) ~port_OP_TX_CTRL_DATA_WIDTH_MASK) |
                                        port_OP_GET_TX_CTRL_DATA_WIDTH(breakWidth);

        /* Generate break */
        port_OP_TX_FIFO_WR_REG = 0u;

        /* Wait for break completion */
        while (0u == (port_OP_GetTxInterruptSource() & port_OP_INTR_TX_UART_DONE))
        {
        }

        /* Clear all UART TX interrupt sources to  */
        port_OP_ClearTxInterruptSource(port_OP_INTR_TX_ALL);

        /* Restore TX interrupt sources and data width */
        port_OP_TX_CTRL_REG = txCtrlReg;
        port_OP_SetTxInterruptMode(txIntrReg);
    }
#endif /* (port_OP_UART_TX_DIRECTION) */


#if (port_OP_UART_WAKE_ENABLE_CONST)
    /*******************************************************************************
    * Function Name: port_OP_UartSaveConfig
    ****************************************************************************//**
    *
    *  Clears and enables an interrupt on a falling edge of the Rx input. The GPIO
    *  interrupt does not track in the active mode, therefore requires to be
    *  cleared by this API.
    *
    *******************************************************************************/
    void port_OP_UartSaveConfig(void)
    {
    #if (port_OP_UART_RX_WAKEUP_IRQ)
        /* Set SKIP_START if requested (set by default). */
        if (0u != port_OP_skipStart)
        {
            port_OP_UART_RX_CTRL_REG |= (uint32)  port_OP_UART_RX_CTRL_SKIP_START;
        }
        else
        {
            port_OP_UART_RX_CTRL_REG &= (uint32) ~port_OP_UART_RX_CTRL_SKIP_START;
        }

        /* Clear RX GPIO interrupt status and pending interrupt in NVIC because
        * falling edge on RX line occurs while UART communication in active mode.
        * Enable interrupt: next interrupt trigger should wakeup device.
        */
        port_OP_CLEAR_UART_RX_WAKE_INTR;
        port_OP_RxWakeClearPendingInt();
        port_OP_RxWakeEnableInt();
    #endif /* (port_OP_UART_RX_WAKEUP_IRQ) */
    }


    /*******************************************************************************
    * Function Name: port_OP_UartRestoreConfig
    ****************************************************************************//**
    *
    *  Disables the RX GPIO interrupt. Until this function is called the interrupt
    *  remains active and triggers on every falling edge of the UART RX line.
    *
    *******************************************************************************/
    void port_OP_UartRestoreConfig(void)
    {
    #if (port_OP_UART_RX_WAKEUP_IRQ)
        /* Disable interrupt: no more triggers in active mode */
        port_OP_RxWakeDisableInt();
    #endif /* (port_OP_UART_RX_WAKEUP_IRQ) */
    }


    #if (port_OP_UART_RX_WAKEUP_IRQ)
        /*******************************************************************************
        * Function Name: port_OP_UART_WAKEUP_ISR
        ****************************************************************************//**
        *
        *  Handles the Interrupt Service Routine for the SCB UART mode GPIO wakeup
        *  event. This event is configured to trigger on a falling edge of the RX line.
        *
        *******************************************************************************/
        CY_ISR(port_OP_UART_WAKEUP_ISR)
        {
        #ifdef port_OP_UART_WAKEUP_ISR_ENTRY_CALLBACK
            port_OP_UART_WAKEUP_ISR_EntryCallback();
        #endif /* port_OP_UART_WAKEUP_ISR_ENTRY_CALLBACK */

            port_OP_CLEAR_UART_RX_WAKE_INTR;

        #ifdef port_OP_UART_WAKEUP_ISR_EXIT_CALLBACK
            port_OP_UART_WAKEUP_ISR_ExitCallback();
        #endif /* port_OP_UART_WAKEUP_ISR_EXIT_CALLBACK */
        }
    #endif /* (port_OP_UART_RX_WAKEUP_IRQ) */
#endif /* (port_OP_UART_RX_WAKEUP_IRQ) */


/* [] END OF FILE */
