/*******************************************************************************
* File Name: LCD_MISO.h  
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

#if !defined(CY_PINS_LCD_MISO_ALIASES_H) /* Pins LCD_MISO_ALIASES_H */
#define CY_PINS_LCD_MISO_ALIASES_H

#include "cytypes.h"
#include "cyfitter.h"
#include "cypins.h"


/***************************************
*              Constants        
***************************************/
#define LCD_MISO_0			(LCD_MISO__0__PC)
#define LCD_MISO_0_PS		(LCD_MISO__0__PS)
#define LCD_MISO_0_PC		(LCD_MISO__0__PC)
#define LCD_MISO_0_DR		(LCD_MISO__0__DR)
#define LCD_MISO_0_SHIFT	(LCD_MISO__0__SHIFT)
#define LCD_MISO_0_INTR	((uint16)((uint16)0x0003u << (LCD_MISO__0__SHIFT*2u)))

#define LCD_MISO_INTR_ALL	 ((uint16)(LCD_MISO_0_INTR))


#endif /* End Pins LCD_MISO_ALIASES_H */


/* [] END OF FILE */
