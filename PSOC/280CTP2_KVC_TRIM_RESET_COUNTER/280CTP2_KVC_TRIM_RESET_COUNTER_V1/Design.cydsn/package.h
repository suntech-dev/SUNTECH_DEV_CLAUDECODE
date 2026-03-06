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
#ifndef _PACKAGE_H_
#define _PACKAGE_H_

#include "main.h"
    
//#define PROJECT_NAME1
#define USER_PROJECT_TRIM_COUNT
//#define USER_PROJECT_PATTERN_SEWING_MACHINE  

#ifdef USER_PROJECT_PATTERN_SEWING_MACHINE
    #define PROJECT_FIRMWARE_VERSION "1.1.4"    
#endif    

#ifdef USER_PROJECT_TRIM_COUNT
    #define PROJECT_FIRMWARE_VERSION "0.0.1"    
#endif

void initUserProject();
uint8 userProjectDataValidation();
#endif    
/* [] END OF FILE */
