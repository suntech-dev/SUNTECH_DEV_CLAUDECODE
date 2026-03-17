/***************************************************************************//**
* \file WIFI_PINS.h
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

#if !defined(CY_SCB_PINS_WIFI_H)
#define CY_SCB_PINS_WIFI_H

#include "cydevice_trm.h"
#include "cyfitter.h"
#include "cytypes.h"


/***************************************
*   Conditional Compilation Parameters
****************************************/

/* Unconfigured pins */
#define WIFI_REMOVE_RX_WAKE_SCL_MOSI_PIN  (1u)
#define WIFI_REMOVE_RX_SCL_MOSI_PIN      (1u)
#define WIFI_REMOVE_TX_SDA_MISO_PIN      (1u)
#define WIFI_REMOVE_CTS_SCLK_PIN      (1u)
#define WIFI_REMOVE_RTS_SS0_PIN      (1u)
#define WIFI_REMOVE_SS1_PIN                 (1u)
#define WIFI_REMOVE_SS2_PIN                 (1u)
#define WIFI_REMOVE_SS3_PIN                 (1u)

/* Mode defined pins */
#define WIFI_REMOVE_I2C_PINS                (1u)
#define WIFI_REMOVE_SPI_MASTER_PINS         (1u)
#define WIFI_REMOVE_SPI_MASTER_SCLK_PIN     (1u)
#define WIFI_REMOVE_SPI_MASTER_MOSI_PIN     (1u)
#define WIFI_REMOVE_SPI_MASTER_MISO_PIN     (1u)
#define WIFI_REMOVE_SPI_MASTER_SS0_PIN      (1u)
#define WIFI_REMOVE_SPI_MASTER_SS1_PIN      (1u)
#define WIFI_REMOVE_SPI_MASTER_SS2_PIN      (1u)
#define WIFI_REMOVE_SPI_MASTER_SS3_PIN      (1u)
#define WIFI_REMOVE_SPI_SLAVE_PINS          (1u)
#define WIFI_REMOVE_SPI_SLAVE_MOSI_PIN      (1u)
#define WIFI_REMOVE_SPI_SLAVE_MISO_PIN      (1u)
#define WIFI_REMOVE_UART_TX_PIN             (0u)
#define WIFI_REMOVE_UART_RX_TX_PIN          (1u)
#define WIFI_REMOVE_UART_RX_PIN             (0u)
#define WIFI_REMOVE_UART_RX_WAKE_PIN        (1u)
#define WIFI_REMOVE_UART_RTS_PIN            (1u)
#define WIFI_REMOVE_UART_CTS_PIN            (1u)

/* Unconfigured pins */
#define WIFI_RX_WAKE_SCL_MOSI_PIN (0u == WIFI_REMOVE_RX_WAKE_SCL_MOSI_PIN)
#define WIFI_RX_SCL_MOSI_PIN     (0u == WIFI_REMOVE_RX_SCL_MOSI_PIN)
#define WIFI_TX_SDA_MISO_PIN     (0u == WIFI_REMOVE_TX_SDA_MISO_PIN)
#define WIFI_CTS_SCLK_PIN     (0u == WIFI_REMOVE_CTS_SCLK_PIN)
#define WIFI_RTS_SS0_PIN     (0u == WIFI_REMOVE_RTS_SS0_PIN)
#define WIFI_SS1_PIN                (0u == WIFI_REMOVE_SS1_PIN)
#define WIFI_SS2_PIN                (0u == WIFI_REMOVE_SS2_PIN)
#define WIFI_SS3_PIN                (0u == WIFI_REMOVE_SS3_PIN)

/* Mode defined pins */
#define WIFI_I2C_PINS               (0u == WIFI_REMOVE_I2C_PINS)
#define WIFI_SPI_MASTER_PINS        (0u == WIFI_REMOVE_SPI_MASTER_PINS)
#define WIFI_SPI_MASTER_SCLK_PIN    (0u == WIFI_REMOVE_SPI_MASTER_SCLK_PIN)
#define WIFI_SPI_MASTER_MOSI_PIN    (0u == WIFI_REMOVE_SPI_MASTER_MOSI_PIN)
#define WIFI_SPI_MASTER_MISO_PIN    (0u == WIFI_REMOVE_SPI_MASTER_MISO_PIN)
#define WIFI_SPI_MASTER_SS0_PIN     (0u == WIFI_REMOVE_SPI_MASTER_SS0_PIN)
#define WIFI_SPI_MASTER_SS1_PIN     (0u == WIFI_REMOVE_SPI_MASTER_SS1_PIN)
#define WIFI_SPI_MASTER_SS2_PIN     (0u == WIFI_REMOVE_SPI_MASTER_SS2_PIN)
#define WIFI_SPI_MASTER_SS3_PIN     (0u == WIFI_REMOVE_SPI_MASTER_SS3_PIN)
#define WIFI_SPI_SLAVE_PINS         (0u == WIFI_REMOVE_SPI_SLAVE_PINS)
#define WIFI_SPI_SLAVE_MOSI_PIN     (0u == WIFI_REMOVE_SPI_SLAVE_MOSI_PIN)
#define WIFI_SPI_SLAVE_MISO_PIN     (0u == WIFI_REMOVE_SPI_SLAVE_MISO_PIN)
#define WIFI_UART_TX_PIN            (0u == WIFI_REMOVE_UART_TX_PIN)
#define WIFI_UART_RX_TX_PIN         (0u == WIFI_REMOVE_UART_RX_TX_PIN)
#define WIFI_UART_RX_PIN            (0u == WIFI_REMOVE_UART_RX_PIN)
#define WIFI_UART_RX_WAKE_PIN       (0u == WIFI_REMOVE_UART_RX_WAKE_PIN)
#define WIFI_UART_RTS_PIN           (0u == WIFI_REMOVE_UART_RTS_PIN)
#define WIFI_UART_CTS_PIN           (0u == WIFI_REMOVE_UART_CTS_PIN)


/***************************************
*             Includes
****************************************/

#if (WIFI_RX_WAKE_SCL_MOSI_PIN)
    #include "WIFI_uart_rx_wake_i2c_scl_spi_mosi.h"
#endif /* (WIFI_RX_SCL_MOSI) */

#if (WIFI_RX_SCL_MOSI_PIN)
    #include "WIFI_uart_rx_i2c_scl_spi_mosi.h"
#endif /* (WIFI_RX_SCL_MOSI) */

#if (WIFI_TX_SDA_MISO_PIN)
    #include "WIFI_uart_tx_i2c_sda_spi_miso.h"
#endif /* (WIFI_TX_SDA_MISO) */

#if (WIFI_CTS_SCLK_PIN)
    #include "WIFI_uart_cts_spi_sclk.h"
#endif /* (WIFI_CTS_SCLK) */

#if (WIFI_RTS_SS0_PIN)
    #include "WIFI_uart_rts_spi_ss0.h"
#endif /* (WIFI_RTS_SS0_PIN) */

#if (WIFI_SS1_PIN)
    #include "WIFI_spi_ss1.h"
#endif /* (WIFI_SS1_PIN) */

#if (WIFI_SS2_PIN)
    #include "WIFI_spi_ss2.h"
#endif /* (WIFI_SS2_PIN) */

#if (WIFI_SS3_PIN)
    #include "WIFI_spi_ss3.h"
#endif /* (WIFI_SS3_PIN) */

#if (WIFI_I2C_PINS)
    #include "WIFI_scl.h"
    #include "WIFI_sda.h"
#endif /* (WIFI_I2C_PINS) */

#if (WIFI_SPI_MASTER_PINS)
#if (WIFI_SPI_MASTER_SCLK_PIN)
    #include "WIFI_sclk_m.h"
#endif /* (WIFI_SPI_MASTER_SCLK_PIN) */

#if (WIFI_SPI_MASTER_MOSI_PIN)
    #include "WIFI_mosi_m.h"
#endif /* (WIFI_SPI_MASTER_MOSI_PIN) */

#if (WIFI_SPI_MASTER_MISO_PIN)
    #include "WIFI_miso_m.h"
#endif /*(WIFI_SPI_MASTER_MISO_PIN) */
#endif /* (WIFI_SPI_MASTER_PINS) */

#if (WIFI_SPI_SLAVE_PINS)
    #include "WIFI_sclk_s.h"
    #include "WIFI_ss_s.h"

#if (WIFI_SPI_SLAVE_MOSI_PIN)
    #include "WIFI_mosi_s.h"
#endif /* (WIFI_SPI_SLAVE_MOSI_PIN) */

#if (WIFI_SPI_SLAVE_MISO_PIN)
    #include "WIFI_miso_s.h"
#endif /*(WIFI_SPI_SLAVE_MISO_PIN) */
#endif /* (WIFI_SPI_SLAVE_PINS) */

#if (WIFI_SPI_MASTER_SS0_PIN)
    #include "WIFI_ss0_m.h"
#endif /* (WIFI_SPI_MASTER_SS0_PIN) */

#if (WIFI_SPI_MASTER_SS1_PIN)
    #include "WIFI_ss1_m.h"
#endif /* (WIFI_SPI_MASTER_SS1_PIN) */

#if (WIFI_SPI_MASTER_SS2_PIN)
    #include "WIFI_ss2_m.h"
#endif /* (WIFI_SPI_MASTER_SS2_PIN) */

#if (WIFI_SPI_MASTER_SS3_PIN)
    #include "WIFI_ss3_m.h"
#endif /* (WIFI_SPI_MASTER_SS3_PIN) */

#if (WIFI_UART_TX_PIN)
    #include "WIFI_tx.h"
#endif /* (WIFI_UART_TX_PIN) */

#if (WIFI_UART_RX_TX_PIN)
    #include "WIFI_rx_tx.h"
#endif /* (WIFI_UART_RX_TX_PIN) */

#if (WIFI_UART_RX_PIN)
    #include "WIFI_rx.h"
#endif /* (WIFI_UART_RX_PIN) */

#if (WIFI_UART_RX_WAKE_PIN)
    #include "WIFI_rx_wake.h"
#endif /* (WIFI_UART_RX_WAKE_PIN) */

#if (WIFI_UART_RTS_PIN)
    #include "WIFI_rts.h"
#endif /* (WIFI_UART_RTS_PIN) */

#if (WIFI_UART_CTS_PIN)
    #include "WIFI_cts.h"
#endif /* (WIFI_UART_CTS_PIN) */


/***************************************
*              Registers
***************************************/

#if (WIFI_RX_SCL_MOSI_PIN)
    #define WIFI_RX_SCL_MOSI_HSIOM_REG   (*(reg32 *) WIFI_uart_rx_i2c_scl_spi_mosi__0__HSIOM)
    #define WIFI_RX_SCL_MOSI_HSIOM_PTR   ( (reg32 *) WIFI_uart_rx_i2c_scl_spi_mosi__0__HSIOM)
    
    #define WIFI_RX_SCL_MOSI_HSIOM_MASK      (WIFI_uart_rx_i2c_scl_spi_mosi__0__HSIOM_MASK)
    #define WIFI_RX_SCL_MOSI_HSIOM_POS       (WIFI_uart_rx_i2c_scl_spi_mosi__0__HSIOM_SHIFT)
    #define WIFI_RX_SCL_MOSI_HSIOM_SEL_GPIO  (WIFI_uart_rx_i2c_scl_spi_mosi__0__HSIOM_GPIO)
    #define WIFI_RX_SCL_MOSI_HSIOM_SEL_I2C   (WIFI_uart_rx_i2c_scl_spi_mosi__0__HSIOM_I2C)
    #define WIFI_RX_SCL_MOSI_HSIOM_SEL_SPI   (WIFI_uart_rx_i2c_scl_spi_mosi__0__HSIOM_SPI)
    #define WIFI_RX_SCL_MOSI_HSIOM_SEL_UART  (WIFI_uart_rx_i2c_scl_spi_mosi__0__HSIOM_UART)
    
#elif (WIFI_RX_WAKE_SCL_MOSI_PIN)
    #define WIFI_RX_WAKE_SCL_MOSI_HSIOM_REG   (*(reg32 *) WIFI_uart_rx_wake_i2c_scl_spi_mosi__0__HSIOM)
    #define WIFI_RX_WAKE_SCL_MOSI_HSIOM_PTR   ( (reg32 *) WIFI_uart_rx_wake_i2c_scl_spi_mosi__0__HSIOM)
    
    #define WIFI_RX_WAKE_SCL_MOSI_HSIOM_MASK      (WIFI_uart_rx_wake_i2c_scl_spi_mosi__0__HSIOM_MASK)
    #define WIFI_RX_WAKE_SCL_MOSI_HSIOM_POS       (WIFI_uart_rx_wake_i2c_scl_spi_mosi__0__HSIOM_SHIFT)
    #define WIFI_RX_WAKE_SCL_MOSI_HSIOM_SEL_GPIO  (WIFI_uart_rx_wake_i2c_scl_spi_mosi__0__HSIOM_GPIO)
    #define WIFI_RX_WAKE_SCL_MOSI_HSIOM_SEL_I2C   (WIFI_uart_rx_wake_i2c_scl_spi_mosi__0__HSIOM_I2C)
    #define WIFI_RX_WAKE_SCL_MOSI_HSIOM_SEL_SPI   (WIFI_uart_rx_wake_i2c_scl_spi_mosi__0__HSIOM_SPI)
    #define WIFI_RX_WAKE_SCL_MOSI_HSIOM_SEL_UART  (WIFI_uart_rx_wake_i2c_scl_spi_mosi__0__HSIOM_UART)    
   
    #define WIFI_RX_WAKE_SCL_MOSI_INTCFG_REG (*(reg32 *) WIFI_uart_rx_wake_i2c_scl_spi_mosi__0__INTCFG)
    #define WIFI_RX_WAKE_SCL_MOSI_INTCFG_PTR ( (reg32 *) WIFI_uart_rx_wake_i2c_scl_spi_mosi__0__INTCFG)
    #define WIFI_RX_WAKE_SCL_MOSI_INTCFG_TYPE_POS  (WIFI_uart_rx_wake_i2c_scl_spi_mosi__SHIFT)
    #define WIFI_RX_WAKE_SCL_MOSI_INTCFG_TYPE_MASK ((uint32) WIFI_INTCFG_TYPE_MASK << \
                                                                           WIFI_RX_WAKE_SCL_MOSI_INTCFG_TYPE_POS)
#else
    /* None of pins WIFI_RX_SCL_MOSI_PIN or WIFI_RX_WAKE_SCL_MOSI_PIN present.*/
#endif /* (WIFI_RX_SCL_MOSI_PIN) */

#if (WIFI_TX_SDA_MISO_PIN)
    #define WIFI_TX_SDA_MISO_HSIOM_REG   (*(reg32 *) WIFI_uart_tx_i2c_sda_spi_miso__0__HSIOM)
    #define WIFI_TX_SDA_MISO_HSIOM_PTR   ( (reg32 *) WIFI_uart_tx_i2c_sda_spi_miso__0__HSIOM)
    
    #define WIFI_TX_SDA_MISO_HSIOM_MASK      (WIFI_uart_tx_i2c_sda_spi_miso__0__HSIOM_MASK)
    #define WIFI_TX_SDA_MISO_HSIOM_POS       (WIFI_uart_tx_i2c_sda_spi_miso__0__HSIOM_SHIFT)
    #define WIFI_TX_SDA_MISO_HSIOM_SEL_GPIO  (WIFI_uart_tx_i2c_sda_spi_miso__0__HSIOM_GPIO)
    #define WIFI_TX_SDA_MISO_HSIOM_SEL_I2C   (WIFI_uart_tx_i2c_sda_spi_miso__0__HSIOM_I2C)
    #define WIFI_TX_SDA_MISO_HSIOM_SEL_SPI   (WIFI_uart_tx_i2c_sda_spi_miso__0__HSIOM_SPI)
    #define WIFI_TX_SDA_MISO_HSIOM_SEL_UART  (WIFI_uart_tx_i2c_sda_spi_miso__0__HSIOM_UART)
#endif /* (WIFI_TX_SDA_MISO_PIN) */

#if (WIFI_CTS_SCLK_PIN)
    #define WIFI_CTS_SCLK_HSIOM_REG   (*(reg32 *) WIFI_uart_cts_spi_sclk__0__HSIOM)
    #define WIFI_CTS_SCLK_HSIOM_PTR   ( (reg32 *) WIFI_uart_cts_spi_sclk__0__HSIOM)
    
    #define WIFI_CTS_SCLK_HSIOM_MASK      (WIFI_uart_cts_spi_sclk__0__HSIOM_MASK)
    #define WIFI_CTS_SCLK_HSIOM_POS       (WIFI_uart_cts_spi_sclk__0__HSIOM_SHIFT)
    #define WIFI_CTS_SCLK_HSIOM_SEL_GPIO  (WIFI_uart_cts_spi_sclk__0__HSIOM_GPIO)
    #define WIFI_CTS_SCLK_HSIOM_SEL_I2C   (WIFI_uart_cts_spi_sclk__0__HSIOM_I2C)
    #define WIFI_CTS_SCLK_HSIOM_SEL_SPI   (WIFI_uart_cts_spi_sclk__0__HSIOM_SPI)
    #define WIFI_CTS_SCLK_HSIOM_SEL_UART  (WIFI_uart_cts_spi_sclk__0__HSIOM_UART)
#endif /* (WIFI_CTS_SCLK_PIN) */

#if (WIFI_RTS_SS0_PIN)
    #define WIFI_RTS_SS0_HSIOM_REG   (*(reg32 *) WIFI_uart_rts_spi_ss0__0__HSIOM)
    #define WIFI_RTS_SS0_HSIOM_PTR   ( (reg32 *) WIFI_uart_rts_spi_ss0__0__HSIOM)
    
    #define WIFI_RTS_SS0_HSIOM_MASK      (WIFI_uart_rts_spi_ss0__0__HSIOM_MASK)
    #define WIFI_RTS_SS0_HSIOM_POS       (WIFI_uart_rts_spi_ss0__0__HSIOM_SHIFT)
    #define WIFI_RTS_SS0_HSIOM_SEL_GPIO  (WIFI_uart_rts_spi_ss0__0__HSIOM_GPIO)
    #define WIFI_RTS_SS0_HSIOM_SEL_I2C   (WIFI_uart_rts_spi_ss0__0__HSIOM_I2C)
    #define WIFI_RTS_SS0_HSIOM_SEL_SPI   (WIFI_uart_rts_spi_ss0__0__HSIOM_SPI)
#if !(WIFI_CY_SCBIP_V0 || WIFI_CY_SCBIP_V1)
    #define WIFI_RTS_SS0_HSIOM_SEL_UART  (WIFI_uart_rts_spi_ss0__0__HSIOM_UART)
#endif /* !(WIFI_CY_SCBIP_V0 || WIFI_CY_SCBIP_V1) */
#endif /* (WIFI_RTS_SS0_PIN) */

#if (WIFI_SS1_PIN)
    #define WIFI_SS1_HSIOM_REG  (*(reg32 *) WIFI_spi_ss1__0__HSIOM)
    #define WIFI_SS1_HSIOM_PTR  ( (reg32 *) WIFI_spi_ss1__0__HSIOM)
    
    #define WIFI_SS1_HSIOM_MASK     (WIFI_spi_ss1__0__HSIOM_MASK)
    #define WIFI_SS1_HSIOM_POS      (WIFI_spi_ss1__0__HSIOM_SHIFT)
    #define WIFI_SS1_HSIOM_SEL_GPIO (WIFI_spi_ss1__0__HSIOM_GPIO)
    #define WIFI_SS1_HSIOM_SEL_I2C  (WIFI_spi_ss1__0__HSIOM_I2C)
    #define WIFI_SS1_HSIOM_SEL_SPI  (WIFI_spi_ss1__0__HSIOM_SPI)
#endif /* (WIFI_SS1_PIN) */

#if (WIFI_SS2_PIN)
    #define WIFI_SS2_HSIOM_REG     (*(reg32 *) WIFI_spi_ss2__0__HSIOM)
    #define WIFI_SS2_HSIOM_PTR     ( (reg32 *) WIFI_spi_ss2__0__HSIOM)
    
    #define WIFI_SS2_HSIOM_MASK     (WIFI_spi_ss2__0__HSIOM_MASK)
    #define WIFI_SS2_HSIOM_POS      (WIFI_spi_ss2__0__HSIOM_SHIFT)
    #define WIFI_SS2_HSIOM_SEL_GPIO (WIFI_spi_ss2__0__HSIOM_GPIO)
    #define WIFI_SS2_HSIOM_SEL_I2C  (WIFI_spi_ss2__0__HSIOM_I2C)
    #define WIFI_SS2_HSIOM_SEL_SPI  (WIFI_spi_ss2__0__HSIOM_SPI)
#endif /* (WIFI_SS2_PIN) */

#if (WIFI_SS3_PIN)
    #define WIFI_SS3_HSIOM_REG     (*(reg32 *) WIFI_spi_ss3__0__HSIOM)
    #define WIFI_SS3_HSIOM_PTR     ( (reg32 *) WIFI_spi_ss3__0__HSIOM)
    
    #define WIFI_SS3_HSIOM_MASK     (WIFI_spi_ss3__0__HSIOM_MASK)
    #define WIFI_SS3_HSIOM_POS      (WIFI_spi_ss3__0__HSIOM_SHIFT)
    #define WIFI_SS3_HSIOM_SEL_GPIO (WIFI_spi_ss3__0__HSIOM_GPIO)
    #define WIFI_SS3_HSIOM_SEL_I2C  (WIFI_spi_ss3__0__HSIOM_I2C)
    #define WIFI_SS3_HSIOM_SEL_SPI  (WIFI_spi_ss3__0__HSIOM_SPI)
#endif /* (WIFI_SS3_PIN) */

#if (WIFI_I2C_PINS)
    #define WIFI_SCL_HSIOM_REG  (*(reg32 *) WIFI_scl__0__HSIOM)
    #define WIFI_SCL_HSIOM_PTR  ( (reg32 *) WIFI_scl__0__HSIOM)
    
    #define WIFI_SCL_HSIOM_MASK     (WIFI_scl__0__HSIOM_MASK)
    #define WIFI_SCL_HSIOM_POS      (WIFI_scl__0__HSIOM_SHIFT)
    #define WIFI_SCL_HSIOM_SEL_GPIO (WIFI_sda__0__HSIOM_GPIO)
    #define WIFI_SCL_HSIOM_SEL_I2C  (WIFI_sda__0__HSIOM_I2C)
    
    #define WIFI_SDA_HSIOM_REG  (*(reg32 *) WIFI_sda__0__HSIOM)
    #define WIFI_SDA_HSIOM_PTR  ( (reg32 *) WIFI_sda__0__HSIOM)
    
    #define WIFI_SDA_HSIOM_MASK     (WIFI_sda__0__HSIOM_MASK)
    #define WIFI_SDA_HSIOM_POS      (WIFI_sda__0__HSIOM_SHIFT)
    #define WIFI_SDA_HSIOM_SEL_GPIO (WIFI_sda__0__HSIOM_GPIO)
    #define WIFI_SDA_HSIOM_SEL_I2C  (WIFI_sda__0__HSIOM_I2C)
#endif /* (WIFI_I2C_PINS) */

#if (WIFI_SPI_SLAVE_PINS)
    #define WIFI_SCLK_S_HSIOM_REG   (*(reg32 *) WIFI_sclk_s__0__HSIOM)
    #define WIFI_SCLK_S_HSIOM_PTR   ( (reg32 *) WIFI_sclk_s__0__HSIOM)
    
    #define WIFI_SCLK_S_HSIOM_MASK      (WIFI_sclk_s__0__HSIOM_MASK)
    #define WIFI_SCLK_S_HSIOM_POS       (WIFI_sclk_s__0__HSIOM_SHIFT)
    #define WIFI_SCLK_S_HSIOM_SEL_GPIO  (WIFI_sclk_s__0__HSIOM_GPIO)
    #define WIFI_SCLK_S_HSIOM_SEL_SPI   (WIFI_sclk_s__0__HSIOM_SPI)
    
    #define WIFI_SS0_S_HSIOM_REG    (*(reg32 *) WIFI_ss0_s__0__HSIOM)
    #define WIFI_SS0_S_HSIOM_PTR    ( (reg32 *) WIFI_ss0_s__0__HSIOM)
    
    #define WIFI_SS0_S_HSIOM_MASK       (WIFI_ss0_s__0__HSIOM_MASK)
    #define WIFI_SS0_S_HSIOM_POS        (WIFI_ss0_s__0__HSIOM_SHIFT)
    #define WIFI_SS0_S_HSIOM_SEL_GPIO   (WIFI_ss0_s__0__HSIOM_GPIO)  
    #define WIFI_SS0_S_HSIOM_SEL_SPI    (WIFI_ss0_s__0__HSIOM_SPI)
#endif /* (WIFI_SPI_SLAVE_PINS) */

#if (WIFI_SPI_SLAVE_MOSI_PIN)
    #define WIFI_MOSI_S_HSIOM_REG   (*(reg32 *) WIFI_mosi_s__0__HSIOM)
    #define WIFI_MOSI_S_HSIOM_PTR   ( (reg32 *) WIFI_mosi_s__0__HSIOM)
    
    #define WIFI_MOSI_S_HSIOM_MASK      (WIFI_mosi_s__0__HSIOM_MASK)
    #define WIFI_MOSI_S_HSIOM_POS       (WIFI_mosi_s__0__HSIOM_SHIFT)
    #define WIFI_MOSI_S_HSIOM_SEL_GPIO  (WIFI_mosi_s__0__HSIOM_GPIO)
    #define WIFI_MOSI_S_HSIOM_SEL_SPI   (WIFI_mosi_s__0__HSIOM_SPI)
#endif /* (WIFI_SPI_SLAVE_MOSI_PIN) */

#if (WIFI_SPI_SLAVE_MISO_PIN)
    #define WIFI_MISO_S_HSIOM_REG   (*(reg32 *) WIFI_miso_s__0__HSIOM)
    #define WIFI_MISO_S_HSIOM_PTR   ( (reg32 *) WIFI_miso_s__0__HSIOM)
    
    #define WIFI_MISO_S_HSIOM_MASK      (WIFI_miso_s__0__HSIOM_MASK)
    #define WIFI_MISO_S_HSIOM_POS       (WIFI_miso_s__0__HSIOM_SHIFT)
    #define WIFI_MISO_S_HSIOM_SEL_GPIO  (WIFI_miso_s__0__HSIOM_GPIO)
    #define WIFI_MISO_S_HSIOM_SEL_SPI   (WIFI_miso_s__0__HSIOM_SPI)
#endif /* (WIFI_SPI_SLAVE_MISO_PIN) */

#if (WIFI_SPI_MASTER_MISO_PIN)
    #define WIFI_MISO_M_HSIOM_REG   (*(reg32 *) WIFI_miso_m__0__HSIOM)
    #define WIFI_MISO_M_HSIOM_PTR   ( (reg32 *) WIFI_miso_m__0__HSIOM)
    
    #define WIFI_MISO_M_HSIOM_MASK      (WIFI_miso_m__0__HSIOM_MASK)
    #define WIFI_MISO_M_HSIOM_POS       (WIFI_miso_m__0__HSIOM_SHIFT)
    #define WIFI_MISO_M_HSIOM_SEL_GPIO  (WIFI_miso_m__0__HSIOM_GPIO)
    #define WIFI_MISO_M_HSIOM_SEL_SPI   (WIFI_miso_m__0__HSIOM_SPI)
#endif /* (WIFI_SPI_MASTER_MISO_PIN) */

#if (WIFI_SPI_MASTER_MOSI_PIN)
    #define WIFI_MOSI_M_HSIOM_REG   (*(reg32 *) WIFI_mosi_m__0__HSIOM)
    #define WIFI_MOSI_M_HSIOM_PTR   ( (reg32 *) WIFI_mosi_m__0__HSIOM)
    
    #define WIFI_MOSI_M_HSIOM_MASK      (WIFI_mosi_m__0__HSIOM_MASK)
    #define WIFI_MOSI_M_HSIOM_POS       (WIFI_mosi_m__0__HSIOM_SHIFT)
    #define WIFI_MOSI_M_HSIOM_SEL_GPIO  (WIFI_mosi_m__0__HSIOM_GPIO)
    #define WIFI_MOSI_M_HSIOM_SEL_SPI   (WIFI_mosi_m__0__HSIOM_SPI)
#endif /* (WIFI_SPI_MASTER_MOSI_PIN) */

#if (WIFI_SPI_MASTER_SCLK_PIN)
    #define WIFI_SCLK_M_HSIOM_REG   (*(reg32 *) WIFI_sclk_m__0__HSIOM)
    #define WIFI_SCLK_M_HSIOM_PTR   ( (reg32 *) WIFI_sclk_m__0__HSIOM)
    
    #define WIFI_SCLK_M_HSIOM_MASK      (WIFI_sclk_m__0__HSIOM_MASK)
    #define WIFI_SCLK_M_HSIOM_POS       (WIFI_sclk_m__0__HSIOM_SHIFT)
    #define WIFI_SCLK_M_HSIOM_SEL_GPIO  (WIFI_sclk_m__0__HSIOM_GPIO)
    #define WIFI_SCLK_M_HSIOM_SEL_SPI   (WIFI_sclk_m__0__HSIOM_SPI)
#endif /* (WIFI_SPI_MASTER_SCLK_PIN) */

#if (WIFI_SPI_MASTER_SS0_PIN)
    #define WIFI_SS0_M_HSIOM_REG    (*(reg32 *) WIFI_ss0_m__0__HSIOM)
    #define WIFI_SS0_M_HSIOM_PTR    ( (reg32 *) WIFI_ss0_m__0__HSIOM)
    
    #define WIFI_SS0_M_HSIOM_MASK       (WIFI_ss0_m__0__HSIOM_MASK)
    #define WIFI_SS0_M_HSIOM_POS        (WIFI_ss0_m__0__HSIOM_SHIFT)
    #define WIFI_SS0_M_HSIOM_SEL_GPIO   (WIFI_ss0_m__0__HSIOM_GPIO)
    #define WIFI_SS0_M_HSIOM_SEL_SPI    (WIFI_ss0_m__0__HSIOM_SPI)
#endif /* (WIFI_SPI_MASTER_SS0_PIN) */

#if (WIFI_SPI_MASTER_SS1_PIN)
    #define WIFI_SS1_M_HSIOM_REG    (*(reg32 *) WIFI_ss1_m__0__HSIOM)
    #define WIFI_SS1_M_HSIOM_PTR    ( (reg32 *) WIFI_ss1_m__0__HSIOM)
    
    #define WIFI_SS1_M_HSIOM_MASK       (WIFI_ss1_m__0__HSIOM_MASK)
    #define WIFI_SS1_M_HSIOM_POS        (WIFI_ss1_m__0__HSIOM_SHIFT)
    #define WIFI_SS1_M_HSIOM_SEL_GPIO   (WIFI_ss1_m__0__HSIOM_GPIO)
    #define WIFI_SS1_M_HSIOM_SEL_SPI    (WIFI_ss1_m__0__HSIOM_SPI)
#endif /* (WIFI_SPI_MASTER_SS1_PIN) */

#if (WIFI_SPI_MASTER_SS2_PIN)
    #define WIFI_SS2_M_HSIOM_REG    (*(reg32 *) WIFI_ss2_m__0__HSIOM)
    #define WIFI_SS2_M_HSIOM_PTR    ( (reg32 *) WIFI_ss2_m__0__HSIOM)
    
    #define WIFI_SS2_M_HSIOM_MASK       (WIFI_ss2_m__0__HSIOM_MASK)
    #define WIFI_SS2_M_HSIOM_POS        (WIFI_ss2_m__0__HSIOM_SHIFT)
    #define WIFI_SS2_M_HSIOM_SEL_GPIO   (WIFI_ss2_m__0__HSIOM_GPIO)
    #define WIFI_SS2_M_HSIOM_SEL_SPI    (WIFI_ss2_m__0__HSIOM_SPI)
#endif /* (WIFI_SPI_MASTER_SS2_PIN) */

#if (WIFI_SPI_MASTER_SS3_PIN)
    #define WIFI_SS3_M_HSIOM_REG    (*(reg32 *) WIFI_ss3_m__0__HSIOM)
    #define WIFI_SS3_M_HSIOM_PTR    ( (reg32 *) WIFI_ss3_m__0__HSIOM)
    
    #define WIFI_SS3_M_HSIOM_MASK      (WIFI_ss3_m__0__HSIOM_MASK)
    #define WIFI_SS3_M_HSIOM_POS       (WIFI_ss3_m__0__HSIOM_SHIFT)
    #define WIFI_SS3_M_HSIOM_SEL_GPIO  (WIFI_ss3_m__0__HSIOM_GPIO)
    #define WIFI_SS3_M_HSIOM_SEL_SPI   (WIFI_ss3_m__0__HSIOM_SPI)
#endif /* (WIFI_SPI_MASTER_SS3_PIN) */

#if (WIFI_UART_RX_PIN)
    #define WIFI_RX_HSIOM_REG   (*(reg32 *) WIFI_rx__0__HSIOM)
    #define WIFI_RX_HSIOM_PTR   ( (reg32 *) WIFI_rx__0__HSIOM)
    
    #define WIFI_RX_HSIOM_MASK      (WIFI_rx__0__HSIOM_MASK)
    #define WIFI_RX_HSIOM_POS       (WIFI_rx__0__HSIOM_SHIFT)
    #define WIFI_RX_HSIOM_SEL_GPIO  (WIFI_rx__0__HSIOM_GPIO)
    #define WIFI_RX_HSIOM_SEL_UART  (WIFI_rx__0__HSIOM_UART)
#endif /* (WIFI_UART_RX_PIN) */

#if (WIFI_UART_RX_WAKE_PIN)
    #define WIFI_RX_WAKE_HSIOM_REG   (*(reg32 *) WIFI_rx_wake__0__HSIOM)
    #define WIFI_RX_WAKE_HSIOM_PTR   ( (reg32 *) WIFI_rx_wake__0__HSIOM)
    
    #define WIFI_RX_WAKE_HSIOM_MASK      (WIFI_rx_wake__0__HSIOM_MASK)
    #define WIFI_RX_WAKE_HSIOM_POS       (WIFI_rx_wake__0__HSIOM_SHIFT)
    #define WIFI_RX_WAKE_HSIOM_SEL_GPIO  (WIFI_rx_wake__0__HSIOM_GPIO)
    #define WIFI_RX_WAKE_HSIOM_SEL_UART  (WIFI_rx_wake__0__HSIOM_UART)
#endif /* (WIFI_UART_WAKE_RX_PIN) */

#if (WIFI_UART_CTS_PIN)
    #define WIFI_CTS_HSIOM_REG   (*(reg32 *) WIFI_cts__0__HSIOM)
    #define WIFI_CTS_HSIOM_PTR   ( (reg32 *) WIFI_cts__0__HSIOM)
    
    #define WIFI_CTS_HSIOM_MASK      (WIFI_cts__0__HSIOM_MASK)
    #define WIFI_CTS_HSIOM_POS       (WIFI_cts__0__HSIOM_SHIFT)
    #define WIFI_CTS_HSIOM_SEL_GPIO  (WIFI_cts__0__HSIOM_GPIO)
    #define WIFI_CTS_HSIOM_SEL_UART  (WIFI_cts__0__HSIOM_UART)
#endif /* (WIFI_UART_CTS_PIN) */

#if (WIFI_UART_TX_PIN)
    #define WIFI_TX_HSIOM_REG   (*(reg32 *) WIFI_tx__0__HSIOM)
    #define WIFI_TX_HSIOM_PTR   ( (reg32 *) WIFI_tx__0__HSIOM)
    
    #define WIFI_TX_HSIOM_MASK      (WIFI_tx__0__HSIOM_MASK)
    #define WIFI_TX_HSIOM_POS       (WIFI_tx__0__HSIOM_SHIFT)
    #define WIFI_TX_HSIOM_SEL_GPIO  (WIFI_tx__0__HSIOM_GPIO)
    #define WIFI_TX_HSIOM_SEL_UART  (WIFI_tx__0__HSIOM_UART)
#endif /* (WIFI_UART_TX_PIN) */

#if (WIFI_UART_RX_TX_PIN)
    #define WIFI_RX_TX_HSIOM_REG   (*(reg32 *) WIFI_rx_tx__0__HSIOM)
    #define WIFI_RX_TX_HSIOM_PTR   ( (reg32 *) WIFI_rx_tx__0__HSIOM)
    
    #define WIFI_RX_TX_HSIOM_MASK      (WIFI_rx_tx__0__HSIOM_MASK)
    #define WIFI_RX_TX_HSIOM_POS       (WIFI_rx_tx__0__HSIOM_SHIFT)
    #define WIFI_RX_TX_HSIOM_SEL_GPIO  (WIFI_rx_tx__0__HSIOM_GPIO)
    #define WIFI_RX_TX_HSIOM_SEL_UART  (WIFI_rx_tx__0__HSIOM_UART)
#endif /* (WIFI_UART_RX_TX_PIN) */

#if (WIFI_UART_RTS_PIN)
    #define WIFI_RTS_HSIOM_REG      (*(reg32 *) WIFI_rts__0__HSIOM)
    #define WIFI_RTS_HSIOM_PTR      ( (reg32 *) WIFI_rts__0__HSIOM)
    
    #define WIFI_RTS_HSIOM_MASK     (WIFI_rts__0__HSIOM_MASK)
    #define WIFI_RTS_HSIOM_POS      (WIFI_rts__0__HSIOM_SHIFT)    
    #define WIFI_RTS_HSIOM_SEL_GPIO (WIFI_rts__0__HSIOM_GPIO)
    #define WIFI_RTS_HSIOM_SEL_UART (WIFI_rts__0__HSIOM_UART)    
#endif /* (WIFI_UART_RTS_PIN) */


/***************************************
*        Registers Constants
***************************************/

/* HSIOM switch values. */ 
#define WIFI_HSIOM_DEF_SEL      (0x00u)
#define WIFI_HSIOM_GPIO_SEL     (0x00u)
/* The HSIOM values provided below are valid only for WIFI_CY_SCBIP_V0 
* and WIFI_CY_SCBIP_V1. It is not recommended to use them for 
* WIFI_CY_SCBIP_V2. Use pin name specific HSIOM constants provided 
* above instead for any SCB IP block version.
*/
#define WIFI_HSIOM_UART_SEL     (0x09u)
#define WIFI_HSIOM_I2C_SEL      (0x0Eu)
#define WIFI_HSIOM_SPI_SEL      (0x0Fu)

/* Pins settings index. */
#define WIFI_RX_WAKE_SCL_MOSI_PIN_INDEX   (0u)
#define WIFI_RX_SCL_MOSI_PIN_INDEX       (0u)
#define WIFI_TX_SDA_MISO_PIN_INDEX       (1u)
#define WIFI_CTS_SCLK_PIN_INDEX       (2u)
#define WIFI_RTS_SS0_PIN_INDEX       (3u)
#define WIFI_SS1_PIN_INDEX                  (4u)
#define WIFI_SS2_PIN_INDEX                  (5u)
#define WIFI_SS3_PIN_INDEX                  (6u)

/* Pins settings mask. */
#define WIFI_RX_WAKE_SCL_MOSI_PIN_MASK ((uint32) 0x01u << WIFI_RX_WAKE_SCL_MOSI_PIN_INDEX)
#define WIFI_RX_SCL_MOSI_PIN_MASK     ((uint32) 0x01u << WIFI_RX_SCL_MOSI_PIN_INDEX)
#define WIFI_TX_SDA_MISO_PIN_MASK     ((uint32) 0x01u << WIFI_TX_SDA_MISO_PIN_INDEX)
#define WIFI_CTS_SCLK_PIN_MASK     ((uint32) 0x01u << WIFI_CTS_SCLK_PIN_INDEX)
#define WIFI_RTS_SS0_PIN_MASK     ((uint32) 0x01u << WIFI_RTS_SS0_PIN_INDEX)
#define WIFI_SS1_PIN_MASK                ((uint32) 0x01u << WIFI_SS1_PIN_INDEX)
#define WIFI_SS2_PIN_MASK                ((uint32) 0x01u << WIFI_SS2_PIN_INDEX)
#define WIFI_SS3_PIN_MASK                ((uint32) 0x01u << WIFI_SS3_PIN_INDEX)

/* Pin interrupt constants. */
#define WIFI_INTCFG_TYPE_MASK           (0x03u)
#define WIFI_INTCFG_TYPE_FALLING_EDGE   (0x02u)

/* Pin Drive Mode constants. */
#define WIFI_PIN_DM_ALG_HIZ  (0u)
#define WIFI_PIN_DM_DIG_HIZ  (1u)
#define WIFI_PIN_DM_OD_LO    (4u)
#define WIFI_PIN_DM_STRONG   (6u)


/***************************************
*          Macro Definitions
***************************************/

/* Return drive mode of the pin */
#define WIFI_DM_MASK    (0x7u)
#define WIFI_DM_SIZE    (3u)
#define WIFI_GET_P4_PIN_DM(reg, pos) \
    ( ((reg) & (uint32) ((uint32) WIFI_DM_MASK << (WIFI_DM_SIZE * (pos)))) >> \
                                                              (WIFI_DM_SIZE * (pos)) )

#if (WIFI_TX_SDA_MISO_PIN)
    #define WIFI_CHECK_TX_SDA_MISO_PIN_USED \
                (WIFI_PIN_DM_ALG_HIZ != \
                    WIFI_GET_P4_PIN_DM(WIFI_uart_tx_i2c_sda_spi_miso_PC, \
                                                   WIFI_uart_tx_i2c_sda_spi_miso_SHIFT))
#endif /* (WIFI_TX_SDA_MISO_PIN) */

#if (WIFI_RTS_SS0_PIN)
    #define WIFI_CHECK_RTS_SS0_PIN_USED \
                (WIFI_PIN_DM_ALG_HIZ != \
                    WIFI_GET_P4_PIN_DM(WIFI_uart_rts_spi_ss0_PC, \
                                                   WIFI_uart_rts_spi_ss0_SHIFT))
#endif /* (WIFI_RTS_SS0_PIN) */

/* Set bits-mask in register */
#define WIFI_SET_REGISTER_BITS(reg, mask, pos, mode) \
                    do                                           \
                    {                                            \
                        (reg) = (((reg) & ((uint32) ~(uint32) (mask))) | ((uint32) ((uint32) (mode) << (pos)))); \
                    }while(0)

/* Set bit in the register */
#define WIFI_SET_REGISTER_BIT(reg, mask, val) \
                    ((val) ? ((reg) |= (mask)) : ((reg) &= ((uint32) ~((uint32) (mask)))))

#define WIFI_SET_HSIOM_SEL(reg, mask, pos, sel) WIFI_SET_REGISTER_BITS(reg, mask, pos, sel)
#define WIFI_SET_INCFG_TYPE(reg, mask, pos, intType) \
                                                        WIFI_SET_REGISTER_BITS(reg, mask, pos, intType)
#define WIFI_SET_INP_DIS(reg, mask, val) WIFI_SET_REGISTER_BIT(reg, mask, val)

/* WIFI_SET_I2C_SCL_DR(val) - Sets I2C SCL DR register.
*  WIFI_SET_I2C_SCL_HSIOM_SEL(sel) - Sets I2C SCL HSIOM settings.
*/
/* SCB I2C: scl signal */
#if (WIFI_CY_SCBIP_V0)
#if (WIFI_I2C_PINS)
    #define WIFI_SET_I2C_SCL_DR(val) WIFI_scl_Write(val)

    #define WIFI_SET_I2C_SCL_HSIOM_SEL(sel) \
                          WIFI_SET_HSIOM_SEL(WIFI_SCL_HSIOM_REG,  \
                                                         WIFI_SCL_HSIOM_MASK, \
                                                         WIFI_SCL_HSIOM_POS,  \
                                                         (sel))
    #define WIFI_WAIT_SCL_SET_HIGH  (0u == WIFI_scl_Read())

/* Unconfigured SCB: scl signal */
#elif (WIFI_RX_WAKE_SCL_MOSI_PIN)
    #define WIFI_SET_I2C_SCL_DR(val) \
                            WIFI_uart_rx_wake_i2c_scl_spi_mosi_Write(val)

    #define WIFI_SET_I2C_SCL_HSIOM_SEL(sel) \
                    WIFI_SET_HSIOM_SEL(WIFI_RX_WAKE_SCL_MOSI_HSIOM_REG,  \
                                                   WIFI_RX_WAKE_SCL_MOSI_HSIOM_MASK, \
                                                   WIFI_RX_WAKE_SCL_MOSI_HSIOM_POS,  \
                                                   (sel))

    #define WIFI_WAIT_SCL_SET_HIGH  (0u == WIFI_uart_rx_wake_i2c_scl_spi_mosi_Read())

#elif (WIFI_RX_SCL_MOSI_PIN)
    #define WIFI_SET_I2C_SCL_DR(val) \
                            WIFI_uart_rx_i2c_scl_spi_mosi_Write(val)


    #define WIFI_SET_I2C_SCL_HSIOM_SEL(sel) \
                            WIFI_SET_HSIOM_SEL(WIFI_RX_SCL_MOSI_HSIOM_REG,  \
                                                           WIFI_RX_SCL_MOSI_HSIOM_MASK, \
                                                           WIFI_RX_SCL_MOSI_HSIOM_POS,  \
                                                           (sel))

    #define WIFI_WAIT_SCL_SET_HIGH  (0u == WIFI_uart_rx_i2c_scl_spi_mosi_Read())

#else
    #define WIFI_SET_I2C_SCL_DR(val)        do{ /* Does nothing */ }while(0)
    #define WIFI_SET_I2C_SCL_HSIOM_SEL(sel) do{ /* Does nothing */ }while(0)

    #define WIFI_WAIT_SCL_SET_HIGH  (0u)
#endif /* (WIFI_I2C_PINS) */

/* SCB I2C: sda signal */
#if (WIFI_I2C_PINS)
    #define WIFI_WAIT_SDA_SET_HIGH  (0u == WIFI_sda_Read())
/* Unconfigured SCB: sda signal */
#elif (WIFI_TX_SDA_MISO_PIN)
    #define WIFI_WAIT_SDA_SET_HIGH  (0u == WIFI_uart_tx_i2c_sda_spi_miso_Read())
#else
    #define WIFI_WAIT_SDA_SET_HIGH  (0u)
#endif /* (WIFI_MOSI_SCL_RX_PIN) */
#endif /* (WIFI_CY_SCBIP_V0) */

/* Clear UART wakeup source */
#if (WIFI_RX_SCL_MOSI_PIN)
    #define WIFI_CLEAR_UART_RX_WAKE_INTR        do{ /* Does nothing */ }while(0)
    
#elif (WIFI_RX_WAKE_SCL_MOSI_PIN)
    #define WIFI_CLEAR_UART_RX_WAKE_INTR \
            do{                                      \
                (void) WIFI_uart_rx_wake_i2c_scl_spi_mosi_ClearInterrupt(); \
            }while(0)

#elif(WIFI_UART_RX_WAKE_PIN)
    #define WIFI_CLEAR_UART_RX_WAKE_INTR \
            do{                                      \
                (void) WIFI_rx_wake_ClearInterrupt(); \
            }while(0)
#else
#endif /* (WIFI_RX_SCL_MOSI_PIN) */


/***************************************
* The following code is DEPRECATED and
* must not be used.
***************************************/

/* Unconfigured pins */
#define WIFI_REMOVE_MOSI_SCL_RX_WAKE_PIN    WIFI_REMOVE_RX_WAKE_SCL_MOSI_PIN
#define WIFI_REMOVE_MOSI_SCL_RX_PIN         WIFI_REMOVE_RX_SCL_MOSI_PIN
#define WIFI_REMOVE_MISO_SDA_TX_PIN         WIFI_REMOVE_TX_SDA_MISO_PIN
#ifndef WIFI_REMOVE_SCLK_PIN
#define WIFI_REMOVE_SCLK_PIN                WIFI_REMOVE_CTS_SCLK_PIN
#endif /* WIFI_REMOVE_SCLK_PIN */
#ifndef WIFI_REMOVE_SS0_PIN
#define WIFI_REMOVE_SS0_PIN                 WIFI_REMOVE_RTS_SS0_PIN
#endif /* WIFI_REMOVE_SS0_PIN */

/* Unconfigured pins */
#define WIFI_MOSI_SCL_RX_WAKE_PIN   WIFI_RX_WAKE_SCL_MOSI_PIN
#define WIFI_MOSI_SCL_RX_PIN        WIFI_RX_SCL_MOSI_PIN
#define WIFI_MISO_SDA_TX_PIN        WIFI_TX_SDA_MISO_PIN
#ifndef WIFI_SCLK_PIN
#define WIFI_SCLK_PIN               WIFI_CTS_SCLK_PIN
#endif /* WIFI_SCLK_PIN */
#ifndef WIFI_SS0_PIN
#define WIFI_SS0_PIN                WIFI_RTS_SS0_PIN
#endif /* WIFI_SS0_PIN */

#if (WIFI_MOSI_SCL_RX_WAKE_PIN)
    #define WIFI_MOSI_SCL_RX_WAKE_HSIOM_REG     WIFI_RX_WAKE_SCL_MOSI_HSIOM_REG
    #define WIFI_MOSI_SCL_RX_WAKE_HSIOM_PTR     WIFI_RX_WAKE_SCL_MOSI_HSIOM_REG
    #define WIFI_MOSI_SCL_RX_WAKE_HSIOM_MASK    WIFI_RX_WAKE_SCL_MOSI_HSIOM_REG
    #define WIFI_MOSI_SCL_RX_WAKE_HSIOM_POS     WIFI_RX_WAKE_SCL_MOSI_HSIOM_REG

    #define WIFI_MOSI_SCL_RX_WAKE_INTCFG_REG    WIFI_RX_WAKE_SCL_MOSI_HSIOM_REG
    #define WIFI_MOSI_SCL_RX_WAKE_INTCFG_PTR    WIFI_RX_WAKE_SCL_MOSI_HSIOM_REG

    #define WIFI_MOSI_SCL_RX_WAKE_INTCFG_TYPE_POS   WIFI_RX_WAKE_SCL_MOSI_HSIOM_REG
    #define WIFI_MOSI_SCL_RX_WAKE_INTCFG_TYPE_MASK  WIFI_RX_WAKE_SCL_MOSI_HSIOM_REG
#endif /* (WIFI_RX_WAKE_SCL_MOSI_PIN) */

#if (WIFI_MOSI_SCL_RX_PIN)
    #define WIFI_MOSI_SCL_RX_HSIOM_REG      WIFI_RX_SCL_MOSI_HSIOM_REG
    #define WIFI_MOSI_SCL_RX_HSIOM_PTR      WIFI_RX_SCL_MOSI_HSIOM_PTR
    #define WIFI_MOSI_SCL_RX_HSIOM_MASK     WIFI_RX_SCL_MOSI_HSIOM_MASK
    #define WIFI_MOSI_SCL_RX_HSIOM_POS      WIFI_RX_SCL_MOSI_HSIOM_POS
#endif /* (WIFI_MOSI_SCL_RX_PIN) */

#if (WIFI_MISO_SDA_TX_PIN)
    #define WIFI_MISO_SDA_TX_HSIOM_REG      WIFI_TX_SDA_MISO_HSIOM_REG
    #define WIFI_MISO_SDA_TX_HSIOM_PTR      WIFI_TX_SDA_MISO_HSIOM_REG
    #define WIFI_MISO_SDA_TX_HSIOM_MASK     WIFI_TX_SDA_MISO_HSIOM_REG
    #define WIFI_MISO_SDA_TX_HSIOM_POS      WIFI_TX_SDA_MISO_HSIOM_REG
#endif /* (WIFI_MISO_SDA_TX_PIN_PIN) */

#if (WIFI_SCLK_PIN)
    #ifndef WIFI_SCLK_HSIOM_REG
    #define WIFI_SCLK_HSIOM_REG     WIFI_CTS_SCLK_HSIOM_REG
    #define WIFI_SCLK_HSIOM_PTR     WIFI_CTS_SCLK_HSIOM_PTR
    #define WIFI_SCLK_HSIOM_MASK    WIFI_CTS_SCLK_HSIOM_MASK
    #define WIFI_SCLK_HSIOM_POS     WIFI_CTS_SCLK_HSIOM_POS
    #endif /* WIFI_SCLK_HSIOM_REG */
#endif /* (WIFI_SCLK_PIN) */

#if (WIFI_SS0_PIN)
    #ifndef WIFI_SS0_HSIOM_REG
    #define WIFI_SS0_HSIOM_REG      WIFI_RTS_SS0_HSIOM_REG
    #define WIFI_SS0_HSIOM_PTR      WIFI_RTS_SS0_HSIOM_PTR
    #define WIFI_SS0_HSIOM_MASK     WIFI_RTS_SS0_HSIOM_MASK
    #define WIFI_SS0_HSIOM_POS      WIFI_RTS_SS0_HSIOM_POS
    #endif /* WIFI_SS0_HSIOM_REG */
#endif /* (WIFI_SS0_PIN) */

#define WIFI_MOSI_SCL_RX_WAKE_PIN_INDEX WIFI_RX_WAKE_SCL_MOSI_PIN_INDEX
#define WIFI_MOSI_SCL_RX_PIN_INDEX      WIFI_RX_SCL_MOSI_PIN_INDEX
#define WIFI_MISO_SDA_TX_PIN_INDEX      WIFI_TX_SDA_MISO_PIN_INDEX
#ifndef WIFI_SCLK_PIN_INDEX
#define WIFI_SCLK_PIN_INDEX             WIFI_CTS_SCLK_PIN_INDEX
#endif /* WIFI_SCLK_PIN_INDEX */
#ifndef WIFI_SS0_PIN_INDEX
#define WIFI_SS0_PIN_INDEX              WIFI_RTS_SS0_PIN_INDEX
#endif /* WIFI_SS0_PIN_INDEX */

#define WIFI_MOSI_SCL_RX_WAKE_PIN_MASK WIFI_RX_WAKE_SCL_MOSI_PIN_MASK
#define WIFI_MOSI_SCL_RX_PIN_MASK      WIFI_RX_SCL_MOSI_PIN_MASK
#define WIFI_MISO_SDA_TX_PIN_MASK      WIFI_TX_SDA_MISO_PIN_MASK
#ifndef WIFI_SCLK_PIN_MASK
#define WIFI_SCLK_PIN_MASK             WIFI_CTS_SCLK_PIN_MASK
#endif /* WIFI_SCLK_PIN_MASK */
#ifndef WIFI_SS0_PIN_MASK
#define WIFI_SS0_PIN_MASK              WIFI_RTS_SS0_PIN_MASK
#endif /* WIFI_SS0_PIN_MASK */

#endif /* (CY_SCB_PINS_WIFI_H) */


/* [] END OF FILE */
