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
#include "internalFlash.h"
#include "externalFlash.h"

#define LOGICAL_EEPROM_START         0

INTERNAL_CONFIG g_internalFlash;

uint8 IsAvaliableInternalFalsh()
{
//    if(g_internalFlash.watermark != INTERNAL_FLASH_WATER_MARK)
//    {
//        printf("WaterMark Error of EEPROM\r\n");
//        return FALSE;
//    }
//
//    if(g_internalFlash.watermark != checkSum((uint8*) &g_internalFlash.data, sizeof(INTERNAL_CONFIG)-sizeof(g_internalFlash.checksum)))
//    {
//        printf("CRC Error of EEPROM\r\n");        
//        return FALSE;
//    }

    return TRUE;
}

/*Logical Size of Em_EEPROM*/
static cy_en_em_eeprom_status_t eepromReturnValue;  /* internalFlash.c 내부 전용 */

/* EEPROM storage in work flash, this is defined in Em_EEPROM.c*/
#if defined (__ICCARM__)
#pragma data_alignment = CY_FLASH_SIZEOF_ROW
const uint8_t Em_EEPROM_em_EepromStorage[Em_EEPROM_PHYSICAL_SIZE] = {0u};
#else
const uint8_t Em_EEPROM_em_EepromStorage[Em_EEPROM_PHYSICAL_SIZE]
 __ALIGNED(CY_FLASH_SIZEOF_ROW) = {0u};
#endif /* defined (__ICCARM__) */

void initInternalFlash()
{

//    if(LOGICAL_EEPROM_SIZE != sizeof(Em_EEPROM_em_EepromStorage))
//    {
//        printf("WIFI_CONFIG size is not same as Em_EEPROM_PHYSICAL_SIZE\r\n");
//        return;
//    }
//    cy_en_em_eeprom_status_t eepromReturnValue = Em_EEPROM_Init((uint32_t)Em_EEPROM_em_EepromStorage);
    uint32_t flashAddress = (uint32_t)(0x0003FF00 -  0x00000100   -     Em_EEPROM_PHYSICAL_SIZE);
    //                                 End       don't use last block      EEPROM SIZE
//    printf("flash address %0x, %0x %0x %0x\r\n",flashAddress, 0x0003FF00, 0x00000100, Em_EEPROM_PHYSICAL_SIZE);
    
    cy_en_em_eeprom_status_t eepromReturnValue = Em_EEPROM_Init(flashAddress);
    if(eepromReturnValue != CY_EM_EEPROM_SUCCESS)
    {
        printf("ERROR in initEEPROM()\r\n");
        return;
    }    
    
    LoadInternalFlash();
}

void ResetInternalFlash()
{
    memset(&g_internalFlash, 0,  sizeof(INTERNAL_CONFIG));
    SaveInternalFlash(); 
}

uint8 LoadInternalFlash()
{
    memset(&g_internalFlash, 0,  sizeof(INTERNAL_CONFIG));
    
    eepromReturnValue = Em_EEPROM_Read(LOGICAL_EEPROM_START, &g_internalFlash, sizeof(INTERNAL_CONFIG));

    if(eepromReturnValue != CY_EM_EEPROM_SUCCESS)
    {
        memset(&g_internalFlash, 0, sizeof(INTERNAL_CONFIG));        
        return FALSE;
    }
    
    return IsAvaliableInternalFalsh();
}

uint8 SaveInternalFlash()
{
    g_internalFlash.watermark = INTERNAL_FLASH_WATER_MARK;
    g_internalFlash.checksum = checkSum((uint8*) &g_internalFlash.data, sizeof(INTERNAL_CONFIG)-sizeof(g_internalFlash.checksum));
    
    cy_en_em_eeprom_status_t eepromReturnValue = Em_EEPROM_Write(LOGICAL_EEPROM_START, &g_internalFlash,sizeof(INTERNAL_CONFIG));    
    if(eepromReturnValue != CY_EM_EEPROM_SUCCESS)
    {
        printf("Can't Save InternalFlash\r\n");
        return FALSE;
    }
    
    return TRUE;
}

/* [] END OF FILE */
