/*******************************************************************************
* File Name: InputPin.c  
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
#include "InputPin.h"

static InputPin_BACKUP_STRUCT  InputPin_backup = {0u, 0u, 0u};


/*******************************************************************************
* Function Name: InputPin_Sleep
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
*  \snippet InputPin_SUT.c usage_InputPin_Sleep_Wakeup
*******************************************************************************/
void InputPin_Sleep(void)
{
    #if defined(InputPin__PC)
        InputPin_backup.pcState = InputPin_PC;
    #else
        #if (CY_PSOC4_4200L)
            /* Save the regulator state and put the PHY into suspend mode */
            InputPin_backup.usbState = InputPin_CR1_REG;
            InputPin_USB_POWER_REG |= InputPin_USBIO_ENTER_SLEEP;
            InputPin_CR1_REG &= InputPin_USBIO_CR1_OFF;
        #endif
    #endif
    #if defined(CYIPBLOCK_m0s8ioss_VERSION) && defined(InputPin__SIO)
        InputPin_backup.sioState = InputPin_SIO_REG;
        /* SIO requires unregulated output buffer and single ended input buffer */
        InputPin_SIO_REG &= (uint32)(~InputPin_SIO_LPM_MASK);
    #endif  
}


/*******************************************************************************
* Function Name: InputPin_Wakeup
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
*  Refer to InputPin_Sleep() for an example usage.
*******************************************************************************/
void InputPin_Wakeup(void)
{
    #if defined(InputPin__PC)
        InputPin_PC = InputPin_backup.pcState;
    #else
        #if (CY_PSOC4_4200L)
            /* Restore the regulator state and come out of suspend mode */
            InputPin_USB_POWER_REG &= InputPin_USBIO_EXIT_SLEEP_PH1;
            InputPin_CR1_REG = InputPin_backup.usbState;
            InputPin_USB_POWER_REG &= InputPin_USBIO_EXIT_SLEEP_PH2;
        #endif
    #endif
    #if defined(CYIPBLOCK_m0s8ioss_VERSION) && defined(InputPin__SIO)
        InputPin_SIO_REG = InputPin_backup.sioState;
    #endif
}


/* [] END OF FILE */
