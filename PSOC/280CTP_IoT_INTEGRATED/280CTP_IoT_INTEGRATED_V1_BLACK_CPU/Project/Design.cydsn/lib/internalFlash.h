/* ========================================
 *
 * Copyright Suntech, 2023.03.30
 * All Rights Reserved
 * UNPUBLISHED, LICENSED SOFTWARE.
 *
 * CONFIDENTIAL AND PROPRIETARY INFORMATION
 * WHICH IS THE PROPERTY OF Suntech.
 *
 * ========================================
*/

#ifndef _INTERNAL_FLASH_H_
#define _INTERNAL_FLASH_H_

#include "main.h"
    
#define INTERNAL_FLASH_WATER_MARK  12345

#define LOGICAL_EEPROM_SIZE          128u

    
// the size of following(EXTERNAL_CONFIG) data structure less than 64Byte
typedef struct {
    unsigned short   watermark;
    unsigned char    data    [LOGICAL_EEPROM_SIZE-4];    
    unsigned short   checksum;
} INTERNAL_CONFIG;

void  ResetInternalFlash();
uint8 LoadInternalFlash();
uint8 SaveInternalFlash();

void initInternalFlash();

extern INTERNAL_CONFIG g_internalFlash;

#endif
    
