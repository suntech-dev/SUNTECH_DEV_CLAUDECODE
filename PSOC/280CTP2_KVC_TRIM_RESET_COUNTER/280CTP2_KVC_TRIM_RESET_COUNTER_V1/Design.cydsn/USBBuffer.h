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
#ifndef _USB_BUFFER_H_
#define _USB_BUFFER_H_
    
#include "lib/USB.h"
    
#define MAX_USB_RECEIVE_BUFFER 500
    
uint8 *getUSB_ReceiveBuffer();  // g_USB_ReceiveBuffer[]의 pointer 가져옴

extern uint16 g_size_USB_ReceiveBuffer; // USB에서 수신된 data size

void printf_USB(const char *fmt, ...);

#endif    
/* [] END OF FILE */
