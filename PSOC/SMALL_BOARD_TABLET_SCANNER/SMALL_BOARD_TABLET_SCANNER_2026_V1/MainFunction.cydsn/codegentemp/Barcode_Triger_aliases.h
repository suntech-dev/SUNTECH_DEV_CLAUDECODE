/*******************************************************************************
* File Name: Barcode_Triger.h  
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

#if !defined(CY_PINS_Barcode_Triger_ALIASES_H) /* Pins Barcode_Triger_ALIASES_H */
#define CY_PINS_Barcode_Triger_ALIASES_H

#include "cytypes.h"
#include "cyfitter.h"
#include "cypins.h"


/***************************************
*              Constants        
***************************************/
#define Barcode_Triger_0			(Barcode_Triger__0__PC)
#define Barcode_Triger_0_PS		(Barcode_Triger__0__PS)
#define Barcode_Triger_0_PC		(Barcode_Triger__0__PC)
#define Barcode_Triger_0_DR		(Barcode_Triger__0__DR)
#define Barcode_Triger_0_SHIFT	(Barcode_Triger__0__SHIFT)
#define Barcode_Triger_0_INTR	((uint16)((uint16)0x0003u << (Barcode_Triger__0__SHIFT*2u)))

#define Barcode_Triger_INTR_ALL	 ((uint16)(Barcode_Triger_0_INTR))


#endif /* End Pins Barcode_Triger_ALIASES_H */


/* [] END OF FILE */
