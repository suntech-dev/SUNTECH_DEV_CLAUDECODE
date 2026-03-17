/*******************************************************************************
* File Name: Pin_WorkingTime.h  
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

#if !defined(CY_PINS_Pin_WorkingTime_ALIASES_H) /* Pins Pin_WorkingTime_ALIASES_H */
#define CY_PINS_Pin_WorkingTime_ALIASES_H

#include "cytypes.h"
#include "cyfitter.h"
#include "cypins.h"


/***************************************
*              Constants        
***************************************/
#define Pin_WorkingTime_0			(Pin_WorkingTime__0__PC)
#define Pin_WorkingTime_0_PS		(Pin_WorkingTime__0__PS)
#define Pin_WorkingTime_0_PC		(Pin_WorkingTime__0__PC)
#define Pin_WorkingTime_0_DR		(Pin_WorkingTime__0__DR)
#define Pin_WorkingTime_0_SHIFT	(Pin_WorkingTime__0__SHIFT)
#define Pin_WorkingTime_0_INTR	((uint16)((uint16)0x0003u << (Pin_WorkingTime__0__SHIFT*2u)))

#define Pin_WorkingTime_INTR_ALL	 ((uint16)(Pin_WorkingTime_0_INTR))


#endif /* End Pins Pin_WorkingTime_ALIASES_H */


/* [] END OF FILE */
