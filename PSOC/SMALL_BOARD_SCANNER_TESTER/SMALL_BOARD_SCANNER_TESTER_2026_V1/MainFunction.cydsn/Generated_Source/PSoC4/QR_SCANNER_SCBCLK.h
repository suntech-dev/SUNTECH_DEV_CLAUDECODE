/*******************************************************************************
* File Name: QR_SCANNER_SCBCLK.h
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

#if !defined(CY_CLOCK_QR_SCANNER_SCBCLK_H)
#define CY_CLOCK_QR_SCANNER_SCBCLK_H

#include <cytypes.h>
#include <cyfitter.h>


/***************************************
*        Function Prototypes
***************************************/
#if defined CYREG_PERI_DIV_CMD

void QR_SCANNER_SCBCLK_StartEx(uint32 alignClkDiv);
#define QR_SCANNER_SCBCLK_Start() \
    QR_SCANNER_SCBCLK_StartEx(QR_SCANNER_SCBCLK__PA_DIV_ID)

#else

void QR_SCANNER_SCBCLK_Start(void);

#endif/* CYREG_PERI_DIV_CMD */

void QR_SCANNER_SCBCLK_Stop(void);

void QR_SCANNER_SCBCLK_SetFractionalDividerRegister(uint16 clkDivider, uint8 clkFractional);

uint16 QR_SCANNER_SCBCLK_GetDividerRegister(void);
uint8  QR_SCANNER_SCBCLK_GetFractionalDividerRegister(void);

#define QR_SCANNER_SCBCLK_Enable()                         QR_SCANNER_SCBCLK_Start()
#define QR_SCANNER_SCBCLK_Disable()                        QR_SCANNER_SCBCLK_Stop()
#define QR_SCANNER_SCBCLK_SetDividerRegister(clkDivider, reset)  \
    QR_SCANNER_SCBCLK_SetFractionalDividerRegister((clkDivider), 0u)
#define QR_SCANNER_SCBCLK_SetDivider(clkDivider)           QR_SCANNER_SCBCLK_SetDividerRegister((clkDivider), 1u)
#define QR_SCANNER_SCBCLK_SetDividerValue(clkDivider)      QR_SCANNER_SCBCLK_SetDividerRegister((clkDivider) - 1u, 1u)


/***************************************
*             Registers
***************************************/
#if defined CYREG_PERI_DIV_CMD

#define QR_SCANNER_SCBCLK_DIV_ID     QR_SCANNER_SCBCLK__DIV_ID

#define QR_SCANNER_SCBCLK_CMD_REG    (*(reg32 *)CYREG_PERI_DIV_CMD)
#define QR_SCANNER_SCBCLK_CTRL_REG   (*(reg32 *)QR_SCANNER_SCBCLK__CTRL_REGISTER)
#define QR_SCANNER_SCBCLK_DIV_REG    (*(reg32 *)QR_SCANNER_SCBCLK__DIV_REGISTER)

#define QR_SCANNER_SCBCLK_CMD_DIV_SHIFT          (0u)
#define QR_SCANNER_SCBCLK_CMD_PA_DIV_SHIFT       (8u)
#define QR_SCANNER_SCBCLK_CMD_DISABLE_SHIFT      (30u)
#define QR_SCANNER_SCBCLK_CMD_ENABLE_SHIFT       (31u)

#define QR_SCANNER_SCBCLK_CMD_DISABLE_MASK       ((uint32)((uint32)1u << QR_SCANNER_SCBCLK_CMD_DISABLE_SHIFT))
#define QR_SCANNER_SCBCLK_CMD_ENABLE_MASK        ((uint32)((uint32)1u << QR_SCANNER_SCBCLK_CMD_ENABLE_SHIFT))

#define QR_SCANNER_SCBCLK_DIV_FRAC_MASK  (0x000000F8u)
#define QR_SCANNER_SCBCLK_DIV_FRAC_SHIFT (3u)
#define QR_SCANNER_SCBCLK_DIV_INT_MASK   (0xFFFFFF00u)
#define QR_SCANNER_SCBCLK_DIV_INT_SHIFT  (8u)

#else 

#define QR_SCANNER_SCBCLK_DIV_REG        (*(reg32 *)QR_SCANNER_SCBCLK__REGISTER)
#define QR_SCANNER_SCBCLK_ENABLE_REG     QR_SCANNER_SCBCLK_DIV_REG
#define QR_SCANNER_SCBCLK_DIV_FRAC_MASK  QR_SCANNER_SCBCLK__FRAC_MASK
#define QR_SCANNER_SCBCLK_DIV_FRAC_SHIFT (16u)
#define QR_SCANNER_SCBCLK_DIV_INT_MASK   QR_SCANNER_SCBCLK__DIVIDER_MASK
#define QR_SCANNER_SCBCLK_DIV_INT_SHIFT  (0u)

#endif/* CYREG_PERI_DIV_CMD */

#endif /* !defined(CY_CLOCK_QR_SCANNER_SCBCLK_H) */

/* [] END OF FILE */
