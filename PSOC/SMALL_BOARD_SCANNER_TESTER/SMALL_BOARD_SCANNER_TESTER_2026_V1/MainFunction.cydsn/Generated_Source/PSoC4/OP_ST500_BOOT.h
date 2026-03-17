/***************************************************************************//**
* \file OP_ST500_BOOT.h
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

#if !defined(CY_SCB_BOOT_OP_ST500_H)
#define CY_SCB_BOOT_OP_ST500_H

#include "OP_ST500_PVT.h"

#if (OP_ST500_SCB_MODE_I2C_INC)
    #include "OP_ST500_I2C.h"
#endif /* (OP_ST500_SCB_MODE_I2C_INC) */

#if (OP_ST500_SCB_MODE_EZI2C_INC)
    #include "OP_ST500_EZI2C.h"
#endif /* (OP_ST500_SCB_MODE_EZI2C_INC) */

#if (OP_ST500_SCB_MODE_SPI_INC || OP_ST500_SCB_MODE_UART_INC)
    #include "OP_ST500_SPI_UART.h"
#endif /* (OP_ST500_SCB_MODE_SPI_INC || OP_ST500_SCB_MODE_UART_INC) */


/***************************************
*  Conditional Compilation Parameters
****************************************/

/* Bootloader communication interface enable */
#define OP_ST500_BTLDR_COMM_ENABLED ((CYDEV_BOOTLOADER_IO_COMP == CyBtldr_OP_ST500) || \
                                             (CYDEV_BOOTLOADER_IO_COMP == CyBtldr_Custom_Interface))

/* Enable I2C bootloader communication */
#if (OP_ST500_SCB_MODE_I2C_INC)
    #define OP_ST500_I2C_BTLDR_COMM_ENABLED     (OP_ST500_BTLDR_COMM_ENABLED && \
                                                            (OP_ST500_SCB_MODE_UNCONFIG_CONST_CFG || \
                                                             OP_ST500_I2C_SLAVE_CONST))
#else
     #define OP_ST500_I2C_BTLDR_COMM_ENABLED    (0u)
#endif /* (OP_ST500_SCB_MODE_I2C_INC) */

/* EZI2C does not support bootloader communication. Provide empty APIs */
#if (OP_ST500_SCB_MODE_EZI2C_INC)
    #define OP_ST500_EZI2C_BTLDR_COMM_ENABLED   (OP_ST500_BTLDR_COMM_ENABLED && \
                                                         OP_ST500_SCB_MODE_UNCONFIG_CONST_CFG)
#else
    #define OP_ST500_EZI2C_BTLDR_COMM_ENABLED   (0u)
#endif /* (OP_ST500_EZI2C_BTLDR_COMM_ENABLED) */

/* Enable SPI bootloader communication */
#if (OP_ST500_SCB_MODE_SPI_INC)
    #define OP_ST500_SPI_BTLDR_COMM_ENABLED     (OP_ST500_BTLDR_COMM_ENABLED && \
                                                            (OP_ST500_SCB_MODE_UNCONFIG_CONST_CFG || \
                                                             OP_ST500_SPI_SLAVE_CONST))
#else
        #define OP_ST500_SPI_BTLDR_COMM_ENABLED (0u)
#endif /* (OP_ST500_SPI_BTLDR_COMM_ENABLED) */

/* Enable UART bootloader communication */
#if (OP_ST500_SCB_MODE_UART_INC)
       #define OP_ST500_UART_BTLDR_COMM_ENABLED    (OP_ST500_BTLDR_COMM_ENABLED && \
                                                            (OP_ST500_SCB_MODE_UNCONFIG_CONST_CFG || \
                                                             (OP_ST500_UART_RX_DIRECTION && \
                                                              OP_ST500_UART_TX_DIRECTION)))
#else
     #define OP_ST500_UART_BTLDR_COMM_ENABLED   (0u)
#endif /* (OP_ST500_UART_BTLDR_COMM_ENABLED) */

/* Enable bootloader communication */
#define OP_ST500_BTLDR_COMM_MODE_ENABLED    (OP_ST500_I2C_BTLDR_COMM_ENABLED   || \
                                                     OP_ST500_SPI_BTLDR_COMM_ENABLED   || \
                                                     OP_ST500_EZI2C_BTLDR_COMM_ENABLED || \
                                                     OP_ST500_UART_BTLDR_COMM_ENABLED)


/***************************************
*        Function Prototypes
***************************************/

#if defined(CYDEV_BOOTLOADER_IO_COMP) && (OP_ST500_I2C_BTLDR_COMM_ENABLED)
    /* I2C Bootloader physical layer functions */
    void OP_ST500_I2CCyBtldrCommStart(void);
    void OP_ST500_I2CCyBtldrCommStop (void);
    void OP_ST500_I2CCyBtldrCommReset(void);
    cystatus OP_ST500_I2CCyBtldrCommRead       (uint8 pData[], uint16 size, uint16 * count, uint8 timeOut);
    cystatus OP_ST500_I2CCyBtldrCommWrite(const uint8 pData[], uint16 size, uint16 * count, uint8 timeOut);

    /* Map I2C specific bootloader communication APIs to SCB specific APIs */
    #if (OP_ST500_SCB_MODE_I2C_CONST_CFG)
        #define OP_ST500_CyBtldrCommStart   OP_ST500_I2CCyBtldrCommStart
        #define OP_ST500_CyBtldrCommStop    OP_ST500_I2CCyBtldrCommStop
        #define OP_ST500_CyBtldrCommReset   OP_ST500_I2CCyBtldrCommReset
        #define OP_ST500_CyBtldrCommRead    OP_ST500_I2CCyBtldrCommRead
        #define OP_ST500_CyBtldrCommWrite   OP_ST500_I2CCyBtldrCommWrite
    #endif /* (OP_ST500_SCB_MODE_I2C_CONST_CFG) */

#endif /* defined(CYDEV_BOOTLOADER_IO_COMP) && (OP_ST500_I2C_BTLDR_COMM_ENABLED) */


#if defined(CYDEV_BOOTLOADER_IO_COMP) && (OP_ST500_EZI2C_BTLDR_COMM_ENABLED)
    /* Bootloader physical layer functions */
    void OP_ST500_EzI2CCyBtldrCommStart(void);
    void OP_ST500_EzI2CCyBtldrCommStop (void);
    void OP_ST500_EzI2CCyBtldrCommReset(void);
    cystatus OP_ST500_EzI2CCyBtldrCommRead       (uint8 pData[], uint16 size, uint16 * count, uint8 timeOut);
    cystatus OP_ST500_EzI2CCyBtldrCommWrite(const uint8 pData[], uint16 size, uint16 * count, uint8 timeOut);

    /* Map EZI2C specific bootloader communication APIs to SCB specific APIs */
    #if (OP_ST500_SCB_MODE_EZI2C_CONST_CFG)
        #define OP_ST500_CyBtldrCommStart   OP_ST500_EzI2CCyBtldrCommStart
        #define OP_ST500_CyBtldrCommStop    OP_ST500_EzI2CCyBtldrCommStop
        #define OP_ST500_CyBtldrCommReset   OP_ST500_EzI2CCyBtldrCommReset
        #define OP_ST500_CyBtldrCommRead    OP_ST500_EzI2CCyBtldrCommRead
        #define OP_ST500_CyBtldrCommWrite   OP_ST500_EzI2CCyBtldrCommWrite
    #endif /* (OP_ST500_SCB_MODE_EZI2C_CONST_CFG) */

#endif /* defined(CYDEV_BOOTLOADER_IO_COMP) && (OP_ST500_EZI2C_BTLDR_COMM_ENABLED) */

#if defined(CYDEV_BOOTLOADER_IO_COMP) && (OP_ST500_SPI_BTLDR_COMM_ENABLED)
    /* SPI Bootloader physical layer functions */
    void OP_ST500_SpiCyBtldrCommStart(void);
    void OP_ST500_SpiCyBtldrCommStop (void);
    void OP_ST500_SpiCyBtldrCommReset(void);
    cystatus OP_ST500_SpiCyBtldrCommRead       (uint8 pData[], uint16 size, uint16 * count, uint8 timeOut);
    cystatus OP_ST500_SpiCyBtldrCommWrite(const uint8 pData[], uint16 size, uint16 * count, uint8 timeOut);

    /* Map SPI specific bootloader communication APIs to SCB specific APIs */
    #if (OP_ST500_SCB_MODE_SPI_CONST_CFG)
        #define OP_ST500_CyBtldrCommStart   OP_ST500_SpiCyBtldrCommStart
        #define OP_ST500_CyBtldrCommStop    OP_ST500_SpiCyBtldrCommStop
        #define OP_ST500_CyBtldrCommReset   OP_ST500_SpiCyBtldrCommReset
        #define OP_ST500_CyBtldrCommRead    OP_ST500_SpiCyBtldrCommRead
        #define OP_ST500_CyBtldrCommWrite   OP_ST500_SpiCyBtldrCommWrite
    #endif /* (OP_ST500_SCB_MODE_SPI_CONST_CFG) */

#endif /* defined(CYDEV_BOOTLOADER_IO_COMP) && (OP_ST500_SPI_BTLDR_COMM_ENABLED) */

#if defined(CYDEV_BOOTLOADER_IO_COMP) && (OP_ST500_UART_BTLDR_COMM_ENABLED)
    /* UART Bootloader physical layer functions */
    void OP_ST500_UartCyBtldrCommStart(void);
    void OP_ST500_UartCyBtldrCommStop (void);
    void OP_ST500_UartCyBtldrCommReset(void);
    cystatus OP_ST500_UartCyBtldrCommRead       (uint8 pData[], uint16 size, uint16 * count, uint8 timeOut);
    cystatus OP_ST500_UartCyBtldrCommWrite(const uint8 pData[], uint16 size, uint16 * count, uint8 timeOut);

    /* Map UART specific bootloader communication APIs to SCB specific APIs */
    #if (OP_ST500_SCB_MODE_UART_CONST_CFG)
        #define OP_ST500_CyBtldrCommStart   OP_ST500_UartCyBtldrCommStart
        #define OP_ST500_CyBtldrCommStop    OP_ST500_UartCyBtldrCommStop
        #define OP_ST500_CyBtldrCommReset   OP_ST500_UartCyBtldrCommReset
        #define OP_ST500_CyBtldrCommRead    OP_ST500_UartCyBtldrCommRead
        #define OP_ST500_CyBtldrCommWrite   OP_ST500_UartCyBtldrCommWrite
    #endif /* (OP_ST500_SCB_MODE_UART_CONST_CFG) */

#endif /* defined(CYDEV_BOOTLOADER_IO_COMP) && (OP_ST500_UART_BTLDR_COMM_ENABLED) */

/**
* \addtogroup group_bootloader
* @{
*/

#if defined(CYDEV_BOOTLOADER_IO_COMP) && (OP_ST500_BTLDR_COMM_ENABLED)
    #if (OP_ST500_SCB_MODE_UNCONFIG_CONST_CFG)
        /* Bootloader physical layer functions */
        void OP_ST500_CyBtldrCommStart(void);
        void OP_ST500_CyBtldrCommStop (void);
        void OP_ST500_CyBtldrCommReset(void);
        cystatus OP_ST500_CyBtldrCommRead       (uint8 pData[], uint16 size, uint16 * count, uint8 timeOut);
        cystatus OP_ST500_CyBtldrCommWrite(const uint8 pData[], uint16 size, uint16 * count, uint8 timeOut);
    #endif /* (OP_ST500_SCB_MODE_UNCONFIG_CONST_CFG) */

    /* Map SCB specific bootloader communication APIs to common APIs */
    #if (CYDEV_BOOTLOADER_IO_COMP == CyBtldr_OP_ST500)
        #define CyBtldrCommStart    OP_ST500_CyBtldrCommStart
        #define CyBtldrCommStop     OP_ST500_CyBtldrCommStop
        #define CyBtldrCommReset    OP_ST500_CyBtldrCommReset
        #define CyBtldrCommWrite    OP_ST500_CyBtldrCommWrite
        #define CyBtldrCommRead     OP_ST500_CyBtldrCommRead
    #endif /* (CYDEV_BOOTLOADER_IO_COMP == CyBtldr_OP_ST500) */

#endif /* defined(CYDEV_BOOTLOADER_IO_COMP) && (OP_ST500_BTLDR_COMM_ENABLED) */

/** @} group_bootloader */

/***************************************
*           API Constants
***************************************/

/* Timeout unit in milliseconds */
#define OP_ST500_WAIT_1_MS  (1u)

/* Return number of bytes to copy into bootloader buffer */
#define OP_ST500_BYTES_TO_COPY(actBufSize, bufSize) \
                            ( ((uint32)(actBufSize) < (uint32)(bufSize)) ? \
                                ((uint32) (actBufSize)) : ((uint32) (bufSize)) )

/* Size of Read/Write buffers for I2C bootloader  */
#define OP_ST500_I2C_BTLDR_SIZEOF_READ_BUFFER   (64u)
#define OP_ST500_I2C_BTLDR_SIZEOF_WRITE_BUFFER  (64u)

/* Byte to byte time interval: calculated basing on current component
* data rate configuration, can be defined in project if required.
*/
#ifndef OP_ST500_SPI_BYTE_TO_BYTE
    #define OP_ST500_SPI_BYTE_TO_BYTE   (160u)
#endif

/* Byte to byte time interval: calculated basing on current component
* baud rate configuration, can be defined in the project if required.
*/
#ifndef OP_ST500_UART_BYTE_TO_BYTE
    #define OP_ST500_UART_BYTE_TO_BYTE  (2086u)
#endif /* OP_ST500_UART_BYTE_TO_BYTE */

#endif /* (CY_SCB_BOOT_OP_ST500_H) */


/* [] END OF FILE */
