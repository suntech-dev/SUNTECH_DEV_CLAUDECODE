/*******************************************************************************
* File Name: WIFI_SCBCLK.h
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

#if !defined(CY_CLOCK_WIFI_SCBCLK_H)
#define CY_CLOCK_WIFI_SCBCLK_H

#include <cytypes.h>
#include <cyfitter.h>


/***************************************
*        Function Prototypes
***************************************/
#if defined CYREG_PERI_DIV_CMD

void WIFI_SCBCLK_StartEx(uint32 alignClkDiv);
#define WIFI_SCBCLK_Start() \
    WIFI_SCBCLK_StartEx(WIFI_SCBCLK__PA_DIV_ID)

#else

void WIFI_SCBCLK_Start(void);

#endif/* CYREG_PERI_DIV_CMD */

void WIFI_SCBCLK_Stop(void);

void WIFI_SCBCLK_SetFractionalDividerRegister(uint16 clkDivider, uint8 clkFractional);

uint16 WIFI_SCBCLK_GetDividerRegister(void);
uint8  WIFI_SCBCLK_GetFractionalDividerRegister(void);

#define WIFI_SCBCLK_Enable()                         WIFI_SCBCLK_Start()
#define WIFI_SCBCLK_Disable()                        WIFI_SCBCLK_Stop()
#define WIFI_SCBCLK_SetDividerRegister(clkDivider, reset)  \
    WIFI_SCBCLK_SetFractionalDividerRegister((clkDivider), 0u)
#define WIFI_SCBCLK_SetDivider(clkDivider)           WIFI_SCBCLK_SetDividerRegister((clkDivider), 1u)
#define WIFI_SCBCLK_SetDividerValue(clkDivider)      WIFI_SCBCLK_SetDividerRegister((clkDivider) - 1u, 1u)


/***************************************
*             Registers
***************************************/
#if defined CYREG_PERI_DIV_CMD

#define WIFI_SCBCLK_DIV_ID     WIFI_SCBCLK__DIV_ID

#define WIFI_SCBCLK_CMD_REG    (*(reg32 *)CYREG_PERI_DIV_CMD)
#define WIFI_SCBCLK_CTRL_REG   (*(reg32 *)WIFI_SCBCLK__CTRL_REGISTER)
#define WIFI_SCBCLK_DIV_REG    (*(reg32 *)WIFI_SCBCLK__DIV_REGISTER)

#define WIFI_SCBCLK_CMD_DIV_SHIFT          (0u)
#define WIFI_SCBCLK_CMD_PA_DIV_SHIFT       (8u)
#define WIFI_SCBCLK_CMD_DISABLE_SHIFT      (30u)
#define WIFI_SCBCLK_CMD_ENABLE_SHIFT       (31u)

#define WIFI_SCBCLK_CMD_DISABLE_MASK       ((uint32)((uint32)1u << WIFI_SCBCLK_CMD_DISABLE_SHIFT))
#define WIFI_SCBCLK_CMD_ENABLE_MASK        ((uint32)((uint32)1u << WIFI_SCBCLK_CMD_ENABLE_SHIFT))

#define WIFI_SCBCLK_DIV_FRAC_MASK  (0x000000F8u)
#define WIFI_SCBCLK_DIV_FRAC_SHIFT (3u)
#define WIFI_SCBCLK_DIV_INT_MASK   (0xFFFFFF00u)
#define WIFI_SCBCLK_DIV_INT_SHIFT  (8u)

#else 

#define WIFI_SCBCLK_DIV_REG        (*(reg32 *)WIFI_SCBCLK__REGISTER)
#define WIFI_SCBCLK_ENABLE_REG     WIFI_SCBCLK_DIV_REG
#define WIFI_SCBCLK_DIV_FRAC_MASK  WIFI_SCBCLK__FRAC_MASK
#define WIFI_SCBCLK_DIV_FRAC_SHIFT (16u)
#define WIFI_SCBCLK_DIV_INT_MASK   WIFI_SCBCLK__DIVIDER_MASK
#define WIFI_SCBCLK_DIV_INT_SHIFT  (0u)

#endif/* CYREG_PERI_DIV_CMD */

#endif /* !defined(CY_CLOCK_WIFI_SCBCLK_H) */

/* [] END OF FILE */
