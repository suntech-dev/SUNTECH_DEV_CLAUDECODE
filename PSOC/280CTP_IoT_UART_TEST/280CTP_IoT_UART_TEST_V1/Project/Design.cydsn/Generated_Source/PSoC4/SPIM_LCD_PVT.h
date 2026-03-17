/*******************************************************************************
* File Name: .h
* Version 2.50
*
* Description:
*  This private header file contains internal definitions for the SPIM
*  component. Do not use these definitions directly in your application.
*
* Note:
*
********************************************************************************
* Copyright 2012-2015, Cypress Semiconductor Corporation. All rights reserved.
* You may use this file only in accordance with the license, terms, conditions,
* disclaimers, and limitations in the end user license agreement accompanying
* the software package with which this file was provided.
*******************************************************************************/

#if !defined(CY_SPIM_PVT_SPIM_LCD_H)
#define CY_SPIM_PVT_SPIM_LCD_H

#include "SPIM_LCD.h"


/**********************************
*   Functions with external linkage
**********************************/


/**********************************
*   Variables with external linkage
**********************************/

extern volatile uint8 SPIM_LCD_swStatusTx;
extern volatile uint8 SPIM_LCD_swStatusRx;

#if(SPIM_LCD_TX_SOFTWARE_BUF_ENABLED)
    extern volatile uint8 SPIM_LCD_txBuffer[SPIM_LCD_TX_BUFFER_SIZE];
    extern volatile uint8 SPIM_LCD_txBufferRead;
    extern volatile uint8 SPIM_LCD_txBufferWrite;
    extern volatile uint8 SPIM_LCD_txBufferFull;
#endif /* (SPIM_LCD_TX_SOFTWARE_BUF_ENABLED) */

#if(SPIM_LCD_RX_SOFTWARE_BUF_ENABLED)
    extern volatile uint8 SPIM_LCD_rxBuffer[SPIM_LCD_RX_BUFFER_SIZE];
    extern volatile uint8 SPIM_LCD_rxBufferRead;
    extern volatile uint8 SPIM_LCD_rxBufferWrite;
    extern volatile uint8 SPIM_LCD_rxBufferFull;
#endif /* (SPIM_LCD_RX_SOFTWARE_BUF_ENABLED) */

#endif /* CY_SPIM_PVT_SPIM_LCD_H */


/* [] END OF FILE */
