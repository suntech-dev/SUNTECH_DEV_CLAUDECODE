
#ifndef _PACKAGE_H_
#define _PACKAGE_H_

#include "main.h"
    
//#define PROJECT_NAME1
#define USER_PROJECT_PATTERN_SEWING_MACHINE  

#ifdef USER_PROJECT_PATTERN_SEWING_MACHINE
    #define PROJECT_FIRMWARE_VERSION "V2.0.0"
#endif

//#define USE_CURRENT_SENSOR_FOR_COUNTTING          // use 2pin port5.5

void initUserProject();
uint8 userProjectDataValidation();
#endif    
/* [] END OF FILE */