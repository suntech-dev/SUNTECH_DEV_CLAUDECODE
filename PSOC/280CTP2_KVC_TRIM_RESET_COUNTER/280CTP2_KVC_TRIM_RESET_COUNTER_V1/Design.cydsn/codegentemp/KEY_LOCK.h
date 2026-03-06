/*******************************************************************************
* File Name: KEY_LOCK.h  
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

#if !defined(CY_PINS_KEY_LOCK_H) /* Pins KEY_LOCK_H */
#define CY_PINS_KEY_LOCK_H

#include "cytypes.h"
#include "cyfitter.h"
#include "KEY_LOCK_aliases.h"


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
} KEY_LOCK_BACKUP_STRUCT;

/** @} structures */


/***************************************
*        Function Prototypes             
***************************************/
/**
* \addtogroup group_general
* @{
*/
uint8   KEY_LOCK_Read(void);
void    KEY_LOCK_Write(uint8 value);
uint8   KEY_LOCK_ReadDataReg(void);
#if defined(KEY_LOCK__PC) || (CY_PSOC4_4200L) 
    void    KEY_LOCK_SetDriveMode(uint8 mode);
#endif
void    KEY_LOCK_SetInterruptMode(uint16 position, uint16 mode);
uint8   KEY_LOCK_ClearInterrupt(void);
/** @} general */

/**
* \addtogroup group_power
* @{
*/
void KEY_LOCK_Sleep(void); 
void KEY_LOCK_Wakeup(void);
/** @} power */


/***************************************
*           API Constants        
***************************************/
#if defined(KEY_LOCK__PC) || (CY_PSOC4_4200L) 
    /* Drive Modes */
    #define KEY_LOCK_DRIVE_MODE_BITS        (3)
    #define KEY_LOCK_DRIVE_MODE_IND_MASK    (0xFFFFFFFFu >> (32 - KEY_LOCK_DRIVE_MODE_BITS))

    /**
    * \addtogroup group_constants
    * @{
    */
        /** \addtogroup driveMode Drive mode constants
         * \brief Constants to be passed as "mode" parameter in the KEY_LOCK_SetDriveMode() function.
         *  @{
         */
        #define KEY_LOCK_DM_ALG_HIZ         (0x00u) /**< \brief High Impedance Analog   */
        #define KEY_LOCK_DM_DIG_HIZ         (0x01u) /**< \brief High Impedance Digital  */
        #define KEY_LOCK_DM_RES_UP          (0x02u) /**< \brief Resistive Pull Up       */
        #define KEY_LOCK_DM_RES_DWN         (0x03u) /**< \brief Resistive Pull Down     */
        #define KEY_LOCK_DM_OD_LO           (0x04u) /**< \brief Open Drain, Drives Low  */
        #define KEY_LOCK_DM_OD_HI           (0x05u) /**< \brief Open Drain, Drives High */
        #define KEY_LOCK_DM_STRONG          (0x06u) /**< \brief Strong Drive            */
        #define KEY_LOCK_DM_RES_UPDWN       (0x07u) /**< \brief Resistive Pull Up/Down  */
        /** @} driveMode */
    /** @} group_constants */
#endif

/* Digital Port Constants */
#define KEY_LOCK_MASK               KEY_LOCK__MASK
#define KEY_LOCK_SHIFT              KEY_LOCK__SHIFT
#define KEY_LOCK_WIDTH              1u

/**
* \addtogroup group_constants
* @{
*/
    /** \addtogroup intrMode Interrupt constants
     * \brief Constants to be passed as "mode" parameter in KEY_LOCK_SetInterruptMode() function.
     *  @{
     */
        #define KEY_LOCK_INTR_NONE      ((uint16)(0x0000u)) /**< \brief Disabled             */
        #define KEY_LOCK_INTR_RISING    ((uint16)(0x5555u)) /**< \brief Rising edge trigger  */
        #define KEY_LOCK_INTR_FALLING   ((uint16)(0xaaaau)) /**< \brief Falling edge trigger */
        #define KEY_LOCK_INTR_BOTH      ((uint16)(0xffffu)) /**< \brief Both edge trigger    */
    /** @} intrMode */
/** @} group_constants */

/* SIO LPM definition */
#if defined(KEY_LOCK__SIO)
    #define KEY_LOCK_SIO_LPM_MASK       (0x03u)
#endif

/* USBIO definitions */
#if !defined(KEY_LOCK__PC) && (CY_PSOC4_4200L)
    #define KEY_LOCK_USBIO_ENABLE               ((uint32)0x80000000u)
    #define KEY_LOCK_USBIO_DISABLE              ((uint32)(~KEY_LOCK_USBIO_ENABLE))
    #define KEY_LOCK_USBIO_SUSPEND_SHIFT        CYFLD_USBDEVv2_USB_SUSPEND__OFFSET
    #define KEY_LOCK_USBIO_SUSPEND_DEL_SHIFT    CYFLD_USBDEVv2_USB_SUSPEND_DEL__OFFSET
    #define KEY_LOCK_USBIO_ENTER_SLEEP          ((uint32)((1u << KEY_LOCK_USBIO_SUSPEND_SHIFT) \
                                                        | (1u << KEY_LOCK_USBIO_SUSPEND_DEL_SHIFT)))
    #define KEY_LOCK_USBIO_EXIT_SLEEP_PH1       ((uint32)~((uint32)(1u << KEY_LOCK_USBIO_SUSPEND_SHIFT)))
    #define KEY_LOCK_USBIO_EXIT_SLEEP_PH2       ((uint32)~((uint32)(1u << KEY_LOCK_USBIO_SUSPEND_DEL_SHIFT)))
    #define KEY_LOCK_USBIO_CR1_OFF              ((uint32)0xfffffffeu)
#endif


/***************************************
*             Registers        
***************************************/
/* Main Port Registers */
#if defined(KEY_LOCK__PC)
    /* Port Configuration */
    #define KEY_LOCK_PC                 (* (reg32 *) KEY_LOCK__PC)
#endif
/* Pin State */
#define KEY_LOCK_PS                     (* (reg32 *) KEY_LOCK__PS)
/* Data Register */
#define KEY_LOCK_DR                     (* (reg32 *) KEY_LOCK__DR)
/* Input Buffer Disable Override */
#define KEY_LOCK_INP_DIS                (* (reg32 *) KEY_LOCK__PC2)

/* Interrupt configuration Registers */
#define KEY_LOCK_INTCFG                 (* (reg32 *) KEY_LOCK__INTCFG)
#define KEY_LOCK_INTSTAT                (* (reg32 *) KEY_LOCK__INTSTAT)

/* "Interrupt cause" register for Combined Port Interrupt (AllPortInt) in GSRef component */
#if defined (CYREG_GPIO_INTR_CAUSE)
    #define KEY_LOCK_INTR_CAUSE         (* (reg32 *) CYREG_GPIO_INTR_CAUSE)
#endif

/* SIO register */
#if defined(KEY_LOCK__SIO)
    #define KEY_LOCK_SIO_REG            (* (reg32 *) KEY_LOCK__SIO)
#endif /* (KEY_LOCK__SIO_CFG) */

/* USBIO registers */
#if !defined(KEY_LOCK__PC) && (CY_PSOC4_4200L)
    #define KEY_LOCK_USB_POWER_REG       (* (reg32 *) CYREG_USBDEVv2_USB_POWER_CTRL)
    #define KEY_LOCK_CR1_REG             (* (reg32 *) CYREG_USBDEVv2_CR1)
    #define KEY_LOCK_USBIO_CTRL_REG      (* (reg32 *) CYREG_USBDEVv2_USB_USBIO_CTRL)
#endif    
    
    
/***************************************
* The following code is DEPRECATED and 
* must not be used in new designs.
***************************************/
/**
* \addtogroup group_deprecated
* @{
*/
#define KEY_LOCK_DRIVE_MODE_SHIFT       (0x00u)
#define KEY_LOCK_DRIVE_MODE_MASK        (0x07u << KEY_LOCK_DRIVE_MODE_SHIFT)
/** @} deprecated */

#endif /* End Pins KEY_LOCK_H */


/* [] END OF FILE */
