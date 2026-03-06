/***************************************************************************//**
* \file port_MONITORING_PINS.h
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

#if !defined(CY_SCB_PINS_port_MONITORING_H)
#define CY_SCB_PINS_port_MONITORING_H

#include "cydevice_trm.h"
#include "cyfitter.h"
#include "cytypes.h"


/***************************************
*   Conditional Compilation Parameters
****************************************/

/* Unconfigured pins */
#define port_MONITORING_REMOVE_RX_WAKE_SCL_MOSI_PIN  (1u)
#define port_MONITORING_REMOVE_RX_SCL_MOSI_PIN      (1u)
#define port_MONITORING_REMOVE_TX_SDA_MISO_PIN      (1u)
#define port_MONITORING_REMOVE_CTS_SCLK_PIN      (1u)
#define port_MONITORING_REMOVE_RTS_SS0_PIN      (1u)
#define port_MONITORING_REMOVE_SS1_PIN                 (1u)
#define port_MONITORING_REMOVE_SS2_PIN                 (1u)
#define port_MONITORING_REMOVE_SS3_PIN                 (1u)

/* Mode defined pins */
#define port_MONITORING_REMOVE_I2C_PINS                (1u)
#define port_MONITORING_REMOVE_SPI_MASTER_PINS         (1u)
#define port_MONITORING_REMOVE_SPI_MASTER_SCLK_PIN     (1u)
#define port_MONITORING_REMOVE_SPI_MASTER_MOSI_PIN     (1u)
#define port_MONITORING_REMOVE_SPI_MASTER_MISO_PIN     (1u)
#define port_MONITORING_REMOVE_SPI_MASTER_SS0_PIN      (1u)
#define port_MONITORING_REMOVE_SPI_MASTER_SS1_PIN      (1u)
#define port_MONITORING_REMOVE_SPI_MASTER_SS2_PIN      (1u)
#define port_MONITORING_REMOVE_SPI_MASTER_SS3_PIN      (1u)
#define port_MONITORING_REMOVE_SPI_SLAVE_PINS          (1u)
#define port_MONITORING_REMOVE_SPI_SLAVE_MOSI_PIN      (1u)
#define port_MONITORING_REMOVE_SPI_SLAVE_MISO_PIN      (1u)
#define port_MONITORING_REMOVE_UART_TX_PIN             (0u)
#define port_MONITORING_REMOVE_UART_RX_TX_PIN          (1u)
#define port_MONITORING_REMOVE_UART_RX_PIN             (0u)
#define port_MONITORING_REMOVE_UART_RX_WAKE_PIN        (1u)
#define port_MONITORING_REMOVE_UART_RTS_PIN            (1u)
#define port_MONITORING_REMOVE_UART_CTS_PIN            (1u)

/* Unconfigured pins */
#define port_MONITORING_RX_WAKE_SCL_MOSI_PIN (0u == port_MONITORING_REMOVE_RX_WAKE_SCL_MOSI_PIN)
#define port_MONITORING_RX_SCL_MOSI_PIN     (0u == port_MONITORING_REMOVE_RX_SCL_MOSI_PIN)
#define port_MONITORING_TX_SDA_MISO_PIN     (0u == port_MONITORING_REMOVE_TX_SDA_MISO_PIN)
#define port_MONITORING_CTS_SCLK_PIN     (0u == port_MONITORING_REMOVE_CTS_SCLK_PIN)
#define port_MONITORING_RTS_SS0_PIN     (0u == port_MONITORING_REMOVE_RTS_SS0_PIN)
#define port_MONITORING_SS1_PIN                (0u == port_MONITORING_REMOVE_SS1_PIN)
#define port_MONITORING_SS2_PIN                (0u == port_MONITORING_REMOVE_SS2_PIN)
#define port_MONITORING_SS3_PIN                (0u == port_MONITORING_REMOVE_SS3_PIN)

/* Mode defined pins */
#define port_MONITORING_I2C_PINS               (0u == port_MONITORING_REMOVE_I2C_PINS)
#define port_MONITORING_SPI_MASTER_PINS        (0u == port_MONITORING_REMOVE_SPI_MASTER_PINS)
#define port_MONITORING_SPI_MASTER_SCLK_PIN    (0u == port_MONITORING_REMOVE_SPI_MASTER_SCLK_PIN)
#define port_MONITORING_SPI_MASTER_MOSI_PIN    (0u == port_MONITORING_REMOVE_SPI_MASTER_MOSI_PIN)
#define port_MONITORING_SPI_MASTER_MISO_PIN    (0u == port_MONITORING_REMOVE_SPI_MASTER_MISO_PIN)
#define port_MONITORING_SPI_MASTER_SS0_PIN     (0u == port_MONITORING_REMOVE_SPI_MASTER_SS0_PIN)
#define port_MONITORING_SPI_MASTER_SS1_PIN     (0u == port_MONITORING_REMOVE_SPI_MASTER_SS1_PIN)
#define port_MONITORING_SPI_MASTER_SS2_PIN     (0u == port_MONITORING_REMOVE_SPI_MASTER_SS2_PIN)
#define port_MONITORING_SPI_MASTER_SS3_PIN     (0u == port_MONITORING_REMOVE_SPI_MASTER_SS3_PIN)
#define port_MONITORING_SPI_SLAVE_PINS         (0u == port_MONITORING_REMOVE_SPI_SLAVE_PINS)
#define port_MONITORING_SPI_SLAVE_MOSI_PIN     (0u == port_MONITORING_REMOVE_SPI_SLAVE_MOSI_PIN)
#define port_MONITORING_SPI_SLAVE_MISO_PIN     (0u == port_MONITORING_REMOVE_SPI_SLAVE_MISO_PIN)
#define port_MONITORING_UART_TX_PIN            (0u == port_MONITORING_REMOVE_UART_TX_PIN)
#define port_MONITORING_UART_RX_TX_PIN         (0u == port_MONITORING_REMOVE_UART_RX_TX_PIN)
#define port_MONITORING_UART_RX_PIN            (0u == port_MONITORING_REMOVE_UART_RX_PIN)
#define port_MONITORING_UART_RX_WAKE_PIN       (0u == port_MONITORING_REMOVE_UART_RX_WAKE_PIN)
#define port_MONITORING_UART_RTS_PIN           (0u == port_MONITORING_REMOVE_UART_RTS_PIN)
#define port_MONITORING_UART_CTS_PIN           (0u == port_MONITORING_REMOVE_UART_CTS_PIN)


/***************************************
*             Includes
****************************************/

#if (port_MONITORING_RX_WAKE_SCL_MOSI_PIN)
    #include "port_MONITORING_uart_rx_wake_i2c_scl_spi_mosi.h"
#endif /* (port_MONITORING_RX_SCL_MOSI) */

#if (port_MONITORING_RX_SCL_MOSI_PIN)
    #include "port_MONITORING_uart_rx_i2c_scl_spi_mosi.h"
#endif /* (port_MONITORING_RX_SCL_MOSI) */

#if (port_MONITORING_TX_SDA_MISO_PIN)
    #include "port_MONITORING_uart_tx_i2c_sda_spi_miso.h"
#endif /* (port_MONITORING_TX_SDA_MISO) */

#if (port_MONITORING_CTS_SCLK_PIN)
    #include "port_MONITORING_uart_cts_spi_sclk.h"
#endif /* (port_MONITORING_CTS_SCLK) */

#if (port_MONITORING_RTS_SS0_PIN)
    #include "port_MONITORING_uart_rts_spi_ss0.h"
#endif /* (port_MONITORING_RTS_SS0_PIN) */

#if (port_MONITORING_SS1_PIN)
    #include "port_MONITORING_spi_ss1.h"
#endif /* (port_MONITORING_SS1_PIN) */

#if (port_MONITORING_SS2_PIN)
    #include "port_MONITORING_spi_ss2.h"
#endif /* (port_MONITORING_SS2_PIN) */

#if (port_MONITORING_SS3_PIN)
    #include "port_MONITORING_spi_ss3.h"
#endif /* (port_MONITORING_SS3_PIN) */

#if (port_MONITORING_I2C_PINS)
    #include "port_MONITORING_scl.h"
    #include "port_MONITORING_sda.h"
#endif /* (port_MONITORING_I2C_PINS) */

#if (port_MONITORING_SPI_MASTER_PINS)
#if (port_MONITORING_SPI_MASTER_SCLK_PIN)
    #include "port_MONITORING_sclk_m.h"
#endif /* (port_MONITORING_SPI_MASTER_SCLK_PIN) */

#if (port_MONITORING_SPI_MASTER_MOSI_PIN)
    #include "port_MONITORING_mosi_m.h"
#endif /* (port_MONITORING_SPI_MASTER_MOSI_PIN) */

#if (port_MONITORING_SPI_MASTER_MISO_PIN)
    #include "port_MONITORING_miso_m.h"
#endif /*(port_MONITORING_SPI_MASTER_MISO_PIN) */
#endif /* (port_MONITORING_SPI_MASTER_PINS) */

#if (port_MONITORING_SPI_SLAVE_PINS)
    #include "port_MONITORING_sclk_s.h"
    #include "port_MONITORING_ss_s.h"

#if (port_MONITORING_SPI_SLAVE_MOSI_PIN)
    #include "port_MONITORING_mosi_s.h"
#endif /* (port_MONITORING_SPI_SLAVE_MOSI_PIN) */

#if (port_MONITORING_SPI_SLAVE_MISO_PIN)
    #include "port_MONITORING_miso_s.h"
#endif /*(port_MONITORING_SPI_SLAVE_MISO_PIN) */
#endif /* (port_MONITORING_SPI_SLAVE_PINS) */

#if (port_MONITORING_SPI_MASTER_SS0_PIN)
    #include "port_MONITORING_ss0_m.h"
#endif /* (port_MONITORING_SPI_MASTER_SS0_PIN) */

#if (port_MONITORING_SPI_MASTER_SS1_PIN)
    #include "port_MONITORING_ss1_m.h"
#endif /* (port_MONITORING_SPI_MASTER_SS1_PIN) */

#if (port_MONITORING_SPI_MASTER_SS2_PIN)
    #include "port_MONITORING_ss2_m.h"
#endif /* (port_MONITORING_SPI_MASTER_SS2_PIN) */

#if (port_MONITORING_SPI_MASTER_SS3_PIN)
    #include "port_MONITORING_ss3_m.h"
#endif /* (port_MONITORING_SPI_MASTER_SS3_PIN) */

#if (port_MONITORING_UART_TX_PIN)
    #include "port_MONITORING_tx.h"
#endif /* (port_MONITORING_UART_TX_PIN) */

#if (port_MONITORING_UART_RX_TX_PIN)
    #include "port_MONITORING_rx_tx.h"
#endif /* (port_MONITORING_UART_RX_TX_PIN) */

#if (port_MONITORING_UART_RX_PIN)
    #include "port_MONITORING_rx.h"
#endif /* (port_MONITORING_UART_RX_PIN) */

#if (port_MONITORING_UART_RX_WAKE_PIN)
    #include "port_MONITORING_rx_wake.h"
#endif /* (port_MONITORING_UART_RX_WAKE_PIN) */

#if (port_MONITORING_UART_RTS_PIN)
    #include "port_MONITORING_rts.h"
#endif /* (port_MONITORING_UART_RTS_PIN) */

#if (port_MONITORING_UART_CTS_PIN)
    #include "port_MONITORING_cts.h"
#endif /* (port_MONITORING_UART_CTS_PIN) */


/***************************************
*              Registers
***************************************/

#if (port_MONITORING_RX_SCL_MOSI_PIN)
    #define port_MONITORING_RX_SCL_MOSI_HSIOM_REG   (*(reg32 *) port_MONITORING_uart_rx_i2c_scl_spi_mosi__0__HSIOM)
    #define port_MONITORING_RX_SCL_MOSI_HSIOM_PTR   ( (reg32 *) port_MONITORING_uart_rx_i2c_scl_spi_mosi__0__HSIOM)
    
    #define port_MONITORING_RX_SCL_MOSI_HSIOM_MASK      (port_MONITORING_uart_rx_i2c_scl_spi_mosi__0__HSIOM_MASK)
    #define port_MONITORING_RX_SCL_MOSI_HSIOM_POS       (port_MONITORING_uart_rx_i2c_scl_spi_mosi__0__HSIOM_SHIFT)
    #define port_MONITORING_RX_SCL_MOSI_HSIOM_SEL_GPIO  (port_MONITORING_uart_rx_i2c_scl_spi_mosi__0__HSIOM_GPIO)
    #define port_MONITORING_RX_SCL_MOSI_HSIOM_SEL_I2C   (port_MONITORING_uart_rx_i2c_scl_spi_mosi__0__HSIOM_I2C)
    #define port_MONITORING_RX_SCL_MOSI_HSIOM_SEL_SPI   (port_MONITORING_uart_rx_i2c_scl_spi_mosi__0__HSIOM_SPI)
    #define port_MONITORING_RX_SCL_MOSI_HSIOM_SEL_UART  (port_MONITORING_uart_rx_i2c_scl_spi_mosi__0__HSIOM_UART)
    
#elif (port_MONITORING_RX_WAKE_SCL_MOSI_PIN)
    #define port_MONITORING_RX_WAKE_SCL_MOSI_HSIOM_REG   (*(reg32 *) port_MONITORING_uart_rx_wake_i2c_scl_spi_mosi__0__HSIOM)
    #define port_MONITORING_RX_WAKE_SCL_MOSI_HSIOM_PTR   ( (reg32 *) port_MONITORING_uart_rx_wake_i2c_scl_spi_mosi__0__HSIOM)
    
    #define port_MONITORING_RX_WAKE_SCL_MOSI_HSIOM_MASK      (port_MONITORING_uart_rx_wake_i2c_scl_spi_mosi__0__HSIOM_MASK)
    #define port_MONITORING_RX_WAKE_SCL_MOSI_HSIOM_POS       (port_MONITORING_uart_rx_wake_i2c_scl_spi_mosi__0__HSIOM_SHIFT)
    #define port_MONITORING_RX_WAKE_SCL_MOSI_HSIOM_SEL_GPIO  (port_MONITORING_uart_rx_wake_i2c_scl_spi_mosi__0__HSIOM_GPIO)
    #define port_MONITORING_RX_WAKE_SCL_MOSI_HSIOM_SEL_I2C   (port_MONITORING_uart_rx_wake_i2c_scl_spi_mosi__0__HSIOM_I2C)
    #define port_MONITORING_RX_WAKE_SCL_MOSI_HSIOM_SEL_SPI   (port_MONITORING_uart_rx_wake_i2c_scl_spi_mosi__0__HSIOM_SPI)
    #define port_MONITORING_RX_WAKE_SCL_MOSI_HSIOM_SEL_UART  (port_MONITORING_uart_rx_wake_i2c_scl_spi_mosi__0__HSIOM_UART)    
   
    #define port_MONITORING_RX_WAKE_SCL_MOSI_INTCFG_REG (*(reg32 *) port_MONITORING_uart_rx_wake_i2c_scl_spi_mosi__0__INTCFG)
    #define port_MONITORING_RX_WAKE_SCL_MOSI_INTCFG_PTR ( (reg32 *) port_MONITORING_uart_rx_wake_i2c_scl_spi_mosi__0__INTCFG)
    #define port_MONITORING_RX_WAKE_SCL_MOSI_INTCFG_TYPE_POS  (port_MONITORING_uart_rx_wake_i2c_scl_spi_mosi__SHIFT)
    #define port_MONITORING_RX_WAKE_SCL_MOSI_INTCFG_TYPE_MASK ((uint32) port_MONITORING_INTCFG_TYPE_MASK << \
                                                                           port_MONITORING_RX_WAKE_SCL_MOSI_INTCFG_TYPE_POS)
#else
    /* None of pins port_MONITORING_RX_SCL_MOSI_PIN or port_MONITORING_RX_WAKE_SCL_MOSI_PIN present.*/
#endif /* (port_MONITORING_RX_SCL_MOSI_PIN) */

#if (port_MONITORING_TX_SDA_MISO_PIN)
    #define port_MONITORING_TX_SDA_MISO_HSIOM_REG   (*(reg32 *) port_MONITORING_uart_tx_i2c_sda_spi_miso__0__HSIOM)
    #define port_MONITORING_TX_SDA_MISO_HSIOM_PTR   ( (reg32 *) port_MONITORING_uart_tx_i2c_sda_spi_miso__0__HSIOM)
    
    #define port_MONITORING_TX_SDA_MISO_HSIOM_MASK      (port_MONITORING_uart_tx_i2c_sda_spi_miso__0__HSIOM_MASK)
    #define port_MONITORING_TX_SDA_MISO_HSIOM_POS       (port_MONITORING_uart_tx_i2c_sda_spi_miso__0__HSIOM_SHIFT)
    #define port_MONITORING_TX_SDA_MISO_HSIOM_SEL_GPIO  (port_MONITORING_uart_tx_i2c_sda_spi_miso__0__HSIOM_GPIO)
    #define port_MONITORING_TX_SDA_MISO_HSIOM_SEL_I2C   (port_MONITORING_uart_tx_i2c_sda_spi_miso__0__HSIOM_I2C)
    #define port_MONITORING_TX_SDA_MISO_HSIOM_SEL_SPI   (port_MONITORING_uart_tx_i2c_sda_spi_miso__0__HSIOM_SPI)
    #define port_MONITORING_TX_SDA_MISO_HSIOM_SEL_UART  (port_MONITORING_uart_tx_i2c_sda_spi_miso__0__HSIOM_UART)
#endif /* (port_MONITORING_TX_SDA_MISO_PIN) */

#if (port_MONITORING_CTS_SCLK_PIN)
    #define port_MONITORING_CTS_SCLK_HSIOM_REG   (*(reg32 *) port_MONITORING_uart_cts_spi_sclk__0__HSIOM)
    #define port_MONITORING_CTS_SCLK_HSIOM_PTR   ( (reg32 *) port_MONITORING_uart_cts_spi_sclk__0__HSIOM)
    
    #define port_MONITORING_CTS_SCLK_HSIOM_MASK      (port_MONITORING_uart_cts_spi_sclk__0__HSIOM_MASK)
    #define port_MONITORING_CTS_SCLK_HSIOM_POS       (port_MONITORING_uart_cts_spi_sclk__0__HSIOM_SHIFT)
    #define port_MONITORING_CTS_SCLK_HSIOM_SEL_GPIO  (port_MONITORING_uart_cts_spi_sclk__0__HSIOM_GPIO)
    #define port_MONITORING_CTS_SCLK_HSIOM_SEL_I2C   (port_MONITORING_uart_cts_spi_sclk__0__HSIOM_I2C)
    #define port_MONITORING_CTS_SCLK_HSIOM_SEL_SPI   (port_MONITORING_uart_cts_spi_sclk__0__HSIOM_SPI)
    #define port_MONITORING_CTS_SCLK_HSIOM_SEL_UART  (port_MONITORING_uart_cts_spi_sclk__0__HSIOM_UART)
#endif /* (port_MONITORING_CTS_SCLK_PIN) */

#if (port_MONITORING_RTS_SS0_PIN)
    #define port_MONITORING_RTS_SS0_HSIOM_REG   (*(reg32 *) port_MONITORING_uart_rts_spi_ss0__0__HSIOM)
    #define port_MONITORING_RTS_SS0_HSIOM_PTR   ( (reg32 *) port_MONITORING_uart_rts_spi_ss0__0__HSIOM)
    
    #define port_MONITORING_RTS_SS0_HSIOM_MASK      (port_MONITORING_uart_rts_spi_ss0__0__HSIOM_MASK)
    #define port_MONITORING_RTS_SS0_HSIOM_POS       (port_MONITORING_uart_rts_spi_ss0__0__HSIOM_SHIFT)
    #define port_MONITORING_RTS_SS0_HSIOM_SEL_GPIO  (port_MONITORING_uart_rts_spi_ss0__0__HSIOM_GPIO)
    #define port_MONITORING_RTS_SS0_HSIOM_SEL_I2C   (port_MONITORING_uart_rts_spi_ss0__0__HSIOM_I2C)
    #define port_MONITORING_RTS_SS0_HSIOM_SEL_SPI   (port_MONITORING_uart_rts_spi_ss0__0__HSIOM_SPI)
#if !(port_MONITORING_CY_SCBIP_V0 || port_MONITORING_CY_SCBIP_V1)
    #define port_MONITORING_RTS_SS0_HSIOM_SEL_UART  (port_MONITORING_uart_rts_spi_ss0__0__HSIOM_UART)
#endif /* !(port_MONITORING_CY_SCBIP_V0 || port_MONITORING_CY_SCBIP_V1) */
#endif /* (port_MONITORING_RTS_SS0_PIN) */

#if (port_MONITORING_SS1_PIN)
    #define port_MONITORING_SS1_HSIOM_REG  (*(reg32 *) port_MONITORING_spi_ss1__0__HSIOM)
    #define port_MONITORING_SS1_HSIOM_PTR  ( (reg32 *) port_MONITORING_spi_ss1__0__HSIOM)
    
    #define port_MONITORING_SS1_HSIOM_MASK     (port_MONITORING_spi_ss1__0__HSIOM_MASK)
    #define port_MONITORING_SS1_HSIOM_POS      (port_MONITORING_spi_ss1__0__HSIOM_SHIFT)
    #define port_MONITORING_SS1_HSIOM_SEL_GPIO (port_MONITORING_spi_ss1__0__HSIOM_GPIO)
    #define port_MONITORING_SS1_HSIOM_SEL_I2C  (port_MONITORING_spi_ss1__0__HSIOM_I2C)
    #define port_MONITORING_SS1_HSIOM_SEL_SPI  (port_MONITORING_spi_ss1__0__HSIOM_SPI)
#endif /* (port_MONITORING_SS1_PIN) */

#if (port_MONITORING_SS2_PIN)
    #define port_MONITORING_SS2_HSIOM_REG     (*(reg32 *) port_MONITORING_spi_ss2__0__HSIOM)
    #define port_MONITORING_SS2_HSIOM_PTR     ( (reg32 *) port_MONITORING_spi_ss2__0__HSIOM)
    
    #define port_MONITORING_SS2_HSIOM_MASK     (port_MONITORING_spi_ss2__0__HSIOM_MASK)
    #define port_MONITORING_SS2_HSIOM_POS      (port_MONITORING_spi_ss2__0__HSIOM_SHIFT)
    #define port_MONITORING_SS2_HSIOM_SEL_GPIO (port_MONITORING_spi_ss2__0__HSIOM_GPIO)
    #define port_MONITORING_SS2_HSIOM_SEL_I2C  (port_MONITORING_spi_ss2__0__HSIOM_I2C)
    #define port_MONITORING_SS2_HSIOM_SEL_SPI  (port_MONITORING_spi_ss2__0__HSIOM_SPI)
#endif /* (port_MONITORING_SS2_PIN) */

#if (port_MONITORING_SS3_PIN)
    #define port_MONITORING_SS3_HSIOM_REG     (*(reg32 *) port_MONITORING_spi_ss3__0__HSIOM)
    #define port_MONITORING_SS3_HSIOM_PTR     ( (reg32 *) port_MONITORING_spi_ss3__0__HSIOM)
    
    #define port_MONITORING_SS3_HSIOM_MASK     (port_MONITORING_spi_ss3__0__HSIOM_MASK)
    #define port_MONITORING_SS3_HSIOM_POS      (port_MONITORING_spi_ss3__0__HSIOM_SHIFT)
    #define port_MONITORING_SS3_HSIOM_SEL_GPIO (port_MONITORING_spi_ss3__0__HSIOM_GPIO)
    #define port_MONITORING_SS3_HSIOM_SEL_I2C  (port_MONITORING_spi_ss3__0__HSIOM_I2C)
    #define port_MONITORING_SS3_HSIOM_SEL_SPI  (port_MONITORING_spi_ss3__0__HSIOM_SPI)
#endif /* (port_MONITORING_SS3_PIN) */

#if (port_MONITORING_I2C_PINS)
    #define port_MONITORING_SCL_HSIOM_REG  (*(reg32 *) port_MONITORING_scl__0__HSIOM)
    #define port_MONITORING_SCL_HSIOM_PTR  ( (reg32 *) port_MONITORING_scl__0__HSIOM)
    
    #define port_MONITORING_SCL_HSIOM_MASK     (port_MONITORING_scl__0__HSIOM_MASK)
    #define port_MONITORING_SCL_HSIOM_POS      (port_MONITORING_scl__0__HSIOM_SHIFT)
    #define port_MONITORING_SCL_HSIOM_SEL_GPIO (port_MONITORING_sda__0__HSIOM_GPIO)
    #define port_MONITORING_SCL_HSIOM_SEL_I2C  (port_MONITORING_sda__0__HSIOM_I2C)
    
    #define port_MONITORING_SDA_HSIOM_REG  (*(reg32 *) port_MONITORING_sda__0__HSIOM)
    #define port_MONITORING_SDA_HSIOM_PTR  ( (reg32 *) port_MONITORING_sda__0__HSIOM)
    
    #define port_MONITORING_SDA_HSIOM_MASK     (port_MONITORING_sda__0__HSIOM_MASK)
    #define port_MONITORING_SDA_HSIOM_POS      (port_MONITORING_sda__0__HSIOM_SHIFT)
    #define port_MONITORING_SDA_HSIOM_SEL_GPIO (port_MONITORING_sda__0__HSIOM_GPIO)
    #define port_MONITORING_SDA_HSIOM_SEL_I2C  (port_MONITORING_sda__0__HSIOM_I2C)
#endif /* (port_MONITORING_I2C_PINS) */

#if (port_MONITORING_SPI_SLAVE_PINS)
    #define port_MONITORING_SCLK_S_HSIOM_REG   (*(reg32 *) port_MONITORING_sclk_s__0__HSIOM)
    #define port_MONITORING_SCLK_S_HSIOM_PTR   ( (reg32 *) port_MONITORING_sclk_s__0__HSIOM)
    
    #define port_MONITORING_SCLK_S_HSIOM_MASK      (port_MONITORING_sclk_s__0__HSIOM_MASK)
    #define port_MONITORING_SCLK_S_HSIOM_POS       (port_MONITORING_sclk_s__0__HSIOM_SHIFT)
    #define port_MONITORING_SCLK_S_HSIOM_SEL_GPIO  (port_MONITORING_sclk_s__0__HSIOM_GPIO)
    #define port_MONITORING_SCLK_S_HSIOM_SEL_SPI   (port_MONITORING_sclk_s__0__HSIOM_SPI)
    
    #define port_MONITORING_SS0_S_HSIOM_REG    (*(reg32 *) port_MONITORING_ss0_s__0__HSIOM)
    #define port_MONITORING_SS0_S_HSIOM_PTR    ( (reg32 *) port_MONITORING_ss0_s__0__HSIOM)
    
    #define port_MONITORING_SS0_S_HSIOM_MASK       (port_MONITORING_ss0_s__0__HSIOM_MASK)
    #define port_MONITORING_SS0_S_HSIOM_POS        (port_MONITORING_ss0_s__0__HSIOM_SHIFT)
    #define port_MONITORING_SS0_S_HSIOM_SEL_GPIO   (port_MONITORING_ss0_s__0__HSIOM_GPIO)  
    #define port_MONITORING_SS0_S_HSIOM_SEL_SPI    (port_MONITORING_ss0_s__0__HSIOM_SPI)
#endif /* (port_MONITORING_SPI_SLAVE_PINS) */

#if (port_MONITORING_SPI_SLAVE_MOSI_PIN)
    #define port_MONITORING_MOSI_S_HSIOM_REG   (*(reg32 *) port_MONITORING_mosi_s__0__HSIOM)
    #define port_MONITORING_MOSI_S_HSIOM_PTR   ( (reg32 *) port_MONITORING_mosi_s__0__HSIOM)
    
    #define port_MONITORING_MOSI_S_HSIOM_MASK      (port_MONITORING_mosi_s__0__HSIOM_MASK)
    #define port_MONITORING_MOSI_S_HSIOM_POS       (port_MONITORING_mosi_s__0__HSIOM_SHIFT)
    #define port_MONITORING_MOSI_S_HSIOM_SEL_GPIO  (port_MONITORING_mosi_s__0__HSIOM_GPIO)
    #define port_MONITORING_MOSI_S_HSIOM_SEL_SPI   (port_MONITORING_mosi_s__0__HSIOM_SPI)
#endif /* (port_MONITORING_SPI_SLAVE_MOSI_PIN) */

#if (port_MONITORING_SPI_SLAVE_MISO_PIN)
    #define port_MONITORING_MISO_S_HSIOM_REG   (*(reg32 *) port_MONITORING_miso_s__0__HSIOM)
    #define port_MONITORING_MISO_S_HSIOM_PTR   ( (reg32 *) port_MONITORING_miso_s__0__HSIOM)
    
    #define port_MONITORING_MISO_S_HSIOM_MASK      (port_MONITORING_miso_s__0__HSIOM_MASK)
    #define port_MONITORING_MISO_S_HSIOM_POS       (port_MONITORING_miso_s__0__HSIOM_SHIFT)
    #define port_MONITORING_MISO_S_HSIOM_SEL_GPIO  (port_MONITORING_miso_s__0__HSIOM_GPIO)
    #define port_MONITORING_MISO_S_HSIOM_SEL_SPI   (port_MONITORING_miso_s__0__HSIOM_SPI)
#endif /* (port_MONITORING_SPI_SLAVE_MISO_PIN) */

#if (port_MONITORING_SPI_MASTER_MISO_PIN)
    #define port_MONITORING_MISO_M_HSIOM_REG   (*(reg32 *) port_MONITORING_miso_m__0__HSIOM)
    #define port_MONITORING_MISO_M_HSIOM_PTR   ( (reg32 *) port_MONITORING_miso_m__0__HSIOM)
    
    #define port_MONITORING_MISO_M_HSIOM_MASK      (port_MONITORING_miso_m__0__HSIOM_MASK)
    #define port_MONITORING_MISO_M_HSIOM_POS       (port_MONITORING_miso_m__0__HSIOM_SHIFT)
    #define port_MONITORING_MISO_M_HSIOM_SEL_GPIO  (port_MONITORING_miso_m__0__HSIOM_GPIO)
    #define port_MONITORING_MISO_M_HSIOM_SEL_SPI   (port_MONITORING_miso_m__0__HSIOM_SPI)
#endif /* (port_MONITORING_SPI_MASTER_MISO_PIN) */

#if (port_MONITORING_SPI_MASTER_MOSI_PIN)
    #define port_MONITORING_MOSI_M_HSIOM_REG   (*(reg32 *) port_MONITORING_mosi_m__0__HSIOM)
    #define port_MONITORING_MOSI_M_HSIOM_PTR   ( (reg32 *) port_MONITORING_mosi_m__0__HSIOM)
    
    #define port_MONITORING_MOSI_M_HSIOM_MASK      (port_MONITORING_mosi_m__0__HSIOM_MASK)
    #define port_MONITORING_MOSI_M_HSIOM_POS       (port_MONITORING_mosi_m__0__HSIOM_SHIFT)
    #define port_MONITORING_MOSI_M_HSIOM_SEL_GPIO  (port_MONITORING_mosi_m__0__HSIOM_GPIO)
    #define port_MONITORING_MOSI_M_HSIOM_SEL_SPI   (port_MONITORING_mosi_m__0__HSIOM_SPI)
#endif /* (port_MONITORING_SPI_MASTER_MOSI_PIN) */

#if (port_MONITORING_SPI_MASTER_SCLK_PIN)
    #define port_MONITORING_SCLK_M_HSIOM_REG   (*(reg32 *) port_MONITORING_sclk_m__0__HSIOM)
    #define port_MONITORING_SCLK_M_HSIOM_PTR   ( (reg32 *) port_MONITORING_sclk_m__0__HSIOM)
    
    #define port_MONITORING_SCLK_M_HSIOM_MASK      (port_MONITORING_sclk_m__0__HSIOM_MASK)
    #define port_MONITORING_SCLK_M_HSIOM_POS       (port_MONITORING_sclk_m__0__HSIOM_SHIFT)
    #define port_MONITORING_SCLK_M_HSIOM_SEL_GPIO  (port_MONITORING_sclk_m__0__HSIOM_GPIO)
    #define port_MONITORING_SCLK_M_HSIOM_SEL_SPI   (port_MONITORING_sclk_m__0__HSIOM_SPI)
#endif /* (port_MONITORING_SPI_MASTER_SCLK_PIN) */

#if (port_MONITORING_SPI_MASTER_SS0_PIN)
    #define port_MONITORING_SS0_M_HSIOM_REG    (*(reg32 *) port_MONITORING_ss0_m__0__HSIOM)
    #define port_MONITORING_SS0_M_HSIOM_PTR    ( (reg32 *) port_MONITORING_ss0_m__0__HSIOM)
    
    #define port_MONITORING_SS0_M_HSIOM_MASK       (port_MONITORING_ss0_m__0__HSIOM_MASK)
    #define port_MONITORING_SS0_M_HSIOM_POS        (port_MONITORING_ss0_m__0__HSIOM_SHIFT)
    #define port_MONITORING_SS0_M_HSIOM_SEL_GPIO   (port_MONITORING_ss0_m__0__HSIOM_GPIO)
    #define port_MONITORING_SS0_M_HSIOM_SEL_SPI    (port_MONITORING_ss0_m__0__HSIOM_SPI)
#endif /* (port_MONITORING_SPI_MASTER_SS0_PIN) */

#if (port_MONITORING_SPI_MASTER_SS1_PIN)
    #define port_MONITORING_SS1_M_HSIOM_REG    (*(reg32 *) port_MONITORING_ss1_m__0__HSIOM)
    #define port_MONITORING_SS1_M_HSIOM_PTR    ( (reg32 *) port_MONITORING_ss1_m__0__HSIOM)
    
    #define port_MONITORING_SS1_M_HSIOM_MASK       (port_MONITORING_ss1_m__0__HSIOM_MASK)
    #define port_MONITORING_SS1_M_HSIOM_POS        (port_MONITORING_ss1_m__0__HSIOM_SHIFT)
    #define port_MONITORING_SS1_M_HSIOM_SEL_GPIO   (port_MONITORING_ss1_m__0__HSIOM_GPIO)
    #define port_MONITORING_SS1_M_HSIOM_SEL_SPI    (port_MONITORING_ss1_m__0__HSIOM_SPI)
#endif /* (port_MONITORING_SPI_MASTER_SS1_PIN) */

#if (port_MONITORING_SPI_MASTER_SS2_PIN)
    #define port_MONITORING_SS2_M_HSIOM_REG    (*(reg32 *) port_MONITORING_ss2_m__0__HSIOM)
    #define port_MONITORING_SS2_M_HSIOM_PTR    ( (reg32 *) port_MONITORING_ss2_m__0__HSIOM)
    
    #define port_MONITORING_SS2_M_HSIOM_MASK       (port_MONITORING_ss2_m__0__HSIOM_MASK)
    #define port_MONITORING_SS2_M_HSIOM_POS        (port_MONITORING_ss2_m__0__HSIOM_SHIFT)
    #define port_MONITORING_SS2_M_HSIOM_SEL_GPIO   (port_MONITORING_ss2_m__0__HSIOM_GPIO)
    #define port_MONITORING_SS2_M_HSIOM_SEL_SPI    (port_MONITORING_ss2_m__0__HSIOM_SPI)
#endif /* (port_MONITORING_SPI_MASTER_SS2_PIN) */

#if (port_MONITORING_SPI_MASTER_SS3_PIN)
    #define port_MONITORING_SS3_M_HSIOM_REG    (*(reg32 *) port_MONITORING_ss3_m__0__HSIOM)
    #define port_MONITORING_SS3_M_HSIOM_PTR    ( (reg32 *) port_MONITORING_ss3_m__0__HSIOM)
    
    #define port_MONITORING_SS3_M_HSIOM_MASK      (port_MONITORING_ss3_m__0__HSIOM_MASK)
    #define port_MONITORING_SS3_M_HSIOM_POS       (port_MONITORING_ss3_m__0__HSIOM_SHIFT)
    #define port_MONITORING_SS3_M_HSIOM_SEL_GPIO  (port_MONITORING_ss3_m__0__HSIOM_GPIO)
    #define port_MONITORING_SS3_M_HSIOM_SEL_SPI   (port_MONITORING_ss3_m__0__HSIOM_SPI)
#endif /* (port_MONITORING_SPI_MASTER_SS3_PIN) */

#if (port_MONITORING_UART_RX_PIN)
    #define port_MONITORING_RX_HSIOM_REG   (*(reg32 *) port_MONITORING_rx__0__HSIOM)
    #define port_MONITORING_RX_HSIOM_PTR   ( (reg32 *) port_MONITORING_rx__0__HSIOM)
    
    #define port_MONITORING_RX_HSIOM_MASK      (port_MONITORING_rx__0__HSIOM_MASK)
    #define port_MONITORING_RX_HSIOM_POS       (port_MONITORING_rx__0__HSIOM_SHIFT)
    #define port_MONITORING_RX_HSIOM_SEL_GPIO  (port_MONITORING_rx__0__HSIOM_GPIO)
    #define port_MONITORING_RX_HSIOM_SEL_UART  (port_MONITORING_rx__0__HSIOM_UART)
#endif /* (port_MONITORING_UART_RX_PIN) */

#if (port_MONITORING_UART_RX_WAKE_PIN)
    #define port_MONITORING_RX_WAKE_HSIOM_REG   (*(reg32 *) port_MONITORING_rx_wake__0__HSIOM)
    #define port_MONITORING_RX_WAKE_HSIOM_PTR   ( (reg32 *) port_MONITORING_rx_wake__0__HSIOM)
    
    #define port_MONITORING_RX_WAKE_HSIOM_MASK      (port_MONITORING_rx_wake__0__HSIOM_MASK)
    #define port_MONITORING_RX_WAKE_HSIOM_POS       (port_MONITORING_rx_wake__0__HSIOM_SHIFT)
    #define port_MONITORING_RX_WAKE_HSIOM_SEL_GPIO  (port_MONITORING_rx_wake__0__HSIOM_GPIO)
    #define port_MONITORING_RX_WAKE_HSIOM_SEL_UART  (port_MONITORING_rx_wake__0__HSIOM_UART)
#endif /* (port_MONITORING_UART_WAKE_RX_PIN) */

#if (port_MONITORING_UART_CTS_PIN)
    #define port_MONITORING_CTS_HSIOM_REG   (*(reg32 *) port_MONITORING_cts__0__HSIOM)
    #define port_MONITORING_CTS_HSIOM_PTR   ( (reg32 *) port_MONITORING_cts__0__HSIOM)
    
    #define port_MONITORING_CTS_HSIOM_MASK      (port_MONITORING_cts__0__HSIOM_MASK)
    #define port_MONITORING_CTS_HSIOM_POS       (port_MONITORING_cts__0__HSIOM_SHIFT)
    #define port_MONITORING_CTS_HSIOM_SEL_GPIO  (port_MONITORING_cts__0__HSIOM_GPIO)
    #define port_MONITORING_CTS_HSIOM_SEL_UART  (port_MONITORING_cts__0__HSIOM_UART)
#endif /* (port_MONITORING_UART_CTS_PIN) */

#if (port_MONITORING_UART_TX_PIN)
    #define port_MONITORING_TX_HSIOM_REG   (*(reg32 *) port_MONITORING_tx__0__HSIOM)
    #define port_MONITORING_TX_HSIOM_PTR   ( (reg32 *) port_MONITORING_tx__0__HSIOM)
    
    #define port_MONITORING_TX_HSIOM_MASK      (port_MONITORING_tx__0__HSIOM_MASK)
    #define port_MONITORING_TX_HSIOM_POS       (port_MONITORING_tx__0__HSIOM_SHIFT)
    #define port_MONITORING_TX_HSIOM_SEL_GPIO  (port_MONITORING_tx__0__HSIOM_GPIO)
    #define port_MONITORING_TX_HSIOM_SEL_UART  (port_MONITORING_tx__0__HSIOM_UART)
#endif /* (port_MONITORING_UART_TX_PIN) */

#if (port_MONITORING_UART_RX_TX_PIN)
    #define port_MONITORING_RX_TX_HSIOM_REG   (*(reg32 *) port_MONITORING_rx_tx__0__HSIOM)
    #define port_MONITORING_RX_TX_HSIOM_PTR   ( (reg32 *) port_MONITORING_rx_tx__0__HSIOM)
    
    #define port_MONITORING_RX_TX_HSIOM_MASK      (port_MONITORING_rx_tx__0__HSIOM_MASK)
    #define port_MONITORING_RX_TX_HSIOM_POS       (port_MONITORING_rx_tx__0__HSIOM_SHIFT)
    #define port_MONITORING_RX_TX_HSIOM_SEL_GPIO  (port_MONITORING_rx_tx__0__HSIOM_GPIO)
    #define port_MONITORING_RX_TX_HSIOM_SEL_UART  (port_MONITORING_rx_tx__0__HSIOM_UART)
#endif /* (port_MONITORING_UART_RX_TX_PIN) */

#if (port_MONITORING_UART_RTS_PIN)
    #define port_MONITORING_RTS_HSIOM_REG      (*(reg32 *) port_MONITORING_rts__0__HSIOM)
    #define port_MONITORING_RTS_HSIOM_PTR      ( (reg32 *) port_MONITORING_rts__0__HSIOM)
    
    #define port_MONITORING_RTS_HSIOM_MASK     (port_MONITORING_rts__0__HSIOM_MASK)
    #define port_MONITORING_RTS_HSIOM_POS      (port_MONITORING_rts__0__HSIOM_SHIFT)    
    #define port_MONITORING_RTS_HSIOM_SEL_GPIO (port_MONITORING_rts__0__HSIOM_GPIO)
    #define port_MONITORING_RTS_HSIOM_SEL_UART (port_MONITORING_rts__0__HSIOM_UART)    
#endif /* (port_MONITORING_UART_RTS_PIN) */


/***************************************
*        Registers Constants
***************************************/

/* HSIOM switch values. */ 
#define port_MONITORING_HSIOM_DEF_SEL      (0x00u)
#define port_MONITORING_HSIOM_GPIO_SEL     (0x00u)
/* The HSIOM values provided below are valid only for port_MONITORING_CY_SCBIP_V0 
* and port_MONITORING_CY_SCBIP_V1. It is not recommended to use them for 
* port_MONITORING_CY_SCBIP_V2. Use pin name specific HSIOM constants provided 
* above instead for any SCB IP block version.
*/
#define port_MONITORING_HSIOM_UART_SEL     (0x09u)
#define port_MONITORING_HSIOM_I2C_SEL      (0x0Eu)
#define port_MONITORING_HSIOM_SPI_SEL      (0x0Fu)

/* Pins settings index. */
#define port_MONITORING_RX_WAKE_SCL_MOSI_PIN_INDEX   (0u)
#define port_MONITORING_RX_SCL_MOSI_PIN_INDEX       (0u)
#define port_MONITORING_TX_SDA_MISO_PIN_INDEX       (1u)
#define port_MONITORING_CTS_SCLK_PIN_INDEX       (2u)
#define port_MONITORING_RTS_SS0_PIN_INDEX       (3u)
#define port_MONITORING_SS1_PIN_INDEX                  (4u)
#define port_MONITORING_SS2_PIN_INDEX                  (5u)
#define port_MONITORING_SS3_PIN_INDEX                  (6u)

/* Pins settings mask. */
#define port_MONITORING_RX_WAKE_SCL_MOSI_PIN_MASK ((uint32) 0x01u << port_MONITORING_RX_WAKE_SCL_MOSI_PIN_INDEX)
#define port_MONITORING_RX_SCL_MOSI_PIN_MASK     ((uint32) 0x01u << port_MONITORING_RX_SCL_MOSI_PIN_INDEX)
#define port_MONITORING_TX_SDA_MISO_PIN_MASK     ((uint32) 0x01u << port_MONITORING_TX_SDA_MISO_PIN_INDEX)
#define port_MONITORING_CTS_SCLK_PIN_MASK     ((uint32) 0x01u << port_MONITORING_CTS_SCLK_PIN_INDEX)
#define port_MONITORING_RTS_SS0_PIN_MASK     ((uint32) 0x01u << port_MONITORING_RTS_SS0_PIN_INDEX)
#define port_MONITORING_SS1_PIN_MASK                ((uint32) 0x01u << port_MONITORING_SS1_PIN_INDEX)
#define port_MONITORING_SS2_PIN_MASK                ((uint32) 0x01u << port_MONITORING_SS2_PIN_INDEX)
#define port_MONITORING_SS3_PIN_MASK                ((uint32) 0x01u << port_MONITORING_SS3_PIN_INDEX)

/* Pin interrupt constants. */
#define port_MONITORING_INTCFG_TYPE_MASK           (0x03u)
#define port_MONITORING_INTCFG_TYPE_FALLING_EDGE   (0x02u)

/* Pin Drive Mode constants. */
#define port_MONITORING_PIN_DM_ALG_HIZ  (0u)
#define port_MONITORING_PIN_DM_DIG_HIZ  (1u)
#define port_MONITORING_PIN_DM_OD_LO    (4u)
#define port_MONITORING_PIN_DM_STRONG   (6u)


/***************************************
*          Macro Definitions
***************************************/

/* Return drive mode of the pin */
#define port_MONITORING_DM_MASK    (0x7u)
#define port_MONITORING_DM_SIZE    (3u)
#define port_MONITORING_GET_P4_PIN_DM(reg, pos) \
    ( ((reg) & (uint32) ((uint32) port_MONITORING_DM_MASK << (port_MONITORING_DM_SIZE * (pos)))) >> \
                                                              (port_MONITORING_DM_SIZE * (pos)) )

#if (port_MONITORING_TX_SDA_MISO_PIN)
    #define port_MONITORING_CHECK_TX_SDA_MISO_PIN_USED \
                (port_MONITORING_PIN_DM_ALG_HIZ != \
                    port_MONITORING_GET_P4_PIN_DM(port_MONITORING_uart_tx_i2c_sda_spi_miso_PC, \
                                                   port_MONITORING_uart_tx_i2c_sda_spi_miso_SHIFT))
#endif /* (port_MONITORING_TX_SDA_MISO_PIN) */

#if (port_MONITORING_RTS_SS0_PIN)
    #define port_MONITORING_CHECK_RTS_SS0_PIN_USED \
                (port_MONITORING_PIN_DM_ALG_HIZ != \
                    port_MONITORING_GET_P4_PIN_DM(port_MONITORING_uart_rts_spi_ss0_PC, \
                                                   port_MONITORING_uart_rts_spi_ss0_SHIFT))
#endif /* (port_MONITORING_RTS_SS0_PIN) */

/* Set bits-mask in register */
#define port_MONITORING_SET_REGISTER_BITS(reg, mask, pos, mode) \
                    do                                           \
                    {                                            \
                        (reg) = (((reg) & ((uint32) ~(uint32) (mask))) | ((uint32) ((uint32) (mode) << (pos)))); \
                    }while(0)

/* Set bit in the register */
#define port_MONITORING_SET_REGISTER_BIT(reg, mask, val) \
                    ((val) ? ((reg) |= (mask)) : ((reg) &= ((uint32) ~((uint32) (mask)))))

#define port_MONITORING_SET_HSIOM_SEL(reg, mask, pos, sel) port_MONITORING_SET_REGISTER_BITS(reg, mask, pos, sel)
#define port_MONITORING_SET_INCFG_TYPE(reg, mask, pos, intType) \
                                                        port_MONITORING_SET_REGISTER_BITS(reg, mask, pos, intType)
#define port_MONITORING_SET_INP_DIS(reg, mask, val) port_MONITORING_SET_REGISTER_BIT(reg, mask, val)

/* port_MONITORING_SET_I2C_SCL_DR(val) - Sets I2C SCL DR register.
*  port_MONITORING_SET_I2C_SCL_HSIOM_SEL(sel) - Sets I2C SCL HSIOM settings.
*/
/* SCB I2C: scl signal */
#if (port_MONITORING_CY_SCBIP_V0)
#if (port_MONITORING_I2C_PINS)
    #define port_MONITORING_SET_I2C_SCL_DR(val) port_MONITORING_scl_Write(val)

    #define port_MONITORING_SET_I2C_SCL_HSIOM_SEL(sel) \
                          port_MONITORING_SET_HSIOM_SEL(port_MONITORING_SCL_HSIOM_REG,  \
                                                         port_MONITORING_SCL_HSIOM_MASK, \
                                                         port_MONITORING_SCL_HSIOM_POS,  \
                                                         (sel))
    #define port_MONITORING_WAIT_SCL_SET_HIGH  (0u == port_MONITORING_scl_Read())

/* Unconfigured SCB: scl signal */
#elif (port_MONITORING_RX_WAKE_SCL_MOSI_PIN)
    #define port_MONITORING_SET_I2C_SCL_DR(val) \
                            port_MONITORING_uart_rx_wake_i2c_scl_spi_mosi_Write(val)

    #define port_MONITORING_SET_I2C_SCL_HSIOM_SEL(sel) \
                    port_MONITORING_SET_HSIOM_SEL(port_MONITORING_RX_WAKE_SCL_MOSI_HSIOM_REG,  \
                                                   port_MONITORING_RX_WAKE_SCL_MOSI_HSIOM_MASK, \
                                                   port_MONITORING_RX_WAKE_SCL_MOSI_HSIOM_POS,  \
                                                   (sel))

    #define port_MONITORING_WAIT_SCL_SET_HIGH  (0u == port_MONITORING_uart_rx_wake_i2c_scl_spi_mosi_Read())

#elif (port_MONITORING_RX_SCL_MOSI_PIN)
    #define port_MONITORING_SET_I2C_SCL_DR(val) \
                            port_MONITORING_uart_rx_i2c_scl_spi_mosi_Write(val)


    #define port_MONITORING_SET_I2C_SCL_HSIOM_SEL(sel) \
                            port_MONITORING_SET_HSIOM_SEL(port_MONITORING_RX_SCL_MOSI_HSIOM_REG,  \
                                                           port_MONITORING_RX_SCL_MOSI_HSIOM_MASK, \
                                                           port_MONITORING_RX_SCL_MOSI_HSIOM_POS,  \
                                                           (sel))

    #define port_MONITORING_WAIT_SCL_SET_HIGH  (0u == port_MONITORING_uart_rx_i2c_scl_spi_mosi_Read())

#else
    #define port_MONITORING_SET_I2C_SCL_DR(val)        do{ /* Does nothing */ }while(0)
    #define port_MONITORING_SET_I2C_SCL_HSIOM_SEL(sel) do{ /* Does nothing */ }while(0)

    #define port_MONITORING_WAIT_SCL_SET_HIGH  (0u)
#endif /* (port_MONITORING_I2C_PINS) */

/* SCB I2C: sda signal */
#if (port_MONITORING_I2C_PINS)
    #define port_MONITORING_WAIT_SDA_SET_HIGH  (0u == port_MONITORING_sda_Read())
/* Unconfigured SCB: sda signal */
#elif (port_MONITORING_TX_SDA_MISO_PIN)
    #define port_MONITORING_WAIT_SDA_SET_HIGH  (0u == port_MONITORING_uart_tx_i2c_sda_spi_miso_Read())
#else
    #define port_MONITORING_WAIT_SDA_SET_HIGH  (0u)
#endif /* (port_MONITORING_MOSI_SCL_RX_PIN) */
#endif /* (port_MONITORING_CY_SCBIP_V0) */

/* Clear UART wakeup source */
#if (port_MONITORING_RX_SCL_MOSI_PIN)
    #define port_MONITORING_CLEAR_UART_RX_WAKE_INTR        do{ /* Does nothing */ }while(0)
    
#elif (port_MONITORING_RX_WAKE_SCL_MOSI_PIN)
    #define port_MONITORING_CLEAR_UART_RX_WAKE_INTR \
            do{                                      \
                (void) port_MONITORING_uart_rx_wake_i2c_scl_spi_mosi_ClearInterrupt(); \
            }while(0)

#elif(port_MONITORING_UART_RX_WAKE_PIN)
    #define port_MONITORING_CLEAR_UART_RX_WAKE_INTR \
            do{                                      \
                (void) port_MONITORING_rx_wake_ClearInterrupt(); \
            }while(0)
#else
#endif /* (port_MONITORING_RX_SCL_MOSI_PIN) */


/***************************************
* The following code is DEPRECATED and
* must not be used.
***************************************/

/* Unconfigured pins */
#define port_MONITORING_REMOVE_MOSI_SCL_RX_WAKE_PIN    port_MONITORING_REMOVE_RX_WAKE_SCL_MOSI_PIN
#define port_MONITORING_REMOVE_MOSI_SCL_RX_PIN         port_MONITORING_REMOVE_RX_SCL_MOSI_PIN
#define port_MONITORING_REMOVE_MISO_SDA_TX_PIN         port_MONITORING_REMOVE_TX_SDA_MISO_PIN
#ifndef port_MONITORING_REMOVE_SCLK_PIN
#define port_MONITORING_REMOVE_SCLK_PIN                port_MONITORING_REMOVE_CTS_SCLK_PIN
#endif /* port_MONITORING_REMOVE_SCLK_PIN */
#ifndef port_MONITORING_REMOVE_SS0_PIN
#define port_MONITORING_REMOVE_SS0_PIN                 port_MONITORING_REMOVE_RTS_SS0_PIN
#endif /* port_MONITORING_REMOVE_SS0_PIN */

/* Unconfigured pins */
#define port_MONITORING_MOSI_SCL_RX_WAKE_PIN   port_MONITORING_RX_WAKE_SCL_MOSI_PIN
#define port_MONITORING_MOSI_SCL_RX_PIN        port_MONITORING_RX_SCL_MOSI_PIN
#define port_MONITORING_MISO_SDA_TX_PIN        port_MONITORING_TX_SDA_MISO_PIN
#ifndef port_MONITORING_SCLK_PIN
#define port_MONITORING_SCLK_PIN               port_MONITORING_CTS_SCLK_PIN
#endif /* port_MONITORING_SCLK_PIN */
#ifndef port_MONITORING_SS0_PIN
#define port_MONITORING_SS0_PIN                port_MONITORING_RTS_SS0_PIN
#endif /* port_MONITORING_SS0_PIN */

#if (port_MONITORING_MOSI_SCL_RX_WAKE_PIN)
    #define port_MONITORING_MOSI_SCL_RX_WAKE_HSIOM_REG     port_MONITORING_RX_WAKE_SCL_MOSI_HSIOM_REG
    #define port_MONITORING_MOSI_SCL_RX_WAKE_HSIOM_PTR     port_MONITORING_RX_WAKE_SCL_MOSI_HSIOM_REG
    #define port_MONITORING_MOSI_SCL_RX_WAKE_HSIOM_MASK    port_MONITORING_RX_WAKE_SCL_MOSI_HSIOM_REG
    #define port_MONITORING_MOSI_SCL_RX_WAKE_HSIOM_POS     port_MONITORING_RX_WAKE_SCL_MOSI_HSIOM_REG

    #define port_MONITORING_MOSI_SCL_RX_WAKE_INTCFG_REG    port_MONITORING_RX_WAKE_SCL_MOSI_HSIOM_REG
    #define port_MONITORING_MOSI_SCL_RX_WAKE_INTCFG_PTR    port_MONITORING_RX_WAKE_SCL_MOSI_HSIOM_REG

    #define port_MONITORING_MOSI_SCL_RX_WAKE_INTCFG_TYPE_POS   port_MONITORING_RX_WAKE_SCL_MOSI_HSIOM_REG
    #define port_MONITORING_MOSI_SCL_RX_WAKE_INTCFG_TYPE_MASK  port_MONITORING_RX_WAKE_SCL_MOSI_HSIOM_REG
#endif /* (port_MONITORING_RX_WAKE_SCL_MOSI_PIN) */

#if (port_MONITORING_MOSI_SCL_RX_PIN)
    #define port_MONITORING_MOSI_SCL_RX_HSIOM_REG      port_MONITORING_RX_SCL_MOSI_HSIOM_REG
    #define port_MONITORING_MOSI_SCL_RX_HSIOM_PTR      port_MONITORING_RX_SCL_MOSI_HSIOM_PTR
    #define port_MONITORING_MOSI_SCL_RX_HSIOM_MASK     port_MONITORING_RX_SCL_MOSI_HSIOM_MASK
    #define port_MONITORING_MOSI_SCL_RX_HSIOM_POS      port_MONITORING_RX_SCL_MOSI_HSIOM_POS
#endif /* (port_MONITORING_MOSI_SCL_RX_PIN) */

#if (port_MONITORING_MISO_SDA_TX_PIN)
    #define port_MONITORING_MISO_SDA_TX_HSIOM_REG      port_MONITORING_TX_SDA_MISO_HSIOM_REG
    #define port_MONITORING_MISO_SDA_TX_HSIOM_PTR      port_MONITORING_TX_SDA_MISO_HSIOM_REG
    #define port_MONITORING_MISO_SDA_TX_HSIOM_MASK     port_MONITORING_TX_SDA_MISO_HSIOM_REG
    #define port_MONITORING_MISO_SDA_TX_HSIOM_POS      port_MONITORING_TX_SDA_MISO_HSIOM_REG
#endif /* (port_MONITORING_MISO_SDA_TX_PIN_PIN) */

#if (port_MONITORING_SCLK_PIN)
    #ifndef port_MONITORING_SCLK_HSIOM_REG
    #define port_MONITORING_SCLK_HSIOM_REG     port_MONITORING_CTS_SCLK_HSIOM_REG
    #define port_MONITORING_SCLK_HSIOM_PTR     port_MONITORING_CTS_SCLK_HSIOM_PTR
    #define port_MONITORING_SCLK_HSIOM_MASK    port_MONITORING_CTS_SCLK_HSIOM_MASK
    #define port_MONITORING_SCLK_HSIOM_POS     port_MONITORING_CTS_SCLK_HSIOM_POS
    #endif /* port_MONITORING_SCLK_HSIOM_REG */
#endif /* (port_MONITORING_SCLK_PIN) */

#if (port_MONITORING_SS0_PIN)
    #ifndef port_MONITORING_SS0_HSIOM_REG
    #define port_MONITORING_SS0_HSIOM_REG      port_MONITORING_RTS_SS0_HSIOM_REG
    #define port_MONITORING_SS0_HSIOM_PTR      port_MONITORING_RTS_SS0_HSIOM_PTR
    #define port_MONITORING_SS0_HSIOM_MASK     port_MONITORING_RTS_SS0_HSIOM_MASK
    #define port_MONITORING_SS0_HSIOM_POS      port_MONITORING_RTS_SS0_HSIOM_POS
    #endif /* port_MONITORING_SS0_HSIOM_REG */
#endif /* (port_MONITORING_SS0_PIN) */

#define port_MONITORING_MOSI_SCL_RX_WAKE_PIN_INDEX port_MONITORING_RX_WAKE_SCL_MOSI_PIN_INDEX
#define port_MONITORING_MOSI_SCL_RX_PIN_INDEX      port_MONITORING_RX_SCL_MOSI_PIN_INDEX
#define port_MONITORING_MISO_SDA_TX_PIN_INDEX      port_MONITORING_TX_SDA_MISO_PIN_INDEX
#ifndef port_MONITORING_SCLK_PIN_INDEX
#define port_MONITORING_SCLK_PIN_INDEX             port_MONITORING_CTS_SCLK_PIN_INDEX
#endif /* port_MONITORING_SCLK_PIN_INDEX */
#ifndef port_MONITORING_SS0_PIN_INDEX
#define port_MONITORING_SS0_PIN_INDEX              port_MONITORING_RTS_SS0_PIN_INDEX
#endif /* port_MONITORING_SS0_PIN_INDEX */

#define port_MONITORING_MOSI_SCL_RX_WAKE_PIN_MASK port_MONITORING_RX_WAKE_SCL_MOSI_PIN_MASK
#define port_MONITORING_MOSI_SCL_RX_PIN_MASK      port_MONITORING_RX_SCL_MOSI_PIN_MASK
#define port_MONITORING_MISO_SDA_TX_PIN_MASK      port_MONITORING_TX_SDA_MISO_PIN_MASK
#ifndef port_MONITORING_SCLK_PIN_MASK
#define port_MONITORING_SCLK_PIN_MASK             port_MONITORING_CTS_SCLK_PIN_MASK
#endif /* port_MONITORING_SCLK_PIN_MASK */
#ifndef port_MONITORING_SS0_PIN_MASK
#define port_MONITORING_SS0_PIN_MASK              port_MONITORING_RTS_SS0_PIN_MASK
#endif /* port_MONITORING_SS0_PIN_MASK */

#endif /* (CY_SCB_PINS_port_MONITORING_H) */


/* [] END OF FILE */
