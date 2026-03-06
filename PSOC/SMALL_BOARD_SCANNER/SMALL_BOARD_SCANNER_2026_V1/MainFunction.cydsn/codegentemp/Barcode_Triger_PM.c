/*******************************************************************************
* File Name: Barcode_Triger.c  
* Version 2.20
*
* Description:
*  This file contains APIs to set up the Pins component for low power modes.
*
* Note:
*
********************************************************************************
* Copyright 2015, Cypress Semiconductor Corporation.  All rights reserved.
* You may use this file only in accordance with the license, terms, conditions, 
* disclaimers, and limitations in the end user license agreement accompanying 
* the software package with which this file was provided.
*******************************************************************************/

#include "cytypes.h"
#include "Barcode_Triger.h"

static Barcode_Triger_BACKUP_STRUCT  Barcode_Triger_backup = {0u, 0u, 0u};


/*******************************************************************************
* Function Name: Barcode_Triger_Sleep
****************************************************************************//**
*
* \brief Stores the pin configuration and prepares the pin for entering chip 
*  deep-sleep/hibernate modes. This function applies only to SIO and USBIO pins.
*  It should not be called for GPIO or GPIO_OVT pins.
*
* <b>Note</b> This function is available in PSoC 4 only.
*
* \return 
*  None 
*  
* \sideeffect
*  For SIO pins, this function configures the pin input threshold to CMOS and
*  drive level to Vddio. This is needed for SIO pins when in device 
*  deep-sleep/hibernate modes.
*
* \funcusage
*  \snippet Barcode_Triger_SUT.c usage_Barcode_Triger_Sleep_Wakeup
*******************************************************************************/
void Barcode_Triger_Sleep(void)
{
    #if defined(Barcode_Triger__PC)
        Barcode_Triger_backup.pcState = Barcode_Triger_PC;
    #else
        #if (CY_PSOC4_4200L)
            /* Save the regulator state and put the PHY into suspend mode */
            Barcode_Triger_backup.usbState = Barcode_Triger_CR1_REG;
            Barcode_Triger_USB_POWER_REG |= Barcode_Triger_USBIO_ENTER_SLEEP;
            Barcode_Triger_CR1_REG &= Barcode_Triger_USBIO_CR1_OFF;
        #endif
    #endif
    #if defined(CYIPBLOCK_m0s8ioss_VERSION) && defined(Barcode_Triger__SIO)
        Barcode_Triger_backup.sioState = Barcode_Triger_SIO_REG;
        /* SIO requires unregulated output buffer and single ended input buffer */
        Barcode_Triger_SIO_REG &= (uint32)(~Barcode_Triger_SIO_LPM_MASK);
    #endif  
}


/*******************************************************************************
* Function Name: Barcode_Triger_Wakeup
****************************************************************************//**
*
* \brief Restores the pin configuration that was saved during Pin_Sleep(). This 
* function applies only to SIO and USBIO pins. It should not be called for
* GPIO or GPIO_OVT pins.
*
* For USBIO pins, the wakeup is only triggered for falling edge interrupts.
*
* <b>Note</b> This function is available in PSoC 4 only.
*
* \return 
*  None
*  
* \funcusage
*  Refer to Barcode_Triger_Sleep() for an example usage.
*******************************************************************************/
void Barcode_Triger_Wakeup(void)
{
    #if defined(Barcode_Triger__PC)
        Barcode_Triger_PC = Barcode_Triger_backup.pcState;
    #else
        #if (CY_PSOC4_4200L)
            /* Restore the regulator state and come out of suspend mode */
            Barcode_Triger_USB_POWER_REG &= Barcode_Triger_USBIO_EXIT_SLEEP_PH1;
            Barcode_Triger_CR1_REG = Barcode_Triger_backup.usbState;
            Barcode_Triger_USB_POWER_REG &= Barcode_Triger_USBIO_EXIT_SLEEP_PH2;
        #endif
    #endif
    #if defined(CYIPBLOCK_m0s8ioss_VERSION) && defined(Barcode_Triger__SIO)
        Barcode_Triger_SIO_REG = Barcode_Triger_backup.sioState;
    #endif
}


/* [] END OF FILE */
