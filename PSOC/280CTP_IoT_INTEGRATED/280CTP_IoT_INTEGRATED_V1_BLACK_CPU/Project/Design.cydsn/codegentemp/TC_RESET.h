/*******************************************************************************
* File Name: TC_RESET.h  
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

#if !defined(CY_PINS_TC_RESET_H) /* Pins TC_RESET_H */
#define CY_PINS_TC_RESET_H

#include "cytypes.h"
#include "cyfitter.h"
#include "TC_RESET_aliases.h"


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
} TC_RESET_BACKUP_STRUCT;

/** @} structures */


/***************************************
*        Function Prototypes             
***************************************/
/**
* \addtogroup group_general
* @{
*/
uint8   TC_RESET_Read(void);
void    TC_RESET_Write(uint8 value);
uint8   TC_RESET_ReadDataReg(void);
#if defined(TC_RESET__PC) || (CY_PSOC4_4200L) 
    void    TC_RESET_SetDriveMode(uint8 mode);
#endif
void    TC_RESET_SetInterruptMode(uint16 position, uint16 mode);
uint8   TC_RESET_ClearInterrupt(void);
/** @} general */

/**
* \addtogroup group_power
* @{
*/
void TC_RESET_Sleep(void); 
void TC_RESET_Wakeup(void);
/** @} power */


/***************************************
*           API Constants        
***************************************/
#if defined(TC_RESET__PC) || (CY_PSOC4_4200L) 
    /* Drive Modes */
    #define TC_RESET_DRIVE_MODE_BITS        (3)
    #define TC_RESET_DRIVE_MODE_IND_MASK    (0xFFFFFFFFu >> (32 - TC_RESET_DRIVE_MODE_BITS))

    /**
    * \addtogroup group_constants
    * @{
    */
        /** \addtogroup driveMode Drive mode constants
         * \brief Constants to be passed as "mode" parameter in the TC_RESET_SetDriveMode() function.
         *  @{
         */
        #define TC_RESET_DM_ALG_HIZ         (0x00u) /**< \brief High Impedance Analog   */
        #define TC_RESET_DM_DIG_HIZ         (0x01u) /**< \brief High Impedance Digital  */
        #define TC_RESET_DM_RES_UP          (0x02u) /**< \brief Resistive Pull Up       */
        #define TC_RESET_DM_RES_DWN         (0x03u) /**< \brief Resistive Pull Down     */
        #define TC_RESET_DM_OD_LO           (0x04u) /**< \brief Open Drain, Drives Low  */
        #define TC_RESET_DM_OD_HI           (0x05u) /**< \brief Open Drain, Drives High */
        #define TC_RESET_DM_STRONG          (0x06u) /**< \brief Strong Drive            */
        #define TC_RESET_DM_RES_UPDWN       (0x07u) /**< \brief Resistive Pull Up/Down  */
        /** @} driveMode */
    /** @} group_constants */
#endif

/* Digital Port Constants */
#define TC_RESET_MASK               TC_RESET__MASK
#define TC_RESET_SHIFT              TC_RESET__SHIFT
#define TC_RESET_WIDTH              1u

/**
* \addtogroup group_constants
* @{
*/
    /** \addtogroup intrMode Interrupt constants
     * \brief Constants to be passed as "mode" parameter in TC_RESET_SetInterruptMode() function.
     *  @{
     */
        #define TC_RESET_INTR_NONE      ((uint16)(0x0000u)) /**< \brief Disabled             */
        #define TC_RESET_INTR_RISING    ((uint16)(0x5555u)) /**< \brief Rising edge trigger  */
        #define TC_RESET_INTR_FALLING   ((uint16)(0xaaaau)) /**< \brief Falling edge trigger */
        #define TC_RESET_INTR_BOTH      ((uint16)(0xffffu)) /**< \brief Both edge trigger    */
    /** @} intrMode */
/** @} group_constants */

/* SIO LPM definition */
#if defined(TC_RESET__SIO)
    #define TC_RESET_SIO_LPM_MASK       (0x03u)
#endif

/* USBIO definitions */
#if !defined(TC_RESET__PC) && (CY_PSOC4_4200L)
    #define TC_RESET_USBIO_ENABLE               ((uint32)0x80000000u)
    #define TC_RESET_USBIO_DISABLE              ((uint32)(~TC_RESET_USBIO_ENABLE))
    #define TC_RESET_USBIO_SUSPEND_SHIFT        CYFLD_USBDEVv2_USB_SUSPEND__OFFSET
    #define TC_RESET_USBIO_SUSPEND_DEL_SHIFT    CYFLD_USBDEVv2_USB_SUSPEND_DEL__OFFSET
    #define TC_RESET_USBIO_ENTER_SLEEP          ((uint32)((1u << TC_RESET_USBIO_SUSPEND_SHIFT) \
                                                        | (1u << TC_RESET_USBIO_SUSPEND_DEL_SHIFT)))
    #define TC_RESET_USBIO_EXIT_SLEEP_PH1       ((uint32)~((uint32)(1u << TC_RESET_USBIO_SUSPEND_SHIFT)))
    #define TC_RESET_USBIO_EXIT_SLEEP_PH2       ((uint32)~((uint32)(1u << TC_RESET_USBIO_SUSPEND_DEL_SHIFT)))
    #define TC_RESET_USBIO_CR1_OFF              ((uint32)0xfffffffeu)
#endif


/***************************************
*             Registers        
***************************************/
/* Main Port Registers */
#if defined(TC_RESET__PC)
    /* Port Configuration */
    #define TC_RESET_PC                 (* (reg32 *) TC_RESET__PC)
#endif
/* Pin State */
#define TC_RESET_PS                     (* (reg32 *) TC_RESET__PS)
/* Data Register */
#define TC_RESET_DR                     (* (reg32 *) TC_RESET__DR)
/* Input Buffer Disable Override */
#define TC_RESET_INP_DIS                (* (reg32 *) TC_RESET__PC2)

/* Interrupt configuration Registers */
#define TC_RESET_INTCFG                 (* (reg32 *) TC_RESET__INTCFG)
#define TC_RESET_INTSTAT                (* (reg32 *) TC_RESET__INTSTAT)

/* "Interrupt cause" register for Combined Port Interrupt (AllPortInt) in GSRef component */
#if defined (CYREG_GPIO_INTR_CAUSE)
    #define TC_RESET_INTR_CAUSE         (* (reg32 *) CYREG_GPIO_INTR_CAUSE)
#endif

/* SIO register */
#if defined(TC_RESET__SIO)
    #define TC_RESET_SIO_REG            (* (reg32 *) TC_RESET__SIO)
#endif /* (TC_RESET__SIO_CFG) */

/* USBIO registers */
#if !defined(TC_RESET__PC) && (CY_PSOC4_4200L)
    #define TC_RESET_USB_POWER_REG       (* (reg32 *) CYREG_USBDEVv2_USB_POWER_CTRL)
    #define TC_RESET_CR1_REG             (* (reg32 *) CYREG_USBDEVv2_CR1)
    #define TC_RESET_USBIO_CTRL_REG      (* (reg32 *) CYREG_USBDEVv2_USB_USBIO_CTRL)
#endif    
    
    
/***************************************
* The following code is DEPRECATED and 
* must not be used in new designs.
***************************************/
/**
* \addtogroup group_deprecated
* @{
*/
#define TC_RESET_DRIVE_MODE_SHIFT       (0x00u)
#define TC_RESET_DRIVE_MODE_MASK        (0x07u << TC_RESET_DRIVE_MODE_SHIFT)
/** @} deprecated */

#endif /* End Pins TC_RESET_H */


/* [] END OF FILE */
