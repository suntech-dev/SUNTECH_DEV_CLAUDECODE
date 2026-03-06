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

#ifdef USER_PROJECT_PATTERN_SEWING_MACHINE

enum MACHINE_TYPE {
    PATTERN_MACHINE = 0,
    SEWING_MACHINE,    
};

typedef struct {
    uint8  machineType;
    uint16 patternTarget;
    uint16 patternPairCount;
    uint16 patternPair;
    
    uint16 sewingTarget;
    uint16 sewingPairTrim;
    uint16 sewingPair;    
} MACHINE_PARAMETER;

extern MACHINE_PARAMETER *g_ptrMachineParameter;

#endif
/* [] END OF FILE */
