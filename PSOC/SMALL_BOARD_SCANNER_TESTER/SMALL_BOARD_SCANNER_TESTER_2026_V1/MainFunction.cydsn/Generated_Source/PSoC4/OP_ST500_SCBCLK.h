/*******************************************************************************
* File Name: OP_ST500_SCBCLK.h
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

#if !defined(CY_CLOCK_OP_ST500_SCBCLK_H)
#define CY_CLOCK_OP_ST500_SCBCLK_H

#include <cytypes.h>
#include <cyfitter.h>


/***************************************
*        Function Prototypes
***************************************/
#if defined CYREG_PERI_DIV_CMD

void OP_ST500_SCBCLK_StartEx(uint32 alignClkDiv);
#define OP_ST500_SCBCLK_Start() \
    OP_ST500_SCBCLK_StartEx(OP_ST500_SCBCLK__PA_DIV_ID)

#else

void OP_ST500_SCBCLK_Start(void);

#endif/* CYREG_PERI_DIV_CMD */

void OP_ST500_SCBCLK_Stop(void);

void OP_ST500_SCBCLK_SetFractionalDividerRegister(uint16 clkDivider, uint8 clkFractional);

uint16 OP_ST500_SCBCLK_GetDividerRegister(void);
uint8  OP_ST500_SCBCLK_GetFractionalDividerRegister(void);

#define OP_ST500_SCBCLK_Enable()                         OP_ST500_SCBCLK_Start()
#define OP_ST500_SCBCLK_Disable()                        OP_ST500_SCBCLK_Stop()
#define OP_ST500_SCBCLK_SetDividerRegister(clkDivider, reset)  \
    OP_ST500_SCBCLK_SetFractionalDividerRegister((clkDivider), 0u)
#define OP_ST500_SCBCLK_SetDivider(clkDivider)           OP_ST500_SCBCLK_SetDividerRegister((clkDivider), 1u)
#define OP_ST500_SCBCLK_SetDividerValue(clkDivider)      OP_ST500_SCBCLK_SetDividerRegister((clkDivider) - 1u, 1u)


/***************************************
*             Registers
***************************************/
#if defined CYREG_PERI_DIV_CMD

#define OP_ST500_SCBCLK_DIV_ID     OP_ST500_SCBCLK__DIV_ID

#define OP_ST500_SCBCLK_CMD_REG    (*(reg32 *)CYREG_PERI_DIV_CMD)
#define OP_ST500_SCBCLK_CTRL_REG   (*(reg32 *)OP_ST500_SCBCLK__CTRL_REGISTER)
#define OP_ST500_SCBCLK_DIV_REG    (*(reg32 *)OP_ST500_SCBCLK__DIV_REGISTER)

#define OP_ST500_SCBCLK_CMD_DIV_SHIFT          (0u)
#define OP_ST500_SCBCLK_CMD_PA_DIV_SHIFT       (8u)
#define OP_ST500_SCBCLK_CMD_DISABLE_SHIFT      (30u)
#define OP_ST500_SCBCLK_CMD_ENABLE_SHIFT       (31u)

#define OP_ST500_SCBCLK_CMD_DISABLE_MASK       ((uint32)((uint32)1u << OP_ST500_SCBCLK_CMD_DISABLE_SHIFT))
#define OP_ST500_SCBCLK_CMD_ENABLE_MASK        ((uint32)((uint32)1u << OP_ST500_SCBCLK_CMD_ENABLE_SHIFT))

#define OP_ST500_SCBCLK_DIV_FRAC_MASK  (0x000000F8u)
#define OP_ST500_SCBCLK_DIV_FRAC_SHIFT (3u)
#define OP_ST500_SCBCLK_DIV_INT_MASK   (0xFFFFFF00u)
#define OP_ST500_SCBCLK_DIV_INT_SHIFT  (8u)

#else 

#define OP_ST500_SCBCLK_DIV_REG        (*(reg32 *)OP_ST500_SCBCLK__REGISTER)
#define OP_ST500_SCBCLK_ENABLE_REG     OP_ST500_SCBCLK_DIV_REG
#define OP_ST500_SCBCLK_DIV_FRAC_MASK  OP_ST500_SCBCLK__FRAC_MASK
#define OP_ST500_SCBCLK_DIV_FRAC_SHIFT (16u)
#define OP_ST500_SCBCLK_DIV_INT_MASK   OP_ST500_SCBCLK__DIVIDER_MASK
#define OP_ST500_SCBCLK_DIV_INT_SHIFT  (0u)

#endif/* CYREG_PERI_DIV_CMD */

#endif /* !defined(CY_CLOCK_OP_ST500_SCBCLK_H) */

/* [] END OF FILE */
