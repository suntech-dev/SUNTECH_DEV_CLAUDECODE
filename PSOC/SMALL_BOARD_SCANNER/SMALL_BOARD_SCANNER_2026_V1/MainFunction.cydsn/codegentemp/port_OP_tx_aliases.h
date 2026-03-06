/*******************************************************************************
* File Name: port_OP_tx.h  
* Version 2.20
*
* Description:
*  This file contains the Alias definitions for Per-Pin APIs in cypins.h. 
*  Information on using these APIs can be found in the System Reference Guide.
*
* Note:
*
********************************************************************************
* Copyright 2008-2015, Cypress Semiconductor Corporation.  All rights reserved.
* You may use this file only in accordance with the license, terms, conditions, 
* disclaimers, and limitations in the end user license agreement accompanying 
* the software package with which this file was provided.
*******************************************************************************/

#if !defined(CY_PINS_port_OP_tx_ALIASES_H) /* Pins port_OP_tx_ALIASES_H */
#define CY_PINS_port_OP_tx_ALIASES_H

#include "cytypes.h"
#include "cyfitter.h"
#include "cypins.h"


/***************************************
*              Constants        
***************************************/
#define port_OP_tx_0			(port_OP_tx__0__PC)
#define port_OP_tx_0_PS		(port_OP_tx__0__PS)
#define port_OP_tx_0_PC		(port_OP_tx__0__PC)
#define port_OP_tx_0_DR		(port_OP_tx__0__DR)
#define port_OP_tx_0_SHIFT	(port_OP_tx__0__SHIFT)
#define port_OP_tx_0_INTR	((uint16)((uint16)0x0003u << (port_OP_tx__0__SHIFT*2u)))

#define port_OP_tx_INTR_ALL	 ((uint16)(port_OP_tx_0_INTR))


#endif /* End Pins port_OP_tx_ALIASES_H */


/* [] END OF FILE */
