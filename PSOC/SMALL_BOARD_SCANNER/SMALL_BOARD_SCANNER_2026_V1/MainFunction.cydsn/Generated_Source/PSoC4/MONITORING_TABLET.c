/***************************************************************************//**
* \file MONITORING_TABLET.c
* \version 4.0
*
* \brief
*  This file provides the source code to the API for the SCB Component.
*
* Note:
*
*******************************************************************************
* \copyright
* Copyright 2013-2017, Cypress Semiconductor Corporation.  All rights reserved.
* You may use this file only in accordance with the license, terms, conditions,
* disclaimers, and limitations in the end user license agreement accompanying
* the software package with which this file was provided.
*******************************************************************************/

#include "MONITORING_TABLET_PVT.h"

#if (MONITORING_TABLET_SCB_MODE_I2C_INC)
    #include "MONITORING_TABLET_I2C_PVT.h"
#endif /* (MONITORING_TABLET_SCB_MODE_I2C_INC) */

#if (MONITORING_TABLET_SCB_MODE_EZI2C_INC)
    #include "MONITORING_TABLET_EZI2C_PVT.h"
#endif /* (MONITORING_TABLET_SCB_MODE_EZI2C_INC) */

#if (MONITORING_TABLET_SCB_MODE_SPI_INC || MONITORING_TABLET_SCB_MODE_UART_INC)
    #include "MONITORING_TABLET_SPI_UART_PVT.h"
#endif /* (MONITORING_TABLET_SCB_MODE_SPI_INC || MONITORING_TABLET_SCB_MODE_UART_INC) */


/***************************************
*    Run Time Configuration Vars
***************************************/

/* Stores internal component configuration for Unconfigured mode */
#if (MONITORING_TABLET_SCB_MODE_UNCONFIG_CONST_CFG)
    /* Common configuration variables */
    uint8 MONITORING_TABLET_scbMode = MONITORING_TABLET_SCB_MODE_UNCONFIG;
    uint8 MONITORING_TABLET_scbEnableWake;
    uint8 MONITORING_TABLET_scbEnableIntr;

    /* I2C configuration variables */
    uint8 MONITORING_TABLET_mode;
    uint8 MONITORING_TABLET_acceptAddr;

    /* SPI/UART configuration variables */
    volatile uint8 * MONITORING_TABLET_rxBuffer;
    uint8  MONITORING_TABLET_rxDataBits;
    uint32 MONITORING_TABLET_rxBufferSize;

    volatile uint8 * MONITORING_TABLET_txBuffer;
    uint8  MONITORING_TABLET_txDataBits;
    uint32 MONITORING_TABLET_txBufferSize;

    /* EZI2C configuration variables */
    uint8 MONITORING_TABLET_numberOfAddr;
    uint8 MONITORING_TABLET_subAddrSize;
#endif /* (MONITORING_TABLET_SCB_MODE_UNCONFIG_CONST_CFG) */


/***************************************
*     Common SCB Vars
***************************************/
/**
* \addtogroup group_general
* \{
*/

/** MONITORING_TABLET_initVar indicates whether the MONITORING_TABLET 
*  component has been initialized. The variable is initialized to 0 
*  and set to 1 the first time SCB_Start() is called. This allows 
*  the component to restart without reinitialization after the first 
*  call to the MONITORING_TABLET_Start() routine.
*
*  If re-initialization of the component is required, then the 
*  MONITORING_TABLET_Init() function can be called before the 
*  MONITORING_TABLET_Start() or MONITORING_TABLET_Enable() function.
*/
uint8 MONITORING_TABLET_initVar = 0u;


#if (! (MONITORING_TABLET_SCB_MODE_I2C_CONST_CFG || \
        MONITORING_TABLET_SCB_MODE_EZI2C_CONST_CFG))
    /** This global variable stores TX interrupt sources after 
    * MONITORING_TABLET_Stop() is called. Only these TX interrupt sources 
    * will be restored on a subsequent MONITORING_TABLET_Enable() call.
    */
    uint16 MONITORING_TABLET_IntrTxMask = 0u;
#endif /* (! (MONITORING_TABLET_SCB_MODE_I2C_CONST_CFG || \
              MONITORING_TABLET_SCB_MODE_EZI2C_CONST_CFG)) */
/** \} globals */

#if (MONITORING_TABLET_SCB_IRQ_INTERNAL)
#if !defined (CY_REMOVE_MONITORING_TABLET_CUSTOM_INTR_HANDLER)
    void (*MONITORING_TABLET_customIntrHandler)(void) = NULL;
#endif /* !defined (CY_REMOVE_MONITORING_TABLET_CUSTOM_INTR_HANDLER) */
#endif /* (MONITORING_TABLET_SCB_IRQ_INTERNAL) */


/***************************************
*    Private Function Prototypes
***************************************/

static void MONITORING_TABLET_ScbEnableIntr(void);
static void MONITORING_TABLET_ScbModeStop(void);
static void MONITORING_TABLET_ScbModePostEnable(void);


/*******************************************************************************
* Function Name: MONITORING_TABLET_Init
****************************************************************************//**
*
*  Initializes the MONITORING_TABLET component to operate in one of the selected
*  configurations: I2C, SPI, UART or EZI2C.
*  When the configuration is set to "Unconfigured SCB", this function does
*  not do any initialization. Use mode-specific initialization APIs instead:
*  MONITORING_TABLET_I2CInit, MONITORING_TABLET_SpiInit, 
*  MONITORING_TABLET_UartInit or MONITORING_TABLET_EzI2CInit.
*
*******************************************************************************/
void MONITORING_TABLET_Init(void)
{
#if (MONITORING_TABLET_SCB_MODE_UNCONFIG_CONST_CFG)
    if (MONITORING_TABLET_SCB_MODE_UNCONFIG_RUNTM_CFG)
    {
        MONITORING_TABLET_initVar = 0u;
    }
    else
    {
        /* Initialization was done before this function call */
    }

#elif (MONITORING_TABLET_SCB_MODE_I2C_CONST_CFG)
    MONITORING_TABLET_I2CInit();

#elif (MONITORING_TABLET_SCB_MODE_SPI_CONST_CFG)
    MONITORING_TABLET_SpiInit();

#elif (MONITORING_TABLET_SCB_MODE_UART_CONST_CFG)
    MONITORING_TABLET_UartInit();

#elif (MONITORING_TABLET_SCB_MODE_EZI2C_CONST_CFG)
    MONITORING_TABLET_EzI2CInit();

#endif /* (MONITORING_TABLET_SCB_MODE_UNCONFIG_CONST_CFG) */
}


/*******************************************************************************
* Function Name: MONITORING_TABLET_Enable
****************************************************************************//**
*
*  Enables MONITORING_TABLET component operation: activates the hardware and 
*  internal interrupt. It also restores TX interrupt sources disabled after the 
*  MONITORING_TABLET_Stop() function was called (note that level-triggered TX 
*  interrupt sources remain disabled to not cause code lock-up).
*  For I2C and EZI2C modes the interrupt is internal and mandatory for 
*  operation. For SPI and UART modes the interrupt can be configured as none, 
*  internal or external.
*  The MONITORING_TABLET configuration should be not changed when the component
*  is enabled. Any configuration changes should be made after disabling the 
*  component.
*  When configuration is set to “Unconfigured MONITORING_TABLET”, the component 
*  must first be initialized to operate in one of the following configurations: 
*  I2C, SPI, UART or EZ I2C, using the mode-specific initialization API. 
*  Otherwise this function does not enable the component.
*
*******************************************************************************/
void MONITORING_TABLET_Enable(void)
{
#if (MONITORING_TABLET_SCB_MODE_UNCONFIG_CONST_CFG)
    /* Enable SCB block, only if it is already configured */
    if (!MONITORING_TABLET_SCB_MODE_UNCONFIG_RUNTM_CFG)
    {
        MONITORING_TABLET_CTRL_REG |= MONITORING_TABLET_CTRL_ENABLED;

        MONITORING_TABLET_ScbEnableIntr();

        /* Call PostEnable function specific to current operation mode */
        MONITORING_TABLET_ScbModePostEnable();
    }
#else
    MONITORING_TABLET_CTRL_REG |= MONITORING_TABLET_CTRL_ENABLED;

    MONITORING_TABLET_ScbEnableIntr();

    /* Call PostEnable function specific to current operation mode */
    MONITORING_TABLET_ScbModePostEnable();
#endif /* (MONITORING_TABLET_SCB_MODE_UNCONFIG_CONST_CFG) */
}


/*******************************************************************************
* Function Name: MONITORING_TABLET_Start
****************************************************************************//**
*
*  Invokes MONITORING_TABLET_Init() and MONITORING_TABLET_Enable().
*  After this function call, the component is enabled and ready for operation.
*  When configuration is set to "Unconfigured SCB", the component must first be
*  initialized to operate in one of the following configurations: I2C, SPI, UART
*  or EZI2C. Otherwise this function does not enable the component.
*
* \globalvars
*  MONITORING_TABLET_initVar - used to check initial configuration, modified
*  on first function call.
*
*******************************************************************************/
void MONITORING_TABLET_Start(void)
{
    if (0u == MONITORING_TABLET_initVar)
    {
        MONITORING_TABLET_Init();
        MONITORING_TABLET_initVar = 1u; /* Component was initialized */
    }

    MONITORING_TABLET_Enable();
}


/*******************************************************************************
* Function Name: MONITORING_TABLET_Stop
****************************************************************************//**
*
*  Disables the MONITORING_TABLET component: disable the hardware and internal 
*  interrupt. It also disables all TX interrupt sources so as not to cause an 
*  unexpected interrupt trigger because after the component is enabled, the 
*  TX FIFO is empty.
*  Refer to the function MONITORING_TABLET_Enable() for the interrupt 
*  configuration details.
*  This function disables the SCB component without checking to see if 
*  communication is in progress. Before calling this function it may be 
*  necessary to check the status of communication to make sure communication 
*  is complete. If this is not done then communication could be stopped mid 
*  byte and corrupted data could result.
*
*******************************************************************************/
void MONITORING_TABLET_Stop(void)
{
#if (MONITORING_TABLET_SCB_IRQ_INTERNAL)
    MONITORING_TABLET_DisableInt();
#endif /* (MONITORING_TABLET_SCB_IRQ_INTERNAL) */

    /* Call Stop function specific to current operation mode */
    MONITORING_TABLET_ScbModeStop();

    /* Disable SCB IP */
    MONITORING_TABLET_CTRL_REG &= (uint32) ~MONITORING_TABLET_CTRL_ENABLED;

    /* Disable all TX interrupt sources so as not to cause an unexpected
    * interrupt trigger after the component will be enabled because the 
    * TX FIFO is empty.
    * For SCB IP v0, it is critical as it does not mask-out interrupt
    * sources when it is disabled. This can cause a code lock-up in the
    * interrupt handler because TX FIFO cannot be loaded after the block
    * is disabled.
    */
    MONITORING_TABLET_SetTxInterruptMode(MONITORING_TABLET_NO_INTR_SOURCES);

#if (MONITORING_TABLET_SCB_IRQ_INTERNAL)
    MONITORING_TABLET_ClearPendingInt();
#endif /* (MONITORING_TABLET_SCB_IRQ_INTERNAL) */
}


/*******************************************************************************
* Function Name: MONITORING_TABLET_SetRxFifoLevel
****************************************************************************//**
*
*  Sets level in the RX FIFO to generate a RX level interrupt.
*  When the RX FIFO has more entries than the RX FIFO level an RX level
*  interrupt request is generated.
*
*  \param level: Level in the RX FIFO to generate RX level interrupt.
*   The range of valid level values is between 0 and RX FIFO depth - 1.
*
*******************************************************************************/
void MONITORING_TABLET_SetRxFifoLevel(uint32 level)
{
    uint32 rxFifoCtrl;

    rxFifoCtrl = MONITORING_TABLET_RX_FIFO_CTRL_REG;

    rxFifoCtrl &= ((uint32) ~MONITORING_TABLET_RX_FIFO_CTRL_TRIGGER_LEVEL_MASK); /* Clear level mask bits */
    rxFifoCtrl |= ((uint32) (MONITORING_TABLET_RX_FIFO_CTRL_TRIGGER_LEVEL_MASK & level));

    MONITORING_TABLET_RX_FIFO_CTRL_REG = rxFifoCtrl;
}


/*******************************************************************************
* Function Name: MONITORING_TABLET_SetTxFifoLevel
****************************************************************************//**
*
*  Sets level in the TX FIFO to generate a TX level interrupt.
*  When the TX FIFO has less entries than the TX FIFO level an TX level
*  interrupt request is generated.
*
*  \param level: Level in the TX FIFO to generate TX level interrupt.
*   The range of valid level values is between 0 and TX FIFO depth - 1.
*
*******************************************************************************/
void MONITORING_TABLET_SetTxFifoLevel(uint32 level)
{
    uint32 txFifoCtrl;

    txFifoCtrl = MONITORING_TABLET_TX_FIFO_CTRL_REG;

    txFifoCtrl &= ((uint32) ~MONITORING_TABLET_TX_FIFO_CTRL_TRIGGER_LEVEL_MASK); /* Clear level mask bits */
    txFifoCtrl |= ((uint32) (MONITORING_TABLET_TX_FIFO_CTRL_TRIGGER_LEVEL_MASK & level));

    MONITORING_TABLET_TX_FIFO_CTRL_REG = txFifoCtrl;
}


#if (MONITORING_TABLET_SCB_IRQ_INTERNAL)
    /*******************************************************************************
    * Function Name: MONITORING_TABLET_SetCustomInterruptHandler
    ****************************************************************************//**
    *
    *  Registers a function to be called by the internal interrupt handler.
    *  First the function that is registered is called, then the internal interrupt
    *  handler performs any operation such as software buffer management functions
    *  before the interrupt returns.  It is the user's responsibility not to break
    *  the software buffer operations. Only one custom handler is supported, which
    *  is the function provided by the most recent call.
    *  At the initialization time no custom handler is registered.
    *
    *  \param func: Pointer to the function to register.
    *        The value NULL indicates to remove the current custom interrupt
    *        handler.
    *
    *******************************************************************************/
    void MONITORING_TABLET_SetCustomInterruptHandler(void (*func)(void))
    {
    #if !defined (CY_REMOVE_MONITORING_TABLET_CUSTOM_INTR_HANDLER)
        MONITORING_TABLET_customIntrHandler = func; /* Register interrupt handler */
    #else
        if (NULL != func)
        {
            /* Suppress compiler warning */
        }
    #endif /* !defined (CY_REMOVE_MONITORING_TABLET_CUSTOM_INTR_HANDLER) */
    }
#endif /* (MONITORING_TABLET_SCB_IRQ_INTERNAL) */


/*******************************************************************************
* Function Name: MONITORING_TABLET_ScbModeEnableIntr
****************************************************************************//**
*
*  Enables an interrupt for a specific mode.
*
*******************************************************************************/
static void MONITORING_TABLET_ScbEnableIntr(void)
{
#if (MONITORING_TABLET_SCB_IRQ_INTERNAL)
    /* Enable interrupt in NVIC */
    #if (MONITORING_TABLET_SCB_MODE_UNCONFIG_CONST_CFG)
        if (0u != MONITORING_TABLET_scbEnableIntr)
        {
            MONITORING_TABLET_EnableInt();
        }

    #else
        MONITORING_TABLET_EnableInt();

    #endif /* (MONITORING_TABLET_SCB_MODE_UNCONFIG_CONST_CFG) */
#endif /* (MONITORING_TABLET_SCB_IRQ_INTERNAL) */
}


/*******************************************************************************
* Function Name: MONITORING_TABLET_ScbModePostEnable
****************************************************************************//**
*
*  Calls the PostEnable function for a specific operation mode.
*
*******************************************************************************/
static void MONITORING_TABLET_ScbModePostEnable(void)
{
#if (MONITORING_TABLET_SCB_MODE_UNCONFIG_CONST_CFG)
#if (!MONITORING_TABLET_CY_SCBIP_V1)
    if (MONITORING_TABLET_SCB_MODE_SPI_RUNTM_CFG)
    {
        MONITORING_TABLET_SpiPostEnable();
    }
    else if (MONITORING_TABLET_SCB_MODE_UART_RUNTM_CFG)
    {
        MONITORING_TABLET_UartPostEnable();
    }
    else
    {
        /* Unknown mode: do nothing */
    }
#endif /* (!MONITORING_TABLET_CY_SCBIP_V1) */

#elif (MONITORING_TABLET_SCB_MODE_SPI_CONST_CFG)
    MONITORING_TABLET_SpiPostEnable();

#elif (MONITORING_TABLET_SCB_MODE_UART_CONST_CFG)
    MONITORING_TABLET_UartPostEnable();

#else
    /* Unknown mode: do nothing */
#endif /* (MONITORING_TABLET_SCB_MODE_UNCONFIG_CONST_CFG) */
}


/*******************************************************************************
* Function Name: MONITORING_TABLET_ScbModeStop
****************************************************************************//**
*
*  Calls the Stop function for a specific operation mode.
*
*******************************************************************************/
static void MONITORING_TABLET_ScbModeStop(void)
{
#if (MONITORING_TABLET_SCB_MODE_UNCONFIG_CONST_CFG)
    if (MONITORING_TABLET_SCB_MODE_I2C_RUNTM_CFG)
    {
        MONITORING_TABLET_I2CStop();
    }
    else if (MONITORING_TABLET_SCB_MODE_EZI2C_RUNTM_CFG)
    {
        MONITORING_TABLET_EzI2CStop();
    }
#if (!MONITORING_TABLET_CY_SCBIP_V1)
    else if (MONITORING_TABLET_SCB_MODE_SPI_RUNTM_CFG)
    {
        MONITORING_TABLET_SpiStop();
    }
    else if (MONITORING_TABLET_SCB_MODE_UART_RUNTM_CFG)
    {
        MONITORING_TABLET_UartStop();
    }
#endif /* (!MONITORING_TABLET_CY_SCBIP_V1) */
    else
    {
        /* Unknown mode: do nothing */
    }
#elif (MONITORING_TABLET_SCB_MODE_I2C_CONST_CFG)
    MONITORING_TABLET_I2CStop();

#elif (MONITORING_TABLET_SCB_MODE_EZI2C_CONST_CFG)
    MONITORING_TABLET_EzI2CStop();

#elif (MONITORING_TABLET_SCB_MODE_SPI_CONST_CFG)
    MONITORING_TABLET_SpiStop();

#elif (MONITORING_TABLET_SCB_MODE_UART_CONST_CFG)
    MONITORING_TABLET_UartStop();

#else
    /* Unknown mode: do nothing */
#endif /* (MONITORING_TABLET_SCB_MODE_UNCONFIG_CONST_CFG) */
}


#if (MONITORING_TABLET_SCB_MODE_UNCONFIG_CONST_CFG)
    /*******************************************************************************
    * Function Name: MONITORING_TABLET_SetPins
    ****************************************************************************//**
    *
    *  Sets the pins settings accordingly to the selected operation mode.
    *  Only available in the Unconfigured operation mode. The mode specific
    *  initialization function calls it.
    *  Pins configuration is set by PSoC Creator when a specific mode of operation
    *  is selected in design time.
    *
    *  \param mode:      Mode of SCB operation.
    *  \param subMode:   Sub-mode of SCB operation. It is only required for SPI and UART
    *             modes.
    *  \param uartEnableMask: enables TX or RX direction and RTS and CTS signals.
    *
    *******************************************************************************/
    void MONITORING_TABLET_SetPins(uint32 mode, uint32 subMode, uint32 uartEnableMask)
    {
        uint32 pinsDm[MONITORING_TABLET_SCB_PINS_NUMBER];
        uint32 i;
        
    #if (!MONITORING_TABLET_CY_SCBIP_V1)
        uint32 pinsInBuf = 0u;
    #endif /* (!MONITORING_TABLET_CY_SCBIP_V1) */
        
        uint32 hsiomSel[MONITORING_TABLET_SCB_PINS_NUMBER] = 
        {
            MONITORING_TABLET_RX_SCL_MOSI_HSIOM_SEL_GPIO,
            MONITORING_TABLET_TX_SDA_MISO_HSIOM_SEL_GPIO,
            0u,
            0u,
            0u,
            0u,
            0u,
        };

    #if (MONITORING_TABLET_CY_SCBIP_V1)
        /* Supress compiler warning. */
        if ((0u == subMode) || (0u == uartEnableMask))
        {
        }
    #endif /* (MONITORING_TABLET_CY_SCBIP_V1) */

        /* Set default HSIOM to GPIO and Drive Mode to Analog Hi-Z */
        for (i = 0u; i < MONITORING_TABLET_SCB_PINS_NUMBER; i++)
        {
            pinsDm[i] = MONITORING_TABLET_PIN_DM_ALG_HIZ;
        }

        if ((MONITORING_TABLET_SCB_MODE_I2C   == mode) ||
            (MONITORING_TABLET_SCB_MODE_EZI2C == mode))
        {
        #if (MONITORING_TABLET_RX_SCL_MOSI_PIN)
            hsiomSel[MONITORING_TABLET_RX_SCL_MOSI_PIN_INDEX] = MONITORING_TABLET_RX_SCL_MOSI_HSIOM_SEL_I2C;
            pinsDm  [MONITORING_TABLET_RX_SCL_MOSI_PIN_INDEX] = MONITORING_TABLET_PIN_DM_OD_LO;
        #elif (MONITORING_TABLET_RX_WAKE_SCL_MOSI_PIN)
            hsiomSel[MONITORING_TABLET_RX_WAKE_SCL_MOSI_PIN_INDEX] = MONITORING_TABLET_RX_WAKE_SCL_MOSI_HSIOM_SEL_I2C;
            pinsDm  [MONITORING_TABLET_RX_WAKE_SCL_MOSI_PIN_INDEX] = MONITORING_TABLET_PIN_DM_OD_LO;
        #else
        #endif /* (MONITORING_TABLET_RX_SCL_MOSI_PIN) */
        
        #if (MONITORING_TABLET_TX_SDA_MISO_PIN)
            hsiomSel[MONITORING_TABLET_TX_SDA_MISO_PIN_INDEX] = MONITORING_TABLET_TX_SDA_MISO_HSIOM_SEL_I2C;
            pinsDm  [MONITORING_TABLET_TX_SDA_MISO_PIN_INDEX] = MONITORING_TABLET_PIN_DM_OD_LO;
        #endif /* (MONITORING_TABLET_TX_SDA_MISO_PIN) */
        }
    #if (!MONITORING_TABLET_CY_SCBIP_V1)
        else if (MONITORING_TABLET_SCB_MODE_SPI == mode)
        {
        #if (MONITORING_TABLET_RX_SCL_MOSI_PIN)
            hsiomSel[MONITORING_TABLET_RX_SCL_MOSI_PIN_INDEX] = MONITORING_TABLET_RX_SCL_MOSI_HSIOM_SEL_SPI;
        #elif (MONITORING_TABLET_RX_WAKE_SCL_MOSI_PIN)
            hsiomSel[MONITORING_TABLET_RX_WAKE_SCL_MOSI_PIN_INDEX] = MONITORING_TABLET_RX_WAKE_SCL_MOSI_HSIOM_SEL_SPI;
        #else
        #endif /* (MONITORING_TABLET_RX_SCL_MOSI_PIN) */
        
        #if (MONITORING_TABLET_TX_SDA_MISO_PIN)
            hsiomSel[MONITORING_TABLET_TX_SDA_MISO_PIN_INDEX] = MONITORING_TABLET_TX_SDA_MISO_HSIOM_SEL_SPI;
        #endif /* (MONITORING_TABLET_TX_SDA_MISO_PIN) */
        
        #if (MONITORING_TABLET_CTS_SCLK_PIN)
            hsiomSel[MONITORING_TABLET_CTS_SCLK_PIN_INDEX] = MONITORING_TABLET_CTS_SCLK_HSIOM_SEL_SPI;
        #endif /* (MONITORING_TABLET_CTS_SCLK_PIN) */

            if (MONITORING_TABLET_SPI_SLAVE == subMode)
            {
                /* Slave */
                pinsDm[MONITORING_TABLET_RX_SCL_MOSI_PIN_INDEX] = MONITORING_TABLET_PIN_DM_DIG_HIZ;
                pinsDm[MONITORING_TABLET_TX_SDA_MISO_PIN_INDEX] = MONITORING_TABLET_PIN_DM_STRONG;
                pinsDm[MONITORING_TABLET_CTS_SCLK_PIN_INDEX] = MONITORING_TABLET_PIN_DM_DIG_HIZ;

            #if (MONITORING_TABLET_RTS_SS0_PIN)
                /* Only SS0 is valid choice for Slave */
                hsiomSel[MONITORING_TABLET_RTS_SS0_PIN_INDEX] = MONITORING_TABLET_RTS_SS0_HSIOM_SEL_SPI;
                pinsDm  [MONITORING_TABLET_RTS_SS0_PIN_INDEX] = MONITORING_TABLET_PIN_DM_DIG_HIZ;
            #endif /* (MONITORING_TABLET_RTS_SS0_PIN) */

            #if (MONITORING_TABLET_TX_SDA_MISO_PIN)
                /* Disable input buffer */
                 pinsInBuf |= MONITORING_TABLET_TX_SDA_MISO_PIN_MASK;
            #endif /* (MONITORING_TABLET_TX_SDA_MISO_PIN) */
            }
            else 
            {
                /* (Master) */
                pinsDm[MONITORING_TABLET_RX_SCL_MOSI_PIN_INDEX] = MONITORING_TABLET_PIN_DM_STRONG;
                pinsDm[MONITORING_TABLET_TX_SDA_MISO_PIN_INDEX] = MONITORING_TABLET_PIN_DM_DIG_HIZ;
                pinsDm[MONITORING_TABLET_CTS_SCLK_PIN_INDEX] = MONITORING_TABLET_PIN_DM_STRONG;

            #if (MONITORING_TABLET_RTS_SS0_PIN)
                hsiomSel [MONITORING_TABLET_RTS_SS0_PIN_INDEX] = MONITORING_TABLET_RTS_SS0_HSIOM_SEL_SPI;
                pinsDm   [MONITORING_TABLET_RTS_SS0_PIN_INDEX] = MONITORING_TABLET_PIN_DM_STRONG;
                pinsInBuf |= MONITORING_TABLET_RTS_SS0_PIN_MASK;
            #endif /* (MONITORING_TABLET_RTS_SS0_PIN) */

            #if (MONITORING_TABLET_SS1_PIN)
                hsiomSel [MONITORING_TABLET_SS1_PIN_INDEX] = MONITORING_TABLET_SS1_HSIOM_SEL_SPI;
                pinsDm   [MONITORING_TABLET_SS1_PIN_INDEX] = MONITORING_TABLET_PIN_DM_STRONG;
                pinsInBuf |= MONITORING_TABLET_SS1_PIN_MASK;
            #endif /* (MONITORING_TABLET_SS1_PIN) */

            #if (MONITORING_TABLET_SS2_PIN)
                hsiomSel [MONITORING_TABLET_SS2_PIN_INDEX] = MONITORING_TABLET_SS2_HSIOM_SEL_SPI;
                pinsDm   [MONITORING_TABLET_SS2_PIN_INDEX] = MONITORING_TABLET_PIN_DM_STRONG;
                pinsInBuf |= MONITORING_TABLET_SS2_PIN_MASK;
            #endif /* (MONITORING_TABLET_SS2_PIN) */

            #if (MONITORING_TABLET_SS3_PIN)
                hsiomSel [MONITORING_TABLET_SS3_PIN_INDEX] = MONITORING_TABLET_SS3_HSIOM_SEL_SPI;
                pinsDm   [MONITORING_TABLET_SS3_PIN_INDEX] = MONITORING_TABLET_PIN_DM_STRONG;
                pinsInBuf |= MONITORING_TABLET_SS3_PIN_MASK;
            #endif /* (MONITORING_TABLET_SS3_PIN) */

                /* Disable input buffers */
            #if (MONITORING_TABLET_RX_SCL_MOSI_PIN)
                pinsInBuf |= MONITORING_TABLET_RX_SCL_MOSI_PIN_MASK;
            #elif (MONITORING_TABLET_RX_WAKE_SCL_MOSI_PIN)
                pinsInBuf |= MONITORING_TABLET_RX_WAKE_SCL_MOSI_PIN_MASK;
            #else
            #endif /* (MONITORING_TABLET_RX_SCL_MOSI_PIN) */

            #if (MONITORING_TABLET_CTS_SCLK_PIN)
                pinsInBuf |= MONITORING_TABLET_CTS_SCLK_PIN_MASK;
            #endif /* (MONITORING_TABLET_CTS_SCLK_PIN) */
            }
        }
        else /* UART */
        {
            if (MONITORING_TABLET_UART_MODE_SMARTCARD == subMode)
            {
                /* SmartCard */
            #if (MONITORING_TABLET_TX_SDA_MISO_PIN)
                hsiomSel[MONITORING_TABLET_TX_SDA_MISO_PIN_INDEX] = MONITORING_TABLET_TX_SDA_MISO_HSIOM_SEL_UART;
                pinsDm  [MONITORING_TABLET_TX_SDA_MISO_PIN_INDEX] = MONITORING_TABLET_PIN_DM_OD_LO;
            #endif /* (MONITORING_TABLET_TX_SDA_MISO_PIN) */
            }
            else /* Standard or IrDA */
            {
                if (0u != (MONITORING_TABLET_UART_RX_PIN_ENABLE & uartEnableMask))
                {
                #if (MONITORING_TABLET_RX_SCL_MOSI_PIN)
                    hsiomSel[MONITORING_TABLET_RX_SCL_MOSI_PIN_INDEX] = MONITORING_TABLET_RX_SCL_MOSI_HSIOM_SEL_UART;
                    pinsDm  [MONITORING_TABLET_RX_SCL_MOSI_PIN_INDEX] = MONITORING_TABLET_PIN_DM_DIG_HIZ;
                #elif (MONITORING_TABLET_RX_WAKE_SCL_MOSI_PIN)
                    hsiomSel[MONITORING_TABLET_RX_WAKE_SCL_MOSI_PIN_INDEX] = MONITORING_TABLET_RX_WAKE_SCL_MOSI_HSIOM_SEL_UART;
                    pinsDm  [MONITORING_TABLET_RX_WAKE_SCL_MOSI_PIN_INDEX] = MONITORING_TABLET_PIN_DM_DIG_HIZ;
                #else
                #endif /* (MONITORING_TABLET_RX_SCL_MOSI_PIN) */
                }

                if (0u != (MONITORING_TABLET_UART_TX_PIN_ENABLE & uartEnableMask))
                {
                #if (MONITORING_TABLET_TX_SDA_MISO_PIN)
                    hsiomSel[MONITORING_TABLET_TX_SDA_MISO_PIN_INDEX] = MONITORING_TABLET_TX_SDA_MISO_HSIOM_SEL_UART;
                    pinsDm  [MONITORING_TABLET_TX_SDA_MISO_PIN_INDEX] = MONITORING_TABLET_PIN_DM_STRONG;
                    
                    /* Disable input buffer */
                    pinsInBuf |= MONITORING_TABLET_TX_SDA_MISO_PIN_MASK;
                #endif /* (MONITORING_TABLET_TX_SDA_MISO_PIN) */
                }

            #if !(MONITORING_TABLET_CY_SCBIP_V0 || MONITORING_TABLET_CY_SCBIP_V1)
                if (MONITORING_TABLET_UART_MODE_STD == subMode)
                {
                    if (0u != (MONITORING_TABLET_UART_CTS_PIN_ENABLE & uartEnableMask))
                    {
                        /* CTS input is multiplexed with SCLK */
                    #if (MONITORING_TABLET_CTS_SCLK_PIN)
                        hsiomSel[MONITORING_TABLET_CTS_SCLK_PIN_INDEX] = MONITORING_TABLET_CTS_SCLK_HSIOM_SEL_UART;
                        pinsDm  [MONITORING_TABLET_CTS_SCLK_PIN_INDEX] = MONITORING_TABLET_PIN_DM_DIG_HIZ;
                    #endif /* (MONITORING_TABLET_CTS_SCLK_PIN) */
                    }

                    if (0u != (MONITORING_TABLET_UART_RTS_PIN_ENABLE & uartEnableMask))
                    {
                        /* RTS output is multiplexed with SS0 */
                    #if (MONITORING_TABLET_RTS_SS0_PIN)
                        hsiomSel[MONITORING_TABLET_RTS_SS0_PIN_INDEX] = MONITORING_TABLET_RTS_SS0_HSIOM_SEL_UART;
                        pinsDm  [MONITORING_TABLET_RTS_SS0_PIN_INDEX] = MONITORING_TABLET_PIN_DM_STRONG;
                        
                        /* Disable input buffer */
                        pinsInBuf |= MONITORING_TABLET_RTS_SS0_PIN_MASK;
                    #endif /* (MONITORING_TABLET_RTS_SS0_PIN) */
                    }
                }
            #endif /* !(MONITORING_TABLET_CY_SCBIP_V0 || MONITORING_TABLET_CY_SCBIP_V1) */
            }
        }
    #endif /* (!MONITORING_TABLET_CY_SCBIP_V1) */

    /* Configure pins: set HSIOM, DM and InputBufEnable */
    /* Note: the DR register settings do not effect the pin output if HSIOM is other than GPIO */

    #if (MONITORING_TABLET_RX_SCL_MOSI_PIN)
        MONITORING_TABLET_SET_HSIOM_SEL(MONITORING_TABLET_RX_SCL_MOSI_HSIOM_REG,
                                       MONITORING_TABLET_RX_SCL_MOSI_HSIOM_MASK,
                                       MONITORING_TABLET_RX_SCL_MOSI_HSIOM_POS,
                                        hsiomSel[MONITORING_TABLET_RX_SCL_MOSI_PIN_INDEX]);

        MONITORING_TABLET_uart_rx_i2c_scl_spi_mosi_SetDriveMode((uint8) pinsDm[MONITORING_TABLET_RX_SCL_MOSI_PIN_INDEX]);

        #if (!MONITORING_TABLET_CY_SCBIP_V1)
            MONITORING_TABLET_SET_INP_DIS(MONITORING_TABLET_uart_rx_i2c_scl_spi_mosi_INP_DIS,
                                         MONITORING_TABLET_uart_rx_i2c_scl_spi_mosi_MASK,
                                         (0u != (pinsInBuf & MONITORING_TABLET_RX_SCL_MOSI_PIN_MASK)));
        #endif /* (!MONITORING_TABLET_CY_SCBIP_V1) */
    
    #elif (MONITORING_TABLET_RX_WAKE_SCL_MOSI_PIN)
        MONITORING_TABLET_SET_HSIOM_SEL(MONITORING_TABLET_RX_WAKE_SCL_MOSI_HSIOM_REG,
                                       MONITORING_TABLET_RX_WAKE_SCL_MOSI_HSIOM_MASK,
                                       MONITORING_TABLET_RX_WAKE_SCL_MOSI_HSIOM_POS,
                                       hsiomSel[MONITORING_TABLET_RX_WAKE_SCL_MOSI_PIN_INDEX]);

        MONITORING_TABLET_uart_rx_wake_i2c_scl_spi_mosi_SetDriveMode((uint8)
                                                               pinsDm[MONITORING_TABLET_RX_WAKE_SCL_MOSI_PIN_INDEX]);

        MONITORING_TABLET_SET_INP_DIS(MONITORING_TABLET_uart_rx_wake_i2c_scl_spi_mosi_INP_DIS,
                                     MONITORING_TABLET_uart_rx_wake_i2c_scl_spi_mosi_MASK,
                                     (0u != (pinsInBuf & MONITORING_TABLET_RX_WAKE_SCL_MOSI_PIN_MASK)));

         /* Set interrupt on falling edge */
        MONITORING_TABLET_SET_INCFG_TYPE(MONITORING_TABLET_RX_WAKE_SCL_MOSI_INTCFG_REG,
                                        MONITORING_TABLET_RX_WAKE_SCL_MOSI_INTCFG_TYPE_MASK,
                                        MONITORING_TABLET_RX_WAKE_SCL_MOSI_INTCFG_TYPE_POS,
                                        MONITORING_TABLET_INTCFG_TYPE_FALLING_EDGE);
    #else
    #endif /* (MONITORING_TABLET_RX_WAKE_SCL_MOSI_PIN) */

    #if (MONITORING_TABLET_TX_SDA_MISO_PIN)
        MONITORING_TABLET_SET_HSIOM_SEL(MONITORING_TABLET_TX_SDA_MISO_HSIOM_REG,
                                       MONITORING_TABLET_TX_SDA_MISO_HSIOM_MASK,
                                       MONITORING_TABLET_TX_SDA_MISO_HSIOM_POS,
                                        hsiomSel[MONITORING_TABLET_TX_SDA_MISO_PIN_INDEX]);

        MONITORING_TABLET_uart_tx_i2c_sda_spi_miso_SetDriveMode((uint8) pinsDm[MONITORING_TABLET_TX_SDA_MISO_PIN_INDEX]);

    #if (!MONITORING_TABLET_CY_SCBIP_V1)
        MONITORING_TABLET_SET_INP_DIS(MONITORING_TABLET_uart_tx_i2c_sda_spi_miso_INP_DIS,
                                     MONITORING_TABLET_uart_tx_i2c_sda_spi_miso_MASK,
                                    (0u != (pinsInBuf & MONITORING_TABLET_TX_SDA_MISO_PIN_MASK)));
    #endif /* (!MONITORING_TABLET_CY_SCBIP_V1) */
    #endif /* (MONITORING_TABLET_RX_SCL_MOSI_PIN) */

    #if (MONITORING_TABLET_CTS_SCLK_PIN)
        MONITORING_TABLET_SET_HSIOM_SEL(MONITORING_TABLET_CTS_SCLK_HSIOM_REG,
                                       MONITORING_TABLET_CTS_SCLK_HSIOM_MASK,
                                       MONITORING_TABLET_CTS_SCLK_HSIOM_POS,
                                       hsiomSel[MONITORING_TABLET_CTS_SCLK_PIN_INDEX]);

        MONITORING_TABLET_uart_cts_spi_sclk_SetDriveMode((uint8) pinsDm[MONITORING_TABLET_CTS_SCLK_PIN_INDEX]);

        MONITORING_TABLET_SET_INP_DIS(MONITORING_TABLET_uart_cts_spi_sclk_INP_DIS,
                                     MONITORING_TABLET_uart_cts_spi_sclk_MASK,
                                     (0u != (pinsInBuf & MONITORING_TABLET_CTS_SCLK_PIN_MASK)));
    #endif /* (MONITORING_TABLET_CTS_SCLK_PIN) */

    #if (MONITORING_TABLET_RTS_SS0_PIN)
        MONITORING_TABLET_SET_HSIOM_SEL(MONITORING_TABLET_RTS_SS0_HSIOM_REG,
                                       MONITORING_TABLET_RTS_SS0_HSIOM_MASK,
                                       MONITORING_TABLET_RTS_SS0_HSIOM_POS,
                                       hsiomSel[MONITORING_TABLET_RTS_SS0_PIN_INDEX]);

        MONITORING_TABLET_uart_rts_spi_ss0_SetDriveMode((uint8) pinsDm[MONITORING_TABLET_RTS_SS0_PIN_INDEX]);

        MONITORING_TABLET_SET_INP_DIS(MONITORING_TABLET_uart_rts_spi_ss0_INP_DIS,
                                     MONITORING_TABLET_uart_rts_spi_ss0_MASK,
                                     (0u != (pinsInBuf & MONITORING_TABLET_RTS_SS0_PIN_MASK)));
    #endif /* (MONITORING_TABLET_RTS_SS0_PIN) */

    #if (MONITORING_TABLET_SS1_PIN)
        MONITORING_TABLET_SET_HSIOM_SEL(MONITORING_TABLET_SS1_HSIOM_REG,
                                       MONITORING_TABLET_SS1_HSIOM_MASK,
                                       MONITORING_TABLET_SS1_HSIOM_POS,
                                       hsiomSel[MONITORING_TABLET_SS1_PIN_INDEX]);

        MONITORING_TABLET_spi_ss1_SetDriveMode((uint8) pinsDm[MONITORING_TABLET_SS1_PIN_INDEX]);

        MONITORING_TABLET_SET_INP_DIS(MONITORING_TABLET_spi_ss1_INP_DIS,
                                     MONITORING_TABLET_spi_ss1_MASK,
                                     (0u != (pinsInBuf & MONITORING_TABLET_SS1_PIN_MASK)));
    #endif /* (MONITORING_TABLET_SS1_PIN) */

    #if (MONITORING_TABLET_SS2_PIN)
        MONITORING_TABLET_SET_HSIOM_SEL(MONITORING_TABLET_SS2_HSIOM_REG,
                                       MONITORING_TABLET_SS2_HSIOM_MASK,
                                       MONITORING_TABLET_SS2_HSIOM_POS,
                                       hsiomSel[MONITORING_TABLET_SS2_PIN_INDEX]);

        MONITORING_TABLET_spi_ss2_SetDriveMode((uint8) pinsDm[MONITORING_TABLET_SS2_PIN_INDEX]);

        MONITORING_TABLET_SET_INP_DIS(MONITORING_TABLET_spi_ss2_INP_DIS,
                                     MONITORING_TABLET_spi_ss2_MASK,
                                     (0u != (pinsInBuf & MONITORING_TABLET_SS2_PIN_MASK)));
    #endif /* (MONITORING_TABLET_SS2_PIN) */

    #if (MONITORING_TABLET_SS3_PIN)
        MONITORING_TABLET_SET_HSIOM_SEL(MONITORING_TABLET_SS3_HSIOM_REG,
                                       MONITORING_TABLET_SS3_HSIOM_MASK,
                                       MONITORING_TABLET_SS3_HSIOM_POS,
                                       hsiomSel[MONITORING_TABLET_SS3_PIN_INDEX]);

        MONITORING_TABLET_spi_ss3_SetDriveMode((uint8) pinsDm[MONITORING_TABLET_SS3_PIN_INDEX]);

        MONITORING_TABLET_SET_INP_DIS(MONITORING_TABLET_spi_ss3_INP_DIS,
                                     MONITORING_TABLET_spi_ss3_MASK,
                                     (0u != (pinsInBuf & MONITORING_TABLET_SS3_PIN_MASK)));
    #endif /* (MONITORING_TABLET_SS3_PIN) */
    }

#endif /* (MONITORING_TABLET_SCB_MODE_UNCONFIG_CONST_CFG) */


#if (MONITORING_TABLET_CY_SCBIP_V0 || MONITORING_TABLET_CY_SCBIP_V1)
    /*******************************************************************************
    * Function Name: MONITORING_TABLET_I2CSlaveNackGeneration
    ****************************************************************************//**
    *
    *  Sets command to generate NACK to the address or data.
    *
    *******************************************************************************/
    void MONITORING_TABLET_I2CSlaveNackGeneration(void)
    {
        /* Check for EC_AM toggle condition: EC_AM and clock stretching for address are enabled */
        if ((0u != (MONITORING_TABLET_CTRL_REG & MONITORING_TABLET_CTRL_EC_AM_MODE)) &&
            (0u == (MONITORING_TABLET_I2C_CTRL_REG & MONITORING_TABLET_I2C_CTRL_S_NOT_READY_ADDR_NACK)))
        {
            /* Toggle EC_AM before NACK generation */
            MONITORING_TABLET_CTRL_REG &= ~MONITORING_TABLET_CTRL_EC_AM_MODE;
            MONITORING_TABLET_CTRL_REG |=  MONITORING_TABLET_CTRL_EC_AM_MODE;
        }

        MONITORING_TABLET_I2C_SLAVE_CMD_REG = MONITORING_TABLET_I2C_SLAVE_CMD_S_NACK;
    }
#endif /* (MONITORING_TABLET_CY_SCBIP_V0 || MONITORING_TABLET_CY_SCBIP_V1) */


/* [] END OF FILE */
