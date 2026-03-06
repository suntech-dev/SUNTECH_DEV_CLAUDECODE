/***************************************************************************//**
* \file WIFI.c
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

#include "WIFI_PVT.h"

#if (WIFI_SCB_MODE_I2C_INC)
    #include "WIFI_I2C_PVT.h"
#endif /* (WIFI_SCB_MODE_I2C_INC) */

#if (WIFI_SCB_MODE_EZI2C_INC)
    #include "WIFI_EZI2C_PVT.h"
#endif /* (WIFI_SCB_MODE_EZI2C_INC) */

#if (WIFI_SCB_MODE_SPI_INC || WIFI_SCB_MODE_UART_INC)
    #include "WIFI_SPI_UART_PVT.h"
#endif /* (WIFI_SCB_MODE_SPI_INC || WIFI_SCB_MODE_UART_INC) */


/***************************************
*    Run Time Configuration Vars
***************************************/

/* Stores internal component configuration for Unconfigured mode */
#if (WIFI_SCB_MODE_UNCONFIG_CONST_CFG)
    /* Common configuration variables */
    uint8 WIFI_scbMode = WIFI_SCB_MODE_UNCONFIG;
    uint8 WIFI_scbEnableWake;
    uint8 WIFI_scbEnableIntr;

    /* I2C configuration variables */
    uint8 WIFI_mode;
    uint8 WIFI_acceptAddr;

    /* SPI/UART configuration variables */
    volatile uint8 * WIFI_rxBuffer;
    uint8  WIFI_rxDataBits;
    uint32 WIFI_rxBufferSize;

    volatile uint8 * WIFI_txBuffer;
    uint8  WIFI_txDataBits;
    uint32 WIFI_txBufferSize;

    /* EZI2C configuration variables */
    uint8 WIFI_numberOfAddr;
    uint8 WIFI_subAddrSize;
#endif /* (WIFI_SCB_MODE_UNCONFIG_CONST_CFG) */


/***************************************
*     Common SCB Vars
***************************************/
/**
* \addtogroup group_general
* \{
*/

/** WIFI_initVar indicates whether the WIFI 
*  component has been initialized. The variable is initialized to 0 
*  and set to 1 the first time SCB_Start() is called. This allows 
*  the component to restart without reinitialization after the first 
*  call to the WIFI_Start() routine.
*
*  If re-initialization of the component is required, then the 
*  WIFI_Init() function can be called before the 
*  WIFI_Start() or WIFI_Enable() function.
*/
uint8 WIFI_initVar = 0u;


#if (! (WIFI_SCB_MODE_I2C_CONST_CFG || \
        WIFI_SCB_MODE_EZI2C_CONST_CFG))
    /** This global variable stores TX interrupt sources after 
    * WIFI_Stop() is called. Only these TX interrupt sources 
    * will be restored on a subsequent WIFI_Enable() call.
    */
    uint16 WIFI_IntrTxMask = 0u;
#endif /* (! (WIFI_SCB_MODE_I2C_CONST_CFG || \
              WIFI_SCB_MODE_EZI2C_CONST_CFG)) */
/** \} globals */

#if (WIFI_SCB_IRQ_INTERNAL)
#if !defined (CY_REMOVE_WIFI_CUSTOM_INTR_HANDLER)
    void (*WIFI_customIntrHandler)(void) = NULL;
#endif /* !defined (CY_REMOVE_WIFI_CUSTOM_INTR_HANDLER) */
#endif /* (WIFI_SCB_IRQ_INTERNAL) */


/***************************************
*    Private Function Prototypes
***************************************/

static void WIFI_ScbEnableIntr(void);
static void WIFI_ScbModeStop(void);
static void WIFI_ScbModePostEnable(void);


/*******************************************************************************
* Function Name: WIFI_Init
****************************************************************************//**
*
*  Initializes the WIFI component to operate in one of the selected
*  configurations: I2C, SPI, UART or EZI2C.
*  When the configuration is set to "Unconfigured SCB", this function does
*  not do any initialization. Use mode-specific initialization APIs instead:
*  WIFI_I2CInit, WIFI_SpiInit, 
*  WIFI_UartInit or WIFI_EzI2CInit.
*
*******************************************************************************/
void WIFI_Init(void)
{
#if (WIFI_SCB_MODE_UNCONFIG_CONST_CFG)
    if (WIFI_SCB_MODE_UNCONFIG_RUNTM_CFG)
    {
        WIFI_initVar = 0u;
    }
    else
    {
        /* Initialization was done before this function call */
    }

#elif (WIFI_SCB_MODE_I2C_CONST_CFG)
    WIFI_I2CInit();

#elif (WIFI_SCB_MODE_SPI_CONST_CFG)
    WIFI_SpiInit();

#elif (WIFI_SCB_MODE_UART_CONST_CFG)
    WIFI_UartInit();

#elif (WIFI_SCB_MODE_EZI2C_CONST_CFG)
    WIFI_EzI2CInit();

#endif /* (WIFI_SCB_MODE_UNCONFIG_CONST_CFG) */
}


/*******************************************************************************
* Function Name: WIFI_Enable
****************************************************************************//**
*
*  Enables WIFI component operation: activates the hardware and 
*  internal interrupt. It also restores TX interrupt sources disabled after the 
*  WIFI_Stop() function was called (note that level-triggered TX 
*  interrupt sources remain disabled to not cause code lock-up).
*  For I2C and EZI2C modes the interrupt is internal and mandatory for 
*  operation. For SPI and UART modes the interrupt can be configured as none, 
*  internal or external.
*  The WIFI configuration should be not changed when the component
*  is enabled. Any configuration changes should be made after disabling the 
*  component.
*  When configuration is set to “Unconfigured WIFI”, the component 
*  must first be initialized to operate in one of the following configurations: 
*  I2C, SPI, UART or EZ I2C, using the mode-specific initialization API. 
*  Otherwise this function does not enable the component.
*
*******************************************************************************/
void WIFI_Enable(void)
{
#if (WIFI_SCB_MODE_UNCONFIG_CONST_CFG)
    /* Enable SCB block, only if it is already configured */
    if (!WIFI_SCB_MODE_UNCONFIG_RUNTM_CFG)
    {
        WIFI_CTRL_REG |= WIFI_CTRL_ENABLED;

        WIFI_ScbEnableIntr();

        /* Call PostEnable function specific to current operation mode */
        WIFI_ScbModePostEnable();
    }
#else
    WIFI_CTRL_REG |= WIFI_CTRL_ENABLED;

    WIFI_ScbEnableIntr();

    /* Call PostEnable function specific to current operation mode */
    WIFI_ScbModePostEnable();
#endif /* (WIFI_SCB_MODE_UNCONFIG_CONST_CFG) */
}


/*******************************************************************************
* Function Name: WIFI_Start
****************************************************************************//**
*
*  Invokes WIFI_Init() and WIFI_Enable().
*  After this function call, the component is enabled and ready for operation.
*  When configuration is set to "Unconfigured SCB", the component must first be
*  initialized to operate in one of the following configurations: I2C, SPI, UART
*  or EZI2C. Otherwise this function does not enable the component.
*
* \globalvars
*  WIFI_initVar - used to check initial configuration, modified
*  on first function call.
*
*******************************************************************************/
void WIFI_Start(void)
{
    if (0u == WIFI_initVar)
    {
        WIFI_Init();
        WIFI_initVar = 1u; /* Component was initialized */
    }

    WIFI_Enable();
}


/*******************************************************************************
* Function Name: WIFI_Stop
****************************************************************************//**
*
*  Disables the WIFI component: disable the hardware and internal 
*  interrupt. It also disables all TX interrupt sources so as not to cause an 
*  unexpected interrupt trigger because after the component is enabled, the 
*  TX FIFO is empty.
*  Refer to the function WIFI_Enable() for the interrupt 
*  configuration details.
*  This function disables the SCB component without checking to see if 
*  communication is in progress. Before calling this function it may be 
*  necessary to check the status of communication to make sure communication 
*  is complete. If this is not done then communication could be stopped mid 
*  byte and corrupted data could result.
*
*******************************************************************************/
void WIFI_Stop(void)
{
#if (WIFI_SCB_IRQ_INTERNAL)
    WIFI_DisableInt();
#endif /* (WIFI_SCB_IRQ_INTERNAL) */

    /* Call Stop function specific to current operation mode */
    WIFI_ScbModeStop();

    /* Disable SCB IP */
    WIFI_CTRL_REG &= (uint32) ~WIFI_CTRL_ENABLED;

    /* Disable all TX interrupt sources so as not to cause an unexpected
    * interrupt trigger after the component will be enabled because the 
    * TX FIFO is empty.
    * For SCB IP v0, it is critical as it does not mask-out interrupt
    * sources when it is disabled. This can cause a code lock-up in the
    * interrupt handler because TX FIFO cannot be loaded after the block
    * is disabled.
    */
    WIFI_SetTxInterruptMode(WIFI_NO_INTR_SOURCES);

#if (WIFI_SCB_IRQ_INTERNAL)
    WIFI_ClearPendingInt();
#endif /* (WIFI_SCB_IRQ_INTERNAL) */
}


/*******************************************************************************
* Function Name: WIFI_SetRxFifoLevel
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
void WIFI_SetRxFifoLevel(uint32 level)
{
    uint32 rxFifoCtrl;

    rxFifoCtrl = WIFI_RX_FIFO_CTRL_REG;

    rxFifoCtrl &= ((uint32) ~WIFI_RX_FIFO_CTRL_TRIGGER_LEVEL_MASK); /* Clear level mask bits */
    rxFifoCtrl |= ((uint32) (WIFI_RX_FIFO_CTRL_TRIGGER_LEVEL_MASK & level));

    WIFI_RX_FIFO_CTRL_REG = rxFifoCtrl;
}


/*******************************************************************************
* Function Name: WIFI_SetTxFifoLevel
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
void WIFI_SetTxFifoLevel(uint32 level)
{
    uint32 txFifoCtrl;

    txFifoCtrl = WIFI_TX_FIFO_CTRL_REG;

    txFifoCtrl &= ((uint32) ~WIFI_TX_FIFO_CTRL_TRIGGER_LEVEL_MASK); /* Clear level mask bits */
    txFifoCtrl |= ((uint32) (WIFI_TX_FIFO_CTRL_TRIGGER_LEVEL_MASK & level));

    WIFI_TX_FIFO_CTRL_REG = txFifoCtrl;
}


#if (WIFI_SCB_IRQ_INTERNAL)
    /*******************************************************************************
    * Function Name: WIFI_SetCustomInterruptHandler
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
    void WIFI_SetCustomInterruptHandler(void (*func)(void))
    {
    #if !defined (CY_REMOVE_WIFI_CUSTOM_INTR_HANDLER)
        WIFI_customIntrHandler = func; /* Register interrupt handler */
    #else
        if (NULL != func)
        {
            /* Suppress compiler warning */
        }
    #endif /* !defined (CY_REMOVE_WIFI_CUSTOM_INTR_HANDLER) */
    }
#endif /* (WIFI_SCB_IRQ_INTERNAL) */


/*******************************************************************************
* Function Name: WIFI_ScbModeEnableIntr
****************************************************************************//**
*
*  Enables an interrupt for a specific mode.
*
*******************************************************************************/
static void WIFI_ScbEnableIntr(void)
{
#if (WIFI_SCB_IRQ_INTERNAL)
    /* Enable interrupt in NVIC */
    #if (WIFI_SCB_MODE_UNCONFIG_CONST_CFG)
        if (0u != WIFI_scbEnableIntr)
        {
            WIFI_EnableInt();
        }

    #else
        WIFI_EnableInt();

    #endif /* (WIFI_SCB_MODE_UNCONFIG_CONST_CFG) */
#endif /* (WIFI_SCB_IRQ_INTERNAL) */
}


/*******************************************************************************
* Function Name: WIFI_ScbModePostEnable
****************************************************************************//**
*
*  Calls the PostEnable function for a specific operation mode.
*
*******************************************************************************/
static void WIFI_ScbModePostEnable(void)
{
#if (WIFI_SCB_MODE_UNCONFIG_CONST_CFG)
#if (!WIFI_CY_SCBIP_V1)
    if (WIFI_SCB_MODE_SPI_RUNTM_CFG)
    {
        WIFI_SpiPostEnable();
    }
    else if (WIFI_SCB_MODE_UART_RUNTM_CFG)
    {
        WIFI_UartPostEnable();
    }
    else
    {
        /* Unknown mode: do nothing */
    }
#endif /* (!WIFI_CY_SCBIP_V1) */

#elif (WIFI_SCB_MODE_SPI_CONST_CFG)
    WIFI_SpiPostEnable();

#elif (WIFI_SCB_MODE_UART_CONST_CFG)
    WIFI_UartPostEnable();

#else
    /* Unknown mode: do nothing */
#endif /* (WIFI_SCB_MODE_UNCONFIG_CONST_CFG) */
}


/*******************************************************************************
* Function Name: WIFI_ScbModeStop
****************************************************************************//**
*
*  Calls the Stop function for a specific operation mode.
*
*******************************************************************************/
static void WIFI_ScbModeStop(void)
{
#if (WIFI_SCB_MODE_UNCONFIG_CONST_CFG)
    if (WIFI_SCB_MODE_I2C_RUNTM_CFG)
    {
        WIFI_I2CStop();
    }
    else if (WIFI_SCB_MODE_EZI2C_RUNTM_CFG)
    {
        WIFI_EzI2CStop();
    }
#if (!WIFI_CY_SCBIP_V1)
    else if (WIFI_SCB_MODE_SPI_RUNTM_CFG)
    {
        WIFI_SpiStop();
    }
    else if (WIFI_SCB_MODE_UART_RUNTM_CFG)
    {
        WIFI_UartStop();
    }
#endif /* (!WIFI_CY_SCBIP_V1) */
    else
    {
        /* Unknown mode: do nothing */
    }
#elif (WIFI_SCB_MODE_I2C_CONST_CFG)
    WIFI_I2CStop();

#elif (WIFI_SCB_MODE_EZI2C_CONST_CFG)
    WIFI_EzI2CStop();

#elif (WIFI_SCB_MODE_SPI_CONST_CFG)
    WIFI_SpiStop();

#elif (WIFI_SCB_MODE_UART_CONST_CFG)
    WIFI_UartStop();

#else
    /* Unknown mode: do nothing */
#endif /* (WIFI_SCB_MODE_UNCONFIG_CONST_CFG) */
}


#if (WIFI_SCB_MODE_UNCONFIG_CONST_CFG)
    /*******************************************************************************
    * Function Name: WIFI_SetPins
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
    void WIFI_SetPins(uint32 mode, uint32 subMode, uint32 uartEnableMask)
    {
        uint32 pinsDm[WIFI_SCB_PINS_NUMBER];
        uint32 i;
        
    #if (!WIFI_CY_SCBIP_V1)
        uint32 pinsInBuf = 0u;
    #endif /* (!WIFI_CY_SCBIP_V1) */
        
        uint32 hsiomSel[WIFI_SCB_PINS_NUMBER] = 
        {
            WIFI_RX_SCL_MOSI_HSIOM_SEL_GPIO,
            WIFI_TX_SDA_MISO_HSIOM_SEL_GPIO,
            0u,
            0u,
            0u,
            0u,
            0u,
        };

    #if (WIFI_CY_SCBIP_V1)
        /* Supress compiler warning. */
        if ((0u == subMode) || (0u == uartEnableMask))
        {
        }
    #endif /* (WIFI_CY_SCBIP_V1) */

        /* Set default HSIOM to GPIO and Drive Mode to Analog Hi-Z */
        for (i = 0u; i < WIFI_SCB_PINS_NUMBER; i++)
        {
            pinsDm[i] = WIFI_PIN_DM_ALG_HIZ;
        }

        if ((WIFI_SCB_MODE_I2C   == mode) ||
            (WIFI_SCB_MODE_EZI2C == mode))
        {
        #if (WIFI_RX_SCL_MOSI_PIN)
            hsiomSel[WIFI_RX_SCL_MOSI_PIN_INDEX] = WIFI_RX_SCL_MOSI_HSIOM_SEL_I2C;
            pinsDm  [WIFI_RX_SCL_MOSI_PIN_INDEX] = WIFI_PIN_DM_OD_LO;
        #elif (WIFI_RX_WAKE_SCL_MOSI_PIN)
            hsiomSel[WIFI_RX_WAKE_SCL_MOSI_PIN_INDEX] = WIFI_RX_WAKE_SCL_MOSI_HSIOM_SEL_I2C;
            pinsDm  [WIFI_RX_WAKE_SCL_MOSI_PIN_INDEX] = WIFI_PIN_DM_OD_LO;
        #else
        #endif /* (WIFI_RX_SCL_MOSI_PIN) */
        
        #if (WIFI_TX_SDA_MISO_PIN)
            hsiomSel[WIFI_TX_SDA_MISO_PIN_INDEX] = WIFI_TX_SDA_MISO_HSIOM_SEL_I2C;
            pinsDm  [WIFI_TX_SDA_MISO_PIN_INDEX] = WIFI_PIN_DM_OD_LO;
        #endif /* (WIFI_TX_SDA_MISO_PIN) */
        }
    #if (!WIFI_CY_SCBIP_V1)
        else if (WIFI_SCB_MODE_SPI == mode)
        {
        #if (WIFI_RX_SCL_MOSI_PIN)
            hsiomSel[WIFI_RX_SCL_MOSI_PIN_INDEX] = WIFI_RX_SCL_MOSI_HSIOM_SEL_SPI;
        #elif (WIFI_RX_WAKE_SCL_MOSI_PIN)
            hsiomSel[WIFI_RX_WAKE_SCL_MOSI_PIN_INDEX] = WIFI_RX_WAKE_SCL_MOSI_HSIOM_SEL_SPI;
        #else
        #endif /* (WIFI_RX_SCL_MOSI_PIN) */
        
        #if (WIFI_TX_SDA_MISO_PIN)
            hsiomSel[WIFI_TX_SDA_MISO_PIN_INDEX] = WIFI_TX_SDA_MISO_HSIOM_SEL_SPI;
        #endif /* (WIFI_TX_SDA_MISO_PIN) */
        
        #if (WIFI_CTS_SCLK_PIN)
            hsiomSel[WIFI_CTS_SCLK_PIN_INDEX] = WIFI_CTS_SCLK_HSIOM_SEL_SPI;
        #endif /* (WIFI_CTS_SCLK_PIN) */

            if (WIFI_SPI_SLAVE == subMode)
            {
                /* Slave */
                pinsDm[WIFI_RX_SCL_MOSI_PIN_INDEX] = WIFI_PIN_DM_DIG_HIZ;
                pinsDm[WIFI_TX_SDA_MISO_PIN_INDEX] = WIFI_PIN_DM_STRONG;
                pinsDm[WIFI_CTS_SCLK_PIN_INDEX] = WIFI_PIN_DM_DIG_HIZ;

            #if (WIFI_RTS_SS0_PIN)
                /* Only SS0 is valid choice for Slave */
                hsiomSel[WIFI_RTS_SS0_PIN_INDEX] = WIFI_RTS_SS0_HSIOM_SEL_SPI;
                pinsDm  [WIFI_RTS_SS0_PIN_INDEX] = WIFI_PIN_DM_DIG_HIZ;
            #endif /* (WIFI_RTS_SS0_PIN) */

            #if (WIFI_TX_SDA_MISO_PIN)
                /* Disable input buffer */
                 pinsInBuf |= WIFI_TX_SDA_MISO_PIN_MASK;
            #endif /* (WIFI_TX_SDA_MISO_PIN) */
            }
            else 
            {
                /* (Master) */
                pinsDm[WIFI_RX_SCL_MOSI_PIN_INDEX] = WIFI_PIN_DM_STRONG;
                pinsDm[WIFI_TX_SDA_MISO_PIN_INDEX] = WIFI_PIN_DM_DIG_HIZ;
                pinsDm[WIFI_CTS_SCLK_PIN_INDEX] = WIFI_PIN_DM_STRONG;

            #if (WIFI_RTS_SS0_PIN)
                hsiomSel [WIFI_RTS_SS0_PIN_INDEX] = WIFI_RTS_SS0_HSIOM_SEL_SPI;
                pinsDm   [WIFI_RTS_SS0_PIN_INDEX] = WIFI_PIN_DM_STRONG;
                pinsInBuf |= WIFI_RTS_SS0_PIN_MASK;
            #endif /* (WIFI_RTS_SS0_PIN) */

            #if (WIFI_SS1_PIN)
                hsiomSel [WIFI_SS1_PIN_INDEX] = WIFI_SS1_HSIOM_SEL_SPI;
                pinsDm   [WIFI_SS1_PIN_INDEX] = WIFI_PIN_DM_STRONG;
                pinsInBuf |= WIFI_SS1_PIN_MASK;
            #endif /* (WIFI_SS1_PIN) */

            #if (WIFI_SS2_PIN)
                hsiomSel [WIFI_SS2_PIN_INDEX] = WIFI_SS2_HSIOM_SEL_SPI;
                pinsDm   [WIFI_SS2_PIN_INDEX] = WIFI_PIN_DM_STRONG;
                pinsInBuf |= WIFI_SS2_PIN_MASK;
            #endif /* (WIFI_SS2_PIN) */

            #if (WIFI_SS3_PIN)
                hsiomSel [WIFI_SS3_PIN_INDEX] = WIFI_SS3_HSIOM_SEL_SPI;
                pinsDm   [WIFI_SS3_PIN_INDEX] = WIFI_PIN_DM_STRONG;
                pinsInBuf |= WIFI_SS3_PIN_MASK;
            #endif /* (WIFI_SS3_PIN) */

                /* Disable input buffers */
            #if (WIFI_RX_SCL_MOSI_PIN)
                pinsInBuf |= WIFI_RX_SCL_MOSI_PIN_MASK;
            #elif (WIFI_RX_WAKE_SCL_MOSI_PIN)
                pinsInBuf |= WIFI_RX_WAKE_SCL_MOSI_PIN_MASK;
            #else
            #endif /* (WIFI_RX_SCL_MOSI_PIN) */

            #if (WIFI_CTS_SCLK_PIN)
                pinsInBuf |= WIFI_CTS_SCLK_PIN_MASK;
            #endif /* (WIFI_CTS_SCLK_PIN) */
            }
        }
        else /* UART */
        {
            if (WIFI_UART_MODE_SMARTCARD == subMode)
            {
                /* SmartCard */
            #if (WIFI_TX_SDA_MISO_PIN)
                hsiomSel[WIFI_TX_SDA_MISO_PIN_INDEX] = WIFI_TX_SDA_MISO_HSIOM_SEL_UART;
                pinsDm  [WIFI_TX_SDA_MISO_PIN_INDEX] = WIFI_PIN_DM_OD_LO;
            #endif /* (WIFI_TX_SDA_MISO_PIN) */
            }
            else /* Standard or IrDA */
            {
                if (0u != (WIFI_UART_RX_PIN_ENABLE & uartEnableMask))
                {
                #if (WIFI_RX_SCL_MOSI_PIN)
                    hsiomSel[WIFI_RX_SCL_MOSI_PIN_INDEX] = WIFI_RX_SCL_MOSI_HSIOM_SEL_UART;
                    pinsDm  [WIFI_RX_SCL_MOSI_PIN_INDEX] = WIFI_PIN_DM_DIG_HIZ;
                #elif (WIFI_RX_WAKE_SCL_MOSI_PIN)
                    hsiomSel[WIFI_RX_WAKE_SCL_MOSI_PIN_INDEX] = WIFI_RX_WAKE_SCL_MOSI_HSIOM_SEL_UART;
                    pinsDm  [WIFI_RX_WAKE_SCL_MOSI_PIN_INDEX] = WIFI_PIN_DM_DIG_HIZ;
                #else
                #endif /* (WIFI_RX_SCL_MOSI_PIN) */
                }

                if (0u != (WIFI_UART_TX_PIN_ENABLE & uartEnableMask))
                {
                #if (WIFI_TX_SDA_MISO_PIN)
                    hsiomSel[WIFI_TX_SDA_MISO_PIN_INDEX] = WIFI_TX_SDA_MISO_HSIOM_SEL_UART;
                    pinsDm  [WIFI_TX_SDA_MISO_PIN_INDEX] = WIFI_PIN_DM_STRONG;
                    
                    /* Disable input buffer */
                    pinsInBuf |= WIFI_TX_SDA_MISO_PIN_MASK;
                #endif /* (WIFI_TX_SDA_MISO_PIN) */
                }

            #if !(WIFI_CY_SCBIP_V0 || WIFI_CY_SCBIP_V1)
                if (WIFI_UART_MODE_STD == subMode)
                {
                    if (0u != (WIFI_UART_CTS_PIN_ENABLE & uartEnableMask))
                    {
                        /* CTS input is multiplexed with SCLK */
                    #if (WIFI_CTS_SCLK_PIN)
                        hsiomSel[WIFI_CTS_SCLK_PIN_INDEX] = WIFI_CTS_SCLK_HSIOM_SEL_UART;
                        pinsDm  [WIFI_CTS_SCLK_PIN_INDEX] = WIFI_PIN_DM_DIG_HIZ;
                    #endif /* (WIFI_CTS_SCLK_PIN) */
                    }

                    if (0u != (WIFI_UART_RTS_PIN_ENABLE & uartEnableMask))
                    {
                        /* RTS output is multiplexed with SS0 */
                    #if (WIFI_RTS_SS0_PIN)
                        hsiomSel[WIFI_RTS_SS0_PIN_INDEX] = WIFI_RTS_SS0_HSIOM_SEL_UART;
                        pinsDm  [WIFI_RTS_SS0_PIN_INDEX] = WIFI_PIN_DM_STRONG;
                        
                        /* Disable input buffer */
                        pinsInBuf |= WIFI_RTS_SS0_PIN_MASK;
                    #endif /* (WIFI_RTS_SS0_PIN) */
                    }
                }
            #endif /* !(WIFI_CY_SCBIP_V0 || WIFI_CY_SCBIP_V1) */
            }
        }
    #endif /* (!WIFI_CY_SCBIP_V1) */

    /* Configure pins: set HSIOM, DM and InputBufEnable */
    /* Note: the DR register settings do not effect the pin output if HSIOM is other than GPIO */

    #if (WIFI_RX_SCL_MOSI_PIN)
        WIFI_SET_HSIOM_SEL(WIFI_RX_SCL_MOSI_HSIOM_REG,
                                       WIFI_RX_SCL_MOSI_HSIOM_MASK,
                                       WIFI_RX_SCL_MOSI_HSIOM_POS,
                                        hsiomSel[WIFI_RX_SCL_MOSI_PIN_INDEX]);

        WIFI_uart_rx_i2c_scl_spi_mosi_SetDriveMode((uint8) pinsDm[WIFI_RX_SCL_MOSI_PIN_INDEX]);

        #if (!WIFI_CY_SCBIP_V1)
            WIFI_SET_INP_DIS(WIFI_uart_rx_i2c_scl_spi_mosi_INP_DIS,
                                         WIFI_uart_rx_i2c_scl_spi_mosi_MASK,
                                         (0u != (pinsInBuf & WIFI_RX_SCL_MOSI_PIN_MASK)));
        #endif /* (!WIFI_CY_SCBIP_V1) */
    
    #elif (WIFI_RX_WAKE_SCL_MOSI_PIN)
        WIFI_SET_HSIOM_SEL(WIFI_RX_WAKE_SCL_MOSI_HSIOM_REG,
                                       WIFI_RX_WAKE_SCL_MOSI_HSIOM_MASK,
                                       WIFI_RX_WAKE_SCL_MOSI_HSIOM_POS,
                                       hsiomSel[WIFI_RX_WAKE_SCL_MOSI_PIN_INDEX]);

        WIFI_uart_rx_wake_i2c_scl_spi_mosi_SetDriveMode((uint8)
                                                               pinsDm[WIFI_RX_WAKE_SCL_MOSI_PIN_INDEX]);

        WIFI_SET_INP_DIS(WIFI_uart_rx_wake_i2c_scl_spi_mosi_INP_DIS,
                                     WIFI_uart_rx_wake_i2c_scl_spi_mosi_MASK,
                                     (0u != (pinsInBuf & WIFI_RX_WAKE_SCL_MOSI_PIN_MASK)));

         /* Set interrupt on falling edge */
        WIFI_SET_INCFG_TYPE(WIFI_RX_WAKE_SCL_MOSI_INTCFG_REG,
                                        WIFI_RX_WAKE_SCL_MOSI_INTCFG_TYPE_MASK,
                                        WIFI_RX_WAKE_SCL_MOSI_INTCFG_TYPE_POS,
                                        WIFI_INTCFG_TYPE_FALLING_EDGE);
    #else
    #endif /* (WIFI_RX_WAKE_SCL_MOSI_PIN) */

    #if (WIFI_TX_SDA_MISO_PIN)
        WIFI_SET_HSIOM_SEL(WIFI_TX_SDA_MISO_HSIOM_REG,
                                       WIFI_TX_SDA_MISO_HSIOM_MASK,
                                       WIFI_TX_SDA_MISO_HSIOM_POS,
                                        hsiomSel[WIFI_TX_SDA_MISO_PIN_INDEX]);

        WIFI_uart_tx_i2c_sda_spi_miso_SetDriveMode((uint8) pinsDm[WIFI_TX_SDA_MISO_PIN_INDEX]);

    #if (!WIFI_CY_SCBIP_V1)
        WIFI_SET_INP_DIS(WIFI_uart_tx_i2c_sda_spi_miso_INP_DIS,
                                     WIFI_uart_tx_i2c_sda_spi_miso_MASK,
                                    (0u != (pinsInBuf & WIFI_TX_SDA_MISO_PIN_MASK)));
    #endif /* (!WIFI_CY_SCBIP_V1) */
    #endif /* (WIFI_RX_SCL_MOSI_PIN) */

    #if (WIFI_CTS_SCLK_PIN)
        WIFI_SET_HSIOM_SEL(WIFI_CTS_SCLK_HSIOM_REG,
                                       WIFI_CTS_SCLK_HSIOM_MASK,
                                       WIFI_CTS_SCLK_HSIOM_POS,
                                       hsiomSel[WIFI_CTS_SCLK_PIN_INDEX]);

        WIFI_uart_cts_spi_sclk_SetDriveMode((uint8) pinsDm[WIFI_CTS_SCLK_PIN_INDEX]);

        WIFI_SET_INP_DIS(WIFI_uart_cts_spi_sclk_INP_DIS,
                                     WIFI_uart_cts_spi_sclk_MASK,
                                     (0u != (pinsInBuf & WIFI_CTS_SCLK_PIN_MASK)));
    #endif /* (WIFI_CTS_SCLK_PIN) */

    #if (WIFI_RTS_SS0_PIN)
        WIFI_SET_HSIOM_SEL(WIFI_RTS_SS0_HSIOM_REG,
                                       WIFI_RTS_SS0_HSIOM_MASK,
                                       WIFI_RTS_SS0_HSIOM_POS,
                                       hsiomSel[WIFI_RTS_SS0_PIN_INDEX]);

        WIFI_uart_rts_spi_ss0_SetDriveMode((uint8) pinsDm[WIFI_RTS_SS0_PIN_INDEX]);

        WIFI_SET_INP_DIS(WIFI_uart_rts_spi_ss0_INP_DIS,
                                     WIFI_uart_rts_spi_ss0_MASK,
                                     (0u != (pinsInBuf & WIFI_RTS_SS0_PIN_MASK)));
    #endif /* (WIFI_RTS_SS0_PIN) */

    #if (WIFI_SS1_PIN)
        WIFI_SET_HSIOM_SEL(WIFI_SS1_HSIOM_REG,
                                       WIFI_SS1_HSIOM_MASK,
                                       WIFI_SS1_HSIOM_POS,
                                       hsiomSel[WIFI_SS1_PIN_INDEX]);

        WIFI_spi_ss1_SetDriveMode((uint8) pinsDm[WIFI_SS1_PIN_INDEX]);

        WIFI_SET_INP_DIS(WIFI_spi_ss1_INP_DIS,
                                     WIFI_spi_ss1_MASK,
                                     (0u != (pinsInBuf & WIFI_SS1_PIN_MASK)));
    #endif /* (WIFI_SS1_PIN) */

    #if (WIFI_SS2_PIN)
        WIFI_SET_HSIOM_SEL(WIFI_SS2_HSIOM_REG,
                                       WIFI_SS2_HSIOM_MASK,
                                       WIFI_SS2_HSIOM_POS,
                                       hsiomSel[WIFI_SS2_PIN_INDEX]);

        WIFI_spi_ss2_SetDriveMode((uint8) pinsDm[WIFI_SS2_PIN_INDEX]);

        WIFI_SET_INP_DIS(WIFI_spi_ss2_INP_DIS,
                                     WIFI_spi_ss2_MASK,
                                     (0u != (pinsInBuf & WIFI_SS2_PIN_MASK)));
    #endif /* (WIFI_SS2_PIN) */

    #if (WIFI_SS3_PIN)
        WIFI_SET_HSIOM_SEL(WIFI_SS3_HSIOM_REG,
                                       WIFI_SS3_HSIOM_MASK,
                                       WIFI_SS3_HSIOM_POS,
                                       hsiomSel[WIFI_SS3_PIN_INDEX]);

        WIFI_spi_ss3_SetDriveMode((uint8) pinsDm[WIFI_SS3_PIN_INDEX]);

        WIFI_SET_INP_DIS(WIFI_spi_ss3_INP_DIS,
                                     WIFI_spi_ss3_MASK,
                                     (0u != (pinsInBuf & WIFI_SS3_PIN_MASK)));
    #endif /* (WIFI_SS3_PIN) */
    }

#endif /* (WIFI_SCB_MODE_UNCONFIG_CONST_CFG) */


#if (WIFI_CY_SCBIP_V0 || WIFI_CY_SCBIP_V1)
    /*******************************************************************************
    * Function Name: WIFI_I2CSlaveNackGeneration
    ****************************************************************************//**
    *
    *  Sets command to generate NACK to the address or data.
    *
    *******************************************************************************/
    void WIFI_I2CSlaveNackGeneration(void)
    {
        /* Check for EC_AM toggle condition: EC_AM and clock stretching for address are enabled */
        if ((0u != (WIFI_CTRL_REG & WIFI_CTRL_EC_AM_MODE)) &&
            (0u == (WIFI_I2C_CTRL_REG & WIFI_I2C_CTRL_S_NOT_READY_ADDR_NACK)))
        {
            /* Toggle EC_AM before NACK generation */
            WIFI_CTRL_REG &= ~WIFI_CTRL_EC_AM_MODE;
            WIFI_CTRL_REG |=  WIFI_CTRL_EC_AM_MODE;
        }

        WIFI_I2C_SLAVE_CMD_REG = WIFI_I2C_SLAVE_CMD_S_NACK;
    }
#endif /* (WIFI_CY_SCBIP_V0 || WIFI_CY_SCBIP_V1) */


/* [] END OF FILE */
