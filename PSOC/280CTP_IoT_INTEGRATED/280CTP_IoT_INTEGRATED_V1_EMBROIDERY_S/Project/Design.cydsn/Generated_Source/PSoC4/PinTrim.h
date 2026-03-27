/*******************************************************************************
* File Name: PinTrim.h  
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

#if !defined(CY_PINS_PinTrim_H) /* Pins PinTrim_H */
#define CY_PINS_PinTrim_H

#include "cytypes.h"
#include "cyfitter.h"
#include "PinTrim_aliases.h"


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
} PinTrim_BACKUP_STRUCT;

/** @} structures */


/***************************************
*        Function Prototypes             
***************************************/
/**
* \addtogroup group_general
* @{
*/
uint8   PinTrim_Read(void);
void    PinTrim_Write(uint8 value);
uint8   PinTrim_ReadDataReg(void);
#if defined(PinTrim__PC) || (CY_PSOC4_4200L) 
    void    PinTrim_SetDriveMode(uint8 mode);
#endif
void    PinTrim_SetInterruptMode(uint16 position, uint16 mode);
uint8   PinTrim_ClearInterrupt(void);
/** @} general */

/**
* \addtogroup group_power
* @{
*/
void PinTrim_Sleep(void); 
void PinTrim_Wakeup(void);
/** @} power */


/***************************************
*           API Constants        
***************************************/
#if defined(PinTrim__PC) || (CY_PSOC4_4200L) 
    /* Drive Modes */
    #define PinTrim_DRIVE_MODE_BITS        (3)
    #define PinTrim_DRIVE_MODE_IND_MASK    (0xFFFFFFFFu >> (32 - PinTrim_DRIVE_MODE_BITS))

    /**
    * \addtogroup group_constants
    * @{
    */
        /** \addtogroup driveMode Drive mode constants
         * \brief Constants to be passed as "mode" parameter in the PinTrim_SetDriveMode() function.
         *  @{
         */
        #define PinTrim_DM_ALG_HIZ         (0x00u) /**< \brief High Impedance Analog   */
        #define PinTrim_DM_DIG_HIZ         (0x01u) /**< \brief High Impedance Digital  */
        #define PinTrim_DM_RES_UP          (0x02u) /**< \brief Resistive Pull Up       */
        #define PinTrim_DM_RES_DWN         (0x03u) /**< \brief Resistive Pull Down     */
        #define PinTrim_DM_OD_LO           (0x04u) /**< \brief Open Drain, Drives Low  */
        #define PinTrim_DM_OD_HI           (0x05u) /**< \brief Open Drain, Drives High */
        #define PinTrim_DM_STRONG          (0x06u) /**< \brief Strong Drive            */
        #define PinTrim_DM_RES_UPDWN       (0x07u) /**< \brief Resistive Pull Up/Down  */
        /** @} driveMode */
    /** @} group_constants */
#endif

/* Digital Port Constants */
#define PinTrim_MASK               PinTrim__MASK
#define PinTrim_SHIFT              PinTrim__SHIFT
#define PinTrim_WIDTH              1u

/**
* \addtogroup group_constants
* @{
*/
    /** \addtogroup intrMode Interrupt constants
     * \brief Constants to be passed as "mode" parameter in PinTrim_SetInterruptMode() function.
     *  @{
     */
        #define PinTrim_INTR_NONE      ((uint16)(0x0000u)) /**< \brief Disabled             */
        #define PinTrim_INTR_RISING    ((uint16)(0x5555u)) /**< \brief Rising edge trigger  */
        #define PinTrim_INTR_FALLING   ((uint16)(0xaaaau)) /**< \brief Falling edge trigger */
        #define PinTrim_INTR_BOTH      ((uint16)(0xffffu)) /**< \brief Both edge trigger    */
    /** @} intrMode */
/** @} group_constants */

/* SIO LPM definition */
#if defined(PinTrim__SIO)
    #define PinTrim_SIO_LPM_MASK       (0x03u)
#endif

/* USBIO definitions */
#if !defined(PinTrim__PC) && (CY_PSOC4_4200L)
    #define PinTrim_USBIO_ENABLE               ((uint32)0x80000000u)
    #define PinTrim_USBIO_DISABLE              ((uint32)(~PinTrim_USBIO_ENABLE))
    #define PinTrim_USBIO_SUSPEND_SHIFT        CYFLD_USBDEVv2_USB_SUSPEND__OFFSET
    #define PinTrim_USBIO_SUSPEND_DEL_SHIFT    CYFLD_USBDEVv2_USB_SUSPEND_DEL__OFFSET
    #define PinTrim_USBIO_ENTER_SLEEP          ((uint32)((1u << PinTrim_USBIO_SUSPEND_SHIFT) \
                                                        | (1u << PinTrim_USBIO_SUSPEND_DEL_SHIFT)))
    #define PinTrim_USBIO_EXIT_SLEEP_PH1       ((uint32)~((uint32)(1u << PinTrim_USBIO_SUSPEND_SHIFT)))
    #define PinTrim_USBIO_EXIT_SLEEP_PH2       ((uint32)~((uint32)(1u << PinTrim_USBIO_SUSPEND_DEL_SHIFT)))
    #define PinTrim_USBIO_CR1_OFF              ((uint32)0xfffffffeu)
#endif


/***************************************
*             Registers        
***************************************/
/* Main Port Registers */
#if defined(PinTrim__PC)
    /* Port Configuration */
    #define PinTrim_PC                 (* (reg32 *) PinTrim__PC)
#endif
/* Pin State */
#define PinTrim_PS                     (* (reg32 *) PinTrim__PS)
/* Data Register */
#define PinTrim_DR                     (* (reg32 *) PinTrim__DR)
/* Input Buffer Disable Override */
#define PinTrim_INP_DIS                (* (reg32 *) PinTrim__PC2)

/* Interrupt configuration Registers */
#define PinTrim_INTCFG                 (* (reg32 *) PinTrim__INTCFG)
#define PinTrim_INTSTAT                (* (reg32 *) PinTrim__INTSTAT)

/* "Interrupt cause" register for Combined Port Interrupt (AllPortInt) in GSRef component */
#if defined (CYREG_GPIO_INTR_CAUSE)
    #define PinTrim_INTR_CAUSE         (* (reg32 *) CYREG_GPIO_INTR_CAUSE)
#endif

/* SIO register */
#if defined(PinTrim__SIO)
    #define PinTrim_SIO_REG            (* (reg32 *) PinTrim__SIO)
#endif /* (PinTrim__SIO_CFG) */

/* USBIO registers */
#if !defined(PinTrim__PC) && (CY_PSOC4_4200L)
    #define PinTrim_USB_POWER_REG       (* (reg32 *) CYREG_USBDEVv2_USB_POWER_CTRL)
    #define PinTrim_CR1_REG             (* (reg32 *) CYREG_USBDEVv2_CR1)
    #define PinTrim_USBIO_CTRL_REG      (* (reg32 *) CYREG_USBDEVv2_USB_USBIO_CTRL)
#endif    
    
    
/***************************************
* The following code is DEPRECATED and 
* must not be used in new designs.
***************************************/
/**
* \addtogroup group_deprecated
* @{
*/
#define PinTrim_DRIVE_MODE_SHIFT       (0x00u)
#define PinTrim_DRIVE_MODE_MASK        (0x07u << PinTrim_DRIVE_MODE_SHIFT)
/** @} deprecated */

#endif /* End Pins PinTrim_H */


/* [] END OF FILE */
