/***************************************************************************//**
* \file QR_SCANNER.c
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

#include "QR_SCANNER_PVT.h"

#if (QR_SCANNER_SCB_MODE_I2C_INC)
    #include "QR_SCANNER_I2C_PVT.h"
#endif /* (QR_SCANNER_SCB_MODE_I2C_INC) */

#if (QR_SCANNER_SCB_MODE_EZI2C_INC)
    #include "QR_SCANNER_EZI2C_PVT.h"
#endif /* (QR_SCANNER_SCB_MODE_EZI2C_INC) */

#if (QR_SCANNER_SCB_MODE_SPI_INC || QR_SCANNER_SCB_MODE_UART_INC)
    #include "QR_SCANNER_SPI_UART_PVT.h"
#endif /* (QR_SCANNER_SCB_MODE_SPI_INC || QR_SCANNER_SCB_MODE_UART_INC) */


/***************************************
*    Run Time Configuration Vars
***************************************/

/* Stores internal component configuration for Unconfigured mode */
#if (QR_SCANNER_SCB_MODE_UNCONFIG_CONST_CFG)
    /* Common configuration variables */
    uint8 QR_SCANNER_scbMode = QR_SCANNER_SCB_MODE_UNCONFIG;
    uint8 QR_SCANNER_scbEnableWake;
    uint8 QR_SCANNER_scbEnableIntr;

    /* I2C configuration variables */
    uint8 QR_SCANNER_mode;
    uint8 QR_SCANNER_acceptAddr;

    /* SPI/UART configuration variables */
    volatile uint8 * QR_SCANNER_rxBuffer;
    uint8  QR_SCANNER_rxDataBits;
    uint32 QR_SCANNER_rxBufferSize;

    volatile uint8 * QR_SCANNER_txBuffer;
    uint8  QR_SCANNER_txDataBits;
    uint32 QR_SCANNER_txBufferSize;

    /* EZI2C configuration variables */
    uint8 QR_SCANNER_numberOfAddr;
    uint8 QR_SCANNER_subAddrSize;
#endif /* (QR_SCANNER_SCB_MODE_UNCONFIG_CONST_CFG) */


/***************************************
*     Common SCB Vars
***************************************/
/**
* \addtogroup group_general
* \{
*/

/** QR_SCANNER_initVar indicates whether the QR_SCANNER 
*  component has been initialized. The variable is initialized to 0 
*  and set to 1 the first time SCB_Start() is called. This allows 
*  the component to restart without reinitialization after the first 
*  call to the QR_SCANNER_Start() routine.
*
*  If re-initialization of the component is required, then the 
*  QR_SCANNER_Init() function can be called before the 
*  QR_SCANNER_Start() or QR_SCANNER_Enable() function.
*/
uint8 QR_SCANNER_initVar = 0u;


#if (! (QR_SCANNER_SCB_MODE_I2C_CONST_CFG || \
        QR_SCANNER_SCB_MODE_EZI2C_CONST_CFG))
    /** This global variable stores TX interrupt sources after 
    * QR_SCANNER_Stop() is called. Only these TX interrupt sources 
    * will be restored on a subsequent QR_SCANNER_Enable() call.
    */
    uint16 QR_SCANNER_IntrTxMask = 0u;
#endif /* (! (QR_SCANNER_SCB_MODE_I2C_CONST_CFG || \
              QR_SCANNER_SCB_MODE_EZI2C_CONST_CFG)) */
/** \} globals */

#if (QR_SCANNER_SCB_IRQ_INTERNAL)
#if !defined (CY_REMOVE_QR_SCANNER_CUSTOM_INTR_HANDLER)
    void (*QR_SCANNER_customIntrHandler)(void) = NULL;
#endif /* !defined (CY_REMOVE_QR_SCANNER_CUSTOM_INTR_HANDLER) */
#endif /* (QR_SCANNER_SCB_IRQ_INTERNAL) */


/***************************************
*    Private Function Prototypes
***************************************/

static void QR_SCANNER_ScbEnableIntr(void);
static void QR_SCANNER_ScbModeStop(void);
static void QR_SCANNER_ScbModePostEnable(void);


/*******************************************************************************
* Function Name: QR_SCANNER_Init
****************************************************************************//**
*
*  Initializes the QR_SCANNER component to operate in one of the selected
*  configurations: I2C, SPI, UART or EZI2C.
*  When the configuration is set to "Unconfigured SCB", this function does
*  not do any initialization. Use mode-specific initialization APIs instead:
*  QR_SCANNER_I2CInit, QR_SCANNER_SpiInit, 
*  QR_SCANNER_UartInit or QR_SCANNER_EzI2CInit.
*
*******************************************************************************/
void QR_SCANNER_Init(void)
{
#if (QR_SCANNER_SCB_MODE_UNCONFIG_CONST_CFG)
    if (QR_SCANNER_SCB_MODE_UNCONFIG_RUNTM_CFG)
    {
        QR_SCANNER_initVar = 0u;
    }
    else
    {
        /* Initialization was done before this function call */
    }

#elif (QR_SCANNER_SCB_MODE_I2C_CONST_CFG)
    QR_SCANNER_I2CInit();

#elif (QR_SCANNER_SCB_MODE_SPI_CONST_CFG)
    QR_SCANNER_SpiInit();

#elif (QR_SCANNER_SCB_MODE_UART_CONST_CFG)
    QR_SCANNER_UartInit();

#elif (QR_SCANNER_SCB_MODE_EZI2C_CONST_CFG)
    QR_SCANNER_EzI2CInit();

#endif /* (QR_SCANNER_SCB_MODE_UNCONFIG_CONST_CFG) */
}


/*******************************************************************************
* Function Name: QR_SCANNER_Enable
****************************************************************************//**
*
*  Enables QR_SCANNER component operation: activates the hardware and 
*  internal interrupt. It also restores TX interrupt sources disabled after the 
*  QR_SCANNER_Stop() function was called (note that level-triggered TX 
*  interrupt sources remain disabled to not cause code lock-up).
*  For I2C and EZI2C modes the interrupt is internal and mandatory for 
*  operation. For SPI and UART modes the interrupt can be configured as none, 
*  internal or external.
*  The QR_SCANNER configuration should be not changed when the component
*  is enabled. Any configuration changes should be made after disabling the 
*  component.
*  When configuration is set to “Unconfigured QR_SCANNER”, the component 
*  must first be initialized to operate in one of the following configurations: 
*  I2C, SPI, UART or EZ I2C, using the mode-specific initialization API. 
*  Otherwise this function does not enable the component.
*
*******************************************************************************/
void QR_SCANNER_Enable(void)
{
#if (QR_SCANNER_SCB_MODE_UNCONFIG_CONST_CFG)
    /* Enable SCB block, only if it is already configured */
    if (!QR_SCANNER_SCB_MODE_UNCONFIG_RUNTM_CFG)
    {
        QR_SCANNER_CTRL_REG |= QR_SCANNER_CTRL_ENABLED;

        QR_SCANNER_ScbEnableIntr();

        /* Call PostEnable function specific to current operation mode */
        QR_SCANNER_ScbModePostEnable();
    }
#else
    QR_SCANNER_CTRL_REG |= QR_SCANNER_CTRL_ENABLED;

    QR_SCANNER_ScbEnableIntr();

    /* Call PostEnable function specific to current operation mode */
    QR_SCANNER_ScbModePostEnable();
#endif /* (QR_SCANNER_SCB_MODE_UNCONFIG_CONST_CFG) */
}


/*******************************************************************************
* Function Name: QR_SCANNER_Start
****************************************************************************//**
*
*  Invokes QR_SCANNER_Init() and QR_SCANNER_Enable().
*  After this function call, the component is enabled and ready for operation.
*  When configuration is set to "Unconfigured SCB", the component must first be
*  initialized to operate in one of the following configurations: I2C, SPI, UART
*  or EZI2C. Otherwise this function does not enable the component.
*
* \globalvars
*  QR_SCANNER_initVar - used to check initial configuration, modified
*  on first function call.
*
*******************************************************************************/
void QR_SCANNER_Start(void)
{
    if (0u == QR_SCANNER_initVar)
    {
        QR_SCANNER_Init();
        QR_SCANNER_initVar = 1u; /* Component was initialized */
    }

    QR_SCANNER_Enable();
}


/*******************************************************************************
* Function Name: QR_SCANNER_Stop
****************************************************************************//**
*
*  Disables the QR_SCANNER component: disable the hardware and internal 
*  interrupt. It also disables all TX interrupt sources so as not to cause an 
*  unexpected interrupt trigger because after the component is enabled, the 
*  TX FIFO is empty.
*  Refer to the function QR_SCANNER_Enable() for the interrupt 
*  configuration details.
*  This function disables the SCB component without checking to see if 
*  communication is in progress. Before calling this function it may be 
*  necessary to check the status of communication to make sure communication 
*  is complete. If this is not done then communication could be stopped mid 
*  byte and corrupted data could result.
*
*******************************************************************************/
void QR_SCANNER_Stop(void)
{
#if (QR_SCANNER_SCB_IRQ_INTERNAL)
    QR_SCANNER_DisableInt();
#endif /* (QR_SCANNER_SCB_IRQ_INTERNAL) */

    /* Call Stop function specific to current operation mode */
    QR_SCANNER_ScbModeStop();

    /* Disable SCB IP */
    QR_SCANNER_CTRL_REG &= (uint32) ~QR_SCANNER_CTRL_ENABLED;

    /* Disable all TX interrupt sources so as not to cause an unexpected
    * interrupt trigger after the component will be enabled because the 
    * TX FIFO is empty.
    * For SCB IP v0, it is critical as it does not mask-out interrupt
    * sources when it is disabled. This can cause a code lock-up in the
    * interrupt handler because TX FIFO cannot be loaded after the block
    * is disabled.
    */
    QR_SCANNER_SetTxInterruptMode(QR_SCANNER_NO_INTR_SOURCES);

#if (QR_SCANNER_SCB_IRQ_INTERNAL)
    QR_SCANNER_ClearPendingInt();
#endif /* (QR_SCANNER_SCB_IRQ_INTERNAL) */
}


/*******************************************************************************
* Function Name: QR_SCANNER_SetRxFifoLevel
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
void QR_SCANNER_SetRxFifoLevel(uint32 level)
{
    uint32 rxFifoCtrl;

    rxFifoCtrl = QR_SCANNER_RX_FIFO_CTRL_REG;

    rxFifoCtrl &= ((uint32) ~QR_SCANNER_RX_FIFO_CTRL_TRIGGER_LEVEL_MASK); /* Clear level mask bits */
    rxFifoCtrl |= ((uint32) (QR_SCANNER_RX_FIFO_CTRL_TRIGGER_LEVEL_MASK & level));

    QR_SCANNER_RX_FIFO_CTRL_REG = rxFifoCtrl;
}


/*******************************************************************************
* Function Name: QR_SCANNER_SetTxFifoLevel
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
void QR_SCANNER_SetTxFifoLevel(uint32 level)
{
    uint32 txFifoCtrl;

    txFifoCtrl = QR_SCANNER_TX_FIFO_CTRL_REG;

    txFifoCtrl &= ((uint32) ~QR_SCANNER_TX_FIFO_CTRL_TRIGGER_LEVEL_MASK); /* Clear level mask bits */
    txFifoCtrl |= ((uint32) (QR_SCANNER_TX_FIFO_CTRL_TRIGGER_LEVEL_MASK & level));

    QR_SCANNER_TX_FIFO_CTRL_REG = txFifoCtrl;
}


#if (QR_SCANNER_SCB_IRQ_INTERNAL)
    /*******************************************************************************
    * Function Name: QR_SCANNER_SetCustomInterruptHandler
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
    void QR_SCANNER_SetCustomInterruptHandler(void (*func)(void))
    {
    #if !defined (CY_REMOVE_QR_SCANNER_CUSTOM_INTR_HANDLER)
        QR_SCANNER_customIntrHandler = func; /* Register interrupt handler */
    #else
        if (NULL != func)
        {
            /* Suppress compiler warning */
        }
    #endif /* !defined (CY_REMOVE_QR_SCANNER_CUSTOM_INTR_HANDLER) */
    }
#endif /* (QR_SCANNER_SCB_IRQ_INTERNAL) */


/*******************************************************************************
* Function Name: QR_SCANNER_ScbModeEnableIntr
****************************************************************************//**
*
*  Enables an interrupt for a specific mode.
*
*******************************************************************************/
static void QR_SCANNER_ScbEnableIntr(void)
{
#if (QR_SCANNER_SCB_IRQ_INTERNAL)
    /* Enable interrupt in NVIC */
    #if (QR_SCANNER_SCB_MODE_UNCONFIG_CONST_CFG)
        if (0u != QR_SCANNER_scbEnableIntr)
        {
            QR_SCANNER_EnableInt();
        }

    #else
        QR_SCANNER_EnableInt();

    #endif /* (QR_SCANNER_SCB_MODE_UNCONFIG_CONST_CFG) */
#endif /* (QR_SCANNER_SCB_IRQ_INTERNAL) */
}


/*******************************************************************************
* Function Name: QR_SCANNER_ScbModePostEnable
****************************************************************************//**
*
*  Calls the PostEnable function for a specific operation mode.
*
*******************************************************************************/
static void QR_SCANNER_ScbModePostEnable(void)
{
#if (QR_SCANNER_SCB_MODE_UNCONFIG_CONST_CFG)
#if (!QR_SCANNER_CY_SCBIP_V1)
    if (QR_SCANNER_SCB_MODE_SPI_RUNTM_CFG)
    {
        QR_SCANNER_SpiPostEnable();
    }
    else if (QR_SCANNER_SCB_MODE_UART_RUNTM_CFG)
    {
        QR_SCANNER_UartPostEnable();
    }
    else
    {
        /* Unknown mode: do nothing */
    }
#endif /* (!QR_SCANNER_CY_SCBIP_V1) */

#elif (QR_SCANNER_SCB_MODE_SPI_CONST_CFG)
    QR_SCANNER_SpiPostEnable();

#elif (QR_SCANNER_SCB_MODE_UART_CONST_CFG)
    QR_SCANNER_UartPostEnable();

#else
    /* Unknown mode: do nothing */
#endif /* (QR_SCANNER_SCB_MODE_UNCONFIG_CONST_CFG) */
}


/*******************************************************************************
* Function Name: QR_SCANNER_ScbModeStop
****************************************************************************//**
*
*  Calls the Stop function for a specific operation mode.
*
*******************************************************************************/
static void QR_SCANNER_ScbModeStop(void)
{
#if (QR_SCANNER_SCB_MODE_UNCONFIG_CONST_CFG)
    if (QR_SCANNER_SCB_MODE_I2C_RUNTM_CFG)
    {
        QR_SCANNER_I2CStop();
    }
    else if (QR_SCANNER_SCB_MODE_EZI2C_RUNTM_CFG)
    {
        QR_SCANNER_EzI2CStop();
    }
#if (!QR_SCANNER_CY_SCBIP_V1)
    else if (QR_SCANNER_SCB_MODE_SPI_RUNTM_CFG)
    {
        QR_SCANNER_SpiStop();
    }
    else if (QR_SCANNER_SCB_MODE_UART_RUNTM_CFG)
    {
        QR_SCANNER_UartStop();
    }
#endif /* (!QR_SCANNER_CY_SCBIP_V1) */
    else
    {
        /* Unknown mode: do nothing */
    }
#elif (QR_SCANNER_SCB_MODE_I2C_CONST_CFG)
    QR_SCANNER_I2CStop();

#elif (QR_SCANNER_SCB_MODE_EZI2C_CONST_CFG)
    QR_SCANNER_EzI2CStop();

#elif (QR_SCANNER_SCB_MODE_SPI_CONST_CFG)
    QR_SCANNER_SpiStop();

#elif (QR_SCANNER_SCB_MODE_UART_CONST_CFG)
    QR_SCANNER_UartStop();

#else
    /* Unknown mode: do nothing */
#endif /* (QR_SCANNER_SCB_MODE_UNCONFIG_CONST_CFG) */
}


#if (QR_SCANNER_SCB_MODE_UNCONFIG_CONST_CFG)
    /*******************************************************************************
    * Function Name: QR_SCANNER_SetPins
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
    void QR_SCANNER_SetPins(uint32 mode, uint32 subMode, uint32 uartEnableMask)
    {
        uint32 pinsDm[QR_SCANNER_SCB_PINS_NUMBER];
        uint32 i;
        
    #if (!QR_SCANNER_CY_SCBIP_V1)
        uint32 pinsInBuf = 0u;
    #endif /* (!QR_SCANNER_CY_SCBIP_V1) */
        
        uint32 hsiomSel[QR_SCANNER_SCB_PINS_NUMBER] = 
        {
            QR_SCANNER_RX_SCL_MOSI_HSIOM_SEL_GPIO,
            QR_SCANNER_TX_SDA_MISO_HSIOM_SEL_GPIO,
            0u,
            0u,
            0u,
            0u,
            0u,
        };

    #if (QR_SCANNER_CY_SCBIP_V1)
        /* Supress compiler warning. */
        if ((0u == subMode) || (0u == uartEnableMask))
        {
        }
    #endif /* (QR_SCANNER_CY_SCBIP_V1) */

        /* Set default HSIOM to GPIO and Drive Mode to Analog Hi-Z */
        for (i = 0u; i < QR_SCANNER_SCB_PINS_NUMBER; i++)
        {
            pinsDm[i] = QR_SCANNER_PIN_DM_ALG_HIZ;
        }

        if ((QR_SCANNER_SCB_MODE_I2C   == mode) ||
            (QR_SCANNER_SCB_MODE_EZI2C == mode))
        {
        #if (QR_SCANNER_RX_SCL_MOSI_PIN)
            hsiomSel[QR_SCANNER_RX_SCL_MOSI_PIN_INDEX] = QR_SCANNER_RX_SCL_MOSI_HSIOM_SEL_I2C;
            pinsDm  [QR_SCANNER_RX_SCL_MOSI_PIN_INDEX] = QR_SCANNER_PIN_DM_OD_LO;
        #elif (QR_SCANNER_RX_WAKE_SCL_MOSI_PIN)
            hsiomSel[QR_SCANNER_RX_WAKE_SCL_MOSI_PIN_INDEX] = QR_SCANNER_RX_WAKE_SCL_MOSI_HSIOM_SEL_I2C;
            pinsDm  [QR_SCANNER_RX_WAKE_SCL_MOSI_PIN_INDEX] = QR_SCANNER_PIN_DM_OD_LO;
        #else
        #endif /* (QR_SCANNER_RX_SCL_MOSI_PIN) */
        
        #if (QR_SCANNER_TX_SDA_MISO_PIN)
            hsiomSel[QR_SCANNER_TX_SDA_MISO_PIN_INDEX] = QR_SCANNER_TX_SDA_MISO_HSIOM_SEL_I2C;
            pinsDm  [QR_SCANNER_TX_SDA_MISO_PIN_INDEX] = QR_SCANNER_PIN_DM_OD_LO;
        #endif /* (QR_SCANNER_TX_SDA_MISO_PIN) */
        }
    #if (!QR_SCANNER_CY_SCBIP_V1)
        else if (QR_SCANNER_SCB_MODE_SPI == mode)
        {
        #if (QR_SCANNER_RX_SCL_MOSI_PIN)
            hsiomSel[QR_SCANNER_RX_SCL_MOSI_PIN_INDEX] = QR_SCANNER_RX_SCL_MOSI_HSIOM_SEL_SPI;
        #elif (QR_SCANNER_RX_WAKE_SCL_MOSI_PIN)
            hsiomSel[QR_SCANNER_RX_WAKE_SCL_MOSI_PIN_INDEX] = QR_SCANNER_RX_WAKE_SCL_MOSI_HSIOM_SEL_SPI;
        #else
        #endif /* (QR_SCANNER_RX_SCL_MOSI_PIN) */
        
        #if (QR_SCANNER_TX_SDA_MISO_PIN)
            hsiomSel[QR_SCANNER_TX_SDA_MISO_PIN_INDEX] = QR_SCANNER_TX_SDA_MISO_HSIOM_SEL_SPI;
        #endif /* (QR_SCANNER_TX_SDA_MISO_PIN) */
        
        #if (QR_SCANNER_CTS_SCLK_PIN)
            hsiomSel[QR_SCANNER_CTS_SCLK_PIN_INDEX] = QR_SCANNER_CTS_SCLK_HSIOM_SEL_SPI;
        #endif /* (QR_SCANNER_CTS_SCLK_PIN) */

            if (QR_SCANNER_SPI_SLAVE == subMode)
            {
                /* Slave */
                pinsDm[QR_SCANNER_RX_SCL_MOSI_PIN_INDEX] = QR_SCANNER_PIN_DM_DIG_HIZ;
                pinsDm[QR_SCANNER_TX_SDA_MISO_PIN_INDEX] = QR_SCANNER_PIN_DM_STRONG;
                pinsDm[QR_SCANNER_CTS_SCLK_PIN_INDEX] = QR_SCANNER_PIN_DM_DIG_HIZ;

            #if (QR_SCANNER_RTS_SS0_PIN)
                /* Only SS0 is valid choice for Slave */
                hsiomSel[QR_SCANNER_RTS_SS0_PIN_INDEX] = QR_SCANNER_RTS_SS0_HSIOM_SEL_SPI;
                pinsDm  [QR_SCANNER_RTS_SS0_PIN_INDEX] = QR_SCANNER_PIN_DM_DIG_HIZ;
            #endif /* (QR_SCANNER_RTS_SS0_PIN) */

            #if (QR_SCANNER_TX_SDA_MISO_PIN)
                /* Disable input buffer */
                 pinsInBuf |= QR_SCANNER_TX_SDA_MISO_PIN_MASK;
            #endif /* (QR_SCANNER_TX_SDA_MISO_PIN) */
            }
            else 
            {
                /* (Master) */
                pinsDm[QR_SCANNER_RX_SCL_MOSI_PIN_INDEX] = QR_SCANNER_PIN_DM_STRONG;
                pinsDm[QR_SCANNER_TX_SDA_MISO_PIN_INDEX] = QR_SCANNER_PIN_DM_DIG_HIZ;
                pinsDm[QR_SCANNER_CTS_SCLK_PIN_INDEX] = QR_SCANNER_PIN_DM_STRONG;

            #if (QR_SCANNER_RTS_SS0_PIN)
                hsiomSel [QR_SCANNER_RTS_SS0_PIN_INDEX] = QR_SCANNER_RTS_SS0_HSIOM_SEL_SPI;
                pinsDm   [QR_SCANNER_RTS_SS0_PIN_INDEX] = QR_SCANNER_PIN_DM_STRONG;
                pinsInBuf |= QR_SCANNER_RTS_SS0_PIN_MASK;
            #endif /* (QR_SCANNER_RTS_SS0_PIN) */

            #if (QR_SCANNER_SS1_PIN)
                hsiomSel [QR_SCANNER_SS1_PIN_INDEX] = QR_SCANNER_SS1_HSIOM_SEL_SPI;
                pinsDm   [QR_SCANNER_SS1_PIN_INDEX] = QR_SCANNER_PIN_DM_STRONG;
                pinsInBuf |= QR_SCANNER_SS1_PIN_MASK;
            #endif /* (QR_SCANNER_SS1_PIN) */

            #if (QR_SCANNER_SS2_PIN)
                hsiomSel [QR_SCANNER_SS2_PIN_INDEX] = QR_SCANNER_SS2_HSIOM_SEL_SPI;
                pinsDm   [QR_SCANNER_SS2_PIN_INDEX] = QR_SCANNER_PIN_DM_STRONG;
                pinsInBuf |= QR_SCANNER_SS2_PIN_MASK;
            #endif /* (QR_SCANNER_SS2_PIN) */

            #if (QR_SCANNER_SS3_PIN)
                hsiomSel [QR_SCANNER_SS3_PIN_INDEX] = QR_SCANNER_SS3_HSIOM_SEL_SPI;
                pinsDm   [QR_SCANNER_SS3_PIN_INDEX] = QR_SCANNER_PIN_DM_STRONG;
                pinsInBuf |= QR_SCANNER_SS3_PIN_MASK;
            #endif /* (QR_SCANNER_SS3_PIN) */

                /* Disable input buffers */
            #if (QR_SCANNER_RX_SCL_MOSI_PIN)
                pinsInBuf |= QR_SCANNER_RX_SCL_MOSI_PIN_MASK;
            #elif (QR_SCANNER_RX_WAKE_SCL_MOSI_PIN)
                pinsInBuf |= QR_SCANNER_RX_WAKE_SCL_MOSI_PIN_MASK;
            #else
            #endif /* (QR_SCANNER_RX_SCL_MOSI_PIN) */

            #if (QR_SCANNER_CTS_SCLK_PIN)
                pinsInBuf |= QR_SCANNER_CTS_SCLK_PIN_MASK;
            #endif /* (QR_SCANNER_CTS_SCLK_PIN) */
            }
        }
        else /* UART */
        {
            if (QR_SCANNER_UART_MODE_SMARTCARD == subMode)
            {
                /* SmartCard */
            #if (QR_SCANNER_TX_SDA_MISO_PIN)
                hsiomSel[QR_SCANNER_TX_SDA_MISO_PIN_INDEX] = QR_SCANNER_TX_SDA_MISO_HSIOM_SEL_UART;
                pinsDm  [QR_SCANNER_TX_SDA_MISO_PIN_INDEX] = QR_SCANNER_PIN_DM_OD_LO;
            #endif /* (QR_SCANNER_TX_SDA_MISO_PIN) */
            }
            else /* Standard or IrDA */
            {
                if (0u != (QR_SCANNER_UART_RX_PIN_ENABLE & uartEnableMask))
                {
                #if (QR_SCANNER_RX_SCL_MOSI_PIN)
                    hsiomSel[QR_SCANNER_RX_SCL_MOSI_PIN_INDEX] = QR_SCANNER_RX_SCL_MOSI_HSIOM_SEL_UART;
                    pinsDm  [QR_SCANNER_RX_SCL_MOSI_PIN_INDEX] = QR_SCANNER_PIN_DM_DIG_HIZ;
                #elif (QR_SCANNER_RX_WAKE_SCL_MOSI_PIN)
                    hsiomSel[QR_SCANNER_RX_WAKE_SCL_MOSI_PIN_INDEX] = QR_SCANNER_RX_WAKE_SCL_MOSI_HSIOM_SEL_UART;
                    pinsDm  [QR_SCANNER_RX_WAKE_SCL_MOSI_PIN_INDEX] = QR_SCANNER_PIN_DM_DIG_HIZ;
                #else
                #endif /* (QR_SCANNER_RX_SCL_MOSI_PIN) */
                }

                if (0u != (QR_SCANNER_UART_TX_PIN_ENABLE & uartEnableMask))
                {
                #if (QR_SCANNER_TX_SDA_MISO_PIN)
                    hsiomSel[QR_SCANNER_TX_SDA_MISO_PIN_INDEX] = QR_SCANNER_TX_SDA_MISO_HSIOM_SEL_UART;
                    pinsDm  [QR_SCANNER_TX_SDA_MISO_PIN_INDEX] = QR_SCANNER_PIN_DM_STRONG;
                    
                    /* Disable input buffer */
                    pinsInBuf |= QR_SCANNER_TX_SDA_MISO_PIN_MASK;
                #endif /* (QR_SCANNER_TX_SDA_MISO_PIN) */
                }

            #if !(QR_SCANNER_CY_SCBIP_V0 || QR_SCANNER_CY_SCBIP_V1)
                if (QR_SCANNER_UART_MODE_STD == subMode)
                {
                    if (0u != (QR_SCANNER_UART_CTS_PIN_ENABLE & uartEnableMask))
                    {
                        /* CTS input is multiplexed with SCLK */
                    #if (QR_SCANNER_CTS_SCLK_PIN)
                        hsiomSel[QR_SCANNER_CTS_SCLK_PIN_INDEX] = QR_SCANNER_CTS_SCLK_HSIOM_SEL_UART;
                        pinsDm  [QR_SCANNER_CTS_SCLK_PIN_INDEX] = QR_SCANNER_PIN_DM_DIG_HIZ;
                    #endif /* (QR_SCANNER_CTS_SCLK_PIN) */
                    }

                    if (0u != (QR_SCANNER_UART_RTS_PIN_ENABLE & uartEnableMask))
                    {
                        /* RTS output is multiplexed with SS0 */
                    #if (QR_SCANNER_RTS_SS0_PIN)
                        hsiomSel[QR_SCANNER_RTS_SS0_PIN_INDEX] = QR_SCANNER_RTS_SS0_HSIOM_SEL_UART;
                        pinsDm  [QR_SCANNER_RTS_SS0_PIN_INDEX] = QR_SCANNER_PIN_DM_STRONG;
                        
                        /* Disable input buffer */
                        pinsInBuf |= QR_SCANNER_RTS_SS0_PIN_MASK;
                    #endif /* (QR_SCANNER_RTS_SS0_PIN) */
                    }
                }
            #endif /* !(QR_SCANNER_CY_SCBIP_V0 || QR_SCANNER_CY_SCBIP_V1) */
            }
        }
    #endif /* (!QR_SCANNER_CY_SCBIP_V1) */

    /* Configure pins: set HSIOM, DM and InputBufEnable */
    /* Note: the DR register settings do not effect the pin output if HSIOM is other than GPIO */

    #if (QR_SCANNER_RX_SCL_MOSI_PIN)
        QR_SCANNER_SET_HSIOM_SEL(QR_SCANNER_RX_SCL_MOSI_HSIOM_REG,
                                       QR_SCANNER_RX_SCL_MOSI_HSIOM_MASK,
                                       QR_SCANNER_RX_SCL_MOSI_HSIOM_POS,
                                        hsiomSel[QR_SCANNER_RX_SCL_MOSI_PIN_INDEX]);

        QR_SCANNER_uart_rx_i2c_scl_spi_mosi_SetDriveMode((uint8) pinsDm[QR_SCANNER_RX_SCL_MOSI_PIN_INDEX]);

        #if (!QR_SCANNER_CY_SCBIP_V1)
            QR_SCANNER_SET_INP_DIS(QR_SCANNER_uart_rx_i2c_scl_spi_mosi_INP_DIS,
                                         QR_SCANNER_uart_rx_i2c_scl_spi_mosi_MASK,
                                         (0u != (pinsInBuf & QR_SCANNER_RX_SCL_MOSI_PIN_MASK)));
        #endif /* (!QR_SCANNER_CY_SCBIP_V1) */
    
    #elif (QR_SCANNER_RX_WAKE_SCL_MOSI_PIN)
        QR_SCANNER_SET_HSIOM_SEL(QR_SCANNER_RX_WAKE_SCL_MOSI_HSIOM_REG,
                                       QR_SCANNER_RX_WAKE_SCL_MOSI_HSIOM_MASK,
                                       QR_SCANNER_RX_WAKE_SCL_MOSI_HSIOM_POS,
                                       hsiomSel[QR_SCANNER_RX_WAKE_SCL_MOSI_PIN_INDEX]);

        QR_SCANNER_uart_rx_wake_i2c_scl_spi_mosi_SetDriveMode((uint8)
                                                               pinsDm[QR_SCANNER_RX_WAKE_SCL_MOSI_PIN_INDEX]);

        QR_SCANNER_SET_INP_DIS(QR_SCANNER_uart_rx_wake_i2c_scl_spi_mosi_INP_DIS,
                                     QR_SCANNER_uart_rx_wake_i2c_scl_spi_mosi_MASK,
                                     (0u != (pinsInBuf & QR_SCANNER_RX_WAKE_SCL_MOSI_PIN_MASK)));

         /* Set interrupt on falling edge */
        QR_SCANNER_SET_INCFG_TYPE(QR_SCANNER_RX_WAKE_SCL_MOSI_INTCFG_REG,
                                        QR_SCANNER_RX_WAKE_SCL_MOSI_INTCFG_TYPE_MASK,
                                        QR_SCANNER_RX_WAKE_SCL_MOSI_INTCFG_TYPE_POS,
                                        QR_SCANNER_INTCFG_TYPE_FALLING_EDGE);
    #else
    #endif /* (QR_SCANNER_RX_WAKE_SCL_MOSI_PIN) */

    #if (QR_SCANNER_TX_SDA_MISO_PIN)
        QR_SCANNER_SET_HSIOM_SEL(QR_SCANNER_TX_SDA_MISO_HSIOM_REG,
                                       QR_SCANNER_TX_SDA_MISO_HSIOM_MASK,
                                       QR_SCANNER_TX_SDA_MISO_HSIOM_POS,
                                        hsiomSel[QR_SCANNER_TX_SDA_MISO_PIN_INDEX]);

        QR_SCANNER_uart_tx_i2c_sda_spi_miso_SetDriveMode((uint8) pinsDm[QR_SCANNER_TX_SDA_MISO_PIN_INDEX]);

    #if (!QR_SCANNER_CY_SCBIP_V1)
        QR_SCANNER_SET_INP_DIS(QR_SCANNER_uart_tx_i2c_sda_spi_miso_INP_DIS,
                                     QR_SCANNER_uart_tx_i2c_sda_spi_miso_MASK,
                                    (0u != (pinsInBuf & QR_SCANNER_TX_SDA_MISO_PIN_MASK)));
    #endif /* (!QR_SCANNER_CY_SCBIP_V1) */
    #endif /* (QR_SCANNER_RX_SCL_MOSI_PIN) */

    #if (QR_SCANNER_CTS_SCLK_PIN)
        QR_SCANNER_SET_HSIOM_SEL(QR_SCANNER_CTS_SCLK_HSIOM_REG,
                                       QR_SCANNER_CTS_SCLK_HSIOM_MASK,
                                       QR_SCANNER_CTS_SCLK_HSIOM_POS,
                                       hsiomSel[QR_SCANNER_CTS_SCLK_PIN_INDEX]);

        QR_SCANNER_uart_cts_spi_sclk_SetDriveMode((uint8) pinsDm[QR_SCANNER_CTS_SCLK_PIN_INDEX]);

        QR_SCANNER_SET_INP_DIS(QR_SCANNER_uart_cts_spi_sclk_INP_DIS,
                                     QR_SCANNER_uart_cts_spi_sclk_MASK,
                                     (0u != (pinsInBuf & QR_SCANNER_CTS_SCLK_PIN_MASK)));
    #endif /* (QR_SCANNER_CTS_SCLK_PIN) */

    #if (QR_SCANNER_RTS_SS0_PIN)
        QR_SCANNER_SET_HSIOM_SEL(QR_SCANNER_RTS_SS0_HSIOM_REG,
                                       QR_SCANNER_RTS_SS0_HSIOM_MASK,
                                       QR_SCANNER_RTS_SS0_HSIOM_POS,
                                       hsiomSel[QR_SCANNER_RTS_SS0_PIN_INDEX]);

        QR_SCANNER_uart_rts_spi_ss0_SetDriveMode((uint8) pinsDm[QR_SCANNER_RTS_SS0_PIN_INDEX]);

        QR_SCANNER_SET_INP_DIS(QR_SCANNER_uart_rts_spi_ss0_INP_DIS,
                                     QR_SCANNER_uart_rts_spi_ss0_MASK,
                                     (0u != (pinsInBuf & QR_SCANNER_RTS_SS0_PIN_MASK)));
    #endif /* (QR_SCANNER_RTS_SS0_PIN) */

    #if (QR_SCANNER_SS1_PIN)
        QR_SCANNER_SET_HSIOM_SEL(QR_SCANNER_SS1_HSIOM_REG,
                                       QR_SCANNER_SS1_HSIOM_MASK,
                                       QR_SCANNER_SS1_HSIOM_POS,
                                       hsiomSel[QR_SCANNER_SS1_PIN_INDEX]);

        QR_SCANNER_spi_ss1_SetDriveMode((uint8) pinsDm[QR_SCANNER_SS1_PIN_INDEX]);

        QR_SCANNER_SET_INP_DIS(QR_SCANNER_spi_ss1_INP_DIS,
                                     QR_SCANNER_spi_ss1_MASK,
                                     (0u != (pinsInBuf & QR_SCANNER_SS1_PIN_MASK)));
    #endif /* (QR_SCANNER_SS1_PIN) */

    #if (QR_SCANNER_SS2_PIN)
        QR_SCANNER_SET_HSIOM_SEL(QR_SCANNER_SS2_HSIOM_REG,
                                       QR_SCANNER_SS2_HSIOM_MASK,
                                       QR_SCANNER_SS2_HSIOM_POS,
                                       hsiomSel[QR_SCANNER_SS2_PIN_INDEX]);

        QR_SCANNER_spi_ss2_SetDriveMode((uint8) pinsDm[QR_SCANNER_SS2_PIN_INDEX]);

        QR_SCANNER_SET_INP_DIS(QR_SCANNER_spi_ss2_INP_DIS,
                                     QR_SCANNER_spi_ss2_MASK,
                                     (0u != (pinsInBuf & QR_SCANNER_SS2_PIN_MASK)));
    #endif /* (QR_SCANNER_SS2_PIN) */

    #if (QR_SCANNER_SS3_PIN)
        QR_SCANNER_SET_HSIOM_SEL(QR_SCANNER_SS3_HSIOM_REG,
                                       QR_SCANNER_SS3_HSIOM_MASK,
                                       QR_SCANNER_SS3_HSIOM_POS,
                                       hsiomSel[QR_SCANNER_SS3_PIN_INDEX]);

        QR_SCANNER_spi_ss3_SetDriveMode((uint8) pinsDm[QR_SCANNER_SS3_PIN_INDEX]);

        QR_SCANNER_SET_INP_DIS(QR_SCANNER_spi_ss3_INP_DIS,
                                     QR_SCANNER_spi_ss3_MASK,
                                     (0u != (pinsInBuf & QR_SCANNER_SS3_PIN_MASK)));
    #endif /* (QR_SCANNER_SS3_PIN) */
    }

#endif /* (QR_SCANNER_SCB_MODE_UNCONFIG_CONST_CFG) */


#if (QR_SCANNER_CY_SCBIP_V0 || QR_SCANNER_CY_SCBIP_V1)
    /*******************************************************************************
    * Function Name: QR_SCANNER_I2CSlaveNackGeneration
    ****************************************************************************//**
    *
    *  Sets command to generate NACK to the address or data.
    *
    *******************************************************************************/
    void QR_SCANNER_I2CSlaveNackGeneration(void)
    {
        /* Check for EC_AM toggle condition: EC_AM and clock stretching for address are enabled */
        if ((0u != (QR_SCANNER_CTRL_REG & QR_SCANNER_CTRL_EC_AM_MODE)) &&
            (0u == (QR_SCANNER_I2C_CTRL_REG & QR_SCANNER_I2C_CTRL_S_NOT_READY_ADDR_NACK)))
        {
            /* Toggle EC_AM before NACK generation */
            QR_SCANNER_CTRL_REG &= ~QR_SCANNER_CTRL_EC_AM_MODE;
            QR_SCANNER_CTRL_REG |=  QR_SCANNER_CTRL_EC_AM_MODE;
        }

        QR_SCANNER_I2C_SLAVE_CMD_REG = QR_SCANNER_I2C_SLAVE_CMD_S_NACK;
    }
#endif /* (QR_SCANNER_CY_SCBIP_V0 || QR_SCANNER_CY_SCBIP_V1) */


/* [] END OF FILE */
