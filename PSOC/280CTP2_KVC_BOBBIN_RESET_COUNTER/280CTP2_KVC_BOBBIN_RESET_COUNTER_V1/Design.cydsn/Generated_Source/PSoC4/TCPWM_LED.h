/*******************************************************************************
* File Name: TCPWM_LED.h
* Version 2.10
*
* Description:
*  This file provides constants and parameter values for the TCPWM_LED
*  component.
*
* Note:
*  None
*
********************************************************************************
* Copyright 2013-2015, Cypress Semiconductor Corporation.  All rights reserved.
* You may use this file only in accordance with the license, terms, conditions,
* disclaimers, and limitations in the end user license agreement accompanying
* the software package with which this file was provided.
*******************************************************************************/

#if !defined(CY_TCPWM_TCPWM_LED_H)
#define CY_TCPWM_TCPWM_LED_H


#include "CyLib.h"
#include "cytypes.h"
#include "cyfitter.h"


/*******************************************************************************
* Internal Type defines
*******************************************************************************/

/* Structure to save state before go to sleep */
typedef struct
{
    uint8  enableState;
} TCPWM_LED_BACKUP_STRUCT;


/*******************************************************************************
* Variables
*******************************************************************************/
extern uint8  TCPWM_LED_initVar;


/***************************************
*   Conditional Compilation Parameters
****************************************/

#define TCPWM_LED_CY_TCPWM_V2                    (CYIPBLOCK_m0s8tcpwm_VERSION == 2u)
#define TCPWM_LED_CY_TCPWM_4000                  (CY_PSOC4_4000)

/* TCPWM Configuration */
#define TCPWM_LED_CONFIG                         (7lu)

/* Quad Mode */
/* Parameters */
#define TCPWM_LED_QUAD_ENCODING_MODES            (0lu)
#define TCPWM_LED_QUAD_AUTO_START                (1lu)

/* Signal modes */
#define TCPWM_LED_QUAD_INDEX_SIGNAL_MODE         (0lu)
#define TCPWM_LED_QUAD_PHIA_SIGNAL_MODE          (3lu)
#define TCPWM_LED_QUAD_PHIB_SIGNAL_MODE          (3lu)
#define TCPWM_LED_QUAD_STOP_SIGNAL_MODE          (0lu)

/* Signal present */
#define TCPWM_LED_QUAD_INDEX_SIGNAL_PRESENT      (0lu)
#define TCPWM_LED_QUAD_STOP_SIGNAL_PRESENT       (0lu)

/* Interrupt Mask */
#define TCPWM_LED_QUAD_INTERRUPT_MASK            (1lu)

/* Timer/Counter Mode */
/* Parameters */
#define TCPWM_LED_TC_RUN_MODE                    (0lu)
#define TCPWM_LED_TC_COUNTER_MODE                (0lu)
#define TCPWM_LED_TC_COMP_CAP_MODE               (2lu)
#define TCPWM_LED_TC_PRESCALER                   (0lu)

/* Signal modes */
#define TCPWM_LED_TC_RELOAD_SIGNAL_MODE          (0lu)
#define TCPWM_LED_TC_COUNT_SIGNAL_MODE           (3lu)
#define TCPWM_LED_TC_START_SIGNAL_MODE           (0lu)
#define TCPWM_LED_TC_STOP_SIGNAL_MODE            (0lu)
#define TCPWM_LED_TC_CAPTURE_SIGNAL_MODE         (0lu)

/* Signal present */
#define TCPWM_LED_TC_RELOAD_SIGNAL_PRESENT       (0lu)
#define TCPWM_LED_TC_COUNT_SIGNAL_PRESENT        (0lu)
#define TCPWM_LED_TC_START_SIGNAL_PRESENT        (0lu)
#define TCPWM_LED_TC_STOP_SIGNAL_PRESENT         (0lu)
#define TCPWM_LED_TC_CAPTURE_SIGNAL_PRESENT      (0lu)

/* Interrupt Mask */
#define TCPWM_LED_TC_INTERRUPT_MASK              (1lu)

/* PWM Mode */
/* Parameters */
#define TCPWM_LED_PWM_KILL_EVENT                 (0lu)
#define TCPWM_LED_PWM_STOP_EVENT                 (0lu)
#define TCPWM_LED_PWM_MODE                       (4lu)
#define TCPWM_LED_PWM_OUT_N_INVERT               (0lu)
#define TCPWM_LED_PWM_OUT_INVERT                 (0lu)
#define TCPWM_LED_PWM_ALIGN                      (0lu)
#define TCPWM_LED_PWM_RUN_MODE                   (0lu)
#define TCPWM_LED_PWM_DEAD_TIME_CYCLE            (0lu)
#define TCPWM_LED_PWM_PRESCALER                  (0lu)

/* Signal modes */
#define TCPWM_LED_PWM_RELOAD_SIGNAL_MODE         (0lu)
#define TCPWM_LED_PWM_COUNT_SIGNAL_MODE          (3lu)
#define TCPWM_LED_PWM_START_SIGNAL_MODE          (0lu)
#define TCPWM_LED_PWM_STOP_SIGNAL_MODE           (0lu)
#define TCPWM_LED_PWM_SWITCH_SIGNAL_MODE         (0lu)

/* Signal present */
#define TCPWM_LED_PWM_RELOAD_SIGNAL_PRESENT      (0lu)
#define TCPWM_LED_PWM_COUNT_SIGNAL_PRESENT       (0lu)
#define TCPWM_LED_PWM_START_SIGNAL_PRESENT       (0lu)
#define TCPWM_LED_PWM_STOP_SIGNAL_PRESENT        (0lu)
#define TCPWM_LED_PWM_SWITCH_SIGNAL_PRESENT      (0lu)

/* Interrupt Mask */
#define TCPWM_LED_PWM_INTERRUPT_MASK             (0lu)


/***************************************
*    Initial Parameter Constants
***************************************/

/* Timer/Counter Mode */
#define TCPWM_LED_TC_PERIOD_VALUE                (65535lu)
#define TCPWM_LED_TC_COMPARE_VALUE               (65535lu)
#define TCPWM_LED_TC_COMPARE_BUF_VALUE           (65535lu)
#define TCPWM_LED_TC_COMPARE_SWAP                (0lu)

/* PWM Mode */
#define TCPWM_LED_PWM_PERIOD_VALUE               (999lu)
#define TCPWM_LED_PWM_PERIOD_BUF_VALUE           (65535lu)
#define TCPWM_LED_PWM_PERIOD_SWAP                (0lu)
#define TCPWM_LED_PWM_COMPARE_VALUE              (100lu)
#define TCPWM_LED_PWM_COMPARE_BUF_VALUE          (65535lu)
#define TCPWM_LED_PWM_COMPARE_SWAP               (0lu)


/***************************************
*    Enumerated Types and Parameters
***************************************/

#define TCPWM_LED__LEFT 0
#define TCPWM_LED__RIGHT 1
#define TCPWM_LED__CENTER 2
#define TCPWM_LED__ASYMMETRIC 3

#define TCPWM_LED__X1 0
#define TCPWM_LED__X2 1
#define TCPWM_LED__X4 2

#define TCPWM_LED__PWM 4
#define TCPWM_LED__PWM_DT 5
#define TCPWM_LED__PWM_PR 6

#define TCPWM_LED__INVERSE 1
#define TCPWM_LED__DIRECT 0

#define TCPWM_LED__CAPTURE 2
#define TCPWM_LED__COMPARE 0

#define TCPWM_LED__TRIG_LEVEL 3
#define TCPWM_LED__TRIG_RISING 0
#define TCPWM_LED__TRIG_FALLING 1
#define TCPWM_LED__TRIG_BOTH 2

#define TCPWM_LED__INTR_MASK_TC 1
#define TCPWM_LED__INTR_MASK_CC_MATCH 2
#define TCPWM_LED__INTR_MASK_NONE 0
#define TCPWM_LED__INTR_MASK_TC_CC 3

#define TCPWM_LED__UNCONFIG 8
#define TCPWM_LED__TIMER 1
#define TCPWM_LED__QUAD 3
#define TCPWM_LED__PWM_SEL 7

#define TCPWM_LED__COUNT_UP 0
#define TCPWM_LED__COUNT_DOWN 1
#define TCPWM_LED__COUNT_UPDOWN0 2
#define TCPWM_LED__COUNT_UPDOWN1 3


/* Prescaler */
#define TCPWM_LED_PRESCALE_DIVBY1                ((uint32)(0u << TCPWM_LED_PRESCALER_SHIFT))
#define TCPWM_LED_PRESCALE_DIVBY2                ((uint32)(1u << TCPWM_LED_PRESCALER_SHIFT))
#define TCPWM_LED_PRESCALE_DIVBY4                ((uint32)(2u << TCPWM_LED_PRESCALER_SHIFT))
#define TCPWM_LED_PRESCALE_DIVBY8                ((uint32)(3u << TCPWM_LED_PRESCALER_SHIFT))
#define TCPWM_LED_PRESCALE_DIVBY16               ((uint32)(4u << TCPWM_LED_PRESCALER_SHIFT))
#define TCPWM_LED_PRESCALE_DIVBY32               ((uint32)(5u << TCPWM_LED_PRESCALER_SHIFT))
#define TCPWM_LED_PRESCALE_DIVBY64               ((uint32)(6u << TCPWM_LED_PRESCALER_SHIFT))
#define TCPWM_LED_PRESCALE_DIVBY128              ((uint32)(7u << TCPWM_LED_PRESCALER_SHIFT))

/* TCPWM set modes */
#define TCPWM_LED_MODE_TIMER_COMPARE             ((uint32)(TCPWM_LED__COMPARE         <<  \
                                                                  TCPWM_LED_MODE_SHIFT))
#define TCPWM_LED_MODE_TIMER_CAPTURE             ((uint32)(TCPWM_LED__CAPTURE         <<  \
                                                                  TCPWM_LED_MODE_SHIFT))
#define TCPWM_LED_MODE_QUAD                      ((uint32)(TCPWM_LED__QUAD            <<  \
                                                                  TCPWM_LED_MODE_SHIFT))
#define TCPWM_LED_MODE_PWM                       ((uint32)(TCPWM_LED__PWM             <<  \
                                                                  TCPWM_LED_MODE_SHIFT))
#define TCPWM_LED_MODE_PWM_DT                    ((uint32)(TCPWM_LED__PWM_DT          <<  \
                                                                  TCPWM_LED_MODE_SHIFT))
#define TCPWM_LED_MODE_PWM_PR                    ((uint32)(TCPWM_LED__PWM_PR          <<  \
                                                                  TCPWM_LED_MODE_SHIFT))

/* Quad Modes */
#define TCPWM_LED_MODE_X1                        ((uint32)(TCPWM_LED__X1              <<  \
                                                                  TCPWM_LED_QUAD_MODE_SHIFT))
#define TCPWM_LED_MODE_X2                        ((uint32)(TCPWM_LED__X2              <<  \
                                                                  TCPWM_LED_QUAD_MODE_SHIFT))
#define TCPWM_LED_MODE_X4                        ((uint32)(TCPWM_LED__X4              <<  \
                                                                  TCPWM_LED_QUAD_MODE_SHIFT))

/* Counter modes */
#define TCPWM_LED_COUNT_UP                       ((uint32)(TCPWM_LED__COUNT_UP        <<  \
                                                                  TCPWM_LED_UPDOWN_SHIFT))
#define TCPWM_LED_COUNT_DOWN                     ((uint32)(TCPWM_LED__COUNT_DOWN      <<  \
                                                                  TCPWM_LED_UPDOWN_SHIFT))
#define TCPWM_LED_COUNT_UPDOWN0                  ((uint32)(TCPWM_LED__COUNT_UPDOWN0   <<  \
                                                                  TCPWM_LED_UPDOWN_SHIFT))
#define TCPWM_LED_COUNT_UPDOWN1                  ((uint32)(TCPWM_LED__COUNT_UPDOWN1   <<  \
                                                                  TCPWM_LED_UPDOWN_SHIFT))

/* PWM output invert */
#define TCPWM_LED_INVERT_LINE                    ((uint32)(TCPWM_LED__INVERSE         <<  \
                                                                  TCPWM_LED_INV_OUT_SHIFT))
#define TCPWM_LED_INVERT_LINE_N                  ((uint32)(TCPWM_LED__INVERSE         <<  \
                                                                  TCPWM_LED_INV_COMPL_OUT_SHIFT))

/* Trigger modes */
#define TCPWM_LED_TRIG_RISING                    ((uint32)TCPWM_LED__TRIG_RISING)
#define TCPWM_LED_TRIG_FALLING                   ((uint32)TCPWM_LED__TRIG_FALLING)
#define TCPWM_LED_TRIG_BOTH                      ((uint32)TCPWM_LED__TRIG_BOTH)
#define TCPWM_LED_TRIG_LEVEL                     ((uint32)TCPWM_LED__TRIG_LEVEL)

/* Interrupt mask */
#define TCPWM_LED_INTR_MASK_TC                   ((uint32)TCPWM_LED__INTR_MASK_TC)
#define TCPWM_LED_INTR_MASK_CC_MATCH             ((uint32)TCPWM_LED__INTR_MASK_CC_MATCH)

/* PWM Output Controls */
#define TCPWM_LED_CC_MATCH_SET                   (0x00u)
#define TCPWM_LED_CC_MATCH_CLEAR                 (0x01u)
#define TCPWM_LED_CC_MATCH_INVERT                (0x02u)
#define TCPWM_LED_CC_MATCH_NO_CHANGE             (0x03u)
#define TCPWM_LED_OVERLOW_SET                    (0x00u)
#define TCPWM_LED_OVERLOW_CLEAR                  (0x04u)
#define TCPWM_LED_OVERLOW_INVERT                 (0x08u)
#define TCPWM_LED_OVERLOW_NO_CHANGE              (0x0Cu)
#define TCPWM_LED_UNDERFLOW_SET                  (0x00u)
#define TCPWM_LED_UNDERFLOW_CLEAR                (0x10u)
#define TCPWM_LED_UNDERFLOW_INVERT               (0x20u)
#define TCPWM_LED_UNDERFLOW_NO_CHANGE            (0x30u)

/* PWM Align */
#define TCPWM_LED_PWM_MODE_LEFT                  (TCPWM_LED_CC_MATCH_CLEAR        |   \
                                                         TCPWM_LED_OVERLOW_SET           |   \
                                                         TCPWM_LED_UNDERFLOW_NO_CHANGE)
#define TCPWM_LED_PWM_MODE_RIGHT                 (TCPWM_LED_CC_MATCH_SET          |   \
                                                         TCPWM_LED_OVERLOW_NO_CHANGE     |   \
                                                         TCPWM_LED_UNDERFLOW_CLEAR)
#define TCPWM_LED_PWM_MODE_ASYM                  (TCPWM_LED_CC_MATCH_INVERT       |   \
                                                         TCPWM_LED_OVERLOW_SET           |   \
                                                         TCPWM_LED_UNDERFLOW_CLEAR)

#if (TCPWM_LED_CY_TCPWM_V2)
    #if(TCPWM_LED_CY_TCPWM_4000)
        #define TCPWM_LED_PWM_MODE_CENTER                (TCPWM_LED_CC_MATCH_INVERT       |   \
                                                                 TCPWM_LED_OVERLOW_NO_CHANGE     |   \
                                                                 TCPWM_LED_UNDERFLOW_CLEAR)
    #else
        #define TCPWM_LED_PWM_MODE_CENTER                (TCPWM_LED_CC_MATCH_INVERT       |   \
                                                                 TCPWM_LED_OVERLOW_SET           |   \
                                                                 TCPWM_LED_UNDERFLOW_CLEAR)
    #endif /* (TCPWM_LED_CY_TCPWM_4000) */
#else
    #define TCPWM_LED_PWM_MODE_CENTER                (TCPWM_LED_CC_MATCH_INVERT       |   \
                                                             TCPWM_LED_OVERLOW_NO_CHANGE     |   \
                                                             TCPWM_LED_UNDERFLOW_CLEAR)
#endif /* (TCPWM_LED_CY_TCPWM_NEW) */

/* Command operations without condition */
#define TCPWM_LED_CMD_CAPTURE                    (0u)
#define TCPWM_LED_CMD_RELOAD                     (8u)
#define TCPWM_LED_CMD_STOP                       (16u)
#define TCPWM_LED_CMD_START                      (24u)

/* Status */
#define TCPWM_LED_STATUS_DOWN                    (1u)
#define TCPWM_LED_STATUS_RUNNING                 (2u)


/***************************************
*        Function Prototypes
****************************************/

void   TCPWM_LED_Init(void);
void   TCPWM_LED_Enable(void);
void   TCPWM_LED_Start(void);
void   TCPWM_LED_Stop(void);

void   TCPWM_LED_SetMode(uint32 mode);
void   TCPWM_LED_SetCounterMode(uint32 counterMode);
void   TCPWM_LED_SetPWMMode(uint32 modeMask);
void   TCPWM_LED_SetQDMode(uint32 qdMode);

void   TCPWM_LED_SetPrescaler(uint32 prescaler);
void   TCPWM_LED_TriggerCommand(uint32 mask, uint32 command);
void   TCPWM_LED_SetOneShot(uint32 oneShotEnable);
uint32 TCPWM_LED_ReadStatus(void);

void   TCPWM_LED_SetPWMSyncKill(uint32 syncKillEnable);
void   TCPWM_LED_SetPWMStopOnKill(uint32 stopOnKillEnable);
void   TCPWM_LED_SetPWMDeadTime(uint32 deadTime);
void   TCPWM_LED_SetPWMInvert(uint32 mask);

void   TCPWM_LED_SetInterruptMode(uint32 interruptMask);
uint32 TCPWM_LED_GetInterruptSourceMasked(void);
uint32 TCPWM_LED_GetInterruptSource(void);
void   TCPWM_LED_ClearInterrupt(uint32 interruptMask);
void   TCPWM_LED_SetInterrupt(uint32 interruptMask);

void   TCPWM_LED_WriteCounter(uint32 count);
uint32 TCPWM_LED_ReadCounter(void);

uint32 TCPWM_LED_ReadCapture(void);
uint32 TCPWM_LED_ReadCaptureBuf(void);

void   TCPWM_LED_WritePeriod(uint32 period);
uint32 TCPWM_LED_ReadPeriod(void);
void   TCPWM_LED_WritePeriodBuf(uint32 periodBuf);
uint32 TCPWM_LED_ReadPeriodBuf(void);

void   TCPWM_LED_WriteCompare(uint32 compare);
uint32 TCPWM_LED_ReadCompare(void);
void   TCPWM_LED_WriteCompareBuf(uint32 compareBuf);
uint32 TCPWM_LED_ReadCompareBuf(void);

void   TCPWM_LED_SetPeriodSwap(uint32 swapEnable);
void   TCPWM_LED_SetCompareSwap(uint32 swapEnable);

void   TCPWM_LED_SetCaptureMode(uint32 triggerMode);
void   TCPWM_LED_SetReloadMode(uint32 triggerMode);
void   TCPWM_LED_SetStartMode(uint32 triggerMode);
void   TCPWM_LED_SetStopMode(uint32 triggerMode);
void   TCPWM_LED_SetCountMode(uint32 triggerMode);

void   TCPWM_LED_SaveConfig(void);
void   TCPWM_LED_RestoreConfig(void);
void   TCPWM_LED_Sleep(void);
void   TCPWM_LED_Wakeup(void);


/***************************************
*             Registers
***************************************/

#define TCPWM_LED_BLOCK_CONTROL_REG              (*(reg32 *) TCPWM_LED_cy_m0s8_tcpwm_1__TCPWM_CTRL )
#define TCPWM_LED_BLOCK_CONTROL_PTR              ( (reg32 *) TCPWM_LED_cy_m0s8_tcpwm_1__TCPWM_CTRL )
#define TCPWM_LED_COMMAND_REG                    (*(reg32 *) TCPWM_LED_cy_m0s8_tcpwm_1__TCPWM_CMD )
#define TCPWM_LED_COMMAND_PTR                    ( (reg32 *) TCPWM_LED_cy_m0s8_tcpwm_1__TCPWM_CMD )
#define TCPWM_LED_INTRRUPT_CAUSE_REG             (*(reg32 *) TCPWM_LED_cy_m0s8_tcpwm_1__TCPWM_INTR_CAUSE )
#define TCPWM_LED_INTRRUPT_CAUSE_PTR             ( (reg32 *) TCPWM_LED_cy_m0s8_tcpwm_1__TCPWM_INTR_CAUSE )
#define TCPWM_LED_CONTROL_REG                    (*(reg32 *) TCPWM_LED_cy_m0s8_tcpwm_1__CTRL )
#define TCPWM_LED_CONTROL_PTR                    ( (reg32 *) TCPWM_LED_cy_m0s8_tcpwm_1__CTRL )
#define TCPWM_LED_STATUS_REG                     (*(reg32 *) TCPWM_LED_cy_m0s8_tcpwm_1__STATUS )
#define TCPWM_LED_STATUS_PTR                     ( (reg32 *) TCPWM_LED_cy_m0s8_tcpwm_1__STATUS )
#define TCPWM_LED_COUNTER_REG                    (*(reg32 *) TCPWM_LED_cy_m0s8_tcpwm_1__COUNTER )
#define TCPWM_LED_COUNTER_PTR                    ( (reg32 *) TCPWM_LED_cy_m0s8_tcpwm_1__COUNTER )
#define TCPWM_LED_COMP_CAP_REG                   (*(reg32 *) TCPWM_LED_cy_m0s8_tcpwm_1__CC )
#define TCPWM_LED_COMP_CAP_PTR                   ( (reg32 *) TCPWM_LED_cy_m0s8_tcpwm_1__CC )
#define TCPWM_LED_COMP_CAP_BUF_REG               (*(reg32 *) TCPWM_LED_cy_m0s8_tcpwm_1__CC_BUFF )
#define TCPWM_LED_COMP_CAP_BUF_PTR               ( (reg32 *) TCPWM_LED_cy_m0s8_tcpwm_1__CC_BUFF )
#define TCPWM_LED_PERIOD_REG                     (*(reg32 *) TCPWM_LED_cy_m0s8_tcpwm_1__PERIOD )
#define TCPWM_LED_PERIOD_PTR                     ( (reg32 *) TCPWM_LED_cy_m0s8_tcpwm_1__PERIOD )
#define TCPWM_LED_PERIOD_BUF_REG                 (*(reg32 *) TCPWM_LED_cy_m0s8_tcpwm_1__PERIOD_BUFF )
#define TCPWM_LED_PERIOD_BUF_PTR                 ( (reg32 *) TCPWM_LED_cy_m0s8_tcpwm_1__PERIOD_BUFF )
#define TCPWM_LED_TRIG_CONTROL0_REG              (*(reg32 *) TCPWM_LED_cy_m0s8_tcpwm_1__TR_CTRL0 )
#define TCPWM_LED_TRIG_CONTROL0_PTR              ( (reg32 *) TCPWM_LED_cy_m0s8_tcpwm_1__TR_CTRL0 )
#define TCPWM_LED_TRIG_CONTROL1_REG              (*(reg32 *) TCPWM_LED_cy_m0s8_tcpwm_1__TR_CTRL1 )
#define TCPWM_LED_TRIG_CONTROL1_PTR              ( (reg32 *) TCPWM_LED_cy_m0s8_tcpwm_1__TR_CTRL1 )
#define TCPWM_LED_TRIG_CONTROL2_REG              (*(reg32 *) TCPWM_LED_cy_m0s8_tcpwm_1__TR_CTRL2 )
#define TCPWM_LED_TRIG_CONTROL2_PTR              ( (reg32 *) TCPWM_LED_cy_m0s8_tcpwm_1__TR_CTRL2 )
#define TCPWM_LED_INTERRUPT_REQ_REG              (*(reg32 *) TCPWM_LED_cy_m0s8_tcpwm_1__INTR )
#define TCPWM_LED_INTERRUPT_REQ_PTR              ( (reg32 *) TCPWM_LED_cy_m0s8_tcpwm_1__INTR )
#define TCPWM_LED_INTERRUPT_SET_REG              (*(reg32 *) TCPWM_LED_cy_m0s8_tcpwm_1__INTR_SET )
#define TCPWM_LED_INTERRUPT_SET_PTR              ( (reg32 *) TCPWM_LED_cy_m0s8_tcpwm_1__INTR_SET )
#define TCPWM_LED_INTERRUPT_MASK_REG             (*(reg32 *) TCPWM_LED_cy_m0s8_tcpwm_1__INTR_MASK )
#define TCPWM_LED_INTERRUPT_MASK_PTR             ( (reg32 *) TCPWM_LED_cy_m0s8_tcpwm_1__INTR_MASK )
#define TCPWM_LED_INTERRUPT_MASKED_REG           (*(reg32 *) TCPWM_LED_cy_m0s8_tcpwm_1__INTR_MASKED )
#define TCPWM_LED_INTERRUPT_MASKED_PTR           ( (reg32 *) TCPWM_LED_cy_m0s8_tcpwm_1__INTR_MASKED )


/***************************************
*       Registers Constants
***************************************/

/* Mask */
#define TCPWM_LED_MASK                           ((uint32)TCPWM_LED_cy_m0s8_tcpwm_1__TCPWM_CTRL_MASK)

/* Shift constants for control register */
#define TCPWM_LED_RELOAD_CC_SHIFT                (0u)
#define TCPWM_LED_RELOAD_PERIOD_SHIFT            (1u)
#define TCPWM_LED_PWM_SYNC_KILL_SHIFT            (2u)
#define TCPWM_LED_PWM_STOP_KILL_SHIFT            (3u)
#define TCPWM_LED_PRESCALER_SHIFT                (8u)
#define TCPWM_LED_UPDOWN_SHIFT                   (16u)
#define TCPWM_LED_ONESHOT_SHIFT                  (18u)
#define TCPWM_LED_QUAD_MODE_SHIFT                (20u)
#define TCPWM_LED_INV_OUT_SHIFT                  (20u)
#define TCPWM_LED_INV_COMPL_OUT_SHIFT            (21u)
#define TCPWM_LED_MODE_SHIFT                     (24u)

/* Mask constants for control register */
#define TCPWM_LED_RELOAD_CC_MASK                 ((uint32)(TCPWM_LED_1BIT_MASK        <<  \
                                                                            TCPWM_LED_RELOAD_CC_SHIFT))
#define TCPWM_LED_RELOAD_PERIOD_MASK             ((uint32)(TCPWM_LED_1BIT_MASK        <<  \
                                                                            TCPWM_LED_RELOAD_PERIOD_SHIFT))
#define TCPWM_LED_PWM_SYNC_KILL_MASK             ((uint32)(TCPWM_LED_1BIT_MASK        <<  \
                                                                            TCPWM_LED_PWM_SYNC_KILL_SHIFT))
#define TCPWM_LED_PWM_STOP_KILL_MASK             ((uint32)(TCPWM_LED_1BIT_MASK        <<  \
                                                                            TCPWM_LED_PWM_STOP_KILL_SHIFT))
#define TCPWM_LED_PRESCALER_MASK                 ((uint32)(TCPWM_LED_8BIT_MASK        <<  \
                                                                            TCPWM_LED_PRESCALER_SHIFT))
#define TCPWM_LED_UPDOWN_MASK                    ((uint32)(TCPWM_LED_2BIT_MASK        <<  \
                                                                            TCPWM_LED_UPDOWN_SHIFT))
#define TCPWM_LED_ONESHOT_MASK                   ((uint32)(TCPWM_LED_1BIT_MASK        <<  \
                                                                            TCPWM_LED_ONESHOT_SHIFT))
#define TCPWM_LED_QUAD_MODE_MASK                 ((uint32)(TCPWM_LED_3BIT_MASK        <<  \
                                                                            TCPWM_LED_QUAD_MODE_SHIFT))
#define TCPWM_LED_INV_OUT_MASK                   ((uint32)(TCPWM_LED_2BIT_MASK        <<  \
                                                                            TCPWM_LED_INV_OUT_SHIFT))
#define TCPWM_LED_MODE_MASK                      ((uint32)(TCPWM_LED_3BIT_MASK        <<  \
                                                                            TCPWM_LED_MODE_SHIFT))

/* Shift constants for trigger control register 1 */
#define TCPWM_LED_CAPTURE_SHIFT                  (0u)
#define TCPWM_LED_COUNT_SHIFT                    (2u)
#define TCPWM_LED_RELOAD_SHIFT                   (4u)
#define TCPWM_LED_STOP_SHIFT                     (6u)
#define TCPWM_LED_START_SHIFT                    (8u)

/* Mask constants for trigger control register 1 */
#define TCPWM_LED_CAPTURE_MASK                   ((uint32)(TCPWM_LED_2BIT_MASK        <<  \
                                                                  TCPWM_LED_CAPTURE_SHIFT))
#define TCPWM_LED_COUNT_MASK                     ((uint32)(TCPWM_LED_2BIT_MASK        <<  \
                                                                  TCPWM_LED_COUNT_SHIFT))
#define TCPWM_LED_RELOAD_MASK                    ((uint32)(TCPWM_LED_2BIT_MASK        <<  \
                                                                  TCPWM_LED_RELOAD_SHIFT))
#define TCPWM_LED_STOP_MASK                      ((uint32)(TCPWM_LED_2BIT_MASK        <<  \
                                                                  TCPWM_LED_STOP_SHIFT))
#define TCPWM_LED_START_MASK                     ((uint32)(TCPWM_LED_2BIT_MASK        <<  \
                                                                  TCPWM_LED_START_SHIFT))

/* MASK */
#define TCPWM_LED_1BIT_MASK                      ((uint32)0x01u)
#define TCPWM_LED_2BIT_MASK                      ((uint32)0x03u)
#define TCPWM_LED_3BIT_MASK                      ((uint32)0x07u)
#define TCPWM_LED_6BIT_MASK                      ((uint32)0x3Fu)
#define TCPWM_LED_8BIT_MASK                      ((uint32)0xFFu)
#define TCPWM_LED_16BIT_MASK                     ((uint32)0xFFFFu)

/* Shift constant for status register */
#define TCPWM_LED_RUNNING_STATUS_SHIFT           (30u)


/***************************************
*    Initial Constants
***************************************/

#define TCPWM_LED_CTRL_QUAD_BASE_CONFIG                                                          \
        (((uint32)(TCPWM_LED_QUAD_ENCODING_MODES     << TCPWM_LED_QUAD_MODE_SHIFT))       |\
         ((uint32)(TCPWM_LED_CONFIG                  << TCPWM_LED_MODE_SHIFT)))

#define TCPWM_LED_CTRL_PWM_BASE_CONFIG                                                           \
        (((uint32)(TCPWM_LED_PWM_STOP_EVENT          << TCPWM_LED_PWM_STOP_KILL_SHIFT))   |\
         ((uint32)(TCPWM_LED_PWM_OUT_INVERT          << TCPWM_LED_INV_OUT_SHIFT))         |\
         ((uint32)(TCPWM_LED_PWM_OUT_N_INVERT        << TCPWM_LED_INV_COMPL_OUT_SHIFT))   |\
         ((uint32)(TCPWM_LED_PWM_MODE                << TCPWM_LED_MODE_SHIFT)))

#define TCPWM_LED_CTRL_PWM_RUN_MODE                                                              \
            ((uint32)(TCPWM_LED_PWM_RUN_MODE         << TCPWM_LED_ONESHOT_SHIFT))
            
#define TCPWM_LED_CTRL_PWM_ALIGN                                                                 \
            ((uint32)(TCPWM_LED_PWM_ALIGN            << TCPWM_LED_UPDOWN_SHIFT))

#define TCPWM_LED_CTRL_PWM_KILL_EVENT                                                            \
             ((uint32)(TCPWM_LED_PWM_KILL_EVENT      << TCPWM_LED_PWM_SYNC_KILL_SHIFT))

#define TCPWM_LED_CTRL_PWM_DEAD_TIME_CYCLE                                                       \
            ((uint32)(TCPWM_LED_PWM_DEAD_TIME_CYCLE  << TCPWM_LED_PRESCALER_SHIFT))

#define TCPWM_LED_CTRL_PWM_PRESCALER                                                             \
            ((uint32)(TCPWM_LED_PWM_PRESCALER        << TCPWM_LED_PRESCALER_SHIFT))

#define TCPWM_LED_CTRL_TIMER_BASE_CONFIG                                                         \
        (((uint32)(TCPWM_LED_TC_PRESCALER            << TCPWM_LED_PRESCALER_SHIFT))       |\
         ((uint32)(TCPWM_LED_TC_COUNTER_MODE         << TCPWM_LED_UPDOWN_SHIFT))          |\
         ((uint32)(TCPWM_LED_TC_RUN_MODE             << TCPWM_LED_ONESHOT_SHIFT))         |\
         ((uint32)(TCPWM_LED_TC_COMP_CAP_MODE        << TCPWM_LED_MODE_SHIFT)))
        
#define TCPWM_LED_QUAD_SIGNALS_MODES                                                             \
        (((uint32)(TCPWM_LED_QUAD_PHIA_SIGNAL_MODE   << TCPWM_LED_COUNT_SHIFT))           |\
         ((uint32)(TCPWM_LED_QUAD_INDEX_SIGNAL_MODE  << TCPWM_LED_RELOAD_SHIFT))          |\
         ((uint32)(TCPWM_LED_QUAD_STOP_SIGNAL_MODE   << TCPWM_LED_STOP_SHIFT))            |\
         ((uint32)(TCPWM_LED_QUAD_PHIB_SIGNAL_MODE   << TCPWM_LED_START_SHIFT)))

#define TCPWM_LED_PWM_SIGNALS_MODES                                                              \
        (((uint32)(TCPWM_LED_PWM_SWITCH_SIGNAL_MODE  << TCPWM_LED_CAPTURE_SHIFT))         |\
         ((uint32)(TCPWM_LED_PWM_COUNT_SIGNAL_MODE   << TCPWM_LED_COUNT_SHIFT))           |\
         ((uint32)(TCPWM_LED_PWM_RELOAD_SIGNAL_MODE  << TCPWM_LED_RELOAD_SHIFT))          |\
         ((uint32)(TCPWM_LED_PWM_STOP_SIGNAL_MODE    << TCPWM_LED_STOP_SHIFT))            |\
         ((uint32)(TCPWM_LED_PWM_START_SIGNAL_MODE   << TCPWM_LED_START_SHIFT)))

#define TCPWM_LED_TIMER_SIGNALS_MODES                                                            \
        (((uint32)(TCPWM_LED_TC_CAPTURE_SIGNAL_MODE  << TCPWM_LED_CAPTURE_SHIFT))         |\
         ((uint32)(TCPWM_LED_TC_COUNT_SIGNAL_MODE    << TCPWM_LED_COUNT_SHIFT))           |\
         ((uint32)(TCPWM_LED_TC_RELOAD_SIGNAL_MODE   << TCPWM_LED_RELOAD_SHIFT))          |\
         ((uint32)(TCPWM_LED_TC_STOP_SIGNAL_MODE     << TCPWM_LED_STOP_SHIFT))            |\
         ((uint32)(TCPWM_LED_TC_START_SIGNAL_MODE    << TCPWM_LED_START_SHIFT)))
        
#define TCPWM_LED_TIMER_UPDOWN_CNT_USED                                                          \
                ((TCPWM_LED__COUNT_UPDOWN0 == TCPWM_LED_TC_COUNTER_MODE)                  ||\
                 (TCPWM_LED__COUNT_UPDOWN1 == TCPWM_LED_TC_COUNTER_MODE))

#define TCPWM_LED_PWM_UPDOWN_CNT_USED                                                            \
                ((TCPWM_LED__CENTER == TCPWM_LED_PWM_ALIGN)                               ||\
                 (TCPWM_LED__ASYMMETRIC == TCPWM_LED_PWM_ALIGN))               
        
#define TCPWM_LED_PWM_PR_INIT_VALUE              (1u)
#define TCPWM_LED_QUAD_PERIOD_INIT_VALUE         (0x8000u)



#endif /* End CY_TCPWM_TCPWM_LED_H */

/* [] END OF FILE */
