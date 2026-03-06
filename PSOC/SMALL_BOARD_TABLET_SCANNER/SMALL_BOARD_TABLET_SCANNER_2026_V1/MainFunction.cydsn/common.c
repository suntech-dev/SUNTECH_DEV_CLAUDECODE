/* ========================================
 *
 * Copyright SUNTECH, 2018-2026
 * All Rights Reserved
 * UNPUBLISHED, LICENSED SOFTWARE.
 *
 * CONFIDENTIAL AND PROPRIETARY INFORMATION
 * WHICH IS THE PROPERTY OF SUNTECH.
 *
 * ========================================
*/
#include "main.h"
#include <stdlib.h>
#include <stdarg.h>

void init()
{
    initSysTick();
    
    MONITORING_Start();
    USB_OP_Start();
    BARCODE_Start();
    //UART_OP_Start();
        
    //SetDesignNumber(0x00);

}


/*
void convert(char *str) //YES 3314 CPU + Green Switch
{    
    int sum = 0;
    for(unsigned int i=0; i < strlen(str); i++) sum += str[i] - '0';
    
    char ret[4];
    ret[2] = '\r';
    ret[3] = '\0';    
	
    switch(sum)
    {
       case 0:     break;
       case 1: ret[0] = 0x31;  ret[1] = 0x45;   break;     
       case 2: ret[0] = 0x31;  ret[1] = 0x44;   break;
       case 3: ret[0] = 0x31;  ret[1] = 0x43;   break;
       case 4: ret[0] = 0x31;  ret[1] = 0x42;   break;
       case 5: ret[0] = 0x31;  ret[1] = 0x41;   break;
       case 6: ret[0] = 0x31;  ret[1] = 0x39;   break;
       case 7: ret[0] = 0x31;  ret[1] = 0x38;   break;
       case 8: ret[0] = 0x31;  ret[1] = 0x37;   break;
       case 9: ret[0] = 0x31;  ret[1] = 0x36;   break;
       case 10: ret[0] = 0x31;  ret[1] = 0x35;   break;
       case 11: ret[0] = 0x31;  ret[1] = 0x34;   break;
       case 12: ret[0] = 0x31;  ret[1] = 0x33;   break;
       case 13: ret[0] = 0x31;  ret[1] = 0x32;   break;
       case 14: ret[0] = 0x31;  ret[1] = 0x31;   break;
       case 15: ret[0] = 0x31;  ret[1] = 0x30;   break;
       case 16: ret[0] = 0x30;  ret[1] = 0x46;   break;
       case 17: ret[0] = 0x30;  ret[1] = 0x45;   break;
       case 18: ret[0] = 0x30;  ret[1] = 0x44;   break;
       case 19: ret[0] = 0x30;  ret[1] = 0x43;   break;
       case 20: ret[0] = 0x30;  ret[1] = 0x42;   break;
       case 21: ret[0] = 0x30;  ret[1] = 0x41;   break;
       case 22: ret[0] = 0x30;  ret[1] = 0x39;   break;
       case 23: ret[0] = 0x30;  ret[1] = 0x38;   break;
       case 24: ret[0] = 0x30;  ret[1] = 0x37;   break;
       case 25: ret[0] = 0x30;  ret[1] = 0x36;   break;
       case 26: ret[0] = 0x30;  ret[1] = 0x35;   break;
       case 27: ret[0] = 0x30;  ret[1] = 0x34;   break;
    }
    
    UART_OP_UartPutString(ret);  
}
*/


void BootloaderStart()
{
    MONITORING_UartPutString(__DATE__);
    MONITORING_UartPutString("\r\nCurrent firmware Version is ");
    MONITORING_UartPutString(FIRMWARE_VERSION);
    MONITORING_UartPutString("\r\nFirmware Upgrade Ready");
    MONITORING_UartPutString("(115200bps).");
 
    Bootloadable_Load();    
}


/*
void SetDesignName(char *name)  //OLD OP + 8 PIN
{
    char str[20];
    strncpy(str,name,3);
    str[0] = ' ';
    
    SetDesignNumber((uint8) atoi(str));
}
*/


/*
void SetDesignNumber(uint8 number)  //OLD OP + 8 PIN
{
    DesignNumber_Write((uint8) ~number);
}
*/


/*
void xprintf(const char *fmt, ...)
{
    va_list ap;
    char buff[256];
    
    va_start(ap, fmt);

    vsprintf(buff, fmt, ap);
    va_end(ap);
    
    USB_OP_UartPutString(buff);
}
*/

/* [] END OF FILE */
