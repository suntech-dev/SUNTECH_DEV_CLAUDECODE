/***************************************************************************//**
* \file port_OP.c
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

#include "port_OP_PVT.h"

#if (port_OP_SCB_MODE_I2C_INC)
    #include "port_OP_I2C_PVT.h"
#endif /* (port_OP_SCB_MODE_I2C_INC) */

#if (port_OP_SCB_MODE_EZI2C_INC)
    #include "port_OP_EZI2C_PVT.h"
#endif /* (port_OP_SCB_MODE_EZI2C_INC) */

#if (port_OP_SCB_MODE_SPI_INC || port_OP_SCB_MODE_UART_INC)
    #include "port_OP_SPI_UART_PVT.h"
#endif /* (port_OP_SCB_MODE_SPI_INC || port_OP_SCB_MODE_UART_INC) */


/***************************************
*    Run Time Configuration Vars
***************************************/

/* Stores internal component configuration for Unconfigured mode */
#if (port_OP_SCB_MODE_UNCONFIG_CONST_CFG)
    /* Common configuration variables */
    uint8 port_OP_scbMode = port_OP_SCB_MODE_UNCONFIG;
    uint8 port_OP_scbEnableWake;
    uint8 port_OP_scbEnableIntr;

    /* I2C configuration variables */
    uint8 port_OP_mode;
    uint8 port_OP_acceptAddr;

    /* SPI/UART configuration variables */
    volatile uint8 * port_OP_rxBuffer;
    uint8  port_OP_rxDataBits;
    uint32 port_OP_rxBufferSize;

    volatile uint8 * port_OP_txBuffer;
    uint8  port_OP_txDataBits;
    uint32 port_OP_txBufferSize;

    /* EZI2C configuration variables */
    uint8 port_OP_numberOfAddr;
    uint8 port_OP_subAddrSize;
#endif /* (port_OP_SCB_MODE_UNCONFIG_CONST_CFG) */


/***************************************
*     Common SCB Vars
***************************************/
/**
* \addtogroup group_general
* \{
*/

/** port_OP_initVar indicates whether the port_OP 
*  component has been initialized. The variable is initialized to 0 
*  and set to 1 the first time SCB_Start() is called. This allows 
*  the component to restart without reinitialization after the first 
*  call to the port_OP_Start() routine.
*
*  If re-initialization of the component is required, then the 
*  port_OP_Init() function can be called before the 
*  port_OP_Start() or port_OP_Enable() function.
*/
uint8 port_OP_initVar = 0u;


#if (! (port_OP_SCB_MODE_I2C_CONST_CFG || \
        port_OP_SCB_MODE_EZI2C_CONST_CFG))
    /** This global variable stores TX interrupt sources after 
    * port_OP_Stop() is called. Only these TX interrupt sources 
    * will be restored on a subsequent port_OP_Enable() call.
    */
    uint16 port_OP_IntrTxMask = 0u;
#endif /* (! (port_OP_SCB_MODE_I2C_CONST_CFG || \
              port_OP_SCB_MODE_EZI2C_CONST_CFG)) */
/** \} globals */

#if (port_OP_SCB_IRQ_INTERNAL)
#if !defined (CY_REMOVE_port_OP_CUSTOM_INTR_HANDLER)
    void (*port_OP_customIntrHandler)(void) = NULL;
#endif /* !defined (CY_REMOVE_port_OP_CUSTOM_INTR_HANDLER) */
#endif /* (port_OP_SCB_IRQ_INTERNAL) */


/***************************************
*    Private Function Prototypes
***************************************/

static void port_OP_ScbEnableIntr(void);
static void port_OP_ScbModeStop(void);
static void port_OP_ScbModePostEnable(void);


/*******************************************************************************
* Function Name: port_OP_Init
****************************************************************************//**
*
*  Initializes the port_OP component to operate in one of the selected
*  configurations: I2C, SPI, UART or EZI2C.
*  When the configuration is set to "Unconfigured SCB", this function does
*  not do any initialization. Use mode-specific initialization APIs instead:
*  port_OP_I2CInit, port_OP_SpiInit, 
*  port_OP_UartInit or port_OP_EzI2CInit.
*
*******************************************************************************/
void port_OP_Init(void)
{
#if (port_OP_SCB_MODE_UNCONFIG_CONST_CFG)
    if (port_OP_SCB_MODE_UNCONFIG_RUNTM_CFG)
    {
        port_OP_initVar = 0u;
    }
    else
    {
        /* Initialization was done before this function call */
    }

#elif (port_OP_SCB_MODE_I2C_CONST_CFG)
    port_OP_I2CInit();

#elif (port_OP_SCB_MODE_SPI_CONST_CFG)
    port_OP_SpiInit();

#elif (port_OP_SCB_MODE_UART_CONST_CFG)
    port_OP_UartInit();

#elif (port_OP_SCB_MODE_EZI2C_CONST_CFG)
    port_OP_EzI2CInit();

#endif /* (port_OP_SCB_MODE_UNCONFIG_CONST_CFG) */
}


/*******************************************************************************
* Function Name: port_OP_Enable
****************************************************************************//**
*
*  Enables port_OP component operation: activates the hardware and 
*  internal interrupt. It also restores TX interrupt sources disabled after the 
*  port_OP_Stop() function was called (note that level-triggered TX 
*  interrupt sources remain disabled to not cause code lock-up).
*  For I2C and EZI2C modes the interrupt is internal and mandatory for 
*  operation. For SPI and UART modes the interrupt can be configured as none, 
*  internal or external.
*  The port_OP configuration should be not changed when the component
*  is enabled. Any configuration changes should be made after disabling the 
*  component.
*  When configuration is set to “Unconfigured port_OP”, the component 
*  must first be initialized to operate in one of the following configurations: 
*  I2C, SPI, UART or EZ I2C, using the mode-specific initialization API. 
*  Otherwise this function does not enable the component.
*
*******************************************************************************/
void port_OP_Enable(void)
{
#if (port_OP_SCB_MODE_UNCONFIG_CONST_CFG)
    /* Enable SCB block, only if it is already configured */
    if (!port_OP_SCB_MODE_UNCONFIG_RUNTM_CFG)
    {
        port_OP_CTRL_REG |= port_OP_CTRL_ENABLED;

        port_OP_ScbEnableIntr();

        /* Call PostEnable function specific to current operation mode */
        port_OP_ScbModePostEnable();
    }
#else
    port_OP_CTRL_REG |= port_OP_CTRL_ENABLED;

    port_OP_ScbEnableIntr();

    /* Call PostEnable function specific to current operation mode */
    port_OP_ScbModePostEnable();
#endif /* (port_OP_SCB_MODE_UNCONFIG_CONST_CFG) */
}


/*******************************************************************************
* Function Name: port_OP_Start
****************************************************************************//**
*
*  Invokes port_OP_Init() and port_OP_Enable().
*  After this function call, the component is enabled and ready for operation.
*  When configuration is set to "Unconfigured SCB", the component must first be
*  initialized to operate in one of the following configurations: I2C, SPI, UART
*  or EZI2C. Otherwise this function does not enable the component.
*
* \globalvars
*  port_OP_initVar - used to check initial configuration, modified
*  on first function call.
*
*******************************************************************************/
void port_OP_Start(void)
{
    if (0u == port_OP_initVar)
    {
        port_OP_Init();
        port_OP_initVar = 1u; /* Component was initialized */
    }

    port_OP_Enable();
}


/*******************************************************************************
* Function Name: port_OP_Stop
****************************************************************************//**
*
*  Disables the port_OP component: disable the hardware and internal 
*  interrupt. It also disables all TX interrupt sources so as not to cause an 
*  unexpected interrupt trigger because after the component is enabled, the 
*  TX FIFO is empty.
*  Refer to the function port_OP_Enable() for the interrupt 
*  configuration details.
*  This function disables the SCB component without checking to see if 
*  communication is in progress. Before calling this function it may be 
*  necessary to check the status of communication to make sure communication 
*  is complete. If this is not done then communication could be stopped mid 
*  byte and corrupted data could result.
*
*******************************************************************************/
void port_OP_Stop(void)
{
#if (port_OP_SCB_IRQ_INTERNAL)
    port_OP_DisableInt();
#endif /* (port_OP_SCB_IRQ_INTERNAL) */

    /* Call Stop function specific to current operation mode */
    port_OP_ScbModeStop();

    /* Disable SCB IP */
    port_OP_CTRL_REG &= (uint32) ~port_OP_CTRL_ENABLED;

    /* Disable all TX interrupt sources so as not to cause an unexpected
    * interrupt trigger after the component will be enabled because the 
    * TX FIFO is empty.
    * For SCB IP v0, it is critical as it does not mask-out interrupt
    * sources when it is disabled. This can cause a code lock-up in the
    * interrupt handler because TX FIFO cannot be loaded after the block
    * is disabled.
    */
    port_OP_SetTxInterruptMode(port_OP_NO_INTR_SOURCES);

#if (port_OP_SCB_IRQ_INTERNAL)
    port_OP_ClearPendingInt();
#endif /* (port_OP_SCB_IRQ_INTERNAL) */
}


/*******************************************************************************
* Function Name: port_OP_SetRxFifoLevel
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
void port_OP_SetRxFifoLevel(uint32 level)
{
    uint32 rxFifoCtrl;

    rxFifoCtrl = port_OP_RX_FIFO_CTRL_REG;

    rxFifoCtrl &= ((uint32) ~port_OP_RX_FIFO_CTRL_TRIGGER_LEVEL_MASK); /* Clear level mask bits */
    rxFifoCtrl |= ((uint32) (port_OP_RX_FIFO_CTRL_TRIGGER_LEVEL_MASK & level));

    port_OP_RX_FIFO_CTRL_REG = rxFifoCtrl;
}


/*******************************************************************************
* Function Name: port_OP_SetTxFifoLevel
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
void port_OP_SetTxFifoLevel(uint32 level)
{
    uint32 txFifoCtrl;

    txFifoCtrl = port_OP_TX_FIFO_CTRL_REG;

    txFifoCtrl &= ((uint32) ~port_OP_TX_FIFO_CTRL_TRIGGER_LEVEL_MASK); /* Clear level mask bits */
    txFifoCtrl |= ((uint32) (port_OP_TX_FIFO_CTRL_TRIGGER_LEVEL_MASK & level));

    port_OP_TX_FIFO_CTRL_REG = txFifoCtrl;
}


#if (port_OP_SCB_IRQ_INTERNAL)
    /*******************************************************************************
    * Function Name: port_OP_SetCustomInterruptHandler
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
    void port_OP_SetCustomInterruptHandler(void (*func)(void))
    {
    #if !defined (CY_REMOVE_port_OP_CUSTOM_INTR_HANDLER)
        port_OP_customIntrHandler = func; /* Register interrupt handler */
    #else
        if (NULL != func)
        {
            /* Suppress compiler warning */
        }
    #endif /* !defined (CY_REMOVE_port_OP_CUSTOM_INTR_HANDLER) */
    }
#endif /* (port_OP_SCB_IRQ_INTERNAL) */


/*******************************************************************************
* Function Name: port_OP_ScbModeEnableIntr
****************************************************************************//**
*
*  Enables an interrupt for a specific mode.
*
*******************************************************************************/
static void port_OP_ScbEnableIntr(void)
{
#if (port_OP_SCB_IRQ_INTERNAL)
    /* Enable interrupt in NVIC */
    #if (port_OP_SCB_MODE_UNCONFIG_CONST_CFG)
        if (0u != port_OP_scbEnableIntr)
        {
            port_OP_EnableInt();
        }

    #else
        port_OP_EnableInt();

    #endif /* (port_OP_SCB_MODE_UNCONFIG_CONST_CFG) */
#endif /* (port_OP_SCB_IRQ_INTERNAL) */
}


/*******************************************************************************
* Function Name: port_OP_ScbModePostEnable
****************************************************************************//**
*
*  Calls the PostEnable function for a specific operation mode.
*
*******************************************************************************/
static void port_OP_ScbModePostEnable(void)
{
#if (port_OP_SCB_MODE_UNCONFIG_CONST_CFG)
#if (!port_OP_CY_SCBIP_V1)
    if (port_OP_SCB_MODE_SPI_RUNTM_CFG)
    {
        port_OP_SpiPostEnable();
    }
    else if (port_OP_SCB_MODE_UART_RUNTM_CFG)
    {
        port_OP_UartPostEnable();
    }
    else
    {
        /* Unknown mode: do nothing */
    }
#endif /* (!port_OP_CY_SCBIP_V1) */

#elif (port_OP_SCB_MODE_SPI_CONST_CFG)
    port_OP_SpiPostEnable();

#elif (port_OP_SCB_MODE_UART_CONST_CFG)
    port_OP_UartPostEnable();

#else
    /* Unknown mode: do nothing */
#endif /* (port_OP_SCB_MODE_UNCONFIG_CONST_CFG) */
}


/*******************************************************************************
* Function Name: port_OP_ScbModeStop
****************************************************************************//**
*
*  Calls the Stop function for a specific operation mode.
*
*******************************************************************************/
static void port_OP_ScbModeStop(void)
{
#if (port_OP_SCB_MODE_UNCONFIG_CONST_CFG)
    if (port_OP_SCB_MODE_I2C_RUNTM_CFG)
    {
        port_OP_I2CStop();
    }
    else if (port_OP_SCB_MODE_EZI2C_RUNTM_CFG)
    {
        port_OP_EzI2CStop();
    }
#if (!port_OP_CY_SCBIP_V1)
    else if (port_OP_SCB_MODE_SPI_RUNTM_CFG)
    {
        port_OP_SpiStop();
    }
    else if (port_OP_SCB_MODE_UART_RUNTM_CFG)
    {
        port_OP_UartStop();
    }
#endif /* (!port_OP_CY_SCBIP_V1) */
    else
    {
        /* Unknown mode: do nothing */
    }
#elif (port_OP_SCB_MODE_I2C_CONST_CFG)
    port_OP_I2CStop();

#elif (port_OP_SCB_MODE_EZI2C_CONST_CFG)
    port_OP_EzI2CStop();

#elif (port_OP_SCB_MODE_SPI_CONST_CFG)
    port_OP_SpiStop();

#elif (port_OP_SCB_MODE_UART_CONST_CFG)
    port_OP_UartStop();

#else
    /* Unknown mode: do nothing */
#endif /* (port_OP_SCB_MODE_UNCONFIG_CONST_CFG) */
}


#if (port_OP_SCB_MODE_UNCONFIG_CONST_CFG)
    /*******************************************************************************
    * Function Name: port_OP_SetPins
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
    void port_OP_SetPins(uint32 mode, uint32 subMode, uint32 uartEnableMask)
    {
        uint32 pinsDm[port_OP_SCB_PINS_NUMBER];
        uint32 i;
        
    #if (!port_OP_CY_SCBIP_V1)
        uint32 pinsInBuf = 0u;
    #endif /* (!port_OP_CY_SCBIP_V1) */
        
        uint32 hsiomSel[port_OP_SCB_PINS_NUMBER] = 
        {
            port_OP_RX_SCL_MOSI_HSIOM_SEL_GPIO,
            port_OP_TX_SDA_MISO_HSIOM_SEL_GPIO,
            0u,
            0u,
            0u,
            0u,
            0u,
        };

    #if (port_OP_CY_SCBIP_V1)
        /* Supress compiler warning. */
        if ((0u == subMode) || (0u == uartEnableMask))
        {
        }
    #endif /* (port_OP_CY_SCBIP_V1) */

        /* Set default HSIOM to GPIO and Drive Mode to Analog Hi-Z */
        for (i = 0u; i < port_OP_SCB_PINS_NUMBER; i++)
        {
            pinsDm[i] = port_OP_PIN_DM_ALG_HIZ;
        }

        if ((port_OP_SCB_MODE_I2C   == mode) ||
            (port_OP_SCB_MODE_EZI2C == mode))
        {
        #if (port_OP_RX_SCL_MOSI_PIN)
            hsiomSel[port_OP_RX_SCL_MOSI_PIN_INDEX] = port_OP_RX_SCL_MOSI_HSIOM_SEL_I2C;
            pinsDm  [port_OP_RX_SCL_MOSI_PIN_INDEX] = port_OP_PIN_DM_OD_LO;
        #elif (port_OP_RX_WAKE_SCL_MOSI_PIN)
            hsiomSel[port_OP_RX_WAKE_SCL_MOSI_PIN_INDEX] = port_OP_RX_WAKE_SCL_MOSI_HSIOM_SEL_I2C;
            pinsDm  [port_OP_RX_WAKE_SCL_MOSI_PIN_INDEX] = port_OP_PIN_DM_OD_LO;
        #else
        #endif /* (port_OP_RX_SCL_MOSI_PIN) */
        
        #if (port_OP_TX_SDA_MISO_PIN)
            hsiomSel[port_OP_TX_SDA_MISO_PIN_INDEX] = port_OP_TX_SDA_MISO_HSIOM_SEL_I2C;
            pinsDm  [port_OP_TX_SDA_MISO_PIN_INDEX] = port_OP_PIN_DM_OD_LO;
        #endif /* (port_OP_TX_SDA_MISO_PIN) */
        }
    #if (!port_OP_CY_SCBIP_V1)
        else if (port_OP_SCB_MODE_SPI == mode)
        {
        #if (port_OP_RX_SCL_MOSI_PIN)
            hsiomSel[port_OP_RX_SCL_MOSI_PIN_INDEX] = port_OP_RX_SCL_MOSI_HSIOM_SEL_SPI;
        #elif (port_OP_RX_WAKE_SCL_MOSI_PIN)
            hsiomSel[port_OP_RX_WAKE_SCL_MOSI_PIN_INDEX] = port_OP_RX_WAKE_SCL_MOSI_HSIOM_SEL_SPI;
        #else
        #endif /* (port_OP_RX_SCL_MOSI_PIN) */
        
        #if (port_OP_TX_SDA_MISO_PIN)
            hsiomSel[port_OP_TX_SDA_MISO_PIN_INDEX] = port_OP_TX_SDA_MISO_HSIOM_SEL_SPI;
        #endif /* (port_OP_TX_SDA_MISO_PIN) */
        
        #if (port_OP_CTS_SCLK_PIN)
            hsiomSel[port_OP_CTS_SCLK_PIN_INDEX] = port_OP_CTS_SCLK_HSIOM_SEL_SPI;
        #endif /* (port_OP_CTS_SCLK_PIN) */

            if (port_OP_SPI_SLAVE == subMode)
            {
                /* Slave */
                pinsDm[port_OP_RX_SCL_MOSI_PIN_INDEX] = port_OP_PIN_DM_DIG_HIZ;
                pinsDm[port_OP_TX_SDA_MISO_PIN_INDEX] = port_OP_PIN_DM_STRONG;
                pinsDm[port_OP_CTS_SCLK_PIN_INDEX] = port_OP_PIN_DM_DIG_HIZ;

            #if (port_OP_RTS_SS0_PIN)
                /* Only SS0 is valid choice for Slave */
                hsiomSel[port_OP_RTS_SS0_PIN_INDEX] = port_OP_RTS_SS0_HSIOM_SEL_SPI;
                pinsDm  [port_OP_RTS_SS0_PIN_INDEX] = port_OP_PIN_DM_DIG_HIZ;
            #endif /* (port_OP_RTS_SS0_PIN) */

            #if (port_OP_TX_SDA_MISO_PIN)
                /* Disable input buffer */
                 pinsInBuf |= port_OP_TX_SDA_MISO_PIN_MASK;
            #endif /* (port_OP_TX_SDA_MISO_PIN) */
            }
            else 
            {
                /* (Master) */
                pinsDm[port_OP_RX_SCL_MOSI_PIN_INDEX] = port_OP_PIN_DM_STRONG;
                pinsDm[port_OP_TX_SDA_MISO_PIN_INDEX] = port_OP_PIN_DM_DIG_HIZ;
                pinsDm[port_OP_CTS_SCLK_PIN_INDEX] = port_OP_PIN_DM_STRONG;

            #if (port_OP_RTS_SS0_PIN)
                hsiomSel [port_OP_RTS_SS0_PIN_INDEX] = port_OP_RTS_SS0_HSIOM_SEL_SPI;
                pinsDm   [port_OP_RTS_SS0_PIN_INDEX] = port_OP_PIN_DM_STRONG;
                pinsInBuf |= port_OP_RTS_SS0_PIN_MASK;
            #endif /* (port_OP_RTS_SS0_PIN) */

            #if (port_OP_SS1_PIN)
                hsiomSel [port_OP_SS1_PIN_INDEX] = port_OP_SS1_HSIOM_SEL_SPI;
                pinsDm   [port_OP_SS1_PIN_INDEX] = port_OP_PIN_DM_STRONG;
                pinsInBuf |= port_OP_SS1_PIN_MASK;
            #endif /* (port_OP_SS1_PIN) */

            #if (port_OP_SS2_PIN)
                hsiomSel [port_OP_SS2_PIN_INDEX] = port_OP_SS2_HSIOM_SEL_SPI;
                pinsDm   [port_OP_SS2_PIN_INDEX] = port_OP_PIN_DM_STRONG;
                pinsInBuf |= port_OP_SS2_PIN_MASK;
            #endif /* (port_OP_SS2_PIN) */

            #if (port_OP_SS3_PIN)
                hsiomSel [port_OP_SS3_PIN_INDEX] = port_OP_SS3_HSIOM_SEL_SPI;
                pinsDm   [port_OP_SS3_PIN_INDEX] = port_OP_PIN_DM_STRONG;
                pinsInBuf |= port_OP_SS3_PIN_MASK;
            #endif /* (port_OP_SS3_PIN) */

                /* Disable input buffers */
            #if (port_OP_RX_SCL_MOSI_PIN)
                pinsInBuf |= port_OP_RX_SCL_MOSI_PIN_MASK;
            #elif (port_OP_RX_WAKE_SCL_MOSI_PIN)
                pinsInBuf |= port_OP_RX_WAKE_SCL_MOSI_PIN_MASK;
            #else
            #endif /* (port_OP_RX_SCL_MOSI_PIN) */

            #if (port_OP_CTS_SCLK_PIN)
                pinsInBuf |= port_OP_CTS_SCLK_PIN_MASK;
            #endif /* (port_OP_CTS_SCLK_PIN) */
            }
        }
        else /* UART */
        {
            if (port_OP_UART_MODE_SMARTCARD == subMode)
            {
                /* SmartCard */
            #if (port_OP_TX_SDA_MISO_PIN)
                hsiomSel[port_OP_TX_SDA_MISO_PIN_INDEX] = port_OP_TX_SDA_MISO_HSIOM_SEL_UART;
                pinsDm  [port_OP_TX_SDA_MISO_PIN_INDEX] = port_OP_PIN_DM_OD_LO;
            #endif /* (port_OP_TX_SDA_MISO_PIN) */
            }
            else /* Standard or IrDA */
            {
                if (0u != (port_OP_UART_RX_PIN_ENABLE & uartEnableMask))
                {
                #if (port_OP_RX_SCL_MOSI_PIN)
                    hsiomSel[port_OP_RX_SCL_MOSI_PIN_INDEX] = port_OP_RX_SCL_MOSI_HSIOM_SEL_UART;
                    pinsDm  [port_OP_RX_SCL_MOSI_PIN_INDEX] = port_OP_PIN_DM_DIG_HIZ;
                #elif (port_OP_RX_WAKE_SCL_MOSI_PIN)
                    hsiomSel[port_OP_RX_WAKE_SCL_MOSI_PIN_INDEX] = port_OP_RX_WAKE_SCL_MOSI_HSIOM_SEL_UART;
                    pinsDm  [port_OP_RX_WAKE_SCL_MOSI_PIN_INDEX] = port_OP_PIN_DM_DIG_HIZ;
                #else
                #endif /* (port_OP_RX_SCL_MOSI_PIN) */
                }

                if (0u != (port_OP_UART_TX_PIN_ENABLE & uartEnableMask))
                {
                #if (port_OP_TX_SDA_MISO_PIN)
                    hsiomSel[port_OP_TX_SDA_MISO_PIN_INDEX] = port_OP_TX_SDA_MISO_HSIOM_SEL_UART;
                    pinsDm  [port_OP_TX_SDA_MISO_PIN_INDEX] = port_OP_PIN_DM_STRONG;
                    
                    /* Disable input buffer */
                    pinsInBuf |= port_OP_TX_SDA_MISO_PIN_MASK;
                #endif /* (port_OP_TX_SDA_MISO_PIN) */
                }

            #if !(port_OP_CY_SCBIP_V0 || port_OP_CY_SCBIP_V1)
                if (port_OP_UART_MODE_STD == subMode)
                {
                    if (0u != (port_OP_UART_CTS_PIN_ENABLE & uartEnableMask))
                    {
                        /* CTS input is multiplexed with SCLK */
                    #if (port_OP_CTS_SCLK_PIN)
                        hsiomSel[port_OP_CTS_SCLK_PIN_INDEX] = port_OP_CTS_SCLK_HSIOM_SEL_UART;
                        pinsDm  [port_OP_CTS_SCLK_PIN_INDEX] = port_OP_PIN_DM_DIG_HIZ;
                    #endif /* (port_OP_CTS_SCLK_PIN) */
                    }

                    if (0u != (port_OP_UART_RTS_PIN_ENABLE & uartEnableMask))
                    {
                        /* RTS output is multiplexed with SS0 */
                    #if (port_OP_RTS_SS0_PIN)
                        hsiomSel[port_OP_RTS_SS0_PIN_INDEX] = port_OP_RTS_SS0_HSIOM_SEL_UART;
                        pinsDm  [port_OP_RTS_SS0_PIN_INDEX] = port_OP_PIN_DM_STRONG;
                        
                        /* Disable input buffer */
                        pinsInBuf |= port_OP_RTS_SS0_PIN_MASK;
                    #endif /* (port_OP_RTS_SS0_PIN) */
                    }
                }
            #endif /* !(port_OP_CY_SCBIP_V0 || port_OP_CY_SCBIP_V1) */
            }
        }
    #endif /* (!port_OP_CY_SCBIP_V1) */

    /* Configure pins: set HSIOM, DM and InputBufEnable */
    /* Note: the DR register settings do not effect the pin output if HSIOM is other than GPIO */

    #if (port_OP_RX_SCL_MOSI_PIN)
        port_OP_SET_HSIOM_SEL(port_OP_RX_SCL_MOSI_HSIOM_REG,
                                       port_OP_RX_SCL_MOSI_HSIOM_MASK,
                                       port_OP_RX_SCL_MOSI_HSIOM_POS,
                                        hsiomSel[port_OP_RX_SCL_MOSI_PIN_INDEX]);

        port_OP_uart_rx_i2c_scl_spi_mosi_SetDriveMode((uint8) pinsDm[port_OP_RX_SCL_MOSI_PIN_INDEX]);

        #if (!port_OP_CY_SCBIP_V1)
            port_OP_SET_INP_DIS(port_OP_uart_rx_i2c_scl_spi_mosi_INP_DIS,
                                         port_OP_uart_rx_i2c_scl_spi_mosi_MASK,
                                         (0u != (pinsInBuf & port_OP_RX_SCL_MOSI_PIN_MASK)));
        #endif /* (!port_OP_CY_SCBIP_V1) */
    
    #elif (port_OP_RX_WAKE_SCL_MOSI_PIN)
        port_OP_SET_HSIOM_SEL(port_OP_RX_WAKE_SCL_MOSI_HSIOM_REG,
                                       port_OP_RX_WAKE_SCL_MOSI_HSIOM_MASK,
                                       port_OP_RX_WAKE_SCL_MOSI_HSIOM_POS,
                                       hsiomSel[port_OP_RX_WAKE_SCL_MOSI_PIN_INDEX]);

        port_OP_uart_rx_wake_i2c_scl_spi_mosi_SetDriveMode((uint8)
                                                               pinsDm[port_OP_RX_WAKE_SCL_MOSI_PIN_INDEX]);

        port_OP_SET_INP_DIS(port_OP_uart_rx_wake_i2c_scl_spi_mosi_INP_DIS,
                                     port_OP_uart_rx_wake_i2c_scl_spi_mosi_MASK,
                                     (0u != (pinsInBuf & port_OP_RX_WAKE_SCL_MOSI_PIN_MASK)));

         /* Set interrupt on falling edge */
        port_OP_SET_INCFG_TYPE(port_OP_RX_WAKE_SCL_MOSI_INTCFG_REG,
                                        port_OP_RX_WAKE_SCL_MOSI_INTCFG_TYPE_MASK,
                                        port_OP_RX_WAKE_SCL_MOSI_INTCFG_TYPE_POS,
                                        port_OP_INTCFG_TYPE_FALLING_EDGE);
    #else
    #endif /* (port_OP_RX_WAKE_SCL_MOSI_PIN) */

    #if (port_OP_TX_SDA_MISO_PIN)
        port_OP_SET_HSIOM_SEL(port_OP_TX_SDA_MISO_HSIOM_REG,
                                       port_OP_TX_SDA_MISO_HSIOM_MASK,
                                       port_OP_TX_SDA_MISO_HSIOM_POS,
                                        hsiomSel[port_OP_TX_SDA_MISO_PIN_INDEX]);

        port_OP_uart_tx_i2c_sda_spi_miso_SetDriveMode((uint8) pinsDm[port_OP_TX_SDA_MISO_PIN_INDEX]);

    #if (!port_OP_CY_SCBIP_V1)
        port_OP_SET_INP_DIS(port_OP_uart_tx_i2c_sda_spi_miso_INP_DIS,
                                     port_OP_uart_tx_i2c_sda_spi_miso_MASK,
                                    (0u != (pinsInBuf & port_OP_TX_SDA_MISO_PIN_MASK)));
    #endif /* (!port_OP_CY_SCBIP_V1) */
    #endif /* (port_OP_RX_SCL_MOSI_PIN) */

    #if (port_OP_CTS_SCLK_PIN)
        port_OP_SET_HSIOM_SEL(port_OP_CTS_SCLK_HSIOM_REG,
                                       port_OP_CTS_SCLK_HSIOM_MASK,
                                       port_OP_CTS_SCLK_HSIOM_POS,
                                       hsiomSel[port_OP_CTS_SCLK_PIN_INDEX]);

        port_OP_uart_cts_spi_sclk_SetDriveMode((uint8) pinsDm[port_OP_CTS_SCLK_PIN_INDEX]);

        port_OP_SET_INP_DIS(port_OP_uart_cts_spi_sclk_INP_DIS,
                                     port_OP_uart_cts_spi_sclk_MASK,
                                     (0u != (pinsInBuf & port_OP_CTS_SCLK_PIN_MASK)));
    #endif /* (port_OP_CTS_SCLK_PIN) */

    #if (port_OP_RTS_SS0_PIN)
        port_OP_SET_HSIOM_SEL(port_OP_RTS_SS0_HSIOM_REG,
                                       port_OP_RTS_SS0_HSIOM_MASK,
                                       port_OP_RTS_SS0_HSIOM_POS,
                                       hsiomSel[port_OP_RTS_SS0_PIN_INDEX]);

        port_OP_uart_rts_spi_ss0_SetDriveMode((uint8) pinsDm[port_OP_RTS_SS0_PIN_INDEX]);

        port_OP_SET_INP_DIS(port_OP_uart_rts_spi_ss0_INP_DIS,
                                     port_OP_uart_rts_spi_ss0_MASK,
                                     (0u != (pinsInBuf & port_OP_RTS_SS0_PIN_MASK)));
    #endif /* (port_OP_RTS_SS0_PIN) */

    #if (port_OP_SS1_PIN)
        port_OP_SET_HSIOM_SEL(port_OP_SS1_HSIOM_REG,
                                       port_OP_SS1_HSIOM_MASK,
                                       port_OP_SS1_HSIOM_POS,
                                       hsiomSel[port_OP_SS1_PIN_INDEX]);

        port_OP_spi_ss1_SetDriveMode((uint8) pinsDm[port_OP_SS1_PIN_INDEX]);

        port_OP_SET_INP_DIS(port_OP_spi_ss1_INP_DIS,
                                     port_OP_spi_ss1_MASK,
                                     (0u != (pinsInBuf & port_OP_SS1_PIN_MASK)));
    #endif /* (port_OP_SS1_PIN) */

    #if (port_OP_SS2_PIN)
        port_OP_SET_HSIOM_SEL(port_OP_SS2_HSIOM_REG,
                                       port_OP_SS2_HSIOM_MASK,
                                       port_OP_SS2_HSIOM_POS,
                                       hsiomSel[port_OP_SS2_PIN_INDEX]);

        port_OP_spi_ss2_SetDriveMode((uint8) pinsDm[port_OP_SS2_PIN_INDEX]);

        port_OP_SET_INP_DIS(port_OP_spi_ss2_INP_DIS,
                                     port_OP_spi_ss2_MASK,
                                     (0u != (pinsInBuf & port_OP_SS2_PIN_MASK)));
    #endif /* (port_OP_SS2_PIN) */

    #if (port_OP_SS3_PIN)
        port_OP_SET_HSIOM_SEL(port_OP_SS3_HSIOM_REG,
                                       port_OP_SS3_HSIOM_MASK,
                                       port_OP_SS3_HSIOM_POS,
                                       hsiomSel[port_OP_SS3_PIN_INDEX]);

        port_OP_spi_ss3_SetDriveMode((uint8) pinsDm[port_OP_SS3_PIN_INDEX]);

        port_OP_SET_INP_DIS(port_OP_spi_ss3_INP_DIS,
                                     port_OP_spi_ss3_MASK,
                                     (0u != (pinsInBuf & port_OP_SS3_PIN_MASK)));
    #endif /* (port_OP_SS3_PIN) */
    }

#endif /* (port_OP_SCB_MODE_UNCONFIG_CONST_CFG) */


#if (port_OP_CY_SCBIP_V0 || port_OP_CY_SCBIP_V1)
    /*******************************************************************************
    * Function Name: port_OP_I2CSlaveNackGeneration
    ****************************************************************************//**
    *
    *  Sets command to generate NACK to the address or data.
    *
    *******************************************************************************/
    void port_OP_I2CSlaveNackGeneration(void)
    {
        /* Check for EC_AM toggle condition: EC_AM and clock stretching for address are enabled */
        if ((0u != (port_OP_CTRL_REG & port_OP_CTRL_EC_AM_MODE)) &&
            (0u == (port_OP_I2C_CTRL_REG & port_OP_I2C_CTRL_S_NOT_READY_ADDR_NACK)))
        {
            /* Toggle EC_AM before NACK generation */
            port_OP_CTRL_REG &= ~port_OP_CTRL_EC_AM_MODE;
            port_OP_CTRL_REG |=  port_OP_CTRL_EC_AM_MODE;
        }

        port_OP_I2C_SLAVE_CMD_REG = port_OP_I2C_SLAVE_CMD_S_NACK;
    }
#endif /* (port_OP_CY_SCBIP_V0 || port_OP_CY_SCBIP_V1) */


/* [] END OF FILE */
