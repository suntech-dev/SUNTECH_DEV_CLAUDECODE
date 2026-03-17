/*******************************************************************************
* File Name: TCPWM_LED.c
* Version 2.10
*
* Description:
*  This file provides the source code to the API for the TCPWM_LED
*  component
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

#include "TCPWM_LED.h"

uint8 TCPWM_LED_initVar = 0u;


/*******************************************************************************
* Function Name: TCPWM_LED_Init
********************************************************************************
*
* Summary:
*  Initialize/Restore default TCPWM_LED configuration.
*
* Parameters:
*  None
*
* Return:
*  None
*
*******************************************************************************/
void TCPWM_LED_Init(void)
{

    /* Set values from customizer to CTRL */
    #if (TCPWM_LED__QUAD == TCPWM_LED_CONFIG)
        TCPWM_LED_CONTROL_REG = TCPWM_LED_CTRL_QUAD_BASE_CONFIG;
        
        /* Set values from customizer to CTRL1 */
        TCPWM_LED_TRIG_CONTROL1_REG  = TCPWM_LED_QUAD_SIGNALS_MODES;

        /* Set values from customizer to INTR */
        TCPWM_LED_SetInterruptMode(TCPWM_LED_QUAD_INTERRUPT_MASK);
        
         /* Set other values */
        TCPWM_LED_SetCounterMode(TCPWM_LED_COUNT_DOWN);
        TCPWM_LED_WritePeriod(TCPWM_LED_QUAD_PERIOD_INIT_VALUE);
        TCPWM_LED_WriteCounter(TCPWM_LED_QUAD_PERIOD_INIT_VALUE);
    #endif  /* (TCPWM_LED__QUAD == TCPWM_LED_CONFIG) */

    #if (TCPWM_LED__TIMER == TCPWM_LED_CONFIG)
        TCPWM_LED_CONTROL_REG = TCPWM_LED_CTRL_TIMER_BASE_CONFIG;
        
        /* Set values from customizer to CTRL1 */
        TCPWM_LED_TRIG_CONTROL1_REG  = TCPWM_LED_TIMER_SIGNALS_MODES;
    
        /* Set values from customizer to INTR */
        TCPWM_LED_SetInterruptMode(TCPWM_LED_TC_INTERRUPT_MASK);
        
        /* Set other values from customizer */
        TCPWM_LED_WritePeriod(TCPWM_LED_TC_PERIOD_VALUE );

        #if (TCPWM_LED__COMPARE == TCPWM_LED_TC_COMP_CAP_MODE)
            TCPWM_LED_WriteCompare(TCPWM_LED_TC_COMPARE_VALUE);

            #if (1u == TCPWM_LED_TC_COMPARE_SWAP)
                TCPWM_LED_SetCompareSwap(1u);
                TCPWM_LED_WriteCompareBuf(TCPWM_LED_TC_COMPARE_BUF_VALUE);
            #endif  /* (1u == TCPWM_LED_TC_COMPARE_SWAP) */
        #endif  /* (TCPWM_LED__COMPARE == TCPWM_LED_TC_COMP_CAP_MODE) */

        /* Initialize counter value */
        #if (TCPWM_LED_CY_TCPWM_V2 && TCPWM_LED_TIMER_UPDOWN_CNT_USED && !TCPWM_LED_CY_TCPWM_4000)
            TCPWM_LED_WriteCounter(1u);
        #elif(TCPWM_LED__COUNT_DOWN == TCPWM_LED_TC_COUNTER_MODE)
            TCPWM_LED_WriteCounter(TCPWM_LED_TC_PERIOD_VALUE);
        #else
            TCPWM_LED_WriteCounter(0u);
        #endif /* (TCPWM_LED_CY_TCPWM_V2 && TCPWM_LED_TIMER_UPDOWN_CNT_USED && !TCPWM_LED_CY_TCPWM_4000) */
    #endif  /* (TCPWM_LED__TIMER == TCPWM_LED_CONFIG) */

    #if (TCPWM_LED__PWM_SEL == TCPWM_LED_CONFIG)
        TCPWM_LED_CONTROL_REG = TCPWM_LED_CTRL_PWM_BASE_CONFIG;

        #if (TCPWM_LED__PWM_PR == TCPWM_LED_PWM_MODE)
            TCPWM_LED_CONTROL_REG |= TCPWM_LED_CTRL_PWM_RUN_MODE;
            TCPWM_LED_WriteCounter(TCPWM_LED_PWM_PR_INIT_VALUE);
        #else
            TCPWM_LED_CONTROL_REG |= TCPWM_LED_CTRL_PWM_ALIGN | TCPWM_LED_CTRL_PWM_KILL_EVENT;
            
            /* Initialize counter value */
            #if (TCPWM_LED_CY_TCPWM_V2 && TCPWM_LED_PWM_UPDOWN_CNT_USED && !TCPWM_LED_CY_TCPWM_4000)
                TCPWM_LED_WriteCounter(1u);
            #elif (TCPWM_LED__RIGHT == TCPWM_LED_PWM_ALIGN)
                TCPWM_LED_WriteCounter(TCPWM_LED_PWM_PERIOD_VALUE);
            #else 
                TCPWM_LED_WriteCounter(0u);
            #endif  /* (TCPWM_LED_CY_TCPWM_V2 && TCPWM_LED_PWM_UPDOWN_CNT_USED && !TCPWM_LED_CY_TCPWM_4000) */
        #endif  /* (TCPWM_LED__PWM_PR == TCPWM_LED_PWM_MODE) */

        #if (TCPWM_LED__PWM_DT == TCPWM_LED_PWM_MODE)
            TCPWM_LED_CONTROL_REG |= TCPWM_LED_CTRL_PWM_DEAD_TIME_CYCLE;
        #endif  /* (TCPWM_LED__PWM_DT == TCPWM_LED_PWM_MODE) */

        #if (TCPWM_LED__PWM == TCPWM_LED_PWM_MODE)
            TCPWM_LED_CONTROL_REG |= TCPWM_LED_CTRL_PWM_PRESCALER;
        #endif  /* (TCPWM_LED__PWM == TCPWM_LED_PWM_MODE) */

        /* Set values from customizer to CTRL1 */
        TCPWM_LED_TRIG_CONTROL1_REG  = TCPWM_LED_PWM_SIGNALS_MODES;
    
        /* Set values from customizer to INTR */
        TCPWM_LED_SetInterruptMode(TCPWM_LED_PWM_INTERRUPT_MASK);

        /* Set values from customizer to CTRL2 */
        #if (TCPWM_LED__PWM_PR == TCPWM_LED_PWM_MODE)
            TCPWM_LED_TRIG_CONTROL2_REG =
                    (TCPWM_LED_CC_MATCH_NO_CHANGE    |
                    TCPWM_LED_OVERLOW_NO_CHANGE      |
                    TCPWM_LED_UNDERFLOW_NO_CHANGE);
        #else
            #if (TCPWM_LED__LEFT == TCPWM_LED_PWM_ALIGN)
                TCPWM_LED_TRIG_CONTROL2_REG = TCPWM_LED_PWM_MODE_LEFT;
            #endif  /* ( TCPWM_LED_PWM_LEFT == TCPWM_LED_PWM_ALIGN) */

            #if (TCPWM_LED__RIGHT == TCPWM_LED_PWM_ALIGN)
                TCPWM_LED_TRIG_CONTROL2_REG = TCPWM_LED_PWM_MODE_RIGHT;
            #endif  /* ( TCPWM_LED_PWM_RIGHT == TCPWM_LED_PWM_ALIGN) */

            #if (TCPWM_LED__CENTER == TCPWM_LED_PWM_ALIGN)
                TCPWM_LED_TRIG_CONTROL2_REG = TCPWM_LED_PWM_MODE_CENTER;
            #endif  /* ( TCPWM_LED_PWM_CENTER == TCPWM_LED_PWM_ALIGN) */

            #if (TCPWM_LED__ASYMMETRIC == TCPWM_LED_PWM_ALIGN)
                TCPWM_LED_TRIG_CONTROL2_REG = TCPWM_LED_PWM_MODE_ASYM;
            #endif  /* (TCPWM_LED__ASYMMETRIC == TCPWM_LED_PWM_ALIGN) */
        #endif  /* (TCPWM_LED__PWM_PR == TCPWM_LED_PWM_MODE) */

        /* Set other values from customizer */
        TCPWM_LED_WritePeriod(TCPWM_LED_PWM_PERIOD_VALUE );
        TCPWM_LED_WriteCompare(TCPWM_LED_PWM_COMPARE_VALUE);

        #if (1u == TCPWM_LED_PWM_COMPARE_SWAP)
            TCPWM_LED_SetCompareSwap(1u);
            TCPWM_LED_WriteCompareBuf(TCPWM_LED_PWM_COMPARE_BUF_VALUE);
        #endif  /* (1u == TCPWM_LED_PWM_COMPARE_SWAP) */

        #if (1u == TCPWM_LED_PWM_PERIOD_SWAP)
            TCPWM_LED_SetPeriodSwap(1u);
            TCPWM_LED_WritePeriodBuf(TCPWM_LED_PWM_PERIOD_BUF_VALUE);
        #endif  /* (1u == TCPWM_LED_PWM_PERIOD_SWAP) */
    #endif  /* (TCPWM_LED__PWM_SEL == TCPWM_LED_CONFIG) */
    
}


/*******************************************************************************
* Function Name: TCPWM_LED_Enable
********************************************************************************
*
* Summary:
*  Enables the TCPWM_LED.
*
* Parameters:
*  None
*
* Return:
*  None
*
*******************************************************************************/
void TCPWM_LED_Enable(void)
{
    uint8 enableInterrupts;

    enableInterrupts = CyEnterCriticalSection();
    TCPWM_LED_BLOCK_CONTROL_REG |= TCPWM_LED_MASK;
    CyExitCriticalSection(enableInterrupts);

    /* Start Timer or PWM if start input is absent */
    #if (TCPWM_LED__PWM_SEL == TCPWM_LED_CONFIG)
        #if (0u == TCPWM_LED_PWM_START_SIGNAL_PRESENT)
            TCPWM_LED_TriggerCommand(TCPWM_LED_MASK, TCPWM_LED_CMD_START);
        #endif /* (0u == TCPWM_LED_PWM_START_SIGNAL_PRESENT) */
    #endif /* (TCPWM_LED__PWM_SEL == TCPWM_LED_CONFIG) */

    #if (TCPWM_LED__TIMER == TCPWM_LED_CONFIG)
        #if (0u == TCPWM_LED_TC_START_SIGNAL_PRESENT)
            TCPWM_LED_TriggerCommand(TCPWM_LED_MASK, TCPWM_LED_CMD_START);
        #endif /* (0u == TCPWM_LED_TC_START_SIGNAL_PRESENT) */
    #endif /* (TCPWM_LED__TIMER == TCPWM_LED_CONFIG) */
    
    #if (TCPWM_LED__QUAD == TCPWM_LED_CONFIG)
        #if (0u != TCPWM_LED_QUAD_AUTO_START)
            TCPWM_LED_TriggerCommand(TCPWM_LED_MASK, TCPWM_LED_CMD_RELOAD);
        #endif /* (0u != TCPWM_LED_QUAD_AUTO_START) */
    #endif  /* (TCPWM_LED__QUAD == TCPWM_LED_CONFIG) */
}


/*******************************************************************************
* Function Name: TCPWM_LED_Start
********************************************************************************
*
* Summary:
*  Initializes the TCPWM_LED with default customizer
*  values when called the first time and enables the TCPWM_LED.
*  For subsequent calls the configuration is left unchanged and the component is
*  just enabled.
*
* Parameters:
*  None
*
* Return:
*  None
*
* Global variables:
*  TCPWM_LED_initVar: global variable is used to indicate initial
*  configuration of this component.  The variable is initialized to zero and set
*  to 1 the first time TCPWM_LED_Start() is called. This allows
*  enabling/disabling a component without re-initialization in all subsequent
*  calls to the TCPWM_LED_Start() routine.
*
*******************************************************************************/
void TCPWM_LED_Start(void)
{
    if (0u == TCPWM_LED_initVar)
    {
        TCPWM_LED_Init();
        TCPWM_LED_initVar = 1u;
    }

    TCPWM_LED_Enable();
}


/*******************************************************************************
* Function Name: TCPWM_LED_Stop
********************************************************************************
*
* Summary:
*  Disables the TCPWM_LED.
*
* Parameters:
*  None
*
* Return:
*  None
*
*******************************************************************************/
void TCPWM_LED_Stop(void)
{
    uint8 enableInterrupts;

    enableInterrupts = CyEnterCriticalSection();

    TCPWM_LED_BLOCK_CONTROL_REG &= (uint32)~TCPWM_LED_MASK;

    CyExitCriticalSection(enableInterrupts);
}


/*******************************************************************************
* Function Name: TCPWM_LED_SetMode
********************************************************************************
*
* Summary:
*  Sets the operation mode of the TCPWM_LED. This function is used when
*  configured as a generic TCPWM_LED and the actual mode of operation is
*  set at runtime. The mode must be set while the component is disabled.
*
* Parameters:
*  mode: Mode for the TCPWM_LED to operate in
*   Values:
*   - TCPWM_LED_MODE_TIMER_COMPARE - Timer / Counter with
*                                                 compare capability
*         - TCPWM_LED_MODE_TIMER_CAPTURE - Timer / Counter with
*                                                 capture capability
*         - TCPWM_LED_MODE_QUAD - Quadrature decoder
*         - TCPWM_LED_MODE_PWM - PWM
*         - TCPWM_LED_MODE_PWM_DT - PWM with dead time
*         - TCPWM_LED_MODE_PWM_PR - PWM with pseudo random capability
*
* Return:
*  None
*
*******************************************************************************/
void TCPWM_LED_SetMode(uint32 mode)
{
    uint8 enableInterrupts;

    enableInterrupts = CyEnterCriticalSection();

    TCPWM_LED_CONTROL_REG &= (uint32)~TCPWM_LED_MODE_MASK;
    TCPWM_LED_CONTROL_REG |= mode;

    CyExitCriticalSection(enableInterrupts);
}


/*******************************************************************************
* Function Name: TCPWM_LED_SetQDMode
********************************************************************************
*
* Summary:
*  Sets the the Quadrature Decoder to one of the 3 supported modes.
*  Its functionality is only applicable to Quadrature Decoder operation.
*
* Parameters:
*  qdMode: Quadrature Decoder mode
*   Values:
*         - TCPWM_LED_MODE_X1 - Counts on phi 1 rising
*         - TCPWM_LED_MODE_X2 - Counts on both edges of phi1 (2x faster)
*         - TCPWM_LED_MODE_X4 - Counts on both edges of phi1 and phi2
*                                        (4x faster)
*
* Return:
*  None
*
*******************************************************************************/
void TCPWM_LED_SetQDMode(uint32 qdMode)
{
    uint8 enableInterrupts;

    enableInterrupts = CyEnterCriticalSection();

    TCPWM_LED_CONTROL_REG &= (uint32)~TCPWM_LED_QUAD_MODE_MASK;
    TCPWM_LED_CONTROL_REG |= qdMode;

    CyExitCriticalSection(enableInterrupts);
}


/*******************************************************************************
* Function Name: TCPWM_LED_SetPrescaler
********************************************************************************
*
* Summary:
*  Sets the prescaler value that is applied to the clock input.  Not applicable
*  to a PWM with the dead time mode or Quadrature Decoder mode.
*
* Parameters:
*  prescaler: Prescaler divider value
*   Values:
*         - TCPWM_LED_PRESCALE_DIVBY1    - Divide by 1 (no prescaling)
*         - TCPWM_LED_PRESCALE_DIVBY2    - Divide by 2
*         - TCPWM_LED_PRESCALE_DIVBY4    - Divide by 4
*         - TCPWM_LED_PRESCALE_DIVBY8    - Divide by 8
*         - TCPWM_LED_PRESCALE_DIVBY16   - Divide by 16
*         - TCPWM_LED_PRESCALE_DIVBY32   - Divide by 32
*         - TCPWM_LED_PRESCALE_DIVBY64   - Divide by 64
*         - TCPWM_LED_PRESCALE_DIVBY128  - Divide by 128
*
* Return:
*  None
*
*******************************************************************************/
void TCPWM_LED_SetPrescaler(uint32 prescaler)
{
    uint8 enableInterrupts;

    enableInterrupts = CyEnterCriticalSection();

    TCPWM_LED_CONTROL_REG &= (uint32)~TCPWM_LED_PRESCALER_MASK;
    TCPWM_LED_CONTROL_REG |= prescaler;

    CyExitCriticalSection(enableInterrupts);
}


/*******************************************************************************
* Function Name: TCPWM_LED_SetOneShot
********************************************************************************
*
* Summary:
*  Writes the register that controls whether the TCPWM_LED runs
*  continuously or stops when terminal count is reached.  By default the
*  TCPWM_LED operates in the continuous mode.
*
* Parameters:
*  oneShotEnable
*   Values:
*     - 0 - Continuous
*     - 1 - Enable One Shot
*
* Return:
*  None
*
*******************************************************************************/
void TCPWM_LED_SetOneShot(uint32 oneShotEnable)
{
    uint8 enableInterrupts;

    enableInterrupts = CyEnterCriticalSection();

    TCPWM_LED_CONTROL_REG &= (uint32)~TCPWM_LED_ONESHOT_MASK;
    TCPWM_LED_CONTROL_REG |= ((uint32)((oneShotEnable & TCPWM_LED_1BIT_MASK) <<
                                                               TCPWM_LED_ONESHOT_SHIFT));

    CyExitCriticalSection(enableInterrupts);
}


/*******************************************************************************
* Function Name: TCPWM_LED_SetPWMMode
********************************************************************************
*
* Summary:
*  Writes the control register that determines what mode of operation the PWM
*  output lines are driven in.  There is a setting for what to do on a
*  comparison match (CC_MATCH), on an overflow (OVERFLOW) and on an underflow
*  (UNDERFLOW).  The value for each of the three must be ORed together to form
*  the mode.
*
* Parameters:
*  modeMask: A combination of three mode settings.  Mask must include a value
*  for each of the three or use one of the preconfigured PWM settings.
*   Values:
*     - CC_MATCH_SET        - Set on comparison match
*     - CC_MATCH_CLEAR      - Clear on comparison match
*     - CC_MATCH_INVERT     - Invert on comparison match
*     - CC_MATCH_NO_CHANGE  - No change on comparison match
*     - OVERLOW_SET         - Set on overflow
*     - OVERLOW_CLEAR       - Clear on  overflow
*     - OVERLOW_INVERT      - Invert on overflow
*     - OVERLOW_NO_CHANGE   - No change on overflow
*     - UNDERFLOW_SET       - Set on underflow
*     - UNDERFLOW_CLEAR     - Clear on underflow
*     - UNDERFLOW_INVERT    - Invert on underflow
*     - UNDERFLOW_NO_CHANGE - No change on underflow
*     - PWM_MODE_LEFT       - Setting for left aligned PWM.  Should be combined
*                             with up counting mode
*     - PWM_MODE_RIGHT      - Setting for right aligned PWM.  Should be combined
*                             with down counting mode
*     - PWM_MODE_CENTER     - Setting for center aligned PWM.  Should be
*                             combined with up/down 0 mode
*     - PWM_MODE_ASYM       - Setting for asymmetric PWM.  Should be combined
*                             with up/down 1 mode
*
* Return:
*  None
*
*******************************************************************************/
void TCPWM_LED_SetPWMMode(uint32 modeMask)
{
    TCPWM_LED_TRIG_CONTROL2_REG = (modeMask & TCPWM_LED_6BIT_MASK);
}



/*******************************************************************************
* Function Name: TCPWM_LED_SetPWMSyncKill
********************************************************************************
*
* Summary:
*  Writes the register that controls whether the PWM kill signal (stop input)
*  causes asynchronous or synchronous kill operation.  By default the kill
*  operation is asynchronous.  This functionality is only applicable to the PWM
*  and PWM with dead time modes.
*
*  For Synchronous mode the kill signal disables both the line and line_n
*  signals until the next terminal count.
*
*  For Asynchronous mode the kill signal disables both the line and line_n
*  signals when the kill signal is present.  This mode should only be used
*  when the kill signal (stop input) is configured in the pass through mode
*  (Level sensitive signal).

*
* Parameters:
*  syncKillEnable
*   Values:
*     - 0 - Asynchronous
*     - 1 - Synchronous
*
* Return:
*  None
*
*******************************************************************************/
void TCPWM_LED_SetPWMSyncKill(uint32 syncKillEnable)
{
    uint8 enableInterrupts;

    enableInterrupts = CyEnterCriticalSection();

    TCPWM_LED_CONTROL_REG &= (uint32)~TCPWM_LED_PWM_SYNC_KILL_MASK;
    TCPWM_LED_CONTROL_REG |= ((uint32)((syncKillEnable & TCPWM_LED_1BIT_MASK)  <<
                                               TCPWM_LED_PWM_SYNC_KILL_SHIFT));

    CyExitCriticalSection(enableInterrupts);
}


/*******************************************************************************
* Function Name: TCPWM_LED_SetPWMStopOnKill
********************************************************************************
*
* Summary:
*  Writes the register that controls whether the PWM kill signal (stop input)
*  causes the PWM counter to stop.  By default the kill operation does not stop
*  the counter.  This functionality is only applicable to the three PWM modes.
*
*
* Parameters:
*  stopOnKillEnable
*   Values:
*     - 0 - Don't stop
*     - 1 - Stop
*
* Return:
*  None
*
*******************************************************************************/
void TCPWM_LED_SetPWMStopOnKill(uint32 stopOnKillEnable)
{
    uint8 enableInterrupts;

    enableInterrupts = CyEnterCriticalSection();

    TCPWM_LED_CONTROL_REG &= (uint32)~TCPWM_LED_PWM_STOP_KILL_MASK;
    TCPWM_LED_CONTROL_REG |= ((uint32)((stopOnKillEnable & TCPWM_LED_1BIT_MASK)  <<
                                                         TCPWM_LED_PWM_STOP_KILL_SHIFT));

    CyExitCriticalSection(enableInterrupts);
}


/*******************************************************************************
* Function Name: TCPWM_LED_SetPWMDeadTime
********************************************************************************
*
* Summary:
*  Writes the dead time control value.  This value delays the rising edge of
*  both the line and line_n signals the designated number of cycles resulting
*  in both signals being inactive for that many cycles.  This functionality is
*  only applicable to the PWM in the dead time mode.

*
* Parameters:
*  Dead time to insert
*   Values: 0 to 255
*
* Return:
*  None
*
*******************************************************************************/
void TCPWM_LED_SetPWMDeadTime(uint32 deadTime)
{
    uint8 enableInterrupts;

    enableInterrupts = CyEnterCriticalSection();

    TCPWM_LED_CONTROL_REG &= (uint32)~TCPWM_LED_PRESCALER_MASK;
    TCPWM_LED_CONTROL_REG |= ((uint32)((deadTime & TCPWM_LED_8BIT_MASK) <<
                                                          TCPWM_LED_PRESCALER_SHIFT));

    CyExitCriticalSection(enableInterrupts);
}


/*******************************************************************************
* Function Name: TCPWM_LED_SetPWMInvert
********************************************************************************
*
* Summary:
*  Writes the bits that control whether the line and line_n outputs are
*  inverted from their normal output values.  This functionality is only
*  applicable to the three PWM modes.
*
* Parameters:
*  mask: Mask of outputs to invert.
*   Values:
*         - TCPWM_LED_INVERT_LINE   - Inverts the line output
*         - TCPWM_LED_INVERT_LINE_N - Inverts the line_n output
*
* Return:
*  None
*
*******************************************************************************/
void TCPWM_LED_SetPWMInvert(uint32 mask)
{
    uint8 enableInterrupts;

    enableInterrupts = CyEnterCriticalSection();

    TCPWM_LED_CONTROL_REG &= (uint32)~TCPWM_LED_INV_OUT_MASK;
    TCPWM_LED_CONTROL_REG |= mask;

    CyExitCriticalSection(enableInterrupts);
}



/*******************************************************************************
* Function Name: TCPWM_LED_WriteCounter
********************************************************************************
*
* Summary:
*  Writes a new 16bit counter value directly into the counter register, thus
*  setting the counter (not the period) to the value written. It is not
*  advised to write to this field when the counter is running.
*
* Parameters:
*  count: value to write
*
* Return:
*  None
*
*******************************************************************************/
void TCPWM_LED_WriteCounter(uint32 count)
{
    TCPWM_LED_COUNTER_REG = (count & TCPWM_LED_16BIT_MASK);
}


/*******************************************************************************
* Function Name: TCPWM_LED_ReadCounter
********************************************************************************
*
* Summary:
*  Reads the current counter value.
*
* Parameters:
*  None
*
* Return:
*  Current counter value
*
*******************************************************************************/
uint32 TCPWM_LED_ReadCounter(void)
{
    return (TCPWM_LED_COUNTER_REG & TCPWM_LED_16BIT_MASK);
}


/*******************************************************************************
* Function Name: TCPWM_LED_SetCounterMode
********************************************************************************
*
* Summary:
*  Sets the counter mode.  Applicable to all modes except Quadrature Decoder
*  and the PWM with a pseudo random output.
*
* Parameters:
*  counterMode: Enumerated counter type values
*   Values:
*     - TCPWM_LED_COUNT_UP       - Counts up
*     - TCPWM_LED_COUNT_DOWN     - Counts down
*     - TCPWM_LED_COUNT_UPDOWN0  - Counts up and down. Terminal count
*                                         generated when counter reaches 0
*     - TCPWM_LED_COUNT_UPDOWN1  - Counts up and down. Terminal count
*                                         generated both when counter reaches 0
*                                         and period
*
* Return:
*  None
*
*******************************************************************************/
void TCPWM_LED_SetCounterMode(uint32 counterMode)
{
    uint8 enableInterrupts;

    enableInterrupts = CyEnterCriticalSection();

    TCPWM_LED_CONTROL_REG &= (uint32)~TCPWM_LED_UPDOWN_MASK;
    TCPWM_LED_CONTROL_REG |= counterMode;

    CyExitCriticalSection(enableInterrupts);
}


/*******************************************************************************
* Function Name: TCPWM_LED_WritePeriod
********************************************************************************
*
* Summary:
*  Writes the 16 bit period register with the new period value.
*  To cause the counter to count for N cycles this register should be written
*  with N-1 (counts from 0 to period inclusive).
*
* Parameters:
*  period: Period value
*
* Return:
*  None
*
*******************************************************************************/
void TCPWM_LED_WritePeriod(uint32 period)
{
    TCPWM_LED_PERIOD_REG = (period & TCPWM_LED_16BIT_MASK);
}


/*******************************************************************************
* Function Name: TCPWM_LED_ReadPeriod
********************************************************************************
*
* Summary:
*  Reads the 16 bit period register.
*
* Parameters:
*  None
*
* Return:
*  Period value
*
*******************************************************************************/
uint32 TCPWM_LED_ReadPeriod(void)
{
    return (TCPWM_LED_PERIOD_REG & TCPWM_LED_16BIT_MASK);
}


/*******************************************************************************
* Function Name: TCPWM_LED_SetCompareSwap
********************************************************************************
*
* Summary:
*  Writes the register that controls whether the compare registers are
*  swapped. When enabled in the Timer/Counter mode(without capture) the swap
*  occurs at a TC event. In the PWM mode the swap occurs at the next TC event
*  following a hardware switch event.
*
* Parameters:
*  swapEnable
*   Values:
*     - 0 - Disable swap
*     - 1 - Enable swap
*
* Return:
*  None
*
*******************************************************************************/
void TCPWM_LED_SetCompareSwap(uint32 swapEnable)
{
    uint8 enableInterrupts;

    enableInterrupts = CyEnterCriticalSection();

    TCPWM_LED_CONTROL_REG &= (uint32)~TCPWM_LED_RELOAD_CC_MASK;
    TCPWM_LED_CONTROL_REG |= (swapEnable & TCPWM_LED_1BIT_MASK);

    CyExitCriticalSection(enableInterrupts);
}


/*******************************************************************************
* Function Name: TCPWM_LED_WritePeriodBuf
********************************************************************************
*
* Summary:
*  Writes the 16 bit period buf register with the new period value.
*
* Parameters:
*  periodBuf: Period value
*
* Return:
*  None
*
*******************************************************************************/
void TCPWM_LED_WritePeriodBuf(uint32 periodBuf)
{
    TCPWM_LED_PERIOD_BUF_REG = (periodBuf & TCPWM_LED_16BIT_MASK);
}


/*******************************************************************************
* Function Name: TCPWM_LED_ReadPeriodBuf
********************************************************************************
*
* Summary:
*  Reads the 16 bit period buf register.
*
* Parameters:
*  None
*
* Return:
*  Period value
*
*******************************************************************************/
uint32 TCPWM_LED_ReadPeriodBuf(void)
{
    return (TCPWM_LED_PERIOD_BUF_REG & TCPWM_LED_16BIT_MASK);
}


/*******************************************************************************
* Function Name: TCPWM_LED_SetPeriodSwap
********************************************************************************
*
* Summary:
*  Writes the register that controls whether the period registers are
*  swapped. When enabled in Timer/Counter mode the swap occurs at a TC event.
*  In the PWM mode the swap occurs at the next TC event following a hardware
*  switch event.
*
* Parameters:
*  swapEnable
*   Values:
*     - 0 - Disable swap
*     - 1 - Enable swap
*
* Return:
*  None
*
*******************************************************************************/
void TCPWM_LED_SetPeriodSwap(uint32 swapEnable)
{
    uint8 enableInterrupts;

    enableInterrupts = CyEnterCriticalSection();

    TCPWM_LED_CONTROL_REG &= (uint32)~TCPWM_LED_RELOAD_PERIOD_MASK;
    TCPWM_LED_CONTROL_REG |= ((uint32)((swapEnable & TCPWM_LED_1BIT_MASK) <<
                                                            TCPWM_LED_RELOAD_PERIOD_SHIFT));

    CyExitCriticalSection(enableInterrupts);
}


/*******************************************************************************
* Function Name: TCPWM_LED_WriteCompare
********************************************************************************
*
* Summary:
*  Writes the 16 bit compare register with the new compare value. Not
*  applicable for Timer/Counter with Capture or in Quadrature Decoder modes.
*
* Parameters:
*  compare: Compare value
*
* Return:
*  None
*
* Note:
*  It is not recommended to use the value equal to "0" or equal to 
*  "period value" in Center or Asymmetric align PWM modes on the 
*  PSoC 4100/PSoC 4200 devices.
*  PSoC 4000 devices write the 16 bit compare register with the decremented 
*  compare value in the Up counting mode (except 0x0u), and the incremented 
*  compare value in the Down counting mode (except 0xFFFFu).
*
*******************************************************************************/
void TCPWM_LED_WriteCompare(uint32 compare)
{
    #if (TCPWM_LED_CY_TCPWM_4000)
        uint32 currentMode;
    #endif /* (TCPWM_LED_CY_TCPWM_4000) */

    #if (TCPWM_LED_CY_TCPWM_4000)
        currentMode = ((TCPWM_LED_CONTROL_REG & TCPWM_LED_UPDOWN_MASK) >> TCPWM_LED_UPDOWN_SHIFT);

        if (((uint32)TCPWM_LED__COUNT_DOWN == currentMode) && (0xFFFFu != compare))
        {
            compare++;
        }
        else if (((uint32)TCPWM_LED__COUNT_UP == currentMode) && (0u != compare))
        {
            compare--;
        }
        else
        {
        }
        
    
    #endif /* (TCPWM_LED_CY_TCPWM_4000) */
    
    TCPWM_LED_COMP_CAP_REG = (compare & TCPWM_LED_16BIT_MASK);
}


/*******************************************************************************
* Function Name: TCPWM_LED_ReadCompare
********************************************************************************
*
* Summary:
*  Reads the compare register. Not applicable for Timer/Counter with Capture
*  or in Quadrature Decoder modes.
*  PSoC 4000 devices read the incremented compare register value in the 
*  Up counting mode (except 0xFFFFu), and the decremented value in the 
*  Down counting mode (except 0x0u).
*
* Parameters:
*  None
*
* Return:
*  Compare value
*
* Note:
*  PSoC 4000 devices read the incremented compare register value in the 
*  Up counting mode (except 0xFFFFu), and the decremented value in the 
*  Down counting mode (except 0x0u).
*
*******************************************************************************/
uint32 TCPWM_LED_ReadCompare(void)
{
    #if (TCPWM_LED_CY_TCPWM_4000)
        uint32 currentMode;
        uint32 regVal;
    #endif /* (TCPWM_LED_CY_TCPWM_4000) */

    #if (TCPWM_LED_CY_TCPWM_4000)
        currentMode = ((TCPWM_LED_CONTROL_REG & TCPWM_LED_UPDOWN_MASK) >> TCPWM_LED_UPDOWN_SHIFT);
        
        regVal = TCPWM_LED_COMP_CAP_REG;
        
        if (((uint32)TCPWM_LED__COUNT_DOWN == currentMode) && (0u != regVal))
        {
            regVal--;
        }
        else if (((uint32)TCPWM_LED__COUNT_UP == currentMode) && (0xFFFFu != regVal))
        {
            regVal++;
        }
        else
        {
        }

        return (regVal & TCPWM_LED_16BIT_MASK);
    #else
        return (TCPWM_LED_COMP_CAP_REG & TCPWM_LED_16BIT_MASK);
    #endif /* (TCPWM_LED_CY_TCPWM_4000) */
}


/*******************************************************************************
* Function Name: TCPWM_LED_WriteCompareBuf
********************************************************************************
*
* Summary:
*  Writes the 16 bit compare buffer register with the new compare value. Not
*  applicable for Timer/Counter with Capture or in Quadrature Decoder modes.
*
* Parameters:
*  compareBuf: Compare value
*
* Return:
*  None
*
* Note:
*  It is not recommended to use the value equal to "0" or equal to 
*  "period value" in Center or Asymmetric align PWM modes on the 
*  PSoC 4100/PSoC 4200 devices.
*  PSoC 4000 devices write the 16 bit compare register with the decremented 
*  compare value in the Up counting mode (except 0x0u), and the incremented 
*  compare value in the Down counting mode (except 0xFFFFu).
*
*******************************************************************************/
void TCPWM_LED_WriteCompareBuf(uint32 compareBuf)
{
    #if (TCPWM_LED_CY_TCPWM_4000)
        uint32 currentMode;
    #endif /* (TCPWM_LED_CY_TCPWM_4000) */

    #if (TCPWM_LED_CY_TCPWM_4000)
        currentMode = ((TCPWM_LED_CONTROL_REG & TCPWM_LED_UPDOWN_MASK) >> TCPWM_LED_UPDOWN_SHIFT);

        if (((uint32)TCPWM_LED__COUNT_DOWN == currentMode) && (0xFFFFu != compareBuf))
        {
            compareBuf++;
        }
        else if (((uint32)TCPWM_LED__COUNT_UP == currentMode) && (0u != compareBuf))
        {
            compareBuf --;
        }
        else
        {
        }
    #endif /* (TCPWM_LED_CY_TCPWM_4000) */
    
    TCPWM_LED_COMP_CAP_BUF_REG = (compareBuf & TCPWM_LED_16BIT_MASK);
}


/*******************************************************************************
* Function Name: TCPWM_LED_ReadCompareBuf
********************************************************************************
*
* Summary:
*  Reads the compare buffer register. Not applicable for Timer/Counter with
*  Capture or in Quadrature Decoder modes.
*
* Parameters:
*  None
*
* Return:
*  Compare buffer value
*
* Note:
*  PSoC 4000 devices read the incremented compare register value in the 
*  Up counting mode (except 0xFFFFu), and the decremented value in the 
*  Down counting mode (except 0x0u).
*
*******************************************************************************/
uint32 TCPWM_LED_ReadCompareBuf(void)
{
    #if (TCPWM_LED_CY_TCPWM_4000)
        uint32 currentMode;
        uint32 regVal;
    #endif /* (TCPWM_LED_CY_TCPWM_4000) */

    #if (TCPWM_LED_CY_TCPWM_4000)
        currentMode = ((TCPWM_LED_CONTROL_REG & TCPWM_LED_UPDOWN_MASK) >> TCPWM_LED_UPDOWN_SHIFT);

        regVal = TCPWM_LED_COMP_CAP_BUF_REG;
        
        if (((uint32)TCPWM_LED__COUNT_DOWN == currentMode) && (0u != regVal))
        {
            regVal--;
        }
        else if (((uint32)TCPWM_LED__COUNT_UP == currentMode) && (0xFFFFu != regVal))
        {
            regVal++;
        }
        else
        {
        }

        return (regVal & TCPWM_LED_16BIT_MASK);
    #else
        return (TCPWM_LED_COMP_CAP_BUF_REG & TCPWM_LED_16BIT_MASK);
    #endif /* (TCPWM_LED_CY_TCPWM_4000) */
}


/*******************************************************************************
* Function Name: TCPWM_LED_ReadCapture
********************************************************************************
*
* Summary:
*  Reads the captured counter value. This API is applicable only for
*  Timer/Counter with the capture mode and Quadrature Decoder modes.
*
* Parameters:
*  None
*
* Return:
*  Capture value
*
*******************************************************************************/
uint32 TCPWM_LED_ReadCapture(void)
{
    return (TCPWM_LED_COMP_CAP_REG & TCPWM_LED_16BIT_MASK);
}


/*******************************************************************************
* Function Name: TCPWM_LED_ReadCaptureBuf
********************************************************************************
*
* Summary:
*  Reads the capture buffer register. This API is applicable only for
*  Timer/Counter with the capture mode and Quadrature Decoder modes.
*
* Parameters:
*  None
*
* Return:
*  Capture buffer value
*
*******************************************************************************/
uint32 TCPWM_LED_ReadCaptureBuf(void)
{
    return (TCPWM_LED_COMP_CAP_BUF_REG & TCPWM_LED_16BIT_MASK);
}


/*******************************************************************************
* Function Name: TCPWM_LED_SetCaptureMode
********************************************************************************
*
* Summary:
*  Sets the capture trigger mode. For PWM mode this is the switch input.
*  This input is not applicable to the Timer/Counter without Capture and
*  Quadrature Decoder modes.
*
* Parameters:
*  triggerMode: Enumerated trigger mode value
*   Values:
*     - TCPWM_LED_TRIG_LEVEL     - Level
*     - TCPWM_LED_TRIG_RISING    - Rising edge
*     - TCPWM_LED_TRIG_FALLING   - Falling edge
*     - TCPWM_LED_TRIG_BOTH      - Both rising and falling edge
*
* Return:
*  None
*
*******************************************************************************/
void TCPWM_LED_SetCaptureMode(uint32 triggerMode)
{
    uint8 enableInterrupts;

    enableInterrupts = CyEnterCriticalSection();

    TCPWM_LED_TRIG_CONTROL1_REG &= (uint32)~TCPWM_LED_CAPTURE_MASK;
    TCPWM_LED_TRIG_CONTROL1_REG |= triggerMode;

    CyExitCriticalSection(enableInterrupts);
}


/*******************************************************************************
* Function Name: TCPWM_LED_SetReloadMode
********************************************************************************
*
* Summary:
*  Sets the reload trigger mode. For Quadrature Decoder mode this is the index
*  input.
*
* Parameters:
*  triggerMode: Enumerated trigger mode value
*   Values:
*     - TCPWM_LED_TRIG_LEVEL     - Level
*     - TCPWM_LED_TRIG_RISING    - Rising edge
*     - TCPWM_LED_TRIG_FALLING   - Falling edge
*     - TCPWM_LED_TRIG_BOTH      - Both rising and falling edge
*
* Return:
*  None
*
*******************************************************************************/
void TCPWM_LED_SetReloadMode(uint32 triggerMode)
{
    uint8 enableInterrupts;

    enableInterrupts = CyEnterCriticalSection();

    TCPWM_LED_TRIG_CONTROL1_REG &= (uint32)~TCPWM_LED_RELOAD_MASK;
    TCPWM_LED_TRIG_CONTROL1_REG |= ((uint32)(triggerMode << TCPWM_LED_RELOAD_SHIFT));

    CyExitCriticalSection(enableInterrupts);
}


/*******************************************************************************
* Function Name: TCPWM_LED_SetStartMode
********************************************************************************
*
* Summary:
*  Sets the start trigger mode. For Quadrature Decoder mode this is the
*  phiB input.
*
* Parameters:
*  triggerMode: Enumerated trigger mode value
*   Values:
*     - TCPWM_LED_TRIG_LEVEL     - Level
*     - TCPWM_LED_TRIG_RISING    - Rising edge
*     - TCPWM_LED_TRIG_FALLING   - Falling edge
*     - TCPWM_LED_TRIG_BOTH      - Both rising and falling edge
*
* Return:
*  None
*
*******************************************************************************/
void TCPWM_LED_SetStartMode(uint32 triggerMode)
{
    uint8 enableInterrupts;

    enableInterrupts = CyEnterCriticalSection();

    TCPWM_LED_TRIG_CONTROL1_REG &= (uint32)~TCPWM_LED_START_MASK;
    TCPWM_LED_TRIG_CONTROL1_REG |= ((uint32)(triggerMode << TCPWM_LED_START_SHIFT));

    CyExitCriticalSection(enableInterrupts);
}


/*******************************************************************************
* Function Name: TCPWM_LED_SetStopMode
********************************************************************************
*
* Summary:
*  Sets the stop trigger mode. For PWM mode this is the kill input.
*
* Parameters:
*  triggerMode: Enumerated trigger mode value
*   Values:
*     - TCPWM_LED_TRIG_LEVEL     - Level
*     - TCPWM_LED_TRIG_RISING    - Rising edge
*     - TCPWM_LED_TRIG_FALLING   - Falling edge
*     - TCPWM_LED_TRIG_BOTH      - Both rising and falling edge
*
* Return:
*  None
*
*******************************************************************************/
void TCPWM_LED_SetStopMode(uint32 triggerMode)
{
    uint8 enableInterrupts;

    enableInterrupts = CyEnterCriticalSection();

    TCPWM_LED_TRIG_CONTROL1_REG &= (uint32)~TCPWM_LED_STOP_MASK;
    TCPWM_LED_TRIG_CONTROL1_REG |= ((uint32)(triggerMode << TCPWM_LED_STOP_SHIFT));

    CyExitCriticalSection(enableInterrupts);
}


/*******************************************************************************
* Function Name: TCPWM_LED_SetCountMode
********************************************************************************
*
* Summary:
*  Sets the count trigger mode. For Quadrature Decoder mode this is the phiA
*  input.
*
* Parameters:
*  triggerMode: Enumerated trigger mode value
*   Values:
*     - TCPWM_LED_TRIG_LEVEL     - Level
*     - TCPWM_LED_TRIG_RISING    - Rising edge
*     - TCPWM_LED_TRIG_FALLING   - Falling edge
*     - TCPWM_LED_TRIG_BOTH      - Both rising and falling edge
*
* Return:
*  None
*
*******************************************************************************/
void TCPWM_LED_SetCountMode(uint32 triggerMode)
{
    uint8 enableInterrupts;

    enableInterrupts = CyEnterCriticalSection();

    TCPWM_LED_TRIG_CONTROL1_REG &= (uint32)~TCPWM_LED_COUNT_MASK;
    TCPWM_LED_TRIG_CONTROL1_REG |= ((uint32)(triggerMode << TCPWM_LED_COUNT_SHIFT));

    CyExitCriticalSection(enableInterrupts);
}


/*******************************************************************************
* Function Name: TCPWM_LED_TriggerCommand
********************************************************************************
*
* Summary:
*  Triggers the designated command to occur on the designated TCPWM instances.
*  The mask can be used to apply this command simultaneously to more than one
*  instance.  This allows multiple TCPWM instances to be synchronized.
*
* Parameters:
*  mask: A combination of mask bits for each instance of the TCPWM that the
*        command should apply to.  This function from one instance can be used
*        to apply the command to any of the instances in the design.
*        The mask value for a specific instance is available with the MASK
*        define.
*  command: Enumerated command values. Capture command only applicable for
*           Timer/Counter with Capture and PWM modes.
*   Values:
*     - TCPWM_LED_CMD_CAPTURE    - Trigger Capture/Switch command
*     - TCPWM_LED_CMD_RELOAD     - Trigger Reload/Index command
*     - TCPWM_LED_CMD_STOP       - Trigger Stop/Kill command
*     - TCPWM_LED_CMD_START      - Trigger Start/phiB command
*
* Return:
*  None
*
*******************************************************************************/
void TCPWM_LED_TriggerCommand(uint32 mask, uint32 command)
{
    uint8 enableInterrupts;

    enableInterrupts = CyEnterCriticalSection();

    TCPWM_LED_COMMAND_REG = ((uint32)(mask << command));

    CyExitCriticalSection(enableInterrupts);
}


/*******************************************************************************
* Function Name: TCPWM_LED_ReadStatus
********************************************************************************
*
* Summary:
*  Reads the status of the TCPWM_LED.
*
* Parameters:
*  None
*
* Return:
*  Status
*   Values:
*     - TCPWM_LED_STATUS_DOWN    - Set if counting down
*     - TCPWM_LED_STATUS_RUNNING - Set if counter is running
*
*******************************************************************************/
uint32 TCPWM_LED_ReadStatus(void)
{
    return ((TCPWM_LED_STATUS_REG >> TCPWM_LED_RUNNING_STATUS_SHIFT) |
            (TCPWM_LED_STATUS_REG & TCPWM_LED_STATUS_DOWN));
}


/*******************************************************************************
* Function Name: TCPWM_LED_SetInterruptMode
********************************************************************************
*
* Summary:
*  Sets the interrupt mask to control which interrupt
*  requests generate the interrupt signal.
*
* Parameters:
*   interruptMask: Mask of bits to be enabled
*   Values:
*     - TCPWM_LED_INTR_MASK_TC       - Terminal count mask
*     - TCPWM_LED_INTR_MASK_CC_MATCH - Compare count / capture mask
*
* Return:
*  None
*
*******************************************************************************/
void TCPWM_LED_SetInterruptMode(uint32 interruptMask)
{
    TCPWM_LED_INTERRUPT_MASK_REG =  interruptMask;
}


/*******************************************************************************
* Function Name: TCPWM_LED_GetInterruptSourceMasked
********************************************************************************
*
* Summary:
*  Gets the interrupt requests masked by the interrupt mask.
*
* Parameters:
*   None
*
* Return:
*  Masked interrupt source
*   Values:
*     - TCPWM_LED_INTR_MASK_TC       - Terminal count mask
*     - TCPWM_LED_INTR_MASK_CC_MATCH - Compare count / capture mask
*
*******************************************************************************/
uint32 TCPWM_LED_GetInterruptSourceMasked(void)
{
    return (TCPWM_LED_INTERRUPT_MASKED_REG);
}


/*******************************************************************************
* Function Name: TCPWM_LED_GetInterruptSource
********************************************************************************
*
* Summary:
*  Gets the interrupt requests (without masking).
*
* Parameters:
*  None
*
* Return:
*  Interrupt request value
*   Values:
*     - TCPWM_LED_INTR_MASK_TC       - Terminal count mask
*     - TCPWM_LED_INTR_MASK_CC_MATCH - Compare count / capture mask
*
*******************************************************************************/
uint32 TCPWM_LED_GetInterruptSource(void)
{
    return (TCPWM_LED_INTERRUPT_REQ_REG);
}


/*******************************************************************************
* Function Name: TCPWM_LED_ClearInterrupt
********************************************************************************
*
* Summary:
*  Clears the interrupt request.
*
* Parameters:
*   interruptMask: Mask of interrupts to clear
*   Values:
*     - TCPWM_LED_INTR_MASK_TC       - Terminal count mask
*     - TCPWM_LED_INTR_MASK_CC_MATCH - Compare count / capture mask
*
* Return:
*  None
*
*******************************************************************************/
void TCPWM_LED_ClearInterrupt(uint32 interruptMask)
{
    TCPWM_LED_INTERRUPT_REQ_REG = interruptMask;
}


/*******************************************************************************
* Function Name: TCPWM_LED_SetInterrupt
********************************************************************************
*
* Summary:
*  Sets a software interrupt request.
*
* Parameters:
*   interruptMask: Mask of interrupts to set
*   Values:
*     - TCPWM_LED_INTR_MASK_TC       - Terminal count mask
*     - TCPWM_LED_INTR_MASK_CC_MATCH - Compare count / capture mask
*
* Return:
*  None
*
*******************************************************************************/
void TCPWM_LED_SetInterrupt(uint32 interruptMask)
{
    TCPWM_LED_INTERRUPT_SET_REG = interruptMask;
}


/* [] END OF FILE */
