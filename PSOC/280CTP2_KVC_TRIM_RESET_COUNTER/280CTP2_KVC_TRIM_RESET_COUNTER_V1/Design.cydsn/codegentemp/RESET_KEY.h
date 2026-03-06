/*******************************************************************************
* File Name: RESET_KEY.h  
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

#if !defined(CY_PINS_RESET_KEY_H) /* Pins RESET_KEY_H */
#define CY_PINS_RESET_KEY_H

#include "cytypes.h"
#include "cyfitter.h"
#include "RESET_KEY_aliases.h"


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
} RESET_KEY_BACKUP_STRUCT;

/** @} structures */


/***************************************
*        Function Prototypes             
***************************************/
/**
* \addtogroup group_general
* @{
*/
uint8   RESET_KEY_Read(void);
void    RESET_KEY_Write(uint8 value);
uint8   RESET_KEY_ReadDataReg(void);
#if defined(RESET_KEY__PC) || (CY_PSOC4_4200L) 
    void    RESET_KEY_SetDriveMode(uint8 mode);
#endif
void    RESET_KEY_SetInterruptMode(uint16 position, uint16 mode);
uint8   RESET_KEY_ClearInterrupt(void);
/** @} general */

/**
* \addtogroup group_power
* @{
*/
void RESET_KEY_Sleep(void); 
void RESET_KEY_Wakeup(void);
/** @} power */


/***************************************
*           API Constants        
***************************************/
#if defined(RESET_KEY__PC) || (CY_PSOC4_4200L) 
    /* Drive Modes */
    #define RESET_KEY_DRIVE_MODE_BITS        (3)
    #define RESET_KEY_DRIVE_MODE_IND_MASK    (0xFFFFFFFFu >> (32 - RESET_KEY_DRIVE_MODE_BITS))

    /**
    * \addtogroup group_constants
    * @{
    */
        /** \addtogroup driveMode Drive mode constants
         * \brief Constants to be passed as "mode" parameter in the RESET_KEY_SetDriveMode() function.
         *  @{
         */
        #define RESET_KEY_DM_ALG_HIZ         (0x00u) /**< \brief High Impedance Analog   */
        #define RESET_KEY_DM_DIG_HIZ         (0x01u) /**< \brief High Impedance Digital  */
        #define RESET_KEY_DM_RES_UP          (0x02u) /**< \brief Resistive Pull Up       */
        #define RESET_KEY_DM_RES_DWN         (0x03u) /**< \brief Resistive Pull Down     */
        #define RESET_KEY_DM_OD_LO           (0x04u) /**< \brief Open Drain, Drives Low  */
        #define RESET_KEY_DM_OD_HI           (0x05u) /**< \brief Open Drain, Drives High */
        #define RESET_KEY_DM_STRONG          (0x06u) /**< \brief Strong Drive            */
        #define RESET_KEY_DM_RES_UPDWN       (0x07u) /**< \brief Resistive Pull Up/Down  */
        /** @} driveMode */
    /** @} group_constants */
#endif

/* Digital Port Constants */
#define RESET_KEY_MASK               RESET_KEY__MASK
#define RESET_KEY_SHIFT              RESET_KEY__SHIFT
#define RESET_KEY_WIDTH              1u

/**
* \addtogroup group_constants
* @{
*/
    /** \addtogroup intrMode Interrupt constants
     * \brief Constants to be passed as "mode" parameter in RESET_KEY_SetInterruptMode() function.
     *  @{
     */
        #define RESET_KEY_INTR_NONE      ((uint16)(0x0000u)) /**< \brief Disabled             */
        #define RESET_KEY_INTR_RISING    ((uint16)(0x5555u)) /**< \brief Rising edge trigger  */
        #define RESET_KEY_INTR_FALLING   ((uint16)(0xaaaau)) /**< \brief Falling edge trigger */
        #define RESET_KEY_INTR_BOTH      ((uint16)(0xffffu)) /**< \brief Both edge trigger    */
    /** @} intrMode */
/** @} group_constants */

/* SIO LPM definition */
#if defined(RESET_KEY__SIO)
    #define RESET_KEY_SIO_LPM_MASK       (0x03u)
#endif

/* USBIO definitions */
#if !defined(RESET_KEY__PC) && (CY_PSOC4_4200L)
    #define RESET_KEY_USBIO_ENABLE               ((uint32)0x80000000u)
    #define RESET_KEY_USBIO_DISABLE              ((uint32)(~RESET_KEY_USBIO_ENABLE))
    #define RESET_KEY_USBIO_SUSPEND_SHIFT        CYFLD_USBDEVv2_USB_SUSPEND__OFFSET
    #define RESET_KEY_USBIO_SUSPEND_DEL_SHIFT    CYFLD_USBDEVv2_USB_SUSPEND_DEL__OFFSET
    #define RESET_KEY_USBIO_ENTER_SLEEP          ((uint32)((1u << RESET_KEY_USBIO_SUSPEND_SHIFT) \
                                                        | (1u << RESET_KEY_USBIO_SUSPEND_DEL_SHIFT)))
    #define RESET_KEY_USBIO_EXIT_SLEEP_PH1       ((uint32)~((uint32)(1u << RESET_KEY_USBIO_SUSPEND_SHIFT)))
    #define RESET_KEY_USBIO_EXIT_SLEEP_PH2       ((uint32)~((uint32)(1u << RESET_KEY_USBIO_SUSPEND_DEL_SHIFT)))
    #define RESET_KEY_USBIO_CR1_OFF              ((uint32)0xfffffffeu)
#endif


/***************************************
*             Registers        
***************************************/
/* Main Port Registers */
#if defined(RESET_KEY__PC)
    /* Port Configuration */
    #define RESET_KEY_PC                 (* (reg32 *) RESET_KEY__PC)
#endif
/* Pin State */
#define RESET_KEY_PS                     (* (reg32 *) RESET_KEY__PS)
/* Data Register */
#define RESET_KEY_DR                     (* (reg32 *) RESET_KEY__DR)
/* Input Buffer Disable Override */
#define RESET_KEY_INP_DIS                (* (reg32 *) RESET_KEY__PC2)

/* Interrupt configuration Registers */
#define RESET_KEY_INTCFG                 (* (reg32 *) RESET_KEY__INTCFG)
#define RESET_KEY_INTSTAT                (* (reg32 *) RESET_KEY__INTSTAT)

/* "Interrupt cause" register for Combined Port Interrupt (AllPortInt) in GSRef component */
#if defined (CYREG_GPIO_INTR_CAUSE)
    #define RESET_KEY_INTR_CAUSE         (* (reg32 *) CYREG_GPIO_INTR_CAUSE)
#endif

/* SIO register */
#if defined(RESET_KEY__SIO)
    #define RESET_KEY_SIO_REG            (* (reg32 *) RESET_KEY__SIO)
#endif /* (RESET_KEY__SIO_CFG) */

/* USBIO registers */
#if !defined(RESET_KEY__PC) && (CY_PSOC4_4200L)
    #define RESET_KEY_USB_POWER_REG       (* (reg32 *) CYREG_USBDEVv2_USB_POWER_CTRL)
    #define RESET_KEY_CR1_REG             (* (reg32 *) CYREG_USBDEVv2_CR1)
    #define RESET_KEY_USBIO_CTRL_REG      (* (reg32 *) CYREG_USBDEVv2_USB_USBIO_CTRL)
#endif    
    
    
/***************************************
* The following code is DEPRECATED and 
* must not be used in new designs.
***************************************/
/**
* \addtogroup group_deprecated
* @{
*/
#define RESET_KEY_DRIVE_MODE_SHIFT       (0x00u)
#define RESET_KEY_DRIVE_MODE_MASK        (0x07u << RESET_KEY_DRIVE_MODE_SHIFT)
/** @} deprecated */

#endif /* End Pins RESET_KEY_H */


/* [] END OF FILE */
