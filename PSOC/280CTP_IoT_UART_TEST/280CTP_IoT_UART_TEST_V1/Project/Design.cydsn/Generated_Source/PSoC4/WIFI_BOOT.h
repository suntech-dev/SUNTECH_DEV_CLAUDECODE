/***************************************************************************//**
* \file WIFI_BOOT.h
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

#if !defined(CY_SCB_BOOT_WIFI_H)
#define CY_SCB_BOOT_WIFI_H

#include "WIFI_PVT.h"

#if (WIFI_SCB_MODE_I2C_INC)
    #include "WIFI_I2C.h"
#endif /* (WIFI_SCB_MODE_I2C_INC) */

#if (WIFI_SCB_MODE_EZI2C_INC)
    #include "WIFI_EZI2C.h"
#endif /* (WIFI_SCB_MODE_EZI2C_INC) */

#if (WIFI_SCB_MODE_SPI_INC || WIFI_SCB_MODE_UART_INC)
    #include "WIFI_SPI_UART.h"
#endif /* (WIFI_SCB_MODE_SPI_INC || WIFI_SCB_MODE_UART_INC) */


/***************************************
*  Conditional Compilation Parameters
****************************************/

/* Bootloader communication interface enable */
#define WIFI_BTLDR_COMM_ENABLED ((CYDEV_BOOTLOADER_IO_COMP == CyBtldr_WIFI) || \
                                             (CYDEV_BOOTLOADER_IO_COMP == CyBtldr_Custom_Interface))

/* Enable I2C bootloader communication */
#if (WIFI_SCB_MODE_I2C_INC)
    #define WIFI_I2C_BTLDR_COMM_ENABLED     (WIFI_BTLDR_COMM_ENABLED && \
                                                            (WIFI_SCB_MODE_UNCONFIG_CONST_CFG || \
                                                             WIFI_I2C_SLAVE_CONST))
#else
     #define WIFI_I2C_BTLDR_COMM_ENABLED    (0u)
#endif /* (WIFI_SCB_MODE_I2C_INC) */

/* EZI2C does not support bootloader communication. Provide empty APIs */
#if (WIFI_SCB_MODE_EZI2C_INC)
    #define WIFI_EZI2C_BTLDR_COMM_ENABLED   (WIFI_BTLDR_COMM_ENABLED && \
                                                         WIFI_SCB_MODE_UNCONFIG_CONST_CFG)
#else
    #define WIFI_EZI2C_BTLDR_COMM_ENABLED   (0u)
#endif /* (WIFI_EZI2C_BTLDR_COMM_ENABLED) */

/* Enable SPI bootloader communication */
#if (WIFI_SCB_MODE_SPI_INC)
    #define WIFI_SPI_BTLDR_COMM_ENABLED     (WIFI_BTLDR_COMM_ENABLED && \
                                                            (WIFI_SCB_MODE_UNCONFIG_CONST_CFG || \
                                                             WIFI_SPI_SLAVE_CONST))
#else
        #define WIFI_SPI_BTLDR_COMM_ENABLED (0u)
#endif /* (WIFI_SPI_BTLDR_COMM_ENABLED) */

/* Enable UART bootloader communication */
#if (WIFI_SCB_MODE_UART_INC)
       #define WIFI_UART_BTLDR_COMM_ENABLED    (WIFI_BTLDR_COMM_ENABLED && \
                                                            (WIFI_SCB_MODE_UNCONFIG_CONST_CFG || \
                                                             (WIFI_UART_RX_DIRECTION && \
                                                              WIFI_UART_TX_DIRECTION)))
#else
     #define WIFI_UART_BTLDR_COMM_ENABLED   (0u)
#endif /* (WIFI_UART_BTLDR_COMM_ENABLED) */

/* Enable bootloader communication */
#define WIFI_BTLDR_COMM_MODE_ENABLED    (WIFI_I2C_BTLDR_COMM_ENABLED   || \
                                                     WIFI_SPI_BTLDR_COMM_ENABLED   || \
                                                     WIFI_EZI2C_BTLDR_COMM_ENABLED || \
                                                     WIFI_UART_BTLDR_COMM_ENABLED)


/***************************************
*        Function Prototypes
***************************************/

#if defined(CYDEV_BOOTLOADER_IO_COMP) && (WIFI_I2C_BTLDR_COMM_ENABLED)
    /* I2C Bootloader physical layer functions */
    void WIFI_I2CCyBtldrCommStart(void);
    void WIFI_I2CCyBtldrCommStop (void);
    void WIFI_I2CCyBtldrCommReset(void);
    cystatus WIFI_I2CCyBtldrCommRead       (uint8 pData[], uint16 size, uint16 * count, uint8 timeOut);
    cystatus WIFI_I2CCyBtldrCommWrite(const uint8 pData[], uint16 size, uint16 * count, uint8 timeOut);

    /* Map I2C specific bootloader communication APIs to SCB specific APIs */
    #if (WIFI_SCB_MODE_I2C_CONST_CFG)
        #define WIFI_CyBtldrCommStart   WIFI_I2CCyBtldrCommStart
        #define WIFI_CyBtldrCommStop    WIFI_I2CCyBtldrCommStop
        #define WIFI_CyBtldrCommReset   WIFI_I2CCyBtldrCommReset
        #define WIFI_CyBtldrCommRead    WIFI_I2CCyBtldrCommRead
        #define WIFI_CyBtldrCommWrite   WIFI_I2CCyBtldrCommWrite
    #endif /* (WIFI_SCB_MODE_I2C_CONST_CFG) */

#endif /* defined(CYDEV_BOOTLOADER_IO_COMP) && (WIFI_I2C_BTLDR_COMM_ENABLED) */


#if defined(CYDEV_BOOTLOADER_IO_COMP) && (WIFI_EZI2C_BTLDR_COMM_ENABLED)
    /* Bootloader physical layer functions */
    void WIFI_EzI2CCyBtldrCommStart(void);
    void WIFI_EzI2CCyBtldrCommStop (void);
    void WIFI_EzI2CCyBtldrCommReset(void);
    cystatus WIFI_EzI2CCyBtldrCommRead       (uint8 pData[], uint16 size, uint16 * count, uint8 timeOut);
    cystatus WIFI_EzI2CCyBtldrCommWrite(const uint8 pData[], uint16 size, uint16 * count, uint8 timeOut);

    /* Map EZI2C specific bootloader communication APIs to SCB specific APIs */
    #if (WIFI_SCB_MODE_EZI2C_CONST_CFG)
        #define WIFI_CyBtldrCommStart   WIFI_EzI2CCyBtldrCommStart
        #define WIFI_CyBtldrCommStop    WIFI_EzI2CCyBtldrCommStop
        #define WIFI_CyBtldrCommReset   WIFI_EzI2CCyBtldrCommReset
        #define WIFI_CyBtldrCommRead    WIFI_EzI2CCyBtldrCommRead
        #define WIFI_CyBtldrCommWrite   WIFI_EzI2CCyBtldrCommWrite
    #endif /* (WIFI_SCB_MODE_EZI2C_CONST_CFG) */

#endif /* defined(CYDEV_BOOTLOADER_IO_COMP) && (WIFI_EZI2C_BTLDR_COMM_ENABLED) */

#if defined(CYDEV_BOOTLOADER_IO_COMP) && (WIFI_SPI_BTLDR_COMM_ENABLED)
    /* SPI Bootloader physical layer functions */
    void WIFI_SpiCyBtldrCommStart(void);
    void WIFI_SpiCyBtldrCommStop (void);
    void WIFI_SpiCyBtldrCommReset(void);
    cystatus WIFI_SpiCyBtldrCommRead       (uint8 pData[], uint16 size, uint16 * count, uint8 timeOut);
    cystatus WIFI_SpiCyBtldrCommWrite(const uint8 pData[], uint16 size, uint16 * count, uint8 timeOut);

    /* Map SPI specific bootloader communication APIs to SCB specific APIs */
    #if (WIFI_SCB_MODE_SPI_CONST_CFG)
        #define WIFI_CyBtldrCommStart   WIFI_SpiCyBtldrCommStart
        #define WIFI_CyBtldrCommStop    WIFI_SpiCyBtldrCommStop
        #define WIFI_CyBtldrCommReset   WIFI_SpiCyBtldrCommReset
        #define WIFI_CyBtldrCommRead    WIFI_SpiCyBtldrCommRead
        #define WIFI_CyBtldrCommWrite   WIFI_SpiCyBtldrCommWrite
    #endif /* (WIFI_SCB_MODE_SPI_CONST_CFG) */

#endif /* defined(CYDEV_BOOTLOADER_IO_COMP) && (WIFI_SPI_BTLDR_COMM_ENABLED) */

#if defined(CYDEV_BOOTLOADER_IO_COMP) && (WIFI_UART_BTLDR_COMM_ENABLED)
    /* UART Bootloader physical layer functions */
    void WIFI_UartCyBtldrCommStart(void);
    void WIFI_UartCyBtldrCommStop (void);
    void WIFI_UartCyBtldrCommReset(void);
    cystatus WIFI_UartCyBtldrCommRead       (uint8 pData[], uint16 size, uint16 * count, uint8 timeOut);
    cystatus WIFI_UartCyBtldrCommWrite(const uint8 pData[], uint16 size, uint16 * count, uint8 timeOut);

    /* Map UART specific bootloader communication APIs to SCB specific APIs */
    #if (WIFI_SCB_MODE_UART_CONST_CFG)
        #define WIFI_CyBtldrCommStart   WIFI_UartCyBtldrCommStart
        #define WIFI_CyBtldrCommStop    WIFI_UartCyBtldrCommStop
        #define WIFI_CyBtldrCommReset   WIFI_UartCyBtldrCommReset
        #define WIFI_CyBtldrCommRead    WIFI_UartCyBtldrCommRead
        #define WIFI_CyBtldrCommWrite   WIFI_UartCyBtldrCommWrite
    #endif /* (WIFI_SCB_MODE_UART_CONST_CFG) */

#endif /* defined(CYDEV_BOOTLOADER_IO_COMP) && (WIFI_UART_BTLDR_COMM_ENABLED) */

/**
* \addtogroup group_bootloader
* @{
*/

#if defined(CYDEV_BOOTLOADER_IO_COMP) && (WIFI_BTLDR_COMM_ENABLED)
    #if (WIFI_SCB_MODE_UNCONFIG_CONST_CFG)
        /* Bootloader physical layer functions */
        void WIFI_CyBtldrCommStart(void);
        void WIFI_CyBtldrCommStop (void);
        void WIFI_CyBtldrCommReset(void);
        cystatus WIFI_CyBtldrCommRead       (uint8 pData[], uint16 size, uint16 * count, uint8 timeOut);
        cystatus WIFI_CyBtldrCommWrite(const uint8 pData[], uint16 size, uint16 * count, uint8 timeOut);
    #endif /* (WIFI_SCB_MODE_UNCONFIG_CONST_CFG) */

    /* Map SCB specific bootloader communication APIs to common APIs */
    #if (CYDEV_BOOTLOADER_IO_COMP == CyBtldr_WIFI)
        #define CyBtldrCommStart    WIFI_CyBtldrCommStart
        #define CyBtldrCommStop     WIFI_CyBtldrCommStop
        #define CyBtldrCommReset    WIFI_CyBtldrCommReset
        #define CyBtldrCommWrite    WIFI_CyBtldrCommWrite
        #define CyBtldrCommRead     WIFI_CyBtldrCommRead
    #endif /* (CYDEV_BOOTLOADER_IO_COMP == CyBtldr_WIFI) */

#endif /* defined(CYDEV_BOOTLOADER_IO_COMP) && (WIFI_BTLDR_COMM_ENABLED) */

/** @} group_bootloader */

/***************************************
*           API Constants
***************************************/

/* Timeout unit in milliseconds */
#define WIFI_WAIT_1_MS  (1u)

/* Return number of bytes to copy into bootloader buffer */
#define WIFI_BYTES_TO_COPY(actBufSize, bufSize) \
                            ( ((uint32)(actBufSize) < (uint32)(bufSize)) ? \
                                ((uint32) (actBufSize)) : ((uint32) (bufSize)) )

/* Size of Read/Write buffers for I2C bootloader  */
#define WIFI_I2C_BTLDR_SIZEOF_READ_BUFFER   (64u)
#define WIFI_I2C_BTLDR_SIZEOF_WRITE_BUFFER  (64u)

/* Byte to byte time interval: calculated basing on current component
* data rate configuration, can be defined in project if required.
*/
#ifndef WIFI_SPI_BYTE_TO_BYTE
    #define WIFI_SPI_BYTE_TO_BYTE   (160u)
#endif

/* Byte to byte time interval: calculated basing on current component
* baud rate configuration, can be defined in the project if required.
*/
#ifndef WIFI_UART_BYTE_TO_BYTE
    #define WIFI_UART_BYTE_TO_BYTE  (175u)
#endif /* WIFI_UART_BYTE_TO_BYTE */

#endif /* (CY_SCB_BOOT_WIFI_H) */


/* [] END OF FILE */
