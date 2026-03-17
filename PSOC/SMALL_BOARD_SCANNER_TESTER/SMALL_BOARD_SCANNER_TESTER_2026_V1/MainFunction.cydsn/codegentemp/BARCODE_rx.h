/*******************************************************************************
* File Name: BARCODE_rx.h  
* Version 2.20
*
* Description:
*  This file contains Pin function prototypes and register defines
*
********************************************************************************
* Copyright 2008-2015, Cypress Semiconductor Corporation.  All rights reserved.
* You may use this file only in accordance with the license, terms, conditions, 
* disclaimers, and limitations in the end user license agreement accompanying 
* the software package with which this file was provided.
*******************************************************************************/

#if !defined(CY_PINS_BARCODE_rx_H) /* Pins BARCODE_rx_H */
#define CY_PINS_BARCODE_rx_H

#include "cytypes.h"
#include "cyfitter.h"
#include "BARCODE_rx_aliases.h"


/***************************************
*     Data Struct Definitions
***************************************/

/**
* \addtogroup group_structures
* @{
*/
    
/* Structure for sleep mode support */
typedef struct
{
    uint32 pcState; /**< State of the port control register */
    uint32 sioState; /**< State of the SIO configuration */
    uint32 usbState; /**< State of the USBIO regulator */
} BARCODE_rx_BACKUP_STRUCT;

/** @} structures */


/***************************************
*        Function Prototypes             
***************************************/
/**
* \addtogroup group_general
* @{
*/
uint8   BARCODE_rx_Read(void);
void    BARCODE_rx_Write(uint8 value);
uint8   BARCODE_rx_ReadDataReg(void);
#if defined(BARCODE_rx__PC) || (CY_PSOC4_4200L) 
    void    BARCODE_rx_SetDriveMode(uint8 mode);
#endif
void    BARCODE_rx_SetInterruptMode(uint16 position, uint16 mode);
uint8   BARCODE_rx_ClearInterrupt(void);
/** @} general */

/**
* \addtogroup group_power
* @{
*/
void BARCODE_rx_Sleep(void); 
void BARCODE_rx_Wakeup(void);
/** @} power */


/***************************************
*           API Constants        
***************************************/
#if defined(BARCODE_rx__PC) || (CY_PSOC4_4200L) 
    /* Drive Modes */
    #define BARCODE_rx_DRIVE_MODE_BITS        (3)
    #define BARCODE_rx_DRIVE_MODE_IND_MASK    (0xFFFFFFFFu >> (32 - BARCODE_rx_DRIVE_MODE_BITS))

    /**
    * \addtogroup group_constants
    * @{
    */
        /** \addtogroup driveMode Drive mode constants
         * \brief Constants to be passed as "mode" parameter in the BARCODE_rx_SetDriveMode() function.
         *  @{
         */
        #define BARCODE_rx_DM_ALG_HIZ         (0x00u) /**< \brief High Impedance Analog   */
        #define BARCODE_rx_DM_DIG_HIZ         (0x01u) /**< \brief High Impedance Digital  */
        #define BARCODE_rx_DM_RES_UP          (0x02u) /**< \brief Resistive Pull Up       */
        #define BARCODE_rx_DM_RES_DWN         (0x03u) /**< \brief Resistive Pull Down     */
        #define BARCODE_rx_DM_OD_LO           (0x04u) /**< \brief Open Drain, Drives Low  */
        #define BARCODE_rx_DM_OD_HI           (0x05u) /**< \brief Open Drain, Drives High */
        #define BARCODE_rx_DM_STRONG          (0x06u) /**< \brief Strong Drive            */
        #define BARCODE_rx_DM_RES_UPDWN       (0x07u) /**< \brief Resistive Pull Up/Down  */
        /** @} driveMode */
    /** @} group_constants */
#endif

/* Digital Port Constants */
#define BARCODE_rx_MASK               BARCODE_rx__MASK
#define BARCODE_rx_SHIFT              BARCODE_rx__SHIFT
#define BARCODE_rx_WIDTH              1u

/**
* \addtogroup group_constants
* @{
*/
    /** \addtogroup intrMode Interrupt constants
     * \brief Constants to be passed as "mode" parameter in BARCODE_rx_SetInterruptMode() function.
     *  @{
     */
        #define BARCODE_rx_INTR_NONE      ((uint16)(0x0000u)) /**< \brief Disabled             */
        #define BARCODE_rx_INTR_RISING    ((uint16)(0x5555u)) /**< \brief Rising edge trigger  */
        #define BARCODE_rx_INTR_FALLING   ((uint16)(0xaaaau)) /**< \brief Falling edge trigger */
        #define BARCODE_rx_INTR_BOTH      ((uint16)(0xffffu)) /**< \brief Both edge trigger    */
    /** @} intrMode */
/** @} group_constants */

/* SIO LPM definition */
#if defined(BARCODE_rx__SIO)
    #define BARCODE_rx_SIO_LPM_MASK       (0x03u)
#endif

/* USBIO definitions */
#if !defined(BARCODE_rx__PC) && (CY_PSOC4_4200L)
    #define BARCODE_rx_USBIO_ENABLE               ((uint32)0x80000000u)
    #define BARCODE_rx_USBIO_DISABLE              ((uint32)(~BARCODE_rx_USBIO_ENABLE))
    #define BARCODE_rx_USBIO_SUSPEND_SHIFT        CYFLD_USBDEVv2_USB_SUSPEND__OFFSET
    #define BARCODE_rx_USBIO_SUSPEND_DEL_SHIFT    CYFLD_USBDEVv2_USB_SUSPEND_DEL__OFFSET
    #define BARCODE_rx_USBIO_ENTER_SLEEP          ((uint32)((1u << BARCODE_rx_USBIO_SUSPEND_SHIFT) \
                                                        | (1u << BARCODE_rx_USBIO_SUSPEND_DEL_SHIFT)))
    #define BARCODE_rx_USBIO_EXIT_SLEEP_PH1       ((uint32)~((uint32)(1u << BARCODE_rx_USBIO_SUSPEND_SHIFT)))
    #define BARCODE_rx_USBIO_EXIT_SLEEP_PH2       ((uint32)~((uint32)(1u << BARCODE_rx_USBIO_SUSPEND_DEL_SHIFT)))
    #define BARCODE_rx_USBIO_CR1_OFF              ((uint32)0xfffffffeu)
#endif


/***************************************
*             Registers        
***************************************/
/* Main Port Registers */
#if defined(BARCODE_rx__PC)
    /* Port Configuration */
    #define BARCODE_rx_PC                 (* (reg32 *) BARCODE_rx__PC)
#endif
/* Pin State */
#define BARCODE_rx_PS                     (* (reg32 *) BARCODE_rx__PS)
/* Data Register */
#define BARCODE_rx_DR                     (* (reg32 *) BARCODE_rx__DR)
/* Input Buffer Disable Override */
#define BARCODE_rx_INP_DIS                (* (reg32 *) BARCODE_rx__PC2)

/* Interrupt configuration Registers */
#define BARCODE_rx_INTCFG                 (* (reg32 *) BARCODE_rx__INTCFG)
#define BARCODE_rx_INTSTAT                (* (reg32 *) BARCODE_rx__INTSTAT)

/* "Interrupt cause" register for Combined Port Interrupt (AllPortInt) in GSRef component */
#if defined (CYREG_GPIO_INTR_CAUSE)
    #define BARCODE_rx_INTR_CAUSE         (* (reg32 *) CYREG_GPIO_INTR_CAUSE)
#endif

/* SIO register */
#if defined(BARCODE_rx__SIO)
    #define BARCODE_rx_SIO_REG            (* (reg32 *) BARCODE_rx__SIO)
#endif /* (BARCODE_rx__SIO_CFG) */

/* USBIO registers */
#if !defined(BARCODE_rx__PC) && (CY_PSOC4_4200L)
    #define BARCODE_rx_USB_POWER_REG       (* (reg32 *) CYREG_USBDEVv2_USB_POWER_CTRL)
    #define BARCODE_rx_CR1_REG             (* (reg32 *) CYREG_USBDEVv2_CR1)
    #define BARCODE_rx_USBIO_CTRL_REG      (* (reg32 *) CYREG_USBDEVv2_USB_USBIO_CTRL)
#endif    
    
    
/***************************************
* The following code is DEPRECATED and 
* must not be used in new designs.
***************************************/
/**
* \addtogroup group_deprecated
* @{
*/
#define BARCODE_rx_DRIVE_MODE_SHIFT       (0x00u)
#define BARCODE_rx_DRIVE_MODE_MASK        (0x07u << BARCODE_rx_DRIVE_MODE_SHIFT)
/** @} deprecated */

#endif /* End Pins BARCODE_rx_H */


/* [] END OF FILE */
