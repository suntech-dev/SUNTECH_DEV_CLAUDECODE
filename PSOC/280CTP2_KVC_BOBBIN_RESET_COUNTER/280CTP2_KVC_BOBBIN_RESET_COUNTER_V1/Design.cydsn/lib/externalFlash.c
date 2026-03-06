#include "externalFlash.h"
#include "w25qxx.h"
#include "UI.h"
#include "server.h"
#include "package.h"

EXTERNAL_CONFIG g_externalFlash;

uint32_t g_uSectorForConfig;
uint32_t g_uAddressForConfig;

// Data 초기화
void initExternalFlash()
{
    while(!W25qxx_Init()); 
    
    g_uSectorForConfig = (w25qxx.SectorCount-1);                   // 마지막 섹터
    g_uAddressForConfig = g_uSectorForConfig * w25qxx.SectorSize;  // 주소
       
    if(LoadExternalFlashConfig() == FALSE)
    {
        memset(&g_externalFlash,0, sizeof(g_externalFlash));
        
        SetDefaultExternalFlashConfig();
        SetUserProjectDefaultExternalFlashConfig();   
        ValidationExternalFlashConfig();    
                
        SaveExternalFlashConfig();
        return;
    }
      
    ValidationExternalFlashConfig();
    
}

// external flash에 데이터가 한번도 안썼다면 데이터 초기화
void SetDefaultExternalFlashConfig()
{
    SetDefaultConfigServer();
    
    getMiscConfig()->bDisplayDir  = DISPLAY_DIRECTION_PORTRAIT;
    getMiscConfig()->uBrightness = 50;
}

// 데이터가 유효한지 검사하고, 문제가 있으면 데이터를 초기화 한다.
void ValidationExternalFlashConfig()
{ 
    ValidationConfigServer();
    
    if(userProjectDataValidation()==FALSE) SetUserProjectDefaultExternalFlashConfig();

    if(getMiscConfig()->bDisplayDir != DISPLAY_DIRECTION_PORTRAIT &&  
       getMiscConfig()->bDisplayDir != DISPLAY_DIRECTION_LANDSCAPE)            getMiscConfig()->bDisplayDir = DISPLAY_DIRECTION_PORTRAIT;
    
    if(getMiscConfig()->uBrightness <= 0 || getMiscConfig()->uBrightness > 99) getMiscConfig()->uBrightness = 50;
            
    
}

// external flash에서 데이터를 호출한다
uint8 LoadExternalFlashConfig()
{
    uint8_t *p = (uint8 *) & g_externalFlash;
    
    W25qxx_ReadBytes(p, g_uAddressForConfig,sizeof(EXTERNAL_CONFIG));
    uint16 crc = externalFlashCRC();
    
    if(g_externalFlash.CRC != externalFlashCRC())  return FALSE;
    if(strcmp(g_externalFlash.watermark, EXTERNAL_FLASH_WATER_MARK) != 0) return FALSE;

    return TRUE;
}

// external flash에 데이터를 저장한다
void SaveExternalFlashConfig()
{
    strcpy(g_externalFlash.watermark,EXTERNAL_FLASH_WATER_MARK);
    g_externalFlash.CRC = externalFlashCRC();
    
    W25qxx_EraseSector(g_uSectorForConfig);
    
    uint8_t *p = (uint8 *) & g_externalFlash;
        
    for(uint i=0; i < sizeof(EXTERNAL_CONFIG); i++)
    {
        W25qxx_WriteByte(p[i], g_uAddressForConfig+i);
    }     
}

uint16 externalFlashCRC()
{
    return checkSum((uint8 *) &g_externalFlash, sizeof(g_externalFlash) - 4);
}

//////////////////////////////////////////////////////////////////////////////
uint16 checkSum(uint8 *data, uint16 size)
{
    uint16 checkSum = 0;
    for(int i=1; i < size; i++)
    {
        checkSum += data[i];
    }
    return (~checkSum + 1);
}

EXTERNAL_MISC_CONFIG *getMiscConfig()
{
    return (EXTERNAL_MISC_CONFIG *) &getExternalConfigData()[CONFIG_DATA_MISC];
}

EXTERNAL_CONFIG *getConfig()
{
    return (EXTERNAL_CONFIG *) &getExternalConfigData()[CONFIG_DATA_MISC];
}

unsigned char *getExternalConfigData()
{
    return g_externalFlash.data;
}
unsigned char *getExternalConfigUserData()
{
    return g_externalFlash.userData;
}