/*******************************************************************************
* File Name: PinTrim.h  
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

#if !defined(CY_PINS_PinTrim_ALIASES_H) /* Pins PinTrim_ALIASES_H */
#define CY_PINS_PinTrim_ALIASES_H

#include "cytypes.h"
#include "cyfitter.h"
#include "cypins.h"


/***************************************
*              Constants        
***************************************/
#define PinTrim_0			(PinTrim__0__PC)
#define PinTrim_0_PS		(PinTrim__0__PS)
#define PinTrim_0_PC		(PinTrim__0__PC)
#define PinTrim_0_DR		(PinTrim__0__DR)
#define PinTrim_0_SHIFT	(PinTrim__0__SHIFT)
#define PinTrim_0_INTR	((uint16)((uint16)0x0003u << (PinTrim__0__SHIFT*2u)))

#define PinTrim_INTR_ALL	 ((uint16)(PinTrim_0_INTR))


#endif /* End Pins PinTrim_ALIASES_H */


/* [] END OF FILE */
