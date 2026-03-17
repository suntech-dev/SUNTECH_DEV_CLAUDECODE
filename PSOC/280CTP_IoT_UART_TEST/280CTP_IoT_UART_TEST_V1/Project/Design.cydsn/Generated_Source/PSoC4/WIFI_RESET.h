/*******************************************************************************
* File Name: WIFI_RESET.h  
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

#if !defined(CY_PINS_WIFI_RESET_H) /* Pins WIFI_RESET_H */
#define CY_PINS_WIFI_RESET_H

#include "cytypes.h"
#include "cyfitter.h"
#include "WIFI_RESET_aliases.h"


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
} WIFI_RESET_BACKUP_STRUCT;

/** @} structures */


/***************************************
*        Function Prototypes             
***************************************/
/**
* \addtogroup group_general
* @{
*/
uint8   WIFI_RESET_Read(void);
void    WIFI_RESET_Write(uint8 value);
uint8   WIFI_RESET_ReadDataReg(void);
#if defined(WIFI_RESET__PC) || (CY_PSOC4_4200L) 
    void    WIFI_RESET_SetDriveMode(uint8 mode);
#endif
void    WIFI_RESET_SetInterruptMode(uint16 position, uint16 mode);
uint8   WIFI_RESET_ClearInterrupt(void);
/** @} general */

/**
* \addtogroup group_power
* @{
*/
void WIFI_RESET_Sleep(void); 
void WIFI_RESET_Wakeup(void);
/** @} power */


/***************************************
*           API Constants        
***************************************/
#if defined(WIFI_RESET__PC) || (CY_PSOC4_4200L) 
    /* Drive Modes */
    #define WIFI_RESET_DRIVE_MODE_BITS        (3)
    #define WIFI_RESET_DRIVE_MODE_IND_MASK    (0xFFFFFFFFu >> (32 - WIFI_RESET_DRIVE_MODE_BITS))

    /**
    * \addtogroup group_constants
    * @{
    */
        /** \addtogroup driveMode Drive mode constants
         * \brief Constants to be passed as "mode" parameter in the WIFI_RESET_SetDriveMode() function.
         *  @{
         */
        #define WIFI_RESET_DM_ALG_HIZ         (0x00u) /**< \brief High Impedance Analog   */
        #define WIFI_RESET_DM_DIG_HIZ         (0x01u) /**< \brief High Impedance Digital  */
        #define WIFI_RESET_DM_RES_UP          (0x02u) /**< \brief Resistive Pull Up       */
        #define WIFI_RESET_DM_RES_DWN         (0x03u) /**< \brief Resistive Pull Down     */
        #define WIFI_RESET_DM_OD_LO           (0x04u) /**< \brief Open Drain, Drives Low  */
        #define WIFI_RESET_DM_OD_HI           (0x05u) /**< \brief Open Drain, Drives High */
        #define WIFI_RESET_DM_STRONG          (0x06u) /**< \brief Strong Drive            */
        #define WIFI_RESET_DM_RES_UPDWN       (0x07u) /**< \brief Resistive Pull Up/Down  */
        /** @} driveMode */
    /** @} group_constants */
#endif

/* Digital Port Constants */
#define WIFI_RESET_MASK               WIFI_RESET__MASK
#define WIFI_RESET_SHIFT              WIFI_RESET__SHIFT
#define WIFI_RESET_WIDTH              1u

/**
* \addtogroup group_constants
* @{
*/
    /** \addtogroup intrMode Interrupt constants
     * \brief Constants to be passed as "mode" parameter in WIFI_RESET_SetInterruptMode() function.
     *  @{
     */
        #define WIFI_RESET_INTR_NONE      ((uint16)(0x0000u)) /**< \brief Disabled             */
        #define WIFI_RESET_INTR_RISING    ((uint16)(0x5555u)) /**< \brief Rising edge trigger  */
        #define WIFI_RESET_INTR_FALLING   ((uint16)(0xaaaau)) /**< \brief Falling edge trigger */
        #define WIFI_RESET_INTR_BOTH      ((uint16)(0xffffu)) /**< \brief Both edge trigger    */
    /** @} intrMode */
/** @} group_constants */

/* SIO LPM definition */
#if defined(WIFI_RESET__SIO)
    #define WIFI_RESET_SIO_LPM_MASK       (0x03u)
#endif

/* USBIO definitions */
#if !defined(WIFI_RESET__PC) && (CY_PSOC4_4200L)
    #define WIFI_RESET_USBIO_ENABLE               ((uint32)0x80000000u)
    #define WIFI_RESET_USBIO_DISABLE              ((uint32)(~WIFI_RESET_USBIO_ENABLE))
    #define WIFI_RESET_USBIO_SUSPEND_SHIFT        CYFLD_USBDEVv2_USB_SUSPEND__OFFSET
    #define WIFI_RESET_USBIO_SUSPEND_DEL_SHIFT    CYFLD_USBDEVv2_USB_SUSPEND_DEL__OFFSET
    #define WIFI_RESET_USBIO_ENTER_SLEEP          ((uint32)((1u << WIFI_RESET_USBIO_SUSPEND_SHIFT) \
                                                        | (1u << WIFI_RESET_USBIO_SUSPEND_DEL_SHIFT)))
    #define WIFI_RESET_USBIO_EXIT_SLEEP_PH1       ((uint32)~((uint32)(1u << WIFI_RESET_USBIO_SUSPEND_SHIFT)))
    #define WIFI_RESET_USBIO_EXIT_SLEEP_PH2       ((uint32)~((uint32)(1u << WIFI_RESET_USBIO_SUSPEND_DEL_SHIFT)))
    #define WIFI_RESET_USBIO_CR1_OFF              ((uint32)0xfffffffeu)
#endif


/***************************************
*             Registers        
***************************************/
/* Main Port Registers */
#if defined(WIFI_RESET__PC)
    /* Port Configuration */
    #define WIFI_RESET_PC                 (* (reg32 *) WIFI_RESET__PC)
#endif
/* Pin State */
#define WIFI_RESET_PS                     (* (reg32 *) WIFI_RESET__PS)
/* Data Register */
#define WIFI_RESET_DR                     (* (reg32 *) WIFI_RESET__DR)
/* Input Buffer Disable Override */
#define WIFI_RESET_INP_DIS                (* (reg32 *) WIFI_RESET__PC2)

/* Interrupt configuration Registers */
#define WIFI_RESET_INTCFG                 (* (reg32 *) WIFI_RESET__INTCFG)
#define WIFI_RESET_INTSTAT                (* (reg32 *) WIFI_RESET__INTSTAT)

/* "Interrupt cause" register for Combined Port Interrupt (AllPortInt) in GSRef component */
#if defined (CYREG_GPIO_INTR_CAUSE)
    #define WIFI_RESET_INTR_CAUSE         (* (reg32 *) CYREG_GPIO_INTR_CAUSE)
#endif

/* SIO register */
#if defined(WIFI_RESET__SIO)
    #define WIFI_RESET_SIO_REG            (* (reg32 *) WIFI_RESET__SIO)
#endif /* (WIFI_RESET__SIO_CFG) */

/* USBIO registers */
#if !defined(WIFI_RESET__PC) && (CY_PSOC4_4200L)
    #define WIFI_RESET_USB_POWER_REG       (* (reg32 *) CYREG_USBDEVv2_USB_POWER_CTRL)
    #define WIFI_RESET_CR1_REG             (* (reg32 *) CYREG_USBDEVv2_CR1)
    #define WIFI_RESET_USBIO_CTRL_REG      (* (reg32 *) CYREG_USBDEVv2_USB_USBIO_CTRL)
#endif    
    
    
/***************************************
* The following code is DEPRECATED and 
* must not be used in new designs.
***************************************/
/**
* \addtogroup group_deprecated
* @{
*/
#define WIFI_RESET_DRIVE_MODE_SHIFT       (0x00u)
#define WIFI_RESET_DRIVE_MODE_MASK        (0x07u << WIFI_RESET_DRIVE_MODE_SHIFT)
/** @} deprecated */

#endif /* End Pins WIFI_RESET_H */


/* [] END OF FILE */
