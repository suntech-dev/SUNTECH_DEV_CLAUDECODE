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
#ifndef __WARNING_LIGHT_H__
#define __WARNING_LIGHT_H__
#include "main.h"
    
typedef struct {
    uint8_t bRED;
    uint16_t uBlinkTimeRED;
    
    uint8_t bGREEN;
    uint16_t uBlinkTimeGREEN;
    
    uint8_t bBUZZER;
    uint16_t uBlinkTimeBUZZER;
    
    uint16 index_RED;
    uint16 index_GREEN;
    uint16 index_BUZZER;
    
} WARNING_LIGHT;

void initWarning();
void WarningLightSet(int16 index, uint8 OnOff, uint16 blinkTimeInterval);
void WarningLight();

#define USE_BUZZER
#define BUZZER_ON_OFF_TIME_INTERVAL 500

#endif
/* [] END OF FILE */
