/*******************************************************************************
* File Name: FOOT_SW.h  
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

#if !defined(CY_PINS_FOOT_SW_ALIASES_H) /* Pins FOOT_SW_ALIASES_H */
#define CY_PINS_FOOT_SW_ALIASES_H

#include "cytypes.h"
#include "cyfitter.h"
#include "cypins.h"


/***************************************
*              Constants        
***************************************/
#define FOOT_SW_0			(FOOT_SW__0__PC)
#define FOOT_SW_0_PS		(FOOT_SW__0__PS)
#define FOOT_SW_0_PC		(FOOT_SW__0__PC)
#define FOOT_SW_0_DR		(FOOT_SW__0__DR)
#define FOOT_SW_0_SHIFT	(FOOT_SW__0__SHIFT)
#define FOOT_SW_0_INTR	((uint16)((uint16)0x0003u << (FOOT_SW__0__SHIFT*2u)))

#define FOOT_SW_INTR_ALL	 ((uint16)(FOOT_SW_0_INTR))


#endif /* End Pins FOOT_SW_ALIASES_H */


/* [] END OF FILE */
