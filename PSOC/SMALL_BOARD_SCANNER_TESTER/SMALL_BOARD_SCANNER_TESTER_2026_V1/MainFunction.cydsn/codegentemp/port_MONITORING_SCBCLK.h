/*******************************************************************************
* File Name: port_MONITORING_SCBCLK.h
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

#if !defined(CY_CLOCK_port_MONITORING_SCBCLK_H)
#define CY_CLOCK_port_MONITORING_SCBCLK_H

#include <cytypes.h>
#include <cyfitter.h>


/***************************************
*        Function Prototypes
***************************************/
#if defined CYREG_PERI_DIV_CMD

void port_MONITORING_SCBCLK_StartEx(uint32 alignClkDiv);
#define port_MONITORING_SCBCLK_Start() \
    port_MONITORING_SCBCLK_StartEx(port_MONITORING_SCBCLK__PA_DIV_ID)

#else

void port_MONITORING_SCBCLK_Start(void);

#endif/* CYREG_PERI_DIV_CMD */

void port_MONITORING_SCBCLK_Stop(void);

void port_MONITORING_SCBCLK_SetFractionalDividerRegister(uint16 clkDivider, uint8 clkFractional);

uint16 port_MONITORING_SCBCLK_GetDividerRegister(void);
uint8  port_MONITORING_SCBCLK_GetFractionalDividerRegister(void);

#define port_MONITORING_SCBCLK_Enable()                         port_MONITORING_SCBCLK_Start()
#define port_MONITORING_SCBCLK_Disable()                        port_MONITORING_SCBCLK_Stop()
#define port_MONITORING_SCBCLK_SetDividerRegister(clkDivider, reset)  \
    port_MONITORING_SCBCLK_SetFractionalDividerRegister((clkDivider), 0u)
#define port_MONITORING_SCBCLK_SetDivider(clkDivider)           port_MONITORING_SCBCLK_SetDividerRegister((clkDivider), 1u)
#define port_MONITORING_SCBCLK_SetDividerValue(clkDivider)      port_MONITORING_SCBCLK_SetDividerRegister((clkDivider) - 1u, 1u)


/***************************************
*             Registers
***************************************/
#if defined CYREG_PERI_DIV_CMD

#define port_MONITORING_SCBCLK_DIV_ID     port_MONITORING_SCBCLK__DIV_ID

#define port_MONITORING_SCBCLK_CMD_REG    (*(reg32 *)CYREG_PERI_DIV_CMD)
#define port_MONITORING_SCBCLK_CTRL_REG   (*(reg32 *)port_MONITORING_SCBCLK__CTRL_REGISTER)
#define port_MONITORING_SCBCLK_DIV_REG    (*(reg32 *)port_MONITORING_SCBCLK__DIV_REGISTER)

#define port_MONITORING_SCBCLK_CMD_DIV_SHIFT          (0u)
#define port_MONITORING_SCBCLK_CMD_PA_DIV_SHIFT       (8u)
#define port_MONITORING_SCBCLK_CMD_DISABLE_SHIFT      (30u)
#define port_MONITORING_SCBCLK_CMD_ENABLE_SHIFT       (31u)

#define port_MONITORING_SCBCLK_CMD_DISABLE_MASK       ((uint32)((uint32)1u << port_MONITORING_SCBCLK_CMD_DISABLE_SHIFT))
#define port_MONITORING_SCBCLK_CMD_ENABLE_MASK        ((uint32)((uint32)1u << port_MONITORING_SCBCLK_CMD_ENABLE_SHIFT))

#define port_MONITORING_SCBCLK_DIV_FRAC_MASK  (0x000000F8u)
#define port_MONITORING_SCBCLK_DIV_FRAC_SHIFT (3u)
#define port_MONITORING_SCBCLK_DIV_INT_MASK   (0xFFFFFF00u)
#define port_MONITORING_SCBCLK_DIV_INT_SHIFT  (8u)

#else 

#define port_MONITORING_SCBCLK_DIV_REG        (*(reg32 *)port_MONITORING_SCBCLK__REGISTER)
#define port_MONITORING_SCBCLK_ENABLE_REG     port_MONITORING_SCBCLK_DIV_REG
#define port_MONITORING_SCBCLK_DIV_FRAC_MASK  port_MONITORING_SCBCLK__FRAC_MASK
#define port_MONITORING_SCBCLK_DIV_FRAC_SHIFT (16u)
#define port_MONITORING_SCBCLK_DIV_INT_MASK   port_MONITORING_SCBCLK__DIVIDER_MASK
#define port_MONITORING_SCBCLK_DIV_INT_SHIFT  (0u)

#endif/* CYREG_PERI_DIV_CMD */

#endif /* !defined(CY_CLOCK_port_MONITORING_SCBCLK_H) */

/* [] END OF FILE */
