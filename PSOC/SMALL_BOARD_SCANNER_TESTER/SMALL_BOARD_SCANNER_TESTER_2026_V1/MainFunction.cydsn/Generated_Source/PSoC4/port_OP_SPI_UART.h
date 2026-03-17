/***************************************************************************//**
* \file port_OP_SPI_UART.h
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

#if !defined(CY_SCB_SPI_UART_port_OP_H)
#define CY_SCB_SPI_UART_port_OP_H

#include "port_OP.h"


/***************************************
*   SPI Initial Parameter Constants
****************************************/

#define port_OP_SPI_MODE                   (0u)
#define port_OP_SPI_SUB_MODE               (0u)
#define port_OP_SPI_CLOCK_MODE             (0u)
#define port_OP_SPI_OVS_FACTOR             (16u)
#define port_OP_SPI_MEDIAN_FILTER_ENABLE   (0u)
#define port_OP_SPI_LATE_MISO_SAMPLE_ENABLE (0u)
#define port_OP_SPI_RX_DATA_BITS_NUM       (8u)
#define port_OP_SPI_TX_DATA_BITS_NUM       (8u)
#define port_OP_SPI_WAKE_ENABLE            (0u)
#define port_OP_SPI_BITS_ORDER             (1u)
#define port_OP_SPI_TRANSFER_SEPARATION    (1u)
#define port_OP_SPI_NUMBER_OF_SS_LINES     (1u)
#define port_OP_SPI_RX_BUFFER_SIZE         (8u)
#define port_OP_SPI_TX_BUFFER_SIZE         (8u)

#define port_OP_SPI_INTERRUPT_MODE         (0u)

#define port_OP_SPI_INTR_RX_MASK           (0x0u)
#define port_OP_SPI_INTR_TX_MASK           (0x0u)

#define port_OP_SPI_RX_TRIGGER_LEVEL       (7u)
#define port_OP_SPI_TX_TRIGGER_LEVEL       (0u)

#define port_OP_SPI_BYTE_MODE_ENABLE       (0u)
#define port_OP_SPI_FREE_RUN_SCLK_ENABLE   (0u)
#define port_OP_SPI_SS0_POLARITY           (0u)
#define port_OP_SPI_SS1_POLARITY           (0u)
#define port_OP_SPI_SS2_POLARITY           (0u)
#define port_OP_SPI_SS3_POLARITY           (0u)


/***************************************
*   UART Initial Parameter Constants
****************************************/

#define port_OP_UART_SUB_MODE              (0u)
#define port_OP_UART_DIRECTION             (3u)
#define port_OP_UART_DATA_BITS_NUM         (8u)
#define port_OP_UART_PARITY_TYPE           (2u)
#define port_OP_UART_STOP_BITS_NUM         (2u)
#define port_OP_UART_OVS_FACTOR            (12u)
#define port_OP_UART_IRDA_LOW_POWER        (0u)
#define port_OP_UART_MEDIAN_FILTER_ENABLE  (0u)
#define port_OP_UART_RETRY_ON_NACK         (0u)
#define port_OP_UART_IRDA_POLARITY         (0u)
#define port_OP_UART_DROP_ON_FRAME_ERR     (0u)
#define port_OP_UART_DROP_ON_PARITY_ERR    (0u)
#define port_OP_UART_WAKE_ENABLE           (0u)
#define port_OP_UART_RX_BUFFER_SIZE        (8u)
#define port_OP_UART_TX_BUFFER_SIZE        (8u)
#define port_OP_UART_MP_MODE_ENABLE        (0u)
#define port_OP_UART_MP_ACCEPT_ADDRESS     (0u)
#define port_OP_UART_MP_RX_ADDRESS         (0x2u)
#define port_OP_UART_MP_RX_ADDRESS_MASK    (0xFFu)

#define port_OP_UART_INTERRUPT_MODE        (0u)

#define port_OP_UART_INTR_RX_MASK          (0x0u)
#define port_OP_UART_INTR_TX_MASK          (0x0u)

#define port_OP_UART_RX_TRIGGER_LEVEL      (7u)
#define port_OP_UART_TX_TRIGGER_LEVEL      (0u)

#define port_OP_UART_BYTE_MODE_ENABLE      (0u)
#define port_OP_UART_CTS_ENABLE            (0u)
#define port_OP_UART_CTS_POLARITY          (0u)
#define port_OP_UART_RTS_ENABLE            (0u)
#define port_OP_UART_RTS_POLARITY          (0u)
#define port_OP_UART_RTS_FIFO_LEVEL        (4u)

#define port_OP_UART_RX_BREAK_WIDTH        (11u)

/* SPI mode enum */
#define port_OP_SPI_SLAVE  (0u)
#define port_OP_SPI_MASTER (1u)

/* UART direction enum */
#define port_OP_UART_RX    (1u)
#define port_OP_UART_TX    (2u)
#define port_OP_UART_TX_RX (3u)


/***************************************
*   Conditional Compilation Parameters
****************************************/

#if(port_OP_SCB_MODE_UNCONFIG_CONST_CFG)

    /* Mode */
    #define port_OP_SPI_SLAVE_CONST        (1u)
    #define port_OP_SPI_MASTER_CONST       (1u)

    /* Direction */
    #define port_OP_RX_DIRECTION           (1u)
    #define port_OP_TX_DIRECTION           (1u)
    #define port_OP_UART_RX_DIRECTION      (1u)
    #define port_OP_UART_TX_DIRECTION      (1u)

    /* Only external RX and TX buffer for Uncofigured mode */
    #define port_OP_INTERNAL_RX_SW_BUFFER   (0u)
    #define port_OP_INTERNAL_TX_SW_BUFFER   (0u)

    /* Get RX and TX buffer size */
    #define port_OP_INTERNAL_RX_BUFFER_SIZE    (port_OP_rxBufferSize + 1u)
    #define port_OP_RX_BUFFER_SIZE             (port_OP_rxBufferSize)
    #define port_OP_TX_BUFFER_SIZE             (port_OP_txBufferSize)

    /* Return true if buffer is provided */
    #define port_OP_CHECK_RX_SW_BUFFER (NULL != port_OP_rxBuffer)
    #define port_OP_CHECK_TX_SW_BUFFER (NULL != port_OP_txBuffer)

    /* Always provide global variables to support RX and TX buffers */
    #define port_OP_INTERNAL_RX_SW_BUFFER_CONST    (1u)
    #define port_OP_INTERNAL_TX_SW_BUFFER_CONST    (1u)

    /* Get wakeup enable option */
    #define port_OP_SPI_WAKE_ENABLE_CONST  (1u)
    #define port_OP_UART_WAKE_ENABLE_CONST (1u)
    #define port_OP_CHECK_SPI_WAKE_ENABLE  ((0u != port_OP_scbEnableWake) && port_OP_SCB_MODE_SPI_RUNTM_CFG)
    #define port_OP_CHECK_UART_WAKE_ENABLE ((0u != port_OP_scbEnableWake) && port_OP_SCB_MODE_UART_RUNTM_CFG)

    /* SPI/UART: TX or RX FIFO size */
    #if (port_OP_CY_SCBIP_V0 || port_OP_CY_SCBIP_V1)
        #define port_OP_SPI_UART_FIFO_SIZE             (port_OP_FIFO_SIZE)
        #define port_OP_CHECK_UART_RTS_CONTROL_FLOW    (0u)
    #else
        #define port_OP_SPI_UART_FIFO_SIZE (port_OP_GET_FIFO_SIZE(port_OP_CTRL_REG & \
                                                                                    port_OP_CTRL_BYTE_MODE))

        #define port_OP_CHECK_UART_RTS_CONTROL_FLOW \
                    ((port_OP_SCB_MODE_UART_RUNTM_CFG) && \
                     (0u != port_OP_GET_UART_FLOW_CTRL_TRIGGER_LEVEL(port_OP_UART_FLOW_CTRL_REG)))
    #endif /* (port_OP_CY_SCBIP_V0 || port_OP_CY_SCBIP_V1) */

#else

    /* Internal RX and TX buffer: for SPI or UART */
    #if (port_OP_SCB_MODE_SPI_CONST_CFG)

        /* SPI Direction */
        #define port_OP_SPI_RX_DIRECTION (1u)
        #define port_OP_SPI_TX_DIRECTION (1u)

        /* Get FIFO size */
        #define port_OP_SPI_UART_FIFO_SIZE port_OP_GET_FIFO_SIZE(port_OP_SPI_BYTE_MODE_ENABLE)

        /* SPI internal RX and TX buffers */
        #define port_OP_INTERNAL_SPI_RX_SW_BUFFER  (port_OP_SPI_RX_BUFFER_SIZE > \
                                                                port_OP_SPI_UART_FIFO_SIZE)
        #define port_OP_INTERNAL_SPI_TX_SW_BUFFER  (port_OP_SPI_TX_BUFFER_SIZE > \
                                                                port_OP_SPI_UART_FIFO_SIZE)

        /* Internal SPI RX and TX buffer */
        #define port_OP_INTERNAL_RX_SW_BUFFER  (port_OP_INTERNAL_SPI_RX_SW_BUFFER)
        #define port_OP_INTERNAL_TX_SW_BUFFER  (port_OP_INTERNAL_SPI_TX_SW_BUFFER)

        /* Internal SPI RX and TX buffer size */
        #define port_OP_INTERNAL_RX_BUFFER_SIZE    (port_OP_SPI_RX_BUFFER_SIZE + 1u)
        #define port_OP_RX_BUFFER_SIZE             (port_OP_SPI_RX_BUFFER_SIZE)
        #define port_OP_TX_BUFFER_SIZE             (port_OP_SPI_TX_BUFFER_SIZE)

        /* Get wakeup enable option */
        #define port_OP_SPI_WAKE_ENABLE_CONST  (0u != port_OP_SPI_WAKE_ENABLE)
        #define port_OP_UART_WAKE_ENABLE_CONST (0u)

    #else

        /* UART Direction */
        #define port_OP_UART_RX_DIRECTION (0u != (port_OP_UART_DIRECTION & port_OP_UART_RX))
        #define port_OP_UART_TX_DIRECTION (0u != (port_OP_UART_DIRECTION & port_OP_UART_TX))

        /* Get FIFO size */
        #define port_OP_SPI_UART_FIFO_SIZE port_OP_GET_FIFO_SIZE(port_OP_UART_BYTE_MODE_ENABLE)

        /* UART internal RX and TX buffers */
        #define port_OP_INTERNAL_UART_RX_SW_BUFFER  (port_OP_UART_RX_BUFFER_SIZE > \
                                                                port_OP_SPI_UART_FIFO_SIZE)
        #define port_OP_INTERNAL_UART_TX_SW_BUFFER  (port_OP_UART_TX_BUFFER_SIZE > \
                                                                    port_OP_SPI_UART_FIFO_SIZE)

        /* Internal UART RX and TX buffer */
        #define port_OP_INTERNAL_RX_SW_BUFFER  (port_OP_INTERNAL_UART_RX_SW_BUFFER)
        #define port_OP_INTERNAL_TX_SW_BUFFER  (port_OP_INTERNAL_UART_TX_SW_BUFFER)

        /* Internal UART RX and TX buffer size */
        #define port_OP_INTERNAL_RX_BUFFER_SIZE    (port_OP_UART_RX_BUFFER_SIZE + 1u)
        #define port_OP_RX_BUFFER_SIZE             (port_OP_UART_RX_BUFFER_SIZE)
        #define port_OP_TX_BUFFER_SIZE             (port_OP_UART_TX_BUFFER_SIZE)

        /* Get wakeup enable option */
        #define port_OP_SPI_WAKE_ENABLE_CONST  (0u)
        #define port_OP_UART_WAKE_ENABLE_CONST (0u != port_OP_UART_WAKE_ENABLE)

    #endif /* (port_OP_SCB_MODE_SPI_CONST_CFG) */

    /* Mode */
    #define port_OP_SPI_SLAVE_CONST    (port_OP_SPI_MODE == port_OP_SPI_SLAVE)
    #define port_OP_SPI_MASTER_CONST   (port_OP_SPI_MODE == port_OP_SPI_MASTER)

    /* Direction */
    #define port_OP_RX_DIRECTION ((port_OP_SCB_MODE_SPI_CONST_CFG) ? \
                                            (port_OP_SPI_RX_DIRECTION) : (port_OP_UART_RX_DIRECTION))

    #define port_OP_TX_DIRECTION ((port_OP_SCB_MODE_SPI_CONST_CFG) ? \
                                            (port_OP_SPI_TX_DIRECTION) : (port_OP_UART_TX_DIRECTION))

    /* Internal RX and TX buffer: for SPI or UART. Used in conditional compilation check */
    #define port_OP_CHECK_RX_SW_BUFFER (port_OP_INTERNAL_RX_SW_BUFFER)
    #define port_OP_CHECK_TX_SW_BUFFER (port_OP_INTERNAL_TX_SW_BUFFER)

    /* Provide global variables to support RX and TX buffers */
    #define port_OP_INTERNAL_RX_SW_BUFFER_CONST    (port_OP_INTERNAL_RX_SW_BUFFER)
    #define port_OP_INTERNAL_TX_SW_BUFFER_CONST    (port_OP_INTERNAL_TX_SW_BUFFER)

    /* Wake up enable */
    #define port_OP_CHECK_SPI_WAKE_ENABLE  (port_OP_SPI_WAKE_ENABLE_CONST)
    #define port_OP_CHECK_UART_WAKE_ENABLE (port_OP_UART_WAKE_ENABLE_CONST)

    /* UART flow control: not applicable for CY_SCBIP_V0 || CY_SCBIP_V1 */
    #define port_OP_CHECK_UART_RTS_CONTROL_FLOW    (port_OP_SCB_MODE_UART_CONST_CFG && \
                                                             port_OP_UART_RTS_ENABLE)

#endif /* End (port_OP_SCB_MODE_UNCONFIG_CONST_CFG) */


/***************************************
*       Type Definitions
***************************************/

/**
* \addtogroup group_structures
* @{
*/

/* port_OP_SPI_INIT_STRUCT */
typedef struct
{
    /** Mode of operation for SPI. The following defines are available choices:
     *  - port_OP_SPI_SLAVE
     *  - port_OP_SPI_MASTE
    */
    uint32 mode;

    /** Submode of operation for SPI. The following defines are available
     *  choices:
     *  - port_OP_SPI_MODE_MOTOROLA
     *  - port_OP_SPI_MODE_TI_COINCIDES
     *  - port_OP_SPI_MODE_TI_PRECEDES
     *  - port_OP_SPI_MODE_NATIONAL
    */
    uint32 submode;

    /** Determines the sclk relationship for Motorola submode. Ignored
     *  for other submodes. The following defines are available choices:
     *  - port_OP_SPI_SCLK_CPHA0_CPOL0
     *  - port_OP_SPI_SCLK_CPHA0_CPOL1
     *  - port_OP_SPI_SCLK_CPHA1_CPOL0
     *  - port_OP_SPI_SCLK_CPHA1_CPOL1
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
     *  - port_OP_BITS_ORDER_LSB_FIRST
     *  - port_OP_BITS_ORDER_MSB_FIRST
    */
    uint32 bitOrder;

    /** Determines whether transfers are back to back or have SS disabled
     *  between words. Ignored for slave mode. The following defines are
     *  available choices:
     *  - port_OP_SPI_TRANSFER_CONTINUOUS
     *  - port_OP_SPI_TRANSFER_SEPARATED
    */
    uint32 transferSeperation;

    /** Size of the RX buffer in bytes/words (depends on rxDataBits parameter).
     *  A value equal to the RX FIFO depth implies the usage of buffering in
     *  hardware. A value greater than the RX FIFO depth results in a software
     *  buffer.
     *  The port_OP_INTR _RX_NOT_EMPTY interrupt has to be enabled to
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
     *  - port_OP_INTR_RX_FIFO_LEVEL
     *  - port_OP_INTR_RX_NOT_EMPTY
     *  - port_OP_INTR_RX_FULL
     *  - port_OP_INTR_RX_OVERFLOW
     *  - port_OP_INTR_RX_UNDERFLOW
     *  - port_OP_INTR_SLAVE_SPI_BUS_ERROR
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
     *  - port_OP_INTR_TX_FIFO_LEVEL
     *  - port_OP_INTR_TX_NOT_FULL
     *  - port_OP_INTR_TX_EMPTY
     *  - port_OP_INTR_TX_OVERFLOW
     *  - port_OP_INTR_TX_UNDERFLOW
     *  - port_OP_INTR_MASTER_SPI_DONE
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
     *  port_OP_SPI_SLAVE_SELECT0 corresponds to slave select 0
     *  polarity, bit port_OP_SPI_SLAVE_SELECT1 – slave select 1
     *  polarity and so on. Polarity constants are:
     *  - port_OP_SPI_SS_ACTIVE_LOW
     *  - port_OP_SPI_SS_ACTIVE_HIGH
     *
     *  Ignored for all devices other than PSoC 4100 BLE / PSoC 4200 BLE /
     *  PSoC 4100M / PSoC 4200M / PSoC 4200L / PSoC 4000S / PSoC 4100S /
     *  PSoC Analog Coprocessor.
    */
    uint8 polaritySs;
} port_OP_SPI_INIT_STRUCT;


/* port_OP_UART_INIT_STRUCT */
typedef struct
{
    /** Mode of operation for the UART. The following defines are available
     *  choices:
     *  - port_OP_UART_MODE_STD
     *  - port_OP_UART_MODE_SMARTCARD
     *  - port_OP_UART_MODE_IRDA
    */
    uint32 mode;

    /** Direction of operation for the UART. The following defines are available
     *  choices:
     *  - port_OP_UART_TX_RX
     *  - port_OP_UART_RX
     *  - port_OP_UART_TX
    */
    uint32 direction;

    /** Number of data bits.
    */
    uint32 dataBits;

    /** Determines the parity. The following defines are available choices:
     *  - port_OP_UART_PARITY_EVEN
     *  - port_OP_UART_PARITY_ODD
     *  - port_OP_UART_PARITY_NONE
    */
    uint32 parity;

    /** Determines the number of stop bits. The following defines are available
     *  choices:
     *  - port_OP_UART_STOP_BITS_1
     *  - port_OP_UART_STOP_BITS_1_5
     *  - port_OP_UART_STOP_BITS_2
    */
    uint32 stopBits;

    /** Oversampling factor for the UART.
     *
     *  Note The oversampling factor values are changed when enableIrdaLowPower
     *  is enabled:
     *  - port_OP_UART_IRDA_LP_OVS16
     *  - port_OP_UART_IRDA_LP_OVS32
     *  - port_OP_UART_IRDA_LP_OVS48
     *  - port_OP_UART_IRDA_LP_OVS96
     *  - port_OP_UART_IRDA_LP_OVS192
     *  - port_OP_UART_IRDA_LP_OVS768
     *  - port_OP_UART_IRDA_LP_OVS1536
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
     *  The port_OP_INTR _RX_NOT_EMPTY interrupt has to be enabled to
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
     *  - port_OP_INTR_RX_FIFO_LEVEL
     *  - port_OP_INTR_RX_NOT_EMPTY
     *  - port_OP_INTR_RX_FULL
     *  - port_OP_INTR_RX_OVERFLOW
     *  - port_OP_INTR_RX_UNDERFLOW
     *  - port_OP_INTR_RX_FRAME_ERROR
     *  - port_OP_INTR_RX_PARITY_ERROR
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
     *  - port_OP_INTR_TX_FIFO_LEVEL
     *  - port_OP_INTR_TX_NOT_FULL
     *  - port_OP_INTR_TX_EMPTY
     *  - port_OP_INTR_TX_OVERFLOW
     *  - port_OP_INTR_TX_UNDERFLOW
     *  - port_OP_INTR_TX_UART_DONE
     *  - port_OP_INTR_TX_UART_NACK
     *  - port_OP_INTR_TX_UART_ARB_LOST
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
     *  - port_OP_UART_CTS_ACTIVE_LOW
     *  - port_OP_UART_CTS_ACTIVE_HIGH
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
     *  - port_OP_UART_RTS_ ACTIVE_LOW
     *  - port_OP_UART_RTS_ACTIVE_HIGH
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
} port_OP_UART_INIT_STRUCT;

/** @} structures */


/***************************************
*        Function Prototypes
***************************************/

/**
* \addtogroup group_spi
* @{
*/
/* SPI specific functions */
#if(port_OP_SCB_MODE_UNCONFIG_CONST_CFG)
    void port_OP_SpiInit(const port_OP_SPI_INIT_STRUCT *config);
#endif /* (port_OP_SCB_MODE_UNCONFIG_CONST_CFG) */


#if(port_OP_SCB_MODE_SPI_INC)
    /*******************************************************************************
    * Function Name: port_OP_SpiIsBusBusy
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
    #define port_OP_SpiIsBusBusy() ((uint32) (0u != (port_OP_SPI_STATUS_REG & \
                                                              port_OP_SPI_STATUS_BUS_BUSY)))

    #if (port_OP_SPI_MASTER_CONST)
        void port_OP_SpiSetActiveSlaveSelect(uint32 slaveSelect);
    #endif /*(port_OP_SPI_MASTER_CONST) */

    #if !(port_OP_CY_SCBIP_V0 || port_OP_CY_SCBIP_V1)
        void port_OP_SpiSetSlaveSelectPolarity(uint32 slaveSelect, uint32 polarity);
    #endif /* !(port_OP_CY_SCBIP_V0 || port_OP_CY_SCBIP_V1) */
#endif /* (port_OP_SCB_MODE_SPI_INC) */
/** @} spi */

/**
* \addtogroup group_uart
* @{
*/
/* UART specific functions */
#if(port_OP_SCB_MODE_UNCONFIG_CONST_CFG)
    void port_OP_UartInit(const port_OP_UART_INIT_STRUCT *config);
#endif /* (port_OP_SCB_MODE_UNCONFIG_CONST_CFG) */

#if(port_OP_SCB_MODE_UART_INC)
    void port_OP_UartSetRxAddress(uint32 address);
    void port_OP_UartSetRxAddressMask(uint32 addressMask);


    /* UART RX direction APIs */
    #if(port_OP_UART_RX_DIRECTION)
        uint32 port_OP_UartGetChar(void);
        uint32 port_OP_UartGetByte(void);

        #if !(port_OP_CY_SCBIP_V0 || port_OP_CY_SCBIP_V1)
            /* UART APIs for Flow Control */
            void port_OP_UartSetRtsPolarity(uint32 polarity);
            void port_OP_UartSetRtsFifoLevel(uint32 level);
        #endif /* !(port_OP_CY_SCBIP_V0 || port_OP_CY_SCBIP_V1) */
    #endif /* (port_OP_UART_RX_DIRECTION) */

    /* UART TX direction APIs */
    #if(port_OP_UART_TX_DIRECTION)
        /*******************************************************************************
        * Function Name: port_OP_UartPutChar
        ****************************************************************************//**
        *
        *  Places a byte of data in the transmit buffer to be sent at the next available
        *  bus time. This function is blocking and waits until there is a space
        *  available to put requested data in the transmit buffer.
        *  For UART Multi Processor mode this function can send 9-bits data as well.
        *  Use port_OP_UART_MP_MARK to add a mark to create an address byte.
        *
        *  \param txDataByte: the data to be transmitted.
        *
        *******************************************************************************/
        #define port_OP_UartPutChar(ch)    port_OP_SpiUartWriteTxData((uint32)(ch))

        void port_OP_UartPutString(const char8 string[]);
        void port_OP_UartPutCRLF(uint32 txDataByte);
        void port_OP_UartSendBreakBlocking(uint32 breakWidth);

        #if !(port_OP_CY_SCBIP_V0 || port_OP_CY_SCBIP_V1)
            /* UART APIs for Flow Control */
            void port_OP_UartEnableCts(void);
            void port_OP_UartDisableCts(void);
            void port_OP_UartSetCtsPolarity(uint32 polarity);
        #endif /* !(port_OP_CY_SCBIP_V0 || port_OP_CY_SCBIP_V1) */
    #endif /* (port_OP_UART_TX_DIRECTION) */
#endif /* (port_OP_SCB_MODE_UART_INC) */
/** @} uart */

/**
* \addtogroup group_spi_uart
* @{
*/
#if(port_OP_RX_DIRECTION)
    uint32 port_OP_SpiUartReadRxData(void);
    uint32 port_OP_SpiUartGetRxBufferSize(void);
    void   port_OP_SpiUartClearRxBuffer(void);
#endif /* (port_OP_RX_DIRECTION) */

/* Common APIs TX direction */
#if(port_OP_TX_DIRECTION)
    void   port_OP_SpiUartWriteTxData(uint32 txData);
    void   port_OP_SpiUartPutArray(const uint8 wrBuf[], uint32 count);
    uint32 port_OP_SpiUartGetTxBufferSize(void);
    void   port_OP_SpiUartClearTxBuffer(void);
#endif /* (port_OP_TX_DIRECTION) */
/** @} spi_uart */

CY_ISR_PROTO(port_OP_SPI_UART_ISR);

#if(port_OP_UART_RX_WAKEUP_IRQ)
    CY_ISR_PROTO(port_OP_UART_WAKEUP_ISR);
#endif /* (port_OP_UART_RX_WAKEUP_IRQ) */


/***************************************
*     Buffer Access Macro Definitions
***************************************/

#if(port_OP_SCB_MODE_UNCONFIG_CONST_CFG)
    /* RX direction */
    void   port_OP_PutWordInRxBuffer  (uint32 idx, uint32 rxDataByte);
    uint32 port_OP_GetWordFromRxBuffer(uint32 idx);

    /* TX direction */
    void   port_OP_PutWordInTxBuffer  (uint32 idx, uint32 txDataByte);
    uint32 port_OP_GetWordFromTxBuffer(uint32 idx);

#else
    /* RX direction */
    #if(port_OP_INTERNAL_RX_SW_BUFFER_CONST)
        #define port_OP_PutWordInRxBuffer(idx, rxDataByte) \
                do{                                                 \
                    port_OP_rxBufferInternal[(idx)] = ((uint8) (rxDataByte)); \
                }while(0)

        #define port_OP_GetWordFromRxBuffer(idx) port_OP_rxBufferInternal[(idx)]

    #endif /* (port_OP_INTERNAL_RX_SW_BUFFER_CONST) */

    /* TX direction */
    #if(port_OP_INTERNAL_TX_SW_BUFFER_CONST)
        #define port_OP_PutWordInTxBuffer(idx, txDataByte) \
                    do{                                             \
                        port_OP_txBufferInternal[(idx)] = ((uint8) (txDataByte)); \
                    }while(0)

        #define port_OP_GetWordFromTxBuffer(idx) port_OP_txBufferInternal[(idx)]

    #endif /* (port_OP_INTERNAL_TX_SW_BUFFER_CONST) */

#endif /* (port_OP_TX_SW_BUFFER_ENABLE) */


/***************************************
*         SPI API Constants
***************************************/

/* SPI sub mode enum */
#define port_OP_SPI_MODE_MOTOROLA      (0x00u)
#define port_OP_SPI_MODE_TI_COINCIDES  (0x01u)
#define port_OP_SPI_MODE_TI_PRECEDES   (0x11u)
#define port_OP_SPI_MODE_NATIONAL      (0x02u)
#define port_OP_SPI_MODE_MASK          (0x03u)
#define port_OP_SPI_MODE_TI_PRECEDES_MASK  (0x10u)
#define port_OP_SPI_MODE_NS_MICROWIRE  (port_OP_SPI_MODE_NATIONAL)

/* SPI phase and polarity mode enum */
#define port_OP_SPI_SCLK_CPHA0_CPOL0   (0x00u)
#define port_OP_SPI_SCLK_CPHA0_CPOL1   (0x02u)
#define port_OP_SPI_SCLK_CPHA1_CPOL0   (0x01u)
#define port_OP_SPI_SCLK_CPHA1_CPOL1   (0x03u)

/* SPI bits order enum */
#define port_OP_BITS_ORDER_LSB_FIRST   (0u)
#define port_OP_BITS_ORDER_MSB_FIRST   (1u)

/* SPI transfer separation enum */
#define port_OP_SPI_TRANSFER_SEPARATED     (0u)
#define port_OP_SPI_TRANSFER_CONTINUOUS    (1u)

/* SPI slave select constants */
#define port_OP_SPI_SLAVE_SELECT0    (port_OP_SCB__SS0_POSISTION)
#define port_OP_SPI_SLAVE_SELECT1    (port_OP_SCB__SS1_POSISTION)
#define port_OP_SPI_SLAVE_SELECT2    (port_OP_SCB__SS2_POSISTION)
#define port_OP_SPI_SLAVE_SELECT3    (port_OP_SCB__SS3_POSISTION)

/* SPI slave select polarity settings */
#define port_OP_SPI_SS_ACTIVE_LOW  (0u)
#define port_OP_SPI_SS_ACTIVE_HIGH (1u)

#define port_OP_INTR_SPIM_TX_RESTORE   (port_OP_INTR_TX_OVERFLOW)

#define port_OP_INTR_SPIS_TX_RESTORE     (port_OP_INTR_TX_OVERFLOW | \
                                                 port_OP_INTR_TX_UNDERFLOW)

/***************************************
*         UART API Constants
***************************************/

/* UART sub-modes enum */
#define port_OP_UART_MODE_STD          (0u)
#define port_OP_UART_MODE_SMARTCARD    (1u)
#define port_OP_UART_MODE_IRDA         (2u)

/* UART direction enum */
#define port_OP_UART_RX    (1u)
#define port_OP_UART_TX    (2u)
#define port_OP_UART_TX_RX (3u)

/* UART parity enum */
#define port_OP_UART_PARITY_EVEN   (0u)
#define port_OP_UART_PARITY_ODD    (1u)
#define port_OP_UART_PARITY_NONE   (2u)

/* UART stop bits enum */
#define port_OP_UART_STOP_BITS_1   (2u)
#define port_OP_UART_STOP_BITS_1_5 (3u)
#define port_OP_UART_STOP_BITS_2   (4u)

/* UART IrDA low power OVS enum */
#define port_OP_UART_IRDA_LP_OVS16     (16u)
#define port_OP_UART_IRDA_LP_OVS32     (32u)
#define port_OP_UART_IRDA_LP_OVS48     (48u)
#define port_OP_UART_IRDA_LP_OVS96     (96u)
#define port_OP_UART_IRDA_LP_OVS192    (192u)
#define port_OP_UART_IRDA_LP_OVS768    (768u)
#define port_OP_UART_IRDA_LP_OVS1536   (1536u)

/* Uart MP: mark (address) and space (data) bit definitions */
#define port_OP_UART_MP_MARK       (0x100u)
#define port_OP_UART_MP_SPACE      (0x000u)

/* UART CTS/RTS polarity settings */
#define port_OP_UART_CTS_ACTIVE_LOW    (0u)
#define port_OP_UART_CTS_ACTIVE_HIGH   (1u)
#define port_OP_UART_RTS_ACTIVE_LOW    (0u)
#define port_OP_UART_RTS_ACTIVE_HIGH   (1u)

/* Sources of RX errors */
#define port_OP_INTR_RX_ERR        (port_OP_INTR_RX_OVERFLOW    | \
                                             port_OP_INTR_RX_UNDERFLOW   | \
                                             port_OP_INTR_RX_FRAME_ERROR | \
                                             port_OP_INTR_RX_PARITY_ERROR)

/* Shifted INTR_RX_ERR defines ONLY for port_OP_UartGetByte() */
#define port_OP_UART_RX_OVERFLOW       (port_OP_INTR_RX_OVERFLOW << 8u)
#define port_OP_UART_RX_UNDERFLOW      (port_OP_INTR_RX_UNDERFLOW << 8u)
#define port_OP_UART_RX_FRAME_ERROR    (port_OP_INTR_RX_FRAME_ERROR << 8u)
#define port_OP_UART_RX_PARITY_ERROR   (port_OP_INTR_RX_PARITY_ERROR << 8u)
#define port_OP_UART_RX_ERROR_MASK     (port_OP_UART_RX_OVERFLOW    | \
                                                 port_OP_UART_RX_UNDERFLOW   | \
                                                 port_OP_UART_RX_FRAME_ERROR | \
                                                 port_OP_UART_RX_PARITY_ERROR)

#define port_OP_INTR_UART_TX_RESTORE   (port_OP_INTR_TX_OVERFLOW  | \
                                                 port_OP_INTR_TX_UART_NACK | \
                                                 port_OP_INTR_TX_UART_DONE | \
                                                 port_OP_INTR_TX_UART_ARB_LOST)


/***************************************
*     Vars with External Linkage
***************************************/

#if(port_OP_SCB_MODE_UNCONFIG_CONST_CFG)
    extern const port_OP_SPI_INIT_STRUCT  port_OP_configSpi;
    extern const port_OP_UART_INIT_STRUCT port_OP_configUart;
#endif /* (port_OP_SCB_MODE_UNCONFIG_CONST_CFG) */

#if (port_OP_UART_WAKE_ENABLE_CONST && port_OP_UART_RX_WAKEUP_IRQ)
    extern uint8 port_OP_skipStart;
#endif /* (port_OP_UART_WAKE_ENABLE_CONST && port_OP_UART_RX_WAKEUP_IRQ) */


/***************************************
*    Specific SPI Macro Definitions
***************************************/

#define port_OP_GET_SPI_INTR_SLAVE_MASK(sourceMask)  ((sourceMask) & port_OP_INTR_SLAVE_SPI_BUS_ERROR)
#define port_OP_GET_SPI_INTR_MASTER_MASK(sourceMask) ((sourceMask) & port_OP_INTR_MASTER_SPI_DONE)
#define port_OP_GET_SPI_INTR_RX_MASK(sourceMask) \
                                             ((sourceMask) & (uint32) ~port_OP_INTR_SLAVE_SPI_BUS_ERROR)

#define port_OP_GET_SPI_INTR_TX_MASK(sourceMask) \
                                             ((sourceMask) & (uint32) ~port_OP_INTR_MASTER_SPI_DONE)


/***************************************
*    Specific UART Macro Definitions
***************************************/

#define port_OP_UART_GET_CTRL_OVS_IRDA_LP(oversample) \
        ((port_OP_UART_IRDA_LP_OVS16   == (oversample)) ? port_OP_CTRL_OVS_IRDA_LP_OVS16 : \
         ((port_OP_UART_IRDA_LP_OVS32   == (oversample)) ? port_OP_CTRL_OVS_IRDA_LP_OVS32 : \
          ((port_OP_UART_IRDA_LP_OVS48   == (oversample)) ? port_OP_CTRL_OVS_IRDA_LP_OVS48 : \
           ((port_OP_UART_IRDA_LP_OVS96   == (oversample)) ? port_OP_CTRL_OVS_IRDA_LP_OVS96 : \
            ((port_OP_UART_IRDA_LP_OVS192  == (oversample)) ? port_OP_CTRL_OVS_IRDA_LP_OVS192 : \
             ((port_OP_UART_IRDA_LP_OVS768  == (oversample)) ? port_OP_CTRL_OVS_IRDA_LP_OVS768 : \
              ((port_OP_UART_IRDA_LP_OVS1536 == (oversample)) ? port_OP_CTRL_OVS_IRDA_LP_OVS1536 : \
                                                                          port_OP_CTRL_OVS_IRDA_LP_OVS16)))))))

#define port_OP_GET_UART_RX_CTRL_ENABLED(direction) ((0u != (port_OP_UART_RX & (direction))) ? \
                                                                     (port_OP_RX_CTRL_ENABLED) : (0u))

#define port_OP_GET_UART_TX_CTRL_ENABLED(direction) ((0u != (port_OP_UART_TX & (direction))) ? \
                                                                     (port_OP_TX_CTRL_ENABLED) : (0u))


/***************************************
*        SPI Register Settings
***************************************/

#define port_OP_CTRL_SPI      (port_OP_CTRL_MODE_SPI)
#define port_OP_SPI_RX_CTRL   (port_OP_RX_CTRL_ENABLED)
#define port_OP_SPI_TX_CTRL   (port_OP_TX_CTRL_ENABLED)


/***************************************
*       SPI Init Register Settings
***************************************/

#define port_OP_SPI_SS_POLARITY \
             (((uint32) port_OP_SPI_SS0_POLARITY << port_OP_SPI_SLAVE_SELECT0) | \
              ((uint32) port_OP_SPI_SS1_POLARITY << port_OP_SPI_SLAVE_SELECT1) | \
              ((uint32) port_OP_SPI_SS2_POLARITY << port_OP_SPI_SLAVE_SELECT2) | \
              ((uint32) port_OP_SPI_SS3_POLARITY << port_OP_SPI_SLAVE_SELECT3))

#if(port_OP_SCB_MODE_SPI_CONST_CFG)

    /* SPI Configuration */
    #define port_OP_SPI_DEFAULT_CTRL \
                    (port_OP_GET_CTRL_OVS(port_OP_SPI_OVS_FACTOR) | \
                     port_OP_GET_CTRL_BYTE_MODE (port_OP_SPI_BYTE_MODE_ENABLE) | \
                     port_OP_GET_CTRL_EC_AM_MODE(port_OP_SPI_WAKE_ENABLE)      | \
                     port_OP_CTRL_SPI)

    #define port_OP_SPI_DEFAULT_SPI_CTRL \
                    (port_OP_GET_SPI_CTRL_CONTINUOUS    (port_OP_SPI_TRANSFER_SEPARATION)       | \
                     port_OP_GET_SPI_CTRL_SELECT_PRECEDE(port_OP_SPI_SUB_MODE &                   \
                                                                  port_OP_SPI_MODE_TI_PRECEDES_MASK)     | \
                     port_OP_GET_SPI_CTRL_SCLK_MODE     (port_OP_SPI_CLOCK_MODE)                | \
                     port_OP_GET_SPI_CTRL_LATE_MISO_SAMPLE(port_OP_SPI_LATE_MISO_SAMPLE_ENABLE) | \
                     port_OP_GET_SPI_CTRL_SCLK_CONTINUOUS(port_OP_SPI_FREE_RUN_SCLK_ENABLE)     | \
                     port_OP_GET_SPI_CTRL_SSEL_POLARITY (port_OP_SPI_SS_POLARITY)               | \
                     port_OP_GET_SPI_CTRL_SUB_MODE      (port_OP_SPI_SUB_MODE)                  | \
                     port_OP_GET_SPI_CTRL_MASTER_MODE   (port_OP_SPI_MODE))

    /* RX direction */
    #define port_OP_SPI_DEFAULT_RX_CTRL \
                    (port_OP_GET_RX_CTRL_DATA_WIDTH(port_OP_SPI_RX_DATA_BITS_NUM)     | \
                     port_OP_GET_RX_CTRL_BIT_ORDER (port_OP_SPI_BITS_ORDER)           | \
                     port_OP_GET_RX_CTRL_MEDIAN    (port_OP_SPI_MEDIAN_FILTER_ENABLE) | \
                     port_OP_SPI_RX_CTRL)

    #define port_OP_SPI_DEFAULT_RX_FIFO_CTRL \
                    port_OP_GET_RX_FIFO_CTRL_TRIGGER_LEVEL(port_OP_SPI_RX_TRIGGER_LEVEL)

    /* TX direction */
    #define port_OP_SPI_DEFAULT_TX_CTRL \
                    (port_OP_GET_TX_CTRL_DATA_WIDTH(port_OP_SPI_TX_DATA_BITS_NUM) | \
                     port_OP_GET_TX_CTRL_BIT_ORDER (port_OP_SPI_BITS_ORDER)       | \
                     port_OP_SPI_TX_CTRL)

    #define port_OP_SPI_DEFAULT_TX_FIFO_CTRL \
                    port_OP_GET_TX_FIFO_CTRL_TRIGGER_LEVEL(port_OP_SPI_TX_TRIGGER_LEVEL)

    /* Interrupt sources */
    #define port_OP_SPI_DEFAULT_INTR_SPI_EC_MASK   (port_OP_NO_INTR_SOURCES)

    #define port_OP_SPI_DEFAULT_INTR_I2C_EC_MASK   (port_OP_NO_INTR_SOURCES)
    #define port_OP_SPI_DEFAULT_INTR_SLAVE_MASK \
                    (port_OP_SPI_INTR_RX_MASK & port_OP_INTR_SLAVE_SPI_BUS_ERROR)

    #define port_OP_SPI_DEFAULT_INTR_MASTER_MASK \
                    (port_OP_SPI_INTR_TX_MASK & port_OP_INTR_MASTER_SPI_DONE)

    #define port_OP_SPI_DEFAULT_INTR_RX_MASK \
                    (port_OP_SPI_INTR_RX_MASK & (uint32) ~port_OP_INTR_SLAVE_SPI_BUS_ERROR)

    #define port_OP_SPI_DEFAULT_INTR_TX_MASK \
                    (port_OP_SPI_INTR_TX_MASK & (uint32) ~port_OP_INTR_MASTER_SPI_DONE)

#endif /* (port_OP_SCB_MODE_SPI_CONST_CFG) */


/***************************************
*        UART Register Settings
***************************************/

#define port_OP_CTRL_UART      (port_OP_CTRL_MODE_UART)
#define port_OP_UART_RX_CTRL   (port_OP_RX_CTRL_LSB_FIRST) /* LSB for UART goes first */
#define port_OP_UART_TX_CTRL   (port_OP_TX_CTRL_LSB_FIRST) /* LSB for UART goes first */


/***************************************
*      UART Init Register Settings
***************************************/

#if(port_OP_SCB_MODE_UART_CONST_CFG)

    /* UART configuration */
    #if(port_OP_UART_MODE_IRDA == port_OP_UART_SUB_MODE)

        #define port_OP_DEFAULT_CTRL_OVS   ((0u != port_OP_UART_IRDA_LOW_POWER) ?              \
                                (port_OP_UART_GET_CTRL_OVS_IRDA_LP(port_OP_UART_OVS_FACTOR)) : \
                                (port_OP_CTRL_OVS_IRDA_OVS16))

    #else

        #define port_OP_DEFAULT_CTRL_OVS   port_OP_GET_CTRL_OVS(port_OP_UART_OVS_FACTOR)

    #endif /* (port_OP_UART_MODE_IRDA == port_OP_UART_SUB_MODE) */

    #define port_OP_UART_DEFAULT_CTRL \
                                (port_OP_GET_CTRL_BYTE_MODE  (port_OP_UART_BYTE_MODE_ENABLE)  | \
                                 port_OP_GET_CTRL_ADDR_ACCEPT(port_OP_UART_MP_ACCEPT_ADDRESS) | \
                                 port_OP_DEFAULT_CTRL_OVS                                              | \
                                 port_OP_CTRL_UART)

    #define port_OP_UART_DEFAULT_UART_CTRL \
                                    (port_OP_GET_UART_CTRL_MODE(port_OP_UART_SUB_MODE))

    /* RX direction */
    #define port_OP_UART_DEFAULT_RX_CTRL_PARITY \
                                ((port_OP_UART_PARITY_NONE != port_OP_UART_PARITY_TYPE) ?      \
                                  (port_OP_GET_UART_RX_CTRL_PARITY(port_OP_UART_PARITY_TYPE) | \
                                   port_OP_UART_RX_CTRL_PARITY_ENABLED) : (0u))

    #define port_OP_UART_DEFAULT_UART_RX_CTRL \
                    (port_OP_GET_UART_RX_CTRL_MODE(port_OP_UART_STOP_BITS_NUM)                    | \
                     port_OP_GET_UART_RX_CTRL_POLARITY(port_OP_UART_IRDA_POLARITY)                | \
                     port_OP_GET_UART_RX_CTRL_MP_MODE(port_OP_UART_MP_MODE_ENABLE)                | \
                     port_OP_GET_UART_RX_CTRL_DROP_ON_PARITY_ERR(port_OP_UART_DROP_ON_PARITY_ERR) | \
                     port_OP_GET_UART_RX_CTRL_DROP_ON_FRAME_ERR(port_OP_UART_DROP_ON_FRAME_ERR)   | \
                     port_OP_GET_UART_RX_CTRL_BREAK_WIDTH(port_OP_UART_RX_BREAK_WIDTH)            | \
                     port_OP_UART_DEFAULT_RX_CTRL_PARITY)


    #define port_OP_UART_DEFAULT_RX_CTRL \
                                (port_OP_GET_RX_CTRL_DATA_WIDTH(port_OP_UART_DATA_BITS_NUM)        | \
                                 port_OP_GET_RX_CTRL_MEDIAN    (port_OP_UART_MEDIAN_FILTER_ENABLE) | \
                                 port_OP_GET_UART_RX_CTRL_ENABLED(port_OP_UART_DIRECTION))

    #define port_OP_UART_DEFAULT_RX_FIFO_CTRL \
                                port_OP_GET_RX_FIFO_CTRL_TRIGGER_LEVEL(port_OP_UART_RX_TRIGGER_LEVEL)

    #define port_OP_UART_DEFAULT_RX_MATCH_REG  ((0u != port_OP_UART_MP_MODE_ENABLE) ?          \
                                (port_OP_GET_RX_MATCH_ADDR(port_OP_UART_MP_RX_ADDRESS) | \
                                 port_OP_GET_RX_MATCH_MASK(port_OP_UART_MP_RX_ADDRESS_MASK)) : (0u))

    /* TX direction */
    #define port_OP_UART_DEFAULT_TX_CTRL_PARITY (port_OP_UART_DEFAULT_RX_CTRL_PARITY)

    #define port_OP_UART_DEFAULT_UART_TX_CTRL \
                                (port_OP_GET_UART_TX_CTRL_MODE(port_OP_UART_STOP_BITS_NUM)       | \
                                 port_OP_GET_UART_TX_CTRL_RETRY_NACK(port_OP_UART_RETRY_ON_NACK) | \
                                 port_OP_UART_DEFAULT_TX_CTRL_PARITY)

    #define port_OP_UART_DEFAULT_TX_CTRL \
                                (port_OP_GET_TX_CTRL_DATA_WIDTH(port_OP_UART_DATA_BITS_NUM) | \
                                 port_OP_GET_UART_TX_CTRL_ENABLED(port_OP_UART_DIRECTION))

    #define port_OP_UART_DEFAULT_TX_FIFO_CTRL \
                                port_OP_GET_TX_FIFO_CTRL_TRIGGER_LEVEL(port_OP_UART_TX_TRIGGER_LEVEL)

    #define port_OP_UART_DEFAULT_FLOW_CTRL \
                        (port_OP_GET_UART_FLOW_CTRL_TRIGGER_LEVEL(port_OP_UART_RTS_FIFO_LEVEL) | \
                         port_OP_GET_UART_FLOW_CTRL_RTS_POLARITY (port_OP_UART_RTS_POLARITY)   | \
                         port_OP_GET_UART_FLOW_CTRL_CTS_POLARITY (port_OP_UART_CTS_POLARITY)   | \
                         port_OP_GET_UART_FLOW_CTRL_CTS_ENABLE   (port_OP_UART_CTS_ENABLE))

    /* Interrupt sources */
    #define port_OP_UART_DEFAULT_INTR_I2C_EC_MASK  (port_OP_NO_INTR_SOURCES)
    #define port_OP_UART_DEFAULT_INTR_SPI_EC_MASK  (port_OP_NO_INTR_SOURCES)
    #define port_OP_UART_DEFAULT_INTR_SLAVE_MASK   (port_OP_NO_INTR_SOURCES)
    #define port_OP_UART_DEFAULT_INTR_MASTER_MASK  (port_OP_NO_INTR_SOURCES)
    #define port_OP_UART_DEFAULT_INTR_RX_MASK      (port_OP_UART_INTR_RX_MASK)
    #define port_OP_UART_DEFAULT_INTR_TX_MASK      (port_OP_UART_INTR_TX_MASK)

#endif /* (port_OP_SCB_MODE_UART_CONST_CFG) */


/***************************************
* The following code is DEPRECATED and
* must not be used.
***************************************/

#define port_OP_SPIM_ACTIVE_SS0    (port_OP_SPI_SLAVE_SELECT0)
#define port_OP_SPIM_ACTIVE_SS1    (port_OP_SPI_SLAVE_SELECT1)
#define port_OP_SPIM_ACTIVE_SS2    (port_OP_SPI_SLAVE_SELECT2)
#define port_OP_SPIM_ACTIVE_SS3    (port_OP_SPI_SLAVE_SELECT3)

#endif /* CY_SCB_SPI_UART_port_OP_H */


/* [] END OF FILE */
