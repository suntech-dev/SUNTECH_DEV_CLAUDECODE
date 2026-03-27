/*******************************************************************************
* File Name: LCD_RES.h  
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

#if !defined(CY_PINS_LCD_RES_ALIASES_H) /* Pins LCD_RES_ALIASES_H */
#define CY_PINS_LCD_RES_ALIASES_H

#include "cytypes.h"
#include "cyfitter.h"
#include "cypins.h"


/***************************************
*              Constants        
***************************************/
#define LCD_RES_0			(LCD_RES__0__PC)
#define LCD_RES_0_PS		(LCD_RES__0__PS)
#define LCD_RES_0_PC		(LCD_RES__0__PC)
#define LCD_RES_0_DR		(LCD_RES__0__DR)
#define LCD_RES_0_SHIFT	(LCD_RES__0__SHIFT)
#define LCD_RES_0_INTR	((uint16)((uint16)0x0003u << (LCD_RES__0__SHIFT*2u)))

#define LCD_RES_INTR_ALL	 ((uint16)(LCD_RES_0_INTR))


#endif /* End Pins LCD_RES_ALIASES_H */


/* [] END OF FILE */
