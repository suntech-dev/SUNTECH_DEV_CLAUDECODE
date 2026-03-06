/***************************************************************************//**
* \file BARCODE_BOOT.h
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

#if !defined(CY_SCB_BOOT_BARCODE_H)
#define CY_SCB_BOOT_BARCODE_H

#include "BARCODE_PVT.h"

#if (BARCODE_SCB_MODE_I2C_INC)
    #include "BARCODE_I2C.h"
#endif /* (BARCODE_SCB_MODE_I2C_INC) */

#if (BARCODE_SCB_MODE_EZI2C_INC)
    #include "BARCODE_EZI2C.h"
#endif /* (BARCODE_SCB_MODE_EZI2C_INC) */

#if (BARCODE_SCB_MODE_SPI_INC || BARCODE_SCB_MODE_UART_INC)
    #include "BARCODE_SPI_UART.h"
#endif /* (BARCODE_SCB_MODE_SPI_INC || BARCODE_SCB_MODE_UART_INC) */


/***************************************
*  Conditional Compilation Parameters
****************************************/

/* Bootloader communication interface enable */
#define BARCODE_BTLDR_COMM_ENABLED ((CYDEV_BOOTLOADER_IO_COMP == CyBtldr_BARCODE) || \
                                             (CYDEV_BOOTLOADER_IO_COMP == CyBtldr_Custom_Interface))

/* Enable I2C bootloader communication */
#if (BARCODE_SCB_MODE_I2C_INC)
    #define BARCODE_I2C_BTLDR_COMM_ENABLED     (BARCODE_BTLDR_COMM_ENABLED && \
                                                            (BARCODE_SCB_MODE_UNCONFIG_CONST_CFG || \
                                                             BARCODE_I2C_SLAVE_CONST))
#else
     #define BARCODE_I2C_BTLDR_COMM_ENABLED    (0u)
#endif /* (BARCODE_SCB_MODE_I2C_INC) */

/* EZI2C does not support bootloader communication. Provide empty APIs */
#if (BARCODE_SCB_MODE_EZI2C_INC)
    #define BARCODE_EZI2C_BTLDR_COMM_ENABLED   (BARCODE_BTLDR_COMM_ENABLED && \
                                                         BARCODE_SCB_MODE_UNCONFIG_CONST_CFG)
#else
    #define BARCODE_EZI2C_BTLDR_COMM_ENABLED   (0u)
#endif /* (BARCODE_EZI2C_BTLDR_COMM_ENABLED) */

/* Enable SPI bootloader communication */
#if (BARCODE_SCB_MODE_SPI_INC)
    #define BARCODE_SPI_BTLDR_COMM_ENABLED     (BARCODE_BTLDR_COMM_ENABLED && \
                                                            (BARCODE_SCB_MODE_UNCONFIG_CONST_CFG || \
                                                             BARCODE_SPI_SLAVE_CONST))
#else
        #define BARCODE_SPI_BTLDR_COMM_ENABLED (0u)
#endif /* (BARCODE_SPI_BTLDR_COMM_ENABLED) */

/* Enable UART bootloader communication */
#if (BARCODE_SCB_MODE_UART_INC)
       #define BARCODE_UART_BTLDR_COMM_ENABLED    (BARCODE_BTLDR_COMM_ENABLED && \
                                                            (BARCODE_SCB_MODE_UNCONFIG_CONST_CFG || \
                                                             (BARCODE_UART_RX_DIRECTION && \
                                                              BARCODE_UART_TX_DIRECTION)))
#else
     #define BARCODE_UART_BTLDR_COMM_ENABLED   (0u)
#endif /* (BARCODE_UART_BTLDR_COMM_ENABLED) */

/* Enable bootloader communication */
#define BARCODE_BTLDR_COMM_MODE_ENABLED    (BARCODE_I2C_BTLDR_COMM_ENABLED   || \
                                                     BARCODE_SPI_BTLDR_COMM_ENABLED   || \
                                                     BARCODE_EZI2C_BTLDR_COMM_ENABLED || \
                                                     BARCODE_UART_BTLDR_COMM_ENABLED)


/***************************************
*        Function Prototypes
***************************************/

#if defined(CYDEV_BOOTLOADER_IO_COMP) && (BARCODE_I2C_BTLDR_COMM_ENABLED)
    /* I2C Bootloader physical layer functions */
    void BARCODE_I2CCyBtldrCommStart(void);
    void BARCODE_I2CCyBtldrCommStop (void);
    void BARCODE_I2CCyBtldrCommReset(void);
    cystatus BARCODE_I2CCyBtldrCommRead       (uint8 pData[], uint16 size, uint16 * count, uint8 timeOut);
    cystatus BARCODE_I2CCyBtldrCommWrite(const uint8 pData[], uint16 size, uint16 * count, uint8 timeOut);

    /* Map I2C specific bootloader communication APIs to SCB specific APIs */
    #if (BARCODE_SCB_MODE_I2C_CONST_CFG)
        #define BARCODE_CyBtldrCommStart   BARCODE_I2CCyBtldrCommStart
        #define BARCODE_CyBtldrCommStop    BARCODE_I2CCyBtldrCommStop
        #define BARCODE_CyBtldrCommReset   BARCODE_I2CCyBtldrCommReset
        #define BARCODE_CyBtldrCommRead    BARCODE_I2CCyBtldrCommRead
        #define BARCODE_CyBtldrCommWrite   BARCODE_I2CCyBtldrCommWrite
    #endif /* (BARCODE_SCB_MODE_I2C_CONST_CFG) */

#endif /* defined(CYDEV_BOOTLOADER_IO_COMP) && (BARCODE_I2C_BTLDR_COMM_ENABLED) */


#if defined(CYDEV_BOOTLOADER_IO_COMP) && (BARCODE_EZI2C_BTLDR_COMM_ENABLED)
    /* Bootloader physical layer functions */
    void BARCODE_EzI2CCyBtldrCommStart(void);
    void BARCODE_EzI2CCyBtldrCommStop (void);
    void BARCODE_EzI2CCyBtldrCommReset(void);
    cystatus BARCODE_EzI2CCyBtldrCommRead       (uint8 pData[], uint16 size, uint16 * count, uint8 timeOut);
    cystatus BARCODE_EzI2CCyBtldrCommWrite(const uint8 pData[], uint16 size, uint16 * count, uint8 timeOut);

    /* Map EZI2C specific bootloader communication APIs to SCB specific APIs */
    #if (BARCODE_SCB_MODE_EZI2C_CONST_CFG)
        #define BARCODE_CyBtldrCommStart   BARCODE_EzI2CCyBtldrCommStart
        #define BARCODE_CyBtldrCommStop    BARCODE_EzI2CCyBtldrCommStop
        #define BARCODE_CyBtldrCommReset   BARCODE_EzI2CCyBtldrCommReset
        #define BARCODE_CyBtldrCommRead    BARCODE_EzI2CCyBtldrCommRead
        #define BARCODE_CyBtldrCommWrite   BARCODE_EzI2CCyBtldrCommWrite
    #endif /* (BARCODE_SCB_MODE_EZI2C_CONST_CFG) */

#endif /* defined(CYDEV_BOOTLOADER_IO_COMP) && (BARCODE_EZI2C_BTLDR_COMM_ENABLED) */

#if defined(CYDEV_BOOTLOADER_IO_COMP) && (BARCODE_SPI_BTLDR_COMM_ENABLED)
    /* SPI Bootloader physical layer functions */
    void BARCODE_SpiCyBtldrCommStart(void);
    void BARCODE_SpiCyBtldrCommStop (void);
    void BARCODE_SpiCyBtldrCommReset(void);
    cystatus BARCODE_SpiCyBtldrCommRead       (uint8 pData[], uint16 size, uint16 * count, uint8 timeOut);
    cystatus BARCODE_SpiCyBtldrCommWrite(const uint8 pData[], uint16 size, uint16 * count, uint8 timeOut);

    /* Map SPI specific bootloader communication APIs to SCB specific APIs */
    #if (BARCODE_SCB_MODE_SPI_CONST_CFG)
        #define BARCODE_CyBtldrCommStart   BARCODE_SpiCyBtldrCommStart
        #define BARCODE_CyBtldrCommStop    BARCODE_SpiCyBtldrCommStop
        #define BARCODE_CyBtldrCommReset   BARCODE_SpiCyBtldrCommReset
        #define BARCODE_CyBtldrCommRead    BARCODE_SpiCyBtldrCommRead
        #define BARCODE_CyBtldrCommWrite   BARCODE_SpiCyBtldrCommWrite
    #endif /* (BARCODE_SCB_MODE_SPI_CONST_CFG) */

#endif /* defined(CYDEV_BOOTLOADER_IO_COMP) && (BARCODE_SPI_BTLDR_COMM_ENABLED) */

#if defined(CYDEV_BOOTLOADER_IO_COMP) && (BARCODE_UART_BTLDR_COMM_ENABLED)
    /* UART Bootloader physical layer functions */
    void BARCODE_UartCyBtldrCommStart(void);
    void BARCODE_UartCyBtldrCommStop (void);
    void BARCODE_UartCyBtldrCommReset(void);
    cystatus BARCODE_UartCyBtldrCommRead       (uint8 pData[], uint16 size, uint16 * count, uint8 timeOut);
    cystatus BARCODE_UartCyBtldrCommWrite(const uint8 pData[], uint16 size, uint16 * count, uint8 timeOut);

    /* Map UART specific bootloader communication APIs to SCB specific APIs */
    #if (BARCODE_SCB_MODE_UART_CONST_CFG)
        #define BARCODE_CyBtldrCommStart   BARCODE_UartCyBtldrCommStart
        #define BARCODE_CyBtldrCommStop    BARCODE_UartCyBtldrCommStop
        #define BARCODE_CyBtldrCommReset   BARCODE_UartCyBtldrCommReset
        #define BARCODE_CyBtldrCommRead    BARCODE_UartCyBtldrCommRead
        #define BARCODE_CyBtldrCommWrite   BARCODE_UartCyBtldrCommWrite
    #endif /* (BARCODE_SCB_MODE_UART_CONST_CFG) */

#endif /* defined(CYDEV_BOOTLOADER_IO_COMP) && (BARCODE_UART_BTLDR_COMM_ENABLED) */

/**
* \addtogroup group_bootloader
* @{
*/

#if defined(CYDEV_BOOTLOADER_IO_COMP) && (BARCODE_BTLDR_COMM_ENABLED)
    #if (BARCODE_SCB_MODE_UNCONFIG_CONST_CFG)
        /* Bootloader physical layer functions */
        void BARCODE_CyBtldrCommStart(void);
        void BARCODE_CyBtldrCommStop (void);
        void BARCODE_CyBtldrCommReset(void);
        cystatus BARCODE_CyBtldrCommRead       (uint8 pData[], uint16 size, uint16 * count, uint8 timeOut);
        cystatus BARCODE_CyBtldrCommWrite(const uint8 pData[], uint16 size, uint16 * count, uint8 timeOut);
    #endif /* (BARCODE_SCB_MODE_UNCONFIG_CONST_CFG) */

    /* Map SCB specific bootloader communication APIs to common APIs */
    #if (CYDEV_BOOTLOADER_IO_COMP == CyBtldr_BARCODE)
        #define CyBtldrCommStart    BARCODE_CyBtldrCommStart
        #define CyBtldrCommStop     BARCODE_CyBtldrCommStop
        #define CyBtldrCommReset    BARCODE_CyBtldrCommReset
        #define CyBtldrCommWrite    BARCODE_CyBtldrCommWrite
        #define CyBtldrCommRead     BARCODE_CyBtldrCommRead
    #endif /* (CYDEV_BOOTLOADER_IO_COMP == CyBtldr_BARCODE) */

#endif /* defined(CYDEV_BOOTLOADER_IO_COMP) && (BARCODE_BTLDR_COMM_ENABLED) */

/** @} group_bootloader */

/***************************************
*           API Constants
***************************************/

/* Timeout unit in milliseconds */
#define BARCODE_WAIT_1_MS  (1u)

/* Return number of bytes to copy into bootloader buffer */
#define BARCODE_BYTES_TO_COPY(actBufSize, bufSize) \
                            ( ((uint32)(actBufSize) < (uint32)(bufSize)) ? \
                                ((uint32) (actBufSize)) : ((uint32) (bufSize)) )

/* Size of Read/Write buffers for I2C bootloader  */
#define BARCODE_I2C_BTLDR_SIZEOF_READ_BUFFER   (64u)
#define BARCODE_I2C_BTLDR_SIZEOF_WRITE_BUFFER  (64u)

/* Byte to byte time interval: calculated basing on current component
* data rate configuration, can be defined in project if required.
*/
#ifndef BARCODE_SPI_BYTE_TO_BYTE
    #define BARCODE_SPI_BYTE_TO_BYTE   (160u)
#endif

/* Byte to byte time interval: calculated basing on current component
* baud rate configuration, can be defined in the project if required.
*/
#ifndef BARCODE_UART_BYTE_TO_BYTE
    #define BARCODE_UART_BYTE_TO_BYTE  (2086u)
#endif /* BARCODE_UART_BYTE_TO_BYTE */

#endif /* (CY_SCB_BOOT_BARCODE_H) */


/* [] END OF FILE */
