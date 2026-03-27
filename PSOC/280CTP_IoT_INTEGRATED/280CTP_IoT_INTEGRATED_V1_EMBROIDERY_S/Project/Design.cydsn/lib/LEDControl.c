/* ========================================
 *
 * Copyright Suntech, 2023.02.01
 * All Rights Reserved
 * UNPUBLISHED, LICENSED SOFTWARE.
 *
 * CONFIDENTIAL AND PROPRIETARY INFORMATION
 * WHICH IS THE PROPERTY OF Suntech.
 *
 * ========================================
*/
#include "LEDControl.h"
#include "externalFlash.h"
uint16 g_uLED1_Color = LED_OFF;
uint16 g_uLED2_Color = LED_OFF;
uint8  g_bLED1_Flickering = FALSE;   
uint8  g_bLED2_Flickering = FALSE;  

void initLEDControl()
{
    TCPWM_LED_Start();    
    Control_Reg_LED_1_Write(g_uLED1_Color);
    Control_Reg_LED_2_Write(g_uLED2_Color);  
    
    if(getMiscConfig()->uBrightness < 0 || getMiscConfig()->uBrightness > 99) getMiscConfig()->uBrightness = 50;
  
    LED_Brightness(getMiscConfig()->uBrightness); 
}

void LED_Brightness(uint16 brightness)
{
    TCPWM_LED_WriteCompare(brightness);
}

void LED_OneSecondControl()
{
    static uint8 count =0;
    static uint8 Flickering = FALSE;
    
    Flickering = Flickering ? FALSE : TRUE;
    
    if(g_bLED1_Flickering)
    {
        if(Flickering) Control_Reg_LED_1_Write(g_uLED1_Color);
        else           Control_Reg_LED_1_Write(LED_OFF);
    } else {
        Control_Reg_LED_1_Write(g_uLED1_Color);
    }
    

    if(g_bLED2_Flickering)
    {
        if(Flickering) Control_Reg_LED_2_Write(g_uLED2_Color);
        else           Control_Reg_LED_2_Write(LED_OFF);
    } else {
        Control_Reg_LED_2_Write(g_uLED2_Color);
    }
    
}
/* [] END OF FILE */
