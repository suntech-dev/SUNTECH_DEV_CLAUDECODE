/*******************************************************************************
* File Name: SwClk.h
* Version 2.20
*
*  Description:
*   Provides the function and constant definitions for the clock component.
*
*  Note:
*
********************************************************************************
* Copyright 2008-2012, Cypress Semiconductor Corporation.  All rights reserved.
* You may use this file only in accordance with the license, terms, conditions, 
* disclaimers, and limitations in the end user license agreement accompanying 
* the software package with which this file was provided.
*******************************************************************************/

#if !defined(CY_CLOCK_SwClk_H)
#define CY_CLOCK_SwClk_H

#include <cytypes.h>
#include <cyfitter.h>


/***************************************
*        Function Prototypes
***************************************/
#if defined CYREG_PERI_DIV_CMD

void SwClk_StartEx(uint32 alignClkDiv);
#define SwClk_Start() \
    SwClk_StartEx(SwClk__PA_DIV_ID)

#else

void SwClk_Start(void);

#endif/* CYREG_PERI_DIV_CMD */

void SwClk_Stop(void);

void SwClk_SetFractionalDividerRegister(uint16 clkDivider, uint8 clkFractional);

uint16 SwClk_GetDividerRegister(void);
uint8  SwClk_GetFractionalDividerRegister(void);

#define SwClk_Enable()                         SwClk_Start()
#define SwClk_Disable()                        SwClk_Stop()
#define SwClk_SetDividerRegister(clkDivider, reset)  \
    SwClk_SetFractionalDividerRegister((clkDivider), 0u)
#define SwClk_SetDivider(clkDivider)           SwClk_SetDividerRegister((clkDivider), 1u)
#define SwClk_SetDividerValue(clkDivider)      SwClk_SetDividerRegister((clkDivider) - 1u, 1u)


/***************************************
*             Registers
***************************************/
#if defined CYREG_PERI_DIV_CMD

#define SwClk_DIV_ID     SwClk__DIV_ID

#define SwClk_CMD_REG    (*(reg32 *)CYREG_PERI_DIV_CMD)
#define SwClk_CTRL_REG   (*(reg32 *)SwClk__CTRL_REGISTER)
#define SwClk_DIV_REG    (*(reg32 *)SwClk__DIV_REGISTER)

#define SwClk_CMD_DIV_SHIFT          (0u)
#define SwClk_CMD_PA_DIV_SHIFT       (8u)
#define SwClk_CMD_DISABLE_SHIFT      (30u)
#define SwClk_CMD_ENABLE_SHIFT       (31u)

#define SwClk_CMD_DISABLE_MASK       ((uint32)((uint32)1u << SwClk_CMD_DISABLE_SHIFT))
#define SwClk_CMD_ENABLE_MASK        ((uint32)((uint32)1u << SwClk_CMD_ENABLE_SHIFT))

#define SwClk_DIV_FRAC_MASK  (0x000000F8u)
#define SwClk_DIV_FRAC_SHIFT (3u)
#define SwClk_DIV_INT_MASK   (0xFFFFFF00u)
#define SwClk_DIV_INT_SHIFT  (8u)

#else 

#define SwClk_DIV_REG        (*(reg32 *)SwClk__REGISTER)
#define SwClk_ENABLE_REG     SwClk_DIV_REG
#define SwClk_DIV_FRAC_MASK  SwClk__FRAC_MASK
#define SwClk_DIV_FRAC_SHIFT (16u)
#define SwClk_DIV_INT_MASK   SwClk__DIVIDER_MASK
#define SwClk_DIV_INT_SHIFT  (0u)

#endif/* CYREG_PERI_DIV_CMD */

#endif /* !defined(CY_CLOCK_SwClk_H) */

/* [] END OF FILE */
