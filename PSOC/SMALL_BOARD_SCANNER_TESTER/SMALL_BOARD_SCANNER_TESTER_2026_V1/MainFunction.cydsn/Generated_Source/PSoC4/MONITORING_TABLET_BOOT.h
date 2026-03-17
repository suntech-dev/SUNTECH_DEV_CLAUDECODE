/***************************************************************************//**
* \file MONITORING_TABLET_BOOT.h
* \version 4.0
*
* \brief
*  This file provides constants and parameter values of the bootloader
*  communication APIs for the SCB Component.
*
* Note:
*
********************************************************************************
* \copyright
* Copyright 2014-2017, Cypress Semiconductor Corporation. All rights reserved.
* You may use this file only in accordance with the license, terms, conditions,
* disclaimers, and limitations in the end user license agreement accompanying
* the software package with which this file was provided.
*******************************************************************************/

#if !defined(CY_SCB_BOOT_MONITORING_TABLET_H)
#define CY_SCB_BOOT_MONITORING_TABLET_H

#include "MONITORING_TABLET_PVT.h"

#if (MONITORING_TABLET_SCB_MODE_I2C_INC)
    #include "MONITORING_TABLET_I2C.h"
#endif /* (MONITORING_TABLET_SCB_MODE_I2C_INC) */

#if (MONITORING_TABLET_SCB_MODE_EZI2C_INC)
    #include "MONITORING_TABLET_EZI2C.h"
#endif /* (MONITORING_TABLET_SCB_MODE_EZI2C_INC) */

#if (MONITORING_TABLET_SCB_MODE_SPI_INC || MONITORING_TABLET_SCB_MODE_UART_INC)
    #include "MONITORING_TABLET_SPI_UART.h"
#endif /* (MONITORING_TABLET_SCB_MODE_SPI_INC || MONITORING_TABLET_SCB_MODE_UART_INC) */


/***************************************
*  Conditional Compilation Parameters
****************************************/

/* Bootloader communication interface enable */
#define MONITORING_TABLET_BTLDR_COMM_ENABLED ((CYDEV_BOOTLOADER_IO_COMP == CyBtldr_MONITORING_TABLET) || \
                                             (CYDEV_BOOTLOADER_IO_COMP == CyBtldr_Custom_Interface))

/* Enable I2C bootloader communication */
#if (MONITORING_TABLET_SCB_MODE_I2C_INC)
    #define MONITORING_TABLET_I2C_BTLDR_COMM_ENABLED     (MONITORING_TABLET_BTLDR_COMM_ENABLED && \
                                                            (MONITORING_TABLET_SCB_MODE_UNCONFIG_CONST_CFG || \
                                                             MONITORING_TABLET_I2C_SLAVE_CONST))
#else
     #define MONITORING_TABLET_I2C_BTLDR_COMM_ENABLED    (0u)
#endif /* (MONITORING_TABLET_SCB_MODE_I2C_INC) */

/* EZI2C does not support bootloader communication. Provide empty APIs */
#if (MONITORING_TABLET_SCB_MODE_EZI2C_INC)
    #define MONITORING_TABLET_EZI2C_BTLDR_COMM_ENABLED   (MONITORING_TABLET_BTLDR_COMM_ENABLED && \
                                                         MONITORING_TABLET_SCB_MODE_UNCONFIG_CONST_CFG)
#else
    #define MONITORING_TABLET_EZI2C_BTLDR_COMM_ENABLED   (0u)
#endif /* (MONITORING_TABLET_EZI2C_BTLDR_COMM_ENABLED) */

/* Enable SPI bootloader communication */
#if (MONITORING_TABLET_SCB_MODE_SPI_INC)
    #define MONITORING_TABLET_SPI_BTLDR_COMM_ENABLED     (MONITORING_TABLET_BTLDR_COMM_ENABLED && \
                                                            (MONITORING_TABLET_SCB_MODE_UNCONFIG_CONST_CFG || \
                                                             MONITORING_TABLET_SPI_SLAVE_CONST))
#else
        #define MONITORING_TABLET_SPI_BTLDR_COMM_ENABLED (0u)
#endif /* (MONITORING_TABLET_SPI_BTLDR_COMM_ENABLED) */

/* Enable UART bootloader communication */
#if (MONITORING_TABLET_SCB_MODE_UART_INC)
       #define MONITORING_TABLET_UART_BTLDR_COMM_ENABLED    (MONITORING_TABLET_BTLDR_COMM_ENABLED && \
                                                            (MONITORING_TABLET_SCB_MODE_UNCONFIG_CONST_CFG || \
                                                             (MONITORING_TABLET_UART_RX_DIRECTION && \
                                                              MONITORING_TABLET_UART_TX_DIRECTION)))
#else
     #define MONITORING_TABLET_UART_BTLDR_COMM_ENABLED   (0u)
#endif /* (MONITORING_TABLET_UART_BTLDR_COMM_ENABLED) */

/* Enable bootloader communication */
#define MONITORING_TABLET_BTLDR_COMM_MODE_ENABLED    (MONITORING_TABLET_I2C_BTLDR_COMM_ENABLED   || \
                                                     MONITORING_TABLET_SPI_BTLDR_COMM_ENABLED   || \
                                                     MONITORING_TABLET_EZI2C_BTLDR_COMM_ENABLED || \
                                                     MONITORING_TABLET_UART_BTLDR_COMM_ENABLED)


/***************************************
*        Function Prototypes
***************************************/

#if defined(CYDEV_BOOTLOADER_IO_COMP) && (MONITORING_TABLET_I2C_BTLDR_COMM_ENABLED)
    /* I2C Bootloader physical layer functions */
    void MONITORING_TABLET_I2CCyBtldrCommStart(void);
    void MONITORING_TABLET_I2CCyBtldrCommStop (void);
    void MONITORING_TABLET_I2CCyBtldrCommReset(void);
    cystatus MONITORING_TABLET_I2CCyBtldrCommRead       (uint8 pData[], uint16 size, uint16 * count, uint8 timeOut);
    cystatus MONITORING_TABLET_I2CCyBtldrCommWrite(const uint8 pData[], uint16 size, uint16 * count, uint8 timeOut);

    /* Map I2C specific bootloader communication APIs to SCB specific APIs */
    #if (MONITORING_TABLET_SCB_MODE_I2C_CONST_CFG)
        #define MONITORING_TABLET_CyBtldrCommStart   MONITORING_TABLET_I2CCyBtldrCommStart
        #define MONITORING_TABLET_CyBtldrCommStop    MONITORING_TABLET_I2CCyBtldrCommStop
        #define MONITORING_TABLET_CyBtldrCommReset   MONITORING_TABLET_I2CCyBtldrCommReset
        #define MONITORING_TABLET_CyBtldrCommRead    MONITORING_TABLET_I2CCyBtldrCommRead
        #define MONITORING_TABLET_CyBtldrCommWrite   MONITORING_TABLET_I2CCyBtldrCommWrite
    #endif /* (MONITORING_TABLET_SCB_MODE_I2C_CONST_CFG) */

#endif /* defined(CYDEV_BOOTLOADER_IO_COMP) && (MONITORING_TABLET_I2C_BTLDR_COMM_ENABLED) */


#if defined(CYDEV_BOOTLOADER_IO_COMP) && (MONITORING_TABLET_EZI2C_BTLDR_COMM_ENABLED)
    /* Bootloader physical layer functions */
    void MONITORING_TABLET_EzI2CCyBtldrCommStart(void);
    void MONITORING_TABLET_EzI2CCyBtldrCommStop (void);
    void MONITORING_TABLET_EzI2CCyBtldrCommReset(void);
    cystatus MONITORING_TABLET_EzI2CCyBtldrCommRead       (uint8 pData[], uint16 size, uint16 * count, uint8 timeOut);
    cystatus MONITORING_TABLET_EzI2CCyBtldrCommWrite(const uint8 pData[], uint16 size, uint16 * count, uint8 timeOut);

    /* Map EZI2C specific bootloader communication APIs to SCB specific APIs */
    #if (MONITORING_TABLET_SCB_MODE_EZI2C_CONST_CFG)
        #define MONITORING_TABLET_CyBtldrCommStart   MONITORING_TABLET_EzI2CCyBtldrCommStart
        #define MONITORING_TABLET_CyBtldrCommStop    MONITORING_TABLET_EzI2CCyBtldrCommStop
        #define MONITORING_TABLET_CyBtldrCommReset   MONITORING_TABLET_EzI2CCyBtldrCommReset
        #define MONITORING_TABLET_CyBtldrCommRead    MONITORING_TABLET_EzI2CCyBtldrCommRead
        #define MONITORING_TABLET_CyBtldrCommWrite   MONITORING_TABLET_EzI2CCyBtldrCommWrite
    #endif /* (MONITORING_TABLET_SCB_MODE_EZI2C_CONST_CFG) */

#endif /* defined(CYDEV_BOOTLOADER_IO_COMP) && (MONITORING_TABLET_EZI2C_BTLDR_COMM_ENABLED) */

#if defined(CYDEV_BOOTLOADER_IO_COMP) && (MONITORING_TABLET_SPI_BTLDR_COMM_ENABLED)
    /* SPI Bootloader physical layer functions */
    void MONITORING_TABLET_SpiCyBtldrCommStart(void);
    void MONITORING_TABLET_SpiCyBtldrCommStop (void);
    void MONITORING_TABLET_SpiCyBtldrCommReset(void);
    cystatus MONITORING_TABLET_SpiCyBtldrCommRead       (uint8 pData[], uint16 size, uint16 * count, uint8 timeOut);
    cystatus MONITORING_TABLET_SpiCyBtldrCommWrite(const uint8 pData[], uint16 size, uint16 * count, uint8 timeOut);

    /* Map SPI specific bootloader communication APIs to SCB specific APIs */
    #if (MONITORING_TABLET_SCB_MODE_SPI_CONST_CFG)
        #define MONITORING_TABLET_CyBtldrCommStart   MONITORING_TABLET_SpiCyBtldrCommStart
        #define MONITORING_TABLET_CyBtldrCommStop    MONITORING_TABLET_SpiCyBtldrCommStop
        #define MONITORING_TABLET_CyBtldrCommReset   MONITORING_TABLET_SpiCyBtldrCommReset
        #define MONITORING_TABLET_CyBtldrCommRead    MONITORING_TABLET_SpiCyBtldrCommRead
        #define MONITORING_TABLET_CyBtldrCommWrite   MONITORING_TABLET_SpiCyBtldrCommWrite
    #endif /* (MONITORING_TABLET_SCB_MODE_SPI_CONST_CFG) */

#endif /* defined(CYDEV_BOOTLOADER_IO_COMP) && (MONITORING_TABLET_SPI_BTLDR_COMM_ENABLED) */

#if defined(CYDEV_BOOTLOADER_IO_COMP) && (MONITORING_TABLET_UART_BTLDR_COMM_ENABLED)
    /* UART Bootloader physical layer functions */
    void MONITORING_TABLET_UartCyBtldrCommStart(void);
    void MONITORING_TABLET_UartCyBtldrCommStop (void);
    void MONITORING_TABLET_UartCyBtldrCommReset(void);
    cystatus MONITORING_TABLET_UartCyBtldrCommRead       (uint8 pData[], uint16 size, uint16 * count, uint8 timeOut);
    cystatus MONITORING_TABLET_UartCyBtldrCommWrite(const uint8 pData[], uint16 size, uint16 * count, uint8 timeOut);

    /* Map UART specific bootloader communication APIs to SCB specific APIs */
    #if (MONITORING_TABLET_SCB_MODE_UART_CONST_CFG)
        #define MONITORING_TABLET_CyBtldrCommStart   MONITORING_TABLET_UartCyBtldrCommStart
        #define MONITORING_TABLET_CyBtldrCommStop    MONITORING_TABLET_UartCyBtldrCommStop
        #define MONITORING_TABLET_CyBtldrCommReset   MONITORING_TABLET_UartCyBtldrCommReset
        #define MONITORING_TABLET_CyBtldrCommRead    MONITORING_TABLET_UartCyBtldrCommRead
        #define MONITORING_TABLET_CyBtldrCommWrite   MONITORING_TABLET_UartCyBtldrCommWrite
    #endif /* (MONITORING_TABLET_SCB_MODE_UART_CONST_CFG) */

#endif /* defined(CYDEV_BOOTLOADER_IO_COMP) && (MONITORING_TABLET_UART_BTLDR_COMM_ENABLED) */

/**
* \addtogroup group_bootloader
* @{
*/

#if defined(CYDEV_BOOTLOADER_IO_COMP) && (MONITORING_TABLET_BTLDR_COMM_ENABLED)
    #if (MONITORING_TABLET_SCB_MODE_UNCONFIG_CONST_CFG)
        /* Bootloader physical layer functions */
        void MONITORING_TABLET_CyBtldrCommStart(void);
        void MONITORING_TABLET_CyBtldrCommStop (void);
        void MONITORING_TABLET_CyBtldrCommReset(void);
        cystatus MONITORING_TABLET_CyBtldrCommRead       (uint8 pData[], uint16 size, uint16 * count, uint8 timeOut);
        cystatus MONITORING_TABLET_CyBtldrCommWrite(const uint8 pData[], uint16 size, uint16 * count, uint8 timeOut);
    #endif /* (MONITORING_TABLET_SCB_MODE_UNCONFIG_CONST_CFG) */

    /* Map SCB specific bootloader communication APIs to common APIs */
    #if (CYDEV_BOOTLOADER_IO_COMP == CyBtldr_MONITORING_TABLET)
        #define CyBtldrCommStart    MONITORING_TABLET_CyBtldrCommStart
        #define CyBtldrCommStop     MONITORING_TABLET_CyBtldrCommStop
        #define CyBtldrCommReset    MONITORING_TABLET_CyBtldrCommReset
        #define CyBtldrCommWrite    MONITORING_TABLET_CyBtldrCommWrite
        #define CyBtldrCommRead     MONITORING_TABLET_CyBtldrCommRead
    #endif /* (CYDEV_BOOTLOADER_IO_COMP == CyBtldr_MONITORING_TABLET) */

#endif /* defined(CYDEV_BOOTLOADER_IO_COMP) && (MONITORING_TABLET_BTLDR_COMM_ENABLED) */

/** @} group_bootloader */

/***************************************
*           API Constants
***************************************/

/* Timeout unit in milliseconds */
#define MONITORING_TABLET_WAIT_1_MS  (1u)

/* Return number of bytes to copy into bootloader buffer */
#define MONITORING_TABLET_BYTES_TO_COPY(actBufSize, bufSize) \
                            ( ((uint32)(actBufSize) < (uint32)(bufSize)) ? \
                                ((uint32) (actBufSize)) : ((uint32) (bufSize)) )

/* Size of Read/Write buffers for I2C bootloader  */
#define MONITORING_TABLET_I2C_BTLDR_SIZEOF_READ_BUFFER   (64u)
#define MONITORING_TABLET_I2C_BTLDR_SIZEOF_WRITE_BUFFER  (64u)

/* Byte to byte time interval: calculated basing on current component
* data rate configuration, can be defined in project if required.
*/
#ifndef MONITORING_TABLET_SPI_BYTE_TO_BYTE
    #define MONITORING_TABLET_SPI_BYTE_TO_BYTE   (160u)
#endif

/* Byte to byte time interval: calculated basing on current component
* baud rate configuration, can be defined in the project if required.
*/
#ifndef MONITORING_TABLET_UART_BYTE_TO_BYTE
    #define MONITORING_TABLET_UART_BYTE_TO_BYTE  (2086u)
#endif /* MONITORING_TABLET_UART_BYTE_TO_BYTE */

#endif /* (CY_SCB_BOOT_MONITORING_TABLET_H) */


/* [] END OF FILE */
