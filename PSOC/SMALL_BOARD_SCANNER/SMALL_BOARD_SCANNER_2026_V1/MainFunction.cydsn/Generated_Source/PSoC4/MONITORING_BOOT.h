/***************************************************************************//**
* \file MONITORING_BOOT.h
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

#if !defined(CY_SCB_BOOT_MONITORING_H)
#define CY_SCB_BOOT_MONITORING_H

#include "MONITORING_PVT.h"

#if (MONITORING_SCB_MODE_I2C_INC)
    #include "MONITORING_I2C.h"
#endif /* (MONITORING_SCB_MODE_I2C_INC) */

#if (MONITORING_SCB_MODE_EZI2C_INC)
    #include "MONITORING_EZI2C.h"
#endif /* (MONITORING_SCB_MODE_EZI2C_INC) */

#if (MONITORING_SCB_MODE_SPI_INC || MONITORING_SCB_MODE_UART_INC)
    #include "MONITORING_SPI_UART.h"
#endif /* (MONITORING_SCB_MODE_SPI_INC || MONITORING_SCB_MODE_UART_INC) */


/***************************************
*  Conditional Compilation Parameters
****************************************/

/* Bootloader communication interface enable */
#define MONITORING_BTLDR_COMM_ENABLED ((CYDEV_BOOTLOADER_IO_COMP == CyBtldr_MONITORING) || \
                                             (CYDEV_BOOTLOADER_IO_COMP == CyBtldr_Custom_Interface))

/* Enable I2C bootloader communication */
#if (MONITORING_SCB_MODE_I2C_INC)
    #define MONITORING_I2C_BTLDR_COMM_ENABLED     (MONITORING_BTLDR_COMM_ENABLED && \
                                                            (MONITORING_SCB_MODE_UNCONFIG_CONST_CFG || \
                                                             MONITORING_I2C_SLAVE_CONST))
#else
     #define MONITORING_I2C_BTLDR_COMM_ENABLED    (0u)
#endif /* (MONITORING_SCB_MODE_I2C_INC) */

/* EZI2C does not support bootloader communication. Provide empty APIs */
#if (MONITORING_SCB_MODE_EZI2C_INC)
    #define MONITORING_EZI2C_BTLDR_COMM_ENABLED   (MONITORING_BTLDR_COMM_ENABLED && \
                                                         MONITORING_SCB_MODE_UNCONFIG_CONST_CFG)
#else
    #define MONITORING_EZI2C_BTLDR_COMM_ENABLED   (0u)
#endif /* (MONITORING_EZI2C_BTLDR_COMM_ENABLED) */

/* Enable SPI bootloader communication */
#if (MONITORING_SCB_MODE_SPI_INC)
    #define MONITORING_SPI_BTLDR_COMM_ENABLED     (MONITORING_BTLDR_COMM_ENABLED && \
                                                            (MONITORING_SCB_MODE_UNCONFIG_CONST_CFG || \
                                                             MONITORING_SPI_SLAVE_CONST))
#else
        #define MONITORING_SPI_BTLDR_COMM_ENABLED (0u)
#endif /* (MONITORING_SPI_BTLDR_COMM_ENABLED) */

/* Enable UART bootloader communication */
#if (MONITORING_SCB_MODE_UART_INC)
       #define MONITORING_UART_BTLDR_COMM_ENABLED    (MONITORING_BTLDR_COMM_ENABLED && \
                                                            (MONITORING_SCB_MODE_UNCONFIG_CONST_CFG || \
                                                             (MONITORING_UART_RX_DIRECTION && \
                                                              MONITORING_UART_TX_DIRECTION)))
#else
     #define MONITORING_UART_BTLDR_COMM_ENABLED   (0u)
#endif /* (MONITORING_UART_BTLDR_COMM_ENABLED) */

/* Enable bootloader communication */
#define MONITORING_BTLDR_COMM_MODE_ENABLED    (MONITORING_I2C_BTLDR_COMM_ENABLED   || \
                                                     MONITORING_SPI_BTLDR_COMM_ENABLED   || \
                                                     MONITORING_EZI2C_BTLDR_COMM_ENABLED || \
                                                     MONITORING_UART_BTLDR_COMM_ENABLED)


/***************************************
*        Function Prototypes
***************************************/

#if defined(CYDEV_BOOTLOADER_IO_COMP) && (MONITORING_I2C_BTLDR_COMM_ENABLED)
    /* I2C Bootloader physical layer functions */
    void MONITORING_I2CCyBtldrCommStart(void);
    void MONITORING_I2CCyBtldrCommStop (void);
    void MONITORING_I2CCyBtldrCommReset(void);
    cystatus MONITORING_I2CCyBtldrCommRead       (uint8 pData[], uint16 size, uint16 * count, uint8 timeOut);
    cystatus MONITORING_I2CCyBtldrCommWrite(const uint8 pData[], uint16 size, uint16 * count, uint8 timeOut);

    /* Map I2C specific bootloader communication APIs to SCB specific APIs */
    #if (MONITORING_SCB_MODE_I2C_CONST_CFG)
        #define MONITORING_CyBtldrCommStart   MONITORING_I2CCyBtldrCommStart
        #define MONITORING_CyBtldrCommStop    MONITORING_I2CCyBtldrCommStop
        #define MONITORING_CyBtldrCommReset   MONITORING_I2CCyBtldrCommReset
        #define MONITORING_CyBtldrCommRead    MONITORING_I2CCyBtldrCommRead
        #define MONITORING_CyBtldrCommWrite   MONITORING_I2CCyBtldrCommWrite
    #endif /* (MONITORING_SCB_MODE_I2C_CONST_CFG) */

#endif /* defined(CYDEV_BOOTLOADER_IO_COMP) && (MONITORING_I2C_BTLDR_COMM_ENABLED) */


#if defined(CYDEV_BOOTLOADER_IO_COMP) && (MONITORING_EZI2C_BTLDR_COMM_ENABLED)
    /* Bootloader physical layer functions */
    void MONITORING_EzI2CCyBtldrCommStart(void);
    void MONITORING_EzI2CCyBtldrCommStop (void);
    void MONITORING_EzI2CCyBtldrCommReset(void);
    cystatus MONITORING_EzI2CCyBtldrCommRead       (uint8 pData[], uint16 size, uint16 * count, uint8 timeOut);
    cystatus MONITORING_EzI2CCyBtldrCommWrite(const uint8 pData[], uint16 size, uint16 * count, uint8 timeOut);

    /* Map EZI2C specific bootloader communication APIs to SCB specific APIs */
    #if (MONITORING_SCB_MODE_EZI2C_CONST_CFG)
        #define MONITORING_CyBtldrCommStart   MONITORING_EzI2CCyBtldrCommStart
        #define MONITORING_CyBtldrCommStop    MONITORING_EzI2CCyBtldrCommStop
        #define MONITORING_CyBtldrCommReset   MONITORING_EzI2CCyBtldrCommReset
        #define MONITORING_CyBtldrCommRead    MONITORING_EzI2CCyBtldrCommRead
        #define MONITORING_CyBtldrCommWrite   MONITORING_EzI2CCyBtldrCommWrite
    #endif /* (MONITORING_SCB_MODE_EZI2C_CONST_CFG) */

#endif /* defined(CYDEV_BOOTLOADER_IO_COMP) && (MONITORING_EZI2C_BTLDR_COMM_ENABLED) */

#if defined(CYDEV_BOOTLOADER_IO_COMP) && (MONITORING_SPI_BTLDR_COMM_ENABLED)
    /* SPI Bootloader physical layer functions */
    void MONITORING_SpiCyBtldrCommStart(void);
    void MONITORING_SpiCyBtldrCommStop (void);
    void MONITORING_SpiCyBtldrCommReset(void);
    cystatus MONITORING_SpiCyBtldrCommRead       (uint8 pData[], uint16 size, uint16 * count, uint8 timeOut);
    cystatus MONITORING_SpiCyBtldrCommWrite(const uint8 pData[], uint16 size, uint16 * count, uint8 timeOut);

    /* Map SPI specific bootloader communication APIs to SCB specific APIs */
    #if (MONITORING_SCB_MODE_SPI_CONST_CFG)
        #define MONITORING_CyBtldrCommStart   MONITORING_SpiCyBtldrCommStart
        #define MONITORING_CyBtldrCommStop    MONITORING_SpiCyBtldrCommStop
        #define MONITORING_CyBtldrCommReset   MONITORING_SpiCyBtldrCommReset
        #define MONITORING_CyBtldrCommRead    MONITORING_SpiCyBtldrCommRead
        #define MONITORING_CyBtldrCommWrite   MONITORING_SpiCyBtldrCommWrite
    #endif /* (MONITORING_SCB_MODE_SPI_CONST_CFG) */

#endif /* defined(CYDEV_BOOTLOADER_IO_COMP) && (MONITORING_SPI_BTLDR_COMM_ENABLED) */

#if defined(CYDEV_BOOTLOADER_IO_COMP) && (MONITORING_UART_BTLDR_COMM_ENABLED)
    /* UART Bootloader physical layer functions */
    void MONITORING_UartCyBtldrCommStart(void);
    void MONITORING_UartCyBtldrCommStop (void);
    void MONITORING_UartCyBtldrCommReset(void);
    cystatus MONITORING_UartCyBtldrCommRead       (uint8 pData[], uint16 size, uint16 * count, uint8 timeOut);
    cystatus MONITORING_UartCyBtldrCommWrite(const uint8 pData[], uint16 size, uint16 * count, uint8 timeOut);

    /* Map UART specific bootloader communication APIs to SCB specific APIs */
    #if (MONITORING_SCB_MODE_UART_CONST_CFG)
        #define MONITORING_CyBtldrCommStart   MONITORING_UartCyBtldrCommStart
        #define MONITORING_CyBtldrCommStop    MONITORING_UartCyBtldrCommStop
        #define MONITORING_CyBtldrCommReset   MONITORING_UartCyBtldrCommReset
        #define MONITORING_CyBtldrCommRead    MONITORING_UartCyBtldrCommRead
        #define MONITORING_CyBtldrCommWrite   MONITORING_UartCyBtldrCommWrite
    #endif /* (MONITORING_SCB_MODE_UART_CONST_CFG) */

#endif /* defined(CYDEV_BOOTLOADER_IO_COMP) && (MONITORING_UART_BTLDR_COMM_ENABLED) */

/**
* \addtogroup group_bootloader
* @{
*/

#if defined(CYDEV_BOOTLOADER_IO_COMP) && (MONITORING_BTLDR_COMM_ENABLED)
    #if (MONITORING_SCB_MODE_UNCONFIG_CONST_CFG)
        /* Bootloader physical layer functions */
        void MONITORING_CyBtldrCommStart(void);
        void MONITORING_CyBtldrCommStop (void);
        void MONITORING_CyBtldrCommReset(void);
        cystatus MONITORING_CyBtldrCommRead       (uint8 pData[], uint16 size, uint16 * count, uint8 timeOut);
        cystatus MONITORING_CyBtldrCommWrite(const uint8 pData[], uint16 size, uint16 * count, uint8 timeOut);
    #endif /* (MONITORING_SCB_MODE_UNCONFIG_CONST_CFG) */

    /* Map SCB specific bootloader communication APIs to common APIs */
    #if (CYDEV_BOOTLOADER_IO_COMP == CyBtldr_MONITORING)
        #define CyBtldrCommStart    MONITORING_CyBtldrCommStart
        #define CyBtldrCommStop     MONITORING_CyBtldrCommStop
        #define CyBtldrCommReset    MONITORING_CyBtldrCommReset
        #define CyBtldrCommWrite    MONITORING_CyBtldrCommWrite
        #define CyBtldrCommRead     MONITORING_CyBtldrCommRead
    #endif /* (CYDEV_BOOTLOADER_IO_COMP == CyBtldr_MONITORING) */

#endif /* defined(CYDEV_BOOTLOADER_IO_COMP) && (MONITORING_BTLDR_COMM_ENABLED) */

/** @} group_bootloader */

/***************************************
*           API Constants
***************************************/

/* Timeout unit in milliseconds */
#define MONITORING_WAIT_1_MS  (1u)

/* Return number of bytes to copy into bootloader buffer */
#define MONITORING_BYTES_TO_COPY(actBufSize, bufSize) \
                            ( ((uint32)(actBufSize) < (uint32)(bufSize)) ? \
                                ((uint32) (actBufSize)) : ((uint32) (bufSize)) )

/* Size of Read/Write buffers for I2C bootloader  */
#define MONITORING_I2C_BTLDR_SIZEOF_READ_BUFFER   (64u)
#define MONITORING_I2C_BTLDR_SIZEOF_WRITE_BUFFER  (64u)

/* Byte to byte time interval: calculated basing on current component
* data rate configuration, can be defined in project if required.
*/
#ifndef MONITORING_SPI_BYTE_TO_BYTE
    #define MONITORING_SPI_BYTE_TO_BYTE   (160u)
#endif

/* Byte to byte time interval: calculated basing on current component
* baud rate configuration, can be defined in the project if required.
*/
#ifndef MONITORING_UART_BYTE_TO_BYTE
    #define MONITORING_UART_BYTE_TO_BYTE  (2086u)
#endif /* MONITORING_UART_BYTE_TO_BYTE */

#endif /* (CY_SCB_BOOT_MONITORING_H) */


/* [] END OF FILE */
