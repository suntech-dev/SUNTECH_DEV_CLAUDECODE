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

#ifndef _EXTERNAL_FLASH_H_
#define _EXTERNAL_FLASH_H_

#include "main.h"
    
#define EXTERNAL_FLASH_WATER_MARK "SUNTECH IOT"

#define MAX_DATA_SIZE_CONFIG          200    
#define MAX_DATA_SIZE_EXTERNAL_CONFIG 300

// the size of following(EXTERNAL_CONFIG) data structure less than 4KB
typedef struct {
    char watermark[20];
    unsigned char    data    [MAX_DATA_SIZE_CONFIG];    
    unsigned char    userData[MAX_DATA_SIZE_EXTERNAL_CONFIG];
    unsigned short   CRC;
} EXTERNAL_CONFIG;

unsigned char *getExternalConfigData();
unsigned char *getExternalConfigUserData();

void  initExternalFlash();                        // Data 초기화
void  SetDefaultExternalFlashConfig();            // external flash에 데이터가 한번도 안썼다면 데이터 초기화 (data)
void  SetUserProjectDefaultExternalFlashConfig(); // external flash에 데이터가 한번도 안썼다면 사용자 데이터(userData) 초기화 다른 파일에서 이함수가 정의가 되 있어야 한다
uint8 LoadExternalFlashConfig();                  // external flash에서 데이터를 호출한다
void  SaveExternalFlashConfig();                  // external flash에 데이터를 저장한다

uint16 externalFlashCRC();

//////////////////////////////////////////////////////////////////////////////////
enum EXTERNAL_CONFIG_DATA {                       // EXTERNAL_CONFIG.data에 놓여있는 주소
    CONFIG_DATA_CONFIG     =   0,                 // Config데이터를 저장할 주소
    CONFIG_DATA_MISC       = 150,                 // EXTERNAL_MISC_CONFIG가 놓여 있을 위치
};

//////////////////////////////////////////////////////////////////////////////////
typedef struct {
    uint16 bDisplayDir;
    uint16 uBrightness;
    uint16 uStartMenu;                            // 0 : 모니터링, 1:안돈
} EXTERNAL_MISC_CONFIG;

EXTERNAL_MISC_CONFIG *getMiscConfig();

void ValidationExternalFlashConfig();

void SaveMiscConfig();
//////////////////////////////////////////////////////////////////////////////////
uint16 checkSum(uint8 *data, uint16 size); // checksum function
#endif    