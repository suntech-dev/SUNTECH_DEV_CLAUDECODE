/***************************************************************************//**
* \file OP_ST500_BOOT.c
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

#include "OP_ST500_BOOT.h"

#if defined(CYDEV_BOOTLOADER_IO_COMP) && (OP_ST500_BTLDR_COMM_ENABLED) && \
                                (OP_ST500_SCB_MODE_UNCONFIG_CONST_CFG)

/*******************************************************************************
* Function Name: OP_ST500_CyBtldrCommStart
****************************************************************************//**
*
*  Starts OP_ST500 component. After this function call the component is 
*  ready for communication.
*
*******************************************************************************/
void OP_ST500_CyBtldrCommStart(void)
{
    if (OP_ST500_SCB_MODE_I2C_RUNTM_CFG)
    {
        OP_ST500_I2CCyBtldrCommStart();
    }
    else if (OP_ST500_SCB_MODE_EZI2C_RUNTM_CFG)
    {
        OP_ST500_EzI2CCyBtldrCommStart();
    }
#if (!OP_ST500_CY_SCBIP_V1)
    else if (OP_ST500_SCB_MODE_SPI_RUNTM_CFG)
    {
        OP_ST500_SpiCyBtldrCommStart();
    }
    else if (OP_ST500_SCB_MODE_UART_RUNTM_CFG)
    {
        OP_ST500_UartCyBtldrCommStart();
    }
#endif /* (!OP_ST500_CY_SCBIP_V1) */
    else
    {
        /* Unknown mode: do nothing */
    }
}


/*******************************************************************************
* Function Name: OP_ST500_CyBtldrCommStop
****************************************************************************//**
*
*  Stops OP_ST500 component.
*
*******************************************************************************/
void OP_ST500_CyBtldrCommStop(void)
{
    if (OP_ST500_SCB_MODE_I2C_RUNTM_CFG)
    {
        OP_ST500_I2CCyBtldrCommStop();
    }
    else if (OP_ST500_SCB_MODE_EZI2C_RUNTM_CFG)
    {
        OP_ST500_EzI2CCyBtldrCommStop();
    }
#if (!OP_ST500_CY_SCBIP_V1)
    else if (OP_ST500_SCB_MODE_SPI_RUNTM_CFG)
    {
        OP_ST500_SpiCyBtldrCommStop();
    }
    else if (OP_ST500_SCB_MODE_UART_RUNTM_CFG)
    {
        OP_ST500_UartCyBtldrCommStop();
    }
#endif /* (!OP_ST500_CY_SCBIP_V1) */
    else
    {
        /* Unknown mode: do nothing */
    }
}


/*******************************************************************************
* Function Name: OP_ST500_CyBtldrCommReset
****************************************************************************//**
*
*  Clears OP_ST500 component buffers.
*
*******************************************************************************/
void OP_ST500_CyBtldrCommReset(void)
{
    if(OP_ST500_SCB_MODE_I2C_RUNTM_CFG)
    {
        OP_ST500_I2CCyBtldrCommReset();
    }
    else if(OP_ST500_SCB_MODE_EZI2C_RUNTM_CFG)
    {
        OP_ST500_EzI2CCyBtldrCommReset();
    }
#if (!OP_ST500_CY_SCBIP_V1)
    else if(OP_ST500_SCB_MODE_SPI_RUNTM_CFG)
    {
        OP_ST500_SpiCyBtldrCommReset();
    }
    else if(OP_ST500_SCB_MODE_UART_RUNTM_CFG)
    {
        OP_ST500_UartCyBtldrCommReset();
    }
#endif /* (!OP_ST500_CY_SCBIP_V1) */
    else
    {
        /* Unknown mode: do nothing */
    }
}


/*******************************************************************************
* Function Name: OP_ST500_CyBtldrCommRead
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
cystatus OP_ST500_CyBtldrCommRead(uint8 pData[], uint16 size, uint16 * count, uint8 timeOut)
{
    cystatus status;

    if(OP_ST500_SCB_MODE_I2C_RUNTM_CFG)
    {
        status = OP_ST500_I2CCyBtldrCommRead(pData, size, count, timeOut);
    }
    else if(OP_ST500_SCB_MODE_EZI2C_RUNTM_CFG)
    {
        status = OP_ST500_EzI2CCyBtldrCommRead(pData, size, count, timeOut);
    }
#if (!OP_ST500_CY_SCBIP_V1)
    else if(OP_ST500_SCB_MODE_SPI_RUNTM_CFG)
    {
        status = OP_ST500_SpiCyBtldrCommRead(pData, size, count, timeOut);
    }
    else if(OP_ST500_SCB_MODE_UART_RUNTM_CFG)
    {
        status = OP_ST500_UartCyBtldrCommRead(pData, size, count, timeOut);
    }
#endif /* (!OP_ST500_CY_SCBIP_V1) */
    else
    {
        status = CYRET_INVALID_STATE; /* Unknown mode: return invalid status */
    }

    return(status);
}


/*******************************************************************************
* Function Name: OP_ST500_CyBtldrCommWrite
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
cystatus OP_ST500_CyBtldrCommWrite(const uint8 pData[], uint16 size, uint16 * count, uint8 timeOut)
{
    cystatus status;

    if(OP_ST500_SCB_MODE_I2C_RUNTM_CFG)
    {
        status = OP_ST500_I2CCyBtldrCommWrite(pData, size, count, timeOut);
    }
    else if(OP_ST500_SCB_MODE_EZI2C_RUNTM_CFG)
    {
        status = OP_ST500_EzI2CCyBtldrCommWrite(pData, size, count, timeOut);
    }
#if (!OP_ST500_CY_SCBIP_V1)
    else if(OP_ST500_SCB_MODE_SPI_RUNTM_CFG)
    {
        status = OP_ST500_SpiCyBtldrCommWrite(pData, size, count, timeOut);
    }
    else if(OP_ST500_SCB_MODE_UART_RUNTM_CFG)
    {
        status = OP_ST500_UartCyBtldrCommWrite(pData, size, count, timeOut);
    }
#endif /* (!OP_ST500_CY_SCBIP_V1) */
    else
    {
        status = CYRET_INVALID_STATE; /* Unknown mode: return invalid status */
    }

    return(status);
}

#endif /* defined(CYDEV_BOOTLOADER_IO_COMP) && (OP_ST500_BTLDR_COMM_MODE_ENABLED) */


/* [] END OF FILE */
