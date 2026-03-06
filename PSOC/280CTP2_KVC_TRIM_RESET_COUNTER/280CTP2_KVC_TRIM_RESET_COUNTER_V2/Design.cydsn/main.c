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
#include "menuDesign.h"
#include "lib/LEDControl.h"
#include "lib/USB.h"
#include "count.h"
#include "Bootloadable.h"
#include <string.h>

//#include "lib/image.h"
//#include "lib/fonts.h"

extern uint8 OneSecondIndex;

// image.h를 사용하면, SRAM의 50%를 차지한다.

// 부트로더 진입 함수 (서브메뉴에서 호출용)
void EnterBootloaderMode(void)
{
    printf("Entering Bootloader Mode...\r\n");

    // LED 표시 (빨간색 5번 깜박임)
    /* for(int i = 0; i < 5; i++)
    {
        LED1_R_Write(1);
        CyDelay(200);
        LED1_R_Write(0);
        CyDelay(200);
    } */
    // 마지막에 켜진 상태 유지
    /* LED1_R_Write(1);
    CyDelay(1000); */

    // 부트로더 모드로 전환
    Bootloadable_Load();
    CySoftwareReset();
}

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

    printf("Hello. Main loop started.\r\n");
    printf("Use SET menu -> USB Update to enter bootloader mode.\r\n");

    for(;;)
    {
        OneMilliSecond_MainLoop(); //1ms 마다 작동하는 함수 (시간은 정확하지 않음)
        OneSecond_MainLoop();      // 1s 마다 작동하는 함수 (시간은 정확하지 않음)

        // CheckTitleTouch();      // 제거: 새로운 서브메뉴 방식으로 변경

        if(isFinishCounter_1s(OneSecondIndex)) LED_OneSecondControl();

        MenuLoop();

        SetCountLoop();
    }
}


/* [] END OF FILE */
