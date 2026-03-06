/*******************************************************************************
* File Name: SPIM_FLASH.h
* Version 2.50
*
* Description:
*  Contains the function prototypes, constants and register definition
*  of the SPI Master Component.
*
* Note:
*  None
*
********************************************************************************
* Copyright 2008-2015, Cypress Semiconductor Corporation.  All rights reserved.
* You may use this file only in accordance with the license, terms, conditions,
* disclaimers, and limitations in the end user license agreement accompanying
* the software package with which this file was provided.
*******************************************************************************/

#if !defined(CY_SPIM_SPIM_FLASH_H)
#define CY_SPIM_SPIM_FLASH_H

#include "cyfitter.h"
#include "cytypes.h"
#include "CyLib.h" /* For CyEnterCriticalSection() and CyExitCriticalSection() functions */


/***************************************
*   Conditional Compilation Parameters
***************************************/

#define SPIM_FLASH_INTERNAL_CLOCK             (1u)

#if(0u != SPIM_FLASH_INTERNAL_CLOCK)
    #include "SPIM_FLASH_IntClock.h"
#endif /* (0u != SPIM_FLASH_INTERNAL_CLOCK) */

#define SPIM_FLASH_MODE                       (1u)
#define SPIM_FLASH_DATA_WIDTH                 (8u)
#define SPIM_FLASH_MODE_USE_ZERO              (1u)
#define SPIM_FLASH_BIDIRECTIONAL_MODE         (0u)

/* Internal interrupt handling */
#define SPIM_FLASH_TX_BUFFER_SIZE             (4u)
#define SPIM_FLASH_RX_BUFFER_SIZE             (4u)
#define SPIM_FLASH_INTERNAL_TX_INT_ENABLED    (0u)
#define SPIM_FLASH_INTERNAL_RX_INT_ENABLED    (0u)

#define SPIM_FLASH_SINGLE_REG_SIZE            (8u)
#define SPIM_FLASH_USE_SECOND_DATAPATH        (SPIM_FLASH_DATA_WIDTH > SPIM_FLASH_SINGLE_REG_SIZE)

#define SPIM_FLASH_FIFO_SIZE                  (4u)
#define SPIM_FLASH_TX_SOFTWARE_BUF_ENABLED    ((0u != SPIM_FLASH_INTERNAL_TX_INT_ENABLED) && \
                                                     (SPIM_FLASH_TX_BUFFER_SIZE > SPIM_FLASH_FIFO_SIZE))

#define SPIM_FLASH_RX_SOFTWARE_BUF_ENABLED    ((0u != SPIM_FLASH_INTERNAL_RX_INT_ENABLED) && \
                                                     (SPIM_FLASH_RX_BUFFER_SIZE > SPIM_FLASH_FIFO_SIZE))


/***************************************
*        Data Struct Definition
***************************************/

/* Sleep Mode API Support */
typedef struct
{
    uint8 enableState;
    uint8 cntrPeriod;
} SPIM_FLASH_BACKUP_STRUCT;


/***************************************
*        Function Prototypes
***************************************/

void  SPIM_FLASH_Init(void)                           ;
void  SPIM_FLASH_Enable(void)                         ;
void  SPIM_FLASH_Start(void)                          ;
void  SPIM_FLASH_Stop(void)                           ;

void  SPIM_FLASH_EnableTxInt(void)                    ;
void  SPIM_FLASH_EnableRxInt(void)                    ;
void  SPIM_FLASH_DisableTxInt(void)                   ;
void  SPIM_FLASH_DisableRxInt(void)                   ;

void  SPIM_FLASH_Sleep(void)                          ;
void  SPIM_FLASH_Wakeup(void)                         ;
void  SPIM_FLASH_SaveConfig(void)                     ;
void  SPIM_FLASH_RestoreConfig(void)                  ;

void  SPIM_FLASH_SetTxInterruptMode(uint8 intSrc)     ;
void  SPIM_FLASH_SetRxInterruptMode(uint8 intSrc)     ;
uint8 SPIM_FLASH_ReadTxStatus(void)                   ;
uint8 SPIM_FLASH_ReadRxStatus(void)                   ;
void  SPIM_FLASH_WriteTxData(uint8 txData)  \
                                                            ;
uint8 SPIM_FLASH_ReadRxData(void) \
                                                            ;
uint8 SPIM_FLASH_GetRxBufferSize(void)                ;
uint8 SPIM_FLASH_GetTxBufferSize(void)                ;
void  SPIM_FLASH_ClearRxBuffer(void)                  ;
void  SPIM_FLASH_ClearTxBuffer(void)                  ;
void  SPIM_FLASH_ClearFIFO(void)                              ;
void  SPIM_FLASH_PutArray(const uint8 buffer[], uint8 byteCount) \
                                                            ;

#if(0u != SPIM_FLASH_BIDIRECTIONAL_MODE)
    void  SPIM_FLASH_TxEnable(void)                   ;
    void  SPIM_FLASH_TxDisable(void)                  ;
#endif /* (0u != SPIM_FLASH_BIDIRECTIONAL_MODE) */

CY_ISR_PROTO(SPIM_FLASH_TX_ISR);
CY_ISR_PROTO(SPIM_FLASH_RX_ISR);


/***************************************
*   Variable with external linkage
***************************************/

extern uint8 SPIM_FLASH_initVar;


/***************************************
*           API Constants
***************************************/

#define SPIM_FLASH_TX_ISR_NUMBER     ((uint8) (SPIM_FLASH_TxInternalInterrupt__INTC_NUMBER))
#define SPIM_FLASH_RX_ISR_NUMBER     ((uint8) (SPIM_FLASH_RxInternalInterrupt__INTC_NUMBER))

#define SPIM_FLASH_TX_ISR_PRIORITY   ((uint8) (SPIM_FLASH_TxInternalInterrupt__INTC_PRIOR_NUM))
#define SPIM_FLASH_RX_ISR_PRIORITY   ((uint8) (SPIM_FLASH_RxInternalInterrupt__INTC_PRIOR_NUM))


/***************************************
*    Initial Parameter Constants
***************************************/

#define SPIM_FLASH_INT_ON_SPI_DONE    ((uint8) (0u   << SPIM_FLASH_STS_SPI_DONE_SHIFT))
#define SPIM_FLASH_INT_ON_TX_EMPTY    ((uint8) (0u   << SPIM_FLASH_STS_TX_FIFO_EMPTY_SHIFT))
#define SPIM_FLASH_INT_ON_TX_NOT_FULL ((uint8) (0u << \
                                                                           SPIM_FLASH_STS_TX_FIFO_NOT_FULL_SHIFT))
#define SPIM_FLASH_INT_ON_BYTE_COMP   ((uint8) (0u  << SPIM_FLASH_STS_BYTE_COMPLETE_SHIFT))
#define SPIM_FLASH_INT_ON_SPI_IDLE    ((uint8) (0u   << SPIM_FLASH_STS_SPI_IDLE_SHIFT))

/* Disable TX_NOT_FULL if software buffer is used */
#define SPIM_FLASH_INT_ON_TX_NOT_FULL_DEF ((SPIM_FLASH_TX_SOFTWARE_BUF_ENABLED) ? \
                                                                        (0u) : (SPIM_FLASH_INT_ON_TX_NOT_FULL))

/* TX interrupt mask */
#define SPIM_FLASH_TX_INIT_INTERRUPTS_MASK    (SPIM_FLASH_INT_ON_SPI_DONE  | \
                                                     SPIM_FLASH_INT_ON_TX_EMPTY  | \
                                                     SPIM_FLASH_INT_ON_TX_NOT_FULL_DEF | \
                                                     SPIM_FLASH_INT_ON_BYTE_COMP | \
                                                     SPIM_FLASH_INT_ON_SPI_IDLE)

#define SPIM_FLASH_INT_ON_RX_FULL         ((uint8) (0u << \
                                                                          SPIM_FLASH_STS_RX_FIFO_FULL_SHIFT))
#define SPIM_FLASH_INT_ON_RX_NOT_EMPTY    ((uint8) (0u << \
                                                                          SPIM_FLASH_STS_RX_FIFO_NOT_EMPTY_SHIFT))
#define SPIM_FLASH_INT_ON_RX_OVER         ((uint8) (0u << \
                                                                          SPIM_FLASH_STS_RX_FIFO_OVERRUN_SHIFT))

/* RX interrupt mask */
#define SPIM_FLASH_RX_INIT_INTERRUPTS_MASK    (SPIM_FLASH_INT_ON_RX_FULL      | \
                                                     SPIM_FLASH_INT_ON_RX_NOT_EMPTY | \
                                                     SPIM_FLASH_INT_ON_RX_OVER)
/* Nubmer of bits to receive/transmit */
#define SPIM_FLASH_BITCTR_INIT            (((uint8) (SPIM_FLASH_DATA_WIDTH << 1u)) - 1u)


/***************************************
*             Registers
***************************************/
#if(CY_PSOC3 || CY_PSOC5)
    #define SPIM_FLASH_TXDATA_REG (* (reg8 *) \
                                                SPIM_FLASH_BSPIM_sR8_Dp_u0__F0_REG)
    #define SPIM_FLASH_TXDATA_PTR (  (reg8 *) \
                                                SPIM_FLASH_BSPIM_sR8_Dp_u0__F0_REG)
    #define SPIM_FLASH_RXDATA_REG (* (reg8 *) \
                                                SPIM_FLASH_BSPIM_sR8_Dp_u0__F1_REG)
    #define SPIM_FLASH_RXDATA_PTR (  (reg8 *) \
                                                SPIM_FLASH_BSPIM_sR8_Dp_u0__F1_REG)
#else   /* PSOC4 */
    #if(SPIM_FLASH_USE_SECOND_DATAPATH)
        #define SPIM_FLASH_TXDATA_REG (* (reg16 *) \
                                          SPIM_FLASH_BSPIM_sR8_Dp_u0__16BIT_F0_REG)
        #define SPIM_FLASH_TXDATA_PTR (  (reg16 *) \
                                          SPIM_FLASH_BSPIM_sR8_Dp_u0__16BIT_F0_REG)
        #define SPIM_FLASH_RXDATA_REG (* (reg16 *) \
                                          SPIM_FLASH_BSPIM_sR8_Dp_u0__16BIT_F1_REG)
        #define SPIM_FLASH_RXDATA_PTR (  (reg16 *) \
                                          SPIM_FLASH_BSPIM_sR8_Dp_u0__16BIT_F1_REG)
    #else
        #define SPIM_FLASH_TXDATA_REG (* (reg8 *) \
                                                SPIM_FLASH_BSPIM_sR8_Dp_u0__F0_REG)
        #define SPIM_FLASH_TXDATA_PTR (  (reg8 *) \
                                                SPIM_FLASH_BSPIM_sR8_Dp_u0__F0_REG)
        #define SPIM_FLASH_RXDATA_REG (* (reg8 *) \
                                                SPIM_FLASH_BSPIM_sR8_Dp_u0__F1_REG)
        #define SPIM_FLASH_RXDATA_PTR (  (reg8 *) \
                                                SPIM_FLASH_BSPIM_sR8_Dp_u0__F1_REG)
    #endif /* (SPIM_FLASH_USE_SECOND_DATAPATH) */
#endif     /* (CY_PSOC3 || CY_PSOC5) */

#define SPIM_FLASH_AUX_CONTROL_DP0_REG (* (reg8 *) \
                                        SPIM_FLASH_BSPIM_sR8_Dp_u0__DP_AUX_CTL_REG)
#define SPIM_FLASH_AUX_CONTROL_DP0_PTR (  (reg8 *) \
                                        SPIM_FLASH_BSPIM_sR8_Dp_u0__DP_AUX_CTL_REG)

#if(SPIM_FLASH_USE_SECOND_DATAPATH)
    #define SPIM_FLASH_AUX_CONTROL_DP1_REG  (* (reg8 *) \
                                        SPIM_FLASH_BSPIM_sR8_Dp_u1__DP_AUX_CTL_REG)
    #define SPIM_FLASH_AUX_CONTROL_DP1_PTR  (  (reg8 *) \
                                        SPIM_FLASH_BSPIM_sR8_Dp_u1__DP_AUX_CTL_REG)
#endif /* (SPIM_FLASH_USE_SECOND_DATAPATH) */

#define SPIM_FLASH_COUNTER_PERIOD_REG     (* (reg8 *) SPIM_FLASH_BSPIM_BitCounter__PERIOD_REG)
#define SPIM_FLASH_COUNTER_PERIOD_PTR     (  (reg8 *) SPIM_FLASH_BSPIM_BitCounter__PERIOD_REG)
#define SPIM_FLASH_COUNTER_CONTROL_REG    (* (reg8 *) SPIM_FLASH_BSPIM_BitCounter__CONTROL_AUX_CTL_REG)
#define SPIM_FLASH_COUNTER_CONTROL_PTR    (  (reg8 *) SPIM_FLASH_BSPIM_BitCounter__CONTROL_AUX_CTL_REG)

#define SPIM_FLASH_TX_STATUS_REG          (* (reg8 *) SPIM_FLASH_BSPIM_TxStsReg__STATUS_REG)
#define SPIM_FLASH_TX_STATUS_PTR          (  (reg8 *) SPIM_FLASH_BSPIM_TxStsReg__STATUS_REG)
#define SPIM_FLASH_RX_STATUS_REG          (* (reg8 *) SPIM_FLASH_BSPIM_RxStsReg__STATUS_REG)
#define SPIM_FLASH_RX_STATUS_PTR          (  (reg8 *) SPIM_FLASH_BSPIM_RxStsReg__STATUS_REG)

#define SPIM_FLASH_CONTROL_REG            (* (reg8 *) \
                                      SPIM_FLASH_BSPIM_BidirMode_CtrlReg__CONTROL_REG)
#define SPIM_FLASH_CONTROL_PTR            (  (reg8 *) \
                                      SPIM_FLASH_BSPIM_BidirMode_CtrlReg__CONTROL_REG)

#define SPIM_FLASH_TX_STATUS_MASK_REG     (* (reg8 *) SPIM_FLASH_BSPIM_TxStsReg__MASK_REG)
#define SPIM_FLASH_TX_STATUS_MASK_PTR     (  (reg8 *) SPIM_FLASH_BSPIM_TxStsReg__MASK_REG)
#define SPIM_FLASH_RX_STATUS_MASK_REG     (* (reg8 *) SPIM_FLASH_BSPIM_RxStsReg__MASK_REG)
#define SPIM_FLASH_RX_STATUS_MASK_PTR     (  (reg8 *) SPIM_FLASH_BSPIM_RxStsReg__MASK_REG)

#define SPIM_FLASH_TX_STATUS_ACTL_REG     (* (reg8 *) SPIM_FLASH_BSPIM_TxStsReg__STATUS_AUX_CTL_REG)
#define SPIM_FLASH_TX_STATUS_ACTL_PTR     (  (reg8 *) SPIM_FLASH_BSPIM_TxStsReg__STATUS_AUX_CTL_REG)
#define SPIM_FLASH_RX_STATUS_ACTL_REG     (* (reg8 *) SPIM_FLASH_BSPIM_RxStsReg__STATUS_AUX_CTL_REG)
#define SPIM_FLASH_RX_STATUS_ACTL_PTR     (  (reg8 *) SPIM_FLASH_BSPIM_RxStsReg__STATUS_AUX_CTL_REG)

#if(SPIM_FLASH_USE_SECOND_DATAPATH)
    #define SPIM_FLASH_AUX_CONTROLDP1     (SPIM_FLASH_AUX_CONTROL_DP1_REG)
#endif /* (SPIM_FLASH_USE_SECOND_DATAPATH) */


/***************************************
*       Register Constants
***************************************/

/* Status Register Definitions */
#define SPIM_FLASH_STS_SPI_DONE_SHIFT             (0x00u)
#define SPIM_FLASH_STS_TX_FIFO_EMPTY_SHIFT        (0x01u)
#define SPIM_FLASH_STS_TX_FIFO_NOT_FULL_SHIFT     (0x02u)
#define SPIM_FLASH_STS_BYTE_COMPLETE_SHIFT        (0x03u)
#define SPIM_FLASH_STS_SPI_IDLE_SHIFT             (0x04u)
#define SPIM_FLASH_STS_RX_FIFO_FULL_SHIFT         (0x04u)
#define SPIM_FLASH_STS_RX_FIFO_NOT_EMPTY_SHIFT    (0x05u)
#define SPIM_FLASH_STS_RX_FIFO_OVERRUN_SHIFT      (0x06u)

#define SPIM_FLASH_STS_SPI_DONE           ((uint8) (0x01u << SPIM_FLASH_STS_SPI_DONE_SHIFT))
#define SPIM_FLASH_STS_TX_FIFO_EMPTY      ((uint8) (0x01u << SPIM_FLASH_STS_TX_FIFO_EMPTY_SHIFT))
#define SPIM_FLASH_STS_TX_FIFO_NOT_FULL   ((uint8) (0x01u << SPIM_FLASH_STS_TX_FIFO_NOT_FULL_SHIFT))
#define SPIM_FLASH_STS_BYTE_COMPLETE      ((uint8) (0x01u << SPIM_FLASH_STS_BYTE_COMPLETE_SHIFT))
#define SPIM_FLASH_STS_SPI_IDLE           ((uint8) (0x01u << SPIM_FLASH_STS_SPI_IDLE_SHIFT))
#define SPIM_FLASH_STS_RX_FIFO_FULL       ((uint8) (0x01u << SPIM_FLASH_STS_RX_FIFO_FULL_SHIFT))
#define SPIM_FLASH_STS_RX_FIFO_NOT_EMPTY  ((uint8) (0x01u << SPIM_FLASH_STS_RX_FIFO_NOT_EMPTY_SHIFT))
#define SPIM_FLASH_STS_RX_FIFO_OVERRUN    ((uint8) (0x01u << SPIM_FLASH_STS_RX_FIFO_OVERRUN_SHIFT))

/* TX and RX masks for clear on read bits */
#define SPIM_FLASH_TX_STS_CLR_ON_RD_BYTES_MASK    (0x09u)
#define SPIM_FLASH_RX_STS_CLR_ON_RD_BYTES_MASK    (0x40u)

/* StatusI Register Interrupt Enable Control Bits */
/* As defined by the Register map for the AUX Control Register */
#define SPIM_FLASH_INT_ENABLE     (0x10u) /* Enable interrupt from statusi */
#define SPIM_FLASH_TX_FIFO_CLR    (0x01u) /* F0 - TX FIFO */
#define SPIM_FLASH_RX_FIFO_CLR    (0x02u) /* F1 - RX FIFO */
#define SPIM_FLASH_FIFO_CLR       (SPIM_FLASH_TX_FIFO_CLR | SPIM_FLASH_RX_FIFO_CLR)

/* Bit Counter (7-bit) Control Register Bit Definitions */
/* As defined by the Register map for the AUX Control Register */
#define SPIM_FLASH_CNTR_ENABLE    (0x20u) /* Enable CNT7 */

/* Bi-Directional mode control bit */
#define SPIM_FLASH_CTRL_TX_SIGNAL_EN  (0x01u)

/* Datapath Auxillary Control Register definitions */
#define SPIM_FLASH_AUX_CTRL_FIFO0_CLR         (0x01u)
#define SPIM_FLASH_AUX_CTRL_FIFO1_CLR         (0x02u)
#define SPIM_FLASH_AUX_CTRL_FIFO0_LVL         (0x04u)
#define SPIM_FLASH_AUX_CTRL_FIFO1_LVL         (0x08u)
#define SPIM_FLASH_STATUS_ACTL_INT_EN_MASK    (0x10u)

/* Component disabled */
#define SPIM_FLASH_DISABLED   (0u)


/***************************************
*       Macros
***************************************/

/* Returns true if componentn enabled */
#define SPIM_FLASH_IS_ENABLED (0u != (SPIM_FLASH_TX_STATUS_ACTL_REG & SPIM_FLASH_INT_ENABLE))

/* Retuns TX status register */
#define SPIM_FLASH_GET_STATUS_TX(swTxSts) ( (uint8)(SPIM_FLASH_TX_STATUS_REG | \
                                                          ((swTxSts) & SPIM_FLASH_TX_STS_CLR_ON_RD_BYTES_MASK)) )
/* Retuns RX status register */
#define SPIM_FLASH_GET_STATUS_RX(swRxSts) ( (uint8)(SPIM_FLASH_RX_STATUS_REG | \
                                                          ((swRxSts) & SPIM_FLASH_RX_STS_CLR_ON_RD_BYTES_MASK)) )


/***************************************
* The following code is DEPRECATED and 
* should not be used in new projects.
***************************************/

#define SPIM_FLASH_WriteByte   SPIM_FLASH_WriteTxData
#define SPIM_FLASH_ReadByte    SPIM_FLASH_ReadRxData
void  SPIM_FLASH_SetInterruptMode(uint8 intSrc)       ;
uint8 SPIM_FLASH_ReadStatus(void)                     ;
void  SPIM_FLASH_EnableInt(void)                      ;
void  SPIM_FLASH_DisableInt(void)                     ;

#define SPIM_FLASH_TXDATA                 (SPIM_FLASH_TXDATA_REG)
#define SPIM_FLASH_RXDATA                 (SPIM_FLASH_RXDATA_REG)
#define SPIM_FLASH_AUX_CONTROLDP0         (SPIM_FLASH_AUX_CONTROL_DP0_REG)
#define SPIM_FLASH_TXBUFFERREAD           (SPIM_FLASH_txBufferRead)
#define SPIM_FLASH_TXBUFFERWRITE          (SPIM_FLASH_txBufferWrite)
#define SPIM_FLASH_RXBUFFERREAD           (SPIM_FLASH_rxBufferRead)
#define SPIM_FLASH_RXBUFFERWRITE          (SPIM_FLASH_rxBufferWrite)

#define SPIM_FLASH_COUNTER_PERIOD         (SPIM_FLASH_COUNTER_PERIOD_REG)
#define SPIM_FLASH_COUNTER_CONTROL        (SPIM_FLASH_COUNTER_CONTROL_REG)
#define SPIM_FLASH_STATUS                 (SPIM_FLASH_TX_STATUS_REG)
#define SPIM_FLASH_CONTROL                (SPIM_FLASH_CONTROL_REG)
#define SPIM_FLASH_STATUS_MASK            (SPIM_FLASH_TX_STATUS_MASK_REG)
#define SPIM_FLASH_STATUS_ACTL            (SPIM_FLASH_TX_STATUS_ACTL_REG)

#define SPIM_FLASH_INIT_INTERRUPTS_MASK  (SPIM_FLASH_INT_ON_SPI_DONE     | \
                                                SPIM_FLASH_INT_ON_TX_EMPTY     | \
                                                SPIM_FLASH_INT_ON_TX_NOT_FULL_DEF  | \
                                                SPIM_FLASH_INT_ON_RX_FULL      | \
                                                SPIM_FLASH_INT_ON_RX_NOT_EMPTY | \
                                                SPIM_FLASH_INT_ON_RX_OVER      | \
                                                SPIM_FLASH_INT_ON_BYTE_COMP)
                                                
#define SPIM_FLASH_DataWidth                  (SPIM_FLASH_DATA_WIDTH)
#define SPIM_FLASH_InternalClockUsed          (SPIM_FLASH_INTERNAL_CLOCK)
#define SPIM_FLASH_InternalTxInterruptEnabled (SPIM_FLASH_INTERNAL_TX_INT_ENABLED)
#define SPIM_FLASH_InternalRxInterruptEnabled (SPIM_FLASH_INTERNAL_RX_INT_ENABLED)
#define SPIM_FLASH_ModeUseZero                (SPIM_FLASH_MODE_USE_ZERO)
#define SPIM_FLASH_BidirectionalMode          (SPIM_FLASH_BIDIRECTIONAL_MODE)
#define SPIM_FLASH_Mode                       (SPIM_FLASH_MODE)
#define SPIM_FLASH_DATAWIDHT                  (SPIM_FLASH_DATA_WIDTH)
#define SPIM_FLASH_InternalInterruptEnabled   (0u)

#define SPIM_FLASH_TXBUFFERSIZE   (SPIM_FLASH_TX_BUFFER_SIZE)
#define SPIM_FLASH_RXBUFFERSIZE   (SPIM_FLASH_RX_BUFFER_SIZE)

#define SPIM_FLASH_TXBUFFER       SPIM_FLASH_txBuffer
#define SPIM_FLASH_RXBUFFER       SPIM_FLASH_rxBuffer

#endif /* (CY_SPIM_SPIM_FLASH_H) */


/* [] END OF FILE */
