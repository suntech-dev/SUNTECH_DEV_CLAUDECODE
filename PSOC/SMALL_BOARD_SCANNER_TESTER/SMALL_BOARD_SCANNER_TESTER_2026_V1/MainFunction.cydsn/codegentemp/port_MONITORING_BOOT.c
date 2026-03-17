/***************************************************************************//**
* \file port_MONITORING_BOOT.c
* \version 4.0
*
* \brief
*  This file provides the source code of the bootloader communication APIs
*  for the SCB Component Unconfigured mode.
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

#include "port_MONITORING_BOOT.h"

#if defined(CYDEV_BOOTLOADER_IO_COMP) && (port_MONITORING_BTLDR_COMM_ENABLED) && \
                                (port_MONITORING_SCB_MODE_UNCONFIG_CONST_CFG)

/*******************************************************************************
* Function Name: port_MONITORING_CyBtldrCommStart
****************************************************************************//**
*
*  Starts port_MONITORING component. After this function call the component is 
*  ready for communication.
*
*******************************************************************************/
void port_MONITORING_CyBtldrCommStart(void)
{
    if (port_MONITORING_SCB_MODE_I2C_RUNTM_CFG)
    {
        port_MONITORING_I2CCyBtldrCommStart();
    }
    else if (port_MONITORING_SCB_MODE_EZI2C_RUNTM_CFG)
    {
        port_MONITORING_EzI2CCyBtldrCommStart();
    }
#if (!port_MONITORING_CY_SCBIP_V1)
    else if (port_MONITORING_SCB_MODE_SPI_RUNTM_CFG)
    {
        port_MONITORING_SpiCyBtldrCommStart();
    }
    else if (port_MONITORING_SCB_MODE_UART_RUNTM_CFG)
    {
        port_MONITORING_UartCyBtldrCommStart();
    }
#endif /* (!port_MONITORING_CY_SCBIP_V1) */
    else
    {
        /* Unknown mode: do nothing */
    }
}


/*******************************************************************************
* Function Name: port_MONITORING_CyBtldrCommStop
****************************************************************************//**
*
*  Stops port_MONITORING component.
*
*******************************************************************************/
void port_MONITORING_CyBtldrCommStop(void)
{
    if (port_MONITORING_SCB_MODE_I2C_RUNTM_CFG)
    {
        port_MONITORING_I2CCyBtldrCommStop();
    }
    else if (port_MONITORING_SCB_MODE_EZI2C_RUNTM_CFG)
    {
        port_MONITORING_EzI2CCyBtldrCommStop();
    }
#if (!port_MONITORING_CY_SCBIP_V1)
    else if (port_MONITORING_SCB_MODE_SPI_RUNTM_CFG)
    {
        port_MONITORING_SpiCyBtldrCommStop();
    }
    else if (port_MONITORING_SCB_MODE_UART_RUNTM_CFG)
    {
        port_MONITORING_UartCyBtldrCommStop();
    }
#endif /* (!port_MONITORING_CY_SCBIP_V1) */
    else
    {
        /* Unknown mode: do nothing */
    }
}


/*******************************************************************************
* Function Name: port_MONITORING_CyBtldrCommReset
****************************************************************************//**
*
*  Clears port_MONITORING component buffers.
*
*******************************************************************************/
void port_MONITORING_CyBtldrCommReset(void)
{
    if(port_MONITORING_SCB_MODE_I2C_RUNTM_CFG)
    {
        port_MONITORING_I2CCyBtldrCommReset();
    }
    else if(port_MONITORING_SCB_MODE_EZI2C_RUNTM_CFG)
    {
        port_MONITORING_EzI2CCyBtldrCommReset();
    }
#if (!port_MONITORING_CY_SCBIP_V1)
    else if(port_MONITORING_SCB_MODE_SPI_RUNTM_CFG)
    {
        port_MONITORING_SpiCyBtldrCommReset();
    }
    else if(port_MONITORING_SCB_MODE_UART_RUNTM_CFG)
    {
        port_MONITORING_UartCyBtldrCommReset();
    }
#endif /* (!port_MONITORING_CY_SCBIP_V1) */
    else
    {
        /* Unknown mode: do nothing */
    }
}


/*******************************************************************************
* Function Name: port_MONITORING_CyBtldrCommRead
****************************************************************************//**
*
*  Allows the caller to read data from the bootloader host (the host writes the 
*  data). The function handles polling to allow a block of data to be completely
*  received from the host device.
*
*  \param pData: Pointer to storage for the block of data to be read from the
*   bootloader host.
*  \param size: Number of bytes to be read.
*  \param count: Pointer to the variable to write the number of bytes actually
*   read.
*  \param timeOut: Number of units in 10 ms to wait before returning because of a
*   timeout.
*
* \return
*  \return
*  cystatus: Returns CYRET_SUCCESS if no problem was encountered or returns the
*  value that best describes the problem. For more information refer to 
*  the “Return Codes” section of the System Reference Guide.
*
*******************************************************************************/
cystatus port_MONITORING_CyBtldrCommRead(uint8 pData[], uint16 size, uint16 * count, uint8 timeOut)
{
    cystatus status;

    if(port_MONITORING_SCB_MODE_I2C_RUNTM_CFG)
    {
        status = port_MONITORING_I2CCyBtldrCommRead(pData, size, count, timeOut);
    }
    else if(port_MONITORING_SCB_MODE_EZI2C_RUNTM_CFG)
    {
        status = port_MONITORING_EzI2CCyBtldrCommRead(pData, size, count, timeOut);
    }
#if (!port_MONITORING_CY_SCBIP_V1)
    else if(port_MONITORING_SCB_MODE_SPI_RUNTM_CFG)
    {
        status = port_MONITORING_SpiCyBtldrCommRead(pData, size, count, timeOut);
    }
    else if(port_MONITORING_SCB_MODE_UART_RUNTM_CFG)
    {
        status = port_MONITORING_UartCyBtldrCommRead(pData, size, count, timeOut);
    }
#endif /* (!port_MONITORING_CY_SCBIP_V1) */
    else
    {
        status = CYRET_INVALID_STATE; /* Unknown mode: return invalid status */
    }

    return(status);
}


/*******************************************************************************
* Function Name: port_MONITORING_CyBtldrCommWrite
****************************************************************************//**
*
*  Allows the caller to write data to the bootloader host (the host reads the 
*  data). The function does not use timeout and returns after data has been copied
*  into the slave read buffer. This data available to be read by the bootloader
*  host until following host data write.
*
*  \param pData: Pointer to the block of data to be written to the bootloader host.
*  \param size: Number of bytes to be written.
*  \param count: Pointer to the variable to write the number of bytes actually
*   written.
*  \param timeOut: Number of units in 10 ms to wait before returning because of a
*   timeout.
*
*  \return
*  cystatus: Returns CYRET_SUCCESS if no problem was encountered or returns the
*  value that best describes the problem. For more information refer to 
*  the “Return Codes” section of the System Reference Guide.
*
*******************************************************************************/
cystatus port_MONITORING_CyBtldrCommWrite(const uint8 pData[], uint16 size, uint16 * count, uint8 timeOut)
{
    cystatus status;

    if(port_MONITORING_SCB_MODE_I2C_RUNTM_CFG)
    {
        status = port_MONITORING_I2CCyBtldrCommWrite(pData, size, count, timeOut);
    }
    else if(port_MONITORING_SCB_MODE_EZI2C_RUNTM_CFG)
    {
        status = port_MONITORING_EzI2CCyBtldrCommWrite(pData, size, count, timeOut);
    }
#if (!port_MONITORING_CY_SCBIP_V1)
    else if(port_MONITORING_SCB_MODE_SPI_RUNTM_CFG)
    {
        status = port_MONITORING_SpiCyBtldrCommWrite(pData, size, count, timeOut);
    }
    else if(port_MONITORING_SCB_MODE_UART_RUNTM_CFG)
    {
        status = port_MONITORING_UartCyBtldrCommWrite(pData, size, count, timeOut);
    }
#endif /* (!port_MONITORING_CY_SCBIP_V1) */
    else
    {
        status = CYRET_INVALID_STATE; /* Unknown mode: return invalid status */
    }

    return(status);
}

#endif /* defined(CYDEV_BOOTLOADER_IO_COMP) && (port_MONITORING_BTLDR_COMM_MODE_ENABLED) */


/* [] END OF FILE */
