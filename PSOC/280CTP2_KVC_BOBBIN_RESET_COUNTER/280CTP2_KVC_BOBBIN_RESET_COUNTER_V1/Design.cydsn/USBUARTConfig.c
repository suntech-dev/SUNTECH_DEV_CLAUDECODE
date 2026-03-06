/* ========================================
 *
 * Copyright YOUR COMPANY, THE YEAR
 * All Rights Reserved
 * UNPUBLISHED, LICENSED SOFTWARE.
 *
 * CONFIDENTIAL AND PROPRIETARY INFORMATION
 * WHICH IS THE PROPERTY OF your company.
 *
 * ========================================
*/
#include "USBUARTConfig.h"
#include "JSONParser.h"
#include "eeprom.h"
#include "wifi.h"
#include "widget.h"
#include "UI.h"
#include "displayInfo.h"

#include "w25qxx.h"
#include "RealTimeClock.h"

uint8 g_bUSBUART_Connected = FALSE;
uint8 g_requestType = 0;
uint16 g_USBUART_Config_Loop = 0;

#define USBUART_BUFFER_SIZE 64

char g_Cmd[100];

void initUSBUARTConfig()
{
    USBUART_Start(0,USBUART_5V_OPERATION);
//    USBUART_linesCoding[0][0] = 0x80u; // 9600
//    USBUART_linesCoding[0][1] = 0x25u;
//    USBUART_linesCoding[0][2] = 0x00;
//    USBUART_linesCoding[0][3] = 0x00;  
//    
//    USBUART_linesCoding[1][0] = 0x80u; // 9600
//    USBUART_linesCoding[1][1] = 0x25u;
//    USBUART_linesCoding[1][2] = 0x00;
//    USBUART_linesCoding[1][3] = 0x00;       
}

_Bool USBUART_IsTxReady(_Bool allow_blocking);


void UABUART_ConfigurationChange()
{
    /* Host can send double SET_INTERFACE request. */    
    if (0u != USBUART_IsConfigurationChanged())
    {
        /* Initialize IN endpoints when device is configured. */
        if (0u != USBUART_GetConfiguration())
        {
            /* Enumeration is done, enable OUT endpoint to receive data 
             * from host. */
            USBUART_CDC_Init();
        }
    }  
}
void loopUSBUARTConfig()
{
    uint16 count;
    uint8 buffer[USBUART_BUFFER_SIZE+1];
    /* Host can send double SET_INTERFACE request. */
    UABUART_ConfigurationChange();

    /* Service USB CDC when device is configured. */
    if (0u != USBUART_GetConfiguration())
    {
        /* Check for input data from host. */
        if (0u != USBUART_DataIsReady())
        {
            /* Read received data and re-enable OUT endpoint. */
            count = USBUART_GetAll(buffer);

            if (0u != count)
            {
                /* Wait until component is ready to send data to host. */
                while (0u == USBUART_CDCIsReady())
                {
                }

                /* Send data back to host. */
              //  USBUART_PutData(buffer, count);
                buffer[count] = 0;
                if(2==parsingReceivedData(buffer))
                {
               //     USBUART_PutString("{\"response\":\"OK\"}");
                }
                
                UART_SpiUartPutArray(buffer, count);

//                /* If the last sent packet is exactly the maximum packet 
//                *  size, it is followed by a zero-length packet to assure
//                *  that the end of the segment is properly identified by 
//                *  the terminal.
//                */
//                if (USBUART_BUFFER_SIZE == count)
//                {
//                    /* Wait until component is ready to send data to PC. */
//                    while (0u == USBUART_CDCIsReady())
//                    {
//                    }
//
//                    /* Send zero-length packet to PC. */
//                    USBUART_PutData(NULL, 0u);
//                }
            }
        }
    }
    
    
    if(isPassedOneSecond())
    {  
    //    DEBUG_printf("%d\r\n", ggggggg);
    }
   // USB_WIFI_Monitoring(0);
}

void USBUART_printf(const char *fmt, ...)
{
    va_list ap;
    char buff[USBUART_BUFFER_SIZE+1];
    
    va_start(ap, fmt);

    vsprintf(buff, fmt, ap);
    va_end(ap);
    
    uint16 nLength = strlen(buff);
    if(nLength > USBUART_BUFFER_SIZE) DEBUG_printf("Out of Data in USBUARTprintf()\r\n");
    
    while(USBUART_IsTxReady(TRUE)==0);  
    USBUART_PutString(buff); 
}


void USB_WIFI_Monitoring(char c)
{
    static uint16 count = 0;
    static uint8 buffer[256];
    static uint16 SizeOfJsonMonitoring = 0;
   // static uint16 MAX_SIZE_OF_DATA = 0;
   // static char hexChar[3];
    
    if(g_bUSBUART_Connected==FALSE) return;
    
    if (0u != USBUART_GetConfiguration())
    {
        if(c)
        {
            buffer[count++] = c;
            buffer[count  ] = 0;
        }
        if(0u == USBUART_IsTxReady(TRUE)) return;
 
        if(c == '\n' ||  count >= USBUART_BUFFER_SIZE)
        {
            if(count < USBUART_BUFFER_SIZE)
            {
                USBUART_PutData(buffer, count);
                count = 0;
            }
            else
            {
                USBUART_PutData(buffer, count);
                
                for(int i=USBUART_BUFFER_SIZE; i < count; i++) buffer[i-USBUART_BUFFER_SIZE] = buffer[i];
                
                count-= USBUART_BUFFER_SIZE;
                
                /* Wait until component is ready to send data to PC. */
                while (0u == USBUART_CDCIsReady());

                /* Send zero-length packet to PC. */
                USBUART_PutData(NULL, 0u);
            }
        }

    }
}

uint8 USBUART_Config_Loop()
{

    return FALSE;
}


_Bool usb_active = USBUART_TRUE;

_Bool USBUART_IsTxReady(_Bool allow_blocking)
{

_Bool ret = USBUART_TRUE;

 
    if(allow_blocking == USBUART_TRUE)

    {    // blocking implementation

    int8 subsystick_wrap_cnt = 0;

    int32_t subsystick_reload = (int32_t)CySysTickGetReload();    // SysTick Reload value

    int32_t subsystick_cnt = (int32_t)CySysTickGetValue();    // SysTick Timestamp when we entered this function

  

        CySysTickStart();        // Start the SysTick interval @ 1ms.  This allows USBUART_IsTxReady() to use the SysTick count for determining if the host is not responding.

        while(USBUART_CDCIsReady() == 0)

        { // Tx Port is not ready.

            if(usb_active == USBUART_FALSE) {ret = USBUART_FALSE; break;}    // break out of while() because we detected the USB port is not active.

            else

            {    // check the SysTick counter timestamp exceeds up to 2 times the SysTick Reload value (approx. 2ms)

            int32_t subsystick_cnt_diff = subsystick_cnt - (int32)CySysTickGetValue();    // get the difference between the current and prev timestamps

                if(subsystick_cnt_diff < 0) { subsystick_wrap_cnt++; }    // if current subsystick value > prev value increment wrap cnt

                if(( (subsystick_wrap_cnt*subsystick_reload) + (subsystick_cnt_diff)) > (subsystick_reload*2))

                {

                    ret = USBUART_FALSE;    // Exceeded SysTick 1ms

                    break;                    // break out of while() because we detected the USB port is not active.

                }

                subsystick_cnt = (int32_t)CySysTickGetValue();                            // store new current value

            }

        }

        usb_active = ret;

    }

    else ret = (USBUART_CDCIsReady() ? USBUART_TRUE : USBUART_FALSE);        // non-blocking

    return ret;

}
/* [] END OF FILE */
