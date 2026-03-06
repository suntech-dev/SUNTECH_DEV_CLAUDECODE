/*******************************************************************************
* File Name: Barcode_Triger.h  
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

#if !defined(CY_PINS_Barcode_Triger_H) /* Pins Barcode_Triger_H */
#define CY_PINS_Barcode_Triger_H

#include "cytypes.h"
#include "cyfitter.h"
#include "Barcode_Triger_aliases.h"


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
} Barcode_Triger_BACKUP_STRUCT;

/** @} structures */


/***************************************
*        Function Prototypes             
***************************************/
/**
* \addtogroup group_general
* @{
*/
uint8   Barcode_Triger_Read(void);
void    Barcode_Triger_Write(uint8 value);
uint8   Barcode_Triger_ReadDataReg(void);
#if defined(Barcode_Triger__PC) || (CY_PSOC4_4200L) 
    void    Barcode_Triger_SetDriveMode(uint8 mode);
#endif
void    Barcode_Triger_SetInterruptMode(uint16 position, uint16 mode);
uint8   Barcode_Triger_ClearInterrupt(void);
/** @} general */

/**
* \addtogroup group_power
* @{
*/
void Barcode_Triger_Sleep(void); 
void Barcode_Triger_Wakeup(void);
/** @} power */


/***************************************
*           API Constants        
***************************************/
#if defined(Barcode_Triger__PC) || (CY_PSOC4_4200L) 
    /* Drive Modes */
    #define Barcode_Triger_DRIVE_MODE_BITS        (3)
    #define Barcode_Triger_DRIVE_MODE_IND_MASK    (0xFFFFFFFFu >> (32 - Barcode_Triger_DRIVE_MODE_BITS))

    /**
    * \addtogroup group_constants
    * @{
    */
        /** \addtogroup driveMode Drive mode constants
         * \brief Constants to be passed as "mode" parameter in the Barcode_Triger_SetDriveMode() function.
         *  @{
         */
        #define Barcode_Triger_DM_ALG_HIZ         (0x00u) /**< \brief High Impedance Analog   */
        #define Barcode_Triger_DM_DIG_HIZ         (0x01u) /**< \brief High Impedance Digital  */
        #define Barcode_Triger_DM_RES_UP          (0x02u) /**< \brief Resistive Pull Up       */
        #define Barcode_Triger_DM_RES_DWN         (0x03u) /**< \brief Resistive Pull Down     */
        #define Barcode_Triger_DM_OD_LO           (0x04u) /**< \brief Open Drain, Drives Low  */
        #define Barcode_Triger_DM_OD_HI           (0x05u) /**< \brief Open Drain, Drives High */
        #define Barcode_Triger_DM_STRONG          (0x06u) /**< \brief Strong Drive            */
        #define Barcode_Triger_DM_RES_UPDWN       (0x07u) /**< \brief Resistive Pull Up/Down  */
        /** @} driveMode */
    /** @} group_constants */
#endif

/* Digital Port Constants */
#define Barcode_Triger_MASK               Barcode_Triger__MASK
#define Barcode_Triger_SHIFT              Barcode_Triger__SHIFT
#define Barcode_Triger_WIDTH              1u

/**
* \addtogroup group_constants
* @{
*/
    /** \addtogroup intrMode Interrupt constants
     * \brief Constants to be passed as "mode" parameter in Barcode_Triger_SetInterruptMode() function.
     *  @{
     */
        #define Barcode_Triger_INTR_NONE      ((uint16)(0x0000u)) /**< \brief Disabled             */
        #define Barcode_Triger_INTR_RISING    ((uint16)(0x5555u)) /**< \brief Rising edge trigger  */
        #define Barcode_Triger_INTR_FALLING   ((uint16)(0xaaaau)) /**< \brief Falling edge trigger */
        #define Barcode_Triger_INTR_BOTH      ((uint16)(0xffffu)) /**< \brief Both edge trigger    */
    /** @} intrMode */
/** @} group_constants */

/* SIO LPM definition */
#if defined(Barcode_Triger__SIO)
    #define Barcode_Triger_SIO_LPM_MASK       (0x03u)
#endif

/* USBIO definitions */
#if !defined(Barcode_Triger__PC) && (CY_PSOC4_4200L)
    #define Barcode_Triger_USBIO_ENABLE               ((uint32)0x80000000u)
    #define Barcode_Triger_USBIO_DISABLE              ((uint32)(~Barcode_Triger_USBIO_ENABLE))
    #define Barcode_Triger_USBIO_SUSPEND_SHIFT        CYFLD_USBDEVv2_USB_SUSPEND__OFFSET
    #define Barcode_Triger_USBIO_SUSPEND_DEL_SHIFT    CYFLD_USBDEVv2_USB_SUSPEND_DEL__OFFSET
    #define Barcode_Triger_USBIO_ENTER_SLEEP          ((uint32)((1u << Barcode_Triger_USBIO_SUSPEND_SHIFT) \
                                                        | (1u << Barcode_Triger_USBIO_SUSPEND_DEL_SHIFT)))
    #define Barcode_Triger_USBIO_EXIT_SLEEP_PH1       ((uint32)~((uint32)(1u << Barcode_Triger_USBIO_SUSPEND_SHIFT)))
    #define Barcode_Triger_USBIO_EXIT_SLEEP_PH2       ((uint32)~((uint32)(1u << Barcode_Triger_USBIO_SUSPEND_DEL_SHIFT)))
    #define Barcode_Triger_USBIO_CR1_OFF              ((uint32)0xfffffffeu)
#endif


/***************************************
*             Registers        
***************************************/
/* Main Port Registers */
#if defined(Barcode_Triger__PC)
    /* Port Configuration */
    #define Barcode_Triger_PC                 (* (reg32 *) Barcode_Triger__PC)
#endif
/* Pin State */
#define Barcode_Triger_PS                     (* (reg32 *) Barcode_Triger__PS)
/* Data Register */
#define Barcode_Triger_DR                     (* (reg32 *) Barcode_Triger__DR)
/* Input Buffer Disable Override */
#define Barcode_Triger_INP_DIS                (* (reg32 *) Barcode_Triger__PC2)

/* Interrupt configuration Registers */
#define Barcode_Triger_INTCFG                 (* (reg32 *) Barcode_Triger__INTCFG)
#define Barcode_Triger_INTSTAT                (* (reg32 *) Barcode_Triger__INTSTAT)

/* "Interrupt cause" register for Combined Port Interrupt (AllPortInt) in GSRef component */
#if defined (CYREG_GPIO_INTR_CAUSE)
    #define Barcode_Triger_INTR_CAUSE         (* (reg32 *) CYREG_GPIO_INTR_CAUSE)
#endif

/* SIO register */
#if defined(Barcode_Triger__SIO)
    #define Barcode_Triger_SIO_REG            (* (reg32 *) Barcode_Triger__SIO)
#endif /* (Barcode_Triger__SIO_CFG) */

/* USBIO registers */
#if !defined(Barcode_Triger__PC) && (CY_PSOC4_4200L)
    #define Barcode_Triger_USB_POWER_REG       (* (reg32 *) CYREG_USBDEVv2_USB_POWER_CTRL)
    #define Barcode_Triger_CR1_REG             (* (reg32 *) CYREG_USBDEVv2_CR1)
    #define Barcode_Triger_USBIO_CTRL_REG      (* (reg32 *) CYREG_USBDEVv2_USB_USBIO_CTRL)
#endif    
    
    
/***************************************
* The following code is DEPRECATED and 
* must not be used in new designs.
***************************************/
/**
* \addtogroup group_deprecated
* @{
*/
#define Barcode_Triger_DRIVE_MODE_SHIFT       (0x00u)
#define Barcode_Triger_DRIVE_MODE_MASK        (0x07u << Barcode_Triger_DRIVE_MODE_SHIFT)
/** @} deprecated */

#endif /* End Pins Barcode_Triger_H */


/* [] END OF FILE */
