
#include "setup.h"
#include "lib/ST7789V.h"
#include "lib/FT5x46.h"
#include "lib/sysTick.h"
#include "lib/LEDControl.h"
#include "lib/UI.h"
#include "lib/USB.h"
#include "userTimer.h"

uint8 OneSecondIndex;

void SetUp(void)
{
    BUZZER_Write(0);
    
    //LCD    
    SPIM_LCD_Start(); 
    LCD_RES_Write(0);
    CyDelay(10);
    
    ST7789V_Init();
    initUI();
    setDisplayDirection(DISPLAY_DIRECTION_PORTRAIT);
        
    TouchHardwareInit(); //Touch
    I2C_TC_Start();    
    
    // SPIM_FLASH_Start(); // External Flash Memory
    
    initLEDControl();
    initTimer();
    initUSB();
    UART_Start();
    /* initWIFI(); -- WiFi 미사용 (UART TEST 버전) */
    
    OneSecondIndex = registerCounter_1s(1);
    
    // initUserProject();
    // initServer();        
    // initExternalFlash();
    initLEDControl();
  
    // initInternalFlash();
    //initRepository();

    //initUSBUARTConfig();   

    //UART_WIFI_Start();
    //WIFI_EN_Write(1); 
    //WIFI_Init();
            
    //PWM_ONE_SECOND_Start();
    //UART_Start();
            
    //LCD
    //SPIM_LCD_Start(); 
    //LCD_RES_Write(0);
    //CyDelay(10);       
    //LCD_RES_Write(1);
    
    //Touch
    //TouchHardwareInit();
    //I2C_TC_Start();
            
    //UART_UartPutString("=== SUNTECH IoT ===\r\n");
        
    //ST7789V_Init();
    //initRealTimeClock();    
    //initFLASH();        
    //initInfor();
 
    // initCount();
    /* initMenu() / initWarning() / ANDON 자동진입 -- UART TEST 버전에서 미사용 */
}


/* [] END OF FILE */