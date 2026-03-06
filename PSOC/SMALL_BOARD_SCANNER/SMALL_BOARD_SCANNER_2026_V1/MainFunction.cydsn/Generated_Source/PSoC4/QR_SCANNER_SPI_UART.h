/***************************************************************************//**
* \file QR_SCANNER_SPI_UART.h
* \version 4.0
*
* \brief
*  This file provides constants and parameter values for the SCB Component in
*  SPI and UART modes.
*
* Note:
*
********************************************************************************
* \copyright
* Copyright 2013-2017, Cypress Semiconductor Corporation.  All rights reserved.
* You may use this file only in accordance with the license, terms, conditions,
* disclaimers, and limitations in the end user license agreement accompanying
* the software package with which this file was provided.
*******************************************************************************/

#if !defined(CY_SCB_SPI_UART_QR_SCANNER_H)
#define CY_SCB_SPI_UART_QR_SCANNER_H

#include "QR_SCANNER.h"


/***************************************
*   SPI Initial Parameter Constants
****************************************/

#define QR_SCANNER_SPI_MODE                   (0u)
#define QR_SCANNER_SPI_SUB_MODE               (0u)
#define QR_SCANNER_SPI_CLOCK_MODE             (0u)
#define QR_SCANNER_SPI_OVS_FACTOR             (16u)
#define QR_SCANNER_SPI_MEDIAN_FILTER_ENABLE   (0u)
#define QR_SCANNER_SPI_LATE_MISO_SAMPLE_ENABLE (0u)
#define QR_SCANNER_SPI_RX_DATA_BITS_NUM       (8u)
#define QR_SCANNER_SPI_TX_DATA_BITS_NUM       (8u)
#define QR_SCANNER_SPI_WAKE_ENABLE            (0u)
#define QR_SCANNER_SPI_BITS_ORDER             (1u)
#define QR_SCANNER_SPI_TRANSFER_SEPARATION    (1u)
#define QR_SCANNER_SPI_NUMBER_OF_SS_LINES     (1u)
#define QR_SCANNER_SPI_RX_BUFFER_SIZE         (8u)
#define QR_SCANNER_SPI_TX_BUFFER_SIZE         (8u)

#define QR_SCANNER_SPI_INTERRUPT_MODE         (0u)

#define QR_SCANNER_SPI_INTR_RX_MASK           (0x0u)
#define QR_SCANNER_SPI_INTR_TX_MASK           (0x0u)

#define QR_SCANNER_SPI_RX_TRIGGER_LEVEL       (7u)
#define QR_SCANNER_SPI_TX_TRIGGER_LEVEL       (0u)

#define QR_SCANNER_SPI_BYTE_MODE_ENABLE       (0u)
#define QR_SCANNER_SPI_FREE_RUN_SCLK_ENABLE   (0u)
#define QR_SCANNER_SPI_SS0_POLARITY           (0u)
#define QR_SCANNER_SPI_SS1_POLARITY           (0u)
#define QR_SCANNER_SPI_SS2_POLARITY           (0u)
#define QR_SCANNER_SPI_SS3_POLARITY           (0u)


/***************************************
*   UART Initial Parameter Constants
****************************************/

#define QR_SCANNER_UART_SUB_MODE              (0u)
#define QR_SCANNER_UART_DIRECTION             (3u)
#define QR_SCANNER_UART_DATA_BITS_NUM         (8u)
#define QR_SCANNER_UART_PARITY_TYPE           (2u)
#define QR_SCANNER_UART_STOP_BITS_NUM         (2u)
#define QR_SCANNER_UART_OVS_FACTOR            (12u)
#define QR_SCANNER_UART_IRDA_LOW_POWER        (0u)
#define QR_SCANNER_UART_MEDIAN_FILTER_ENABLE  (0u)
#define QR_SCANNER_UART_RETRY_ON_NACK         (0u)
#define QR_SCANNER_UART_IRDA_POLARITY         (0u)
#define QR_SCANNER_UART_DROP_ON_FRAME_ERR     (0u)
#define QR_SCANNER_UART_DROP_ON_PARITY_ERR    (0u)
#define QR_SCANNER_UART_WAKE_ENABLE           (0u)
#define QR_SCANNER_UART_RX_BUFFER_SIZE        (8u)
#define QR_SCANNER_UART_TX_BUFFER_SIZE        (8u)
#define QR_SCANNER_UART_MP_MODE_ENABLE        (0u)
#define QR_SCANNER_UART_MP_ACCEPT_ADDRESS     (0u)
#define QR_SCANNER_UART_MP_RX_ADDRESS         (0x2u)
#define QR_SCANNER_UART_MP_RX_ADDRESS_MASK    (0xFFu)

#define QR_SCANNER_UART_INTERRUPT_MODE        (0u)

#define QR_SCANNER_UART_INTR_RX_MASK          (0x0u)
#define QR_SCANNER_UART_INTR_TX_MASK          (0x0u)

#define QR_SCANNER_UART_RX_TRIGGER_LEVEL      (7u)
#define QR_SCANNER_UART_TX_TRIGGER_LEVEL      (0u)

#define QR_SCANNER_UART_BYTE_MODE_ENABLE      (0u)
#define QR_SCANNER_UART_CTS_ENABLE            (0u)
#define QR_SCANNER_UART_CTS_POLARITY          (0u)
#define QR_SCANNER_UART_RTS_ENABLE            (0u)
#define QR_SCANNER_UART_RTS_POLARITY          (0u)
#define QR_SCANNER_UART_RTS_FIFO_LEVEL        (4u)

#define QR_SCANNER_UART_RX_BREAK_WIDTH        (11u)

/* SPI mode enum */
#define QR_SCANNER_SPI_SLAVE  (0u)
#define QR_SCANNER_SPI_MASTER (1u)

/* UART direction enum */
#define QR_SCANNER_UART_RX    (1u)
#define QR_SCANNER_UART_TX    (2u)
#define QR_SCANNER_UART_TX_RX (3u)


/***************************************
*   Conditional Compilation Parameters
****************************************/

#if(QR_SCANNER_SCB_MODE_UNCONFIG_CONST_CFG)

    /* Mode */
    #define QR_SCANNER_SPI_SLAVE_CONST        (1u)
    #define QR_SCANNER_SPI_MASTER_CONST       (1u)

    /* Direction */
    #define QR_SCANNER_RX_DIRECTION           (1u)
    #define QR_SCANNER_TX_DIRECTION           (1u)
    #define QR_SCANNER_UART_RX_DIRECTION      (1u)
    #define QR_SCANNER_UART_TX_DIRECTION      (1u)

    /* Only external RX and TX buffer for Uncofigured mode */
    #define QR_SCANNER_INTERNAL_RX_SW_BUFFER   (0u)
    #define QR_SCANNER_INTERNAL_TX_SW_BUFFER   (0u)

    /* Get RX and TX buffer size */
    #define QR_SCANNER_INTERNAL_RX_BUFFER_SIZE    (QR_SCANNER_rxBufferSize + 1u)
    #define QR_SCANNER_RX_BUFFER_SIZE             (QR_SCANNER_rxBufferSize)
    #define QR_SCANNER_TX_BUFFER_SIZE             (QR_SCANNER_txBufferSize)

    /* Return true if buffer is provided */
    #define QR_SCANNER_CHECK_RX_SW_BUFFER (NULL != QR_SCANNER_rxBuffer)
    #define QR_SCANNER_CHECK_TX_SW_BUFFER (NULL != QR_SCANNER_txBuffer)

    /* Always provide global variables to support RX and TX buffers */
    #define QR_SCANNER_INTERNAL_RX_SW_BUFFER_CONST    (1u)
    #define QR_SCANNER_INTERNAL_TX_SW_BUFFER_CONST    (1u)

    /* Get wakeup enable option */
    #define QR_SCANNER_SPI_WAKE_ENABLE_CONST  (1u)
    #define QR_SCANNER_UART_WAKE_ENABLE_CONST (1u)
    #define QR_SCANNER_CHECK_SPI_WAKE_ENABLE  ((0u != QR_SCANNER_scbEnableWake) && QR_SCANNER_SCB_MODE_SPI_RUNTM_CFG)
    #define QR_SCANNER_CHECK_UART_WAKE_ENABLE ((0u != QR_SCANNER_scbEnableWake) && QR_SCANNER_SCB_MODE_UART_RUNTM_CFG)

    /* SPI/UART: TX or RX FIFO size */
    #if (QR_SCANNER_CY_SCBIP_V0 || QR_SCANNER_CY_SCBIP_V1)
        #define QR_SCANNER_SPI_UART_FIFO_SIZE             (QR_SCANNER_FIFO_SIZE)
        #define QR_SCANNER_CHECK_UART_RTS_CONTROL_FLOW    (0u)
    #else
        #define QR_SCANNER_SPI_UART_FIFO_SIZE (QR_SCANNER_GET_FIFO_SIZE(QR_SCANNER_CTRL_REG & \
                                                                                    QR_SCANNER_CTRL_BYTE_MODE))

        #define QR_SCANNER_CHECK_UART_RTS_CONTROL_FLOW \
                    ((QR_SCANNER_SCB_MODE_UART_RUNTM_CFG) && \
                     (0u != QR_SCANNER_GET_UART_FLOW_CTRL_TRIGGER_LEVEL(QR_SCANNER_UART_FLOW_CTRL_REG)))
    #endif /* (QR_SCANNER_CY_SCBIP_V0 || QR_SCANNER_CY_SCBIP_V1) */

#else

    /* Internal RX and TX buffer: for SPI or UART */
    #if (QR_SCANNER_SCB_MODE_SPI_CONST_CFG)

        /* SPI Direction */
        #define QR_SCANNER_SPI_RX_DIRECTION (1u)
        #define QR_SCANNER_SPI_TX_DIRECTION (1u)

        /* Get FIFO size */
        #define QR_SCANNER_SPI_UART_FIFO_SIZE QR_SCANNER_GET_FIFO_SIZE(QR_SCANNER_SPI_BYTE_MODE_ENABLE)

        /* SPI internal RX and TX buffers */
        #define QR_SCANNER_INTERNAL_SPI_RX_SW_BUFFER  (QR_SCANNER_SPI_RX_BUFFER_SIZE > \
                                                                QR_SCANNER_SPI_UART_FIFO_SIZE)
        #define QR_SCANNER_INTERNAL_SPI_TX_SW_BUFFER  (QR_SCANNER_SPI_TX_BUFFER_SIZE > \
                                                                QR_SCANNER_SPI_UART_FIFO_SIZE)

        /* Internal SPI RX and TX buffer */
        #define QR_SCANNER_INTERNAL_RX_SW_BUFFER  (QR_SCANNER_INTERNAL_SPI_RX_SW_BUFFER)
        #define QR_SCANNER_INTERNAL_TX_SW_BUFFER  (QR_SCANNER_INTERNAL_SPI_TX_SW_BUFFER)

        /* Internal SPI RX and TX buffer size */
        #define QR_SCANNER_INTERNAL_RX_BUFFER_SIZE    (QR_SCANNER_SPI_RX_BUFFER_SIZE + 1u)
        #define QR_SCANNER_RX_BUFFER_SIZE             (QR_SCANNER_SPI_RX_BUFFER_SIZE)
        #define QR_SCANNER_TX_BUFFER_SIZE             (QR_SCANNER_SPI_TX_BUFFER_SIZE)

        /* Get wakeup enable option */
        #define QR_SCANNER_SPI_WAKE_ENABLE_CONST  (0u != QR_SCANNER_SPI_WAKE_ENABLE)
        #define QR_SCANNER_UART_WAKE_ENABLE_CONST (0u)

    #else

        /* UART Direction */
        #define QR_SCANNER_UART_RX_DIRECTION (0u != (QR_SCANNER_UART_DIRECTION & QR_SCANNER_UART_RX))
        #define QR_SCANNER_UART_TX_DIRECTION (0u != (QR_SCANNER_UART_DIRECTION & QR_SCANNER_UART_TX))

        /* Get FIFO size */
        #define QR_SCANNER_SPI_UART_FIFO_SIZE QR_SCANNER_GET_FIFO_SIZE(QR_SCANNER_UART_BYTE_MODE_ENABLE)

        /* UART internal RX and TX buffers */
        #define QR_SCANNER_INTERNAL_UART_RX_SW_BUFFER  (QR_SCANNER_UART_RX_BUFFER_SIZE > \
                                                                QR_SCANNER_SPI_UART_FIFO_SIZE)
        #define QR_SCANNER_INTERNAL_UART_TX_SW_BUFFER  (QR_SCANNER_UART_TX_BUFFER_SIZE > \
                                                                    QR_SCANNER_SPI_UART_FIFO_SIZE)

        /* Internal UART RX and TX buffer */
        #define QR_SCANNER_INTERNAL_RX_SW_BUFFER  (QR_SCANNER_INTERNAL_UART_RX_SW_BUFFER)
        #define QR_SCANNER_INTERNAL_TX_SW_BUFFER  (QR_SCANNER_INTERNAL_UART_TX_SW_BUFFER)

        /* Internal UART RX and TX buffer size */
        #define QR_SCANNER_INTERNAL_RX_BUFFER_SIZE    (QR_SCANNER_UART_RX_BUFFER_SIZE + 1u)
        #define QR_SCANNER_RX_BUFFER_SIZE             (QR_SCANNER_UART_RX_BUFFER_SIZE)
        #define QR_SCANNER_TX_BUFFER_SIZE             (QR_SCANNER_UART_TX_BUFFER_SIZE)

        /* Get wakeup enable option */
        #define QR_SCANNER_SPI_WAKE_ENABLE_CONST  (0u)
        #define QR_SCANNER_UART_WAKE_ENABLE_CONST (0u != QR_SCANNER_UART_WAKE_ENABLE)

    #endif /* (QR_SCANNER_SCB_MODE_SPI_CONST_CFG) */

    /* Mode */
    #define QR_SCANNER_SPI_SLAVE_CONST    (QR_SCANNER_SPI_MODE == QR_SCANNER_SPI_SLAVE)
    #define QR_SCANNER_SPI_MASTER_CONST   (QR_SCANNER_SPI_MODE == QR_SCANNER_SPI_MASTER)

    /* Direction */
    #define QR_SCANNER_RX_DIRECTION ((QR_SCANNER_SCB_MODE_SPI_CONST_CFG) ? \
                                            (QR_SCANNER_SPI_RX_DIRECTION) : (QR_SCANNER_UART_RX_DIRECTION))

    #define QR_SCANNER_TX_DIRECTION ((QR_SCANNER_SCB_MODE_SPI_CONST_CFG) ? \
                                            (QR_SCANNER_SPI_TX_DIRECTION) : (QR_SCANNER_UART_TX_DIRECTION))

    /* Internal RX and TX buffer: for SPI or UART. Used in conditional compilation check */
    #define QR_SCANNER_CHECK_RX_SW_BUFFER (QR_SCANNER_INTERNAL_RX_SW_BUFFER)
    #define QR_SCANNER_CHECK_TX_SW_BUFFER (QR_SCANNER_INTERNAL_TX_SW_BUFFER)

    /* Provide global variables to support RX and TX buffers */
    #define QR_SCANNER_INTERNAL_RX_SW_BUFFER_CONST    (QR_SCANNER_INTERNAL_RX_SW_BUFFER)
    #define QR_SCANNER_INTERNAL_TX_SW_BUFFER_CONST    (QR_SCANNER_INTERNAL_TX_SW_BUFFER)

    /* Wake up enable */
    #define QR_SCANNER_CHECK_SPI_WAKE_ENABLE  (QR_SCANNER_SPI_WAKE_ENABLE_CONST)
    #define QR_SCANNER_CHECK_UART_WAKE_ENABLE (QR_SCANNER_UART_WAKE_ENABLE_CONST)

    /* UART flow control: not applicable for CY_SCBIP_V0 || CY_SCBIP_V1 */
    #define QR_SCANNER_CHECK_UART_RTS_CONTROL_FLOW    (QR_SCANNER_SCB_MODE_UART_CONST_CFG && \
                                                             QR_SCANNER_UART_RTS_ENABLE)

#endif /* End (QR_SCANNER_SCB_MODE_UNCONFIG_CONST_CFG) */


/***************************************
*       Type Definitions
***************************************/

/**
* \addtogroup group_structures
* @{
*/

/* QR_SCANNER_SPI_INIT_STRUCT */
typedef struct
{
    /** Mode of operation for SPI. The following defines are available choices:
     *  - QR_SCANNER_SPI_SLAVE
     *  - QR_SCANNER_SPI_MASTE
    */
    uint32 mode;

    /** Submode of operation for SPI. The following defines are available
     *  choices:
     *  - QR_SCANNER_SPI_MODE_MOTOROLA
     *  - QR_SCANNER_SPI_MODE_TI_COINCIDES
     *  - QR_SCANNER_SPI_MODE_TI_PRECEDES
     *  - QR_SCANNER_SPI_MODE_NATIONAL
    */
    uint32 submode;

    /** Determines the sclk relationship for Motorola submode. Ignored
     *  for other submodes. The following defines are available choices:
     *  - QR_SCANNER_SPI_SCLK_CPHA0_CPOL0
     *  - QR_SCANNER_SPI_SCLK_CPHA0_CPOL1
     *  - QR_SCANNER_SPI_SCLK_CPHA1_CPOL0
     *  - QR_SCANNER_SPI_SCLK_CPHA1_CPOL1
    */
    uint32 sclkMode;

    /** Oversampling factor for the SPI clock. Ignored for Slave mode operation.
    */
    uint32 oversample;

    /** Applies median filter on the input lines: 0 – not applied, 1 – applied.
    */
    uint32 enableMedianFilter;

    /** Applies late sampling of MISO line: 0 – not applied, 1 – applied.
     *  Ignored for slave mode.
    */
    uint32 enableLateSampling;

    /** Enables wakeup from low power mode: 0 – disable, 1 – enable.
     *  Ignored for master mode.
    */
    uint32 enableWake;

    /** Number of data bits for RX direction.
     *  Different dataBitsRx and dataBitsTx are only allowed for National
     *  submode.
    */
    uint32 rxDataBits;

    /** Number of data bits for TX direction.
     *  Different dataBitsRx and dataBitsTx are only allowed for National
     *  submode.
    */
    uint32 txDataBits;

    /** Determines the bit ordering. The following defines are available
     *  choices:
     *  - QR_SCANNER_BITS_ORDER_LSB_FIRST
     *  - QR_SCANNER_BITS_ORDER_MSB_FIRST
    */
    uint32 bitOrder;

    /** Determines whether transfers are back to back or have SS disabled
     *  between words. Ignored for slave mode. The following defines are
     *  available choices:
     *  - QR_SCANNER_SPI_TRANSFER_CONTINUOUS
     *  - QR_SCANNER_SPI_TRANSFER_SEPARATED
    */
    uint32 transferSeperation;

    /** Size of the RX buffer in bytes/words (depends on rxDataBits parameter).
     *  A value equal to the RX FIFO depth implies the usage of buffering in
     *  hardware. A value greater than the RX FIFO depth results in a software
     *  buffer.
     *  The QR_SCANNER_INTR _RX_NOT_EMPTY interrupt has to be enabled to
     *  transfer data into the software buffer.
     *  - The RX and TX FIFO depth is equal to 8 bytes/words for PSoC 4100 /
     *    PSoC 4200 devices.
     *  - The RX and TX FIFO depth is equal to 8 bytes/words or 16
     *    bytes (Byte mode is enabled) for PSoC 4100 BLE / PSoC 4200 BLE /
     *    PSoC 4100M / PSoC 4200M / PSoC 4200L / PSoC 4000S / PSoC 4100S /
     *    PSoC Analog Coprocessor devices.
    */
    uint32 rxBufferSize;

    /** Buffer space provided for a RX software buffer:
     *  - A NULL pointer must be provided to use hardware buffering.
     *  - A pointer to an allocated buffer must be provided to use software
     *    buffering. The buffer size must equal (rxBufferSize + 1) in bytes if
     *    dataBitsRx is less or equal to 8, otherwise (2 * (rxBufferSize + 1))
     *    in bytes. The software RX buffer always keeps one element empty.
     *    For correct operation the allocated RX buffer has to be one element
     *    greater than maximum packet size expected to be received.
    */
    uint8* rxBuffer;

    /** Size of the TX buffer in bytes/words(depends on txDataBits parameter).
     *  A value equal to the TX FIFO depth implies the usage of buffering in
     *  hardware. A value greater than the TX FIFO depth results in a software
     *  buffer.
     *  - The RX and TX FIFO depth is equal to 8 bytes/words for PSoC 4100 /
     *    PSoC 4200 devices.
     *  - The RX and TX FIFO depth is equal to 8 bytes/words or 16
     *    bytes (Byte mode is enabled) for PSoC 4100 BLE / PSoC 4200 BLE /
     *    PSoC 4100M / PSoC 4200M / PSoC 4200L / PSoC 4000S / PSoC 4100S /
     *    PSoC Analog Coprocessor devices.
    */
    uint32 txBufferSize;

    /** Buffer space provided for a TX software buffer:
     *  - A NULL pointer must be provided to use hardware buffering.
     *  - A pointer to an allocated buffer must be provided to use software
     *    buffering. The buffer size must equal txBufferSize if dataBitsTx is
     *    less or equal to 8, otherwise (2* txBufferSize).
    */
    uint8* txBuffer;

    /** Enables component interrupt: 0 – disable, 1 – enable.
     *  The interrupt has to be enabled if software buffer is used.
    */
    uint32 enableInterrupt;

    /** Mask of enabled interrupt sources for the RX direction. This mask is
     *  written regardless of the setting of the enable Interrupt field.
     *  Multiple sources are enabled by providing a value that is the OR of
     *  all of the following sources to enable:
     *  - QR_SCANNER_INTR_RX_FIFO_LEVEL
     *  - QR_SCANNER_INTR_RX_NOT_EMPTY
     *  - QR_SCANNER_INTR_RX_FULL
     *  - QR_SCANNER_INTR_RX_OVERFLOW
     *  - QR_SCANNER_INTR_RX_UNDERFLOW
     *  - QR_SCANNER_INTR_SLAVE_SPI_BUS_ERROR
    */
    uint32 rxInterruptMask;

    /** FIFO level for an RX FIFO level interrupt. This value is written
     *  regardless of whether the RX FIFO level interrupt source is enabled.
    */
    uint32 rxTriggerLevel;

    /** Mask of enabled interrupt sources for the TX direction. This mask is
     *  written regardless of the setting of the enable Interrupt field.
     *  Multiple sources are enabled by providing a value that is the OR of
     *  all of the following sources to enable:
     *  - QR_SCANNER_INTR_TX_FIFO_LEVEL
     *  - QR_SCANNER_INTR_TX_NOT_FULL
     *  - QR_SCANNER_INTR_TX_EMPTY
     *  - QR_SCANNER_INTR_TX_OVERFLOW
     *  - QR_SCANNER_INTR_TX_UNDERFLOW
     *  - QR_SCANNER_INTR_MASTER_SPI_DONE
    */
    uint32 txInterruptMask;

    /** FIFO level for a TX FIFO level interrupt. This value is written
     * regardless of whether the TX FIFO level interrupt source is enabled.
    */
    uint32 txTriggerLevel;

    /** When enabled the TX and RX FIFO depth is doubled and equal to
     *  16 bytes: 0 – disable, 1 – enable. This implies that number of
     *  TX and RX data bits must be less than or equal to 8.
     *
     *  Ignored for all devices other than PSoC 4100 BLE / PSoC 4200 BLE /
     *  PSoC 4100M / PSoC 4200M / PSoC 4200L / PSoC 4000S / PSoC 4100S /
     *  PSoC Analog Coprocessor.
    */
    uint8 enableByteMode;

    /** Enables continuous SCLK generation by the SPI master: 0 – disable,
     *  1 – enable.
     *
     *  Ignored for all devices other than PSoC 4100 BLE / PSoC 4200 BLE /
     *  PSoC 4100M / PSoC 4200M / PSoC 4200L / PSoC 4000S / PSoC 4100S /
     *  PSoC Analog Coprocessor.
    */
    uint8 enableFreeRunSclk;

    /** Active polarity of slave select lines 0-3. This is bit mask where bit
     *  QR_SCANNER_SPI_SLAVE_SELECT0 corresponds to slave select 0
     *  polarity, bit QR_SCANNER_SPI_SLAVE_SELECT1 – slave select 1
     *  polarity and so on. Polarity constants are:
     *  - QR_SCANNER_SPI_SS_ACTIVE_LOW
     *  - QR_SCANNER_SPI_SS_ACTIVE_HIGH
     *
     *  Ignored for all devices other than PSoC 4100 BLE / PSoC 4200 BLE /
     *  PSoC 4100M / PSoC 4200M / PSoC 4200L / PSoC 4000S / PSoC 4100S /
     *  PSoC Analog Coprocessor.
    */
    uint8 polaritySs;
} QR_SCANNER_SPI_INIT_STRUCT;


/* QR_SCANNER_UART_INIT_STRUCT */
typedef struct
{
    /** Mode of operation for the UART. The following defines are available
     *  choices:
     *  - QR_SCANNER_UART_MODE_STD
     *  - QR_SCANNER_UART_MODE_SMARTCARD
     *  - QR_SCANNER_UART_MODE_IRDA
    */
    uint32 mode;

    /** Direction of operation for the UART. The following defines are available
     *  choices:
     *  - QR_SCANNER_UART_TX_RX
     *  - QR_SCANNER_UART_RX
     *  - QR_SCANNER_UART_TX
    */
    uint32 direction;

    /** Number of data bits.
    */
    uint32 dataBits;

    /** Determines the parity. The following defines are available choices:
     *  - QR_SCANNER_UART_PARITY_EVEN
     *  - QR_SCANNER_UART_PARITY_ODD
     *  - QR_SCANNER_UART_PARITY_NONE
    */
    uint32 parity;

    /** Determines the number of stop bits. The following defines are available
     *  choices:
     *  - QR_SCANNER_UART_STOP_BITS_1
     *  - QR_SCANNER_UART_STOP_BITS_1_5
     *  - QR_SCANNER_UART_STOP_BITS_2
    */
    uint32 stopBits;

    /** Oversampling factor for the UART.
     *
     *  Note The oversampling factor values are changed when enableIrdaLowPower
     *  is enabled:
     *  - QR_SCANNER_UART_IRDA_LP_OVS16
     *  - QR_SCANNER_UART_IRDA_LP_OVS32
     *  - QR_SCANNER_UART_IRDA_LP_OVS48
     *  - QR_SCANNER_UART_IRDA_LP_OVS96
     *  - QR_SCANNER_UART_IRDA_LP_OVS192
     *  - QR_SCANNER_UART_IRDA_LP_OVS768
     *  - QR_SCANNER_UART_IRDA_LP_OVS1536
    */
    uint32 oversample;

    /** Enables IrDA low power RX mode operation: 0 – disable, 1 – enable.
     *  The TX functionality does not work when enabled.
    */
    uint32 enableIrdaLowPower;

    /** Applies median filter on the input lines:  0 – not applied, 1 – applied.
    */
    uint32 enableMedianFilter;

    /** Enables retry when NACK response was received: 0 – disable, 1 – enable.
     *  Only current content of TX FIFO is re-sent.
     *  Ignored for modes other than SmartCard.
    */
    uint32 enableRetryNack;

    /** Inverts polarity of RX line: 0 – non-inverting, 1 – inverting.
     *  Ignored for modes other than IrDA.
    */
    uint32 enableInvertedRx;

    /** Drop data from RX FIFO if parity error is detected: 0 – disable,
     *  1 – enable.
    */
    uint32 dropOnParityErr;

    /** Drop data from RX FIFO if a frame error is detected: 0 – disable,
     *  1 – enable.
    */
    uint32 dropOnFrameErr;

    /** Enables wakeup from low power mode: 0 – disable, 1 – enable.
     *  Ignored for modes other than standard UART. The RX functionality
     *  has to be enabled.
    */
    uint32 enableWake;

    /** Size of the RX buffer in bytes/words (depends on rxDataBits parameter).
     *  A value equal to the RX FIFO depth implies the usage of buffering in
     *  hardware. A value greater than the RX FIFO depth results in a software
     *  buffer.
     *  The QR_SCANNER_INTR _RX_NOT_EMPTY interrupt has to be enabled to
     *  transfer data into the software buffer.
     *  - The RX and TX FIFO depth is equal to 8 bytes/words for PSoC 4100 /
     *    PSoC 4200 devices.
     *  - The RX and TX FIFO depth is equal to 8 bytes/words or 16
     *    bytes (Byte mode is enabled) for PSoC 4100 BLE / PSoC 4200 BLE /
     *    PSoC 4100M / PSoC 4200M / PSoC 4200L / PSoC 4000S / PSoC 4100S /
     *    PSoC Analog Coprocessor devices.
    */
    uint32 rxBufferSize;

    /** Buffer space provided for a RX software buffer:
     *  - A NULL pointer must be provided to use hardware buffering.
     *  - A pointer to an allocated buffer must be provided to use software
     *    buffering. The buffer size must equal (rxBufferSize + 1) in bytes if
     *    dataBitsRx is less or equal to 8, otherwise (2 * (rxBufferSize + 1))
     *    in bytes. The software RX buffer always keeps one element empty.
     *    For correct operation the allocated RX buffer has to be one element
     *    greater than maximum packet size expected to be received.
    */
    uint8* rxBuffer;

    /** Size of the TX buffer in bytes/words(depends on txDataBits parameter).
     *  A value equal to the TX FIFO depth implies the usage of buffering in
     *  hardware. A value greater than the TX FIFO depth results in a software
     *  buffer.
     *  - The RX and TX FIFO depth is equal to 8 bytes/words for PSoC 4100 /
     *    PSoC 4200 devices.
     *  - The RX and TX FIFO depth is equal to 8 bytes/words or 16
     *    bytes (Byte mode is enabled) for PSoC 4100 BLE / PSoC 4200 BLE /
     *    PSoC 4100M / PSoC 4200M / PSoC 4200L / PSoC 4000S / PSoC 4100S /
     *    PSoC Analog Coprocessor devices.
    */
    uint32 txBufferSize;

    /** Buffer space provided for a TX software buffer:
     *  - A NULL pointer must be provided to use hardware buffering.
     *  - A pointer to an allocated buffer must be provided to use software
     *    buffering. The buffer size must equal txBufferSize if dataBitsTx is
     *    less or equal to 8, otherwise (2* txBufferSize).
    */
    uint8* txBuffer;

    /** Enables multiprocessor mode: 0 – disable, 1 – enable.
    */
    uint32 enableMultiproc;

    /** Enables matched address to be accepted: 0 – disable, 1 – enable.
    */
    uint32 multiprocAcceptAddr;

    /** 8 bit address to match in Multiprocessor mode. Ignored for other modes.
    */
    uint32 multiprocAddr;

    /** 8 bit mask of address bits that are compared for a Multiprocessor
     *  address match. Ignored for other modes.
     *  - Bit value 0 – excludes bit from address comparison.
     *  - Bit value 1 – the bit needs to match with the corresponding bit
     *   of the device address.
    */
    uint32 multiprocAddrMask;

    /** Enables component interrupt: 0 – disable, 1 – enable.
     *  The interrupt has to be enabled if software buffer is used.
    */
    uint32 enableInterrupt;

    /** Mask of interrupt sources to enable in the RX direction. This mask is
     *  written regardless of the setting of the enableInterrupt field.
     *  Multiple sources are enabled by providing a value that is the OR of
     *  all of the following sources to enable:
     *  - QR_SCANNER_INTR_RX_FIFO_LEVEL
     *  - QR_SCANNER_INTR_RX_NOT_EMPTY
     *  - QR_SCANNER_INTR_RX_FULL
     *  - QR_SCANNER_INTR_RX_OVERFLOW
     *  - QR_SCANNER_INTR_RX_UNDERFLOW
     *  - QR_SCANNER_INTR_RX_FRAME_ERROR
     *  - QR_SCANNER_INTR_RX_PARITY_ERROR
    */
    uint32 rxInterruptMask;

    /** FIFO level for an RX FIFO level interrupt. This value is written
     *  regardless of whether the RX FIFO level interrupt source is enabled.
    */
    uint32 rxTriggerLevel;

    /** Mask of interrupt sources to enable in the TX direction. This mask is
     *  written regardless of the setting of the enableInterrupt field.
     *  Multiple sources are enabled by providing a value that is the OR of
     *  all of the following sources to enable:
     *  - QR_SCANNER_INTR_TX_FIFO_LEVEL
     *  - QR_SCANNER_INTR_TX_NOT_FULL
     *  - QR_SCANNER_INTR_TX_EMPTY
     *  - QR_SCANNER_INTR_TX_OVERFLOW
     *  - QR_SCANNER_INTR_TX_UNDERFLOW
     *  - QR_SCANNER_INTR_TX_UART_DONE
     *  - QR_SCANNER_INTR_TX_UART_NACK
     *  - QR_SCANNER_INTR_TX_UART_ARB_LOST
    */
    uint32 txInterruptMask;

    /** FIFO level for a TX FIFO level interrupt. This value is written
     *  regardless of whether the TX FIFO level interrupt source is enabled.
    */
    uint32 txTriggerLevel;

    /** When enabled the TX and RX FIFO depth is doubled and equal to
     *  16 bytes: 0 – disable, 1 – enable. This implies that number of
     *  Data bits must be less than or equal to 8.
     *
     *  Ignored for all devices other than PSoC 4100 BLE / PSoC 4200 BLE /
     *  PSoC 4100M / PSoC 4200M / PSoC 4200L / PSoC 4000S / PSoC 4100S /
     *  PSoC Analog Coprocessor.
    */
    uint8 enableByteMode;

    /** Enables usage of CTS input signal by the UART transmitter : 0 – disable,
     *  1 – enable.
     *
     *  Ignored for all devices other than PSoC 4100 BLE / PSoC 4200 BLE /
     *  PSoC 4100M / PSoC 4200M / PSoC 4200L / PSoC 4000S / PSoC 4100S /
     *  PSoC Analog Coprocessor.
    */
    uint8 enableCts;

    /** Sets active polarity of CTS input signal:
     *  - QR_SCANNER_UART_CTS_ACTIVE_LOW
     *  - QR_SCANNER_UART_CTS_ACTIVE_HIGH
     *
     *  Ignored for all devices other than PSoC 4100 BLE / PSoC 4200 BLE /
     *  PSoC 4100M / PSoC 4200M / PSoC 4200L / PSoC 4000S / PSoC 4100S /
     *  PSoC Analog Coprocessor.
    */
    uint8 ctsPolarity;

    /** RX FIFO level for RTS signal activation. While the RX FIFO has fewer
     *  entries than the RTS FIFO level value the RTS signal remains active,
     *  otherwise the RTS signal becomes inactive. By setting this field to 0,
     *  RTS signal activation is disabled.
     *
     *  Ignored for all devices other than PSoC 4100 BLE / PSoC 4200 BLE /
     *  PSoC 4100M / PSoC 4200M / PSoC 4200L / PSoC 4000S / PSoC 4100S /
     *  PSoC Analog Coprocessor.
    */
    uint8 rtsRxFifoLevel;

    /** Sets active polarity of RTS output signal:
     *  - QR_SCANNER_UART_RTS_ ACTIVE_LOW
     *  - QR_SCANNER_UART_RTS_ACTIVE_HIGH
     *
     *  Ignored for all devices other than PSoC 4100 BLE / PSoC 4200 BLE /
     *  PSoC 4100M / PSoC 4200M / PSoC 4200L / PSoC 4000S / PSoC 4100S /
     *  PSoC Analog Coprocessor.
    */
    uint8 rtsPolarity;

    /** Configures the width of a break signal in that triggers the break
     *  detection interrupt source. A Break is a low level on the RX line.
     *  Valid range is 1-16 UART bits times.
    */
    uint8 breakWidth;
} QR_SCANNER_UART_INIT_STRUCT;

/** @} structures */


/***************************************
*        Function Prototypes
***************************************/

/**
* \addtogroup group_spi
* @{
*/
/* SPI specific functions */
#if(QR_SCANNER_SCB_MODE_UNCONFIG_CONST_CFG)
    void QR_SCANNER_SpiInit(const QR_SCANNER_SPI_INIT_STRUCT *config);
#endif /* (QR_SCANNER_SCB_MODE_UNCONFIG_CONST_CFG) */


#if(QR_SCANNER_SCB_MODE_SPI_INC)
    /*******************************************************************************
    * Function Name: QR_SCANNER_SpiIsBusBusy
    ****************************************************************************//**
    *
    *  Returns the current status on the bus. The bus status is determined using
    *  the slave select signal.
    *  - Motorola and National Semiconductor sub-modes: The bus is busy after
    *    the slave select line is activated and lasts until the slave select line
    *    is deactivated.
    *  - Texas Instrument sub-modes: The bus is busy at the moment of the initial
    *    pulse on the slave select line and lasts until the transfer is complete.
    *    If SPI Master is configured to use "Separated transfers"
    *    (see Continuous versus Separated Transfer Separation), the bus is busy
    *    during each element transfer and is free between each element transfer.
    *    The Master does not activate SS line immediately after data has been
    *    written into the TX FIFO.
    *
    *  \return slaveSelect: Current status on the bus.
    *   If the returned value is nonzero, the bus is busy.
    *   If zero is returned, the bus is free. The bus status is determined using
    *   the slave select signal.
    *
    *******************************************************************************/
    #define QR_SCANNER_SpiIsBusBusy() ((uint32) (0u != (QR_SCANNER_SPI_STATUS_REG & \
                                                              QR_SCANNER_SPI_STATUS_BUS_BUSY)))

    #if (QR_SCANNER_SPI_MASTER_CONST)
        void QR_SCANNER_SpiSetActiveSlaveSelect(uint32 slaveSelect);
    #endif /*(QR_SCANNER_SPI_MASTER_CONST) */

    #if !(QR_SCANNER_CY_SCBIP_V0 || QR_SCANNER_CY_SCBIP_V1)
        void QR_SCANNER_SpiSetSlaveSelectPolarity(uint32 slaveSelect, uint32 polarity);
    #endif /* !(QR_SCANNER_CY_SCBIP_V0 || QR_SCANNER_CY_SCBIP_V1) */
#endif /* (QR_SCANNER_SCB_MODE_SPI_INC) */
/** @} spi */

/**
* \addtogroup group_uart
* @{
*/
/* UART specific functions */
#if(QR_SCANNER_SCB_MODE_UNCONFIG_CONST_CFG)
    void QR_SCANNER_UartInit(const QR_SCANNER_UART_INIT_STRUCT *config);
#endif /* (QR_SCANNER_SCB_MODE_UNCONFIG_CONST_CFG) */

#if(QR_SCANNER_SCB_MODE_UART_INC)
    void QR_SCANNER_UartSetRxAddress(uint32 address);
    void QR_SCANNER_UartSetRxAddressMask(uint32 addressMask);


    /* UART RX direction APIs */
    #if(QR_SCANNER_UART_RX_DIRECTION)
        uint32 QR_SCANNER_UartGetChar(void);
        uint32 QR_SCANNER_UartGetByte(void);

        #if !(QR_SCANNER_CY_SCBIP_V0 || QR_SCANNER_CY_SCBIP_V1)
            /* UART APIs for Flow Control */
            void QR_SCANNER_UartSetRtsPolarity(uint32 polarity);
            void QR_SCANNER_UartSetRtsFifoLevel(uint32 level);
        #endif /* !(QR_SCANNER_CY_SCBIP_V0 || QR_SCANNER_CY_SCBIP_V1) */
    #endif /* (QR_SCANNER_UART_RX_DIRECTION) */

    /* UART TX direction APIs */
    #if(QR_SCANNER_UART_TX_DIRECTION)
        /*******************************************************************************
        * Function Name: QR_SCANNER_UartPutChar
        ****************************************************************************//**
        *
        *  Places a byte of data in the transmit buffer to be sent at the next available
        *  bus time. This function is blocking and waits until there is a space
        *  available to put requested data in the transmit buffer.
        *  For UART Multi Processor mode this function can send 9-bits data as well.
        *  Use QR_SCANNER_UART_MP_MARK to add a mark to create an address byte.
        *
        *  \param txDataByte: the data to be transmitted.
        *
        *******************************************************************************/
        #define QR_SCANNER_UartPutChar(ch)    QR_SCANNER_SpiUartWriteTxData((uint32)(ch))

        void QR_SCANNER_UartPutString(const char8 string[]);
        void QR_SCANNER_UartPutCRLF(uint32 txDataByte);
        void QR_SCANNER_UartSendBreakBlocking(uint32 breakWidth);

        #if !(QR_SCANNER_CY_SCBIP_V0 || QR_SCANNER_CY_SCBIP_V1)
            /* UART APIs for Flow Control */
            void QR_SCANNER_UartEnableCts(void);
            void QR_SCANNER_UartDisableCts(void);
            void QR_SCANNER_UartSetCtsPolarity(uint32 polarity);
        #endif /* !(QR_SCANNER_CY_SCBIP_V0 || QR_SCANNER_CY_SCBIP_V1) */
    #endif /* (QR_SCANNER_UART_TX_DIRECTION) */
#endif /* (QR_SCANNER_SCB_MODE_UART_INC) */
/** @} uart */

/**
* \addtogroup group_spi_uart
* @{
*/
#if(QR_SCANNER_RX_DIRECTION)
    uint32 QR_SCANNER_SpiUartReadRxData(void);
    uint32 QR_SCANNER_SpiUartGetRxBufferSize(void);
    void   QR_SCANNER_SpiUartClearRxBuffer(void);
#endif /* (QR_SCANNER_RX_DIRECTION) */

/* Common APIs TX direction */
#if(QR_SCANNER_TX_DIRECTION)
    void   QR_SCANNER_SpiUartWriteTxData(uint32 txData);
    void   QR_SCANNER_SpiUartPutArray(const uint8 wrBuf[], uint32 count);
    uint32 QR_SCANNER_SpiUartGetTxBufferSize(void);
    void   QR_SCANNER_SpiUartClearTxBuffer(void);
#endif /* (QR_SCANNER_TX_DIRECTION) */
/** @} spi_uart */

CY_ISR_PROTO(QR_SCANNER_SPI_UART_ISR);

#if(QR_SCANNER_UART_RX_WAKEUP_IRQ)
    CY_ISR_PROTO(QR_SCANNER_UART_WAKEUP_ISR);
#endif /* (QR_SCANNER_UART_RX_WAKEUP_IRQ) */


/***************************************
*     Buffer Access Macro Definitions
***************************************/

#if(QR_SCANNER_SCB_MODE_UNCONFIG_CONST_CFG)
    /* RX direction */
    void   QR_SCANNER_PutWordInRxBuffer  (uint32 idx, uint32 rxDataByte);
    uint32 QR_SCANNER_GetWordFromRxBuffer(uint32 idx);

    /* TX direction */
    void   QR_SCANNER_PutWordInTxBuffer  (uint32 idx, uint32 txDataByte);
    uint32 QR_SCANNER_GetWordFromTxBuffer(uint32 idx);

#else
    /* RX direction */
    #if(QR_SCANNER_INTERNAL_RX_SW_BUFFER_CONST)
        #define QR_SCANNER_PutWordInRxBuffer(idx, rxDataByte) \
                do{                                                 \
                    QR_SCANNER_rxBufferInternal[(idx)] = ((uint8) (rxDataByte)); \
                }while(0)

        #define QR_SCANNER_GetWordFromRxBuffer(idx) QR_SCANNER_rxBufferInternal[(idx)]

    #endif /* (QR_SCANNER_INTERNAL_RX_SW_BUFFER_CONST) */

    /* TX direction */
    #if(QR_SCANNER_INTERNAL_TX_SW_BUFFER_CONST)
        #define QR_SCANNER_PutWordInTxBuffer(idx, txDataByte) \
                    do{                                             \
                        QR_SCANNER_txBufferInternal[(idx)] = ((uint8) (txDataByte)); \
                    }while(0)

        #define QR_SCANNER_GetWordFromTxBuffer(idx) QR_SCANNER_txBufferInternal[(idx)]

    #endif /* (QR_SCANNER_INTERNAL_TX_SW_BUFFER_CONST) */

#endif /* (QR_SCANNER_TX_SW_BUFFER_ENABLE) */


/***************************************
*         SPI API Constants
***************************************/

/* SPI sub mode enum */
#define QR_SCANNER_SPI_MODE_MOTOROLA      (0x00u)
#define QR_SCANNER_SPI_MODE_TI_COINCIDES  (0x01u)
#define QR_SCANNER_SPI_MODE_TI_PRECEDES   (0x11u)
#define QR_SCANNER_SPI_MODE_NATIONAL      (0x02u)
#define QR_SCANNER_SPI_MODE_MASK          (0x03u)
#define QR_SCANNER_SPI_MODE_TI_PRECEDES_MASK  (0x10u)
#define QR_SCANNER_SPI_MODE_NS_MICROWIRE  (QR_SCANNER_SPI_MODE_NATIONAL)

/* SPI phase and polarity mode enum */
#define QR_SCANNER_SPI_SCLK_CPHA0_CPOL0   (0x00u)
#define QR_SCANNER_SPI_SCLK_CPHA0_CPOL1   (0x02u)
#define QR_SCANNER_SPI_SCLK_CPHA1_CPOL0   (0x01u)
#define QR_SCANNER_SPI_SCLK_CPHA1_CPOL1   (0x03u)

/* SPI bits order enum */
#define QR_SCANNER_BITS_ORDER_LSB_FIRST   (0u)
#define QR_SCANNER_BITS_ORDER_MSB_FIRST   (1u)

/* SPI transfer separation enum */
#define QR_SCANNER_SPI_TRANSFER_SEPARATED     (0u)
#define QR_SCANNER_SPI_TRANSFER_CONTINUOUS    (1u)

/* SPI slave select constants */
#define QR_SCANNER_SPI_SLAVE_SELECT0    (QR_SCANNER_SCB__SS0_POSISTION)
#define QR_SCANNER_SPI_SLAVE_SELECT1    (QR_SCANNER_SCB__SS1_POSISTION)
#define QR_SCANNER_SPI_SLAVE_SELECT2    (QR_SCANNER_SCB__SS2_POSISTION)
#define QR_SCANNER_SPI_SLAVE_SELECT3    (QR_SCANNER_SCB__SS3_POSISTION)

/* SPI slave select polarity settings */
#define QR_SCANNER_SPI_SS_ACTIVE_LOW  (0u)
#define QR_SCANNER_SPI_SS_ACTIVE_HIGH (1u)

#define QR_SCANNER_INTR_SPIM_TX_RESTORE   (QR_SCANNER_INTR_TX_OVERFLOW)

#define QR_SCANNER_INTR_SPIS_TX_RESTORE     (QR_SCANNER_INTR_TX_OVERFLOW | \
                                                 QR_SCANNER_INTR_TX_UNDERFLOW)

/***************************************
*         UART API Constants
***************************************/

/* UART sub-modes enum */
#define QR_SCANNER_UART_MODE_STD          (0u)
#define QR_SCANNER_UART_MODE_SMARTCARD    (1u)
#define QR_SCANNER_UART_MODE_IRDA         (2u)

/* UART direction enum */
#define QR_SCANNER_UART_RX    (1u)
#define QR_SCANNER_UART_TX    (2u)
#define QR_SCANNER_UART_TX_RX (3u)

/* UART parity enum */
#define QR_SCANNER_UART_PARITY_EVEN   (0u)
#define QR_SCANNER_UART_PARITY_ODD    (1u)
#define QR_SCANNER_UART_PARITY_NONE   (2u)

/* UART stop bits enum */
#define QR_SCANNER_UART_STOP_BITS_1   (2u)
#define QR_SCANNER_UART_STOP_BITS_1_5 (3u)
#define QR_SCANNER_UART_STOP_BITS_2   (4u)

/* UART IrDA low power OVS enum */
#define QR_SCANNER_UART_IRDA_LP_OVS16     (16u)
#define QR_SCANNER_UART_IRDA_LP_OVS32     (32u)
#define QR_SCANNER_UART_IRDA_LP_OVS48     (48u)
#define QR_SCANNER_UART_IRDA_LP_OVS96     (96u)
#define QR_SCANNER_UART_IRDA_LP_OVS192    (192u)
#define QR_SCANNER_UART_IRDA_LP_OVS768    (768u)
#define QR_SCANNER_UART_IRDA_LP_OVS1536   (1536u)

/* Uart MP: mark (address) and space (data) bit definitions */
#define QR_SCANNER_UART_MP_MARK       (0x100u)
#define QR_SCANNER_UART_MP_SPACE      (0x000u)

/* UART CTS/RTS polarity settings */
#define QR_SCANNER_UART_CTS_ACTIVE_LOW    (0u)
#define QR_SCANNER_UART_CTS_ACTIVE_HIGH   (1u)
#define QR_SCANNER_UART_RTS_ACTIVE_LOW    (0u)
#define QR_SCANNER_UART_RTS_ACTIVE_HIGH   (1u)

/* Sources of RX errors */
#define QR_SCANNER_INTR_RX_ERR        (QR_SCANNER_INTR_RX_OVERFLOW    | \
                                             QR_SCANNER_INTR_RX_UNDERFLOW   | \
                                             QR_SCANNER_INTR_RX_FRAME_ERROR | \
                                             QR_SCANNER_INTR_RX_PARITY_ERROR)

/* Shifted INTR_RX_ERR defines ONLY for QR_SCANNER_UartGetByte() */
#define QR_SCANNER_UART_RX_OVERFLOW       (QR_SCANNER_INTR_RX_OVERFLOW << 8u)
#define QR_SCANNER_UART_RX_UNDERFLOW      (QR_SCANNER_INTR_RX_UNDERFLOW << 8u)
#define QR_SCANNER_UART_RX_FRAME_ERROR    (QR_SCANNER_INTR_RX_FRAME_ERROR << 8u)
#define QR_SCANNER_UART_RX_PARITY_ERROR   (QR_SCANNER_INTR_RX_PARITY_ERROR << 8u)
#define QR_SCANNER_UART_RX_ERROR_MASK     (QR_SCANNER_UART_RX_OVERFLOW    | \
                                                 QR_SCANNER_UART_RX_UNDERFLOW   | \
                                                 QR_SCANNER_UART_RX_FRAME_ERROR | \
                                                 QR_SCANNER_UART_RX_PARITY_ERROR)

#define QR_SCANNER_INTR_UART_TX_RESTORE   (QR_SCANNER_INTR_TX_OVERFLOW  | \
                                                 QR_SCANNER_INTR_TX_UART_NACK | \
                                                 QR_SCANNER_INTR_TX_UART_DONE | \
                                                 QR_SCANNER_INTR_TX_UART_ARB_LOST)


/***************************************
*     Vars with External Linkage
***************************************/

#if(QR_SCANNER_SCB_MODE_UNCONFIG_CONST_CFG)
    extern const QR_SCANNER_SPI_INIT_STRUCT  QR_SCANNER_configSpi;
    extern const QR_SCANNER_UART_INIT_STRUCT QR_SCANNER_configUart;
#endif /* (QR_SCANNER_SCB_MODE_UNCONFIG_CONST_CFG) */

#if (QR_SCANNER_UART_WAKE_ENABLE_CONST && QR_SCANNER_UART_RX_WAKEUP_IRQ)
    extern uint8 QR_SCANNER_skipStart;
#endif /* (QR_SCANNER_UART_WAKE_ENABLE_CONST && QR_SCANNER_UART_RX_WAKEUP_IRQ) */


/***************************************
*    Specific SPI Macro Definitions
***************************************/

#define QR_SCANNER_GET_SPI_INTR_SLAVE_MASK(sourceMask)  ((sourceMask) & QR_SCANNER_INTR_SLAVE_SPI_BUS_ERROR)
#define QR_SCANNER_GET_SPI_INTR_MASTER_MASK(sourceMask) ((sourceMask) & QR_SCANNER_INTR_MASTER_SPI_DONE)
#define QR_SCANNER_GET_SPI_INTR_RX_MASK(sourceMask) \
                                             ((sourceMask) & (uint32) ~QR_SCANNER_INTR_SLAVE_SPI_BUS_ERROR)

#define QR_SCANNER_GET_SPI_INTR_TX_MASK(sourceMask) \
                                             ((sourceMask) & (uint32) ~QR_SCANNER_INTR_MASTER_SPI_DONE)


/***************************************
*    Specific UART Macro Definitions
***************************************/

#define QR_SCANNER_UART_GET_CTRL_OVS_IRDA_LP(oversample) \
        ((QR_SCANNER_UART_IRDA_LP_OVS16   == (oversample)) ? QR_SCANNER_CTRL_OVS_IRDA_LP_OVS16 : \
         ((QR_SCANNER_UART_IRDA_LP_OVS32   == (oversample)) ? QR_SCANNER_CTRL_OVS_IRDA_LP_OVS32 : \
          ((QR_SCANNER_UART_IRDA_LP_OVS48   == (oversample)) ? QR_SCANNER_CTRL_OVS_IRDA_LP_OVS48 : \
           ((QR_SCANNER_UART_IRDA_LP_OVS96   == (oversample)) ? QR_SCANNER_CTRL_OVS_IRDA_LP_OVS96 : \
            ((QR_SCANNER_UART_IRDA_LP_OVS192  == (oversample)) ? QR_SCANNER_CTRL_OVS_IRDA_LP_OVS192 : \
             ((QR_SCANNER_UART_IRDA_LP_OVS768  == (oversample)) ? QR_SCANNER_CTRL_OVS_IRDA_LP_OVS768 : \
              ((QR_SCANNER_UART_IRDA_LP_OVS1536 == (oversample)) ? QR_SCANNER_CTRL_OVS_IRDA_LP_OVS1536 : \
                                                                          QR_SCANNER_CTRL_OVS_IRDA_LP_OVS16)))))))

#define QR_SCANNER_GET_UART_RX_CTRL_ENABLED(direction) ((0u != (QR_SCANNER_UART_RX & (direction))) ? \
                                                                     (QR_SCANNER_RX_CTRL_ENABLED) : (0u))

#define QR_SCANNER_GET_UART_TX_CTRL_ENABLED(direction) ((0u != (QR_SCANNER_UART_TX & (direction))) ? \
                                                                     (QR_SCANNER_TX_CTRL_ENABLED) : (0u))


/***************************************
*        SPI Register Settings
***************************************/

#define QR_SCANNER_CTRL_SPI      (QR_SCANNER_CTRL_MODE_SPI)
#define QR_SCANNER_SPI_RX_CTRL   (QR_SCANNER_RX_CTRL_ENABLED)
#define QR_SCANNER_SPI_TX_CTRL   (QR_SCANNER_TX_CTRL_ENABLED)


/***************************************
*       SPI Init Register Settings
***************************************/

#define QR_SCANNER_SPI_SS_POLARITY \
             (((uint32) QR_SCANNER_SPI_SS0_POLARITY << QR_SCANNER_SPI_SLAVE_SELECT0) | \
              ((uint32) QR_SCANNER_SPI_SS1_POLARITY << QR_SCANNER_SPI_SLAVE_SELECT1) | \
              ((uint32) QR_SCANNER_SPI_SS2_POLARITY << QR_SCANNER_SPI_SLAVE_SELECT2) | \
              ((uint32) QR_SCANNER_SPI_SS3_POLARITY << QR_SCANNER_SPI_SLAVE_SELECT3))

#if(QR_SCANNER_SCB_MODE_SPI_CONST_CFG)

    /* SPI Configuration */
    #define QR_SCANNER_SPI_DEFAULT_CTRL \
                    (QR_SCANNER_GET_CTRL_OVS(QR_SCANNER_SPI_OVS_FACTOR) | \
                     QR_SCANNER_GET_CTRL_BYTE_MODE (QR_SCANNER_SPI_BYTE_MODE_ENABLE) | \
                     QR_SCANNER_GET_CTRL_EC_AM_MODE(QR_SCANNER_SPI_WAKE_ENABLE)      | \
                     QR_SCANNER_CTRL_SPI)

    #define QR_SCANNER_SPI_DEFAULT_SPI_CTRL \
                    (QR_SCANNER_GET_SPI_CTRL_CONTINUOUS    (QR_SCANNER_SPI_TRANSFER_SEPARATION)       | \
                     QR_SCANNER_GET_SPI_CTRL_SELECT_PRECEDE(QR_SCANNER_SPI_SUB_MODE &                   \
                                                                  QR_SCANNER_SPI_MODE_TI_PRECEDES_MASK)     | \
                     QR_SCANNER_GET_SPI_CTRL_SCLK_MODE     (QR_SCANNER_SPI_CLOCK_MODE)                | \
                     QR_SCANNER_GET_SPI_CTRL_LATE_MISO_SAMPLE(QR_SCANNER_SPI_LATE_MISO_SAMPLE_ENABLE) | \
                     QR_SCANNER_GET_SPI_CTRL_SCLK_CONTINUOUS(QR_SCANNER_SPI_FREE_RUN_SCLK_ENABLE)     | \
                     QR_SCANNER_GET_SPI_CTRL_SSEL_POLARITY (QR_SCANNER_SPI_SS_POLARITY)               | \
                     QR_SCANNER_GET_SPI_CTRL_SUB_MODE      (QR_SCANNER_SPI_SUB_MODE)                  | \
                     QR_SCANNER_GET_SPI_CTRL_MASTER_MODE   (QR_SCANNER_SPI_MODE))

    /* RX direction */
    #define QR_SCANNER_SPI_DEFAULT_RX_CTRL \
                    (QR_SCANNER_GET_RX_CTRL_DATA_WIDTH(QR_SCANNER_SPI_RX_DATA_BITS_NUM)     | \
                     QR_SCANNER_GET_RX_CTRL_BIT_ORDER (QR_SCANNER_SPI_BITS_ORDER)           | \
                     QR_SCANNER_GET_RX_CTRL_MEDIAN    (QR_SCANNER_SPI_MEDIAN_FILTER_ENABLE) | \
                     QR_SCANNER_SPI_RX_CTRL)

    #define QR_SCANNER_SPI_DEFAULT_RX_FIFO_CTRL \
                    QR_SCANNER_GET_RX_FIFO_CTRL_TRIGGER_LEVEL(QR_SCANNER_SPI_RX_TRIGGER_LEVEL)

    /* TX direction */
    #define QR_SCANNER_SPI_DEFAULT_TX_CTRL \
                    (QR_SCANNER_GET_TX_CTRL_DATA_WIDTH(QR_SCANNER_SPI_TX_DATA_BITS_NUM) | \
                     QR_SCANNER_GET_TX_CTRL_BIT_ORDER (QR_SCANNER_SPI_BITS_ORDER)       | \
                     QR_SCANNER_SPI_TX_CTRL)

    #define QR_SCANNER_SPI_DEFAULT_TX_FIFO_CTRL \
                    QR_SCANNER_GET_TX_FIFO_CTRL_TRIGGER_LEVEL(QR_SCANNER_SPI_TX_TRIGGER_LEVEL)

    /* Interrupt sources */
    #define QR_SCANNER_SPI_DEFAULT_INTR_SPI_EC_MASK   (QR_SCANNER_NO_INTR_SOURCES)

    #define QR_SCANNER_SPI_DEFAULT_INTR_I2C_EC_MASK   (QR_SCANNER_NO_INTR_SOURCES)
    #define QR_SCANNER_SPI_DEFAULT_INTR_SLAVE_MASK \
                    (QR_SCANNER_SPI_INTR_RX_MASK & QR_SCANNER_INTR_SLAVE_SPI_BUS_ERROR)

    #define QR_SCANNER_SPI_DEFAULT_INTR_MASTER_MASK \
                    (QR_SCANNER_SPI_INTR_TX_MASK & QR_SCANNER_INTR_MASTER_SPI_DONE)

    #define QR_SCANNER_SPI_DEFAULT_INTR_RX_MASK \
                    (QR_SCANNER_SPI_INTR_RX_MASK & (uint32) ~QR_SCANNER_INTR_SLAVE_SPI_BUS_ERROR)

    #define QR_SCANNER_SPI_DEFAULT_INTR_TX_MASK \
                    (QR_SCANNER_SPI_INTR_TX_MASK & (uint32) ~QR_SCANNER_INTR_MASTER_SPI_DONE)

#endif /* (QR_SCANNER_SCB_MODE_SPI_CONST_CFG) */


/***************************************
*        UART Register Settings
***************************************/

#define QR_SCANNER_CTRL_UART      (QR_SCANNER_CTRL_MODE_UART)
#define QR_SCANNER_UART_RX_CTRL   (QR_SCANNER_RX_CTRL_LSB_FIRST) /* LSB for UART goes first */
#define QR_SCANNER_UART_TX_CTRL   (QR_SCANNER_TX_CTRL_LSB_FIRST) /* LSB for UART goes first */


/***************************************
*      UART Init Register Settings
***************************************/

#if(QR_SCANNER_SCB_MODE_UART_CONST_CFG)

    /* UART configuration */
    #if(QR_SCANNER_UART_MODE_IRDA == QR_SCANNER_UART_SUB_MODE)

        #define QR_SCANNER_DEFAULT_CTRL_OVS   ((0u != QR_SCANNER_UART_IRDA_LOW_POWER) ?              \
                                (QR_SCANNER_UART_GET_CTRL_OVS_IRDA_LP(QR_SCANNER_UART_OVS_FACTOR)) : \
                                (QR_SCANNER_CTRL_OVS_IRDA_OVS16))

    #else

        #define QR_SCANNER_DEFAULT_CTRL_OVS   QR_SCANNER_GET_CTRL_OVS(QR_SCANNER_UART_OVS_FACTOR)

    #endif /* (QR_SCANNER_UART_MODE_IRDA == QR_SCANNER_UART_SUB_MODE) */

    #define QR_SCANNER_UART_DEFAULT_CTRL \
                                (QR_SCANNER_GET_CTRL_BYTE_MODE  (QR_SCANNER_UART_BYTE_MODE_ENABLE)  | \
                                 QR_SCANNER_GET_CTRL_ADDR_ACCEPT(QR_SCANNER_UART_MP_ACCEPT_ADDRESS) | \
                                 QR_SCANNER_DEFAULT_CTRL_OVS                                              | \
                                 QR_SCANNER_CTRL_UART)

    #define QR_SCANNER_UART_DEFAULT_UART_CTRL \
                                    (QR_SCANNER_GET_UART_CTRL_MODE(QR_SCANNER_UART_SUB_MODE))

    /* RX direction */
    #define QR_SCANNER_UART_DEFAULT_RX_CTRL_PARITY \
                                ((QR_SCANNER_UART_PARITY_NONE != QR_SCANNER_UART_PARITY_TYPE) ?      \
                                  (QR_SCANNER_GET_UART_RX_CTRL_PARITY(QR_SCANNER_UART_PARITY_TYPE) | \
                                   QR_SCANNER_UART_RX_CTRL_PARITY_ENABLED) : (0u))

    #define QR_SCANNER_UART_DEFAULT_UART_RX_CTRL \
                    (QR_SCANNER_GET_UART_RX_CTRL_MODE(QR_SCANNER_UART_STOP_BITS_NUM)                    | \
                     QR_SCANNER_GET_UART_RX_CTRL_POLARITY(QR_SCANNER_UART_IRDA_POLARITY)                | \
                     QR_SCANNER_GET_UART_RX_CTRL_MP_MODE(QR_SCANNER_UART_MP_MODE_ENABLE)                | \
                     QR_SCANNER_GET_UART_RX_CTRL_DROP_ON_PARITY_ERR(QR_SCANNER_UART_DROP_ON_PARITY_ERR) | \
                     QR_SCANNER_GET_UART_RX_CTRL_DROP_ON_FRAME_ERR(QR_SCANNER_UART_DROP_ON_FRAME_ERR)   | \
                     QR_SCANNER_GET_UART_RX_CTRL_BREAK_WIDTH(QR_SCANNER_UART_RX_BREAK_WIDTH)            | \
                     QR_SCANNER_UART_DEFAULT_RX_CTRL_PARITY)


    #define QR_SCANNER_UART_DEFAULT_RX_CTRL \
                                (QR_SCANNER_GET_RX_CTRL_DATA_WIDTH(QR_SCANNER_UART_DATA_BITS_NUM)        | \
                                 QR_SCANNER_GET_RX_CTRL_MEDIAN    (QR_SCANNER_UART_MEDIAN_FILTER_ENABLE) | \
                                 QR_SCANNER_GET_UART_RX_CTRL_ENABLED(QR_SCANNER_UART_DIRECTION))

    #define QR_SCANNER_UART_DEFAULT_RX_FIFO_CTRL \
                                QR_SCANNER_GET_RX_FIFO_CTRL_TRIGGER_LEVEL(QR_SCANNER_UART_RX_TRIGGER_LEVEL)

    #define QR_SCANNER_UART_DEFAULT_RX_MATCH_REG  ((0u != QR_SCANNER_UART_MP_MODE_ENABLE) ?          \
                                (QR_SCANNER_GET_RX_MATCH_ADDR(QR_SCANNER_UART_MP_RX_ADDRESS) | \
                                 QR_SCANNER_GET_RX_MATCH_MASK(QR_SCANNER_UART_MP_RX_ADDRESS_MASK)) : (0u))

    /* TX direction */
    #define QR_SCANNER_UART_DEFAULT_TX_CTRL_PARITY (QR_SCANNER_UART_DEFAULT_RX_CTRL_PARITY)

    #define QR_SCANNER_UART_DEFAULT_UART_TX_CTRL \
                                (QR_SCANNER_GET_UART_TX_CTRL_MODE(QR_SCANNER_UART_STOP_BITS_NUM)       | \
                                 QR_SCANNER_GET_UART_TX_CTRL_RETRY_NACK(QR_SCANNER_UART_RETRY_ON_NACK) | \
                                 QR_SCANNER_UART_DEFAULT_TX_CTRL_PARITY)

    #define QR_SCANNER_UART_DEFAULT_TX_CTRL \
                                (QR_SCANNER_GET_TX_CTRL_DATA_WIDTH(QR_SCANNER_UART_DATA_BITS_NUM) | \
                                 QR_SCANNER_GET_UART_TX_CTRL_ENABLED(QR_SCANNER_UART_DIRECTION))

    #define QR_SCANNER_UART_DEFAULT_TX_FIFO_CTRL \
                                QR_SCANNER_GET_TX_FIFO_CTRL_TRIGGER_LEVEL(QR_SCANNER_UART_TX_TRIGGER_LEVEL)

    #define QR_SCANNER_UART_DEFAULT_FLOW_CTRL \
                        (QR_SCANNER_GET_UART_FLOW_CTRL_TRIGGER_LEVEL(QR_SCANNER_UART_RTS_FIFO_LEVEL) | \
                         QR_SCANNER_GET_UART_FLOW_CTRL_RTS_POLARITY (QR_SCANNER_UART_RTS_POLARITY)   | \
                         QR_SCANNER_GET_UART_FLOW_CTRL_CTS_POLARITY (QR_SCANNER_UART_CTS_POLARITY)   | \
                         QR_SCANNER_GET_UART_FLOW_CTRL_CTS_ENABLE   (QR_SCANNER_UART_CTS_ENABLE))

    /* Interrupt sources */
    #define QR_SCANNER_UART_DEFAULT_INTR_I2C_EC_MASK  (QR_SCANNER_NO_INTR_SOURCES)
    #define QR_SCANNER_UART_DEFAULT_INTR_SPI_EC_MASK  (QR_SCANNER_NO_INTR_SOURCES)
    #define QR_SCANNER_UART_DEFAULT_INTR_SLAVE_MASK   (QR_SCANNER_NO_INTR_SOURCES)
    #define QR_SCANNER_UART_DEFAULT_INTR_MASTER_MASK  (QR_SCANNER_NO_INTR_SOURCES)
    #define QR_SCANNER_UART_DEFAULT_INTR_RX_MASK      (QR_SCANNER_UART_INTR_RX_MASK)
    #define QR_SCANNER_UART_DEFAULT_INTR_TX_MASK      (QR_SCANNER_UART_INTR_TX_MASK)

#endif /* (QR_SCANNER_SCB_MODE_UART_CONST_CFG) */


/***************************************
* The following code is DEPRECATED and
* must not be used.
***************************************/

#define QR_SCANNER_SPIM_ACTIVE_SS0    (QR_SCANNER_SPI_SLAVE_SELECT0)
#define QR_SCANNER_SPIM_ACTIVE_SS1    (QR_SCANNER_SPI_SLAVE_SELECT1)
#define QR_SCANNER_SPIM_ACTIVE_SS2    (QR_SCANNER_SPI_SLAVE_SELECT2)
#define QR_SCANNER_SPIM_ACTIVE_SS3    (QR_SCANNER_SPI_SLAVE_SELECT3)

#endif /* CY_SCB_SPI_UART_QR_SCANNER_H */


/* [] END OF FILE */
