
#include "main.h"
#include "setup.h"
#include "userTimer.h"
#include "USBJsonConfig.h"
#include "lib/WIFI.h"
#include "menuDesign.h"
#include "lib/LEDControl.h"
#include "count.h"
#include "package.h"
#include "userProjectPatternSewing.h"
#include "lib/internalFlash.h"
#include "andonApi.h"
//#include "lib/image.h"
//#include "lib/fonts.h"

extern uint8 OneSecondIndex;

// image.h를 사용하면, SRAM의 50%를 차지한다.
void currentSensorRoutine();

int main(void)
{
    /* Enable WCO */
    CySysClkWcoStart();
    
    CyGlobalIntEnable; /* Enable global interrupts. */
 
    SetUp();
 
    LCD_Backlight_Write(1);  
        
    g_uLED1_Color = LED_RED;
    g_bLED1_Flickering = TRUE;

    printf("SunTech IoT Start...\r\n");    
    //printf("00-->%d %d : %d\r\n", g_ptrMachineParameter->current_enable, g_ptrMachineParameter->current_sensor_threshold, g_ptrMachineParameter->andon_enable);
   
    for(;;)
    {

        currentSensorRoutine();

        OneMilliSecond_MainLoop(); //1ms 마다 작동하는 함수 (시간은 정확하지 않음)
        OneSecond_MainLoop();      // 1s 마다 작동하는 함수 (시간은 정확하지 않음)   
        
        usbJsonParsorLoop();
        
        wifiLoop();   
        
        if(isFinishCounter_1s(OneSecondIndex))
        {
            WorkingTimeCount();
            LED_OneSecondControl();// LED2_B_Write(!LED2_B_ReadDataReg());
            
            if(g_bStartTrimPin)
            {
                g_bTrimElapsedTime++;
                if(TrimPin_Read() != 0)
                {
                    g_bStartTrimPin = FALSE;
                    makeAndonSewingCount2(g_bTrimElapsedTime);
                    g_bTrimElapsedTime = 0;
                }
            }
        }

        MenuLoop();        
        CountFunc();        

    }
}
/* [] END OF FILE */
