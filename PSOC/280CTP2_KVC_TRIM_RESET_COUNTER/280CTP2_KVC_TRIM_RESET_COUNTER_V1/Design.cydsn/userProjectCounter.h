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
#include "package.h"

#ifdef USER_PROJECT_TRIM_COUNT

#define MAX_SET_TRIM_COUNT 99
    
typedef struct {
    uint16  setTrimCount;
   // uint16  curTrimCount;
} MACHINE_PARAMETER;

extern MACHINE_PARAMETER *g_ptrMachineParameter;

#endif
/* [] END OF FILE */
