/*******************************************************************************
* File Name: MONITORING_TABLET_SCBCLK.h
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

#if !defined(CY_CLOCK_MONITORING_TABLET_SCBCLK_H)
#define CY_CLOCK_MONITORING_TABLET_SCBCLK_H

#include <cytypes.h>
#include <cyfitter.h>


/***************************************
*        Function Prototypes
***************************************/
#if defined CYREG_PERI_DIV_CMD

void MONITORING_TABLET_SCBCLK_StartEx(uint32 alignClkDiv);
#define MONITORING_TABLET_SCBCLK_Start() \
    MONITORING_TABLET_SCBCLK_StartEx(MONITORING_TABLET_SCBCLK__PA_DIV_ID)

#else

void MONITORING_TABLET_SCBCLK_Start(void);

#endif/* CYREG_PERI_DIV_CMD */

void MONITORING_TABLET_SCBCLK_Stop(void);

void MONITORING_TABLET_SCBCLK_SetFractionalDividerRegister(uint16 clkDivider, uint8 clkFractional);

uint16 MONITORING_TABLET_SCBCLK_GetDividerRegister(void);
uint8  MONITORING_TABLET_SCBCLK_GetFractionalDividerRegister(void);

#define MONITORING_TABLET_SCBCLK_Enable()                         MONITORING_TABLET_SCBCLK_Start()
#define MONITORING_TABLET_SCBCLK_Disable()                        MONITORING_TABLET_SCBCLK_Stop()
#define MONITORING_TABLET_SCBCLK_SetDividerRegister(clkDivider, reset)  \
    MONITORING_TABLET_SCBCLK_SetFractionalDividerRegister((clkDivider), 0u)
#define MONITORING_TABLET_SCBCLK_SetDivider(clkDivider)           MONITORING_TABLET_SCBCLK_SetDividerRegister((clkDivider), 1u)
#define MONITORING_TABLET_SCBCLK_SetDividerValue(clkDivider)      MONITORING_TABLET_SCBCLK_SetDividerRegister((clkDivider) - 1u, 1u)


/***************************************
*             Registers
***************************************/
#if defined CYREG_PERI_DIV_CMD

#define MONITORING_TABLET_SCBCLK_DIV_ID     MONITORING_TABLET_SCBCLK__DIV_ID

#define MONITORING_TABLET_SCBCLK_CMD_REG    (*(reg32 *)CYREG_PERI_DIV_CMD)
#define MONITORING_TABLET_SCBCLK_CTRL_REG   (*(reg32 *)MONITORING_TABLET_SCBCLK__CTRL_REGISTER)
#define MONITORING_TABLET_SCBCLK_DIV_REG    (*(reg32 *)MONITORING_TABLET_SCBCLK__DIV_REGISTER)

#define MONITORING_TABLET_SCBCLK_CMD_DIV_SHIFT          (0u)
#define MONITORING_TABLET_SCBCLK_CMD_PA_DIV_SHIFT       (8u)
#define MONITORING_TABLET_SCBCLK_CMD_DISABLE_SHIFT      (30u)
#define MONITORING_TABLET_SCBCLK_CMD_ENABLE_SHIFT       (31u)

#define MONITORING_TABLET_SCBCLK_CMD_DISABLE_MASK       ((uint32)((uint32)1u << MONITORING_TABLET_SCBCLK_CMD_DISABLE_SHIFT))
#define MONITORING_TABLET_SCBCLK_CMD_ENABLE_MASK        ((uint32)((uint32)1u << MONITORING_TABLET_SCBCLK_CMD_ENABLE_SHIFT))

#define MONITORING_TABLET_SCBCLK_DIV_FRAC_MASK  (0x000000F8u)
#define MONITORING_TABLET_SCBCLK_DIV_FRAC_SHIFT (3u)
#define MONITORING_TABLET_SCBCLK_DIV_INT_MASK   (0xFFFFFF00u)
#define MONITORING_TABLET_SCBCLK_DIV_INT_SHIFT  (8u)

#else 

#define MONITORING_TABLET_SCBCLK_DIV_REG        (*(reg32 *)MONITORING_TABLET_SCBCLK__REGISTER)
#define MONITORING_TABLET_SCBCLK_ENABLE_REG     MONITORING_TABLET_SCBCLK_DIV_REG
#define MONITORING_TABLET_SCBCLK_DIV_FRAC_MASK  MONITORING_TABLET_SCBCLK__FRAC_MASK
#define MONITORING_TABLET_SCBCLK_DIV_FRAC_SHIFT (16u)
#define MONITORING_TABLET_SCBCLK_DIV_INT_MASK   MONITORING_TABLET_SCBCLK__DIVIDER_MASK
#define MONITORING_TABLET_SCBCLK_DIV_INT_SHIFT  (0u)

#endif/* CYREG_PERI_DIV_CMD */

#endif /* !defined(CY_CLOCK_MONITORING_TABLET_SCBCLK_H) */

/* [] END OF FILE */
