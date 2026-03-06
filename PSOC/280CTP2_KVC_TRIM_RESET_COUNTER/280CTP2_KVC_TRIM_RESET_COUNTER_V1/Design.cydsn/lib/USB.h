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
#ifndef _USB_H_
#define _USB_H_
    
#include "main.h"
    
#define MAX_USB_RECEIVE_BUFFER 500
    
void initUSB();
void usbLoop();

void USB_ReceiveData(uint8 *buffer, int size);

uint8 *getUSB_ReceiveBuffer();  // g_USB_ReceiveBuffer[]의 pointer 가져옴
extern uint16 g_size_USB_ReceiveBuffer; // USB에서 수신된 data size

void printf_USB(const char *fmt, ...);

#endif    

/* [] END OF FILE */
