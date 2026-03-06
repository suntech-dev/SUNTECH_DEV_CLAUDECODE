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

#ifndef __MAIN_H__
#define __MAIN_H__
    
#include "project.h"  
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <stdarg.h>
        
#define FIRMWARE_VERSION "20211101(0.0.8)"
    
#ifndef ON
    #define ON (1u)
#endif
#ifndef OFF
    #define OFF (0u)
#endif
#ifndef TRUE
    #define TRUE (1u)
#endif
#ifndef FALSE
    #define FALSE (0u)
#endif

#define MIN(a,b) ((a) < (b) ? (a) : (b))
#define MAX(a,b) ((a) > (b) ? (a) : (b))

#define STX 0x02
#define ETX 0x03
#define EOT 0x04   
#define LF  0x0A
#define FF  0x0C    
#define CR  0x0D
#define ESC 0x1B

#define CONVERT_TO_4BYTE(a,b) (((uint32_t)a << 16) | b)
#define CONVERT_TO_HIGH(a)    ((uint16_t)((a >> 16) & 0xFFFF))
#define CONVERT_TO_LOW(a)     ((uint16_t) (a & 0xFFFF))
#define ADD_CONVERT_TO_4BYTE(H,L,add) { uint32 tmp = CONVERT_TO_4BYTE(H,L)+add; H = CONVERT_TO_HIGH(tmp); L = CONVERT_TO_LOW(tmp); }
struct sParameter
{
    uint8   stat_TC_INT;
    uint16  pos_X;
    uint16  pos_Y;
};

struct sParameter Param;

//#define _WIFI_MONITORING_

#define BOOT_LOADERBLE_EXIST

// 부트로더 진입 함수
void EnterBootloaderMode(void);

#endif

/* [] END OF FILE */
