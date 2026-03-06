/***************************************************************************//**
* \file OP_ST500_PINS.h
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

#if !defined(CY_SCB_PINS_OP_ST500_H)
#define CY_SCB_PINS_OP_ST500_H

#include "cydevice_trm.h"
#include "cyfitter.h"
#include "cytypes.h"


/***************************************
*   Conditional Compilation Parameters
****************************************/

/* Unconfigured pins */
#define OP_ST500_REMOVE_RX_WAKE_SCL_MOSI_PIN  (1u)
#define OP_ST500_REMOVE_RX_SCL_MOSI_PIN      (1u)
#define OP_ST500_REMOVE_TX_SDA_MISO_PIN      (1u)
#define OP_ST500_REMOVE_CTS_SCLK_PIN      (1u)
#define OP_ST500_REMOVE_RTS_SS0_PIN      (1u)
#define OP_ST500_REMOVE_SS1_PIN                 (1u)
#define OP_ST500_REMOVE_SS2_PIN                 (1u)
#define OP_ST500_REMOVE_SS3_PIN                 (1u)

/* Mode defined pins */
#define OP_ST500_REMOVE_I2C_PINS                (1u)
#define OP_ST500_REMOVE_SPI_MASTER_PINS         (1u)
#define OP_ST500_REMOVE_SPI_MASTER_SCLK_PIN     (1u)
#define OP_ST500_REMOVE_SPI_MASTER_MOSI_PIN     (1u)
#define OP_ST500_REMOVE_SPI_MASTER_MISO_PIN     (1u)
#define OP_ST500_REMOVE_SPI_MASTER_SS0_PIN      (1u)
#define OP_ST500_REMOVE_SPI_MASTER_SS1_PIN      (1u)
#define OP_ST500_REMOVE_SPI_MASTER_SS2_PIN      (1u)
#define OP_ST500_REMOVE_SPI_MASTER_SS3_PIN      (1u)
#define OP_ST500_REMOVE_SPI_SLAVE_PINS          (1u)
#define OP_ST500_REMOVE_SPI_SLAVE_MOSI_PIN      (1u)
#define OP_ST500_REMOVE_SPI_SLAVE_MISO_PIN      (1u)
#define OP_ST500_REMOVE_UART_TX_PIN             (0u)
#define OP_ST500_REMOVE_UART_RX_TX_PIN          (1u)
#define OP_ST500_REMOVE_UART_RX_PIN             (0u)
#define OP_ST500_REMOVE_UART_RX_WAKE_PIN        (1u)
#define OP_ST500_REMOVE_UART_RTS_PIN            (1u)
#define OP_ST500_REMOVE_UART_CTS_PIN            (1u)

/* Unconfigured pins */
#define OP_ST500_RX_WAKE_SCL_MOSI_PIN (0u == OP_ST500_REMOVE_RX_WAKE_SCL_MOSI_PIN)
#define OP_ST500_RX_SCL_MOSI_PIN     (0u == OP_ST500_REMOVE_RX_SCL_MOSI_PIN)
#define OP_ST500_TX_SDA_MISO_PIN     (0u == OP_ST500_REMOVE_TX_SDA_MISO_PIN)
#define OP_ST500_CTS_SCLK_PIN     (0u == OP_ST500_REMOVE_CTS_SCLK_PIN)
#define OP_ST500_RTS_SS0_PIN     (0u == OP_ST500_REMOVE_RTS_SS0_PIN)
#define OP_ST500_SS1_PIN                (0u == OP_ST500_REMOVE_SS1_PIN)
#define OP_ST500_SS2_PIN                (0u == OP_ST500_REMOVE_SS2_PIN)
#define OP_ST500_SS3_PIN                (0u == OP_ST500_REMOVE_SS3_PIN)

/* Mode defined pins */
#define OP_ST500_I2C_PINS               (0u == OP_ST500_REMOVE_I2C_PINS)
#define OP_ST500_SPI_MASTER_PINS        (0u == OP_ST500_REMOVE_SPI_MASTER_PINS)
#define OP_ST500_SPI_MASTER_SCLK_PIN    (0u == OP_ST500_REMOVE_SPI_MASTER_SCLK_PIN)
#define OP_ST500_SPI_MASTER_MOSI_PIN    (0u == OP_ST500_REMOVE_SPI_MASTER_MOSI_PIN)
#define OP_ST500_SPI_MASTER_MISO_PIN    (0u == OP_ST500_REMOVE_SPI_MASTER_MISO_PIN)
#define OP_ST500_SPI_MASTER_SS0_PIN     (0u == OP_ST500_REMOVE_SPI_MASTER_SS0_PIN)
#define OP_ST500_SPI_MASTER_SS1_PIN     (0u == OP_ST500_REMOVE_SPI_MASTER_SS1_PIN)
#define OP_ST500_SPI_MASTER_SS2_PIN     (0u == OP_ST500_REMOVE_SPI_MASTER_SS2_PIN)
#define OP_ST500_SPI_MASTER_SS3_PIN     (0u == OP_ST500_REMOVE_SPI_MASTER_SS3_PIN)
#define OP_ST500_SPI_SLAVE_PINS         (0u == OP_ST500_REMOVE_SPI_SLAVE_PINS)
#define OP_ST500_SPI_SLAVE_MOSI_PIN     (0u == OP_ST500_REMOVE_SPI_SLAVE_MOSI_PIN)
#define OP_ST500_SPI_SLAVE_MISO_PIN     (0u == OP_ST500_REMOVE_SPI_SLAVE_MISO_PIN)
#define OP_ST500_UART_TX_PIN            (0u == OP_ST500_REMOVE_UART_TX_PIN)
#define OP_ST500_UART_RX_TX_PIN         (0u == OP_ST500_REMOVE_UART_RX_TX_PIN)
#define OP_ST500_UART_RX_PIN            (0u == OP_ST500_REMOVE_UART_RX_PIN)
#define OP_ST500_UART_RX_WAKE_PIN       (0u == OP_ST500_REMOVE_UART_RX_WAKE_PIN)
#define OP_ST500_UART_RTS_PIN           (0u == OP_ST500_REMOVE_UART_RTS_PIN)
#define OP_ST500_UART_CTS_PIN           (0u == OP_ST500_REMOVE_UART_CTS_PIN)


/***************************************
*             Includes
****************************************/

#if (OP_ST500_RX_WAKE_SCL_MOSI_PIN)
    #include "OP_ST500_uart_rx_wake_i2c_scl_spi_mosi.h"
#endif /* (OP_ST500_RX_SCL_MOSI) */

#if (OP_ST500_RX_SCL_MOSI_PIN)
    #include "OP_ST500_uart_rx_i2c_scl_spi_mosi.h"
#endif /* (OP_ST500_RX_SCL_MOSI) */

#if (OP_ST500_TX_SDA_MISO_PIN)
    #include "OP_ST500_uart_tx_i2c_sda_spi_miso.h"
#endif /* (OP_ST500_TX_SDA_MISO) */

#if (OP_ST500_CTS_SCLK_PIN)
    #include "OP_ST500_uart_cts_spi_sclk.h"
#endif /* (OP_ST500_CTS_SCLK) */

#if (OP_ST500_RTS_SS0_PIN)
    #include "OP_ST500_uart_rts_spi_ss0.h"
#endif /* (OP_ST500_RTS_SS0_PIN) */

#if (OP_ST500_SS1_PIN)
    #include "OP_ST500_spi_ss1.h"
#endif /* (OP_ST500_SS1_PIN) */

#if (OP_ST500_SS2_PIN)
    #include "OP_ST500_spi_ss2.h"
#endif /* (OP_ST500_SS2_PIN) */

#if (OP_ST500_SS3_PIN)
    #include "OP_ST500_spi_ss3.h"
#endif /* (OP_ST500_SS3_PIN) */

#if (OP_ST500_I2C_PINS)
    #include "OP_ST500_scl.h"
    #include "OP_ST500_sda.h"
#endif /* (OP_ST500_I2C_PINS) */

#if (OP_ST500_SPI_MASTER_PINS)
#if (OP_ST500_SPI_MASTER_SCLK_PIN)
    #include "OP_ST500_sclk_m.h"
#endif /* (OP_ST500_SPI_MASTER_SCLK_PIN) */

#if (OP_ST500_SPI_MASTER_MOSI_PIN)
    #include "OP_ST500_mosi_m.h"
#endif /* (OP_ST500_SPI_MASTER_MOSI_PIN) */

#if (OP_ST500_SPI_MASTER_MISO_PIN)
    #include "OP_ST500_miso_m.h"
#endif /*(OP_ST500_SPI_MASTER_MISO_PIN) */
#endif /* (OP_ST500_SPI_MASTER_PINS) */

#if (OP_ST500_SPI_SLAVE_PINS)
    #include "OP_ST500_sclk_s.h"
    #include "OP_ST500_ss_s.h"

#if (OP_ST500_SPI_SLAVE_MOSI_PIN)
    #include "OP_ST500_mosi_s.h"
#endif /* (OP_ST500_SPI_SLAVE_MOSI_PIN) */

#if (OP_ST500_SPI_SLAVE_MISO_PIN)
    #include "OP_ST500_miso_s.h"
#endif /*(OP_ST500_SPI_SLAVE_MISO_PIN) */
#endif /* (OP_ST500_SPI_SLAVE_PINS) */

#if (OP_ST500_SPI_MASTER_SS0_PIN)
    #include "OP_ST500_ss0_m.h"
#endif /* (OP_ST500_SPI_MASTER_SS0_PIN) */

#if (OP_ST500_SPI_MASTER_SS1_PIN)
    #include "OP_ST500_ss1_m.h"
#endif /* (OP_ST500_SPI_MASTER_SS1_PIN) */

#if (OP_ST500_SPI_MASTER_SS2_PIN)
    #include "OP_ST500_ss2_m.h"
#endif /* (OP_ST500_SPI_MASTER_SS2_PIN) */

#if (OP_ST500_SPI_MASTER_SS3_PIN)
    #include "OP_ST500_ss3_m.h"
#endif /* (OP_ST500_SPI_MASTER_SS3_PIN) */

#if (OP_ST500_UART_TX_PIN)
    #include "OP_ST500_tx.h"
#endif /* (OP_ST500_UART_TX_PIN) */

#if (OP_ST500_UART_RX_TX_PIN)
    #include "OP_ST500_rx_tx.h"
#endif /* (OP_ST500_UART_RX_TX_PIN) */

#if (OP_ST500_UART_RX_PIN)
    #include "OP_ST500_rx.h"
#endif /* (OP_ST500_UART_RX_PIN) */

#if (OP_ST500_UART_RX_WAKE_PIN)
    #include "OP_ST500_rx_wake.h"
#endif /* (OP_ST500_UART_RX_WAKE_PIN) */

#if (OP_ST500_UART_RTS_PIN)
    #include "OP_ST500_rts.h"
#endif /* (OP_ST500_UART_RTS_PIN) */

#if (OP_ST500_UART_CTS_PIN)
    #include "OP_ST500_cts.h"
#endif /* (OP_ST500_UART_CTS_PIN) */


/***************************************
*              Registers
***************************************/

#if (OP_ST500_RX_SCL_MOSI_PIN)
    #define OP_ST500_RX_SCL_MOSI_HSIOM_REG   (*(reg32 *) OP_ST500_uart_rx_i2c_scl_spi_mosi__0__HSIOM)
    #define OP_ST500_RX_SCL_MOSI_HSIOM_PTR   ( (reg32 *) OP_ST500_uart_rx_i2c_scl_spi_mosi__0__HSIOM)
    
    #define OP_ST500_RX_SCL_MOSI_HSIOM_MASK      (OP_ST500_uart_rx_i2c_scl_spi_mosi__0__HSIOM_MASK)
    #define OP_ST500_RX_SCL_MOSI_HSIOM_POS       (OP_ST500_uart_rx_i2c_scl_spi_mosi__0__HSIOM_SHIFT)
    #define OP_ST500_RX_SCL_MOSI_HSIOM_SEL_GPIO  (OP_ST500_uart_rx_i2c_scl_spi_mosi__0__HSIOM_GPIO)
    #define OP_ST500_RX_SCL_MOSI_HSIOM_SEL_I2C   (OP_ST500_uart_rx_i2c_scl_spi_mosi__0__HSIOM_I2C)
    #define OP_ST500_RX_SCL_MOSI_HSIOM_SEL_SPI   (OP_ST500_uart_rx_i2c_scl_spi_mosi__0__HSIOM_SPI)
    #define OP_ST500_RX_SCL_MOSI_HSIOM_SEL_UART  (OP_ST500_uart_rx_i2c_scl_spi_mosi__0__HSIOM_UART)
    
#elif (OP_ST500_RX_WAKE_SCL_MOSI_PIN)
    #define OP_ST500_RX_WAKE_SCL_MOSI_HSIOM_REG   (*(reg32 *) OP_ST500_uart_rx_wake_i2c_scl_spi_mosi__0__HSIOM)
    #define OP_ST500_RX_WAKE_SCL_MOSI_HSIOM_PTR   ( (reg32 *) OP_ST500_uart_rx_wake_i2c_scl_spi_mosi__0__HSIOM)
    
    #define OP_ST500_RX_WAKE_SCL_MOSI_HSIOM_MASK      (OP_ST500_uart_rx_wake_i2c_scl_spi_mosi__0__HSIOM_MASK)
    #define OP_ST500_RX_WAKE_SCL_MOSI_HSIOM_POS       (OP_ST500_uart_rx_wake_i2c_scl_spi_mosi__0__HSIOM_SHIFT)
    #define OP_ST500_RX_WAKE_SCL_MOSI_HSIOM_SEL_GPIO  (OP_ST500_uart_rx_wake_i2c_scl_spi_mosi__0__HSIOM_GPIO)
    #define OP_ST500_RX_WAKE_SCL_MOSI_HSIOM_SEL_I2C   (OP_ST500_uart_rx_wake_i2c_scl_spi_mosi__0__HSIOM_I2C)
    #define OP_ST500_RX_WAKE_SCL_MOSI_HSIOM_SEL_SPI   (OP_ST500_uart_rx_wake_i2c_scl_spi_mosi__0__HSIOM_SPI)
    #define OP_ST500_RX_WAKE_SCL_MOSI_HSIOM_SEL_UART  (OP_ST500_uart_rx_wake_i2c_scl_spi_mosi__0__HSIOM_UART)    
   
    #define OP_ST500_RX_WAKE_SCL_MOSI_INTCFG_REG (*(reg32 *) OP_ST500_uart_rx_wake_i2c_scl_spi_mosi__0__INTCFG)
    #define OP_ST500_RX_WAKE_SCL_MOSI_INTCFG_PTR ( (reg32 *) OP_ST500_uart_rx_wake_i2c_scl_spi_mosi__0__INTCFG)
    #define OP_ST500_RX_WAKE_SCL_MOSI_INTCFG_TYPE_POS  (OP_ST500_uart_rx_wake_i2c_scl_spi_mosi__SHIFT)
    #define OP_ST500_RX_WAKE_SCL_MOSI_INTCFG_TYPE_MASK ((uint32) OP_ST500_INTCFG_TYPE_MASK << \
                                                                           OP_ST500_RX_WAKE_SCL_MOSI_INTCFG_TYPE_POS)
#else
    /* None of pins OP_ST500_RX_SCL_MOSI_PIN or OP_ST500_RX_WAKE_SCL_MOSI_PIN present.*/
#endif /* (OP_ST500_RX_SCL_MOSI_PIN) */

#if (OP_ST500_TX_SDA_MISO_PIN)
    #define OP_ST500_TX_SDA_MISO_HSIOM_REG   (*(reg32 *) OP_ST500_uart_tx_i2c_sda_spi_miso__0__HSIOM)
    #define OP_ST500_TX_SDA_MISO_HSIOM_PTR   ( (reg32 *) OP_ST500_uart_tx_i2c_sda_spi_miso__0__HSIOM)
    
    #define OP_ST500_TX_SDA_MISO_HSIOM_MASK      (OP_ST500_uart_tx_i2c_sda_spi_miso__0__HSIOM_MASK)
    #define OP_ST500_TX_SDA_MISO_HSIOM_POS       (OP_ST500_uart_tx_i2c_sda_spi_miso__0__HSIOM_SHIFT)
    #define OP_ST500_TX_SDA_MISO_HSIOM_SEL_GPIO  (OP_ST500_uart_tx_i2c_sda_spi_miso__0__HSIOM_GPIO)
    #define OP_ST500_TX_SDA_MISO_HSIOM_SEL_I2C   (OP_ST500_uart_tx_i2c_sda_spi_miso__0__HSIOM_I2C)
    #define OP_ST500_TX_SDA_MISO_HSIOM_SEL_SPI   (OP_ST500_uart_tx_i2c_sda_spi_miso__0__HSIOM_SPI)
    #define OP_ST500_TX_SDA_MISO_HSIOM_SEL_UART  (OP_ST500_uart_tx_i2c_sda_spi_miso__0__HSIOM_UART)
#endif /* (OP_ST500_TX_SDA_MISO_PIN) */

#if (OP_ST500_CTS_SCLK_PIN)
    #define OP_ST500_CTS_SCLK_HSIOM_REG   (*(reg32 *) OP_ST500_uart_cts_spi_sclk__0__HSIOM)
    #define OP_ST500_CTS_SCLK_HSIOM_PTR   ( (reg32 *) OP_ST500_uart_cts_spi_sclk__0__HSIOM)
    
    #define OP_ST500_CTS_SCLK_HSIOM_MASK      (OP_ST500_uart_cts_spi_sclk__0__HSIOM_MASK)
    #define OP_ST500_CTS_SCLK_HSIOM_POS       (OP_ST500_uart_cts_spi_sclk__0__HSIOM_SHIFT)
    #define OP_ST500_CTS_SCLK_HSIOM_SEL_GPIO  (OP_ST500_uart_cts_spi_sclk__0__HSIOM_GPIO)
    #define OP_ST500_CTS_SCLK_HSIOM_SEL_I2C   (OP_ST500_uart_cts_spi_sclk__0__HSIOM_I2C)
    #define OP_ST500_CTS_SCLK_HSIOM_SEL_SPI   (OP_ST500_uart_cts_spi_sclk__0__HSIOM_SPI)
    #define OP_ST500_CTS_SCLK_HSIOM_SEL_UART  (OP_ST500_uart_cts_spi_sclk__0__HSIOM_UART)
#endif /* (OP_ST500_CTS_SCLK_PIN) */

#if (OP_ST500_RTS_SS0_PIN)
    #define OP_ST500_RTS_SS0_HSIOM_REG   (*(reg32 *) OP_ST500_uart_rts_spi_ss0__0__HSIOM)
    #define OP_ST500_RTS_SS0_HSIOM_PTR   ( (reg32 *) OP_ST500_uart_rts_spi_ss0__0__HSIOM)
    
    #define OP_ST500_RTS_SS0_HSIOM_MASK      (OP_ST500_uart_rts_spi_ss0__0__HSIOM_MASK)
    #define OP_ST500_RTS_SS0_HSIOM_POS       (OP_ST500_uart_rts_spi_ss0__0__HSIOM_SHIFT)
    #define OP_ST500_RTS_SS0_HSIOM_SEL_GPIO  (OP_ST500_uart_rts_spi_ss0__0__HSIOM_GPIO)
    #define OP_ST500_RTS_SS0_HSIOM_SEL_I2C   (OP_ST500_uart_rts_spi_ss0__0__HSIOM_I2C)
    #define OP_ST500_RTS_SS0_HSIOM_SEL_SPI   (OP_ST500_uart_rts_spi_ss0__0__HSIOM_SPI)
#if !(OP_ST500_CY_SCBIP_V0 || OP_ST500_CY_SCBIP_V1)
    #define OP_ST500_RTS_SS0_HSIOM_SEL_UART  (OP_ST500_uart_rts_spi_ss0__0__HSIOM_UART)
#endif /* !(OP_ST500_CY_SCBIP_V0 || OP_ST500_CY_SCBIP_V1) */
#endif /* (OP_ST500_RTS_SS0_PIN) */

#if (OP_ST500_SS1_PIN)
    #define OP_ST500_SS1_HSIOM_REG  (*(reg32 *) OP_ST500_spi_ss1__0__HSIOM)
    #define OP_ST500_SS1_HSIOM_PTR  ( (reg32 *) OP_ST500_spi_ss1__0__HSIOM)
    
    #define OP_ST500_SS1_HSIOM_MASK     (OP_ST500_spi_ss1__0__HSIOM_MASK)
    #define OP_ST500_SS1_HSIOM_POS      (OP_ST500_spi_ss1__0__HSIOM_SHIFT)
    #define OP_ST500_SS1_HSIOM_SEL_GPIO (OP_ST500_spi_ss1__0__HSIOM_GPIO)
    #define OP_ST500_SS1_HSIOM_SEL_I2C  (OP_ST500_spi_ss1__0__HSIOM_I2C)
    #define OP_ST500_SS1_HSIOM_SEL_SPI  (OP_ST500_spi_ss1__0__HSIOM_SPI)
#endif /* (OP_ST500_SS1_PIN) */

#if (OP_ST500_SS2_PIN)
    #define OP_ST500_SS2_HSIOM_REG     (*(reg32 *) OP_ST500_spi_ss2__0__HSIOM)
    #define OP_ST500_SS2_HSIOM_PTR     ( (reg32 *) OP_ST500_spi_ss2__0__HSIOM)
    
    #define OP_ST500_SS2_HSIOM_MASK     (OP_ST500_spi_ss2__0__HSIOM_MASK)
    #define OP_ST500_SS2_HSIOM_POS      (OP_ST500_spi_ss2__0__HSIOM_SHIFT)
    #define OP_ST500_SS2_HSIOM_SEL_GPIO (OP_ST500_spi_ss2__0__HSIOM_GPIO)
    #define OP_ST500_SS2_HSIOM_SEL_I2C  (OP_ST500_spi_ss2__0__HSIOM_I2C)
    #define OP_ST500_SS2_HSIOM_SEL_SPI  (OP_ST500_spi_ss2__0__HSIOM_SPI)
#endif /* (OP_ST500_SS2_PIN) */

#if (OP_ST500_SS3_PIN)
    #define OP_ST500_SS3_HSIOM_REG     (*(reg32 *) OP_ST500_spi_ss3__0__HSIOM)
    #define OP_ST500_SS3_HSIOM_PTR     ( (reg32 *) OP_ST500_spi_ss3__0__HSIOM)
    
    #define OP_ST500_SS3_HSIOM_MASK     (OP_ST500_spi_ss3__0__HSIOM_MASK)
    #define OP_ST500_SS3_HSIOM_POS      (OP_ST500_spi_ss3__0__HSIOM_SHIFT)
    #define OP_ST500_SS3_HSIOM_SEL_GPIO (OP_ST500_spi_ss3__0__HSIOM_GPIO)
    #define OP_ST500_SS3_HSIOM_SEL_I2C  (OP_ST500_spi_ss3__0__HSIOM_I2C)
    #define OP_ST500_SS3_HSIOM_SEL_SPI  (OP_ST500_spi_ss3__0__HSIOM_SPI)
#endif /* (OP_ST500_SS3_PIN) */

#if (OP_ST500_I2C_PINS)
    #define OP_ST500_SCL_HSIOM_REG  (*(reg32 *) OP_ST500_scl__0__HSIOM)
    #define OP_ST500_SCL_HSIOM_PTR  ( (reg32 *) OP_ST500_scl__0__HSIOM)
    
    #define OP_ST500_SCL_HSIOM_MASK     (OP_ST500_scl__0__HSIOM_MASK)
    #define OP_ST500_SCL_HSIOM_POS      (OP_ST500_scl__0__HSIOM_SHIFT)
    #define OP_ST500_SCL_HSIOM_SEL_GPIO (OP_ST500_sda__0__HSIOM_GPIO)
    #define OP_ST500_SCL_HSIOM_SEL_I2C  (OP_ST500_sda__0__HSIOM_I2C)
    
    #define OP_ST500_SDA_HSIOM_REG  (*(reg32 *) OP_ST500_sda__0__HSIOM)
    #define OP_ST500_SDA_HSIOM_PTR  ( (reg32 *) OP_ST500_sda__0__HSIOM)
    
    #define OP_ST500_SDA_HSIOM_MASK     (OP_ST500_sda__0__HSIOM_MASK)
    #define OP_ST500_SDA_HSIOM_POS      (OP_ST500_sda__0__HSIOM_SHIFT)
    #define OP_ST500_SDA_HSIOM_SEL_GPIO (OP_ST500_sda__0__HSIOM_GPIO)
    #define OP_ST500_SDA_HSIOM_SEL_I2C  (OP_ST500_sda__0__HSIOM_I2C)
#endif /* (OP_ST500_I2C_PINS) */

#if (OP_ST500_SPI_SLAVE_PINS)
    #define OP_ST500_SCLK_S_HSIOM_REG   (*(reg32 *) OP_ST500_sclk_s__0__HSIOM)
    #define OP_ST500_SCLK_S_HSIOM_PTR   ( (reg32 *) OP_ST500_sclk_s__0__HSIOM)
    
    #define OP_ST500_SCLK_S_HSIOM_MASK      (OP_ST500_sclk_s__0__HSIOM_MASK)
    #define OP_ST500_SCLK_S_HSIOM_POS       (OP_ST500_sclk_s__0__HSIOM_SHIFT)
    #define OP_ST500_SCLK_S_HSIOM_SEL_GPIO  (OP_ST500_sclk_s__0__HSIOM_GPIO)
    #define OP_ST500_SCLK_S_HSIOM_SEL_SPI   (OP_ST500_sclk_s__0__HSIOM_SPI)
    
    #define OP_ST500_SS0_S_HSIOM_REG    (*(reg32 *) OP_ST500_ss0_s__0__HSIOM)
    #define OP_ST500_SS0_S_HSIOM_PTR    ( (reg32 *) OP_ST500_ss0_s__0__HSIOM)
    
    #define OP_ST500_SS0_S_HSIOM_MASK       (OP_ST500_ss0_s__0__HSIOM_MASK)
    #define OP_ST500_SS0_S_HSIOM_POS        (OP_ST500_ss0_s__0__HSIOM_SHIFT)
    #define OP_ST500_SS0_S_HSIOM_SEL_GPIO   (OP_ST500_ss0_s__0__HSIOM_GPIO)  
    #define OP_ST500_SS0_S_HSIOM_SEL_SPI    (OP_ST500_ss0_s__0__HSIOM_SPI)
#endif /* (OP_ST500_SPI_SLAVE_PINS) */

#if (OP_ST500_SPI_SLAVE_MOSI_PIN)
    #define OP_ST500_MOSI_S_HSIOM_REG   (*(reg32 *) OP_ST500_mosi_s__0__HSIOM)
    #define OP_ST500_MOSI_S_HSIOM_PTR   ( (reg32 *) OP_ST500_mosi_s__0__HSIOM)
    
    #define OP_ST500_MOSI_S_HSIOM_MASK      (OP_ST500_mosi_s__0__HSIOM_MASK)
    #define OP_ST500_MOSI_S_HSIOM_POS       (OP_ST500_mosi_s__0__HSIOM_SHIFT)
    #define OP_ST500_MOSI_S_HSIOM_SEL_GPIO  (OP_ST500_mosi_s__0__HSIOM_GPIO)
    #define OP_ST500_MOSI_S_HSIOM_SEL_SPI   (OP_ST500_mosi_s__0__HSIOM_SPI)
#endif /* (OP_ST500_SPI_SLAVE_MOSI_PIN) */

#if (OP_ST500_SPI_SLAVE_MISO_PIN)
    #define OP_ST500_MISO_S_HSIOM_REG   (*(reg32 *) OP_ST500_miso_s__0__HSIOM)
    #define OP_ST500_MISO_S_HSIOM_PTR   ( (reg32 *) OP_ST500_miso_s__0__HSIOM)
    
    #define OP_ST500_MISO_S_HSIOM_MASK      (OP_ST500_miso_s__0__HSIOM_MASK)
    #define OP_ST500_MISO_S_HSIOM_POS       (OP_ST500_miso_s__0__HSIOM_SHIFT)
    #define OP_ST500_MISO_S_HSIOM_SEL_GPIO  (OP_ST500_miso_s__0__HSIOM_GPIO)
    #define OP_ST500_MISO_S_HSIOM_SEL_SPI   (OP_ST500_miso_s__0__HSIOM_SPI)
#endif /* (OP_ST500_SPI_SLAVE_MISO_PIN) */

#if (OP_ST500_SPI_MASTER_MISO_PIN)
    #define OP_ST500_MISO_M_HSIOM_REG   (*(reg32 *) OP_ST500_miso_m__0__HSIOM)
    #define OP_ST500_MISO_M_HSIOM_PTR   ( (reg32 *) OP_ST500_miso_m__0__HSIOM)
    
    #define OP_ST500_MISO_M_HSIOM_MASK      (OP_ST500_miso_m__0__HSIOM_MASK)
    #define OP_ST500_MISO_M_HSIOM_POS       (OP_ST500_miso_m__0__HSIOM_SHIFT)
    #define OP_ST500_MISO_M_HSIOM_SEL_GPIO  (OP_ST500_miso_m__0__HSIOM_GPIO)
    #define OP_ST500_MISO_M_HSIOM_SEL_SPI   (OP_ST500_miso_m__0__HSIOM_SPI)
#endif /* (OP_ST500_SPI_MASTER_MISO_PIN) */

#if (OP_ST500_SPI_MASTER_MOSI_PIN)
    #define OP_ST500_MOSI_M_HSIOM_REG   (*(reg32 *) OP_ST500_mosi_m__0__HSIOM)
    #define OP_ST500_MOSI_M_HSIOM_PTR   ( (reg32 *) OP_ST500_mosi_m__0__HSIOM)
    
    #define OP_ST500_MOSI_M_HSIOM_MASK      (OP_ST500_mosi_m__0__HSIOM_MASK)
    #define OP_ST500_MOSI_M_HSIOM_POS       (OP_ST500_mosi_m__0__HSIOM_SHIFT)
    #define OP_ST500_MOSI_M_HSIOM_SEL_GPIO  (OP_ST500_mosi_m__0__HSIOM_GPIO)
    #define OP_ST500_MOSI_M_HSIOM_SEL_SPI   (OP_ST500_mosi_m__0__HSIOM_SPI)
#endif /* (OP_ST500_SPI_MASTER_MOSI_PIN) */

#if (OP_ST500_SPI_MASTER_SCLK_PIN)
    #define OP_ST500_SCLK_M_HSIOM_REG   (*(reg32 *) OP_ST500_sclk_m__0__HSIOM)
    #define OP_ST500_SCLK_M_HSIOM_PTR   ( (reg32 *) OP_ST500_sclk_m__0__HSIOM)
    
    #define OP_ST500_SCLK_M_HSIOM_MASK      (OP_ST500_sclk_m__0__HSIOM_MASK)
    #define OP_ST500_SCLK_M_HSIOM_POS       (OP_ST500_sclk_m__0__HSIOM_SHIFT)
    #define OP_ST500_SCLK_M_HSIOM_SEL_GPIO  (OP_ST500_sclk_m__0__HSIOM_GPIO)
    #define OP_ST500_SCLK_M_HSIOM_SEL_SPI   (OP_ST500_sclk_m__0__HSIOM_SPI)
#endif /* (OP_ST500_SPI_MASTER_SCLK_PIN) */

#if (OP_ST500_SPI_MASTER_SS0_PIN)
    #define OP_ST500_SS0_M_HSIOM_REG    (*(reg32 *) OP_ST500_ss0_m__0__HSIOM)
    #define OP_ST500_SS0_M_HSIOM_PTR    ( (reg32 *) OP_ST500_ss0_m__0__HSIOM)
    
    #define OP_ST500_SS0_M_HSIOM_MASK       (OP_ST500_ss0_m__0__HSIOM_MASK)
    #define OP_ST500_SS0_M_HSIOM_POS        (OP_ST500_ss0_m__0__HSIOM_SHIFT)
    #define OP_ST500_SS0_M_HSIOM_SEL_GPIO   (OP_ST500_ss0_m__0__HSIOM_GPIO)
    #define OP_ST500_SS0_M_HSIOM_SEL_SPI    (OP_ST500_ss0_m__0__HSIOM_SPI)
#endif /* (OP_ST500_SPI_MASTER_SS0_PIN) */

#if (OP_ST500_SPI_MASTER_SS1_PIN)
    #define OP_ST500_SS1_M_HSIOM_REG    (*(reg32 *) OP_ST500_ss1_m__0__HSIOM)
    #define OP_ST500_SS1_M_HSIOM_PTR    ( (reg32 *) OP_ST500_ss1_m__0__HSIOM)
    
    #define OP_ST500_SS1_M_HSIOM_MASK       (OP_ST500_ss1_m__0__HSIOM_MASK)
    #define OP_ST500_SS1_M_HSIOM_POS        (OP_ST500_ss1_m__0__HSIOM_SHIFT)
    #define OP_ST500_SS1_M_HSIOM_SEL_GPIO   (OP_ST500_ss1_m__0__HSIOM_GPIO)
    #define OP_ST500_SS1_M_HSIOM_SEL_SPI    (OP_ST500_ss1_m__0__HSIOM_SPI)
#endif /* (OP_ST500_SPI_MASTER_SS1_PIN) */

#if (OP_ST500_SPI_MASTER_SS2_PIN)
    #define OP_ST500_SS2_M_HSIOM_REG    (*(reg32 *) OP_ST500_ss2_m__0__HSIOM)
    #define OP_ST500_SS2_M_HSIOM_PTR    ( (reg32 *) OP_ST500_ss2_m__0__HSIOM)
    
    #define OP_ST500_SS2_M_HSIOM_MASK       (OP_ST500_ss2_m__0__HSIOM_MASK)
    #define OP_ST500_SS2_M_HSIOM_POS        (OP_ST500_ss2_m__0__HSIOM_SHIFT)
    #define OP_ST500_SS2_M_HSIOM_SEL_GPIO   (OP_ST500_ss2_m__0__HSIOM_GPIO)
    #define OP_ST500_SS2_M_HSIOM_SEL_SPI    (OP_ST500_ss2_m__0__HSIOM_SPI)
#endif /* (OP_ST500_SPI_MASTER_SS2_PIN) */

#if (OP_ST500_SPI_MASTER_SS3_PIN)
    #define OP_ST500_SS3_M_HSIOM_REG    (*(reg32 *) OP_ST500_ss3_m__0__HSIOM)
    #define OP_ST500_SS3_M_HSIOM_PTR    ( (reg32 *) OP_ST500_ss3_m__0__HSIOM)
    
    #define OP_ST500_SS3_M_HSIOM_MASK      (OP_ST500_ss3_m__0__HSIOM_MASK)
    #define OP_ST500_SS3_M_HSIOM_POS       (OP_ST500_ss3_m__0__HSIOM_SHIFT)
    #define OP_ST500_SS3_M_HSIOM_SEL_GPIO  (OP_ST500_ss3_m__0__HSIOM_GPIO)
    #define OP_ST500_SS3_M_HSIOM_SEL_SPI   (OP_ST500_ss3_m__0__HSIOM_SPI)
#endif /* (OP_ST500_SPI_MASTER_SS3_PIN) */

#if (OP_ST500_UART_RX_PIN)
    #define OP_ST500_RX_HSIOM_REG   (*(reg32 *) OP_ST500_rx__0__HSIOM)
    #define OP_ST500_RX_HSIOM_PTR   ( (reg32 *) OP_ST500_rx__0__HSIOM)
    
    #define OP_ST500_RX_HSIOM_MASK      (OP_ST500_rx__0__HSIOM_MASK)
    #define OP_ST500_RX_HSIOM_POS       (OP_ST500_rx__0__HSIOM_SHIFT)
    #define OP_ST500_RX_HSIOM_SEL_GPIO  (OP_ST500_rx__0__HSIOM_GPIO)
    #define OP_ST500_RX_HSIOM_SEL_UART  (OP_ST500_rx__0__HSIOM_UART)
#endif /* (OP_ST500_UART_RX_PIN) */

#if (OP_ST500_UART_RX_WAKE_PIN)
    #define OP_ST500_RX_WAKE_HSIOM_REG   (*(reg32 *) OP_ST500_rx_wake__0__HSIOM)
    #define OP_ST500_RX_WAKE_HSIOM_PTR   ( (reg32 *) OP_ST500_rx_wake__0__HSIOM)
    
    #define OP_ST500_RX_WAKE_HSIOM_MASK      (OP_ST500_rx_wake__0__HSIOM_MASK)
    #define OP_ST500_RX_WAKE_HSIOM_POS       (OP_ST500_rx_wake__0__HSIOM_SHIFT)
    #define OP_ST500_RX_WAKE_HSIOM_SEL_GPIO  (OP_ST500_rx_wake__0__HSIOM_GPIO)
    #define OP_ST500_RX_WAKE_HSIOM_SEL_UART  (OP_ST500_rx_wake__0__HSIOM_UART)
#endif /* (OP_ST500_UART_WAKE_RX_PIN) */

#if (OP_ST500_UART_CTS_PIN)
    #define OP_ST500_CTS_HSIOM_REG   (*(reg32 *) OP_ST500_cts__0__HSIOM)
    #define OP_ST500_CTS_HSIOM_PTR   ( (reg32 *) OP_ST500_cts__0__HSIOM)
    
    #define OP_ST500_CTS_HSIOM_MASK      (OP_ST500_cts__0__HSIOM_MASK)
    #define OP_ST500_CTS_HSIOM_POS       (OP_ST500_cts__0__HSIOM_SHIFT)
    #define OP_ST500_CTS_HSIOM_SEL_GPIO  (OP_ST500_cts__0__HSIOM_GPIO)
    #define OP_ST500_CTS_HSIOM_SEL_UART  (OP_ST500_cts__0__HSIOM_UART)
#endif /* (OP_ST500_UART_CTS_PIN) */

#if (OP_ST500_UART_TX_PIN)
    #define OP_ST500_TX_HSIOM_REG   (*(reg32 *) OP_ST500_tx__0__HSIOM)
    #define OP_ST500_TX_HSIOM_PTR   ( (reg32 *) OP_ST500_tx__0__HSIOM)
    
    #define OP_ST500_TX_HSIOM_MASK      (OP_ST500_tx__0__HSIOM_MASK)
    #define OP_ST500_TX_HSIOM_POS       (OP_ST500_tx__0__HSIOM_SHIFT)
    #define OP_ST500_TX_HSIOM_SEL_GPIO  (OP_ST500_tx__0__HSIOM_GPIO)
    #define OP_ST500_TX_HSIOM_SEL_UART  (OP_ST500_tx__0__HSIOM_UART)
#endif /* (OP_ST500_UART_TX_PIN) */

#if (OP_ST500_UART_RX_TX_PIN)
    #define OP_ST500_RX_TX_HSIOM_REG   (*(reg32 *) OP_ST500_rx_tx__0__HSIOM)
    #define OP_ST500_RX_TX_HSIOM_PTR   ( (reg32 *) OP_ST500_rx_tx__0__HSIOM)
    
    #define OP_ST500_RX_TX_HSIOM_MASK      (OP_ST500_rx_tx__0__HSIOM_MASK)
    #define OP_ST500_RX_TX_HSIOM_POS       (OP_ST500_rx_tx__0__HSIOM_SHIFT)
    #define OP_ST500_RX_TX_HSIOM_SEL_GPIO  (OP_ST500_rx_tx__0__HSIOM_GPIO)
    #define OP_ST500_RX_TX_HSIOM_SEL_UART  (OP_ST500_rx_tx__0__HSIOM_UART)
#endif /* (OP_ST500_UART_RX_TX_PIN) */

#if (OP_ST500_UART_RTS_PIN)
    #define OP_ST500_RTS_HSIOM_REG      (*(reg32 *) OP_ST500_rts__0__HSIOM)
    #define OP_ST500_RTS_HSIOM_PTR      ( (reg32 *) OP_ST500_rts__0__HSIOM)
    
    #define OP_ST500_RTS_HSIOM_MASK     (OP_ST500_rts__0__HSIOM_MASK)
    #define OP_ST500_RTS_HSIOM_POS      (OP_ST500_rts__0__HSIOM_SHIFT)    
    #define OP_ST500_RTS_HSIOM_SEL_GPIO (OP_ST500_rts__0__HSIOM_GPIO)
    #define OP_ST500_RTS_HSIOM_SEL_UART (OP_ST500_rts__0__HSIOM_UART)    
#endif /* (OP_ST500_UART_RTS_PIN) */


/***************************************
*        Registers Constants
***************************************/

/* HSIOM switch values. */ 
#define OP_ST500_HSIOM_DEF_SEL      (0x00u)
#define OP_ST500_HSIOM_GPIO_SEL     (0x00u)
/* The HSIOM values provided below are valid only for OP_ST500_CY_SCBIP_V0 
* and OP_ST500_CY_SCBIP_V1. It is not recommended to use them for 
* OP_ST500_CY_SCBIP_V2. Use pin name specific HSIOM constants provided 
* above instead for any SCB IP block version.
*/
#define OP_ST500_HSIOM_UART_SEL     (0x09u)
#define OP_ST500_HSIOM_I2C_SEL      (0x0Eu)
#define OP_ST500_HSIOM_SPI_SEL      (0x0Fu)

/* Pins settings index. */
#define OP_ST500_RX_WAKE_SCL_MOSI_PIN_INDEX   (0u)
#define OP_ST500_RX_SCL_MOSI_PIN_INDEX       (0u)
#define OP_ST500_TX_SDA_MISO_PIN_INDEX       (1u)
#define OP_ST500_CTS_SCLK_PIN_INDEX       (2u)
#define OP_ST500_RTS_SS0_PIN_INDEX       (3u)
#define OP_ST500_SS1_PIN_INDEX                  (4u)
#define OP_ST500_SS2_PIN_INDEX                  (5u)
#define OP_ST500_SS3_PIN_INDEX                  (6u)

/* Pins settings mask. */
#define OP_ST500_RX_WAKE_SCL_MOSI_PIN_MASK ((uint32) 0x01u << OP_ST500_RX_WAKE_SCL_MOSI_PIN_INDEX)
#define OP_ST500_RX_SCL_MOSI_PIN_MASK     ((uint32) 0x01u << OP_ST500_RX_SCL_MOSI_PIN_INDEX)
#define OP_ST500_TX_SDA_MISO_PIN_MASK     ((uint32) 0x01u << OP_ST500_TX_SDA_MISO_PIN_INDEX)
#define OP_ST500_CTS_SCLK_PIN_MASK     ((uint32) 0x01u << OP_ST500_CTS_SCLK_PIN_INDEX)
#define OP_ST500_RTS_SS0_PIN_MASK     ((uint32) 0x01u << OP_ST500_RTS_SS0_PIN_INDEX)
#define OP_ST500_SS1_PIN_MASK                ((uint32) 0x01u << OP_ST500_SS1_PIN_INDEX)
#define OP_ST500_SS2_PIN_MASK                ((uint32) 0x01u << OP_ST500_SS2_PIN_INDEX)
#define OP_ST500_SS3_PIN_MASK                ((uint32) 0x01u << OP_ST500_SS3_PIN_INDEX)

/* Pin interrupt constants. */
#define OP_ST500_INTCFG_TYPE_MASK           (0x03u)
#define OP_ST500_INTCFG_TYPE_FALLING_EDGE   (0x02u)

/* Pin Drive Mode constants. */
#define OP_ST500_PIN_DM_ALG_HIZ  (0u)
#define OP_ST500_PIN_DM_DIG_HIZ  (1u)
#define OP_ST500_PIN_DM_OD_LO    (4u)
#define OP_ST500_PIN_DM_STRONG   (6u)


/***************************************
*          Macro Definitions
***************************************/

/* Return drive mode of the pin */
#define OP_ST500_DM_MASK    (0x7u)
#define OP_ST500_DM_SIZE    (3u)
#define OP_ST500_GET_P4_PIN_DM(reg, pos) \
    ( ((reg) & (uint32) ((uint32) OP_ST500_DM_MASK << (OP_ST500_DM_SIZE * (pos)))) >> \
                                                              (OP_ST500_DM_SIZE * (pos)) )

#if (OP_ST500_TX_SDA_MISO_PIN)
    #define OP_ST500_CHECK_TX_SDA_MISO_PIN_USED \
                (OP_ST500_PIN_DM_ALG_HIZ != \
                    OP_ST500_GET_P4_PIN_DM(OP_ST500_uart_tx_i2c_sda_spi_miso_PC, \
                                                   OP_ST500_uart_tx_i2c_sda_spi_miso_SHIFT))
#endif /* (OP_ST500_TX_SDA_MISO_PIN) */

#if (OP_ST500_RTS_SS0_PIN)
    #define OP_ST500_CHECK_RTS_SS0_PIN_USED \
                (OP_ST500_PIN_DM_ALG_HIZ != \
                    OP_ST500_GET_P4_PIN_DM(OP_ST500_uart_rts_spi_ss0_PC, \
                                                   OP_ST500_uart_rts_spi_ss0_SHIFT))
#endif /* (OP_ST500_RTS_SS0_PIN) */

/* Set bits-mask in register */
#define OP_ST500_SET_REGISTER_BITS(reg, mask, pos, mode) \
                    do                                           \
                    {                                            \
                        (reg) = (((reg) & ((uint32) ~(uint32) (mask))) | ((uint32) ((uint32) (mode) << (pos)))); \
                    }while(0)

/* Set bit in the register */
#define OP_ST500_SET_REGISTER_BIT(reg, mask, val) \
                    ((val) ? ((reg) |= (mask)) : ((reg) &= ((uint32) ~((uint32) (mask)))))

#define OP_ST500_SET_HSIOM_SEL(reg, mask, pos, sel) OP_ST500_SET_REGISTER_BITS(reg, mask, pos, sel)
#define OP_ST500_SET_INCFG_TYPE(reg, mask, pos, intType) \
                                                        OP_ST500_SET_REGISTER_BITS(reg, mask, pos, intType)
#define OP_ST500_SET_INP_DIS(reg, mask, val) OP_ST500_SET_REGISTER_BIT(reg, mask, val)

/* OP_ST500_SET_I2C_SCL_DR(val) - Sets I2C SCL DR register.
*  OP_ST500_SET_I2C_SCL_HSIOM_SEL(sel) - Sets I2C SCL HSIOM settings.
*/
/* SCB I2C: scl signal */
#if (OP_ST500_CY_SCBIP_V0)
#if (OP_ST500_I2C_PINS)
    #define OP_ST500_SET_I2C_SCL_DR(val) OP_ST500_scl_Write(val)

    #define OP_ST500_SET_I2C_SCL_HSIOM_SEL(sel) \
                          OP_ST500_SET_HSIOM_SEL(OP_ST500_SCL_HSIOM_REG,  \
                                                         OP_ST500_SCL_HSIOM_MASK, \
                                                         OP_ST500_SCL_HSIOM_POS,  \
                                                         (sel))
    #define OP_ST500_WAIT_SCL_SET_HIGH  (0u == OP_ST500_scl_Read())

/* Unconfigured SCB: scl signal */
#elif (OP_ST500_RX_WAKE_SCL_MOSI_PIN)
    #define OP_ST500_SET_I2C_SCL_DR(val) \
                            OP_ST500_uart_rx_wake_i2c_scl_spi_mosi_Write(val)

    #define OP_ST500_SET_I2C_SCL_HSIOM_SEL(sel) \
                    OP_ST500_SET_HSIOM_SEL(OP_ST500_RX_WAKE_SCL_MOSI_HSIOM_REG,  \
                                                   OP_ST500_RX_WAKE_SCL_MOSI_HSIOM_MASK, \
                                                   OP_ST500_RX_WAKE_SCL_MOSI_HSIOM_POS,  \
                                                   (sel))

    #define OP_ST500_WAIT_SCL_SET_HIGH  (0u == OP_ST500_uart_rx_wake_i2c_scl_spi_mosi_Read())

#elif (OP_ST500_RX_SCL_MOSI_PIN)
    #define OP_ST500_SET_I2C_SCL_DR(val) \
                            OP_ST500_uart_rx_i2c_scl_spi_mosi_Write(val)


    #define OP_ST500_SET_I2C_SCL_HSIOM_SEL(sel) \
                            OP_ST500_SET_HSIOM_SEL(OP_ST500_RX_SCL_MOSI_HSIOM_REG,  \
                                                           OP_ST500_RX_SCL_MOSI_HSIOM_MASK, \
                                                           OP_ST500_RX_SCL_MOSI_HSIOM_POS,  \
                                                           (sel))

    #define OP_ST500_WAIT_SCL_SET_HIGH  (0u == OP_ST500_uart_rx_i2c_scl_spi_mosi_Read())

#else
    #define OP_ST500_SET_I2C_SCL_DR(val)        do{ /* Does nothing */ }while(0)
    #define OP_ST500_SET_I2C_SCL_HSIOM_SEL(sel) do{ /* Does nothing */ }while(0)

    #define OP_ST500_WAIT_SCL_SET_HIGH  (0u)
#endif /* (OP_ST500_I2C_PINS) */

/* SCB I2C: sda signal */
#if (OP_ST500_I2C_PINS)
    #define OP_ST500_WAIT_SDA_SET_HIGH  (0u == OP_ST500_sda_Read())
/* Unconfigured SCB: sda signal */
#elif (OP_ST500_TX_SDA_MISO_PIN)
    #define OP_ST500_WAIT_SDA_SET_HIGH  (0u == OP_ST500_uart_tx_i2c_sda_spi_miso_Read())
#else
    #define OP_ST500_WAIT_SDA_SET_HIGH  (0u)
#endif /* (OP_ST500_MOSI_SCL_RX_PIN) */
#endif /* (OP_ST500_CY_SCBIP_V0) */

/* Clear UART wakeup source */
#if (OP_ST500_RX_SCL_MOSI_PIN)
    #define OP_ST500_CLEAR_UART_RX_WAKE_INTR        do{ /* Does nothing */ }while(0)
    
#elif (OP_ST500_RX_WAKE_SCL_MOSI_PIN)
    #define OP_ST500_CLEAR_UART_RX_WAKE_INTR \
            do{                                      \
                (void) OP_ST500_uart_rx_wake_i2c_scl_spi_mosi_ClearInterrupt(); \
            }while(0)

#elif(OP_ST500_UART_RX_WAKE_PIN)
    #define OP_ST500_CLEAR_UART_RX_WAKE_INTR \
            do{                                      \
                (void) OP_ST500_rx_wake_ClearInterrupt(); \
            }while(0)
#else
#endif /* (OP_ST500_RX_SCL_MOSI_PIN) */


/***************************************
* The following code is DEPRECATED and
* must not be used.
***************************************/

/* Unconfigured pins */
#define OP_ST500_REMOVE_MOSI_SCL_RX_WAKE_PIN    OP_ST500_REMOVE_RX_WAKE_SCL_MOSI_PIN
#define OP_ST500_REMOVE_MOSI_SCL_RX_PIN         OP_ST500_REMOVE_RX_SCL_MOSI_PIN
#define OP_ST500_REMOVE_MISO_SDA_TX_PIN         OP_ST500_REMOVE_TX_SDA_MISO_PIN
#ifndef OP_ST500_REMOVE_SCLK_PIN
#define OP_ST500_REMOVE_SCLK_PIN                OP_ST500_REMOVE_CTS_SCLK_PIN
#endif /* OP_ST500_REMOVE_SCLK_PIN */
#ifndef OP_ST500_REMOVE_SS0_PIN
#define OP_ST500_REMOVE_SS0_PIN                 OP_ST500_REMOVE_RTS_SS0_PIN
#endif /* OP_ST500_REMOVE_SS0_PIN */

/* Unconfigured pins */
#define OP_ST500_MOSI_SCL_RX_WAKE_PIN   OP_ST500_RX_WAKE_SCL_MOSI_PIN
#define OP_ST500_MOSI_SCL_RX_PIN        OP_ST500_RX_SCL_MOSI_PIN
#define OP_ST500_MISO_SDA_TX_PIN        OP_ST500_TX_SDA_MISO_PIN
#ifndef OP_ST500_SCLK_PIN
#define OP_ST500_SCLK_PIN               OP_ST500_CTS_SCLK_PIN
#endif /* OP_ST500_SCLK_PIN */
#ifndef OP_ST500_SS0_PIN
#define OP_ST500_SS0_PIN                OP_ST500_RTS_SS0_PIN
#endif /* OP_ST500_SS0_PIN */

#if (OP_ST500_MOSI_SCL_RX_WAKE_PIN)
    #define OP_ST500_MOSI_SCL_RX_WAKE_HSIOM_REG     OP_ST500_RX_WAKE_SCL_MOSI_HSIOM_REG
    #define OP_ST500_MOSI_SCL_RX_WAKE_HSIOM_PTR     OP_ST500_RX_WAKE_SCL_MOSI_HSIOM_REG
    #define OP_ST500_MOSI_SCL_RX_WAKE_HSIOM_MASK    OP_ST500_RX_WAKE_SCL_MOSI_HSIOM_REG
    #define OP_ST500_MOSI_SCL_RX_WAKE_HSIOM_POS     OP_ST500_RX_WAKE_SCL_MOSI_HSIOM_REG

    #define OP_ST500_MOSI_SCL_RX_WAKE_INTCFG_REG    OP_ST500_RX_WAKE_SCL_MOSI_HSIOM_REG
    #define OP_ST500_MOSI_SCL_RX_WAKE_INTCFG_PTR    OP_ST500_RX_WAKE_SCL_MOSI_HSIOM_REG

    #define OP_ST500_MOSI_SCL_RX_WAKE_INTCFG_TYPE_POS   OP_ST500_RX_WAKE_SCL_MOSI_HSIOM_REG
    #define OP_ST500_MOSI_SCL_RX_WAKE_INTCFG_TYPE_MASK  OP_ST500_RX_WAKE_SCL_MOSI_HSIOM_REG
#endif /* (OP_ST500_RX_WAKE_SCL_MOSI_PIN) */

#if (OP_ST500_MOSI_SCL_RX_PIN)
    #define OP_ST500_MOSI_SCL_RX_HSIOM_REG      OP_ST500_RX_SCL_MOSI_HSIOM_REG
    #define OP_ST500_MOSI_SCL_RX_HSIOM_PTR      OP_ST500_RX_SCL_MOSI_HSIOM_PTR
    #define OP_ST500_MOSI_SCL_RX_HSIOM_MASK     OP_ST500_RX_SCL_MOSI_HSIOM_MASK
    #define OP_ST500_MOSI_SCL_RX_HSIOM_POS      OP_ST500_RX_SCL_MOSI_HSIOM_POS
#endif /* (OP_ST500_MOSI_SCL_RX_PIN) */

#if (OP_ST500_MISO_SDA_TX_PIN)
    #define OP_ST500_MISO_SDA_TX_HSIOM_REG      OP_ST500_TX_SDA_MISO_HSIOM_REG
    #define OP_ST500_MISO_SDA_TX_HSIOM_PTR      OP_ST500_TX_SDA_MISO_HSIOM_REG
    #define OP_ST500_MISO_SDA_TX_HSIOM_MASK     OP_ST500_TX_SDA_MISO_HSIOM_REG
    #define OP_ST500_MISO_SDA_TX_HSIOM_POS      OP_ST500_TX_SDA_MISO_HSIOM_REG
#endif /* (OP_ST500_MISO_SDA_TX_PIN_PIN) */

#if (OP_ST500_SCLK_PIN)
    #ifndef OP_ST500_SCLK_HSIOM_REG
    #define OP_ST500_SCLK_HSIOM_REG     OP_ST500_CTS_SCLK_HSIOM_REG
    #define OP_ST500_SCLK_HSIOM_PTR     OP_ST500_CTS_SCLK_HSIOM_PTR
    #define OP_ST500_SCLK_HSIOM_MASK    OP_ST500_CTS_SCLK_HSIOM_MASK
    #define OP_ST500_SCLK_HSIOM_POS     OP_ST500_CTS_SCLK_HSIOM_POS
    #endif /* OP_ST500_SCLK_HSIOM_REG */
#endif /* (OP_ST500_SCLK_PIN) */

#if (OP_ST500_SS0_PIN)
    #ifndef OP_ST500_SS0_HSIOM_REG
    #define OP_ST500_SS0_HSIOM_REG      OP_ST500_RTS_SS0_HSIOM_REG
    #define OP_ST500_SS0_HSIOM_PTR      OP_ST500_RTS_SS0_HSIOM_PTR
    #define OP_ST500_SS0_HSIOM_MASK     OP_ST500_RTS_SS0_HSIOM_MASK
    #define OP_ST500_SS0_HSIOM_POS      OP_ST500_RTS_SS0_HSIOM_POS
    #endif /* OP_ST500_SS0_HSIOM_REG */
#endif /* (OP_ST500_SS0_PIN) */

#define OP_ST500_MOSI_SCL_RX_WAKE_PIN_INDEX OP_ST500_RX_WAKE_SCL_MOSI_PIN_INDEX
#define OP_ST500_MOSI_SCL_RX_PIN_INDEX      OP_ST500_RX_SCL_MOSI_PIN_INDEX
#define OP_ST500_MISO_SDA_TX_PIN_INDEX      OP_ST500_TX_SDA_MISO_PIN_INDEX
#ifndef OP_ST500_SCLK_PIN_INDEX
#define OP_ST500_SCLK_PIN_INDEX             OP_ST500_CTS_SCLK_PIN_INDEX
#endif /* OP_ST500_SCLK_PIN_INDEX */
#ifndef OP_ST500_SS0_PIN_INDEX
#define OP_ST500_SS0_PIN_INDEX              OP_ST500_RTS_SS0_PIN_INDEX
#endif /* OP_ST500_SS0_PIN_INDEX */

#define OP_ST500_MOSI_SCL_RX_WAKE_PIN_MASK OP_ST500_RX_WAKE_SCL_MOSI_PIN_MASK
#define OP_ST500_MOSI_SCL_RX_PIN_MASK      OP_ST500_RX_SCL_MOSI_PIN_MASK
#define OP_ST500_MISO_SDA_TX_PIN_MASK      OP_ST500_TX_SDA_MISO_PIN_MASK
#ifndef OP_ST500_SCLK_PIN_MASK
#define OP_ST500_SCLK_PIN_MASK             OP_ST500_CTS_SCLK_PIN_MASK
#endif /* OP_ST500_SCLK_PIN_MASK */
#ifndef OP_ST500_SS0_PIN_MASK
#define OP_ST500_SS0_PIN_MASK              OP_ST500_RTS_SS0_PIN_MASK
#endif /* OP_ST500_SS0_PIN_MASK */

#endif /* (CY_SCB_PINS_OP_ST500_H) */


/* [] END OF FILE */
