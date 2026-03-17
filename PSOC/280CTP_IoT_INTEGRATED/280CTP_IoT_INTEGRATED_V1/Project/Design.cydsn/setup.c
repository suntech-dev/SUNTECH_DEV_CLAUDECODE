
#include "setup.h"
#include "lib/ST7789V.h"
#include "lib/FT5x46.h"
#include "lib/sysTick.h"
#include "lib/menu.h"
#include "lib/RealTimeClock.h"
#include "lib/LEDControl.h"
#include "lib/w25qxx.h"
#include "lib/UI.h"
#include "lib/WIFI.h"
#include "lib/externalFlash.h"
#include "lib/internalFlash.h"
#include "lib/server.h"
#include "userTimer.h"
#include "USBJsonConfig.h"
#include "count.h"
#include "WarningLight.h"

//#include "repository.h"
#include "andonApi.h"
#include "userMenu.h"
#include "package.h"
#include "uartProcess.h"

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
        
    TouchHardwareInit(); //Touch
    I2C_TC_Start();    
    
    SPIM_FLASH_Start(); // External Flash Memory
    
    initUartProcess();
    
    initLEDControl();
    initTimer();
        
    initUSBJsonParsor();
    initWIFI();
    
    OneSecondIndex = registerCounter_1s(1);
    
    initUserProject();
    initServer();        
    initExternalFlash();
    initLEDControl();
  
    initInternalFlash();
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
 
    initCount();    
    initMenu();   
    initWarning();
    MenuLoop();
    
    if(getCount()->andonEntry==TRUE)
    {        
        MENUNODE *find = findChildNode(g_MenuNode, "ANDON");
        if(find)
        {
            g_MenuNode = find;
            g_MenuNode->func((MENUNODE *) g_MenuNode, TRUE); 
        }
    }    
}

/* [] END OF FILE */