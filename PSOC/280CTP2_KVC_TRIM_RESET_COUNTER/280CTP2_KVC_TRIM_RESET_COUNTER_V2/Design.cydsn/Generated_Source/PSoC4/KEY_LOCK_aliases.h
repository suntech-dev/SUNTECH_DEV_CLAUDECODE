/*******************************************************************************
* File Name: KEY_LOCK.h  
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

#if !defined(CY_PINS_KEY_LOCK_ALIASES_H) /* Pins KEY_LOCK_ALIASES_H */
#define CY_PINS_KEY_LOCK_ALIASES_H

#include "cytypes.h"
#include "cyfitter.h"
#include "cypins.h"


/***************************************
*              Constants        
***************************************/
#define KEY_LOCK_0			(KEY_LOCK__0__PC)
#define KEY_LOCK_0_PS		(KEY_LOCK__0__PS)
#define KEY_LOCK_0_PC		(KEY_LOCK__0__PC)
#define KEY_LOCK_0_DR		(KEY_LOCK__0__DR)
#define KEY_LOCK_0_SHIFT	(KEY_LOCK__0__SHIFT)
#define KEY_LOCK_0_INTR	((uint16)((uint16)0x0003u << (KEY_LOCK__0__SHIFT*2u)))

#define KEY_LOCK_INTR_ALL	 ((uint16)(KEY_LOCK_0_INTR))


#endif /* End Pins KEY_LOCK_ALIASES_H */


/* [] END OF FILE */
