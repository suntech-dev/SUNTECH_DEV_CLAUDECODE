/***************************************************************************//**
* \file QR_SCANNER_PINS.h
* \version 4.0
*
* \brief
*  This file provides constants and parameter values for the pin components
*  buried into SCB Component.
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

#if !defined(CY_SCB_PINS_QR_SCANNER_H)
#define CY_SCB_PINS_QR_SCANNER_H

#include "cydevice_trm.h"
#include "cyfitter.h"
#include "cytypes.h"


/***************************************
*   Conditional Compilation Parameters
****************************************/

/* Unconfigured pins */
#define QR_SCANNER_REMOVE_RX_WAKE_SCL_MOSI_PIN  (1u)
#define QR_SCANNER_REMOVE_RX_SCL_MOSI_PIN      (1u)
#define QR_SCANNER_REMOVE_TX_SDA_MISO_PIN      (1u)
#define QR_SCANNER_REMOVE_CTS_SCLK_PIN      (1u)
#define QR_SCANNER_REMOVE_RTS_SS0_PIN      (1u)
#define QR_SCANNER_REMOVE_SS1_PIN                 (1u)
#define QR_SCANNER_REMOVE_SS2_PIN                 (1u)
#define QR_SCANNER_REMOVE_SS3_PIN                 (1u)

/* Mode defined pins */
#define QR_SCANNER_REMOVE_I2C_PINS                (1u)
#define QR_SCANNER_REMOVE_SPI_MASTER_PINS         (1u)
#define QR_SCANNER_REMOVE_SPI_MASTER_SCLK_PIN     (1u)
#define QR_SCANNER_REMOVE_SPI_MASTER_MOSI_PIN     (1u)
#define QR_SCANNER_REMOVE_SPI_MASTER_MISO_PIN     (1u)
#define QR_SCANNER_REMOVE_SPI_MASTER_SS0_PIN      (1u)
#define QR_SCANNER_REMOVE_SPI_MASTER_SS1_PIN      (1u)
#define QR_SCANNER_REMOVE_SPI_MASTER_SS2_PIN      (1u)
#define QR_SCANNER_REMOVE_SPI_MASTER_SS3_PIN      (1u)
#define QR_SCANNER_REMOVE_SPI_SLAVE_PINS          (1u)
#define QR_SCANNER_REMOVE_SPI_SLAVE_MOSI_PIN      (1u)
#define QR_SCANNER_REMOVE_SPI_SLAVE_MISO_PIN      (1u)
#define QR_SCANNER_REMOVE_UART_TX_PIN             (0u)
#define QR_SCANNER_REMOVE_UART_RX_TX_PIN          (1u)
#define QR_SCANNER_REMOVE_UART_RX_PIN             (0u)
#define QR_SCANNER_REMOVE_UART_RX_WAKE_PIN        (1u)
#define QR_SCANNER_REMOVE_UART_RTS_PIN            (1u)
#define QR_SCANNER_REMOVE_UART_CTS_PIN            (1u)

/* Unconfigured pins */
#define QR_SCANNER_RX_WAKE_SCL_MOSI_PIN (0u == QR_SCANNER_REMOVE_RX_WAKE_SCL_MOSI_PIN)
#define QR_SCANNER_RX_SCL_MOSI_PIN     (0u == QR_SCANNER_REMOVE_RX_SCL_MOSI_PIN)
#define QR_SCANNER_TX_SDA_MISO_PIN     (0u == QR_SCANNER_REMOVE_TX_SDA_MISO_PIN)
#define QR_SCANNER_CTS_SCLK_PIN     (0u == QR_SCANNER_REMOVE_CTS_SCLK_PIN)
#define QR_SCANNER_RTS_SS0_PIN     (0u == QR_SCANNER_REMOVE_RTS_SS0_PIN)
#define QR_SCANNER_SS1_PIN                (0u == QR_SCANNER_REMOVE_SS1_PIN)
#define QR_SCANNER_SS2_PIN                (0u == QR_SCANNER_REMOVE_SS2_PIN)
#define QR_SCANNER_SS3_PIN                (0u == QR_SCANNER_REMOVE_SS3_PIN)

/* Mode defined pins */
#define QR_SCANNER_I2C_PINS               (0u == QR_SCANNER_REMOVE_I2C_PINS)
#define QR_SCANNER_SPI_MASTER_PINS        (0u == QR_SCANNER_REMOVE_SPI_MASTER_PINS)
#define QR_SCANNER_SPI_MASTER_SCLK_PIN    (0u == QR_SCANNER_REMOVE_SPI_MASTER_SCLK_PIN)
#define QR_SCANNER_SPI_MASTER_MOSI_PIN    (0u == QR_SCANNER_REMOVE_SPI_MASTER_MOSI_PIN)
#define QR_SCANNER_SPI_MASTER_MISO_PIN    (0u == QR_SCANNER_REMOVE_SPI_MASTER_MISO_PIN)
#define QR_SCANNER_SPI_MASTER_SS0_PIN     (0u == QR_SCANNER_REMOVE_SPI_MASTER_SS0_PIN)
#define QR_SCANNER_SPI_MASTER_SS1_PIN     (0u == QR_SCANNER_REMOVE_SPI_MASTER_SS1_PIN)
#define QR_SCANNER_SPI_MASTER_SS2_PIN     (0u == QR_SCANNER_REMOVE_SPI_MASTER_SS2_PIN)
#define QR_SCANNER_SPI_MASTER_SS3_PIN     (0u == QR_SCANNER_REMOVE_SPI_MASTER_SS3_PIN)
#define QR_SCANNER_SPI_SLAVE_PINS         (0u == QR_SCANNER_REMOVE_SPI_SLAVE_PINS)
#define QR_SCANNER_SPI_SLAVE_MOSI_PIN     (0u == QR_SCANNER_REMOVE_SPI_SLAVE_MOSI_PIN)
#define QR_SCANNER_SPI_SLAVE_MISO_PIN     (0u == QR_SCANNER_REMOVE_SPI_SLAVE_MISO_PIN)
#define QR_SCANNER_UART_TX_PIN            (0u == QR_SCANNER_REMOVE_UART_TX_PIN)
#define QR_SCANNER_UART_RX_TX_PIN         (0u == QR_SCANNER_REMOVE_UART_RX_TX_PIN)
#define QR_SCANNER_UART_RX_PIN            (0u == QR_SCANNER_REMOVE_UART_RX_PIN)
#define QR_SCANNER_UART_RX_WAKE_PIN       (0u == QR_SCANNER_REMOVE_UART_RX_WAKE_PIN)
#define QR_SCANNER_UART_RTS_PIN           (0u == QR_SCANNER_REMOVE_UART_RTS_PIN)
#define QR_SCANNER_UART_CTS_PIN           (0u == QR_SCANNER_REMOVE_UART_CTS_PIN)


/***************************************
*             Includes
****************************************/

#if (QR_SCANNER_RX_WAKE_SCL_MOSI_PIN)
    #include "QR_SCANNER_uart_rx_wake_i2c_scl_spi_mosi.h"
#endif /* (QR_SCANNER_RX_SCL_MOSI) */

#if (QR_SCANNER_RX_SCL_MOSI_PIN)
    #include "QR_SCANNER_uart_rx_i2c_scl_spi_mosi.h"
#endif /* (QR_SCANNER_RX_SCL_MOSI) */

#if (QR_SCANNER_TX_SDA_MISO_PIN)
    #include "QR_SCANNER_uart_tx_i2c_sda_spi_miso.h"
#endif /* (QR_SCANNER_TX_SDA_MISO) */

#if (QR_SCANNER_CTS_SCLK_PIN)
    #include "QR_SCANNER_uart_cts_spi_sclk.h"
#endif /* (QR_SCANNER_CTS_SCLK) */

#if (QR_SCANNER_RTS_SS0_PIN)
    #include "QR_SCANNER_uart_rts_spi_ss0.h"
#endif /* (QR_SCANNER_RTS_SS0_PIN) */

#if (QR_SCANNER_SS1_PIN)
    #include "QR_SCANNER_spi_ss1.h"
#endif /* (QR_SCANNER_SS1_PIN) */

#if (QR_SCANNER_SS2_PIN)
    #include "QR_SCANNER_spi_ss2.h"
#endif /* (QR_SCANNER_SS2_PIN) */

#if (QR_SCANNER_SS3_PIN)
    #include "QR_SCANNER_spi_ss3.h"
#endif /* (QR_SCANNER_SS3_PIN) */

#if (QR_SCANNER_I2C_PINS)
    #include "QR_SCANNER_scl.h"
    #include "QR_SCANNER_sda.h"
#endif /* (QR_SCANNER_I2C_PINS) */

#if (QR_SCANNER_SPI_MASTER_PINS)
#if (QR_SCANNER_SPI_MASTER_SCLK_PIN)
    #include "QR_SCANNER_sclk_m.h"
#endif /* (QR_SCANNER_SPI_MASTER_SCLK_PIN) */

#if (QR_SCANNER_SPI_MASTER_MOSI_PIN)
    #include "QR_SCANNER_mosi_m.h"
#endif /* (QR_SCANNER_SPI_MASTER_MOSI_PIN) */

#if (QR_SCANNER_SPI_MASTER_MISO_PIN)
    #include "QR_SCANNER_miso_m.h"
#endif /*(QR_SCANNER_SPI_MASTER_MISO_PIN) */
#endif /* (QR_SCANNER_SPI_MASTER_PINS) */

#if (QR_SCANNER_SPI_SLAVE_PINS)
    #include "QR_SCANNER_sclk_s.h"
    #include "QR_SCANNER_ss_s.h"

#if (QR_SCANNER_SPI_SLAVE_MOSI_PIN)
    #include "QR_SCANNER_mosi_s.h"
#endif /* (QR_SCANNER_SPI_SLAVE_MOSI_PIN) */

#if (QR_SCANNER_SPI_SLAVE_MISO_PIN)
    #include "QR_SCANNER_miso_s.h"
#endif /*(QR_SCANNER_SPI_SLAVE_MISO_PIN) */
#endif /* (QR_SCANNER_SPI_SLAVE_PINS) */

#if (QR_SCANNER_SPI_MASTER_SS0_PIN)
    #include "QR_SCANNER_ss0_m.h"
#endif /* (QR_SCANNER_SPI_MASTER_SS0_PIN) */

#if (QR_SCANNER_SPI_MASTER_SS1_PIN)
    #include "QR_SCANNER_ss1_m.h"
#endif /* (QR_SCANNER_SPI_MASTER_SS1_PIN) */

#if (QR_SCANNER_SPI_MASTER_SS2_PIN)
    #include "QR_SCANNER_ss2_m.h"
#endif /* (QR_SCANNER_SPI_MASTER_SS2_PIN) */

#if (QR_SCANNER_SPI_MASTER_SS3_PIN)
    #include "QR_SCANNER_ss3_m.h"
#endif /* (QR_SCANNER_SPI_MASTER_SS3_PIN) */

#if (QR_SCANNER_UART_TX_PIN)
    #include "QR_SCANNER_tx.h"
#endif /* (QR_SCANNER_UART_TX_PIN) */

#if (QR_SCANNER_UART_RX_TX_PIN)
    #include "QR_SCANNER_rx_tx.h"
#endif /* (QR_SCANNER_UART_RX_TX_PIN) */

#if (QR_SCANNER_UART_RX_PIN)
    #include "QR_SCANNER_rx.h"
#endif /* (QR_SCANNER_UART_RX_PIN) */

#if (QR_SCANNER_UART_RX_WAKE_PIN)
    #include "QR_SCANNER_rx_wake.h"
#endif /* (QR_SCANNER_UART_RX_WAKE_PIN) */

#if (QR_SCANNER_UART_RTS_PIN)
    #include "QR_SCANNER_rts.h"
#endif /* (QR_SCANNER_UART_RTS_PIN) */

#if (QR_SCANNER_UART_CTS_PIN)
    #include "QR_SCANNER_cts.h"
#endif /* (QR_SCANNER_UART_CTS_PIN) */


/***************************************
*              Registers
***************************************/

#if (QR_SCANNER_RX_SCL_MOSI_PIN)
    #define QR_SCANNER_RX_SCL_MOSI_HSIOM_REG   (*(reg32 *) QR_SCANNER_uart_rx_i2c_scl_spi_mosi__0__HSIOM)
    #define QR_SCANNER_RX_SCL_MOSI_HSIOM_PTR   ( (reg32 *) QR_SCANNER_uart_rx_i2c_scl_spi_mosi__0__HSIOM)
    
    #define QR_SCANNER_RX_SCL_MOSI_HSIOM_MASK      (QR_SCANNER_uart_rx_i2c_scl_spi_mosi__0__HSIOM_MASK)
    #define QR_SCANNER_RX_SCL_MOSI_HSIOM_POS       (QR_SCANNER_uart_rx_i2c_scl_spi_mosi__0__HSIOM_SHIFT)
    #define QR_SCANNER_RX_SCL_MOSI_HSIOM_SEL_GPIO  (QR_SCANNER_uart_rx_i2c_scl_spi_mosi__0__HSIOM_GPIO)
    #define QR_SCANNER_RX_SCL_MOSI_HSIOM_SEL_I2C   (QR_SCANNER_uart_rx_i2c_scl_spi_mosi__0__HSIOM_I2C)
    #define QR_SCANNER_RX_SCL_MOSI_HSIOM_SEL_SPI   (QR_SCANNER_uart_rx_i2c_scl_spi_mosi__0__HSIOM_SPI)
    #define QR_SCANNER_RX_SCL_MOSI_HSIOM_SEL_UART  (QR_SCANNER_uart_rx_i2c_scl_spi_mosi__0__HSIOM_UART)
    
#elif (QR_SCANNER_RX_WAKE_SCL_MOSI_PIN)
    #define QR_SCANNER_RX_WAKE_SCL_MOSI_HSIOM_REG   (*(reg32 *) QR_SCANNER_uart_rx_wake_i2c_scl_spi_mosi__0__HSIOM)
    #define QR_SCANNER_RX_WAKE_SCL_MOSI_HSIOM_PTR   ( (reg32 *) QR_SCANNER_uart_rx_wake_i2c_scl_spi_mosi__0__HSIOM)
    
    #define QR_SCANNER_RX_WAKE_SCL_MOSI_HSIOM_MASK      (QR_SCANNER_uart_rx_wake_i2c_scl_spi_mosi__0__HSIOM_MASK)
    #define QR_SCANNER_RX_WAKE_SCL_MOSI_HSIOM_POS       (QR_SCANNER_uart_rx_wake_i2c_scl_spi_mosi__0__HSIOM_SHIFT)
    #define QR_SCANNER_RX_WAKE_SCL_MOSI_HSIOM_SEL_GPIO  (QR_SCANNER_uart_rx_wake_i2c_scl_spi_mosi__0__HSIOM_GPIO)
    #define QR_SCANNER_RX_WAKE_SCL_MOSI_HSIOM_SEL_I2C   (QR_SCANNER_uart_rx_wake_i2c_scl_spi_mosi__0__HSIOM_I2C)
    #define QR_SCANNER_RX_WAKE_SCL_MOSI_HSIOM_SEL_SPI   (QR_SCANNER_uart_rx_wake_i2c_scl_spi_mosi__0__HSIOM_SPI)
    #define QR_SCANNER_RX_WAKE_SCL_MOSI_HSIOM_SEL_UART  (QR_SCANNER_uart_rx_wake_i2c_scl_spi_mosi__0__HSIOM_UART)    
   
    #define QR_SCANNER_RX_WAKE_SCL_MOSI_INTCFG_REG (*(reg32 *) QR_SCANNER_uart_rx_wake_i2c_scl_spi_mosi__0__INTCFG)
    #define QR_SCANNER_RX_WAKE_SCL_MOSI_INTCFG_PTR ( (reg32 *) QR_SCANNER_uart_rx_wake_i2c_scl_spi_mosi__0__INTCFG)
    #define QR_SCANNER_RX_WAKE_SCL_MOSI_INTCFG_TYPE_POS  (QR_SCANNER_uart_rx_wake_i2c_scl_spi_mosi__SHIFT)
    #define QR_SCANNER_RX_WAKE_SCL_MOSI_INTCFG_TYPE_MASK ((uint32) QR_SCANNER_INTCFG_TYPE_MASK << \
                                                                           QR_SCANNER_RX_WAKE_SCL_MOSI_INTCFG_TYPE_POS)
#else
    /* None of pins QR_SCANNER_RX_SCL_MOSI_PIN or QR_SCANNER_RX_WAKE_SCL_MOSI_PIN present.*/
#endif /* (QR_SCANNER_RX_SCL_MOSI_PIN) */

#if (QR_SCANNER_TX_SDA_MISO_PIN)
    #define QR_SCANNER_TX_SDA_MISO_HSIOM_REG   (*(reg32 *) QR_SCANNER_uart_tx_i2c_sda_spi_miso__0__HSIOM)
    #define QR_SCANNER_TX_SDA_MISO_HSIOM_PTR   ( (reg32 *) QR_SCANNER_uart_tx_i2c_sda_spi_miso__0__HSIOM)
    
    #define QR_SCANNER_TX_SDA_MISO_HSIOM_MASK      (QR_SCANNER_uart_tx_i2c_sda_spi_miso__0__HSIOM_MASK)
    #define QR_SCANNER_TX_SDA_MISO_HSIOM_POS       (QR_SCANNER_uart_tx_i2c_sda_spi_miso__0__HSIOM_SHIFT)
    #define QR_SCANNER_TX_SDA_MISO_HSIOM_SEL_GPIO  (QR_SCANNER_uart_tx_i2c_sda_spi_miso__0__HSIOM_GPIO)
    #define QR_SCANNER_TX_SDA_MISO_HSIOM_SEL_I2C   (QR_SCANNER_uart_tx_i2c_sda_spi_miso__0__HSIOM_I2C)
    #define QR_SCANNER_TX_SDA_MISO_HSIOM_SEL_SPI   (QR_SCANNER_uart_tx_i2c_sda_spi_miso__0__HSIOM_SPI)
    #define QR_SCANNER_TX_SDA_MISO_HSIOM_SEL_UART  (QR_SCANNER_uart_tx_i2c_sda_spi_miso__0__HSIOM_UART)
#endif /* (QR_SCANNER_TX_SDA_MISO_PIN) */

#if (QR_SCANNER_CTS_SCLK_PIN)
    #define QR_SCANNER_CTS_SCLK_HSIOM_REG   (*(reg32 *) QR_SCANNER_uart_cts_spi_sclk__0__HSIOM)
    #define QR_SCANNER_CTS_SCLK_HSIOM_PTR   ( (reg32 *) QR_SCANNER_uart_cts_spi_sclk__0__HSIOM)
    
    #define QR_SCANNER_CTS_SCLK_HSIOM_MASK      (QR_SCANNER_uart_cts_spi_sclk__0__HSIOM_MASK)
    #define QR_SCANNER_CTS_SCLK_HSIOM_POS       (QR_SCANNER_uart_cts_spi_sclk__0__HSIOM_SHIFT)
    #define QR_SCANNER_CTS_SCLK_HSIOM_SEL_GPIO  (QR_SCANNER_uart_cts_spi_sclk__0__HSIOM_GPIO)
    #define QR_SCANNER_CTS_SCLK_HSIOM_SEL_I2C   (QR_SCANNER_uart_cts_spi_sclk__0__HSIOM_I2C)
    #define QR_SCANNER_CTS_SCLK_HSIOM_SEL_SPI   (QR_SCANNER_uart_cts_spi_sclk__0__HSIOM_SPI)
    #define QR_SCANNER_CTS_SCLK_HSIOM_SEL_UART  (QR_SCANNER_uart_cts_spi_sclk__0__HSIOM_UART)
#endif /* (QR_SCANNER_CTS_SCLK_PIN) */

#if (QR_SCANNER_RTS_SS0_PIN)
    #define QR_SCANNER_RTS_SS0_HSIOM_REG   (*(reg32 *) QR_SCANNER_uart_rts_spi_ss0__0__HSIOM)
    #define QR_SCANNER_RTS_SS0_HSIOM_PTR   ( (reg32 *) QR_SCANNER_uart_rts_spi_ss0__0__HSIOM)
    
    #define QR_SCANNER_RTS_SS0_HSIOM_MASK      (QR_SCANNER_uart_rts_spi_ss0__0__HSIOM_MASK)
    #define QR_SCANNER_RTS_SS0_HSIOM_POS       (QR_SCANNER_uart_rts_spi_ss0__0__HSIOM_SHIFT)
    #define QR_SCANNER_RTS_SS0_HSIOM_SEL_GPIO  (QR_SCANNER_uart_rts_spi_ss0__0__HSIOM_GPIO)
    #define QR_SCANNER_RTS_SS0_HSIOM_SEL_I2C   (QR_SCANNER_uart_rts_spi_ss0__0__HSIOM_I2C)
    #define QR_SCANNER_RTS_SS0_HSIOM_SEL_SPI   (QR_SCANNER_uart_rts_spi_ss0__0__HSIOM_SPI)
#if !(QR_SCANNER_CY_SCBIP_V0 || QR_SCANNER_CY_SCBIP_V1)
    #define QR_SCANNER_RTS_SS0_HSIOM_SEL_UART  (QR_SCANNER_uart_rts_spi_ss0__0__HSIOM_UART)
#endif /* !(QR_SCANNER_CY_SCBIP_V0 || QR_SCANNER_CY_SCBIP_V1) */
#endif /* (QR_SCANNER_RTS_SS0_PIN) */

#if (QR_SCANNER_SS1_PIN)
    #define QR_SCANNER_SS1_HSIOM_REG  (*(reg32 *) QR_SCANNER_spi_ss1__0__HSIOM)
    #define QR_SCANNER_SS1_HSIOM_PTR  ( (reg32 *) QR_SCANNER_spi_ss1__0__HSIOM)
    
    #define QR_SCANNER_SS1_HSIOM_MASK     (QR_SCANNER_spi_ss1__0__HSIOM_MASK)
    #define QR_SCANNER_SS1_HSIOM_POS      (QR_SCANNER_spi_ss1__0__HSIOM_SHIFT)
    #define QR_SCANNER_SS1_HSIOM_SEL_GPIO (QR_SCANNER_spi_ss1__0__HSIOM_GPIO)
    #define QR_SCANNER_SS1_HSIOM_SEL_I2C  (QR_SCANNER_spi_ss1__0__HSIOM_I2C)
    #define QR_SCANNER_SS1_HSIOM_SEL_SPI  (QR_SCANNER_spi_ss1__0__HSIOM_SPI)
#endif /* (QR_SCANNER_SS1_PIN) */

#if (QR_SCANNER_SS2_PIN)
    #define QR_SCANNER_SS2_HSIOM_REG     (*(reg32 *) QR_SCANNER_spi_ss2__0__HSIOM)
    #define QR_SCANNER_SS2_HSIOM_PTR     ( (reg32 *) QR_SCANNER_spi_ss2__0__HSIOM)
    
    #define QR_SCANNER_SS2_HSIOM_MASK     (QR_SCANNER_spi_ss2__0__HSIOM_MASK)
    #define QR_SCANNER_SS2_HSIOM_POS      (QR_SCANNER_spi_ss2__0__HSIOM_SHIFT)
    #define QR_SCANNER_SS2_HSIOM_SEL_GPIO (QR_SCANNER_spi_ss2__0__HSIOM_GPIO)
    #define QR_SCANNER_SS2_HSIOM_SEL_I2C  (QR_SCANNER_spi_ss2__0__HSIOM_I2C)
    #define QR_SCANNER_SS2_HSIOM_SEL_SPI  (QR_SCANNER_spi_ss2__0__HSIOM_SPI)
#endif /* (QR_SCANNER_SS2_PIN) */

#if (QR_SCANNER_SS3_PIN)
    #define QR_SCANNER_SS3_HSIOM_REG     (*(reg32 *) QR_SCANNER_spi_ss3__0__HSIOM)
    #define QR_SCANNER_SS3_HSIOM_PTR     ( (reg32 *) QR_SCANNER_spi_ss3__0__HSIOM)
    
    #define QR_SCANNER_SS3_HSIOM_MASK     (QR_SCANNER_spi_ss3__0__HSIOM_MASK)
    #define QR_SCANNER_SS3_HSIOM_POS      (QR_SCANNER_spi_ss3__0__HSIOM_SHIFT)
    #define QR_SCANNER_SS3_HSIOM_SEL_GPIO (QR_SCANNER_spi_ss3__0__HSIOM_GPIO)
    #define QR_SCANNER_SS3_HSIOM_SEL_I2C  (QR_SCANNER_spi_ss3__0__HSIOM_I2C)
    #define QR_SCANNER_SS3_HSIOM_SEL_SPI  (QR_SCANNER_spi_ss3__0__HSIOM_SPI)
#endif /* (QR_SCANNER_SS3_PIN) */

#if (QR_SCANNER_I2C_PINS)
    #define QR_SCANNER_SCL_HSIOM_REG  (*(reg32 *) QR_SCANNER_scl__0__HSIOM)
    #define QR_SCANNER_SCL_HSIOM_PTR  ( (reg32 *) QR_SCANNER_scl__0__HSIOM)
    
    #define QR_SCANNER_SCL_HSIOM_MASK     (QR_SCANNER_scl__0__HSIOM_MASK)
    #define QR_SCANNER_SCL_HSIOM_POS      (QR_SCANNER_scl__0__HSIOM_SHIFT)
    #define QR_SCANNER_SCL_HSIOM_SEL_GPIO (QR_SCANNER_sda__0__HSIOM_GPIO)
    #define QR_SCANNER_SCL_HSIOM_SEL_I2C  (QR_SCANNER_sda__0__HSIOM_I2C)
    
    #define QR_SCANNER_SDA_HSIOM_REG  (*(reg32 *) QR_SCANNER_sda__0__HSIOM)
    #define QR_SCANNER_SDA_HSIOM_PTR  ( (reg32 *) QR_SCANNER_sda__0__HSIOM)
    
    #define QR_SCANNER_SDA_HSIOM_MASK     (QR_SCANNER_sda__0__HSIOM_MASK)
    #define QR_SCANNER_SDA_HSIOM_POS      (QR_SCANNER_sda__0__HSIOM_SHIFT)
    #define QR_SCANNER_SDA_HSIOM_SEL_GPIO (QR_SCANNER_sda__0__HSIOM_GPIO)
    #define QR_SCANNER_SDA_HSIOM_SEL_I2C  (QR_SCANNER_sda__0__HSIOM_I2C)
#endif /* (QR_SCANNER_I2C_PINS) */

#if (QR_SCANNER_SPI_SLAVE_PINS)
    #define QR_SCANNER_SCLK_S_HSIOM_REG   (*(reg32 *) QR_SCANNER_sclk_s__0__HSIOM)
    #define QR_SCANNER_SCLK_S_HSIOM_PTR   ( (reg32 *) QR_SCANNER_sclk_s__0__HSIOM)
    
    #define QR_SCANNER_SCLK_S_HSIOM_MASK      (QR_SCANNER_sclk_s__0__HSIOM_MASK)
    #define QR_SCANNER_SCLK_S_HSIOM_POS       (QR_SCANNER_sclk_s__0__HSIOM_SHIFT)
    #define QR_SCANNER_SCLK_S_HSIOM_SEL_GPIO  (QR_SCANNER_sclk_s__0__HSIOM_GPIO)
    #define QR_SCANNER_SCLK_S_HSIOM_SEL_SPI   (QR_SCANNER_sclk_s__0__HSIOM_SPI)
    
    #define QR_SCANNER_SS0_S_HSIOM_REG    (*(reg32 *) QR_SCANNER_ss0_s__0__HSIOM)
    #define QR_SCANNER_SS0_S_HSIOM_PTR    ( (reg32 *) QR_SCANNER_ss0_s__0__HSIOM)
    
    #define QR_SCANNER_SS0_S_HSIOM_MASK       (QR_SCANNER_ss0_s__0__HSIOM_MASK)
    #define QR_SCANNER_SS0_S_HSIOM_POS        (QR_SCANNER_ss0_s__0__HSIOM_SHIFT)
    #define QR_SCANNER_SS0_S_HSIOM_SEL_GPIO   (QR_SCANNER_ss0_s__0__HSIOM_GPIO)  
    #define QR_SCANNER_SS0_S_HSIOM_SEL_SPI    (QR_SCANNER_ss0_s__0__HSIOM_SPI)
#endif /* (QR_SCANNER_SPI_SLAVE_PINS) */

#if (QR_SCANNER_SPI_SLAVE_MOSI_PIN)
    #define QR_SCANNER_MOSI_S_HSIOM_REG   (*(reg32 *) QR_SCANNER_mosi_s__0__HSIOM)
    #define QR_SCANNER_MOSI_S_HSIOM_PTR   ( (reg32 *) QR_SCANNER_mosi_s__0__HSIOM)
    
    #define QR_SCANNER_MOSI_S_HSIOM_MASK      (QR_SCANNER_mosi_s__0__HSIOM_MASK)
    #define QR_SCANNER_MOSI_S_HSIOM_POS       (QR_SCANNER_mosi_s__0__HSIOM_SHIFT)
    #define QR_SCANNER_MOSI_S_HSIOM_SEL_GPIO  (QR_SCANNER_mosi_s__0__HSIOM_GPIO)
    #define QR_SCANNER_MOSI_S_HSIOM_SEL_SPI   (QR_SCANNER_mosi_s__0__HSIOM_SPI)
#endif /* (QR_SCANNER_SPI_SLAVE_MOSI_PIN) */

#if (QR_SCANNER_SPI_SLAVE_MISO_PIN)
    #define QR_SCANNER_MISO_S_HSIOM_REG   (*(reg32 *) QR_SCANNER_miso_s__0__HSIOM)
    #define QR_SCANNER_MISO_S_HSIOM_PTR   ( (reg32 *) QR_SCANNER_miso_s__0__HSIOM)
    
    #define QR_SCANNER_MISO_S_HSIOM_MASK      (QR_SCANNER_miso_s__0__HSIOM_MASK)
    #define QR_SCANNER_MISO_S_HSIOM_POS       (QR_SCANNER_miso_s__0__HSIOM_SHIFT)
    #define QR_SCANNER_MISO_S_HSIOM_SEL_GPIO  (QR_SCANNER_miso_s__0__HSIOM_GPIO)
    #define QR_SCANNER_MISO_S_HSIOM_SEL_SPI   (QR_SCANNER_miso_s__0__HSIOM_SPI)
#endif /* (QR_SCANNER_SPI_SLAVE_MISO_PIN) */

#if (QR_SCANNER_SPI_MASTER_MISO_PIN)
    #define QR_SCANNER_MISO_M_HSIOM_REG   (*(reg32 *) QR_SCANNER_miso_m__0__HSIOM)
    #define QR_SCANNER_MISO_M_HSIOM_PTR   ( (reg32 *) QR_SCANNER_miso_m__0__HSIOM)
    
    #define QR_SCANNER_MISO_M_HSIOM_MASK      (QR_SCANNER_miso_m__0__HSIOM_MASK)
    #define QR_SCANNER_MISO_M_HSIOM_POS       (QR_SCANNER_miso_m__0__HSIOM_SHIFT)
    #define QR_SCANNER_MISO_M_HSIOM_SEL_GPIO  (QR_SCANNER_miso_m__0__HSIOM_GPIO)
    #define QR_SCANNER_MISO_M_HSIOM_SEL_SPI   (QR_SCANNER_miso_m__0__HSIOM_SPI)
#endif /* (QR_SCANNER_SPI_MASTER_MISO_PIN) */

#if (QR_SCANNER_SPI_MASTER_MOSI_PIN)
    #define QR_SCANNER_MOSI_M_HSIOM_REG   (*(reg32 *) QR_SCANNER_mosi_m__0__HSIOM)
    #define QR_SCANNER_MOSI_M_HSIOM_PTR   ( (reg32 *) QR_SCANNER_mosi_m__0__HSIOM)
    
    #define QR_SCANNER_MOSI_M_HSIOM_MASK      (QR_SCANNER_mosi_m__0__HSIOM_MASK)
    #define QR_SCANNER_MOSI_M_HSIOM_POS       (QR_SCANNER_mosi_m__0__HSIOM_SHIFT)
    #define QR_SCANNER_MOSI_M_HSIOM_SEL_GPIO  (QR_SCANNER_mosi_m__0__HSIOM_GPIO)
    #define QR_SCANNER_MOSI_M_HSIOM_SEL_SPI   (QR_SCANNER_mosi_m__0__HSIOM_SPI)
#endif /* (QR_SCANNER_SPI_MASTER_MOSI_PIN) */

#if (QR_SCANNER_SPI_MASTER_SCLK_PIN)
    #define QR_SCANNER_SCLK_M_HSIOM_REG   (*(reg32 *) QR_SCANNER_sclk_m__0__HSIOM)
    #define QR_SCANNER_SCLK_M_HSIOM_PTR   ( (reg32 *) QR_SCANNER_sclk_m__0__HSIOM)
    
    #define QR_SCANNER_SCLK_M_HSIOM_MASK      (QR_SCANNER_sclk_m__0__HSIOM_MASK)
    #define QR_SCANNER_SCLK_M_HSIOM_POS       (QR_SCANNER_sclk_m__0__HSIOM_SHIFT)
    #define QR_SCANNER_SCLK_M_HSIOM_SEL_GPIO  (QR_SCANNER_sclk_m__0__HSIOM_GPIO)
    #define QR_SCANNER_SCLK_M_HSIOM_SEL_SPI   (QR_SCANNER_sclk_m__0__HSIOM_SPI)
#endif /* (QR_SCANNER_SPI_MASTER_SCLK_PIN) */

#if (QR_SCANNER_SPI_MASTER_SS0_PIN)
    #define QR_SCANNER_SS0_M_HSIOM_REG    (*(reg32 *) QR_SCANNER_ss0_m__0__HSIOM)
    #define QR_SCANNER_SS0_M_HSIOM_PTR    ( (reg32 *) QR_SCANNER_ss0_m__0__HSIOM)
    
    #define QR_SCANNER_SS0_M_HSIOM_MASK       (QR_SCANNER_ss0_m__0__HSIOM_MASK)
    #define QR_SCANNER_SS0_M_HSIOM_POS        (QR_SCANNER_ss0_m__0__HSIOM_SHIFT)
    #define QR_SCANNER_SS0_M_HSIOM_SEL_GPIO   (QR_SCANNER_ss0_m__0__HSIOM_GPIO)
    #define QR_SCANNER_SS0_M_HSIOM_SEL_SPI    (QR_SCANNER_ss0_m__0__HSIOM_SPI)
#endif /* (QR_SCANNER_SPI_MASTER_SS0_PIN) */

#if (QR_SCANNER_SPI_MASTER_SS1_PIN)
    #define QR_SCANNER_SS1_M_HSIOM_REG    (*(reg32 *) QR_SCANNER_ss1_m__0__HSIOM)
    #define QR_SCANNER_SS1_M_HSIOM_PTR    ( (reg32 *) QR_SCANNER_ss1_m__0__HSIOM)
    
    #define QR_SCANNER_SS1_M_HSIOM_MASK       (QR_SCANNER_ss1_m__0__HSIOM_MASK)
    #define QR_SCANNER_SS1_M_HSIOM_POS        (QR_SCANNER_ss1_m__0__HSIOM_SHIFT)
    #define QR_SCANNER_SS1_M_HSIOM_SEL_GPIO   (QR_SCANNER_ss1_m__0__HSIOM_GPIO)
    #define QR_SCANNER_SS1_M_HSIOM_SEL_SPI    (QR_SCANNER_ss1_m__0__HSIOM_SPI)
#endif /* (QR_SCANNER_SPI_MASTER_SS1_PIN) */

#if (QR_SCANNER_SPI_MASTER_SS2_PIN)
    #define QR_SCANNER_SS2_M_HSIOM_REG    (*(reg32 *) QR_SCANNER_ss2_m__0__HSIOM)
    #define QR_SCANNER_SS2_M_HSIOM_PTR    ( (reg32 *) QR_SCANNER_ss2_m__0__HSIOM)
    
    #define QR_SCANNER_SS2_M_HSIOM_MASK       (QR_SCANNER_ss2_m__0__HSIOM_MASK)
    #define QR_SCANNER_SS2_M_HSIOM_POS        (QR_SCANNER_ss2_m__0__HSIOM_SHIFT)
    #define QR_SCANNER_SS2_M_HSIOM_SEL_GPIO   (QR_SCANNER_ss2_m__0__HSIOM_GPIO)
    #define QR_SCANNER_SS2_M_HSIOM_SEL_SPI    (QR_SCANNER_ss2_m__0__HSIOM_SPI)
#endif /* (QR_SCANNER_SPI_MASTER_SS2_PIN) */

#if (QR_SCANNER_SPI_MASTER_SS3_PIN)
    #define QR_SCANNER_SS3_M_HSIOM_REG    (*(reg32 *) QR_SCANNER_ss3_m__0__HSIOM)
    #define QR_SCANNER_SS3_M_HSIOM_PTR    ( (reg32 *) QR_SCANNER_ss3_m__0__HSIOM)
    
    #define QR_SCANNER_SS3_M_HSIOM_MASK      (QR_SCANNER_ss3_m__0__HSIOM_MASK)
    #define QR_SCANNER_SS3_M_HSIOM_POS       (QR_SCANNER_ss3_m__0__HSIOM_SHIFT)
    #define QR_SCANNER_SS3_M_HSIOM_SEL_GPIO  (QR_SCANNER_ss3_m__0__HSIOM_GPIO)
    #define QR_SCANNER_SS3_M_HSIOM_SEL_SPI   (QR_SCANNER_ss3_m__0__HSIOM_SPI)
#endif /* (QR_SCANNER_SPI_MASTER_SS3_PIN) */

#if (QR_SCANNER_UART_RX_PIN)
    #define QR_SCANNER_RX_HSIOM_REG   (*(reg32 *) QR_SCANNER_rx__0__HSIOM)
    #define QR_SCANNER_RX_HSIOM_PTR   ( (reg32 *) QR_SCANNER_rx__0__HSIOM)
    
    #define QR_SCANNER_RX_HSIOM_MASK      (QR_SCANNER_rx__0__HSIOM_MASK)
    #define QR_SCANNER_RX_HSIOM_POS       (QR_SCANNER_rx__0__HSIOM_SHIFT)
    #define QR_SCANNER_RX_HSIOM_SEL_GPIO  (QR_SCANNER_rx__0__HSIOM_GPIO)
    #define QR_SCANNER_RX_HSIOM_SEL_UART  (QR_SCANNER_rx__0__HSIOM_UART)
#endif /* (QR_SCANNER_UART_RX_PIN) */

#if (QR_SCANNER_UART_RX_WAKE_PIN)
    #define QR_SCANNER_RX_WAKE_HSIOM_REG   (*(reg32 *) QR_SCANNER_rx_wake__0__HSIOM)
    #define QR_SCANNER_RX_WAKE_HSIOM_PTR   ( (reg32 *) QR_SCANNER_rx_wake__0__HSIOM)
    
    #define QR_SCANNER_RX_WAKE_HSIOM_MASK      (QR_SCANNER_rx_wake__0__HSIOM_MASK)
    #define QR_SCANNER_RX_WAKE_HSIOM_POS       (QR_SCANNER_rx_wake__0__HSIOM_SHIFT)
    #define QR_SCANNER_RX_WAKE_HSIOM_SEL_GPIO  (QR_SCANNER_rx_wake__0__HSIOM_GPIO)
    #define QR_SCANNER_RX_WAKE_HSIOM_SEL_UART  (QR_SCANNER_rx_wake__0__HSIOM_UART)
#endif /* (QR_SCANNER_UART_WAKE_RX_PIN) */

#if (QR_SCANNER_UART_CTS_PIN)
    #define QR_SCANNER_CTS_HSIOM_REG   (*(reg32 *) QR_SCANNER_cts__0__HSIOM)
    #define QR_SCANNER_CTS_HSIOM_PTR   ( (reg32 *) QR_SCANNER_cts__0__HSIOM)
    
    #define QR_SCANNER_CTS_HSIOM_MASK      (QR_SCANNER_cts__0__HSIOM_MASK)
    #define QR_SCANNER_CTS_HSIOM_POS       (QR_SCANNER_cts__0__HSIOM_SHIFT)
    #define QR_SCANNER_CTS_HSIOM_SEL_GPIO  (QR_SCANNER_cts__0__HSIOM_GPIO)
    #define QR_SCANNER_CTS_HSIOM_SEL_UART  (QR_SCANNER_cts__0__HSIOM_UART)
#endif /* (QR_SCANNER_UART_CTS_PIN) */

#if (QR_SCANNER_UART_TX_PIN)
    #define QR_SCANNER_TX_HSIOM_REG   (*(reg32 *) QR_SCANNER_tx__0__HSIOM)
    #define QR_SCANNER_TX_HSIOM_PTR   ( (reg32 *) QR_SCANNER_tx__0__HSIOM)
    
    #define QR_SCANNER_TX_HSIOM_MASK      (QR_SCANNER_tx__0__HSIOM_MASK)
    #define QR_SCANNER_TX_HSIOM_POS       (QR_SCANNER_tx__0__HSIOM_SHIFT)
    #define QR_SCANNER_TX_HSIOM_SEL_GPIO  (QR_SCANNER_tx__0__HSIOM_GPIO)
    #define QR_SCANNER_TX_HSIOM_SEL_UART  (QR_SCANNER_tx__0__HSIOM_UART)
#endif /* (QR_SCANNER_UART_TX_PIN) */

#if (QR_SCANNER_UART_RX_TX_PIN)
    #define QR_SCANNER_RX_TX_HSIOM_REG   (*(reg32 *) QR_SCANNER_rx_tx__0__HSIOM)
    #define QR_SCANNER_RX_TX_HSIOM_PTR   ( (reg32 *) QR_SCANNER_rx_tx__0__HSIOM)
    
    #define QR_SCANNER_RX_TX_HSIOM_MASK      (QR_SCANNER_rx_tx__0__HSIOM_MASK)
    #define QR_SCANNER_RX_TX_HSIOM_POS       (QR_SCANNER_rx_tx__0__HSIOM_SHIFT)
    #define QR_SCANNER_RX_TX_HSIOM_SEL_GPIO  (QR_SCANNER_rx_tx__0__HSIOM_GPIO)
    #define QR_SCANNER_RX_TX_HSIOM_SEL_UART  (QR_SCANNER_rx_tx__0__HSIOM_UART)
#endif /* (QR_SCANNER_UART_RX_TX_PIN) */

#if (QR_SCANNER_UART_RTS_PIN)
    #define QR_SCANNER_RTS_HSIOM_REG      (*(reg32 *) QR_SCANNER_rts__0__HSIOM)
    #define QR_SCANNER_RTS_HSIOM_PTR      ( (reg32 *) QR_SCANNER_rts__0__HSIOM)
    
    #define QR_SCANNER_RTS_HSIOM_MASK     (QR_SCANNER_rts__0__HSIOM_MASK)
    #define QR_SCANNER_RTS_HSIOM_POS      (QR_SCANNER_rts__0__HSIOM_SHIFT)    
    #define QR_SCANNER_RTS_HSIOM_SEL_GPIO (QR_SCANNER_rts__0__HSIOM_GPIO)
    #define QR_SCANNER_RTS_HSIOM_SEL_UART (QR_SCANNER_rts__0__HSIOM_UART)    
#endif /* (QR_SCANNER_UART_RTS_PIN) */


/***************************************
*        Registers Constants
***************************************/

/* HSIOM switch values. */ 
#define QR_SCANNER_HSIOM_DEF_SEL      (0x00u)
#define QR_SCANNER_HSIOM_GPIO_SEL     (0x00u)
/* The HSIOM values provided below are valid only for QR_SCANNER_CY_SCBIP_V0 
* and QR_SCANNER_CY_SCBIP_V1. It is not recommended to use them for 
* QR_SCANNER_CY_SCBIP_V2. Use pin name specific HSIOM constants provided 
* above instead for any SCB IP block version.
*/
#define QR_SCANNER_HSIOM_UART_SEL     (0x09u)
#define QR_SCANNER_HSIOM_I2C_SEL      (0x0Eu)
#define QR_SCANNER_HSIOM_SPI_SEL      (0x0Fu)

/* Pins settings index. */
#define QR_SCANNER_RX_WAKE_SCL_MOSI_PIN_INDEX   (0u)
#define QR_SCANNER_RX_SCL_MOSI_PIN_INDEX       (0u)
#define QR_SCANNER_TX_SDA_MISO_PIN_INDEX       (1u)
#define QR_SCANNER_CTS_SCLK_PIN_INDEX       (2u)
#define QR_SCANNER_RTS_SS0_PIN_INDEX       (3u)
#define QR_SCANNER_SS1_PIN_INDEX                  (4u)
#define QR_SCANNER_SS2_PIN_INDEX                  (5u)
#define QR_SCANNER_SS3_PIN_INDEX                  (6u)

/* Pins settings mask. */
#define QR_SCANNER_RX_WAKE_SCL_MOSI_PIN_MASK ((uint32) 0x01u << QR_SCANNER_RX_WAKE_SCL_MOSI_PIN_INDEX)
#define QR_SCANNER_RX_SCL_MOSI_PIN_MASK     ((uint32) 0x01u << QR_SCANNER_RX_SCL_MOSI_PIN_INDEX)
#define QR_SCANNER_TX_SDA_MISO_PIN_MASK     ((uint32) 0x01u << QR_SCANNER_TX_SDA_MISO_PIN_INDEX)
#define QR_SCANNER_CTS_SCLK_PIN_MASK     ((uint32) 0x01u << QR_SCANNER_CTS_SCLK_PIN_INDEX)
#define QR_SCANNER_RTS_SS0_PIN_MASK     ((uint32) 0x01u << QR_SCANNER_RTS_SS0_PIN_INDEX)
#define QR_SCANNER_SS1_PIN_MASK                ((uint32) 0x01u << QR_SCANNER_SS1_PIN_INDEX)
#define QR_SCANNER_SS2_PIN_MASK                ((uint32) 0x01u << QR_SCANNER_SS2_PIN_INDEX)
#define QR_SCANNER_SS3_PIN_MASK                ((uint32) 0x01u << QR_SCANNER_SS3_PIN_INDEX)

/* Pin interrupt constants. */
#define QR_SCANNER_INTCFG_TYPE_MASK           (0x03u)
#define QR_SCANNER_INTCFG_TYPE_FALLING_EDGE   (0x02u)

/* Pin Drive Mode constants. */
#define QR_SCANNER_PIN_DM_ALG_HIZ  (0u)
#define QR_SCANNER_PIN_DM_DIG_HIZ  (1u)
#define QR_SCANNER_PIN_DM_OD_LO    (4u)
#define QR_SCANNER_PIN_DM_STRONG   (6u)


/***************************************
*          Macro Definitions
***************************************/

/* Return drive mode of the pin */
#define QR_SCANNER_DM_MASK    (0x7u)
#define QR_SCANNER_DM_SIZE    (3u)
#define QR_SCANNER_GET_P4_PIN_DM(reg, pos) \
    ( ((reg) & (uint32) ((uint32) QR_SCANNER_DM_MASK << (QR_SCANNER_DM_SIZE * (pos)))) >> \
                                                              (QR_SCANNER_DM_SIZE * (pos)) )

#if (QR_SCANNER_TX_SDA_MISO_PIN)
    #define QR_SCANNER_CHECK_TX_SDA_MISO_PIN_USED \
                (QR_SCANNER_PIN_DM_ALG_HIZ != \
                    QR_SCANNER_GET_P4_PIN_DM(QR_SCANNER_uart_tx_i2c_sda_spi_miso_PC, \
                                                   QR_SCANNER_uart_tx_i2c_sda_spi_miso_SHIFT))
#endif /* (QR_SCANNER_TX_SDA_MISO_PIN) */

#if (QR_SCANNER_RTS_SS0_PIN)
    #define QR_SCANNER_CHECK_RTS_SS0_PIN_USED \
                (QR_SCANNER_PIN_DM_ALG_HIZ != \
                    QR_SCANNER_GET_P4_PIN_DM(QR_SCANNER_uart_rts_spi_ss0_PC, \
                                                   QR_SCANNER_uart_rts_spi_ss0_SHIFT))
#endif /* (QR_SCANNER_RTS_SS0_PIN) */

/* Set bits-mask in register */
#define QR_SCANNER_SET_REGISTER_BITS(reg, mask, pos, mode) \
                    do                                           \
                    {                                            \
                        (reg) = (((reg) & ((uint32) ~(uint32) (mask))) | ((uint32) ((uint32) (mode) << (pos)))); \
                    }while(0)

/* Set bit in the register */
#define QR_SCANNER_SET_REGISTER_BIT(reg, mask, val) \
                    ((val) ? ((reg) |= (mask)) : ((reg) &= ((uint32) ~((uint32) (mask)))))

#define QR_SCANNER_SET_HSIOM_SEL(reg, mask, pos, sel) QR_SCANNER_SET_REGISTER_BITS(reg, mask, pos, sel)
#define QR_SCANNER_SET_INCFG_TYPE(reg, mask, pos, intType) \
                                                        QR_SCANNER_SET_REGISTER_BITS(reg, mask, pos, intType)
#define QR_SCANNER_SET_INP_DIS(reg, mask, val) QR_SCANNER_SET_REGISTER_BIT(reg, mask, val)

/* QR_SCANNER_SET_I2C_SCL_DR(val) - Sets I2C SCL DR register.
*  QR_SCANNER_SET_I2C_SCL_HSIOM_SEL(sel) - Sets I2C SCL HSIOM settings.
*/
/* SCB I2C: scl signal */
#if (QR_SCANNER_CY_SCBIP_V0)
#if (QR_SCANNER_I2C_PINS)
    #define QR_SCANNER_SET_I2C_SCL_DR(val) QR_SCANNER_scl_Write(val)

    #define QR_SCANNER_SET_I2C_SCL_HSIOM_SEL(sel) \
                          QR_SCANNER_SET_HSIOM_SEL(QR_SCANNER_SCL_HSIOM_REG,  \
                                                         QR_SCANNER_SCL_HSIOM_MASK, \
                                                         QR_SCANNER_SCL_HSIOM_POS,  \
                                                         (sel))
    #define QR_SCANNER_WAIT_SCL_SET_HIGH  (0u == QR_SCANNER_scl_Read())

/* Unconfigured SCB: scl signal */
#elif (QR_SCANNER_RX_WAKE_SCL_MOSI_PIN)
    #define QR_SCANNER_SET_I2C_SCL_DR(val) \
                            QR_SCANNER_uart_rx_wake_i2c_scl_spi_mosi_Write(val)

    #define QR_SCANNER_SET_I2C_SCL_HSIOM_SEL(sel) \
                    QR_SCANNER_SET_HSIOM_SEL(QR_SCANNER_RX_WAKE_SCL_MOSI_HSIOM_REG,  \
                                                   QR_SCANNER_RX_WAKE_SCL_MOSI_HSIOM_MASK, \
                                                   QR_SCANNER_RX_WAKE_SCL_MOSI_HSIOM_POS,  \
                                                   (sel))

    #define QR_SCANNER_WAIT_SCL_SET_HIGH  (0u == QR_SCANNER_uart_rx_wake_i2c_scl_spi_mosi_Read())

#elif (QR_SCANNER_RX_SCL_MOSI_PIN)
    #define QR_SCANNER_SET_I2C_SCL_DR(val) \
                            QR_SCANNER_uart_rx_i2c_scl_spi_mosi_Write(val)


    #define QR_SCANNER_SET_I2C_SCL_HSIOM_SEL(sel) \
                            QR_SCANNER_SET_HSIOM_SEL(QR_SCANNER_RX_SCL_MOSI_HSIOM_REG,  \
                                                           QR_SCANNER_RX_SCL_MOSI_HSIOM_MASK, \
                                                           QR_SCANNER_RX_SCL_MOSI_HSIOM_POS,  \
                                                           (sel))

    #define QR_SCANNER_WAIT_SCL_SET_HIGH  (0u == QR_SCANNER_uart_rx_i2c_scl_spi_mosi_Read())

#else
    #define QR_SCANNER_SET_I2C_SCL_DR(val)        do{ /* Does nothing */ }while(0)
    #define QR_SCANNER_SET_I2C_SCL_HSIOM_SEL(sel) do{ /* Does nothing */ }while(0)

    #define QR_SCANNER_WAIT_SCL_SET_HIGH  (0u)
#endif /* (QR_SCANNER_I2C_PINS) */

/* SCB I2C: sda signal */
#if (QR_SCANNER_I2C_PINS)
    #define QR_SCANNER_WAIT_SDA_SET_HIGH  (0u == QR_SCANNER_sda_Read())
/* Unconfigured SCB: sda signal */
#elif (QR_SCANNER_TX_SDA_MISO_PIN)
    #define QR_SCANNER_WAIT_SDA_SET_HIGH  (0u == QR_SCANNER_uart_tx_i2c_sda_spi_miso_Read())
#else
    #define QR_SCANNER_WAIT_SDA_SET_HIGH  (0u)
#endif /* (QR_SCANNER_MOSI_SCL_RX_PIN) */
#endif /* (QR_SCANNER_CY_SCBIP_V0) */

/* Clear UART wakeup source */
#if (QR_SCANNER_RX_SCL_MOSI_PIN)
    #define QR_SCANNER_CLEAR_UART_RX_WAKE_INTR        do{ /* Does nothing */ }while(0)
    
#elif (QR_SCANNER_RX_WAKE_SCL_MOSI_PIN)
    #define QR_SCANNER_CLEAR_UART_RX_WAKE_INTR \
            do{                                      \
                (void) QR_SCANNER_uart_rx_wake_i2c_scl_spi_mosi_ClearInterrupt(); \
            }while(0)

#elif(QR_SCANNER_UART_RX_WAKE_PIN)
    #define QR_SCANNER_CLEAR_UART_RX_WAKE_INTR \
            do{                                      \
                (void) QR_SCANNER_rx_wake_ClearInterrupt(); \
            }while(0)
#else
#endif /* (QR_SCANNER_RX_SCL_MOSI_PIN) */


/***************************************
* The following code is DEPRECATED and
* must not be used.
***************************************/

/* Unconfigured pins */
#define QR_SCANNER_REMOVE_MOSI_SCL_RX_WAKE_PIN    QR_SCANNER_REMOVE_RX_WAKE_SCL_MOSI_PIN
#define QR_SCANNER_REMOVE_MOSI_SCL_RX_PIN         QR_SCANNER_REMOVE_RX_SCL_MOSI_PIN
#define QR_SCANNER_REMOVE_MISO_SDA_TX_PIN         QR_SCANNER_REMOVE_TX_SDA_MISO_PIN
#ifndef QR_SCANNER_REMOVE_SCLK_PIN
#define QR_SCANNER_REMOVE_SCLK_PIN                QR_SCANNER_REMOVE_CTS_SCLK_PIN
#endif /* QR_SCANNER_REMOVE_SCLK_PIN */
#ifndef QR_SCANNER_REMOVE_SS0_PIN
#define QR_SCANNER_REMOVE_SS0_PIN                 QR_SCANNER_REMOVE_RTS_SS0_PIN
#endif /* QR_SCANNER_REMOVE_SS0_PIN */

/* Unconfigured pins */
#define QR_SCANNER_MOSI_SCL_RX_WAKE_PIN   QR_SCANNER_RX_WAKE_SCL_MOSI_PIN
#define QR_SCANNER_MOSI_SCL_RX_PIN        QR_SCANNER_RX_SCL_MOSI_PIN
#define QR_SCANNER_MISO_SDA_TX_PIN        QR_SCANNER_TX_SDA_MISO_PIN
#ifndef QR_SCANNER_SCLK_PIN
#define QR_SCANNER_SCLK_PIN               QR_SCANNER_CTS_SCLK_PIN
#endif /* QR_SCANNER_SCLK_PIN */
#ifndef QR_SCANNER_SS0_PIN
#define QR_SCANNER_SS0_PIN                QR_SCANNER_RTS_SS0_PIN
#endif /* QR_SCANNER_SS0_PIN */

#if (QR_SCANNER_MOSI_SCL_RX_WAKE_PIN)
    #define QR_SCANNER_MOSI_SCL_RX_WAKE_HSIOM_REG     QR_SCANNER_RX_WAKE_SCL_MOSI_HSIOM_REG
    #define QR_SCANNER_MOSI_SCL_RX_WAKE_HSIOM_PTR     QR_SCANNER_RX_WAKE_SCL_MOSI_HSIOM_REG
    #define QR_SCANNER_MOSI_SCL_RX_WAKE_HSIOM_MASK    QR_SCANNER_RX_WAKE_SCL_MOSI_HSIOM_REG
    #define QR_SCANNER_MOSI_SCL_RX_WAKE_HSIOM_POS     QR_SCANNER_RX_WAKE_SCL_MOSI_HSIOM_REG

    #define QR_SCANNER_MOSI_SCL_RX_WAKE_INTCFG_REG    QR_SCANNER_RX_WAKE_SCL_MOSI_HSIOM_REG
    #define QR_SCANNER_MOSI_SCL_RX_WAKE_INTCFG_PTR    QR_SCANNER_RX_WAKE_SCL_MOSI_HSIOM_REG

    #define QR_SCANNER_MOSI_SCL_RX_WAKE_INTCFG_TYPE_POS   QR_SCANNER_RX_WAKE_SCL_MOSI_HSIOM_REG
    #define QR_SCANNER_MOSI_SCL_RX_WAKE_INTCFG_TYPE_MASK  QR_SCANNER_RX_WAKE_SCL_MOSI_HSIOM_REG
#endif /* (QR_SCANNER_RX_WAKE_SCL_MOSI_PIN) */

#if (QR_SCANNER_MOSI_SCL_RX_PIN)
    #define QR_SCANNER_MOSI_SCL_RX_HSIOM_REG      QR_SCANNER_RX_SCL_MOSI_HSIOM_REG
    #define QR_SCANNER_MOSI_SCL_RX_HSIOM_PTR      QR_SCANNER_RX_SCL_MOSI_HSIOM_PTR
    #define QR_SCANNER_MOSI_SCL_RX_HSIOM_MASK     QR_SCANNER_RX_SCL_MOSI_HSIOM_MASK
    #define QR_SCANNER_MOSI_SCL_RX_HSIOM_POS      QR_SCANNER_RX_SCL_MOSI_HSIOM_POS
#endif /* (QR_SCANNER_MOSI_SCL_RX_PIN) */

#if (QR_SCANNER_MISO_SDA_TX_PIN)
    #define QR_SCANNER_MISO_SDA_TX_HSIOM_REG      QR_SCANNER_TX_SDA_MISO_HSIOM_REG
    #define QR_SCANNER_MISO_SDA_TX_HSIOM_PTR      QR_SCANNER_TX_SDA_MISO_HSIOM_REG
    #define QR_SCANNER_MISO_SDA_TX_HSIOM_MASK     QR_SCANNER_TX_SDA_MISO_HSIOM_REG
    #define QR_SCANNER_MISO_SDA_TX_HSIOM_POS      QR_SCANNER_TX_SDA_MISO_HSIOM_REG
#endif /* (QR_SCANNER_MISO_SDA_TX_PIN_PIN) */

#if (QR_SCANNER_SCLK_PIN)
    #ifndef QR_SCANNER_SCLK_HSIOM_REG
    #define QR_SCANNER_SCLK_HSIOM_REG     QR_SCANNER_CTS_SCLK_HSIOM_REG
    #define QR_SCANNER_SCLK_HSIOM_PTR     QR_SCANNER_CTS_SCLK_HSIOM_PTR
    #define QR_SCANNER_SCLK_HSIOM_MASK    QR_SCANNER_CTS_SCLK_HSIOM_MASK
    #define QR_SCANNER_SCLK_HSIOM_POS     QR_SCANNER_CTS_SCLK_HSIOM_POS
    #endif /* QR_SCANNER_SCLK_HSIOM_REG */
#endif /* (QR_SCANNER_SCLK_PIN) */

#if (QR_SCANNER_SS0_PIN)
    #ifndef QR_SCANNER_SS0_HSIOM_REG
    #define QR_SCANNER_SS0_HSIOM_REG      QR_SCANNER_RTS_SS0_HSIOM_REG
    #define QR_SCANNER_SS0_HSIOM_PTR      QR_SCANNER_RTS_SS0_HSIOM_PTR
    #define QR_SCANNER_SS0_HSIOM_MASK     QR_SCANNER_RTS_SS0_HSIOM_MASK
    #define QR_SCANNER_SS0_HSIOM_POS      QR_SCANNER_RTS_SS0_HSIOM_POS
    #endif /* QR_SCANNER_SS0_HSIOM_REG */
#endif /* (QR_SCANNER_SS0_PIN) */

#define QR_SCANNER_MOSI_SCL_RX_WAKE_PIN_INDEX QR_SCANNER_RX_WAKE_SCL_MOSI_PIN_INDEX
#define QR_SCANNER_MOSI_SCL_RX_PIN_INDEX      QR_SCANNER_RX_SCL_MOSI_PIN_INDEX
#define QR_SCANNER_MISO_SDA_TX_PIN_INDEX      QR_SCANNER_TX_SDA_MISO_PIN_INDEX
#ifndef QR_SCANNER_SCLK_PIN_INDEX
#define QR_SCANNER_SCLK_PIN_INDEX             QR_SCANNER_CTS_SCLK_PIN_INDEX
#endif /* QR_SCANNER_SCLK_PIN_INDEX */
#ifndef QR_SCANNER_SS0_PIN_INDEX
#define QR_SCANNER_SS0_PIN_INDEX              QR_SCANNER_RTS_SS0_PIN_INDEX
#endif /* QR_SCANNER_SS0_PIN_INDEX */

#define QR_SCANNER_MOSI_SCL_RX_WAKE_PIN_MASK QR_SCANNER_RX_WAKE_SCL_MOSI_PIN_MASK
#define QR_SCANNER_MOSI_SCL_RX_PIN_MASK      QR_SCANNER_RX_SCL_MOSI_PIN_MASK
#define QR_SCANNER_MISO_SDA_TX_PIN_MASK      QR_SCANNER_TX_SDA_MISO_PIN_MASK
#ifndef QR_SCANNER_SCLK_PIN_MASK
#define QR_SCANNER_SCLK_PIN_MASK             QR_SCANNER_CTS_SCLK_PIN_MASK
#endif /* QR_SCANNER_SCLK_PIN_MASK */
#ifndef QR_SCANNER_SS0_PIN_MASK
#define QR_SCANNER_SS0_PIN_MASK              QR_SCANNER_RTS_SS0_PIN_MASK
#endif /* QR_SCANNER_SS0_PIN_MASK */

#endif /* (CY_SCB_PINS_QR_SCANNER_H) */


/* [] END OF FILE */
