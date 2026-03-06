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
#ifndef __LED_CONTROL_H__
#define __LED_CONTROL_H__
#include "main.h"
    
#define LED_OFF         0x00       
#define LED_RED         0x01
#define LED_GREEN       0x02        
#define LED_YELLOW      0x03     
#define LED_BLUE        0x04 
#define LED_PINK        0x05
#define LED_CYAN        0x06
#define LED_WHITE       0x07 
    
extern uint16 g_uLED1_Color;
extern uint16 g_uLED2_Color;
extern uint8  g_bLED1_Flickering;    
extern uint8  g_bLED2_Flickering;    
void initLEDControl();
void LED_OneSecondControl();
void LED_Brightness(uint16 brightness);
#endif    
/* [] END OF FILE */
