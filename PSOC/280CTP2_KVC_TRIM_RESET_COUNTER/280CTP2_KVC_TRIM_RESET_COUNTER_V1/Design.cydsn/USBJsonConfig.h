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
#ifndef _USB_JSON_PARSOR_H_
#define _USB_JSON_PARSOR_H_

#include "lib/USB.h"    

enum ENUM_REQUEST_TYPE {
    REQ_NONE = 0,
    REQ_SYSTEM_CMD,
    REQ_AT_CMD,
    REQ_READ_CONFIG,
    REQ_WRITE_CONFIG,
};

extern uint8 g_USBRequest;

typedef struct {
    char url[50];
    uint8 SSIDIndex;
    uint8 bWithMAC;
    uint16 uRequestType;
} CONFIG_META;

extern CONFIG_META g_ConfigMeta;

void initUSBJsonParsor();
void usbJsonParsorLoop();
void SendConfigData();
#endif    
/* [] END OF FILE */

