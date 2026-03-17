/*******************************************************************************
* File Name: BARCODE_SCBCLK.h
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

#if !defined(CY_CLOCK_BARCODE_SCBCLK_H)
#define CY_CLOCK_BARCODE_SCBCLK_H

#include <cytypes.h>
#include <cyfitter.h>


/***************************************
*        Function Prototypes
***************************************/
#if defined CYREG_PERI_DIV_CMD

void BARCODE_SCBCLK_StartEx(uint32 alignClkDiv);
#define BARCODE_SCBCLK_Start() \
    BARCODE_SCBCLK_StartEx(BARCODE_SCBCLK__PA_DIV_ID)

#else

void BARCODE_SCBCLK_Start(void);

#endif/* CYREG_PERI_DIV_CMD */

void BARCODE_SCBCLK_Stop(void);

void BARCODE_SCBCLK_SetFractionalDividerRegister(uint16 clkDivider, uint8 clkFractional);

uint16 BARCODE_SCBCLK_GetDividerRegister(void);
uint8  BARCODE_SCBCLK_GetFractionalDividerRegister(void);

#define BARCODE_SCBCLK_Enable()                         BARCODE_SCBCLK_Start()
#define BARCODE_SCBCLK_Disable()                        BARCODE_SCBCLK_Stop()
#define BARCODE_SCBCLK_SetDividerRegister(clkDivider, reset)  \
    BARCODE_SCBCLK_SetFractionalDividerRegister((clkDivider), 0u)
#define BARCODE_SCBCLK_SetDivider(clkDivider)           BARCODE_SCBCLK_SetDividerRegister((clkDivider), 1u)
#define BARCODE_SCBCLK_SetDividerValue(clkDivider)      BARCODE_SCBCLK_SetDividerRegister((clkDivider) - 1u, 1u)


/***************************************
*             Registers
***************************************/
#if defined CYREG_PERI_DIV_CMD

#define BARCODE_SCBCLK_DIV_ID     BARCODE_SCBCLK__DIV_ID

#define BARCODE_SCBCLK_CMD_REG    (*(reg32 *)CYREG_PERI_DIV_CMD)
#define BARCODE_SCBCLK_CTRL_REG   (*(reg32 *)BARCODE_SCBCLK__CTRL_REGISTER)
#define BARCODE_SCBCLK_DIV_REG    (*(reg32 *)BARCODE_SCBCLK__DIV_REGISTER)

#define BARCODE_SCBCLK_CMD_DIV_SHIFT          (0u)
#define BARCODE_SCBCLK_CMD_PA_DIV_SHIFT       (8u)
#define BARCODE_SCBCLK_CMD_DISABLE_SHIFT      (30u)
#define BARCODE_SCBCLK_CMD_ENABLE_SHIFT       (31u)

#define BARCODE_SCBCLK_CMD_DISABLE_MASK       ((uint32)((uint32)1u << BARCODE_SCBCLK_CMD_DISABLE_SHIFT))
#define BARCODE_SCBCLK_CMD_ENABLE_MASK        ((uint32)((uint32)1u << BARCODE_SCBCLK_CMD_ENABLE_SHIFT))

#define BARCODE_SCBCLK_DIV_FRAC_MASK  (0x000000F8u)
#define BARCODE_SCBCLK_DIV_FRAC_SHIFT (3u)
#define BARCODE_SCBCLK_DIV_INT_MASK   (0xFFFFFF00u)
#define BARCODE_SCBCLK_DIV_INT_SHIFT  (8u)

#else 

#define BARCODE_SCBCLK_DIV_REG        (*(reg32 *)BARCODE_SCBCLK__REGISTER)
#define BARCODE_SCBCLK_ENABLE_REG     BARCODE_SCBCLK_DIV_REG
#define BARCODE_SCBCLK_DIV_FRAC_MASK  BARCODE_SCBCLK__FRAC_MASK
#define BARCODE_SCBCLK_DIV_FRAC_SHIFT (16u)
#define BARCODE_SCBCLK_DIV_INT_MASK   BARCODE_SCBCLK__DIVIDER_MASK
#define BARCODE_SCBCLK_DIV_INT_SHIFT  (0u)

#endif/* CYREG_PERI_DIV_CMD */

#endif /* !defined(CY_CLOCK_BARCODE_SCBCLK_H) */

/* [] END OF FILE */
