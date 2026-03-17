/*******************************************************************************
* File Name: OP_ST500_rx.h  
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

#if !defined(CY_PINS_OP_ST500_rx_ALIASES_H) /* Pins OP_ST500_rx_ALIASES_H */
#define CY_PINS_OP_ST500_rx_ALIASES_H

#include "cytypes.h"
#include "cyfitter.h"
#include "cypins.h"


/***************************************
*              Constants        
***************************************/
#define OP_ST500_rx_0			(OP_ST500_rx__0__PC)
#define OP_ST500_rx_0_PS		(OP_ST500_rx__0__PS)
#define OP_ST500_rx_0_PC		(OP_ST500_rx__0__PC)
#define OP_ST500_rx_0_DR		(OP_ST500_rx__0__DR)
#define OP_ST500_rx_0_SHIFT	(OP_ST500_rx__0__SHIFT)
#define OP_ST500_rx_0_INTR	((uint16)((uint16)0x0003u << (OP_ST500_rx__0__SHIFT*2u)))

#define OP_ST500_rx_INTR_ALL	 ((uint16)(OP_ST500_rx_0_INTR))


#endif /* End Pins OP_ST500_rx_ALIASES_H */


/* [] END OF FILE */
