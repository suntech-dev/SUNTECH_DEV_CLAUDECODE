/***************************************************************************//**
* \file BARCODE.c
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

#include "BARCODE_PVT.h"

#if (BARCODE_SCB_MODE_I2C_INC)
    #include "BARCODE_I2C_PVT.h"
#endif /* (BARCODE_SCB_MODE_I2C_INC) */

#if (BARCODE_SCB_MODE_EZI2C_INC)
    #include "BARCODE_EZI2C_PVT.h"
#endif /* (BARCODE_SCB_MODE_EZI2C_INC) */

#if (BARCODE_SCB_MODE_SPI_INC || BARCODE_SCB_MODE_UART_INC)
    #include "BARCODE_SPI_UART_PVT.h"
#endif /* (BARCODE_SCB_MODE_SPI_INC || BARCODE_SCB_MODE_UART_INC) */


/***************************************
*    Run Time Configuration Vars
***************************************/

/* Stores internal component configuration for Unconfigured mode */
#if (BARCODE_SCB_MODE_UNCONFIG_CONST_CFG)
    /* Common configuration variables */
    uint8 BARCODE_scbMode = BARCODE_SCB_MODE_UNCONFIG;
    uint8 BARCODE_scbEnableWake;
    uint8 BARCODE_scbEnableIntr;

    /* I2C configuration variables */
    uint8 BARCODE_mode;
    uint8 BARCODE_acceptAddr;

    /* SPI/UART configuration variables */
    volatile uint8 * BARCODE_rxBuffer;
    uint8  BARCODE_rxDataBits;
    uint32 BARCODE_rxBufferSize;

    volatile uint8 * BARCODE_txBuffer;
    uint8  BARCODE_txDataBits;
    uint32 BARCODE_txBufferSize;

    /* EZI2C configuration variables */
    uint8 BARCODE_numberOfAddr;
    uint8 BARCODE_subAddrSize;
#endif /* (BARCODE_SCB_MODE_UNCONFIG_CONST_CFG) */


/***************************************
*     Common SCB Vars
***************************************/
/**
* \addtogroup group_general
* \{
*/

/** BARCODE_initVar indicates whether the BARCODE 
*  component has been initialized. The variable is initialized to 0 
*  and set to 1 the first time SCB_Start() is called. This allows 
*  the component to restart without reinitialization after the first 
*  call to the BARCODE_Start() routine.
*
*  If re-initialization of the component is required, then the 
*  BARCODE_Init() function can be called before the 
*  BARCODE_Start() or BARCODE_Enable() function.
*/
uint8 BARCODE_initVar = 0u;


#if (! (BARCODE_SCB_MODE_I2C_CONST_CFG || \
        BARCODE_SCB_MODE_EZI2C_CONST_CFG))
    /** This global variable stores TX interrupt sources after 
    * BARCODE_Stop() is called. Only these TX interrupt sources 
    * will be restored on a subsequent BARCODE_Enable() call.
    */
    uint16 BARCODE_IntrTxMask = 0u;
#endif /* (! (BARCODE_SCB_MODE_I2C_CONST_CFG || \
              BARCODE_SCB_MODE_EZI2C_CONST_CFG)) */
/** \} globals */

#if (BARCODE_SCB_IRQ_INTERNAL)
#if !defined (CY_REMOVE_BARCODE_CUSTOM_INTR_HANDLER)
    void (*BARCODE_customIntrHandler)(void) = NULL;
#endif /* !defined (CY_REMOVE_BARCODE_CUSTOM_INTR_HANDLER) */
#endif /* (BARCODE_SCB_IRQ_INTERNAL) */


/***************************************
*    Private Function Prototypes
***************************************/

static void BARCODE_ScbEnableIntr(void);
static void BARCODE_ScbModeStop(void);
static void BARCODE_ScbModePostEnable(void);


/*******************************************************************************
* Function Name: BARCODE_Init
****************************************************************************//**
*
*  Initializes the BARCODE component to operate in one of the selected
*  configurations: I2C, SPI, UART or EZI2C.
*  When the configuration is set to "Unconfigured SCB", this function does
*  not do any initialization. Use mode-specific initialization APIs instead:
*  BARCODE_I2CInit, BARCODE_SpiInit, 
*  BARCODE_UartInit or BARCODE_EzI2CInit.
*
*******************************************************************************/
void BARCODE_Init(void)
{
#if (BARCODE_SCB_MODE_UNCONFIG_CONST_CFG)
    if (BARCODE_SCB_MODE_UNCONFIG_RUNTM_CFG)
    {
        BARCODE_initVar = 0u;
    }
    else
    {
        /* Initialization was done before this function call */
    }

#elif (BARCODE_SCB_MODE_I2C_CONST_CFG)
    BARCODE_I2CInit();

#elif (BARCODE_SCB_MODE_SPI_CONST_CFG)
    BARCODE_SpiInit();

#elif (BARCODE_SCB_MODE_UART_CONST_CFG)
    BARCODE_UartInit();

#elif (BARCODE_SCB_MODE_EZI2C_CONST_CFG)
    BARCODE_EzI2CInit();

#endif /* (BARCODE_SCB_MODE_UNCONFIG_CONST_CFG) */
}


/*******************************************************************************
* Function Name: BARCODE_Enable
****************************************************************************//**
*
*  Enables BARCODE component operation: activates the hardware and 
*  internal interrupt. It also restores TX interrupt sources disabled after the 
*  BARCODE_Stop() function was called (note that level-triggered TX 
*  interrupt sources remain disabled to not cause code lock-up).
*  For I2C and EZI2C modes the interrupt is internal and mandatory for 
*  operation. For SPI and UART modes the interrupt can be configured as none, 
*  internal or external.
*  The BARCODE configuration should be not changed when the component
*  is enabled. Any configuration changes should be made after disabling the 
*  component.
*  When configuration is set to “Unconfigured BARCODE”, the component 
*  must first be initialized to operate in one of the following configurations: 
*  I2C, SPI, UART or EZ I2C, using the mode-specific initialization API. 
*  Otherwise this function does not enable the component.
*
*******************************************************************************/
void BARCODE_Enable(void)
{
#if (BARCODE_SCB_MODE_UNCONFIG_CONST_CFG)
    /* Enable SCB block, only if it is already configured */
    if (!BARCODE_SCB_MODE_UNCONFIG_RUNTM_CFG)
    {
        BARCODE_CTRL_REG |= BARCODE_CTRL_ENABLED;

        BARCODE_ScbEnableIntr();

        /* Call PostEnable function specific to current operation mode */
        BARCODE_ScbModePostEnable();
    }
#else
    BARCODE_CTRL_REG |= BARCODE_CTRL_ENABLED;

    BARCODE_ScbEnableIntr();

    /* Call PostEnable function specific to current operation mode */
    BARCODE_ScbModePostEnable();
#endif /* (BARCODE_SCB_MODE_UNCONFIG_CONST_CFG) */
}


/*******************************************************************************
* Function Name: BARCODE_Start
****************************************************************************//**
*
*  Invokes BARCODE_Init() and BARCODE_Enable().
*  After this function call, the component is enabled and ready for operation.
*  When configuration is set to "Unconfigured SCB", the component must first be
*  initialized to operate in one of the following configurations: I2C, SPI, UART
*  or EZI2C. Otherwise this function does not enable the component.
*
* \globalvars
*  BARCODE_initVar - used to check initial configuration, modified
*  on first function call.
*
*******************************************************************************/
void BARCODE_Start(void)
{
    if (0u == BARCODE_initVar)
    {
        BARCODE_Init();
        BARCODE_initVar = 1u; /* Component was initialized */
    }

    BARCODE_Enable();
}


/*******************************************************************************
* Function Name: BARCODE_Stop
****************************************************************************//**
*
*  Disables the BARCODE component: disable the hardware and internal 
*  interrupt. It also disables all TX interrupt sources so as not to cause an 
*  unexpected interrupt trigger because after the component is enabled, the 
*  TX FIFO is empty.
*  Refer to the function BARCODE_Enable() for the interrupt 
*  configuration details.
*  This function disables the SCB component without checking to see if 
*  communication is in progress. Before calling this function it may be 
*  necessary to check the status of communication to make sure communication 
*  is complete. If this is not done then communication could be stopped mid 
*  byte and corrupted data could result.
*
*******************************************************************************/
void BARCODE_Stop(void)
{
#if (BARCODE_SCB_IRQ_INTERNAL)
    BARCODE_DisableInt();
#endif /* (BARCODE_SCB_IRQ_INTERNAL) */

    /* Call Stop function specific to current operation mode */
    BARCODE_ScbModeStop();

    /* Disable SCB IP */
    BARCODE_CTRL_REG &= (uint32) ~BARCODE_CTRL_ENABLED;

    /* Disable all TX interrupt sources so as not to cause an unexpected
    * interrupt trigger after the component will be enabled because the 
    * TX FIFO is empty.
    * For SCB IP v0, it is critical as it does not mask-out interrupt
    * sources when it is disabled. This can cause a code lock-up in the
    * interrupt handler because TX FIFO cannot be loaded after the block
    * is disabled.
    */
    BARCODE_SetTxInterruptMode(BARCODE_NO_INTR_SOURCES);

#if (BARCODE_SCB_IRQ_INTERNAL)
    BARCODE_ClearPendingInt();
#endif /* (BARCODE_SCB_IRQ_INTERNAL) */
}


/*******************************************************************************
* Function Name: BARCODE_SetRxFifoLevel
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
void BARCODE_SetRxFifoLevel(uint32 level)
{
    uint32 rxFifoCtrl;

    rxFifoCtrl = BARCODE_RX_FIFO_CTRL_REG;

    rxFifoCtrl &= ((uint32) ~BARCODE_RX_FIFO_CTRL_TRIGGER_LEVEL_MASK); /* Clear level mask bits */
    rxFifoCtrl |= ((uint32) (BARCODE_RX_FIFO_CTRL_TRIGGER_LEVEL_MASK & level));

    BARCODE_RX_FIFO_CTRL_REG = rxFifoCtrl;
}


/*******************************************************************************
* Function Name: BARCODE_SetTxFifoLevel
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
void BARCODE_SetTxFifoLevel(uint32 level)
{
    uint32 txFifoCtrl;

    txFifoCtrl = BARCODE_TX_FIFO_CTRL_REG;

    txFifoCtrl &= ((uint32) ~BARCODE_TX_FIFO_CTRL_TRIGGER_LEVEL_MASK); /* Clear level mask bits */
    txFifoCtrl |= ((uint32) (BARCODE_TX_FIFO_CTRL_TRIGGER_LEVEL_MASK & level));

    BARCODE_TX_FIFO_CTRL_REG = txFifoCtrl;
}


#if (BARCODE_SCB_IRQ_INTERNAL)
    /*******************************************************************************
    * Function Name: BARCODE_SetCustomInterruptHandler
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
    void BARCODE_SetCustomInterruptHandler(void (*func)(void))
    {
    #if !defined (CY_REMOVE_BARCODE_CUSTOM_INTR_HANDLER)
        BARCODE_customIntrHandler = func; /* Register interrupt handler */
    #else
        if (NULL != func)
        {
            /* Suppress compiler warning */
        }
    #endif /* !defined (CY_REMOVE_BARCODE_CUSTOM_INTR_HANDLER) */
    }
#endif /* (BARCODE_SCB_IRQ_INTERNAL) */


/*******************************************************************************
* Function Name: BARCODE_ScbModeEnableIntr
****************************************************************************//**
*
*  Enables an interrupt for a specific mode.
*
*******************************************************************************/
static void BARCODE_ScbEnableIntr(void)
{
#if (BARCODE_SCB_IRQ_INTERNAL)
    /* Enable interrupt in NVIC */
    #if (BARCODE_SCB_MODE_UNCONFIG_CONST_CFG)
        if (0u != BARCODE_scbEnableIntr)
        {
            BARCODE_EnableInt();
        }

    #else
        BARCODE_EnableInt();

    #endif /* (BARCODE_SCB_MODE_UNCONFIG_CONST_CFG) */
#endif /* (BARCODE_SCB_IRQ_INTERNAL) */
}


/*******************************************************************************
* Function Name: BARCODE_ScbModePostEnable
****************************************************************************//**
*
*  Calls the PostEnable function for a specific operation mode.
*
*******************************************************************************/
static void BARCODE_ScbModePostEnable(void)
{
#if (BARCODE_SCB_MODE_UNCONFIG_CONST_CFG)
#if (!BARCODE_CY_SCBIP_V1)
    if (BARCODE_SCB_MODE_SPI_RUNTM_CFG)
    {
        BARCODE_SpiPostEnable();
    }
    else if (BARCODE_SCB_MODE_UART_RUNTM_CFG)
    {
        BARCODE_UartPostEnable();
    }
    else
    {
        /* Unknown mode: do nothing */
    }
#endif /* (!BARCODE_CY_SCBIP_V1) */

#elif (BARCODE_SCB_MODE_SPI_CONST_CFG)
    BARCODE_SpiPostEnable();

#elif (BARCODE_SCB_MODE_UART_CONST_CFG)
    BARCODE_UartPostEnable();

#else
    /* Unknown mode: do nothing */
#endif /* (BARCODE_SCB_MODE_UNCONFIG_CONST_CFG) */
}


/*******************************************************************************
* Function Name: BARCODE_ScbModeStop
****************************************************************************//**
*
*  Calls the Stop function for a specific operation mode.
*
*******************************************************************************/
static void BARCODE_ScbModeStop(void)
{
#if (BARCODE_SCB_MODE_UNCONFIG_CONST_CFG)
    if (BARCODE_SCB_MODE_I2C_RUNTM_CFG)
    {
        BARCODE_I2CStop();
    }
    else if (BARCODE_SCB_MODE_EZI2C_RUNTM_CFG)
    {
        BARCODE_EzI2CStop();
    }
#if (!BARCODE_CY_SCBIP_V1)
    else if (BARCODE_SCB_MODE_SPI_RUNTM_CFG)
    {
        BARCODE_SpiStop();
    }
    else if (BARCODE_SCB_MODE_UART_RUNTM_CFG)
    {
        BARCODE_UartStop();
    }
#endif /* (!BARCODE_CY_SCBIP_V1) */
    else
    {
        /* Unknown mode: do nothing */
    }
#elif (BARCODE_SCB_MODE_I2C_CONST_CFG)
    BARCODE_I2CStop();

#elif (BARCODE_SCB_MODE_EZI2C_CONST_CFG)
    BARCODE_EzI2CStop();

#elif (BARCODE_SCB_MODE_SPI_CONST_CFG)
    BARCODE_SpiStop();

#elif (BARCODE_SCB_MODE_UART_CONST_CFG)
    BARCODE_UartStop();

#else
    /* Unknown mode: do nothing */
#endif /* (BARCODE_SCB_MODE_UNCONFIG_CONST_CFG) */
}


#if (BARCODE_SCB_MODE_UNCONFIG_CONST_CFG)
    /*******************************************************************************
    * Function Name: BARCODE_SetPins
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
    void BARCODE_SetPins(uint32 mode, uint32 subMode, uint32 uartEnableMask)
    {
        uint32 pinsDm[BARCODE_SCB_PINS_NUMBER];
        uint32 i;
        
    #if (!BARCODE_CY_SCBIP_V1)
        uint32 pinsInBuf = 0u;
    #endif /* (!BARCODE_CY_SCBIP_V1) */
        
        uint32 hsiomSel[BARCODE_SCB_PINS_NUMBER] = 
        {
            BARCODE_RX_SCL_MOSI_HSIOM_SEL_GPIO,
            BARCODE_TX_SDA_MISO_HSIOM_SEL_GPIO,
            0u,
            0u,
            0u,
            0u,
            0u,
        };

    #if (BARCODE_CY_SCBIP_V1)
        /* Supress compiler warning. */
        if ((0u == subMode) || (0u == uartEnableMask))
        {
        }
    #endif /* (BARCODE_CY_SCBIP_V1) */

        /* Set default HSIOM to GPIO and Drive Mode to Analog Hi-Z */
        for (i = 0u; i < BARCODE_SCB_PINS_NUMBER; i++)
        {
            pinsDm[i] = BARCODE_PIN_DM_ALG_HIZ;
        }

        if ((BARCODE_SCB_MODE_I2C   == mode) ||
            (BARCODE_SCB_MODE_EZI2C == mode))
        {
        #if (BARCODE_RX_SCL_MOSI_PIN)
            hsiomSel[BARCODE_RX_SCL_MOSI_PIN_INDEX] = BARCODE_RX_SCL_MOSI_HSIOM_SEL_I2C;
            pinsDm  [BARCODE_RX_SCL_MOSI_PIN_INDEX] = BARCODE_PIN_DM_OD_LO;
        #elif (BARCODE_RX_WAKE_SCL_MOSI_PIN)
            hsiomSel[BARCODE_RX_WAKE_SCL_MOSI_PIN_INDEX] = BARCODE_RX_WAKE_SCL_MOSI_HSIOM_SEL_I2C;
            pinsDm  [BARCODE_RX_WAKE_SCL_MOSI_PIN_INDEX] = BARCODE_PIN_DM_OD_LO;
        #else
        #endif /* (BARCODE_RX_SCL_MOSI_PIN) */
        
        #if (BARCODE_TX_SDA_MISO_PIN)
            hsiomSel[BARCODE_TX_SDA_MISO_PIN_INDEX] = BARCODE_TX_SDA_MISO_HSIOM_SEL_I2C;
            pinsDm  [BARCODE_TX_SDA_MISO_PIN_INDEX] = BARCODE_PIN_DM_OD_LO;
        #endif /* (BARCODE_TX_SDA_MISO_PIN) */
        }
    #if (!BARCODE_CY_SCBIP_V1)
        else if (BARCODE_SCB_MODE_SPI == mode)
        {
        #if (BARCODE_RX_SCL_MOSI_PIN)
            hsiomSel[BARCODE_RX_SCL_MOSI_PIN_INDEX] = BARCODE_RX_SCL_MOSI_HSIOM_SEL_SPI;
        #elif (BARCODE_RX_WAKE_SCL_MOSI_PIN)
            hsiomSel[BARCODE_RX_WAKE_SCL_MOSI_PIN_INDEX] = BARCODE_RX_WAKE_SCL_MOSI_HSIOM_SEL_SPI;
        #else
        #endif /* (BARCODE_RX_SCL_MOSI_PIN) */
        
        #if (BARCODE_TX_SDA_MISO_PIN)
            hsiomSel[BARCODE_TX_SDA_MISO_PIN_INDEX] = BARCODE_TX_SDA_MISO_HSIOM_SEL_SPI;
        #endif /* (BARCODE_TX_SDA_MISO_PIN) */
        
        #if (BARCODE_CTS_SCLK_PIN)
            hsiomSel[BARCODE_CTS_SCLK_PIN_INDEX] = BARCODE_CTS_SCLK_HSIOM_SEL_SPI;
        #endif /* (BARCODE_CTS_SCLK_PIN) */

            if (BARCODE_SPI_SLAVE == subMode)
            {
                /* Slave */
                pinsDm[BARCODE_RX_SCL_MOSI_PIN_INDEX] = BARCODE_PIN_DM_DIG_HIZ;
                pinsDm[BARCODE_TX_SDA_MISO_PIN_INDEX] = BARCODE_PIN_DM_STRONG;
                pinsDm[BARCODE_CTS_SCLK_PIN_INDEX] = BARCODE_PIN_DM_DIG_HIZ;

            #if (BARCODE_RTS_SS0_PIN)
                /* Only SS0 is valid choice for Slave */
                hsiomSel[BARCODE_RTS_SS0_PIN_INDEX] = BARCODE_RTS_SS0_HSIOM_SEL_SPI;
                pinsDm  [BARCODE_RTS_SS0_PIN_INDEX] = BARCODE_PIN_DM_DIG_HIZ;
            #endif /* (BARCODE_RTS_SS0_PIN) */

            #if (BARCODE_TX_SDA_MISO_PIN)
                /* Disable input buffer */
                 pinsInBuf |= BARCODE_TX_SDA_MISO_PIN_MASK;
            #endif /* (BARCODE_TX_SDA_MISO_PIN) */
            }
            else 
            {
                /* (Master) */
                pinsDm[BARCODE_RX_SCL_MOSI_PIN_INDEX] = BARCODE_PIN_DM_STRONG;
                pinsDm[BARCODE_TX_SDA_MISO_PIN_INDEX] = BARCODE_PIN_DM_DIG_HIZ;
                pinsDm[BARCODE_CTS_SCLK_PIN_INDEX] = BARCODE_PIN_DM_STRONG;

            #if (BARCODE_RTS_SS0_PIN)
                hsiomSel [BARCODE_RTS_SS0_PIN_INDEX] = BARCODE_RTS_SS0_HSIOM_SEL_SPI;
                pinsDm   [BARCODE_RTS_SS0_PIN_INDEX] = BARCODE_PIN_DM_STRONG;
                pinsInBuf |= BARCODE_RTS_SS0_PIN_MASK;
            #endif /* (BARCODE_RTS_SS0_PIN) */

            #if (BARCODE_SS1_PIN)
                hsiomSel [BARCODE_SS1_PIN_INDEX] = BARCODE_SS1_HSIOM_SEL_SPI;
                pinsDm   [BARCODE_SS1_PIN_INDEX] = BARCODE_PIN_DM_STRONG;
                pinsInBuf |= BARCODE_SS1_PIN_MASK;
            #endif /* (BARCODE_SS1_PIN) */

            #if (BARCODE_SS2_PIN)
                hsiomSel [BARCODE_SS2_PIN_INDEX] = BARCODE_SS2_HSIOM_SEL_SPI;
                pinsDm   [BARCODE_SS2_PIN_INDEX] = BARCODE_PIN_DM_STRONG;
                pinsInBuf |= BARCODE_SS2_PIN_MASK;
            #endif /* (BARCODE_SS2_PIN) */

            #if (BARCODE_SS3_PIN)
                hsiomSel [BARCODE_SS3_PIN_INDEX] = BARCODE_SS3_HSIOM_SEL_SPI;
                pinsDm   [BARCODE_SS3_PIN_INDEX] = BARCODE_PIN_DM_STRONG;
                pinsInBuf |= BARCODE_SS3_PIN_MASK;
            #endif /* (BARCODE_SS3_PIN) */

                /* Disable input buffers */
            #if (BARCODE_RX_SCL_MOSI_PIN)
                pinsInBuf |= BARCODE_RX_SCL_MOSI_PIN_MASK;
            #elif (BARCODE_RX_WAKE_SCL_MOSI_PIN)
                pinsInBuf |= BARCODE_RX_WAKE_SCL_MOSI_PIN_MASK;
            #else
            #endif /* (BARCODE_RX_SCL_MOSI_PIN) */

            #if (BARCODE_CTS_SCLK_PIN)
                pinsInBuf |= BARCODE_CTS_SCLK_PIN_MASK;
            #endif /* (BARCODE_CTS_SCLK_PIN) */
            }
        }
        else /* UART */
        {
            if (BARCODE_UART_MODE_SMARTCARD == subMode)
            {
                /* SmartCard */
            #if (BARCODE_TX_SDA_MISO_PIN)
                hsiomSel[BARCODE_TX_SDA_MISO_PIN_INDEX] = BARCODE_TX_SDA_MISO_HSIOM_SEL_UART;
                pinsDm  [BARCODE_TX_SDA_MISO_PIN_INDEX] = BARCODE_PIN_DM_OD_LO;
            #endif /* (BARCODE_TX_SDA_MISO_PIN) */
            }
            else /* Standard or IrDA */
            {
                if (0u != (BARCODE_UART_RX_PIN_ENABLE & uartEnableMask))
                {
                #if (BARCODE_RX_SCL_MOSI_PIN)
                    hsiomSel[BARCODE_RX_SCL_MOSI_PIN_INDEX] = BARCODE_RX_SCL_MOSI_HSIOM_SEL_UART;
                    pinsDm  [BARCODE_RX_SCL_MOSI_PIN_INDEX] = BARCODE_PIN_DM_DIG_HIZ;
                #elif (BARCODE_RX_WAKE_SCL_MOSI_PIN)
                    hsiomSel[BARCODE_RX_WAKE_SCL_MOSI_PIN_INDEX] = BARCODE_RX_WAKE_SCL_MOSI_HSIOM_SEL_UART;
                    pinsDm  [BARCODE_RX_WAKE_SCL_MOSI_PIN_INDEX] = BARCODE_PIN_DM_DIG_HIZ;
                #else
                #endif /* (BARCODE_RX_SCL_MOSI_PIN) */
                }

                if (0u != (BARCODE_UART_TX_PIN_ENABLE & uartEnableMask))
                {
                #if (BARCODE_TX_SDA_MISO_PIN)
                    hsiomSel[BARCODE_TX_SDA_MISO_PIN_INDEX] = BARCODE_TX_SDA_MISO_HSIOM_SEL_UART;
                    pinsDm  [BARCODE_TX_SDA_MISO_PIN_INDEX] = BARCODE_PIN_DM_STRONG;
                    
                    /* Disable input buffer */
                    pinsInBuf |= BARCODE_TX_SDA_MISO_PIN_MASK;
                #endif /* (BARCODE_TX_SDA_MISO_PIN) */
                }

            #if !(BARCODE_CY_SCBIP_V0 || BARCODE_CY_SCBIP_V1)
                if (BARCODE_UART_MODE_STD == subMode)
                {
                    if (0u != (BARCODE_UART_CTS_PIN_ENABLE & uartEnableMask))
                    {
                        /* CTS input is multiplexed with SCLK */
                    #if (BARCODE_CTS_SCLK_PIN)
                        hsiomSel[BARCODE_CTS_SCLK_PIN_INDEX] = BARCODE_CTS_SCLK_HSIOM_SEL_UART;
                        pinsDm  [BARCODE_CTS_SCLK_PIN_INDEX] = BARCODE_PIN_DM_DIG_HIZ;
                    #endif /* (BARCODE_CTS_SCLK_PIN) */
                    }

                    if (0u != (BARCODE_UART_RTS_PIN_ENABLE & uartEnableMask))
                    {
                        /* RTS output is multiplexed with SS0 */
                    #if (BARCODE_RTS_SS0_PIN)
                        hsiomSel[BARCODE_RTS_SS0_PIN_INDEX] = BARCODE_RTS_SS0_HSIOM_SEL_UART;
                        pinsDm  [BARCODE_RTS_SS0_PIN_INDEX] = BARCODE_PIN_DM_STRONG;
                        
                        /* Disable input buffer */
                        pinsInBuf |= BARCODE_RTS_SS0_PIN_MASK;
                    #endif /* (BARCODE_RTS_SS0_PIN) */
                    }
                }
            #endif /* !(BARCODE_CY_SCBIP_V0 || BARCODE_CY_SCBIP_V1) */
            }
        }
    #endif /* (!BARCODE_CY_SCBIP_V1) */

    /* Configure pins: set HSIOM, DM and InputBufEnable */
    /* Note: the DR register settings do not effect the pin output if HSIOM is other than GPIO */

    #if (BARCODE_RX_SCL_MOSI_PIN)
        BARCODE_SET_HSIOM_SEL(BARCODE_RX_SCL_MOSI_HSIOM_REG,
                                       BARCODE_RX_SCL_MOSI_HSIOM_MASK,
                                       BARCODE_RX_SCL_MOSI_HSIOM_POS,
                                        hsiomSel[BARCODE_RX_SCL_MOSI_PIN_INDEX]);

        BARCODE_uart_rx_i2c_scl_spi_mosi_SetDriveMode((uint8) pinsDm[BARCODE_RX_SCL_MOSI_PIN_INDEX]);

        #if (!BARCODE_CY_SCBIP_V1)
            BARCODE_SET_INP_DIS(BARCODE_uart_rx_i2c_scl_spi_mosi_INP_DIS,
                                         BARCODE_uart_rx_i2c_scl_spi_mosi_MASK,
                                         (0u != (pinsInBuf & BARCODE_RX_SCL_MOSI_PIN_MASK)));
        #endif /* (!BARCODE_CY_SCBIP_V1) */
    
    #elif (BARCODE_RX_WAKE_SCL_MOSI_PIN)
        BARCODE_SET_HSIOM_SEL(BARCODE_RX_WAKE_SCL_MOSI_HSIOM_REG,
                                       BARCODE_RX_WAKE_SCL_MOSI_HSIOM_MASK,
                                       BARCODE_RX_WAKE_SCL_MOSI_HSIOM_POS,
                                       hsiomSel[BARCODE_RX_WAKE_SCL_MOSI_PIN_INDEX]);

        BARCODE_uart_rx_wake_i2c_scl_spi_mosi_SetDriveMode((uint8)
                                                               pinsDm[BARCODE_RX_WAKE_SCL_MOSI_PIN_INDEX]);

        BARCODE_SET_INP_DIS(BARCODE_uart_rx_wake_i2c_scl_spi_mosi_INP_DIS,
                                     BARCODE_uart_rx_wake_i2c_scl_spi_mosi_MASK,
                                     (0u != (pinsInBuf & BARCODE_RX_WAKE_SCL_MOSI_PIN_MASK)));

         /* Set interrupt on falling edge */
        BARCODE_SET_INCFG_TYPE(BARCODE_RX_WAKE_SCL_MOSI_INTCFG_REG,
                                        BARCODE_RX_WAKE_SCL_MOSI_INTCFG_TYPE_MASK,
                                        BARCODE_RX_WAKE_SCL_MOSI_INTCFG_TYPE_POS,
                                        BARCODE_INTCFG_TYPE_FALLING_EDGE);
    #else
    #endif /* (BARCODE_RX_WAKE_SCL_MOSI_PIN) */

    #if (BARCODE_TX_SDA_MISO_PIN)
        BARCODE_SET_HSIOM_SEL(BARCODE_TX_SDA_MISO_HSIOM_REG,
                                       BARCODE_TX_SDA_MISO_HSIOM_MASK,
                                       BARCODE_TX_SDA_MISO_HSIOM_POS,
                                        hsiomSel[BARCODE_TX_SDA_MISO_PIN_INDEX]);

        BARCODE_uart_tx_i2c_sda_spi_miso_SetDriveMode((uint8) pinsDm[BARCODE_TX_SDA_MISO_PIN_INDEX]);

    #if (!BARCODE_CY_SCBIP_V1)
        BARCODE_SET_INP_DIS(BARCODE_uart_tx_i2c_sda_spi_miso_INP_DIS,
                                     BARCODE_uart_tx_i2c_sda_spi_miso_MASK,
                                    (0u != (pinsInBuf & BARCODE_TX_SDA_MISO_PIN_MASK)));
    #endif /* (!BARCODE_CY_SCBIP_V1) */
    #endif /* (BARCODE_RX_SCL_MOSI_PIN) */

    #if (BARCODE_CTS_SCLK_PIN)
        BARCODE_SET_HSIOM_SEL(BARCODE_CTS_SCLK_HSIOM_REG,
                                       BARCODE_CTS_SCLK_HSIOM_MASK,
                                       BARCODE_CTS_SCLK_HSIOM_POS,
                                       hsiomSel[BARCODE_CTS_SCLK_PIN_INDEX]);

        BARCODE_uart_cts_spi_sclk_SetDriveMode((uint8) pinsDm[BARCODE_CTS_SCLK_PIN_INDEX]);

        BARCODE_SET_INP_DIS(BARCODE_uart_cts_spi_sclk_INP_DIS,
                                     BARCODE_uart_cts_spi_sclk_MASK,
                                     (0u != (pinsInBuf & BARCODE_CTS_SCLK_PIN_MASK)));
    #endif /* (BARCODE_CTS_SCLK_PIN) */

    #if (BARCODE_RTS_SS0_PIN)
        BARCODE_SET_HSIOM_SEL(BARCODE_RTS_SS0_HSIOM_REG,
                                       BARCODE_RTS_SS0_HSIOM_MASK,
                                       BARCODE_RTS_SS0_HSIOM_POS,
                                       hsiomSel[BARCODE_RTS_SS0_PIN_INDEX]);

        BARCODE_uart_rts_spi_ss0_SetDriveMode((uint8) pinsDm[BARCODE_RTS_SS0_PIN_INDEX]);

        BARCODE_SET_INP_DIS(BARCODE_uart_rts_spi_ss0_INP_DIS,
                                     BARCODE_uart_rts_spi_ss0_MASK,
                                     (0u != (pinsInBuf & BARCODE_RTS_SS0_PIN_MASK)));
    #endif /* (BARCODE_RTS_SS0_PIN) */

    #if (BARCODE_SS1_PIN)
        BARCODE_SET_HSIOM_SEL(BARCODE_SS1_HSIOM_REG,
                                       BARCODE_SS1_HSIOM_MASK,
                                       BARCODE_SS1_HSIOM_POS,
                                       hsiomSel[BARCODE_SS1_PIN_INDEX]);

        BARCODE_spi_ss1_SetDriveMode((uint8) pinsDm[BARCODE_SS1_PIN_INDEX]);

        BARCODE_SET_INP_DIS(BARCODE_spi_ss1_INP_DIS,
                                     BARCODE_spi_ss1_MASK,
                                     (0u != (pinsInBuf & BARCODE_SS1_PIN_MASK)));
    #endif /* (BARCODE_SS1_PIN) */

    #if (BARCODE_SS2_PIN)
        BARCODE_SET_HSIOM_SEL(BARCODE_SS2_HSIOM_REG,
                                       BARCODE_SS2_HSIOM_MASK,
                                       BARCODE_SS2_HSIOM_POS,
                                       hsiomSel[BARCODE_SS2_PIN_INDEX]);

        BARCODE_spi_ss2_SetDriveMode((uint8) pinsDm[BARCODE_SS2_PIN_INDEX]);

        BARCODE_SET_INP_DIS(BARCODE_spi_ss2_INP_DIS,
                                     BARCODE_spi_ss2_MASK,
                                     (0u != (pinsInBuf & BARCODE_SS2_PIN_MASK)));
    #endif /* (BARCODE_SS2_PIN) */

    #if (BARCODE_SS3_PIN)
        BARCODE_SET_HSIOM_SEL(BARCODE_SS3_HSIOM_REG,
                                       BARCODE_SS3_HSIOM_MASK,
                                       BARCODE_SS3_HSIOM_POS,
                                       hsiomSel[BARCODE_SS3_PIN_INDEX]);

        BARCODE_spi_ss3_SetDriveMode((uint8) pinsDm[BARCODE_SS3_PIN_INDEX]);

        BARCODE_SET_INP_DIS(BARCODE_spi_ss3_INP_DIS,
                                     BARCODE_spi_ss3_MASK,
                                     (0u != (pinsInBuf & BARCODE_SS3_PIN_MASK)));
    #endif /* (BARCODE_SS3_PIN) */
    }

#endif /* (BARCODE_SCB_MODE_UNCONFIG_CONST_CFG) */


#if (BARCODE_CY_SCBIP_V0 || BARCODE_CY_SCBIP_V1)
    /*******************************************************************************
    * Function Name: BARCODE_I2CSlaveNackGeneration
    ****************************************************************************//**
    *
    *  Sets command to generate NACK to the address or data.
    *
    *******************************************************************************/
    void BARCODE_I2CSlaveNackGeneration(void)
    {
        /* Check for EC_AM toggle condition: EC_AM and clock stretching for address are enabled */
        if ((0u != (BARCODE_CTRL_REG & BARCODE_CTRL_EC_AM_MODE)) &&
            (0u == (BARCODE_I2C_CTRL_REG & BARCODE_I2C_CTRL_S_NOT_READY_ADDR_NACK)))
        {
            /* Toggle EC_AM before NACK generation */
            BARCODE_CTRL_REG &= ~BARCODE_CTRL_EC_AM_MODE;
            BARCODE_CTRL_REG |=  BARCODE_CTRL_EC_AM_MODE;
        }

        BARCODE_I2C_SLAVE_CMD_REG = BARCODE_I2C_SLAVE_CMD_S_NACK;
    }
#endif /* (BARCODE_CY_SCBIP_V0 || BARCODE_CY_SCBIP_V1) */


/* [] END OF FILE */
