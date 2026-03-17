/*******************************************************************************
* File Name: TrimPin.h  
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

#if !defined(CY_PINS_TrimPin_ALIASES_H) /* Pins TrimPin_ALIASES_H */
#define CY_PINS_TrimPin_ALIASES_H

#include "cytypes.h"
#include "cyfitter.h"
#include "cypins.h"


/***************************************
*              Constants        
***************************************/
#define TrimPin_0			(TrimPin__0__PC)
#define TrimPin_0_PS		(TrimPin__0__PS)
#define TrimPin_0_PC		(TrimPin__0__PC)
#define TrimPin_0_DR		(TrimPin__0__DR)
#define TrimPin_0_SHIFT	(TrimPin__0__SHIFT)
#define TrimPin_0_INTR	((uint16)((uint16)0x0003u << (TrimPin__0__SHIFT*2u)))

#define TrimPin_INTR_ALL	 ((uint16)(TrimPin_0_INTR))


#endif /* End Pins TrimPin_ALIASES_H */


/* [] END OF FILE */
