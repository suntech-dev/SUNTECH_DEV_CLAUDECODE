/***************************************************************************//**
* \file BARCODE_PINS.h
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

#if !defined(CY_SCB_PINS_BARCODE_H)
#define CY_SCB_PINS_BARCODE_H

#include "cydevice_trm.h"
#include "cyfitter.h"
#include "cytypes.h"


/***************************************
*   Conditional Compilation Parameters
****************************************/

/* Unconfigured pins */
#define BARCODE_REMOVE_RX_WAKE_SCL_MOSI_PIN  (1u)
#define BARCODE_REMOVE_RX_SCL_MOSI_PIN      (1u)
#define BARCODE_REMOVE_TX_SDA_MISO_PIN      (1u)
#define BARCODE_REMOVE_CTS_SCLK_PIN      (1u)
#define BARCODE_REMOVE_RTS_SS0_PIN      (1u)
#define BARCODE_REMOVE_SS1_PIN                 (1u)
#define BARCODE_REMOVE_SS2_PIN                 (1u)
#define BARCODE_REMOVE_SS3_PIN                 (1u)

/* Mode defined pins */
#define BARCODE_REMOVE_I2C_PINS                (1u)
#define BARCODE_REMOVE_SPI_MASTER_PINS         (1u)
#define BARCODE_REMOVE_SPI_MASTER_SCLK_PIN     (1u)
#define BARCODE_REMOVE_SPI_MASTER_MOSI_PIN     (1u)
#define BARCODE_REMOVE_SPI_MASTER_MISO_PIN     (1u)
#define BARCODE_REMOVE_SPI_MASTER_SS0_PIN      (1u)
#define BARCODE_REMOVE_SPI_MASTER_SS1_PIN      (1u)
#define BARCODE_REMOVE_SPI_MASTER_SS2_PIN      (1u)
#define BARCODE_REMOVE_SPI_MASTER_SS3_PIN      (1u)
#define BARCODE_REMOVE_SPI_SLAVE_PINS          (1u)
#define BARCODE_REMOVE_SPI_SLAVE_MOSI_PIN      (1u)
#define BARCODE_REMOVE_SPI_SLAVE_MISO_PIN      (1u)
#define BARCODE_REMOVE_UART_TX_PIN             (0u)
#define BARCODE_REMOVE_UART_RX_TX_PIN          (1u)
#define BARCODE_REMOVE_UART_RX_PIN             (0u)
#define BARCODE_REMOVE_UART_RX_WAKE_PIN        (1u)
#define BARCODE_REMOVE_UART_RTS_PIN            (1u)
#define BARCODE_REMOVE_UART_CTS_PIN            (1u)

/* Unconfigured pins */
#define BARCODE_RX_WAKE_SCL_MOSI_PIN (0u == BARCODE_REMOVE_RX_WAKE_SCL_MOSI_PIN)
#define BARCODE_RX_SCL_MOSI_PIN     (0u == BARCODE_REMOVE_RX_SCL_MOSI_PIN)
#define BARCODE_TX_SDA_MISO_PIN     (0u == BARCODE_REMOVE_TX_SDA_MISO_PIN)
#define BARCODE_CTS_SCLK_PIN     (0u == BARCODE_REMOVE_CTS_SCLK_PIN)
#define BARCODE_RTS_SS0_PIN     (0u == BARCODE_REMOVE_RTS_SS0_PIN)
#define BARCODE_SS1_PIN                (0u == BARCODE_REMOVE_SS1_PIN)
#define BARCODE_SS2_PIN                (0u == BARCODE_REMOVE_SS2_PIN)
#define BARCODE_SS3_PIN                (0u == BARCODE_REMOVE_SS3_PIN)

/* Mode defined pins */
#define BARCODE_I2C_PINS               (0u == BARCODE_REMOVE_I2C_PINS)
#define BARCODE_SPI_MASTER_PINS        (0u == BARCODE_REMOVE_SPI_MASTER_PINS)
#define BARCODE_SPI_MASTER_SCLK_PIN    (0u == BARCODE_REMOVE_SPI_MASTER_SCLK_PIN)
#define BARCODE_SPI_MASTER_MOSI_PIN    (0u == BARCODE_REMOVE_SPI_MASTER_MOSI_PIN)
#define BARCODE_SPI_MASTER_MISO_PIN    (0u == BARCODE_REMOVE_SPI_MASTER_MISO_PIN)
#define BARCODE_SPI_MASTER_SS0_PIN     (0u == BARCODE_REMOVE_SPI_MASTER_SS0_PIN)
#define BARCODE_SPI_MASTER_SS1_PIN     (0u == BARCODE_REMOVE_SPI_MASTER_SS1_PIN)
#define BARCODE_SPI_MASTER_SS2_PIN     (0u == BARCODE_REMOVE_SPI_MASTER_SS2_PIN)
#define BARCODE_SPI_MASTER_SS3_PIN     (0u == BARCODE_REMOVE_SPI_MASTER_SS3_PIN)
#define BARCODE_SPI_SLAVE_PINS         (0u == BARCODE_REMOVE_SPI_SLAVE_PINS)
#define BARCODE_SPI_SLAVE_MOSI_PIN     (0u == BARCODE_REMOVE_SPI_SLAVE_MOSI_PIN)
#define BARCODE_SPI_SLAVE_MISO_PIN     (0u == BARCODE_REMOVE_SPI_SLAVE_MISO_PIN)
#define BARCODE_UART_TX_PIN            (0u == BARCODE_REMOVE_UART_TX_PIN)
#define BARCODE_UART_RX_TX_PIN         (0u == BARCODE_REMOVE_UART_RX_TX_PIN)
#define BARCODE_UART_RX_PIN            (0u == BARCODE_REMOVE_UART_RX_PIN)
#define BARCODE_UART_RX_WAKE_PIN       (0u == BARCODE_REMOVE_UART_RX_WAKE_PIN)
#define BARCODE_UART_RTS_PIN           (0u == BARCODE_REMOVE_UART_RTS_PIN)
#define BARCODE_UART_CTS_PIN           (0u == BARCODE_REMOVE_UART_CTS_PIN)


/***************************************
*             Includes
****************************************/

#if (BARCODE_RX_WAKE_SCL_MOSI_PIN)
    #include "BARCODE_uart_rx_wake_i2c_scl_spi_mosi.h"
#endif /* (BARCODE_RX_SCL_MOSI) */

#if (BARCODE_RX_SCL_MOSI_PIN)
    #include "BARCODE_uart_rx_i2c_scl_spi_mosi.h"
#endif /* (BARCODE_RX_SCL_MOSI) */

#if (BARCODE_TX_SDA_MISO_PIN)
    #include "BARCODE_uart_tx_i2c_sda_spi_miso.h"
#endif /* (BARCODE_TX_SDA_MISO) */

#if (BARCODE_CTS_SCLK_PIN)
    #include "BARCODE_uart_cts_spi_sclk.h"
#endif /* (BARCODE_CTS_SCLK) */

#if (BARCODE_RTS_SS0_PIN)
    #include "BARCODE_uart_rts_spi_ss0.h"
#endif /* (BARCODE_RTS_SS0_PIN) */

#if (BARCODE_SS1_PIN)
    #include "BARCODE_spi_ss1.h"
#endif /* (BARCODE_SS1_PIN) */

#if (BARCODE_SS2_PIN)
    #include "BARCODE_spi_ss2.h"
#endif /* (BARCODE_SS2_PIN) */

#if (BARCODE_SS3_PIN)
    #include "BARCODE_spi_ss3.h"
#endif /* (BARCODE_SS3_PIN) */

#if (BARCODE_I2C_PINS)
    #include "BARCODE_scl.h"
    #include "BARCODE_sda.h"
#endif /* (BARCODE_I2C_PINS) */

#if (BARCODE_SPI_MASTER_PINS)
#if (BARCODE_SPI_MASTER_SCLK_PIN)
    #include "BARCODE_sclk_m.h"
#endif /* (BARCODE_SPI_MASTER_SCLK_PIN) */

#if (BARCODE_SPI_MASTER_MOSI_PIN)
    #include "BARCODE_mosi_m.h"
#endif /* (BARCODE_SPI_MASTER_MOSI_PIN) */

#if (BARCODE_SPI_MASTER_MISO_PIN)
    #include "BARCODE_miso_m.h"
#endif /*(BARCODE_SPI_MASTER_MISO_PIN) */
#endif /* (BARCODE_SPI_MASTER_PINS) */

#if (BARCODE_SPI_SLAVE_PINS)
    #include "BARCODE_sclk_s.h"
    #include "BARCODE_ss_s.h"

#if (BARCODE_SPI_SLAVE_MOSI_PIN)
    #include "BARCODE_mosi_s.h"
#endif /* (BARCODE_SPI_SLAVE_MOSI_PIN) */

#if (BARCODE_SPI_SLAVE_MISO_PIN)
    #include "BARCODE_miso_s.h"
#endif /*(BARCODE_SPI_SLAVE_MISO_PIN) */
#endif /* (BARCODE_SPI_SLAVE_PINS) */

#if (BARCODE_SPI_MASTER_SS0_PIN)
    #include "BARCODE_ss0_m.h"
#endif /* (BARCODE_SPI_MASTER_SS0_PIN) */

#if (BARCODE_SPI_MASTER_SS1_PIN)
    #include "BARCODE_ss1_m.h"
#endif /* (BARCODE_SPI_MASTER_SS1_PIN) */

#if (BARCODE_SPI_MASTER_SS2_PIN)
    #include "BARCODE_ss2_m.h"
#endif /* (BARCODE_SPI_MASTER_SS2_PIN) */

#if (BARCODE_SPI_MASTER_SS3_PIN)
    #include "BARCODE_ss3_m.h"
#endif /* (BARCODE_SPI_MASTER_SS3_PIN) */

#if (BARCODE_UART_TX_PIN)
    #include "BARCODE_tx.h"
#endif /* (BARCODE_UART_TX_PIN) */

#if (BARCODE_UART_RX_TX_PIN)
    #include "BARCODE_rx_tx.h"
#endif /* (BARCODE_UART_RX_TX_PIN) */

#if (BARCODE_UART_RX_PIN)
    #include "BARCODE_rx.h"
#endif /* (BARCODE_UART_RX_PIN) */

#if (BARCODE_UART_RX_WAKE_PIN)
    #include "BARCODE_rx_wake.h"
#endif /* (BARCODE_UART_RX_WAKE_PIN) */

#if (BARCODE_UART_RTS_PIN)
    #include "BARCODE_rts.h"
#endif /* (BARCODE_UART_RTS_PIN) */

#if (BARCODE_UART_CTS_PIN)
    #include "BARCODE_cts.h"
#endif /* (BARCODE_UART_CTS_PIN) */


/***************************************
*              Registers
***************************************/

#if (BARCODE_RX_SCL_MOSI_PIN)
    #define BARCODE_RX_SCL_MOSI_HSIOM_REG   (*(reg32 *) BARCODE_uart_rx_i2c_scl_spi_mosi__0__HSIOM)
    #define BARCODE_RX_SCL_MOSI_HSIOM_PTR   ( (reg32 *) BARCODE_uart_rx_i2c_scl_spi_mosi__0__HSIOM)
    
    #define BARCODE_RX_SCL_MOSI_HSIOM_MASK      (BARCODE_uart_rx_i2c_scl_spi_mosi__0__HSIOM_MASK)
    #define BARCODE_RX_SCL_MOSI_HSIOM_POS       (BARCODE_uart_rx_i2c_scl_spi_mosi__0__HSIOM_SHIFT)
    #define BARCODE_RX_SCL_MOSI_HSIOM_SEL_GPIO  (BARCODE_uart_rx_i2c_scl_spi_mosi__0__HSIOM_GPIO)
    #define BARCODE_RX_SCL_MOSI_HSIOM_SEL_I2C   (BARCODE_uart_rx_i2c_scl_spi_mosi__0__HSIOM_I2C)
    #define BARCODE_RX_SCL_MOSI_HSIOM_SEL_SPI   (BARCODE_uart_rx_i2c_scl_spi_mosi__0__HSIOM_SPI)
    #define BARCODE_RX_SCL_MOSI_HSIOM_SEL_UART  (BARCODE_uart_rx_i2c_scl_spi_mosi__0__HSIOM_UART)
    
#elif (BARCODE_RX_WAKE_SCL_MOSI_PIN)
    #define BARCODE_RX_WAKE_SCL_MOSI_HSIOM_REG   (*(reg32 *) BARCODE_uart_rx_wake_i2c_scl_spi_mosi__0__HSIOM)
    #define BARCODE_RX_WAKE_SCL_MOSI_HSIOM_PTR   ( (reg32 *) BARCODE_uart_rx_wake_i2c_scl_spi_mosi__0__HSIOM)
    
    #define BARCODE_RX_WAKE_SCL_MOSI_HSIOM_MASK      (BARCODE_uart_rx_wake_i2c_scl_spi_mosi__0__HSIOM_MASK)
    #define BARCODE_RX_WAKE_SCL_MOSI_HSIOM_POS       (BARCODE_uart_rx_wake_i2c_scl_spi_mosi__0__HSIOM_SHIFT)
    #define BARCODE_RX_WAKE_SCL_MOSI_HSIOM_SEL_GPIO  (BARCODE_uart_rx_wake_i2c_scl_spi_mosi__0__HSIOM_GPIO)
    #define BARCODE_RX_WAKE_SCL_MOSI_HSIOM_SEL_I2C   (BARCODE_uart_rx_wake_i2c_scl_spi_mosi__0__HSIOM_I2C)
    #define BARCODE_RX_WAKE_SCL_MOSI_HSIOM_SEL_SPI   (BARCODE_uart_rx_wake_i2c_scl_spi_mosi__0__HSIOM_SPI)
    #define BARCODE_RX_WAKE_SCL_MOSI_HSIOM_SEL_UART  (BARCODE_uart_rx_wake_i2c_scl_spi_mosi__0__HSIOM_UART)    
   
    #define BARCODE_RX_WAKE_SCL_MOSI_INTCFG_REG (*(reg32 *) BARCODE_uart_rx_wake_i2c_scl_spi_mosi__0__INTCFG)
    #define BARCODE_RX_WAKE_SCL_MOSI_INTCFG_PTR ( (reg32 *) BARCODE_uart_rx_wake_i2c_scl_spi_mosi__0__INTCFG)
    #define BARCODE_RX_WAKE_SCL_MOSI_INTCFG_TYPE_POS  (BARCODE_uart_rx_wake_i2c_scl_spi_mosi__SHIFT)
    #define BARCODE_RX_WAKE_SCL_MOSI_INTCFG_TYPE_MASK ((uint32) BARCODE_INTCFG_TYPE_MASK << \
                                                                           BARCODE_RX_WAKE_SCL_MOSI_INTCFG_TYPE_POS)
#else
    /* None of pins BARCODE_RX_SCL_MOSI_PIN or BARCODE_RX_WAKE_SCL_MOSI_PIN present.*/
#endif /* (BARCODE_RX_SCL_MOSI_PIN) */

#if (BARCODE_TX_SDA_MISO_PIN)
    #define BARCODE_TX_SDA_MISO_HSIOM_REG   (*(reg32 *) BARCODE_uart_tx_i2c_sda_spi_miso__0__HSIOM)
    #define BARCODE_TX_SDA_MISO_HSIOM_PTR   ( (reg32 *) BARCODE_uart_tx_i2c_sda_spi_miso__0__HSIOM)
    
    #define BARCODE_TX_SDA_MISO_HSIOM_MASK      (BARCODE_uart_tx_i2c_sda_spi_miso__0__HSIOM_MASK)
    #define BARCODE_TX_SDA_MISO_HSIOM_POS       (BARCODE_uart_tx_i2c_sda_spi_miso__0__HSIOM_SHIFT)
    #define BARCODE_TX_SDA_MISO_HSIOM_SEL_GPIO  (BARCODE_uart_tx_i2c_sda_spi_miso__0__HSIOM_GPIO)
    #define BARCODE_TX_SDA_MISO_HSIOM_SEL_I2C   (BARCODE_uart_tx_i2c_sda_spi_miso__0__HSIOM_I2C)
    #define BARCODE_TX_SDA_MISO_HSIOM_SEL_SPI   (BARCODE_uart_tx_i2c_sda_spi_miso__0__HSIOM_SPI)
    #define BARCODE_TX_SDA_MISO_HSIOM_SEL_UART  (BARCODE_uart_tx_i2c_sda_spi_miso__0__HSIOM_UART)
#endif /* (BARCODE_TX_SDA_MISO_PIN) */

#if (BARCODE_CTS_SCLK_PIN)
    #define BARCODE_CTS_SCLK_HSIOM_REG   (*(reg32 *) BARCODE_uart_cts_spi_sclk__0__HSIOM)
    #define BARCODE_CTS_SCLK_HSIOM_PTR   ( (reg32 *) BARCODE_uart_cts_spi_sclk__0__HSIOM)
    
    #define BARCODE_CTS_SCLK_HSIOM_MASK      (BARCODE_uart_cts_spi_sclk__0__HSIOM_MASK)
    #define BARCODE_CTS_SCLK_HSIOM_POS       (BARCODE_uart_cts_spi_sclk__0__HSIOM_SHIFT)
    #define BARCODE_CTS_SCLK_HSIOM_SEL_GPIO  (BARCODE_uart_cts_spi_sclk__0__HSIOM_GPIO)
    #define BARCODE_CTS_SCLK_HSIOM_SEL_I2C   (BARCODE_uart_cts_spi_sclk__0__HSIOM_I2C)
    #define BARCODE_CTS_SCLK_HSIOM_SEL_SPI   (BARCODE_uart_cts_spi_sclk__0__HSIOM_SPI)
    #define BARCODE_CTS_SCLK_HSIOM_SEL_UART  (BARCODE_uart_cts_spi_sclk__0__HSIOM_UART)
#endif /* (BARCODE_CTS_SCLK_PIN) */

#if (BARCODE_RTS_SS0_PIN)
    #define BARCODE_RTS_SS0_HSIOM_REG   (*(reg32 *) BARCODE_uart_rts_spi_ss0__0__HSIOM)
    #define BARCODE_RTS_SS0_HSIOM_PTR   ( (reg32 *) BARCODE_uart_rts_spi_ss0__0__HSIOM)
    
    #define BARCODE_RTS_SS0_HSIOM_MASK      (BARCODE_uart_rts_spi_ss0__0__HSIOM_MASK)
    #define BARCODE_RTS_SS0_HSIOM_POS       (BARCODE_uart_rts_spi_ss0__0__HSIOM_SHIFT)
    #define BARCODE_RTS_SS0_HSIOM_SEL_GPIO  (BARCODE_uart_rts_spi_ss0__0__HSIOM_GPIO)
    #define BARCODE_RTS_SS0_HSIOM_SEL_I2C   (BARCODE_uart_rts_spi_ss0__0__HSIOM_I2C)
    #define BARCODE_RTS_SS0_HSIOM_SEL_SPI   (BARCODE_uart_rts_spi_ss0__0__HSIOM_SPI)
#if !(BARCODE_CY_SCBIP_V0 || BARCODE_CY_SCBIP_V1)
    #define BARCODE_RTS_SS0_HSIOM_SEL_UART  (BARCODE_uart_rts_spi_ss0__0__HSIOM_UART)
#endif /* !(BARCODE_CY_SCBIP_V0 || BARCODE_CY_SCBIP_V1) */
#endif /* (BARCODE_RTS_SS0_PIN) */

#if (BARCODE_SS1_PIN)
    #define BARCODE_SS1_HSIOM_REG  (*(reg32 *) BARCODE_spi_ss1__0__HSIOM)
    #define BARCODE_SS1_HSIOM_PTR  ( (reg32 *) BARCODE_spi_ss1__0__HSIOM)
    
    #define BARCODE_SS1_HSIOM_MASK     (BARCODE_spi_ss1__0__HSIOM_MASK)
    #define BARCODE_SS1_HSIOM_POS      (BARCODE_spi_ss1__0__HSIOM_SHIFT)
    #define BARCODE_SS1_HSIOM_SEL_GPIO (BARCODE_spi_ss1__0__HSIOM_GPIO)
    #define BARCODE_SS1_HSIOM_SEL_I2C  (BARCODE_spi_ss1__0__HSIOM_I2C)
    #define BARCODE_SS1_HSIOM_SEL_SPI  (BARCODE_spi_ss1__0__HSIOM_SPI)
#endif /* (BARCODE_SS1_PIN) */

#if (BARCODE_SS2_PIN)
    #define BARCODE_SS2_HSIOM_REG     (*(reg32 *) BARCODE_spi_ss2__0__HSIOM)
    #define BARCODE_SS2_HSIOM_PTR     ( (reg32 *) BARCODE_spi_ss2__0__HSIOM)
    
    #define BARCODE_SS2_HSIOM_MASK     (BARCODE_spi_ss2__0__HSIOM_MASK)
    #define BARCODE_SS2_HSIOM_POS      (BARCODE_spi_ss2__0__HSIOM_SHIFT)
    #define BARCODE_SS2_HSIOM_SEL_GPIO (BARCODE_spi_ss2__0__HSIOM_GPIO)
    #define BARCODE_SS2_HSIOM_SEL_I2C  (BARCODE_spi_ss2__0__HSIOM_I2C)
    #define BARCODE_SS2_HSIOM_SEL_SPI  (BARCODE_spi_ss2__0__HSIOM_SPI)
#endif /* (BARCODE_SS2_PIN) */

#if (BARCODE_SS3_PIN)
    #define BARCODE_SS3_HSIOM_REG     (*(reg32 *) BARCODE_spi_ss3__0__HSIOM)
    #define BARCODE_SS3_HSIOM_PTR     ( (reg32 *) BARCODE_spi_ss3__0__HSIOM)
    
    #define BARCODE_SS3_HSIOM_MASK     (BARCODE_spi_ss3__0__HSIOM_MASK)
    #define BARCODE_SS3_HSIOM_POS      (BARCODE_spi_ss3__0__HSIOM_SHIFT)
    #define BARCODE_SS3_HSIOM_SEL_GPIO (BARCODE_spi_ss3__0__HSIOM_GPIO)
    #define BARCODE_SS3_HSIOM_SEL_I2C  (BARCODE_spi_ss3__0__HSIOM_I2C)
    #define BARCODE_SS3_HSIOM_SEL_SPI  (BARCODE_spi_ss3__0__HSIOM_SPI)
#endif /* (BARCODE_SS3_PIN) */

#if (BARCODE_I2C_PINS)
    #define BARCODE_SCL_HSIOM_REG  (*(reg32 *) BARCODE_scl__0__HSIOM)
    #define BARCODE_SCL_HSIOM_PTR  ( (reg32 *) BARCODE_scl__0__HSIOM)
    
    #define BARCODE_SCL_HSIOM_MASK     (BARCODE_scl__0__HSIOM_MASK)
    #define BARCODE_SCL_HSIOM_POS      (BARCODE_scl__0__HSIOM_SHIFT)
    #define BARCODE_SCL_HSIOM_SEL_GPIO (BARCODE_sda__0__HSIOM_GPIO)
    #define BARCODE_SCL_HSIOM_SEL_I2C  (BARCODE_sda__0__HSIOM_I2C)
    
    #define BARCODE_SDA_HSIOM_REG  (*(reg32 *) BARCODE_sda__0__HSIOM)
    #define BARCODE_SDA_HSIOM_PTR  ( (reg32 *) BARCODE_sda__0__HSIOM)
    
    #define BARCODE_SDA_HSIOM_MASK     (BARCODE_sda__0__HSIOM_MASK)
    #define BARCODE_SDA_HSIOM_POS      (BARCODE_sda__0__HSIOM_SHIFT)
    #define BARCODE_SDA_HSIOM_SEL_GPIO (BARCODE_sda__0__HSIOM_GPIO)
    #define BARCODE_SDA_HSIOM_SEL_I2C  (BARCODE_sda__0__HSIOM_I2C)
#endif /* (BARCODE_I2C_PINS) */

#if (BARCODE_SPI_SLAVE_PINS)
    #define BARCODE_SCLK_S_HSIOM_REG   (*(reg32 *) BARCODE_sclk_s__0__HSIOM)
    #define BARCODE_SCLK_S_HSIOM_PTR   ( (reg32 *) BARCODE_sclk_s__0__HSIOM)
    
    #define BARCODE_SCLK_S_HSIOM_MASK      (BARCODE_sclk_s__0__HSIOM_MASK)
    #define BARCODE_SCLK_S_HSIOM_POS       (BARCODE_sclk_s__0__HSIOM_SHIFT)
    #define BARCODE_SCLK_S_HSIOM_SEL_GPIO  (BARCODE_sclk_s__0__HSIOM_GPIO)
    #define BARCODE_SCLK_S_HSIOM_SEL_SPI   (BARCODE_sclk_s__0__HSIOM_SPI)
    
    #define BARCODE_SS0_S_HSIOM_REG    (*(reg32 *) BARCODE_ss0_s__0__HSIOM)
    #define BARCODE_SS0_S_HSIOM_PTR    ( (reg32 *) BARCODE_ss0_s__0__HSIOM)
    
    #define BARCODE_SS0_S_HSIOM_MASK       (BARCODE_ss0_s__0__HSIOM_MASK)
    #define BARCODE_SS0_S_HSIOM_POS        (BARCODE_ss0_s__0__HSIOM_SHIFT)
    #define BARCODE_SS0_S_HSIOM_SEL_GPIO   (BARCODE_ss0_s__0__HSIOM_GPIO)  
    #define BARCODE_SS0_S_HSIOM_SEL_SPI    (BARCODE_ss0_s__0__HSIOM_SPI)
#endif /* (BARCODE_SPI_SLAVE_PINS) */

#if (BARCODE_SPI_SLAVE_MOSI_PIN)
    #define BARCODE_MOSI_S_HSIOM_REG   (*(reg32 *) BARCODE_mosi_s__0__HSIOM)
    #define BARCODE_MOSI_S_HSIOM_PTR   ( (reg32 *) BARCODE_mosi_s__0__HSIOM)
    
    #define BARCODE_MOSI_S_HSIOM_MASK      (BARCODE_mosi_s__0__HSIOM_MASK)
    #define BARCODE_MOSI_S_HSIOM_POS       (BARCODE_mosi_s__0__HSIOM_SHIFT)
    #define BARCODE_MOSI_S_HSIOM_SEL_GPIO  (BARCODE_mosi_s__0__HSIOM_GPIO)
    #define BARCODE_MOSI_S_HSIOM_SEL_SPI   (BARCODE_mosi_s__0__HSIOM_SPI)
#endif /* (BARCODE_SPI_SLAVE_MOSI_PIN) */

#if (BARCODE_SPI_SLAVE_MISO_PIN)
    #define BARCODE_MISO_S_HSIOM_REG   (*(reg32 *) BARCODE_miso_s__0__HSIOM)
    #define BARCODE_MISO_S_HSIOM_PTR   ( (reg32 *) BARCODE_miso_s__0__HSIOM)
    
    #define BARCODE_MISO_S_HSIOM_MASK      (BARCODE_miso_s__0__HSIOM_MASK)
    #define BARCODE_MISO_S_HSIOM_POS       (BARCODE_miso_s__0__HSIOM_SHIFT)
    #define BARCODE_MISO_S_HSIOM_SEL_GPIO  (BARCODE_miso_s__0__HSIOM_GPIO)
    #define BARCODE_MISO_S_HSIOM_SEL_SPI   (BARCODE_miso_s__0__HSIOM_SPI)
#endif /* (BARCODE_SPI_SLAVE_MISO_PIN) */

#if (BARCODE_SPI_MASTER_MISO_PIN)
    #define BARCODE_MISO_M_HSIOM_REG   (*(reg32 *) BARCODE_miso_m__0__HSIOM)
    #define BARCODE_MISO_M_HSIOM_PTR   ( (reg32 *) BARCODE_miso_m__0__HSIOM)
    
    #define BARCODE_MISO_M_HSIOM_MASK      (BARCODE_miso_m__0__HSIOM_MASK)
    #define BARCODE_MISO_M_HSIOM_POS       (BARCODE_miso_m__0__HSIOM_SHIFT)
    #define BARCODE_MISO_M_HSIOM_SEL_GPIO  (BARCODE_miso_m__0__HSIOM_GPIO)
    #define BARCODE_MISO_M_HSIOM_SEL_SPI   (BARCODE_miso_m__0__HSIOM_SPI)
#endif /* (BARCODE_SPI_MASTER_MISO_PIN) */

#if (BARCODE_SPI_MASTER_MOSI_PIN)
    #define BARCODE_MOSI_M_HSIOM_REG   (*(reg32 *) BARCODE_mosi_m__0__HSIOM)
    #define BARCODE_MOSI_M_HSIOM_PTR   ( (reg32 *) BARCODE_mosi_m__0__HSIOM)
    
    #define BARCODE_MOSI_M_HSIOM_MASK      (BARCODE_mosi_m__0__HSIOM_MASK)
    #define BARCODE_MOSI_M_HSIOM_POS       (BARCODE_mosi_m__0__HSIOM_SHIFT)
    #define BARCODE_MOSI_M_HSIOM_SEL_GPIO  (BARCODE_mosi_m__0__HSIOM_GPIO)
    #define BARCODE_MOSI_M_HSIOM_SEL_SPI   (BARCODE_mosi_m__0__HSIOM_SPI)
#endif /* (BARCODE_SPI_MASTER_MOSI_PIN) */

#if (BARCODE_SPI_MASTER_SCLK_PIN)
    #define BARCODE_SCLK_M_HSIOM_REG   (*(reg32 *) BARCODE_sclk_m__0__HSIOM)
    #define BARCODE_SCLK_M_HSIOM_PTR   ( (reg32 *) BARCODE_sclk_m__0__HSIOM)
    
    #define BARCODE_SCLK_M_HSIOM_MASK      (BARCODE_sclk_m__0__HSIOM_MASK)
    #define BARCODE_SCLK_M_HSIOM_POS       (BARCODE_sclk_m__0__HSIOM_SHIFT)
    #define BARCODE_SCLK_M_HSIOM_SEL_GPIO  (BARCODE_sclk_m__0__HSIOM_GPIO)
    #define BARCODE_SCLK_M_HSIOM_SEL_SPI   (BARCODE_sclk_m__0__HSIOM_SPI)
#endif /* (BARCODE_SPI_MASTER_SCLK_PIN) */

#if (BARCODE_SPI_MASTER_SS0_PIN)
    #define BARCODE_SS0_M_HSIOM_REG    (*(reg32 *) BARCODE_ss0_m__0__HSIOM)
    #define BARCODE_SS0_M_HSIOM_PTR    ( (reg32 *) BARCODE_ss0_m__0__HSIOM)
    
    #define BARCODE_SS0_M_HSIOM_MASK       (BARCODE_ss0_m__0__HSIOM_MASK)
    #define BARCODE_SS0_M_HSIOM_POS        (BARCODE_ss0_m__0__HSIOM_SHIFT)
    #define BARCODE_SS0_M_HSIOM_SEL_GPIO   (BARCODE_ss0_m__0__HSIOM_GPIO)
    #define BARCODE_SS0_M_HSIOM_SEL_SPI    (BARCODE_ss0_m__0__HSIOM_SPI)
#endif /* (BARCODE_SPI_MASTER_SS0_PIN) */

#if (BARCODE_SPI_MASTER_SS1_PIN)
    #define BARCODE_SS1_M_HSIOM_REG    (*(reg32 *) BARCODE_ss1_m__0__HSIOM)
    #define BARCODE_SS1_M_HSIOM_PTR    ( (reg32 *) BARCODE_ss1_m__0__HSIOM)
    
    #define BARCODE_SS1_M_HSIOM_MASK       (BARCODE_ss1_m__0__HSIOM_MASK)
    #define BARCODE_SS1_M_HSIOM_POS        (BARCODE_ss1_m__0__HSIOM_SHIFT)
    #define BARCODE_SS1_M_HSIOM_SEL_GPIO   (BARCODE_ss1_m__0__HSIOM_GPIO)
    #define BARCODE_SS1_M_HSIOM_SEL_SPI    (BARCODE_ss1_m__0__HSIOM_SPI)
#endif /* (BARCODE_SPI_MASTER_SS1_PIN) */

#if (BARCODE_SPI_MASTER_SS2_PIN)
    #define BARCODE_SS2_M_HSIOM_REG    (*(reg32 *) BARCODE_ss2_m__0__HSIOM)
    #define BARCODE_SS2_M_HSIOM_PTR    ( (reg32 *) BARCODE_ss2_m__0__HSIOM)
    
    #define BARCODE_SS2_M_HSIOM_MASK       (BARCODE_ss2_m__0__HSIOM_MASK)
    #define BARCODE_SS2_M_HSIOM_POS        (BARCODE_ss2_m__0__HSIOM_SHIFT)
    #define BARCODE_SS2_M_HSIOM_SEL_GPIO   (BARCODE_ss2_m__0__HSIOM_GPIO)
    #define BARCODE_SS2_M_HSIOM_SEL_SPI    (BARCODE_ss2_m__0__HSIOM_SPI)
#endif /* (BARCODE_SPI_MASTER_SS2_PIN) */

#if (BARCODE_SPI_MASTER_SS3_PIN)
    #define BARCODE_SS3_M_HSIOM_REG    (*(reg32 *) BARCODE_ss3_m__0__HSIOM)
    #define BARCODE_SS3_M_HSIOM_PTR    ( (reg32 *) BARCODE_ss3_m__0__HSIOM)
    
    #define BARCODE_SS3_M_HSIOM_MASK      (BARCODE_ss3_m__0__HSIOM_MASK)
    #define BARCODE_SS3_M_HSIOM_POS       (BARCODE_ss3_m__0__HSIOM_SHIFT)
    #define BARCODE_SS3_M_HSIOM_SEL_GPIO  (BARCODE_ss3_m__0__HSIOM_GPIO)
    #define BARCODE_SS3_M_HSIOM_SEL_SPI   (BARCODE_ss3_m__0__HSIOM_SPI)
#endif /* (BARCODE_SPI_MASTER_SS3_PIN) */

#if (BARCODE_UART_RX_PIN)
    #define BARCODE_RX_HSIOM_REG   (*(reg32 *) BARCODE_rx__0__HSIOM)
    #define BARCODE_RX_HSIOM_PTR   ( (reg32 *) BARCODE_rx__0__HSIOM)
    
    #define BARCODE_RX_HSIOM_MASK      (BARCODE_rx__0__HSIOM_MASK)
    #define BARCODE_RX_HSIOM_POS       (BARCODE_rx__0__HSIOM_SHIFT)
    #define BARCODE_RX_HSIOM_SEL_GPIO  (BARCODE_rx__0__HSIOM_GPIO)
    #define BARCODE_RX_HSIOM_SEL_UART  (BARCODE_rx__0__HSIOM_UART)
#endif /* (BARCODE_UART_RX_PIN) */

#if (BARCODE_UART_RX_WAKE_PIN)
    #define BARCODE_RX_WAKE_HSIOM_REG   (*(reg32 *) BARCODE_rx_wake__0__HSIOM)
    #define BARCODE_RX_WAKE_HSIOM_PTR   ( (reg32 *) BARCODE_rx_wake__0__HSIOM)
    
    #define BARCODE_RX_WAKE_HSIOM_MASK      (BARCODE_rx_wake__0__HSIOM_MASK)
    #define BARCODE_RX_WAKE_HSIOM_POS       (BARCODE_rx_wake__0__HSIOM_SHIFT)
    #define BARCODE_RX_WAKE_HSIOM_SEL_GPIO  (BARCODE_rx_wake__0__HSIOM_GPIO)
    #define BARCODE_RX_WAKE_HSIOM_SEL_UART  (BARCODE_rx_wake__0__HSIOM_UART)
#endif /* (BARCODE_UART_WAKE_RX_PIN) */

#if (BARCODE_UART_CTS_PIN)
    #define BARCODE_CTS_HSIOM_REG   (*(reg32 *) BARCODE_cts__0__HSIOM)
    #define BARCODE_CTS_HSIOM_PTR   ( (reg32 *) BARCODE_cts__0__HSIOM)
    
    #define BARCODE_CTS_HSIOM_MASK      (BARCODE_cts__0__HSIOM_MASK)
    #define BARCODE_CTS_HSIOM_POS       (BARCODE_cts__0__HSIOM_SHIFT)
    #define BARCODE_CTS_HSIOM_SEL_GPIO  (BARCODE_cts__0__HSIOM_GPIO)
    #define BARCODE_CTS_HSIOM_SEL_UART  (BARCODE_cts__0__HSIOM_UART)
#endif /* (BARCODE_UART_CTS_PIN) */

#if (BARCODE_UART_TX_PIN)
    #define BARCODE_TX_HSIOM_REG   (*(reg32 *) BARCODE_tx__0__HSIOM)
    #define BARCODE_TX_HSIOM_PTR   ( (reg32 *) BARCODE_tx__0__HSIOM)
    
    #define BARCODE_TX_HSIOM_MASK      (BARCODE_tx__0__HSIOM_MASK)
    #define BARCODE_TX_HSIOM_POS       (BARCODE_tx__0__HSIOM_SHIFT)
    #define BARCODE_TX_HSIOM_SEL_GPIO  (BARCODE_tx__0__HSIOM_GPIO)
    #define BARCODE_TX_HSIOM_SEL_UART  (BARCODE_tx__0__HSIOM_UART)
#endif /* (BARCODE_UART_TX_PIN) */

#if (BARCODE_UART_RX_TX_PIN)
    #define BARCODE_RX_TX_HSIOM_REG   (*(reg32 *) BARCODE_rx_tx__0__HSIOM)
    #define BARCODE_RX_TX_HSIOM_PTR   ( (reg32 *) BARCODE_rx_tx__0__HSIOM)
    
    #define BARCODE_RX_TX_HSIOM_MASK      (BARCODE_rx_tx__0__HSIOM_MASK)
    #define BARCODE_RX_TX_HSIOM_POS       (BARCODE_rx_tx__0__HSIOM_SHIFT)
    #define BARCODE_RX_TX_HSIOM_SEL_GPIO  (BARCODE_rx_tx__0__HSIOM_GPIO)
    #define BARCODE_RX_TX_HSIOM_SEL_UART  (BARCODE_rx_tx__0__HSIOM_UART)
#endif /* (BARCODE_UART_RX_TX_PIN) */

#if (BARCODE_UART_RTS_PIN)
    #define BARCODE_RTS_HSIOM_REG      (*(reg32 *) BARCODE_rts__0__HSIOM)
    #define BARCODE_RTS_HSIOM_PTR      ( (reg32 *) BARCODE_rts__0__HSIOM)
    
    #define BARCODE_RTS_HSIOM_MASK     (BARCODE_rts__0__HSIOM_MASK)
    #define BARCODE_RTS_HSIOM_POS      (BARCODE_rts__0__HSIOM_SHIFT)    
    #define BARCODE_RTS_HSIOM_SEL_GPIO (BARCODE_rts__0__HSIOM_GPIO)
    #define BARCODE_RTS_HSIOM_SEL_UART (BARCODE_rts__0__HSIOM_UART)    
#endif /* (BARCODE_UART_RTS_PIN) */


/***************************************
*        Registers Constants
***************************************/

/* HSIOM switch values. */ 
#define BARCODE_HSIOM_DEF_SEL      (0x00u)
#define BARCODE_HSIOM_GPIO_SEL     (0x00u)
/* The HSIOM values provided below are valid only for BARCODE_CY_SCBIP_V0 
* and BARCODE_CY_SCBIP_V1. It is not recommended to use them for 
* BARCODE_CY_SCBIP_V2. Use pin name specific HSIOM constants provided 
* above instead for any SCB IP block version.
*/
#define BARCODE_HSIOM_UART_SEL     (0x09u)
#define BARCODE_HSIOM_I2C_SEL      (0x0Eu)
#define BARCODE_HSIOM_SPI_SEL      (0x0Fu)

/* Pins settings index. */
#define BARCODE_RX_WAKE_SCL_MOSI_PIN_INDEX   (0u)
#define BARCODE_RX_SCL_MOSI_PIN_INDEX       (0u)
#define BARCODE_TX_SDA_MISO_PIN_INDEX       (1u)
#define BARCODE_CTS_SCLK_PIN_INDEX       (2u)
#define BARCODE_RTS_SS0_PIN_INDEX       (3u)
#define BARCODE_SS1_PIN_INDEX                  (4u)
#define BARCODE_SS2_PIN_INDEX                  (5u)
#define BARCODE_SS3_PIN_INDEX                  (6u)

/* Pins settings mask. */
#define BARCODE_RX_WAKE_SCL_MOSI_PIN_MASK ((uint32) 0x01u << BARCODE_RX_WAKE_SCL_MOSI_PIN_INDEX)
#define BARCODE_RX_SCL_MOSI_PIN_MASK     ((uint32) 0x01u << BARCODE_RX_SCL_MOSI_PIN_INDEX)
#define BARCODE_TX_SDA_MISO_PIN_MASK     ((uint32) 0x01u << BARCODE_TX_SDA_MISO_PIN_INDEX)
#define BARCODE_CTS_SCLK_PIN_MASK     ((uint32) 0x01u << BARCODE_CTS_SCLK_PIN_INDEX)
#define BARCODE_RTS_SS0_PIN_MASK     ((uint32) 0x01u << BARCODE_RTS_SS0_PIN_INDEX)
#define BARCODE_SS1_PIN_MASK                ((uint32) 0x01u << BARCODE_SS1_PIN_INDEX)
#define BARCODE_SS2_PIN_MASK                ((uint32) 0x01u << BARCODE_SS2_PIN_INDEX)
#define BARCODE_SS3_PIN_MASK                ((uint32) 0x01u << BARCODE_SS3_PIN_INDEX)

/* Pin interrupt constants. */
#define BARCODE_INTCFG_TYPE_MASK           (0x03u)
#define BARCODE_INTCFG_TYPE_FALLING_EDGE   (0x02u)

/* Pin Drive Mode constants. */
#define BARCODE_PIN_DM_ALG_HIZ  (0u)
#define BARCODE_PIN_DM_DIG_HIZ  (1u)
#define BARCODE_PIN_DM_OD_LO    (4u)
#define BARCODE_PIN_DM_STRONG   (6u)


/***************************************
*          Macro Definitions
***************************************/

/* Return drive mode of the pin */
#define BARCODE_DM_MASK    (0x7u)
#define BARCODE_DM_SIZE    (3u)
#define BARCODE_GET_P4_PIN_DM(reg, pos) \
    ( ((reg) & (uint32) ((uint32) BARCODE_DM_MASK << (BARCODE_DM_SIZE * (pos)))) >> \
                                                              (BARCODE_DM_SIZE * (pos)) )

#if (BARCODE_TX_SDA_MISO_PIN)
    #define BARCODE_CHECK_TX_SDA_MISO_PIN_USED \
                (BARCODE_PIN_DM_ALG_HIZ != \
                    BARCODE_GET_P4_PIN_DM(BARCODE_uart_tx_i2c_sda_spi_miso_PC, \
                                                   BARCODE_uart_tx_i2c_sda_spi_miso_SHIFT))
#endif /* (BARCODE_TX_SDA_MISO_PIN) */

#if (BARCODE_RTS_SS0_PIN)
    #define BARCODE_CHECK_RTS_SS0_PIN_USED \
                (BARCODE_PIN_DM_ALG_HIZ != \
                    BARCODE_GET_P4_PIN_DM(BARCODE_uart_rts_spi_ss0_PC, \
                                                   BARCODE_uart_rts_spi_ss0_SHIFT))
#endif /* (BARCODE_RTS_SS0_PIN) */

/* Set bits-mask in register */
#define BARCODE_SET_REGISTER_BITS(reg, mask, pos, mode) \
                    do                                           \
                    {                                            \
                        (reg) = (((reg) & ((uint32) ~(uint32) (mask))) | ((uint32) ((uint32) (mode) << (pos)))); \
                    }while(0)

/* Set bit in the register */
#define BARCODE_SET_REGISTER_BIT(reg, mask, val) \
                    ((val) ? ((reg) |= (mask)) : ((reg) &= ((uint32) ~((uint32) (mask)))))

#define BARCODE_SET_HSIOM_SEL(reg, mask, pos, sel) BARCODE_SET_REGISTER_BITS(reg, mask, pos, sel)
#define BARCODE_SET_INCFG_TYPE(reg, mask, pos, intType) \
                                                        BARCODE_SET_REGISTER_BITS(reg, mask, pos, intType)
#define BARCODE_SET_INP_DIS(reg, mask, val) BARCODE_SET_REGISTER_BIT(reg, mask, val)

/* BARCODE_SET_I2C_SCL_DR(val) - Sets I2C SCL DR register.
*  BARCODE_SET_I2C_SCL_HSIOM_SEL(sel) - Sets I2C SCL HSIOM settings.
*/
/* SCB I2C: scl signal */
#if (BARCODE_CY_SCBIP_V0)
#if (BARCODE_I2C_PINS)
    #define BARCODE_SET_I2C_SCL_DR(val) BARCODE_scl_Write(val)

    #define BARCODE_SET_I2C_SCL_HSIOM_SEL(sel) \
                          BARCODE_SET_HSIOM_SEL(BARCODE_SCL_HSIOM_REG,  \
                                                         BARCODE_SCL_HSIOM_MASK, \
                                                         BARCODE_SCL_HSIOM_POS,  \
                                                         (sel))
    #define BARCODE_WAIT_SCL_SET_HIGH  (0u == BARCODE_scl_Read())

/* Unconfigured SCB: scl signal */
#elif (BARCODE_RX_WAKE_SCL_MOSI_PIN)
    #define BARCODE_SET_I2C_SCL_DR(val) \
                            BARCODE_uart_rx_wake_i2c_scl_spi_mosi_Write(val)

    #define BARCODE_SET_I2C_SCL_HSIOM_SEL(sel) \
                    BARCODE_SET_HSIOM_SEL(BARCODE_RX_WAKE_SCL_MOSI_HSIOM_REG,  \
                                                   BARCODE_RX_WAKE_SCL_MOSI_HSIOM_MASK, \
                                                   BARCODE_RX_WAKE_SCL_MOSI_HSIOM_POS,  \
                                                   (sel))

    #define BARCODE_WAIT_SCL_SET_HIGH  (0u == BARCODE_uart_rx_wake_i2c_scl_spi_mosi_Read())

#elif (BARCODE_RX_SCL_MOSI_PIN)
    #define BARCODE_SET_I2C_SCL_DR(val) \
                            BARCODE_uart_rx_i2c_scl_spi_mosi_Write(val)


    #define BARCODE_SET_I2C_SCL_HSIOM_SEL(sel) \
                            BARCODE_SET_HSIOM_SEL(BARCODE_RX_SCL_MOSI_HSIOM_REG,  \
                                                           BARCODE_RX_SCL_MOSI_HSIOM_MASK, \
                                                           BARCODE_RX_SCL_MOSI_HSIOM_POS,  \
                                                           (sel))

    #define BARCODE_WAIT_SCL_SET_HIGH  (0u == BARCODE_uart_rx_i2c_scl_spi_mosi_Read())

#else
    #define BARCODE_SET_I2C_SCL_DR(val)        do{ /* Does nothing */ }while(0)
    #define BARCODE_SET_I2C_SCL_HSIOM_SEL(sel) do{ /* Does nothing */ }while(0)

    #define BARCODE_WAIT_SCL_SET_HIGH  (0u)
#endif /* (BARCODE_I2C_PINS) */

/* SCB I2C: sda signal */
#if (BARCODE_I2C_PINS)
    #define BARCODE_WAIT_SDA_SET_HIGH  (0u == BARCODE_sda_Read())
/* Unconfigured SCB: sda signal */
#elif (BARCODE_TX_SDA_MISO_PIN)
    #define BARCODE_WAIT_SDA_SET_HIGH  (0u == BARCODE_uart_tx_i2c_sda_spi_miso_Read())
#else
    #define BARCODE_WAIT_SDA_SET_HIGH  (0u)
#endif /* (BARCODE_MOSI_SCL_RX_PIN) */
#endif /* (BARCODE_CY_SCBIP_V0) */

/* Clear UART wakeup source */
#if (BARCODE_RX_SCL_MOSI_PIN)
    #define BARCODE_CLEAR_UART_RX_WAKE_INTR        do{ /* Does nothing */ }while(0)
    
#elif (BARCODE_RX_WAKE_SCL_MOSI_PIN)
    #define BARCODE_CLEAR_UART_RX_WAKE_INTR \
            do{                                      \
                (void) BARCODE_uart_rx_wake_i2c_scl_spi_mosi_ClearInterrupt(); \
            }while(0)

#elif(BARCODE_UART_RX_WAKE_PIN)
    #define BARCODE_CLEAR_UART_RX_WAKE_INTR \
            do{                                      \
                (void) BARCODE_rx_wake_ClearInterrupt(); \
            }while(0)
#else
#endif /* (BARCODE_RX_SCL_MOSI_PIN) */


/***************************************
* The following code is DEPRECATED and
* must not be used.
***************************************/

/* Unconfigured pins */
#define BARCODE_REMOVE_MOSI_SCL_RX_WAKE_PIN    BARCODE_REMOVE_RX_WAKE_SCL_MOSI_PIN
#define BARCODE_REMOVE_MOSI_SCL_RX_PIN         BARCODE_REMOVE_RX_SCL_MOSI_PIN
#define BARCODE_REMOVE_MISO_SDA_TX_PIN         BARCODE_REMOVE_TX_SDA_MISO_PIN
#ifndef BARCODE_REMOVE_SCLK_PIN
#define BARCODE_REMOVE_SCLK_PIN                BARCODE_REMOVE_CTS_SCLK_PIN
#endif /* BARCODE_REMOVE_SCLK_PIN */
#ifndef BARCODE_REMOVE_SS0_PIN
#define BARCODE_REMOVE_SS0_PIN                 BARCODE_REMOVE_RTS_SS0_PIN
#endif /* BARCODE_REMOVE_SS0_PIN */

/* Unconfigured pins */
#define BARCODE_MOSI_SCL_RX_WAKE_PIN   BARCODE_RX_WAKE_SCL_MOSI_PIN
#define BARCODE_MOSI_SCL_RX_PIN        BARCODE_RX_SCL_MOSI_PIN
#define BARCODE_MISO_SDA_TX_PIN        BARCODE_TX_SDA_MISO_PIN
#ifndef BARCODE_SCLK_PIN
#define BARCODE_SCLK_PIN               BARCODE_CTS_SCLK_PIN
#endif /* BARCODE_SCLK_PIN */
#ifndef BARCODE_SS0_PIN
#define BARCODE_SS0_PIN                BARCODE_RTS_SS0_PIN
#endif /* BARCODE_SS0_PIN */

#if (BARCODE_MOSI_SCL_RX_WAKE_PIN)
    #define BARCODE_MOSI_SCL_RX_WAKE_HSIOM_REG     BARCODE_RX_WAKE_SCL_MOSI_HSIOM_REG
    #define BARCODE_MOSI_SCL_RX_WAKE_HSIOM_PTR     BARCODE_RX_WAKE_SCL_MOSI_HSIOM_REG
    #define BARCODE_MOSI_SCL_RX_WAKE_HSIOM_MASK    BARCODE_RX_WAKE_SCL_MOSI_HSIOM_REG
    #define BARCODE_MOSI_SCL_RX_WAKE_HSIOM_POS     BARCODE_RX_WAKE_SCL_MOSI_HSIOM_REG

    #define BARCODE_MOSI_SCL_RX_WAKE_INTCFG_REG    BARCODE_RX_WAKE_SCL_MOSI_HSIOM_REG
    #define BARCODE_MOSI_SCL_RX_WAKE_INTCFG_PTR    BARCODE_RX_WAKE_SCL_MOSI_HSIOM_REG

    #define BARCODE_MOSI_SCL_RX_WAKE_INTCFG_TYPE_POS   BARCODE_RX_WAKE_SCL_MOSI_HSIOM_REG
    #define BARCODE_MOSI_SCL_RX_WAKE_INTCFG_TYPE_MASK  BARCODE_RX_WAKE_SCL_MOSI_HSIOM_REG
#endif /* (BARCODE_RX_WAKE_SCL_MOSI_PIN) */

#if (BARCODE_MOSI_SCL_RX_PIN)
    #define BARCODE_MOSI_SCL_RX_HSIOM_REG      BARCODE_RX_SCL_MOSI_HSIOM_REG
    #define BARCODE_MOSI_SCL_RX_HSIOM_PTR      BARCODE_RX_SCL_MOSI_HSIOM_PTR
    #define BARCODE_MOSI_SCL_RX_HSIOM_MASK     BARCODE_RX_SCL_MOSI_HSIOM_MASK
    #define BARCODE_MOSI_SCL_RX_HSIOM_POS      BARCODE_RX_SCL_MOSI_HSIOM_POS
#endif /* (BARCODE_MOSI_SCL_RX_PIN) */

#if (BARCODE_MISO_SDA_TX_PIN)
    #define BARCODE_MISO_SDA_TX_HSIOM_REG      BARCODE_TX_SDA_MISO_HSIOM_REG
    #define BARCODE_MISO_SDA_TX_HSIOM_PTR      BARCODE_TX_SDA_MISO_HSIOM_REG
    #define BARCODE_MISO_SDA_TX_HSIOM_MASK     BARCODE_TX_SDA_MISO_HSIOM_REG
    #define BARCODE_MISO_SDA_TX_HSIOM_POS      BARCODE_TX_SDA_MISO_HSIOM_REG
#endif /* (BARCODE_MISO_SDA_TX_PIN_PIN) */

#if (BARCODE_SCLK_PIN)
    #ifndef BARCODE_SCLK_HSIOM_REG
    #define BARCODE_SCLK_HSIOM_REG     BARCODE_CTS_SCLK_HSIOM_REG
    #define BARCODE_SCLK_HSIOM_PTR     BARCODE_CTS_SCLK_HSIOM_PTR
    #define BARCODE_SCLK_HSIOM_MASK    BARCODE_CTS_SCLK_HSIOM_MASK
    #define BARCODE_SCLK_HSIOM_POS     BARCODE_CTS_SCLK_HSIOM_POS
    #endif /* BARCODE_SCLK_HSIOM_REG */
#endif /* (BARCODE_SCLK_PIN) */

#if (BARCODE_SS0_PIN)
    #ifndef BARCODE_SS0_HSIOM_REG
    #define BARCODE_SS0_HSIOM_REG      BARCODE_RTS_SS0_HSIOM_REG
    #define BARCODE_SS0_HSIOM_PTR      BARCODE_RTS_SS0_HSIOM_PTR
    #define BARCODE_SS0_HSIOM_MASK     BARCODE_RTS_SS0_HSIOM_MASK
    #define BARCODE_SS0_HSIOM_POS      BARCODE_RTS_SS0_HSIOM_POS
    #endif /* BARCODE_SS0_HSIOM_REG */
#endif /* (BARCODE_SS0_PIN) */

#define BARCODE_MOSI_SCL_RX_WAKE_PIN_INDEX BARCODE_RX_WAKE_SCL_MOSI_PIN_INDEX
#define BARCODE_MOSI_SCL_RX_PIN_INDEX      BARCODE_RX_SCL_MOSI_PIN_INDEX
#define BARCODE_MISO_SDA_TX_PIN_INDEX      BARCODE_TX_SDA_MISO_PIN_INDEX
#ifndef BARCODE_SCLK_PIN_INDEX
#define BARCODE_SCLK_PIN_INDEX             BARCODE_CTS_SCLK_PIN_INDEX
#endif /* BARCODE_SCLK_PIN_INDEX */
#ifndef BARCODE_SS0_PIN_INDEX
#define BARCODE_SS0_PIN_INDEX              BARCODE_RTS_SS0_PIN_INDEX
#endif /* BARCODE_SS0_PIN_INDEX */

#define BARCODE_MOSI_SCL_RX_WAKE_PIN_MASK BARCODE_RX_WAKE_SCL_MOSI_PIN_MASK
#define BARCODE_MOSI_SCL_RX_PIN_MASK      BARCODE_RX_SCL_MOSI_PIN_MASK
#define BARCODE_MISO_SDA_TX_PIN_MASK      BARCODE_TX_SDA_MISO_PIN_MASK
#ifndef BARCODE_SCLK_PIN_MASK
#define BARCODE_SCLK_PIN_MASK             BARCODE_CTS_SCLK_PIN_MASK
#endif /* BARCODE_SCLK_PIN_MASK */
#ifndef BARCODE_SS0_PIN_MASK
#define BARCODE_SS0_PIN_MASK              BARCODE_RTS_SS0_PIN_MASK
#endif /* BARCODE_SS0_PIN_MASK */

#endif /* (CY_SCB_PINS_BARCODE_H) */


/* [] END OF FILE */
