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
#include "main.h"
#include "uart.h"

void DEBUG_printf(const char *fmt, ...)
{
    va_list ap;
    char buff[1024];
    
    va_start(ap, fmt);

    vsprintf(buff, fmt, ap);
    va_end(ap);
    
    UART_USB_PutString(buff);   
}

void DEBUG_Prompt_printf(char *prompt, const char *fmt, ...)
{
    va_list ap;
    char buff[1024];
    
    va_start(ap, fmt);

    vsprintf(buff, fmt, ap);
    va_end(ap);
    UART_USB_PutString(prompt);   
    UART_USB_PutString(buff);   
}

/* [] END OF FILE */
