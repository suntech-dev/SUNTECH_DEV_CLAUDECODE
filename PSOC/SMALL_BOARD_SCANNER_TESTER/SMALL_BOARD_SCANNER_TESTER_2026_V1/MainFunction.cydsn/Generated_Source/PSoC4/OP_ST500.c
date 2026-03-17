/***************************************************************************//**
* \file OP_ST500.c
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

#include "OP_ST500_PVT.h"

#if (OP_ST500_SCB_MODE_I2C_INC)
    #include "OP_ST500_I2C_PVT.h"
#endif /* (OP_ST500_SCB_MODE_I2C_INC) */

#if (OP_ST500_SCB_MODE_EZI2C_INC)
    #include "OP_ST500_EZI2C_PVT.h"
#endif /* (OP_ST500_SCB_MODE_EZI2C_INC) */

#if (OP_ST500_SCB_MODE_SPI_INC || OP_ST500_SCB_MODE_UART_INC)
    #include "OP_ST500_SPI_UART_PVT.h"
#endif /* (OP_ST500_SCB_MODE_SPI_INC || OP_ST500_SCB_MODE_UART_INC) */


/***************************************
*    Run Time Configuration Vars
***************************************/

/* Stores internal component configuration for Unconfigured mode */
#if (OP_ST500_SCB_MODE_UNCONFIG_CONST_CFG)
    /* Common configuration variables */
    uint8 OP_ST500_scbMode = OP_ST500_SCB_MODE_UNCONFIG;
    uint8 OP_ST500_scbEnableWake;
    uint8 OP_ST500_scbEnableIntr;

    /* I2C configuration variables */
    uint8 OP_ST500_mode;
    uint8 OP_ST500_acceptAddr;

    /* SPI/UART configuration variables */
    volatile uint8 * OP_ST500_rxBuffer;
    uint8  OP_ST500_rxDataBits;
    uint32 OP_ST500_rxBufferSize;

    volatile uint8 * OP_ST500_txBuffer;
    uint8  OP_ST500_txDataBits;
    uint32 OP_ST500_txBufferSize;

    /* EZI2C configuration variables */
    uint8 OP_ST500_numberOfAddr;
    uint8 OP_ST500_subAddrSize;
#endif /* (OP_ST500_SCB_MODE_UNCONFIG_CONST_CFG) */


/***************************************
*     Common SCB Vars
***************************************/
/**
* \addtogroup group_general
* \{
*/

/** OP_ST500_initVar indicates whether the OP_ST500 
*  component has been initialized. The variable is initialized to 0 
*  and set to 1 the first time SCB_Start() is called. This allows 
*  the component to restart without reinitialization after the first 
*  call to the OP_ST500_Start() routine.
*
*  If re-initialization of the component is required, then the 
*  OP_ST500_Init() function can be called before the 
*  OP_ST500_Start() or OP_ST500_Enable() function.
*/
uint8 OP_ST500_initVar = 0u;


#if (! (OP_ST500_SCB_MODE_I2C_CONST_CFG || \
        OP_ST500_SCB_MODE_EZI2C_CONST_CFG))
    /** This global variable stores TX interrupt sources after 
    * OP_ST500_Stop() is called. Only these TX interrupt sources 
    * will be restored on a subsequent OP_ST500_Enable() call.
    */
    uint16 OP_ST500_IntrTxMask = 0u;
#endif /* (! (OP_ST500_SCB_MODE_I2C_CONST_CFG || \
              OP_ST500_SCB_MODE_EZI2C_CONST_CFG)) */
/** \} globals */

#if (OP_ST500_SCB_IRQ_INTERNAL)
#if !defined (CY_REMOVE_OP_ST500_CUSTOM_INTR_HANDLER)
    void (*OP_ST500_customIntrHandler)(void) = NULL;
#endif /* !defined (CY_REMOVE_OP_ST500_CUSTOM_INTR_HANDLER) */
#endif /* (OP_ST500_SCB_IRQ_INTERNAL) */


/***************************************
*    Private Function Prototypes
***************************************/

static void OP_ST500_ScbEnableIntr(void);
static void OP_ST500_ScbModeStop(void);
static void OP_ST500_ScbModePostEnable(void);


/*******************************************************************************
* Function Name: OP_ST500_Init
****************************************************************************//**
*
*  Initializes the OP_ST500 component to operate in one of the selected
*  configurations: I2C, SPI, UART or EZI2C.
*  When the configuration is set to "Unconfigured SCB", this function does
*  not do any initialization. Use mode-specific initialization APIs instead:
*  OP_ST500_I2CInit, OP_ST500_SpiInit, 
*  OP_ST500_UartInit or OP_ST500_EzI2CInit.
*
*******************************************************************************/
void OP_ST500_Init(void)
{
#if (OP_ST500_SCB_MODE_UNCONFIG_CONST_CFG)
    if (OP_ST500_SCB_MODE_UNCONFIG_RUNTM_CFG)
    {
        OP_ST500_initVar = 0u;
    }
    else
    {
        /* Initialization was done before this function call */
    }

#elif (OP_ST500_SCB_MODE_I2C_CONST_CFG)
    OP_ST500_I2CInit();

#elif (OP_ST500_SCB_MODE_SPI_CONST_CFG)
    OP_ST500_SpiInit();

#elif (OP_ST500_SCB_MODE_UART_CONST_CFG)
    OP_ST500_UartInit();

#elif (OP_ST500_SCB_MODE_EZI2C_CONST_CFG)
    OP_ST500_EzI2CInit();

#endif /* (OP_ST500_SCB_MODE_UNCONFIG_CONST_CFG) */
}


/*******************************************************************************
* Function Name: OP_ST500_Enable
****************************************************************************//**
*
*  Enables OP_ST500 component operation: activates the hardware and 
*  internal interrupt. It also restores TX interrupt sources disabled after the 
*  OP_ST500_Stop() function was called (note that level-triggered TX 
*  interrupt sources remain disabled to not cause code lock-up).
*  For I2C and EZI2C modes the interrupt is internal and mandatory for 
*  operation. For SPI and UART modes the interrupt can be configured as none, 
*  internal or external.
*  The OP_ST500 configuration should be not changed when the component
*  is enabled. Any configuration changes should be made after disabling the 
*  component.
*  When configuration is set to “Unconfigured OP_ST500”, the component 
*  must first be initialized to operate in one of the following configurations: 
*  I2C, SPI, UART or EZ I2C, using the mode-specific initialization API. 
*  Otherwise this function does not enable the component.
*
*******************************************************************************/
void OP_ST500_Enable(void)
{
#if (OP_ST500_SCB_MODE_UNCONFIG_CONST_CFG)
    /* Enable SCB block, only if it is already configured */
    if (!OP_ST500_SCB_MODE_UNCONFIG_RUNTM_CFG)
    {
        OP_ST500_CTRL_REG |= OP_ST500_CTRL_ENABLED;

        OP_ST500_ScbEnableIntr();

        /* Call PostEnable function specific to current operation mode */
        OP_ST500_ScbModePostEnable();
    }
#else
    OP_ST500_CTRL_REG |= OP_ST500_CTRL_ENABLED;

    OP_ST500_ScbEnableIntr();

    /* Call PostEnable function specific to current operation mode */
    OP_ST500_ScbModePostEnable();
#endif /* (OP_ST500_SCB_MODE_UNCONFIG_CONST_CFG) */
}


/*******************************************************************************
* Function Name: OP_ST500_Start
****************************************************************************//**
*
*  Invokes OP_ST500_Init() and OP_ST500_Enable().
*  After this function call, the component is enabled and ready for operation.
*  When configuration is set to "Unconfigured SCB", the component must first be
*  initialized to operate in one of the following configurations: I2C, SPI, UART
*  or EZI2C. Otherwise this function does not enable the component.
*
* \globalvars
*  OP_ST500_initVar - used to check initial configuration, modified
*  on first function call.
*
*******************************************************************************/
void OP_ST500_Start(void)
{
    if (0u == OP_ST500_initVar)
    {
        OP_ST500_Init();
        OP_ST500_initVar = 1u; /* Component was initialized */
    }

    OP_ST500_Enable();
}


/*******************************************************************************
* Function Name: OP_ST500_Stop
****************************************************************************//**
*
*  Disables the OP_ST500 component: disable the hardware and internal 
*  interrupt. It also disables all TX interrupt sources so as not to cause an 
*  unexpected interrupt trigger because after the component is enabled, the 
*  TX FIFO is empty.
*  Refer to the function OP_ST500_Enable() for the interrupt 
*  configuration details.
*  This function disables the SCB component without checking to see if 
*  communication is in progress. Before calling this function it may be 
*  necessary to check the status of communication to make sure communication 
*  is complete. If this is not done then communication could be stopped mid 
*  byte and corrupted data could result.
*
*******************************************************************************/
void OP_ST500_Stop(void)
{
#if (OP_ST500_SCB_IRQ_INTERNAL)
    OP_ST500_DisableInt();
#endif /* (OP_ST500_SCB_IRQ_INTERNAL) */

    /* Call Stop function specific to current operation mode */
    OP_ST500_ScbModeStop();

    /* Disable SCB IP */
    OP_ST500_CTRL_REG &= (uint32) ~OP_ST500_CTRL_ENABLED;

    /* Disable all TX interrupt sources so as not to cause an unexpected
    * interrupt trigger after the component will be enabled because the 
    * TX FIFO is empty.
    * For SCB IP v0, it is critical as it does not mask-out interrupt
    * sources when it is disabled. This can cause a code lock-up in the
    * interrupt handler because TX FIFO cannot be loaded after the block
    * is disabled.
    */
    OP_ST500_SetTxInterruptMode(OP_ST500_NO_INTR_SOURCES);

#if (OP_ST500_SCB_IRQ_INTERNAL)
    OP_ST500_ClearPendingInt();
#endif /* (OP_ST500_SCB_IRQ_INTERNAL) */
}


/*******************************************************************************
* Function Name: OP_ST500_SetRxFifoLevel
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
void OP_ST500_SetRxFifoLevel(uint32 level)
{
    uint32 rxFifoCtrl;

    rxFifoCtrl = OP_ST500_RX_FIFO_CTRL_REG;

    rxFifoCtrl &= ((uint32) ~OP_ST500_RX_FIFO_CTRL_TRIGGER_LEVEL_MASK); /* Clear level mask bits */
    rxFifoCtrl |= ((uint32) (OP_ST500_RX_FIFO_CTRL_TRIGGER_LEVEL_MASK & level));

    OP_ST500_RX_FIFO_CTRL_REG = rxFifoCtrl;
}


/*******************************************************************************
* Function Name: OP_ST500_SetTxFifoLevel
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
void OP_ST500_SetTxFifoLevel(uint32 level)
{
    uint32 txFifoCtrl;

    txFifoCtrl = OP_ST500_TX_FIFO_CTRL_REG;

    txFifoCtrl &= ((uint32) ~OP_ST500_TX_FIFO_CTRL_TRIGGER_LEVEL_MASK); /* Clear level mask bits */
    txFifoCtrl |= ((uint32) (OP_ST500_TX_FIFO_CTRL_TRIGGER_LEVEL_MASK & level));

    OP_ST500_TX_FIFO_CTRL_REG = txFifoCtrl;
}


#if (OP_ST500_SCB_IRQ_INTERNAL)
    /*******************************************************************************
    * Function Name: OP_ST500_SetCustomInterruptHandler
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
    void OP_ST500_SetCustomInterruptHandler(void (*func)(void))
    {
    #if !defined (CY_REMOVE_OP_ST500_CUSTOM_INTR_HANDLER)
        OP_ST500_customIntrHandler = func; /* Register interrupt handler */
    #else
        if (NULL != func)
        {
            /* Suppress compiler warning */
        }
    #endif /* !defined (CY_REMOVE_OP_ST500_CUSTOM_INTR_HANDLER) */
    }
#endif /* (OP_ST500_SCB_IRQ_INTERNAL) */


/*******************************************************************************
* Function Name: OP_ST500_ScbModeEnableIntr
****************************************************************************//**
*
*  Enables an interrupt for a specific mode.
*
*******************************************************************************/
static void OP_ST500_ScbEnableIntr(void)
{
#if (OP_ST500_SCB_IRQ_INTERNAL)
    /* Enable interrupt in NVIC */
    #if (OP_ST500_SCB_MODE_UNCONFIG_CONST_CFG)
        if (0u != OP_ST500_scbEnableIntr)
        {
            OP_ST500_EnableInt();
        }

    #else
        OP_ST500_EnableInt();

    #endif /* (OP_ST500_SCB_MODE_UNCONFIG_CONST_CFG) */
#endif /* (OP_ST500_SCB_IRQ_INTERNAL) */
}


/*******************************************************************************
* Function Name: OP_ST500_ScbModePostEnable
****************************************************************************//**
*
*  Calls the PostEnable function for a specific operation mode.
*
*******************************************************************************/
static void OP_ST500_ScbModePostEnable(void)
{
#if (OP_ST500_SCB_MODE_UNCONFIG_CONST_CFG)
#if (!OP_ST500_CY_SCBIP_V1)
    if (OP_ST500_SCB_MODE_SPI_RUNTM_CFG)
    {
        OP_ST500_SpiPostEnable();
    }
    else if (OP_ST500_SCB_MODE_UART_RUNTM_CFG)
    {
        OP_ST500_UartPostEnable();
    }
    else
    {
        /* Unknown mode: do nothing */
    }
#endif /* (!OP_ST500_CY_SCBIP_V1) */

#elif (OP_ST500_SCB_MODE_SPI_CONST_CFG)
    OP_ST500_SpiPostEnable();

#elif (OP_ST500_SCB_MODE_UART_CONST_CFG)
    OP_ST500_UartPostEnable();

#else
    /* Unknown mode: do nothing */
#endif /* (OP_ST500_SCB_MODE_UNCONFIG_CONST_CFG) */
}


/*******************************************************************************
* Function Name: OP_ST500_ScbModeStop
****************************************************************************//**
*
*  Calls the Stop function for a specific operation mode.
*
*******************************************************************************/
static void OP_ST500_ScbModeStop(void)
{
#if (OP_ST500_SCB_MODE_UNCONFIG_CONST_CFG)
    if (OP_ST500_SCB_MODE_I2C_RUNTM_CFG)
    {
        OP_ST500_I2CStop();
    }
    else if (OP_ST500_SCB_MODE_EZI2C_RUNTM_CFG)
    {
        OP_ST500_EzI2CStop();
    }
#if (!OP_ST500_CY_SCBIP_V1)
    else if (OP_ST500_SCB_MODE_SPI_RUNTM_CFG)
    {
        OP_ST500_SpiStop();
    }
    else if (OP_ST500_SCB_MODE_UART_RUNTM_CFG)
    {
        OP_ST500_UartStop();
    }
#endif /* (!OP_ST500_CY_SCBIP_V1) */
    else
    {
        /* Unknown mode: do nothing */
    }
#elif (OP_ST500_SCB_MODE_I2C_CONST_CFG)
    OP_ST500_I2CStop();

#elif (OP_ST500_SCB_MODE_EZI2C_CONST_CFG)
    OP_ST500_EzI2CStop();

#elif (OP_ST500_SCB_MODE_SPI_CONST_CFG)
    OP_ST500_SpiStop();

#elif (OP_ST500_SCB_MODE_UART_CONST_CFG)
    OP_ST500_UartStop();

#else
    /* Unknown mode: do nothing */
#endif /* (OP_ST500_SCB_MODE_UNCONFIG_CONST_CFG) */
}


#if (OP_ST500_SCB_MODE_UNCONFIG_CONST_CFG)
    /*******************************************************************************
    * Function Name: OP_ST500_SetPins
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
    void OP_ST500_SetPins(uint32 mode, uint32 subMode, uint32 uartEnableMask)
    {
        uint32 pinsDm[OP_ST500_SCB_PINS_NUMBER];
        uint32 i;
        
    #if (!OP_ST500_CY_SCBIP_V1)
        uint32 pinsInBuf = 0u;
    #endif /* (!OP_ST500_CY_SCBIP_V1) */
        
        uint32 hsiomSel[OP_ST500_SCB_PINS_NUMBER] = 
        {
            OP_ST500_RX_SCL_MOSI_HSIOM_SEL_GPIO,
            OP_ST500_TX_SDA_MISO_HSIOM_SEL_GPIO,
            0u,
            0u,
            0u,
            0u,
            0u,
        };

    #if (OP_ST500_CY_SCBIP_V1)
        /* Supress compiler warning. */
        if ((0u == subMode) || (0u == uartEnableMask))
        {
        }
    #endif /* (OP_ST500_CY_SCBIP_V1) */

        /* Set default HSIOM to GPIO and Drive Mode to Analog Hi-Z */
        for (i = 0u; i < OP_ST500_SCB_PINS_NUMBER; i++)
        {
            pinsDm[i] = OP_ST500_PIN_DM_ALG_HIZ;
        }

        if ((OP_ST500_SCB_MODE_I2C   == mode) ||
            (OP_ST500_SCB_MODE_EZI2C == mode))
        {
        #if (OP_ST500_RX_SCL_MOSI_PIN)
            hsiomSel[OP_ST500_RX_SCL_MOSI_PIN_INDEX] = OP_ST500_RX_SCL_MOSI_HSIOM_SEL_I2C;
            pinsDm  [OP_ST500_RX_SCL_MOSI_PIN_INDEX] = OP_ST500_PIN_DM_OD_LO;
        #elif (OP_ST500_RX_WAKE_SCL_MOSI_PIN)
            hsiomSel[OP_ST500_RX_WAKE_SCL_MOSI_PIN_INDEX] = OP_ST500_RX_WAKE_SCL_MOSI_HSIOM_SEL_I2C;
            pinsDm  [OP_ST500_RX_WAKE_SCL_MOSI_PIN_INDEX] = OP_ST500_PIN_DM_OD_LO;
        #else
        #endif /* (OP_ST500_RX_SCL_MOSI_PIN) */
        
        #if (OP_ST500_TX_SDA_MISO_PIN)
            hsiomSel[OP_ST500_TX_SDA_MISO_PIN_INDEX] = OP_ST500_TX_SDA_MISO_HSIOM_SEL_I2C;
            pinsDm  [OP_ST500_TX_SDA_MISO_PIN_INDEX] = OP_ST500_PIN_DM_OD_LO;
        #endif /* (OP_ST500_TX_SDA_MISO_PIN) */
        }
    #if (!OP_ST500_CY_SCBIP_V1)
        else if (OP_ST500_SCB_MODE_SPI == mode)
        {
        #if (OP_ST500_RX_SCL_MOSI_PIN)
            hsiomSel[OP_ST500_RX_SCL_MOSI_PIN_INDEX] = OP_ST500_RX_SCL_MOSI_HSIOM_SEL_SPI;
        #elif (OP_ST500_RX_WAKE_SCL_MOSI_PIN)
            hsiomSel[OP_ST500_RX_WAKE_SCL_MOSI_PIN_INDEX] = OP_ST500_RX_WAKE_SCL_MOSI_HSIOM_SEL_SPI;
        #else
        #endif /* (OP_ST500_RX_SCL_MOSI_PIN) */
        
        #if (OP_ST500_TX_SDA_MISO_PIN)
            hsiomSel[OP_ST500_TX_SDA_MISO_PIN_INDEX] = OP_ST500_TX_SDA_MISO_HSIOM_SEL_SPI;
        #endif /* (OP_ST500_TX_SDA_MISO_PIN) */
        
        #if (OP_ST500_CTS_SCLK_PIN)
            hsiomSel[OP_ST500_CTS_SCLK_PIN_INDEX] = OP_ST500_CTS_SCLK_HSIOM_SEL_SPI;
        #endif /* (OP_ST500_CTS_SCLK_PIN) */

            if (OP_ST500_SPI_SLAVE == subMode)
            {
                /* Slave */
                pinsDm[OP_ST500_RX_SCL_MOSI_PIN_INDEX] = OP_ST500_PIN_DM_DIG_HIZ;
                pinsDm[OP_ST500_TX_SDA_MISO_PIN_INDEX] = OP_ST500_PIN_DM_STRONG;
                pinsDm[OP_ST500_CTS_SCLK_PIN_INDEX] = OP_ST500_PIN_DM_DIG_HIZ;

            #if (OP_ST500_RTS_SS0_PIN)
                /* Only SS0 is valid choice for Slave */
                hsiomSel[OP_ST500_RTS_SS0_PIN_INDEX] = OP_ST500_RTS_SS0_HSIOM_SEL_SPI;
                pinsDm  [OP_ST500_RTS_SS0_PIN_INDEX] = OP_ST500_PIN_DM_DIG_HIZ;
            #endif /* (OP_ST500_RTS_SS0_PIN) */

            #if (OP_ST500_TX_SDA_MISO_PIN)
                /* Disable input buffer */
                 pinsInBuf |= OP_ST500_TX_SDA_MISO_PIN_MASK;
            #endif /* (OP_ST500_TX_SDA_MISO_PIN) */
            }
            else 
            {
                /* (Master) */
                pinsDm[OP_ST500_RX_SCL_MOSI_PIN_INDEX] = OP_ST500_PIN_DM_STRONG;
                pinsDm[OP_ST500_TX_SDA_MISO_PIN_INDEX] = OP_ST500_PIN_DM_DIG_HIZ;
                pinsDm[OP_ST500_CTS_SCLK_PIN_INDEX] = OP_ST500_PIN_DM_STRONG;

            #if (OP_ST500_RTS_SS0_PIN)
                hsiomSel [OP_ST500_RTS_SS0_PIN_INDEX] = OP_ST500_RTS_SS0_HSIOM_SEL_SPI;
                pinsDm   [OP_ST500_RTS_SS0_PIN_INDEX] = OP_ST500_PIN_DM_STRONG;
                pinsInBuf |= OP_ST500_RTS_SS0_PIN_MASK;
            #endif /* (OP_ST500_RTS_SS0_PIN) */

            #if (OP_ST500_SS1_PIN)
                hsiomSel [OP_ST500_SS1_PIN_INDEX] = OP_ST500_SS1_HSIOM_SEL_SPI;
                pinsDm   [OP_ST500_SS1_PIN_INDEX] = OP_ST500_PIN_DM_STRONG;
                pinsInBuf |= OP_ST500_SS1_PIN_MASK;
            #endif /* (OP_ST500_SS1_PIN) */

            #if (OP_ST500_SS2_PIN)
                hsiomSel [OP_ST500_SS2_PIN_INDEX] = OP_ST500_SS2_HSIOM_SEL_SPI;
                pinsDm   [OP_ST500_SS2_PIN_INDEX] = OP_ST500_PIN_DM_STRONG;
                pinsInBuf |= OP_ST500_SS2_PIN_MASK;
            #endif /* (OP_ST500_SS2_PIN) */

            #if (OP_ST500_SS3_PIN)
                hsiomSel [OP_ST500_SS3_PIN_INDEX] = OP_ST500_SS3_HSIOM_SEL_SPI;
                pinsDm   [OP_ST500_SS3_PIN_INDEX] = OP_ST500_PIN_DM_STRONG;
                pinsInBuf |= OP_ST500_SS3_PIN_MASK;
            #endif /* (OP_ST500_SS3_PIN) */

                /* Disable input buffers */
            #if (OP_ST500_RX_SCL_MOSI_PIN)
                pinsInBuf |= OP_ST500_RX_SCL_MOSI_PIN_MASK;
            #elif (OP_ST500_RX_WAKE_SCL_MOSI_PIN)
                pinsInBuf |= OP_ST500_RX_WAKE_SCL_MOSI_PIN_MASK;
            #else
            #endif /* (OP_ST500_RX_SCL_MOSI_PIN) */

            #if (OP_ST500_CTS_SCLK_PIN)
                pinsInBuf |= OP_ST500_CTS_SCLK_PIN_MASK;
            #endif /* (OP_ST500_CTS_SCLK_PIN) */
            }
        }
        else /* UART */
        {
            if (OP_ST500_UART_MODE_SMARTCARD == subMode)
            {
                /* SmartCard */
            #if (OP_ST500_TX_SDA_MISO_PIN)
                hsiomSel[OP_ST500_TX_SDA_MISO_PIN_INDEX] = OP_ST500_TX_SDA_MISO_HSIOM_SEL_UART;
                pinsDm  [OP_ST500_TX_SDA_MISO_PIN_INDEX] = OP_ST500_PIN_DM_OD_LO;
            #endif /* (OP_ST500_TX_SDA_MISO_PIN) */
            }
            else /* Standard or IrDA */
            {
                if (0u != (OP_ST500_UART_RX_PIN_ENABLE & uartEnableMask))
                {
                #if (OP_ST500_RX_SCL_MOSI_PIN)
                    hsiomSel[OP_ST500_RX_SCL_MOSI_PIN_INDEX] = OP_ST500_RX_SCL_MOSI_HSIOM_SEL_UART;
                    pinsDm  [OP_ST500_RX_SCL_MOSI_PIN_INDEX] = OP_ST500_PIN_DM_DIG_HIZ;
                #elif (OP_ST500_RX_WAKE_SCL_MOSI_PIN)
                    hsiomSel[OP_ST500_RX_WAKE_SCL_MOSI_PIN_INDEX] = OP_ST500_RX_WAKE_SCL_MOSI_HSIOM_SEL_UART;
                    pinsDm  [OP_ST500_RX_WAKE_SCL_MOSI_PIN_INDEX] = OP_ST500_PIN_DM_DIG_HIZ;
                #else
                #endif /* (OP_ST500_RX_SCL_MOSI_PIN) */
                }

                if (0u != (OP_ST500_UART_TX_PIN_ENABLE & uartEnableMask))
                {
                #if (OP_ST500_TX_SDA_MISO_PIN)
                    hsiomSel[OP_ST500_TX_SDA_MISO_PIN_INDEX] = OP_ST500_TX_SDA_MISO_HSIOM_SEL_UART;
                    pinsDm  [OP_ST500_TX_SDA_MISO_PIN_INDEX] = OP_ST500_PIN_DM_STRONG;
                    
                    /* Disable input buffer */
                    pinsInBuf |= OP_ST500_TX_SDA_MISO_PIN_MASK;
                #endif /* (OP_ST500_TX_SDA_MISO_PIN) */
                }

            #if !(OP_ST500_CY_SCBIP_V0 || OP_ST500_CY_SCBIP_V1)
                if (OP_ST500_UART_MODE_STD == subMode)
                {
                    if (0u != (OP_ST500_UART_CTS_PIN_ENABLE & uartEnableMask))
                    {
                        /* CTS input is multiplexed with SCLK */
                    #if (OP_ST500_CTS_SCLK_PIN)
                        hsiomSel[OP_ST500_CTS_SCLK_PIN_INDEX] = OP_ST500_CTS_SCLK_HSIOM_SEL_UART;
                        pinsDm  [OP_ST500_CTS_SCLK_PIN_INDEX] = OP_ST500_PIN_DM_DIG_HIZ;
                    #endif /* (OP_ST500_CTS_SCLK_PIN) */
                    }

                    if (0u != (OP_ST500_UART_RTS_PIN_ENABLE & uartEnableMask))
                    {
                        /* RTS output is multiplexed with SS0 */
                    #if (OP_ST500_RTS_SS0_PIN)
                        hsiomSel[OP_ST500_RTS_SS0_PIN_INDEX] = OP_ST500_RTS_SS0_HSIOM_SEL_UART;
                        pinsDm  [OP_ST500_RTS_SS0_PIN_INDEX] = OP_ST500_PIN_DM_STRONG;
                        
                        /* Disable input buffer */
                        pinsInBuf |= OP_ST500_RTS_SS0_PIN_MASK;
                    #endif /* (OP_ST500_RTS_SS0_PIN) */
                    }
                }
            #endif /* !(OP_ST500_CY_SCBIP_V0 || OP_ST500_CY_SCBIP_V1) */
            }
        }
    #endif /* (!OP_ST500_CY_SCBIP_V1) */

    /* Configure pins: set HSIOM, DM and InputBufEnable */
    /* Note: the DR register settings do not effect the pin output if HSIOM is other than GPIO */

    #if (OP_ST500_RX_SCL_MOSI_PIN)
        OP_ST500_SET_HSIOM_SEL(OP_ST500_RX_SCL_MOSI_HSIOM_REG,
                                       OP_ST500_RX_SCL_MOSI_HSIOM_MASK,
                                       OP_ST500_RX_SCL_MOSI_HSIOM_POS,
                                        hsiomSel[OP_ST500_RX_SCL_MOSI_PIN_INDEX]);

        OP_ST500_uart_rx_i2c_scl_spi_mosi_SetDriveMode((uint8) pinsDm[OP_ST500_RX_SCL_MOSI_PIN_INDEX]);

        #if (!OP_ST500_CY_SCBIP_V1)
            OP_ST500_SET_INP_DIS(OP_ST500_uart_rx_i2c_scl_spi_mosi_INP_DIS,
                                         OP_ST500_uart_rx_i2c_scl_spi_mosi_MASK,
                                         (0u != (pinsInBuf & OP_ST500_RX_SCL_MOSI_PIN_MASK)));
        #endif /* (!OP_ST500_CY_SCBIP_V1) */
    
    #elif (OP_ST500_RX_WAKE_SCL_MOSI_PIN)
        OP_ST500_SET_HSIOM_SEL(OP_ST500_RX_WAKE_SCL_MOSI_HSIOM_REG,
                                       OP_ST500_RX_WAKE_SCL_MOSI_HSIOM_MASK,
                                       OP_ST500_RX_WAKE_SCL_MOSI_HSIOM_POS,
                                       hsiomSel[OP_ST500_RX_WAKE_SCL_MOSI_PIN_INDEX]);

        OP_ST500_uart_rx_wake_i2c_scl_spi_mosi_SetDriveMode((uint8)
                                                               pinsDm[OP_ST500_RX_WAKE_SCL_MOSI_PIN_INDEX]);

        OP_ST500_SET_INP_DIS(OP_ST500_uart_rx_wake_i2c_scl_spi_mosi_INP_DIS,
                                     OP_ST500_uart_rx_wake_i2c_scl_spi_mosi_MASK,
                                     (0u != (pinsInBuf & OP_ST500_RX_WAKE_SCL_MOSI_PIN_MASK)));

         /* Set interrupt on falling edge */
        OP_ST500_SET_INCFG_TYPE(OP_ST500_RX_WAKE_SCL_MOSI_INTCFG_REG,
                                        OP_ST500_RX_WAKE_SCL_MOSI_INTCFG_TYPE_MASK,
                                        OP_ST500_RX_WAKE_SCL_MOSI_INTCFG_TYPE_POS,
                                        OP_ST500_INTCFG_TYPE_FALLING_EDGE);
    #else
    #endif /* (OP_ST500_RX_WAKE_SCL_MOSI_PIN) */

    #if (OP_ST500_TX_SDA_MISO_PIN)
        OP_ST500_SET_HSIOM_SEL(OP_ST500_TX_SDA_MISO_HSIOM_REG,
                                       OP_ST500_TX_SDA_MISO_HSIOM_MASK,
                                       OP_ST500_TX_SDA_MISO_HSIOM_POS,
                                        hsiomSel[OP_ST500_TX_SDA_MISO_PIN_INDEX]);

        OP_ST500_uart_tx_i2c_sda_spi_miso_SetDriveMode((uint8) pinsDm[OP_ST500_TX_SDA_MISO_PIN_INDEX]);

    #if (!OP_ST500_CY_SCBIP_V1)
        OP_ST500_SET_INP_DIS(OP_ST500_uart_tx_i2c_sda_spi_miso_INP_DIS,
                                     OP_ST500_uart_tx_i2c_sda_spi_miso_MASK,
                                    (0u != (pinsInBuf & OP_ST500_TX_SDA_MISO_PIN_MASK)));
    #endif /* (!OP_ST500_CY_SCBIP_V1) */
    #endif /* (OP_ST500_RX_SCL_MOSI_PIN) */

    #if (OP_ST500_CTS_SCLK_PIN)
        OP_ST500_SET_HSIOM_SEL(OP_ST500_CTS_SCLK_HSIOM_REG,
                                       OP_ST500_CTS_SCLK_HSIOM_MASK,
                                       OP_ST500_CTS_SCLK_HSIOM_POS,
                                       hsiomSel[OP_ST500_CTS_SCLK_PIN_INDEX]);

        OP_ST500_uart_cts_spi_sclk_SetDriveMode((uint8) pinsDm[OP_ST500_CTS_SCLK_PIN_INDEX]);

        OP_ST500_SET_INP_DIS(OP_ST500_uart_cts_spi_sclk_INP_DIS,
                                     OP_ST500_uart_cts_spi_sclk_MASK,
                                     (0u != (pinsInBuf & OP_ST500_CTS_SCLK_PIN_MASK)));
    #endif /* (OP_ST500_CTS_SCLK_PIN) */

    #if (OP_ST500_RTS_SS0_PIN)
        OP_ST500_SET_HSIOM_SEL(OP_ST500_RTS_SS0_HSIOM_REG,
                                       OP_ST500_RTS_SS0_HSIOM_MASK,
                                       OP_ST500_RTS_SS0_HSIOM_POS,
                                       hsiomSel[OP_ST500_RTS_SS0_PIN_INDEX]);

        OP_ST500_uart_rts_spi_ss0_SetDriveMode((uint8) pinsDm[OP_ST500_RTS_SS0_PIN_INDEX]);

        OP_ST500_SET_INP_DIS(OP_ST500_uart_rts_spi_ss0_INP_DIS,
                                     OP_ST500_uart_rts_spi_ss0_MASK,
                                     (0u != (pinsInBuf & OP_ST500_RTS_SS0_PIN_MASK)));
    #endif /* (OP_ST500_RTS_SS0_PIN) */

    #if (OP_ST500_SS1_PIN)
        OP_ST500_SET_HSIOM_SEL(OP_ST500_SS1_HSIOM_REG,
                                       OP_ST500_SS1_HSIOM_MASK,
                                       OP_ST500_SS1_HSIOM_POS,
                                       hsiomSel[OP_ST500_SS1_PIN_INDEX]);

        OP_ST500_spi_ss1_SetDriveMode((uint8) pinsDm[OP_ST500_SS1_PIN_INDEX]);

        OP_ST500_SET_INP_DIS(OP_ST500_spi_ss1_INP_DIS,
                                     OP_ST500_spi_ss1_MASK,
                                     (0u != (pinsInBuf & OP_ST500_SS1_PIN_MASK)));
    #endif /* (OP_ST500_SS1_PIN) */

    #if (OP_ST500_SS2_PIN)
        OP_ST500_SET_HSIOM_SEL(OP_ST500_SS2_HSIOM_REG,
                                       OP_ST500_SS2_HSIOM_MASK,
                                       OP_ST500_SS2_HSIOM_POS,
                                       hsiomSel[OP_ST500_SS2_PIN_INDEX]);

        OP_ST500_spi_ss2_SetDriveMode((uint8) pinsDm[OP_ST500_SS2_PIN_INDEX]);

        OP_ST500_SET_INP_DIS(OP_ST500_spi_ss2_INP_DIS,
                                     OP_ST500_spi_ss2_MASK,
                                     (0u != (pinsInBuf & OP_ST500_SS2_PIN_MASK)));
    #endif /* (OP_ST500_SS2_PIN) */

    #if (OP_ST500_SS3_PIN)
        OP_ST500_SET_HSIOM_SEL(OP_ST500_SS3_HSIOM_REG,
                                       OP_ST500_SS3_HSIOM_MASK,
                                       OP_ST500_SS3_HSIOM_POS,
                                       hsiomSel[OP_ST500_SS3_PIN_INDEX]);

        OP_ST500_spi_ss3_SetDriveMode((uint8) pinsDm[OP_ST500_SS3_PIN_INDEX]);

        OP_ST500_SET_INP_DIS(OP_ST500_spi_ss3_INP_DIS,
                                     OP_ST500_spi_ss3_MASK,
                                     (0u != (pinsInBuf & OP_ST500_SS3_PIN_MASK)));
    #endif /* (OP_ST500_SS3_PIN) */
    }

#endif /* (OP_ST500_SCB_MODE_UNCONFIG_CONST_CFG) */


#if (OP_ST500_CY_SCBIP_V0 || OP_ST500_CY_SCBIP_V1)
    /*******************************************************************************
    * Function Name: OP_ST500_I2CSlaveNackGeneration
    ****************************************************************************//**
    *
    *  Sets command to generate NACK to the address or data.
    *
    *******************************************************************************/
    void OP_ST500_I2CSlaveNackGeneration(void)
    {
        /* Check for EC_AM toggle condition: EC_AM and clock stretching for address are enabled */
        if ((0u != (OP_ST500_CTRL_REG & OP_ST500_CTRL_EC_AM_MODE)) &&
            (0u == (OP_ST500_I2C_CTRL_REG & OP_ST500_I2C_CTRL_S_NOT_READY_ADDR_NACK)))
        {
            /* Toggle EC_AM before NACK generation */
            OP_ST500_CTRL_REG &= ~OP_ST500_CTRL_EC_AM_MODE;
            OP_ST500_CTRL_REG |=  OP_ST500_CTRL_EC_AM_MODE;
        }

        OP_ST500_I2C_SLAVE_CMD_REG = OP_ST500_I2C_SLAVE_CMD_S_NACK;
    }
#endif /* (OP_ST500_CY_SCBIP_V0 || OP_ST500_CY_SCBIP_V1) */


/* [] END OF FILE */
