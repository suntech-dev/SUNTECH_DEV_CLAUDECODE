/*******************************************************************************
* File Name: InputPin.h  
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

#if !defined(CY_PINS_InputPin_ALIASES_H) /* Pins InputPin_ALIASES_H */
#define CY_PINS_InputPin_ALIASES_H

#include "cytypes.h"
#include "cyfitter.h"
#include "cypins.h"


/***************************************
*              Constants        
***************************************/
#define InputPin_0			(InputPin__0__PC)
#define InputPin_0_PS		(InputPin__0__PS)
#define InputPin_0_PC		(InputPin__0__PC)
#define InputPin_0_DR		(InputPin__0__DR)
#define InputPin_0_SHIFT	(InputPin__0__SHIFT)
#define InputPin_0_INTR	((uint16)((uint16)0x0003u << (InputPin__0__SHIFT*2u)))

#define InputPin_INTR_ALL	 ((uint16)(InputPin_0_INTR))


#endif /* End Pins InputPin_ALIASES_H */


/* [] END OF FILE */
