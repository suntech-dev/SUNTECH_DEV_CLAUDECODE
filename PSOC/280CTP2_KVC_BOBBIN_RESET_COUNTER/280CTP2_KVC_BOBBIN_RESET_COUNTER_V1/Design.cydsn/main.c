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
#include "main.h"
#include "setup.h"
#include "userTimer.h"
#include "USBJsonConfig.h"
#include "lib/WIFI.h"
#include "menuDesign.h"
#include "lib/LEDControl.h"
#include "count.h"

//#include "lib/image.h"
//#include "lib/fonts.h"

extern uint8 OneSecondIndex;

// image.h를 사용하면, SRAM의 50%를 차지한다.

int main(void)
{
    /* Enable WCO */
    CySysClkWcoStart();
    
    CyGlobalIntEnable; /* Enable global interrupts. */
 
    SetUp();
 
    LCD_Backlight_Write(1);
        
    //g_uLED1_Color = LED_RED;
    //g_bLED1_Flickering = TRUE;  
    g_uLED1_Color = LED_GREEN;
    g_bLED1_Flickering = FALSE;     

    printf("Hello.\r\n");
    for(;;)
    {
        OneMilliSecond_MainLoop(); //1ms 마다 작동하는 함수 (시간은 정확하지 않음)
        OneSecond_MainLoop();      // 1s 마다 작동하는 함수 (시간은 정확하지 않음)   
        
        usbJsonParsorLoop();
        
        wifiLoop();   
        
        if(isFinishCounter_1s(OneSecondIndex)) LED_OneSecondControl();// LED2_B_Write(!LED2_B_ReadDataReg());
        
        MenuLoop();
        
        SetCountLoop();
        
       // if(RESERVED_IN_2_Read()) printf(".");
       // else printf("-");;
    }
}


/* [] END OF FILE */
