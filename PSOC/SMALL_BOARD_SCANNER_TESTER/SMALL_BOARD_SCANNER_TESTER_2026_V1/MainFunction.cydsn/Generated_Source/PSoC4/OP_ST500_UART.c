/***************************************************************************//**
* \file OP_ST500_UART.c
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

#include "OP_ST500_PVT.h"
#include "OP_ST500_SPI_UART_PVT.h"
#include "cyapicallbacks.h"

#if (OP_ST500_UART_WAKE_ENABLE_CONST && OP_ST500_UART_RX_WAKEUP_IRQ)
    /**
    * \addtogroup group_globals
    * \{
    */
    /** This global variable determines whether to enable Skip Start
    * functionality when OP_ST500_Sleep() function is called:
    * 0 – disable, other values – enable. Default value is 1.
    * It is only available when Enable wakeup from Deep Sleep Mode is enabled.
    */
    uint8 OP_ST500_skipStart = 1u;
    /** \} globals */
#endif /* (OP_ST500_UART_WAKE_ENABLE_CONST && OP_ST500_UART_RX_WAKEUP_IRQ) */

#if(OP_ST500_SCB_MODE_UNCONFIG_CONST_CFG)

    /***************************************
    *  Configuration Structure Initialization
    ***************************************/

    const OP_ST500_UART_INIT_STRUCT OP_ST500_configUart =
    {
        OP_ST500_UART_SUB_MODE,
        OP_ST500_UART_DIRECTION,
        OP_ST500_UART_DATA_BITS_NUM,
        OP_ST500_UART_PARITY_TYPE,
        OP_ST500_UART_STOP_BITS_NUM,
        OP_ST500_UART_OVS_FACTOR,
        OP_ST500_UART_IRDA_LOW_POWER,
        OP_ST500_UART_MEDIAN_FILTER_ENABLE,
        OP_ST500_UART_RETRY_ON_NACK,
        OP_ST500_UART_IRDA_POLARITY,
        OP_ST500_UART_DROP_ON_PARITY_ERR,
        OP_ST500_UART_DROP_ON_FRAME_ERR,
        OP_ST500_UART_WAKE_ENABLE,
        0u,
        NULL,
        0u,
        NULL,
        OP_ST500_UART_MP_MODE_ENABLE,
        OP_ST500_UART_MP_ACCEPT_ADDRESS,
        OP_ST500_UART_MP_RX_ADDRESS,
        OP_ST500_UART_MP_RX_ADDRESS_MASK,
        (uint32) OP_ST500_SCB_IRQ_INTERNAL,
        OP_ST500_UART_INTR_RX_MASK,
        OP_ST500_UART_RX_TRIGGER_LEVEL,
        OP_ST500_UART_INTR_TX_MASK,
        OP_ST500_UART_TX_TRIGGER_LEVEL,
        (uint8) OP_ST500_UART_BYTE_MODE_ENABLE,
        (uint8) OP_ST500_UART_CTS_ENABLE,
        (uint8) OP_ST500_UART_CTS_POLARITY,
        (uint8) OP_ST500_UART_RTS_POLARITY,
        (uint8) OP_ST500_UART_RTS_FIFO_LEVEL,
        (uint8) OP_ST500_UART_RX_BREAK_WIDTH
    };


    /*******************************************************************************
    * Function Name: OP_ST500_UartInit
    ****************************************************************************//**
    *
    *  Configures the OP_ST500 for UART operation.
    *
    *  This function is intended specifically to be used when the OP_ST500
    *  configuration is set to “Unconfigured OP_ST500” in the customizer.
    *  After initializing the OP_ST500 in UART mode using this function,
    *  the component can be enabled using the OP_ST500_Start() or
    * OP_ST500_Enable() function.
    *  This function uses a pointer to a structure that provides the configuration
    *  settings. This structure contains the same information that would otherwise
    *  be provided by the customizer settings.
    *
    *  \param config: pointer to a structure that contains the following list of
    *   fields. These fields match the selections available in the customizer.
    *   Refer to the customizer for further description of the settings.
    *
    *******************************************************************************/
    void OP_ST500_UartInit(const OP_ST500_UART_INIT_STRUCT *config)
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

        #if !(OP_ST500_CY_SCBIP_V0 || OP_ST500_CY_SCBIP_V1)
            /* Add RTS and CTS pins to configure */
            pinsConfig |= (0u != config->rtsRxFifoLevel) ? (OP_ST500_UART_RTS_PIN_ENABLE) : (0u);
            pinsConfig |= (0u != config->enableCts)      ? (OP_ST500_UART_CTS_PIN_ENABLE) : (0u);
        #endif /* !(OP_ST500_CY_SCBIP_V0 || OP_ST500_CY_SCBIP_V1) */

            /* Configure pins */
            OP_ST500_SetPins(OP_ST500_SCB_MODE_UART, config->mode, pinsConfig);

            /* Store internal configuration */
            OP_ST500_scbMode       = (uint8) OP_ST500_SCB_MODE_UART;
            OP_ST500_scbEnableWake = (uint8) config->enableWake;
            OP_ST500_scbEnableIntr = (uint8) config->enableInterrupt;

            /* Set RX direction internal variables */
            OP_ST500_rxBuffer      =         config->rxBuffer;
            OP_ST500_rxDataBits    = (uint8) config->dataBits;
            OP_ST500_rxBufferSize  =         config->rxBufferSize;

            /* Set TX direction internal variables */
            OP_ST500_txBuffer      =         config->txBuffer;
            OP_ST500_txDataBits    = (uint8) config->dataBits;
            OP_ST500_txBufferSize  =         config->txBufferSize;

            /* Configure UART interface */
            if(OP_ST500_UART_MODE_IRDA == config->mode)
            {
                /* OVS settings: IrDA */
                OP_ST500_CTRL_REG  = ((0u != config->enableIrdaLowPower) ?
                                                (OP_ST500_UART_GET_CTRL_OVS_IRDA_LP(config->oversample)) :
                                                (OP_ST500_CTRL_OVS_IRDA_OVS16));
            }
            else
            {
                /* OVS settings: UART and SmartCard */
                OP_ST500_CTRL_REG  = OP_ST500_GET_CTRL_OVS(config->oversample);
            }

            OP_ST500_CTRL_REG     |= OP_ST500_GET_CTRL_BYTE_MODE  (config->enableByteMode)      |
                                             OP_ST500_GET_CTRL_ADDR_ACCEPT(config->multiprocAcceptAddr) |
                                             OP_ST500_CTRL_UART;

            /* Configure sub-mode: UART, SmartCard or IrDA */
            OP_ST500_UART_CTRL_REG = OP_ST500_GET_UART_CTRL_MODE(config->mode);

            /* Configure RX direction */
            OP_ST500_UART_RX_CTRL_REG = OP_ST500_GET_UART_RX_CTRL_MODE(config->stopBits)              |
                                        OP_ST500_GET_UART_RX_CTRL_POLARITY(config->enableInvertedRx)          |
                                        OP_ST500_GET_UART_RX_CTRL_MP_MODE(config->enableMultiproc)            |
                                        OP_ST500_GET_UART_RX_CTRL_DROP_ON_PARITY_ERR(config->dropOnParityErr) |
                                        OP_ST500_GET_UART_RX_CTRL_DROP_ON_FRAME_ERR(config->dropOnFrameErr)   |
                                        OP_ST500_GET_UART_RX_CTRL_BREAK_WIDTH(config->breakWidth);

            if(OP_ST500_UART_PARITY_NONE != config->parity)
            {
               OP_ST500_UART_RX_CTRL_REG |= OP_ST500_GET_UART_RX_CTRL_PARITY(config->parity) |
                                                    OP_ST500_UART_RX_CTRL_PARITY_ENABLED;
            }

            OP_ST500_RX_CTRL_REG      = OP_ST500_GET_RX_CTRL_DATA_WIDTH(config->dataBits)       |
                                                OP_ST500_GET_RX_CTRL_MEDIAN(config->enableMedianFilter) |
                                                OP_ST500_GET_UART_RX_CTRL_ENABLED(config->direction);

            OP_ST500_RX_FIFO_CTRL_REG = OP_ST500_GET_RX_FIFO_CTRL_TRIGGER_LEVEL(config->rxTriggerLevel);

            /* Configure MP address */
            OP_ST500_RX_MATCH_REG     = OP_ST500_GET_RX_MATCH_ADDR(config->multiprocAddr) |
                                                OP_ST500_GET_RX_MATCH_MASK(config->multiprocAddrMask);

            /* Configure RX direction */
            OP_ST500_UART_TX_CTRL_REG = OP_ST500_GET_UART_TX_CTRL_MODE(config->stopBits) |
                                                OP_ST500_GET_UART_TX_CTRL_RETRY_NACK(config->enableRetryNack);

            if(OP_ST500_UART_PARITY_NONE != config->parity)
            {
               OP_ST500_UART_TX_CTRL_REG |= OP_ST500_GET_UART_TX_CTRL_PARITY(config->parity) |
                                                    OP_ST500_UART_TX_CTRL_PARITY_ENABLED;
            }

            OP_ST500_TX_CTRL_REG      = OP_ST500_GET_TX_CTRL_DATA_WIDTH(config->dataBits)    |
                                                OP_ST500_GET_UART_TX_CTRL_ENABLED(config->direction);

            OP_ST500_TX_FIFO_CTRL_REG = OP_ST500_GET_TX_FIFO_CTRL_TRIGGER_LEVEL(config->txTriggerLevel);

        #if !(OP_ST500_CY_SCBIP_V0 || OP_ST500_CY_SCBIP_V1)
            OP_ST500_UART_FLOW_CTRL_REG = OP_ST500_GET_UART_FLOW_CTRL_CTS_ENABLE(config->enableCts) | \
                                            OP_ST500_GET_UART_FLOW_CTRL_CTS_POLARITY (config->ctsPolarity)  | \
                                            OP_ST500_GET_UART_FLOW_CTRL_RTS_POLARITY (config->rtsPolarity)  | \
                                            OP_ST500_GET_UART_FLOW_CTRL_TRIGGER_LEVEL(config->rtsRxFifoLevel);
        #endif /* !(OP_ST500_CY_SCBIP_V0 || OP_ST500_CY_SCBIP_V1) */

            /* Configure interrupt with UART handler but do not enable it */
            CyIntDisable    (OP_ST500_ISR_NUMBER);
            CyIntSetPriority(OP_ST500_ISR_NUMBER, OP_ST500_ISR_PRIORITY);
            (void) CyIntSetVector(OP_ST500_ISR_NUMBER, &OP_ST500_SPI_UART_ISR);

            /* Configure WAKE interrupt */
        #if(OP_ST500_UART_RX_WAKEUP_IRQ)
            CyIntDisable    (OP_ST500_RX_WAKE_ISR_NUMBER);
            CyIntSetPriority(OP_ST500_RX_WAKE_ISR_NUMBER, OP_ST500_RX_WAKE_ISR_PRIORITY);
            (void) CyIntSetVector(OP_ST500_RX_WAKE_ISR_NUMBER, &OP_ST500_UART_WAKEUP_ISR);
        #endif /* (OP_ST500_UART_RX_WAKEUP_IRQ) */

            /* Configure interrupt sources */
            OP_ST500_INTR_I2C_EC_MASK_REG = OP_ST500_NO_INTR_SOURCES;
            OP_ST500_INTR_SPI_EC_MASK_REG = OP_ST500_NO_INTR_SOURCES;
            OP_ST500_INTR_SLAVE_MASK_REG  = OP_ST500_NO_INTR_SOURCES;
            OP_ST500_INTR_MASTER_MASK_REG = OP_ST500_NO_INTR_SOURCES;
            OP_ST500_INTR_RX_MASK_REG     = config->rxInterruptMask;
            OP_ST500_INTR_TX_MASK_REG     = config->txInterruptMask;

            /* Configure TX interrupt sources to restore. */
            OP_ST500_IntrTxMask = LO16(OP_ST500_INTR_TX_MASK_REG);

            /* Clear RX buffer indexes */
            OP_ST500_rxBufferHead     = 0u;
            OP_ST500_rxBufferTail     = 0u;
            OP_ST500_rxBufferOverflow = 0u;

            /* Clear TX buffer indexes */
            OP_ST500_txBufferHead = 0u;
            OP_ST500_txBufferTail = 0u;
        }
    }

#else

    /*******************************************************************************
    * Function Name: OP_ST500_UartInit
    ****************************************************************************//**
    *
    *  Configures the SCB for the UART operation.
    *
    *******************************************************************************/
    void OP_ST500_UartInit(void)
    {
        /* Configure UART interface */
        OP_ST500_CTRL_REG = OP_ST500_UART_DEFAULT_CTRL;

        /* Configure sub-mode: UART, SmartCard or IrDA */
        OP_ST500_UART_CTRL_REG = OP_ST500_UART_DEFAULT_UART_CTRL;

        /* Configure RX direction */
        OP_ST500_UART_RX_CTRL_REG = OP_ST500_UART_DEFAULT_UART_RX_CTRL;
        OP_ST500_RX_CTRL_REG      = OP_ST500_UART_DEFAULT_RX_CTRL;
        OP_ST500_RX_FIFO_CTRL_REG = OP_ST500_UART_DEFAULT_RX_FIFO_CTRL;
        OP_ST500_RX_MATCH_REG     = OP_ST500_UART_DEFAULT_RX_MATCH_REG;

        /* Configure TX direction */
        OP_ST500_UART_TX_CTRL_REG = OP_ST500_UART_DEFAULT_UART_TX_CTRL;
        OP_ST500_TX_CTRL_REG      = OP_ST500_UART_DEFAULT_TX_CTRL;
        OP_ST500_TX_FIFO_CTRL_REG = OP_ST500_UART_DEFAULT_TX_FIFO_CTRL;

    #if !(OP_ST500_CY_SCBIP_V0 || OP_ST500_CY_SCBIP_V1)
        OP_ST500_UART_FLOW_CTRL_REG = OP_ST500_UART_DEFAULT_FLOW_CTRL;
    #endif /* !(OP_ST500_CY_SCBIP_V0 || OP_ST500_CY_SCBIP_V1) */

        /* Configure interrupt with UART handler but do not enable it */
    #if(OP_ST500_SCB_IRQ_INTERNAL)
        CyIntDisable    (OP_ST500_ISR_NUMBER);
        CyIntSetPriority(OP_ST500_ISR_NUMBER, OP_ST500_ISR_PRIORITY);
        (void) CyIntSetVector(OP_ST500_ISR_NUMBER, &OP_ST500_SPI_UART_ISR);
    #endif /* (OP_ST500_SCB_IRQ_INTERNAL) */

        /* Configure WAKE interrupt */
    #if(OP_ST500_UART_RX_WAKEUP_IRQ)
        CyIntDisable    (OP_ST500_RX_WAKE_ISR_NUMBER);
        CyIntSetPriority(OP_ST500_RX_WAKE_ISR_NUMBER, OP_ST500_RX_WAKE_ISR_PRIORITY);
        (void) CyIntSetVector(OP_ST500_RX_WAKE_ISR_NUMBER, &OP_ST500_UART_WAKEUP_ISR);
    #endif /* (OP_ST500_UART_RX_WAKEUP_IRQ) */

        /* Configure interrupt sources */
        OP_ST500_INTR_I2C_EC_MASK_REG = OP_ST500_UART_DEFAULT_INTR_I2C_EC_MASK;
        OP_ST500_INTR_SPI_EC_MASK_REG = OP_ST500_UART_DEFAULT_INTR_SPI_EC_MASK;
        OP_ST500_INTR_SLAVE_MASK_REG  = OP_ST500_UART_DEFAULT_INTR_SLAVE_MASK;
        OP_ST500_INTR_MASTER_MASK_REG = OP_ST500_UART_DEFAULT_INTR_MASTER_MASK;
        OP_ST500_INTR_RX_MASK_REG     = OP_ST500_UART_DEFAULT_INTR_RX_MASK;
        OP_ST500_INTR_TX_MASK_REG     = OP_ST500_UART_DEFAULT_INTR_TX_MASK;

        /* Configure TX interrupt sources to restore. */
        OP_ST500_IntrTxMask = LO16(OP_ST500_INTR_TX_MASK_REG);

    #if(OP_ST500_INTERNAL_RX_SW_BUFFER_CONST)
        OP_ST500_rxBufferHead     = 0u;
        OP_ST500_rxBufferTail     = 0u;
        OP_ST500_rxBufferOverflow = 0u;
    #endif /* (OP_ST500_INTERNAL_RX_SW_BUFFER_CONST) */

    #if(OP_ST500_INTERNAL_TX_SW_BUFFER_CONST)
        OP_ST500_txBufferHead = 0u;
        OP_ST500_txBufferTail = 0u;
    #endif /* (OP_ST500_INTERNAL_TX_SW_BUFFER_CONST) */
    }
#endif /* (OP_ST500_SCB_MODE_UNCONFIG_CONST_CFG) */


/*******************************************************************************
* Function Name: OP_ST500_UartPostEnable
****************************************************************************//**
*
*  Restores HSIOM settings for the UART output pins (TX and/or RTS) to be
*  controlled by the SCB UART.
*
*******************************************************************************/
void OP_ST500_UartPostEnable(void)
{
#if (OP_ST500_SCB_MODE_UNCONFIG_CONST_CFG)
    #if (OP_ST500_TX_SDA_MISO_PIN)
        if (OP_ST500_CHECK_TX_SDA_MISO_PIN_USED)
        {
            /* Set SCB UART to drive the output pin */
            OP_ST500_SET_HSIOM_SEL(OP_ST500_TX_SDA_MISO_HSIOM_REG, OP_ST500_TX_SDA_MISO_HSIOM_MASK,
                                           OP_ST500_TX_SDA_MISO_HSIOM_POS, OP_ST500_TX_SDA_MISO_HSIOM_SEL_UART);
        }
    #endif /* (OP_ST500_TX_SDA_MISO_PIN_PIN) */

    #if !(OP_ST500_CY_SCBIP_V0 || OP_ST500_CY_SCBIP_V1)
        #if (OP_ST500_RTS_SS0_PIN)
            if (OP_ST500_CHECK_RTS_SS0_PIN_USED)
            {
                /* Set SCB UART to drive the output pin */
                OP_ST500_SET_HSIOM_SEL(OP_ST500_RTS_SS0_HSIOM_REG, OP_ST500_RTS_SS0_HSIOM_MASK,
                                               OP_ST500_RTS_SS0_HSIOM_POS, OP_ST500_RTS_SS0_HSIOM_SEL_UART);
            }
        #endif /* (OP_ST500_RTS_SS0_PIN) */
    #endif /* !(OP_ST500_CY_SCBIP_V0 || OP_ST500_CY_SCBIP_V1) */

#else
    #if (OP_ST500_UART_TX_PIN)
         /* Set SCB UART to drive the output pin */
        OP_ST500_SET_HSIOM_SEL(OP_ST500_TX_HSIOM_REG, OP_ST500_TX_HSIOM_MASK,
                                       OP_ST500_TX_HSIOM_POS, OP_ST500_TX_HSIOM_SEL_UART);
    #endif /* (OP_ST500_UART_TX_PIN) */

    #if (OP_ST500_UART_RTS_PIN)
        /* Set SCB UART to drive the output pin */
        OP_ST500_SET_HSIOM_SEL(OP_ST500_RTS_HSIOM_REG, OP_ST500_RTS_HSIOM_MASK,
                                       OP_ST500_RTS_HSIOM_POS, OP_ST500_RTS_HSIOM_SEL_UART);
    #endif /* (OP_ST500_UART_RTS_PIN) */
#endif /* (OP_ST500_SCB_MODE_UNCONFIG_CONST_CFG) */

    /* Restore TX interrupt sources. */
    OP_ST500_SetTxInterruptMode(OP_ST500_IntrTxMask);
}


/*******************************************************************************
* Function Name: OP_ST500_UartStop
****************************************************************************//**
*
*  Changes the HSIOM settings for the UART output pins (TX and/or RTS) to keep
*  them inactive after the block is disabled. The output pins are controlled by
*  the GPIO data register. Also, the function disables the skip start feature
*  to not cause it to trigger after the component is enabled.
*
*******************************************************************************/
void OP_ST500_UartStop(void)
{
#if(OP_ST500_SCB_MODE_UNCONFIG_CONST_CFG)
    #if (OP_ST500_TX_SDA_MISO_PIN)
        if (OP_ST500_CHECK_TX_SDA_MISO_PIN_USED)
        {
            /* Set GPIO to drive output pin */
            OP_ST500_SET_HSIOM_SEL(OP_ST500_TX_SDA_MISO_HSIOM_REG, OP_ST500_TX_SDA_MISO_HSIOM_MASK,
                                           OP_ST500_TX_SDA_MISO_HSIOM_POS, OP_ST500_TX_SDA_MISO_HSIOM_SEL_GPIO);
        }
    #endif /* (OP_ST500_TX_SDA_MISO_PIN_PIN) */

    #if !(OP_ST500_CY_SCBIP_V0 || OP_ST500_CY_SCBIP_V1)
        #if (OP_ST500_RTS_SS0_PIN)
            if (OP_ST500_CHECK_RTS_SS0_PIN_USED)
            {
                /* Set output pin state after block is disabled */
                OP_ST500_uart_rts_spi_ss0_Write(OP_ST500_GET_UART_RTS_INACTIVE);

                /* Set GPIO to drive output pin */
                OP_ST500_SET_HSIOM_SEL(OP_ST500_RTS_SS0_HSIOM_REG, OP_ST500_RTS_SS0_HSIOM_MASK,
                                               OP_ST500_RTS_SS0_HSIOM_POS, OP_ST500_RTS_SS0_HSIOM_SEL_GPIO);
            }
        #endif /* (OP_ST500_RTS_SS0_PIN) */
    #endif /* !(OP_ST500_CY_SCBIP_V0 || OP_ST500_CY_SCBIP_V1) */

#else
    #if (OP_ST500_UART_TX_PIN)
        /* Set GPIO to drive output pin */
        OP_ST500_SET_HSIOM_SEL(OP_ST500_TX_HSIOM_REG, OP_ST500_TX_HSIOM_MASK,
                                       OP_ST500_TX_HSIOM_POS, OP_ST500_TX_HSIOM_SEL_GPIO);
    #endif /* (OP_ST500_UART_TX_PIN) */

    #if (OP_ST500_UART_RTS_PIN)
        /* Set output pin state after block is disabled */
        OP_ST500_rts_Write(OP_ST500_GET_UART_RTS_INACTIVE);

        /* Set GPIO to drive output pin */
        OP_ST500_SET_HSIOM_SEL(OP_ST500_RTS_HSIOM_REG, OP_ST500_RTS_HSIOM_MASK,
                                       OP_ST500_RTS_HSIOM_POS, OP_ST500_RTS_HSIOM_SEL_GPIO);
    #endif /* (OP_ST500_UART_RTS_PIN) */

#endif /* (OP_ST500_SCB_MODE_UNCONFIG_CONST_CFG) */

#if (OP_ST500_UART_WAKE_ENABLE_CONST)
    /* Disable skip start feature used for wakeup */
    OP_ST500_UART_RX_CTRL_REG &= (uint32) ~OP_ST500_UART_RX_CTRL_SKIP_START;
#endif /* (OP_ST500_UART_WAKE_ENABLE_CONST) */

    /* Store TX interrupt sources (exclude level triggered). */
    OP_ST500_IntrTxMask = LO16(OP_ST500_GetTxInterruptMode() & OP_ST500_INTR_UART_TX_RESTORE);
}


/*******************************************************************************
* Function Name: OP_ST500_UartSetRxAddress
****************************************************************************//**
*
*  Sets the hardware detectable receiver address for the UART in the
*  Multiprocessor mode.
*
*  \param address: Address for hardware address detection.
*
*******************************************************************************/
void OP_ST500_UartSetRxAddress(uint32 address)
{
     uint32 matchReg;

    matchReg = OP_ST500_RX_MATCH_REG;

    matchReg &= ((uint32) ~OP_ST500_RX_MATCH_ADDR_MASK); /* Clear address bits */
    matchReg |= ((uint32)  (address & OP_ST500_RX_MATCH_ADDR_MASK)); /* Set address  */

    OP_ST500_RX_MATCH_REG = matchReg;
}


/*******************************************************************************
* Function Name: OP_ST500_UartSetRxAddressMask
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
void OP_ST500_UartSetRxAddressMask(uint32 addressMask)
{
    uint32 matchReg;

    matchReg = OP_ST500_RX_MATCH_REG;

    matchReg &= ((uint32) ~OP_ST500_RX_MATCH_MASK_MASK); /* Clear address mask bits */
    matchReg |= ((uint32) (addressMask << OP_ST500_RX_MATCH_MASK_POS));

    OP_ST500_RX_MATCH_REG = matchReg;
}


#if(OP_ST500_UART_RX_DIRECTION)
    /*******************************************************************************
    * Function Name: OP_ST500_UartGetChar
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
    *   Check OP_ST500_rxBufferOverflow to capture that error condition.
    *
    *******************************************************************************/
    uint32 OP_ST500_UartGetChar(void)
    {
        uint32 rxData = 0u;

        /* Reads data only if there is data to read */
        if (0u != OP_ST500_SpiUartGetRxBufferSize())
        {
            rxData = OP_ST500_SpiUartReadRxData();
        }

        if (OP_ST500_CHECK_INTR_RX(OP_ST500_INTR_RX_ERR))
        {
            rxData = 0u; /* Error occurred: returns zero */
            OP_ST500_ClearRxInterruptSource(OP_ST500_INTR_RX_ERR);
        }

        return (rxData);
    }


    /*******************************************************************************
    * Function Name: OP_ST500_UartGetByte
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
    *   - OP_ST500_UART_RX_OVERFLOW - Attempt to write to a full
    *     receiver FIFO.
    *   - OP_ST500_UART_RX_UNDERFLOW    Attempt to read from an empty
    *     receiver FIFO.
    *   - OP_ST500_UART_RX_FRAME_ERROR - UART framing error detected.
    *   - OP_ST500_UART_RX_PARITY_ERROR - UART parity error detected.
    *
    *  \sideeffect
    *   The errors bits may not correspond with reading characters due to
    *   RX FIFO and software buffer usage.
    *   RX software buffer is enabled: The internal software buffer overflow
    *   is not treated as an error condition.
    *   Check OP_ST500_rxBufferOverflow to capture that error condition.
    *
    *******************************************************************************/
    uint32 OP_ST500_UartGetByte(void)
    {
        uint32 rxData;
        uint32 tmpStatus;

        #if (OP_ST500_CHECK_RX_SW_BUFFER)
        {
            OP_ST500_DisableInt();
        }
        #endif

        if (0u != OP_ST500_SpiUartGetRxBufferSize())
        {
            /* Enables interrupt to receive more bytes: at least one byte is in
            * buffer.
            */
            #if (OP_ST500_CHECK_RX_SW_BUFFER)
            {
                OP_ST500_EnableInt();
            }
            #endif

            /* Get received byte */
            rxData = OP_ST500_SpiUartReadRxData();
        }
        else
        {
            /* Reads a byte directly from RX FIFO: underflow is raised in the
            * case of empty. Otherwise the first received byte will be read.
            */
            rxData = OP_ST500_RX_FIFO_RD_REG;


            /* Enables interrupt to receive more bytes. */
            #if (OP_ST500_CHECK_RX_SW_BUFFER)
            {

                /* The byte has been read from RX FIFO. Clear RX interrupt to
                * not involve interrupt handler when RX FIFO is empty.
                */
                OP_ST500_ClearRxInterruptSource(OP_ST500_INTR_RX_NOT_EMPTY);

                OP_ST500_EnableInt();
            }
            #endif
        }

        /* Get and clear RX error mask */
        tmpStatus = (OP_ST500_GetRxInterruptSource() & OP_ST500_INTR_RX_ERR);
        OP_ST500_ClearRxInterruptSource(OP_ST500_INTR_RX_ERR);

        /* Puts together data and error status:
        * MP mode and accept address: 9th bit is set to notify mark.
        */
        rxData |= ((uint32) (tmpStatus << 8u));

        return (rxData);
    }


    #if !(OP_ST500_CY_SCBIP_V0 || OP_ST500_CY_SCBIP_V1)
        /*******************************************************************************
        * Function Name: OP_ST500_UartSetRtsPolarity
        ****************************************************************************//**
        *
        *  Sets active polarity of RTS output signal.
        *  Only available for PSoC 4100 BLE / PSoC 4200 BLE / PSoC 4100M / PSoC 4200M /
        *  PSoC 4200L / PSoC 4000S / PSoC 4100S / PSoC Analog Coprocessor devices.
        *
        *  \param polarity: Active polarity of RTS output signal.
        *   - OP_ST500_UART_RTS_ACTIVE_LOW  - RTS signal is active low.
        *   - OP_ST500_UART_RTS_ACTIVE_HIGH - RTS signal is active high.
        *
        *******************************************************************************/
        void OP_ST500_UartSetRtsPolarity(uint32 polarity)
        {
            if(0u != polarity)
            {
                OP_ST500_UART_FLOW_CTRL_REG |= (uint32)  OP_ST500_UART_FLOW_CTRL_RTS_POLARITY;
            }
            else
            {
                OP_ST500_UART_FLOW_CTRL_REG &= (uint32) ~OP_ST500_UART_FLOW_CTRL_RTS_POLARITY;
            }
        }


        /*******************************************************************************
        * Function Name: OP_ST500_UartSetRtsFifoLevel
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
        void OP_ST500_UartSetRtsFifoLevel(uint32 level)
        {
            uint32 uartFlowCtrl;

            uartFlowCtrl = OP_ST500_UART_FLOW_CTRL_REG;

            uartFlowCtrl &= ((uint32) ~OP_ST500_UART_FLOW_CTRL_TRIGGER_LEVEL_MASK); /* Clear level mask bits */
            uartFlowCtrl |= ((uint32) (OP_ST500_UART_FLOW_CTRL_TRIGGER_LEVEL_MASK & level));

            OP_ST500_UART_FLOW_CTRL_REG = uartFlowCtrl;
        }
    #endif /* !(OP_ST500_CY_SCBIP_V0 || OP_ST500_CY_SCBIP_V1) */

#endif /* (OP_ST500_UART_RX_DIRECTION) */


#if(OP_ST500_UART_TX_DIRECTION)
    /*******************************************************************************
    * Function Name: OP_ST500_UartPutString
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
    void OP_ST500_UartPutString(const char8 string[])
    {
        uint32 bufIndex;

        bufIndex = 0u;

        /* Blocks the control flow until all data has been sent */
        while(string[bufIndex] != ((char8) 0))
        {
            OP_ST500_UartPutChar((uint32) string[bufIndex]);
            bufIndex++;
        }
    }


    /*******************************************************************************
    * Function Name: OP_ST500_UartPutCRLF
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
    void OP_ST500_UartPutCRLF(uint32 txDataByte)
    {
        OP_ST500_UartPutChar(txDataByte);  /* Blocks control flow until all data has been sent */
        OP_ST500_UartPutChar(0x0Du);       /* Blocks control flow until all data has been sent */
        OP_ST500_UartPutChar(0x0Au);       /* Blocks control flow until all data has been sent */
    }


    #if !(OP_ST500_CY_SCBIP_V0 || OP_ST500_CY_SCBIP_V1)
        /*******************************************************************************
        * Function Name: OP_ST500SCB_UartEnableCts
        ****************************************************************************//**
        *
        *  Enables usage of CTS input signal by the UART transmitter.
        *  Only available for PSoC 4100 BLE / PSoC 4200 BLE / PSoC 4100M / PSoC 4200M /
        *  PSoC 4200L / PSoC 4000S / PSoC 4100S / PSoC Analog Coprocessor devices.
        *
        *******************************************************************************/
        void OP_ST500_UartEnableCts(void)
        {
            OP_ST500_UART_FLOW_CTRL_REG |= (uint32)  OP_ST500_UART_FLOW_CTRL_CTS_ENABLE;
        }


        /*******************************************************************************
        * Function Name: OP_ST500_UartDisableCts
        ****************************************************************************//**
        *
        *  Disables usage of CTS input signal by the UART transmitter.
        *  Only available for PSoC 4100 BLE / PSoC 4200 BLE / PSoC 4100M / PSoC 4200M /
        *  PSoC 4200L / PSoC 4000S / PSoC 4100S / PSoC Analog Coprocessor devices.
        *
        *******************************************************************************/
        void OP_ST500_UartDisableCts(void)
        {
            OP_ST500_UART_FLOW_CTRL_REG &= (uint32) ~OP_ST500_UART_FLOW_CTRL_CTS_ENABLE;
        }


        /*******************************************************************************
        * Function Name: OP_ST500_UartSetCtsPolarity
        ****************************************************************************//**
        *
        *  Sets active polarity of CTS input signal.
        *  Only available for PSoC 4100 BLE / PSoC 4200 BLE / PSoC 4100M / PSoC 4200M /
        *  PSoC 4200L / PSoC 4000S / PSoC 4100S / PSoC Analog Coprocessor devices.
        *
        * \param
        * polarity: Active polarity of CTS output signal.
        *   - OP_ST500_UART_CTS_ACTIVE_LOW  - CTS signal is active low.
        *   - OP_ST500_UART_CTS_ACTIVE_HIGH - CTS signal is active high.
        *
        *******************************************************************************/
        void OP_ST500_UartSetCtsPolarity(uint32 polarity)
        {
            if (0u != polarity)
            {
                OP_ST500_UART_FLOW_CTRL_REG |= (uint32)  OP_ST500_UART_FLOW_CTRL_CTS_POLARITY;
            }
            else
            {
                OP_ST500_UART_FLOW_CTRL_REG &= (uint32) ~OP_ST500_UART_FLOW_CTRL_CTS_POLARITY;
            }
        }
    #endif /* !(OP_ST500_CY_SCBIP_V0 || OP_ST500_CY_SCBIP_V1) */


    /*******************************************************************************
    * Function Name: OP_ST500_UartSendBreakBlocking
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
    void OP_ST500_UartSendBreakBlocking(uint32 breakWidth)
    {
        uint32 txCtrlReg;
        uint32 txIntrReg;

        /* Disable all UART TX interrupt source and clear UART TX Done history */
        txIntrReg = OP_ST500_GetTxInterruptMode();
        OP_ST500_SetTxInterruptMode(0u);
        OP_ST500_ClearTxInterruptSource(OP_ST500_INTR_TX_UART_DONE);

        /* Store TX CTRL configuration */
        txCtrlReg = OP_ST500_TX_CTRL_REG;

        /* Set break width */
        OP_ST500_TX_CTRL_REG = (OP_ST500_TX_CTRL_REG & (uint32) ~OP_ST500_TX_CTRL_DATA_WIDTH_MASK) |
                                        OP_ST500_GET_TX_CTRL_DATA_WIDTH(breakWidth);

        /* Generate break */
        OP_ST500_TX_FIFO_WR_REG = 0u;

        /* Wait for break completion */
        while (0u == (OP_ST500_GetTxInterruptSource() & OP_ST500_INTR_TX_UART_DONE))
        {
        }

        /* Clear all UART TX interrupt sources to  */
        OP_ST500_ClearTxInterruptSource(OP_ST500_INTR_TX_ALL);

        /* Restore TX interrupt sources and data width */
        OP_ST500_TX_CTRL_REG = txCtrlReg;
        OP_ST500_SetTxInterruptMode(txIntrReg);
    }
#endif /* (OP_ST500_UART_TX_DIRECTION) */


#if (OP_ST500_UART_WAKE_ENABLE_CONST)
    /*******************************************************************************
    * Function Name: OP_ST500_UartSaveConfig
    ****************************************************************************//**
    *
    *  Clears and enables an interrupt on a falling edge of the Rx input. The GPIO
    *  interrupt does not track in the active mode, therefore requires to be
    *  cleared by this API.
    *
    *******************************************************************************/
    void OP_ST500_UartSaveConfig(void)
    {
    #if (OP_ST500_UART_RX_WAKEUP_IRQ)
        /* Set SKIP_START if requested (set by default). */
        if (0u != OP_ST500_skipStart)
        {
            OP_ST500_UART_RX_CTRL_REG |= (uint32)  OP_ST500_UART_RX_CTRL_SKIP_START;
        }
        else
        {
            OP_ST500_UART_RX_CTRL_REG &= (uint32) ~OP_ST500_UART_RX_CTRL_SKIP_START;
        }

        /* Clear RX GPIO interrupt status and pending interrupt in NVIC because
        * falling edge on RX line occurs while UART communication in active mode.
        * Enable interrupt: next interrupt trigger should wakeup device.
        */
        OP_ST500_CLEAR_UART_RX_WAKE_INTR;
        OP_ST500_RxWakeClearPendingInt();
        OP_ST500_RxWakeEnableInt();
    #endif /* (OP_ST500_UART_RX_WAKEUP_IRQ) */
    }


    /*******************************************************************************
    * Function Name: OP_ST500_UartRestoreConfig
    ****************************************************************************//**
    *
    *  Disables the RX GPIO interrupt. Until this function is called the interrupt
    *  remains active and triggers on every falling edge of the UART RX line.
    *
    *******************************************************************************/
    void OP_ST500_UartRestoreConfig(void)
    {
    #if (OP_ST500_UART_RX_WAKEUP_IRQ)
        /* Disable interrupt: no more triggers in active mode */
        OP_ST500_RxWakeDisableInt();
    #endif /* (OP_ST500_UART_RX_WAKEUP_IRQ) */
    }


    #if (OP_ST500_UART_RX_WAKEUP_IRQ)
        /*******************************************************************************
        * Function Name: OP_ST500_UART_WAKEUP_ISR
        ****************************************************************************//**
        *
        *  Handles the Interrupt Service Routine for the SCB UART mode GPIO wakeup
        *  event. This event is configured to trigger on a falling edge of the RX line.
        *
        *******************************************************************************/
        CY_ISR(OP_ST500_UART_WAKEUP_ISR)
        {
        #ifdef OP_ST500_UART_WAKEUP_ISR_ENTRY_CALLBACK
            OP_ST500_UART_WAKEUP_ISR_EntryCallback();
        #endif /* OP_ST500_UART_WAKEUP_ISR_ENTRY_CALLBACK */

            OP_ST500_CLEAR_UART_RX_WAKE_INTR;

        #ifdef OP_ST500_UART_WAKEUP_ISR_EXIT_CALLBACK
            OP_ST500_UART_WAKEUP_ISR_ExitCallback();
        #endif /* OP_ST500_UART_WAKEUP_ISR_EXIT_CALLBACK */
        }
    #endif /* (OP_ST500_UART_RX_WAKEUP_IRQ) */
#endif /* (OP_ST500_UART_RX_WAKEUP_IRQ) */


/* [] END OF FILE */
