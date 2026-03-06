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
#include "setup.h"
#include "lib/ST7789V.h"
#include "lib/FT5x46.h"
#include "lib/sysTick.h"
#include "lib/menu.h"
#include "lib/RealTimeClock.h"
#include "lib/LEDControl.h"
#include "lib/w25qxx.h"
#include "lib/UI.h"
#include "lib/externalFlash.h"
#include "lib/internalFlash.h"
#include "lib/USB.h"
#include "userTimer.h"
#include "count.h"

//#include "repository.h"
#include "userMenu.h"
#include "package.h"

uint8 OneSecondIndex;

void SetUp(void)
{
    BUZZER_Write(0);
    
    // LCD    
    SPIM_LCD_Start(); 
    LCD_RES_Write(0);
    CyDelay(10);
    
    ST7789V_Init(); 
    initUI();
        
    TouchHardwareInit(); //Touch
    I2C_TC_Start();    
    
    SPIM_FLASH_Start(); // External Flash Memory
    UART_Start();
    
    initLEDControl();
    initTimer();

    OneSecondIndex = registerCounter_1s(1);

    initUserProject();
    initExternalFlash();
    initLEDControl();

    initInternalFlash();
 //   initRepository();

    initUSB();    // USB 초기화 추가 - COM 포트 생성을 위해 필요
//    initUSBUARTConfig();    
//    

////    UART_WIFI_Start();
////    WIFI_EN_Write(1); 
////    WIFI_Init();
//        
//    PWM_ONE_SECOND_Start();
//    UART_Start();
//        
//    // LCD
//    SPIM_LCD_Start(); 
//    LCD_RES_Write(0);
//    CyDelay(10);
//    
//    LCD_RES_Write(1);
//
//    //Touch
//    TouchHardwareInit();
//    I2C_TC_Start();
//        
//    UART_UartPutString("=== SUNTECH IoT ===\r\n");
//    
//    ST7789V_Init();    
//    
//    initRealTimeClock();
//
//    initFLASH();
//    
// //   initInfor();
 
    initCount();    
    initMenu();    
    MenuLoop();
}

/* [] END OF FILE */
