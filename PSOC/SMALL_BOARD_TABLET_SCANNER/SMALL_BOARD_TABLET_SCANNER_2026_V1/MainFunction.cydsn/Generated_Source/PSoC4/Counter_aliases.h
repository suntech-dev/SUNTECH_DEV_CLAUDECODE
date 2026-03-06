/*******************************************************************************
* File Name: Counter.h  
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

#if !defined(CY_PINS_Counter_ALIASES_H) /* Pins Counter_ALIASES_H */
#define CY_PINS_Counter_ALIASES_H

#include "cytypes.h"
#include "cyfitter.h"
#include "cypins.h"


/***************************************
*              Constants        
***************************************/
#define Counter_0			(Counter__0__PC)
#define Counter_0_PS		(Counter__0__PS)
#define Counter_0_PC		(Counter__0__PC)
#define Counter_0_DR		(Counter__0__DR)
#define Counter_0_SHIFT	(Counter__0__SHIFT)
#define Counter_0_INTR	((uint16)((uint16)0x0003u << (Counter__0__SHIFT*2u)))

#define Counter_INTR_ALL	 ((uint16)(Counter_0_INTR))


#endif /* End Pins Counter_ALIASES_H */


/* [] END OF FILE */
