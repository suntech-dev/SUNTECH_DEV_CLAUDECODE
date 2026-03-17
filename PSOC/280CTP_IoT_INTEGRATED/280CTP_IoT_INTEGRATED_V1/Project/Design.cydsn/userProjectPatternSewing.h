
#include "package.h"

#ifdef USER_PROJECT_PATTERN_SEWING_MACHINE

#ifndef _USER_PROJECT_PATTERN_SEWING_H_
#define _USER_PROJECT_PATTERN_SEWING_H_

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
    
    uint32 lastPowerOnDateTime;
    
    uint16 andon_enable;
    uint16 current_enable;
    uint16 current_sensor_threshold;
    
    uint8  bAutoReset;
    
} MACHINE_PARAMETER;

extern MACHINE_PARAMETER *g_ptrMachineParameter;

uint32 getTarget();
void setTarget(uint32 value);

uint32 getActual();
void setActual(uint32 value);
             

#endif    

#endif
/* [] END OF FILE */