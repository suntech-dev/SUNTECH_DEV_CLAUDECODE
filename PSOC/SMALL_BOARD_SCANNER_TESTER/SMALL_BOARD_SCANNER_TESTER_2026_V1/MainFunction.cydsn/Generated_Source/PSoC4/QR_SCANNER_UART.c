/***************************************************************************//**
* \file QR_SCANNER_UART.c
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

#include "QR_SCANNER_PVT.h"
#include "QR_SCANNER_SPI_UART_PVT.h"
#include "cyapicallbacks.h"

#if (QR_SCANNER_UART_WAKE_ENABLE_CONST && QR_SCANNER_UART_RX_WAKEUP_IRQ)
    /**
    * \addtogroup group_globals
    * \{
    */
    /** This global variable determines whether to enable Skip Start
    * functionality when QR_SCANNER_Sleep() function is called:
    * 0 – disable, other values – enable. Default value is 1.
    * It is only available when Enable wakeup from Deep Sleep Mode is enabled.
    */
    uint8 QR_SCANNER_skipStart = 1u;
    /** \} globals */
#endif /* (QR_SCANNER_UART_WAKE_ENABLE_CONST && QR_SCANNER_UART_RX_WAKEUP_IRQ) */

#if(QR_SCANNER_SCB_MODE_UNCONFIG_CONST_CFG)

    /***************************************
    *  Configuration Structure Initialization
    ***************************************/

    const QR_SCANNER_UART_INIT_STRUCT QR_SCANNER_configUart =
    {
        QR_SCANNER_UART_SUB_MODE,
        QR_SCANNER_UART_DIRECTION,
        QR_SCANNER_UART_DATA_BITS_NUM,
        QR_SCANNER_UART_PARITY_TYPE,
        QR_SCANNER_UART_STOP_BITS_NUM,
        QR_SCANNER_UART_OVS_FACTOR,
        QR_SCANNER_UART_IRDA_LOW_POWER,
        QR_SCANNER_UART_MEDIAN_FILTER_ENABLE,
        QR_SCANNER_UART_RETRY_ON_NACK,
        QR_SCANNER_UART_IRDA_POLARITY,
        QR_SCANNER_UART_DROP_ON_PARITY_ERR,
        QR_SCANNER_UART_DROP_ON_FRAME_ERR,
        QR_SCANNER_UART_WAKE_ENABLE,
        0u,
        NULL,
        0u,
        NULL,
        QR_SCANNER_UART_MP_MODE_ENABLE,
        QR_SCANNER_UART_MP_ACCEPT_ADDRESS,
        QR_SCANNER_UART_MP_RX_ADDRESS,
        QR_SCANNER_UART_MP_RX_ADDRESS_MASK,
        (uint32) QR_SCANNER_SCB_IRQ_INTERNAL,
        QR_SCANNER_UART_INTR_RX_MASK,
        QR_SCANNER_UART_RX_TRIGGER_LEVEL,
        QR_SCANNER_UART_INTR_TX_MASK,
        QR_SCANNER_UART_TX_TRIGGER_LEVEL,
        (uint8) QR_SCANNER_UART_BYTE_MODE_ENABLE,
        (uint8) QR_SCANNER_UART_CTS_ENABLE,
        (uint8) QR_SCANNER_UART_CTS_POLARITY,
        (uint8) QR_SCANNER_UART_RTS_POLARITY,
        (uint8) QR_SCANNER_UART_RTS_FIFO_LEVEL,
        (uint8) QR_SCANNER_UART_RX_BREAK_WIDTH
    };


    /*******************************************************************************
    * Function Name: QR_SCANNER_UartInit
    ****************************************************************************//**
    *
    *  Configures the QR_SCANNER for UART operation.
    *
    *  This function is intended specifically to be used when the QR_SCANNER
    *  configuration is set to “Unconfigured QR_SCANNER” in the customizer.
    *  After initializing the QR_SCANNER in UART mode using this function,
    *  the component can be enabled using the QR_SCANNER_Start() or
    * QR_SCANNER_Enable() function.
    *  This function uses a pointer to a structure that provides the configuration
    *  settings. This structure contains the same information that would otherwise
    *  be provided by the customizer settings.
    *
    *  \param config: pointer to a structure that contains the following list of
    *   fields. These fields match the selections available in the customizer.
    *   Refer to the customizer for further description of the settings.
    *
    *******************************************************************************/
    void QR_SCANNER_UartInit(const QR_SCANNER_UART_INIT_STRUCT *config)
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

        #if !(QR_SCANNER_CY_SCBIP_V0 || QR_SCANNER_CY_SCBIP_V1)
            /* Add RTS and CTS pins to configure */
            pinsConfig |= (0u != config->rtsRxFifoLevel) ? (QR_SCANNER_UART_RTS_PIN_ENABLE) : (0u);
            pinsConfig |= (0u != config->enableCts)      ? (QR_SCANNER_UART_CTS_PIN_ENABLE) : (0u);
        #endif /* !(QR_SCANNER_CY_SCBIP_V0 || QR_SCANNER_CY_SCBIP_V1) */

            /* Configure pins */
            QR_SCANNER_SetPins(QR_SCANNER_SCB_MODE_UART, config->mode, pinsConfig);

            /* Store internal configuration */
            QR_SCANNER_scbMode       = (uint8) QR_SCANNER_SCB_MODE_UART;
            QR_SCANNER_scbEnableWake = (uint8) config->enableWake;
            QR_SCANNER_scbEnableIntr = (uint8) config->enableInterrupt;

            /* Set RX direction internal variables */
            QR_SCANNER_rxBuffer      =         config->rxBuffer;
            QR_SCANNER_rxDataBits    = (uint8) config->dataBits;
            QR_SCANNER_rxBufferSize  =         config->rxBufferSize;

            /* Set TX direction internal variables */
            QR_SCANNER_txBuffer      =         config->txBuffer;
            QR_SCANNER_txDataBits    = (uint8) config->dataBits;
            QR_SCANNER_txBufferSize  =         config->txBufferSize;

            /* Configure UART interface */
            if(QR_SCANNER_UART_MODE_IRDA == config->mode)
            {
                /* OVS settings: IrDA */
                QR_SCANNER_CTRL_REG  = ((0u != config->enableIrdaLowPower) ?
                                                (QR_SCANNER_UART_GET_CTRL_OVS_IRDA_LP(config->oversample)) :
                                                (QR_SCANNER_CTRL_OVS_IRDA_OVS16));
            }
            else
            {
                /* OVS settings: UART and SmartCard */
                QR_SCANNER_CTRL_REG  = QR_SCANNER_GET_CTRL_OVS(config->oversample);
            }

            QR_SCANNER_CTRL_REG     |= QR_SCANNER_GET_CTRL_BYTE_MODE  (config->enableByteMode)      |
                                             QR_SCANNER_GET_CTRL_ADDR_ACCEPT(config->multiprocAcceptAddr) |
                                             QR_SCANNER_CTRL_UART;

            /* Configure sub-mode: UART, SmartCard or IrDA */
            QR_SCANNER_UART_CTRL_REG = QR_SCANNER_GET_UART_CTRL_MODE(config->mode);

            /* Configure RX direction */
            QR_SCANNER_UART_RX_CTRL_REG = QR_SCANNER_GET_UART_RX_CTRL_MODE(config->stopBits)              |
                                        QR_SCANNER_GET_UART_RX_CTRL_POLARITY(config->enableInvertedRx)          |
                                        QR_SCANNER_GET_UART_RX_CTRL_MP_MODE(config->enableMultiproc)            |
                                        QR_SCANNER_GET_UART_RX_CTRL_DROP_ON_PARITY_ERR(config->dropOnParityErr) |
                                        QR_SCANNER_GET_UART_RX_CTRL_DROP_ON_FRAME_ERR(config->dropOnFrameErr)   |
                                        QR_SCANNER_GET_UART_RX_CTRL_BREAK_WIDTH(config->breakWidth);

            if(QR_SCANNER_UART_PARITY_NONE != config->parity)
            {
               QR_SCANNER_UART_RX_CTRL_REG |= QR_SCANNER_GET_UART_RX_CTRL_PARITY(config->parity) |
                                                    QR_SCANNER_UART_RX_CTRL_PARITY_ENABLED;
            }

            QR_SCANNER_RX_CTRL_REG      = QR_SCANNER_GET_RX_CTRL_DATA_WIDTH(config->dataBits)       |
                                                QR_SCANNER_GET_RX_CTRL_MEDIAN(config->enableMedianFilter) |
                                                QR_SCANNER_GET_UART_RX_CTRL_ENABLED(config->direction);

            QR_SCANNER_RX_FIFO_CTRL_REG = QR_SCANNER_GET_RX_FIFO_CTRL_TRIGGER_LEVEL(config->rxTriggerLevel);

            /* Configure MP address */
            QR_SCANNER_RX_MATCH_REG     = QR_SCANNER_GET_RX_MATCH_ADDR(config->multiprocAddr) |
                                                QR_SCANNER_GET_RX_MATCH_MASK(config->multiprocAddrMask);

            /* Configure RX direction */
            QR_SCANNER_UART_TX_CTRL_REG = QR_SCANNER_GET_UART_TX_CTRL_MODE(config->stopBits) |
                                                QR_SCANNER_GET_UART_TX_CTRL_RETRY_NACK(config->enableRetryNack);

            if(QR_SCANNER_UART_PARITY_NONE != config->parity)
            {
               QR_SCANNER_UART_TX_CTRL_REG |= QR_SCANNER_GET_UART_TX_CTRL_PARITY(config->parity) |
                                                    QR_SCANNER_UART_TX_CTRL_PARITY_ENABLED;
            }

            QR_SCANNER_TX_CTRL_REG      = QR_SCANNER_GET_TX_CTRL_DATA_WIDTH(config->dataBits)    |
                                                QR_SCANNER_GET_UART_TX_CTRL_ENABLED(config->direction);

            QR_SCANNER_TX_FIFO_CTRL_REG = QR_SCANNER_GET_TX_FIFO_CTRL_TRIGGER_LEVEL(config->txTriggerLevel);

        #if !(QR_SCANNER_CY_SCBIP_V0 || QR_SCANNER_CY_SCBIP_V1)
            QR_SCANNER_UART_FLOW_CTRL_REG = QR_SCANNER_GET_UART_FLOW_CTRL_CTS_ENABLE(config->enableCts) | \
                                            QR_SCANNER_GET_UART_FLOW_CTRL_CTS_POLARITY (config->ctsPolarity)  | \
                                            QR_SCANNER_GET_UART_FLOW_CTRL_RTS_POLARITY (config->rtsPolarity)  | \
                                            QR_SCANNER_GET_UART_FLOW_CTRL_TRIGGER_LEVEL(config->rtsRxFifoLevel);
        #endif /* !(QR_SCANNER_CY_SCBIP_V0 || QR_SCANNER_CY_SCBIP_V1) */

            /* Configure interrupt with UART handler but do not enable it */
            CyIntDisable    (QR_SCANNER_ISR_NUMBER);
            CyIntSetPriority(QR_SCANNER_ISR_NUMBER, QR_SCANNER_ISR_PRIORITY);
            (void) CyIntSetVector(QR_SCANNER_ISR_NUMBER, &QR_SCANNER_SPI_UART_ISR);

            /* Configure WAKE interrupt */
        #if(QR_SCANNER_UART_RX_WAKEUP_IRQ)
            CyIntDisable    (QR_SCANNER_RX_WAKE_ISR_NUMBER);
            CyIntSetPriority(QR_SCANNER_RX_WAKE_ISR_NUMBER, QR_SCANNER_RX_WAKE_ISR_PRIORITY);
            (void) CyIntSetVector(QR_SCANNER_RX_WAKE_ISR_NUMBER, &QR_SCANNER_UART_WAKEUP_ISR);
        #endif /* (QR_SCANNER_UART_RX_WAKEUP_IRQ) */

            /* Configure interrupt sources */
            QR_SCANNER_INTR_I2C_EC_MASK_REG = QR_SCANNER_NO_INTR_SOURCES;
            QR_SCANNER_INTR_SPI_EC_MASK_REG = QR_SCANNER_NO_INTR_SOURCES;
            QR_SCANNER_INTR_SLAVE_MASK_REG  = QR_SCANNER_NO_INTR_SOURCES;
            QR_SCANNER_INTR_MASTER_MASK_REG = QR_SCANNER_NO_INTR_SOURCES;
            QR_SCANNER_INTR_RX_MASK_REG     = config->rxInterruptMask;
            QR_SCANNER_INTR_TX_MASK_REG     = config->txInterruptMask;

            /* Configure TX interrupt sources to restore. */
            QR_SCANNER_IntrTxMask = LO16(QR_SCANNER_INTR_TX_MASK_REG);

            /* Clear RX buffer indexes */
            QR_SCANNER_rxBufferHead     = 0u;
            QR_SCANNER_rxBufferTail     = 0u;
            QR_SCANNER_rxBufferOverflow = 0u;

            /* Clear TX buffer indexes */
            QR_SCANNER_txBufferHead = 0u;
            QR_SCANNER_txBufferTail = 0u;
        }
    }

#else

    /*******************************************************************************
    * Function Name: QR_SCANNER_UartInit
    ****************************************************************************//**
    *
    *  Configures the SCB for the UART operation.
    *
    *******************************************************************************/
    void QR_SCANNER_UartInit(void)
    {
        /* Configure UART interface */
        QR_SCANNER_CTRL_REG = QR_SCANNER_UART_DEFAULT_CTRL;

        /* Configure sub-mode: UART, SmartCard or IrDA */
        QR_SCANNER_UART_CTRL_REG = QR_SCANNER_UART_DEFAULT_UART_CTRL;

        /* Configure RX direction */
        QR_SCANNER_UART_RX_CTRL_REG = QR_SCANNER_UART_DEFAULT_UART_RX_CTRL;
        QR_SCANNER_RX_CTRL_REG      = QR_SCANNER_UART_DEFAULT_RX_CTRL;
        QR_SCANNER_RX_FIFO_CTRL_REG = QR_SCANNER_UART_DEFAULT_RX_FIFO_CTRL;
        QR_SCANNER_RX_MATCH_REG     = QR_SCANNER_UART_DEFAULT_RX_MATCH_REG;

        /* Configure TX direction */
        QR_SCANNER_UART_TX_CTRL_REG = QR_SCANNER_UART_DEFAULT_UART_TX_CTRL;
        QR_SCANNER_TX_CTRL_REG      = QR_SCANNER_UART_DEFAULT_TX_CTRL;
        QR_SCANNER_TX_FIFO_CTRL_REG = QR_SCANNER_UART_DEFAULT_TX_FIFO_CTRL;

    #if !(QR_SCANNER_CY_SCBIP_V0 || QR_SCANNER_CY_SCBIP_V1)
        QR_SCANNER_UART_FLOW_CTRL_REG = QR_SCANNER_UART_DEFAULT_FLOW_CTRL;
    #endif /* !(QR_SCANNER_CY_SCBIP_V0 || QR_SCANNER_CY_SCBIP_V1) */

        /* Configure interrupt with UART handler but do not enable it */
    #if(QR_SCANNER_SCB_IRQ_INTERNAL)
        CyIntDisable    (QR_SCANNER_ISR_NUMBER);
        CyIntSetPriority(QR_SCANNER_ISR_NUMBER, QR_SCANNER_ISR_PRIORITY);
        (void) CyIntSetVector(QR_SCANNER_ISR_NUMBER, &QR_SCANNER_SPI_UART_ISR);
    #endif /* (QR_SCANNER_SCB_IRQ_INTERNAL) */

        /* Configure WAKE interrupt */
    #if(QR_SCANNER_UART_RX_WAKEUP_IRQ)
        CyIntDisable    (QR_SCANNER_RX_WAKE_ISR_NUMBER);
        CyIntSetPriority(QR_SCANNER_RX_WAKE_ISR_NUMBER, QR_SCANNER_RX_WAKE_ISR_PRIORITY);
        (void) CyIntSetVector(QR_SCANNER_RX_WAKE_ISR_NUMBER, &QR_SCANNER_UART_WAKEUP_ISR);
    #endif /* (QR_SCANNER_UART_RX_WAKEUP_IRQ) */

        /* Configure interrupt sources */
        QR_SCANNER_INTR_I2C_EC_MASK_REG = QR_SCANNER_UART_DEFAULT_INTR_I2C_EC_MASK;
        QR_SCANNER_INTR_SPI_EC_MASK_REG = QR_SCANNER_UART_DEFAULT_INTR_SPI_EC_MASK;
        QR_SCANNER_INTR_SLAVE_MASK_REG  = QR_SCANNER_UART_DEFAULT_INTR_SLAVE_MASK;
        QR_SCANNER_INTR_MASTER_MASK_REG = QR_SCANNER_UART_DEFAULT_INTR_MASTER_MASK;
        QR_SCANNER_INTR_RX_MASK_REG     = QR_SCANNER_UART_DEFAULT_INTR_RX_MASK;
        QR_SCANNER_INTR_TX_MASK_REG     = QR_SCANNER_UART_DEFAULT_INTR_TX_MASK;

        /* Configure TX interrupt sources to restore. */
        QR_SCANNER_IntrTxMask = LO16(QR_SCANNER_INTR_TX_MASK_REG);

    #if(QR_SCANNER_INTERNAL_RX_SW_BUFFER_CONST)
        QR_SCANNER_rxBufferHead     = 0u;
        QR_SCANNER_rxBufferTail     = 0u;
        QR_SCANNER_rxBufferOverflow = 0u;
    #endif /* (QR_SCANNER_INTERNAL_RX_SW_BUFFER_CONST) */

    #if(QR_SCANNER_INTERNAL_TX_SW_BUFFER_CONST)
        QR_SCANNER_txBufferHead = 0u;
        QR_SCANNER_txBufferTail = 0u;
    #endif /* (QR_SCANNER_INTERNAL_TX_SW_BUFFER_CONST) */
    }
#endif /* (QR_SCANNER_SCB_MODE_UNCONFIG_CONST_CFG) */


/*******************************************************************************
* Function Name: QR_SCANNER_UartPostEnable
****************************************************************************//**
*
*  Restores HSIOM settings for the UART output pins (TX and/or RTS) to be
*  controlled by the SCB UART.
*
*******************************************************************************/
void QR_SCANNER_UartPostEnable(void)
{
#if (QR_SCANNER_SCB_MODE_UNCONFIG_CONST_CFG)
    #if (QR_SCANNER_TX_SDA_MISO_PIN)
        if (QR_SCANNER_CHECK_TX_SDA_MISO_PIN_USED)
        {
            /* Set SCB UART to drive the output pin */
            QR_SCANNER_SET_HSIOM_SEL(QR_SCANNER_TX_SDA_MISO_HSIOM_REG, QR_SCANNER_TX_SDA_MISO_HSIOM_MASK,
                                           QR_SCANNER_TX_SDA_MISO_HSIOM_POS, QR_SCANNER_TX_SDA_MISO_HSIOM_SEL_UART);
        }
    #endif /* (QR_SCANNER_TX_SDA_MISO_PIN_PIN) */

    #if !(QR_SCANNER_CY_SCBIP_V0 || QR_SCANNER_CY_SCBIP_V1)
        #if (QR_SCANNER_RTS_SS0_PIN)
            if (QR_SCANNER_CHECK_RTS_SS0_PIN_USED)
            {
                /* Set SCB UART to drive the output pin */
                QR_SCANNER_SET_HSIOM_SEL(QR_SCANNER_RTS_SS0_HSIOM_REG, QR_SCANNER_RTS_SS0_HSIOM_MASK,
                                               QR_SCANNER_RTS_SS0_HSIOM_POS, QR_SCANNER_RTS_SS0_HSIOM_SEL_UART);
            }
        #endif /* (QR_SCANNER_RTS_SS0_PIN) */
    #endif /* !(QR_SCANNER_CY_SCBIP_V0 || QR_SCANNER_CY_SCBIP_V1) */

#else
    #if (QR_SCANNER_UART_TX_PIN)
         /* Set SCB UART to drive the output pin */
        QR_SCANNER_SET_HSIOM_SEL(QR_SCANNER_TX_HSIOM_REG, QR_SCANNER_TX_HSIOM_MASK,
                                       QR_SCANNER_TX_HSIOM_POS, QR_SCANNER_TX_HSIOM_SEL_UART);
    #endif /* (QR_SCANNER_UART_TX_PIN) */

    #if (QR_SCANNER_UART_RTS_PIN)
        /* Set SCB UART to drive the output pin */
        QR_SCANNER_SET_HSIOM_SEL(QR_SCANNER_RTS_HSIOM_REG, QR_SCANNER_RTS_HSIOM_MASK,
                                       QR_SCANNER_RTS_HSIOM_POS, QR_SCANNER_RTS_HSIOM_SEL_UART);
    #endif /* (QR_SCANNER_UART_RTS_PIN) */
#endif /* (QR_SCANNER_SCB_MODE_UNCONFIG_CONST_CFG) */

    /* Restore TX interrupt sources. */
    QR_SCANNER_SetTxInterruptMode(QR_SCANNER_IntrTxMask);
}


/*******************************************************************************
* Function Name: QR_SCANNER_UartStop
****************************************************************************//**
*
*  Changes the HSIOM settings for the UART output pins (TX and/or RTS) to keep
*  them inactive after the block is disabled. The output pins are controlled by
*  the GPIO data register. Also, the function disables the skip start feature
*  to not cause it to trigger after the component is enabled.
*
*******************************************************************************/
void QR_SCANNER_UartStop(void)
{
#if(QR_SCANNER_SCB_MODE_UNCONFIG_CONST_CFG)
    #if (QR_SCANNER_TX_SDA_MISO_PIN)
        if (QR_SCANNER_CHECK_TX_SDA_MISO_PIN_USED)
        {
            /* Set GPIO to drive output pin */
            QR_SCANNER_SET_HSIOM_SEL(QR_SCANNER_TX_SDA_MISO_HSIOM_REG, QR_SCANNER_TX_SDA_MISO_HSIOM_MASK,
                                           QR_SCANNER_TX_SDA_MISO_HSIOM_POS, QR_SCANNER_TX_SDA_MISO_HSIOM_SEL_GPIO);
        }
    #endif /* (QR_SCANNER_TX_SDA_MISO_PIN_PIN) */

    #if !(QR_SCANNER_CY_SCBIP_V0 || QR_SCANNER_CY_SCBIP_V1)
        #if (QR_SCANNER_RTS_SS0_PIN)
            if (QR_SCANNER_CHECK_RTS_SS0_PIN_USED)
            {
                /* Set output pin state after block is disabled */
                QR_SCANNER_uart_rts_spi_ss0_Write(QR_SCANNER_GET_UART_RTS_INACTIVE);

                /* Set GPIO to drive output pin */
                QR_SCANNER_SET_HSIOM_SEL(QR_SCANNER_RTS_SS0_HSIOM_REG, QR_SCANNER_RTS_SS0_HSIOM_MASK,
                                               QR_SCANNER_RTS_SS0_HSIOM_POS, QR_SCANNER_RTS_SS0_HSIOM_SEL_GPIO);
            }
        #endif /* (QR_SCANNER_RTS_SS0_PIN) */
    #endif /* !(QR_SCANNER_CY_SCBIP_V0 || QR_SCANNER_CY_SCBIP_V1) */

#else
    #if (QR_SCANNER_UART_TX_PIN)
        /* Set GPIO to drive output pin */
        QR_SCANNER_SET_HSIOM_SEL(QR_SCANNER_TX_HSIOM_REG, QR_SCANNER_TX_HSIOM_MASK,
                                       QR_SCANNER_TX_HSIOM_POS, QR_SCANNER_TX_HSIOM_SEL_GPIO);
    #endif /* (QR_SCANNER_UART_TX_PIN) */

    #if (QR_SCANNER_UART_RTS_PIN)
        /* Set output pin state after block is disabled */
        QR_SCANNER_rts_Write(QR_SCANNER_GET_UART_RTS_INACTIVE);

        /* Set GPIO to drive output pin */
        QR_SCANNER_SET_HSIOM_SEL(QR_SCANNER_RTS_HSIOM_REG, QR_SCANNER_RTS_HSIOM_MASK,
                                       QR_SCANNER_RTS_HSIOM_POS, QR_SCANNER_RTS_HSIOM_SEL_GPIO);
    #endif /* (QR_SCANNER_UART_RTS_PIN) */

#endif /* (QR_SCANNER_SCB_MODE_UNCONFIG_CONST_CFG) */

#if (QR_SCANNER_UART_WAKE_ENABLE_CONST)
    /* Disable skip start feature used for wakeup */
    QR_SCANNER_UART_RX_CTRL_REG &= (uint32) ~QR_SCANNER_UART_RX_CTRL_SKIP_START;
#endif /* (QR_SCANNER_UART_WAKE_ENABLE_CONST) */

    /* Store TX interrupt sources (exclude level triggered). */
    QR_SCANNER_IntrTxMask = LO16(QR_SCANNER_GetTxInterruptMode() & QR_SCANNER_INTR_UART_TX_RESTORE);
}


/*******************************************************************************
* Function Name: QR_SCANNER_UartSetRxAddress
****************************************************************************//**
*
*  Sets the hardware detectable receiver address for the UART in the
*  Multiprocessor mode.
*
*  \param address: Address for hardware address detection.
*
*******************************************************************************/
void QR_SCANNER_UartSetRxAddress(uint32 address)
{
     uint32 matchReg;

    matchReg = QR_SCANNER_RX_MATCH_REG;

    matchReg &= ((uint32) ~QR_SCANNER_RX_MATCH_ADDR_MASK); /* Clear address bits */
    matchReg |= ((uint32)  (address & QR_SCANNER_RX_MATCH_ADDR_MASK)); /* Set address  */

    QR_SCANNER_RX_MATCH_REG = matchReg;
}


/*******************************************************************************
* Function Name: QR_SCANNER_UartSetRxAddressMask
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
void QR_SCANNER_UartSetRxAddressMask(uint32 addressMask)
{
    uint32 matchReg;

    matchReg = QR_SCANNER_RX_MATCH_REG;

    matchReg &= ((uint32) ~QR_SCANNER_RX_MATCH_MASK_MASK); /* Clear address mask bits */
    matchReg |= ((uint32) (addressMask << QR_SCANNER_RX_MATCH_MASK_POS));

    QR_SCANNER_RX_MATCH_REG = matchReg;
}


#if(QR_SCANNER_UART_RX_DIRECTION)
    /*******************************************************************************
    * Function Name: QR_SCANNER_UartGetChar
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
    *   Check QR_SCANNER_rxBufferOverflow to capture that error condition.
    *
    *******************************************************************************/
    uint32 QR_SCANNER_UartGetChar(void)
    {
        uint32 rxData = 0u;

        /* Reads data only if there is data to read */
        if (0u != QR_SCANNER_SpiUartGetRxBufferSize())
        {
            rxData = QR_SCANNER_SpiUartReadRxData();
        }

        if (QR_SCANNER_CHECK_INTR_RX(QR_SCANNER_INTR_RX_ERR))
        {
            rxData = 0u; /* Error occurred: returns zero */
            QR_SCANNER_ClearRxInterruptSource(QR_SCANNER_INTR_RX_ERR);
        }

        return (rxData);
    }


    /*******************************************************************************
    * Function Name: QR_SCANNER_UartGetByte
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
    *   - QR_SCANNER_UART_RX_OVERFLOW - Attempt to write to a full
    *     receiver FIFO.
    *   - QR_SCANNER_UART_RX_UNDERFLOW    Attempt to read from an empty
    *     receiver FIFO.
    *   - QR_SCANNER_UART_RX_FRAME_ERROR - UART framing error detected.
    *   - QR_SCANNER_UART_RX_PARITY_ERROR - UART parity error detected.
    *
    *  \sideeffect
    *   The errors bits may not correspond with reading characters due to
    *   RX FIFO and software buffer usage.
    *   RX software buffer is enabled: The internal software buffer overflow
    *   is not treated as an error condition.
    *   Check QR_SCANNER_rxBufferOverflow to capture that error condition.
    *
    *******************************************************************************/
    uint32 QR_SCANNER_UartGetByte(void)
    {
        uint32 rxData;
        uint32 tmpStatus;

        #if (QR_SCANNER_CHECK_RX_SW_BUFFER)
        {
            QR_SCANNER_DisableInt();
        }
        #endif

        if (0u != QR_SCANNER_SpiUartGetRxBufferSize())
        {
            /* Enables interrupt to receive more bytes: at least one byte is in
            * buffer.
            */
            #if (QR_SCANNER_CHECK_RX_SW_BUFFER)
            {
                QR_SCANNER_EnableInt();
            }
            #endif

            /* Get received byte */
            rxData = QR_SCANNER_SpiUartReadRxData();
        }
        else
        {
            /* Reads a byte directly from RX FIFO: underflow is raised in the
            * case of empty. Otherwise the first received byte will be read.
            */
            rxData = QR_SCANNER_RX_FIFO_RD_REG;


            /* Enables interrupt to receive more bytes. */
            #if (QR_SCANNER_CHECK_RX_SW_BUFFER)
            {

                /* The byte has been read from RX FIFO. Clear RX interrupt to
                * not involve interrupt handler when RX FIFO is empty.
                */
                QR_SCANNER_ClearRxInterruptSource(QR_SCANNER_INTR_RX_NOT_EMPTY);

                QR_SCANNER_EnableInt();
            }
            #endif
        }

        /* Get and clear RX error mask */
        tmpStatus = (QR_SCANNER_GetRxInterruptSource() & QR_SCANNER_INTR_RX_ERR);
        QR_SCANNER_ClearRxInterruptSource(QR_SCANNER_INTR_RX_ERR);

        /* Puts together data and error status:
        * MP mode and accept address: 9th bit is set to notify mark.
        */
        rxData |= ((uint32) (tmpStatus << 8u));

        return (rxData);
    }


    #if !(QR_SCANNER_CY_SCBIP_V0 || QR_SCANNER_CY_SCBIP_V1)
        /*******************************************************************************
        * Function Name: QR_SCANNER_UartSetRtsPolarity
        ****************************************************************************//**
        *
        *  Sets active polarity of RTS output signal.
        *  Only available for PSoC 4100 BLE / PSoC 4200 BLE / PSoC 4100M / PSoC 4200M /
        *  PSoC 4200L / PSoC 4000S / PSoC 4100S / PSoC Analog Coprocessor devices.
        *
        *  \param polarity: Active polarity of RTS output signal.
        *   - QR_SCANNER_UART_RTS_ACTIVE_LOW  - RTS signal is active low.
        *   - QR_SCANNER_UART_RTS_ACTIVE_HIGH - RTS signal is active high.
        *
        *******************************************************************************/
        void QR_SCANNER_UartSetRtsPolarity(uint32 polarity)
        {
            if(0u != polarity)
            {
                QR_SCANNER_UART_FLOW_CTRL_REG |= (uint32)  QR_SCANNER_UART_FLOW_CTRL_RTS_POLARITY;
            }
            else
            {
                QR_SCANNER_UART_FLOW_CTRL_REG &= (uint32) ~QR_SCANNER_UART_FLOW_CTRL_RTS_POLARITY;
            }
        }


        /*******************************************************************************
        * Function Name: QR_SCANNER_UartSetRtsFifoLevel
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
        void QR_SCANNER_UartSetRtsFifoLevel(uint32 level)
        {
            uint32 uartFlowCtrl;

            uartFlowCtrl = QR_SCANNER_UART_FLOW_CTRL_REG;

            uartFlowCtrl &= ((uint32) ~QR_SCANNER_UART_FLOW_CTRL_TRIGGER_LEVEL_MASK); /* Clear level mask bits */
            uartFlowCtrl |= ((uint32) (QR_SCANNER_UART_FLOW_CTRL_TRIGGER_LEVEL_MASK & level));

            QR_SCANNER_UART_FLOW_CTRL_REG = uartFlowCtrl;
        }
    #endif /* !(QR_SCANNER_CY_SCBIP_V0 || QR_SCANNER_CY_SCBIP_V1) */

#endif /* (QR_SCANNER_UART_RX_DIRECTION) */


#if(QR_SCANNER_UART_TX_DIRECTION)
    /*******************************************************************************
    * Function Name: QR_SCANNER_UartPutString
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
    void QR_SCANNER_UartPutString(const char8 string[])
    {
        uint32 bufIndex;

        bufIndex = 0u;

        /* Blocks the control flow until all data has been sent */
        while(string[bufIndex] != ((char8) 0))
        {
            QR_SCANNER_UartPutChar((uint32) string[bufIndex]);
            bufIndex++;
        }
    }


    /*******************************************************************************
    * Function Name: QR_SCANNER_UartPutCRLF
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
    void QR_SCANNER_UartPutCRLF(uint32 txDataByte)
    {
        QR_SCANNER_UartPutChar(txDataByte);  /* Blocks control flow until all data has been sent */
        QR_SCANNER_UartPutChar(0x0Du);       /* Blocks control flow until all data has been sent */
        QR_SCANNER_UartPutChar(0x0Au);       /* Blocks control flow until all data has been sent */
    }


    #if !(QR_SCANNER_CY_SCBIP_V0 || QR_SCANNER_CY_SCBIP_V1)
        /*******************************************************************************
        * Function Name: QR_SCANNERSCB_UartEnableCts
        ****************************************************************************//**
        *
        *  Enables usage of CTS input signal by the UART transmitter.
        *  Only available for PSoC 4100 BLE / PSoC 4200 BLE / PSoC 4100M / PSoC 4200M /
        *  PSoC 4200L / PSoC 4000S / PSoC 4100S / PSoC Analog Coprocessor devices.
        *
        *******************************************************************************/
        void QR_SCANNER_UartEnableCts(void)
        {
            QR_SCANNER_UART_FLOW_CTRL_REG |= (uint32)  QR_SCANNER_UART_FLOW_CTRL_CTS_ENABLE;
        }


        /*******************************************************************************
        * Function Name: QR_SCANNER_UartDisableCts
        ****************************************************************************//**
        *
        *  Disables usage of CTS input signal by the UART transmitter.
        *  Only available for PSoC 4100 BLE / PSoC 4200 BLE / PSoC 4100M / PSoC 4200M /
        *  PSoC 4200L / PSoC 4000S / PSoC 4100S / PSoC Analog Coprocessor devices.
        *
        *******************************************************************************/
        void QR_SCANNER_UartDisableCts(void)
        {
            QR_SCANNER_UART_FLOW_CTRL_REG &= (uint32) ~QR_SCANNER_UART_FLOW_CTRL_CTS_ENABLE;
        }


        /*******************************************************************************
        * Function Name: QR_SCANNER_UartSetCtsPolarity
        ****************************************************************************//**
        *
        *  Sets active polarity of CTS input signal.
        *  Only available for PSoC 4100 BLE / PSoC 4200 BLE / PSoC 4100M / PSoC 4200M /
        *  PSoC 4200L / PSoC 4000S / PSoC 4100S / PSoC Analog Coprocessor devices.
        *
        * \param
        * polarity: Active polarity of CTS output signal.
        *   - QR_SCANNER_UART_CTS_ACTIVE_LOW  - CTS signal is active low.
        *   - QR_SCANNER_UART_CTS_ACTIVE_HIGH - CTS signal is active high.
        *
        *******************************************************************************/
        void QR_SCANNER_UartSetCtsPolarity(uint32 polarity)
        {
            if (0u != polarity)
            {
                QR_SCANNER_UART_FLOW_CTRL_REG |= (uint32)  QR_SCANNER_UART_FLOW_CTRL_CTS_POLARITY;
            }
            else
            {
                QR_SCANNER_UART_FLOW_CTRL_REG &= (uint32) ~QR_SCANNER_UART_FLOW_CTRL_CTS_POLARITY;
            }
        }
    #endif /* !(QR_SCANNER_CY_SCBIP_V0 || QR_SCANNER_CY_SCBIP_V1) */


    /*******************************************************************************
    * Function Name: QR_SCANNER_UartSendBreakBlocking
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
    void QR_SCANNER_UartSendBreakBlocking(uint32 breakWidth)
    {
        uint32 txCtrlReg;
        uint32 txIntrReg;

        /* Disable all UART TX interrupt source and clear UART TX Done history */
        txIntrReg = QR_SCANNER_GetTxInterruptMode();
        QR_SCANNER_SetTxInterruptMode(0u);
        QR_SCANNER_ClearTxInterruptSource(QR_SCANNER_INTR_TX_UART_DONE);

        /* Store TX CTRL configuration */
        txCtrlReg = QR_SCANNER_TX_CTRL_REG;

        /* Set break width */
        QR_SCANNER_TX_CTRL_REG = (QR_SCANNER_TX_CTRL_REG & (uint32) ~QR_SCANNER_TX_CTRL_DATA_WIDTH_MASK) |
                                        QR_SCANNER_GET_TX_CTRL_DATA_WIDTH(breakWidth);

        /* Generate break */
        QR_SCANNER_TX_FIFO_WR_REG = 0u;

        /* Wait for break completion */
        while (0u == (QR_SCANNER_GetTxInterruptSource() & QR_SCANNER_INTR_TX_UART_DONE))
        {
        }

        /* Clear all UART TX interrupt sources to  */
        QR_SCANNER_ClearTxInterruptSource(QR_SCANNER_INTR_TX_ALL);

        /* Restore TX interrupt sources and data width */
        QR_SCANNER_TX_CTRL_REG = txCtrlReg;
        QR_SCANNER_SetTxInterruptMode(txIntrReg);
    }
#endif /* (QR_SCANNER_UART_TX_DIRECTION) */


#if (QR_SCANNER_UART_WAKE_ENABLE_CONST)
    /*******************************************************************************
    * Function Name: QR_SCANNER_UartSaveConfig
    ****************************************************************************//**
    *
    *  Clears and enables an interrupt on a falling edge of the Rx input. The GPIO
    *  interrupt does not track in the active mode, therefore requires to be
    *  cleared by this API.
    *
    *******************************************************************************/
    void QR_SCANNER_UartSaveConfig(void)
    {
    #if (QR_SCANNER_UART_RX_WAKEUP_IRQ)
        /* Set SKIP_START if requested (set by default). */
        if (0u != QR_SCANNER_skipStart)
        {
            QR_SCANNER_UART_RX_CTRL_REG |= (uint32)  QR_SCANNER_UART_RX_CTRL_SKIP_START;
        }
        else
        {
            QR_SCANNER_UART_RX_CTRL_REG &= (uint32) ~QR_SCANNER_UART_RX_CTRL_SKIP_START;
        }

        /* Clear RX GPIO interrupt status and pending interrupt in NVIC because
        * falling edge on RX line occurs while UART communication in active mode.
        * Enable interrupt: next interrupt trigger should wakeup device.
        */
        QR_SCANNER_CLEAR_UART_RX_WAKE_INTR;
        QR_SCANNER_RxWakeClearPendingInt();
        QR_SCANNER_RxWakeEnableInt();
    #endif /* (QR_SCANNER_UART_RX_WAKEUP_IRQ) */
    }


    /*******************************************************************************
    * Function Name: QR_SCANNER_UartRestoreConfig
    ****************************************************************************//**
    *
    *  Disables the RX GPIO interrupt. Until this function is called the interrupt
    *  remains active and triggers on every falling edge of the UART RX line.
    *
    *******************************************************************************/
    void QR_SCANNER_UartRestoreConfig(void)
    {
    #if (QR_SCANNER_UART_RX_WAKEUP_IRQ)
        /* Disable interrupt: no more triggers in active mode */
        QR_SCANNER_RxWakeDisableInt();
    #endif /* (QR_SCANNER_UART_RX_WAKEUP_IRQ) */
    }


    #if (QR_SCANNER_UART_RX_WAKEUP_IRQ)
        /*******************************************************************************
        * Function Name: QR_SCANNER_UART_WAKEUP_ISR
        ****************************************************************************//**
        *
        *  Handles the Interrupt Service Routine for the SCB UART mode GPIO wakeup
        *  event. This event is configured to trigger on a falling edge of the RX line.
        *
        *******************************************************************************/
        CY_ISR(QR_SCANNER_UART_WAKEUP_ISR)
        {
        #ifdef QR_SCANNER_UART_WAKEUP_ISR_ENTRY_CALLBACK
            QR_SCANNER_UART_WAKEUP_ISR_EntryCallback();
        #endif /* QR_SCANNER_UART_WAKEUP_ISR_ENTRY_CALLBACK */

            QR_SCANNER_CLEAR_UART_RX_WAKE_INTR;

        #ifdef QR_SCANNER_UART_WAKEUP_ISR_EXIT_CALLBACK
            QR_SCANNER_UART_WAKEUP_ISR_ExitCallback();
        #endif /* QR_SCANNER_UART_WAKEUP_ISR_EXIT_CALLBACK */
        }
    #endif /* (QR_SCANNER_UART_RX_WAKEUP_IRQ) */
#endif /* (QR_SCANNER_UART_RX_WAKEUP_IRQ) */


/* [] END OF FILE */
