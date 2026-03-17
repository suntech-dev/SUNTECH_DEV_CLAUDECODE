
#include "main.h"
#include "setup.h"
#include "userTimer.h"
#include "lib/LEDControl.h"
#include "lib/USB.h"
#include "uartJson.h"

extern uint8 OneSecondIndex;

int main(void)
{
    /* Enable WCO */
    CySysClkWcoStart();

    CyGlobalIntEnable; /* Enable global interrupts. */

    SetUp();

    LCD_Backlight_Write(1);

    // g_uLED1_Color = LED_RED;
    g_uLED1_Color = LED_GREEN;
    // g_bLED1_Flickering = TRUE;

    printf("SunTech CTP280 UART RX TEST Start...\r\n");

    for(;;)
    {
        OneMilliSecond_MainLoop(); /* 1ms 마다 작동하는 함수 */
        OneSecond_MainLoop();      /* 1s 마다 작동하는 함수 */

        usbLoop();                 /* USB CDC enumeration 처리 */
        uartJsonLoop();            /* UART 수신 + JSON 파싱 */

        if(isFinishCounter_1s(OneSecondIndex))
        {
            LED_OneSecondControl();
        }

        uartJsonDrawScreen();      /* LCD 뷰어 화면 갱신 */
        uartJsonHandleTouch();     /* Up/Down 스크롤 터치 처리 */
    }
}
/* [] END OF FILE */
