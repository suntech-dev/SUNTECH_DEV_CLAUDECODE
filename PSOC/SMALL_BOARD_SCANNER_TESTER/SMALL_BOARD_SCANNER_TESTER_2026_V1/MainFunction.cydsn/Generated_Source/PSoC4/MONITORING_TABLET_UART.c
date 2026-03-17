/***************************************************************************//**
* \file MONITORING_TABLET_UART.c
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

#include "MONITORING_TABLET_PVT.h"
#include "MONITORING_TABLET_SPI_UART_PVT.h"
#include "cyapicallbacks.h"

#if (MONITORING_TABLET_UART_WAKE_ENABLE_CONST && MONITORING_TABLET_UART_RX_WAKEUP_IRQ)
    /**
    * \addtogroup group_globals
    * \{
    */
    /** This global variable determines whether to enable Skip Start
    * functionality when MONITORING_TABLET_Sleep() function is called:
    * 0 – disable, other values – enable. Default value is 1.
    * It is only available when Enable wakeup from Deep Sleep Mode is enabled.
    */
    uint8 MONITORING_TABLET_skipStart = 1u;
    /** \} globals */
#endif /* (MONITORING_TABLET_UART_WAKE_ENABLE_CONST && MONITORING_TABLET_UART_RX_WAKEUP_IRQ) */

#if(MONITORING_TABLET_SCB_MODE_UNCONFIG_CONST_CFG)

    /***************************************
    *  Configuration Structure Initialization
    ***************************************/

    const MONITORING_TABLET_UART_INIT_STRUCT MONITORING_TABLET_configUart =
    {
        MONITORING_TABLET_UART_SUB_MODE,
        MONITORING_TABLET_UART_DIRECTION,
        MONITORING_TABLET_UART_DATA_BITS_NUM,
        MONITORING_TABLET_UART_PARITY_TYPE,
        MONITORING_TABLET_UART_STOP_BITS_NUM,
        MONITORING_TABLET_UART_OVS_FACTOR,
        MONITORING_TABLET_UART_IRDA_LOW_POWER,
        MONITORING_TABLET_UART_MEDIAN_FILTER_ENABLE,
        MONITORING_TABLET_UART_RETRY_ON_NACK,
        MONITORING_TABLET_UART_IRDA_POLARITY,
        MONITORING_TABLET_UART_DROP_ON_PARITY_ERR,
        MONITORING_TABLET_UART_DROP_ON_FRAME_ERR,
        MONITORING_TABLET_UART_WAKE_ENABLE,
        0u,
        NULL,
        0u,
        NULL,
        MONITORING_TABLET_UART_MP_MODE_ENABLE,
        MONITORING_TABLET_UART_MP_ACCEPT_ADDRESS,
        MONITORING_TABLET_UART_MP_RX_ADDRESS,
        MONITORING_TABLET_UART_MP_RX_ADDRESS_MASK,
        (uint32) MONITORING_TABLET_SCB_IRQ_INTERNAL,
        MONITORING_TABLET_UART_INTR_RX_MASK,
        MONITORING_TABLET_UART_RX_TRIGGER_LEVEL,
        MONITORING_TABLET_UART_INTR_TX_MASK,
        MONITORING_TABLET_UART_TX_TRIGGER_LEVEL,
        (uint8) MONITORING_TABLET_UART_BYTE_MODE_ENABLE,
        (uint8) MONITORING_TABLET_UART_CTS_ENABLE,
        (uint8) MONITORING_TABLET_UART_CTS_POLARITY,
        (uint8) MONITORING_TABLET_UART_RTS_POLARITY,
        (uint8) MONITORING_TABLET_UART_RTS_FIFO_LEVEL,
        (uint8) MONITORING_TABLET_UART_RX_BREAK_WIDTH
    };


    /*******************************************************************************
    * Function Name: MONITORING_TABLET_UartInit
    ****************************************************************************//**
    *
    *  Configures the MONITORING_TABLET for UART operation.
    *
    *  This function is intended specifically to be used when the MONITORING_TABLET
    *  configuration is set to “Unconfigured MONITORING_TABLET” in the customizer.
    *  After initializing the MONITORING_TABLET in UART mode using this function,
    *  the component can be enabled using the MONITORING_TABLET_Start() or
    * MONITORING_TABLET_Enable() function.
    *  This function uses a pointer to a structure that provides the configuration
    *  settings. This structure contains the same information that would otherwise
    *  be provided by the customizer settings.
    *
    *  \param config: pointer to a structure that contains the following list of
    *   fields. These fields match the selections available in the customizer.
    *   Refer to the customizer for further description of the settings.
    *
    *******************************************************************************/
    void MONITORING_TABLET_UartInit(const MONITORING_TABLET_UART_INIT_STRUCT *config)
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

        #if !(MONITORING_TABLET_CY_SCBIP_V0 || MONITORING_TABLET_CY_SCBIP_V1)
            /* Add RTS and CTS pins to configure */
            pinsConfig |= (0u != config->rtsRxFifoLevel) ? (MONITORING_TABLET_UART_RTS_PIN_ENABLE) : (0u);
            pinsConfig |= (0u != config->enableCts)      ? (MONITORING_TABLET_UART_CTS_PIN_ENABLE) : (0u);
        #endif /* !(MONITORING_TABLET_CY_SCBIP_V0 || MONITORING_TABLET_CY_SCBIP_V1) */

            /* Configure pins */
            MONITORING_TABLET_SetPins(MONITORING_TABLET_SCB_MODE_UART, config->mode, pinsConfig);

            /* Store internal configuration */
            MONITORING_TABLET_scbMode       = (uint8) MONITORING_TABLET_SCB_MODE_UART;
            MONITORING_TABLET_scbEnableWake = (uint8) config->enableWake;
            MONITORING_TABLET_scbEnableIntr = (uint8) config->enableInterrupt;

            /* Set RX direction internal variables */
            MONITORING_TABLET_rxBuffer      =         config->rxBuffer;
            MONITORING_TABLET_rxDataBits    = (uint8) config->dataBits;
            MONITORING_TABLET_rxBufferSize  =         config->rxBufferSize;

            /* Set TX direction internal variables */
            MONITORING_TABLET_txBuffer      =         config->txBuffer;
            MONITORING_TABLET_txDataBits    = (uint8) config->dataBits;
            MONITORING_TABLET_txBufferSize  =         config->txBufferSize;

            /* Configure UART interface */
            if(MONITORING_TABLET_UART_MODE_IRDA == config->mode)
            {
                /* OVS settings: IrDA */
                MONITORING_TABLET_CTRL_REG  = ((0u != config->enableIrdaLowPower) ?
                                                (MONITORING_TABLET_UART_GET_CTRL_OVS_IRDA_LP(config->oversample)) :
                                                (MONITORING_TABLET_CTRL_OVS_IRDA_OVS16));
            }
            else
            {
                /* OVS settings: UART and SmartCard */
                MONITORING_TABLET_CTRL_REG  = MONITORING_TABLET_GET_CTRL_OVS(config->oversample);
            }

            MONITORING_TABLET_CTRL_REG     |= MONITORING_TABLET_GET_CTRL_BYTE_MODE  (config->enableByteMode)      |
                                             MONITORING_TABLET_GET_CTRL_ADDR_ACCEPT(config->multiprocAcceptAddr) |
                                             MONITORING_TABLET_CTRL_UART;

            /* Configure sub-mode: UART, SmartCard or IrDA */
            MONITORING_TABLET_UART_CTRL_REG = MONITORING_TABLET_GET_UART_CTRL_MODE(config->mode);

            /* Configure RX direction */
            MONITORING_TABLET_UART_RX_CTRL_REG = MONITORING_TABLET_GET_UART_RX_CTRL_MODE(config->stopBits)              |
                                        MONITORING_TABLET_GET_UART_RX_CTRL_POLARITY(config->enableInvertedRx)          |
                                        MONITORING_TABLET_GET_UART_RX_CTRL_MP_MODE(config->enableMultiproc)            |
                                        MONITORING_TABLET_GET_UART_RX_CTRL_DROP_ON_PARITY_ERR(config->dropOnParityErr) |
                                        MONITORING_TABLET_GET_UART_RX_CTRL_DROP_ON_FRAME_ERR(config->dropOnFrameErr)   |
                                        MONITORING_TABLET_GET_UART_RX_CTRL_BREAK_WIDTH(config->breakWidth);

            if(MONITORING_TABLET_UART_PARITY_NONE != config->parity)
            {
               MONITORING_TABLET_UART_RX_CTRL_REG |= MONITORING_TABLET_GET_UART_RX_CTRL_PARITY(config->parity) |
                                                    MONITORING_TABLET_UART_RX_CTRL_PARITY_ENABLED;
            }

            MONITORING_TABLET_RX_CTRL_REG      = MONITORING_TABLET_GET_RX_CTRL_DATA_WIDTH(config->dataBits)       |
                                                MONITORING_TABLET_GET_RX_CTRL_MEDIAN(config->enableMedianFilter) |
                                                MONITORING_TABLET_GET_UART_RX_CTRL_ENABLED(config->direction);

            MONITORING_TABLET_RX_FIFO_CTRL_REG = MONITORING_TABLET_GET_RX_FIFO_CTRL_TRIGGER_LEVEL(config->rxTriggerLevel);

            /* Configure MP address */
            MONITORING_TABLET_RX_MATCH_REG     = MONITORING_TABLET_GET_RX_MATCH_ADDR(config->multiprocAddr) |
                                                MONITORING_TABLET_GET_RX_MATCH_MASK(config->multiprocAddrMask);

            /* Configure RX direction */
            MONITORING_TABLET_UART_TX_CTRL_REG = MONITORING_TABLET_GET_UART_TX_CTRL_MODE(config->stopBits) |
                                                MONITORING_TABLET_GET_UART_TX_CTRL_RETRY_NACK(config->enableRetryNack);

            if(MONITORING_TABLET_UART_PARITY_NONE != config->parity)
            {
               MONITORING_TABLET_UART_TX_CTRL_REG |= MONITORING_TABLET_GET_UART_TX_CTRL_PARITY(config->parity) |
                                                    MONITORING_TABLET_UART_TX_CTRL_PARITY_ENABLED;
            }

            MONITORING_TABLET_TX_CTRL_REG      = MONITORING_TABLET_GET_TX_CTRL_DATA_WIDTH(config->dataBits)    |
                                                MONITORING_TABLET_GET_UART_TX_CTRL_ENABLED(config->direction);

            MONITORING_TABLET_TX_FIFO_CTRL_REG = MONITORING_TABLET_GET_TX_FIFO_CTRL_TRIGGER_LEVEL(config->txTriggerLevel);

        #if !(MONITORING_TABLET_CY_SCBIP_V0 || MONITORING_TABLET_CY_SCBIP_V1)
            MONITORING_TABLET_UART_FLOW_CTRL_REG = MONITORING_TABLET_GET_UART_FLOW_CTRL_CTS_ENABLE(config->enableCts) | \
                                            MONITORING_TABLET_GET_UART_FLOW_CTRL_CTS_POLARITY (config->ctsPolarity)  | \
                                            MONITORING_TABLET_GET_UART_FLOW_CTRL_RTS_POLARITY (config->rtsPolarity)  | \
                                            MONITORING_TABLET_GET_UART_FLOW_CTRL_TRIGGER_LEVEL(config->rtsRxFifoLevel);
        #endif /* !(MONITORING_TABLET_CY_SCBIP_V0 || MONITORING_TABLET_CY_SCBIP_V1) */

            /* Configure interrupt with UART handler but do not enable it */
            CyIntDisable    (MONITORING_TABLET_ISR_NUMBER);
            CyIntSetPriority(MONITORING_TABLET_ISR_NUMBER, MONITORING_TABLET_ISR_PRIORITY);
            (void) CyIntSetVector(MONITORING_TABLET_ISR_NUMBER, &MONITORING_TABLET_SPI_UART_ISR);

            /* Configure WAKE interrupt */
        #if(MONITORING_TABLET_UART_RX_WAKEUP_IRQ)
            CyIntDisable    (MONITORING_TABLET_RX_WAKE_ISR_NUMBER);
            CyIntSetPriority(MONITORING_TABLET_RX_WAKE_ISR_NUMBER, MONITORING_TABLET_RX_WAKE_ISR_PRIORITY);
            (void) CyIntSetVector(MONITORING_TABLET_RX_WAKE_ISR_NUMBER, &MONITORING_TABLET_UART_WAKEUP_ISR);
        #endif /* (MONITORING_TABLET_UART_RX_WAKEUP_IRQ) */

            /* Configure interrupt sources */
            MONITORING_TABLET_INTR_I2C_EC_MASK_REG = MONITORING_TABLET_NO_INTR_SOURCES;
            MONITORING_TABLET_INTR_SPI_EC_MASK_REG = MONITORING_TABLET_NO_INTR_SOURCES;
            MONITORING_TABLET_INTR_SLAVE_MASK_REG  = MONITORING_TABLET_NO_INTR_SOURCES;
            MONITORING_TABLET_INTR_MASTER_MASK_REG = MONITORING_TABLET_NO_INTR_SOURCES;
            MONITORING_TABLET_INTR_RX_MASK_REG     = config->rxInterruptMask;
            MONITORING_TABLET_INTR_TX_MASK_REG     = config->txInterruptMask;

            /* Configure TX interrupt sources to restore. */
            MONITORING_TABLET_IntrTxMask = LO16(MONITORING_TABLET_INTR_TX_MASK_REG);

            /* Clear RX buffer indexes */
            MONITORING_TABLET_rxBufferHead     = 0u;
            MONITORING_TABLET_rxBufferTail     = 0u;
            MONITORING_TABLET_rxBufferOverflow = 0u;

            /* Clear TX buffer indexes */
            MONITORING_TABLET_txBufferHead = 0u;
            MONITORING_TABLET_txBufferTail = 0u;
        }
    }

#else

    /*******************************************************************************
    * Function Name: MONITORING_TABLET_UartInit
    ****************************************************************************//**
    *
    *  Configures the SCB for the UART operation.
    *
    *******************************************************************************/
    void MONITORING_TABLET_UartInit(void)
    {
        /* Configure UART interface */
        MONITORING_TABLET_CTRL_REG = MONITORING_TABLET_UART_DEFAULT_CTRL;

        /* Configure sub-mode: UART, SmartCard or IrDA */
        MONITORING_TABLET_UART_CTRL_REG = MONITORING_TABLET_UART_DEFAULT_UART_CTRL;

        /* Configure RX direction */
        MONITORING_TABLET_UART_RX_CTRL_REG = MONITORING_TABLET_UART_DEFAULT_UART_RX_CTRL;
        MONITORING_TABLET_RX_CTRL_REG      = MONITORING_TABLET_UART_DEFAULT_RX_CTRL;
        MONITORING_TABLET_RX_FIFO_CTRL_REG = MONITORING_TABLET_UART_DEFAULT_RX_FIFO_CTRL;
        MONITORING_TABLET_RX_MATCH_REG     = MONITORING_TABLET_UART_DEFAULT_RX_MATCH_REG;

        /* Configure TX direction */
        MONITORING_TABLET_UART_TX_CTRL_REG = MONITORING_TABLET_UART_DEFAULT_UART_TX_CTRL;
        MONITORING_TABLET_TX_CTRL_REG      = MONITORING_TABLET_UART_DEFAULT_TX_CTRL;
        MONITORING_TABLET_TX_FIFO_CTRL_REG = MONITORING_TABLET_UART_DEFAULT_TX_FIFO_CTRL;

    #if !(MONITORING_TABLET_CY_SCBIP_V0 || MONITORING_TABLET_CY_SCBIP_V1)
        MONITORING_TABLET_UART_FLOW_CTRL_REG = MONITORING_TABLET_UART_DEFAULT_FLOW_CTRL;
    #endif /* !(MONITORING_TABLET_CY_SCBIP_V0 || MONITORING_TABLET_CY_SCBIP_V1) */

        /* Configure interrupt with UART handler but do not enable it */
    #if(MONITORING_TABLET_SCB_IRQ_INTERNAL)
        CyIntDisable    (MONITORING_TABLET_ISR_NUMBER);
        CyIntSetPriority(MONITORING_TABLET_ISR_NUMBER, MONITORING_TABLET_ISR_PRIORITY);
        (void) CyIntSetVector(MONITORING_TABLET_ISR_NUMBER, &MONITORING_TABLET_SPI_UART_ISR);
    #endif /* (MONITORING_TABLET_SCB_IRQ_INTERNAL) */

        /* Configure WAKE interrupt */
    #if(MONITORING_TABLET_UART_RX_WAKEUP_IRQ)
        CyIntDisable    (MONITORING_TABLET_RX_WAKE_ISR_NUMBER);
        CyIntSetPriority(MONITORING_TABLET_RX_WAKE_ISR_NUMBER, MONITORING_TABLET_RX_WAKE_ISR_PRIORITY);
        (void) CyIntSetVector(MONITORING_TABLET_RX_WAKE_ISR_NUMBER, &MONITORING_TABLET_UART_WAKEUP_ISR);
    #endif /* (MONITORING_TABLET_UART_RX_WAKEUP_IRQ) */

        /* Configure interrupt sources */
        MONITORING_TABLET_INTR_I2C_EC_MASK_REG = MONITORING_TABLET_UART_DEFAULT_INTR_I2C_EC_MASK;
        MONITORING_TABLET_INTR_SPI_EC_MASK_REG = MONITORING_TABLET_UART_DEFAULT_INTR_SPI_EC_MASK;
        MONITORING_TABLET_INTR_SLAVE_MASK_REG  = MONITORING_TABLET_UART_DEFAULT_INTR_SLAVE_MASK;
        MONITORING_TABLET_INTR_MASTER_MASK_REG = MONITORING_TABLET_UART_DEFAULT_INTR_MASTER_MASK;
        MONITORING_TABLET_INTR_RX_MASK_REG     = MONITORING_TABLET_UART_DEFAULT_INTR_RX_MASK;
        MONITORING_TABLET_INTR_TX_MASK_REG     = MONITORING_TABLET_UART_DEFAULT_INTR_TX_MASK;

        /* Configure TX interrupt sources to restore. */
        MONITORING_TABLET_IntrTxMask = LO16(MONITORING_TABLET_INTR_TX_MASK_REG);

    #if(MONITORING_TABLET_INTERNAL_RX_SW_BUFFER_CONST)
        MONITORING_TABLET_rxBufferHead     = 0u;
        MONITORING_TABLET_rxBufferTail     = 0u;
        MONITORING_TABLET_rxBufferOverflow = 0u;
    #endif /* (MONITORING_TABLET_INTERNAL_RX_SW_BUFFER_CONST) */

    #if(MONITORING_TABLET_INTERNAL_TX_SW_BUFFER_CONST)
        MONITORING_TABLET_txBufferHead = 0u;
        MONITORING_TABLET_txBufferTail = 0u;
    #endif /* (MONITORING_TABLET_INTERNAL_TX_SW_BUFFER_CONST) */
    }
#endif /* (MONITORING_TABLET_SCB_MODE_UNCONFIG_CONST_CFG) */


/*******************************************************************************
* Function Name: MONITORING_TABLET_UartPostEnable
****************************************************************************//**
*
*  Restores HSIOM settings for the UART output pins (TX and/or RTS) to be
*  controlled by the SCB UART.
*
*******************************************************************************/
void MONITORING_TABLET_UartPostEnable(void)
{
#if (MONITORING_TABLET_SCB_MODE_UNCONFIG_CONST_CFG)
    #if (MONITORING_TABLET_TX_SDA_MISO_PIN)
        if (MONITORING_TABLET_CHECK_TX_SDA_MISO_PIN_USED)
        {
            /* Set SCB UART to drive the output pin */
            MONITORING_TABLET_SET_HSIOM_SEL(MONITORING_TABLET_TX_SDA_MISO_HSIOM_REG, MONITORING_TABLET_TX_SDA_MISO_HSIOM_MASK,
                                           MONITORING_TABLET_TX_SDA_MISO_HSIOM_POS, MONITORING_TABLET_TX_SDA_MISO_HSIOM_SEL_UART);
        }
    #endif /* (MONITORING_TABLET_TX_SDA_MISO_PIN_PIN) */

    #if !(MONITORING_TABLET_CY_SCBIP_V0 || MONITORING_TABLET_CY_SCBIP_V1)
        #if (MONITORING_TABLET_RTS_SS0_PIN)
            if (MONITORING_TABLET_CHECK_RTS_SS0_PIN_USED)
            {
                /* Set SCB UART to drive the output pin */
                MONITORING_TABLET_SET_HSIOM_SEL(MONITORING_TABLET_RTS_SS0_HSIOM_REG, MONITORING_TABLET_RTS_SS0_HSIOM_MASK,
                                               MONITORING_TABLET_RTS_SS0_HSIOM_POS, MONITORING_TABLET_RTS_SS0_HSIOM_SEL_UART);
            }
        #endif /* (MONITORING_TABLET_RTS_SS0_PIN) */
    #endif /* !(MONITORING_TABLET_CY_SCBIP_V0 || MONITORING_TABLET_CY_SCBIP_V1) */

#else
    #if (MONITORING_TABLET_UART_TX_PIN)
         /* Set SCB UART to drive the output pin */
        MONITORING_TABLET_SET_HSIOM_SEL(MONITORING_TABLET_TX_HSIOM_REG, MONITORING_TABLET_TX_HSIOM_MASK,
                                       MONITORING_TABLET_TX_HSIOM_POS, MONITORING_TABLET_TX_HSIOM_SEL_UART);
    #endif /* (MONITORING_TABLET_UART_TX_PIN) */

    #if (MONITORING_TABLET_UART_RTS_PIN)
        /* Set SCB UART to drive the output pin */
        MONITORING_TABLET_SET_HSIOM_SEL(MONITORING_TABLET_RTS_HSIOM_REG, MONITORING_TABLET_RTS_HSIOM_MASK,
                                       MONITORING_TABLET_RTS_HSIOM_POS, MONITORING_TABLET_RTS_HSIOM_SEL_UART);
    #endif /* (MONITORING_TABLET_UART_RTS_PIN) */
#endif /* (MONITORING_TABLET_SCB_MODE_UNCONFIG_CONST_CFG) */

    /* Restore TX interrupt sources. */
    MONITORING_TABLET_SetTxInterruptMode(MONITORING_TABLET_IntrTxMask);
}


/*******************************************************************************
* Function Name: MONITORING_TABLET_UartStop
****************************************************************************//**
*
*  Changes the HSIOM settings for the UART output pins (TX and/or RTS) to keep
*  them inactive after the block is disabled. The output pins are controlled by
*  the GPIO data register. Also, the function disables the skip start feature
*  to not cause it to trigger after the component is enabled.
*
*******************************************************************************/
void MONITORING_TABLET_UartStop(void)
{
#if(MONITORING_TABLET_SCB_MODE_UNCONFIG_CONST_CFG)
    #if (MONITORING_TABLET_TX_SDA_MISO_PIN)
        if (MONITORING_TABLET_CHECK_TX_SDA_MISO_PIN_USED)
        {
            /* Set GPIO to drive output pin */
            MONITORING_TABLET_SET_HSIOM_SEL(MONITORING_TABLET_TX_SDA_MISO_HSIOM_REG, MONITORING_TABLET_TX_SDA_MISO_HSIOM_MASK,
                                           MONITORING_TABLET_TX_SDA_MISO_HSIOM_POS, MONITORING_TABLET_TX_SDA_MISO_HSIOM_SEL_GPIO);
        }
    #endif /* (MONITORING_TABLET_TX_SDA_MISO_PIN_PIN) */

    #if !(MONITORING_TABLET_CY_SCBIP_V0 || MONITORING_TABLET_CY_SCBIP_V1)
        #if (MONITORING_TABLET_RTS_SS0_PIN)
            if (MONITORING_TABLET_CHECK_RTS_SS0_PIN_USED)
            {
                /* Set output pin state after block is disabled */
                MONITORING_TABLET_uart_rts_spi_ss0_Write(MONITORING_TABLET_GET_UART_RTS_INACTIVE);

                /* Set GPIO to drive output pin */
                MONITORING_TABLET_SET_HSIOM_SEL(MONITORING_TABLET_RTS_SS0_HSIOM_REG, MONITORING_TABLET_RTS_SS0_HSIOM_MASK,
                                               MONITORING_TABLET_RTS_SS0_HSIOM_POS, MONITORING_TABLET_RTS_SS0_HSIOM_SEL_GPIO);
            }
        #endif /* (MONITORING_TABLET_RTS_SS0_PIN) */
    #endif /* !(MONITORING_TABLET_CY_SCBIP_V0 || MONITORING_TABLET_CY_SCBIP_V1) */

#else
    #if (MONITORING_TABLET_UART_TX_PIN)
        /* Set GPIO to drive output pin */
        MONITORING_TABLET_SET_HSIOM_SEL(MONITORING_TABLET_TX_HSIOM_REG, MONITORING_TABLET_TX_HSIOM_MASK,
                                       MONITORING_TABLET_TX_HSIOM_POS, MONITORING_TABLET_TX_HSIOM_SEL_GPIO);
    #endif /* (MONITORING_TABLET_UART_TX_PIN) */

    #if (MONITORING_TABLET_UART_RTS_PIN)
        /* Set output pin state after block is disabled */
        MONITORING_TABLET_rts_Write(MONITORING_TABLET_GET_UART_RTS_INACTIVE);

        /* Set GPIO to drive output pin */
        MONITORING_TABLET_SET_HSIOM_SEL(MONITORING_TABLET_RTS_HSIOM_REG, MONITORING_TABLET_RTS_HSIOM_MASK,
                                       MONITORING_TABLET_RTS_HSIOM_POS, MONITORING_TABLET_RTS_HSIOM_SEL_GPIO);
    #endif /* (MONITORING_TABLET_UART_RTS_PIN) */

#endif /* (MONITORING_TABLET_SCB_MODE_UNCONFIG_CONST_CFG) */

#if (MONITORING_TABLET_UART_WAKE_ENABLE_CONST)
    /* Disable skip start feature used for wakeup */
    MONITORING_TABLET_UART_RX_CTRL_REG &= (uint32) ~MONITORING_TABLET_UART_RX_CTRL_SKIP_START;
#endif /* (MONITORING_TABLET_UART_WAKE_ENABLE_CONST) */

    /* Store TX interrupt sources (exclude level triggered). */
    MONITORING_TABLET_IntrTxMask = LO16(MONITORING_TABLET_GetTxInterruptMode() & MONITORING_TABLET_INTR_UART_TX_RESTORE);
}


/*******************************************************************************
* Function Name: MONITORING_TABLET_UartSetRxAddress
****************************************************************************//**
*
*  Sets the hardware detectable receiver address for the UART in the
*  Multiprocessor mode.
*
*  \param address: Address for hardware address detection.
*
*******************************************************************************/
void MONITORING_TABLET_UartSetRxAddress(uint32 address)
{
     uint32 matchReg;

    matchReg = MONITORING_TABLET_RX_MATCH_REG;

    matchReg &= ((uint32) ~MONITORING_TABLET_RX_MATCH_ADDR_MASK); /* Clear address bits */
    matchReg |= ((uint32)  (address & MONITORING_TABLET_RX_MATCH_ADDR_MASK)); /* Set address  */

    MONITORING_TABLET_RX_MATCH_REG = matchReg;
}


/*******************************************************************************
* Function Name: MONITORING_TABLET_UartSetRxAddressMask
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
void MONITORING_TABLET_UartSetRxAddressMask(uint32 addressMask)
{
    uint32 matchReg;

    matchReg = MONITORING_TABLET_RX_MATCH_REG;

    matchReg &= ((uint32) ~MONITORING_TABLET_RX_MATCH_MASK_MASK); /* Clear address mask bits */
    matchReg |= ((uint32) (addressMask << MONITORING_TABLET_RX_MATCH_MASK_POS));

    MONITORING_TABLET_RX_MATCH_REG = matchReg;
}


#if(MONITORING_TABLET_UART_RX_DIRECTION)
    /*******************************************************************************
    * Function Name: MONITORING_TABLET_UartGetChar
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
    *   Check MONITORING_TABLET_rxBufferOverflow to capture that error condition.
    *
    *******************************************************************************/
    uint32 MONITORING_TABLET_UartGetChar(void)
    {
        uint32 rxData = 0u;

        /* Reads data only if there is data to read */
        if (0u != MONITORING_TABLET_SpiUartGetRxBufferSize())
        {
            rxData = MONITORING_TABLET_SpiUartReadRxData();
        }

        if (MONITORING_TABLET_CHECK_INTR_RX(MONITORING_TABLET_INTR_RX_ERR))
        {
            rxData = 0u; /* Error occurred: returns zero */
            MONITORING_TABLET_ClearRxInterruptSource(MONITORING_TABLET_INTR_RX_ERR);
        }

        return (rxData);
    }


    /*******************************************************************************
    * Function Name: MONITORING_TABLET_UartGetByte
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
    *   - MONITORING_TABLET_UART_RX_OVERFLOW - Attempt to write to a full
    *     receiver FIFO.
    *   - MONITORING_TABLET_UART_RX_UNDERFLOW    Attempt to read from an empty
    *     receiver FIFO.
    *   - MONITORING_TABLET_UART_RX_FRAME_ERROR - UART framing error detected.
    *   - MONITORING_TABLET_UART_RX_PARITY_ERROR - UART parity error detected.
    *
    *  \sideeffect
    *   The errors bits may not correspond with reading characters due to
    *   RX FIFO and software buffer usage.
    *   RX software buffer is enabled: The internal software buffer overflow
    *   is not treated as an error condition.
    *   Check MONITORING_TABLET_rxBufferOverflow to capture that error condition.
    *
    *******************************************************************************/
    uint32 MONITORING_TABLET_UartGetByte(void)
    {
        uint32 rxData;
        uint32 tmpStatus;

        #if (MONITORING_TABLET_CHECK_RX_SW_BUFFER)
        {
            MONITORING_TABLET_DisableInt();
        }
        #endif

        if (0u != MONITORING_TABLET_SpiUartGetRxBufferSize())
        {
            /* Enables interrupt to receive more bytes: at least one byte is in
            * buffer.
            */
            #if (MONITORING_TABLET_CHECK_RX_SW_BUFFER)
            {
                MONITORING_TABLET_EnableInt();
            }
            #endif

            /* Get received byte */
            rxData = MONITORING_TABLET_SpiUartReadRxData();
        }
        else
        {
            /* Reads a byte directly from RX FIFO: underflow is raised in the
            * case of empty. Otherwise the first received byte will be read.
            */
            rxData = MONITORING_TABLET_RX_FIFO_RD_REG;


            /* Enables interrupt to receive more bytes. */
            #if (MONITORING_TABLET_CHECK_RX_SW_BUFFER)
            {

                /* The byte has been read from RX FIFO. Clear RX interrupt to
                * not involve interrupt handler when RX FIFO is empty.
                */
                MONITORING_TABLET_ClearRxInterruptSource(MONITORING_TABLET_INTR_RX_NOT_EMPTY);

                MONITORING_TABLET_EnableInt();
            }
            #endif
        }

        /* Get and clear RX error mask */
        tmpStatus = (MONITORING_TABLET_GetRxInterruptSource() & MONITORING_TABLET_INTR_RX_ERR);
        MONITORING_TABLET_ClearRxInterruptSource(MONITORING_TABLET_INTR_RX_ERR);

        /* Puts together data and error status:
        * MP mode and accept address: 9th bit is set to notify mark.
        */
        rxData |= ((uint32) (tmpStatus << 8u));

        return (rxData);
    }


    #if !(MONITORING_TABLET_CY_SCBIP_V0 || MONITORING_TABLET_CY_SCBIP_V1)
        /*******************************************************************************
        * Function Name: MONITORING_TABLET_UartSetRtsPolarity
        ****************************************************************************//**
        *
        *  Sets active polarity of RTS output signal.
        *  Only available for PSoC 4100 BLE / PSoC 4200 BLE / PSoC 4100M / PSoC 4200M /
        *  PSoC 4200L / PSoC 4000S / PSoC 4100S / PSoC Analog Coprocessor devices.
        *
        *  \param polarity: Active polarity of RTS output signal.
        *   - MONITORING_TABLET_UART_RTS_ACTIVE_LOW  - RTS signal is active low.
        *   - MONITORING_TABLET_UART_RTS_ACTIVE_HIGH - RTS signal is active high.
        *
        *******************************************************************************/
        void MONITORING_TABLET_UartSetRtsPolarity(uint32 polarity)
        {
            if(0u != polarity)
            {
                MONITORING_TABLET_UART_FLOW_CTRL_REG |= (uint32)  MONITORING_TABLET_UART_FLOW_CTRL_RTS_POLARITY;
            }
            else
            {
                MONITORING_TABLET_UART_FLOW_CTRL_REG &= (uint32) ~MONITORING_TABLET_UART_FLOW_CTRL_RTS_POLARITY;
            }
        }


        /*******************************************************************************
        * Function Name: MONITORING_TABLET_UartSetRtsFifoLevel
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
        void MONITORING_TABLET_UartSetRtsFifoLevel(uint32 level)
        {
            uint32 uartFlowCtrl;

            uartFlowCtrl = MONITORING_TABLET_UART_FLOW_CTRL_REG;

            uartFlowCtrl &= ((uint32) ~MONITORING_TABLET_UART_FLOW_CTRL_TRIGGER_LEVEL_MASK); /* Clear level mask bits */
            uartFlowCtrl |= ((uint32) (MONITORING_TABLET_UART_FLOW_CTRL_TRIGGER_LEVEL_MASK & level));

            MONITORING_TABLET_UART_FLOW_CTRL_REG = uartFlowCtrl;
        }
    #endif /* !(MONITORING_TABLET_CY_SCBIP_V0 || MONITORING_TABLET_CY_SCBIP_V1) */

#endif /* (MONITORING_TABLET_UART_RX_DIRECTION) */


#if(MONITORING_TABLET_UART_TX_DIRECTION)
    /*******************************************************************************
    * Function Name: MONITORING_TABLET_UartPutString
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
    void MONITORING_TABLET_UartPutString(const char8 string[])
    {
        uint32 bufIndex;

        bufIndex = 0u;

        /* Blocks the control flow until all data has been sent */
        while(string[bufIndex] != ((char8) 0))
        {
            MONITORING_TABLET_UartPutChar((uint32) string[bufIndex]);
            bufIndex++;
        }
    }


    /*******************************************************************************
    * Function Name: MONITORING_TABLET_UartPutCRLF
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
    void MONITORING_TABLET_UartPutCRLF(uint32 txDataByte)
    {
        MONITORING_TABLET_UartPutChar(txDataByte);  /* Blocks control flow until all data has been sent */
        MONITORING_TABLET_UartPutChar(0x0Du);       /* Blocks control flow until all data has been sent */
        MONITORING_TABLET_UartPutChar(0x0Au);       /* Blocks control flow until all data has been sent */
    }


    #if !(MONITORING_TABLET_CY_SCBIP_V0 || MONITORING_TABLET_CY_SCBIP_V1)
        /*******************************************************************************
        * Function Name: MONITORING_TABLETSCB_UartEnableCts
        ****************************************************************************//**
        *
        *  Enables usage of CTS input signal by the UART transmitter.
        *  Only available for PSoC 4100 BLE / PSoC 4200 BLE / PSoC 4100M / PSoC 4200M /
        *  PSoC 4200L / PSoC 4000S / PSoC 4100S / PSoC Analog Coprocessor devices.
        *
        *******************************************************************************/
        void MONITORING_TABLET_UartEnableCts(void)
        {
            MONITORING_TABLET_UART_FLOW_CTRL_REG |= (uint32)  MONITORING_TABLET_UART_FLOW_CTRL_CTS_ENABLE;
        }


        /*******************************************************************************
        * Function Name: MONITORING_TABLET_UartDisableCts
        ****************************************************************************//**
        *
        *  Disables usage of CTS input signal by the UART transmitter.
        *  Only available for PSoC 4100 BLE / PSoC 4200 BLE / PSoC 4100M / PSoC 4200M /
        *  PSoC 4200L / PSoC 4000S / PSoC 4100S / PSoC Analog Coprocessor devices.
        *
        *******************************************************************************/
        void MONITORING_TABLET_UartDisableCts(void)
        {
            MONITORING_TABLET_UART_FLOW_CTRL_REG &= (uint32) ~MONITORING_TABLET_UART_FLOW_CTRL_CTS_ENABLE;
        }


        /*******************************************************************************
        * Function Name: MONITORING_TABLET_UartSetCtsPolarity
        ****************************************************************************//**
        *
        *  Sets active polarity of CTS input signal.
        *  Only available for PSoC 4100 BLE / PSoC 4200 BLE / PSoC 4100M / PSoC 4200M /
        *  PSoC 4200L / PSoC 4000S / PSoC 4100S / PSoC Analog Coprocessor devices.
        *
        * \param
        * polarity: Active polarity of CTS output signal.
        *   - MONITORING_TABLET_UART_CTS_ACTIVE_LOW  - CTS signal is active low.
        *   - MONITORING_TABLET_UART_CTS_ACTIVE_HIGH - CTS signal is active high.
        *
        *******************************************************************************/
        void MONITORING_TABLET_UartSetCtsPolarity(uint32 polarity)
        {
            if (0u != polarity)
            {
                MONITORING_TABLET_UART_FLOW_CTRL_REG |= (uint32)  MONITORING_TABLET_UART_FLOW_CTRL_CTS_POLARITY;
            }
            else
            {
                MONITORING_TABLET_UART_FLOW_CTRL_REG &= (uint32) ~MONITORING_TABLET_UART_FLOW_CTRL_CTS_POLARITY;
            }
        }
    #endif /* !(MONITORING_TABLET_CY_SCBIP_V0 || MONITORING_TABLET_CY_SCBIP_V1) */


    /*******************************************************************************
    * Function Name: MONITORING_TABLET_UartSendBreakBlocking
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
    void MONITORING_TABLET_UartSendBreakBlocking(uint32 breakWidth)
    {
        uint32 txCtrlReg;
        uint32 txIntrReg;

        /* Disable all UART TX interrupt source and clear UART TX Done history */
        txIntrReg = MONITORING_TABLET_GetTxInterruptMode();
        MONITORING_TABLET_SetTxInterruptMode(0u);
        MONITORING_TABLET_ClearTxInterruptSource(MONITORING_TABLET_INTR_TX_UART_DONE);

        /* Store TX CTRL configuration */
        txCtrlReg = MONITORING_TABLET_TX_CTRL_REG;

        /* Set break width */
        MONITORING_TABLET_TX_CTRL_REG = (MONITORING_TABLET_TX_CTRL_REG & (uint32) ~MONITORING_TABLET_TX_CTRL_DATA_WIDTH_MASK) |
                                        MONITORING_TABLET_GET_TX_CTRL_DATA_WIDTH(breakWidth);

        /* Generate break */
        MONITORING_TABLET_TX_FIFO_WR_REG = 0u;

        /* Wait for break completion */
        while (0u == (MONITORING_TABLET_GetTxInterruptSource() & MONITORING_TABLET_INTR_TX_UART_DONE))
        {
        }

        /* Clear all UART TX interrupt sources to  */
        MONITORING_TABLET_ClearTxInterruptSource(MONITORING_TABLET_INTR_TX_ALL);

        /* Restore TX interrupt sources and data width */
        MONITORING_TABLET_TX_CTRL_REG = txCtrlReg;
        MONITORING_TABLET_SetTxInterruptMode(txIntrReg);
    }
#endif /* (MONITORING_TABLET_UART_TX_DIRECTION) */


#if (MONITORING_TABLET_UART_WAKE_ENABLE_CONST)
    /*******************************************************************************
    * Function Name: MONITORING_TABLET_UartSaveConfig
    ****************************************************************************//**
    *
    *  Clears and enables an interrupt on a falling edge of the Rx input. The GPIO
    *  interrupt does not track in the active mode, therefore requires to be
    *  cleared by this API.
    *
    *******************************************************************************/
    void MONITORING_TABLET_UartSaveConfig(void)
    {
    #if (MONITORING_TABLET_UART_RX_WAKEUP_IRQ)
        /* Set SKIP_START if requested (set by default). */
        if (0u != MONITORING_TABLET_skipStart)
        {
            MONITORING_TABLET_UART_RX_CTRL_REG |= (uint32)  MONITORING_TABLET_UART_RX_CTRL_SKIP_START;
        }
        else
        {
            MONITORING_TABLET_UART_RX_CTRL_REG &= (uint32) ~MONITORING_TABLET_UART_RX_CTRL_SKIP_START;
        }

        /* Clear RX GPIO interrupt status and pending interrupt in NVIC because
        * falling edge on RX line occurs while UART communication in active mode.
        * Enable interrupt: next interrupt trigger should wakeup device.
        */
        MONITORING_TABLET_CLEAR_UART_RX_WAKE_INTR;
        MONITORING_TABLET_RxWakeClearPendingInt();
        MONITORING_TABLET_RxWakeEnableInt();
    #endif /* (MONITORING_TABLET_UART_RX_WAKEUP_IRQ) */
    }


    /*******************************************************************************
    * Function Name: MONITORING_TABLET_UartRestoreConfig
    ****************************************************************************//**
    *
    *  Disables the RX GPIO interrupt. Until this function is called the interrupt
    *  remains active and triggers on every falling edge of the UART RX line.
    *
    *******************************************************************************/
    void MONITORING_TABLET_UartRestoreConfig(void)
    {
    #if (MONITORING_TABLET_UART_RX_WAKEUP_IRQ)
        /* Disable interrupt: no more triggers in active mode */
        MONITORING_TABLET_RxWakeDisableInt();
    #endif /* (MONITORING_TABLET_UART_RX_WAKEUP_IRQ) */
    }


    #if (MONITORING_TABLET_UART_RX_WAKEUP_IRQ)
        /*******************************************************************************
        * Function Name: MONITORING_TABLET_UART_WAKEUP_ISR
        ****************************************************************************//**
        *
        *  Handles the Interrupt Service Routine for the SCB UART mode GPIO wakeup
        *  event. This event is configured to trigger on a falling edge of the RX line.
        *
        *******************************************************************************/
        CY_ISR(MONITORING_TABLET_UART_WAKEUP_ISR)
        {
        #ifdef MONITORING_TABLET_UART_WAKEUP_ISR_ENTRY_CALLBACK
            MONITORING_TABLET_UART_WAKEUP_ISR_EntryCallback();
        #endif /* MONITORING_TABLET_UART_WAKEUP_ISR_ENTRY_CALLBACK */

            MONITORING_TABLET_CLEAR_UART_RX_WAKE_INTR;

        #ifdef MONITORING_TABLET_UART_WAKEUP_ISR_EXIT_CALLBACK
            MONITORING_TABLET_UART_WAKEUP_ISR_ExitCallback();
        #endif /* MONITORING_TABLET_UART_WAKEUP_ISR_EXIT_CALLBACK */
        }
    #endif /* (MONITORING_TABLET_UART_RX_WAKEUP_IRQ) */
#endif /* (MONITORING_TABLET_UART_RX_WAKEUP_IRQ) */


/* [] END OF FILE */
