/*******************************************************************************
* File Name: Counter.h  
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

#if !defined(CY_PINS_Counter_H) /* Pins Counter_H */
#define CY_PINS_Counter_H

#include "cytypes.h"
#include "cyfitter.h"
#include "Counter_aliases.h"


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
} Counter_BACKUP_STRUCT;

/** @} structures */


/***************************************
*        Function Prototypes             
***************************************/
/**
* \addtogroup group_general
* @{
*/
uint8   Counter_Read(void);
void    Counter_Write(uint8 value);
uint8   Counter_ReadDataReg(void);
#if defined(Counter__PC) || (CY_PSOC4_4200L) 
    void    Counter_SetDriveMode(uint8 mode);
#endif
void    Counter_SetInterruptMode(uint16 position, uint16 mode);
uint8   Counter_ClearInterrupt(void);
/** @} general */

/**
* \addtogroup group_power
* @{
*/
void Counter_Sleep(void); 
void Counter_Wakeup(void);
/** @} power */


/***************************************
*           API Constants        
***************************************/
#if defined(Counter__PC) || (CY_PSOC4_4200L) 
    /* Drive Modes */
    #define Counter_DRIVE_MODE_BITS        (3)
    #define Counter_DRIVE_MODE_IND_MASK    (0xFFFFFFFFu >> (32 - Counter_DRIVE_MODE_BITS))

    /**
    * \addtogroup group_constants
    * @{
    */
        /** \addtogroup driveMode Drive mode constants
         * \brief Constants to be passed as "mode" parameter in the Counter_SetDriveMode() function.
         *  @{
         */
        #define Counter_DM_ALG_HIZ         (0x00u) /**< \brief High Impedance Analog   */
        #define Counter_DM_DIG_HIZ         (0x01u) /**< \brief High Impedance Digital  */
        #define Counter_DM_RES_UP          (0x02u) /**< \brief Resistive Pull Up       */
        #define Counter_DM_RES_DWN         (0x03u) /**< \brief Resistive Pull Down     */
        #define Counter_DM_OD_LO           (0x04u) /**< \brief Open Drain, Drives Low  */
        #define Counter_DM_OD_HI           (0x05u) /**< \brief Open Drain, Drives High */
        #define Counter_DM_STRONG          (0x06u) /**< \brief Strong Drive            */
        #define Counter_DM_RES_UPDWN       (0x07u) /**< \brief Resistive Pull Up/Down  */
        /** @} driveMode */
    /** @} group_constants */
#endif

/* Digital Port Constants */
#define Counter_MASK               Counter__MASK
#define Counter_SHIFT              Counter__SHIFT
#define Counter_WIDTH              1u

/**
* \addtogroup group_constants
* @{
*/
    /** \addtogroup intrMode Interrupt constants
     * \brief Constants to be passed as "mode" parameter in Counter_SetInterruptMode() function.
     *  @{
     */
        #define Counter_INTR_NONE      ((uint16)(0x0000u)) /**< \brief Disabled             */
        #define Counter_INTR_RISING    ((uint16)(0x5555u)) /**< \brief Rising edge trigger  */
        #define Counter_INTR_FALLING   ((uint16)(0xaaaau)) /**< \brief Falling edge trigger */
        #define Counter_INTR_BOTH      ((uint16)(0xffffu)) /**< \brief Both edge trigger    */
    /** @} intrMode */
/** @} group_constants */

/* SIO LPM definition */
#if defined(Counter__SIO)
    #define Counter_SIO_LPM_MASK       (0x03u)
#endif

/* USBIO definitions */
#if !defined(Counter__PC) && (CY_PSOC4_4200L)
    #define Counter_USBIO_ENABLE               ((uint32)0x80000000u)
    #define Counter_USBIO_DISABLE              ((uint32)(~Counter_USBIO_ENABLE))
    #define Counter_USBIO_SUSPEND_SHIFT        CYFLD_USBDEVv2_USB_SUSPEND__OFFSET
    #define Counter_USBIO_SUSPEND_DEL_SHIFT    CYFLD_USBDEVv2_USB_SUSPEND_DEL__OFFSET
    #define Counter_USBIO_ENTER_SLEEP          ((uint32)((1u << Counter_USBIO_SUSPEND_SHIFT) \
                                                        | (1u << Counter_USBIO_SUSPEND_DEL_SHIFT)))
    #define Counter_USBIO_EXIT_SLEEP_PH1       ((uint32)~((uint32)(1u << Counter_USBIO_SUSPEND_SHIFT)))
    #define Counter_USBIO_EXIT_SLEEP_PH2       ((uint32)~((uint32)(1u << Counter_USBIO_SUSPEND_DEL_SHIFT)))
    #define Counter_USBIO_CR1_OFF              ((uint32)0xfffffffeu)
#endif


/***************************************
*             Registers        
***************************************/
/* Main Port Registers */
#if defined(Counter__PC)
    /* Port Configuration */
    #define Counter_PC                 (* (reg32 *) Counter__PC)
#endif
/* Pin State */
#define Counter_PS                     (* (reg32 *) Counter__PS)
/* Data Register */
#define Counter_DR                     (* (reg32 *) Counter__DR)
/* Input Buffer Disable Override */
#define Counter_INP_DIS                (* (reg32 *) Counter__PC2)

/* Interrupt configuration Registers */
#define Counter_INTCFG                 (* (reg32 *) Counter__INTCFG)
#define Counter_INTSTAT                (* (reg32 *) Counter__INTSTAT)

/* "Interrupt cause" register for Combined Port Interrupt (AllPortInt) in GSRef component */
#if defined (CYREG_GPIO_INTR_CAUSE)
    #define Counter_INTR_CAUSE         (* (reg32 *) CYREG_GPIO_INTR_CAUSE)
#endif

/* SIO register */
#if defined(Counter__SIO)
    #define Counter_SIO_REG            (* (reg32 *) Counter__SIO)
#endif /* (Counter__SIO_CFG) */

/* USBIO registers */
#if !defined(Counter__PC) && (CY_PSOC4_4200L)
    #define Counter_USB_POWER_REG       (* (reg32 *) CYREG_USBDEVv2_USB_POWER_CTRL)
    #define Counter_CR1_REG             (* (reg32 *) CYREG_USBDEVv2_CR1)
    #define Counter_USBIO_CTRL_REG      (* (reg32 *) CYREG_USBDEVv2_USB_USBIO_CTRL)
#endif    
    
    
/***************************************
* The following code is DEPRECATED and 
* must not be used in new designs.
***************************************/
/**
* \addtogroup group_deprecated
* @{
*/
#define Counter_DRIVE_MODE_SHIFT       (0x00u)
#define Counter_DRIVE_MODE_MASK        (0x07u << Counter_DRIVE_MODE_SHIFT)
/** @} deprecated */

#endif /* End Pins Counter_H */


/* [] END OF FILE */
