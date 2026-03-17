/* ========================================
 *
 * Copyright Suntech, 2023.02.01
 * All Rights Reserved
 * UNPUBLISHED, LICENSED SOFTWARE.
 *
 * CONFIDENTIAL AND PROPRIETARY INFORMATION
 * WHICH IS THE PROPERTY OF your company.
 *
 * ========================================
*/
#include "USB.h"
#include "main.h"
#include "USBJsonConfig.h"

#define USBUART_BUFFER_SIZE 64
uint8 g_usb_buffer[USBUART_BUFFER_SIZE+1];
uint16 g_usb_count;


void initUSB()
{
    USBUART_Start(0,USBUART_5V_OPERATION);    
}

static uint8 s_cdc_init_done = 0u;
static uint8 s_startup_sent  = 0u;

void usbLoop()
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
            s_cdc_init_done = 1u;
        }
        else
        {
            /* USB 분리 — 재연결 시 스타트업 메시지 다시 표시 */
            s_cdc_init_done = 0u;
            s_startup_sent  = 0u;
        }
    }

    /* Fallback: 보드가 USB 연결 상태에서 부팅된 경우 IsConfigurationChanged()가
     * 발생하지 않아 CDC_Init()이 누락될 수 있음 — GetConfiguration()이 이미
     * non-zero 이면 여기서 한 번만 초기화. */
    if (0u == s_cdc_init_done && 0u != USBUART_GetConfiguration())
    {
        USBUART_CDC_Init();
        s_cdc_init_done = 1u;
    }

    /* Service USB CDC when device is configured. */
    if (0u != USBUART_GetConfiguration())
    {
        /* CDC가 준비된 직후 최초 1회 스타트업 메시지 전송 */
        if (0u == s_startup_sent && 0u != USBUART_CDCIsReady())
        {
            USBUART_PutString("=== USB TESTER START ===\r\n");
            s_startup_sent = 1u;
        }

        /* Check for input data from host. */
        if (0u != USBUART_DataIsReady())
        {
            /* Read received data and re-enable OUT endpoint. */
            g_usb_count = USBUART_GetAll(g_usb_buffer);

            if (0u != g_usb_count)
            {
                USB_ReceiveData(g_usb_buffer,g_usb_count);
            }
        }
    }
}


uint8 g_USB_ReceiveBuffer[MAX_USB_RECEIVE_BUFFER+1];
uint16 g_size_USB_ReceiveBuffer=0;

uint8 *getUSB_ReceiveBuffer() {return g_USB_ReceiveBuffer;};

void USB_ReceiveData(uint8 *buffer, int size)
{
    if(MAX_USB_RECEIVE_BUFFER < g_size_USB_ReceiveBuffer+size)
    {
        printf("Error : Overflow in USB_ReceiveData()\r\n");
        return;
    }
    
    memcpy(&g_USB_ReceiveBuffer[g_size_USB_ReceiveBuffer], buffer, size);
    g_size_USB_ReceiveBuffer += size;
    g_USB_ReceiveBuffer[g_size_USB_ReceiveBuffer] = 0;
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

void printf_USB(const char *fmt, ...)
{
    va_list ap;
    char buff[USBUART_BUFFER_SIZE+1];
    
    va_start(ap, fmt);

    vsprintf(buff, fmt, ap);
    va_end(ap);
    
    uint16 nLength = strlen(buff);
    if(nLength > USBUART_BUFFER_SIZE) printf("Out of Data in USBUARTprintf()\r\n");
    
    while(USBUART_IsTxReady(TRUE)==0);  
    USBUART_PutString(buff); 
}

/* [] END OF FILE */
